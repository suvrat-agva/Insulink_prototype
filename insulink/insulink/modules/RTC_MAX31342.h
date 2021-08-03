#ifndef RTC_MAX31342_H_
#define RTC_MAX31342_H_

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include <string.h>
#include <stdlib.h>

#include "../lib/UART_1_AVR128DA64.h"
#include "../lib/I2C_0_AVR128DA64.h"

#include "STATUS.h"
#include "FUNCTIONALITY_INSULIN.h"

/************************************************************************/
/* RTC MAX31342 integration                                                                     */
/************************************************************************/

#define MAX_31342_ADDRESS 0x69

uint16_t length = 7;
uint8_t data_rtc[7];
bool alarm = false;

uint8_t hour, minute, second, dow, date, month, year;

uint8_t dec2bcd(uint8_t num)
{
	return ((num/10 * 16) + (num % 10));
}

// Convert Binary Coded Decimal (BCD) to Decimal
uint16_t bcd2dec(uint8_t num)
{
	return ((num/16 * 10) + (num % 16));
}


void set_RTC_address(uint8_t register_address, uint8_t register_value)
{
	I2C_0_sendAddress(MAX_31342_ADDRESS,0);
	
	I2C_0_sendData(register_address);
	I2C_0_sendData(register_value);
	
	I2C_0_stop_transmission();
}

/************************************************************************/
/* RTC Startup Process
Use the following procedure to enable RTC and set time.
1. Exit software reset and enable oscillator (SWRSTN = 1 and OSCONZ = 0) in register Config_reg1 (0x00).
2. Write RTC time to registers 0x06-0x0C.
3. Write SET_RTC = 1 in register Config_reg2 (0x01).
4. Wait 10ms.
5. Write SET_RTC = 0 in register Config_reg2 (0x01).                                                                     */
/************************************************************************/
void set_RTC_default_Max31342(void)
{
	//set_RTC_address(0x00, (0x00|(1 << 0)|(1 << 3)|(0x3 << 1)));
	//set_RTC_address(0x01, (0x00 | (1 << 3)|(1 << 1)));
	
	set_RTC_address(0x00, 0x07);
	set_RTC_address(0x01, 0x0C);
	set_RTC_address(0x03, 0x07);
	set_RTC_address(0x04, 0x00);
	
	
	set_RTC_address(0x06, dec2bcd(0));
	set_RTC_address(0x07, dec2bcd(1));
	set_RTC_address(0x08, dec2bcd(2));
	set_RTC_address(0x09, dec2bcd(4));
	set_RTC_address(0x0A, dec2bcd(5));
	set_RTC_address(0x0B, dec2bcd(6));
	set_RTC_address(0x0C, dec2bcd(7));
	
	set_RTC_address(0x17, 0x00);
	set_RTC_address(0x02, 0x58);
		
	set_RTC_address(0x01, (0x00|(1 << 1)));
	_delay_ms(10);
	set_RTC_address(0x01, (0x00|(1 << 3)|(1 << 2)));
}

void set_RTC_manually_Max31342(int seconds, int minutes, int hours, int dow, int date, int months, int years)
{
	set_RTC_address(0x00, 0x07);
	set_RTC_address(0x01, 0x0C);
	set_RTC_address(0x03, 0x07);
	set_RTC_address(0x04, 0x00);
	
	
	set_RTC_address(0x06, dec2bcd(seconds));
	set_RTC_address(0x07, dec2bcd(minutes));
	set_RTC_address(0x08, dec2bcd(hours));
	set_RTC_address(0x09, dec2bcd(dow));
	set_RTC_address(0x0A, dec2bcd(date));
	set_RTC_address(0x0B, dec2bcd(months));
	set_RTC_address(0x0C, dec2bcd(years));
	
	set_RTC_address(0x17, 0x00);
	set_RTC_address(0x02, 0x58);
	
	set_RTC_address(0x01, (0x00|(1 << 1)));
	_delay_ms(10);
	set_RTC_address(0x01, (0x00|(1 << 3)|(1 << 2)));
}

uint8_t get_RTC_address(uint8_t register_address)
{
	char data;
	I2C_0_stop_transmission();
	
	I2C_0_sendAddress(MAX_31342_ADDRESS, 0);
	I2C_0_sendData(register_address);
	I2C_0_stop_transmission();
	
	I2C_0_sendAddress(MAX_31342_ADDRESS, 1);
	data = I2C_0_recieveData();
	TWI0.MCTRLB |= (1 << 2);   // Send NACK
	I2C_0_stop_transmission();
	
	return data;
}

void get_RTC(void)
{
	I2C_0_sendAddress(MAX_31342_ADDRESS, 0);
	I2C_0_sendData(0x06);
	I2C_0_stop_transmission();
	_delay_ms(1);
	
	I2C_0_sendAddress(MAX_31342_ADDRESS, 1);
	
	int i = 0;
	
	while (i < (length - 1))
	{
		data_rtc[i] = I2C_0_recieveData();
		i++;
	}
	
	data_rtc[i] = I2C_0_recieveData();
	TWI0.MCTRLB |= (1 << 2);   // Send NACK
	//TWI0.MCTRLB |= (1 << 2);   // Send ACK
	I2C_0_stop_transmission();
	
	_delay_ms(1);
	//TWI0.MCTRLB &= ~(1 << 2);
}


/************************************************************************/
/* PD3 -> alarm1 interrupt, PD4 -> alarm2 interrupt                                                                     */
/************************************************************************/
void set_alarm1(int hours, int minutes)
{
	sei();
	PORTG_PIN2CTRL |= 0x3;    // set interrupt for alarm1
	//PORTD_PIN4CTRL |= 0x3;    // set interrupt for alarm2
	get_RTC_address(0x05);
	char config_reg1 = get_RTC_address(0x00);
	//char int_en_reg = get_RTC_address(0x04);
	
	set_RTC_address(0x00, (config_reg1 | (1 << 6)));    // set INTCN = 1, ECLK = 0..
	set_RTC_address(0x04, (0x00 | (1 << 0)));
	
	set_RTC_address(0x0D, dec2bcd(0));
	set_RTC_address(0x0E, dec2bcd(minutes));
	set_RTC_address(0x0F, dec2bcd(hours));
	
	set_RTC_address(0x10, 0x80);
	set_RTC_address(0x11, (0x00 | (1 << 6) | (1 << 7)));
}

ISR(PORTG_PORT_vect)
{
	if (PORTG_INTFLAGS & (1 << 2))
	{
		PORTG_INTFLAGS |= (1 << 2);
		
		USART1_sendString("Alarm1");
		//get_RTC_address(0x05);
		alarm = true;
		//check_bolus();
	}
	// 	else if (PORTD_INTFLAGS & (1 << 4))
	// 	{
	// 		USART1_sendString("Alarm2");
	// 		get_RTC_address(0x05);
	// 		PORTD_INTFLAGS |= (1 << 4);
	// 	}
}

/************************************************************************/
/* Update all time parameters.                                                                     */
/******************************************************* *****************/
void read_time(void)
{
	
	char Today_time[20] = "";
	
	char seconds[5], minutes[5], hours[5];
	
	intToStr(bcd2dec(get_RTC_address(0x08)),hours,0);
	_delay_ms(1);
	intToStr(bcd2dec(get_RTC_address(0x07)),minutes,0);
	_delay_ms(1);
	intToStr(bcd2dec(get_RTC_address(0x06)),seconds,0);
	_delay_ms(1);
	
	strcat(Today_time,hours);
	strcat(Today_time,":");
	strcat(Today_time,minutes);
	strcat(Today_time,":");
	strcat(Today_time,seconds);
	
	//USART1_sendString(Today_date);
	USART1_sendString(Today_time);
	//_delay_ms(1);
	//strcpy(Today_date,"");
	strcpy(Today_time,"");
	//_delay_ms(1000);
}

void update_time(void)
{
// 	hour = 4;
// 	minute = 30;
// 	second = 15;
// 	
// 	dow = 7;
// 	
// 	date = 12;
// 	month = 12;
// 	year = 1
    sleep_disable();
	cli();
	_delay_ms(2);
	
	hour = bcd2dec(get_RTC_address(0x08));
	minute = bcd2dec(get_RTC_address(0x07));
	second = bcd2dec(get_RTC_address(0x06));
	
	dow = bcd2dec(get_RTC_address(0x09));
	
	date = bcd2dec(get_RTC_address(0x0A));
	month = bcd2dec(get_RTC_address(0x0B));
	year = bcd2dec(get_RTC_address(0x0C));
	
	
	sleep_enable();
	sei();
	_delay_ms(2);
	//read_time();
}
#endif