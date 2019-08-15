/**
 * File    	:  eth_server103.h
 * author	:  mayunliang
 * This file is part of WGB-900
 * Copyright (C), 2012, XJ ELECTRIC Co., LTD.
 * Date     :  2012-07-03
 * Function	:  TCP103ͨѶ

 * Change Logs :
 * Date             Author           Notes
 * 2012-07-03     mayunliang      the first version
*/

#ifndef __ETH_SERVER103_H__
#define __ETH_SERVER103_H__

#include "base_define.h"
#include "base_task.h"

/*ͨѶ�˿ڲ������ݽṹ����*/

typedef struct
{
//	uint16_t enable;              //����Ȩ
//	uint16_t busNo;               //����˿�
//	uint16_t portNo;				//�˿ںţ��ڲ�CPU֮��ͨѶ�˿ںţ���������
//	uint16_t portType;			//�˿�����
//	uint16_t protocolType;		//Э������
//	uint16_t address;				//ͨѶ��ַ
	//��̫���˿ڲ���
    uint32_t  IPAddress;         //IP��ַ
    uint32_t  netMaskAddr;       //��������
    uint32_t  gateWayIPAddr;     //����IP��ַ
    uint32_t  TCP_PortNumber;    //TCP�˿ں�
    uint32_t  UDP_PortNumber;    //UDP�˿ں�
}PORT_CON_STRUCT;


// ����103ͨѶ����
void eth_server103(void);
void RtdbProcess(void);
#endif
