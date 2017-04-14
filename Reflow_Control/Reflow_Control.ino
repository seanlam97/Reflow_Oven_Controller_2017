/* 
 * Author: Sean Lam
 * Date: April 13, 2017
 * Purpose: Control reflow process by selecting temperatures and times
 * NOTE: LCD Code has been adapted from Dr. Jesus Calvino-Fraga from ELEC 291
 *        (a project course for 2nd year electrical engineers at the 
 *         University of British Columbia)
 */

// Pushbutton pin assignments
#define PUSH1 3
#define PUSH2 4
#define PUSH3 5
#define PUSH4 6

// LCD pin assignments
#define LCD_RS 7
#define LCD_RW 8
#define LCD_E  9
#define LCD_D4 10
#define LCD_D5 11
#define LCD_D6 12
#define LCD_D7 13
#define CHARS_PER_LINE 16

void setup() {
  // Set digital pins 7 - 13 on Arduino as output to LCD
  pinMode(LCD_RS, OUTPUT);
  pinMode(LCD_RW, OUTPUT);
  pinMode(LCD_E, OUTPUT);
  pinMode(LCD_D4, OUTPUT);
  pinMode(LCD_D5, OUTPUT);
  pinMode(LCD_D6, OUTPUT);
  pinMode(LCD_D7, OUTPUT);

  // Set digital pins 3 - 6 on Arduino as pushbutton inputs
  pinMode(PUSH1, INPUT);
  pinMode(PUSH2, INPUT);
  pinMode(PUSH3, INPUT);
  pinMode(PUSH4, INPUT);
}

void loop() {
  int flag = 0;
  int BUT1, BUT2, BUT3, BUT4;
  LCD_4BIT();

  while(1)
  {
    if(digitalRead(PUSH1))
    {
      LCDprint("Sean", 1, 1);
    }
    else if(digitalRead(PUSH2))
    {
      LCDprint("Bob", 1, 1);
    }
    else if(digitalRead(PUSH3))
    {
      LCDprint("Tom", 1, 1);
    }
    else if(digitalRead(PUSH4))
    {
      LCDprint("Luna", 1, 1);
    }
    else
    {
      LCDprint("NOTHING", 2, 0);
    }
  }
}

void LCD_pulse (void)
{
  digitalWrite(LCD_E, HIGH);
  delayMicroseconds(40);
  digitalWrite(LCD_E, LOW);
}

void LCD_byte (unsigned char x)
{
  digitalWrite(LCD_D7, bitRead(x, 7));
  digitalWrite(LCD_D6, bitRead(x, 6));
  digitalWrite(LCD_D5, bitRead(x, 5));
  digitalWrite(LCD_D4, bitRead(x, 4));
  LCD_pulse();
  delayMicroseconds(40);
  digitalWrite(LCD_D7, bitRead(x, 3));
  digitalWrite(LCD_D6, bitRead(x, 2));
  digitalWrite(LCD_D5, bitRead(x, 1));
  digitalWrite(LCD_D4, bitRead(x, 0));
  LCD_pulse();
}

void WriteData (unsigned char x)
{
  digitalWrite(LCD_RS, HIGH);
  LCD_byte(x);
  delay(2);
}

void WriteCommand (unsigned char x)
{
  digitalWrite(LCD_RS, LOW);
  LCD_byte(x);
  delay(5);
}

void LCD_4BIT (void)
{
  digitalWrite(LCD_E, LOW); // Resting state of LCD's enable pin is zero
  digitalWrite(LCD_RW, LOW); // We are only writing to the LCD in this program
  delay(20);
  // First make sure the LCD is in 8-bit mode and then change to 4-bit mode
  WriteCommand(0x33);
  WriteCommand(0x33);
  WriteCommand(0x32); // Change to 4-bit mode

  // Configure the LCD
  WriteCommand(0x28);
  WriteCommand(0x0c);
  WriteCommand(0x01); // Clear screen command (takes some time)
  delay(20); // Wait for clear screen command to finsih.
}

void LCDprint(char * string, unsigned char line, int clear_yn)
{
  int j;

  WriteCommand(line==2?0xc0:0x80);
  delay(5);
  for(j=0; string[j]!=0; j++) WriteData(string[j]);// Write the message
  if(clear_yn) for(; j<CHARS_PER_LINE; j++) WriteData(' '); // Clear the rest of the line
}

