#include <Arduino.h>
#include "trilateration.h"


// Pin and device declarations {{{
#define ID_BIT_1 PB1
#define ID_BIT_2 PB0
#define ID_BIT_3 PA7
#define ID_BIT_4 PA6

#define DBG_SERIAL Serial2
// DEV_SERIAL is used for LiFi and UWB. device 
// samultaneously. That is, RX on STM32 board is
// connected to UWB., and TX is connected to
// Lifi ( for TX of it is bot being used ).
#define DEV_SERIAL Serial3
#define LOC_SERIAL Serial3

#define LED_CONTROL PB13

#define LED_STATE PB14

#define SEED_PIN PA0
// }}}

// Parameter declarations {{{

// The master device will receive distance between
// tag and each beacon, calculate its location,
// and then transmit the location to the specified
// device ( 1 ~ 6 ) via RS-485 via UART.
#define RLS_MASTER 0x0
#define RLS_ALLWAYS_ON 0x0A // Keeps light always on.
// Definations below is about debug mode. While the
// module runs under debug mode, it will generate
// a fake data. For master, it generates a fake
// distance data, for slave, it generates a fake
// localtion data. If you want to run under debug
// mode, set bit 1 to 'ON'. Otherwise the module
// will runs under release mode.
#define DBG_MASTER   0x8    // Debug in master mode.
#define DBG_SLAVE    0x9    // Debug in slave mode.

#define WINDOW_SIZE  20000  // Send window ( us )
#define LED_DEADTIME 120    // Led dead time ( us )
// }}}

extern unsigned char ID;
extern vec3d solution;

void loc_setup();
void loc_loop();

uint32_t hex2deci(const char* strHex);
void SendDataToLED(String s);
int MasterPreprocess(String comdata);
void AllwaysOn();
void Master();
void Slave();
void DBG_Master();
void DBG_Slave();
int GetID();
