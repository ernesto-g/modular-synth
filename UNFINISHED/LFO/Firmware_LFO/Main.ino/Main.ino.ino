#include "LFO.h"
#include "Logic.h"
#include "FrontPanel.h"

void setup() {

  pinMode(1, OUTPUT); // BOC

  lfo_init();
  fp_init();
  log_init();


}




void loop() {

  lfo_loop();
  fp_loop();
  log_loop();
  
}
