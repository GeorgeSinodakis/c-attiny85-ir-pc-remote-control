#include <avr/io.h>
#define F_CPU 16000000UL
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <stdbool.h>

//pin definitions
#define red_led 		4	//output
#define blue_led 		3	//output
#define pc_reset		2	//output
#define pc_poweron		1	//output
#define ir_receiver		0	//input

#define thr 			8

volatile uint8_t raw[60], in = 0;

ISR(PCINT0_vect)
{
	if (in == 0)
	{
		TCNT0 = 0; // TIMER STEP=16us
		in++;
	}
	else
	{
		raw[in] = TCNT0;
		TCNT0 = 0;
		in++;
	}
}

bool compareArrays(uint8_t *array1, uint8_t *array2, uint8_t l, uint8_t tol)
{
	for (uint8_t k = 0; k < l; k++)
	{
		if (array1[k] > array2[k])
		{
			if (array1[k] - array2[k] > tol) return 0;
		}
		else
		{
			if (array2[k] - array1[k] > tol) return 0;
		}
	}
	return 1;
}

bool findIn(uint8_t *large,uint8_t l_large, uint8_t *small, uint8_t l_small, uint8_t tol)
{
	//find if the small array exist in the large array with some tolerance
	for(uint8_t offset = 0; offset + l_small <= l_large; offset++)
	{
		if(compareArrays(large + offset, small, l_small, tol)) return 1;
	}
	return 0;
}

void powerOn(void)
{
	PORTB = 0;
	DDRB = 1 << blue_led | 1 << pc_poweron;
	_delay_ms(100);
	DDRB = 0;
}

void reset(void)
{
	PORTB = 0;
	DDRB |= 1 << red_led | 1 << pc_reset;
	_delay_ms(100);
	DDRB = 0;
}

int main(void)
{
	uint8_t blue1[39] = {167, 55, 28, 55, 28, 28, 28, 28, 83, 83, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 55, 28, 28, 28, 28, 55, 28, 28, 28, 28, 28, 28, 28};
	uint8_t blue2[41] = {167, 55, 28, 55, 28, 28, 28, 28, 28, 55, 55, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 55, 28, 28, 28, 28, 55, 28, 28, 28, 28, 28, 28, 28};
	uint8_t red1[35] = {167, 55, 28, 55, 28, 28, 28, 28, 83, 83, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 55, 28, 28, 55, 55, 28, 28, 55, 55};
	uint8_t red2[37] = {167, 55, 28, 55, 28, 28, 28, 28, 28, 55, 55, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 55, 28, 28, 55, 55, 28, 28, 55, 55};

	WDTCR = 1 << WDCE | 1 << WDE;	//setting watch dog timer
	WDTCR = 1 << WDP3 | 1 << WDP0;

	GIMSK = 1 << PCIE;
	PCMSK = 1 << PCINT0;	//pin change interrupt

	TCCR0B = 1 << CS02;	//timer0

	DDRB = 0;

	_delay_ms(1);

	sei();
	while (1)
	{
		if (TCNT0 > 230 && in > 20)
		{
			cli();

			if (findIn((uint8_t*) raw, 60, blue1, 39, thr) || findIn((uint8_t*) raw, 60, blue2, 41, thr))
			{
				powerOn();
			}
			else if (findIn((uint8_t*) raw, 60, red1, 35, thr) || findIn((uint8_t*) raw, 60, red2, 37, thr))
			{
				reset();
			}
			
			_delay_ms(100);

			in = 0;
			GIFR |= 1 << INTF0;	//disable any pending interrupt

			sei();
		}
		asm volatile ("wdr");
	}
}