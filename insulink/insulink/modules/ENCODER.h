#ifndef ENCODER_H_
#define ENCODER_H_

#include <avr/io.h>
#include "../lib/UART_1_AVR128DA64.h"
#include "RTC_AVR128DA64.h"

/************************************************************************/
/* Encoder                                                                     */
/************************************************************************/
long encoder_pos = 0;
long encoder_average = 0;
long encoder_average_last = 0;

#define MOTOR_FORWARD 2
#define MOTOR_REVERSE 1
#define MOTOR_STOP 0
uint8_t motor_direction = 0;   // 1 -> reverse, 2 -> forward, 3 -> Stop

bool encoder_average_complete = false;
bool check_for_direction = false;    // 0->reverse, 1 -> forward

int encoder_step = 0;  // Encoder temp used for averaging.

void disable_encoder(void)
{
	PORTE_PIN4CTRL &= ~(0x7 << 0);
	encoder_pos = 0;
	encoder_average = 0;
	encoder_average_last = 0;
}

void enable_encoder(void)
{
	PORTE_PIN4CTRL |= (0x2 << 0);
	encoder_pos = 0;
	encoder_average = 0;
	encoder_average_last = 0;
}

void Encoder_init(void)
{
	sei();
	PORTE_PIN4CTRL |= 0x2 ;   // Interrupt with Rising Edge
}

ISR(PORTE_PORT_vect)
{
	if (PORTE_INTFLAGS & (1 << 4))
	{
		PORTE_INTFLAGS |= (1 << 4);
		
		if (PORTE_IN & (1 << 5))
		{
			encoder_pos++;
		}
		else
		{
			encoder_pos--;
		}
		
		if (encoder_step < 10)
		{
			encoder_average += encoder_pos;
			encoder_step++;
		}
		else
		{
			encoder_step = 0;
			encoder_average = encoder_average / 10;
			if (encoder_average > encoder_average_last)
			{
				//USART1_sendChar('-');
				motor_direction = MOTOR_REVERSE;
			}
			else if (encoder_average < encoder_average_last)
			{
				//USART1_sendChar('+');
				motor_direction = MOTOR_FORWARD;
			}
			else
			{
				motor_direction = MOTOR_STOP;
			}
			
			encoder_average_last = encoder_average;
			encoder_average = 0;
			encoder_average_complete = true;
		}
	}
	if (PORTE.INTFLAGS & (1 << 6))
	{
		Button_DOWN = true;
		PORTE.INTFLAGS |= (1 << 6);
		//USART1_sendString("DOWN");
	}
}

int check_motor_status(void)
{
	static unsigned long last_time = 0;
	if (encoder_average_complete)
	{
		encoder_average_complete = false;
		last_time = millis;
		if (motor_direction == MOTOR_FORWARD)
		{
			//USART1_sendString("Motor in Forward direction.");
			return 1;
		}
		else
		{
			//USART1_sendString("Motor in Reverse direction.");
			return -1;
		}
		motor_direction = MOTOR_STOP;
	}
	else if ((millis - last_time) > 5000)
	{
		return 0;
		//USART1_sendString("Motor is stop, Time over.");
	}
	return 0;
}

#endif