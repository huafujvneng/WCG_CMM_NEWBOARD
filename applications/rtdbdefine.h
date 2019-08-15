/**************************************************************************
Copyright (C), 2012, XJ ELECTRIC Co., LTD.
�ļ���  ��  rtdbdefine.h
����    ��  wanghusen
��Ŀ���ƣ�
����    ��  ���ݴ洢��Ӳ�����û�������
�������ڣ�  2017/9/6
��ע    ��
�޸ļ�¼��
**************************************************************************/
#ifndef _RTDBDEFINE_H
#define _RTDBDEFINE_H
#include "base_define.h"
/*
#define UARTMAXNUM        8

#define IMPORT_NUM        1
#define SAMP_NUM
#define DEVICEMAXNUM      20
#define YKLENGTH          20
#define YTLENGTH          200*/




/*���ݴ洢���� ˵��*/

/*

struct CENTERADDR
{
	unsigned char sourceaddr;
	unsigned char sourceindex;
};


struct MESSAGEBUFF
{
	int             length;         //���ĳ���
	struct CENTERADDR*      Centeraddr;
	unsigned char   message[256];   //���ļĴ���
};

struct PROTOCOLBUFF
{
	int             importin;
	int             importout;
	struct MESSAGEBUFF*     importbuff[IMPORT_NUM];     //��Ҫ���ļĴ���
	int             sampin;
	int             sampout;
	struct MESSAGEBUFF*     sampbuff[SAMP_NUM];         //��ͨ���ļĴ���
};*/


/***************************************���ݿⶨ��******************************************************/

/*�������ݴ洢��ʽ*/


/*�������ݻ��������ݴ洢�ṹ*/
/*struct RTDBBASEDEF
{
	struct    UARTDEVICESTRUCT*        device[DEVICEMAXNUM];
   // struct    RTDBYCSTRUCT *           ycdatabase;
   // struct    RTDBYXSTRUCT *           yxdatabase;
   // struct    RTDBYMSTRUCT *           ymdatabase;
   // struct    RTDBYKSTRUCT *           ykdatabase;
   // struct    RTDBYTSTRUCT *           ytdatabase;


};*/

/*����ң�����ݱ�λ�洢�ṹ*/
/*struct RTDBYXCHGDEF
{
				uint8_t                  devicechgnum;//
	struct    UARTDEVICESTRUCT*        uartdevice;   //
	struct    RTDBYXCHGSTRUCT *        yxchgdata;//

};*/

/*�ļ�������Ϣ*/
struct FILE_INFDEFINE
{
	uint8_t     file_name;
	uint8_t     file_type;
	uint8_t     file_ver[4];
	uint8_t     file_user;
	uint8_t     filetdate[4];
};


/****************��̫��������Ϣ����***********************/

struct  ETH_CONF
{
	uint8_t     eth_index;//��Ϣ���
	uint8_t     eth_num;//ͨ����
	uint8_t     eth_type;//����[TYPE = SERIAL]
	uint8_t     eth_protocol;//��Լ����[PROTOCOL = MODBUS] [PROTOCOL = IEC103]
	uint8_t     eth_daulnet;	//˫����־

	uint32_t    eth_IP1;//IP1��ַ
	uint8_t     eth_port;//IP1�˿�

	//uint32_t    eth_IP2;//IP2��ַ
	//uint8_t     eth_port2;//IP2�˿�    

	uint16_t    eth_recv_time;//���ճ�ʱʱ��
	uint16_t    eth_send_time;//���ͳ�ʱ
	uint16_t    eth_yctrans_time;//ң��ѭ����������ʱ��
	uint8_t     eth_trans_soe;//�Ƿ���SOE
	uint8_t     eth_trans_soefg;//�Ƿ���SOE�����ź�
	uint8_t     eth_yc_datatype;//[YCSENDFLAG = INT16MAX] [YCSENDFLAG = FLOAT32]ң����������
	uint8_t     eth_trans_yxnum;//ÿ֡����ң�Ÿ���
	uint8_t     eth_trans_ycnum;//ÿ֡����ң�����
	uint8_t     eth_timingselect;//�Ƿ�Ĭ��Уʱͨ��
	uint16_t    eth_yk_timeout;//ң�س�ʱʱ��

	uint32_t    eth_MASK1;
	uint32_t    eth_GW1;

	//uint32_t    eth_IP2;
	//uint32_t    eth_MASK2;
	//uint32_t    eth_GW2;
};



/****************���ڷ������ݽṹ�嶨��***********************/


struct DEVICEPARA
{

	uint8_t        deviceaddr; //װ�õ�ַ
	//uint8_t        yccmd; 	
	//uint16_t        ycaddr;//��λ��ǰ ��λ�ں�
	uint8_t        ycnum;
	//uint8_t        ycdatalen;//ң�����ݳ��� ������֡ͷ����ַ+��� CRC��2���ֽڣ�

	//uint8_t        yxcmd; 	    
	//uint16_t        yxaddr;//��λ��ǰ ��λ�ں�
	uint8_t        yxnum;
	//uint8_t        yxdatalen;//ң�����ݳ��� ������֡ͷ����ַ+��� CRC��2���ֽڣ�

   // uint8_t        ykdlcmd; 	//���� ��·��    
   // uint16_t        ykdladdr;//��λ��ǰ ��λ�ں�
	uint8_t        ykdlnum;//ykbuf[YKLENGTH];       
   // uint8_t        ykdldatalen;//ң�����ݳ��� ������֡ͷ����ַ+��� CRC��2���ֽڣ�

   // uint8_t        ykybcmd; 	    // ѹ�� ��ֵ����
   // uint16_t        ykybaddr;//��λ��ǰ ��λ�ں�
	uint8_t        ykybnum;//ykbuf[YKLENGTH];       
   // uint8_t        ykybdatalen;//ң�����ݳ��� ������֡ͷ����ַ+��� CRC��2���ֽڣ�    

	//uint8_t        ymcmd;//�����
		//uint16_t        ymaddr;
	uint8_t        ymnum;
	//uint8_t        ymdatalen;

	//uint8_t        reddzcmd;//��ֵ
	//uint16_t        reddzaddr;
	uint8_t        reddznum;


	//uint8_t        ytdzcmd;
	//uint16_t        ytdzaddr;
	uint8_t        ytdznum;


};


struct  UART_CONF
{
	uint8_t                   uart_usedflg;//�ô�������
	uint8_t                   channel_index;//ͨ�����
	uint8_t                   channel_num;//ͨ����
	uint8_t                   channel_type;//ͨ������
	uint8_t                   uart_protocol;	//��Լ���� 
	uint16_t                  uart_read_time;//��ѯ���ʱ�� 
	uint16_t                  uart_receive_time;//���յȴ�ʱ��    
	uint8_t                   uart_transpond;//����ģʽ
	uint8_t                   uart_port;//���ں�

	uint16_t                   uart_baudrate;//������
	uint8_t                   uart_datawidth;//���ݿ��

	uint8_t                   uart_stopbit;//ֹͣλ 

	uint8_t                   uart_parity;//Ч�鷽ʽ

	uint8_t                   uart_type;//��������

	uint8_t                   devusednum;    //�豸����
	uint8_t                   devaddr[DEVICEMAXNUM];//�豸��ַ
	//struct DEVICEPARA         deviceconf[DEVICEMAXNUM];//


};




//���崮���豸���״̬
struct UART_TXRX_STRUCT
{
	uint8_t    uart_devonline;

	uint8_t    uart_devicenum;

	uint8_t    uart_readycflg;
	uint8_t    uart_readyxflg;
	uint8_t    uart_ykflg;
	uint8_t    uart_TxOkflg;
	uint8_t    uart_StartTxflg;
	uint8_t    uart_TxErrflg;
	uint8_t    uart_TxErrcount;

	uint8_t    uart_RxOkflg;
	uint8_t    uart_RxErrflg;
	uint8_t    uart_RxErrcount;

	//uint8_t    uart_TxMsgbuf[UART_DATA_LEN];

	//uint8_t    uart_RxMsgbuf[UART_DATA_LEN];

	int32_t		 uart_nASendcount;	//���Ͱ���
	int32_t		 uart_nARecvcount;	//��ȷ���յİ���
	int32_t		 uart_nAErrcount;	//���մ���İ���
};//UART_TXRX_STRUCT;


struct UART_TXRX_STATUS
{
	struct UART_TXRX_STRUCT   uartdevice[DEVICEMAXNUM];

	uint8_t   reddevindex;//��˳�� ��ѯ������װ�� 
	uint8_t   uart_rtdbflg;//���� ����Զ�̲���
	uint8_t   uart_opdevbuf[DEVICEMAXNUM];//ҪԶ�̲�����װ�õ�ַ
	uint8_t    uart_redycyxflg;	  //��������ѯ ң�� ң�� ��־
	uint8_t   uart_trans_buf[256];
	uint8_t   uart_recv_buf[256];
	uint8_t   uart_trans_flg;
	uint8_t   uart_recv_flg;
	uint32_t  uart_trans_time;
	uint32_t  uart_recv_time;
};

struct ETH_TXRX_STATUS
{

	uint8_t    eth_trans_flg;
	uint8_t    eth_recv_flg;
	uint32_t   eth_trans_time;
	uint32_t   eth_recv_time;
	uint8_t    eth_trans_buf[300];
	uint8_t    eth_recv_buf[300];
};




#endif




