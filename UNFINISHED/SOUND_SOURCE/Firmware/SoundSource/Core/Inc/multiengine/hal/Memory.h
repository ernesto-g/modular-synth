/*
 * Memory.h
 *
 *  Created on: Sep 12, 2020
 *      Author: ernesto
 */

#ifndef INC_MULTIENGINE_HAL_MEMORY_H_
#define INC_MULTIENGINE_HAL_MEMORY_H_


class Memory{
	public:
		uint32_t readUInt32(uint16_t address);
		uint8_t writeUInt32(uint16_t address,uint32_t value);

		uint8_t readUInt8(uint16_t address);
		uint8_t writeUInt8(uint16_t address,uint8_t value);

		uint16_t readUInt16(uint16_t address);
		uint8_t writeUInt16(uint16_t address,uint16_t value);

		const static uint16_t ADDR_ADC_CODE_C2=0;
		const static uint16_t ADDR_ADC_CODE_C4=4;
		const static uint16_t ADDR_ADC_CODE_FM=8;
		const static uint16_t ADDR_ADC_CODE_P0_MIN=12;
		const static uint16_t ADDR_ADC_CODE_P0_MAX=16;
		const static uint16_t ADDR_ADC_CODE_P1_MIN=20;
		const static uint16_t ADDR_ADC_CODE_P1_MAX=24;
		const static uint8_t ADDR_CURRENT_OSCILLATOR=28;
		// 29 to 31 free
		const static uint8_t ADDR_BASE_SETTINGS=32;

};


#endif /* INC_MULTIENGINE_HAL_MEMORY_H_ */
