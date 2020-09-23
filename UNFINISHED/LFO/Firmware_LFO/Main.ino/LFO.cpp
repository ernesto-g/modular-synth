#include <Arduino.h>
#include "Waveforms.h"
#include "LFO.h"

#define NUM_VOICES  3
#define ACCUMULATOR_STEPS 512

struct DDS {
    uint16_t increment;
    uint16_t position;
    uint16_t accumulator;
};
volatile struct DDS voices[NUM_VOICES];

volatile uint8_t flagPendigPhaseUpdate[NUM_VOICES];
volatile uint16_t valuePendigPhaseUpdate[NUM_VOICES];

volatile uint8_t flagLfoRenderFinished=0;

#define SET_LFO_0(X)  OCR2B=X
#define SET_LFO_1(X)  OCR1A=X
#define SET_LFO_2(X)  OCR2A=X

// Private variables
static volatile unsigned char waveType[NUM_VOICES]={LFO_WAVE_TYPE_SINE,LFO_WAVE_TYPE_SINE,LFO_WAVE_TYPE_SINE};


void lfo_init(void)
{
    // timer 2 PWM
  pinMode(3, OUTPUT);
  pinMode(11, OUTPUT);
  TCCR2A = _BV(COM2A1) | _BV(COM2B1) | _BV(WGM21) | _BV(WGM20);
  TCCR2B = _BV(CS00); // 62.5Khz
  OCR2A = 128; //PIN11 0 to 255
  OCR2B = 128; // PIN3  0 to 255

  // timer 1 PWM
  pinMode(9, OUTPUT);
  DDRB  |= (1 << DDB1) | (1 << DDB2);
  TCCR1A = (1 << COM1A1) | (1 << WGM11);
  TCCR1B = (1 << WGM12) | (1 << WGM13) | (1 << CS10); // 62.5Khz
  ICR1 = 256; // top count
  OCR1A = 128; // PIN9 0 to 255
  OCR1B = 0;

  // Configure TIMER0 for DDS interrupt
  cli();//stop interrupts
  TCCR0A = 0;// set entire TCCR0A register to 0
  TCCR0B = 0;// same for TCCR0B
  TCNT0  = 0;//initialize counter value to 0
  OCR0A = 12; // 52uS 
  OCR0A = 24; // 104uS 

  // turn on CTC mode
  TCCR0A |= (1 << WGM01);
  // Set CS01 and CS00 bits for 64 prescaler
  TCCR0B |= (1 << CS01) | (1 << CS00);   
  // enable timer compare interrupt
  TIMSK0 |= (1 << OCIE0A);
  sei();//allow interrupts
  //___________________________________




  unsigned char i;
  for(i=0; i<NUM_VOICES; i++)
  {              
    voices[i].accumulator=0;
    voices[i].position=0;
    lfo_setWaveType(i,LFO_WAVE_TYPE_SINE);
    lfo_setFrequencyFrom_ADC(i,768); 
  }
 
}

ISR(TIMER0_COMPA_vect) // Period: 104us. process: 25uS
{
  //digitalWrite(1,HIGH);  
  unsigned short PWMValue;
  unsigned char i;

  for(i=0; i<NUM_VOICES; i++)
  {              
      switch(waveType[i])
      {
        case LFO_WAVE_TYPE_SINE:
          PWMValue = pgm_read_byte_near(SINETABLE + voices[i].position );
          break;
        case LFO_WAVE_TYPE_TRIANGLE:
          PWMValue = pgm_read_byte_near(TRIANGLETABLE + voices[i].position );
          break;
        case LFO_WAVE_TYPE_EXP:
          PWMValue = pgm_read_byte_near(EXPTABLE + voices[i].position );
          break;
      }
      switch(i)
      {
        case 0: SET_LFO_0(PWMValue);break;  
        case 1: SET_LFO_1(PWMValue);break;  
        case 2: SET_LFO_2(PWMValue);break;  
      }

      // Calculate step
      voices[i].accumulator += voices[i].increment;
      voices[i].position += voices[i].accumulator / ACCUMULATOR_STEPS;
      voices[i].accumulator = voices[i].accumulator % ACCUMULATOR_STEPS;
      voices[i].position = voices[i].position % TABLE_SIZE;
      //________________

      /*      
      if(i==0)
      {
        // update phase for 1 and 2
        if(flagPendigPhaseUpdate[1]==1)
        {
          flagPendigPhaseUpdate[1]=0;
          voices[1].position = voices[0].position + valuePendigPhaseUpdate[1];
          voices[1].accumulator = voices[0].accumulator;
          voices[1].position = voices[1].position % TABLE_SIZE;
        }
        if(flagPendigPhaseUpdate[2]==1)
        {
          flagPendigPhaseUpdate[2]=0;
          voices[2].position = voices[0].position + valuePendigPhaseUpdate[2];
          voices[2].accumulator = voices[0].accumulator;
          voices[2].position = voices[2].position % TABLE_SIZE;
        }
      }*/
  }
  
  //digitalWrite(1,LOW);  
  flagLfoRenderFinished=1;
}

void lfo_setWaveType(unsigned char index,unsigned char type)
{
    waveType[index] = type;
}


void lfo_setFrequencyFrom_ADC(unsigned char index,unsigned int adcValue)
{   
    // 1:104.8seg, 7850:75Hz, 15700:150Hz 
    voices[index].increment = pgm_read_word_near(TABLE_POT_TO_FM + adcValue);    
}

void lfo_setPhaseFromADC(unsigned char index,unsigned int adcValue)
{
  if(index==1 || index==2)
  {
        // 1024:180°
        // 512: 90°
        // 256: 45°
        cli();//stop interrupts
        voices[index].position = voices[0].position + adcValue;
        voices[index].position = voices[index].position % TABLE_SIZE;
        sei();//allow interrupts
        /*        
        cli();//stop interrupts
        flagPendigPhaseUpdate[index] = 1;
        valuePendigPhaseUpdate[index] = adcValue;
        sei();//allow interrupts
        */
  }
}

void lfo_reset(unsigned char index)
{
    voices[index].accumulator=0;
    voices[index].position=0;
}


void lfo_loop(void)
{
  if(flagLfoRenderFinished)
  {
    flagLfoRenderFinished=0;
    /*
    // update phase for 1 and 2
    if(flagPendigPhaseUpdate[1]==1)
    {
      flagPendigPhaseUpdate[1]=0;
      voices[1].position = voices[0].position + valuePendigPhaseUpdate[1];
      //voices[1].accumulator = voices[0].accumulator;
    }
    if(flagPendigPhaseUpdate[2]==1)
    {
      flagPendigPhaseUpdate[2]=0;
      voices[2].position = voices[0].position + valuePendigPhaseUpdate[2];
    }*/
  }
}
