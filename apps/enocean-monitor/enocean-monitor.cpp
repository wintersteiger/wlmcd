// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <cinttypes>
#include <vector>
#include <iostream>
#include <fstream>
#include <mutex>
#include <memory>
#include <map>
#include <random>

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

#include "enocean_frame.h"
#include "enocean_codec.h"
#include "enocean_telegrams.h"
#include "enocean_ui.h"
#include "enocean_gateway.h"

static std::mutex mtx;
static std::shared_ptr<Shell> shell;
static std::unique_ptr<EnOcean::Gateway> gateway;
static uint64_t irqs = 0, num_manual = 0;

static bool fRX(std::shared_ptr<Radio> radio)
{
  const std::lock_guard<std::mutex> lock(mtx);

  std::vector<uint8_t> packet;
  radio->Receive(packet);

  double rssi = radio->RSSI();
  double lqi = radio->LQI();

  gateway->receive(std::move(packet), rssi);

  radio->Goto(Radio::State::RX);

  return true;
}

static void fTX(std::shared_ptr<Radio> radio, std::shared_ptr<EnOcean::Encoder> encoder, const EnOcean::Frame &f)
{
  static std::random_device rd;
  static std::mt19937 gen(rd());
  static std::uniform_real_distribution<> rdist(0.0, 1.0);

  auto encoded = encoder->Encode(f);

  uint64_t slot2 = 1 + 8.0 * rdist(gen);
  uint64_t slot3 = 20 + 19.0 * rdist(gen);
  {
    const std::lock_guard<std::mutex> lock(mtx);
    radio->Transmit(encoded);
    radio->Goto(Radio::State::RX);
  }
  sleep_ms(slot2-1);
  // sleep_ms(9);
  {
    const std::lock_guard<std::mutex> lock(mtx);
    radio->Transmit(encoded);
    radio->Goto(Radio::State::RX);
  }
  // sleep_ms(10);
  sleep_ms(slot3-slot2-1);
  {
    const std::lock_guard<std::mutex> lock(mtx);
    radio->Transmit(encoded);
    radio->Goto(Radio::State::RX);
  }
  // sleep_ms(25);
  sleep_ms(40-slot3-1);
}

static bool fIRQ(std::shared_ptr<Radio> radio)
{
  irqs = radio->IRQHandler();
  if (shell)
    shell->controller->Update(false);
  if (irqs & 0x00000201)
    return fRX(radio);
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

  if (!encapsulate) {
    fTX(radio, encoder, static_cast<EnOcean::Frame>(t));
  }
  else {
    EnOcean::Frame f = EnOcean::AddressedTelegram(t, 0x050E3224 /*0x0580CC3A*/);
    fTX(radio, encoder, f);
  }
}

int main()
{
  try {
    shell = get_shell(1.0);
    std::string gateway_config = "enocean-gateway.json";
    std::string gateway_cache = "enocean-gateway.cache.json";

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
    auto bme280 = std::make_shared<BME280>("bme280.cfg");
    auto enocean_ui = std::make_shared<EnOceanUI>(gateway, radio_devs, bme280, &num_manual);

    std::vector<GPIOWatcher<Radio>*> gpio_watchers;
#ifdef USE_C1101
    gpio_watchers.push_back(new GPIOWatcher<Radio>("/dev/gpiochip0", 25, "WLMCD-CC1101", radio, true,
      [](int, unsigned, const timespec*, std::shared_ptr<Radio> radio) {
        return fRX(radio);
      }));
#else
    gpio_watchers.push_back(new GPIOWatcher<Radio>("/dev/gpiochip0", 25, "WLMCD-SPIRIT1", radio, false,
      [](int, unsigned, const timespec*, std::shared_ptr<Radio> radio) {
        return fIRQ(radio);
      }));
#endif

    shell->controller->AddBackgroundDevice(std::make_shared<BackgroundTask>([radio]() {
      if (mtx.try_lock()) {
        if (radio->RXReady()) {
          mtx.unlock();
          fRX(radio);
          num_manual++;
        }
        else {
          radio->Goto(Radio::State::RX);
          mtx.unlock();
        }
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

    shell->controller->AddCommand("checksum", [](const std::string &args){
      auto argbytes = from_hex(args);
      auto x = checksum(argbytes);
      UI::Info("checksum is %02x", x);
    });

    shell->controller->AddCommand("checkxor", [](const std::string &args){
      auto argbytes = from_hex(args);
      auto x = checkxor(argbytes);
      UI::Info("checkxor is %02x", x);
    });

    shell->controller->AddCommand("run", [radio](const std::string &args){
      radio->Goto(Radio::State::RX);
    });

    shell->controller->AddCommand("rx", [radio](const std::string &args){
      return fRX(radio);
    });

    shell->controller->AddCommand("s", [&gateway_config, &gateway_cache](const std::string &args){
      return gateway->save(gateway_config, gateway_cache);
    });

    shell->controller->AddCommand("p", [](const std::string &args){
      return gateway->ping();
    });

    shell->controller->AddCommand("i", [](const std::string &args){
      return gateway->inject(std::make_shared<EnOcean::Frame>(from_hex(args)), 0.0);
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
