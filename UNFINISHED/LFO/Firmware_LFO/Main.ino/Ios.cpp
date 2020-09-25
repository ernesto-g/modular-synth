#include <Arduino.h>

#define PIN_CLK_IN_D2   2
#define PIN_BOC1_D1     1

#define PIN_ENCODER_D4          4
#define PIN_ENCODER_D5          5
#define PIN_ENCODER_D6          6

#define PIN_LED0             7
#define PIN_LED1             8
#define PIN_LED2            10
#define PIN_LED3            12
#define PIN_LED4            13

void ios_init(void)
{
    pinMode(PIN_CLK_IN_D2,INPUT_PULLUP);
    pinMode(PIN_BOC1_D1,OUTPUT);  
    digitalWrite(PIN_BOC1_D1,0);

    // encoder configuration
    pinMode(PIN_ENCODER_D4,INPUT_PULLUP); // Encoder A
    pinMode(PIN_ENCODER_D5,INPUT_PULLUP); // Encoder B
    pinMode(PIN_ENCODER_D6,INPUT_PULLUP); // Encoder SW
    // Enable interrupts for D4 D5 pins
    PCICR |= (1 << PCIE2);    // This enables Pin Change Interrupt 2 that covers D4 and D5
    PCMSK2 |= (1 << PCINT20) | (1 << PCINT21);  // This enables the interrupt for pin D4 and D5    
    //_______________________
    
    pinMode(PIN_LED0,OUTPUT);
    pinMode(PIN_LED1,OUTPUT);
    pinMode(PIN_LED2,OUTPUT);
    pinMode(PIN_LED3,OUTPUT);
    pinMode(PIN_LED4,OUTPUT);    
}

void ios_setLed(uint8_t index,uint8_t value)
{
  switch(index)
  {
    case 0: digitalWrite(PIN_LED0,value);break;
    case 1: digitalWrite(PIN_LED1,value);break;
    case 2: digitalWrite(PIN_LED2,value);break;
    case 3: digitalWrite(PIN_LED3,value);break;
    case 4: digitalWrite(PIN_LED4,value);break;
  }
}

void ios_setBocOut(uint8_t value)
{
  digitalWrite(PIN_BOC1_D1,value);
}


uint8_t ios_getClkIn(void)
{
    return digitalRead(PIN_CLK_IN_D2);
}

uint8_t ios_getEncoderSw(void)
{
  return digitalRead(PIN_ENCODER_D6);
}

uint8_t ios_getEncoderPinA(void)
{
  return PIN_ENCODER_D4;
}
uint8_t ios_getEncoderPinB(void)
{
  return PIN_ENCODER_D5;
}


uint16_t ios_getAnalogValue(uint8_t index)
{
  switch(index)
  {
    case 0: return analogRead(A0); 
    case 1: return analogRead(A1); 
    case 2: return analogRead(A2); 
    case 3: return analogRead(A3); 
  }
  return 0;  
}
