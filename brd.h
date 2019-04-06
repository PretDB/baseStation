#include "trilateration.h"


// Defination and includes {{{
#define DEVICE_ID                    u8Device_ID

#define DEFAULT_SERIAL_TIMEOUT_LONG  500 // microseconds
#define DEFAULT_SERIAL_TIMEOUT_SHORT 10
#define DEFAULT_DELAY                100

#define DEFAULT_MAXTRIAL             5  // max trails to communacate with esp8266

#define DEFAULT_TARGET_UDP_PORT      6875
#define DEFAULT_LOCAL_UDP_PORT       6875

#define USE_SEMAPHORE_DMA1

#define DBG_SERIAL Serial2
#define BRD_SERIAL Serial1

extern bool bWiFiConnected;
extern bool bIPGot;
extern bool bUDPEstablished;
extern vec3d solution;

void brd_setup();
void brd_loop();
bool BroadcastMessage(String msg);
void CheckMessage();
bool EnterThrough();
bool EstablishUDP();
unsigned char CheckNetwork();
bool SetAndTest(String* pCommand, String* pExpectedStart);
String SetAndGet(String* pCommand, String* pExpectedStart);
void ClearState();
String Transmit(String* pCommand);
String Receive();
