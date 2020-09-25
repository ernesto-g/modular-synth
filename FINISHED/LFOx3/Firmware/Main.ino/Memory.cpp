#include <EEPROM.h>

#define ADDRESS_WAVEFORM_LFO_0    0
#define ADDRESS_WAVEFORM_LFO_1    1
#define ADDRESS_WAVEFORM_LFO_2    2



uint8_t mem_readWaveform(uint8_t lfoIndex)
{
    switch(lfoIndex)
    {
        case 0: return EEPROM.read(ADDRESS_WAVEFORM_LFO_0);
        case 1: return EEPROM.read(ADDRESS_WAVEFORM_LFO_1);
        case 2: return EEPROM.read(ADDRESS_WAVEFORM_LFO_2);
    }
    return 0;
}

void mem_writeWaveform(uint8_t lfoIndex, uint8_t value)
{
    switch(lfoIndex)
    {
        case 0: EEPROM.update(ADDRESS_WAVEFORM_LFO_0, value); break;
        case 1: EEPROM.update(ADDRESS_WAVEFORM_LFO_1, value); break;
        case 2: EEPROM.update(ADDRESS_WAVEFORM_LFO_2, value); break;
    }  
}
