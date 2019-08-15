/**************************************************************************
Copyright (C), 2012, XJ ELECTRIC Co., LTD.
文件名  ：  modbus600.c
作者    ：  wanghusen
项目名称：  
功能    ：  串口0处理程序 负责该串口的数据接收 发送
创建日期：  2017/9/6 
备注    ：  降低耦合度，每个文件处理程序尽量独立使用
修改记录：
**************************************************************************/

#include "modbus600.h"

//定义 遥测 查询命令 查询地址
uint16_t read_yc_buf1[3] = {0x0004, 0x2000 , 0x12 };

//定义 遥信 查询命令 查询地址
uint16_t read_yx_buf1[3] = {0x0002, 0x0000 ,0x30};

//遥控//断路器
uint16_t trans_ykdl_buf1[2] = {0x0005, 0x3000 };

//遥控压板
uint16_t trans_ykyb_buf1[2] = {0x0006, 0x4000 };
//读取遥脉
uint16_t trans_ym_buf1[3] = {0x0004, 0x2100 ,0x08};




extern uint16_t  crc16(uint8_t *puchMsg, uint8_t usDataLen);
extern struct  UART_CONF uartpara[UARTMAXNUM] ;
extern struct  UART_TXRX_STATUS uart_status[UARTMAXNUM];
extern uint8_t    uarttxflg ,uartrxflg ;
/********************************************************************************/
/*函数说明                                                                      */
/*    数据复制程序                      */
/*                                                                              */
/*参数说明                                                                      */
/*p1目标地址 ，*p2源地址，num复制个数                                         */
/*返回值              null                                                      */ 
/********************************************************************************/
extern void datacopy(uint16_t *p1,uint16_t *p2,uint16_t num);


//按照字节复制
extern void datacopy_8(uint8_t *p1,uint8_t *p2,uint16_t num);


/********************************************************************************/
/*函数说明                                                                      */
/*    数据比较程序                      */
/*                                                                              */
/*参数说明                                                                      */
/*p1目标地址 ，*p2源地址，num比较个数                                         */
/*返回值              true                                                      */ 
/********************************************************************************/
extern uint8_t arry_compa(uint16_t *p1,uint16_t *p2,uint16_t num);


/********************************************************************************/
/*函数说明                                                                      */
/*    MODBUS遥控命令下发                    */
/*                                                                              */
/*参数说明                                                                      */
/*    uartportflg:                    设备串口号                                 */

/*    devindex:                    该串口下装置存储序号                      */
/*    *transbuf:                    发送数据指针                           */


/*返回值                                                                        */
/*    i                      返回发送数据长度                               */
/********************************************************************************/
uint8_t yk_dl_cmdmd600_1[5] = {0x05,0x30,0x00,0xff,0x00};
uint8_t yk_yb_cmdmd600_1[5] = {0x06,0x40,0x00,0x00,0x00};
uint8_t yk_time_cmdmd600_1[6] = {0x10,0x19,0x00,0x00,0x03,0x06};

uint8_t trans_yk_cmd_modbus_1(uint8_t uartportflg,uint8_t devindex,uint8_t *transbuf)
{
    uint16_t i=0;
	  uint8_t deviceaddr;
	  uint16_t num;
	  uint8_t ykinf,ykdcc;
	  
	  
	
    
	
  	switch(uartportflg)
  	{
	  	case 0:
				switch(yk_data_buf0[devindex][2])//yk data type
				{
					
				    case REMOTE_DL_TYPE:
					  if((yk_data_buf0[devindex][5] == 0x81)||(yk_data_buf0[devindex][5] == 0x82))//遥控选择
            {
							yk_data_buf0[devindex][0] = 0x55;
							uart_status[uartportflg].uartdevice[devindex].uart_ykflg = 0;
							return FALSE;
						}							
					  else if((yk_data_buf0[devindex][5] == 0x01)||(yk_data_buf0[devindex][5] == 0x02))//遥控执行
						{
							ykinf = yk_data_buf0[devindex][4];
							ykdcc = yk_data_buf0[devindex][5];
							transbuf[i++] = yk_data_buf0[devindex][3];
						
							transbuf[i++] = yk_dl_cmdmd600_1[0];
							transbuf[i++] = yk_dl_cmdmd600_1[1];							
							transbuf[i++] = yk_dl_cmdmd600_1[2] + ykinf*2 + ykdcc;// inf
							transbuf[i++] = yk_dl_cmdmd600_1[3];							
							transbuf[i++] = yk_dl_cmdmd600_1[4];		
							uart_status[uartportflg].uartdevice[devindex].uart_ykflg = 0;
              yk_data_buf0[devindex][0] = 0x55;							
							return i;
							
						}
						break;
						
					case REMOTE_FG_TYPE:
						
						 transbuf[i++] = yk_data_buf0[devindex][3];						 
						 transbuf[i++] = yk_dl_cmdmd600_1[0];
						 transbuf[i++] = yk_dl_cmdmd600_1[1];					
						 transbuf[i++] = yk_dl_cmdmd600_1[2];
						 transbuf[i++] = yk_dl_cmdmd600_1[3];
						 transbuf[i++] = yk_dl_cmdmd600_1[4];
						 uart_status[uartportflg].uartdevice[devindex].uart_ykflg = 0;
             yk_data_buf0[devindex][0] = 0x55;	
             return i;					
						 break;
					//uint8_t yk_yb_cmdmd600_1[5] = {0x06,0x40,0x00,0x00,0x00};
					case REMOTE_YB_TYPE:
							ykinf = yk_data_buf0[devindex][4];
							ykdcc = yk_data_buf0[devindex][5];
							transbuf[i++] = yk_data_buf0[devindex][3];	//addr									 
						  transbuf[i++] = yk_yb_cmdmd600_1[0]; //0x06
						  transbuf[i++] = yk_yb_cmdmd600_1[1];					
						  transbuf[i++] = yk_yb_cmdmd600_1[2]+ykinf;
						  transbuf[i++] = yk_yb_cmdmd600_1[3];
						  transbuf[i++] = ykdcc;
						  uart_status[uartportflg].uartdevice[devindex].uart_ykflg = 0;
              yk_data_buf0[devindex][0] = 0x55;							
              return i;							
						  break;				

					
					case REMOTE_DZ_TYPE:
						
						 break;	
					
          case  REMOTE_TIME_TYPE://yk_time_cmdmd600_1[6] = {0x10,0x19,0x00,0x00,0x03,0x06};
						  transbuf[i++] = yk_data_buf0[devindex][3];
						  transbuf[i++] = yk_time_cmdmd600_1[0];
						  transbuf[i++] = yk_time_cmdmd600_1[1];
						  transbuf[i++] = yk_time_cmdmd600_1[2];
						  transbuf[i++] = yk_time_cmdmd600_1[3];
						  transbuf[i++] = yk_time_cmdmd600_1[4];
						  transbuf[i++] = yk_time_cmdmd600_1[5];

              
							transbuf[i++] = (TIME.year/10*16)+(TIME.year%10);
							transbuf[i++] = (TIME.month/10*16)+(TIME.month%10);							
							transbuf[i++] = (TIME.day/10*16)+(TIME.day%10);							
							transbuf[i++] = (TIME.hour/10*16)+(TIME.hour%10);
							transbuf[i++] = (TIME.minute/10*16)+(TIME.minute%10);
							transbuf[i++] = (TIME.second/10*16)+(TIME.second%10);


						  uart_status[uartportflg].uartdevice[devindex].uart_ykflg = 0;
              yk_data_buf0[devindex][0] = 0x55;		
              return i;						
						 break;
					
					case  REMOTE_QHDZ_TYPE:
							ykinf = yk_data_buf0[devindex][4];
							ykdcc = yk_data_buf0[devindex][5];
							transbuf[i++] = yk_data_buf0[devindex][3];	//addr									 
						  transbuf[i++] = yk_yb_cmdmd600_1[0]; //0x06
						  transbuf[i++] = yk_yb_cmdmd600_1[1];					
						  transbuf[i++] = yk_yb_cmdmd600_1[2]+ykinf;
						  transbuf[i++] = yk_yb_cmdmd600_1[3];
						  transbuf[i++] = ykdcc;
						  uart_status[uartportflg].uartdevice[devindex].uart_ykflg = 0;
              yk_data_buf0[devindex][0] = 0x55;	
              return i;							 
					    break;
					
					
				}
			
			break;
		
	  	
	  	case 1:
				switch(yk_data_buf1[devindex][2])//yk data type
				{
					
				    case REMOTE_DL_TYPE:
					  if((yk_data_buf1[devindex][5] == 0x81)||(yk_data_buf1[devindex][5] == 0x82))//遥控选择
            {
							yk_data_buf1[devindex][0] = 0x55;
							uart_status[uartportflg].uartdevice[devindex].uart_ykflg = 0;
							return FALSE;
						}							
					  else if((yk_data_buf1[devindex][5] == 0x01)||(yk_data_buf1[devindex][5] == 0x02))//遥控执行
						{
							ykinf = yk_data_buf1[devindex][4];
							ykdcc = yk_data_buf1[devindex][5];
							transbuf[i++] = yk_data_buf1[devindex][3];
						
							transbuf[i++] = yk_dl_cmdmd600_1[0];
							transbuf[i++] = yk_dl_cmdmd600_1[1];							
							transbuf[i++] = yk_dl_cmdmd600_1[2] + ykinf*2 + ykdcc;// inf
							transbuf[i++] = yk_dl_cmdmd600_1[3];							
							transbuf[i++] = yk_dl_cmdmd600_1[4];		
							uart_status[uartportflg].uartdevice[devindex].uart_ykflg = 0;
              yk_data_buf1[devindex][0] = 0x55;							
							return i;
							
						}
						break;
						
					case REMOTE_FG_TYPE:
						
						 transbuf[i++] = yk_data_buf1[devindex][3];						 
						 transbuf[i++] = yk_dl_cmdmd600_1[0];
						 transbuf[i++] = yk_dl_cmdmd600_1[1];					
						 transbuf[i++] = yk_dl_cmdmd600_1[2];
						 transbuf[i++] = yk_dl_cmdmd600_1[3];
						 transbuf[i++] = yk_dl_cmdmd600_1[4];
						 uart_status[uartportflg].uartdevice[devindex].uart_ykflg = 0;
             yk_data_buf1[devindex][0] = 0x55;	
             return i;					
						 break;
					//uint8_t yk_yb_cmdmd600_1[5] = {0x06,0x40,0x00,0x00,0x00};
					case REMOTE_YB_TYPE:
							ykinf = yk_data_buf1[devindex][4];
							ykdcc = yk_data_buf1[devindex][5];
							transbuf[i++] = yk_data_buf0[devindex][3];	//addr									 
						  transbuf[i++] = yk_yb_cmdmd600_1[0]; //0x06
						  transbuf[i++] = yk_yb_cmdmd600_1[1];					
						  transbuf[i++] = yk_yb_cmdmd600_1[2]+ykinf;
						  transbuf[i++] = yk_yb_cmdmd600_1[3];
						  transbuf[i++] = ykdcc;
						  uart_status[uartportflg].uartdevice[devindex].uart_ykflg = 0;
              yk_data_buf1[devindex][0] = 0x55;							
              return i;							
						  break;				

					
					case REMOTE_DZ_TYPE:
						
						 break;	
					
          case  REMOTE_TIME_TYPE://yk_time_cmdmd600_1[6] = {0x10,0x19,0x00,0x00,0x03,0x06};
						  transbuf[i++] = yk_data_buf1[devindex][3];
						  transbuf[i++] = yk_time_cmdmd600_1[0];
						  transbuf[i++] = yk_time_cmdmd600_1[1];
						  transbuf[i++] = yk_time_cmdmd600_1[2];
						  transbuf[i++] = yk_time_cmdmd600_1[3];
						  transbuf[i++] = yk_time_cmdmd600_1[4];
						  transbuf[i++] = yk_time_cmdmd600_1[5];

              
							transbuf[i++] = (TIME.year/10*16)+(TIME.year%10);
							transbuf[i++] = (TIME.month/10*16)+(TIME.month%10);							
							transbuf[i++] = (TIME.day/10*16)+(TIME.day%10);							
							transbuf[i++] = (TIME.hour/10*16)+(TIME.hour%10);
							transbuf[i++] = (TIME.minute/10*16)+(TIME.minute%10);
							transbuf[i++] = (TIME.second/10*16)+(TIME.second%10);


						  uart_status[uartportflg].uartdevice[devindex].uart_ykflg = 0;
              yk_data_buf1[devindex][0] = 0x55;		
              return i;						
						 break;
					
					case  REMOTE_QHDZ_TYPE:
							ykinf = yk_data_buf1[devindex][4];
							ykdcc = yk_data_buf1[devindex][5];
							transbuf[i++] = yk_data_buf1[devindex][3];	//addr									 
						  transbuf[i++] = yk_yb_cmdmd600_1[0]; //0x06
						  transbuf[i++] = yk_yb_cmdmd600_1[1];					
						  transbuf[i++] = yk_yb_cmdmd600_1[2]+ykinf;
						  transbuf[i++] = yk_yb_cmdmd600_1[3];
						  transbuf[i++] = ykdcc;
						  uart_status[uartportflg].uartdevice[devindex].uart_ykflg = 0;
              yk_data_buf1[devindex][0] = 0x55;	
              return i;							 
					    break;
					
					
				}
			
			break;

	  	case 3:
				switch(yk_data_buf3[devindex][2])//yk data type
				{
					
				    case REMOTE_DL_TYPE:
					  if((yk_data_buf3[devindex][5] == 0x81)||(yk_data_buf3[devindex][5] == 0x82))//遥控选择
            {
							yk_data_buf3[devindex][0] = 0x55;
							uart_status[uartportflg].uartdevice[devindex].uart_ykflg = 0;
							return FALSE;
						}							
					  else if((yk_data_buf3[devindex][5] == 0x01)||(yk_data_buf3[devindex][5] == 0x02))//遥控执行
						{
							ykinf = yk_data_buf3[devindex][4];
							ykdcc = yk_data_buf3[devindex][5];
							transbuf[i++] = yk_data_buf0[devindex][3];
						
							transbuf[i++] = yk_dl_cmdmd600_1[0];
							transbuf[i++] = yk_dl_cmdmd600_1[1];							
							transbuf[i++] = yk_dl_cmdmd600_1[2] + ykinf*2 + ykdcc;// inf
							transbuf[i++] = yk_dl_cmdmd600_1[3];							
							transbuf[i++] = yk_dl_cmdmd600_1[4];		
							uart_status[uartportflg].uartdevice[devindex].uart_ykflg = 0;
              yk_data_buf3[devindex][0] = 0x55;							
							return i;
							
						}
						break;
						
					case REMOTE_FG_TYPE:
						
						 transbuf[i++] = yk_data_buf3[devindex][3];						 
						 transbuf[i++] = yk_dl_cmdmd600_1[0];
						 transbuf[i++] = yk_dl_cmdmd600_1[1];					
						 transbuf[i++] = yk_dl_cmdmd600_1[2];
						 transbuf[i++] = yk_dl_cmdmd600_1[3];
						 transbuf[i++] = yk_dl_cmdmd600_1[4];
						 uart_status[uartportflg].uartdevice[devindex].uart_ykflg = 0;
             yk_data_buf3[devindex][0] = 0x55;	
             return i;					
						 break;
					//uint8_t yk_yb_cmdmd600_1[5] = {0x06,0x40,0x00,0x00,0x00};
					case REMOTE_YB_TYPE:
							ykinf = yk_data_buf3[devindex][4];
							ykdcc = yk_data_buf3[devindex][5];
							transbuf[i++] = yk_data_buf0[devindex][3];	//addr									 
						  transbuf[i++] = yk_yb_cmdmd600_1[0]; //0x06
						  transbuf[i++] = yk_yb_cmdmd600_1[1];					
						  transbuf[i++] = yk_yb_cmdmd600_1[2]+ykinf;
						  transbuf[i++] = yk_yb_cmdmd600_1[3];
						  transbuf[i++] = ykdcc;
						  uart_status[uartportflg].uartdevice[devindex].uart_ykflg = 0;
              yk_data_buf3[devindex][0] = 0x55;							
              return i;							
						  break;				

					
					case REMOTE_DZ_TYPE:
						
						 break;	
					
          case  REMOTE_TIME_TYPE://yk_time_cmdmd600_1[6] = {0x10,0x19,0x00,0x00,0x03,0x06};
						  transbuf[i++] = yk_data_buf0[devindex][3];
						  transbuf[i++] = yk_time_cmdmd600_1[0];
						  transbuf[i++] = yk_time_cmdmd600_1[1];
						  transbuf[i++] = yk_time_cmdmd600_1[2];
						  transbuf[i++] = yk_time_cmdmd600_1[3];
						  transbuf[i++] = yk_time_cmdmd600_1[4];
						  transbuf[i++] = yk_time_cmdmd600_1[5];

              
							transbuf[i++] = (TIME.year/10*16)+(TIME.year%10);
							transbuf[i++] = (TIME.month/10*16)+(TIME.month%10);							
							transbuf[i++] = (TIME.day/10*16)+(TIME.day%10);							
							transbuf[i++] = (TIME.hour/10*16)+(TIME.hour%10);
							transbuf[i++] = (TIME.minute/10*16)+(TIME.minute%10);
							transbuf[i++] = (TIME.second/10*16)+(TIME.second%10);


						  uart_status[uartportflg].uartdevice[devindex].uart_ykflg = 0;
              yk_data_buf3[devindex][0] = 0x55;		
              return i;						
						 break;
					
					case  REMOTE_QHDZ_TYPE:
							ykinf = yk_data_buf3[devindex][4];
							ykdcc = yk_data_buf3[devindex][5];
							transbuf[i++] = yk_data_buf3[devindex][3];	//addr									 
						  transbuf[i++] = yk_yb_cmdmd600_1[0]; //0x06
						  transbuf[i++] = yk_yb_cmdmd600_1[1];					
						  transbuf[i++] = yk_yb_cmdmd600_1[2]+ykinf;
						  transbuf[i++] = yk_yb_cmdmd600_1[3];
						  transbuf[i++] = ykdcc;
						  uart_status[uartportflg].uartdevice[devindex].uart_ykflg = 0;
              yk_data_buf3[devindex][0] = 0x55;	
              return i;							 
					    break;
					
					
				}
			
			break;


	  	case 4:
				switch(yk_data_buf4[devindex][2])//yk data type
				{
					
				    case REMOTE_DL_TYPE:
					  if((yk_data_buf4[devindex][5] == 0x81)||(yk_data_buf4[devindex][5] == 0x82))//遥控选择
            {
							yk_data_buf4[devindex][0] = 0x55;
							uart_status[uartportflg].uartdevice[devindex].uart_ykflg = 0;
							return FALSE;
						}							
					  else if((yk_data_buf4[devindex][5] == 0x01)||(yk_data_buf4[devindex][5] == 0x02))//遥控执行
						{
							ykinf = yk_data_buf4[devindex][4];
							ykdcc = yk_data_buf4[devindex][5];
							transbuf[i++] = yk_data_buf4[devindex][3];
						
							transbuf[i++] = yk_dl_cmdmd600_1[0];
							transbuf[i++] = yk_dl_cmdmd600_1[1];							
							transbuf[i++] = yk_dl_cmdmd600_1[2] + ykinf*2 + ykdcc;// inf
							transbuf[i++] = yk_dl_cmdmd600_1[3];							
							transbuf[i++] = yk_dl_cmdmd600_1[4];		
							uart_status[uartportflg].uartdevice[devindex].uart_ykflg = 0;
              yk_data_buf4[devindex][0] = 0x55;							
							return i;
							
						}
						break;
						
					case REMOTE_FG_TYPE:
						
						 transbuf[i++] = yk_data_buf4[devindex][3];						 
						 transbuf[i++] = yk_dl_cmdmd600_1[0];
						 transbuf[i++] = yk_dl_cmdmd600_1[1];					
						 transbuf[i++] = yk_dl_cmdmd600_1[2];
						 transbuf[i++] = yk_dl_cmdmd600_1[3];
						 transbuf[i++] = yk_dl_cmdmd600_1[4];
						 uart_status[uartportflg].uartdevice[devindex].uart_ykflg = 0;
             yk_data_buf4[devindex][0] = 0x55;	
             return i;					
						 break;
					//uint8_t yk_yb_cmdmd600_1[5] = {0x06,0x40,0x00,0x00,0x00};
					case REMOTE_YB_TYPE:
							ykinf = yk_data_buf4[devindex][4];
							ykdcc = yk_data_buf4[devindex][5];
							transbuf[i++] = yk_data_buf0[devindex][3];	//addr									 
						  transbuf[i++] = yk_yb_cmdmd600_1[0]; //0x06
						  transbuf[i++] = yk_yb_cmdmd600_1[1];					
						  transbuf[i++] = yk_yb_cmdmd600_1[2]+ykinf;
						  transbuf[i++] = yk_yb_cmdmd600_1[3];
						  transbuf[i++] = ykdcc;
						  uart_status[uartportflg].uartdevice[devindex].uart_ykflg = 0;
              yk_data_buf4[devindex][0] = 0x55;							
              return i;							
						  break;				

					
					case REMOTE_DZ_TYPE:
						
						 break;	
					
          case  REMOTE_TIME_TYPE://yk_time_cmdmd600_1[6] = {0x10,0x19,0x00,0x00,0x03,0x06};
						  transbuf[i++] = yk_data_buf4[devindex][3];
						  transbuf[i++] = yk_time_cmdmd600_1[0];
						  transbuf[i++] = yk_time_cmdmd600_1[1];
						  transbuf[i++] = yk_time_cmdmd600_1[2];
						  transbuf[i++] = yk_time_cmdmd600_1[3];
						  transbuf[i++] = yk_time_cmdmd600_1[4];
						  transbuf[i++] = yk_time_cmdmd600_1[5];

              
							transbuf[i++] = (TIME.year/10*16)+(TIME.year%10);
							transbuf[i++] = (TIME.month/10*16)+(TIME.month%10);							
							transbuf[i++] = (TIME.day/10*16)+(TIME.day%10);							
							transbuf[i++] = (TIME.hour/10*16)+(TIME.hour%10);
							transbuf[i++] = (TIME.minute/10*16)+(TIME.minute%10);
							transbuf[i++] = (TIME.second/10*16)+(TIME.second%10);


						  uart_status[uartportflg].uartdevice[devindex].uart_ykflg = 0;
              yk_data_buf4[devindex][0] = 0x55;		
              return i;						
						 break;
					
					case  REMOTE_QHDZ_TYPE:
							ykinf = yk_data_buf4[devindex][4];
							ykdcc = yk_data_buf4[devindex][5];
							transbuf[i++] = yk_data_buf4[devindex][3];	//addr									 
						  transbuf[i++] = yk_yb_cmdmd600_1[0]; //0x06
						  transbuf[i++] = yk_yb_cmdmd600_1[1];					
						  transbuf[i++] = yk_yb_cmdmd600_1[2]+ykinf;
						  transbuf[i++] = yk_yb_cmdmd600_1[3];
						  transbuf[i++] = ykdcc;
						  uart_status[uartportflg].uartdevice[devindex].uart_ykflg = 0;
              yk_data_buf4[devindex][0] = 0x55;	
              return i;							 
					    break;
					
					
				}
			
			break;


	  	case 5:
				switch(yk_data_buf5[devindex][2])//yk data type
				{
					
				    case REMOTE_DL_TYPE:
					  if((yk_data_buf5[devindex][5] == 0x81)||(yk_data_buf5[devindex][5] == 0x82))//遥控选择
            {
							yk_data_buf5[devindex][0] = 0x55;
							uart_status[uartportflg].uartdevice[devindex].uart_ykflg = 0;
							return FALSE;
						}							
					  else if((yk_data_buf5[devindex][5] == 0x01)||(yk_data_buf5[devindex][5] == 0x02))//遥控执行
						{
							ykinf = yk_data_buf5[devindex][4];
							ykdcc = yk_data_buf5[devindex][5];
							transbuf[i++] = yk_data_buf5[devindex][3];
						
							transbuf[i++] = yk_dl_cmdmd600_1[0];
							transbuf[i++] = yk_dl_cmdmd600_1[1];							
							transbuf[i++] = yk_dl_cmdmd600_1[2] + ykinf*2 + ykdcc;// inf
							transbuf[i++] = yk_dl_cmdmd600_1[3];							
							transbuf[i++] = yk_dl_cmdmd600_1[4];		
							uart_status[uartportflg].uartdevice[devindex].uart_ykflg = 0;
              yk_data_buf5[devindex][0] = 0x55;							
							return i;
							
						}
						break;
						
					case REMOTE_FG_TYPE:
						
						 transbuf[i++] = yk_data_buf5[devindex][3];						 
						 transbuf[i++] = yk_dl_cmdmd600_1[0];
						 transbuf[i++] = yk_dl_cmdmd600_1[1];					
						 transbuf[i++] = yk_dl_cmdmd600_1[2];
						 transbuf[i++] = yk_dl_cmdmd600_1[3];
						 transbuf[i++] = yk_dl_cmdmd600_1[4];
						 uart_status[uartportflg].uartdevice[devindex].uart_ykflg = 0;
             yk_data_buf5[devindex][0] = 0x55;	
             return i;					
						 break;
					//uint8_t yk_yb_cmdmd600_1[5] = {0x06,0x40,0x00,0x00,0x00};
					case REMOTE_YB_TYPE:
							ykinf = yk_data_buf5[devindex][4];
							ykdcc = yk_data_buf5[devindex][5];
							transbuf[i++] = yk_data_buf5[devindex][3];	//addr									 
						  transbuf[i++] = yk_yb_cmdmd600_1[0]; //0x06
						  transbuf[i++] = yk_yb_cmdmd600_1[1];					
						  transbuf[i++] = yk_yb_cmdmd600_1[2]+ykinf;
						  transbuf[i++] = yk_yb_cmdmd600_1[3];
						  transbuf[i++] = ykdcc;
						  uart_status[uartportflg].uartdevice[devindex].uart_ykflg = 0;
              yk_data_buf5[devindex][0] = 0x55;							
              return i;							
						  break;				

					
					case REMOTE_DZ_TYPE:
						
						 break;	
					
          case  REMOTE_TIME_TYPE://yk_time_cmdmd600_1[6] = {0x10,0x19,0x00,0x00,0x03,0x06};
						  transbuf[i++] = yk_data_buf5[devindex][3];
						  transbuf[i++] = yk_time_cmdmd600_1[0];
						  transbuf[i++] = yk_time_cmdmd600_1[1];
						  transbuf[i++] = yk_time_cmdmd600_1[2];
						  transbuf[i++] = yk_time_cmdmd600_1[3];
						  transbuf[i++] = yk_time_cmdmd600_1[4];
						  transbuf[i++] = yk_time_cmdmd600_1[5];

              
							transbuf[i++] = (TIME.year/10*16)+(TIME.year%10);
							transbuf[i++] = (TIME.month/10*16)+(TIME.month%10);							
							transbuf[i++] = (TIME.day/10*16)+(TIME.day%10);							
							transbuf[i++] = (TIME.hour/10*16)+(TIME.hour%10);
							transbuf[i++] = (TIME.minute/10*16)+(TIME.minute%10);
							transbuf[i++] = (TIME.second/10*16)+(TIME.second%10);


						  uart_status[uartportflg].uartdevice[devindex].uart_ykflg = 0;
              yk_data_buf5[devindex][0] = 0x55;		
              return i;						
						 break;
					
					case  REMOTE_QHDZ_TYPE:
							ykinf = yk_data_buf5[devindex][4];
							ykdcc = yk_data_buf5[devindex][5];
							transbuf[i++] = yk_data_buf0[devindex][3];	//addr									 
						  transbuf[i++] = yk_yb_cmdmd600_1[0]; //0x06
						  transbuf[i++] = yk_yb_cmdmd600_1[1];					
						  transbuf[i++] = yk_yb_cmdmd600_1[2]+ykinf;
						  transbuf[i++] = yk_yb_cmdmd600_1[3];
						  transbuf[i++] = ykdcc;
						  uart_status[uartportflg].uartdevice[devindex].uart_ykflg = 0;
              yk_data_buf5[devindex][0] = 0x55;	
              return i;							 
					    break;
					
					
				}
			
			break;

	  	case 6:
				switch(yk_data_buf6[devindex][2])//yk data type
				{
					
				    case REMOTE_DL_TYPE:
					  if((yk_data_buf6[devindex][5] == 0x81)||(yk_data_buf6[devindex][5] == 0x82))//遥控选择
            {
							yk_data_buf6[devindex][0] = 0x55;
							uart_status[uartportflg].uartdevice[devindex].uart_ykflg = 0;
							return FALSE;
						}							
					  else if((yk_data_buf6[devindex][5] == 0x01)||(yk_data_buf6[devindex][5] == 0x02))//遥控执行
						{
							ykinf = yk_data_buf6[devindex][4];
							ykdcc = yk_data_buf6[devindex][5];
							transbuf[i++] = yk_data_buf6[devindex][3];
						
							transbuf[i++] = yk_dl_cmdmd600_1[0];
							transbuf[i++] = yk_dl_cmdmd600_1[1];							
							transbuf[i++] = yk_dl_cmdmd600_1[2] + ykinf*2 + ykdcc;// inf
							transbuf[i++] = yk_dl_cmdmd600_1[3];							
							transbuf[i++] = yk_dl_cmdmd600_1[4];		
							uart_status[uartportflg].uartdevice[devindex].uart_ykflg = 0;
              yk_data_buf6[devindex][0] = 0x55;							
							return i;
							
						}
						break;
						
					case REMOTE_FG_TYPE:
						
						 transbuf[i++] = yk_data_buf6[devindex][3];						 
						 transbuf[i++] = yk_dl_cmdmd600_1[0];
						 transbuf[i++] = yk_dl_cmdmd600_1[1];					
						 transbuf[i++] = yk_dl_cmdmd600_1[2];
						 transbuf[i++] = yk_dl_cmdmd600_1[3];
						 transbuf[i++] = yk_dl_cmdmd600_1[4];
						 uart_status[uartportflg].uartdevice[devindex].uart_ykflg = 0;
             yk_data_buf6[devindex][0] = 0x55;	
             return i;					
						 break;
					//uint8_t yk_yb_cmdmd600_1[5] = {0x06,0x40,0x00,0x00,0x00};
					case REMOTE_YB_TYPE:
							ykinf = yk_data_buf6[devindex][4];
							ykdcc = yk_data_buf6[devindex][5];
							transbuf[i++] = yk_data_buf0[devindex][3];	//addr									 
						  transbuf[i++] = yk_yb_cmdmd600_1[0]; //0x06
						  transbuf[i++] = yk_yb_cmdmd600_1[1];					
						  transbuf[i++] = yk_yb_cmdmd600_1[2]+ykinf;
						  transbuf[i++] = yk_yb_cmdmd600_1[3];
						  transbuf[i++] = ykdcc;
						  uart_status[uartportflg].uartdevice[devindex].uart_ykflg = 0;
              yk_data_buf6[devindex][0] = 0x55;							
              return i;							
						  break;				

					
					case REMOTE_DZ_TYPE:
						
						 break;	
					
          case  REMOTE_TIME_TYPE://yk_time_cmdmd600_1[6] = {0x10,0x19,0x00,0x00,0x03,0x06};
						  transbuf[i++] = yk_data_buf6[devindex][3];
						  transbuf[i++] = yk_time_cmdmd600_1[0];
						  transbuf[i++] = yk_time_cmdmd600_1[1];
						  transbuf[i++] = yk_time_cmdmd600_1[2];
						  transbuf[i++] = yk_time_cmdmd600_1[3];
						  transbuf[i++] = yk_time_cmdmd600_1[4];
						  transbuf[i++] = yk_time_cmdmd600_1[5];

              
							transbuf[i++] = (TIME.year/10*16)+(TIME.year%10);
							transbuf[i++] = (TIME.month/10*16)+(TIME.month%10);							
							transbuf[i++] = (TIME.day/10*16)+(TIME.day%10);							
							transbuf[i++] = (TIME.hour/10*16)+(TIME.hour%10);
							transbuf[i++] = (TIME.minute/10*16)+(TIME.minute%10);
							transbuf[i++] = (TIME.second/10*16)+(TIME.second%10);


						  uart_status[uartportflg].uartdevice[devindex].uart_ykflg = 0;
              yk_data_buf6[devindex][0] = 0x55;		
              return i;						
						 break;
					
					case  REMOTE_QHDZ_TYPE:
							ykinf = yk_data_buf6[devindex][4];
							ykdcc = yk_data_buf6[devindex][5];
							transbuf[i++] = yk_data_buf0[devindex][3];	//addr									 
						  transbuf[i++] = yk_yb_cmdmd600_1[0]; //0x06
						  transbuf[i++] = yk_yb_cmdmd600_1[1];					
						  transbuf[i++] = yk_yb_cmdmd600_1[2]+ykinf;
						  transbuf[i++] = yk_yb_cmdmd600_1[3];
						  transbuf[i++] = ykdcc;
						  uart_status[uartportflg].uartdevice[devindex].uart_ykflg = 0;
              yk_data_buf6[devindex][0] = 0x55;	
              return i;							 
					    break;
					
					
				}
			
			break;


	  	case 7:
				switch(yk_data_buf0[devindex][2])//yk data type
				{
					
				    case REMOTE_DL_TYPE:
					  if((yk_data_buf7[devindex][5] == 0x81)||(yk_data_buf7[devindex][5] == 0x82))//遥控选择
            {
							yk_data_buf7[devindex][0] = 0x55;
							uart_status[uartportflg].uartdevice[devindex].uart_ykflg = 0;
							return FALSE;
						}							
					  else if((yk_data_buf7[devindex][5] == 0x01)||(yk_data_buf7[devindex][5] == 0x02))//遥控执行
						{
							ykinf = yk_data_buf7[devindex][4];
							ykdcc = yk_data_buf7[devindex][5];
							transbuf[i++] = yk_data_buf0[devindex][3];
						
							transbuf[i++] = yk_dl_cmdmd600_1[0];
							transbuf[i++] = yk_dl_cmdmd600_1[1];							
							transbuf[i++] = yk_dl_cmdmd600_1[2] + ykinf*2 + ykdcc;// inf
							transbuf[i++] = yk_dl_cmdmd600_1[3];							
							transbuf[i++] = yk_dl_cmdmd600_1[4];		
							uart_status[uartportflg].uartdevice[devindex].uart_ykflg = 0;
              yk_data_buf7[devindex][0] = 0x55;							
							return i;
							
						}
						break;
						
					case REMOTE_FG_TYPE:
						
						 transbuf[i++] = yk_data_buf7[devindex][3];						 
						 transbuf[i++] = yk_dl_cmdmd600_1[0];
						 transbuf[i++] = yk_dl_cmdmd600_1[1];					
						 transbuf[i++] = yk_dl_cmdmd600_1[2];
						 transbuf[i++] = yk_dl_cmdmd600_1[3];
						 transbuf[i++] = yk_dl_cmdmd600_1[4];
						 uart_status[uartportflg].uartdevice[devindex].uart_ykflg = 0;
             yk_data_buf7[devindex][0] = 0x55;	
             return i;					
						 break;
					//uint8_t yk_yb_cmdmd600_1[5] = {0x06,0x40,0x00,0x00,0x00};
					case REMOTE_YB_TYPE:
							ykinf = yk_data_buf7[devindex][4];
							ykdcc = yk_data_buf7[devindex][5];
							transbuf[i++] = yk_data_buf7[devindex][3];	//addr									 
						  transbuf[i++] = yk_yb_cmdmd600_1[0]; //0x06
						  transbuf[i++] = yk_yb_cmdmd600_1[1];					
						  transbuf[i++] = yk_yb_cmdmd600_1[2]+ykinf;
						  transbuf[i++] = yk_yb_cmdmd600_1[3];
						  transbuf[i++] = ykdcc;
						  uart_status[uartportflg].uartdevice[devindex].uart_ykflg = 0;
              yk_data_buf7[devindex][0] = 0x55;							
              return i;							
						  break;				

					
					case REMOTE_DZ_TYPE:
						
						 break;	
					
          case  REMOTE_TIME_TYPE://yk_time_cmdmd600_1[6] = {0x10,0x19,0x00,0x00,0x03,0x06};
						  transbuf[i++] = yk_data_buf7[devindex][3];
						  transbuf[i++] = yk_time_cmdmd600_1[0];
						  transbuf[i++] = yk_time_cmdmd600_1[1];
						  transbuf[i++] = yk_time_cmdmd600_1[2];
						  transbuf[i++] = yk_time_cmdmd600_1[3];
						  transbuf[i++] = yk_time_cmdmd600_1[4];
						  transbuf[i++] = yk_time_cmdmd600_1[5];

              
							transbuf[i++] = (TIME.year/10*16)+(TIME.year%10);
							transbuf[i++] = (TIME.month/10*16)+(TIME.month%10);							
							transbuf[i++] = (TIME.day/10*16)+(TIME.day%10);							
							transbuf[i++] = (TIME.hour/10*16)+(TIME.hour%10);
							transbuf[i++] = (TIME.minute/10*16)+(TIME.minute%10);
							transbuf[i++] = (TIME.second/10*16)+(TIME.second%10);


						  uart_status[uartportflg].uartdevice[devindex].uart_ykflg = 0;
              yk_data_buf7[devindex][0] = 0x55;		
              return i;						
						 break;
					
					case  REMOTE_QHDZ_TYPE:
							ykinf = yk_data_buf7[devindex][4];
							ykdcc = yk_data_buf7[devindex][5];
							transbuf[i++] = yk_data_buf7[devindex][3];	//addr									 
						  transbuf[i++] = yk_yb_cmdmd600_1[0]; //0x06
						  transbuf[i++] = yk_yb_cmdmd600_1[1];					
						  transbuf[i++] = yk_yb_cmdmd600_1[2]+ykinf;
						  transbuf[i++] = yk_yb_cmdmd600_1[3];
						  transbuf[i++] = ykdcc;
						  uart_status[uartportflg].uartdevice[devindex].uart_ykflg = 0;
              yk_data_buf7[devindex][0] = 0x55;	
              return i;							 
					    break;
					
					
				}
			
			break;
				
	  }
	  
		

	

}



uint8_t read_yc_modbus_1(uint8_t uartportflg,uint8_t devindex,uint8_t *transbuf)
{
	  uint8_t i = 0;
	  transbuf[i++] = uartpara[uartportflg].devaddr[devindex];
	  transbuf[i++] = read_yc_buf1[0]&0xff;
	  transbuf[i++] = (read_yc_buf1[1]>>8)&0xff;
    transbuf[i++] = read_yc_buf1[1]&0xff;
	  transbuf[i++] = 0;
	  transbuf[i++] = read_yc_buf1[2]&0xff;
	  //transbuf[i++] = uartpara[uartportflg].deviceconf[devindex].ycnum;
	  return i;
}
//uint16_t trans_ym_buf1[3] = {0x0004, 0x2100 ,0x04};
uint8_t read_ym_modbus_1(uint8_t uartportflg,uint8_t devindex,uint8_t *transbuf)
{
	  uint8_t i = 0;
	  transbuf[i++] = uartpara[uartportflg].devaddr[devindex];
	  transbuf[i++] = trans_ym_buf1[0]&0xff;
	  transbuf[i++] = (trans_ym_buf1[1]>>8)&0xff;
    transbuf[i++] = trans_ym_buf1[1]&0xff;
	  transbuf[i++] = 0;
	  transbuf[i++] = trans_ym_buf1[2]&0xff;
	  //transbuf[i++] = uartpara[uartportflg].deviceconf[devindex].ycnum;
	  return i;
}

uint8_t read_yx_modbus_1(uint8_t uartportflg,uint8_t devindex,uint8_t *transbuf)
{
	  uint8_t i = 0;
	  transbuf[i++] = uartpara[uartportflg].devaddr[devindex];
	  transbuf[i++] = read_yx_buf1[0];	
	  transbuf[i++] = (read_yx_buf1[1]>>8)&0xff;
    transbuf[i++] = read_yx_buf1[1]&0xff;
	  transbuf[i++] = 0;
	  transbuf[i++] = read_yx_buf1[2]&0xff;

	
	  return i;	
}





uint8_t  Deal_Comm_Proce_1(rt_device_t dev)//,uint8_t *tx_buf,uint8_t *rx_buf)//modbus WGB-611
{
	
	
	  uint16_t Check_Code=0;
//    uint8_t tx_number;
    uint8_t uartportflg;  
	  uint8_t data_num;
	  uint8_t i;
	
	  struct uart_device* uart;
 

  	uart = (struct uart_device*)dev->user_data;
	
	  if(uart == &uart0)
		{
			uartportflg = 0;
		}
		else if(uart == &uart1)
    {
			uartportflg = 1;
		}
		else if(uart == &uart3)
    {
			uartportflg = 3;
		}
		else if(uart == &uart4)
    {
			uartportflg = 4;
		}    
		else if(uart == &uart5)
    {
			uartportflg = 5;
		}    
		else if(uart == &uart6)
    {
			uartportflg = 6;
		}
		else if(uart == &uart7)
    {
			uartportflg = 7;
		}
		else
		{
			return FALSE;
		}


    if(uart_status[uartportflg].uartdevice[uart_status[uartportflg].reddevindex].uart_RxOkflg == 1)
		{
				uart_status[uartportflg].uartdevice[uart_status[uartportflg].reddevindex].uart_RxOkflg = 0;
				uart_status[uartportflg].reddevindex++;
		}
		else
		{
		    uart_status[uartportflg].uartdevice[uart_status[uartportflg].reddevindex].uart_RxErrcount++;
				  
				if(uart_status[uartportflg].uartdevice[uart_status[uartportflg].reddevindex].uart_TxErrflg == 1)
				{
					  uart_status[uartportflg].uartdevice[uart_status[uartportflg].reddevindex].uart_devonline = 0;
					  uart_status[uartportflg].reddevindex++;
						
				}
				else
				{
            if(uart_status[uartportflg].uartdevice[uart_status[uartportflg].reddevindex].uart_RxErrcount > 2)
						{
							   uart_status[uartportflg].uartdevice[uart_status[uartportflg].reddevindex].uart_devonline = 0;
								 uart_status[uartportflg].uartdevice[uart_status[uartportflg].reddevindex].uart_TxErrflg = 1;
								 uart_status[uartportflg].uartdevice[uart_status[uartportflg].reddevindex].uart_RxErrcount = 0;
								 uart_status[uartportflg].reddevindex++;
							   
						}
				}
					
				 
    }
			
    if(uart_status[uartportflg].reddevindex >= uartpara[uartportflg].devusednum )
	  {
		    uart_status[uartportflg].reddevindex = 0;
				//uart_status[uartportflg].uart_redycyxflg ^= 1;
				uart_status[uartportflg].uart_redycyxflg++;	
        if(uart_status[uartportflg].uart_redycyxflg > 2)
        {
					uart_status[uartportflg].uart_redycyxflg = 0;
				}	
							
		}

    //查出有操作的对象
		
		
    if(uart_status[uartportflg].uart_rtdbflg != 0)//有远方操作
    {
		    for(i=0;i<DEVICEMAXNUM;i++)
			  {
				    if(uart_status[uartportflg].uartdevice[i].uart_ykflg == 1)
            {
			          uart_status[uartportflg].reddevindex = i;
			          data_num = trans_yk_cmd_modbus_1(uartportflg,uart_status[uartportflg].reddevindex,uart->int_tx->tx_buffer);							
						    uart_status[uartportflg].uartdevice[i].uart_ykflg	= 0;//清除遥控标志
							  break;
						}							

			  }
				if(uart_status[uartportflg].uart_rtdbflg >= 1)
				    uart_status[uartportflg].uart_rtdbflg--;
        
				


			
    }	
    else
    {
		    
        if(uart_status[uartportflg].uart_redycyxflg == 0)// read yc data
				{
				   
				    data_num = read_yc_modbus_1(uartportflg,uart_status[uartportflg].reddevindex,uart->int_tx->tx_buffer);
				
				}
				else if(uart_status[uartportflg].uart_redycyxflg == 1)//read yx data
				{
				    data_num = read_yx_modbus_1(uartportflg,uart_status[uartportflg].reddevindex,uart->int_tx->tx_buffer);
				}
				else if(uart_status[uartportflg].uart_redycyxflg == 2)//read ym data
        {
					  data_num = read_ym_modbus_1(uartportflg,uart_status[uartportflg].reddevindex,uart->int_tx->tx_buffer);
				}
			
			
			
			
		}	
	      if(data_num == FALSE)//data error
				{
					  return FALSE;
				}
			  Check_Code = crc16(uart->int_tx->tx_buffer,data_num);
				uart->int_tx->tx_buffer[data_num+1] = Check_Code&0xff;
				uart->int_tx->tx_buffer[data_num] = (Check_Code>>8)&0xff;
				
			if(rt_device_write(dev, 0, uart->int_tx->tx_buffer, data_num+1)==0)
      {
         uart_status[uartportflg].uartdevice[uart_status[uartportflg].reddevindex].uart_TxErrcount++;
         return FALSE;				
			}	
			if(uarttxflg != 0)
			{
         uart_status[uartportflg].uart_trans_buf[0] = data_num+2;			
			   datacopy_8(&uart_status[uartportflg].uart_trans_buf[1],uart->int_tx->tx_buffer,data_num+2);
			   uart_status[uartportflg].uart_trans_flg = 1;				
			}

		return TRUE;
}


void Receive_Commdata_Proce_1(rt_device_t dev)//,uint8_t *tx_buf,uint8_t *rx_buf,uint8_t uartportflg)
{
	  uint8_t uartportflg;
	  uint16_t Check_Code=0;
	  uint8_t receive_number,i;
//	  uint8_t *ptr1;
    struct uart_device* uart;
//	  uint8_t curr_addr;
	 // uint8_t uartportflg;  
	  
	
    uart = (struct uart_device*)dev->user_data;	
	  
    
	  if(uart->int_rx->rx_buffer[1] == 0x05)//遥控
		{
				  receive_number = 8;
		}
		else if((uart->int_rx->rx_buffer[1]&0x80)==0x80)// error data
		{
			   receive_number = 5;
		}
		else
		{
			receive_number = uart->int_rx->rx_buffer[2] + 5;
		}
	  
	 // receive_number = uart->int_rx->rx_buffer[2] + 5;
	  
	  if(uart->int_rx->rx_length < receive_number)
		{
			  return;
		}
		
		if(uart == &uart0)
		{
			uartportflg = 0;
		}
		else if(uart == &uart1)
    {
			uartportflg = 1;
		}
		else if(uart == &uart3)
    {
			uartportflg = 3;
		}
		else if(uart == &uart4)
    {
			uartportflg = 4;
		}    
		else if(uart == &uart5)
    {
			uartportflg = 5;
		}    
		else if(uart == &uart6)
    {
			uartportflg = 6;
		}
		else if(uart == &uart7)
    {
			uartportflg = 7;
		}
		else
		{
			return ;
		}
		
		uart_status[uartportflg].uartdevice[uart_status[uartportflg].reddevindex].uart_devonline = 1;		
		if(uartrxflg != 0)
		{
		    uart_status[uartportflg].uart_recv_buf[0] = receive_number;
		    datacopy_8(&uart_status[uartportflg].uart_recv_buf[1],uart->int_rx->rx_buffer,receive_number);
		    uart_status[uartportflg].uart_recv_flg = 1;			
		}


		
		if(uart->int_rx->rx_buffer[1] < 0x05)//receive soem yc yx data
		{
		    Check_Code = crc16(uart->int_rx->rx_buffer,receive_number-2);
			  if((uart->int_rx->rx_buffer[receive_number-1] == (Check_Code&0xff))
			    &&(uart->int_rx->rx_buffer[receive_number-2] == (Check_Code>>8)))
				{
					  //curr_addr = uart->int_rx->rx_buffer[0];
					  if(uart->int_rx->rx_buffer[1] ==0x04)
						{
							
                if(uart_status[uartportflg].uart_redycyxflg == 0)//yc
							  {
                    switch(uartportflg)
							      {
									  
			             	    case 0:
											
										    yc_data_buf0[uart_status[uartportflg].reddevindex][0] = 0x7f;// true data flag
										    yc_data_buf0[uart_status[uartportflg].reddevindex][1] = uart->int_rx->rx_buffer[0];//data  source addr
										    yc_data_buf0[uart_status[uartportflg].reddevindex][2] = uart->int_rx->rx_buffer[2]>>1;//data number
											  for(i=0;i<yc_data_buf0[uart_status[uartportflg].reddevindex][2];i++)
									      {
													  yc_data_buf0[uart_status[uartportflg].reddevindex][i+3] = (uart->int_rx->rx_buffer[2*i+3]<<8)|
													                                                             (uart->int_rx->rx_buffer[2*i+4]);
												}
											  break;
			             	    case 1:
											
										    yc_data_buf1[uart_status[uartportflg].reddevindex][0] = 0x7f;// true data flag
										    yc_data_buf1[uart_status[uartportflg].reddevindex][1] = uart->int_rx->rx_buffer[0];//data  source addr
										    yc_data_buf1[uart_status[uartportflg].reddevindex][2] = uart->int_rx->rx_buffer[2]>>1;//data number
											  for(i=0;i<yc_data_buf1[uartportflg][2];i++)
									      {
													  yc_data_buf1[uart_status[uart_status[uartportflg].reddevindex].reddevindex][i+3] = (uart->int_rx->rx_buffer[2*i+3]<<8)|
													                                                             (uart->int_rx->rx_buffer[2*i+4]);
												}
											  break;												
			             	    case 3:
											
										    yc_data_buf3[uart_status[uartportflg].reddevindex][0] = 0x7f;// true data flag
										    yc_data_buf3[uart_status[uartportflg].reddevindex][1] = uart->int_rx->rx_buffer[0];//data  source addr
										    yc_data_buf3[uart_status[uartportflg].reddevindex][2] = uart->int_rx->rx_buffer[2]>>1;//data number
											  for(i=0;i<yc_data_buf3[uartportflg][2];i++)
									      {
													  yc_data_buf3[uart_status[uart_status[uartportflg].reddevindex].reddevindex][i+3] = (uart->int_rx->rx_buffer[2*i+3]<<8)|
													                                                             (uart->int_rx->rx_buffer[2*i+4]);
												}
											  break;												
			             	    case 4:
											
										    yc_data_buf4[uart_status[uartportflg].reddevindex][0] = 0x7f;// true data flag
										    yc_data_buf4[uart_status[uartportflg].reddevindex][1] = uart->int_rx->rx_buffer[0];//data  source addr
										    yc_data_buf4[uart_status[uartportflg].reddevindex][2] = uart->int_rx->rx_buffer[2]>>1;//data number
											  for(i=0;i<yc_data_buf4[uartportflg][2];i++)
									      {
													  yc_data_buf4[uart_status[uart_status[uartportflg].reddevindex].reddevindex][i+3] = (uart->int_rx->rx_buffer[2*i+3]<<8)|
													                                                             (uart->int_rx->rx_buffer[2*i+4]);
												}
											  break;												
			             	    case 5:
											
										    yc_data_buf5[uart_status[uartportflg].reddevindex][0] = 0x7f;// true data flag
										    yc_data_buf5[uart_status[uartportflg].reddevindex][1] = uart->int_rx->rx_buffer[0];//data  source addr
										    yc_data_buf5[uart_status[uartportflg].reddevindex][2] = uart->int_rx->rx_buffer[2]>>1;//data number
											  for(i=0;i<yc_data_buf5[uartportflg][2];i++)
									      {
													  yc_data_buf5[uart_status[uart_status[uartportflg].reddevindex].reddevindex][i+3] = (uart->int_rx->rx_buffer[2*i+3]<<8)|
													                                                             (uart->int_rx->rx_buffer[2*i+4]);
												}
											  break;												
			                 	case 6:
											
										    yc_data_buf6[uart_status[uartportflg].reddevindex][0] = 0x7f;// true data flag
										    yc_data_buf6[uart_status[uartportflg].reddevindex][1] = uart->int_rx->rx_buffer[0];//data  source addr
										    yc_data_buf6[uart_status[uartportflg].reddevindex][2] = uart->int_rx->rx_buffer[2]>>1;//data number
											  for(i=0;i<yc_data_buf6[uartportflg][2];i++)
									      {
													  yc_data_buf6[uart_status[uart_status[uartportflg].reddevindex].reddevindex][i+3] = (uart->int_rx->rx_buffer[2*i+3]<<8)|
													                                                             (uart->int_rx->rx_buffer[2*i+4]);
												}
											  break;												
			             	    case 7:
											
										    yc_data_buf7[uart_status[uartportflg].reddevindex][0] = 0x7f;// true data flag
										    yc_data_buf7[uart_status[uartportflg].reddevindex][1] = uart->int_rx->rx_buffer[0];//data  source addr
										    yc_data_buf7[uart_status[uartportflg].reddevindex][2] = uart->int_rx->rx_buffer[2]>>1;//data number
											  for(i=0;i<yc_data_buf7[uart_status[uartportflg].reddevindex][2];i++)
									      {
													  yc_data_buf7[uart_status[uartportflg].reddevindex][i+3] = (uart->int_rx->rx_buffer[2*i+3]<<8)|
													                                                             (uart->int_rx->rx_buffer[2*i+4]);
												}
											  break;	
												default:
													break;
												
								    }											
                }
						    else if(uart_status[uartportflg].uart_redycyxflg == 2)
								{
                    switch(uartportflg)
							      {
									  
			             	    case 0:
											
										    ym_data_buf0[uart_status[uartportflg].reddevindex][0] = 0x7f;// true data flag
										    ym_data_buf0[uart_status[uartportflg].reddevindex][1] = uart->int_rx->rx_buffer[0];//data  source addr
										    ym_data_buf0[uart_status[uartportflg].reddevindex][2] = uart->int_rx->rx_buffer[2]>>1;//data number
											  for(i=0;i<ym_data_buf0[uart_status[uartportflg].reddevindex][2];i++)
									      {
													  ym_data_buf0[uart_status[uartportflg].reddevindex][i+3] = (uart->int_rx->rx_buffer[2*i+3]<<8)|
													                                                             (uart->int_rx->rx_buffer[2*i+4]);
												}
											  break;
			             	    case 1:
											
										    ym_data_buf1[uart_status[uartportflg].reddevindex][0] = 0x7f;// true data flag
										    ym_data_buf1[uart_status[uartportflg].reddevindex][1] = uart->int_rx->rx_buffer[0];//data  source addr
										    ym_data_buf1[uart_status[uartportflg].reddevindex][2] = uart->int_rx->rx_buffer[2]>>1;//data number
											  for(i=0;i<ym_data_buf1[uartportflg][2];i++)
									      {
													  ym_data_buf1[uart_status[uart_status[uartportflg].reddevindex].reddevindex][i+3] = (uart->int_rx->rx_buffer[2*i+3]<<8)|
													                                                             (uart->int_rx->rx_buffer[2*i+4]);
												}
											  break;												
			             	    case 3:
											
										    ym_data_buf3[uart_status[uartportflg].reddevindex][0] = 0x7f;// true data flag
										    ym_data_buf3[uart_status[uartportflg].reddevindex][1] = uart->int_rx->rx_buffer[0];//data  source addr
										    ym_data_buf3[uart_status[uartportflg].reddevindex][2] = uart->int_rx->rx_buffer[2]>>1;//data number
											  for(i=0;i<ym_data_buf3[uartportflg][2];i++)
									      {
													  ym_data_buf3[uart_status[uart_status[uartportflg].reddevindex].reddevindex][i+3] = (uart->int_rx->rx_buffer[2*i+3]<<8)|
													                                                             (uart->int_rx->rx_buffer[2*i+4]);
												}
											  break;												
			             	    case 4:
											
										    ym_data_buf4[uart_status[uartportflg].reddevindex][0] = 0x7f;// true data flag
										    ym_data_buf4[uart_status[uartportflg].reddevindex][1] = uart->int_rx->rx_buffer[0];//data  source addr
										    ym_data_buf4[uart_status[uartportflg].reddevindex][2] = uart->int_rx->rx_buffer[2]>>1;//data number
											  for(i=0;i<ym_data_buf4[uartportflg][2];i++)
									      {
													  ym_data_buf4[uart_status[uart_status[uartportflg].reddevindex].reddevindex][i+3] = (uart->int_rx->rx_buffer[2*i+3]<<8)|
													                                                             (uart->int_rx->rx_buffer[2*i+4]);
												}
											  break;												
			             	    case 5:
											
										    ym_data_buf5[uart_status[uartportflg].reddevindex][0] = 0x7f;// true data flag
										    ym_data_buf5[uart_status[uartportflg].reddevindex][1] = uart->int_rx->rx_buffer[0];//data  source addr
										    ym_data_buf5[uart_status[uartportflg].reddevindex][2] = uart->int_rx->rx_buffer[2]>>1;//data number
											  for(i=0;i<ym_data_buf5[uartportflg][2];i++)
									      {
													  ym_data_buf5[uart_status[uart_status[uartportflg].reddevindex].reddevindex][i+3] = (uart->int_rx->rx_buffer[2*i+3]<<8)|
													                                                             (uart->int_rx->rx_buffer[2*i+4]);
												}
											  break;												
			                 	case 6:
											
										    ym_data_buf6[uart_status[uartportflg].reddevindex][0] = 0x7f;// true data flag
										    ym_data_buf6[uart_status[uartportflg].reddevindex][1] = uart->int_rx->rx_buffer[0];//data  source addr
										    ym_data_buf6[uart_status[uartportflg].reddevindex][2] = uart->int_rx->rx_buffer[2]>>1;//data number
											  for(i=0;i<ym_data_buf6[uartportflg][2];i++)
									      {
													  ym_data_buf6[uart_status[uart_status[uartportflg].reddevindex].reddevindex][i+3] = (uart->int_rx->rx_buffer[2*i+3]<<8)|
													                                                             (uart->int_rx->rx_buffer[2*i+4]);
												}
											  break;												
			             	    case 7:
											
										    ym_data_buf7[uart_status[uartportflg].reddevindex][0] = 0x7f;// true data flag
										    ym_data_buf7[uart_status[uartportflg].reddevindex][1] = uart->int_rx->rx_buffer[0];//data  source addr
										    ym_data_buf7[uart_status[uartportflg].reddevindex][2] = uart->int_rx->rx_buffer[2]>>1;//data number
											  for(i=0;i<ym_data_buf7[uart_status[uartportflg].reddevindex][2];i++)
									      {
													  ym_data_buf7[uart_status[uartportflg].reddevindex][i+3] = (uart->int_rx->rx_buffer[2*i+3]<<8)|
													                                                             (uart->int_rx->rx_buffer[2*i+4]);
												}
											  break;	
												default:
													break;
												
								    }												
                }							
																			
						}
						else if(uart->int_rx->rx_buffer[1] == 0x02)
						{
							  switch(uartportflg)
								{
									case 0:
									    if((yx_chgdata_buf0[uart_status[uartportflg].reddevindex][0] == 0x55)||
											(yx_chgdata_buf0[uart_status[uartportflg].reddevindex][0] == 0x88))
											{
												  datacopy(&yx_chgdata_buf0[uart_status[uartportflg].reddevindex][1],yx_data_buf0[uart_status[uartportflg].reddevindex]
												           ,(yx_data_buf0[uart_status[uartportflg].reddevindex][1]+2));
											    yx_chgdata_buf0[uart_status[uartportflg].reddevindex][0] = 0x88;
											}
											
											yx_data_buf0[uart_status[uartportflg].reddevindex][0] = uart->int_rx->rx_buffer[0];
											yx_data_buf0[uart_status[uartportflg].reddevindex][1] = uart->int_rx->rx_buffer[2]*8;//yx number
									    //取出遥信数据，按照字节存放
											for(i=0;i<uart->int_rx->rx_buffer[2];i++)
											{
												  yx_data_buf0[uart_status[uartportflg].reddevindex][i*8+2] = uart->int_rx->rx_buffer[i+3]&0x0001;
											    yx_data_buf0[uart_status[uartportflg].reddevindex][i*8+3] = (uart->int_rx->rx_buffer[i+3]>>1)&0x0001;
											    yx_data_buf0[uart_status[uartportflg].reddevindex][i*8+4] = (uart->int_rx->rx_buffer[i+3]>>2)&0x0001;											
											    yx_data_buf0[uart_status[uartportflg].reddevindex][i*8+5] = (uart->int_rx->rx_buffer[i+3]>>3)&0x0001;
											    yx_data_buf0[uart_status[uartportflg].reddevindex][i*8+6] = (uart->int_rx->rx_buffer[i+3]>>4)&0x0001;											
													yx_data_buf0[uart_status[uartportflg].reddevindex][i*8+7] = (uart->int_rx->rx_buffer[i+3]>>5)&0x0001;
											    yx_data_buf0[uart_status[uartportflg].reddevindex][i*8+8] = (uart->int_rx->rx_buffer[i+3]>>6)&0x0001;						
											    yx_data_buf0[uart_status[uartportflg].reddevindex][i*8+9] = (uart->int_rx->rx_buffer[i+3]>>7)&0x0001;
												
											
											}
											if((yx_chgdata_buf0[uart_status[uartportflg].reddevindex][0] != 0x55)
												  &&(yx_chgdata_buf0[uart_status[uartportflg].reddevindex][0] != 0x88))
											{
												  datacopy(&yx_chgdata_buf0[uart_status[uartportflg].reddevindex][1],yx_data_buf0[uart_status[uartportflg].reddevindex]
												           ,yx_data_buf0[uart_status[uartportflg].reddevindex][1]+2);
												  yx_chgdata_buf0[uart_status[uartportflg].reddevindex][0] = 0x88;
												
											}
											else
											{
												  if(arry_compa(&yx_chgdata_buf0[uart_status[uartportflg].reddevindex][3],&yx_data_buf0[uart_status[uartportflg].reddevindex][2]
													  ,yx_data_buf0[uart_status[uartportflg].reddevindex][1]) == 1)
													{
														  yx_chgdata_buf0[uart_status[uartportflg].reddevindex][0] = 0x55;
														
													}
													else
													{
														  yx_chgdata_buf0[uart_status[uartportflg].reddevindex][0] = 0x88;
													}
											}
											
											
										break;

									case 1:
									    if((yx_chgdata_buf1[uart_status[uartportflg].reddevindex][0] == 0x55)||
											(yx_chgdata_buf1[uart_status[uartportflg].reddevindex][0] == 0x88))
											{
												  datacopy(&yx_chgdata_buf1[uart_status[uartportflg].reddevindex][1],yx_data_buf1[uart_status[uartportflg].reddevindex]
												           ,(yx_data_buf1[uart_status[uartportflg].reddevindex][1]+2));
											    yx_chgdata_buf1[uart_status[uartportflg].reddevindex][0] = 0x88;
											}
											
											yx_data_buf1[uart_status[uartportflg].reddevindex][0] = uart->int_rx->rx_buffer[0];
											yx_data_buf1[uart_status[uartportflg].reddevindex][1] = uart->int_rx->rx_buffer[2]*8;//yx number
									    //取出遥信数据，按照字节存放
											for(i=0;i<uart->int_rx->rx_buffer[2];i++)
											{
												  yx_data_buf1[uart_status[uartportflg].reddevindex][i*8+2] = uart->int_rx->rx_buffer[i+3]&0x0001;
											    yx_data_buf1[uart_status[uartportflg].reddevindex][i*8+3] = (uart->int_rx->rx_buffer[i+3]>>1)&0x0001;
											    yx_data_buf1[uart_status[uartportflg].reddevindex][i*8+4] = (uart->int_rx->rx_buffer[i+3]>>2)&0x0001;											
											    yx_data_buf1[uart_status[uartportflg].reddevindex][i*8+5] = (uart->int_rx->rx_buffer[i+3]>>3)&0x0001;
											    yx_data_buf1[uart_status[uartportflg].reddevindex][i*8+6] = (uart->int_rx->rx_buffer[i+3]>>4)&0x0001;											
													yx_data_buf1[uart_status[uartportflg].reddevindex][i*8+7] = (uart->int_rx->rx_buffer[i+3]>>5)&0x0001;
											    yx_data_buf1[uart_status[uartportflg].reddevindex][i*8+8] = (uart->int_rx->rx_buffer[i+3]>>6)&0x0001;						
											    yx_data_buf1[uart_status[uartportflg].reddevindex][i*8+9] = (uart->int_rx->rx_buffer[i+3]>>7)&0x0001;
												
											
											}
											if((yx_chgdata_buf1[uart_status[uartportflg].reddevindex][0] != 0x55)
												  &&(yx_chgdata_buf1[uart_status[uartportflg].reddevindex][0] != 0x88))
											{
												  datacopy(&yx_chgdata_buf1[uart_status[uartportflg].reddevindex][1],yx_data_buf1[uart_status[uartportflg].reddevindex]
												           ,yx_data_buf1[uart_status[uartportflg].reddevindex][1]+2);
												  yx_chgdata_buf1[uart_status[uartportflg].reddevindex][0] = 0x88;
												
											}
											else
											{
												  if(arry_compa(&yx_chgdata_buf1[uart_status[uartportflg].reddevindex][3],&yx_data_buf1[uart_status[uartportflg].reddevindex][2]
													  ,yx_data_buf1[uart_status[uartportflg].reddevindex][1]) == 1)
													{
														  yx_chgdata_buf1[uart_status[uartportflg].reddevindex][0] = 0x55;
														
													}
													else
													{
														  yx_chgdata_buf1[uart_status[uartportflg].reddevindex][0] = 0x88;
													}
											}
											
											
										break;										
										
									case 3:
									    if((yx_chgdata_buf3[uart_status[uartportflg].reddevindex][0] == 0x55)||
											(yx_chgdata_buf3[uart_status[uartportflg].reddevindex][0] == 0x88))
											{
												  datacopy(&yx_chgdata_buf3[uart_status[uartportflg].reddevindex][1],yx_data_buf3[uart_status[uartportflg].reddevindex]
												           ,(yx_data_buf3[uart_status[uartportflg].reddevindex][1]+2));
											    yx_chgdata_buf3[uart_status[uartportflg].reddevindex][0] = 0x88;
											}
											
											yx_data_buf3[uart_status[uartportflg].reddevindex][0] = uart->int_rx->rx_buffer[0];
											yx_data_buf3[uart_status[uartportflg].reddevindex][1] = uart->int_rx->rx_buffer[2]*8;//yx number
									    //取出遥信数据，按照字节存放
											for(i=0;i<uart->int_rx->rx_buffer[2];i++)
											{
												  yx_data_buf3[uart_status[uartportflg].reddevindex][i*8+2] = uart->int_rx->rx_buffer[i+3]&0x0001;
											    yx_data_buf3[uart_status[uartportflg].reddevindex][i*8+3] = (uart->int_rx->rx_buffer[i+3]>>1)&0x0001;
											    yx_data_buf3[uart_status[uartportflg].reddevindex][i*8+4] = (uart->int_rx->rx_buffer[i+3]>>2)&0x0001;											
											    yx_data_buf3[uart_status[uartportflg].reddevindex][i*8+5] = (uart->int_rx->rx_buffer[i+3]>>3)&0x0001;
											    yx_data_buf3[uart_status[uartportflg].reddevindex][i*8+6] = (uart->int_rx->rx_buffer[i+3]>>4)&0x0001;											
													yx_data_buf3[uart_status[uartportflg].reddevindex][i*8+7] = (uart->int_rx->rx_buffer[i+3]>>5)&0x0001;
											    yx_data_buf3[uart_status[uartportflg].reddevindex][i*8+8] = (uart->int_rx->rx_buffer[i+3]>>6)&0x0001;						
											    yx_data_buf3[uart_status[uartportflg].reddevindex][i*8+9] = (uart->int_rx->rx_buffer[i+3]>>7)&0x0001;
												
											
											}
											if((yx_chgdata_buf3[uart_status[uartportflg].reddevindex][0] != 0x55)
												  &&(yx_chgdata_buf3[uart_status[uartportflg].reddevindex][0] != 0x88))
											{
												  datacopy(&yx_chgdata_buf3[uart_status[uartportflg].reddevindex][1],yx_data_buf3[uart_status[uartportflg].reddevindex]
												           ,yx_data_buf3[uart_status[uartportflg].reddevindex][1]+2);
												  yx_chgdata_buf3[uart_status[uartportflg].reddevindex][0] = 0x88;
												
											}
											else
											{
												  if(arry_compa(&yx_chgdata_buf3[uart_status[uartportflg].reddevindex][3],&yx_data_buf3[uart_status[uartportflg].reddevindex][2]
													  ,yx_data_buf3[uart_status[uartportflg].reddevindex][1]) == 1)
													{
														  yx_chgdata_buf3[uart_status[uartportflg].reddevindex][0] = 0x55;
														
													}
													else
													{
														  yx_chgdata_buf3[uart_status[uartportflg].reddevindex][0] = 0x88;
													}
											}
											
											
										break;
										
									case 4:
									    if((yx_chgdata_buf4[uart_status[uartportflg].reddevindex][0] == 0x55)||
											(yx_chgdata_buf4[uart_status[uartportflg].reddevindex][0] == 0x88))
											{
												  datacopy(&yx_chgdata_buf4[uart_status[uartportflg].reddevindex][1],yx_data_buf4[uart_status[uartportflg].reddevindex]
												           ,(yx_data_buf4[uart_status[uartportflg].reddevindex][1]+2));
											    yx_chgdata_buf4[uart_status[uartportflg].reddevindex][0] = 0x88;
											}
											
											yx_data_buf4[uart_status[uartportflg].reddevindex][0] = uart->int_rx->rx_buffer[0];
											yx_data_buf4[uart_status[uartportflg].reddevindex][1] = uart->int_rx->rx_buffer[2]*8;//yx number
									    //取出遥信数据，按照字节存放
											for(i=0;i<uart->int_rx->rx_buffer[2];i++)
											{
												  yx_data_buf4[uart_status[uartportflg].reddevindex][i*8+2] = uart->int_rx->rx_buffer[i+3]&0x0001;
											    yx_data_buf4[uart_status[uartportflg].reddevindex][i*8+3] = (uart->int_rx->rx_buffer[i+3]>>1)&0x0001;
											    yx_data_buf4[uart_status[uartportflg].reddevindex][i*8+4] = (uart->int_rx->rx_buffer[i+3]>>2)&0x0001;											
											    yx_data_buf4[uart_status[uartportflg].reddevindex][i*8+5] = (uart->int_rx->rx_buffer[i+3]>>3)&0x0001;
											    yx_data_buf4[uart_status[uartportflg].reddevindex][i*8+6] = (uart->int_rx->rx_buffer[i+3]>>4)&0x0001;											
													yx_data_buf4[uart_status[uartportflg].reddevindex][i*8+7] = (uart->int_rx->rx_buffer[i+3]>>5)&0x0001;
											    yx_data_buf4[uart_status[uartportflg].reddevindex][i*8+8] = (uart->int_rx->rx_buffer[i+3]>>6)&0x0001;						
											    yx_data_buf4[uart_status[uartportflg].reddevindex][i*8+9] = (uart->int_rx->rx_buffer[i+3]>>7)&0x0001;
												
											
											}
											if((yx_chgdata_buf4[uart_status[uartportflg].reddevindex][0] != 0x55)
												  &&(yx_chgdata_buf4[uart_status[uartportflg].reddevindex][0] != 0x88))
											{
												  datacopy(&yx_chgdata_buf4[uart_status[uartportflg].reddevindex][1],yx_data_buf4[uart_status[uartportflg].reddevindex]
												           ,yx_data_buf4[uart_status[uartportflg].reddevindex][1]+2);
												  yx_chgdata_buf4[uart_status[uartportflg].reddevindex][0] = 0x88;
												
											}
											else
											{
												  if(arry_compa(&yx_chgdata_buf4[uart_status[uartportflg].reddevindex][3],&yx_data_buf4[uart_status[uartportflg].reddevindex][2]
													  ,yx_data_buf4[uart_status[uartportflg].reddevindex][1]) == 1)
													{
														  yx_chgdata_buf4[uart_status[uartportflg].reddevindex][0] = 0x55;
														
													}
													else
													{
														  yx_chgdata_buf4[uart_status[uartportflg].reddevindex][0] = 0x88;
													}
											}
											
											
										break;
										
									case 5:
									    if((yx_chgdata_buf5[uart_status[uartportflg].reddevindex][0] == 0x55)||
											(yx_chgdata_buf5[uart_status[uartportflg].reddevindex][0] == 0x88))
											{
												  datacopy(&yx_chgdata_buf5[uart_status[uartportflg].reddevindex][1],yx_data_buf5[uart_status[uartportflg].reddevindex]
												           ,(yx_data_buf5[uart_status[uartportflg].reddevindex][1]+2));
											    yx_chgdata_buf5[uart_status[uartportflg].reddevindex][0] = 0x88;
											}
											
											yx_data_buf5[uart_status[uartportflg].reddevindex][0] = uart->int_rx->rx_buffer[0];
											yx_data_buf5[uart_status[uartportflg].reddevindex][1] = uart->int_rx->rx_buffer[2]*8;//yx number
									    //取出遥信数据，按照字节存放
											for(i=0;i<uart->int_rx->rx_buffer[2];i++)
											{
												  yx_data_buf5[uart_status[uartportflg].reddevindex][i*8+2] = uart->int_rx->rx_buffer[i+3]&0x0001;
											    yx_data_buf5[uart_status[uartportflg].reddevindex][i*8+3] = (uart->int_rx->rx_buffer[i+3]>>1)&0x0001;
											    yx_data_buf5[uart_status[uartportflg].reddevindex][i*8+4] = (uart->int_rx->rx_buffer[i+3]>>2)&0x0001;											
											    yx_data_buf5[uart_status[uartportflg].reddevindex][i*8+5] = (uart->int_rx->rx_buffer[i+3]>>3)&0x0001;
											    yx_data_buf5[uart_status[uartportflg].reddevindex][i*8+6] = (uart->int_rx->rx_buffer[i+3]>>4)&0x0001;											
													yx_data_buf5[uart_status[uartportflg].reddevindex][i*8+7] = (uart->int_rx->rx_buffer[i+3]>>5)&0x0001;
											    yx_data_buf5[uart_status[uartportflg].reddevindex][i*8+8] = (uart->int_rx->rx_buffer[i+3]>>6)&0x0001;						
											    yx_data_buf5[uart_status[uartportflg].reddevindex][i*8+9] = (uart->int_rx->rx_buffer[i+3]>>7)&0x0001;
												
											
											}
											if((yx_chgdata_buf5[uart_status[uartportflg].reddevindex][0] != 0x55)
												  &&(yx_chgdata_buf5[uart_status[uartportflg].reddevindex][0] != 0x88))
											{
												  datacopy(&yx_chgdata_buf5[uart_status[uartportflg].reddevindex][1],yx_data_buf5[uart_status[uartportflg].reddevindex]
												           ,yx_data_buf5[uart_status[uartportflg].reddevindex][1]+2);
												  yx_chgdata_buf5[uart_status[uartportflg].reddevindex][0] = 0x88;
												
											}
											else
											{
												  if(arry_compa(&yx_chgdata_buf5[uart_status[uartportflg].reddevindex][3],&yx_data_buf5[uart_status[uartportflg].reddevindex][2]
													  ,yx_data_buf5[uart_status[uartportflg].reddevindex][1]) == 1)
													{
														  yx_chgdata_buf5[uart_status[uartportflg].reddevindex][0] = 0x55;
														
													}
													else
													{
														  yx_chgdata_buf5[uart_status[uartportflg].reddevindex][0] = 0x88;
													}
											}
											
											
										break;										
										
									case 6:
									    if((yx_chgdata_buf6[uart_status[uartportflg].reddevindex][0] == 0x55)||
											(yx_chgdata_buf6[uart_status[uartportflg].reddevindex][0] == 0x88))
											{
												  datacopy(&yx_chgdata_buf6[uart_status[uartportflg].reddevindex][1],yx_data_buf6[uart_status[uartportflg].reddevindex]
												           ,(yx_data_buf6[uart_status[uartportflg].reddevindex][1]+2));
											    yx_chgdata_buf6[uart_status[uartportflg].reddevindex][0] = 0x88;
											}
											
											yx_data_buf6[uart_status[uartportflg].reddevindex][0] = uart->int_rx->rx_buffer[0];
											yx_data_buf6[uart_status[uartportflg].reddevindex][1] = uart->int_rx->rx_buffer[2]*8;//yx number
									    //取出遥信数据，按照字节存放
											for(i=0;i<uart->int_rx->rx_buffer[2];i++)
											{
												  yx_data_buf6[uart_status[uartportflg].reddevindex][i*8+2] = uart->int_rx->rx_buffer[i+3]&0x0001;
											    yx_data_buf6[uart_status[uartportflg].reddevindex][i*8+3] = (uart->int_rx->rx_buffer[i+3]>>1)&0x0001;
											    yx_data_buf6[uart_status[uartportflg].reddevindex][i*8+4] = (uart->int_rx->rx_buffer[i+3]>>2)&0x0001;											
											    yx_data_buf6[uart_status[uartportflg].reddevindex][i*8+5] = (uart->int_rx->rx_buffer[i+3]>>3)&0x0001;
											    yx_data_buf6[uart_status[uartportflg].reddevindex][i*8+6] = (uart->int_rx->rx_buffer[i+3]>>4)&0x0001;											
													yx_data_buf6[uart_status[uartportflg].reddevindex][i*8+7] = (uart->int_rx->rx_buffer[i+3]>>5)&0x0001;
											    yx_data_buf6[uart_status[uartportflg].reddevindex][i*8+8] = (uart->int_rx->rx_buffer[i+3]>>6)&0x0001;						
											    yx_data_buf6[uart_status[uartportflg].reddevindex][i*8+9] = (uart->int_rx->rx_buffer[i+3]>>7)&0x0001;
												
											
											}
											if((yx_chgdata_buf6[uart_status[uartportflg].reddevindex][0] != 0x55)
												  &&(yx_chgdata_buf6[uart_status[uartportflg].reddevindex][0] != 0x88))
											{
												  datacopy(&yx_chgdata_buf6[uart_status[uartportflg].reddevindex][1],yx_data_buf6[uart_status[uartportflg].reddevindex]
												           ,yx_data_buf6[uart_status[uartportflg].reddevindex][1]+2);
												  yx_chgdata_buf6[uart_status[uartportflg].reddevindex][0] = 0x88;
												
											}
											else
											{
												  if(arry_compa(&yx_chgdata_buf6[uart_status[uartportflg].reddevindex][3],&yx_data_buf6[uart_status[uartportflg].reddevindex][2]
													  ,yx_data_buf6[uart_status[uartportflg].reddevindex][1]) == 1)
													{
														  yx_chgdata_buf6[uart_status[uartportflg].reddevindex][0] = 0x55;
														
													}
													else
													{
														  yx_chgdata_buf6[uart_status[uartportflg].reddevindex][0] = 0x88;
													}
											}
											
											
										break;

									case 7:
									    if((yx_chgdata_buf7[uart_status[uartportflg].reddevindex][0] == 0x55)||
											(yx_chgdata_buf7[uart_status[uartportflg].reddevindex][0] == 0x88))
											{
												  datacopy(&yx_chgdata_buf7[uart_status[uartportflg].reddevindex][1],yx_data_buf7[uart_status[uartportflg].reddevindex]
												           ,(yx_data_buf7[uart_status[uartportflg].reddevindex][1]+2));
											    yx_chgdata_buf7[uart_status[uartportflg].reddevindex][0] = 0x88;
											}
											
											yx_data_buf7[uart_status[uartportflg].reddevindex][0] = uart->int_rx->rx_buffer[0];
											yx_data_buf7[uart_status[uartportflg].reddevindex][1] = uart->int_rx->rx_buffer[2]*8;//yx number
									    //取出遥信数据，按照字节存放
											for(i=0;i<uart->int_rx->rx_buffer[2];i++)
											{
												  yx_data_buf7[uart_status[uartportflg].reddevindex][i*8+2] = uart->int_rx->rx_buffer[i+3]&0x0001;
											    yx_data_buf7[uart_status[uartportflg].reddevindex][i*8+3] = (uart->int_rx->rx_buffer[i+3]>>1)&0x0001;
											    yx_data_buf7[uart_status[uartportflg].reddevindex][i*8+4] = (uart->int_rx->rx_buffer[i+3]>>2)&0x0001;											
											    yx_data_buf7[uart_status[uartportflg].reddevindex][i*8+5] = (uart->int_rx->rx_buffer[i+3]>>3)&0x0001;
											    yx_data_buf7[uart_status[uartportflg].reddevindex][i*8+6] = (uart->int_rx->rx_buffer[i+3]>>4)&0x0001;											
													yx_data_buf7[uart_status[uartportflg].reddevindex][i*8+7] = (uart->int_rx->rx_buffer[i+3]>>5)&0x0001;
											    yx_data_buf7[uart_status[uartportflg].reddevindex][i*8+8] = (uart->int_rx->rx_buffer[i+3]>>6)&0x0001;						
											    yx_data_buf7[uart_status[uartportflg].reddevindex][i*8+9] = (uart->int_rx->rx_buffer[i+3]>>7)&0x0001;
												
											
											}
											if((yx_chgdata_buf7[uart_status[uartportflg].reddevindex][0] != 0x55)
												  &&(yx_chgdata_buf7[uart_status[uartportflg].reddevindex][0] != 0x88))
											{
												  datacopy(&yx_chgdata_buf7[uart_status[uartportflg].reddevindex][1],yx_data_buf7[uart_status[uartportflg].reddevindex]
												           ,yx_data_buf7[uart_status[uartportflg].reddevindex][1]+2);
												  yx_chgdata_buf7[uart_status[uartportflg].reddevindex][0] = 0x88;
												
											}
											else
											{
												  if(arry_compa(&yx_chgdata_buf7[uart_status[uartportflg].reddevindex][3],&yx_data_buf7[uart_status[uartportflg].reddevindex][2]
													  ,yx_data_buf7[uart_status[uartportflg].reddevindex][1]) == 1)
													{
														  yx_chgdata_buf7[uart_status[uartportflg].reddevindex][0] = 0x55;
														
													}
													else
													{
														  yx_chgdata_buf7[uart_status[uartportflg].reddevindex][0] = 0x88;
													}
											}
											
											
										break;										
										
										default:
											break;																				
										
																			
										
								}
							
							
						}
					
						
						uart->int_rx->receive_flg = 0xa5;//接收正确

           uart->int_rx->rx_length = 0;
           uart->int_rx->read_index = uart0.int_rx->save_index = 0;//每次接收均是从缓冲区第一个字节开始
           uart->int_rx->rx_end_tick = rt_tick_get();
           
					uart_status[uartportflg].uartdevice[uart_status[uartportflg].reddevindex].uart_RxOkflg = 1;
       		uart_status[uartportflg].uartdevice[uart_status[uartportflg].reddevindex].uart_nARecvcount++;
					uart_status[uartportflg].uartdevice[uart_status[uartportflg].reddevindex].uart_TxErrflg = 0;	
						
           //uart0para_buf[uart0devindex].uart_RxOkflg = 1;         	
           //uart0para_buf[uart0devindex].uart_nARecvcount++;
           //uart0para_buf[uart0devindex].uart_TxErrflg = 0;	
					  
						
				}
				else
        {
					
	          uart->int_rx->rx_length = 0;
            uart->int_rx->read_index = uart->int_rx->save_index = 0;//每次接收均是从缓冲区第一个字节开始
					
        }	
			
			
			
			
			
			
		}
		else if(uart->int_rx->rx_buffer[1] == 0x05)//control
	  {
	  	  	uart->int_rx->receive_flg = 0xa5;//接收正确

           uart->int_rx->rx_length = 0;
           uart->int_rx->read_index = uart0.int_rx->save_index = 0;//每次接收均是从缓冲区第一个字节开始
           uart->int_rx->rx_end_tick = rt_tick_get();
           
					uart_status[uartportflg].uartdevice[uart_status[uartportflg].reddevindex].uart_RxOkflg = 1;
       		uart_status[uartportflg].uartdevice[uart_status[uartportflg].reddevindex].uart_nARecvcount++;
					uart_status[uartportflg].uartdevice[uart_status[uartportflg].reddevindex].uart_TxErrflg = 0;	
	  }
	  else//receive error
		{
	          uart->int_rx->rx_length = 0;
            uart->int_rx->read_index = uart->int_rx->save_index = 0;//每次接收均是从缓冲区第一个字节开始		    
		}
	
	
	
	
	
	
}




//end