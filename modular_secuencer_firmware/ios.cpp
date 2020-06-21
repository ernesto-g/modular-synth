#include <Arduino.h>
#include <Wire.h>
#include "Adafruit_MCP4725.h"
#include "ios.h"

#define SWICTHES_MODE_NC  // Used swicthes are Normal Closed

#define PIN_GATE_1            1
#define PIN_EXT_CLK           2
#define PIN_EXT_RST           3

#define PIN_GATE_2            6
#define PIN_GATE_3            7
#define PIN_GATE_4            8

#define PIN_MUX_A             10
#define PIN_SR_LATCH          11
#define PIN_MUX_B_SR_DATA     12
#define PIN_MUX_C_SR_CLOCK    13

#define PIN_SW_TRACK_RUN      14 // OK
#define PIN_SW_SCALE_DIR      15 // OK
#define PIN_SW_RPT_PROB       16 // OK
#define PIN_SW_SHIFT          17 // OK
#define PIN_ANALOG_SW_LEN_MODE   A7

#define PIN_ANALOG_MULTIPLEXED  A6


Adafruit_MCP4725 dac;

void ios_init(void)
{
  pinMode(PIN_GATE_1,OUTPUT);
  digitalWrite(PIN_GATE_1,LOW);
  pinMode(PIN_GATE_2,OUTPUT);
  digitalWrite(PIN_GATE_2,LOW);
  pinMode(PIN_GATE_3,OUTPUT);
  digitalWrite(PIN_GATE_3,LOW);
  pinMode(PIN_GATE_4,OUTPUT);
  digitalWrite(PIN_GATE_4,LOW);

  pinMode(PIN_MUX_A,OUTPUT);
  digitalWrite(PIN_MUX_A,LOW);

  pinMode(PIN_MUX_B_SR_DATA,OUTPUT);
  digitalWrite(PIN_MUX_B_SR_DATA,LOW);

  pinMode(PIN_MUX_C_SR_CLOCK,OUTPUT);
  digitalWrite(PIN_MUX_C_SR_CLOCK,LOW);

  pinMode(PIN_SR_LATCH,OUTPUT);
  digitalWrite(PIN_SR_LATCH,LOW);

  //__________

  // configure SW pins as input
  pinMode(PIN_EXT_CLK,INPUT);
  pinMode(PIN_EXT_RST,INPUT);
  pinMode(PIN_SW_TRACK_RUN,INPUT_PULLUP);
  pinMode(PIN_SW_SCALE_DIR,INPUT);
  pinMode(PIN_SW_SHIFT,INPUT);
  pinMode(PIN_SW_RPT_PROB,INPUT);
  
  // configure SW pins pullups
  digitalWrite(PIN_EXT_CLK,HIGH);
  digitalWrite(PIN_EXT_RST,HIGH);
  digitalWrite(PIN_SW_SCALE_DIR,HIGH);
  digitalWrite(PIN_SW_SHIFT,HIGH);
  digitalWrite(PIN_SW_RPT_PROB,HIGH);

   // DAC
   dac.begin(0x60);

  // Configure TIMER1 for PWM (Velocity output)
  DDRB |= (1 << DDB1);
  TCCR1A =
      (1 << COM1A1) |
      // Fast PWM mode.
      (1 << WGM11);
  TCCR1B =
      // Fast PWM mode.
      (1 << WGM12) | (1 << WGM13) |
      // No clock prescaling (fastest possible freq).
      (1 << CS10); 
  OCR1A = 0;
  ICR1 = 128; // 125Khz
  //__________________________  

}

void ios_setCVout(int value)
{
    dac.setVoltage(value, false);
}

void ios_setVelocityOut(unsigned char value)
{
    OCR1A = value;
}

int ios_readMuxAnalog(void)
{
    return analogRead(PIN_ANALOG_MULTIPLEXED);
}

int ios_readSw(int input)
{
    #ifdef SWICTHES_MODE_NC
      switch(input)
      {
          case IO_SW_TRACK_RUN: if(digitalRead(PIN_SW_TRACK_RUN)==LOW)return HIGH;else return LOW; 
          case IO_SW_SCALE_DIR: if(digitalRead(PIN_SW_SCALE_DIR)==LOW)return HIGH;else return LOW; 
          case IO_SW_SHIFT: if(digitalRead(PIN_SW_SHIFT)==LOW)return HIGH;else return LOW; 
          case IO_SW_RPT_PROB: if(digitalRead(PIN_SW_RPT_PROB)==LOW)return HIGH;else return LOW; 
          case IO_SW_LEN_MODE:
          { 
                unsigned int v =  analogRead(PIN_ANALOG_SW_LEN_MODE);
                if(v < 512)
                  return HIGH;
                else
                  return LOW;
            break;
          }
      }
    #else    
      switch(input)
      {
          case IO_SW_TRACK_RUN: return digitalRead(PIN_SW_TRACK_RUN); 
          case IO_SW_SCALE_DIR: return digitalRead(PIN_SW_SCALE_DIR); 
          case IO_SW_LEN_MODE: return digitalRead(PIN_SW_LEN_MODE); 
          case IO_SW_RPT_PROB: return digitalRead(PIN_SW_RPT_PROB); 
          case IO_SW_SHIFT:
          { 
                unsigned int v =  analogRead(PIN_ANALOG_SW_SHIFT);
                if(v < 512)
                  return LOW;
                else
                  return HIGH;
            break;
          }
      }
   #endif
}


void ios_setLo(int outNumber)
{
    switch(outNumber)
    {
      case IOS_GATE_1: digitalWrite(PIN_GATE_1,LOW);break;
      case IOS_GATE_2: digitalWrite(PIN_GATE_2,LOW);break;
      case IOS_GATE_3: digitalWrite(PIN_GATE_3,LOW);break;
      case IOS_GATE_4: digitalWrite(PIN_GATE_4,LOW);break;
      case IOS_MUX_A: digitalWrite(PIN_MUX_A,LOW);break;
      case IOS_MUX_B: digitalWrite(PIN_MUX_B_SR_DATA,LOW);break;
      case IOS_MUX_C: digitalWrite(PIN_MUX_C_SR_CLOCK,LOW);break;
      case IOS_SR_LATCH: digitalWrite(PIN_SR_LATCH,LOW);break;
    }
}
void ios_setHi(int outNumber)
{
    switch(outNumber)
    {
      case IOS_GATE_1: digitalWrite(PIN_GATE_1,HIGH);break;
      case IOS_GATE_2: digitalWrite(PIN_GATE_2,HIGH);break;
      case IOS_GATE_3: digitalWrite(PIN_GATE_3,HIGH);break;
      case IOS_GATE_4: digitalWrite(PIN_GATE_4,HIGH);break;
      case IOS_MUX_A: digitalWrite(PIN_MUX_A,HIGH);break;
      case IOS_MUX_B: digitalWrite(PIN_MUX_B_SR_DATA,HIGH);break;
      case IOS_MUX_C: digitalWrite(PIN_MUX_C_SR_CLOCK,HIGH);break;
      case IOS_SR_LATCH: digitalWrite(PIN_SR_LATCH,HIGH);break;
    } 
}
