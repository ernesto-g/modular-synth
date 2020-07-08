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

#define MIDI_CMD_NOTE_ON  144
#define MIDI_CMD_NOTE_OFF  128
#define MIDI_CMD_CONTROL_CHANGE 0xB0

#define MIDI_STATE_IDLE       0
#define MIDI_STATE_RVC_DATA1  1
#define MIDI_STATE_RVC_DATA2  2

#define MIDI_BYTE_TIMEOUT   200 // 200ms

typedef struct S_MidiInfo {
  unsigned char channel;
  unsigned char cmd;
  unsigned char note;
  unsigned char vel;
}MidiInfo;

typedef struct S_KeyPressedInfo {
  unsigned char note;
  unsigned char flagFree; 
}KeyPressedInfo;


void midi_stateMachine(unsigned char midiByte);
void midi_loop(void);
void midi_init(void);
void midi_tickMs(void);
void midi_setMidiChn(unsigned char val);
unsigned char midi_getMidiChn(void);
unsigned char midi_nextMidiChn(void);
unsigned char midi_getNextKeyForRepeat(void);
unsigned int midi_calculateValueForCV(unsigned char key);
unsigned char midi_thereAreNoKeysPressed(void);
