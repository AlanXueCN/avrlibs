#include "int1.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stddef.h>
#include "bits/bits.h"
#include "utils/utils.h"

// Имена регистров.
#define INT1_REG_INT    GICR
#define INT1_BIT_INT    INT1
#define INT1_REG_CONF   MCUCR
#define INT1_BIT_ISC0   ISC10
#define INT1_BIT_ISC1   ISC11

// Сохранение и запрещение и восстановление разрешения прерывания int1.
#define __int1_interrupt_save_disable() __interrupts_save_disable_in(INT1_REG_INT, INT1_BIT_INT)
#define __int1_interrupt_restrore() __interrupts_restore_in(INT1_REG_INT, INT1_BIT_INT)

// Максимальные значения.
#define INT1_SENSE_CONTROL_MAX  INT1_SENSE_CONTROL_RISING_EDGE

typedef struct _Int0{
    pin_t pin;
    int1_callback_t callback;
}int1_t;

int1_t int1;


ISR(INT1_vect)
{
    if(int1.callback) int1.callback();
}

err_t int1_init(uint8_t port_n, uint8_t pin_n, bool need_pullup)
{
    int1.callback = NULL;
    
    err_t err = pin_init(&int1.pin, port_n, pin_n);
    if(err != E_NO_ERROR) return err;
    
    pin_set_in(&int1.pin);
    
    if(need_pullup) pin_pullup_enable(&int1.pin);
    
    return E_NO_ERROR;
}

pin_t* int1_pin(void)
{
    return &int1.pin;
}

int1_callback_t int1_callback(void)
{
    return int1.callback;
}

void int1_set_callback(int1_callback_t callback)
{
    __int1_interrupt_save_disable();
    
    int1.callback = callback;
    
    __int1_interrupt_restrore();
}

int1_sense_control_t int1_sense_control(void)
{
    int1_sense_control_t res = 0;
    
    if(BIT_TEST(INT1_REG_CONF, INT1_BIT_ISC0)) BIT_ON(res, 0);
    if(BIT_TEST(INT1_REG_CONF, INT1_BIT_ISC1)) BIT_ON(res, 1);
    
    return res;
}

err_t int1_set_sense_control(int1_sense_control_t sense_control)
{
    if(sense_control > INT1_SENSE_CONTROL_MAX) return E_INVALID_VALUE;
    
    __int1_interrupt_save_disable();
    
    BIT_SET(INT1_REG_CONF, INT1_BIT_ISC0, sense_control); sense_control >>= 1;
    BIT_SET(INT1_REG_CONF, INT1_BIT_ISC1, sense_control);
    
    __int1_interrupt_restrore();
    
    return E_NO_ERROR;
}

bool int1_enabled(void)
{
    return BIT_VALUE(INT1_REG_INT, INT1_BIT_INT);
}

void int1_enable(void)
{
    BIT_ON(INT1_REG_INT, INT1_BIT_INT);
}

void int1_disable(void)
{
    BIT_OFF(INT1_REG_INT, INT1_BIT_INT);
}
