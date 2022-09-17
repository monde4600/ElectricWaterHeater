/*
 *
 *  Created on: May 21, 2022
 *      Author: Ahmed El-Gaafrawy
 */

#include "../LIBRARY/stdTypes.h"
#include "../LIBRARY/errorState.h"

#include "../MCAL/DIO/DIO_int.h"
//#include "../MCAL/ADC/ADC_int.h"

#include "../MCAL/GIE/GIE_int.h"
#include "../HAL/two_7seg_decoder/two_7SEG_Int.h"
#include "../MCAL/EXTI/EXTI_config.h"
#include "../MCAL/EXTI/EXTI_int.h"



//#include "../MCAL/TIMER1/TIMER1_config.h"
//#include "../MCAL/TIMER1/TIMER1_int.h"

//#include "../HAL/LCD/LCD_int.h"
//#include "../HAL/Keypad/Keypad_int.h"

//#include "../MCAL/UART/UART_int.h"
#include "../MCAL/TIMER2/TIMER2_int.h"
//#include "util/delay.h"
#include "../HAL/Temp_sensor/Temp_sensor_intr.h"

/*int main()
{
	char data[20];
	UART_enuInit();
	DIO_enuSetPinDirection(DIO_u8GROUP_D , DIO_u8PIN4,DIO_u8OUTPUT);
	DIO_enuSetPinValue(DIO_u8GROUP_D,DIO_u8PIN4,DIO_u8LOW);
	while(1)
	{
		UART_enuRecieveString(data);
		UART_enuSendString(data);
		UART_enuSendChar('\r');
		u8 i = 0;
		u8 on = 0;
		u8 off = 0;
		while(data[i])
		{
			on=0;
			off=0;
			if(data[i]== "Turn led on"[i])	on = 1;
			else if(data[i]== "Turn led off"[i])	off = 1;
			else
			{
				on= 0;
				off=0;
				break;
			}
			i++;
		}
		if (on)		DIO_enuSetPinValue(DIO_u8GROUP_D,DIO_u8PIN4,DIO_u8HIGH);
		else if (off)	DIO_enuSetPinValue(DIO_u8GROUP_D,DIO_u8PIN4,DIO_u8LOW);
	}

}*/

/*int main()
{
	//char data[20];
	UART_enuInit();

	UART_enuSendChar('s');
	UART_enuSendChar('\r');
	UART_enuSendFloot(349643.3548);
	//UART_enuSendChar('l');
}
*/

//void togglepind3(void);
/*void togglepind3()
{
	DIO_enuTogglePinValue(DIO_u8GROUP_D,DIO_u8PIN3);
}

void togglepind4()
{
	DIO_enuTogglePinValue(DIO_u8GROUP_D,DIO_u8PIN4);
}


int main(void)
{
	DIO_enuSetPinDirection(DIO_u8GROUP_D,DIO_u8PIN3,DIO_u8OUTPUT);
	DIO_enuSetPinDirection(DIO_u8GROUP_D,DIO_u8PIN4,DIO_u8OUTPUT);
	TIMER2_enuInit();
	TIMER2_enuEnableInterrupt(TIM2_COMPMATCH_INTER);
	TIMER2_enuEnableInterrupt(TIM2_OVERFLOW_INTER);
	TIMER2_enuSetPWMDuty(70);
	TIMER2_enuCallBackFun(TIM2_COMPMATCH_INTER,togglepind3,NULL);
	TIMER2_enuCallBackFun(TIM2_OVERFLOW_INTER,togglepind4,NULL);

	GIE_enuEnable();
	while(1);

}
*/
#define TepretureADD	0

#define UP_Button	DIO_u8GROUP_D,DIO_u8PIN2
#define DOWN_Button	DIO_u8GROUP_D,DIO_u8PIN3
#define ONOFF_Button	DIO_u8GROUP_B,DIO_u8PIN2
#define Heater_Pin		DIO_u8GROUP_A,DIO_u8PIN1
#define HeaterLed_Pin	DIO_u8GROUP_B,DIO_u8PIN1
#define Coolant_Pin		DIO_u8GROUP_B,DIO_u8PIN0
#define CoolantLed_Pin	DIO_u8GROUP_B,DIO_u8PIN3

#define	TempSetTime			5000		//ms
#define TempReadperiod		100

#define TOP					256

#define PRESCALLER			1024

#define OVFSTempSetCounts	((u64)TempSetTime * (F_CPU/1000)/((u64)PRESCALLER*TOP))
#define TempReadCounts		((u64)TempReadperiod * (F_CPU/1000)/((u64)PRESCALLER*TOP))
#define One_secound			((u64)1000 * (F_CPU/1000)/((u64)PRESCALLER*TOP))
u8 OneSecCounts = One_secound;
u16 TempSetCounter = 0;
u16 GlobalCounter=0;
extern EXTI_t EXTI_AstrEXTIConfig [];
u8 tempre_set = 60;
typedef enum{ON,OFF}Power;

Power Controller_PowerMood=OFF;

void EXTI_ONOFFfun()
{

	if(Controller_PowerMood == OFF)
	{
		EXTI_enuEnableInterrupt(&EXTI_AstrEXTIConfig[0]);
		EXTI_enuEnableInterrupt(&EXTI_AstrEXTIConfig[1]);
		TIMER2_enuEnableInterrupt(TIM2_OVERFLOW_INTER);
		Controller_PowerMood = ON;
	}
	else
	{
		EXTI_enuDisableInterrupt(&EXTI_AstrEXTIConfig[0]);
		EXTI_enuDisableInterrupt(&EXTI_AstrEXTIConfig[1]);
		TIMER2_enuDisableInterrupt(TIM2_OVERFLOW_INTER);
		Controller_PowerMood = OFF;
		S7SEG2_enuSendInteger(0);
		DIO_enuSetPinValue(Heater_Pin,DIO_u8LOW);
		DIO_enuSetPinValue(HeaterLed_Pin,DIO_u8LOW);
		DIO_enuSetPinValue(CoolantLed_Pin,DIO_u8LOW);
		DIO_enuSetPinValue(Coolant_Pin,DIO_u8LOW);
	}
}

void IncreaseTemp()
{


	if(tempre_set<75 && TempSetCounter)
	{
		tempre_set += 5;
	}
	TempSetCounter = OVFSTempSetCounts;
}

void DecreaseTemp()
{

	if(tempre_set>35 && TempSetCounter)
	{
		tempre_set -= 5;
	}
	TempSetCounter = OVFSTempSetCounts;
}

void Timerovfcounter()
{
	if(GlobalCounter)	GlobalCounter--;
	if(OneSecCounts)	OneSecCounts--;
	if(TempSetCounter)
	{
		TempSetCounter--;
		if(!TempSetCounter)
			EEPROM_write(TepretureADD,tempre_set);
	}
}




int main(void)
{

	u16 Current_Temp;
	S7SEG2_enuInit();
	EXTI_enuInit();
	EXTI_enuCallBack(&EXTI_AstrEXTIConfig[0],IncreaseTemp,NULL);
	EXTI_enuCallBack(&EXTI_AstrEXTIConfig[1],DecreaseTemp,NULL);
	EXTI_enuCallBack(&EXTI_AstrEXTIConfig[2],EXTI_ONOFFfun,NULL);
	DIO_enuSetPinDirection(UP_Button,DIO_u8INPUT);
	DIO_enuSetPinDirection(DOWN_Button,DIO_u8INPUT);
	DIO_enuSetPinDirection(ONOFF_Button,DIO_u8INPUT);
	DIO_enuSetPinValue(UP_Button,DIO_u8HIGH);
	DIO_enuSetPinValue(DOWN_Button,DIO_u8HIGH);
	DIO_enuSetPinValue(ONOFF_Button,DIO_u8HIGH);

	DIO_enuSetPinDirection(Heater_Pin,DIO_u8OUTPUT);
	DIO_enuSetPinDirection(HeaterLed_Pin,DIO_u8OUTPUT);
	DIO_enuSetPinValue(Heater_Pin,DIO_u8LOW);
	DIO_enuSetPinValue(HeaterLed_Pin,DIO_u8LOW);

	DIO_enuSetPinDirection(Coolant_Pin,DIO_u8OUTPUT);
	DIO_enuSetPinDirection(CoolantLed_Pin,DIO_u8OUTPUT);
	DIO_enuSetPinValue(CoolantLed_Pin,DIO_u8LOW);
	DIO_enuSetPinValue(Coolant_Pin,DIO_u8LOW);

	TEMP_voidInit();
	TEMP_u16GetValue(&Current_Temp);
	GIE_enuEnable();
	TIMER2_enuInit();
	TIMER2_enuCallBackFun(TIM2_OVERFLOW_INTER,Timerovfcounter,NULL);
	TempSetCounter = 0;
	{
		u8 EpromRead;
		EEPROM_read(TepretureADD,&EpromRead);
		if(EpromRead!=0XFF)		tempre_set=EpromRead;
	}

	u8 TempArray[10]={Current_Temp,Current_Temp,Current_Temp,Current_Temp,Current_Temp,Current_Temp,Current_Temp,Current_Temp,Current_Temp,Current_Temp};
	Controller_PowerMood=OFF;
	EXTI_enuDisableInterrupt(&EXTI_AstrEXTIConfig[0]);
	EXTI_enuDisableInterrupt(&EXTI_AstrEXTIConfig[1]);
	TIMER2_enuDisableInterrupt(TIM2_OVERFLOW_INTER);
	while(1)
	{
		while(Controller_PowerMood == OFF);


		if(TempSetCounter)		S7SEG2_enuSendInteger(tempre_set);
		else			S7SEG2_enuSendInteger(Current_Temp);
		if(!GlobalCounter)
		{
			GlobalCounter=TempReadCounts;
			static u8 TempIndex=0;
			TEMP_u16GetValue(&Current_Temp);
			if(TempIndex == 10 )
				TempIndex = 0;
			TempArray[TempIndex++] =  Current_Temp;

		}
		u16 TempAvarage=(TempArray[0]+TempArray[1]+TempArray[2]+TempArray[3]+TempArray[4]+TempArray[5]+TempArray[6]+TempArray[7]+TempArray[8]+TempArray[9])/10;
		//*((volatile u8*)0X3F)=TempAvarage;
		u8 Coolerpinstate;
		DIO_enuGetPinValue(Coolant_Pin,&Coolerpinstate);
		u8 Heaterpinstate;
		DIO_enuGetPinValue(Heater_Pin,&Heaterpinstate);

		if(TempAvarage < (tempre_set-5))
		{

			if(Heaterpinstate!=DIO_u8HIGH)
			{
				DIO_enuSetPinValue(Heater_Pin,DIO_u8HIGH);
				DIO_enuSetPinValue(Coolant_Pin,DIO_u8LOW);
				DIO_enuSetPinValue(CoolantLed_Pin,DIO_u8LOW);

			}


		}
		else if( TempAvarage > ( tempre_set+5))
		{

			if(Coolerpinstate!=DIO_u8HIGH)
			{
				DIO_enuSetPinValue(Heater_Pin,DIO_u8LOW);
				DIO_enuSetPinValue(Coolant_Pin,DIO_u8HIGH);
				DIO_enuSetPinValue(HeaterLed_Pin,DIO_u8LOW);
			}

		}
		if(!(OneSecCounts)&& Coolerpinstate==DIO_u8HIGH)
		{
			OneSecCounts = One_secound;
			DIO_enuTogglePinValue(CoolantLed_Pin);
		}
		else if(!(OneSecCounts)&&Heaterpinstate==DIO_u8HIGH)
		{
			OneSecCounts = One_secound;
			DIO_enuTogglePinValue(HeaterLed_Pin);
		}


	}


}
