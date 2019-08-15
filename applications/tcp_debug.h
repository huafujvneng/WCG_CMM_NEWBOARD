#ifndef _TCP_DEBUG_H_
#define _TCP_DEBUG_H_

#include <stdint.h>
//#include <stdio.h>
#include <rtthread.h>
#include "base_task.h"
#include "fm3_ext_int.h"
#include <string.h>
#include <lwip/api.h>
#include <lwip/sockets.h>

#include "ethernetif.h"
#include "base_define.h"
#include "rtdbdefine.h"
#define		TCP_DEBUG_EN

#define		DEBUG_BUF_SIZE		2048ul
#define		DEBUG_PRINTF_MAX	256ul
#define		DEBUG_RXBUF_SIZE	256ul


#define		PEEK			(1)


//define the server cmd
//������ϢΪʮ���ֽ� 55 AA 00 0A 00 00 00 00 00 00
/*    ֡ͷ      ���ݸ���   �������к�   �������к�   ��������              
      2 byte      2 byte      2 byte      2 byte      2 byte 
*/
#define START_CODE	0x55AA			//֡ͷ			

#define CMPROTOCOL_COMMAND_BASE_MASK								0xF000	//������ ����
#define CMPROTOCOL_COMMAND_SUB_MASK									0x0FFF	//������ ����

#define CMPROTOCOL_COMMAND_COMMON									0x0000	//�������� ����
#define CMPROTOCOL_COMMAND_COMMON_DISCOVER							0x0000	//�������� �ӽ� ֪̽װ�� 
#define CMPROTOCOL_COMMAND_COMMON_INIT								0x0001	//�������� �ӽ� ��ʼ������ȡװ����Ϣ
#define CMPROTOCOL_COMMAND_COMMON_ESTABLISHED						0x0002	//�������� �ӽ� ��֪װ��������
#define CMPROTOCOL_COMMAND_COMMON_RESET								0x0003	//�������� �ӽ� ����װ��

#define CMPROTOCOL_COMMAND_STREAM_CTL							    0x1000	//���Ŀ������� ����
#define CMPROTOCOL_COMMAND_STREAM_CTL_STREAM_BASE_MASK				0x0080	//���Ŀ������� �ӽ� ͨ���������루ѡ������ͨ������װ��ͨ����
#define CMPROTOCOL_COMMAND_STREAM_CTL_STREAM_INDEX_MASK				0x007F	//���Ŀ������� �ӽ� ͨ��������
#define CMPROTOCOL_COMMAND_STREAM_CTL_UPSTREAM_BASE					0x0000	//���Ŀ������� �ӽ� ����ͨ���Ż�ֵ eth
#define CMPROTOCOL_COMMAND_STREAM_CTL_UPSTREAM_BASE_ALL				0x007F	//���Ŀ������� �ӽ� ��������ͨ��
#define CMPROTOCOL_COMMAND_STREAM_CTL_DEVICESTREAM_BASE				0x0080	//���Ŀ������� �ӽ� װ��ͨ���Ż�ֵ device
#define CMPROTOCOL_COMMAND_STREAM_CTL_DEVICESTREAM_BASE_ALL			0x007F	//���Ŀ������� �ӽ� ����װ��ͨ��
#define CMPROTOCOL_COMMAND_STREAM_CTL_STREAM_TX						0x0100	//���Ŀ������� �ӽ� ����ͨ�����ͷ��ͱ���ʹ��״̬
#define CMPROTOCOL_COMMAND_STREAM_CTL_STREAM_RX						0x0200	//���Ŀ������� �ӽ� ����ͨ�����ͽ��ܱ���ʹ��״̬
#define CMPROTOCOL_COMMAND_STREAM_CTL_STREAM_ON						0x0400	//���Ŀ������� �ӽ� ����ͨ�����ͱ���ʹ��
#define CMPROTOCOL_COMMAND_STREAM_CTL_STREAM_OFF					0x0800	//���Ŀ������� �ӽ� ����ͨ�����ͱ��Ľ�ֹ

#define CMPROTOCOL_COMMAND_STREAM_DATA								0x2000	//������������ ���� �ش�
#define CMPROTOCOL_COMMAND_STREAM_DATA_STREAM_BASE_MASK				0x0080	//������������ �ӽ� ͨ���������루ѡ������ͨ������װ��ͨ����
#define CMPROTOCOL_COMMAND_STREAM_DATA_STREAM_INDEX_MASK			0x007F	//������������ �ӽ� ͨ��������
#define CMPROTOCOL_COMMAND_STREAM_DATA_UPSTREAM_BASE				0x0000	//������������ �ӽ� ����ͨ���Ż�ֵ
#define CMPROTOCOL_COMMAND_STREAM_DATA_DEVICESTREAM_BASE			0x0080	//������������ �ӽ� װ��ͨ���Ż�ֵ
#define CMPROTOCOL_COMMAND_STREAM_DATA_STREAM_TX					0x0100	//������������ �ӽ� ����Ϊ��Ӧͨ�����͵�
#define CMPROTOCOL_COMMAND_STREAM_DATA_STREAM_RX					0x0200	//������������ �ӽ� ����Ϊ��Ӧͨ�����յ�

#define CMPROTOCOL_COMMAND_CFG										  0x4000	//���ò������� ����
#define CMPROTOCOL_COMMAND_CFG_CTL									0x0000	//���ò������� �ӽ� ���������ļ�
#define CMPROTOCOL_COMMAND_CFG_SEND									0x0100	//���ò������� �ӽ� ���������ļ�
#define CMPROTOCOL_COMMAND_CFG_GET									0x0200	//���ò������� �ӽ� ��ȡ�����ļ�

#define CMPROTOCOL_COMMAND_TXFLG                    0X00 //
#define CMPROTOCOL_COMMAND_RXFLG                    0X01
#define CMPROTOCOL_COMMAND_UPFLG                    0X00
#define CMPROTOCOL_COMMAND_DOWNFLG                  0X01


typedef struct __attribute__((__aligned__(4)))
{
	volatile uint32_t busy;
	
	volatile uint16_t write_p;
	volatile uint16_t send_p;
	
	volatile uint16_t recv_p;
	volatile uint16_t handle_p;
	
	char buf[DEBUG_BUF_SIZE];
	char buf1[16];
	char rxbuf[DEBUG_RXBUF_SIZE];
} DEBUGBUF;


void tcp_debug_init(void);
//uint8_t	tcp_debug_printf(const char *fmt, ...);
void eth_server_debug(void);

void debug_buf_write(const char *buf, uint8_t size);
uint32_t debug_buf_read(char *buf, uint32_t size, uint32_t flag);

#endif 
