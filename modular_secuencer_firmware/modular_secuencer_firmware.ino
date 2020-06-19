#include <Arduino.h>
#include "ios.h"
#include "FrontPanel.h"
#include "RythmManager.h"
#include "TrackManager.h"

static volatile int prueba=0;

static volatile unsigned char tickDivider=0;
//ISR(TIMER1_COMPA_vect) // timer1 interrupt. systick. 100uS
ISR(TIMER0_COMPA_vect)
{
    // 100uS base time
    rthm_tick();
    //inst_tick();
    //_______________
    
    // 1ms base time
    tickDivider++;
    if(tickDivider==10) 
    {
        tickDivider=0;
        frontp_tick1Ms();
        //midi_tickMs();
        prueba++;
    }
    //_____________


}


void setup() {

  // Configure serial port for MIDI input
  //Serial.begin(31250); 
  Serial.begin(57600); // debug
  //_____________________________________

  ios_init();
  frontp_init();
  rthm_init();
  track_init();
  
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

  int toggle=0;

  // main loop
  while(1)
  {
      frontp_loop();
      rthm_loop();
      track_loop();

      
      if(frontp_getSwState(SW_TRACK_RUN)==FRONT_PANEL_SW_STATE_SHORT)
      {
          frontp_resetSwState(SW_TRACK_RUN);
          Serial.print("Se presiono RUN\n");
          rthm_play();
      }

      if(frontp_getSwState(SW_SCALE_DIR)==FRONT_PANEL_SW_STATE_SHORT)
      {
          frontp_resetSwState(SW_SCALE_DIR);
          Serial.print("Se presiono SCALE\n");
          track_nextScale();
      }
      

      /*
      if(prueba>=1000)
      {
          prueba=0;
          if(toggle==0)
          {
            toggle=1;
          }
          else
          {
            toggle=0;
          }
          frontp_setLed(7, toggle);   


          int analogVal = frontp_readAnalogStepValue(0);
          Serial.print("Valor analog:");
          Serial.print(analogVal);
          Serial.print("\n");

          ios_setCVout(analogVal*4);
          
          ios_setVelocityOut(analogVal/8);
      }
      */
  }
}
