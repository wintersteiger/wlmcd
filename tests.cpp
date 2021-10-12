// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include "evohome_tests.h"
#include "radbot_tests.h"
#include "enocean_tests.h"

int main(int argc, const char **argv)
{
  int r = 0;

  // if (evohome_tests(argc, argv))
  //   r = 1;
  // if (radbot_tests(argc, argv))
  //   r = 1;
  if (enocean_tests(argc, argv))
    r = 1;

  return r;
}
