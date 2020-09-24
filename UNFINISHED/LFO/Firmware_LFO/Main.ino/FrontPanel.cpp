#include <Arduino.h>

void fp_init(void)
{
  
}


void fp_loop(void)
{
  
}

uint16_t fp_getPotValue(uint8_t index)
{
  switch(index)
  {
    case 0: return analogRead(A0); 
    case 1: return analogRead(A1); 
    case 2: return analogRead(A2); 
  }
  return 0;  
}
