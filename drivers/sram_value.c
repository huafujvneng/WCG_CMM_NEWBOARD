/**************************************************************************
Copyright (C), 2012, XJ ELECTRIC Co., LTD.
文件名  ：  sram_value.c
作者    ：  wanghusen
项目名称：   
功能    ：  定义外部RAM编译的变量
创建日期：  2017年5月13日
备注    ：   

**************************************************************************/
#include <rtthread.h>
#include "base_define.h"

char test_value,test_value1;

//定义内存区 根据串口数量分配内存存取数据会比较直观
//port0
rt_uint16_t yc_data_buf0[RTDB_CHANL_NUM][RTDB_YC_MAXNUM]  __attribute__ ((section ("EXRAM")));// save yc data
rt_uint16_t yx_data_buf0[RTDB_CHANL_NUM][RTDB_YX_MAXNUM]  __attribute__ ((section ("EXRAM")));//save yx data
rt_uint16_t ym_data_buf0[RTDB_CHANL_NUM][RTDB_YM_MAXNUM]  __attribute__ ((section ("EXRAM")));//save ym data
rt_uint16_t yx_chgdata_buf0[RTDB_YX_CHGMAXNUM][RTDB_YX_MAXNUM]  __attribute__ ((section ("EXRAM")));//save yx change data
rt_uint16_t yk_data_buf0[RTDB_CHANL_NUM][RTDB_YC_MAXNUM]  __attribute__ ((section ("EXRAM")));// save yk data

//port1
rt_uint16_t yc_data_buf1[RTDB_CHANL_NUM][RTDB_YC_MAXNUM]  __attribute__ ((section ("EXRAM")));// save yc data
rt_uint16_t yx_data_buf1[RTDB_CHANL_NUM][RTDB_YX_MAXNUM]  __attribute__ ((section ("EXRAM")));//save yx data
rt_uint16_t yx_chgdata_buf1[RTDB_YX_CHGMAXNUM][RTDB_YX_MAXNUM]  __attribute__ ((section ("EXRAM")));//save yx change data
rt_uint16_t ym_data_buf1[RTDB_CHANL_NUM][RTDB_YM_MAXNUM]  __attribute__ ((section ("EXRAM")));//save ym data
rt_uint16_t yk_data_buf1[RTDB_CHANL_NUM][RTDB_YC_MAXNUM]  __attribute__ ((section ("EXRAM")));// save yk data
//port3
rt_uint16_t yc_data_buf3[RTDB_CHANL_NUM][RTDB_YC_MAXNUM]  __attribute__ ((section ("EXRAM")));// save yc data
rt_uint16_t yx_data_buf3[RTDB_CHANL_NUM][RTDB_YX_MAXNUM]  __attribute__ ((section ("EXRAM")));//save yx data
rt_uint16_t yx_chgdata_buf3[RTDB_YX_CHGMAXNUM][RTDB_YX_MAXNUM]  __attribute__ ((section ("EXRAM")));//save yx change data
rt_uint16_t ym_data_buf3[RTDB_CHANL_NUM][RTDB_YM_MAXNUM]  __attribute__ ((section ("EXRAM")));//save ym data
rt_uint16_t yk_data_buf3[RTDB_CHANL_NUM][RTDB_YC_MAXNUM]  __attribute__ ((section ("EXRAM")));// save yk data
//port4
rt_uint16_t yc_data_buf4[RTDB_CHANL_NUM][RTDB_YC_MAXNUM]  __attribute__ ((section ("EXRAM")));// save yc data
rt_uint16_t yx_data_buf4[RTDB_CHANL_NUM][RTDB_YX_MAXNUM]  __attribute__ ((section ("EXRAM")));//save yx data
rt_uint16_t yx_chgdata_buf4[RTDB_YX_CHGMAXNUM][RTDB_YX_MAXNUM]  __attribute__ ((section ("EXRAM")));//save yx change data
rt_uint16_t ym_data_buf4[RTDB_CHANL_NUM][RTDB_YM_MAXNUM]  __attribute__ ((section ("EXRAM")));//save ym data
rt_uint16_t yk_data_buf4[RTDB_CHANL_NUM][RTDB_YC_MAXNUM]  __attribute__ ((section ("EXRAM")));// save yk data
//port5
rt_uint16_t yc_data_buf5[RTDB_CHANL_NUM][RTDB_YC_MAXNUM]  __attribute__ ((section ("EXRAM")));// save yc data
rt_uint16_t yx_data_buf5[RTDB_CHANL_NUM][RTDB_YX_MAXNUM]  __attribute__ ((section ("EXRAM")));//save yx data
rt_uint16_t yx_chgdata_buf5[RTDB_YX_CHGMAXNUM][RTDB_YX_MAXNUM]  __attribute__ ((section ("EXRAM")));//save yx change data
rt_uint16_t ym_data_buf5[RTDB_CHANL_NUM][RTDB_YM_MAXNUM]  __attribute__ ((section ("EXRAM")));//save ym data
rt_uint16_t yk_data_buf5[RTDB_CHANL_NUM][RTDB_YC_MAXNUM]  __attribute__ ((section ("EXRAM")));// save yk data

//port6
rt_uint16_t yc_data_buf6[RTDB_CHANL_NUM][RTDB_YC_MAXNUM]  __attribute__ ((section ("EXRAM")));// save yc data
rt_uint16_t yx_data_buf6[RTDB_CHANL_NUM][RTDB_YX_MAXNUM]  __attribute__ ((section ("EXRAM")));//save yx data
rt_uint16_t yx_chgdata_buf6[RTDB_YX_CHGMAXNUM][RTDB_YX_MAXNUM]  __attribute__ ((section ("EXRAM")));//save yx change data
rt_uint16_t ym_data_buf6[RTDB_CHANL_NUM][RTDB_YM_MAXNUM]  __attribute__ ((section ("EXRAM")));//save ym data
rt_uint16_t yk_data_buf6[RTDB_CHANL_NUM][RTDB_YC_MAXNUM]  __attribute__ ((section ("EXRAM")));// save yk data
//port7
rt_uint16_t yc_data_buf7[RTDB_CHANL_NUM][RTDB_YC_MAXNUM]  __attribute__ ((section ("EXRAM")));// save yc data
rt_uint16_t yx_data_buf7[RTDB_CHANL_NUM][RTDB_YX_MAXNUM]  __attribute__ ((section ("EXRAM")));//save yx data
rt_uint16_t yx_chgdata_buf7[RTDB_YX_CHGMAXNUM][RTDB_YX_MAXNUM]  __attribute__ ((section ("EXRAM")));//save yx change data
rt_uint16_t ym_data_buf7[RTDB_CHANL_NUM][RTDB_YM_MAXNUM]  __attribute__ ((section ("EXRAM")));//save ym data
rt_uint16_t yk_data_buf7[RTDB_CHANL_NUM][RTDB_YC_MAXNUM]  __attribute__ ((section ("EXRAM")));// save yk data





void init_sram(void)
{
	char i;
	for(i=0;i<RTDB_CHANL_NUM;i++)
	{
			memset(yc_data_buf0[i],0,sizeof(yc_data_buf0[i]));
		  memset(yx_data_buf0[i],0,sizeof(yx_data_buf0[i]));
		  memset(ym_data_buf0[i],0,sizeof(ym_data_buf0[i]));		
      memset(yx_chgdata_buf0[i],0,sizeof(yx_chgdata_buf0[i]));
		  memset(yk_data_buf0[i],0,sizeof(yk_data_buf0[i]));
		
		
			memset(yc_data_buf1[i],0,sizeof(yc_data_buf1[i]));
		  memset(yx_data_buf1[i],0,sizeof(yx_data_buf1[i]));
		  memset(ym_data_buf1[i],0,sizeof(ym_data_buf1[i]));			
      memset(yx_chgdata_buf1[i],0,sizeof(yx_chgdata_buf1[i]));
		  memset(yk_data_buf1[i],0,sizeof(yk_data_buf1[i]));
		
			memset(yc_data_buf3[i],0,sizeof(yc_data_buf3[i]));
		  memset(yx_data_buf3[i],0,sizeof(yx_data_buf3[i]));
		  memset(ym_data_buf3[i],0,sizeof(ym_data_buf3[i]));	
      memset(yx_chgdata_buf3[i],0,sizeof(yx_chgdata_buf3[i]));
		  memset(yk_data_buf3[i],0,sizeof(yk_data_buf3[i]));
		
			memset(yc_data_buf4[i],0,sizeof(yc_data_buf4[i]));
		  memset(yx_data_buf4[i],0,sizeof(yx_data_buf4[i]));
		  memset(ym_data_buf4[i],0,sizeof(ym_data_buf4[i]));			  
      memset(yx_chgdata_buf4[i],0,sizeof(yx_chgdata_buf4[i]));
		  memset(yk_data_buf4[i],0,sizeof(yk_data_buf4[i]));
		
			memset(yc_data_buf5[i],0,sizeof(yc_data_buf5[i]));
		  memset(yx_data_buf5[i],0,sizeof(yx_data_buf5[i]));
		  memset(ym_data_buf5[i],0,sizeof(ym_data_buf5[i]));			  
      memset(yx_chgdata_buf5[i],0,sizeof(yx_chgdata_buf5[i]));
		  memset(yk_data_buf5[i],0,sizeof(yk_data_buf5[i]));
		  
			memset(yc_data_buf6[i],0,sizeof(yc_data_buf6[i]));
		  memset(yx_data_buf6[i],0,sizeof(yx_data_buf6[i]));
		  memset(ym_data_buf6[i],0,sizeof(ym_data_buf6[i]));	
      memset(yx_chgdata_buf6[i],0,sizeof(yx_chgdata_buf6[i]));
		  memset(yk_data_buf6[i],0,sizeof(yk_data_buf6[i]));
		  
			memset(yc_data_buf7[i],0,sizeof(yc_data_buf7[i]));
		  memset(yx_data_buf7[i],0,sizeof(yx_data_buf7[i]));
		  memset(ym_data_buf7[i],0,sizeof(ym_data_buf7[i]));	
      memset(yx_chgdata_buf7[i],0,sizeof(yx_chgdata_buf7[i]));
		  memset(yk_data_buf7[i],0,sizeof(yk_data_buf7[i]));			
	}

}

/*end*/