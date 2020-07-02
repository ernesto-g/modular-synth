#include "ios.h"
#include "FrontPanel.h"
#include "RythmManager.h"
#include "TrackManager.h"
#include "Logic.h"
#include "MidiManager.h"

// Defines
#define LOGIC_STATE_SHOW_STEPS    0
#define LOGIC_STATE_SHOW_OPTION   1
#define BPM_MIN   30
#define BPM_MAX   600
#define TIMEOUT_SHOW_OPTION   3000;


// Private functions
static void showOption(int optionValue);
static int imShowingOption(void);
static int nextMode(void);


// Private variables
static unsigned char flagShift;
static unsigned char logicState;
static volatile unsigned int timeoutShowCurrentOption;
static int currentOption;
static int currentMode;
static Config devConfig;
static unsigned char flagConfiguringMidiChannel=0;
static unsigned char flagConfiguringClkSrc=0;
static unsigned char flagConfiguringRstMode=0;



void logic_tick1ms(void)
{
    if(timeoutShowCurrentOption>0)
      timeoutShowCurrentOption--;
  
}

void logic_init(void)
{
    flagShift=0;
    logicState = LOGIC_STATE_SHOW_STEPS;

    currentMode = LOGIC_MODE_0_4TRACKS;

    // Default config
    devConfig.clkSrc = CONFIG_CLK_SRC_INT;
    devConfig.midiChn = CONFIG_MIDI_CHN_0;
    devConfig.resetBehaviour = CONFIG_RST_BHV_BACK2ONE;
    //_______________


    ios_init();
    frontp_init();
    rthm_init();
    track_init();
    midi_init();

    
    frontp_setEncoderPosition(rthm_getCurrentTempo());

    midi_setMidiChn(devConfig.midiChn);
    rthm_setClkSrc(devConfig.clkSrc);
    rthm_setRstMode(devConfig.resetBehaviour);

}


void logic_loop(void)
{
      frontp_loop();
      rthm_loop();
      track_loop();
      midi_loop();


      if(frontp_getSwState(SW_SHIFT)==FRONT_PANEL_SW_STATE_JUST_PRESSED)
      {
          frontp_resetSwState(SW_SHIFT);
          flagShift=1;
      }
      if(frontp_getSwState(SW_SHIFT)==FRONT_PANEL_SW_STATE_JUST_RELEASED)
      {
          frontp_resetSwState(SW_SHIFT);
          flagShift=0;
      }
      

      if(frontp_getSwState(SW_TRACK_RUN)==FRONT_PANEL_SW_STATE_SHORT)
      {
          frontp_resetSwState(SW_TRACK_RUN);
          if(flagConfiguringMidiChannel==1)
          {
              // change midi channel
              int ch = midi_nextMidiChn();
              if(ch==0)
                ch=255;
              else
                ch=ch-1;
              showOption(ch);
          }
          else
          {
              if(flagShift==0)
              {
                if(rthm_getState()==0)
                    rthm_play();
                else
                    rthm_stop();
              }
              else
              {
                if(imShowingOption())
                    showOption(track_nextTrack()); 
                else
                    showOption(track_getCurrentTrack());
              }
          }
      }

      if(frontp_getSwState(SW_SCALE_DIR)==FRONT_PANEL_SW_STATE_SHORT)
      {
          frontp_resetSwState(SW_SCALE_DIR);
          if(flagShift==0)
          {
            if(imShowingOption())
                showOption(rthm_nextDirection()); 
            else
                showOption(rthm_getCurrentDirection());
          }
          else
          {
            if(imShowingOption())
                showOption(track_nextScale()); 
            else
                showOption(track_getCurrentScale());
          }
      }

      if(frontp_getSwState(SW_RPT_PROB)==FRONT_PANEL_SW_STATE_SHORT)
      {
          frontp_resetSwState(SW_RPT_PROB);
          if(flagConfiguringClkSrc==1)
          {
              // Configure clk source
              showOption(rthm_nextClkSrc());
          }
          else
          {
              if(flagShift==0)
              {
                if(imShowingOption())
                    showOption(rthm_nextClockDivisor()); 
                else
                    showOption(rthm_getCurrentClockDivisor());
              }
              else
              {
                if(imShowingOption())
                    showOption(track_nextProbRptMode()); 
                else
                    showOption(track_getCurrentProbRptMode());
              }
          }
      }

      if(frontp_getSwState(SW_LEN_MODE)==FRONT_PANEL_SW_STATE_SHORT)
      {
          frontp_resetSwState(SW_LEN_MODE);
          if(flagConfiguringRstMode==1)
          {
              // Set rst in mode
              showOption(rthm_nextRstMode());          
          }
          else
          {
              if(flagShift==0)
              {
                if(imShowingOption())
                    showOption(rthm_nextLen()); 
                else
                    showOption(rthm_getCurrentLen());
              }
              else
              {
                if(imShowingOption())
                    showOption(nextMode()); 
                else
                    showOption(logic_getCurrentMode());
              }
          }
      }

      // Advance Configurations (long press buttons)
      if(frontp_getSwState(SW_TRACK_RUN)==FRONT_PANEL_SW_STATE_LONG)
      {
          frontp_resetSwState(SW_TRACK_RUN);
          // Set midi channel
          flagConfiguringMidiChannel=1;
          int ch = midi_getMidiChn();
          if(ch==0)
            showOption(255);
          else
            showOption(ch-1);        
      }
      if(frontp_getSwState(SW_RPT_PROB)==FRONT_PANEL_SW_STATE_LONG)
      {
          frontp_resetSwState(SW_RPT_PROB);
          // set clock source (ext/int)
          flagConfiguringClkSrc=1;
          showOption(rthm_getClkSrc());
      }
      if(frontp_getSwState(SW_LEN_MODE)==FRONT_PANEL_SW_STATE_LONG)
      {
          frontp_resetSwState(SW_LEN_MODE);
          // Set reset in mode
          flagConfiguringRstMode=1;
          showOption(rthm_getRstMode());          
      }
      //____________________________________________
      

    // Encoder manager
    static int pos = 0;
    if(flagShift==0)
    {
      // BPM control
      int newPos = frontp_getEncoderPosition();
      if(newPos<BPM_MIN){
          newPos=BPM_MIN;
          frontp_setEncoderPosition(BPM_MIN);
      }
      if(newPos>=BPM_MAX)
      {
         newPos=BPM_MAX;
         frontp_setEncoderPosition(BPM_MAX);          
      }
      if (pos != newPos) {
        pos = newPos;
        rthm_setTempo(newPos);
      }
    }
    else
    {
      // Euclidean offset control
      // TODO
    }
    //______________

    switch(logicState)
    {
        case LOGIC_STATE_SHOW_STEPS:
        {
            int stepInTrack = track_getCurrentStepInTrack();
            // show step led
            frontp_showStepInLed(stepInTrack,1);            
            break;
        }
        case LOGIC_STATE_SHOW_OPTION:
        {
            frontp_showStepInLedBlinking(currentOption,1);
            if(timeoutShowCurrentOption==0)
            {
              logicState = LOGIC_STATE_SHOW_STEPS;
              // reset all config states
              flagConfiguringMidiChannel=0;
              flagConfiguringClkSrc=0;
              flagConfiguringRstMode=0;
            }
            break;
        }
    }
}

Config* logic_getConfig(void)
{
    return &devConfig;
}

int logic_getCurrentMode(void)
{
  return currentMode;
}
static int nextMode(void)
{
    currentMode++;
    if(currentMode>LOGIC_MODE_3_EUCLIDEAN_PLUS_3TRACKS)
    {
        currentMode = LOGIC_MODE_0_4TRACKS;
    }
    return currentMode;
}

static void showOption(int optionValue)
{
    currentOption = optionValue;
    logicState = LOGIC_STATE_SHOW_OPTION;
    timeoutShowCurrentOption = TIMEOUT_SHOW_OPTION;
}

static int imShowingOption(void)
{
  if(logicState == LOGIC_STATE_SHOW_OPTION)
      return 1;
  return 0;
}
