#include "louketin.h"
#include "brd.h"
#include "trilateration.h"

#define LED_RUN    PB12

extern bool bWiFiConnected;
extern bool bIPGot;
extern bool bUDPEstablished;
extern unsigned char ID;
extern char tag;
extern vec3d solution;

void setup() {
  pinMode(LED_RUN, OUTPUT);
  loc_setup();
  brd_setup();
}

void loop() {
  digitalWrite(LED_RUN, LOW);
  
  loc_loop();
  if(ID == RLS_SWITCH)
  {
    brd_loop();
    
    
  }
  digitalWrite(LED_RUN, HIGH);
  // delay(100);
}
