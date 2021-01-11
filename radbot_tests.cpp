// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <cstring>
#include <iostream>
#include <vector>

#include "radbot.h"
#include "radbot_tests.h"

static std::vector<const char *> vectors = {
  "3ecf24d2b2a0fd20d1911a5b0062cc8ea6e5321e209e371de1d39b75d69ba5e3590c894288d7b5d309d5260250a28b482a2a9c33f2ce7893b520f7b7aec4803fddbbffeffefbdffffff7ffefffcfbfbeeff7fff7ffffffdffefffbffbdffff76ffffff9fdfffffed3dffffffe7fff7ffe7fbf3bffbffffff7fdffffff7ffffff",
  "3ecf54d2b2a0fd20dce5932b16b3ee9483740885097a803ca23b135100c223442ed934bdb4f7791409d5260250d518e3ffab435c4535ea7fd8dc28a75cf28033fffffbe6ffbffffffbdffbbfffdffefe7afdedffffdeffdfff7fff7ddf7ff7ffbfff7ffffff7faffffffbffffffffdfff3ff7bfaaedff7ffffdfffeffffffcaf",
};

int radbot_tests(int argc, const char **argv) {
  int r = 0;
  static Radbot::Decoder decoder("id", "key");

  if (argc != 1) {
    vectors.clear();
    for (int i = 1; i < argc; i++)
      vectors.push_back(argv[i]);
  }

  for (auto& str : vectors) {
    std::vector<uint8_t> bytes = hex_string_to_bytes(str);
    try {
      const std::string &msg = decoder.Decode(bytes);
      std::cout << "msg   : " << msg << std::endl;
    } catch (const std::runtime_error &err) {
      std::cout << "Failed: " << str << " " << std::endl;
      r = 1;
    }
  }

  return r;
}
