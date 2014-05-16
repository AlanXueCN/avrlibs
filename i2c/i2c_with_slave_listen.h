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
#define E_I2C_BUS_ERROR                 (E_I2C + 2)
#define E_I2C_BUS_ARBITRATION_LOST      (E_I2C + 3)
#define E_I2C_DEVICE_NOT_RESPONDING     (E_I2C + 4)
#define E_I2C_DEVICE_DATA_NACK          (E_I2C + 5)
#define E_I2C_BUS_BUSY                  (E_BUSY)

//Адресация.
#define I2C_ADDRESS_MAX         0x7f
#define I2C_ADDRESS_MIN         0x01
#define I2C_ADDRESS_DEFAULT     I2C_ADDRESS_MAX
#define I2C_BROADCAST_ENABLED   1
#define I2C_BROADCAST_DISABLED  0

//Статус.
#define I2C_STATUS_IDLE          0
#define I2C_STATUS_DATA_READED   1
#define I2C_STATUS_DATA_WRITED   2
#define I2C_STATUS_READING       3
#define I2C_STATUS_WRITING       4

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

//Размер адреса страницы.
#define I2C_PAGE_ADDRESS_SIZE_8      1
#define I2C_PAGE_ADDRESS_SIZE_16     2
#define I2C_PAGE_ADDRESS_SIZE_MIN    I2C_PAGE_ADDRESS_SIZE_8
#ifdef I2C_NEED_PAGE_ADDRESS_32BIT
#define I2C_PAGE_ADDRESS_SIZE_32     4
#define I2C_PAGE_ADDRESS_SIZE_MAX    I2C_PAGE_ADDRESS_SIZE_32
#else
#define I2C_PAGE_ADDRESS_SIZE_MAX    I2C_PAGE_ADDRESS_SIZE_16
#endif
#define I2C_PAGE_ADDRESS_SIZE_INVALID     3

/**
 * Адрес страницы 8 бит.
 */
#pragma pack(push, 1)
typedef union {
    uint8_t value;
#ifdef I2C_NEED_PAGE_ADDRESS_32BIT
    uint8_t reserved[3];
#else
    uint8_t reserved[1];
#endif
}i2c_page_address8_t;
#pragma pack(pop)

/**
 * Адрес страницы 16 бит.
 */
#pragma pack(push, 1)
typedef union {
    uint16_t value;
#ifdef I2C_NEED_PAGE_ADDRESS_32BIT
    uint8_t reserved[2];
#endif
}i2c_page_address16_t;
#pragma pack(pop)

/**
 * Адрес страницы 32 бит.
 */
#ifdef I2C_NEED_PAGE_ADDRESS_32BIT
#pragma pack(push, 1)
typedef union {
    uint32_t value;
}i2c_page_address32_t;
#pragma pack(pop)
#endif

/**
 * Адрес страницы с нужной размерностью.
 */
typedef struct I2C_Page_Address {
    union {
        i2c_page_address8_t  addr8;
        i2c_page_address16_t addr16;
#ifdef I2C_NEED_PAGE_ADDRESS_32BIT
        i2c_page_address32_t addr32;
#endif
    }address;
    uint8_t size;
}i2c_page_address_t;


//Виды событий.
#define I2C_EVENT_ERROR                 1
#define I2C_EVENT_MASTER_DATA_WRITED    2
#define I2C_EVENT_MASTER_DATA_READED    3
#define I2C_EVENT_SLAVE_DATA_WRITED     4
#define I2C_EVENT_SLAVE_DATA_READED     5

/**
 * Событие для каллбэка.
 */
typedef uint8_t i2c_event_t;

/**
 * Тип каллбэка.
 * Принимает событие.
 * Возвращает true, если событие обработано, иначе false.
 */
typedef bool (*i2c_callback_t)(i2c_event_t);


//Идентификатор передачи по умолчанию.
#define I2C_DEFAULT_TRANSFER_ID 0

/**
 * Тип идентификатора передачи.
 */
typedef uint8_t i2c_transfer_id_t;

/**
 * Инициализирует состояние шины i2c.
 */
extern void i2c_init(void);

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
 * Получает текущую ошибку шины.
 * @return Код ошибки.
 */
extern err_t i2c_error(void);

/**
 * Получает число переданных байт в режиме мастер.
 * @return Число байт.
 */
extern i2c_size_t i2c_master_bytes_transmitted(void);

/**
 * Получает число переданных байт в режиме слейв.
 * @return Число байт.
 */
extern i2c_size_t i2c_slave_bytes_transmitted(void);

/**
 * Получает занятость шины.
 * @return true если шина занята, иначе false.
 */
extern bool i2c_is_busy(void);

/**
 * Получает каллбэк.
 * @return Каллбэк.
 */
extern i2c_callback_t i2c_callback(void);

/**
 * Устанавливает каллбэк.
 * @param callback Каллбэк.
 */
extern void i2c_set_callback(i2c_callback_t callback);

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
extern err_t i2c_master_read_at(i2c_address_t device, const i2c_page_address_t* page_address, void* data, i2c_size_t data_size);

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
extern err_t i2c_master_write_at(i2c_address_t device, const i2c_page_address_t* page_address, const void* data, i2c_size_t data_size);

/**
 * Начинает слушать запросы к данным по шине i2c в режиме слейв.
 * @param page_address адрес в устройстве.
 * @param data данные.
 * @param data_size размер данных.
 * @return Код ошибки.
 */
extern err_t i2c_slave_listen(i2c_page_address_t* page_address, void* data, i2c_size_t data_size);

/**
 * Прекращает слушать запросы к данным по шине i2c в режиме слейв.
 */
extern void i2c_slave_end_listening(void);

//extern uint8_t i2c_twi_status(void);

#endif	/* I2C_H */

