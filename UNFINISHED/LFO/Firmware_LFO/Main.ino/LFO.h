

#define LFO_WAVE_TYPE_SINE      0
#define LFO_WAVE_TYPE_TRIANGLE  1
#define LFO_WAVE_TYPE_EXP       2
#define LFO_WAVE_TYPE_RANDOM    3
#define LFO_WAVE_TYPE_SAW       4



void lfo_init(void);
void lfo_setWaveType(unsigned char index,unsigned char type);
void lfo_setFrequencyFrom_ADC(unsigned char index,unsigned int adcValue);
void lfo_setPhaseFromADC(unsigned char index,unsigned int adcValue);

void lfo_reset(unsigned char index);
void lfo_loop(void);
