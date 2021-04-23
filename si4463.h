// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _SI4463_H_
#define _SI4463_H_

#include <mutex>

#include "device.h"
#include "register.h"

class SI4463 : public Device<uint16_t, uint8_t>
{
public:
  class RegisterTableSet;
  RegisterTableSet &RTS;

  typedef enum {
    POWER_UP = 0x02, // Command to power-up the device and select the operational mode and functionality.
    // COMMON_COMMANDS
    NOP = 0x00, // No Operation command.
    PART_INFO = 0x01, // Reports basic information about the device.
    FUNC_INFO = 0x10, // Returns the Function revision information of the device.
    SET_PROPERTY  = 0x11, // Sets the value of one or more properties.
    GET_PROPERTY = 0x12, // Retrieves the value of one or more properties
    GPIO_PIN_CFG = 0x13, // Configures the GPIO pins.
    FIFO_INFO = 0x15, // Access the current byte counts in the TX and RX FIFOs, and provide for resetting the FIFOs.
    GET_INT_STATUS = 0x20, // Returns the interrupt status of ALL the possible interrupt events.
    REQUEST_DEVICE_STATE = 0x33, // Request current device state and channel.
    CHANGE_STATE = 0x34, // Manually switch the chip to a desired operating state.
    READ_CMD_BUFF = 0x44, // Used to read CTS and the command response.
    FRR_A_READ = 0x50, // Reads the fast response registers (FRR) starting with FRR_A.
    FRR_B_READ = 0x51, // Reads the fast response registers (FRR) starting with FRR_B.
    FRR_C_READ = 0x53, // Reads the fast response registers (FRR) starting with FRR_C.
    FRR_D_READ =0x57, // Reads the fast response registers (FRR) starting with FRR_D.
    // IR_CAL_COMMANDS
    IRCAL = 0x17, // Image rejection calibration.
    IRCAL_MANUAL = 0x1a, // Image rejection calibration.
    // TX_COMMANDS
    START_TX = 0x31, // Switches to TX state and starts transmission of a packet.
    TX_HOP = 0x37, // Hop to a new frequency while in TX.
    WRITE_TX_FIFO = 0x66, // Writes data byte(s) to the TX FIFO.
    // RX_COMMANDS
    PACKET_INFO = 0x16, // Returns information about the length of the variable field in the last packet received, and (optionally) overrides field length.
    GET_MODEM_STATUS = 0x22, // Returns the interrupt status of the Modem Interrupt Group.
    START_RX = 0x32, // Switches to RX state and starts reception of a packet.
    RX_HOP = 0x36, // Manually hop to a new frequency while in RX mode.
    READ_RX_FIFO = 0x77, // Reads data byte(s) from the RX FIFO.
    // ADVANCED_COMMANDS
    GET_ADC_READING = 0x14, // Performs conversions using the Auxiliary ADC and returns the results of those conversions.
    GET_PH_STATUS = 0x21, // Returns the interrupt status of the Packet Handler Interrupt Group (both STATUS and PENDING).
    GET_CHIP_STATUS = 0x23, // Returns the interrupt status of the Chip Interrupt Group (both STATUS and PENDING).
  } Command;

public:
  SI4463(int ceSPI = 0, int nSEL = 10, int nIRQ = 6, int SDN=5, uint32_t XO_FREQ = 30000000, std::vector<Decoder*> decoders = {});
  virtual ~SI4463();

  virtual const char* Name() const { return "SI4463"; }

  void UpdateFrequent();
  void UpdateInfrequent();

  using Device::Write;
  using Device::Read;

  virtual void Write(std::ostream &os);
  virtual void Read(std::istream &is);

  virtual uint8_t Read(const uint16_t &addr);
  virtual std::vector<uint8_t> Read(const uint16_t &addr, size_t length);
  virtual std::vector<uint8_t> Read(const Register<uint16_t, uint8_t> &r, size_t length) { return Device::Read(r, length); }

  virtual void Write(const uint16_t &addr, const uint8_t &value);
  virtual void Write(const uint16_t &addr, const std::vector<uint8_t> &values);
  virtual void Write(const Register<uint16_t, uint8_t> &r, const uint8_t &v) { Device::Write(r, v); }

  void Reset();
  uint8_t Strobe(const Command &cmd, size_t delay_us = 1);

  void PowerUp();
  void Nop();
  std::vector<uint8_t> PartInfo();
  std::vector<uint8_t> FuncInfo();
  void SetProperty(uint8_t group, uint8_t num_props, uint8_t start_prop, const std::vector<uint8_t> &values);
  std::vector<uint8_t> GetProperty(uint8_t group, uint8_t num_props, uint8_t start_prop);
  std::vector<uint8_t> GPIOPinCfg(uint8_t gpio[4], uint8_t nirq, uint8_t sdo, uint8_t gen_config);
  std::vector<uint8_t> FIFOInfo(uint8_t fifo);
  std::vector<uint8_t> GetIntStatus(uint8_t PHClrPend, uint8_t modemClrPend, uint8_t chipClrPend);
  std::vector<uint8_t> RequestDeviceState();
  void ChangeState(uint8_t nextState1);
  std::vector<uint8_t> ReadCmdBuff();
  std::vector<uint8_t> FRRARead();
  std::vector<uint8_t> FRRBRead();
  std::vector<uint8_t> FRRCRead();
  std::vector<uint8_t> FRRDRead();

  void IRCal(uint8_t searching_step_size, uint8_t searching_RSSI_avg, uint8_t RX_chain_setting1, uint8_t RX_chain_setting2);
  std::vector<uint8_t> IRCalManual(uint8_t IRCalAmp, uint8_t IRCalPH);

  void StartTX(uint8_t channel, uint8_t condition, uint16_t tx_len, uint8_t tx_delay, uint8_t num_repeat);
  void TXHop(uint8_t channel, uint32_t frac, uint16_t vco_cnt, uint16_t pll_settle_time);
  void WriteTXFIFO(const std::vector<uint8_t> &data);

  std::vector<uint8_t> PacketInfo(uint8_t field_number, uint16_t len, uint16_t len_diff);
  std::vector<uint8_t> GetModemStatus(uint8_t modem_clr_pend);

  std::vector<uint8_t> GetADCReading(uint8_t adc_en, uint8_t adc_cfg);

protected:
  std::mutex mtx;
  int dSPI, ceSPI, nSEL, nIRQ, SDN;
  uint32_t XO_FREQ;
  std::vector<uint8_t> combuf, cmdbuf;

  void RW(std::vector<uint8_t> &values, size_t n);
};

#endif
