//*****************************************************************************
//
// File Name	: 'MCP48XX.h'
// Title		: Hardware Definition file for Microchip MC48XX SPI 8/10/12 bit DAC, 
// Author		: Joe Niven - Copyright (C) 2012 All Rights Reserved
// Created		: 2012-06-15
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


#ifndef MCP48XX_H_
#define MCP48XX_H_

#include <avr/io.h>
#include "SPI_AVR8_Fixed.h"

//#warning MCP48XX.h assumes that SPI_INIT() has already been called before MCP48_Init()!

//Settings
//Set the resolution of the DAC device (Should match the device your using)
#define _MCP48_BITS		8

//The following defines if there is a separate /LDAC pin to use when loading values; 
//comment out if using the configuration described below!

#define _MCP48_USE_LDAC

/* NOTE: /LDAC may be tied to /CS through an inverter to free up an IO pin, and new
   DAC values will be loaded auto-magically when the part is deselected!	   */

//Pin configuration
#define _MCP48_CS_BIT		4
#define _MCP48_CS_PORT		PORTB
#define _MCP48_CS_DDR		DDRB

#define _MCP48_LDAC_BIT		3
#define _MCP48_LDAC_PORT	PORTB
#define _MCP48_LDAC_DDR		DDRB

//Configuration Macros
#define _MCP48_ACTIVE	0x10
#define _MCP48_GAIN_1	0x20
#define _MCP48_GAIN_2	0x0

//Convenience Macros
#define _MCP48_SELECT	( _MCP48_CS_PORT   &= ~_BV(_MCP48_CS_BIT) )
#define _MCP48_DESELECT	( _MCP48_CS_PORT   |=  _BV(_MCP48_CS_BIT)  )
#define _MCP48_LOAD_DAC	( _MCP48_LDAC_PORT &= ~_BV(_MCP48_LDAC_BIT) )
#define _MCP48_DAC_OFF	( _MCP48_LDAC_PORT |=  _BV(_MCP48_LDAC_BIT)  )

//Helper Functions

//Initialize the MCP48XX device
static inline void MCP48_Init() __attribute__((always_inline));
static inline void MCP48_Init()
{
	_MCP48_CS_PORT |= _BV(_MCP48_CS_BIT);
	_MCP48_CS_DDR |= _BV(_MCP48_CS_BIT);
#if defined( _MCP48_USE_LDAC )
	_MCP48_LDAC_PORT |= _BV(1 << _MCP48_LDAC_BIT);
	_MCP48_LDAC_DDR |= _BV(1 << _MCP48_LDAC_BIT);
#endif
	_MCP48_SELECT;
	SPI_SendByte(_MCP48_ACTIVE);
	SPI_SendByte(0x0);
	_MCP48_DESELECT;
}
//Sets the value of the MCP48XX device output and wakes it up, loading the DAC immediately
static inline void MCP48_SetValue(uint16_t val, uint8_t gain) __attribute__((always_inline));
static inline void MCP48_SetValue(uint16_t val, uint8_t gain) 
{
	static uint16_t out;
	static uint8_t high, low;

	out = val << (12 - _MCP48_BITS); 
	high = (out & 0xf00) >> 8;
	low = (out & 0x0ff);
	_MCP48_SELECT;
	SPI_SendByte(_MCP48_ACTIVE | gain | high);
	SPI_SendByte(low);
	_MCP48_DESELECT;
#if defined( _MCP48_USE_LDAC )
	_MCP48_DAC_OFF;
#endif	
	_delay_us(1);
#if defined( _MCP48_USE_LDAC )	
	_MCP48_LOAD_DAC;
#endif	
}
#if defined( _MCP48_USE_LDAC )
//Sets the value of the MCP48XX device output and wakes it up if it is sleeping.  
//The DAC must be loaded manually by toggling the /LDAC line!
//Function only available when using a separate /LDAC line!
static inline void MCP48_SetValue_NoLoad(uint8_t val, uint8_t gain) __attribute__((always_inline));
static inline void MCP48_SetValue_NoLoad(uint8_t val, uint8_t gain)
{
	static uint16_t out;
	static uint16_t high, low;
	out = val << (12 - _MCP48_BITS);
	high = (out & 0xf00) >> 8;
	low = (out & 0x0ff);
	_MCP48_SELECT;
	SPI_SendByte(gain | high);
	SPI_SendByte(low);
	_MCP48_DESELECT;
}
#endif
//Puts the MCP48XX device to sleep and sets its output value to 0
static inline void MCP48_Shutdown() __attribute__((always_inline));
static inline void MCP48_Shutdown()
{
	_MCP48_SELECT;
	SPI_SendByte(0x0);
	SPI_SendByte(0x0);
	_MCP48_DESELECT;
}

#endif /* MCP48XX_H_ */