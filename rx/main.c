#include "msp430.h"
#include "uart.h"

u16 tick_time=0;

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
    TA1CCR0 = 100 - 1;              // Period
    TA1CTL = TASSEL_1+ID_3+TAIE;    // ACLK, up mode
    TA1CTL|=MC_1;
    _EINT();
}

void IO_init(void)
{
    P1DIR&=~(BIT1);
    P1IE|=BIT1;
    P1IES|=BIT1;
    P1IFG&=~(BIT1);
    _EINT();
}

#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
    unsigned int count=0;
    if((P1IFG&BIT1)==BIT1)
    {
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
    case 0x0a: //overflow
        tick_time++;
        break;
    default:
        break;
    }
}

void send_timer_sync(void)
{
    u16 *pOut;
    send_data.add=0x0100; //low byte is in front or:0x0100
    send_data.cmd=0x01;//reqest time
    pOut=(u16 *)&send_data.data;
    *pOut=tick_time;
    uart_send(&send_data);
}

int main (void)
{
    u16 *pIn,*pOut,cnt;
    u16 t[6];
    long timeA1=0,timeB=0,timeA2=0,dt=0,sum_of_time;
    WDTCTL = WDTPW + WDTHOLD;
    bc();
    TIM_init();
    IO_init();
    UART_Init();
    while(1)
    {
        if(uart_data.data_ready==1)
        {
            if(uart_data.cmd==0x00)
            {
                cnt=0;
                sum_of_time=0;
                send_timer_sync();
            }
            if(uart_data.cmd==0x02)
            {
                pIn=(u16 *)(&uart_data.data[0]);
                timeA2=(tick_time>*pIn)?tick_time:(65536+tick_time);
                //timeA1=(*pIn);
                timeA1=uart_data.data[0];
                timeA1=uart_data.data[1]<<8;
                timeB=(*(pIn+1)>*pIn)?*(pIn+1):(65536+*(pIn+1));
                dt=(timeA2-timeA1)/2;//round trip time
                if(dt<350&&dt>100)
                {
                    t[cnt]=timeA1-timeB+dt;
                    sum_of_time+=t[cnt];
                    cnt++;
                    t[cnt]=timeA2-timeB-2*dt;
                    sum_of_time+=t[cnt];
                    cnt++;
                    if(cnt<6)
                       send_timer_sync();
                    else  //get enough data to caculate time
                    {
                        tick_time=(u16)((sum_of_time/6)+tick_time)%0xffff;
                        pOut=(u16 *)(&send_data.data[0]);
                        send_data.add=0x0000; //low byte is in front to host
                        send_data.cmd=0xf1;//reqest time
                        *pOut=(sum_of_time/6)%0xffff;
                        *(pOut+1)=tick_time;
                        *(pOut+2)=(timeA2%0xffff);
                        *(pOut+3)=(timeB%0xffff);
                        uart_send(&send_data);
                    }
                }

            }
            uart_data.data_ready=0;
        }
    }
    return 0;
}
