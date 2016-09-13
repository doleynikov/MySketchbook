Components: i2c 1602 display, ds1307 rtc, ESP01, power supply GND +5v +3.3v

  Wire.begin(0, 2); //i2c init
  
  For now it is somekind of compilation of different examples from internet and arduino library.
  
  TODO:
  *)add control buttons 
  *)allow periodical NTP calls
  *)if NTP successfull - show some icon
  *)turn off WIFI between NTP calls
  *)allow editing (from serial of PS/2 keyboard?) of WIFI name, username, password, ntp server and so on
  *) if no WIFI available - just get time from RTC
  
