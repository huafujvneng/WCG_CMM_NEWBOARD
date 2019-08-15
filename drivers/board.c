/*
 * File      : board.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2009 - 2011 RT-Thread Develop Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2011-02-24     Bernard      first implementation
 * 2012-11-01     mayunliang   apply to WGB device
 */

#include "board.h"

/**
 * @addtogroup FM3
 */

/*@{*/
extern void ports_init(void);

extern void rt_hw_dma_init(void);

extern void rtc_init(void);
extern void system_time_increase(void);
extern void run_init(void);
extern void Init_WatchDog(void);
extern void FM32156_init(void);
/**
 * This is the timer interrupt service routine.tick=1ms
 *
 */
void SysTick_Handler(void)
{
	/* enter interrupt */
	rt_interrupt_enter();

	rt_tick_increase();
    //increase clock
	system_time_increase();//系统软时钟走时


	/* leave interrupt */
	rt_interrupt_leave();
}


/**
* This function will initial FM3 Easy Kit board.
 */extern void delay(uint16_t temp);
void rt_hw_board_init()
{
    /* disable all analog input. */
    FM3_GPIO->ADE = 0;

	/* set priority group */
	NVIC_SetPriorityGrouping(5);

    /* init FM3 External Interrupt. */
    fm3_eint_init();

    /* init systick */
    SysTick_Config(SystemCoreClock/RT_TICK_PER_SECOND);
	NVIC_SetPriority(SysTick_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 3, 3));

	/* Initialize all for i/o port */
	ports_init();


    /*init fm31256*/
		  delay(1000);
		FM32156_init();
	/*init DMA */
    rt_hw_dma_init();


	 run_init();	


	/* initialize UART device */
//	rt_hw_serial_init();
	/* set console as UART device */
	//rt_console_set_device(CONSOLE_DEVICE);
  /*init the FM31256 watch dog*/

   Init_WatchDog();
	
}


/*@}*/
