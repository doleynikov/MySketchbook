#include "host.h"
#include "basic.h"
#include <pgmspace.h>
#include <Arduino.h>
#define echo 1

char screenBuffer[SCREEN_WIDTH * SCREEN_HEIGHT];
char lineDirty[SCREEN_HEIGHT];
char lineBuffer[64];
int curX = 0, curY = 0;
volatile char flash = 0, redraw = 0;
char inputMode = 0;
char inkeyChar = 0;
char buzPin = 0;

const char bytesFreeStr[] PROGMEM = "bytes free";

void host_init(int buzzerPin) {
  buzPin = buzzerPin;
  if (buzPin)
    pinMode(buzPin, OUTPUT);
}

void host_sleep(long ms) {
  delay(ms);
}

void host_digitalWrite(int pin, int state) {
  digitalWrite(pin, state ? HIGH : LOW);
}

int host_digitalRead(int pin) {
  return digitalRead(pin);
}

int host_analogRead(int pin) {
  return analogRead(pin);
}

void host_pinMode(int pin, int mode) {
  pinMode(pin, mode);
}

void host_click() {
  if (!buzPin) return;
  digitalWrite(buzPin, HIGH);
  delay(1);
  digitalWrite(buzPin, LOW);
}

void host_startupTone() {
  if (!buzPin) return;
  for (int i = 1; i <= 2; i++) {
    for (int j = 0; j < 50 * i; j++) {
      digitalWrite(buzPin, HIGH);
      delay(3 - i);
      digitalWrite(buzPin, LOW);
      delay(3 - i);
    }
    delay(100);
  }
}

//display module (display and serial)

void host_cls() {
 // Serial.println("CLS:");
}

void host_moveCursor(int x, int y) {
 // Serial.println("host_moveCursor");
}

void host_outputString(char *str) {
  while (*str) {
    host_outputChar(*str++);
  }
}

void host_outputChar(char c) {
  Serial.write(c);
}

void host_outputProgMemString(const char *p) {
  while (1) {
    unsigned char c = pgm_read_byte(p++);
    if (c == 0) break;
    host_outputChar(c);
  }
}

int host_outputInt(long num) {
  // returns len
  long i = num, xx = 1;
  int c = 0;

  do {
    c++;
    xx *= 10;
    i /= 10;
  }
  while (i);

  for (int i = 0; i < c; i++) {
    xx /= 10;
    char digit = ((num / xx) % 10) + '0';
    host_outputChar(digit);
  }
  return c;
}

char *host_floatToStr(float f, char *buf) {
  // floats have approx 7 sig figs
  float a = fabs(f);
  if (f == 0.0f) {
    buf[0] = '0';
    buf[1] = 0;
  }
  else if (a < 0.0001 || a > 1000000) {
    // this will output -1.123456E99 = 13 characters max including trailing nul
    //        dtostre(f, buf, 6, 0);
    dtostrf(f, 0, 6, buf);
  }
  else {
    int decPos = 7 - (int)(floor(log10(a)) + 1.0f);
    dtostrf(f, 1, decPos, buf);
    if (decPos) {
      // remove trailing 0s
      char *p = buf;
      while (*p) p++;
      p--;
      while (*p == '0') {
        *p-- = 0;
      }
      if (*p == '.') *p = 0;
    }
  }
  return buf;
}

void host_outputFloat(float f) {
  char buf[16];
  host_outputString(host_floatToStr(f, buf));
}

void host_newLine() {
  host_outputChar('\r');
  host_outputChar('\n');
}

//чтение символов

char *host_readLine() {
  inputMode = 1;

  host_outputChar('>');

  int pos = 0;
  bool done = false;
  while (!done) {
    while (Serial.available()) {
      host_click();
      // read the next key
      char c = Serial.read();
      if (c >= 32 && c <= 126)
      { lineBuffer[pos++] = c;
        host_outputChar(c);
      }
      else if (c == PS2_ENTER)
      { done = true;
        host_newLine();
        lineBuffer[pos] = 0;
      }
    }
  }
  lineBuffer[pos] = 0;
  inputMode = 0;
  return &lineBuffer[0];
}

char host_getKey() {
  char c = inkeyChar;
  inkeyChar = 0;
  if (c >= 32 && c <= 126)
    return c;
  else return 0;
}

bool host_ESCPressed() {
  while (Serial.available()) {
    // read the next key
    inkeyChar = Serial.read();
    if (inkeyChar == PS2_ESC)
      return true;
  }
  return false;
}

void host_outputFreeMem(unsigned int val)
{
  host_newLine();
  host_outputInt(val);
  host_outputChar(' ');
  host_outputProgMemString(bytesFreeStr);
}

void host_saveProgram(bool autoexec) {
  /*    EEPROM.write(0, autoexec ? MAGIC_AUTORUN_NUMBER : 0x00);
      EEPROM.write(1, sysPROGEND & 0xFF);
      EEPROM.write(2, (sysPROGEND >> 8) & 0xFF);
      for (int i=0; i<sysPROGEND; i++)
          EEPROM.write(3+i, mem[i]);
  */
}

void host_loadProgram() {
  /*    // skip the autorun byte
      sysPROGEND = EEPROM.read(1) | (EEPROM.read(2) << 8);
      for (int i=0; i<sysPROGEND; i++)
          mem[i] = EEPROM.read(i+3);
  */
}


