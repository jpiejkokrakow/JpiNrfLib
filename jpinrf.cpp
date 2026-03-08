#include <jpinrf.h>

TXPAYLOAD txpayload;
RF24 radio(CE_PIN, CSN_PIN);


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