//*****************************************************************************
//
// File Name	: 'SpotWelder.h'
// Title		: Capacitive Discharge spot welder - Main Header File
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

#ifndef _SpotWelder_H_
#define _SpotWelder_H_

#include <stdlib.h>
#include <string.h>

#include <avr/io.h>
#include <avr/sfr_defs.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include <util/atomic.h>

//UI data Declarations and helper functions
#include "UIControl.h"
#include "UIActions.h"

//Main Weld control helpers
#include "WeldCtrl.h"

//Internal Peripheral Drivers:
#include "Drivers/SPI_AVR8_Fixed.h"		//SPI Peripheral
#include "Drivers/GPIO.h"				//GPIO Definitions
#include "Drivers/TimerControl.h"		//Timer Functions

//External Hardware Drivers:
#include "Drivers/VFDDrv.h"				//VFD/LCD Driver
#include "Drivers/MCP48XX.h"			//DAC driver

//Custom Types
//Function Prototypes
void InitializeHardware(void);
//Load settings from EEPROM
void LoadSettings(void);

#endif
