/**
 * @file spi.h
 * Библиотека для работы с SPI.
 */

#ifndef SPI_H
#define	SPI_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "errors/errors.h"

//! Ошибки SPI.
//#define E_SPI (E_USER + 60)

//! Режим SPI.
//! Ведущий.
#define SPI_MODE_MASTER 1
//! Ведомый.
#define SPI_MODE_SLAVE 0
//! Тип режима SPI.
typedef uint8_t spi_mode_t;

//! Порядок бит при передаче.
//! Передача сперва 7го бита.
#define SPI_DATA_ORDER_MSB_FIRST 0
//! Передача сперва 0го бита.
#define SPI_DATA_ORDER_LSB_FIRST 1
//! Тип порядка бит при передаче.
typedef uint8_t spi_data_order_t;

//! Полярность синхроимпульсов.
/**
 * Высокий уровень при подаче синхроимпульса,
 * Низкий уровень при простое.
 */
#define SPI_CLOCK_POLARITY_HIGH 0
/**
 * Низкий уровень при подаче синхроимпульса,
 * Высокий уровень при простое.
 */
#define SPI_CLOCK_POLARITY_LOW  1
//! Тип полярности синхроимпульсов.
typedef uint8_t spi_clock_polarity_t;

//! Фаза чтения и установки данных на линии.
//! Чтение на начальном, установка на завершающем фронте.
#define SPI_CPHA_LEADING_SAMPLE_TRAILING_SETUP  0
//! Установка на начальном, чтение на завершающем фронте.
#define SPI_CPHA_LEADING_SETUP_TRAILING_SAMPLE  1
//! Тип фазы.
typedef uint8_t spi_clock_phase_t;

//! Делитель тактирования SPI.
#define SPI_CLOCK_RATE_4     0
#define SPI_CLOCK_RATE_16    1
#define SPI_CLOCK_RATE_64    2
#define SPI_CLOCK_RATE_128   3
#define SPI_CLOCK_RATE_2X_2  4
#define SPI_CLOCK_RATE_2X_8  5
#define SPI_CLOCK_RATE_2X_32 6
#define SPI_CLOCK_RATE_2X_64 7
//! Тип делителя тактирования SPI.
typedef uint8_t spi_clock_rate_t;

//! Состояния SPI.
//! Простой.
#define SPI_STATE_IDLE                   0
//! Пересылка данных.
#define SPI_STATE_DATA_TRANSFERING       1
//! Успешное завершение пересылки.
#define SPI_STATE_DATA_TRANSFERED        2
//! Передача данных.
#define SPI_STATE_DATA_WRITING           3
//! Успешное завершение передачи данных.
#define SPI_STATE_DATA_WRITED            4
//! Приём данных.
#define SPI_STATE_DATA_READING           5
//! Успешное завершение приёма данных.
#define SPI_STATE_DATA_READED            6
//! Пересылка данных ведомым.
#define SPI_STATE_SLAVE_DATA_TRANSFERING 7
//! Переход в режим ведомого из-за обращения другого ведущего.
#define SPI_STATE_LOW_PRIORITY           8
//! Пересылка данных прервана обращением другого ведущего.
#define SPI_STATE_INTERRUPTED            9
//! Тип состояния.
typedef uint8_t spi_state_t;

//Идентификатор передачи по умолчанию.
#define SPI_DEFAULT_TRANSFER_ID 0

//! Тип идентификатора передачи.
typedef uint8_t spi_transfer_id_t;

/** Тип каллбэка окончания пересылки в режиме Master.
 * @return true, если событие обработано, иначе false.
 */
typedef bool (*spi_master_callback_t)(void);

/** Тип каллбэка при пересылке очередного байта в режиме Slave.
 * @param received_byte Принятый байт.
 * @return Байт для пересылки.
 */
typedef uint8_t (*spi_slave_callback_t)(uint8_t received_byte);

//! Значение пересылаемого байта по-умолчанию
#define SPI_DATA_DEFAULT 0


/**
 * Инициализирует SPI.
 * @param spi_port Номер порта с SPI.
 * @param mosi_pin Номер пина MOSI.
 * @param miso_pin Номер пина MISO.
 * @param sck_pin Номер пина SCK.
 * @param ss_pin Номер пина SS.
 * @param mode Режим SPI.
 * @param clock_rate Делитель частоты для SPI.
 * @return Код ошибки.
 */
extern err_t spi_init(uint8_t spi_port, uint8_t mosi_pin, uint8_t miso_pin,
                      uint8_t sck_pin, uint8_t ss_pin,
                      spi_mode_t mode, spi_clock_rate_t clock_rate);

/**
 * Получает режим SPI.
 * @return Режим SPI.
 */
extern spi_mode_t spi_mode(void);

/**
 * Устанавливает режим SPI.
 * @param mode Режим SPI.
 */
extern err_t spi_set_mode(spi_mode_t mode);

/**
 * Получает порядок бит при передаче.
 * @return Порядок бит при передаче.
 */
extern spi_data_order_t spi_data_order(void);

/**
 * Устанавливает порядок бит при передаче.
 * @param data_order Порядок бит при передаче.
 * @return Код ошибки.
 */
extern err_t spi_set_data_order(spi_data_order_t data_order);

/**
 * Получает полярность синхроимпульса.
 * @return Полярность синхроимпульса.
 */
extern spi_clock_polarity_t spi_clock_polarity(void);

/**
 * Устанавливает полярность синхроимпульса.
 * @param clock_polarity Полярность синхроимпульса.
 * @return Код ошибки.
 */
extern err_t spi_set_clock_polarity(spi_clock_polarity_t clock_polarity);

/**
 * Получате фазу чтения и установки данных на линии.
 * @return Фаза чтения и установки данных на линии.
 */
extern spi_clock_phase_t spi_clock_phase(void);

/**
 * Устанавливает фазу чтения и установки данных на линии.
 * @param clock_phase Фаза чтения и установки данных на линии.
 * @return Код ошибки.
 */
extern err_t spi_set_clock_phase(spi_clock_phase_t clock_phase);

/**
 * Получает состояние SPI.
 * @return Состояние SPI.
 */
extern spi_state_t spi_state(void);

/**
 * Получает флаг занятости SPI.
 * @return Флаг занятости SPI.
 */
extern bool spi_busy(void);

/**
 * Получает идентификатор передачи.
 * @return Идентификатор передачи.
 */
extern spi_transfer_id_t spi_transfer_id(void);

/**
 * Устанавливает идентификатор передачи.
 * @param id Идентификатор передачи.
 */
extern void spi_set_transfer_id(spi_transfer_id_t id);

/**
 * Получает каллбэк ведущего.
 * @return Каллбэк ведущего.
 */
extern spi_master_callback_t spi_master_callback(void);

/**
 * Устанавливает каллбэк ведущего.
 * @param callback Каллбэк ведущего.
 */
extern void spi_set_master_callback(spi_master_callback_t callback);

/**
 * Получает каллбэк ведомого.
 * @return Каллбэк ведомого.
 */
extern spi_slave_callback_t spi_slave_callback(void);

/**
 * Устанавливает каллбэк ведомого.
 * @param callback Каллбэк ведомого.
 */
extern void spi_set_slave_callback(spi_slave_callback_t callback);

/**
 * Асинхронно передаёт и принимает данные по SPI.
 * Один из буферов может быть NULL.
 * @param tx_data Данные для передачи.
 * @param rx_data Буфер для приёма данных.
 * @param size Размер данных, которые нужно передать и принять.
 * @return Код ошибки.
 */
extern err_t spi_transfer(const void* tx_data, void* rx_data, size_t size);

/**
 * Асинхронно передаёт данные по SPI.
 * @param data Данные для передачи.
 * @param size Размер данных.
 * @return Код ошибки.
 */
extern err_t spi_write(const void* data, size_t size);

/**
 * Асинхронно принимает данные по SPI.
 * @param data Буфер для приёма данных.
 * @param size Размер данных.
 * @return Код ошибки.
 */
extern err_t spi_read(void* data, size_t size);

/**
 * Асинхронно передаёт, затем принимает данные по SPI.
 * @param tx_data Данные для передачи.
 * @param rx_data Буфер для приёма данных.
 * @param size Размер данных, которые нужно передать и принять.
 * @return Код ошибки.
 */
extern err_t spi_write_then_read(const void* tx_data, size_t tx_size, void* rx_data, size_t rx_size);

#endif	/* SPI_H */

