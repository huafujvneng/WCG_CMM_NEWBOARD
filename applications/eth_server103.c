/**
 * File    	:  eth_server103.c
 * author	:  mayunliang
 * This file is part of WGB-900
 * Copyright (C), 2012, XJ ELECTRIC Co., LTD.
 * Date     :  2012-07-03
 * Function	:  TCP103通讯

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
//  数据结构相关部分
************************************************/
#define	 OPERATOR_PORT 						2404						//normal connection port
#define	 ENGINEER_PORT						2403						//Engineer connection port
#define	 DEBUG_PORT							  4661						//debug information port



//rtSvrMain
#define	MAX_CLIENT_NUM			8
#define	MAX_CHILDPORT_STACK		2408//1024//4096//子端口任务堆栈大小



//tcp103CentralTask
#define READ_TIMEOUT            100
#define WRITE_TIMEOUT           1000
#define APDU_HEAD_LENGTH		10
#define ASDU_COPY_LENGTH		10
#define	DEVICE_NAME_LENGTH		8
#define	VALUE_EACH_FRAME		60    //每帧中的定值最大数量
#define SOE_CONFIRMED			-1

//#define WAVE_PAG_LENGTH			200

//#define MSG_TYPE		//显示报文类型
//#define MSG_ERROR		//显示错误信息
//#define MSG_BUFFER	//显示报文内容

#define MAKEWORD(a,b) ( ((a)<<8)+(b) )
#define LOBYTE(a) ( (a)&0x0ff )
#define HIBYTE(a) ( ((a)>>8)&0x0ff )
#define LOWORD(a) ( (a)&0x0ffff )
#define HIWORD(a) ( ((a)>>16)&0x0ffff )

#define logMsg rt_kprintf



#define YK_DL_END_INDEX 0x6100

//APCI结构定义
typedef struct _APCI
{
	uint8_t	m_byteStartChar;	//启动字符
	uint8_t	m_byteApduLength;	//APDU长度
	uint16_t	m_wordSendNum;		//发送序列号
	uint16_t	m_wordRecvNum;		//接收序列号
}APCI;

//APDU结构定义
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
	APCI	m_APCI;				//控制域

	uint8_t  m_byteType;
	uint8_t  m_byteVSQ;
	uint16_t m_wordCOT;
	uint8_t  m_byteCOMADD;		//单元公共地址
	uint8_t  m_byteADD;         //装置地址

	uint8_t	m_msgdata[MAX_MSG];//数据区(从这里开始104和103就不一样了)
}APDU;

typedef struct _SOEINFO
{
	uint8_t soeCount;                 //需通讯上送的SOE个数
	int8_t soeType;                   //需通讯上送的SOE类型
}SOEINFO;

typedef struct _WAVEDATAFILE
{
	int32_t	packageCnt;					//录波数据总包数
	int32_t	waveLen;					//录波数据文件总长度
	uint8_t autoSendFlag;             //主动上送标记
	uint8_t pad1;
}WAVEDATAFILE;


//通讯参数结构
typedef struct _PARAMS
{
	//装置地址
	int32_t		m_device_address;

	//连接标志
	int32_t		m_serverFd;		//任务socket标志符
	int32_t		m_nLinkSocket;	//表示处理SOE信息时的链路号
	rt_thread_t	m_taskID;	//任务ID号
	uint32_t	m_clientIP;		//客户端IP地址
	uint32_t  m_severPort;    //服务器端口

	//缓冲区指针
	APDU*	m_pSendMsg;		//发送缓冲区
	APDU*	m_pRecvMsg;		//接收缓冲区
	APDU*	m_pSaveMsg;		//保存接收缓冲区
	APCI*	m_pCtrlMsg;		//控制报文缓冲区

	uint16_t	m_nSendNum;		//发送序列号
	uint16_t	m_nRecvNum;		//接收序列号
	uint16_t	m_nConfirmedNum;//已被确认的序列号
	uint16_t	m_nSendNum_SOE;	//保存SOE发送时的发送序列号

	int32_t		m_nSendPacket;	//发送包数
	int32_t		m_nRecvPacket;	//正确接收的包数
	int32_t		m_nErrPackets;	//接收错误的包数

	//控制信息
	uint8_t	m_bRecvStartDT;	//是否收到启动报文
	uint8_t	m_bWorkingPort;	//是否工作端口
//    uint8_t	m_bSOESend;	    //是否发送SOE
	uint32_t  tick_SOESend;

	//SOE确认标志
	int32_t		m_nAutoConfirmed;//-1表示已经确认

	//wusenlin add
	//PORT_CON_STRUCT *m_thePortConfig;

	//yantianjun add
    //SOE
	SOEINFO soeInfo;

	//录波数据
	uint8_t waveTransferringFlag;		//正在传输标志
	uint8_t waveChannelName;          //传输的通道名称
	uint32_t tick_WaveStart;

	int32_t soeCnt; //debug 检测SOE个数,ASDU41、ASDU2、ASDU1、ASDU80
}PARAMS;

//通讯子任务参数结构
typedef struct _CHILDTASKPAR
{
    int32_t serverFd;
    uint32_t s_addr;
    uint16_t sin_port;
    PARAMS** ppParams;//主任务管理子任务的指针数组
//    PORT_CON_STRUCT *PortConfig;//通讯配置参数
}CHILDTASKPAR;
//static int32_t DICnt =0 ; //debug 检测SOE个数,ASDU41、ASDU2、ASDU1、ASDU80
/*************************************************
//  处理函数相关部分
************************************************/
static int32_t centralSend(PARAMS *pParams);
static int32_t centralRecv(PARAMS *pParams);
static int32_t centralProcess(PARAMS *pParams);


//#include "inetLib.h"

/*定义通用的函数接口*/
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
	rt_kprintf(" ");//在标签后加个空格

	for(i = 0; i < dataLen; i++)
	{
		sprintf((char*)chaBuffer, "%02x", buf[i]);
		printBuffer[i * 3] = chaBuffer[0];
		printBuffer[i * 3 + 1] = chaBuffer[1];
		printBuffer[i * 3 + 2] = 0x20;//空格
	}
	i -= 1;
	printBuffer[i * 3 + 3] = 0x0a;//换行

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
  

	//发送报文
	while(nSentLen<nLength)
	{
		if(selectWrite(pParams->m_serverFd,WRITE_TIMEOUT) == OK)
		{
			count=send(pParams->m_serverFd,//send() is  lwip api
				((char *)(pParams->m_pSendMsg))+nSentLen,//发送缓冲区数据指针向后偏移SendLen(已发送长度)
				(int32_t)(nLength-nSentLen),//发送长度为总长度减去已发送长度
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

	//打印报文
#ifdef MSG_BUFFER
	rt_kprintf("port%d--",pParams->m_nLinkSocket);
	PrintfMessage("[Send]",pBuffer,nLength);
#endif
	//报文监视
	//logMsg("port%d[Send]--SN = %04x,RN = %04x\n",pParams->m_nLinkSocket,pParams->m_nSendNum,pParams->m_nRecvNum,0,0,0);
	eth_status.eth_trans_flg = 1;
	//eth_status.eth_trans_buf
	rt_memcpy(&eth_status.eth_trans_buf[10], pParams->m_pSendMsg,nSentLen);
    //wusenlin
	//统计信息
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
		recv_count=recv(pParams->m_serverFd,&pBuffer[0],2,0);//先把前两个字节接收了
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
            // 由于是非阻塞的模式,所以当errno为EAGAIN（=EWOULDBLOCK）时,表示当前缓冲区已无数据可读
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
            // 到这里表示对端的socket已正常关闭.
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
            // 由于是非阻塞的模式,所以当errno为EAGAIN（=EWOULDBLOCK）时,表示当前缓冲区已无数据可读
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
            // 到这里表示对端的socket已正常关闭.
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

	//校验接收和发送序列号
	bCtrlMsg = (pParams->m_pRecvMsg->m_APCI.m_byteApduLength==4)?TRUE:FALSE;
	if( (!bCtrlMsg) && (pParams->m_pRecvMsg->m_APCI.m_wordSendNum > pParams->m_nRecvNum) )
	{
		logMsg("centralRecv error-3!\n",0,0,0,0,0,0);
		tTime = Now();
		logMsg("Time  = %d-%d-%d:%d:%d:%d!\n",tTime.month,tTime.day,tTime.hour,tTime.minute,tTime.msec/1000,tTime.msec%1000);//debug

		pParams->m_nErrPackets++;		//记录错误的包数
		return (HAVE_ERROR);
	}

	pParams->m_nRecvPacket++;				//记录正确的包数

	if(!bCtrlMsg)
	{
		pParams->m_nRecvNum += 2;
	}

    if((pParams->m_pRecvMsg->m_APCI.m_wordSendNum & 0x03) != 0x03)//非U报文
    {
        pParams->m_nConfirmedNum = pParams->m_pRecvMsg->m_APCI.m_wordRecvNum;
    }
//报文监视
		eth_status.eth_recv_flg = 1;
		
		rt_memcpy(&eth_status.eth_recv_buf[10],pBuffer,pBuffer[1]+2);
	//打印报文
#ifdef MSG_BUFFER
	rt_kprintf("port%d--",pParams->m_nLinkSocket);
	PrintfMessage("[Recv]",pBuffer,pBuffer[1]+2);
#endif
	//logMsg("port%d[Recv]--SN = %04x,RN = %04x\n",pParams->m_nLinkSocket,pParams->m_pRecvMsg->m_APCI.m_wordSendNum,pParams->m_pRecvMsg->m_APCI.m_wordRecvNum,0,0,0);

	//wusenlin
	//记录接收的报文数和字节数
	pLinkPortParam = GetLinkPortParam( pParams->m_nLinkSocket );
    pLinkPortParam->nRxPacks = pParams->m_nRecvPacket;

	return (NO_ERROR);
}

//发送控制类消息
static int32_t sendCtrlMsg(PARAMS* pParams)
{
	int32_t count=0;
	int32_t nSentLen = 0;
	int32_t cannotwriteCnt = 0;

	//char* pBuffer = (char*)(pParams->m_pCtrlMsg);
	TPORT_PARAM_STRUCT* pLinkPortParam;
	TDateTime tTime;

	//发送报文
	while(nSentLen<6)
	{
		if(selectWrite(pParams->m_serverFd,WRITE_TIMEOUT) == OK)
		{
			count=send(pParams->m_serverFd,
				((char *)(pParams->m_pCtrlMsg))+nSentLen,//发送缓冲区数据指针向后偏移SendLen(已发送长度)
				(int32_t)(6-nSentLen),//发送长度为总长度减去已发送长度
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

	//打印报文
#ifdef MSG_BUFFER
	rt_kprintf("port%d--",pParams->m_nLinkSocket);
	PrintfMessage("[Send]",pBuffer,6);
#endif

	//wusenlin
	//统计信息
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


//装置通讯状态
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
        pSendMsg->m_msgdata[nIndex++] = 1;//通讯正常 双点信息

        dtime = Now(); //获取当前时间
        nMillis = dtime.msec + dtime.second*1000;
        pSendMsg->m_msgdata[nIndex++] = LOBYTE(nMillis);//毫秒低字节
        pSendMsg->m_msgdata[nIndex++] = HIBYTE(nMillis);//毫秒高字节
        pSendMsg->m_msgdata[nIndex++] = dtime.minute;//分
        pSendMsg->m_msgdata[nIndex++] = dtime.hour;//时

        //附加信息SIN,自发上送时
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
        pSendMsg->m_msgdata[nIndex++] = 0x00;//通讯正常(单点)
        if(uiCOT == IEC104_COT_M_spont) //自发
        {
            pSendMsg->m_byteVSQ |= 0x80;

            dtime = Now(); //获取当前时间
            nMillis = dtime.msec + dtime.second*1000;
            pSendMsg->m_msgdata[nIndex++] = LOBYTE(nMillis);//毫秒低字节
            pSendMsg->m_msgdata[nIndex++] = HIBYTE(nMillis);//毫秒高字节
            pSendMsg->m_msgdata[nIndex++] = dtime.minute;//分
            pSendMsg->m_msgdata[nIndex++] = dtime.hour;//时
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
        if(nCOT==2)//循环上送
        {
            pSendMsg->m_byteVSQ = MeasValCnt;
            pSendMsg->m_msgdata[INF_103] = pSendMeas->measID;
            nindex = MSG_103;
            for(i=0; i<MeasValCnt; i++,pSendMeas++)
            {
                pSendMsg->m_msgdata[nindex++] = LOBYTE(pSendMeas->measVal);
                pSendMsg->m_msgdata[nindex++] = HIBYTE(pSendMeas->measVal);
                pSendMeas->bAutoSendFlg &= ~(0x01<<pParams->m_nLinkSocket);//清突变上送标记
            }
        }
        else    //突变上送
        {
            nindex = MSG_103-1;
            for(i=0; i<MeasValCnt; i++,pSendMeas++)
            {
                if(pSendMeas->bAutoSendFlg&(0x01<<pParams->m_nLinkSocket))
                {
                    pSendMsg->m_msgdata[nindex++] = pSendMeas->measID;
                    pSendMsg->m_msgdata[nindex++] = LOBYTE(pSendMeas->measVal);
                    pSendMsg->m_msgdata[nindex++] = HIBYTE(pSendMeas->measVal);
                    pSendMeas->bAutoSendFlg &= ~(0x01<<pParams->m_nLinkSocket);//清突变上送标记
                }
            }
            pSendMsg->m_byteVSQ = (nindex-1)/3|0x80;
        }
    }
    else*/ if(nFUN == 9)//遥测数据采用循环上送，不用突变
    {
        if(nCOT==1)//循环上送
        {
            nindex = MSG_104;
            pSendMsg->m_byteVSQ = MeasValCnt|0x80;
            pSendMsg->m_msgdata[INF_104L] = 0x01;//pSendMeas->measID-91;   //103从92开始 104从0x4001开始
            pSendMsg->m_msgdata[INF_104M] = 0x40;
            pSendMsg->m_msgdata[INF_104H] = 0x00;
		

					  for(i=0; i<MeasValCnt; i++)//MeasValCnt
            {
                pSendMsg->m_msgdata[nindex++] = LOBYTE(yc_message_buf[i+2])&0xff;
                pSendMsg->m_msgdata[nindex++] = HIBYTE(yc_message_buf[i+2]);
							QDS103 = LOBYTE(yc_message_buf[i+2])&0x07;
							  
                
                QDS104 = 0;
                if(QDS103&0x01)//溢出位//
                {
                    QDS104 |= 0x01;
                }
                if(QDS103&0x02)//有效位///
                {
                    QDS104 |= 0x80;
                }
                if(QDS103&0x04)///用103的备用位作为101的取代位//
                {
                    QDS104 |= 0x10;
                }
                pSendMsg->m_msgdata[nindex++] = QDS104;
                pSendMeas->bAutoSendFlg &= ~(0x01<<pParams->m_nLinkSocket);//清突变上送标记
            }
						
        }
        else if(nCOT==3)//突变上送// no use
        {
            nindex = INF_104L;
            for(i=0; (i<MeasValCnt)&&(i<40); i++,pSendMeas++)
            {
                if(pSendMeas->bAutoSendFlg&(0x01<<pParams->m_nLinkSocket))
                {
                    pSendMsg->m_msgdata[nindex++] = pSendMeas->measID-91;   //0x4001开始
                    pSendMsg->m_msgdata[nindex++] = 0x40;
                    pSendMsg->m_msgdata[nindex++] = 0x00;
                    pSendMsg->m_msgdata[nindex++] = LOBYTE(pSendMeas->measVal)&0xff;
                    pSendMsg->m_msgdata[nindex++] = HIBYTE(pSendMeas->measVal);
                    QDS103 = LOBYTE(pSendMeas->measVal)&0x07;
                    QDS104 = 0;
                    if(QDS103&0x01)/*溢出位*/
                    {
                        QDS104 |= 0x01;
                    }
                    if(QDS103&0x02)/*有效位*/
                    {
                        QDS104 |= 0x80;
                    }
                    if(QDS103&0x04)/*用103的备用位作为101的取代位*/
                    {
                        QDS104 |= 0x10;
                    }
                    pSendMsg->m_msgdata[nindex++] = QDS104;
                    pSendMeas->bAutoSendFlg &= ~(0x01<<pParams->m_nLinkSocket);//清突变上送标记
                }
            }
            pSendMsg->m_byteVSQ = nindex/6;
        }
    }

	pSendMsg->m_APCI.m_byteStartChar = 0x68;
	pSendMsg->m_APCI.m_byteApduLength = APDU_HEAD_LENGTH + nindex;

	return centralSend(pParams);
  
}

//判断是否发生SOE
static int32_t JudgeNewSOE(PARAMS *pParams)
{
	uint8_t	soeCount = 0;          //查询得到的SOE数量
	int8_t	soeType  = -1;         //SOE类型

	OP_GetSOEQueueSendType(pParams->m_nLinkSocket, &soeType, &soeCount);

	if(soeCount)
	{
		pParams->soeInfo.soeCount = soeCount;
		pParams->soeInfo.soeType = soeType;

/*		if(soeType == SOE_WAVE)
		{
			if(pParams->waveTransferringFlag == TRUE) //上送录波过程中不再处理新的录波
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

//保护动作 -103
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

	pSendMsg->m_msgdata[nIndex++] = pActionSOE->nDPI&0x03;//双点信息

	//相对时间
	pSendMsg->m_msgdata[nIndex++] = LOBYTE(pActionSOE->nPosTime);//低字节
	pSendMsg->m_msgdata[nIndex++] = HIBYTE(pActionSOE->nPosTime);//高字节

	//故障序号
	pSendMsg->m_msgdata[nIndex++] = LOBYTE(pActionSOE->nFAN);//低字节
	pSendMsg->m_msgdata[nIndex++] = HIBYTE(pActionSOE->nFAN);//高字节

	//获取时间
	nMillis = pActionSOE->dtTime.msec + pActionSOE->dtTime.second*1000;
	pSendMsg->m_msgdata[nIndex++] = LOBYTE(nMillis);//毫秒低字节
	pSendMsg->m_msgdata[nIndex++] = HIBYTE(nMillis);//毫秒高字节
	pSendMsg->m_msgdata[nIndex++] = pActionSOE->dtTime.minute;//分
	pSendMsg->m_msgdata[nIndex++] = pActionSOE->dtTime.hour;//时
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
        pSendMsg->m_msgdata[nIndex++] = pActionSOE->dtTime.day;//日
        pSendMsg->m_msgdata[nIndex++] = pActionSOE->dtTime.month;//月
        pSendMsg->m_msgdata[nIndex++] = pActionSOE->dtTime.year;//年
		pSendMsg->m_msgdata[nIndex++] = 0;//SIN
		pSendMsg->m_msgdata[nIndex++] = 0;//故障类型

		for(i=0; i<pActionSOE->nResultCnt; i++)
		{
/*			pSendMsg->m_msgdata[nIndex++] = pActionSOE->ResultVal[i].measID + 1; //从1开始
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
//保护动作 -104
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
    pSendMsg->m_msgdata[nIndex++] = pActionSOE->nDPI&0x03;//双点信息
	//相对时间Relative time
	pSendMsg->m_msgdata[nIndex++] = LOBYTE(pActionSOE->nPosTime);//低字节
	pSendMsg->m_msgdata[nIndex++] = HIBYTE(pActionSOE->nPosTime);//高字节
    //获取时间
    nMillis = pActionSOE->dtTime.msec + pActionSOE->dtTime.second*1000;
    pSendMsg->m_msgdata[nIndex++] = LOBYTE(nMillis);//毫秒低字节
    pSendMsg->m_msgdata[nIndex++] = HIBYTE(nMillis);//毫秒高字节
    pSendMsg->m_msgdata[nIndex++] = pActionSOE->dtTime.minute;//分
    pSendMsg->m_msgdata[nIndex++] = pActionSOE->dtTime.hour;//时
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

//保护告警 -103
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

    pSendMsg->m_msgdata[nIndex++] = pWarningSOE->nDPI&0x03;//双点信息
    //获取时间
    nMillis = pWarningSOE->dtTime.msec + pWarningSOE->dtTime.second*1000;
    pSendMsg->m_msgdata[nIndex++] = LOBYTE(nMillis);//毫秒低字节
    pSendMsg->m_msgdata[nIndex++] = HIBYTE(nMillis);//毫秒高字节
    pSendMsg->m_msgdata[nIndex++] = pWarningSOE->dtTime.minute;//分
    pSendMsg->m_msgdata[nIndex++] = pWarningSOE->dtTime.hour;//时

    pSendMsg->m_msgdata[nIndex++] = 0;//附加信息SIN,自发上送时
    pSendMsg->m_APCI.m_byteApduLength = APDU_HEAD_LENGTH + nIndex;

	pParams->soeCnt++;
	logMsg("SOE---ASDU1-------,busNo=%d,soeCnt=%d!\n",pParams->m_nLinkSocket,pParams->soeCnt,0,0,0,0);

	//发送报文
	return centralSend(pParams);
}

//保护告警 -104
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
    pSendMsg->m_msgdata[nIndex++] = pWarningSOE->nDPI&0x03;//双点信息
    //获取时间
    nMillis = pWarningSOE->dtTime.msec + pWarningSOE->dtTime.second*1000;
    pSendMsg->m_msgdata[nIndex++] = LOBYTE(nMillis);//毫秒低字节
    pSendMsg->m_msgdata[nIndex++] = HIBYTE(nMillis);//毫秒高字节
    pSendMsg->m_msgdata[nIndex++] = pWarningSOE->dtTime.minute;//分
    pSendMsg->m_msgdata[nIndex++] = pWarningSOE->dtTime.hour;//时
    pSendMsg->m_msgdata[nIndex++] = pWarningSOE->dtTime.day;
    pSendMsg->m_msgdata[nIndex++] = pWarningSOE->dtTime.month;
    pSendMsg->m_msgdata[nIndex++] = pWarningSOE->dtTime.year;

    pSendMsg->m_APCI.m_byteApduLength = APDU_HEAD_LENGTH + nIndex;

	pParams->soeCnt++;
	logMsg("SOE---ASDU31-------,busNo=%d,soeCnt=%d!\n",pParams->m_nLinkSocket,pParams->soeCnt,0,0,0,0);

	//发送报文
	return centralSend(pParams);
}

//开入遥信变位 -103
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

    if(pDievent->n103Inf == 64) //检修压板
    {
        pSendMsg->m_byteType = 0x01;
        pSendMsg->m_byteVSQ = 0x81;
        pSendMsg->m_wordCOT = 1;
        //pSendMsg->m_bytePRM = 0;
        pSendMsg->m_byteCOMADD = MAINTENANCE_COMADDR;
        pSendMsg->m_byteADD = pParams->m_device_address;
        pSendMsg->m_msgdata[FUN_103] = PROTECT_FUN;
        pSendMsg->m_msgdata[INF_103] = pDievent->n103Inf;
        pSendMsg->m_msgdata[nIndex++] = pDievent->nSPI+1;//双点信息

        //获取时间
        nMillis = pDievent->dtTime.msec + pDievent->dtTime.second*1000;
        pSendMsg->m_msgdata[nIndex++] = LOBYTE(nMillis);//毫秒低字节
        pSendMsg->m_msgdata[nIndex++] = HIBYTE(nMillis);//毫秒高字节
        pSendMsg->m_msgdata[nIndex++] = pDievent->dtTime.minute;//分
        pSendMsg->m_msgdata[nIndex++] = pDievent->dtTime.hour;//时

        pSendMsg->m_msgdata[nIndex++] = 0;  //SIN
        pSendMsg->m_APCI.m_byteApduLength = APDU_HEAD_LENGTH + nIndex;
        pParams->soeCnt++;
        logMsg("SOE---ASDU1-------,busNo=%d,soeCnt=%d!\n",pParams->m_nLinkSocket,pParams->soeCnt,0,0,0,0);

        //发送报文
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
        pSendMsg->m_msgdata[nIndex++] = pDievent->nSPI;//单点信息

        //获取时间
        nMillis = pDievent->dtTime.msec + pDievent->dtTime.second*1000;
        pSendMsg->m_msgdata[nIndex++] = LOBYTE(nMillis);//毫秒低字节
        pSendMsg->m_msgdata[nIndex++] = HIBYTE(nMillis);//毫秒高字节
        pSendMsg->m_msgdata[nIndex++] = pDievent->dtTime.minute;//分
        pSendMsg->m_msgdata[nIndex++] = pDievent->dtTime.hour;//时
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
        pSendMsg->m_msgdata[nIndex++] = pDievent->nSPI;//单点信息

        //获取时间
        nMillis = pDievent->dtTime.msec + pDievent->dtTime.second*1000;
        pSendMsg->m_msgdata[nIndex++] = LOBYTE(nMillis);//毫秒低字节
        pSendMsg->m_msgdata[nIndex++] = HIBYTE(nMillis);//毫秒高字节
        pSendMsg->m_msgdata[nIndex++] = pDievent->dtTime.minute;//分
        pSendMsg->m_msgdata[nIndex++] = pDievent->dtTime.hour;//时
    }
	pSendMsg->m_msgdata[nIndex++] = 0;  //SIN
	pSendMsg->m_APCI.m_byteApduLength = APDU_HEAD_LENGTH + nIndex;
	pParams->soeCnt++;
	logMsg("SOE---ASDU41-------,busNo=%d,soeCnt=%d!\n",pParams->m_nLinkSocket,pParams->soeCnt,0,0,0,0);

	//发送报文
	return centralSend(pParams);
}

//开入遥信变位 -104
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

    /*if(pDievent->n103Inf == 64) //检修压板
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
        pSendMsg->m_msgdata[nIndex++] = pDievent->nSPI+1;//双点信息
        //获取时间
        nMillis = pDievent->dtTime.msec + pDievent->dtTime.second*1000;
        pSendMsg->m_msgdata[nIndex++] = LOBYTE(nMillis);//毫秒低字节
        pSendMsg->m_msgdata[nIndex++] = HIBYTE(nMillis);//毫秒高字节
        pSendMsg->m_msgdata[nIndex++] = pDievent->dtTime.minute;//分
        pSendMsg->m_msgdata[nIndex++] = pDievent->dtTime.hour;//时
        pSendMsg->m_msgdata[nIndex++] = pDievent->dtTime.day;
        pSendMsg->m_msgdata[nIndex++] = pDievent->dtTime.month;
        pSendMsg->m_msgdata[nIndex++] = pDievent->dtTime.year;
        pSendMsg->m_APCI.m_byteApduLength = APDU_HEAD_LENGTH + nIndex;

        //发送报文
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
            pSendMsg->m_msgdata[nIndex++] = pDievent->nSPI;//单点信息

            //获取时间
            nMillis = pDievent->dtTime.msec + pDievent->dtTime.second*1000;
            pSendMsg->m_msgdata[nIndex++] = LOBYTE(nMillis);//毫秒低字节
            pSendMsg->m_msgdata[nIndex++] = HIBYTE(nMillis);//毫秒高字节
            pSendMsg->m_msgdata[nIndex++] = pDievent->dtTime.minute;//分
            pSendMsg->m_msgdata[nIndex++] = pDievent->dtTime.hour;//时
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

        //发送报文
        return centralSend(pParams);
    //}
}
//总信号+压板变位+对点+动作返回 -103
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
    if((pGeneralSOE->nASDU == 1)&&(pGeneralSOE->n103Inf == 64))  //检修压板 扇区是0
    {
        pSendMsg->m_byteCOMADD = MAINTENANCE_COMADDR;
    }

	pSendMsg->m_msgdata[INF_103] = pGeneralSOE->n103Inf;

	pSendMsg->m_msgdata[nIndex++] = pGeneralSOE->nDPI&0x03;//双点信息

    if(pGeneralSOE->nASDU == 2)//相对时间 故障序号
    {
        pSendMsg->m_msgdata[nIndex++] = 0;
        pSendMsg->m_msgdata[nIndex++] = 0;
        pSendMsg->m_msgdata[nIndex++] = 0;
        pSendMsg->m_msgdata[nIndex++] = 0;
    }
	//获取时间
	nMillis = pGeneralSOE->dtTime.msec + pGeneralSOE->dtTime.second*1000;
	pSendMsg->m_msgdata[nIndex++] = LOBYTE(nMillis);//毫秒低字节
	pSendMsg->m_msgdata[nIndex++] = HIBYTE(nMillis);//毫秒高字节
	pSendMsg->m_msgdata[nIndex++] = pGeneralSOE->dtTime.minute;//分
	pSendMsg->m_msgdata[nIndex++] = pGeneralSOE->dtTime.hour;//时

	pSendMsg->m_msgdata[nIndex++] = 0;//附加信息SIN,自发上送时
	pSendMsg->m_APCI.m_byteApduLength = APDU_HEAD_LENGTH + nIndex;

	pParams->soeCnt++;
	logMsg("SOE---ASDU1-------,busNo=%d,soeCnt=%d!\n",pParams->m_nLinkSocket,pParams->soeCnt,0,0,0,0);

	//发送报文
	return centralSend(pParams);
}

//总信号+压板变位+对点+动作返回 -104
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
        pSendMsg->m_msgdata[nIndex++] = pGeneralSOE->nDPI&0x01;//单点信息

        //获取时间
        nMillis = pGeneralSOE->dtTime.msec + pGeneralSOE->dtTime.second*1000;
        pSendMsg->m_msgdata[nIndex++] = LOBYTE(nMillis);//毫秒低字节
        pSendMsg->m_msgdata[nIndex++] = HIBYTE(nMillis);//毫秒高字节
        pSendMsg->m_msgdata[nIndex++] = pGeneralSOE->dtTime.minute;//分
        pSendMsg->m_msgdata[nIndex++] = pGeneralSOE->dtTime.hour;//时
        pSendMsg->m_msgdata[nIndex++] = pGeneralSOE->dtTime.day;
        pSendMsg->m_msgdata[nIndex++] = pGeneralSOE->dtTime.month;
        pSendMsg->m_msgdata[nIndex++] = pGeneralSOE->dtTime.year;
    }
    else if(pGeneralSOE->nASDU == 1)
    {
        pSendMsg->m_byteType = IEC104_TI_M_DP_TB_1;
        pSendMsg->m_byteVSQ = 1;
        pSendMsg->m_wordCOT = IEC104_COT_M_spont;
        if(pGeneralSOE->n103Inf == 64)  //检修压板 扇区是0
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
        pSendMsg->m_msgdata[nIndex++] = pGeneralSOE->nDPI&0x03;//双点信息
        //获取时间
        nMillis = pGeneralSOE->dtTime.msec + pGeneralSOE->dtTime.second*1000;
        pSendMsg->m_msgdata[nIndex++] = LOBYTE(nMillis);//毫秒低字节
        pSendMsg->m_msgdata[nIndex++] = HIBYTE(nMillis);//毫秒高字节
        pSendMsg->m_msgdata[nIndex++] = pGeneralSOE->dtTime.minute;//分
        pSendMsg->m_msgdata[nIndex++] = pGeneralSOE->dtTime.hour;//时
        pSendMsg->m_msgdata[nIndex++] = pGeneralSOE->dtTime.day;
        pSendMsg->m_msgdata[nIndex++] = pGeneralSOE->dtTime.month;
        pSendMsg->m_msgdata[nIndex++] = pGeneralSOE->dtTime.year;

    }
    else if(pGeneralSOE->nASDU == 2)//对点 相对时间=0 故障序号=0
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
        pSendMsg->m_msgdata[nIndex++] = pGeneralSOE->nDPI&0x03;//双点信息
        //相对时间Relative time
        pSendMsg->m_msgdata[nIndex++] = 0;//低字节
        pSendMsg->m_msgdata[nIndex++] = 0;//高字节
        //获取时间
        nMillis = pGeneralSOE->dtTime.msec + pGeneralSOE->dtTime.second*1000;
        pSendMsg->m_msgdata[nIndex++] = LOBYTE(nMillis);//毫秒低字节
        pSendMsg->m_msgdata[nIndex++] = HIBYTE(nMillis);//毫秒高字节
        pSendMsg->m_msgdata[nIndex++] = pGeneralSOE->dtTime.minute;//分
        pSendMsg->m_msgdata[nIndex++] = pGeneralSOE->dtTime.hour;//时
        pSendMsg->m_msgdata[nIndex++] = pGeneralSOE->dtTime.day;
        pSendMsg->m_msgdata[nIndex++] = pGeneralSOE->dtTime.month;
        pSendMsg->m_msgdata[nIndex++] = pGeneralSOE->dtTime.year;
    }

	pSendMsg->m_APCI.m_byteApduLength = APDU_HEAD_LENGTH + nIndex;

	//发送报文
	return centralSend(pParams);
}

//录波扰动表
static int32_t ProcessAsdu23(PARAMS *pParams, uint16_t uiCOT)
{
	/*int32_t		nIndex = MSG_103;
	APDU*	pSendMsg = pParams->m_pSendMsg;
	int32_t		nMillis=0;
	int32_t		i;
	TDisturbData* pDisturbData;

    OP_GetpSOEQueue(pParams->m_nLinkSocket, SOE_WAVE,(void**)&pDisturbData);//录波扰动表

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
            pSendMsg->m_byteVSQ++;  //录波的总次数

           //故障序号
            pSendMsg->m_msgdata[nIndex++] = LOBYTE(pDisturbData->nFAN);//低字节
            pSendMsg->m_msgdata[nIndex++] = HIBYTE(pDisturbData->nFAN);//高字节
            pSendMsg->m_msgdata[nIndex++] = (0x01 | ((pParams->waveTransferringFlag==TRUE)? 0x02 : 0)
                                            | ((pDisturbData->nValid &0x01)? 0 : 0x04));//故障状态

            //获取时间
            nMillis = pDisturbData->dtTime.msec + pDisturbData->dtTime.second*1000;
            pSendMsg->m_msgdata[nIndex++] = LOBYTE(nMillis);//毫秒低字节
            pSendMsg->m_msgdata[nIndex++] = HIBYTE(nMillis);//毫秒高字节
            pSendMsg->m_msgdata[nIndex++] = pDisturbData->dtTime.minute;//分
            pSendMsg->m_msgdata[nIndex++] = pDisturbData->dtTime.hour;//时
            pSendMsg->m_msgdata[nIndex++] = pDisturbData->dtTime.day;//日
            pSendMsg->m_msgdata[nIndex++] = pDisturbData->dtTime.month;//月
            pSendMsg->m_msgdata[nIndex++] = pDisturbData->dtTime.year;//年

            if(pSendMsg->m_byteVSQ >= 8)  //录波的总次数
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

    //获取时间
    nMillis = pDisturbData->dtTime.msec + pDisturbData->dtTime.second*1000;
    pSendMsg->m_msgdata[nIndex++] = LOBYTE(nMillis);//毫秒低字节
    pSendMsg->m_msgdata[nIndex++] = HIBYTE(nMillis);//毫秒高字节
    pSendMsg->m_msgdata[nIndex++] = pDisturbData->dtTime.minute;//分
    pSendMsg->m_msgdata[nIndex++] = pDisturbData->dtTime.hour;//时

    pSendMsg->m_APCI.m_byteApduLength = APDU_HEAD_LENGTH + nIndex;

    return centralSend(pParams);
}*///whs 暂时不用      

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

	pSendMsg->m_msgdata[nIndex++] = pParams->waveChannelName;//模拟量名称，最大=CHANNEL_NUMBER

	if(pParams->waveChannelName < 5) //电流
    {
        fRatedValue1 = 5.0*GetARMSysConfigValue(DI_GET_CURRENT_RATIO);
        fRatedValue2 = 5.0;
        fRatioValue  = 229.1;
    }
    else                //电压
    {
        fRatedValue1 = 100.0*GetARMSysConfigValue(DI_GET_VOLTAGE_RATIO);
        fRatedValue2 = 100.0;
        fRatioValue  = 191.5;
    }

	pchar = (uint8_t*)&fRatedValue1;//1次额定值
	pSendMsg->m_msgdata[nIndex++] = *pchar++;
	pSendMsg->m_msgdata[nIndex++] = *pchar++;
	pSendMsg->m_msgdata[nIndex++] = *pchar++;
	pSendMsg->m_msgdata[nIndex++] = *pchar++;

	pchar = (uint8_t*)&fRatedValue2;//2次额定值
	pSendMsg->m_msgdata[nIndex++] = *pchar++;
	pSendMsg->m_msgdata[nIndex++] = *pchar++;
	pSendMsg->m_msgdata[nIndex++] = *pchar++;
	pSendMsg->m_msgdata[nIndex++] = *pchar++;

	pchar = (uint8_t*)&fRatioValue;//换算系数
	pSendMsg->m_msgdata[nIndex++] = *pchar++;
	pSendMsg->m_msgdata[nIndex++] = *pchar++;
	pSendMsg->m_msgdata[nIndex++] = *pchar++;
	pSendMsg->m_msgdata[nIndex++] = *pchar++;

    pSendMsg->m_APCI.m_byteApduLength = APDU_HEAD_LENGTH + nIndex;

    return centralSend(pParams);
}*///whs 暂时不用      

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
}*///whs 暂时不用      

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
}*///whs 暂时不用      

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

    //初始状态报文
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

    //状态变位报文
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
}*///whs 暂时不用      

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
}*///whs 暂时不用      

//录波
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
        case 1:                                     //管理机启动故障录波数据上送
            pParams->waveTransferringFlag = TRUE;
			rt_kprintf("SOE_WAVE-----wave report---,LinkSocket=%d!\n",pParams->m_nLinkSocket);
            nError = ProcessAsdu26(pParams, pDisturbData);   //回答故障录波数据准备好报文
            break;

        case 2:                                     //管理机请求送数据
            nError = ProcessAsdu28(pParams, pDisturbData);   //回答故障录波状态量数据准备好报文
            break;

        case 3:                                     //管理机退出送数据
            nError = ProcessAsdu31(pParams, pDisturbData, 33);    //回答传送结束认可
            pParams->waveTransferringFlag = FALSE;
            break;

        case 8:                                     //管理机请求上送模拟量数据
            pParams->waveChannelName = pParams->m_pRecvMsg->m_msgdata[6];//指定的通道
            nError = ProcessAsdu30(pParams, pDisturbData);    //发送模拟量数据
            break;

        case 9:                                     //管理机退出本路模拟量上送
            if( pParams->m_pRecvMsg->m_msgdata[6] != 0 )	//管理机退出本路模拟量上送
            {
                if ( pParams->m_pRecvMsg->m_msgdata[6] < CHANNEL_NUMBER )//指定的通道
                {
                    pParams->waveChannelName = pParams->m_pRecvMsg->m_msgdata[6];//指定的通道
                    nError = ProcessAsdu27(pParams, pDisturbData);      //回答下一路模拟量
                }
                else
                {
                    nError = ProcessAsdu31(pParams, pDisturbData, 32);      //回答模拟量发送结束
                }
            }
            else		//管理机跳过本路模拟量上送
            {
                if( ProcessAsdu31(pParams, pDisturbData, 36) == HAVE_ERROR )      //回答跳过本路模拟量上送的认可报文
                {
                    return HAVE_ERROR;
                }
                pParams->waveChannelName++;
                nError = ProcessAsdu27(pParams, pDisturbData);      //回答下一路模拟量
            }
            break;

        case 16:                                    //管理机请求发送状态量
            nError = ProcessAsdu29(pParams, pDisturbData);   //准备回答状态量
            break;

        case 17:                                    //管理机终止发送状态量
            nError = ProcessAsdu31(pParams, pDisturbData, 39);  //回答跳过状态量数据上送的认可报文
            pParams->waveChannelName = 1;//从第一通道开始
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
}*///whs 暂时不用      

//回答ASDU25
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
        case 64:			                            //录波发送成功，本次录波发送结束
            pParams->waveTransferringFlag = FALSE;
            break;

        case 65:			                            //录波发送失败，重发被记录的扰动表
            pParams->waveTransferringFlag = FALSE;
            ProcessAsdu23(pParams,31);
            break;

        case 66:			                            //管理机回答本路模拟量发送成功
            pParams->waveChannelName = pParams->m_pRecvMsg->m_msgdata[6];//指定的通道
            if (pParams->waveChannelName < CHANNEL_NUMBER)
            {                                           //准备发下一路模拟量准备好报文
                pParams->waveChannelName++;
                nError = ProcessAsdu27(pParams, pDisturbData);   //发模拟量准备好报文
            }
            else
            {                                           //回答模拟量发送完毕报文
                nError = ProcessAsdu31(pParams, pDisturbData, 32);
            }
            break;

        case 67:			                            //管理机回答本路模拟量发送失败
            pParams->waveChannelName = pParams->m_pRecvMsg->m_msgdata[6];//指定的通道
            if (pParams->waveChannelName <= CHANNEL_NUMBER)
            {
                nError = ProcessAsdu27(pParams, pDisturbData);   //发模拟量准备好报文
            }
            else
            {                                           //回答模拟量发送完毕报文
                nError = ProcessAsdu31(pParams, pDisturbData, 32);
            }
            break;

        case 68:			                            //管理机回答状态量发送成功
            pParams->waveChannelName = 1;
            nError = ProcessAsdu27(pParams, pDisturbData);   //发模拟量准备好报文
            break;

        case 69:			                           //管理机回答状态量发送失败
            nError = ProcessAsdu28(pParams, pDisturbData);   //重发状态量准备好报文
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
}*///whs 暂时不用      

static int32_t	ProcessSOE(PARAMS *pParams)
{
	int32_t		nError = NO_ERROR;
	APDU*	pApdu = (APDU*)pParams->m_pSendMsg;

	//置发送报文头部信息
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

        if(pParams->soeInfo.soeType != SOE_WAVE)//除录波扰动表外，其它SOE需要确认
        {
            pParams->m_nAutoConfirmed = ((nError==NO_ERROR)?pParams->soeInfo.soeType:SOE_CONFIRMED);//如果没发送成功，置确认标志，下次可继续发
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

        pParams->m_nAutoConfirmed = ((nError==NO_ERROR)?pParams->soeInfo.soeType:SOE_CONFIRMED);//如果没发送成功，置确认标志，下次可继续发
        pParams->tick_SOESend = rt_tick_get();
        pParams->m_nSendNum_SOE = pParams->m_nSendNum;
    }

	return nError;
}

//遥控断路器 -103
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
			rt_kprintf("遥控选择错误\n");
		}
	}
	else if((nDCC == 1) || (nDCC == 2)) //execute
	{
		nError = OP_RemoteCtrlExec(pParams->m_pRecvMsg->m_msgdata[FUN_103], pParams->m_pRecvMsg->m_msgdata[INF_103], nDCC, pParams->m_pSaveMsg->m_msgdata[MSG_103],pParams->m_device_address);
		if(!nError)
		{
			rt_kprintf("遥控执行错误\n");
		}
		pParams->m_pSaveMsg->m_msgdata[MSG_103] = 0;
	}
	else //撤销
	{
		pParams->m_pSaveMsg->m_msgdata[MSG_103] = 0;
		nError = 1;
	}

	memcpy(&pSendMsg->m_byteType,&pParams->m_pRecvMsg->m_byteType,ASDU_COPY_LENGTH);
	if(!nError)
	{
		pSendMsg->m_wordCOT = 76;//否定,返校错误
	}
	else
    {
        pSendMsg->m_wordCOT = 12;//肯定
    }
	pSendMsg->m_APCI.m_byteApduLength = APDU_HEAD_LENGTH + 4;
	return centralSend(pParams);
}

//遥控断路器 投退压板 远方复归 -104
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
	
	if(pParams->m_pRecvMsg->m_byteType == 45)   //单点
        nDCS++;

    if(nlINF < YK_DL_END_INDEX) //断路器 每个设备最多 遥控点// 暂时固定,根据实际情况设定
    {
        if(pParams->m_pRecvMsg->m_msgdata[MSG_104] &0x80)   //选择/撤销
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
                rt_kprintf("遥控执行错误\n");
            }
        }
        else
        {
            pSendMsg->m_wordCOT = 71;
        }
    }
    else if((pParams->m_pRecvMsg->m_msgdata[MSG_104] &0x80) == 0)   //复归  压板  定值区切换
    {
        nlINF -= YK_DL_END_INDEX;
        if( nlINF == 0x00 )     //复归
        {
            nError = OP_SignalReset((uint8_t)nlINF, nDCS,pParams->m_device_address);
            if(pParams->m_pRecvMsg->m_byteADD == 0xff)
            {
                return(NO_ERROR);
            }
        }
        else if( nlINF < 100 )//软压板投退
        {
            nError = OP_MdfySoftStrap((uint8_t)nlINF, nDCS,pParams->m_device_address);
        }
        else                    //定值区切换
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

//查询 修改 保护定值 -103
static int32_t ProcessAsdu61(PARAMS *pParams)
{
	uint8_t nINF;
	uint8_t SetZone;
	uint8_t SetCount;
	uint8_t	bSetValue = 0;  //设置定值是否成功
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
            SetZone = GetARMSysConfigValue(ID_GET_CURRENT_ZONE);//当前定值区号
        }

        nINF = pSendMsg->m_msgdata[INF_103];
        if(nINF == 100) //查询
        {
            SetCount = OP_GetSetPointValue(SetZone, &pSendMsg->m_msgdata[4]);
            if( (SetCount == 0)||(SetCount > VALUE_EACH_FRAME) )    //无定值|定值太长
            {
                pSendMsg->m_wordCOT = 21;
            }
            else
            {
                pSendMsg->m_msgdata[MSG_103] = (pSendMsg->m_msgdata[MSG_103]&0xf0) | SetZone;//CPU板地址|定值区号
                pSendMsg->m_msgdata[MSG_103+1] = 0;//定值序号从0开始

                pSendMsg->m_byteVSQ = SetCount+1;//定值个数+1
                pSendMsg->m_APCI.m_byteApduLength = APDU_HEAD_LENGTH + 4 + SetCount*3;

                if(centralSend(pParams)==HAVE_ERROR)
                {
                    return HAVE_ERROR;
                }

                //结束帧
                pSendMsg->m_byteVSQ = 0x81;
                pSendMsg->m_wordCOT = 10;
                pSendMsg->m_APCI.m_byteApduLength = APDU_HEAD_LENGTH + 4;
            }
        }
        else if(nINF == 101) //预发
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
            pSendMsg->m_msgdata[MSG_103] = (pSendMsg->m_msgdata[MSG_103]&0xf0) | SetZone;//CPU板地址|定值区号

            if(centralSend(pParams)==HAVE_ERROR)
            {
                return HAVE_ERROR;
            }

            //结束帧
            pSendMsg->m_byteVSQ = 0x81;
            pSendMsg->m_wordCOT = 10;
            pSendMsg->m_APCI.m_byteApduLength = APDU_HEAD_LENGTH + 4;
        }
        else if((nINF == 103) || (nINF == 105)) //执行/撤销
        {
            if(nINF == 103)//执行
            {
                if(pParams->m_pSaveMsg->m_msgdata[INF_103] == 101)
                {
                    pParams->m_pSaveMsg->m_msgdata[INF_103] = 0;
                    bSetValue = OP_MdfySetPointValue(SetZone, &pParams->m_pSaveMsg->m_msgdata[4],
                                                     pParams->m_pSaveMsg->m_byteVSQ-1, 1);
                }
                pSendMsg->m_wordCOT = bSetValue? 20:21;
                pSendMsg->m_msgdata[INF_103] = 104;
                pSendMsg->m_msgdata[MSG_103] = (pSendMsg->m_msgdata[MSG_103]&0xf0) | SetZone;//CPU板地址|定值区号
            }
            else
            {
                pParams->m_pSaveMsg->m_msgdata[INF_103] = 0;
                pSendMsg->m_wordCOT = 20;
                pSendMsg->m_msgdata[INF_103] = 106;
                pSendMsg->m_msgdata[MSG_103] = (pSendMsg->m_msgdata[MSG_103]&0xf0) | SetZone;//CPU板地址|定值区号
            }
        }
        else
        {
            pSendMsg->m_wordCOT = 21;
        }
    }

	return centralSend(pParams);
}

//查询 保护定值 -104
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
            SetZone = GetARMSysConfigValue(ID_GET_CURRENT_ZONE);//当前定值区号
        }
        //查询
        SetCount = OP_GetSetPointValue(SetZone, &pSendMsg->m_msgdata[MSG_104]);
        if( (SetCount == 0)||(SetCount > VALUE_EACH_FRAME) )    //无定值|定值太长
        {
            pSendMsg->m_byteType = IEC104_TI_M_ME_NB_1;
            pSendMsg->m_byteVSQ = 1;
            pSendMsg->m_wordCOT = IEC104_COT_M_deactcon;
        }
        else
        {
            pSendMsg->m_byteType = IEC104_TI_M_ME_NB_1;
            pSendMsg->m_byteVSQ = SetCount|0x80;//定值个数
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

            //结束帧
            pSendMsg->m_byteType = IEC104_TI_C_RD_NA_1;
            pSendMsg->m_byteVSQ = 1;
            pSendMsg->m_wordCOT = IEC103_COT_M_queryEND;
            pSendMsg->m_APCI.m_byteApduLength = APDU_HEAD_LENGTH + 3;
        }
    }
    return centralSend(pParams);
}

//修改 单个保护定值 -104
static int32_t ProcessAsdu49(PARAMS *pParams)
{
	uint32_t nlINF;
	uint8_t SetZone;
	uint8_t	bSetValue = 0;  //设置定值是否成功
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
        if((pParams->m_pRecvMsg->m_wordCOT == 6)&&(pParams->m_pRecvMsg->m_msgdata[6]==0x80)) //预发
        {
            memcpy(pParams->m_pSaveMsg, pParams->m_pRecvMsg, pParams->m_pRecvMsg->m_APCI.m_byteApduLength+2);

            pSendMsg->m_wordCOT = 7;
        }
        else if(pParams->m_pRecvMsg->m_msgdata[6]==0x00) //执行/撤销
        {
            if(pParams->m_pRecvMsg->m_wordCOT == 6)//执行
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

//查询SOE
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

	pSendMsg->m_msgdata[nIndex++] = pAppointedSOE->nDPI&0x03;//双点信息

	//相对时间
	pSendMsg->m_msgdata[nIndex++] = LOBYTE(pAppointedSOE->nPosTime);//低字节
	pSendMsg->m_msgdata[nIndex++] = HIBYTE(pAppointedSOE->nPosTime);//高字节

	//故障序号
	pSendMsg->m_msgdata[nIndex++] = LOBYTE(pAppointedSOE->nFAN);//低字节
	pSendMsg->m_msgdata[nIndex++] = HIBYTE(pAppointedSOE->nFAN);//高字节

	//获取时间
	nMillis = pAppointedSOE->dtTime.msec + pAppointedSOE->dtTime.second*1000;
	pSendMsg->m_msgdata[nIndex++] = LOBYTE(nMillis);//毫秒低字节
	pSendMsg->m_msgdata[nIndex++] = HIBYTE(nMillis);//毫秒高字节
	pSendMsg->m_msgdata[nIndex++] = pAppointedSOE->dtTime.minute;//分
	pSendMsg->m_msgdata[nIndex++] = pAppointedSOE->dtTime.hour;//时
    pSendMsg->m_msgdata[nIndex++] = pAppointedSOE->dtTime.day;//日
    pSendMsg->m_msgdata[nIndex++] = pAppointedSOE->dtTime.month;//月
    pSendMsg->m_msgdata[nIndex++] = pAppointedSOE->dtTime.year;//年
    pSendMsg->m_msgdata[nIndex++] = FaultRptNumber;//SIN
    pSendMsg->m_msgdata[nIndex++] = 0;//故障类型

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
	pSendMsg->m_wordCOT = 10;*///whs 暂时不用 
	return centralSend(pParams);     
}

//时钟同步-103
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
		rt_kprintf("校时失败\n");
	}

	memcpy(pParams->m_pSendMsg,pParams->m_pRecvMsg,pParams->m_pRecvMsg->m_APCI.m_byteApduLength+2);

	return centralSend(pParams);
}

//时钟同步-104
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
		rt_kprintf("校时失败-IEC104\n");
	}

	memcpy(pParams->m_pSendMsg,pParams->m_pRecvMsg,pParams->m_pRecvMsg->m_APCI.m_byteApduLength+2);
    pParams->m_pSendMsg->m_wordCOT = 7;

	return centralSend(pParams);
}

//响应总召唤-103
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
        //装置通讯状态, 检修压板状态
        if(ProcessDeviceState(pParams,IEC103_TI_M_TM_TA_3,IEC103_COT_M_totalQUERY)==HAVE_ERROR)
        {
            return HAVE_ERROR;
        }
        nIndex = MSG_103;
        //检修压板
        pSendMsg->m_byteType = 1;
        pSendMsg->m_byteVSQ = 0x81;
        pSendMsg->m_wordCOT = 9;
        //pSendMsg->m_bytePRM = 0;
        pSendMsg->m_byteCOMADD = 0;
        pSendMsg->m_byteADD = pParams->m_device_address;
        pSendMsg->m_msgdata[FUN_103] = PROTECT_FUN;

        pSendMsg->m_msgdata[INF_103] = 64;
        pSendMsg->m_msgdata[nIndex++] = GetARMSysConfigValue(ID_GET_JXSTRAP_STATE)+1;//检修压板状态

        dtime = Now(); //获取当前时间
        nMillis = dtime.msec + dtime.second*1000;
        pSendMsg->m_msgdata[nIndex++] = LOBYTE(nMillis);//毫秒低字节
        pSendMsg->m_msgdata[nIndex++] = HIBYTE(nMillis);//毫秒高字节
        pSendMsg->m_msgdata[nIndex++] = dtime.minute;//分
        pSendMsg->m_msgdata[nIndex++] = dtime.hour;//时
        pSendMsg->m_msgdata[nIndex++] = pParams->m_pRecvMsg->m_msgdata[MSG_103];//总召唤序号

        pSendMsg->m_APCI.m_byteApduLength = APDU_HEAD_LENGTH + nIndex;

        if(centralSend(pParams)==HAVE_ERROR)
        {
            return HAVE_ERROR;
        }
        break;

    case 1:     //ASDU_1的信息 //告警 软压板 录波
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
                dtime = Now(); //获取当前时间
                nMillis = dtime.msec + dtime.second*1000;
                pSendMsg->m_msgdata[nIndex++] = LOBYTE(nMillis);//毫秒低字节
                pSendMsg->m_msgdata[nIndex++] = HIBYTE(nMillis);//毫秒高字节
                pSendMsg->m_msgdata[nIndex++] = dtime.minute;//分
                pSendMsg->m_msgdata[nIndex++] = dtime.hour;//时
                pSendMsg->m_msgdata[nIndex++] = pParams->m_pRecvMsg->m_msgdata[MSG_103];//总召唤序号

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
                dtime = Now(); //获取当前时间
                nMillis = dtime.msec + dtime.second*1000;
                pSendMsg->m_msgdata[nIndex++] = LOBYTE(nMillis);//毫秒低字节
                pSendMsg->m_msgdata[nIndex++] = HIBYTE(nMillis);//毫秒高字节
                pSendMsg->m_msgdata[nIndex++] = dtime.minute;//分
                pSendMsg->m_msgdata[nIndex++] = dtime.hour;//时
                pSendMsg->m_msgdata[nIndex++] = pParams->m_pRecvMsg->m_msgdata[MSG_103];//总召唤序号

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

    case 2:     //ASDU_41的信息，用ASDU_40上送 开入 档位总召用ASDU_38上送
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
	//ASDU8 总召唤结束
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

//响应总召唤-104
static int32_t ProcessAsdu100(PARAMS *pParams)
{
	uint8_t	nNum;
	int32_t		nIndex;
	APDU*	pSendMsg = pParams->m_pSendMsg;
	uint8_t   RecvCOMADD = pParams->m_pRecvMsg->m_byteCOMADD;

	//ASDU100 总召唤确认
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
        //装置通讯状态, 检修压板状态
        /*if(ProcessDeviceState(pParams,IEC104_TI_M_SP_NA_1,IEC104_COT_M_introgen)==HAVE_ERROR)
        {
            return HAVE_ERROR;
        }
        nIndex = MSG_104;
        //检修压板
        pSendMsg->m_byteType = IEC104_TI_M_DP_NA_1;
        pSendMsg->m_byteVSQ = 1;
        pSendMsg->m_wordCOT = IEC104_COT_M_introgen;
        pSendMsg->m_byteCOMADD = RecvCOMADD;
        pSendMsg->m_byteADD = pParams->m_device_address;

        pSendMsg->m_msgdata[INF_104L] = 65;
        pSendMsg->m_msgdata[INF_104M] = 0;
        pSendMsg->m_msgdata[INF_104H] = 0;
        pSendMsg->m_msgdata[nIndex++] = GetARMSysConfigValue(ID_GET_JXSTRAP_STATE)+1;//检修压板状态

        pSendMsg->m_APCI.m_byteApduLength = APDU_HEAD_LENGTH + nIndex;

        if(centralSend(pParams)==HAVE_ERROR)
        {
            return HAVE_ERROR;
        }*/
				return HAVE_ERROR;
        break;

    case 1:     //告警 软压板
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

    case 2:     //开入
        //if(GetARMSysConfigValue(ID_GET_JXSTRAP_STATE) == 0)   //maintenance strap off
        //{
            nNum = 0;
            if(OP_GetDIState(&(pSendMsg->m_msgdata[3]), &nNum, pParams->m_device_address) == HAVE_ERROR)
					  {
							  return HAVE_ERROR;
						}    

            pSendMsg->m_byteType = IEC104_TI_M_SP_NA_1;
            pSendMsg->m_byteVSQ = (nNum+2)|0x80;//连续上送
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
	//ASDU100 总召唤结束
	memcpy(pParams->m_pSendMsg,pParams->m_pRecvMsg,pParams->m_pRecvMsg->m_APCI.m_byteApduLength+2);
    pParams->m_pSendMsg->m_wordCOT = 10;
	return centralSend(pParams);
	rt_kprintf("pSendMsg:  %x.%x.%x.%x.%x\n",pSendMsg->m_msgdata[0],pSendMsg->m_msgdata[1],pSendMsg->m_msgdata[2],pSendMsg->m_msgdata[3],pSendMsg->m_msgdata[4]);

}

void save_general_report(uint8_t nASDU,uint8_t nDispID, uint8_t nDPI, uint8_t nCOT);
//信号复归 定值区切换(许继的定值区切换确认INF=224) 软压板投退
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
	if( RecvINF == 19 )     //复归
	{
		nYesOrNo = OP_SignalReset(RecvINF, RecvDCO,pParams->m_device_address);
		if(pParams->m_pRecvMsg->m_byteADD == 0xff)
        {
            return(NO_ERROR);
        }
	}
	else if( RecvINF < 100 )//软压板投退
	{
	    nYesOrNo = OP_MdfySoftStrap(RecvINF, RecvDCO,pParams->m_device_address);
	}
	else                    //定值区切换
    {
        nYesOrNo = OP_MdfySetPointGroup(RecvINF, RecvDCO,pParams->m_device_address);
        RecvINF = 224;   //定值区变化的信息序号是224
	}

/*	switch( RecvINF )
	{
		case 19:        //复归
			nYesOrNo = OP_SignalReset(RecvINF, RecvDCO,pParams->m_device_address);
		break;

		case 100:		// zone 0 定值区切换
		case 101: 		// zone 1
		case 102:		// zone 2
		case 103:		// zone 3
		case 104:		// zone 4
		case 105:		// zone 5
		case 106:		// zone 6
		case 107:		// zone 7
			nYesOrNo = OP_MdfySetPointGroup(RecvINF, RecvDCO,pParams->m_device_address);
			//pParams->m_pRecvMsg->m_msgdata[INF_103] = 224;   //8000系统是这么要求的
            save_general_report(ASDU_1,2,RecvDCO,(nYesOrNo==1)?20:21);//第3个信号是定值区变化,遥控修改定值区返回状态的传输原因是20
			return(NO_ERROR);
//        break;

		default:		//软压板投退
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

	dtime = Now(); //获取当前时间
	nMillis = dtime.msec + dtime.second*1000;
	pSendMsg->m_msgdata[nIndex++] = LOBYTE(nMillis);//毫秒低字节
	pSendMsg->m_msgdata[nIndex++] = HIBYTE(nMillis);//毫秒高字节
	pSendMsg->m_msgdata[nIndex++] = dtime.minute;//分
	pSendMsg->m_msgdata[nIndex++] = dtime.hour;//时

	pSendMsg->m_msgdata[nIndex++] = pParams->m_pRecvMsg->m_msgdata[3];

	pSendMsg->m_APCI.m_byteApduLength = APDU_HEAD_LENGTH + nIndex;
	return centralSend(pParams);
}

//电度量 冻结->冻结返回->返回电度量 -103
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
	nNum = OP_GetPowerEnergy103(pParams->m_pRecvMsg->m_msgdata[2], &nInf, &pMsg, ymdevaddr);//获取电度量
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
	pSendMsg->m_msgdata[INF_103] = nInf;    //第一个脉冲的信息序号=6

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

//电度量 冻结->冻结返回->返回电度量 -104
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
	nNum = OP_GetPowerEnergy103(pParams->m_pRecvMsg->m_msgdata[MSG_104], &nInf, &pMsg, ymdevaddr);//获取电度量
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
	pSendMsg->m_msgdata[INF_104L] = 0x01;    //第一个脉冲的信息序号=0x6401
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


//重置主任务中的指针数组，对子任务的标记
static void resetTask(PARAMS** ppParams,PARAMS* pParams)
{
	int32_t i=0;
	if(!pParams)//指针为空，直接返回
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
//任务退出时(exitTask,1)
static void clearTask(PARAMS* pParams)
{
	if(!pParams)//指针为空，直接返回
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
//异常退出时的清理
static void exitTask(PARAMS** ppParams,PARAMS* pParams,const char* error)
{
	rt_kprintf(error);

	resetTask(ppParams,pParams);

	clearTask(pParams); //释放申请的内存。

	//rt-thread 线程自己删除自己后要做一次线程调度，在删除自己前一定要释放申请的内存。
	rt_thread_delete(rt_thread_self());
    rt_schedule();
}

//判断当前IP地址新来的连接是否是已经存在一个前连接
static int32_t getConnState(PARAMS** ppParams,uint32_t clientIP,uint32_t serverPort)
{
	//返回值:1表示新增连接，0表示删除旧连接，建立新连接。
	int32_t i=0;
	int32_t nRetVal = 0;

	for(i=0;i<MAX_CLIENT_NUM;i++)
	{
		PARAMS* pParams =ppParams[i];


		if( pParams == 0 )//表示还有空位，可以新建连接
		{
			nRetVal = 1;//找到就置位，不可马上返回，需等遍历结束
		}
		else if(( pParams->m_clientIP == clientIP )//如果找到重复客户端IP和服务器端口号则删除任务,而且该位置可以重用,可以立即返回
            &&(pParams->m_severPort == serverPort))
		{
			//deleteTask(ppParams,pParams);
			ppParams[i]->m_device_address = -1;//0;//设为0，将还原在vxSvrMain中指针数组中的标记位(等同于resetTask)
			rt_thread_delay(100);   //让出CPU，让重复IP的线程自己删除自己后再重新创建一个新的。
			return 1;
		}
	}
	return nRetVal;
}
//保存当前新建的连接的状态
static int32_t setConnState(PARAMS** ppParams,PARAMS* pParams)
{
	//在主任务的子任务管理数组中，找到空位，保存该任务的参数结构
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

//发送STARTDT_CONFIRM
static int32_t sendStartDTConfirm(PARAMS* pParams)
{
	pParams->m_pCtrlMsg->m_byteStartChar=0x68;
	pParams->m_pCtrlMsg->m_byteApduLength=4;
	pParams->m_pCtrlMsg->m_wordSendNum=0x0B;
	pParams->m_pCtrlMsg->m_wordRecvNum=0;

	return(sendCtrlMsg(pParams));
}
//发送STOPDT_CONFIRM
static int32_t sendStopDTConfirm(PARAMS* pParams)
{
	pParams->m_pCtrlMsg->m_byteStartChar=0x68;
	pParams->m_pCtrlMsg->m_byteApduLength=4;
	pParams->m_pCtrlMsg->m_wordSendNum=0x23;
	pParams->m_pCtrlMsg->m_wordRecvNum=0;

	return(sendCtrlMsg(pParams));
}
//发送TESTFR_CONFIRM
static int32_t sendTESTFRConfirm(PARAMS* pParams)
{
	pParams->m_pCtrlMsg->m_byteStartChar=0x68;
	pParams->m_pCtrlMsg->m_byteApduLength=4;
	pParams->m_pCtrlMsg->m_wordSendNum=0x83;
	pParams->m_pCtrlMsg->m_wordRecvNum=0;

	return(sendCtrlMsg(pParams));
}
//发送TESTFR
static int32_t sendTESTFRCmd(PARAMS* pParams)
{
	pParams->m_pCtrlMsg->m_byteStartChar=0x68;
	pParams->m_pCtrlMsg->m_byteApduLength=4;
	pParams->m_pCtrlMsg->m_wordSendNum=0x43;
	pParams->m_pCtrlMsg->m_wordRecvNum=0;

	return(sendCtrlMsg(pParams));
}
//解析报文
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

	if(pParams->m_pRecvMsg->m_APCI.m_byteApduLength == 4)//U、S
	{
		memcpy(pParams->m_pCtrlMsg,pParams->m_pRecvMsg,6);
		pApci = (APCI*)(pParams->m_pCtrlMsg);


   
		
		if(pApci->m_wordSendNum == 0x07)
		{
			rt_kprintf("接收到STARTDT报文！\n");
			pParams->m_bRecvStartDT = TRUE;
			//pParams->tick_SOESend = rt_tick_get();
			if(sendStartDTConfirm(pParams) == HAVE_ERROR)
			{
				return HAVE_ERROR;
			}

			//SOE置位
            if(!OP_EstablishLink(pParams->m_serverFd, pParams->m_clientIP))	//处理SOE信息相关指针
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
                if(ProcessDeviceState(pParams,IEC103_TI_M_TM_TA_3,IEC103_COT_M_per)==HAVE_ERROR)//装置是否需要一个单元地址，用于对上链接
                {
                    return HAVE_ERROR;
                }
                return(ProcessAsdu05(pParams));
            }
		}
		else if((pParams->m_bRecvStartDT) && (pApci->m_wordSendNum == 0x13))
		{
//			rt_kprintf("接收到STOPDT报文！\n");
			if(pParams->m_bWorkingPort == TRUE)
			{
				//logMsg("WorkingPort is FALSE:   busNo=%d, LinkSocket=%d!\n",pParams->m_nLinkSocket,pParams->m_nLinkSocket,0,0,0);
				pParams->m_bWorkingPort = FALSE;
			}

			return(sendStopDTConfirm(pParams));
		}
        else if((pApci->m_wordSendNum == 0x43))
		{
//			rt_kprintf("接收到TESTFR报文！\n");

			return(sendTESTFRConfirm(pParams));
		}
        else if((pApci->m_wordSendNum == 0x83))
        {
            return NO_ERROR;
        }
		else if((pParams->m_bRecvStartDT) && (pApci->m_wordSendNum == 0x01))
		{
//			rt_kprintf("接收到S_CONFIRM报文！\n");
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
    else if(pParams->m_bRecvStartDT)//&&(pParams->m_pRecvMsg->m_byteADD==pParams->m_device_address)) //I whs不固定地址，响应总召地址
	{
		pParams->m_device_address = pParams->m_pRecvMsg->m_byteADD ; //whs 跟随总召地址
		//rt_kprintf("****接收到I报文 pBuffer[1]=%d  地址  %d;  类型  %d\n",pBuffer[1],pParams->m_pRecvMsg->m_byteADD,pParams->m_pRecvMsg->m_byteType);

		if(pParams->m_bWorkingPort == FALSE)
		{
			//logMsg("WorkingPort is TRUE:   busNo=%d, LinkSocket=%d!\n",pParams->m_nLinkSocket,pParams->m_nLinkSocket,0,0,0);
			pParams->m_bWorkingPort = TRUE;
		}
		//*/

		if(pParams->m_nAutoConfirmed != SOE_CONFIRMED)  //soe确认收到，指针移位
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

		//置发送报文头部信息
		pApdu->m_APCI.m_byteStartChar = 0x68;
		pApdu->m_APCI.m_byteApduLength = 0;

        if(pParams->m_severPort==ENGINEER_PORT)
        {
            if(GetARMSysConfigValue(ID_GET_JXSTRAP_STATE) == 0)   //maintenance strap off
            {
                switch(pParams->m_pRecvMsg->m_byteType)
                {
                case 6:         //校时
                    nError = ProcessAsdu06(pParams);
                    break;

                case 7:         //总召
                    nError = ProcessAsdu07(pParams);
                    break;

                case 20:        //复归 压板 定值区
                    nError = ProcessAsdu20(pParams);
                    break;

                case 64:        //断路器
                    nError = ProcessAsdu64(pParams);
                    break;

                case 61:        //保护定值
                    nError = ProcessAsdu61(pParams);
                    break;

                case 62:		//get SOE
                    nError = ProcessAsdu62(pParams);
                    break;

                case 88:        //电度量
                    nError = ProcessAsdu88(pParams);
                    break;

                //录波
               /* case 24:        //录波上送
                    pParams->tick_WaveStart = rt_tick_get();
                    nError=ProcessAsdu24(pParams);
                    break;

                case 25:		//接收认可处理
                    pParams->tick_WaveStart = rt_tick_get();
                    nError = ProcessAsdu25(pParams);
                    break;*///whs 暂时不用      

                case 60:        //查版本号
                    nError = ProcessAsdu05(pParams);
                    break;

                default:        //否定应答
                    nError = ProcessAsduNoACK(pParams);
                    break;
                }
            }
            else if(pParams->m_pRecvMsg->m_byteType == 7)         //总召
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
                case 103:         //校时
                    nError = ProcessAsdu103(pParams);
                    break;

                case 100:         //总召
                    nError = ProcessAsdu100(pParams);
                    break;

                case 45:
                case 46:        //断路器 复归 压板 定值区
                    nError = ProcessAsdu46(pParams);
                    break;

                case 102:        //查询保护定值
                    nError = ProcessAsdu102(pParams);
                    break;

                case 49:        //修改单个保护定值
                    nError = ProcessAsdu49(pParams);
                    break;

                case 101:        //电度量
                    nError = ProcessAsdu101(pParams);
                    break;

                default:        //否定应答
                    memcpy(pParams->m_pSendMsg,pParams->m_pRecvMsg,pParams->m_pRecvMsg->m_APCI.m_byteApduLength+2);
                    pParams->m_pSendMsg->m_wordCOT = 71;
                    nError = centralSend(pParams);
                    break;
                }
            }
            else if(pParams->m_pRecvMsg->m_byteType == 7)         //总召
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

	APDU	*pRecvMsg;	 //当前链路的接收缓冲区
	APDU	*pSendMsg;	 //当前链路的发送缓冲区
	APDU	*pSaveMsg;	 //当前链路的保存缓冲区
	APCI	*pCtrlMsg;	 //当前链路的控制信息缓冲区
	PARAMS	*pParams;	 //参数传递结构体
	uint32_t tickRecvOld; //接收计时器前值
	uint32_t tickRecvNew; //接收计时器当前值

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
	pParams->m_clientIP = clientAddr; //保存当前的任务对应的客户端IP地址
	pParams->m_severPort = severPort; //保存当前的任务对应的服务器端口号
	pParams->m_taskID   = rt_thread_self();//当前任务的ID号，用于删除任务。

//    pParams->m_thePortConfig = thePortConfig;//

	if(!OP_EstablishLink(serverFd, clientAddr))	//处理SOE信息相关指针
	{
	    pParams->m_device_address = -1;
	}
	pParams->m_nLinkSocket = serverFd;
	pParams->m_nSendPacket = 0;
	pParams->m_nRecvPacket = 0;		//正确接收的总包数
	pParams->m_nErrPackets = 0;	//接收错误的包数

	pParams->m_bRecvStartDT = FALSE;
	pParams->m_bWorkingPort = FALSE;
	//pParams->m_bSOESend = FALSE;
	pParams->m_nAutoConfirmed = SOE_CONFIRMED;
	pParams->soeCnt = 0;

	if(!setConnState(ppParams,pParams))
	{
		exitTask(ppParams,pParams,"设置连接状态异常，任务退出\n");
	}

	while(1)
    {
		// 有相同IP地址连接服务器同一端口，删除本线程
		if(pParams->m_device_address == -1)
		{
			rt_kprintf("Link Error serverFd = %d",serverFd);
			exitTask(ppParams,pParams,"\n same port linked,exit this task\n");
			break;
		}

        //装置地址修改，删除本线程
        if(pParams->m_device_address != GetARMSysConfigValue(DI_GET_DEV_ADDR))
		{
			exitTask(ppParams,pParams,"\n device address changed,exit this task 103\n");
			break;
		}


		//处理接收报文
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
				//解析并发送报文
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
				rt_kprintf("103超过60秒链路无数据接收，任务退出\n");
				exitTask(ppParams,pParams,"Recv Timer Error\n");
			}
		}
		//处理自发上送信息
        //StartDT后开始主动发送，出现12个未应答I报文后停止发送
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

				//if(tick_Interval >= RT_TICK_PER_SECOND*20)//20s之后无确认清标记
				{
					OP_SendSomeSOE(pParams->m_nLinkSocket,pParams->m_nAutoConfirmed,pParams->soeInfo.soeCount);
					pParams->m_nAutoConfirmed = SOE_CONFIRMED;
				}
			}

			//SOE
			//if((pParams->m_bWorkingPort == TRUE) && (pParams->m_bSOESend == TRUE) && (pParams->m_nAutoConfirmed == SOE_CONFIRMED) && (JudgeNewSOE(pParams) == OK))
			if((pParams->m_bWorkingPort == TRUE) && (pParams->m_nAutoConfirmed == SOE_CONFIRMED) && (JudgeNewSOE(pParams) == OK))
			{
				rt_kprintf("new SOE ,m_nLinkSocket=%d, soeType=%d,！\n",pParams->m_nLinkSocket,pParams->soeInfo.soeType);
				if(ProcessSOE(pParams) == HAVE_ERROR)
				{
					exitTask(ppParams,pParams,"Auto ProcessSOE error\n");
				}
			}

			//变化YC:越死区+定时
            tickYCNew = rt_tick_get();
            tick_Interval = tickYCNew - tickYCOld;
			if((tick_Interval >= RT_TICK_PER_SECOND)&&(GetARMSysConfigValue(ID_GET_JXSTRAP_STATE)==0))    //1s  maintenance strap off
            {
                tickYCOld = tickYCNew;
                tick_Interval = tickYCNew - tickYCOld1;
                if(tick_Interval >= RT_TICK_PER_SECOND*GetARMSysConfigValue(ID_GET_CYCLE_TIMES))//循环上送时间
                {
                    tickYCOld1 = tickYCNew;
                    rt_kprintf("time out send YC----- LinkSocket=%d!\n",pParams->m_nLinkSocket);
                    if(ProcessSendYC(pParams, 50, 2) == HAVE_ERROR)
                    {
                        exitTask(ppParams,pParams,"time out ProcessYC error\n");
                    }
                }
                else if(OP_GetThresholdSendMeas(pParams->m_nLinkSocket,50))//越死区判断10%
                {
                    rt_kprintf("over threshold send YC-----LinkSocket=%d!\n",pParams->m_nLinkSocket);
                    if(ProcessSendYC(pParams, 50, 1) == HAVE_ERROR)
                    {
                        exitTask(ppParams,pParams,"over threshold ProcessYC error\n");

                    }
                }
            }

		}

		//wave timeout处理
		if(pParams->waveTransferringFlag == TRUE)
		{
			tick_Interval = rt_tick_get() - pParams->tick_WaveStart;

			if(tick_Interval >= RT_TICK_PER_SECOND*10)//10s
			{
				rt_kprintf("召唤录波数据超时,LinkSocket=%d!\n",pParams->m_nLinkSocket);
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

	APDU	*pRecvMsg;	 //当前链路的接收缓冲区
	APDU	*pSendMsg;	 //当前链路的发送缓冲区
	APDU	*pSaveMsg;	 //当前链路的保存缓冲区
	APCI	*pCtrlMsg;	 //当前链路的控制信息缓冲区
	PARAMS	*pParams;	 //参数传递结构体
	uint32_t tickRecvOld; //接收计时器前值
	uint32_t tickRecvNew; //接收计时器当前值

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
	pParams->m_clientIP = clientAddr; //保存当前的任务对应的客户端IP地址
	pParams->m_severPort = severPort; //保存当前的任务对应的服务器端口号
	pParams->m_taskID   = rt_thread_self();//当前任务的ID号，用于删除任务。

//    pParams->m_thePortConfig = thePortConfig;//

	if(!OP_EstablishLink(serverFd, clientAddr))	//处理SOE信息相关指针
	{
	    pParams->m_device_address = -1;
	}
	pParams->m_nLinkSocket = serverFd;
	pParams->m_nSendPacket = 0;
	pParams->m_nRecvPacket = 0;		//正确接收的总包数
	pParams->m_nErrPackets = 0;	//接收错误的包数

	pParams->m_bRecvStartDT = FALSE;
	pParams->m_bWorkingPort = FALSE;
	//pParams->m_bSOESend = FALSE;
	pParams->m_nAutoConfirmed = SOE_CONFIRMED;
	pParams->soeCnt = 0;

	if(!setConnState(ppParams,pParams))
	{
		exitTask(ppParams,pParams,"设置连接状态异常，任务退出\n");
	}

	while(1)
    {
		// 有相同IP地址连接服务器同一端口，删除本线程
		if(pParams->m_device_address == -1)
		{
			rt_kprintf("Link Error serverFd = %d IEC104",serverFd);
			exitTask(ppParams,pParams,"\n same port linked,exit this task IEC104\n");
			break;
		}

        //装置地址修改，删除本线程
   /* if(pParams->m_device_address != GetARMSysConfigValue(DI_GET_DEV_ADDR))
		{
			exitTask(ppParams,pParams,"\n device address changed,exit this task IEC104\n");
			break;
		}*///装置作为数据中转，只有IP 没有地址

    
		//处理接收报文
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
				//解析并发送报文
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
				rt_kprintf("104超过60秒链路无数据接收，任务退出\n");
				exitTask(ppParams,pParams,"Recv Timer Error\n");
			}
		}
		//处理自发上送信息
        //StartDT后开始主动发送，出现12个未应答I报文后停止发送
		if((pParams->m_bRecvStartDT == TRUE)&&(pParams->m_nSendNum - pParams->m_nConfirmedNum < 24))    //12*2
		{


			if(pParams->m_nAutoConfirmed != SOE_CONFIRMED)//
			{
				tick_SOESendNew = rt_tick_get();
				tick_Interval = tick_SOESendNew - pParams->tick_SOESend;

				//if(tick_Interval >= RT_TICK_PER_SECOND*20)//20s之后无确认清标记
				{
					OP_SendSomeSOE(pParams->m_nLinkSocket,pParams->m_nAutoConfirmed,pParams->soeInfo.soeCount);
					pParams->m_nAutoConfirmed = SOE_CONFIRMED;
				}
			}
			//SOE
			//if((pParams->m_bWorkingPort == TRUE) && (pParams->m_bSOESend == TRUE) && (pParams->m_nAutoConfirmed == SOE_CONFIRMED) && (JudgeNewSOE(pParams) == OK))
			if((pParams->m_bWorkingPort == TRUE) && (pParams->m_nAutoConfirmed == SOE_CONFIRMED) && (JudgeNewSOE(pParams) == OK))
			{
				rt_kprintf("new SOE ,m_nLinkSocket=%d, soeType=%d,！\n",pParams->m_nLinkSocket,pParams->soeInfo.soeType);
				if(ProcessSOE(pParams) == HAVE_ERROR)
				{
					exitTask(ppParams,pParams,"Auto ProcessSOE error\n");
				}
			}

			//变化YC:越死区+定时
            tickYCNew = rt_tick_get();
            tick_Interval = tickYCNew - tickYCOld;
			if((tick_Interval >= RT_TICK_PER_SECOND)&&(GetARMSysConfigValue(ID_GET_JXSTRAP_STATE)==0))    //1s  maintenance strap off
            {
                tickYCOld = tickYCNew;
                tick_Interval = tickYCNew - tickYCOld1;
                if(tick_Interval >= RT_TICK_PER_SECOND/5)//*GetARMSysConfigValue(ID_GET_CYCLE_TIMES))//循环上送时间//10s
                {
                    tickYCOld1 = tickYCNew;
                    //rt_kprintf("time out send YC----- LinkSocket=%d!\n",pParams->m_nLinkSocket);
                    if(ProcessSendYC(pParams, IEC104_TI_M_ME_NA_1, IEC104_COT_M_cyc) == HAVE_ERROR)
                    {
                        exitTask(ppParams,pParams,"time out ProcessYC error\n");
                    }
                }
								
                /*else if(OP_GetThresholdSendMeas(pParams->m_nLinkSocket,9))//越死区判断10%
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


PARAMS* ppParams[MAX_CLIENT_NUM];//保存每个任务的PARAMS参数结构的指针数组
/*主任务实现*/
void TCP103CommuTsk(void *parameter)
{

	struct ip_addr ip_addr;
	char* netif_name =  (char*)parameter;

	//PARAMS* ppParams[MAX_CLIENT_NUM];//保存每个任务的PARAMS参数结构的指针数组

	struct sockaddr_in  serverAddr; /*Server地址*/
	uint32_t       serverPort=ENGINEER_PORT;/*Server端口号*/

	struct sockaddr_in  clientAddr; /*连接的客户端的IP地址*/

	char	taskName[20];
	rt_thread_t	nTaskID;
	CHILDTASKPAR childParam;

	int32_t     listenFd;/* listen socket file descriptor */
	int32_t     serverFd;/* server socket file descriptor */

	u32_t		addrsize;//地址结构体的大小
	int32_t     blockflag = 1;

	int32_t		j=0;
//    int32_t  optval = 1;
	TDateTime tTime;
//	int32_t nNetTimeout=1;//1ms

	rt_kprintf("%s Server Main Task Started\n", netif_name);

	//初始化保存每个任务的PARAMS参数结构的指针数组全为0值
	for(j=0;j<MAX_CLIENT_NUM;j++)
	{
		ppParams[j] = 0;
	}

    get_if(netif_name, &ip_addr, RT_NULL, RT_NULL);// IP地址

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

	//注意考虑这个大小设为多少合适
	//if(setsockopt(listenFd,SOL_SOCKET,SO_SNDBUF,(char*)&buffersize,sizeof(buffersize)) == ERROR)
	{
	//	rt_kprintf("setsockopt send buffer size\n");
	//	return ERROR;
	}

    //发送时限
    //setsockopt(listenFd,SOL_SOCKET,SO_SNDTIMEO,(char *)&nNetTimeout,sizeof(int32_t));


	//if(setsockopt(listenFd,SOL_SOCKET,SO_RCVBUF,(char*)&buffersize,sizeof(buffersize)) == ERROR)
	{
	//	rt_kprintf("setsockopt recv buffer size\n");
	//	return ERROR;
	}

//	if(lwip_ioctl(listenFd,FIONBIO,&blockflag)<0)   //非阻塞// no use whs
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
		//accept这段代码会导致ERRNO＝46
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
			rt_kprintf("\n\n新连接的客户端地址为: %x \n\n",clientAddr.sin_addr.s_addr);
//////////////////////////////////////////////////////////////
            if(lwip_ioctl(serverFd,FIONBIO,&blockflag)<0)   //设置serverFd为非阻塞
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
			//检查并设置该地址的客户端的连接，状态,同时限制连接数
			//连接数限制不可以在accept之前进行，否则，在连接数已满时，任何一个连接都不能通过删除前一连接，而重新连接
			if(!getConnState(ppParams,clientAddr.sin_addr.s_addr,serverPort))
			{
				rt_kprintf("已达到最大连接数:%d.忽略新连接\n",MAX_CLIENT_NUM);
				lwip_close(serverFd);
				rt_thread_delay(100);
				NVIC_SystemReset();//实现系统复位
				continue;
			}

			rt_kprintf("A new client connected! serverFd = %d\n",serverFd);

			tTime = Now();
			logMsg("Time  = %02d-%02d-%02d:%02d:%02d:%03d!\n",tTime.month,tTime.day,tTime.hour,tTime.minute,tTime.msec/1000,tTime.msec%1000);//debug

			sprintf(taskName,"Client%d",serverFd);

			childParam.serverFd = serverFd;//子任务用于通讯的Socket
			childParam.s_addr   = clientAddr.sin_addr.s_addr;//客户端(即后台机)的IP地址
			childParam.sin_port = serverPort;//对应服务器(即装置)的Socket端口号
//			childParam.sin_port = clientAddr.sin_port;//客户端(即后台机)的Socket端口号
			childParam.ppParams = ppParams;//主任务管理子任务的参数结构体的指针数组
//			childParam.PortConfig = pPortConfig;//端口配置参数

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

	struct sockaddr_in  serverAddr; /*Server地址*/
	uint32_t       serverPort=OPERATOR_PORT;/*Server端口号*/

	struct sockaddr_in  clientAddr; /*连接的客户端的IP地址*/

	char	taskName[20];
	rt_thread_t	nTaskID;
	CHILDTASKPAR childParam;

	int32_t     listenFd;/* listen socket file descriptor */
	int32_t     serverFd;/* server socket file descriptor */

	u32_t		addrsize;//地址结构体的大小
	int32_t     blockflag = 1;

	int32_t		j=0;
//    int32_t  optval = 1;
	TDateTime tTime;
//	int32_t nNetTimeout=1;//1ms

	rt_kprintf("%s Server Main Task Started IEC104\n", netif_name);

	//初始化保存每个任务的PARAMS参数结构的指针数组全为0值
	for(j=0;j<MAX_CLIENT_NUM;j++)
	{
		ppParams[j] = 0;
	}

    get_if(netif_name, &ip_addr, RT_NULL, RT_NULL);// IP地址

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

//	if(lwip_ioctl(listenFd,FIONBIO,&blockflag)<0)   //非阻塞
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
		//accept这段代码会导致ERRNO＝46
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
			rt_kprintf("\n\nIEC104 新连接的客户端地址为: %x \n\n",clientAddr.sin_addr.s_addr);
//////////////////////////////////////////////////////////////
            if(lwip_ioctl(serverFd,FIONBIO,&blockflag)<0)   //设置serverFd为非阻塞
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
			//检查并设置该地址的客户端的连接，状态,同时限制连接数
			//连接数限制不可以在accept之前进行，否则，在连接数已满时，任何一个连接都不能通过删除前一连接，而重新连接
			if(!getConnState(ppParams,clientAddr.sin_addr.s_addr,serverPort))
			{
				rt_kprintf("已达到最大连接数:%d.忽略新连接 IEC104\n",MAX_CLIENT_NUM);
				lwip_close(serverFd);
				rt_thread_delay(100);//whs20160201
				NVIC_SystemReset();//实现系统复位
				continue;
			}

			rt_kprintf("A new client connected! serverFd = %d IEC104\n",serverFd);

			tTime = Now();
			logMsg("Time  = %02d-%02d-%02d:%02d:%02d:%03d!\n",tTime.month,tTime.day,tTime.hour,tTime.minute,tTime.msec/1000,tTime.msec%1000);//debug

			sprintf(taskName,"Client%d",serverFd);

			childParam.serverFd = serverFd;//子任务用于通讯的Socket
			childParam.s_addr   = clientAddr.sin_addr.s_addr;//客户端(即后台机)的IP地址
			childParam.sin_port = serverPort;//对应服务器(即装置)的Socket端口号
//			childParam.sin_port = clientAddr.sin_port;//客户端(即后台机)的Socket端口号
			childParam.ppParams = ppParams;//主任务管理子任务的参数结构体的指针数组
//			childParam.PortConfig = pPortConfig;//端口配置参数

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

