# 1 "/home/dimao/MySketchbook/TinyBasicV020W/TinyBasicV020W.ino"
# 1 "/home/dimao/MySketchbook/TinyBasicV020W/TinyBasicV020W.ino"
// TinyBASIC.cpp : Defines the entry point for the console application.
// Author: Mike Field <hamster@snap.net.nz>
//
// Modified by aafent / Afentakis Andreas
//
// IF testing with Visual C
//#include "stdafx.h"

# 10 "/home/dimao/MySketchbook/TinyBasicV020W/TinyBasicV020W.ino" 2
# 11 "/home/dimao/MySketchbook/TinyBasicV020W/TinyBasicV020W.ino" 2

Ticker timer;
PCD8544 screen;

void timerIsr()
{
  ESP.wdtFeed();
//  Serial.println(".");
  delay(0);
//  yield();
}



// ASCII Characters
# 36 "/home/dimao/MySketchbook/TinyBasicV020W/TinyBasicV020W.ino"
typedef short unsigned LINENUM;



static unsigned char program[1024];
static unsigned char *txtpos, *list_line;
static unsigned char expression_error;
static unsigned char *tempsp;

/***********************************************************/
// Keyword table and constants - the last character has 0x80 added to it
static unsigned char keywords[] = {
  'L', 'I', 'S', 'T' + 0x80,
  'L', 'O', 'A', 'D' + 0x80,
  'N', 'E', 'W' + 0x80,
  'R', 'U', 'N' + 0x80,
  'S', 'A', 'V', 'E' + 0x80,
  'N', 'E', 'X', 'T' + 0x80,
  'L', 'E', 'T' + 0x80,
  'I', 'F' + 0x80,
  'G', 'O', 'T', 'O' + 0x80,
  'G', 'O', 'S', 'U', 'B' + 0x80,
  'R', 'E', 'T', 'U', 'R', 'N' + 0x80,
  'R', 'E', 'M' + 0x80,
  'F', 'O', 'R' + 0x80,
  'I', 'N', 'P', 'U', 'T' + 0x80,
  'P', 'R', 'I', 'N', 'T' + 0x80,
  'P', 'O', 'K', 'E' + 0x80,
  'S', 'T', 'O', 'P' + 0x80,
  'B', 'Y', 'E' + 0x80,
  0
};
# 89 "/home/dimao/MySketchbook/TinyBasicV020W/TinyBasicV020W.ino"
struct stack_for_frame {
  char frame_type;
  char for_var;
  short int terminal;
  short int step;
  unsigned char *current_line;
  unsigned char *txtpos;
};

struct stack_gosub_frame {
  char frame_type;
  unsigned char *current_line;
  unsigned char *txtpos;
};

static unsigned char func_tab[] = {
  'P', 'E', 'E', 'K' + 0x80,
  'A', 'B', 'S' + 0x80,
  0
};




static unsigned char to_tab[] = {
  'T', 'O' + 0x80,
  0
};

static unsigned char step_tab[] = {
  'S', 'T', 'E', 'P' + 0x80,
  0
};

static unsigned char relop_tab[] = {
  '>', '=' + 0x80,
  '<', '>' + 0x80,
  '>' + 0x80,
  '=' + 0x80,
  '<', '=' + 0x80,
  '<' + 0x80,
  0
};
# 144 "/home/dimao/MySketchbook/TinyBasicV020W/TinyBasicV020W.ino"
static unsigned char *stack_limit;
static unsigned char *program_start;
static unsigned char *program_end;
static unsigned char *stack; // Software stack for things that should go on the CPU stack
static unsigned char *variables_begin;
static unsigned char *current_line;
static unsigned char *sp;


static unsigned char table_index;
static LINENUM linenum;

static const unsigned char okmsg[] = "OK";
static const unsigned char whatmsg[] = "What? ";
static const unsigned char howmsg[] = "How?";
static const unsigned char sorrymsg[] = "Sorry!";
static const unsigned char initmsg[] = "TinyBasic in C V0.02.";
static const unsigned char memorymsg[] = " bytes free.";
static const unsigned char breakmsg[] = "break!";
static const unsigned char unimplimentedmsg[] = "Unimplemented";
static const unsigned char backspacemsg[] = "\b \b";

static int inchar(void);
static void outchar(unsigned char c);
static void line_terminator(void);
static short int expression(void);
static unsigned char breakcheck(void);
/***************************************************************************/
static void ignore_blanks(void)
{
  while (*txtpos == ' ' || *txtpos == '\t')
    txtpos++;
}

/***************************************************************************/
static void scantable(unsigned char *table)
{
  int i = 0;
  table_index = 0;
  while (1)
  {
    delay(0);
    // Run out of table entries?
    if (table[0] == 0)
      return;

    // Do we match this character?
    if (txtpos[i] == table[0])
    {
      i++;
      table++;
    }
    else
    {
      // do we match the last character of keywork (with 0x80 added)? If so, return
      if (txtpos[i] + 0x80 == table[0])
      {
        txtpos += i + 1; // Advance the pointer to following the keyword
        ignore_blanks();
        return;
      }

      // Forward to the end of this keyword
      while ((table[0] & 0x80) == 0)
        table++;

      // Now move on to the first character of the next word, and reset the position index
      table++;
      table_index++;
      ignore_blanks();
      i = 0;
    }
  }
}

/***************************************************************************/
static void pushb(unsigned char b)
{
  sp--;
  *sp = b;
}

/***************************************************************************/
static unsigned char popb()
{
  unsigned char b;
  b = *sp;
  sp++;
  return b;
}

/***************************************************************************/
static void printnum(int num)
{
  int digits = 0;

  if (num < 0)
  {
    num = -num;
    outchar('-');
  }

  do {
    pushb(num % 10 + '0');
    num = num / 10;
    digits++;
  }
  while (num > 0);

  while (digits > 0)
  {
    outchar(popb());
    digits--;
  }
}
/***************************************************************************/
static unsigned short testnum(void)
{
  unsigned short num = 0;
  ignore_blanks();

  while (*txtpos >= '0' && *txtpos <= '9' )
  {
    // Trap overflows
    if (num >= 0xFFFF / 10)
    {
      num = 0xFFFF;
      break;
    }

    num = num * 10 + *txtpos - '0';
    txtpos++;
  }
  return num;
}

/***************************************************************************/
static void printmsgNoNL(const unsigned char *msg)
{
  while (*msg)
  {
    outchar(*msg);
    msg++;
  }
}

/***************************************************************************/
static unsigned char print_quoted_string(void)
{
  int i = 0;
  unsigned char delim = *txtpos;
  if (delim != '"' && delim != '\'')
    return 0;
  txtpos++;

  // Check we have a closing delimiter
  while (txtpos[i] != delim)
  {
    if (txtpos[i] == '\n')
      return 0;
    i++;
  }

  // Print the characters
  while (*txtpos != delim)
  {
    outchar(*txtpos);
    txtpos++;
  }
  txtpos++; // Skip over the last delimiter

  return 1;
}

/***************************************************************************/
static void printmsg(const unsigned char *msg)
{
  printmsgNoNL(msg);
  line_terminator();
}

/***************************************************************************/
static void getln(char prompt)
{
  outchar(prompt);
  txtpos = program_end + sizeof(LINENUM);

  while (1)
  {
    delay(0);
    char c = inchar();
    switch (c)
    {
      case '\n':
        break;
      case '\r':
        line_terminator();
        // Terminate all strings with a NL
        txtpos[0] = '\n';
        return;
      case 0x08:
        if (txtpos == program_end)
          break;
        txtpos--;
        printmsg(backspacemsg);
        break;
      default:
        // We need to leave at least one space to allow us to shuffle the line into order
        if (txtpos == variables_begin - 2)
          outchar('\b');
        else
        {
          txtpos[0] = c;
          txtpos++;

          outchar(c);

        }
    }
  }
}

/***************************************************************************/
static unsigned char *findline(void)
{
  unsigned char *line = program_start;
  while (1)
  {
    delay(0);
    if (line == program_end)
      return line;

    if (((LINENUM *)line)[0] >= linenum)
      return line;

    // Add the line length onto the current address, to get to the next line;
    line += line[sizeof(LINENUM)];
  }
}

/***************************************************************************/
static void toUppercaseBuffer(void)
{
  unsigned char *c = program_end + sizeof(LINENUM);
  unsigned char quote = 0;

  while (*c != '\n')
  {
    // Are we in a quoted string?
    if (*c == quote)
      quote = 0;
    else if (*c == '"' || *c == '\'')
      quote = *c;
    else if (quote == 0 && *c >= 'a' && *c <= 'z')
      *c = *c + 'A' - 'a';
    c++;
  }
}

/***************************************************************************/
void printline()
{
  LINENUM line_num;

  line_num = *((LINENUM *)(list_line));
  list_line += sizeof(LINENUM) + sizeof(char);

  // Output the line */
  printnum(line_num);
  outchar(' ');
  while (*list_line != '\n')
  {
    outchar(*list_line);
    list_line++;
  }
  list_line++;
  line_terminator();
}

/***************************************************************************/
static short int expr4(void)
{

  if (*txtpos == '0')
  {
    txtpos++;
    return 0;
  }

  if (*txtpos >= '1' && *txtpos <= '9')
  {
    short int a = 0;
    do {
      a = a * 10 + *txtpos - '0';
      txtpos++;
    } while (*txtpos >= '0' && *txtpos <= '9');
    return a;
  }

  // Is it a function or variable reference?
  if (txtpos[0] >= 'A' && txtpos[0] <= 'Z')
  {
    short int a;
    // Is it a variable reference (single alpha)
    if (txtpos[1] < 'A' || txtpos[1] > 'Z')
    {
      a = ((short int *)variables_begin)[*txtpos - 'A'];
      txtpos++;
      return a;
    }

    // Is it a function with a single parameter
    scantable(func_tab);
    if (table_index == 2)
      goto expr4_error;

    unsigned char f = table_index;

    if (*txtpos != '(')
      goto expr4_error;

    txtpos++;
    a = expression();
    if (*txtpos != ')')
      goto expr4_error;
    txtpos++;
    switch (f)
    {
      case 0:
        return program[a];
      case 1:
        if (a < 0)
          return -a;
        return a;
    }
  }

  if (*txtpos == '(')
  {
    short int a;
    txtpos++;
    a = expression();
    if (*txtpos != ')')
      goto expr4_error;

    txtpos++;
    return a;
  }

expr4_error:
  expression_error = 1;
  return 0;

}

/***************************************************************************/
static short int expr3(void)
{
  short int a, b;

  a = expr4();
  while (1)
  {
    delay(0);
    if (*txtpos == '*')
    {
      txtpos++;
      b = expr4();
      a *= b;
    }
    else if (*txtpos == '/')
    {
      txtpos++;
      b = expr4();
      if (b != 0)
        a /= b;
      else
        expression_error = 1;
    }
    else
      return a;
  }
}

/***************************************************************************/
static short int expr2(void)
{
  short int a, b;

  if (*txtpos == '-' || *txtpos == '+')
    a = 0;
  else
    a = expr3();

  while (1)
  {
    delay(0);
    if (*txtpos == '-')
    {
      txtpos++;
      b = expr3();
      a -= b;
    }
    else if (*txtpos == '+')
    {
      txtpos++;
      b = expr3();
      a += b;
    }
    else
      return a;
  }
}
/***************************************************************************/
static short int expression(void)
{
  short int a, b;

  a = expr2();
  // Check if we have an error
  if (expression_error) return a;

  scantable(relop_tab);
  if (table_index == 6)
    return a;

  switch (table_index)
  {
    case 0:
      b = expr2();
      if (a >= b) return 1;
      break;
    case 1:
      b = expr2();
      if (a != b) return 1;
      break;
    case 2:
      b = expr2();
      if (a > b) return 1;
      break;
    case 3:
      b = expr2();
      if (a == b) return 1;
      break;
    case 4:
      b = expr2();
      if (a <= b) return 1;
      break;
    case 5:
      b = expr2();
      if (a < b) return 1;
      break;
  }
  return 0;
}

/***************************************************************************/
void loop()
{
  unsigned char *start;
  unsigned char *newEnd;
  unsigned char linelen;

  delay(0);
  ESP.wdtDisable();
  ESP.wdtEnable(150000);

  program_start = program;
  program_end = program_start;
  sp = program + sizeof(program); // Needed for printnum
  stack_limit = program + sizeof(program) - (sizeof(struct stack_for_frame)*5);
  variables_begin = stack_limit - 27 * sizeof(short int) /* Size of variables in bytes*/;
  printmsg(initmsg);
  printnum(variables_begin - program_end);
  printmsg(memorymsg);

warmstart:
  // this signifies that it is running in 'direct' mode.
  current_line = 0;
  sp = program + sizeof(program);
  printmsg(okmsg);

prompt:
  getln('>');
  toUppercaseBuffer();

  txtpos = program_end + sizeof(unsigned short);

  // Find the end of the freshly entered line
  while (*txtpos != '\n')
    txtpos++;

  // Move it to the end of program_memory
  {
    unsigned char *dest;
    dest = variables_begin - 1;
    while (1)
    {
      delay(0);
      *dest = *txtpos;
      if (txtpos == program_end + sizeof(unsigned short))
        break;
      dest--;
      txtpos--;
    }
    txtpos = dest;
  }

  // Now see if we have a line number
  linenum = testnum();
  ignore_blanks();
  if (linenum == 0)
    goto direct;

  if (linenum == 0xFFFF)
    goto qhow;

  // Find the length of what is left, including the (yet-to-be-populated) line header
  linelen = 0;
  while (txtpos[linelen] != '\n')
    linelen++;
  linelen++; // Include the NL in the line length
  linelen += sizeof(unsigned short) + sizeof(char); // Add space for the line number and line length

  // Now we have the number, add the line header.
  txtpos -= 3;
  *((unsigned short *)txtpos) = linenum;
  txtpos[sizeof(LINENUM)] = linelen;


  // Merge it into the rest of the program
  start = findline();

  // If a line with that number exists, then remove it
  if (start != program_end && *((LINENUM *)start) == linenum)
  {
    unsigned char *dest, *from;
    unsigned tomove;

    from = start + start[sizeof(LINENUM)];
    dest = start;

    tomove = program_end - from;
    while ( tomove > 0)
    {
      *dest = *from;
      from++;
      dest++;
      tomove--;
    }
    program_end = dest;
  }

  if (txtpos[sizeof(LINENUM) + sizeof(char)] == '\n') // If the line has no txt, it was just a delete
    goto prompt;



  // Make room for the new line, either all in one hit or lots of little shuffles
  while (linelen > 0)
  {
    unsigned int tomove;
    unsigned char *from, *dest;
    unsigned int space_to_make;

    space_to_make = txtpos - program_end;

    if (space_to_make > linelen)
      space_to_make = linelen;
    newEnd = program_end + space_to_make;
    tomove = program_end - start;


    // Source and destination - as these areas may overlap we need to move bottom up
    from = program_end;
    dest = newEnd;
    while (tomove > 0)
    {
      from--;
      dest--;
      *dest = *from;
      tomove--;
    }

    // Copy over the bytes into the new space
    for (tomove = 0; tomove < space_to_make; tomove++)
    {
      *start = *txtpos;
      txtpos++;
      start++;
      linelen--;
    }
    program_end = newEnd;
  }
  goto prompt;

unimplemented:
  printmsg(unimplimentedmsg);
  goto prompt;

qhow:
  printmsg(howmsg);
  goto prompt;

qwhat:
  printmsgNoNL(whatmsg);
  if (current_line != __null)
  {
    unsigned char tmp = *txtpos;
    if (*txtpos != '\n')
      *txtpos = '^';
    list_line = current_line;
    printline();
    *txtpos = tmp;
  }
  line_terminator();
  goto prompt;

qsorry:
  printmsg(sorrymsg);
  goto warmstart;

run_next_statement:
  while (*txtpos == ':')
    txtpos++;
  ignore_blanks();
  if (*txtpos == '\n')
    goto execnextline;
  goto interperateAtTxtpos;

direct:
  txtpos = program_end + sizeof(LINENUM);
  if (*txtpos == '\n')
    goto prompt;

interperateAtTxtpos:
  if (breakcheck())
  {
    printmsg(breakmsg);
    goto warmstart;
  }

  scantable(keywords);

  switch (table_index)
  {
    case 0:
      goto list;
    case 1:
      goto unimplemented; /////////////////
    case 2:
      if (txtpos[0] != '\n')
        goto qwhat;
      program_end = program_start;
      goto prompt;
    case 3:
      current_line = program_start;
      goto execline;
    case 4:
      goto unimplemented; //////////////////////
    case 5:
      goto next;
    case 6:
      goto assignment;
    case 7:
      short int val;
      expression_error = 0;
      val = expression();
      if (expression_error || *txtpos == '\n')
        goto qhow;
      if (val != 0)
        goto interperateAtTxtpos;
      goto execnextline;

    case 8:
      expression_error = 0;
      linenum = expression();
      if (expression_error || *txtpos != '\n')
        goto qhow;
      current_line = findline();
      goto execline;

    case 9:
      goto gosub;
    case 10:
      goto gosub_return;
    case 11:
      goto execnextline; // Ignore line completely
    case 12:
      goto forloop;
    case 13:
      goto input;
    case 14:
      goto print;
    case 15:
      goto poke;
    case 16:
      // This is the easy way to end - set the current line to the end of program attempt to run it
      if (txtpos[0] != '\n')
        goto qwhat;
      current_line = program_end;
      goto execline;
    case 17:
      // Leave the basic interperater
      return;
    case 18:
      goto assignment;
    default:
      break;
  }

execnextline:
  if (current_line == __null) // Processing direct commands?
    goto prompt;
  current_line += current_line[sizeof(LINENUM)];

execline:
  if (current_line == program_end) // Out of lines to run
    goto warmstart;
  txtpos = current_line + sizeof(LINENUM) + sizeof(char);
  goto interperateAtTxtpos;

input:
  {
    unsigned char var;
    ignore_blanks();
    if (*txtpos < 'A' || *txtpos > 'Z')
      goto qwhat;
    var = *txtpos;
    txtpos++;
    ignore_blanks();
    if (*txtpos != '\n' && *txtpos != ':')
      goto qwhat;
    ((short int *)variables_begin)[var - 'A'] = 99;

    goto run_next_statement;
  }
forloop:
  {
    unsigned char var;
    short int initial, step, terminal;
    ignore_blanks();
    if (*txtpos < 'A' || *txtpos > 'Z')
      goto qwhat;
    var = *txtpos;
    txtpos++;
    ignore_blanks();
    if (*txtpos != '=')
      goto qwhat;
    txtpos++;
    ignore_blanks();

    expression_error = 0;
    initial = expression();
    if (expression_error)
      goto qwhat;

    scantable(to_tab);
    if (table_index != 0)
      goto qwhat;

    terminal = expression();
    if (expression_error)
      goto qwhat;

    scantable(step_tab);
    if (table_index == 0)
    {
      step = expression();
      if (expression_error)
        goto qwhat;
    }
    else
      step = 1;
    ignore_blanks();
    if (*txtpos != '\n' && *txtpos != ':')
      goto qwhat;


    if (!expression_error && *txtpos == '\n')
    {
      struct stack_for_frame *f;
      if (sp + sizeof(struct stack_for_frame) < stack_limit)
        goto qsorry;

      sp -= sizeof(struct stack_for_frame);
      f = (struct stack_for_frame *)sp;
      ((short int *)variables_begin)[var - 'A'] = initial;
      f->frame_type = 'F';
      f->for_var = var;
      f->terminal = terminal;
      f->step = step;
      f->txtpos = txtpos;
      f->current_line = current_line;
      goto run_next_statement;
    }
  }
  goto qhow;

gosub:
  expression_error = 0;
  linenum = expression();
  if (!expression_error && *txtpos == '\n')
  {
    struct stack_gosub_frame *f;
    if (sp + sizeof(struct stack_gosub_frame) < stack_limit)
      goto qsorry;

    sp -= sizeof(struct stack_gosub_frame);
    f = (struct stack_gosub_frame *)sp;
    f->frame_type = 'G';
    f->txtpos = txtpos;
    f->current_line = current_line;
    current_line = findline();
    goto execline;
  }
  goto qhow;

next:
  // Fnd the variable name
  ignore_blanks();
  if (*txtpos < 'A' || *txtpos > 'Z')
    goto qhow;
  txtpos++;
  ignore_blanks();
  if (*txtpos != ':' && *txtpos != '\n')
    goto qwhat;

gosub_return:
  // Now walk up the stack frames and find the frame we want, if present
  tempsp = sp;
  while (tempsp < program + sizeof(program) - 1)
  {
    switch (tempsp[0])
    {
      case 'G':
        if (table_index == 10)
        {
          struct stack_gosub_frame *f = (struct stack_gosub_frame *)tempsp;
          current_line = f->current_line;
          txtpos = f->txtpos;
          sp += sizeof(struct stack_gosub_frame);
          goto run_next_statement;
        }
        // This is not the loop you are looking for... so Walk back up the stack
        tempsp += sizeof(struct stack_gosub_frame);
        break;
      case 'F':
        // Flag, Var, Final, Step
        if (table_index == 5)
        {
          struct stack_for_frame *f = (struct stack_for_frame *)tempsp;
          // Is the the variable we are looking for?
          if (txtpos[-1] == f->for_var)
          {
            short int *varaddr = ((short int *)variables_begin) + txtpos[-1] - 'A';
            *varaddr = *varaddr + f->step;
            // Use a different test depending on the sign of the step increment
            if ((f->step > 0 && *varaddr <= f->terminal) || (f->step < 0 && *varaddr >= f->terminal))
            {
              // We have to loop so don't pop the stack
              txtpos = f->txtpos;
              current_line = f->current_line;
              goto run_next_statement;
            }
            // We've run to the end of the loop. drop out of the loop, popping the stack
            sp = tempsp + sizeof(struct stack_for_frame);
            goto run_next_statement;
          }
        }
        // This is not the loop you are looking for... so Walk back up the stack
        tempsp += sizeof(struct stack_for_frame);
        break;
      default:
        printf("Stack is stuffed!\n");
        goto warmstart;
    }
  }
  // Didn't find the variable we've been looking for
  goto qhow;

assignment:
  {
    short int value;
    short int *var;

    if (*txtpos < 'A' || *txtpos > 'Z')
      goto qhow;
    var = (short int *)variables_begin + *txtpos - 'A';
    txtpos++;

    ignore_blanks();

    if (*txtpos != '=')
      goto qwhat;
    txtpos++;
    ignore_blanks();
    expression_error = 0;
    value = expression();
    if (expression_error)
      goto qwhat;
    // Check that we are at the end of the statement
    if (*txtpos != '\n' && *txtpos != ':')
      goto qwhat;
    *var = value;
  }
  goto run_next_statement;
poke:
  {
    short int value;
    unsigned char *address;

    // Work out where to put it
    expression_error = 0;
    value = expression();
    if (expression_error)
      goto qwhat;
    address = (unsigned char *)value;

    // check for a comma
    ignore_blanks();
    if (*txtpos != ',')
      goto qwhat;
    txtpos++;
    ignore_blanks();

    // Now get the value to assign
    expression_error = 0;
    value = expression();
    if (expression_error)
      goto qwhat;
    printf("Poke %p value %i\n", address, (unsigned char)value);
    // Check that we are at the end of the statement
    if (*txtpos != '\n' && *txtpos != ':')
      goto qwhat;
  }
  goto run_next_statement;

list:
  linenum = testnum(); // Retuns 0 if no line found.

  // Should be EOL
  if (txtpos[0] != '\n')
    goto qwhat;

  // Find the line
  list_line = findline();
  while (list_line != program_end)
    printline();
  goto warmstart;

print:
  // If we have an empty list then just put out a NL
  if (*txtpos == ':' )
  {
    line_terminator();
    txtpos++;
    goto run_next_statement;
  }
  if (*txtpos == '\n')
  {
    goto execnextline;
  }

  while (1)
  {
    delay(0);
    ignore_blanks();
    if (print_quoted_string())
    {
      ;
    }
    else if (*txtpos == '"' || *txtpos == '\'')
      goto qwhat;
    else
    {
      short int e;
      expression_error = 0;
      e = expression();
      if (expression_error)
        goto qwhat;
      printnum(e);
    }

    // At this point we have three options, a comma or a new line
    if (*txtpos == ',')
      txtpos++; // Skip the comma and move onto the next
    else if (txtpos[0] == ';' && (txtpos[1] == '\n' || txtpos[1] == ':'))
    {
      txtpos++; // This has to be the end of the print - no newline
      break;
    }
    else if (*txtpos == '\n' || *txtpos == ':')
    {
      line_terminator(); // The end of the print statement
      break;
    }
    else
      goto qwhat;
  }
  goto run_next_statement;
}

/***************************************************************************/
static void line_terminator(void)
{
  outchar('\n');
  outchar('\r');
}

/***********************************************************/
void setup()
{
  ESP.wdtDisable();
  ESP.wdtEnable(150000);
  delay(0);
  timer.attach(1, timerIsr);
  Serial.begin(115200); // opens serial port, sets data rate to 9600 bps
  Serial.println("boot.");

  screen.begin(84, 48);
  screen.setContrast(60);
  screen.clear();
  screen.setCursor(0, 0);
  screen.println("!");

     uint32_t realSize = ESP.getFlashChipRealSize();
    uint32_t ideSize = ESP.getFlashChipSize();
    FlashMode_t ideMode = ESP.getFlashChipMode();

    Serial.printf("Flash real id:   %08X\n", ESP.getFlashChipId());
    Serial.printf("Flash real size: %u\n\n", realSize);

    Serial.printf("Flash ide  size: %u\n", ideSize);
    Serial.printf("Flash ide speed: %u\n", ESP.getFlashChipSpeed());
    Serial.printf("Flash ide mode:  %s\n", (ideMode == FM_QIO ? "QIO" : ideMode == FM_QOUT ? "QOUT" : ideMode == FM_DIO ? "DIO" : ideMode == FM_DOUT ? "DOUT" : "UNKNOWN"));

    if(ideSize != realSize) {
        Serial.println("Flash Chip configuration wrong!\n");
    } else {
        Serial.println("Flash Chip configuration ok.\n");
    }

delay(500);
}

/***********************************************************/
static unsigned char breakcheck(void)
{

  if (Serial.available())
    return Serial.read() == 0x03;
  return 0;

}
/***********************************************************/
static int inchar()
{

  while (1)
  {
    delay(0);
    if (Serial.available())
      return Serial.read();
  }

}

/***********************************************************/
static void outchar(unsigned char c)
{
  Serial.write(c);
  screen.write(c);
}
