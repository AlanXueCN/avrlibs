 /**
 * @file timer0.h
 * Библиотека для работы с таймером 0.
 */

#ifndef TIMER0_H
#define TIMER0_H

#include <avr/io.h>

#if defined(OCR0)
    #include "timer0.h_m16"
#else
    #include "timer0.h_m8"
#endif

#endif  /* TIMER0_H */
