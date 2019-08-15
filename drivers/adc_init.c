/**
 * File    	:  adc_init.c
 * author	:
 * This file is part of WGB-900
 * Copyright (C), 2012, XJ ELECTRIC Co., LTD.
 * Date     :  2012-11-02
 * Function	:  初始化ADC及其触发使用的定时器，开使用的中断

 * Change Logs :
 * Date             Author           Notes
 * 2012-10-03                       the first version
*/

#include <rthw.h>
#include <rtthread.h>

#include "board.h"

#include "adc_task.h"


#define BPS_SPI_7606  20000000
#define CS7606_1   bFM3_GPIO_PDOR1_P5
#define CS7606_2   bFM3_GPIO_PDOR1_PB

#define BUSY7606_1   bFM3_GPIO_PDIR1_P4
#define BUSY7606_2   bFM3_GPIO_PDIR1_PA

#define CONST7606  bFM3_GPIO_PDOR1_PE
#define RST7606    bFM3_GPIO_PDOR1_PC
/**
 * uart1_1初始为spi（模式I）
 *
 */
void spi_7606_init(void)
{
    /* initialize UART1 */
    uint32_t APB2_clock = (SystemCoreClock >> (APBC2_PSR_Val & 0x03));

    // Set Uart Ch1_1 Port, SIN1_1:P11 SOT1_1:P12 SCK1_1:P13 CS1:P15 BUSY1:P14 CONST:P1E RST:P1C
    // Set Uart Ch4_1 Port, SIN2_2:P17 SOT2_2:P18 SCK2_2:P19 CS2:P1B BUSY2:P1A CONST:P1E RST:P1C

    FM3_GPIO->ADE = 0; /* disable all analog input. */
    FM3_GPIO->PCR1 = FM3_GPIO->PCR1 | 0xFFFF; //上拉
    FM3_GPIO->PFR1 = FM3_GPIO->PFR1 | (1<<1) | (1<<2) |(1<<3) | (1<<7) | (1<<8) | (1<<9); //外设口配置
    FM3_GPIO->DDR1 = FM3_GPIO->DDR1 | 0x5B6D; //0输入，1输出配置

    bFM3_GPIO_EPFR07_SIN1S0 = 0;
    bFM3_GPIO_EPFR07_SIN1S1 = 1;
    bFM3_GPIO_EPFR07_SOT1B0 = 0;
    bFM3_GPIO_EPFR07_SOT1B1 = 1;
    bFM3_GPIO_EPFR07_SCK1B0 = 0;
    bFM3_GPIO_EPFR07_SCK1B1 = 1;

    bFM3_GPIO_EPFR07_SIN2S0 = 1;
    bFM3_GPIO_EPFR07_SIN2S1 = 1;
    bFM3_GPIO_EPFR07_SOT2B0 = 1;
    bFM3_GPIO_EPFR07_SOT2B1 = 1;
    bFM3_GPIO_EPFR07_SCK2B0 = 1;
    bFM3_GPIO_EPFR07_SCK2B1 = 1;

    FM3_MFS1_CSIO->SCR = 0XA0;  /*先复位CSIO口*/
    FM3_MFS1_CSIO->SMR = 0X46;  /*正常模式2，不唤醒，时钟空闲高，高位在前，SCKE=1，SOE=0*/
    FM3_MFS1_CSIO->SSR = 0X80;  /*先清接受错误标志*/
    FM3_MFS1_CSIO->ESCR = 0X00;
    FM3_MFS1_CSIO->BGR = (APB2_clock + (BPS_SPI_7606/2))/BPS_SPI_7606 - 1;
    FM3_MFS1_CSIO->SCR |= 0x03;  /*使能收发*/

    FM3_MFS2_CSIO->SCR = 0XA0;  /*先复位CSIO口*/
    FM3_MFS2_CSIO->SMR = 0X46;  /*正常模式2，不唤醒，时钟空闲高，高位在前，SCKE=1，SOE=0*/
    FM3_MFS2_CSIO->SSR = 0X80;  /*先清接受错误标志*/
    FM3_MFS2_CSIO->ESCR = 0X00;
    FM3_MFS2_CSIO->BGR = (APB2_clock + (BPS_SPI_7606/2))/BPS_SPI_7606 - 1;
    FM3_MFS2_CSIO->SCR |= 0x03;  /*使能收发*/

    RST7606 = 0;
    RST7606 = 1;
    RST7606 = 0;
    CS7606_1 = 1;
    CS7606_2 = 1;
}

extern int16_t  SampleData[Sample_Channel][SAMPLE_NUMBER];
extern uint8_t ad_error_num;
/***
*FUNCTION:	read 8*2 bytes from 7606-1
*INPUT:	ch: channel
*OUTPUT:	NO
***/
void ReadDataFrom7606_1(uint16_t ch)
{
	uint8_t i;
	int16_t sdata;
	uint8_t *pdata=(uint8_t*)&sdata;

	while(BUSY7606_1);//convst后40ns变高
	CS7606_1 = 0;
	for(i=0; i<8; i++)
	{
		FM3_MFS1_CSIO->TDR = 0xa5;          // 写任意数据 输出SCLK
		while (!bFM3_MFS1_CSIO_SSR_TBI);    // TBI=0 未传输结束，等待

		*(pdata+1)  = FM3_MFS1_CSIO->RDR;
		if(bFM3_MFS1_CSIO_SSR_ORE)
            bFM3_MFS1_CSIO_SSR_REC= 1; //有溢出错误时，清标志

		FM3_MFS1_CSIO->TDR = 0xa5;          // 写任意数据 输出SCLK
		while (!bFM3_MFS1_CSIO_SSR_TBI);    // TBI=0 未传输结束，等待

		*pdata  = FM3_MFS1_CSIO->RDR;
		if(bFM3_MFS1_CSIO_SSR_ORE)
            bFM3_MFS1_CSIO_SSR_REC= 1; //有溢出错误时，清标志
        SampleData[i][ch]=sdata;
	}
	CS7606_1 = 1;
}
/***
*FUNCTION:	read 8*2 bytes from 7606-2
*INPUT:	ch: channel
*OUTPUT:	NO
***/
void ReadDataFrom7606_2(uint16_t ch)
{
	uint8_t i;
	int16_t sdata;
	uint8_t *pdata=(uint8_t*)&sdata;

	while(BUSY7606_2);//convst后40ns变高
	CS7606_2 = 0;
	for(i=8; i<15; i++)
	{
		FM3_MFS2_CSIO->TDR = 0xa5;          // 写任意数据 输出SCLK
		while (!bFM3_MFS2_CSIO_SSR_TBI);    // TBI=0 未传输结束，等待

		*(pdata+1)  = FM3_MFS2_CSIO->RDR;
		if(bFM3_MFS2_CSIO_SSR_ORE)
            bFM3_MFS2_CSIO_SSR_REC= 1; //有溢出错误时，清标志

		FM3_MFS2_CSIO->TDR = 0xa5;          // 写任意数据 输出SCLK
		while (!bFM3_MFS2_CSIO_SSR_TBI);    // TBI=0 未传输结束，等待

		*pdata  = FM3_MFS2_CSIO->RDR;
		if(bFM3_MFS2_CSIO_SSR_ORE)
            bFM3_MFS2_CSIO_SSR_REC= 1; //有溢出错误时，清标志
        SampleData[i][ch]=sdata;
	}
	//AD故障检测
    FM3_MFS2_CSIO->TDR = 0xa5;          // 写任意数据 输出SCLK
    while (!bFM3_MFS2_CSIO_SSR_TBI);    // TBI=0 未传输结束，等待

    *(pdata+1)  = FM3_MFS2_CSIO->RDR;
    if(bFM3_MFS2_CSIO_SSR_ORE)
        bFM3_MFS2_CSIO_SSR_REC= 1; //有溢出错误时，清标志

    FM3_MFS2_CSIO->TDR = 0xa5;          // 写任意数据 输出SCLK
    while (!bFM3_MFS2_CSIO_SSR_TBI);    // TBI=0 未传输结束，等待

    *pdata  = FM3_MFS2_CSIO->RDR;
    if(bFM3_MFS2_CSIO_SSR_ORE)
        bFM3_MFS2_CSIO_SSR_REC= 1; //有溢出错误时，清标志
    if((sdata>19660)||(sdata<13106))  //2.5V
    {
        if(ad_error_num<255)
            ad_error_num++;
    }
    else if(ad_error_num>0)
        ad_error_num--;
    CS7606_2 = 1;
}



/*********************************************************/
/*****    双定时器初始化（AD采样的中断定时器）   *******/
/*********************************************************/
void Init_Timebase (void)
{
    // Timer 1 used
    FM3_DTIM->TIMER1LOAD = 22500;//0xEA60; // 定时器1载入寄存器,0.833msx(144/2)x1000=60000=EA60
    FM3_DTIM->TIMER1CONTROL = 0xe2;// 控制寄存器 count cyclus-time, periodic mode, 32 bit
}

/*********************************************************/
/*****    中断等级初始化子程序    ************************/
/*********************************************************/
void Init_IrqLevels(void)
{
	NVIC_SetPriority(DTIM_QDU_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));//Set the Level of the DTIF(Motor Emergency Stop) IST
//	NVIC_SetPriority(ADC0_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 1));//Set the Level of the ADC0 IST
//	NVIC_SetPriority(ADC1_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 1));//Set the Level of the ADC1 IST
//	NVIC_SetPriority(ADC2_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 1));//Set the Level of the ADC2 IST
}

/*********************************************************/
/*****    中断使能初始化子程序    ************************/
/*********************************************************/
void Init_Enable(void)
{
	NVIC_EnableIRQ(DTIM_QDU_IRQn);
//	NVIC_EnableIRQ(ADC0_IRQn);
//	NVIC_EnableIRQ(ADC1_IRQn);
//	NVIC_EnableIRQ(ADC2_IRQn);
}

/**
* This function will initial ADC and the related function.
*
*/
void adc_init(void)
{
	spi_7606_init();
	Init_Timebase();
	Init_IrqLevels();
	Init_Enable();
}

/*@}*/
