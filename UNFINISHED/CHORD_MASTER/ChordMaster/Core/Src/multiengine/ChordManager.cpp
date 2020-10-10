/*
 * ChordManager.cpp
 *
 *  Created on: Oct 7, 2020
 *      Author: ernesto
 */

#include "multiengine/ChordManager.h"

#define SEMI * 128
#define DELTA_HYSTERESIS_FOR_QUALITY	150
#define DELTA_HYSTERESIS_FOR_VOICING	150
#define DELTA_HYSTERESIS_FOR_VARIAT		150


#define QUALITY_MAJ		0
#define QUALITY_MIN		1
#define QUALITY_DOM		2
#define QUALITY_DIM		3
#define QUALITY_SUS		4
#define QUALITY_AUG		5


static uint8_t calculateQuality(uint16_t param);
static uint8_t calculateVariation(uint16_t param,uint8_t quality);
static uint8_t calculateVoicing(uint16_t param);



void ChordManager::init(void) {

	this->currentChordIntervals[0]=0;
	this->currentChordIntervals[1]=0;
	this->currentChordIntervals[2]=0;
	this->pitch=0;
	this->harmonizerEnabled=1;
	this->harmonizerScale=1;
}


void ChordManager::calculateVoicings(int32_t pitchValueIn, uint16_t qualityValueIn,uint16_t variationValueIn,uint16_t voicingValue,int32_t rootValueIn)
{
	uint8_t quality;
	if(this->harmonizerEnabled==1)
	{
		quality = calculateHarmonizedQuality(pitchValueIn,rootValueIn);
	}
	else
	{
		quality = calculateQuality(qualityValueIn>>2); // get quality from input CV and Knob
	}


	uint8_t variation = calculateVariation(variationValueIn>>2,quality);
	uint8_t voicing = calculateVoicing(voicingValue);

	switch(quality)
	{
		case QUALITY_MAJ: // maj
		{
			switch(variation){
				case 0:{
					// Major triad
					currentChordIntervals[0]=4 SEMI;
					currentChordIntervals[1]=7 SEMI;
					currentChordIntervals[2]=12 SEMI;
					break;
				}
				case 1:{
					// Major7
					currentChordIntervals[0]=4 SEMI;
					currentChordIntervals[1]=7 SEMI;
					currentChordIntervals[2]=11 SEMI;
					break;
				}
				case 2:{
					// 6
					currentChordIntervals[0]=4 SEMI;
					currentChordIntervals[1]=7 SEMI;
					currentChordIntervals[2]=9 SEMI;
					break;
				}
				case 3:{
					// 7-5
					currentChordIntervals[0]=4 SEMI;
					currentChordIntervals[1]=6 SEMI;
					currentChordIntervals[2]=10 SEMI;
					break;
				}
			}
			break;
		}
		case QUALITY_MIN: // min
		{
			switch(variation){
				case 0:{
					// MINOR
					currentChordIntervals[0]=3 SEMI;
					currentChordIntervals[1]=7 SEMI;
					currentChordIntervals[2]=12 SEMI;
					break;
				}
				case 1:{
					// MINOR7
					currentChordIntervals[0]=3 SEMI;
					currentChordIntervals[1]=7 SEMI;
					currentChordIntervals[2]=11 SEMI;
					break;
				}
				case 2:{
					// MINOR6
					currentChordIntervals[0]=3 SEMI;
					currentChordIntervals[1]=7 SEMI;
					currentChordIntervals[2]=9 SEMI;
					break;
				}
				case 3:{
					// MINMAJ7
					currentChordIntervals[0]=3 SEMI;
					currentChordIntervals[1]=7 SEMI;
					currentChordIntervals[2]=11 SEMI;
					break;
				}
			}
			break;
		}
		case QUALITY_DOM: // dom
		{
			currentChordIntervals[0]=4 SEMI;
			currentChordIntervals[1]=7 SEMI;
			currentChordIntervals[2]=10 SEMI;
			break;
		}
		case QUALITY_DIM: // dim
		{
			switch(variation){
				case 0:{
					// DIM
					currentChordIntervals[0]=3 SEMI;
					currentChordIntervals[1]=6 SEMI;
					currentChordIntervals[2]=12 SEMI;
					break;
				}
				case 1:{
					// DIM7
					currentChordIntervals[0]=3 SEMI;
					currentChordIntervals[1]=6 SEMI;
					currentChordIntervals[2]=9 SEMI;
					break;
				}
				case 2:{
					// half-dim
					currentChordIntervals[0]=3 SEMI;
					currentChordIntervals[1]=6 SEMI;
					currentChordIntervals[2]=10 SEMI;
					break;
				}
			}
			break;
		}
		case QUALITY_SUS: // sus
		{
			switch(variation){
				case 0:{
					// SUS2
					currentChordIntervals[0]=2 SEMI;
					currentChordIntervals[1]=7 SEMI;
					currentChordIntervals[2]=12 SEMI;
					break;
				}
				case 1:{
					// SUS4
					currentChordIntervals[0]=5 SEMI;
					currentChordIntervals[1]=7 SEMI;
					currentChordIntervals[2]=12 SEMI;
					break;
				}
			}
			break;
		}
		case QUALITY_AUG: // aug
		{
			switch(variation){
				case 0:{
					// AUG
					currentChordIntervals[0]=4 SEMI;
					currentChordIntervals[1]=8 SEMI;
					currentChordIntervals[2]=12 SEMI;
					break;
				}
				case 1:{
					// AUG7
					currentChordIntervals[0]=4 SEMI;
					currentChordIntervals[1]=8 SEMI;
					currentChordIntervals[2]=10 SEMI;
					break;
				}
				case 2:{
					// 6
					currentChordIntervals[0]=4 SEMI;
					currentChordIntervals[1]=7 SEMI;
					currentChordIntervals[2]=9 SEMI;
					break;
				}
			}
			break;
		}
	}

	// calculate voicings
	int32_t voice0 = pitchValueIn;

	switch(voicing)
	{
		case 1: // first OK
		{
			this->pitch = voice0 + 12 SEMI;
			currentChordIntervals[0] = currentChordIntervals[0] - 12 SEMI;
			currentChordIntervals[1] = currentChordIntervals[1] - 12 SEMI;
			currentChordIntervals[2] = currentChordIntervals[2] - 12 SEMI;
			break;
		}
		case 2: // second  OK
		{
			this->pitch = voice0 + 12 SEMI;
			//currentChordIntervals[0] = currentChordIntervals[0] - 12 SEMI;
			currentChordIntervals[1] = currentChordIntervals[1] - 12 SEMI;
			currentChordIntervals[2] = currentChordIntervals[2] - 12 SEMI;
			break;
		}
		case 3: // third OK
		{
			this->pitch = voice0 + 12 SEMI;
			//currentChordIntervals[0] = currentChordIntervals[0] - 12 SEMI;
			//currentChordIntervals[1] = currentChordIntervals[1] - 12 SEMI;
			currentChordIntervals[2] = currentChordIntervals[2] - 12 SEMI;
			break;
		}
		case 4: // drop2 OK
		{
			this->pitch = voice0;
			currentChordIntervals[1] = currentChordIntervals[1] - 12 SEMI;
			break;
		}
		case 5: // drop3 OK
		{
			this->pitch = voice0;
			currentChordIntervals[0] = currentChordIntervals[0] - 12 SEMI;
			break;
		}
		case 6: // spread OK
		{
			this->pitch = voice0 - 12 SEMI ;
			currentChordIntervals[0] = currentChordIntervals[0] + 12 SEMI;
			currentChordIntervals[1] = currentChordIntervals[1] + 12 SEMI;
			currentChordIntervals[2] = currentChordIntervals[2] + 24 SEMI;
			break;
		}
		case 7: // spread2 OK
		{
			this->pitch = voice0 - 12 SEMI ;
			//currentChordIntervals[0] = currentChordIntervals[0] + 12 SEMI;
			currentChordIntervals[1] = currentChordIntervals[1] + 12 SEMI;
			currentChordIntervals[2] = currentChordIntervals[2] + 24 SEMI;
			break;
		}

		default: // root OK
		{
			this->pitch = voice0;
			break;
		}
	}





}

int16_t ChordManager::getChord1(void)
{
	return this->currentChordIntervals[0];
}
int16_t ChordManager::getChord2(void)
{
	return this->currentChordIntervals[1];
}
int16_t ChordManager::getChord3(void)
{
	return this->currentChordIntervals[2];
}
int32_t ChordManager::getPitch(void)
{
	return this->pitch;
}




static uint8_t calculateVoicing(uint16_t param)
{
	uint8_t ret;
	static uint8_t prevRet=0;

	// voicing:
	//     = 0 : root
	//     = 1 : first
	//     = 2 : Second
	//     = 3 : third

	//     = 4 : drop2
	//     = 5 : drop3
	//     = 6 : spread
	//     = 7 : spread2
	if(param>(3584+DELTA_HYSTERESIS_FOR_VOICING))
		ret=7;
	else if(param>(3584-DELTA_HYSTERESIS_FOR_VOICING))
		ret=prevRet;

	else if(param>(3072+DELTA_HYSTERESIS_FOR_VOICING))
		ret=6;
	else if(param>(3072-DELTA_HYSTERESIS_FOR_VOICING))
		ret=prevRet;

	else if(param>(2560+DELTA_HYSTERESIS_FOR_VOICING))
		ret=5;
	else if(param>(2560-DELTA_HYSTERESIS_FOR_VOICING))
		ret=prevRet;

	else if(param>(2048+DELTA_HYSTERESIS_FOR_VOICING))
		ret=4;
	else if(param>(2048-DELTA_HYSTERESIS_FOR_VOICING))
		ret=prevRet;

	else if(param>(1536+DELTA_HYSTERESIS_FOR_VOICING))
		ret=3;
	else if(param>(1536-DELTA_HYSTERESIS_FOR_VOICING))
		ret=prevRet;

	else if(param>(1024+DELTA_HYSTERESIS_FOR_VOICING))
		ret=2;
	else if(param>(1024-DELTA_HYSTERESIS_FOR_VOICING))
		ret=prevRet;

	else if(param>(512+DELTA_HYSTERESIS_FOR_VOICING))
		ret=1;
	else if(param>(512-DELTA_HYSTERESIS_FOR_VOICING))
		ret=prevRet;

	else
		ret=0;

	prevRet = ret;
	return ret;
}

static uint8_t calculateQuality(uint16_t param)
{
	uint8_t ret;
	static uint8_t prevRet=0;

	// quality: 0 to 5
	// 0-820     = 0
	// 820-1640  = 1
	// 1640-2460 = 2
	// 2460-3280 = 3
	// 3280-4100 = 4
	// 4100-4095 = 5
	if(param>(4100+DELTA_HYSTERESIS_FOR_QUALITY))
		ret= 5;
	else if(param>(4100-DELTA_HYSTERESIS_FOR_QUALITY))
		ret = prevRet;

	else if(param>(3280+DELTA_HYSTERESIS_FOR_QUALITY))
		ret= 4;
	else if(param>(3280-DELTA_HYSTERESIS_FOR_QUALITY))
		ret= prevRet;

	else if(param>(2460+DELTA_HYSTERESIS_FOR_QUALITY))
		ret= 3;
	else if(param>(2460-DELTA_HYSTERESIS_FOR_QUALITY))
		ret= prevRet;


	else if(param>(1640+DELTA_HYSTERESIS_FOR_QUALITY))
		ret= 2;
	else if(param>(1640-DELTA_HYSTERESIS_FOR_QUALITY))
		ret= prevRet;

	else if(param>(820+DELTA_HYSTERESIS_FOR_QUALITY))
		ret= 1;
	else if(param>(820-DELTA_HYSTERESIS_FOR_QUALITY))
		ret= prevRet;

	else
		ret=0;

	prevRet = ret;
	return ret;
}
static uint8_t calculateVariation(uint16_t param,uint8_t quality)
{
	uint8_t ret;
	static uint8_t prevRet01=0;
	static uint8_t prevRet35=0;
	static uint8_t prevRet4=0;

	if(quality==0 ||quality==1)
	{
		// 0 - 3
		if(param>(3072+DELTA_HYSTERESIS_FOR_VARIAT))
			ret=3;
		else if(param>(3072-DELTA_HYSTERESIS_FOR_VARIAT))
			ret= prevRet01;

		else if(param>(2048+DELTA_HYSTERESIS_FOR_VARIAT))
			ret= 2;
		else if(param>(2048-DELTA_HYSTERESIS_FOR_VARIAT))
			ret= prevRet01;

		else if(param>(1024+DELTA_HYSTERESIS_FOR_VARIAT))
			ret= 1;
		else if(param>(1024-DELTA_HYSTERESIS_FOR_VARIAT))
			ret= prevRet01;
		else
			ret=0;

		prevRet01 = ret;
		return ret;
	}

	if(quality==3 ||quality==5)
	{
		// 0 - 2
		if(param>(2730+DELTA_HYSTERESIS_FOR_VARIAT))
			ret=2;
		else if(param>(2730-DELTA_HYSTERESIS_FOR_VARIAT))
			ret=prevRet35;

		else if(param>(1365+DELTA_HYSTERESIS_FOR_VARIAT))
			ret=1;
		else if(param>(1365-DELTA_HYSTERESIS_FOR_VARIAT))
			ret=prevRet35;
		else
			ret=0;

		prevRet35 = ret;
		return ret;
	}
	if(quality==4)
	{
		// 0 - 1
		if(param>(2048+DELTA_HYSTERESIS_FOR_VARIAT))
			ret = 1;
		else if(param>(2048-DELTA_HYSTERESIS_FOR_VARIAT))
			ret = prevRet4;
		else
			ret=0;

		prevRet4 = ret;
		return ret;
	}

	return 0;
}


uint8_t ChordManager::calculateHarmonizedQuality(int32_t pitchValueIn,int32_t rootValueIn)
{
	uint16_t notePositionInScale;
	uint16_t note = pitchValueIn % (12 SEMI);
	uint16_t root = rootValueIn % (12 SEMI);
	if(note>=root)
	{
		notePositionInScale = (note - root)/(1 SEMI);
	}
	else
	{
		notePositionInScale = (( (12 SEMI)-root) + note)/(1 SEMI);
	}

	switch(this->harmonizerScale)
	{
		case 0: // MAJOR SCALE
		{
			switch(notePositionInScale)
			{
				case 0: return QUALITY_MAJ; //(diatonic)  I
				case 1: return QUALITY_MAJ; //(modal interchange)
				case 2: return QUALITY_MIN; //(diatonic)  ii
				case 3: return QUALITY_MAJ; //(modal interchange)
				case 4: return QUALITY_MIN; //(diatonic)  iii
				case 5: return QUALITY_MAJ; //(diatonic)  IV
				case 6: return QUALITY_DOM; //(modal interchange)
				case 7: return QUALITY_DOM; //(diatonic)  V
				case 8: return QUALITY_MAJ; //(modal interchange)
				case 9: return QUALITY_MIN; //(diatonic)  vi
				case 10: return QUALITY_DOM; //(modal interchange)
				case 11: return QUALITY_DIM; //(diatonic) vii
			}
			break;
		}
		case 1: // MINOR
		{
			switch(notePositionInScale)
			{
				case 0: return QUALITY_MIN; // i
				case 1: return QUALITY_MAJ;
				case 2: return QUALITY_DIM; // ii
				case 3: return QUALITY_MAJ; // III
				case 4: return QUALITY_DIM;
				case 5: return QUALITY_MIN; // iv
				case 6: return QUALITY_AUG;
				case 7: return QUALITY_DOM; // E  V DOM
				case 8: return QUALITY_MAJ; // VI
				case 9: return QUALITY_DIM;
				case 10: return QUALITY_MAJ; // VII
				case 11: return QUALITY_DIM;
			}
			break;
		}
	}
	return QUALITY_MAJ;
}
