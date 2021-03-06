

#define LFO_WAVE_TYPE_SINE      0
#define LFO_WAVE_TYPE_TRIANGLE  1
#define LFO_WAVE_TYPE_EXP       2
#define LFO_WAVE_TYPE_RANDOM    3
#define LFO_WAVE_TYPE_SAW       4

#define LFO_FREE_MODE     0
#define LFO_SEQ_MODE      1
#define LFO_PHASE_MODE    2



void lfo_init(void);
void lfo_setWaveType(uint8_t index,uint8_t type);
void lfo_setFrequencyFrom_ADC(unsigned char index,unsigned int adcValue);
void lfo_setPhaseFromADC(unsigned char index,unsigned int adcValue);

void lfo_reset(unsigned char index);
void lfo_loop(void);
uint8_t lfo_getWaveType(uint8_t index);

void lfo_clkEventForFreeMode(void);
void lfo_clkEventForPhaseMode(uint16_t potValue1,uint16_t potValue2);
void lfo_clkEventForSequencerMode(void);

void lfo_setMode(uint8_t mode);
void lfo_setSteps(uint8_t lfoIndex, uint8_t steps);
void lfo_setSystickCallback(void (*callback)(void));
