#include <Arduino.h>
#include "RythmManager.h"
#include "TrackManager.h"
#include "Logic.h"
#include "ios.h"
#include "Memory.h"



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
static unsigned char prevDirection[TRACKS_LEN];
static unsigned char pendulumDir[TRACKS_LEN];
static unsigned char skipCounter[TRACKS_LEN];
static volatile unsigned char randomSeedCounter=0;
static unsigned char flagPendingNextDir[TRACKS_LEN]; // change of direction occurs only after step 1
static unsigned char clockDivisor[TRACKS_LEN];
static unsigned char clockCounter[TRACKS_LEN];
static unsigned char currentClkSrc;
static unsigned char currentRstMode;
static volatile unsigned char thereWasRisingEdgeClk=0;
static volatile unsigned char thereWasRisingEdgeRst=0;
static volatile unsigned char externalRstState=1;
static unsigned char rstCounter;


// Private functions
static int calculateNextStep(unsigned char trackIndex);
static int incStep(unsigned char indexTrack);
static int subStep(unsigned char indexTrack);
static void loadNextDirection(int currentTrack);
static void reverseCurrentDirection(unsigned char trackIndex);
static void restoreCurrentDirection(unsigned char trackIndex);



void rthm_tick(void)
{
    randomSeedCounter++;
  
    if(tempoCounter>0)
      tempoCounter--;
    
}

static void extClkInterrupt(void)
{
    if(ios_readClkIn()==0)
    {
        thereWasRisingEdgeClk=1;
    }
}

static void extRstInterrupt(void)
{
    externalRstState = ios_readRstIn();
    if(externalRstState==0)
    {
        thereWasRisingEdgeRst=1;         
    }
}

void rthm_init(void)
{
  rthm_setTempo(50); 
   
  flagPlay=0;

  currentClkSrc=CONFIG_CLK_SRC_INT;
  currentRstMode = CONFIG_RST_BHV_BACK2ONE;
  rstCounter=0;
  
  unsigned char trackIndex;
  for(trackIndex=0; trackIndex<TRACKS_LEN; trackIndex++)
  {
      // load initial values from eeprom
      currentDirection[trackIndex]=mem_getDirection(trackIndex); //DIR_FORWARD;
      clockDivisor[trackIndex]= mem_getClkDiv(trackIndex); //1; 
      trackEndStep[trackIndex]=mem_getLen(trackIndex); //8;
      //_________________________________
      
      // init aux arrays
      pendulumDir[trackIndex]=0;
      skipCounter[trackIndex]=0;
      flagPendingNextDir[trackIndex]=0;
      clockCounter[trackIndex]=0;
      //________________
  }

  ios_configureInterruptForExtClk(extClkInterrupt);
  ios_configureInterruptForExtRst(extRstInterrupt);
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
  tempoCounter=0;
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

    mem_saveDirection(dirToReturn,currentTrack);    
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

    mem_saveClkDiv(clockDivisor[currentTrack],currentTrack);
        
    return clockDivisor[currentTrack]-1;
}

int rthm_nextLen(void)
{
    int currentTrack=track_getCurrentTrack();

    trackEndStep[currentTrack]++;
    if(trackEndStep[currentTrack]>=9)
      trackEndStep[currentTrack]=2;

    mem_saveLen(trackEndStep[currentTrack],currentTrack);

    return trackEndStep[currentTrack]-1; // led1 is 0, so I substract 1 to value
}
int rthm_getCurrentLen(void)
{
    int currentTrack=track_getCurrentTrack();  
    return trackEndStep[currentTrack]-1; // led1 is 0, so I substract 1 to value  
}


  
void rthm_loop(void)
{  
    unsigned char flagStepEvent=0;

    // Clock Management
    if(currentClkSrc == CONFIG_CLK_SRC_INT)
    {
        // Internal clock
        if(tempoCounter==0)
        {
            tempoCounter = currentTempoTicks;
            flagStepEvent=1;
        }
    }
    else
    {
        // External clock
        if(thereWasRisingEdgeClk==1)
        {
            thereWasRisingEdgeClk=0;
            flagStepEvent=1;
        }
    }
    //____________________________________________
    

    // Reset management
    switch(currentRstMode)
    {
        case CONFIG_RST_BHV_BACK2ONE:
        {
            if(thereWasRisingEdgeRst==1)
            {
                thereWasRisingEdgeRst=0;
                // back to step one in all tracks
                unsigned char trackIndex;
                for(trackIndex=0; trackIndex<TRACKS_LEN; trackIndex++)
                {
                   stepIndex[trackIndex]=-1;
                   clockCounter[trackIndex]=0;
                }
            }
            break;
        }
        case CONFIG_RST_BHV_HOLD_WHILE_HI:
        {
            if(externalRstState==0)
            {
                return ; // skip code "Play steps"
            }
            break;
        }
        case CONFIG_RST_BHV_INVER_DIR:
        {
            if(thereWasRisingEdgeRst==1)
            {
                thereWasRisingEdgeRst=0;
                rstCounter++;
                if(rstCounter==1)
                {
                    unsigned char trackIndex;
                    for(trackIndex=0; trackIndex<TRACKS_LEN; trackIndex++)              
                        reverseCurrentDirection(trackIndex);
                }
                else if(rstCounter==2)
                {
                    rstCounter=0;
                    unsigned char trackIndex;
                    for(trackIndex=0; trackIndex<TRACKS_LEN; trackIndex++)              
                        restoreCurrentDirection(trackIndex);
                }
            }
            break;
        }
    }
    //____________________________________________


    // Play steps
    if(flagStepEvent==1 && flagPlay==1)
    {
        flagStepEvent=0;
      
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
            /*
            if(trackIndex==0 && logic_getCurrentMode()!=LOGIC_MODE_0_4TRACKS)
            {
                continue;           
            }*/
            //___________________________________________________
            
            if(calculateNextStep(trackIndex)>=0)
              track_playStep(rthm_getCurrentStep(trackIndex),trackIndex);  
            else      
              track_silenceStep(-1,trackIndex);
        }        
    }
    //____________________________________________-

}


void rthm_setClkSrc(unsigned char clkSrc)
{
    currentClkSrc = clkSrc;
}
unsigned char rthm_getClkSrc(void)
{
    return currentClkSrc;
}
unsigned char rthm_nextClkSrc(void)
{
    currentClkSrc++;
    if(currentClkSrc>CONFIG_CLK_SRC_EXT)
        currentClkSrc=CONFIG_CLK_SRC_INT;

    return currentClkSrc;
}


void rthm_setRstMode(unsigned char mode)
{
    currentRstMode = mode;
}
unsigned char rthm_getRstMode(void)
{
    return currentRstMode;
}
unsigned char rthm_nextRstMode(void)
{
    currentRstMode++;
    if(currentRstMode>CONFIG_RST_BHV_INVER_DIR)
        currentRstMode = CONFIG_RST_BHV_BACK2ONE;
    return currentRstMode;
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

static void reverseCurrentDirection(unsigned char trackIndex)
{
    prevDirection[trackIndex] = currentDirection[trackIndex]; // save current direction before change it
    switch(currentDirection[trackIndex])
    {
        case DIR_FORWARD:
        {
            currentDirection[trackIndex] = DIR_REVERSE;
            break;  
        }
        case DIR_REVERSE:
        {
            currentDirection[trackIndex] = DIR_FORWARD;
            break;  
        }
        case DIR_PENDULUM:
        {
            break;
        }
        case DIR_STAGGER:
        {
            currentDirection[trackIndex] = DIR_REVERSE;
            break;
        }
        case DIR_SKIP:
        {
            currentDirection[trackIndex] = DIR_REVERSE;
            break;
        }
        case DIR_RAND_1:
        {
            currentDirection[trackIndex] = DIR_FORWARD;
            break;  
        }
        case DIR_RAND_2:
        {
            currentDirection[trackIndex] = DIR_FORWARD;
            break;
        }        
    }
   switch(currentDirection[trackIndex])
   {
        case DIR_PENDULUM:pendulumDir[trackIndex]=0;break;
        case DIR_SKIP:skipCounter[trackIndex]=0;break;         
   }    
}
static void restoreCurrentDirection(unsigned char trackIndex)
{
    currentDirection[trackIndex] = prevDirection[trackIndex]; // restore current direction
    switch(currentDirection[trackIndex])
    {
        case DIR_PENDULUM:pendulumDir[trackIndex]=0;break;
        case DIR_SKIP:skipCounter[trackIndex]=0;break;         
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
