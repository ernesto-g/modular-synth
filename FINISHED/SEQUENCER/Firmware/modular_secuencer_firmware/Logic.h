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

#define LOGIC_MODE_0_4TRACKS                  0
#define LOGIC_MODE_1_MIDI_PLUS_3TRACKS        1
#define LOGIC_MODE_2_ARP_PLUS_3TRACKS         2
#define LOGIC_MODE_3_EUCLIDEAN_PLUS_3TRACKS   3


// Config values
#define CONFIG_CLK_SRC_INT    0
#define CONFIG_CLK_SRC_EXT    1

#define CONFIG_MIDI_CHN_0     0
#define CONFIG_MIDI_CHN_1     1
#define CONFIG_MIDI_CHN_2     2
#define CONFIG_MIDI_CHN_3     3
#define CONFIG_MIDI_CHN_4     4
#define CONFIG_MIDI_CHN_5     5
#define CONFIG_MIDI_CHN_6     6
#define CONFIG_MIDI_CHN_7     7

#define CONFIG_RST_BHV_BACK2ONE         0
#define CONFIG_RST_BHV_HOLD_WHILE_HI    1
#define CONFIG_RST_BHV_INVER_DIR        2


//___________________________________________
 


// Public functions
int logic_getCurrentMode(void);
void logic_init(void);
void logic_loop(void);
void logic_tick1ms(void);
//_________________
