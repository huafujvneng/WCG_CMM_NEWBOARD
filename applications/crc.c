/********************************************************************************/
/*版权：                                                                        */
/*    本程序所有版权归城乡电网自动化系统部技术部所有                            */
/*                                                                              */
/*说明：                                                                        */
/*    本文件用于集成归约用CRC函数打包。以利于新的文件方便调用。                 */
/*    本程序允许所有用户调用，但不可修改原函数，以防止因改动其中的一部分导致原  */
/*来编写的所有的规约转换出现错误。                                              */
/*                                                                              */
/*规定：                                                                        */
/*    利用本程序编写的所有文件必须遵从所有源代码采用规范的编程格式编写：        */
/*     1.在需要{}的时候必须使用相应的括号，不能随意节省；                       */
/*     2.所有优先级的裁定采用圆括弧（()）的方式强制进行，不采用C语言默认的优先  */
/*       级，以使程序简单明了；                                                 */
/*     3.注释采用汉字标注，简单明了；                                           */
/*     4.所有文件修改见change.c或change.h                                       */
/*     5.所有tab键采用4个空格代替                                               */
/*                                                                              */
/********************************************************************************/
#include "crc.h"



// CRC ??????
const uint8_t auchCRCHi[]={
	0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x01,0xC0,
	0x80,0x41,0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,
	0x00,0xC1,0x81,0x40,0x00,0xC1,0x81,0x40,0x01,0xC0,
	0x80,0x41,0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,
	0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x00,0xC1,
	0x81,0x40,0x01,0xC0,0x80,0x41,0x01,0xC0,0x80,0x41,
	0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x00,0xC1,
	0x81,0x40,0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,
	0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x01,0xC0,
	0x80,0x41,0x00,0xC1,0x81,0x40,0x00,0xC1,0x81,0x40,
	0x01,0xC0,0x80,0x41,0x01,0xC0,0x80,0x41,0x00,0xC1,
	0x81,0x40,0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,
	0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x01,0xC0,
	0x80,0x41,0x00,0xC1,0x81,0x40,0x00,0xC1,0x81,0x40,
	0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,0x01,0xC0,
	0x80,0x41,0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,
	0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x01,0xC0,
	0x80,0x41,0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,
	0x00,0xC1,0x81,0x40,0x00,0xC1,0x81,0x40,0x01,0xC0,
	0x80,0x41,0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,
	0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,0x01,0xC0,
	0x80,0x41,0x00,0xC1,0x81,0x40,0x00,0xC1,0x81,0x40,
	0x01,0xC0,0x80,0x41,0x01,0xC0,0x80,0x41,0x00,0xC1,
	0x81,0x40,0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,
	0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x01,0xC0,
	0x80,0x41,0x00,0xC1,0x81,0x40} ;
// CRC??????
const uint8_t auchCRCLo[]={
	0x00,0xC0,0xC1,0x01,0xC3,0x03,0x02,0xC2,0xC6,0x06,
	0x07,0xC7,0x05,0xC5,0xC4,0x04,0xCC,0x0C,0x0D,0xCD,
	0x0F,0xCF,0xCE,0x0E,0x0A,0xCA,0xCB,0x0B,0xC9,0x09,
	0x08,0xC8,0xD8,0x18,0x19,0xD9,0x1B,0xDB,0xDA,0x1A,
	0x1E,0xDE,0xDF,0x1F,0xDD,0x1D,0x1C,0xDC,0x14,0xD4,
	0xD5,0x15,0xD7,0x17,0x16,0xD6,0xD2,0x12,0x13,0xD3,
	0x11,0xD1,0xD0,0x10,0xF0,0x30,0x31,0xF1,0x33,0xF3,
	0xF2,0x32,0x36,0xF6,0xF7,0x37,0xF5,0x35,0x34,0xF4,
	0x3C,0xFC,0xFD,0x3D,0xFF,0x3F,0x3E,0xFE,0xFA,0x3A,
	0x3B,0xFB,0x39,0xF9,0xF8,0x38,0x28,0xE8,0xE9,0x29,
	0xEB,0x2B,0x2A,0xEA,0xEE,0x2E,0x2F,0xEF,0x2D,0xED,
	0xEC,0x2C,0xE4,0x24,0x25,0xE5,0x27,0xE7,0xE6,0x26,
	0x22,0xE2,0xE3,0x23,0xE1,0x21,0x20,0xE0,0xA0,0x60,
	0x61,0xA1,0x63,0xA3,0xA2,0x62,0x66,0xA6,0xA7,0x67,
	0xA5,0x65,0x64,0xA4,0x6C,0xAC,0xAD,0x6D,0xAF,0x6F,
	0x6E,0xAE,0xAA,0x6A,0x6B,0xAB,0x69,0xA9,0xA8,0x68,
	0x78,0xB8,0xB9,0x79,0xBB,0x7B,0x7A,0xBA,0xBE,0x7E,
	0x7F,0xBF,0x7D,0xBD,0xBC,0x7C,0xB4,0x74,0x75,0xB5,
	0x77,0xB7,0xB6,0x76,0x72,0xB2,0xB3,0x73,0xB1,0x71,
	0x70,0xB0,0x50,0x90,0x91,0x51,0x93,0x53,0x52,0x92,
	0x96,0x56,0x57,0x97,0x55,0x95,0x94,0x54,0x9C,0x5C,
	0x5D,0x9D,0x5F,0x9F,0x9E,0x5E,0x5A,0x9A,0x9B,0x5B,
	0x99,0x59,0x58,0x98,0x88,0x48,0x49,0x89,0x4B,0x8B,
	0x8A,0x4A,0x4E,0x8E,0x8F,0x4F,0x8D,0x4D,0x4C,0x8C,
	0x44,0x84,0x85,0x45,0x87,0x47,0x46,0x86,0x82,0x42,
	0x43,0x83,0x41,0x81,0x80,0x40};

/********************************************************************************/
/*函数说明                                                                      */
/*    MODBUS规约16位CRC校验函数                                                 */
/*                                                                              */
/*参数说明                                                                      */
/*    *puchMsg:                需要校验的寄存器                                 */
/*    usDataLen:                   需要校验的报文长度                            */
/*返回值                                                                        */
/*    无符号整形的校验值                                                        */
/********************************************************************************/
uint16_t  crc16(uint8_t *puchMsg, uint8_t usDataLen)
{
	uint8_t uchCRCHi = 0xFF; // 高CRC字节初始化
	uint8_t uchCRCLo = 0xFF; // 低CRC 字节初始化
	uint16_t uIndex;           // CRC循环中的索引
	while(usDataLen--)             // 传输消息缓冲区
	{
		uIndex = uchCRCHi ^ *puchMsg++;// 计算CRC
		uchCRCHi = uchCRCLo ^ auchCRCHi[uIndex];
		uchCRCLo = auchCRCLo[uIndex];
	}
	return (uchCRCHi << 8 | uchCRCLo);//结果Hi中放crc低8位，Lo放高8位
}

/********************************************************************************/
/*函数说明                                                                      */
/*    设备程序CRC版本校验                                               */
/*                                                                              */
/*参数说明                                                                      */
/*    *puchMsg:                需要校验的寄存器                                 */
/*    usDataLen:                   需要校验的报文长度                            */
/*返回值                                                                        */
/*    无符号整形的校验值                                                        */
/********************************************************************************/
uint16_t crc32(uint8_t *puchMsg, uint32_t usDataLen)
{
	uint8_t uchCRCHi = 0xFF; // 高CRC字节初始化Para_set
	uint8_t uchCRCLo = 0xFF; // 低CRC字节初始化
	uint16_t uIndex;           // CRC循环中的索引
	uint16_t crc;
	while(usDataLen--)             // 传输消息缓冲区
	{
		uIndex = uchCRCHi ^ *puchMsg++;// 计算CRC
		uchCRCHi = uchCRCLo ^ auchCRCHi[uIndex];
		uchCRCLo = auchCRCLo[uIndex];
	}
	crc = (uchCRCHi << 8 | uchCRCLo);//结果Hi中放crc低8位，Lo放高8位
	return(crc);
}


/********************************************************************************/
/*函数说明                                                                      */
/*    MODBUS规约16位CRC校验函数                                                 */
/*                                                                              */
/*参数说明                                                                      */
/*    *pCRCbuff:                需要校验的寄存器                                */
/*    length:                   需要校验的报文长度                              */
/*返回值                                                                        */
/*    无符号整形的校验值                                                        */
/********************************************************************************/
uint16_t Modbus_Crc(unsigned char *pCRCbuff,int length)
{
    unsigned char *p=pCRCbuff;
    unsigned int crc=0xffff;
    int i,j=0;

    for(i=0;i<length;i++)
    {
        crc ^= *(p+i);
        for(j=0;j<8;j++)
        {
            if((crc&0x0001)==0x0001)
            {
                crc=(crc>>1)&0xffff;
                crc^=0xa001;
            }
            else
            {
                crc=(crc>>1)&0xffff;
            }
        }
    }
    return crc;
}

/********************************************************************************/
/*函数说明                                                                      */
/*    dlt645规约电度表校验                                                      */
/*                                                                              */
/*参数说明                                                                      */
/*    *pCRCbuff:                需要校验的寄存器                                */
/*返回值                                                                        */
/*    无符号字符型的校验值                                                      */
/********************************************************************************/
unsigned char DLT645_Crc(unsigned char *pCRCbuff)
{
    unsigned char *p=pCRCbuff;
    unsigned char crc;
    int i;

    crc=0;
    for(i=0;i<(*(p+9)+10);i++)
    {
        crc +=*(p+i);
    }
    return crc;

}

/********************************************************************************/
/*函数说明                                                                      */
/*    得到CRC规约的校验码                                                       */
/*                                                                              */
/*参数说明                                                                      */
/*    *p:                       需要校验的报文                                  */
/*    iLen:                     需要校验的报文长度                              */
/*返回值                                                                        */
/*    返回校验码                                                                */
/********************************************************************************/
unsigned char Cdt_Crc(unsigned char *p,int iLen)
{
    long GenVal=0x0107;
    int i,j;
    unsigned char crc=0;

    unsigned char temp; 
    int reg=0;

    for (i=0;i<iLen+1;i++)
    {                   
        if (i<iLen)
            temp=*(p+i);
        else
            temp=0;

        for (j=7;j>=0;j--)
        {
            reg=(reg<<1) | ((temp>>j) & 1);
            if (reg & 0x100)
            reg=reg ^ GenVal;
        }
    }
    crc=reg;
    crc=crc ^ 0xFF;
    return crc;
}


/********************************************************************************/
/*函数说明                                                                      */
/*    累加和校验                                                                */
/*                                                                              */
/*参数说明                                                                      */
/*    *p:                       报文寄存器                                      */
/*    index:                    开始计算校验和的寄存器位置,从零计算             */
/*    length:                   开始计算校验和的寄存器数量                      */
/*返回值                                                                        */
/*    计算所得的校验码                                                          */
/********************************************************************************/
int Sum_Crc(unsigned char *p,int index,int length)
{
    int crc=0;
    int i=0;
    
    for(i=0;i<length;i++)
    {
        crc  +=*(p+index+i);
    }
    return crc;
}

