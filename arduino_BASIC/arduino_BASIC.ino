#include <PS2Keyboard.h>
#include <EEPROM.h>

#include "basic.h"
#include "host.h"

// Keyboard
const int DataPin = 8;
const int IRQpin =  3;
PS2Keyboard keyboard;


// NB Keyboard needs a seperate ground from the OLED

// buzzer pin, 0 = disabled/not present
#define BUZZER_PIN    5

// BASIC
unsigned char mem[MEMORY_SIZE];
#define TOKEN_BUF_SIZE    64
unsigned char tokenBuf[TOKEN_BUF_SIZE];

const char welcomeStr[] PROGMEM = "Arduino BASIC";
char autorun = 0;

void setup() {
    keyboard.begin(DataPin, IRQpin);
    Serial.begin(9600);
Serial.println("Setup start.");        

    reset();
Serial.println("Setup 1.");        
    host_init(BUZZER_PIN);
Serial.println("Setup 2.");        
    host_cls();
Serial.println("Setup 3.");        
    host_outputProgMemString(welcomeStr);
Serial.println("Setup 4.");        
    // show memory size
    host_outputFreeMem(sysVARSTART - sysPROGEND);
Serial.println("Setup 5.");        
    host_showBuffer();
Serial.println("Setup 6.");        
    
    if (EEPROM.read(0) == MAGIC_AUTORUN_NUMBER)
        autorun = 1;
    else
        host_startupTone();

Serial.println("Setup done.");        
}

void loop() {
    int ret = ERROR_NONE;

    if (!autorun) {
Serial.println("Starting main loop.");        

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

