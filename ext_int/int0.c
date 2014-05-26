#include "int0.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stddef.h>
#include "bits/bits.h"
#include "utils/utils.h"

// Имена регистров.
#define INT0_REG_INT    GICR
#define INT0_BIT_INT    INT0
#define INT0_REG_CONF   MCUCR
#define INT0_BIT_ISC0   ISC00
#define INT0_BIT_ISC1   ISC01

// Сохранение и запрещение и восстановление разрешения прерывания int0.
#define __int0_interrupt_save_disable() __interrupts_save_disable_in(INT0_REG_INT, INT0_BIT_INT)
#define __int0_interrupt_restrore() __interrupts_restore_in(INT0_REG_INT, INT0_BIT_INT)

// Максимальные значения.
#define INT0_SENSE_CONTROL_MAX  INT0_SENSE_CONTROL_RISING_EDGE

typedef struct _Int0{
    pin_t pin;
    int0_callback_t callback;
}int0_t;

int0_t int0;


ISR(INT0_vect)
{
    if(int0.callback) int0.callback();
}

err_t int0_init(uint8_t port_n, uint8_t pin_n, bool need_pullup)
{
    int0.callback = NULL;
    
    err_t err = pin_init(&int0.pin, port_n, pin_n);
    if(err != E_NO_ERROR) return err;
    
    pin_set_in(&int0.pin);
    
    if(need_pullup) pin_pullup_enable(&int0.pin);
    
    return E_NO_ERROR;
}

pin_t* int0_pin(void)
{
    return &int0.pin;
}

int0_callback_t int0_callback(void)
{
    return int0.callback;
}

void int0_set_callback(int0_callback_t callback)
{
    __int0_interrupt_save_disable();
    
    int0.callback = callback;
    
    __int0_interrupt_restrore();
}

int0_sense_control_t int0_sense_control(void)
{
    int0_sense_control_t res = 0;
    
    if(BIT_TEST(INT0_REG_CONF, INT0_BIT_ISC0)) BIT_ON(res, 0);
    if(BIT_TEST(INT0_REG_CONF, INT0_BIT_ISC1)) BIT_ON(res, 1);
    
    return res;
}

err_t int0_set_sense_control(int0_sense_control_t sense_control)
{
    if(sense_control > INT0_SENSE_CONTROL_MAX) return E_INVALID_VALUE;
    
    __int0_interrupt_save_disable();
    
    BIT_SET(INT0_REG_CONF, INT0_BIT_ISC0, sense_control); sense_control >>= 1;
    BIT_SET(INT0_REG_CONF, INT0_BIT_ISC1, sense_control);
    
    __int0_interrupt_restrore();
    
    return E_NO_ERROR;
}

bool int0_enabled(void)
{
    return BIT_VALUE(INT0_REG_INT, INT0_BIT_INT);
}

void int0_enable(void)
{
    BIT_ON(INT0_REG_INT, INT0_BIT_INT);
}

void int0_disable(void)
{
    BIT_OFF(INT0_REG_INT, INT0_BIT_INT);
}
