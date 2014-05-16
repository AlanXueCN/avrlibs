/**
 * @file lcd44780.h
 * Библиотека для работы с LCD на контроллере, совместимом с HD44780.
 */

#ifndef LCD44780_H
#define	LCD44780_H

#include <stdbool.h>
#include <stdint.h>
#include "ports/ports.h"
#include "errors/errors.h"

//! Ширина шины данных.
//! 4 бит.
#define LCD44780_DATA_BUS_WIDTH_4BIT 0
//! 8 бит.
#define LCD44780_DATA_BUS_WIDTH_8BIT 1
#define LCD44780_DATA_BUS_WIDTH_MAX_BIT LCD44780_DATA_BUS_WIDTH_8BIT

//! Тип ширины шины данных.
typedef uint8_t lcd44780_data_width_t;

//! Режимы движения дисплея/курсора при записи.
//! Смещение курсора(адреса) при записи.
//! Вправо (инкремент).
#define LCD44780_ENTRY_MODE_CURSOR_INCREMENT    1
//! Влево (декремент).
#define LCD44780_ENTRY_MODE_CURSOR_DECREMENT    0
//! Смещение дисплея при записи.
//! Влево.
#define LCD44780_ENTRY_MODE_DISPLAY_SHIFT_LEFT    1
//! Вправо.
#define LCD44780_ENTRY_MODE_DISPLAY_SHIFT_RIGHT   0

//! Тип режима движения.
typedef uint8_t lcd44780_entry_mode_t;

//! Перемещение дисплея/курсора без записи.
//! Смещаемый элемент.
//! Дисплей.
#define LCD44780_SHIFT_DISPLAY          1
//! Курсор.
#define LCD44780_SHIFT_CURSOR           0
//! Направление смещения.
//! Вправо.
#define LCD44780_SHIFT_RIGHT            1
//! Влево.
#define LCD44780_SHIFT_LEFT             0

//! Тип элемента перемещения.
typedef uint8_t lcd44780_shift_entry_t;
//! Тип направления перемещения.
typedef uint8_t lcd44780_shift_direction_t;

//! Конфигурация дисплея.
//! Число линий.
//! Одна линия.
#define LCD44780_LINE_NUMBER_1          0
//! Две линии.
#define LCD44780_LINE_NUMBER_2          1
//! Тип шрифта.
//! 5x8
#define LCD44780_FONT_TYPE_5X8          0
//! 5x11
#define LCD44780_FONT_TYPE_5X11         1

//! Тип числа линий.
typedef uint8_t lcd44780_line_number_t;
//! Тип типа шрифта.
typedef uint8_t lcd44780_font_type_t;

//! Адреса строк дисплея.
//! Адреса дисплея 8x1 или 16x1.
//! Первая строка.
#define LCD44780_8X1_LINE_0      0x0
#define LCD44780_16X1_LINE_0     0x0

//! Адреса дисплея 16x2 или 8x2.
//! Первая строка.
#define LCD44780_8X2_LINE_0      0x0
#define LCD44780_16X2_LINE_0     0x0
//! Вторая строка.
#define LCD44780_8X2_LINE_1      0x40
#define LCD44780_16X2_LINE_1     0x40

//! Адреса дисплея 16x4.
//! Первая строка.
#define LCD44780_16X4_LINE_0     0x0
//! Вторая строка.
#define LCD44780_16X4_LINE_1     0x40
//! Третья строка.
#define LCD44780_16X4_LINE_2     0x10
//! Четвёртая строка.
#define LCD44780_16X4_LINE_3     0x50

//! Адреса дисплея 20x2.
//! Первая строка.
#define LCD44780_20X2_LINE_0     0x0
//! Вторая строка.
#define LCD44780_20X2_LINE_1     0x40

//! Адреса дисплея 20x4.
//! Первая строка.
#define LCD44780_20X4_LINE_0     0x0
//! Вторая строка.
#define LCD44780_20X4_LINE_1     0x40
//! Третья строка.
#define LCD44780_20X4_LINE_2     0x14
//! Четвёртая строка.
#define LCD44780_20X4_LINE_3     0x54

//! Адреса дисплея 40x2.
//! Первая строка.
#define LCD44780_40X2_LINE_0     0x0
//! Вторая строка.
#define LCD44780_40X2_LINE_1     0x40

/**
 * Структура LCD.
 */
typedef struct _Lcd44780 {
    pin_range_t pins_data;
    pin_t pin_rs;
    pin_t pin_rw;
    pin_t pin_e;
    lcd44780_data_width_t data_bus_width;
}lcd44780_t;

/**
 * Инициализирует LCD.
 * @param lcd LCD.
 * @param data_port Порт данных.
 * @param data_offset Смещение пинов данных в порту.
 * @param data_bus_width ширина шины данных.
 * @param rs_port Порт входа RS.
 * @param rs_pin_n Номер пина входа RS.
 * @param rw_port Порт входа RW.
 * @param rw_pin_n Номер пина входа RW.
 * @param e_port Порт входа E.
 * @param e_pin_n Номер пина входа E.
 * @return Код ошибки.
 */
extern err_t lcd44780_init(lcd44780_t* lcd,
                    uint8_t data_port, uint8_t data_offset,
                    lcd44780_data_width_t data_bus_width,
                    uint8_t rs_port, uint8_t rs_pin_n,
                    uint8_t rw_port, uint8_t rw_pin_n,
                    uint8_t e_port, uint8_t e_pin_n);

/**
 * Проверяет занятость LCD.
 * @param lcd LCD.
 * @return флаг занятости LCD.
 */
extern bool lcd44780_busy(lcd44780_t* lcd);

/**
 * Ждёт сброса флага занятости LCD.
 * @param lcd LCD.
 * @return флаг освобождения LCD.
 */
extern bool lcd44780_wait(lcd44780_t* lcd);

/**
 * Получает текущий адрес.
 * @param lcd LCD.
 * @return Адрес.
 */
extern uint8_t lcd44780_address(lcd44780_t* lcd);

/**
 * Очищает LCD.
 * @param lcd LCD.
 */
extern err_t lcd44780_clear(lcd44780_t* lcd);

/**
 * Возвращает курсор LCD домой.
 * @param lcd LCD.
 */
extern err_t lcd44780_home(lcd44780_t* lcd);

/**
 * Устанавливает режимы смещения курсора/дисплея.
 * @param lcd LCD.
 * @param cursor_mode Режим смещения курсора.
 * @param display_mode Режим смещения дисплея.
 */
extern err_t lcd44780_entry_mode(lcd44780_t* lcd, lcd44780_entry_mode_t cursor_mode, lcd44780_entry_mode_t display_mode);

/**
 * Устанавливает включенность дисплея, курсора и мигания курсора.
 * @param lcd LCD.
 * @param display_on флаг включения дисплея.
 * @param cursor_on флаг включения курсора.
 * @param cursor_blink_on флаг включения моргания курсора.
 */
extern err_t lcd44780_control(lcd44780_t* lcd, bool display_on, bool cursor_on, bool cursor_blink_on);

/**
 * Смещает элемент дисплея в заданном направлении.
 * @param lcd LCD.
 * @param shift_entry Смещаемый элемент.
 * @param shift_direction Направление смещения.
 */
extern err_t lcd44780_shift(lcd44780_t* lcd, lcd44780_shift_entry_t shift_entry, lcd44780_shift_direction_t shift_direction);

/**
 * Конфигурирует дисплей.
 * @param lcd LCD.
 * @param line_number Число линий (задаётся константой).
 * @param font_type Тип шрифта.
 */
extern err_t lcd44780_configure(lcd44780_t* lcd, lcd44780_line_number_t line_number, lcd44780_font_type_t font_type);

/**
 * Устанавливает курсор на CGRAM на заданный адрес.
 * @param lcd LCD.
 * @param address Адрес.
 */
extern err_t lcd44780_set_cgram(lcd44780_t* lcd, uint8_t address);

/**
 * Устанавливает курсор на DDRAM на заданный адрес.
 * @param lcd LCD.
 * @param address Адрес.
 */
extern err_t lcd44780_set_ddram(lcd44780_t* lcd, uint8_t address);

/**
 * Записывает в LCD символ.
 * @param lcd LCD.
 * @param c Символ.
 */
extern err_t lcd44780_putc(lcd44780_t* lcd, char c);

/**
 * Читает из LCD символ.
 * Устанавливает переменную errno.
 * @param lcd LCD.
 * @return символ.
 */
extern char lcd44780_getc(lcd44780_t* lcd);

/**
 * Записывает в lcd строку символов.
 * @param lcd LCD.
 * @param s ASCIIz строка.
 */
extern err_t lcd44780_puts(lcd44780_t* lcd, const char* s);

/**
 * Записывает символы в LCD.
 * @param lcd LCD.
 * @param s Массив символов.
 * @param size Размер массива символов.
 */
extern err_t lcd44780_write(lcd44780_t* lcd, const char* s, uint8_t size);

#endif	/* LCD44780_H */

