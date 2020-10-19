/*  Modular 8 steps Sequencer
    Copyright (C) 2020  Ernesto Gigliotti <ernestogigliotti@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/    

#include <Arduino.h>
#include "RotaryEncoder.h"
#include "FrontPanel.h"
#include "ios.h"

#define PIN_ENCODER_D4          4
#define PIN_ENCODER_D5          5


#define ANALOGS_IN_LEN          8

#define ANALOG_STATE_SET_MUX    0
#define ANALOG_STATE_WAIT_MUX   1
#define ANALOG_STATE_VALUE_READ 2
#define ANALOG_STATE_SHOW_LEDS  3

#define LEN_SW          5
#define LEDS_LEN        8

#define TIMEOUT_BOUNCE      10 // 10ms
#define TIMEOUT_SHORT_PRESS 1000  // 1sec
#define TIMEOUT_LONG_PRESS  2000  // 2sec
#define LED_BLINK_TIMEOUT  200
#define LED_BLINK_TIMEOUT_FAST_OFF  10
#define LED_BLINK_TIMEOUT_FAST_ON  8


#define LED_STATE_BLINK_FAST    3
#define LED_STATE_BLINK         2
#define LED_STATE_ON            1
#define LED_STATE_OFF           0



#define STATE_IDLE                  0
#define STATE_PRESSED               1
#define STATE_WAIT_BOUNCE           2
#define STATE_PRESS_CONFIRMED       3
#define STATE_WAIT_RELEASE          4
#define STATE_WAIT_RELEASE2         5
#define STATE_WAIT_BOUNCE_RELEASE   6

// Private variables
static RotaryEncoder encoder(PIN_ENCODER_D5, PIN_ENCODER_D4);
static unsigned char state[LEN_SW];
static unsigned char switchesState[LEN_SW];
static volatile unsigned int timeouts[LEN_SW];
static volatile int timeoutAnalogSt=0;
static unsigned int stepAnalogValues[ANALOGS_IN_LEN];
static unsigned char stepsLeds=0;
static volatile unsigned char toggleForBlink;
static volatile unsigned int toggleForBlinkCounter;
static unsigned char ledsState[LEDS_LEN];

static volatile unsigned char toggleForBlinkFast;
static volatile unsigned int toggleForBlinkCounterFast;



// Private functions
static int getIO(int swIndex);
static void swStateMachine(int swIndex);
static void analogsStateMachine(void);
static void setMuxValue(int val);
static void showLeds(unsigned char val);
static void ledsStateMachine(void);



void frontp_tick1Ms(void)
{   
    unsigned char i;
    for(i=0; i<LEN_SW; i++)
    {
        if(timeouts[i]<=TIMEOUT_LONG_PRESS)
          timeouts[i]++;  
    }

    if(timeoutAnalogSt>0)
    {
      timeoutAnalogSt--;
    }

    if(toggleForBlinkCounter==0)
    {
        toggleForBlinkCounter=LED_BLINK_TIMEOUT;
        if(toggleForBlink)
          toggleForBlink=0;
        else
          toggleForBlink=1;
    }
    else
      toggleForBlinkCounter--;

    
    
    if(toggleForBlinkCounterFast==0)
    {
        if(toggleForBlinkFast){
          toggleForBlinkFast=0;
          toggleForBlinkCounterFast=LED_BLINK_TIMEOUT_FAST_OFF;
        }
        else{
          toggleForBlinkFast=1;
          toggleForBlinkCounterFast=LED_BLINK_TIMEOUT_FAST_ON;
        }
    }
    else
      toggleForBlinkCounterFast--;
}

// The Interrupt Service Routine for Pin Change Interrupt 2
// This routine will only be called on any signal change on D4 and D5: exactly where we need to check.
ISR(PCINT2_vect) {
  encoder.tick(); // just call tick() to check the state.

}

void frontp_init(void)
{
    //Serial.begin(115200); // debug
    //Serial.print("INICIO\n");
    toggleForBlinkCounter=LED_BLINK_TIMEOUT;
    toggleForBlink=1;
    
    pinMode(PIN_ENCODER_D4,INPUT_PULLUP);
    pinMode(PIN_ENCODER_D5,INPUT_PULLUP);
   
    // Enable interrupts for D4 D5 pins
    PCICR |= (1 << PCIE2);    // This enables Pin Change Interrupt 2 that covers D4 and D5
    PCMSK2 |= (1 << PCINT20) | (1 << PCINT21);  // This enables the interrupt for pin D4 and D5
}

int frontp_getEncoderPosition(void)
{
    return encoder.getPosition();
}

void frontp_setEncoderPosition(int pos)
{
    encoder.setPosition(pos);
}


void frontp_loop(void)
{    
    int i;
    for(i=0;i<LEN_SW; i++)
      swStateMachine(i);

    analogsStateMachine();

    ledsStateMachine();
}

int frontp_getSwState(int swIndex)
{
    return switchesState[swIndex];
}
void frontp_resetSwState(int swIndex)
{
    switchesState[swIndex]=FRONT_PANEL_SW_STATE_IDLE;
}

void frontp_setLed(int led, int value)
{
    if(value==1)
    {
        stepsLeds|= 1 << led;  
    }
    else
    {
        stepsLeds&= (~(1 << led));  
    }    
}

void frontp_showStepInLed(int stepIndex, int flagTurnOffOthers)
{ 
    if(flagTurnOffOthers)
    {
      int i;
      for(i=0; i<LEDS_LEN; i++)
        ledsState[i]=LED_STATE_OFF;
    }
    ledsState[stepIndex]=LED_STATE_ON;         
}
void frontp_showStepInLedBlinking(int stepIndex, int flagTurnOffOthers)
{
    if(flagTurnOffOthers)
    {
      int i;
      for(i=0; i<LEDS_LEN; i++)
        ledsState[i]=LED_STATE_OFF;
    }
    if(stepIndex>=0 && stepIndex<LEDS_LEN)
      ledsState[stepIndex]=LED_STATE_BLINK;      
}

void frontp_showBinaryInLedBlinking(int value, int flagTurnOffOthers)
{
    if(flagTurnOffOthers)
    {
      int i;
      for(i=0; i<LEDS_LEN; i++)
        ledsState[i]=LED_STATE_OFF;
    }
    if(value>=0 && value<=255)
    {
      if((value&0x01)!=0) ledsState[0]=LED_STATE_BLINK;        
      if((value&0x02)!=0) ledsState[1]=LED_STATE_BLINK;        
      if((value&0x04)!=0) ledsState[2]=LED_STATE_BLINK;        
      if((value&0x08)!=0) ledsState[3]=LED_STATE_BLINK;        
      if((value&0x10)!=0) ledsState[4]=LED_STATE_BLINK;        
      if((value&0x20)!=0) ledsState[5]=LED_STATE_BLINK;        
      if((value&0x40)!=0) ledsState[6]=LED_STATE_BLINK;        
      if((value&0x80)!=0) ledsState[7]=LED_STATE_BLINK;        
    }
}

unsigned int frontp_readAnalogStepValue(int index)
{
    switch(index) // map physically analog inputs to match leds steps
    {
      case 0: index=1;break;
      case 1: index=5;break;
      case 2: index=7;break;
      case 3: index=6;break;
      case 4: index=4;break;
      case 5: index=3;break;
      case 6: index=2;break;
      case 7: index=0;break;
    }
    return  stepAnalogValues[index];
}

void frontp_showEuclideanStateInLeds(unsigned char stepInTrack,unsigned char* euclideanSteps)
{
    int i;
    for(i=0; i<LEDS_LEN; i++)
    {
        if(euclideanSteps[i]==1)
          ledsState[i]= LED_STATE_ON;
        else
          ledsState[i]= LED_STATE_OFF;   
    }

    ledsState[stepInTrack]= LED_STATE_BLINK_FAST;
}








static int getIO(int swIndex)
{
    switch(swIndex)
    {
        case SW_TRACK_RUN:  return IO_SW_TRACK_RUN;
        case SW_SCALE_DIR:  return IO_SW_SCALE_DIR;
        case SW_LEN_MODE:   return IO_SW_LEN_MODE;
        case SW_RPT_PROB:   return IO_SW_RPT_PROB;        
        case SW_SHIFT:      return IO_SW_SHIFT;        
    }
    return -1;
}

static void ledsStateMachine(void)
{
    stepsLeds = 0; // TURN ALL LEDS OFF

    int stepIndex;
    for(stepIndex=0; stepIndex<LEDS_LEN; stepIndex++)
    {
        // TURN SOME LEDS ON
        if(ledsState[stepIndex]==LED_STATE_BLINK)
        {
            // blink led
            if(toggleForBlink)
            {
                frontp_setLed(stepIndex, 1);
            }
        }
        else if(ledsState[stepIndex]==LED_STATE_ON)
        {
            frontp_setLed(stepIndex, 1);
        }
        else if(ledsState[stepIndex]==LED_STATE_BLINK_FAST)
        {
            // blink led
            if(toggleForBlinkFast)
            {
                frontp_setLed(stepIndex, 1);
            }          
        }
    }
      
}

static void swStateMachine(int swIndex)
{
    switch(state[swIndex])
    {
        case STATE_IDLE:
        {
            if(ios_readSw(getIO(swIndex))==LOW)
            {
                // sw pressed
                state[swIndex] = STATE_PRESSED;
                //if(swIndex==4) Serial.print("1\n");
            }
            break; 
        }
        case STATE_PRESSED:
        {
            timeouts[swIndex]=0;
            state[swIndex] = STATE_WAIT_BOUNCE;            
            //if(swIndex==4) Serial.print("2\n");
            break;
        }
        case STATE_WAIT_BOUNCE:
        {
            if(timeouts[swIndex]>=TIMEOUT_BOUNCE)
            {
                if(ios_readSw(getIO(swIndex))==LOW)
                {
                    state[swIndex] = STATE_PRESS_CONFIRMED;
                    //if(swIndex==4) Serial.print("3\n");
             
                }
                else{
                    state[swIndex] = STATE_IDLE;
                    //if(swIndex==4) Serial.print("3B\n");
                }  
            }
            else
            {
                if(ios_readSw(getIO(swIndex))==HIGH){
                    state[swIndex] = STATE_IDLE; // bouncing
                    //if(swIndex==4) Serial.print("3C\n");
                }   
            }
            break;
        }
        case STATE_PRESS_CONFIRMED:
        {
            // wait for short or long press
            timeouts[swIndex]=0;
            state[swIndex] = STATE_WAIT_RELEASE;
            switchesState[swIndex] = FRONT_PANEL_SW_STATE_JUST_PRESSED;
            //if(swIndex==4) Serial.print("4\n");
            break;
        }
        case STATE_WAIT_RELEASE:
        {
            if(ios_readSw(getIO(swIndex))==HIGH) // released
            {
                //if(swIndex==4) Serial.print("5\n");
                // released, check time
                if(timeouts[swIndex]<TIMEOUT_SHORT_PRESS)
                    switchesState[swIndex] = FRONT_PANEL_SW_STATE_SHORT;
                else
                    switchesState[swIndex] = FRONT_PANEL_SW_STATE_LONG;
                // wait bounce again
                timeouts[swIndex]=0;
                state[swIndex] = STATE_WAIT_BOUNCE_RELEASE;                
            }
            if(timeouts[swIndex]>TIMEOUT_LONG_PRESS)
            {
                switchesState[swIndex] = FRONT_PANEL_SW_STATE_LONG;
                state[swIndex] = STATE_WAIT_RELEASE2;
                //if(swIndex==4) Serial.print("6\n");
            }
            
            break;
        }
        case STATE_WAIT_RELEASE2:
        {
            if(ios_readSw(getIO(swIndex))==HIGH) // released
            {
                // wait bounce again
                timeouts[swIndex]=0;
                state[swIndex] = STATE_WAIT_BOUNCE_RELEASE;
                //if(swIndex==4) Serial.print("7\n");
            }          
            break; 
        }
        case STATE_WAIT_BOUNCE_RELEASE:
        {
            if(timeouts[swIndex]>=TIMEOUT_BOUNCE)
            {
              switchesState[swIndex] = FRONT_PANEL_SW_STATE_JUST_RELEASED;
              state[swIndex] = STATE_IDLE;
              //if(swIndex==4) Serial.print("8\n");
            }    
            break;
        }
    }
}

static void setMuxValue(int val)
{
    switch(val)
    {
        case 0:ios_setLo(IOS_MUX_A);ios_setLo(IOS_MUX_B);ios_setLo(IOS_MUX_C);break; //000
        case 1:ios_setHi(IOS_MUX_A);ios_setLo(IOS_MUX_B);ios_setLo(IOS_MUX_C);break; //100
        case 2:ios_setLo(IOS_MUX_A);ios_setHi(IOS_MUX_B);ios_setLo(IOS_MUX_C);break; //010
        case 3:ios_setHi(IOS_MUX_A);ios_setHi(IOS_MUX_B);ios_setLo(IOS_MUX_C);break; //110
        case 4:ios_setLo(IOS_MUX_A);ios_setLo(IOS_MUX_B);ios_setHi(IOS_MUX_C);break; //001
        case 5:ios_setHi(IOS_MUX_A);ios_setLo(IOS_MUX_B);ios_setHi(IOS_MUX_C);break; //101
        case 6:ios_setLo(IOS_MUX_A);ios_setHi(IOS_MUX_B);ios_setHi(IOS_MUX_C);break; //011
        case 7:ios_setHi(IOS_MUX_A);ios_setHi(IOS_MUX_B);ios_setHi(IOS_MUX_C);break; //111
    }
}




void analogsStateMachine(void)
{
    static int readAnalogState=0;
    static int muxValue=0;
    
    switch(readAnalogState)
    {
        case ANALOG_STATE_SET_MUX:
        {
           setMuxValue(muxValue);        
           timeoutAnalogSt=2;
           readAnalogState = ANALOG_STATE_WAIT_MUX;
           break;
        }
        case ANALOG_STATE_WAIT_MUX:
        {
          if(timeoutAnalogSt==0)
          {
              stepAnalogValues[muxValue] = ios_readMuxAnalog();       
              readAnalogState = ANALOG_STATE_VALUE_READ;
          }
          break;
        }
        case ANALOG_STATE_VALUE_READ:
        {
          muxValue++;
          if(muxValue>=ANALOGS_IN_LEN)
          {
              muxValue = 0;
              readAnalogState = ANALOG_STATE_SHOW_LEDS;
          }
          else
          {
              readAnalogState=ANALOG_STATE_SET_MUX;
          }
          break;
        }
        case ANALOG_STATE_SHOW_LEDS:
        {
          // Show leds through shift register
          showLeds(stepsLeds);
          //_________________________________
          
          readAnalogState = ANALOG_STATE_SET_MUX;          
          break;
        }
    }
  
}

static void showLeds(unsigned char val)
{
    int i;
    ios_setLo(IOS_SR_LATCH);
    ios_setLo(IOS_SR_CLOCK);
    for(i=0; i<8 ; i++)
    {      
        if( (val&(0x80>>i)) == 0)      
        {
          ios_setLo(IOS_SR_DATA);          
        }
        else
        {
          ios_setHi(IOS_SR_DATA);                    
        }

      ios_setHi(IOS_SR_CLOCK);
      ios_setLo(IOS_SR_CLOCK);        
    }
    ios_setHi(IOS_SR_LATCH);
}
