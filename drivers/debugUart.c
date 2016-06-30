#include "stm8l15x.h"
#include "boardiodef.h"
#include "debugUart.h"

void debugUartInit(void)
{
	//init debug usart1 pin
	/* USART configured as follow:
	- BaudRate = 115200 baud  
	- Word Length = 8 Bits
	- One Stop Bit
	- No parity
	- Receive and transmit enabled
	- USART Clock disabled
	*/
	 /* Enable USART clock */
	CLK_PeripheralClockConfig(CLK_Peripheral_USART1, ENABLE);

	/* Configure USART Tx as alternate function push-pull  (software pull up)*/
	GPIO_ExternalPullUpConfig(USART1_TX_PORT, USART1_TX_PIN, ENABLE);

	/* Configure USART Rx as alternate function push-pull  (software pull up)*/
	GPIO_ExternalPullUpConfig(USART1_RX_PORT, USART1_RX_PIN, ENABLE);

	/* USART configuration */
	USART_Init(USART1, 
			  (uint32_t)115200,
			  USART_WordLength_8b,
			  USART_StopBits_1,
			  USART_Parity_No,
			  (USART_Mode_TypeDef)(USART_Mode_Tx | USART_Mode_Rx));


	/* Enable the USART Receive interrupt: this interrupt is generated when the USART
	receive data register is not empty */
	//USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	
	/* Enable the USART Transmit complete interrupt: this interrupt is generated when the USART
	transmit Shift Register is empty */
	//USART_ITConfig(USART1, USART_IT_TC, ENABLE);

	/* Enable USART */
	USART_Cmd(USART1, ENABLE);
}


int dbgSendChar(int ch) 
{
	while (!(USART1->SR & USART_FLAG_TXE)); // USART1 可换成你程序中通信的串口
	/* Transmit Data */
	USART1->DR = (uint8_t)ch;
	return (ch);
}


