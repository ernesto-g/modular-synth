#include "LFO.h"
#include "Logic.h"

void setup() {

  pinMode(1, OUTPUT); // BOC

  lfo_init();
  log_init();

}




void loop() {

  lfo_loop();
  log_loop();
  
}
