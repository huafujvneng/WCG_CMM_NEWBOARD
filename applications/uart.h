
#ifndef __UART_H__
#define __UART_H__

#include "serial.h"
#include "protect.h"
#include "dmac_fm3.h"
#include "base_define.h"

extern void DMA_MFS(DMA_MFSTrig MFS_trig,
             uint8_t DMA_ch,
             uint8_t DMA_ReptMode,
             uint32_t src,
             uint32_t des,
             uint16_t bytes);


extern uint8_t  Deal_Comm_Proce_0(rt_device_t dev);
extern uint8_t  Deal_Comm_Proce_1(rt_device_t dev);
extern uint8_t  Deal_Comm_Proce_2(rt_device_t dev);
extern uint8_t  Deal_Comm_Proce_3(rt_device_t dev);
extern uint8_t  Deal_Comm_Proce_4(rt_device_t dev);

extern void Receive_Commdata_Proce_0(rt_device_t dev);
extern void Receive_Commdata_Proce_1(rt_device_t dev);
extern void Receive_Commdata_Proce_2(rt_device_t dev);
extern void Receive_Commdata_Proce_3(rt_device_t dev);
extern void Receive_Commdata_Proce_4(rt_device_t dev);

extern void recvmsgto104mod(rt_device_t dev);


extern rt_tick_t Rs485_time;

extern rt_uint8_t zzaddr,zzaddr_0,zzaddr_1,zzaddr_3,zzaddr_4,zzaddr_5,zzaddr_6,zzaddr_7;

extern rt_uint32_t baudrate_0,baudrate_1,baudrate_3,baudrate_4,baudrate_5,baudrate_6,baudrate_7;
extern rt_uint16_t parity_0,parity_1,parity_3,parity_4,parity_5,parity_6,parity_7;
extern rt_uint16_t protocol_0,protocol_1,protocol_3,protocol_4,protocol_5,protocol_6,protocol_7;



extern struct rt_semaphore uart0_sem;
extern struct rt_semaphore uart1_sem;
extern struct rt_semaphore uart3_sem;
extern struct rt_semaphore uart4_sem;
extern struct rt_semaphore uart5_sem;
extern struct rt_semaphore uart6_sem;
extern struct rt_semaphore uart7_sem;



typedef void (*DMA_Callback)(uint8_t Inttype);


#define RS485_RX_BUFFER_SIZE	255
#define RS485_TX_BUFFER_SIZE	255


/*FSEL=0，FIFO1为发送，FIFO2为接收*/
#define FCR_FE1          0x01U
#define FCR_FE2          0x02U
#define FCR_FCL1         0x04U
#define FCR_FCL2         0x08U
#define FCR_FSET         0x10U
#define FCR_FLD          0x20U
#define FCR_FLST         0x40U
#define FCR_FSEL         0x01U
#define FCR_FTIE         0x02U
#define FCR_FDRQ         0x04U
#define FCR_FRIE         0x08U
#define FCR_FLSTE        0x10U
#define FCR_FTST0        0x40U
#define FCR_FTST1        0x80U

#define FCR0_FE1          0x01U
#define FCR0_FE2          0x02U
#define FCR0_FCL1         0x04U
#define FCR0_FCL2         0x08U
#define FCR0_FSET         0x10U
#define FCR0_FLD          0x20U
#define FCR0_FLST         0x40U
#define FCR1_FSEL         0x01U
#define FCR1_FTIE         0x02U
#define FCR1_FDRQ         0x04U
#define FCR1_FRIE         0x08U
#define FCR1_FLSTE        0x10U
#define FCR1_FTST0        0x40U
#define FCR1_FTST1        0x80U


#define FBYTE_FD0        0x01U
#define FBYTE_FD1        0x02U
#define FBYTE_FD2        0x01U
#define FBYTE_FD3        0x08U
#define FBYTE_FD4        0x10U
#define FBYTE_FD5        0x20U
#define FBYTE_FD6        0x40U
#define FBYTE_FD7        0x80U
#define FBYTE_FD8        0x01U
#define FBYTE_FD9        0x02U
#define FBYTE_FD10       0x04U
#define FBYTE_FD11       0x08U
#define FBYTE_FD12       0x10U
#define FBYTE_FD13       0x20U
#define FBYTE_FD14       0x40U
#define FBYTE_FD15       0x80U

#define FBYTE1_FD0        0x01U
#define FBYTE1_FD1        0x02U
#define FBYTE1_FD2        0x01U
#define FBYTE1_FD3        0x08U
#define FBYTE1_FD4        0x10U
#define FBYTE1_FD5        0x20U
#define FBYTE1_FD6        0x40U
#define FBYTE1_FD7        0x80U
#define FBYTE2_FD8        0x01U
#define FBYTE2_FD9        0x02U
#define FBYTE2_FD10       0x04U
#define FBYTE2_FD11       0x08U
#define FBYTE2_FD12       0x10U
#define FBYTE2_FD13       0x20U
#define FBYTE2_FD14       0x40U
#define FBYTE2_FD15       0x80U



#define BAUDRATE_150            150               //波特率
#define BAUDRATE_200            200
#define BAUDRATE_300            300
#define BAUDRATE_600            600
#define BAUDRATE_1200           1200
#define BAUDRATE_1800           1800
#define BAUDRATE_2400           2400
#define BAUDRATE_4800           4800
#define BAUDRATE_9600           9600
#define BAUDRATE_19200          19200
#define BAUDRATE_38400          38400
#define BAUDRATE_57600          57600

#define PARITYBIT_NONE          0                  //none校验
#define PARITYBIT_ODD           1                  //odd校验
#define PARITYBIT_EVEN          2                  //even校验
#define PARITYBIT_SPACE         3                  //space校验
#define PARITYBIT_MARK          4                  //mark校验


#define	STOPBIT_1               1                  //停止位
#define	STOPBIT_2               2

#define	DATABIT_8               8                  //数据位8
#define DATABiT_7               7
#define	DATABIT_6               6
#define DATABiT_5               5


struct uart_int_rx
{
	uint8_t  rx_buffer[RS485_RX_BUFFER_SIZE];
	uint8_t  CommErr_buffer[RS485_RX_BUFFER_SIZE];
    uint8_t SetsBuffer[RS485_RX_BUFFER_SIZE];
	uint8_t read_index, save_index;/**read为接收的首地址，save为接收的结束地址**/
    uint8_t rx_length;
    rt_tick_t   rx_end_tick;
    uint8_t receive_flg;

    uint32_t MsgTransUpFlagJK1;
    uint32_t	MeasureTime;
    uint8_t ImageCtrCodeJK1;       /*image of FCB_ControlCode of a frame*/
    uint32_t  Measure_Data_RDY;//whs20160201
    uint8_t Gen_Insp_IndexJK1;
    uint8_t Gen_Din_IndexJK1;
    uint8_t	iSetNumber;		// 定值个数
    uint8_t  iStartNumber;

};

struct uart_int_tx
{
	uint8_t  tx_buffer[RS485_TX_BUFFER_SIZE];
	uint8_t write_index, save_index;/**write_index为发送的首地址，save_index为发送的结束地址**/
    rt_tick_t   tx_end_tick;
    uint8_t Inspect_NumberJK1;
    uint8_t Identity_NumberJK1;
    uint8_t Common_Addr_JK1;
    uint8_t ASDU20_DCO_CPU_JK1;
    uint8_t ASDU20_Message_JK1;
    uint8_t ASDU61_DCO_CPU_JK1;
    uint8_t ASDU61_Message_JK1;
    uint8_t ASDU62_DCO_CPU_JK1;
    uint8_t ASDU64_DCO_CPU_JK1;
    uint8_t ASDU64_Message_JK1;
    uint8_t ASDU88_DCO_CPU_JK1;
    uint8_t ASDU88_Message_JK1;
    uint8_t pSOE_Protect_out;
    uint8_t pSOE_Alarm_out;
    uint8_t pSOE_Change_out;
    uint8_t pSignal_General;
    uint8_t ASDU_70_flg;

    uint8_t  Disturb_NO_trans;
    uint8_t  DisturbChannel;
    uint8_t  DisturbASDU;
    uint8_t  DisturbState;
    uint16_t  DisturbPointNumber;
    uint8_t  Index;//总召送到哪个了

};



struct uart_device
{
	FM3_MFS03_UART_TypeDef* uart_device;// whs 2017/9/4 12:00:12 串口都按照无FIFO模式设置
	/* irq number */
	IRQn_Type rx_irq, tx_irq;

	/* rx structure */
	struct uart_int_rx* int_rx;
	/* tx structure */
	struct uart_int_tx* int_tx;
};

void communication_task(void);

#endif
