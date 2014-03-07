#ifndef _USART_H_
#define _USART_H_
#define u8 unsigned char
#define u16 unsigned int

#define UART_DATA_LEN 8

struct Received_Data_struct
{
  u8 head[3];
  u8 cmd;
  u8 data[UART_DATA_LEN];
  u8 check;
  u8 data_ready;
};
#pragma pack(push)
#pragma pack(1)
struct Send_Data_struct
{
    u8 headaa;
    u16 add;
    u8 head55;
    u8 head[3];
    u8 cmd;
    u8 data[UART_DATA_LEN];
    u8 check;
};
#pragma pack(pop)
//public function
void UART_Init(void);
int uart_send(struct Send_Data_struct *data);
//public variable
extern struct Received_Data_struct uart_data;
extern struct Send_Data_struct send_data;
#endif
