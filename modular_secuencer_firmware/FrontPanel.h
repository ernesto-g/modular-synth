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

#define FRONT_PANEL_SW_STATE_IDLE           0
#define FRONT_PANEL_SW_STATE_JUST_PRESSED   1
#define FRONT_PANEL_SW_STATE_SHORT          2
#define FRONT_PANEL_SW_STATE_LONG           3
#define FRONT_PANEL_SW_STATE_JUST_RELEASED  4


#define SW_TRACK_RUN            0
#define SW_SCALE_DIR            1
#define SW_LEN_MODE             2
#define SW_RPT_PROB             3
#define SW_SHIFT                4


void frontp_init(void);
void frontp_loop(void);
void frontp_tick1Ms(void);

int frontp_getSwState(int swIndex);
void frontp_resetSwState(int swIndex);

int frontp_getEncoderPosition(void);
void frontp_setEncoderPosition(int pos);


void frontp_setLed(int led, int value);
void frontp_showStepInLed(int stepIndex, int flagTurnOffOthers);
void frontp_showStepInLedBlinking(int stepIndex, int flagTurnOffOthers);


unsigned int frontp_readAnalogStepValue(int index);

void frontp_showEuclideanStateInLeds(unsigned char stepInTrack,unsigned char* euclideanSteps);
