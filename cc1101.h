// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _CC1101_H_
#define _CC1101_H_

#include <ostream>
#include <mutex>
#include <stdexcept>
#include <functional>
#include <thread>

#include "device.h"
#include "register.h"
#include "spidev.h"
#include "gpio_watcher.h"

class CC1101 : public Device<uint8_t, uint8_t>, SPIDev
{
public:
  class RegisterTable;
  RegisterTable *RT;

  typedef enum {
    // Command Strobes (burst bit zero)
    // Note: For register addresses in the range 0x30-0x3D, the burst bit (0x40) is used to select between
    // status registers when burst bit is one, and between command strobes when burst bit is zero.
    SRES    = 0x30 | 0x00, // Reset chip.
    SFSTXON = 0x31 | 0x00, // Enable and calibrate frequency synthesizer.
    SXOFF   = 0x32 | 0x00, // Turn off crystal oscillator.
    SCAL    = 0x33 | 0x00, // Calibrate frequency synthesizer and turn it off.
    SRX     = 0x34 | 0x00, // Enable RX.
    STX     = 0x35 | 0x00, // Enable TX.
    SIDLE   = 0x36 | 0x00, // Exit RX / TX | 0x00, turn off frequency synthesizer and exit Wake-On-Radio mode if applicable.
    SAFC    = 0x37 | 0x00, // Deprecated. Perform manual frequency compensation instead.
                          // See https://e2e.ti.com/cfs-file/__key/communityserver-discussions-components-files/156/DN015_5F00_Permanent_5F00_Frequency_5F00_Offset_5F00_Compensation.pdf
    SWOR    = 0x38 | 0x00, // Start automatic RX polling sequence (Wake-on-Radio).
    SPWD    = 0x39 | 0x00, // Enter power down mode when CSn goes high.
    SFRX    = 0x3A | 0x00, // Flush the RX FIFO buffer.
    SFTX    = 0x3B | 0x00, // Flush the TX FIFO buffer.
    SWORRST = 0x3C | 0x00, // Reset real time clock to Event1 value.
    SNOP    = 0x3D         // No operation.
  } CommandStrobe;

  typedef enum {
    SLEEP            = 0x00,
    IDLE             = 0x01,
    XOFF             = 0x02,
    VCOON_MC         = 0x03,
    REGON_MC         = 0x04,
    MANCAL           = 0x05,
    VCOON            = 0x06,
    REGON            = 0x07,
    STARTCAL         = 0x08,
    BWBOOST          = 0x09,
    FS_LOCK          = 0x0A,
    IFADCON          = 0x0B,
    ENDCAL           = 0x0C,
    RX               = 0x0D,
    RX_END           = 0x0E,
    RX_RST           = 0x0F,
    TXRX_SWITCH      = 0x10,
    RXFIFO_OVERFLOW  = 0x11,
    FSTXON           = 0x12,
    TX               = 0x13,
    TX_END           = 0x14,
    RXTX_SWITCH      = 0x15,
    TXFIFO_UNDERFLOW = 0x16
  } State;

protected:
  int spi_channel;
  const double f_xosc;
  uint8_t *recv_buf;
  size_t recv_buf_sz, recv_buf_begin, recv_buf_pos;
  std::mutex mtx;

  inline size_t recv_buf_held() const {
    return recv_buf_begin <= recv_buf_pos ?
                (recv_buf_pos - recv_buf_begin) :
                (recv_buf_sz - recv_buf_begin + recv_buf_pos);
  }

public:
  class StatusByte {
    uint8_t s;
    public:
      enum SState { IDLE=0b000, RX=0b001, TX=0b010, FSTXON=0b011, CALIBRATE=0b100, SETTLING=0b101, RXFIFO_OVERFLOW=0b110, TXFIFO_UNDERFLOW=0b111 };

      StatusByte(uint8_t s) : s(s) {};
      StatusByte(const StatusByte &other) : s(other.s) {};

      bool CHIP_RDYn() const { return (s >> 7) == 0; };
      uint8_t STATE() const { return (s & 0x70) >> 4; }
      uint8_t FIFO_BYTES_AVAILABLE() const { return s & 0x0F; }

      SState State() const { uint8_t t = STATE(); return (SState)t; }

      std::string StateString() const {
        switch (State()) {
          case SState::IDLE: return "IDLE";
          case SState::RX: return "RX";
          case SState::TX: return "TX";
          case SState::FSTXON: return "FSTXON";
          case SState::CALIBRATE: return "CALIBRATE";
          case SState::SETTLING: return "SETTLING";
          case SState::RXFIFO_OVERFLOW: return "RXFIFO O/F";
          case SState::TXFIFO_UNDERFLOW: return "TXFIFO U/F";
          default: return "UNKNOWN";
        }
      }
  };

  class Config {
  protected:
    CC1101* c;
    std::vector<uint8_t> data;
    void CHK(int i, int min, int max) const { if (i < min || i > max) throw std::runtime_error("invalid index"); }
    std::vector<uint8_t> patable;

  public:
    Config() : c(NULL) {}
    Config(CC1101 *c);
    Config(const CC1101::Config &other) : c(other.c), data(other.data) { }
    ~Config() {};

    uint8_t IOCFG(int i)   const { CHK(i, 0, 2); return data[0x02-i]; }
    uint8_t FIFOTHR()      const { return data[0x03]; }
    uint8_t SYNC(int i)    const { CHK(i, 0, 1); return data[0x05-i]; }
    uint8_t PKTLEN()       const { return data[0x06]; }
    uint8_t PKTCTRL(int i) const { CHK(i, 0, 1); return data[0x08-i]; }
    uint8_t ADDR()         const { return data[0x09]; }
    uint8_t CHANNR()       const { return data[0x0A]; }
    uint8_t FSCTRL(int i)  const { CHK(i, 0, 1); return data[0x0C-i]; }
    uint8_t FREQ(int i)    const { CHK(i, 0, 2); return data[0x0F-i]; }
    uint8_t MDMCFG(int i)  const { CHK(i, 0, 4); return data[0x14-i]; }
    uint8_t DEVIATN()      const { return data[0x15]; }
    uint8_t MCSM(int i)    const { CHK(i, 0, 2); return data[0x18-i]; }
    uint8_t FOCCFG()       const { return data[0x19]; }
    uint8_t BSCFG()        const { return data[0x1A]; }
    uint8_t AGCCTRL(int i) const { CHK(i, 0, 2); return data[0x1D-i]; }
    uint8_t WOREVT(int i)  const { CHK(i, 0, 1); return data[0x1F-i]; }
    uint8_t WORCTRL()      const { return data[0x20]; }
    uint8_t FREND(int i)   const { CHK(i, 0, 1); return data[0x22-i]; }
    uint8_t FSCAL(int i)   const { CHK(i, 0, 3); return data[0x26-i]; }
    uint8_t RCCTRL(int i)  const { CHK(i, 0, 1); return data[0x28-i]; }
    uint8_t FSTEST()       const { return data[0x29]; }
    uint8_t PTEST()        const { return data[0x2A]; }
    uint8_t AGCTEST()      const { return data[0x2B]; }
    uint8_t TEST(int i)    const { CHK(i, 0, 2); return data[0x2E -i]; }
    uint8_t PATABLE(int i) const { CHK(i, 0, 7); return patable[i]; }

    uint8_t Get(const Register<uint8_t, uint8_t> &r) const { return data[r.Address()]; }
    void Set(const Register<uint8_t, uint8_t> &r, uint8_t v) { c->Write(r.Address(), v); }

    void Write();
    void Update();
  };

  CC1101(unsigned spi_bus, unsigned spi_channel, const std::string &config_file = "", double f_xosc = 26.0*1e6);
  virtual ~CC1101();

  void Reset() override;

  State GetState();

  Config GetConfig();

  StatusByte Strobe(CommandStrobe cs, size_t delay_ms = 0);
  StatusByte StrobeFor(CommandStrobe cs, State st, size_t delay_ms = 0);

  virtual const char* Name() const override { return "CC1101"; }

  using Device::Read;
  using Device::Write;

  virtual uint8_t Read(const uint8_t &addr) override;
  virtual std::vector<uint8_t> Read(const uint8_t &addr, size_t length) override;

  virtual void Write(const uint8_t &addr, const uint8_t &value) override { WriteS(addr, value); }
  virtual void Write(const uint8_t &addr, const std::vector<uint8_t> &values) override { WriteS(addr, values); }

  StatusByte WriteS(const uint8_t &addr, const uint8_t &value);
  StatusByte WriteS(Register<uint8_t, uint8_t> &reg, const uint8_t &value);
  StatusByte WriteS(const uint8_t &addr, const std::vector<uint8_t> &values);
  StatusByte WriteS(Register<uint8_t, uint8_t> &reg, const std::vector<uint8_t> &values);

  static std::string StateName(State st);

  void Setup(const std::vector<uint8_t> &config, const std::vector<uint8_t> &patable = { 0xC6, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });

  void Receive(std::vector<uint8_t> &pkt);
  void Transmit(const std::vector<uint8_t> &pkt);
  virtual void Test(const std::vector<uint8_t> &data) override;

  virtual void UpdateFrequent() override;
  virtual void UpdateInfrequent() override;

  virtual void Write(std::ostream &os) override;
  virtual void Read(std::istream &is) override;

  double F_XOSC() const { return f_xosc; }

  double rFOE() const;
  double rRSSI() const;
  static double rRSSI(uint8_t value);
  double rLQI() const;
  static double rLQI(uint8_t value);
  double rFrequency() const;
  double rDataRate() const;
  double rDeviation() const;
  double rFilterBW() const;
  double rIFFrequency() const;
  double rChannelSpacing() const;
  double rEvent0() const;
  double rRXTimeout() const;
};

#endif
