// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <cstring>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <cinttypes>
#include <fstream>

#include <serialization.h>

#include "enocean_frame.h"
#include "enocean_codec.h"
#include "enocean_tests.h"

static std::vector<const char *> vectors = {
  "00ab2ed7773776d76d776d37714f69337729480000000000004688215004",
  "aacbb5ddcb4dbb5b2ddb4ddc53da4cddc4220000155976bbb969b76b65bb69bb8a7b499bb8844000",
  "5e590840d0ac443d6eb8020c2a42b2ed7772d36d56d776d37714f6933772aa800000000000000000000000200000000000000000000000000565daeee5a6daadaeeda6ee29ed266ee555000000000000000000000000000000000000000000000000",
  "559769bbb5baaa9bbbb69bb8a7b499bb8ab40000000000000000000000000000000000000000000000003acbb4dddadd554ddddb4ddc53da4cddc55a00000000000000000000000000000000000000000000",
  "002640200443ff2ed7da3d35ddb4ddc53da4cddd352000000000000000000000000000000000000000000000000000000000facbb5ddcddda3d35ddb4ddc53da4cddd35200000000000000000000000000000000000000000000000000000000010000082000",
  "5565d65dada2a5ed5eeaeeda6ee29ed265a652229e21a6eed650"
};

static std::vector<const char*> roundtrip_vectors = {
  "a6a52b5434080580cc3aaabbccdd8032"
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
      auto frames = decoder.get_frames(bytes);
      std::cout << frames.size() << ":";
      for (auto &f : frames)
        std::cout << " " << f->describe();
      std::cout << std::endl;
    } catch (const std::runtime_error &err) {
      std::cout << "Failed: " << str << " (" << err.what() << ")" << std::endl;
      r = 1;
    }
  }

  EnOcean::Encoder encoder;

  for (auto& str : roundtrip_vectors)
  {
    try {
      EnOcean::Frame f1(from_hex(str));
      auto enc = encoder.Encode(f1);
      auto frames = decoder.get_frames(enc);
      std::cout << frames.size() << ":";
      for (auto &f : frames)
        std::cout << " " << f->describe();
      std::cout << std::endl;
      if (frames.size() != 1)
        throw std::logic_error("incorrect number of frames");
      auto rstr = to_hex(*frames[0]);
      if (str != rstr)
        throw std::logic_error(std::string(" != ") + rstr);
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

int enocean_tests(int argc, const char **argv)
{
  int r = 0;
  if ((r = decoder_tests(argc, argv)) != 0) return r;
  if ((r = encoder_tests()) != 0) return r;
  return r;
}

int main(int argc, const char **argv)
{
  return enocean_tests(argc, argv);
}