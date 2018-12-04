#include <Arduino.h>
#include "trilateration.h"


// Pin and device declarations {{{
#define ID_BIT_1 PB0
#define ID_BIT_2 PB1
#define ID_BIT_3 PA6
#define ID_BIT_4 PA7

#define DBG_SERIAL Serial
#define DEV_SERIAL Serial3
#define DAT_SERIAL Serial2


#define LED_CONTROL PB13

#define LED_STATE PB12
// }}}

// Parameter declarations {{{

// The master device will receive distance between
// tag and each beacon, calculate its location,
// and then transmit the location to the specified
// device ( 1 ~ 6 ) via RS-485 via UART.
#define RLS_MASTER 0x0
// Definations below is about debug mode. While the
// module runs under debug mode, it will generate
// a fake data. For master, it generates a fake
// distance data, for slave, it generates a fake
// localtion data. If you want to run under debug
// mode, set bit 1 to 'ON'. Otherwise the module
// will runs under release mode.
#define DBG_MASTER   0x8    // Debug in master mode.
#define DBG_SLAVE    0x9    // Debug in slave mode.
// }}}

extern unsigned char ID;
extern vec3d solution;

void loc_setup();
void loc_loop();

uint32_t hex2deci(const char* strHex);
void SendDataToLED(String s);
int MasterPreprocess(String comdata);
void Master();
void Slave();
void DBG_Master();
void DBG_Slave();
int GetID();
