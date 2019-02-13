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
  String msg = "^T" + String(tag) + "X" + String(solution.x) + "Y" + String(solution.y) + "$";
  
  loc_loop();
  if(ID == RLS_MASTER)
  {
    brd_loop();
    BroadcastMessage(msg);
  }
  digitalWrite(LED_RUN, HIGH);
  delay(100);
}
