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

// Outputs
#define IOS_GATE_1              0
#define IOS_GATE_2              1
#define IOS_GATE_3              2
#define IOS_GATE_4              3

#define IOS_MUX_A               4
#define IOS_MUX_B               5
#define IOS_MUX_C               6

#define IOS_SR_LATCH            7
#define IOS_SR_DATA             IOS_MUX_B
#define IOS_SR_CLOCK            IOS_MUX_C

// Inputs
#define IO_SW_TRACK_RUN            0
#define IO_SW_SCALE_DIR            1
#define IO_SW_LEN_MODE             2
#define IO_SW_RPT_PROB             3
#define IO_SW_SHIFT                4




void ios_setLo(int outNumber);
void ios_setHi(int outNumber);
int ios_readSw(int swPin);
unsigned int ios_readMuxAnalog(void);

int ios_readClkIn(void);
void ios_configureInterruptForExtClk( void (*pFnCallback)(void) );
int ios_readRstIn(void);
void ios_configureInterruptForExtRst( void (*pFnCallback)(void) );

void ios_setCVout(unsigned int value);
void ios_setVelocityOut(unsigned char value);

void ios_init(void);
