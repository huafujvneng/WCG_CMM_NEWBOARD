/**************************************************************************
Copyright (C), 2012, XJ ELECTRIC Co., LTD.
文件名  ：  dmac_fm3.c
作者    ：  wanghusen
项目名称：   
功能    ：  串口发送使用DMA
创建日期：  2017年5月13日
备注    ：  DMA0用于UART0_TX  ...   DMA7用于UART7_TX

**************************************************************************/

#include "dmac_fm3.h"
//#include "serial_init.h"
/*!
 ******************************************************************************
 ** \brief specify DMA irq
 ******************************************************************************
 */
IRQn_Type irqn;
/*!
 ******************************************************************************
 ** \brief flags for indicating DMA0 busy on normal data transfer by software trigger mode
 ******************************************************************************
 */
uint8_t NormSoftTrigFlag0=0;
/*!
 ******************************************************************************
 ** \brief flags for indicating DMA1 busy on normal data transfer by software trigger mode
 ******************************************************************************
 */
uint8_t NormSoftTrigFlag1=0;
/*!
 ******************************************************************************
 ** \brief flags for indicating DMA2 busy on normal data transfer by software trigger mode
 ******************************************************************************
 */
uint8_t NormSoftTrigFlag2=0;
/*!
 ******************************************************************************
 ** \brief flags for indicating DMA3 busy on normal data transfer by software trigger mode
 ******************************************************************************
 */
uint8_t NormSoftTrigFlag3=0;
/*!
 ******************************************************************************
 ** \brief flags for indicating DMA4 busy on normal data transfer by software trigger mode
 ******************************************************************************
 */
uint8_t NormSoftTrigFlag4=0;
/*!
 ******************************************************************************
 ** \brief flags for indicating DMA5 busy on normal data transfer by software trigger mode
 ******************************************************************************
 */
uint8_t NormSoftTrigFlag5=0;
/*!
 ******************************************************************************
 ** \brief flags for indicating DMA6 busy on normal data transfer by software trigger mode
 ******************************************************************************
 */
uint8_t NormSoftTrigFlag6=0;
/*!
 ******************************************************************************
 ** \brief flags for indicating DMA7 busy on normal data transfer by software trigger mode
 ******************************************************************************
 */
uint8_t NormSoftTrigFlag7=0;

/*---------------------------------------------------------------------------*/
/* global data                                                               */
/*---------------------------------------------------------------------------*/

/*!
 ******************************************************************************
 ** \brief DMA callback function array
 ******************************************************************************
 */
DMA_Callback DMA_IrqCallback[] =
{
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
};
/*---------------------------------------------------------------------------*/
/* global functions                                                          */
/*---------------------------------------------------------------------------*/
void rt_hw_dma_init(void)
{
        FM3_INTREQ->DRQSEL |= 0x0AA8A000;//UART0 1 3 4、6、7、5发送中断到DMA
        FM3_DMAC->DMACR = 0x80000000;//0x80000000;


        NVIC_SetPriority(DMAC0_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 1, 1));
        NVIC_EnableIRQ(DMAC0_IRQn);

        NVIC_SetPriority(DMAC1_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 1, 1));
        NVIC_EnableIRQ(DMAC1_IRQn);

        //NVIC_SetPriority(DMAC2_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 1, 1));
        //NVIC_EnableIRQ(DMAC2_IRQn);


        NVIC_SetPriority(DMAC3_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 1, 1));
        NVIC_EnableIRQ(DMAC3_IRQn);
        
        
        NVIC_SetPriority(DMAC4_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 1, 1));
        NVIC_EnableIRQ(DMAC4_IRQn);        
        
        
        NVIC_SetPriority(DMAC5_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 1, 1));
        NVIC_EnableIRQ(DMAC5_IRQn);         
        
        NVIC_SetPriority(DMAC6_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 1, 1));
        NVIC_EnableIRQ(DMAC6_IRQn);         
        
        NVIC_SetPriority(DMAC7_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 1, 1));
        NVIC_EnableIRQ(DMAC7_IRQn);         
        
        
}

/*!
 ******************************************************************************
 ** \brief check DMA channel0 busy status on normal software trigger mode
 **
 ** \param none
 **
 ** \retval 1 busy
 ** \retval 0 not busy
 **
 ******************************************************************************
 */
uint8_t DMA_IsNormSoftTrigBusy_Ch0(void)
{
    if( NormSoftTrigFlag0 == 0 )
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

/*!
 ******************************************************************************
 ** \brief check DMA channel1 busy status on normal software trigger mode
 **
 ** \param none
 **
 ** \retval 1 busy
 ** \retval 0 not busy
 **
 ******************************************************************************
 */
uint8_t DMA_IsNormSoftTrigBusy_Ch1(void)
{
    if( NormSoftTrigFlag1 == 0 )
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

/*!
 ******************************************************************************
 ** \brief check DMA channel2 busy status on normal software trigger mode
 **
 ** \param none
 **
 ** \retval 1 busy
 ** \retval 0 not busy
 **
 ******************************************************************************
 */
uint8_t DMA_IsNormSoftTrigBusy_Ch2(void)
{
    if( NormSoftTrigFlag2 == 0 )
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

/*!
 ******************************************************************************
 ** \brief check DMA channel3 busy status on normal software trigger mode
 **
 ** \param none
 **
 ** \retval 1 busy
 ** \retval 0 not busy
 **
 ******************************************************************************
 */
uint8_t DMA_IsNormSoftTrigBusy_Ch3(void)
{
    if( NormSoftTrigFlag3 == 0 )
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

/*!
 ******************************************************************************
 ** \brief check DMA channel4 busy status on normal software trigger mode
 **
 ** \param none
 **
 ** \retval 1 busy
 ** \retval 0 not busy
 **
 ******************************************************************************
 */
uint8_t DMA_IsNormSoftTrigBusy_Ch4(void)
{
    if( NormSoftTrigFlag4 == 0 )
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

/*!
 ******************************************************************************
 ** \brief check DMA channel5 busy status on normal software trigger mode
 **
 ** \param none
 **
 ** \retval 1 busy
 ** \retval 0 not busy
 **
 ******************************************************************************
 */
uint8_t DMA_IsNormSoftTrigBusy_Ch5(void)
{
    if( NormSoftTrigFlag5 == 0 )
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

/*!
 ******************************************************************************
 ** \brief check DMA channel6 busy status on normal software trigger mode
 **
 ** \param none
 **
 ** \retval 1 busy
 ** \retval 0 not busy
 **
 ******************************************************************************
 */
uint8_t DMA_IsNormSoftTrigBusy_Ch6(void)
{
    if( NormSoftTrigFlag6 == 0 )
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

/*!
 ******************************************************************************
 ** \brief check DMA channel7 busy status on normal software trigger mode
 **
 ** \param none
 **
 ** \retval 1 busy
 ** \retval 0 not busy
 **
 ******************************************************************************
 */
uint8_t DMA_IsNormSoftTrigBusy_Ch7(void)
{
    if( NormSoftTrigFlag7 == 0 )
    {
        return 0;
    }
    else
    {
        return 1;
    }
}


/*!
 ******************************************************************************
 ** \brief DMA0 handler function  用于UART0_TX
 **
 ** \param none
 **
 ** \retval none
 **
 ******************************************************************************
 */
void DMAC0_Handler(void)
{
    DMAIntType tpye;
    rt_interrupt_enter();
    if( (FM3_DMAC->DMACB0 & 0x00070000) == 0x00010000 )
    {
        tpye=ErrAddOverflow;
    }
    else if( (FM3_DMAC->DMACB0 & 0x00070000) == 0x00020000 )
    {
        tpye=StopRqst;
    }
    else if( (FM3_DMAC->DMACB0 & 0x00070000) == 0x00030000 )
    {
        tpye=ErrSrsAddErr;
    }
    else if( (FM3_DMAC->DMACB0 & 0x00070000) == 0x00040000 )
    {
        tpye=ErrDesAddErr;
    }
    else if( (FM3_DMAC->DMACB0 & 0x00070000) == 0x00050000 )
    {
        tpye=Success;
    }
    else if( (FM3_DMAC->DMACB0 & 0x00070000) == 0x00070000 )
    {
        tpye=Pause;
    }

    if( DMA_IrqCallback[0]!=NULL )
    {
        DMA_IrqCallback[0](tpye);
    }

    FM3_DMAC->DMACB0 &= 0xFFF8FFFF;//clear SS bit
    NormSoftTrigFlag0 = 0;
    bFM3_MFS0_UART_SCR_TBIE = 0;//SCR_TBIE
//    DMA_Disable(DMA_ch_2);
//    UART_CH5_RXEN;
//    rt_sem_release(&uart5_sem);

//    count_uart5 ++;
   // bFM3_MFS5_UART_SCR_TBIE = 0;//SCR_TIE
//    rt_kprintf(" DMA0TX_END! \n");
    rt_interrupt_leave();

}

/*!
 ******************************************************************************
 ** \brief DMA1 handler function   用于UART1_TX
 **
 ** \param none
 **
 ** \retval none
 **
 ******************************************************************************
 */
void DMAC1_Handler(void)
{
    DMAIntType tpye;
    rt_interrupt_enter();

    if( (FM3_DMAC->DMACB1 & 0x00070000) == 0x00010000 )
    {
        tpye=ErrAddOverflow;
    }
    else if( (FM3_DMAC->DMACB1 & 0x00070000) == 0x00020000 )
    {
        tpye=StopRqst;
    }
    else if( (FM3_DMAC->DMACB1 & 0x00070000) == 0x00030000 )
    {
        tpye=ErrSrsAddErr;
    }
    else if( (FM3_DMAC->DMACB1 & 0x00070000) == 0x00040000 )
    {
        tpye=ErrDesAddErr;
    }
    else if( (FM3_DMAC->DMACB1 & 0x00070000) == 0x00050000 )
    {
        tpye=Success;
    }
    else if( (FM3_DMAC->DMACB1 & 0x00070000) == 0x00070000 )
    {
        tpye=Pause;
    }

    if( DMA_IrqCallback[1]!=NULL )
    {
        DMA_IrqCallback[1](tpye);
    }

    FM3_DMAC->DMACB1 &= 0xFFF8FFFF;
    NormSoftTrigFlag1=0;


    bFM3_MFS1_UART_SCR_TBIE = 0;//SCR_TBIE
//    DMA_Disable(DMA_ch_1);
//    UART_CH7_RXEN;
//    rt_sem_release(&uart7_sem);

//    rt_kprintf(" DMA1TX_END! \n");
    rt_interrupt_leave();
}

/*!
 ******************************************************************************
 ** \brief DMA2 handler function
 **
 ** \param none
 **
 ** \retval none
 **
 ******************************************************************************
 */
/*void DMAC2_Handler(void)
{
    DMAIntType tpye;

    rt_interrupt_enter();

    if( (FM3_DMAC->DMACB2 & 0x00070000) == 0x00010000 )
    {
        tpye=ErrAddOverflow;
    }
    else if( (FM3_DMAC->DMACB2 & 0x00070000) == 0x00020000 )
    {
        tpye=StopRqst;
    }
    else if( (FM3_DMAC->DMACB2 & 0x00070000) == 0x00030000 )
    {
        tpye=ErrSrsAddErr;
    }
    else if( (FM3_DMAC->DMACB2 & 0x00070000) == 0x00040000 )
    {
        tpye=ErrDesAddErr;
    }
    else if( (FM3_DMAC->DMACB2 & 0x00070000) == 0x00050000 )
    {
        tpye=Success;
    }
    else if( (FM3_DMAC->DMACB2 & 0x00070000) == 0x00070000 )
    {
        tpye=Pause;
    }

    if( DMA_IrqCallback[2]!=NULL )
    {
        DMA_IrqCallback[2](tpye);
    }

    FM3_DMAC->DMACB2 &= 0xFFF8FFFF;
    NormSoftTrigFlag2=0;

    bFM3_MFS4_UART_SCR_TBIE = 0;//SCR_TBIE

    rt_interrupt_leave();

}*/
void write_cmd_sp(uint8_t u8Data);
/*!
 ******************************************************************************
 ** \brief DMA3 handler function
 **
 ** \param none
 **
 ** \retval none

 */
void DMAC3_Handler(void)
{
    DMAIntType tpye;
    rt_interrupt_enter();

    if( (FM3_DMAC->DMACB3 & 0x00070000) == 0x00010000 )
    {
        tpye=ErrAddOverflow;
    }
    else if( (FM3_DMAC->DMACB3 & 0x00070000) == 0x00020000 )
    {
        tpye=StopRqst;
    }
    else if( (FM3_DMAC->DMACB3 & 0x00070000) == 0x00030000 )
    {
        tpye=ErrSrsAddErr;
    }
    else if( (FM3_DMAC->DMACB3 & 0x00070000) == 0x00040000 )
    {
        tpye=ErrDesAddErr;
    }
    else if( (FM3_DMAC->DMACB3 & 0x00070000) == 0x00050000 )
    {
        tpye=Success;
    }
    else if( (FM3_DMAC->DMACB3 & 0x00070000) == 0x00070000 )
    {
        tpye=Pause;
    }

    if( DMA_IrqCallback[3]!=NULL )
    {
        DMA_IrqCallback[3](tpye);
    }

    FM3_DMAC->DMACB3 &= 0xFFF8FFFF;
    NormSoftTrigFlag3=0;


    bFM3_MFS3_UART_SCR_TBIE = 0;//SCR_TBIE


//    rt_kprintf(" DMA3TX_END! \n");
    rt_interrupt_leave();


}

/*!
 ******************************************************************************
 ** \brief DMA4 handler function
 **
 ** \param none
 **
 ** \retval none
 **
 ******************************************************************************
 */
void DMAC4_Handler(void)
{
    DMAIntType tpye;
    rt_interrupt_enter();

    if( (FM3_DMAC->DMACB4 & 0x00070000) == 0x00010000 )
    {
        tpye=ErrAddOverflow;
    }
    else if( (FM3_DMAC->DMACB4 & 0x00070000) == 0x00020000 )
    {
        tpye=StopRqst;
    }
    else if( (FM3_DMAC->DMACB4 & 0x00070000) == 0x00030000 )
    {
        tpye=ErrSrsAddErr;
    }
    else if( (FM3_DMAC->DMACB4 & 0x00070000) == 0x00040000 )
    {
        tpye=ErrDesAddErr;
    }
    else if( (FM3_DMAC->DMACB4 & 0x00070000) == 0x00050000 )
    {
        tpye=Success;
    }
    else if( (FM3_DMAC->DMACB4 & 0x00070000) == 0x00070000 )
    {
        tpye=Pause;
    }

    if( DMA_IrqCallback[4]!=NULL )
    {
        DMA_IrqCallback[4](tpye);
    }

    FM3_DMAC->DMACB4 &= 0xFFF8FFFF;
    NormSoftTrigFlag4=0;


    bFM3_MFS4_UART_SCR_TBIE = 0;//SCR_TBIE


//    rt_kprintf(" DMA4TX_END! \n");
    rt_interrupt_leave();

}

/*!
 ******************************************************************************
 ** \brief DMA5 handler function
 **
 ** \param none
 **
 ** \retval none
 **
 ******************************************************************************
 */
void DMAC5_Handler(void)
{
    DMAIntType tpye;
    rt_interrupt_enter();

    if( (FM3_DMAC->DMACB5 & 0x00070000) == 0x00010000 )
    {
        tpye=ErrAddOverflow;
    }
    else if( (FM3_DMAC->DMACB5 & 0x00070000) == 0x00020000 )
    {
        tpye=StopRqst;
    }
    else if( (FM3_DMAC->DMACB5 & 0x00070000) == 0x00030000 )
    {
        tpye=ErrSrsAddErr;
    }
    else if( (FM3_DMAC->DMACB5 & 0x00070000) == 0x00040000 )
    {
        tpye=ErrDesAddErr;
    }
    else if( (FM3_DMAC->DMACB5 & 0x00070000) == 0x00050000 )
    {
        tpye=Success;
    }
    else if( (FM3_DMAC->DMACB5 & 0x00070000) == 0x00070000 )
    {
        tpye=Pause;
    }

    if( DMA_IrqCallback[5]!=NULL )
    {
        DMA_IrqCallback[5](tpye);
    }

    FM3_DMAC->DMACB5 &= 0xFFF8FFFF;
    NormSoftTrigFlag5=0;
    bFM3_MFS5_UART_SCR_TBIE = 0;//SCR_TBIE


//    rt_kprintf(" DMA5TX_END! \n");
    rt_interrupt_leave();

}

/*!
 ******************************************************************************
 ** \brief DMA6 handler function
 **
 ** \param none
 **
 ** \retval none
 **
 ******************************************************************************
 */
void DMAC6_Handler(void)
{

    DMAIntType tpye;
    rt_interrupt_enter();

    if( (FM3_DMAC->DMACB6 & 0x00070000) == 0x00010000 )
    {
        tpye=ErrAddOverflow;
    }
    else if( (FM3_DMAC->DMACB6 & 0x00070000) == 0x00020000 )
    {
        tpye=StopRqst;
    }
    else if( (FM3_DMAC->DMACB6 & 0x00070000) == 0x00030000 )
    {
        tpye=ErrSrsAddErr;
    }
    else if( (FM3_DMAC->DMACB6 & 0x00070000) == 0x00040000 )
    {
        tpye=ErrDesAddErr;
    }
    else if( (FM3_DMAC->DMACB6 & 0x00070000) == 0x00050000 )
    {
        tpye=Success;
    }
    else if( (FM3_DMAC->DMACB6 & 0x00070000) == 0x00070000 )
    {
        tpye=Pause;
    }

    if( DMA_IrqCallback[6]!=NULL )
    {
        DMA_IrqCallback[6](tpye);
    }

    FM3_DMAC->DMACB6 &= 0xFFF8FFFF;
    NormSoftTrigFlag6=0;
    bFM3_MFS6_UART_SCR_TBIE = 0;//SCR_TBIE


//    rt_kprintf(" DMA6TX_END! \n");
    rt_interrupt_leave();

}

/*!
 ******************************************************************************
 ** \brief DMA7 handler function
 **
 ** \param none
 **
 ** \retval none
 **
 ******************************************************************************
 */
//int test=0;

void DMAC7_Handler(void)
{
    DMAIntType tpye;
    rt_interrupt_enter();

    if( (FM3_DMAC->DMACB7 & 0x00070000) == 0x00010000 )
    {
        tpye=ErrAddOverflow;
    }
    else if( (FM3_DMAC->DMACB7 & 0x00070000) == 0x00020000 )
    {
        tpye=StopRqst;
    }
    else if( (FM3_DMAC->DMACB7 & 0x00070000) == 0x00030000 )
    {
        tpye=ErrSrsAddErr;
    }
    else if( (FM3_DMAC->DMACB7 & 0x00070000) == 0x00040000 )
    {
        tpye=ErrDesAddErr;
    }
    else if( (FM3_DMAC->DMACB7 & 0x00070000) == 0x00050000 )
    {
        tpye=Success;
    }
    else if( (FM3_DMAC->DMACB7 & 0x00070000) == 0x00070000 )
    {
        tpye=Pause;
    }

    if( DMA_IrqCallback[7]!=NULL )
    {
        DMA_IrqCallback[7](tpye);
    }

    FM3_DMAC->DMACB7 &= 0xFFF8FFFF;
    NormSoftTrigFlag7=0;
    bFM3_MFS7_UART_SCR_TBIE = 0;//SCR_TBIE


//    rt_kprintf(" DMA7TX_END! \n");
    rt_interrupt_leave();

}

/*!
 ******************************************************************************
 ** \brief DMA configuration setting for MFS
 **
 ** \param MFS_trig MFS trigger type.
 **        This parameter can be one of the following values:
 ** \arg   ch0_Rx: MFS ch0 Rx
 ** \arg   ch0_Tx: MFS ch0 Tx
 ** \arg   ch1_Rx: MFS ch0 Rx
 ** \arg   ch1_Tx: MFS ch1 Tx
 ** \arg   ch2_Rx: MFS ch0 Rx
 ** \arg   ch2_Tx: MFS ch2 Tx
 ** \arg   ch3_Rx: MFS ch0 Rx
 ** \arg   ch3_Tx: MFS ch3 Tx
 ** \arg   ch4_Rx: MFS ch0 Rx
 ** \arg   ch4_Tx: MFS ch4 Tx
 ** \arg   ch5_Rx: MFS ch0 Rx
 ** \arg   ch5_Tx: MFS ch5 Tx
 ** \arg   ch6_Rx: MFS ch0 Rx
 ** \arg   ch6_Tx: MFS ch6 Tx
 ** \arg   ch7_Rx: MFS ch0 Rx
 ** \arg   ch7_Tx: MFS ch7 Tx

 ** \param DMA_ch DMA channel.
 **        This parameter can be one of the following values:
 ** \arg   0: DMA channel 0
 ** \arg   1: DMA channel 1
 ** \arg   2: DMA channel 2
 ** \arg   3: DMA channel 3
 ** \arg   4: DMA channel 4
 ** \arg   5: DMA channel 5
 ** \arg   6: DMA channel 6
 ** \arg   7: DMA channel 7

 ** \param DMA_ReptMode DMA repeat mode.
 **        This parameter can be one of the following values:
 ** \arg   0: Single mode. Only catchs the first interrupt request.
 ** \arg   1: Repeat mode. Catchs every interrupt request. But source address and target address and byte counts will be reloaded.

 ** \param src Source address.
 **        This parameter can be one of the following values:
 ** \arg   Physical adress

 ** \param des Destination address.
 **        This parameter can be one of the following values:
 ** \arg   Physical adress

 ** \param bytes Byte counts.
 **        This parameter can be one of the following values:
 ** \arg   0-65535: count in byte. Transfer counts=bytes+1. How many data need to be transferred.

 ** \retval None
 **
 ******************************************************************************
 */
void DMA_MFS(DMA_MFSTrig MFS_trig,
             uint8_t DMA_ch,
             uint8_t DMA_ReptMode,
             uint32_t src,
             uint32_t des,
             uint16_t bytes)
{    
    //uart_dma_status = 1;

/*    if(MFS_trig == ch0_Rx)
    {
        FM3_INTREQ->DRQSEL = 0x00001000;
        FM3_DMAC->DMACR = 0x80000000;//0x80000000;
        *(&(FM3_DMAC->DMACA0)+DMA_ch*0x04) = (0x96000000 | bytes);
        *(&(FM3_DMAC->DMACB0)+DMA_ch*0x04) = 0x22180000;
        *(&(FM3_DMAC->DMACSA0)+DMA_ch*0x04) = (uint32_t) (&(FM3_MFS0_UART->RDR));
        *(&(FM3_DMAC->DMACDA0)+DMA_ch*0x04) = des;
    }*/
    if(MFS_trig == ch0_Tx)
    {
       // FM3_INTREQ->DRQSEL = (0x00001000<<1);
        //FM3_DMAC->DMACR = 0x80000000;//0x80000000;
        //NormSoftTrigFlag0 = 1;
        *(&(FM3_DMAC->DMACA0)+DMA_ch*0x04) = ((0x96000000+(1<<23)) | bytes);
        *(&(FM3_DMAC->DMACB0)+DMA_ch*0x04) = 0x21180000;
        *(&(FM3_DMAC->DMACSA0)+DMA_ch*0x04) = src;
        *(&(FM3_DMAC->DMACDA0)+DMA_ch*0x04) = (uint32_t) (&(FM3_MFS0_UART->TDR));
    }
    /*if(MFS_trig == ch1_Rx)
    {
        FM3_INTREQ->DRQSEL = (0x00001000<<2);
        FM3_DMAC->DMACR = 0x80000000;
        *(&(FM3_DMAC->DMACA0)+DMA_ch*0x04) = ((0x96000000+(2<<23)) | bytes);
        *(&(FM3_DMAC->DMACB0)+DMA_ch*0x04) = 0x22180000;
        *(&(FM3_DMAC->DMACSA0)+DMA_ch*0x04) = (uint32_t) (&(FM3_MFS1_UART->RDR));
        *(&(FM3_DMAC->DMACDA0)+DMA_ch*0x04) = des;
    }*/
    if(MFS_trig == ch1_Tx)
    {
			
       // FM3_INTREQ->DRQSEL = (0x00001000<<3);
        //FM3_DMAC->DMACR = 0x80000000;
        //NormSoftTrigFlag1 = 1;
        *(&(FM3_DMAC->DMACA0)+DMA_ch*0x04) = ((0x96000000+(3<<23)) | bytes);
        *(&(FM3_DMAC->DMACB0)+DMA_ch*0x04) = 0x21180000;
        *(&(FM3_DMAC->DMACSA0)+DMA_ch*0x04) = src;
        *(&(FM3_DMAC->DMACDA0)+DMA_ch*0x04) = (uint32_t) (&(FM3_MFS1_UART->TDR));
    }
    /*if(MFS_trig == ch2_Rx)
    {
        FM3_INTREQ->DRQSEL = (0x00001000<<4);
        FM3_DMAC->DMACR = 0x80000000;
        *(&(FM3_DMAC->DMACA0)+DMA_ch*0x04) = ((0x96000000+(4<<23)) | bytes);
        *(&(FM3_DMAC->DMACB0)+DMA_ch*0x04) = 0x22180000;
        *(&(FM3_DMAC->DMACSA0)+DMA_ch*0x04) = (uint32_t) (&(FM3_MFS2_UART->RDR));
        *(&(FM3_DMAC->DMACDA0)+DMA_ch*0x04) = des;
    }
    if(MFS_trig == ch2_Tx)
    {
        FM3_INTREQ->DRQSEL = (0x00001000<<5);
        FM3_DMAC->DMACR = 0x80000000;
        *(&(FM3_DMAC->DMACA0)+DMA_ch*0x04) = ((0x96000000+(5<<23)) | bytes);
        *(&(FM3_DMAC->DMACB0)+DMA_ch*0x04) = 0x21180000;
        *(&(FM3_DMAC->DMACSA0)+DMA_ch*0x04) = src;
        *(&(FM3_DMAC->DMACDA0)+DMA_ch*0x04) = (uint32_t) (&(FM3_MFS2_UART->TDR));
    }
    if(MFS_trig == ch3_Rx)
    {
        FM3_INTREQ->DRQSEL = (0x00001000<<6);
        FM3_DMAC->DMACR = 0x80000000;
        *(&(FM3_DMAC->DMACA0)+DMA_ch*0x04) = ((0x96000000+(6<<23)) | bytes);
        *(&(FM3_DMAC->DMACB0)+DMA_ch*0x04) = 0x22180000;
        *(&(FM3_DMAC->DMACSA0)+DMA_ch*0x04) = (uint32_t) (&(FM3_MFS3_UART->RDR));
        *(&(FM3_DMAC->DMACDA0)+DMA_ch*0x04) = des;
    }*/
    if(MFS_trig == ch3_Tx)
    {
       // FM3_INTREQ->DRQSEL = (0x00001000<<7);
        //FM3_DMAC->DMACR = 0x80000000;
        //NormSoftTrigFlag3 = 1;
        *(&(FM3_DMAC->DMACA0)+DMA_ch*0x04) = ((0x96000000+(7<<23)) | bytes);
        *(&(FM3_DMAC->DMACB0)+DMA_ch*0x04) = 0x21180000;
        *(&(FM3_DMAC->DMACSA0)+DMA_ch*0x04) = src;
        *(&(FM3_DMAC->DMACDA0)+DMA_ch*0x04) = (uint32_t) (&(FM3_MFS3_UART->TDR));
    }
    /*if(MFS_trig == ch4_Rx)
    {
        FM3_INTREQ->DRQSEL = (0x00001000<<8);
        FM3_DMAC->DMACR = 0x80000000;
        *(&(FM3_DMAC->DMACA0)+DMA_ch*0x04) = ((0x96000000+(8<<23)) | bytes);
        *(&(FM3_DMAC->DMACB0)+DMA_ch*0x04) = 0x22180000;
        *(&(FM3_DMAC->DMACSA0)+DMA_ch*0x04) = (uint32_t) (&(FM3_MFS4_UART->RDR));
        *(&(FM3_DMAC->DMACDA0)+DMA_ch*0x04) = des;
    }
*/    if(MFS_trig == ch4_Tx)
    {
       // FM3_INTREQ->DRQSEL |= (0x00001000<<9);
       // FM3_DMAC->DMACR = 0x80000000;//0x80000000;

        //NormSoftTrigFlag4 = 1;
        *(&(FM3_DMAC->DMACA0)+DMA_ch*0x04) = ((0x96000000+(9<<23)) | bytes);
        *(&(FM3_DMAC->DMACB0)+DMA_ch*0x04) = 0x21180000;
        *(&(FM3_DMAC->DMACSA0)+DMA_ch*0x04) = src;
        *(&(FM3_DMAC->DMACDA0)+DMA_ch*0x04) = (uint32_t) (&(FM3_MFS4_UART->TDR));
    }
/*    if(MFS_trig == ch5_Rx)
    {
        FM3_INTREQ->DRQSEL = (0x00001000<<10);
        FM3_DMAC->DMACR = 0x80000000;
        *(&(FM3_DMAC->DMACA0)+DMA_ch*0x04) = ((0x96000000+(10<<23)) | bytes);
        *(&(FM3_DMAC->DMACB0)+DMA_ch*0x04) = 0x22180000;
        *(&(FM3_DMAC->DMACSA0)+DMA_ch*0x04) = (uint32_t) (&(FM3_MFS5_UART->RDR));
        *(&(FM3_DMAC->DMACDA0)+DMA_ch*0x04) = des;
    }*/ 
		if(MFS_trig == ch5_Tx)
		{
						//FM3_INTREQ->DRQSEL |= (0x00001000<<11);
						//FM3_DMAC->DMACR = 0x80000000;//0x80000000;
						//NormSoftTrigFlag5 = 1;
						*(&(FM3_DMAC->DMACA0)+DMA_ch*0x04) = ((0x96000000+(11<<23)) | bytes);
						*(&(FM3_DMAC->DMACB0)+DMA_ch*0x04) = 0x21180000;//0x21180000
						*(&(FM3_DMAC->DMACSA0)+DMA_ch*0x04) = src;
						*(&(FM3_DMAC->DMACDA0)+DMA_ch*0x04) = (uint32_t) (&(FM3_MFS5_UART->TDR));
		}		
		

  /*  if(MFS_trig == ch6_Rx)
    {
        FM3_INTREQ->DRQSEL = (0x00001000<<12);
        FM3_DMAC->DMACR = 0x80000000;
        *(&(FM3_DMAC->DMACA0)+DMA_ch*0x04) = ((0x96000000+(12<<23)) | bytes);
        *(&(FM3_DMAC->DMACB0)+DMA_ch*0x04) = 0x22180000;
        *(&(FM3_DMAC->DMACSA0)+DMA_ch*0x04) = (uint32_t) (&(FM3_MFS6_UART->RDR));
        *(&(FM3_DMAC->DMACDA0)+DMA_ch*0x04) = des;
    }
*/ 
    if(MFS_trig == ch6_Tx)
    {
        //FM3_INTREQ->DRQSEL = (0x00001000<<13);
        //FM3_DMAC->DMACR = 0x80000000;
        //NormSoftTrigFlag6 = 1;
        *(&(FM3_DMAC->DMACA0)+DMA_ch*0x04) = ((0x96000000+(13<<23)) | bytes);
        *(&(FM3_DMAC->DMACB0)+DMA_ch*0x04) = 0x21180000;
        *(&(FM3_DMAC->DMACSA0)+DMA_ch*0x04) = src;
        *(&(FM3_DMAC->DMACDA0)+DMA_ch*0x04) = (uint32_t) (&(FM3_MFS6_UART->TDR));
    }		
/*    if(MFS_trig == ch7_Rx)
    {
        FM3_INTREQ->DRQSEL = (0x00001000<<14);
        FM3_DMAC->DMACR = 0x80000000;
        *(&(FM3_DMAC->DMACA0)+DMA_ch*0x04) = ((0x96000000+(14<<23)) | bytes);
        *(&(FM3_DMAC->DMACB0)+DMA_ch*0x04) = 0x22180000;
        *(&(FM3_DMAC->DMACSA0)+DMA_ch*0x04) = (uint32_t) (&(FM3_MFS7_UART->RDR));
        *(&(FM3_DMAC->DMACDA0)+DMA_ch*0x04) = des;
    }
*/    if(MFS_trig == ch7_Tx)
    {
        //FM3_INTREQ->DRQSEL |= (0x00001000<<15);
        //FM3_DMAC->DMACR = 0x80000000;//0x80000000;
        //NormSoftTrigFlag7 = 1;
        *(&(FM3_DMAC->DMACA0)+DMA_ch*0x04) = ((0x96000000+(15<<23)) | bytes);
        *(&(FM3_DMAC->DMACB0)+DMA_ch*0x04) = 0x21180000;//0x21180000;
        *(&(FM3_DMAC->DMACSA0)+DMA_ch*0x04) = src;
        *(&(FM3_DMAC->DMACDA0)+DMA_ch*0x04) = (uint32_t) (&(FM3_MFS7_UART->TDR));
    }

    if(DMA_ReptMode==1)
    {
        *(&(FM3_DMAC->DMACB0)+DMA_ch*0x04) |= 0x00000001;//set EM
        *(&(FM3_DMAC->DMACB0)+DMA_ch*0x04) |= 0x00E00000;//set RC,RD,RS
        *(&(FM3_DMAC->DMACB0)+DMA_ch*0x04) &= 0xFFF7FFFF;//clear CI
    }


    if(DMA_ch == 0)
    {
        irqn = DMAC0_IRQn;
        NVIC_SetPriority(irqn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 1, 1));
        NVIC_EnableIRQ(irqn);
//        NVIC_SetPriority(irqn, 0);
    }

    if(DMA_ch == 1)
    {
        irqn = DMAC1_IRQn;
        NVIC_SetPriority(irqn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 1, 1));
        NVIC_EnableIRQ(irqn);
//        NVIC_SetPriority(irqn, 0);
    }

    if(DMA_ch == 2)
    {
        irqn = DMAC2_IRQn;
      //set interrupt priority
        NVIC_SetPriority(irqn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 1, 1));
        NVIC_EnableIRQ(irqn);
//        NVIC_SetPriority(irqn, 0);

    }

    if(DMA_ch == 3)
    {
        irqn = DMAC3_IRQn;
        NVIC_SetPriority(irqn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 1, 1));
        NVIC_EnableIRQ(irqn);
//        NVIC_SetPriority(irqn, 0);
    }

    if(DMA_ch == 4)
    {
        irqn = DMAC4_IRQn;
        NVIC_SetPriority(irqn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 1, 1));
        NVIC_EnableIRQ(irqn);
//        NVIC_SetPriority(irqn, 0);
    }

    if(DMA_ch == 5)
    {
        irqn = DMAC5_IRQn;
        NVIC_SetPriority(irqn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 1, 1));
        NVIC_EnableIRQ(irqn);
//        NVIC_SetPriority(irqn, 0);
    }

    if(DMA_ch == 6)
    {
        irqn = DMAC6_IRQn;
        NVIC_SetPriority(irqn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 1, 1));
        NVIC_EnableIRQ(irqn);
//        NVIC_SetPriority(irqn, 0);
    }

    if(DMA_ch == 7)
    {
        irqn = DMAC7_IRQn;
        NVIC_SetPriority(irqn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 1, 1));
        NVIC_EnableIRQ(irqn);
//        NVIC_SetPriority(irqn, 0);
    }
    uart_dma_status = 0;
}



/*!
 ******************************************************************************
 ** \brief DMA pause.
 **
 ** \param DMA_ch DMA channel.
 **        This parameter can be one of the following values:
 ** \arg   0: DMA channel 0
 ** \arg   1: DMA channel 1
 ** \arg   2: DMA channel 2
 ** \arg   3: DMA channel 3
 ** \arg   4: DMA channel 4
 ** \arg   5: DMA channel 5
 ** \arg   6: DMA channel 6
 ** \arg   7: DMA channel 7

 ** \retval None
 **
 ******************************************************************************
 */
void DMA_Pause(unsigned char DMA_ch)
{
    *(&(FM3_DMAC->DMACA0)+DMA_ch*0x04) |= 0x40000000;
}

/*!
 ******************************************************************************
 ** \brief DMA resume
 **
 ** \param DMA_ch DMA channel.
 **        This parameter can be one of the following values:
 ** \arg   0: DMA channel 0
 ** \arg   1: DMA channel 1
 ** \arg   2: DMA channel 2
 ** \arg   3: DMA channel 3
 ** \arg   4: DMA channel 4
 ** \arg   5: DMA channel 5
 ** \arg   6: DMA channel 6
 ** \arg   7: DMA channel 7

 ** \retval None
 **
 ******************************************************************************
 */
void DMA_Resume(unsigned char DMA_ch)
{
    *(&(FM3_DMAC->DMACA0)+DMA_ch*0x04) &= 0xBFFFFFFF;
}

/*!
 ******************************************************************************
 ** \brief DMA disable.
 **
 ** \param DMA_ch DMA channel.
 **        This parameter can be one of the following values:
 ** \arg   0: DMA channel 0
 ** \arg   1: DMA channel 1
 ** \arg   2: DMA channel 2
 ** \arg   3: DMA channel 3
 ** \arg   4: DMA channel 4
 ** \arg   5: DMA channel 5
 ** \arg   6: DMA channel 6
 ** \arg   7: DMA channel 7

 ** \retval None
 **
 ******************************************************************************
 */
void DMA_Disable(unsigned char DMA_ch)
{
    *(&(FM3_DMAC->DMACA0)+DMA_ch*0x04) &= 0x7FFFFFFF;
}

/*!
 ******************************************************************************
 ** \brief DMA enable.
 **
 ** \param DMA_ch DMA channel.
 **        This parameter can be one of the following values:
 ** \arg   0: DMA channel 0
 ** \arg   1: DMA channel 1
 ** \arg   2: DMA channel 2
 ** \arg   3: DMA channel 3
 ** \arg   4: DMA channel 4
 ** \arg   5: DMA channel 5
 ** \arg   6: DMA channel 6
 ** \arg   7: DMA channel 7

 ** \retval None
 **
 ******************************************************************************
 */
void DMA_Enable(unsigned char DMA_ch)
{
    *(&(FM3_DMAC->DMACA0)+DMA_ch*0x04) |= 0x80000000;
}
