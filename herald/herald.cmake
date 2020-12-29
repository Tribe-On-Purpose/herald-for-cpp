
set(HERALD_HEADERS 
	${HERALD_BASE}/include/herald.h
	${HERALD_BASE}/include/context.h
	${HERALD_BASE}/include/device.h
  ${HERALD_BASE}/include/default_sensor_delegate.h
  ${HERALD_BASE}/include/sensor_array.h 
  ${HERALD_BASE}/include/sensor_delegate.h
  ${HERALD_BASE}/include/sensor.h
  ${HERALD_BASE}/include/ble/ble_concrete.h
  ${HERALD_BASE}/include/ble/ble_database_delegate.h
  ${HERALD_BASE}/include/ble/ble_database.h
  ${HERALD_BASE}/include/ble/ble_device_delegate.h
  ${HERALD_BASE}/include/ble/ble_device.h
  ${HERALD_BASE}/include/ble/ble_receiver.h
  ${HERALD_BASE}/include/ble/ble_sensor.h
  ${HERALD_BASE}/include/ble/ble_sensor_configuration.h
  ${HERALD_BASE}/include/ble/ble_transmitter.h
  ${HERALD_BASE}/include/ble/ble_tx_power.h
  ${HERALD_BASE}/include/ble/bluetooth_state_manager.h
  ${HERALD_BASE}/include/ble/bluetooth_state_manager_delegate.h
  ${HERALD_BASE}/include/data/sensor_logger.h
  ${HERALD_BASE}/include/datatype/base64_string.h 
  ${HERALD_BASE}/include/datatype/bluetooth_state.h
  ${HERALD_BASE}/include/datatype/data.h
  ${HERALD_BASE}/include/datatype/date.h
  ${HERALD_BASE}/include/datatype/encounter.h
  ${HERALD_BASE}/include/datatype/error_code.h
  ${HERALD_BASE}/include/datatype/immediate_send_data.h
  ${HERALD_BASE}/include/datatype/location_reference.h
  ${HERALD_BASE}/include/datatype/location.h
  ${HERALD_BASE}/include/datatype/payload_data.h
  ${HERALD_BASE}/include/datatype/payload_sharing_data.h
  ${HERALD_BASE}/include/datatype/payload_timestamp.h
  ${HERALD_BASE}/include/datatype/placename_location_reference.h
  ${HERALD_BASE}/include/datatype/proximity.h
  ${HERALD_BASE}/include/datatype/pseudo_device_address.h
  ${HERALD_BASE}/include/datatype/randomness.h
  ${HERALD_BASE}/include/datatype/rssi.h
  ${HERALD_BASE}/include/datatype/sensor_state.h
  ${HERALD_BASE}/include/datatype/sensor_type.h
  ${HERALD_BASE}/include/datatype/signal_characteristic_data.h
  ${HERALD_BASE}/include/datatype/target_identifier.h
  ${HERALD_BASE}/include/datatype/time_interval.h
  ${HERALD_BASE}/include/datatype/wgs84.h
  ${HERALD_BASE}/include/payload/payload_data_supplier.h
  ${HERALD_BASE}/include/payload/beacon/beacon_payload_data_supplier.h
  ${HERALD_BASE}/include/payload/fixed/fixed_payload_data_supplier.h
  ${HERALD_BASE}/include/payload/extended/extended_data.h

)
set(HERALD_HEADERS_ZEPHYR 
  ${HERALD_BASE}/include/zephyr_context.h
  ${HERALD_BASE}/include/ble/ble_concrete_zephyr.h
)
set(HERALD_SOURCES
  ${HERALD_BASE}/src/data/sensor_logger.cpp
	${HERALD_BASE}/src/datatype/base64_string.cpp
	${HERALD_BASE}/src/datatype/data.cpp
	${HERALD_BASE}/src/datatype/date.cpp
	${HERALD_BASE}/src/datatype/encounter.cpp
	${HERALD_BASE}/src/datatype/immediate_send_data.cpp
	${HERALD_BASE}/src/datatype/location.cpp
	${HERALD_BASE}/src/datatype/payload_data.cpp
	${HERALD_BASE}/src/datatype/pseudo_device_address.cpp
	${HERALD_BASE}/src/datatype/rssi.cpp
	${HERALD_BASE}/src/datatype/signal_characteristic_data.cpp
	${HERALD_BASE}/src/datatype/target_identifier.cpp
	${HERALD_BASE}/src/datatype/time_interval.cpp
	${HERALD_BASE}/src/datatype/uuid.cpp
	${HERALD_BASE}/src/payload/beacon/beacon_payload_data_supplier.cpp
	${HERALD_BASE}/src/payload/fixed/fixed_payload_data_supplier.cpp
	${HERALD_BASE}/src/payload/extended/extended_data.cpp
  ${HERALD_BASE}/src/default_sensor_delegate.cpp
  ${HERALD_BASE}/src/context.cpp
  ${HERALD_BASE}/src/sensor_array.cpp
)
set(HERALD_SOURCES_ZEPHYR
  ${HERALD_BASE}/src/ble/zephyr/concrete_ble_transmitter_nrf.cpp
  ${HERALD_BASE}/src/zephyr_context.cpp
)
