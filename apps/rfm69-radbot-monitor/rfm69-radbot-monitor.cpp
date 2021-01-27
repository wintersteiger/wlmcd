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
#include <rfm69.h>
#include <rfm69_ui.h>
#include <rfm69_ui_raw.h>
#include <radbot.h>
#include <radbot_ui.h>
#include <gpio_watcher.h>
#include <gpio_button.h>
#include <sleep.h>

static volatile int rx_cnt = 0;
static FILE *logfile = NULL;
static Controller *controller = NULL;
static int exit_code = 0;
static Radbot::Decoder *radbot_decoder = NULL;
static Radbot::Encoder *radbot_encoder = NULL;
static RadbotUI *radbot_ui = NULL;
static RFM69 *rfm69 = NULL;
static RFM69UI *rfm69_ui = NULL;
static RFM69UIRaw *rfm69_ui_raw = NULL;
static std::vector<GPIOWatcher<RFM69>*> gpio_watchers;

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
  delete(rfm69_ui);
  rfm69_ui = NULL;
  delete(rfm69_ui_raw);
  rfm69_ui_raw = NULL;

  delete(radbot_decoder);
  radbot_decoder = NULL;
  delete(radbot_encoder);
  radbot_encoder = NULL;
  delete(rfm69);
  rfm69 = NULL;

  if (logfile) {
    fclose(logfile);
    logfile = NULL;
  }

  mtx.unlock();

  if (UI::End() != OK)
    printf("UI cleanup error\n");

  if (signal != 0) {
    printf("Signal %d (%s); bailing out.\n", signal, strsignal(signal));
    exit_code = 2;
  }
}

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

static bool RFM69_fRX(RFM69 *rfm69, Radbot::Decoder *decoder)
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
    UI::Start();
    logfile = fopen("log.csv", "a");

    GPIOButton reset_button("/dev/gpiochip0", 6);

    rfm69 = new RFM69(1, 2, "rfm69-radbot.cfg");

    auto radbot_cfg = nlohmann::json::parse(std::ifstream("radbot.cfg"));
    std::string radbot_id = radbot_cfg["radbot"]["id"];
    std::string radbot_key = radbot_cfg["radbot"]["key"];
    radbot_decoder = new Radbot::Decoder(radbot_id, radbot_key);
    radbot_encoder = new Radbot::Encoder(radbot_id, radbot_key);

    rfm69_ui = new RFM69UI(*rfm69, &reset_button);
    rfm69_ui_raw = new RFM69UIRaw(*rfm69);
    radbot_ui = new RadbotUI(radbot_decoder->state, { rfm69 });

    gpio_watchers.push_back(new GPIOWatcher<RFM69>("/dev/gpiochip0", 26, "WLMCD-RFM69", rfm69,
      [](int, unsigned, const timespec*, RFM69 *rfm69) {
        return RFM69_fRX(rfm69, radbot_decoder);
      }));

    controller = new Controller(0);

    std::signal(SIGINT, cleanup);
    std::signal(SIGABRT, cleanup);

    controller->AddSystem(rfm69_ui);
    controller->AddSystem(rfm69_ui_raw);
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
