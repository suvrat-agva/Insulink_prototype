#ifndef DATAFRAME_H_
#define DATAFRAME_H_

#include "EEPROM_AT24C32E.h"
#include "EEPROM_AVR128DA64.h"

#include "../lib/UART_1_AVR128DA64.h"

void usb_sync(void);
void read_all_packets(bool SendGlucosePacket);

void clear_external_eeprom(void)
{
	int i = 0;
	for (i = 0; i < 4096; i++)
	{
		AT24C32E_EEPROM_set_data(i, 0xff);
		//_delay_ms(1);
	}
}
bool check_date_start(int addr)
{
	int date = AT24C32E_EEPROM_get_data_16bits(addr) / 100;
	if ((date < 32) && (date > 0))
	{
		return true;
	}
	return false;
}

int read_bolus_eeprom(int eeprom_addr)
{
	char temp[15] = "\0";
	
	USART1_sendChar('B');  // Printing BOLUS identifier.
	
	eeprom_addr++;         // Increment address for data.
	
	intToStr(AT24C32E_EEPROM_get_data_16bits(eeprom_addr), buff, 4);  // time
	strcat(temp, buff);
	intToStr(AT24C32E_EEPROM_get_data(eeprom_addr + 2), buff, 2);   // unit
	strcat(temp, buff);
	intToStr(AT24C32E_EEPROM_get_data(eeprom_addr + 3), buff, 1);   // type
	strcat(temp, buff);
	
	eeprom_addr += 4;
	
	USART1_sendString_without_newline(temp);
	return(eeprom_addr);
}

int read_basal_eeprom(int eeprom_addr)
{
	char temp[15] = "\0";
	
	USART1_sendChar('Z');  // Printing BASAL identifier.
	
	eeprom_addr++;         // Increment address for data.
	
	intToStr(AT24C32E_EEPROM_get_data_16bits(eeprom_addr), buff, 4);   // Start time
	strcat(temp, buff);
	intToStr(AT24C32E_EEPROM_get_data_16bits(eeprom_addr + 2), buff, 4);  // End time
	strcat(temp, buff);
	intToStr(AT24C32E_EEPROM_get_data_16bits(eeprom_addr + 4), buff, 5);    // Flowrate
	strcat(temp, buff);
	
	eeprom_addr += 6;
	
	USART1_sendString_without_newline(temp);
	return(eeprom_addr);
}

int read_glucose_eeprom(int eeprom_addr)
{
	char temp[15] = "\0";
	
	USART1_sendChar('G');  // Printing GLUCOSE identifier.
	
	eeprom_addr++;         // Increment address for data.
	
	intToStr(AT24C32E_EEPROM_get_data_16bits(eeprom_addr), buff, 4);   // Start time
	strcat(temp, buff);
	intToStr(AT24C32E_EEPROM_get_data_16bits(eeprom_addr + 2), buff, 3);  // Glucose value
	strcat(temp, buff);
	intToStr(AT24C32E_EEPROM_get_data(eeprom_addr + 4), buff, 1);    // type
	strcat(temp, buff);
	
	eeprom_addr += 5;
	
	USART1_sendString_without_newline(temp);
	return eeprom_addr;
}

int read_smart_bolus_eeprom(int eeprom_addr)
{
	char temp[30] = "\0";
	
	USART1_sendChar('W');  // Printing GLUCOSE identifier.
	
	eeprom_addr++;         // Increment address for data.
	
	intToStr(AT24C32E_EEPROM_get_data_16bits(eeprom_addr), buff, 4);   // Start time
	strcat(temp, buff);
	intToStr(AT24C32E_EEPROM_get_data(eeprom_addr + 2), buff, 2);  // Units
	strcat(temp, buff);
	intToStr(AT24C32E_EEPROM_get_data(eeprom_addr + 3), buff, 1);    // type
	strcat(temp, buff);
	intToStr(AT24C32E_EEPROM_get_data(eeprom_addr + 4), buff, 3);    // Carb intake
	strcat(temp, buff);
	intToStr(AT24C32E_EEPROM_get_data(eeprom_addr + 5), buff, 3);    // C/I ratio
	strcat(temp, buff);
	intToStr(AT24C32E_EEPROM_get_data(eeprom_addr + 6), buff, 3);    // Insulin sensitivity
	strcat(temp, buff);
	intToStr(AT24C32E_EEPROM_get_data(eeprom_addr + 7), buff, 3);    // Lower BG range
	strcat(temp, buff);
	intToStr(AT24C32E_EEPROM_get_data(eeprom_addr + 8), buff, 3);    // Higher BG range
	strcat(temp, buff);
	intToStr(AT24C32E_EEPROM_get_data(eeprom_addr + 9), buff, 3);    // Active insulin
	strcat(temp, buff);
	
	eeprom_addr += 10;
	
	USART1_sendString_without_newline(temp);
	return eeprom_addr;
}

/************************************************************************/
/* argument: SendGlucosePacket -> 0: Sending Bolus packet, 1: Sending Glucose packet                                                                     */
/************************************************************************/
void read_all_packets(bool SendGlucosePacket)
{
	int DateMonth = 0, Year = 0, StartAddress = 0, EndAddress = 0, MaxAddress = 0, MinAddress = 0;
	if (SendGlucosePacket)
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
	
	//USART1_sendString("Start address");
	//USART1_sendInt(StartAddress);
	//USART1_sendString("End Address");
	//USART1_sendInt(EndAddress);
	
	int AddressTemp = StartAddress;
	if (StartAddress < EndAddress)    // Currently arranged in linear.
	{
		while(AddressTemp < EndAddress)
		{
			// Send enteries.
			if (check_date_start(AddressTemp))
			{
				DateMonth = AT24C32E_EEPROM_get_data_16bits(AddressTemp);
				Year = AT24C32E_EEPROM_get_data_16bits(AddressTemp + 2);
				
				USART1_sendChar('#');
				USART1_sendChar('?');
				intToStr(DateMonth, buff, 4);
				USART1_sendString_without_newline(buff);
				intToStr(Year, buff, 4);
				USART1_sendString_without_newline(buff);
				//USART1_sendChar('\n');
				AddressTemp += 4;
			}
			else
			{
				switch(AT24C32E_EEPROM_get_data(AddressTemp))
				{
					case 'B':{
						//USART1_sendString("Bolus Packet.");
						USART1_sendChar(',');
						AddressTemp = read_bolus_eeprom(AddressTemp);
						//AddressTemp += 5;
						break;
					}
					case 'Z':{
						USART1_sendChar(',');
						AddressTemp = read_basal_eeprom(AddressTemp);
						//USART1_sendString("Basal Packet.");
						//AddressTemp += 7;
						break;
					}
					case 'W':{
						USART1_sendChar(',');
						AddressTemp = read_smart_bolus_eeprom(AddressTemp);
						//USART1_sendString("Smart bolus packet");
						//AddressTemp += 11;
						break;
					}
					case 'G':{
						USART1_sendChar(',');
						AddressTemp = read_glucose_eeprom(AddressTemp);
						break;
					}
					default: {
						AddressTemp++;
						break;
					}
				}
			}
		}
	}
	else                      // Enteries are circular now.
	{
		while(AddressTemp <=  MaxAddress)
		{
			// Fetch data to Maximum.
			if (check_date_start(AddressTemp))
			{
				DateMonth = AT24C32E_EEPROM_get_data_16bits(AddressTemp);
				Year = AT24C32E_EEPROM_get_data_16bits(AddressTemp + 2);
				
				USART1_sendChar('#');
				USART1_sendChar('?');
				intToStr(DateMonth, buff, 4);
				USART1_sendString_without_newline(buff);
				intToStr(Year, buff, 4);
				USART1_sendString_without_newline(buff);
				
				AddressTemp += 4;
			}
			else
			{
				switch(AT24C32E_EEPROM_get_data(AddressTemp))
				{
					case 'B':{
						//USART1_sendString("Bolus Packet.");
						USART1_sendChar(',');
						AddressTemp = read_bolus_eeprom(AddressTemp);
						//AddressTemp += 5;
						break;
					}
					case 'Z':{
						USART1_sendChar(',');
						AddressTemp = read_basal_eeprom(AddressTemp);
						//USART1_sendString("Basal Packet.");
						//AddressTemp += 7;
						break;
					}
					case 'W':{
						USART1_sendChar(',');
						AddressTemp = read_smart_bolus_eeprom(AddressTemp);
						//USART1_sendString("Smart bolus packet");
						//AddressTemp += 11;
						break;
					}
					case 'G':{
						USART1_sendChar(',');
						AddressTemp = read_glucose_eeprom(AddressTemp);
						break;
					}
					default: {
						AddressTemp++;
						break;
					}
				}
			}
		}
		AddressTemp = MinAddress;   // Initialize to bolus min value
		while(AddressTemp <= EndAddress)
		{
			// Fetch Data to EndAddress
			if (check_date_start(AddressTemp))
			{
				DateMonth = AT24C32E_EEPROM_get_data_16bits(AddressTemp);
				Year = AT24C32E_EEPROM_get_data_16bits(AddressTemp + 2);
				
				USART1_sendChar('#');
				USART1_sendChar('?');
				intToStr(DateMonth, buff, 4);
				USART1_sendString_without_newline(buff);
				intToStr(Year, buff, 4);
				USART1_sendString_without_newline(buff);
				
				AddressTemp += 4;
			}
			else
			{
				switch(AT24C32E_EEPROM_get_data(AddressTemp))
				{
					case 'B':{
						USART1_sendChar(',');
						//USART1_sendString("Bolus Packet.");
						AddressTemp = read_bolus_eeprom(AddressTemp);
						//AddressTemp += 5;
						break;
					}
					case 'Z':{
						USART1_sendChar(',');
						AddressTemp = read_basal_eeprom(AddressTemp);
						//USART1_sendString("Basal Packet.");
						//AddressTemp += 7;
						break;
					}
					case 'W':{
						USART1_sendChar(',');
						AddressTemp = read_smart_bolus_eeprom(AddressTemp);
						//USART1_sendString("Smart bolus packet");
						//AddressTemp += 11;
						break;
					}
					case 'G':{
						USART1_sendChar(',');
						AddressTemp = read_glucose_eeprom(AddressTemp);
						break;
					}
					default: {
						AddressTemp++;
						break;
					}
				}
			}
		}
	}
}

/************************************************************************/
/* Insulink syncing with software.                                                                     */
/******************************************************************************************************************************
Function to get the Acknowlwdgement after a communication
ACK01: Secure connection established will return 1 on success
ACK02: Bolus and basal data sent successfully will return 2 on success
ACK03: Glucometer data sent successfully will return 3 successfully
ACK04: mobility details shared successfully will return 4 successfully
otherwise invalid will return 0
also if more than 3 seconds taken communication will be timed out will return 5
*******************************************************************************************************************************/

uint8_t get_ack(void)
{
	int i = 0;
	char ack[6] = "\0";
	
	for(i = 0; i < 5; i++)
	{
		ack[i] = USART1_readChar();
	}
	ack[i] = '\0';
	//USART1_sendInt(strlen(ack));
	//USART1_sendString(ack);
	
	if (!(strcmp(ack, "ACK01")))
	{
		return 1;
	}
	else if (!(strcmp(ack, "ACK02")))
	{
		return 2;
	}
	else if (!(strcmp(ack, "ACK03")))
	{
		return 3;
	}
	else if (!(strcmp(ack, "ACK04")))
	{
		return 4;
	}
	return 0;
}
void usb_sync(void)
{
	
	//USART1_sendString("usb_sync");
	
	// Send User id
// 	USART1_sendString("?1A00000001#");
// 	if (get_ack() != 1)
// 	{
// 		USART1_sendString("Wrong acknowledged..\nConnection break!!");
// 		return;
// 	}
	
	//Sending Bolus Basal data
	//USART1_sendChar('?');
	//
	//USART1_sendString_without_newline("1A00000001");
	cli();
	read_all_packets(0);   // Sending Bolus, Basal, Smart bolus packets.
	
	//read_all_packets(1);   // Sending Glucometer packets.
	//
	//USART1_sendString("?1A00000001,U096200520211050210520211250090");
	//
	//USART1_sendChar('#');
	sei();
	/*  All data sent, Now waiting for Acknowledge.  */
	
	//if (get_ack() != 2)
	//{
		//USART1_sendString("Wrong acknowledged..\nConnection break!!");
		//return;
	//}
	//USART1_sendString("successfully sent.");
}

/************************************************************************/
/* Bolus history                                                                     */
/************************************************************************/
/* Function to check valid date and month. If date is valid it returns true else false.                                   */
/************************************************************************/
char date_string[11] = "\0";

/************************************************************************/
/* Function to print date.                                                                    */
/************************************************************************/
int read_date_history(int _address)
{
	char buff_[5] = "\0";	// Buffer to store the combination of date and month.
	char _buff[11] = "\0";	// Buffer to store the the date with separator in string format
	char separator[2] = "/";	// day, month and year separator
	//char *date = date_string;
	/*date_string = "\0";*/
	uint16_t val = AT24C32E_EEPROM_get_data_16bits(_address);	// Read the 16 bit data from external eeprom from address which is passed.
	// The data is day and month in four digits as DDMM format.
	intToStr((val / 100), buff_, 2);	// separate the day and store as string in buff_. Only two digits from starting are saved.
	strcat(_buff, buff_);	// Concatenate the string in buff_ with _buff (_buff has null initial value).
	strcat(_buff, separator);	// Separator symbol added.
	
	intToStr((val % 100), buff_, 2); // Separate the month and store as string in buff_. Only two digits are saved.
	strcat(_buff, buff_);	// Concatenate the month with day and separator symbol (available in _buff) and save in _buff.
	strcat(_buff, separator);	// Separator symbol added.
	_address += 2;	// Increase the eeprom address by 2 bytes.
	
	val = AT24C32E_EEPROM_get_data_16bits(_address);	// Read the year.
	intToStr(val, buff_, 4);  // Convert the year into the string temporarily and store in buff_. Only four digits are saved.
	strcat(_buff, buff_);	// Concatenate with day, month and separators and save in _buff.
	strcpy(date_string, _buff);	// Copy the date string from _buff to date_string to print multiple times till the date is changed.
	_address += 2;	// Increase the eeprom address by 2 bytes.
	return _address;	// Return EEPROM next read address.
}

/************************************************************************/
/* Function to print the bolus history. Returns EEPROM next read address.          */
/************************************************************************/
int read_bolus_history(int eeprom_addr)
{
	//char temp[15] = "\0";
	
	USART1_sendChar('\t');  // Send the horizontal space of single tab.
	
	eeprom_addr++;         // Increment of eeprom address for data by 1-byte.
	
	char buff_[5] = "\0";	// Buffer to store the combination of hour or minute.
	USART1_sendChar('B');	// Send bolus identifier on UART port.
	USART1_sendChar('\t');	// Send the horizontal space of single tab on UART port.
	
	uint16_t val = AT24C32E_EEPROM_get_data_16bits(eeprom_addr);	// Read the 16 bit data from external eeprom from address.
	// The data is hh and mm in four digits as hhmm format.
	intToStr(val / 100, buff_, 2);	// Separate the hour and convert it into string and store in buff_. Only 2 digits are stored.
	USART1_sendString(buff_);  // Send start time "hour" on UART port.
	USART1_sendChar(':');	// Send separator symbol on UART port
	intToStr(val % 100, buff_, 2);	// Separate the minute and convert it into string and store in buff_. Only 2 digits are stored.
	USART1_sendString(buff_);  // Send start time "minute" on UART port.
	USART1_sendChar('\t');	// Send the horizontal space of single tab.
	eeprom_addr += 2;	// Increment of eeprom address for data by 2-byte.
	
	intToStr(AT24C32E_EEPROM_get_data(eeprom_addr++), buff_, 2);	// Read the units of insulin taken.
	//	Convert into string and assign only 2 digits in buff_ and increase the address by 1-byte.
	USART1_sendString(buff_);	// Send units taken on UART port.
	USART1_sendChar('U');	// Send character 'U' on UART port. Indicates unit.
	USART1_sendChar('\t');	// Send the horizontal space of single tab on UART port.
	
	val = AT24C32E_EEPROM_get_data(eeprom_addr++);  // Analyzing bolus type and increase the eeprom address by 1-byte.
	if (val == 1)
	{
		USART1_sendString("5min");
	}
	else
	{
		USART1_sendString("Imdt.");
	}
	return(eeprom_addr);	// Return EEPROM next read address.
}


/************************************************************************/
/* Function to print the basel history. Returns EEPROM next read address.       */
/************************************************************************/
int read_basal_history(int eeprom_addr)
{
	char temp[3] = "\0";	// Buffer to store basel data in string format.
	USART1_sendChar('\t');	// Send the horizontal space of single tab on UART port.
	
	eeprom_addr++;	// Increment address for data.
	
	USART1_sendChar('Z');	// Send basel identifier on UART port.
	USART1_sendChar('\t');	// Send the horizontal space of single tab on UART port.
	
	uint16_t val = AT24C32E_EEPROM_get_data_16bits(eeprom_addr);	// Read the 16 bit data from external eeprom from address.
	// The data is hh and mm in four digits as hhmm format.
	intToStr(val / 100,temp, 2);	// Separate the start time "hour" and convert it into string and store in buff_. Only 2 digits are stored.
	USART1_sendString(temp);	// Send start time "hour" on UART port.
	USART1_sendChar(':');	// Send date separator.
	intToStr(val % 100, temp, 2);	// Separate the start time "minute" and convert it into string and store in buff_. Only 2 digits are stored.
	USART1_sendString(temp);  // Send start time "minute" on UART port.
	USART1_sendChar('\t');	// Send the horizontal space of single tab on UART port.
	eeprom_addr += 2;	// Increase the eeprom address by 2-bytes.
	
	val = AT24C32E_EEPROM_get_data_16bits(eeprom_addr);	// Read the 16 bit data from external eeprom from address.
	// The data is hh and mm in four digits as hhmm format.
	intToStr(val / 100, temp, 2);	// Separate the end time "hour" and convert it into string and store in buff_. Only 2 digits are stored.
	USART1_sendString(temp);  // Send end time "hour" on UART port.
	USART1_sendChar(':');	// Send date separator on UART port.
	intToStr(val % 100, temp, 2);	// Separate the end time "minute" and convert it into string and store in buff_. Only 2 digits are stored.
	USART1_sendString(temp);  // Send end time "minute" on UART port.
	USART1_sendChar('\t');	// Send the horizontal space of single tab on UART port.
	eeprom_addr += 2;	// Increase the eeprom address by 2-bytes.
	
	USART1_sendInt(AT24C32E_EEPROM_get_data_16bits(eeprom_addr++));	// Send flow of insulin as integer on UART port and increase the eeprom address.
	//	USART1_sendFloat(((float)AT24C32E_EEPROM_get_data_16bits(eeprom_addr++))/1000.0, 3);	// Send flow of insulin in 3 point decimal on UART port and increase the eeprom address.
	return(eeprom_addr);	// Return eeprom next read address.
}


/************************************************************************/
/* Function to print the glucose history. Returns EEPROM next read address.       */
/************************************************************************/
int read_glucose_history(int eeprom_addr)
{
	char temp[5] = "\0";	// Buffer to store glucose data.
	
	USART1_sendChar('\t');	// Send the horizontal space of single tab on UART port.
	
	eeprom_addr++;	// Increase the eeprom address by 1-byte.
	
	USART1_sendChar('G');  // Send glucose identifier on UART port.
	USART1_sendChar('\t');	// Send the horizontal space of single tab on UART port.
	
	uint16_t val = AT24C32E_EEPROM_get_data_16bits(eeprom_addr);	// Read the 16 bit data from external eeprom from address.
	// The data is hh and mm in four digits as hhmm format.
	intToStr(val / 100,temp, 2);	// Separate the start time "hour" and convert it into string and store in buff_. Only 2 digits are stored.
	USART1_sendString(temp);	// Send start time "hour" on UART port.
	USART1_sendChar(':');	// Send start time separator on UART port.
	intToStr(val % 100, temp, 2);	// Separate the end time "hour" and convert it into string and store in buff_. Only 2 digits are stored.
	USART1_sendString(temp);	// Send start time "hour" on UART port.
	USART1_sendChar('\t');	// Send the horizontal space of single tab on UART port.
	eeprom_addr += 2;	// Increase the eeprom address by 2-bytes.
	
	intToStr(AT24C32E_EEPROM_get_data_16bits(eeprom_addr), temp, 3);  // Read glucose value. Convert into string upto 3 digits and store in temp.
	USART1_sendString(temp);	// Send glucose value on UART port.
	USART1_sendChar('\t');	// Send the horizontal space of single tab on UART port.
	eeprom_addr += 2;	// Increase the eeprom address by 2-bytes.
	
	val = AT24C32E_EEPROM_get_data(eeprom_addr ++);	// Read reading type.
	if (val == 0)
	{
		USART1_sendString("Fasting");
	}
	else if (val == 1)
	{
		USART1_sendString("PP");
	}
	else if (val == 2)
	{
		USART1_sendString("Random");
	}
	else
	{
		;
	}
	/*USART1_sendString(temp);*/
	return eeprom_addr;	// Return eeprom next read address.
}


/************************************************************************/
/* Function to print the smart bolus history. Returns EEPROM next read address.       */
/************************************************************************/
int read_smart_bolus_history(int eeprom_addr)
{
	char temp[4] = "\0";	// Buffer to store smart bolus data.
	USART1_sendChar('\t');	// Send the horizontal space of single tab on UART port.
	USART1_sendChar('W');	// Printing smart bolus identifier.
	USART1_sendChar('\t');	// Send the horizontal space of single tab on UART port.
	eeprom_addr++;	// Increase the eeprom address by 1-byte.
	
	// 	intToStr(AT24C32E_EEPROM_get_data_16bits(eeprom_addr), buff, 4);   // Start time
	// 	strcat(temp, buff);
	uint16_t val = AT24C32E_EEPROM_get_data_16bits(eeprom_addr);	// Read the 16 bit data from external eeprom from address.
	// The data is hh and mm in four digits as hhmm format.
	intToStr(val / 100,temp, 2);	// Separate the start time "hour" and convert it into string and store in buff_. Only 2 digits are stored.
	USART1_sendString(temp);  // Send start time "hour" on UART port.
	USART1_sendChar(':');	// Send time separator.
	intToStr(val % 100, temp, 2);	// Separate the start time "minute" and convert it into string and store in buff_. Only 2 digits are stored.
	USART1_sendString(temp);  // Send start time "minute" on UART port.
	USART1_sendChar('\t');	// Send the horizontal space of single tab on UART port.
	eeprom_addr += 2;	// Increase the eeprom address by 2-bytes.
	
	intToStr(AT24C32E_EEPROM_get_data(eeprom_addr++), temp, 2);	// Read the units of insulin taken.
	// Convert into string and assign only 2 digits in buff_ and increase the address by 1-byte.
	USART1_sendString(temp);	// Send insulin units on UART port.
	USART1_sendChar('U');	// Send character 'U' on UART port. Indicates unit.
	USART1_sendChar('\t');	// Send the horizontal space of single tab on UART port.
	val = AT24C32E_EEPROM_get_data(eeprom_addr ++);	// Analyzing bolus type and increase the eeprom address by 1-byte.
	if (val == 0)
	{
		USART1_sendString("Imdt.");
	}
	else if (val == 1)
	{
		USART1_sendString("5min");
	}
	else
	{
		;
	}
	USART1_sendChar('\t');	// Send the horizontal space of single tab on UART port.
	intToStr(AT24C32E_EEPROM_get_data(eeprom_addr++), temp, 3);	// Get Carb Intake, convert into string, store upto 3 digits in temp and increase eeprom address 1-byte.
	USART1_sendString(temp); // Send Carb Intake value on UART port.
	USART1_sendChar('\t');	// Send the horizontal space of single tab on UART port.
	intToStr(AT24C32E_EEPROM_get_data(eeprom_addr++), temp, 3);	// Get C/I ratio, convert into string, store upto 3 digits in temp and increase eeprom address 1-byte.
	USART1_sendString(temp); // Send C/I ratio on UART port.
	USART1_sendChar('\t');	// Send the horizontal space of single tab on UART port.
	intToStr(AT24C32E_EEPROM_get_data(eeprom_addr++), temp, 3);	// Get Insulin Sensitivity, convert into string, store upto 3 digits in temp and increase eeprom address 1-byte.
	USART1_sendString(temp);	// Send Insulin Sensitivity on UART port.
	USART1_sendChar('\t');	// Send the horizontal space of single tab on UART port.
	intToStr(AT24C32E_EEPROM_get_data(eeprom_addr++), temp, 3);	// Get Lower Blood Glucose range, convert into string, store upto 3 digits in temp and increase eeprom address 1-byte.
	USART1_sendString(temp);	// Send Lower Blood Glucose range on UART port.
	USART1_sendChar('\t');	// Send the horizontal space of single tab on UART port.
	intToStr(AT24C32E_EEPROM_get_data(eeprom_addr++), temp, 3);	// Get Higher Blood Glucose range, convert into string, store upto 3 digits in temp and increase eeprom address 1-byte.
	USART1_sendString(temp);	// Send Higher Blood Glucose range on UART port.
	USART1_sendChar('\t');	// Send the horizontal space of single tab on UART port.
	intToStr(AT24C32E_EEPROM_get_data(eeprom_addr++), temp, 3);	// Get Active Insulin, convert into string, store upto 3 digits in temp and increase eeprom address 1-byte.
	USART1_sendString(temp);	// Send Active Insulin value on UART port.
	return eeprom_addr;	// Return eeprom next read address.
}/************************************************************************/

void read_bolus_basel_packets(void)
{
	int previous_date_address = 0;
	int StartAddress = read_data_in_eeprom_SPM(EEPROM_START_ADDRESS_INDEX);
	int EndAddress = read_data_in_eeprom_SPM(EEPROM_END_ADDRESS_INDEX);
	//	int EndAddress =  read_data_in_eeprom_SPM(EEPROM_GLUCOSE_END_ADDRESS_INDEX);
	int AddressTemp = StartAddress;
	
	if (StartAddress < EndAddress)    // Currently arranged in linear.
	{
		while(AddressTemp < EndAddress)
		{
			// Send enteries.
			if (check_date_start(AddressTemp))
			{
				previous_date_address = AddressTemp;
				AddressTemp = read_date_history(previous_date_address);
				//AddressTemp += 4;
			}
			else
			{
				switch(AT24C32E_EEPROM_get_data(AddressTemp))
				{
					case 'B':{
						USART1_sendChar('\n');
						USART1_sendString(date_string);
						AddressTemp = read_bolus_history(AddressTemp);
						//AddressTemp += 5;
						break;
					}
					case 'Z':{
						USART1_sendChar('\n');
						USART1_sendString(date_string);
						AddressTemp = read_basal_history(AddressTemp);
						//AddressTemp += 7;
						break;
					}
					case 'W':{
						USART1_sendChar('\n');
						USART1_sendString(date_string);
						AddressTemp = read_smart_bolus_history(AddressTemp);
						//USART1_sendString("Smart bolus packet");
						//AddressTemp += 11;
						break;
					}
					default: {
						AddressTemp++;
						break;
					}
				}
			}
		}
	}
	else                      // Enteries are circular now.
	{
		while(AddressTemp <=  BOLUS_ADDRESS_MAX)
		{
			// Fetch data to Maximum.
			if (check_date_start(AddressTemp))
			{
				//date_string[11] = "\0";
				strcpy(date_string, "");
				previous_date_address = AddressTemp;
				AddressTemp = read_date_history(previous_date_address);
				/*AddressTemp +=4;*/
			}
			else
			{
				switch(AT24C32E_EEPROM_get_data(AddressTemp))
				{
					case 'B':{
						USART1_sendChar('\n');
						USART1_sendString(date_string);
						//USART1_sendString("Bolus Packet.");
						AddressTemp = read_bolus_history(AddressTemp);
						//AddressTemp += 5;
						break;
					}
					case 'Z':{
						USART1_sendChar('\n');
						USART1_sendString(date_string);
						AddressTemp = read_basal_history(AddressTemp);
						//USART1_sendString("Basal Packet.");
						//AddressTemp += 7;
						break;
					}
					case 'W':{
						USART1_sendChar('\n');
						USART1_sendString(date_string);
						AddressTemp = read_smart_bolus_history(AddressTemp);
						//USART1_sendString("Smart bolus packet");
						/*AddressTemp += 11;*/
						break;
					}
					default: {
						AddressTemp++;
						break;
					}
				}
			}
		}
		
		AddressTemp = BOLUS_ADDRESS_MIN;
		while(AddressTemp <=  EndAddress)
		{
			// Fetch data to Maximum.
			if (check_date_start(AddressTemp))
			{
				//date_string[11] = "\0";
				strcpy(date_string, "");
				previous_date_address = AddressTemp;
				AddressTemp = read_date_history(previous_date_address);
				/*AddressTemp +=4;*/
			}
			else
			{
				switch(AT24C32E_EEPROM_get_data(AddressTemp))
				{
					case 'B':{
						USART1_sendChar('\n');
						USART1_sendString(date_string);
						//USART1_sendString("Bolus Packet.");
						AddressTemp = read_bolus_history(AddressTemp);
						//AddressTemp += 5;
						break;
					}
					case 'Z':{
						USART1_sendChar('\n');
						USART1_sendString(date_string);
						AddressTemp = read_basal_history(AddressTemp);
						//USART1_sendString("Basal Packet.");
						//AddressTemp += 7;
						break;
					}
					case 'W':{
						USART1_sendChar('\n');
						USART1_sendString(date_string);
						AddressTemp = read_smart_bolus_history(AddressTemp);
						//USART1_sendString("Smart bolus packet");
						/*AddressTemp += 11;*/
						break;
					}
					default: {
						AddressTemp++;
						break;
					}
				}
			}
		}
	}
}

void read_glucose_packets(void)
{
	int previous_date_address = 0;
	int StartAddress = read_data_in_eeprom_SPM(GLUCOSE_START_ADDRESS_INDEX);
	int EndAddress =  read_data_in_eeprom_SPM(GLUCOSE_END_ADDRESS_INDEX);
	int AddressTemp = StartAddress;
	
	if (StartAddress < EndAddress)    // Currently arranged in linear.
	{
		while(AddressTemp < EndAddress)
		{
			// Send enteries.
			if (check_date_start(AddressTemp))
			{
				previous_date_address = AddressTemp;
				AddressTemp = read_date_history(previous_date_address);
				//AddressTemp += 4;
			}
			else
			{
				switch(AT24C32E_EEPROM_get_data(AddressTemp))
				{
					case 'G':{
						USART1_sendChar('\n');
						USART1_sendString(date_string);
						AddressTemp = read_glucose_history(AddressTemp);
						break;
					}
					default: {
						AddressTemp++;
						break;
					}
				}
			}
		}
	}
	else                      // Entries are circular now.
	{
		while(AddressTemp <=  GLUCOSE_ADDRESS_MAX)
		{
			// Fetch data to Maximum.
			if (check_date_start(AddressTemp))
			{
				strcpy(date_string, "");
				previous_date_address = AddressTemp;
				AddressTemp = read_date_history(previous_date_address);
				//AddressTemp +=4;
			}
			else
			{
				switch(AT24C32E_EEPROM_get_data(AddressTemp))
				{
					case 'G':{
						USART1_sendChar('\n');
						USART1_sendString(date_string);
						AddressTemp = read_glucose_history(AddressTemp);
						break;
					}
					default: {
						AddressTemp++;
						break;
					}
				}
			}
		}
		
		AddressTemp = GLUCOSE_ADDRESS_MIN;
		while(AddressTemp <=  EndAddress)
		{
			// Fetch data to Maximum.
			if (check_date_start(AddressTemp))
			{
				strcpy(date_string, "");
				previous_date_address = AddressTemp;
				AddressTemp = read_date_history(previous_date_address);
				//AddressTemp +=4;
			}
			else
			{
				switch(AT24C32E_EEPROM_get_data(AddressTemp))
				{
					case 'G':{
						USART1_sendChar('\n');
						USART1_sendString(date_string);
						AddressTemp = read_glucose_history(AddressTemp);
						break;
					}
					default: {
						AddressTemp++;
						break;
					}
				}
			}
		}
	}
}
#endif