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

#include <curses.h>

#include <json.hpp>

#include <controller.h>
#include <ui.h>
#include <cc1101.h>
#include <cc1101_ui.h>
#include <cc1101_ui_raw.h>
#include <radbot.h>
#include <radbot_ui.h>
#include <sleep.h>

static volatile int rx_cnt = 0;
static FILE *logfile = NULL;
static Controller *controller = NULL;
static int exit_code = 0;
static CC1101 *cc1101 = NULL;
static CC1101UI *cc1101_ui = NULL;
static CC1101UIRaw *cc1101_ui_raw = NULL;
static RadbotUI *radbot_ui = NULL;
static Radbot::Decoder *radbot_decoder = NULL;
static Radbot::Encoder *radbot_encoder = NULL;
static std::vector<GPIOWatcher<CC1101>*> gpio_watchers;

static std::mutex mtx;

void cleanup(int signal = 0)
{
  mtx.lock();

  for (auto w : gpio_watchers)
    delete(w);
  gpio_watchers.clear();

  if (controller) {
    controller->Stop();
    int cleanup_it = 20;
    while (controller->Running() && --cleanup_it != 0)
      sleep_ms(50);
  }

  delete(controller);
  controller = NULL;
  delete(radbot_ui);
  radbot_ui = NULL;
  delete(cc1101_ui);
  cc1101_ui = NULL;
  delete(cc1101_ui_raw);
  cc1101_ui_raw = NULL;

  delete(radbot_decoder);
  radbot_decoder = NULL;
  delete(radbot_encoder);
  radbot_encoder = NULL;
  delete(cc1101);
  cc1101 = NULL;

  if (logfile) {
    fclose(logfile);
    logfile = NULL;
  }

  mtx.unlock();

  if (UI::End() != OK)
    std::cout << "UI cleanup error" << std::endl;

  if (signal != 0) {
    std::cout << "Signal " << signal << " (" << strsignal(signal) << "); bailing out." << std::endl;
    exit_code = 2;
  }
}

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

static bool CC1101_fRX(CC1101 *cc1101, Radbot::Decoder *decoder)
{
  if (!decoder) {
    UI::Log("CC1101_fRX: no decoder");
    return true;
  }

  mtx.lock();

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

  mtx.unlock();

  return true;
}

int main()
{
  try {
    UI::Start();
    logfile = fopen("log.csv", "a");

    auto radbot_cfg = nlohmann::json::parse(std::ifstream("radbot.cfg"));
    std::string radbot_id = radbot_cfg["radbot"]["id"];
    std::string radbot_key = radbot_cfg["radbot"]["key"];
    radbot_decoder = new Radbot::Decoder(radbot_id, radbot_key);
    radbot_encoder = new Radbot::Encoder(radbot_id, radbot_key);

    cc1101 = new CC1101(0, 0, "cc1101-radbot.cfg");

    radbot_ui = new RadbotUI(radbot_decoder->state, { cc1101 });
    cc1101_ui = new CC1101UI(*cc1101);
    cc1101_ui_raw = new CC1101UIRaw(*cc1101);

    gpio_watchers.push_back(new GPIOWatcher<CC1101>("/dev/gpiochip0", 25, "WLMCD-CC1101", cc1101,
      [](int, unsigned, const timespec*, CC1101 *cc1101) {
        return CC1101_fRX(cc1101, radbot_decoder);
      }));

    controller = new Controller(0);

    std::signal(SIGINT, cleanup);
    std::signal(SIGABRT, cleanup);

    controller->AddSystem(cc1101_ui);
    controller->AddSystem(cc1101_ui_raw);
    controller->AddSystem(radbot_ui, radbot_decoder, radbot_encoder);

    controller->Run();
  }
  catch (std::exception &ex) {
    cleanup();
    std::cout << "Exception: " << ex.what() << std::endl;
    exit_code = 1;
  }
  catch (...) {
    cleanup();
    std::cout << "Caught unknown exception." << std::endl;
    exit_code = 1;
  }

  if (exit_code == 0)
    cleanup();

  return exit_code;
}
