/**
 * File    	:  eth_server103.c
 * author	:  mayunliang
 * This file is part of WGB-900
 * Copyright (C), 2012, XJ ELECTRIC Co., LTD.
 * Date     :  2012-07-03
 * Function	:  TCP103ͨѶ

 * Change Logs :
 * Date             Author           Notes
 * 2012-07-03     mayunliang      the first version
*/

#include <stdio.h>
#include <string.h>

#include <lwip/api.h>
#include <lwip/sockets.h>

#include "eth_server103.h"
#include "ethernetif.h"
#include "eth_callee.h"
#include "iec.h"


extern struct ETH_TXRX_STATUS eth_status;
/*************************************************
//  ���ݽṹ��ز���
************************************************/
#define	 OPERATOR_PORT 						2404						//normal connection port
#define	 ENGINEER_PORT						2403						//Engineer connection port
#define	 DEBUG_PORT							  4661						//debug information port



//rtSvrMain
#define	MAX_CLIENT_NUM			8
#define	MAX_CHILDPORT_STACK		2408//1024//4096//�Ӷ˿������ջ��С



//tcp103CentralTask
#define READ_TIMEOUT            100
#define WRITE_TIMEOUT           1000
#define APDU_HEAD_LENGTH		10
#define ASDU_COPY_LENGTH		10
#define	DEVICE_NAME_LENGTH		8
#define	VALUE_EACH_FRAME		60    //ÿ֡�еĶ�ֵ�������
#define SOE_CONFIRMED			-1

//#define WAVE_PAG_LENGTH			200

//#define MSG_TYPE		//��ʾ��������
//#define MSG_ERROR		//��ʾ������Ϣ
//#define MSG_BUFFER	//��ʾ��������

#define MAKEWORD(a,b) ( ((a)<<8)+(b) )
#define LOBYTE(a) ( (a)&0x0ff )
#define HIBYTE(a) ( ((a)>>8)&0x0ff )
#define LOWORD(a) ( (a)&0x0ffff )
#define HIWORD(a) ( ((a)>>16)&0x0ffff )

#define logMsg rt_kprintf



#define YK_DL_END_INDEX 0x6100

//APCI�ṹ����
typedef struct _APCI
{
	uint8_t	m_byteStartChar;	//�����ַ�
	uint8_t	m_byteApduLength;	//APDU����
	uint16_t	m_wordSendNum;		//�������к�
	uint16_t	m_wordRecvNum;		//�������к�
}APCI;

//APDU�ṹ����
#define MAX_MSG 243
#define FUN_103 0
#define INF_103 1
#define MSG_103 2
#define INF_104L 0
#define INF_104M 1
#define INF_104H 2
#define MSG_104 3
typedef struct _APDU
{
	APCI	m_APCI;				//������

	uint8_t  m_byteType;
	uint8_t  m_byteVSQ;
	uint16_t m_wordCOT;
	uint8_t  m_byteCOMADD;		//��Ԫ������ַ
	uint8_t  m_byteADD;         //װ�õ�ַ

	uint8_t	m_msgdata[MAX_MSG];//������(�����￪ʼ104��103�Ͳ�һ����)
}APDU;

typedef struct _SOEINFO
{
	uint8_t soeCount;                 //��ͨѶ���͵�SOE����
	int8_t soeType;                   //��ͨѶ���͵�SOE����
}SOEINFO;

typedef struct _WAVEDATAFILE
{
	int32_t	packageCnt;					//¼�������ܰ���
	int32_t	waveLen;					//¼�������ļ��ܳ���
	uint8_t autoSendFlag;             //�������ͱ��
	uint8_t pad1;
}WAVEDATAFILE;


//ͨѶ�����ṹ
typedef struct _PARAMS
{
	//װ�õ�ַ
	int32_t		m_device_address;

	//���ӱ�־
	int32_t		m_serverFd;		//����socket��־��
	int32_t		m_nLinkSocket;	//��ʾ����SOE��Ϣʱ����·��
	rt_thread_t	m_taskID;	//����ID��
	uint32_t	m_clientIP;		//�ͻ���IP��ַ
	uint32_t  m_severPort;    //�������˿�

	//������ָ��
	APDU*	m_pSendMsg;		//���ͻ�����
	APDU*	m_pRecvMsg;		//���ջ�����
	APDU*	m_pSaveMsg;		//������ջ�����
	APCI*	m_pCtrlMsg;		//���Ʊ��Ļ�����

	uint16_t	m_nSendNum;		//�������к�
	uint16_t	m_nRecvNum;		//�������к�
	uint16_t	m_nConfirmedNum;//�ѱ�ȷ�ϵ����к�
	uint16_t	m_nSendNum_SOE;	//����SOE����ʱ�ķ������к�

	int32_t		m_nSendPacket;	//���Ͱ���
	int32_t		m_nRecvPacket;	//��ȷ���յİ���
	int32_t		m_nErrPackets;	//���մ���İ���

	//������Ϣ
	uint8_t	m_bRecvStartDT;	//�Ƿ��յ���������
	uint8_t	m_bWorkingPort;	//�Ƿ����˿�
//    uint8_t	m_bSOESend;	    //�Ƿ���SOE
	uint32_t  tick_SOESend;

	//SOEȷ�ϱ�־
	int32_t		m_nAutoConfirmed;//-1��ʾ�Ѿ�ȷ��

	//wusenlin add
	//PORT_CON_STRUCT *m_thePortConfig;

	//yantianjun add
    //SOE
	SOEINFO soeInfo;

	//¼������
	uint8_t waveTransferringFlag;		//���ڴ����־
	uint8_t waveChannelName;          //�����ͨ������
	uint32_t tick_WaveStart;

	int32_t soeCnt; //debug ���SOE����,ASDU41��ASDU2��ASDU1��ASDU80
}PARAMS;

//ͨѶ����������ṹ
typedef struct _CHILDTASKPAR
{
    int32_t serverFd;
    uint32_t s_addr;
    uint16_t sin_port;
    PARAMS** ppParams;//����������������ָ������
//    PORT_CON_STRUCT *PortConfig;//ͨѶ���ò���
}CHILDTASKPAR;
//static int32_t DICnt =0 ; //debug ���SOE����,ASDU41��ASDU2��ASDU1��ASDU80
/*************************************************
//  ��������ز���
************************************************/
static int32_t centralSend(PARAMS *pParams);
static int32_t centralRecv(PARAMS *pParams);
static int32_t centralProcess(PARAMS *pParams);


//#include "inetLib.h"

/*����ͨ�õĺ����ӿ�*/
static int32_t selectWrite(int32_t sFd,int32_t WriteTimeout)
{
	struct              fd_set  writeFds;
	struct              timeval timeout;
	int32_t                    width;

	if(WriteTimeout>=1000)
	{
		timeout.tv_sec = WriteTimeout/1000;
        timeout.tv_usec = WriteTimeout%1000;
	}
	else
    {
        timeout.tv_sec = 0;
        timeout.tv_usec = WriteTimeout;
	}

	FD_ZERO (&writeFds);
	FD_SET (sFd, &writeFds);
	width = sFd+1;

	if (select (width, (fd_set *)0, &writeFds, (fd_set *)0, &timeout) <= 0)
	{
		return (ERROR);
	}

	if (FD_ISSET (sFd, &writeFds))
	{
		return(OK);
	}
	else
	{
		return(ERROR);
	}
}

static int32_t selectRead(int32_t sFd,int32_t ReadTimeout)
{
	struct              fd_set readFds;
	struct              timeval timeout;
	int32_t                    width;



	if(ReadTimeout>=1000)
	{
		timeout.tv_sec = ReadTimeout/1000;
        timeout.tv_usec = ReadTimeout%1000;
	}
	else
    {
        timeout.tv_sec = 0;
        timeout.tv_usec = ReadTimeout;
	}

	FD_ZERO (&readFds);//#define FD_ZERO(set) (((fd_set FAR *)(set))->fd_count=0)
	FD_SET (sFd, &readFds); //typedef struct fd_set FD_SET
	width = sFd+1;

	if (select (width, &readFds, (fd_set *)0, (fd_set *)0, &timeout) <= 0)
	{
		return (ERROR);
	}

	if (FD_ISSET (sFd, &readFds))//#define FD_ISSET(fd, set) __WSAFDIsSet((SOCKET)(fd), (fd_set FAR *)(set))
	{
		return(OK);
	}
	else
	{
		return(ERROR);
	}
}

/*static void PrintfMessage(const char* tag,unsigned char *buf, unsigned short len)
{
	int32_t i;
	unsigned char chaBuffer[10];
	unsigned char printBuffer[1024];
	unsigned short dataLen;
	unsigned short printLen;
	dataLen = len;
	printLen = dataLen * 3 + 1;

	rt_kprintf(tag);
	rt_kprintf(" ");//�ڱ�ǩ��Ӹ��ո�

	for(i = 0; i < dataLen; i++)
	{
		sprintf((char*)chaBuffer, "%02x", buf[i]);
		printBuffer[i * 3] = chaBuffer[0];
		printBuffer[i * 3 + 1] = chaBuffer[1];
		printBuffer[i * 3 + 2] = 0x20;//�ո�
	}
	i -= 1;
	printBuffer[i * 3 + 3] = 0x0a;//����

	for(i = 0; i < printLen; i++)
	{
		rt_kprintf("%c",printBuffer[i]);
	}
	rt_kprintf("\n");
}
*/
static int32_t centralSend(PARAMS *pParams)
{
	int32_t count = 0;
	int32_t  nLength = 0;
	APDU* pApdu;
	int32_t nSentLen = 0;

	char* pBuffer = (char*)(pParams->m_pSendMsg);

	TPORT_PARAM_STRUCT* pLinkPortParam;
	int32_t cannotwriteCnt = 0;

	TDateTime tTime;

	nLength = pBuffer[1]+2;

	pApdu = (APDU*)(pParams->m_pSendMsg);
	pApdu->m_APCI.m_wordSendNum = pParams->m_nSendNum;
	pApdu->m_APCI.m_wordRecvNum = pParams->m_nRecvNum;
  

	//���ͱ���
	while(nSentLen<nLength)
	{
		if(selectWrite(pParams->m_serverFd,WRITE_TIMEOUT) == OK)
		{
			count=send(pParams->m_serverFd,//send() is  lwip api
				((char *)(pParams->m_pSendMsg))+nSentLen,//���ͻ���������ָ�����ƫ��SendLen(�ѷ��ͳ���)
				(int32_t)(nLength-nSentLen),//���ͳ���Ϊ�ܳ��ȼ�ȥ�ѷ��ͳ���
				0);

			if((count<0) || (count==0 && errno!=EWOULDBLOCK))
			{
				logMsg("centralSend error-1,errno = %i!\n",errno,0,0,0,0,0);
				tTime = Now();
				logMsg("Time  = %d-%d-%d:%d:%d:%d!\n",tTime.month,tTime.day,tTime.hour,tTime.minute,tTime.msec/1000,tTime.msec%1000);//debug
				return HAVE_ERROR;
			}
			else if(count==0)
			{
				rt_thread_delay(10); // delay 10ms
			}
			else
			{
				nSentLen+=count;
			}
		}
		else
		{
			cannotwriteCnt++;
			if(cannotwriteCnt >= 3)//3s
			{
				logMsg("centralSend error-2!\n",0,0,0,0,0,0);
				tTime = Now();
				logMsg("Time  = %d-%d-%d:%d:%d:%d!\n",tTime.month,tTime.day,tTime.hour,tTime.minute,tTime.msec/1000,tTime.msec%1000);//debug
				return HAVE_ERROR;
			}
			continue;
		}
	}

	//��ӡ����
#ifdef MSG_BUFFER
	rt_kprintf("port%d--",pParams->m_nLinkSocket);
	PrintfMessage("[Send]",pBuffer,nLength);
#endif
	//���ļ���
	//logMsg("port%d[Send]--SN = %04x,RN = %04x\n",pParams->m_nLinkSocket,pParams->m_nSendNum,pParams->m_nRecvNum,0,0,0);
	eth_status.eth_trans_flg = 1;
	//eth_status.eth_trans_buf
	rt_memcpy(&eth_status.eth_trans_buf[10], pParams->m_pSendMsg,nSentLen);
    //wusenlin
	//ͳ����Ϣ
	pParams->m_nSendPacket++;
	pLinkPortParam = GetLinkPortParam( pParams->m_nLinkSocket );
	pLinkPortParam->nTxPacks = pParams->m_nSendPacket;

	pParams->m_nSendNum += 2;

	return(NO_ERROR);
}

static int32_t	centralRecv(PARAMS *pParams)
{
	int32_t recv_count=0;
	int32_t length = 0;
	int32_t index=2;
	uint8_t bCtrlMsg = FALSE;

	char* pBuffer = (char*)pParams->m_pRecvMsg;

	TPORT_PARAM_STRUCT* pLinkPortParam;
	TDateTime tTime;

	int32_t EAgainNum = 0;

	while(pBuffer[0] != 0x68)
	{
		recv_count=recv(pParams->m_serverFd,&pBuffer[0],2,0);//�Ȱ�ǰ�����ֽڽ�����
		/*
		//old
		if((recv_count==ERROR && (errno != EWOULDBLOCK )) || recv_count==0)
		{
			logMsg("centralRecv error-1,errno = %i!\n",errno,0,0,0,0,0);
			return(HAVE_ERROR);
		}
		if(recv_count<0)
		{
			rt_thread_delay(RT_TICK_PER_SECOND/100); // delay 10ms
			continue;
		}
		*/

        //////////////////////////////////////////////////////////////
		if(recv_count < 0)
        {
            // �����Ƿ�������ģʽ,���Ե�errnoΪEAGAIN��=EWOULDBLOCK��ʱ,��ʾ��ǰ�������������ݿɶ�
            if( (errno == EWOULDBLOCK)&&(EAgainNum < 3) )
            {
                rt_thread_delay(RT_TICK_PER_SECOND/100); // delay 10ms
                EAgainNum++;
                continue;
            }
            else
                return(HAVE_ERROR);
        }
        else if(recv_count == 0)
        {
            // �������ʾ�Զ˵�socket�������ر�.
            return(HAVE_ERROR);
        }
        ///////////////////////////////////////////////////////////////
#if 0
		if(recv_count==ERROR || recv_count==0)
		{
			logMsg("centralRecv error-1,errno = %i!\n",errno,0,0,0,0,0);
			tTime = Now();
			logMsg("Time  = %d-%d-%d:%d:%d:%d!\n",tTime.month,tTime.day,tTime.hour,tTime.minute,tTime.msec/1000,tTime.msec%1000);//debug
			return(HAVE_ERROR);
		}
#endif
	}

	length = pBuffer[1];
	EAgainNum = 0;

	while(length)
	{
		recv_count=recv(pParams->m_serverFd,&pBuffer[index],length,0);
		/*
		//old
		if ((recv_count==ERROR && (errno != EWOULDBLOCK )) || recv_count==0)
		{
			logMsg("centralRecv error-2,errno = %i!\n",errno,0,0,0,0,0);
			return(HAVE_ERROR);
		}
		if(recv_count<0)
		{
			rt_thread_delay(RT_TICK_PER_SECOND/100); // delay 10ms
			continue;
		}
		*/
		//////////////////////////////////////////////////////////////
		if(recv_count < 0)
        {
            // �����Ƿ�������ģʽ,���Ե�errnoΪEAGAIN��=EWOULDBLOCK��ʱ,��ʾ��ǰ�������������ݿɶ�
            if( (errno == EWOULDBLOCK)&&(EAgainNum < 3) )
            {
                rt_thread_delay(RT_TICK_PER_SECOND/100); // delay 10ms
                EAgainNum++;
                continue;
            }
            else
                return(HAVE_ERROR);
        }
        else if(recv_count == 0)
        {
            // �������ʾ�Զ˵�socket�������ر�.
            return(HAVE_ERROR);
        }
        ///////////////////////////////////////////////////////////////

#if 0
		if(recv_count==ERROR || recv_count==0)
		{
			logMsg("centralRecv error-2,errno = %i!\n",errno,0,0,0,0,0);
			tTime = Now();
			logMsg("Time  = %d-%d-%d:%d:%d:%d!\n",tTime.month,tTime.day,tTime.hour,tTime.minute,tTime.msec/1000,tTime.msec%1000);//debug
			return(HAVE_ERROR);
		}
#endif
		index += recv_count;
		length -= recv_count;
	}

	//У����պͷ������к�
	bCtrlMsg = (pParams->m_pRecvMsg->m_APCI.m_byteApduLength==4)?TRUE:FALSE;
	if( (!bCtrlMsg) && (pParams->m_pRecvMsg->m_APCI.m_wordSendNum > pParams->m_nRecvNum) )
	{
		logMsg("centralRecv error-3!\n",0,0,0,0,0,0);
		tTime = Now();
		logMsg("Time  = %d-%d-%d:%d:%d:%d!\n",tTime.month,tTime.day,tTime.hour,tTime.minute,tTime.msec/1000,tTime.msec%1000);//debug

		pParams->m_nErrPackets++;		//��¼����İ���
		return (HAVE_ERROR);
	}

	pParams->m_nRecvPacket++;				//��¼��ȷ�İ���

	if(!bCtrlMsg)
	{
		pParams->m_nRecvNum += 2;
	}

    if((pParams->m_pRecvMsg->m_APCI.m_wordSendNum & 0x03) != 0x03)//��U����
    {
        pParams->m_nConfirmedNum = pParams->m_pRecvMsg->m_APCI.m_wordRecvNum;
    }
//���ļ���
		eth_status.eth_recv_flg = 1;
		
		rt_memcpy(&eth_status.eth_recv_buf[10],pBuffer,pBuffer[1]+2);
	//��ӡ����
#ifdef MSG_BUFFER
	rt_kprintf("port%d--",pParams->m_nLinkSocket);
	PrintfMessage("[Recv]",pBuffer,pBuffer[1]+2);
#endif
	//logMsg("port%d[Recv]--SN = %04x,RN = %04x\n",pParams->m_nLinkSocket,pParams->m_pRecvMsg->m_APCI.m_wordSendNum,pParams->m_pRecvMsg->m_APCI.m_wordRecvNum,0,0,0);

	//wusenlin
	//��¼���յı��������ֽ���
	pLinkPortParam = GetLinkPortParam( pParams->m_nLinkSocket );
    pLinkPortParam->nRxPacks = pParams->m_nRecvPacket;

	return (NO_ERROR);
}

//���Ϳ�������Ϣ
static int32_t sendCtrlMsg(PARAMS* pParams)
{
	int32_t count=0;
	int32_t nSentLen = 0;
	int32_t cannotwriteCnt = 0;

	//char* pBuffer = (char*)(pParams->m_pCtrlMsg);
	TPORT_PARAM_STRUCT* pLinkPortParam;
	TDateTime tTime;

	//���ͱ���
	while(nSentLen<6)
	{
		if(selectWrite(pParams->m_serverFd,WRITE_TIMEOUT) == OK)
		{
			count=send(pParams->m_serverFd,
				((char *)(pParams->m_pCtrlMsg))+nSentLen,//���ͻ���������ָ�����ƫ��SendLen(�ѷ��ͳ���)
				(int32_t)(6-nSentLen),//���ͳ���Ϊ�ܳ��ȼ�ȥ�ѷ��ͳ���
				0);
			if ((count<0) || (count==0 && errno!=EWOULDBLOCK))
			{
				logMsg("sendCtrlMsg error-1,errno = %i!\n",errno,0,0,0,0,0);
				tTime = Now();
				logMsg("Time  = %d-%d-%d:%d:%d:%d!\n",tTime.month,tTime.day,tTime.hour,tTime.minute,tTime.msec/1000,tTime.msec%1000);//debug
				return HAVE_ERROR;
			}
			else if(count==0)
			{
				rt_thread_delay(RT_TICK_PER_SECOND/100); // delay 10ms
			}
			else
			{
				nSentLen+=count;
			}
		}
		else
		{
			cannotwriteCnt++;
			if(cannotwriteCnt >= 3)
			{
				logMsg("sendCtrlMsg error-2,errno = %i!\n",errno,0,0,0,0,0);
				tTime = Now();
				logMsg("Time  = %d-%d-%d:%d:%d:%d!\n",tTime.month,tTime.day,tTime.hour,tTime.minute,tTime.msec/1000,tTime.msec%1000);//debug
				return HAVE_ERROR;
			}
			continue;
		}
	}

	//��ӡ����
#ifdef MSG_BUFFER
	rt_kprintf("port%d--",pParams->m_nLinkSocket);
	PrintfMessage("[Send]",pBuffer,6);
#endif

	//wusenlin
	//ͳ����Ϣ
	pParams->m_nSendPacket++;
	pLinkPortParam = GetLinkPortParam( pParams->m_nLinkSocket );
	pLinkPortParam->nTxPacks = pParams->m_nSendPacket;

	return(NO_ERROR);
}

static int32_t ProcessAsduNoACK(PARAMS *pParams)
{
	memcpy(pParams->m_pSendMsg,pParams->m_pRecvMsg,pParams->m_pRecvMsg->m_APCI.m_byteApduLength+2);

	pParams->m_pSendMsg->m_wordCOT = 21;

	return centralSend(pParams);
}


//װ��ͨѶ״̬
static int32_t ProcessDeviceState(PARAMS *pParams, uint8_t ucTI, uint16_t uiCOT)
{
	int32_t		nIndex;
	APDU*	pSendMsg = pParams->m_pSendMsg;

    TDateTime dtime;
    uint16_t nMillis;

    if(pParams->m_severPort==ENGINEER_PORT)
    {
        pSendMsg->m_APCI.m_byteStartChar = 0x68;
        pSendMsg->m_byteType = ucTI;
        pSendMsg->m_byteVSQ = 0x81;
        pSendMsg->m_wordCOT = uiCOT;
        pSendMsg->m_byteCOMADD = 0;
        pSendMsg->m_byteADD = pParams->m_device_address;
        pSendMsg->m_msgdata[FUN_103] = 1;
        pSendMsg->m_msgdata[INF_103] = 192;

        nIndex = MSG_103;
        pSendMsg->m_msgdata[nIndex++] = 1;//ͨѶ���� ˫����Ϣ

        dtime = Now(); //��ȡ��ǰʱ��
        nMillis = dtime.msec + dtime.second*1000;
        pSendMsg->m_msgdata[nIndex++] = LOBYTE(nMillis);//������ֽ�
        pSendMsg->m_msgdata[nIndex++] = HIBYTE(nMillis);//������ֽ�
        pSendMsg->m_msgdata[nIndex++] = dtime.minute;//��
        pSendMsg->m_msgdata[nIndex++] = dtime.hour;//ʱ

        //������ϢSIN,�Է�����ʱ
        pSendMsg->m_msgdata[nIndex++] = pParams->m_pRecvMsg->m_msgdata[MSG_103];
        pSendMsg->m_APCI.m_byteApduLength = APDU_HEAD_LENGTH + nIndex;
    }
    else if(pParams->m_severPort==OPERATOR_PORT)
    {
        pSendMsg->m_APCI.m_byteStartChar = 0x68;
        pSendMsg->m_byteType = ucTI;
        pSendMsg->m_byteVSQ = 0x01;
        pSendMsg->m_wordCOT = uiCOT;
        pSendMsg->m_byteCOMADD = 0;
        pSendMsg->m_byteADD = pParams->m_device_address;
        pSendMsg->m_msgdata[INF_104L] = 0x01;
        pSendMsg->m_msgdata[INF_104M] = 0x00;
        pSendMsg->m_msgdata[INF_104H] = 0x00;

        nIndex = MSG_104;
        pSendMsg->m_msgdata[nIndex++] = 0x00;//ͨѶ����(����)
        if(uiCOT == IEC104_COT_M_spont) //�Է�
        {
            pSendMsg->m_byteVSQ |= 0x80;

            dtime = Now(); //��ȡ��ǰʱ��
            nMillis = dtime.msec + dtime.second*1000;
            pSendMsg->m_msgdata[nIndex++] = LOBYTE(nMillis);//������ֽ�
            pSendMsg->m_msgdata[nIndex++] = HIBYTE(nMillis);//������ֽ�
            pSendMsg->m_msgdata[nIndex++] = dtime.minute;//��
            pSendMsg->m_msgdata[nIndex++] = dtime.hour;//ʱ
            pSendMsg->m_msgdata[nIndex++] = dtime.day;
            pSendMsg->m_msgdata[nIndex++] = dtime.month;
            pSendMsg->m_msgdata[nIndex++] = dtime.year;
        }
        pSendMsg->m_APCI.m_byteApduLength = APDU_HEAD_LENGTH + nIndex;
    }

    return centralSend(pParams);
}

static int32_t ProcessAsdu05(PARAMS *pParams)
{
	char DeviceName[10];

	APDU*	pSendMsg = pParams->m_pSendMsg;

	pSendMsg->m_APCI.m_byteStartChar = 0x68;
	pSendMsg->m_byteType = 0x05;
	pSendMsg->m_byteVSQ = 0x81;
	pSendMsg->m_wordCOT = 5;
	//pSendMsg->m_bytePRM = 0;
	pSendMsg->m_byteCOMADD = PROTECT_COMADDR;
	pSendMsg->m_byteADD = pParams->m_device_address;
	pSendMsg->m_msgdata[FUN_103] = PROTECT_FUN;
	pSendMsg->m_msgdata[INF_103] = 4;

	if(pParams->m_pRecvMsg->m_byteType == 60)
    {
        pSendMsg->m_wordCOT = 64;
        pSendMsg->m_msgdata[INF_103] = 6;
    }

	pSendMsg->m_msgdata[MSG_103] = 2;  //COL
	strcpy(DeviceName,DEVICE_NAME);
	memcpy(&pSendMsg->m_msgdata[3], DeviceName, DEVICE_NAME_LENGTH);
	pSendMsg->m_msgdata[11] = LOBYTE(DEVICE_VER);     //verL
	pSendMsg->m_msgdata[12] = HIBYTE(DEVICE_VER);    //verH
pSendMsg->m_msgdata[13] = LOBYTE(DEVICE_CRC);    //crcL
pSendMsg->m_msgdata[14] = HIBYTE(DEVICE_CRC);    //crcH

	pSendMsg->m_APCI.m_byteApduLength = APDU_HEAD_LENGTH + 15;
	return centralSend(pParams);
}
extern uint8_t index_trans_yc;
extern int16_t yc_message_buf[];
extern void rtdb_modbusto104(uint8_t ask_data);
static int32_t ProcessSendYC(PARAMS *pParams,uint8_t nFUN, uint8_t nCOT) //ASDU_50 ASDU_9
{
	int32_t i,nindex;
	TMeasure103* pSendMeas;

	uint8_t MeasValCnt;
	uint8_t QDS103,QDS104;

	APDU*	pSendMsg = pParams->m_pSendMsg;

	
  OP_GetMeasureValue();
  
  /*if(OP_GetMeasureValue(&(pSendMsg->m_msgdata[0]), &nNum, pParams->m_device_address) == HAVE_ERROR)
					  {
							  return HAVE_ERROR;
						} */
	
	pSendMsg->m_byteType = nFUN;
	pSendMsg->m_wordCOT = nCOT;

	pSendMsg->m_byteCOMADD = MONITOR_COMADDR;
 
  pSendMsg->m_byteADD = yc_message_buf[0]&0xff;
  MeasValCnt = yc_message_buf[1]&0xff;
	/*if(nFUN == 50)// no use
    {
        pSendMsg->m_msgdata[FUN_103] = MONITOR_FUN;
        if(nCOT==2)//ѭ������
        {
            pSendMsg->m_byteVSQ = MeasValCnt;
            pSendMsg->m_msgdata[INF_103] = pSendMeas->measID;
            nindex = MSG_103;
            for(i=0; i<MeasValCnt; i++,pSendMeas++)
            {
                pSendMsg->m_msgdata[nindex++] = LOBYTE(pSendMeas->measVal);
                pSendMsg->m_msgdata[nindex++] = HIBYTE(pSendMeas->measVal);
                pSendMeas->bAutoSendFlg &= ~(0x01<<pParams->m_nLinkSocket);//��ͻ�����ͱ��
            }
        }
        else    //ͻ������
        {
            nindex = MSG_103-1;
            for(i=0; i<MeasValCnt; i++,pSendMeas++)
            {
                if(pSendMeas->bAutoSendFlg&(0x01<<pParams->m_nLinkSocket))
                {
                    pSendMsg->m_msgdata[nindex++] = pSendMeas->measID;
                    pSendMsg->m_msgdata[nindex++] = LOBYTE(pSendMeas->measVal);
                    pSendMsg->m_msgdata[nindex++] = HIBYTE(pSendMeas->measVal);
                    pSendMeas->bAutoSendFlg &= ~(0x01<<pParams->m_nLinkSocket);//��ͻ�����ͱ��
                }
            }
            pSendMsg->m_byteVSQ = (nindex-1)/3|0x80;
        }
    }
    else*/ if(nFUN == 9)//ң�����ݲ���ѭ�����ͣ�����ͻ��
    {
        if(nCOT==1)//ѭ������
        {
            nindex = MSG_104;
            pSendMsg->m_byteVSQ = MeasValCnt|0x80;
            pSendMsg->m_msgdata[INF_104L] = 0x01;//pSendMeas->measID-91;   //103��92��ʼ 104��0x4001��ʼ
            pSendMsg->m_msgdata[INF_104M] = 0x40;
            pSendMsg->m_msgdata[INF_104H] = 0x00;
		

					  for(i=0; i<MeasValCnt; i++)//MeasValCnt
            {
                pSendMsg->m_msgdata[nindex++] = LOBYTE(yc_message_buf[i+2])&0xff;
                pSendMsg->m_msgdata[nindex++] = HIBYTE(yc_message_buf[i+2]);
							QDS103 = LOBYTE(yc_message_buf[i+2])&0x07;
							  
                
                QDS104 = 0;
                if(QDS103&0x01)//���λ//
                {
                    QDS104 |= 0x01;
                }
                if(QDS103&0x02)//��Чλ///
                {
                    QDS104 |= 0x80;
                }
                if(QDS103&0x04)///��103�ı���λ��Ϊ101��ȡ��λ//
                {
                    QDS104 |= 0x10;
                }
                pSendMsg->m_msgdata[nindex++] = QDS104;
                pSendMeas->bAutoSendFlg &= ~(0x01<<pParams->m_nLinkSocket);//��ͻ�����ͱ��
            }
						
        }
        else if(nCOT==3)//ͻ������// no use
        {
            nindex = INF_104L;
            for(i=0; (i<MeasValCnt)&&(i<40); i++,pSendMeas++)
            {
                if(pSendMeas->bAutoSendFlg&(0x01<<pParams->m_nLinkSocket))
                {
                    pSendMsg->m_msgdata[nindex++] = pSendMeas->measID-91;   //0x4001��ʼ
                    pSendMsg->m_msgdata[nindex++] = 0x40;
                    pSendMsg->m_msgdata[nindex++] = 0x00;
                    pSendMsg->m_msgdata[nindex++] = LOBYTE(pSendMeas->measVal)&0xff;
                    pSendMsg->m_msgdata[nindex++] = HIBYTE(pSendMeas->measVal);
                    QDS103 = LOBYTE(pSendMeas->measVal)&0x07;
                    QDS104 = 0;
                    if(QDS103&0x01)/*���λ*/
                    {
                        QDS104 |= 0x01;
                    }
                    if(QDS103&0x02)/*��Чλ*/
                    {
                        QDS104 |= 0x80;
                    }
                    if(QDS103&0x04)/*��103�ı���λ��Ϊ101��ȡ��λ*/
                    {
                        QDS104 |= 0x10;
                    }
                    pSendMsg->m_msgdata[nindex++] = QDS104;
                    pSendMeas->bAutoSendFlg &= ~(0x01<<pParams->m_nLinkSocket);//��ͻ�����ͱ��
                }
            }
            pSendMsg->m_byteVSQ = nindex/6;
        }
    }

	pSendMsg->m_APCI.m_byteStartChar = 0x68;
	pSendMsg->m_APCI.m_byteApduLength = APDU_HEAD_LENGTH + nindex;

	return centralSend(pParams);
  
}

//�ж��Ƿ���SOE
static int32_t JudgeNewSOE(PARAMS *pParams)
{
	uint8_t	soeCount = 0;          //��ѯ�õ���SOE����
	int8_t	soeType  = -1;         //SOE����

	OP_GetSOEQueueSendType(pParams->m_nLinkSocket, &soeType, &soeCount);

	if(soeCount)
	{
		pParams->soeInfo.soeCount = soeCount;
		pParams->soeInfo.soeType = soeType;

/*		if(soeType == SOE_WAVE)
		{
			if(pParams->waveTransferringFlag == TRUE) //����¼�������в��ٴ����µ�¼��
			{
				pParams->soeInfo.soeCount = 0;
                pParams->soeInfo.soeType = -1;
				return(ERROR);
			}
			else
			{
				pWaveDesc = OPWAVE_GetCommWaveDescInfo(pParams->m_nLinkSocket, 1);
				if((pWaveDesc->nWaveSize + 53) > 200000)//200K
				{
					logMsg("SOE_WAVE-----confirmed-----wave length > 200K,m_nLinkSocket=%d!\n",pParams->m_nLinkSocket,0,0,0,0,0);
					OP_SendSomeSOE(pParams->m_nLinkSocket, SOE_WAVE, 1);
					return(ERROR);
				}

				pParams->soeInfo.soeCount = soeCount;
				pParams->soeInfo.soeType = soeType;
				return(OK);
			}
		}*/
		return OK;
	}
	return(ERROR);
}

//�������� -103
static int32_t ProcessAutoAction_103(PARAMS *pParams)
{
	TSOE_ACTION_STRUCT *pActionSOE;

	int32_t		nIndex = MSG_103;
	APDU*	pSendMsg = pParams->m_pSendMsg;
	int32_t		nMillis = 0;
	int32_t		nError = NO_ERROR;
	int32_t		i, j;
	float   fResultValue;
	uint8_t*   pnResultValue;

	OP_GetpSOEQueue(pParams->m_nLinkSocket, SOE_ACTION, (void**)&pActionSOE);

	pSendMsg->m_byteType = 0x02;
	pSendMsg->m_byteVSQ = 0x81;
	pSendMsg->m_wordCOT = 1;
	//pSendMsg->m_bytePRM = 0;
	pSendMsg->m_byteCOMADD = PROTECT_COMADDR;
	pSendMsg->m_byteADD = pParams->m_device_address;
	pSendMsg->m_msgdata[FUN_103] = PROTECT_FUN;
	pSendMsg->m_msgdata[INF_103] = pActionSOE->n103Inf;//D_ProtAction_Define[pActionSOE->nDispID].n103Inf;

	pSendMsg->m_msgdata[nIndex++] = pActionSOE->nDPI&0x03;//˫����Ϣ

	//���ʱ��
	pSendMsg->m_msgdata[nIndex++] = LOBYTE(pActionSOE->nPosTime);//���ֽ�
	pSendMsg->m_msgdata[nIndex++] = HIBYTE(pActionSOE->nPosTime);//���ֽ�

	//�������
	pSendMsg->m_msgdata[nIndex++] = LOBYTE(pActionSOE->nFAN);//���ֽ�
	pSendMsg->m_msgdata[nIndex++] = HIBYTE(pActionSOE->nFAN);//���ֽ�

	//��ȡʱ��
	nMillis = pActionSOE->dtTime.msec + pActionSOE->dtTime.second*1000;
	pSendMsg->m_msgdata[nIndex++] = LOBYTE(nMillis);//������ֽ�
	pSendMsg->m_msgdata[nIndex++] = HIBYTE(nMillis);//������ֽ�
	pSendMsg->m_msgdata[nIndex++] = pActionSOE->dtTime.minute;//��
	pSendMsg->m_msgdata[nIndex++] = pActionSOE->dtTime.hour;//ʱ
	pSendMsg->m_msgdata[nIndex++] = 0;//SIN

	pSendMsg->m_APCI.m_byteApduLength = APDU_HEAD_LENGTH + nIndex;
	nError |=  centralSend(pParams);

	pParams->soeCnt++;
	logMsg("SOE---ASDU2-------,busNo=%d,soeCnt=%d!\n",pParams->m_nLinkSocket,pParams->soeCnt,0,0,0,0);

	if((pActionSOE->nDPI&0x02) && pActionSOE->nResultCnt)
	{
		pSendMsg->m_byteType = 70;
        pSendMsg->m_byteVSQ = pActionSOE->nResultCnt+1;
		nIndex = 11;
        pSendMsg->m_msgdata[nIndex++] = pActionSOE->dtTime.day;//��
        pSendMsg->m_msgdata[nIndex++] = pActionSOE->dtTime.month;//��
        pSendMsg->m_msgdata[nIndex++] = pActionSOE->dtTime.year;//��
		pSendMsg->m_msgdata[nIndex++] = 0;//SIN
		pSendMsg->m_msgdata[nIndex++] = 0;//��������

		for(i=0; i<pActionSOE->nResultCnt; i++)
		{
/*			pSendMsg->m_msgdata[nIndex++] = pActionSOE->ResultVal[i].measID + 1; //��1��ʼ
			pSendMsg->m_msgdata[nIndex++] = LOBYTE(pActionSOE->ResultVal[i].measVal);
			pSendMsg->m_msgdata[nIndex++] = HIBYTE(pActionSOE->ResultVal[i].measVal);
*/
			/*if(pActionSOE->n103Inf == 111)//low Fjg
				fResultValue = (float)pActionSOE->ResultVal[i]/100.00;
			else*/
                fResultValue = (float)pActionSOE->ResultVal[i]/1000.0;
			pnResultValue = (uint8_t*) &fResultValue;
			for(j=0; j<4; j++)  //R32.23
                pSendMsg->m_msgdata[nIndex++]= *pnResultValue++;
		}

		pSendMsg->m_APCI.m_byteApduLength = APDU_HEAD_LENGTH + nIndex;
		nError |=  centralSend(pParams);
//		pParams->soeCnt++;
		logMsg("SOE---ASDU70-------,busNo=%d,soeCnt=%d!\n",pParams->m_nLinkSocket,pParams->soeCnt,0,0,0,0);
	}

	return (nError&HAVE_ERROR)?HAVE_ERROR:NO_ERROR;
}
//�������� -104
static int32_t ProcessAutoAction_104(PARAMS *pParams)
{
	TSOE_ACTION_STRUCT *pActionSOE;

	int32_t		nIndex;
	APDU*	pSendMsg = pParams->m_pSendMsg;
	int32_t		nMillis = 0;
	int32_t		nError = NO_ERROR;
	int32_t		i, j;
	float   fResultValue;
	uint8_t*   pnResultValue;

	OP_GetpSOEQueue(pParams->m_nLinkSocket, SOE_ACTION, (void**)&pActionSOE);

	pSendMsg->m_byteType = IEC104_TI_M_EP_TD_1;
	pSendMsg->m_byteVSQ = 1;
	pSendMsg->m_wordCOT = IEC104_COT_M_spont;
	pSendMsg->m_byteCOMADD = PROTECT_COMADDR;
	pSendMsg->m_byteADD = pParams->m_device_address;
    nIndex = MSG_104;
    pSendMsg->m_msgdata[INF_104L] = pActionSOE->n103Inf+1;
    pSendMsg->m_msgdata[INF_104M] = 0x10;
    pSendMsg->m_msgdata[INF_104H] = 0;
    pSendMsg->m_msgdata[nIndex++] = pActionSOE->nDPI&0x03;//˫����Ϣ
	//���ʱ��Relative time
	pSendMsg->m_msgdata[nIndex++] = LOBYTE(pActionSOE->nPosTime);//���ֽ�
	pSendMsg->m_msgdata[nIndex++] = HIBYTE(pActionSOE->nPosTime);//���ֽ�
    //��ȡʱ��
    nMillis = pActionSOE->dtTime.msec + pActionSOE->dtTime.second*1000;
    pSendMsg->m_msgdata[nIndex++] = LOBYTE(nMillis);//������ֽ�
    pSendMsg->m_msgdata[nIndex++] = HIBYTE(nMillis);//������ֽ�
    pSendMsg->m_msgdata[nIndex++] = pActionSOE->dtTime.minute;//��
    pSendMsg->m_msgdata[nIndex++] = pActionSOE->dtTime.hour;//ʱ
    pSendMsg->m_msgdata[nIndex++] = pActionSOE->dtTime.day;
    pSendMsg->m_msgdata[nIndex++] = pActionSOE->dtTime.month;
    pSendMsg->m_msgdata[nIndex++] = pActionSOE->dtTime.year;

	pSendMsg->m_APCI.m_byteApduLength = APDU_HEAD_LENGTH + nIndex;
	nError |=  centralSend(pParams);

	pParams->soeCnt++;
	logMsg("SOE---ASDU38-------,busNo=%d,soeCnt=%d!\n",pParams->m_nLinkSocket,pParams->soeCnt,0,0,0,0);

	if((pActionSOE->nDPI&0x02) && pActionSOE->nResultCnt)
	{
		pSendMsg->m_byteType = 166;
		nIndex = 13;
		pSendMsg->m_msgdata[nIndex++] = pActionSOE->nResultCnt;

		for(i=0; i<pActionSOE->nResultCnt; i++)
		{
			/*if(pActionSOE->n103Inf == 111)//low Fjg
				fResultValue = (float)pActionSOE->ResultVal[i]/100.00;
			else*/
                fResultValue = (float)pActionSOE->ResultVal[i]/1000.0;
			pnResultValue = (uint8_t*) &fResultValue;
			for(j=0; j<4; j++)  //R32.23
                pSendMsg->m_msgdata[nIndex++]= *pnResultValue++;
		}

		pSendMsg->m_APCI.m_byteApduLength = APDU_HEAD_LENGTH + nIndex;
		nError |=  centralSend(pParams);
//		pParams->soeCnt++;
		logMsg("SOE---ASDU166-------,busNo=%d,soeCnt=%d!\n",pParams->m_nLinkSocket,pParams->soeCnt,0,0,0,0);
	}

	return (nError&HAVE_ERROR)?HAVE_ERROR:NO_ERROR;
}

//�����澯 -103
static int32_t ProcessAutoWarning_103(PARAMS *pParams)
{
	TSOE_WARNING_STRUCT *pWarningSOE;

	int32_t		nIndex = MSG_103;
	APDU*	pSendMsg = pParams->m_pSendMsg;
	int32_t		nMillis = 0;

	OP_GetpSOEQueue(pParams->m_nLinkSocket, SOE_WARNING, (void**)&pWarningSOE);

    pSendMsg->m_byteType = 0x01;
    pSendMsg->m_byteVSQ = 0x81;
    pSendMsg->m_wordCOT = 1;
    pSendMsg->m_byteCOMADD = PROTECT_COMADDR;
    pSendMsg->m_byteADD = pParams->m_device_address;
    pSendMsg->m_msgdata[FUN_103] = PROTECT_FUN;
    pSendMsg->m_msgdata[INF_103] = pWarningSOE->n103Inf;

    pSendMsg->m_msgdata[nIndex++] = pWarningSOE->nDPI&0x03;//˫����Ϣ
    //��ȡʱ��
    nMillis = pWarningSOE->dtTime.msec + pWarningSOE->dtTime.second*1000;
    pSendMsg->m_msgdata[nIndex++] = LOBYTE(nMillis);//������ֽ�
    pSendMsg->m_msgdata[nIndex++] = HIBYTE(nMillis);//������ֽ�
    pSendMsg->m_msgdata[nIndex++] = pWarningSOE->dtTime.minute;//��
    pSendMsg->m_msgdata[nIndex++] = pWarningSOE->dtTime.hour;//ʱ

    pSendMsg->m_msgdata[nIndex++] = 0;//������ϢSIN,�Է�����ʱ
    pSendMsg->m_APCI.m_byteApduLength = APDU_HEAD_LENGTH + nIndex;

	pParams->soeCnt++;
	logMsg("SOE---ASDU1-------,busNo=%d,soeCnt=%d!\n",pParams->m_nLinkSocket,pParams->soeCnt,0,0,0,0);

	//���ͱ���
	return centralSend(pParams);
}

//�����澯 -104
static int32_t ProcessAutoWarning_104(PARAMS *pParams)
{
	TSOE_WARNING_STRUCT *pWarningSOE;

	int32_t		nIndex;
	APDU*	pSendMsg = pParams->m_pSendMsg;
	int32_t		nMillis = 0;

	OP_GetpSOEQueue(pParams->m_nLinkSocket, SOE_WARNING, (void**)&pWarningSOE);

    pSendMsg->m_byteType = IEC104_TI_M_DP_TB_1;
    pSendMsg->m_byteVSQ = 1;
    pSendMsg->m_wordCOT = IEC104_COT_M_spont;
    pSendMsg->m_byteCOMADD = PROTECT_COMADDR;
    pSendMsg->m_byteADD = pParams->m_device_address;
    nIndex = MSG_104;
    pSendMsg->m_msgdata[INF_104L] = pWarningSOE->n103Inf+1;
    pSendMsg->m_msgdata[INF_104M] = 0;
    pSendMsg->m_msgdata[INF_104H] = 0;
    pSendMsg->m_msgdata[nIndex++] = pWarningSOE->nDPI&0x03;//˫����Ϣ
    //��ȡʱ��
    nMillis = pWarningSOE->dtTime.msec + pWarningSOE->dtTime.second*1000;
    pSendMsg->m_msgdata[nIndex++] = LOBYTE(nMillis);//������ֽ�
    pSendMsg->m_msgdata[nIndex++] = HIBYTE(nMillis);//������ֽ�
    pSendMsg->m_msgdata[nIndex++] = pWarningSOE->dtTime.minute;//��
    pSendMsg->m_msgdata[nIndex++] = pWarningSOE->dtTime.hour;//ʱ
    pSendMsg->m_msgdata[nIndex++] = pWarningSOE->dtTime.day;
    pSendMsg->m_msgdata[nIndex++] = pWarningSOE->dtTime.month;
    pSendMsg->m_msgdata[nIndex++] = pWarningSOE->dtTime.year;

    pSendMsg->m_APCI.m_byteApduLength = APDU_HEAD_LENGTH + nIndex;

	pParams->soeCnt++;
	logMsg("SOE---ASDU31-------,busNo=%d,soeCnt=%d!\n",pParams->m_nLinkSocket,pParams->soeCnt,0,0,0,0);

	//���ͱ���
	return centralSend(pParams);
}

//����ң�ű�λ -103
static int32_t ProcessAutoDIEvent_103(PARAMS *pParams)
{
	TSOE_DIEVENT_STRUCT* pDievent;

	int32_t		nIndex = MSG_103;
	APDU*	pSendMsg = pParams->m_pSendMsg;
	int32_t		nMillis=0;
	int32_t     nNum;
	int32_t		i;

	nNum = pParams->soeInfo.soeCount;

	OP_GetpSOEQueue(pParams->m_nLinkSocket, SOE_DIEVENT, (void**)&pDievent);

    if(pDievent->n103Inf == 64) //����ѹ��
    {
        pSendMsg->m_byteType = 0x01;
        pSendMsg->m_byteVSQ = 0x81;
        pSendMsg->m_wordCOT = 1;
        //pSendMsg->m_bytePRM = 0;
        pSendMsg->m_byteCOMADD = MAINTENANCE_COMADDR;
        pSendMsg->m_byteADD = pParams->m_device_address;
        pSendMsg->m_msgdata[FUN_103] = PROTECT_FUN;
        pSendMsg->m_msgdata[INF_103] = pDievent->n103Inf;
        pSendMsg->m_msgdata[nIndex++] = pDievent->nSPI+1;//˫����Ϣ

        //��ȡʱ��
        nMillis = pDievent->dtTime.msec + pDievent->dtTime.second*1000;
        pSendMsg->m_msgdata[nIndex++] = LOBYTE(nMillis);//������ֽ�
        pSendMsg->m_msgdata[nIndex++] = HIBYTE(nMillis);//������ֽ�
        pSendMsg->m_msgdata[nIndex++] = pDievent->dtTime.minute;//��
        pSendMsg->m_msgdata[nIndex++] = pDievent->dtTime.hour;//ʱ

        pSendMsg->m_msgdata[nIndex++] = 0;  //SIN
        pSendMsg->m_APCI.m_byteApduLength = APDU_HEAD_LENGTH + nIndex;
        pParams->soeCnt++;
        logMsg("SOE---ASDU1-------,busNo=%d,soeCnt=%d!\n",pParams->m_nLinkSocket,pParams->soeCnt,0,0,0,0);

        //���ͱ���
        return centralSend(pParams);
    }
    else
    {
        pSendMsg->m_byteType = 0x29;
        pSendMsg->m_byteVSQ = nNum | 0x80;
        pSendMsg->m_wordCOT = 1;
        //pSendMsg->m_bytePRM = 0;
        pSendMsg->m_byteCOMADD = MONITOR_COMADDR;
        pSendMsg->m_byteADD = pParams->m_device_address;
        pSendMsg->m_msgdata[FUN_103] = MONITOR_FUN;
        pSendMsg->m_msgdata[INF_103] = pDievent->n103Inf;
        pSendMsg->m_msgdata[nIndex++] = pDievent->nSPI;//������Ϣ

        //��ȡʱ��
        nMillis = pDievent->dtTime.msec + pDievent->dtTime.second*1000;
        pSendMsg->m_msgdata[nIndex++] = LOBYTE(nMillis);//������ֽ�
        pSendMsg->m_msgdata[nIndex++] = HIBYTE(nMillis);//������ֽ�
        pSendMsg->m_msgdata[nIndex++] = pDievent->dtTime.minute;//��
        pSendMsg->m_msgdata[nIndex++] = pDievent->dtTime.hour;//ʱ
    }

    for(i = 1; i < nNum; i++)
    {
        pDievent++;
        if(pDievent > OP_GetSOEQRear(SOE_DIEVENT))
        {
            pDievent = OP_GetSOEQHead(SOE_DIEVENT);
        }

        while( !(pDievent->nValid & 0x01) )
        {
            pDievent++;
            if(pDievent > OP_GetSOEQRear(SOE_DIEVENT))
            {
                pDievent = OP_GetSOEQHead(SOE_DIEVENT);
            }
        }
        pSendMsg->m_msgdata[nIndex++] = pDievent->n103Inf;
        pSendMsg->m_msgdata[nIndex++] = pDievent->nSPI;//������Ϣ

        //��ȡʱ��
        nMillis = pDievent->dtTime.msec + pDievent->dtTime.second*1000;
        pSendMsg->m_msgdata[nIndex++] = LOBYTE(nMillis);//������ֽ�
        pSendMsg->m_msgdata[nIndex++] = HIBYTE(nMillis);//������ֽ�
        pSendMsg->m_msgdata[nIndex++] = pDievent->dtTime.minute;//��
        pSendMsg->m_msgdata[nIndex++] = pDievent->dtTime.hour;//ʱ
    }
	pSendMsg->m_msgdata[nIndex++] = 0;  //SIN
	pSendMsg->m_APCI.m_byteApduLength = APDU_HEAD_LENGTH + nIndex;
	pParams->soeCnt++;
	logMsg("SOE---ASDU41-------,busNo=%d,soeCnt=%d!\n",pParams->m_nLinkSocket,pParams->soeCnt,0,0,0,0);

	//���ͱ���
	return centralSend(pParams);
}

//����ң�ű�λ -104
static int32_t ProcessAutoDIEvent_104(PARAMS *pParams)
{
	TSOE_DIEVENT_STRUCT* pDievent;

	int32_t		nIndex = MSG_103;
	APDU*	pSendMsg = pParams->m_pSendMsg;
	int32_t		nMillis=0;
	int32_t     nNum;
	int32_t		i;

	nNum = pParams->soeInfo.soeCount;

	OP_GetpSOEQueue(pParams->m_nLinkSocket, SOE_DIEVENT, (void**)&pDievent);

    /*if(pDievent->n103Inf == 64) //����ѹ��
    {
        pSendMsg->m_byteType = IEC104_TI_M_DP_TB_1;
        pSendMsg->m_byteVSQ = 1;
        pSendMsg->m_wordCOT = IEC104_COT_M_spont;
        pSendMsg->m_byteCOMADD = MAINTENANCE_COMADDR;
        pSendMsg->m_byteADD = pDievent->ndeviceaddr;//pParams->m_device_address;//whs
        nIndex = MSG_104;
        pSendMsg->m_msgdata[INF_104L] = pDievent->n103Inf+1;
        pSendMsg->m_msgdata[INF_104M] = 0;
        pSendMsg->m_msgdata[INF_104H] = 0;
        pSendMsg->m_msgdata[nIndex++] = pDievent->nSPI+1;//˫����Ϣ
        //��ȡʱ��
        nMillis = pDievent->dtTime.msec + pDievent->dtTime.second*1000;
        pSendMsg->m_msgdata[nIndex++] = LOBYTE(nMillis);//������ֽ�
        pSendMsg->m_msgdata[nIndex++] = HIBYTE(nMillis);//������ֽ�
        pSendMsg->m_msgdata[nIndex++] = pDievent->dtTime.minute;//��
        pSendMsg->m_msgdata[nIndex++] = pDievent->dtTime.hour;//ʱ
        pSendMsg->m_msgdata[nIndex++] = pDievent->dtTime.day;
        pSendMsg->m_msgdata[nIndex++] = pDievent->dtTime.month;
        pSendMsg->m_msgdata[nIndex++] = pDievent->dtTime.year;
        pSendMsg->m_APCI.m_byteApduLength = APDU_HEAD_LENGTH + nIndex;

        //���ͱ���
        return centralSend(pParams);
    }
    else
    {*/
        pSendMsg->m_byteType = IEC104_TI_M_SP_TB_1;
        pSendMsg->m_byteVSQ = nNum;
        pSendMsg->m_wordCOT = IEC104_COT_M_spont;
        pSendMsg->m_byteCOMADD = MONITOR_COMADDR;
        pSendMsg->m_byteADD = pDievent->ndeviceaddr;//pParams->m_device_address;//whs
        nIndex = 0;
        for(i = 0; i < nNum; i++)
        {
            pSendMsg->m_msgdata[nIndex++] = pDievent->n103Inf-148;
            pSendMsg->m_msgdata[nIndex++] = 0;
            pSendMsg->m_msgdata[nIndex++] = 0;
            pSendMsg->m_msgdata[nIndex++] = pDievent->nSPI;//������Ϣ

            //��ȡʱ��
            nMillis = pDievent->dtTime.msec + pDievent->dtTime.second*1000;
            pSendMsg->m_msgdata[nIndex++] = LOBYTE(nMillis);//������ֽ�
            pSendMsg->m_msgdata[nIndex++] = HIBYTE(nMillis);//������ֽ�
            pSendMsg->m_msgdata[nIndex++] = pDievent->dtTime.minute;//��
            pSendMsg->m_msgdata[nIndex++] = pDievent->dtTime.hour;//ʱ
            pSendMsg->m_msgdata[nIndex++] = pDievent->dtTime.day;
            pSendMsg->m_msgdata[nIndex++] = pDievent->dtTime.month;
            pSendMsg->m_msgdata[nIndex++] = pDievent->dtTime.year;

            pDievent++;
            if(pDievent > OP_GetSOEQRear(SOE_DIEVENT))
            {
                pDievent = OP_GetSOEQHead(SOE_DIEVENT);
            }

            while( !(pDievent->nValid & 0x01) )
            {
                pDievent++;
                if(pDievent > OP_GetSOEQRear(SOE_DIEVENT))
                {
                    pDievent = OP_GetSOEQHead(SOE_DIEVENT);
                }
            }
        }
        pSendMsg->m_APCI.m_byteApduLength = APDU_HEAD_LENGTH + nIndex;

        //���ͱ���
        return centralSend(pParams);
    //}
}
//���ź�+ѹ���λ+�Ե�+�������� -103
static int32_t ProcessAutoGeneral_103(PARAMS *pParams)
{
	TSOE_GENERAL_STRUCT *pGeneralSOE;

	int32_t		nIndex = MSG_103;
	APDU*	pSendMsg = pParams->m_pSendMsg;
	int32_t		nMillis = 0;

	OP_GetpSOEQueue(pParams->m_nLinkSocket, SOE_GENERAL,(void**)&pGeneralSOE);

	pSendMsg->m_byteType = pGeneralSOE->nASDU;
	pSendMsg->m_byteVSQ = 0x81;
	pSendMsg->m_wordCOT = pGeneralSOE->nCOT;
	//pSendMsg->m_bytePRM = 0;
	pSendMsg->m_byteADD = pParams->m_device_address;
	if(pGeneralSOE->nASDU == 41)
    {
        pSendMsg->m_byteCOMADD = MONITOR_COMADDR;
        pSendMsg->m_msgdata[FUN_103] = MONITOR_FUN;
    }
    else
    {
        pSendMsg->m_byteCOMADD = PROTECT_COMADDR;
        pSendMsg->m_msgdata[FUN_103] = PROTECT_FUN;
    }
    if((pGeneralSOE->nASDU == 1)&&(pGeneralSOE->n103Inf == 64))  //����ѹ�� ������0
    {
        pSendMsg->m_byteCOMADD = MAINTENANCE_COMADDR;
    }

	pSendMsg->m_msgdata[INF_103] = pGeneralSOE->n103Inf;

	pSendMsg->m_msgdata[nIndex++] = pGeneralSOE->nDPI&0x03;//˫����Ϣ

    if(pGeneralSOE->nASDU == 2)//���ʱ�� �������
    {
        pSendMsg->m_msgdata[nIndex++] = 0;
        pSendMsg->m_msgdata[nIndex++] = 0;
        pSendMsg->m_msgdata[nIndex++] = 0;
        pSendMsg->m_msgdata[nIndex++] = 0;
    }
	//��ȡʱ��
	nMillis = pGeneralSOE->dtTime.msec + pGeneralSOE->dtTime.second*1000;
	pSendMsg->m_msgdata[nIndex++] = LOBYTE(nMillis);//������ֽ�
	pSendMsg->m_msgdata[nIndex++] = HIBYTE(nMillis);//������ֽ�
	pSendMsg->m_msgdata[nIndex++] = pGeneralSOE->dtTime.minute;//��
	pSendMsg->m_msgdata[nIndex++] = pGeneralSOE->dtTime.hour;//ʱ

	pSendMsg->m_msgdata[nIndex++] = 0;//������ϢSIN,�Է�����ʱ
	pSendMsg->m_APCI.m_byteApduLength = APDU_HEAD_LENGTH + nIndex;

	pParams->soeCnt++;
	logMsg("SOE---ASDU1-------,busNo=%d,soeCnt=%d!\n",pParams->m_nLinkSocket,pParams->soeCnt,0,0,0,0);

	//���ͱ���
	return centralSend(pParams);
}

//���ź�+ѹ���λ+�Ե�+�������� -104
static int32_t ProcessAutoGeneral_104(PARAMS *pParams)
{
	TSOE_GENERAL_STRUCT *pGeneralSOE;

	int32_t		nIndex = MSG_103;
	APDU*	pSendMsg = pParams->m_pSendMsg;
	int32_t		nMillis = 0;

	OP_GetpSOEQueue(pParams->m_nLinkSocket, SOE_GENERAL,(void**)&pGeneralSOE);

    if(pGeneralSOE->nASDU == 41)
    {
        pSendMsg->m_byteType = IEC104_TI_M_SP_TB_1;
        pSendMsg->m_byteVSQ = 1;
        pSendMsg->m_wordCOT = IEC104_COT_M_spont;
        pSendMsg->m_byteCOMADD = MONITOR_COMADDR;
        pSendMsg->m_byteADD = pParams->m_device_address;
        nIndex = 0;

        pSendMsg->m_msgdata[nIndex++] = pGeneralSOE->n103Inf-148;
        pSendMsg->m_msgdata[nIndex++] = 0;
        pSendMsg->m_msgdata[nIndex++] = 0;
        pSendMsg->m_msgdata[nIndex++] = pGeneralSOE->nDPI&0x01;//������Ϣ

        //��ȡʱ��
        nMillis = pGeneralSOE->dtTime.msec + pGeneralSOE->dtTime.second*1000;
        pSendMsg->m_msgdata[nIndex++] = LOBYTE(nMillis);//������ֽ�
        pSendMsg->m_msgdata[nIndex++] = HIBYTE(nMillis);//������ֽ�
        pSendMsg->m_msgdata[nIndex++] = pGeneralSOE->dtTime.minute;//��
        pSendMsg->m_msgdata[nIndex++] = pGeneralSOE->dtTime.hour;//ʱ
        pSendMsg->m_msgdata[nIndex++] = pGeneralSOE->dtTime.day;
        pSendMsg->m_msgdata[nIndex++] = pGeneralSOE->dtTime.month;
        pSendMsg->m_msgdata[nIndex++] = pGeneralSOE->dtTime.year;
    }
    else if(pGeneralSOE->nASDU == 1)
    {
        pSendMsg->m_byteType = IEC104_TI_M_DP_TB_1;
        pSendMsg->m_byteVSQ = 1;
        pSendMsg->m_wordCOT = IEC104_COT_M_spont;
        if(pGeneralSOE->n103Inf == 64)  //����ѹ�� ������0
        {
            pSendMsg->m_byteCOMADD = MAINTENANCE_COMADDR;
        }
        else
        {
            pSendMsg->m_byteCOMADD = PROTECT_COMADDR;
        }
        pSendMsg->m_byteADD = pParams->m_device_address;
        nIndex = MSG_104;
        pSendMsg->m_msgdata[INF_104L] = pGeneralSOE->n103Inf+1;
        pSendMsg->m_msgdata[INF_104M] = 0;
        pSendMsg->m_msgdata[INF_104H] = 0;
        pSendMsg->m_msgdata[nIndex++] = pGeneralSOE->nDPI&0x03;//˫����Ϣ
        //��ȡʱ��
        nMillis = pGeneralSOE->dtTime.msec + pGeneralSOE->dtTime.second*1000;
        pSendMsg->m_msgdata[nIndex++] = LOBYTE(nMillis);//������ֽ�
        pSendMsg->m_msgdata[nIndex++] = HIBYTE(nMillis);//������ֽ�
        pSendMsg->m_msgdata[nIndex++] = pGeneralSOE->dtTime.minute;//��
        pSendMsg->m_msgdata[nIndex++] = pGeneralSOE->dtTime.hour;//ʱ
        pSendMsg->m_msgdata[nIndex++] = pGeneralSOE->dtTime.day;
        pSendMsg->m_msgdata[nIndex++] = pGeneralSOE->dtTime.month;
        pSendMsg->m_msgdata[nIndex++] = pGeneralSOE->dtTime.year;

    }
    else if(pGeneralSOE->nASDU == 2)//�Ե� ���ʱ��=0 �������=0
    {
        pSendMsg->m_byteType = IEC104_TI_M_EP_TD_1;
        pSendMsg->m_byteVSQ = 1;
        pSendMsg->m_wordCOT = IEC104_COT_M_spont;
        pSendMsg->m_byteCOMADD = PROTECT_COMADDR;
        pSendMsg->m_byteADD = pParams->m_device_address;
        nIndex = MSG_104;
        pSendMsg->m_msgdata[INF_104L] = pGeneralSOE->n103Inf+1;
        pSendMsg->m_msgdata[INF_104M] = 0x10;
        pSendMsg->m_msgdata[INF_104H] = 0;
        pSendMsg->m_msgdata[nIndex++] = pGeneralSOE->nDPI&0x03;//˫����Ϣ
        //���ʱ��Relative time
        pSendMsg->m_msgdata[nIndex++] = 0;//���ֽ�
        pSendMsg->m_msgdata[nIndex++] = 0;//���ֽ�
        //��ȡʱ��
        nMillis = pGeneralSOE->dtTime.msec + pGeneralSOE->dtTime.second*1000;
        pSendMsg->m_msgdata[nIndex++] = LOBYTE(nMillis);//������ֽ�
        pSendMsg->m_msgdata[nIndex++] = HIBYTE(nMillis);//������ֽ�
        pSendMsg->m_msgdata[nIndex++] = pGeneralSOE->dtTime.minute;//��
        pSendMsg->m_msgdata[nIndex++] = pGeneralSOE->dtTime.hour;//ʱ
        pSendMsg->m_msgdata[nIndex++] = pGeneralSOE->dtTime.day;
        pSendMsg->m_msgdata[nIndex++] = pGeneralSOE->dtTime.month;
        pSendMsg->m_msgdata[nIndex++] = pGeneralSOE->dtTime.year;
    }

	pSendMsg->m_APCI.m_byteApduLength = APDU_HEAD_LENGTH + nIndex;

	//���ͱ���
	return centralSend(pParams);
}

//¼���Ŷ���
static int32_t ProcessAsdu23(PARAMS *pParams, uint16_t uiCOT)
{
	/*int32_t		nIndex = MSG_103;
	APDU*	pSendMsg = pParams->m_pSendMsg;
	int32_t		nMillis=0;
	int32_t		i;
	TDisturbData* pDisturbData;

    OP_GetpSOEQueue(pParams->m_nLinkSocket, SOE_WAVE,(void**)&pDisturbData);//¼���Ŷ���

    pSendMsg->m_byteType = 23;
    pSendMsg->m_byteVSQ = 0;
    pSendMsg->m_wordCOT = uiCOT;
    //pSendMsg->m_bytePRM = 0;
    pSendMsg->m_byteCOMADD = PROTECT_COMADDR;
    pSendMsg->m_byteADD = pParams->m_device_address;
    pSendMsg->m_msgdata[FUN_103] = PROTECT_FUN;
    pSendMsg->m_msgdata[INF_103] = 0;

    for(i = 0; i < WAVE_INFORMATION_SUM; i++)
    {
        pDisturbData--;
        if(pDisturbData < OP_GetSOEQHead(SOE_WAVE))
        {
            pDisturbData = OP_GetSOEQRear(SOE_WAVE);
        }
        if(pDisturbData->nValid &0x01)
        {
            pSendMsg->m_byteVSQ++;  //¼�����ܴ���

           //�������
            pSendMsg->m_msgdata[nIndex++] = LOBYTE(pDisturbData->nFAN);//���ֽ�
            pSendMsg->m_msgdata[nIndex++] = HIBYTE(pDisturbData->nFAN);//���ֽ�
            pSendMsg->m_msgdata[nIndex++] = (0x01 | ((pParams->waveTransferringFlag==TRUE)? 0x02 : 0)
                                            | ((pDisturbData->nValid &0x01)? 0 : 0x04));//����״̬

            //��ȡʱ��
            nMillis = pDisturbData->dtTime.msec + pDisturbData->dtTime.second*1000;
            pSendMsg->m_msgdata[nIndex++] = LOBYTE(nMillis);//������ֽ�
            pSendMsg->m_msgdata[nIndex++] = HIBYTE(nMillis);//������ֽ�
            pSendMsg->m_msgdata[nIndex++] = pDisturbData->dtTime.minute;//��
            pSendMsg->m_msgdata[nIndex++] = pDisturbData->dtTime.hour;//ʱ
            pSendMsg->m_msgdata[nIndex++] = pDisturbData->dtTime.day;//��
            pSendMsg->m_msgdata[nIndex++] = pDisturbData->dtTime.month;//��
            pSendMsg->m_msgdata[nIndex++] = pDisturbData->dtTime.year;//��

            if(pSendMsg->m_byteVSQ >= 8)  //¼�����ܴ���
            {
                break;
            }
        }

    }
    pSendMsg->m_APCI.m_byteApduLength = APDU_HEAD_LENGTH + nIndex;
*/
    return centralSend(pParams);
}

/*static int32_t ProcessAsdu26(PARAMS *pParams, TDisturbData* pDisturbData)
{
	int32_t		nIndex = MSG_103;
	APDU*	pSendMsg = pParams->m_pSendMsg;
	int32_t		nMillis=0;

	pSendMsg->m_APCI.m_byteStartChar = 0x68;
	pSendMsg->m_byteType = 26;
	pSendMsg->m_byteVSQ = 0x81;
	pSendMsg->m_wordCOT = 31;
	//pSendMsg->m_bytePRM = 0;
	pSendMsg->m_byteCOMADD = PROTECT_COMADDR;
	pSendMsg->m_byteADD = pParams->m_device_address;
	pSendMsg->m_msgdata[FUN_103] = PROTECT_FUN;
	pSendMsg->m_msgdata[INF_103] = 0;
	pSendMsg->m_msgdata[nIndex++] = 0;  //no use
	pSendMsg->m_msgdata[nIndex++] = 1;  //data type

	pSendMsg->m_msgdata[nIndex++] = LOBYTE(pDisturbData->nFAN);
	pSendMsg->m_msgdata[nIndex++] = HIBYTE(pDisturbData->nFAN);
	pSendMsg->m_msgdata[nIndex++] = LOBYTE(pDisturbData->nNOF);
	pSendMsg->m_msgdata[nIndex++] = HIBYTE(pDisturbData->nNOF);

	pSendMsg->m_msgdata[nIndex++] = CHANNEL_NUMBER;//NOC
	pSendMsg->m_msgdata[nIndex++] = LOBYTE(CHANNEL_SAMPLE_NUMBER);//NOE
	pSendMsg->m_msgdata[nIndex++] = HIBYTE(CHANNEL_SAMPLE_NUMBER);

	pSendMsg->m_msgdata[nIndex++] = LOBYTE(SAMPLE_INTERVAL);//INT
	pSendMsg->m_msgdata[nIndex++] = HIBYTE(SAMPLE_INTERVAL);

    //��ȡʱ��
    nMillis = pDisturbData->dtTime.msec + pDisturbData->dtTime.second*1000;
    pSendMsg->m_msgdata[nIndex++] = LOBYTE(nMillis);//������ֽ�
    pSendMsg->m_msgdata[nIndex++] = HIBYTE(nMillis);//������ֽ�
    pSendMsg->m_msgdata[nIndex++] = pDisturbData->dtTime.minute;//��
    pSendMsg->m_msgdata[nIndex++] = pDisturbData->dtTime.hour;//ʱ

    pSendMsg->m_APCI.m_byteApduLength = APDU_HEAD_LENGTH + nIndex;

    return centralSend(pParams);
}*///whs ��ʱ����      

/*static int32_t ProcessAsdu27(PARAMS *pParams, TDisturbData* pDisturbData)
{
	int32_t		nIndex = MSG_103;
	APDU*	pSendMsg = pParams->m_pSendMsg;
	float   fRatedValue1, fRatedValue2, fRatioValue;
	uint8_t   *pchar;

	pSendMsg->m_APCI.m_byteStartChar = 0x68;
	pSendMsg->m_byteType = 27;
	pSendMsg->m_byteVSQ = 0x81;
	pSendMsg->m_wordCOT = 31;
	//pSendMsg->m_bytePRM = 0;
	pSendMsg->m_byteCOMADD = PROTECT_COMADDR;
	pSendMsg->m_byteADD = pParams->m_device_address;
	pSendMsg->m_msgdata[FUN_103] = PROTECT_FUN;
	pSendMsg->m_msgdata[INF_103] = 0;
	pSendMsg->m_msgdata[nIndex++] = 0;  //no use
	pSendMsg->m_msgdata[nIndex++] = 1;  //data type

	pSendMsg->m_msgdata[nIndex++] = LOBYTE(pDisturbData->nFAN);
	pSendMsg->m_msgdata[nIndex++] = HIBYTE(pDisturbData->nFAN);

	pSendMsg->m_msgdata[nIndex++] = pParams->waveChannelName;//ģ�������ƣ����=CHANNEL_NUMBER

	if(pParams->waveChannelName < 5) //����
    {
        fRatedValue1 = 5.0*GetARMSysConfigValue(DI_GET_CURRENT_RATIO);
        fRatedValue2 = 5.0;
        fRatioValue  = 229.1;
    }
    else                //��ѹ
    {
        fRatedValue1 = 100.0*GetARMSysConfigValue(DI_GET_VOLTAGE_RATIO);
        fRatedValue2 = 100.0;
        fRatioValue  = 191.5;
    }

	pchar = (uint8_t*)&fRatedValue1;//1�ζֵ
	pSendMsg->m_msgdata[nIndex++] = *pchar++;
	pSendMsg->m_msgdata[nIndex++] = *pchar++;
	pSendMsg->m_msgdata[nIndex++] = *pchar++;
	pSendMsg->m_msgdata[nIndex++] = *pchar++;

	pchar = (uint8_t*)&fRatedValue2;//2�ζֵ
	pSendMsg->m_msgdata[nIndex++] = *pchar++;
	pSendMsg->m_msgdata[nIndex++] = *pchar++;
	pSendMsg->m_msgdata[nIndex++] = *pchar++;
	pSendMsg->m_msgdata[nIndex++] = *pchar++;

	pchar = (uint8_t*)&fRatioValue;//����ϵ��
	pSendMsg->m_msgdata[nIndex++] = *pchar++;
	pSendMsg->m_msgdata[nIndex++] = *pchar++;
	pSendMsg->m_msgdata[nIndex++] = *pchar++;
	pSendMsg->m_msgdata[nIndex++] = *pchar++;

    pSendMsg->m_APCI.m_byteApduLength = APDU_HEAD_LENGTH + nIndex;

    return centralSend(pParams);
}*///whs ��ʱ����      

/*static int32_t ProcessAsdu28(PARAMS *pParams, TDisturbData* pDisturbData)
{
	int32_t		nIndex = MSG_103;
	APDU*	pSendMsg = pParams->m_pSendMsg;

	pSendMsg->m_APCI.m_byteStartChar = 0x68;
	pSendMsg->m_byteType = 28;
	pSendMsg->m_byteVSQ = 0x81;
	pSendMsg->m_wordCOT = 31;
	//pSendMsg->m_bytePRM = 0;
	pSendMsg->m_byteCOMADD = PROTECT_COMADDR;
	pSendMsg->m_byteADD = pParams->m_device_address;
	pSendMsg->m_msgdata[FUN_103] = PROTECT_FUN;
	pSendMsg->m_msgdata[INF_103] = 0;
	pSendMsg->m_msgdata[nIndex++] = 0; //cmd type
	pSendMsg->m_msgdata[nIndex++] = 0;  //data type

	pSendMsg->m_msgdata[nIndex++] = LOBYTE(pDisturbData->nFAN);
	pSendMsg->m_msgdata[nIndex++] = HIBYTE(pDisturbData->nFAN);

	pSendMsg->m_APCI.m_byteApduLength = APDU_HEAD_LENGTH + nIndex;

    return centralSend(pParams);
}*///whs ��ʱ����      

/*static int32_t ProcessAsdu31(PARAMS *pParams, TDisturbData* pDisturbData, uint8_t CmdType)
{
	int32_t		nIndex = MSG_103;
	APDU*	pSendMsg = pParams->m_pSendMsg;

	pSendMsg->m_APCI.m_byteStartChar = 0x68;
	pSendMsg->m_byteType = 31;
	pSendMsg->m_byteVSQ = 0x81;
	pSendMsg->m_wordCOT = 31;
	//pSendMsg->m_bytePRM = 0;
	pSendMsg->m_byteCOMADD = PROTECT_COMADDR;
	pSendMsg->m_byteADD = pParams->m_device_address;
	pSendMsg->m_msgdata[FUN_103] = PROTECT_FUN;
	pSendMsg->m_msgdata[INF_103] = 0;
	pSendMsg->m_msgdata[nIndex++] = CmdType;
	pSendMsg->m_msgdata[nIndex++] = 1;  //data type

	pSendMsg->m_msgdata[nIndex++] = LOBYTE(pDisturbData->nFAN);
	pSendMsg->m_msgdata[nIndex++] = HIBYTE(pDisturbData->nFAN);
	if((CmdType == 35) || (CmdType == 37))
	{
	    pSendMsg->m_msgdata[nIndex++] = pParams->waveChannelName;  //channel number
	}
	else
    {
        pSendMsg->m_msgdata[nIndex++] = 0;
    }


	pSendMsg->m_APCI.m_byteApduLength = APDU_HEAD_LENGTH + nIndex;

    return centralSend(pParams);
}*///whs ��ʱ����      

/*static int32_t ProcessAsdu29(PARAMS *pParams, TDisturbData* pDisturbData)
{
	int32_t		nIndex = MSG_103;
	APDU*	pSendMsg = pParams->m_pSendMsg;
	uint8_t   DigitPointIndex = 1;
	uint32_t  nbit = 1, nlong = 0;
	uint8_t   i;

	pSendMsg->m_APCI.m_byteStartChar = 0x68;
	pSendMsg->m_byteType = 29;
	pSendMsg->m_byteVSQ = 0x81;
	pSendMsg->m_wordCOT = 31;
	//pSendMsg->m_bytePRM = 0;
	pSendMsg->m_byteCOMADD = PROTECT_COMADDR;
	pSendMsg->m_byteADD = pParams->m_device_address;
	pSendMsg->m_msgdata[FUN_103] = PROTECT_FUN;
	pSendMsg->m_msgdata[INF_103] = 0;

	pSendMsg->m_msgdata[nIndex++] = LOBYTE(pDisturbData->nFAN);
	pSendMsg->m_msgdata[nIndex++] = HIBYTE(pDisturbData->nFAN);
	pSendMsg->m_msgdata[nIndex++] = DIGIT_NUMBER;   //NOT

    //��ʼ״̬����
	pSendMsg->m_msgdata[nIndex++] = 0;
	pSendMsg->m_msgdata[nIndex++] = 0;

    for(i = 0; i < DIGIT_NUMBER; i++)
    {
        pSendMsg->m_msgdata[nIndex++] = PROTECT_FUN;
        pSendMsg->m_msgdata[nIndex++] = DIGIT_INF_START + i;
        pSendMsg->m_msgdata[nIndex++] = (pDisturbData->DigitData[0] & nbit) != 0 ? 2 : 1;
        nbit <<= 1;
    }
	pSendMsg->m_APCI.m_byteApduLength = APDU_HEAD_LENGTH + nIndex;

    if(centralSend(pParams)==HAVE_ERROR)
    {
        return HAVE_ERROR;
    }
    rt_thread_delay(20);

    //״̬��λ����
    while(DigitPointIndex < CHANNEL_SAMPLE_NUMBER)
    {
        pSendMsg->m_msgdata[4] = 0;   //NOT
        nlong = pDisturbData->DigitData[DigitPointIndex] ^ pDisturbData->DigitData[DigitPointIndex-1];
        if( nlong )
        {
            nIndex = 5;
            nbit = 1;
            pSendMsg->m_msgdata[nIndex++] = DigitPointIndex;
            pSendMsg->m_msgdata[nIndex++] = 0;

            for(i = 0; i < DIGIT_NUMBER; i++)
            {
                if( nbit & nlong)
                {
                    pSendMsg->m_msgdata[4]++;   //NOT
                    pSendMsg->m_msgdata[nIndex++] = PROTECT_FUN;
                    pSendMsg->m_msgdata[nIndex++] = DIGIT_INF_START + i;
                    pSendMsg->m_msgdata[nIndex++] = (pDisturbData->DigitData[DigitPointIndex] & nbit) != 0 ? 2 : 1;
                }
                nbit <<= 1;
            }
            pSendMsg->m_APCI.m_byteApduLength = APDU_HEAD_LENGTH + nIndex;

            if(centralSend(pParams)==HAVE_ERROR)
            {
                return HAVE_ERROR;
            }
            rt_thread_delay(20);
        }
        DigitPointIndex++;
    }

    return ProcessAsdu31(pParams, pDisturbData, 38);
}*///whs ��ʱ����      

/*static int32_t ProcessAsdu30(PARAMS *pParams, TDisturbData* pDisturbData)
{
	int32_t		nIndex = MSG_103;
	APDU*	pSendMsg = pParams->m_pSendMsg;
	int32_t		AnalogPointData;
	uint16_t   AnalogPointIndex = 0;
	uint8_t   ChannelName = pParams->waveChannelName;
	uint8_t   i;

    if(ChannelName > CHANNEL_NUMBER)
    {
        return ProcessAsdu31(pParams, pDisturbData, 37);
    }

	pSendMsg->m_APCI.m_byteStartChar = 0x68;
	pSendMsg->m_byteType = 30;
	pSendMsg->m_byteVSQ = 0x81;
	pSendMsg->m_wordCOT = 31;
	//pSendMsg->m_bytePRM = 0;
	pSendMsg->m_byteCOMADD = PROTECT_COMADDR;
	pSendMsg->m_byteADD = pParams->m_device_address;
	pSendMsg->m_msgdata[FUN_103] = PROTECT_FUN;
	pSendMsg->m_msgdata[INF_103] = 0;
	pSendMsg->m_msgdata[nIndex++] = 0;  //no use
	pSendMsg->m_msgdata[nIndex++] = 1;  //data type

	pSendMsg->m_msgdata[nIndex++] = LOBYTE(pDisturbData->nFAN);
	pSendMsg->m_msgdata[nIndex++] = HIBYTE(pDisturbData->nFAN);

	pSendMsg->m_msgdata[nIndex++] = ChannelName;
	pSendMsg->m_msgdata[nIndex++] = CYCLE_SAMPLE_NUMBER;

    while ( AnalogPointIndex < CHANNEL_SAMPLE_NUMBER )
    {
        nIndex = 8;
        pSendMsg->m_msgdata[nIndex++] = LOBYTE(AnalogPointIndex);//NFE
        pSendMsg->m_msgdata[nIndex++] = HIBYTE(AnalogPointIndex);

 		for (i = 0; i < CYCLE_SAMPLE_NUMBER; i++)
		{
			AnalogPointData = pDisturbData->AnalogData[ChannelName-1][AnalogPointIndex++];
			pSendMsg->m_msgdata[nIndex++] = LOBYTE(AnalogPointData);
			pSendMsg->m_msgdata[nIndex++] = HIBYTE(AnalogPointData);
		}

        pSendMsg->m_APCI.m_byteApduLength = APDU_HEAD_LENGTH + nIndex;
        if( centralSend(pParams) == HAVE_ERROR )
        {
            return HAVE_ERROR;
        }
        rt_thread_delay(20);
    }
	return ProcessAsdu31(pParams, pDisturbData, 35);
}*///whs ��ʱ����      

//¼��
/*static int32_t ProcessAsdu24(PARAMS *pParams)
{
	TDisturbData* pDisturbData;

	int32_t nError = 0;
	uint8_t nCmdType;
	uint16_t nFAN;

	nCmdType = pParams->m_pRecvMsg->m_msgdata[MSG_103];
	nFAN = (pParams->m_pRecvMsg->m_msgdata[4] | (pParams->m_pRecvMsg->m_msgdata[5]<<8));
	if((pDisturbData = OP_SearchDisturb(nFAN)) != RT_NULL)
	{
		switch(nCmdType)
		{
        case 1:                                     //�������������¼����������
            pParams->waveTransferringFlag = TRUE;
			rt_kprintf("SOE_WAVE-----wave report---,LinkSocket=%d!\n",pParams->m_nLinkSocket);
            nError = ProcessAsdu26(pParams, pDisturbData);   //�ش����¼������׼���ñ���
            break;

        case 2:                                     //���������������
            nError = ProcessAsdu28(pParams, pDisturbData);   //�ش����¼��״̬������׼���ñ���
            break;

        case 3:                                     //������˳�������
            nError = ProcessAsdu31(pParams, pDisturbData, 33);    //�ش��ͽ����Ͽ�
            pParams->waveTransferringFlag = FALSE;
            break;

        case 8:                                     //�������������ģ��������
            pParams->waveChannelName = pParams->m_pRecvMsg->m_msgdata[6];//ָ����ͨ��
            nError = ProcessAsdu30(pParams, pDisturbData);    //����ģ��������
            break;

        case 9:                                     //������˳���·ģ��������
            if( pParams->m_pRecvMsg->m_msgdata[6] != 0 )	//������˳���·ģ��������
            {
                if ( pParams->m_pRecvMsg->m_msgdata[6] < CHANNEL_NUMBER )//ָ����ͨ��
                {
                    pParams->waveChannelName = pParams->m_pRecvMsg->m_msgdata[6];//ָ����ͨ��
                    nError = ProcessAsdu27(pParams, pDisturbData);      //�ش���һ·ģ����
                }
                else
                {
                    nError = ProcessAsdu31(pParams, pDisturbData, 32);      //�ش�ģ�������ͽ���
                }
            }
            else		//�����������·ģ��������
            {
                if( ProcessAsdu31(pParams, pDisturbData, 36) == HAVE_ERROR )      //�ش�������·ģ�������͵��Ͽɱ���
                {
                    return HAVE_ERROR;
                }
                pParams->waveChannelName++;
                nError = ProcessAsdu27(pParams, pDisturbData);      //�ش���һ·ģ����
            }
            break;

        case 16:                                    //�����������״̬��
            nError = ProcessAsdu29(pParams, pDisturbData);   //׼���ش�״̬��
            break;

        case 17:                                    //�������ֹ����״̬��
            nError = ProcessAsdu31(pParams, pDisturbData, 39);  //�ش�����״̬���������͵��Ͽɱ���
            pParams->waveChannelName = 1;//�ӵ�һͨ����ʼ
            break;

        default:
            break;
		}
	}
	else
	{
        memcpy(pParams->m_pSendMsg,pParams->m_pRecvMsg,pParams->m_pRecvMsg->m_APCI.m_byteApduLength+2);
        pParams->m_pSendMsg->m_byteType = 25;
        pParams->m_pSendMsg->m_msgdata[MSG_103] = 3;  //go out
        nError = centralSend(pParams);
	}
	return nError;
}*///whs ��ʱ����      

//�ش�ASDU25
/*static int32_t ProcessAsdu25(PARAMS *pParams)
{
	TDisturbData* pDisturbData;

	int32_t nError = 0;
	uint8_t nCmdType = 0;
	uint16_t nFAN = 0;

	nCmdType = pParams->m_pRecvMsg->m_msgdata[MSG_103];
	nFAN = (pParams->m_pRecvMsg->m_msgdata[4] | (pParams->m_pRecvMsg->m_msgdata[5]<<8));
	if((pDisturbData = OP_SearchDisturb(nFAN)) != RT_NULL)
	{
		switch(nCmdType)
		{
        case 64:			                            //¼�����ͳɹ�������¼�����ͽ���
            pParams->waveTransferringFlag = FALSE;
            break;

        case 65:			                            //¼������ʧ�ܣ��ط�����¼���Ŷ���
            pParams->waveTransferringFlag = FALSE;
            ProcessAsdu23(pParams,31);
            break;

        case 66:			                            //������ش�·ģ�������ͳɹ�
            pParams->waveChannelName = pParams->m_pRecvMsg->m_msgdata[6];//ָ����ͨ��
            if (pParams->waveChannelName < CHANNEL_NUMBER)
            {                                           //׼������һ·ģ����׼���ñ���
                pParams->waveChannelName++;
                nError = ProcessAsdu27(pParams, pDisturbData);   //��ģ����׼���ñ���
            }
            else
            {                                           //�ش�ģ����������ϱ���
                nError = ProcessAsdu31(pParams, pDisturbData, 32);
            }
            break;

        case 67:			                            //������ش�·ģ��������ʧ��
            pParams->waveChannelName = pParams->m_pRecvMsg->m_msgdata[6];//ָ����ͨ��
            if (pParams->waveChannelName <= CHANNEL_NUMBER)
            {
                nError = ProcessAsdu27(pParams, pDisturbData);   //��ģ����׼���ñ���
            }
            else
            {                                           //�ش�ģ����������ϱ���
                nError = ProcessAsdu31(pParams, pDisturbData, 32);
            }
            break;

        case 68:			                            //������ش�״̬�����ͳɹ�
            pParams->waveChannelName = 1;
            nError = ProcessAsdu27(pParams, pDisturbData);   //��ģ����׼���ñ���
            break;

        case 69:			                           //������ش�״̬������ʧ��
            nError = ProcessAsdu28(pParams, pDisturbData);   //�ط�״̬��׼���ñ���
            break;

		default:
			nError = ProcessAsduNoACK(pParams);
			break;
		}
	}
	else
	{
        memcpy(pParams->m_pSendMsg,pParams->m_pRecvMsg,pParams->m_pRecvMsg->m_APCI.m_byteApduLength+2);
        pParams->m_pSendMsg->m_msgdata[MSG_103] = 3;  //go out
        nError = centralSend(pParams);
	}
	return nError;
}*///whs ��ʱ����      

static int32_t	ProcessSOE(PARAMS *pParams)
{
	int32_t		nError = NO_ERROR;
	APDU*	pApdu = (APDU*)pParams->m_pSendMsg;

	//�÷��ͱ���ͷ����Ϣ
	pApdu->m_APCI.m_byteStartChar  = 0x68;
	pApdu->m_APCI.m_byteApduLength = 0;

    if(pParams->m_severPort == ENGINEER_PORT)
    {
        switch(pParams->soeInfo.soeType)
        {
        case SOE_ACTION:
            nError = ProcessAutoAction_103(pParams);
            break;

        case SOE_WARNING:
            nError = ProcessAutoWarning_103(pParams);
            break;

        case SOE_DIEVENT:
            nError = ProcessAutoDIEvent_103(pParams);
            break;

        /*case SOE_GENERAL:
            nError = ProcessAutoGeneral_103(pParams);
            break;

        case SOE_WAVE:
            nError = ProcessAsdu23(pParams,31);
            break;*/

        default:
            break;
        }

        if(pParams->soeInfo.soeType != SOE_WAVE)//��¼���Ŷ����⣬����SOE��Ҫȷ��
        {
            pParams->m_nAutoConfirmed = ((nError==NO_ERROR)?pParams->soeInfo.soeType:SOE_CONFIRMED);//���û���ͳɹ�����ȷ�ϱ�־���´οɼ�����
            pParams->tick_SOESend = rt_tick_get();
            pParams->m_nSendNum_SOE = pParams->m_nSendNum;
        }

    }
    else if(pParams->m_severPort == OPERATOR_PORT)
    {
        switch(pParams->soeInfo.soeType)
        {
        case SOE_ACTION:
            nError = ProcessAutoAction_104(pParams);
            break;

        case SOE_WARNING:
            nError = ProcessAutoWarning_104(pParams);
            break;

        case SOE_DIEVENT:
            nError = ProcessAutoDIEvent_104(pParams);
            break;

        /*case SOE_GENERAL:
            nError = ProcessAutoGeneral_104(pParams);
            break;*/

        default:
            break;
        }

        pParams->m_nAutoConfirmed = ((nError==NO_ERROR)?pParams->soeInfo.soeType:SOE_CONFIRMED);//���û���ͳɹ�����ȷ�ϱ�־���´οɼ�����
        pParams->tick_SOESend = rt_tick_get();
        pParams->m_nSendNum_SOE = pParams->m_nSendNum;
    }

	return nError;
}

//ң�ض�·�� -103
static int32_t ProcessAsdu64(PARAMS *pParams)
{
	int32_t nError = 0;
	uint8_t nDCC = 0;
	APDU*	pSendMsg = pParams->m_pSendMsg;

	nDCC = (pParams->m_pRecvMsg->m_msgdata[MSG_103] & 0xc3);
	if((nDCC == 0x81) || (nDCC == 0x82)) //select
	{
		pParams->m_pSaveMsg->m_msgdata[MSG_103] = nDCC;
		nError = OP_RemoteCtrlSelt(pParams->m_pRecvMsg->m_msgdata[FUN_103], pParams->m_pRecvMsg->m_msgdata[INF_103], nDCC&0xff,pParams->m_device_address);
		if(!nError)
		{
			pParams->m_pSaveMsg->m_msgdata[MSG_103] = 0;
			rt_kprintf("ң��ѡ�����\n");
		}
	}
	else if((nDCC == 1) || (nDCC == 2)) //execute
	{
		nError = OP_RemoteCtrlExec(pParams->m_pRecvMsg->m_msgdata[FUN_103], pParams->m_pRecvMsg->m_msgdata[INF_103], nDCC, pParams->m_pSaveMsg->m_msgdata[MSG_103],pParams->m_device_address);
		if(!nError)
		{
			rt_kprintf("ң��ִ�д���\n");
		}
		pParams->m_pSaveMsg->m_msgdata[MSG_103] = 0;
	}
	else //����
	{
		pParams->m_pSaveMsg->m_msgdata[MSG_103] = 0;
		nError = 1;
	}

	memcpy(&pSendMsg->m_byteType,&pParams->m_pRecvMsg->m_byteType,ASDU_COPY_LENGTH);
	if(!nError)
	{
		pSendMsg->m_wordCOT = 76;//��,��У����
	}
	else
    {
        pSendMsg->m_wordCOT = 12;//�϶�
    }
	pSendMsg->m_APCI.m_byteApduLength = APDU_HEAD_LENGTH + 4;
	return centralSend(pParams);
}

//ң�ض�·�� Ͷ��ѹ�� Զ������ -104
static int32_t ProcessAsdu46(PARAMS *pParams)
{
	int32_t nError = 0;
	uint32_t nlINF;
	uint8_t nDCS = 0;
	APDU*	pSendMsg = pParams->m_pSendMsg;

	memcpy(pSendMsg, pParams->m_pRecvMsg, pParams->m_pRecvMsg->m_APCI.m_byteApduLength+2);
	nlINF = pParams->m_pRecvMsg->m_msgdata[INF_104H];
	nlINF = pParams->m_pRecvMsg->m_msgdata[INF_104M] | (nlINF<<8);
	nlINF = pParams->m_pRecvMsg->m_msgdata[INF_104L] | (nlINF<<8);

   nDCS = (pParams->m_pRecvMsg->m_msgdata[MSG_104] & 0xff);
    //nDCS = pParams->m_pRecvMsg->m_msgdata[MSG_104];
	
	if(pParams->m_pRecvMsg->m_byteType == 45)   //����
        nDCS++;

    if(nlINF < YK_DL_END_INDEX) //��·�� ÿ���豸��� ң�ص�// ��ʱ�̶�,����ʵ������趨
    {
        if(pParams->m_pRecvMsg->m_msgdata[MSG_104] &0x80)   //ѡ��/����
        {
            if(OP_RemoteCtrlSelt(1, (uint8_t)(nlINF-0x6001), nDCS,pParams->m_device_address))
            {
                pSendMsg->m_wordCOT+=1;
            }
            else
            {
                if(pSendMsg->m_wordCOT == 7)
                {
                    pSendMsg->m_wordCOT = 71;
                }
                else
                {
                    pSendMsg->m_wordCOT = 73;
                }
            }

        }
        else if((nDCS == 1) || (nDCS == 2)) //execute
        {
            nError = OP_RemoteCtrlExec(1, (uint8_t)(nlINF-0x6001), nDCS, nDCS|0x80,pParams->m_device_address);
            if(nError)
            {
                pSendMsg->m_wordCOT = 7;
                if(centralSend(pParams)==HAVE_ERROR)
                {
                    return HAVE_ERROR;
                }

                pSendMsg->m_wordCOT = 10;
            }
            else
            {
                pSendMsg->m_wordCOT = 71;
                rt_kprintf("ң��ִ�д���\n");
            }
        }
        else
        {
            pSendMsg->m_wordCOT = 71;
        }
    }
    else if((pParams->m_pRecvMsg->m_msgdata[MSG_104] &0x80) == 0)   //����  ѹ��  ��ֵ���л�
    {
        nlINF -= YK_DL_END_INDEX;
        if( nlINF == 0x00 )     //����
        {
            nError = OP_SignalReset((uint8_t)nlINF, nDCS,pParams->m_device_address);
            if(pParams->m_pRecvMsg->m_byteADD == 0xff)
            {
                return(NO_ERROR);
            }
        }
        else if( nlINF < 100 )//��ѹ��Ͷ��
        {
            nError = OP_MdfySoftStrap((uint8_t)nlINF, nDCS,pParams->m_device_address);
        }
        else                    //��ֵ���л�
        {
        	  nlINF -= 100;
            nError = OP_MdfySetPointGroup((uint8_t)nlINF, nDCS,pParams->m_device_address);
        }

        if(nError)
        {
            pSendMsg->m_wordCOT = 7;
            if(centralSend(pParams)==HAVE_ERROR)
            {
                return HAVE_ERROR;
            }

            pSendMsg->m_wordCOT = 10;
        }
        else
        {
            pSendMsg->m_wordCOT = 71;
        }
    }
    else
    {
        pSendMsg->m_wordCOT+=1;
    }

	pSendMsg->m_APCI.m_byteApduLength = APDU_HEAD_LENGTH + 4;
	return centralSend(pParams);
}

//��ѯ �޸� ������ֵ -103
static int32_t ProcessAsdu61(PARAMS *pParams)
{
	uint8_t nINF;
	uint8_t SetZone;
	uint8_t SetCount;
	uint8_t	bSetValue = 0;  //���ö�ֵ�Ƿ�ɹ�
	APDU*	pSendMsg = pParams->m_pSendMsg;

	memcpy(pSendMsg, pParams->m_pRecvMsg, pParams->m_pRecvMsg->m_APCI.m_byteApduLength+2);
	SetZone = (pParams->m_pRecvMsg->m_msgdata[MSG_103] & 0x0f);
	if((SetZone > 7) && (SetZone != 0x0f))
    {
        pSendMsg->m_wordCOT = 21;
    }
    else
    {
        if(SetZone == 0x0f)
        {
            SetZone = GetARMSysConfigValue(ID_GET_CURRENT_ZONE);//��ǰ��ֵ����
        }

        nINF = pSendMsg->m_msgdata[INF_103];
        if(nINF == 100) //��ѯ
        {
            SetCount = OP_GetSetPointValue(SetZone, &pSendMsg->m_msgdata[4]);
            if( (SetCount == 0)||(SetCount > VALUE_EACH_FRAME) )    //�޶�ֵ|��ֵ̫��
            {
                pSendMsg->m_wordCOT = 21;
            }
            else
            {
                pSendMsg->m_msgdata[MSG_103] = (pSendMsg->m_msgdata[MSG_103]&0xf0) | SetZone;//CPU���ַ|��ֵ����
                pSendMsg->m_msgdata[MSG_103+1] = 0;//��ֵ��Ŵ�0��ʼ

                pSendMsg->m_byteVSQ = SetCount+1;//��ֵ����+1
                pSendMsg->m_APCI.m_byteApduLength = APDU_HEAD_LENGTH + 4 + SetCount*3;

                if(centralSend(pParams)==HAVE_ERROR)
                {
                    return HAVE_ERROR;
                }

                //����֡
                pSendMsg->m_byteVSQ = 0x81;
                pSendMsg->m_wordCOT = 10;
                pSendMsg->m_APCI.m_byteApduLength = APDU_HEAD_LENGTH + 4;
            }
        }
        else if(nINF == 101) //Ԥ��
        {
            if(pParams->m_pRecvMsg->m_msgdata[3] == 0)
            {
                memcpy(pParams->m_pSaveMsg, pParams->m_pRecvMsg, pParams->m_pRecvMsg->m_APCI.m_byteApduLength+2);
            }
            else
            {
                memcpy(&pParams->m_pSaveMsg->m_msgdata[pParams->m_pSaveMsg->m_byteVSQ*3+1],
                       &pParams->m_pRecvMsg->m_msgdata[4], (pParams->m_pRecvMsg->m_byteVSQ-1)*3);
                pParams->m_pSaveMsg->m_byteVSQ +=    (pParams->m_pRecvMsg->m_byteVSQ-1);
            }

            pSendMsg->m_msgdata[INF_103] = 102;
            pSendMsg->m_msgdata[MSG_103] = (pSendMsg->m_msgdata[MSG_103]&0xf0) | SetZone;//CPU���ַ|��ֵ����

            if(centralSend(pParams)==HAVE_ERROR)
            {
                return HAVE_ERROR;
            }

            //����֡
            pSendMsg->m_byteVSQ = 0x81;
            pSendMsg->m_wordCOT = 10;
            pSendMsg->m_APCI.m_byteApduLength = APDU_HEAD_LENGTH + 4;
        }
        else if((nINF == 103) || (nINF == 105)) //ִ��/����
        {
            if(nINF == 103)//ִ��
            {
                if(pParams->m_pSaveMsg->m_msgdata[INF_103] == 101)
                {
                    pParams->m_pSaveMsg->m_msgdata[INF_103] = 0;
                    bSetValue = OP_MdfySetPointValue(SetZone, &pParams->m_pSaveMsg->m_msgdata[4],
                                                     pParams->m_pSaveMsg->m_byteVSQ-1, 1);
                }
                pSendMsg->m_wordCOT = bSetValue? 20:21;
                pSendMsg->m_msgdata[INF_103] = 104;
                pSendMsg->m_msgdata[MSG_103] = (pSendMsg->m_msgdata[MSG_103]&0xf0) | SetZone;//CPU���ַ|��ֵ����
            }
            else
            {
                pParams->m_pSaveMsg->m_msgdata[INF_103] = 0;
                pSendMsg->m_wordCOT = 20;
                pSendMsg->m_msgdata[INF_103] = 106;
                pSendMsg->m_msgdata[MSG_103] = (pSendMsg->m_msgdata[MSG_103]&0xf0) | SetZone;//CPU���ַ|��ֵ����
            }
        }
        else
        {
            pSendMsg->m_wordCOT = 21;
        }
    }

	return centralSend(pParams);
}

//��ѯ ������ֵ -104
static int32_t ProcessAsdu102(PARAMS *pParams)
{
	uint32_t nlINF;
	uint8_t SetZone;
	uint8_t SetCount;
	APDU*	pSendMsg = pParams->m_pSendMsg;

	memcpy(pSendMsg, pParams->m_pRecvMsg, pParams->m_pRecvMsg->m_APCI.m_byteApduLength+2);
	nlINF = pParams->m_pRecvMsg->m_msgdata[INF_104H];
	nlINF = pParams->m_pRecvMsg->m_msgdata[INF_104M] | (nlINF<<8);
	nlINF = pParams->m_pRecvMsg->m_msgdata[INF_104L] | (nlINF<<8);
	SetZone = (uint8_t)((nlINF-0x3001)>>8);

    if(((nlINF&0x00f0ff) != 0x3001) || (SetZone > 8))
    {
        pSendMsg->m_byteType = IEC104_TI_M_ME_NB_1;
        pSendMsg->m_byteVSQ = 1;
        pSendMsg->m_wordCOT = IEC104_COT_M_deactcon;
    }
    else
    {
        if(SetZone == 8)
        {
            SetZone = GetARMSysConfigValue(ID_GET_CURRENT_ZONE);//��ǰ��ֵ����
        }
        //��ѯ
        SetCount = OP_GetSetPointValue(SetZone, &pSendMsg->m_msgdata[MSG_104]);
        if( (SetCount == 0)||(SetCount > VALUE_EACH_FRAME) )    //�޶�ֵ|��ֵ̫��
        {
            pSendMsg->m_byteType = IEC104_TI_M_ME_NB_1;
            pSendMsg->m_byteVSQ = 1;
            pSendMsg->m_wordCOT = IEC104_COT_M_deactcon;
        }
        else
        {
            pSendMsg->m_byteType = IEC104_TI_M_ME_NB_1;
            pSendMsg->m_byteVSQ = SetCount|0x80;//��ֵ����
            pSendMsg->m_wordCOT = IEC104_COT_M_req;

            nlINF = ((uint32_t)SetZone<<8) + 0x3001;

            pSendMsg->m_msgdata[INF_104L] = nlINF&0x0ff;
            pSendMsg->m_msgdata[INF_104M] = (nlINF>>8)&0x0ff;
            pSendMsg->m_msgdata[INF_104H] = 0;

            pSendMsg->m_APCI.m_byteApduLength = APDU_HEAD_LENGTH + 3 + SetCount*3;

            if(centralSend(pParams)==HAVE_ERROR)
            {
                return HAVE_ERROR;
            }

            //����֡
            pSendMsg->m_byteType = IEC104_TI_C_RD_NA_1;
            pSendMsg->m_byteVSQ = 1;
            pSendMsg->m_wordCOT = IEC103_COT_M_queryEND;
            pSendMsg->m_APCI.m_byteApduLength = APDU_HEAD_LENGTH + 3;
        }
    }
    return centralSend(pParams);
}

//�޸� ����������ֵ -104
static int32_t ProcessAsdu49(PARAMS *pParams)
{
	uint32_t nlINF;
	uint8_t SetZone;
	uint8_t	bSetValue = 0;  //���ö�ֵ�Ƿ�ɹ�
	APDU*	pSendMsg = pParams->m_pSendMsg;

	memcpy(pSendMsg, pParams->m_pRecvMsg, pParams->m_pRecvMsg->m_APCI.m_byteApduLength+2);
	nlINF = pParams->m_pRecvMsg->m_msgdata[INF_104H];
	nlINF = pParams->m_pRecvMsg->m_msgdata[INF_104M] | (nlINF<<8);
	nlINF = pParams->m_pRecvMsg->m_msgdata[INF_104L] | (nlINF<<8);
	SetZone = (uint8_t)((nlINF-0x3001)>>8);

    if((nlINF< 0x3001) || (nlINF> 0x3800) || (nlINF&0x0ff)>DZ_NUM-2)
    {
        pSendMsg->m_byteType = IEC104_TI_C_SE_NB_1;
        if(pParams->m_pRecvMsg->m_wordCOT == 6)
        {
            pSendMsg->m_wordCOT = 71;
        }
        else if(pParams->m_pRecvMsg->m_wordCOT == 8)
        {
            pSendMsg->m_wordCOT = 73;
        }
    }
    else
    {
        if((pParams->m_pRecvMsg->m_wordCOT == 6)&&(pParams->m_pRecvMsg->m_msgdata[6]==0x80)) //Ԥ��
        {
            memcpy(pParams->m_pSaveMsg, pParams->m_pRecvMsg, pParams->m_pRecvMsg->m_APCI.m_byteApduLength+2);

            pSendMsg->m_wordCOT = 7;
        }
        else if(pParams->m_pRecvMsg->m_msgdata[6]==0x00) //ִ��/����
        {
            if(pParams->m_pRecvMsg->m_wordCOT == 6)//ִ��
            {
                if(pParams->m_pSaveMsg->m_msgdata[6] == 0x80)
                {
                    if((pParams->m_pRecvMsg->m_msgdata[3]==pParams->m_pSaveMsg->m_msgdata[3])&&
                        (pParams->m_pRecvMsg->m_msgdata[4]==pParams->m_pSaveMsg->m_msgdata[4])&&
                        (pParams->m_pRecvMsg->m_msgdata[5]==pParams->m_pSaveMsg->m_msgdata[5]))
                    {
                        bSetValue = OP_MdfySetPointValue(SetZone, &pParams->m_pSaveMsg->m_msgdata[3],
                                                         pParams->m_pSaveMsg->m_byteVSQ, nlINF&0x0ff);
                        pSendMsg->m_wordCOT = bSetValue? 7:71;
                    }
                    else
                    {
                        pSendMsg->m_wordCOT = 71;
                    }

                }
            }
            else
            {
                pSendMsg->m_wordCOT = 9;
            }
        }
        else
        {
            if(pParams->m_pRecvMsg->m_wordCOT == 6)
            {
                pSendMsg->m_wordCOT = 71;
            }
            else if(pParams->m_pRecvMsg->m_wordCOT == 8)
            {
                pSendMsg->m_wordCOT = 73;
            }
        }
    }

	return centralSend(pParams);
}

//��ѯSOE
static int32_t ProcessAsdu62(PARAMS *pParams)
{
	/*int32_t		nIndex = MSG_103;
	APDU*	pSendMsg = pParams->m_pSendMsg;
	int32_t		nMillis = 0;
	int32_t		i, j;
	float   fResultValue;
	uint8_t*   pnResultValue;
	TSOE_ACTION_STRUCT* pAppointedSOE;
	uint8_t   FaultRptNumber;

	memcpy(pSendMsg, pParams->m_pRecvMsg, pParams->m_pRecvMsg->m_APCI.m_byteApduLength+2);
	FaultRptNumber = pParams->m_pRecvMsg->m_msgdata[MSG_103];
	if((pAppointedSOE = OP_GetAppointedSOE(FaultRptNumber)) != RT_NULL)
	{
		pSendMsg->m_wordCOT = 20;
		if(centralSend(pParams) == HAVE_ERROR)
        {
            return HAVE_ERROR;
        }
	}
	else
	{
	    pSendMsg->m_wordCOT = 21;
	    return centralSend(pParams);
	}

	pSendMsg->m_byteType = 70;
	pSendMsg->m_byteVSQ = pAppointedSOE->nResultCnt+1;;
	pSendMsg->m_wordCOT = 64;
	//pSendMsg->m_bytePRM = 0;
	pSendMsg->m_byteCOMADD = PROTECT_COMADDR;
	pSendMsg->m_byteADD = pParams->m_device_address;
	pSendMsg->m_msgdata[FUN_103] = PROTECT_FUN;
	pSendMsg->m_msgdata[INF_103] = pAppointedSOE->n103Inf;//D_ProtAction_Define[pAppointedSOE->nDispID].n103Inf;

	pSendMsg->m_msgdata[nIndex++] = pAppointedSOE->nDPI&0x03;//˫����Ϣ

	//���ʱ��
	pSendMsg->m_msgdata[nIndex++] = LOBYTE(pAppointedSOE->nPosTime);//���ֽ�
	pSendMsg->m_msgdata[nIndex++] = HIBYTE(pAppointedSOE->nPosTime);//���ֽ�

	//�������
	pSendMsg->m_msgdata[nIndex++] = LOBYTE(pAppointedSOE->nFAN);//���ֽ�
	pSendMsg->m_msgdata[nIndex++] = HIBYTE(pAppointedSOE->nFAN);//���ֽ�

	//��ȡʱ��
	nMillis = pAppointedSOE->dtTime.msec + pAppointedSOE->dtTime.second*1000;
	pSendMsg->m_msgdata[nIndex++] = LOBYTE(nMillis);//������ֽ�
	pSendMsg->m_msgdata[nIndex++] = HIBYTE(nMillis);//������ֽ�
	pSendMsg->m_msgdata[nIndex++] = pAppointedSOE->dtTime.minute;//��
	pSendMsg->m_msgdata[nIndex++] = pAppointedSOE->dtTime.hour;//ʱ
    pSendMsg->m_msgdata[nIndex++] = pAppointedSOE->dtTime.day;//��
    pSendMsg->m_msgdata[nIndex++] = pAppointedSOE->dtTime.month;//��
    pSendMsg->m_msgdata[nIndex++] = pAppointedSOE->dtTime.year;//��
    pSendMsg->m_msgdata[nIndex++] = FaultRptNumber;//SIN
    pSendMsg->m_msgdata[nIndex++] = 0;//��������

    for(i=0; i< pAppointedSOE->nResultCnt; i++)
	{
		if(pAppointedSOE->n103Inf == 111)
			fResultValue = (float)pAppointedSOE->ResultVal[i]/100.0;
		else
            fResultValue = (float)pAppointedSOE->ResultVal[i]/1000.0;
		pnResultValue = (uint8_t*) &fResultValue;
		for(j=0; j<4; j++)  //R32.23
        {
            pSendMsg->m_msgdata[nIndex++] = *pnResultValue++;
        }
	}
    pSendMsg->m_APCI.m_byteApduLength = APDU_HEAD_LENGTH + nIndex;
    if(centralSend(pParams) == HAVE_ERROR)
    {
        return HAVE_ERROR;
    }

    memcpy(pSendMsg, pParams->m_pRecvMsg, pParams->m_pRecvMsg->m_APCI.m_byteApduLength+2);
	pSendMsg->m_wordCOT = 10;*///whs ��ʱ���� 
	return centralSend(pParams);     
}

//ʱ��ͬ��-103
static int32_t ProcessAsdu06(PARAMS *pParams)
{
	TDateTime tTime;
	int32_t nTemp = 0;

	tTime.year  = (pParams->m_pRecvMsg->m_msgdata[8])&0x7F;
	tTime.month = (pParams->m_pRecvMsg->m_msgdata[7])&0x0F;
	tTime.day   = (pParams->m_pRecvMsg->m_msgdata[6])&0x1F;
	tTime.hour  = (pParams->m_pRecvMsg->m_msgdata[5])&0x1F;
	tTime.minute = (pParams->m_pRecvMsg->m_msgdata[4])&0x3F;
	nTemp = MAKEWORD(pParams->m_pRecvMsg->m_msgdata[3],pParams->m_pRecvMsg->m_msgdata[MSG_103]);
	tTime.second = nTemp/1000;
	tTime.msec = nTemp%1000;

	if(OP_ModifyTime((&tTime),pParams->m_device_address))
	{
		rt_kprintf("Уʱʧ��\n");
	}

	memcpy(pParams->m_pSendMsg,pParams->m_pRecvMsg,pParams->m_pRecvMsg->m_APCI.m_byteApduLength+2);

	return centralSend(pParams);
}

//ʱ��ͬ��-104
static int32_t ProcessAsdu103(PARAMS *pParams)
{
	TDateTime tTime;
	int32_t nTemp = 0;

	tTime.year  = (pParams->m_pRecvMsg->m_msgdata[9])&0x7F;
	tTime.month = (pParams->m_pRecvMsg->m_msgdata[8])&0x0F;
	tTime.day   = (pParams->m_pRecvMsg->m_msgdata[7])&0x1F;
	tTime.hour  = (pParams->m_pRecvMsg->m_msgdata[6])&0x1F;
	tTime.minute = (pParams->m_pRecvMsg->m_msgdata[5])&0x3F;
	nTemp = MAKEWORD(pParams->m_pRecvMsg->m_msgdata[4],pParams->m_pRecvMsg->m_msgdata[MSG_104]);
	tTime.second = nTemp/1000;
	tTime.msec = nTemp%1000;

	if(OP_ModifyTime((&tTime),pParams->m_device_address))
	{ 
		rt_kprintf("Уʱʧ��-IEC104\n");
	}

	memcpy(pParams->m_pSendMsg,pParams->m_pRecvMsg,pParams->m_pRecvMsg->m_APCI.m_byteApduLength+2);
    pParams->m_pSendMsg->m_wordCOT = 7;

	return centralSend(pParams);
}

//��Ӧ���ٻ�-103
static int32_t ProcessAsdu07(PARAMS *pParams)
{
	uint8_t	nNum;
	int32_t		nIndex;
	APDU*	pSendMsg = pParams->m_pSendMsg;
	uint8_t   RecvCOMADD = pParams->m_pRecvMsg->m_byteCOMADD;

    TDateTime dtime;
    uint16_t nMillis;

	switch(RecvCOMADD)
	{
    case 0:
        //װ��ͨѶ״̬, ����ѹ��״̬
        if(ProcessDeviceState(pParams,IEC103_TI_M_TM_TA_3,IEC103_COT_M_totalQUERY)==HAVE_ERROR)
        {
            return HAVE_ERROR;
        }
        nIndex = MSG_103;
        //����ѹ��
        pSendMsg->m_byteType = 1;
        pSendMsg->m_byteVSQ = 0x81;
        pSendMsg->m_wordCOT = 9;
        //pSendMsg->m_bytePRM = 0;
        pSendMsg->m_byteCOMADD = 0;
        pSendMsg->m_byteADD = pParams->m_device_address;
        pSendMsg->m_msgdata[FUN_103] = PROTECT_FUN;

        pSendMsg->m_msgdata[INF_103] = 64;
        pSendMsg->m_msgdata[nIndex++] = GetARMSysConfigValue(ID_GET_JXSTRAP_STATE)+1;//����ѹ��״̬

        dtime = Now(); //��ȡ��ǰʱ��
        nMillis = dtime.msec + dtime.second*1000;
        pSendMsg->m_msgdata[nIndex++] = LOBYTE(nMillis);//������ֽ�
        pSendMsg->m_msgdata[nIndex++] = HIBYTE(nMillis);//������ֽ�
        pSendMsg->m_msgdata[nIndex++] = dtime.minute;//��
        pSendMsg->m_msgdata[nIndex++] = dtime.hour;//ʱ
        pSendMsg->m_msgdata[nIndex++] = pParams->m_pRecvMsg->m_msgdata[MSG_103];//���ٻ����

        pSendMsg->m_APCI.m_byteApduLength = APDU_HEAD_LENGTH + nIndex;

        if(centralSend(pParams)==HAVE_ERROR)
        {
            return HAVE_ERROR;
        }
        break;

    case 1:     //ASDU_1����Ϣ //�澯 ��ѹ�� ¼��
        if(GetARMSysConfigValue(ID_GET_JXSTRAP_STATE) == 0)   //maintenance strap off
        {
            nNum = 0;
            OP_GetWarningState(&(pSendMsg->m_msgdata[INF_103]), &nNum);
            while(nNum)
            {
                pSendMsg->m_byteType = 1;
                pSendMsg->m_byteVSQ = 0x81;
                pSendMsg->m_wordCOT = 9;
                //pSendMsg->m_bytePRM = 0;
                pSendMsg->m_byteCOMADD = PROTECT_COMADDR;
                pSendMsg->m_byteADD = pParams->m_device_address;
                pSendMsg->m_msgdata[FUN_103] = PROTECT_FUN;

                nIndex = MSG_103+1;
                dtime = Now(); //��ȡ��ǰʱ��
                nMillis = dtime.msec + dtime.second*1000;
                pSendMsg->m_msgdata[nIndex++] = LOBYTE(nMillis);//������ֽ�
                pSendMsg->m_msgdata[nIndex++] = HIBYTE(nMillis);//������ֽ�
                pSendMsg->m_msgdata[nIndex++] = dtime.minute;//��
                pSendMsg->m_msgdata[nIndex++] = dtime.hour;//ʱ
                pSendMsg->m_msgdata[nIndex++] = pParams->m_pRecvMsg->m_msgdata[MSG_103];//���ٻ����

                pSendMsg->m_APCI.m_byteApduLength = APDU_HEAD_LENGTH + nIndex;

                if(centralSend(pParams)==HAVE_ERROR)
                {
                    return HAVE_ERROR;
                }
                rt_thread_delay(20);
                OP_GetWarningState(&(pSendMsg->m_msgdata[INF_103]), &nNum);
            }

            rt_thread_delay(20);
            nNum = 0;
            OP_GetSoftStrapState(&(pSendMsg->m_msgdata[INF_103]), &nNum);
            while(nNum)
            {
                pSendMsg->m_byteType = 1;
                pSendMsg->m_byteVSQ = 0x81;
                pSendMsg->m_wordCOT = 9;
                //pSendMsg->m_bytePRM = 0;
                pSendMsg->m_byteCOMADD = PROTECT_COMADDR;
                pSendMsg->m_byteADD = pParams->m_device_address;
                pSendMsg->m_msgdata[FUN_103] = PROTECT_FUN;

                nIndex = MSG_103+1;
                dtime = Now(); //��ȡ��ǰʱ��
                nMillis = dtime.msec + dtime.second*1000;
                pSendMsg->m_msgdata[nIndex++] = LOBYTE(nMillis);//������ֽ�
                pSendMsg->m_msgdata[nIndex++] = HIBYTE(nMillis);//������ֽ�
                pSendMsg->m_msgdata[nIndex++] = dtime.minute;//��
                pSendMsg->m_msgdata[nIndex++] = dtime.hour;//ʱ
                pSendMsg->m_msgdata[nIndex++] = pParams->m_pRecvMsg->m_msgdata[MSG_103];//���ٻ����

                pSendMsg->m_APCI.m_byteApduLength = APDU_HEAD_LENGTH + nIndex;

                if(centralSend(pParams)==HAVE_ERROR)
                {
                    return HAVE_ERROR;
                }
                rt_thread_delay(20);
                OP_GetSoftStrapState(&(pSendMsg->m_msgdata[INF_103]), &nNum);
            }

            if(ProcessAsdu23(pParams,9)==HAVE_ERROR)
            {
                return HAVE_ERROR;
            }
        }
        break;

    case 2:     //ASDU_41����Ϣ����ASDU_40���� ���� ��λ������ASDU_38����
        if(GetARMSysConfigValue(ID_GET_JXSTRAP_STATE) == 0)   //maintenance strap off
        {
            nNum = 0;
           // OP_GetDIState(&(pSendMsg->m_msgdata[INF_103]), &nNum);

            pSendMsg->m_byteType = 0x28;
            pSendMsg->m_byteVSQ = nNum|0x80;
            pSendMsg->m_wordCOT = 9;
            //pSendMsg->m_bytePRM = 0;
            pSendMsg->m_byteCOMADD = MONITOR_COMADDR;
            pSendMsg->m_byteADD = pParams->m_device_address;
            pSendMsg->m_msgdata[FUN_103] = MONITOR_FUN;

            pSendMsg->m_msgdata[nNum*2+1] = pParams->m_pRecvMsg->m_msgdata[2];
            pSendMsg->m_APCI.m_byteApduLength = APDU_HEAD_LENGTH + nNum*2 + 2;

            if(centralSend(pParams)==HAVE_ERROR)
            {
                return HAVE_ERROR;
            }
        }
        break;
	}

	rt_thread_delay(20);
	//ASDU8 ���ٻ�����
	pSendMsg->m_byteType = 0x08;
	pSendMsg->m_byteVSQ = 0x81;
	pSendMsg->m_wordCOT = 10;
	//pSendMsg->m_bytePRM = 0;
	pSendMsg->m_byteCOMADD = RecvCOMADD;
	pSendMsg->m_byteADD = pParams->m_device_address;
	pSendMsg->m_msgdata[FUN_103] = GLOBAL_FUN;
	pSendMsg->m_msgdata[INF_103] = 0;
	pSendMsg->m_msgdata[MSG_103] = pParams->m_pRecvMsg->m_msgdata[MSG_103];
	pSendMsg->m_APCI.m_byteApduLength = APDU_HEAD_LENGTH + 3;
	return centralSend(pParams);
}

//��Ӧ���ٻ�-104
static int32_t ProcessAsdu100(PARAMS *pParams)
{
	uint8_t	nNum;
	int32_t		nIndex;
	APDU*	pSendMsg = pParams->m_pSendMsg;
	uint8_t   RecvCOMADD = pParams->m_pRecvMsg->m_byteCOMADD;

	//ASDU100 ���ٻ�ȷ��
	memcpy(pParams->m_pSendMsg,pParams->m_pRecvMsg,pParams->m_pRecvMsg->m_APCI.m_byteApduLength+2);
    pParams->m_pSendMsg->m_wordCOT = 7;
    if(centralSend(pParams)==HAVE_ERROR)
    {
        return HAVE_ERROR;
    }

	rt_thread_delay(20);
	switch(RecvCOMADD)
	{
    case 0:
        //װ��ͨѶ״̬, ����ѹ��״̬
        /*if(ProcessDeviceState(pParams,IEC104_TI_M_SP_NA_1,IEC104_COT_M_introgen)==HAVE_ERROR)
        {
            return HAVE_ERROR;
        }
        nIndex = MSG_104;
        //����ѹ��
        pSendMsg->m_byteType = IEC104_TI_M_DP_NA_1;
        pSendMsg->m_byteVSQ = 1;
        pSendMsg->m_wordCOT = IEC104_COT_M_introgen;
        pSendMsg->m_byteCOMADD = RecvCOMADD;
        pSendMsg->m_byteADD = pParams->m_device_address;

        pSendMsg->m_msgdata[INF_104L] = 65;
        pSendMsg->m_msgdata[INF_104M] = 0;
        pSendMsg->m_msgdata[INF_104H] = 0;
        pSendMsg->m_msgdata[nIndex++] = GetARMSysConfigValue(ID_GET_JXSTRAP_STATE)+1;//����ѹ��״̬

        pSendMsg->m_APCI.m_byteApduLength = APDU_HEAD_LENGTH + nIndex;

        if(centralSend(pParams)==HAVE_ERROR)
        {
            return HAVE_ERROR;
        }*/
				return HAVE_ERROR;
        break;

    case 1:     //�澯 ��ѹ��
        //if(GetARMSysConfigValue(ID_GET_JXSTRAP_STATE) == 0)   //maintenance strap off
       // {
		    return HAVE_ERROR;
		    break;
            /*nNum = 0;
            OP_GetWarningState(&(pSendMsg->m_msgdata[0]), &nNum);
            while(nNum)
            {
                pSendMsg->m_byteType = IEC104_TI_M_DP_NA_1;
                pSendMsg->m_byteVSQ = 1;
                pSendMsg->m_wordCOT = IEC104_COT_M_introgen;
                pSendMsg->m_byteCOMADD = RecvCOMADD;
                pSendMsg->m_byteADD = pParams->m_device_address;

                nIndex = MSG_104;
                pSendMsg->m_msgdata[INF_104L] = pSendMsg->m_msgdata[0]+1;
                pSendMsg->m_msgdata[MSG_104] = pSendMsg->m_msgdata[1];
                pSendMsg->m_msgdata[INF_104M] = 0;
                pSendMsg->m_msgdata[INF_104H] = 0;

                pSendMsg->m_APCI.m_byteApduLength = APDU_HEAD_LENGTH + 4;

                if(centralSend(pParams)==HAVE_ERROR)
                {
                    return HAVE_ERROR;
                }
                rt_thread_delay(20);
                OP_GetWarningState(&(pSendMsg->m_msgdata[0]), &nNum);
            }

            rt_thread_delay(20);
            nNum = 0;
            OP_GetSoftStrapState(&(pSendMsg->m_msgdata[0]), &nNum);
            while(nNum)
            {
                pSendMsg->m_byteType = IEC104_TI_M_DP_NA_1;
                pSendMsg->m_byteVSQ = 1;
                pSendMsg->m_wordCOT = IEC104_COT_M_introgen;
                pSendMsg->m_byteCOMADD = RecvCOMADD;
                pSendMsg->m_byteADD = pParams->m_device_address;

                pSendMsg->m_msgdata[INF_104L] = pSendMsg->m_msgdata[0]+1;
                pSendMsg->m_msgdata[MSG_104] = pSendMsg->m_msgdata[1];
                pSendMsg->m_msgdata[INF_104M] = 0;
                pSendMsg->m_msgdata[INF_104H] = 0;

                pSendMsg->m_APCI.m_byteApduLength = APDU_HEAD_LENGTH + 4;

                if(centralSend(pParams)==HAVE_ERROR)
                {
                    return HAVE_ERROR;
                }
                rt_thread_delay(20);
                OP_GetSoftStrapState(&(pSendMsg->m_msgdata[0]), &nNum);
            }
        
        break;*/

    case 2:     //����
        //if(GetARMSysConfigValue(ID_GET_JXSTRAP_STATE) == 0)   //maintenance strap off
        //{
            nNum = 0;
            if(OP_GetDIState(&(pSendMsg->m_msgdata[3]), &nNum, pParams->m_device_address) == HAVE_ERROR)
					  {
							  return HAVE_ERROR;
						}    

            pSendMsg->m_byteType = IEC104_TI_M_SP_NA_1;
            pSendMsg->m_byteVSQ = (nNum+2)|0x80;//��������
            pSendMsg->m_wordCOT = 20;
            pSendMsg->m_byteCOMADD = RecvCOMADD;
            pSendMsg->m_byteADD = pParams->m_device_address;

            pSendMsg->m_APCI.m_byteApduLength = APDU_HEAD_LENGTH + (nNum+2);//*4;
            
						pSendMsg->m_msgdata[0] = 0x01;
						pSendMsg->m_msgdata[1] = 0x02;
						pSendMsg->m_msgdata[2] = 0x03;
						
						
            
            /*for(nIndex=nNum*4; nIndex>0;  )
            {
                nNum--;
                pSendMsg->m_msgdata[--nIndex] = pSendMsg->m_msgdata[nNum*2+1];
                pSendMsg->m_msgdata[--nIndex] = 0;
                pSendMsg->m_msgdata[--nIndex] = 0;
                pSendMsg->m_msgdata[--nIndex] = pSendMsg->m_msgdata[nNum*2]-148;
            }*/

            if(centralSend(pParams)==HAVE_ERROR)
            {
                return HAVE_ERROR;
            }
        //}
        break;
	}

	rt_thread_delay(20);
	//ASDU100 ���ٻ�����
	memcpy(pParams->m_pSendMsg,pParams->m_pRecvMsg,pParams->m_pRecvMsg->m_APCI.m_byteApduLength+2);
    pParams->m_pSendMsg->m_wordCOT = 10;
	return centralSend(pParams);
	rt_kprintf("pSendMsg:  %x.%x.%x.%x.%x\n",pSendMsg->m_msgdata[0],pSendMsg->m_msgdata[1],pSendMsg->m_msgdata[2],pSendMsg->m_msgdata[3],pSendMsg->m_msgdata[4]);

}

void save_general_report(uint8_t nASDU,uint8_t nDispID, uint8_t nDPI, uint8_t nCOT);
//�źŸ��� ��ֵ���л�(��̵Ķ�ֵ���л�ȷ��INF=224) ��ѹ��Ͷ��
static int32_t ProcessAsdu20(PARAMS *pParams)
{
	int32_t		nIndex = MSG_103;
	APDU*	pSendMsg = pParams->m_pSendMsg;
	uint8_t   RecvINF;
	uint8_t   RecvDCO;
	int32_t		nYesOrNo;
	TDateTime dtime;
	int32_t		nMillis = 0;

	RecvINF = pParams->m_pRecvMsg->m_msgdata[INF_103];
	RecvDCO = (pParams->m_pRecvMsg->m_msgdata[MSG_103] & 0x03);
	if( RecvINF == 19 )     //����
	{
		nYesOrNo = OP_SignalReset(RecvINF, RecvDCO,pParams->m_device_address);
		if(pParams->m_pRecvMsg->m_byteADD == 0xff)
        {
            return(NO_ERROR);
        }
	}
	else if( RecvINF < 100 )//��ѹ��Ͷ��
	{
	    nYesOrNo = OP_MdfySoftStrap(RecvINF, RecvDCO,pParams->m_device_address);
	}
	else                    //��ֵ���л�
    {
        nYesOrNo = OP_MdfySetPointGroup(RecvINF, RecvDCO,pParams->m_device_address);
        RecvINF = 224;   //��ֵ���仯����Ϣ�����224
	}

/*	switch( RecvINF )
	{
		case 19:        //����
			nYesOrNo = OP_SignalReset(RecvINF, RecvDCO,pParams->m_device_address);
		break;

		case 100:		// zone 0 ��ֵ���л�
		case 101: 		// zone 1
		case 102:		// zone 2
		case 103:		// zone 3
		case 104:		// zone 4
		case 105:		// zone 5
		case 106:		// zone 6
		case 107:		// zone 7
			nYesOrNo = OP_MdfySetPointGroup(RecvINF, RecvDCO,pParams->m_device_address);
			//pParams->m_pRecvMsg->m_msgdata[INF_103] = 224;   //8000ϵͳ����ôҪ���
            save_general_report(ASDU_1,2,RecvDCO,(nYesOrNo==1)?20:21);//��3���ź��Ƕ�ֵ���仯,ң���޸Ķ�ֵ������״̬�Ĵ���ԭ����20
			return(NO_ERROR);
//        break;

		default:		//��ѹ��Ͷ��
			nYesOrNo = OP_MdfySoftStrap(RecvINF, RecvDCO,pParams->m_device_address);
		break;
	}*/
	pSendMsg->m_byteType = 0x01;
	pSendMsg->m_byteVSQ = 0x81;
	pSendMsg->m_wordCOT = (nYesOrNo==1)?20:21;
	//pSendMsg->m_bytePRM = 0;
	pSendMsg->m_byteCOMADD = PROTECT_COMADDR;
	pSendMsg->m_byteADD = pParams->m_device_address;
	pSendMsg->m_msgdata[FUN_103] = pParams->m_pRecvMsg->m_msgdata[FUN_103];
	pSendMsg->m_msgdata[INF_103] = RecvINF;

	pSendMsg->m_msgdata[nIndex++] = pParams->m_pRecvMsg->m_msgdata[MSG_103];

	dtime = Now(); //��ȡ��ǰʱ��
	nMillis = dtime.msec + dtime.second*1000;
	pSendMsg->m_msgdata[nIndex++] = LOBYTE(nMillis);//������ֽ�
	pSendMsg->m_msgdata[nIndex++] = HIBYTE(nMillis);//������ֽ�
	pSendMsg->m_msgdata[nIndex++] = dtime.minute;//��
	pSendMsg->m_msgdata[nIndex++] = dtime.hour;//ʱ

	pSendMsg->m_msgdata[nIndex++] = pParams->m_pRecvMsg->m_msgdata[3];

	pSendMsg->m_APCI.m_byteApduLength = APDU_HEAD_LENGTH + nIndex;
	return centralSend(pParams);
}

//����� ����->���᷵��->���ص���� -103
static int32_t ProcessAsdu88(PARAMS *pParams)
{
	int32_t		nIndex;
	APDU*	pSendMsg = pParams->m_pSendMsg;
	int32_t		i=0;

	uint8_t nInf;
	TMeterMsg* pMsg;
	uint8_t* pchar;
	uint8_t nNum;
  uint8_t ymdevaddr;
	//ASDU88
	
	memcpy(&pSendMsg->m_byteType,&pParams->m_pRecvMsg->m_byteType,ASDU_COPY_LENGTH);
	pSendMsg->m_APCI.m_byteApduLength = APDU_HEAD_LENGTH + 4;
	pSendMsg->m_wordCOT = 2;
	ymdevaddr = pParams->m_pRecvMsg->m_byteADD;
	nNum = OP_GetPowerEnergy103(pParams->m_pRecvMsg->m_msgdata[2], &nInf, &pMsg, ymdevaddr);//��ȡ�����
	if(nNum == 0)
    {
        pSendMsg->m_wordCOT = 66;
        return centralSend(pParams);
    }
	else if(centralSend(pParams)==HAVE_ERROR)
	{
        pSendMsg->m_wordCOT = 2;
		return HAVE_ERROR;
	}

	//ASDU36
	pSendMsg->m_byteType = 0x24;
	pSendMsg->m_byteVSQ = nNum;
	pSendMsg->m_wordCOT = 2;
	//pSendMsg->m_bytePRM = 0;
	pSendMsg->m_byteCOMADD = MONITOR_COMADDR;
	pSendMsg->m_byteADD = pParams->m_device_address;
	pSendMsg->m_msgdata[FUN_103] = MONITOR_FUN;
	pSendMsg->m_msgdata[INF_103] = nInf;    //��һ���������Ϣ���=6

	nIndex  = MSG_103;
	for(i=0; i<nNum; i++,pMsg++)
	{
        pchar = (uint8_t*)pMsg;
		pSendMsg->m_msgdata[nIndex++] = *pchar++;
		pSendMsg->m_msgdata[nIndex++] = *pchar++;
		pSendMsg->m_msgdata[nIndex++] = *pchar++;
		pSendMsg->m_msgdata[nIndex++] = *pchar;
		pSendMsg->m_msgdata[nIndex++] = pMsg->SequenceNum;
	}

	pSendMsg->m_msgdata[nIndex++] = pParams->m_pRecvMsg->m_msgdata[3];
	pSendMsg->m_APCI.m_byteApduLength = APDU_HEAD_LENGTH + nIndex;

	if(centralSend(pParams)==HAVE_ERROR)
	{
		return HAVE_ERROR;
	}

	memcpy(&pSendMsg->m_byteType,&pParams->m_pRecvMsg->m_byteType,ASDU_COPY_LENGTH);
	pSendMsg->m_APCI.m_byteApduLength = APDU_HEAD_LENGTH + 4;
    pSendMsg->m_wordCOT = 10;
	pSendMsg->m_msgdata[3] = 0; 

	return centralSend(pParams);   
}

//����� ����->���᷵��->���ص���� -104
static int32_t ProcessAsdu101(PARAMS *pParams)
{
	int32_t		nIndex;
	APDU*	pSendMsg = pParams->m_pSendMsg;
	int32_t		i=0;

	uint8_t nInf;
	TMeterMsg* pMsg;
	uint8_t* pchar;
	uint8_t nNum;
	uint8_t ymdevaddr;

	//ASDU101
	memcpy(&pSendMsg->m_byteType,&pParams->m_pRecvMsg->m_byteType,ASDU_COPY_LENGTH);
	pSendMsg->m_APCI.m_byteApduLength = APDU_HEAD_LENGTH + 4;
	pSendMsg->m_wordCOT = IEC104_COT_M_acton;
	ymdevaddr = pParams->m_pRecvMsg->m_byteADD;
	nNum = OP_GetPowerEnergy103(pParams->m_pRecvMsg->m_msgdata[MSG_104], &nInf, &pMsg, ymdevaddr);//��ȡ�����
	if(nNum == 0)
    {
        pSendMsg->m_wordCOT = IEC104_COT_M_PN | IEC104_COT_M_acton;
        return centralSend(pParams);
    }
	else if(centralSend(pParams)==HAVE_ERROR)
	{
		pSendMsg->m_wordCOT = IEC104_COT_M_acton;
		return HAVE_ERROR;
	}

	//ASDU15
	pSendMsg->m_byteType = IEC104_TI_M_IT_NA_1;
	pSendMsg->m_byteVSQ = nNum|0x80;
	pSendMsg->m_wordCOT = IEC104_COT_M_reqcogen;
	pSendMsg->m_byteCOMADD = MONITOR_COMADDR;
	pSendMsg->m_byteADD = pParams->m_device_address;
	pSendMsg->m_msgdata[INF_104L] = 0x01;    //��һ���������Ϣ���=0x6401
	pSendMsg->m_msgdata[INF_104M] = 0x64;
	pSendMsg->m_msgdata[INF_104H] = 0;

	nIndex = MSG_104;
	for(i=0; i<nNum; i++,pMsg++)
	{
        pchar = (uint8_t*)pMsg;
		pSendMsg->m_msgdata[nIndex++] = *pchar++;
		pSendMsg->m_msgdata[nIndex++] = *pchar++;
		pSendMsg->m_msgdata[nIndex++] = *pchar++;
		pSendMsg->m_msgdata[nIndex++] = *pchar;
		pSendMsg->m_msgdata[nIndex++] = pMsg->SequenceNum;
	}

	pSendMsg->m_APCI.m_byteApduLength = APDU_HEAD_LENGTH + nIndex;

	if(centralSend(pParams)==HAVE_ERROR)
	{
		return HAVE_ERROR;
	}

	memcpy(&pSendMsg->m_byteType,&pParams->m_pRecvMsg->m_byteType,ASDU_COPY_LENGTH);
	pSendMsg->m_APCI.m_byteApduLength = APDU_HEAD_LENGTH + 4;
    pSendMsg->m_wordCOT = IEC104_COT_M_actterm;     

	return centralSend(pParams);
}


//�����������е�ָ�����飬��������ı��
static void resetTask(PARAMS** ppParams,PARAMS* pParams)
{
	int32_t i=0;
	if(!pParams)//ָ��Ϊ�գ�ֱ�ӷ���
		return;

	for(i=0;i<MAX_CLIENT_NUM;i++)
	{
		if((ppParams[i]->m_clientIP == pParams->m_clientIP)
            &&(ppParams[i]->m_severPort == pParams->m_severPort))
		{
			ppParams[i] = 0;
			break;
		}
	}
}
//�����˳�ʱ(exitTask,1)
static void clearTask(PARAMS* pParams)
{
	if(!pParams)//ָ��Ϊ�գ�ֱ�ӷ���
		return;

	shutdown(pParams->m_serverFd,2);
	lwip_close(pParams->m_serverFd);

	if(pParams->m_nAutoConfirmed == SOE_CONFIRMED)
	{
	   OP_DetachLink(pParams->m_serverFd, pParams->m_clientIP);
	}

	if(pParams->m_pRecvMsg) rt_free(pParams->m_pRecvMsg);
	if(pParams->m_pSendMsg) rt_free(pParams->m_pSendMsg);
	if(pParams->m_pSaveMsg) rt_free(pParams->m_pSaveMsg);
	if(pParams->m_pCtrlMsg) rt_free(pParams->m_pCtrlMsg);

	if(pParams) rt_free(pParams);
}
//�쳣�˳�ʱ������
static void exitTask(PARAMS** ppParams,PARAMS* pParams,const char* error)
{
	rt_kprintf(error);

	resetTask(ppParams,pParams);

	clearTask(pParams); //�ͷ�������ڴ档

	//rt-thread �߳��Լ�ɾ���Լ���Ҫ��һ���̵߳��ȣ���ɾ���Լ�ǰһ��Ҫ�ͷ�������ڴ档
	rt_thread_delete(rt_thread_self());
    rt_schedule();
}

//�жϵ�ǰIP��ַ�����������Ƿ����Ѿ�����һ��ǰ����
static int32_t getConnState(PARAMS** ppParams,uint32_t clientIP,uint32_t serverPort)
{
	//����ֵ:1��ʾ�������ӣ�0��ʾɾ�������ӣ����������ӡ�
	int32_t i=0;
	int32_t nRetVal = 0;

	for(i=0;i<MAX_CLIENT_NUM;i++)
	{
		PARAMS* pParams =ppParams[i];


		if( pParams == 0 )//��ʾ���п�λ�������½�����
		{
			nRetVal = 1;//�ҵ�����λ���������Ϸ��أ���ȱ�������
		}
		else if(( pParams->m_clientIP == clientIP )//����ҵ��ظ��ͻ���IP�ͷ������˿ں���ɾ������,���Ҹ�λ�ÿ�������,������������
            &&(pParams->m_severPort == serverPort))
		{
			//deleteTask(ppParams,pParams);
			ppParams[i]->m_device_address = -1;//0;//��Ϊ0������ԭ��vxSvrMain��ָ�������еı��λ(��ͬ��resetTask)
			rt_thread_delay(100);   //�ó�CPU�����ظ�IP���߳��Լ�ɾ���Լ��������´���һ���µġ�
			return 1;
		}
	}
	return nRetVal;
}
//���浱ǰ�½������ӵ�״̬
static int32_t setConnState(PARAMS** ppParams,PARAMS* pParams)
{
	//�����������������������У��ҵ���λ�����������Ĳ����ṹ
	int32_t i;
	for(i=0;i<MAX_CLIENT_NUM;i++)
	{
		PARAMS* pParamsTemp =ppParams[i];

		if(pParamsTemp==0)
		{
			ppParams[i] = pParams;
			return 1;
		}
	}
	return 0;
}

//����STARTDT_CONFIRM
static int32_t sendStartDTConfirm(PARAMS* pParams)
{
	pParams->m_pCtrlMsg->m_byteStartChar=0x68;
	pParams->m_pCtrlMsg->m_byteApduLength=4;
	pParams->m_pCtrlMsg->m_wordSendNum=0x0B;
	pParams->m_pCtrlMsg->m_wordRecvNum=0;

	return(sendCtrlMsg(pParams));
}
//����STOPDT_CONFIRM
static int32_t sendStopDTConfirm(PARAMS* pParams)
{
	pParams->m_pCtrlMsg->m_byteStartChar=0x68;
	pParams->m_pCtrlMsg->m_byteApduLength=4;
	pParams->m_pCtrlMsg->m_wordSendNum=0x23;
	pParams->m_pCtrlMsg->m_wordRecvNum=0;

	return(sendCtrlMsg(pParams));
}
//����TESTFR_CONFIRM
static int32_t sendTESTFRConfirm(PARAMS* pParams)
{
	pParams->m_pCtrlMsg->m_byteStartChar=0x68;
	pParams->m_pCtrlMsg->m_byteApduLength=4;
	pParams->m_pCtrlMsg->m_wordSendNum=0x83;
	pParams->m_pCtrlMsg->m_wordRecvNum=0;

	return(sendCtrlMsg(pParams));
}
//����TESTFR
static int32_t sendTESTFRCmd(PARAMS* pParams)
{
	pParams->m_pCtrlMsg->m_byteStartChar=0x68;
	pParams->m_pCtrlMsg->m_byteApduLength=4;
	pParams->m_pCtrlMsg->m_wordSendNum=0x43;
	pParams->m_pCtrlMsg->m_wordRecvNum=0;

	return(sendCtrlMsg(pParams));
}
//��������
static int32_t centralProcess(PARAMS *pParams)
{
	int32_t nError = NO_ERROR;
//	char* pBuffer = (char*)(pParams->m_pRecvMsg);
	APDU* pApdu;
	APCI *pApci;

	if(!pParams)
	{
		return HAVE_ERROR;
	}

	if(pParams->m_pRecvMsg->m_APCI.m_byteApduLength == 4)//U��S
	{
		memcpy(pParams->m_pCtrlMsg,pParams->m_pRecvMsg,6);
		pApci = (APCI*)(pParams->m_pCtrlMsg);


   
		
		if(pApci->m_wordSendNum == 0x07)
		{
			rt_kprintf("���յ�STARTDT���ģ�\n");
			pParams->m_bRecvStartDT = TRUE;
			//pParams->tick_SOESend = rt_tick_get();
			if(sendStartDTConfirm(pParams) == HAVE_ERROR)
			{
				return HAVE_ERROR;
			}

			//SOE��λ
            if(!OP_EstablishLink(pParams->m_serverFd, pParams->m_clientIP))	//����SOE��Ϣ���ָ��
            {
                pParams->m_device_address = -1;
                return HAVE_ERROR;
            }

            if(pParams->m_severPort == OPERATOR_PORT)
            {
                return(ProcessDeviceState(pParams,IEC104_TI_M_SP_TB_1,IEC104_COT_M_spont));
            }
            else if(pParams->m_severPort == ENGINEER_PORT)
            {
                if(ProcessDeviceState(pParams,IEC103_TI_M_TM_TA_3,IEC103_COT_M_per)==HAVE_ERROR)//װ���Ƿ���Ҫһ����Ԫ��ַ�����ڶ�������
                {
                    return HAVE_ERROR;
                }
                return(ProcessAsdu05(pParams));
            }
		}
		else if((pParams->m_bRecvStartDT) && (pApci->m_wordSendNum == 0x13))
		{
//			rt_kprintf("���յ�STOPDT���ģ�\n");
			if(pParams->m_bWorkingPort == TRUE)
			{
				//logMsg("WorkingPort is FALSE:   busNo=%d, LinkSocket=%d!\n",pParams->m_nLinkSocket,pParams->m_nLinkSocket,0,0,0);
				pParams->m_bWorkingPort = FALSE;
			}

			return(sendStopDTConfirm(pParams));
		}
        else if((pApci->m_wordSendNum == 0x43))
		{
//			rt_kprintf("���յ�TESTFR���ģ�\n");

			return(sendTESTFRConfirm(pParams));
		}
        else if((pApci->m_wordSendNum == 0x83))
        {
            return NO_ERROR;
        }
		else if((pParams->m_bRecvStartDT) && (pApci->m_wordSendNum == 0x01))
		{
//			rt_kprintf("���յ�S_CONFIRM���ģ�\n");
			if(pParams->m_bWorkingPort == FALSE)
			{
				//logMsg("WorkingPort is TRUE:   busNo=%d, LinkSocket=%d!\n",pParams->m_nLinkSocket,pParams->m_nLinkSocket,0,0,0);
				pParams->m_bWorkingPort = TRUE;
			}

			if(pParams->m_nAutoConfirmed != SOE_CONFIRMED)
			{
				//if(pApci->m_wordRecvNum == pParams->m_nSendNum)
				if(pApci->m_wordRecvNum >= pParams->m_nSendNum_SOE)
				{
					logMsg("OP_SendSomeSOE success--S frame:  m_nAutoConfirmed=%d,m_wordRecvNum=%d,m_nSendNum=%d,m_nSendNum_SOE=%d\n",
                            pParams->m_nAutoConfirmed,pParams->m_pRecvMsg->m_APCI.m_wordRecvNum,pParams->m_nSendNum,pParams->m_nSendNum_SOE,0,0);
					OP_SendSomeSOE(pParams->m_nLinkSocket,pParams->m_nAutoConfirmed,pParams->soeInfo.soeCount);
					pParams->m_nAutoConfirmed = SOE_CONFIRMED;
					return NO_ERROR;
				}
				else
				{
					//logMsg("soe confirm is not fit--S frame:  m_nAutoConfirmed=%d,m_wordRecvNum=%d,m_nSendNum=%d\n",pParams->m_nAutoConfirmed,pApci->m_wordRecvNum,pParams->m_nSendNum,0,0,0);
				}
			}
		}
	}
    else if(pParams->m_bRecvStartDT)//&&(pParams->m_pRecvMsg->m_byteADD==pParams->m_device_address)) //I whs���̶���ַ����Ӧ���ٵ�ַ
	{
		pParams->m_device_address = pParams->m_pRecvMsg->m_byteADD ; //whs �������ٵ�ַ
		//rt_kprintf("****���յ�I���� pBuffer[1]=%d  ��ַ  %d;  ����  %d\n",pBuffer[1],pParams->m_pRecvMsg->m_byteADD,pParams->m_pRecvMsg->m_byteType);

		if(pParams->m_bWorkingPort == FALSE)
		{
			//logMsg("WorkingPort is TRUE:   busNo=%d, LinkSocket=%d!\n",pParams->m_nLinkSocket,pParams->m_nLinkSocket,0,0,0);
			pParams->m_bWorkingPort = TRUE;
		}
		//*/

		if(pParams->m_nAutoConfirmed != SOE_CONFIRMED)  //soeȷ���յ���ָ����λ
		{
			//if(pParams->m_pRecvMsg->m_APCI.m_wordRecvNum == pParams->m_nSendNum)
			if(pParams->m_pRecvMsg->m_APCI.m_wordRecvNum >= pParams->m_nSendNum_SOE)
			{
				logMsg("OP_SendSomeSOE success--I frame:  m_nAutoConfirmed=%d,m_wordRecvNum=%d,m_nSendNum=%d,m_nSendNum_SOE=%d\n",
                        pParams->m_nAutoConfirmed,pParams->m_pRecvMsg->m_APCI.m_wordRecvNum,pParams->m_nSendNum,pParams->m_nSendNum_SOE,0,0);
				OP_SendSomeSOE(pParams->m_nLinkSocket,pParams->m_nAutoConfirmed,pParams->soeInfo.soeCount);
				pParams->m_nAutoConfirmed = SOE_CONFIRMED;
			}
			else
			{
				//logMsg("soe confirm is not fit--I frame:  m_nAutoConfirmed=%d,m_wordRecvNum=%d,m_nSendNum=%d\n",pParams->m_nAutoConfirmed,pParams->m_pRecvMsg->m_APCI.m_wordRecvNum,pParams->m_nSendNum,0,0,0);
			}
		}

		pApdu = (APDU*)pParams->m_pSendMsg;

		//�÷��ͱ���ͷ����Ϣ
		pApdu->m_APCI.m_byteStartChar = 0x68;
		pApdu->m_APCI.m_byteApduLength = 0;

        if(pParams->m_severPort==ENGINEER_PORT)
        {
            if(GetARMSysConfigValue(ID_GET_JXSTRAP_STATE) == 0)   //maintenance strap off
            {
                switch(pParams->m_pRecvMsg->m_byteType)
                {
                case 6:         //Уʱ
                    nError = ProcessAsdu06(pParams);
                    break;

                case 7:         //����
                    nError = ProcessAsdu07(pParams);
                    break;

                case 20:        //���� ѹ�� ��ֵ��
                    nError = ProcessAsdu20(pParams);
                    break;

                case 64:        //��·��
                    nError = ProcessAsdu64(pParams);
                    break;

                case 61:        //������ֵ
                    nError = ProcessAsdu61(pParams);
                    break;

                case 62:		//get SOE
                    nError = ProcessAsdu62(pParams);
                    break;

                case 88:        //�����
                    nError = ProcessAsdu88(pParams);
                    break;

                //¼��
               /* case 24:        //¼������
                    pParams->tick_WaveStart = rt_tick_get();
                    nError=ProcessAsdu24(pParams);
                    break;

                case 25:		//�����Ͽɴ���
                    pParams->tick_WaveStart = rt_tick_get();
                    nError = ProcessAsdu25(pParams);
                    break;*///whs ��ʱ����      

                case 60:        //��汾��
                    nError = ProcessAsdu05(pParams);
                    break;

                default:        //��Ӧ��
                    nError = ProcessAsduNoACK(pParams);
                    break;
                }
            }
            else if(pParams->m_pRecvMsg->m_byteType == 7)         //����
            {
                nError = ProcessAsdu07(pParams);
            }
        }
        else if(pParams->m_severPort==OPERATOR_PORT)
        {
            if(GetARMSysConfigValue(ID_GET_JXSTRAP_STATE) == 0)   //maintenance strap off
            {
                switch(pParams->m_pRecvMsg->m_byteType)
                {
                case 103:         //Уʱ
                    nError = ProcessAsdu103(pParams);
                    break;

                case 100:         //����
                    nError = ProcessAsdu100(pParams);
                    break;

                case 45:
                case 46:        //��·�� ���� ѹ�� ��ֵ��
                    nError = ProcessAsdu46(pParams);
                    break;

                case 102:        //��ѯ������ֵ
                    nError = ProcessAsdu102(pParams);
                    break;

                case 49:        //�޸ĵ���������ֵ
                    nError = ProcessAsdu49(pParams);
                    break;

                case 101:        //�����
                    nError = ProcessAsdu101(pParams);
                    break;

                default:        //��Ӧ��
                    memcpy(pParams->m_pSendMsg,pParams->m_pRecvMsg,pParams->m_pRecvMsg->m_APCI.m_byteApduLength+2);
                    pParams->m_pSendMsg->m_wordCOT = 71;
                    nError = centralSend(pParams);
                    break;
                }
            }
            else if(pParams->m_pRecvMsg->m_byteType == 7)         //����
            {
                nError = ProcessAsdu100(pParams);
            }
        }
        else
        {
            nError = NO_ERROR;
        }
	}

	return nError;
}

static void tcp103CentralTask(void* parameter)
{
    CHILDTASKPAR* child = (CHILDTASKPAR*)parameter;
    int32_t		serverFd = child->serverFd;
    uint32_t 	clientAddr = child->s_addr;
    uint32_t  severPort = child->sin_port;
//    uint16_t  clientPort;
    PARAMS** ppParams = child->ppParams;
//    PORT_CON_STRUCT* thePortConfig = child->PortConfig;

	APDU	*pRecvMsg;	 //��ǰ��·�Ľ��ջ�����
	APDU	*pSendMsg;	 //��ǰ��·�ķ��ͻ�����
	APDU	*pSaveMsg;	 //��ǰ��·�ı��滺����
	APCI	*pCtrlMsg;	 //��ǰ��·�Ŀ�����Ϣ������
	PARAMS	*pParams;	 //�������ݽṹ��
	uint32_t tickRecvOld; //���ռ�ʱ��ǰֵ
	uint32_t tickRecvNew; //���ռ�ʱ����ǰֵ

	uint32_t tickYCOld,tickYCOld1;
	uint32_t tickYCNew;
	uint32_t tick_SOESendNew;

	uint8_t times_TESTFR=0;
	uint32_t tick_Interval;

	TDateTime tTime;

	tickRecvOld = rt_tick_get();
	tickYCOld = rt_tick_get();
	tickYCOld1 = rt_tick_get();

	rt_kprintf("tcp103CentralTask%d started\n",serverFd);
	rt_kprintf("tcp103CentralTask clientIP:  %d.%d.%d.%d\n",clientAddr & 0xff,(clientAddr >> 8) & 0xff,(clientAddr >> 16) & 0xff,(clientAddr >> 24) & 0xff);

	pRecvMsg = rt_malloc(sizeof(APDU));
	pSendMsg = rt_malloc(sizeof(APDU));
	pSaveMsg = rt_malloc(sizeof(APDU));
	pCtrlMsg = rt_malloc(sizeof(APCI));
	pParams	 = rt_malloc(sizeof(PARAMS));

	memset((char*)pRecvMsg, 0, sizeof(APDU));
	memset((char*)pSendMsg, 0, sizeof(APDU));
	memset((char*)pSaveMsg, 0, sizeof(APDU));
	memset((char*)pCtrlMsg, 0, sizeof(APCI));
	memset((char*)pParams, 0, sizeof(PARAMS));


	pParams->m_device_address = GetARMSysConfigValue(DI_GET_DEV_ADDR);
	pParams->m_nRecvNum = 0;
	pParams->m_nSendNum = 0;
	pParams->m_nConfirmedNum = 0;
	pParams->m_nSendNum_SOE = 0;
	pParams->m_pRecvMsg = pRecvMsg;
	pParams->m_pSendMsg = pSendMsg;
	pParams->m_pSaveMsg = pSaveMsg;
	pParams->m_pCtrlMsg = pCtrlMsg;

	pParams->m_serverFd = serverFd;
	pParams->m_clientIP = clientAddr; //���浱ǰ�������Ӧ�Ŀͻ���IP��ַ
	pParams->m_severPort = severPort; //���浱ǰ�������Ӧ�ķ������˿ں�
	pParams->m_taskID   = rt_thread_self();//��ǰ�����ID�ţ�����ɾ������

//    pParams->m_thePortConfig = thePortConfig;//

	if(!OP_EstablishLink(serverFd, clientAddr))	//����SOE��Ϣ���ָ��
	{
	    pParams->m_device_address = -1;
	}
	pParams->m_nLinkSocket = serverFd;
	pParams->m_nSendPacket = 0;
	pParams->m_nRecvPacket = 0;		//��ȷ���յ��ܰ���
	pParams->m_nErrPackets = 0;	//���մ���İ���

	pParams->m_bRecvStartDT = FALSE;
	pParams->m_bWorkingPort = FALSE;
	//pParams->m_bSOESend = FALSE;
	pParams->m_nAutoConfirmed = SOE_CONFIRMED;
	pParams->soeCnt = 0;

	if(!setConnState(ppParams,pParams))
	{
		exitTask(ppParams,pParams,"��������״̬�쳣�������˳�\n");
	}

	while(1)
    {
		// ����ͬIP��ַ���ӷ�����ͬһ�˿ڣ�ɾ�����߳�
		if(pParams->m_device_address == -1)
		{
			rt_kprintf("Link Error serverFd = %d",serverFd);
			exitTask(ppParams,pParams,"\n same port linked,exit this task\n");
			break;
		}

        //װ�õ�ַ�޸ģ�ɾ�����߳�
        if(pParams->m_device_address != GetARMSysConfigValue(DI_GET_DEV_ADDR))
		{
			exitTask(ppParams,pParams,"\n device address changed,exit this task 103\n");
			break;
		}


		//������ձ���
		if(selectRead(serverFd,READ_TIMEOUT) == OK)
		{
			tickRecvOld = rt_tick_get();
			times_TESTFR = 0;
			memset((char*)(pParams->m_pRecvMsg), 0, sizeof(APDU));
            if(centralRecv(pParams) == HAVE_ERROR)
			{
				rt_kprintf("port%d--103",pParams->m_nLinkSocket);
				exitTask(ppParams,pParams,"Read error 103\n");
			}
			else
			{
				//���������ͱ���
				if(centralProcess(pParams) == HAVE_ERROR)
				{
					exitTask(ppParams,pParams,"Process error\n");
				}
			}
		}
		else
		{
			tickRecvNew = rt_tick_get();
			tick_Interval = tickRecvNew - tickRecvOld;

            if(tick_Interval >= RT_TICK_PER_SECOND*35)	 //35s
            {
                sendTESTFRCmd(pParams);
                tickRecvOld = tickRecvNew;
                times_TESTFR++;
            }
			if(times_TESTFR >= 3)	 //3*35s
			{
				tTime = Now();
				logMsg("Time  = %d-%d-%d:%d:%d:%d!\n",tTime.month,tTime.day,tTime.hour,tTime.minute,tTime.msec/1000,tTime.msec%1000);//debug
				rt_kprintf("103����60����·�����ݽ��գ������˳�\n");
				exitTask(ppParams,pParams,"Recv Timer Error\n");
			}
		}
		//�����Է�������Ϣ
        //StartDT��ʼ�������ͣ�����12��δӦ��I���ĺ�ֹͣ����
		if((pParams->m_bRecvStartDT == TRUE)&&(pParams->m_nSendNum - pParams->m_nConfirmedNum < 24))    //12*2
		{
			/*
			if(pParams->m_bSOESend == FALSE)
			{
				tick_SOESendNew = rt_tick_get();
				tick_Interval = tick_SOESendNew - pParams->tick_SOESend;

				if(tick_Interval >= 400)
				{
					pParams->m_bSOESend = TRUE;
				}
			}
			*/

			if(pParams->m_nAutoConfirmed != SOE_CONFIRMED)
			{
				tick_SOESendNew = rt_tick_get();
				tick_Interval = tick_SOESendNew - pParams->tick_SOESend;

				//if(tick_Interval >= RT_TICK_PER_SECOND*20)//20s֮����ȷ������
				{
					OP_SendSomeSOE(pParams->m_nLinkSocket,pParams->m_nAutoConfirmed,pParams->soeInfo.soeCount);
					pParams->m_nAutoConfirmed = SOE_CONFIRMED;
				}
			}

			//SOE
			//if((pParams->m_bWorkingPort == TRUE) && (pParams->m_bSOESend == TRUE) && (pParams->m_nAutoConfirmed == SOE_CONFIRMED) && (JudgeNewSOE(pParams) == OK))
			if((pParams->m_bWorkingPort == TRUE) && (pParams->m_nAutoConfirmed == SOE_CONFIRMED) && (JudgeNewSOE(pParams) == OK))
			{
				rt_kprintf("new SOE ,m_nLinkSocket=%d, soeType=%d,��\n",pParams->m_nLinkSocket,pParams->soeInfo.soeType);
				if(ProcessSOE(pParams) == HAVE_ERROR)
				{
					exitTask(ppParams,pParams,"Auto ProcessSOE error\n");
				}
			}

			//�仯YC:Խ����+��ʱ
            tickYCNew = rt_tick_get();
            tick_Interval = tickYCNew - tickYCOld;
			if((tick_Interval >= RT_TICK_PER_SECOND)&&(GetARMSysConfigValue(ID_GET_JXSTRAP_STATE)==0))    //1s  maintenance strap off
            {
                tickYCOld = tickYCNew;
                tick_Interval = tickYCNew - tickYCOld1;
                if(tick_Interval >= RT_TICK_PER_SECOND*GetARMSysConfigValue(ID_GET_CYCLE_TIMES))//ѭ������ʱ��
                {
                    tickYCOld1 = tickYCNew;
                    rt_kprintf("time out send YC----- LinkSocket=%d!\n",pParams->m_nLinkSocket);
                    if(ProcessSendYC(pParams, 50, 2) == HAVE_ERROR)
                    {
                        exitTask(ppParams,pParams,"time out ProcessYC error\n");
                    }
                }
                else if(OP_GetThresholdSendMeas(pParams->m_nLinkSocket,50))//Խ�����ж�10%
                {
                    rt_kprintf("over threshold send YC-----LinkSocket=%d!\n",pParams->m_nLinkSocket);
                    if(ProcessSendYC(pParams, 50, 1) == HAVE_ERROR)
                    {
                        exitTask(ppParams,pParams,"over threshold ProcessYC error\n");

                    }
                }
            }

		}

		//wave timeout����
		if(pParams->waveTransferringFlag == TRUE)
		{
			tick_Interval = rt_tick_get() - pParams->tick_WaveStart;

			if(tick_Interval >= RT_TICK_PER_SECOND*10)//10s
			{
				rt_kprintf("�ٻ�¼�����ݳ�ʱ,LinkSocket=%d!\n",pParams->m_nLinkSocket);
                pParams->waveTransferringFlag = FALSE;
			}
		}

		rt_thread_delay(RT_TICK_PER_SECOND/100); // delay 10ms
    }
}

static void tcp104CentralTask(void* parameter)
{
    CHILDTASKPAR* child = (CHILDTASKPAR*)parameter;
    int32_t		serverFd = child->serverFd;
    uint32_t 	clientAddr = child->s_addr;
    uint32_t  severPort = child->sin_port;
//    uint16_t  clientPort;
    PARAMS** ppParams = child->ppParams;
//    PORT_CON_STRUCT* thePortConfig = child->PortConfig;

	APDU	*pRecvMsg;	 //��ǰ��·�Ľ��ջ�����
	APDU	*pSendMsg;	 //��ǰ��·�ķ��ͻ�����
	APDU	*pSaveMsg;	 //��ǰ��·�ı��滺����
	APCI	*pCtrlMsg;	 //��ǰ��·�Ŀ�����Ϣ������
	PARAMS	*pParams;	 //�������ݽṹ��
	uint32_t tickRecvOld; //���ռ�ʱ��ǰֵ
	uint32_t tickRecvNew; //���ռ�ʱ����ǰֵ

	uint32_t tickYCOld,tickYCOld1;
	uint32_t tickYCNew;
	uint32_t tick_SOESendNew;

	uint8_t times_TESTFR=0;
	uint32_t tick_Interval;

	TDateTime tTime;

	tickRecvOld = rt_tick_get();
	tickYCOld = rt_tick_get();
	tickYCOld1 = rt_tick_get();

	rt_kprintf("tcp104CentralTask%d started\n",serverFd);
	rt_kprintf("tcp104CentralTask clientIP:  %d.%d.%d.%d\n",clientAddr & 0xff,(clientAddr >> 8) & 0xff,(clientAddr >> 16) & 0xff,(clientAddr >> 24) & 0xff);

	pRecvMsg = rt_malloc(sizeof(APDU));
	pSendMsg = rt_malloc(sizeof(APDU));
	pSaveMsg = rt_malloc(sizeof(APDU));
	pCtrlMsg = rt_malloc(sizeof(APCI));
	pParams	 = rt_malloc(sizeof(PARAMS));

	memset((char*)pRecvMsg, 0, sizeof(APDU));
	memset((char*)pSendMsg, 0, sizeof(APDU));
	memset((char*)pSaveMsg, 0, sizeof(APDU));
	memset((char*)pCtrlMsg, 0, sizeof(APCI));
	memset((char*)pParams, 0, sizeof(PARAMS));


	pParams->m_device_address = GetARMSysConfigValue(DI_GET_DEV_ADDR);
	pParams->m_nRecvNum = 0;
	pParams->m_nSendNum = 0;
	pParams->m_nConfirmedNum = 0;
	pParams->m_nSendNum_SOE = 0;
	pParams->m_pRecvMsg = pRecvMsg;
	pParams->m_pSendMsg = pSendMsg;
	pParams->m_pSaveMsg = pSaveMsg;
	pParams->m_pCtrlMsg = pCtrlMsg;

	pParams->m_serverFd = serverFd;
	pParams->m_clientIP = clientAddr; //���浱ǰ�������Ӧ�Ŀͻ���IP��ַ
	pParams->m_severPort = severPort; //���浱ǰ�������Ӧ�ķ������˿ں�
	pParams->m_taskID   = rt_thread_self();//��ǰ�����ID�ţ�����ɾ������

//    pParams->m_thePortConfig = thePortConfig;//

	if(!OP_EstablishLink(serverFd, clientAddr))	//����SOE��Ϣ���ָ��
	{
	    pParams->m_device_address = -1;
	}
	pParams->m_nLinkSocket = serverFd;
	pParams->m_nSendPacket = 0;
	pParams->m_nRecvPacket = 0;		//��ȷ���յ��ܰ���
	pParams->m_nErrPackets = 0;	//���մ���İ���

	pParams->m_bRecvStartDT = FALSE;
	pParams->m_bWorkingPort = FALSE;
	//pParams->m_bSOESend = FALSE;
	pParams->m_nAutoConfirmed = SOE_CONFIRMED;
	pParams->soeCnt = 0;

	if(!setConnState(ppParams,pParams))
	{
		exitTask(ppParams,pParams,"��������״̬�쳣�������˳�\n");
	}

	while(1)
    {
		// ����ͬIP��ַ���ӷ�����ͬһ�˿ڣ�ɾ�����߳�
		if(pParams->m_device_address == -1)
		{
			rt_kprintf("Link Error serverFd = %d IEC104",serverFd);
			exitTask(ppParams,pParams,"\n same port linked,exit this task IEC104\n");
			break;
		}

        //װ�õ�ַ�޸ģ�ɾ�����߳�
   /* if(pParams->m_device_address != GetARMSysConfigValue(DI_GET_DEV_ADDR))
		{
			exitTask(ppParams,pParams,"\n device address changed,exit this task IEC104\n");
			break;
		}*///װ����Ϊ������ת��ֻ��IP û�е�ַ

    
		//������ձ���
		if(selectRead(serverFd,READ_TIMEOUT) == OK)
		{
			tickRecvOld = rt_tick_get();
			times_TESTFR = 0;
			memset((char*)(pParams->m_pRecvMsg), 0, sizeof(APDU));
      if(centralRecv(pParams) == HAVE_ERROR)
			{
				rt_kprintf("port%d--104",pParams->m_nLinkSocket);
				exitTask(ppParams,pParams,"Read error 104\n");
			}
			else
			{
				//ProcessAsdu100(pParams);
				//���������ͱ���
				if(centralProcess(pParams) == HAVE_ERROR)
				{
					exitTask(ppParams,pParams,"Process error\n");
				}
			}
		}
		else
		{
			tickRecvNew = rt_tick_get();
			tick_Interval = tickRecvNew - tickRecvOld;

			if(tick_Interval >= RT_TICK_PER_SECOND*35)	 //35s
      {
                sendTESTFRCmd(pParams);
                tickRecvOld = tickRecvNew;
                times_TESTFR++;
      }
			if(times_TESTFR >= 3)	 //3*35s
			{
				tTime = Now();
				logMsg("Time  = %d-%d-%d:%d:%d:%d!\n",tTime.month,tTime.day,tTime.hour,tTime.minute,tTime.msec/1000,tTime.msec%1000);//debug
				rt_kprintf("104����60����·�����ݽ��գ������˳�\n");
				exitTask(ppParams,pParams,"Recv Timer Error\n");
			}
		}
		//�����Է�������Ϣ
        //StartDT��ʼ�������ͣ�����12��δӦ��I���ĺ�ֹͣ����
		if((pParams->m_bRecvStartDT == TRUE)&&(pParams->m_nSendNum - pParams->m_nConfirmedNum < 24))    //12*2
		{


			if(pParams->m_nAutoConfirmed != SOE_CONFIRMED)//
			{
				tick_SOESendNew = rt_tick_get();
				tick_Interval = tick_SOESendNew - pParams->tick_SOESend;

				//if(tick_Interval >= RT_TICK_PER_SECOND*20)//20s֮����ȷ������
				{
					OP_SendSomeSOE(pParams->m_nLinkSocket,pParams->m_nAutoConfirmed,pParams->soeInfo.soeCount);
					pParams->m_nAutoConfirmed = SOE_CONFIRMED;
				}
			}
			//SOE
			//if((pParams->m_bWorkingPort == TRUE) && (pParams->m_bSOESend == TRUE) && (pParams->m_nAutoConfirmed == SOE_CONFIRMED) && (JudgeNewSOE(pParams) == OK))
			if((pParams->m_bWorkingPort == TRUE) && (pParams->m_nAutoConfirmed == SOE_CONFIRMED) && (JudgeNewSOE(pParams) == OK))
			{
				rt_kprintf("new SOE ,m_nLinkSocket=%d, soeType=%d,��\n",pParams->m_nLinkSocket,pParams->soeInfo.soeType);
				if(ProcessSOE(pParams) == HAVE_ERROR)
				{
					exitTask(ppParams,pParams,"Auto ProcessSOE error\n");
				}
			}

			//�仯YC:Խ����+��ʱ
            tickYCNew = rt_tick_get();
            tick_Interval = tickYCNew - tickYCOld;
			if((tick_Interval >= RT_TICK_PER_SECOND)&&(GetARMSysConfigValue(ID_GET_JXSTRAP_STATE)==0))    //1s  maintenance strap off
            {
                tickYCOld = tickYCNew;
                tick_Interval = tickYCNew - tickYCOld1;
                if(tick_Interval >= RT_TICK_PER_SECOND/5)//*GetARMSysConfigValue(ID_GET_CYCLE_TIMES))//ѭ������ʱ��//10s
                {
                    tickYCOld1 = tickYCNew;
                    //rt_kprintf("time out send YC----- LinkSocket=%d!\n",pParams->m_nLinkSocket);
                    if(ProcessSendYC(pParams, IEC104_TI_M_ME_NA_1, IEC104_COT_M_cyc) == HAVE_ERROR)
                    {
                        exitTask(ppParams,pParams,"time out ProcessYC error\n");
                    }
                }
								
                /*else if(OP_GetThresholdSendMeas(pParams->m_nLinkSocket,9))//Խ�����ж�10%
                {
                    rt_kprintf("over threshold send YC-----LinkSocket=%d!\n",pParams->m_nLinkSocket);
                    if(ProcessSendYC(pParams, IEC104_TI_M_ME_NA_1, IEC104_COT_M_spont) == HAVE_ERROR)
                    {
                        exitTask(ppParams,pParams,"over threshold ProcessYC error\n");

                    }
                }*/
            }

		}
		rt_thread_delay(RT_TICK_PER_SECOND/100); // delay 10ms
    }
}


PARAMS* ppParams[MAX_CLIENT_NUM];//����ÿ�������PARAMS�����ṹ��ָ������
/*������ʵ��*/
void TCP103CommuTsk(void *parameter)
{

	struct ip_addr ip_addr;
	char* netif_name =  (char*)parameter;

	//PARAMS* ppParams[MAX_CLIENT_NUM];//����ÿ�������PARAMS�����ṹ��ָ������

	struct sockaddr_in  serverAddr; /*Server��ַ*/
	uint32_t       serverPort=ENGINEER_PORT;/*Server�˿ں�*/

	struct sockaddr_in  clientAddr; /*���ӵĿͻ��˵�IP��ַ*/

	char	taskName[20];
	rt_thread_t	nTaskID;
	CHILDTASKPAR childParam;

	int32_t     listenFd;/* listen socket file descriptor */
	int32_t     serverFd;/* server socket file descriptor */

	u32_t		addrsize;//��ַ�ṹ��Ĵ�С
	int32_t     blockflag = 1;

	int32_t		j=0;
//    int32_t  optval = 1;
	TDateTime tTime;
//	int32_t nNetTimeout=1;//1ms

	rt_kprintf("%s Server Main Task Started\n", netif_name);

	//��ʼ������ÿ�������PARAMS�����ṹ��ָ������ȫΪ0ֵ
	for(j=0;j<MAX_CLIENT_NUM;j++)
	{
		ppParams[j] = 0;
	}

    get_if(netif_name, &ip_addr, RT_NULL, RT_NULL);// IP��ַ

	addrsize = sizeof(struct sockaddr_in);
	memset((char*)&serverAddr,0,sizeof(struct sockaddr_in));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_len = (uint8_t)sizeof(struct sockaddr_in);
	serverAddr.sin_port = htons(serverPort);
	serverAddr.sin_addr.s_addr = (ip_addr.addr);//htonl(INADDR_ANY);

//	serverAddr.sin_addr.s_addr = inet_addr("192.168.0.30");

	if( (listenFd = socket(AF_INET,SOCK_STREAM,0)) == ERROR )
	{
		rt_kprintf("socket create error\n");
		return;// ERROR;
	}

	//ע�⿼�������С��Ϊ���ٺ���
	//if(setsockopt(listenFd,SOL_SOCKET,SO_SNDBUF,(char*)&buffersize,sizeof(buffersize)) == ERROR)
	{
	//	rt_kprintf("setsockopt send buffer size\n");
	//	return ERROR;
	}

    //����ʱ��
    //setsockopt(listenFd,SOL_SOCKET,SO_SNDTIMEO,(char *)&nNetTimeout,sizeof(int32_t));


	//if(setsockopt(listenFd,SOL_SOCKET,SO_RCVBUF,(char*)&buffersize,sizeof(buffersize)) == ERROR)
	{
	//	rt_kprintf("setsockopt recv buffer size\n");
	//	return ERROR;
	}

//	if(lwip_ioctl(listenFd,FIONBIO,&blockflag)<0)   //������// no use whs
//	{
//		rt_kprintf("socket ioctl error\n");
//		lwip_close(listenFd);
//		return;// ERROR;
//	}

	if(bind(listenFd,(struct sockaddr*)&serverAddr,sizeof(struct sockaddr_in)) == ERROR)
	{
		rt_kprintf("bind error\n");
		lwip_close(listenFd);
		return;// ERROR;
	}

	if(listen(listenFd,8) == ERROR)
	{
		rt_kprintf("listen error\n");
		lwip_close(listenFd);
		return;// ERROR;
	}

	while(1)
    {
		//accept��δ���ᵼ��ERRNO��46
		if (((serverFd = accept (listenFd, (struct sockaddr *) &clientAddr, &addrsize )) == ERROR)
            && (errno != EWOULDBLOCK ))
        {
            rt_kprintf ("accept listenFd(%d) error(%d)\n",listenFd,errno);
            //lwip_close (listenFd);
            //return (ERROR);
            continue;
        }
        if (serverFd >= 0)
        {
			rt_kprintf("\n\n�����ӵĿͻ��˵�ַΪ: %x \n\n",clientAddr.sin_addr.s_addr);
//////////////////////////////////////////////////////////////
            if(lwip_ioctl(serverFd,FIONBIO,&blockflag)<0)   //����serverFdΪ������
            {
                rt_kprintf("socket ioctl error\n");
                lwip_close(serverFd);
                continue;
            }

 /*           // set socket keep alive.
            if(lwip_setsockopt(serverFd, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval)) < 0)
            {
                rt_kprintf("SO_KEEPALIVE failed\r\n");
                lwip_close(serverFd);
                continue;
            }
*/
///////////////////////////////////////////////////////////////
			//��鲢���øõ�ַ�Ŀͻ��˵����ӣ�״̬,ͬʱ����������
			//���������Ʋ�������accept֮ǰ���У�����������������ʱ���κ�һ�����Ӷ�����ͨ��ɾ��ǰһ���ӣ�����������
			if(!getConnState(ppParams,clientAddr.sin_addr.s_addr,serverPort))
			{
				rt_kprintf("�Ѵﵽ���������:%d.����������\n",MAX_CLIENT_NUM);
				lwip_close(serverFd);
				rt_thread_delay(100);
				NVIC_SystemReset();//ʵ��ϵͳ��λ
				continue;
			}

			rt_kprintf("A new client connected! serverFd = %d\n",serverFd);

			tTime = Now();
			logMsg("Time  = %02d-%02d-%02d:%02d:%02d:%03d!\n",tTime.month,tTime.day,tTime.hour,tTime.minute,tTime.msec/1000,tTime.msec%1000);//debug

			sprintf(taskName,"Client%d",serverFd);

			childParam.serverFd = serverFd;//����������ͨѶ��Socket
			childParam.s_addr   = clientAddr.sin_addr.s_addr;//�ͻ���(����̨��)��IP��ַ
			childParam.sin_port = serverPort;//��Ӧ������(��װ��)��Socket�˿ں�
//			childParam.sin_port = clientAddr.sin_port;//�ͻ���(����̨��)��Socket�˿ں�
			childParam.ppParams = ppParams;//���������������Ĳ����ṹ���ָ������
//			childParam.PortConfig = pPortConfig;//�˿����ò���

			nTaskID = rt_thread_create( taskName,
										tcp103CentralTask, (void *)&childParam,
										MAX_CHILDPORT_STACK, 20, 10);

			if (nTaskID != RT_NULL)
            {
                rt_thread_startup(nTaskID);
                rt_kprintf ("rt_thread_create Succeeded\n");
            }
			else
			{
				rt_kprintf ("rt_thread_create Failed\n");
				lwip_close(serverFd);
			}
        }
 //       else
//		{
//			rt_thread_delay(RT_TICK_PER_SECOND/100); // delay 10ms
//		}
    }
}

//104
void TCP104CommuTsk(void *parameter)
{

	struct ip_addr ip_addr;
	char* netif_name =  (char*)parameter;

	struct sockaddr_in  serverAddr; /*Server��ַ*/
	uint32_t       serverPort=OPERATOR_PORT;/*Server�˿ں�*/

	struct sockaddr_in  clientAddr; /*���ӵĿͻ��˵�IP��ַ*/

	char	taskName[20];
	rt_thread_t	nTaskID;
	CHILDTASKPAR childParam;

	int32_t     listenFd;/* listen socket file descriptor */
	int32_t     serverFd;/* server socket file descriptor */

	u32_t		addrsize;//��ַ�ṹ��Ĵ�С
	int32_t     blockflag = 1;

	int32_t		j=0;
//    int32_t  optval = 1;
	TDateTime tTime;
//	int32_t nNetTimeout=1;//1ms

	rt_kprintf("%s Server Main Task Started IEC104\n", netif_name);

	//��ʼ������ÿ�������PARAMS�����ṹ��ָ������ȫΪ0ֵ
	for(j=0;j<MAX_CLIENT_NUM;j++)
	{
		ppParams[j] = 0;
	}

    get_if(netif_name, &ip_addr, RT_NULL, RT_NULL);// IP��ַ

	addrsize = sizeof(struct sockaddr_in);
	memset((char*)&serverAddr,0,sizeof(struct sockaddr_in));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_len = (uint8_t)sizeof(struct sockaddr_in);
	serverAddr.sin_port = htons(serverPort);
	serverAddr.sin_addr.s_addr = (ip_addr.addr);//htonl(INADDR_ANY);

	if( (listenFd = socket(AF_INET,SOCK_STREAM,0)) == ERROR )
	{
		rt_kprintf("socket create error\n");
		return;// ERROR;
	}

//	if(lwip_ioctl(listenFd,FIONBIO,&blockflag)<0)   //������
//	{
//		rt_kprintf("socket ioctl error\n");
//		lwip_close(listenFd);
//		return;// ERROR;
//	}

	if(bind(listenFd,(struct sockaddr*)&serverAddr,sizeof(struct sockaddr_in)) == ERROR)
	{
		rt_kprintf("bind error\n");
		lwip_close(listenFd);
		return;// ERROR;
	}

	if(listen(listenFd,8) == ERROR)
	{
		rt_kprintf("listen error\n");
		lwip_close(listenFd);
		return;// ERROR;
	}

	while(1)
    {
		//accept��δ���ᵼ��ERRNO��46
		if (((serverFd = accept (listenFd, (struct sockaddr *) &clientAddr, &addrsize )) == ERROR)
            && (errno != EWOULDBLOCK ))
        {
            rt_kprintf ("accept listenFd(%d) error(%d) IEC104\n",listenFd,errno);
            //lwip_close (listenFd);
            //return (ERROR);
            continue;
        }
        if (serverFd >= 0)
        {
			rt_kprintf("\n\nIEC104 �����ӵĿͻ��˵�ַΪ: %x \n\n",clientAddr.sin_addr.s_addr);
//////////////////////////////////////////////////////////////
            if(lwip_ioctl(serverFd,FIONBIO,&blockflag)<0)   //����serverFdΪ������
            {
                rt_kprintf("socket ioctl error IEC104\n");
                lwip_close(serverFd);
                continue;
            }

 /*           // set socket keep alive.
            if(lwip_setsockopt(serverFd, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval)) < 0)
            {
                rt_kprintf("SO_KEEPALIVE failed\r\n");
                lwip_close(serverFd);
                continue;
            }
*/
///////////////////////////////////////////////////////////////
			//��鲢���øõ�ַ�Ŀͻ��˵����ӣ�״̬,ͬʱ����������
			//���������Ʋ�������accept֮ǰ���У�����������������ʱ���κ�һ�����Ӷ�����ͨ��ɾ��ǰһ���ӣ�����������
			if(!getConnState(ppParams,clientAddr.sin_addr.s_addr,serverPort))
			{
				rt_kprintf("�Ѵﵽ���������:%d.���������� IEC104\n",MAX_CLIENT_NUM);
				lwip_close(serverFd);
				rt_thread_delay(100);//whs20160201
				NVIC_SystemReset();//ʵ��ϵͳ��λ
				continue;
			}

			rt_kprintf("A new client connected! serverFd = %d IEC104\n",serverFd);

			tTime = Now();
			logMsg("Time  = %02d-%02d-%02d:%02d:%02d:%03d!\n",tTime.month,tTime.day,tTime.hour,tTime.minute,tTime.msec/1000,tTime.msec%1000);//debug

			sprintf(taskName,"Client%d",serverFd);

			childParam.serverFd = serverFd;//����������ͨѶ��Socket
			childParam.s_addr   = clientAddr.sin_addr.s_addr;//�ͻ���(����̨��)��IP��ַ
			childParam.sin_port = serverPort;//��Ӧ������(��װ��)��Socket�˿ں�
//			childParam.sin_port = clientAddr.sin_port;//�ͻ���(����̨��)��Socket�˿ں�
			childParam.ppParams = ppParams;//���������������Ĳ����ṹ���ָ������
//			childParam.PortConfig = pPortConfig;//�˿����ò���

			nTaskID = rt_thread_create( taskName,
										tcp104CentralTask, (void *)&childParam,
										MAX_CHILDPORT_STACK, 20, 10);

			if (nTaskID != RT_NULL)
            {
                rt_thread_startup(nTaskID);
                rt_kprintf ("rt_thread_create Succeeded\n");
            }
			else
			{
				rt_kprintf ("rt_thread_create Failed\n");
				lwip_close(serverFd);
			}
        }
    //    else
		//{
		//	rt_thread_delay(RT_TICK_PER_SECOND/100); // delay 10ms
		//}
    }
}




void eth_server103(void)
{
	rt_thread_t tid;
//	rt_kprintf(" build time: %s %s\n", __DATE__, __TIME__);


	tid = rt_thread_create("ServerA4", TCP104CommuTsk, (void*)"e0",
		MAX_CHILDPORT_STACK, 21, 5);
	if (tid != RT_NULL) rt_thread_startup(tid);

	
	tid = rt_thread_create("RtdbProce", RtdbProcess, RT_NULL,
		512*2, 21, 5);
	if (tid != RT_NULL) rt_thread_startup(tid);
}

