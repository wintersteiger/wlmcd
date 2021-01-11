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

#include <wiringPi.h>
#include <wiringPiSPI.h>

#include <json.hpp>

#include <controller.h>
#include <ui.h>
#include <cc1101.h>
#include <cc1101_ui.h>
#include <cc1101_ui_raw.h>
#include <radbot.h>
#include <radbot_ui.h>

static volatile int rx_cnt = 0;
static FILE *logfile = NULL;
static Controller *controller = NULL;
static int return_value = 0;

void cleanup(int signal = 0)
{
  if (controller) {
    controller->Stop();
    int cleanup_it = 20;
    while (controller->Running() && --cleanup_it != 0)
      delay(25);
    delete controller;
    controller = NULL;
  }

  if (logfile) {
    fclose(logfile);
    logfile = NULL;
  }

  if (UI::End() != OK)
    printf("UI cleanup error\n");

  if (signal != 0) {
    printf("Signal %d (%s); bailing out.\n", signal, strsignal(signal));
    return_value = 2;
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

CC1101 *cc1101 = NULL;
Decoder *decoder = NULL;

static void CC1101_fRX(void)
{
  CC1101::Status status = cc1101->GetStatus();

  static std::vector<uint8_t> packet;
  cc1101->Receive(packet);
  size_t pkt_sz = packet.size();

  static char lbuf[1024];
  char *p = &lbuf[0];

  p += snprintf(lbuf, sizeof(lbuf), "RCVD %lu bytes", pkt_sz);

  if (pkt_sz > 0) {
    if (pkt_sz > 64) pkt_sz = 64;
    double rssi = status.rRSSI();
    double lqi = status.rLQI();
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
}

int main(void)
{
  try {
    UI::Start();
    logfile = fopen("log.csv", "a");

    if (wiringPiSetup() != 0)
      throw std::runtime_error("Failed to set up wiringPi.");

    if (wiringPiSPISetup(0, 10000000 /* CC1101 max 10 Mhz */) < 0)
      throw std::runtime_error("Failed to set up SPI bus.");

    int GDO[3] = { 0, 0, 6 };
    CC1101 cc1101(GDO, 10, "cc1101-radbot.cfg");
    CC1101UI cc1101_ui(cc1101);
    CC1101UIRaw cc1101_ui_raw(cc1101);

    auto radbot_cfg = nlohmann::json::parse(std::ifstream("radbot.cfg"));
    std::string radbot_id = radbot_cfg["radbot"]["id"];
    std::string radbot_key = radbot_cfg["radbot"]["key"];
    Radbot::Decoder radbot_decoder(radbot_id, radbot_key);
    RadbotUI radbot_ui(radbot_decoder.state);

    ::cc1101 = &cc1101;
    ::decoder = &radbot_decoder;

    if (wiringPiISR(GDO[2], INT_EDGE_RISING, CC1101_fRX) < 0)
      throw std::runtime_error("Failed to set up CC1101 interrupt handler.");


    controller = new Controller(0);

    std::signal(SIGINT, cleanup);
    std::signal(SIGABRT, cleanup);

    controller->AddSystem(&cc1101_ui);
    controller->AddSystem(&cc1101_ui_raw);
    controller->AddSystem(&radbot_ui, &radbot_decoder);

    controller->Run();

    cleanup();
  }
  catch (std::exception &ex) {
    cleanup();
    printf("Exception: %s\n", ex.what());
    return_value = 1;
  }
  catch (...) {
    cleanup();
    printf("Caught unknown exception.\n");
    return_value = 1;
  }

  return return_value;
}
