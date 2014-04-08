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
//------------------------------------------------------------------------------
//  Description:  This file contains functions that allow the MSP430 device to
//  access the SPI interface of the CC1100/CC2500.  There are multiple
//  instances of each function; the one to be compiled is selected by the
//  system variable TI_CC_RF_SER_INTF, defined in "TI_CC_hardware_board.h".
//
//  MSP430/CC1100-2500 Interface Code Library v1.1
//
//  W. Goh
//  Texas Instruments, Inc.
//  December 2009
//  IAR Embedded Workbench v4.20
//------------------------------------------------------------------------------
// Change Log:
//------------------------------------------------------------------------------
// Version:  1.1
// Comments: Fixed several bugs where it is stuck in a infinite while loop
//           Added support for 5xx
//
// Version:  1.00
// Comments: Initial Release Version
//------------------------------------------------------------------------------

#include "include.h"
#include "TI_CC_spi.h"

//------------------------------------------------------------------------------
//  void TI_CC_SPISetup(void)
//
//  DESCRIPTION:
//  Configures the assigned interface to function as a SPI port and
//  initializes it.
//------------------------------------------------------------------------------
//  void TI_CC_SPIWriteReg(char addr, char value)
//
//  DESCRIPTION:
//  Writes "value" to a single configuration register at address "addr".
//------------------------------------------------------------------------------
//  void TI_CC_SPIWriteBurstReg(char addr, char *buffer, char count)
//
//  DESCRIPTION:
//  Writes values to multiple configuration registers, the first register being
//  at address "addr".  First data byte is at "buffer", and both addr and
//  buffer are incremented sequentially (within the CCxxxx and MSP430,
//  respectively) until "count" writes have been performed.
//------------------------------------------------------------------------------
//  char TI_CC_SPIReadReg(char addr)
//
//  DESCRIPTION:
//  Reads a single configuration register at address "addr" and returns the
//  value read.
//------------------------------------------------------------------------------
//  void TI_CC_SPIReadBurstReg(char addr, char *buffer, char count)
//
//  DESCRIPTION:
//  Reads multiple configuration registers, the first register being at address
//  "addr".  Values read are deposited sequentially starting at address
//  "buffer", until "count" registers have been read.
//------------------------------------------------------------------------------
//  char TI_CC_SPIReadStatus(char addr)
//
//  DESCRIPTION:
//  Special read function for reading status registers.  Reads status register
//  at register "addr" and returns the value read.
//------------------------------------------------------------------------------
//  void TI_CC_SPIStrobe(char strobe)
//
//  DESCRIPTION:
//  Special write function for writing to command strobe registers.  Writes
//  to the strobe at address "addr".
//------------------------------------------------------------------------------


// Delay function. # of CPU cycles delayed is similar to "cycles". Specifically,
// it's ((cycles-15) % 6) + 15.  Not exact, but gives a sense of the real-time
// delay.  Also, if MCLK ~1MHz, "cycles" is similar to # of useconds delayed.
void TI_CC_Wait(unsigned int cycles)
{
  while(cycles>15)                          // 15 cycles consumed by overhead
    cycles = cycles - 6;                    // 6 cycles consumed each iteration
}

//******************************************************************************
// If USI is used
//******************************************************************************

void TI_CC_SPISetup(void)
{
  TI_CC_CSn_PxOUT |= TI_CC_CSn_PIN;
  TI_CC_CSn_PxDIR |= TI_CC_CSn_PIN;         // /CS disable

  USICTL0 |= USIPE7 +  USIPE6 + USIPE5 + USIMST + USIOE;// Port, SPI master
  USICKCTL = USISSEL_2 + USICKPL    + USIDIV_2;           // SCLK = SMCLK
  USICTL0 &= ~USISWRST;                     // USI released for operation

  USISRL = 0x00;                            // Ensure SDO low instead of high,
  USICNT = 1;                               // to avoid conflict with CCxxxx
}

void TI_CC_SPIWriteReg(char addr, char value)
{
  TI_CC_CSn_PxOUT &= ~TI_CC_CSn_PIN;        // /CS enable
  while (TI_CC_SPI_USI_PxIN&TI_CC_SPI_USI_SOMI);// Wait for CCxxxx ready
  USISRL = addr;                            // Load address
  USICNT = 8;                               // Send it
  while (!(USICTL1&USIIFG));                // Wait for TX to finish
  USISRL = value;                           // Load value
  USICNT = 8;                               // Send it
  while (!(USICTL1&USIIFG));                // Wait for TX to finish
  TI_CC_CSn_PxOUT |= TI_CC_CSn_PIN;         // /CS disable
}

void TI_CC_SPIWriteBurstReg(char addr, char *buffer, char count)
{
  unsigned int i;

  TI_CC_CSn_PxOUT &= ~TI_CC_CSn_PIN;        // /CS enable
  while (TI_CC_SPI_USI_PxIN&TI_CC_SPI_USI_SOMI);// Wait for CCxxxx ready
  USISRL = addr | TI_CCxxx0_WRITE_BURST;    // Load address
  USICNT = 8;                               // Send it
  while (!(USICTL1&USIIFG));                // Wait for TX to finish
  for (i = 0; i < count; i++)
  {
    USISRL = buffer[i];                     // Load data
    USICNT = 8;                             // Send it
    while (!(USICTL1&USIIFG));              // Wait for TX to finish
  }
  TI_CC_CSn_PxOUT |= TI_CC_CSn_PIN;         // /CS disable
}

char TI_CC_SPIReadReg(char addr)
{
  char x;

  TI_CC_CSn_PxOUT &= ~TI_CC_CSn_PIN;        // /CS enable
  while (TI_CC_SPI_USI_PxIN&TI_CC_SPI_USI_SOMI);// Wait for CCxxxx ready
  USISRL = addr | TI_CCxxx0_READ_SINGLE;    // Load address
  USICNT = 8;                               // Send it
  while (!(USICTL1&USIIFG));                // Wait for TX to finish
  USICNT = 8;                               // Dummy write so we can read data
  while (!(USICTL1&USIIFG));                // Wait for RX to finish
  x = USISRL;                               // Store data
  TI_CC_CSn_PxOUT |= TI_CC_CSn_PIN;         // /CS disable

  return x;
}

void TI_CC_SPIReadBurstReg(char addr, char *buffer, char count)
{
  unsigned int i;

  TI_CC_CSn_PxOUT &= ~TI_CC_CSn_PIN;        // /CS enable
  while (TI_CC_SPI_USI_PxIN&TI_CC_SPI_USI_SOMI);// Wait for CCxxxx ready
  USISRL = addr | TI_CCxxx0_READ_BURST;     // Load address
  USICNT = 8;                               // Send it
  while (!(USICTL1&USIIFG));                // Wait for TX to finish
  for (i = 0; i < count; i++)
  {
    USICNT = 8;                             // Dummy write so we can read data
    while (!(USICTL1&USIIFG));              // Wait for RX to finish
    buffer[i] = USISRL;                     // Store data
  }
  TI_CC_CSn_PxOUT |= TI_CC_CSn_PIN;         // /CS disable
}

char TI_CC_SPIReadStatus(char addr)
{
  char x;

  TI_CC_CSn_PxOUT &= ~TI_CC_CSn_PIN;        // /CS enable
  while (TI_CC_SPI_USI_PxIN&TI_CC_SPI_USI_SOMI);// Wait for CCxxxx ready
  USISRL = addr | TI_CCxxx0_READ_BURST;     // Load address
  USICNT = 8;                               // Send it
  while (!(USICTL1&USIIFG));                // Wait for TX to finish
  USICNT = 8;                               // Dummy write so we can read data
  while (!(USICTL1&USIIFG));                // Wait for RX to finish
  x = USISRL;                               // Store data
  TI_CC_CSn_PxOUT |= TI_CC_CSn_PIN;         // /CS disable

  return x;
}

void TI_CC_SPIStrobe(char strobe)
{
  TI_CC_CSn_PxOUT &= ~TI_CC_CSn_PIN;        // /CS enable
  while (TI_CC_SPI_USI_PxIN&TI_CC_SPI_USI_SOMI);// Wait for CCxxxx ready
  USISRL = strobe;                          // Load strobe
  USICNT = 8;                               // Send it
  while (!(USICTL1&USIIFG));                // Wait for TX to finish
  TI_CC_CSn_PxOUT |= TI_CC_CSn_PIN;         // /CS disable
}

void TI_CC_PowerupResetCCxxxx(void)
{
  // Sec. 27.1 of CC1100 datasheet
  TI_CC_CSn_PxOUT |= TI_CC_CSn_PIN;
  TI_CC_Wait(30);
  TI_CC_CSn_PxOUT &= ~TI_CC_CSn_PIN;
  TI_CC_Wait(30);
  TI_CC_CSn_PxOUT |= TI_CC_CSn_PIN;
  TI_CC_Wait(45);

  TI_CC_CSn_PxOUT &= ~TI_CC_CSn_PIN;
  while (TI_CC_SPI_USI_PxIN&TI_CC_SPI_USI_SOMI);
  USISRL = TI_CCxxx0_SRES;
  USICNT = 8;
  while (!(USICTL1&USIIFG));
  while (TI_CC_SPI_USI_PxIN&TI_CC_SPI_USI_SOMI);
  TI_CC_CSn_PxOUT |= TI_CC_CSn_PIN;
}

// End of USI

