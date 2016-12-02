// Terminal control
#include <PCD85448266.h>

#include <Ticker.h>

Ticker timer;

char inkey=0;

uint32_t timer1_counter=0;

#define LCD_W 14
#define LCD_H 6
PCD8544 lcd;
int lcdX = 0;
int lcdY = LCD_H-1; // start from the bottom line. Then - scrollUp
char screen[LCD_H][LCD_W];

void host_display_init()
{
  lcd.begin(84, 48);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.setContrast(90);
}

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

void host_init()
{
  initTimer();
  host_display_init();

}

void host_outchar(char c)
{
  lcd.write(c);
  }

void host_display_clear_rest()
{
  lcd.setCursor(lcdX, lcdY);         // cursor to original position
  for (int i = lcdX; i < LCD_W; i++) // clear the rest of line
  {
    host_outchar(' ');
    screen[lcdY][i] = ' ';
  }
  lcd.setCursor(lcdX, lcdY);         //return cursor to original position
//  Serial.println("clear rest("+String(lcdX)+","+String(lcdY)+")");
}

void host_display_scroll()
{
  lcd.setCursor(0, 0);
  for (int y = 0; y < LCD_H - 1; y++) //move screen one line up
  {
    for (int x = 0; x < LCD_W; x++)
    {
      screen[y][x] = screen[y + 1][x];
      host_outchar(screen[y + 1][x]);
    }
  }
  lcd.setCursor(lcdX, lcdY);         //return cursor to original position
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

  else if (c == 10) {
    host_display_scroll();
 //     Serial.println("LF ("+String(lcdX)+","+String(lcdY)+")");
    host_display_clear_rest();
  }//LineFeed - next line
  else if (c == 13) {
    lcdX = 0;
 //     Serial.println("CR ("+String(lcdX)+","+String(lcdY)+")");
  }//CR - first position of current line
  lcd.setCursor(lcdX, lcdY);
}

void host_display_cls()
{
  lcd.clear();
  lcd.setCursor(0, 0);
lcdX=0;
lcdY=LCD_H-1;  
}
int host_getch() {
  delay(1);  //Soft WDT対策
  return Serial.read();
}
int host_kbhit() {
  delay(1);  //Soft WDT対策
  return Serial.available();
}

