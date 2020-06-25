#include <Arduino.h>
#include "ios.h"
#include "FrontPanel.h"
#include "RythmManager.h"
#include "TrackManager.h"
#include "Logic.h"
#include "MidiManager.h"



static volatile unsigned char tickDivider=0;
ISR(TIMER0_COMPA_vect) // timer0 interrupt. systick. 100uS
{
    // 100uS base time
    rthm_tick();
    //_______________
    
    // 1ms base time
    tickDivider++;
    if(tickDivider==10) 
    {
        tickDivider=0;
        frontp_tick1Ms();
        track_tick1ms();
        logic_tick1ms();
        midi_tickMs();
    }
    //_____________


}


void setup() {

  // Configure serial port for MIDI input
  //Serial.begin(31250); 
  //Serial.begin(57600); // debug
  //_____________________________________

  ios_init();
  frontp_init();
  rthm_init();
  track_init();
  logic_init();
  midi_init();
  
  // Configure TIMER0 for systick interrupt
  cli();//stop interrupts
  TCCR0A = 0;// set entire TCCR0A register to 0
  TCCR0B = 0;// same for TCCR0B
  TCNT0  = 0;//initialize counter value to 0
  OCR0A = 25; // 100uS 
  // turn on CTC mode
  TCCR0A |= (1 << WGM01);
  // Set CS01 and CS00 bits for 64 prescaler
  TCCR0B |= (1 << CS01) | (1 << CS00);   
  // enable timer compare interrupt
  TIMSK0 |= (1 << OCIE0A);
  sei();//allow interrupts
  //___________________________________

}

void loop() {

  // main loop
  while(1)
  {
      frontp_loop();
      rthm_loop();
      track_loop();
      logic_loop(); 
      midi_loop();
  }
  
}
