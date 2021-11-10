// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <cinttypes>
#include <cstring>
#include <curses.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <mutex>
#include <memory>
#include <map>

#include <shell.h>
#include <ui.h>
#include <sleep.h>
#include <gpio_watcher.h>
#include <cc1101.h>
#include <cc1101_ui.h>
#include <cc1101_ui_raw.h>
#include <spirit1.h>
#include <spirit1_ui.h>
#include <spirit1_rt.h>
#include <gpio_button.h>

#include "radio_test_ui.h"
#include "radio_test_tracker.h"

static volatile int rx_cnt = 0;
static FILE *logfile = NULL;
static std::mutex radio_mtx, log_mtx;
static std::shared_ptr<RadioTestTracker> radio_test_tracker = nullptr;
uint64_t irqs = 0;

static int rxlog(double rssi, double lqi, const std::vector<uint8_t> &raw_packet, const std::string &msg, const std::string &err)
{
  int r = 0;
  if (logfile) {
    const std::lock_guard<std::mutex> lock(log_mtx);
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

static bool fRX(std::shared_ptr<Radio> radio)
{
  std::vector<std::vector<uint8_t>> packets;
  UI::Log("RX");
  {
    const std::lock_guard<std::mutex> lock(radio_mtx);

    double rssi = 0.0, lqi = 0.0;
    std::vector<uint8_t> packet;
    radio->Receive(packet);

    if (packet.size() > 2) {
      // rssi = CC1101::rRSSI(packet.back());
      // packet.pop_back();
      // lqi = CC1101::rLQI(packet.back());
      // packet.pop_back();
    }

    // if (radio->GetState() != CC1101::State::RX) {
    //   UI::Log("RX failed, restarting.");
    //   radio->StrobeFor(CC1101::SRX, CC1101::State::RX, 10);
    // }

    char lbuf[1024];
    char *p = &lbuf[0];

    if (packet.size() > 0) {
      p += sprintf(p, "RX rssi=%4.0fdBm lqi=%3.0f%% N=%d: ", rssi, lqi, packet.size());
      if (packet.size() < 128)
        p += sprintf(p, "%s", bytes_to_hex(packet).c_str());
      else
        p += sprintf(p, "...");

      std::vector<uint8_t> raw_packet = packet;
      std::string msg_str = "", err_str = "";
      bool decoded = false;

      try {
        packets.push_back(packet);
      }
      catch (const std::runtime_error &err) {
        err_str = err.what();
        p += sprintf(p, " ERROR: %s", err.what());
      }

      rxlog(rssi, lqi, raw_packet, msg_str, err_str);

      UI::Log(lbuf);
      rx_cnt++;
    }
  }

  for (const auto &packet : packets)
    radio_test_tracker->receive(packet);

  return true;
}

static bool fIRQ(std::shared_ptr<Radio> radio)
{
  irqs = radio->IRQHandler();
  if (irqs & 0x01)
    return fRX(radio);
  return true;
}

static void fTX(std::shared_ptr<Radio> radio, const std::vector<uint8_t> &packet)
{
  UI::Log("TX: %s", bytes_to_hex(packet).c_str());

  const std::lock_guard<std::mutex> lock(radio_mtx);
  try {
    radio->Transmit(packet);
  }
  catch (std::exception &ex) {
    UI::Log("Exception during TX: %s", ex.what());
  }
  catch (...) {
    UI::Log("Unknown exception during TX");
  }

  radio->Goto(Radio::State::RX);
}

static void manualTX(std::shared_ptr<Radio> radio, const std::string &args)
{
  auto argbytes = from_hex(args);

  if (argbytes.empty())
    UI::Error("argument is not in hex format");
  else
    fTX(radio, argbytes);
}

int main(int argc, const char **argv)
{
  if (argc < 2) {
    std::cout << "Usage: " << argv[0] << " cc1101|spirit1" << std::endl;
    return 1;
  }

  try
  {
    auto shell = get_shell(1.0);

    std::shared_ptr<Radio> radio;
    std::shared_ptr<UI> radio_ui, radio_ui_raw;

    if (strcmp(argv[1], "cc1101") == 0) {
      auto cc1101 = std::make_shared<CC1101>(0, 0, "cc1101.cfg");
      radio = std::static_pointer_cast<Radio>(cc1101);
      radio_ui = std::make_shared<CC1101UI>(cc1101);
      radio_ui_raw = make_cc1101_raw_ui(cc1101);
    }
    else if (strcmp(argv[1], "spirit1") == 0) {
      auto spirit1 = std::make_shared<SPIRIT1>(0, 0, "spirit1.cfg");
      radio = std::static_pointer_cast<Radio>(spirit1);
      radio_ui = std::make_shared<SPIRIT1UI>(spirit1, irqs);
      auto reset_button = std::make_shared<GPIOButton>("/dev/gpiochip0", 5);
      radio_ui_raw = make_spirit1_raw_ui(spirit1, reset_button);
    }
    else
      throw std::runtime_error(std::string("Unknown radio `") + argv[1] + "`");

    radio_test_tracker = std::make_shared<RadioTestTracker>(
      [radio](const std::vector<uint8_t> &packet){ fTX(radio, packet); });

    std::vector<std::shared_ptr<Radio>> radio_devs = {radio};
    auto radio_test_ui = std::make_shared<RadioTestUI>(radio_devs, radio_test_tracker);

    std::vector<GPIOWatcher<Radio>*> gpio_watchers;
    if (argv[1] == "cc1101") {
      gpio_watchers.push_back(new GPIOWatcher<Radio>("/dev/gpiochip0", 25, "WLMCD-CC1101", radio, false,
        [](int, unsigned, const timespec*, std::shared_ptr<Radio> radio) {
          return fRX(radio);
        }));
    }
    else {
      gpio_watchers.push_back(new GPIOWatcher<Radio>("/dev/gpiochip0", 25, "WLMCD-CC1101", radio, false,
        [](int, unsigned, const timespec*, std::shared_ptr<Radio> radio) {
          return fIRQ(radio);
        }));
    }

    auto tf = [radio](const std::string &args) { manualTX(radio, args); };
    shell->controller->AddCommand("t", tf);
    shell->controller->AddCommand("transmit", tf);

    shell->controller->AddCommand("log", [](const std::string &args) {
      const std::lock_guard<std::mutex> lock(log_mtx);
      if (args.empty()) {
        fclose(logfile);
        logfile = NULL;
        UI::Info("Logging disabled");
      } else {
        if (logfile) fclose(logfile);
        logfile = fopen(args.c_str(), "a");
        UI::Info("Logging to %s", args.c_str());
      }
    });

    shell->controller->AddSystem(radio_ui);
    shell->controller->AddSystem(radio_ui_raw);
    shell->controller->AddSystem(radio_test_ui);
    shell->controller->AddBackgroundDevice(radio_test_tracker);

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
