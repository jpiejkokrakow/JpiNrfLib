#include <jpinrf.h>
#include <Adafruit_VL53L0X.h>


TXPAYLOAD txpayload;
RF24 radio(CE_PIN, CSN_PIN);



char *getRangeStatusString(BYTE rangeStatus) {
  static char buf[VL53L0X_MAX_STRING_LENGTH + 1];

  VL53L0X_GetRangeStatusString(rangeStatus, buf);
  buf[VL53L0X_MAX_STRING_LENGTH] = 0;
  return buf;
}



bool nrf_configure(RF24& radio) {
	
	if (!radio.begin())
		return false;

	radio.setAddressWidth(5);
    radio.setPALevel(nrf_power);
    radio.setChannel(nrf_channel);
    radio.setAutoAck(nrf_auto_ack);
    radio.setDataRate(nrf_datarate);
	if( nrf_dynamic_payload ) {
		radio.enableDynamicPayloads();
		if( nrf_ack_payload )
			radio.enableAckPayload();
		else
			radio.disableAckPayload();
	}
	else {
		radio.disableDynamicPayloads();
		radio.disableAckPayload();
	}
	return true;
}