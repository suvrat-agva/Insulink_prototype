# Insulink

An insulin pump is a small computerized device. It delivers insulin through a thin tube that goes under your skin.The device releases insulin almost the way your body naturally would: a steady flow throughout the day and night, called basal insulin, and an extra dose at mealtime, called a bolus, to handle rising blood sugar from the food you eat. You program the pump for both basal and bolus doses. If you eat more than normal, you can program a larger bolus to cover the carbs in your food. A bolus can bring down high blood sugar at other times, too.

The pump is about the size of a smartphone. You attach it to your body using an infusion set: thin plastic tubing and either a needle or a small tapered tube called a cannula you put under the skin. The place where you put it in -- your belly, buttock, or sometimes thigh -- is called the infusion site. Some pumps come with inserters for easier placement even in hard-to-reach areas.

Link:
https://www.webmd.com/diabetes/insulin-pump

--------------------------------------------------------------------------------

# Project code version
  insulink_v1.0.1

--------------------------------------------------------------------------------

#Project Status
 Bolus, Basal and Smart Bolus is set and delived the particularly selected dose units.
 In this verison, only bolus option is introduced.
 No Power consumption till now.
 
 set time at initial was introduced.
 solved error in circular writing in eeprom.
 Set and get device ID added
--------------------------------------------------------------------------------

# Dependencies
#include "UART_1_AVR128DA64.h"
#include "ADC_AVR128DA64.h"
#include "I2C_0_AVR128DA64.h"
#include "SPI_0_AVR128DA64.h"


// drivers--------------------

"STATUS.h"
"ENCODER.h"
"RTC_AVR128DA64.h"
"FUNCTIONALITY_INSULIN.h"
"RTC_MAX31342.h"
"EEPROM_AVR128DA64.h"
"DataFrame.h"
"write_eeprom.h"
"MotorCalculation.h"
"EEPROM_AT24C32E.h"
"SystemUI.h"
--------------------------------------------------------------------------------

# Hardware Used
1. AVR128DA64
2. drv8835
3. max31342
4. AT24C32E
5. ch340
6. stepper motor
7. stepper motor encoder
8. temperature sensor
9. buzzer
10. vibration motor
11. boost ic
12. mic

--------------------------------------------------------------------------------

# Getting the source
git@github.com:amardeep-agva/Insulink_V1.0.0.git
