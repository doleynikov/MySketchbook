
// Адаптировано!
// Работает! 1.6.5
// сделать ввод с сериального порта! в файле host.cpp

//#include <font.h>
#include "Arduino.h"

#include <PCD8544.h>
//#include <SSD1306ASCII.h>
// ^ - modified for faster SPI
#include <PS2Keyboard.h>
#include <EEPROM.h>

#include "basic.h"
#include "host.h"

// Define in host.h if using an external EEPROM e.g. 24LC256
// Should be connected to the I2C pins
// SDA -> Analog Pin 4, SCL -> Analog Pin 5
// See e.g. http://www.hobbytronics.co.uk/arduino-external-eeprom

// If using an external EEPROM, you'll also have to initialise it by
// running once with the appropriate lines enabled in setup() - see below

#if EXTERNAL_EEPROM
#include <i2cmaster.h>
// Instance of class for hardware master with pullups enabled
TwiMaster rtc(true);
#endif

// Keyboard
const int DataPin = 8;
const int IRQpin =  2;
PS2Keyboard keyboard;

// OLED
//#define OLED_DATA 9
//#define OLED_CLK 10
//#define OLED_DC 11
//#define OLED_CS 12
//#define OLED_RST 13
//SSD1306ASCII oled(OLED_DATA, OLED_CLK, OLED_DC, OLED_RST, OLED_CS);

//        PCD8544(unsigned char sclk  = 7,//13   /* clock       (screen pin 2) */
//                unsigned char sdin  = 6,//11   /* data-in     (screen pin 3) */
//                unsigned char dc    = 5,   /* data select (screen pin 4) */
//                unsigned char reset = 3,   /* reset       (screen pin 8) */
//                unsigned char sce   = 4);  /* enable      (screen pin 5) */

 PCD8544 screen (7,6,5,3,4);

// NB Keyboard needs a seperate ground from the OLED

// buzzer pin, 0 = disabled/not present
#define BUZZER_PIN    9

// BASIC
unsigned char mem[MEMORY_SIZE];
#define TOKEN_BUF_SIZE    64
unsigned char tokenBuf[TOKEN_BUF_SIZE];

const char welcomeStr[] PROGMEM = "Arduino BASIC";
char autorun = 0;

void setup() {

    Serial.begin(9600);   // opens serial port, sets data rate to 9600 bps

    keyboard.begin(DataPin, IRQpin);
//    oled.ssd1306_init(SSD1306_SWITCHCAPVCC);

  screen.begin(84, 48);
  screen.setContrast(70);

  screen.clear();
  screen.setCursor(0, 0);

    reset();
    host_init(BUZZER_PIN);
    host_cls();
    host_outputProgMemString(welcomeStr);
    // show memory size
    host_outputFreeMem(sysVARSTART - sysPROGEND);
    host_showBuffer();
    
    // IF USING EXTERNAL EEPROM
    // The following line 'wipes' the external EEPROM and prepares
    // it for use. Uncomment it, upload the sketch, then comment it back
    // in again and upload again, if you use a new EEPROM.
    // writeExtEEPROM(0,0); writeExtEEPROM(1,0);

    if (EEPROM.read(0) == MAGIC_AUTORUN_NUMBER)
        autorun = 1;
    else
        host_startupTone();
}

void loop() {
    int ret = ERROR_NONE;

    if (!autorun) {
        // get a line from the user
        char *input = host_readLine();
        // special editor commands
        if (input[0] == '?' && input[1] == 0) {
            host_outputFreeMem(sysVARSTART - sysPROGEND);
            host_showBuffer();
            return;
        }
        // otherwise tokenize
        ret = tokenize((unsigned char*)input, tokenBuf, TOKEN_BUF_SIZE);
    }
    else {
        host_loadProgram();
        tokenBuf[0] = TOKEN_RUN;
        tokenBuf[1] = 0;
        autorun = 0;
    }
    // execute the token buffer
    if (ret == ERROR_NONE) {
        host_newLine();
        ret = processInput(tokenBuf);
    }
    if (ret != ERROR_NONE) {
        host_newLine();
        if (lineNumber !=0) {
            host_outputInt(lineNumber);
            host_outputChar('-');
        }
        host_outputProgMemString((char *)pgm_read_word(&(errorTable[ret])));
    }
}

