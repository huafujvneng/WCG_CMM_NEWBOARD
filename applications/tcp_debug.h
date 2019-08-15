#ifndef _TCP_DEBUG_H_
#define _TCP_DEBUG_H_

#include <stdint.h>
//#include <stdio.h>
#include <rtthread.h>
#include "base_task.h"
#include "fm3_ext_int.h"
#include <string.h>
#include <lwip/api.h>
#include <lwip/sockets.h>

#include "ethernetif.h"
#include "base_define.h"
#include "rtdbdefine.h"
#define		TCP_DEBUG_EN

#define		DEBUG_BUF_SIZE		2048ul
#define		DEBUG_PRINTF_MAX	256ul
#define		DEBUG_RXBUF_SIZE	256ul


#define		PEEK			(1)


//define the server cmd
//控制信息为十个字节 55 AA 00 0A 00 00 00 00 00 00
/*    帧头      数据个数   发送序列号   接收序列号   控制命令              
      2 byte      2 byte      2 byte      2 byte      2 byte 
*/
#define START_CODE	0x55AA			//帧头			

#define CMPROTOCOL_COMMAND_BASE_MASK								0xF000	//主命令 掩码
#define CMPROTOCOL_COMMAND_SUB_MASK									0x0FFF	//子命令 掩码

#define CMPROTOCOL_COMMAND_COMMON									0x0000	//公共命令 主键
#define CMPROTOCOL_COMMAND_COMMON_DISCOVER							0x0000	//公共命令 子健 探知装置 
#define CMPROTOCOL_COMMAND_COMMON_INIT								0x0001	//公共命令 子健 初始化，获取装置信息
#define CMPROTOCOL_COMMAND_COMMON_ESTABLISHED						0x0002	//公共命令 子健 告知装置已连接
#define CMPROTOCOL_COMMAND_COMMON_RESET								0x0003	//公共命令 子健 重启装置

#define CMPROTOCOL_COMMAND_STREAM_CTL							    0x1000	//报文控制命令 主键
#define CMPROTOCOL_COMMAND_STREAM_CTL_STREAM_BASE_MASK				0x0080	//报文控制命令 子健 通道类型掩码（选择上行通道还是装置通道）
#define CMPROTOCOL_COMMAND_STREAM_CTL_STREAM_INDEX_MASK				0x007F	//报文控制命令 子健 通道号掩码
#define CMPROTOCOL_COMMAND_STREAM_CTL_UPSTREAM_BASE					0x0000	//报文控制命令 子健 上行通道号基值 eth
#define CMPROTOCOL_COMMAND_STREAM_CTL_UPSTREAM_BASE_ALL				0x007F	//报文控制命令 子健 所有上行通道
#define CMPROTOCOL_COMMAND_STREAM_CTL_DEVICESTREAM_BASE				0x0080	//报文控制命令 子健 装置通道号基值 device
#define CMPROTOCOL_COMMAND_STREAM_CTL_DEVICESTREAM_BASE_ALL			0x007F	//报文控制命令 子健 所有装置通道
#define CMPROTOCOL_COMMAND_STREAM_CTL_STREAM_TX						0x0100	//报文控制命令 子健 控制通道上送发送报文使能状态
#define CMPROTOCOL_COMMAND_STREAM_CTL_STREAM_RX						0x0200	//报文控制命令 子健 控制通道上送接受报文使能状态
#define CMPROTOCOL_COMMAND_STREAM_CTL_STREAM_ON						0x0400	//报文控制命令 子健 控制通道上送报文使能
#define CMPROTOCOL_COMMAND_STREAM_CTL_STREAM_OFF					0x0800	//报文控制命令 子健 控制通道上送报文禁止

#define CMPROTOCOL_COMMAND_STREAM_DATA								0x2000	//报文数据命令 主键 回传
#define CMPROTOCOL_COMMAND_STREAM_DATA_STREAM_BASE_MASK				0x0080	//报文数据命令 子健 通道类型掩码（选择上行通道还是装置通道）
#define CMPROTOCOL_COMMAND_STREAM_DATA_STREAM_INDEX_MASK			0x007F	//报文数据命令 子健 通道号掩码
#define CMPROTOCOL_COMMAND_STREAM_DATA_UPSTREAM_BASE				0x0000	//报文数据命令 子健 上行通道号基值
#define CMPROTOCOL_COMMAND_STREAM_DATA_DEVICESTREAM_BASE			0x0080	//报文数据命令 子健 装置通道号基值
#define CMPROTOCOL_COMMAND_STREAM_DATA_STREAM_TX					0x0100	//报文数据命令 子健 报文为对应通道发送的
#define CMPROTOCOL_COMMAND_STREAM_DATA_STREAM_RX					0x0200	//报文数据命令 子健 报文为对应通道接收的

#define CMPROTOCOL_COMMAND_CFG										  0x4000	//配置操作命令 主键
#define CMPROTOCOL_COMMAND_CFG_CTL									0x0000	//配置操作命令 子健 控制配置文件
#define CMPROTOCOL_COMMAND_CFG_SEND									0x0100	//配置操作命令 子健 发送配置文件
#define CMPROTOCOL_COMMAND_CFG_GET									0x0200	//配置操作命令 子健 读取配置文件

#define CMPROTOCOL_COMMAND_TXFLG                    0X00 //
#define CMPROTOCOL_COMMAND_RXFLG                    0X01
#define CMPROTOCOL_COMMAND_UPFLG                    0X00
#define CMPROTOCOL_COMMAND_DOWNFLG                  0X01


typedef struct __attribute__((__aligned__(4)))
{
	volatile uint32_t busy;
	
	volatile uint16_t write_p;
	volatile uint16_t send_p;
	
	volatile uint16_t recv_p;
	volatile uint16_t handle_p;
	
	char buf[DEBUG_BUF_SIZE];
	char buf1[16];
	char rxbuf[DEBUG_RXBUF_SIZE];
} DEBUGBUF;


void tcp_debug_init(void);
//uint8_t	tcp_debug_printf(const char *fmt, ...);
void eth_server_debug(void);

void debug_buf_write(const char *buf, uint8_t size);
uint32_t debug_buf_read(char *buf, uint32_t size, uint32_t flag);

#endif 
