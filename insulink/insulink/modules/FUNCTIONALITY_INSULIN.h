#ifndef FUNTIONALITY_INSULIN_H_
#define FUNTIONALITY_INSULIN_H_

#include <avr/io.h>
#include <util/delay.h>
#include <stdbool.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <stdlib.h>

#include "../lib/UART_1_AVR128DA64.h"
#include "EEPROM_AVR128DA64.h"
#include "MotorCalculation.h"
#include "RTC_AVR128DA64.h"
#include "RTC_MAX31342.h"
#include "SystemUI.h"
#include "write_eeprom.h"


/************************************************************************/
/* Prototypes.                                                                     */
/************************************************************************/
void check_bolus(void);

//extern bool Button_BOLUS = false, Button_LOCK = false, Button_BACK = false, Button_SETTING = false, Button_UP = false, Button_DOWN = false;

// FUNCTIONS -------------------------------------

void move_motor_single_rotation_with_sleep(void)
{
	PORTE.OUT |= (0x0A);
	
	PORTE.OUT |= (0xF << 0);
	_delay_ms(2);
	PORTE.OUT &= ~(1 << 0);
	_delay_ms(2);
	PORTE.OUT &= ~(1 << 2);
	_delay_ms(2);
	PORTE.OUT |= (1 << 0);
	_delay_ms(2);
	PORTE.OUTCLR |= 0x0f;
	GO_TO_SLEEP();
}

void delay(long ms)
{
	int i = 0;
	Button_BOLUS = false;
	for(i = 0; i < ms; i++)
	{
		if (Button_BOLUS)
		{
			Button_BOLUS = false;
			USART1_sendString("Wants to exit from basal? Press Bolus to exit or Back to continue");
			Button_BOLUS = false;
			Button_BACK = false;
			
			while((!Button_BACK) && (!Button_BOLUS))
			{
				_delay_ms(10);
			}
			if (Button_BOLUS)
			{
				BASAL_SYSTEM_STATUS = BASAL_DELIVERY_INTERRUPTED;
				return;
			}
		}
		_delay_ms(1);
	}
}


uint16_t StarttimeBasalNext, EndtimeBasalNext;
float configure_rate_next;

/************************************************************************/
/* Insulin Setup functions..                                                                     */
/************************************************************************/

/* basal_start_delivery is used to start the basal dose 
   as per the timings required by user.
   
   parameters:
   Starttime: In hours:mins
   Endtime: In hours:mins
   configure_rate : In units/hour.                                                                 */
/************************************************************************/
void basal_start_delivery(uint16_t Starttime, uint16_t Endtime, float configure_rate)
{
	long dose_steps_temp = 0;
	uint16_t dateMonth_basal = ((date * 100) + month);
	uint16_t Year_basal = 2000 + year;
	
	BASAL_SYSTEM_STATUS = BASAL_DELIVERY_START;
	
	//set_rtc_value(configure_insulin_basal(Starttime, Endtime, configure_rate));  // Setting up internal RTC for motor steps.
	long ms = configure_insulin_basal(Starttime, Endtime, configure_rate);
    
	//g_current_basal_time = Endtime;
	USART1_sendString_without_newline("Endtime: ");
	USART1_sendInt(Endtime);
	
	// Set up alarm for END time.
	//set_alarm1((int)(Endtime / 100), (int)(Endtime % 100));
// 	BASAL_SYSTEM_STATUS = BASAL_END_ALARM_SET;
// 	USART1_sendString("End ALARM set.");
	
	BASAL_SYSTEM_STATUS = BASAL_DELIVERY_ON_PROCESS;
	
	RTC_interrupt = false;
	
	while((dose_steps_temp < dose_steps) && (BASAL_SYSTEM_STATUS == BASAL_DELIVERY_ON_PROCESS) && ((((hour * 100) + minute) < Endtime))) 
	{
		dose_steps_temp++;
		USART1_sendInt(dose_steps_temp);
		read_time();
		
 		PORTE.OUT |= (0x0A);
 		
 		PORTE.OUT |= (0xF << 0);
 		_delay_ms(4);
 		PORTE.OUT &= ~(1 << 0);
 		_delay_ms(4);
 		PORTE.OUT &= ~(1 << 2);
 		_delay_ms(4);
 		PORTE.OUT |= (1 << 0);
 		_delay_ms(4);
 		PORTE.OUTCLR |= 0x0f;
		//while(!RTC_interrupt)
		//{
			//_delay_ms(1);
			//sleep_cpu();
		//}
		//RTC_interrupt = false;
		//GO_TO_SLEEP();
		delay(ms);
		//move_motor_single_rotation_with_sleep();
	}
	
	RTC.CTRLA &= ~(1 << 0);   // RTC disabled.
	
	//basal_set = false;
	
	if (BASAL_SYSTEM_STATUS != BASAL_DELIVERY_INTERRUPTED)
	{
		BASAL_SYSTEM_STATUS = NO_PROCESS;
		USART1_sendString("Delivery Done \nWriting to EEPROM..");
		date_write(dateMonth_basal, Year_basal, 0);
		basal_write(Starttime, Endtime, (configure_rate * 1000));
		USART1_sendString("Basal write to eeprom complete.");
	}
	set_next_basal_with_endtime(Endtime);
	_delay_ms(2000);
}


/************************************************************************/
/* basal_configure is used to to configure and start the basal dose 
   as per the timings required by user.
   
   parameters:
   Starttime: In hours:mins
   Endtime: In hours:mins
   configure_rate : In units/hour.                                                                 */
/************************************************************************/
void basal_configure(uint16_t Starttime, uint16_t Endtime, float configure_rate)
{
	if ((((Starttime / 100)) <= hour) && (((Starttime % 100) <= minute)))    // Checks for hour, minute
	{
		USART1_sendString("Process Start.");
		basal_start_delivery(Starttime, Endtime, configure_rate);	
	}
	else
	{
		StarttimeBasalNext = Starttime;
		EndtimeBasalNext = Endtime;
		configure_rate_next = configure_rate;
		
		g_current_basal_time = StarttimeBasalNext;
		USART1_sendString_without_newline("Set start time to: ");
		USART1_sendInt(g_current_basal_time);
		//set_alarm1((int)(Starttime / 100), (int)(Starttime % 100));  // Setup Alarm to start process.
		BASAL_SYSTEM_STATUS = BASAL_START_ALARM_SET;
		//USART1_sendString("Alarm set for basal.");
	}
}


void check_bolus(void)
{
	if (BASAL_SYSTEM_STATUS == BASAL_START_ALARM_SET)
	{
		USART1_sendString("Starttime:");
		USART1_sendInt(StarttimeBasalNext);
		USART1_sendString("Endtime:");
		USART1_sendInt(EndtimeBasalNext);
		USART1_sendString("Flowrate:");
		USART1_sendFloat(configure_rate_next, 2);
		
		basal_configure(StarttimeBasalNext, EndtimeBasalNext, configure_rate_next);
	}
	else if (BASAL_SYSTEM_STATUS == BASAL_DELIVERY_ON_PROCESS)
	{
		BASAL_SYSTEM_STATUS = BASAL_DELIVERY_STOP;
	}
}

void check_for_basal_delivery(void)
{
	if (g_start_basal)
	{
		USART1_sendInt((hour * 100) + minute);
		USART1_sendInt(g_current_basal_time);
		if (((hour * 100) + minute) >= g_current_basal_time)
		{
			USART1_sendString("Time matched.");
			check_bolus();
		}
	}
}

// --------------------------------------------------------------------------------
//           BOLUS SECTION

#define BOLUS_IMMEDIATE 0
#define BOLUS_5MIN      1
/************************************************************************/
/* bolus configure: Function used to configure bolus and inject according to user.
   Parameters:
              units: Units required by user.
			  type: 0 -> immediate, 1 -> 5 min              
			  bolus_type: 0 -> Manual bolus, 1 -> Smart bolus.                                                      */
/************************************************************************/
void bolus_configure(float units, bool type, bool bolus_type)
{
	int i = 0;
	long dose_steps_temp = 0;
	double steps_delay = 0.0;   // For bolus 5min
	uint16_t DateMonth_bolus = ((date * 100) + month);
	uint16_t Year_bolus = 2000 + year;
	uint16_t Time_bolus = ((hour * 100) + minute);
	
	if (bolus_type)
	{
		USART1_sendString("Smart bolus.");
	}
	else
	{
		USART1_sendString("Manual bolus.");
	}
	
	if (type == BOLUS_IMMEDIATE)
	{
		USART1_sendString("IMMEDIATE BOLUS");
		
		configure_insulin_bolus_immediate(units);
		
		USART1_sendString("BOLUS, Injecting insulin \nStarts in..");
		
		i = 3;
		while(i > 0)
		{
			USART1_sendInt(i--);
			_delay_ms(1000);
		}
		
		USART1_sendString("START");
		
		PORTE.OUT |= (0x0A);
		BOLUS_SYSTEM_STATUS = BOLUS_DELIVERY_ON_PROCESS;
		while(dose_steps_temp < dose_steps)
		{
			dose_steps_temp++;
			PORTE.OUT |= (0xF << 0);
			_delay_ms(4);
			PORTE.OUT &= ~(1 << 0);
			_delay_ms(4);
			PORTE.OUT &= ~(1 << 2);
			_delay_ms(4);
			PORTE.OUT |= (1 << 0);
			_delay_ms(4);
			
			PORTE.OUT &= ~(0xF << 0);
			_delay_ms(80);
		}
		
		PORTE.OUTCLR |= 0x0f;
		
		BOLUS_SYSTEM_STATUS = NO_PROCESS;
		USART1_sendString("Bolus injected. \nComplete.");
	}
	else
	{
		USART1_sendString("5MIN BOLUS");
		
		USART1_sendString("Dose units:");
		USART1_sendFloat(units, 2);
		
		USART1_sendString("Flowrate:");
		USART1_sendFloat((units / 0.08333), 2);
		
		steps_delay = configure_insulin_bolus_5min(units, (units / 0.08333));
		
		if (steps_delay < 16.0)
		{
			USART1_sendString("Delay smaller than 2ms. \nSetting to 2ms.");
			steps_delay = 16.0;
		}
		
		steps_delay = (steps_delay + 0.5) - 20.0;
		
		set_rtc_value(steps_delay);   // Setting RTC for wakeup.
		
		USART1_sendString("BOLUS, Injecting insulin \nStarts in..");
		
		i = 3;
		while(i > 0)
		{
			USART1_sendInt(i--);
			_delay_ms(1000);
		}
		
		USART1_sendString("START");
		PORTE.OUT |= (0x0A);
		BOLUS_SYSTEM_STATUS = BOLUS_DELIVERY_ON_PROCESS;
		RTC_interrupt = false;
		while(dose_steps_temp < dose_steps)
		{
			dose_steps_temp++;
			USART1_sendInt(dose_steps_temp);
			PORTE.OUT |= (0xF << 0);
			_delay_ms(4);
			PORTE.OUT &= ~(1 << 0);
			_delay_ms(4);
			PORTE.OUT &= ~(1 << 2);
			_delay_ms(4);
			PORTE.OUT |= (1 << 0);
			_delay_ms(4);
			PORTE.OUT &= ~(0x0f << 0);
			//delay(steps_delay);
			while(!RTC_interrupt)
			{
				_delay_ms(1);
				sleep_cpu();
			}
			RTC_interrupt = false;
			//_delay_ms(1);
			//_delay_ms(3);
			
		}
		
	}
	
	RTC.CTRLA &= ~(1 << 0);   // RTC disabled.
	
	BOLUS_SYSTEM_STATUS = NO_PROCESS;
	USART1_sendString("Bolus injected. \nComplete.");
	USART1_sendString("Writing bolus to eeprom..");
	date_write(DateMonth_bolus, Year_bolus, 0);
	if (bolus_type)
	{
		USART1_sendString("Smart bolus");
		smart_bolus_write(Time_bolus, units, type, CarbIntake, carbinsulin_ratio, Insulinsensitivity, lowerBGrange, higherBGrange, ActiveInsulin);
		USART1_sendString("Smart bolus written to EEPROM.");
	}
	else
	{
		USART1_sendString("Manual bolus");
		bolus_write(Time_bolus, units, type);
		USART1_sendString("Manual Bolus written to eeprom.");
	}
}


#endif