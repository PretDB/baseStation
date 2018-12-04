#include "louketin.h"
#include "brd.h"
#include "trilateration.h"

extern bool bWiFiConnected;
extern bool bIPGot;
extern bool bUDPEstablished;
extern unsigned char ID;
extern vec3d solution;

void setup() {
  loc_setup();
  brd_setup();
}

void loop() {
  String loc = String(solution.x) + "\t" + String(solution.y);
  loc_loop();
  brd_loop();
  Serial.println(BroadcastMessage(loc));
}
