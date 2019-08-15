//#pragma once	//这是C++的防止重复编译的语句，如用C，需要改成
				#ifndef __CMCFGFUNC_H__
				#define __CMCFGFUNC_H__
				#endif


#include <stdio.h>  
#include <stdlib.h>  
#include <string.h> 
#include <stdint.h>
#include <stdbool.h>

#include <base_define.h>
//#define THROW_STRING		//这个宏用来定义解析CFG文件时抛出的字符串，在上位机软件中需要这个功能，通讯管理机时把这个功能撤销

#ifndef THROW_STRING
#define nullptr NULL
#define strtok_s strtok_r
#define _stricmp strcmp
#define _strnicmp strncmp
#endif

#define  SERIALPORT_NUM                           8

#define  KEY_TYPE_MASK                            0xF000
#define  KEY_SUBTYPE_MASK                         0x0F00
#define  KEY_FUNCTION_MASK                        0x00FF

#define  KEY_TYPE_CFG                             0x1000
#define  KEY_TYPE_CHANNEL                         0x2000
#define  KEY_TYPE_DEVICE                          0x3000
#define  KEY_TYPE_UPSTREAM                        0x4000

#define  KEY_CFG_SUPPORT                          0x1000
#define  KEY_CFG_TYPE                             0x1001
#define  KEY_CFG_VER                              0x1002
#define  KEY_CFG_USER                             0x1003
#define  KEY_CFG_DATE                             0x1004
#define  KEY_CFG_TIME                             0x1005

#define  KEY_CHANNEL_INDEX                        0x2000
#define  KEY_CHANNEL_NUM                          0x2001
#define  KEY_CHANNEL_TYPE                         0x2002
#define  KEY_CHANNEL_PROTOCOL                     0x2003
#define  KEY_CHANNEL_INTERVAL                     0x2004
#define  KEY_CHANNEL_TIMEOUT                      0x2005
#define  KEY_CHANNEL_TRANSPOND                    0x2006
#define  KEY_CHANNEL_SERIALPORT                   0x2007
#define  KEY_CHANNEL_SERIALBAUDRATE               0x2008
#define  KEY_CHANNEL_SERIALWIDTH                  0x2009
#define  KEY_CHANNEL_SERIALSTOP                   0x200A
#define  KEY_CHANNEL_SERIALPARITY                 0x200B
#define  KEY_CHANNEL_SERIALTYPE                   0x200C
#define  KEY_CHANNEL_IP1                          0x200D
#define  KEY_CHANNEL_PORT1                        0x200E
#define  KEY_CHANNEL_IP2                          0x200F
#define  KEY_CHANNEL_PORT2                        0x2010
#define  KEY_CHANNEL_DEVICENUM                    0x2011
#define  KEY_CHANNEL_DEVICELIST                   0x2012

#define  KEY_DEVICE                               0x3000
#define  KEY_DEVICE_PARENT                        0x3000
#define  KEY_DEVICE_ADDR                          0x3001
#define  KEY_DEVICE_PROTOCOL                      0x3002
#define  KEY_DEVICE_NAME                          0x3003
#define  KEY_DEVICE_SECTORNUM                     0x3004
#define  KEY_DEVICE_FUN                           0x3005
#define  KEY_DEVICE_YXNUM                         0x3006
#define  KEY_DEVICE_YCNUM                         0x3007
#define  KEY_DEVICE_YKNUM                         0x3008
#define  KEY_DEVICE_YMNUM                         0x3009
#define  KEY_DEVICE_SJNUM                         0x300A

#define  KEY_DEVICE_INFPOINTCFG_YX                0x3100
#define  KEY_DEVICE_INFPOINTCFG_YX_INDEX          0x3100
#define  KEY_DEVICE_INFPOINTCFG_YX_SEND           0x3101
#define  KEY_DEVICE_INFPOINTCFG_YX_COMMAND        0x3102
#define  KEY_DEVICE_INFPOINTCFG_YX_ADDR           0x3103
#define  KEY_DEVICE_INFPOINTCFG_YX_NAME           0x3104
#define  KEY_DEVICE_INFPOINTCFG_YX_SECTOR         0x3105
#define  KEY_DEVICE_INFPOINTCFG_YX_INF            0x3106
#define  KEY_DEVICE_INFPOINTCFG_YX_FUN            0x3107
#define  KEY_DEVICE_INFPOINTCFG_YX_ASDU           0x3108
#define  KEY_DEVICE_INFPOINTCFG_YX_LEVEL          0x3109
#define  KEY_DEVICE_INFPOINTCFG_YX_FUNCTION       0x310A

#define  KEY_DEVICE_INFPOINTCFG_YC                0x3200
#define  KEY_DEVICE_INFPOINTCFG_YC_INDEX          0x3200
#define  KEY_DEVICE_INFPOINTCFG_YC_SEND           0x3201
#define  KEY_DEVICE_INFPOINTCFG_YC_COMMAND        0x3202
#define  KEY_DEVICE_INFPOINTCFG_YC_ADDR           0x3203
#define  KEY_DEVICE_INFPOINTCFG_YC_NAME           0x3204
#define  KEY_DEVICE_INFPOINTCFG_YC_SECTOR         0x3205
#define  KEY_DEVICE_INFPOINTCFG_YC_INF            0x3206
#define  KEY_DEVICE_INFPOINTCFG_YC_FUN            0x3207
#define  KEY_DEVICE_INFPOINTCFG_YC_ASDU           0x3208
#define  KEY_DEVICE_INFPOINTCFG_YC_GAIN           0x3209
#define  KEY_DEVICE_INFPOINTCFG_YC_SLIM           0x320A
#define  KEY_DEVICE_INFPOINTCFG_YC_SHIFT          0x320B
#define  KEY_DEVICE_INFPOINTCFG_YC_OFFSET         0x320C
#define  KEY_DEVICE_INFPOINTCFG_YC_RANGE          0x320D

#define  KEY_DEVICE_INFPOINTCFG_YK                0x3300
#define  KEY_DEVICE_INFPOINTCFG_YK_INDEX          0x3300
#define  KEY_DEVICE_INFPOINTCFG_YK_SEND           0x3301
#define  KEY_DEVICE_INFPOINTCFG_YK_COMMAND        0x3302
#define  KEY_DEVICE_INFPOINTCFG_YK_ADDR           0x3303
#define  KEY_DEVICE_INFPOINTCFG_YK_NAME           0x3304
#define  KEY_DEVICE_INFPOINTCFG_YK_SECTOR         0x3305
#define  KEY_DEVICE_INFPOINTCFG_YK_INF            0x3306
#define  KEY_DEVICE_INFPOINTCFG_YK_FUN            0x3307
#define  KEY_DEVICE_INFPOINTCFG_YK_ASDU           0x3308
#define  KEY_DEVICE_INFPOINTCFG_YK_DIRECT         0x3309
#define  KEY_DEVICE_INFPOINTCFG_YK_RETURN         0x330A
#define  KEY_DEVICE_INFPOINTCFG_YK_TOTALRETURN    0x330B

#define  KEY_DEVICE_INFPOINTCFG_YM                0x3400
#define  KEY_DEVICE_INFPOINTCFG_YM_INDEX          0x3400
#define  KEY_DEVICE_INFPOINTCFG_YM_SEND           0x3401
#define  KEY_DEVICE_INFPOINTCFG_YM_COMMAND        0x3402
#define  KEY_DEVICE_INFPOINTCFG_YM_ADDR           0x3403
#define  KEY_DEVICE_INFPOINTCFG_YM_NAME           0x3404
#define  KEY_DEVICE_INFPOINTCFG_YM_SECTOR         0x3405
#define  KEY_DEVICE_INFPOINTCFG_YM_INF            0x3406
#define  KEY_DEVICE_INFPOINTCFG_YM_FUN            0x3407
#define  KEY_DEVICE_INFPOINTCFG_YM_ASDU           0x3408

#define  KEY_DEVICE_INFPOINTCFG_SJ                0x3500
#define  KEY_DEVICE_INFPOINTCFG_SJ_INDEX          0x3500
#define  KEY_DEVICE_INFPOINTCFG_SJ_SEND           0x3501
#define  KEY_DEVICE_INFPOINTCFG_SJ_COMMAND        0x3502
#define  KEY_DEVICE_INFPOINTCFG_SJ_ADDR           0x3503
#define  KEY_DEVICE_INFPOINTCFG_SJ_NAME           0x3504
#define  KEY_DEVICE_INFPOINTCFG_SJ_SECTOR         0x3505
#define  KEY_DEVICE_INFPOINTCFG_SJ_INF            0x3506
#define  KEY_DEVICE_INFPOINTCFG_SJ_FUN            0x3507
#define  KEY_DEVICE_INFPOINTCFG_SJ_ASDU           0x3508
#define  KEY_DEVICE_INFPOINTCFG_SJ_LEVEL          0x3509
#define  KEY_DEVICE_INFPOINTCFG_SJ_FUNCTION       0x350A

#define  KEY_UPSTREAM_INDEX                       0x4000
#define  KEY_UPSTREAM_NUM                         0x4001
#define  KEY_UPSTREAM_TYPE                        0x4002
#define  KEY_UPSTREAM_PROTOCOL                    0x4003
#define  KEY_UPSTREAM_DAULNET                     0x4004
#define  KEY_UPSTREAM_IP1                         0x4005
#define  KEY_UPSTREAM_PORT1                       0x4006
#define  KEY_UPSTREAM_IP2                         0x4007
#define  KEY_UPSTREAM_PORT2                       0x4008
#define  KEY_UPSTREAM_RECVTIMEOUT                 0x4009
#define  KEY_UPSTREAM_SENDTIMEOUT                 0x400A
#define  KEY_UPSTREAM_YCSENDCYCLE                 0x400B
#define  KEY_UPSTREAM_SOEENABLE                   0x400C
#define  KEY_UPSTREAM_SOERESETENABLE              0x400D
#define  KEY_UPSTREAM_YCSENDFLAG                  0x400E
#define  KEY_UPSTREAM_YXMAXSENDNUM                0x400F
#define  KEY_UPSTREAM_YCMAXSENDNUM                0x4010
#define  KEY_UPSTREAM_TIMINGSELECT                0x4011
#define  KEY_UPSTREAM_YKTIMEOUT                   0x4012
typedef struct
{
	uint16_t u16CfgType;
	int32_t  i32CfgValue;
}CfgOneKeyStruct;

typedef enum
{
	idle = 0,
	read_define = 1,
	read_channel = 2,
	read_device = 3,
	read_upstream = 4,

} CfgState;

typedef enum
{
	unknown = -1,
	none = 0,
	format = 1,
	unfinished = 2,


} CfgError;

typedef struct
{
	CfgState state;
	CfgError error;

	bool b_start_read;
	bool b_define_right;

#ifdef THROW_STRING
	uint32_t  throw_string_index;
	char throw_string[8][64];
#endif
	
}CfgHandleStruct;

uint32_t CfgHandleInit(CfgHandleStruct *pHandle);
uint32_t GetCfgFromCharBuf(CfgHandleStruct *pHandle, uint8_t *buf, CfgOneKeyStruct *pKey);

































