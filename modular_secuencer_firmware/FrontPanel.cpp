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

#define TIMEOUT_BOUNCE      10 // 10ms
#define TIMEOUT_SHORT_PRESS 1000  // 1sec
#define TIMEOUT_LONG_PRESS  2000  // 2sec

#define STATE_IDLE                  0
#define STATE_PRESSED               1
#define STATE_WAIT_BOUNCE           2
#define STATE_PRESS_CONFIRMED       3
#define STATE_WAIT_RELEASE          4
#define STATE_WAIT_RELEASE2         5
#define STATE_WAIT_BOUNCE_RELEASE   6


static RotaryEncoder encoder(PIN_ENCODER_D5, PIN_ENCODER_D4);
static unsigned char state[LEN_SW];
static unsigned char switchesState[LEN_SW];
static volatile unsigned int timeouts[LEN_SW];
static volatile int timeoutAnalogSt=0;
static int stepAnalogValues[ANALOGS_IN_LEN];
static unsigned char stepsLeds=0;


static int getIO(int swIndex);
static void swStateMachine(int swIndex);
static void analogsStateMachine(void);
static void setMuxValue(int val);
static void showLeds(unsigned char val);



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
}

// The Interrupt Service Routine for Pin Change Interrupt 2
// This routine will only be called on any signal change on D4 and D5: exactly where we need to check.
ISR(PCINT2_vect) {
  encoder.tick(); // just call tick() to check the state.

}

void frontp_init(void)
{
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
void frontp_showStepInLed(int stepIndex)
{
    stepsLeds = 0;
    if(stepIndex>=0 && stepIndex<=7)
    {
        stepsLeds|= 1 << stepIndex;    
    }          
}

int frontp_readAnalogStepValue(int index)
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
            }
            break; 
        }
        case STATE_PRESSED:
        {
            timeouts[swIndex]=0;
            state[swIndex] = STATE_WAIT_BOUNCE;            
            break;
        }
        case STATE_WAIT_BOUNCE:
        {
            if(timeouts[swIndex]>=TIMEOUT_BOUNCE)
            {
                if(ios_readSw(getIO(swIndex))==LOW)
                {
                    state[swIndex] = STATE_PRESS_CONFIRMED;             
                }
                else
                    state[swIndex] = STATE_IDLE;  
            }
            else
            {
                if(ios_readSw(getIO(swIndex))==HIGH)
                    state[swIndex] = STATE_IDLE; // bouncing   
            }
            break;
        }
        case STATE_PRESS_CONFIRMED:
        {
            // wait for short or long press
            timeouts[swIndex]=0;
            state[swIndex] = STATE_WAIT_RELEASE;
            switchesState[swIndex] = FRONT_PANEL_SW_STATE_JUST_PRESSED;
            break;
        }
        case STATE_WAIT_RELEASE:
        {
            if(ios_readSw(getIO(swIndex))==HIGH) // released
            {
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
            }          
            break; 
        }
        case STATE_WAIT_BOUNCE_RELEASE:
        {
            if(timeouts[swIndex]>=TIMEOUT_BOUNCE)
            {
              switchesState[swIndex] = FRONT_PANEL_SW_STATE_JUST_RELEASED;
              state[swIndex] = STATE_IDLE;
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
