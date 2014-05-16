#include "timer2.h"
#include <avr/interrupt.h>


/*
 * TIMER 0
 */

typedef struct _Timer0_State {
    uint8_t clock;
    timer_callback_t overflow_callback;
    timer_callback_t compare_callback;
}timer2_state_t;

timer2_state_t timer2_state = {0};

ISR(TIMER2_OVF_vect)
{
    if(timer2_state.overflow_callback) timer2_state.overflow_callback();
}

ISR(TIMER2_COMP_vect)
{
    if(timer2_state.compare_callback) timer2_state.compare_callback();
}

err_t timer2_set_mode(uint8_t mode)
{
    if(mode > 0x3) return E_INVALID_VALUE;
    
    //Подождём флаг готовности регистра.
    BIT_WAIT_OFF(ASSR, TCR2UB);
    //Очистим биты режима.
    TCCR2 &= ~((1 << WGM21) | (1 << WGM20));
    //Подождём флаг готовности регистра.
    BIT_WAIT_OFF(ASSR, TCR2UB);
    //Установим.
    TCCR2 |= (((mode & 0x2) >> 1) << WGM21) |
             ((mode & 0x1) << WGM20);
    //Подождём флаг готовности регистра.
    BIT_WAIT_OFF(ASSR, TCR2UB);
    
    return E_NO_ERROR;
}

err_t timer2_set_com_mode(uint8_t com_mode)
{
    if(com_mode > 0x3) return E_INVALID_VALUE;
    //для PWM режимов работы таймера нет режима TOGGLE.
    if((TCCR2 & WGM20) && com_mode == 0x1) return E_INVALID_VALUE;
    
    //Подождём флаг готовности регистра.
    BIT_WAIT_OFF(ASSR, TCR2UB);
    //Очистим биты режима.
    TCCR2 &= ~((1 << COM21) | (1 << COM20));
    //Подождём флаг готовности регистра.
    BIT_WAIT_OFF(ASSR, TCR2UB);
    //Установим.
    TCCR2 |= (((com_mode & 0x2) >> 1) << COM21) |
             ((com_mode & 0x1) << COM20);
    //Подождём флаг готовности регистра.
    BIT_WAIT_OFF(ASSR, TCR2UB);
    
    return E_NO_ERROR;
}

err_t timer2_set_clock(uint8_t clock)
{
    if(clock > 0x7) return E_INVALID_VALUE;
    
    timer2_state.clock = clock;
    
    return E_NO_ERROR;
}

timer_callback_t timer2_set_overflow_callback(timer_callback_t callback)
{
    timer_callback_t prev_callback = timer2_state.overflow_callback;
    
    //Запретим прерывание.
    BIT_OFF(TIMSK, TOIE2);
    //Установим каллбэк.
    timer2_state.overflow_callback = callback;
    //И разрешим снова, если нужно.
    if(timer2_state.overflow_callback){
        BIT_ON(TIMSK, TOIE2);
    }
    
    return prev_callback;
}

timer_callback_t timer2_set_compare_match_callback(timer_callback_t callback)
{
    timer_callback_t prev_callback = timer2_state.compare_callback;
    
    //Запретим прерывание.
    BIT_OFF(TIMSK, OCIE2);
    //Установим каллбэк.
    timer2_state.compare_callback = callback;
    //И разрешим снова, если нужно.
    if(timer2_state.compare_callback){
        BIT_ON(TIMSK, OCIE2);
    }
    
    return prev_callback;
}

err_t timer2_start()
{
    if(timer2_state.clock == 0) return E_TIMER_INVALID_CLOCK;
    
    //сбросим значение часов.
    timer2_stop();
    //Подождём флаг готовности регистра.
    BIT_WAIT_OFF(ASSR, TCR2UB);
    //Установим.
    TCCR2 |= (((timer2_state.clock & 0x4) >> 2) << CS22) |
             (((timer2_state.clock & 0x2) >> 1) << CS21) |
             ((timer2_state.clock & 0x1) << CS20);
    //Подождём флаг готовности регистра.
    BIT_WAIT_OFF(ASSR, TCR2UB);
    
    return E_NO_ERROR;
}

void timer2_stop()
{
    //Подождём флаг готовности регистра.
    BIT_WAIT_OFF(ASSR, TCR2UB);
    //Очистим биты источника тактирования.
    TCCR2 &= ~((1 << CS22) | (1 << CS21) | (1 << CS20));
    //Подождём флаг готовности регистра.
    BIT_WAIT_OFF(ASSR, TCR2UB);
}
