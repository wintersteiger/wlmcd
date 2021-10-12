// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <cstring>
#include <iostream>
#include <vector>
#include <cinttypes>
#include <fstream>

#include "enocean.h"
#include "enocean_tests.h"

static std::vector<const char *> vectors = {
  "00ab2ed7773776d76d776d37714f69337729480000000000004688215004",
  "aacbb5ddcb4dbb5b2ddb4ddc53da4cddc4220000155976bbb969b76b65bb69bb8a7b499bb8844000",
};

static int decoder_tests(int argc, const char **argv) {
  int r = 0;
  static EnOcean::Decoder decoder;

  if (argc != 1) {
   vectors.clear();
    for (int i = 1; i < argc; i++)
      vectors.push_back(argv[i]);
  }

  for (auto& str : vectors) {
    std::vector<uint8_t> bytes = hex_string_to_bytes(str);
    try {
      const std::string &msg = decoder.Decode(bytes);
      std::cout << msg << std::endl;
    } catch (const std::runtime_error &err) {
      std::cout << "Failed: " << str << " (" << err.what() << ")" << std::endl;
      r = 1;
    }
  }

  return r;
}

static int encoder_tests()
{
  return 0;
}

int enocean_tests(int argc, const char **argv) {
  int r = 0;
  if ((r = decoder_tests(argc, argv)) != 0) return r;
  if ((r = encoder_tests()) != 0) return r;
  return r;
}
