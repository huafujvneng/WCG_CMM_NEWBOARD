
#ifndef __UARTSERVER1_H__
#define __UARTSERVER1_H__




//#include "uart_server.h"
#include "base_define.h"
#include "rtdbdefine.h"
#include "dmac_fm3.h"
#include "uart.h"

//extern struct uart_device uart1;

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


#endif
