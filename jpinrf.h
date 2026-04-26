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


#define SP(str) Serial.print(str)
#define SPV(val, base) Serial.print(val, base)
#define SPLN(str) Serial.println(str)

#ifdef DEBUG_CODE 
#define DBSP(str) Serial.print(str)
#define DBSPV(val, base) Serial.print(val, base)
#define DBSPLN(str) Serial.println(str)
#else 
#define DBSP(str) 
#define DBSPV(val, base) 
#define DBSPLN(str) 
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

#define MAX_DRVS 4
typedef struct  {
	PLDHEADER hdr;
	BYTE drv_number;
	int8_t drv_pct[MAX_DRVS];
} DRVPAYLOAD;


const int max_queue_pld_types = 6;
typedef struct  {
  char *name;
  QueueHandle_t handle;
  BYTE    qu_size;
  BYTE    el_size;
  pld_type_e pldtypes[max_queue_pld_types];
} queue_desc_t;


struct task_desc_t;
typedef bool (*InitFunction_t)(task_desc_t *);

struct task_desc_t {
  TaskFunction_t task_func;
  InitFunction_t init_func;
  TaskHandle_t task_handle;
  char *name;
  uint32_t stack_size;
  USHORT priority;
  USHORT coreid;
  queue_desc_t *in_q_desc;
  queue_desc_t *out_q_desc;
};



extern TXPAYLOAD txpayload;

#define CE_PIN 4
#define CSN_PIN 5
extern RF24 radio;


char *getBootReasonString(esp_reset_reason_t reason);
char *getRangeStatusString(BYTE rangeStatus);

bool nrf_configure(RF24& radio);

void SetupQueuesAndTasks(queue_desc_t *queues, task_desc_t *tasks);
bool dispatch_payload(queue_desc_t *queues, TXPAYLOAD *p, BYTE len);




#endif  // __JPI_NRF_H__