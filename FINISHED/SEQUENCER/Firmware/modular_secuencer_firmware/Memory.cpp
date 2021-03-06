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

#include <EEPROM.h>

#define MEM_ADDR_FREE                   0

#define MEM_ADDR_MODE                   1
#define MEM_ADDR_MIDI_CHN               2
#define MEM_ADDR_RST_BHV                3
#define MEM_ADDR_CLK_MODE               4
#define MEM_ADDR_CURRENT_TRACK          5
#define MEM_ADDR_CURRENT_SCALE          6

#define MEM_ADDR_TEMPO_H                7
#define MEM_ADDR_TEMPO_L                8


#define MEM_ADDR_CURRENT_DIR_0          10
#define MEM_ADDR_CURRENT_DIR_1          11
#define MEM_ADDR_CURRENT_DIR_2          12
#define MEM_ADDR_CURRENT_DIR_3          13

#define MEM_ADDR_CURRENT_LEN_0          14
#define MEM_ADDR_CURRENT_LEN_1          15
#define MEM_ADDR_CURRENT_LEN_2          16
#define MEM_ADDR_CURRENT_LEN_3          17

#define MEM_ADDR_CURRENT_CLK_DIV_0      18
#define MEM_ADDR_CURRENT_CLK_DIV_1      19
#define MEM_ADDR_CURRENT_CLK_DIV_2      20
#define MEM_ADDR_CURRENT_CLK_DIV_3      21

#define MEM_ADDR_EUCLID_STEPS_ON        22
#define MEM_ADDR_EUCLID_STEPS_OFFSET    23

#define MEM_ADDR_CURRENT_ROOT_NOTE      24


unsigned char mem_getRootNote(void)
{
    unsigned char val;
    val=EEPROM.read(MEM_ADDR_CURRENT_ROOT_NOTE);
    if(val<12)
      return val;
    return 0;
}

void mem_saveRootNote(unsigned char val)
{
  EEPROM.update(MEM_ADDR_CURRENT_ROOT_NOTE, val);
}

void mem_saveClkDiv(unsigned char clkDiv,unsigned char trackIndex)
{
    switch(trackIndex)
    {
        case 0: EEPROM.update(MEM_ADDR_CURRENT_CLK_DIV_0, clkDiv);break;
        case 1: EEPROM.update(MEM_ADDR_CURRENT_CLK_DIV_1, clkDiv);break;
        case 2: EEPROM.update(MEM_ADDR_CURRENT_CLK_DIV_2, clkDiv);break;
        case 3: EEPROM.update(MEM_ADDR_CURRENT_CLK_DIV_3, clkDiv);break;
    }
}
unsigned char mem_getClkDiv(unsigned char trackIndex)
{
    unsigned char val;
    switch(trackIndex)
    {
        case 0: val=EEPROM.read(MEM_ADDR_CURRENT_CLK_DIV_0);break;
        case 1: val=EEPROM.read(MEM_ADDR_CURRENT_CLK_DIV_1);break;
        case 2: val=EEPROM.read(MEM_ADDR_CURRENT_CLK_DIV_2);break;
        case 3: val=EEPROM.read(MEM_ADDR_CURRENT_CLK_DIV_3);break;
    }  
    if(val<=8)
      return val;
    return 1; // default=1
}





void mem_saveLen(unsigned char len,unsigned char trackIndex)
{
    switch(trackIndex)
    {
        case 0: EEPROM.update(MEM_ADDR_CURRENT_LEN_0, len);break;
        case 1: EEPROM.update(MEM_ADDR_CURRENT_LEN_1, len);break;
        case 2: EEPROM.update(MEM_ADDR_CURRENT_LEN_2, len);break;
        case 3: EEPROM.update(MEM_ADDR_CURRENT_LEN_3, len);break;
    }
}
unsigned char mem_getLen(unsigned char trackIndex)
{
    unsigned char val;
    switch(trackIndex)
    {
        case 0: val=EEPROM.read(MEM_ADDR_CURRENT_LEN_0);break;
        case 1: val=EEPROM.read(MEM_ADDR_CURRENT_LEN_1);break;
        case 2: val=EEPROM.read(MEM_ADDR_CURRENT_LEN_2);break;
        case 3: val=EEPROM.read(MEM_ADDR_CURRENT_LEN_3);break;
    }  
    if(val<=8)
      return val;
    return 8; // default is 8
}




void mem_saveDirection(unsigned char dir,unsigned char trackIndex)
{
    switch(trackIndex)
    {
        case 0: EEPROM.update(MEM_ADDR_CURRENT_DIR_0, dir);break;
        case 1: EEPROM.update(MEM_ADDR_CURRENT_DIR_1, dir);break;
        case 2: EEPROM.update(MEM_ADDR_CURRENT_DIR_2, dir);break;
        case 3: EEPROM.update(MEM_ADDR_CURRENT_DIR_3, dir);break;
    }
}
unsigned char mem_getDirection(unsigned char trackIndex)
{
    unsigned char val;
    switch(trackIndex)
    {
        case 0: val=EEPROM.read(MEM_ADDR_CURRENT_DIR_0);break;
        case 1: val=EEPROM.read(MEM_ADDR_CURRENT_DIR_1);break;
        case 2: val=EEPROM.read(MEM_ADDR_CURRENT_DIR_2);break;
        case 3: val=EEPROM.read(MEM_ADDR_CURRENT_DIR_3);break;
    }  
    if(val<8)
      return val;
    return 0;
}



void mem_saveTempo(unsigned int tempo)
{
    EEPROM.update(MEM_ADDR_TEMPO_H, (tempo<<8)&0xFF);
    EEPROM.update(MEM_ADDR_TEMPO_L, (tempo)&0xFF);
}

unsigned int mem_getTempo(void)
{
    unsigned int val=0;
    val=EEPROM.read(MEM_ADDR_TEMPO_H);
    val = val <<8;
    val = val & 0xFF00;
    val = val | EEPROM.read(MEM_ADDR_TEMPO_L);
    if(val<=600)
      return val;
    return 300; // default value
}



void mem_saveScale(unsigned char scale)
{
    EEPROM.update(MEM_ADDR_CURRENT_SCALE, scale);
}

unsigned char mem_getScale(void)
{
    unsigned char val;
    val=EEPROM.read(MEM_ADDR_CURRENT_SCALE);
    if(val<8)
      return val;
    return 0;
}





unsigned char mem_getCurrentTrack(void)
{
    unsigned char track = EEPROM.read(MEM_ADDR_CURRENT_TRACK);
    if(track<8)
      return track;
    return 0;  
}
void mem_saveCurrentTrack(unsigned char track)
{
     EEPROM.update(MEM_ADDR_CURRENT_TRACK, track);
}




unsigned char mem_getClockMode(void)
{
    unsigned char mode = EEPROM.read(MEM_ADDR_CLK_MODE);
    if(mode<8)
      return mode;
    return 0;  
}
void mem_saveClockMode(unsigned char mode)
{
     EEPROM.update(MEM_ADDR_CLK_MODE, mode);
}



unsigned char mem_getResetBehaviour(void)
{
    unsigned char mode = EEPROM.read(MEM_ADDR_RST_BHV);
    if(mode<8)
      return mode;
    return 0;  
}

void mem_saveResetBehaviour(unsigned char mode)
{
     EEPROM.update(MEM_ADDR_RST_BHV, mode);
}



unsigned char mem_getMidiChn(void)
{
    unsigned char ch = EEPROM.read(MEM_ADDR_MIDI_CHN);
    if(ch<128)
      return ch;
    return 0;    
}

void mem_saveMidiChn(unsigned char ch)
{
     EEPROM.update(MEM_ADDR_MIDI_CHN, ch); 
}


void mem_saveMode(unsigned char mode)
{
    EEPROM.update(MEM_ADDR_MODE, mode);
}

unsigned char mem_getMode(void)
{
    unsigned char mode = EEPROM.read(MEM_ADDR_MODE);
    if(mode<8)
      return mode;
    return 0;    
} 




unsigned char mem_getEuclideanStepsOn(void)
{
    unsigned char steps = EEPROM.read(MEM_ADDR_EUCLID_STEPS_ON);
    if(steps<=8)
      return steps;
    return 1;  
}
void mem_saveEuclideanStepsOn(unsigned char steps)
{
     EEPROM.update(MEM_ADDR_EUCLID_STEPS_ON, steps);
}

void mem_saveEuclideanStepsOffset(int offset)
{
    unsigned char offsetu=(unsigned char)offset;
    EEPROM.update(MEM_ADDR_EUCLID_STEPS_OFFSET, offsetu );
}

int mem_getEuclideanStepsOffset(void)
{
    unsigned char valu=0;
    int val;
    valu=EEPROM.read(MEM_ADDR_EUCLID_STEPS_OFFSET);
    if(valu>=128)
      val = ((int)(256 -valu))*(-1);
    else
      val = (int)valu;
    
    if(val<=7 && val >=-7)
      return val;
    return 0; // default value
}
