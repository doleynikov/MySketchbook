
// C64 Emulator, works with pic32MX and pic32MZ (SDZL board) under UECIDE
// Pito's version, July 2014
// http://forum.arduino.cc/index.php?topic=193216.msg1793065#msg1793065
// Caps Lock must be ON!
// Use terminal e.g. Teraterm 40x25 char

//#include <ESP8266WiFi.h>


uint8_t curkey = 0;

uint8_t VRAM[1000];


//extern "C" {
  #define RAM_SIZE 30816 //SOMEWHAT LESS THAN 32kB
	uint8_t RAM[RAM_SIZE];
  
	uint16_t getpc();
	uint8_t getop();
	void exec6502(int32_t tickcount);
	void reset6502();
	void serout(uint8_t val) {
		Serial.write(val);
	}
	uint8_t getkey() {
		return(curkey);
	}
	void clearkey() {
		curkey = 0;
	}
	void printhex(uint16_t val) {
		Serial.print(val, HEX);
		Serial.println();
	}
//}

void VTposition(uint8_t row, uint8_t col) {
	  Serial.write(27);
    Serial.write('[');
    Serial.print(row+1);
    Serial.write(';');
    Serial.print(col+1);
    Serial.write('H');
}

#include <Ticker.h>
Ticker timer;
void timerIsr()
{
  ESP.wdtFeed();
  delay(0);
}

void setup () {
  //WiFi.mode(WIFI_OFF);
  timer.attach(1, timerIsr);
  ESP.wdtDisable();
  ESP.wdtEnable(150000);
	Serial.begin (115200);
  Serial.setDebugOutput(true);
  delay(10000);//needed to let user open terminal/monitor
	
	Serial.println ("Prep VRAM..");

	for (int i=0; i<1000; i++){
		VRAM[i] = RAM[i+1024];
	}

 Serial.println ("Init CPU..");
 reset6502();
 Serial.println ("Starting CPU..");
 delay(1000);
  
}

int counter = 1;
int effc = 1;

void loop () {
  
	uint16_t v_address = 0;
	uint16_t last_v_address = 0;
	int eff = 0;

    uint8_t col_c = 0;

    Serial.print("\x1b[H");

	for (uint8_t row=0; row<25; row++) {
		
		for (uint8_t col=0; col<40; col++) {

			if (Serial.available()) {
				curkey = Serial.read() & 0x7F;
				RAM[198] = 1;
				RAM[631] = curkey; 
			}

			exec6502(100);

			uint8_t petscii = RAM[v_address + 1024];

			if (VRAM[v_address] != petscii) {

				VRAM[v_address] = petscii;

				if (((v_address - last_v_address) > 1) 
				|| (col_c >= 40)) { 

				VTposition(row, col);
				col_c = col;
				}	

				if (petscii<32) petscii = petscii + 64;
								
				Serial.write(petscii);

				last_v_address = v_address;
					
				}
				
			col_c++;
			v_address++;
		}

	}
	
}

/*
uint8_t curkey = 0;

//extern "C" {
uint16_t getpc();
uint8_t getop();
void exec6502(int32_t tickcount);
void reset6502();

void serout(uint8_t val) {
  Serial.write(val);
}

uint8_t getkey() {
  return(curkey);
}

void clearkey() {
  curkey = 0;
}

void printhex(uint16_t val) {
  Serial.print(val, HEX);
  Serial.println();
}
//}

void setup () {
  Serial.begin (115200);
  Serial.println ();
  Serial.println("Press key to starting 6502 CPU...");
  reset6502();
  while (!Serial.available()) {
    delay(100);
  }  
}

void loop () {
  exec6502(100); //if timing is enabled, this value is in 6502 clock ticks. otherwise, simply instruction count.
  if (Serial.available()) {
    curkey = Serial.read() & 0x7F;
    //Serial.println("receive key");
  }  
}
*/
