#ifndef FM3_EXT_INT_H_INCLUDED
#define FM3_EXT_INT_H_INCLUDED

#include <rtthread.h>

#include "board.h"

typedef enum
{
    EINT_TRIGGER_LOW = 0,
    EINT_TRIGGER_HIGH,
    EINT_TRIGGER_RISING,
    EINT_TRIGGER_FALLING,
}eint_trigger_typedef;

typedef enum
{
                 /* CFG_VALUE      SHIFT       INT */
    /* EINT00 */
	EINT00_0_P50 = (0x00<<16) | (( 0 * 2)<<8) | ( 0),
	EINT00_1_P05 = (0x02<<16) | (( 0 * 2)<<8) | ( 0),
	EINT00_2_PD2 = (0x03<<16) | (( 0 * 2)<<8) | ( 0),

    /* EINT01 */
	EINT01_0_P51 = (0x00<<16) | (( 1 * 2)<<8) | ( 1),
	EINT01_1_P06 = (0x02<<16) | (( 1 * 2)<<8) | ( 1),
	EINT01_2_P24 = (0x03<<16) | (( 1 * 2)<<8) | ( 1),

    /* EINT02 */
	EINT02_0_P52 = (0x00<<16) | (( 2 * 2)<<8) | ( 2),
	EINT02_1_P11 = (0x02<<16) | (( 2 * 2)<<8) | ( 2),
	EINT02_2_P27 = (0x03<<16) | (( 2 * 2)<<8) | ( 2),

    /* EINT03 */
	EINT03_0_PA4 = (0x00<<16) | (( 3 * 2)<<8) | ( 3),
	EINT03_1_P14 = (0x02<<16) | (( 3 * 2)<<8) | ( 3),
	EINT03_2_P30 = (0x03<<16) | (( 3 * 2)<<8) | ( 3),

    /* EINT04 */
	EINT04_0_P33 = (0x00<<16) | (( 4 * 2)<<8) | ( 4),
	EINT04_1_P17 = (0x02<<16) | (( 4 * 2)<<8) | ( 4),
	EINT04_2_P31 = (0x03<<16) | (( 4 * 2)<<8) | ( 4),

    /* EINT05 */
	EINT05_0_P20 = (0x00<<16) | (( 5 * 2)<<8) | ( 5),
	EINT05_1_P1A = (0x02<<16) | (( 5 * 2)<<8) | ( 5),
	EINT05_2_P32 = (0x03<<16) | (( 5 * 2)<<8) | ( 5),

    /* EINT06 */
	EINT06_0_PF3 = (0x00<<16) | (( 6 * 2)<<8) | ( 6),
	EINT06_1_P21 = (0x02<<16) | (( 6 * 2)<<8) | ( 6),
	EINT06_2_P4E = (0x03<<16) | (( 6 * 2)<<8) | ( 6),

    /* EINT07 */
	EINT07_0_PF4 = (0x00<<16) | (( 7 * 2)<<8) | ( 7),
	EINT07_1_P75 = (0x02<<16) | (( 7 * 2)<<8) | ( 7),
	EINT07_2_P53 = (0x03<<16) | (( 7 * 2)<<8) | ( 7),

    /* EINT08 */
	EINT08_0_PF5 = (0x00<<16) | (( 8 * 2)<<8) | ( 8),
	EINT08_1_P35 = (0x02<<16) | (( 8 * 2)<<8) | ( 8),
	EINT08_2_P56 = (0x03<<16) | (( 8 * 2)<<8) | ( 8),

    /* EINT09 */
	EINT09_0_P28 = (0x00<<16) | (( 9 * 2)<<8) | ( 9),
	EINT09_1_P36 = (0x02<<16) | (( 9 * 2)<<8) | ( 9),
	EINT09_2_P59 = (0x03<<16) | (( 9 * 2)<<8) | ( 9),

    /* EINT10 */
	EINT10_0_P7B = (0x00<<16) | ((10 * 2)<<8) | (10),
	EINT10_1_P37 = (0x02<<16) | ((10 * 2)<<8) | (10),
	EINT10_2_PA5 = (0x03<<16) | ((10 * 2)<<8) | (10),

    /* EINT11 */
	EINT11_0_P7C = (0x00<<16) | ((11 * 2)<<8) | (11),
	EINT11_1_P38 = (0x02<<16) | ((11 * 2)<<8) | (11),
	EINT11_2_P76 = (0x03<<16) | ((11 * 2)<<8) | (11),

    /* EINT12 */
	EINT12_0_P7D = (0x00<<16) | ((12 * 2)<<8) | (12),
	EINT12_1_P40 = (0x02<<16) | ((12 * 2)<<8) | (12),
	EINT12_2_P77 = (0x03<<16) | ((12 * 2)<<8) | (12),

    /* EINT13 */
	EINT13_0_PF0 = (0x00<<16) | ((13 * 2)<<8) | (13),
	EINT13_1_P41 = (0x02<<16) | ((13 * 2)<<8) | (13),
	EINT13_2_P71 = (0x03<<16) | ((13 * 2)<<8) | (13),

    /* EINT14 */
	EINT14_0_PF1 = (0x00<<16) | ((14 * 2)<<8) | (14),
	EINT14_1_P48 = (0x02<<16) | ((14 * 2)<<8) | (14),
	EINT14_2_P72 = (0x03<<16) | ((14 * 2)<<8) | (14),

    /* EINT15 */
	EINT15_0_PF2 = (0x00<<16) | ((15 * 2)<<8) | (15),
	EINT15_1_P60 = (0x02<<16) | ((15 * 2)<<8) | (15),
	EINT15_2_P73 = (0x03<<16) | ((15 * 2)<<8) | (15),

    /* EINT16 */
	EINT16_0_PB0 = (0x00<<16) | ((16 * 2)<<8) | (16),
	EINT16_1_P57 = (0x02<<16) | ((16 * 2)<<8) | (16),

    /* EINT17 */
	EINT17_0_PB1 = (0x00<<16) | ((17 * 2)<<8) | (17),
	EINT17_1_P58 = (0x02<<16) | ((17 * 2)<<8) | (17),

    /* EINT18 */
	EINT18_0_PB2 = (0x00<<16) | ((18 * 2)<<8) | (18),
	EINT18_1_P5A = (0x02<<16) | ((18 * 2)<<8) | (18),

    /* EINT19 */
	EINT19_0_PB3 = (0x00<<16) | ((19 * 2)<<8) | (19),
	EINT19_1_P5B = (0x02<<16) | ((19 * 2)<<8) | (19),

    /* EINT20 */
	EINT20_0_PB4 = (0x00<<16) | ((20 * 2)<<8) | (20),
	EINT20_1_P16 = (0x02<<16) | ((20 * 2)<<8) | (20),

    /* EINT21 */
	EINT21_0_PB5 = (0x00<<16) | ((21 * 2)<<8) | (21),
	EINT21_1_P18 = (0x02<<16) | ((21 * 2)<<8) | (21),

    /* EINT22 */
	EINT22_0_PB6 = (0x00<<16) | ((22 * 2)<<8) | (22),
	EINT22_1_P19 = (0x02<<16) | ((22 * 2)<<8) | (22),

    /* EINT23 */
	EINT23_0_PB7 = (0x00<<16) | ((23 * 2)<<8) | (23),
	EINT23_1_P79 = (0x02<<16) | ((23 * 2)<<8) | (23),

    /* EINT24 */
	EINT24_0_P7E = (0x00<<16) | ((24 * 2)<<8) | (24),
	EINT24_1_P7A = (0x02<<16) | ((24 * 2)<<8) | (24),

    /* EINT25 */
	EINT25_0_P7F = (0x00<<16) | ((25 * 2)<<8) | (25),
	EINT25_1_P1B = (0x02<<16) | ((25 * 2)<<8) | (25),

    /* EINT26 */
	EINT26_0_P94 = (0x00<<16) | ((26 * 2)<<8) | (26),
	EINT26_1_P1C = (0x02<<16) | ((26 * 2)<<8) | (26),

    /* EINT27 */
	EINT27_0_P95 = (0x00<<16) | ((27 * 2)<<8) | (27),
	EINT27_1_P1D = (0x02<<16) | ((27 * 2)<<8) | (27),

    /* EINT28 */
	EINT28_0_P5C = (0x00<<16) | ((28 * 2)<<8) | (28),
	EINT28_1_P1E = (0x02<<16) | ((28 * 2)<<8) | (28),

    /* EINT29 */
	EINT29_0_P5D = (0x00<<16) | ((29 * 2)<<8) | (29),
	EINT29_1_P1F = (0x02<<16) | ((29 * 2)<<8) | (29),

    /* EINT30 */
	EINT30_0_P90 = (0x00<<16) | ((30 * 2)<<8) | (30),
	EINT30_1_PD0 = (0x02<<16) | ((30 * 2)<<8) | (30),

    /* EINT31 */
	EINT31_0_P91 = (0x00<<16) | ((31 * 2)<<8) | (31),
	EINT31_1_PD1 = (0x02<<16) | ((31 * 2)<<8) | (31),
}eint_pin_typedef;

/** \brief init FM3 External Interrupt.
 *
 * \param void
 * \return void
 *
 */
extern void fm3_eint_init(void);

/** \brief install External Interrupt handler.
 *
 * \param vector int Interrupt vector.(0~31)
 * \param new_handler the new handler.
 * \param *old_handler the pointer to save old handler.
 * \return
 *
 */
extern void fm3_eint_install(int vector,
                               rt_isr_handler_t new_handler,
                               rt_isr_handler_t *old_handler);

/** \brief config External Interrupt trigger mode.
 *
 * \param vector int Interrupt vector.(0~31)
 * \param eint_trigger eint_trigger_typedef
 * \return void
 *
 */
extern void fm3_eint_trigger_config(int vector,
                                    eint_trigger_typedef eint_trigger);

/** \brief get External Interrupt trigger mode.
 *
 * \param vector int Interrupt vector.(0~31)
 * \return eint_trigger_typedef
 *
 */
extern eint_trigger_typedef fm3_eint_trigger_get(int vector);

/** \brief select which pin generate interrupt signal.
 * \param pin eint_pin_typedef
 * \return vector
 *
 */
extern int fm3_eint_pin_select(eint_pin_typedef pin);

/** \brief enable FM3 External Interrupt.
 *
 * \param vector int Interrupt vector.(0~31)
 * \return void
 *
 */
extern void fm3_eint_enable(int vector);

/** \brief disable FM3 External Interrupt.
 *
 * \param vector int Interrupt vector.(0~31)
 * \return void
 *
 */
extern void fm3_eint_disable(int vector);

#endif // FM3_EXT_INT_H_INCLUDED
