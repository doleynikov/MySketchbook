#include "host.h"
#include "basic.h"
#include <PS2Keyboard.h>
#include <EEPROM.h>
#include <Ticker.h>
#include <PCD85448266.h>
#define KEY_ENTER PS2_ENTER
#define SCREEN_WIDTH        14
#define SCREEN_HEIGHT       6

#define SIZE_LINE 64
#define LCD_W 14
#define LCD_H 6

PCD8544 lcd;
Ticker timer;

int lcdX = 0;
int lcdY = LCD_H-1; // start from the bottom line. Then - scrollUp
char screen[LCD_H][LCD_W];
char lbuf[SIZE_LINE]; //Command line buffer


extern PS2Keyboard keyboard;
extern EEPROMClass EEPROM;
int timer1_counter=0; // time since boot

int curX = 0, curY = 0;
volatile char flash = 0, redraw = 0;
char inputMode = 0;
char inkeyChar = 0;
char buzPin = 0;

const char bytesFreeStr[] PROGMEM = "bytes free";

void timerIsr()
{
  ESP.wdtFeed();
  timer1_counter++;
  delay(0);
}

void initTimer() {
    noInterrupts();           // disable all interrupts
    ESP.wdtDisable();
  timer.attach(1, timerIsr);
    interrupts();             // enable all interrupts
    ESP.wdtEnable(1000);
}


//*****************************************************************************************

void host_moveCursor(int x, int y) {
    if (x<0) x = 0;
    if (x>=SCREEN_WIDTH) x = SCREEN_WIDTH-1;
    if (y<0) y = 0;
    if (y>=SCREEN_HEIGHT) y = SCREEN_HEIGHT-1;
  lcd.setCursor(x, y);
lcdX=x;
lcdY=y;  

}
void host_outchar(char c)
{
  lcd.write(c);
  }

void host_display_clear_rest()
{
  host_moveCursor(lcdX, lcdY);         // cursor to original position
  for (int i = lcdX; i < LCD_W; i++) // clear the rest of line
  {
    host_outchar(' ');
    screen[lcdY][i] = ' ';
  }
  host_moveCursor(lcdX, lcdY);         //return cursor to original position
//  Serial.println("clear rest("+String(lcdX)+","+String(lcdY)+")");
}

void host_display_scroll()
{
  host_moveCursor(0, 0);
  for (int y = 0; y < LCD_H - 1; y++) //move screen one line up
  {
    for (int x = 0; x < LCD_W; x++)
    {
      screen[y][x] = screen[y + 1][x];
      host_outchar(screen[y + 1][x]);
    }
  }
  host_moveCursor(lcdX, lcdY);         //return cursor to original position
  host_display_clear_rest();
//        Serial.println("scroll ("+String(lcdX)+","+String(lcdY)+")");
}

void host_display_write(char c)
{
  if (c != 13 && c != 10) // regular character
  {
    if (lcdX >= LCD_W) // is x-position out of the screen?
    {
      lcdX = 0;
      host_display_scroll();
    }
    host_outchar(c);
    screen[lcdY][lcdX] = c;
    lcdX++;
  }
  else if (c == 8) {
   lcdX--;
  host_moveCursor(lcdX, lcdY);
   
  }//LineFeed - next line

  else if (c == 10) {
    host_display_scroll();
    host_display_clear_rest();
  }//LineFeed - next line
  else if (c == 13) {
    lcdX = 0;
  }//CR - first position of current line
  host_moveCursor(lcdX, lcdY);
}

void host_display_init()
{
  lcd.begin(84, 48);
  lcd.clear();
  host_moveCursor(0, 0);
  
  lcd.setContrast(90);
}

void host_display_cls()
{
  lcd.clear();
  host_moveCursor(0, 0);
lcdX=0;
lcdY=LCD_H-1;  
}

void host_cls() {
host_display_cls(); 
}
//*****************************************************************************************

void host_init(int buzzerPin) {
    buzPin = buzzerPin;
   host_display_init();
    if (buzPin)
        pinMode(buzPin, OUTPUT);
    initTimer();
}

void host_sleep(long ms) {
    delay(ms);
}

void host_digitalWrite(int pin,int state) {
    digitalWrite(pin, state ? HIGH : LOW);
}

int host_digitalRead(int pin) {
    return digitalRead(pin);
}

int host_analogRead(int pin) {
    return analogRead(pin);
}

void host_pinMode(int pin,int mode) {
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
    for (int i=1; i<=2; i++) {
        for (int j=0; j<50*i; j++) {
            digitalWrite(buzPin, HIGH);
            delay(3-i);
            digitalWrite(buzPin, LOW);
            delay(3-i);
        }
        delay(100);
    }    
}

void host_outputString(char *str) {
    while (*str) {
        host_outputChar(*str++);
    }
}

void host_outputProgMemString(const char *p) {
    while (1) {
        unsigned char c = pgm_read_byte(p++);
        if (c == 0) break;
        host_outputChar(c);
    }
}

void host_outputChar(char c) {
host_display_write(c);
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

    for (int i=0; i<c; i++) {
        xx /= 10;
        char digit = ((num/xx) % 10) + '0';
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
    else if (a<0.0001 || a>1000000) {
        // this will output -1.123456E99 = 13 characters max including trailing nul
        dtostrf(f,  0,6,buf);
    }
    else {
        int decPos = 7 - (int)(floor(log10(a))+1.0f);
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
host_outputChar(13);
host_outputChar(10);
}
int host_getch() {
  delay(1);  //Soft WDT対策
  return Serial.read();
}
int host_kbhit() {
  delay(1);  //Soft WDT対策
  return Serial.available();
}
char c_isprint(char c) {
  return (c >= 32 && c <= 126);
}
char c_isspace(char c) {
  return (c == ' ' || (c <= 13 && c >= 9));
}
char *host_readLine() {
  char c;  
  unsigned char len; 
  len = 0;  
  while ((c = host_getch()) != KEY_ENTER) { 
    if (c == 9) c = ' '; 
    if (((c == 8) || (c == 127)) && (len > 0)) {
      len--; 
      host_display_write(8); host_display_write(' '); host_display_write(8); 
    } else
      if (c_isprint(c) && (len < (SIZE_LINE - 1))) {
        lbuf[len++] = c; 
        host_display_write(c); 
      }
    delay(1);
  }
  host_newLine();  
  lbuf[len] = 0;  

  if (len > 0) {  
    while (c_isspace(lbuf[--len]));    lbuf[++len] = 0;  
  }
  Serial.println(lbuf);
    return &lbuf[0];
}

char host_getKey() {
    return host_getch();
}

bool host_ESCPressed() {
    while (host_kbhit()) {
        // read the next key
        inkeyChar = host_getKey();
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
    EEPROM.write(0, autoexec ? MAGIC_AUTORUN_NUMBER : 0x00);
    EEPROM.write(1, sysPROGEND & 0xFF);
    EEPROM.write(2, (sysPROGEND >> 8) & 0xFF);
    for (int i=0; i<sysPROGEND; i++)
        EEPROM.write(3+i, mem[i]);
}

void host_loadProgram() {
    // skip the autorun byte
    sysPROGEND = EEPROM.read(1) | (EEPROM.read(2) << 8);
    for (int i=0; i<sysPROGEND; i++)
        mem[i] = EEPROM.read(i+3);
}

void host_showBuffer(){}

