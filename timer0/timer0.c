#ifndef TIMER0_C
#define TIMER0_C

#include <avr/io.h>

#if defined(OCR0)
    #include "timer0.c_m16"
#else
    #include "timer0.c_m8"
#endif

#endif  /* TIMER0_C */
