#include <stdio.h>  
#include <stdlib.h>  
#include <string.h> 
#include "stdint.h"
#include "../applications/tcp_debug.h"
#include "../applications/cmcfgfunc.h"
//extern void phy_int_init(void);
extern struct  UART_TXRX_STATUS uart_status[UARTMAXNUM];

extern struct ETH_TXRX_STATUS eth_status;
extern uint32_t eth_IP1, eth_MASK1, eth_GW1, eth_IP2, eth_MASK2, eth_GW2;
extern uint32_t GetCfgFromCharBuf(CfgHandleStruct* pHandle, uint8_t* buf, CfgOneKeyStruct* pKey);
DEBUGBUF debug_buf;
CfgHandleStruct poutput;
CfgOneKeyStruct keypoint[100];
uint8_t    uarttxflg = 0, uartrxflg = 0; //,closeuarttxflg,closeuartrxflg,openuarttxrxflg,closeuarttxrxflg;
uint8_t    ethtxflg = 0, ethrxflg = 0;   //openethtxflg,openethrxflg,closeethtxflg,closeethrxflg,openethtxrxflg,closeethtxrxflg;

char   UDPTransmitBuffer[300];


char  UDPTransmitBuffer1[96] = { 'T', 'H', 'E', 'R', 'E', ' ',
																	'I', 'S', ' ',
																	'A', ' ',
																	'X', 'J', ' ',
																	'D', 'E', 'V', 'I', 'C', 'E', ':',
																	' ', ' ', '-',
																	' ', ' ', '-',
																	' ', ' ', '-',
																	' ', ' ', '-',
																	' ', ' ', '-',
																	' ', ' ', ' ',
																	'T', 'y', 'p', 'e', ':',
																	0, 0, 0, 0, 0, 0, 0, 0,
																	0, 0, 0, 0, 0, 0, 0, 0,
																	0, 0, 0, 0, 0, 0, 0, 0,
																	0, 0, 0, 0, 0, 0, 0, 0,

																	0, 0, 0, 0, 0, 0, 0, 0,
																	0, 0, 0, 0, 0, 0, 0, 0,
																	0, 0, 0, 0 };
char device_type[32] = { 'C','M','P','-','0','0','0','1',0 };





int datatoblock()
{
	char str[] = "ab,cd,ef";
	char* ptr;
	printf("before strtok:  str=%s\n", str);
	printf("begin:\n");
	ptr = strtok(str, ",");
	while (ptr != NULL) {
		printf("str=%s\n", str);
		printf("ptr=%s\n", ptr);
		ptr = strtok(NULL, ",");
	}
	system("pause");
	return 0;
}




void tcp_debug_init(void)
{
	debug_buf.busy = 0;

	debug_buf.write_p = 0;
	debug_buf.send_p = 0;

	rt_memset(debug_buf.buf, 0x00, sizeof(debug_buf.buf));

	debug_buf.recv_p = 0;
	debug_buf.handle_p = 0;

	rt_memset(debug_buf.rxbuf, 0x00, sizeof(debug_buf.rxbuf));

}





void debug_buf_write(const char* buf, uint8_t size)
{
	uint16_t u16spare;
	uint16_t u16_p;
	DEBUGBUF* pDebug;
	char* pbuf;

	pDebug = &debug_buf;

	while (debug_buf.busy);
	pDebug->busy = 1;

	while (1)
	{
		if (pDebug->write_p == pDebug->send_p)
		{
			u16spare = sizeof(debug_buf.buf);
		}
		else
		{
			if (pDebug->write_p > pDebug->send_p)
			{
				u16spare = sizeof(debug_buf.buf) - pDebug->write_p + pDebug->send_p;
			}
			else
			{
				u16spare = pDebug->send_p - pDebug->write_p;
			}
		}

		if (u16spare < (size + 3))
		{

			u16_p = pDebug->send_p;
			pbuf = &(pDebug->buf[u16_p]);
			while (1)
			{
				if (*pbuf)
				{
					*pbuf = '\0';
					u16_p++;
					if (u16_p >= sizeof(debug_buf.buf))
					{
						u16_p = 0;
						pbuf = pDebug->buf;
					}
					pbuf++;
				}
				else
				{
					u16_p++;
					if (u16_p >= sizeof(debug_buf.buf))
					{
						u16_p = 0;
					}
					break;
				}
			}
			pDebug->send_p = u16_p;
		}
		else
		{
			break;
		}
	}

	u16_p = pDebug->write_p;
	pbuf = &(pDebug->buf[u16_p]);
	while (size)
	{
		*pbuf = *buf++;
		size--;

		u16_p++;
		if (u16_p >= sizeof(debug_buf.buf))
		{
			u16_p = 0;
			pbuf = pDebug->buf;
		}
		else
			pbuf++;
	}

	*pbuf = '\0';

	u16_p++;
	if (u16_p >= sizeof(debug_buf.buf))
	{
		u16_p = 0;
	}

	pDebug->write_p = u16_p;

	pDebug->busy = 0;

}




uint16_t debug_send_ready(char* buf)
{
	DEBUGBUF* pDebug;
	uint16_t u16_p;
	uint32_t  length;
	char* pbuf;

	pDebug = &debug_buf;

	while (debug_buf.busy);

	if (pDebug->send_p == pDebug->write_p)
		return 0x00;

	pDebug->busy = 1;

	u16_p = pDebug->send_p;
	pbuf = &(pDebug->buf[u16_p]);
	length = 0;

	while (1)
	{
		*buf = *pbuf;
		length++;
		if (length > 255)
		{
			break;
		}
		u16_p++;

		if (u16_p >= sizeof(debug_buf.buf))
		{
			u16_p = 0;
			pbuf = pDebug->buf;
		}
		else
		{
			pbuf++;
		}

		if (*buf == '\0')
		{
			pDebug->send_p = u16_p;
			break;
		}

		buf++;
	}

	pDebug->busy = 0;

	return length;
}
/*
uint8_t	tcp_debug_printf(const char *fmt, ...)
{
	va_list args;
	uint8_t length;

	static char buf[DEBUG_PRINTF_MAX];

	va_start(args, fmt);
	length = vsnprintf(buf, sizeof(buf), fmt, args);

	debug_buf_write(buf,length);

	va_end(args);

	return length;
}
*/
uint16_t debug_recv_ready(char* buf, uint32_t size)
{
	uint16_t u16spare;
	uint16_t u16_p;
	DEBUGBUF* pDebug;
	char* pbuf;

	pDebug = &debug_buf;

	if (size == 0xFFFFFFFF)
		return 0x0000;

	if (pDebug->handle_p == pDebug->recv_p)
	{
		u16spare = sizeof(debug_buf.rxbuf);
	}
	else
	{
		if (pDebug->recv_p > pDebug->handle_p)
		{
			u16spare = sizeof(debug_buf.rxbuf) - pDebug->recv_p + pDebug->handle_p;
		}
		else
		{
			u16spare = pDebug->handle_p - pDebug->recv_p;
		}
	}

	if (u16spare < size + 2)
	{
		return 0xFFFF;
	}

	u16_p = pDebug->recv_p;
	pbuf = &(pDebug->rxbuf[u16_p]);
	u16spare = size;
	while (size)
	{
		*pbuf = *buf++;
		size--;

		u16_p++;
		if (u16_p >= sizeof(debug_buf.rxbuf))
		{
			u16_p = 0;
			pbuf = pDebug->rxbuf;
		}
		else
			pbuf++;
	}

	pDebug->recv_p = u16_p;

	return u16spare;
}




uint32_t debug_buf_read(char* buf, uint32_t size, uint32_t flag)
{
	uint16_t u16UMhandle;
	uint16_t u16_p;
	DEBUGBUF* pDebug;
	char* pbuf;

	pDebug = &debug_buf;

	if (pDebug->recv_p == pDebug->handle_p)
	{
		u16UMhandle = 0;
		return u16UMhandle;
	}
	else
	{
		if (pDebug->handle_p > pDebug->recv_p)
		{
			u16UMhandle = sizeof(debug_buf.rxbuf) - pDebug->handle_p + pDebug->recv_p;
		}
		else
		{
			u16UMhandle = pDebug->recv_p - pDebug->handle_p;
		}
	}

	if (u16UMhandle > size)
		u16UMhandle = size;
	else
		size = u16UMhandle;

	u16_p = pDebug->handle_p;
	pbuf = &(pDebug->rxbuf[u16_p]);

	while (size)
	{
		*buf++ = *pbuf;
		size--;

		u16_p++;
		if (u16_p >= sizeof(debug_buf.rxbuf))
		{
			u16_p = 0;
			pbuf = &(pDebug->rxbuf[u16_p]);
		}
		else
		{
			pbuf++;
		}
	}

	if (flag == 0)
	{
		pDebug->handle_p = u16_p;
	}

	return u16UMhandle;
}



void DebugCommuTsk(void* parameter)
{
	struct ip_addr	ip_addr;
	char* netif_name = (char*)parameter;

	struct sockaddr_in	serverAddr; /*Server地址*/
	uint32_t	serverPort = 9750;/*Server端口号*/

	struct sockaddr_in	clientAddr;  /*连接的客户端的IP地址*/

	int32_t     listenFd;/* listen socket file descriptor */
	int32_t     serverFd;/* server socket file descriptor */

	u32_t	addrsize;//地址结构体的大小
	int32_t     blockflag = 1;

	int32_t 	recv_count = 0;
	char		buf[256];

	get_if(netif_name, &ip_addr, RT_NULL, RT_NULL);// IP地址

	addrsize = sizeof(struct sockaddr_in);
	memset((char*)& serverAddr, 0, sizeof(struct sockaddr_in));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_len = (uint8_t)sizeof(struct sockaddr_in);
	serverAddr.sin_port = htons(serverPort);
	serverAddr.sin_addr.s_addr = (ip_addr.addr);//htonl(INADDR_ANY);

	if ((listenFd = socket(AF_INET, SOCK_STREAM, 0)) == ERROR)
	{
		return;// ERROR;
	}

	if (lwip_ioctl(listenFd, FIONBIO, &blockflag) < 0)   //非阻塞
	{
		lwip_close(listenFd);
		return;// ERROR;
	}

	if (bind(listenFd, (struct sockaddr*) & serverAddr, sizeof(struct sockaddr_in)) == ERROR)
	{
		lwip_close(listenFd);
		return;// ERROR;
	}

	if (listen(listenFd, 1) == ERROR)
	{
		lwip_close(listenFd);
		return;// ERROR;
	}

	serverFd = -1;
	while (1)
	{
		if (serverFd != ERROR)
		{
			recv_count = recv(serverFd, buf, 256, 0);
			if (recv_count == 0)
			{
				lwip_close(serverFd);
				serverFd = ERROR;
			}
			else
			{
				if (recv_count > 0)
				{
					debug_recv_ready(buf, recv_count);
				}

				recv_count = debug_send_ready(buf);
				if (recv_count)
				{
					send(serverFd, buf, recv_count, 0);
				}
			}

		}
		else
		{
			if (((serverFd = accept(listenFd, (struct sockaddr*) & clientAddr, &addrsize)) == ERROR) && (errno != EWOULDBLOCK))
			{
				//return (ERROR);
				continue;
			}
			if (serverFd > 0)
			{
				//accept这段代码会导致ERRNO＝46
				/////////////////////////////////////////////////////////////
				if (lwip_ioctl(serverFd, FIONBIO, &blockflag) < 0)   //设置serverFd为非阻塞
				{
					lwip_close(serverFd);
					continue;
				}

				///////////////////////////////////////////////////////////////
							//检查并设置该地址的客户端的连接，状态,同时限制连接数
							//连接数限制不可以在accept之前进行，否则，在连接数已满时，任何一个连接都不能通过删除前一连接，而重新连接
							/*
							if(!getConnState(ppParams,clientAddr.sin_addr.s_addr,serverPort))
							{
								lwip_close(serverFd);
								continue;
							}
							*/
			}
		}

		rt_thread_delay(RT_TICK_PER_SECOND / 100); // delay 10ms
	}



}



//增加帧头
static uint8_t addframeheader(char* outputbuf, char* inputbuf)
{
	int8_t i;
	for (i = 0; i < 10; i++)
	{
		outputbuf[i] = inputbuf[i];
	}
	return i;
}

//建立链接
static uint8_t creatlinktoask()
{

}

//回传数据处理
static uint8_t transaddframheader(char* outputbuf, uint8_t datatype, char chatype, uint8_t chanum)// 数据类型（发送 接收)    通道类型（以太网 串口）
{
	uint8_t  i;
	uint16_t tempdata = 0;
	i = 0;
	outputbuf[i++] = (START_CODE >> 8) & 0xff;
	outputbuf[i++] = START_CODE & 0xff;
	outputbuf[i++] = 0x00;
	outputbuf[i++] = 0x00;
	outputbuf[i++] = 0x00;
	outputbuf[i++] = 0x00;
	outputbuf[i++] = 0x00;
	outputbuf[i++] = 0x00;

	if (datatype == 0)
	{
		if (chanum > 2)
		{
			chanum = chanum - 1;
		}
		tempdata = CMPROTOCOL_COMMAND_STREAM_DATA | CMPROTOCOL_COMMAND_STREAM_DATA_STREAM_TX | chanum;
	}
	else
	{
		if (chanum > 2)
		{
			chanum = chanum - 1;
		}
		tempdata = CMPROTOCOL_COMMAND_STREAM_DATA | CMPROTOCOL_COMMAND_STREAM_DATA_STREAM_RX | chanum;
	}

	if (chatype == 0)//eth
	{
		tempdata = tempdata | CMPROTOCOL_COMMAND_STREAM_DATA_UPSTREAM_BASE;
	}
	else
	{
		tempdata = tempdata | CMPROTOCOL_COMMAND_STREAM_DATA_DEVICESTREAM_BASE;
	}
	outputbuf[i++] = (tempdata >> 8) & 0xff;
	outputbuf[i++] = tempdata & 0xff;
	return i;

}
char device_inf[20] =
{                          //VER                 //CRC                     //DATA
	'C' , 'M' , 'P' , '-' , '0' , '1' , '0' , '0' , '1' , '2' , '3' , '4' , '2' , '0' , '1' , '9' , '0' , '8' , '0' , '5'

};



//公共命令 子健 初始化，获取装置信息
//查询 本装置基本信息 //55AA0023000000000001FFFF0100201712120008000041646D696E6973747261746F72
static uint16_t senddeviceinf(char* outputbuf, char* inputbuf)
{
	uint8_t temp, i, j;
	for (i = 0; i < inputbuf[3]; i++)
	{
		outputbuf[i] = inputbuf[i];
	}

	for (j = 0; j < sizeof(device_inf); j++, i++)
	{
		outputbuf[i] = device_inf[j];
	}
	outputbuf[3] = sizeof(device_inf) + 10;
	return i;


	//
}
//查询 串口 上行下行数据
//查询 网口 网口上行下行数据

//查询 crc 版本 



//公共命令 子健 探知装置
static uint8_t sendcommdata(char* outputbuf, char* inputbuf)
{
	uint8_t i, m;

	m = addframeheader(outputbuf, inputbuf);

	/*for(i= m;i<(sizeof(device_inf)+10);i++)
	  {
			outputbuf[i] = device_inf[i-10];
		}*/

	outputbuf[3] = m;
	return m;


}



char wrong_inf[9] =
{
	'W' , 'R' , 'O' , 'N' , 'G' , 'D' , 'A' , 'T' , 'A'
};


//错误命令
static uint8_t wrongcmdack(char* outputbuf, char* inputbuf)
{
	uint8_t i, m;

	m = addframeheader(outputbuf, inputbuf);

	for (i = m; i < (sizeof(wrong_inf) + 10); i++)
	{
		outputbuf[i] = wrong_inf[i - 10];
	}

	outputbuf[3] = i;
	return i;
}


uint8_t delet_rt_threadflg;
void delet_rt_thread(void)
{
	delet_rt_threadflg = 1;


}

extern void fm3_emac_hw_init(void);
extern void set_if_h(char* netif_name, uint32_t ip_addr, uint32_t gw_addr, uint32_t nm_addr);


void UpdataSearchTsk(void* parameter)
{
	struct ip_addr			ip_addr;
	uint8_t mac_addr[6];
	char* netif_name = (char*)parameter;

	struct sockaddr_in	serverAddr; /*Server??*/
	uint32_t						serverPort = 9378;//9761;/*Server???*/
	static char 	pbuf[1536];
	struct sockaddr_in	clientAddr;  /*???????IP??*/

	int32_t     listenFd;/* listen socket file descriptor */

	u32_t				addrsize;//????????
	int32_t     blockflag = 1;

	uint32_t		i;
	uint16_t		wPutLen;
	uint8_t			ucdata;
	int32_t 		recv_count = 0;

	uint16_t   ctrldata;
	uint8_t    readethflg, readuarttransflg = 1, readuartrecvflg = 1;

	uint8_t    recvflg = 0;
	uint16_t   framheader;
	uint8_t    summonflg = 0;

	get_if(netif_name, &ip_addr, RT_NULL, RT_NULL);// IP??

	addrsize = sizeof(struct sockaddr_in);
	memset((char*)& serverAddr, 0, sizeof(struct sockaddr_in));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_len = (uint8_t)sizeof(struct sockaddr_in);
	serverAddr.sin_port = htons(serverPort);
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if ((listenFd = socket(AF_INET, SOCK_DGRAM, 0)) == ERROR)
	{
		return;// ERROR;
	}

	if (lwip_ioctl(listenFd, FIONBIO, &blockflag) < 0)   //???
	{
		lwip_close(listenFd);
		return;// ERROR;
	}

	if (bind(listenFd, (struct sockaddr*) & serverAddr, sizeof(struct sockaddr_in)) == ERROR)
	{
		lwip_close(listenFd);
		return;// ERROR;
	}
	//CfgHandleInit(&poutput);
	while (1)
	{
		recv_count = recvfrom(listenFd, pbuf, 1536, 0, (struct sockaddr*) & clientAddr, &addrsize);


		if (recv_count > 0)////55 AA 00 0A 00 00 00 00 00 00
		{
			if (recv_count < pbuf[3])//
				goto udpsleep;//return;
			framheader = (pbuf[0] << 8) | pbuf[1];
			if (framheader == START_CODE)//有效数据
			{
				wPutLen = 0;
				ctrldata = (pbuf[8] << 8) | pbuf[9];


				if (ctrldata == CMPROTOCOL_COMMAND_COMMON_DISCOVER)
				{
					wPutLen = sendcommdata(UDPTransmitBuffer, pbuf);
				}
				else if (ctrldata == CMPROTOCOL_COMMAND_COMMON_INIT)
				{
					wPutLen = senddeviceinf(UDPTransmitBuffer, pbuf);
				}
				else if ((ctrldata & CMPROTOCOL_COMMAND_STREAM_CTL) == CMPROTOCOL_COMMAND_STREAM_CTL)//ctrl cmd 报文控制命令 主键
				{

					if ((ctrldata & CMPROTOCOL_COMMAND_STREAM_CTL_STREAM_OFF) == CMPROTOCOL_COMMAND_STREAM_CTL_STREAM_OFF)//close the data transmit 报文控制命令 子健 控制通道上送报文禁止
					{

						if ((ctrldata & CMPROTOCOL_COMMAND_STREAM_CTL_STREAM_TX) == CMPROTOCOL_COMMAND_STREAM_CTL_STREAM_TX)//tx
						{
							if ((ctrldata & CMPROTOCOL_COMMAND_STREAM_CTL_DEVICESTREAM_BASE_ALL) == CMPROTOCOL_COMMAND_STREAM_CTL_DEVICESTREAM_BASE_ALL)//device
							{
								uarttxflg = uarttxflg & (~(ctrldata & CMPROTOCOL_COMMAND_STREAM_CTL_UPSTREAM_BASE_ALL));
								if (uarttxflg > 2)
									uarttxflg += 1;
							}
							if ((ctrldata & CMPROTOCOL_COMMAND_STREAM_CTL_UPSTREAM_BASE_ALL) == CMPROTOCOL_COMMAND_STREAM_CTL_UPSTREAM_BASE_ALL)//eth
							{
								ethtxflg = ethtxflg & (~(ctrldata & CMPROTOCOL_COMMAND_STREAM_CTL_UPSTREAM_BASE_ALL));

							}
						}
						if ((ctrldata & CMPROTOCOL_COMMAND_STREAM_CTL_STREAM_RX) == CMPROTOCOL_COMMAND_STREAM_CTL_STREAM_RX)//rx
						{
							if ((ctrldata & CMPROTOCOL_COMMAND_STREAM_CTL_DEVICESTREAM_BASE_ALL) == CMPROTOCOL_COMMAND_STREAM_CTL_DEVICESTREAM_BASE_ALL)//device
							{
								uartrxflg = uartrxflg & (~(ctrldata & CMPROTOCOL_COMMAND_STREAM_CTL_UPSTREAM_BASE_ALL));
								if (uartrxflg > 2)
									uartrxflg += 1;
							}
							if ((ctrldata & CMPROTOCOL_COMMAND_STREAM_CTL_UPSTREAM_BASE_ALL) == CMPROTOCOL_COMMAND_STREAM_CTL_UPSTREAM_BASE_ALL)//eth
							{
								ethrxflg = ethrxflg & (~(ctrldata & CMPROTOCOL_COMMAND_STREAM_CTL_UPSTREAM_BASE_ALL));
							}
						}


					}
					else if ((ctrldata & CMPROTOCOL_COMMAND_STREAM_CTL_STREAM_ON) == CMPROTOCOL_COMMAND_STREAM_CTL_STREAM_ON)//open the data transmit	
					{

						if ((ctrldata & CMPROTOCOL_COMMAND_STREAM_CTL_STREAM_TX) == CMPROTOCOL_COMMAND_STREAM_CTL_STREAM_TX)//tx
						{
							if ((ctrldata & CMPROTOCOL_COMMAND_STREAM_CTL_DEVICESTREAM_BASE) == CMPROTOCOL_COMMAND_STREAM_CTL_DEVICESTREAM_BASE)//device
							{
								uarttxflg = ctrldata & CMPROTOCOL_COMMAND_STREAM_CTL_UPSTREAM_BASE_ALL;
								if (uarttxflg > 2)
									uarttxflg += 1;
								summonflg = 1;
							}
							if ((ctrldata & CMPROTOCOL_COMMAND_STREAM_CTL_DEVICESTREAM_BASE) != CMPROTOCOL_COMMAND_STREAM_CTL_DEVICESTREAM_BASE)//eth
							{
								ethtxflg = ctrldata & CMPROTOCOL_COMMAND_STREAM_CTL_UPSTREAM_BASE_ALL;
								summonflg = 1;
							}
						}
						if ((ctrldata & CMPROTOCOL_COMMAND_STREAM_CTL_STREAM_RX) == CMPROTOCOL_COMMAND_STREAM_CTL_STREAM_RX)//rx
						{
							if ((ctrldata & CMPROTOCOL_COMMAND_STREAM_CTL_DEVICESTREAM_BASE) == CMPROTOCOL_COMMAND_STREAM_CTL_DEVICESTREAM_BASE)//device
							{
								uartrxflg = ctrldata & CMPROTOCOL_COMMAND_STREAM_CTL_UPSTREAM_BASE_ALL;
								if (uartrxflg > 2)
									uartrxflg += 1;
								summonflg = 1;
							}
							if ((ctrldata & CMPROTOCOL_COMMAND_STREAM_CTL_DEVICESTREAM_BASE) != CMPROTOCOL_COMMAND_STREAM_CTL_DEVICESTREAM_BASE)//eth
							{
								ethrxflg = ctrldata & CMPROTOCOL_COMMAND_STREAM_CTL_UPSTREAM_BASE_ALL;
								summonflg = 1;
							}
						}


					}
					wPutLen = addframeheader(UDPTransmitBuffer, pbuf);

				}
				else if ((ctrldata & CMPROTOCOL_COMMAND_CFG) == CMPROTOCOL_COMMAND_CFG)//get config file	
				{
					wPutLen = addframeheader(UDPTransmitBuffer, pbuf);
					UDPTransmitBuffer[2] = 0x00;
					UDPTransmitBuffer[3] = 0x0a;
					GetCfgFromCharBuf(&poutput, &pbuf[10], keypoint);
				}


				//test
				else if (ctrldata == CMPROTOCOL_COMMAND_COMMON_RESET)//装置重启
				{
					NVIC_SystemReset();//
				}
			}

			else
			{
				wrongcmdack(UDPTransmitBuffer, pbuf);
			}


			UDPTransmitBuffer[wPutLen++] = 0;
			sendto(listenFd, UDPTransmitBuffer, wPutLen, 0, (struct sockaddr*) & clientAddr, addrsize);


		}




		// 
		if (summonflg == 1)
		{
			if ((ethrxflg | ethtxflg | uartrxflg | uarttxflg) == 0)
			{
				summonflg = 0;


			}
			else
			{
				if (uarttxflg != 0)
				{
					//if(uarttxflg > 2)
					  //	uarttxflg = uarttxflg -1;
					if (uart_status[uarttxflg - 1].uart_trans_flg == 1)
					{
						transaddframheader(UDPTransmitBuffer, CMPROTOCOL_COMMAND_TXFLG, CMPROTOCOL_COMMAND_DOWNFLG, uarttxflg);

						UDPTransmitBuffer[2] = ((uart_status[uarttxflg - 1].uart_trans_buf[0] + 10) >> 8) & 0xff;
						UDPTransmitBuffer[3] = (uart_status[uarttxflg - 1].uart_trans_buf[0] + 10) & 0xff;

						for (wPutLen = 10; wPutLen < uart_status[uarttxflg - 1].uart_trans_buf[0] + 10; wPutLen++)
						{
							UDPTransmitBuffer[wPutLen] = uart_status[uarttxflg - 1].uart_trans_buf[wPutLen - 10 + 1];
						}

						uart_status[uarttxflg - 1].uart_trans_flg = 0;
						sendto(listenFd, UDPTransmitBuffer, wPutLen, 0, (struct sockaddr*) & clientAddr, addrsize);
						// sendto(listenFd,&uart_status[uarttxflg-1].uart_trans_buf[1],uart_status[uarttxflg-1].uart_trans_buf[0],0,(struct sockaddr *)&clientAddr, addrsize);
							  //  uart_status[uarttxflg-1].uart_trans_flg	= 0;
					}

				}
				if (uartrxflg != 0)
				{
					//  if(uartrxflg > 2)
					  //		  uartrxflg = uartrxflg -1;

					if (uart_status[uartrxflg - 1].uart_recv_flg == 1)
					{
						wPutLen = 0;


						transaddframheader(UDPTransmitBuffer, CMPROTOCOL_COMMAND_RXFLG, CMPROTOCOL_COMMAND_DOWNFLG, uartrxflg);
						UDPTransmitBuffer[2] = ((uart_status[uarttxflg - 1].uart_recv_buf[0] + 10) >> 8) & 0xff;
						UDPTransmitBuffer[3] = (uart_status[uarttxflg - 1].uart_recv_buf[0] + 10) & 0xff;



						for (wPutLen = 10; wPutLen < uart_status[uartrxflg - 1].uart_recv_buf[0] + 10; wPutLen++)
						{
							UDPTransmitBuffer[wPutLen] = uart_status[uartrxflg - 1].uart_recv_buf[wPutLen - 10 + 1];
						}
						uart_status[uartrxflg - 1].uart_recv_flg = 0;

						sendto(listenFd, UDPTransmitBuffer, wPutLen, 0, (struct sockaddr*) & clientAddr, addrsize);
						//sendto(listenFd,&uart_status[uartrxflg -1].uart_recv_buf[1],uart_status[uartrxflg -1].uart_recv_buf[0],0,(struct sockaddr *)&clientAddr, addrsize);	
						//uart_status[uartrxflg-1].uart_recv_flg	= 0;	
					}

				}

				if (ethrxflg != 0)
				{
					if (eth_status.eth_recv_flg == 1)
					{

						transaddframheader(eth_status.eth_recv_buf, CMPROTOCOL_COMMAND_RXFLG, CMPROTOCOL_COMMAND_UPFLG, ethrxflg);
						eth_status.eth_recv_buf[2] = ((eth_status.eth_recv_buf[11] + 12) >> 8) & 0xff;
						eth_status.eth_recv_buf[3] = (eth_status.eth_recv_buf[11] + 12) & 0xff;
						sendto(listenFd, eth_status.eth_recv_buf, eth_status.eth_recv_buf[11] + 12, 0, (struct sockaddr*) & clientAddr, addrsize);
						eth_status.eth_recv_flg = 0;
					}
				}

				if (ethtxflg != 0)
				{
					if (eth_status.eth_trans_flg == 1)
					{
						transaddframheader(eth_status.eth_trans_buf, CMPROTOCOL_COMMAND_TXFLG, CMPROTOCOL_COMMAND_UPFLG, ethtxflg);
						eth_status.eth_trans_buf[2] = ((eth_status.eth_trans_buf[11] + 12) >> 8) & 0xff;
						eth_status.eth_trans_buf[3] = (eth_status.eth_trans_buf[11] + 12) & 0xff;
						sendto(listenFd, eth_status.eth_trans_buf, eth_status.eth_trans_buf[11] + 12, 0, (struct sockaddr*) & clientAddr, addrsize);
						eth_status.eth_trans_flg = 0;
					}
				}
			}


		}


	udpsleep:
		rt_thread_delay(RT_TICK_PER_SECOND / 50); // delay 20ms   
	}


}




void eth_server_debug(void)
{
	rt_thread_t tid;

	tid = rt_thread_create("ServerDebug", DebugCommuTsk, (void*)"e0",
		1024, 21, 5);
	if (tid != RT_NULL) rt_thread_startup(tid);

	tid = rt_thread_create("ServerSearch", UpdataSearchTsk, (void*)"e0",
		2048, 21, 5);
	if (tid != RT_NULL) rt_thread_startup(tid);

}



/*//'CALL ALIVE XJ DEVICE'// 判断是搜索还是发数据 记下客户端IP和端口
// 有接收到数据，把末端清零 //
pbuf[recv_count] = '\0';
if (strcmp(pbuf, "q") == 0 || strcmp(pbuf, "Q") == 0)
{
// 如果是首字母是q或Q，关闭这个连接
	   lwip_close(listenFd);
	   break;
}
else if (strcmp(pbuf, "exit") == 0)//'exit'
{
// 如果接收的是exit，则关闭整个服务端
	lwip_close(listenFd);

	//stop = RT_TRUE;
	break;
}

	  if(strcmp(pbuf, "uart0") == 0)//read uart x tx and rx  data
	  {
		  recvflg = 1;
		  //break;
	  }

	  if(strcmp(pbuf, "eth0") == 0)// read eth x tx and rx data
	  {
		  recvflg = 1;
		  //break;
	  }


	  mac_addr[0] = 0x00;
	  mac_addr[1] = (ip_addr.addr>>24)&0xff;
	  mac_addr[2] = (ip_addr.addr>>16)&0xff;
	  mac_addr[3] = (ip_addr.addr>>8)&0xff;
	  mac_addr[4] = (ip_addr.addr)&0xff;
	  mac_addr[5] = 0x0B;



	  for(i = 0; i < 6; i++)
	  {
		  ucdata = ((mac_addr[i] & 0xf0) >> 4);
		  if(ucdata < 0x0a)
			  ucdata += 0x30;
		  else
			  ucdata += 0x37;
		  UDPTransmitBuffer[21 + i * 3] = ucdata;
		  ucdata = mac_addr[i] & 0x0f;
		  if(ucdata < 0x0a)
			  ucdata += 0x30;
		  else
			  ucdata += 0x37;
		  UDPTransmitBuffer[22 + i * 3] = ucdata;
	  }
	  wPutLen = 44;
	  for(i = 0; i < 32; i++)
	  {
		  ucdata = device_type[i];
		  if(ucdata)
		  {
			  UDPTransmitBuffer[wPutLen + i] = ucdata;
		  }
		  else
		  {
			  UDPTransmitBuffer[wPutLen + i] = ' ';
			  i++;
			  break;
		  }
	  }
	  wPutLen += i;*/





