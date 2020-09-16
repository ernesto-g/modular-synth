/*
MIT License

Copyright (c) 2020 Ernesto Gigliotti
Copyright (c) 2018 Marcel Meyer-Garcia

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 */


extern "C" {
#include "stm32f1xx_hal.h"
#include "multiengine/hal/MultiEngineHAL.h"

static I2C_HandleTypeDef hi2c1;
#define MEM_ADDRESS	0xA0

static void (*callbackDMA)(int) = NULL;
static void MX_I2C1_Init(void);

void DMA1_Channel5_IRQHandler()
{
	// check if the "transfer complete event" has triggered the interrupt
	if( DMA1->ISR & DMA_ISR_TCIF5 ){
		// complete transfer
		if(callbackDMA!=NULL)
			callbackDMA(1);
	}
	else if(DMA1->ISR & DMA_ISR_HTIF5)
	{
		// half transfer
		if(callbackDMA!=NULL)
			callbackDMA(0);
	}
	// clear the interrupt flag
	DMA1->IFCR |= DMA_IFCR_CGIF5;
}

void mehal_init(uint32_t* samplesBuffer,uint32_t samplesBufferSize,void (*fnCallbackDMA)(int))
{
	callbackDMA = fnCallbackDMA;

	/* TIMER 2 SETUP FOR PWM */
	// enable timer 2 clock
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
	// enable AFIO and GPIO port A clock
	RCC->APB2ENR |= RCC_APB2ENR_AFIOEN | RCC_APB2ENR_IOPAEN;
	// configure pin PA0 as AFIO push-pull output with max speed
	GPIOA->CRL &=~(GPIO_CRL_MODE0 | GPIO_CRL_CNF0);
	GPIOA->CRL |= GPIO_CRL_MODE0 | GPIO_CRL_CNF0_1;

	// set timer 2 prescaler to 1, so the timer frequency is equal to the APB2 frequency (here 72MHz)
	// the actual prescaling factor is PSC+1
	TIM2->PSC = 0;
	// set the auto-reload value, i.e. the max counter value
	TIM2->ARR = 255;
	// set the output compare value which determines the duty cycle
	TIM2->CCR1 = 128;
	// configure channel 1 to PWM mode 1 together with the preload feature
	// refer to ST's app note "AN4776" p.14-16 for details about preload
	// PWM mode 1: duty_cycle = CCRx / ARR
	// PWM mode 2: duty_cycle = 1 - CCRx / ARR
	// if the counter is not up- but downcounting it is inverted
	// by default, the timers are upcounting
	TIM2->CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1PE;
	// set polarity of the output to non-inverted (default, so actually not necessary)
	TIM2->CCER &=~TIM_CCER_CC1P;
	// enable the compare outputs
	TIM2->CCER |= TIM_CCER_CC1E;
	// left-aligned mode is default

	/* 	enable center-aligned mode 3
	 	in center-aligned mode the PWM frequency is only half as high as in edge aligned mode
	 	so here f_PWM = f_APB2 / ( 2*(PSC+1)*ARR ) = approx. 140.6 kHz
	*/
	//TIM2->CR1 |= TIM_CR1_CMS;

	// enable the timer 2 counter
	TIM2->CR1 |= TIM_CR1_CEN;


	/* DMA SETUP */
	//enable DMA1 clock
	RCC->AHBENR |= RCC_AHBENR_DMA1EN;
	// set the peripheral address where to put the data
	// in this case it's the timers compare (duty-cycle) register
	DMA1_Channel5->CPAR = (uint32_t) (&(TIM2->CCR1));
	// set the memory address from where to fetch the data
	// this is the starting address of our array of samples
	DMA1_Channel5->CMAR = (uint32_t) samplesBuffer;
	//DMA1_Channel5->CMAR = (uint32_t) sine_LUT;
	// set the number of data (samples) to be transferred
	DMA1_Channel5->CNDTR = samplesBufferSize;
	//DMA1_Channel5->CNDTR = 9; //for sine_LUT
	// set data transfer direction to "memory->peripheral"
	DMA1_Channel5->CCR |= DMA_CCR_DIR;
	// set the DMA channel priority to "high"
	DMA1_Channel5->CCR |= DMA_CCR_PL_1;

	// enable circular mode (this means it will be repeated eternally)
	DMA1_Channel5->CCR |= DMA_CCR_CIRC;

	// enable memory increment mode
	DMA1_Channel5->CCR |= DMA_CCR_MINC;
	// disable peripheral increment mode
	DMA1_Channel5->CCR &=~DMA_CCR_PINC;
	// set memory size to 8bit
	DMA1_Channel5->CCR &=~DMA_CCR_MSIZE;
	// set peripheral size to 16bit
	DMA1_Channel5->CCR |= DMA_CCR_PSIZE_0;

	// enable transfer complete interrupt and halft complete interrupt
	DMA1_Channel5->CCR |= DMA_CCR_TCIE;
	DMA1_Channel5->CCR |= DMA_CCR_HTIE;
	// enable DMA1 interrupt
	NVIC_EnableIRQ(DMA1_Channel5_IRQn);
	// globally enable interrupts
	__enable_irq();
	//________________________________________________________________

	// enable the DMA channel
	DMA1_Channel5->CCR |= DMA_CCR_EN;


	/* TIMER 1 SETUP */
	// enable the clock for Timer1 (f_PCLK2=72MHz)
	RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
	// use a prescaler of 1, i.e. f_TIM1=f_PCLK2/(0+1)=72MHz
	TIM1->PSC = 0;
	// set timer1 auto-reload value
	// so the timer overflow will occur with a frequency of 72MHz/1633=approx. 44.1kHz
	//TIM1->ARR = 1633;
	TIM1->ARR = 750; // 96Khz
	// enable timer update DMA request
	// i.e. at every counter update/overflow, a DMA request will be issued
	// which triggers a DMA data transfer
	TIM1->DIER |= TIM_DIER_UDE;
	// start timer1
	TIM1->CR1 |= TIM_CR1_CEN;


	// PINS Configuration
	__HAL_RCC_GPIOB_CLK_ENABLE(); // Enable clck for portb
	__HAL_RCC_GPIOA_CLK_ENABLE(); // Enable clck for porta
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	/*
	// PIN  B6 (I2C DATA)
	GPIO_InitStruct.Pin = GPIO_PIN_6;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	//((GPIOB->IDR>>6)&0x01) // Read DATA
	//GPIOB->CRL &= ~(GPIO_CRL_MODE6); // DATA:INPUT
	//GPIOB->CRL |= (GPIO_CRL_MODE6_1); // DATA:OUTPUT
	//GPIOB -> BSRR = (1 << 6); // DATA=1
	//GPIOB -> BRR = (1 << 6); // DATA=0

	// PIN  B7 (I2C CLK)
	GPIO_InitStruct.Pin = GPIO_PIN_7;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	//GPIOB -> BSRR = (1 << 7); // CLK=1
	//GPIOB -> BRR = (1 << 7); // CLK=0
	*/

	// PIN  B11 GATE_IN
	GPIO_InitStruct.Pin = GPIO_PIN_11;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	// PIN  B12 ENC_A
	GPIO_InitStruct.Pin = GPIO_PIN_12;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	// PIN  B13 ENC_B
	GPIO_InitStruct.Pin = GPIO_PIN_13;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	// PIN  B14 ENC_SW
	GPIO_InitStruct.Pin = GPIO_PIN_14;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	// PIN  B15 LED_CONFIG
	GPIO_InitStruct.Pin = GPIO_PIN_15;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_RESET); //Configure GPIO pin Output Level


	// PIN  A1 ANALOG INPUT PARAM2
	GPIO_InitStruct.Pin = GPIO_PIN_1;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	// PIN  A2 ANALOG INPUT PARAM1
	GPIO_InitStruct.Pin = GPIO_PIN_2;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	// PIN  A3 ANALOG INPUT FINE_TUNE
	GPIO_InitStruct.Pin = GPIO_PIN_3;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	// PIN  A4 ANALOG INPUT FM
	GPIO_InitStruct.Pin = GPIO_PIN_4;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	// PIN  A5 ANALOG INPUT PITCH
	GPIO_InitStruct.Pin = GPIO_PIN_5;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);


	// PIN  A8 LED_BANK1
	GPIO_InitStruct.Pin = GPIO_PIN_8;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET); //Configure GPIO pin Output Level

	// PIN  A9 LED_BANK0
	GPIO_InitStruct.Pin = GPIO_PIN_9;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_RESET); //Configure GPIO pin Output Level

	// PIN  A10 DISPLAY_CLK
	GPIO_InitStruct.Pin = GPIO_PIN_10;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_RESET); //Configure GPIO pin Output Level

	// PIN  A11 DISPLAY_LATCH
	GPIO_InitStruct.Pin = GPIO_PIN_11;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_RESET); //Configure GPIO pin Output Level

	// PIN  A12 DISPLAY_DATA
	GPIO_InitStruct.Pin = GPIO_PIN_12;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET); //Configure GPIO pin Output Level


	// Configure ADC1
	// set ADC clock prescaler to 6 to get a 12MHZ ADC clock (14MHz is maximum)
	RCC->CFGR &=~RCC_CFGR_ADCPRE_0;
	RCC->CFGR |= RCC_CFGR_ADCPRE_1;
	// enable ADC1 clock
	RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
	// enable ADC1
	ADC1->CR2 |= ADC_CR2_ADON;
	// select software start as external event trigger
	ADC1->CR2 |= (ADC_CR2_EXTSEL_2 | ADC_CR2_EXTSEL_1 | ADC_CR2_EXTSEL_0);
	// set sample time to minimum (1.5µs)
	ADC1->SMPR2 &=~(ADC_SMPR2_SMP0_2 | ADC_SMPR2_SMP0_1 | ADC_SMPR2_SMP0_0);
	// set data alignment to right
	ADC1->CR2 &=~ADC_CR2_ALIGN;
	// wait 10µs before ADC calibration
	//delay(10);
	HAL_Delay(1); // 1ms
	// start calibration of ADC1
	ADC1->CR2 |= ADC_CR2_CAL;
	// wait until calibration is finished
	while( (ADC1->CR2) & ADC_CR2_CAL );

	// Configure I2C
	MX_I2C1_Init();


}

uint16_t mehal_readADC(uint8_t channel)
{
	channel = channel + 1; // A0 is not used (its PWM output)


	// check if parameter is within range
	if( (channel>17) || (channel<0) ) return 0xFFFF;
	// set number of ADC channels to scan to 1 (so we read only 1 channel)
	ADC1->SQR1 = 0;
	// set the channel that we want to use
	ADC1->SQR3 = (channel << ADC_SQR3_SQ1_Pos);
	// clear the end of conversion flag
	ADC1->SR &=~ADC_SR_EOC;
	// start conversion
	ADC1->CR2 |= ADC_CR2_ADON;
	// trigger conversion
	ADC1->CR2 |= ADC_CR2_SWSTART;
	// wait until conversion is finished
	while( !(ADC1->SR & ADC_SR_EOC) );
	// read conversion result (12bit) from the data register and return it
	return ((ADC1->DR) & 0b111111111111);
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 400000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    //Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}


uint8_t mehal_i2cMemRead(uint16_t memAddress,uint8_t *pData, uint16_t size)
{
	return (uint8_t)HAL_I2C_Mem_Read(&hi2c1, MEM_ADDRESS, memAddress, I2C_MEMADD_SIZE_8BIT, pData, size, 300);
}

uint8_t mehal_i2cMemWrite(uint16_t memAddress,uint8_t *pData, uint16_t size,uint8_t flagWait)
{
	uint8_t r;
	r= (uint8_t)HAL_I2C_Mem_Write(&hi2c1, MEM_ADDRESS, memAddress, I2C_MEMADD_SIZE_8BIT, pData, size, 500);
	if(flagWait)
		mehal_delay(30);
	return r;
}


void mehal_toogleBoardLed(void)
{
	  HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
}

void mehal_debugPinSet(void)
{
	//HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
}
void mehal_debugPinReset(void)
{
	//HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
}

uint8_t mehal_readGateInput(void)
{
	return HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_11);
}
uint8_t mehal_readEncoderSwitch(void)
{
	return HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_14);
}
uint8_t mehal_readEncoderA(void)
{
	return HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12);
}
uint8_t mehal_readEncoderB(void)
{
	return HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13);
}


void mehal_sendDataTo595(uint8_t data)
{
	// LATCH=0
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_RESET);
	// CLK=0
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_RESET);

	uint8_t i;
	for(i=0; i<8;i++)
	{
		if( (data&(0x80>>i)) == 0)
		{
			// DATA=0
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET);
		}
		else
		{
			// DATA=1
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_SET);
		}
		// CLK=1
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_SET);
		// CLK=0
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_RESET);
	}
	// LATCH=1
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_SET);
}


void mehal_setConfigLed(uint8_t state)
{
	if(state==1)
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_SET);
	else
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_RESET);
}
void mehal_setBank1Led(uint8_t state)
{
	if(state==1)
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);
	else
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
}

void mehal_setBank0Led(uint8_t state)
{
	if(state==1)
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_SET);
	else
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_RESET);
}


void mehal_delay(uint32_t t)
{
	  HAL_Delay(t);
}




} // extern C end





