#include <ArduinoJson.h>
#include <Arduino.h>
#include "brd.h"
#include "louketin.h"

// Global variables {{{

unsigned char u8Device_ID = '17';


String sWifiCheck = "AT+CWJAP?";
String sWifiCheckExpected = "+CWJAP";
String sIPCheck = "AT+CIFSR";
String sIPCheckExpected = "+CIFSR:";
String sMux = "AT+CIPMUX=0";
String sUDPClose = "AT+CIPCLOSE";
String sUDPCloseExpected = "1,CLOSED";
String sUDPEstablishCommand = "AT+CIPSTART=\"UDP\",\"255.255.255.255\",6868,6688,0";
String sUDPStartExpected = "CONNECT";
String sUDPCheck = "AT+CIPSTATUS";
String sUDPCheckExpected = "STATUS";
String sSend = "AT+CIPSEND=";
String sSetThrough = "AT+CIPMODE=1";

String sAT = "AT";
String sSetNoEcho = "ATE0";
String sOK = "OK";
String sRecv = "Recv";



static uint32_t u32MessageID = 1;
bool bWiFiConnected = false;
bool bIPGot = false;
bool bUDPEstablished = false;

static String sIP;
static String sTargetIP = "255.255.255.255";

static uint16_t u16Pattern = 0x0000;
// }}}

// post_setup: Initialize the BRD serial port.  // {{{
void brd_setup()
{
  BRD_SERIAL.begin(115200);
  BRD_SERIAL.setTimeout(DEFAULT_SERIAL_TIMEOUT_SHORT);

  AllwaysOff();
}  // }}}

// This method only maintain the udp connection.
void brd_loop()
{
  unsigned char netStatus = CheckNetwork();
  DBG_SERIAL.println("\tNet Status: " + String(netStatus));

  if (netStatus == 0x03)
  {
    //Establish udp
    EstablishUDP();
    // DBG_SERIAL.println("EstablishUDP");
  }
  if (netStatus == 0x07)
  {
    CheckMessage();
  }
}

// CheckMessage: {{{
void CheckMessage()
{
  String msg = Receive();
  DBG_SERIAL.println("msg: " + msg);
  if (msg.startsWith("OFF"))
  {
    // Turn led off.
    AllwaysOff();
  }
  else if (msg.startsWith("ON"))
  {
    // Turn led on.
    AllwaysOn();
  }
}  // }}}

// BroadcastMessage: ret=bool Broadcast messgage given {{{
bool BroadcastMessage(String msg)
{
  String len = sSend + String(msg.length() + 2); // 2 is length of the tail.
  if (SetAndTest(&len, &sOK) == false)
  {
    return false;
  }


  if (SetAndTest(&msg, &sRecv) == false)
  {
    return false;
  }
  else
  {
    return true;
  }
  return false;
}  // }}}

bool EnterThrough()
{
  if (CheckNetwork() == 0b111)
  {
    // DBG_SERIAL.println("connected");
  }
  else
  {
  }
  return false;
}

// EstablishUDP: ret=bool establish UDP connection to broad cast messages.  {{{
bool EstablishUDP()
{
  if (SetAndTest(&sMux, &sOK) == false)
  {
    return false;
  }

  // I don't know what happend in statements below
  //if(SetAndTest(&sUDPCloseExpected, &sUDPCloseExpected) == false)
  //{
  //	return false;
  //}

  if (SetAndTest(&sUDPEstablishCommand, &sUDPStartExpected))
  {
    bUDPEstablished = true;
    return true;
  }
  else
  {
    bUDPEstablished = false;
    return false;
  }
}  // }}}

// CheckNetwork: Check WiFi connectin {{{
unsigned char CheckNetwork()
{
  if (bWiFiConnected == false)
  {
    if (SetAndTest(&sWifiCheck, &sWifiCheckExpected))
    {
      bWiFiConnected = true;
    }
    else
    {
      bWiFiConnected = false;
    }
  }

  if (bWiFiConnected == true && bIPGot == false)
  {
    String retval = SetAndGet(&sIPCheck, &sIPCheckExpected);
    retval.trim();
    String ip = retval.substring(14, retval.indexOf('"', 14));
    if (ip.startsWith("0."))
    {
      bIPGot = false;
    }
    else
    {
      bIPGot = true;
      sIP = ip;
    }
  }
  if (bWiFiConnected == true && bIPGot == true && bUDPEstablished == false)
  {
    String ret = SetAndGet(&sUDPCheck, &sUDPCheckExpected);
    ret.trim();
    int start = ret.indexOf('+');

    if (start != -1)
    {
      bUDPEstablished = true;
    }
    else  // UDP connection not established
    {
      bUDPEstablished = false;
    }
  }
  unsigned char retval = 0;
  if (bWiFiConnected)
  {
    retval |= 0x01;
  }
  if (bIPGot)
  {
    retval |= 0x02;
  }
  if (bUDPEstablished)
  {
    retval |= 0x04;
  }
  return retval;
} // }}}

// SetAndTest: ret=bool Test Value {{{
// Give a command, and an expected start
// Return true if command excuted with right return value starts with given
// start string(expectedStart).
bool SetAndTest(String* pCommand, String* pExpectedStart)
{
  for (int a = 0; a < DEFAULT_MAXTRIAL; a++)
  {
    String ret = Transmit(pCommand);
    if (ret.startsWith(*pExpectedStart))
    {
      return true;
    }
    if (a >= 3)
    {
      ClearState();
    }
    delay(DEFAULT_DELAY);
  }
  return false;
} // }}}

// SetAndGet: ret=String. Set and Get return value {{{
String SetAndGet(String* pCommand, String* pExpectedStart)
{
  for (int a = 0; a < DEFAULT_MAXTRIAL; a++)
  {
    String ret = Transmit(pCommand);

    if (ret.startsWith(*pExpectedStart))
    {
      return ret;
    }
    // esp may be not clear if a is larger than 3
    if (a >= 3)
    {
      ClearState();
    }
  }
  // After 5 trials with error echos from esp, return ""
  return "";
} // }}}

// ClearState: ret=void. Clear esp's state {{{
void ClearState()
{
  for (int a = 0; a < DEFAULT_MAXTRIAL; a++)
  {
    Transmit(&sSetNoEcho);
    Transmit(&sAT);
    delay(DEFAULT_DELAY);
  }
}  // }}}

// Transmit: Send to and receive from data from esp {{{
// If command == NULL, receive only.
String Transmit(String* pCommand)
{
  if (pCommand != NULL)
  {
    BRD_SERIAL.print(*pCommand);
    BRD_SERIAL.print("\r\n");
    BRD_SERIAL.flush();
  }
  else
  {
    BRD_SERIAL.setTimeout(DEFAULT_SERIAL_TIMEOUT_LONG);
  }
  String ret = Receive();
  BRD_SERIAL.setTimeout(DEFAULT_SERIAL_TIMEOUT_SHORT);

  return ret;
}  // }}}

// Receiver: Receive data from esp {{{
String Receive()
{
  String data = BRD_SERIAL.readString();
  data.trim();
  DBG_SERIAL.println("data: " + data);
  // If data comes from socket, remove its head.
  if (data.startsWith("+IPD,"))
  {
    data = data.substring(5);
    data = data.substring(data.indexOf(':') + 1);
  }
  return data;
}  // }}}
