/**
 * @file lcd0108.h
 * Библиотека для работы с LCD на контроллере, совместимом с KS0108.
 */

#ifndef LCD0108_H
#define	LCD0108_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "ports/ports.h"
#include "errors/errors.h"

/**
 * Структура LCD.
 */
typedef struct _Lcd0108 {
    bool inverse_cs;
    port_t port_data;
    pin_t pin_reset;
    pin_t pin_cs0;
    pin_t pin_cs1;
    pin_t pin_rs;
    pin_t pin_rw;
    pin_t pin_e;
}lcd0108_t;

//! Размер экрана.
//! Ширина.
#define LCD0108_WIDTH   128
//! Высота.
#define LCD0108_HEIGHT  64
//! Высота в страницах.
#define LCD0108_PAGES_HEIGHT 8

//! Максимальные значения адресации.
//! Максимальный номер страницы.
#define LCD0108_PAGE_MAX        7
//! Максимальный адрес страницы.
#define LCD0108_ADDRESS_MAX     127

/**
 * Инициализирует LCD.
 * @param lcd LCD.
 * @param inverse_cs Инвертировать ли сигналы выбора контроллера.
 * @param data_port Порт данных.
 * @param reset_port Порт входа RESET.
 * @param reset_pin_n Номер пина входа RESET.
 * @param cs0_port Порт входа CS0.
 * @param cs0_pin_n Номер пина входа CS0.
 * @param cs1_port Порт входа CS1.
 * @param cs1_pin_n Номер пина входа CS1.
 * @param rs_port Порт входа RS.
 * @param rs_pin_n Номер пина входа RS.
 * @param rw_port Порт входа RW.
 * @param rw_pin_n Номер пина входа RW.
 * @param e_port Порт входа E.
 * @param e_pin_n Номер пина входа E.
 * @return Код ошибки.
 */
extern err_t lcd0108_init(lcd0108_t* lcd,
                    bool inverse_cs, uint8_t data_port,
                    uint8_t reset_port, uint8_t reset_pin_n,
                    uint8_t cs0_port, uint8_t cs0_pin_n,
                    uint8_t cs1_port, uint8_t cs1_pin_n,
                    uint8_t rs_port, uint8_t rs_pin_n,
                    uint8_t rw_port, uint8_t rw_pin_n,
                    uint8_t e_port, uint8_t e_pin_n);

/**
 * Проверяет занятость LCD.
 * @param lcd LCD.
 * @return флаг занятости LCD.
 */
extern bool lcd0108_busy(lcd0108_t* lcd);

/**
 * Ждёт сброса флага занятости LCD.
 * @param lcd LCD.
 * @return флаг освобождения LCD.
 */
extern bool lcd0108_wait(lcd0108_t* lcd);

/**
 * Устанавливает включенность дисплея, курсора и мигания курсора.
 * @param lcd LCD.
 * @param display_on флаг включения дисплея.
 */
extern err_t lcd0108_control(lcd0108_t* lcd, bool display_on);

/**
 * Устанавливает стартовую линию LCD.
 * @param lcd LCD.
 * @param start_line стартовая линия.
 * @return Код ошибки.
 */
extern err_t lcd0108_set_start_line(lcd0108_t* lcd, uint8_t start_line);

/**
 * Записывает символы в LCD.
 * @param lcd LCD.
 * @patam page Номер страницы.
 * @param address Адрес в странице.
 * @param data Данные.
 * @param size Размер данных.
 */
extern err_t lcd0108_write(lcd0108_t* lcd, uint8_t page, uint8_t address, const uint8_t* data, size_t size);

/**
 * Записывает блок данных в LCD.
 * @param lcd LCD.
 * @param page Номер страницы.
 * @param address Адрес в странице.
 * @param data Данные.
 * @param width Ширина блока данных.
 * @param height Высота блока данных.
 * @param data_line_size Размер строки в буфере данных.
 * @return Код ошибки.
 */
extern err_t lcd0108_write_block(lcd0108_t* lcd, uint8_t page, uint8_t address, const uint8_t* data, uint8_t width, uint8_t height, uint8_t data_line_size);

/**
 * Очищает LCD.
 * @param lcd LCD.
 * @return Код ошибки.
 */
extern err_t lcd0108_clear(lcd0108_t* lcd);

#endif	/* LCD0108_H */

