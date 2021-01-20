//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#include "herald/zephyr_context.h"
#include "herald/ble/ble_concrete.h"
#include "herald/ble/ble_database.h"
#include "herald/ble/ble_receiver.h"
#include "herald/ble/ble_sensor.h"
#include "herald/ble/ble_transmitter.h"
#include "herald/ble/bluetooth_state_manager.h"
#include "herald/data/sensor_logger.h"

// nRF Connect SDK includes
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>

// C++17 includes
#include <memory>
#include <vector>
#include <cstring>

namespace herald {
namespace ble {

using namespace herald::datatype;
using namespace herald::data;

static PayloadDataSupplier* latestPds = NULL;

class ConcreteBLETransmitter::Impl {
public:
  Impl(std::shared_ptr<Context> ctx, std::shared_ptr<BluetoothStateManager> bluetoothStateManager, 
    std::shared_ptr<PayloadDataSupplier> payloadDataSupplier, std::shared_ptr<BLEDatabase> bleDatabase);
  ~Impl();

  std::shared_ptr<ZephyrContext> m_context;
  std::shared_ptr<BluetoothStateManager> m_stateManager;
  std::shared_ptr<PayloadDataSupplier> m_pds;
  std::shared_ptr<BLEDatabase> m_db;

  std::vector<std::shared_ptr<SensorDelegate>> delegates;

  SensorLogger logger;
};

/* Herald Service Variables */
static struct bt_uuid_128 herald_uuid = BT_UUID_INIT_128(
	0x9b, 0xfd, 0x5b, 0xd6, 0x72, 0x45, 0x1e, 0x80, 0xd3, 0x42, 0x46, 0x47, 0xaf, 0x32, 0x81, 0x42
);
static struct bt_uuid_128 herald_char_signal_uuid = BT_UUID_INIT_128(
	0x11, 0x1a, 0x82, 0x80, 0x9a, 0xe0, 0x24, 0x83, 0x7a, 0x43, 0x2e, 0x09, 0x13, 0xb8, 0x17, 0xf6
);
static struct bt_uuid_128 herald_char_payload_uuid = BT_UUID_INIT_128(
	0xe7, 0x33, 0x89, 0x8f, 0xe3, 0x43, 0x21, 0xa1, 0x29, 0x48, 0x05, 0x8f, 0xf8, 0xc0, 0x98, 0x3e
);

// TODO bind the below to a class and control their values

static uint8_t vnd_value[] = { 'V', 'e', 'n', 'd', 'o', 'r' };

static ssize_t read_vnd(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			void *buf, uint16_t len, uint16_t offset)
{
	const char *value = (const char*)attr->user_data;

	return bt_gatt_attr_read(conn, attr, buf, len, offset, value,
				 strlen(value));
}

static ssize_t write_vnd(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			 const void *buf, uint16_t len, uint16_t offset,
			 uint8_t flags)
{
	uint8_t *value = (uint8_t*)attr->user_data;

	if (offset + len > sizeof(vnd_value)) {
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
	}

	memcpy(value + offset, buf, len);

	return len;
}

// static uint8_t payloadData[100]; // TODO ensure this doesn't overflow
// static uint8_t payloadSize = 0;

static ssize_t read_payload(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			void *buf, uint16_t len, uint16_t offset)
{
  const char *value = (const char*)attr->user_data;
  if (NULL != latestPds) {
    PayloadTimestamp pts; // now
    auto payload = latestPds->payload(pts,nullptr);
    if (payload.has_value()) {
      char* newvalue = new char[payload->size()];
      std::size_t i;
      for (i = 0;i < payload->size();i++) {
        newvalue[i] = (char)payload->at(i);
      }
      // newvalue[i] = '\0'; // termination string
      // newvalue[0] = 'w';
      // newvalue[1] = 't';
      // newvalue[2] = 'a';
      // newvalue[3] = 'f';
      // newvalue[4] = '\0';
      // std::strcpy(value, newvalue);
      // value = (const char*)newvalue;
      value = newvalue;
      // value = "venue value";
      return bt_gatt_attr_read(conn, attr, buf, len, offset, value,
        payload->size());
    // } else {
    //   value = "venue value"; // TODO replace with the use of PDS
    }
  }
  return bt_gatt_attr_read(conn, attr, buf, len, offset, value,
    strlen(value));
}

static ssize_t write_payload(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			 const void *buf, uint16_t len, uint16_t offset,
			 uint8_t flags)
{
	uint8_t *value = (uint8_t*)attr->user_data;

	if (offset + len > sizeof(vnd_value)) {
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
	}

	memcpy(value + offset, buf, len);

	return len;
}

// Define kernel memory statically so we definitely have it
BT_GATT_SERVICE_DEFINE(herald_svc,
  BT_GATT_PRIMARY_SERVICE(&herald_uuid),
  BT_GATT_CHARACTERISTIC(&herald_char_signal_uuid.uuid,
            BT_GATT_CHRC_WRITE,
            BT_GATT_PERM_WRITE,
            read_vnd,write_vnd, nullptr),
  BT_GATT_CHARACTERISTIC(&herald_char_payload_uuid.uuid,
            BT_GATT_CHRC_READ,
            BT_GATT_PERM_READ,
            read_payload, write_payload, nullptr)
);
static auto bp = BT_LE_ADV_CONN_NAME;
static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA_BYTES(BT_DATA_UUID16_ALL, 
					BT_UUID_16_ENCODE(BT_UUID_DIS_VAL),
					BT_UUID_16_ENCODE(BT_UUID_GATT_VAL),
					BT_UUID_16_ENCODE(BT_UUID_GAP_VAL)
	),
	BT_DATA_BYTES(BT_DATA_UUID128_ALL,
		      0x9b, 0xfd, 0x5b, 0xd6, 0x72, 0x45, 0x1e, 0x80, 
					0xd3, 0x42, 0x46, 0x47, 0xaf, 0x32, 0x81, 0x42
	),
};


ConcreteBLETransmitter::Impl::Impl(
  std::shared_ptr<Context> ctx, std::shared_ptr<BluetoothStateManager> bluetoothStateManager, 
    std::shared_ptr<PayloadDataSupplier> payloadDataSupplier, std::shared_ptr<BLEDatabase> bleDatabase)
  : m_context(std::static_pointer_cast<ZephyrContext>(ctx)), // Herald API guarantees this to be safe
    m_stateManager(bluetoothStateManager),
    m_pds(payloadDataSupplier),
    m_db(bleDatabase),
    delegates(),
    logger(ctx,"Sensor","BLE.ConcreteBLETransmitter")
{
  latestPds = m_pds.get();
}

ConcreteBLETransmitter::Impl::~Impl()
{
  latestPds = NULL;
}

// ssize_t ConcreteBLETransmitter::Impl::read_payload(struct bt_conn *conn, const struct bt_gatt_attr *attr,
// 			void *buf, uint16_t len, uint16_t offset)
// {
// 	const char *value = (const char*)attr->user_data;

//   value = "venue value";

// 	return bt_gatt_attr_read(conn, attr, buf, len, offset, value,
// 				 strlen(value));
// }

// ssize_t ConcreteBLETransmitter::Impl::write_payload(struct bt_conn *conn, const struct bt_gatt_attr *attr,
// 			 const void *buf, uint16_t len, uint16_t offset,
// 			 uint8_t flags)
// {
// 	uint8_t *value = (uint8_t*)attr->user_data;

// 	if (offset + len > sizeof(vnd_value)) {
// 		return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
// 	}

// 	memcpy(value + offset, buf, len);

// 	return len;
// }







ConcreteBLETransmitter::ConcreteBLETransmitter(
  std::shared_ptr<Context> ctx, std::shared_ptr<BluetoothStateManager> bluetoothStateManager, 
    std::shared_ptr<PayloadDataSupplier> payloadDataSupplier, std::shared_ptr<BLEDatabase> bleDatabase)
  : mImpl(std::make_shared<Impl>(ctx,bluetoothStateManager,
      payloadDataSupplier,bleDatabase))
{
  ;
}

ConcreteBLETransmitter::~ConcreteBLETransmitter()
{
  // stop(); // stops using m_addr
}

std::optional<std::shared_ptr<CoordinationProvider>>
ConcreteBLETransmitter::coordinationProvider()
{
  return {};
}

void
ConcreteBLETransmitter::add(std::shared_ptr<SensorDelegate> delegate)
{
  mImpl->delegates.push_back(delegate);
}

// 6. Implement any additional transmitter functionality, as required

void
ConcreteBLETransmitter::start()
{
  mImpl->logger.debug("ConcreteBLETransmitter::start");

  // Ensure our zephyr context has bluetooth ready
  mImpl->m_context->startBluetooth();

  // Now start advertising
  // See https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/reference/bluetooth/gap.html#group__bt__gap_1gac45d16bfe21c3c38e834c293e5ebc42b
  // bt_le_adv_param param{}; // TODO initialise parameters
  // bt_data ad{};
  // size_t ad_len;
  // bt_data sd{}; // service definition
  // size_t sd_len; // service definition length
  int success = bt_le_adv_start(herald::ble::bp, herald::ble::ad, ARRAY_SIZE(herald::ble::ad), NULL, 0);
  if (!success) {
    // TODO log
    return;
  }
  mImpl->logger.debug("ConcreteBLETransmitter::start completed successfully");
}

void
ConcreteBLETransmitter::stop()
{
  mImpl->logger.debug("ConcreteBLETransmitter::stop");
  int success = bt_le_adv_stop();
  if (!success) {
    // TODO log
    return;
  }
  // Don't stop Bluetooth altogether - this is done by the ZephyrContext->stopBluetooth() function only
  mImpl->logger.debug("ConcreteBLETransmitter::stop completed successfully");
}

}
}