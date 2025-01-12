#include "libADC.hpp"

void ADC_Init(void){
	// ADCSRA – ADC Control and Status Register A
	ADCSRA = (1<<ADEN)|(1<<ADPS0)|(1<<ADPS1)|(1<<ADPS2);
	//	ADEN: bit 7 - enable ADC
	//	ADPS0,1,2: bit 0,1,2 - prescaler (128 division factor)

	// ADMUX - ADC Multiplexer Selection Register
	ADMUX  =  (1<<REFS0);
	//REFS0: bit 6 - Reference Selection Bits ->  Internal 1.1V Voltage Reference with external capacitor at AREF pin
}

int  ADC_conversion(void){
	ADMUX &= (~(1<<MUX2) & ~(1<<MUX1)) ;
	ADCSRA |= (1<<ADSC);
	// ADSC: bit 6 - ADC - Start Conversion
	while(ADCSRA & (1<<ADSC)); 	//	when the conversion ends ADSC will turn 0

	return ADC;
}

int ADC_read_noise(void){
	ADMUX |= (1<<MUX2) | (1<<MUX1);
	ADCSRA |= (1<<ADSC);

	while(ADCSRA & (1<<ADSC));

	return ADC;

}
