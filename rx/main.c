#include "msp430.h"

unsigned int dist;
void bc(void)
{
    unsigned int i;
    BCSCTL1 |= XTS;                           // LFXT1 = HF XTAL
     BCSCTL3 |= LFXT1S1;                       // LFXT1S1 = 3-16Mhz

     do
     {
       IFG1 &= ~OFIFG;                         // Clear OSCFault flag
       for (i = 0xFF; i > 0; i--);             // Time for flag to set
     }
     while (IFG1 & OFIFG);                     // OSCFault flag still set?
     BCSCTL2 |= SELM_3+SELS;                        // MCLK= LFXT1 SMCLK=LFXT1
}

void TIM_init(void)
{
    TA1CTL|=TACLR;
    TA1CCR0 = 60000 - 1;                         // PWM Period
    TA1CTL = TASSEL_1+ID_3+TAIE;    // ACLK, up mode
}

void IO_init(void)
{
    P1DIR&=~(BIT0+BIT1);
    P1IE|=BIT0+BIT1;
    P1IES|=BIT0+BIT1;
    P1IFG&=~(BIT0+BIT1);
}

#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
    unsigned int count;
    if((P1IFG&BIT0)==BIT0)//if get start signal
    {
        TA1CTL&=~MC_1; //stop timer
        TA1R=0;//clear timer
        TA1CTL|=MC_1; //start timer
        P1IE|=BIT1;
        P1IFG&=~BIT0;
    }
    if((P1IFG&BIT1)==BIT1)
    {
        TA1CTL&=~MC_1; //stop timer
        count=TA1R;
        dist=340*count/1000;
        P1IE&=~BIT1;  //stop catching sound.just want the first sound
        P1IFG&=~BIT1;
    }
}

#pragma vector = TIMER1_A1_VECTOR
__interrupt void TA1_ISR(void)
{
    switch(TA1IV)//TA1IV is read clear so it must be read only once.
    {
    case 0x0a:
        P1IE&=~BIT1;  //timeout,stop catching sound.
        TA1CTL&=~MC_1; //stop timer
        TA1R=0;//clear timer
        break;
    default:
        break;
    }
}

int main (void)
{
    WDTCTL = WDTPW + WDTHOLD;
    bc();
    TIM_init();
    IO_init();
    _EINT();
    while(1)
    {
        LPM1;
    }
    return 0;
}
