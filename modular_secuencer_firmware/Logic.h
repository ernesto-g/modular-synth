#define LOGIC_MODE_0_4TRACKS                  0
#define LOGIC_MODE_1_MIDI_PLUS_3TRACKS        1
#define LOGIC_MODE_2_ARP_PLUS_3TRACKS         2
#define LOGIC_MODE_3_EUCLIDEAN_PLUS_3TRACKS   3


// Config values
#define CONFIG_CLK_SRC_INT    0
#define CONFIG_CLK_SRC_EXT    1

#define CONFIG_MIDI_CHN_0     0
#define CONFIG_MIDI_CHN_1     1
#define CONFIG_MIDI_CHN_2     2
#define CONFIG_MIDI_CHN_3     3
#define CONFIG_MIDI_CHN_4     4
#define CONFIG_MIDI_CHN_5     5
#define CONFIG_MIDI_CHN_6     6
#define CONFIG_MIDI_CHN_7     7

#define CONFIG_RST_BHV_BACK2ONE         0
#define CONFIG_RST_BHV_HOLD_WHILE_HI    1
#define CONFIG_RST_BHV_INVER_DIR        2


struct S_Config
{
    unsigned char clkSrc;
    unsigned char midiChn;
    unsigned char resetBehaviour;
};

typedef struct S_Config Config;
//___________________________________________
 


// Public functions
int logic_getCurrentMode(void);
void logic_init(void);
void logic_loop(void);
void logic_tick1ms(void);
Config* logic_getConfig(void);
//_________________
