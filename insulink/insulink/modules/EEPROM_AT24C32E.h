#ifndef EEPROM_AT24C32E_H_
#define EEPROM_AT24C32E_H_

#include "../lib/I2C_0_AVR128DA64.h"

/************************************************************************/
/* AT24C32E_EEPROM                                                                     */
/************************************************************************/
#define AT24C32E_EEPROM 0x50

uint8_t AT24C32E_EEPROM_get_data(uint16_t address)
{
	char temp;
	//_delay_ms(5);
	TWI0.MSTATUS = 0x1;
	TWI0.MCTRLB |= (1 << 3);
	I2C_0_sendAddress(AT24C32E_EEPROM,0);
	
	temp = (address & 0xf00) >> 8;  // Storing 4 bits of MSB of ADDRESS HIGH, right  shifting 8 times
	I2C_0_sendData(temp);
	//I2C_0_sendData(0x00);
	
	temp = address & 0xff;
	I2C_0_sendData(temp);
	//I2C_0_sendData(0x01);
	
	//I2C_0_stop_transmission();
	I2C_0_sendAddress(AT24C32E_EEPROM,1);
	
	temp = I2C_0_recieveData();
	
	I2C_0_stop_transmission();
	_delay_ms(1);
	return temp;
}

uint16_t AT24C32E_EEPROM_get_data_16bits(uint16_t address)
{
	return ((AT24C32E_EEPROM_get_data(address + 1) << 8) | AT24C32E_EEPROM_get_data(address));
}

void AT24C32E_EEPROM_set_data(uint16_t address, uint8_t data)
{
	char temp;
	
	I2C_0_sendAddress(AT24C32E_EEPROM,0);
	
	temp = (address & 0xf00) >> 8;  // Storing 4 bits of MSB of ADDRESS HIGH, right  shifting 8 times
	I2C_0_sendData(temp);
	//I2C_0_sendData(0x00);
	
	temp = address & 0xff;
	I2C_0_sendData(temp);
	//I2C_0_sendData(0x01);
	
	I2C_0_sendData(data);
	_delay_ms(1);
	//_delay_us(100);
	//I2C_0_status();
	I2C_0_stop_transmission();
	
	_delay_ms(5);
}

void AT24C32E_EEPROM_set_data_16bits(uint16_t address, uint16_t data)
{
	AT24C32E_EEPROM_set_data(address, data);

	AT24C32E_EEPROM_set_data((address + 1), (data >> 8));
}


#endif