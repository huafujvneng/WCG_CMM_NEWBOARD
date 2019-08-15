
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
//whs ��ʱ����  extern uint8_t value_change_flag;
//whs ��ʱ����  extern uint8_t yb_change_flag;
//whs ��ʱ����  extern uint8_t time_change_flag;
//whs ��ʱ����  extern uint8_t setqht0,time_write_flg,fgb;
//whs ��ʱ����  extern uint16_t  cx_crc,dz_crc;
extern uint16_t password;
extern uint16_t  dz_buf_xg[];//�޸�
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
//whs ��ʱ����  extern void get_pro_data(void);
//extern void get_dz(uint8_t zone);
void save_general_report(uint8_t nDispID,uint8_t n103Inf,uint8_t ASDU, uint8_t nDPI);
extern void rt_hw_uart4_init(void);
//whs ��ʱ����  extern void get_pro_data(void);
extern void drive_export(uint8_t i);
extern void rt_hw_uart5_init(void);
extern void rt_hw_uart7_init(void);
extern void save_ope_report(uint8_t nDispID);
extern uint32_t by_out_data[];
extern void time_correct_init(void);
//extern INTTOBIT8 Opreat_bz;*/

///////////////////////////////////////////////////////////
//============================================


#define Setting_COMMAND_See	  0x00001000  /*�鿴��ֵ*///
#define Ver_See               0x00002000  /*��CRC1*/
#define Di_See                0x00004000  /*������*/
#define Reportclose_See       0x00008000  /*����բ��¼*/
#define Reportalrm_See        0x00010000  /*���澯��¼*/
#define Dichange_See          0x00020000  /*����λ��¼*/
#define Reportopreat_See      0x00040000  /*��������¼*/
#define Addr_See      	   	  0x01000000  /*��װ�ò���*/
#define Jgkd_See	   	      0x02000000  /*�������̶Ƚ��*/
#define Time_See	   	      0x04000000  /*��ʱ��*/
#define Jump_See	   	      0x08000000  /*��ѹ��*/
#define Report_See	   	      0x10000000  /*����ʾ����*/
#define Gen_Inspect_End       0x00000040  /*���ٻ�����*/
#define Dcout_See             0x20000000  /*����ֱ���������*/
/******autotest**********/
#define Key_See               0x80000000  /*�ض��������*/
#define Led_drive             0x40000000  /*�����źŵ�*/
#define Com232_test           0x00100000  /*���232���ڼ��*/
#define Dog_test              0x00200000  /*��忴�Ź����*/
#define Dog_rst               0x00400000  /*��忴�Ź�����*/
#define Devinf_read           0x00800000  /*���汾��Ϣ*/
/******autotest**********/

#define FrameMeas   0xbb
#define FrameTime   0xcc

//��ʾ���
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
