void track_init(unsigned char initTrack,unsigned char initScale);
void track_playStep(int indexStep,int trackIndex);
void track_loop(void);
int track_nextScale(void);
void track_tick1ms(void);
int track_getCurrentTrack(void);
void track_silenceStep(int stepIndex, int trackIndex);
int track_nextTrack(void);
int track_getCurrentStepInTrack(void);
int track_getCurrentScale(void);

int track_nextProbRptMode(void);
int track_getCurrentProbRptMode(void);

unsigned char track_prevEuclideanStep(void);
unsigned char track_nextEuclideanStep(void);
unsigned char track_getCurrentEuclideanStepsOn(void);
unsigned char* track_getEuclideanStepsState(void);

int track_getCurrentEuclideanOffset(void);
int track_nextEuclideanOffset(void);
int track_prevEuclideanOffset(void);
