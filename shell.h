// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _SHELL_H_
#define _SHELL_H_

#include <memory>

#include "controller.h"

class Shell {
public:
  static std::shared_ptr<Shell> Get() { return instance; }
  virtual ~Shell();

  std::unique_ptr<Controller> controller = nullptr;
  int exit_code = 0;

//private:
  static std::shared_ptr<Shell> instance;
  Shell(double frequency = 1 /* Hz */,
        size_t frequent_interval=17,
        size_t infrequent_interval=71);
};

std::shared_ptr<Shell> get_shell(
  double frequency = 1 /* Hz */,
  size_t frequent_interval=17,
  size_t infrequent_interval=71);

#endif // _SHELL_H_