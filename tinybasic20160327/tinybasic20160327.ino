
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#define printByte(args)  write(args);
#include "RTClib.h"
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>





/***************************************************************************/
#define EESIZE 510
#define SCREEN_W	80
#define SCREEN_H	48
// TinyBASIC.cpp : An implementation of TinyBASIC in C
// Original Author : Mike Field - hamster@snap.net.nz
// This Version: George Gray - halfbyteblog.wordpress.com
// Based on TinyBasic for 68000, by Gordon Brandly (see http://members.shaw.ca/gbrandly/68ktinyb.html)
// which itself was Derived from Palo Alto Tiny BASIC as  published in the May 1976 issue of Dr. Dobb's Journal.  
// 0.03 21/01/2011 : Added INPUT routine M
//                 : Reorganised memory layout
//                 : Expanded all error messages
//                 : Break key added
//                 : Removed the calls to printf (left by debugging)
//2013: began working on modifying for console-George Gray, HalfByte
//2014: heavily modified for HalfByte Console.
//added graphics statements
//added sound
//fixed POKE
//added SERIAL and PIN access
//added POLY and CROSSHAIRS
//Enhanced LIST
//added ARC to draw arcs, pie chart pieces
//2016: modified by KOYAMA

#undef  PROGMEM
#define PROGMEM __attribute__((section(".progmem.vars")))

#include <EEPROM.h>
#include <math.h>

//int outswitch=true;
int stopFlag=false;

#ifndef ARDUINO
#include "stdafx.h"
#include <conio.h>
#endif 

// ASCII Characters
#define CR		'\r'
#define NL		'\n'
#define TAB		'\t'
#define BELL	'\b'
#define DEL		'\127'
#define BS		0x08
#define ESC		0x1B
#define SPACE	' '
#define CTRLC	0x19
#define CTRLH	0x7F
#define CTRLS	0x13
#define CTRLX	0x18

const double PI_180=0.017453278;

typedef short unsigned LINENUM;

/***********************************************************/
// Keyword table and constants - the last character has 0x80 added to it
static unsigned char __attribute__((section(".progmem.data"))) keywords[]={
  'L','I','S','T'+0x80, 'L','O','A','D'+0x80, 'N','E','W'+0x80, 'R','U','N'+0x80,
  'S','A','V','E'+0x80, 'N','E','X','T'+0x80, 'L','E','T'+0x80,	 'I','F'+0x80, 'G','O','T','O'+0x80,
  'G','O','S','U','B'+0x80,	'R','E','T','U','R','N'+0x80,	'R','E','M'+0x80, 'F','O','R'+0x80,
  'I','N','P','U','T'+0x80,	'P','R','I','N','T'+0x80, 'P','O','K','E'+0x80, 'S','T','O','P'+0x80,
  '@'+0x80,	'?'+0x80, 'A','W','R','I','T','E'+0x80, 'D','W','R','I','T','E'+0x80,
  'M','E','M'+0x80, 'T','O','N','E'+0x80, 'D','E','L','A','Y'+0x80, 'C','L','E','A','R'+0x80, '#'+0x80,
  0
};
enum {
  KW_LIST=0, KW_LOAD, KW_NEW, KW_RUN,
  KW_SAVE ,KW_NEXT, KW_LET, KW_IF, KW_GOTO,
  KW_GOSUB, KW_RETURN, KW_REM, KW_FOR,
  KW_INPUT, KW_PRINT, KW_POKE, KW_STOP,
  KW_AT, KW_QMARK, KW_AWRITE, KW_DWRITE,
  KW_MEM, KW_TONE, KW_DELAY, KW_CLEAR, KW_HASHTAG,
  KW_DEFAULT};

struct stack_for_frame{
  char frame_type, for_var;
  short int terminal, step;
  unsigned char *current_line, *txtpos;
};

struct stack_gosub_frame{
  char frame_type;
  unsigned char *current_line, *txtpos;
};

static unsigned char __attribute__((section(".progmem.data"))) func_tab[]={
  'P','E','E','K'+0x80, 'A','B','S'+0x80, 'R','N','D'+0x80, 'C','H','R'+0x80,
  '@'+0x80, 'M','E','M'+0x80, 'T','O','P'+0x80, 'A','R','E','A','D'+0x80,
  'D','R','E','A','D'+0x80, 'S','I','N'+0x80, 'C','O','S'+0x80,
  'I','N','K','E','Y'+0x80, 'I','N'+0x80, 
  0
};

#define FUNC_PEEK  	0
#define FUNC_ABS  	1
#define FUNC_RND 	2
#define FUNC_CHR	3
#define FUNC_AT		4
#define FUNC_MEM	5
#define FUNC_TOP    6
#define FUNC_AREAD  7
#define FUNC_DREAD  8
#define FUNC_SIN    9
#define FUNC_COS    10
#define FUNC_INKEY  11
#define FUNC_UARTIN  12

#define FUNC_UNKNOWN 13

static unsigned char __attribute__((section(".progmem.data"))) to_tab[]={
  'T','O'+0x80,  0
};

static unsigned char __attribute__((section(".progmem.data"))) step_tab[]  {
  'S','T','E','P'+0x80,  0
};

static unsigned char __attribute__((section(".progmem.data"))) relop_tab[]={
  '>','='+0x80, '<','>'+0x80, '>'+0x80, '='+0x80, '<','='+0x80, '<'+0x80, '!','='+0x80, 0
};

#define RELOP_GE		0
#define RELOP_NE		1
#define RELOP_GT		2
#define RELOP_EQ		3
#define RELOP_LE		4
#define RELOP_LT		5
#define RELOP_NEX		6
#define RELOP_UNKNOWN	7

static unsigned char __attribute__((section(".progmem.data"))) highlow_tab[]={
  'H','I','G','H'+0x80,	'H','I'+0x80, 'L','O','W'+0x80, 'L','O'+0x80, 0
};

#define HIGHLOW_HIGH    1
#define HIGHLOW_UNKNOWN 4
#define STACK_SIZE (sizeof(struct stack_for_frame)*5)
#define VAR_SIZE sizeof(short int) // Size of variables in bytes

static unsigned char memory[EESIZE-2+27*VAR_SIZE+STACK_SIZE];
static unsigned char *txtpos, *list_line, expression_error, *tempsp;
static unsigned char *stack_limit, *program_start, *program_end;
static unsigned char *stack;	// Software stack for things that should go on the CPU stack
static unsigned char *variables_table, *current_line, *spt;
#define STACK_GOSUB_FLAG 'G'
#define STACK_FOR_FLAG 'F'
static unsigned char table_index;
static LINENUM linenum;

static const unsigned char okmsg[]				PROGMEM= "Ready";
static const unsigned char badlinemsg[]			PROGMEM= "Bad line #";
static const unsigned char invalidexprmsg[]     PROGMEM= "Expr error";
static const unsigned char syntaxmsg[]          PROGMEM= "Syntax error";
static const unsigned char badinputmsg[]        PROGMEM= "\nBad Number";
static const unsigned char nomemmsg[]	        PROGMEM= "No memory!";
static const unsigned char initmsg[]	        PROGMEM= "HalfByte TinyBasic";
static const unsigned char memorymsg[]	        PROGMEM= " bytes free.";
static const unsigned char breakmsg[]	        PROGMEM= "Break!";
static const unsigned char stackstuffedmsg[]    PROGMEM= "Stack!\n";
static const unsigned char unimplimentedmsg[]	PROGMEM= "Not yet";
static const unsigned char backspacemsg[]		PROGMEM= "\b \b";
static const unsigned char hitkeymsg[]			PROGMEM= "Hit any key!";
static const unsigned char autorunmsg[]			PROGMEM= "Program is running...";

static int inchar(void);
static void outchar(unsigned char c);
static void line_terminator(void);
static short int expression(void);
static unsigned char breakcheck(void);
/***************************************************************************/
static void ignore_blanks(void){ while(*txtpos==SPACE || *txtpos==TAB)  txtpos++; }
/***************************************************************************/
static void scantable(unsigned char *table){
  int i=0;
  ignore_blanks();
  table_index=0;
  while(1){
    if(pgm_read_byte(table)==0)  return;	// Run out of table entries?
    if(txtpos[i]==pgm_read_byte(table)){	// Do we match this character?
      i++;	table++;
    }else{	// do we match the last character of keywork (with 0x80 added)? If so, return
      if(txtpos[i]+0x80==pgm_read_byte(table)){
        txtpos += i+1;  // Advance the pointer to following the keyword
        ignore_blanks();
        return;
      }
      // Forward to the end of this keyword
      while((pgm_read_byte(table) & 0x80)==0)  table++;
      // Now move on to the first character of the next word, and reset the position index
      table++;	table_index++;
      i=0;
    }
  }
}
/***************************************************************************/
static void pushb(unsigned char b){	spt--;	*spt=b;	}
/***************************************************************************/
static unsigned char popb(){ unsigned char b=*spt; spt++; return b;}
/***************************************************************************/
static void printnum(int num){
  int digits=0;
  if(num<0){	num=-num;	outchar('-');	}
  do{ pushb(num%10+'0'); num=num/10; digits++; }while(num>0);
  while(digits>0){  outchar(popb());  digits--; }
}
/***************************************************************************/
static unsigned short testnum(void){			// return line number
  unsigned short num=0;
  ignore_blanks();
  for(;*txtpos>='0' && *txtpos<='9'; txtpos++){
    if(num>=0xFFFF/10){	num=0xFFFF;	break;	}	// Trap overflows
    num=num*10+*txtpos-'0';
  }
  return	num;
}
/***************************************************************************/
unsigned char check_statement_end(void){ ignore_blanks(); return (*txtpos==NL) || (*txtpos==':');}
/***************************************************************************/
void printmsgNoNL(const unsigned char *msg){ while(pgm_read_byte(msg)!=0)  outchar(pgm_read_byte(msg++));}
/***************************************************************************/
static unsigned char print_quoted_string(void){
  unsigned char delim=*txtpos;
  if(delim!='"' && delim!='\'')  return 0;
  txtpos++;
  for(int i=0;txtpos[i]!=delim;i++) if(txtpos[i]==NL) return 0;// Check we have a closing delimiter
  for(;*txtpos!=delim; txtpos++) outchar(*txtpos);	// Print the characters
  txtpos++; ignore_blanks();						// Skip over the last delimiter
  return 1;
}
/***************************************************************************/
static void printmsg(const unsigned char *msg){	printmsgNoNL(msg);	line_terminator();	}
/***************************************************************************/
unsigned char getln(char prompt){
  outchar(prompt);
  txtpos=program_end+sizeof(LINENUM);
  while(1){
    char c=inchar();
    switch(c){
    case CR:
    case NL:
      line_terminator();
      txtpos[0]=NL;		// Terminate all strings with a NL
      return 1;
    case CTRLC:	return 0;
    case BS:
    case CTRLH:
      if(txtpos==program_end)  break;
      txtpos--;
      printmsgNoNL(backspacemsg);
      break;
    default:
      // We need to leave at least one space to allow us to shuffle the line into order
      if(txtpos==spt-2)	outchar(BELL);
      else{txtpos[0]=c;	txtpos++;	outchar(c); }
    }
  }
}
/***************************************************************************/
static unsigned char *findline(void){
  unsigned char *line=program_start;
  while(1){
    if(line==program_end)  return line;
    if(((LINENUM *)line)[0]>=linenum)  return line;
    // Add the line length onto the current address, to get to the next line;
    line+=line[sizeof(LINENUM)];
  }
}
/***************************************************************************/
static void toUppercaseBuffer(void){
  unsigned char *c=program_end+sizeof(LINENUM);
  unsigned char quote=0;
  while(*c!=NL){
    if(*c==quote)  quote=0;    // Are we in a quoted string?
    else if(*c=='"' || *c=='\'')  quote=*c;
    else if(quote==0 && *c>='a' && *c<='z')  *c=*c+'A'-'a';
    c++;
  }
}
/***************************************************************************/
void printline(){
  LINENUM line_num;
  line_num=*((LINENUM *)(list_line));
  list_line+=sizeof(LINENUM)+sizeof(char);
  printnum(line_num);  // Output the line
  outchar(' ');
  while(*list_line!=NL){  outchar(*list_line);  list_line++; }
  list_line++;
  line_terminator();
}
/***************************************************************************/
static short int expr4(void){
  short int a=0, b=0;
  // fix provided by Jurg Wullschleger wullschleger@gmail.com
  ignore_blanks();	// fixes whitespace and unary operations
  if(*txtpos=='-'){  txtpos++; return -expr4();  }
  if(*txtpos=='!'){  txtpos++; return !expr4();  }
  if(*txtpos=='0'){	txtpos++;	a=0;	goto success;	}	// end fix
  if(*txtpos>='1' && *txtpos<='9'){
    do{	a=a*10+*txtpos-'0';	txtpos++; }while(*txtpos>='0' && *txtpos<='9');
    goto success;
  }
  scantable(highlow_tab);
  if(table_index!=HIGHLOW_UNKNOWN){
    a=(table_index<=HIGHLOW_HIGH)?1:0;
    goto success;
  }
  if(txtpos[0]>='A' && txtpos[0]<='Z'){	// Is it a function or variable reference?
    if(txtpos[1]<'A' || txtpos[1]>'Z'){	// Is it a variable reference (single alpha)
      a=((short int *)variables_table)[*txtpos-'A'];
      txtpos++;
      return a;
    }
    // Is it a function with a single parameter
    scantable(func_tab);
    if(table_index==FUNC_UNKNOWN)  goto expr4_error;
    unsigned char f=table_index;
    if(*txtpos!='(')	goto expr4_error;
    txtpos++;
    a=expression();
    ignore_blanks();
    if(*txtpos!=','){    // check for a comma
      ignore_blanks;	if(*txtpos!=')')  goto expr4_error;
    }else {
      txtpos++; b=expression();
      ignore_blanks;	if(*txtpos!=')')  goto expr4_error;
    }
    txtpos++;
    switch(f){
    case FUNC_AT:
    case FUNC_PEEK:	return memory[a];
    case FUNC_ABS:	return (a>=0? a: -a);
    case FUNC_RND:
      int tempR;
      tempR=random(a);      // generate random number between 0 and x
	  return (tempR>=0? tempR: -tempR);
//    case FUNC_TOP:  return (short int)program_end;
    case FUNC_TOP:  return (uintptr_t)program_end;
    case FUNC_CHR:			// output ascii
        if(a<256){	outchar(a);	expression_error=15;	}
        goto success;
    case FUNC_AREAD:		return analogRead(0);
    /*There’s only one analog input pin, labeled ADC. To read the ADC pin, make a function call to analogRead(A0). 
    Remember that this pin has a weird maximum voltage of 1V – you’ll get a 10-bit value (0-1023) proportional to a voltage between 0 and 1V.*/
    case FUNC_DREAD:	pinMode(a, INPUT_PULLUP);	return digitalRead(a);
    case FUNC_MEM:	return stack_limit-program_end;		// Return memory remaining
    case FUNC_SIN:	return sin(a*PI_180)*1000;	// returns (value*10^3) for integer eval
    case FUNC_COS:	return cos(a*PI_180)*1000;	// returns (value*10^3) for integer eval
    case FUNC_INKEY:return Serial.read();// grab a key from keyboard, if there
    case FUNC_UARTIN:	return Serial.read();	// get a byte of serial
    }
  }
  if(*txtpos=='('){
    txtpos++; a=expression();	if(*txtpos!=')')  goto expr4_error;
    txtpos++; goto success;
  }
expr4_error:	expression_error=1;
success:		ignore_blanks();	return a;
}
/***************************************************************************/
static short int expr3(void){
  short int a, b;
  a=expr4();
  while(1){
    if(*txtpos=='*'){	  txtpos++;	b=expr4(); a*=b;}
    else if(*txtpos=='/'){txtpos++;	b=expr4(); if(b!=0) a/=b; else expression_error=1;}
    else if(*txtpos=='%'){txtpos++; b=expr4(); if(b!=0) a%=b; else expression_error=1;}
    else if(*txtpos=='&'){txtpos++;	b=expr4(); if(b!=0) a&=b; else expression_error=1;}
    else if(*txtpos=='|'){txtpos++;	b=expr4(); if(b!=0) a|=b; else expression_error=1;}
    else if(*txtpos=='^'){txtpos++;	b=expr4(); if(b!=0) return pow(a,b);}
    else  return a;
  }
}
/***************************************************************************/
static short int expr2(void){
  short int a, b;
  if(*txtpos=='-' || *txtpos=='+')    a=0;
  else    a=expr3();
  while(1){
    if(*txtpos=='-'){		txtpos++;	b=expr3();	a-=b;}
    else if(*txtpos=='+'){	txtpos++;	b=expr3();	a+=b;}
    else  return a;
  }
}
/***************************************************************************/
static short int expression(void){
  short int a, b;
  a=expr2();
  if(expression_error)	return a;
  scantable(relop_tab);
  if(table_index==RELOP_UNKNOWN)  return a;
  switch(table_index){
	case RELOP_GE:	b=expr2();	if(a>=b) return 1;	break;
	case RELOP_NEX:
	case RELOP_NE:	b=expr2();	if(a!=b) return 1;	break;
	case RELOP_GT:	b=expr2();	if(a>b)  return 1;	break;
	case RELOP_EQ:	b=expr2();	if(a==b) return 1;	break;
	case RELOP_LE:	b=expr2();	if(a<=b) return 1;	break;
	case RELOP_LT:	b=expr2();	if(a<b)  return 1;	break;
  }
  return 0;
}
/***************************************************************************/
void loop(){
  unsigned char *start, *newEnd, linelen;
  int clr_count;
  boolean isDigital;
  variables_table=memory;
  program_start=memory + 27*VAR_SIZE;
  program_end=program_start;
  spt=memory+sizeof(memory);
  stack_limit=spt-STACK_SIZE;
   Serial.println("hello!");

  printmsg(initmsg);
  {								// Auto load
  int size=(EEPROM.read(EESIZE-2))+(EEPROM.read(EESIZE-1)<<8);
  if(size<=EESIZE-2){
    program_end=program_start+size;
    for(int i=0; i<size; i++)	memory[i+27*VAR_SIZE]=EEPROM.read(i);
  }
  }
  printnum(stack_limit-program_end); printmsg(memorymsg);
  printmsg(hitkeymsg);
  for(int i=0; i<100; i++){		// Auto run after 3sec 
	if(Serial.available()){ Serial.read();	goto warmstart;}
	delay(30);
  }
  printmsg(autorunmsg);
  current_line=program_start;	goto execline;

warmstart:  // this signifies that it is running in 'direct' mode.
  current_line=0;
  spt=memory+sizeof(memory);
  printmsg(okmsg);
prompt:
  while(!getln('>'))  line_terminator();
  toUppercaseBuffer();
  txtpos=program_end+sizeof(unsigned short);
  while(*txtpos!=NL) txtpos++;  // Find the end of the freshly entered line
  {								// Move it to the end of program_memory
    unsigned char *dest;
    for(dest=spt-1; ; dest--){
      *dest=*txtpos;
      if(txtpos==program_end+sizeof(unsigned short))  break;
      txtpos--;
    }
    txtpos=dest;
  }
  // Now see if we have a line number
  linenum=testnum();
  ignore_blanks();	if(linenum==0)  goto direct;
  if(linenum==0xFFFF)  goto badline;
  // Find the length of what is left, including the (yet-to-be-populated) line header
  for(linelen=0; txtpos[linelen]!=NL; linelen++);
  linelen++; // Include the NL in the line length
  linelen+=sizeof(unsigned short)+sizeof(char); // Add space for the line number and line length
  // Now we have the number, add the line header.
  txtpos-=3;
  *((unsigned short *)txtpos)=linenum;
  txtpos[sizeof(LINENUM)]=linelen;
  // Merge it into the rest of the program
  start=findline();
  // If a line with that number exists, then remove it
  if(start!=program_end && *((LINENUM *)start)==linenum){
    unsigned char *dest, *from;
    unsigned tomove;
    from=start+start[sizeof(LINENUM)];
    dest=start;
    for(tomove=program_end-from; tomove>0; tomove--){ *dest=*from;	from++;	dest++; }
    program_end=dest;
  }
  if(txtpos[sizeof(LINENUM)+sizeof(char)]==NL) goto prompt;	// If the line has no txt, it was just a delete
  // Make room for the new line, either all in one hit or lots of little shuffles
  while(linelen>0){	
    unsigned int tomove;
    unsigned char *from,*dest;
    unsigned int space_to_make=txtpos-program_end;
    if(space_to_make>linelen)  space_to_make=linelen;
    newEnd=program_end+space_to_make;
    // Source and destination - as these areas may overlap we need to move bottom up
    from=program_end;	dest=newEnd;
    for(tomove=program_end-start; tomove>0; tomove--){ from--;	dest--;	*dest=*from;}
    // Copy over the bytes into the new space
    for(tomove=0; tomove<space_to_make; tomove++){ *start=*txtpos; txtpos++; start++; linelen--;}
    program_end=newEnd;
  }
  goto prompt;
unimplemented:	printmsg(unimplimentedmsg);	goto prompt;
badline:		printmsg(badlinemsg);		goto prompt;
invalidexpr:	printmsg(invalidexprmsg);	goto prompt;
syntaxerror:
  printmsg(syntaxmsg);
  if(current_line!=(void *)0){
    unsigned char tmp=*txtpos;
    if(*txtpos!=NL)  *txtpos='^';
    list_line=current_line;
    printline();
    *txtpos=tmp;
  }
  line_terminator();
  goto prompt;
stackstuffed:	printmsg(stackstuffedmsg);	goto warmstart;
nomem:			printmsg(nomemmsg);			goto warmstart;
run_next_statement:
  if(breakcheck()){	printmsg(breakmsg);	goto warmstart;	}
  while(*txtpos==':')    txtpos++;
  ignore_blanks();	if(*txtpos==NL)  goto execnextline;
  goto interperateAtTxtpos;
direct: 
  txtpos=program_end+sizeof(LINENUM);
  if(*txtpos==NL)  goto prompt;
interperateAtTxtpos:
  if(breakcheck()){	printmsg(breakmsg);	goto warmstart;	}
  scantable(keywords);
  ignore_blanks();
  switch(table_index){
      case KW_LIST:	goto list;
      case KW_LOAD:	goto eLoad;
      case KW_NEW:
        if(txtpos[0]!=NL)  goto syntaxerror;
        program_end=program_start;
        goto prompt;
      case KW_RUN:	current_line=program_start;	goto execline;
      case KW_SAVE:	goto eSave;
      case KW_NEXT:	goto next;
      case KW_LET:	goto assignment;
      case KW_IF:{
          short int val;
          expression_error=0;
          val=expression();
          if(expression_error || *txtpos==NL)  goto invalidexpr;
          if(val!=0)   goto interperateAtTxtpos;
          goto execnextline;
        }
      case KW_GOTO:
        expression_error=0;
        linenum=expression(); if(expression_error || *txtpos != NL)  goto invalidexpr;
        current_line=findline();
        goto execline;
      case KW_GOSUB:	goto gosub;
      case KW_RETURN:	goto gosub_return; 
      case KW_HASHTAG:
      case KW_REM:		goto execnextline;	// Ignore line completely
      case KW_FOR:		goto forloop; 
      case KW_INPUT:	goto input; 
      case KW_QMARK:
      case KW_PRINT:	goto print;
      case KW_AT:
      case KW_POKE:		goto poke;
      case KW_STOP: // This is the easy way to end - set the current line to the end of program attempt to run it
        if(txtpos[0]!=NL)  goto syntaxerror;
        current_line=program_end;
        goto execline;
      case KW_AWRITE:	isDigital=false;goto awrite;
      case KW_DWRITE:	isDigital=true;	goto dwrite;
      case KW_MEM:		goto mem;
      case KW_TONE:		goto mTone;
      case KW_DELAY:
        expression_error=0;
        delay(expression());
        goto run_next_statement;
      case KW_CLEAR:
        for(clr_count=0; clr_count<27*VAR_SIZE; clr_count++)	memory[(int)clr_count]=0;
        goto run_next_statement;
      case KW_DEFAULT:	goto assignment;
      default:			break;
      }
execnextline:
  if(current_line==(void *)0)	  goto prompt;	// Processing direct commands?
  current_line += current_line[sizeof(LINENUM)];
execline:
  if(current_line == program_end) goto warmstart; // Out of lines to run
  txtpos=current_line+sizeof(LINENUM)+sizeof(char);
  goto interperateAtTxtpos;
input:{
    unsigned char isneg=0, *temptxtpos;
    short int *var;
    ignore_blanks();	if(*txtpos<'A' || *txtpos>'Z')  goto syntaxerror;
    var=((short int *)variables_table)+*txtpos-'A';
    txtpos++;	if(!check_statement_end())	goto syntaxerror;
again:
    temptxtpos=txtpos;
    if(!getln('?'))  goto warmstart;
    txtpos=program_end+sizeof(LINENUM);    // Go to where the buffer is read
    if(*txtpos=='-'){	isneg=1;	txtpos++;	}
    *var=0;
    do{	*var=*var*10+*txtpos-'0';	txtpos++; }while(*txtpos>='0' && *txtpos<='9');
    ignore_blanks();	if(*txtpos!=NL){	printmsg(badinputmsg);	goto again;}
    if(isneg)  *var=-*var;
    goto run_next_statement;
  }
mem:  // memory free
  printnum(stack_limit-program_end);
  printmsg(memorymsg);
  goto run_next_statement;
awrite:
dwrite:{
  short int pinNo, val;
  expression_error=0;
  pinNo=checkParm();		// Get the pin number
  checkForComma();	if(stopFlag)	goto prompt; 
  expression_error=0; val=expression();
  pinMode(pinNo, OUTPUT);
  if(isDigital)	digitalWrite(pinNo, val);
  else  		analogWrite( pinNo, val);
  }
  goto run_next_statement;
// --------------------------------------------------------------------------
forloop:{
    unsigned char var;
    short int initial, step, terminal;
    if(*txtpos<'A' || *txtpos>'Z')  goto syntaxerror;
    var=*txtpos;
    txtpos++;
    scantable(relop_tab);	if(table_index != RELOP_EQ)  goto syntaxerror;
    expression_error=0;
    initial=expression();	if(expression_error)  goto invalidexpr;
    scantable(to_tab);		if(table_index != 0)  goto syntaxerror;
    terminal=expression();	if(expression_error)  goto invalidexpr;
    scantable(step_tab);
    if(table_index==0){
      step=expression();	if(expression_error)  goto invalidexpr;
    }else  step=1;
    if(!check_statement_end())  goto syntaxerror;
    if(!expression_error && *txtpos==NL){
      struct stack_for_frame *f;
      if(spt+sizeof(struct stack_for_frame)<stack_limit)  goto nomem;
      spt-=sizeof(struct stack_for_frame);
      f=(struct stack_for_frame *)spt;
      ((short int *)variables_table)[var-'A'] = initial;
      f->frame_type = STACK_FOR_FLAG;
      f->for_var = var;
      f->terminal = terminal;
      f->step     = step;
      f->txtpos   = txtpos;
      f->current_line = current_line;
      goto run_next_statement;
    }
  }
  goto syntaxerror;
gosub:
  expression_error=0;
  linenum=expression();	if(expression_error)  goto invalidexpr;
  if(!expression_error && *txtpos==NL){
    struct stack_gosub_frame *f;
    if(spt+sizeof(struct stack_gosub_frame)<stack_limit)  goto nomem;
    spt -= sizeof(struct stack_gosub_frame);
    f = (struct stack_gosub_frame *)spt;
    f->frame_type = STACK_GOSUB_FLAG;
    f->txtpos = txtpos;
    f->current_line=current_line;
    current_line=findline();
    goto execline;
  }
  goto syntaxerror;
next:  // Fnd the variable name
  ignore_blanks();	if(*txtpos<'A' || *txtpos>'Z')  goto syntaxerror;
  txtpos++;	if(!check_statement_end())  goto syntaxerror;
gosub_return:  // Now walk up the stack frames and find the frame we want, if present
  for(tempsp=spt; tempsp<memory+sizeof(memory)-1; ){
    switch(tempsp[0]){
    case STACK_GOSUB_FLAG:
      if(table_index==KW_RETURN){
        struct stack_gosub_frame *f=(struct stack_gosub_frame *)tempsp;
        current_line	= f->current_line;
        txtpos			= f->txtpos;
        spt += sizeof(struct stack_gosub_frame);
        goto run_next_statement;
      }
      // This is not the loop you are looking for... so Walk back up the stack
      tempsp += sizeof(struct stack_gosub_frame);
      break;
    case STACK_FOR_FLAG:
      // Flag, Var, Final, Step
      if(table_index==KW_NEXT){
        struct stack_for_frame *f=(struct stack_for_frame *)tempsp;
        if(txtpos[-1] == f->for_var){        // Is the the variable we are looking for?
          short int *varaddr = ((short int *)variables_table) + txtpos[-1] - 'A'; 
          *varaddr = *varaddr + f->step;
          // Use a different test depending on the sign of the step increment
          if((f->step>0 && *varaddr <= f->terminal) || (f->step < 0 && *varaddr >= f->terminal)){
            // We have to loop so don't pop the stack
            txtpos = f->txtpos;
            current_line = f->current_line;
            goto run_next_statement;
          }
          // We've run to the end of the loop. drop out of the loop, popping the stack
          spt=tempsp+sizeof(struct stack_for_frame);
          goto run_next_statement;
        }
      }
      // This is not the loop you are looking for... so Walk back up the stack
      tempsp += sizeof(struct stack_for_frame);
      break;
    default:	goto stackstuffed;
    }
  }
  goto syntaxerror;  // Didn't find the variable we've been looking for
assignment:{
    short int value, *var;
    if(*txtpos<'A' || *txtpos>'Z')  goto syntaxerror;
    var=(short int *)variables_table + *txtpos-'A';
    txtpos++;
    ignore_blanks();	if(*txtpos!='=')  goto syntaxerror;
    txtpos++; ignore_blanks();  expression_error=0;
    value=expression();	if(expression_error)  goto invalidexpr;
    if(!check_statement_end())  goto syntaxerror;    // Check that we are at the end of the statement
    *var=value;
  }
  goto run_next_statement;
poke:{
    short int value;
    unsigned char *address;
    // Work out where to put it
    expression_error=0;
    value=expression();	if(expression_error)  goto invalidexpr;
    address=(unsigned char *)value;
    ignore_blanks();	if(*txtpos!=',')  goto syntaxerror;
    txtpos++; ignore_blanks();
    if(*txtpos=='"'){		// check for a quote
      for(txtpos++; *txtpos!='"'; txtpos++){
        memory[(int)address]=*txtpos;
        address++;
      }
      memory[(int)address]=0x0D;	// put newline at end of string
      txtpos++; goto run_next_statement;
    }
    expression_error=0;				// Now get the value to assign
    value=expression();	if(expression_error)  goto invalidexpr;
    memory[(int)address]=value;
    if(!check_statement_end())  goto syntaxerror;    // Check that we are at the end of the statement
  }
  goto run_next_statement;
mTone:{
    unsigned int pin, freq, dur;
    pin=checkParm();	if(stopFlag)  goto prompt;
    checkForComma();	if(stopFlag)  goto prompt;
    freq=checkParm();	if(stopFlag)  goto prompt;
    checkForComma();	if(stopFlag)  goto prompt;
    dur=checkParm();	if(stopFlag)  goto prompt;
    tone(pin, freq, dur); delay(dur+10);
    if(!check_statement_end())  goto syntaxerror;    // Check that we are at the end of the statement
  }
  goto run_next_statement;
list:
  linenum=testnum(); // Retuns 0 if no line found.
  if(*txtpos=='.'){
    list_line=findline();
    printline();
    goto warmstart;
  }
  if(*txtpos == '-'){
    int lincount=0, numlines=5;
    for(list_line=findline(); lincount!=numlines; lincount ++){
      printline();
      if(list_line==program_end)  goto warmstart;
    }
    goto warmstart;
  }
  if(txtpos[0]!=NL)  goto syntaxerror;	// Should be EOL
  for(list_line=findline(); list_line!=program_end; )	printline();
  goto warmstart;

eLoad:{	// load from EEPROM to RAM
  int size=(EEPROM.read(EESIZE-2))+(EEPROM.read(EESIZE-1)<<8);
  if(size<=EESIZE-2){
    program_end=program_start+size;
    for(int i=0; i<size; i++)	memory[i+27*VAR_SIZE]=EEPROM.read(i);
  }
  }
  goto warmstart;
eSave:{  // save memory to the EEPROM-EESIZE bytes, current usable RAM
  int size=int(program_end)-int(program_start);
  for(int i=0; i<size; i++)	EEPROM.write(i, memory[i+27*VAR_SIZE]);
  EEPROM.write(EESIZE-2, size&0xff); EEPROM.write(EESIZE-1, size>>8);
  }
  goto warmstart;
sprint:
print:  // If we have an empty list then just put out a NL
  if(*txtpos==':'){	line_terminator();	txtpos++;	goto run_next_statement;	}
  if(*txtpos==NL)  goto execnextline;
  while(1){
    ignore_blanks();
    if(print_quoted_string()) ;
    else if(*txtpos=='"' || *txtpos=='\'')  goto syntaxerror;
    else{
      expression_error=0;
      short int e=expression();
      if(expression_error && expression_error!=15)   goto invalidexpr;
      if(expression_error!=15)	printnum(e);
    }
    // At this point we have three options, a comma or a new line
    if(*txtpos==',')	txtpos++;	// Skip the comma and move onto the next
    else if(txtpos[0]==';' && (txtpos[1]==NL || txtpos[1]==':')){
      txtpos++; // This has to be the end of the print - no newline
      break;
    }else if(check_statement_end()){	line_terminator();	break;}	// The end of the print statement
    else  goto syntaxerror;
  }
  goto run_next_statement;
}
/***************************************************************************/
static int checkParm(void){
  expression_error=0;
  int value=expression();
  if(expression_error){	printmsg(invalidexprmsg);	stopFlag=true;	return 0;	}
  else{	stopFlag=false;	return value;	}
}
/***************************************************************************/
static int checkForComma(void){
  ignore_blanks();
  if(*txtpos!=','){
    printmsg(syntaxmsg);
    if(current_line!=(void *)0){
      unsigned char tmp=*txtpos;
      if(*txtpos!=NL)  *txtpos='^';
      list_line=current_line;
      printline();
      *txtpos=tmp;
    }
    line_terminator();
    stopFlag=true;
  }else{ 
    txtpos++;
    ignore_blanks();
    stopFlag=false;
  }
}
/***************************************************************************/
static void line_terminator(void){	outchar(NL);	outchar(CR);	}
/***********************************************************/
static unsigned char breakcheck(void){
  if(Serial.available())		return Serial.read()==ESC;
  else  return 0;
}
/***********************************************************/
static int inchar(){
  while(1){
	if(Serial.available())			return Serial.read();
  }
}
/***********************************************************/
static void outchar(unsigned char c){
  Serial.write(c);
}
/***********************************************************/
void setup(){
  Serial.begin(9600);	// opens serial port, sets data rate to 9600 bps
  Serial.println("TinyBasic");
}
