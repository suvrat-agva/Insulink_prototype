#ifndef STATUS_H_
#define STATUS_H_

uint8_t BASAL_SYSTEM_STATUS;
uint8_t BOLUS_SYSTEM_STATUS;
uint8_t SYSTEM_STATUS;

bool g_bg_alert = false;

float carbinsulin_ratio = 10.0, Insulinsensitivity = 10.0;
int lowerBGrange = 80, higherBGrange = 100;

float CarbIntake = 0.0, ActiveInsulin = 0.0;

bool Button_BOLUS = false, Button_OK = false, Button_BACK = false, Button_SETTING = false, Button_UP = false, Button_DOWN = false;
uint16_t g_current_basal_time = 0;
bool g_start_basal = false;
bool RTC_interrupt = false;

#define SYSTEM_LOCK                  0x01
#define LCD_BACKLIGHT_OFF            0x02
#define LCD_BACKLIGHT_ON             0x03
#define SYSTEM_LOCK_BACKLIGHT_ON     0x04
#define SYSTEM_LOCK_BACKLIGHT_OFF    0x05
#define SYSTEM_UNLOCK                0x06

#define NO_PROCESS                   0x00
#define BASAL_REQUEST                0x01
#define BASAL_DELIVERY_ON_PROCESS    0x02
#define BASAL_DELIVERY_STOP          0x03
#define BASAL_DELIVERY_INTERRUPTED   0x04
#define BASAL_START_ALARM_SET        0x05
#define BASAL_END_ALARM_SET          0x06
#define BASAL_DELIVERY_START         0x07

#define BOLUS_DELIVERY_ON_PROCESS    0x08
#define BOLUS_DELIVERY_INTERRUPTED   0x09
#define BOLUS_DELIVERY_STOP          0x0A


/* Keypad Section , Changed to ISR accordingly         */
#define BUTTON_OK_DEFINE    (PORTB.PIN2CTRL |= (1 << 3)|(0x3))
#define BUTTON_BOLUS_DEFINE (PORTB.PIN6CTRL |= (1 << 3)|(0x3))
#define BUTTON_BACK_DEFINE  (PORTC.PIN2CTRL |= (1 << 3)|(0x3))
#define BUTTON_SETTING_DEFINE (PORTC.PIN6CTRL |= (1 << 3)|(0x3))
#define BUTTON_UP_DEFINE    (PORTF.PIN2CTRL |= (1 << 3)|(0x3))
#define BUTTON_DOWN_DEFINE  (PORTE.PIN6CTRL |= (1 << 3)|(0x3))

#define BUTTON_OK_CLEAR    (PORTB.PIN2CTRL = 0x00)
#define BUTTON_BOLUS_CLEAR (PORTB.PIN6CTRL = 0x00)
#define BUTTON_BACK_CLEAR  (PORTC.PIN2CTRL = 0x00)
#define BUTTON_SETTING_CLEAR (PORTC.PIN6CTRL = 0x00)
#define BUTTON_UP_CLEAR    (PORTF.PIN2CTRL = 0x00)
#define BUTTON_DOWN_CLEAR  (PORTE.PIN6CTRL = 0x00)

//--------------------------------------------------
#define EEPROM_START_ADDRESS_INDEX 16    // These defines for bolus and basal read
#define EEPROM_END_ADDRESS_INDEX 18
#define EEPROM_LAST_DATE_INDEX 20

#define BOLUS_ADDRESS_MAX 3071
#define BOLUS_ADDRESS_MIN 0

#define GLUCOSE_START_ADDRESS_INDEX  22
#define GLUCOSE_END_ADDRESS_INDEX    24
#define GLUCOSE_LAST_DATE_INDEX      26

#define GLUCOSE_ADDRESS_MAX   4095
#define GLUCOSE_ADDRESS_MIN   3072



//------------------------------------------------
#define EEPROM_CURRENT_ADDRESS_INDEX   EEPROM_END_ADDRESS_INDEX
#define EEPROM_START_DATE_INDEX        EEPROM_START_ADDRESS_INDEX
#define EEPROM_DATE_ADDRESS_INDEX      EEPROM_LAST_DATE_INDEX

#define EEPROM_GLUCO_CURRENT_DATE_INDEX  GLUCOSE_END_ADDRESS_INDEX
#define EEPROM_GLUCO_START_DATE_INDEX    GLUCOSE_START_ADDRESS_INDEX
#define EEPROM_GLUCO_DATE_ADDR_INDEX     GLUCOSE_LAST_DATE_INDEX

#define EEPROM_B_START BOLUS_ADDRESS_MIN
#define EEPROM_B_END   BOLUS_ADDRESS_MAX
#define EEPROM_G_START GLUCOSE_ADDRESS_MIN
#define EEPROM_G_END   GLUCOSE_ADDRESS_MAX
//------------------------------------------------

#define BASAL_ENTRY_INDEX      31
#define BASAL_TOTAL_ENTERIES   30

void bolus(void);
void bolus_option(void);
void basal(void);
void set_next_basal_with_endtime(uint16_t);
void check_bolus(void);
void update_time(void);

#endif