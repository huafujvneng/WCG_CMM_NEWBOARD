/**
 * File    	:  rtc_init.c
 * author	:
 * This file is part of WGB-900
 * Copyright (C), 2012, XJ ELECTRIC Co., LTD.
 * Date     :  2012-11-02
 * Function	:  ��ʼ��RTCʹ�õĽӿں��жϺ���

 * Change Logs :
 * Date             Author           Notes
 * 2012-10-03                       the first version
*/

#include <rthw.h>
#include <rtthread.h>
#include <fm3_ext_int.h>

#include "board.h"
#include "i2c_rtc.h"


#define RTC_INT_NO    24// 13
//#define GPS_INT_NO    24//

extern TDateTime TIME;

uint16_t time_correct_mode;//1-pps,2-ppm,3-B.����ֵӲʱ��Уʱ��
uint32_t d_time_b;

uint8_t Bcode_byte[10];
uint8_t Bcode_byte_pst = 0;
uint8_t Bcode_bit = 0;
uint8_t Bcode_err_flg = 1;

struct rt_semaphore time_sem;

/**
**    �ⲿ�ж�INT13_1��P41 ��Ӳʱ�ӣ������ӳ���    24
**/
void rtc_isr(int vector)
{
    static uint32_t time_b;

    static uint8_t Bcode_bit_pst =0;

    switch(time_correct_mode)
    {
    case 1://pps
        d_time_b = rjsz - time_b;
        if(d_time_b > 900)//wzw   ������
        {
            if(TIME.msec > 499)//GPS��ʱ�������
            TIME.msec = 999;
            else
            TIME.msec = 0;
        }
        time_b = rjsz;
        break;

    case 2://ppm
        d_time_b = rjsz - time_b;
        if(d_time_b > 50000)//wzw   ������
        {
            if(TIME.second > 30)
            {
                TIME.second = 59;
                TIME.msec = 999;
            }
            else
            {
                TIME.second = 0;
                TIME.msec = 0;
            }
        }
        time_b = rjsz;
        break;

    case 3://B
        if((FM3_EXTI->ELVR1 & 0x00010000) == 0)//�������ж�
            time_b = rjsz;
        else//�½����ж�
        {
            d_time_b = rjsz - time_b;

            if((d_time_b >= 1) && (d_time_b <= 3))//��ʼʶ���� ���߲ο���Ԫ 8ms
            {
                if(Bcode_bit == 0x01) //�ο���־ ��������8ms
                {
                    TIME.msec = 10;//����10ms
                    Bcode_bit_pst = 0;
                    Bcode_byte_pst =0;
                    Bcode_err_flg = 0;
                }
                else
                {
                    Bcode_bit = 0x01;
                    Bcode_bit_pst = 0;
                    if(Bcode_byte_pst < 10)
                    {
                        Bcode_byte_pst++;    //λ����ԪP0-P9
                    }
                    else
                    {
                        Bcode_byte_pst = 0;
                        Bcode_err_flg = 1;
                    }
                }
            }
            else if((d_time_b >= 4) && (d_time_b <= 6)) //5
            {
                Bcode_byte[Bcode_byte_pst] |= Bcode_bit;
                Bcode_bit <<= 1;
                Bcode_bit_pst++;
            }
            else if((d_time_b >= 7) && (d_time_b <= 9)) //2
            {
                if(Bcode_bit_pst != 4)
                {
                    Bcode_byte[Bcode_byte_pst] &= ~Bcode_bit;
                    Bcode_bit <<= 1;
                }
                Bcode_bit_pst++;
            }
            else
            {
                //������
                time_b = rjsz;
                Bcode_err_flg = 1;
            }
        }
        FM3_EXTI->EICL = FM3_EXTI->EICL &~(1<<24);//���ⲿ�ж�Դ
        FM3_EXTI->ENIR = FM3_EXTI->ENIR &~(1<<24);//��ֹ�ж�
        FM3_EXTI->ELVR1 = FM3_EXTI->ELVR1 ^ (1<<16) ; //�ı䴥����
        FM3_EXTI->ENIR = FM3_EXTI->ENIR |(1<<24);//ʹ���ж�
        FM3_EXTI->EIRR = FM3_EXTI->EIRR &~(1<<24);//���ⲿ�ж�Դ
        if((Bcode_byte_pst > 5)&&(Bcode_err_flg == 0))
        {
            Bcode_err_flg = 1;
            rt_sem_release(&time_sem);
        }
        break;

    default://Ӳʱ�Ӷ�ʱ
        d_time_b = rjsz - time_b;
        if(d_time_b < 200000)//wzw   ������
        {
            time_b = rjsz;
            return;
        }

        if(TIME.msec > 499)//GPS��ʱ�������
        TIME.msec = 999;
        else
        TIME.msec = 0;

        break;

    }
}

/**
*  �ⲿ�жϳ�ʼ    INT24_1��P7A
*
*/
void time_correct_init(void)
{
    //return;//wzw
    FM3_GPIO->PFR7 = FM3_GPIO->PFR7 |(1<<10);
    FM3_GPIO->DDR7 = FM3_GPIO->DDR7 &~(1<<10);
    FM3_GPIO->EPFR15 = FM3_GPIO->EPFR15 & ~(1<<16) | (1<<17);

    FM3_EXTI->ENIR = FM3_EXTI->ENIR &~(1<<24);//��ֹ�ж�

    write_rtc_byte(0x01,0x00);  //�رն�ʱ�ж�
    /*�Ѳ�Ҫ�Ĺ��ܼĴ�������ɽ�ֹ*/
    write_rtc_byte(0x09,0x10);
    write_rtc_byte(0x0a,0x10);
    write_rtc_byte(0x0b,0x10);
    write_rtc_byte(0x0c,0x10);
    write_rtc_byte(0x0d,0x00);

    if(time_correct_mode == 3)//B���ʱ
    {
        FM3_EXTI->EICL = FM3_EXTI->EICL &~(1<<24);//���ⲿ�ж�Դ
        FM3_EXTI->ENIR = FM3_EXTI->ENIR &~(1<<24);//��ֹ�ж�
        FM3_EXTI->ELVR1 = FM3_EXTI->ELVR1 | (1<<17); //��������Ч
        FM3_EXTI->ENIR = FM3_EXTI->ENIR |(1<<24);//ʹ���ж�
        FM3_EXTI->EIRR = FM3_EXTI->EIRR &~(1<<24);//���ⲿ�ж�Դ


    }
    else
    {
        FM3_EXTI->EICL = FM3_EXTI->EICL &~(1<<24);//���ⲿ�ж�Դ
        FM3_EXTI->ENIR = FM3_EXTI->ENIR &~(1<<24);//��ֹ�ж�
        FM3_EXTI->ELVR1 = FM3_EXTI->ELVR1 |(1<<16)|(1<<17); //�½�����Ч
        FM3_EXTI->ENIR = FM3_EXTI->ENIR |(1<<24);//ʹ���ж�
        FM3_EXTI->EIRR = FM3_EXTI->EIRR &~(1<<24);//���ⲿ�ж�Դ


        if(time_correct_mode > 0)//pps ppmУʱ
        {
            //wzw
        }
        else//Ӳʱ�Ӷ�ʱ
        {

            /* enable rtc interrupt */

            write_rtc_byte(0x00,0x00);  //POR��Դ��λ����ʧЧ

            /*4���ж�1��*/
            write_rtc_byte(0x01,0x11);  //������ʱ�ж�
            write_rtc_byte(0x0e,0x82);  //��ʱ������Ч 1HZ
            write_rtc_byte(0x0f,240);  //������ֵ 240s
            //write_rtc_byte(0x0e,0x82);  //��ʱ������Ч 1HZ
            //write_rtc_byte(0x0f,10);  //������ֵ 240s  5s

        }

    }
    /* register handler */
    {
        rt_isr_handler_t func;

        func = (rt_isr_handler_t)rtc_isr;
        fm3_eint_install(RTC_INT_NO, func, RT_NULL);

    }


    fm3_eint_trigger_config(RTC_INT_NO, EINT_TRIGGER_FALLING); /* set trigger. */
    fm3_eint_enable(RTC_INT_NO);

    NVIC_SetPriority(EXINT8_31_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 1, 2));//�ж����ȼ�
    NVIC_EnableIRQ(EXINT8_31_IRQn);   //ʹ���ж�

    fm3_eint_pin_select(EINT24_1_P7A); /*EINT24_1_P7A select EINT source. */

}

/**
* This function will initial RTC and the related function.
*
*/
void rtc_init(void)
{
    /* ģ��I2C  ʱ����*/
    SDA_PFR = 0;
    SCL_PFR = 0;
    SDA_DDR = 1;
    SCL_DDR = 1;
    SDA_O = 1;
    SCL_O = 1;
}

extern const uint8_t month_day_tab[];  //ÿ�µ�����
void time_correct_b(void)
{
    uint8_t second_b;
    uint8_t minute_b;
    uint8_t hour_b;
    uint16_t day_b;
    uint16_t month_b;
    uint16_t year_b;

    if(Bcode_byte_pst == 6)//����һ��������ʱ����ж�ʱ����ʵʱ���Ѿ���λ���ˣ���Ϊ׼������8ms���ԡ�
    {
        second_b = (Bcode_byte[0]&0x0f) + ((Bcode_byte[0]>>4) &0x07)*10;
        minute_b = (Bcode_byte[1]&0x0f) + ((Bcode_byte[1]>>4) &0x07)*10;
        hour_b   = (Bcode_byte[2]&0x0f) + ((Bcode_byte[2]>>4) &0x03)*10;
        day_b    = (Bcode_byte[3]&0x0f) + ((Bcode_byte[3]>>4) &0x0f)*10 + (Bcode_byte[4]&0x03)*100;
        year_b   = (Bcode_byte[5]&0x0f) + ((Bcode_byte[5]>>4) &0x0f)*10;

        if((second_b<=60) || (minute_b<60) || (hour_b<24) || (day_b<=366))//�����ֵ���ж�ʱ
        {
            TIME.second = second_b;
            TIME.minute = minute_b;
            TIME.hour = hour_b;
            TIME.year = year_b;

            month_b = 1;
            while(day_b > (((TIME.year%4 == 0) && (month_b == 2))?
                            month_day_tab[month_b]+1:month_day_tab[month_b]))
            {
                day_b -= (((TIME.year%4 == 0) && (month_b == 2))?
                            month_day_tab[month_b]+1:month_day_tab[month_b]);
                month_b++;
            }
            TIME.day = day_b;
            TIME.month = month_b;
        }

    }
}

/*ALIGN(RT_ALIGN_SIZE)
static uint8_t time_correct_stack[200];
static struct rt_thread time_correct_thread;
void time_correct_thread_entry(void *parameter)
{

    rt_err_t result;

    time_correct_init();

    while(1)
    {

        result = rt_sem_take(&time_sem, (RT_TICK_PER_SECOND)*5);//
        if(result == -RT_ETIMEOUT)
        {
            if(time_correct_mode == 3)
                time_correct_init();
            else
                rt_thread_delay( 60 );//wzw ����B���ʱʱ������
        }
        else
        {
            time_correct_b();
        }
    }
}
*/

void time_correct_task(void)
{
   /* rt_err_t result;

    rt_sem_init(&time_sem, "time_sem", 0, RT_IPC_FLAG_FIFO);
    result = rt_thread_init(&time_correct_thread,
                            "time_correct",
                            time_correct_thread_entry, RT_NULL,
                            (uint8_t*)&time_correct_stack[0], sizeof(time_correct_stack), 8, 200);//wzw  ���ȼ�?
    if (result == RT_EOK)
    {
        rt_thread_startup(&time_correct_thread);
    }
*/
}


/*@}*/
