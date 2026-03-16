#ifndef __JPI_NRF_H__
#define __JPI_NRF_H__

#include <Arduino.h>
#include <SPI.h>
#include <RF24.h>

typedef uint8_t BYTE;
typedef uint16_t USHORT;
typedef uint32_t UINT;
typedef unsigned long ULONG;

#define STOP  while(1)


#define DEBUG_CODE 1
#ifdef DEBUG_CODE 
#define SP(str) Serial.print(str)
#define SPV(val, base) Serial.print(val, base)
#define SPLN(str) Serial.println(str)
#else 
#define SP(str) 
#define SPV(val, base) 
#define SPLN(str) 
#endif


const rf24_pa_dbm_e nrf_power = RF24_PA_MIN;
const bool nrf_auto_ack = true;
const bool nrf_dynamic_payload = true;
const bool nrf_ack_payload = true;
const BYTE nrf_channel = 37;
const rf24_datarate_e nrf_datarate = RF24_1MBPS; 

const uint64_t nrf_addr_telemetry = 0xF0F0F0F0E1LL;


typedef enum _pld_type_enum : uint8_t {
	PLDT_NON = 0,
	PLDT_ACK = 1,
	PLDT_LOX = 2,
	PLDT_DRV = 3,
	PLDT_PRT = 4
} pld_type_e;

#define MAXPAYLOADSIZE 32
 
typedef struct {
    USHORT counter;
	pld_type_e pld_type;
	BYTE pld_len;
} PLDHEADER;

typedef struct {
	PLDHEADER hdr;
	BYTE data[MAXPAYLOADSIZE - sizeof(PLDHEADER)-1];
} TXPAYLOAD;

typedef struct {
   PLDHEADER hdr;
   USHORT ack_counter;
   BYTE   cmd;
} ACKPAYLOAD;

typedef struct {
   PLDHEADER hdr;
   char msg[MAXPAYLOADSIZE-sizeof(PLDHEADER)];
} PRTPAYLOAD;


typedef struct {
	USHORT range;
	BYTE   status;
	BYTE   padding1;
} LOX_MEASURE;

#define MAX_LOX_MEASURES 6
typedef struct  {
	PLDHEADER hdr;
	BYTE nsensors;
	LOX_MEASURE lm[MAX_LOX_MEASURES];
} LOXPAYLOAD;

//extern LOXPAYLOAD loxpayload;
extern TXPAYLOAD txpayload;

#define CE_PIN 4
#define CSN_PIN 5
extern RF24 radio;


bool nrf_configure(RF24& radio);


#endif  // __JPI_NRF_H__