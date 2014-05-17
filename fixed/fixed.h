/**
 * @file fixed.h
 * Библиотека для работы с числами с фиксированной запятой.
 */

#ifndef FIXED_H
#define	FIXED_H

#include <stdint.h>
#include "defs/defs.h"

#define FIXED_PART_SIZE 8
//! Целочисленный тип нужной размерности.
typedef int16_t fixed_int_t;
//! Целочисленный тип половинной размерности.
typedef int8_t fixed_half_t;
//! Число с фиксированной запятой.
typedef fixed_int_t fixed_t;

#define fixed_make_int(i) (((fixed_int_t)i) << FIXED_PART_SIZE)
#define fixed_make_int32(i) (((int32_t)i) << FIXED_PART_SIZE)
#define fixed_make_fract(dividend, divider) (fixed_make_int(dividend) / divider)
#define fixed_make_fract32(dividend, divider) (fixed_make_int32(dividend) / divider)

/**
 * Создаёт число с фиксированной запятой.
 * @param i Целое.
 * @return Число с фиксированной запятой.
 */
extern fixed_t fixed_from_int(int8_t i);

/**
 * Создаёт число с фиксированной запятой.
 * @param dividend Делимое.
 * @param divider Делитель.
 * @return Число с фиксированной запятой.
 */
extern fixed_t fixed_from_fract(int16_t dividend, int16_t divider);

/**
 * Получает модуль числа с фиксированной запятой.
 * @param f Число с фиксированной запятой.
 * @return Модуль числа с фиксированной запятой.
 */
extern fixed_t fixed_abs(fixed_t f);

/**
 * Округляет число с фиксированной запятой.
 * @param f Число с фиксированной запятой.
 * @return Округлённое число с фиксированной запятой.
 */
extern fixed_t fixed_round(fixed_t f);

/**
 * Получает целую часть числа с фиксированной запятой.
 * @param f Число с фиксированной запятой.
 * @return Целая часть числа с фиксированной запятой.
 */
extern fixed_half_t fixed_get_int(fixed_t f);

/**
 * Получает дробную часть числа с фиксированной запятой.
 * @param f Число с фиксированной запятой.
 * @return Дробная часть числа с фиксированной запятой.
 */
extern fixed_half_t fixed_get_fract(fixed_t f);


#endif	/* FIXED_H */

