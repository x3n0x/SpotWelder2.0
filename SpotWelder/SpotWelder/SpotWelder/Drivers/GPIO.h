//*****************************************************************************
//
// File Name	: 'GPIO.h'
// Title		: Capacitive Discharge spot welder - GPIO Definitions
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


#ifndef GPIO_H_
#define GPIO_H_

#include <avr/io.h>

//UI Interface Pins ***********************************************************
//Switches
#define _SWA			3
#define _SWB			4
#define _SWPORT			PORTA
#define _SWDDR			DDRA
#define _SWPINS			PINA
//Encoder
#define _ENCA			0
#define _ENCB			1
#define _ENCPORT		PORTB
#define _ENCDDR			DDRB
#define _ENCPINS		PINB
//Foot switch detect input
#define _FSWINPIN		3
#define _FSWINPORT		PORTD
#define _FSWINDDR		DDRD
#define _FSWINPINS		PIND
//Beeper Output Pin
#define _BEEPOUTPIN		6
#define _BEEPOUTPORT	PORTD
#define _BEEPOUTDDR		DDRD
#define _BEEPOUTPINS	PIND

//Weld Control Pins ***********************************************************
//Weld Pulse Control
#define _WELDOUTPIN		7
#define _WELDOUTPORT	PORTD
#define _WELDOUTDDR		DDRD
#define _WELDOUTPINS	PIND
//Zero Cross detect input
#define _ZCINPIN		2
#define _ZCINPORT		PORTD
#define _ZCINDDR		DDRD
#define _ZCINPINS		PIND
//Terminal Connect Sense Input 
#define _CSINPIN		2
#define _CSINPORT		PORTB
#define _CSINDDR		DDRB
#define _CSINPINS		PINB
//Measurement Relay Output 
#define _MRELAYOUTPIN	4
#define _MRELAYOUTPORT	PORTD
#define _MRELAYOUTDDR	DDRD
#define _MRELAYOUTPINS	PIND

//Port control Macros *********************************************************
//Weld Control
#define _GPIOWeld_OFF	(_WELDOUTPORT &= ~_BV(_WELDOUTPIN))
#define _GPIOWeld_ON	(_WELDOUTPORT |=  _BV(_WELDOUTPIN))
#define _GPIOWeld_TGL	(_WELDOUTPINS |=  _BV(_WELDOUTPIN))
//Beeper
#define _BEEP_OFF		(_BEEPOUTPORT &= ~_BV(_BEEPOUTPIN))
#define _BEEP_ON		(_BEEPOUTPORT |=  _BV(_BEEPOUTPIN))
#define _BEEP_TGL		(_BEEPOUTPINS |=  _BV(_BEEPOUTPIN))
//Terminal Sense Relay
#define _MRELAY_OFF		(_MRELAYOUTPORT &= ~_BV(_MRELAYOUTPIN))
#define _MRELAY_ON		(_MRELAYOUTPORT |=  _BV(_MRELAYOUTPIN))
#define _MRELAY_TGL		(_MRELAYOUTPINS |=  _BV(_MRELAYOUTPIN))

//GPIO Functions **************************************************************
//Initialize GPIO
static inline void GPIO_Init(void) __attribute__((always_inline));
static inline void GPIO_Init(void){
	
	//Set SW IO to inputs with Pull-ups
	_SWDDR  &= ~(_BV(_SWA) | _BV(_SWB));
	_SWPORT |=  (_BV(_SWA) | _BV(_SWB));
	
	//Set ENC IO to inputs with Pull-ups
	_ENCDDR  &= ~(_BV(_ENCA) | _BV(_ENCB));
	_ENCPORT |=  (_BV(_ENCA) | _BV(_ENCB));
	
	//Set ZC to input with Pull-ups
	_ZCINDDR  &= ~(_BV(_ZCINPIN));
	_ZCINPORT |=  (_BV(_ZCINPIN));
	
	//Set FSW to input with Pull-ups
	_FSWINDDR  &= ~(_BV(_FSWINPIN));
	_FSWINPORT |=  (_BV(_FSWINPIN));
	
	//Set CSIN to input with Pull-ups 
	_CSINDDR  &= ~(_BV(_CSINPIN));
	_CSINPORT |= (_BV(_CSINPIN)); 
		
	//Set WELD_OUT to output
	_WELDOUTPORT &= ~_BV(_WELDOUTPIN);
	_WELDOUTDDR  |=  _BV(_WELDOUTPIN);
	
	//Set BEEP Out to output
	_BEEPOUTPORT &= ~_BV(_BEEPOUTPIN);
	_BEEPOUTDDR	 |=	 _BV(_BEEPOUTPIN);
	
	//Set Relay out to output
	_MRELAYOUTPORT &= ~_BV(_MRELAYOUTPIN);
	_MRELAYOUTDDR  |=  _BV(_MRELAYOUTPIN); 
		
	//Disable digital IO port on ADC0-2
	//to allow ADC to be used
	DIDR0 |= 0x3;
}
#endif /* GPIO_H_ */