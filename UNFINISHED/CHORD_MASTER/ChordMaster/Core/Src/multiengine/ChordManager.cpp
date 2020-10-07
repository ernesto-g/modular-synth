/*
 * ChordManager.cpp
 *
 *  Created on: Oct 7, 2020
 *      Author: ernesto
 */

#include "multiengine/ChordManager.h"

#define SEMI * 128


static uint8_t calculateQuality(uint16_t param);
static uint8_t calculateVariation(uint16_t param,uint8_t quality);
static uint8_t calculateVoicing(uint16_t param);



void ChordManager::init(void) {

	this->currentChordIntervals[0]=0;
	this->currentChordIntervals[1]=0;
	this->currentChordIntervals[2]=0;
	this->pitch=0;
}


void ChordManager::calculateVoicings(int32_t pitchValueIn, uint16_t qualityValueIn,uint16_t variationValueIn,uint16_t voicingValue)
{
	uint8_t quality = calculateQuality(qualityValueIn>>2);
	uint8_t variation = calculateVariation(variationValueIn>>2,quality);
	uint8_t voicing = calculateVoicing(voicingValue);

	switch(quality)
	{
		case 0: // maj
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
		case 1: // min
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
		case 2: // dom
		{
			currentChordIntervals[0]=4 SEMI;
			currentChordIntervals[1]=7 SEMI;
			currentChordIntervals[2]=10 SEMI;
			break;
		}
		case 3: // dim
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
		case 4: // sus
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
		case 5: // aug
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
	// voicing:
	//     = 0 : root
	//     = 1 : first
	//     = 2 : Second
	//     = 3 : third

	//     = 4 : drop2
	//     = 5 : drop3
	//     = 6 : spread
	//     = 7 : spread2
	uint8_t r = param>>9;
	if(r>=8)
		r=7;
	return r;
}

static uint8_t calculateQuality(uint16_t param)
{
	// quality: 0 to 5
	// 0-820     = 0
	// 820-1640  = 1
	// 1640-2460 = 2
	// 2460-3280 = 3
	// 3280-4100 = 4
	// 4100-4095 = 5
	if(param>4100)
		return 5;
	if(param>3280)
		return 4;
	if(param>2460)
		return 3;
	if(param>1640)
		return 2;
	if(param>820)
		return 1;

	return 0;
}
static uint8_t calculateVariation(uint16_t param,uint8_t quality)
{
	if(quality==0 ||quality==1)
	{
		// 0 - 3
		if(param>3072)
			return 3;
		if(param>2048)
			return 2;
		if(param>1024)
			return 1;
		return 0;
	}

	if(quality==3 ||quality==5)
	{
		// 0 - 2
		if(param>2730)
			return 2;
		if(param>1365)
			return 1;
		return 0;
	}
	if(quality==4)
	{
		// 0 - 1
		if(param<2048)
			return 0;
		return 1;
	}

	return 0;
}


