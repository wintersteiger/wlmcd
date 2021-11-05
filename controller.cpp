// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <unistd.h>

#include <cmath>
#include <csignal>
#include <ctime>
#include <stdexcept>
#include <string>
#include <vector>
#include <memory>
#include <fstream>

#include <curses.h>

#include "sleep.h"
#include "device.h"
#include "ui.h"
#include "decoder.h"
#include "encoder.h"
#include "controller.h"

#define KEY_FUN [](auto ctrl, auto ui, auto d, auto e)

static volatile bool have_timer = false;
static volatile bool timed_out = false;
static timer_t timerid;
static sigset_t sigmask_timer, sigmask_empty;

static void timer_handler(int sig, siginfo_t *si, void *uc)
{
  timed_out = true;
}

static void timer_setup(double frequency)
{
    struct sigevent sev;
    struct sigaction sa;
    int signal = SIGRTMIN;

    sigemptyset(&sigmask_empty);
    sigemptyset(&sigmask_timer);
    sigaddset(&sigmask_timer, signal);

    // Set up handler
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = timer_handler;
    sigemptyset(&sa.sa_mask);
    if (sigaction(signal, &sa, NULL) == -1)
      throw std::runtime_error("sigaction");

    // Create timer
    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = signal;
    sev.sigev_value.sival_ptr = &timerid;
    if (timer_create(CLOCK_REALTIME, &sev, &timerid) == -1)
      throw std::runtime_error("timer_create");

    // Start timer
    double period = 1.0 / frequency;
    double seconds = 0.0;
    double fraction = modf(period, &seconds);
    struct itimerspec its;
    its.it_value.tv_sec = seconds;
    its.it_value.tv_nsec = round(fraction * 1e9);
    its.it_interval.tv_sec = its.it_value.tv_sec;
    its.it_interval.tv_nsec = its.it_value.tv_nsec;

    if (timer_settime(timerid, 0, &its, NULL) == -1)
      throw std::runtime_error("timer_settime");

    // Unblock signal delivery
    if (sigprocmask(SIG_UNBLOCK, &sigmask_timer, NULL) == -1)
      throw std::runtime_error("sigprocmask");

    have_timer = true;
}

class UpdateThread {
public:
  UpdateThread(std::shared_ptr<DeviceBase> device) : device(device), done(false) {
    auto f = [=](std::shared_ptr<DeviceBase> device) {
      try {
        device->UpdateTimed();
      }
      catch (std::exception &ex) {
        UI::Log("Exception during threaded update: %s", ex.what());
      }
      catch (...) {
        UI::Log("Unknown exception during threaded update");
      }
      done = true;
    };
    thread = std::thread(f, device);
  }
  virtual ~UpdateThread() {}
  bool Done() const { return done; }
  void Join() { if (thread.joinable()) thread.join(); }
protected:
  std::thread thread;
  std::shared_ptr<DeviceBase> device;
  bool done;
};

Controller::Controller(
  double frequency,
  size_t frequent_interval,
  size_t infrequent_interval) :
  frequent_interval(frequent_interval),
  infrequent_interval(infrequent_interval),
  frequency(frequency),
  running(false),
  cur_frequent_interval(frequent_interval),
  cur_infrequent_interval(infrequent_interval),
  cur_frequency(frequency),
  ui_inx(SIZE_MAX),
  decoder_inx(SIZE_MAX),
  encoder_inx(SIZE_MAX)
{
  if (!have_timer && cur_frequency != 0)
    timer_setup(cur_frequency);

  key_bindings['q'] = KEY_FUN {
    ctrl->Stop();
  };

  key_bindings['r'] =
  key_bindings[KEY_REFRESH] =
  key_bindings[KEY_RESIZE] = KEY_FUN {
    ctrl->cur_frequent_interval = ctrl->frequent_interval;
    ctrl->cur_infrequent_interval = ctrl->infrequent_interval;
    ctrl->cur_frequency = ctrl->frequency;
    if (ctrl->frequency != 0) {
      for (auto d : ui->Devices()) {
        d->UpdateTimed();
        d->UpdateFrequent();
        d->UpdateInfrequent();
      }
    }
    ui->Reset();
  };

  key_bindings[KEY_HOME] = KEY_FUN { ui->First(); };
  key_bindings[KEY_UP] = KEY_FUN { ui->Previous(); };
  key_bindings[KEY_DOWN] = KEY_FUN { ui->Next(); };
  key_bindings[KEY_END] = KEY_FUN { ui->Last(); };

  key_bindings[KEY_PPAGE] = KEY_FUN { ui->ScrollUp(); };
  key_bindings[KEY_NPAGE] = KEY_FUN { ui->ScrollDown(); };

  key_bindings[KEY_LEFT] = KEY_FUN { ui->Left(); };
  key_bindings[KEY_RIGHT] = KEY_FUN { ui->Right(); };

  key_bindings['\n'] =
  key_bindings[KEY_ENTER] = KEY_FUN {
    ctrl->PauseTimer();
    ui->Edit();
    ctrl->ResumeTimer();
  };

  key_bindings['d'] = KEY_FUN { ui->Describe(); };
  key_bindings['h'] = KEY_FUN { ui->Describe(); };

  key_bindings[':'] = KEY_FUN {
    // Disable timer; because wgetnstr apparently gets
    // interrupted by real-time signals.
    ctrl->PauseTimer();
    std::string cmd = ui->GetCommand();
    ctrl->ResumeTimer();

    std::string verb, args;
    if (cmd.size() > 0)
    {
      size_t fpos = cmd.find(' ');
      verb = cmd.substr(0, fpos);
      args = fpos == std::string::npos ? "" : cmd.substr(fpos + 1);

      while (std::isspace(args[0]))
        args = args.substr(1);

      if (verb == "q" || verb == "quit")
        ctrl->Stop();
      else if (verb == "w" || verb == "write") {
        try {
          std::ofstream f(args);
          auto devs = ui->Devices();
          if (devs.size() == 0)
            UI::Error("No devices");
          else if (devs.size() == 1)
            (*devs.begin())->Write(f);
          else {
            f << "{\n  \"devices\": [\n";
            for (auto it = devs.begin(); it != devs.end(); it++) {
              if (it != devs.begin())
                f << ",\n";
              (*it)->Write(f);
            }
            f << "  ]\n}\n";
          }
          f.close();
          UI::Log("Wrote configuration to %s", args.c_str());
        }
        catch (std::exception &ex) {
          UI::Error("%s", ex.what());
        }
      }
      else if (verb == "r" || verb == "read") {
        try {
          if (ui->Devices().size() > 1)
            UI::Error("Cannot read configurations of multiple devices");
          for (auto device : ui->Devices()) {
            auto is = std::ifstream(args);
            device->Read(is);
          }
          UI::Log("Read configuration from %s", args.c_str());
        }
        catch (std::exception &ex) {
          UI::Error("%s", ex.what());
        }
      }
      else if (verb == "R" || verb == "reset") {
        for (auto device : ui->Devices())
            device->Reset();
      }
      else {
        auto cit = ctrl->commands.find(verb);
        if (cit != ctrl->commands.end()) {
          cit->second(args);
        }
        else
          UI::Error("Unknown command '%s'", verb.c_str());
      }
    }
  };

  key_bindings[' '] = KEY_FUN {
    ui->Flip();
    for (auto &device : ui->Devices())
      device->UpdateInfrequent();
    ui->Update(false);
  };

  key_bindings['/'] = KEY_FUN {
    ctrl->PauseTimer();
    ctrl->last_search = ui->GetCommand("/");
    ctrl->ResumeTimer();
    ui->FindNext(ctrl->last_search);
  };

  key_bindings['?'] = KEY_FUN {
    ctrl->PauseTimer();
    ctrl->last_search = ui->GetCommand("?");
    ctrl->ResumeTimer();
    ui->FindPrev(ctrl->last_search);
  };

  key_bindings['n'] = KEY_FUN {
    ui->FindNext(ctrl->last_search);
  };

  key_bindings['p'] = KEY_FUN {
    ui->FindPrev(ctrl->last_search);
  };
}

Controller::~Controller()
{
  Stop();
}

void Controller::AddSystem(
  std::shared_ptr<UI> ui,
  std::shared_ptr<Decoder> decoder,
  std::shared_ptr<Encoder> encoder)
{
  uis.push_back(ui);
  decoders.push_back(decoder);
  encoders.push_back(encoder);
}

bool Controller::SelectSystem(size_t inx)
{
  if (inx >= uis.size())
    return false;

  ui_inx = decoder_inx = encoder_inx = inx;
  return true;
}

void Controller::ThreadCleanup()
{
  const std::lock_guard<std::mutex> lock(threads_mtx);
  try {
    if (!threads.empty()) {
      std::set<UpdateThread*> joined;
      for (UpdateThread *t : threads) {
        if (t && t->Done()) {
          t->Join();
          delete(t);
          joined.insert(t);
        }
      }
      for (UpdateThread *t : joined)
        threads.erase(t);
      if (threads.empty() && ui_inx < uis.size())
          uis[ui_inx]->Update(false);
    }
  }
  catch (std::exception &ex) {
    UI::Log("Exception during background thread cleanup: %s", ex.what());
  }
  catch (...) {
    UI::Log("Caught unknown exception during background thread cleanup");
  }
}

void Controller::Run()
{
  if (uis.size() == 0)
    throw std::runtime_error("No UI to run.");

  SelectSystem(0);
  running = true;
  uis[ui_inx]->Reset();

  for (size_t i = 0; running; i++)
  {
    try {
      UI::indicator_value = threads.size();

      int key = uis[ui_inx]->GetKey();
      if (key != ERR)
      {
        auto kit = key_bindings.find(key);
        if (kit != key_bindings.end())
          kit->second(this, uis[ui_inx], decoders[decoder_inx], encoders[encoder_inx]);
        else if (KEY_F(1) <= key && key <= KEY_F(64))
        {
          size_t inx = key - KEY_F0 - 1;
          if (inx >= uis.size())
            UI::Error("No such UI");
          else if (ui_inx != inx) {
            SelectSystem(inx);
            uis[ui_inx]->Reset();
          }
        }
      }

      if (i % cur_frequent_interval == 0)
      {
        try {
          for (auto &device : uis[ui_inx]->Devices())
            device->UpdateFrequent();
          for (auto &device : background_devices)
            device->UpdateFrequent();
          uis[ui_inx]->Update(false);
        }
        catch (std::exception &ex) {
          UI::Log("Exception: %s", ex.what());
          i = 0;
          cur_frequent_interval *= 2;
        }
      }
      else if (i % cur_infrequent_interval == 0)
      {
        try {
          for (auto &device : uis[ui_inx]->Devices())
            device->UpdateInfrequent();
          for (auto &device : background_devices)
            device->UpdateInfrequent();
          uis[ui_inx]->Update(false);
        }
        catch (std::exception &ex) {
          UI::Log("Exception: %s", ex.what());
          i = 0;
          cur_infrequent_interval *= 2;
        }
      }

      if (timed_out)
      {
        timed_out = false;
        ThreadCleanup();
        try {
          const std::lock_guard<std::mutex> lock(threads_mtx);
          for (auto &device : uis[ui_inx]->Devices()) {
            UpdateThread *t = new UpdateThread(device);
            if (t == NULL)
              throw std::runtime_error("could not spawn thread");
            threads.insert(t);
          }
          for (auto &device : background_devices) {
            UpdateThread *t = new UpdateThread(device);
            if (t == NULL)
              throw std::runtime_error("could not spawn thread");
            threads.insert(t);
          }
        }
        catch (std::exception &ex) {
          UI::Log("Exception during background thread creation: %s", ex.what());
        }
        catch (...) {
          UI::Log("Caught unknown exception during background thread creation");
        }
      }

      sleep_ms(11);
    }
    catch (std::exception &ex) {
      UI::Log("Exception: %s", ex.what());
      uis[ui_inx]->Update(false);
      sleep_ms(250);
    }
    catch (...) {
      UI::Log("Caught unknown exception.");
      uis[ui_inx]->Update(false);
      sleep_ms(250);
    }
  }

  ui_inx = SIZE_MAX;
  running = false;
  timed_out = false;
}

void Controller::Stop()
{
  if (have_timer) {
    timer_delete(timerid);
    size_t num_threads = threads.size();
    size_t last = num_threads;
    while (num_threads > 0) {
      ThreadCleanup();
      if (num_threads != last) {
        UI::Info("threads to join: %lu", num_threads);
        if (ui_inx != SIZE_MAX)
          uis[ui_inx]->Update(false);
      }
      sleep_ms(10);
      last = num_threads;
      num_threads = threads.size();
    }
  }
  running = false;
}

void Controller::AddBackgroundDevice(std::shared_ptr<DeviceBase> device)
{
  background_devices.insert(device);
}

void Controller::PauseTimer()
{
  if (sigprocmask(SIG_BLOCK, &sigmask_timer, NULL) == -1)
    throw std::runtime_error("sigprocmask");
}

void Controller::ResumeTimer()
{
  if (sigprocmask(SIG_UNBLOCK, &sigmask_timer, NULL) == -1)
    throw std::runtime_error("sigprocmask");
}

void Controller::AddCommand(const std::string &verb, std::function<void(const std::string&)> f)
{
  commands[verb] = f;
}