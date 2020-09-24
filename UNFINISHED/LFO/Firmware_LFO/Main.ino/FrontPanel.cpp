#include <Arduino.h>
#include "RotaryEncoder.h"

#define SW_STATE_IDLE           0
#define SW_STATE_WAIT_RELEASE   1

#define PIN_ENCODER_D4          4
#define PIN_ENCODER_D5          5
#define PIN_ENCODER_D6          6

#define PIN_LED0             7
#define PIN_LED1             8
#define PIN_LED2            10
#define PIN_LED3            12
#define PIN_LED4            13


static RotaryEncoder encoder(PIN_ENCODER_D4, PIN_ENCODER_D5);
static uint8_t encoderSwState=0;
static uint8_t swState=SW_STATE_IDLE;

// The Interrupt Service Routine for Pin Change Interrupt 2
// This routine will only be called on any signal change on D4 and D5: exactly where we need to check.
ISR(PCINT2_vect) {
  encoder.tick(); // just call tick() to check the state.

}

void fp_init(void)
{
    pinMode(PIN_ENCODER_D4,INPUT_PULLUP); // Encoder A
    pinMode(PIN_ENCODER_D5,INPUT_PULLUP); // Encoder B
    pinMode(PIN_ENCODER_D6,INPUT_PULLUP); // Encoder SW

    pinMode(PIN_LED0,OUTPUT);
    pinMode(PIN_LED1,OUTPUT);
    pinMode(PIN_LED2,OUTPUT);
    pinMode(PIN_LED3,OUTPUT);
    pinMode(PIN_LED4,OUTPUT);
    
   
    // Enable interrupts for D4 D5 pins
    PCICR |= (1 << PCIE2);    // This enables Pin Change Interrupt 2 that covers D4 and D5
    PCMSK2 |= (1 << PCINT20) | (1 << PCINT21);  // This enables the interrupt for pin D4 and D5
}


void fp_loop(void)
{
    switch(swState)
    {
        case SW_STATE_IDLE:
        {
          if(digitalRead(PIN_ENCODER_D6)==0)
          {
              encoderSwState=1;
              swState=SW_STATE_WAIT_RELEASE;
          }
          break;
        }
        case SW_STATE_WAIT_RELEASE:
        {
          if(digitalRead(PIN_ENCODER_D6)==1)
          {
              swState=SW_STATE_IDLE;              
          }
          break;
        }
    }

}

uint16_t fp_getPotValue(uint8_t index)
{
  switch(index)
  {
    case 0: return analogRead(A0); 
    case 1: return analogRead(A1); 
    case 2: return analogRead(A2); 
  }
  return 0;  
}

int fp_getEncoderPosition(void)
{
    return encoder.getPosition();
}

void fp_setEncoderPosition(int pos)
{
    encoder.setPosition(pos);
}

int fp_getEncoderSw(void)
{
    int r =  encoderSwState;
    encoderSwState=0;
    return r;
}

uint8_t fp_getModeSwitch(void)
{
  int val = analogRead(A3);

  if(val>=0 && val<340)
      return 0;  // mode free
      
  if(val>=340 && val<680)
      return 1;  // mode sequencer
  
  if(val>=680)
      return 2; // mode phases
      
}

void fp_showLfoInLeds(uint8_t lfoIndex)
{
    digitalWrite(PIN_LED3,0);
    digitalWrite(PIN_LED4,0);
    switch(lfoIndex)
    {
        case 0: 
        {
          digitalWrite(PIN_LED0,1);
          digitalWrite(PIN_LED1,0);
          digitalWrite(PIN_LED2,0);
          break;
        }
        case 1: 
        {
          digitalWrite(PIN_LED0,0);
          digitalWrite(PIN_LED1,1);
          digitalWrite(PIN_LED2,0);
          break;
        }
        case 2: 
        {
          digitalWrite(PIN_LED0,0);
          digitalWrite(PIN_LED1,0);
          digitalWrite(PIN_LED2,1);
          break;
        }
    }
}


void fp_showWaveTypeInLeds(uint8_t wave)
{
    switch(wave)
    {
        case 0: 
        {
          digitalWrite(PIN_LED0,1);
          digitalWrite(PIN_LED1,0);
          digitalWrite(PIN_LED2,0);
          digitalWrite(PIN_LED3,0);
          digitalWrite(PIN_LED4,0);
          break;
        }
        case 1: 
        {
          digitalWrite(PIN_LED0,0);
          digitalWrite(PIN_LED1,1);
          digitalWrite(PIN_LED2,0);
          digitalWrite(PIN_LED3,0);
          digitalWrite(PIN_LED4,0);
          break;
        }
        case 2: 
        {
          digitalWrite(PIN_LED0,0);
          digitalWrite(PIN_LED1,0);
          digitalWrite(PIN_LED2,1);
          digitalWrite(PIN_LED3,0);
          digitalWrite(PIN_LED4,0);
          break;
        }
        case 3:
        {
          digitalWrite(PIN_LED0,0);
          digitalWrite(PIN_LED1,0);
          digitalWrite(PIN_LED2,0);
          digitalWrite(PIN_LED3,1);
          digitalWrite(PIN_LED4,0);          
          break;
        }
        case 4:
        {
          digitalWrite(PIN_LED0,0);
          digitalWrite(PIN_LED1,0);
          digitalWrite(PIN_LED2,0);
          digitalWrite(PIN_LED3,0);
          digitalWrite(PIN_LED4,1);          
          break;
        }        
    }
}
