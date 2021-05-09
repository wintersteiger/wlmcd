#include <cstring>
#include <csignal>
#include <memory>

#include "controller.h"
#include "ui.h"

#include "shell.h"

std::shared_ptr<Shell> Shell::instance = nullptr;

static void signal_handler(int signal)
{
  try
  {
    if (Shell::instance && Shell::instance->controller)
      Shell::instance->controller->Stop();

    if (UI::End() != OK)
      printf("UI cleanup error\n");

    if (signal != 0) {
      printf("Signal %d (%s); bailing out.\n", signal, strsignal(signal));
      if (Shell::instance)
        Shell::instance->exit_code = 2;
    }
    else {
      if (Shell::instance)
        Shell::instance->exit_code = 0;
    }
  }
  catch (...) {
    printf("Caught unknown exception while handling signal %d (%s).\n", signal, strsignal(signal));
    if (Shell::instance)
      Shell::instance->exit_code = 3;
  }
};

Shell::Shell(double frequency,
  size_t frequent_interval,
  size_t infrequent_interval)
{
  if (Shell::instance)
    throw std::logic_error("Shell is a singleton; use Get()");

  UI::Start();

  controller = std::make_unique<Controller>(frequency, frequent_interval, infrequent_interval);

  std::signal(SIGINT, signal_handler);
  std::signal(SIGABRT, signal_handler);
}

Shell::~Shell()
{
  signal_handler(0);
}

std::shared_ptr<Shell> get_shell(
  double frequency,
  size_t frequent_interval,
  size_t infrequent_interval)
{
  if (!Shell::instance)
    Shell::instance = std::make_shared<Shell>(frequency, frequent_interval, infrequent_interval);
  return Shell::instance;
}
