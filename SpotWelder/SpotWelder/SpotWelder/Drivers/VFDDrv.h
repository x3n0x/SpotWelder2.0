//*****************************************************************************
//
// File Name	: 'VFDDrv.h'
// Title		: Noritake-ITRON VFD Driver Header File
// Author		: Joe Niven - Copyright (C) 2014 All Rights Reserved
// Created		: 10/17/2014 9:12:04 PM
// Revised		: 01/25/2015 9:12:00 PM
// Version		: 1.0
// Target MCU	: Atmel Mega AVR series
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

//See VFDDrv.c for release notes and updates

//Some important notes about this Code:

//HD44780 Compatible VFD controllers are much faster than HD44780 type LCD 
//controllers.  Some of the options below will allow you to use a slower 
//controller, and also send Custom settings on initialization for display 
//Characteristics, such as line etc.

//For Example, a Display implementing the ST7066U Controller requires the 
//init to set up the number of lines, Where as a Noritake 16x2 VFD usually 
//does not require that particular Setting to be sent. Uncomment the Custom 
//init define, and build a command for your particular display as needed.  
//See the documentation for your LCD to learn what needs to happen on init.

//This Library implements the 3x Enable toggles to help wake most displays.
//If more custom init is needed, see vfdInit() to add steps etc.  It also
//uses an actual 'WAIT' for the Busy flag, allowing the fastest possible 
//updates to the display.

#ifndef VFDDRV_H_
#define VFDDRV_H_

//VFD IO pins Definition
//Note:  Currently the driver uses a Single 8 Bit IO port for Byte Transfer

//Uncomment this if you want to use an HD44780 Compatible LCD that does not 
//support the Noritake brightness Command (Non-VFD)
#define NO_BRIGHTNESS		1

//Uncomment this if you want to interface with a Slower controller IC such 
//as a HD44780 which has longer instruction execution times
//#define LONG_DELAYS			1

//Uncomment to enable Custom Function Set On Initialization 
#define CUSTOM_INIT			1
//See your LCD docs to set this to the right Value!!
//Set the Custom Function Set Value here
#define CUSTOM_FSET			0x38

//Byte Transfer Port
#define _vfdPORT			PORTC
#define _vfdDDR				DDRC
#define _vfdPINS			PINC
//Control Signals
#define _vfdRSBIT			5
#define _vfdRSPORT			PORTA
#define _vfdRSDDR			DDRA

#define _vfdRWBIT			6
#define _vfdRWPORT			PORTA
#define _vfdRWDDR			DDRA

#define _vfdENBIT			7
#define _vfdENPORT			PORTA
#define _vfdENDDR			DDRA

//Command Definitions
#define _vfdCmdClr			0x1
#define _vfdCmdHome			0x2
#define _vfdCmdESet			0x4
#define _vfdCmdDpOn			0x8
#define _vfdCmdShft			0x10
#define _vfdCmdFSet			0x20
#define _vfdCmdCGAddr		0x40
#define _vfdCmdDDAddr		0x80

//Args - Brightness
#define _vfdBright00		0x0
#define _vfdBright75		0x1
#define _vfdBright50		0x2
#define _vfdBright25		0x3

//Args - Shift
#define _vfdShift			0x8				//Display Shift
#define _vfdShRgt			0x4				//Shift Right  

//Args - Display On/Off
#define _vfdDpyEN			0x4				//Display On
#define _vfdCurEN			0x2				//Cursor On	
#define _vfdBlnkEN			0x1				//Blink @ Cursor On

//Args - Entry
#define _vfdIncAddr			0x2				//Increment Address
#define _vfdSftEN			0x1				//Enable Display Shift

//Args - Function 
#define	_vfd8Bit			0x10			//Set Interface Width to 8 bits
#define _vfd2Lines			0x08			//Set 2 Line Mode
#define _vfdFMT5x11			0x04			//Set 5x11 Character Mode

//Display Attributes (Must match your display)
#define _vfdNumLines		2
#define _vfdNumChars		16
#define _vfdLine0Addr		0x0
#define _vfdLine1Addr		0x40

//Universal Constants
#define _vfdON				1
#define _vfdOFF				0

//Left, right or Current Page (For string Print Operations)
typedef enum vfdScreenPos{
		_vfdLEFTPage = 0,
		_vfdRIGHTPage = 1,
		_vfdTHISPage = 2
	
	}vfdScreenPos;

//Control Functions 
//Send a command to the VFD
void vfdSendCmd(uint8_t cmd);
//Send data (or Characters) to the VFD		
void vfdSendData(uint8_t data);
//Goto location XY
void vfdGotoXY(uint8_t x, uint8_t y);
//Print a String on the VFD
void vfdPrintStr(const char* str, uint8_t len);
//Print a String on the VFD @ X, Y
#if (_vfdNumChars <= 20)

//For Display with 20 or less, can use 'Left' and 'Right' page functionality
void vfdPrintStrXY(const char* str, uint8_t len, uint8_t x, uint8_t y, vfdScreenPos Page);

#else

//For Displays with more than 20, cannot use 'Page' Function 
void vfdPrintStrXY(const char* str, uint8_t len, uint8_t x, uint8_t y);

#endif 
//Copy a string from SRAM to VFD
void vfdCopyStr( char* str, uint8_t len, uint8_t x, uint8_t y);
//display a Number at x,y
void vfdDisplayNum( uint16_t num, uint8_t x, uint8_t y);

//Shift Left 
void vfdShiftLeft(uint8_t n);
//Shift Right
void vfdShiftRight(uint8_t n);

//These functions only work if Display has 20 or less chars per line 
#if (_vfdNumChars <= 20)

//Switch to the 'Left' Screen
void vfdFlipPageLeft(uint8_t DelayMS);
//Switch to the 'Right' Screen
void vfdFlipPageRight(uint8_t DelayMS);

#endif 

//Cursor On/Off
void vfdSetCursor(uint8_t state);
//Display Enable/Disable
void vfdSetDisplay(uint8_t state);
//Set VFD brightness
void vfdSetBright(uint8_t bright);
//Send a special Character to the VFD
void vfdSetChar(uint8_t Loc, const uint8_t *bitmap);
//Clear the Display
void vfdClr(void);
//Initialize the VFD
void vfdInit(void);

#endif /* VFDDRV_H_ */