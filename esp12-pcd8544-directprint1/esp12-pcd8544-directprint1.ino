#include <PCD85448266.h>

/*
 * PCD8544 - Interface with Philips PCD8544 (or compatible) LCDs.
 *
 * Copyright (c) 2010 Carlos Rodrigues <cefrodrigues@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

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





// A custom glyph (a smiley)...
static const byte glyph[] = { B00010000, B00110100, B00110000, B00110100, B00010000 };


static PCD8544 lcd;


void setup() {
  // PCD8544-compatible displays may have a different resolution...
  lcd.begin(84, 48);

  // Add the smiley to position "0" of the ASCII table...
  lcd.createChar(0, glyph);
}


void loop() {
  // Just to show the program is alive...
  static int counter = 0;

  // Write a piece of text on the first line...
  lcd.setCursor(0, 0);
  lcd.print("Hello, World!");

  // Write the counter on the second line...
  lcd.setCursor(0, 1);
  lcd.print(counter, DEC);
  lcd.write(' ');
  lcd.write(0);  // write the smiley
lcd.println("1 Here is an example of loooonnnng test string to print");
  delay(500);
lcd.println("2 Here is an example of loooonnnng test string to print");
  delay(500);
lcd.println("3 Here is an example of loooonnnng test string to print");
  delay(500);
lcd.println("4 Here is an example of loooonnnng test string to print");
  delay(500);
lcd.println("5 Here is an example of loooonnnng test string to print");
  delay(500);
lcd.println("6 Here is an example of loooonnnng test string to print");
  delay(500);
lcd.println("7 Here is an example of loooonnnng test string to print");
  delay(500);
lcd.println("8 Here is an example of loooonnnng test string to print");
  delay(500);
lcd.println("9 Here is an example of loooonnnng test string to print");
  delay(500);
lcd.println("a Here is an example of loooonnnng test string to print");
  delay(500);
lcd.println("b Here is an example of loooonnnng test string to print");
  delay(500);
lcd.println("c Here is an example of loooonnnng test string to print");

  // Use a potentiometer to set the LCD contrast...
  // short level = map(analogRead(A0), 0, 1023, 0, 127);
  // lcd.setContrast(level);

  delay(1000);
  counter++;

  lcd.clear();
}


/* EOF - HelloWorld.ino */
