/**
 * File    	:  eth_callee.h
 * author	:  mayunliang
 * This file is part of WGB-900
 * Copyright (C), 2012, XJ ELECTRIC Co., LTD.
 * Date     :  2012-07-03
 * Function	:  eth_callee.c中函数的声明

 * Change Logs :
 * Date             Author           Notes
 * 2012-07-03     mayunliang      the first version
*/

#ifndef __ETH_CALLEE_H__
#define __ETH_CALLEE_H__

#include "base_define.h"

#define MAINTENANCE_COMADDR 0
#define PROTECT_COMADDR 1
#define MONITOR_COMADDR 2
#define BROADCAST_COMADDR 0xff

#define SOE_ACTION 1
#define SOE_WARNING 2
#define SOE_DIEVENT 3
#define SOE_GENERAL 4
#define SOE_WAVE 5


/*the  flag of control CB/Tap operation*/
#define _ACT              0x40
#define _SE               0x80
#define _DCO              0x03
#define _FRZ              0xc0
#define _REQ              0x3f

// 获取装置信息的ID号
#define DI_GET_DEV_ADDR 0
#define DI_GET_CURRENT_RATIO 1
#define DI_GET_VOLTAGE_RATIO 2
#define ID_GET_CYCLE_TIMES 3
#define ID_GET_JXSTRAP_STATE 4
#define ID_GET_CURRENT_ZONE 5

///*以太网每个连接对应的报告信息的上送位置
typedef struct _TLINK_INFO
{
	int32_t IPAddr;
	uint8_t ActionReadIndex;
	uint8_t WarningReadIndex;
	uint8_t DInputReadIndex;
	uint8_t GeneralReadIndex;
	uint8_t WaveReadIndex;
}TLINK_INFO;

extern TMeasure103 measure_send[];

void OP_InitMeasureID(uint8_t nINF);
void OP_GetMeasureValue(void);

int OP_SendSomeSOE(uint8_t LinkSocket, uint8_t SOEType, uint8_t SOECount);

uint8_t OP_GetSendMeasureCount(void);

uint8_t OP_GetCurrentSetPointGroup(void);

uint8_t OP_MdfySetPointValue(uint8_t nSetZone, uint8_t* pNewSetValue, uint8_t SetNum, uint8_t SetStart);

uint8_t OP_ModifyTime(TDateTime* tTime,int32_t deviceaddr);


uint8_t OP_GetPowerEnergy103(uint8_t nDET, uint8_t *nInf, TMeterMsg **ppMsg,uint8_t ymdevaddr);//获取电度量

int OP_DetachLink(int socketfd, int clientAddr);//删除正确确认SOE信息的链路信息

int OP_EstablishLink(int socketfd, int clientAddr);	//表示处理SOE信息时的链路号

uint8_t OP_GetThresholdSendMeas(int LinkSocket, uint8_t nFun);//查询是否有越线遥测要上送


//TWaveDescripInfo* OPWAVE_GetCommWaveDescInfo(int, uint8_t);

//TWaveDescripInfo* OPWAVE_GetWaveDescInfo(uint8_t);

TPORT_PARAM_STRUCT* GetLinkPortParam( int );

TDateTime Now(void);

int GetARMSysConfigValue(uint8_t ConfigType);

uint8_t OPWAVE_StartWave(void);

void Process103Meas(void);

void OP_GetDeviceStatus103(uint8_t* pInf, uint8_t** pVal, uint8_t* nNum);


void OP_GetWarningState(uint8_t* pdata, uint8_t* pnNum);// 获取告警信息状态
void OP_GetSoftStrapState(uint8_t* pdata, uint8_t* pnNum);
int8_t OP_GetDIState(uint8_t* pdata, uint8_t* pnNum,uint8_t calldeviceaddr);// 获取所有开入状态

int OP_SignalReset(uint8_t nINF, uint8_t nDCO,uint8_t devaddr);// 信号复归
int OP_MdfySetPointGroup(uint8_t nINF, uint8_t nDCO,uint8_t devaddr);// 定值区切换
uint8_t OP_MdfySoftStrap(uint8_t nINF, uint8_t nDCO,uint8_t devaddr);// 软压板投退

int OP_RemoteCtrlSelt(uint8_t byteFUN, uint8_t byteINF, uint8_t nDCC,uint8_t devaddr);//遥控
int OP_RemoteCtrlExec(uint8_t byteFUN, uint8_t byteINF, uint8_t nDCC, uint8_t LastnDCC,uint8_t devaddr);

uint8_t OP_GetSetPointValue(uint8_t nSetZone, uint8_t* pSendMsg);//定值查询组织数据

uint8_t OP_GetCurrentMeasure103(TMeasure103** ppSendMeas);//遥测数据

void OP_GetSOEQueueSendType(uint8_t LinkSocket, int8_t *pSOEType, uint8_t *pSOECount);
int OP_GetpSOEQueue(uint8_t LinkSocket, uint8_t SOEType, void** ppSOEQueue);
//TSOE_ACTION_STRUCT* OP_GetAppointedSOE(uint8_t FaultNumber);
void* OP_GetSOEQHead(uint8_t SOEType);
void* OP_GetSOEQRear(uint8_t SOEType);

//TDisturbData* OP_SearchDisturb(uint16_t nFAN);

#endif	//_RT_INC_H
