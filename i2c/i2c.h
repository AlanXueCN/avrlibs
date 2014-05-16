/**
 * @file i2c.h
 * Библиотека для работы с шиной I2C.
 */

#ifndef I2C_H
#define	I2C_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "errors/errors.h"


//Ошибки.
#define E_I2C                           (E_USER + 20)
#define E_I2C_INVALID_FREQ              (E_I2C + 1)

//Адресация.
#define I2C_ADDRESS_MAX         0x7f
#define I2C_ADDRESS_MIN         0x01
#define I2C_ADDRESS_DEFAULT     I2C_ADDRESS_MAX
#define I2C_BROADCAST_ENABLED   1
#define I2C_BROADCAST_DISABLED  0

//Статус.
#define I2C_STATUS_IDLE                0
#define I2C_STATUS_READING             1
#define I2C_STATUS_WRITING             2
#define I2C_STATUS_DATA_READED         3
#define I2C_STATUS_DATA_WRITED         4
#define I2C_STATUS_SLAVE_READING       5
#define I2C_STATUS_SLAVE_WRITING       6
#define I2C_STATUS_SLAVE_DATA_READED   7
#define I2C_STATUS_SLAVE_DATA_WRITED   8
#define I2C_STATUS_BUS_ERROR           9
#define I2C_STATUS_ARBITRATION_LOST    10
#define I2C_STATUS_NOT_RESPONDING      11
#define I2C_STATUS_REJECTED            12

//Значение байта по-умолчанию.
#define I2C_DATA_DEFAULT_VALUE 0xff

/**
 * Тип размера данных.
 */
typedef size_t i2c_size_t;

/**
 * Тип адреса устройства.
 */
typedef uint8_t i2c_address_t;

/**
 * Тип состояния шины.
 */
typedef uint8_t i2c_status_t;

/**
 * Тип каллбэка.
 * Вызывается при окончании приёма/передачи
 * в режимах ведущего и ведомого, или ошибках.
 * @return true, если событие обработано, иначе false.
 */
typedef bool (*i2c_callback_t)(void);

//! Виды действий i2c.
//! Чтение.
#define I2C_READ 0
//! Запись.
#define I2C_WRITE 1
//! Тип действия i2c.
typedef uint8_t i2c_action_t;

/**
 * Тип каллбэка ведомого.
 * Вызывается при начале приёма/передачи
 * и при приёме/передаче каждого байта в режиме ведомого.
 * Если data == NULL, обозначает начало передачи.
 * Иначе передаёт принятый байт, в случае I2C_READ, или
 * принимает байт для передачи, в случае I2C_WRITE.
 * @param action Действие.
 * @param data Буфер с принятым байтом, либо для передаваемого байта.
 * @return Возможность принимать или передавать ещё данные,
 * Возможность передать/принять более одного байта, если data == NULL.
 */
typedef bool (*i2c_slave_callback_t)(i2c_action_t action, uint8_t* data);

//Идентификатор передачи по умолчанию.
#define I2C_DEFAULT_TRANSFER_ID 0

/**
 * Тип идентификатора передачи.
 */
typedef uint8_t i2c_transfer_id_t;

/**
 * Инициализирует состояние шины i2c.
 * @param freq Частота в kHz.
 * @return Код ошибки.
 */
extern err_t i2c_init(uint16_t freq);

/**
 * Устанавливает частоту шины i2c.
 * @param freq Частота в kHz.
 * @return Код ошибки.
 */
extern err_t i2c_set_freq(uint16_t freq);

/**
 * Устанавливает параметры адреса.
 * @param address Адрес.
 * @param bcast_enabled Разрешённость ответа на широковещательный адрес.
 */
extern void i2c_set_address(i2c_address_t address, bool bcast_enabled);

/**
 * Получает статус шины.
 * @return Статус шины.
 */
extern i2c_status_t i2c_status(void);

/**
 * Получает занятость шины.
 * @return true если шина занята, иначе false.
 */
extern bool i2c_is_busy(void);

/**
 * Получает каллбэк ведущего.
 * @return Каллбэк ведущего.
 */
extern i2c_callback_t i2c_callback(void);

/**
 * Устанавливает каллбэк ведущего.
 * @param callback Каллбэк ведущего.
 */
extern void i2c_set_callback(i2c_callback_t callback);

/**
 * Получает каллбэк принимающего ведомого.
 * @return Каллбэк принимающего ведомого.
 */
extern i2c_slave_callback_t i2c_slave_callback(void);

/**
 * Устанавливает каллбэк принимающего ведомого.
 * @param callback Каллбэк принимающего ведомого.
 */
extern void i2c_set_slave_callback(i2c_slave_callback_t callback);

/**
 * Получает идентификатор передачи.
 * @return Идентификатор передачи.
 */
extern i2c_transfer_id_t i2c_transfer_id(void);

/**
 * Устанавливает идентификатор передачи.
 * @param id Идентификатор передачи.
 */
extern void i2c_set_transfer_id(i2c_transfer_id_t id);

/**
 * Получает число переданных байт в режиме мастер.
 * @return Число байт.
 */
extern i2c_size_t i2c_master_bytes_transmitted(void);

/**
 * Получает данные по шине i2c в режиме мастер.
 * @param device адрес устройства.
 * @param data данные.
 * @param data_size размер данных.
 * @return Код ошибки.
 */
extern err_t i2c_master_read(i2c_address_t device, void* data, i2c_size_t data_size);

/**
 * Получает данные по шине i2c в режиме мастер.
 * @param device адрес устройства.
 * @param page_address адрес в устройстве.
 * @param data данные.
 * @param data_size размер данных.
 * @return Код ошибки.
 */
extern err_t i2c_master_read_at(i2c_address_t device, const void* page_address, size_t page_address_size, void* data, i2c_size_t data_size);

/**
 * Передаёт данные по шине i2c в режиме мастер.
 * @param device адрес устройства.
 * @param data данные.
 * @param data_size размер данных.
 * @return Код ошибки.
 */
extern err_t i2c_master_write(i2c_address_t device, const void* data, i2c_size_t data_size);

/**
 * Передаёт данные по шине i2c в режиме мастер.
 * @param device адрес устройства.
 * @param page_address адрес в устройстве.
 * @param data данные.
 * @param data_size размер данных.
 * @return Код ошибки.
 */
extern err_t i2c_master_write_at(i2c_address_t device, const void* page_address, size_t page_address_size, const void* data, i2c_size_t data_size);

/**
 * Начинает слушать запросы к данным по шине i2c в режиме слейв.
 */
extern void i2c_slave_listen(void);

/**
 * Прекращает слушать запросы к данным по шине i2c в режиме слейв.
 */
extern void i2c_slave_end_listening(void);

//extern uint8_t i2c_twi_status(void);

#endif	/* I2C_H */

