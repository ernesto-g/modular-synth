#include <Arduino.h>
#include "TrackManager.h"
#include "ios.h"
#include "FrontPanel.h"
#include "RythmManager.h"
#include "Logic.h"

#define ANALOG_TO_0_100(X)    (((unsigned long)(X/4)*(unsigned long)100UL)/255UL)
#define ANALOG_TO_0_255(X)    (X/4)

#define TRACK_0     0
#define TRACK_LEN   4
#define STEPS_LEN   8

#define SCALE_MODE_MICRO    0
#define SCALE_MODE_CHROM    1
#define SCALE_MODE_MAJOR    2
#define SCALE_MODE_MINOR    3
#define SCALE_MODE_BLUES    4
#define SCALE_MODE_PHRY     5
#define SCALE_MODE_LYDI     6
#define SCALE_MODE_DORI     7


#define GATE_STATE_IDLE         0
#define GATE_STATE_WAIT_FOR_LOW 1
#define GATE_STATE_LOW_FINISHED 2


// Private variables
static unsigned char currentTrack;
static unsigned char currentStepInTrack[TRACK_LEN];
static unsigned char currentScaleMode;
static unsigned char gateState[TRACK_LEN];
static volatile unsigned int timeoutGate[TRACK_LEN];

static unsigned int currentStepValue[TRACK_LEN][STEPS_LEN]; // Current prob assigned to each step
static unsigned int analogStepPrevValue[TRACK_LEN][STEPS_LEN]; // Last value read from potentiometer for each step


// Private functions
static void updateCVout(void);
static int calculateValueForCV(unsigned int val1024);
static void loadTimeoutForGate(int trackIndex);
static void loadCurrentAnalogValueForTrack(void);
static int getChance(int trackNumber);


//Scale tables
static const PROGMEM unsigned int CHROM_TABLE[60] = {136,144,152,161,171,181,192,203,215,228,242,256,271,287,304,322,342,362,383,406,430,456,483,512,542,575,609,645,683,724,767,813,861,912,966,1024,1085,1149,1217,1290,1367,1448,1534,1625,1722,1824,1933,2048,2169,2298,2435,2580,2733,2896,3068,3250,3443,3648,3865,4095};
static const PROGMEM unsigned int MAJOR_TABLE[35] = {136,152,171,181,203,228,228,271,304,342,362,406,456,512,542,609,683,724,813,912,1024,1085,1217,1367,1448,1625,1824,2048,2169,2435,2733,2896,3250,3648,4095};
static const PROGMEM unsigned int MINOR_TABLE[35] = {136,152,161,181,203,215,242,271,304,322,362,406,430,483,542,609,645,724,813,861,966 ,1085,1217,1290,1448,1625,1722,1933,2169,2435,2580,2896,3250,3443,3865};
static const PROGMEM unsigned int BLUES_TABLE[30] = {136,161,181,192,203,242,271,322,362,383,406,483,542,645,724,767,813,966,1085,1290,1448,1534,1625,1933,2169,2580,2896,3068,3250,3865};
static const PROGMEM unsigned int PHRY_TABLE[35] = {136,144,161,181,203,215,242,271,287,322,362,406,430,483,542,575,645,724,813,861,966,1085,1149,1290,1448,1625,1722,1933,2169,2298,2580,2896,3250,3443,3865};
static const PROGMEM unsigned int LYDI_TABLE[35] = {136,152,171,192,203,228,228,271,304,342,383,406,456,512,542,609,683,767,813,912,1024,1085,1217,1367,1534,1625,1824,2048,2169,2435,2733,3068,3250,3648,4095};
static const PROGMEM unsigned int DORI_TABLE[35] = {136,152,161,181,203,228,242,271,304,322,362,406,456,483,542,609,645,724,813,912,966,1085,1217,1290,1448,1625,1824,1933,2169,2435,2580,2896,3250,3648,3865};

void track_tick1ms(void)
{
    int i;

    for(i=0; i<TRACK_LEN; i++)
    {
        if(timeoutGate[i]>0)
        {
            timeoutGate[i]--;
        }
    }
}


void track_init(void)
{
    currentTrack=0;
    
    currentScaleMode=SCALE_MODE_MICRO;
    int i;
    int j;
    for(i=0; i<TRACK_LEN; i++)
    {
        gateState[i]=GATE_STATE_IDLE;
        timeoutGate[i]=0;

        if(i==0)
        {
            // track 0
            for(j=0; j<STEPS_LEN; j++)
            {
                currentStepValue[0][j] = ANALOG_TO_0_255(frontp_readAnalogStepValue(j));
                analogStepPrevValue[0][j] = currentStepValue[0][j];
            }
        }
        else
        {
          for(j=0; j<STEPS_LEN; j++)
            currentStepValue[i][j] = 100;
        }
    }
}


void track_silenceStep(int stepIndex, int trackIndex)
{
    currentStepInTrack[trackIndex] = stepIndex;
    
}
void track_playStep(int stepIndex, int trackIndex)
{
    currentStepInTrack[trackIndex] = stepIndex;
  
    switch(trackIndex)
    {
        case 0:
        {
            // Set velocity
            ios_setVelocityOut(127); // Max value
                        
            // Set V/OCT out
            updateCVout();            

            // Set GATE 1
            ios_setHi(IOS_GATE_1);

            loadTimeoutForGate(trackIndex);
            break;
        }  
        case 1:
        {
            if(getChance(IOS_GATE_2))
            {
                ios_setHi(IOS_GATE_2);
                loadTimeoutForGate(trackIndex);
            }
            break;
        }          
        case 2:
        {
            if(getChance(IOS_GATE_3))
            {
              ios_setHi(IOS_GATE_3);
              loadTimeoutForGate(trackIndex);
            }
            break;
        }          
        case 3:
        {
            if(getChance(IOS_GATE_4))
            {
              ios_setHi(IOS_GATE_4);
              loadTimeoutForGate(trackIndex);
            }
            break;
        }          
    }
}




void track_loop(void)
{
      // Check for changes on analog inputs
      int stepIndex;
      for(stepIndex=0; stepIndex<STEPS_LEN; stepIndex++)
      {
          unsigned int valueRead;
          if(currentTrack==0)
            valueRead = ANALOG_TO_0_255(frontp_readAnalogStepValue(stepIndex));
          else
            valueRead = ANALOG_TO_0_100(frontp_readAnalogStepValue(stepIndex));
          
          if(analogStepPrevValue[currentTrack][stepIndex] != valueRead)
          {
              // Value changed
              currentStepValue[currentTrack][stepIndex] = valueRead;
          }
      }
      //___________________________________


      // update analog in value and assign to function
      if(currentTrack==0 && logic_getCurrentMode()==LOGIC_MODE_0_4TRACKS)
        updateCVout();

      // Gate manager
      int i;
      for(i=0; i<TRACK_LEN; i++)
      {
          switch(gateState[i])
          {
              case GATE_STATE_IDLE:
              {
                  break;
              }
              case GATE_STATE_WAIT_FOR_LOW:
              {
                  if(timeoutGate[i]<=0)
                  {
                      switch(i)
                      {
                          case 0: ios_setLo(IOS_GATE_1);break;
                          case 1: ios_setLo(IOS_GATE_2);break;
                          case 2: ios_setLo(IOS_GATE_3);break;
                          case 3: ios_setLo(IOS_GATE_4);break;
                      }
                      gateState[i] = GATE_STATE_LOW_FINISHED;
                  }
                  break;
              }
              case GATE_STATE_LOW_FINISHED:
              {
                  gateState[i] = GATE_STATE_IDLE;
                  break;
              }
          }
      }    
}


int track_nextTrack(void)
{
    currentTrack++;
    if(currentTrack>=TRACK_LEN)
      currentTrack=0;

    loadCurrentAnalogValueForTrack();     
    return currentTrack;
}

static void loadCurrentAnalogValueForTrack(void)
{
    int stepIndex;
    for(stepIndex=0; stepIndex<STEPS_LEN; stepIndex++)
    {
        unsigned int valueRead;
        if(currentTrack==0)
          valueRead = ANALOG_TO_0_255(frontp_readAnalogStepValue(stepIndex));
        else
          valueRead = ANALOG_TO_0_100(frontp_readAnalogStepValue(stepIndex));
          
        analogStepPrevValue[currentTrack][stepIndex] = valueRead;
    }
}

static int getChance(int trackNumber)
{
    unsigned int prob = currentStepValue[trackNumber][currentStepInTrack[trackNumber]];
    
    if(prob>=98)
      return 1;
    if(prob<=2)
      return 0;
    
    int val = random(0, 100);
    if(val < prob )
        return 1;
    return 0;
}

int track_nextScale(void)
{
  currentScaleMode++;
  if(currentScaleMode>=8)
    currentScaleMode=0;

  return currentScaleMode;
}

int track_getCurrentTrack(void)
{
  return currentTrack;
}

int track_getCurrentStepInTrack(void)
{
    return currentStepInTrack[currentTrack];
}

int track_getCurrentScale(void)
{
  return currentScaleMode;
}

static void updateCVout(void)
{
    unsigned int val255 = currentStepValue[TRACK_0][currentStepInTrack[TRACK_0]]; //frontp_readAnalogStepValue(currentStepInTrack[TRACK_0]);

    //debug
    /*
    Serial.print("los 8 valores del track 0:");
    Serial.print(currentStepValue[TRACK_0][0]);Serial.print(" ");
    Serial.print(currentStepValue[TRACK_0][1]);Serial.print(" ");
    Serial.print(currentStepValue[TRACK_0][2]);Serial.print(" ");
    Serial.print(currentStepValue[TRACK_0][3]);Serial.print(" ");
    Serial.print(currentStepValue[TRACK_0][4]);Serial.print(" ");
    Serial.print(currentStepValue[TRACK_0][5]);Serial.print(" ");
    Serial.print(currentStepValue[TRACK_0][6]);Serial.print(" ");
    Serial.print(currentStepValue[TRACK_0][7]);Serial.print("\n");
    */
    ios_setCVout(calculateValueForCV(val255));
}


static int calculateValueForCV(unsigned int val255)
{    
    switch(currentScaleMode)
    {
        case SCALE_MODE_MICRO: {
          return val255*16;
        }
        case SCALE_MODE_CHROM: return pgm_read_word(&(CHROM_TABLE[(val255*59)/255])); 
        case SCALE_MODE_MAJOR: return pgm_read_word(&(MAJOR_TABLE[(val255*34)/255])); 
        case SCALE_MODE_MINOR: return pgm_read_word(&(MINOR_TABLE[(val255*34)/255]));
        case SCALE_MODE_BLUES: return pgm_read_word(&(BLUES_TABLE[(val255*29)/255]));
        case SCALE_MODE_PHRY : return pgm_read_word(&(PHRY_TABLE[(val255*34)/255]));
        case SCALE_MODE_LYDI : return pgm_read_word(&(LYDI_TABLE[(val255*34)/255]));
        case SCALE_MODE_DORI : return pgm_read_word(&(DORI_TABLE[(val255*34)/255]));
    }

    return 0;
}

static void loadTimeoutForGate(int trackIndex)
{
      unsigned int gateTimeout = rthm_getCurrentTempoPeriodMs() / 2; // Gate timeout = 50% tempo period
      
      timeoutGate[trackIndex] = gateTimeout;
      gateState[trackIndex] = GATE_STATE_WAIT_FOR_LOW; 
}
