//*****************************************************************************
//
// File Name	: 'SpotWelder.c'
// Title		: Capacitive Discharge spot welder - Main Code Block
// Author		: Joe Niven - Copyright (C) 2012 All Rights Reserved
// Created		: 2012-06-05
// Revised		: 
// Version		: 1.0
// Target MCU	: Atmel AVR series
//
//*****************************************************************************
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are
//  met:
//  * Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//  * Neither the name of J-Squared nor the
//    names of its contributors may be used to endorse or promote products
//    derived from this software without specific prior written permission.

//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS
//  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
//  TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
//  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
//  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
//  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
//  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
//  OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
//  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
//  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
//  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//*****************************************************************************

#include "SpotWelder.h"

//Variables

//FW and Version Messages (Must be 16 chars long!)
const char FWNameStr[] PROGMEM		= "-Micro Weld Pro-";
const char FWVerMsgStr[] PROGMEM	= "  fw 1.0 c2014  ";
const char FWAuthorStr[] PROGMEM	= "   Joe Niven    ";

PGM_P FWName = FWNameStr;
PGM_P FWVerMsg = FWVerMsgStr;

//Working variables 

//Reference to Global Weld Settings 
extern weldctrl_s_t WeldSettings;
//Reference to DAC Setting 
extern uint8_t ContactTrigLevel;

//Locally visible 
static uint16_t AREF_Calibrated;
//static uint16_t AREF_Offset;

//EEPROM data and Variables

//Basic Weld Settings (Non-Volatile)
uint16_t	EEMEM ee_WELD_VOLTAGE_MV = 3500;	//Weld Voltage in mV
uint16_t	EEMEM ee_WELD_P0_LENGTH	 = 250;		//Weld Pulse 0 Length (mS)
uint16_t	EEMEM ee_WELD_P1_LENGTH  = 300;		//Weld Pulse 1 Length (mS)
uint16_t	EEMEM ee_WELD_IP_DELAY	 = 100;		//Inter-pulse Delay length (mS)
uint16_t	EEMEM ee_WELD_TRIG_DELAY = 1000; 	//Trigger Delay (mS)
uint16_t	EEMEM ee_WELD_TRIGGER	 = 0;		//Weld Trigger Type (See SpotWelder.h for Trigger Type Enum)
uint16_t	EEMEM ee_WELD_TYPE		 = 2;		//Weld Pulse Type

//Analog Calibration (Non-Volatile)
uint16_t	EEMEM ee_AREF_CAL		;			//Calibrated AREF

//DAC Setting (Trigger threshold)
uint8_t		EEMEM ee_DAC_Setting	= 200;		//DAC Setting

//Load settings from EEPROM to SRAM
void LoadSettings(void){
	
	uint16_t TempVal;
//Load the last programmed settings
//Load Voltage for welders that support it	
	if ( (TempVal = eeprom_read_word(&ee_WELD_VOLTAGE_MV)) != 0xffff) 
		WeldSettings.Voltage = TempVal;
	else
		WeldSettings.Voltage = _WeldDef_Voltage;
//Load Pulse 0 Length
	if ( (TempVal = eeprom_read_word(&ee_WELD_P0_LENGTH)) != 0xffff)
		WeldSettings.P0_Length = TempVal;
	else
		WeldSettings.P0_Length = _WeldDef_P0;
//Load Pulse 1 Length		
	if ( (TempVal = eeprom_read_word(&ee_WELD_P1_LENGTH)) != 0xffff)
		WeldSettings.P1_Length = TempVal;
	else
		WeldSettings.P1_Length = _WeldDef_P1;
//Load Inter-pulse Delay	
	if ( (TempVal = eeprom_read_word(&ee_WELD_IP_DELAY)) != 0xffff)
		WeldSettings.IP_Delay = TempVal;
	else
		WeldSettings.IP_Delay = _WeldDef_IP;
//Load Trigger Delay
	if ( (TempVal = eeprom_read_word(&ee_WELD_TRIG_DELAY)) != 0xffff)
		WeldSettings.Trig_Delay = TempVal;
	else
		WeldSettings.Trig_Delay = _WeldDef_TrigDel;
//Load Trigger Type			
	if ( (TempVal = eeprom_read_word(&ee_WELD_TRIGGER)) != 0xffff)
		WeldSettings.Trigger = (weldtrigger_e_t)TempVal;
	else
		WeldSettings.Trigger = (weldtrigger_e_t)_WeldDef_Trig;
//Load Weld Type
	if ( (TempVal = eeprom_read_word(&ee_WELD_TYPE)) != 0xffff){
		WeldSettings.Type = (weldtype_e_t)TempVal;
		//Check Type to see if we need to 'Fix' the trigger
		if(WeldSettings.Type == wTypeContinuous){
			//check if trigger is set to foot switch
			if(WeldSettings.Trigger != wTrigFootSwitch){
				//Fix it if not
				WeldSettings.Trigger = wTrigFootSwitch;
				eeprom_update_word(&ee_WELD_TRIGGER, (uint16_t)wTrigFootSwitch);	
			}
		}
	}else{
		WeldSettings.Type = (weldtype_e_t)_WeldDef_Type;
	}
//Load Cal Value	
	if ( (TempVal = eeprom_read_word(&ee_AREF_CAL)) != 0xffff)
		AREF_Calibrated = TempVal;
	else
		AREF_Calibrated = 5000;
//Load DAC Setting (Contact Trigger Level)	
	if ( (TempVal = eeprom_read_byte(&ee_DAC_Setting)) != 0xffff)
		ContactTrigLevel = TempVal;
	else
		ContactTrigLevel = _WeldDef_TrigThrs;
	
}

void InitializeHardware(void)
{
	//Initialize SPI
	SPI_Init(_SPI_SPEED_FCPU_DIV_2 | _SPI_ORDER_MSB_FIRST | _SPI_SCK_LEAD_FALLING | _SPI_SAMPLE_TRAILING | _SPI_MODE_MASTER);
	//Initialize Timers
	InitializeTimers();
	//Initialize UI System
	UI_Init();
	//Initialize VFD Display
	vfdInit();
	//Initialize GPIO
	GPIO_Init();
	//Start System Timer
	StartSystemTimer();
	//Load Settings from EEPROM
	LoadSettings();
	//Initialize DAC
	MCP48_Init();
	//Build the Menu Tree
	uiHelper_LoadMenus();
	//Initialize Weld System
	WELD_Init();
	//Start Up Beep
	Beep(100);
	//Enable Interrupts
	sei();
}

int main(void)
{
	//Initialize Hardware
	InitializeHardware();
	//Run the UI with no menu - Shows the Title screen
	UI_Service();
	_delay_ms(3000);
	//Activate The First Menu		
	uiObj_Activate(1);
	//Clear the VFD
	vfdClr();
	
	//Main Program Loop
	while(1){
		//Run the UI	
		UI_Service();
		//Run the Welding Control System
		WELD_Service();
	}

}

	

