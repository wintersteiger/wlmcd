#ifndef _GPIO_WATCHER_H_
#define _GPIO_WATCHER_H_

#include <cstring>
#include <functional>
#include <thread>

#include <errno.h>

#include <gpiod.h>

template <typename T>
class GPIOWatcher {
public:
  GPIOWatcher(const char *chip, int offset, const char *consumer,
              std::shared_ptr<T> data, bool rising_edge,
              std::function<bool(int, unsigned, const struct timespec*, std::shared_ptr<T>&)> f) :
    data(data),
    f(f),
    running(true),
    keep_running(true),
    thread(NULL)
  {
    struct timespec timeout = { .tv_sec = 0, .tv_nsec = 250000000 };
    thread = new std::thread([this, chip, offset, consumer, timeout, rising_edge]() {
      if (gpiod_ctxless_event_monitor(chip, rising_edge ? GPIOD_CTXLESS_EVENT_RISING_EDGE :
                                                          GPIOD_CTXLESS_EVENT_FALLING_EDGE,
                                      offset, false, consumer, &timeout,
                                      NULL /* poll callback */,
                                      EventCallback,
                                      this) != 0)
      {
        throw std::runtime_error(std::string("could not start GPIO event monitor thread: ") + strerror(errno));
      }
      keep_running = running = false;
    });
  }

  virtual ~GPIOWatcher()
  {
    Stop();
    if (thread) {
      thread->join();
      delete (thread);
    }
  }

  void Stop() { keep_running = false; }

  bool IsRunning() const { return running; }

protected:
  std::shared_ptr<T> data;
  std::function<bool(int, unsigned, const struct timespec*, std::shared_ptr<T>&)> f;
  volatile bool running, keep_running;
  std::thread *thread;

  static int EventCallback(int event_type, unsigned int offset, const struct timespec *timestamp, void *data)
  {
    GPIOWatcher<T> *w = (GPIOWatcher<T>*)data;

    if (!w->keep_running)
      return GPIOD_CTXLESS_EVENT_CB_RET_STOP;

    if (event_type == GPIOD_CTXLESS_EVENT_CB_RISING_EDGE ||
        event_type == GPIOD_CTXLESS_EVENT_CB_FALLING_EDGE ) {
      bool ok = w->f(event_type, offset, timestamp, w->data);
      if (!ok) return GPIOD_CTXLESS_EVENT_CB_RET_ERR;
    }

    return GPIOD_CTXLESS_EVENT_CB_RET_OK;
  }
};

#endif // _GPIO_WATCHER_H_
