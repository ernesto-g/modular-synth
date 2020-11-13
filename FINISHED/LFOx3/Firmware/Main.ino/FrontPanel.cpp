#include "RotaryEncoder.h"
#include "Ios.h"

#define SW_STATE_IDLE           0
#define SW_STATE_WAIT_RELEASE   1

#define LED0  0
#define LED1  1
#define LED2  2
#define LED3  3
#define LED4  4


static RotaryEncoder encoder(ios_getEncoderPinA(),ios_getEncoderPinB());
static uint8_t encoderSwState=0;
static uint8_t swState=SW_STATE_IDLE;

// The Interrupt Service Routine for Pin Change Interrupt 2
// This routine will only be called on any signal change on D4 and D5: exactly where we need to check.
ISR(PCINT2_vect) {
  encoder.tick(); // just call tick() to check the state.

}

void fp_init(void)
{

}


void fp_loop(void)
{
    switch(swState)
    {
        case SW_STATE_IDLE:
        {
          if(ios_getEncoderSw()==0)
          {
              encoderSwState=1;
              swState=SW_STATE_WAIT_RELEASE;
          }
          break;
        }
        case SW_STATE_WAIT_RELEASE:
        {
          if(ios_getEncoderSw()==1)
          {
              swState=SW_STATE_IDLE;              
          }
          break;
        }
    }

}

uint16_t fp_getPotValue(uint8_t index)
{
    return ios_getAnalogValue(index);
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
  int val = ios_getAnalogValue(3); //3: switch mode analog input

  if(val>=0 && val<340)
      return 0;  // mode Phase

  if(val>=340 && val<680)
      return 1;  // mode sequencer
  
  if(val>=680)
      return 2; // mode free
      
}

void fp_showLfoInLeds(uint8_t lfoIndex)
{
    ios_setLed(LED3,0); //digitalWrite(PIN_LED3,0);
    ios_setLed(LED4,0); //digitalWrite(PIN_LED4,0);
    switch(lfoIndex)
    {
        case 0: 
        {
          ios_setLed(LED0,1); //digitalWrite(PIN_LED0,1);
          ios_setLed(LED1,0); //digitalWrite(PIN_LED1,0);
          ios_setLed(LED2,0); //digitalWrite(PIN_LED2,0);
          break;
        }
        case 1: 
        {
          ios_setLed(LED0,0); //digitalWrite(PIN_LED0,0);
          ios_setLed(LED1,1); //digitalWrite(PIN_LED1,1);
          ios_setLed(LED2,0); //digitalWrite(PIN_LED2,0);
          break;
        }
        case 2: 
        {
          ios_setLed(LED0,0); //digitalWrite(PIN_LED0,0);
          ios_setLed(LED1,0); //digitalWrite(PIN_LED1,0);
          ios_setLed(LED2,1); //digitalWrite(PIN_LED2,1);
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
          ios_setLed(LED0,1);
          ios_setLed(LED1,0);
          ios_setLed(LED2,0);
          ios_setLed(LED3,0);
          ios_setLed(LED4,0);
          break;
        }
        case 1: 
        {
          ios_setLed(LED0,0);
          ios_setLed(LED1,1);
          ios_setLed(LED2,0);
          ios_setLed(LED3,0);
          ios_setLed(LED4,0);
          break;
        }
        case 2: 
        {
          ios_setLed(LED0,0);
          ios_setLed(LED1,0);
          ios_setLed(LED2,1);
          ios_setLed(LED3,0);
          ios_setLed(LED4,0);
          break;
        }
        case 3:
        {
          ios_setLed(LED0,0);
          ios_setLed(LED1,0);
          ios_setLed(LED2,0);
          ios_setLed(LED3,1);
          ios_setLed(LED4,0);          
          break;
        }
        case 4:
        {
          ios_setLed(LED0,0);
          ios_setLed(LED1,0);
          ios_setLed(LED2,0);
          ios_setLed(LED3,0);
          ios_setLed(LED4,1);          
          break;
        }        
    }
}

void fp_showStepsInLeds(uint8_t steps)
{
  if((steps&0x01)==0x01)
      ios_setLed(LED0,1);
  else
      ios_setLed(LED0,0);
  
  if((steps&0x02)==0x02)
      ios_setLed(LED1,1);
  else
      ios_setLed(LED1,0);

  if((steps&0x04)==0x04)
      ios_setLed(LED2,1);
  else
      ios_setLed(LED2,0);

  if((steps&0x08)==0x08)
      ios_setLed(LED3,1);
  else
      ios_setLed(LED3,0);

  if((steps&0x10)==0x10)
      ios_setLed(LED4,1);
  else
      ios_setLed(LED4,0);
}
