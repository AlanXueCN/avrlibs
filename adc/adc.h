 /**
 * @file adc.h
 * Библиотека для работы с ADC.
 */

#ifndef ADC_H
#define ADC_H

#include <avr/io.h>

#if defined(MUX4)
    #include "adc.h_m16"
#else
    #include "adc.h_m8"
#endif

#endif  /* ADC_H */
