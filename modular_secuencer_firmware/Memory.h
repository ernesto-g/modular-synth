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

unsigned char mem_getMode(void);
void mem_saveMode(unsigned char mode);
unsigned char mem_getMidiChn(void);
void mem_saveMidiChn(unsigned char ch);
unsigned char mem_getResetBehaviour(void);
void mem_saveResetBehaviour(unsigned char mode);
unsigned char mem_getClockMode(void);
void mem_saveClockMode(unsigned char mode);
unsigned char mem_getCurrentTrack(void);
void mem_saveCurrentTrack(unsigned char track);
unsigned char mem_getScale(void);
void mem_saveScale(unsigned char scale);
void mem_saveTempo(unsigned int tempo);
unsigned int mem_getTempo(void);


void mem_saveDirection(unsigned char dir,unsigned char trackIndex);
unsigned char mem_getDirection(unsigned char trackIndex);
void mem_saveLen(unsigned char len,unsigned char trackIndex);
unsigned char mem_getLen(unsigned char trackIndex);
void mem_saveClkDiv(unsigned char clkDiv,unsigned char trackIndex);
unsigned char mem_getClkDiv(unsigned char trackIndex);

int mem_getEuclideanStepsOffset(void);
void mem_saveEuclideanStepsOffset(int offset);
void mem_saveEuclideanStepsOn(unsigned char steps);
unsigned char mem_getEuclideanStepsOn(void);
