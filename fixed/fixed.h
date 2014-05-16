/**
 * @file fixed.h
 * Библиотека для работы с числами с фиксированной запятой.
 */

#ifndef FIXED_H
#define	FIXED_H

#include <stdint.h>
#include "defs/defs.h"

#define FIXED_PART_SIZE 8
typedef int16_t fixed_int_t;
//Число с фиксированной запятой.
typedef fixed_int_t fixed_t;

#define fixed_make_int(i) (((fixed_int_t)i) << FIXED_PART_SIZE)
#define fixed_make_fract(dividend, divider) (fixed_make_int(dividend) / divider)

/**
 * Создаёт число с фиксированной запятой.
 * @param i Целое.
 * @return Число с фиксированной запятой.
 */
ALWAYS_INLINE static fixed_t fixed_from_int(int8_t i)
{
    return fixed_make_int(i);
}

/**
 * Создаёт число с фиксированной запятой.
 * @param dividend Делимое.
 * @param divider Делитель.
 * @return Число с фиксированной запятой.
 */
ALWAYS_INLINE static fixed_t fixed_from_fract(int16_t dividend, int16_t divider)
{
    return fixed_make_fract(dividend, divider);
}

/**
 * Получает модуль числа с фиксированной запятой.
 * @param f Число с фиксированной запятой.
 * @return Модуль числа с фиксированной запятой.
 */
ALWAYS_INLINE static fixed_t fixed_abs(fixed_t f)
{
    return f >= 0 ? f : -f;
}

/**
 * Округляет число с фиксированной запятой.
 * @param f Число с фиксированной запятой.
 * @return Округлённое число с фиксированной запятой.
 */
ALWAYS_INLINE static int16_t fixed_round(fixed_t f)
{
    return (f + 128) & 0xff00;
}

/**
 * Получает целую часть числа с фиксированной запятой.
 * @param f Число с фиксированной запятой.
 * @return Целая часть числа с фиксированной запятой.
 */
ALWAYS_INLINE static int8_t fixed_get_int(fixed_t f)
{
    if(f < 0) return -((-f) >> 8);
    return (f >> 8);
}

/**
 * Получает дробную часть числа с фиксированной запятой.
 * @param f Число с фиксированной запятой.
 * @return Дробная часть числа с фиксированной запятой.
 */
ALWAYS_INLINE static int8_t fixed_get_fract(fixed_t f)
{
    if(f < 0){
        return (fixed_int_t)100 * (fixed_int_t)(-((-f) & 0xff)) / (fixed_int_t)256;
    }
    return (fixed_int_t)100 * (fixed_int_t)(f & 0xff) / (fixed_int_t)256;
}


#endif	/* FIXED_H */

