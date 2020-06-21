#include <Arduino.h>

#include "ios.h"
#include "FrontPanel.h"
#include "RythmManager.h"
#include "TrackManager.h"
#include "Logic.h"

#define BPM_MIN   30
#define BPM_MAX   600


static unsigned char flagShift;

void logic_init(void)
{
    flagShift=0;

    frontp_setEncoderPosition(rthm_getCurrentTempo());
}


void logic_loop(void)
{
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
          if(flagShift==0)
          {
            if(rthm_getState()==0)
            {
                rthm_play();
            }
            else
            {
                rthm_stop();
            }
            Serial.print("RUN\n");
          }
          else
          {
            Serial.print("TRACK\n");            
          }
      }

      if(frontp_getSwState(SW_SCALE_DIR)==FRONT_PANEL_SW_STATE_SHORT)
      {
          frontp_resetSwState(SW_SCALE_DIR);
          if(flagShift==0)
          {
            Serial.print("DIR\n");
            rthm_nextDirection();
          }
          else
          {
            Serial.print("SCALE\n");
            track_nextScale();            
          }
      }

      if(frontp_getSwState(SW_RPT_PROB)==FRONT_PANEL_SW_STATE_SHORT)
      {
          frontp_resetSwState(SW_RPT_PROB);
          if(flagShift==0)
          {
            Serial.print("CLK\n");
          }
          else
          {
            Serial.print("PROB/RPT\n");
          }
      }

      if(frontp_getSwState(SW_LEN_MODE)==FRONT_PANEL_SW_STATE_SHORT)
      {
          frontp_resetSwState(SW_LEN_MODE);
          if(flagShift==0)
          {
            Serial.print("LEN\n");
            rthm_nextLen();
          }
          else
          {
            Serial.print("MODE\n");
          }
      }

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

}
