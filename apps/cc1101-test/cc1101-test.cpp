#include <vector>
#include <memory>

#include <cc1101.h>
#include <cc1101_rt.h>
#include <gpio_watcher.h>
#include <sleep.h>

static volatile int rx_cnt = 0;
static std::mutex mtx;

static bool fRX(std::shared_ptr<CC1101> cc1101)
{
  const std::lock_guard<std::mutex> lock(mtx);

  std::vector<uint8_t> packet;
  cc1101->Receive(packet);
  rx_cnt++;

  if (packet.size() > 2) {
    double rssi = cc1101->rRSSI(packet.back());
    packet.pop_back();
    double lqi = cc1101->rLQI(packet.back());
    packet.pop_back();

    printf("RX %03u rssi=%4.0fdBm lqi=%3.0f%%: %s\n", rx_cnt, rssi, lqi, bytes_to_hex(packet).c_str());
  }

  if (cc1101->GetState() != CC1101::State::RX)
  {
    printf("RX failed, restarting.\n");
    cc1101->StrobeFor(CC1101::SRX, CC1101::State::RX, 10);
  }

  return true;
}

static void fTX(std::shared_ptr<CC1101> cc1101, const std::string &args)
{
  const std::lock_guard<std::mutex> lock(mtx);

  auto argbytes = from_hex(args);

  if (argbytes.empty()) {
    printf("argument is not in hex format\n");
    return;
  }

  if (argbytes.size() < 4) {
    printf("argument too short\n");
    return;
  }

  // printf("TX: %s\n", bytes_to_hex(argbytes).c_str());
  cc1101->Transmit(argbytes);
  cc1101->Strobe(CC1101::CommandStrobe::SRX);
}

int main()
{
  try {
    auto cc1101 = std::make_shared<CC1101>(0, 0, "cc1101.cfg");

    std::vector<GPIOWatcher<CC1101>*> gpio_watchers;
    gpio_watchers.push_back(new GPIOWatcher<CC1101>("/dev/gpiochip0", 25, "WLMCD-CC1101", cc1101, true,
      [](int, unsigned, const timespec*, std::shared_ptr<CC1101> cc1101) {
        return fRX(cc1101);
      }));

    printf("part number: %02x\n", cc1101->RT->PARTNUM());
    printf("part version: %02x\n", cc1101->RT->VERSION());
    printf("frequency: %.2f MHz\n", cc1101->rFrequency() / 1e6);
    printf("data rate: %.2f kbps\n", cc1101->rDataRate() / 1e3);
    printf("state: %s\n", CC1101::StateName(cc1101->GetState()).c_str());
    // cc1101->Write(std::cout);

    while (true) {
      sleep_ms(1000);
      // fTX(cc1101, "5565d65daeeeeeee6eeaeeda6ee29ed265a652229e21a6ee2e10");
      //fTX(cc1101, "0001020304050607080900010203040506070809000102030405060708090001");
    }
  }
  catch (std::exception &ex) {
    printf("caught exception: %s\n", ex.what());
    return 1;
  }
  catch (...) {
    printf("caught unknown exception\n");
    return 1;
  }

  return 0;
}