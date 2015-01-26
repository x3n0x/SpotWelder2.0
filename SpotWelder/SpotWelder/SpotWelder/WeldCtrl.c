//*****************************************************************************
//
// File Name	: 'WeldControl.C'
// Title		: MiniWeld Pro Miniature Spot Welder - Weld control support 
// Author		: Joe Niven - Copyright (C) 2012 All Rights Reserved
// Created		: 10/26/2014 9:39:56 PM
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


//Global Weld Settings ********************************************************
weldctrl_s_t WeldSettings;

uint8_t ContactTrigLevel;

//Weld Control Local Variables ************************************************

//Master weld enabled?
static uint8_t WeldEnabled = 0;
//Is weld Triggered?
static volatile uint8_t WeldTriggered = 3;
//Current Weld Cycle
static weldcycle_s_t CurWeldCycle;
//Zero Cross Detection
static volatile uint8_t ZeroXLost = 0;
static volatile uint8_t ZeroX_Detected = 0; 
static volatile uint8_t ZeroX_Polarity = 0;
static volatile uint32_t ZeroX_LastDetectedTS = 0;

//Macros

//Analog or Terminal detect
#define _DisTermDetect		(EIMSK &= ~_BV(INT2))
#define _EnaTermDetect		(EIMSK |=  _BV(INT2))

//Foot Switch
#define _DisFootSW			(EIMSK &= ~_BV(INT1))
#define _EnaFootSW			(EIMSK |=  _BV(INT1))

//Zero Cross
#define _DisZeroX			(EIMSK &= ~_BV(INT0))
#define _EnaZeroX			(EIMSK |=  _BV(INT0))

//Function implementations ****************************************************

//Interrupt Handlers       **********
//INT2 - Detects 'connected' Weld terminals to automatically trigger 
//               a weld when this feature is enabled 
ISR(INT2_vect){
	
	uint8_t PinState;
	//Check to make sure switch is pressed still
	PinState = _CSINPINS & _BV(_CSINPIN);
	if(!PinState){
		WeldTriggered = 0;
	}else{
		//rESET aCTIVITY
		UI_ResetActivity();
		if (WeldEnabled) WeldTriggered = 1;
		//Disable further Detection for Now
		_DisTermDetect;
	}
}



//Interrupt 0 - Detects Zero Cross to allow proper weld triggering in sync
//              with the AC line current/Voltage
ISR(INT0_vect){
	uint8_t TempPins;
	
	//Grab Input state
	TempPins = _ZCINPINS;
	//Set Detected Flag
	ZeroX_Detected = 1;
	//Zero X has been Detected
	ZeroXLost = 0;
	//Check polarity of input
	if((TempPins & _BV(_ZCINPIN)) == 0){
		//Low Polarity 
		ZeroX_Polarity = 0;
	}else{
		ZeroX_Polarity = 1;
	}
	
	//Save timestamp of Last detected Zero Cross
	ZeroX_LastDetectedTS = GetSysTicks();
	
}

//Interrupt 1 - Detects Foot switch to initiate a weld
ISR(INT1_vect){
	uint8_t PinState;
	//Check to make sure switch is pressed still
	PinState = _FSWINPINS & _BV(_FSWINPIN);
	
	if(PinState){
		WeldTriggered = 0;	
	}else{
		//rESET aCTIVITY
		UI_ResetActivity();
		
		//Check to see if Foot SW mode is enabled
		if(WeldSettings.Trigger == wTrigFootSwitch){
			//If Yes, Trigger weld
			if (WeldEnabled) WeldTriggered = 1;
		}
		
		//Disable further Detection for Now
		_DisFootSW;
	}
}

//Private Control Functions 

//Control Functions *********
uint8_t WaitZeroX(void){
	
	uint32_t EntryTime;
	
	EntryTime = GetSysTicks();
	
	//Wait for Zero X	
	while(!ZeroX_Detected){
		if(GetSysTicks() > (EntryTime + (_MAXZeroXLossTime_mS / _MS_PER_SYSTICK))) break;
	}
	
	//Check if Zero Cross Occurred	
	if(ZeroX_Detected){
		//Zero X Occurred, Reset and return 1
		ZeroX_Detected = 0;
		return 1;
	}else{
		//No Zero X (Timed Out)
		//Set Flag
		ZeroXLost = 1;
		//Return 0
		return 0;
	}
	
}

//Prepare welder for operation
void WELD_Init(void){
	//Enable interrupts
	//Analog Comparator (Rising Edge Interrupt)
	//ACSR |= (_BV(ACIS0)| _BV(ACIS1) | _BV(ACIE));
	//INT0 = Zero Cross, INT1 = Foot switch, INT2 = TERM DET
	//Int0 (Any Edge 0x01), Int1 (Low Level 0x00), INT2 (rising Edge 0x3)
	EICRA |= (_BV(ISC00) | _BV(ISC21) | _BV(ISC20) );
	//Enable INT0, 1, 2
	EIMSK |= (_BV(INT0) | _BV(INT1) | _BV(INT2));
	//Set initial disabled state 
	WeldEnabled = 0;
	//Set Not triggered 
	WeldTriggered = 0;
}
//Weld servicer - runs weld cycles - call periodically to run weld system
void WELD_Service(void){
	
	static uint32_t NextStepTime, EntryTime, NextWeld;
	static uint8_t TriggerStarted, ResetStarted;
			
	EntryTime = GetSysTicks();
	
	//Trigger state 0, reset the trigger system
	if(WeldTriggered == 0){
		//Check if we can reset the trigger yet
		if(CurWeldCycle.Stage == WeldStage_Wait){
			//Check what trigger mode is being used and reset it
			if (WeldSettings.Trigger == wTrigContact){
				if(WeldEnabled) {
					//Connect Terminal Measure Relay
					_MRELAY_ON;
					//Enable Terminal Measure Detect
					_EnaTermDetect;
				}
			}
			
			if (WeldSettings.Trigger == wTrigFootSwitch){
				if(WeldEnabled) {
					//Disconnect Terminal Measure Relay
					_MRELAY_OFF;
					//Enable Foot switch detection
					_EnaFootSW;
				}
			}
		}
	}
	
	//Check if a weld has been triggered:
	if(WeldTriggered == 1){
		//What mode are we in?
		switch (WeldSettings.Trigger){
			//Contact detection Trigger
			case wTrigContact:
				if(!TriggerStarted){
					UI_ForceUpdate();
					//Set next Trigger step time
					NextStepTime = EntryTime + (WeldSettings.Trig_Delay / _MS_PER_SYSTICK);
					//Prepare next trigger state
					TriggerStarted = 1;
					Beep(100);
				}else{
					//Check if Terminals disconnected
					if((_CSINPINS & _BV(_CSINPIN)) == 0){
						//Terminals Disconnected
						//Reset Trigger
						WeldTriggered = TriggerStarted = 0;
						Beep(100);
						UI_ForceUpdate();
					}else{
						if(EntryTime > NextStepTime){
							//Disconnect Terminal Measure relay
							_MRELAY_OFF;
							//Go to next stage of triggering
							WeldTriggered = 2;
							//Reset trigger state
							TriggerStarted = 0;
							//Beep to signify start Weld
							Beep(50);
						}
					}
				}
				break;
			//Foot-Switch Trigger
			case wTrigFootSwitch:
				if(!TriggerStarted){
					UI_ForceUpdate();
					//Set Next Trigger step time
					if(WeldSettings.Type == wTypeContinuous)
						NextStepTime = EntryTime + (_UI_MIN_FOOTSW_MS / _MS_PER_SYSTICK);
					else
						NextStepTime = EntryTime + (WeldSettings.Trig_Delay / _MS_PER_SYSTICK);
					//Prepare Next trigger state
					TriggerStarted = 1;
					//Beep to indicate detection of Foot Switch
					Beep(100);
				}else{
					//Check if switch still depressed
					if((_FSWINPINS & _BV(_FSWINPIN)) != 0){
						//Foot Switch was released prematurely...
						//Reset Trigger
						WeldTriggered = TriggerStarted = 0;
						Beep(100);
						UI_ForceUpdate();
					}else{
						//Has time expired yet?
						if(EntryTime > NextStepTime){
							//Disconnect Measure relay
							_MRELAY_OFF;
							//Go to next trigger stage
							WeldTriggered = 2;
							//Reset Trigger state
							TriggerStarted = 0;
							//Compute first warning beep time
							NextStepTime = EntryTime + (_UI_CONTWELD_WARN_INT_MS / _MS_PER_SYSTICK);
							//Beep to signify Weld STart 
							Beep(50);
						}
					}
				}
				break;
			//Any other State
			default:
			TriggerStarted = WeldEnabled = 0;
		}
	}
	
	//check for stage 2 triggering (Start the weld)
	if(WeldTriggered == 2){
		//Check what weld mode we are in
		switch (WeldSettings.Type){
			case wTypeContinuous:
				//Check if Foot Switch is still depressed
				if((_FSWINPINS & _BV(_FSWINPIN)) == 0){
					//Check if enabled
					if(WeldEnabled){
						//Time to beep yet?
						if(EntryTime > NextStepTime){
							Beep(5);
							NextStepTime = EntryTime + (_UI_CONTWELD_WARN_INT_MS / _MS_PER_SYSTICK);
							UI_ResetActivity();
						}
						//See if weld is already active...
						if(CurWeldCycle.Stage != WeldStage_Run){
							//Wait for Zero X
							if( WaitZeroX() ){
								//Turn On Weld
								_GPIOWeld_ON;
								CurWeldCycle.Stage = WeldStage_Run;
							}
						}
					}else{
						//Weld Was disabled for some reason...
						_GPIOWeld_OFF;
						CurWeldCycle.Stage = WeldStage_End;	
						//Reset Trigger
						WeldTriggered = 3;
					}
				}else{
					//Wait for Zero X
					WaitZeroX();
					//Turn Off Weld
					_GPIOWeld_OFF;
					//Set stage
					CurWeldCycle.Stage = WeldStage_End;
					//Reset trigger
					WeldTriggered = 3;
				}
				
				//Sync State
				SetActiveWeldState(CurWeldCycle.Stage);
				break;
			
			case wTypeSinglePulse:
			case wTypeDoublePulse:
				//Load Weld Parameters
				CurWeldCycle.Pulse_0_Ticks = WeldSettings.P0_Length;
				CurWeldCycle.Pulse_1_Ticks = WeldSettings.P1_Length;
				CurWeldCycle.Delay_0_Ticks = WeldSettings.IP_Delay;
				if(WeldSettings.Type == wTypeSinglePulse) CurWeldCycle.Type = WeldType_Single;
				if(WeldSettings.Type == wTypeDoublePulse) CurWeldCycle.Type = WeldType_Double;
				CurWeldCycle.Stage = WeldStage_Wait;
				//Prepare to start Weld
				if(WeldEnabled){
					//Disconnect Measurement Relay
					_MRELAY_OFF;
					//Start the Weld
					StartWeldCycle(&CurWeldCycle);
					UI_ResetActivity();
				}
				//Reset Trigger
				WeldTriggered = 3;
				break;
			
			default:
				//Reset state Machine 
				WeldTriggered = 3;
				//Set stage
				CurWeldCycle.Stage = WeldStage_End;
		}
		
		//Set Next Entry Time
		if(WeldTriggered == 3){
			NextWeld = EntryTime + (_INTERWELD_Delay_mS / _MS_PER_SYSTICK);
		}
		
	}	
	
	if( (CurWeldCycle.Stage == WeldStage_Wait) || 
	    (CurWeldCycle.Stage == WeldStage_End ) ){
		//Check for Activity in the UI
		if(!UI_GetActivity()){
			//Set the Weld State to End
			CurWeldCycle.Stage = WeldStage_End;
			//Set the Trigger stage to 3
			WeldTriggered = 3;
			//Set Next Weld Time
			NextWeld = EntryTime + (_INTERWELD_Delay_mS / _MS_PER_SYSTICK);
			//Enable Foot switch detection
			_EnaFootSW;
		}
	}
	
	//Check for weld stage 3 - Wait for next weld 
	if(WeldTriggered == 3){
		if(GetActiveWeldState() == WeldStage_End){
			//Check to see if terminals or foot-switch have been released
			//Terminals 
			if(WeldSettings.Trigger == wTrigContact){
				//Reconnect Measurement relay
				_MRELAY_ON;
				if((_CSINPINS & _BV(_CSINPIN)) == 0 ) 
					ResetStarted = 1;
				else
					ResetStarted = 0;
			}
			//Foot switch
			if(WeldSettings.Trigger == wTrigFootSwitch){
				if((_FSWINPINS & _BV(_FSWINPIN)) != 0 )
					ResetStarted = 1;
				else 
					ResetStarted = 0;
			}
			
			if(!ResetStarted){
				//Set Next Weld Time
				NextWeld = EntryTime + (_INTERWELD_Delay_mS / _MS_PER_SYSTICK);
			}
			
			if(EntryTime > NextWeld){
				WeldTriggered = 0;
				CurWeldCycle.Stage = WeldStage_Wait;
				SetActiveWeldState(WeldStage_Wait);
				UI_ForceUpdate();
			}
		}
	}
	
	//Check for Zero_X Dropout
	if( ZeroXLost ){
		//Zero Cross has not been detected
		//Breaker may be Open or Something is damaged
		//Disable Weld
		DisableWeld();
		
		Beep(100);
		
		uint8_t MessageDisplayed = 0;
		
		//Display error Message Until Fixed
		while(1){
			
			if(!MessageDisplayed){
				vfdClr();
				vfdPrintStrXY(PSTR("AC Not Detected:"), 16, 0, 0, _vfdTHISPage);
				vfdPrintStrXY(PSTR(" Check Breaker! "), 16, 0, 1, _vfdTHISPage);
				MessageDisplayed = 1;
			}
			
			if(!ZeroXLost) break;
		}
		
		//Reset the UI
		UI_ResetActivity();
					
		//Enable the Weld
		EnableWeld();
	}
}


//External Access and Helper Functions **********

//Disable weld - if a cycle is in progress, it will be halted
void DisableWeld(void){
	//Check if we still need to disable Weld
	if(WeldEnabled){
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
			//Stop any in progress Weld 
			EmergencyHaltWeld();
			//Disable any more welds 
			WeldEnabled = 0;
			//Disconnect Measurement relay
			_MRELAY_OFF;
		}
		//Update home screen
		//UI_ForceUpdate();
	}
}
//Enable Weld
int EnableWeld(void){
	
	//Verify there are valid parameters before enabling
	//Check Weld settings - 
	//PO length
	if ( (WeldSettings.P0_Length < _MINWeldPulseLength_mS ) ||
	     (WeldSettings.P0_Length > _MAXWeldPulseLength_mS ) )	return (-1);
	//P1 Length
	if ( (WeldSettings.P1_Length < _MINWeldPulseLength_mS ) ||
	     (WeldSettings.P1_Length > _MAXWeldPulseLength_mS ) )   return (-2);
	//IP Delay
	if( (WeldSettings.IP_Delay < _MINWeldPulseDelay_mS) ||
	    (WeldSettings.IP_Delay > _MAXWeldPulseDelay_mS) )		return (-3);
		
	//Trig Delay
	if( (WeldSettings.Trig_Delay < _MINWeldPulseDelay_mS) ||
	    (WeldSettings.Trig_Delay > _MAXWeldPulseDelay_mS) )		return (-4);
	
	//Weld Type
	if( (WeldSettings.Type != wTypeContinuous)  &&
	    (WeldSettings.Type != wTypeSinglePulse) &&
		(WeldSettings.Type != wTypeDoublePulse) )		return (-5);
	
	//Enable Weld Cycles to be started 
	if(!WeldEnabled){
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
			//_MRELAY_OFF;
			//Load DAC Value
			MCP48_SetValue((uint16_t)ContactTrigLevel, _MCP48_GAIN_2);
			//Set up Weld State Machine
			WeldEnabled = 3;
			_StartWeldTimer;
		}
		
		UI_ForceUpdate();
	}
	
	return 1;
}
//Get trigger state
uint8_t IsWeldTriggered(void){
	return WeldTriggered;
}

//Get Enabled state
uint8_t IsWeldEnabled(void){
	return WeldEnabled;
}

//Get Current Weld Settings
weldctrl_s_t* GetWeldSettings(void){
	return &WeldSettings;
}