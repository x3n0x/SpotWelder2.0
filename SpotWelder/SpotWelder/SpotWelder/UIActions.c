//*****************************************************************************
//
// File Name	: 'UIActions.c'
// Title		: UI Action Implementations 
// Author		: Joe Niven - Copyright (C) 2014 All Rights Reserved
// Created		: 10/19/2014 11:44:52 PM
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

//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
//  TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
//  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR
//  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
//  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
//  OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
//  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
//  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
//  OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
//  SUCH DAMAGE.
//*****************************************************************************

#include "SpotWelder.h"

//Reference to external Weld settings parameters
extern weldctrl_s_t WeldSettings;
//Reference to external Dac (Trigger threshold) Setting 
extern uint8_t ContactTrigLevel;

extern uint16_t	EEMEM ee_WELD_VOLTAGE_MV;			//Weld Voltage in mV
extern uint16_t	EEMEM ee_WELD_P0_LENGTH	;			//Weld Pulse 0 Length (mS)
extern uint16_t	EEMEM ee_WELD_P1_LENGTH ;			//Weld Pulse 1 Length (mS)
extern uint16_t	EEMEM ee_WELD_IP_DELAY	;			//Inter-pulse Delay length (mS)
extern uint16_t	EEMEM ee_WELD_TRIG_DELAY;			//Trigger Delay (mS)
extern uint16_t	EEMEM ee_WELD_TRIGGER	;			//Weld Trigger Type (See SpotWelder.h for Trigger Type Enum)
extern uint16_t	EEMEM ee_WELD_TYPE		;			//Weld Pulse Type
extern uint8_t	EEMEM ee_DAC_Setting	;			//Contact Trigger Threshold
	
//Local Variables 
static uint8_t MenuIDs[_uiMaxMenuObjs];
static uint8_t MenuCount = 0;
static uint16_t TempVal;

static swstatus_s_t MySwitchStatus;

//Local String Containers
static char DispValue[16];
static char Number[9];

//UI Helper functions *********************************************************
//UI Menu initializer - Each Menu needs an entry
int uiHelper_LoadMenus(void){
	
	static uint8_t MenuIndex = 1;
	
	//Container for initializing menus 
	static uiObj_struct_t* tempObjPtr;
	static uiObj_struct_t tempMenuObj;
	static UIObjHandle tempHandle; 
	
		
	//PO Menu
	tempMenuObj.Prev = 255;  //Previous is Home Screen
	tempMenuObj.Next = 2;
	tempMenuObj.Current.MenuText    = PSTR("Set P0 Length - ");
	tempMenuObj.Current.MenuTextLen = 16;
	tempMenuObj.Current.ActionText  = PSTR("GO...     View");
	tempMenuObj.Current.ActionTextLen = 14;
	tempMenuObj.Current.TargetParam = (void*)&WeldSettings.P0_Length;
	tempMenuObj.Current.ActionFunc1 = &uiAct_SetP0Time;
	tempMenuObj.Current.ActionFunc2 = &uiAct_ShowP0Time;
	
	tempHandle = uiObj_Register(&tempMenuObj);
	MenuIDs[MenuIndex] = (uint8_t) ((0x00000FFF & GetSysTicks()) | (MenuIndex << 4));
	tempObjPtr->MenuUID = MenuIDs[MenuIndex++];
	
	//P1 Menu
	tempMenuObj.Prev = tempHandle;  //Previous is P0 Menu
	tempMenuObj.Next = 3;
	tempMenuObj.Current.MenuText    = PSTR("Set P1 Length - ");
	tempMenuObj.Current.MenuTextLen = 16;
	tempMenuObj.Current.ActionText  = PSTR("GO...     View");
	tempMenuObj.Current.ActionTextLen = 14;
	tempMenuObj.Current.TargetParam = (void*)&WeldSettings.P1_Length;
	tempMenuObj.Current.ActionFunc1 = &uiAct_SetP1Time;
	tempMenuObj.Current.ActionFunc2 = &uiAct_ShowP1Time;
	
	tempHandle = uiObj_Register(&tempMenuObj);
	MenuIDs[MenuIndex] = (uint8_t) ((0x00000FFF & GetSysTicks()) | (MenuIndex << 4));
	tempObjPtr->MenuUID = MenuIDs[MenuIndex++];
	
	//IP Menu
	tempMenuObj.Prev = tempHandle;  //Previous is P1 Menu
	tempMenuObj.Next = 4;
	tempMenuObj.Current.MenuText    = PSTR("Set IP Length - ");
	tempMenuObj.Current.MenuTextLen = 16;
	tempMenuObj.Current.ActionText  = PSTR("GO...     View");
	tempMenuObj.Current.ActionTextLen = 14;
	tempMenuObj.Current.TargetParam = (void*)&WeldSettings.IP_Delay;
	tempMenuObj.Current.ActionFunc1 = &uiAct_SetIPTime;
	tempMenuObj.Current.ActionFunc2 = &uiAct_ShowIPTime;
	
	tempHandle = uiObj_Register(&tempMenuObj);
	MenuIDs[MenuIndex] = (uint8_t) ((0x00000FFF & GetSysTicks()) | (MenuIndex << 4));
	tempObjPtr->MenuUID = MenuIDs[MenuIndex++];
	
	//Trig Delay Menu
	tempMenuObj.Prev = tempHandle;  //Previous is IP Menu
	tempMenuObj.Next = 5;
	tempMenuObj.Current.MenuText    = PSTR("Set Trig Delay -");
	tempMenuObj.Current.MenuTextLen = 16;
	tempMenuObj.Current.ActionText  = PSTR("GO...     View");
	tempMenuObj.Current.ActionTextLen = 14;
	tempMenuObj.Current.TargetParam = (void*)&WeldSettings.Trig_Delay;
	tempMenuObj.Current.ActionFunc1 = &uiAct_SetTrigDlyTime;
	tempMenuObj.Current.ActionFunc2 = &uiAct_ShowTrigDlyTime;
	
	tempHandle = uiObj_Register(&tempMenuObj);
	MenuIDs[MenuIndex] = (uint8_t) ((0x00000FFF & GetSysTicks()) | (MenuIndex << 4));
	tempObjPtr->MenuUID = MenuIDs[MenuIndex++];
	
	//Trig Type Menu
	tempMenuObj.Prev = tempHandle;  //Previous is Trig Delay Menu
	tempMenuObj.Next = 6;
	tempMenuObj.Current.MenuText    = PSTR("Set Trig Type - ");
	tempMenuObj.Current.MenuTextLen = 16;
	tempMenuObj.Current.ActionText  = PSTR("GO...     View");
	tempMenuObj.Current.ActionTextLen = 14;
	tempMenuObj.Current.TargetParam = (void*)&WeldSettings.Trig_Delay;
	tempMenuObj.Current.ActionFunc1 = &uiAct_SetTrigType;
	tempMenuObj.Current.ActionFunc2 = &uiAct_ShowTrigType;
	
	tempHandle = uiObj_Register(&tempMenuObj);
	MenuIDs[MenuIndex] = (uint8_t) ((0x00000FFF & GetSysTicks()) | (MenuIndex << 4));
	tempObjPtr->MenuUID = MenuIDs[MenuIndex++];
	
	//Trigger Threshold Level
	tempMenuObj.Prev = tempHandle;  //Previous is Trig Type Menu
	tempMenuObj.Next = 7;
	tempMenuObj.Current.MenuText    = PSTR("Set Trig Level -");
	tempMenuObj.Current.MenuTextLen = 16;
	tempMenuObj.Current.ActionText  = PSTR("GO...     View");
	tempMenuObj.Current.ActionTextLen = 14;
	tempMenuObj.Current.TargetParam = (void*)&ContactTrigLevel;
	tempMenuObj.Current.ActionFunc1 = &uiAct_SetTrigThrsh;
	tempMenuObj.Current.ActionFunc2 = &uiAct_ShowTrigThrsh;
	
	tempHandle = uiObj_Register(&tempMenuObj);
	MenuIDs[MenuIndex] = (uint8_t) ((0x00000FFF & GetSysTicks()) | (MenuIndex << 4));
	tempObjPtr->MenuUID = MenuIDs[MenuIndex++];
	
	//Weld Type Menu
	tempMenuObj.Prev = tempHandle;  //Previous is Trig Type Menu
	tempMenuObj.Next = 8;
	tempMenuObj.Current.MenuText    = PSTR("Set Weld Type - ");
	tempMenuObj.Current.MenuTextLen = 16;
	tempMenuObj.Current.ActionText  = PSTR("GO...     View");
	tempMenuObj.Current.ActionTextLen = 14;
	tempMenuObj.Current.TargetParam = (void*)&WeldSettings.Trig_Delay;
	tempMenuObj.Current.ActionFunc1 = &uiAct_SetWeldType;
	tempMenuObj.Current.ActionFunc2 = &uiAct_ShowWeldType;
	
	tempHandle = uiObj_Register(&tempMenuObj);
	MenuIDs[MenuIndex] = (uint8_t) ((0x00000FFF & GetSysTicks()) | (MenuIndex << 4));
	tempObjPtr->MenuUID = MenuIDs[MenuIndex++];
		
	//Reset Defaults 
	tempMenuObj.Prev = tempHandle;  //Previous is Weld Type Menu
	tempMenuObj.Next = _uiObjVoidHandle;
	tempMenuObj.Current.MenuText    = PSTR("Reset Defaults -");
	tempMenuObj.Current.MenuTextLen = 16;
	tempMenuObj.Current.ActionText  = PSTR("PUSH      BOTH");
	tempMenuObj.Current.ActionTextLen = 14;
	tempMenuObj.Current.ActionFunc1 = 0;
	tempMenuObj.Current.ActionFunc2 = 0;
	tempMenuObj.Current.ActionFunc3 = &uiAct_RestoreDefaults;
		
	tempHandle = uiObj_Register(&tempMenuObj);
	MenuIDs[MenuIndex] = (uint8_t) ((0x00000FFF & GetSysTicks()) | (MenuIndex << 4));
	tempObjPtr->MenuUID = MenuIDs[MenuIndex++];
	/*
	
	//Hook all menus to their Next Menu
	//Save Count and Reset Index
	MenuCount = MenuIndex;
	MenuIndex = 0;
	//Start with first registered and work through 
	//P0 Menu Next
	tempObjPtr = uiObj_GetObject(MenuIDs[MenuIndex++]);
	tempObjPtr->Next = uiObj_GetHandle(MenuIDs[MenuIndex]);
	//P1 Menu Next
	tempObjPtr = uiObj_GetObject(MenuIDs[MenuIndex++]);
	tempObjPtr->Next = uiObj_GetHandle(MenuIDs[MenuIndex]);
	//IP Menu Next 
	tempObjPtr = uiObj_GetObject(MenuIDs[MenuIndex++]);
	tempObjPtr->Next = uiObj_GetHandle(MenuIDs[MenuIndex]);
	//Trig Delay Next 
	tempObjPtr = uiObj_GetObject(MenuIDs[MenuIndex++]);
	tempObjPtr->Next = uiObj_GetHandle(MenuIDs[MenuIndex]);
	//Trig Type Next
	tempObjPtr = uiObj_GetObject(MenuIDs[MenuIndex++]);
	tempObjPtr->Next = uiObj_GetHandle(MenuIDs[MenuIndex]); 
	//Weld Type Next 
	tempObjPtr = uiObj_GetObject(MenuIDs[MenuIndex++]);
	tempObjPtr->Next = uiObj_GetHandle(MenuIDs[MenuIndex]);
	//Reset defaults is the Last one - No Next Needed!
	*/
	return MenuCount;
		
}
//Generic Action to Set a Numeric Parameter
int uiHelper_SetNumericParam(void* Param, uint16_t uBound, uint16_t lBound, uint16_t dVal, uint8_t increment){
	
	static uint8_t DoneEdit, Change;
	static uint16_t NewVal, CurVal;
	
	int retVal = 0;
	
	static uint8_t numLen, start, pos;
		  
	//Set the default and Current Values first 
	CurVal = *(uint16_t*)Param;
	//Check current setting to see if in range
	if(CurVal < lBound) CurVal = lBound;
		NewVal = CurVal + 1;
	//Reset done Editing Flag
	DoneEdit = Change = 0;
	//Reset change indicator
	
	//edit loop 
	while (!DoneEdit){
		//Check switches 
		UI_ProcessInput(&MySwitchStatus);
		//Has value changed since last iteration?
		if(CurVal != NewVal){
			//Save the NewValue
			NewVal = CurVal;
			//Clear out display string 
			memset((void*)DispValue, 0x20, 16);
			//generate string of new value
			utoa(NewVal, Number, 10);
			numLen = strlen(Number);
			start = 8 - (numLen / 2);
			pos = 0;
			//Copy Number to display String 
			do{
				numLen --;
				DispValue[start++] = Number[pos++];
			} while (numLen);
			//Insert less indicator if still possible
			//if(CurVal > lBound) DispValue[7] = 0b00011110;
			//Insert more indicator if still possible
			//if(CurVal < uBound) DispValue[8] = 0b00011101;
			//Display the Action Caption
			if(Change)
				vfdPrintStrXY(PSTR(" Save      Exit "), 16, 0, 1, _vfdTHISPage);
			else
				vfdPrintStrXY(PSTR("           Exit "), 16, 0, 1, _vfdTHISPage);
			//Insert less indicator if still possible
			if(CurVal > lBound) {
				vfdGotoXY(7,1);
				vfdSendData(0b00011110);
			}
			//Insert more indicator if still possible
			if(CurVal < uBound){
				vfdGotoXY(8,1);
				vfdSendData(0b00011101);
			}
			//Display the Value
			vfdCopyStr(DispValue, 16, 0, 0);			
		}
		//Decide what to do about input
		//Did encoder change?
		if(MySwitchStatus.encChange == SW_IsChange){
			if(MySwitchStatus.encCount){
				//CW Direction - Add
				if(MySwitchStatus.encDirection == ENC_DIR_A){
					do{
						if(CurVal < uBound) CurVal += increment;
						MySwitchStatus.encCount--;		
					} while (MySwitchStatus.encCount);
					//Value Changed
					Change = 1;
				}
				//CCW Direction - Subtract
				if(MySwitchStatus.encDirection == ENC_DIR_B){
					do{
						if(CurVal > lBound) CurVal -= increment;
						MySwitchStatus.encCount--;
					} while (MySwitchStatus.encCount);
					//Value Changed 
					Change = 1;
				}
			}  //end encoder count check 
			
			//Reset status
			MySwitchStatus.encChange = SW_NoChange;
		}
		//Did switches change?
		if(MySwitchStatus.swChange == SW_IsChange){
			//Switch 1 was pressed - Save the New Value
			if(MySwitchStatus.swA_Duration == 1){
				//NewVal already contains the new value 
				if(Change){
					//Value has changed
					retVal = 1;
					//Exit edit loop
					DoneEdit = 1;
				}
			}
			//Switch 2 was pressed - No Change
			if(MySwitchStatus.swB_Duration == 1){
				NewVal = *(uint16_t*)Param;
				//No change to value 
				retVal = 0;
				//Exit edit Loop
				DoneEdit = 1;
			}
			//Switch 3(both) was pressed - default 
			if(MySwitchStatus.swC_Duration == 1){
				//Indicate status 
				vfdClr();
				vfdPrintStrXY(PSTR("Setting Default!"), 16, 0, 0, _vfdTHISPage);
				_delay_ms(uiSaveDelayMS);
				vfdClr();
				//Set default value 
				CurVal = dVal;
				NewVal = CurVal + 1;
				//Value has changed 
				Change = 1;
			}
			//Reset Status 
			MySwitchStatus.swChange = SW_NoChange;
		} //end switch change 
	} //end edit loop 
	
	//indicate status to User...
	vfdClr();
	
	switch (retVal){
		case 0:
			vfdPrintStrXY(PSTR("  No Change...  "), 16, 0, 0, _vfdTHISPage);
			break;
		case 1:
			vfdPrintStrXY(PSTR("Saving Value ..."), 16, 0, 0, _vfdTHISPage);
			break;
	}
	//Delay
	_delay_ms(uiSaveDelayMS);
	
	vfdClr();
	
	UI_ResetActivity();
		  
	//Save New Value to pointer
	memcpy(Param, (void*)&NewVal, sizeof(NewVal));
	
	return retVal;
}
//Generic Display a Numeric parameter 
void uiHelper_DisplayNumeric(void* Param, const char* Units, uint8_t lenUnits){
	
	static uint8_t numLen, start, pos;
	
	//Get value 
	TempVal = *(uint16_t*)Param;
	//Clear out display string
	memset((void*)DispValue, 0x20, 16);
	//generate string of new value
	utoa(TempVal, Number, 10);
	numLen = strlen(Number);
	start = 8 - ((numLen + lenUnits) / 2);
	pos = 0;
	//Copy Number to display String
	do{
		numLen --;
		DispValue[start++] = Number[pos++];
	} while (numLen);
	vfdClr();
	//Display the Value
	vfdCopyStr(DispValue, 16, 0, 0);
	//Display the units
	vfdPrintStrXY(Units, lenUnits, start+1, 0, _vfdTHISPage);
	
	_delay_ms(uiViewDelayMS);
		
	UI_ResetActivity();
	
	vfdClr();
	
}

//UI Action function Definitions **********************************************
//Each menu requires at least one action

//Action to Set P0 Time
int uiAct_SetP0Time(void){
	
	TempVal = WeldSettings.P0_Length;
		
	if( uiHelper_SetNumericParam(&TempVal, 
	    _MAXWeldPulseLength_mS,
		_MINWeldPulseLength_mS,
		_WeldDef_P0,
		_MINWeldPulseLength_mS) )
		{
			WeldSettings.P0_Length = TempVal;							 
			eeprom_update_word(&ee_WELD_P0_LENGTH, TempVal);
		}

	return 0;

}
int uiAct_ShowP0Time(void){
	
	uiHelper_DisplayNumeric(&WeldSettings.P0_Length, PSTR("ms"), 2);
	return 0;
	
}
//Action to Set P1 Time
int uiAct_SetP1Time(void){
	
	TempVal = WeldSettings.P1_Length;
	
	if( uiHelper_SetNumericParam(&TempVal,
	_MAXWeldPulseLength_mS,
	_MINWeldPulseLength_mS,
	_WeldDef_P1,
	_MINWeldPulseLength_mS) )
	{
		WeldSettings.P1_Length = TempVal;
		eeprom_update_word(&ee_WELD_P1_LENGTH, TempVal);
	}

	return 0;
	
}
int uiAct_ShowP1Time(void){
	
	uiHelper_DisplayNumeric(&WeldSettings.P1_Length, PSTR("ms"), 2);
	return 0;
	
}
//Action to Set IP Time
int uiAct_SetIPTime(void){
	
	TempVal = WeldSettings.IP_Delay;
	
	if( uiHelper_SetNumericParam(&TempVal,
	_MAXWeldPulseDelay_mS,
	_MINWeldPulseDelay_mS,
	_WeldDef_IP,
	_MINWeldPulseDelay_mS) )
	{
		WeldSettings.IP_Delay = TempVal;
		eeprom_update_word(&ee_WELD_IP_DELAY, TempVal);
	}

	return 0;
	
}
int uiAct_ShowIPTime(void){
	
	uiHelper_DisplayNumeric(&WeldSettings.IP_Delay, PSTR("ms"), 2);
	return 0;
	
}
//Action to Set Trig Delay Time
int uiAct_SetTrigDlyTime(void){
	
	TempVal = WeldSettings.IP_Delay;
	
	if( uiHelper_SetNumericParam(&TempVal,
	_MAXWeldPulseDelay_mS,
	_MINWeldPulseDelay_mS,
	_WeldDef_TrigDel,
	_MINWeldPulseDelay_mS) )
	{
		WeldSettings.Trig_Delay = TempVal;
		eeprom_update_word(&ee_WELD_TRIG_DELAY, TempVal);
	}

	return 0;
	
}
int uiAct_ShowTrigDlyTime(void){
	
	uiHelper_DisplayNumeric(&WeldSettings.Trig_Delay, PSTR("ms"), 2);
	return 0;
	
}
//Action to Set Trig Type
int uiAct_SetTrigType(void){
	
	weldtrigger_e_t NewTrig, CurTrig;
	
	uint8_t SaveSetting = 1;
		
	//Reset the input state 
	UI_ResetInputState(&MySwitchStatus);
		
	//Set Current Value 
	if(WeldSettings.Trigger == wTrigContact){
		CurTrig = wTrigContact;
		NewTrig = wTrigFootSwitch;
	}else{
		CurTrig = wTrigFootSwitch;
		NewTrig = wTrigContact;
	}
	
	//Edit loop
	while(1){
		//Check switch States
		UI_ProcessInput(&MySwitchStatus);
		//Has value Changed?
		if(NewTrig != CurTrig){
			//Save new Value 
			NewTrig = CurTrig;
			//Display Value...
			if(NewTrig == wTrigContact)
				vfdPrintStrXY(PSTR("Contact Det Trig"), 16, 0, 0, _vfdTHISPage);
			else
				vfdPrintStrXY(PSTR("Foot-switch Trig"), 16, 0, 0, _vfdTHISPage);
			//Display action Caption
			vfdPrintStrXY(PSTR("Save            "), 16, 0, 1, _vfdTHISPage);
		}
		
		//Check switch status 
		//check encoder
		if(MySwitchStatus.encChange == SW_IsChange){
			//Encoder state changed
			if(MySwitchStatus.encCount >= 1){
				if(CurTrig == wTrigContact)
					CurTrig = wTrigFootSwitch;
				else
					CurTrig = wTrigContact;
			}
			//Reset status 
			UI_ResetInputState(&MySwitchStatus);
		}
		
		//Check switch
		if(MySwitchStatus.swChange == SW_IsChange){
			//Switch state changed
			if((MySwitchStatus.swA_Duration == 1) ||
			   (MySwitchStatus.swA_Duration == 2) ){
				//Switch was pressed
				//Check if Trigger type is allowed for Weld mode
				if(NewTrig == wTrigContact){
					//Check Weld Type Setting
					if(WeldSettings.Type == wTypeContinuous){
						//Contact Trigger not allowed for Continuous mode
						vfdClr();
						vfdPrintStrXY(PSTR("Invalid  for"), 12, 2,0, _vfdTHISPage);
						vfdPrintStrXY(PSTR("Continuous Mode!"), 16, 0, 1, _vfdTHISPage);
						_delay_ms(uiViewDelayMS);
						vfdClr();
						//Set trigger to foot switch
						NewTrig = wTrigFootSwitch;
						//Clear save flag
						SaveSetting = 0;
					}
				}
				//check if we need to save
				if(SaveSetting){
					WeldSettings.Trigger = NewTrig;
					eeprom_update_word(&ee_WELD_TRIGGER, (uint16_t)NewTrig);
					//indicate to user
					vfdClr();
					vfdPrintStrXY(PSTR("Setting Saved..."), 16, 0, 0, _vfdTHISPage);
					_delay_ms(uiSaveDelayMS);
					vfdClr();
				}
				
				break;	
			}
				
			//Reset status
			UI_ResetInputState(&MySwitchStatus);
		}
	}
	
	return 0;
}
int uiAct_ShowTrigType(void){
	
	vfdClr();
	//Display Value...
	if(WeldSettings.Trigger == wTrigContact)
		vfdPrintStrXY(PSTR("Contact Det Trig"), 16, 0, 0, _vfdTHISPage);
	else
		vfdPrintStrXY(PSTR("Foot-switch Trig"), 16, 0, 0, _vfdTHISPage);
	
	_delay_ms(uiViewDelayMS);
	
	vfdClr();
	
	return 0;
	
}
//Action to Set Weld Type
int uiAct_SetWeldType(void){
	
	weldtype_e_t NewWeld, CurWeld;
	
	UI_ResetInputState(&MySwitchStatus);
	
	//Set Current Value
	if(WeldSettings.Type == wTypeContinuous){
		CurWeld = wTypeContinuous;
		NewWeld = wTypeDoublePulse;
	}
	if( WeldSettings.Type == wTypeDoublePulse){
		CurWeld = wTypeDoublePulse;
		NewWeld = wTypeContinuous;
	}
	if( WeldSettings.Type == wTypeSinglePulse){
		CurWeld = wTypeSinglePulse;
		NewWeld = wTypeContinuous;
	}
	
	//Edit loop
	while(1){
		//Check switch States
		UI_ProcessInput(&MySwitchStatus);
		//Has value Changed?
		if(NewWeld != CurWeld){
			//Save new Value
			NewWeld = CurWeld;
			//Display Value...
			if(NewWeld == wTypeContinuous)
				vfdPrintStrXY(PSTR("  Manual Weld   "), 16, 0, 0, _vfdTHISPage);
			if(NewWeld == wTypeDoublePulse)
				vfdPrintStrXY(PSTR(" Dbl Pulse Weld "), 16, 0, 0, _vfdTHISPage);
			if(NewWeld == wTypeSinglePulse)
			    vfdPrintStrXY(PSTR(" Sgl Pulse Weld "), 16, 0, 0, _vfdTHISPage);
			//Display action Caption
			vfdPrintStrXY(PSTR("Save            "), 16, 0, 1, _vfdTHISPage);
		}
		
		//Check switch status
		//check encoder
		if(MySwitchStatus.encChange == SW_IsChange){
			//Encoder state changed
			if(MySwitchStatus.encCount >= 1){
				if(CurWeld == wTypeContinuous){
					CurWeld = wTypeSinglePulse;
				}
				else if (CurWeld == wTypeSinglePulse){
					CurWeld = wTypeDoublePulse;
				}
				else if (CurWeld == wTypeDoublePulse){
					CurWeld = wTypeContinuous;
				}
			}
			//Reset status
			UI_ResetInputState(&MySwitchStatus);
		}
		
		//Check switch
		if(MySwitchStatus.swChange == SW_IsChange){
			//Switch state changed
			if((MySwitchStatus.swA_Duration == 1) ||
			   (MySwitchStatus.swA_Duration == 2) ){
				//Switch was pressed
				//Save New Weld Type
				WeldSettings.Type = NewWeld;
				//If Continuous (Manual) is selected, 
				// Set the foot-switch as the trigger
				if(NewWeld == wTypeContinuous){ 
					vfdClr();
					vfdPrintStrXY(PSTR("Setting Trig To "),16,0 ,0 ,_vfdTHISPage);
					vfdPrintStrXY(PSTR("Foot  Sw"),8 ,4 ,1 , _vfdTHISPage);
					_delay_ms(uiViewDelayMS);
					WeldSettings.Trigger = wTrigFootSwitch;
					eeprom_update_word(&ee_WELD_TYPE, (uint16_t)NewWeld);
				}
				//Save New Weld Type
				eeprom_update_word(&ee_WELD_TRIGGER, (uint16_t)wTrigFootSwitch);
				//indicate to user
				vfdClr();
				vfdPrintStrXY(PSTR("Setting Saved..."), 16, 0, 0, _vfdTHISPage);
				_delay_ms(uiSaveDelayMS);
				vfdClr();
				
				//Reset weld state
				SetActiveWeldState(WeldStage_Wait);
				
				break;
			}
			//Reset status
			UI_ResetInputState(&MySwitchStatus);
		}
	}
	
	return 0;
	
}
int uiAct_ShowWeldType(void){
	
	vfdClr();
	//Display Value...
	if(WeldSettings.Type == wTypeContinuous)
		vfdPrintStrXY(PSTR("  Manual Weld   "), 16, 0, 0, _vfdTHISPage);
	if(WeldSettings.Type == wTypeDoublePulse)
		vfdPrintStrXY(PSTR(" Dbl Pulse Weld "), 16, 0, 0, _vfdTHISPage);
	if(WeldSettings.Type == wTypeSinglePulse)
		vfdPrintStrXY(PSTR(" Sgl Pulse Weld "), 16, 0, 0, _vfdTHISPage);
	
	_delay_ms(uiViewDelayMS);
	
	vfdClr();
	
	return 0;
}
//Action to Set Contact trigger threshold
int uiAct_SetTrigThrsh(void){
	
	TempVal = (uint16_t)(ContactTrigLevel * 16);
	
	if( uiHelper_SetNumericParam(&TempVal,
								 4096,
								 64,
								 (16*_WeldDef_TrigThrs),
								 16) )
	{
		ContactTrigLevel = (TempVal / 16);
		eeprom_update_byte(&ee_DAC_Setting, ContactTrigLevel);
		MCP48_SetValue(ContactTrigLevel, _MCP48_GAIN_2);
	}

	return 0;
}

//Show the Trigger threshold 
int uiAct_ShowTrigThrsh(void){
	
	TempVal = (uint16_t)( 16 * ContactTrigLevel);
	
	uiHelper_DisplayNumeric((uint16_t*)&TempVal,PSTR("mV"), 2);
	return 0; 
	
}

//Action to set defaults
int uiAct_RestoreDefaults(void){
	
	UI_ResetInputState(&MySwitchStatus);
	
	WeldSettings.P0_Length = _WeldDef_P0;
	WeldSettings.P1_Length = _WeldDef_P1;
	WeldSettings.IP_Delay = _WeldDef_IP;
	WeldSettings.Trig_Delay = _WeldDef_TrigDel;
	WeldSettings.Trigger = wTrigFootSwitch;
	WeldSettings.Type = wTypeContinuous;
	
	vfdClr();
	vfdPrintStrXY(PSTR(" Defaults  Set! "), 16, 0, 0, _vfdTHISPage);
	_delay_ms(uiSaveDelayMS);
	vfdClr();

	return 0;
}

