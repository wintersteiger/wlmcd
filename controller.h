// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _CONTROLLER_H_
#define _CONTROLLER_H_

#include <vector>
#include <map>
#include <functional>
#include <set>
#include <thread>
#include <mutex>

#include "device.h"

class Decoder;
class Encoder;
class UI;
class UpdateThread;

class Controller {
public:
  size_t frequent_interval, infrequent_interval;
  double frequency;

  Controller(
    double frequency = 1 /* Hz */,
    size_t frequent_interval=17,
    size_t infrequent_interval=71);
  virtual ~Controller();

  void AddSystem(std::shared_ptr<UI> ui,
                 std::shared_ptr<Decoder> decoder = nullptr,
                 std::shared_ptr<Encoder> encoder = nullptr);
  bool SelectSystem(size_t inx);
  void Run();
  void Stop();

  bool Running() const { return running; }

  void AddBackgroundDevice(std::shared_ptr<DeviceBase> device);

  void AddCommand(const std::string &verb, std::function<void(const std::string&)> f);

  void Update(bool full);

protected:
  bool running;
  size_t cur_frequent_interval, cur_infrequent_interval;
  double cur_frequency;
  size_t ui_inx, decoder_inx, encoder_inx;
  std::vector<std::shared_ptr<UI>> uis;
  std::vector<std::shared_ptr<Decoder>> decoders;
  std::vector<std::shared_ptr<Encoder>> encoders;
  std::set<std::shared_ptr<DeviceBase>> background_devices;
  std::string last_search;

  typedef std::function<void(Controller*,
                             std::shared_ptr<UI>,
                             std::shared_ptr<Decoder>,
                             std::shared_ptr<Encoder>)> key_fun_t;
  std::map<int, key_fun_t> key_bindings;

  std::map<const std::string, std::function<void(const std::string&)>> commands;

  void PauseTimer();
  void ResumeTimer();

  std::mutex threads_mtx;
  std::set<UpdateThread*> threads;

  void ThreadCleanup();
};

void Test(Controller *ctrl, UI *ui, Decoder *dec, Encoder *enc);

#endif // #ifndef _CONTROLLER_H_
