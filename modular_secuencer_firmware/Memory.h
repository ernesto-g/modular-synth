unsigned char mem_getMode(void);
void mem_saveMode(unsigned char mode);
unsigned char mem_getMidiChn(void);
void mem_saveMidiChn(unsigned char ch);
unsigned char mem_getResetBehaviour(void);
void mem_saveResetBehaviour(unsigned char mode);
unsigned char mem_getClockMode(void);
void mem_saveClockMode(unsigned char mode);
unsigned char mem_getCurrentTrack(void);
void mem_saveCurrentTrack(unsigned char track);
unsigned char mem_getScale(void);
void mem_saveScale(unsigned char scale);
void mem_saveTempo(unsigned int tempo);
unsigned int mem_getTempo(void);


void mem_saveDirection(unsigned char dir,unsigned char trackIndex);
unsigned char mem_getDirection(unsigned char trackIndex);
void mem_saveLen(unsigned char len,unsigned char trackIndex);
unsigned char mem_getLen(unsigned char trackIndex);
void mem_saveClkDiv(unsigned char clkDiv,unsigned char trackIndex);
unsigned char mem_getClkDiv(unsigned char trackIndex);
