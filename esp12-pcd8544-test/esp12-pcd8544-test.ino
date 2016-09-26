#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>

/******************************************************************
ESP8266 with PCD8544 display
== Parts ==
* Adafruit Huzzah ESP8266 https://www.adafruit.com/products/2471
* Adafruit PCD8544/5110 display https://www.adafruit.com/product/338
* Adafruit USB to TTL serial cable https://www.adafruit.com/products/954
== Connection ==
USB TTL     Huzzah      Nokia 5110  Description
            ESP8266     PCD8544
            GND         GND         Ground
            3V          VCC         3.3V from Huzzah to display
            14          CLK         Output from ESP SPI clock
            13          DIN         Output from ESP SPI MOSI to display data input
            12          D/C         Output from display data/command to ESP
            #5          CS          Output from ESP to chip select/enable display
            #4          RST         Output from ESP to reset display
            15          LED         3.3V to turn backlight on, GND off
GND (blk)   GND                     Ground
5V  (red)   V+                      5V power from PC or charger
TX  (green) RX                      Serial data from IDE to ESP
RX  (white) TX                      Serial data to ESP from IDE
******************************************************************/

// Hardware SPI (faster, but must use certain hardware pins):
// SCK is LCD serial clock (SCLK) - this is pin 14 on Huzzah ESP8266
// MOSI is LCD DIN - this is pin 13 on an Huzzah ESP8266
// pin 12 - Data/Command select (D/C) on an Huzzah ESP8266
// pin 5 - LCD chip select (CS)
// pin 4 - LCD reset (RST)
Adafruit_PCD8544 display = Adafruit_PCD8544(12, 5, 4);



// Looks a capital C with two horizontal lines
const uint8_t EURO_SYMBOL[] PROGMEM = {
  B00110000,
  B01001000,
  B11100000,
  B01000000,
  B11100000,
  B01001000,
  B00110000,
};

// 60 second delay between normal updates
#define DELAY_NORMAL    (60*1000)
// 10 minute delay between updates after an error
#define DELAY_ERROR     (10*60*1000)


void setup()
{
  Serial.begin(115200);

  // Turn LCD backlight on
  pinMode(15, OUTPUT);
  digitalWrite(15, HIGH);

  display.begin();
  // init done

  // you can change the contrast around to adapt the display
  // for the best viewing!
  display.setContrast(50);

  display.display(); // show splashscreen

  Serial.println();
  Serial.print(F("Welcome!"));
 delay (500);
  // text display tests
  display.setTextSize(1);
  display.setTextWrap(false);
  display.setTextColor(BLACK);

  // Prices provided by CoinDesk so give them credit.
  // See http://www.coindesk.com/api/ for details.
  display.clearDisplay();   // clears the screen and buffer
  display.println(F("Here we are!"));
  display.println();
  display.setTextWrap(true);
  display.println(F("here is an example of some looong test string!"));
  display.setTextWrap(false);
  display.display();
  delay(2000);
}

void loop()
{
  display.println("1");
  display.println("12");
  display.println("123");
  display.println("1234");
  display.println("12345");
  display.println("123456");
  display.println("1234567");
  display.println("12345678");
  display.println("123456789");
  display.display();
delay(1000);
 yield();
}

