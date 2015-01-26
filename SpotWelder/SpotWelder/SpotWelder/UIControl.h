//*****************************************************************************
//
// File Name	: 'UIControl.h'
// Title		: Capacitive Discharge spot welder - UI Data Declarations
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

#ifndef UICONTROL_H_
#define UICONTROL_H_

//Setting Limits for Contrast and Back-light
#define _UI_CONTRAST_MAX			64
#define _UI_CONTRAST_MIN			0
#define _UI_BACKLIGHT_MAX			255
#define _UI_BACKLIGHT_MIN			0

//Defines for switch press durations (10's of milliseconds (system ticks))
#define _UI_SWDURATION_0			5			//Press
#define _UI_SWDURATION_1			200			//Hold 1
#define _UI_SWDURATION_2			400			//Hold 2
#define _UI_SWTESTCOUNT				30			//Test count

//UI Behavior
#define _UI_MENU_SWEEP_TIME			200
#define _UI_HOME_TIMEOUT_MS			3000
#define _UI_ACT_TIMEOUT_MS			60000
#define _UI_SCRSAV_TIME_MS			50
#define _UI_MIN_FOOTSW_MS			50
#define _UI_CONTWELD_WARN_INT_MS	500

//Some Constants 
#define _uiObjHomeHandle			0			//Home Menu - Always Zero
#define _uiObjVoidHandle			255			//Undefined Menu - Always 255				
#define _uiMaxMenuObjs				16

//Custom Types for UI control
typedef enum enc_dir_enum_t
	{
		ENC_DIR_A,			//CW
		ENC_DIR_B			//CCW
	} enc_dir_enum_t;
//Switch change Enum	
typedef enum sw_change_enum_t
	{
		SW_NoChange,
		SW_IsChange
	} sw_change_enum_t;


typedef uint8_t UIObjHandle;


//UI Menu Object
typedef struct uiMenuObj_struct_t
	{
		const char*			    MenuText;					//Pointer to Menu Text
		const char*				ActionText;					//Pointer to Action Text
		int						(*ActionFunc1)(void);		//SW1 Action
		int						(*ActionFunc2)(void);		//SW2 Action
		int						(*ActionFunc3)(void);		//SW3 (Both) Action
		int						(*ActionFunc4)(void);		//SW1 Hold Action
		int						(*ActionFunc5)(void);		//SW2 Hold Action
		uint8_t					MenuTextLen;				//Length of Menu Text (Usually 16, Maximum 16)
		uint8_t					ActionTextLen;				//Length of Action Text (Maximum 16) (0, 16 are reserved for Prev and Next Indicators)
		void*					TargetParam;				//Targeted Parameter
	}uiMenuObj_struct_t;

//UI object structure
typedef struct uiObj_struct_t
	{
		UIObjHandle				Prev;						//Previous Menu in chain, if any
		UIObjHandle				Next;						//Next Menu in chain if any
		uiMenuObj_struct_t		Current;					//The current Menu
		UIObjHandle				MyHandle;					//This Objects Handle
		uint8_t					MenuUID;					//This Objects UID
	}uiObj_struct_t;


//UI Switch and input control structure type
typedef struct SwStatusStruct
	{
		sw_change_enum_t swChange;		//Did the switches change recently - since last check?
		uint8_t swA_Duration;			//Duration: either 1, 2, or 3 (Switches can be held) 
		uint8_t swB_Duration;			//4 means timing a press, 0 means no press
		uint8_t swC_Duration;			//'Fake' Switch (Both depressed)
		uint8_t OldSwPins;				//Last Switch Pin state 
		uint8_t NewSwPins;				//Current Switch Pin State 
		uint32_t MySwCount;				//Current Switch Timeer
		sw_change_enum_t encChange;		//Did the encoder change recently - since last check?
		enc_dir_enum_t	encDirection;	//Encoder direction
		uint8_t encCount;				//Encoder Delta since last check	
	}swstatus_s_t;

//Function Prototypes

//UI object handlers **********************************************************
//Register a UI Object
UIObjHandle uiObj_Register(uiObj_struct_t* NewObj);
//Get a UI Object's Handle
UIObjHandle uiObj_GetHandle(uint8_t reqMenuUID);
//Get an Object by Handle
uiObj_struct_t* uiObj_GetObject(UIObjHandle target);
//Set a UI Objects Action Function
int uiObj_SetAction(UIObjHandle Handle, uint8_t ActionID, int (*ActFunc)(void));
//Set A UI Objects Next
int uiObj_SetNext(UIObjHandle Handle, UIObjHandle NewNext);
//Set a UI Objects Prev
int uiObj_SetPrev(UIObjHandle Handle, UIObjHandle NewPrev);
//Run a UI Object's specified Action
int uiObj_RunAction(UIObjHandle Handle, uint8_t ActionID);
//Activate a UI Object
int uiObj_Activate(UIObjHandle Handle);
//Draw a UI Objects Previous menu (On Left)
int uiObj_DrawPrev(UIObjHandle Handle);
//Draw a UI Objects Next Menu (On Right)
int uiObj_DrawNext(UIObjHandle Handle);

//UI control Functions*********************************************************
//Initializes all the parameters for the UI to operate, loads from eeprom
void UI_Init(void);
// VFD ScreenSaver
void UI_ScreenSaver(void);
// Reset the activity Timer
void UI_ResetActivity(void);
//Get the activity State
uint8_t UI_GetActivity(void);
//Process the Switch and encoder signals 
void UI_ProcessInput(swstatus_s_t * TargetSwStatus);
//Reset the input states 
void UI_ResetInputState(swstatus_s_t* TargetSwStatus);
// Run the UI
void UI_Service(void);
// Draw the Home status screen
void UI_Status(uint8_t Update);
//Force a Status display update
void UI_ForceUpdate(void);


#endif /* UICONTROL_H_ */