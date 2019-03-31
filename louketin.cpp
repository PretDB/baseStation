#include "louketin.h"
#include "trilateration.h"

// Global variable declarations {{{
// ID: 0 for master..
// if ID == 0, the device will be the master
// device who send data to the other devies
unsigned char ID = 0x0F;    // This is a invalid ID

vec3d solution;
vec3d anchors[4] = { { 1.50, 1.25, 0.00 },
  { 3.00, 2.75, 0.00 },
  { 4.50, 1.25, 0.00 },
  { 0.00, 0.00, 0.00 }
};
char tag = '0';
int dists[4];
String comdata = "";

String fake_dis[] = { "mc 07 00000b0e 000005dc 00000b0e 00000000 0001 c0 40424042 a1:0",
                      "mc 07 000005dc 00000b0e 000013a6 00000000 0002 c0 40424042 a0:0",
                      "mc 07 00000960 00000bb8 00000960 00000000 0003 c0 40424042 a1:0",
                      "mc 07 00000b0e 000005dc 00000b0e 00000000 0004 c0 40424042 a0:1",
                      "mc 07 000005dc 00000b0e 000013a6 00000000 0005 c0 40424042 a1:1",
                      "mc 07 00000960 00000bb8 00000960 00000000 0006 c0 40424042 a0:1"
                    };
String fake_loc[] = { "^B1T0X2.4Y1.5$%",
                      "^B2T0X0.0Y1.5$%",
                      "^B3T0X2.0Y0$%",
                      "^B4T1X2.4Y1.5$%",
                      "^B5T1X0.0Y1.5$%",
                      "^B6T1X2.0Y0$%"
                    };

// }}}

// loc setup {{{
void loc_setup()
{
  pinMode(LED_CONTROL, OUTPUT);    // LED controll pin
  pinMode(LED_STATE, OUTPUT);      // State indicator
  pinMode(ID_BIT_1, INPUT);        // ID pin bit 0
  pinMode(ID_BIT_2, INPUT);
  pinMode(ID_BIT_3, INPUT);
  pinMode(ID_BIT_4, INPUT);

  // Init seed pin as analog input to read a random value to init
  // the random number generator.
  pinMode(SEED_PIN, INPUT_ANALOG); 

  // DEV_SERIAL for external devices, such as led transmiter and
  // uwb module.
  DEV_SERIAL.begin(115200);
  DEV_SERIAL.setTimeout(500);
  DBG_SERIAL.begin(115200);    // Debug

  // Clear Serial data on rx line.
  while (DEV_SERIAL.read() >= 0);

  DBG_SERIAL.println("loc Initialized");
}
// }}}

// loc loop{{{
void loc_loop()
{
  ID = GetID();

  DBG_SERIAL.print("ID = " + String(ID));
  switch (ID)
  {
    case RLS_ALLWAYS_ON:
      AllwaysOn();
      break;
    case RLS_MASTER:
      DBG_SERIAL.println(", Master, Release");
      Master();
      break;

    case DBG_MASTER:
      DBG_SERIAL.println(", Master, Debug");
      DBG_Master();
      delay(900);
      break;

    case DBG_SLAVE:
      DBG_SERIAL.println(", Slave, Debug");
      DBG_Slave();
      delay(900);
      break;

    // If current device is not MASTER, it will send its ID.
    default:
      DBG_SERIAL.println(", default (Slave), Release");
      Slave();
      break;
  }
}
// }}}

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

void SendDataToLED(String s)
{
  digitalWrite(LED_STATE, LOW);
  DEV_SERIAL.print(s);
  digitalWrite(LED_CONTROL, LOW);
  delayMicroseconds(LED_DEADTIME * (s.length() + 1));
  digitalWrite(LED_CONTROL, HIGH);
  digitalWrite(LED_STATE, HIGH);
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
  int index = comdata.indexOf(':');
  if (index == -1)
  {
    return -1;    // Not a valid Message.
  }
  else
  {
    if (comdata.length() >= 60 && comdata.indexOf('7') < 5)
    {
      tag = comdata[index - 1];
      String st0 = comdata.substring(index - 55, index - 47);
      String st1 = comdata.substring(index - 46, index - 38);
      String st2 = comdata.substring(index - 37, index - 29);
      dists[0] = hex2deci(st0.c_str());
      dists[1] = hex2deci(st1.c_str());
      dists[2] = hex2deci(st2.c_str());
      dists[3] = dists[0];
      DBG_SERIAL.print(dists[0]);
      DBG_SERIAL.print(" ");
      DBG_SERIAL.print(dists[1]);
      DBG_SERIAL.print(" ");
      DBG_SERIAL.println(dists[2]);
      return 0;
    }
    else
    {
      return -1;    // Message broken.
    }
  }
}

void AllwaysOn()
{
  digitalWrite(LED_CONTROL, HIGH);
  digitalWrite(LED_STATE, LOW);
}

void Master()
{
  if (DEV_SERIAL)
  {
    comdata = DEV_SERIAL.readStringUntil('\n');

    DBG_SERIAL.println(comdata);
    if (MasterPreprocess(comdata) == 0)    // Got a set of valid distance data.
    {
      GetLocation(&solution, 0, anchors, dists);

      while (DEV_SERIAL.read() >= 0);
    }
  }
  else
  {
    comdata = "";
  }
  while (DEV_SERIAL.read() >= 0);

  // Send ID as a slave
  Slave();
}
void Slave()
{
  //
  //             Total window size         
  // /---------------------------------------\
  //
  // /---------------------------------------\
  // |                                       |
  // \---------------------------------------/
  //
  // \---------------------------------/
  // Valid send start time ( windowSize - deadTime )
  //                          Dead time \----/

  String message = String(ID);
  int startTime = random(WINDOW_SIZE - LED_DEADTIME);
  
  delayMicroseconds(startTime);
  SendDataToLED(message);
  delayMicroseconds(WINDOW_SIZE - startTime);
}

void DBG_Master()
{
  static int fake_dis_num = 0;
  static int currentSendDevice = 1;
  String comdata = fake_dis[fake_dis_num];
  if (MasterPreprocess(comdata) == 0)
  {
    GetLocation(&solution, 0, anchors, dists);
    //String msg = "^B" + String(currentSendDevice) + "T" + String(tag) + "X" + String(solution.x) + "Y" + String(solution.y) + "$%";
    String msg = "^B" + String(currentSendDevice) + "B" + String(currentSendDevice) + "$%";
    DBG_SERIAL.println(msg);
  }

  fake_dis_num++;

  if (fake_dis_num == 6)
  {
    fake_dis_num = 0;
  }

  if (currentSendDevice == 6)
  {
    currentSendDevice = 1;
  }
  else
  {
    currentSendDevice++;
  }
}

// DBG_Slave only send loc data to led transmiter.
void DBG_Slave()
{
  Slave();
}
