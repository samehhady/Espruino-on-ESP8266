//Stupid bit of code that does the bare minimum to make os_printf work.

/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Jeroen Domburg <jeroen@spritesmods.com> wrote this file. As long as you retain 
 * this notice you can do whatever you want with this stuff. If we meet some day, 
 * and you think this stuff is worth it, you can buy me a beer in return. 
 * ----------------------------------------------------------------------------
 */


#include "espmissingincludes.h"
#include "ets_sys.h"
#include "osapi.h"
#include "uart_hw.h"

static void ICACHE_FLASH_ATTR stdoutUartTxd(char c) {
	//Wait until there is room in the FIFO
	while (((READ_PERI_REG(UART_STATUS(0))>>UART_TXFIFO_CNT_S)&UART_TXFIFO_CNT)>=126) ;
	//Send the character
	WRITE_PERI_REG(UART_FIFO(0), c);
}

static void ICACHE_FLASH_ATTR stdoutPutchar(char c) {
	//convert \n -> \r\n
	if (c=='\n') stdoutUartTxd('\r');
	stdoutUartTxd(c);
}

//LOCAL
static void uart0_rx_intr_handler(void *param) {
	os_printf("recieved");
	return;
	/* uart0 and uart1 intr combine togther, when interrupt occur, see reg 0x3ff20020, bit2, bit0 represents
	 * uart1 and uart0 respectively
	 */
	RcvMsgBuff *pRxBuff = (RcvMsgBuff *)param;
	uint8 RcvChar;
	
	if (UART_RXFIFO_FULL_INT_ST != (READ_PERI_REG(UART_INT_ST(UART0)) & UART_RXFIFO_FULL_INT_ST)) {
		return;
	}
	
	WRITE_PERI_REG(UART_INT_CLR(UART0), UART_RXFIFO_FULL_INT_CLR);
	
	while (READ_PERI_REG(UART_STATUS(UART0)) & (UART_RXFIFO_CNT << UART_RXFIFO_CNT_S)) {
		RcvChar = READ_PERI_REG(UART_FIFO(UART0)) & 0xFF;
		
		/* you can add your handle code below.*/
		
		*(pRxBuff->pWritePos) = RcvChar;
		
		// insert here for get one command line from uart
		if (RcvChar == '\r') {
			os_printf("Recieved: %s", "neki");
			pRxBuff->BuffState = WRITE_OVER;
		}
		
		pRxBuff->pWritePos++;
		
		if (pRxBuff->pWritePos == (pRxBuff->pRcvMsgBuff + RX_BUFF_SIZE)) {
			// overflow ...we may need more error handle here.
			pRxBuff->pWritePos = pRxBuff->pRcvMsgBuff ;
		}
	}
}

extern UartDevice UartDev;

void stdoutInit() {
	// init recerive handler
	/* rcv_buff size if 0x100 */
	ETS_UART_INTR_ATTACH(uart0_rx_intr_handler,  &(UartDev.rcv_buff));
//	ets_isr_attach(ETS_UART_INUM, uart0_rx_intr_handler, &(UartDev.rcv_buff));

	//Enable TxD pin
	PIN_PULLUP_DIS(PERIPHS_IO_MUX_U0TXD_U);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0TXD_U, FUNC_U0TXD);

	//Set baud rate and other serial parameters to 115200,n,8,1
	uart_div_modify(0, UART_CLK_FREQ/BIT_RATE_115200);
//	uart_div_modify(0, UART_CLK_FREQ/BIT_RATE_460800);
	WRITE_PERI_REG(UART_CONF0(0),
				   UartDev.exist_parity |
				   UartDev.parity |
//				   (STICK_PARITY_DIS) |
				   (ONE_STOP_BIT << UART_STOP_BIT_NUM_S) |
				   (EIGHT_BITS << UART_BIT_NUM_S)
	);

	//Reset tx & rx fifo
	SET_PERI_REG_MASK(UART_CONF0(0), UART_RXFIFO_RST|UART_TXFIFO_RST);
	CLEAR_PERI_REG_MASK(UART_CONF0(0), UART_RXFIFO_RST|UART_TXFIFO_RST);

	//set rx fifo trigger
	WRITE_PERI_REG(UART_CONF1(0), (UartDev.rcv_buff.TrigLvl & UART_RXFIFO_FULL_THRHD) << UART_RXFIFO_FULL_THRHD_S);
	
	//Clear pending interrupts
	WRITE_PERI_REG(UART_INT_CLR(0), 0xffff);

	//enable rx_interrupt
	SET_PERI_REG_MASK(UART_INT_ENA(0), UART_RXFIFO_FULL_INT_ENA);

	//Install our own putchar handler
	os_install_putc1((void *)stdoutPutchar);
}

/*

uart_config(uint8 uart_no)
{
	if (uart_no == UART1) {
		PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_U1TXD_BK);
	} else {
		ETS_UART_INTR_ATTACH(uart0_rx_intr_handler,  &(UartDev.rcv_buff));
		PIN_PULLUP_DIS(PERIPHS_IO_MUX_U0TXD_U);
		PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0TXD_U, FUNC_U0TXD);
	}
	
	uart_div_modify(uart_no, UART_CLK_FREQ / (UartDev.baut_rate));
	
	WRITE_PERI_REG(UART_CONF0(uart_no),    UartDev.exist_parity
				   | UartDev.parity
				   | (UartDev.stop_bits << UART_STOP_BIT_NUM_S)
				   | (UartDev.data_bits << UART_BIT_NUM_S));
	
	
	//clear rx and tx fifo,not ready
	SET_PERI_REG_MASK(UART_CONF0(uart_no), UART_RXFIFO_RST | UART_TXFIFO_RST);
	CLEAR_PERI_REG_MASK(UART_CONF0(uart_no), UART_RXFIFO_RST | UART_TXFIFO_RST);
	
	//set rx fifo trigger
	WRITE_PERI_REG(UART_CONF1(uart_no), (UartDev.rcv_buff.TrigLvl & UART_RXFIFO_FULL_THRHD) << UART_RXFIFO_FULL_THRHD_S);
	
	//clear all interrupt
	WRITE_PERI_REG(UART_INT_CLR(uart_no), 0xffff);
	//enable rx_interrupt
	SET_PERI_REG_MASK(UART_INT_ENA(uart_no), UART_RXFIFO_FULL_INT_ENA);
}

*/
