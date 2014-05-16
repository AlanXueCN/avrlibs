/**
 * @file ssi.h
 * Библиотека для работы с простой последовательной шиной данных.
 */

#ifndef SSI_H
#define	SSI_H

#include "ports/ports.h"
#include "errors/errors.h"
#include <stdint.h>
#include <stdbool.h>


/**
 * Serial Synchronous Interface.
 */

//! Порядок бит.
//! Сперва MSB.
#define SSI_BIT_ORDER_MSB_TO_LSB 0
//! Сперва LSB.
#define SSI_BIT_ORDER_LSB_TO_MSB 1

//! Тип порядка бит.
typedef uint8_t ssi_bit_order_t;

//! Подтяжка линии.
//! К GND
#define SSI_PULLUP_NONE_OR_GND 0
//! К Vcc
#define SSI_PULLUP_VCC  1

//! Тип подтяжки линии.
typedef uint8_t ssi_pullup_t;

//! Позиция чтения бита.
//! После поднятия синхроимпульса.
#define SSI_BIT_READ_AT_RISING_EDGE     0
//! После опускания синхроимпульса.
#define SSI_BIT_READ_AT_FALLING_EDGE    1

//! Тип позиции чтения бита.
typedef uint8_t ssi_bit_read_pos_t;

//! Структура последовательного интерфейса.
typedef struct _SSI {
    pin_t pin_sda;
    pin_t pin_scl;
    uint8_t half_period_time_us;
    ssi_pullup_t sda_pullup;
    ssi_pullup_t scl_pullup;
    ssi_bit_order_t bit_order;
    ssi_bit_read_pos_t bit_read_pos;
}ssi_t;

typedef uint8_t ssi_size_t;

/**
 * Инициализирует последовательный интерфейс.
 * Чтение и запись начинаются с MSB.
 * @param ssi Последовательный интерфейс.
 * @param sda_port_n Порт линии данных.
 * @param sda_pin_n Пин линии данных.
 * @param scl_port_n Порт линии синхронизации.
 * @param scl_pin_n Пин линии синхронизации.
 * @param sda_pullup Тип подтяжки линии SDA.
 * @param scl_pullup Тип подтяжки линии SCL.
 * @param bit_order Порядок передачи бит.
 * @param bit_read_pos Позиция чтения бита.
 * @param half_period_time_us Время полупериода (в мкс).
 * @return Код ошибки.
 */
extern err_t ssi_init(ssi_t* ssi,
                        uint8_t sda_port_n, uint8_t sda_pin_n, ssi_pullup_t sda_pullup,
                        uint8_t scl_port_n, uint8_t scl_pin_n, ssi_pullup_t scl_pullup,
                        ssi_bit_order_t bit_order, ssi_bit_read_pos_t bit_read_pos,
                        uint8_t half_period_time_us);

/**
 * Начинает обмен данными по шине.
 * @param ssi Последовательный интерфейс.
 */
extern void ssi_begin(ssi_t* ssi);

/**
 * Завершает обмен данными по шине.
 * @param ssi Последовательный интерфейс.
 */
extern void ssi_end(ssi_t* ssi);

/**
 * Записывает бит в шину.
 * @param ssi Последовательный интерфейс.
 * @param bit Бит данных.
 */
extern void ssi_write_bit(ssi_t* ssi, uint8_t bit);

/**
 * Записывает байт в шину.
 * @param ssi Последовательный интерфейс.
 * @param byte Байт данных.
 */
extern void ssi_write_byte(ssi_t* ssi, uint8_t byte);

/**
 * Записывает массив байт в шину.
 * @param ssi Последовательный интерфейс.
 * @param data Данные.
 * @param size Размер данных.
 */
extern void ssi_write(ssi_t* ssi, const void* data, ssi_size_t size);

/**
 * Читает байт из шины.
 * @param ssi Последовательный интерфейс.
 * @return Прочитанный байт.
 */
extern uint8_t ssi_read_byte(ssi_t* ssi);

/**
 * Читает бит из шины.
 * @param ssi Последовательный интерфейс.
 * @return Прочитанный бит.
 */
extern uint8_t ssi_read_bit(ssi_t* ssi);

/**
 * Читает массив байт из шины.
 * @param ssi Последовательный интерфейс.
 * @param data Данные.
 * @param size Размер данных.
 */
extern void ssi_read(ssi_t* ssi, void* data, ssi_size_t size);

/**
 * Посылает команду чтения и сразу начинает читать данные,
 * Начиная с низпадающего фронта импульса синхронизации.
 * Такая процедура используется в DS1302.
 * @param ssi Последовательный интерфейс.
 * @param cmd Команда чтения.
 * @param data Данные.
 * @param size Размер данных.
 */
extern void ssi_cmd_read(ssi_t* ssi, uint8_t cmd, void* data, ssi_size_t size);

#endif	/* SSI_H */

