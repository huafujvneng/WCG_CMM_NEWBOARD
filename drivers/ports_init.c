/**
 * File    	:  ports_init.c
 * author	:  mayunliang
 * This file is part of WGB-900
 * Copyright (C), 2012, XJ ELECTRIC Co., LTD.
 * Date     :  2012-11-02
 * Function	:  初始化总线接口，I/O口，灯

 * Change Logs :
 * Date             Author           Notes
 * 2012-10-03     mayunliang      the first version
 *2017-08-30      wanghusen
*/

#include <rthw.h>
#include <rtthread.h>

#include "board.h"

//#define TEST bFM3_GPIO_PDOR8_P1

/**
 * Initialize all for i/o port.
 *
 */
/*********************************************************/
/*****  外部总线初始化程序  ******************************/
/*********************************************************/
void Init_ExtBus(void)
{
	FM3_GPIO->PFR4 = 0x00007c00;// P4A -- P4E as extbus MDATA[0:4]
	FM3_GPIO->PFR7 = 0x000007ff;// P70 -- P7A as extbus MDATA[5:15]
	FM3_GPIO->PFR1 = 0x0000fff0;// P14 -- P1F as extbus MAD[00:11]
	FM3_GPIO->PFR2 = 0x000001f0;// P24 -- P29 as extbus MAD[12:17]

	bFM3_GPIO_PFR5_P0 = 1;      // P50 use MOEX
	bFM3_GPIO_PFR5_P1 = 1;      // P51 use MWEX

	//bFM3_GPIO_PFR5_PB = 1;      // P5B use MCSX1 AD1CS
	//bFM3_GPIO_PFR1_P3 = 1;      // P13 use MCSX4 AD2CS
	bFM3_GPIO_PFR1_P3 = 1;      // P13 use MCSX4 SRAMCS

	//FM3_EXBUS->MODE1 = 0x00000001;
	//FM3_EXBUS->TIM1  = 0x00020000; // 0xfeefffff;
	//FM3_EXBUS->AREA1 = 0x00000010; // Start CS1 0x6100 0000 length 1MB ADC1)

	FM3_EXBUS->MODE4 = 0x00000001;//16bit sram
	FM3_EXBUS->TIM4  = 0x00020000; // 0xfeefffff;
	FM3_EXBUS->AREA4 = 0x00000040; // Start CS4 0x6400 0000 length 1MB SRAM)

	//FM3_EXBUS->MODE7 = 0x00000001;
	//FM3_EXBUS->TIM7  = 0x00020000; // 0xfeefffff;
	//FM3_EXBUS->AREA7 = 0x00000070; // Start CS7 0x6700 0000 length 1MB SRAM)

	FM3_GPIO->EPFR10 = 0x00000000;
	//bFM3_GPIO_EPFR10_UECS1E = 1;    // use CS1 single
	bFM3_GPIO_EPFR10_UECS4E = 1;    // use CS4 single
	//bFM3_GPIO_EPFR10_UECS7E = 1;    // use CS7 single
	bFM3_GPIO_EPFR10_UEOEXE = 1;    // use RD(OEX) single
	bFM3_GPIO_EPFR10_UEWEXE = 1;    // use WR(WEX) single
	bFM3_GPIO_EPFR10_UEA17E = 1;    // use MAD17 single
	bFM3_GPIO_EPFR10_UEA16E = 1;    // use MAD16 single
	bFM3_GPIO_EPFR10_UEA15E = 1;    // use MAD15 single
	bFM3_GPIO_EPFR10_UEA14E = 1;    // use MAD14 single
	bFM3_GPIO_EPFR10_UEA13E = 1;    // use MAD13 single
	bFM3_GPIO_EPFR10_UEA12E = 1;    // use MAD12 single
	bFM3_GPIO_EPFR10_UEA11E = 1;    // use MAD11 single
	bFM3_GPIO_EPFR10_UEA10E = 1;    // use MAD10 single
	bFM3_GPIO_EPFR10_UEA09E = 1;    // use MAD09 single
	bFM3_GPIO_EPFR10_UEA08E = 1;    // use MAD08 single
	bFM3_GPIO_EPFR10_UEDTHB = 1;    // use UEDTHB single
	bFM3_GPIO_EPFR10_UEDEFB = 1;    // use UEDEFB single
	FM3_EXBUS->DCLKR = 0x13; // 0x1f

} 
 
 
void ports_init(void)
{

    FM3_GPIO->ADE =  0; // P10~1F used to ADC

    FM3_GPIO->PFR3 &= 0x00ff;  // set P38-f fuction is GPIO.
    FM3_GPIO->DDR3 |= 0xff00;    // set P38-f  output.
    FM3_GPIO->PDOR3|= 0xff00;    // set P38-f  high level.

    FM3_GPIO->SPSR &= 0xfffe;   //屏蔽P46 P47的晶振功能

//init run led
    bFM3_GPIO_PFRF_P6 = 0; // set P80 fuction is GPIO.
    bFM3_GPIO_DDRF_P6 = 1; // set P80 output.
    
 
    bFM3_GPIO_PFR4_P6 = 0; // set P46 fuction is GPIO.
    bFM3_GPIO_DDR4_P6 = 1; // set P46 output.
    bFM3_GPIO_PFR4_P7 = 0; // set P47 fuction is GPIO.
    bFM3_GPIO_DDR4_P7 = 1; // set P47 output.    
   // bFM3_GPIO_PDOR4_P6 = 0;
   // bFM3_GPIO_PDOR4_P7 = 0;  
  
}

/*@}*/
