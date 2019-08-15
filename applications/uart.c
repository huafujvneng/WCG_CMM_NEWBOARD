/**************************************************************************
Copyright (C), 2012, XJ ELECTRIC Co., LTD.
文件名  ：  uart.c
作者    ：  wuzhanwei
项目名称：  WGB-600A
功能    ：  485通信处理程序
创建日期：  2012年9月1日
备注    ：
修改记录：
**************************************************************************/

#include "uart.h"
#include "board.h"

extern rt_uint32_t baudrate_0, baudrate_1, baudrate_3, baudrate_4, baudrate_5, baudrate_6, baudrate_7;
extern rt_uint16_t parity_0, parity_1, parity_3, parity_4, parity_5, parity_6, parity_7;
extern rt_uint16_t protocol_0, protocol_1, protocol_3, protocol_4, protocol_5, protocol_6, protocol_7;

extern struct  UART_CONF uartpara[UARTMAXNUM];
extern struct  UART_TXRX_STATUS uart_status[UARTMAXNUM];
/* 信号量控制块 */
struct rt_semaphore uart0_sem;
struct rt_semaphore uart1_sem;

struct rt_semaphore uart3_sem;
struct rt_semaphore uart4_sem;
struct rt_semaphore uart5_sem;
struct rt_semaphore uart6_sem;
struct rt_semaphore uart7_sem;




uint8_t uart_dma_status;
uint16_t uart0errcount, uart1errcount, uart3errcount, uart4errcount, uart5errcount, uart6errcount, uart7errcount;
uint8_t uart0trascount, uart1trascount, uart3trascount, uart4trascount, uart5trascount, uart6trascount, uart7trascount;
/**************************************************************************
目的/功能：串口初始化
全局变量：
输入参数：
输出参数：
调用关系：
**************************************************************************/
static rt_err_t rt_uart_init(rt_device_t dev)
{
	struct uart_device* uart = (struct uart_device*) dev->user_data;

	if (!(dev->flag & RT_DEVICE_FLAG_ACTIVATED))
	{
		if (dev->flag & RT_DEVICE_FLAG_INT_RX)
		{
			rt_memset(uart->int_rx->rx_buffer, 0,
				sizeof(uart->int_rx->rx_buffer));
			uart->int_rx->read_index = 0;
			uart->int_rx->save_index = 0;
			uart->int_rx->rx_length = 0;
			uart->int_rx->receive_flg = 0;
			uart->int_rx->rx_end_tick = rt_tick_get();

			uart->int_rx->MsgTransUpFlagJK1 = 0;
			uart->int_rx->MeasureTime = 0;
			uart->int_rx->ImageCtrCodeJK1 = 0;
			uart->int_rx->Measure_Data_RDY = 0;
			uart->int_rx->Gen_Insp_IndexJK1 = 0;
			uart->int_rx->Gen_Din_IndexJK1 = 0;
		}

		if (dev->flag & RT_DEVICE_FLAG_INT_TX)
		{
			rt_memset(uart->int_tx->tx_buffer, 0,
				sizeof(uart->int_tx->tx_buffer));
			uart->int_tx->tx_buffer[0] = 0x10;//缓冲区放一个初始值
			uart->int_tx->tx_buffer[1] = 0x20;
			uart->int_tx->tx_buffer[2] = zzaddr;
			uart->int_tx->tx_buffer[3] = zzaddr + 0x20;
			uart->int_tx->tx_buffer[4] = 0x10;

			uart->int_tx->write_index = 0;
			uart->int_tx->save_index = 0;
			uart->int_tx->tx_end_tick = rt_tick_get();
		}

		dev->flag |= RT_DEVICE_FLAG_ACTIVATED;
	}

	return RT_EOK;
}

/**************************************************************************
目的/功能：把接收到的数据存到缓冲区
全局变量：
输入参数：
输出参数：
调用关系：
**************************************************************************/
uint8_t temp_databuf[10];
static void rt_uart_save(struct uart_device* uart, rt_size_t size)
{

	uint8_t* ptr;

	while (size)
	{

		ptr = (uint8_t*) & (uart->uart_device->RDR);//
		uart->int_rx->rx_buffer[uart->int_rx->save_index] = *ptr;

		uart->int_rx->save_index++;
		ptr++;
		size--;
		temp_databuf[size] = uart->uart_device->RDR;
		if (uart->int_rx->save_index >= RS485_RX_BUFFER_SIZE)
			uart->int_rx->save_index = 0;

		/* if the next position is read index, discard this 'read char' */
		if (uart->int_rx->save_index == uart->int_rx->read_index)
		{
			uart->int_rx->read_index++;
			if (uart->int_rx->read_index >= RS485_RX_BUFFER_SIZE)
				uart->int_rx->read_index = 0;
		}
		if (size == 0)
		{
			/* get rx length */
			uart->int_rx->rx_length = uart->int_rx->read_index > uart->int_rx->save_index ?
				RS485_RX_BUFFER_SIZE - uart->int_rx->read_index + uart->int_rx->save_index :
				uart->int_rx->save_index - uart->int_rx->read_index;
		}
	}
}

/**************************************************************************
目的/功能：打开串口
全局变量：
输入参数：
输出参数：
调用关系：
**************************************************************************/
static rt_err_t rt_uart_open(rt_device_t dev, uint16_t oflag)
{
	struct uart_device* uart;

	//    RT_ASSERT(dev != RT_NULL);
	uart = (struct uart_device*) dev->user_data;

	/* set interrupt priority */
	NVIC_SetPriority(uart->rx_irq, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 3, 1));
	/* enable interrupt */
	UART_ENABLE_IRQ(uart->rx_irq);

	return RT_EOK;
}


/**************************************************************************
目的/功能：把数据存到发送缓冲区并发出去
全局变量：
输入参数：
输出参数：
调用关系：
**************************************************************************/
extern struct uart_device uart0;
extern struct uart_device uart1;
extern struct uart_device uart3;
extern struct uart_device uart4;
extern struct uart_device uart5;
extern struct uart_device uart6;
extern struct uart_device uart7;
uint8_t uart_dma_status;

static rt_size_t rt_uart_write(rt_device_t dev, rt_off_t pos,
	const void* buffer, rt_size_t size)
{
	rt_err_t err_code;

	uint32_t src;
	uint32_t des;

	struct uart_device* uart;

	err_code = RT_EOK;
	uart = (struct uart_device*)dev->user_data;

	uart_dma_status = 1;
	if (uart == &uart0)//
	{
		src = (uint32_t)& uart->int_tx->tx_buffer;

		DMA_MFS(ch0_Tx, DMA_ch_0, 0, src, des, size);//
		bFM3_MFS0_UART_SCR_TBIE = 1;//SCR_TBIE
	}
	//return;
	if (uart == &uart1)//
	{
		src = (uint32_t)& uart->int_tx->tx_buffer;

		DMA_MFS(ch1_Tx, DMA_ch_1, 0, src, des, size);//
		bFM3_MFS1_UART_SCR_TBIE = 1;//SCR_TBIE
	}

	if (uart == &uart3)//
	{
		src = (uint32_t)& uart->int_tx->tx_buffer;

		// src = (uint32_t)&(uart->int_tx->tx_buffer);
		DMA_MFS(ch3_Tx, DMA_ch_3, 0, src, des, size);
		bFM3_MFS3_UART_SCR_TBIE = 1;//SCR_TBIE
	}
	if (uart == &uart4)//
	{
		src = (uint32_t)& uart->int_tx->tx_buffer;
		//src = (uint32_t)&uart4.int_tx->tx_buffer;
		DMA_MFS(ch4_Tx, DMA_ch_4, 0, src, des, size);//-1
		bFM3_MFS4_UART_SCR_TBIE = 1;//SCR_TBIE

	}

	if (uart == &uart5)//
	{
		src = (uint32_t)& uart->int_tx->tx_buffer;
		//src = (uint32_t)&(uart->int_tx->tx_buffer);
		DMA_MFS(ch5_Tx, DMA_ch_5, 0, src, des, size);//
		bFM3_MFS5_UART_SCR_TBIE = 1;//SCR_TBIE
	}

	if (uart == &uart6)//
	{
		src = (uint32_t)& uart->int_tx->tx_buffer;
		//src = (uint32_t)&(uart->int_tx->tx_buffer);
		DMA_MFS(ch6_Tx, DMA_ch_6, 0, src, des, size);//
		bFM3_MFS6_UART_SCR_TBIE = 1;//SCR_TBIE
	}

	if (uart == &uart7)//
	{
		src = (uint32_t)& uart->int_tx->tx_buffer;
		//src = (uint32_t)&(uart->int_tx->tx_buffer);
		DMA_MFS(ch7_Tx, DMA_ch_7, 0, src, des, size);//
		bFM3_MFS7_UART_SCR_TBIE = 1;//SCR_TBIE
	}

	uart->int_tx->write_index = uart->int_tx->save_index = 0;//每次发送均是从缓冲区第一个字节开始

	/* set error code */
	rt_set_errno(err_code);
	uart_dma_status = 0;

	return size;
}

/**************************************************************************
目的/功能：注册串口
全局变量：
输入参数：
输出参数：
调用关系：
**************************************************************************/
rt_err_t rt_hw_uart_register(rt_device_t device, const char* name,
	uint32_t flag, struct uart_device* uart)
{
	//    RT_ASSERT(device != RT_NULL);

	device->type = RT_Device_Class_Char;
	device->rx_indicate = RT_NULL;
	device->tx_complete = RT_NULL;
	device->init = rt_uart_init;
	device->open = rt_uart_open;
	device->close = RT_NULL;
	device->read = RT_NULL;
	device->write = rt_uart_write;
	device->control = RT_NULL;
	device->user_data = uart;

	/* register a character device */
	return rt_device_register(device, name, RT_DEVICE_FLAG_RDWR | flag);
}

/**************************************************************************
目的/功能：接收中断处理
全局变量：
输入参数：
输出参数：
调用关系：
**************************************************************************/
uint8_t uart0_index = 0;
void rt_hw_uart0_isr(rt_device_t device)
{
	uint8_t rx_size;
	uint8_t dummy = 0;

	struct uart_device* uart = (struct uart_device*) device->user_data;

	if ((uart->uart_device->SSR & 0x38) > 0)//(dummy > 0)//接收出错处理
	{
		uart->uart_device->SSR |= SSR_REC;
		dummy = uart->uart_device->RDR;
		dummy++;


		uart->int_rx->receive_flg = 0;
		uart->int_rx->rx_length = 0;
		uart->int_rx->read_index = uart->int_rx->save_index = 0;//每次接收均是从缓冲区第一个字节开始

		return;
	}

	while (uart->uart_device->SSR & 0x04)//SSR的RDRF位为1时，bFM3_MFS4_UART_SSR_RDRF
	{

		rx_size = 1;//uart->uart_device->FBYTE2;

		rt_uart_save(uart, rx_size);
	}
	// uart0_databuf[0] = uart0->int_rx->rx_buffer[6];
	rt_sem_release(&uart0_sem);


}


void rt_hw_uart1_isr(rt_device_t device)
{
	uint8_t rx_size;
	uint8_t dummy = 0;

	struct uart_device* uart = (struct uart_device*) device->user_data;

	if ((uart->uart_device->SSR & 0x38) > 0)//(dummy > 0)//接收出错处理
	{
		uart->uart_device->SSR |= SSR_REC;
		dummy = uart->uart_device->RDR;
		dummy++;


		uart->int_rx->receive_flg = 0;
		uart->int_rx->rx_length = 0;
		uart->int_rx->read_index = uart->int_rx->save_index = 0;//每次接收均是从缓冲区第一个字节开始

		return;
	}

	while (uart->uart_device->SSR & 0x04)//SSR的RDRF位为1时，bFM3_MFS4_UART_SSR_RDRF
	{

		rx_size = 1;

		rt_uart_save(uart, rx_size);
	}

	rt_sem_release(&uart1_sem);


}


void rt_hw_uart3_isr(rt_device_t device)
{
	uint8_t rx_size;
	uint8_t dummy = 0;

	struct uart_device* uart = (struct uart_device*) device->user_data;

	if ((uart->uart_device->SSR & 0x38) > 0)//(dummy > 0)//接收出错处理
	{
		uart->uart_device->SSR |= SSR_REC;
		dummy = uart->uart_device->RDR;
		dummy++;


		uart->int_rx->receive_flg = 0;
		uart->int_rx->rx_length = 0;
		uart->int_rx->read_index = uart->int_rx->save_index = 0;//每次接收均是从缓冲区第一个字节开始

		return;
	}

	while (uart->uart_device->SSR & 0x04)//SSR的RDRF位为1时，bFM3_MFS4_UART_SSR_RDRF
	{

		rx_size = 1;

		rt_uart_save(uart, rx_size);
	}

	rt_sem_release(&uart3_sem);


}


void rt_hw_uart4_isr(rt_device_t device)
{
	uint8_t rx_size;
	uint8_t dummy = 0;

	struct uart_device* uart = (struct uart_device*) device->user_data;

	if ((uart->uart_device->SSR & 0x38) > 0)//(dummy > 0)//接收出错处理
	{
		uart->uart_device->SSR |= SSR_REC;
		dummy = uart->uart_device->RDR;
		dummy++;


		uart->int_rx->receive_flg = 0;
		uart->int_rx->rx_length = 0;
		uart->int_rx->read_index = uart->int_rx->save_index = 0;//每次接收均是从缓冲区第一个字节开始

		return;
	}

	while (uart->uart_device->SSR & 0x04)//SSR的RDRF位为1时，bFM3_MFS4_UART_SSR_RDRF
	{

		rx_size = 1;

		rt_uart_save(uart, rx_size);
	}

	rt_sem_release(&uart4_sem);


}

void rt_hw_uart5_isr(rt_device_t device)
{
	uint8_t rx_size;
	uint8_t dummy = 0;

	struct uart_device* uart = (struct uart_device*) device->user_data;

	if ((uart->uart_device->SSR & 0x38) > 0)//(dummy > 0)//接收出错处理
	{
		uart->uart_device->SSR |= SSR_REC;
		dummy = uart->uart_device->RDR;
		dummy++;


		uart->int_rx->receive_flg = 0;
		uart->int_rx->rx_length = 0;
		uart->int_rx->read_index = uart->int_rx->save_index = 0;//每次接收均是从缓冲区第一个字节开始

		return;
	}

	while (uart->uart_device->SSR & 0x04)//SSR的RDRF位为1时，bFM3_MFS4_UART_SSR_RDRF
	{

		rx_size = 1;

		rt_uart_save(uart, rx_size);
	}

	rt_sem_release(&uart5_sem);


}


void rt_hw_uart6_isr(rt_device_t device)
{
	uint8_t rx_size;
	uint8_t dummy = 0;

	struct uart_device* uart = (struct uart_device*) device->user_data;

	if ((uart->uart_device->SSR & 0x38) > 0)//(dummy > 0)//接收出错处理
	{
		uart->uart_device->SSR |= SSR_REC;
		dummy = uart->uart_device->RDR;
		dummy++;


		uart->int_rx->receive_flg = 0;
		uart->int_rx->rx_length = 0;
		uart->int_rx->read_index = uart->int_rx->save_index = 0;//每次接收均是从缓冲区第一个字节开始

		return;
	}

	while (uart->uart_device->SSR & 0x04)//SSR的RDRF位为1时，bFM3_MFS4_UART_SSR_RDRF
	{

		rx_size = 1;

		rt_uart_save(uart, rx_size);
	}

	rt_sem_release(&uart6_sem);


}


void rt_hw_uart7_isr(rt_device_t device)
{
	uint8_t rx_size;
	uint8_t dummy = 0;

	struct uart_device* uart = (struct uart_device*) device->user_data;

	if ((uart->uart_device->SSR & 0x38) > 0)//(dummy > 0)//接收出错处理
	{
		uart->uart_device->SSR |= SSR_REC;
		dummy = uart->uart_device->RDR;
		dummy++;


		uart->int_rx->receive_flg = 0;
		uart->int_rx->rx_length = 0;
		uart->int_rx->read_index = uart->int_rx->save_index = 0;//每次接收均是从缓冲区第一个字节开始

		return;
	}

	while (uart->uart_device->SSR & 0x04)//SSR的RDRF位为1时，bFM3_MFS4_UART_SSR_RDRF
	{

		rx_size = 1;

		rt_uart_save(uart, rx_size);
	}

	rt_sem_release(&uart7_sem);


}



/**************************************************************************
目的/功能：串口初始化结构体定义
全局变量：
输入参数：
输出参数：
调用关系：
**************************************************************************/
#ifdef RT_USING_UART0
/* UART0 device driver structure */
#define UART0	FM3_MFS0_UART
struct uart_int_rx uart0_int_rx;
struct uart_int_tx uart0_int_tx;
struct uart_device uart0 =
{
	UART0,
	MFS0RX_IRQn,
	MFS0TX_IRQn,
	&uart0_int_rx,
	&uart0_int_tx,

};
struct rt_device uart0_device;

void MFS0RX_IRQHandler(void)
{
	/* enter interrupt */
	rt_interrupt_enter();
	rt_hw_uart0_isr(&uart0_device);
	/* leave interrupt */
	rt_interrupt_leave();
}
#endif /**< #ifdef RT_USING_UART0 */

#ifdef RT_USING_UART1
/* UART1 device driver structure */
#define UART1	FM3_MFS1_UART
struct uart_int_rx uart1_int_rx;
struct uart_int_tx uart1_int_tx;
struct uart_device uart1 =
{
	UART1,
	MFS1RX_IRQn,
	MFS1TX_IRQn,
	&uart1_int_rx,
	&uart1_int_tx,
};
struct rt_device uart1_device;

void MFS1RX_IRQHandler(void)
{
	/* enter interrupt */
	rt_interrupt_enter();
	rt_hw_uart1_isr(&uart1_device);
	/* leave interrupt */
	rt_interrupt_leave();
}
#endif /**< #ifdef RT_USING_UART1 */





#ifdef RT_USING_UART3
/* UART3 device driver structure */
#define UART3	FM3_MFS3_UART
struct uart_int_rx uart3_int_rx;
struct uart_int_tx uart3_int_tx;
struct uart_device uart3 =
{
	UART3,
	MFS3RX_IRQn,
	MFS3TX_IRQn,
	&uart3_int_rx,
	&uart3_int_tx,
};
struct rt_device uart3_device;

void MFS3RX_IRQHandler(void)
{
	/* enter interrupt */
	rt_interrupt_enter();
	rt_hw_uart3_isr(&uart3_device);
	/* leave interrupt */
	rt_interrupt_leave();
}
#endif /**< #ifdef RT_USING_UART3 */


#ifdef RT_USING_UART4
/* UART4 device driver structure */
#define UART4	FM3_MFS4_UART
struct uart_int_rx uart4_int_rx;
struct uart_int_tx uart4_int_tx;
struct uart_device uart4 =
{
	(FM3_MFS03_UART_TypeDef*)UART4,
	MFS4RX_IRQn,
	MFS4TX_IRQn,
	&uart4_int_rx,
	&uart4_int_tx,
};
struct rt_device uart4_device;

void MFS4RX_IRQHandler(void)
{
	/* enter interrupt */
	rt_interrupt_enter();
	rt_hw_uart4_isr(&uart4_device);
	/* leave interrupt */
	rt_interrupt_leave();
}
#endif /**< #ifdef RT_USING_UART4 */

#ifdef RT_USING_UART5
/* UART5 device driver structure */
#define UART5	FM3_MFS5_UART
struct uart_int_rx uart5_int_rx;
struct uart_int_tx uart5_int_tx;
struct uart_device uart5 =
{
	(FM3_MFS03_UART_TypeDef*)UART5,
	MFS5RX_IRQn,
	MFS5TX_IRQn,
	&uart5_int_rx,
	&uart5_int_tx,

};
struct rt_device uart5_device;

void MFS5RX_IRQHandler(void)
{
	/* enter interrupt */
	rt_interrupt_enter();
	rt_hw_uart5_isr(&uart5_device);
	/* leave interrupt */
	rt_interrupt_leave();
}
#endif /**< #ifdef RT_USING_UART5 */



#ifdef RT_USING_UART6
/* UART6 device driver structure */
#define UART6	FM3_MFS6_UART
struct uart_int_rx uart6_int_rx;
struct uart_int_tx uart6_int_tx;
struct uart_device uart6 =
{
	(FM3_MFS03_UART_TypeDef*)UART6,
	MFS6RX_IRQn,
	MFS6TX_IRQn,
	&uart6_int_rx,
	&uart6_int_tx,

};
struct rt_device uart6_device;

void MFS6RX_IRQHandler(void)
{
	/* enter interrupt */
	rt_interrupt_enter();
	rt_hw_uart6_isr(&uart6_device);
	/* leave interrupt */
	rt_interrupt_leave();
}
#endif /**< #ifdef RT_USING_UART6 */





#ifdef RT_USING_UART7
/* UART7 device driver structure */
#define UART7	FM3_MFS7_UART
struct uart_int_rx uart7_int_rx;
struct uart_int_tx uart7_int_tx;
struct uart_device uart7 =
{
	(FM3_MFS03_UART_TypeDef*)UART7,
	MFS7RX_IRQn,
	MFS7TX_IRQn,
	&uart7_int_rx,
	&uart7_int_tx,
};
struct rt_device uart7_device;

void MFS7RX_IRQHandler(void)
{
	/* enter interrupt */
	rt_interrupt_enter();
	rt_hw_uart7_isr(&uart7_device);
	/* leave interrupt */
	rt_interrupt_leave();
}
#endif /**< #ifdef RT_USING_UART7 */

/**************************************************************************
目的/功能：串口初始化及注册
全局变量：
输入参数：
输出参数：
调用关系：
**************************************************************************/
void rt_hw_uart0_init(void)
{
	uint32_t APB2_clock = (SystemCoreClock >> (APBC2_PSR_Val & 0x03));
	uint32_t baudrate;

	/* initialize UART0 */
  //FM3_GPIO->PCR2 = FM3_GPIO->PCR2 | (1<<1) | (1<<2);  
	FM3_GPIO->PCR2 = FM3_GPIO->PCR2 | (1 << 1) | (1 << 2);
	FM3_GPIO->PFR2 |= (1 << 0x01) | (1 << 0x02);   // P21->SIN0_0, P22->SOT0_0  
	FM3_GPIO->EPFR07 |= (1 << 6);
	uart0.uart_device->SCR = 0x80;   // UART Reset_CU
	/*baudrate_0 = 1;//test
	switch(uartpara[0].uart_baudrate)
	{
		case 0:
			baudrate = 4800;
			break;
		case 2:
			baudrate = 19200;
			break;
		case 3:
			baudrate = 38400;
			break;
		default:
			baudrate = 9600;
			break;
	}*/
	baudrate = uartpara[0].uart_baudrate;
	uart0.uart_device->SMR = SMR_MD_UART | SMR_SOE;
	uart0.uart_device->BGR = (APB2_clock + (baudrate / 2)) / baudrate - 1;//36000000UL
	uart0.uart_device->ESCR = ESCR_DATABITS_8;// | ESCR_PEN;
	if (uartpara[0].uart_parity > 0)
	{
		uart0.uart_device->ESCR |= ESCR_PEN;
		if (uartpara[0].uart_parity == 2)
			uart0.uart_device->ESCR |= ESCR_P; //奇校验
	}
	uart0.uart_device->SSR = 0x80;   // 0x80 = clear receive error flag        
	uart0.uart_device->SCR = SCR_RXE | SCR_TXE | SCR_RIE;//|= 0x03;   // RX, TX enable	
	  /* register UART0 device */
	rt_hw_uart_register(&uart0_device,
						"uart0",
						RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX
						| RT_DEVICE_FLAG_STREAM | RT_DEVICE_FLAG_DMA_TX,
						&uart0);

	rt_uart_init(&uart0_device);

}

void rt_hw_uart1_init(void)
{
	uint32_t APB2_clock = (SystemCoreClock >> (APBC2_PSR_Val & 0x03));
	uint32_t baudrate;

	//initialize UART1 
	 //Set Uart Ch1 Port, SIN1_0:P56, SOT1_0:P57 
	FM3_GPIO->PCR5 = FM3_GPIO->PCR5 | (1 << 6) | (1 << 7);
	FM3_GPIO->PFR5 = FM3_GPIO->PFR5 | (1 << 6) | (1 << 7);
	FM3_GPIO->EPFR07 = FM3_GPIO->EPFR07 | (1 << 10) | (1 << 12);

	uart1.uart_device->SCR = 0x80;   // UART reset
	/*baudrate_1 = 1;//test
	switch(uartpara[1].uart_baudrate)
	{
		case 0:
			baudrate = 4800;
			break;
		case 2:
			baudrate = 19200;
			break;
		case 3:
			baudrate = 38400;
			break;
		default:
			baudrate = 9600;
			break;
	}*/
	baudrate = uartpara[1].uart_baudrate;
	uart1.uart_device->SMR = SMR_MD_UART | SMR_SOE;
	uart1.uart_device->BGR = (APB2_clock + (baudrate / 2)) / baudrate - 1;//36000000UL
	uart1.uart_device->ESCR = ESCR_DATABITS_8;// | ESCR_PEN;
	if (uartpara[1].uart_parity > 0)
	{
		uart1.uart_device->ESCR |= ESCR_PEN;
		if (uartpara[1].uart_parity == 2)
			uart1.uart_device->ESCR |= ESCR_P; //奇校验
	}
	uart1.uart_device->SSR = 0x80;   // 0x80 = clear receive error flag        
	uart1.uart_device->SCR = SCR_RXE | SCR_TXE | SCR_RIE;//|= 0x03;   // RX, TX enable	

	/* register UART1 device */
	rt_hw_uart_register(&uart1_device,
						"uart1",
						RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX
						| RT_DEVICE_FLAG_STREAM | RT_DEVICE_FLAG_DMA_TX,
						&uart1);
	rt_uart_init(&uart1_device);

}

void rt_hw_uart3_init(void)
{
	uint32_t APB2_clock = (SystemCoreClock >> (APBC2_PSR_Val & 0x03));
	uint32_t baudrate;

	//initialize UART3 
	 //Set Uart Ch3 Port, SIN3_2:P48, SOT3_2:P49 
	FM3_GPIO->PCR4 = FM3_GPIO->PCR4 | (1 << 8) | (1 << 9);
	FM3_GPIO->PFR4 = FM3_GPIO->PFR4 | (1 << 8) | (1 << 9);
	FM3_GPIO->EPFR07 = FM3_GPIO->EPFR07 | (3 << 22) | (3 << 24);

	uart3.uart_device->SCR = 0x80;   // UART reset
	/*baudrate_3 = 1;//test
	switch(uartpara[3].uart_baudrate)
	{
		case 0:
			baudrate = 4800;
			break;
		case 2:
			baudrate = 19200;
			break;
		case 3:
			baudrate = 38400;
			break;
		default:
			baudrate = 9600;
			break;
	}*/
	baudrate = uartpara[3].uart_baudrate;
	uart3.uart_device->SMR = SMR_MD_UART | SMR_SOE;
	uart3.uart_device->BGR = (APB2_clock + (baudrate / 2)) / baudrate - 1;//36000000UL
	uart3.uart_device->ESCR = ESCR_DATABITS_8;// | ESCR_PEN;
	if (uartpara[3].uart_parity > 0)
	{
		uart3.uart_device->ESCR |= ESCR_PEN;
		if (uartpara[3].uart_parity == 2)
			uart3.uart_device->ESCR |= ESCR_P; //奇校验
	}
	uart3.uart_device->SSR = 0x80;   // 0x80 = clear receive error flag        
	uart3.uart_device->SCR = SCR_RXE | SCR_TXE | SCR_RIE;//|= 0x03;   // RX, TX enable	

	/* register UART1 device */
	rt_hw_uart_register(&uart3_device,
						"uart3",
						RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX
						| RT_DEVICE_FLAG_STREAM | RT_DEVICE_FLAG_DMA_TX,
						&uart3);
	rt_uart_init(&uart3_device);


}
void rt_hw_uart4_init(void)
{
	uint32_t APB2_clock = (SystemCoreClock >> (APBC2_PSR_Val & 0x03));
	uint32_t baudrate;

	// initialize UART4 //
	FM3_GPIO->PCR0 = FM3_GPIO->PCR0 | (1 << 8) | (1 << 9);
	FM3_GPIO->PFR0 = FM3_GPIO->PFR0 | (1 << 5) | (1 << 6);
	FM3_GPIO->EPFR08 = FM3_GPIO->EPFR08 | (3 << 4) | (3 << 6);

	uart4.uart_device->SCR = 0x80;   // UART reset
	/*baudrate_4 = 1;//test
	switch(uartpara[4].uart_baudrate)
	{
		case 0:
			baudrate = 4800;
			break;
		case 2:
			baudrate = 19200;
			break;
		case 3:
			baudrate = 38400;
			break;
		default:
			baudrate = 9600;
			break;
	}*/
	baudrate = uartpara[4].uart_baudrate;
	uart4.uart_device->SMR = SMR_MD_UART | SMR_SOE;
	uart4.uart_device->BGR = (APB2_clock + (baudrate / 2)) / baudrate - 1;//36000000UL
	uart4.uart_device->ESCR = ESCR_DATABITS_8;// | ESCR_PEN;
	if (uartpara[4].uart_parity > 0)
	{
		uart4.uart_device->ESCR |= ESCR_PEN;
		if (uartpara[4].uart_parity == 2)
			uart4.uart_device->ESCR |= ESCR_P; //奇校验
	}
	uart4.uart_device->SSR = 0x80;   // 0x80 = clear receive error flag        
	uart4.uart_device->SCR = SCR_RXE | SCR_TXE | SCR_RIE;//|= 0x03;   // RX, TX enable	

	rt_hw_uart_register(&uart4_device,
						"uart4",
						RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX
						| RT_DEVICE_FLAG_STREAM | RT_DEVICE_FLAG_DMA_TX,
						&uart4);
	rt_uart_init(&uart4_device);

}


void rt_hw_uart5_init(void)
{
	uint32_t APB2_clock = (SystemCoreClock >> (APBC2_PSR_Val & 0x03));
	uint32_t baudrate;

	/* initialize UART5 */
	/* Set Uart Ch5 Port, SIN5_2:P36, SOT5_2:P37 SCK5_2:P38*/
	FM3_GPIO->PCR3 = FM3_GPIO->PCR3 | (1 << 6) | (1 << 7);
	FM3_GPIO->PFR3 = FM3_GPIO->PFR3 | (1 << 6) | (1 << 7);
	FM3_GPIO->EPFR08 = FM3_GPIO->EPFR08 | (3 << 10) | (3 << 12);

	uart5.uart_device->SCR = 0x80;   // UART reset
	/*baudrate_5 = 1;//test
	switch(uartpara[5].uart_baudrate)
	{
		case 0:
			baudrate = 4800;
			break;
		case 2:
			baudrate = 19200;
			break;
		case 3:
			baudrate = 38400;
			break;
		default:
			baudrate = 9600;
			break;
	}*/
	baudrate = uartpara[5].uart_baudrate;
	uart5.uart_device->SMR = SMR_MD_UART | SMR_SOE;
	uart5.uart_device->BGR = (APB2_clock + (baudrate / 2)) / baudrate - 1;//36000000UL
	uart5.uart_device->ESCR = ESCR_DATABITS_8;// | ESCR_PEN;
	if (uartpara[5].uart_parity > 0)
	{
		uart5.uart_device->ESCR |= ESCR_PEN;
		if (uartpara[5].uart_parity == 2)
			uart5.uart_device->ESCR |= ESCR_P; //奇校验
	}
	uart5.uart_device->SSR = 0x80;   // 0x80 = clear receive error flag        
	uart5.uart_device->SCR = SCR_RXE | SCR_TXE | SCR_RIE;//|= 0x03;   // RX, TX enable	


	/* register UART5 device */
	rt_hw_uart_register(&uart5_device,
						"uart5",
						RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX
						| RT_DEVICE_FLAG_STREAM | RT_DEVICE_FLAG_DMA_TX,
						&uart5);
	rt_uart_init(&uart5_device);

}

void rt_hw_uart6_init(void)
{
	uint32_t APB2_clock = (SystemCoreClock >> (APBC2_PSR_Val & 0x03));
	uint32_t baudrate;

	// initialize UART6 //
	// Set Uart Ch6 Port, SIN6_0:P53, SOT6_0:P54 //
	FM3_GPIO->PCR5 = FM3_GPIO->PCR5 | (1 << 3) | (1 << 4);
	FM3_GPIO->PFR5 = FM3_GPIO->PFR5 | (1 << 3) | (1 << 4);
	FM3_GPIO->EPFR08 = FM3_GPIO->EPFR08 | (1 << 16) | (1 << 18);

	uart6.uart_device->SCR = 0x80;   // UART reset
	/*baudrate_6 = 1;//test
	switch(uartpara[6].uart_baudrate)
	{
		case 0:
			baudrate = 4800;
			break;
		case 2:
			baudrate = 19200;
			break;
		case 3:
			baudrate = 38400;
			break;
		default:
			baudrate = 9600;
			break;
	}*/
	baudrate = uartpara[6].uart_baudrate;
	uart6.uart_device->SMR = SMR_MD_UART | SMR_SOE;
	uart6.uart_device->BGR = (APB2_clock + (baudrate / 2)) / baudrate - 1;//36000000UL
	uart6.uart_device->ESCR = ESCR_DATABITS_8;// | ESCR_PEN;
	if (uartpara[6].uart_parity > 0)
	{
		uart6.uart_device->ESCR |= ESCR_PEN;
		if (uartpara[6].uart_parity == 2)
			uart6.uart_device->ESCR |= ESCR_P; //奇校验
	}
	uart6.uart_device->SSR = 0x80;   // 0x80 = clear receive error flag        
	uart6.uart_device->SCR = SCR_RXE | SCR_TXE | SCR_RIE;//|= 0x03;   // RX, TX enable	


	/* register UART6 device */
	rt_hw_uart_register(&uart6_device,
						"uart6",
						RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX
						| RT_DEVICE_FLAG_STREAM | RT_DEVICE_FLAG_DMA_TX,
						&uart6);
	rt_uart_init(&uart6_device);


}

void rt_hw_uart7_init(void)
{
	uint32_t APB2_clock = (SystemCoreClock >> (APBC2_PSR_Val & 0x03));
	uint32_t baudrate;

	/* initialize UART7 */
	/* Set Uart Ch7 Port, SIN7_0:P59, SOT7_0:P5A SCK7_0:P5B*/
	FM3_GPIO->PCR5 = FM3_GPIO->PCR5 | (1 << 9) | (1 << 10);
	FM3_GPIO->PFR5 = FM3_GPIO->PFR5 | (1 << 9) | (1 << 10);
	FM3_GPIO->EPFR08 = FM3_GPIO->EPFR08 | (1 << 22) | (1 << 24);

	uart7.uart_device->SCR = 0x80;   // UART reset
	/*baudrate_7 = 1;//test
	switch(uartpara[7].uart_baudrate)
	{
		case 0:
			baudrate = 4800;
			break;
		case 2:
			baudrate = 19200;
			break;
		case 3:
			baudrate = 38400;
			break;
		default:
			baudrate = 9600;
			break;
	}*/
	baudrate = uartpara[7].uart_baudrate;
	uart7.uart_device->SMR = SMR_MD_UART | SMR_SOE;
	uart7.uart_device->BGR = (APB2_clock + (baudrate / 2)) / baudrate - 1;//36000000UL
	uart7.uart_device->ESCR = ESCR_DATABITS_8;// | ESCR_PEN;
	if (uartpara[7].uart_parity > 0)
	{
		uart7.uart_device->ESCR |= ESCR_PEN;
		if (uartpara[7].uart_parity == 2)
			uart7.uart_device->ESCR |= ESCR_P; //奇校验
	}
	uart7.uart_device->SSR = 0x80;   // 0x80 = clear receive error flag        
	uart7.uart_device->SCR = SCR_RXE | SCR_TXE | SCR_RIE;//|= 0x03;   // RX, TX enable	


	/* register UART7 device */
	rt_hw_uart_register(&uart7_device,
						"uart7",
						RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX
						| RT_DEVICE_FLAG_STREAM | RT_DEVICE_FLAG_DMA_TX,
						&uart7);
	rt_uart_init(&uart7_device);




}





uint32_t uart0_tx_rjsz, uart1_tx_rjsz, uart3_tx_rjsz, uart4_tx_rjsz, uart5_tx_rjsz, uart6_tx_rjsz, uart7_tx_rjsz;
uint8_t uart0txflg;
#define UARTRXTIMEOUT     300

//串口0线程
//ALIGN(RT_ALIGN_SIZE)

extern void run_init(void);
ALIGN(RT_ALIGN_SIZE)
static uint8_t uart0_stack[1024];
static struct rt_thread uart0_thread;
void uart0_thread_entry(void* parameter)
{

	rt_err_t result;
	rt_device_t dev = RT_NULL;
	//    rt_uint32_t level;
		//struct uart_device* uart;
	//    uint32_t src;
	//    uint32_t des;  
	rt_hw_uart0_init(); //return;
	dev = rt_device_find("uart0");
	rt_device_open(dev, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_DMA_TX);
	//uart = (struct uart_device*)dev->user_data;

	rt_thread_delay(RT_TICK_PER_SECOND * 2);


	while (1)
	{

		//rt_enter_critical();//调度器上锁
		result = rt_sem_take(&uart0_sem, (RT_TICK_PER_SECOND / 20));//      
		if (uartpara[0].uart_usedflg == 1)
		{
			if (result == -RT_ETIMEOUT)
			{
				uart0trascount++;
				uart0errcount++;

				if (uart0errcount > 200)
				{
					//run_init();

					serial_led0_off();
					rt_hw_uart0_init();
					uart0errcount = 0;
				}
				else
				{

					if ((rjsz - uart_status[0].uart_trans_time) >= uartpara[0].uart_read_time)
					{
						switch (uartpara[0].uart_protocol)
						{
						case 0:
							Deal_Comm_Proce_0(dev);
							break;
						case 1:
							Deal_Comm_Proce_1(dev);
							break;
						case 2:
							Deal_Comm_Proce_2(dev);
							break;
						case 3:
							Deal_Comm_Proce_3(dev);
							break;
						case 4:
							Deal_Comm_Proce_4(dev);
							break;
						default:
							break;
						}


						uart0trascount = 0;
						uart_status[0].uart_trans_time = rjsz;

					}

				}

			}
			else
			{
				uart0errcount = 0;

				switch (uartpara[0].uart_protocol)
				{
				case 0:
					Receive_Commdata_Proce_0(dev);//接收数据处理
					break;
				case 1:
					Receive_Commdata_Proce_1(dev);//接收数据处理
					break;
				case 2:
					Receive_Commdata_Proce_2(dev);//接收数据处理
					break;
				case 3:
					Receive_Commdata_Proce_3(dev);//接收数据处理
					break;
				case 4:
					Receive_Commdata_Proce_4(dev);//接收数据处理
					break;
				default:
					break;
				}

				serial_led0_on();
			}
			//rt_thread_delay(100);    

		}   	//rt_exit_critical();//调度器解锁

	}


}



//串口1线程
ALIGN(RT_ALIGN_SIZE)
static uint8_t uart1_stack[1024];
static struct rt_thread uart1_thread;
void uart1_thread_entry(void* parameter)
{
	rt_err_t result;
	rt_device_t dev = RT_NULL;
	//    struct uart_device* uart;
	//    uint32_t src;
	//    uint32_t des;

	rt_hw_uart1_init();
	dev = rt_device_find("uart1");
	rt_device_open(dev, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_DMA_TX);
	//    uart = (struct uart_device*)dev->user_data;


	rt_thread_delay(RT_TICK_PER_SECOND * 2);

	while (1)
	{


		result = rt_sem_take(&uart1_sem, (RT_TICK_PER_SECOND / 20));//
		if (uartpara[1].uart_usedflg == 1)
		{
			if (result == -RT_ETIMEOUT)
			{
				uart1trascount++;
				uart1errcount++;
				if (uart1errcount > 600)
				{
					serial_led1_off();
					rt_hw_uart1_init();
					uart1errcount = 0;
				}
				else
				{
					if ((rjsz - uart_status[1].uart_trans_time) >= uartpara[1].uart_read_time)
					{

						switch (uartpara[1].uart_protocol)
						{
						case 0:
							Deal_Comm_Proce_0(dev);
							break;
						case 1:
							Deal_Comm_Proce_1(dev);
							break;
						case 2:
							Deal_Comm_Proce_2(dev);
							break;
						case 3:
							Deal_Comm_Proce_3(dev);
							break;
						case 4:
							Deal_Comm_Proce_4(dev);
							break;
						default:
							break;
						}
						uart1trascount = 0;
						uart_status[1].uart_trans_time = rjsz;

					}

				}

			}
			else
			{
				uart1errcount = 0;
				switch (uartpara[1].uart_protocol)
				{
				case 0:
					Receive_Commdata_Proce_0(dev);//接收数据处理
					break;
				case 1:
					Receive_Commdata_Proce_1(dev);//接收数据处理
					break;
				case 2:
					Receive_Commdata_Proce_2(dev);//接收数据处理
					break;
				case 3:
					Receive_Commdata_Proce_3(dev);//接收数据处理
					break;
				case 4:
					Receive_Commdata_Proce_4(dev);//接收数据处理
					break;
				default:
					break;
				}
				serial_led1_on();

			}

		}
	}
}



//串口3线程
ALIGN(RT_ALIGN_SIZE)
static uint8_t uart3_stack[1024];
static struct rt_thread uart3_thread;
void uart3_thread_entry(void* parameter)
{

	rt_err_t result;
	rt_device_t dev = RT_NULL;
	//    struct uart_device* uart;
	//    uint32_t src;
	//    uint32_t des;
	rt_hw_uart3_init();
	dev = rt_device_find("uart3");
	rt_device_open(dev, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_DMA_TX);
	//    uart = (struct uart_device*)dev->user_data;


	rt_thread_delay(RT_TICK_PER_SECOND * 2);

	while (1)
	{

		//rt_enter_critical();//调度器上锁
		result = rt_sem_take(&uart3_sem, (RT_TICK_PER_SECOND / 20));//
		if (uartpara[3].uart_usedflg == 1)
		{
			if (result == -RT_ETIMEOUT)
			{
				uart3trascount++;
				uart3errcount++;
				if (uart3errcount > 600)
				{
					serial_led3_off();
					rt_hw_uart3_init();
					uart3errcount = 0;
				}
				else
				{

					if ((rjsz - uart_status[3].uart_trans_time) >= uartpara[3].uart_read_time)
					{

						switch (uartpara[3].uart_protocol)
						{
						case 0:
							Deal_Comm_Proce_0(dev);
							break;
						case 1:
							Deal_Comm_Proce_1(dev);
							break;
						case 2:
							Deal_Comm_Proce_2(dev);
							break;
						case 3:
							Deal_Comm_Proce_3(dev);
							break;
						case 4:
							Deal_Comm_Proce_4(dev);
							break;
						default:
							break;
						}
						uart3trascount = 0;
						uart_status[3].uart_trans_time = rjsz;

					}
					//level = rt_hw_interrupt_disable();

					  //rt_hw_interrupt_enable(level);



				}

			}
			else
			{
				uart3errcount = 0;

				switch (uartpara[3].uart_protocol)
				{
				case 0:
					Receive_Commdata_Proce_0(dev);//接收数据处理
					break;
				case 1:
					Receive_Commdata_Proce_1(dev);//接收数据处理
					break;
				case 2:
					Receive_Commdata_Proce_2(dev);//接收数据处理
					break;
				case 3:
					Receive_Commdata_Proce_3(dev);//接收数据处理
					break;
				case 4:
					Receive_Commdata_Proce_4(dev);//接收数据处理
					break;
				default:
					break;
				}
				serial_led3_on();

			}

		}
	}//rt_exit_critical();//调度器解锁
}



//串口4线程
ALIGN(RT_ALIGN_SIZE)
static uint8_t uart4_stack[1024];
static struct rt_thread uart4_thread;
void uart4_thread_entry(void* parameter)
{
	rt_err_t result;
	rt_device_t dev = RT_NULL;
	//    struct uart_device* uart;
	//    uint32_t src;
	//    uint32_t des;

	rt_hw_uart4_init();
	dev = rt_device_find("uart4");
	rt_device_open(dev, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_DMA_TX);

	rt_thread_delay(RT_TICK_PER_SECOND * 2);

	while (1)
	{


		result = rt_sem_take(&uart4_sem, (RT_TICK_PER_SECOND / 20));//
		if (uartpara[4].uart_usedflg == 1)
		{
			if (result == -RT_ETIMEOUT)
			{
				uart4trascount++;
				uart4errcount++;
				if (uart4errcount > 600)
				{
					serial_led4_off();
					rt_hw_uart4_init();
					uart4errcount = 0;
				}
				else
				{
					if ((rjsz - uart_status[4].uart_trans_time) >= uartpara[4].uart_read_time)
					{

						switch (uartpara[4].uart_protocol)
						{
						case 0:
							Deal_Comm_Proce_0(dev);
							break;
						case 1:
							Deal_Comm_Proce_1(dev);
							break;
						case 2:
							Deal_Comm_Proce_2(dev);
							break;
						case 3:
							Deal_Comm_Proce_3(dev);
							break;
						case 4:
							Deal_Comm_Proce_4(dev);
							break;
						default:
							break;
						}
						uart4trascount = 0;
						uart_status[4].uart_trans_time = rjsz;

					}
					//level = rt_hw_interrupt_disable();

					  //rt_hw_interrupt_enable(level);



				}

			}
			else
			{
				uart4errcount = 0;
				switch (uartpara[4].uart_protocol)
				{
				case 0:
					Receive_Commdata_Proce_0(dev);//接收数据处理
					break;
				case 1:
					Receive_Commdata_Proce_1(dev);//接收数据处理
					break;
				case 2:
					Receive_Commdata_Proce_2(dev);//接收数据处理
					break;
				case 3:
					Receive_Commdata_Proce_3(dev);//接收数据处理
					break;
				case 4:
					Receive_Commdata_Proce_4(dev);//接收数据处理
					break;
				default:
					break;
				}
				serial_led4_on();

			}

		}
	}
}



//串口5线程
ALIGN(RT_ALIGN_SIZE)
static uint8_t uart5_stack[1024];
static struct rt_thread uart5_thread;
void uart5_thread_entry(void* parameter)
{
	rt_err_t result;
	rt_device_t dev = RT_NULL;
	//    struct uart_device* uart;
	//    uint32_t src;
	//    uint32_t des;

	rt_hw_uart5_init();
	dev = rt_device_find("uart5");
	rt_device_open(dev, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_DMA_TX);

	rt_thread_delay(RT_TICK_PER_SECOND * 2);

	while (1)
	{

		result = rt_sem_take(&uart5_sem, (RT_TICK_PER_SECOND / 20));//
		if (uartpara[5].uart_usedflg == 1)
		{
			if (result == -RT_ETIMEOUT)
			{
				uart5trascount++;
				uart5errcount++;
				if (uart5errcount > 600)
				{
					serial_led5_off();
					rt_hw_uart5_init();
					uart5errcount = 0;

				}
				else
				{

					if ((rjsz - uart_status[5].uart_trans_time) >= uartpara[5].uart_read_time)
					{

						switch (uartpara[5].uart_protocol)
						{
						case 0:
							Deal_Comm_Proce_0(dev);
							break;
						case 1:
							Deal_Comm_Proce_1(dev);
							break;
						case 2:
							Deal_Comm_Proce_2(dev);
							break;
						case 3:
							Deal_Comm_Proce_3(dev);
							break;
						case 4:
							Deal_Comm_Proce_4(dev);
							break;
						default:
							break;
						}
						uart5trascount = 0;
						uart_status[5].uart_trans_time = rjsz;

					}
					//if(uart5trascount>3)
					//{

					//}               
					//level = rt_hw_interrupt_disable();

					//rt_hw_interrupt_enable(level);

				}

			}
			else
			{
				uart5errcount = 0;
				switch (uartpara[5].uart_protocol)
				{
				case 0:
					Receive_Commdata_Proce_0(dev);//接收数据处理
					break;
				case 1:
					Receive_Commdata_Proce_1(dev);//接收数据处理
					break;
				case 2:
					Receive_Commdata_Proce_2(dev);//接收数据处理
					break;
				case 3:
					Receive_Commdata_Proce_3(dev);//接收数据处理
					break;
				case 4:
					Receive_Commdata_Proce_4(dev);//接收数据处理
					break;
				default:
					break;
				}
				serial_led5_on();
			}
		}
	}
}


//串口6线程
ALIGN(RT_ALIGN_SIZE)
static uint8_t uart6_stack[1024];
static struct rt_thread uart6_thread;
void uart6_thread_entry(void* parameter)
{
	rt_err_t result;
	rt_device_t dev = RT_NULL;
	//    struct uart_device* uart;
	//    uint32_t src;
	//    uint32_t des;
	rt_hw_uart6_init();
	dev = rt_device_find("uart6");
	rt_device_open(dev, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_DMA_TX);

	rt_thread_delay(RT_TICK_PER_SECOND * 2);

	while (1)
	{

		result = rt_sem_take(&uart6_sem, (RT_TICK_PER_SECOND / 20));//
		if (uartpara[6].uart_usedflg == 1)
		{
			if (result == -RT_ETIMEOUT)
			{
				uart6trascount++;
				uart6errcount++;
				if (uart6errcount > 600)
				{
					serial_led6_off();
					rt_hw_uart6_init();
					uart6errcount = 0;

				}
				else
				{
					if ((rjsz - uart_status[6].uart_trans_time) >= uartpara[6].uart_read_time)
					{

						switch (uartpara[6].uart_protocol)
						{
						case 0:
							Deal_Comm_Proce_0(dev);
							break;
						case 1:
							Deal_Comm_Proce_1(dev);
							break;
						case 2:
							Deal_Comm_Proce_2(dev);
							break;
						case 3:
							Deal_Comm_Proce_3(dev);
							break;
						case 4:
							Deal_Comm_Proce_4(dev);
							break;
						default:
							break;
						}
						uart6trascount = 0;
						uart_status[6].uart_trans_time = rjsz;

					}

				}

			}
			else
			{
				uart6errcount = 0;
				switch (uartpara[6].uart_protocol)
				{
				case 0:
					Receive_Commdata_Proce_0(dev);//接收数据处理
					break;
				case 1:
					Receive_Commdata_Proce_1(dev);//接收数据处理
					break;
				case 2:
					Receive_Commdata_Proce_2(dev);//接收数据处理
					break;
				case 3:
					Receive_Commdata_Proce_3(dev);//接收数据处理
					break;
				case 4:
					Receive_Commdata_Proce_4(dev);//接收数据处理
					break;
				default:
					break;
				}
				serial_led6_on();

			}

		}
	}
}

//uint8_t uart7errcount;
ALIGN(RT_ALIGN_SIZE)
static uint8_t uart7_stack[1024];
static struct rt_thread uart7_thread;
void uart7_thread_entry(void* parameter)
{
	rt_err_t result;
	rt_device_t dev = RT_NULL;
	//    struct uart_device* uart;
	//    uint32_t src;
	//    uint32_t des;
	rt_hw_uart7_init();
	dev = rt_device_find("uart7");
	rt_device_open(dev, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_DMA_TX);
	rt_thread_delay(RT_TICK_PER_SECOND * 2);

	while (1)
	{

		result = rt_sem_take(&uart7_sem, (RT_TICK_PER_SECOND / 20));//
		if (uartpara[7].uart_usedflg == 1)
		{
			if (result == -RT_ETIMEOUT)
			{
				uart7trascount++;
				uart7errcount++;
				if (uart7errcount > 600)
				{
					serial_led7_off();
					rt_hw_uart7_init();
					uart7errcount = 0;
				}
				else
				{
					if ((rjsz - uart_status[7].uart_trans_time) >= uartpara[7].uart_read_time)
					{

						switch (uartpara[7].uart_protocol)
						{
						case 0:
							Deal_Comm_Proce_0(dev);
							break;
						case 1:
							Deal_Comm_Proce_1(dev);
							break;
						case 2:
							Deal_Comm_Proce_2(dev);
							break;
						case 3:
							Deal_Comm_Proce_3(dev);
							break;
						case 4:
							Deal_Comm_Proce_4(dev);
							break;
						default:
							break;
						}
						uart7trascount = 0;
						uart_status[7].uart_trans_time = rjsz;

					}
					//level = rt_hw_interrupt_disable();

					//rt_hw_interrupt_enable(level);

				}

			}
			else
			{
				uart7errcount = 0;


				switch (uartpara[7].uart_protocol)
				{
				case 0:
					Receive_Commdata_Proce_0(dev);//接收数据处理
					break;
				case 1:
					Receive_Commdata_Proce_1(dev);//接收数据处理
					break;
				case 2:
					Receive_Commdata_Proce_2(dev);//接收数据处理
					break;
				case 3:
					Receive_Commdata_Proce_3(dev);//接收数据处理
					break;
				case 4:
					Receive_Commdata_Proce_4(dev);//接收数据处理
					break;
				default:
					break;
				}



				serial_led7_on();

			}
		}

	}
}


//extern void RS485_trans_process();
uint8_t uartporttxindex = 0, rtdbcmdflg;
static void uart_tx__thread_entry(void* parameter)
{



	while (1)
	{

		serial_led0_flash();

		uartporttxindex++;
		rt_thread_delay(RT_TICK_PER_SECOND / 2); /* sleep 0.5 second*/
	}
}

void communication_task(void)
{
	rt_err_t result;



	/* 初始化静态信号量，初始值是0 */

	rt_sem_init(&uart0_sem, "uart0_sem", 0, RT_IPC_FLAG_FIFO);
	rt_sem_init(&uart1_sem, "uart1_sem", 0, RT_IPC_FLAG_FIFO);
	rt_sem_init(&uart3_sem, "uart3_sem", 0, RT_IPC_FLAG_FIFO);
	rt_sem_init(&uart4_sem, "uart4_sem", 0, RT_IPC_FLAG_FIFO);
	rt_sem_init(&uart5_sem, "uart5_sem", 0, RT_IPC_FLAG_FIFO);
	rt_sem_init(&uart6_sem, "uart6_sem", 0, RT_IPC_FLAG_FIFO);
	rt_sem_init(&uart7_sem, "uart7_sem", 0, RT_IPC_FLAG_FIFO);
	//rt_thread_delay(1);

	  //run_init();	
	 /* init "uart0" thread */
	result = rt_thread_init(&uart0_thread,
		"uart0",
		uart0_thread_entry, RT_NULL,
		(uint8_t*)& uart0_stack[0], sizeof(uart0_stack), 10, RT_TICK_PER_SECOND / 2);
	if (result == RT_EOK)
	{
		rt_thread_startup(&uart0_thread);
	}


	result = rt_thread_init(&uart1_thread,
		"uart1",
		uart1_thread_entry, RT_NULL,
		(uint8_t*)& uart1_stack[0], sizeof(uart1_stack), 10, RT_TICK_PER_SECOND / 2);
	if (result == RT_EOK)
	{
		rt_thread_startup(&uart1_thread);
	}



	result = rt_thread_init(&uart3_thread,
		"uart3",
		uart3_thread_entry, RT_NULL,
		(uint8_t*)& uart3_stack[0], sizeof(uart3_stack), 10, RT_TICK_PER_SECOND / 2);
	if (result == RT_EOK)
	{
		rt_thread_startup(&uart3_thread);
	}


	result = rt_thread_init(&uart4_thread,
		"uart4",
		uart4_thread_entry, RT_NULL,
		(uint8_t*)& uart4_stack[0], sizeof(uart4_stack), 10, RT_TICK_PER_SECOND / 2);
	if (result == RT_EOK)
	{
		rt_thread_startup(&uart4_thread);
	}


	result = rt_thread_init(&uart5_thread,
		"uart5",
		uart5_thread_entry, RT_NULL,
		(uint8_t*)& uart5_stack[0], sizeof(uart5_stack), 10, RT_TICK_PER_SECOND / 2);
	if (result == RT_EOK)
	{
		rt_thread_startup(&uart5_thread);
	}


	result = rt_thread_init(&uart6_thread,
		"uart6",
		uart6_thread_entry, RT_NULL,
		(uint8_t*)& uart6_stack[0], sizeof(uart6_stack), 10, RT_TICK_PER_SECOND / 2);
	if (result == RT_EOK)
	{
		rt_thread_startup(&uart6_thread);
	}


	result = rt_thread_init(&uart7_thread,
		"uart7",
		uart7_thread_entry, RT_NULL,
		(uint8_t*)& uart7_stack[0], sizeof(uart7_stack), 10, RT_TICK_PER_SECOND / 2);
	if (result == RT_EOK)
	{
		rt_thread_startup(&uart7_thread);
	}


	/* 创建串口发送线程 */
	/*tid = rt_thread_create("uart_tx",uart_tx__thread_entry, RT_NULL,
			512*2, 23, 5);
	if (tid != RT_NULL)	rt_thread_startup(tid);*/
}


/*@}*/
