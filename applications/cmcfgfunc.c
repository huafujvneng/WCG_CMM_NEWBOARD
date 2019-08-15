//#include "stdafx.h"				//非MFC应用时屏蔽掉这一句
#include "applications/cmcfgfunc.h"
#include "base_task.h"


extern struct  UART_CONF uartpara[UARTMAXNUM] ;
extern struct ETH_CONF ethpara;
extern struct FILE_INFDEFINE fileinf;		
//extern struct CfgOneKeyStruct keypoint[100];


#ifndef THROW_STRING
#define nullptr NULL
#define strtok_s strtok_r
#define _stricmp strcmp
#define _strnicmp strncmp
#endif
#define sscanf_s sscanf
uint8_t asc2bcd(uint8_t *pDes, uint8_t des_size, char *pSrc, uint8_t length)
{
	uint8_t size = 0;
	
	while (length && des_size)
	{
		*pDes = pSrc[0] & 0x0F;
		des_size--;
		size++;
		length--;
		if (!length)
			break;
		*pDes <<= 4;
		*pDes |= pSrc[1] & 0x0F;
		pDes ++;
		length--;
		pSrc += 2;
	}
	return size;
}

uint32_t CfgHandleInit(CfgHandleStruct *pHandle)
{
	pHandle->state = idle;
	pHandle->error = none;
	pHandle->b_define_right = false;
	pHandle->b_start_read = false;

	return 0x0;
}
void saveconfhandle(void)
{
	  
    rt_mb_send(&mb, TYPE_RECORD_ETH);//发存储邮件
		
}

uint32_t GetCfgFromCharBuf(CfgHandleStruct *pHandle, uint8_t *buf, CfgOneKeyStruct *pKey)
{
#define GetCfgError(X)   {pHandle->error = (X);return u32ResultNum;}

	uint32_t    u32ResultNum = 0;
	size_t      posH = 0, posT = 0, posS = 0;
	const char  *pNote = nullptr;
	char        *pTok = nullptr, *pTok_Next = nullptr;
	char        CHroot = 0;
	size_t      discard = 0;
	size_t      length;
	int32_t     i32data;
	int32_t     ipaddr[4];
  char        savetype = 0;//define(1) channel(2) device(3)  upstream(4)
  char        uart_index;
  char        i;
  char        tempbuf[21];
#ifdef THROW_STRING
#define throw_one_string(pSrc) {if(pHandle->throw_string_index < 7){strcpy_s(pHandle->throw_string[pHandle->throw_string_index], 64, (pSrc));pHandle->throw_string_index++;}}
	pHandle->throw_string_index = 0;
#endif

	length = strlen((const char *)buf);
	//先删掉注释
	pNote = strstr((const char *)buf, "\\");
	if (pNote != nullptr)
	{
		length -= strlen(pNote);
		*(char *)&pNote[0] = 0;
	}
	pNote = strstr((const char *)buf, "//");
	if (pNote != nullptr)
	{
		length -= strlen(pNote);
		*(char *)&pNote[0] = 0;
	}
  CfgHandleInit(pHandle);
 
	while (length)
	{
		posH = strcspn((const char *)buf, "<[");//找到第一个控制字符限定词
		if (posH >= length)
			GetCfgError(format);
		CHroot = buf[posH];
		buf += posH;
		length -= posH;
		posH = 0;
		posH = strspn((const char *)&buf[posH + 1], " ");//找到第一个控制字符
		if (posH >= length)
			GetCfgError(format);
		posH++;
		posT = strcspn((const char *)&buf[posH], ">]") + posH;//找到控制字符限定词结尾
		if (posT >= length)
			GetCfgError(unfinished);
		buf[posT] = 0;//这里将本循环所要解析的字符串最后的控制字符变为0，保证后面的strtok_s不会越界
		discard = posT + 1;
		posS = strcspn((const char *)&buf[posH], " ") + posH;//找到最后一个控制字符
		if (posS < length)
		{
			posT = posS;
		}
		posT--;

		switch (CHroot)
		{
		case '<'://块起始
			if (_strnicmp((const char *)&buf[posH], "define", posT - posH + 1) == 0)
			{
				if (pHandle->state != idle)
					GetCfgError(format);
				pHandle->state = read_define;
				for(i=0;i<8;i++)
				{
					uartpara[i].uart_usedflg = 0;//暂停串口通信
				}
    				
			}
			else if (_strnicmp((const char *)&buf[posH], "channel", posT - posH + 1) == 0)
			{
				if (pHandle->state != idle)
					GetCfgError(format);
				pHandle->state = read_channel;
			}
			else if (_strnicmp((const char *)&buf[posH], "device", posT - posH + 1) == 0)
			{
				if (pHandle->state != idle)
					GetCfgError(format);
				pHandle->state = read_device;
			}
			else if (_strnicmp((const char *)&buf[posH], "upstream", posT - posH + 1) == 0)
			{
				if (pHandle->state != idle)
					GetCfgError(format);
				pHandle->state = read_upstream;
			}
			else if (_strnicmp((const char *)&buf[posH], "define@", posT - posH + 1) == 0)
			{
				if (pHandle->state != read_define)
					GetCfgError(format);
				pHandle->state = idle;
			}
			else if (_strnicmp((const char *)&buf[posH], "channel@", posT - posH + 1) == 0)
			{
				if (pHandle->state != read_channel)
					GetCfgError(format);
				pHandle->state = idle;
			
			}
			else if (_strnicmp((const char *)&buf[posH], "device@", posT - posH + 1) == 0)
			{
				if (pHandle->state != read_device)
					GetCfgError(format);
				pHandle->state = idle;
			}
			else if (_strnicmp((const char *)&buf[posH], "upstream@", posT - posH + 1) == 0)
			{
				if (pHandle->state != read_upstream)
					GetCfgError(format);
				pHandle->state = idle;
				saveconfhandle();
			}
			else
			{
				GetCfgError(format);
			}
			length -= discard;
			buf += discard;
			continue;
		case '['://属性起始
			switch(pHandle->state)
			{
			case read_define:
				//读属性关键字
			  savetype  = 1;
				pTok = strtok_s((char *)&buf[posH], " =]", &pTok_Next);

				//第一个属性
				do
				{
					if (_stricmp(pTok, "support") == 0)
					{
						pTok = strtok_s(nullptr, " =]", &pTok_Next);
						if (_stricmp(pTok, "wcm6000") != 0)
							GetCfgError(format);
						pKey->u16CfgType = KEY_CFG_SUPPORT;
						pKey->i32CfgValue = 1;
						pKey++;
						u32ResultNum++;

					}
					else if (_stricmp(pTok, "type") == 0)
					{
						pTok = strtok_s(nullptr, " =]", &pTok_Next);
						if (_stricmp(pTok, "cfg") == 0)
						{
							pKey->u16CfgType = KEY_CFG_TYPE;
							pKey->i32CfgValue = 0;
							pKey++;
							u32ResultNum++;
						}
						else if (_stricmp(pTok, "model") == 0)
						{
							pKey->u16CfgType = KEY_CFG_TYPE;
							pKey->i32CfgValue = 1;
							pKey++;
							u32ResultNum++;
						}
						else
							GetCfgError(format);
					}
					else if (_stricmp(pTok, "ver") == 0)
					{
						pTok = strtok_s(nullptr, " =]", &pTok_Next);
						pKey->u16CfgType = KEY_CFG_VER;
						sscanf_s(pTok, "%d", &(pKey->i32CfgValue));
						pKey++;
						u32ResultNum++;
					}
					else if (_stricmp(pTok, "user") == 0)
					{
						pTok = strtok_s(nullptr, " =]", &pTok_Next);
						pKey->u16CfgType = KEY_CFG_USER;
						pKey->i32CfgValue = -1;
#ifdef THROW_STRING
						throw_one_string(pTok);
						pKey->i32CfgValue = pHandle->throw_string_index - 1;
#endif
						pKey++;
						u32ResultNum++;
					}
					else if (_stricmp(pTok, "date") == 0)
					{
						pTok = strtok_s(nullptr, " =]", &pTok_Next);
						pKey->u16CfgType = KEY_CFG_DATE;
						if (strlen(pTok) > 8)
						{
							GetCfgError(format);
						}
						asc2bcd((uint8_t *)&pKey->i32CfgValue, sizeof(pKey->i32CfgValue), pTok, 8);
						pKey++;
						u32ResultNum++;
					}
					else if (_stricmp(pTok, "time") == 0)
					{
						pTok = strtok_s(nullptr, " =]", &pTok_Next);
						pKey->u16CfgType = KEY_CFG_TIME;
						if (strlen(pTok) > 6)
						{
							GetCfgError(format);
						}
						asc2bcd((uint8_t *)&pKey->i32CfgValue, sizeof(pKey->i32CfgValue), pTok, 6);
						pKey++;
						u32ResultNum++;
					}
					else
					{//无效属性 不解析

					}

					pTok = strtok_s(nullptr, " =]", &pTok_Next);
				} while (pTok != nullptr);
				break;
			case read_channel:
				//读属性关键字
			  savetype  = 2;
				pTok = strtok_s((char *)&buf[posH], " =]", &pTok_Next);

				//第一个属性
				do
				{
					if (_stricmp(pTok, "index") == 0)
					{
						pTok = strtok_s(nullptr, " =]", &pTok_Next);
						pKey->u16CfgType = KEY_CHANNEL_INDEX;
						sscanf_s(pTok, "%d", &(pKey->i32CfgValue));
						uart_index = pKey->i32CfgValue;
						if(uart_index >= 2)
							uart_index += 1;
						uartpara[uart_index].uart_usedflg = 1;
						uartpara[uart_index].channel_index = pKey->i32CfgValue;						
						pKey++;
						u32ResultNum++;
						
					}
					else if (_stricmp(pTok, "num") == 0)
					{
						pTok = strtok_s(nullptr, " =]", &pTok_Next);
						sscanf_s(pTok, "%d", &i32data);
						if((i32data == 0) || (i32data > 16))
							GetCfgError(format);
						pKey->u16CfgType = KEY_CHANNEL_NUM;
						pKey->i32CfgValue = i32data;
						uartpara[uart_index].channel_num = pKey->i32CfgValue;						
						pKey++;
						u32ResultNum++;

					}
					else if (_stricmp(pTok, "type") == 0)
					{
						pTok = strtok_s(nullptr, " =]", &pTok_Next);
						if (_stricmp(pTok, "serial") == 0)
						{
							pKey->u16CfgType = KEY_CHANNEL_TYPE;
							pKey->i32CfgValue = 0;
							//pKey++;
							//u32ResultNum++;
						}
						else if (_stricmp(pTok, "ethan") == 0)
						{
							pKey->u16CfgType = KEY_CHANNEL_TYPE;
							pKey->i32CfgValue = 1;
							//pKey++;
							//u32ResultNum++;
						}
						else
							GetCfgError(format);
						
						uartpara[uart_index].channel_type = pKey->i32CfgValue;
							pKey++;
							u32ResultNum++;						
					}
					else if (_stricmp(pTok, "protocol") == 0)
					{
						pTok = strtok_s(nullptr, " =]", &pTok_Next);
						if (_stricmp(pTok, "modbus") == 0)
						{
							pKey->u16CfgType = KEY_CHANNEL_PROTOCOL;
							pKey->i32CfgValue = 0;
							//pKey++;
							//u32ResultNum++;
						}
						else if (_stricmp(pTok, "iec103") == 0)
						{
							pKey->u16CfgType = KEY_CHANNEL_PROTOCOL;
							pKey->i32CfgValue = 1;
							//pKey++;
							//u32ResultNum++;
						}
						else if (_stricmp(pTok, "hfna2s") == 0)
						{
							pKey->u16CfgType = KEY_CHANNEL_PROTOCOL;
							pKey->i32CfgValue = 2;
							//pKey++;
							//u32ResultNum++;
						}
						else
							GetCfgError(format);

						uartpara[uart_index].uart_protocol = pKey->i32CfgValue;	
						pKey++;
						u32ResultNum++;						
					}
					else if (_stricmp(pTok, "interval") == 0)
					{
						pTok = strtok_s(nullptr, " =]", &pTok_Next);
						pKey->u16CfgType = KEY_CHANNEL_INTERVAL;
						sscanf_s(pTok, "%d", &(pKey->i32CfgValue));
						uartpara[uart_index].uart_read_time = pKey->i32CfgValue;						
						pKey++;
						u32ResultNum++;
						

					}
					else if (_stricmp(pTok, "timeout") == 0)
					{
						pTok = strtok_s(nullptr, " =]", &pTok_Next);
						pKey->u16CfgType = KEY_CHANNEL_TIMEOUT;
						sscanf_s(pTok, "%d", &(pKey->i32CfgValue));
						uartpara[uart_index].uart_receive_time = pKey->i32CfgValue;						
						pKey++;
						u32ResultNum++;
						

					}
					else if (_stricmp(pTok, "transpond") == 0)
					{
						pTok = strtok_s(nullptr, " =]", &pTok_Next);
						if (_stricmp(pTok, "no") == 0)
						{
							pKey->u16CfgType = KEY_CHANNEL_TRANSPOND;
							pKey->i32CfgValue = 0;
							//pKey++;
							//u32ResultNum++;
						}
						else if (_stricmp(pTok, "yes") == 0)
						{
							pKey->u16CfgType = KEY_CHANNEL_TRANSPOND;
							pKey->i32CfgValue = 1;
							//pKey++;
							//u32ResultNum++;
						}
						else
							GetCfgError(format);
						
						uartpara[uart_index].uart_transpond = pKey->i32CfgValue;
						pKey++;
						u32ResultNum++;							
					}
					else if (_stricmp(pTok, "serialport") == 0)
					{
						pTok = strtok_s(nullptr, " =]", &pTok_Next);
						pKey->i32CfgValue = -1;
						i32data = sscanf_s(pTok, "com%d", &(pKey->i32CfgValue));//这里需要先判断
						//i32data = sscanf_s(pTok, "%d", &(pKey->i32CfgValue));
						if((i32data == 0) || (i32data > SERIALPORT_NUM))
							GetCfgError(format);
						pKey->u16CfgType = KEY_CHANNEL_SERIALPORT;

						
						uartpara[uart_index].uart_port = i32data;
						pKey++;
						u32ResultNum++;						
					}
					else if (_stricmp(pTok, "serialbaudrate") == 0)
					{
						pTok = strtok_s(nullptr, " =]", &pTok_Next);
						pKey->u16CfgType = KEY_CHANNEL_SERIALBAUDRATE;
						sscanf_s(pTok, "%d", &(pKey->i32CfgValue));

						
						uartpara[uart_index].uart_baudrate = pKey->i32CfgValue;
						pKey++;
						u32ResultNum++;						
					}
					else if (_stricmp(pTok, "serialwidth") == 0)
					{
						pTok = strtok_s(nullptr, " =]", &pTok_Next);
						pKey->u16CfgType = KEY_CHANNEL_SERIALWIDTH;
						sscanf_s(pTok, "%d", &(pKey->i32CfgValue));

						
						uartpara[uart_index].uart_datawidth = pKey->i32CfgValue;
						pKey++;
						u32ResultNum++;						
					}
					else if (_stricmp(pTok, "serialstop") == 0)
					{
						pTok = strtok_s(nullptr, " =]", &pTok_Next);
						pKey->u16CfgType = KEY_CHANNEL_SERIALSTOP;
						sscanf_s(pTok, "%d", &(pKey->i32CfgValue));

						
						uartpara[uart_index].uart_stopbit = pKey->i32CfgValue;
						pKey++;
						u32ResultNum++;						
					}
					else if (_stricmp(pTok, "serialparity") == 0)
					{
						pTok = strtok_s(nullptr, " =]", &pTok_Next);
						if (_stricmp(pTok, "none") == 0)
						{
							pKey->u16CfgType = KEY_CHANNEL_SERIALPARITY;
							pKey->i32CfgValue = 0;
							//pKey++;
							//u32ResultNum++;
						}
						else if (_stricmp(pTok, "even") == 0)
						{
							pKey->u16CfgType = KEY_CHANNEL_SERIALPARITY;
							pKey->i32CfgValue = 1;
							//pKey++;
							//u32ResultNum++;
						}
						else if (_stricmp(pTok, "odd") == 0)
						{
							pKey->u16CfgType = KEY_CHANNEL_SERIALPARITY;
							pKey->i32CfgValue = 2;
							//pKey++;
							//u32ResultNum++;
						}
						else
							GetCfgError(format);
						
						uartpara[uart_index].uart_parity = pKey->i32CfgValue;
						pKey++;
						u32ResultNum++;							
					}
					else if (_stricmp(pTok, "serialtype") == 0)
					{
						pTok = strtok_s(nullptr, " =]", &pTok_Next);
						if (_stricmp(pTok, "485") == 0)
						{
							pKey->u16CfgType = KEY_CHANNEL_SERIALTYPE;
							pKey->i32CfgValue = 0;
							//pKey++;
							//u32ResultNum++;
						}
						else if (_stricmp(pTok, "422") == 0)
						{
							pKey->u16CfgType = KEY_CHANNEL_SERIALTYPE;
							pKey->i32CfgValue = 1;
							//pKey++;
							//u32ResultNum++;
						}
						else if (_stricmp(pTok, "232") == 0)
						{
							pKey->u16CfgType = KEY_CHANNEL_SERIALTYPE;
							pKey->i32CfgValue = 2;
							//pKey++;
							//u32ResultNum++;
						}
						else
							GetCfgError(format);
						
						uartpara[uart_index].uart_type = pKey->i32CfgValue;
						pKey++;
						u32ResultNum++;							
					}
					else if (_stricmp(pTok, "ip1") == 0)
					{
						pTok = strtok_s(nullptr, " =]", &pTok_Next);
						sscanf_s(pTok, "%3d.%3d.%3d.%3d", (int32_t *)&(ipaddr[3]),
							(int32_t *)&(ipaddr[2]),
							(int32_t *)&(ipaddr[1]),
							(int32_t *)&(ipaddr[0]));
						if((ipaddr[0] > 255) || (ipaddr[1] > 255) || (ipaddr[2] > 255) || (ipaddr[3] > 255))
							GetCfgError(format);
						pKey->u16CfgType = KEY_CHANNEL_IP1;
						pKey->i32CfgValue = (ipaddr[3] << 24) || (ipaddr[2] << 16) || (ipaddr[1] << 8) || ipaddr[0];
						pKey++;
						u32ResultNum++;
					}
					else if (_stricmp(pTok, "port1") == 0)
					{
						pTok = strtok_s(nullptr, " =]", &pTok_Next);
						pKey->u16CfgType = KEY_CHANNEL_PORT1;
						sscanf_s(pTok, "%d", &(pKey->i32CfgValue));
						pKey++;
						u32ResultNum++;
					}
					else if (_stricmp(pTok, "ip2") == 0)
					{
						pTok = strtok_s(nullptr, " =]", &pTok_Next);
						sscanf_s(pTok, "%3d.%3d.%3d.%3d", (int32_t *)&(ipaddr[3]),
							(int32_t *)&(ipaddr[2]),
							(int32_t *)&(ipaddr[1]),
							(int32_t *)&(ipaddr[0]));
						if ((ipaddr[0] > 255) || (ipaddr[1] > 255) || (ipaddr[2] > 255) || (ipaddr[3] > 255))
							GetCfgError(format);
						pKey->u16CfgType = KEY_CHANNEL_IP2;
						pKey->i32CfgValue = (ipaddr[3] << 24) || (ipaddr[2] << 16) || (ipaddr[1] << 8) || ipaddr[0];
						pKey++;
						u32ResultNum++;
					}
					else if (_stricmp(pTok, "port2") == 0)
					{
						pTok = strtok_s(nullptr, " =]", &pTok_Next);
						pKey->u16CfgType = KEY_CHANNEL_PORT2;
						sscanf_s(pTok, "%d", &(pKey->i32CfgValue));
						pKey++;
						u32ResultNum++;
					}
					else if(_stricmp(pTok,"devicenum") == 0)//read device num
					{
						pTok = strtok_s(nullptr,"=]",&pTok_Next);
						pKey->u16CfgType = KEY_CHANNEL_DEVICENUM;
						sscanf_s(pTok,"%d",&(pKey->i32CfgValue));
						uartpara[uart_index].devusednum = pKey->i32CfgValue;
						pKey++;
						u32ResultNum++;
			
					}
					else if(_stricmp(pTok,"deviceaddrlist") == 0)//read device addr list [DEVICEADDRLIST = 0102030405060708]
					{
						pTok = strtok_s(nullptr,"=]",&pTok_Next);
						pKey->u16CfgType = KEY_CHANNEL_DEVICELIST;
						sscanf_s(pTok,"%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x",
						(int32_t *)&tempbuf[0],(int32_t *)&tempbuf[1],(int32_t *)&tempbuf[2],(int32_t *)&tempbuf[3],(int32_t *)&tempbuf[4],(int32_t *)&tempbuf[5],(int32_t *)&tempbuf[6],(int32_t *)&tempbuf[7]
						,(int32_t *)&tempbuf[8],(int32_t *)&tempbuf[9],(int32_t *)&tempbuf[10],(int32_t *)&tempbuf[11],(int32_t *)&tempbuf[12],(int32_t *)&tempbuf[13],(int32_t *)&tempbuf[14],(int32_t *)&tempbuf[15]
						,(int32_t *)&tempbuf[16],(int32_t *)&tempbuf[17],(int32_t *)&tempbuf[18],(int32_t *)&tempbuf[19]);//&(pKey->i32CfgValue));
						for(i=0; i<uartpara[uart_index].devusednum; i++)
						{
							uartpara[uart_index].devaddr[i] = tempbuf[i];
						}
						pKey++;
						u32ResultNum++;
					}
					else
					{//无效属性 不解析

					}

					pTok = strtok_s(nullptr, " =]", &pTok_Next);
				} while (pTok != nullptr);

				
				break;
			case read_device:
				//读属性关键字
			  savetype  = 3;
				pTok = strtok_s((char *)&buf[posH], " =]", &pTok_Next);

				//第一个属性
				do
				{
					if (_stricmp(pTok, "parent") == 0)
					{
						pTok = strtok_s(nullptr, " =]", &pTok_Next);
						i32data = 0;
						sscanf_s(pTok, "CHANNEL%d", &i32data);
						if((i32data == 0) || (i32data > 16))
							GetCfgError(format);
						pKey->u16CfgType = KEY_DEVICE_PARENT;
						pKey->i32CfgValue = i32data;
						pKey++;
						u32ResultNum++;
					}
					else if (_stricmp(pTok, "addr") == 0)
					{
						pTok = strtok_s(nullptr, " =]", &pTok_Next);
						pKey->u16CfgType = KEY_DEVICE_ADDR;
						sscanf_s(pTok, "%d", &(pKey->i32CfgValue));
						pKey++;
						u32ResultNum++;
					}
					else if (_stricmp(pTok, "protocol") == 0)
					{
						pTok = strtok_s(nullptr, " =]", &pTok_Next);
						if (_stricmp(pTok, "modbus") == 0)
						{
							pKey->u16CfgType = KEY_DEVICE_PROTOCOL;
							pKey->i32CfgValue = 0;
							pKey++;
							u32ResultNum++;
						}
						else if (_stricmp(pTok, "iec103") == 0)
						{
							pKey->u16CfgType = KEY_DEVICE_PROTOCOL;
							pKey->i32CfgValue = 1;
							pKey++;
							u32ResultNum++;
						}
						else if (_stricmp(pTok, "hfna2s") == 0)
						{
							pKey->u16CfgType = KEY_DEVICE_PROTOCOL;
							pKey->i32CfgValue = 2;
							pKey++;
							u32ResultNum++;
						}
						else
							GetCfgError(format);
					}
					else if (_stricmp(pTok, "name") == 0)
					{
						pTok = strtok_s(nullptr, " =]", &pTok_Next);
						pKey->u16CfgType = KEY_DEVICE_NAME;
						pKey->i32CfgValue = -1;
#ifdef THROW_STRING
						throw_one_string(pTok);
						pKey->i32CfgValue = pHandle->throw_string_index - 1;
#endif
						pKey++;
						u32ResultNum++;
					}
					else if (_stricmp(pTok, "sectornum") == 0)
					{
						pTok = strtok_s(nullptr, " =]", &pTok_Next);
						pKey->u16CfgType = KEY_DEVICE_SECTORNUM;
						sscanf_s(pTok, "%d", &(pKey->i32CfgValue));
						pKey++;
						u32ResultNum++;
					}
					else if (_stricmp(pTok, "fun") == 0)
					{
						pTok = strtok_s(nullptr, " =]", &pTok_Next);
						pKey->u16CfgType = KEY_DEVICE_FUN;
						sscanf_s(pTok, "%d", &(pKey->i32CfgValue));
						pKey++;
						u32ResultNum++;
					}
					else if (_stricmp(pTok, "yxnum") == 0)
					{
						pTok = strtok_s(nullptr, " =]", &pTok_Next);
						pKey->u16CfgType = KEY_DEVICE_YXNUM;
						sscanf_s(pTok, "%d", &(pKey->i32CfgValue));
						pKey++;
						u32ResultNum++;
					}
					else if (_stricmp(pTok, "ycnum") == 0)
					{
						pTok = strtok_s(nullptr, " =]", &pTok_Next);
						pKey->u16CfgType = KEY_DEVICE_YCNUM;
						sscanf_s(pTok, "%d", &(pKey->i32CfgValue));
						pKey++;
						u32ResultNum++;
					}
					else if (_stricmp(pTok, "yknum") == 0)
					{
						pTok = strtok_s(nullptr, " =]", &pTok_Next);
						pKey->u16CfgType = KEY_DEVICE_YKNUM;
						sscanf_s(pTok, "%d", &(pKey->i32CfgValue));
						pKey++;
						u32ResultNum++;
					}
					else if (_stricmp(pTok, "ymnum") == 0)
					{
						pTok = strtok_s(nullptr, " =]", &pTok_Next);
						pKey->u16CfgType = KEY_DEVICE_YMNUM;
						sscanf_s(pTok, "%d", &(pKey->i32CfgValue));
						pKey++;
						u32ResultNum++;
					}
					else if (_stricmp(pTok, "sjnum") == 0)
					{
						pTok = strtok_s(nullptr, " =]", &pTok_Next);
						pKey->u16CfgType = KEY_DEVICE_SJNUM;
						sscanf_s(pTok, "%d", &(pKey->i32CfgValue));
						pKey++;
						u32ResultNum++;
					}
					else if (_stricmp(pTok, "infpoint") == 0)
					{
						pTok = strtok_s(nullptr, " =]", &pTok_Next);
						
						if (_stricmp(pTok, "yx") == 0)
						{
							pTok = strtok_s(nullptr, " =]", &pTok_Next);
							do
							{
								if (_stricmp(pTok, "#index") == 0)
								{
									pTok = strtok_s(nullptr, " =]", &pTok_Next);
									pKey->u16CfgType = KEY_DEVICE_INFPOINTCFG_YX_INDEX;
									sscanf_s(pTok, "%d", &(pKey->i32CfgValue));
									pKey++;
									u32ResultNum++;
								}
								else if (_stricmp(pTok, "#send") == 0)
								{
									pTok = strtok_s(nullptr, " =]", &pTok_Next);
									pKey->u16CfgType = KEY_DEVICE_INFPOINTCFG_YX_SEND;
									sscanf_s(pTok, "%4x", &(pKey->i32CfgValue));
									pKey++;
									u32ResultNum++;
								}
								else if (_stricmp(pTok, "#command") == 0)
								{
									pTok = strtok_s(nullptr, " =]", &pTok_Next);
									pKey->u16CfgType = KEY_DEVICE_INFPOINTCFG_YX_COMMAND;
									sscanf_s(pTok, "%d", &(pKey->i32CfgValue));
									pKey++;
									u32ResultNum++;
								}
								else if (_stricmp(pTok, "#addr") == 0)
								{
									pTok = strtok_s(nullptr, " =]", &pTok_Next);
									pKey->u16CfgType = KEY_DEVICE_INFPOINTCFG_YX_ADDR;
									sscanf_s(pTok, "0x%x", &(pKey->i32CfgValue));
									pKey++;
									u32ResultNum++;
								}
								else if (_stricmp(pTok, "#name") == 0)
								{
									pTok = strtok_s(nullptr, " =]", &pTok_Next);
									pKey->u16CfgType = KEY_DEVICE_INFPOINTCFG_YX_NAME;
									pKey->i32CfgValue = -1;
#ifdef THROW_STRING
									throw_one_string(pTok);
									pKey->i32CfgValue = pHandle->throw_string_index - 1;
#endif
									pKey++;
									u32ResultNum++;
								}
								else if (_stricmp(pTok, "#sector") == 0)
								{
									pTok = strtok_s(nullptr, " =]", &pTok_Next);
									pKey->u16CfgType = KEY_DEVICE_INFPOINTCFG_YX_SECTOR;
									sscanf_s(pTok, "%d", &(pKey->i32CfgValue));
									pKey++;
									u32ResultNum++;
								}
								else if (_stricmp(pTok, "#inf") == 0)
								{
									pTok = strtok_s(nullptr, " =]", &pTok_Next);
									pKey->u16CfgType = KEY_DEVICE_INFPOINTCFG_YX_INF;
									sscanf_s(pTok, "%d", &(pKey->i32CfgValue));
									pKey++;
									u32ResultNum++;
								}
								else if (_stricmp(pTok, "#fun") == 0)
								{
									pTok = strtok_s(nullptr, " =]", &pTok_Next);
									pKey->u16CfgType = KEY_DEVICE_INFPOINTCFG_YX_FUN;
									sscanf_s(pTok, "%d", &(pKey->i32CfgValue));
									pKey++;
									u32ResultNum++;
								}
								else if (_stricmp(pTok, "#asdu") == 0)
								{
									pTok = strtok_s(nullptr, " =]", &pTok_Next);
									pKey->u16CfgType = KEY_DEVICE_INFPOINTCFG_YX_ASDU;
									sscanf_s(pTok, "%d", &(pKey->i32CfgValue));
									pKey++;
									u32ResultNum++;
								}
								else if (_stricmp(pTok, "#level") == 0)
								{
									pTok = strtok_s(nullptr, " =]", &pTok_Next);
									if (_stricmp(pTok, "normal") == 0)
									{
										pKey->u16CfgType = KEY_DEVICE_INFPOINTCFG_YX_LEVEL;
										pKey->i32CfgValue = 0;
										pKey++;
										u32ResultNum++;
									}
									else if (_stricmp(pTok, "alarm") == 0)
									{
										pKey->u16CfgType = KEY_DEVICE_INFPOINTCFG_YX_LEVEL;
										pKey->i32CfgValue = 1;
										pKey++;
										u32ResultNum++;
									}
									else if (_stricmp(pTok, "accident") == 0)
									{
										pKey->u16CfgType = KEY_DEVICE_INFPOINTCFG_YX_LEVEL;
										pKey->i32CfgValue = 2;
										pKey++;
										u32ResultNum++;
									}
									else
										GetCfgError(format);
								}
								else if (_stricmp(pTok, "#function") == 0)
								{
									pTok = strtok_s(nullptr, " =]", &pTok_Next);
									if (_stricmp(pTok, "set") == 0)
									{
										pKey->u16CfgType = KEY_DEVICE_INFPOINTCFG_YX_FUNCTION;
										pKey->i32CfgValue = 0;
										pKey++;
										u32ResultNum++;
									}
									else if (_stricmp(pTok, "reset") == 0)
									{
										pKey->u16CfgType = KEY_DEVICE_INFPOINTCFG_YX_FUNCTION;
										pKey->i32CfgValue = 1;
										pKey++;
										u32ResultNum++;
									}
									else if (_stricmp(pTok, "all") == 0)
									{
										pKey->u16CfgType = KEY_DEVICE_INFPOINTCFG_YX_FUNCTION;
										pKey->i32CfgValue = 2;
										pKey++;
										u32ResultNum++;
									}
									else
										GetCfgError(format);
								}
								else
								{
									//无效的关键字
								}

								pTok = strtok_s(nullptr, " =]", &pTok_Next);
							} while (pTok != nullptr);
						}
						else if (_stricmp(pTok, "yc") == 0)
						{
							pTok = strtok_s(nullptr, " =]", &pTok_Next);
							do
							{
								if (_stricmp(pTok, "#index") == 0)
								{
									pTok = strtok_s(nullptr, " =]", &pTok_Next);
									pKey->u16CfgType = KEY_DEVICE_INFPOINTCFG_YC_INDEX;
									sscanf_s(pTok, "%d", &(pKey->i32CfgValue));
									pKey++;
									u32ResultNum++;
								}
								else if (_stricmp(pTok, "#send") == 0)
								{
									pTok = strtok_s(nullptr, " =]", &pTok_Next);
									pKey->u16CfgType = KEY_DEVICE_INFPOINTCFG_YC_SEND;
									sscanf_s(pTok, "%4x", &(pKey->i32CfgValue));
									pKey++;
									u32ResultNum++;
								}
								else if (_stricmp(pTok, "#command") == 0)
								{
									pTok = strtok_s(nullptr, " =]", &pTok_Next);
									pKey->u16CfgType = KEY_DEVICE_INFPOINTCFG_YC_COMMAND;
									sscanf_s(pTok, "%d", &(pKey->i32CfgValue));
									pKey++;
									u32ResultNum++;
								}
								else if (_stricmp(pTok, "#addr") == 0)
								{
									pTok = strtok_s(nullptr, " =]", &pTok_Next);
									pKey->u16CfgType = KEY_DEVICE_INFPOINTCFG_YC_ADDR;
									sscanf_s(pTok, "0x%x", &(pKey->i32CfgValue));
									pKey++;
									u32ResultNum++;
								}
								else if (_stricmp(pTok, "#name") == 0)
								{
									pTok = strtok_s(nullptr, " =]", &pTok_Next);
									pKey->u16CfgType = KEY_DEVICE_INFPOINTCFG_YC_NAME;
									pKey->i32CfgValue = -1;
#ifdef THROW_STRING
									throw_one_string(pTok);
									pKey->i32CfgValue = pHandle->throw_string_index - 1;
#endif
									pKey++;
									u32ResultNum++;
								}
								else if (_stricmp(pTok, "#sector") == 0)
								{
									pTok = strtok_s(nullptr, " =]", &pTok_Next);
									pKey->u16CfgType = KEY_DEVICE_INFPOINTCFG_YC_SECTOR;
									sscanf_s(pTok, "%d", &(pKey->i32CfgValue));
									pKey++;
									u32ResultNum++;
								}
								else if (_stricmp(pTok, "#inf") == 0)
								{
									pTok = strtok_s(nullptr, " =]", &pTok_Next);
									pKey->u16CfgType = KEY_DEVICE_INFPOINTCFG_YC_INF;
									sscanf_s(pTok, "%d", &(pKey->i32CfgValue));
									pKey++;
									u32ResultNum++;
								}
								else if (_stricmp(pTok, "#fun") == 0)
								{
									pTok = strtok_s(nullptr, " =]", &pTok_Next);
									pKey->u16CfgType = KEY_DEVICE_INFPOINTCFG_YC_FUN;
									sscanf_s(pTok, "%d", &(pKey->i32CfgValue));
									pKey++;
									u32ResultNum++;
								}
								else if (_stricmp(pTok, "#asdu") == 0)
								{
									pTok = strtok_s(nullptr, " =]", &pTok_Next);
									pKey->u16CfgType = KEY_DEVICE_INFPOINTCFG_YC_ASDU;
									sscanf_s(pTok, "%d", &(pKey->i32CfgValue));
									pKey++;
									u32ResultNum++;
								}
								else if (_stricmp(pTok, "#gain") == 0)
								{
									pTok = strtok_s(nullptr, " =]", &pTok_Next);
									pKey->u16CfgType = KEY_DEVICE_INFPOINTCFG_YC_GAIN;
									sscanf_s(pTok, "%d", &(pKey->i32CfgValue));
									pKey++;
									u32ResultNum++;
								}
								else if (_stricmp(pTok, "#slim") == 0)
								{
									pTok = strtok_s(nullptr, " =]", &pTok_Next);
									pKey->u16CfgType = KEY_DEVICE_INFPOINTCFG_YC_SLIM;
									sscanf_s(pTok, "%d", &(pKey->i32CfgValue));
									pKey++;
									u32ResultNum++;
								}
								else if (_stricmp(pTok, "#shift") == 0)
								{
									pTok = strtok_s(nullptr, " =]", &pTok_Next);
									if (_stricmp(pTok, "no") == 0)
									{
										pKey->u16CfgType = KEY_DEVICE_INFPOINTCFG_YC_SHIFT;
										pKey->i32CfgValue = 0;
										pKey++;
										u32ResultNum++;
									}
									else if (_stricmp(pTok, "yes") == 0)
									{
										pKey->u16CfgType = KEY_DEVICE_INFPOINTCFG_YC_SHIFT;
										pKey->i32CfgValue = 1;
										pKey++;
										u32ResultNum++;
									}
									else
										GetCfgError(format);
								}
								else if (_stricmp(pTok, "#offset") == 0)
								{
									pTok = strtok_s(nullptr, " =]", &pTok_Next);
									pKey->u16CfgType = KEY_DEVICE_INFPOINTCFG_YC_OFFSET;
									sscanf_s(pTok, "%d", &(pKey->i32CfgValue));
									pKey++;
									u32ResultNum++;
								}
								else if (_stricmp(pTok, "#range") == 0)
								{
									pTok = strtok_s(nullptr, " =]", &pTok_Next);
									pKey->u16CfgType = KEY_DEVICE_INFPOINTCFG_YC_RANGE;
									sscanf_s(pTok, "%d", &(pKey->i32CfgValue));
									pKey++;
									u32ResultNum++;
								}
								else
								{
									//无效的关键字
								}

								pTok = strtok_s(nullptr, " =]", &pTok_Next);
							} while (pTok != nullptr);
						}
						else if (_stricmp(pTok, "yk") == 0)
						{
							pTok = strtok_s(nullptr, " =]", &pTok_Next);
							do
							{
								if (_stricmp(pTok, "#index") == 0)
								{
									pTok = strtok_s(nullptr, " =]", &pTok_Next);
									pKey->u16CfgType = KEY_DEVICE_INFPOINTCFG_YK_INDEX;
									sscanf_s(pTok, "%d", &(pKey->i32CfgValue));
									pKey++;
									u32ResultNum++;
								}
								else if (_stricmp(pTok, "#send") == 0)
								{
									pTok = strtok_s(nullptr, " =]", &pTok_Next);
									pKey->u16CfgType = KEY_DEVICE_INFPOINTCFG_YK_SEND;
									sscanf_s(pTok, "%4x", &(pKey->i32CfgValue));
									pKey++;
									u32ResultNum++;
								}
								else if (_stricmp(pTok, "#command") == 0)
								{
									pTok = strtok_s(nullptr, " =]", &pTok_Next);
									pKey->u16CfgType = KEY_DEVICE_INFPOINTCFG_YK_COMMAND;
									sscanf_s(pTok, "%d", &(pKey->i32CfgValue));
									pKey++;
									u32ResultNum++;
								}
								else if (_stricmp(pTok, "#addr") == 0)
								{
									pTok = strtok_s(nullptr, " =]", &pTok_Next);
									pKey->u16CfgType = KEY_DEVICE_INFPOINTCFG_YK_ADDR;
									sscanf_s(pTok, "0x%x", &(pKey->i32CfgValue));
									pKey++;
									u32ResultNum++;
								}
								else if (_stricmp(pTok, "#name") == 0)
								{
									pTok = strtok_s(nullptr, " =]", &pTok_Next);
									pKey->u16CfgType = KEY_DEVICE_INFPOINTCFG_YK_NAME;
									pKey->i32CfgValue = -1;
#ifdef THROW_STRING
									throw_one_string(pTok);
									pKey->i32CfgValue = pHandle->throw_string_index - 1;
#endif
									pKey++;
									u32ResultNum++;
								}
								else if (_stricmp(pTok, "#sector") == 0)
								{
									pTok = strtok_s(nullptr, " =]", &pTok_Next);
									pKey->u16CfgType = KEY_DEVICE_INFPOINTCFG_YK_SECTOR;
									sscanf_s(pTok, "%d", &(pKey->i32CfgValue));
									pKey++;
									u32ResultNum++;
								}
								else if (_stricmp(pTok, "#inf") == 0)
								{
									pTok = strtok_s(nullptr, " =]", &pTok_Next);
									pKey->u16CfgType = KEY_DEVICE_INFPOINTCFG_YK_INF;
									sscanf_s(pTok, "%d", &(pKey->i32CfgValue));
									pKey++;
									u32ResultNum++;
								}
								else if (_stricmp(pTok, "#fun") == 0)
								{
									pTok = strtok_s(nullptr, " =]", &pTok_Next);
									pKey->u16CfgType = KEY_DEVICE_INFPOINTCFG_YK_FUN;
									sscanf_s(pTok, "%d", &(pKey->i32CfgValue));
									pKey++;
									u32ResultNum++;
								}
								else if (_stricmp(pTok, "#asdu") == 0)
								{
									pTok = strtok_s(nullptr, " =]", &pTok_Next);
									pKey->u16CfgType = KEY_DEVICE_INFPOINTCFG_YK_ASDU;
									sscanf_s(pTok, "%d", &(pKey->i32CfgValue));
									pKey++;
									u32ResultNum++;
								}
								else if (_stricmp(pTok, "#direct") == 0)
								{
									pTok = strtok_s(nullptr, " =]", &pTok_Next);
									if (_stricmp(pTok, "no") == 0)
									{
										pKey->u16CfgType = KEY_DEVICE_INFPOINTCFG_YK_DIRECT;
										pKey->i32CfgValue = 0;
										pKey++;
										u32ResultNum++;
									}
									else if (_stricmp(pTok, "yes") == 0)
									{
										pKey->u16CfgType = KEY_DEVICE_INFPOINTCFG_YK_DIRECT;
										pKey->i32CfgValue = 1;
										pKey++;
										u32ResultNum++;
									}
									else
										GetCfgError(format);
								}
								else if (_stricmp(pTok, "#return") == 0)
								{
									pTok = strtok_s(nullptr, " =]", &pTok_Next);
									if (_stricmp(pTok, "no") == 0)
									{
										pKey->u16CfgType = KEY_DEVICE_INFPOINTCFG_YK_RETURN;
										pKey->i32CfgValue = 0;
										pKey++;
										u32ResultNum++;
									}
									else if (_stricmp(pTok, "yes") == 0)
									{
										pKey->u16CfgType = KEY_DEVICE_INFPOINTCFG_YK_RETURN;
										pKey->i32CfgValue = 1;
										pKey++;
										u32ResultNum++;
									}
									else
										GetCfgError(format);
								}
								else if (_stricmp(pTok, "#totalreturn") == 0)
								{
									pTok = strtok_s(nullptr, " =]", &pTok_Next);
									if (_stricmp(pTok, "no") == 0)
									{
										pKey->u16CfgType = KEY_DEVICE_INFPOINTCFG_YK_TOTALRETURN;
										pKey->i32CfgValue = 0;
										pKey++;
										u32ResultNum++;
									}
									else if (_stricmp(pTok, "yes") == 0)
									{
										pKey->u16CfgType = KEY_DEVICE_INFPOINTCFG_YK_TOTALRETURN;
										pKey->i32CfgValue = 1;
										pKey++;
										u32ResultNum++;
									}
									else
										GetCfgError(format);
								}
								else
								{
									//无效的关键字
								}

								pTok = strtok_s(nullptr, " =]", &pTok_Next);
							} while (pTok != nullptr);
						}
						else if (_stricmp(pTok, "ym") == 0)
						{
							pTok = strtok_s(nullptr, " =]", &pTok_Next);
							do
							{
								if (_stricmp(pTok, "#index") == 0)
								{
									pTok = strtok_s(nullptr, " =]", &pTok_Next);
									pKey->u16CfgType = KEY_DEVICE_INFPOINTCFG_YM_INDEX;
									sscanf_s(pTok, "%d", &(pKey->i32CfgValue));
									pKey++;
									u32ResultNum++;
								}
								else if (_stricmp(pTok, "#send") == 0)
								{
									pTok = strtok_s(nullptr, " =]", &pTok_Next);
									pKey->u16CfgType = KEY_DEVICE_INFPOINTCFG_YM_SEND;
									sscanf_s(pTok, "%4x", &(pKey->i32CfgValue));
									pKey++;
									u32ResultNum++;
								}
								else if (_stricmp(pTok, "#command") == 0)
								{
									pTok = strtok_s(nullptr, " =]", &pTok_Next);
									pKey->u16CfgType = KEY_DEVICE_INFPOINTCFG_YM_COMMAND;
									sscanf_s(pTok, "%d", &(pKey->i32CfgValue));
									pKey++;
									u32ResultNum++;
								}
								else if (_stricmp(pTok, "#addr") == 0)
								{
									pTok = strtok_s(nullptr, " =]", &pTok_Next);
									pKey->u16CfgType = KEY_DEVICE_INFPOINTCFG_YM_ADDR;
									sscanf_s(pTok, "0x%x", &(pKey->i32CfgValue));
									pKey++;
									u32ResultNum++;
								}
								else if (_stricmp(pTok, "#name") == 0)
								{
									pTok = strtok_s(nullptr, " =]", &pTok_Next);
									pKey->u16CfgType = KEY_DEVICE_INFPOINTCFG_YM_NAME;
									pKey->i32CfgValue = -1;
#ifdef THROW_STRING
									throw_one_string(pTok);
									pKey->i32CfgValue = pHandle->throw_string_index - 1;
#endif
									pKey++;
									u32ResultNum++;
								}
								else if (_stricmp(pTok, "#sector") == 0)
								{
									pTok = strtok_s(nullptr, " =]", &pTok_Next);
									pKey->u16CfgType = KEY_DEVICE_INFPOINTCFG_YM_SECTOR;
									sscanf_s(pTok, "%d", &(pKey->i32CfgValue));
									pKey++;
									u32ResultNum++;
								}
								else if (_stricmp(pTok, "#inf") == 0)
								{
									pTok = strtok_s(nullptr, " =]", &pTok_Next);
									pKey->u16CfgType = KEY_DEVICE_INFPOINTCFG_YM_INF;
									sscanf_s(pTok, "%d", &(pKey->i32CfgValue));
									pKey++;
									u32ResultNum++;
								}
								else if (_stricmp(pTok, "#fun") == 0)
								{
									pTok = strtok_s(nullptr, " =]", &pTok_Next);
									pKey->u16CfgType = KEY_DEVICE_INFPOINTCFG_YM_FUN;
									sscanf_s(pTok, "%d", &(pKey->i32CfgValue));
									pKey++;
									u32ResultNum++;
								}
								else if (_stricmp(pTok, "#asdu") == 0)
								{
									pTok = strtok_s(nullptr, " =]", &pTok_Next);
									pKey->u16CfgType = KEY_DEVICE_INFPOINTCFG_YM_ASDU;
									sscanf_s(pTok, "%d", &(pKey->i32CfgValue));
									pKey++;
									u32ResultNum++;
								}
								else
								{
									//无效的关键字
								}

								pTok = strtok_s(nullptr, " =]", &pTok_Next);
							} while (pTok != nullptr);
						}
						else if (_stricmp(pTok, "sj") == 0)
						{
							pTok = strtok_s(nullptr, " =]", &pTok_Next);
							do
							{
								if (_stricmp(pTok, "#index") == 0)
								{
									pTok = strtok_s(nullptr, " =]", &pTok_Next);
									pKey->u16CfgType = KEY_DEVICE_INFPOINTCFG_SJ_INDEX;
									sscanf_s(pTok, "%d", &(pKey->i32CfgValue));
									pKey++;
									u32ResultNum++;
								}
								else if (_stricmp(pTok, "#send") == 0)
								{
									pTok = strtok_s(nullptr, " =]", &pTok_Next);
									pKey->u16CfgType = KEY_DEVICE_INFPOINTCFG_SJ_SEND;
									sscanf_s(pTok, "%4x", &(pKey->i32CfgValue));
									pKey++;
									u32ResultNum++;
								}
								else if (_stricmp(pTok, "#command") == 0)
								{
									pTok = strtok_s(nullptr, " =]", &pTok_Next);
									pKey->u16CfgType = KEY_DEVICE_INFPOINTCFG_SJ_COMMAND;
									sscanf_s(pTok, "%d", &(pKey->i32CfgValue));
									pKey++;
									u32ResultNum++;
								}
								else if (_stricmp(pTok, "#addr") == 0)
								{
									pTok = strtok_s(nullptr, " =]", &pTok_Next);
									pKey->u16CfgType = KEY_DEVICE_INFPOINTCFG_SJ_ADDR;
									sscanf_s(pTok, "0x%x", &(pKey->i32CfgValue));
									pKey++;
									u32ResultNum++;
								}
								else if (_stricmp(pTok, "#name") == 0)
								{
									pTok = strtok_s(nullptr, " =]", &pTok_Next);
									pKey->u16CfgType = KEY_DEVICE_INFPOINTCFG_SJ_NAME;
									pKey->i32CfgValue = -1;
#ifdef THROW_STRING
									throw_one_string(pTok);
									pKey->i32CfgValue = pHandle->throw_string_index - 1;
#endif
									pKey++;
									u32ResultNum++;
								}
								else if (_stricmp(pTok, "#sector") == 0)
								{
									pTok = strtok_s(nullptr, " =]", &pTok_Next);
									pKey->u16CfgType = KEY_DEVICE_INFPOINTCFG_SJ_SECTOR;
									sscanf_s(pTok, "%d", &(pKey->i32CfgValue));
									pKey++;
									u32ResultNum++;
								}
								else if (_stricmp(pTok, "#inf") == 0)
								{
									pTok = strtok_s(nullptr, " =]", &pTok_Next);
									pKey->u16CfgType = KEY_DEVICE_INFPOINTCFG_SJ_INF;
									sscanf_s(pTok, "%d", &(pKey->i32CfgValue));
									pKey++;
									u32ResultNum++;
								}
								else if (_stricmp(pTok, "#fun") == 0)
								{
									pTok = strtok_s(nullptr, " =]", &pTok_Next);
									pKey->u16CfgType = KEY_DEVICE_INFPOINTCFG_SJ_FUN;
									sscanf_s(pTok, "%d", &(pKey->i32CfgValue));
									pKey++;
									u32ResultNum++;
								}
								else if (_stricmp(pTok, "#asdu") == 0)
								{
									pTok = strtok_s(nullptr, " =]", &pTok_Next);
									pKey->u16CfgType = KEY_DEVICE_INFPOINTCFG_SJ_ASDU;
									sscanf_s(pTok, "%d", &(pKey->i32CfgValue));
									pKey++;
									u32ResultNum++;
								}
								else if (_stricmp(pTok, "#level") == 0)
								{
									pTok = strtok_s(nullptr, " =]", &pTok_Next);
									if (_stricmp(pTok, "normal") == 0)
									{
										pKey->u16CfgType = KEY_DEVICE_INFPOINTCFG_SJ_LEVEL;
										pKey->i32CfgValue = 0;
										pKey++;
										u32ResultNum++;
									}
									else if (_stricmp(pTok, "alarm") == 0)
									{
										pKey->u16CfgType = KEY_DEVICE_INFPOINTCFG_SJ_LEVEL;
										pKey->i32CfgValue = 1;
										pKey++;
										u32ResultNum++;
									}
									else if (_stricmp(pTok, "accident") == 0)
									{
										pKey->u16CfgType = KEY_DEVICE_INFPOINTCFG_SJ_LEVEL;
										pKey->i32CfgValue = 2;
										pKey++;
										u32ResultNum++;
									}
									else
										GetCfgError(format);
								}
								else if (_stricmp(pTok, "#function") == 0)
								{
									pTok = strtok_s(nullptr, " =]", &pTok_Next);
									if (_stricmp(pTok, "set") == 0)
									{
										pKey->u16CfgType = KEY_DEVICE_INFPOINTCFG_SJ_FUNCTION;
										pKey->i32CfgValue = 0;
										pKey++;
										u32ResultNum++;
									}
									else if (_stricmp(pTok, "reset") == 0)
									{
										pKey->u16CfgType = KEY_DEVICE_INFPOINTCFG_SJ_FUNCTION;
										pKey->i32CfgValue = 1;
										pKey++;
										u32ResultNum++;
									}
									else if (_stricmp(pTok, "all") == 0)
									{
										pKey->u16CfgType = KEY_DEVICE_INFPOINTCFG_SJ_FUNCTION;
										pKey->i32CfgValue = 2;
										pKey++;
										u32ResultNum++;
									}
									else
										GetCfgError(format);
								}
								else
								{
									//无效的关键字
								}

								pTok = strtok_s(nullptr, " =]", &pTok_Next);
							} while (pTok != nullptr);
						}
						else
							GetCfgError(format);

						//这里已经使用strtor_s将本周期的语句解析完了，就不经过外循环的do-while了
						break;
					}
					else
					{//无效属性 不解析

					}

					pTok = strtok_s(nullptr, " =]", &pTok_Next);
				} while (pTok != nullptr);
				break;
			case read_upstream:
				//读属性关键字
			  savetype  = 4;
				pTok = strtok_s((char *)&buf[posH], " =]", &pTok_Next);

				//第一个属性
				do
				{
					if (_stricmp(pTok, "index") == 0)
					{
						pTok = strtok_s(nullptr, " =]", &pTok_Next);
						pKey->u16CfgType = KEY_UPSTREAM_INDEX;
						sscanf_s(pTok, "%d", &(pKey->i32CfgValue));
						ethpara.eth_index = pKey->i32CfgValue;
						pKey++;
						u32ResultNum++;
					}
					else if (_stricmp(pTok, "num") == 0)
					{
						pTok = strtok_s(nullptr, " =]", &pTok_Next);
						sscanf_s(pTok, "%d", &i32data);
						if ((i32data == 0) || (i32data > 16))
							GetCfgError(format);
						pKey->u16CfgType = KEY_UPSTREAM_NUM;
						pKey->i32CfgValue = i32data;
						ethpara.eth_num = pKey->i32CfgValue;
						pKey++;
						u32ResultNum++;
					}
					else if (_stricmp(pTok, "type") == 0)
					{
						pTok = strtok_s(nullptr, " =]", &pTok_Next);
						if (_stricmp(pTok, "serial") == 0)
						{
							pKey->u16CfgType = KEY_UPSTREAM_TYPE;
							pKey->i32CfgValue = 0;
							//pKey++;
							//u32ResultNum++;
						}
						else if (_stricmp(pTok, "ethan") == 0)
						{
							pKey->u16CfgType = KEY_UPSTREAM_TYPE;
							pKey->i32CfgValue = 1;
							//pKey++;
							//u32ResultNum++;
						}
						else
							GetCfgError(format);
						  ethpara.eth_type  = pKey->i32CfgValue;
							pKey++;
							u32ResultNum++;						
						
					}
					else if (_stricmp(pTok, "protocol") == 0)
					{
						pTok = strtok_s(nullptr, " =]", &pTok_Next);
						if (_stricmp(pTok, "modbus") == 0)
						{
							pKey->u16CfgType = KEY_UPSTREAM_PROTOCOL;
							pKey->i32CfgValue = 0;
							//pKey++;
							//u32ResultNum++;
						}
						else if (_stricmp(pTok, "iec103") == 0)
						{
							pKey->u16CfgType = KEY_UPSTREAM_PROTOCOL;
							pKey->i32CfgValue = 1;
							//pKey++;
							//u32ResultNum++;
						}
						else if (_stricmp(pTok, "iec104") == 0)
						{
							pKey->u16CfgType = KEY_UPSTREAM_PROTOCOL;
							pKey->i32CfgValue = 2;
							//pKey++;
							//u32ResultNum++;
						}
						else
							GetCfgError(format);
						  ethpara.eth_protocol = pKey->i32CfgValue;
							pKey++;
							u32ResultNum++;						
					}
					else if (_stricmp(pTok, "daulnet") == 0)
					{
						pTok = strtok_s(nullptr, " =]", &pTok_Next);
						if (_stricmp(pTok, "no") == 0)
						{
							pKey->u16CfgType = KEY_UPSTREAM_DAULNET;
							pKey->i32CfgValue = 0;
							//pKey++;
							//u32ResultNum++;
						}
						else if (_stricmp(pTok, "yes") == 0)
						{
							pKey->u16CfgType = KEY_UPSTREAM_DAULNET;
							pKey->i32CfgValue = 1;
							//pKey++;
							//u32ResultNum++;
						}
						else
							GetCfgError(format);
              ethpara.eth_daulnet = pKey->i32CfgValue;						
							pKey++;
							u32ResultNum++;						
					}
					else if (_stricmp(pTok, "ip1") == 0)
					{
						pTok = strtok_s(nullptr, " =]", &pTok_Next);
						sscanf_s(pTok, "%3d.%3d.%3d.%3d", (int32_t *)&(ipaddr[3]),
							(int32_t *)&(ipaddr[2]),
							(int32_t *)&(ipaddr[1]),
							(int32_t *)&(ipaddr[0]));
						if ((ipaddr[0] > 255) || (ipaddr[1] > 255) || (ipaddr[2] > 255) || (ipaddr[3] > 255))
							GetCfgError(format);
						pKey->u16CfgType = KEY_UPSTREAM_IP1;
						pKey->i32CfgValue = ((uint32_t)ipaddr[3] << 24) | ((uint32_t)ipaddr[2] << 16) | ((uint32_t)ipaddr[1] << 8) | (uint32_t)ipaddr[0];
						ethpara.eth_IP1  = pKey->i32CfgValue;
						pKey++;
						u32ResultNum++;
					}
					else if (_stricmp(pTok, "port1") == 0)
					{
						pTok = strtok_s(nullptr, " =]", &pTok_Next);
						pKey->u16CfgType = KEY_UPSTREAM_PORT1;
						sscanf_s(pTok, "%d", &(pKey->i32CfgValue));
						ethpara.eth_port  = pKey->i32CfgValue;
						pKey++;
						u32ResultNum++;
					}
					else if (_stricmp(pTok, "ip2") == 0)
					{
						pTok = strtok_s(nullptr, " =]", &pTok_Next);
						sscanf_s(pTok, "%3d.%3d.%3d.%3d", (int32_t *)&(ipaddr[3]),
							(int32_t *)&(ipaddr[2]),
							(int32_t *)&(ipaddr[1]),
							(int32_t *)&(ipaddr[0]));
						if ((ipaddr[0] > 255) || (ipaddr[1] > 255) || (ipaddr[2] > 255) || (ipaddr[3] > 255))
							GetCfgError(format);
						pKey->u16CfgType = KEY_UPSTREAM_IP2;
						pKey->i32CfgValue = ((uint32_t)ipaddr[3] << 24) | ((uint32_t)ipaddr[2] << 16) | ((uint32_t)ipaddr[1] << 8) | (uint32_t)ipaddr[0];
						pKey++;
						u32ResultNum++;
					}
					else if (_stricmp(pTok, "port2") == 0)
					{
						pTok = strtok_s(nullptr, " =]", &pTok_Next);
						pKey->u16CfgType = KEY_UPSTREAM_PORT2;
						sscanf_s(pTok, "%d", &(pKey->i32CfgValue));
						pKey++;
						u32ResultNum++;
					}
					else if (_stricmp(pTok, "recvtimeout") == 0)
					{
						pTok = strtok_s(nullptr, " =]", &pTok_Next);
						pKey->u16CfgType = KEY_UPSTREAM_RECVTIMEOUT;
						sscanf_s(pTok, "%d", &(pKey->i32CfgValue));
						ethpara.eth_recv_time = pKey->i32CfgValue;
						pKey++;
						u32ResultNum++;
					}
					else if (_stricmp(pTok, "sendtimeout") == 0)
					{
						pTok = strtok_s(nullptr, " =]", &pTok_Next);
						pKey->u16CfgType = KEY_UPSTREAM_SENDTIMEOUT;
						sscanf_s(pTok, "%d", &(pKey->i32CfgValue));
						ethpara.eth_send_time  = pKey->i32CfgValue;
						pKey++;
						u32ResultNum++;
					}
					else if (_stricmp(pTok, "ycsendcycle") == 0)
					{
						pTok = strtok_s(nullptr, " =]", &pTok_Next);
						pKey->u16CfgType = KEY_UPSTREAM_YCSENDCYCLE;
						sscanf_s(pTok, "%d", &(pKey->i32CfgValue));
						ethpara.eth_yctrans_time  = pKey->i32CfgValue;
						pKey++;
						u32ResultNum++;
					}
					else if (_stricmp(pTok, "soeenable") == 0)
					{
						pTok = strtok_s(nullptr, " =]", &pTok_Next);
						if (_stricmp(pTok, "no") == 0)
						{
							pKey->u16CfgType = KEY_UPSTREAM_SOEENABLE;
							pKey->i32CfgValue = 0;
							//pKey++;
							//u32ResultNum++;
						}
						else if (_stricmp(pTok, "yes") == 0)
						{
							pKey->u16CfgType = KEY_UPSTREAM_SOEENABLE;
							pKey->i32CfgValue = 1;
							//pKey++;
							//u32ResultNum++;
						}
						else
							GetCfgError(format);
						ethpara.eth_trans_soe = pKey->i32CfgValue;	
						pKey++;
						u32ResultNum++;						
						
					}
					else if (_stricmp(pTok, "soeresetenable") == 0)
					{
						pTok = strtok_s(nullptr, " =]", &pTok_Next);
						if (_stricmp(pTok, "no") == 0)
						{
							pKey->u16CfgType = KEY_UPSTREAM_SOERESETENABLE;
							pKey->i32CfgValue = 0;
							//pKey++;
							//u32ResultNum++;
						}
						else if (_stricmp(pTok, "yes") == 0)
						{
							pKey->u16CfgType = KEY_UPSTREAM_SOERESETENABLE;
							pKey->i32CfgValue = 1;
							//pKey++;
							//u32ResultNum++;
						}
						else
							GetCfgError(format);
						ethpara.eth_trans_soefg  = pKey->i32CfgValue;
						pKey++;
						u32ResultNum++;						
						
					}
					else if (_stricmp(pTok, "ycsendflag") == 0)
					{
						pTok = strtok_s(nullptr, " =]", &pTok_Next);
						if (_stricmp(pTok, "int16") == 0)
						{
							pKey->u16CfgType = KEY_UPSTREAM_YCSENDFLAG;
							pKey->i32CfgValue = 0;
							//pKey++;
							//u32ResultNum++;
						}
						else if (_stricmp(pTok, "int16max") == 0)
						{
							pKey->u16CfgType = KEY_UPSTREAM_YCSENDFLAG;
							pKey->i32CfgValue = 1;
							//pKey++;
							//u32ResultNum++;
						}
						else if (_stricmp(pTok, "float32") == 0)
						{
							pKey->u16CfgType = KEY_UPSTREAM_YCSENDFLAG;
							pKey->i32CfgValue = 2;
							//pKey++;
							//u32ResultNum++;
						}
						else
							GetCfgError(format);
						ethpara.eth_yc_datatype = pKey->i32CfgValue;
						pKey++;
					  u32ResultNum++;						
						
						
					}
					else if (_stricmp(pTok, "yxmaxsendnum") == 0)
					{
						pTok = strtok_s(nullptr, " =]", &pTok_Next);
						pKey->u16CfgType = KEY_UPSTREAM_YXMAXSENDNUM;
						sscanf_s(pTok, "%d", &(pKey->i32CfgValue));
						ethpara.eth_trans_yxnum = pKey->i32CfgValue;
						pKey++;
						u32ResultNum++;
					}
					else if (_stricmp(pTok, "ycmaxsendnum") == 0)
					{
						pTok = strtok_s(nullptr, " =]", &pTok_Next);
						pKey->u16CfgType = KEY_UPSTREAM_YCMAXSENDNUM;
						sscanf_s(pTok, "%d", &(pKey->i32CfgValue));
						ethpara.eth_trans_ycnum = pKey->i32CfgValue;
						pKey++;
						u32ResultNum++;
					}
					else if (_stricmp(pTok, "timingselect") == 0)
					{
						pTok = strtok_s(nullptr, " =]", &pTok_Next);
						if (_stricmp(pTok, "no") == 0)
						{
							pKey->u16CfgType = KEY_UPSTREAM_TIMINGSELECT;
							pKey->i32CfgValue = 0;
							//pKey++;
							//u32ResultNum++;
						}
						else if (_stricmp(pTok, "yes") == 0)
						{
							pKey->u16CfgType = KEY_UPSTREAM_TIMINGSELECT;
							pKey->i32CfgValue = 1;
							//pKey++;
							//u32ResultNum++;
						}
						else
							GetCfgError(format);
						ethpara.eth_timingselect = pKey->i32CfgValue;
						pKey++;
						u32ResultNum++;						
						
					}
					else if (_stricmp(pTok, "yktimeout") == 0)
					{
						pTok = strtok_s(nullptr, " =]", &pTok_Next);
						pKey->u16CfgType = KEY_UPSTREAM_YKTIMEOUT;
						sscanf_s(pTok, "%d", &(pKey->i32CfgValue));
						ethpara.eth_yk_timeout  = pKey->i32CfgValue;
						pKey++;
						u32ResultNum++;
					}
					else
					{//无效属性 不解析

					}

					pTok = strtok_s(nullptr, " =]", &pTok_Next);
				} while (pTok != nullptr);
				break;
			}
			break;
		default:
			pHandle->error = format;
			return 0;
		}
		length -= discard;
		buf += discard;
	}
	//save config 
	/*if(savetype == 1)
	{
		
	}
	else if(savetype == 2)
	{
				//save uart config
				
			if(uartpara[uart_index].channel_num == 0)   //串口1数据
			{
				  uartpara[uart_index].uart_usedflg = 1;
				 //strcpy(uartpara[uart_index],keypoint,sizeof(uartpara[uart_index]));
				  rt_mb_send(&mb, TYPE_RECORD_UART0);//发存储邮件
			}
    	
	}
	else if(savetype == 3)
	{
		
	}
	else if(savetype == 4)
	{
		
	}*/
			
	return u32ResultNum;
}






