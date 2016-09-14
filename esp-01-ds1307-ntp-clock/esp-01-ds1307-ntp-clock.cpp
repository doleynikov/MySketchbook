
// This is for compatibility with both arduino 1.0 and previous versions
//#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#define printByte(args)  write(args);
#include "RTClib.h"
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

//char ssid[] = "mao";  //  your network SSID (name)
//char pass[] = "maomaomao";       // your network password

char ssid[] = "yuterra.main";  //  your network SSID (name)
char pass[] = "apwh46fds";       // your network password

unsigned int localPort = 2390;      // local port to listen for UDP packets
IPAddress timeServerIP;
//const char* ntpServerName = "time.nist.gov";
const char* ntpServerName = "ntp.yuterra.ru";
int TZ = 3;
const int NTP_PACKET_SIZE = 48;
byte packetBuffer[ NTP_PACKET_SIZE];
WiFiUDP udp;

LiquidCrystal_I2C lcd(0x27, 16, 2);
RTC_DS1307 rtc;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

int eval1 = 0;
int bl = 0;

int Hour = 0; int Minute = 0; int Second = 0; int Year = 0; int Month = 0; int Day = 0;

byte degree[8]  =
{
  B01000,
  B10100,
  B01000,
  B00111,
  B01000,
  B01000,
  B01000,
  B00111,
};

byte moon[8][8] = {
  {
    B10001,
    B00111,
    B00111,
    B01111,
    B01111,
    B00111,
    B00111,
    B10001,
  }, {
    B10001,
    B11100,
    B11100,
    B11110,
    B11110,
    B11100,
    B11100,
    B10001,
  }, {
    B10001,
    B00011,
    B00011,
    B00011,
    B00011,
    B00011,
    B00011,
    B10001,
  }, {
    B10001,
    B11000,
    B11000,
    B11000,
    B11000,
    B11000,
    B11000,
    B10001,
  }, {
    B10000,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B10000,
  }, {
    B00001,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B00001,
  }, {
    B10001,
    B00110,
    B00100,
    B01100,
    B01100,
    B00100,
    B00110,
    B10001,
  }, {
    B10001,
    B01100,
    B00100,
    B00110,
    B00110,
    B00100,
    B01100,
    B10001,
  }
};

void setup() {
  Wire.begin(0, 2);
  lcd.init();                      // initialize the lcd
  lcd.backlight();
  bl = 1;
  lcd.clear();
  lcd.createChar(0, degree);
  lcd.print("Starting...");
  Serial.begin(9600);
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    lcd.setCursor(0, 0); // устанавливаем позицию курсора на экране (на один символ правее левого верхнего угла)

    lcd.print("Couldn't find RTC");
    while (1);
  }
  use();
  WiFi.begin(ssid, pass);
  int i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (i++ > 20) {
      break; // no wifi
    }
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("Starting UDP");
  udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(udp.localPort());
  delay(100);
  getNtpTime();
}

void loop() {
  if (Serial.available()) {
    write_RTC();
  }

  DataRead ();
  DataDisplay();
  delay (10);

}

char getPhase(int Y, int M, int D) {
  double AG, IP;
  char phase;
  long YY, MM, K1, K2, K3, JD;
  YY = Y - floor((12 - M) / 10);
  MM = M + 9;
  if (MM >= 12) MM = MM - 12;

  K1 = floor(365.25 * (YY + 4712));
  K2 = floor(30.6 * MM + 0.5);
  K3 = floor(floor((YY / 100) + 49) * 0.75) - 38;

  JD = K1 + K2 + D + 59;
  if (JD > 2299160)
    JD = JD - K3;

  IP = normalize((JD - 2451550.1) / 29.530588853);
  AG = IP * 29.53;
  if (AG < 1.84566)
  {
    lcd.createChar(1, moon[4]);
    lcd.createChar(2, moon[5]);
  }
  else if (AG < 5.53699)
  {
    lcd.createChar(1, moon[4]);
    lcd.createChar(2, moon[7]);
  }
  else if (AG < 9.22831)
  {
    lcd.createChar(1, moon[4]);
    lcd.createChar(2, moon[1]);
  }
  else if (AG < 12.91963)
  {
    lcd.createChar(1, moon[2]);
    lcd.createChar(2, moon[1]);
  }
  else if (AG < 16.61096)
  {
    lcd.createChar(1, moon[0]);
    lcd.createChar(2, moon[1]);
  }
  else if (AG < 20.30228)
  {
    lcd.createChar(1, moon[0]);
    lcd.createChar(2, moon[3]);
  }
  else if (AG < 23.99361)
  {
    lcd.createChar(1, moon[0]);
    lcd.createChar(2, moon[5]);
  }
  else if (AG < 27.68493)
  {
    lcd.createChar(1, moon[6]);
    lcd.createChar(2, moon[5]);
  }
  else
  {
    lcd.createChar(1, moon[4]);
    lcd.createChar(2, moon[5]);
  }

  phase = AG;
  return phase;
}

double normalize(double v) {           // normalize moon calculation between 0-1
  v = v - floor(v);
  if (v < 0) v = v + 1;
  return v;
}


// use explanation message
void use() {
  Serial.println("\nUSE      : u U h[00-23]m[00-59]s[00-59]D[01-31]M[01-12]Y[00-49]");
  Serial.println("\nEXEMPLE  : h09m35d03 set time to 09h35 ");
  Serial.println("\nCommands : h** : hour,  m** : minutes, s** : seconds ");
  Serial.println("             M** : month, Y** : year,    D** : day of month.");
  Serial.println("u or U shows this message, b - backlight, all other caracter shows time.");
}
// DS1307 time read function
void read_RTC() {
  DateTime now = rtc.now();
  Serial.print("\nActual time : ");
  Serial.print(now.hour(), DEC); //read the hour
  Serial.print(":");
  Serial.print(now.minute(), DEC); //read minutes without update (false)
  Serial.print(":");
  Serial.print(now.second(), DEC); //read seconds
  Serial.print(" ");                 // some space for a more happy life
  Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
  Serial.print(" ");
  Serial.print(now.day(), DEC); //read date
  Serial.print("/");
  Serial.print(now.month(), DEC); //read month
  Serial.print("/");
  Serial.println(now.year(), DEC); //read year
}

String addZero(int val)
{
  if (val < 10) return "0" + String(val);
  else return String(val);
}

// set clock values
void write_RTC() {
  char value = 0;
  char command = 0;
  command = Serial.read();
  delay(50);                            //delay to allow good serial port reading
  value = byte((Serial.read() - 48) * 10);      //-48 becaus ASCII value for 0 is 48, 1 is 49, etc and *10 because we read tens first
  delay(50);
  value += byte((Serial.read() - 48));          //and then we read units
  switch (command) {
    case 'h' :
      Hour = value;
      Serial.print("hours set to ");
      Serial.println(value, DEC);
      break;
    case 'm' :
      Minute = value;
      Serial.print("minutes set to ");
      Serial.println(value, DEC);
      break;
    case 's' :
      Second = value;
      Serial.print("seconds set to ");
      Serial.println(value, DEC);
      break;
    case 'D' :
      Day = value;
      Serial.print("day of month set to ");
      Serial.println(value, DEC);
      break;
    case 'M' :
      Month = value;
      Serial.print("month set to ");
      Serial.println(value, DEC);
      break;
    case 'Y' :
      Year = value;
      Serial.print("year set to ");
      Serial.println(value, DEC);
      break;
    case 'u' :
    case 'U' :
      use();
      break;
    case 'r' :
      //      RTC.stop();
      Serial.println("Clock stopped");
      break;
    case 'R' :
      //      RTC.start();
      Serial.println("Clock running");
      lcd.init();                      // initialize the lcd
      lcd.backlight();
      lcd.clear();
      break;
    case 'b' :
      if (  bl == 1)
      {
        lcd.noBacklight();
        bl = 0;
      }
      else
      {
        lcd.backlight();
        bl = 1;
      }
      break;

    default :
      break;
  }
  // This line sets the RTC with an explicit date & time, for example to set
  // January 21, 2014 at 3am you would call:
  // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  // int Hour = 0; int Minute = 0; int Second = 0; int Year = 0; int Month = 0; int Day = 0;
  rtc.adjust(DateTime(Year, Month, Day, Hour, Minute, Second));
  delay(100);
  read_RTC();
}

void DataRead ()
{
  int i;
  DateTime now = rtc.now();

  Hour = now.hour(); // Получаем значение текущего часа
  Minute = now.minute();
  Second = now.second();
  Year   = now.year();
  Month  = now.month();
  Day    = now.day();

  getPhase(Year, Month,  Day);
}

void DataDisplay ()
{
  lcd.setCursor(0, 0); // устанавливаем позицию курсора на экране (на один символ правее левого верхнего угла)
  lcd.print(addZero(Hour));
  lcd.print(":");
  lcd.print(addZero(Minute));
  lcd.print(":");
  lcd.print(addZero(Second)); // Выводим время
  lcd.setCursor(0, 1);
  lcd.print(addZero(Day));
  lcd.print("/");
  lcd.print(addZero(Month));
  lcd.print("/");
  lcd.print(addZero(Year));
  lcd.setCursor(14, 1);
  lcd.printByte(1);
  lcd.printByte(2);
  //delay(100);
}

void getNtpTime() {
  //get a random server from the pool
  WiFi.hostByName(ntpServerName, timeServerIP);

  sendNTPpacket(timeServerIP); // send an NTP packet to a time server
  // wait to see if a reply is available
  delay(1000);

  int cb = udp.parsePacket();
  if (!cb) {
    Serial.println("no tnp response yet");
  }
  else {
    Serial.print("ntp response received, length=");
    Serial.println(cb);
    // We've received a packet, read the data from it
    udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

    //the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, esxtract the two words:

    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    Serial.print("Seconds since Jan 1 1900 = " );
    Serial.println(secsSince1900);

    // now convert NTP time into everyday time:
    Serial.print("Unix time = ");
    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    const unsigned long seventyYears = 2208988800UL;
    // subtract seventy years:
    unsigned long epoch = secsSince1900 - seventyYears;
    // print Unix time:
    Serial.println(epoch);


    // print the hour, minute and second:
    Serial.print("The UTC time is ");       // UTC is the time at Greenwich Meridian (GMT)
    Hour = (epoch  % 86400L) / 3600;
    Serial.print(Hour); // print the hour (86400 equals secs per day)
    Serial.print(':');
    Minute = (epoch  % 3600) / 60;
    Serial.print(Minute); // print the minute (3600 equals secs per minute)
    Serial.print(':');
    Second = epoch % 60;
    Serial.println(epoch % 60); // print the second

    DateTime now = rtc.now();

    //  Hour = now.hour(); // Получаем значение текущего часа
    //  Minute = now.minute();
    //  Second = now.second();
    Year   = now.year();
    Month  = now.month();
    Day    = now.day();

    Serial.println("Ajust RTC!"); // print the second

    rtc.adjust(DateTime(Year, Month, Day, Hour + TZ, Minute, Second));

  }
}

// send an NTP request to the time server at the given address
unsigned long sendNTPpacket(IPAddress& address)
{
  Serial.println("sending NTP packet...");
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  udp.beginPacket(address, 123); //NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}

