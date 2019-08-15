/**************************************************************************
Copyright (C), 2012, XJ ELECTRIC Co., LTD.
文件名  ：  rtdbdefine.h
作者    ：  wanghusen
项目名称：
功能    ：  数据存储及硬件配置基本定义
创建日期：  2017/9/6
备注    ：
修改记录：
**************************************************************************/
#ifndef _RTDBDEFINE_H
#define _RTDBDEFINE_H
#include "base_define.h"
/*
#define UARTMAXNUM        8

#define IMPORT_NUM        1
#define SAMP_NUM
#define DEVICEMAXNUM      20
#define YKLENGTH          20
#define YTLENGTH          200*/




/*数据存储定义 说明*/

/*

struct CENTERADDR
{
	unsigned char sourceaddr;
	unsigned char sourceindex;
};


struct MESSAGEBUFF
{
	int             length;         //报文长度
	struct CENTERADDR*      Centeraddr;
	unsigned char   message[256];   //报文寄存器
};

struct PROTOCOLBUFF
{
	int             importin;
	int             importout;
	struct MESSAGEBUFF*     importbuff[IMPORT_NUM];     //重要报文寄存器
	int             sampin;
	int             sampout;
	struct MESSAGEBUFF*     sampbuff[SAMP_NUM];         //普通报文寄存器
};*/


/***************************************数据库定义******************************************************/

/*定义数据存储格式*/


/*定义数据缓冲区数据存储结构*/
/*struct RTDBBASEDEF
{
	struct    UARTDEVICESTRUCT*        device[DEVICEMAXNUM];
   // struct    RTDBYCSTRUCT *           ycdatabase;
   // struct    RTDBYXSTRUCT *           yxdatabase;
   // struct    RTDBYMSTRUCT *           ymdatabase;
   // struct    RTDBYKSTRUCT *           ykdatabase;
   // struct    RTDBYTSTRUCT *           ytdatabase;


};*/

/*定义遥信数据变位存储结构*/
/*struct RTDBYXCHGDEF
{
				uint8_t                  devicechgnum;//
	struct    UARTDEVICESTRUCT*        uartdevice;   //
	struct    RTDBYXCHGSTRUCT *        yxchgdata;//

};*/

/*文件基础信息*/
struct FILE_INFDEFINE
{
	uint8_t     file_name;
	uint8_t     file_type;
	uint8_t     file_ver[4];
	uint8_t     file_user;
	uint8_t     filetdate[4];
};


/****************以太网基础信息配置***********************/

struct  ETH_CONF
{
	uint8_t     eth_index;//消息序号
	uint8_t     eth_num;//通道号
	uint8_t     eth_type;//类型[TYPE = SERIAL]
	uint8_t     eth_protocol;//规约类型[PROTOCOL = MODBUS] [PROTOCOL = IEC103]
	uint8_t     eth_daulnet;	//双网标志

	uint32_t    eth_IP1;//IP1地址
	uint8_t     eth_port;//IP1端口

	//uint32_t    eth_IP2;//IP2地址
	//uint8_t     eth_port2;//IP2端口    

	uint16_t    eth_recv_time;//接收超时时间
	uint16_t    eth_send_time;//发送超时
	uint16_t    eth_yctrans_time;//遥测循环发送周期时间
	uint8_t     eth_trans_soe;//是否发送SOE
	uint8_t     eth_trans_soefg;//是否发送SOE复归信号
	uint8_t     eth_yc_datatype;//[YCSENDFLAG = INT16MAX] [YCSENDFLAG = FLOAT32]遥测数据类型
	uint8_t     eth_trans_yxnum;//每帧上送遥信个数
	uint8_t     eth_trans_ycnum;//每帧上送遥测个数
	uint8_t     eth_timingselect;//是否默认校时通道
	uint16_t    eth_yk_timeout;//遥控超时时间

	uint32_t    eth_MASK1;
	uint32_t    eth_GW1;

	//uint32_t    eth_IP2;
	//uint32_t    eth_MASK2;
	//uint32_t    eth_GW2;
};



/****************串口发送数据结构体定义***********************/


struct DEVICEPARA
{

	uint8_t        deviceaddr; //装置地址
	//uint8_t        yccmd; 	
	//uint16_t        ycaddr;//高位在前 低位在后
	uint8_t        ycnum;
	//uint8_t        ycdatalen;//遥测数据长度 不包括帧头（地址+命令） CRC（2个字节）

	//uint8_t        yxcmd; 	    
	//uint16_t        yxaddr;//高位在前 低位在后
	uint8_t        yxnum;
	//uint8_t        yxdatalen;//遥信数据长度 不包括帧头（地址+命令） CRC（2个字节）

   // uint8_t        ykdlcmd; 	//复归 断路器    
   // uint16_t        ykdladdr;//高位在前 低位在后
	uint8_t        ykdlnum;//ykbuf[YKLENGTH];       
   // uint8_t        ykdldatalen;//遥控数据长度 不包括帧头（地址+命令） CRC（2个字节）

   // uint8_t        ykybcmd; 	    // 压板 定值区号
   // uint16_t        ykybaddr;//高位在前 低位在后
	uint8_t        ykybnum;//ykbuf[YKLENGTH];       
   // uint8_t        ykybdatalen;//遥控数据长度 不包括帧头（地址+命令） CRC（2个字节）    

	//uint8_t        ymcmd;//电度量
		//uint16_t        ymaddr;
	uint8_t        ymnum;
	//uint8_t        ymdatalen;

	//uint8_t        reddzcmd;//定值
	//uint16_t        reddzaddr;
	uint8_t        reddznum;


	//uint8_t        ytdzcmd;
	//uint16_t        ytdzaddr;
	uint8_t        ytdznum;


};


struct  UART_CONF
{
	uint8_t                   uart_usedflg;//该串口在用
	uint8_t                   channel_index;//通道序号
	uint8_t                   channel_num;//通道号
	uint8_t                   channel_type;//通道类型
	uint8_t                   uart_protocol;	//规约类型 
	uint16_t                  uart_read_time;//查询间隔时间 
	uint16_t                  uart_receive_time;//接收等待时间    
	uint8_t                   uart_transpond;//网关模式
	uint8_t                   uart_port;//串口号

	uint16_t                   uart_baudrate;//波特率
	uint8_t                   uart_datawidth;//数据宽度

	uint8_t                   uart_stopbit;//停止位 

	uint8_t                   uart_parity;//效验方式

	uint8_t                   uart_type;//串口类型

	uint8_t                   devusednum;    //设备数量
	uint8_t                   devaddr[DEVICEMAXNUM];//设备地址
	//struct DEVICEPARA         deviceconf[DEVICEMAXNUM];//


};




//定义串口设备相关状态
struct UART_TXRX_STRUCT
{
	uint8_t    uart_devonline;

	uint8_t    uart_devicenum;

	uint8_t    uart_readycflg;
	uint8_t    uart_readyxflg;
	uint8_t    uart_ykflg;
	uint8_t    uart_TxOkflg;
	uint8_t    uart_StartTxflg;
	uint8_t    uart_TxErrflg;
	uint8_t    uart_TxErrcount;

	uint8_t    uart_RxOkflg;
	uint8_t    uart_RxErrflg;
	uint8_t    uart_RxErrcount;

	//uint8_t    uart_TxMsgbuf[UART_DATA_LEN];

	//uint8_t    uart_RxMsgbuf[UART_DATA_LEN];

	int32_t		 uart_nASendcount;	//发送包数
	int32_t		 uart_nARecvcount;	//正确接收的包数
	int32_t		 uart_nAErrcount;	//接收错误的包数
};//UART_TXRX_STRUCT;


struct UART_TXRX_STATUS
{
	struct UART_TXRX_STRUCT   uartdevice[DEVICEMAXNUM];

	uint8_t   reddevindex;//按顺序 查询串口下装置 
	uint8_t   uart_rtdbflg;//串口 下有远程操作
	uint8_t   uart_opdevbuf[DEVICEMAXNUM];//要远程操作的装置地址
	uint8_t    uart_redycyxflg;	  //串口下轮询 遥测 遥信 标志
	uint8_t   uart_trans_buf[256];
	uint8_t   uart_recv_buf[256];
	uint8_t   uart_trans_flg;
	uint8_t   uart_recv_flg;
	uint32_t  uart_trans_time;
	uint32_t  uart_recv_time;
};

struct ETH_TXRX_STATUS
{

	uint8_t    eth_trans_flg;
	uint8_t    eth_recv_flg;
	uint32_t   eth_trans_time;
	uint32_t   eth_recv_time;
	uint8_t    eth_trans_buf[300];
	uint8_t    eth_recv_buf[300];
};




#endif




