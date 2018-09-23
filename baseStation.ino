#include "trilateration.h"

#define ID_BIT_0 PB0
#define ID_BIT_1 PB1
#define ID_BIT_2 PB10
#define ID_BIT_3 PB11

#define LED_CONTROL PC13

#define MASTER 1


// ID: 0 for debug, 1~6 for specific device.
// if ID == 1, the device will be the master
// device who send data to the other devies.
static unsigned char ID = 0;

vec3d solution;
vec3d anchors[4];
int dists[4];
int currentSendDevice = 2;
String comdata = "";




uint32_t hex2deci(const char* strHex);
void SendDataToLED(String s);
int Master();
int GetID();


void setup() {
  // put your setup code here, to run once:
  anchors[0].x = 0;
  anchors[0].y = 0;
  anchors[0].z = 0;
  anchors[1].x = 2.4;
  anchors[1].y = 3;
  anchors[1].z = 0;
  anchors[2].x = 4.8;
  anchors[2].y = 0;
  anchors[2].z = 0;

  pinMode(PC13, OUTPUT);
  pinMode(ID_BIT_0, OUTPUT);
  pinMode(ID_BIT_1, OUTPUT);
  pinMode(ID_BIT_2, OUTPUT);
  pinMode(ID_BIT_3, OUTPUT);
  // Serial1 for data rail
  Serial.begin(115200);
  Serial.setTimeout(200);
  // Serial2 for external devices, such as led transmiter and
  // uwb module.
  Serial2.begin(115200);
  ID = GetID();
  // Clear Serial data on rx line.
  while (Serial.read() >= 0);
  while (Serial2.read() >= 0);
}

void loop() {
  // put your main code here, to run repeatedly:
  switch (ID)
  {
    case MASTER:
      Master();
      break;

    // If current device is not MASTER, it will only listens data
    // from 485 line on Serial1, and then transmit to Serial2.
    default:
      Slave();
      break;
  }
}

void SendDataToLED(String s)
{
  digitalWrite(LED_CONTROL, LOW);
  delay(1);
  Serial2.println(s);
  delay(1);
  digitalWrite(LED_CONTROL, HIGH);
}

uint32_t hex2deci(const char* strHex)
{
  uint32_t dwValue = 0;
  const char *pCh = strHex;
  while (*pCh != 0)
  {
    dwValue <<= 4;
    if (*pCh >= '0' && *pCh <= '9')
    {
      dwValue += *pCh - '0';
    }
    else if (*pCh >= 'A' && *pCh <= 'F')
    {
      dwValue += *pCh - 'A' + 10;
    }
    else if (*pCh >= 'a' && *pCh <= 'f')
    {
      dwValue += *pCh - 'a' + 10;
    }
    else return 0;
    pCh++;
  }
  return dwValue;
}

int GetID()
{
  int i = 0;
  i |= digitalRead(ID_BIT_0);
  i <<= 1;
  i |= digitalRead(ID_BIT_1);
  i <<= 1;
  i |= digitalRead(ID_BIT_2);
  i <<= 1;
  i |= digitalRead(ID_BIT_3);
  return i;
}
int Master()
{
  if (Serial2)
  {
    comdata = Serial2.readStringUntil('\n');
    int index = comdata.indexOf('m');
    if (index == -1)
    {
      return;
    }
    else
    {
      char tag = comdata[comdata.indexOf(':') + 1];
      dists[0] = hex2deci(comdata.substring(index + 6, index + 14).c_str());
      dists[1] = hex2deci(comdata.substring(index + 15, index + 23).c_str());
      dists[2] = hex2deci(comdata.substring(index + 24, index + 32).c_str());
      dists[3] = hex2deci(comdata.substring(index + 33, index + 41).c_str());
      if (comdata[inde + 1] == 'c')
      {
        dists[3] = dists[0];
        GetLocation(&solution, 0, anchors, dists);
        String msg = "^B" + String(currentSendDevice) + "T" + String(tag) + "X" + String(solution.x) + "Y" + String(solution.y) + "$%";
        Serial.println(msg);
        while (Serial2.read() >= 0);
        currentSendDevice++;
        if (currentSendDevice > 6)
        {
          currentSendDevice = 2;
        }
      }
    }
    else
    {
      comdata = "";
    }
  }
}
void Slave()
{
  if (Serial)
  {
    comdata = Serial.readStringUntil('%');
    int index = comdata.indexOf('B');
    if (index == -1 || String(comdata[index + 1]).toInt() != ID)
    {
      return;
    }
    else
    {
      String msg = "^" + comdata.substring(index + 2, comdata.indexOf('$')) + "$";
      Serial2.println(msg);
    }
  }
  while (Serial.read() >= 0);
}

