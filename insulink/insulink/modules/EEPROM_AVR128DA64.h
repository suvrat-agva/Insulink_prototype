#ifndef EEPROM_AVR128DA64_H_
#define EEPROM_AVR128DA64_H_

#include <avr/io.h>
#include <avr/eeprom.h>


// #define CURRENT_MOTOR_STEPS_INDEX 1
// #define DOSE_STEPS_INDEX 3
// #define STEPS_DELAY_INDEX 5
// #define CURRENT_SETTING_INDEX 7
// #define TOTAL_SETTINGS_INDEX 9
// #define BASE_DOSE_SETTING_INDEX 11
// #define BASE_FLOWRATE_SETTING_INDEX 13

/************************************************************************/
/* AVR128DA28/32/48/64
NVMCTRL - Nonvolatile Memory Controller Command Modes/LPM(1)
Reading of the memory arrays is handled using the LD* instructions.
The erase of the whole Flash (CHER) or the EEPROM (EECHER) is started by writing commands to the
NVMCTRL.CTRLA register. The other write/erase operations are just enabled by writing commands to the NVMCTRL.CTRLA register and must be
by writes using ST/SPM(1) followed instructions to the memory arrays.
Note:
1. LPM/SPM cannot be used for EEPROM.
To write a command in the NVMCTRL.CTRLA register, the following sequence needs to be executed:
1.Confirm that any previous operation is completed by reading the Busy (EEBUSY and FBUSY) flags in the NVMCTRL.STATUS register.
2. Write the appropriate key to the Configuration Change Protection (CPU.CCP) register to unlock the NVM Control A (NVMCTRL.CTRLA) register.
3. Write the desired command value to the CMD bit field in the Control A (NVMCTRL.CTRLA) register within the next four instructions.
To perform a write/erase operation in the NVM, the following steps are required:
1. Confirm that any previous operation is completed by reading the Busy (EEBUSY and FBUSY) flags in the NVMCTRL.STATUS register.
2. Optional: If the Flash is accessed in the CPU data space, map the corresponding 32 KB Flash section into the data space by writing the FLMAP bit field in the NVMCTRL.CTRLB register.
3. Write the desired command value to the NVMCTRL.CTRLA register as described before.
4. Write to the correct address in the data space/code space using the ST*SPM instructions.
5. Optional: If multiple write operations are required, go to step 4.
6. Write a NOOP or NOCMD command to the NVMCTRL.CTRLA register to clear the current command.


0x00 NOCMD No command
0x01 NOOP No operation
0x02 FLWR Flash Write Enable
0x08 FLPER Flash Page Erase Enable
0x09 FLMPER2 Flash 2-page Erase Enable
0x0A FLMPER4 Flash 4-page Erase Enable
0x0B FLMPER8 Flash 8-page Erase Enable
0x0C FLMPER16 Flash 16-page Erase Enable
0x0D FLMPER32 Flash 32-page Erase Enable
0x12 EEWR EEPROM Write Enable
0x13 EEERWR EEPROM Erase and Write Enable
0x18 EEBER EEPROM Byte Erase Enable
0x19 EEMBER2 EEPROM 2-byte Erase Enable
0x1A EEMBER4 EEPROM 4-byte Erase Enable
0x1B EEMBER8 EEPROM 8-byte Erase Enable
0x1C EEMBER16 EEPROM 16-byte Erase Enable
0x1D EEMBER32 EEPROM 32-byte Erase Enable
0x20 CHER Erase Flash and EEPROM. EEPROM is skipped if EESAVE fuse is set.
(UPDI access only.)
0x30 EECHER Erase EEPROM
Other - Reserved                                                                     */
/************************************************************************/
typedef uint16_t eeprom_addr_t;
void set_NVM_command_SPM(char command)
{
	CCP = 0x9D;   // SPM - Allow Self-Programming.
	NVMCTRL_CTRLA = command;
}

void write_data_in_eeprom_SPM(eeprom_addr_t index,uint16_t data)
{
	while(NVMCTRL.STATUS & (NVMCTRL_EEBUSY_bm));
	set_NVM_command_SPM(NVMCTRL_CMD_EEERWR_gc);

	//eeprom_write_word((MAPPED_EEPROM_START + index), data);
	*(uint16_t*)(eeprom_addr_t)(MAPPED_EEPROM_START + index) = data;

	set_NVM_command_SPM(NVMCTRL_CMD_NONE_gc);
}

void write_data_in_eeprom_SPM_8bits(eeprom_addr_t index,uint8_t data)
{
	while(NVMCTRL.STATUS & (NVMCTRL_EEBUSY_bm));
	set_NVM_command_SPM(NVMCTRL_CMD_EEERWR_gc);

	//eeprom_write_word((MAPPED_EEPROM_START + index), data);
	*(uint16_t*)(eeprom_addr_t)(MAPPED_EEPROM_START + index) = data;

	set_NVM_command_SPM(NVMCTRL_CMD_NONE_gc);
}

uint16_t read_data_in_eeprom_SPM(eeprom_addr_t index)
{
	uint16_t data;
	eeprom_is_ready();
	data = eeprom_read_word((uint16_t *)index);
	return data;
}

uint8_t read_data_in_eeprom_SPM_8bits(eeprom_addr_t index)
{
	uint8_t data;
	eeprom_is_ready();
	data = eeprom_read_word((uint16_t *)index);
	return data;
}

#endif