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
#include "TrackManager.h"
#include "ios.h"
#include "FrontPanel.h"
#include "RythmManager.h"
#include "Logic.h"
#include "MidiManager.h"
#include "Memory.h"



#define ANALOG_TO_0_100(X)    (((unsigned long)(X/4)*(unsigned long)100UL)/255UL)
#define ANALOG_TO_0_255(X)    (X/4)
#define SCALE_100_TO_8(X)     ( (X*(unsigned int)8)/100 )

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
#define GATE_STATE_WAIT_FOR_HI  3
#define GATE_STATE_HI_FINISHED  4
              

// Private variables
static unsigned char currentTrack;
static unsigned char currentStepInTrack[TRACK_LEN];
static unsigned char currentScaleMode;
static unsigned char gateState[TRACK_LEN];
static volatile unsigned int timeoutGate[TRACK_LEN];
static unsigned char gateRepeatitionsCounter[TRACK_LEN];

static unsigned int currentNoteProbValue[TRACK_LEN][STEPS_LEN]; // Current prob assigned to each step CAMBIAR NOMBRE A PROBABILITY!!!!!!!!!!!!!!!!
static unsigned int analogStepPrevValue[TRACK_LEN][STEPS_LEN]; // Last value read from potentiometer for each step
static unsigned int currentRepeatitionsValue[TRACK_LEN][STEPS_LEN];
static unsigned char gateRepeatitionsMax[TRACK_LEN];
static unsigned char flagRepeatProbability;

static unsigned char euclideanSteps[STEPS_LEN];
static unsigned char euclideanStepsOn;
static int euclideanStepsOffset;


// Private functions
static void updateCVout(void);
static int calculateValueForCV(unsigned int val1024);
static void loadTimeoutForGate(int trackIndex);
static void loadCurrentAnalogValueForTrack(void);
static int getChance(int trackNumber);
static unsigned int calculateHiTimeoutForRepeatitions(int trackIndex);
static unsigned char getMaxRepeatitions(int trackIndex);
static unsigned int calculateLoTimeoutForRepeatitions(int trackIndex);
static void arpeggiatorManager(void);

static void euclideanInit(void);
static unsigned char hasToPlayStepInEuclideanMode(unsigned char trackIndex,unsigned char stepIndex);
static void rotateEuclideanStepsLeft (void);
static void rotateEuclideanStepsRight (void);
static void reloadEuclideanSteps(void);



//Scale tables
static const PROGMEM unsigned int CHROM_TABLE[60] = {68,137,205,273,341,410,478,546,614,682,751,819,887,956,1024,1092,1160,1229,1297,1365,1433,1501,1570,1638,1706,1775,1843,1911,1979,2048,2116,2184,2252,2320,2389,2457,2525,2594,2662,2730,2798,2867,2935,3003,3071,3139,3208,3276,3344,3413,3481,3549,3617,3686,3754,3822,3890,3958,4027,4095};
static const PROGMEM unsigned int MAJOR_TABLE[35] = {68,205,341,410,546,682,819,887,1024,1160,1229,1365,1501,1638,1706,1843,1979,2048,2184,2320,2457,2525,2662,2798,2867,3003,3139,3276,3344,3481,3617,3686,3822,3958,4095};
static const PROGMEM unsigned int MINOR_TABLE[35] = {68,205,273,410,546,614,751,887,1024,1092,1229,1365,1433,1570,1706,1843,1911,2048,2184,2252,2389,2525,2662,2730,2867,3003,3071,3208,3344,3481,3549,3686,3822,3890,4027};
static const PROGMEM unsigned int BLUES_TABLE[30] = {68,273,410,478,546,751,887,1092,1229,1297,1365,1570,1706,1911,2048,2116,2184,2389,2525,2730,2867,2935,3003,3208,3344,3549,3686,3754,3822,4027};
static const PROGMEM unsigned int PHRY_TABLE[35] = {68,137,273,410,546,614,751,887,956,1092,1229,1365,1433,1570,1706,1775,1911,2048,2184,2252,2389,2525,2594,2730,2867,3003,3071,3208,3344,3413,3549,3686,3822,3890,4027};
static const PROGMEM unsigned int LYDI_TABLE[35] = {68,205,341,478,546,682,819,887,1024,1160,1297,1365,1501,1638,1706,1843,1979,2116,2184,2320,2457,2525,2662,2798,2935,3003,3139,3276,3344,3481,3617,3754,3822,3958,4095};
static const PROGMEM unsigned int DORI_TABLE[35] = {68,205,273,410,546,682,751,887,1024,1092,1229,1365,1501,1570,1706,1843,1911,2048,2184,2320,2389,2525,2662,2730,2867,3003,3139,3208,3344,3481,3549,3686,3822,3958,4027};

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


void track_init(unsigned char initTrack,unsigned char initScale)
{
    int i;
    int j;

    euclideanInit();

    if(initTrack<TRACK_LEN)
      currentTrack=initTrack;
    else
      currentTrack=TRACK_0;

    if(initScale>SCALE_MODE_DORI)
      initScale = SCALE_MODE_MICRO;
  
    flagRepeatProbability=0; // init on probability mode    
    currentScaleMode=initScale;
    
    for(i=0; i<TRACK_LEN; i++)
    {
        gateState[i]=GATE_STATE_IDLE;
        timeoutGate[i]=0;
        gateRepeatitionsCounter[i]=0;

        if(i==0)
        {
            // track 0
            for(j=0; j<STEPS_LEN; j++)
            {
                currentNoteProbValue[0][j] = ANALOG_TO_0_255(frontp_readAnalogStepValue(j));
                analogStepPrevValue[0][j] = currentNoteProbValue[0][j];
                currentRepeatitionsValue[0][j]=1; // for track 0 repetitions are always 1
            }
        }
        else
        {
          for(j=0; j<STEPS_LEN; j++){
            currentNoteProbValue[i][j] = 100; // tracks 1 to 3, probability=100, repetitions=1
            currentRepeatitionsValue[i][j]=1;
          }
        }
    }
}


void track_silenceStep(int stepIndex, int trackIndex)
{
    if(trackIndex==TRACK_0 & (logic_getCurrentMode()==LOGIC_MODE_1_MIDI_PLUS_3TRACKS))
    {
        return; // ignore event. track 0 works with midi input 
    }
    
    currentStepInTrack[trackIndex] = stepIndex;
    
}
void track_playStep(int stepIndex, int trackIndex)
{
    if(trackIndex==TRACK_0 && (logic_getCurrentMode()==LOGIC_MODE_1_MIDI_PLUS_3TRACKS))
    {
        return; // ignore event. track 0 works with midi input 
    }

    if(trackIndex==TRACK_0 && (logic_getCurrentMode()==LOGIC_MODE_2_ARP_PLUS_3TRACKS))
    {
        if(midi_thereAreNoKeysPressed()==1)
        {
            return; // in arpeggiator mode, if there are no keys pressed, gate is not triggered
        }
    }
  
    currentStepInTrack[trackIndex] = stepIndex;

    // euclidean mode
    if((logic_getCurrentMode()==LOGIC_MODE_3_EUCLIDEAN_PLUS_3TRACKS))
    {
        if(hasToPlayStepInEuclideanMode(trackIndex,stepIndex)==0)
          return; 
    }
    //_______________
    
  
    switch(trackIndex)
    {
        case 0:
        {
            // if control is set to 0, dont trigger
            if(currentNoteProbValue[trackIndex][stepIndex]<=0)
            {
                break;
            }
            //______________________________________
            
            // Set velocity
            ios_setVelocityOut(127); // Max value
                        
            // Set V/OCT out
            updateCVout();            

            // Set GATE 1
            ios_setHi(IOS_GATE_1);

            loadTimeoutForGate(trackIndex);
            gateRepeatitionsCounter[trackIndex] = 0;
            break;
        }  
        case 1:
        {
            if(getChance(IOS_GATE_2))
            {
                ios_setHi(IOS_GATE_2);
                loadTimeoutForGate(trackIndex);
                gateRepeatitionsCounter[trackIndex] = 0;
            }
            break;
        }          
        case 2:
        {
            if(getChance(IOS_GATE_3))
            {
              ios_setHi(IOS_GATE_3);
              loadTimeoutForGate(trackIndex);
              gateRepeatitionsCounter[trackIndex] = 0;
            }
            break;
        }          
        case 3:
        {
            if(getChance(IOS_GATE_4))
            {
              ios_setHi(IOS_GATE_4);
              loadTimeoutForGate(trackIndex);
              gateRepeatitionsCounter[trackIndex] = 0;
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
              analogStepPrevValue[currentTrack][stepIndex] = valueRead;
            
              // Value changed
              if(currentTrack==0)
                currentNoteProbValue[currentTrack][stepIndex] = valueRead;
              else
              {
                if(flagRepeatProbability==1)
                  currentRepeatitionsValue[currentTrack][stepIndex]=SCALE_100_TO_8(valueRead);
                else
                  currentNoteProbValue[currentTrack][stepIndex] = valueRead;
              }
              //
          }
      }
      //___________________________________


      // update analog in value and assign to function
      if(currentTrack==0 && logic_getCurrentMode()==LOGIC_MODE_0_4TRACKS)
        updateCVout();
      //______________________________________________


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
                  gateRepeatitionsCounter[i]++;
                  if(gateRepeatitionsCounter[i]>=gateRepeatitionsMax[i])
                    gateState[i] = GATE_STATE_IDLE;
                  else
                  {
                      // reload gate pulse
                      gateState[i] = GATE_STATE_WAIT_FOR_HI; // used on repeatitions
                      timeoutGate[i]=calculateHiTimeoutForRepeatitions(i);
                  }
                  break;
              }
              case GATE_STATE_WAIT_FOR_HI:
              {
                  if(timeoutGate[i]<=0)
                  {
                      switch(i)
                      {
                          case 0: ios_setHi(IOS_GATE_1);break;
                          case 1: ios_setHi(IOS_GATE_2);break;
                          case 2: ios_setHi(IOS_GATE_3);break;
                          case 3: ios_setHi(IOS_GATE_4);break;
                      }
                      gateState[i] = GATE_STATE_HI_FINISHED;
                  }
                  break;
              }
              case GATE_STATE_HI_FINISHED:
              {
                  loadTimeoutForGate(i);   
                  break;
              }
          }// switch
      } // for    
} // fn


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
    unsigned int prob = currentNoteProbValue[trackNumber][currentStepInTrack[trackNumber]];
    
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

int track_nextProbRptMode(void)
{
    if(flagRepeatProbability==0)
      flagRepeatProbability=1;
    else
      flagRepeatProbability=0;

    loadCurrentAnalogValueForTrack();  

    return flagRepeatProbability;
}

int track_getCurrentProbRptMode(void)
{
    return flagRepeatProbability;
}

static void updateCVout(void)
{
    unsigned int val255;
    
    if(logic_getCurrentMode()==LOGIC_MODE_2_ARP_PLUS_3TRACKS)
    {
        // Arpeggiator mode,          
        unsigned char keyNote = midi_getNextKeyForRepeat();
        if(keyNote!=0xFF)
            ios_setCVout(midi_calculateValueForCV(keyNote));
    }
    else
    {
        val255 = currentNoteProbValue[TRACK_0][currentStepInTrack[TRACK_0]];
        ios_setCVout(calculateValueForCV(val255));
    }    
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


static unsigned char getMaxRepeatitions(int trackIndex)
{
    unsigned char currentStep = currentStepInTrack[trackIndex];
    unsigned char repMax = currentRepeatitionsValue[trackIndex][currentStep];
    if(repMax<1)
      repMax=1;
    else if(repMax>8)
      repMax=8;
      
    return repMax;
}
static unsigned int calculateHiTimeoutForRepeatitions(int trackIndex)
{
    unsigned int gateTimeout;
    gateTimeout = rthm_getCurrentTempoPeriodMs() / 2; // Gate timeout = 50% tempo period. No Repeatitions
    
    if(gateRepeatitionsMax[trackIndex]>1)
    {
        // Repeatitions enabled
        gateTimeout = gateTimeout / 8; 
    }
    
    return gateTimeout;    
}
static unsigned int calculateLoTimeoutForRepeatitions(int trackIndex)
{
    unsigned int gateTimeout;
    gateTimeout = rthm_getCurrentTempoPeriodMs() / 2; // Gate timeout = 50% tempo period.
    
    if(gateRepeatitionsMax[trackIndex]>1)
    {
        // Repeatitions enabled
        gateTimeout = gateTimeout / (gateRepeatitionsMax[trackIndex]+1); 
    }
    
    return gateTimeout;  
}

static void loadTimeoutForGate(int trackIndex)
{
      gateRepeatitionsMax[trackIndex] = getMaxRepeatitions(trackIndex);
      
      timeoutGate[trackIndex] = calculateLoTimeoutForRepeatitions(trackIndex);
      gateState[trackIndex] = GATE_STATE_WAIT_FOR_LOW;       
}





//************************************ EUCLIDEAN MODE **************************************************************************************
static void euclideanInit(void)
{
  euclideanStepsOn=mem_getEuclideanStepsOn();
  euclideanStepsOffset=mem_getEuclideanStepsOffset();

  reloadEuclideanSteps();
}

unsigned char* track_getEuclideanStepsState(void)
{
    return euclideanSteps;
}

unsigned char track_getCurrentEuclideanStepsOn(void)
{
    return euclideanStepsOn;
}

int track_getCurrentEuclideanOffset(void)
{
    return euclideanStepsOffset;
}

int track_nextEuclideanOffset(void)
{
    euclideanStepsOffset++;
    if(euclideanStepsOffset>=8)
      euclideanStepsOffset=7;
      
    reloadEuclideanSteps();
    mem_saveEuclideanStepsOffset(euclideanStepsOffset);
    return euclideanStepsOffset;
}
int track_prevEuclideanOffset(void)
{
    euclideanStepsOffset--;
    if(euclideanStepsOffset<=-8)
      euclideanStepsOffset=-7;

    reloadEuclideanSteps();
    mem_saveEuclideanStepsOffset(euclideanStepsOffset);
    return euclideanStepsOffset;
}

unsigned char track_nextEuclideanStep(void)
{
    euclideanStepsOn++;
    if(euclideanStepsOn>=9)
      euclideanStepsOn=8; 

    reloadEuclideanSteps();
    mem_saveEuclideanStepsOn(euclideanStepsOn);
    return euclideanStepsOn;
}

unsigned char track_prevEuclideanStep(void)
{
    euclideanStepsOn--;
    if(euclideanStepsOn==0)
      euclideanStepsOn=1; 

    reloadEuclideanSteps();
    mem_saveEuclideanStepsOn(euclideanStepsOn);
    return euclideanStepsOn;
}



static unsigned char hasToPlayStepInEuclideanMode(unsigned char trackIndex,unsigned char stepIndex)
{
    if(trackIndex==0)
    {
        if(euclideanSteps[stepIndex]==1)
          return 1;
        else
          return 0;        
    }
    return 1;
}

static void reloadEuclideanSteps(void)
{
  int i;
  euclideanSteps[0]=0;
  euclideanSteps[1]=0;
  euclideanSteps[2]=0;
  euclideanSteps[3]=0;
  euclideanSteps[4]=0;
  euclideanSteps[5]=0;
  euclideanSteps[6]=0;
  euclideanSteps[7]=0;
  
  switch(euclideanStepsOn)
  {
      case 1:  euclideanSteps[0]=1;break;
      case 2:  euclideanSteps[0]=1;euclideanSteps[4]=1;break;
      case 3:  euclideanSteps[0]=1;euclideanSteps[3]=1;euclideanSteps[6]=1;break;
      case 4:  euclideanSteps[0]=1;euclideanSteps[2]=1;euclideanSteps[4]=1;euclideanSteps[6]=1;break;
      case 5:  
        euclideanSteps[0]=1;
        euclideanSteps[1]=1;
        euclideanSteps[2]=1;
        euclideanSteps[4]=1;
        euclideanSteps[6]=1;
        break;
      case 6:  
        euclideanSteps[0]=1;
        euclideanSteps[1]=1;
        euclideanSteps[2]=1;
        euclideanSteps[4]=1;
        euclideanSteps[5]=1;
        euclideanSteps[6]=1;
        break;
      case 7:  
        euclideanSteps[0]=1;
        euclideanSteps[1]=1;
        euclideanSteps[2]=1;
        euclideanSteps[3]=1;
        euclideanSteps[4]=1;
        euclideanSteps[5]=1;
        euclideanSteps[6]=1;
        break;
      case 8:  
        euclideanSteps[0]=1;
        euclideanSteps[1]=1;
        euclideanSteps[2]=1;
        euclideanSteps[3]=1;
        euclideanSteps[4]=1;
        euclideanSteps[5]=1;
        euclideanSteps[6]=1;
        euclideanSteps[7]=1;
      break;
  }  
  if(euclideanStepsOffset>0)
  {
      // right rotation
      for(i=0; i<euclideanStepsOffset; i++)
        rotateEuclideanStepsRight();
  }
  else if(euclideanStepsOffset<0)
  {
      // left rotation
      int maxI = euclideanStepsOffset*(-1);
      for(i=0; i<maxI; i++)
        rotateEuclideanStepsLeft();
  }  
}

static void rotateEuclideanStepsRight (void)
{
    unsigned char aux;
    unsigned char i;
    
    aux = euclideanSteps[7];
    for (i=7; i>0; i--)
    {
        euclideanSteps[i] = euclideanSteps[i-1];
    }
    euclideanSteps[0] = aux;
}

static void rotateEuclideanStepsLeft (void)
{
    unsigned char aux;
    unsigned char i;
    
    aux = euclideanSteps[0];
    for (i=0; i<7; i++)
    {
        euclideanSteps[i] = euclideanSteps[i+1];
    }
    euclideanSteps[7] = aux;
}

//___________________________________________________________________________________________________________________________________________
