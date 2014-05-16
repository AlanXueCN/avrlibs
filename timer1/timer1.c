#include "timer1.h"
#include <avr/interrupt.h>


/*
 * TIMER 1
 */

typedef struct _Timer1_State {
    uint8_t clock;
    timer_callback_t overflow_callback;
    timer_callback_t compare_a_callback;
    timer_callback_t compare_b_callback;
    timer_callback_t capture_callback;
}timer1_state_t;

timer1_state_t timer1_state = {0};

ISR(TIMER1_OVF_vect)
{
    if(timer1_state.overflow_callback) timer1_state.overflow_callback();
}

ISR(TIMER1_COMPA_vect)
{
    if(timer1_state.compare_a_callback) timer1_state.compare_a_callback();
}

ISR(TIMER1_COMPB_vect)
{
    if(timer1_state.compare_b_callback) timer1_state.compare_b_callback();
}

ISR(TIMER1_CAPT_vect)
{
    if(timer1_state.capture_callback) timer1_state.capture_callback();
}


err_t timer1_set_mode(uint8_t mode)
{
    if(mode > 0xf || mode == TIMER1_MODE_RESERVED) return E_INVALID_VALUE;
    
    //Очистим биты режима.
    TCCR1B &= ~((1 << WGM13) | (1 << WGM12));
    TCCR1A &= ~((1 << WGM11) | (1 << WGM10));
    //Установим.
    TCCR1B |= (((mode & 0x8) >> 3) << WGM13) |
              (((mode & 0x4) >> 2) << WGM12);
    TCCR1A |= (((mode & 0x2) >> 1) << WGM11) |
              ( (mode & 0x1) << WGM10);
    
    return E_NO_ERROR;
}

err_t timer1_set_com_a_mode(uint8_t com_mode)
{
    if(com_mode > 0x3) return E_INVALID_VALUE;
    
    //Очистим биты режима.
    TCCR1A &= ~((1 << COM1A1) | (1 << COM1A0));
    //Установим.
    TCCR1A |= (((com_mode & 0x2) >> 1) << COM1A1) |
              ((com_mode & 0x1) << COM1A0);
    
    return E_NO_ERROR;
}

err_t timer1_set_com_b_mode(uint8_t com_mode)
{
    if(com_mode > 0x3) return E_INVALID_VALUE;
    
    //Очистим биты режима.
    TCCR1A &= ~((1 << COM1B1) | (1 << COM1B0));
    //Установим.
    TCCR1A |= (((com_mode & 0x2) >> 1) << COM1B1) |
              ((com_mode & 0x1) << COM1B0);
    
    return E_NO_ERROR;
}

err_t timer1_set_clock(uint8_t clock)
{
    if(clock > 0x7) return E_INVALID_VALUE;
    
    timer1_state.clock = clock;
    
    return E_NO_ERROR;
}

timer_callback_t timer1_set_overflow_callback(timer_callback_t callback)
{
    timer_callback_t prev_callback = timer1_state.overflow_callback;
    
    //Запретим прерывание.
    BIT_OFF(TIMSK, TOIE1);
    //Установим каллбэк.
    timer1_state.overflow_callback = callback;
    //И разрешим снова, если нужно.
    if(timer1_state.overflow_callback){
        BIT_ON(TIMSK, TOIE1);
    }
    
    return prev_callback;
}

timer_callback_t timer1_set_compare_match_a_callback(timer_callback_t callback)
{
    timer_callback_t prev_callback = timer1_state.compare_a_callback;
    
    //Запретим прерывание.
    BIT_OFF(TIMSK, OCIE1A);
    //Установим каллбэк.
    timer1_state.compare_a_callback = callback;
    //И разрешим снова, если нужно.
    if(timer1_state.compare_a_callback){
        BIT_ON(TIMSK, OCIE1A);
    }
    
    return prev_callback;
}

timer_callback_t timer1_set_compare_match_b_callback(timer_callback_t callback)
{
    timer_callback_t prev_callback = timer1_state.compare_b_callback;
    
    //Запретим прерывание.
    BIT_OFF(TIMSK, OCIE1B);
    //Установим каллбэк.
    timer1_state.compare_b_callback = callback;
    //И разрешим снова, если нужно.
    if(timer1_state.compare_b_callback){
        BIT_ON(TIMSK, OCIE1B);
    }
    
    return prev_callback;
}

timer_callback_t timer1_set_capture_callback(timer_callback_t callback)
{
    timer_callback_t prev_callback = timer1_state.capture_callback;
    
    //Запретим прерывание.
    BIT_OFF(TIMSK, TICIE1);
    //Установим каллбэк.
    timer1_state.capture_callback = callback;
    //И разрешим снова, если нужно.
    if(timer1_state.capture_callback){
        BIT_ON(TIMSK, TICIE1);
    }
    
    return prev_callback;
}

err_t timer1_start()
{
    if(timer1_state.clock == 0) return E_TIMER_INVALID_CLOCK;
    
    //сбросим значение часов.
    timer1_stop();
    //Установим.
    TCCR1B |= (((timer1_state.clock & 0x4) >> 2) << CS12) |
             (((timer1_state.clock & 0x2) >> 1) << CS11) |
             ((timer1_state.clock & 0x1) << CS10);
    
    return E_NO_ERROR;
}

void timer1_stop()
{
    //Очистим биты источника тактирования.
    TCCR1B &= ~((1 << CS12) | (1 << CS11) | (1 << CS10));
}
