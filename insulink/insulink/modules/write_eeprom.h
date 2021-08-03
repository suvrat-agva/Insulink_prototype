#ifndef WRITE_EEPROM_H_
#define WRITE_EEPROM_H_

#include "EEPROM_AT24C32E.h"
#include "EEPROM_AVR128DA64.h"
#include "DataFrame.h"

#include "../lib/UART_1_AVR128DA64.h"




/************************************************************************/
/* Update StartAddress with clearing 5 enteries                                                                 */
/************************************************************************/
void clear_5_entries(bool PacketType)
{
	int i = 0;
	int StartAddress = 0, EndAddress = 0, MaxAddress = 0, MinAddress;
	if (PacketType)
	{
		StartAddress = read_data_in_eeprom_SPM(GLUCOSE_START_ADDRESS_INDEX);
		EndAddress = read_data_in_eeprom_SPM(GLUCOSE_END_ADDRESS_INDEX);
		MaxAddress = GLUCOSE_ADDRESS_MAX;
		MinAddress = GLUCOSE_ADDRESS_MIN;
	}
	else
	{
		StartAddress = read_data_in_eeprom_SPM(EEPROM_START_ADDRESS_INDEX);
		EndAddress = read_data_in_eeprom_SPM(EEPROM_END_ADDRESS_INDEX);
		MaxAddress = BOLUS_ADDRESS_MAX;
		MinAddress = BOLUS_ADDRESS_MIN;
	}
	
	int AddressTemp = StartAddress;
	i = 6;
	if (StartAddress < EndAddress)    // Currently arranged in linear.
	{
		while((AddressTemp < EndAddress) && (i))
		{
			// Send enteries.
			if (check_date_start(AddressTemp))
			{
				AddressTemp += 4;
				--i;
			}
			else
			{
				switch(AT24C32E_EEPROM_get_data(AddressTemp))
				{
					case 'B':{
						AddressTemp += 5;
						break;
					}
					case 'Z':{
						AddressTemp += 7;
						break;
					}
					case 'W':{
						AddressTemp += 11;
						break;
					}
					case  'G':{
						AddressTemp += 6;
						break;
					}
					default:{
						AddressTemp++;
						break;
					}
				}
			}
		}
		
		AddressTemp = AddressTemp - 4;
		USART1_sendString("StartAddress:");
		USART1_sendInt(StartAddress);
		USART1_sendString("Till location to erased");
		USART1_sendInt(AddressTemp);
		
		USART1_sendString("Clearing enteries..");
		// Clearing Enteries..
		while(StartAddress < AddressTemp)
		{
			AT24C32E_EEPROM_set_data(StartAddress, 0xff);
			StartAddress++;
		}
		
		USART1_sendString("Enteries cleared.");
	}
	else                      // Enteries are circular now.
	{
		while((AddressTemp <=  MaxAddress) && (i))
		{
			// Fetch data to Maximum.
			if (check_date_start(AddressTemp))
			{
				AddressTemp += 4;
				--i;
			}
			else
			{
				switch(AT24C32E_EEPROM_get_data(AddressTemp))
				{
					case 'B':{
						AddressTemp += 5;
						break;
					}
					case 'Z':{
						AddressTemp += 7;
						break;
					}
					case 'W':{
						AddressTemp += 11;
						break;
					}
					case  'G':{
						AddressTemp += 6;
						break;
					}
					default:{
						AddressTemp++;
						break;
					}
				}
			}
		}
		
		// Clearing one side entry.
		USART1_sendString("Enteries in Circular. \nTotal enteries found till max:");
		USART1_sendInt(i);
		AddressTemp = AddressTemp - 4;
		USART1_sendString("StartAddress:");
		USART1_sendInt(StartAddress);
		USART1_sendString("Till location to erased");
		USART1_sendInt(AddressTemp);
		
		USART1_sendString("Clearing enteries..");
		// Clearing Enteries..
		while(StartAddress < AddressTemp)
		{
			AT24C32E_EEPROM_set_data(StartAddress, 0xff);
			StartAddress++;
		}
		
		USART1_sendString("Enteries cleared.");
		
		if (i != 0)    // Check for enteries left or not.
		{
			AddressTemp = MinAddress;   // Initialize to bolus min value
			while((AddressTemp <= EndAddress) && i)
			{
				// Fetch Data to EndAddress
				if (check_date_start(AddressTemp))
				{
					AddressTemp += 4;
					--i;
				}
				else
				{
					switch(AT24C32E_EEPROM_get_data(AddressTemp))
					{
						case 'B':{
							AddressTemp += 5;
							break;
						}
						case 'Z':{
							AddressTemp += 7;
							break;
						}
						case 'W':{
							AddressTemp += 11;
							break;
						}
						case  'G':{
							AddressTemp += 6;
							break;
						}
						default:{
							AddressTemp++;
							break;
						}
					}
				}
			}
			
			USART1_sendString("Clearing remainig enteries..");
			StartAddress = 0;
			AddressTemp = AddressTemp - 4;
			while(StartAddress < AddressTemp)
			{
				AT24C32E_EEPROM_set_data(StartAddress, 0xff);
				StartAddress++;
			}
			
			USART1_sendString("All enteries cleared.");
		}
	}
	
	//AddressTemp = AddressTemp - 4;
	USART1_sendString("Setting Starting Address to:");
	USART1_sendInt(AddressTemp);
	
	USART1_sendString("Start address set to this date and month:");
	USART1_sendInt(AT24C32E_EEPROM_get_data_16bits(AddressTemp));
	
	if (PacketType)
	{
		write_data_in_eeprom_SPM(GLUCOSE_START_ADDRESS_INDEX, AddressTemp);
	}
	else
	{
		write_data_in_eeprom_SPM(EEPROM_START_ADDRESS_INDEX, AddressTemp);
	}
	USART1_sendString("Address set.");
}

/************************************************************************/
/* Update EndAddress.                                                                     */
/************************************************************************/
void update_eeprom_last_addr(int length, bool PacketType)
{
	int StartAddress = 0, EndAddress = 0, MaxAddress = 0, MinAddress = 0;
	if (PacketType)
	{
		StartAddress = read_data_in_eeprom_SPM(GLUCOSE_START_ADDRESS_INDEX);
		EndAddress = read_data_in_eeprom_SPM(GLUCOSE_END_ADDRESS_INDEX);
		MaxAddress = GLUCOSE_ADDRESS_MAX;
		MinAddress = GLUCOSE_ADDRESS_MIN;
	}
	else
	{
		StartAddress = read_data_in_eeprom_SPM(EEPROM_START_ADDRESS_INDEX);
		EndAddress = read_data_in_eeprom_SPM(EEPROM_END_ADDRESS_INDEX);
		MaxAddress = BOLUS_ADDRESS_MAX;
		MinAddress = BOLUS_ADDRESS_MIN;
	}
	
	USART1_sendString_without_newline("Start Address: ");
	USART1_sendInt(StartAddress);
	USART1_sendString_without_newline("End Address: ");
	USART1_sendInt(EndAddress);
	
	
	//  Checking address to set.
	if (StartAddress < EndAddress)
	{
		if (length < (MaxAddress - EndAddress))
		{
			USART1_sendString("Space available.");
			return;
		}
		else
		{
			clear_5_entries(PacketType);
			USART1_sendString("Setting End Address to min");
			if (PacketType)
			{
				write_data_in_eeprom_SPM(GLUCOSE_END_ADDRESS_INDEX, GLUCOSE_ADDRESS_MIN);
			}
			else
			{
				write_data_in_eeprom_SPM(EEPROM_END_ADDRESS_INDEX, BOLUS_ADDRESS_MIN);
			}
		}
	}
	else
	{
		USART1_sendString("--------------Circular---------------------------");
		if (length < ((StartAddress - EndAddress)))
		{
			USART1_sendString("Space available.");
			return;
		}
		else
		{
			clear_5_entries(PacketType);
		}
	}
}


/***************************************************************************************************************************
function to write date
args: ddmm : 17th july as '1707'
	  year : 2021 as '2021'
	  type: 0 for basal bolus date 1 for glucometer date

*************************************************************************************************************************/
void date_write (uint16_t ddmm,uint16_t year, int typ){
	uint16_t data_address =0;
	if ((typ == 0 ))
	{
	// WRITING BOLUS AND BASAL DATA
       
	   data_address = read_data_in_eeprom_SPM(EEPROM_CURRENT_ADDRESS_INDEX);
	   if(data_address == 0xffff)
	   {
		data_address = EEPROM_B_START;
		//USART1_sendString("No previously loaded address found");
		AT24C32E_EEPROM_set_data_16bits(data_address,ddmm);
		data_address += 2;
		_delay_ms(1);
		AT24C32E_EEPROM_set_data_16bits(data_address,year);
		data_address += 2;
	   }
	   else
	   {
		USART1_sendString("Previously loaded address found");
		
		update_eeprom_last_addr(4, 0);   // Check for 4 bytes for date.
		data_address = read_data_in_eeprom_SPM(EEPROM_CURRENT_ADDRESS_INDEX);
		
		uint16_t current_date = read_data_in_eeprom_SPM(EEPROM_DATE_ADDRESS_INDEX);
		if (current_date == ddmm){
			USART1_sendString("Date entry for the same date is already done");
		}
		else
		{
			
			USART1_sendString("Logging Date entry for the day");
			AT24C32E_EEPROM_set_data_16bits(data_address,ddmm);
			data_address += 2;
			_delay_ms(1);
			AT24C32E_EEPROM_set_data_16bits(data_address,year);
			data_address += 2;
		}
	  }
	write_data_in_eeprom_SPM(EEPROM_CURRENT_ADDRESS_INDEX,data_address);
	_delay_ms(1);
	write_data_in_eeprom_SPM(EEPROM_DATE_ADDRESS_INDEX, ddmm);
	USART1_sendInt(data_address);
	USART1_sendString("Done logging data to eeprom");
	USART1_sendChar('\n');
	//data_address =0;
}
else if((typ == 1))
      {
		
		data_address = read_data_in_eeprom_SPM(EEPROM_GLUCO_CURRENT_DATE_INDEX);
		USART1_sendString("Entering date for glucose");
		USART1_sendInt(data_address);
		if(data_address == 0xffff)
		{
			data_address = EEPROM_G_START;
			USART1_sendString("No previously loaded address found");
			AT24C32E_EEPROM_set_data_16bits(data_address,ddmm);
			data_address += 2;
			_delay_ms(1);
			AT24C32E_EEPROM_set_data_16bits(data_address,year);
			data_address += 2;
		}else
		{
			USART1_sendString("Previously loaded address found");
			
			update_eeprom_last_addr(4, 1);   // Check for 4 bytes for date.
			data_address = read_data_in_eeprom_SPM(EEPROM_CURRENT_ADDRESS_INDEX);
			
			uint16_t current_date = read_data_in_eeprom_SPM(EEPROM_GLUCO_DATE_ADDR_INDEX);
			if (current_date == ddmm)
		{
				USART1_sendString("Date entry for the same date is already done");
		}else
		{
				
				USART1_sendString("Logging Date entry for the day");
				AT24C32E_EEPROM_set_data_16bits(data_address,ddmm);
				data_address += 2;
				_delay_ms(1);
				AT24C32E_EEPROM_set_data_16bits(data_address,year);
				data_address += 2;
		}
		}
		write_data_in_eeprom_SPM(EEPROM_GLUCO_CURRENT_DATE_INDEX,data_address);
		_delay_ms(1);
		write_data_in_eeprom_SPM(EEPROM_GLUCO_DATE_ADDR_INDEX, ddmm);
		USART1_sendInt(data_address);
		USART1_sendString("Done logging data to eeprom");
		USART1_sendChar('\n');
		//data_address =0;
	
	    }
	_delay_ms(1);
}


void bolus_write(uint16_t hhmm, uint8_t units, uint8_t type){
	uint16_t data_address =0;
	 data_address = read_data_in_eeprom_SPM(EEPROM_CURRENT_ADDRESS_INDEX);
	
	update_eeprom_last_addr(5, 0);   // Check for 4 bytes for Bolus.
	data_address = read_data_in_eeprom_SPM(EEPROM_CURRENT_ADDRESS_INDEX);
	
		USART1_sendString("logging MANUAL BOLUS DATA");
		
		AT24C32E_EEPROM_set_data(data_address,'B');
		data_address++;
		AT24C32E_EEPROM_set_data_16bits(data_address,hhmm);
		data_address+=2;
		AT24C32E_EEPROM_set_data(data_address,units);
		data_address++;
		AT24C32E_EEPROM_set_data(data_address,type);
		data_address++;
		
		_delay_ms(1);
		write_data_in_eeprom_SPM(EEPROM_CURRENT_ADDRESS_INDEX,data_address);
		USART1_sendString("Done logging data to internal eeprom: bolus addr");
		USART1_sendInt(data_address);
		USART1_sendChar('\n');
	//	data_address =0;
	
}

void basal_write(uint16_t start_time, uint16_t end_time, uint16_t rate){
	uint16_t data_address =0;
	 data_address = read_data_in_eeprom_SPM(EEPROM_CURRENT_ADDRESS_INDEX);
	 
	 update_eeprom_last_addr(7, 0);   // Check for 4 bytes for Basal.
	 data_address = read_data_in_eeprom_SPM(EEPROM_CURRENT_ADDRESS_INDEX);
	 
	AT24C32E_EEPROM_set_data(data_address,'Z');
	data_address++;
	AT24C32E_EEPROM_set_data_16bits(data_address, start_time);
	data_address += 2;
	AT24C32E_EEPROM_set_data_16bits(data_address, end_time);
	data_address += 2;
	AT24C32E_EEPROM_set_data_16bits(data_address, rate);
	data_address += 2;
	_delay_ms(1);
	write_data_in_eeprom_SPM(EEPROM_CURRENT_ADDRESS_INDEX, data_address);
	USART1_sendString("logged Basal data add. in EEPROM internal");
	USART1_sendInt(data_address);
	USART1_sendChar('\n');
	//data_address =0;
}

void smart_bolus_write(uint16_t time,uint8_t units,uint8_t type,uint8_t carb_intk, uint8_t carb_by_insulin,uint8_t I_sens,uint8_t low_bg,uint8_t high_bg,uint8_t act_insulin){
	uint16_t data_address = 0;
	 data_address = read_data_in_eeprom_SPM(EEPROM_CURRENT_ADDRESS_INDEX);
	 
	 update_eeprom_last_addr(11, 0);   // Check for 4 bytes for smart bolus.
	 data_address = read_data_in_eeprom_SPM(EEPROM_CURRENT_ADDRESS_INDEX);
	 
	AT24C32E_EEPROM_set_data(data_address,'W');
	data_address++;
	AT24C32E_EEPROM_set_data_16bits(data_address, time);
	data_address += 2;
	AT24C32E_EEPROM_set_data(data_address,units);
	data_address++;
	AT24C32E_EEPROM_set_data(data_address,type);
	data_address++;
	AT24C32E_EEPROM_set_data(data_address, carb_intk);
	data_address++;
	AT24C32E_EEPROM_set_data(data_address, carb_by_insulin);
	data_address++;
	AT24C32E_EEPROM_set_data(data_address,I_sens);
	data_address++;
	AT24C32E_EEPROM_set_data(data_address,low_bg);
	data_address++;
	AT24C32E_EEPROM_set_data(data_address, high_bg);
	data_address++;
	AT24C32E_EEPROM_set_data(data_address, act_insulin);
	data_address++;
	_delay_ms(1);
	write_data_in_eeprom_SPM(EEPROM_CURRENT_ADDRESS_INDEX,data_address);
	USART1_sendString("Address updated in internal EEPROM after smart bolus");
	USART1_sendInt(data_address);
	USART1_sendChar('\n');
	//data_address =0;
}

void gluco_data_write(uint16_t read_time, uint16_t reading, uint8_t type){
	uint16_t data_address = 0;
    data_address = read_data_in_eeprom_SPM(EEPROM_GLUCO_CURRENT_DATE_INDEX);
	
	update_eeprom_last_addr(6, 0);   // Check for 4 bytes for date.
	data_address = read_data_in_eeprom_SPM(EEPROM_GLUCO_CURRENT_DATE_INDEX);
	
	AT24C32E_EEPROM_set_data(data_address,'G');
	data_address++;
	AT24C32E_EEPROM_set_data_16bits(data_address, read_time);
	data_address += 2;
	AT24C32E_EEPROM_set_data_16bits(data_address,reading);
	data_address+= 2;
	AT24C32E_EEPROM_set_data(data_address,type);
	data_address++;
	_delay_ms(1);
	write_data_in_eeprom_SPM(EEPROM_GLUCO_CURRENT_DATE_INDEX,data_address);
	USART1_sendChar('\n');
	//data_address =0;
	USART1_sendString("Gluco data sent");
	USART1_sendInt(data_address);
}

/************************************************************************/
/* Setting and retrieving Device ID.                                                                     */
/************************************************************************/
void set_device_id(char year, char month, char majorID, char minorID)
{
	write_data_in_eeprom_SPM_8bits(0, year);
	write_data_in_eeprom_SPM_8bits(1, month);
	write_data_in_eeprom_SPM_8bits(2, majorID);
	write_data_in_eeprom_SPM(3, minorID);
}

void get_device_id(void)
{
	char deviceid[15] = "";
	
	intToStr(read_data_in_eeprom_SPM_8bits(0), buff, 0);
	strcat(deviceid, buff);
    
	deviceid[1] = read_data_in_eeprom_SPM_8bits(1);
	deviceid[2] = '\0';
	intToStr(read_data_in_eeprom_SPM_8bits(2), buff, 3);
	strcat(deviceid, buff);
	intToStr(read_data_in_eeprom_SPM(3), buff, 5);
	strcat(deviceid, buff);
	
	USART1_sendString_without_newline(deviceid);
}
#endif