// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

// #include <bluetooth/bluetooth.h>
// #include <bluetooth/hci.h>
// #include <bluetooth/hci_lib.h>
// #include <bluetooth/sdp.h>

#include <core/dbus/bus.h>

#include <core/dbus/asio/executor.h>
#include <core/dbus/interfaces/properties.h>
#include <core/dbus/types/stl/tuple.h>
#include <core/dbus/types/stl/vector.h>
#include <core/dbus/types/struct.h>

namespace dbus = core::dbus;

#include "ble100.h"

BLE100::BLE100() :
  Device<uint8_t, uint8_t>()
{
  responsive = false;
}

BLE100::~BLE100()
{
}