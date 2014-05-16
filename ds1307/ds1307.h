/**
 * @file ds1307.h
 * Библиотека для работы с часами реального времени DS1307.
 */

#ifndef DS1307_H
#define	DS1307_H

#include <stdbool.h>
#include <stdint.h>
#include "errors/errors.h"
#include "future/future.h"
#include "i2c/i2c.h"


#define E_DS1307_BUSY (E_BUSY)

//Адрес ds1307.
#define DS1307_I2C_ADDRESS 0x68

//Идентификатор передачи i2c для ds1307
#define DS1307_I2C_TRANSFER_ID DS1307_I2C_ADDRESS

//Тип статуса.
typedef uint8_t ds1307_status_t;

//Статусы.
#define DS1307_STATUS_NONE 0
#define DS1307_STATUS_ERROR 1
//read
#define DS1307_STATUS_READ 2
#define DS1307_STATUS_READING 3
#define DS1307_STATUS_READED 4
//write
#define DS1307_STATUS_WRITE 5
#define DS1307_STATUS_WRITING 6
#define DS1307_STATUS_WRITED 7


//Структура даты и времени.
typedef struct Ds1307_DT {
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t day;
    uint8_t date;
    uint8_t month;
    uint8_t year;
    bool is_ampm;
    bool pm;
}ds1307_datetime_t;

/**
 * Каллбэк i2c.
 * @param event Событие i2c.
 * @return true, если событие обработано, иначе false.
 */
extern bool ds1307_i2c_callback(void);

/**
 * Инициализация RTC.
 * @return Код ошибки.
 */
extern err_t ds1307_init(void);

/**
 * Получает флаг процесса выполнения операции.
 * @return Флаг процесса выполнения операции.
 */
extern bool ds1307_in_process(void);

/**
 * Получает флаг завершения текущей операции.
 * @return Флаг завершения текущей операции.
 */
extern bool ds1307_done(void);

/**
 * Получает статус.
 * @return Статус.
 */
extern ds1307_status_t ds1307_status(void);

/**
 * Получает код ошибки.
 * @return Код ошибки.
 */
extern err_t ds1307_error(void);

/**
 * Читает память.
 * @return Код ошибки.
 */
extern err_t ds1307_read(void);

/**
 * Записывает память.
 * @return Код ошибки.
 */
extern err_t ds1307_write(void);

/**
 * Получает дату и время.
 * @param datetime Дата и время.
 */
extern void ds1307_datetime_get(ds1307_datetime_t* datetime);

/**
 * Устанавливает дату и время.
 * @param datetime Дата и время.
 */
extern void ds1307_datetime_set(ds1307_datetime_t* datetime);

/**
 * Получает флаг запущенности часов.
 * @return Флаг запущенности часов.
 */
extern bool ds1307_running(void);

/**
 * Устанавливает флаг запущенности часов.
 * @param running Флаг запущенности часов.
 */
extern void ds1307_set_running(bool running);

/**
 * Записывает состояние часов.
 * @return Код ошибки.
 */
extern err_t ds1307_write_running();

#endif	/* DS1307_H */

