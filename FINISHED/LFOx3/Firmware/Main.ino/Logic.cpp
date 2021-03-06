#include <Arduino.h>
#include "LFO.h"
#include "Logic.h"
#include "FrontPanel.h"
#include "Ios.h"
#include "Memory.h"

#define LOGIC_STATE_SELECT_LFO      0
#define LOGIC_STATE_SELECT_WAVEFORM 1

#define POT_CHANGE_THRESHOLD  1

static uint8_t currentMode;
static int8_t selectedLfo=0;
static uint8_t state;
static uint8_t flagDisableShowLfo;
static volatile uint16_t timeoutEnableShowLfo=0;

static void sysTick(void) // System interrupt ~1ms
{
    if(timeoutEnableShowLfo>0)
    {
      timeoutEnableShowLfo--;
      if(timeoutEnableShowLfo==0)
        flagDisableShowLfo=0; // Show lfo selection in leds again
    }
}

void log_init(void)
{
    state = LOGIC_STATE_SELECT_LFO;
    currentMode = MODE_PHASE;
    fp_setEncoderPosition(selectedLfo);
    lfo_setSystickCallback(sysTick);
    flagDisableShowLfo=0;

    
    lfo_setWaveType(0,mem_readWaveform(0));
    lfo_setWaveType(1,mem_readWaveform(1));
    lfo_setWaveType(2,mem_readWaveform(2));

    randomSeed(fp_getPotValue(1));
}

void log_loop(void)
{
  static int16_t pot0ValueOld = 0xFFFF;
  static int16_t pot1ValueOld = 0xFFFF;
  static int16_t pot2ValueOld = 0xFFFF;
  static uint8_t clkOld=1;

  
  int16_t pot0Value = fp_getPotValue(0); 
  int16_t pot1Value = fp_getPotValue(1); 
  int16_t pot2Value = fp_getPotValue(2); 

  switch(state)
  {
      case LOGIC_STATE_SELECT_LFO:
      {
        selectedLfo = fp_getEncoderPosition();
        if(selectedLfo>=3){
          selectedLfo=0;
          fp_setEncoderPosition(selectedLfo);
        }
        if(selectedLfo<0){  
          selectedLfo=2;
          fp_setEncoderPosition(selectedLfo);
        }

        if(flagDisableShowLfo==0)
          fp_showLfoInLeds(selectedLfo);
          
        if(fp_getEncoderSw())
        {
            state = LOGIC_STATE_SELECT_WAVEFORM;            
            fp_setEncoderPosition(lfo_getWaveType(selectedLfo));
        }
        break;
      }
      case LOGIC_STATE_SELECT_WAVEFORM:
      {
        int16_t wave = fp_getEncoderPosition();
        if(wave>=5){
          wave=0;
          fp_setEncoderPosition(wave);
        }
        if(wave<0){
          wave=4;
          fp_setEncoderPosition(wave);
        }

        fp_showWaveTypeInLeds(wave);
        lfo_setWaveType(selectedLfo,wave);
        

        if(fp_getEncoderSw())
        {
            state = LOGIC_STATE_SELECT_LFO;
            fp_setEncoderPosition(selectedLfo);
            // save in eeprom
            mem_writeWaveform(selectedLfo,wave);
        }        
        break;
      }
  }

  currentMode = fp_getModeSwitch();

  switch(currentMode)
  {
      case MODE_FREE:
      {
          lfo_setMode(LFO_FREE_MODE);
        
          lfo_setFrequencyFrom_ADC(0,pot0Value);
          lfo_setFrequencyFrom_ADC(1,pot1Value);
          lfo_setFrequencyFrom_ADC(2,pot2Value);
          break;
      }
      case MODE_PHASE:
      {
          lfo_setMode(LFO_PHASE_MODE);

          // same freq for all LFOs, set by pot0
          lfo_setFrequencyFrom_ADC(0,pot0Value);
          lfo_setFrequencyFrom_ADC(1,pot0Value);
          lfo_setFrequencyFrom_ADC(2,pot0Value);

          // update phase
          if(pot1Value>(pot1ValueOld+POT_CHANGE_THRESHOLD) || pot1Value<(pot1ValueOld-POT_CHANGE_THRESHOLD) )
            lfo_setPhaseFromADC(1,pot1Value);
          
          if(pot2Value>(pot2ValueOld+POT_CHANGE_THRESHOLD) || pot2Value<(pot2ValueOld-POT_CHANGE_THRESHOLD) )
            lfo_setPhaseFromADC(2,pot2Value);
          //_____________
          
          break;
      }
      case MODE_SEQUENCER:
      {
          lfo_setMode(LFO_SEQ_MODE);

          int16_t pot0Value32 = pot0Value/32;
          int16_t pot0Value32Old = pot0ValueOld/32;          
          int16_t pot1Value32 = pot1Value/32;
          int16_t pot2Value32 = pot2Value/32;
          int16_t pot1Value32Old = pot1ValueOld/32;
          int16_t pot2Value32Old = pot2ValueOld/32;

          if(pot0Value32!=pot0Value32Old)
          {
            lfo_setSteps(0,pot0Value32);
            flagDisableShowLfo=1;
            timeoutEnableShowLfo=4000;
            if(state!=LOGIC_STATE_SELECT_WAVEFORM)
              fp_showStepsInLeds(pot0Value32);
          }            
          
          if(pot1Value32!=pot1Value32Old)
          {
            lfo_setSteps(1, pot1Value32);
            flagDisableShowLfo=1;
            timeoutEnableShowLfo=4000;
            if(state!=LOGIC_STATE_SELECT_WAVEFORM)
              fp_showStepsInLeds(pot1Value32);            
          }
          if(pot2Value32!=pot2Value32Old)
          {
            lfo_setSteps(2, pot2Value32);
            flagDisableShowLfo=1;
            timeoutEnableShowLfo=4000;
            if(state!=LOGIC_STATE_SELECT_WAVEFORM)
              fp_showStepsInLeds(pot2Value32);            
          }
        
         
          break;
      }
  }

  // Clk Input detection
    if(ios_getClkIn()==0)
    {
      if(clkOld==1)
      {
        clkOld=0;
        switch(currentMode)
        {
            case MODE_FREE: lfo_clkEventForFreeMode();break;
            case MODE_PHASE: lfo_clkEventForPhaseMode(pot1Value,pot2Value);break;
            case MODE_SEQUENCER: lfo_clkEventForSequencerMode();break;
        } 
      }
    }
    else
    {
      clkOld=1;                  
    }
  //____________________


  pot0ValueOld = pot0Value;
  pot1ValueOld = pot1Value;
  pot2ValueOld = pot2Value;
  
}
