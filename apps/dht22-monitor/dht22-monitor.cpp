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
#include <dht22.h>
#include <dht22_ui.h>
#include <sleep.h>

static std::mutex mtx;
static Controller *controller = NULL;
static int exit_code = 0;
static DHT22 *dht22 = NULL;
static DHT22UI *dht22_ui = NULL;

void cleanup(int signal = 0)
{
  mtx.lock();

  if (controller) {
    controller->Stop();
    int cleanup_it = 20;
    while (controller->Running() && --cleanup_it != 0)
      sleep_ms(50);
  }

  delete(dht22_ui);
  dht22_ui = NULL;
  delete(dht22);
  dht22 = NULL;

  mtx.unlock();

  if (UI::End() != OK)
    printf("UI cleanup error\n");

  if (signal != 0) {
    printf("Signal %d (%s); bailing out.\n", signal, strsignal(signal));
    exit_code = 2;
  }
}

int main(void)
{
  try {
    UI::Start();

    dht22 = new DHT22("/dev/gpiochip0", 4);
    dht22_ui = new DHT22UI(*dht22);

    controller = new Controller(1.0);

    std::signal(SIGINT, cleanup);
    std::signal(SIGABRT, cleanup);

    controller->AddSystem(dht22_ui);

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
