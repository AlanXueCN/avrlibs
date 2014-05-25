#include "fixed.h"

fixed_t fixed_from_int(int8_t i)
{
    return fixed_make_from_int(i);
}

fixed_t fixed_from_fract(int16_t dividend, int16_t divider)
{
    if(dividend > 127 || dividend < -128){
        return fixed_make_from_fract32(dividend, divider);
    }
    return fixed_make_from_fract(dividend, divider);
}

fixed_t fixed_abs(fixed_t f)
{
    return f >= 0 ? f : -f;
}

fixed_t fixed_round(fixed_t f)
{
    return (f + 128) & 0xff00;
}

fixed_half_t fixed_get_int(fixed_t f)
{
    if(f < 0) return -((-f) >> 8);
    return (f >> 8);
}

fixed_half_t fixed_get_fract(fixed_t f)
{
    fixed_int_t value;
    if(f >= 0){
        value = (fixed_int_t)(f & 0xff);
    }else{
        value = (fixed_int_t)(-((-f) & 0xff));
    }
    return (value * 100) / 256;
}
