/*
 * File      : application.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2009 - 2011, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2011-05-24     Bernard      the first version
 */

/**
 * @addtogroup FM3
 */
/*@{*/

#include <rtthread.h>
#include "board.h"
#include "eth_server103.h"
#include "uart.h"
#include "Applications/tcp_debug.h"

#ifdef RT_USING_DFS
/* dfs init */
#include <dfs_init.h>
/* dfs filesystem:ELM filesystem init */
#include <dfs_elm.h>
/* dfs Filesystem APIs */
#include <dfs_fs.h>
#endif

#ifdef RT_USING_LWIP
#include <lwip/sys.h>
#include <lwip/api.h>
#include <netif/ethernetif.h>
#endif /* RT_USING_LWIP */

void rt_init_thread_entry(void *parameter)
{
    /* Filesystem Initialization */
#ifdef RT_USING_DFS
    {
        /* init the device filesystem */
        dfs_init();

#ifdef RT_USING_DFS_ELMFAT
        /* init the elm chan FatFs filesystam*/
        elm_init();

        /* mount ELM FatFs on NAND flash as root directory */
        if (dfs_mount("sd0", "/", "elm", 0, 0) == 0)
        {
            rt_kprintf("File System initialized!\n");
        }
        else
            rt_kprintf("File System initialzation failed!\n");
#endif
    }
#endif

    /* LwIP Initialization */
#ifdef RT_USING_LWIP     //wzw  #if  0   //
    {
        extern void lwip_sys_init(void);
        extern void fm3_emac_hw_init(void);
        extern void set_ethernet_e_cout_clock(int is_rmii);

        eth_system_device_init();

        {
            /* RXDV0 MII_MODE_A: 1*///whs use phy0
            FM3_GPIO->DDRC  |=  (1<<5); /* set GPIO output */
            FM3_GPIO->PDORC &= ~(1<<5); /* output LOW. */
            FM3_GPIO->PDORC |=  (1<<5); /* output HIGH. */
            /* SNI_MODE_A: 0*/

            /* MII_MODE_B: 1*///whs use phy1
          //  FM3_GPIO->DDRC  |=  (1<<0); /* set GPIO output */
          //  FM3_GPIO->PDORC &= ~(1<<0); /* output LOW. */
          //  FM3_GPIO->PDORC |=  (1<<0); /* output HIGH. */

            /* SNI_MODE_B: 0*/
        }

        /**< config PHY0_RESET */
        /**< config PHY1_RESET */
        /* PHY RESET : PC9(xuji dual-PHY). */
        {
            FM3_GPIO->DDRC |=  (1<<9); /* set GPIO output */
            FM3_GPIO->PDORC &= ~(1<<9); /* output LOW. */
            rt_thread_delay(200);
            FM3_GPIO->PDORC |=  (1<<9); /* output HIGH. */
            rt_thread_delay(200);
        }

        /* xuji dual-PHY use ext RMII_REF_CLK. */
        /* set_ethernet_e_cout_clock(1); */

        /* register ethernetif device */
        fm3_emac_hw_init();

        /* init all device */
        rt_device_init_all();

        /* init lwip system */
        lwip_sys_init();
        rt_kprintf("TCP/IP initialized!\n");
    }

    /* set ip */
    {
        extern uint32_t eth_IP1,eth_MASK1,eth_GW1,eth_IP2,eth_MASK2,eth_GW2;
			  extern struct ETH_CONF ethpara;
        void set_if(char* netif_name, char* ip_addr, char* gw_addr, char* nm_addr);
        void set_if_h(char* netif_name, rt_uint32_t ip_addr, rt_uint32_t gw_addr, rt_uint32_t nm_addr);
        //set_if("e0", "11.100.100.2", "11.100.100.1", "255.255.255.0");//whs test
			eth_IP1 = ethpara.eth_IP1;
			eth_MASK1 = 0xffffff00;
      set_if_h("e0", eth_IP1, (eth_IP1&0xffffff00)+1, eth_MASK1);

			//set_if("e0", ethpara.eth_IP1, (ethpara.eth_IP1&0xffffff00)+1, eth_MASK1);//"255.255.255.0");//eth_MASK1);
       // set_if_h("e0", eth_IP1, eth_GW1, eth_MASK1
      
    }

    eth_server103();
    eth_server_debug();
#endif
}

void normal_routine(void);
//void time_correct_task(void);
int rt_application_init()
{
    rt_thread_t tid;

//	cpu_usage_init();


    tid = rt_thread_create("init",
                           rt_init_thread_entry, RT_NULL,
                           1024, 8, 20);
    if (tid != RT_NULL) rt_thread_startup(tid);


    normal_routine();
    //time_correct_task();
		// run_init();	
    communication_task();
    return 0;
}

/*@}*/
