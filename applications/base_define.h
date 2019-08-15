/**
 * File    	:  base_define.h
 * author	:  mayunliang
 * This file is part of WGB-900
 * Copyright (C), 2012, XJ ELECTRIC Co., LTD.
 * Date     :  2012-07-03
 * Function	:  基本的数据结构和宏定义

 * Change Logs :
 * Date             Author           Notes
 * 2012-07-03     mayunliang      the first version
 *2017-05-13      wanghusen       add com  define
*/

#ifndef __BASE_DEFINE_H__
#define __BASE_DEFINE_H__

#include <rtthread.h>
#include "board.h"
//#include "stdlib.h"


//常量定义区
#define HAVE_ERROR				-1	//有错误
#define ERROR					-1	//有错误
#define NO_ERROR				0	//无错误
#define OK						0	//无错误
#define TRUE					1	//
#define FALSE					0	//



#define UART_DATA_LEN     255

//数据库常量定义区
#define RTDB_CHANL_NUM    20//定义内存中存储外设通道 外接设备
#define RTDB_YC_MAXNUM    256//定义 一帧遥测报文 最大长度
#define RTDB_YX_MAXNUM    256//定义 一帧遥信报文 最大长度
#define RTDB_YM_MAXNUM    50//定义 一帧遥信报文 最大长度
#define RTDB_YX_CHGMAXNUM    20//定义 存储变位数据最大通道数
#define RTDB_YK_MAXNUM    20//定义 一帧遥控报文 最大长度
#define RTDB_DZ_MAXNUM    256//定义 一帧定值报文 最大长度




//定义串口灯


#define serial_led0_on() bFM3_GPIO_PDOR3_PF = 0;
#define serial_led0_off() bFM3_GPIO_PDOR3_PF = 1;
#define serial_led0_flash() bFM3_GPIO_PDOR3_PF ^= 1;


#define serial_led1_on() bFM3_GPIO_PDOR3_PE = 0;
#define serial_led1_off() bFM3_GPIO_PDOR3_PE = 1;
#define serial_led1_flash() bFM3_GPIO_PDOR3_PE ^= 1;


#define serial_led3_on() bFM3_GPIO_PDOR3_PD = 0;
#define serial_led3_off() bFM3_GPIO_PDOR3_PD = 1;
#define serial_led3_flash() bFM3_GPIO_PDOR3_PD ^= 1;


#define serial_led4_on() bFM3_GPIO_PDOR3_PC = 0;
#define serial_led4_off() bFM3_GPIO_PDOR3_PC = 1;
#define serial_led4_flash() bFM3_GPIO_PDOR3_PC ^= 1;


#define serial_led5_on() bFM3_GPIO_PDOR3_PB = 0;
#define serial_led5_off() bFM3_GPIO_PDOR3_PB = 1;
#define serial_led5_flash() bFM3_GPIO_PDOR3_PB ^= 1;


#define serial_led6_on() bFM3_GPIO_PDOR3_PA = 0;
#define serial_led6_off() bFM3_GPIO_PDOR3_PA = 1;
#define serial_led6_flash() bFM3_GPIO_PDOR3_PA ^= 1;


#define serial_led7_on() bFM3_GPIO_PDOR3_P9 = 0;
#define serial_led7_off() bFM3_GPIO_PDOR3_P9 = 1;
#define serial_led7_flash() bFM3_GPIO_PDOR3_P9 ^= 1;


#define serial_led8_on() bFM3_GPIO_PDOR3_P8 = 0;
#define serial_led8_off() bFM3_GPIO_PDOR3_P8 = 1;
#define serial_led8_flash() bFM3_GPIO_PDOR3_P8 ^= 1;



#define DEVICE_NAME "XJDEVICE"
#define DEVICE_VER  0x0100 //2017/2/24 星期五 下午 4:05:04
#define PROTECT_FUN 248

#define MONITOR_FUN 1
#define GLOBAL_FUN 0xff

#define CT_VALUE_2	5.0		//电流二次侧额定值
#define PT_VALUE_2	100.0	//电压二次侧额定值

#define REPORT_SUM 200      //报告个数

 
#define	YFKR	4
#define	CNKR	5
#define	TW	    1
#define	HW	    0
#define	JXZT	17
#define    ASDU_1            1
#define    ASDU_2            2
#define    ASDU_41           41
#define    ASDU_43           43


#define DZ_NUM 100       //最大定值长度
//#define YB_NUM 14            //压板长度
//#define PRO_NUM 19           //保护个数

#define UARTMAXNUM        8

#define IMPORT_NUM        1
#define SAMP_NUM   
#define DEVICEMAXNUM      20
#define YKLENGTH          20
#define YTLENGTH          200


//#define PROTOCOLNUM       2//该版本程序所用规约类型个数 

//遥控数据类型 遥控刀闸 0   复归  1   遥控压板 2  修改定值 3    对时 4  定值区切换 5
#define REMOTE_DL_TYPE   0//
#define REMOTE_FG_TYPE   1
#define REMOTE_YB_TYPE   2
#define REMOTE_DZ_TYPE   3
#define REMOTE_TIME_TYPE 4
#define REMOTE_QHDZ_TYPE 5



///*定义数据在磁电存储器中的地址

#define WAWE_ADDR   0x0001
#define ETH_ADDR    0x0002


///*时间结构体定义
typedef struct _TDateTime
{
    uint8_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint16_t  msec;
}TDateTime;

///*开入结构体定义
typedef struct _TDIState103
{
     uint8_t      nDiInf;             //开关量序号
     uint8_t      n103Inf;            //INF号
     uint8_t      StableState;        //稳定状态
     uint8_t      ThisState;          //当前状态
     uint8_t      JitterFlg;          //状态抖动
     uint8_t      ChangeFlg;          //状态改变标志
     uint8_t      JitterCnt;          //抖动计数
     TDateTime  dtTime;             //时间
}TDIState103;



///*测量值对应的结构体，包括对应多个连接的主动上送标记
typedef struct _TMeasure103
{
	uint8_t measID;
	int16_t measVal;
	uint16_t bAutoSendFlg;    //主动上送标记
}TMeasure103;

typedef struct _TMeterMsg
{
	int32_t meterVal;     //电度值
	uint8_t SequenceNum;  //顺序号
}TMeterMsg;



///*动作信息结构体
typedef struct _TSOE_ACTION_STRUCT
{
	uint8_t nValid;       //报告有效标志低两位，1—有效 0-无效
                        //X X X X X X 显示 上送(注：如果不上送，上送标志位置0)
	uint8_t nPortID;      //报告对应的设备端口号
	uint8_t ndeviceaddr;   //产生报告的下位机地址	
	uint8_t n103Inf;      //报告信息序号
	

	uint8_t nDPI;         //动作_返回标志 2-动作 1-返回
	TDateTime dtTime;   //报告动作时间
	uint16_t nPosTime;    //报告动作相对时间
	uint16_t nFAN;        //故障序号
	int32_t ResultVal[6]; //报告动作结果数据
	uint8_t nResultCnt;   //报告动作结果数量
}TSOE_ACTION_STRUCT;

///*告警信息结构体
typedef struct _TSOE_WARNING_STRUCT
{
	uint8_t nValid;       //报告有效标志低两位，1—有效 0-无效
                        //X X X X X X 显示 上送(注：如果不上送，上送标志位置0)
	uint8_t nPortID;      //报告对应的设备端口号
	uint8_t ndeviceaddr;   //产生报告的下位机地址	
	uint8_t n103Inf;      //报告信息序号

	uint8_t nDPI;         //动作_返回标志 2-动作 1-返回
	TDateTime dtTime;   //报告动作时间
	int32_t ResultVal[6]; //报告动作结果数据
	uint8_t nResultCnt;   //报告动作结果数量
}TSOE_WARNING_STRUCT;

///*开入信息结构体
typedef struct _TSOE_DIEVENT_STRUCT
{
	uint8_t nValid;       //报告有效标志低两位，1—有效 0-无效
                        //X X X X X X 显示 上送(注：如果不上送，上送标志位置0)
	uint8_t nPortID;      //报告对应的设备端口号
	uint8_t ndeviceaddr;   //产生报告的下位机地址	
	uint8_t n103Inf;      //报告信息序号

	uint8_t nSPI;         //动作_返回标志 1-动作 0-返回
	TDateTime dtTime;   //报告动作时间
}TSOE_DIEVENT_STRUCT;


///*操作信息结构体
typedef struct _TSOE_OPERATION_STRUCT
{
	uint8_t nValid;       //报告有效标志低两位，1—有效 0-无效
                        //X X X X X X 显示 上送(注：如果不上送，上送标志位置0)
	uint8_t nDispID;      //报告对应的显示序号，和显示信息表有关
	TDateTime dtTime;   //报告动作时间
}TSOE_OPERATION_STRUCT;


///*总信号信息结构体（包括压板变位,跳闸返回报告，对点）
typedef struct _TSOE_GENERAL_STRUCT
{
	uint8_t nValid;       //报告有效标志低两位，1—有效 0-无效
                        //X X X X X X 显示 上送(注：如果不上送，上送标志位置0)
	uint8_t nDispID;      //报告对应的显示序号，和显示信息表有关
	uint8_t n103Inf;      //报告信息序号，可以和nDispID通过一个数组对应起来对点直接填INF
	uint8_t nASDU;        //类别标示ASDU
	uint8_t nCOT;         //传输原因
	uint8_t nDPI;         //动作_返回标志 2-动作 1-返回
	TDateTime dtTime;   //报告动作时间
}TSOE_GENERAL_STRUCT;


typedef struct _TPORT_PARAM_STRUCT
{
    int nTxPacks;
    int nRxPacks;

}TPORT_PARAM_STRUCT;

extern uint16_t DEVICE_CRC;

//定值相关结构体定义
typedef struct _TSET_PARA_STRUCT
{
	uint16_t max;
	uint16_t min;
	uint8_t  scale;  //系数
}TSET_PARA_STRUCT;






#endif



