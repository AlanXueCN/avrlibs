/**
 * @file display_7seg.h
 * Общие функции для 7-сегментных индикаторов по работе с символами.
 */

#ifndef DISPLAY_7SEG_H
#define	DISPLAY_7SEG_H

#include <stdint.h>
#include "display_7seg_codes.h"


/**
 * Получает код переданной цифры.
 * @param digit Цифра.
 * @return Код для отображения на дисплее.
 */
extern value7seg_t display_7seg_digit_code(uint8_t digit);

/**
 * Получает код символа.
 * @param c Символ.
 * @return Код для отображения на дисплее.
 */
extern value7seg_t display_7seg_char_code(char c);

/**
 * Преобразует символы строки в значения для отображения на дисплее.
 * @param values Значения для отображения.
 * @param str Строка.
 */
extern void display_7seg_convert(value7seg_t* values, const char* str);

#endif	/* DISPLAY_7SEG_H */

