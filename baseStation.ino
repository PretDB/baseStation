#include "louketin.h"
#include "brd.h"
#include "trilateration.h"

extern bool bWiFiConnected;
extern bool bIPGot;
extern bool bUDPEstablished;
extern unsigned char ID;
extern char tag;
extern vec3d solution;

void setup() {
  loc_setup();
  brd_setup();
}

void loop() {
  String msg = "^T" + String(tag) + "X" + String(solution.x) + "Y" + String(solution.y) + "$";
  loc_loop();
  brd_loop();
  BroadcastMessage(msg);
}
