#include <Arduino.h>
#include "RythmManager.h"
#include "TrackManager.h"
#include "Logic.h"


#define SEC_TO_TICK(S)  (S*10000)
#define SEC_TO_MS(S)  (S*1000)


#define TRACKS_LEN      4


#define DIR_FORWARD   0
#define DIR_REVERSE   1
#define DIR_PENDULUM  2
#define DIR_STAGGER   3
#define DIR_SKIP      4
#define DIR_RAND_1    5
#define DIR_RAND_2    6




// Private variables
static unsigned char flagPlay;
static int currentTempo;
static unsigned int currentTempoTicks;
static volatile int tempoCounter;
static unsigned char stepIndex[TRACKS_LEN];
static unsigned char trackEndStep[TRACKS_LEN];
static unsigned char currentDirection[TRACKS_LEN];
static unsigned char pendulumDir[TRACKS_LEN];
static unsigned char skipCounter[TRACKS_LEN];
static volatile unsigned char randomSeedCounter=0;
static unsigned char flagPendingNextDir[TRACKS_LEN]; // change of direction occurs only after step 1
static unsigned char clockDivisor[TRACKS_LEN];
static unsigned char clockCounter[TRACKS_LEN];


// Private functions
static int calculateNextStep(unsigned char trackIndex);
static int incStep(unsigned char indexTrack);
static int subStep(unsigned char indexTrack);
static void loadNextDirection(int currentTrack);



void rthm_tick(void)
{
    randomSeedCounter++;
  
    if(tempoCounter>0)
      tempoCounter--;
    
}

void rthm_init(void)
{
  rthm_setTempo(200); 
   
  flagPlay=0;

  unsigned char trackIndex;
  for(trackIndex=0; trackIndex<TRACKS_LEN; trackIndex++)
  {
      trackEndStep[trackIndex]=8;
      currentDirection[trackIndex]=DIR_FORWARD;
      pendulumDir[trackIndex]=0;
      skipCounter[trackIndex]=0;
      flagPendingNextDir[trackIndex]=0;
      clockDivisor[trackIndex]=1; 
      clockCounter[trackIndex]=0;
  }
  
}



void rthm_setTempo(int tempo)
{
    currentTempo = tempo;
    currentTempoTicks = (unsigned int)SEC_TO_TICK( (60.0/tempo) );
}

int rthm_getCurrentTempo(void)
{
  return currentTempo;
}

unsigned int rthm_getCurrentTempoPeriodMs(void)
{
    return (unsigned int)SEC_TO_MS( (60.0/currentTempo) );
}

int rthm_getCurrentStep(unsigned char indexTrack)
{
  return stepIndex[indexTrack];
}

void rthm_stop(void)
{
    flagPlay=0;
}

void rthm_play(void)
{
  randomSeed(randomSeedCounter);

  unsigned char trackIndex;
  for(trackIndex=0; trackIndex<TRACKS_LEN; trackIndex++)
  {
    stepIndex[trackIndex]=-1;
    clockCounter[trackIndex]=0;
  }
  
  flagPlay=1;
}

int rthm_getState(void)
{
  return flagPlay;
}


int rthm_nextDirection(void)
{
    int currentTrack=track_getCurrentTrack();
    flagPendingNextDir[currentTrack]++;

    // calculate next dir to return
    int dirToReturn = currentDirection[currentTrack];
    int pressNumber = flagPendingNextDir[currentTrack];
    while(pressNumber>0)
    {
        dirToReturn++;
        pressNumber--;
        if(dirToReturn>DIR_RAND_2)
            dirToReturn=DIR_FORWARD;
    }
    return dirToReturn;
}
int rthm_getCurrentDirection(void)
{
    int currentTrack=track_getCurrentTrack();
    return currentDirection[currentTrack];
}

int rthm_getCurrentClockDivisor(void)
{
    int currentTrack=track_getCurrentTrack();
    return clockDivisor[currentTrack]-1;
}
int rthm_nextClockDivisor(void)
{
    int currentTrack=track_getCurrentTrack();
    clockDivisor[currentTrack]++;
    if(clockDivisor[currentTrack]>8)
        clockDivisor[currentTrack]=1;
        
    return clockDivisor[currentTrack]-1;
}

int rthm_nextLen(void)
{
    int currentTrack=track_getCurrentTrack();

    trackEndStep[currentTrack]++;
    if(trackEndStep[currentTrack]>=9)
      trackEndStep[currentTrack]=2;

    return trackEndStep[currentTrack]-1; // led1 is 0, so I substract 1 to value
}
int rthm_getCurrentLen(void)
{
    int currentTrack=track_getCurrentTrack();  
    return trackEndStep[currentTrack]-1; // led1 is 0, so I substract 1 to value  
}

void rthm_loop(void)
{  
    if(tempoCounter==0 && flagPlay==1)
    {
        tempoCounter = currentTempoTicks;

        // Check what tracks should be played
        unsigned char trackIndex;
        for(trackIndex=0; trackIndex<TRACKS_LEN; trackIndex++)
        {   
            // skip step by clockDivisor
            clockCounter[trackIndex]++;
            if(clockCounter[trackIndex]<clockDivisor[trackIndex])
            {
                continue;
            }
            else
              clockCounter[trackIndex]=0;
            //__________________________
            
            // Track 0 only works on "LOGIC_MODE_0_4TRACKS" mode
            if(trackIndex==0 && logic_getCurrentMode()!=LOGIC_MODE_0_4TRACKS)
            {
                continue;           
            }
            //___________________________________________________
            
            if(calculateNextStep(trackIndex)>=0)
              track_playStep(rthm_getCurrentStep(trackIndex),trackIndex);  
            else      
              track_silenceStep(-1,trackIndex);
        }        
    }
}


static void loadNextDirection(int currentTrack)
{
    
    currentDirection[currentTrack]++;
    if(currentDirection[currentTrack]>DIR_RAND_2)
      currentDirection[currentTrack]=DIR_FORWARD;

   switch(currentDirection[currentTrack])
   {
        case DIR_PENDULUM:pendulumDir[currentTrack]=0;break;
        case DIR_SKIP:skipCounter[currentTrack]=0;break;         
   }
}

static int calculateNextStep(unsigned char trackIndex)
{
    switch(currentDirection[trackIndex])
    {
        case DIR_FORWARD:
        {
            incStep(trackIndex); // Inc step
            break;  
        }
        case DIR_REVERSE:
        {
            subStep(trackIndex); // Sub step
            break;  
        }
        case DIR_PENDULUM:
        {
            if(pendulumDir[trackIndex]==0)
            {
                // FORWARD
                if(stepIndex[trackIndex]==(trackEndStep[trackIndex]-1))
                {
                    // Current step is the last one, start reverse
                    pendulumDir[trackIndex]=1;
                    subStep(trackIndex);                  
                }
                else
                  incStep(trackIndex);
                
            }
            else
            {
                // REVERSE
                if(stepIndex[trackIndex]==0)
                {
                    // Current step is the first one, start Forward
                    pendulumDir[trackIndex]=0;
                    incStep(trackIndex);
                }
                else
                  subStep(trackIndex);
            }
            break;  
        }
        case DIR_STAGGER:
        {
          int rndVal = random(0, 100);
          if(rndVal<25)
          {
              // 25% chance to enter here
              incStep(trackIndex);
              incStep(trackIndex);
          }
          else if(25<=rndVal && rndVal<50)
          {
              // 25 % chance to enter here          
              subStep(trackIndex);
          }
          else
          {
              // 50% chance to enter here
              incStep(trackIndex);
          }
          break;  
        }
        case DIR_SKIP:
        {
            incStep(trackIndex);
            if(stepIndex[trackIndex]==skipCounter[trackIndex])
            {
                skipCounter[trackIndex]++;
                if(skipCounter[trackIndex]>=trackEndStep[trackIndex])
                  skipCounter[trackIndex]=0;
                incStep(trackIndex);
            }
            break;  
        }
        case DIR_RAND_1:
        {
            stepIndex[trackIndex] = random(0, trackEndStep[trackIndex]);
            
            break;  
        }
        case DIR_RAND_2:
        {
            stepIndex[trackIndex] = random(0, trackEndStep[trackIndex]+1);
            if(stepIndex[trackIndex]==(trackEndStep[trackIndex]))
            {
                stepIndex[trackIndex] = -1; // silence
                return -1;            
            }
            break;  
        }        
    }

    if(stepIndex[trackIndex]==0)
    {
        // Sequence started from step 0, check pending changes on direction
        
        while(flagPendingNextDir[trackIndex]>0) // Check if there are pending changes on direction
        {
            loadNextDirection(trackIndex); // apply pending direction change
            flagPendingNextDir[trackIndex]--;
        }
    }
    
    return stepIndex[trackIndex];
}

static int incStep(unsigned char indexTrack)
{
    int ret=0;
    stepIndex[indexTrack]++;
    if(stepIndex[indexTrack]>=trackEndStep[indexTrack])
    {
        stepIndex[indexTrack]=0;
        ret=1;
    }      
    return ret;
}
static int subStep(unsigned char indexTrack)
{
    int ret=0;
    stepIndex[indexTrack]--;
    if(stepIndex[indexTrack]==255)
    {
        stepIndex[indexTrack]=trackEndStep[indexTrack]-1;
        ret=1;
    }      
    return ret;
}
