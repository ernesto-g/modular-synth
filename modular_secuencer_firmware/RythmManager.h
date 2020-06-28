

void rthm_tick(void);
void rthm_init(void);
void rthm_setTempo(int tempo);
int rthm_getCurrentTempo(void);
void rthm_stop(void);
void rthm_play(void);
int rthm_getState(void);
void rthm_loop(void);
int rthm_nextDirection(void);
int rthm_nextLen(void);
int rthm_getCurrentLen(void);
unsigned int rthm_getCurrentTempoPeriodMs(void);
int rthm_getCurrentDirection(void);
void rthm_setClkSrc(unsigned char clkSrc);
int rthm_getCurrentClockDivisor(void);
int rthm_nextClockDivisor(void);
void rthm_setRstMode(unsigned char mode);
