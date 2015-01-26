//*****************************************************************************
//
// File Name	: 'TimerControl.h'
// Title		: Capacitive Discharge spot welder - System Timer Definitions
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



#ifndef TIMERCONTROL_H_
#define TIMERCONTROL_H_

//Defines ans Settings/constants
#define _TMR0_COUNTS_PER_TICK		143		//Timer 0 counts per system tick:  144 gives approx 10mS ticks @ 14.7456 mHz ps = 1024;
#define _MS_PER_SYSTICK				10
#define _TMR1_COUNTS_PER_TICK		720		//Timer 1 counts: 720 counts ~ 50 mS @ 14.7456 mHz ps = 1024 
#define _MS_PER_WELDTICK			50

//Timer control macros
//Weld timer
#define _StartWeldTimer	            TCCR1B |=  ( _BV(CS12) | _BV(CS10) )
#define _StopWeldTimer				TCCR1B &= ~( _BV(CS12) | _BV(CS11) | (1 <<CS10) )
//System Timer
#define _StartSystemTimer			TCCR0B = _BV(CS02) | (1 <<CS00)
#define _StopSystemTimer			TCCR0B = ~(_BV(CS02) | _BV(CS00))

//Custom Types/ enums
typedef enum systimeractive_enum_t
	{
		SYS_TIMER_STOPPED, 
		SYS_TIMER_ACTIVE
	} systimeractive_enum_t;

//Weld enabled Enum
typedef enum weldenabled_enum_t	
	{
		Weld_NotEnabled,
		Weld_Enabled
	} weldenabled_enum_t;
//Weld cycle state machine indicator
typedef enum weldcycle_enum_t
	{
		WeldStage_Wait,
		WeldStage_Pulse0,
		WeldStage_Delay,
		WeldStage_Pulse1,
		WeldStage_Run,
		WeldStage_End
	} weldcycle_enum_t;
//Weld Type Enum (Single or Dual Pulse)
typedef enum weldtype_enum_t
	{
		WeldType_Single = 1,
		WeldType_Double	= 2,
	} weldtype_enum_t;

//Struct to hold all data about a weld cycle	
typedef struct weldcycle_s_t
	{
		uint16_t Pulse_0_Ticks;
		uint16_t Pulse_1_Ticks;
		uint16_t Delay_0_Ticks;
		weldcycle_enum_t Stage;
		weldtype_enum_t Type;					
	} weldcycle_s_t;

//Initialize and configure both timers; does not start them!	
void InitializeTimers(void);
//Start up the system timer (TIMER0) in CTC Mode
void StartSystemTimer(void);
//Stop the system timer
void StopSystemTimer(void);
//Get the system Tick Count
uint32_t GetSysTicks(void);

//Utility routines 
//Run the beeper
void Beep(uint32_t timeMS);

//Weld Control Routines 
//Start a weld cycle (NewWeldCycle has weld cycle lengths in mS)  Will not work if a cycle is in progress!
void StartWeldCycle(weldcycle_s_t * NewWeldCycle);
//Advance the weld state machine
void SetNextWeldState(void);
//Set the current Weld State
int SetActiveWeldState (weldcycle_enum_t stage);
//Get the current Weld State
weldcycle_enum_t GetActiveWeldState(void);
//Emergency Halt a weld if in progress
void EmergencyHaltWeld(void);

#endif /* TIMERCONTROL_H_ */