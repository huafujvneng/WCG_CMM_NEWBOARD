/**
 * File    	:  base_define.h
 * author	:  mayunliang
 * This file is part of WGB-900
 * Copyright (C), 2012, XJ ELECTRIC Co., LTD.
 * Date     :  2012-07-03
 * Function	:  ���������ݽṹ�ͺ궨��

 * Change Logs :
 * Date             Author           Notes
 * 2012-07-03     mayunliang      the first version
 *2017-05-13      wanghusen       add com  define
*/

#ifndef __BASE_DEFINE_H__
#define __BASE_DEFINE_H__

#include <rtthread.h>
#include "board.h"
//#include "stdlib.h"


//����������
#define HAVE_ERROR				-1	//�д���
#define ERROR					-1	//�д���
#define NO_ERROR				0	//�޴���
#define OK						0	//�޴���
#define TRUE					1	//
#define FALSE					0	//



#define UART_DATA_LEN     255

//���ݿⳣ��������
#define RTDB_CHANL_NUM    20//�����ڴ��д洢����ͨ�� ����豸
#define RTDB_YC_MAXNUM    256//���� һ֡ң�ⱨ�� ��󳤶�
#define RTDB_YX_MAXNUM    256//���� һ֡ң�ű��� ��󳤶�
#define RTDB_YM_MAXNUM    50//���� һ֡ң�ű��� ��󳤶�
#define RTDB_YX_CHGMAXNUM    20//���� �洢��λ�������ͨ����
#define RTDB_YK_MAXNUM    20//���� һ֡ң�ر��� ��󳤶�
#define RTDB_DZ_MAXNUM    256//���� һ֡��ֵ���� ��󳤶�




//���崮�ڵ�


#define serial_led0_on() bFM3_GPIO_PDOR3_PF = 0;
#define serial_led0_off() bFM3_GPIO_PDOR3_PF = 1;
#define serial_led0_flash() bFM3_GPIO_PDOR3_PF ^= 1;


#define serial_led1_on() bFM3_GPIO_PDOR3_PE = 0;
#define serial_led1_off() bFM3_GPIO_PDOR3_PE = 1;
#define serial_led1_flash() bFM3_GPIO_PDOR3_PE ^= 1;


#define serial_led3_on() bFM3_GPIO_PDOR3_PD = 0;
#define serial_led3_off() bFM3_GPIO_PDOR3_PD = 1;
#define serial_led3_flash() bFM3_GPIO_PDOR3_PD ^= 1;


#define serial_led4_on() bFM3_GPIO_PDOR3_PC = 0;
#define serial_led4_off() bFM3_GPIO_PDOR3_PC = 1;
#define serial_led4_flash() bFM3_GPIO_PDOR3_PC ^= 1;


#define serial_led5_on() bFM3_GPIO_PDOR3_PB = 0;
#define serial_led5_off() bFM3_GPIO_PDOR3_PB = 1;
#define serial_led5_flash() bFM3_GPIO_PDOR3_PB ^= 1;


#define serial_led6_on() bFM3_GPIO_PDOR3_PA = 0;
#define serial_led6_off() bFM3_GPIO_PDOR3_PA = 1;
#define serial_led6_flash() bFM3_GPIO_PDOR3_PA ^= 1;


#define serial_led7_on() bFM3_GPIO_PDOR3_P9 = 0;
#define serial_led7_off() bFM3_GPIO_PDOR3_P9 = 1;
#define serial_led7_flash() bFM3_GPIO_PDOR3_P9 ^= 1;


#define serial_led8_on() bFM3_GPIO_PDOR3_P8 = 0;
#define serial_led8_off() bFM3_GPIO_PDOR3_P8 = 1;
#define serial_led8_flash() bFM3_GPIO_PDOR3_P8 ^= 1;



#define DEVICE_NAME "XJDEVICE"
#define DEVICE_VER  0x0100 //2017/2/24 ������ ���� 4:05:04
#define PROTECT_FUN 248

#define MONITOR_FUN 1
#define GLOBAL_FUN 0xff

#define CT_VALUE_2	5.0		//�������β�ֵ
#define PT_VALUE_2	100.0	//��ѹ���β�ֵ

#define REPORT_SUM 200      //�������

 
#define	YFKR	4
#define	CNKR	5
#define	TW	    1
#define	HW	    0
#define	JXZT	17
#define    ASDU_1            1
#define    ASDU_2            2
#define    ASDU_41           41
#define    ASDU_43           43


#define DZ_NUM 100       //���ֵ����
//#define YB_NUM 14            //ѹ�峤��
//#define PRO_NUM 19           //��������

#define UARTMAXNUM        8

#define IMPORT_NUM        1
#define SAMP_NUM   
#define DEVICEMAXNUM      20
#define YKLENGTH          20
#define YTLENGTH          200


//#define PROTOCOLNUM       2//�ð汾�������ù�Լ���͸��� 

//ң���������� ң�ص�բ 0   ����  1   ң��ѹ�� 2  �޸Ķ�ֵ 3    ��ʱ 4  ��ֵ���л� 5
#define REMOTE_DL_TYPE   0//
#define REMOTE_FG_TYPE   1
#define REMOTE_YB_TYPE   2
#define REMOTE_DZ_TYPE   3
#define REMOTE_TIME_TYPE 4
#define REMOTE_QHDZ_TYPE 5



///*���������ڴŵ�洢���еĵ�ַ

#define WAWE_ADDR   0x0001
#define ETH_ADDR    0x0002


///*ʱ��ṹ�嶨��
typedef struct _TDateTime
{
    uint8_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint16_t  msec;
}TDateTime;

///*����ṹ�嶨��
typedef struct _TDIState103
{
     uint8_t      nDiInf;             //���������
     uint8_t      n103Inf;            //INF��
     uint8_t      StableState;        //�ȶ�״̬
     uint8_t      ThisState;          //��ǰ״̬
     uint8_t      JitterFlg;          //״̬����
     uint8_t      ChangeFlg;          //״̬�ı��־
     uint8_t      JitterCnt;          //��������
     TDateTime  dtTime;             //ʱ��
}TDIState103;



///*����ֵ��Ӧ�Ľṹ�壬������Ӧ������ӵ��������ͱ��
typedef struct _TMeasure103
{
	uint8_t measID;
	int16_t measVal;
	uint16_t bAutoSendFlg;    //�������ͱ��
}TMeasure103;

typedef struct _TMeterMsg
{
	int32_t meterVal;     //���ֵ
	uint8_t SequenceNum;  //˳���
}TMeterMsg;



///*������Ϣ�ṹ��
typedef struct _TSOE_ACTION_STRUCT
{
	uint8_t nValid;       //������Ч��־����λ��1����Ч 0-��Ч
                        //X X X X X X ��ʾ ����(ע����������ͣ����ͱ�־λ��0)
	uint8_t nPortID;      //�����Ӧ���豸�˿ں�
	uint8_t ndeviceaddr;   //�����������λ����ַ	
	uint8_t n103Inf;      //������Ϣ���
	

	uint8_t nDPI;         //����_���ر�־ 2-���� 1-����
	TDateTime dtTime;   //���涯��ʱ��
	uint16_t nPosTime;    //���涯�����ʱ��
	uint16_t nFAN;        //�������
	int32_t ResultVal[6]; //���涯���������
	uint8_t nResultCnt;   //���涯���������
}TSOE_ACTION_STRUCT;

///*�澯��Ϣ�ṹ��
typedef struct _TSOE_WARNING_STRUCT
{
	uint8_t nValid;       //������Ч��־����λ��1����Ч 0-��Ч
                        //X X X X X X ��ʾ ����(ע����������ͣ����ͱ�־λ��0)
	uint8_t nPortID;      //�����Ӧ���豸�˿ں�
	uint8_t ndeviceaddr;   //�����������λ����ַ	
	uint8_t n103Inf;      //������Ϣ���

	uint8_t nDPI;         //����_���ر�־ 2-���� 1-����
	TDateTime dtTime;   //���涯��ʱ��
	int32_t ResultVal[6]; //���涯���������
	uint8_t nResultCnt;   //���涯���������
}TSOE_WARNING_STRUCT;

///*������Ϣ�ṹ��
typedef struct _TSOE_DIEVENT_STRUCT
{
	uint8_t nValid;       //������Ч��־����λ��1����Ч 0-��Ч
                        //X X X X X X ��ʾ ����(ע����������ͣ����ͱ�־λ��0)
	uint8_t nPortID;      //�����Ӧ���豸�˿ں�
	uint8_t ndeviceaddr;   //�����������λ����ַ	
	uint8_t n103Inf;      //������Ϣ���

	uint8_t nSPI;         //����_���ر�־ 1-���� 0-����
	TDateTime dtTime;   //���涯��ʱ��
}TSOE_DIEVENT_STRUCT;


///*������Ϣ�ṹ��
typedef struct _TSOE_OPERATION_STRUCT
{
	uint8_t nValid;       //������Ч��־����λ��1����Ч 0-��Ч
                        //X X X X X X ��ʾ ����(ע����������ͣ����ͱ�־λ��0)
	uint8_t nDispID;      //�����Ӧ����ʾ��ţ�����ʾ��Ϣ���й�
	TDateTime dtTime;   //���涯��ʱ��
}TSOE_OPERATION_STRUCT;


///*���ź���Ϣ�ṹ�壨����ѹ���λ,��բ���ر��棬�Ե㣩
typedef struct _TSOE_GENERAL_STRUCT
{
	uint8_t nValid;       //������Ч��־����λ��1����Ч 0-��Ч
                        //X X X X X X ��ʾ ����(ע����������ͣ����ͱ�־λ��0)
	uint8_t nDispID;      //�����Ӧ����ʾ��ţ�����ʾ��Ϣ���й�
	uint8_t n103Inf;      //������Ϣ��ţ����Ժ�nDispIDͨ��һ�������Ӧ�����Ե�ֱ����INF
	uint8_t nASDU;        //����ʾASDU
	uint8_t nCOT;         //����ԭ��
	uint8_t nDPI;         //����_���ر�־ 2-���� 1-����
	TDateTime dtTime;   //���涯��ʱ��
}TSOE_GENERAL_STRUCT;


typedef struct _TPORT_PARAM_STRUCT
{
    int nTxPacks;
    int nRxPacks;

}TPORT_PARAM_STRUCT;

extern uint16_t DEVICE_CRC;

//��ֵ��ؽṹ�嶨��
typedef struct _TSET_PARA_STRUCT
{
	uint16_t max;
	uint16_t min;
	uint8_t  scale;  //ϵ��
}TSET_PARA_STRUCT;






#endif



