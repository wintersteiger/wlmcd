// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <csignal>
#include <cstring>
#include <cinttypes>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <algorithm>

#include <json.hpp>

#include <shell.h>
#include <rfm69.h>
#include <rfm69_ui.h>
#include <rfm69_ui_raw.h>
#include <radbot.h>
#include <radbot_ui.h>
#include <gpio_watcher.h>
#include <gpio_button.h>

static volatile int rx_cnt = 0;
static FILE *logfile = NULL;
static std::mutex mtx;

int rxlog(double rssi, const std::vector<uint8_t> &raw_packet, const std::string &msg, const std::string &err)
{
  int r = 0;
  if (logfile) {
    static char time_buf[256];
    time_t t;
    time(&t);
    struct tm * lt = localtime(&t);
    strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", lt);
    fprintf(logfile, "%s,%0.2f,", time_buf, rssi);
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

static bool RFM69_fRX(std::shared_ptr<RFM69> rfm69, std::shared_ptr<Radbot::Decoder> decoder)
{
  if (!decoder) {
    UI::Log("RFM69_fRX: no decoder");
    return true;
  }

  mtx.lock();

  static std::vector<uint8_t> packet;
  double rssi = rfm69->rRSSI();
  rfm69->Receive(packet);

  static char lbuf[1024];
  char *p = &lbuf[0];

  if (packet.size() > 0) {
    p += snprintf(p, sizeof(lbuf), "RX rssi=%4.0fdBm N=%d ", rssi, packet.size());

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

    rxlog(rssi, raw_packet, msg_str, err_str);
  }
  else
    p += sprintf(p, "RX failed.");

  UI::Log(lbuf);
  rx_cnt++;

  mtx.unlock();

  return true;
}

int main(void)
{
  try {
    auto shell = get_shell(0);
    logfile = fopen("log.csv", "a");

    GPIOButton reset_button("/dev/gpiochip0", 6);

    auto rfm69 = std::make_shared<RFM69>(1, 2, "rfm69-radbot.cfg");

    auto radbot_cfg = nlohmann::json::parse(std::ifstream("radbot.cfg"));
    std::string radbot_id = radbot_cfg["radbot"]["id"];
    std::string radbot_key = radbot_cfg["radbot"]["key"];
    auto radbot_decoder = std::make_shared<Radbot::Decoder>(radbot_id, radbot_key);
    auto radbot_encoder = std::make_shared<Radbot::Encoder>(radbot_id, radbot_key);

    auto rfm69_ui = std::make_shared<RFM69UI>(*rfm69, &reset_button);
    auto rfm69_ui_raw = std::make_shared<RFM69UIRaw>(*rfm69);

    std::vector<std::shared_ptr<DeviceBase>> devs = {rfm69};
    auto radbot_ui = std::make_shared<RadbotUI>(radbot_decoder->state, devs);

    std::vector<GPIOWatcher<RFM69>*> gpio_watchers;
    gpio_watchers.push_back(new GPIOWatcher<RFM69>("/dev/gpiochip0", 26, "WLMCD-RFM69", rfm69,
      [&radbot_decoder](int, unsigned, const timespec*, std::shared_ptr<RFM69> rfm69) {
        return RFM69_fRX(rfm69, radbot_decoder);
      }));

    shell->controller->AddSystem(rfm69_ui);
    shell->controller->AddSystem(rfm69_ui_raw);
    shell->controller->AddSystem(radbot_ui, radbot_decoder, radbot_encoder);
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

  return 0;
}
