/**
 * @file timer2.h
 * Библиотека для работы с таймером 2.
 */

#ifndef TIMER2_H
#define	TIMER2_H

#include <stdint.h>
#include <stdbool.h>
#include <avr/io.h>
#include "bits/bits.h"
#include "defs/defs.h"
#include "timer/timer.h"


//Режим таймера.
#define TIMER2_MODE_NORMAL 0
#define TIMER2_MODE_PHASE_CORRECT_PWM 1
#define TIMER2_MODE_CTC 2
#define TIMER2_MODE_FAST_PWM 3

//Режимы COM для не-PWM режимов таймера.
#define TIMER2_COM_NONPWM_NORMAL       0
#define TIMER2_COM_NONPWM_TOGGLE       1
#define TIMER2_COM_NONPWM_CLEAR        2
#define TIMER2_COM_NONPWM_SET          3

//Режимы COM для Fast-PWM режима таймера.
#define TIMER2_COM_FAST_PWM_NORMAL       0
#define TIMER2_COM_FAST_PWM_CLEAR        2
#define TIMER2_COM_FAST_PWM_SET          3

//Режимы COM для PhaseCorrect-PWM режима таймера.
#define TIMER2_COM_PHASE_CORRECT_PWM_NORMAL       0
#define TIMER2_COM_PHASE_CORRECT_PWM_CLEAR        2
#define TIMER2_COM_PHASE_CORRECT_PWM_SET          3

//Источники тактирования таймера.
#define TIMER2_CLOCK_NONE                       0
#define TIMER2_CLOCK_INTERNAL_SCALE_1           1
#define TIMER2_CLOCK_INTERNAL_SCALE_8           2
#define TIMER2_CLOCK_INTERNAL_SCALE_32          3
#define TIMER2_CLOCK_INTERNAL_SCALE_64          4
#define TIMER2_CLOCK_INTERNAL_SCALE_128         5
#define TIMER2_CLOCK_INTERNAL_SCALE_256         6
#define TIMER2_CLOCK_INTERNAL_SCALE_1024        7

/**
 * Устанавливает режим работы таймера.
 * @param mode Режим работы.
 * @return Код ошибки.
 */
err_t timer2_set_mode(uint8_t mode);

/**
 * Устанавливает режим работы выходного пина таймера.
 * @param com_mode Режим работы.
 * @return Код ошибки.
 */
err_t timer2_set_com_mode(uint8_t com_mode);

/**
 * Устанавливает источник тактирования.
 * @param clock Источник тактирования.
 * @return Код ошибки.
 */
err_t timer2_set_clock(uint8_t clock);

/**
 * Устанавливает асинхронный режим таймера.
 * @param async Флаг асинхронности.
 */
ALWAYS_INLINE static void timer2_set_async(bool async)
{
    BIT_SET(ASSR, AS2, async);
}

/**
 * Включает принудительно режим COM.
 * @param force_com Включение принудительно.
 */
ALWAYS_INLINE static void timer2_set_force_com(bool force_com)
{
    BIT_WAIT_OFF(ASSR, TCR2UB);
    BIT_SET(TCCR2, FOC2, force_com);
    BIT_WAIT_OFF(ASSR, TCR2UB);
}

/**
 * Устанавливает значение счётчика.
 * @param cntr_value Значение счётчика.
 */
ALWAYS_INLINE static void timer2_set_counter_value(uint8_t cntr_value)
{
    BIT_WAIT_OFF(ASSR, TCN2UB);
    TCNT2 = cntr_value;
    BIT_WAIT_OFF(ASSR, TCN2UB);
}

/**
 * Получает значение счётчика.
 * @return Значение счётчика.
 */
ALWAYS_INLINE static uint8_t timer2_counter_value(void)
{
    return TCNT2;
}

/**
 * Устанавливает сравниваемое значение.
 * @param cmp_value Сравниваемое значение.
 */
ALWAYS_INLINE static void timer2_set_compare_value(uint8_t cmp_value)
{
    BIT_WAIT_OFF(ASSR, OCR2UB);
    OCR2 = cmp_value;
    BIT_WAIT_OFF(ASSR, OCR2UB);
}

/**
 * Получает значение сравнения.
 * @return Значение сравнения.
 */
ALWAYS_INLINE static uint8_t timer2_compare_value(void)
{
    return OCR2;
}

/**
 * Устанавливает каллбэк переполнения таймера.
 * @param callback Каллбэк.
 * @return Предыдущий каллбэк.
 */
timer_callback_t timer2_set_overflow_callback(timer_callback_t callback);

/**
 * Устанавливает каллбэк сравнения.
 * @param callback Каллбэк.
 * @return Предыдущий каллбэк.
 */
timer_callback_t timer2_set_compare_match_callback(timer_callback_t callback);

/**
 * Запускает таймер.
 * @return Код ошибки.
 */
err_t timer2_start();

/**
 * Останавливает таймер.
 */
void timer2_stop();

/**
 * Сбрасывает предделитель таймера 2.
 */
ALWAYS_INLINE static void timer2_prescaler_reset()
{
    BIT_ON(SFIOR, PSR2);
}

#endif	/* TIMER2_H */

