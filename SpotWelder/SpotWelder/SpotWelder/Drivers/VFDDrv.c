//*****************************************************************************
//
// File Name	: 'VFDDrv.c'
// Title		: Noritake-ITRON/VFD or HD44780/LCD Driver Implementation
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

//Change Notes:  

//v0.1 (10/17/2014):  - Initial Creation and testing

//v1.0 (01/25/2015):  - Added Custom init capability to allow Custom Function
//                      Set commands to LCD/VFD on init.
//                    - Added Long Delay Capability for Slow LCD Controllers 
//                    - Initial Release Candidate

//Unique features versus other LCD/VFD Control Libraries:
// - Supports Screen Position independent of Left/Right Shift position in 
//   DDRAM
// - Supports drawing Text 'Off Screen' one full display Left or Right of 
//   the Current Position to allow scrolling a new screen in from the left 
//   or right for Menus, etc
// - Flexible initialization Options versus other Libs without code edits
// - Supports actual Busy Flag polling to allow fastest possible updates to

//AVR LIBC Includes
#include <stdlib.h>
#include <string.h>
//AVR Utility Libs
#include <util/delay.h>
#include <util/atomic.h>
//AVR ARCH Libs
#include <avr/pgmspace.h>
#include <avr/io.h>
//The Header for this lib
#include "VFDDrv.h"

//Defines

#define _vfdData		(_vfdRSPORT |=  _BV(_vfdRSBIT))
#define _vfdCmd			(_vfdRSPORT &= ~_BV(_vfdRSBIT))

#define _vfdSetEN		(_vfdENPORT |=  _BV(_vfdENBIT))
#define _vfdClrEN		(_vfdENPORT &= ~_BV(_vfdENBIT))

#define _vfdWrite		(_vfdRWPORT &= ~_BV(_vfdRWBIT))
#define _vfdRead		(_vfdRWPORT |=  _BV(_vfdRWBIT))

//Delays
#ifndef LONG_DELAYS
	#define _vfdRSDelay		2		//Normal Cycle times for VFD (uS)
	#define _vfdENDelay		23
#else
	#define _vfdRSDelay		100		//Slow Cycle times for HD44780 Compatibles (uS)
	#define _vfdENDelay		100
#endif

//Variables
static uint8_t vfdStatus	= 0;
static uint8_t vfdMode		= 0;
static volatile int HomeX	= 0;
static char Number[8];

//Private functions 
static void vfdWaitBusy(void);
static void vfdWaitBusy(void){
	
	uint8_t Busy = 1;
	uint8_t Temp = 0;
	
	//Set Port to input 
	_vfdPORT = 0x0;
	_vfdDDR = 0x0;
	//Set command 
	_vfdCmd;
	//Set Read
	_vfdRead;
	//Delay
	_delay_us(_vfdRSDelay);
	//Read the Command Register, loop if busy is set
	while(Busy){
		_vfdSetEN;
		_delay_us(_vfdENDelay);
		Temp = _vfdPINS;
		_vfdClrEN;
		_delay_us(_vfdENDelay);
		Busy = (Temp & 0x80);
	}
	//Set Port to Output
	_vfdDDR = 0xFF;
	
	//_delay_us(100);
}

//Delay Loop for non-Constant MS delays 
static void delayVar(uint8_t DelayMS);
static void delayVar(uint8_t DelayMS){
	while (DelayMS){
		_delay_ms(1);
		DelayMS --;
	}
}

//Public Control Functions
//Send a command to the VFD
void vfdSendCmd(uint8_t cmd){
	//Wait for VFD to be ready
	vfdWaitBusy();
	//Set to Command Register 
	_vfdCmd;
	//Set Write
	_vfdWrite;
	//Put Cmd on Port
	_vfdPORT = cmd;
	//delay
	_delay_us(_vfdRSDelay);
	//Strobe Data in
	_vfdSetEN;
	_delay_us(_vfdENDelay);
	_vfdClrEN;
}
//Send data (or Characters) to the VFD		
void vfdSendData(uint8_t data){
	//Wait for VFD to be ready 
	vfdWaitBusy();
	//Set to Data register 
	_vfdData;
	//Set to write
	_vfdWrite;
	//Put data on Port 
	_vfdPORT = data;
	//delay
	_delay_us(_vfdRSDelay);
	//Strobe Data in
	_vfdSetEN;
	_delay_us(_vfdENDelay);
	_vfdClrEN;
}
//Goto location XY
void vfdGotoXY(uint8_t x, uint8_t y){
	
	uint8_t LineStartAddr;
	//Figure out what address to start at
	switch(y){
		case 0:		LineStartAddr = _vfdLine0Addr; break;
		case 1:		LineStartAddr = _vfdLine1Addr; break;
		default:	LineStartAddr = _vfdLine0Addr; 		
	}
	
	if((x + HomeX) > 39) 
		x = x - (40 - HomeX);
	else
		x = (x + HomeX);
		
	//Send the command to the VFD
	vfdSendCmd(_vfdCmdDDAddr | (LineStartAddr + x));
}
//Print a String on the VFD
void vfdPrintStr(const char* str, uint8_t len){
	uint8_t i;
	
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		//Send each character byte
		for (i=0; i < len; i ++){
			vfdSendData(pgm_read_byte(&str[i]));
		}
	}
}

#if (_vfdNumChars <= 20)
//For Display with 20 or less, can use 'Left' and 'Right' page functionality
//Print a String on the VFD @ X, Y
void vfdPrintStrXY( const char* str, uint8_t len, uint8_t x, uint8_t y, vfdScreenPos Page){
	
	int tempX = 0;
		
	//Compute position (Address)
	switch (Page){
		case _vfdLEFTPage:
			tempX = (HomeX - _vfdNumChars) + y;
			if(tempX < 0) tempX = 40 + tempX;
			break;
		case _vfdRIGHTPage:
			tempX = HomeX + _vfdNumChars + y;
			if(tempX > 39) tempX = tempX - 40;
			break;
		case _vfdTHISPage:
			tempX = HomeX + x;
			if(tempX > 39) tempX = tempX - 40;
			break;
	}
	
	x = (uint8_t)tempX;
	
	//Goto Location 
	vfdGotoXY(x,y);
	//Print the string 
	vfdPrintStr(str, len);
}
#else
//For Displays with more than 20, cannot use 'Page' Function
void vfdPrintStrXY(const char* str, uint8_t len, uint8_t x, uint8_t y){
	
	//Goto Location
	vfdGotoXY(x,y);
	//Print the string
	vfdPrintStr(str, len);
	
}

#endif

//Copy a string from SRAM to VFD
void vfdCopyStr( char* str, uint8_t len, uint8_t x, uint8_t y){
	
	uint8_t c = 0;
	
	//Goto Location
	vfdGotoXY(x,y);
	
	//copy the string to the vfd
	for( c = 0; c < len; c++){
		vfdSendData((uint8_t)str[c]);
	}
}

//display a Number at x,y
void vfdDisplayNum( uint16_t num, uint8_t x, uint8_t y){
	
	uint8_t numLen = 0;
	
	//Clear the Number string 
	memset(Number, 0x20, sizeof(Number));
	//generate string of new value
	utoa(num, Number, 10);
	numLen = strlen(Number);
	vfdCopyStr(Number, numLen, x, y);
	
}

//Shift Left
void vfdShiftLeft(uint8_t n){
	
	uint8_t i;
	//Send Shift Left Command n Times 
	for(i=0; i < n; i++){
		vfdSendCmd(_vfdCmdShft | _vfdShift);
		
		if(HomeX < 39)
			HomeX++;
		else
			HomeX = 0;
	}
}
	
//Shift Right
void vfdShiftRight(uint8_t n){
	
	uint8_t i;
	//Send Shift Right command n Times 
	for(i=0; i < n; i++){
		vfdSendCmd(_vfdCmdShft | _vfdShift | _vfdShRgt);
		
		if(HomeX > 0) 
			HomeX--;
		else
			HomeX = 39;
		
	}
}
	

//These functions only work if Display has 20 or less chars per line
#if (_vfdNumChars <= 20)

//Switch to the 'Left' Screen
void vfdFlipPageLeft(uint8_t DelayMS){
	static uint8_t i = 0;
	static uint8_t DelayEach = 0;
	
	DelayEach = DelayMS / _vfdNumChars;
	
	for( i=0; i<_vfdNumChars; i++){
		vfdShiftRight(1);
		delayVar(DelayEach);
	}
}

//Switch to the 'Right' Screen
void vfdFlipPageRight(uint8_t DelayMS){
	static uint8_t i = 0;
	static uint8_t DelayEach = 0;
	
	DelayEach = DelayMS / _vfdNumChars;
	
	for( i=0; i<_vfdNumChars; i++){
		vfdShiftLeft(1);
		delayVar(DelayEach);
	}
}

#endif

//Cursor On/Off
void vfdSetCursor( uint8_t state){
	if(state){
		//Turn Cursor On
		vfdStatus |=  _vfdCurEN;
		vfdSendCmd(_vfdCmdDpOn | vfdStatus);
	}else{
		//Turn Cursor Off
		vfdStatus &= ~_vfdCurEN;
		vfdSendCmd(_vfdCmdDpOn | vfdStatus);
	}
}
//Display Enable/Disable
void vfdSetDisplay( uint8_t state){
	if(state){
		//Turn Display On
		vfdStatus |=  _vfdDpyEN;
		vfdSendCmd(_vfdCmdDpOn | vfdStatus);
	}else{
		//Turn Display Off
		vfdStatus &= ~_vfdDpyEN;
		vfdSendCmd(_vfdCmdDpOn | vfdStatus);
	}
}
//Set VFD brightness
void vfdSetBright(uint8_t bright){

#ifndef NO_BRIGHTNESS
	vfdSendCmd(_vfdCmdFSet | vfdMode);
	vfdSendData(bright);
#endif

}
//Send a special Character to the VFD
void vfdSetChar(uint8_t code, const uint8_t *bitmap){
	
	uint8_t a, pcc;
	uint16_t i;
	
	//Compute start address 
	a = (code<<3);
	
	//Send the bytes 
	for (i=0; i<8; i++){
		//Read from Flash 
		pcc=pgm_read_byte(&bitmap[i]);
		//Set CG Address
		vfdSendCmd(_vfdCmdCGAddr | a++);
		//Send Bitmap Data 
		vfdSendData(pcc);
	}
	
}
//Clear the Display
void vfdClr(void){
	vfdSendCmd(_vfdCmdClr);
	HomeX = 0;
}
//Initialize the VFD
void vfdInit(void){
	//Set initial Mode
	vfdMode = _vfd8Bit;
	//Set initial State (No Cursor, Display Active, no Blink)
	vfdStatus = _vfdDpyEN;
	//Set UP GPIO
	_vfdRSDDR = _vfdRSDDR | _BV(_vfdRSBIT);
	_vfdENDDR = _vfdENDDR | _BV(_vfdENBIT);
	_vfdRWDDR = _vfdRWDDR | _BV(_vfdRWBIT);
	_vfdDDR	   = 0xff;
	
	//Wake the controller (1)
	_vfdSetEN;
	_delay_ms(1);
	_vfdClrEN;
	_delay_ms(10);
	
	//Wake the controller (2)
	_vfdSetEN;
	_delay_ms(1);
	_vfdClrEN;
	_delay_ms(2);
	
	//Wake the controller (3)
	_vfdSetEN;
	_delay_ms(1);
	_vfdClrEN;
	_delay_ms(2);
		
	//Send the function set command 
#ifndef CUSTOM_INIT
	//Default init
	vfdSendCmd(_vfdCmdFSet | vfdMode);
#else
	//Custom Init
	vfdSendCmd(_vfdCmdFSet | CUSTOM_FSET);
#endif 
	//Set Entry Mode: Cursor Shift Right, No DPY Shift
	vfdSendCmd(_vfdCmdESet | _vfdIncAddr);
	//Enable display 
	vfdSendCmd(_vfdCmdDpOn | vfdStatus);
	//Set the brightness to 100%
	vfdSendData(_vfdBright00);
	//Clear the Display
	vfdClr();
}
