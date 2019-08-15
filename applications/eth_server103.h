/**
 * File    	:  eth_server103.h
 * author	:  mayunliang
 * This file is part of WGB-900
 * Copyright (C), 2012, XJ ELECTRIC Co., LTD.
 * Date     :  2012-07-03
 * Function	:  TCP103通讯

 * Change Logs :
 * Date             Author           Notes
 * 2012-07-03     mayunliang      the first version
*/

#ifndef __ETH_SERVER103_H__
#define __ETH_SERVER103_H__

#include "base_define.h"
#include "base_task.h"

/*通讯端口参数数据结构定义*/

typedef struct
{
//	uint16_t enable;              //控制权
//	uint16_t busNo;               //物理端口
//	uint16_t portNo;				//端口号（内部CPU之间通讯端口号，不连续）
//	uint16_t portType;			//端口类型
//	uint16_t protocolType;		//协议类型
//	uint16_t address;				//通讯地址
	//以太网端口参数
    uint32_t  IPAddress;         //IP地址
    uint32_t  netMaskAddr;       //子网掩码
    uint32_t  gateWayIPAddr;     //网关IP地址
    uint32_t  TCP_PortNumber;    //TCP端口号
    uint32_t  UDP_PortNumber;    //UDP端口号
}PORT_CON_STRUCT;


// 启动103通讯任务
void eth_server103(void);
void RtdbProcess(void);
#endif
