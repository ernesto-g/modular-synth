#define LOGIC_MODE_0_4TRACKS                  0
#define LOGIC_MODE_1_MIDI_PLUS_3TRACKS        1
#define LOGIC_MODE_2_ARP_PLUS_3TRACKS         2
#define LOGIC_MODE_3_EUCLIDEAN_PLUS_3TRACKS   3




int logic_getCurrentMode(void);

void logic_init(void);
void logic_loop(void);
void logic_tick1ms(void);
