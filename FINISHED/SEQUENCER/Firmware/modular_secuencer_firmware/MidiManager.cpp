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

#include <Arduino.h>
#include "MidiManager.h"
#include "ios.h"
#include "Logic.h"

#define KEYS_PRESSED_LEN    10
#define USART_BAUDRATE 31250
#define BAUD_PRESCALE (((F_CPU / (USART_BAUDRATE * 16UL))) - 1)


// Private variables
MidiInfo midiInfo;
unsigned char midiStateMachine=MIDI_STATE_IDLE;
static unsigned char midiChannel;
static volatile unsigned char timeout;
volatile unsigned char bufferRx[5][3];
volatile unsigned char bufferRxIndex[5]={0,0,0,0,0};
volatile unsigned char bufferRxNewPacket[5]={0,0,0,0,0};
volatile unsigned char currentBufferIndex=0;
static KeyPressedInfo keysPressed[KEYS_PRESSED_LEN];
static unsigned char repeatKeyIndex;


// Private functions
static void midi_analizeMidiInfo(MidiInfo * pMidiInfo);
static unsigned char deleteKey(unsigned char note);
static unsigned char getIndexOfPressedKey(unsigned char note);
static unsigned char saveKey(unsigned char note);


// Tables
static const PROGMEM unsigned int CHROM_TABLE[60] = {68,137,205,273,341,410,478,546,614,682,751,819,887,956,1024,1092,1160,1229,1297,1365,1433,1501,1570,1638,1706,1775,1843,1911,1979,2048,2116,2184,2252,2320,2389,2457,2525,2594,2662,2730,2798,2867,2935,3003,3071,3139,3208,3276,3344,3413,3481,3549,3617,3686,3754,3822,3890,3958,4027,4095};



ISR(USART_RX_vect)
{
    if(bufferRxNewPacket[currentBufferIndex]==0)
    {
      bufferRx[currentBufferIndex][bufferRxIndex[currentBufferIndex]] = UDR0;
      bufferRxIndex[currentBufferIndex]++;
      if(bufferRxIndex[currentBufferIndex]>=3)
      {
        bufferRxNewPacket[currentBufferIndex] = 1;
        currentBufferIndex++;
        if(currentBufferIndex>=5)
          currentBufferIndex=0;
      }
    }
    else
    {
      uint8_t r = UDR0;
      r = r; // silence compiler warnings  
    }
}


void midi_init(void)
{
    int i;
    for(i=0; i<KEYS_PRESSED_LEN; i++)
      keysPressed[i].flagFree=1;
    
    midiChannel = 0; //mem_loadMidiChn();
    timeout=0;
    repeatKeyIndex=0;

    
    // Configure serial port for MIDI input
    UBRR0L = (uint8_t)(BAUD_PRESCALE & 0xff);
    UBRR0H = (uint8_t)(BAUD_PRESCALE >> 8);
    UCSR0B =
        // interrupt on receive
        //(1 << RXCIE0) | (1 << TXEN0) |
        (1 << RXCIE0) |
        (1 << RXEN0);
    UCSR0C =
        // no parity bit
        (0 << UPM01) |
        (0 << UPM00) |
        // asyncrounous USART 
        (0 << UMSEL01) |
        (0 << UMSEL00) |
        // one stop bit 
        (0 << USBS0) |
        // 8-bits of data 
        (1 << UCSZ01) |
        (1 << UCSZ00);
    
    //_____________________________________
}

void midi_tickMs(void)
{
    if(timeout>0)
      timeout--;
}

void midi_setMidiChn(unsigned char val)
{
    midiChannel = val;
}

unsigned char midi_getMidiChn(void)
{
    return midiChannel;
}

unsigned char midi_nextMidiChn(void)
{
    midiChannel++;
    if(midiChannel>=9)
      midiChannel=0;
      
    return midiChannel;
}


void midi_stateMachine(unsigned char midiByte)
{
  timeout=MIDI_BYTE_TIMEOUT;
  
  switch(midiStateMachine)
  {
      case MIDI_STATE_IDLE:
        {
        // read status byte 
        midiInfo.channel = midiByte & B00001111;
        midiInfo.cmd = midiByte & B11110000;
        midiStateMachine = MIDI_STATE_RVC_DATA1;
        break;
      }
      case MIDI_STATE_RVC_DATA1:
      {
        // read note
        midiInfo.note = midiByte;
        midiStateMachine = MIDI_STATE_RVC_DATA2;        
        break;
      }
      case MIDI_STATE_RVC_DATA2:
      {
        // read velocity
        midiInfo.vel = midiByte;
        midiStateMachine = MIDI_STATE_IDLE;
        midi_analizeMidiInfo(&midiInfo);
        break;
      }
  }

    
}

void midi_loop(void)
{

  if(timeout==0)
      midiStateMachine = MIDI_STATE_IDLE;  

    // MIDI Reception
    unsigned char index;
    for(index=0; index<5; index++)
    {
      if(bufferRxNewPacket[index])
      {       
          if(logic_getCurrentMode()==LOGIC_MODE_1_MIDI_PLUS_3TRACKS ||
             logic_getCurrentMode()==LOGIC_MODE_2_ARP_PLUS_3TRACKS)
          {
            midi_stateMachine(bufferRx[index][0]);
            midi_stateMachine(bufferRx[index][1]);
            midi_stateMachine(bufferRx[index][2]);  
          }    
          bufferRxIndex[index]=0;
          bufferRxNewPacket[index]=0;
      }
    }
    //_______________
    
}


static void midi_analizeMidiInfo(MidiInfo * pMidiInfo)
{
    if(pMidiInfo->channel==midiChannel)
    {
        if(pMidiInfo->cmd==MIDI_CMD_NOTE_ON)
        {
            // NOTE ON 
            if(pMidiInfo->note>=36)
            {
              unsigned char noteTableIndex = pMidiInfo->note-36;
              if( noteTableIndex < 60)
              {
                  if(logic_getCurrentMode()==LOGIC_MODE_1_MIDI_PLUS_3TRACKS)
                  {
                    // SET Velocity output
                    ios_setVelocityOut(pMidiInfo->vel);
                    // SET CV voltage for note
                    ios_setCVout(pgm_read_word(&(CHROM_TABLE[noteTableIndex])));
                    // Set GATE 1
                    ios_setHi(IOS_GATE_1);
                  }
              }
              saveKey(pMidiInfo->note);
            }
        }
        else if(pMidiInfo->cmd==MIDI_CMD_NOTE_OFF)
        {
            
            // NOTE OFF
            deleteKey(pMidiInfo->note);
            if(midi_thereAreNoKeysPressed()==1)
            {
                if(logic_getCurrentMode()==LOGIC_MODE_1_MIDI_PLUS_3TRACKS)
                {
                  // GATE OFF
                  ios_setLo(IOS_GATE_1);
                }
            }
        }
        else if(pMidiInfo->cmd==MIDI_CMD_CONTROL_CHANGE)
        {
          // control change
        }
        else
        {
          // unsuported command          
        }
    }     
}


unsigned int midi_calculateValueForCV(unsigned char key)
{
    unsigned char noteTableIndex = key-36;
    return pgm_read_word(&(CHROM_TABLE[noteTableIndex]));
}

static unsigned char saveKey(unsigned char note)
{
  unsigned char i;
  for(i=0; i<KEYS_PRESSED_LEN; i++)
  {
    if(keysPressed[i].flagFree==1)
    {
        keysPressed[i].flagFree=0;
        keysPressed[i].note = note;
        return 0;
    }
  }
  return 1; // no more space
}
static unsigned char getIndexOfPressedKey(unsigned char note)
{
  unsigned char i;
  for(i=0; i<KEYS_PRESSED_LEN; i++)
  {
    if(keysPressed[i].flagFree==0)
    {
        if(keysPressed[i].note == note)
          return i;
    }
  }
  return 0xFF; 
}
static unsigned char deleteKey(unsigned char note)
{
    unsigned char index = getIndexOfPressedKey(note);
    if(index<KEYS_PRESSED_LEN)
    {
      keysPressed[index].flagFree=1;
      return 1;
    }
    return 0;
}

unsigned char midi_thereAreNoKeysPressed(void)
{
  unsigned char i;
  for(i=0; i<KEYS_PRESSED_LEN; i++)
  {
    if(keysPressed[i].flagFree==0)
    {
      return 0;
    }
  }
  return 1; 
}


unsigned char midi_getNextKeyForRepeat(void)
{
    unsigned char i;
    unsigned char found=0;
    unsigned char ret=0xFF;
    for(i=repeatKeyIndex; i<KEYS_PRESSED_LEN; i++)
    {
        if(keysPressed[i].flagFree==0)
        {
          ret = keysPressed[i].note;
          found = 1;
          break;
        }
    }
    if(found==0)
    {
        repeatKeyIndex = 0;
        for(i=repeatKeyIndex; i<KEYS_PRESSED_LEN; i++)
        {
            if(keysPressed[i].flagFree==0)
            {
              ret = keysPressed[i].note;
              break;
            }
        }        
    }
    
    repeatKeyIndex = i+1;
    if(repeatKeyIndex>=KEYS_PRESSED_LEN)
      repeatKeyIndex = 0;

   return ret;
}
