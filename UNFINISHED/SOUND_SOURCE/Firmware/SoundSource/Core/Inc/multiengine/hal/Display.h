/*
 * Display.h
 *
 *  Created on: Sep 6, 2020
 *      Author: ernesto
 */

#ifndef INC_MULTIENGINE_DISPLAY_H_
#define INC_MULTIENGINE_DISPLAY_H_

#include <stdint.h>

class Display
{
	public:
	void init(void);
	void showChar(int8_t c);
	void showBank(uint8_t bankNumber);
	void showConfig(uint8_t flagOnOff);
	void showValue(uint8_t maxValue,uint8_t value);
};



#endif /* INC_MULTIENGINE_DISPLAY_H_ */
