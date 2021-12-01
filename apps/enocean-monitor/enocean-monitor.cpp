// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <cinttypes>
#include <vector>
#include <iostream>
#include <fstream>
#include <mutex>
#include <memory>
#include <map>

#include <shell.h>
#include <serialization.h>
#include <cc1101.h>
#include <cc1101_ui.h>
#include <cc1101_ui_raw.h>
#include <spirit1.h>
#include <spirit1_ui.h>
#include <ui.h>
#include <sleep.h>
#include <integrity.h>
#include <gpio_button.h>
#include <bme280.h>
#include <bme280_ui.h>

#include "enocean.h"
#include "enocean_telegrams.h"
#include "enocean_ui.h"
#include "enocean_gateway.h"

static volatile int rx_cnt = 0;
static FILE *logfile = NULL;
static std::mutex mtx, log_mtx;
static std::shared_ptr<Shell> shell;
static std::unique_ptr<EnOcean::Gateway> gateway;
uint64_t irqs = 0;

static int flog(const char *label, double rssi, double lqi, const std::vector<uint8_t> &raw_packet, const std::string &msg, const std::string &err)
{
  int r = 0;
  if (logfile) {
    const std::lock_guard<std::mutex> lock(log_mtx);
    static char time_buf[256];
    time_t t;
    time(&t);
    struct tm * lt = localtime(&t);
    strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", lt);
    fprintf(logfile, "%s,\"%s\",%0.2f,%0.2f,", time_buf, label, rssi, lqi);
    for (auto b : raw_packet)
      fprintf(logfile, "%02x", b);
    fprintf(logfile, ",\"");
    for (auto c : msg)
      if (c == '\"') fprintf(logfile, "\\\"");
      else fprintf(logfile, "%c", c);
    fprintf(logfile, "\",\"");
    for (auto c : err)
      if (c == '\"') fprintf(logfile, "\\\"");
      else fprintf(logfile, "%c", c);
    fprintf(logfile, "\"");
    fprintf(logfile, "\n");
    fflush(logfile);
  }
  return r;
}

static bool fRX(std::shared_ptr<Radio> radio, std::shared_ptr<EnOcean::Decoder> decoder, std::shared_ptr<EnOcean::Encoder> encoder)
{
  if (!decoder) {
    UI::Log("no decoder");
    return true;
  }

  std::vector<EnOcean::Frame> frames;
  double rssi = 0.0;
  double lqi = 0.0;

  {
    const std::lock_guard<std::mutex> lock(mtx);

    std::vector<uint8_t> packet;
    radio->Receive(packet);

    rssi = radio->RSSI();
    lqi = radio->LQI();

    char lbuf[1024];
    char *p = &lbuf[0];

    if (packet.size() > 0) {
      p += sprintf(p, "RX rssi=%4.0fdBm lqi=%3.0f%% N=%d", rssi, lqi, packet.size());

      std::string err_str = "";
      bool decoded = false;

      try {
        frames = decoder->get_frames(packet);
      }
      catch (const std::runtime_error &err) {
        err_str = err.what();
        p += sprintf(p, " ERROR: %s", err.what());
      }
    }

    if (frames.empty())
      p += sprintf(p, ": %s", bytes_to_hex(packet).c_str());
    else
    {
      p += sprintf(p, " Frames:");
      for (auto& f : frames) {
        auto d = f.describe();
        p += sprintf(p, " %s", d.c_str());
        flog("RX", rssi, lqi, packet, d.c_str(), "");
      }
    }

    UI::Log(lbuf);
    rx_cnt++;
  }

  radio->Goto(Radio::State::RX);

  for (size_t i=0; i < frames.size(); i++) {
    const EnOcean::Frame &f = frames[i];
    if (f.crc_ok() && (i == 0 || f != frames[i-1]))
      gateway->receive(f, rssi);
  }

  return true;
}

static void fTX(std::shared_ptr<Radio> radio, std::shared_ptr<EnOcean::Encoder> encoder, const EnOcean::Telegram &t)
{
  const EnOcean::Frame &f = t;
  std::string msg = bytes_to_hex(f);
  UI::Log("TX: %s", msg.c_str());
  flog("TX", 0.0, 0.0, f, f.describe().c_str(), "TX");
  auto encoded = encoder->Encode(f);
  // UI::Log("TX: %s ~ %s", msg.c_str(), bytes_to_hex(encoded).c_str());
  radio->Transmit(encoded);
  sleep_ms(5);
  radio->Transmit(encoded);
  sleep_ms(20);
  radio->Transmit(encoded);

  radio->Goto(Radio::State::RX);
}

static bool fIRQ(std::shared_ptr<Radio> radio, std::shared_ptr<EnOcean::Decoder> decoder, std::shared_ptr<EnOcean::Encoder> encoder)
{
  irqs = radio->IRQHandler();
  if (shell)
    shell->controller->Update(false);
  if (irqs & 0x00000201)
    return fRX(radio, decoder, encoder);
  return true;
}

static void manualTX(std::shared_ptr<Radio> radio, std::shared_ptr<EnOcean::Encoder> encoder, const std::string &args, bool encapsulate)
{
  auto argbytes = from_hex(args);

  if (argbytes.empty()) {
    UI::Error("argument is not in hex format");
    return;
  }

  auto rorg = argbytes[0];
  auto payload = std::vector<uint8_t>(argbytes.begin() + 1, argbytes.end());
  EnOcean::Telegram t(EnOcean::Frame(argbytes[0], payload, gateway->txid(), 0x8F));
  // EnOcean::Telegram t(EnOcean::Frame(argbytes[0], payload, 0x00, 0x8F));

  if (!encapsulate)
    fTX(radio, encoder, t);
  else
    fTX(radio, encoder, EnOcean::AddressedTelegram(t, 0x0580CC3A));
}

int main()
{
  try {
    shell = get_shell(1.0);
    logfile = fopen("log.csv", "a");
    std::string gateway_config = "enocean-gateway.json";

    auto decoder = std::make_shared<EnOcean::Decoder>();
    auto encoder = std::make_shared<EnOcean::Encoder>();

#ifdef USE_C1101
    auto radio = std::make_shared<CC1101>(0, 0, "cc1101.cfg");
    auto radio_ui = std::make_shared<CC1101UI>(radio);
    auto radio_ui_raw = make_cc1101_raw_ui(radio);
#else
    auto reset_button = std::make_shared<GPIOButton>("/dev/gpiochip0", 5);
    reset_button->Write(true);
    sleep_ms(2);
    reset_button->Write(false);
    sleep_ms(2);
    auto radio = std::make_shared<SPIRIT1>(0, 0, "spirit1.cfg");
    auto radio_ui = std::make_shared<SPIRIT1UI>(radio, irqs);
    auto radio_ui_raw = make_spirit1_raw_ui(radio, reset_button);
#endif

    gateway = std::make_unique<EnOcean::Gateway>(
      [radio, encoder](const EnOcean::Frame &f){ fTX(radio, encoder, f); });

    std::vector<std::shared_ptr<DeviceBase>> radio_devs = {radio};
    auto bme280 = std::make_shared<BME280>();
    auto enocean_ui = std::make_shared<EnOceanUI>(gateway, radio_devs, bme280);

    std::vector<GPIOWatcher<Radio>*> gpio_watchers;
#ifdef USE_C1101
    gpio_watchers.push_back(new GPIOWatcher<Radio>("/dev/gpiochip0", 25, "WLMCD-CC1101", radio, true,
      [encoder, decoder](int, unsigned, const timespec*, std::shared_ptr<Radio> radio) {
        return fRX(radio, decoder, encoder);
      }));
#else
    gpio_watchers.push_back(new GPIOWatcher<Radio>("/dev/gpiochip0", 25, "WLMCD-SPIRIT1", radio, false,
      [encoder, decoder](int, unsigned, const timespec*, std::shared_ptr<Radio> radio) {
        return fIRQ(radio, decoder, encoder);
      }));
#endif

    shell->controller->AddBackgroundDevice(std::make_shared<BackgroundTask>([&radio, encoder, decoder]() {
      if (radio->RXReady())
        fRX(radio, decoder, encoder);
      {
        const std::lock_guard<std::mutex> lock(mtx);
        radio->Goto(Radio::State::RX);
      }
    }));

    auto tf = [radio, encoder](const std::string &args) { manualTX(radio, encoder, args, false); };
    shell->controller->AddCommand("t", tf);
    shell->controller->AddCommand("transmit", tf);

    auto tfe = [radio, encoder](const std::string &args) { manualTX(radio, encoder, args, true); };
    shell->controller->AddCommand("ta", tfe);

    shell->controller->AddCommand("learn", [](const std::string &args){
      if (args != "off" && args != "on")
        UI::Error("unknown argument");
      else {
        gateway->set_learning(args == "on");
        UI::Info("learning is %s", args.c_str());
      }
    });

    shell->controller->AddCommand("crc8", [](const std::string &args){
      auto argbytes = from_hex(args);
      auto x = crc8(argbytes, 0x07);
      UI::Info("crc8 is %02x", x);
    });

    shell->controller->AddCommand("run", [radio](const std::string &args){
      radio->Goto(Radio::State::RX);
    });

    shell->controller->AddCommand("rx", [radio, decoder, encoder](const std::string &args){
      return fRX(radio, decoder, encoder);
    });

    shell->controller->AddCommand("s", [&gateway_config](const std::string &args){
      return gateway->save(gateway_config);
    });

    shell->controller->AddCommand("p", [&gateway_config](const std::string &args){
      return gateway->ping();
    });

    shell->controller->AddSystem(enocean_ui);
    shell->controller->AddSystem(radio_ui);
    shell->controller->AddSystem(radio_ui_raw);
    shell->controller->AddSystem(std::make_shared<BME280UI>(bme280));
    shell->controller->AddSystem(make_bme280_raw_ui(bme280));

    radio->Goto(Radio::State::RX);

    shell->controller->Run();
    return shell->exit_code;
  }
  catch (std::exception &ex) {
    UI::End();
    std::cout << "Exception: " << ex.what() << std::endl;
    return 1;
  }
  catch (...) {
    UI::End();
    std::cout << "Caught unknown exception." << std::endl;
    return 1;
  }
}
