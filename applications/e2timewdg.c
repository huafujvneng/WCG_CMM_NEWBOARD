/*
 * File      : e2timewdg.c
 *
 * Change Logs:
 * Date           Author       Notes
 *2012-09-11      changyufeng  first version

 *2017-08-08      wanghusen   add i2c read write E2 
 */

#include "e2timewdg.h"

// E2 mem_addr is     1010   000x (read 1 write 0)                    
// E2 rtc wdg addr is 1101 000x(read 1 write 0)  

TDateTime TIME;

/**
 * ��ʱ�ӳ��� delay(75)~=10us
 *
 */
void delay(uint16_t temp)
{
  //uint16_t temp1;

  while(temp--);
  /*{
    temp1 = 1;
    while(temp1--);
  }*/
}

/********************************************************************************************************/
/*********************************************************/
/*I2C��ʼ���ӳ���
*********************************************************/
void I2C_Init(void)
{
	SCL_O=1;
	delay(10);
	SDA_O=1;
	delay(10);
}
/*********************************************************/
/*I2C�����ӳ���
*********************************************************/
void I2C_Start(void)
{
	//		SCL=0;SDA=1;SCL=1;SDA=0;SCL=0;
	//SCL_O = 0;
	//delay(10);
	SDA_DIR=1;
	SDA_O=1;	
	SCL_O=1;

	delay(10);
	SDA_O=0;
	delay(10);
	SCL_O=0;//ǯסI2C���ߣ�׼�����ͺͽ�������	

	
	
}
/*********************************************************
*I2Cֹͣ�ӳ���
*********************************************************/
void I2C_Stop(void)
{
	//	SDA=0;SCL=1;SDA=1;SCL=0;
	SDA_DIR=1;
	SDA_O=0;
	delay(10);
	SCL_O=1;
	delay(10);
	SDA_O=1;
	delay(10); 	
	
}
/*********************************************************/
/*I2Cдһ���ֽ��ӳ���
*********************************************************/
void I2C_Write_Byte(unsigned char data)
{

	uint16_t i,err_time=500;
	SDA_DIR=1;
	for (i=0;i<8;i++)
	{
		if((data&0x80)==0x80)
		SDA_O=1;
		else
		SDA_O=0;
		delay(10);
		SCL_O = 1;
		delay(10);
		SCL_O=0;        // һ��Ҫ��������ʱ�ӣ����ͷ�����
		data<<=1;              //��������һλ
	}
	SDA_DIR=0;//��CPU��SDA������Ϊ���տ�
	delay(10);
	SCL_O=1;
    delay(10);
	while(SDA_I==1)//���ȷ���ź���SDA=1����һֱ�ȴ�
    {
       if(err_time--==0)
         goto abcd;
    }

abcd:
	SCL_O=0;
	delay(10);
	SDA_DIR=1;
}
/*********************************************************/
/*I2C��һ���ֽ��ӳ���
*********************************************************/
rt_uint8_t I2C_Read_Byte(void)
{
	uint8_t read_byte=0;
	uint8_t i;

  SDA_DIR=0;//����SDAΪ����
	for (i=0;i<8;i++)
	{
		SCL_O=0;
		delay(10);
		SCL_O=1;//��ʱ����Ϊ�ߣ�ʹ��������������Ч
		read_byte<<=1;
		read_byte |= SDA_I;//������λ
		delay(10);
	}

	SCL_O=0;
	delay(5);
	SDA_DIR=1;
	return (read_byte);
}
/*********************************************************/
/*I2C����Ӧ���ź��ӳ���
*********************************************************/
void I2C_Receive_Ack(void)
{
	unsigned char i=0;
	SCL_O=1;
	delay(10);
	SDA_DIR=0;
	//SCL_DIR=1;
	while ((SDA_I==0x01)&&(i<255))
	{i++;}
	SCL_O=0;
	SDA_DIR=1;
	delay(10);
}
/*********************************************************/
/*I2C����Ӧ���ź��ӳ���
*********************************************************/
void I2C_Acknowledge_Ack(void)
{
	//	SDA=0;SCL=1;SCL=0;
	SDA_O=0;//��Ӧ���ź�
	delay(10);
	SCL_O=1;
	delay(10);
	SCL_O=0;
}
/*********************************************************/
/*I2C����Ӧ���ź��ӳ���
*********************************************************/
void I2C_No_Acknowledge_Ack(void)
{
	SDA_O=1;//����Ӧ���ź�
	delay(10);
	SCL_O=1;
	delay(10);
	SCL_O=0;
}

/*********************************************************/
/*I2C��n���ֽ��ӳ���
*********************************************************/
void I2C_Read_nbyte(unsigned char *buf1,unsigned int MemAddr,unsigned char length)
{
	
	unsigned char H_data,L_data,i;
	L_data=MemAddr&0xff;// ȡ��λ��ַ
	H_data=(MemAddr>>8)&0xff;// ȡ��λ��ַ
	I2C_Start();
	I2C_Write_Byte(0xa0);//start read E2,
	I2C_Write_Byte(H_data);
	
	I2C_Write_Byte(L_data);
	I2C_Start();
	I2C_Write_Byte(0xa1);
	for(i = 0;i < length-1; i++)
	{
		L_data=I2C_Read_Byte();
	  I2C_Acknowledge_Ack();
		*buf1++ = L_data; 
	}
	*buf1=I2C_Read_Byte();
	I2C_No_Acknowledge_Ack();
	I2C_Stop();
	
	
}
/*********************************************************/
/*****    ��E2дn���ֽڳ���    ****************************/
/*********************************************************/
void I2C_Write_nByte(unsigned int MemAddr,unsigned char *buf,unsigned char length)//addr buf length
{
	
	unsigned char i,H_data,L_data;
	L_data = MemAddr;              //ȡ��λ��ַ
	H_data = MemAddr/256;          //ȡ��λ��ַ
	I2C_Start();
	I2C_Write_Byte(0xa0);
	I2C_Write_Byte(H_data);
	I2C_Write_Byte(L_data);
	for(i=0;i<length;i++)
	{
	    I2C_Write_Byte(*buf++);
	}
	I2C_Stop();	
	

}

/*******************************************************************************************************************/

/*********************************************************/
/*****    ���Ź���ʼ������    ****************************/
/*********************************************************/
void Init_WatchDog(void)
{
	I2C_Start();
	I2C_Write_Byte(0xd0);//	
	I2C_Write_Byte(0x09);//
	I2C_Write_Byte(0x0a);	//kickdog
	I2C_Write_Byte(0x9e);	//kickdog time=3000ms
	I2C_Stop();

	I2C_Start();
	I2C_Write_Byte(0xd0);//		
	I2C_Write_Byte(0x0B);//���� ��λ��ֵ
	I2C_Write_Byte(0x03);	//��ֹд���� ��λ��ѹ4.4V
	I2C_Stop();

}
/*********************************************************/
/*****    �幷����    ************************************/
/*********************************************************/
void Kick_WatchDog(void)
{
	
	I2C_Start();
	I2C_Write_Byte(0xd0);//	
	I2C_Write_Byte(0x09);
	I2C_Write_Byte(0x0a);	//kickdog
	I2C_Stop();
}



/**
*****  ����ʱ��
*
*
**/
uint8_t time_write_flg;
void set_rtc(void)
{
    uint8_t i,timetemp[7];

	  if(time_write_flg)
	  {
        time_write_flg = 0;

        i = 0;
        timetemp[i++] = ((TIME.second+TIME.msec/500)/10 << 4) | ((TIME.second+TIME.msec/500)%10);
        timetemp[i++] = (TIME.minute/10 << 4) | (TIME.minute%10);
        timetemp[i++] = (TIME.hour/10 << 4) | (TIME.hour%10);
        timetemp[i++] = 0x00;//����δʹ��			
        timetemp[i++] = (TIME.day/10 << 4) | (TIME.day%10);

        timetemp[i++] = (TIME.month/10 << 4) | (TIME.month%10);
        timetemp[i++] = ((TIME.year/10 << 4) | (TIME.year%10));
       
        
        I2C_Start();
        I2C_Write_Byte(0xd0);
        I2C_Write_Byte(0x00);
        I2C_Write_Byte(0x02);
        I2C_Stop();
        I2C_Start();
        I2C_Write_Byte(0xd0);
        I2C_Write_Byte(0x02);
                
	
        for(i=0; i<7; i++)
	      {
		        I2C_Write_Byte(timetemp[i]);// Databytes
	      }
	      I2C_Stop();

        I2C_Start();
        I2C_Write_Byte(0xd0);
        I2C_Write_Byte(0x00);
        I2C_Write_Byte(0x00);
        I2C_Stop();	 
       
	}
}

#include <finsh.h>
FINSH_FUNCTION_EXPORT(set_rtc,irq response test);
/**
*  ��ȡװ��Ӳ��ʱ��
*
**/
//uint8_t timetemp[16];
void get_rtc(void)
{
	uint8_t i,timetemp[7];

	/*I2C_Start();
	I2C_Write_Byte(0xa2);
	I2C_Write_Byte(0x02);
  I2C_Start();    
	I2C_Write_Byte(0xa3);*/
	
	I2C_Start();
	I2C_Write_Byte(0xd0);//slave addr
	//I2C_Receive_Ack();
	//FM31256G_Start(0xd0);
	I2C_Write_Byte(0x00);//rtc addr
	//I2C_Receive_Ack();	
	I2C_Write_Byte(0x01);//read time
	//I2C_Receive_Ack();	
	I2C_Stop();
	
	I2C_Start();
	I2C_Write_Byte(0xd0);
	//I2C_Receive_Ack();		
	I2C_Write_Byte(0x01);
	//I2C_Receive_Ack();	
	I2C_Write_Byte(0x00);
	//I2C_Receive_Ack();		
	I2C_Stop();
	///FM31256G_Start(0xd0);    		 //Write Comand to ...
	I2C_Start();
	I2C_Write_Byte(0xd0);	
	//I2C_Receive_Ack();	
	I2C_Write_Byte(0x01);	
	//I2C_Receive_Ack();	
	I2C_Write_Byte(0x02);            //set address from where to read
	I2C_Stop();	
	//FM31256G_Continue(0xd1); 	         //Restart,with READ comand
	I2C_Start();
	I2C_Write_Byte(0xd1);	
	
	for(i=0; i<6; i++)
	 {
	   timetemp[i] = I2C_Read_Byte();

       I2C_Acknowledge_Ack();
     }
	timetemp[i] = I2C_Read_Byte();
	I2C_No_Acknowledge_Ack();
	I2C_Stop();

	i = 0;
	TIME.msec = 0;
	TIME.second = (((timetemp[i] >> 4)& 0x07) * 10) + (timetemp[i] & 0x0f);
	i++;
	TIME.minute = (((timetemp[i] >> 4)& 0x07) * 10) + (timetemp[i] & 0x0f);
	i++;
	TIME.hour = (((timetemp[i] >> 4)& 0x03) * 10) + (timetemp[i] & 0x0f);
	i++;
	i++;//����δʹ��
	TIME.day = (((timetemp[i] >> 4)& 0x03) * 10) + (timetemp[i] & 0x0f);		 
	i++;
	TIME.month = (((timetemp[i] >> 4)& 0x01) * 10) + (timetemp[i] & 0x0f);
	i++;
	TIME.year = ((timetemp[i] >> 4 ) * 10) + (timetemp[i] & 0x0f);
	if(TIME.year > 99)
	{
		TIME.year = 0;
	}
		I2C_Start();
	I2C_Write_Byte(0xd0);	
	//I2C_Receive_Ack();	
	//FM31256G_Start(0xd0);
	I2C_Write_Byte(0x00);
	//I2C_Receive_Ack();	
	I2C_Write_Byte(0x00);
	//I2C_Receive_Ack();	
	I2C_Stop();
		
}
extern uint16_t time_correct_mode;
extern struct rt_mailbox mb;
const uint8_t month_day_tab[] = {0,31,28,31,30,31,30,31,31,30,31,30,31};  //ÿ�µ�����

//* ϵͳ���ʱ�ӽ�λ
void system_time_increase(void)
{

	
    TIME.msec++;
    rjsz++;

	
    if(TIME.msec >= 1000)
    {
        TIME.msec = 0L;
        TIME.second++;
		

			
        if(TIME.second >= 60)
        {
					
        //set_rtc();//test
 /*for(i=0;i<60;i++)
 {
 	temp[i] = i;
 }
					I2C_Write_nbyte(0x003800,temp,60);*/
            TIME.second = 0;
            TIME.minute++;
            if(TIME.minute >= 60)
            {
                TIME.minute = 0;
                TIME.hour++;

                   
                if(TIME.hour >= 24)
                {
                    TIME.hour = 0;
                    TIME.day++;
                 //   day_fresh = 1;

                    if(TIME.day > (((TIME.year%4 == 0) && (TIME.month == 2))?
                                        month_day_tab[TIME.month]+1:month_day_tab[TIME.month]) )
                    {
                        TIME.day = 1;
                        TIME.month++;
                        if(TIME.month > 12)
                        {
                            TIME.month = 1;
                            TIME.year++;
                            if(TIME.year > 99)
                            {
                                TIME.year = 0;
                            }
                        }
                    }
                }
            }
        }
    }
}

void FM32156_init(void)
{

	

	//temp = ((temp|0x01)&0xfd);
	I2C_Start();
	I2C_Write_Byte(0xd0);
	//I2C_Acknowledge_Ack();
	I2C_Write_Byte(0x01);
	//I2C_Acknowledge_Ack();	
	I2C_Start();		
	I2C_Write_Byte(0x00);	
	//I2C_Acknowledge_Ack();	
	//temp = I2C_Read_Byte();	
	I2C_No_Acknowledge_Ack();
	I2C_Stop();

}

///* ��ȡ��ǰʱ��
TDateTime Now(void)
{
    TDateTime time = TIME;
    return time;
}



