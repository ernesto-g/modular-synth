/*
 * MainLoop.h
 *
 *  Created on: Aug 13, 2020
 *      Author: ernesto
 */

#ifndef INC_MULTIENGINE_MAINLOOP_H_
#define INC_MULTIENGINE_MAINLOOP_H_

#include <stdint.h>

class MainLoop
{
	public:
	void init(void);
	void loop(void);
	void render(uint8_t* out, uint32_t outSize);
	void adcStateMachine(void);
};




#endif /* INC_MULTIENGINE_MAINLOOP_H_ */
