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
//  Demo Application for MSP430 eZ430-RF2500 / CC1100-2500 Interface
//  Main code application library v1.2
//
// W. Goh
// Version 1.2
// Texas Instruments, Inc
// December 2009
// Built with IAR Embedded Workbench Version: 4.20
//******************************************************************************
// Change Log:
//******************************************************************************
// Version:  1.2
// Comments: Add startup delay for startup difference between MSP430 and CCxxxx
// Version:  1.1
// Comments: Main application code designed for eZ430-RF2500 board
// Version:  1.00
// Comments: Initial Release Version
//******************************************************************************

#include "include.h"

extern char paTable[];
extern char paTableLen;

char txBuffer[12]={0x10,0x01,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09};
char rxBuffer[12];
unsigned int tick_time=0;
unsigned int i = 0;

void TIM_init(void)
{
  TACCR0 = 100 - 1;                         // PWM Period
  TACTL = TASSEL_2 + MC_3 + TAIE+ID_2;
  _EINT();
}

#pragma vector = TIMER0_A1_VECTOR
__interrupt void TA1_ISR(void)
{
  static int flg=0;
  switch( TA0IV )
  {
  case  2: break;                          // CCR1 not used
  case  4: break;                          // CCR2 not used
  case 10: 
    tick_time++;
  break;
  }
}

void main (void)
{
  WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
  //8Mhz
  if (CALBC1_8MHZ==0xFF)					// If calibration constants erased
  {											
    while(1);                               // do not load, trap CPU!!	
  }
  DCOCTL = 0;                               // Select lowest DCOx and MODx settings
  BCSCTL1 = CALBC1_8MHZ;                    // Set range
  DCOCTL = CALDCO_8MHZ;                     // Set DCO step + modulation

  // 5ms delay to compensate for time to startup between MSP430 and CC1100/2500
  __delay_cycles(5000);
  TIM_init();
  TI_CC_SPISetup();                         // Initialize SPI port

  P2SEL = 0;                                // Sets P2.6 & P2.7 as GPIO
  TI_CC_PowerupResetCCxxxx();               // Reset CCxxxx
  writeRFSettings();                        // Write RF settings to config reg
  TI_CC_SPIWriteBurstReg(TI_CCxxx0_PATABLE, paTable, paTableLen);//Write PATABLE

  // Configure ports -- switch inputs, LEDs, GDO0 to RX packet info from CCxxxx
  TI_CC_SW_PxREN = TI_CC_SW1;               // Enable Pull up resistor
  TI_CC_SW_PxOUT = TI_CC_SW1;               // Enable pull up resistor
  TI_CC_SW_PxIES = TI_CC_SW1;               // Int on falling edge
  TI_CC_SW_PxIFG &= ~(TI_CC_SW1);           // Clr flags
  TI_CC_SW_PxIE = TI_CC_SW1;                // Activate interrupt enables

  TI_CC_GDO0_PxIES |= TI_CC_GDO0_PIN;       // Int on falling edge (end of pkt)
  TI_CC_GDO0_PxIFG &= ~TI_CC_GDO0_PIN;      // Clear flag
  TI_CC_GDO0_PxIE |= TI_CC_GDO0_PIN;        // Enable int on end of packet

  TI_CC_SPIStrobe(TI_CCxxx0_SRX);           // Initialize CCxxxx in RX mode.
                                            // When a pkt is received, it will
                                            // signal on GDO0 and wake CPU
  while(1){}
 // __bis_SR_register(LPM3_bits + GIE);       // Enter LPM3, enable interrupts
}


// The ISR assumes the interrupt came from a pressed button
#pragma vector=PORT1_VECTOR
__interrupt void Port1_ISR (void)
{
  // If Switch was pressed
  if(TI_CC_SW_PxIFG & TI_CC_SW1)
  {
    int *p;
    // Build packet
    txBuffer[0] = 10;                        // Packet length
    txBuffer[1] = 0x01;                     // Packet address
    p=(int*)&txBuffer[2];
    *p=tick_time;
    RFSendPacket(txBuffer, 11);              // Send value over RF
    //__delay_cycles(5000);                   // Switch debounce
    TI_CC_SW_PxIFG &= ~(TI_CC_SW1);           // Clr flag that caused int
    return;
  }

   // if GDO fired
  if(TI_CC_GDO0_PxIFG & TI_CC_GDO0_PIN)
  {
    unsigned int *p;
    unsigned int timeA1,timeB,timeA2;
    char len=10;                             // Len of pkt to be RXed (only addr
                                            // plus data; size byte not incl b/c
                                            // stripped away within RX function)
    if (RFReceivePacket(rxBuffer+1,&len))     // Fetch packet from CCxxxx
    {
      rxBuffer[0] = 10;
     // p=(unsigned int*)&rxBuffer[6];
     // *p=tick_time;
      p=(unsigned int*)&rxBuffer[2];
      timeA1=*p;
      timeA2=tick_time;
      p=(unsigned int*)&rxBuffer[4];
      timeB=*p;
      
      
      
     
      //RFSendPacket(rxBuffer, 11);              // Send value over RF
      //__delay_cycles(5000);                   // Switch debounce
    }
    TI_CC_GDO0_PxIFG &= ~TI_CC_GDO0_PIN;      // After pkt RX, this flag is set.
    return;
  }
}
