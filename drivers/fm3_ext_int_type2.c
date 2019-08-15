#include "board.h"

#include "fm3_ext_int.h"

/* FM3 type0 has External Interrupt 0~15 */
#define MAX_EINT_HANDLER        32

/* External interrupt handler table */

static rt_isr_handler_t eint_isr_table[MAX_EINT_HANDLER];

static void eint_handler(int vector)
{
    rt_kprintf("default external handler! vector:%d\r\n", vector);
}

static void FM3_INT_Handler(void)
{
    rt_uint32_t pend = FM3_EXTI->EIRR & FM3_EXTI->ENIR;
    rt_isr_handler_t func;
    rt_uint32_t i;

    /* clean all external handler */
    FM3_EXTI->EICL = 0;

    for(i=0; i<MAX_EINT_HANDLER; i++)
    {
        if(pend & (1UL<<i))
        {
            func = eint_isr_table[i];
            if(func != RT_NULL)
            {
                func(i);
            }
        }
    }
}
/** \brief External Interrupt Handler ch.0 to ch.7
 *
 * \param void
 * \return void
 *
 */
void INT0_7_Handler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    FM3_INT_Handler();

    /* leave interrupt */
    rt_interrupt_leave();
}

/** \brief External Interrupt Handler ch.8 to ch.31
 *
 * \param void
 * \return void
 *
 */
void INT8_31_Handler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    FM3_INT_Handler();

    /* leave interrupt */
    rt_interrupt_leave();
}

/** \brief install External Interrupt handler.
 *
 * \param vector int Interrupt vector.(0~15)
 * \param new_handler the new handler.
 * \param *old_handler the pointer to save old handler.
 * \return
 *
 */
void fm3_eint_install(int vector,
                      rt_isr_handler_t new_handler,
                      rt_isr_handler_t *old_handler)
{
    if(vector < MAX_EINT_HANDLER)
    {
        if (old_handler != RT_NULL) *old_handler = eint_isr_table[vector];
        if (new_handler != RT_NULL) eint_isr_table[vector] = new_handler;
    }
}

/** \brief enable FM3 External Interrupt.
 *
 * \param vector int Interrupt vector.(0~15)
 * \return void
 *
 */
void fm3_eint_enable(int vector)
{
    /* Clears an interrupt cause. */
    FM3_EXTI->EICL &= ~(1<<vector);

    /* Enables an external interrupt. */
    FM3_EXTI->ENIR |=  (1<<vector);

    if(FM3_EXTI->ENIR & 0xFF)
    {
	    /* set interrupt priority */
	    NVIC_SetPriority(EXINT0_7_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 3, 3));
        NVIC_EnableIRQ(EXINT0_7_IRQn);
    }

    if(FM3_EXTI->ENIR & 0xFF00)
    {
	    /* set interrupt priority */
	    NVIC_SetPriority(EXINT8_31_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 3, 3));
        NVIC_EnableIRQ(EXINT8_31_IRQn);
    }
}

/** \brief disable FM3 External Interrupt.
 *
 * \param vector int Interrupt vector.(0~15)
 * \return void
 *
 */
void fm3_eint_disable(int vector)
{
    FM3_EXTI->ENIR &= ~(1<<vector);

    if((FM3_EXTI->ENIR & 0xFF) == 0)
    {
        NVIC_DisableIRQ(EXINT0_7_IRQn);
    }

    if((FM3_EXTI->ENIR & 0xFF00) == 0)
    {
        NVIC_DisableIRQ(EXINT8_31_IRQn);
    }
}

/** \brief config External Interrupt trigger mode.
 *
 * \param vector int Interrupt vector.(0~31)
 * \param eint_trigger eint_trigger_typedef
 * \return void
 *
 */
void fm3_eint_trigger_config(int vector, eint_trigger_typedef eint_trigger)
{
    volatile uint32_t ELVR;

    if(vector > 15)
    {
        /* 2-bit (LA and LB) basis. */
        vector -= 16;
        vector = vector * 2;

        /* config LAx & LBx. */
        FM3_EXTI->ELVR1 = (FM3_EXTI->ELVR1 & ~(0x03<<vector))
        | (eint_trigger<<vector);

        /* Read the External Interrupt Level Register (ELVR). */
        /* see: External Interrupt and NMI Control Sections */
        ELVR = FM3_EXTI->ELVR1;
    }
    else
    {
        /* 2-bit (LA and LB) basis. */
        vector = vector * 2;

        /* config LAx & LBx. */
        FM3_EXTI->ELVR = (FM3_EXTI->ELVR & ~(0x03<<vector))
        | (eint_trigger<<vector);

        /* Read the External Interrupt Level Register (ELVR). */
        /* see: External Interrupt and NMI Control Sections */
        ELVR = FM3_EXTI->ELVR;
    }

}

/** \brief get External Interrupt trigger mode.
 *
 * \param vector int Interrupt vector.(0~31)
 * \return eint_trigger_typedef
 *
 */
eint_trigger_typedef fm3_eint_trigger_get(int vector)
{
    eint_trigger_typedef ret;

    if(vector > 15)
    {
        /* 2-bit (LA and LB) basis. */
        vector -= 16;
        vector = vector * 2;

        /*get LAx & LBx*/
        ret = (eint_trigger_typedef)
        ((FM3_EXTI->ELVR1 & (0x03<<vector)) >> vector);
    }
    else
    {
        /* 2-bit (LA and LB) basis. */
        vector = vector * 2;

        /*get LAx & LBx*/
        ret = (eint_trigger_typedef)
        ((FM3_EXTI->ELVR & (0x03<<vector)) >> vector);
    }


 	return ret;
}

/** \brief select which pin generate interrupt signal.
 * \param pin eint_pin_typedef
 * \return vector
 *
 */
int fm3_eint_pin_select(eint_pin_typedef eint_pin)
{
	int cfg, shift, eint;

	/* typedef format: CFG_VALUE SHIFT INT */
	cfg   = (eint_pin>>16) & 0xFF;
	shift = (eint_pin>>8)  & 0xFF;
	eint  =  eint_pin      & 0xFF;

	if(eint > 15)
    {
        /* EINT16 ~ EINT31: EPFR15)*/
        shift -= 32;
        FM3_GPIO->EPFR15 &= ~(0x3 << shift); /* clear old value */
        FM3_GPIO->EPFR15 |=  (cfg << shift); /* set new config */
    }
    else
    {
        /* EINT0 ~ EINT15: EPFR06)*/

        FM3_GPIO->EPFR06 &= ~(0x3 << shift); /* clear old value */
        FM3_GPIO->EPFR06 |=  (cfg << shift); /* set new config */
    }

	/* return vector */
    return eint;
}

/** \brief init FM3 External Interrupt.
 *
 * \param void
 * \return void
 *
 */
void fm3_eint_init(void)
{
    rt_uint32_t i;

    /* disable all External Interrupt. */
    FM3_EXTI->ENIR = 0;

    /* init external handler table */
    for(i=0; i<MAX_EINT_HANDLER; i++)
    {
        eint_isr_table[i] = eint_handler;
    }
}
