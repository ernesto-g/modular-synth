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


void rthm_tick(void);
void rthm_init(unsigned int initTempo);
void rthm_loop(void);
unsigned int rthm_getCurrentTempoPeriodMs(void);
int rthm_getState(void);

void rthm_setTempo(int tempo);
int rthm_getCurrentTempo(void);

void rthm_stop(void);
void rthm_play(void);

int rthm_nextLen(void);
int rthm_getCurrentLen(void);

int rthm_getCurrentDirection(void);
int rthm_nextDirection(void);

void rthm_setClkSrc(unsigned char clkSrc);
unsigned char rthm_getClkSrc(void);
unsigned char rthm_nextClkSrc(void);

int rthm_getCurrentClockDivisor(void);
int rthm_nextClockDivisor(void);

void rthm_setRstMode(unsigned char mode);
unsigned char rthm_getRstMode(void);
unsigned char rthm_nextRstMode(void);
