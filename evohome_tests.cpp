// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <cstring>
#include <iostream>
#include <vector>
#include <cinttypes>
#include <fstream>

#include "evohome.h"
#include "evohome_tests.h"

static std::vector<const char *> vectors = {
  "599555954acb352ccb4cd332d52b32d4b2ad2d2ad4b2ad2b2acad52cb352cad565557f",
  "599555954acb352ccb4cd332d52b32d4b2ad2d2ad4b2ad2b54cd55555555553565",
  "b32aab2a95966a599699a665aa56559565595a5599655956559569699aacaaa9a8ad",
  "599555954acb352ccb4cd332d52b2acab2acad2accb2acab2acab4b4cd565557",
  "2ccaaacaa5659a9665a669996a95956559565695665956559566a95a5aab2aab40200a0121000000004000181000000100a00020802000020000401040040000040002404141425000008001180918001040001000a0402820000800010008000040040095810000002002180200000000210000000100408000000102000000",
  "2ccaaacaa5659a9665a669996a95956559565695665956559566a95a5aab2aaa00c102810040000800028022000000000044010040100000a080008004200280a20021084000010020000244000501000000000000000010402802000260c000800242088020009200400204002040a018940020002006420000021020006030",
  "2ccaaacaa5659a9665a669996a9595655956569566595655a566595a56ab2aaa100000000080000000000000000010100000000000000803820040000000000080000100001000080000000042000002110020000020000800802801000040040000400020000114000080c80000144008122100820800000101000050050000",
  "2ccaaacaa5659a9665a669996a95a5655a9655a56559569596a5695695a96a996a9695695956aa96a6695695a5a6996aa695695956aa96a5a9569595695aa655ab2aaa0088000002070041000800020000000502080140000000000008000000800820120801400018128004000048000000820000000810800600c1040c0000",
  "2ccaaacaa5659a9665a669996a95a5655a9655a56559569596a5695695a96a996a9695695956aa96a6695695a5a6996aa695695956aa96a5a9569595695aa655ab2aab011201000000000000180000c00000000000180000008200000400003460222094020040040001000400040000001800080002c0000000008002700000",
  "2ccaaacaa5659a9665a669996a95a5655a9655a56559569596a5695695a96a996a9695695956aa96a6695695a5a6996aa695695956aa96a5a9569595695aa655ab2aab0400000008400000000002000000048080000100000120000000400000000001008000000429000620240000400000040010080800004020c010000042",
  "2ccaaacaa5659a9665a669996a95a5655a9655a56559569596a5695695a96a996a9695695956aa96a6695695a5a6996aa695695956aa96a5a9569595695aa655ab2aaa0080400002005040000000000180000000000250001200200804000010800001100000180000c004060004040000041000008020000200401120000001",
  "2ccaaacaa5659a9665a669996a95a5655a9655a56559569596a5695695a96a996a9695695956aa96a6695695a5a6996aa695695956aa96a5a9569595695aa655ab2aaa210800100006000080200000010002000820080200020404060010018c00c3002042000400000014040300020180004000400818000401210000000000",
  "2ccaaacaa5656a96699965995665a96699965995665a966aaaa55956a59565596a56aaaaaa9656ab2aaa80840010000000000000000080400000028000c00010004000000010040000000802000002000000080000180000b00000c0401002013400000400600406200000000081000002640820000008000046000008040000",
  "2ccaaacaa5659a9665a669996a9595655956569566595655a566695a55ab2aab0000000008100008001040041800008018001004008000000000000002010000800000108000000200000000400000426040c80008000000000002040400204000221000404000000600000000010844080800000300a0020010000010011100",
  "2ccaaacaa5656a96699965995665a96699965995665a966aaaa55956a59565595655aaaaa95655ab2aaa88400010000000000000008000100401000810000160000000000000101000800000542444080064004000c000080000230040008008000000000200040c200000008822440002600000000181002040820000409400",
};

int decoder_tests(int argc, const char **argv) {
  int r = 0;
  static Evohome::Decoder decoder;

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

int encoder_tests()
{
  Evohome::Encoder encoder;
  Evohome::Decoder decoder;

  const char *fname = "test.tx.log";
  std::ifstream is(fname, std::ifstream::in);
  std::string line;
  while (std::getline(is, line))
  {
    std::vector<uint8_t> pkt = hex_string_to_bytes(line.c_str());

    printf("* PKT    : ");
    for (unsigned i = 0; i < pkt.size(); i++)
      printf("%02x", pkt[i]);
    printf("\n");

    std::vector<uint8_t> encoded = encoder.Encode(pkt);

    printf("* ENCODED: ");
    for (unsigned i = 0; i < encoded.size(); i++)
      printf("%02x", encoded[i]);
    printf("\n");

    std::string decoded = decoder.Decode(encoded);

    printf("* DECODED: %s\n", decoded.c_str());
    // for (unsigned i = 0; i < decoded.size(); i++)
    //   printf("%02x", decoded[i]);
    printf("\n");
  }

  is.close();

  return 0;
}

int evohome_tests(int argc, const char **argv) {
  int r = 0;
  if ((r = decoder_tests(argc, argv)) != 0) return r;
  if ((r = encoder_tests()) != 0) return r;
  return r;
}
