
/*   HISTORY                                                                 */
/*                                                                           */
/*   NAME  DATE          REMARKS                                             */
/*                                                                           */


#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include "485comm.h"
#include "base_define.h"
#include "base_task.h"
#include "protect.h"


/*
extern uint8_t busy_7022;

extern uint8_t reset_flag,gps_status;

extern uint16_t  soe_fgfs,time_correct_mode;
extern uint16_t  angle[7];
extern uint32_t aangle,bangle,cangle,Uorder;
extern int32_t  angle_buf[];
extern uint32_t pajg,pbjg,pcjg;

extern uint16_t cbt,ui_m,ui_t;
extern uint8_t cl_chk,tp_chk,hc_chk;
extern uint8_t Save_Number;
extern uint16_t  dz_buf[][DZ_NUM];
//whs 暂时不用  extern uint8_t value_change_flag;
//whs 暂时不用  extern uint8_t yb_change_flag;
//whs 暂时不用  extern uint8_t time_change_flag;
//whs 暂时不用  extern uint8_t setqht0,time_write_flg,fgb;
//whs 暂时不用  extern uint16_t  cx_crc,dz_crc;
extern uint16_t password;
extern uint16_t  dz_buf_xg[];//修改
//extern int8_t Para_DCout[];
//extern int8_t Para_DCout_bak[];
//extern uint16_t  dcjg_buf[];
//extern int16_t Para_DCin[];
//extern int16_t Para_DCin_bak[];

//extern INTTOBIT1 proqdb,xsb_tz,fxs_tz,tzb_fh;
//extern INTTOBIT2 xsb_gj,fxs_gj,gjb_fh;
//extern INTTOBIT3 lb_state;
//extern INTTOBIT6 out_bz;

extern TDateTime TIME;
extern void set_rtc(void);

extern void clear_report(void);
extern void get_calibrate(void);
extern void test_calibrate(void);
extern void SetTime(void);
extern void Init_Uart1(void);
//whs 暂时不用  extern void get_pro_data(void);
//extern void get_dz(uint8_t zone);
void save_general_report(uint8_t nDispID,uint8_t n103Inf,uint8_t ASDU, uint8_t nDPI);
extern void rt_hw_uart4_init(void);
//whs 暂时不用  extern void get_pro_data(void);
extern void drive_export(uint8_t i);
extern void rt_hw_uart5_init(void);
extern void rt_hw_uart7_init(void);
extern void save_ope_report(uint8_t nDispID);
extern uint32_t by_out_data[];
extern void time_correct_init(void);
//extern INTTOBIT8 Opreat_bz;*/

///////////////////////////////////////////////////////////
//============================================


#define Setting_COMMAND_See	  0x00001000  /*查看定值*///
#define Ver_See               0x00002000  /*看CRC1*/
#define Di_See                0x00004000  /*看开入*/
#define Reportclose_See       0x00008000  /*看合闸记录*/
#define Reportalrm_See        0x00010000  /*看告警记录*/
#define Dichange_See          0x00020000  /*看变位记录*/
#define Reportopreat_See      0x00040000  /*看操作记录*/
#define Addr_See      	   	  0x01000000  /*看装置参数*/
#define Jgkd_See	   	      0x02000000  /*看调整刻度结果*/
#define Time_See	   	      0x04000000  /*看时间*/
#define Jump_See	   	      0x08000000  /*看压板*/
#define Report_See	   	      0x10000000  /*看显示报告*/
#define Gen_Inspect_End       0x00000040  /*总召唤结束*/
#define Dcout_See             0x20000000  /*上送直流调整结果*/
/******autotest**********/
#define Key_See               0x80000000  /*回读按键结果*/
#define Led_drive             0x40000000  /*驱动信号灯*/
#define Com232_test           0x00100000  /*面板232串口检测*/
#define Dog_test              0x00200000  /*面板看门狗检测*/
#define Dog_rst               0x00400000  /*面板看门狗重启*/
#define Devinf_read           0x00800000  /*面板版本信息*/
/******autotest**********/

#define FrameMeas   0xbb
#define FrameTime   0xcc

//显示相关
/*void Tran_addtion(uint16_t i);
void TransferFix103JK_d(struct uart_device* uart);
void TransferUnfix103JK1_d(struct uart_device* uart);
void Receive_ASDU_61_d(struct uart_device* uart);
void Receive_ASDU_20_d(struct uart_device* uart);
void Receive_ASDU_62_d(struct uart_device* uart);
void Value_save(struct uart_device* uart);
void Commset_save(struct uart_device* uart);
void Clockset_save(struct uart_device* uart);
void passwordset_save(struct uart_device* uart);
void yaban_save(struct uart_device* uart);
void csset_save(struct uart_device* uart);
void ddset_save(struct uart_device* uart);
void outset_save(struct uart_device* uart);
void Sure_MsgJK2(struct uart_device* uart);
void Trans_Soe_Protect_d(struct uart_device* uart);
void Trans_Soe_Alarm_d(struct uart_device* uart);
void Trans_Soe_Change_d(struct uart_device* uart);
void Trans_Soe_Operate_d(struct uart_device* uart);*/
//end
#endif
//==============================================
//no more
//==============================================
