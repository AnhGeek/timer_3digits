#include "stm8s.h"
//dig1 D6
//dig2 A1
//dig3 A2
uint8_t led7_b[10]     = {0b11001111,0b00010000,0b00010000,0b00010000,0b00000000,0b00100000,0b00100000,0b00010000,0b00000000,0b00000000};
uint8_t led7_c[10]     = {0b01000111,0b01111000,0b11000000,0b01010000,0b01111000,0b01010000,0b01000000,0b01110000,0b01000000,0b01010000};
uint8_t led7_d[10] 		 = {0b00000100,0b00000100,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000100,0b00000000,0b00000000};
uint8_t led7_d_ddr[10] = {0b01000010,0b01000010,0b01000110,0b01000110,0b01000110,0b01000110,0b01000110,0b01000010,0b01000110,0b01000110};

uint8_t DIG1 = 0, DIG2 = 0, DIG3 = 0, DP = 0;
uint8_t seg = 1, mode = 0, input = 0, mode_temp = 0;
int16_t value_set = 0, value = 0;
uint16_t value_button = 0;
uint16_t timer_1ms = 0, delay_after_reset = 20, reset_count = 0;
uint16_t timer1_value = 0, timer_second = 1000, timer_toggle = 0;
void delay(uint16_t nCount)
{
  /* Decrement nCount value */
  while (nCount != 0)
  {
    nCount--;
  }
}
unsigned int clock(void)
{
	unsigned char h = TIM1->CNTRH;
	unsigned char l = TIM1->CNTRL;
	return((unsigned int)(h) << 8 | l);
}
void EEWriteU8(uint16_t address, int8_t value)
{
 // Check if the EEPROM is write-protected. If it is then unlock the EEPROM.
 if ( ((FLASH->IAPSR >> 3)&0x01) == 0) {
 FLASH->DUKR = 0xAE;
 delay(1000);
 FLASH->DUKR = 0x56;
 }
 // Write the data to the EEPROM.
 (*(uint8_t *) (0x4000 + address)) = value;
 // Now write protect the EEPROM.
 FLASH->IAPSR |= (1 << 3);
}
void EEWriteU16(uint16_t address, int16_t value)
{
 // Check if the EEPROM is write-protected. If it is then unlock the EEPROM.
 if ( ((FLASH->IAPSR >> 3)&0x01) == 0) {
 FLASH->DUKR = 0xAE;
 delay(1000);
 FLASH->DUKR = 0x56;
 }
 // Write the data to the EEPROM.
 (*(uint8_t *) (0x4000 + address)) = (value >> 8) & 0xFF;
 (*(uint8_t *) (0x4000 + address + 1)) = value & 0xFF;
 // Now write protect the EEPROM.
 FLASH->IAPSR |= (1 << 3);
}
int16_t EEReadU16(uint16_t address)
{
 // Check if the EEPROM is write-protected. If it is then unlock the EEPROM.
 
 uint8_t *pointer_data = (uint8_t *) (0x4000 + address);
 int16_t data_return = 0;
 data_return = (*pointer_data) << 8;
 data_return += *(pointer_data + 1);
 return data_return;
 

}
INTERRUPT void TIM4_UPD_OVF_IRQHandler(void)
{
	int16_t n = 0;
	timer_1ms++;
	switch(seg)
	{
		case 1:
		{
			n = (value / 100);
			DIG1 = 1;DIG2 = 0; DIG3 = 0; DP = 0;
			if (timer_toggle) DIG1 = 1;
				else DIG1 = 0;
			break;
		}					
		case 2:
		{
			n = ((value / 10) % 10);
			DIG1 = 0; DIG2 = 1; DIG3 = 0; DP = 1;
			if (timer_toggle) {DIG2 = 1; DP = 1;}
				else {DIG2 = 0; DP = 0;}
			break;
		}						
		case 3:
		{
			n = (value % 10);
			DIG1 = 0; DIG2 = 0; DIG3 = 1;  DP = 0;
			if (timer_toggle) DIG3 = 1;
				else DIG3 = 0;
			break;
		}
	}
			GPIOD->DDR  =	 	 led7_d_ddr[n];
			GPIOD->ODR  =    0b00000000 | led7_d[n] | (DIG1 << 6) | (((mode == 1) ? 1 : 0) << 1);
			GPIOA->ODR  =    0b00000000 | (DIG2 << 1) | (DIG3 << 2);	
			GPIOB->ODR  =    led7_b[n];
			GPIOC->ODR  =    led7_c[n] & ~(DP << 6);
				
	
	seg++;
	if(seg > 3)
	{
	seg = 1;
	}
	TIM4->SR1 &= ~(1 << 0);
}
void timer1_init(void){
	TIM1->CR1 = 0x00;
	TIM1->PSCRH = 0x3E; //3E
	TIM1->PSCRL = 0x7F; //7F
	TIM1->CR1 = 0x01;
}
void timer4_init(void) {
	// CK_PSC (internal fMASTER) is divided by the prescaler value.
	TIM4->PSCR = 7;
	TIM4->ARR = 131;
	// Enable update interrupt for timer 4
	TIM4->IER |= (1 << 0);
	// Clear timer interrupt flag
	TIM4->SR1 &= ~(1 << 0);
	// Precalculated value
	//TIM4_CNTR = 0xFF - 126;
	// Enable timer 4
	TIM4->CR1 |= (1 << 0);
	//Enable auto-reload
	TIM4->CR1 |= (1 << 7);

}
void main(void)
{

	CLK->CKDIVR = 0x00; // Set the frequency to 16 MHz
	// Configure timer
	// 1000 ticks per second
	timer4_init();
	timer1_init();
	enableInterrupts();

	GPIOA->DDR = 0b00000110;
	GPIOB->DDR = 0b11111111;
	GPIOC->DDR = 0b11111000;
	GPIOD->DDR = 0b01000010;
	
	GPIOA->CR1 = 0b00001110;
	GPIOB->CR1 = 0b11111111;
	GPIOC->CR1 = 0b11111000;
	GPIOD->CR1 = 0b01111110;
	
	GPIOA->CR2 = 0b00000110;
	GPIOB->CR2 = 0b11111111;
	GPIOC->CR2 = 0b11111000;
	GPIOD->CR2 = 0b01000110;
	while (delay_after_reset != 0)
  {
    delay(60000);
		delay_after_reset--;
  }
	reset_count = EEReadU16(2);
	reset_count++;
	EEWriteU16(2, reset_count);
	value_set = EEReadU16(0);
	value = value_set;
	timer_toggle = 1;
	while(1){
		timer1_value = TIM1->CNTRH<<8;
		timer1_value |= TIM1->CNTRL;
		if((timer1_value - timer_second > 300) && (mode == 2)){
			timer_second = timer1_value;
			if(timer_toggle == 1) {
				timer_toggle = 0;
			} else {
				timer_toggle = 1;
			}
		}
		if((timer_1ms > 95) && (mode == 1)){
			timer_1ms = 0;
			value--;
			if(value <= 0 ) {
				value = value_set; 
				mode = 0;	
				delay(60000);				
			}
		}
		if(((GPIOA->IDR >> 3) & 0x01) == 0x00){
			delay(30000);
			if(((GPIOA->IDR >> 3) & 0x01) == 0x00){
				if((mode == 0) && (mode_temp == 0)) {
					mode = 1;
					mode_temp = 1;
				}
			}
		} else if(((GPIOA->IDR >> 3) & 0x01) == 0x01){
			if((mode == 0) && (mode_temp == 1)) {
				mode_temp = 0;
			}
		}
		if((((GPIOD->IDR >> 3) & 0x01) == 0x00) && (mode != 1)){
			value = value_set;
			if(mode == 2) {mode = 0; timer_toggle = 1;}
			else if(mode == 0){
				mode = 2;
			}
			delay(10000);
			while(((GPIOD->IDR >> 3) & 0x01) == 0x00);
		}
		if(mode == 2){
			if(((GPIOD->IDR >> 4) & 0x01) == 0x00){
				value_set++;
				if(value_set > 999) value_set = 0;
				delay(20000);
				while(((GPIOD->IDR >> 4) & 0x01) == 0x00){
					value_button++;
					timer_toggle = 1;
					if(value_button > 60000) {
						value_button = 0;
						value_set++;
						if(value_set > 999) value_set = 0;
					}
					value = value_set;
				};
				EEWriteU16(0, value_set);
			}
			if(((GPIOD->IDR >> 5) & 0x01) == 0x00){
				value_set--;
				if(value_set < 0) value_set = 999;
				delay(20000);
				while(((GPIOD->IDR >> 5) & 0x01) == 0x00){
					value_button++;
					timer_toggle = 1;
					if(value_button > 60000) {
						value_button = 0;
						value_set--;
						if(value_set < 0) value_set = 999;
					}
					value = value_set;
				};
				EEWriteU16(0, value_set);
			}
		}
	}
}