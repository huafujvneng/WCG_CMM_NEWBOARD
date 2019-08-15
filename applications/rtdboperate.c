/**************************************************************************
Copyright (C), 2012, XJ ELECTRIC Co., LTD.
文件名  ：  rtdboperate.c
作者    ：  wanghusen
项目名称：
功能    ：  数据库
创建日期：  2017/9/6
备注    ：
修改记录：
**************************************************************************/
#include "base_define.h"
#include "rtdbdefine.h"

TSOE_ACTION_STRUCT protect_soe[REPORT_SUM];
TSOE_WARNING_STRUCT alarm_soe[REPORT_SUM];
TSOE_DIEVENT_STRUCT change_soe[REPORT_SUM];
uint8_t  protect_soe_position, change_soe_position, alarm_soe_position, operate_soe_position;


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

extern struct  UART_CONF uartpara[UARTMAXNUM];
extern TDateTime TIME;
//报告采用虚拟方式存在缓冲区
/*********************************************************/
/*****    动作报告存入RAM    *****************************/
/*********************************************************/
void save_pro_report(
	uint8_t nPortID, uint8_t ndeviceaddr, uint8_t pro_soe_inf, uint8_t nDPI, uint16_t nPosTime,
	uint8_t nResultCnt, int32_t* ResultBuf)
{
	uint8_t i;
	uint8_t report_position;
	TSOE_ACTION_STRUCT* pSOE;
	pSOE = protect_soe;

	report_position = protect_soe_position & 0x7f;
	if (report_position < 99)
		protect_soe_position++;
	else
		protect_soe_position = 0x80;


	pSOE[report_position].nValid = 1;
	pSOE[report_position].ndeviceaddr = ndeviceaddr;
	pSOE[report_position].nPortID = nPortID;
	pSOE[report_position].n103Inf = pro_soe_inf;
	pSOE[report_position].nDPI = nDPI;
	pSOE[report_position].dtTime = TIME;
	pSOE[report_position].nResultCnt = nResultCnt;
	for (i = 0; i < nResultCnt; i++)
	{
		pSOE[report_position].ResultVal[i] = ResultBuf[i];
	}

	pSOE[report_position].nPosTime = nPosTime;


}


/*********************************************************/
/*****    告警报告存入RAM    *****************************/
/*********************************************************/
void save_arm_report(
	uint8_t nPortID, uint8_t ndeviceaddr, uint8_t arm_soe_inf, uint8_t nDPI, uint8_t nResultCnt, int32_t* ResultBuf)
{
	uint8_t i;
	uint8_t report_position;
	TSOE_WARNING_STRUCT* pSOE;
	pSOE = alarm_soe;

	report_position = alarm_soe_position & 0x7f;
	if (report_position < 99)
		alarm_soe_position++;
	else
		alarm_soe_position = 0x80;

	pSOE[report_position].nValid = 1;
	pSOE[report_position].nPortID = nPortID;
	pSOE[report_position].ndeviceaddr = ndeviceaddr;
	pSOE[report_position].n103Inf = arm_soe_inf;
	pSOE[report_position].nDPI = nDPI;
	pSOE[report_position].dtTime = TIME;
	pSOE[report_position].nResultCnt = nResultCnt;
	for (i = 0; i < nResultCnt; i++)
	{
		pSOE[report_position].ResultVal[i] = ResultBuf[i];
	}


}

/*********************************************************/
/*****    开入报告存入RAM    *****************************/
/*********************************************************/
void save_di_report(uint8_t nPortID, uint8_t ndeviceaddr, uint8_t din_soe_inf, uint8_t nSPI)
{
	uint8_t report_position;
	TSOE_DIEVENT_STRUCT* pSOE;
	pSOE = change_soe;

	report_position = change_soe_position & 0x7f;
	if (report_position < 99)
		change_soe_position++;
	else
		change_soe_position = 0x80;

	pSOE[report_position].nValid = 1;
	pSOE[report_position].nPortID = nPortID;
	pSOE[report_position].ndeviceaddr = ndeviceaddr;
	pSOE[report_position].n103Inf = din_soe_inf;
	pSOE[report_position].nSPI = nSPI;
	pSOE[report_position].dtTime = TIME;//


}


uint8_t remotectrlflg;
//读取历史数据
void RtdbProcess(void)
{
	uint8_t  data_index;
	uint8_t  i, j, k, temp_data, temp_inf, temp_addr;
	uint8_t m;
	while (1)
	{
		//return;//whs
		data_index++;
		//从缓冲区读取遥信变位信息，以虚拟报告形式存放，借助原综保处理方法发送
	 //判断命令来源，优先执行上级遥控

			//处理串口遥信数据，查询缓冲区变位信息
		if (remotectrlflg)
		{


		}
		else
		{

			for (i = 0; i < 20; i++)//uart0
			{
				if (uartpara[0].uart_usedflg == 0)//uartport null
				{
					break;
				}
				if (yx_chgdata_buf0[i][0] == 0x55)    //有变位报告
				{
					for (j = 0; j < yx_chgdata_buf0[i][2]; j++)
					{
						if (yx_data_buf0[i][j + 2] != yx_chgdata_buf0[i][j + 3])
						{
							temp_data = yx_data_buf0[i][j + 2] & 0xff;
							temp_inf = j + 149;
							temp_addr = yx_data_buf0[i][0];                                      //信息序号统一从149开始
							save_di_report(0, temp_addr, temp_inf, temp_data);
							rt_kprintf("di: inf = %d\n", temp_inf);

						}

					}
					yx_chgdata_buf0[i][0] = 0x88;
				}
				//

			}


			for (i = 0; i < 20; i++)//uart1
			{
				if (uartpara[1].uart_usedflg == 0)
				{
					break;
				}
				if (yx_chgdata_buf1[i][0] == 0x55)    //有变位报告
				{
					for (j = 0; j < yx_chgdata_buf1[i][2]; j++)
					{
						if (yx_data_buf1[i][j + 2] != yx_chgdata_buf1[i][j + 3])
						{
							temp_data = yx_data_buf1[i][j + 2] & 0xff;
							temp_inf = j + 149;
							temp_addr = yx_data_buf1[i][0];                                      //信息序号统一从149开始
							save_di_report(1, temp_addr, temp_inf, temp_data);

						}


					}
					yx_chgdata_buf1[i][0] = 0x88;
				}

			}

			for (i = 0; i < 20; i++)//uart3
			{
				if (uartpara[3].uart_usedflg == 0)
				{
					break;
				}
				if (yx_chgdata_buf3[i][0] == 0x55)    //有变位报告
				{
					for (j = 0; j < yx_chgdata_buf3[i][2]; j++)
					{
						if (yx_data_buf3[i][j + 2] != yx_chgdata_buf3[i][j + 3])
						{
							temp_data = yx_data_buf3[i][j + 2] & 0xff;
							temp_inf = j + 149;
							temp_addr = yx_data_buf3[i][0];                                      //信息序号统一从149开始
							save_di_report(3, temp_addr, temp_inf, temp_data);

						}


					}
					yx_chgdata_buf3[i][0] = 0x88;
				}

			}


			for (i = 0; i < 20; i++)//uart4
			{
				if (uartpara[4].uart_usedflg == 0)
				{
					break;
				}
				if (yx_chgdata_buf4[i][0] == 0x55)    //有变位报告
				{
					for (j = 0; j < yx_chgdata_buf4[i][2]; j++)
					{
						if (yx_data_buf4[i][j + 2] != yx_chgdata_buf4[i][j + 3])
						{
							temp_data = yx_data_buf4[i][j + 2] & 0xff;
							temp_inf = j + 149;
							temp_addr = yx_data_buf4[i][0];                                      //信息序号统一从149开始
							save_di_report(3, temp_addr, temp_inf, temp_data);

						}


					}
					yx_chgdata_buf4[i][0] = 0x88;
				}

			}


			for (i = 0; i < 20; i++)//uart5
			{
				if (uartpara[5].uart_usedflg == 0)
				{
					break;
				}
				if (yx_chgdata_buf5[i][0] == 0x55)    //有变位报告
				{
					for (j = 0; j < yx_chgdata_buf5[i][2]; j++)
					{
						if (yx_data_buf5[i][j + 2] != yx_chgdata_buf5[i][j + 3])
						{
							temp_data = yx_data_buf5[i][j + 2];
							temp_inf = j + 149;
							temp_addr = yx_data_buf5[i][0];                                      //信息序号统一从149开始
							save_di_report(3, temp_addr, temp_inf, temp_data);

						}


					}
					yx_chgdata_buf5[i][0] = 0x88;
				}

			}


			for (i = 0; i < 20; i++)//uart6
			{
				if (uartpara[6].uart_usedflg == 0)
				{
					break;
				}
				if (yx_chgdata_buf6[i][0] == 0x55)    //有变位报告
				{
					for (j = 0; j < yx_chgdata_buf6[i][2]; j++)
					{
						if (yx_data_buf6[i][j + 2] != yx_chgdata_buf6[i][j + 3])
						{
							temp_data = yx_data_buf6[i][j + 2];
							temp_inf = j + 149;
							temp_addr = yx_data_buf6[i][0];                                      //信息序号统一从149开始
							save_di_report(3, temp_addr, temp_inf, temp_data);

						}


					}
					yx_chgdata_buf6[i][0] = 0x88;
				}

			}

			for (i = 0; i < 20; i++)//uart7
			{
				if (uartpara[7].uart_usedflg == 0)
				{
					break;
				}
				if (yx_chgdata_buf7[i][0] == 0x55)    //有变位报告
				{
					for (j = 0; j < yx_chgdata_buf7[i][2]; j++)
					{
						if (yx_data_buf7[i][j + 2] != yx_chgdata_buf7[i][j + 3])
						{
							temp_data = yx_data_buf7[i][j + 2];
							temp_inf = j + 149;
							temp_addr = yx_data_buf7[i][0];                                      //信息序号统一从149开始
							save_di_report(3, temp_addr, temp_inf, temp_data);

						}


					}
					yx_chgdata_buf7[i][0] = 0x88;
				}

			}


		}

		rt_thread_delay(RT_TICK_PER_SECOND / 100);//5ms read data
	}


}
/*end*/