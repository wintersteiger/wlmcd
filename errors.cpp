// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <errno.h>
#include <cstring>
#include <stdexcept>

#include "errors.h"

void throw_errno(const char* msg)
{
  char exmsg[1024];
  snprintf(exmsg, sizeof(exmsg), "%s (%s (%d))", msg, strerror(errno), errno);
  throw std::runtime_error(exmsg);
}