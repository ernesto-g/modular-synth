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

#include "ios.h"
#include "FrontPanel.h"
#include "RythmManager.h"
#include "TrackManager.h"
#include "Logic.h"
#include "MidiManager.h"
#include "Memory.h"

// Defines
#define LOGIC_STATE_SHOW_STEPS          0
#define LOGIC_STATE_SHOW_OPTION         1
#define LOGIC_STATE_SHOW_OPTION_BINARY  2
#define BPM_MIN   30
#define BPM_MAX   600

#define TIMEOUT_SHOW_OPTION         3000;
#define TIMEOUT_SHOW_OPTION_BINARY  5000;


// Private functions
static void showOption(int optionValue);
static void showOptionBinary(int optionValue);
static int imShowingOption(void);
static int nextMode(void);


// Private variables
static unsigned char flagShift;
static unsigned char dirBtnPressed;
static unsigned char logicState;
static volatile unsigned int timeoutShowCurrentOption;
static int currentOption;
static int currentMode;
static unsigned char flagConfiguringMidiChannel=0;
static unsigned char flagConfiguringClkSrc=0;
static unsigned char flagConfiguringRstMode=0;
static unsigned char flagConfiguringRootNote=0;
static volatile unsigned int timeoutPendingWriteTempoInMemory=0;
static volatile unsigned char flagPendingWriteTempoInMemory=0;

static unsigned char NOTE_IN_LEDS_TABLE[12]={
  0b00000001, // C
  0b10000001, // C#
  0b00000010, // D
  0b10000010, // D#
  0b00000100, // E
  0b00001000, // F
  0b10001000, // F#
  0b00010000, // G
  0b10010000, // G#
  0b00100000, // A
  0b10100000, // A#
  0b01000000, // B  
  };

void logic_tick1ms(void)
{
    if(timeoutShowCurrentOption>0)
      timeoutShowCurrentOption--;

    if(timeoutPendingWriteTempoInMemory>0)
      timeoutPendingWriteTempoInMemory--;
 
}

void logic_init(void)
{
    flagShift=0;
    dirBtnPressed=0;
    
    logicState = LOGIC_STATE_SHOW_STEPS;
    currentMode = mem_getMode();

    ios_init();
    frontp_init();
    rthm_init(mem_getTempo());
    track_init(mem_getCurrentTrack(),mem_getScale());
    midi_init();

    
    frontp_setEncoderPosition(rthm_getCurrentTempo());

    midi_setMidiChn(mem_getMidiChn());
    rthm_setClkSrc(mem_getClockMode());
    rthm_setRstMode(mem_getResetBehaviour());

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
          //Serial.print("SHON\n");
      }
      if(frontp_getSwState(SW_SHIFT)==FRONT_PANEL_SW_STATE_JUST_RELEASED)
      {
          frontp_resetSwState(SW_SHIFT);
          flagShift=0;
          //Serial.print("SHOFF\n");
      }

      if(frontp_getSwState(SW_SCALE_DIR)==FRONT_PANEL_SW_STATE_JUST_PRESSED)
      {
          dirBtnPressed=1;
      }
      if(frontp_getSwState(SW_SCALE_DIR)==FRONT_PANEL_SW_STATE_JUST_RELEASED)
      {
          dirBtnPressed=0;
      }
      

      if(frontp_getSwState(SW_TRACK_RUN)==FRONT_PANEL_SW_STATE_SHORT)
      {
          frontp_resetSwState(SW_TRACK_RUN);
          if(flagConfiguringMidiChannel==1)
          {
              // change midi channel
              int ch = midi_nextMidiChn();
              mem_saveMidiChn(ch);
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
                {
                    unsigned char t = track_nextTrack();
                    mem_saveCurrentTrack(t);
                    showOption(t);
                } 
                else
                    showOption(track_getCurrentTrack());
              }
          }
      }

      if(frontp_getSwState(SW_SCALE_DIR)==FRONT_PANEL_SW_STATE_SHORT)
      {
          frontp_resetSwState(SW_SCALE_DIR);
          if(flagConfiguringRootNote==1)
          {
              // change root note
              int note = track_nextRootNote();
              showOptionBinary(NOTE_IN_LEDS_TABLE[note]);
          }
          else
          {
              if(flagShift==0)
              {
                if(currentMode!=LOGIC_MODE_3_EUCLIDEAN_PLUS_3TRACKS)
                {
                  if(imShowingOption())
                      showOption(rthm_nextDirection()); 
                  else
                      showOption(rthm_getCurrentDirection());
                }
              }
              else
              {
                if(imShowingOption())
                {
                    unsigned char scale = track_nextScale();
                    mem_saveScale(scale);
                    showOption(scale); 
                }
                else
                    showOption(track_getCurrentScale());
              }
          }
      }

      if(frontp_getSwState(SW_RPT_PROB)==FRONT_PANEL_SW_STATE_SHORT)
      {
          frontp_resetSwState(SW_RPT_PROB);
          if(flagConfiguringClkSrc==1)
          {
              // Configure clk source
              unsigned char clkSrc = rthm_nextClkSrc();
              mem_saveClockMode(clkSrc);
              showOption(clkSrc);
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
              unsigned char rstBhv = rthm_nextRstMode();
              mem_saveResetBehaviour(rstBhv);
              showOption(rstBhv);          
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
      if(frontp_getSwState(SW_SCALE_DIR)==FRONT_PANEL_SW_STATE_LONG)
      {
          frontp_resetSwState(SW_SCALE_DIR);
          // set root note
          flagConfiguringRootNote=1;
          showOptionBinary(NOTE_IN_LEDS_TABLE[track_getCurrentRootNote()]);
      }      
      //____________________________________________
      

    // Encoder manager
    static int pos = 0;
    static int flagShift0=-1;
    static unsigned char dirBtnPressed0=-1;
    
    if(flagShift0!=flagShift)
    {
      flagShift0=flagShift;
      if(flagShift==0)
      {
          // load bpm in encoder
          pos = rthm_getCurrentTempo();
          frontp_setEncoderPosition(pos);
      }
      else
      {
          // load euclidean steps on in encoder
          pos = track_getCurrentEuclideanStepsOn();
          frontp_setEncoderPosition(pos);         
      }      
    }
    if(dirBtnPressed0!=dirBtnPressed)
    {
      dirBtnPressed0=dirBtnPressed;
      if(dirBtnPressed==1)
      {
          // load euclidean offset in encoder
          pos = track_getCurrentEuclideanOffset();
          frontp_setEncoderPosition(pos);                 
      }
      else
      {
          // load bpm in encoder
          pos = rthm_getCurrentTempo();
          frontp_setEncoderPosition(pos);
      }
    }
    
    if(flagShift==0)
    {
        if(dirBtnPressed==1)
        {
            // OFFSET control for euclidean mode
            if(currentMode==LOGIC_MODE_3_EUCLIDEAN_PLUS_3TRACKS)
            {
                int newPos = frontp_getEncoderPosition();
                if(newPos!=pos)
                {
                  if (newPos > pos)
                    track_nextEuclideanOffset();
                  else
                    track_prevEuclideanOffset();
                
                  pos=newPos;
                  frontp_resetSwState(SW_SCALE_DIR); // clean action for dir button
                }
            }              
        }
        else
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
              timeoutPendingWriteTempoInMemory=30000; // 30sec
              flagPendingWriteTempoInMemory=1;
            }
        }
    }
    else
    {
      // Euclidean offset control
      if(currentMode==LOGIC_MODE_3_EUCLIDEAN_PLUS_3TRACKS)
      {
        int newPos = frontp_getEncoderPosition();
        if(newPos!=pos)
        {
            if (newPos > pos)
              track_nextEuclideanStep();
            else
              track_prevEuclideanStep();

            pos=newPos;
        }
      }
    }
    //______________

    switch(logicState)
    {
        case LOGIC_STATE_SHOW_STEPS:
        {
            int stepInTrack = track_getCurrentStepInTrack();
            if(currentMode==LOGIC_MODE_3_EUCLIDEAN_PLUS_3TRACKS)
            {
                frontp_showEuclideanStateInLeds(stepInTrack,track_getEuclideanStepsState());
            }
            else
            {
                // show step led
                frontp_showStepInLed(stepInTrack,1);
            }            
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
              flagConfiguringRootNote=0;
            }
            break;
        }
        case LOGIC_STATE_SHOW_OPTION_BINARY:
        {
            frontp_showBinaryInLedBlinking(currentOption,1);
            if(timeoutShowCurrentOption==0)
            {
              logicState = LOGIC_STATE_SHOW_STEPS;
              // reset all config states
              flagConfiguringMidiChannel=0;
              flagConfiguringClkSrc=0;
              flagConfiguringRstMode=0;
              flagConfiguringRootNote=0;
            }
            break;
        }
    }


    // write tempo in eeprom
    if(timeoutPendingWriteTempoInMemory==0 && flagPendingWriteTempoInMemory==1)
    {
        flagPendingWriteTempoInMemory=0;
        mem_saveTempo(rthm_getCurrentTempo());
    }  
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
    // save new mode
    mem_saveMode(currentMode);
    return currentMode;
}

static void showOption(int optionValue)
{
    currentOption = optionValue;
    logicState = LOGIC_STATE_SHOW_OPTION;
    timeoutShowCurrentOption = TIMEOUT_SHOW_OPTION;
}
static void showOptionBinary(int optionValue)
{
    currentOption = optionValue;
    logicState = LOGIC_STATE_SHOW_OPTION_BINARY;
    timeoutShowCurrentOption = TIMEOUT_SHOW_OPTION_BINARY;
}

static int imShowingOption(void)
{
  if(logicState == LOGIC_STATE_SHOW_OPTION || logicState == LOGIC_STATE_SHOW_OPTION_BINARY)
      return 1;
  return 0;
}
