/**
 * @file timer1.h
 * Библиотека для работы с таймером 1.
 */

#ifndef TIMER1_H
#define	TIMER1_H

#include <stdint.h>
#include <stdbool.h>
#include <avr/io.h>
#include "bits/bits.h"
#include "defs/defs.h"
#include "timer/timer.h"
#include "utils/utils.h"


//Режим таймера.
#define TIMER1_MODE_NORMAL                              0
#define TIMER1_MODE_PHASE_CORRECT_PWM_8BIT              1
#define TIMER1_MODE_PHASE_CORRECT_PWM_9BIT              2
#define TIMER1_MODE_PHASE_CORRECT_PWM_10BIT             3
#define TIMER1_MODE_CTC_OCA                             4
#define TIMER1_MODE_FAST_PWM_8BIT                       5
#define TIMER1_MODE_FAST_PWM_9BIT                       6
#define TIMER1_MODE_FAST_PWM_10BIT                      7
#define TIMER1_MODE_PHASE_FREQ_CORRECT_PWM_IC           8
#define TIMER1_MODE_PHASE_FREQ_CORRECT_PWM_OCA          9
#define TIMER1_MODE_PHASE_CORRECT_PWM_IC                10
#define TIMER1_MODE_PHASE_CORRECT_PWM_OCA               11
#define TIMER1_MODE_CTC_IC                              12
#define TIMER1_MODE_RESERVED                            13
#define TIMER1_MODE_FAST_PWM_IC                         14
#define TIMER1_MODE_FAST_PWM_OCA                        15

//Режимы COM для не-PWM режимов таймера.
#define TIMER1_COM_NONPWM_NORMAL       0
#define TIMER1_COM_NONPWM_TOGGLE       1
#define TIMER1_COM_NONPWM_CLEAR        2
#define TIMER1_COM_NONPWM_SET          3

//Режимы COM для Fast-PWM режима таймера.
#define TIMER1_COM_FAST_PWM_NORMAL       0
#define TIMER1_COM_FAST_PWM_TOGGLE_A     1
#define TIMER1_COM_FAST_PWM_CLEAR        2
#define TIMER1_COM_FAST_PWM_SET          3

//Режимы COM для PhaseCorrect-PWM режима таймера.
#define TIMER1_COM_PHASE_CORRECT_PWM_NORMAL       0
#define TIMER1_COM_PHASE_CORRECT_PWM_TOGGLE_A     1
#define TIMER1_COM_PHASE_CORRECT_PWM_CLEAR        2
#define TIMER1_COM_PHASE_CORRECT_PWM_SET          3

//Источники тактирования таймера.
#define TIMER1_CLOCK_NONE                       0
#define TIMER1_CLOCK_INTERNAL_SCALE_1           1
#define TIMER1_CLOCK_INTERNAL_SCALE_8           2
#define TIMER1_CLOCK_INTERNAL_SCALE_64          3
#define TIMER1_CLOCK_INTERNAL_SCALE_256         4
#define TIMER1_CLOCK_INTERNAL_SCALE_1024        5
#define TIMER1_CLOCK_EXTERNAL_FALLING_EDGE      6
#define TIMER1_CLOCK_EXTERNAL_RISING_EDGE       7

//Ребро для детектирования на входе захвата.
#define TIMER1_INPUT_CAPTURE_FALLING_EDGE       0
#define TIMER1_INPUT_CAPTURE_RISING_EDGE        1

/**
 * Устанавливает режим работы таймера.
 * @param mode Режим работы.
 * @return Код ошибки.
 */
err_t timer1_set_mode(uint8_t mode);

/**
 * Устанавливает режим работы выходного пина таймера для канала A.
 * @param com_mode Режим работы.
 * @return Код ошибки.
 */
err_t timer1_set_com_a_mode(uint8_t com_mode);

/**
 * Устанавливает режим работы выходного пина таймера для канала B.
 * @param com_mode Режим работы.
 * @return Код ошибки.
 */
err_t timer1_set_com_b_mode(uint8_t com_mode);

/**
 * Устанавливает источник тактирования.
 * @param clock Источник тактирования.
 * @return Код ошибки.
 */
err_t timer1_set_clock(uint8_t clock);

/**
 * Включает принудительно режим COM для канала A.
 * @param force_com Включение принудительно.
 */
ALWAYS_INLINE static void timer1_set_force_com_a(bool force_com)
{
    BIT_SET(TCCR1A, FOC1A, force_com);
}

/**
 * Включает принудительно режим COM для канала A.
 * @param force_com Включение принудительно.
 */
ALWAYS_INLINE static void timer1_set_force_com_b(bool force_com)
{
    BIT_SET(TCCR1A, FOC1B, force_com);
}

/**
 * Устанавливает значение счётчика.
 * @param cntr_value Значение счётчика.
 */
ALWAYS_INLINE static void timer1_set_counter_value(uint16_t cntr_value)
{
    __interrupts_save_disable();
    TCNT1 = cntr_value;
    __interrupts_restore();
}

/**
 * Получает значение счётчика.
 * @return Значение счётчика.
 */
ALWAYS_INLINE static uint16_t timer1_counter_value(void)
{
    uint16_t res;
    __interrupts_save_disable();
    res = TCNT1;
    __interrupts_restore();
    return res;
}

/**
 * Устанавливает значение счётчика детектирования событий.
 * @param cntr_value Значение счётчика детектирования событий.
 */
ALWAYS_INLINE static void timer1_set_ic_value(uint16_t cntr_value)
{
    __interrupts_save_disable();
    ICR1 = cntr_value;
    __interrupts_restore();
}

/**
 * Получает значение счётчика детектирования событий.
 * @return Значение счётчика детектирования событий.
 */
ALWAYS_INLINE static uint16_t timer1_ic_value(void)
{
    uint16_t res;
    __interrupts_save_disable();
    res = ICR1;
    __interrupts_restore();
    return res;
}

/**
 * Очищает флаг прерывания детектирования событий.
 */
ALWAYS_INLINE static void timer1_clear_icf(void)
{
    TIFR = BIT(ICF1);
}

/**
 * Устанавливает сравниваемое значение.
 * @param cmp_value Сравниваемое значение.
 */
ALWAYS_INLINE static void timer1_set_compare_a_value(uint16_t cmp_value)
{
    __interrupts_save_disable();
    OCR1A = cmp_value;
    __interrupts_restore();
}

/**
 * Получает значение сравнения канала A.
 * @return Значение сравнения.
 */
ALWAYS_INLINE static uint16_t timer1_compare_a_value(void)
{
    uint16_t res;
    __interrupts_save_disable();
    res = OCR1A;
    __interrupts_restore();
    return res;
}

/**
 * Устанавливает сравниваемое значение.
 * @param cmp_value Сравниваемое значение.
 */
ALWAYS_INLINE static void timer1_set_compare_b_value(uint16_t cmp_value)
{
    __interrupts_save_disable();
    OCR1B = cmp_value;
    __interrupts_restore();
}

/**
 * Получает значение сравнения канала B.
 * @return Значение сравнения.
 */
ALWAYS_INLINE static uint16_t timer1_compare_b_value(void)
{
    uint16_t res;
    __interrupts_save_disable();
    res = OCR1B;
    __interrupts_restore();
    return res;
}

/**
 * Включает фильтр шума на входе захвата.
 */
ALWAYS_INLINE static void timer1_enable_ic_noise_canseler()
{
    BIT_ON(TCCR1B, ICNC1);
}

/**
 * Выключает фильтр шума на входе захвата.
 */
ALWAYS_INLINE static void timer1_disable_ic_noise_canseler()
{
    BIT_OFF(TCCR1B, ICNC1);
}

/**
 * Устанавливает тип ребра для детектирования на входе захвата.
 * @param ic_edge Тип ребра.
 */
ALWAYS_INLINE static void timer1_set_ic_edge(uint8_t ic_edge)
{
    BIT_SET(TCCR1B, ICES1, ic_edge);
}

/**
 * Устанавливает каллбэк переполнения таймера.
 * @param callback Каллбэк.
 * @return Предыдущий каллбэк.
 */
timer_callback_t timer1_set_overflow_callback(timer_callback_t callback);

/**
 * Устанавливает каллбэк сравнения канала A.
 * @param callback Каллбэк.
 * @return Предыдущий каллбэк.
 */
timer_callback_t timer1_set_compare_b_match_callback(timer_callback_t callback);

/**
 * Устанавливает каллбэк сравнения канала B.
 * @param callback Каллбэк.
 * @return Предыдущий каллбэк.
 */
timer_callback_t timer1_set_compare_a_match_callback(timer_callback_t callback);

/**
 * Устанавливает каллбэк входящего события.
 * @param callback Каллбэк.
 * @return Предыдущий каллбэк.
 */
timer_callback_t timer1_set_capture_callback(timer_callback_t callback);

/**
 * Запускает таймер.
 * @return Код ошибки.
 */
err_t timer1_start();

/**
 * Останавливает таймер.
 */
void timer1_stop();

/**
 * Сбрасывает предделитель таймера 1.
 * !!! У таймеров 0 и 1 общий предделитель. !!!
 */
ALWAYS_INLINE static void timer1_prescaler_reset()
{
    BIT_ON(SFIOR, PSR10);
}

#endif	/* TIMER1_H */

