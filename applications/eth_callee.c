/**
 * File    	:  eth_callee.c
 * author	:  mayunliang
 * This file is part of WGB-900
 * Copyright (C), 2012, XJ ELECTRIC Co., LTD.
 * Date     :  2012-07-03
 * Function	:  以太网通讯中调用的函数定义

 * Change Logs :
 * Date             Author           Notes
 * 2012-07-03     mayunliang      the first version
 * 2017-12-01     wanghusen       add new functiong  for Communication Manager
*/

#include <lwip/api.h>
#include "eth_callee.h"
#include "rtdbdefine.h"

//#include "protect.h"
#include "adc_task.h"
#include "485comm.h"

#define LINK_MAX 12




#define YKTYPE_TZ 0
#define YKTYPE_FG 1
#define YKTYPE_YB 2
#define YKTYPE_DZQH 3

#define YKTZ_SELECT 0
#define YKTZ_EXEC 1


//SOE define
extern TSOE_ACTION_STRUCT   protect_soe[];   //动作报告
extern TSOE_WARNING_STRUCT  alarm_soe[];  //告警报告
extern TSOE_DIEVENT_STRUCT  change_soe[];    //开入报告
extern TSOE_GENERAL_STRUCT  general_report[]; //总信号报告
extern uint8_t protect_soe_position;
extern uint8_t alarm_soe_position;
extern uint8_t change_soe_position;
extern uint8_t general_report_position;
extern uint8_t  disturb_position;
extern  struct  UART_CONF uartpara[UARTMAXNUM];
//struct uart_txconf uartpara[UARTMAXNUM];
extern struct  UART_TXRX_STATUS uart_status[UARTMAXNUM];
/*
传输原因：2
公共地址：2
信息体地址：3

*/
//#define REPORT_SUM 100
//TSOE_ACTION_STRUCT protect_soe[REPORT_SUM];
//TSOE_WARNING_STRUCT alarm_soe[REPORT_SUM];
//TSOE_DIEVENT_STRUCT change_soe[REPORT_SUM];

extern rt_uint16_t yc_data_buf0[RTDB_CHANL_NUM][RTDB_YC_MAXNUM];// save yc data
extern rt_uint16_t yx_data_buf0[RTDB_CHANL_NUM][RTDB_YX_MAXNUM];//save yx data
extern rt_uint16_t ym_data_buf0[RTDB_CHANL_NUM][RTDB_YM_MAXNUM];//save ym data
extern rt_uint16_t yx_chgdata_buf0[RTDB_YX_CHGMAXNUM][RTDB_YX_MAXNUM];//save yx change data
extern rt_uint16_t yk_data_buf0[RTDB_CHANL_NUM][RTDB_YC_MAXNUM];// save yk data

extern rt_uint16_t yc_data_buf1[RTDB_CHANL_NUM][RTDB_YC_MAXNUM];// save yc data
extern rt_uint16_t yx_data_buf1[RTDB_CHANL_NUM][RTDB_YX_MAXNUM];//save yx data
extern rt_uint16_t ym_data_buf1[RTDB_CHANL_NUM][RTDB_YM_MAXNUM];//save ym data
extern rt_uint16_t yx_chgdata_buf1[RTDB_YX_CHGMAXNUM][RTDB_YX_MAXNUM];//save yx change data
extern rt_uint16_t yk_data_buf1[RTDB_CHANL_NUM][RTDB_YC_MAXNUM];// save yk data

extern rt_uint16_t yc_data_buf3[RTDB_CHANL_NUM][RTDB_YC_MAXNUM];// save yc data
extern rt_uint16_t yx_data_buf3[RTDB_CHANL_NUM][RTDB_YX_MAXNUM];//save yx data
extern rt_uint16_t ym_data_buf3[RTDB_CHANL_NUM][RTDB_YM_MAXNUM];//save ym data
extern rt_uint16_t yx_chgdata_buf3[RTDB_YX_CHGMAXNUM][RTDB_YX_MAXNUM];//save yx change data
extern rt_uint16_t yk_data_buf3[RTDB_CHANL_NUM][RTDB_YC_MAXNUM];// save yk data

extern rt_uint16_t yc_data_buf4[RTDB_CHANL_NUM][RTDB_YC_MAXNUM];// save yc data
extern rt_uint16_t yx_data_buf4[RTDB_CHANL_NUM][RTDB_YX_MAXNUM];//save yx data
extern rt_uint16_t ym_data_buf4[RTDB_CHANL_NUM][RTDB_YM_MAXNUM];//save ym data
extern rt_uint16_t yx_chgdata_buf4[RTDB_YX_CHGMAXNUM][RTDB_YX_MAXNUM];//save yx change data
extern rt_uint16_t yk_data_buf4[RTDB_CHANL_NUM][RTDB_YC_MAXNUM];// save yk data


extern rt_uint16_t yc_data_buf5[RTDB_CHANL_NUM][RTDB_YC_MAXNUM];// save yc data
extern rt_uint16_t yx_data_buf5[RTDB_CHANL_NUM][RTDB_YX_MAXNUM];//save yx data
extern rt_uint16_t ym_data_buf5[RTDB_CHANL_NUM][RTDB_YM_MAXNUM];//save ym data
extern rt_uint16_t yx_chgdata_buf5[RTDB_YX_CHGMAXNUM][RTDB_YX_MAXNUM];//save yx change data
extern rt_uint16_t yk_data_buf5[RTDB_CHANL_NUM][RTDB_YC_MAXNUM];// save yk data

extern rt_uint16_t yc_data_buf6[RTDB_CHANL_NUM][RTDB_YC_MAXNUM];// save yc data
extern rt_uint16_t yx_data_buf6[RTDB_CHANL_NUM][RTDB_YX_MAXNUM];//save yx data
extern rt_uint16_t ym_data_buf6[RTDB_CHANL_NUM][RTDB_YM_MAXNUM];//save ym data
extern rt_uint16_t yx_chgdata_buf6[RTDB_YX_CHGMAXNUM][RTDB_YX_MAXNUM];//save yx change data
extern rt_uint16_t yk_data_buf6[RTDB_CHANL_NUM][RTDB_YC_MAXNUM];// save yk data

extern rt_uint16_t yc_data_buf7[RTDB_CHANL_NUM][RTDB_YC_MAXNUM];// save yc data
extern rt_uint16_t yx_data_buf7[RTDB_CHANL_NUM][RTDB_YX_MAXNUM];//save yx data
extern rt_uint16_t ym_data_buf7[RTDB_CHANL_NUM][RTDB_YM_MAXNUM];//save ym data
extern rt_uint16_t yx_chgdata_buf7[RTDB_YX_CHGMAXNUM][RTDB_YX_MAXNUM];//save yx change data
extern rt_uint16_t yk_data_buf7[RTDB_CHANL_NUM][RTDB_YC_MAXNUM];// save yk data

TMeasure103 measure_send[MEASURE50_NUMBER];

uint8_t rtdbopflg0,rtdbopflg1,rtdbopflg3,rtdbopflg4,rtdbopflg5,rtdbopflg6,rtdbopflg7;
//遥控报文定义
//uint8_t yk_cmd_buf0[10],yk_cmd_buf1[10] ,yk_cmd_buf3[10],yk_cmd_buf4[10],yk_cmd_buf5[10],yk_cmd_buf6[10]  ,yk_cmd_buf7[10]; 


TLINK_INFO linkInfo[LINK_MAX];
// 上位机下发地址与下位机实际地址映射  判断地址是否在配置范围
uint16_t OP_DealRealAddr(uint8_t  serveraddr)
{
	  uint8_t  i,j,okflg = 0;
	  for(i = 0;i <= 7;i++)
	  {
				if(i == 2)
					i++;	  	
			  for(j = 0;j < uartpara[i].devusednum;j++)
			  {
					if(serveraddr == uartpara[i].devaddr[j])
					{
						 i = i<<8;//port num
						
		         if(uart_status[i].uartdevice[j].uart_devonline == 0)//设备不在线
		         {
			           return 0xff;
		         }							
						return (i|j);//返回端口号以及地址映射位置
					}
				}

		}
		
    //if(okflg != 1)//超过地址范围
		//{
			  return 0xff;
		//}	
}






/*
portindex//串口设备端口号
byteINF//遥控点信息体地址
devaddr//从机地址
nDCC//遥控信息控制码 双点
ykdatatype////遥控数据类型 遥控刀闸 0   复归  1   遥控压板 2  修改定值 3    对时 4  定值区切换 5
ykexecflg//用于遥控刀闸 预发0  执行1 
*/
uint8_t OP_RemoteToRtdb(uint8_t portindex,uint8_t addrpos ,uint8_t devaddr,uint8_t ykdatatype,uint8_t *remotedatabuf,uint8_t datanum)
{
	  

	
    uint8_t INFindex;
	  uint16_t temp;
	  uint16_t i;
	  
	
		if(uart_status[portindex].uartdevice[addrpos].uart_ykflg == 1)//有遥控命令还未执行
		{
			  return FALSE;
		}
		
		uart_status[portindex].uart_rtdbflg++;//记录远控的个数
		switch(portindex)
    {
			  case 0:
            if(uart_status[portindex].uartdevice[addrpos].uart_devonline == 0)//设备异常或不在线
							  return FALSE;
				    for(i = 0;i < datanum;i++)
			      {
							  yk_data_buf0[addrpos][i+3] = remotedatabuf[i];
						}
						yk_data_buf0[addrpos][0] = 0x88;//
						yk_data_buf0[addrpos][1] = i+3;
						yk_data_buf0[addrpos][2] = ykdatatype;
						uart_status[portindex].uartdevice[addrpos].uart_ykflg = 1;
				    break;
		
			  case 1:
            if(uart_status[portindex].uartdevice[addrpos].uart_devonline == 0)//设备异常或不在线
							  return FALSE;
				    for(i = 0;i < datanum;i++)
			      {
							  yk_data_buf1[addrpos][i+3] = remotedatabuf[i];
						}
						yk_data_buf1[addrpos][0] = 0x88;////
						yk_data_buf1[addrpos][1] = i+3;
						yk_data_buf1[addrpos][2] = ykdatatype;
						uart_status[portindex].uartdevice[addrpos].uart_ykflg = 1;
				    break;		
		
			  case 3:
            if(uart_status[portindex].uartdevice[addrpos].uart_devonline == 0)//设备异常或不在线
							  return FALSE;
				    for(i = 0;i < datanum;i++)
			      {
							  yk_data_buf3[addrpos][i+3] = remotedatabuf[i];
						}
						yk_data_buf3[addrpos][0] = 0x88;////
						yk_data_buf3[addrpos][1] = i+3;
						yk_data_buf3[addrpos][2] = ykdatatype;
						uart_status[portindex].uartdevice[addrpos].uart_ykflg = 1;
				    break;		
				 
					case 4:
            if(uart_status[portindex].uartdevice[addrpos].uart_devonline == 0)//设备异常或不在线
							  return FALSE;
				    for(i = 0;i < datanum;i++)
			      {
							  yk_data_buf4[addrpos][i+3] = remotedatabuf[i];
						}
						yk_data_buf4[addrpos][0] = 0x88;////
						yk_data_buf4[addrpos][1] = i+3;
						yk_data_buf4[addrpos][2] = ykdatatype;
						uart_status[portindex].uartdevice[addrpos].uart_ykflg = 1;
				    break;		
		
			  case 5:
            if(uart_status[portindex].uartdevice[addrpos].uart_devonline == 0)//设备异常或不在线
							  return FALSE;
				    for(i = 0;i < datanum;i++)
			      {
							  yk_data_buf5[addrpos][i+3] = remotedatabuf[i];
						}
						yk_data_buf5[addrpos][0] = 0x88;////
						yk_data_buf5[addrpos][1] = i+3;
						yk_data_buf5[addrpos][2] = ykdatatype;
						uart_status[portindex].uartdevice[addrpos].uart_ykflg = 1;
				    break;		
		
			  case 6:
            if(uart_status[portindex].uartdevice[addrpos].uart_devonline == 0)//设备异常或不在线
							  return FALSE;
				    for(i = 0;i < datanum;i++)
			      {
							  yk_data_buf6[addrpos][i+3] = remotedatabuf[i];
						}
						yk_data_buf6[addrpos][0] = 0x88;////
						yk_data_buf6[addrpos][1] = i+3;
						yk_data_buf6[addrpos][2] = ykdatatype;
						uart_status[portindex].uartdevice[addrpos].uart_ykflg = 1;
				    break;		
		
		
			  case 7:
            if(uart_status[portindex].uartdevice[addrpos].uart_devonline == 0)//设备异常或不在线
							  return FALSE;
				    for(i = 0;i < datanum;i++)
			      {
							  yk_data_buf7[addrpos][i+3] = remotedatabuf[i];
						}
						yk_data_buf7[addrpos][0] = 0x88;////
						yk_data_buf7[addrpos][1] = i+3;
						yk_data_buf7[addrpos][2] = ykdatatype;
						uart_status[portindex].uartdevice[addrpos].uart_ykflg = 1;
				    break;	
				default:
					  return FALSE;
            break;					
		
			
		}			
	

}


// 遥测信息初始化
void OP_InitMeasureID(uint8_t nINF)
{
    TMeasure103* pSendMeas;
    uint8_t i,j;

    if(nINF==50)
    {
        pSendMeas=measure_send;
        for(i=0,j=92; i<MEASURE50_NUMBER; i++,j++,pSendMeas++)
        {
            pSendMeas->measID = j;
        }
    }
}

//whs 暂时不用  void get_arm_pst(void);
//总召唤 获取告警信息状态
void OP_GetWarningState(uint8_t* pdata, uint8_t* pnNum)
{
         
	/* get_arm_pst();
    if(*pnNum == 0)
    {
        *pdata++ = gj_bufer[0].inf;
        *pdata = gj_bufer[0].pst+1;//DPI
        *pnNum = GJ_NUM;
    }
    else
    {
        *pdata++ = gj_bufer[GJ_NUM+1 - *pnNum].inf;
        *pdata = gj_bufer[GJ_NUM+1 - *pnNum].pst + 1;//DPI
        *pnNum = *pnNum-1;
    }*///whs 暂时不用
}

//总召唤 获取软压板状态
void OP_GetSoftStrapState(uint8_t* pdata, uint8_t* pnNum)
{

  /*  if(*pnNum == 0)
    {
        *pdata++ = yb_inf[0];         *pdata = yb_buf[0]+1;//DPI
        *pnNum = YB_NUM;
    }
    else
    {
        *pdata++ = yb_inf[YB_NUM+1 - *pnNum];
       *pdata = yb_buf[YB_NUM+1 - *pnNum]+1;//DPI
        *pnNum = *pnNum-1;
    }*/
}




//总召唤 获取所有开入状态
int8_t OP_GetDIState(uint8_t* pdata, uint8_t* pnNum,uint8_t calldeviceaddr)
{
	 
    uint8_t i,temp;
	  uint8_t portindex,deviceindex;
 
	  int8_t m;
	  
	  temp = OP_DealRealAddr(calldeviceaddr);
	  if(temp == 0xff)
			return FALSE;		
    portindex = (temp>>8)&0x0f;
	  deviceindex = temp&0xff;

	  
	  if(portindex == 0)
		{
			 if((yx_chgdata_buf0[deviceindex][0] != 0x55)&&(yx_chgdata_buf0[deviceindex][0] != 0x88))//no yx data
			 {
				 return HAVE_ERROR;
			 }
			 for(i = 2;i < (yx_data_buf0[deviceindex][1]+2);i++)
			 {
				   *pdata++ = yx_data_buf0[deviceindex][i];
				  // *pdata++ = i -2 +148;
				 
			 } 
		 
		}
		else if(portindex == 1)
		{
			 if((yx_chgdata_buf1[deviceindex][0] != 0x55)&&(yx_chgdata_buf1[deviceindex][0] != 0x88))//no yx data
			 {
				 return HAVE_ERROR;
			 }
			 for(i = 2;i < (yx_data_buf1[deviceindex][1]+2);i++)
			 {
				   *pdata++ = yx_data_buf1[deviceindex][i];
				   //*pdata++ = i -2 +148;
				 
			 } 
		 
		}
		else if(portindex == 3)
		{
			 if((yx_chgdata_buf3[deviceindex][0] != 0x55)&&(yx_chgdata_buf3[deviceindex][0] != 0x88))//no yx data
			 {
				 return HAVE_ERROR;
			 }
			 for(i = 2;i < (yx_data_buf3[deviceindex][1]+2);i++)
			 {
				   *pdata++ = yx_data_buf3[deviceindex][i];
				   //*pdata++ = i -2 +148;
				 
			 } 
		 
		}
		else if(portindex == 4)
		{
			 if((yx_chgdata_buf4[deviceindex][0] != 0x55)&&(yx_chgdata_buf4[deviceindex][0] != 0x88))//no yx data
			 {
				 return HAVE_ERROR;
			 }
			 for(i = 2;i < (yx_data_buf4[deviceindex][1]+2);i++)
			 {
				   *pdata++ = yx_data_buf4[deviceindex][i];
				   //*pdata++ = i -2 +148;
				 
			 } 
		 
		}
		else if(portindex == 5)
		{
			 if((yx_chgdata_buf5[deviceindex][0] != 0x55)&&(yx_chgdata_buf5[deviceindex][0] != 0x88))//no yx data
			 {
				 return HAVE_ERROR;
			 }
			 for(i = 2;i < (yx_data_buf5[deviceindex][1]+2);i++)
			 {
				   *pdata++ = yx_data_buf5[deviceindex][i];
				   //*pdata++ = i -2 +148;
				 
			 } 
		 
		}
		else if(portindex == 6)
		{
			 if((yx_chgdata_buf6[deviceindex][0] != 0x55)&&(yx_chgdata_buf6[deviceindex][0] != 0x88))//no yx data
			 {
				 return HAVE_ERROR;
			 }
			 for(i = 2;i < (yx_data_buf6[deviceindex][1]+2);i++)
			 {
				   *pdata++ = yx_data_buf6[deviceindex][i];
				   //*pdata++ = i -2 +148;
				 
			 } 
		 
		}
		else if(portindex == 7)
		{
			 if((yx_chgdata_buf7[deviceindex][0] != 0x55)&&(yx_chgdata_buf7[deviceindex][0] != 0x88))//no yx data
			 {
				 return HAVE_ERROR;
			 }
			 for(i = 2;i < (yx_data_buf7[deviceindex][1]+2);i++)
			 {
				   *pdata++ = yx_data_buf7[deviceindex][i];
				   //*pdata++ = i -2 +148;
				 
			 } 
		 
		}
    *pnNum = i-1;

}
//whs 暂时不用  extern uint8_t  fgb;
// 信号复归，正常复归返回 1
int OP_SignalReset(uint8_t nINF, uint8_t nDCO,uint8_t devaddr)
{
	  uint16_t temp_port,addrpos,temp;
	  uint8_t i = 0;
	  
	  static uint8_t yk_buf[5];
	//ykdatatype////遥控数据类型 遥控刀闸 0   复归  1   遥控压板 2  修改定值 3    对时 4  定值区切换 5
	  temp = OP_DealRealAddr(devaddr);
	  if(temp == 0xff)
			return FALSE;		
    temp_port = (temp>>8)&0xff;
	  addrpos = temp&0xff;
  		
	
		yk_buf[i++] = devaddr;
		yk_buf[i++] = nINF;
		yk_buf[i++] = nDCO;
		yk_buf[i++] = 0x00;//byteFUN;
		
	  if(	OP_RemoteToRtdb( temp_port,addrpos,devaddr,REMOTE_FG_TYPE,yk_buf,i) == TRUE)
		    return  TRUE;
	  else
		    return  FALSE;

}

 uint8_t setqht0;
void save_ope_report(uint8_t nDispID);

// 定值区切换，正常切换返回 1
int OP_MdfySetPointGroup(uint8_t nINF, uint8_t nDCO,uint8_t devaddr)
{
	
	
	  static uint8_t yk_buf[5];
	  uint16_t temp_port,addrpos,temp;
	  uint8_t i = 0;
	  
    if((nINF>=100) && (nINF<=110))
    {
        nINF -= 100;
    }
    else
    	return FALSE;	 
    
     

	//ykdatatype////遥控数据类型 遥控刀闸 0   复归  1   遥控压板 2  修改定值 3    对时 4  定值区切换 5
	  temp = OP_DealRealAddr(devaddr);
	  if(temp == 0xff)
			return FALSE;		
    temp_port = (temp>>8)&0xff;
	  addrpos = temp&0xff;
  		
	
		yk_buf[i++] = devaddr;
		yk_buf[i++] = nINF;
		yk_buf[i++] = nDCO;
		
		
	  if(	OP_RemoteToRtdb( temp_port,addrpos,devaddr,REMOTE_QHDZ_TYPE,yk_buf,i) == TRUE)
		    return  TRUE;
	  else
		    return  FALSE;
		    
		    

}

void save_general_report(uint8_t nASDU,uint8_t nDispID, uint8_t nDPI, uint8_t nCOT);

extern uint8_t  yb_buf[100];

// 软压板投退，正常投退返回 1
uint8_t OP_MdfySoftStrap(uint8_t nINF, uint8_t nDCO,uint8_t devaddr)
{
	
	  static uint8_t yk_buf[5];
	  uint16_t temp_port,addrpos,temp;
	  uint8_t i = 0;
	
	//ykdatatype////遥控数据类型 遥控刀闸 0   复归  1   遥控压板 2  修改定值 3    对时 4  定值区切换 5
	  temp = OP_DealRealAddr(devaddr);
	  if(temp == 0xff)
			return FALSE;		
    temp_port = (temp>>8)&0xff;
	  addrpos = temp&0xff;
  		
	
		yk_buf[i++] = devaddr;
		yk_buf[i++] = nINF;
		yk_buf[i++] = nDCO;
		
		
	  if(	OP_RemoteToRtdb( temp_port,addrpos,devaddr,REMOTE_YB_TYPE,yk_buf,i) == TRUE)
		    return  TRUE;
	  else
		    return  FALSE;
}


// 遥控选择，选择成功返回 1
int OP_RemoteCtrlSelt(uint8_t byteFUN, uint8_t byteINF, uint8_t nDCC,uint8_t devaddr)
{
	  uint16_t temp_port,addrpos,temp;
	  uint8_t i = 0;
	  
	  static uint8_t yk_buf[5];
	//ykdatatype////遥控数据类型 遥控刀闸 0   复归  1   遥控压板 2  修改定值 3    对时 4  定值区切换 5
	  temp = OP_DealRealAddr(devaddr);
	  if(temp == 0xff)
			return FALSE;		
    temp_port = (temp>>8)&0xff;
	  addrpos = temp&0xff;
  		
	
		yk_buf[i++] = devaddr;
		yk_buf[i++] = byteINF;
		yk_buf[i++] = nDCC;
		yk_buf[i++] = byteFUN;
		
	  if(	OP_RemoteToRtdb( temp_port,addrpos,devaddr,REMOTE_DL_TYPE,yk_buf,i) == TRUE)
		    return  TRUE;
	  else
		    return  FALSE;

}

// 遥控执行，执行成功返回 1
int OP_RemoteCtrlExec(uint8_t byteFUN, uint8_t byteINF, uint8_t nDCC, uint8_t LastnDCC,uint8_t devaddr)
{

	  uint16_t temp_port,addrpos,temp;
	  uint8_t i = 0;
	  
	  static uint8_t yk_buf[5];
	//ykdatatype////遥控数据类型 遥控刀闸 0   复归  1   遥控压板 2  修改定值 3    对时 4  定值区切换 5
	  temp = OP_DealRealAddr(devaddr);
	  if(temp == 0xff)
			return FALSE;		
    temp_port = (temp>>8)&0xff;
	  addrpos = temp&0xff;
  		
	
		yk_buf[i++] = devaddr;
		yk_buf[i++] = byteINF;
		yk_buf[i++] = nDCC;
		yk_buf[i++] = byteFUN;
		
	  if(	OP_RemoteToRtdb( temp_port,addrpos,devaddr,REMOTE_DL_TYPE,yk_buf,i) == TRUE)
		    return  TRUE;
	  else
		    return  FALSE;
		
		

}

//whs 暂时不用  extern uint16_t dz_buf[8][DZ_NUM];
//whs 暂时不用  extern const TSET_PARA_STRUCT set_para[];
// 定值查询组织数据
uint8_t OP_GetSetPointValue(uint8_t nSetZone, uint8_t* pSendMsg)
{
  /*  uint8_t index,*pSetValue;

    if(nSetZone>7)
    {
        return 0;
    }

 //whs 暂时不用     pSetValue = (uint8_t*)&dz_buf[nSetZone][1];
    index = 0;
    while(index < DZ_NUM-2)
    {
        *pSendMsg++ = *pSetValue++;//定值低字节
        *pSendMsg++ = *pSetValue++;//定值高字节
        *pSendMsg++ = set_para[index++].scale;//定值符号字节
    }
    return index;*/
}

uint16_t  crc16(uint8_t*, uint8_t);
//whs 暂时不用  extern uint8_t value_change_flag;


//执行修改定值--保存定值、若是当前区则更新保护定值，保存报告 根据104需要修改这个函数
uint8_t OP_MdfySetPointValue(uint8_t nSetZone, uint8_t* pNewSetValue, uint8_t SetNum, uint8_t SetStart)
{
 /*   uint8_t SetCount,*pSetValue;

    if(nSetZone>7)
    {
        return 0;
    }

    SetCount = SetNum;
    dz_buf[nSetZone][0] = 0xaa55;
    pSetValue = (uint8_t*)&dz_buf[nSetZone][SetStart];
    while(SetCount--)
    {
        *pSetValue++ = *pNewSetValue++;//定值低字节
        *pSetValue++ = *pNewSetValue++;//定值高字节
        pNewSetValue++;//定值符号字节
    }
    dz_buf[nSetZone][DZ_NUM-1] = crc16((uint8_t*)&dz_buf[nSetZone][0], 2*DZ_NUM - 2);
    rt_mb_send(&mb, TYPE_SET_SET|nSetZone);

    if(nSetZone == setqht0)
    {
        //protect_initialize();
//        get_dz(setqht0);//读取新区号定值
//whs 暂时不用  		get_pro_data();
    }
 //whs 暂时不用     save_ope_report(15);
//whs 暂时不用      value_change_flag = 1;*/
	return 1;
}

// 获取遥测数据，返回遥测个数
uint8_t OP_GetCurrentMeasure103(TMeasure103** ppSendMeas)
{
    *ppSendMeas=measure_send;
    return MEASURE50_NUMBER;
}


void* OP_GetSOEQHead(uint8_t SOEType)
{
    void *pSOEQHead;
    switch(SOEType)
    {
    case SOE_ACTION:
        pSOEQHead = protect_soe;
        break;
    case SOE_WARNING:
        pSOEQHead = alarm_soe;
        break;
    case SOE_DIEVENT:
        pSOEQHead = change_soe;
        break;
    /*case SOE_GENERAL:
        pSOEQHead = general_report;
        break;
    case SOE_WAVE:
        pSOEQHead = DisturbInRam;
        break;*/
    default:
        pSOEQHead = RT_NULL;
        break;
    }
    return pSOEQHead;//whs 暂时不用      
}

void* OP_GetSOEQRear(uint8_t SOEType)
{
    void *pSOEQRear;
    switch(SOEType)
    {
    case SOE_ACTION:
        pSOEQRear = &protect_soe[REPORT_SUM-1];
        break;
    case SOE_WARNING:
        pSOEQRear = &alarm_soe[REPORT_SUM-1];
        break;
    case SOE_DIEVENT:
        pSOEQRear = &change_soe[REPORT_SUM-1];
        break;
    /*case SOE_GENERAL:
        pSOEQRear = &general_report[REPORT_SUM-1];
        break;
    case SOE_WAVE:
        pSOEQRear = &DisturbInRam[WAVE_INFORMATION_SUM-1];
        break;*/
    default:
        pSOEQRear = RT_NULL;
        break;
    }
    return pSOEQRear;
}

// 获取新产生的SOE类型及数量,现在采用每次送一个SOE,所以*pSOECount=1;
void OP_GetSOEQueueSendType(uint8_t LinkSocket, int8_t *pSOEType, uint8_t *pSOECount)
{
    uint8_t nindex;

    nindex = linkInfo[LinkSocket].ActionReadIndex;
    while( nindex != (protect_soe_position&0x7f) )
    {
        if( protect_soe[nindex].nValid & 0x01)
        {
            *pSOECount = 1;
            *pSOEType = SOE_ACTION;
            linkInfo[LinkSocket].ActionReadIndex = nindex;
            return;
        }
        nindex++;
        if( nindex >= REPORT_SUM)
        {
            nindex = 0;
        }
    }
    nindex = linkInfo[LinkSocket].WarningReadIndex;
    while( nindex != (alarm_soe_position&0x7f) )
    {
        if( alarm_soe[nindex].nValid & 0x01)
        {
            *pSOECount = 1;
            *pSOEType = SOE_WARNING;
            linkInfo[LinkSocket].WarningReadIndex = nindex;
            return;
        }
        nindex++;
        if( nindex >= REPORT_SUM)
        {
            nindex = 0;
        }
    }

    nindex = linkInfo[LinkSocket].DInputReadIndex;
    while( nindex != (change_soe_position&0x7f) )
    {
        if( change_soe[nindex].nValid & 0x01)
        {
            *pSOECount = 1;
            *pSOEType = SOE_DIEVENT;
            linkInfo[LinkSocket].DInputReadIndex = nindex;
            return;
        }
        nindex++;
        if( nindex >= REPORT_SUM)
        {
            nindex = 0;
        }

    }


   /* nindex = linkInfo[LinkSocket].GeneralReadIndex;
    while( nindex != (general_report_position&0x7f) )
    {
        if( general_report[nindex].nValid & 0x01)
        {
            *pSOECount = 1;
            *pSOEType = SOE_GENERAL;
            linkInfo[LinkSocket].GeneralReadIndex = nindex;
            return;
        }
        nindex++;
        if( nindex >= REPORT_SUM)
        {
            nindex = 0;
        }
    }

    nindex = linkInfo[LinkSocket].WaveReadIndex;
    if( nindex != (disturb_position&0x7f) )
    {
        *pSOECount = WAVE_INFORMATION_SUM;
        *pSOEType = SOE_WAVE;
        linkInfo[LinkSocket].WaveReadIndex = (disturb_position&0x7f);    //录波扰动表不需要记忆，因为有总召唤
    }*/
    return;//whs 暂时不用      
}


int OP_GetpSOEQueue(uint8_t LinkSocket, uint8_t SOEType, void** ppSOEQueue)
{
    uint8_t index;
	int	nError = OK;

    switch(SOEType)
    {
    case SOE_ACTION:
        index = linkInfo[LinkSocket].ActionReadIndex;
        if( protect_soe[index].nValid & 0x01)
        {
            *ppSOEQueue = &protect_soe[index];
        }
        break;
    case SOE_WARNING:
        index = linkInfo[LinkSocket].WarningReadIndex;
        if( alarm_soe[index].nValid & 0x01)
        {
            *ppSOEQueue = &alarm_soe[index];
        }
        break;
    case SOE_DIEVENT:
        index = linkInfo[LinkSocket].DInputReadIndex;
        if( change_soe[index].nValid & 0x01)
        {
            *ppSOEQueue = &change_soe[index];
        }
        break;
    /*case SOE_GENERAL:
        index = linkInfo[LinkSocket].GeneralReadIndex;
        if( general_report[index].nValid & 0x01)
        {
            *ppSOEQueue = &general_report[index];
        }
        break;
    case SOE_WAVE:
        index = linkInfo[LinkSocket].WaveReadIndex;
        *ppSOEQueue = &DisturbInRam[index];
        break;*/
    default:
        nError = ERROR;
        break;
    }
    return nError;   
}

int OP_SendSomeSOE(uint8_t LinkSocket, uint8_t SOEType, uint8_t SOECount)
{
    int nError = OK;
	switch(SOEType)
    {
    case SOE_ACTION:
        linkInfo[LinkSocket].ActionReadIndex++;
        if( linkInfo[LinkSocket].ActionReadIndex >= REPORT_SUM )
        {
            linkInfo[LinkSocket].ActionReadIndex = 0;
        }
        break;
    case SOE_WARNING:
        linkInfo[LinkSocket].WarningReadIndex++;
        if( linkInfo[LinkSocket].WarningReadIndex >= REPORT_SUM )
        {
            linkInfo[LinkSocket].WarningReadIndex = 0;
        }
        break;
    case SOE_DIEVENT:
        linkInfo[LinkSocket].DInputReadIndex += SOECount;
        if( linkInfo[LinkSocket].DInputReadIndex >= REPORT_SUM )
        {
            linkInfo[LinkSocket].DInputReadIndex -= REPORT_SUM;
        }
        break;
  /*     case SOE_WAVE:
        linkInfo[LinkSocket].WaveReadIndex += SOECount;
        if( linkInfo[LinkSocket].WaveReadIndex >= WAVE_INFORMATION_SUM )
        {
            linkInfo[LinkSocket].WaveReadIndex -= 0;
        }
        break;*/
    default:
        nError = ERROR;
        break;
    }
    return nError;
}

//查询报告--有没有对应的报告
TSOE_ACTION_STRUCT* OP_GetAppointedSOE(uint8_t FaultRptNumber)
{
    TSOE_ACTION_STRUCT *pAppointedSOE = RT_NULL;
    uint8_t AppointedIndex;

    AppointedIndex = (protect_soe_position&0x80)? REPORT_SUM:protect_soe_position;
    if(FaultRptNumber < AppointedIndex)
    {
        AppointedIndex =( ((protect_soe_position&0x7f) + REPORT_SUM - FaultRptNumber - 1) % REPORT_SUM );
        if(protect_soe[AppointedIndex].nValid & 0x01)
        {
            pAppointedSOE = &protect_soe[AppointedIndex];
        }
    }
    return pAppointedSOE;//whs 暂时不用      
}

/*TDisturbData* OP_SearchDisturb(uint16_t nFAN)
{
    TDisturbData* pDisturbTrans;
	pDisturbTrans = &DisturbInRam[disturb_position&0x7f];
	do
	{
		if (--pDisturbTrans < DisturbInRam)
		{
			pDisturbTrans = &DisturbInRam[WAVE_INFORMATION_SUM - 1];
		}

		if (pDisturbTrans->nFAN == nFAN)
		{
			return pDisturbTrans;
		}
	}
	while (pDisturbTrans != &DisturbInRam[disturb_position&0x7f]);

	return RT_NULL;
}*///whs 暂时不用      


extern uint8_t time_write_flg;
uint8_t OP_ModifyTime(TDateTime* tTime,int32_t deviceaddr)
{	
    static uint8_t yk_buf[5];
	  uint16_t temp_port,addrpos,temp;
	  uint8_t i = 0;
	  TIME = *tTime;

	//ykdatatype////遥控数据类型 遥控刀闸 0   复归  1   遥控压板 2  修改定值 3    对时 4  定值区切换 5
	  temp = OP_DealRealAddr(deviceaddr);
	  if(temp == 0xff)
			return FALSE;		
    temp_port = (temp>>8)&0xff;
	  addrpos = temp&0xff;
  		
	
		yk_buf[i++] = deviceaddr;
		yk_buf[i++] = 0x00;
		yk_buf[i++] = 0x00;
		yk_buf[i++] = 0x00;
    time_write_flg = 1; //写时钟		
	  if(	OP_RemoteToRtdb( temp_port,addrpos,deviceaddr,REMOTE_TIME_TYPE,yk_buf,i) == TRUE)
		    return  TRUE;
	  else
		    return  FALSE;	

}
//int32_t kw1 = 111,kw2 = 222,kv1 = 333,kv2 = 444;
//68 21 10 00 02 00 0F 84 25 00 02 03 01 64 00 6F 00 00 00 01 DE 00 00 00 02 4D 01 00 00 03 BC 01 00 00 04
uint8_t OP_GetPowerEnergy103(uint8_t nDET, uint8_t *nInf, TMeterMsg **ppMsg,uint8_t ymdevaddr)//获取电度量
{
    static TMeterMsg MeterValue[PULSENUM];
    rt_int32_t ymdata[20];
	  uint16_t temp_port,addrpos,temp;
	  uint8_t num,i;
	  
	  temp = OP_DealRealAddr(ymdevaddr);
	  if(temp_port == 0xff)
			return FALSE;		
    temp_port = (temp>>8)&0xff;
	  addrpos = temp&0xff;


    switch((temp_port)&0xff)
    {
			case 0://uart0
				  if(ym_data_buf0[addrpos][0] != 0x7f)//无数据
						return FALSE;
          num  = ym_data_buf0[addrpos][2];
					num = num>>1;
          for(i=0;i<num;i++)
          {
						ymdata[i] = ym_data_buf0[addrpos][3+2*i]|( ym_data_buf0[addrpos][4+2*i]<<16);
					}  	 					
			break;
					
			case 1:
				  if(ym_data_buf1[addrpos][0] != 0x7f)//无数据
						return FALSE;
          num  = ym_data_buf1[addrpos][2];
					num = num>>1;
          for(i=0;i<num;i++)
          {
						ymdata[i] = ym_data_buf1[addrpos][3+2*i]|( ym_data_buf1[addrpos][4+2*i]<<16);
					}  	 					
			break;					
					
					
			case 3:
				  if(ym_data_buf3[addrpos][0] != 0x7f)//无数据
						return FALSE;
          num  = ym_data_buf3[addrpos][2];
					num = num>>1;
          for(i=0;i<num;i++)
          {
						ymdata[i] = ym_data_buf3[addrpos][3+2*i]|( ym_data_buf3[addrpos][4+2*i]<<16);
					}  	 					
			break;		



			case 4:
				  if(ym_data_buf4[addrpos][0] != 0x7f)//无数据
						return FALSE;
          num  = ym_data_buf4[addrpos][2];
					num = num>>1;
          for(i=0;i<num;i++)
          {
						ymdata[i] = ym_data_buf4[addrpos][3+2*i]|( ym_data_buf4[addrpos][4+2*i]<<16);
					}  	 					
			break;	


			case 5:
				  if(ym_data_buf5[addrpos][0] != 0x7f)//无数据
						return FALSE;
          num  = ym_data_buf5[addrpos][2];
					num = num>>1;
          for(i=0;i<num;i++)
          {
						ymdata[i] = ym_data_buf5[addrpos][3+2*i]|( ym_data_buf5[addrpos][4+2*i]<<16);
					}  	 					
			break;					
					
					
			case 6:
				  if(ym_data_buf6[addrpos][0] != 0x7f)//无数据
						return FALSE;
          num  = ym_data_buf6[addrpos][2];
					num = num>>1;
          for(i=0;i<num;i++)
          {
						ymdata[i] = ym_data_buf6[addrpos][3+2*i]|( ym_data_buf6[addrpos][4+2*i]<<16);
					}  	 					
			break;		



			case 7:
				  if(ym_data_buf7[addrpos][0] != 0x7f)//无数据
						return FALSE;
          num  = ym_data_buf7[addrpos][2];
					num = num>>1;
          for(i=0;i<num;i++)
          {
						ymdata[i] = ym_data_buf7[addrpos][3+2*i]|( ym_data_buf7[addrpos][4+2*i]<<16);
					}  	 					
			break;	
					
					
	    default://非法地址
				return FALSE;
      break;				
					
		}			
		
		
	switch(nDET&_FRZ)
	{
		case 0://无冻结无复位--如果请求电度量就上送上次的冻结量
		 	break;

		case 0x40://冻结不复位--常用功能
			
		    for(i =0;i<num;i++)
		    {
			      MeterValue[i].meterVal = ymdata[i];
            MeterValue[i].SequenceNum = 0x41 + i;
		    }
		break;

		/*case 0x80://冻结带复位
		    if(MeterValue[0].meterVal == kw1)
            {
                MeterValue[0].SequenceNum = 0x41;
            }
            else
            {
                MeterValue[0].meterVal = kw1;
                MeterValue[0].SequenceNum = 0x01;
            }
		    if(MeterValue[1].meterVal == kw2)
            {
                MeterValue[1].SequenceNum = 0x42;
            }
            else
            {
                MeterValue[1].meterVal = kw2;
                MeterValue[1].SequenceNum = 0x02;
            }
		    if(MeterValue[2].meterVal == kv1)
            {
                MeterValue[2].SequenceNum = 0x43;
            }
            else
            {
                MeterValue[2].meterVal = kv1;
                MeterValue[2].SequenceNum = 0x03;
            }
		    if(MeterValue[3].meterVal == kv2)
            {
                MeterValue[3].SequenceNum = 0x44;
            }
            else
            {
                MeterValue[3].meterVal = kv2;
                MeterValue[3].SequenceNum = 0x04;
            }
            kw1 = 0;
            kw2 = 0;
            kv1 = 0;
            kv2 = 0;
		 	break;

		case 0xc0://无冻结只复位
            kw1 = 0;
            kw2 = 0;
            kv1 = 0;
            kv2 = 0;
			break;*/
	}

	//if((nDET&_REQ) == 5)
	if((nDET&_REQ) != 0)
    {
        *nInf = 6;//第一个脉冲的信息序号=6
        *ppMsg = MeterValue;
        return num;//PULSENUM;
    }

    return num;
}


int OP_DetachLink(int socketfd, int clientAddr)
{

    if(socketfd >= LINK_MAX)
        return 0;
    linkInfo[socketfd].IPAddr = 0L;
	return 1;
}

// 新连接建立时，初始化SOE存储位置
int OP_EstablishLink(int socketfd, int clientAddr)
{

    if(socketfd >= LINK_MAX)
        return 0;

    linkInfo[socketfd].IPAddr = clientAddr;
   linkInfo[socketfd].ActionReadIndex = (protect_soe_position&0x7f);
    linkInfo[socketfd].WarningReadIndex = (alarm_soe_position&0x7f);
    linkInfo[socketfd].DInputReadIndex = (change_soe_position&0x7f);
 /*    linkInfo[socketfd].GeneralReadIndex = (general_report_position&0x7f);
    linkInfo[socketfd].WaveReadIndex = (disturb_position&0x7f);*///whs 暂时不用  

    return 1;
}

// 初始化所有SOE存储位置
void OP_InitRecordIndex(void)
{
   uint8_t i;

    for(i=0; i<LINK_MAX; i++)
    {
        linkInfo[i].ActionReadIndex = (protect_soe_position&0x7f);
        linkInfo[i].WarningReadIndex = (alarm_soe_position&0x7f);
        linkInfo[i].DInputReadIndex = (change_soe_position&0x7f);
        //linkInfo[i].GeneralReadIndex = (general_report_position&0x7f);
        //linkInfo[i].WaveReadIndex = (disturb_position&0x7f);
    }

    /* for(i=0; i<WAVE_INFORMATION_SUM; i++)
    {
        DisturbInRam[i].nValid = 0;
    }
    return;*///whs 暂时不用      
}

uint8_t OP_GetThresholdSendMeas(int LinkSocket, uint8_t nFun)
{
    TMeasure103* pSendMeas=measure_send;
    uint8_t i;

    for(i=0; i<MEASURE50_NUMBER; i++,pSendMeas++)
    {
        if(pSendMeas->bAutoSendFlg&(0x01<<LinkSocket))
        {
            return TRUE;
        }
    }
    return FALSE;
}

/*TWaveDescripInfo* OPWAVE_GetCommWaveDescInfo(int a, uint8_t b)
{
	  return RT_NULL;
}

TWaveDescripInfo* OPWAVE_GetWaveDescInfo(uint8_t a)
{
	 return RT_NULL;
}*///whs 暂时不用      

TPORT_PARAM_STRUCT* GetLinkPortParam( int a)
{
	 return RT_NULL;
}

extern uint8_t zzaddr;
int GetARMSysConfigValue(uint8_t ConfigType)
{
    int ConfigData;
    switch(ConfigType)
    {
    case DI_GET_DEV_ADDR: //device addr
        ConfigData = 1;//zzaddr;//
        break;

    case ID_GET_CYCLE_TIMES: //measrue value send cycle time
        ConfigData = 10;
        break;

    case ID_GET_CURRENT_ZONE: //当前定值区
        ConfigData = setqht0;
        break;
    default:
        ConfigData = 0;
    }
    return ConfigData;
}

uint8_t OPWAVE_StartWave(void)
{
	 return 0;
}
void Process103Meas(void)
{

}



void OP_GetDeviceStatus103(uint8_t* pInf, uint8_t** pVal, uint8_t* nNum)//inf=0xc0;
{

    *pInf = 0x0c;
    **pVal = 2;
    *nNum = 1;
}






/********************************************************************************/
/*函数说明                                                                      */
/*    数据库处理程序                                                            */
/*参数说明                                                                      */
/*                                                                     */
/*                                                                */
/*返回值                                                                        */
/*    uartport_num                                                */
/*版本号及修改日期                                                              */
/*    1.00                      2017-09-01                                      */
/********************************************************************************/
int16_t yc_message_buf[130];
int8_t yx_message_buf[130],yx_msgchg_buf[130];
uint8_t index_trans_yc = 0,index_trans_yx,yxcount,index_trans_yxchg;
uint8_t host_call_yx;//查询类型  ask_data,
extern void datacopy(uint16_t *p1,uint16_t *p2,uint8_t num);
void rtdbto104(uint8_t ask_data)
{
    


}



/********************************************************************************/
/*函数说明                                                                      */
/*    数据库遥测数据处理程序                      */
/*                                                                              */
/*参数说明                                                                      */
/*循环读取遥测，按照104格式转化后传递给以太网发送程序                           */

/*返回值              null                                                          */ 
/*版本号及修改日期                                                              */
/*    1.00                      2017-09-01                                      */
/********************************************************************************/
uint8_t readyccount ;
uint8_t uartportnum,uarttransycflg;
uint8_t uart0yccount,uart1yccount,uart3yccount,uart4yccount,uart5yccount,uart6yccount,uart7yccount;
uint8_t uart0devicenum,uart1devicenum,uart3devicenum,uart4devicenum,uart5devicenum,uart6devicenum,uart7devicenum;
uint8_t portusedbuf[7]= {0,1,3,4,5,6,7},portusedflg;

uint8_t porttransycindex = 0,ycaddrindex = 0;//0-7
void  OP_GetMeasureValue(void)
{
    uint8_t  i;
    
    //TMeasure103* pSendMeas;
	
	 /* uart0devicenum = 8;
	  uarttransycflg = portusedbuf[portusedflg]-1;
    if(uarttransycflg > 7 )
		uarttransycflg = 0;*/
	
    //按照串口顺序查询遥测值，依次查询各个地址装置，没有数据或数据无效则跳过
    
		switch(porttransycindex)
    {
			  case 0://read port 0

				    case0add:
			      if(yc_data_buf0[ycaddrindex][0] != 0x7f)//no data
            {
	    			    ycaddrindex++;
	    			    if(ycaddrindex >= uartpara[0].devusednum)
	    				  { 
	    				      ycaddrindex = 0;
	    						  porttransycindex = 1;
	    				  }
	    					else
    							goto case0add;
	    		  }	
						
	    			if(yc_data_buf0[ycaddrindex][0] == 0x7f)
	    		  {
	    			     yc_message_buf[0] = yc_data_buf0[ycaddrindex][1];// device addr
	    			     yc_message_buf[1] = yc_data_buf0[ycaddrindex][2];// device data number
	    			     for(i=0;i < yc_data_buf0[ycaddrindex][2];i++)
			    	     {
				             yc_message_buf[i+2] = yc_data_buf0[ycaddrindex][i+3];//read the yc data to trans buffer
				         }	
								 yc_data_buf0[ycaddrindex][0] = 0x00;
	    			     ycaddrindex++;
			    	     if(ycaddrindex >= uartpara[0].devusednum)
					       { 
					           ycaddrindex = 0;
							       porttransycindex = 1;
					       }
								 
								 break;
			      }						
				
			  case 1://

				    case1add:
			      if(yc_data_buf1[ycaddrindex][0] != 0x7f)//no data
            {
	    			    ycaddrindex++;
	    			    if(ycaddrindex >= uartpara[1].devusednum)
	    				  { 
	    				      ycaddrindex = 0;
	    						  porttransycindex = 3;
	    				  }
	    					else
    							goto case1add;
	    		  }	
						
	    			if(yc_data_buf1[ycaddrindex][0] == 0x7f)
	    		  {
	    			     yc_message_buf[0] = yc_data_buf1[ycaddrindex][1];// device addr
	    			     yc_message_buf[1] = yc_data_buf1[ycaddrindex][2];// device data number
	    			     for(i=0;i < yc_data_buf1[ycaddrindex][2];i++)
			    	     {
				             yc_message_buf[i+2] = yc_data_buf1[ycaddrindex][i+3];//read the yc data to trans buffer
				         }
								 yc_data_buf1[ycaddrindex][0] = 0x00;
	    			     ycaddrindex++;
			    	     if(ycaddrindex >= uartpara[1].devusednum)
					       { 
					           ycaddrindex = 0;
							       porttransycindex = 3;
					       }
								 
								 break;
			      }	
			

			  case 3://

				    case3add:
			      if(yc_data_buf3[ycaddrindex][0] != 0x7f)//no data
            {
	    			    ycaddrindex++;
	    			    if(ycaddrindex >= uartpara[3].devusednum)
	    				  { 
	    				      ycaddrindex = 0;
	    						  porttransycindex = 4;
	    				  }
	    					else
    							goto case3add;
	    		  }	
						
	    			if(yc_data_buf3[ycaddrindex][0] == 0x7f)
	    		  {
	    			     yc_message_buf[0] = yc_data_buf3[ycaddrindex][1];// device addr
	    			     yc_message_buf[1] = yc_data_buf3[ycaddrindex][2];// device data number
	    			     for(i=0;i < yc_data_buf3[ycaddrindex][2];i++)
			    	     {
				             yc_message_buf[i+2] = yc_data_buf3[ycaddrindex][i+3];//read the yc data to trans buffer
				         }	
								 yc_data_buf3[ycaddrindex][0] = 0x00;
	    			     ycaddrindex++;
			    	     if(ycaddrindex >= uartpara[3].devusednum)
					       { 
					           ycaddrindex = 0;
							       porttransycindex = 4;
					       }
								 
								 break;
			      }	

			  case 4://

				    case4add:
			      if(yc_data_buf4[ycaddrindex][0] != 0x7f)//no data
            {
	    			    ycaddrindex++;
	    			    if(ycaddrindex >= uartpara[4].devusednum)
	    				  { 
	    				      ycaddrindex = 0;
	    						  porttransycindex = 5;
	    				  }
	    					else
    							goto case4add;
	    		  }	
						
	    			if(yc_data_buf4[ycaddrindex][0] == 0x7f)
	    		  {
	    			     yc_message_buf[0] = yc_data_buf4[ycaddrindex][1];// device addr
	    			     yc_message_buf[1] = yc_data_buf4[ycaddrindex][2];// device data number
	    			     for(i=0;i < yc_data_buf4[ycaddrindex][2];i++)
			    	     {
				             yc_message_buf[i+2] = yc_data_buf4[ycaddrindex][i+3];//read the yc data to trans buffer
				         }	
								 yc_data_buf4[ycaddrindex][0] = 0x00;
	    			     ycaddrindex++;
			    	     if(ycaddrindex >= uartpara[4].devusednum)
					       { 
					           ycaddrindex = 0;
							       porttransycindex = 5;
					       }
								 
								 break;
			      }

			  case 5://

				    case5add:
			      if(yc_data_buf5[ycaddrindex][0] != 0x7f)//no data
            {
	    			    ycaddrindex++;
	    			    if(ycaddrindex >= uartpara[5].devusednum)
	    				  { 
	    				      ycaddrindex = 0;
	    						  porttransycindex = 6;
	    				  }
	    					else
    							goto case5add;
	    		  }	
						
	    			if(yc_data_buf5[ycaddrindex][0] == 0x7f)
	    		  {
	    			     yc_message_buf[0] = yc_data_buf5[ycaddrindex][1];// device addr
	    			     yc_message_buf[1] = yc_data_buf5[ycaddrindex][2];// device data number
	    			     for(i=0;i < yc_data_buf5[ycaddrindex][2];i++)
			    	     {
				             yc_message_buf[i+2] = yc_data_buf5[ycaddrindex][i+3];//read the yc data to trans buffer
				         }	
								 yc_data_buf5[ycaddrindex][0] = 0x00;
	    			     ycaddrindex++;
			    	     if(ycaddrindex >= uartpara[5].devusednum)
					       { 
					           ycaddrindex = 0;
							       porttransycindex = 6;
					       }
								 
								 break;
			      }		

			  case 6://

				    case6add:
			      if(yc_data_buf6[ycaddrindex][0] != 0x7f)//no data
            {
	    			    ycaddrindex++;
	    			    if(ycaddrindex >= uartpara[6].devusednum)
	    				  { 
	    				      ycaddrindex = 0;
	    						  porttransycindex = 7;
	    				  }
	    					else
    							goto case6add;
	    		  }	
						
	    			if(yc_data_buf6[ycaddrindex][0] == 0x7f)
	    		  {
	    			     yc_message_buf[0] = yc_data_buf6[ycaddrindex][1];// device addr
	    			     yc_message_buf[1] = yc_data_buf6[ycaddrindex][2];// device data number
	    			     for(i=0;i < yc_data_buf6[ycaddrindex][2];i++)
			    	     {
				             yc_message_buf[i+2] = yc_data_buf6[ycaddrindex][i+3];//read the yc data to trans buffer
				         }
								 yc_data_buf6[ycaddrindex][0] = 0x00;
	    			     ycaddrindex++;
			    	     if(ycaddrindex >= uartpara[6].devusednum)
					       { 
					           ycaddrindex = 0;
							       porttransycindex = 7;
					       }
								 
								 break;
			      }	


			  case 7://

				    case7add:
			      if(yc_data_buf7[ycaddrindex][0] != 0x7f)//no data
            {
	    			    ycaddrindex++;
	    			    if(ycaddrindex >= uartpara[7].devusednum)
	    				  { 
	    				      ycaddrindex = 0;
	    						  porttransycindex = 0;
	    				  }
	    					else
    							goto case7add;
	    		  }	
						
	    			if(yc_data_buf7[ycaddrindex][0] == 0x7f)
	    		  {
	    			     yc_message_buf[0] = yc_data_buf7[ycaddrindex][1];// device addr
	    			     yc_message_buf[1] = yc_data_buf7[ycaddrindex][2];// device data number
	    			     for(i=0;i < yc_data_buf7[ycaddrindex][2];i++)
			    	     {
				             yc_message_buf[i+2] = yc_data_buf7[ycaddrindex][i+3];//read the yc data to trans buffer
				         }	
								 yc_data_buf7[ycaddrindex][0] = 0x00;
	    			     ycaddrindex++;
			    	     if(ycaddrindex >= uartpara[7].devusednum)
					       { 
					           ycaddrindex = 0;
							       porttransycindex = 0;
					       }
								 
								 break;
			      }	


						
		}
    
		
	
   /* uarttransycflg++;	
		if(uarttransycflg == 2)
		{
			uarttransycflg = 3;
		}
		if(uarttransycflg > 7)
			uarttransycflg = 0;*/

    
    
    
}

/********************************************************************************/
/*函数说明                                                                      */
/*    数据库数据处理程序                      */
/*                                                                              */
/*参数说明                                                                      */
/*                                          */
/*串口读取 数据后放入数据缓冲区，对遥测数据定时上送*/
/*遥信数据 依靠总召或者突变上送*/
/*遥控数据 以太网接收后判断地址所在位置存入对应串口数据缓冲区，同时置发送标志，*/

/*返回值              null                                                          */ 
/*版本号及修改日期                                                              */
/*    1.00                      2017-09-01                                      */
/********************************************************************************/
uint8_t transyc_rjsz,ykdataflg;
void ope_rtdb(void)
{
	  if(ykdataflg)//有控制或总召数据
	  {
	  	  
	  }	
	  else
	  {
	      
	  }
    
    
    
    
}











/*end*/
