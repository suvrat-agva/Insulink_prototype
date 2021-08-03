#ifndef MOTOR_CALCULATION_H
#define MOTOR_CALCULATION_H

//#define steps_in_revolution 600
#define steps_in_revolution 255
//#define steps_in_revolution 495

char buf[10];

#include <stdio.h>
#include <math.h>

#define PI 3.141592653589793238

//float height = 0.34;
float height = 0.5;
// float radius = 5.875;
float radius = 8.375;

float flow_rate;
float delivery_in_ml;

long motor_steps;
long dose_steps;

// void set_flow_rate(float value)
// {
//   flow_rate = value;
//   USART1_sendString("Flow rate: ");
//   USART1_sendFloat(flow_rate, 4);
// }
// 
// void set_delivery_in_ml(float value)
// {
//   delivery_in_ml = value;
//   USART1_sendString("Dose: ");
//   USART1_sendFloat(delivery_in_ml, 4);
// }

float calculate_steps(float value)
{
  USART1_sendString("Calculating Steps ..");

  float vol_per_rotation_in_ml = 0;
  float steps = 0;

  vol_per_rotation_in_ml = (PI * radius * radius * height);
  USART1_sendString("vol_per_rotation in mm^3 : ");
  USART1_sendFloat(vol_per_rotation_in_ml, 4);
  
  vol_per_rotation_in_ml = vol_per_rotation_in_ml / 1000;      // for conversion mm^3 to ml.

  vol_per_rotation_in_ml = vol_per_rotation_in_ml * 100;    // for conversion based on 100u
 
  steps = (steps_in_revolution / vol_per_rotation_in_ml);
  USART1_sendString("1 unit = ");
  USART1_sendFloat(steps,2);

  steps = (steps_in_revolution / vol_per_rotation_in_ml) * value;
  USART1_sendString("Calculated steps = ");
  USART1_sendFloat(steps, 2);
  return (steps);
}

long configure_dose(float insulin_in_units)
{
  USART1_sendString("Configuring dose");
  long steps = calculate_steps(insulin_in_units);
  
  intToStr(steps,buf,0);
  USART1_sendString("Steps for dose: " );
  USART1_sendString(buf);
  return (steps);
}

/*
   configure_insulin_basal function is used to calculate insulin dose the person wants and find out the delay for one step.
   
   parameters: 
   Starttime: In hours:mins
   Endtime: In hours:mins
   configure_rate : In units/hour.
*/
long configure_insulin_basal(uint16_t StartTime, uint16_t EndTime, float configure_rate)
{
  int minute = 0;
  float TotalTime = 0.0;
  
  int hour = (int)(EndTime / 100) - (int)(StartTime / 100);
  
  if ((StartTime % 100) > (EndTime % 100))
  {
	  minute = (60 - (int)(StartTime % 100)) + (int)(EndTime % 100);
	  TotalTime = hour + (minute / 60.0) - 1.0;
  }
  else
  {
	  minute = (int)(EndTime % 100) - (int)(StartTime % 100);
	  TotalTime = hour + (minute / 60.0);
	  
  }
  
  USART1_sendString("Total Time: ");
  USART1_sendFloat(TotalTime, 2);
  
  float TotalDose = configure_rate * TotalTime;
  USART1_sendString("Total Dose: ");
  USART1_sendFloat(TotalDose, 2);
  
  dose_steps = configure_dose(TotalDose);
  
  float steps = calculate_steps(configure_rate);
  /*
      1 sec = steps_in_sec;
      e.g : let flow rate = 0.1 ml/min
      60 sec  = 0.1ml
       60 sec = calculate_steps(0.1);  // lets say = 1627
       60 sec = 1627 steps
       1 sec = 27 steps
       1000 ms = 27 steps
       1 step_delay = 1000 / 27;
  */
  float steps_delay = 3600000 / steps;
  
  USART1_sendString("steps_delay: ");
  USART1_sendInt(steps_delay );
  return (steps_delay);
}


void configure_insulin_bolus_immediate(float dose)
{
	USART1_sendString("Dose (units):");
	USART1_sendFloat(dose, 2);
	
	dose_steps = configure_dose(dose);
	
	USART1_sendString("Total dose steps: ");
	USART1_sendInt(dose_steps);
}

double configure_insulin_bolus_5min(float dose, float flowrate)
{
	USART1_sendString("Dose (units):");
	USART1_sendFloat(dose, 2);
	
	dose_steps = configure_dose(dose);
	
	USART1_sendString("Total dose steps: ");
	USART1_sendInt(dose_steps);
	
	float steps = calculate_steps(flowrate);
  /*
      1 sec = steps_in_sec;
      e.g : let flow rate = 0.1 ml/min
      60 sec  = 0.1ml
       60 sec = calculate_steps(0.1);  // lets say = 1627
       60 sec = 1627 steps
       1 sec = 27 steps
       1000 ms = 27 steps
       1 step_delay = 1000 / 27;
  */
  double steps_delay = 3600000 / steps;
  
  USART1_sendString("steps_delay: ");
  USART1_sendFloat(steps_delay, 2);
  return (steps_delay);
}

#endif