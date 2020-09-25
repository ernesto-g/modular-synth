#include <Arduino.h>
#include "Waveforms.h"
#include "LFO.h"
#include "Ios.h"

#define NUM_VOICES  3
#define ACCUMULATOR_STEPS 512

#define SET_LFO_0(X)  OCR2B=X
#define SET_LFO_1(X)  OCR1A=X
#define SET_LFO_2(X)  OCR2A=X


struct DDS {
    uint16_t increment;
    uint16_t position;
    uint16_t accumulator;
};

struct SEQ {
    uint16_t steps;
    uint16_t currentStep;
};

// Private variables
static volatile struct DDS voices[NUM_VOICES];
static volatile uint8_t waveType[NUM_VOICES]={LFO_WAVE_TYPE_SINE,LFO_WAVE_TYPE_SINE,LFO_WAVE_TYPE_SINE};
static volatile uint8_t flagSamplesON; 
static struct SEQ sequencerInfo[NUM_VOICES];
static volatile uint8_t timeoutBoc=0;
static volatile uint8_t intDivider=0;
static volatile void (*systickCallback)(void)=NULL;
static volatile uint16_t lfo0lastPosition=0;


// Private functions
static void setOutputByStep(uint8_t i);


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
    sequencerInfo[i].currentStep=0;
    sequencerInfo[i].steps=1;
    lfo_setWaveType(i,LFO_WAVE_TYPE_SAW);
    lfo_setFrequencyFrom_ADC(i,768); 
  }
  flagSamplesON=1;
 
}

ISR(TIMER0_COMPA_vect) // Period: 104us. process: 25uS
{
  unsigned short PWMValue;
  unsigned char i;

  if(flagSamplesON)
  {
    if(voices[0].position<lfo0lastPosition)
    {
      // Begining Of cicle
      ios_setBocOut(1);
      timeoutBoc=2;
    }
    lfo0lastPosition = voices[0].position;


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
          case LFO_WAVE_TYPE_SAW:
            PWMValue = pgm_read_byte_near(SAWTABLE + voices[i].position );
            break;
          case LFO_WAVE_TYPE_RANDOM:
            PWMValue = pgm_read_byte_near(RANDTABLE + voices[i].position );
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
    }
  } 

  intDivider++;
  if(intDivider>=10)
  {
    //1ms
    intDivider=0;

    if(timeoutBoc>0)
      timeoutBoc--;

    if(systickCallback!=NULL)
      systickCallback();  
  }

}

void lfo_setSystickCallback(void (*callback)(void))
{
  systickCallback = callback;
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
  }
}

void lfo_reset(unsigned char index)
{
    voices[index].accumulator=0;
    voices[index].position=0;
}

uint8_t lfo_getWaveType(uint8_t index)
{
    return waveType[index];
}

void lfo_loop(void)
{
    if(timeoutBoc==0)
    {
      ios_setBocOut(0);
    }
}

void lfo_setMode(uint8_t mode)
{
    if(mode==LFO_SEQ_MODE)
    {
      flagSamplesON=0;
    }
    else
    {
      flagSamplesON=1;
    }
}

void lfo_clkEvent(void)
{
    uint8_t i;
    
    if(flagSamplesON)
    {
      // Sync lfo
    }
    else
    {
      // Sequencer mode
      for(i=0; i<NUM_VOICES; i++)
      {
          // Increment step
          sequencerInfo[i].currentStep++;
          if(sequencerInfo[i].currentStep>=sequencerInfo[i].steps)
          {
              sequencerInfo[i].currentStep=0;  
          }
          
          setOutputByStep(i);
          //_______________
      }
      
      // Set BOC output
      if(sequencerInfo[0].currentStep==0)
      {
        ios_setBocOut(1);
        timeoutBoc=2;
      }
      //_______________
    }
}

void lfo_setSteps(uint8_t lfoIndex, uint8_t steps)
{
    sequencerInfo[lfoIndex].steps = steps;
}

static void setOutputByStep(uint8_t i)
{
      uint16_t PWMValue;
      uint16_t tableOffset;

      tableOffset = (sequencerInfo[i].currentStep*(TABLE_SIZE-1))/(sequencerInfo[i].steps-1);
      
      
      switch(waveType[i])
      {
        case LFO_WAVE_TYPE_SINE:
          PWMValue = pgm_read_byte_near(SINETABLE + tableOffset);
          break;
        case LFO_WAVE_TYPE_TRIANGLE:
          PWMValue = pgm_read_byte_near(TRIANGLETABLE + tableOffset);
          break;
        case LFO_WAVE_TYPE_EXP:
          PWMValue = pgm_read_byte_near(EXPTABLE + tableOffset);
          break;
        case LFO_WAVE_TYPE_SAW:
          PWMValue = pgm_read_byte_near(SAWTABLE + tableOffset);
          break;
        case LFO_WAVE_TYPE_RANDOM:
          PWMValue = pgm_read_byte_near(RANDTABLE + tableOffset);
          break;
      }          
      // Output sample
      switch(i)
      {
        case 0: SET_LFO_0(PWMValue);break;
        case 1: SET_LFO_1(PWMValue);break;
        case 2: SET_LFO_2(PWMValue);break;
      }
}
