#include <avr/io.h>
#include <util/delay.h>
#include <string.h>

void ADC_Init(void);
int  ADC_conversion();
int ADC_read_noise(void);