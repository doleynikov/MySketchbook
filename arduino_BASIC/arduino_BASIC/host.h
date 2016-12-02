#include <stdint.h>


#define MAGIC_AUTORUN_NUMBER    0xFC

void host_debug(char* s);

void host_init(int buzzerPin);
void host_sleep(long ms);
void host_digitalWrite(int pin,int state);
int host_digitalRead(int pin);
int host_analogRead(int pin);
void host_pinMode(int pin, int mode);
void host_click();
void host_startupTone();
void host_outputString(char *str);
void host_outputProgMemString(const char *str);
void host_outputChar(char c);
void host_outputFloat(float f);
char *host_floatToStr(float f, char *buf);
int host_outputInt(long val);
void host_newLine();
char *host_readLine();
char host_getKey();
bool host_ESCPressed();
void host_outputFreeMem(unsigned int val);
void host_saveProgram(bool autoexec);
void host_loadProgram();
void host_moveCursor(int first,int second);
void host_cls();
  //unimplemented yet





