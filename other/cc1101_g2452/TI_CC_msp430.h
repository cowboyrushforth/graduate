/* --COPYRIGHT--,BSD
 * Copyright (c) 2011, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * --/COPYRIGHT--*/
//******************************************************************************
//  Description:  This file contains definitions specific to the specific MSP430
//  chosen for this implementation.  MSP430 has multiple interfaces capable
//  of interfacing to the SPI port; each of these is defined in this file.
//
//  The source labels for the definitions (i.e., "P3SEL") can be found in
//  msp430xxxx.h.
//
//  MSP430/CC1100-2500 Interface Code Library v1.1
//
//  W. Goh
//  Texas Instruments, Inc.
//  December 2009
//  IAR Embedded Workbench v4.20
//******************************************************************************
// Change Log:
//******************************************************************************
// Version:  1.1
// Comments: Added support for various MSP430 development tools.
//           Added support for 5xx
// Version:  1.00
// Comments: Initial Release Version
//******************************************************************************

#include "msp430.h"

// SPI port definitions                     // Adjust the values for the chosen
#define TI_CC_SPI_USI_PxDIR     P1DIR
#define TI_CC_SPI_USI_PxIN      P1IN
#define TI_CC_SPI_USI_SIMO      BIT6
#define TI_CC_SPI_USI_SOMI      BIT7
#define TI_CC_SPI_USI_UCLK      BIT5


//******************************************************************************
//  These constants are used to identify the chosen SPI and UART interfaces.
//******************************************************************************
#define TI_CC_SER_INTF_NULL    0
#define TI_CC_SER_INTF_USART0  1
#define TI_CC_SER_INTF_USART1  2
#define TI_CC_SER_INTF_USCIA0  3
#define TI_CC_SER_INTF_USCIA1  4
#define TI_CC_SER_INTF_USCIA2  5
#define TI_CC_SER_INTF_USCIA3  6
#define TI_CC_SER_INTF_USCIB0  7
#define TI_CC_SER_INTF_USCIB1  8
#define TI_CC_SER_INTF_USCIB2  9
#define TI_CC_SER_INTF_USCIB3  10
#define TI_CC_SER_INTF_USI     11
#define TI_CC_SER_INTF_BITBANG 12
