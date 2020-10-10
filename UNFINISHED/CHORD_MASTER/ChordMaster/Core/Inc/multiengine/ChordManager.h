/*
 * ChordManager.h
 *
 *  Created on: Oct 7, 2020
 *      Author: ernesto
 */

#ifndef INC_MULTIENGINE_CHORDMANAGER_H_
#define INC_MULTIENGINE_CHORDMANAGER_H_

#include <stdint.h>

class ChordManager
{
	public:
		void init(void);
		int16_t getChord1(void);
		int16_t getChord2(void);
		int16_t getChord3(void);
		int32_t getPitch(void);
		void calculateVoicings(int32_t pitchIn, uint16_t qualityValueIn,uint16_t variationValueIn,uint16_t voicingValue,int32_t rootValueIn);
	private:
		int16_t currentChordIntervals[3];
		int32_t pitch;
		uint8_t harmonizerEnabled;
		uint8_t harmonizerScale;
		uint8_t calculateHarmonizedQuality(int32_t pitchValueIn,int32_t rootValueIn);
};


#endif /* INC_MULTIENGINE_CHORDMANAGER_H_ */
