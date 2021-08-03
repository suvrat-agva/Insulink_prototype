#define F_CPU 4000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

/************************************************************************/
/* User libraries                                                                     */
/************************************************************************/
#include "lib/UART_1_AVR128DA64.h"
#include "modules/SystemUI.h"

/************************************************************************/
/* Function declarations.                                                                     */
/************************************************************************/
void system_init(void);
void keypad_init(void);


int main(void)
{
	system_init();
	
	//get_device_id();
	//_delay_ms(50);
    while (1) 
    {
		read_time();
		check_for_basal_delivery();
		bolus_option();
		setting();
		
		_delay_ms(5);
		sleep_cpu();
		
		
    }
}


/************************************************************************/
/* System initialization.                                                                     */
/************************************************************************/
void system_init(void)
{
	sei();
	
	PORTE.DIR |= 0xF;
	PORTE.OUT &= ~(0xF << 0); //Disable ENA/ENB (00000000000Brake Condition)
	
	USART1_init(9600);
	keypad_init();
	
	set_millis();
	Encoder_init();
	I2C_0_master_initialization();
	TCA_0_init();
	g_lock_millis = millis;
	
	SYSTEM_STATUS = SYSTEM_UNLOCK;
	BASAL_SYSTEM_STATUS = NO_PROCESS;
	
	// Setting sleep mode to standby.
	set_sleep_mode(SLEEP_MODE_STANDBY);
	sleep_enable();
	//Sleep enabled.
	
	set_device_id(1, 'C', 1, 2);   // Added device ID.
	
	USART1_sendString("System Initialized.");
	
// 	update_time();
// 	if ((!hour) && (!minute) && (!second) && (!year))
// 	{
// 		USART1_sendString("Set Time first..");
// 		set_date_time();
// 	}
}

/************************************************************************/
/* Keypad Section.                               
   Changed to ISR accordingly.                                      */
/************************************************************************/

void keypad_init(void)
{
	sei();
	
	BUTTON_OK_DEFINE;   // Pull up, Falling edge  //OK
	BUTTON_BOLUS_DEFINE;   // Pull up, Falling edge  //BOLUS
	
	BUTTON_BACK_DEFINE;   // Pull up, Falling edge  //B_LEFT
	BUTTON_SETTING_DEFINE;   // Pull up, Falling edge  //B_RIGHT
	
	BUTTON_DOWN_DEFINE;   // Pull up, Falling edge  //DOWN
	
	BUTTON_UP_DEFINE;   // Pull up, Falling edge  //UP
}

ISR(PORTB_PORT_vect)
{
	if (PORTB.INTFLAGS & (1 << 2))
	{
		PORTB.INTFLAGS |= (1 << 2);
		Button_OK = true;
		if ((SYSTEM_STATUS == SYSTEM_LOCK) && ((millis - g_lock_millis) < 5000))
		{
			SYSTEM_STATUS = SYSTEM_UNLOCK;
			
			BUTTON_OK_DEFINE;   //OK
			BUTTON_BOLUS_DEFINE;    //BOLUS
			BUTTON_BACK_DEFINE;   //B_LEFT
			BUTTON_SETTING_DEFINE;  //B_RIGHT
			BUTTON_DOWN_DEFINE;   //DOWN
			BUTTON_UP_DEFINE;    //UP
			
			USART1_sendString("System Unlock.");
		}
		//USART1_sendString("LOCK");
	}
	else if (PORTB.INTFLAGS & (1 << 6))
	{
		g_lock_millis = millis;
		PORTB.INTFLAGS |= (1 << 6);
		Button_BOLUS = true;
		//USART1_sendString("BOLUS");
	}
	//else if (PORTB.INTFLAGS & (1 << 1))
	//{
		//g_lock_millis = millis;
		//PORTB.INTFLAGS |= (1 << 1);
		//Button_DOWN = true;
		////USART1_sendString("DOWN");
	//}
}

ISR(PORTC_PORT_vect)
{
	g_lock_millis = millis;
	if (PORTC.INTFLAGS & (1 << 2))
	{
		PORTC.INTFLAGS |= (1 << 2);
		Button_BACK = true;
		//USART1_sendString("B_LEFT");
	}
	else if (PORTC.INTFLAGS & (1 << 6))
	{
		PORTC.INTFLAGS |= (1 << 6);
		if (SYSTEM_STATUS == SYSTEM_LOCK)
		{
			USART1_sendString("System Lock!! \nPress OK to unlock");
			Button_OK = false;
			BUTTON_OK_DEFINE;
		}
		else
		{
			Button_SETTING = true;
		}
		//USART1_sendString("B_RIGHT");
	}
}

// ISR(PORTE_PORT_vect)
// {
// 	if (PORTE.INTFLAGS & (1 << 6))
// 	{
// 		PORTE.INTFLAGS |= (1 << 6);
// 		USART1_sendString("DOWN");
// 	}
// }

ISR(PORTF_PORT_vect)
{
	g_lock_millis = millis;
	
	if (PORTF.INTFLAGS & (1 << 2))
	{
		PORTF.INTFLAGS |= (1 << 2);
		Button_UP = true;
		//USART1_sendString("UP");
	}
}

