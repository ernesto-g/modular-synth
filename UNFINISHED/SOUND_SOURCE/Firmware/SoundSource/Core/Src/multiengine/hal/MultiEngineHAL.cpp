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

static void (*callbackDMA)(int) = NULL;

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


	// PIN  B12 for debug
	__HAL_RCC_GPIOB_CLK_ENABLE(); // Enable clck for portb
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET); //Configure GPIO pin Output Level
	GPIO_InitStruct.Pin = GPIO_PIN_12;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	// PIN  B13 for GATE INPUT
	GPIO_InitTypeDef GPIO_InitStruct2 = {0};
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET); //Configure GPIO pin Output Level
	GPIO_InitStruct2.Pin = GPIO_PIN_13;
	GPIO_InitStruct2.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct2.Pull = GPIO_PULLUP;
	GPIO_InitStruct2.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct2);


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



void mehal_toogleBoardLed(void)
{
	  HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
}

void mehal_debugPinSet(void)
{
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
}
void mehal_debugPinReset(void)
{
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
}

uint8_t mehal_readGateInput(void)
{
	return HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13);
}


void mehal_delay(uint32_t t)
{
	  HAL_Delay(t);
}



} // extern C end





