/*
 * File      : e2timewdg.h
 *
 * Change Logs:
 * Date           Author       Notes
 * 2012-09-11     changyufeng  first version
 */
#ifndef __E2TIMEWDG_H__
#define __E2TIMEWDG_H__

#include <rthw.h>
#include <rtthread.h>
#include "base_define.h"

//new
#define  SDA_DIR  bFM3_GPIO_DDR4_P7// 0 input  1 output

#define  SDA_I    bFM3_GPIO_PDIR4_P7//as input
#define  SDA_O    bFM3_GPIO_PDOR4_P7// as output
#define  SCL_O    bFM3_GPIO_PDOR4_P6

#define  SDA_PFR  bFM3_GPIO_PFR4_P7
#define  SCL_PFR  bFM3_GPIO_PFR4_P6
#define  SDA_DDR  bFM3_GPIO_DDR4_P7
#define  SCL_DDR  bFM3_GPIO_DDR4_P6



uint32_t rjsz;


uint8_t dma_status;

void write_rtc_byte(uint8_t addr,uint8_t data_byte);
void I2C_Read_nbyte(unsigned char *buf1,unsigned int MemAddr,unsigned char length);
void I2C_Write_nbyte(unsigned int MemAddr,unsigned char *buf1,unsigned char length);
void get_rtc(void);
void set_rtc(void);
void Kick_WatchDog(void);
void Init_WatchDog(void);
void system_time_increase(void);
void FM32156_init(void);

#endif
