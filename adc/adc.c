#ifndef ADC_C
#define ADC_C

#include <avr/io.h>

#if defined(MUX4)
    #include "adc.c_m16"
#else
    #include "adc.c_m8"
#endif

#endif  /* ADC_C */
