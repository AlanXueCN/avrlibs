#include "ds1307.h"
#include <string.h>
#include <stddef.h>
#include "utils/utils.h"
#include "bits/bits.h"



//Структура памяти в ds1307.

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
    uint8_t ampm:1;
    uint8_t reserved2:1;
}hours_byte_t;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct _DayByte {
    uint8_t day:3;
    uint8_t reserved3:5;
}day_byte_t;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct _DateByte {
    uint8_t date:4;
    uint8_t date10:2;
    uint8_t reserved4:2;
}date_byte_t;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct _MonthByte {
    uint8_t month:4;
    uint8_t month10:1;
    uint8_t reserved5:3;
}month_byte_t;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct _YearByte {
    uint8_t year:4;
    uint8_t year10:4;
}year_byte_t;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct _SqweByte {
    uint8_t rs:2;
    uint8_t reserved7_1:2;
    uint8_t sqwe:1;
    uint8_t reserved7_2:2;
    uint8_t out:1;
}sqwe_byte_t;
#pragma pack(pop)

//Полная память.
#pragma pack(push, 1)
typedef struct _Ds1307mem {
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
    sqwe_byte_t sqwe_byte;
} ds1307mem_t;
#pragma pack(pop)


//Срукруна DS1307.
typedef struct _Ds1307 {
    ds1307mem_t memory;
    uint8_t page_address;
    i2c_size_t size_to_rw;
    future_t future;
    ds1307_status_t status;
}ds1307_state_t;

static ds1307_state_t rtc;


static err_t ds1307_do();

bool ds1307_i2c_callback(void)
{
    if(i2c_transfer_id() != DS1307_I2C_TRANSFER_ID) return false;
    
    switch(i2c_status()){
        case I2C_STATUS_SLAVE_DATA_READED:
        case I2C_STATUS_SLAVE_DATA_WRITED:
            return false;
    }
    
    ds1307_do();
    
    return true;
}

static void ds1307_start(ds1307_status_t status)
{
    future_start(&rtc.future);
    rtc.status = status;
}

static void ds1307_next(ds1307_status_t status)
{
    rtc.status = status;
}

static void ds1307_finish(ds1307_status_t status, void* result)
{
    future_finish(&rtc.future, result);
    rtc.status = status;
}

static err_t ds1307_do()
{
    err_t err = E_NO_ERROR;
    switch(rtc.status){
        case DS1307_STATUS_READ:
            i2c_set_transfer_id(DS1307_I2C_TRANSFER_ID);
            err = i2c_master_read_at(DS1307_I2C_ADDRESS, &rtc.page_address, 1, &rtc.memory, rtc.size_to_rw);
            if(err != E_NO_ERROR){
                ds1307_finish(DS1307_STATUS_ERROR, int_to_pvoid(err));
            }else{
                ds1307_next(DS1307_STATUS_READING);
            }
            break;
        case DS1307_STATUS_READING:
            if(i2c_status() == I2C_STATUS_DATA_READED){
                ds1307_finish(DS1307_STATUS_READED, int_to_pvoid(E_NO_ERROR));
            }else{
                ds1307_finish(DS1307_STATUS_ERROR, int_to_pvoid(E_IO_ERROR));
            }
            break;
        case DS1307_STATUS_WRITE:
            i2c_set_transfer_id(DS1307_I2C_TRANSFER_ID);
            err = i2c_master_write_at(DS1307_I2C_ADDRESS, &rtc.page_address, 1, &rtc.memory, rtc.size_to_rw);
            if(err != E_NO_ERROR){
                ds1307_finish(DS1307_STATUS_ERROR, int_to_pvoid(err));
            }else{
                ds1307_next(DS1307_STATUS_WRITING);
            }
            break;
        case DS1307_STATUS_WRITING:
            if(i2c_status() == I2C_STATUS_DATA_WRITED){
                ds1307_finish(DS1307_STATUS_WRITED, int_to_pvoid(E_NO_ERROR));
            }else{
                ds1307_finish(DS1307_STATUS_ERROR, int_to_pvoid(E_IO_ERROR));
            }
            break;
        default:
            break;
    }
    return err;
}

err_t ds1307_init()
{
    memset(&rtc.memory, 0x0, sizeof(ds1307mem_t));
    
    rtc.status = DS1307_STATUS_NONE;
    
    rtc.page_address = 0;
    
    rtc.size_to_rw = sizeof(ds1307mem_t);
    
    future_init(&rtc.future);
    future_set_result(&rtc.future, int_to_pvoid(E_NO_ERROR));
    
    return E_NO_ERROR;
}

bool ds1307_in_process()
{
    return future_running(&rtc.future);
}

bool ds1307_done()
{
    return future_done(&rtc.future);
}

ds1307_status_t ds1307_status(void)
{
    return rtc.status;
}

err_t ds1307_error(void)
{
    return pvoid_to_int(err_t, future_result(&rtc.future));
}

err_t ds1307_read()
{
    if(future_running(&rtc.future)) return E_DS1307_BUSY;
    
    rtc.size_to_rw = sizeof(ds1307mem_t);
    
    ds1307_start(DS1307_STATUS_READ);
    
    return ds1307_do();
}

err_t ds1307_write()
{
    if(future_running(&rtc.future)) return E_DS1307_BUSY;
    
    rtc.size_to_rw = sizeof(ds1307mem_t);
    
    ds1307_start(DS1307_STATUS_WRITE);
    
    return ds1307_do();
}

void ds1307_datetime_get(ds1307_datetime_t* datetime)
{
    datetime->seconds = rtc.memory.seconds_byte.seconds10 * 10 + rtc.memory.seconds_byte.seconds;
    datetime->minutes = rtc.memory.minutes_byte.minutes10 * 10 + rtc.memory.minutes_byte.minutes;
    if(rtc.memory.hours_byte.ampm){
        datetime->is_ampm = true;
        datetime->pm = BIT_VALUE(rtc.memory.hours_byte.hours10, 1);
        datetime->hours   = (rtc.memory.hours_byte.hours10 & 0x1) * 10 + rtc.memory.hours_byte.hours;
    }else{
        datetime->is_ampm = false;
        datetime->hours   = rtc.memory.hours_byte.hours10 * 10 + rtc.memory.hours_byte.hours;
    }
    datetime->day = rtc.memory.day_byte.day;
    datetime->date = rtc.memory.date_byte.date10 * 10 + rtc.memory.date_byte.date;
    datetime->month = rtc.memory.month_byte.month10 * 10 + rtc.memory.month_byte.month;
    datetime->year = rtc.memory.year_byte.year10 * 10 + rtc.memory.year_byte.year;
}

void ds1307_datetime_set(ds1307_datetime_t* datetime)
{
    rtc.memory.seconds_byte.seconds10 = datetime->seconds / 10;
    rtc.memory.seconds_byte.seconds   = datetime->seconds % 10;
    
    rtc.memory.minutes_byte.minutes10 = datetime->minutes / 10;
    rtc.memory.minutes_byte.minutes   = datetime->minutes % 10;
    
    rtc.memory.hours_byte.ampm = datetime->is_ampm;
    if(datetime->is_ampm){
        rtc.memory.hours_byte.hours10 = datetime->hours & 0x1;
        if(datetime->pm) BIT_ON(rtc.memory.hours_byte.hours10, 1);
    }else{
        rtc.memory.hours_byte.hours10 = datetime->hours / 10;
    }
    rtc.memory.hours_byte.hours = datetime->hours % 10;
    
    rtc.memory.day_byte.day = datetime->day;
    
    rtc.memory.date_byte.date10 = datetime->date / 10;
    rtc.memory.date_byte.date   = datetime->date % 10;
    
    rtc.memory.month_byte.month10 = datetime->month / 10;
    rtc.memory.month_byte.month   = datetime->month % 10;
    
    rtc.memory.year_byte.year10 = datetime->year / 10;
    rtc.memory.year_byte.year   = datetime->year % 10;
}

bool ds1307_running(void)
{
    return !rtc.memory.seconds_byte.clock_halt;
}

void ds1307_set_running(bool running)
{
    rtc.memory.seconds_byte.clock_halt = !running;
}

err_t ds1307_write_running()
{
    if(future_running(&rtc.future)) return E_DS1307_BUSY;
    
    rtc.size_to_rw = sizeof(seconds_byte_t);
    
    ds1307_start(DS1307_STATUS_WRITE);
    
    return ds1307_do();
}
