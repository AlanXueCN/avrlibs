#include "dpy7ser.h"
#include "utils/utils.h"
#include "utils/delay.h"
#include "bits/bits.h"
#include "defs/defs.h"


#define DPY7SER_SERIAL_DELAY_US 20


extern err_t dpy7ser_init(dpy7ser_t* dpy,
                        uint8_t pin_data_port_n,
                        uint8_t pin_data_n,
                        uint8_t pin_clk_port_n,
                        uint8_t pin_clk_n,
                        uint8_t pin_swap_port_n,
                        uint8_t pin_swap_n,
                        //uint8_t pin_reset_port_n,
                        //uint8_t pin_reset_n,
                        bool ind_polarity,
                        bool bits_order)
{
    err_t res = pin_init(&dpy->pin_data, pin_data_port_n, pin_data_n);
    if(res != E_NO_ERROR) return res;
    
    res = pin_init(&dpy->pin_clk, pin_clk_port_n, pin_clk_n);
    if(res != E_NO_ERROR) return res;
    
    res = pin_init(&dpy->pin_swap, pin_swap_port_n, pin_swap_n);
    if(res != E_NO_ERROR) return res;
    
    //res = pin_init(&dpy->pin_reset, pin_reset_port_n, pin_reset_n);
    //if(res != E_NO_ERROR) return res;
    
    dpy->ind_polarity = ind_polarity;
    dpy->bits_order = bits_order;
    
    pin_set_out(&dpy->pin_data);
    pin_set_out(&dpy->pin_clk);
    pin_set_out(&dpy->pin_swap);
    
    return E_NO_ERROR;
}

ALWAYS_INLINE static void dpy7ser_bus_clear(dpy7ser_t* dpy)
{
    pin_off(&dpy->pin_data);
    pin_off(&dpy->pin_clk);
    pin_off(&dpy->pin_swap);
}

ALWAYS_INLINE static void dpy7ser_bus_apply(dpy7ser_t* dpy)
{
    pin_on(&dpy->pin_swap);
    delay_us8(DPY7SER_SERIAL_DELAY_US);
    dpy7ser_bus_clear(dpy);
}

ALWAYS_INLINE static void dpy7ser_bus_put_bit(dpy7ser_t* dpy, uint8_t bit)
{
    delay_us8(DPY7SER_SERIAL_DELAY_US);
    
    if(bit) pin_on(&dpy->pin_data);
    else pin_off(&dpy->pin_data);
    
    pin_on(&dpy->pin_clk);
    
    delay_us8(DPY7SER_SERIAL_DELAY_US);
    
    pin_off(&dpy->pin_clk);
}

static void dpy7ser_bus_put_byte(dpy7ser_t* dpy, uint8_t byte)
{
    uint8_t i_bit = 0;
    uint8_t bit = 0;
    
    if(!dpy->ind_polarity){
        byte = ~byte;
    }
    if(dpy->bits_order){
        for(i_bit = 0; i_bit < 8; i_bit ++){
            bit = byte & 0x80;
            byte <<= 1;
            dpy7ser_bus_put_bit(dpy, bit);
        }
    }else{
        for(i_bit = 0; i_bit < 8; i_bit ++){
            bit = byte & 0x1;
            byte >>= 1;
            dpy7ser_bus_put_bit(dpy, bit);
        }
    }
}

void dpy7ser_write(dpy7ser_t* dpy, const value7seg_t* values, uint8_t count)
{
    uint8_t i = 0;
    
    if(count == 0) return;
    
    dpy7ser_bus_clear(dpy);
    
    for(i = count - 1; ; i --){
        dpy7ser_bus_put_byte(dpy, values[i]);
        
        if(i == 0) break;
    }
    
    dpy7ser_bus_apply(dpy);
}

void dpy7ser_put(dpy7ser_t* dpy, value7seg_t value)
{
    dpy7ser_bus_clear(dpy);
    dpy7ser_bus_put_byte(dpy, value);
    dpy7ser_bus_apply(dpy);
}

