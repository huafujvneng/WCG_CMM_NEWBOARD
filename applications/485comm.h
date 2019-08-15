#ifndef __485COMM_H__
#define __485COMM_H__

#include <rtthread.h>
#include "base_define.h"
#include "uart.h"
#include "adc_task.h"

//#include "rtdbdefine.h"

/****************************************/


#define	GJ_NUM      	16//whs 15
#define TZ_NUM          13
#define	MEASURE50_NUMBER	5


#define NONEWDATA   0
#define Start_Code1 	0x10
#define Start_Code2 	0x68
#define EndCode		    0x16
#define ZeroValue	0x00
#define FrameFix	0x55
#define FrameUnFix	0xaa
#define SubAddress	0x01
#define Start_to_trans 	0xaa
#define ReTransmit  	0xdd          /*sure that retransmit */
#define NoTransmit  	0xcc          /*sure that no retransmit */
#define	PROGRAM_VER	0x100

#define PULSENUM          20           //����·��

#define MEASURE_DATA_TIME 2000
#define wsssssss1   2000
#define wsssssss2   2000
#define wsssssss1   2000
#define wsssssss2   2000
#define wsssssss1   2000
#define wsssssss2   2000
#define wsssssss1   2000
#define wsssssss2   2000

/*MessageFlagA ���ı�־*/
#define Reset_FCB         0x00000001
#define Reset_CU          0x00000002
#define Reset_Device      0x00000004
#define ReModify_Time     0x00000008
#define Sets_Parameter    0x00000010  /*��������*/
#define Gen_Inspect_Start 0x00000020  /*���ٻ�����*/
#define Gen_Inspect_End   0x00000040  /*���ٻ�����*/

#define DIN_Status        0x00000080  /*ң��״̬*/
#define DIN_Status_Change 0x00000100  /*ң��״̬��λ*/

#define Jump_Status		0x00000200	//ѹ��״̬
#define Jump_Status_Change	0x00000400	//ѹ��״̬��λ

#define Disturb_Inspection   0x00000800  /*����ʱ1��������¼���Ŷ���*/


#define CB_Status	  0x00001000  /*��·��λ��*/
#define CB_Status_Change  0x00002000  /*��·��λ�øı�*/
#define Coulometer_Freeze 0x00004000  /*��ȶ���*/
#define Coulometer_Count  0x00008000  /*�����*/
#define Return_Version    0x00010000  /*���汾��*/
#define Select_CB_Obj     0x00020000  /*ѡ���·������*/
#define Exec_CB_Obj       0x00040000  /*ִ�ж�·������*///circuit breaker
#define Esc_CB_Obj        0x00080000  /*������·������*/
//#define Command_Reply     0x00100000  /*?*/
#define GetSOE_COMMAND	  0x00100000  /*�����ϱ���*/
//#define WarningSignals   	0x00200000
#define Return_GetSOE        0x00200000  /*�����ϱ���*/

#define ASDU20_Sure     	0x00400000  /*ASDU20�϶��ش�*/
#define ASDU20_Unsure   	0x00800000  /*ASDU20�񶨻ش�*/
#define Self_Inspection	   	    0x01000000
#define ERROR_COMMAND	   	    0x02000000
#define Setting_COMMAND	   	    0x04000000
#define Coulometer_End  0x08000000  /*���������*/
#define Coulometer_No  0x10000000  /*���������*/
#define Setting_COMMAND_OVER	   	    0x20000000 		/*�ж�ֵ�ش�/��ֵ�޸�Ԥ��/��У����ʱ�Ķ�ֵ����֡*/
#define Gen_Inspect_Vector0	0x40000000
#define SendMeasure	0x80000000

/*ErrorMsgFlagA ���ı�־*/
#define Error_EEPROM         0x00000001
#define Error_set            0x00000002
#define Error_Setting        0x00000004
#define OC_ACC_Direction     0x00000008
#define Zero1_Direction      0x00000010
#define Over_Load_Direction  0x00000020
#define Error_AD             0x00000040
#define OC_I_Direction       0x00000080
#define OC_II_Direction      0x00000100
#define OC_III_Direction     0x00000200
#define LF_Direction         0x00000400
#define Reclose_Direction    0x00000800
#define DO_INFO              0x00001000
#define SETTINGNUM_INFO      0x00002000
#define MXPT_INFO            0x00004000
#define XLPT_INFO            0x00008000
#define KZHL_INFO            0x00010000
#define GFHGJ_INFO           0x00020000
#define LXGL_INFO            0x00040000

#define Error_EPROM_         0x00000000
#define Error_RAM_           0x00000001
#define Error_Setting_       0x00000002
#define Error_Relay_         0x00000003
#define Error_5VPower_       0x00000004
#define Error_CPU_Number_    0x00000005
#define Error_AD_            0x00000006
//#define OC_I_Direction_    0x00000007
/*the  flag of control code*/
#define _FCV              0x10
#define _FCB              0x20
#define _PRM              0x40
#define _ACD              0x20
/*the  flag of control CB/Tap operation*/
#define _ACT              0x40
#define _SE               0x80
#define _DCO              0x03
#define _FRZ              0xc0
#define _REQ              0x3f           /* 0x3f    */


extern rt_uint8_t zzaddr,zzaddr_0,zzaddr_1,zzaddr_3,zzaddr_4,zzaddr_5,zzaddr_6,zzaddr_7;
extern void save_ope_report(uint8_t nDispID);
void save_general_report(uint8_t nDispID,uint8_t n103Inf,uint8_t ASDU, uint8_t nDPI);
extern void set_rtc(void);

extern struct uart_device uart0;
extern struct uart_device uart1;
extern struct uart_device uart3;
extern struct uart_device uart4;
extern struct uart_device uart5;
extern struct uart_device uart6;
extern struct uart_device uart7;
extern TDateTime TIME;
extern uint16_t FaultNumber;
extern uint16_t DEVICE_CRC;
extern TMeasure103 measure_send[];



void DisturbASDU23(struct uart_device* uart);
#endif
