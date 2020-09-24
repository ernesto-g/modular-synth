

void fp_init(void);
void fp_loop(void);

uint16_t fp_getPotValue(uint8_t index);
int fp_getEncoderPosition(void);
void fp_setEncoderPosition(int pos);
int fp_getEncoderSw(void);
void fp_showLfoInLeds(uint8_t lfoIndex);
void fp_showWaveTypeInLeds(uint8_t wave);
uint8_t fp_getModeSwitch(void);
