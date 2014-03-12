#include "uart.h"
#include "msp430.h"

void UART_Init(void)
{
    P3SEL |=BIT4+BIT5;//P3.4->TX  P3.5->RX
    UCA0CTL1 |= UCSSEL_2; //SMCLK
    UCA0BR0 = 69;                              // 8MHz 115200  8/115200=69.4444
    UCA0BR1 = 0;                              // 8MHz 115200
    UCA0MCTL = UCBRS1 + UCBRS0;               // Modulation UCBRSx=3 0.444*8=3.552 01010100
    UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
    IE2 |= UCA0RXIE;                          // Enable USCI_A0 RX interrupt
    _EINT();                                  //global interrupt enable
}

unsigned char  CheckSum(unsigned char *uBuff, unsigned char uBuffLen)
{
    unsigned char i,uSum=0;
    for(i=0;i<uBuffLen;i++)
    {
        uSum = uSum + uBuff[i];
    }
    uSum = (~uSum) + 1;
   if(uSum > 0xf0)
        uSum -= 16;
    return uSum;
}

int uart_send(struct Send_Data_struct *data)
{
    u8 *p=(u8 *)data;
    u8 i;
    for(i=0;i<9+UART_DATA_LEN;i++)
    {
        while (!(IFG2&UCA0TXIFG));
        UCA0TXBUF=*(p+i);
    }
    return 0;
}

struct Send_Data_struct send_data={
    .headaa=0xaa,
    .head55=0x55,
    .head={0xfe,0x55,0xaa}
};

struct Received_Data_struct uart_data;

#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
  static u8 index=0;
  if(uart_data.data_ready!=0)
      return;
  u8 *receive_data=(u8*)&uart_data;
  *(receive_data+index)=UCA0RXBUF;
  switch(index)
  {
  case 0 :
      if (*(receive_data+index)!=0xfe)
      {index=0;goto data_serial_end;}break;
  case 1 :
      if (*(receive_data+index)!=0x55)
      {index=0;goto data_serial_end;}break;
  case 2 :
      if (*(receive_data+index)!=0xaa)
      {index=0;goto data_serial_end;}break;
  case UART_DATA_LEN+4:
      //if(CheckSum(receive_data+3,UART_DATA_LEN+1)== *(receive_data+UART_DATA_LEN+4))
      {
          uart_data.data_ready=1;
      }
      index=0;
      goto data_serial_end;
  }
  index++;
data_serial_end:
  return;
}
