/*
             LUFA Library
     Copyright (C) Dean Camera, 2011.

  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org
*/

/*
  Copyright 2011  Dean Camera (dean [at] fourwalledcubicle [dot] com)

  Permission to use, copy, modify, distribute, and sell this
  software and its documentation for any purpose is hereby granted
  without fee, provided that the above copyright notice appear in
  all copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaim all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.

  Modified By Joe Niven 05-JUN-2012 for use outside of LUFA.  
  Must be used only on MegaAVR (32x,16x,4x,8x) 8 Series parts or pin definitions must change!

*/

/** \file
 *  \brief SPI Peripheral Driver (AVR8)
 *
 *  On-chip SPI driver for the 8-bit AVR microcontrollers.
 *
 */

/** \ingroup Group_SPI
 *  \defgroup Group_SPI_AVR8 SPI Peripheral Driver (AVR8)
 *
 *  \section Sec_ModDescription Module Description
 *  Driver for the hardware SPI port available on most 8-bit AVR microcontroller models. This
 *  module provides an easy to use driver for the setup and transfer of data over the
 *  AVR's SPI port.
 *
 *
 *  \section Sec_ExampleUsage Example Usage
 *  The following snippet is an example of how this module may be used within a typical
 *  application.
 *
 *  \code
 *      // Initialise the SPI driver before first use
 *      SPI_Init(_SPI_SPEED_FCPU_DIV_2 | _SPI_ORDER_MSB_FIRST | _SPI_SCK_LEAD_FALLING |
 *               _SPI_SAMPLE_TRAILING | _SPI_MODE_MASTER);
 *
 *      // Send several bytes, ignoring the returned data
 *      SPI_SendByte(0x01);
 *      SPI_SendByte(0x02);
 *      SPI_SendByte(0x03);
 *
 *      // Receive several bytes, sending a dummy 0x00 byte each time
 *      uint8_t Byte1 = SPI_ReceiveByte();
 *      uint8_t Byte2 = SPI_ReceiveByte();
 *      uint8_t Byte3 = SPI_ReceiveByte();
 *
 *      // Send a byte, and store the received byte from the same transaction
 *      uint8_t ResponseByte = SPI_TransferByte(0xDC);
 *  \endcode
 * 
 *  @{
 */

#ifndef __SPI_AVR8_H__
#define __SPI_AVR8_H__

	/* Includes: */
			#include <avr/io.h>
	/* Private Interface - For use in library only: */
	/* Macros: */
			#define _SPI_USE_DOUBLESPEED            _BV(SPE)

	/* Public Interface - May be used in end-application: */
		/* Macros: */
			/** \name SPI Prescaler Configuration Masks */
			//@{
			/** SPI prescaler mask for \c SPI_Init(). Divides the system clock by a factor of 2. */
			#define _SPI_SPEED_FCPU_DIV_2           _SPI_USE_DOUBLESPEED

			/** SPI prescaler mask for \c SPI_Init(). Divides the system clock by a factor of 4. */
			#define _SPI_SPEED_FCPU_DIV_4           0

			/** SPI prescaler mask for \c SPI_Init(). Divides the system clock by a factor of 8. */
			#define _SPI_SPEED_FCPU_DIV_8           (_SPI_USE_DOUBLESPEED | _BV(SPR0))

			/** SPI prescaler mask for \c SPI_Init(). Divides the system clock by a factor of 16. */
			#define _SPI_SPEED_FCPU_DIV_16          _BV(SPR0)

			/** SPI prescaler mask for \c SPI_Init(). Divides the system clock by a factor of 32. */
			#define _SPI_SPEED_FCPU_DIV_32          (_SPI_USE_DOUBLESPEED | _BV(SPR1))

			/** SPI prescaler mask for \c SPI_Init(). Divides the system clock by a factor of 64. */
			#define _SPI_SPEED_FCPU_DIV_64          (_SPI_USE_DOUBLESPEED | _BV(SPR1) | _BV(SPR0))

			/** SPI prescaler mask for \c SPI_Init(). Divides the system clock by a factor of 128. */
			#define _SPI_SPEED_FCPU_DIV_128         (_BV(SPR1) | _BV(SPR0))
			//@}

			/** \name SPI SCK Polarity Configuration Masks */
			//@{
			/** SPI clock polarity mask for \c SPI_Init(). Indicates that the SCK should lead on the rising edge. */
			#define _SPI_SCK_LEAD_RISING            (0 << CPOL)

			/** SPI clock polarity mask for \c SPI_Init(). Indicates that the SCK should lead on the falling edge. */
			#define _SPI_SCK_LEAD_FALLING           _BV(CPOL)
			//@}

			/** \name SPI Sample Edge Configuration Masks */
			//@{
			/** SPI data sample mode mask for \c _SPI_Init(). Indicates that the data should sampled on the leading edge. */
			#define _SPI_SAMPLE_LEADING             (0 << CPHA)

			/** SPI data sample mode mask for \c _SPI_Init(). Indicates that the data should be sampled on the trailing edge. */
			#define _SPI_SAMPLE_TRAILING            _BV(CPHA)
			//@}
			
			/** \name SPI Data Ordering Configuration Masks */
			//@{
			/** SPI data order mask for \c _SPI_Init(). Indicates that data should be shifted out MSB first. */
			#define _SPI_ORDER_MSB_FIRST            (0 << DORD)

			/** SPI data order mask for \c _SPI_Init(). Indicates that data should be shifted out MSB first. */
			#define _SPI_ORDER_LSB_FIRST            _BV(DORD)
			//@}
			
			/** \name SPI Mode Configuration Masks */
			//@{
			/** SPI mode mask for \c _SPI_Init(). Indicates that the SPI interface should be initialized into slave mode. */
			#define _SPI_MODE_SLAVE                 (0 << MSTR)

			/** SPI mode mask for \c _SPI_Init(). Indicates that the SPI interface should be initialized into master mode. */
			#define _SPI_MODE_MASTER                _BV(MSTR)
			//@}
			
		/* Inline Functions: */
			/** Initialises the SPI subsystem, ready for transfers. Must be called before calling any other
			 *  SPI routines.
			 *
			 *  \param[in] SPIOptions  SPI Options, a mask consisting of one of each of the \c _SPI_SPEED_*,
			 *                         \c _SPI_SCK_*, \c _SPI_SAMPLE_*, \c _SPI_ORDER_* and \c _SPI_MODE_* masks.
			 */
			static inline void SPI_Init(const uint8_t SPIOptions)
			{
				DDRB  |=  (_BV(PB5) | _BV(PB7) | _BV(PB4));			//JN: SCLK,MOSI, SS outputs (ATMEGA series)
				//DDRB  |=  (_BV(PB5) | _BV(PB7));					//JN: SCLK and MOSI outputs (ATMEGA series)
				DDRB  &= ~_BV(6);									//JN: MISO and SS Pin Disabled
				//DDRB  &= ~(_BV(4) | _BV(2));					//JN: MISO and SS inputs
				PORTB |=  _BV(6);									//JN: MISO set High
				//PORTB |=  (_BV(4) | _BV(2));					//JN: MISO and SS set High
				SPCR   = (_BV(SPE) | SPIOptions);

				if (SPIOptions & _SPI_USE_DOUBLESPEED)
				  SPSR |= _BV(SPI2X);
				else
				  SPSR &= ~_BV(SPI2X);
			}

			/** Turns off the SPI driver, disabling and returning used hardware to their default configuration. */
			static inline void SPI_Disable(void)
			{
				DDRB  &= ~(_BV(5) | _BV(3));		//JN: SCLK and MOSI inputs
				PORTB &= ~(_BV(4) | _BV(2));		//JN: MISO and SS Cleared

				SPCR   = 0;
				SPSR   = 0;
			}

			/** Sends and receives a byte through the SPI interface, blocking until the transfer is complete.
			 *
			 *  \param[in] Byte  Byte to send through the SPI interface.
			 *
			 *  \return Response byte from the attached SPI device.
			 */
			static inline uint8_t SPI_TransferByte(const uint8_t Byte) __attribute__ ((always_inline));
			static inline uint8_t SPI_TransferByte(const uint8_t Byte)
			{
				SPDR = Byte;
				while (!(SPSR & _BV(SPIF)));
				return SPDR;
			}

			/** Sends a byte through the SPI interface, blocking until the transfer is complete. The response
			 *  byte sent to from the attached SPI device is ignored.
			 *
			 *  \param[in] Byte  Byte to send through the SPI interface.
			 */
			static inline void SPI_SendByte(const uint8_t Byte) __attribute__ ((always_inline));
			static inline void SPI_SendByte(const uint8_t Byte)
			{
				SPDR = Byte;
				while (!(SPSR & _BV(SPIF)));
			}

			/** Sends a dummy byte through the SPI interface, blocking until the transfer is complete. The response
			 *  byte from the attached SPI device is returned.
			 *
			 *  \return The response byte from the attached SPI device.
			 */
			static inline uint8_t SPI_ReceiveByte(void) __attribute__ ((always_inline, warn_unused_result));
			static inline uint8_t SPI_ReceiveByte(void)
			{
				SPDR = 0x00;
				while (!(SPSR & _BV(SPIF)));
				return SPDR;
			}

#endif

/** @} */

