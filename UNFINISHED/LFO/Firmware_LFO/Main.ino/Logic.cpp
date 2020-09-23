#include <Arduino.h>
#include "LFO.h"
#include "Logic.h"

#define POT_CHANGE_THRESHOLD  1

static uint8_t currentMode;


void log_init(void)
{

    currentMode = MODE_PHASE;
}

void log_loop(void)
{
  static int16_t pot0ValueOld = 0xFFFF;
  static int16_t pot1ValueOld = 0xFFFF;
  static int16_t pot2ValueOld = 0xFFFF;
  
  int16_t pot0Value = analogRead(A0); 
  int16_t pot1Value = analogRead(A1); 
  int16_t pot2Value = analogRead(A2); 

  switch(currentMode)
  {
      case MODE_FREE:
      {
          lfo_setFrequencyFrom_ADC(0,pot0Value);
          lfo_setFrequencyFrom_ADC(1,pot1Value);
          lfo_setFrequencyFrom_ADC(2,pot2Value);
          break;
      }
      case MODE_PHASE:
      {
          // same freq set by pot0
          lfo_setFrequencyFrom_ADC(0,pot0Value);
          lfo_setFrequencyFrom_ADC(1,pot0Value);
          lfo_setFrequencyFrom_ADC(2,pot0Value);

          // update phase
          if(pot1Value>(pot1ValueOld+POT_CHANGE_THRESHOLD) || pot1Value<(pot1ValueOld-POT_CHANGE_THRESHOLD) )
            lfo_setPhaseFromADC(1,pot1Value);
          
          if(pot2Value!=pot2ValueOld)
          lfo_setPhaseFromADC(2,pot2Value);
          
          break;
      }
  }


  pot0ValueOld = pot0Value;
  pot1ValueOld = pot1Value;
  pot2ValueOld = pot2Value;
  
}
