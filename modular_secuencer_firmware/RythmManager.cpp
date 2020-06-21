#include <Arduino.h>
#include "RythmManager.h"
#include "TrackManager.h"



#define SEC_TO_TICK(S)  (S*10000)

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


// Private functions
static int calculateNextStep(unsigned char trackIndex);
static int incStep(unsigned char indexTrack);
static int subStep(unsigned char indexTrack);



void rthm_tick(void)
{
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
  }

  randomSeed(analogRead(A7));
  
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
  unsigned char trackIndex;
  for(trackIndex=0; trackIndex<TRACKS_LEN; trackIndex++)
  {
    stepIndex[trackIndex]=-1;
  }
  
    flagPlay=1;
}

int rthm_getState(void)
{
  return flagPlay;
}


void rthm_nextDirection(void)
{
    int currentTrack=track_getCurrentTrack();
    currentDirection[currentTrack]++;
    if(currentDirection[currentTrack]>DIR_RAND_2)
      currentDirection[currentTrack]=DIR_FORWARD;
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
            if(calculateNextStep(trackIndex)>=0)
              track_playStep(rthm_getCurrentStep(trackIndex),trackIndex);  
            else      
              track_silenceStep(-1,trackIndex);
        }        
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
            break;  
        }
        case DIR_SKIP:
        {
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
