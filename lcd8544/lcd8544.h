/**
 * @file spi.h
 * Библиотека для работы с LCD на контроллере PCB8544.
 */

#ifndef LCD8544_H
#define	LCD8544_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "spi/spi.h"
#include "ports/ports.h"
#include "errors/errors.h"
#include "future/future.h"

/**
 * Тип структуры дисплея.
 */
typedef struct _Lcd8544{
    //! Идентификатор передачи.
    spi_transfer_id_t transfer_id;
    //! Пин выбора ведомого.
    pin_t ce_pin;
    //! Пин выбора данных/команды.
    pin_t dc_pin;
    //! Пин сброса.
    pin_t rst_pin;
    //! Будущее.
    future_t future;
    //! Буфер для отправки данных/команды.
    uint8_t data_cmd;
}lcd8544_t;

//! Идентификатор передачи по-умолчанию.
#define LCD8544_DEFAULT_TRANSFER_ID 85

//! Размер дисплея.
#define LCD8544_WIDTH   84
#define LCD8544_HEIGHT  48
#define LCD8544_RAM_WIDTH  84
#define LCD8544_RAM_HEIGHT 6
#define LCD8544_RAM_SIZE (LCD8544_RAM_WIDTH * LCD8544_RAM_HEIGHT)

//! Режим адресации.
//! Инкремент адреса по X.
#define LCD8544_ADDRESSING_MODE_HORIZONTAL  0
//! Инкремент адреса по Y.
#define LCD8544_ADDRESSING_MODE_VERTICAL    1
//! Тип режима адресации.
typedef uint8_t lcd8544_addressing_mode_t;

//! Набор инструкций.
//! Базовый.
#define LCD8544_INSTRUCTION_SET_BASIC    0
//! Расширенный.
#define LCD8544_INSTRUCTION_SET_EXTENDED 1
//! Тип набора инструкций.
typedef uint8_t lcd8544_instruction_set_t;

//! Режим отображения.
//! Всё погашено.
#define LCD8544_DISPLAY_MODE_NONE       0
//! Всё погашено.
#define LCD8544_DISPLAY_MODE_NORMAL     1
//! Всё погашено.
#define LCD8544_DISPLAY_MODE_ALL        2
//! Всё погашено.
#define LCD8544_DISPLAY_MODE_INVERSE    3
//! Тип режима отображения.
typedef uint8_t lcd8544_display_mode_t;

//! Температурный коэффициент.
#define LCD8544_TEMP_COEF_0     0
#define LCD8544_TEMP_COEF_1     1
#define LCD8544_TEMP_COEF_2     2
#define LCD8544_TEMP_COEF_3     3
//! Тип температурного коэффициента.
typedef uint8_t lcd8544_temp_coef_t;

//! Bias voltage system
//! 1/100
#define LCD8544_BIAS_SYSTEM_1_DIV_100   0
//! 1/80
#define LCD8544_BIAS_SYSTEM_1_DIV_80    1
//! 1/65
#define LCD8544_BIAS_SYSTEM_1_DIV_65    2
//! 1/48
#define LCD8544_BIAS_SYSTEM_1_DIV_48    3
//! 1/40
#define LCD8544_BIAS_SYSTEM_1_DIV_40    4
//! 1/24
#define LCD8544_BIAS_SYSTEM_1_DIV_24    5
//! 1/18
#define LCD8544_BIAS_SYSTEM_1_DIV_18    6
//! 1/10
#define LCD8544_BIAS_SYSTEM_1_DIV_10    7
//! По-умолчанию (из даташита).
#define LCD8544_BIAS_SYSTEM_DEFAULT     LCD8544_BIAS_SYSTEM_1_DIV_48
//! Тип bias voltage system
typedef uint8_t lcd8544_bias_system_t;

//! Напряжение (милливольт).
#define LCD8544_VOLTAGE_MIN_mV     3060
#define LCD8544_VOLTAGE_MAX_mV     8500

/**
 * Каллбэк SPI.
 * @return true, если событие обработано, иначе false.
 */
extern bool lcd8544_spi_callback(lcd8544_t* lcd);

/**
 * Инициализирует LCD.
 * @param lcd LCD.
 * @param ce_port Порт пина выбора ведомого SPI.
 * @param ce_pin Пин выбора ведомого SPI.
 * @param dc_port Порт пина выбора команды/данных.
 * @param dc_pin Пин выбора команды/данных.
 * @param rst_port Порт пина reset LCD.
 * @param rst_pin Пин reset LCD.
 * @param transfer_id Идентификатор передачи SPI.
 * @return Код ошибки.
 */
extern err_t lcd8544_init(lcd8544_t* lcd, uint8_t ce_port, uint8_t ce_pin,
                          uint8_t dc_port, uint8_t dc_pin,
                          uint8_t rst_port, uint8_t rst_pin, spi_transfer_id_t transfer_id);

/**
 * Получает флаг занятости LCD.
 * @param lcd LCD.
 * @return Флаг занятости LCD.
 */
extern bool lcd8544_busy(lcd8544_t* lcd);

/**
 * Получает код ошибки асинхронно выполненой операции.
 * @param lcd LCD.
 * @return Код ошибки асинхронно выполненой операции.
 */
extern err_t lcd8544_error(lcd8544_t* lcd);

/**
 * Ожидает завершения операции с LCD.
 * @param lcd LCD.
 * @return Код ошибки операции.
 */
extern err_t lcd8544_wait(lcd8544_t* lcd);

/**
 * Сбрасывает LCD.
 * @param lcd LCD.
 */
extern void lcd8544_reset(lcd8544_t* lcd);

/**
 * Устанавливает функционал LCD.
 * @param lcd LCD.
 * @param power_on Флаг включения LCD.
 * @param addressing_mode Режим адресации.
 * @param instruction_set Набор инструкций.
 * @return Код ошибки.
 */
extern err_t lcd8544_function_set(lcd8544_t* lcd, bool power_on,
                                  lcd8544_addressing_mode_t addressing_mode,
                                  lcd8544_instruction_set_t instruction_set);

/**
 * Записывает данные (8 пикселов) в LCD.
 * @param lcd LCD.
 * @param data Данные.
 * @return Код ошибки.
 */
extern err_t lcd8544_write_data(lcd8544_t* lcd, uint8_t data);

/**
 * Записывает данные в LCD.
 * @param lcd LCD.
 * @param data Данные.
 * @param data_size Размер данных.
 * @return Код ошибки.
 */
extern err_t lcd8544_write(lcd8544_t* lcd, const uint8_t* data, size_t data_size);

/**
 * Устанавливает режим отображения.
 * @param lcd LCD.
 * @param mode Режим отображения.
 * @return Код ошибки.
 */
extern err_t lcd8544_set_display_mode(lcd8544_t* lcd, lcd8544_display_mode_t mode);

/**
 * Устанавливает Y адрес в памяти LCD.
 * @param lcd LCD.
 * @param address Адрес.
 * @return Код ошибки.
 */
extern err_t lcd8544_set_y_address(lcd8544_t* lcd, uint8_t address);

/**
 * Устанавливает X адрес в памяти LCD.
 * @param lcd LCD.
 * @param address Адрес.
 * @return Код ошибки.
 */
extern err_t lcd8544_set_x_address(lcd8544_t* lcd, uint8_t address);

/**
 * Устанавливает значение температурного коэффициента.
 * @param lcd LCD.
 * @param coef Температурный коэффициент.
 * @return Код ошибки.
 */
extern err_t lcd8544_set_temp_coef(lcd8544_t* lcd, lcd8544_temp_coef_t coef);

/**
 * Устанавливает значение отклонения системного напряжения (?).
 * @param lcd LCD.
 * @param bias Отклонение.
 * @return Код ошибки.
 */
extern err_t lcd8544_set_bias_system(lcd8544_t* lcd, lcd8544_bias_system_t bias);

/**
 * Устанавливает значение напряжения.
 * @param lcd LCD.
 * @param mV Напряжение, милливольт.
 * @return Код ошибки.
 */
extern err_t lcd8544_set_voltage(lcd8544_t* lcd, uint16_t mV);

#endif	/* LCD8544_H */

