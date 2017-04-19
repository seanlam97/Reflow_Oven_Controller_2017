/* 
 * Author: Sean Lam
 * Date: April 13, 2017
 * Purpose: Control reflow process by selecting temperatures and times
 * NOTE: LCD Code has been adapted from Dr. Jesus Calvino-Fraga from ELEC 291
 *        (a project course for 2nd year electrical engineers at the 
 *         University of British Columbia). 
 */

#include "Timer.h"  //http://github.com/JChristensen/Timer
 
#define SSR_PIN 2
#define COOL_TEMP 60

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

// Beeper
#define BEEP_PIN 19
#define BEEP_PERIOD 250

// Timer object
Timer t;

// Variables
char BUFF[CHARS_PER_LINE];
int soak_temp, soak_time, reflow_temp, reflow_time;
int soak_temp_ones, soak_temp_tens, soak_temp_hundreds;
int reflow_temp_ones, reflow_temp_tens, reflow_temp_hundreds;
int soak_time_mins, soak_time_secs;
int reflow_time_mins, reflow_time_secs;
double temperature, reading_1, reading_2;
int sensorPin = A0;
int state;
int select_parameter; 

void setup() {
  // Set digital pins 7 - 13 on Arduino as output to LCD
  pinMode(LCD_RS, OUTPUT);
  pinMode(LCD_RW, OUTPUT);
  pinMode(LCD_E, OUTPUT);
  pinMode(LCD_D4, OUTPUT);
  pinMode(LCD_D5, OUTPUT);
  pinMode(LCD_D6, OUTPUT);
  pinMode(LCD_D7, OUTPUT);

  // Set up SSR pin and turn off oven
  pinMode(SSR_PIN, OUTPUT);
  digitalWrite(SSR_PIN, LOW);  

  // Set digital pins 3 - 6 on Arduino as pushbutton inputs
  pinMode(PUSH1, INPUT);
  pinMode(PUSH2, INPUT);
  pinMode(PUSH3, INPUT);
  pinMode(PUSH4, INPUT);

  // Set digital pin 19 as beeper output
  pinMode(BEEP_PIN, OUTPUT);
  
  // Set up serial port with 9600 baud rate
  Serial.begin(9600);

  // Set up time decrement
  int tickEvent1 = t.every(990, decrementSoakTime);
  Serial.print("1 second tick started id=");
  Serial.println(tickEvent1);

  int tickEvent2 = t.every(990, decrementReflowTime);
  Serial.print("1 second tick started id=");
  Serial.println(tickEvent2);
}

void loop() {
  int i;

  // Initialise 
  LCD_4BIT();

  LCDprint("REFLOW OVEN", 1, 1);
  LCDprint("CONTROLLER V1.0", 2, 1);
  delay(2000);
  clear_LCD();

  // Initialize variable for "SELECT REFLOW PROFILE" state
  state = 0;
  soak_temp = 0;
  soak_time = 0;
  reflow_temp = 0;
  reflow_time = 0;
  
  soak_temp_ones = 0;
  soak_temp_tens = 0;
  soak_temp_hundreds = 0;
  
  reflow_temp_ones = 0;
  reflow_temp_tens = 0;
  reflow_temp_hundreds = 0;
  
  soak_time_mins = 0;
  soak_time_secs = 0;
  
  reflow_time_mins = 0;
  reflow_time_secs = 0;
  
  while(1)
  {
    /********** STATE 0: SELECT REFLOW PROFILE ****************/
    select_parameter = 0;
    while(state == 0)
    {
      if(select_parameter == 0)
      { // Select Soak Temperature
        select_parameter = Select_Soak_Temp();
      }
      else if(select_parameter == 1)
      { // Select Soak Time
        select_parameter = Select_Soak_Time();
      }
      else if(select_parameter == 2)
      { // Select Reflow Temperature
        select_parameter = Select_Reflow_Temp();
      }
      else if(select_parameter == 3)
      { // Select Reflow Time
        select_parameter = Select_Reflow_Time();
      }
      else if(select_parameter == 4)
      {
        state = Start_Reflow_YN();
        if(state == 0)
        {
          select_parameter = 0;
        }
      }
    }

    Beep_Milli_Secs(300);
    
    /********** STATE 1: SOAK TEMPERATURE ****************/
    while(state == 1)
    {
      digitalWrite(SSR_PIN, HIGH);
      temperature = analogRead(sensorPin)/2.7 + 20.0;
      displayTemperature(temperature, state);
      
      if(temperature > (soak_temp))
      {
        state = 2;
      }     
    }

    Beep_Milli_Secs(300);

    /************** STATE 2: SOAK TIME ******************/
    while(state == 2)
    {
      temperature = analogRead(sensorPin)/2.7 + 20.0;
      displayTemperature(temperature, state);
     
      t.update();

      if(temperature > soak_temp)
      {
        digitalWrite(SSR_PIN, LOW);
      }
      else
      {
        digitalWrite(SSR_PIN, HIGH);
      }

      if(soak_time <= 0)
      {
        state = 3;
      }
    }

    Beep_Milli_Secs(300);

    /************** STATE 3: REFLOW TEMPERATURE ******************/
    while(state == 3)
    {
      digitalWrite(SSR_PIN, HIGH);
      temperature = analogRead(sensorPin)/2.7 + 20.0;
      displayTemperature(temperature, state);
      
      if(temperature > reflow_temp)
      {
        state = 4;
      }     
    }

    Beep_Milli_Secs(300);

    /************** STATE 4: REFLOW TIME ******************/
    while(state == 4)
    {
      temperature = analogRead(sensorPin)/2.7 + 20.0;
      displayTemperature(temperature, state);
     
      t.update();

      if(temperature > reflow_temp)
      {
        digitalWrite(SSR_PIN, LOW);
      }
      else
      {
        digitalWrite(SSR_PIN, HIGH);
      }

      if(reflow_time <= 0)
      {
        state = 5;
      }
    }

    Beep_Milli_Secs(1000);
    
    /************** STATE 5: COOLING ******************/
    while(state == 5)
    {
      temperature = analogRead(sensorPin)/2.7 + 20.0;
      displayTemperature(temperature, state);
      digitalWrite(SSR_PIN, LOW);

      if(temperature < COOL_TEMP)
      {
        state = 0;
        for(i = 0; i < 60; i++)
        {
          LCDprint("DONE DONE DONE", 1, 1);
          LCDprint("DONE DONE DONE", 2, 1);
          Beep_Milli_Secs(500);
          clear_LCD();
          Beep_Milli_Secs(500);
        }
      }
    }
  }
}

/****************************************************/
/************ REFLOW PROCESS FUNCTIONS **************/
/****************************************************/
int Select_Soak_Temp(void)
{
  int set_select_parameter = 0;
  
  // Add digits
  soak_temp = (soak_temp_hundreds*100) + (soak_temp_tens*10) + soak_temp_ones;
  sprintf(BUFF, "%d deg C", soak_temp);

  // Display selected soak temperature
  LCDprint("Soak Temperature", 1, 1);
  LCDprint(BUFF, 2, 1);

  if(digitalRead(PUSH1))
  {
    delay(100);
    soak_temp_hundreds = soak_temp_hundreds + 1;
    if(soak_temp_hundreds == 10)
    { // overflow handling
      soak_temp_hundreds = 0;
    }
  }
  else if(digitalRead(PUSH2))
  {
    delay(100);
    soak_temp_tens = soak_temp_tens + 1;
    if(soak_temp_tens == 10)
    { // overflow handling
      soak_temp_tens = 0;
    }
  }
  else if(digitalRead(PUSH3))
  {
    delay(100);
    soak_temp_ones = soak_temp_ones + 1;
    if(soak_temp_ones == 10)
    { // overflow handling
      soak_temp_hundreds = 0;
    }
  }
  else if(digitalRead(PUSH4))
  {
    delay(100);
    set_select_parameter = 1;
    clear_LCD();
  }

  return set_select_parameter;
}

int Select_Soak_Time(void)
{
  int set_select_parameter = 1;
  
  soak_time = (soak_time_mins*60) + soak_time_secs;

  sprintf(BUFF, "%d mins %d secs", soak_time_mins, soak_time_secs);
  LCDprint("Soak Time", 1, 1);
  LCDprint(BUFF, 2, 1);

  if(digitalRead(PUSH1))
  {
    delay(50);
    soak_time_mins = soak_time_mins + 1;
    if(soak_time_mins == 10)
    { // overflow handling
      soak_time_mins = 0;
    }
  }
  else if(digitalRead(PUSH2))
  {
    delay(50);
    soak_time_secs = soak_time_secs + 1;
    if(soak_time_secs == 60)
    { // overflow handling
      soak_time_secs = 0;
    }
  }
  else if(digitalRead(PUSH3))
  {
    delay(50);
    soak_time_secs = soak_time_secs - 1;
    if(soak_time_secs == -1)
    { // overflow handling
      soak_time_secs = 59;
    }
  }
  else if(digitalRead(PUSH4))
  {
    delay(100);
    set_select_parameter = 2;
    clear_LCD();
  }
  
  return set_select_parameter;
}

int Select_Reflow_Temp(void)
{
  int set_select_parameter = 2;
  
  // Add digits
  reflow_temp = (reflow_temp_hundreds*100) + (reflow_temp_tens*10) + reflow_temp_ones;
  sprintf(BUFF, "%d deg C", reflow_temp);

  // Display selected soak temperature
  LCDprint("Reflow Temp.", 1, 1);
  LCDprint(BUFF, 2, 1);

  if(digitalRead(PUSH1))
  {
    delay(100);
    reflow_temp_hundreds = reflow_temp_hundreds + 1;
    if(reflow_temp_hundreds == 10)
    { // overflow handling
      reflow_temp_hundreds = 0;
    }
  }
  else if(digitalRead(PUSH2))
  {
    delay(100);
    reflow_temp_tens = reflow_temp_tens + 1;
    if(reflow_temp_tens == 10)
    { // overflow handling
      reflow_temp_tens = 0;
    }
  }
  else if(digitalRead(PUSH3))
  {
    delay(100);
    reflow_temp_ones = reflow_temp_ones + 1;
    if(reflow_temp_ones == 10)
    { // overflow handling
      reflow_temp_hundreds = 0;
    }
  }
  else if(digitalRead(PUSH4))
  {
    delay(100);
    set_select_parameter = 3;
    clear_LCD();
  }

  return set_select_parameter;
}

int Select_Reflow_Time(void)
{
  int set_select_parameter = 3;
  
  reflow_time = (reflow_time_mins*60) + reflow_time_secs;

  sprintf(BUFF, "%d mins %d secs", reflow_time_mins, reflow_time_secs);
  LCDprint("Reflow Time", 1, 1);
  LCDprint(BUFF, 2, 1);

  if(digitalRead(PUSH1))
  {
    delay(50);
    reflow_time_mins = reflow_time_mins + 1;
    if(reflow_time_mins == 10)
    { // overflow handling
      reflow_time_mins = 0;
    }
  }
  else if(digitalRead(PUSH2))
  {
    delay(50);
    reflow_time_secs = reflow_time_secs + 1;
    if(reflow_time_secs == 60)
    { // overflow handling
      reflow_time_secs = 0;
    }
  }
  else if(digitalRead(PUSH3))
  {
    delay(50);
    reflow_time_secs = reflow_time_secs - 1;
    if(reflow_time_secs == -1)
    { // overflow handling
      reflow_time_secs = 59;
    }
  }
  else if(digitalRead(PUSH4))
  {
    delay(100);
    set_select_parameter = 4;
    clear_LCD();
  }
  
  return set_select_parameter;
}

int Start_Reflow_YN(void)
{
  int next_state;

  while(1)
  {
    LCDprint("1: Go Back", 1, 1);
    LCDprint("4: Start Reflow", 2, 1);
    if(digitalRead(PUSH1))
    {
      delay(50);
      next_state = 0;
      break;
    }
    else if(digitalRead(PUSH4))
    {
      delay(50);
      next_state = 1;
      break;
    }
  }

  return next_state;
}

void displayTemperature(int temperature, int state)
{
  if(state == 1)
  { // Soak Temperature
    sprintf(BUFF, "%d C Soak Temp", soak_temp);
    LCDprint(BUFF, 1, 1);
  }
  else if(state == 2)
  { // Soak Time: Includes logic to add a 0 in front of the time for aesthetics
    if(soak_time/60 < 10)
    {
      if( (soak_time - (soak_time/60)*60) < 10)
      {
        sprintf(BUFF, "0%d:0%d Soak Time", soak_time/60, soak_time - (soak_time/60)*60);
      }
      else
      {
        sprintf(BUFF, "0%d:%d Soak Time", soak_time/60, soak_time - (soak_time/60)*60);
      }
    }
    else
    {
      if((soak_time - (soak_time/60)*60) < 10)
      {
        sprintf(BUFF, "%d:0%d Soak Time", soak_time/60, soak_time - (soak_time/60)*60);
      }
      else
      {
        sprintf(BUFF, "%d:%d Soak Time", soak_time/60, soak_time - (soak_time/60)*60);
      }
    }
    LCDprint(BUFF, 1, 1);
  }
  else if(state == 3)
  { // Reflow Temperature
    sprintf(BUFF, "%d C Refl. Temp", reflow_temp);
    LCDprint(BUFF, 1, 1);
  }
  else if(state == 4)
  { // Reflow Time
    if(reflow_time/60 < 10)
    {
      if( (reflow_time - (reflow_time/60)*60) < 10)
      {
        sprintf(BUFF, "0%d:0%d Refl. Time", reflow_time/60, reflow_time - (reflow_time/60)*60);
      }
      else
      {
        sprintf(BUFF, "0%d:%d Refl. Time", reflow_time/60, reflow_time - (reflow_time/60)*60);
      }
    }
    else
    {
      if((reflow_time - (reflow_time/60)*60) < 10)
      {
        sprintf(BUFF, "%d:0%d Refl. Time", reflow_time/60, reflow_time - (reflow_time/60)*60);
      }
      else
      {
        sprintf(BUFF, "%d:%d Refl. Time", reflow_time/60, soak_time - (reflow_time/60)*60);
      }
    }
    LCDprint(BUFF, 1, 1);
  }
  else if(state == 5)
  { // Cooling
     sprintf(BUFF, "%d C Cool Temp", COOL_TEMP);
     LCDprint(BUFF, 1, 1);
  }

    // Print current temperature on second line
    sprintf(BUFF, "%d C Curr. Temp", temperature);
    LCDprint(BUFF, 2, 1);
}

void Beep_Milli_Secs(int milli_secs)
{
  int i;

  for(i = 0; i < milli_secs; i++)
  {
    digitalWrite(BEEP_PIN, HIGH);
    delayMicroseconds(BEEP_PERIOD);
    digitalWrite(BEEP_PIN, LOW);
    delayMicroseconds(BEEP_PERIOD);
    digitalWrite(BEEP_PIN, HIGH);
    delayMicroseconds(BEEP_PERIOD);
    digitalWrite(BEEP_PIN, LOW);
    delayMicroseconds(BEEP_PERIOD);
  }
}

void decrementSoakTime()
{
  if(state == 2)
  {
    soak_time = soak_time - 1;
  }
}

void decrementReflowTime()
{
  if(state == 4)
  {
    reflow_time = reflow_time - 1;
  }
}

/****************************************************/
/***************** LCD FUNCTIONS ********************/
/****************************************************/
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

void clear_LCD(void)
{
  LCDprint(" ", 1, 1);
  LCDprint(" ", 2, 1);
}

