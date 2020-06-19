
// Outputs
#define IOS_GATE_1              0
#define IOS_GATE_2              1
#define IOS_GATE_3              2
#define IOS_GATE_4              3

#define IOS_MUX_A               4
#define IOS_MUX_B               5
#define IOS_MUX_C               6

#define IOS_SR_LATCH            7
#define IOS_SR_DATA             IOS_MUX_B
#define IOS_SR_CLOCK            IOS_MUX_C

// Inputs
#define IO_SW_TRACK_RUN            0
#define IO_SW_SCALE_DIR            1
#define IO_SW_LEN_MODE             2
#define IO_SW_RPT_PROB             3
#define IO_SW_SHIFT                4




void ios_setLo(int outNumber);
void ios_setHi(int outNumber);
int ios_readSw(int swPin);
int ios_readMuxAnalog(void);

void ios_setCVout(int value);
void ios_setVelocityOut(unsigned char value);

void ios_init(void);
