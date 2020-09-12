/*
 * Ui.cpp
 *
 *  Created on: Sep 8, 2020
 *      Author: ernesto
 */


#include <stdint.h>
#include "Ui.h"

#include "braids/settings.h"
#include "braids/drivers/adc.h"

#include "hal/Display.h"
#include "hal/Encoder.h"
#include "hal/Memory.h"


using namespace braids;

#define UI_STATE_SELECT_OSCILLATOR			0
#define UI_STATE_CONFIG_MENU				1
#define UI_STATE_SET_VALUE					2
#define UI_STATE_CALIB						3
#define UI_STATE_CALIB_STEP_2				4


struct S_OscData{
	uint8_t osc;
	uint8_t bank;
	int8_t symbol;
};
typedef struct S_OscData OscData;
#define OSCILLATOR_INDEX_LEN	39
static const OscData LIMITED_OSCILLATORS[OSCILLATOR_INDEX_LEN]={
		  {MACRO_OSC_SHAPE_CSAW,0,'0'},
		  {MACRO_OSC_SHAPE_MORPH,0,'1'},
		  {MACRO_OSC_SHAPE_SAW_SQUARE,0,'2'},
		  {MACRO_OSC_SHAPE_SINE_TRIANGLE,0,'3'},
		  {MACRO_OSC_SHAPE_BUZZ,0,'4'},

		  {MACRO_OSC_SHAPE_SQUARE_SUB,0,'5'},
		  {MACRO_OSC_SHAPE_SAW_SUB,0,'6'},
		  {MACRO_OSC_SHAPE_SQUARE_SYNC,0,'7'},
		  {MACRO_OSC_SHAPE_SAW_SYNC,0,'8'},
		  {MACRO_OSC_SHAPE_TRIPLE_SAW,0,'9'},
		  {MACRO_OSC_SHAPE_TRIPLE_SQUARE,0,'A'},
		  {MACRO_OSC_SHAPE_TRIPLE_TRIANGLE,0,'B'},
		  {MACRO_OSC_SHAPE_TRIPLE_SINE,0,'C'},
		  {MACRO_OSC_SHAPE_TRIPLE_RING_MOD,0,'D'},
		  {MACRO_OSC_SHAPE_SAW_SWARM,0,'E'},
		  {MACRO_OSC_SHAPE_SAW_COMB,0,'F'},
		  {MACRO_OSC_SHAPE_TOY,0,'G'},

		  {MACRO_OSC_SHAPE_DIGITAL_FILTER_LP,0,'H'},
		  {MACRO_OSC_SHAPE_DIGITAL_FILTER_PK,0,'I'},
		  {MACRO_OSC_SHAPE_DIGITAL_FILTER_BP,0,'J'},
		  {MACRO_OSC_SHAPE_DIGITAL_FILTER_HP,1,'0'},
		  {MACRO_OSC_SHAPE_VOSIM,1,'1'},
		  {MACRO_OSC_SHAPE_VOWEL,1,'2'},
		  {MACRO_OSC_SHAPE_VOWEL_FOF,1,'3'},

		  {MACRO_OSC_SHAPE_HARMONICS,1,'4'},

		  {MACRO_OSC_SHAPE_FM,1,'5'},
		  {MACRO_OSC_SHAPE_FEEDBACK_FM,1,'6'},
		  {MACRO_OSC_SHAPE_CHAOTIC_FEEDBACK_FM,1,'7'},

		  {MACRO_OSC_SHAPE_PLUCKED,1,'8'},
		  {MACRO_OSC_SHAPE_BOWED,1,'9'},
		  {MACRO_OSC_SHAPE_BLOWN,1,'A'},
		  {MACRO_OSC_SHAPE_FLUTED,1,'B'},
		  {MACRO_OSC_SHAPE_STRUCK_BELL,1,'C'},
		  {MACRO_OSC_SHAPE_STRUCK_DRUM,1,'D'},
		  {MACRO_OSC_SHAPE_KICK,1,'E'},
		  {MACRO_OSC_SHAPE_CYMBAL,1,'F'},
		  {MACRO_OSC_SHAPE_SNARE,1,'G'},
		  {MACRO_OSC_SHAPE_GRANULAR_CLOUD,1,'H'},
		  {MACRO_OSC_SHAPE_PARTICLE_NOISE,1,'I'}
			};

struct S_ConfigItem{
	int8_t symbol;
	Setting option;
};
typedef struct S_ConfigItem ConfigItem;
#define CONFIG_ITEMS_SYMBOLS_LEN	12
static const ConfigItem CONFIG_ITEMS[CONFIG_ITEMS_SYMBOLS_LEN]={
		{'A',SETTING_AD_ATTACK}, // Attack
		{'D',SETTING_AD_DECAY}, // Decay
		{'F',SETTING_AD_FM}, // FM Mod
		{'T',SETTING_AD_TIMBRE}, // Timbre Mod
		{'C',SETTING_AD_COLOR}, // Color Mod
		{'H',SETTING_AD_VCA}, // VCA Mod
		{'E',SETTING_META_MODULATION}, // Meta on off
		{'G',SETTING_TRIG_SOURCE}, // Trigger auto on off
		{'U',SETTING_QUANTIZER_SCALE}, // Quantizer Off Tone Semitone
		{'O',SETTING_QUANTIZER_ROOT}, // Root
		{'L',SETTING_VCO_FLATTEN}, // Flat On OFF
		{'R',SETTING_VCO_DRIFT} // Drift on off
};


void Ui::init(Adc* adc,Memory* memory) {
	display.init();
	encoder.Init();
	state = UI_STATE_SELECT_OSCILLATOR;
	timeoutCalibCounter=0;
	enterCalibCounter=0;
	this->adc = adc;
	this->memory = memory;

	// Load current oscillator from eeprom
	currentOscillator=(int8_t)memory->readUInt8(Memory::ADDR_CURRENT_OSCILLATOR);
	if(currentOscillator>=OSCILLATOR_INDEX_LEN)
	{
		currentOscillator=0;
	}
	display.showChar(LIMITED_OSCILLATORS[currentOscillator].symbol);
	display.showBank(LIMITED_OSCILLATORS[currentOscillator].bank);
	settings.SetValue(SETTING_OSCILLATOR_SHAPE, LIMITED_OSCILLATORS[currentOscillator].osc);
	//________________________

	// Load all settings from eeprom
	loadAllSettings();

}

void Ui::sysTick(void)
{
	encoder.sysTick();

	if(timeoutCalibCounter>0)
	{
		timeoutCalibCounter--;
		if(timeoutCalibCounter==0)
		{
			enterCalibCounter=0;
		}
	}

}

void Ui::saveAllSettings(void)
{
	uint16_t i;
	for(i=0; i<CONFIG_ITEMS_SYMBOLS_LEN; i++)
	{
		Setting setting_ = CONFIG_ITEMS[i].option;
		int16_t value = settings.GetValue(setting_);
		this->memory->writeUInt16(Memory::ADDR_BASE_SETTINGS+(i*(sizeof(uint16_t))), (uint16_t)value);
	}
}
void Ui::loadAllSettings(void)
{
	uint16_t i;
	for(i=0; i<CONFIG_ITEMS_SYMBOLS_LEN; i++)
	{
		Setting setting_ = CONFIG_ITEMS[i].option;

		int16_t valMem = (int16_t)this->memory->readUInt16(Memory::ADDR_BASE_SETTINGS+(i*(sizeof(uint16_t))));

		int16_t value = settings.metadata(setting_).Clip(valMem);
		settings.SetValue(setting_, value);
	}
}

void Ui::loop(void) {

	encoder.loop();

	switch(state)
	{
		case UI_STATE_SELECT_OSCILLATOR:
		{
			int32_t increment = encoder.increment();
			if (increment != 0)
			{
				if(increment>0){
					currentOscillator++;
				}
				else{
					currentOscillator--;
				}

				if(currentOscillator>=OSCILLATOR_INDEX_LEN)
					currentOscillator=(OSCILLATOR_INDEX_LEN-1);
				if(currentOscillator<0)
					currentOscillator=0;

				display.showChar(LIMITED_OSCILLATORS[currentOscillator].symbol);
				display.showBank(LIMITED_OSCILLATORS[currentOscillator].bank);
				display.showConfig(1);
			}
			if(encoder.pressed())
			{
				settings.SetValue(SETTING_OSCILLATOR_SHAPE, LIMITED_OSCILLATORS[currentOscillator].osc);
				display.showConfig(0);
				memory->writeUInt8(Memory::ADDR_CURRENT_OSCILLATOR, (uint8_t)currentOscillator);
			}

			if(encoder.pressedLong())
			{
				state = UI_STATE_CONFIG_MENU;
				display.showConfig(1);
				display.showBank(2); // both off
				configIndex=0;
				enterCalibCounter=0;
			}
			break;
		}
		case UI_STATE_CONFIG_MENU:
		{
			if(encoder.pressedLong())
			{
				state = UI_STATE_SELECT_OSCILLATOR;
				display.showConfig(0);
				display.showBank(LIMITED_OSCILLATORS[currentOscillator].bank);
				display.showChar(LIMITED_OSCILLATORS[currentOscillator].symbol);

				// save all settings in EEPROM
				saveAllSettings();
				break;
			}

			display.showChar(CONFIG_ITEMS[configIndex].symbol);

			int32_t increment = encoder.increment();
			if (increment != 0)
			{
				if(increment>0)
					configIndex++;
				else
					configIndex--;

				if(configIndex>=CONFIG_ITEMS_SYMBOLS_LEN)
					configIndex=CONFIG_ITEMS_SYMBOLS_LEN-1;
				if(configIndex<0)
					configIndex=0;
			}

			if(encoder.pressed())
			{
				// Enter to option
				state = UI_STATE_SET_VALUE;

				// Calibration mode detection
				if(enterCalibCounter==0)
					timeoutCalibCounter=2000;

				enterCalibCounter++;
				if(enterCalibCounter>=4)
				{
					enterCalibCounter=0;
					state = UI_STATE_CALIB;
					display.showConfig(1);
					display.showBank(3); // all leds ON
					display.showChar('2');
				}
				//___________________________
			}

			break;
		}
		case UI_STATE_SET_VALUE:
		{
			Setting setting_ = CONFIG_ITEMS[configIndex].option;
			int16_t value = settings.GetValue(setting_);

			int32_t increment = encoder.increment();
			if (increment != 0)
			{
				value = settings.metadata(setting_).Clip(value + increment);
				settings.SetValue(setting_, value);
			}

			// show value
			display.showValue(settings.metadata(setting_).max_value,value);

			if(encoder.pressed())
			{
				// go back to config menu
				state = UI_STATE_CONFIG_MENU;
			}
			break;
		}
		case UI_STATE_CALIB:
		{
			if(encoder.pressedLong())
			{
				state = UI_STATE_CONFIG_MENU;
				// go back to config menu
				display.showConfig(1);
				display.showBank(2); // all leds OFF
				break;
			}

			if(encoder.pressed())
			{
				// save CV values and go to step 2
				adc_code_c2 = adc->channel(ADC_CHANNEL_PITCH);
				adc_code_p0_min = adc->channel(ADC_CHANNEL_PARAM0);
				adc_code_p1_min = adc->channel(ADC_CHANNEL_PARAM1);

				state = UI_STATE_CALIB_STEP_2;
				display.showChar('4');
			}
			break;
		}
		case UI_STATE_CALIB_STEP_2:
		{
			if(encoder.pressedLong())
			{
				state = UI_STATE_CONFIG_MENU;
				// go back to config menu
				display.showConfig(1);
				display.showBank(2); // all leds OFF
				break;
			}

			if(encoder.pressed())
			{
				int32_t adc_code_c4 = adc->channel(ADC_CHANNEL_PITCH);
				int32_t adc_code_p0_max = adc->channel(ADC_CHANNEL_PARAM0);
				int32_t adc_code_p1_max = adc->channel(ADC_CHANNEL_PARAM1);
				int32_t adc_code_fm=0; //adc->channel(ADC_CHANNEL_FINE_TUNE);

				settings.Calibrate(adc_code_c2,
						      adc_code_c4,
						      adc_code_fm,
						      adc_code_p0_min,
						      adc_code_p0_max,
						      adc_code_p1_min,
						      adc_code_p1_max);

				// Save in eeprom
				this->memory->writeUInt32(Memory::ADDR_ADC_CODE_C2, adc_code_c2);
				this->memory->writeUInt32(Memory::ADDR_ADC_CODE_C4, adc_code_c4);
				this->memory->writeUInt32(Memory::ADDR_ADC_CODE_FM, adc_code_fm);
				this->memory->writeUInt32(Memory::ADDR_ADC_CODE_P0_MIN, adc_code_p0_min);
				this->memory->writeUInt32(Memory::ADDR_ADC_CODE_P0_MAX, adc_code_p0_max);
				this->memory->writeUInt32(Memory::ADDR_ADC_CODE_P1_MIN, adc_code_p1_min);
				this->memory->writeUInt32(Memory::ADDR_ADC_CODE_P1_MAX, adc_code_p1_max);

				// go back to config menu
				state = UI_STATE_CONFIG_MENU;
			}
			break;
		}
	}

}

