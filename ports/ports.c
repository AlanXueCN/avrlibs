#include <stdbool.h>
#include <stddef.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include "bits/bits.h"
#include "ports.h"



static volatile uint8_t* const __flash ports_addrs[] = {
#ifdef PORTA
    &PORTA, &PINA, &DDRA
#else
    NULL, NULL, NULL
#endif

#ifdef PORTB
    ,&PORTB, &PINB, &DDRB
#else
    ,NULL, NULL, NULL
#endif

#ifdef PORTC
    ,&PORTC, &PINC, &DDRC
#else
    ,NULL, NULL, NULL
#endif

#ifdef PORTD
    ,&PORTD, &PIND, &DDRD
#else
    ,NULL, NULL, NULL
#endif

#ifdef PORTE
    ,&PORTE, &PINE, &DDRE
#else
    ,NULL, NULL, NULL
#endif

#ifdef PORTF
    ,&PORTF, &PINF, &DDRF
#else
    ,NULL, NULL, NULL
#endif
};

static inline bool port_is_valid_number(uint8_t port_number)
{
    return port_number < ((sizeof(ports_addrs) / sizeof(volatile uint8_t*)) / 3);
}

inline volatile uint8_t* port_get_out(uint8_t port_n)
{
    //if(!port_is_valid_number(port_n)) return NULL;
    return ports_addrs[port_n + port_n + port_n + 0];
}

inline volatile uint8_t* port_get_in(uint8_t port_n)
{
    //if(!port_is_valid_number(port_n)) return NULL;
    return ports_addrs[port_n + port_n + port_n + 1];
}

inline volatile uint8_t* port_get_ddr(uint8_t port_n)
{
    //if(!port_is_valid_number(port_n)) return NULL;
    return ports_addrs[port_n + port_n + port_n + 2];
}

err_t port_init(port_t* port, uint8_t port_n)
{
    if(!port_is_valid_number(port_n)) return E_PORTS_INVALID_PORT_NUMBER;
    if((port->out = port_get_out(port_n)) == NULL) return E_PORTS_INVALID_PORT_OUT;
    if((port->in = port_get_in(port_n)) == NULL) return E_PORTS_INVALID_PORT_IN;
    if((port->ddr = port_get_ddr(port_n)) == NULL) return E_PORTS_INVALID_PORT_DDR;
    return E_NO_ERROR;
}

err_t pin_init(pin_t* pin, uint8_t port_n, uint8_t pin_n)
{
    if(pin_n >= 8) return E_PORTS_INVALID_PIN_NUMBER;
    pin->pin_n = pin_n;
    pin->_port_mask = BIT(pin_n);
    return port_init(&pin->port, port_n);
}

err_t pin_range_init(pin_range_t* pin_range, uint8_t port_n, uint8_t pins_offset, uint8_t pins_count)
{
    if(pins_offset >= 8 ) return E_PORTS_INVALID_PINS_OFFSET;
    if(pins_count  >  8 ) return E_PORTS_INVALID_PINS_COUNT;
    if(pins_count  == 0 ) return E_PORTS_INVALID_PINS_COUNT;
    
    pin_range->count = pins_count;
    pin_range->offset = pins_offset;
    pin_range->_port_mask = BIT_MAKE_MASK(pins_count, pins_offset);
    
    return port_init(&pin_range->port, port_n);
}
