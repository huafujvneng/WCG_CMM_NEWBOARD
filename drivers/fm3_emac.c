#include "fm3_emac.h"

/* Global pointers on Tx and Rx descriptor used to track transmit and receive descriptors */
extern EMAC_DMADESCTypeDef  *DMATxDescToSet;
extern EMAC_DMADESCTypeDef  *DMARxDescToGet;


/**
  * Initializes the ETHERNET peripheral according to the specified
  */
uint32_t EMAC_init(FM3_ETHERNET_MAC_TypeDef * FM3_ETHERNET_MAC,
                    uint32_t speed,
                    uint32_t duplex)
{
    uint32_t  value = 0;
    __IO uint32_t i = 0;
    __IO uint32_t timeout = 0;

    /*-------------------------------- MAC Config ------------------------------*/
    /*---------------------- ETHERNET MACMIIAR Configuration -------------------*/
    /* Get the ETHERNET MACMIIAR value */
    value = FM3_ETHERNET_MAC->GAR;
    /* Clear CSR Clock Range CR[2:0] bits */
    value &= MACMIIAR_CR_MASK;

    /* Get hclk frequency value */
    /* Set CR bits depending on hclk value */
    if((SystemCoreClock >= 20000000)&&(SystemCoreClock < 35000000))
    {
        /* CSR Clock Range between 20-35 MHz */
        value |= (uint32_t)EMAC_MACMIIAR_CR_Div16;
    }
    else if((SystemCoreClock >= 35000000)&&(SystemCoreClock < 60000000))
    {
        /* CSR Clock Range between 35-60 MHz */
        value |= (uint32_t)EMAC_MACMIIAR_CR_Div26;
    }
    else if((SystemCoreClock >= 60000000)&&(SystemCoreClock <= 100000000))
    {
        /* CSR Clock Range between 60-100 MHz */
        value |= (uint32_t)EMAC_MACMIIAR_CR_Div42;
    }
    else if((SystemCoreClock >= 100000000)&&(SystemCoreClock <= 150000000))
    {
        /* CSR Clock Range between 100-150 MHz */
        value |= (uint32_t)EMAC_MACMIIAR_CR_Div62;
    }
    else if((SystemCoreClock >= 150000000)&&(SystemCoreClock <= 250000000))
    {
        /* CSR Clock Range between 150-250 MHz */
        value |= (uint32_t)EMAC_MACMIIAR_CR_Div102;
    }
    else /* if((SystemCoreClock >= 250000000)&&(SystemCoreClock <= 300000000)) */
    {
        /* CSR Clock Range between 250-300 MHz */
        value |= (uint32_t)EMAC_MACMIIAR_CR_Div122;
    }
    /* Write to ETHERNET MAC MIIAR: Configure the ETHERNET CSR Clock Range */
    FM3_ETHERNET_MAC0->GAR = (uint32_t)value;

    /*------------------------ ETHERNET MACCR Configuration --------------------*/
    /* Get the ETHERNET MACCR value */
    value = FM3_ETHERNET_MAC->MCR;
    /* Clear WD, PCE, PS, TE and RE bits */
    value &= MACCR_CLEAR_MASK;

    value |= (uint32_t)(EMAC_Watchdog_Enable |
                        EMAC_Jabber_Enable |
                        EMAC_InterFrameGap_96Bit |
                        EMAC_CarrierSense_Enable |
                        speed |
                        EMAC_ReceiveOwn_Enable |
                        EMAC_LoopbackMode_Disable |
                        duplex |
                        EMAC_ChecksumOffload_Enable |
                        EMAC_RetryTransmission_Disable |
                        EMAC_AutomaticPadCRCStrip_Disable |
                        EMAC_BackOffLimit_10 |
                        EMAC_DeferralCheck_Disable);

    /* Write to ETHERNET MACCR */
    value |= (1<<15);
    value &= ~(1<<25);
    value &= ~(1<<24);
    FM3_ETHERNET_MAC->MCR = (uint32_t)value;

    /*----------------------- ETHERNET MACFFR Configuration --------------------*/
    /* Write to ETHERNET MACFFR */
    FM3_ETHERNET_MAC->MFFR = (uint32_t)(EMAC_ReceiveAll_Disable |
                                        EMAC_SourceAddrFilter_Disable |
                                        EMAC_PassControlFrames_BlockAll |
                                        EMAC_BroadcastFramesReception_Enable |
                                        EMAC_DestinationAddrFilter_Normal |
                                        EMAC_PromiscuousMode_Disable |
                                        EMAC_MulticastFramesFilter_None |
                                        EMAC_UnicastFramesFilter_Perfect);

    /*--------------- ETHERNET MACHTHR and MACHTLR Configuration ---------------*/
    /* Write to ETHERNET MACHTHR */
    FM3_ETHERNET_MAC->MHTRH = 0;
    /* Write to ETHERNET MACHTLR */
    FM3_ETHERNET_MAC->MHTRL = 0;
    /*----------------------- ETHERNET MACFCR Configuration --------------------*/
    /* Get the ETHERNET MACFCR value */
    value = FM3_ETHERNET_MAC->FCR;
    /* Clear xx bits */
    value &= MACFCR_CLEAR_MASK;

    value |= (uint32_t)((0 << 16) |
                        EMAC_ZeroQuantaPause_Disable |
                        EMAC_PauseLowThreshold_Minus4 |
                        EMAC_UnicastPauseFrameDetect_Disable |
                        EMAC_ReceiveFlowControl_Disable |
                        EMAC_TransmitFlowControl_Disable);

    /* Write to ETHERNET MACFCR */
    FM3_ETHERNET_MAC->FCR = (uint32_t)value;
    /*----------------------- ETHERNET MACVLANTR Configuration -----------------*/
    FM3_ETHERNET_MAC->VTR = (uint32_t)(EMAC_VLANTagComparison_16Bit |
                                       0);

    /*-------------------------------- DMA Config ------------------------------*/
    /*----------------------- ETHERNET DMAOMR Configuration --------------------*/
    /* Get the ETHERNET DMAOMR value */
    value = FM3_ETHERNET_MAC->OMR;
    /* Clear xx bits */
    value &= DMAOMR_CLEAR_MASK;

    value |= (uint32_t)(EMAC_DropTCPIPChecksumErrorFrame_Disable |
                        EMAC_ReceiveStoreForward_Enable |
                        EMAC_FlushReceivedFrame_Enable |
                        EMAC_TransmitStoreForward_Enable |
                        EMAC_TransmitThresholdControl_64Bytes |
                        EMAC_ForwardErrorFrames_Disable |
                        EMAC_ForwardUndersizedGoodFrames_Disable |
                        EMAC_ReceiveThresholdControl_64Bytes |
                        EMAC_SecondFrameOperate_Disable);

    /* Write to ETHERNET DMAOMR */
    FM3_ETHERNET_MAC->OMR = (uint32_t)value;

    /*----------------------- ETHERNET DMABMR Configuration --------------------*/
    FM3_ETHERNET_MAC->BMR = (uint32_t)(EMAC_AddressAlignedBeats_Enable |
                                       EMAC_FixedBurst_Enable |
                                       EMAC_RxDMABurstLength_32Beat | /* !! if 4xPBL is selected for Tx or Rx it is applied for the other */
                                       EMAC_TxDMABurstLength_32Beat |
                                       (0 << 2) |
                                       EMAC_DMAArbitration_RoundRobin_RxTx_2_1 |
                                       EMAC_DMABMR_USP); /* Enable use of separate PBL for Rx and Tx */

    /* Return Ethernet configuration success */
    return EMAC_SUCCESS;
}

/**
  * Enables or disables the specified ETHERNET DMA interrupts.
  */
void EMAC_INT_config(FM3_ETHERNET_MAC_TypeDef * FM3_ETHERNET_MAC, uint32_t EMAC_DMA_IT, FunctionalState NewState)
{
    if (NewState != DISABLE)
    {
        /* Enable the selected ETHERNET DMA interrupts */
        FM3_ETHERNET_MAC->IER |= EMAC_DMA_IT;
    }
    else
    {
        /* Disable the selected ETHERNET DMA interrupts */
        FM3_ETHERNET_MAC->IER &=(~(uint32_t)EMAC_DMA_IT);
    }
}

/**
  * Configures the selected MAC address.
  */
void EMAC_MAC_Addr_config(FM3_ETHERNET_MAC_TypeDef * FM3_ETHERNET_MAC, uint32_t MacAddr, uint8_t *Addr)
{
    uint32_t value;

    /* Calculate the selectecd MAC address high register */
    value = ((uint32_t)Addr[5] << 8) | (uint32_t)Addr[4];
    /* Load the selectecd MAC address high register */
    (*(__IO uint32_t *) ((uint32_t)FM3_ETHERNET_MAC + 0x40 + MacAddr)) = value;
    /* Calculate the selectecd MAC address low register */
    value = ((uint32_t)Addr[3] << 24) | ((uint32_t)Addr[2] << 16) | ((uint32_t)Addr[1] << 8) | Addr[0];

    /* Load the selectecd MAC address low register */
    (*(__IO uint32_t *) ((uint32_t)FM3_ETHERNET_MAC + 0x44 + MacAddr)) = value;
}

/**
  * Enables or disables the MAC transmission.
  */
void EMAC_MACTransmissionCmd(FM3_ETHERNET_MAC_TypeDef * FM3_ETHERNET_MAC, FunctionalState NewState)
{
    if (NewState != DISABLE)
    {
        /* Enable the MAC transmission */
        FM3_ETHERNET_MAC->MCR |= EMAC_MACCR_TE;
    }
    else
    {
        /* Disable the MAC transmission */
        FM3_ETHERNET_MAC->MCR &= ~EMAC_MACCR_TE;
    }
}

/**
  * Clears the ETHERNET transmit FIFO.
  */
void EMAC_FlushTransmitFIFO(FM3_ETHERNET_MAC_TypeDef * FM3_ETHERNET_MAC)
{
    /* Set the Flush Transmit FIFO bit */
    FM3_ETHERNET_MAC->OMR |= EMAC_DMAOMR_FTF;
}

/**
  * Enables or disables the MAC reception.
  */
void EMAC_MACReceptionCmd(FM3_ETHERNET_MAC_TypeDef * FM3_ETHERNET_MAC, FunctionalState NewState)
{
    if (NewState != DISABLE)
    {
        /* Enable the MAC reception */
        FM3_ETHERNET_MAC->MCR |= EMAC_MACCR_RE;
    }
    else
    {
        /* Disable the MAC reception */
        FM3_ETHERNET_MAC->MCR &= ~EMAC_MACCR_RE;
    }
}

/**
  * Enables or disables the DMA transmission.
  */
void EMAC_DMATransmissionCmd(FM3_ETHERNET_MAC_TypeDef * FM3_ETHERNET_MAC, FunctionalState NewState)
{
    if (NewState != DISABLE)
    {
        /* Enable the DMA transmission */
        FM3_ETHERNET_MAC->OMR |= EMAC_DMAOMR_ST;
    }
    else
    {
        /* Disable the DMA transmission */
        FM3_ETHERNET_MAC->OMR &= ~EMAC_DMAOMR_ST;
    }
}

/**
  * Enables or disables the DMA reception.
  */
void EMAC_DMAReceptionCmd(FM3_ETHERNET_MAC_TypeDef * FM3_ETHERNET_MAC, FunctionalState NewState)
{
    if (NewState != DISABLE)
    {
        /* Enable the DMA reception */
        FM3_ETHERNET_MAC->OMR |= EMAC_DMAOMR_SR;
    }
    else
    {
        /* Disable the DMA reception */
        FM3_ETHERNET_MAC->OMR &= ~EMAC_DMAOMR_SR;
    }
}

/**
  * Enables ENET MAC and DMA reception/transmission
  */
void EMAC_start(FM3_ETHERNET_MAC_TypeDef * FM3_ETHERNET_MAC)
{
    /* Enable transmit state machine of the MAC for transmission on the MII */
    EMAC_MACTransmissionCmd(FM3_ETHERNET_MAC, ENABLE);
    /* Flush Transmit FIFO */
    EMAC_FlushTransmitFIFO(FM3_ETHERNET_MAC);
    /* Enable receive state machine of the MAC for reception from the MII */
    EMAC_MACReceptionCmd(FM3_ETHERNET_MAC, ENABLE);

    /* Start DMA transmission */
    EMAC_DMATransmissionCmd(FM3_ETHERNET_MAC, ENABLE);
    /* Start DMA reception */
    EMAC_DMAReceptionCmd(FM3_ETHERNET_MAC, ENABLE);
}

/**
  * Clears the ETHERNET's DMA interrupt pending bit.
  */
void EMAC_clear_pending(FM3_ETHERNET_MAC_TypeDef * FM3_ETHERNET_MAC, uint32_t pending)
{
    /* Clear the selected ETHERNET DMA IT */
    FM3_ETHERNET_MAC->SR = (uint32_t) pending;
}

/**
  * Resumes the DMA Transmission by writing to the DmaRxPollDemand register
  *   (the data written could be anything). This forces the DMA to resume reception.
  */
void EMAC_resume_reception(FM3_ETHERNET_MAC_TypeDef * FM3_ETHERNET_MAC)
{
    FM3_ETHERNET_MAC->RPDR = 0;
}

/**
  * Resumes the DMA Transmission by writing to the DmaTxPollDemand register
  *   (the data written could be anything). This forces  the DMA to resume transmission.
  */
void EMAC_resume_transmission(FM3_ETHERNET_MAC_TypeDef * FM3_ETHERNET_MAC)
{
    FM3_ETHERNET_MAC->TPDR = 0;
}


/** \brief E_COUT configure
 *
 * \param is_rmii int 0-MII mode, 1-RMII mode.
 * \return void
 *
 */
void set_ethernet_e_cout_clock(int is_rmii)
{
    /* disable USB and Ethernet clock */
    FM3_USBETHERNETCLK->UCCR = 0;
    FM3_USBETHERNETCLK->UPCR7 = 0;

    /* disable PLL interrupt */
    FM3_USBETHERNETCLK->UPINT_ENR = 0;

    /* wait for USB and Ethernet clock stop */
    while((FM3_USBETHERNETCLK->UCCR & 0x13) != 0x00);

    /* disable USB/Ethernet-PLL clock */
    FM3_USBETHERNETCLK->UPCR1 = 0;

    /* select Ethernet-PLL output */
    FM3_USBETHERNETCLK->UCCR = (1<<5); /* [65]: 01-PLL */

    /* select PLL input clock */
    /*UPCR1 [1] UPINC : 0-CLKMO [initial value] */

    /* select clock stabilization time */
    FM3_USBETHERNETCLK->UPCR2 = 0x00;

    /* PLL_OUT  = Fin*N/M/K */
    /* RMII_50M = 4M*50/4/1 */
    /* RMII_25M = 4M*50/8/1 */

    /* Ethernet-PLL clock configuration register(K) initialize */
    FM3_USBETHERNETCLK->UPCR3 = 1-1;

    /* Ethernet-PLL clock configuration register(N) initialize */
    FM3_USBETHERNETCLK->UPCR4 = 50-1; /* N */

    /* Ethernet-PLL clock configuration register(M) initialize */
    if(is_rmii)
    {
        FM3_USBETHERNETCLK->UPCR5 = 4-1; /* M */
    }
    else
    {
        FM3_USBETHERNETCLK->UPCR5 = 8-1; /* M */
    }

    /* enable Ethernet-PLL */
    FM3_USBETHERNETCLK->UPCR7 = 1; /* [0] EPLLEN */

    /* Ethernet-PLL clock stabilize interrupt disable  */
    /* UPINT_ENR [0] UPCSE : 0-Disable generation of interrupt [initial value] */

    /* enable Ethernet-PLL clock */
    /* UPCR1 [1] UPLLEN : 1-Enable USB/Ethernet-PLL oscillation */
    FM3_USBETHERNETCLK->UPCR1 |= (1<<0);

    /* wait for Ethernet-PLL clock ready */
    while((FM3_USBETHERNETCLK->UP_STR & (1<<0)) == 0x00);

    /* enable Ethernet clock output */
    FM3_USBETHERNETCLK->UCCR |= 1<<4; /* [4] : 1-Enable Ethernet clock output */

    /* wait for 5 cycle */
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();

    /* configure E_COUT PIN */
    FM3_GPIO->EPFR14 |= (1 << 26); /* [bit26] E_CKE: E_COUT Output Select Bit */
    FM3_GPIO->PFRC |= (1 << 0x0B); /* PCB : E_COUT */

    return;
}

/**
  * Read a PHY register
  */
uint16_t EMAC_PHY_read(FM3_ETHERNET_MAC_TypeDef * FM3_ETHERNET_MAC, uint16_t PHYAddress, uint16_t PHYReg)
{
    uint32_t value = 0;
    __IO uint32_t timeout = 0;

    /* Get the ETHERNET MACMIIAR value */
    value = FM3_ETHERNET_MAC->GAR;
    /* Keep only the CSR Clock Range CR[2:0] bits value */
    value &= ~MACMIIAR_CR_MASK;
    /* Prepare the MII address register value */
    value |=(((uint32_t)PHYAddress<<11) & EMAC_MACMIIAR_PA); /* Set the PHY device address */
    value |=(((uint32_t)PHYReg<<6) & EMAC_MACMIIAR_MR);      /* Set the PHY register address */
    value &= ~EMAC_MACMIIAR_MW;                              /* Set the read mode */
    value |= EMAC_MACMIIAR_MB;                               /* Set the MII Busy bit */
    /* Write the result value into the MII Address register */
    FM3_ETHERNET_MAC->GAR = value;
    /* Check for the Busy flag */
    do
    {
        timeout++;
        value = FM3_ETHERNET_MAC->GAR;
    }
    while ((value & EMAC_MACMIIAR_MB) && (timeout < (uint32_t)PHY_READ_TO));
    /* Return ERROR in case of timeout */
    if(timeout == PHY_READ_TO)
    {
        return (uint16_t)EMAC_ERROR;
    }

    /* Return data register value */
    return (uint16_t)(FM3_ETHERNET_MAC->GDR);
}

/**
  * Write to a PHY register
  */
uint32_t EMAC_PHY_write(FM3_ETHERNET_MAC_TypeDef * FM3_ETHERNET_MAC, uint16_t PHYAddress, uint16_t PHYReg, uint16_t PHYValue)
{
    uint32_t value = 0;
    __IO uint32_t timeout = 0;

    /* Get the ETHERNET MACMIIAR value */
    value = FM3_ETHERNET_MAC->GAR;
    /* Keep only the CSR Clock Range CR[2:0] bits value */
    value &= ~MACMIIAR_CR_MASK;
    /* Prepare the MII register address value */
    value |=(((uint32_t)PHYAddress<<11) & EMAC_MACMIIAR_PA); /* Set the PHY device address */
    value |=(((uint32_t)PHYReg<<6) & EMAC_MACMIIAR_MR);      /* Set the PHY register address */
    value |= EMAC_MACMIIAR_MW;                               /* Set the write mode */
    value |= EMAC_MACMIIAR_MB;                               /* Set the MII Busy bit */
    /* Give the value to the MII data register */
    FM3_ETHERNET_MAC->GDR = PHYValue;
    /* Write the result value into the MII Address register */
    FM3_ETHERNET_MAC->GAR = value;
    /* Check for the Busy flag */
    do
    {
        timeout++;
        value = FM3_ETHERNET_MAC->GAR;
    }
    while ((value & EMAC_MACMIIAR_MB) && (timeout < (uint32_t)PHY_WRITE_TO));
    /* Return ERROR in case of timeout */
    if(timeout == PHY_WRITE_TO)
    {
        return EMAC_ERROR;
    }

    /* Return SUCCESS */
    return EMAC_SUCCESS;
}
