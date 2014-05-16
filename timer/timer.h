/**
 * @file timer.h
 * Общие типы и значениея для таймеров.
 */

#ifndef TIMER_H
#define	TIMER_H

#include "errors/errors.h"

#define E_TIMER (E_USER + 40)
#define E_TIMER_INVALID_CLOCK (E_TIMER + 1)

typedef void (*timer_callback_t)(void);

#endif	/* TIMER_H */

