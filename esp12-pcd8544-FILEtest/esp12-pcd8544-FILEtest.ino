

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include "spiffs/spiffs.h"
#include <FS.h>

// ESP8266 Software SPI (slower updates, more flexible pin options):
// pin 14 - Serial clock out (SCLK)
// pin 13 - Serial data out (DIN)
// pin 12 - Data/Command select (D/C)
// pin 5 - LCD chip select (CS)
// pin 4 - LCD reset (RST)
//Adafruit_PCD8544 display = Adafruit_PCD8544(14, 13, 12, 5, 4);

Adafruit_PCD8544 display = Adafruit_PCD8544(14, 13, 12, 5, 4);

#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2


#define LOGO16_GLCD_HEIGHT 16
#define LOGO16_GLCD_WIDTH  16

static const unsigned char PROGMEM logo16_glcd_bmp[] =
{ B00000000, B11000000,
  B00000001, B11000000,
  B00000001, B11000000,
  B00000011, B11100000,
  B11110011, B11100000,
  B11111110, B11111000,
  B01111110, B11111111,
  B00110011, B10011111,
  B00011111, B11111100,
  B00001101, B01110000,
  B00011011, B10100000,
  B00111111, B11100000,
  B00111111, B11110000,
  B01111100, B11110000,
  B01110000, B01110000,
  B00000000, B00110000 };

void setup()   {
  Serial.begin(115200);
  SPIFFS.begin();
  display.begin();
  // init done

  // you can change the contrast around to adapt the display
  // for the best viewing!
  display.setContrast(60);

  display.display(); // show splashscreen
  delay(2000);
display.println("ready to write file!");
  display.clearDisplay();   // clears the screen and buffer
  display.display(); // show splashscreen

}


void loop() {
  Dir dir = SPIFFS.openDir(String(F("/") ));
  while (dir.next())
  {
   String fn = dir.fileName();
    Serial.println(fn);
    display.println(fn);
  display.display(); // show splashscreen
    delay(0);
  }
  File f = SPIFFS.open(String("/test01.bas"), "w");
  if (!f)
  {
    Serial.println(F("file save failed"));
    display.println(F("file save failed"));
  display.display(); // show splashscreen
    delay(0);
  }
  else
  {
    f.println("10 a=10");
    f.println("20 for i=a to 10");
    f.println("30 print i");
    f.println("40 next i\r");
    
    f.close();
  }

//File 
  f = SPIFFS.open("/test01.bas", "r");
  if (!f)
  {
    Serial.println("file read failed  :");
    display.println("file read failed");
  display.display(); // show splashscreen
    delay(0);
  }
  else
  {
    Serial.println( f.readStringUntil('\r'));
    f.close();  
  }

      Serial.println( "waiting");

    delay(1000);
  
}



