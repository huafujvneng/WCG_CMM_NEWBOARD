/*
 * File      : eth_driver.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2012, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2011-11-30     aozima       the first version.
 * 2011-12-10     aozima       support dual ethernet.
 * 2011-12-21     aozima       cleanup code.
 * 2012-07-13     aozima       mask all GMAC MMC Interrupt.
 * 2012-07-20     aozima       fixed mask all GMAC MMC Interrupt,and read clear.
 * 2012-07-20     aozima       use memcpy replace byte copy.
 * 2014-03-12     aozima       fixed re-config issue.
 * 2014-07-29     aozima       add more debug cmd.  
 */

#include <rtthread.h>

#include "lwipopts.h"
#include <netif/ethernetif.h>
#include <netif/etharp.h>
#include <lwip/icmp.h>
#include "lwipopts.h"

#include <board.h>
#include "fm3_emac.h"

/* FM3 ETH driver options */
#define RMII_MODE
#define CHECKSUM_BY_HARDWARE
#define USING_MAC0
//#define USING_MAC1//whs no use

#if !defined(RMII_MODE) && defined(USING_MAC1)
#error "Using both ch.0 and ch.1, RMII_MODE must be selected!"
#endif

#define EMAC_DEBUG
//#define EMAC_RX_DUMP
//#define EMAC_TX_DUMP

#ifdef EMAC_DEBUG
#define EMAC_TRACE	        rt_kprintf("[EMAC] ");rt_kprintf
#else
#define EMAC_TRACE(...)
#endif

#define EMAC_RXBUFNB        	4
#define EMAC_TXBUFNB        	2

#define EMAC_PHY_AUTO		    0
#define EMAC_PHY_10MBIT		    1
#define EMAC_PHY_100MBIT        2

#define MAX_ADDR_LEN 6
struct fm3_emac
{
    /* inherit from Ethernet device */
    struct eth_device parent;

    rt_uint32_t ETH_Mode;
    rt_uint32_t ETH_Speed;

    rt_uint8_t phy_addr;
    rt_uint8_t phy_int_no; /* PHY interrupt number. */
    rt_uint8_t phy_speed;

    /* interface address info. */
    rt_uint8_t  dev_addr[MAX_ADDR_LEN];		/* hw address	*/

    FM3_ETHERNET_MAC_TypeDef * FM3_ETHERNET_MAC;
    IRQn_Type ETHER_MAC_IRQ;

    ALIGN(RT_ALIGN_SIZE)
    EMAC_DMADESCTypeDef  DMARxDscrTab[EMAC_RXBUFNB], DMATxDscrTab[EMAC_TXBUFNB];
    ALIGN(RT_ALIGN_SIZE)
    rt_uint8_t Rx_Buff[EMAC_RXBUFNB][EMAC_MAX_PACKET_SIZE];
    ALIGN(RT_ALIGN_SIZE)
    rt_uint8_t Tx_Buff[EMAC_TXBUFNB][EMAC_MAX_PACKET_SIZE];

    EMAC_DMADESCTypeDef  *DMATxDescToSet;
    EMAC_DMADESCTypeDef  *DMARxDescToGet;

    struct rt_semaphore tx_buf_free;
    struct rt_mutex     lock;
};

#ifdef USING_MAC0
static struct fm3_emac fm3_emac_device0;
#endif /* #ifdef USING_MAC0 */

#ifdef USING_MAC1
static struct fm3_emac fm3_emac_device1;
#endif /* #ifdef USING_MAC1 */

/**
  * Initializes the DMA Tx descriptors in chain mode.
  */
static void EMAC_DMA_tx_desc_init(EMAC_DMADESCTypeDef *DMATxDescTab, uint8_t* TxBuff, uint32_t TxBuffCount)
{
    uint32_t i = 0;
    EMAC_DMADESCTypeDef *DMATxDesc;

    /* Fill each DMATxDesc descriptor with the right values */
    for(i=0; i < TxBuffCount; i++)
    {
        /* Get the pointer on the ith member of the Tx Desc list */
        DMATxDesc = DMATxDescTab + i;
        /* Set Second Address Chained bit */
        DMATxDesc->Status = EMAC_DMATxDesc_TCH;

        /* Set Buffer1 address pointer */
        DMATxDesc->Buffer1Addr = (uint32_t)(&TxBuff[i*EMAC_MAX_PACKET_SIZE]);

        /* Initialize the next descriptor with the Next Descriptor Polling Enable */
        if(i < (TxBuffCount-1))
        {
            /* Set next descriptor address register with next descriptor base address */
            DMATxDesc->Buffer2NextDescAddr = (uint32_t)(DMATxDescTab+i+1);
        }
        else
        {
            /* For last descriptor, set next descriptor address register equal to the first descriptor base address */
            DMATxDesc->Buffer2NextDescAddr = (uint32_t) DMATxDescTab;
        }
    }
}

/**
  * Initializes the DMA Rx descriptors in chain mode.
  */
static void EMAC_DMA_rx_desc_init(EMAC_DMADESCTypeDef *DMARxDescTab, uint8_t *RxBuff, uint32_t RxBuffCount)
{
    uint32_t i = 0;
    EMAC_DMADESCTypeDef *DMARxDesc;

    /* Fill each DMARxDesc descriptor with the right values */
    for(i=0; i < RxBuffCount; i++)
    {
        /* Get the pointer on the ith member of the Rx Desc list */
        DMARxDesc = DMARxDescTab+i;
        /* Set Own bit of the Rx descriptor Status */
        DMARxDesc->Status = EMAC_DMARxDesc_OWN;

        /* Set Buffer1 size and Second Address Chained bit */
        DMARxDesc->ControlBufferSize = EMAC_DMARxDesc_RCH | (uint32_t)EMAC_MAX_PACKET_SIZE;
        /* Set Buffer1 address pointer */
        DMARxDesc->Buffer1Addr = (uint32_t)(&RxBuff[i*EMAC_MAX_PACKET_SIZE]);

        /* Initialize the next descriptor with the Next Descriptor Polling Enable */
        if(i < (RxBuffCount-1))
        {
            /* Set next descriptor address register with next descriptor base address */
            DMARxDesc->Buffer2NextDescAddr = (uint32_t)(DMARxDescTab+i+1);
        }
        else
        {
            /* For last descriptor, set next descriptor address register equal to the first descriptor base address */
            DMARxDesc->Buffer2NextDescAddr = (uint32_t)(DMARxDescTab);
        }
    }
}

static rt_err_t fm3_emac_init(rt_device_t dev)
{
    struct fm3_emac * fm3_emac_device;
    FM3_ETHERNET_MAC_TypeDef * FM3_ETHERNET_MAC;

    fm3_emac_device = (struct fm3_emac *)dev;
    FM3_ETHERNET_MAC = fm3_emac_device->FM3_ETHERNET_MAC;

    rt_mutex_take(&fm3_emac_device->lock, RT_WAITING_FOREVER);

    /* Software reset */
    FM3_ETHERNET_MAC->BMR |= (1<<0); /* [bit0]SWR (Software Reset) */

    /* Wait for software reset */
    while(FM3_ETHERNET_MAC->BMR & (1<<0));

    /* Configure ETHERNET */
    EMAC_init(FM3_ETHERNET_MAC, fm3_emac_device->ETH_Speed, fm3_emac_device->ETH_Mode);

    /* mask all GMAC MMC Interrupt.*/
    FM3_ETHERNET_MAC->mmc_cntl = (1<<3) | (1<<0); /* MMC Counter Freeze and reset. */
    FM3_ETHERNET_MAC->mmc_intr_mask_rx = 0xFFFFFFFF;
    FM3_ETHERNET_MAC->mmc_intr_mask_tx = 0xFFFFFFFF;
    FM3_ETHERNET_MAC->mmc_ipc_intr_mask_rx = 0xFFFFFFFF;

    /* Enable DMA Receive interrupt (need to enable in this case Normal interrupt) */
    EMAC_INT_config(FM3_ETHERNET_MAC, EMAC_DMA_INT_NIS | EMAC_DMA_INT_R | EMAC_DMA_INT_T , ENABLE);

    /* Set Transmit Descriptor List Address Register */
    rt_sem_control(&fm3_emac_device->tx_buf_free, RT_IPC_CMD_RESET, (void *)EMAC_TXBUFNB);
    FM3_ETHERNET_MAC->TDLAR = (uint32_t) fm3_emac_device->DMATxDescToSet;

    /* Set Receive Descriptor List Address Register */
    FM3_ETHERNET_MAC->RDLAR = (uint32_t) fm3_emac_device->DMARxDescToGet;

    /* MAC address configuration */
    EMAC_MAC_Addr_config(FM3_ETHERNET_MAC, EMAC_MAC_Address0, (uint8_t*)&fm3_emac_device->dev_addr[0]);

    /* set interrupt priority */
    NVIC_SetPriority(fm3_emac_device->ETHER_MAC_IRQ, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 3, 1));
    NVIC_EnableIRQ(fm3_emac_device->ETHER_MAC_IRQ);

    /* Enable MAC and DMA transmission and reception */
    EMAC_start(FM3_ETHERNET_MAC);

    rt_mutex_release(&fm3_emac_device->lock);

    return RT_EOK;
}

static rt_err_t fm3_emac_open(rt_device_t dev, rt_uint16_t oflag)
{
    return RT_EOK;
}

static rt_err_t fm3_emac_close(rt_device_t dev)
{
    return RT_EOK;
}

static rt_size_t fm3_emac_read(rt_device_t dev, rt_off_t pos, void* buffer, rt_size_t size)
{
    rt_set_errno(-RT_ENOSYS);
    return 0;
}

static rt_size_t fm3_emac_write (rt_device_t dev, rt_off_t pos, const void* buffer, rt_size_t size)
{
    rt_set_errno(-RT_ENOSYS);
    return 0;
}

static rt_err_t fm3_emac_control(rt_device_t dev, rt_uint8_t cmd, void *args)
{
    struct fm3_emac * fm3_emac_device = (struct fm3_emac *)dev;

    switch (cmd)
    {
    case NIOCTL_GADDR:
        /* get mac address */
        if (args) memcpy(args, &fm3_emac_device->dev_addr[0], MAX_ADDR_LEN);
        else return -RT_ERROR;
        break;

    default :
        break;
    }

    return RT_EOK;
}

static void EMAC_IRQHandler(struct fm3_emac * fm3_emac_device)
{
    rt_uint32_t status, ier;
    FM3_ETHERNET_MAC_TypeDef * FM3_ETHERNET_MAC;

    FM3_ETHERNET_MAC = fm3_emac_device->FM3_ETHERNET_MAC;

    /* get DMA IT status */
    status = FM3_ETHERNET_MAC->SR;
    ier = FM3_ETHERNET_MAC->IER;

    /* GMAC MMC Interrupt. */
    if(status & EMAC_DMA_INT_GMI)
    {
        volatile uint32_t dummy;
        volatile uint32_t * reg;

        EMAC_TRACE("EMAC_DMA_INT_GMI\r\n");

        /* read clear all MMC interrupt. */
        reg = &FM3_ETHERNET_MAC->mmc_cntl;
        while((uint32_t)reg < (uint32_t)&FM3_ETHERNET_MAC->rxicmp_err_octets)
        {
            dummy = *reg++;
        }
    }

    /* Normal interrupt summary. */
    if(status & EMAC_DMA_INT_NIS)
    {
        rt_uint32_t nis_clear = EMAC_DMA_INT_NIS;

        /* [0]:Transmit Interrupt. */
        if((status & ier) & EMAC_DMA_INT_T) /* packet transmission */
        {
            rt_sem_release(&fm3_emac_device->tx_buf_free);

            nis_clear |= EMAC_DMA_INT_T;
        }

        /* [2]:Transmit Buffer Unavailable. */

        /* [6]:Receive Interrupt. */
        if((status & ier) & EMAC_DMA_INT_R) /* packet reception */
        {
            /* disable INT_R when reception first packet. */
            EMAC_INT_config(FM3_ETHERNET_MAC, EMAC_DMA_INT_R, DISABLE);

            /* a frame has been received */
            eth_device_ready(&(fm3_emac_device->parent));

            nis_clear |= EMAC_DMA_INT_R;
        }

        /* [14]:Early Receive Interrupt. */

        EMAC_clear_pending(FM3_ETHERNET_MAC, nis_clear);
    }

    /* Abnormal interrupt summary. */
    if( status & EMAC_DMA_INT_AIS)
    {
        rt_uint32_t ais_clear = EMAC_DMA_INT_AIS;

        /* [1]:Transmit Process Stopped. */
        /* [3]:Transmit Jabber Timeout. */
        /* [4]: Receive FIFO Overflow. */
        /* [5]: Transmit Underflow. */
        /* [7]: Receive Buffer Unavailable. */
        /* [8]: Receive Process Stopped. */
        /* [9]: Receive Watchdog Timeout. */
        /* [10]: Early Transmit Interrupt. */
        /* [13]: Fatal Bus Error. */

        EMAC_clear_pending(FM3_ETHERNET_MAC, ais_clear);
    }

    /* When Tx pause, resume transmission */
    if ((status & EMAC_DMASR_TPS) == EMAC_DMASR_TPS_Suspended)
    {
        EMAC_resume_transmission(FM3_ETHERNET_MAC);
    }
}

#ifdef USING_MAC0
void ETHER_MAC0_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    EMAC_IRQHandler(&fm3_emac_device0);

    /* leave interrupt */
    rt_interrupt_leave();
}
#endif /* #ifdef USING_MAC0 */

#ifdef USING_MAC1
void ETHER_MAC1_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    EMAC_IRQHandler(&fm3_emac_device1);

    /* leave interrupt */
    rt_interrupt_leave();
}
#endif /* #ifdef USING_MAC1 */

/* EtherNet Device Interface */
rt_err_t fm3_emac_tx( rt_device_t dev, struct pbuf* p)
{
    struct pbuf* q;
    char * to;
    struct fm3_emac * fm3_emac_device;
    FM3_ETHERNET_MAC_TypeDef * FM3_ETHERNET_MAC;

    fm3_emac_device = (struct fm3_emac *)dev;
    FM3_ETHERNET_MAC = fm3_emac_device->FM3_ETHERNET_MAC;

    /* get free tx buffer */
    {
        rt_err_t result;
        static int err_count = 0;
		
        result = rt_sem_take(&fm3_emac_device->tx_buf_free, RT_TICK_PER_SECOND/10);
        if (result != RT_EOK)//whs if error stop tx and initial the EMAC_DMA
		{
			err_count ++;

			if(err_count > 1)//
			{
			
        /* PHY RESET : PC9(xuji dual-PHY). */
        
        FM3_GPIO->DDRC |=  (1<<9); /* set GPIO output */
        FM3_GPIO->PDORC &= ~(1<<9); /* output LOW. */
        rt_thread_delay(20);
        FM3_GPIO->PDORC |=  (1<<9); /* output HIGH. */
        rt_thread_delay(20);

        rt_kprintf("reset phy\n");	
        			
				// reset tx desc.
				rt_mutex_take(&fm3_emac_device->lock, RT_WAITING_FOREVER);
				
		
			//	 disable transmit state machine of the MAC for transmission on the MII 
				EMAC_MACTransmissionCmd(FM3_ETHERNET_MAC, DISABLE);
				/// Flush Transmit FIFO 
				EMAC_FlushTransmitFIFO(FM3_ETHERNET_MAC);
				// stop DMA transmission 
				EMAC_DMATransmissionCmd(FM3_ETHERNET_MAC, DISABLE);

				// re-initial tx desc.
				EMAC_DMA_tx_desc_init(fm3_emac_device->DMATxDscrTab, &fm3_emac_device->Tx_Buff[0][0], EMAC_TXBUFNB);
				fm3_emac_device->DMATxDescToSet = fm3_emac_device->DMATxDscrTab;

				rt_sem_control(&fm3_emac_device->tx_buf_free, RT_IPC_CMD_RESET, (void *)EMAC_TXBUFNB);
        //reset phy 
				    FM3_GPIO->DDRC |=  (1<<9); /* set GPIO output */
            FM3_GPIO->PDORC &= ~(1<<9); /* output LOW. */
            rt_thread_delay(20);
            FM3_GPIO->PDORC |=  (1<<9); /* output HIGH. */
            rt_thread_delay(20);

				// re-start tx.
				/// Enable transmit state machine of the MAC for transmission on the MII 
				EMAC_MACTransmissionCmd(FM3_ETHERNET_MAC, ENABLE);
				// Flush Transmit FIFO 
				EMAC_FlushTransmitFIFO(FM3_ETHERNET_MAC);
				/// Start DMA transmission 
				EMAC_DMATransmissionCmd(FM3_ETHERNET_MAC, ENABLE);
				
				rt_mutex_release(&fm3_emac_device->lock);
				rt_kprintf("reset tx buf\n");
				
				err_count = 0;
			}
			else
			{
				return -RT_ERROR;
			}
		}
    }

    rt_mutex_take(&fm3_emac_device->lock, RT_WAITING_FOREVER);

    to = (char *)fm3_emac_device->DMATxDescToSet->Buffer1Addr;

    for (q = p; q != NULL; q = q->next)
    {
        /* Copy the frame to be sent into memory pointed by the current ETHERNET DMA Tx descriptor */
        memcpy(to, q->payload, q->len);
        to += q->len;
    }

#ifdef EMAC_TX_DUMP
    {
        rt_uint32_t i;
        rt_uint8_t *ptr = (rt_uint8_t*)(fm3_emac_device->DMATxDescToSet->Buffer1Addr);

        rt_kprintf("\r\n%c%c tx_dump:", fm3_emac_device->parent.netif->name[0], fm3_emac_device->parent.netif->name[1]);
        for(i=0; i<p->tot_len; i++)
        {
            if( (i%8) == 0 )
            {
                rt_kprintf("  ");
            }
            if( (i%16) == 0 )
            {
                rt_kprintf("\r\n");
            }
            rt_kprintf("%02x ",*ptr);
            ptr++;
        }
        rt_kprintf("\r\ndump done!\r\n");
    }
#endif

    /* Setting the Frame Length: bits[12:0] */
    fm3_emac_device->DMATxDescToSet->ControlBufferSize = (p->tot_len & EMAC_DMATxDesc_TBS1);
    /* Setting the last segment and first segment bits (in this case a frame is transmitted in one descriptor) */
    fm3_emac_device->DMATxDescToSet->Status |= EMAC_DMATxDesc_LS | EMAC_DMATxDesc_FS;
    /* Enable TX Completion Interrupt */
    fm3_emac_device->DMATxDescToSet->Status |= EMAC_DMATxDesc_IC;
#ifdef CHECKSUM_BY_HARDWARE
    fm3_emac_device->DMATxDescToSet->Status |= EMAC_DMATxDesc_ChecksumTCPUDPICMPFull;
    /* clean ICMP checksum */
    {
        struct eth_hdr *ethhdr = (struct eth_hdr *)(fm3_emac_device->DMATxDescToSet->Buffer1Addr);
        /* is IP ? */
        if( ethhdr->type == htons(ETHTYPE_IP) )
        {
            struct ip_hdr *iphdr = (struct ip_hdr *)(fm3_emac_device->DMATxDescToSet->Buffer1Addr + SIZEOF_ETH_HDR);
            /* is ICMP ? */
            if( IPH_PROTO(iphdr) == IP_PROTO_ICMP )
            {
                struct icmp_echo_hdr *iecho = (struct icmp_echo_hdr *)(fm3_emac_device->DMATxDescToSet->Buffer1Addr + SIZEOF_ETH_HDR + sizeof(struct ip_hdr) );
                iecho->chksum = 0;
            }
        }
    }
#endif
    /* Set Own bit of the Tx descriptor Status: gives the buffer back to ETHERNET DMA */
    fm3_emac_device->DMATxDescToSet->Status |= EMAC_DMATxDesc_OWN;

    /* When Tx pause, resume transmission */
    if ((FM3_ETHERNET_MAC->SR & EMAC_DMASR_TPS) == EMAC_DMASR_TPS_Suspended)
    {
        EMAC_resume_transmission(FM3_ETHERNET_MAC);
    }

    /* When Tx Buffer unavailable flag is set: clear it and resume transmission */
    if ((FM3_ETHERNET_MAC->SR & EMAC_DMASR_TBUS) != (uint32_t)RESET)
    {
        /* Clear TBUS ETHERNET DMA flag */
        FM3_ETHERNET_MAC->SR = EMAC_DMASR_TBUS;
        /* Transmit Poll Demand to resume DMA transmission*/
        FM3_ETHERNET_MAC->TPDR = 0;
    }

    /* Update the ETHERNET DMA global Tx descriptor with next Tx decriptor */
    /* Chained Mode */
    /* Selects the next DMA Tx descriptor list for next buffer to send */
    fm3_emac_device->DMATxDescToSet = (EMAC_DMADESCTypeDef*) (fm3_emac_device->DMATxDescToSet->Buffer2NextDescAddr);

    rt_mutex_release(&fm3_emac_device->lock);

    /* Return SUCCESS */
    return RT_EOK;
}

/* reception a Ethernet packet. */
struct pbuf * fm3_emac_rx(rt_device_t dev)
{
    struct pbuf* p;
    rt_uint32_t framelength = 0;
    struct fm3_emac * fm3_emac_device;
    FM3_ETHERNET_MAC_TypeDef * FM3_ETHERNET_MAC;

    fm3_emac_device = (struct fm3_emac *)dev;
    FM3_ETHERNET_MAC = fm3_emac_device->FM3_ETHERNET_MAC;

    /* init p pointer */
    p = RT_NULL;

    rt_mutex_take(&fm3_emac_device->lock, RT_WAITING_FOREVER);

    /* Check if the descriptor is owned by the ETHERNET DMA (when set) or CPU (when reset) */
    if(((fm3_emac_device->DMARxDescToGet->Status & EMAC_DMARxDesc_OWN) != (uint32_t)RESET))
    {
        goto _exit;
    }

    if (((fm3_emac_device->DMARxDescToGet->Status & EMAC_DMARxDesc_ES) == (uint32_t)RESET) &&
            ((fm3_emac_device->DMARxDescToGet->Status & EMAC_DMARxDesc_LS) != (uint32_t)RESET) &&
            ((fm3_emac_device->DMARxDescToGet->Status & EMAC_DMARxDesc_FS) != (uint32_t)RESET))
    {
        /* Get the Frame Length of the received packet: substruct 4 bytes of the CRC */
        framelength = ((fm3_emac_device->DMARxDescToGet->Status & EMAC_DMARxDesc_FL)
                       >> EMAC_DMARXDESC_FRAME_LENGTHSHIFT) - 4;

        /* allocate buffer */
        p = pbuf_alloc(PBUF_LINK, framelength, PBUF_RAM);
        if (p != RT_NULL)
        {
            const char * from;
            struct pbuf* q;

            from = (const char *)fm3_emac_device->DMARxDescToGet->Buffer1Addr;

            for (q = p; q != RT_NULL; q= q->next)
            {
                /* Copy the received frame into buffer from memory pointed by the current ETHERNET DMA Rx descriptor */
                memcpy(q->payload, from, q->len);
                from += q->len;
            }
#ifdef EMAC_RX_DUMP
            {
                rt_uint32_t i;
                rt_uint8_t *ptr = (rt_uint8_t*)(fm3_emac_device->DMARxDescToGet->Buffer1Addr);

                rt_kprintf("\r\n%c%c rx_dump:", fm3_emac_device->parent.netif->name[0], fm3_emac_device->parent.netif->name[1]);
                for(i=0; i<p->tot_len; i++)
                {
                    if( (i%8) == 0 )
                    {
                        rt_kprintf("  ");
                    }
                    if( (i%16) == 0 )
                    {
                        rt_kprintf("\r\n");
                    }
                    rt_kprintf("%02x ",*ptr);
                    ptr++;
                }
                rt_kprintf("\r\ndump done!\r\n");
            }
#endif
        }
    }

    /* Set Own bit of the Rx descriptor Status: gives the buffer back to ETHERNET DMA */
    fm3_emac_device->DMARxDescToGet->Status = EMAC_DMARxDesc_OWN;

    /* When Rx Buffer unavailable flag is set: clear it and resume reception */
    if ((FM3_ETHERNET_MAC->SR & EMAC_DMASR_RBUS) != (uint32_t)RESET)
    {
        /* Clear RBUS ETHERNET DMA flag */
        FM3_ETHERNET_MAC->SR = EMAC_DMASR_RBUS;
        /* Resume DMA reception */
        FM3_ETHERNET_MAC->RPDR = 0;
    }

    /* Update the ETHERNET DMA global Rx descriptor with next Rx decriptor */
    /* Chained Mode */
    if((fm3_emac_device->DMARxDescToGet->ControlBufferSize & EMAC_DMARxDesc_RCH) != (uint32_t)RESET)
    {
        /* Selects the next DMA Rx descriptor list for next buffer to read */
        fm3_emac_device->DMARxDescToGet = (EMAC_DMADESCTypeDef*) (fm3_emac_device->DMARxDescToGet->Buffer2NextDescAddr);
    }
    else /* Ring Mode */
    {
        if((fm3_emac_device->DMARxDescToGet->ControlBufferSize & EMAC_DMARxDesc_RER) != (uint32_t)RESET)
        {
            /* Selects the first DMA Rx descriptor for next buffer to read: last Rx descriptor was used */
            fm3_emac_device->DMARxDescToGet = (EMAC_DMADESCTypeDef*) (FM3_ETHERNET_MAC->RDLAR);
        }
        else
        {
            /* Selects the next DMA Rx descriptor list for next buffer to read */
            fm3_emac_device->DMARxDescToGet = (EMAC_DMADESCTypeDef*) ((uint32_t)fm3_emac_device->DMARxDescToGet + 0x10 + ((FM3_ETHERNET_MAC->BMR & EMAC_DMABMR_DSL) >> 2));
        }
    }

_exit:
    rt_mutex_release(&fm3_emac_device->lock);

    if(p == RT_NULL)
        /* enable INT_R when reception last packet. */
        EMAC_INT_config(FM3_ETHERNET_MAC, EMAC_DMA_INT_R, ENABLE);

    return p;
}

/************ PHY **************/
/*
PHY type: DP83849 (xuji)
PHY0 INT: P60 INT15_1
PHY1 INT: PF5 INT08_0
*/

#include <fm3_ext_int.h>

#define PHY0_INT_NO     (15)
#define PHY1_INT_NO     (8)

#define PHY0_ADDR       (0x01)//whs 0x00
#define PHY1_ADDR       (0x01)

#define PHY_LINK_MASK       (1<<0)
#define PHY_100M_MASK       (1<<1)
#define PHY_DUPLEX_MASK     (1<<2)

static struct rt_event event;

#ifdef USING_MAC0
static void phy0_isr(int vector)
{
    /* enter interrupt */
    rt_interrupt_enter();

    fm3_eint_disable(PHY0_INT_NO);
    rt_event_send(&event, (1 << 0));
    EMAC_TRACE("[PHY0] interrupt\r\n");

    /* leave interrupt */
    rt_interrupt_leave();
}
#endif /* USING_MAC0 */

#ifdef USING_MAC1
static void phy1_isr(int vector)
{
    /* enter interrupt */
    rt_interrupt_enter();

    fm3_eint_disable(PHY1_INT_NO);
    rt_event_send(&event, (1 << 1));
    EMAC_TRACE("[PHY1] interrupt\r\n");

    /* leave interrupt */
    rt_interrupt_leave();
}
#endif /* USING_MAC1 */

static void phy_int_init(void)
{
#ifdef USING_MAC0
    FM3_GPIO->DDR6 &= ~(1<<0); /* set GPIO input */
    FM3_GPIO->PCR6 |=  (1<<0); /* enable GPIO Pull-Up function. */
    FM3_GPIO->PFR6 &= ~(1<<0); /* select GPIO function. */
    fm3_eint_pin_select(EINT15_1_P60); /* select EINT source. */

    /* register handler */
    {
        rt_isr_handler_t func;

        func = (rt_isr_handler_t)phy0_isr;
        fm3_eint_install(PHY0_INT_NO, func, RT_NULL);
    }

    fm3_eint_trigger_config(PHY0_INT_NO, EINT_TRIGGER_LOW); /* set trigger. */
    fm3_eint_enable(PHY0_INT_NO);

    /* enable phy interrupt */
    {
        rt_uint16_t micr = 0;
        rt_uint16_t misr = 0;

        micr |= (1<<1); /* bit1: interrupt enable */
        micr |= (1<<0); /* bit0: interrupt output enable */

        misr |= (1<<5); /* bit5: link change */

        /* note: xuji use dual-phy, both phy use MAC0 MDIO. */
        EMAC_PHY_write(FM3_ETHERNET_MAC0, PHY0_ADDR, 0x11, micr);
        EMAC_PHY_write(FM3_ETHERNET_MAC0, PHY0_ADDR, 0x12, misr);
    }
#endif /* USING_MAC0 */

#ifdef USING_MAC1
    FM3_GPIO->DDRF &= ~(1<<5); /* set GPIO input */
    FM3_GPIO->PCRF |=  (1<<5); /* enable GPIO Pull-Up function. */
    FM3_GPIO->PFRF &= ~(1<<5); /* select GPIO function. */
    fm3_eint_pin_select(EINT08_0_PF5); /* select EINT source. */

    /* register handler */
    {
        rt_isr_handler_t func;

        func = (rt_isr_handler_t)phy1_isr;
        fm3_eint_install(PHY1_INT_NO, func, RT_NULL);
    }

    fm3_eint_trigger_config(PHY1_INT_NO, EINT_TRIGGER_LOW); /* set trigger. */
    fm3_eint_enable(PHY1_INT_NO);

    /* enable phy interrupt */
    {
        rt_uint16_t micr = 0;
        rt_uint16_t misr = 0;

        micr |= (1<<1); /* bit1: interrupt enable */
        micr |= (1<<0); /* bit0: interrupt output enable */

        misr |= (1<<5); /* bit5: link change */

        /* note: xuji use dual-phy, both phy use MAC0 MDIO. */
        EMAC_PHY_write(FM3_ETHERNET_MAC0, PHY1_ADDR, 0x11, micr);
        EMAC_PHY_write(FM3_ETHERNET_MAC0, PHY1_ADDR, 0x12, misr);
    }
#endif /* USING_MAC1 */
}

static void phy_check(struct fm3_emac * fm3_emac_device)
{
    rt_uint16_t phy_status;
    rt_uint8_t phy_speed_new = 0;

    /* note: xuji use dual-phy, both phy use MAC0 MDIO. */

    /* read and clean interrupt. */
    phy_status = EMAC_PHY_read(FM3_ETHERNET_MAC0,
                               fm3_emac_device->phy_addr,
                               0x12);

    phy_status = EMAC_PHY_read(FM3_ETHERNET_MAC0,
                               fm3_emac_device->phy_addr,
                               0x10);

    /* [0] 0: link not established, 1: valid link established. */
    if(phy_status & (1<<0))
    {
        phy_speed_new = PHY_LINK_MASK;

        /* [1] 0: half duplex mode, 1: full duplex mode. */
        if(phy_status & (1<<2))
        {
            phy_speed_new |= PHY_DUPLEX_MASK;
        }

        /* [1] 0: 100Mbps, 1: 10Mbps. */
        if( !(phy_status & (1<<1)) )
        {
            phy_speed_new |= PHY_100M_MASK;
        }
    }

    /* linkchange */
    if(phy_speed_new != fm3_emac_device->phy_speed)
    {
        if(phy_speed_new & PHY_LINK_MASK)
        {
            EMAC_TRACE("[PHY%d] link-up %dMbps %s-duplex\r\n",
                       (fm3_emac_device->FM3_ETHERNET_MAC == FM3_ETHERNET_MAC0)?(0):(1),
                       (phy_speed_new & PHY_100M_MASK)?(100):(10),
                       (phy_speed_new & PHY_DUPLEX_MASK)?("full"):("half"));

            if(phy_speed_new & PHY_100M_MASK)
            {
                fm3_emac_device->ETH_Speed = EMAC_Speed_100M;
            }
            else
            {
                fm3_emac_device->ETH_Speed = EMAC_Speed_10M;
            }

            if(phy_speed_new & PHY_DUPLEX_MASK)
            {
                fm3_emac_device->ETH_Mode = EMAC_Mode_FullDuplex;
            }
            else
            {
                fm3_emac_device->ETH_Mode = EMAC_Mode_HalfDuplex;
            }
            fm3_emac_init((rt_device_t)fm3_emac_device);

            /* send link up. */
            eth_device_linkchange(&fm3_emac_device->parent, RT_TRUE);
        } /* link up. */
        else
        {
            EMAC_TRACE("[PHY%d] link-down\r\n",
                       (fm3_emac_device->FM3_ETHERNET_MAC == FM3_ETHERNET_MAC0)?(0):(1) );
            /* send link down. */
            eth_device_linkchange(&fm3_emac_device->parent, RT_FALSE);
        } /* link down. */

        fm3_emac_device->phy_speed = phy_speed_new;
    } /* linkchange */
}

static void phy_monitor_thread_entry(void *parameter)
{
    rt_err_t result;

    phy_int_init();

    while(1)
    {
        rt_uint32_t e;

        fm3_eint_enable(PHY0_INT_NO);
        fm3_eint_enable(PHY1_INT_NO);

        result = rt_event_recv(&event,
                               (1 << 0) | (1 << 1),
                               RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR,
                               RT_TICK_PER_SECOND,
                               &e);

        if(result != RT_EOK)
        {
#ifdef  USING_MAC0
            e |= (1<<0);
#endif // USING_MAC0
#ifdef  USING_MAC1
            e |= (1<<1);
#endif // USING_MAC1
        }

#ifdef  USING_MAC0
        if(e & (1<<0))
            phy_check(&fm3_emac_device0);
#endif // USING_MAC0
#ifdef  USING_MAC1
        if(e & (1<<1))
            phy_check(&fm3_emac_device1);
#endif // USING_MAC1
    }
}
/************ PHY **************/

/*
GPIO               RMII0               RMII1                MII
PC0/E_RXER0_RXDV1                      ch. 1 CRS_DV         ch. 0 RX_ER
PC1/E_RX03_RX11                        ch. 1 RXD [1]        ch. 0 RXD [3]
PC2/E_RX02_RX10                        ch. 1 RXD [0]        ch. 0 RXD [2]
PC3/E_RX01         ch. 0 RXD [1]                            ch. 0 RXD [1]
PC4/E_RX00         ch. 0 RXD [0]                            ch. 0 RXD [0]
PC5/E_RXDV0        ch. 0 CRS_DV                             ch. 0 RX_DV
PC6/E_MDIO0        ch. 0 MDIO                               ch. 0 MDIO
PC7/E_MDC0         ch. 0 MDC                                ch. 0 MDC
PC8/E_RXCK0_REFCK  ch. 0 REF_CLK       ch. 1 REF_CLK        ch. 0 RX_CLK

PC9/E_COL0                                                  ch. 0 COL
PCA/E_CRS0                                                  ch. 0 CRS
PCB/E_COUT         REF_CLK output(??).

PCC/E_MDIO1                            ch. 1 MDIO
PCD/E_TCK0_MDC1                        ch. 1 MDC            ch. 0 TX_CLK
PCE/E_TXER0_TXEN1                      ch. 1 TX_EN          ch. 0 TX_ER
PCF/E_TX03_TX11                        ch. 1 TXD [1]        ch. 0 TXD [3]
PD0/E_TX02_TX10                        ch. 1 TXD [0]        ch. 0 TXD [2]
PD1/E_TX01         ch. 0 TXD [1]                            ch. 0 TXD [1]
PD2/E_TX00         ch. 0 TXD [0]                            ch. 0 TXD [0]
PD3/E_TXEN0        ch. 0 TX_EN                              ch. 0 TX_EN
*/
extern rt_uint32_t eth_IP1,eth_IP2;
void fm3_emac_hw_init(void)
{
    uint32_t value;

    /* IO config */
#ifndef RMII_MODE
#else
    FM3_GPIO->PFRC |= (1<<8); /* PC8/E_RXCK0_REFCK */
    value = 0;
//    value |= (1<<27) ; /* [bit27] E_PSE: PPS0_PPS1 Output Select Bit for Ethernet */
#    ifdef USING_MAC0
    FM3_GPIO->PFRC |= (1<<3) | (1<<4) | (1<<5) | (1<<0x6) | (1<<0x7);
    FM3_GPIO->PFRD |= (1<<1) | (1<<2) | (1<<3);
    value |= (1<<24) ; /* [bit24] E_MD0B :E_MDO0 I/O Select Bit */
    value |= (1<<22) ; /* [bit22] E_MC0E :E_MDC0 Output Select Bit */
    value |= (1<<20) ; /* [bit20] E_TE0E: E_TXEN0 Output Select Bit */
    value |= (1<<18) ; /* [bit18] E_TD0E: E_TX00, E_TX01 Output Select Bit */
#    endif /* #ifdef USING_MAC0 */
#    ifdef USING_MAC1
    FM3_GPIO->PFRC |= (1<<0) | (1<<1) | (1<<2)
                      | (1<<0x0C) | (1<<0x0D) | (1<<0x0E) | (1<<0x0F);
    FM3_GPIO->PFRD |= (1<<0) ;

    /* xuji use dual-PHY, only need one MDIO. */
//    value |= (1<<25) ; /* [bit25] E_MD1B :E_MDO1 I/O Select Bit */
//    value |= (1<<23) ; /* [bit23] E_MC1B :E_MDC1 I/O Select Bit */

    value |= (1<<21) ; /* [bit21] E_TE1E: E_TXER0_TXEN1 Output Select Bit */
    value |= (1<<19) ; /* [bit19] E_TD1E: E_TX02_TX10, E_TX03_TX11 Output Select Bit */
#    endif /* #ifdef USING_MAC1 */
#endif /* #ifndef RMII_MODE */
    FM3_GPIO->EPFR14 = FM3_GPIO->EPFR14 & 0xFFFF0000 | value;

    /* setup Ethernet-MAC */
    value =0;
#ifdef USING_MAC0
    value |= (1<<0);
#endif /* #ifdef USING_MAC0 */
#ifdef USING_MAC1
    value |= (1<<1);
#endif /* #ifdef USING_MAC1 */
    /* step 1: Start clock supply to Ethernet-MAC */
    FM3_ETHERNET_CONTROL->ETH_CLKG = value;

#ifdef RMII_MODE
    value = 1;
#else
    value = 1;
#endif /* #ifdef RMII_MODE */
#ifdef USING_MAC0
    value |= (1<<8); /* [bit8]: RST0 */
#endif /* #ifdef USING_MAC0 */
#ifdef USING_MAC1
    value |= (1<<9); /* [bit9]: RST1 */
#endif /* #ifdef USING_MAC1 */
    /* step 2: Select MII/RMII and issue a hardware reset to Ethernet-MAC */
    FM3_ETHERNET_CONTROL->ETH_MODE = value;

    /* step 3: */
    /* At this point, clock signal (REF_CLK) must be input from external PHY. */
    /* If the clock signal has not been input, wait until it is input. */

    /* step 4: Select MII/RMII and release a hardware reset of Ethernet-MAC */
#ifdef RMII_MODE
    FM3_ETHERNET_CONTROL->ETH_MODE = 1;/* RMII mode */
#else
    FM3_ETHERNET_CONTROL->ETH_MODE = 0;/* MII mode */
#endif /* #ifdef RMII_MODE */

#ifdef USING_MAC0
    memset(&fm3_emac_device0, 0, sizeof(fm3_emac_device0));

    fm3_emac_device0.phy_addr = PHY0_ADDR;
    fm3_emac_device0.phy_int_no = PHY0_INT_NO;

    /* set autonegotiation mode */
    fm3_emac_device0.ETH_Mode  = EMAC_Mode_HalfDuplex;
    fm3_emac_device0.ETH_Speed = EMAC_Speed_10M;

    fm3_emac_device0.FM3_ETHERNET_MAC = FM3_ETHERNET_MAC0;
    fm3_emac_device0.ETHER_MAC_IRQ  = ETHER_MAC0_IRQn;

    // OUI 00-00-0E FUJITSU LIMITED
//    fm3_emac_device0.dev_addr[0] = 0x00;
//    fm3_emac_device0.dev_addr[1] = 0x00;
//    fm3_emac_device0.dev_addr[2] = 0x0E;
    /* set mac address: (only for test) */
//    fm3_emac_device0.dev_addr[3] = 0x12;
//    fm3_emac_device0.dev_addr[4] = 0x34;
//    fm3_emac_device0.dev_addr[5] = 0x56;

    fm3_emac_device0.dev_addr[0] = 0x00;//whs phy0
    fm3_emac_device0.dev_addr[1] = (eth_IP1>>24)&0xff;
    fm3_emac_device0.dev_addr[2] = (eth_IP1>>16)&0xff;
    fm3_emac_device0.dev_addr[3] = (eth_IP1>>8)&0xff;
    fm3_emac_device0.dev_addr[4] = (eth_IP1)&0xff;
    fm3_emac_device0.dev_addr[5] = 0x0B;

    fm3_emac_device0.parent.parent.init		 = fm3_emac_init;
    fm3_emac_device0.parent.parent.open		 = fm3_emac_open;
    fm3_emac_device0.parent.parent.close	 = fm3_emac_close;
    fm3_emac_device0.parent.parent.read		 = fm3_emac_read;
    fm3_emac_device0.parent.parent.write	 = fm3_emac_write;
    fm3_emac_device0.parent.parent.control	 = fm3_emac_control;
    fm3_emac_device0.parent.parent.user_data = RT_NULL;

    fm3_emac_device0.parent.eth_rx			 = fm3_emac_rx;
    fm3_emac_device0.parent.eth_tx			 = fm3_emac_tx;

    /* Initialize Tx Descriptors list: Chain Mode */
    EMAC_DMA_tx_desc_init(fm3_emac_device0.DMATxDscrTab, &fm3_emac_device0.Tx_Buff[0][0], EMAC_TXBUFNB);
    fm3_emac_device0.DMATxDescToSet = fm3_emac_device0.DMATxDscrTab;
    /* Initialize Rx Descriptors list: Chain Mode  */
    EMAC_DMA_rx_desc_init(fm3_emac_device0.DMARxDscrTab, &fm3_emac_device0.Rx_Buff[0][0], EMAC_RXBUFNB);
    fm3_emac_device0.DMARxDescToGet = fm3_emac_device0.DMARxDscrTab;

    /* init EMAC lock */
    rt_mutex_init(&fm3_emac_device0.lock, "emac0", RT_IPC_FLAG_PRIO);
    /* init tx buffer free semaphore */
    rt_sem_init(&fm3_emac_device0.tx_buf_free,
                "tx_buf0",
                EMAC_TXBUFNB,
                RT_IPC_FLAG_FIFO);
    eth_device_init(&(fm3_emac_device0.parent), "e0");
#endif /* #ifdef USING_MAC0 */

#ifdef USING_MAC1
    memset(&fm3_emac_device1, 0, sizeof(fm3_emac_device1));

    fm3_emac_device1.phy_addr = PHY1_ADDR;
    fm3_emac_device1.phy_int_no = PHY1_INT_NO;

    /* set autonegotiation mode */
    fm3_emac_device1.ETH_Mode  = EMAC_Mode_HalfDuplex;
    fm3_emac_device1.ETH_Speed = EMAC_Speed_10M;

    fm3_emac_device1.FM3_ETHERNET_MAC = FM3_ETHERNET_MAC1;
    fm3_emac_device1.ETHER_MAC_IRQ = ETHER_MAC1_IRQn;

    // OUI 00-00-0E FUJITSU LIMITED
//    fm3_emac_device1.dev_addr[0] = 0x00;
//    fm3_emac_device1.dev_addr[1] = 0x00;
//    fm3_emac_device1.dev_addr[2] = 0x0E;
    /* set mac address: (only for test) */
//    fm3_emac_device1.dev_addr[3] = 0x22;
//    fm3_emac_device1.dev_addr[4] = 0x34;
//    fm3_emac_device1.dev_addr[5] = 0x56;

    fm3_emac_device1.dev_addr[0] = 0x00;
    fm3_emac_device1.dev_addr[1] = (eth_IP1>>24)&0xff;
    fm3_emac_device1.dev_addr[2] = (eth_IP1>>16)&0xff;
    fm3_emac_device1.dev_addr[3] = (eth_IP1>>8)&0xff;
    fm3_emac_device1.dev_addr[4] = (eth_IP1)&0xff;
    fm3_emac_device1.dev_addr[5] = 0x0A;

    fm3_emac_device1.parent.parent.init		 = fm3_emac_init;
    fm3_emac_device1.parent.parent.open		 = fm3_emac_open;
    fm3_emac_device1.parent.parent.close	 = fm3_emac_close;
    fm3_emac_device1.parent.parent.read		 = fm3_emac_read;
    fm3_emac_device1.parent.parent.write	 = fm3_emac_write;
    fm3_emac_device1.parent.parent.control	 = fm3_emac_control;
    fm3_emac_device1.parent.parent.user_data = RT_NULL;

    fm3_emac_device1.parent.eth_rx			 = fm3_emac_rx;
    fm3_emac_device1.parent.eth_tx			 = fm3_emac_tx;

    /* Initialize Tx Descriptors list: Chain Mode */
    EMAC_DMA_tx_desc_init(fm3_emac_device1.DMATxDscrTab, &fm3_emac_device1.Tx_Buff[0][0], EMAC_TXBUFNB);
    fm3_emac_device1.DMATxDescToSet = fm3_emac_device1.DMATxDscrTab;
    /* Initialize Rx Descriptors list: Chain Mode  */
    EMAC_DMA_rx_desc_init(fm3_emac_device1.DMARxDscrTab, &fm3_emac_device1.Rx_Buff[0][0], EMAC_RXBUFNB);
    fm3_emac_device1.DMARxDescToGet = fm3_emac_device1.DMARxDscrTab;

    /* init EMAC lock */
    rt_mutex_init(&fm3_emac_device1.lock, "emac1", RT_IPC_FLAG_PRIO);
    /* init tx buffer free semaphore */
    rt_sem_init(&fm3_emac_device1.tx_buf_free,
                "tx_buf1",
                EMAC_TXBUFNB,
                RT_IPC_FLAG_FIFO);
    eth_device_init(&(fm3_emac_device1.parent), "e1");
#endif /* #ifdef USING_MAC1 */

    /* start phy monitor */
    {
        rt_thread_t tid;

        rt_event_init(&event, "event", RT_IPC_FLAG_FIFO);
        fm3_eint_disable(PHY0_INT_NO);
        //fm3_eint_disable(PHY1_INT_NO);

        tid = rt_thread_create("phy",
                               phy_monitor_thread_entry,
                               RT_NULL,
                               512*2,
                               RT_THREAD_PRIORITY_MAX - 2,
                               2);
        if (tid != RT_NULL)
            rt_thread_startup(tid);
    }
}

#ifdef RT_USING_FINSH
#include <finsh.h>

static void phy_search(void)
{
    int i;
    int value;

    for(i=0; i<32; i++)
    {
        value = EMAC_PHY_read(FM3_ETHERNET_MAC0, i, 2);
        rt_kprintf("addr %02d: %04X\n", i, value);
    }
}
FINSH_FUNCTION_EXPORT(phy_search, search phy use MDIO);

static void phy_dump(int addr)
{
    int i;
    int value;

    rt_kprintf("dump phy addr %d\n", addr);

    for(i=0; i<32; i++)
    {
        value = EMAC_PHY_read(FM3_ETHERNET_MAC0, addr, i);
        rt_kprintf("reg %02d: %04X\n", i, value);
    }
}
FINSH_FUNCTION_EXPORT(phy_dump, dump PHY register);

static void phy_write(int addr, int reg, int value)
{
    EMAC_PHY_write(FM3_ETHERNET_MAC0, addr, reg ,value);
}
FINSH_FUNCTION_EXPORT(phy_write, write PHY register);

static void eth_desc_dump(EMAC_DMADESCTypeDef  *DMATxDescToSet,
						  EMAC_DMADESCTypeDef  *DMARxDescToGet)
{
    int count;
    EMAC_DMADESCTypeDef  *DMATxDescToSet_curr, *DMATxDescToSet_back;
    EMAC_DMADESCTypeDef  *DMARxDescToSet_curr, *DMARxDescToSet_back;

    DMATxDescToSet_back = DMATxDescToSet;
    DMATxDescToSet_curr = DMATxDescToSet_back;

    rt_kprintf("TX desc dump:\n");
    count = 0;
    while(1)
    {
        if(DMATxDescToSet_curr->Status & EMAC_DMATxDesc_OWN)
        {
            rt_kprintf("%d EMAC\n", count);
        }
        else
        {
            rt_kprintf("%d CPU\n", count);
        }

        count++;

        /* Selects the next DMA Tx descriptor list for next buffer to send */
        DMATxDescToSet_curr = (EMAC_DMADESCTypeDef*) (DMATxDescToSet_curr->Buffer2NextDescAddr);

        if(DMATxDescToSet_curr == DMATxDescToSet_back)
            break;
    }

    DMARxDescToSet_back = DMARxDescToGet;
    DMARxDescToSet_curr = DMARxDescToSet_back;

    rt_kprintf("\n");
    rt_kprintf("RX desc dump:\n");
    count = 0;
    while(1)
    {
        if(DMARxDescToSet_curr->Status & EMAC_DMARxDesc_OWN)
        {
            rt_kprintf("%d EMAC\n", count);
        }
        else
        {
            rt_kprintf("%d CPU\n", count);
        }

        count++;

        /* Selects the next DMA Tx descriptor list for next buffer to send */
        DMARxDescToSet_curr = (EMAC_DMADESCTypeDef*) (DMARxDescToSet_curr->Buffer2NextDescAddr);

        if(DMARxDescToSet_curr == DMARxDescToSet_back)
            break;
    }
}

static void eth_dump(void)
{
	rt_kprintf("EMAC0 desc dump\n");
	eth_desc_dump(fm3_emac_device0.DMATxDescToSet,
	fm3_emac_device0.DMARxDescToGet);

	//rt_kprintf("\nEMAC1 desc dump\n");
	//eth_desc_dump(fm3_emac_device1.DMATxDescToSet,
	//fm3_emac_device1.DMARxDescToGet);
}
FINSH_FUNCTION_EXPORT(eth_dump, dump eth desc info);
#endif /* RT_USING_FINSH */
