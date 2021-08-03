#ifndef SYSTEM_UI_H_
#define SYSTEM_UI_H_

#include <stdbool.h>

#include "STATUS.h"

#include "ENCODER.h"
#include "RTC_AVR128DA64.h"
#include "FUNCTIONALITY_INSULIN.h"
#include "RTC_MAX31342.h"
#include "EEPROM_AVR128DA64.h"
#include "DataFrame.h"

#define MAX_BOLUS_LIMIT (25.0)


void clear_all_buttons(void)
{
	Button_BOLUS = false;
	Button_OK = false;
	Button_BACK = false;
	Button_SETTING = false;
	Button_UP = false;
	Button_DOWN = false;
}

/************************************************************************/
/* Motor forward and backward                                                                     */
/************************************************************************/
void forward_main(void)
{
	PORTE.OUT |= (0xF << 0);
	_delay_ms(2);
	PORTE.OUT &= ~(1 << 0);
	_delay_ms(2);
	PORTE.OUT &= ~(1 << 2);
	_delay_ms(2);
	PORTE.OUT |= (1 << 0);
	_delay_ms(2);
}

void reverse_main(void)
{
	PORTE.OUT |= (1 << 0);
	_delay_ms(2);
	PORTE.OUT &= ~(1 << 2);
	_delay_ms(2);
	PORTE.OUT &= ~(1 << 0);
	_delay_ms(2);
	PORTE.OUT |= (0xF << 0);
	_delay_ms(2);
}

void check_motor(void)
{
	int AverageMotorDir = 0, i =0;
	
	USART1_sendString("Checking Motor Forward direction..");
	PORTE.OUT |= (0x0A);
	for (i = 0; i < 255; i++)
	{
		forward_main();
		AverageMotorDir += check_motor_status();
	}
	if (AverageMotorDir > 0)
	{
		USART1_sendString("Moves forward.. Confirmed.");
	}else if (AverageMotorDir < 0)
	{
		USART1_sendString("Moves Reverse, ERROR!!");
	}
	else
	{
		USART1_sendString("Motor stop!!");
	}
	
	AverageMotorDir = 0;
	USART1_sendString("Checking Motor Reverse direction..");
	PORTE.OUT |= (0x0A);
	for (i = 0; i < 255; i++)
	{
		reverse_main();
		AverageMotorDir += check_motor_status();
	}
	if (AverageMotorDir > 0)
	{
		USART1_sendString("Moves Reverse.. Confirmed.");
	}else if (AverageMotorDir < 0)
	{
		USART1_sendString("Moves reverse.. Confirmed.");
	}
	else
	{
		USART1_sendString("Motor stop!!");
	}
}

int get_int(int highrange, int lowrange, int variation)
{
	int val = 0;
	
	Button_OK = false;
	while(!Button_OK)
	{
		if (Button_UP)
		{
			Button_UP = false;
			val += variation;
			
			if (val > highrange)
			{
				val = highrange;
			}
			USART1_sendInt(val);
			
		}
		else if (Button_DOWN)
		{
			Button_DOWN = false;
			val -= variation;;
			
			if (val < lowrange)
			{
				val = lowrange;
			}
			USART1_sendInt(val);
			
		}
		_delay_ms(10);
		sleep_cpu();
	}
	
	return val;
}


float get_float(float highrange, float lowrange, float variation)
{
	float val = lowrange;
	
	Button_OK = false;
	while(!Button_OK)
	{
		if (Button_UP)
		{
			Button_UP = false;
			val += variation;
			
			if (val > highrange)
			{
				val = highrange;
			}
			USART1_sendFloat(val, 2);
			
		}
		else if (Button_DOWN)
		{
			Button_DOWN = false;
			val -= variation;;
			
			if (val < lowrange)
			{
				val = lowrange;
			}
			USART1_sendFloat(val, 2);
			
		}
		
		_delay_ms(10);
		sleep_cpu();
	}
	
	return val;
}
/************************************************************************/
/* Smart Bolus 
   Bolus calculated by Insulin device.
   Global Parameters should check first.
   @Parameters:   1. carbinsulin_ratio
                  2. lowerBGrange
				  3. higherBGrange
				  4. Insulinsensitivity        
   @Local variables: 1. CarbIntake
                     2. ActiveInsulin                                                   */
/************************************************************************/
void smart_bolus(void)
{
    float CurrentBG = 0.0;
	
	float DoseUnits = 0.0;
	bool type = false;    //Immediate
	
	USART1_sendString("Continue to SMART BOLUS, press BOLUS.\n or press BACK to go back.");
	Button_BOLUS = false;
	Button_BACK = false;
	while((!Button_BOLUS) && (!Button_BACK))
	{
		sleep_cpu();
		_delay_ms(10);
	}
	
	if (Button_BACK)
	{
		Button_BACK = false;
		return;
		//bolus_option();
	}
	_delay_ms(100);
	Button_BOLUS = false;
	
	USART1_sendString("Carb Insulin ratio:");
	USART1_sendFloat(carbinsulin_ratio, 2);
	USART1_sendString("Insulin sensitivity:");
	USART1_sendFloat(Insulinsensitivity, 2);
	USART1_sendString("Lower BG range:");
	USART1_sendInt(lowerBGrange);
	USART1_sendString("Higher BG range:");
	USART1_sendInt(higherBGrange);
	
	USART1_sendString("Continue with these settings? \nPress Bolus to continue or Press Back to go back.");
	Button_BOLUS = false;
	Button_BACK = false;
	while((!Button_BACK) && (!Button_BOLUS))
	{
		sleep_cpu();
		_delay_ms(50);
	}
	if (Button_BACK)
	{
		Button_BACK = false;
		USART1_sendString("Exit.");
		return;
	}
	
	_delay_ms(100);
	
	USART1_sendString("Enter Carb intake:");
	CarbIntake = get_float(100.0, 0.0, 5.0);
	
	_delay_ms(100);
	USART1_sendString("Wants to enter Current BG? \nPress Bolus to enter or press Back.");
	Button_BOLUS = false;
	Button_BACK = false;
	while((!Button_BACK) && (!Button_BOLUS))
	{
		sleep_cpu();
		_delay_ms(50);
	}
	if (Button_BOLUS)
	{
		Button_BOLUS = false;
		USART1_sendString("Current BG:");
		CurrentBG = get_float(200.0, 50.0, 1.0);
	}
	else
	{
		Button_BACK = false;
		CurrentBG = 0.0;
	}
	
	_delay_ms(100);
	USART1_sendString("Wants to enter Active insulin? \nPress Bolus to enter or press Back.");
	Button_BOLUS = false;
	Button_BACK = false;
	while((!Button_BACK) && (!Button_BOLUS))
	{
		sleep_cpu();
		_delay_ms(50);
	}
	if (Button_BOLUS)
	{
		Button_BOLUS = false;
		USART1_sendString("Enter Active insulin:");
		ActiveInsulin = get_float(100.0, 0.0, 0.5);
	}
	else
	{
		Button_BACK = false;
		ActiveInsulin = 0.0;
	}
	
	USART1_sendString("Carb intake:");
	USART1_sendFloat(CarbIntake, 2);
	USART1_sendString("Active insulin:");
	USART1_sendFloat(ActiveInsulin, 2);
	USART1_sendString("Current BG:");
	USART1_sendFloat(CurrentBG, 2);
	
	if ((CurrentBG == 0.0))
	{
		DoseUnits = (CarbIntake / carbinsulin_ratio);
	}
	else
	{
		if ((CurrentBG > lowerBGrange) && (CurrentBG <higherBGrange))
		{
			DoseUnits = (CarbIntake / carbinsulin_ratio);
		}
		else if (CurrentBG > higherBGrange)
		{
			DoseUnits = (CarbIntake / carbinsulin_ratio) + ((CurrentBG - higherBGrange) / Insulinsensitivity);
		}
		else
		{
			DoseUnits = (CarbIntake / carbinsulin_ratio) - ((lowerBGrange - CurrentBG) / Insulinsensitivity);
		}
	}
	
	// Check if current dose is less than active insulin or not.
	if (DoseUnits <= ActiveInsulin)
	{
		USART1_sendString("Active insulin is more than dose units. \nNo delivery,EXIT.");
		return;
	}
	
	DoseUnits = DoseUnits - ActiveInsulin;
	//USART1_sendString("Dose units:");
	//USART1_sendFloat(DoseUnits, 2);
	
	Button_OK = false;
	USART1_sendString("Select type: \n0 -> IMMEDIATE \n1->5 MIN");
	do
	{
		if (Button_UP)
		{
			Button_UP = false;
			type = true;
			
			if (type)
			{
				USART1_sendString("5 MIN");
			}
			else
			{
				USART1_sendString("IMMEDIATE");
			}
			
		}
		else if (Button_DOWN)
		{
			Button_DOWN = false;
			type = false;
			
			if (type)
			{
				USART1_sendString("5 MIN");
			}
			else
			{
				USART1_sendString("IMMEDIATE");
			}
		}
		_delay_ms(10);
		sleep_cpu();
	} while (!Button_OK);
	
	USART1_sendString("Dose units:");
	USART1_sendFloat(DoseUnits, 2);
	USART1_sendString("Type:");
	if (type)
	{
		USART1_sendString("5 MIN");
	}
	else
	{
		USART1_sendString("IMMEDIATE");
	}
	
	// Delivering Smart bolus.
	bolus_configure(DoseUnits, type, 1);   // Smart bolus.
}


/************************************************************************/
/* Selecting BOLUS dose and type.                                                                     */
/************************************************************************/
float g_max_bolus_limit = 25.0;

void bolus(void)
{
	float DoseUnit = 10.0;
	bool BolusType = 0;
	clear_all_buttons();
	
	USART1_sendString("Continue to BOLUS, press BOLUS.\n or press BACK to go back.");
	Button_BOLUS = false;
	Button_BACK = false;
	while((!Button_BOLUS) && (!Button_BACK))
	{
		_delay_ms(10);
		sleep_cpu();
	}
	
	if (Button_BACK)
	{
		Button_BOLUS = true;
		return;
		//bolus_option();
	}
	_delay_ms(100);
	Button_BACK = false;
	USART1_sendString("Press BACK after selecting dose units. \nEnter Dose units: ");
	//do 
	//{
		//if (Button_UP)
		//{
			//Button_UP = false;
			//DoseUnit += 0.5;
			//
			//if (DoseUnit > g_max_bolus_limit)
			//{
				//DoseUnit = g_max_bolus_limit;
			//}
			//USART1_sendFloat(DoseUnit, 2);
			//
		//}
		//else if (Button_DOWN)
		//{
			//Button_DOWN = false;
			//DoseUnit -= 0.5;;
			//
			//if (DoseUnit < 0)
			//{
				//DoseUnit = 0.0;
			//}
			//USART1_sendFloat(DoseUnit, 2);
			//
		//}
		//_delay_ms(10);
	//} while (!Button_BACK);
	
	DoseUnit = get_float(g_max_bolus_limit, 0.0, 0.5);
	USART1_sendString("Selected BOLUS dose units:");
	USART1_sendFloat(DoseUnit, 2);
	
	Button_OK = false;
	USART1_sendString("Select type: \n0 -> IMMEDIATE \n1->5 MIN");
	do
	{
		if (Button_UP)
		{
			Button_UP = false;
			BolusType = true;
			
			if (BolusType)
			{
				USART1_sendString("5 MIN");
			}
			else
			{
				USART1_sendString("IMMEDIATE");
			}
			
		}
		else if (Button_DOWN)
		{
			Button_DOWN = false;
			BolusType = false;
			
			if (BolusType)
			{
				USART1_sendString("5 MIN");
			}
			else
			{
				USART1_sendString("IMMEDIATE");
			}
		}
		
		_delay_ms(10);
		sleep_cpu();
		
	} while (!Button_OK);
	
	USART1_sendString("Selected BOLUS dose units:");
	USART1_sendFloat(DoseUnit, 2);
	USART1_sendString("Type:");
	if (BolusType)
	{
		USART1_sendString("5 MIN");
	}
	else
	{
		USART1_sendString("IMMEDIATE");
	}
	
	bolus_configure(DoseUnit, BolusType, 0);   // Manual Bolus
}


/************************************************************************/
/* Selecting multiple BASAL.                                                                       */
/************************************************************************/
uint8_t get_hour(void)
{
	int hour = 0;
	USART1_sendString("Press OK when done. \nHour:");
	Button_OK = false;
	while(!Button_OK)
	{
		_delay_ms(10);
		sleep_cpu();
		
		if (Button_UP)
		{
			Button_UP = false;
			hour++;
			if (hour > 23)
			{
				hour = 23;
			}
			
			USART1_sendInt(hour);
		}
		else if (Button_DOWN)
		{
			Button_DOWN = false;
			hour--;
			if (hour < 0)
			{
				hour = 0;
			}
			
			USART1_sendInt(hour);
		}
	}
	_delay_ms(100);
	Button_OK = false;
	return hour;
}

uint8_t get_minute(void)
{
	int minute = 0;
	USART1_sendString("Press OK when done. \nMinute:");
	Button_OK = false;
	while(!Button_OK)
	{
		_delay_ms(10);
		sleep_cpu();
		
		if (Button_UP)
		{
			Button_UP = false;
			minute++;
			if (minute > 59)
			{
				minute = 59;
			}
			
			USART1_sendInt(minute);
		}
		else if (Button_DOWN)
		{
			Button_DOWN = false;
			minute--;
			if (minute < 0)
			{
				minute = 0;
			}
			
			USART1_sendInt(minute);
		}
	}
	_delay_ms(100);
	Button_OK = false;
	return minute;
}

uint16_t get_flowrate(void)
{
	float flowrate = 10.0;
	USART1_sendString("Press OK when done. \nFlowrate:");
	Button_OK = false;
	while(!Button_OK)
	{
		_delay_ms(10);
		sleep_cpu();
		
		if (Button_UP)
		{
			Button_UP = false;
			flowrate += 0.5;
			if (flowrate > 50.0)
			{
				flowrate = 50.0;
			}
			
			USART1_sendFloat(flowrate, 4);
		}
		else if (Button_DOWN)
		{
			Button_DOWN = false;
			flowrate -= 0.5;
			if (flowrate < 0.025)
			{
				flowrate = 0.025;
			}
			
			USART1_sendFloat(flowrate, 4);
		}
	}
	_delay_ms(100);
	Button_OK = false;
	//USART1_sendInt(flowrate * 1000);
	return (flowrate * 1000);
}

#define BASAL_SIZE 6
static uint16_t *ptr = NULL;
static uint8_t NumberOfEnteries = 0;

void add_basal(void)
{
	uint8_t status = 0;
	uint16_t StartTime =0, EndTime = 0;
	static uint16_t PreviousEndTime = 0, pos = 0;
	uint16_t flowrate = 0;
	
	// Start Ending Entries..
	NumberOfEnteries++;   // Adding Entry.
	USART1_sendString("Entry:");
	USART1_sendInt(NumberOfEnteries);
	pos = BASAL_SIZE * (NumberOfEnteries - 1);
	
	
	if (NumberOfEnteries == 1)    // Check for first time entry
	{
		ptr = (uint16_t *)malloc(BASAL_SIZE * sizeof(uint16_t));
		if (ptr == NULL)
		{
			USART1_sendString("Cannot assign space..");
			exit(0);
		}
	}
	else
	{
		ptr = realloc(ptr, NumberOfEnteries * BASAL_SIZE * sizeof(uint16_t));
		if (ptr == NULL)
		{
			USART1_sendString("Cannot assign space..");
			exit(0);
		}
	}
	
	USART1_sendString("StartTime: ");
	StartTime = (get_hour() * 100) + get_minute();
	USART1_sendString("EndTime: ");
	EndTime = (get_hour() * 100) + get_minute();
	USART1_sendString("Flowrate:");
	flowrate = get_flowrate();
	
	if (StartTime > EndTime)
	{
		status = 1;
		USART1_sendString("EndTime should be greater than StartTime");
	}
	else if (PreviousEndTime > StartTime)
	{
		status = 2;
		USART1_sendString("StartTime should be after Previous EndTime");
	}
	
	if (!status)
	{
		//USART1_sendString("Pos:");
		//USART1_sendInt(pos);
		
		*(ptr + pos++) = StartTime;
		
		*(ptr + pos++) = EndTime;
		PreviousEndTime = EndTime;
		
		*(ptr + pos++) = flowrate;
		
		USART1_sendString("Wants to Add more? \nPress Bolus to Add or Back to go back.");
		Button_BOLUS = false;
		Button_BACK = false;
		while((!Button_BACK) && (!Button_BOLUS))
		{
			sleep_cpu();
			_delay_ms(100);
		}
		
		if (Button_BACK)
		{
			Button_BACK = false;
			
			USART1_sendString("Number of Entries:");
			USART1_sendInt(NumberOfEnteries);
			
			int i = 0;
			
			// Saving to Internal EERPOM
			write_data_in_eeprom_SPM_8bits(BASAL_TOTAL_ENTERIES, NumberOfEnteries);
			for (i = 0; i < (3 * NumberOfEnteries); i++)
			{
				write_data_in_eeprom_SPM((BASAL_ENTRY_INDEX + (2*i)), (*(ptr + (i % 3) + (BASAL_SIZE * (i / 3)))));
				
			}
			USART1_sendString("Done writting to EEPROM. \nReading from EEPROM..");
			USART1_sendInt(read_data_in_eeprom_SPM_8bits(BASAL_TOTAL_ENTERIES));
			for (i = 0; i < (3 * NumberOfEnteries); i++)
			{
				USART1_sendInt(read_data_in_eeprom_SPM((BASAL_ENTRY_INDEX + (2*i))));
			}
			
			
			USART1_sendString("Enteries:");
			for (i = 0; i < NumberOfEnteries; i++)
			{
				
				intToStr(*(ptr + (BASAL_SIZE * i)), buff, 4);
				USART1_sendString(buff);
				intToStr(*(ptr + 1 + (BASAL_SIZE * i)), buff, 4);
				USART1_sendString(buff);
				
				USART1_sendInt(*(ptr + 2 + (BASAL_SIZE * i)));
				USART1_sendChar('\n');
			}
			
			free(ptr);    // Clean up memory.
			return;
		}
		
		Button_BOLUS = false;
		add_basal();
	}
	else
	{
		status = 0;
		NumberOfEnteries--;
		add_basal();
	}
}

void set_basal(void)
{	
	// Check for previous bolus settings..
	if ((read_data_in_eeprom_SPM_8bits(BASAL_TOTAL_ENTERIES) != 0) && (read_data_in_eeprom_SPM_8bits(BASAL_TOTAL_ENTERIES) != 255))
	{
		USART1_sendString("Enteries Found..");
		int i = 0;
		USART1_sendInt(read_data_in_eeprom_SPM_8bits(BASAL_TOTAL_ENTERIES));
		for (i = 0; i < (3 * NumberOfEnteries); i++)
		{
			USART1_sendInt(read_data_in_eeprom_SPM((BASAL_ENTRY_INDEX + (2*i))));
		}
		
		USART1_sendString("Wants to clear previous setting? Press OK to clear or BACK to go back");
		Button_OK = false;
		Button_BACK = false;
		
		while((!Button_BACK) && (!Button_OK))
		{
			sleep_cpu();
			_delay_ms(100);
		}
		
		if (Button_BACK)
		{
			return;
		}
		
		Button_OK = false;
		
		NumberOfEnteries = read_data_in_eeprom_SPM_8bits(BASAL_TOTAL_ENTERIES);
		USART1_sendString("Total enteries:");
		USART1_sendInt(NumberOfEnteries);
		USART1_sendString("Clearing enteries..");
		write_data_in_eeprom_SPM_8bits(BASAL_TOTAL_ENTERIES, 0xff);
		for (i = 0; i < (3 * NumberOfEnteries); i++)
		{
			write_data_in_eeprom_SPM((BASAL_ENTRY_INDEX + (2*i)), 0xffff);
		}
		USART1_sendString("Enteries cleared.");
		NumberOfEnteries = 0;
	}
	else
	{
		USART1_sendString("No previous enteries found.");
	}
	
	USART1_sendString("Points to remember:");
	USART1_sendString("1. Time Should be in 24 hour format.");
	USART1_sendString("2. Time is always start from 00:00 to 23:59.\n So choose your starttime and endtime in between such that Endtime > Starttime and\n New StartTime > previous EndTime. ");
	
	NumberOfEnteries = 0;
	add_basal();
}
	

/************************************************************************/
/* Check Basal to start at powerON.                                                                     */
/************************************************************************/
void check_basal_to_deliver(void)
{
	uint8_t EntryNo = 0, i = 0;
	if ((read_data_in_eeprom_SPM_8bits(BASAL_TOTAL_ENTERIES) != 0) && (read_data_in_eeprom_SPM_8bits(BASAL_TOTAL_ENTERIES) != 255))
	{
		uint16_t CurrentTime = (hour * 100) + minute;
		USART1_sendInt(CurrentTime);
		NumberOfEnteries = read_data_in_eeprom_SPM_8bits(BASAL_TOTAL_ENTERIES);
		USART1_sendString("Total enteries:");
		for (i = 0; i < NumberOfEnteries; i++)
		{
			EntryNo++;
			USART1_sendString("EntryNo.");
			USART1_sendInt(EntryNo);
			USART1_sendInt(read_data_in_eeprom_SPM(BASAL_ENTRY_INDEX + (BASAL_SIZE * i) + 2));
			
			if (read_data_in_eeprom_SPM(BASAL_ENTRY_INDEX + (BASAL_SIZE * i) + 2) > CurrentTime)
			{
				USART1_sendString("Entry matched for set..\nEntry No.:");
				USART1_sendInt(EntryNo);
				
				USART1_sendInt(read_data_in_eeprom_SPM(BASAL_ENTRY_INDEX + (BASAL_SIZE * i)));
				USART1_sendInt(read_data_in_eeprom_SPM(BASAL_ENTRY_INDEX + (BASAL_SIZE * i) + 2));
				USART1_sendFloat(read_data_in_eeprom_SPM(BASAL_ENTRY_INDEX + (BASAL_SIZE * i) + 4) / 1000, 4);
				basal_configure(read_data_in_eeprom_SPM(BASAL_ENTRY_INDEX + (BASAL_SIZE * i)), read_data_in_eeprom_SPM(BASAL_ENTRY_INDEX + (BASAL_SIZE * i) + 2), (read_data_in_eeprom_SPM(BASAL_ENTRY_INDEX + (BASAL_SIZE * i) + 4)) / 1000);
				
				g_start_basal = true;
				
				return;
			}
			else
			{
				g_start_basal = false;
			}
		}
	}
}

void set_next_basal_with_endtime(uint16_t Endtime)
{
	uint8_t EntryNo = 0, i = 0;
	USART1_sendString("set_next_basal_with_endtime");
	if ((read_data_in_eeprom_SPM_8bits(BASAL_TOTAL_ENTERIES) != 0) && (read_data_in_eeprom_SPM_8bits(BASAL_TOTAL_ENTERIES) != 255))
	{
		uint16_t CurrentTime = Endtime;
		USART1_sendString_without_newline("Endtime : ");
		USART1_sendInt(CurrentTime);
		NumberOfEnteries = read_data_in_eeprom_SPM_8bits(BASAL_TOTAL_ENTERIES);
		USART1_sendString("Total enteries:");
		for (i = 0; i < NumberOfEnteries; i++)
		{
			EntryNo++;
			USART1_sendString("EntryNo.");
			USART1_sendInt(EntryNo);
			USART1_sendInt(read_data_in_eeprom_SPM(BASAL_ENTRY_INDEX + (BASAL_SIZE * i) + 2));
			
			if (read_data_in_eeprom_SPM(BASAL_ENTRY_INDEX + (BASAL_SIZE * i) + 2) > CurrentTime)      // Checks for endtime to initiate.
			{
				USART1_sendString("Entry matched for set..\nEntry No.:");
				USART1_sendInt(EntryNo);
				
				USART1_sendInt(read_data_in_eeprom_SPM(BASAL_ENTRY_INDEX + (BASAL_SIZE * i)));
				USART1_sendInt(read_data_in_eeprom_SPM(BASAL_ENTRY_INDEX + (BASAL_SIZE * i) + 2));
				USART1_sendFloat(read_data_in_eeprom_SPM(BASAL_ENTRY_INDEX + (BASAL_SIZE * i) + 4) / 1000, 4);
				basal_configure(read_data_in_eeprom_SPM(BASAL_ENTRY_INDEX + (BASAL_SIZE * i)), read_data_in_eeprom_SPM(BASAL_ENTRY_INDEX + (BASAL_SIZE * i) + 2), (read_data_in_eeprom_SPM(BASAL_ENTRY_INDEX + (BASAL_SIZE * i) + 4)) / 1000);
				
				g_start_basal = true;
				
				return;
			}
			else
			{
				g_start_basal = false;
			}
		}
	}
	else
	{
		g_start_basal = false;
	}
}

void review_all_basal_enteries(void)
{
	uint8_t EntryNo = 0, i = 0;
	if ((read_data_in_eeprom_SPM_8bits(BASAL_TOTAL_ENTERIES) != 0) && (read_data_in_eeprom_SPM_8bits(BASAL_TOTAL_ENTERIES) != 255))
	{
		NumberOfEnteries = read_data_in_eeprom_SPM_8bits(BASAL_TOTAL_ENTERIES);
		USART1_sendString("Total enteries:");
		for (i = 0; i < NumberOfEnteries; i++)
		{
			USART1_sendChar('\n');
			EntryNo++;
			USART1_sendString_without_newline("EntryNo. :");
			USART1_sendInt(EntryNo);
			USART1_sendString_without_newline("Start time :");
			USART1_sendInt(read_data_in_eeprom_SPM(BASAL_ENTRY_INDEX + (BASAL_SIZE * i)));
			
			USART1_sendString_without_newline("End time :");
			USART1_sendInt(read_data_in_eeprom_SPM(BASAL_ENTRY_INDEX + (BASAL_SIZE * i) + 2));
			
			USART1_sendString_without_newline("Flowrate :");
			USART1_sendFloat(read_data_in_eeprom_SPM(BASAL_ENTRY_INDEX + (BASAL_SIZE * i) + 4) / 1000, 4);
			USART1_sendChar('\n');
		}
	}
}

void basal(void)
{	
	uint8_t total_option = 3, option = 1;
	
	_delay_ms(100);
	USART1_sendString("Continue to BASAL, press BOLUS.\n or press BACK to go back.");
	Button_BOLUS = false;
	Button_BACK = false;
	while((!Button_BOLUS) && (!Button_BACK))
	{
		sleep_cpu();
		_delay_ms(10);
	}
	
	if (Button_BACK)
	{
		Button_BOLUS = true;
		Button_BACK = false;
		return;
		//bolus_option();
	}
	Button_BOLUS = false;
	
	_delay_ms(100);	
	
	USART1_sendString("Select :");
	USART1_sendString("1. Set Basal");
	USART1_sendString("2. Review Basal");
	USART1_sendString("3. Start Basal");
	
	option = get_int(total_option, 1, 1);
	
	switch(option)
	{
		case 1:{
			USART1_sendString("Set Basal.");
			set_basal();
			break;
		}
		case 2:{
			USART1_sendString("Review Basal.");
			review_all_basal_enteries();
			break;
		}
		case 3:{
			USART1_sendString("Start Basal.");
			g_start_basal = true;
			set_next_basal_with_endtime((hour * 100) + minute);
			//check_basal_to_deliver();
			break;
		}
		default: break;
	}
}
/************************************************************************/
/* Bolus setting                                                                     */
/************************************************************************/
void set_max_bolus_limit(void)
{
	USART1_sendString("Enter Max bolus limit");
	g_max_bolus_limit = get_float(MAX_BOLUS_LIMIT, 0.0, 0.5);
	USART1_sendString_without_newline("Max bolus limit selected = ");
	USART1_sendFloat(g_max_bolus_limit, 2);
}
void set_smart_bolus_parameters(void)
{
	/************************************************************************/
	/* float carbinsulin_ratio = 10.0, Insulinsensitivity = 10.0;
	   int lowerBGrange = 80, higherBGrange = 100;                                                                     */
	/************************************************************************/
	USART1_sendString("Enter carb insulin ratio.");
	carbinsulin_ratio = get_float(50.0, 0.0, 1.0);
	USART1_sendString("Enter insulin sensitivity.");
	Insulinsensitivity = get_float(50.0, 0.0, 1.0);
	USART1_sendString("Lower BG range");
	lowerBGrange = get_int(250, 0, 1);
	USART1_sendString("Higher BG range");
	higherBGrange = get_int(250, 0, 1);
	
	USART1_sendString("Smart bolus setting updated.");
	USART1_sendString("Carb Insulin ratio:");
	USART1_sendFloat(carbinsulin_ratio, 2);
	USART1_sendString("Insulin sensitivity:");
	USART1_sendFloat(Insulinsensitivity, 2);
	USART1_sendString("Lower BG range:");
	USART1_sendInt(lowerBGrange);
	USART1_sendString("Higher BG range:");
	USART1_sendInt(higherBGrange);
}

void bolus_setting(void)
{
	_delay_ms(100);
	USART1_sendString("Continue to BOLUS SETTING, press BOLUS.\n or press BACK to go back.");
	Button_BOLUS = false;
	Button_BACK = false;
	while((!Button_BOLUS) && (!Button_BACK))
	{
		_delay_ms(10);
	}
	
	if (Button_BACK)
	{
		Button_BOLUS = true;
		Button_BACK = false;
		return;
		//bolus_option();
	}
	Button_BOLUS = false;
	
	_delay_ms(100);
	
	USART1_sendString("1. Max Bolus limit \n2. Smart bolus setting.");
	uint8_t option = get_int(2, 1, 1);
	
	switch(option)
	{
		case 1:{
		    set_max_bolus_limit();
			break;
		}
		case 2:{
			set_smart_bolus_parameters();
			break;
		}
		default: break;
	}
}

/************************************************************************/
/* Action performed when BOLUS pressed after system ON.                                                                     */
/************************************************************************/
void bolus_option(void)
{
	int select_option = 1;
	const uint8_t total_option = 6;
	if (Button_BOLUS)
	{
		Button_BOLUS = false;
		
		USART1_sendString("Press OK to confirm.");
		USART1_sendString("1. BOLUS");
		USART1_sendString("2. BASAL");
		USART1_sendString("3. SMART BOLUS");
		USART1_sendString("4. BOLUS SETTING");
		USART1_sendString("5. BOLUS HISTORY");
		USART1_sendString("6. BACK");
		Button_OK = false;
		
		select_option = get_int(total_option, 1 , 1);
// 		if (!select_option)
// 		{
// 			USART1_sendString("BACK");
// 			return;
// 		}
// 		//do 
		//{
			//
			//if (Button_UP)
			//{
				//Button_UP = false;
				//select_option++;
				//
				//if (select_option > total_option)
				//{
					//select_option = total_option;
				//}
				//USART1_sendInt(select_option);
				//
			//}
			//else if (Button_DOWN)
			//{
				//Button_DOWN = false;
				//select_option--;
				//
				//if (select_option < 1)
				//{
					//select_option = 1;
				//}
				//USART1_sendInt(select_option);
			//}
			//else if (Button_BACK)
			//{
				//Button_BACK = false;
				//USART1_sendString("Back");
				//return;
			//}
			//
			//_delay_ms(10);
			//sleep_cpu();
		//} while (!Button_OK);
		//Button_OK = false;
		
		
		switch(select_option)
		{
			case 1: {
				       USART1_sendString("BOLUS selected.");
					   bolus(); 
					   break;
			        }
			case 2: {
				        USART1_sendString("SET BASAL selected.");
				        basal();
				        break;
			         }		
			case 3: {
						USART1_sendString("SMART BOLUS selected.");
						smart_bolus();
						break;
			} 
			case 4: {
						USART1_sendString("BOLUS SETTING SELECTED.");
						bolus_setting();
						break;
			}
			case 5: {
				        USART1_sendString("BOLUS HISTORY");
						read_bolus_basel_packets();
						read_glucose_packets();
				        break;
			}
			case 6:{
				USART1_sendString("BACK");
				break;
			}
			default: break;
		}
	}
}



//------------------------------------ SETTING OPTIONS ------------------------------
void set_date_time(void)
{
	int user_date = 0, user_month = 0, user_year = 0, user_dow = 1, user_hour = 0, user_minute = 0, user_second = 0;
	USART1_sendString("Set Date:");
	user_date = get_int(31, 1, 1);
    
	USART1_sendString("Set Month:");
	user_month = get_int(12, 1, 1);
	
	USART1_sendString("Set Year:");
	user_year = get_int(2099, 2021, 1);
	
	//USART1_sendString("Set Day of week \n 1-> Sunday to 7 -> Saturday.");
	//user_dow = get_int(7, 1, 1);
	
	USART1_sendString("Set Hour:");
	user_hour = get_int(23, 0, 1);
	
	USART1_sendString("Set Minute:");
	user_minute = get_int(59, 0, 1);
	
	USART1_sendString("Set Second:");
	user_second = get_int(59, 0, 1);
	
	USART1_sendString("Updating time..");
	set_RTC_manually_Max31342(user_second, user_minute, user_hour, user_dow, user_date, user_month, (user_year % 100));
	USART1_sendString("Time updated..");
	
	//USART1_sendString("Reseting system.");
	//CCP = 0xD8;
	//RSTCTRL.SWRR |= 0x01;    // Resetting system.
	
}

void setting(void)
{
	uint8_t total_option = 4;
	
	if (Button_SETTING)
	{
		Button_SETTING = false;
	    
		USART1_sendString("Setting Selected.");
		USART1_sendString("1. Set Date/Time \n2. BG Alert \n3. USB SYNC \n4.BACK");
		//uint8_t option = get_int(total_option, 1, 1);
		
		uint8_t option = 1;
		
		option = get_int(total_option, 1, 1);
// 		while(!Button_OK)
// 		{
// 			if (Button_UP)
// 			{
// 				Button_UP = false;
// 				option += 1;
// 				if (option > total_option)
// 				{
// 					option = total_option;
// 				}
// 				USART1_sendInt(option);
// 			}
// 			else if (Button_DOWN)
// 			{
// 				Button_DOWN = false;
// 				option -= 1;
// 				if (option < 1)
// 				{
// 					option = 1;
// 				}
// 				USART1_sendInt(option);
// 			}
// 			else if (Button_BACK)
// 			{
// 				Button_BACK = false;
// 				USART1_sendString("Back");
// 				return;
// 			}
// 			_delay_ms(10);
// 			sleep_cpu();
// 		}
		
		switch(option)
		{
			case 1:{
				USART1_sendString("Setting date and time.");
				set_date_time();
				break;
			}
			case 2:{
				USART1_sendString("BG alert selected.");
				USART1_sendString("1 -> ON , 2 -> OFF");
				g_bg_alert = (get_int(2, 1, 1) % 2);
				if (g_bg_alert)
				{
					USART1_sendString("BG Alert ON.");
				}
				else
				{
					USART1_sendString("BG Alert OFF.");
				}
				break;
			}
			case 3:{
				USART1_sendString("USB SYNC selected.");
				usb_sync();
				break;
			}
			case 4:{
				USART1_sendString("BACK");
				break;
			}
			default: break;
		}
	}
}
#endif