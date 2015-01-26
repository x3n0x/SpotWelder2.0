//*****************************************************************************
//
// File Name	: 'WeldControl.h'
// Title		: MiniWeld Pro Miniature Spot Welder - Weld control support 
// Author		: Joe Niven - Copyright (C) 2012 All Rights Reserved
// Created		: 10/26/2014 9:39:43 PM
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

#ifndef WELDCTRL_H_
#define WELDCTRL_H_

//Weld defaults
#define _WeldDef_Voltage				3500
#define _WeldDef_P0						250
#define _WeldDef_P1						300
#define _WeldDef_IP						100
#define _WeldDef_TrigDel				100
#define _WeldDef_Trig					0
#define _WeldDef_Type					0
#define _WeldDef_TrigThrs				128

//Min/Max Weld parameters
#define _MINWeldPulseDelay_mS			50
#define _MAXWeldPulseDelay_mS			1000
#define _MINWeldPulseLength_mS			50
#define _MAXWeldPulseLength_mS			10000

#define _INTERWELD_Delay_mS				1000

//ZeroX detection settings 
#define _MAXZeroXLossTime_mS			100

//Weld trigger type enum
typedef enum weldtrigger_e_t
{
	wTrigFootSwitch		=	0,
	wTrigContact		=	1
}weldtrigger_e_t;

//Weld Type Enum
typedef enum weldtype_e_t
{
	wTypeContinuous		=	0,
	wTypeSinglePulse	=	1,
	wTypeDoublePulse	=	2
}weldtype_e_t;

//weld definition Structure 
typedef struct weldctrl_s_t
{
	uint16_t Voltage;
	uint16_t P0_Length;
	uint16_t P1_Length;
	uint16_t IP_Delay;
	uint16_t Trig_Delay;
	weldtrigger_e_t Trigger;
	weldtype_e_t Type;
} weldctrl_s_t;

//Control Functions *********
//Wait for Zero X Detection
uint8_t WaitZeroX(void);
//Prepare welder for operation
void WELD_Init(void);
//Weld servicer - runs weld cycles - call periodically to run weld system
void WELD_Service(void);

//External Access and Helper Functions **********
//Get trigger state 1 = triggered & waiting, 2 = Welding started
uint8_t IsWeldTriggered(void);
//Get Enabled state
uint8_t IsWeldEnabled(void);
//Disable weld
void DisableWeld(void);
//Enable Weld
int EnableWeld(void);
//Get Current Weld Settings
weldctrl_s_t* GetWeldSettings(void);


#endif /* WELDCTRL_H_ */