#ifndef _CRC_H
#define _CRC_H


#include "base_define.h"


uint16_t Modbus_Crc(unsigned char *pCRCbuff,int length);


unsigned char DLT645_Crc(unsigned char *pCRCbuff);


unsigned char Cdt_Crc(unsigned char *p,int iLen);


int Sum_Crc(unsigned char *p,int index,int length);

uint16_t  crc16(uint8_t *puchMsg, uint8_t usDataLen);//modbus crc 

uint16_t crc32(uint8_t *puchMsg, uint32_t usDataLen);

#endif
