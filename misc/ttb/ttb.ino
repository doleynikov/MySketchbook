/*
  TOYOSHIKI Tiny BASIC
  (C)2012 Tetsuya Suzuki, All rights reserved.
  You can use, copy, improve, or distribute
  WITHOUT WARRANTY.
  
  Tested in ARDUINO UNO.
  Use UART terminal or serial monitor.
  
  The grammar is the same as
  PALO ALT TinyBASIC by Li-Chen Wang
  Except 2 point to show below.
  
  (1)The contracted form of the description is invalid.
  
  (2)Force abort key
  PALO ALT TinyBASIC -> [Ctrl]+[C]
  TOYOSHIKI TinyBASIC -> [ESC]
  NOTE: There is no input means in serial monitor.
*/

extern "C" {
#include "ttbasic.h"
}

void setup(void)
{
  Serial.begin(9600);
    while(!Serial);
}

void loop(void)
{
  basic();
}
