#include <Arduino.h>
#include "RythmManager.h"
#include "TrackManager.h"



#define SEC_TO_TICK(S)  (S*10000)

#define TRACKS_LEN      4






// Private variables
static unsigned char flagPlay;
static int currentTempo;
static unsigned int currentTempoTicks;
static volatile int tempoCounter;
static unsigned char stepIndex[TRACKS_LEN];
static unsigned char trackEndStep[TRACKS_LEN];


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


void rthm_incStep(unsigned char indexTrack)
{
    stepIndex[indexTrack]++;
    if(stepIndex[indexTrack]>=trackEndStep[indexTrack])
    {
        stepIndex[indexTrack]=0;
    }      
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
    flagPlay=1;
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
            rthm_incStep(trackIndex); // Inc step
            track_playStep(rthm_getCurrentStep(trackIndex),trackIndex);        
        }        
    }
}
