

void rthm_tick(void);
void rthm_init(unsigned int initTempo);
void rthm_loop(void);
unsigned int rthm_getCurrentTempoPeriodMs(void);
int rthm_getState(void);

void rthm_setTempo(int tempo);
int rthm_getCurrentTempo(void);

void rthm_stop(void);
void rthm_play(void);

int rthm_nextLen(void);
int rthm_getCurrentLen(void);

int rthm_getCurrentDirection(void);
int rthm_nextDirection(void);

void rthm_setClkSrc(unsigned char clkSrc);
unsigned char rthm_getClkSrc(void);
unsigned char rthm_nextClkSrc(void);

int rthm_getCurrentClockDivisor(void);
int rthm_nextClockDivisor(void);

void rthm_setRstMode(unsigned char mode);
unsigned char rthm_getRstMode(void);
unsigned char rthm_nextRstMode(void);
