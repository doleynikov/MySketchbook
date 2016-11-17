
#include <PCD85448266.h>
//#include <PS2Keyboard.h>
#include <EEPROM.h>

#include "basic.h"
#include "host.h"

/*
 * To use this sketch, connect the eight pins from your LCD like thus:
 *
 *       PCD8544(unsigned char sclk  = 14,    clock       (display pin 2) 
                unsigned char sdin  = 13,    data-in     (display pin 3)
                unsigned char dc    = 12,    data select (display pin 4)
                unsigned char reset = 4,    reset       (display pin 8) 
                unsigned char sce   = 5);   enable      (display pin 5) 

 * Since these LCDs are +3.3V devices, you have to add extra components to
 * connect it to the digital pins of the Arduino (not necessary if you are
 * using a 3.3V variant of the Arduino, such as Sparkfun's Arduino Pro).
 */


// Define in host.h if using an external EEPROM e.g. 24LC256
// Should be connected to the I2C pins
// SDA -> Analog Pin 4, SCL -> Analog Pin 5
// See e.g. http://www.hobbytronics.co.uk/arduino-external-eeprom

// If using an external EEPROM, you'll also have to initialise it by
// running once with the appropriate lines enabled in setup() - see below



// Keyboard
const int DataPin = 8;
const int IRQpin =  3;
//PS2Keyboard keyboard;

PCD8544 display;

// NB Keyboard needs a seperate ground from the OLED

// buzzer pin, 0 = disabled/not present
#define BUZZER_PIN    0

// BASIC
unsigned char mem[MEMORY_SIZE];
#define TOKEN_BUF_SIZE    64
unsigned char tokenBuf[TOKEN_BUF_SIZE];

const char welcomeStr[] PROGMEM = "Arduino BASIC";
char autorun = 0;

void setup() {
  Serial.begin(115200);
//    keyboard.begin(DataPin, IRQpin);
//    display.ssd1306_init(SSD1306_SWITCHCAPVCC);
display.begin(84,48);
display.println(".");

    reset();
    host_init(BUZZER_PIN);
    host_cls();
    host_outputProgMemString(welcomeStr);
    // show memory size
    host_outputFreeMem(sysVARSTART - sysPROGEND);
    host_showBuffer();
    
 
    if (EEPROM.read(0) == MAGIC_AUTORUN_NUMBER)
        autorun = 1;
    else
        host_startupTone();
}

void loop() {
static int debug=0;
    int ret = ERROR_NONE;
  Serial.printf("loop start debug=%d\n",debug++);
    if (!autorun) {
  Serial.printf("loop 1 debug=%d\n",debug++);
        // get a line from the user
        char *input = host_readLine();
        // special editor commands
        if (input[0] == '?' && input[1] == 0) {
  Serial.printf("loop 2 debug=%d\n",debug++);
            host_outputFreeMem(sysVARSTART - sysPROGEND);
            host_showBuffer();
            return;
        }
  Serial.printf("loop 3 debug=%d input=%s\n",debug++,input);
        // otherwise tokenize
        ret = tokenize((unsigned char*)input, tokenBuf, TOKEN_BUF_SIZE);
  Serial.printf("loop 4 debug=%d\n",debug++);
   
    }
    else {
  Serial.printf("loop 5 debug=%d\n",debug++);
        host_loadProgram();
        tokenBuf[0] = TOKEN_RUN;
        tokenBuf[1] = 0;
        autorun = 0;
    }
  Serial.printf("loop 6 debug=%d\n",debug++);
    // execute the token buffer
    if (ret == ERROR_NONE) {
  Serial.printf("loop 7 debug=%d\n",debug++);
      
        host_newLine();
        ret = processInput(tokenBuf);
    }
  Serial.printf("loop 8 debug=%d\n",debug++);
    
    if (ret != ERROR_NONE) {
  Serial.printf("loop 9 debug=%d\n",debug++);

        host_newLine();
  Serial.printf("loop 10 debug=%d\n",debug++);
        
        if (lineNumber !=0) {
            Serial.printf("loop 11 debug=%d\n",debug++);

            host_outputInt(lineNumber);
            host_outputChar('-');
  Serial.printf("loop 12 debug=%d\n",debug++);
        }
          Serial.printf("loop 13 debug=%d\n",debug++);

        host_outputProgMemString((char *)pgm_read_word(&(errorTable[ret])));
          Serial.printf("loop 14 debug=%d\n",debug++);

    }
}

