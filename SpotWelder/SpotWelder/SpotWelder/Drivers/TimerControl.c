//*****************************************************************************
//
// File Name	: 'TimerControl.c'
// Title		: Capacitive Discharge spot welder - System Timer Helper functions
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

//AVR LIB-C includes
#include <avr/io.h>
#include <avr/sfr_defs.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

//SpotWelder Defines 
#include "../SpotWelder.h"

//Working Variables

//System Tick Counter 
static volatile uint32_t SysTicks;

//Weld control Variables
static volatile weldenabled_enum_t SysWeldEnabler;
static volatile uint16_t NextToggle;
static volatile weldcycle_s_t ActiveWeldCycle; 
static volatile uint16_t WeldTicks;

static systimeractive_enum_t SysTimerActive;
static uint8_t BeepActive = 0;
static uint32_t BeepStart = 0;
static uint32_t BeepTime  = 0;

//System Tick ISR (Timer 0 compare match A interrupt)
ISR(TIMER0_COMPA_vect )
{
	SysTicks++;													//Increment System tick counter
	
	if (BeepActive){
		if((SysTicks - BeepStart) > BeepTime){
			BeepActive = 0;
			_BEEP_OFF;
		}
	}
	
	//_GPIOWeld_TGL;
}
//Weld Cycle Timer (Timer 1 Compare match B interrupt)
ISR(TIMER1_COMPA_vect )
{
	//Time to cycle state machine?
	if(WeldTicks++ == NextToggle){ 
		if(SysWeldEnabler == Weld_Enabled) SetNextWeldState();										//Set proper state in weld state machine
	}
}

//Initialize and configure both timers; does not start them!
void InitializeTimers(void)
{
	SysWeldEnabler = Weld_NotEnabled;							//Make sure weld timer is disabled
	ActiveWeldCycle.Stage = WeldStage_Wait;						//Set initial state of weld state machine
	
	//Timer 1
	_StopWeldTimer;												//Make sure timer is stopped
	TCNT1  = 0;													//Clear Timer1 Count
	OCR1A  = _TMR1_COUNTS_PER_TICK;								//Set initial Compare match value (counts per tick constant => 1 tick)
	TCCR1B = _BV(WGM12);										//Set CTC mode with top=OCR1A
	TIFR1 |= _BV(OCIE1A);// | _BV(ICIE1);						//Ensure clear interrupt flags
 	TIMSK1 = _BV(OCIE1A);// | _BV(ICIE1);						//Enable Timer1 Compare Match B 
	
	//Timer 0
	_StopSystemTimer;											//Make sure timer is stopped
	TCNT0  = 0;													//Clear Timer0 count
	OCR0A  =  _TMR0_COUNTS_PER_TICK;							//Load Compare match Value (counts per sys tick constant)
	TCCR0A = _BV(WGM01);										//Set CTC Mode
	TIFR0 |= _BV(OCIE0A);										//Ensure interrupt flag is clear
	TIMSK0 = _BV(OCIE0A);										//Enable Timer0 Compare Match A interrupt
}

//Start up the system Timer (Timer 0)  Gives 10ms Counts in SysTimer
inline void StartSystemTimer(void)
{
	_StartSystemTimer;											//Set clk/1024 as clock source and start timer
	SysTimerActive = SYS_TIMER_ACTIVE;							//Set timer active flag
}

//Stop the system Timer
inline void StopSystemTimer(void)
{
	_StopSystemTimer;											//Stop timer0
	TCNT0 = 0;													//Reset count
	SysTimerActive = SYS_TIMER_STOPPED;							//Set flag		
}

//Get the system Tick Count
uint32_t GetSysTicks(void){
	
	return SysTicks;
}

//Start the beeper
void Beep(uint32_t timeMS){
	
	BeepTime = (timeMS / 10);
	if(BeepTime < 1) BeepTime = 1;
	BeepActive = 1;
	BeepStart = SysTicks;
	_BEEP_ON;
}

//Start a weld cycle (NewWeldCycle has weld cycle lengths in mS)  Will not do anything if a cycle is in progress!
void StartWeldCycle(weldcycle_s_t * NewWeldCycle)
{
	
	ActiveWeldCycle.Stage = NewWeldCycle->Stage;	
	
	if(ActiveWeldCycle.Stage == WeldStage_Wait)
	{	
		//Set weld cycle type
		ActiveWeldCycle.Type = NewWeldCycle->Type;
		//If there is no active weld cycle, continue
		//Calculate actual timer values from mS values given
		//Default 1 tick delay before cycle start must be included in calculation as offset
		//End of pulse 0
		ActiveWeldCycle.Pulse_0_Ticks =  (NewWeldCycle->Pulse_0_Ticks / _MS_PER_WELDTICK);
		//End of Inter-pulse Delay
		ActiveWeldCycle.Delay_0_Ticks = ((NewWeldCycle->Pulse_0_Ticks + NewWeldCycle->Delay_0_Ticks) / _MS_PER_WELDTICK );
		//End of pulse 1
		ActiveWeldCycle.Pulse_1_Ticks = ((NewWeldCycle->Pulse_0_Ticks + NewWeldCycle->Delay_0_Ticks + NewWeldCycle->Pulse_1_Ticks) / _MS_PER_WELDTICK );					
		
		//ActiveWeldCycle contains actual count values at this point instead of mS Values
		NextToggle = 0;	
		
		
		//Check if we can start the weld Timer
		if( IsWeldEnabled() ){
			 SysWeldEnabler = Weld_Enabled;
			 WeldTicks = 0;
			 vfdClr();
		}
	}
}
//Advance the weld state machine
void SetNextWeldState(void)
{
	static uint16_t WeldOffSet;
	static uint16_t EntryTime;
	
	//Get the current Tick Value 
	EntryTime = WeldTicks;
			
	if(ActiveWeldCycle.Type == WeldType_Double)
	{
		switch(ActiveWeldCycle.Stage)
		{
			case WeldStage_Wait:
				//Wait for Zero-x
				if( WaitZeroX() ) _GPIOWeld_ON;
				//Compute the Weld offset
				WeldOffSet += WeldTicks - EntryTime;
				//Set Next Toggle Time 
				NextToggle = ActiveWeldCycle.Pulse_0_Ticks + WeldOffSet;
				ActiveWeldCycle.Stage = WeldStage_Pulse0;
				//vfdPrintStrXY(PSTR("P0"), 2, 0, 0, _vfdTHISPage);
				break;
			
			case WeldStage_Pulse0:
				_GPIOWeld_OFF;
				NextToggle = ActiveWeldCycle.Delay_0_Ticks + WeldOffSet;
				//Set Next Stage 
				ActiveWeldCycle.Stage = WeldStage_Delay;
				//vfdPrintStrXY(PSTR("D0"), 2, 3, 0, _vfdTHISPage);
				break;
			
			case WeldStage_Delay:
				//Wait for Zero-x
				if( WaitZeroX() ) _GPIOWeld_ON;
				//Compute weld Offset 
				WeldOffSet += WeldTicks - EntryTime;
				//Set Next Toggle Time 
				NextToggle = ActiveWeldCycle.Pulse_1_Ticks + WeldOffSet;
				//Set Next Stage 
				ActiveWeldCycle.Stage = WeldStage_Pulse1;
				//vfdPrintStrXY(PSTR("P1"), 2, 6, 0, _vfdTHISPage);
				break;
			
			case WeldStage_Pulse1:
			
			default:
				NextToggle = 0xFFFF;
				//Turn Off Weld 
				_GPIOWeld_OFF;
				//Set Next stage 
				ActiveWeldCycle.Stage = WeldStage_Wait;
				//Disable Weld 
				SysWeldEnabler = Weld_NotEnabled;
				//vfdPrintStrXY(PSTR("END"), 3, 9, 0, _vfdTHISPage);
		}	
	}
	
	if(ActiveWeldCycle.Type == WeldType_Single)
	{
		switch(ActiveWeldCycle.Stage)
		{
			case WeldStage_Wait:
				//Wait for Zero-x
				if( WaitZeroX() ) _GPIOWeld_ON;
				//Compute the Weld offset
				WeldOffSet += WeldTicks - EntryTime;
				//Set Next Toggle Time 
				NextToggle = ActiveWeldCycle.Pulse_0_Ticks + WeldOffSet;
				//Set Next Stage 
				ActiveWeldCycle.Stage = WeldStage_Pulse0;
				//vfdPrintStrXY(PSTR("P0"), 2, 0, 0, _vfdTHISPage);
				break;
				
			case WeldStage_Pulse0:
			
			default:
				NextToggle = 0xFFFF;
				//Turn Off Weld 
				_GPIOWeld_OFF;
				//Set Next stage
				ActiveWeldCycle.Stage = WeldStage_Wait;
				//Disable weld
				SysWeldEnabler = Weld_NotEnabled;
				//vfdPrintStrXY(PSTR("END"), 3, 3, 0, _vfdTHISPage);
		}
	}
	
	if(SysWeldEnabler == Weld_NotEnabled)						//If for any reason, WeldEnabler = Weld_NotEnabled (safety, error, finished, etc)
	{
		_GPIOWeld_OFF;											//Ensure weld output is OFF!
		ActiveWeldCycle.Stage = WeldStage_End;					//Set wait mode (Weld was halted for some reason, or is finished)
		//Clear Offset
		WeldOffSet = 0;
	}
}

//Set the current Weld State
int SetActiveWeldState (weldcycle_enum_t stage){
	
	//Check if a weld is currently active
	if((ActiveWeldCycle.Stage == WeldStage_Wait) ||
	   (ActiveWeldCycle.Stage == WeldStage_Run)  ||
	   (ActiveWeldCycle.Stage == WeldStage_End)    ){
		//If not, set state
		if(IsWeldEnabled()) {
			ActiveWeldCycle.Stage = stage;
			return (1);
		}else{
			return (0);
		}
	}
	
	//Weld is in progress, cannot change 
	return (-1);
	
}

//Get the current Weld State
weldcycle_enum_t GetActiveWeldState(void){
	return ActiveWeldCycle.Stage;
}

//Emergency Halt a weld if in progress
void EmergencyHaltWeld(void){
	//Disable in progress welds 
	SysWeldEnabler = Weld_NotEnabled;
	//Set Wait Stage
	ActiveWeldCycle.Stage = WeldStage_End;
	//Stop the timer 
	_StopWeldTimer;
	//Turn off the output (If On)
	_GPIOWeld_OFF;	
}

