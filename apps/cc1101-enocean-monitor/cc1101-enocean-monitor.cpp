// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <cinttypes>
#include <vector>
#include <iostream>
#include <fstream>
#include <mutex>

#include <json.hpp>

#include <shell.h>
#include <cc1101.h>
#include <cc1101_ui.h>
#include <cc1101_ui_raw.h>
#include <enocean.h>

static volatile int rx_cnt = 0;
static FILE *logfile = NULL;
static std::mutex mtx;

int rxlog(double rssi, double lqi, const std::vector<uint8_t> &raw_packet, const std::string &msg, const std::string &err)
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

static bool CC1101_fRX(std::shared_ptr<CC1101> cc1101, std::shared_ptr<EnOcean::Decoder> decoder)
{
  if (!decoder) {
    UI::Log("CC1101_fRX: no decoder");
    return true;
  }

  const std::lock_guard<std::mutex> lock(mtx);

  std::vector<uint8_t> packet;
  cc1101->Receive(packet);
  size_t pkt_sz = packet.size();

  char lbuf[1024];
  char *p = &lbuf[0];

  if (pkt_sz > 2) {
    double rssi = cc1101->rRSSI(packet.back());
    packet.pop_back();
    double lqi = cc1101->rLQI(packet.back());
    packet.pop_back();

    p += snprintf(p, sizeof(lbuf), "RX rssi=%4.0fdBm lqi=%3.0f%% N=%d ", rssi, lqi, packet.size());

    std::vector<uint8_t> raw_packet = packet;
    std::string msg_str = "", err_str = "";
    bool decoded = false;

    try {
      msg_str = decoder->Decode(packet);
      p += sprintf(p, "MSG: %s", msg_str.c_str());
    }
    catch (const std::runtime_error &err) {
      err_str = err.what();
      p += sprintf(p, "ERROR: %s", err.what());
    }

    rxlog(rssi, lqi, raw_packet, msg_str, err_str);
  }
  else
    p += sprintf(p, "RX failed.");

  UI::Log(lbuf);
  rx_cnt++;

  return true;
}

int main()
{
  try {
    auto shell = get_shell(0);
    logfile = fopen("log.csv", "a");

    auto enocean_decoder = std::make_shared<EnOcean::Decoder>();
    auto enocean_encoder = std::make_shared<EnOcean::Encoder>();

    auto cc1101 = std::make_shared<CC1101>(0, 0, "cc1101-enocean.cfg");

    std::vector<std::shared_ptr<DeviceBase>> devs = {cc1101};
    auto cc1101_ui = std::make_shared<CC1101UI>(*cc1101);
    auto cc1101_ui_raw = std::make_shared<CC1101UIRaw>(*cc1101);

    std::vector<GPIOWatcher<CC1101>*> gpio_watchers;
    gpio_watchers.push_back(new GPIOWatcher<CC1101>("/dev/gpiochip0", 25, "WLMCD-CC1101", cc1101,
      [&enocean_decoder](int, unsigned, const timespec*, std::shared_ptr<CC1101> cc1101) {
        return CC1101_fRX(cc1101, enocean_decoder);
      }));

    shell->controller->AddSystem(cc1101_ui);
    shell->controller->AddSystem(cc1101_ui_raw);
    shell->controller->Run();
    return shell->exit_code;
  }
  catch (std::exception &ex) {
    std::cout << "Exception: " << ex.what() << std::endl;
    return 1;
  }
  catch (...) {
    std::cout << "Caught unknown exception." << std::endl;
    return 1;
  }
}
