// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <cstring>

#include <openssl/evp.h>

#ifdef OTHER_CRYPTO
#include <cryptopp/hex.h>
#include <cryptopp/filters.h>
#include <cryptopp/aes.h>
#include <cryptopp/gcm.h>

#include <OTAESGCM_OTAESGCM.h>
#endif

#include <json.hpp>
using json = nlohmann::json;

#include "radbot.h"

// #define TEST_VERBOSE

static int decrypt_openssl(const uint8_t* cipher, size_t cipher_len, const uint8_t* iv, const uint8_t* key, const uint8_t *aad, size_t aad_len, const uint8_t* auth_tag, uint8_t *plain_out)
{
  EVP_CIPHER_CTX *ctx;
  if (!(ctx = EVP_CIPHER_CTX_new()) ||
      !EVP_DecryptInit_ex(ctx, EVP_aes_128_gcm(), NULL, NULL, NULL) ||
      !EVP_DecryptInit_ex(ctx, NULL, NULL, key, iv))
      return 1;

  int len = 0;
  if (!EVP_DecryptUpdate(ctx, NULL, &len, aad, aad_len))
     return 1;

  int plain_len = 0;
  if(!EVP_DecryptUpdate(ctx, plain_out, &plain_len, cipher, cipher_len) ||
     !EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, 16, (void*)auth_tag))
    return 1;

  int r = EVP_DecryptFinal_ex(ctx, plain_out + plain_len, &len);
  EVP_CIPHER_CTX_free(ctx);

  return r <= 0;
}

#ifdef OTHER_CRYPTO
static int decrypt_cryptopp(const uint8_t* cipher, size_t cipher_len, const uint8_t* iv, const uint8_t* key, const uint8_t *aad, size_t aad_len, const uint8_t* auth_tag, uint8_t *plain_out)
{
  CryptoPP::GCM<CryptoPP::AES>::Decryption d;
  d.SetKeyWithIV(key, 16, iv, 12);
  CryptoPP::AuthenticatedDecryptionFilter df(d, NULL,
    CryptoPP::AuthenticatedDecryptionFilter::MAC_AT_BEGIN, 16);
  df.ChannelPut("", (const byte*)auth_tag, 16 );
  df.ChannelPut("AAD", (const byte*)aad, aad_len);
  df.ChannelPut("", (const byte*)cipher, cipher_len);
  df.ChannelMessageEnd("");
  bool ok = df.GetLastResult();

  df.SetRetrievalChannel("");
  std::vector<uint8_t> plain;
  size_t n = (size_t)df.MaxRetrievable();
  plain.resize(n);
  if (n > 0)
    df.Get((byte*)plain.data(), n);

  memcpy(plain_out, plain.data(), n);

  return ok ? 0 : 1;
}
#endif

static int decrypt(const uint8_t* cipher, size_t cipher_len, const uint8_t* iv, const uint8_t* key, const uint8_t *aad, size_t aad_len, const uint8_t* auth_tag, uint8_t *plain_out)
{
#ifdef TEST_VERBOSE
  printf("cipher         : ");
  for (size_t i=0; i < cipher_len; i++)
    printf("%02x", cipher[i]);
  printf("\niv             : ");
  for (size_t i=0; i < 12; i++)
    printf("%02x", iv[i]);
  printf("\nkey            : ");
  for (size_t i=0; i < 16; i++)
    printf("%02x", key[i]);
  printf("\naad            : ");
  for (size_t i=0; i < aad_len; i++)
    printf("%02x", aad[i]);
  printf("\ntag            : ");
  for (size_t i=0; i < 16; i++)
    printf("%02x", auth_tag[i]);
  printf("\n");
#endif

  int r = decrypt_openssl(cipher, cipher_len, iv, key, aad, aad_len, auth_tag, plain_out);

#ifdef TEST_VERBOSE
  printf("plain openssl  : ");
  for (size_t i=0; i < cipher_len; i++)
    printf("%02x", plain_out[i]);
  printf(" (%d)\n", r);
#endif

#ifdef OTHER_CRYPTO
  uint8_t plain2[cipher_len];
  int r2 = decrypt_cryptopp(cipher, cipher_len, iv, key, aad, aad_len, auth_tag, plain2);

#ifdef TEST_VERBOSE
  printf("plain crypto++ : ");
  for (size_t i=0; i < cipher_len; i++)
    printf("%02x", plain2[i]);
  printf(" (%d)\n", r2);
#endif

  uint8_t plain3[cipher_len];
  size_t workspace_sz = OTAESGCM::OTAES128GCMGenericWithWorkspace<OTAESGCM::OTAES128E_default_t>::workspaceRequiredMax;
  uint8_t workspace[workspace_sz];
  bool r3 = OTAESGCM::fixed32BTextSize12BNonce16BTagSimpleDec_DEFAULT_WITH_LWORKSPACE(workspace, workspace_sz, key, iv, aad, aad_len, cipher, auth_tag, plain3);

#ifdef TEST_VERBOSE
  printf("plain OTAESGCM : ");
  for (size_t i=0; i < cipher_len; i++)
    printf("%02x", plain3[i]);
  printf(" (%d)\n", r3);
#endif
#endif

  return r;
}

Radbot::State::State() :
  ::State(),
  fault(false),
  low_battery(false),
  tamper_protect(false),
  have_stats(false),
  frost_risk(false),
  valve_(0.0f),
  occupancy_(OC_UNREPORTED),
  supply_voltage(std::make_pair(0, false)),
  room_temp(std::make_pair(0, false)),
  ambient_light(std::make_pair(0, false)),
  vacancy(std::make_pair(0, false)),
  valve_status(std::make_pair(0, false)),
  target_temp(std::make_pair(0, false)),
  setback_temp(std::make_pair(0, false)),
  cum_valve(std::make_pair(0, false)),
  rel_humidity(std::make_pair(0, false)),
  occupancy(std::make_pair(0, false)),
  setback_lockout(std::make_pair(0, false)),
  reset_counter(std::make_pair(0, false)),
  error_report(std::make_pair(0, false))
{}

void Radbot::State::Update(const std::vector<uint8_t> &msg)
{
  if (msg.size() < 3)
    throw std::runtime_error("incomplete message");

  float valve_ = (float)msg[0];
  uint8_t fault = msg[1] & 0x80;
  uint8_t low_battery = msg[1] & 0x40;
  uint8_t tamper_protect = msg[1] & 0x20;
  uint8_t have_stats = msg[1] & 0x10;
  uint8_t frost_risk = msg[1] & 0x02;
  State::Occupancy occupancy_ = (State::Occupancy) ((msg[1] & 0xC0) >> 2);

  try {
    std::string jstr = std::string(msg.data() + 2, msg.data() + msg.size());
    json j = json::parse(jstr);

    for (const auto& e: j.items()) {
      std::string key = e.key();
      if (key == "B|cV")
        supply_voltage = std::make_pair(e.value(), true);
      else if (key == "T|C16") {
        room_temp = std::make_pair(e.value(), true);
      } else if (key == "L") {
        ambient_light = std::make_pair(e.value(), true);
      } else if (key == "vac|h") {
        vacancy = std::make_pair(e.value(), true);
      } else if (key == "v|%") {
        valve_status = std::make_pair(e.value(), true);
      } else if (key == "tT|C") {
        target_temp = std::make_pair(e.value(), true);
      } else if (key == "tS|C") {
        setback_temp = std::make_pair(e.value(), true);
      } else if (key == "vC|%") {
        cum_valve = std::make_pair(e.value(), true);
      } else if (key == "H|%") {
        rel_humidity = std::make_pair(e.value(), true);
      } else if (key == "O") {
        occupancy = std::make_pair(e.value(), true);
      } else if (key == "gE") {
        setback_lockout = std::make_pair(e.value(), true);
      } else if (key == "R") {
        reset_counter = std::make_pair(e.value(), true);
      } else if (key == "err") {
        error_report = std::make_pair(e.value(), true);
      } else {
        throw std::runtime_error(std::string("unknown json key '") + key + "'");
      }
    }
  } catch (...) {
    throw std::runtime_error("json parsing failed");
  }
}

Radbot::Decoder::Decoder(const std::string &id, const std::string &key) : ::Decoder() {
  for (size_t i=0; i < 8; i++)
    sscanf(id.c_str() + (2*i), "%02hhx", &this->id[i]);
  for (size_t i=0; i < 16; i++)
    sscanf(key.c_str() + (2*i), "%02hhx", &this->key[i]);
}

Radbot::Decoder::~Decoder() {}

const std::string& Radbot::Decoder::Decode(std::vector<uint8_t> &bytes)
{
  // From: https://raw.githubusercontent.com/DamonHD/OpenTRV/master/standards/protocol/IoTCommsFrameFormat/SecureBasicFrame-V0.1-201601.txt
  size_t num_bytes = bytes.size();
  size_t pos = 0;

#define FAIL_IF(C,M) { if (C) { throw std::runtime_error(M); } }

  FAIL_IF(num_bytes <= 4, "not enough header bytes");

  const uint8_t* header = &bytes[0];
  const uint8_t *hdr0 = &bytes[pos++];
  const uint8_t *hdr1 = &bytes[pos++];
  const uint8_t *hdr2 = &bytes[pos++];

  size_t frame_len = *hdr0;
  uint8_t frame_type = *hdr1 & 0x7F;
  bool secure = (*hdr1 & 0x80) != 0;
  uint8_t frame_seq_num_m16 = (*hdr2 & 0xF0) >> 4;
  size_t id_len = *hdr2 & 0x0F;
  size_t header_len = 4 + id_len;

  if (frame_len >= num_bytes) {
    char buf[2048];
    sprintf(buf, "not enough frame bytes: frame_len=%d num_bytes=%d", frame_len, num_bytes);
    FAIL_IF(frame_len >= num_bytes, buf);
  }
  FAIL_IF(frame_type == 0x00 || frame_type == 0x7F || frame_type == 0xFF, "invalid frame type");
  FAIL_IF(frame_type != 0x4f, "unknown frame type");
  FAIL_IF(pos + id_len >= num_bytes, "not enough ID bytes");

  const uint8_t *leaf_id = &bytes[pos];
  pos += id_len;

  for (size_t i=0; i < id_len; i++)
    FAIL_IF(leaf_id[i] != id[i], "unknown leaf id");

  size_t body_len = bytes[pos++];
  FAIL_IF(body_len > 251, "body length too large");
  FAIL_IF(pos + body_len >= num_bytes, "not enough body bytes");
  FAIL_IF(body_len >= frame_len - 3, "body length exceeds frame length");

  const uint8_t *body = &bytes[pos];
  pos += body_len;

  size_t trailer_len = frame_len - 3 - body_len - id_len;

  FAIL_IF(trailer_len == 0, "trailer_len == 0");
  FAIL_IF(pos + trailer_len > num_bytes, "not enough trailer bytes");

  const uint8_t *trailer = &bytes[pos];
  pos += trailer_len;

  FAIL_IF(!secure &&
          (trailer[trailer_len-1] == 0x00 || trailer[trailer_len-1] == 0xFF),
          "invalid trailer end");

  char *p = &tmp[0];
  if (id_len > 0) {
    for (size_t i=0; i < id_len; i++)
      p += sprintf(p, "%02x", leaf_id[i]);
    *p++ = ' ';
  }

  if (!secure) {
    // https://github.com/opentrv/OTProtocolCC/blob/master/content/OTProtocolCC/utility/OTProtocolCC_OTProtocolCC.h
    // Use 7-bit CRC with Polynomial 0x5B (1011011, Koopman) = (x+1)(x^6 + x^5 + x^3 + x^2 + 1) = 0x37 (0110111, Normal).
    // See: http://users.ece.cmu.edu/~koopman/roses/dsn04/koopman04_crc_poly_embedded.pdf
    // Should detect all 3-bit errors in up to 7 bytes of payload,
    // see: http://users.ece.cmu.edu/~koopman/crc/0x5b.txt
    FAIL_IF(true, "insecure frames not implemented yet");
  }
  else {
    FAIL_IF(trailer_len < 23, "unexpected trailer length");
    const uint8_t *restart_cnt = &trailer[0];
    const uint8_t *msg_cnt = &trailer[3];
    const uint8_t *auth_tag = &trailer[6];
    const uint8_t *tr = &trailer[22];

    FAIL_IF(*tr != 0x80, "unknown trailer type"); // 0x80 == AES-GCM

    p += sprintf(p, "%u %u",
            restart_cnt[0] << 16 | restart_cnt[1] << 8 | restart_cnt[2],
            msg_cnt[0] << 16 | msg_cnt[1] << 8 | msg_cnt[2]);

    memcpy(iv, id, 6);
    memcpy(iv + 6, restart_cnt, 3);
    memcpy(iv + 9, msg_cnt, 3);

    FAIL_IF(frame_seq_num_m16 != (iv[11] & 0x0f), "frame sequence number mismatch");

    std::vector<uint8_t> msg(body_len, 0);
    int rd = decrypt(body, body_len, iv, key, header, header_len, auth_tag, msg.data());

    FAIL_IF(rd != 0, "decryption failed");

    size_t pad = msg[msg.size()-1];
    FAIL_IF(pad >= msg.size() - 1, "excessive padding in message");
    msg.resize(msg.size() - pad);
    msg[msg.size()-1] = '}';

#ifdef TEST_VERBOSE
    printf("pddng : %lu\n", pad);

    printf("valve : %d\n", message[0]);
    printf("flags : %02x\n", message[1]);

    printf("ascii : ");
    for (size_t i=2; i < msg.size(); i++)
      printf("%c", msg[i]);
    printf("\n");

    printf("trailer: ");
    for (size_t i=0; i < trailer_len; i++) {
      if (i == 23) printf("|");
      printf("%02x", trailer[i]);
    }
    printf("\n");
#endif

    p += sprintf(p, " valve=%d", msg[0]);
    p += sprintf(p, " flags=%02x", msg[1]);
    p += sprintf(p, " message=");
    for (size_t i=2; i < msg.size(); i++)
      p += sprintf(p, "%c", msg[i]);

    state.Update(msg);
  }

  if (pos < frame_len+1) {
    p += sprintf(p, " R");
    do {
      FAIL_IF(pos >= num_bytes, "unexpected end of frame");
      p += sprintf(p, "%02x", bytes[pos++]);
    }
    while (pos < frame_len+1);
  }

  tmp_str = std::string(tmp, p - tmp);
  return tmp_str;
}

Radbot::Encoder::Encoder(const std::string &id, const std::string &key) : ::Encoder() {
  for (size_t i=0; i < 8; i++)
    sscanf(id.c_str() + (2*i), "%02hhx", &this->id[i]);
  for (size_t i=0; i < 16; i++)
    sscanf(key.c_str() + (2*i), "%02hhx", &this->key[i]);
}

Radbot::Encoder::~Encoder() {}

std::vector<uint8_t> Radbot::Encoder::Encode(const std::vector<uint8_t> &data)
{
  std::string pkt = "3ecf94d2b2a0fd20f960db0a57b708317c5d054cb2d1d5a92e78cf847f175e44c9169c2a04f0e5ef09d526057d59789bcd352625be614b1b5e06a9798a718001740449b978be5a729dfe9b7727a8f986d2b4d05fb5fbe92e4b85570fcb1f5f7bb9432ebf682b731b02f05303ac37a35e2e5ceffbba737a84f5cfabc1c4de882f";
  size_t len = pkt.length()/2;
  std::vector<uint8_t> res(len, 0);
  for (size_t i=0; i < len; i++)
    sscanf(pkt.c_str() + (2*i), "%02hhx", &res.data()[i]);
  return res;
}
