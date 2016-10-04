#include <EEPROM.h>

#include <Ticker.h>

#include <PCD85448266.h>

#include "basic.h"
#include "host.h"

Ticker timer;
PCD8544 screen ;

// buzzer pin, 0 = disabled/not present
#define BUZZER_PIN    0

// BASIC
unsigned char mem[MEMORY_SIZE];
#define TOKEN_BUF_SIZE    64
unsigned char tokenBuf[TOKEN_BUF_SIZE];

const char welcomeStr[] PROGMEM = "Arduino BASIC";
char autorun = 0;

void timerIsr()
{
  ESP.wdtFeed();
  Serial.println(".");
  delay(0);
}

void setup() {
  timer.attach(1, timerIsr);
  ESP.wdtDisable();
  ESP.wdtEnable(150000);

  Serial.begin(115200);   // opens serial port, sets data rate to 9600 bps
Serial.println("Begin:");
  screen.begin(84, 48);
  screen.setContrast(60);

  screen.clear();
  screen.setCursor(0, 0);
  screen.println("!");

  reset();
  host_init(BUZZER_PIN);
  host_cls();
  host_outputProgMemString(welcomeStr);
  // show memory size
  host_outputFreeMem(sysVARSTART - sysPROGEND);

  if (EEPROM.read(0) == MAGIC_AUTORUN_NUMBER)
    autorun = 1;
  else
    host_startupTone();
 
}

void loop() {
  int ii=0;
  int ret = ERROR_NONE;
  yield();
  if (!autorun) {
    // get a line from the user
    char *input = host_readLine();
    // special editor commands
    if (input[0] == '?' && input[1] == 0) {
      host_outputFreeMem(sysVARSTART - sysPROGEND);
      return;
    }
    // otherwise tokenize
    Serial.print("INPUT:");
    ii=0;
    while (input[ii]!=0){ Serial.println(input[ii++]);}
    
    ret = tokenize((unsigned char*)input, tokenBuf, TOKEN_BUF_SIZE);
    Serial.println(ret);
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

    Serial.print("NoErrors:");
    ii=0;
    while (!tokenBuf[ii]){ Serial.println(tokenBuf[ii++]);}

    ret = processInput(tokenBuf);
  }
  if (ret != ERROR_NONE) {
    host_newLine();
    if (lineNumber != 0) {
      host_outputInt(lineNumber);
      host_outputChar('-');
    }
    host_outputProgMemString((char *)pgm_read_word(&(errorTable[ret])));
  }
}

