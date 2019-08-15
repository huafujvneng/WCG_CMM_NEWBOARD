/**
 * File    	:  base_task.h
 * author	:  mayunliang
 * This file is part of WGB-900
 * Copyright (C), 2012, XJ ELECTRIC Co., LTD.
 * Date     :  2012-10-03
 * Function	:  处理日常事务，初始化系统、存取eeprom、执行各项自检
               读取测量数据、清看门狗、运行灯、对时等

 * Change Logs :
 * Date             Author           Notes
 * 2012-10-03     mayunliang      the first version
*/


#ifndef __BASE_TASK_H__
#define __BASE_TASK_H__

#include <mb9b610s.h>


#include "rtdbdefine.h"


////E2存储配置信息 地址
#define ADDR_DEFINE          0x0000//存储配置文件基础信息

#define UART_CONF_AREA_0     0x0020//串口0配置

#define UART_CONF_AREA_1     0x1000//串口1配置

#define UART_CONF_AREA_3     0x2000//串口3配置

#define UART_CONF_AREA_4     0x3000//串口配置4

#define UART_CONF_AREA_5     0x4000//串口配置5

#define UART_CONF_AREA_6     0x5000//串口配置6

#define UART_CONF_AREA_7     0x6000//串口配置7

#define ETH_CONF_AREA        0x7000//网口配置


#define CAL_CONF_AREA        0x7ffd//网口配置

//E2存储配置信息 类型
#define TYPE_RECORD_DEFINE        0x01000000
#define TYPE_RECORD_UART0         0x02000000
#define TYPE_RECORD_UART1         0x03000000
#define TYPE_RECORD_UART2         0x04000000
#define TYPE_RECORD_UART3         0x05000000
#define TYPE_RECORD_UART4         0x06000000
#define TYPE_RECORD_UART5         0x07000000
#define TYPE_RECORD_UART6         0x08000000
#define TYPE_RECORD_UART7         0x09000000
#define TYPE_RECORD_ETH           0x0A000000






//MRAM存储地址定义
#define ADDR_RECORD_ACT_PST      0
#define ADDR_RECORD_WARNING_PST    4500
#define ADDR_RECORD_INPUT_PST    9000
#define ADDR_RECORD_OPERATE_PST  10500
#define ADDR_RECORD_ACT         1
#define ADDR_RECORD_WARNING       4501
#define ADDR_RECORD_INPUT       9001
#define ADDR_RECORD_OPERATE     10501

#define ADDR_YB                 12000
#define ADDR_ZONE_USE           12100
#define ADDR_SET_ZONE0          0x003000
#define ADDR_SET_ZONE1          0x003100
#define ADDR_SET_ZONE2          0x003200
#define ADDR_SET_ZONE3          0x003300
#define ADDR_SET_ZONE4          0x003400
#define ADDR_SET_ZONE5          0x003500
#define ADDR_SET_ZONE6          0x003600
#define ADDR_SET_ZONE7          0x003700

#define ADDR_SYSSET_COMM        0x003800
#define ADDR_SYSSET_METER       0x003850
#define ADDR_SYSSET_PASSWD      0x003860
#define ADDR_SYSSET_PARAM       0x003870
#define ADDR_SYSSET_OUTPUT      0x003890
#define ADDR_SYSSET_SELFCHK     0x0038e0

#define ADDR_7022_CRC           0x0038fc
#define ADDR_CALIBRATE_7022     0x003900

#define ADDR_CALIBRATE_ADC      0x003930

#define ADDR_CHANNEL_DCS        0x003970
#define ADDR_CHANNEL_DCIN      0x003980

#define ADDR_SN                 0x003990

#define ADDR_SAVE_WAVE_PST     0x003FF0
#define ADDR_SAVE_WAVE_nFAN    0x003FF1
#define ADDR_SAVE_WAVE1         0x004000
#define ADDR_SAVE_WAVE2
#define ADDR_SAVE_WAVE3
#define ADDR_SAVE_WAVE4
#define ADDR_SAVE_WAVE5

//MRAM保存数据的类型
#define TYPE_RECORD_ACT         0x01000000
#define TYPE_RECORD_WARNING     0x02000000
#define TYPE_RECORD_INPUT       0x03000000
#define TYPE_RECORD_OPERATE     0x04000000
#define TYPE_RECORD_DELETE      0x05000000
#define TYPE_RECORD_WAVE        0x06000000

#define TYPE_SET_YB         0x11000000
#define TYPE_SET_SET        0x12000000
#define TYPE_SET_ZONE       0x13000000

#define TYPE_CALIBRATE_MEASURE  0x21000000
#define TYPE_CALIBRATE_PROTECT  0x22000000
#define TYPE_CALIBRATE_MEAS_CRC 0x23000000

#define TYPE_SYSSET_COMM        0x31000000
#define TYPE_SYSSET_METER       0x32000000
#define TYPE_SYSSET_PASSWORD    0x33000000
#define TYPE_SYSSET_PARAMETER   0x34000000
#define TYPE_SYSSET_BKPPORT     0x35000000
#define TYPE_SYSSET_SELFCHK     0x36000000
#define TYPE_SYSSET_DCOUT       0x37000000
#define TYPE_SYSSET_DCIN        0x38000000
#define TYPE_SYSSET_SN          0x39000000



extern uint32_t eth_IP1,eth_MASK1,eth_GW1,eth_IP2,eth_MASK2,eth_GW2;
extern struct rt_mailbox mb;
extern uint16_t FaultNumber;
extern uint16_t  soe_fgfs;
extern uint16_t  time_correct_mode;
extern uint8_t sn[];

///*base_task.c中调用的外部函数声明
int rt_application_init(void);

void measure(void);


void IC03_1_init(void);
void Init_arm_inf(void);
void OP_InitMeasureID(uint8_t nINF);





#endif
