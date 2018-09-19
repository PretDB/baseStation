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

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial2.begin(115200);
  while (Serial2.read() >= 0);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (Serial2.available() > 0)
  {
    String comdata = Serial2.readStringUntil('\n');
    if (comdata[0] == 'm')
    {
      Serial.print("Raw data: ");
      Serial.println(comdata);

      int d0 = hex2deci(comdata.substring(6, 14).c_str());
      int d1 = hex2deci(comdata.substring(15, 23).c_str());
      int d2 = hex2deci(comdata.substring(24, 32).c_str());
      int d3 = hex2deci(comdata.substring(33, 41).c_str());
      if (comdata[1] == 'c')
      {
        Serial.print("distance to A0: ");
        Serial.print(d0);
        Serial.print("\tdistance to A1: ");
        Serial.print(d1);
        Serial.print("\tdistance to A2: ");
        Serial.println(d2);
      }
    }
    else
    {
      comdata = "";
    }
  }
  while (Serial2.read() >= 0);
}
