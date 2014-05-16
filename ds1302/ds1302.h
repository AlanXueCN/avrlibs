/**
 * @file ds1302.h
 * Библиотека для работы с часами реального времени DS1302.
 */

#ifndef DS1302_H
#define	DS1302_H

#include <stdbool.h>
#include <stdint.h>
#include "errors/errors.h"
#include "ssi/ssi.h"


//Структура памяти в ds1302.

//Побайтно.
#pragma pack(push, 1)
typedef struct _SecondsByte {
    uint8_t seconds:4;
    uint8_t seconds10:3;
    uint8_t clock_halt:1;
}seconds_byte_t;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct _MinutesByte {
    uint8_t minutes:4;
    uint8_t minutes10:3;
    uint8_t reserved1:1;
}minutes_byte_t;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct _HoursByte {
    uint8_t hours:4;
    uint8_t hours10:2;
    uint8_t reserved2:1;
    uint8_t ampm:1;
}hours_byte_t;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct _DateByte {
    uint8_t date:4;
    uint8_t date10:2;
    uint8_t reserved3:2;
}date_byte_t;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct _MonthByte {
    uint8_t month:4;
    uint8_t month10:1;
    uint8_t reserved4:3;
}month_byte_t;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct _DayByte {
    uint8_t day:3;
    uint8_t reserved5:5;
}day_byte_t;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct _YearByte {
    uint8_t year:4;
    uint8_t year10:4;
}year_byte_t;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct _WriteProtect {
    uint8_t reserved:7;
    uint8_t write_protect:1;
}wp_byte_t;
#pragma pack(pop)

//Полная память.
#pragma pack(push, 1)
typedef struct _Ds1302mem {
    //byte 0
    seconds_byte_t seconds_byte;
    //byte 1
    minutes_byte_t minutes_byte;
    //byte 2
    hours_byte_t hours_byte;
    //byte 3
    day_byte_t day_byte;
    //byte 4
    date_byte_t date_byte;
    //byte 5
    month_byte_t month_byte;
    //byte 6
    year_byte_t year_byte;
    //byte 7;
    wp_byte_t wp_byte;
} ds1302mem_t;
#pragma pack(pop)


//Срукруна DS1302.
typedef struct _Ds1302 {
    ds1302mem_t memory;
    ssi_t ssi;
    pin_t pin_sel;
}ds1302_t;


//Структура даты и времени.
typedef struct Ds1302_DT {
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t day;
    uint8_t date;
    uint8_t month;
    uint8_t year;
    bool is_ampm;
    bool pm;
}ds1302_datetime_t;

/**
 * Инициализация RTC.
 * @param rtc RTC.
 * @return Код ошибки.
 */
extern err_t ds1302_init(ds1302_t* rtc,
                         uint8_t sda_port_n, uint8_t sda_pin_n,
                         uint8_t scl_port_n, uint8_t scl_pin_n,
                         uint8_t sel_port_n, uint8_t sel_pin_n);

/**
 * Читает память.
 * @param rtc RTC.
 */
extern void ds1302_read(ds1302_t* rtc);

/**
 * Записывает память.
 * @param rtc RTC.
 */
extern void ds1302_write(ds1302_t* rtc);

/**
 * Получает дату и время.
 * @param rtc RTC.
 * @param datetime Дата и время.
 */
extern void ds1302_datetime_get(ds1302_t* rtc, ds1302_datetime_t* datetime);

/**
 * Устанавливает дату и время.
 * @param rtc RTC.
 * @param datetime Дата и время.
 */
extern void ds1302_datetime_set(ds1302_t* rtc, ds1302_datetime_t* datetime);

/**
 * Получает флаг запущенности часов.
 * @param rtc RTC.
 * @return Флаг запущенности часов.
 */
extern bool ds1302_running(ds1302_t* rtc);

/**
 * Устанавливает флаг запущенности часов.
 * @param rtc RTC.
 * @param running Флаг запущенности часов.
 */
extern void ds1302_set_running(ds1302_t* rtc, bool running);

/**
 * Записывает флаг запущенности часов.
 * @param rtc RTC.
 */
extern void ds1302_write_running(ds1302_t* rtc);

/**
 * Получает флаг защищённости памяти часов.
 * @param rtc RTC.
 * @return Флаг защищённости памяти часов.
 */
extern bool ds1302_write_protected(ds1302_t* rtc);

/**
 * Устанавливает флаг защищённости памяти часов.
 * @param rtc RTC.
 * @param running Флаг защищённости памяти часов.
 */
extern void ds1302_set_write_protected(ds1302_t* rtc, bool write_protected);

/**
 * Записывает флаг защищённости памяти часов.
 * @param rtc RTC.
 */
extern void ds1302_write_write_protected(ds1302_t* rtc);


#endif	/* DS1302_H */

