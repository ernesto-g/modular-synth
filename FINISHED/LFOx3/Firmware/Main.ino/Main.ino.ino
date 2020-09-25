#include "LFO.h"
#include "Logic.h"
#include "FrontPanel.h"
#include "Ios.h"

void setup() 
{
  ios_init();
  lfo_init();
  fp_init();
  log_init();
}


void loop() 
{
  lfo_loop();
  fp_loop();
  log_loop(); 
}
