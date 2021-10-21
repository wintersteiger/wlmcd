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
#include <cc1101.h>
#include <cc1101_ui.h>
#include <cc1101_ui_raw.h>
#include <ui.h>

#include "enocean.h"
#include "enocean_telegrams.h"
#include "enocean_ui.h"
#include "enocean_gateway.h"

typedef CC1101 Radio;

static volatile int rx_cnt = 0;
static FILE *logfile = NULL;
static std::mutex mtx;

static std::unique_ptr<EnOcean::Gateway> gateway;
static std::shared_ptr<EnOceanUI> enocean_ui;

static int rxlog(double rssi, double lqi, const std::vector<uint8_t> &raw_packet, const std::string &msg, const std::string &err)
{
  int r = 0;
  if (logfile) {
    static char time_buf[256];
    time_t t;
    time(&t);
    struct tm * lt = localtime(&t);
    strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", lt);
    fprintf(logfile, "%s,%0.2f,%0.2f,", time_buf, rssi, lqi);
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

  {
    const std::lock_guard<std::mutex> lock(mtx);

    double rssi = 0.0, lqi = 0.0;
    std::vector<uint8_t> packet;
    radio->Receive(packet);

    if (packet.size() > 2) {
      rssi = CC1101::rRSSI(packet.back());
      packet.pop_back();
      lqi = CC1101::rLQI(packet.back());
      packet.pop_back();
    }

    if (radio->GetState() != CC1101::State::RX) {
      UI::Log("RX failed, restarting.");
      radio->StrobeFor(CC1101::SRX, CC1101::State::RX, 10);
    }

    char lbuf[1024];
    char *p = &lbuf[0];

    if (packet.size() > 0) {
      p += sprintf(p, "RX rssi=%4.0fdBm lqi=%3.0f%% N=%d", rssi, lqi, packet.size());

      std::vector<uint8_t> raw_packet = packet;
      std::string msg_str = "", err_str = "";
      bool decoded = false;

      try {
        frames = decoder->get_frames(packet);
        for (auto& f : frames)
          p += sprintf(p, " MSG: %s", f.describe().c_str());
      }
      catch (const std::runtime_error &err) {
        err_str = err.what();
        p += sprintf(p, " ERROR: %s", err.what());
      }

      rxlog(rssi, lqi, raw_packet, msg_str, err_str);
    }

    UI::Log(lbuf);
    rx_cnt++;
  }

  for (auto &f : frames)
    gateway->receive(f);

  return true;
}

static void fTX(std::shared_ptr<Radio> radio, std::shared_ptr<EnOcean::Encoder> encoder, const EnOcean::Frame &f)
{
  UI::Log("TX: %s", bytes_to_hex(f).c_str());
  const std::lock_guard<std::mutex> lock(mtx);
  auto encoded = encoder->Encode(f);
  radio->Transmit(encoded);
  radio->Strobe(CC1101::CommandStrobe::SRX);
}

static void manualTX(std::shared_ptr<Radio> radio, std::shared_ptr<EnOcean::Encoder> encoder, const std::string &args)
{
  auto argbytes = from_hex(args);

  if (argbytes.empty()) {
    UI::Error("argument is not in hex format");
    return;
  }

  if (argbytes.size() < 4) {
    UI::Error("argument too short");
    return;
  }

  EnOcean::Frame f;
  std::array<uint8_t, 4> data = { argbytes[0], argbytes[1], argbytes[2], argbytes[3] };
  auto status = 0x80;
  EnOcean::Telegram_4BS t(data, gateway->txid(), 0x0580CC3A, status, f);
  fTX(radio, encoder, f);
}

int main()
{
  try {
    auto shell = get_shell(0);
    logfile = fopen("log.csv", "a");

    auto decoder = std::make_shared<EnOcean::Decoder>();
    auto encoder = std::make_shared<EnOcean::Encoder>();

    auto radio = std::make_shared<CC1101>(0, 0, "cc1101-enocean.cfg");
    auto radio_ui = std::make_shared<CC1101UI>(*radio);
    auto radio_ui_raw = std::make_shared<CC1101UIRaw>(*radio);

    gateway = std::make_unique<EnOcean::Gateway>(
      [radio, encoder](const EnOcean::Frame &f){ fTX(radio, encoder, f); });

    std::vector<std::shared_ptr<DeviceBase>> radio_devs = {radio};
    enocean_ui = std::make_shared<EnOceanUI>(gateway, radio_devs);

    std::vector<GPIOWatcher<Radio>*> gpio_watchers;
    gpio_watchers.push_back(new GPIOWatcher<Radio>("/dev/gpiochip0", 25, "WLMCD-CC1101", radio,
      [&decoder, &encoder](int, unsigned, const timespec*, std::shared_ptr<Radio> radio) {
        return fRX(radio, decoder, encoder);
      }));

    auto tf = [radio, encoder](const std::string &args) { manualTX(radio, encoder, args); };
    shell->controller->AddCommand("t", tf);
    shell->controller->AddCommand("transmit", tf);

    shell->controller->AddSystem(radio_ui);
    shell->controller->AddSystem(radio_ui_raw);
    shell->controller->AddSystem(enocean_ui);
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
