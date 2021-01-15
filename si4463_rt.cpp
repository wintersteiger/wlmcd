// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include "si4463.h"
#include "si4463_rt.h"

SI4463::RegisterTableSet::RegisterTableSet(SI4463 &device) :
  device(device),
  PartInfo(device),
  FuncInfo(device),
  GPIO(device),
  FIFO(device),
  DeviceState(device),
  Interrupts(device),
  PacketInfo(device),
  ModemStatus(device),
  ADC(device),
  Property(device)
{
}

SI4463::RegisterTableSet::~RegisterTableSet()
{
}

void SI4463::RegisterTableSet::Refresh(bool frequent)
{
  PartInfo.Refresh(frequent);
  FuncInfo.Refresh(frequent);
  GPIO.Refresh(frequent);
  FIFO.Refresh(frequent);
  DeviceState.Refresh(frequent);
  Interrupts.Refresh(frequent);
  PacketInfo.Refresh(frequent);
  ModemStatus.Refresh(frequent);
  ADC.Refresh(frequent);
  Property.Refresh(frequent);
}

void SI4463::RegisterTableSet::PropertyRT::Refresh(bool frequent)
{
  DeviceStateBuffer = device.RequestDeviceState();
}

void SI4463::RegisterTableSet::PropertyRT::WriteFile(const std::string &filename) {}
void SI4463::RegisterTableSet::PropertyRT::ReadFile(const std::string &filename) {}
void SI4463::RegisterTableSet::PropertyRT::Write(const Register<uint16_t, uint8_t> &reg, const uint8_t &value) {}

void SI4463::RegisterTableSet::PartInfoRT::Refresh(bool frequent)
{
  buffer = device.PartInfo();
  buffer.insert(buffer.begin(), 0);
}

void SI4463::RegisterTableSet::FuncInfoRT::Refresh(bool frequent)
{
  if (!frequent) {
    auto tmp = device.FuncInfo();
    buffer.resize(tmp.size() + 1);
    for (size_t i=1; i < tmp.size(); i++)
      buffer[i] = tmp[i-1];
  }
}

void SI4463::RegisterTableSet::GPIORT::Refresh(bool frequent)
{
  if (!frequent) {
    uint8_t drive_strength = buffer.size() == 8 ? buffer[7] : 4 /* low */;
    uint8_t gpiocfg[4] = {0, 0, 0, 0};
    buffer = device.GPIOPinCfg(gpiocfg, 0, 0, drive_strength);
    buffer.insert(buffer.begin(), 0);
  }
}

void SI4463::RegisterTableSet::FIFORT::Refresh(bool frequent)
{
  if (!frequent) {
    buffer = device.FIFOInfo(0);
    buffer.insert(buffer.begin(), 0);
  }
}

void SI4463::RegisterTableSet::DeviceStateRT::Refresh(bool frequent)
{
  if (!frequent) {
    buffer = device.RequestDeviceState();
    buffer.insert(buffer.begin(), 0);
  }
}

void SI4463::RegisterTableSet::InterruptsRT::Refresh(bool frequent)
{
  if (!frequent) {
    buffer = device.GetIntStatus(0xFF, 0xFF, 0xFF);
    buffer.insert(buffer.begin(), 0);
  }
}

void SI4463::RegisterTableSet::PacketInfoRT::Refresh(bool frequent)
{
  if (!frequent) {
    buffer = device.PacketInfo(0, 0, 0);
    buffer.insert(buffer.begin(), 0);
  }
}

void SI4463::RegisterTableSet::ModemStatusRT::Refresh(bool frequent)
{
  if (!frequent) {
    buffer = device.GetModemStatus(0xFF);
    buffer.insert(buffer.begin(), 0);
  }
}

void SI4463::RegisterTableSet::ADCRT::Refresh(bool frequent)
{
  if (!frequent) {
    buffer = device.GetADCReading(0x1F, 0x00); // UDTIME/GPIO_ATT defaults
    buffer.insert(buffer.begin(), 0);
  }
}
