#include <EEPROM.h>

#include "basic.h"
#include "host.h"


// buzzer pin, 0 = disabled/not present
#define BUZZER_PIN    5

// BASIC
unsigned char mem[MEMORY_SIZE];
#define TOKEN_BUF_SIZE    64
unsigned char tokenBuf[TOKEN_BUF_SIZE];

const char welcomeStr[] PROGMEM = "Arduino BASIC";
char autorun = 0;

void setup() {
    Serial.begin(9600);
    reset();
    host_init(BUZZER_PIN);
    host_outputProgMemString(welcomeStr);
    // show memory size
    host_outputFreeMem(sysVARSTART - sysPROGEND);
        host_startupTone();
}

void loop() {
    int ret = ERROR_NONE;

       // get a line from the user
        char *input = host_readLine();
        // special editor commands
        if (input[0] == '?' && input[1] == 0) {
            host_outputFreeMem(sysVARSTART - sysPROGEND);
            return;
        }
        // otherwise tokenize
        ret = tokenize((unsigned char*)input, tokenBuf, TOKEN_BUF_SIZE);
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
        host_outputProgMemString((char *)(&(errorTable[ret])));
    }
}

