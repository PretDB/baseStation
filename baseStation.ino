#include "trilateration.h"

#define ID_BIT_1 PB0
#define ID_BIT_2 PB1
#define ID_BIT_3 PB10
#define ID_BIT_4 PB11

#define LED_CONTROL PC14

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


// ID: 0 for master..
// if ID == 0, the device will be the master
// device who send data to the other devies
static unsigned char ID = 0x0F;    // This is a invalid ID

vec3d solution;
vec3d anchors[4] = { { 0.0, 0.0, 0.0 },
  { 2.4, 3.0, 0.0 },
  { 4.8, 0.0, 0.0 },
  { 0.0, 0.0, 0.0 }
};
char tag = 0;
int dists[4];
String comdata = "";

String fake_dis[] = { "mc 07 00000b0e 000005dc 00000b0e 00000000 0001 c0 40424042 a0:0",
                      "mc 07 000005dc 00000b0e 000013a6 00000000 0002 c0 40424042 a0:0",
                      "mc 07 00000960 00000bb8 00000960 00000000 0003 c0 40424042 a0:0",
                      "mc 07 00000b0e 000005dc 00000b0e 00000000 0004 c0 40424042 a0:1",
                      "mc 07 000005dc 00000b0e 000013a6 00000000 0005 c0 40424042 a0:1",
                      "mc 07 00000960 00000bb8 00000960 00000000 0006 c0 40424042 a0:1"
                    };
String fake_loc[] = { "^B1T0X2.4Y1.5$%",
                      "^B2T0X0.0Y1.5$%",
                      "^B3T0X2.0Y0$%",
                      "^B4T1X2.4Y1.5$%",
                      "^B5T1X0.0Y1.5$%",
                      "^B6T1X2.0Y0$%"
                    };


uint32_t hex2deci(const char* strHex);
void SendDataToLED(String s);
int MasterPreprocess(String comdata);
void Master();
void Slave();
void DBG_Master();
void DBG_Slave();
int GetID();


void setup() {
  // put your setup code here, to run once:
  pinMode(LED_CONTROL, OUTPUT);    // LED controll pin
  pinMode(ID_BIT_1, INPUT);        // ID pin bit 0
  pinMode(ID_BIT_2, INPUT);
  pinMode(ID_BIT_3, INPUT);
  pinMode(ID_BIT_4, INPUT);
  // Serial2 for data rail
  Serial2.begin(2400);
  Serial2.setTimeout(200);
  // Serial1 for external devices, such as led transmiter and
  // uwb module.
  Serial1.begin(115200);
  Serial.begin(115200);    // Debug
  ID = GetID();
  // Clear Serial data on rx line.
  while (Serial2.read() >= 0);
  while (Serial1.read() >= 0);
}

void loop() {
  // put your main code here, to run repeatedly:
  ID = GetID();
  Serial.print("ID = " + String(ID));
  switch (ID)
  {
    case RLS_MASTER:
      Serial.println(", Master, Release");
      Master();
      break;

    case DBG_MASTER:
      Serial.println(", Master, Debug");
      DBG_Master();
      delay(1000);
      break;

    case DBG_SLAVE:
      Serial.println(", Slave, Debug");
      DBG_Slave();
      delay(1000);
      break;

    // If current device is not MASTER, it will only listens data
    // from 485 line on Serial1, and then transmit to Serial1.
    default:
      Serial.println(", default (Slave), Release");
      Slave();
      break;
  }
}

void SendDataToLED(String s)
{
  digitalWrite(LED_CONTROL, LOW);
  delay(1);
  Serial1.println(s);
  delay(2);
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
  i |= digitalRead(ID_BIT_1);
  i <<= 1;
  i |= digitalRead(ID_BIT_2);
  i <<= 1;
  i |= digitalRead(ID_BIT_3);
  i <<= 1;
  i |= digitalRead(ID_BIT_4);
  return i;
}

// MasterPreprocess: input string, set dists to prepare
// to calculate location.
int MasterPreprocess(String comdata)
{
  int index = comdata.indexOf('m');
  if (index == -1)
  {
    return -1;    // Not a valid Message.
  }
  else
  {
    if (comdata[index + 1] == 'c')
    {
      tag = comdata[comdata.indexOf(':') + 1];    // Get tag number.
      dists[0] = hex2deci(comdata.substring(index + 6, index + 14).c_str());
      dists[1] = hex2deci(comdata.substring(index + 15, index + 23).c_str());
      dists[2] = hex2deci(comdata.substring(index + 24, index + 32).c_str());
      dists[3] = dists[0];

      return 0;
    }
    else
    {
      return -1;    // Message broken.
    }
  }
}

void Master()
{
  static int currentSendDevice = 1;
  if (Serial1)
  {
    comdata = Serial1.readStringUntil('\n');
    if (MasterPreprocess(comdata) == 0)    // Got a set of valid distance data.
    {
      GetLocation(&solution, 0, anchors, dists);
      String msg = "^B" + String(currentSendDevice) + "T" + String(tag) + "X" + String(solution.x) + "Y" + String(solution.y) + "$%";

      Serial2.println(msg);
      while (Serial1.read() >= 0);
      currentSendDevice = constrain(++currentSendDevice, 1, 6);
    }
    else
    {
      return;
    }
  }
  else
  {
    comdata = "";
  }
}
void Slave()
{
  if (Serial2)
  {
    comdata = Serial2.readStringUntil('%');
    int index = comdata.indexOf('^');
    if (index == -1 || String(comdata[index + 2]).toInt() != ID)
    {
      return;
    }
    else
    {
      String msg = "^" + comdata.substring(index + 3);
      SendDataToLED(msg);
    }
  }
  while (Serial2.read() >= 0);
}

void DBG_Master()
{
  static int fake_dis_num = 0;

  String comdata = fake_dis[fake_dis_num];
  if (MasterPreprocess(comdata) == 0)
  {
    GetLocation(&solution, 0, anchors, dists);
    String msg = "^B" + String(0) + "T" + String(tag) + "X" + String(solution.x) + "Y" + String(solution.y) + "$%";
    Serial2.println(msg);
    Serial.println(msg);
  }

  fake_dis_num++;
  if(fake_dis_num == 6)
  {
    fake_dis_num = 0;
  }
}

// DBG_Slave only send loc data to led transmiter.
void DBG_Slave()
{
  static int fake_loc_num = 0;

  String comdata = fake_loc[fake_loc_num];

  String msg = "^" + comdata.substring(3);
  SendDataToLED(msg);
  Serial.println(msg);
  fake_loc_num++;
  if(fake_loc_num == 6)
  {
    fake_loc_num = 0;
  }
}

