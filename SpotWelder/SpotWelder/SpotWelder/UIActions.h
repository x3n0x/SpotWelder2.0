//*****************************************************************************
//
// File Name	: 'UIActions.h'
// Title		: UI Action Declarations 
// Author		: Joe Niven - Copyright (C) 2014 All Rights Reserved
// Created		: 10/19/2014 11:42:07 PM
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

#ifndef UIACTIONS_H_
#define UIACTIONS_H_

#include <avr/io.h>

//UI Action Defines
#define uiViewDelayMS		2000
#define uiSaveDelayMS		500


//UI Helper functions *********************************************************
//UI Menu initializer
int uiHelper_LoadMenus(void);
//Generic Action to Set a Numeric Parameter
int uiHelper_SetNumericParam(void* Param, uint16_t uBound, uint16_t lBound, uint16_t dVal, uint8_t increment);
//Generic Value Display Routine 
void uiHelper_DisplayNumeric(void* Param, const char* Units, uint8_t lenUnits);

//UI Action function Definitions **********************************************
//Each menu requires at least one action 

//Action to Set P0 Time
int uiAct_SetP0Time(void);
int uiAct_ShowP0Time(void);
//Action to Set P1 Time
int uiAct_SetP1Time(void);
int uiAct_ShowP1Time(void);
//Action to Set IP Time
int uiAct_SetIPTime(void);
int uiAct_ShowIPTime(void);
//Action to Set Trig Delay Time
int uiAct_SetTrigDlyTime(void);
int uiAct_ShowTrigDlyTime(void);
//Action to Set Trig Type
int uiAct_SetTrigType(void);
int uiAct_ShowTrigType(void);
//Action to Set Weld Type
int uiAct_SetWeldType(void);
int uiAct_ShowWeldType(void);
//Action to Set Contact trigger threshold
int uiAct_SetTrigThrsh(void);
int uiAct_ShowTrigThrsh(void);
//Action to set defaults
int uiAct_RestoreDefaults(void);









#endif /* UIACTIONS_H_ */