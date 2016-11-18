/* Итак, какие функции хочу:
  1) собственно, вывод времени и фазы луны на дисплей
  1.1) функция чтения времени из RTC (DataRead)
  1.2) вывода времени на дисплей     (DataDisplay)
  1.3) расчета фазы луны по дате и времени (getPhase)
  1.4) вывод фазы луны
  2)запрос времени по NTP и с Serial или клавиатуры
  2.1) ввод и запоминание ntp сервера с клавиатуры или с Serial
  2.2) ввод и запоминание имени точки доступа и пароля с Serial или клавиатуры
  2.3) запрос времени по NTP
  2.4) установка времени RTC


*/
//extern "C" {
//#include "ttbasic.h"
//}

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#define printByte(args)  write(args);
#include "RTClib.h"
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
WiFiUDP udp;
LiquidCrystal_I2C lcd(0x27, 16, 2);
RTC_DS1307 rtc;

extern "C" {
#include "ttbasic.h"
}

//char ssid[] = "mao";  //  your network SSID (name)
//char pass[] = "maomaomao";       // your network password

//char ssid[] = { 0x00, 0x00, 0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00, 0x00 };
//char pass[] = { 0x00, 0x00, 0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00, 0x00 };

unsigned int localPort = 2390; // local port to listen for UDP packets
IPAddress timeServerIP;
//const char* ntpServerName = "time.nist.gov";
// char* ntpServerName = "ntp.yuterra.ru";
int TZ = 3;
const int NTP_PACKET_SIZE = 48;
byte packetBuffer[ NTP_PACKET_SIZE];
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

int eval1 = 0;
int backLight = 0;

int Hour = 0;
int Minute = 0;
int Second = 0;
int Year = 0;
int Month = 0;
int Day = 0;

byte symWiFi[8] = {
  B00000,
  B00000,
  B10101,
  B01010,
  B00100,
  B00100,
  B00100,
  B00100,
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
  },
  {
    B10001,
    B11100,
    B11100,
    B11110,
    B11110,
    B11100,
    B11100,
    B10001,
  },
  {
    B10001,
    B00011,
    B00011,
    B00011,
    B00011,
    B00011,
    B00011,
    B10001,
  },
  {
    B10001,
    B11000,
    B11000,
    B11000,
    B11000,
    B11000,
    B11000,
    B10001,
  },
  {
    B10000,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B10000,
  },
  {
    B00001,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B00001,
  },
  {
    B10001,
    B00110,
    B00100,
    B01100,
    B01100,
    B00100,
    B00110,
    B10001,
  },
  {
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

boolean isWiFi=true;

String strssid="yuterra.main"; //  your network SSID (name)
String strpass="apwh46fds"; // your network password
String strntpServerName = "ntp.yuterra.ru";

void setup() {

  ESP.wdtDisable();
  ESP.wdtEnable(150000);
  Wire.begin(0, 2);
  lcd.init(); // initialize the lcd
  lcd.backlight();
  backLight = 1;
  lcd.clear();
  lcd.createChar(0, symWiFi);
  lcd.setCursor(0, 0);
  lcd.print("Starting...");
  Serial.begin(9600);
  if (!rtc.begin()) {
    Serial.println("ERROR: Couldn't find RTC");
    lcd.setCursor(0, 0); 
    lcd.print("Couldn't find RTC");
    //        while (1);
  }

  lcd.clear();
  WiFi.begin( strssid.c_str(), strpass.c_str());
  lcd.setCursor(13, 1);
  int i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (i++ > 20) {
      yield();
      lcd.print("_");
      Serial.println("No WiFi found!!!");
      isWiFi=false;
      break;
    }
  }
  if (isWiFi) {
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.println("Starting UDP");
    udp.begin(localPort);
    Serial.print("Local port: ");
    Serial.println(udp.localPort());
    getNtpTime();
    lcd.printByte(0);
  }
//  Serial.println("------------");
//  Serial.println(WiFi.status());
//  Serial.println("------------");
  
  use();
}

/*void savenvram(char* ssid,char* pass,  char* ntpServerName){
uint8_t ssidData[20]={ 0x00, 0x00, 0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00, 0x00 };
for (int i=0;i<sizeof(ssid);i++)ssidData[i]=ssid[i]  ;
uint8_t passData[15]={ 0x00, 0x00, 0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00, 0x00 };
for (int i=0;i<sizeof(pass);i++)passData[i]=pass[i]  ;
 uint8_t ntpServerNameData[20]={ 0x00, 0x00, 0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00, 0x00 };
for (int i=0;i<sizeof(ntpServerName);i++)ntpServerNameData[i]=ntpServerName[i]  ;


  rtc.writenvram(0,0x01);
  rtc.writenvram(1, ssidData,20);
  rtc.writenvram(21,passData,15);
  rtc.writenvram(36,(uint8_t) ntpServerNameData,20);
  }
void loadnvram(){
  rtc.readnvram( ssidData,20,1);
  rtc.readnvram( passData,15,1);
  rtc.readnvram((uint8_t) ntpServerName,20,1);
  }
*/

void loop() {
  ESP.wdtDisable();
  if (Serial.available()) {
    ajustRTC();
  }
  yield();
  DataRead();
  yield();
  DataDisplay();
  yield();
  delay(10);
  ESP.wdtEnable(1500000);
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
  if (AG < 1.84566) {
    lcd.createChar(1, moon[4]);
    lcd.createChar(2, moon[5]);
  } else if (AG < 5.53699) {
    lcd.createChar(1, moon[4]);
    lcd.createChar(2, moon[7]);
  } else if (AG < 9.22831) {
    lcd.createChar(1, moon[4]);
    lcd.createChar(2, moon[1]);
  } else if (AG < 12.91963) {
    lcd.createChar(1, moon[2]);
    lcd.createChar(2, moon[1]);
  } else if (AG < 16.61096) {
    lcd.createChar(1, moon[0]);
    lcd.createChar(2, moon[1]);
  } else if (AG < 20.30228) {
    lcd.createChar(1, moon[0]);
    lcd.createChar(2, moon[3]);
  } else if (AG < 23.99361) {
    lcd.createChar(1, moon[0]);
    lcd.createChar(2, moon[5]);
  } else if (AG < 27.68493) {
    lcd.createChar(1, moon[6]);
    lcd.createChar(2, moon[5]);
  } else {
    lcd.createChar(1, moon[4]);
    lcd.createChar(2, moon[5]);
  }

  phase = AG;
  return phase;
}

double normalize(double v) { // normalize moon calculation between 0-1
  v = v - floor(v);
  if (v < 0) v = v + 1;
  return v;
}


// use explanation message

void use() {
  Serial.println("===== chip info: =====");
  yield();
  uint32_t realSize = ESP.getFlashChipRealSize();
  uint32_t ideSize = ESP.getFlashChipSize();
  FlashMode_t ideMode = ESP.getFlashChipMode();

  Serial.printf("Flash real id:   %08X\n", ESP.getFlashChipId());
  Serial.printf("Flash real size: %u\n\n", realSize);

  Serial.printf("Flash ide  size: %u\n", ideSize);
  Serial.printf("Flash ide speed: %u\n", ESP.getFlashChipSpeed());
  Serial.printf("Flash ide mode:  %s\n", (ideMode == FM_QIO ? "QIO" : ideMode == FM_QOUT ? "QOUT" : ideMode == FM_DIO ? "DIO" : ideMode == FM_DOUT ? "DOUT" : "UNKNOWN"));

  if (ideSize != realSize) {
    Serial.println("Flash Chip configuration wrong!\n");
  } else {
    Serial.println("Flash Chip configuration ok.\n");
  }
  yield();
  Serial.println("======================");

  Serial.println("\nUSE      : u U h[00-23]m[00-59]s[00-59]D[01-31]M[01-12]Y[00-49]");
  Serial.println("\nEXEMPLE  : h09m35d03 set time to 09h35 ");
  Serial.println("\nCommands : h** : hour,  m** : minutes, s** : seconds ");
  Serial.println("             M** : month, Y** : year,    D** : day of month.");
  Serial.println("u or U shows this message, b - backlight, all other caracter shows time.");
  yield();
  return;
}
// DS1307 time read function

void actualTimeToSerial() {
  Serial.print("\nActual time : ");
  Serial.print(addZero(Hour));
  Serial.print(":");
  Serial.print(addZero(Minute));
  Serial.print(":");
  Serial.print(addZero(Second)); // Выводим время
  Serial.print("   ");
  Serial.print(addZero(Day));
  Serial.print("/");
  Serial.print(addZero(Month));
  Serial.print("/");
  Serial.print(addZero(Year));
}

String addZero(int val) {
  if (val < 10) return "0" + String(val);
  else return String(val);
}

// set clock values

void ajustRTC() {
  char value = 0;
  char command = 0;
  DataRead();
  command = Serial.read();
  delay(50); //delay to allow good serial port reading
  value = byte((Serial.read() - 48) * 10); //-48 becaus ASCII value for 0 is 48, 1 is 49, etc and *10 because we read tens first
  delay(50);
  value += byte((Serial.read() - 48)); //and then we read units
  switch (command) {
    case 'h':
      Hour = value;
      Serial.print("hours set to ");
      Serial.println(value, DEC);
      break;
    case 'm':
      Minute = value;
      Serial.print("minutes set to ");
      Serial.println(value, DEC);
      break;
    case 's':
      Second = value;
      Serial.print("seconds set to ");
      Serial.println(value, DEC);
      break;
    case 'D':
      Day = value;
      Serial.print("day of month set to ");
      Serial.println(value, DEC);
      break;
    case 'M':
      Month = value;
      Serial.print("month set to ");
      Serial.println(value, DEC);
      break;
    case 'Y':
      Year = value;
      Serial.print("year set to ");
      Serial.println(value, DEC);
      break;
    case 'u':
    case 'U':
      use();
      break;
    case 'b':
      lcd.init(); // initialize the lcd
      lcd.clear();
      lcd.home();
      if (backLight == 1) {
        lcd.noBacklight();
        backLight = 0;
      } else {
        lcd.backlight();
        backLight = 1;
      }
      break;
    case 'p':
    case 'P':
      basic();
      lcd.clear();
      lcd.setCursor(0,0);
      break;

    default:
      break;
  }
  // This line sets the RTC with an explicit date & time, for example to set
  // January 21, 2014 at 3am you would call:
  // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  // int Hour = 0; int Minute = 0; int Second = 0; int Year = 0; int Month = 0; int Day = 0;
  rtc.adjust(DateTime(Year, Month, Day, Hour, Minute, Second));
  delay(100);
  actualTimeToSerial();

}

void DataRead() {
  int i;
  DateTime now = rtc.now();
  Hour = now.hour(); // Получаем значение текущего часа
  Minute = now.minute();
  Second = now.second();
  Year = now.year();
  Month = now.month();
  Day = now.day();

  getPhase(Year, Month, Day);
}

void DataDisplay() {
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
  return;
}

void getNtpTime() {
  //get a random server from the pool
//  WiFi.hostByName(ntpServerName, timeServerIP);

WiFi.hostByName(strntpServerName.c_str(), timeServerIP);

  sendNTPpacket(timeServerIP); // send an NTP packet to a time server
  // wait to see if a reply is available
  delay(1000);

  int cb = udp.parsePacket();
  if (!cb) {
    Serial.println("no tnp response yet");
  } else {
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
    Serial.print("Seconds since Jan 1 1900 = ");
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
    DateTime ntpEpoch=epoch+TZ*60*60; //local epoch
    /*Serial.println(ntpEpoch.year());
    Serial.println(ntpEpoch.month());
    Serial.println(ntpEpoch.day());
    Serial.println(ntpEpoch.hour());
    Serial.println(ntpEpoch.minute());
    Serial.println(ntpEpoch.second());

    DateTime now = rtc.now();*/

      Hour = ntpEpoch.hour(); // Получаем значение текущего часа
      Minute = ntpEpoch.minute();
      Second = ntpEpoch.second();
    Year = ntpEpoch.year();
    Month = ntpEpoch.month();
    Day = ntpEpoch.day();

    Serial.println("Ajust RTC!"); // print the second

    rtc.adjust(DateTime(Year, Month, Day, Hour, Minute, Second));

  }
}

// send an NTP request to the time server at the given address

unsigned long sendNTPpacket(IPAddress& address) {
  Serial.println("sending NTP packet...");
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011; // LI, Version, Mode
  packetBuffer[1] = 0; // Stratum, or type of clock
  packetBuffer[2] = 6; // Polling Interval
  packetBuffer[3] = 0xEC; // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  udp.beginPacket(address, 123); //NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}

