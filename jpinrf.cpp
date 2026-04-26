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


char *bootReasonStrings[] {
	"UNKNOWN",
    "POWERON",    //!< Reset due to power-on event
    "EXT",        //!< Reset by external pin (not applicable for ESP32)
    "SW",         //!< Software reset via esp_restart
    "PANIC",      //!< Software reset due to exception/panic
    "INT_WDT",    //!< Reset (software or hardware) due to interrupt watchdog
    "TASK_WDT",   //!< Reset due to task watchdog
    "WDT",        //!< Reset due to other watchdogs
    "DEEPSLEEP",  //!< Reset after exiting deep sleep mode
    "BROWNOUT",   //!< Brownout reset (software or hardware)
    "SDIO",       //!< Reset over SDIO
	
};

char *getBootReasonString(esp_reset_reason_t reason) {
	if(reason > ESP_RST_SDIO) 
		reason = ESP_RST_UNKNOWN;
	return bootReasonStrings[reason];
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

void task_func_stub(void *pvParams) {
	task_desc_t *pt = (task_desc_t*)pvParams;
	pt->task_func(pt);
}

void SetupQueuesAndTasks(queue_desc_t *queues, task_desc_t *tasks) {

  Serial.println("/****************");
  Serial.print(F("sizeof(PLDHEADER) = ")); Serial.println(sizeof(PLDHEADER));
  Serial.print(F("sizeof(LOX_MEASURE) = ")); Serial.println(sizeof(LOX_MEASURE));
  Serial.print(F("sizeof(LOXPAYLOAD) = ")); Serial.println(sizeof(LOXPAYLOAD));
  Serial.print(F("sizeof(TXPAYLOAD) = ")); Serial.println(sizeof(TXPAYLOAD));
  Serial.print(F("sizeof(PRTPAYLOAD) = ")); Serial.println(sizeof(PRTPAYLOAD));
  Serial.print(F("sizeof(ACKPAYLOAD) = ")); Serial.println(sizeof(ACKPAYLOAD));
  Serial.print(F("sizeof(DRVPAYLOAD) = ")); Serial.println(sizeof(DRVPAYLOAD));
  Serial.println("****************/");

  for(queue_desc_t *pqd = queues; pqd->name != NULL; pqd++) {
    QueueHandle_t qh = xQueueCreate(pqd->qu_size, pqd->el_size); 
    if(qh == NULL) {
      Serial.print(F("!!! Failed to create queue: "));
      Serial.println(pqd->name);
      STOP;
    }
    pqd->handle = qh;
  }
  for (task_desc_t *pt = tasks; pt->task_func != NULL; pt++) {
 	  if( (pt->init_func) && !(*pt->init_func)(pt) ) {
		Serial.print(F("!!! Failed to initialize task: "));
        Serial.println(pt->name);
		STOP;
	  } 
      BaseType_t res = xTaskCreatePinnedToCore(
        pt->task_func,
        pt->name,
        pt->stack_size,
        pt,
        pt->priority,
        &pt->task_handle, 
        pt->coreid
      );
      if( res != pdPASS) {
        Serial.print(F("!!! Failed to start task: "));
        Serial.println(pt->name);
        STOP;
	  } else {
        Serial.print(F("Task created: "));
        Serial.println(pt->name);
      } 
  }
}



bool dispatch_payload(queue_desc_t *queues, TXPAYLOAD *p, BYTE len) {
  pld_type_e pld_type = p->hdr.pld_type;

  DBSP("--- Dispatcher: "); DBSPLN(pld_type);
  for(queue_desc_t* pqd = queues; pqd->name != NULL; pqd++) {
	DBSP("------ "); DBSPLN(pqd->name);
    for(pld_type_e *ppt = pqd->pldtypes; *ppt != PLDT_NON; ppt++) {
	  DBSP("--------- "); DBSPLN(*ppt);
      if(*ppt == pld_type &&                                                                                  pqd->handle != NULL) {
          if (xQueueSend(pqd->handle, p, portMAX_DELAY) != pdPASS) {
              Serial.print(F("!!! DISPATCHER: Failed to store data into queue:"));
              Serial.println(pqd->name);
          }
          return true;
      }
    }
  }
  if( p->hdr.pld_type == PLDT_ACK) {
    ACKPAYLOAD *ackp = (ACKPAYLOAD*)p;
    SP("    counter = "); SP(ackp->hdr.counter); SP(", ack_counter = "); SP(ackp->ack_counter); SPLN("");
    SP("    cmmd: "); Serial.write(ackp->cmd); SPLN("");
    return true;
  } 
  return false;
}
