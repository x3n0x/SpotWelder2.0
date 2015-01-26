//*****************************************************************************
//
// File Name	: 'UIControl.c'
// Title		: Capacitive Discharge spot welder - UI Control Helpers
// Author		: Joe Niven - Copyright (C) 2012 All Rights Reserved
// Created		: 2012-06-16
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

//EEPROM data and settings 
//Basic UI settings
uint8_t		EEMEM ee_UIPREF_BACKLIGHT	= 255;			//Back-light Setting
uint8_t		EEMEM ee_UIPREF_CONTRAST	= 0;			//Contrast Setting
uint8_t		EEMEM ee_UIPREF_ENC_SENSE	= 1;			//Encoder Sensitivity

//Reference to Weld Settings 
extern weldctrl_s_t WeldSettings;

//Internal Variables
static uint8_t UI_encSense = 0;
static uint32_t UI_Activity = 0;
static uint8_t Activity = 1;

//Menu system
static uiObj_struct_t MenuList[_uiMaxMenuObjs + 1];
static uint8_t CurMenuIndex = 1;
static UIObjHandle CurrentUIObj = _uiObjVoidHandle;
static swstatus_s_t InputStates;

//Interrupt Usage Variables
static volatile uint8_t EnChange, EnCount, EnCountRaw, EnPins;

//UI Interrupt Vectors  ****************************************************************

//ISR for PCINT0 (Encoder Handler)
ISR(PCINT1_vect)
{		
	EnPins = _ENCPINS & ( _BV(_ENCA) | _BV(_ENCB) );	//Grab Pin Values
	EnChange = 1;											//Set Change Flag
	if(EnCountRaw++ > UI_encSense) 
	{
		EnCount++;
		EnCountRaw = 0;
	}	
}
//ISR for PCINT2 (Switch Handler)
ISR(PCINT0_vect)
{	
	//SwPins = _SWPINS & (_BV(_SWA)	| _BV(_SWB));			//Grab Pin Values
	//SwChange = 1;											//Set Change Flag
}

//UI object handlers **********************************************************
//Register a UI Object
UIObjHandle uiObj_Register(uiObj_struct_t* NewObj){
	
	memcpy((void*)&MenuList[CurMenuIndex], (const void*)NewObj, sizeof(uiObj_struct_t) );
	MenuList[CurMenuIndex].MyHandle = CurMenuIndex;
		
	return (UIObjHandle)(CurMenuIndex++);
}

//Get a UI Object's Handle
UIObjHandle uiObj_GetHandle(uint8_t reqMenuUID){
	
	uint8_t i = 0;
	UIObjHandle retVal = 255;
	
	for(i = 0; i < CurMenuIndex; i++){
		if(MenuList[i].MenuUID == reqMenuUID){
			retVal = MenuList[i].MyHandle;
			break;
		}
	}
	
	return retVal;
}

//Get an Object by Handle
uiObj_struct_t* uiObj_GetObject(UIObjHandle target){
	
	if((target != 255 ) && (target < _uiMaxMenuObjs) && (target < CurMenuIndex) )
	return &MenuList[(uint8_t)target];
	else
	return 0;
}
//Set a UI Objects Action Function
int uiObj_SetAction(UIObjHandle Handle, uint8_t ActionID, int (*ActFunc)(void)){
	
	int retVal = 0;
	
	if(((uint8_t)Handle < CurMenuIndex) && ((uint8_t)Handle != 255)){
		switch(ActionID){
			case 1:		//SW1 Action
			MenuList[(uint8_t)Handle].Current.ActionFunc1 = ActFunc;
			retVal = 1;
			break;
			case 2:		//SW2 Action
			MenuList[(uint8_t)Handle].Current.ActionFunc2 = ActFunc;
			retVal = 2;
			break;
			case 3:		//SW3 (Both) Action
			MenuList[(uint8_t)Handle].Current.ActionFunc3 = ActFunc;
			retVal = 3;
			break;
			case 4:		//SW1 HOLD Action
			MenuList[(uint8_t)Handle].Current.ActionFunc4 = ActFunc;
			retVal = 4;
			break;
			case 5:		//SW2 HOLD action
			MenuList[(uint8_t)Handle].Current.ActionFunc5 = ActFunc;
			retVal = 5;
			break;
			default:
			MenuList[(uint8_t)Handle].Current.ActionFunc3 = ActFunc;
			retVal = 3;
			break;
		}
		}else{
		retVal = (-1);
	}
	
	return retVal;
	
}
//Set A UI Objects Next
int uiObj_SetNext(UIObjHandle Handle, UIObjHandle NewNext){
	
	int retVal = 0;
	
	if( ((uint8_t)Handle != 255) && ((uint8_t)Handle < CurMenuIndex) ){
		MenuList[(uint8_t)Handle].Next = NewNext;
		retVal = (int)Handle;
		}else{
		retVal = (-1);
	}
	
	return retVal;
}
//Set a UI Objects Prev
int uiObj_SetPrev(UIObjHandle Handle, UIObjHandle NewPrev){
	
	int retVal = 0;
	
	if( ((uint8_t)Handle != 255) && ((uint8_t)Handle < CurMenuIndex) ){
		MenuList[(uint8_t)Handle].Prev = NewPrev;
		retVal = (int)Handle;
		}else{
		retVal = (-1);
	}
	
	return retVal;
	
}
//Run a UI Object's specified Action
int uiObj_RunAction(UIObjHandle Handle, uint8_t ActionID){
	
	int retVal = 0;
	
	if( ((uint8_t)Handle != 255) && ((uint8_t)Handle < CurMenuIndex) ){
		
		switch (ActionID){
			case 1:
			if(MenuList[(uint8_t)Handle].Current.ActionFunc1 != 0){
				MenuList[(uint8_t)Handle].Current.ActionFunc1();
				retVal = 1;
				}else{
				retVal = (-2);
			}
			break;
			
			case 2:
			if(MenuList[(uint8_t)Handle].Current.ActionFunc2 != 0){
				MenuList[(uint8_t)Handle].Current.ActionFunc2();
				retVal = 2;
				}else{
				retVal = (-2);
			}
			break;
			
			case 3:
			if(MenuList[(uint8_t)Handle].Current.ActionFunc3 != 0){
				MenuList[(uint8_t)Handle].Current.ActionFunc3();
				retVal = 3;
				}else{
				retVal = (-2);
			}
			break;
			
			case 4:
			if(MenuList[(uint8_t)Handle].Current.ActionFunc4 != 0){
				MenuList[(uint8_t)Handle].Current.ActionFunc4();
				retVal = 4;
				}else{
				retVal = (-2);
			}
			break;
			
			case 5:
			if(MenuList[(uint8_t)Handle].Current.ActionFunc5 != 0){
				MenuList[(uint8_t)Handle].Current.ActionFunc5();
				retVal = 3;
				}else{
				retVal = (-2);
			}
			break;
			
			default:
			retVal = (-3);
		}
		}else{
		retVal = (-1);
	}
	
	return retVal;
	
}
//Activate a UI Object
int uiObj_Activate(UIObjHandle Handle){
	
	int retVal = 0;
	
	if( ((uint8_t)Handle != 255) && ((uint8_t)Handle < CurMenuIndex) ){
		CurrentUIObj = Handle;
		retVal = (int)Handle;
	}else{
		retVal = (-1);
	}
	
	return retVal;

}
//Draw a UI Objects Previous menu (On Left)
int uiObj_DrawPrev(UIObjHandle Handle){
	
	int retVal = 0;
	uiObj_struct_t* Prev;
	
	
	if( ((uint8_t)Handle != 255) && ((uint8_t)Handle < CurMenuIndex) ){
		
		Prev = uiObj_GetObject(MenuList[(uint8_t)Handle].Prev);
		if(Prev){
			vfdPrintStrXY(Prev->Current.MenuText, Prev->Current.MenuTextLen, 0, 0,_vfdLEFTPage);
			vfdPrintStrXY(Prev->Current.ActionText, Prev->Current.ActionTextLen, 1, 1,_vfdLEFTPage);
			retVal = 1;
		}else{
			retVal = (-2);
		}
		}else{
		retVal = (-1);
	}
	
	return retVal;
	
}
//Draw a UI Objects Next Menu (On Right)
int uiObj_DrawNext(UIObjHandle Handle){
	
	int retVal = 0;
	uiObj_struct_t* Next;
	
	
	if( ((uint8_t)Handle != 255) && ((uint8_t)Handle < CurMenuIndex) ){
		
		Next = uiObj_GetObject(MenuList[(uint8_t)Handle].Next);
						
		if(Next){
			vfdPrintStrXY(Next->Current.MenuText, Next->Current.MenuTextLen, 0, 0,_vfdRIGHTPage);
			vfdPrintStrXY(Next->Current.ActionText, Next->Current.ActionTextLen, 1, 1,_vfdRIGHTPage);
			retVal = 1;
		}else{
			retVal = (-2);
		}
	}else{
		retVal = (-1);
	}
	
	return retVal;
	
}

//UI Control Functions	***************************************************************

//Initialize the UI system
void UI_Init(void)
{
	//Load the user settings from EEPROM
	UI_encSense = 2;
	//UI_encSense = eeprom_read_byte(&ee_UIPREF_ENC_SENSE);
		
	//Enable ISRs and set masks for switch and Encoder detection
	PCMSK1 |= _BV(PCINT1 );					  //Set mask for PCINT0(Encoder A only)
	//PCMSK0 |= (_BV(PCINT4) | _BV(PCINT3));	  //Set mask for PCINT22-23 (Switches)
	
	PCICR  |= _BV(PCIE1 ); // |  _BV(PCIE0) );  //Enable Pin Change Interrupts 0 and 1
	
	UI_Activity = (_UI_ACT_TIMEOUT_MS / _MS_PER_SYSTICK) + GetSysTicks();
}

// Reset the activity Timer
void UI_ResetActivity(void){
	
	UI_Activity = (_UI_ACT_TIMEOUT_MS / _MS_PER_SYSTICK) + GetSysTicks();
	
}

//Get the activity State
uint8_t UI_GetActivity(void){
	
	return Activity;
}

// VFD ScreenSaver
void UI_ScreenSaver(void){

	static uint8_t xPos;
	static uint32_t NextUpdate;
	static int dir;
	
	if(GetSysTicks() > NextUpdate){
		
		xPos += dir;
		
		vfdClr();
		
		vfdGotoXY(xPos, 0);
		vfdSendData(0xFF);
		vfdGotoXY((_vfdNumChars - 1)-xPos, 1);
		vfdSendData(0xFF);
		
		if(xPos == _vfdNumChars - 1) dir = -1;	
		if(xPos == 0) dir = 1;
		
		NextUpdate = (_UI_SCRSAV_TIME_MS / _MS_PER_SYSTICK) + GetSysTicks();
		
	}
		
}

//Process UI input from switches etc.
void UI_ProcessInput(swstatus_s_t * TargetSwStatus)
{
	static uint32_t CurTicks;								//Current System Tick Value
	static uint32_t PressLength;
	
	CurTicks = GetSysTicks();								//Get the latest tick value
		
	if(EnChange)											//If Yes, encoder pin changes?
	{
		EnChange = 0;										//Reset Encoder pin change flag
		
		if((EnPins & _BV(_ENCA)) != 0)					//If Encoder, check to see if rising edge of A
		{
			if((EnPins & _BV(_ENCB)) != 0)				//Is B high?
				TargetSwStatus->encDirection = ENC_DIR_B;	//If so, we are going in direction B
			else
				TargetSwStatus->encDirection = ENC_DIR_A;  //Else, going in direction A

			TargetSwStatus->encCount = EnCount;
			TargetSwStatus->encChange = SW_IsChange;		//Encoder Status has changed
			EnCount = 0;									//Reset the 'raw' count
		}
	}
	
	//Get switch pin state
	TargetSwStatus->NewSwPins = _SWPINS & ( _BV(_SWA) | _BV(_SWB) );
	
	//Has the pin status changed?
	if(TargetSwStatus->NewSwPins != TargetSwStatus->OldSwPins){
		//Switch status has changed since last check 
		PressLength = CurTicks - TargetSwStatus->MySwCount;
		TargetSwStatus->MySwCount = CurTicks;
		//Switch pins changed since last check 
		//What switches were pressed, and what was the duration?
		switch (TargetSwStatus->OldSwPins){
			//Both depressed 
			case (0) :
				TargetSwStatus->swC_Duration = 0;
				if(PressLength >= _UI_SWDURATION_0) TargetSwStatus->swC_Duration = 1;
				if(PressLength >= _UI_SWDURATION_1) TargetSwStatus->swC_Duration = 2;
				if(PressLength >= _UI_SWDURATION_2) TargetSwStatus->swC_Duration = 3;
				TargetSwStatus->swChange = SW_IsChange;
				break;
			//SW 'B' Depressed
			case ( _BV(_SWA) ):
				TargetSwStatus->swB_Duration = 0;
				if(PressLength >= _UI_SWDURATION_0) TargetSwStatus->swB_Duration = 1;
				if(PressLength >= _UI_SWDURATION_1) TargetSwStatus->swB_Duration = 2;
				if(PressLength >= _UI_SWDURATION_2) TargetSwStatus->swB_Duration = 3;
				TargetSwStatus->swChange = SW_IsChange;
				break;
			//SW 'A' Depressed
			case ( _BV(_SWB) ):
				TargetSwStatus->swA_Duration = 0;
				if(PressLength >= _UI_SWDURATION_0) TargetSwStatus->swA_Duration = 1;
				if(PressLength >= _UI_SWDURATION_1) TargetSwStatus->swA_Duration = 2;
				if(PressLength >= _UI_SWDURATION_2) TargetSwStatus->swA_Duration = 3;
				TargetSwStatus->swChange = SW_IsChange;
				break;
			//None depressed
			case ( _BV(_SWA) | _BV(_SWB) ):
				TargetSwStatus->swA_Duration = 0;
				TargetSwStatus->swB_Duration = 0;
				TargetSwStatus->swC_Duration = 0;
				TargetSwStatus->swChange = SW_NoChange;
		}
	}
	
	//Save old switch Pin states for reference later 
	TargetSwStatus->OldSwPins = TargetSwStatus->NewSwPins;

}
//Reset the input states
void UI_ResetInputState(swstatus_s_t* TargetSwStatus){
	
	//Reset Encoder
	TargetSwStatus->encChange  = SW_NoChange;
	TargetSwStatus->encCount = 0;
	
	//Reset Switches 
	TargetSwStatus->swChange = SW_NoChange;
	TargetSwStatus->swA_Duration = 0;
	TargetSwStatus->swB_Duration = 0;
	TargetSwStatus->swC_Duration = 0;
}

// Run the UI
void UI_Service(void){
	
	static uint8_t MenuIsDrawn = 0;
	static UIObjHandle OldUIObj = 255;
	static UIObjHandle LastMenu = 1;
	static uiObj_struct_t *RunningObj;
	static uint32_t LastInput;
	static uint8_t UpdateHome;
		
	//See if we need to Redraw
	if(OldUIObj != CurrentUIObj) MenuIsDrawn = 0;
	//Save Current UI Object
	OldUIObj = CurrentUIObj;
	
	//Check if we need to disable the VFD( idle too long )
	if(GetSysTicks() > UI_Activity){
		vfdSetBright(_vfdBright25);
		Activity = 0;
		UpdateHome = 1;
		MenuIsDrawn = 0;
		UI_ScreenSaver();
	}else{
		vfdSetBright(_vfdBright00);
		Activity = 1;
	}
	
	//Check Switches etc 
	UI_ProcessInput(&InputStates);
		
	if( (CurrentUIObj != _uiObjVoidHandle) && 
	    (CurrentUIObj != _uiObjHomeHandle) ){
		//Set flag to Update Home 
		UpdateHome = 1;
		//Draw the UI Object's Menu etc 
		if(!MenuIsDrawn){
			//Get the new Objects pointer
			RunningObj = uiObj_GetObject(CurrentUIObj);
			//Clear the VFD
			vfdClr();
			//Draw the strings
			vfdPrintStrXY( MenuList[CurrentUIObj].Current.MenuText,
			               MenuList[CurrentUIObj].Current.MenuTextLen,
						   0, 0, _vfdTHISPage);
			vfdPrintStrXY( MenuList[CurrentUIObj].Current.ActionText,
						   MenuList[CurrentUIObj].Current.ActionTextLen,
						   1, 1, _vfdTHISPage);
			//See if we need to draw next arrow
			if(RunningObj->Next != 255){
				vfdGotoXY(8,1);
				vfdSendData((uint8_t)(0b01111110));
			}
			//See if we need to Draw Prev Arrow
			if(RunningObj->Prev != 255){
				vfdGotoXY(7,1);
				vfdSendData((uint8_t)(0b01111111));
			}				
			MenuIsDrawn = 1;
		}
		//Check the switch states and execute the appropriate action
		if(InputStates.swChange == SW_IsChange){
			//Switch status has changed - 
			//Disable welding 
			DisableWeld();			
			//Handle SWA Press
			if(InputStates.swA_Duration == 1){
				if(RunningObj->Current.ActionFunc1 != 0)
					RunningObj->Current.ActionFunc1();
				
				UI_ResetInputState(&InputStates);
			}
			//Handle SWB Press
			if(InputStates.swB_Duration == 1){
				if(RunningObj->Current.ActionFunc2 != 0)
					RunningObj->Current.ActionFunc2();
					
				UI_ResetInputState(&InputStates);
			}
			//Handle SWC (BOTH) Press
			if(InputStates.swC_Duration == 1){
				if(RunningObj->Current.ActionFunc3 != 0)
					RunningObj->Current.ActionFunc3();
					
				UI_ResetInputState(&InputStates);
			}
			//Handle SWA HOLD
			if(InputStates.swA_Duration == 2){
				if(RunningObj->Current.ActionFunc4 != 0)
					RunningObj->Current.ActionFunc4();
					
				UI_ResetInputState(&InputStates);
			}
			//Handle SWB HOLD
			if(InputStates.swB_Duration == 2){
				if(RunningObj->Current.ActionFunc5 != 0)
					RunningObj->Current.ActionFunc5();
					
				UI_ResetInputState(&InputStates);
			}
			
			//Reset the Switch states- Switch has been handled 
			InputStates.swChange = SW_NoChange;
			//Redraw Menu
			MenuIsDrawn = 0;
			//Save time stamp
			LastInput = GetSysTicks();
			//Reset activity
			UI_ResetActivity();
		}
		
		//Check Encoder state and execute the appropriate action
		if(InputStates.encChange){
			//Encoder state has changed 
			//Do we have at least one count?
			if(InputStates.encCount >= 1){
				//Dir A - CW
				if(InputStates.encDirection == ENC_DIR_A){
					//go to Next Menu
					if(uiObj_DrawNext(CurrentUIObj) == 1){
						//Switch to the next Menu
						vfdFlipPageRight(_UI_MENU_SWEEP_TIME);
						//Activate the new Object 
						//_delay_ms(1000);
						uiObj_Activate(RunningObj->Next);
					}else{
						//Menu had no Next
						Beep(50);
					}
				}
				//Dir B - CCW
				if(InputStates.encDirection == ENC_DIR_B){
					//go to Prev Menu
					if(uiObj_DrawPrev(CurrentUIObj) == 1){
						//Switch to the Previous Menu
						vfdFlipPageLeft(_UI_MENU_SWEEP_TIME);
						//Activate the new Object
						//_delay_ms(1000);
						uiObj_Activate(RunningObj->Prev);
					}else{
						//Menu Had No Previous 
						Beep(50);
					}
				}
			}
			//Encoder has been handled - Reset state 
			UI_ResetInputState(&InputStates);
			//Redraw Menu 
			MenuIsDrawn = 0;
			//Save time stamp
			LastInput = GetSysTicks();
			//Reset Activity 
			UI_ResetActivity();
		}
		
		//Check if time to show Home Screen
		if((GetSysTicks() - LastInput) > (_UI_HOME_TIMEOUT_MS / _MS_PER_SYSTICK) ){
			LastMenu = CurrentUIObj;
			uiObj_Activate(_uiObjHomeHandle);
			UI_ForceUpdate();
			Beep(50);
		}
		
		
	}else{
		
		//Are we in Undefined Screen?
		if(CurrentUIObj == _uiObjVoidHandle){
			if(!MenuIsDrawn){
				if(Activity){
					vfdClr();
					vfdPrintStrXY(PSTR("-Micro Weld Pro-"), 16, 0, 0, _vfdTHISPage);
					vfdPrintStrXY(PSTR("      v1.0      "), 16, 0, 1, _vfdTHISPage);
				}
				MenuIsDrawn = 1;
				//Disable welding
				DisableWeld();
			}
		}
		//Are we in Home Screen?
		if(CurrentUIObj == _uiObjHomeHandle){
			//vfdClr();
			//Build Home Screen Status Display
			if(Activity){
				//Enable Welding
				EnableWeld();
				//Update Home Screen
				UI_Status(UpdateHome);
			}else{
				//Disable Welding 
				DisableWeld();
				//Update Home Screen
				//UI_Status(UpdateHome);
			}
			//Check for Buttons or Encoder 
			if((InputStates.encChange) || (InputStates.swChange)){
				//Reset INput state 
				UI_ResetInputState(&InputStates);
				//Reset to Last menu
				if(LastMenu == 0)
					CurrentUIObj = 1;
				else
					CurrentUIObj = LastMenu;
				//Reset input time stamp 
				LastInput = GetSysTicks();
				//Disable Welding 
				DisableWeld();
				//Redraw Menu
				MenuIsDrawn = 0;
				//Reset Activity 
				UI_ResetActivity();
				//Beep(50);
			}	
			
			UpdateHome = 0;		
		}
	}
}

// Draw the Home status screen
void UI_Status(uint8_t Update){
	
	static weldcycle_enum_t CurWeldStage, LastWeldStage;
	static uint8_t trigd;
	
	//Get current weld and trigger states
	CurWeldStage = GetActiveWeldState();
	trigd = IsWeldTriggered();
	
	//Update the status display if needed
	
	if( (CurWeldStage != LastWeldStage) || (Update) ){
		
		vfdPrintStrXY(PSTR("TYPE:    STATUS:"),16, 0, 0, _vfdTHISPage);
		
		//Manual Weld
		if(WeldSettings.Type == wTypeContinuous){
			//Running
			if(CurWeldStage == WeldStage_Run)
				vfdPrintStrXY(PSTR(" MAN       RUN  "), 16, 0, 1, _vfdTHISPage);
			//Waiting
			if(CurWeldStage == WeldStage_Wait){
				if(IsWeldEnabled()){
					vfdPrintStrXY(PSTR(" MAN       RDY! "), 16, 0, 1, _vfdTHISPage);
				}else{
					vfdPrintStrXY(PSTR(" MAN       DIS. "), 16, 0, 1, _vfdTHISPage);
				}
			}
			//Ended
			if(CurWeldStage == WeldStage_End){
				vfdPrintStrXY(PSTR(" MAN       WAIT "), 16, 0, 1, _vfdTHISPage);
			}
			
		}
		
		//Single Pulse Weld
		if(WeldSettings.Type == wTypeSinglePulse){
			//Waiting	
			if( (CurWeldStage == WeldStage_Wait) || (Update) ){
				if(IsWeldEnabled()){
					//Ready to run 
					switch (trigd){
						case 0:
							vfdPrintStrXY(PSTR(" 1_P       RDY! "), 16, 0, 1, _vfdTHISPage);
							break;
						case 1:
						case 2:
							vfdPrintStrXY(PSTR(" 1_P     TRIG'D "), 16, 0, 1, _vfdTHISPage);
							break;
					}
				}else{
					vfdPrintStrXY(PSTR(" 1_P   DISABLED "), 16, 0, 1, _vfdTHISPage);
				}
			//Running
			}else{
				
				if(CurWeldStage == WeldStage_End)
					vfdPrintStrXY(PSTR(" 1_P       WAIT "), 16, 0, 1, _vfdTHISPage);
				else
					vfdPrintStrXY(PSTR(" 1_P       RUN  "), 16, 0, 1, _vfdTHISPage);
			}
			
			
		}
		//Single Pulse Weld
		if(WeldSettings.Type == wTypeDoublePulse){
			if((CurWeldStage == WeldStage_Wait) || (Update) ){
				if(IsWeldEnabled()){
					//Ready to run
					switch (trigd){
						case 0:
							vfdPrintStrXY(PSTR(" 2_P       RDY! "), 16, 0, 1, _vfdTHISPage);
							break;
						case 1:
						case 2:
							vfdPrintStrXY(PSTR(" 2_P     TRIG'D "), 16, 0, 1, _vfdTHISPage);
							break;
					}
				}else{
					vfdPrintStrXY(PSTR(" 2_P   DISABLED "), 16, 0, 1, _vfdTHISPage);
				}
				//Running
			}else{
				
				if(CurWeldStage == WeldStage_End)
					vfdPrintStrXY(PSTR(" 2_P       WAIT "), 16, 0, 1, _vfdTHISPage);
				else
					vfdPrintStrXY(PSTR(" 2_P       RUN  "), 16, 0, 1, _vfdTHISPage);
			}
		}
		
		//Show Trigger Setting
		if(WeldSettings.Trigger == wTrigContact){
			vfdPrintStrXY(PSTR("CT"),2 ,6 ,1, _vfdTHISPage);
		}else{
			vfdPrintStrXY(PSTR("FS"),2 ,6, 1, _vfdTHISPage);
		}
		
		LastWeldStage = CurWeldStage;
		
		UI_ResetActivity();
		
	}
}

//Force a Status display update
void UI_ForceUpdate(void){
	
	UI_Status(1);
}
