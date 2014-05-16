#include "ds1302.h"
#include <string.h>
#include <stddef.h>
#include "utils/utils.h"
#include "utils/delay.h"
#include "bits/bits.h"

//! Задержка перед и после передачи.
#define DS1302_IO_SEPARATOR_US 8
#define DS1302_START_IO_DELAY_US 4
#define DS1302_END_IO_DELAY_US 4
//! F_clk / 2
#define DS1302_IO_HALF_PERIOD_US 1

//! Размер читаемой/записываемой памяти
#define DS1302_IO_MEM_SIZE 8

//! Байт адреса/команды.
#define DS1302_CMD_BYTE 0x80
#define DS1302_RAM_BIT 6
#define DS1302_ADDRESS_BIT 1
#define DS1302_ADDRESS_BITS 5
#define DS1302_ADDRESS_MASK BIT_MAKE_MASK(DS1302_ADDRESS_BITS, DS1302_ADDRESS_BIT);
#define DS1302_ADDRESS_MAX 0x1f
#define DS1302_RW_BIT 0


static uint8_t make_cmd_byte(bool ram, uint8_t address, bool read)
{
    uint8_t cmd = DS1302_CMD_BYTE;
    
    if(ram) BIT_ON(cmd, DS1302_RAM_BIT);
    cmd |= (address << DS1302_ADDRESS_BIT) & DS1302_ADDRESS_MASK;
    if(read) BIT_ON(cmd, DS1302_RW_BIT);
    
    return cmd;
}

err_t ds1302_init(ds1302_t* rtc,
                  uint8_t sda_port_n, uint8_t sda_pin_n,
                  uint8_t scl_port_n, uint8_t scl_pin_n,
                  uint8_t sel_port_n, uint8_t sel_pin_n)
{
    err_t err = ssi_init(&rtc->ssi, sda_port_n, sda_pin_n, SSI_PULLUP_NONE_OR_GND,//SSI_PULLUP_NONE_OR_GND
                         scl_port_n, scl_pin_n, SSI_PULLUP_NONE_OR_GND,//SSI_PULLUP_VCC
                         SSI_BIT_ORDER_LSB_TO_MSB, SSI_BIT_READ_AT_FALLING_EDGE, DS1302_IO_HALF_PERIOD_US);
    if(err != E_NO_ERROR) return err;
    
    err = pin_init(&rtc->pin_sel, sel_port_n, sel_pin_n);
    if(err != E_NO_ERROR) return err;
    
    pin_set_out(&rtc->pin_sel);
    pin_off(&rtc->pin_sel);
    
    memset(&rtc->memory, 0x0, sizeof(ds1302mem_t));
    
    return E_NO_ERROR;
}

static void ds1302_read_data(ds1302_t* rtc, uint8_t cmd, void* data, uint8_t size)
{
    ssi_begin(&rtc->ssi);
    
    pin_on(&rtc->pin_sel);
    
    delay_us8(DS1302_START_IO_DELAY_US);
    
    ssi_cmd_read(&rtc->ssi, cmd, data, size);
    
    if(size == DS1302_IO_MEM_SIZE) ssi_write_bit(&rtc->ssi, 0);
    
    delay_us8(DS1302_END_IO_DELAY_US);
    
    pin_off(&rtc->pin_sel);
    
    ssi_end(&rtc->ssi);
    
    delay_us8(DS1302_IO_SEPARATOR_US);
}

static void ds1302_write_data(ds1302_t* rtc, uint8_t cmd, const void* data, uint8_t size)
{
    ssi_begin(&rtc->ssi);
    
    pin_on(&rtc->pin_sel);
    
    delay_us8(DS1302_START_IO_DELAY_US);
    
    ssi_write_byte(&rtc->ssi, cmd);
    ssi_write(&rtc->ssi, data, size);
    
    if(size == DS1302_IO_MEM_SIZE) ssi_write_bit(&rtc->ssi, 0);
    
    delay_us8(DS1302_END_IO_DELAY_US);
    
    pin_off(&rtc->pin_sel);
    
    ssi_end(&rtc->ssi);
    
    delay_us8(DS1302_IO_SEPARATOR_US);
}

void ds1302_read(ds1302_t* rtc)
{
    uint8_t cmd = make_cmd_byte(false, DS1302_ADDRESS_MAX, true);
    ds1302_read_data(rtc, cmd, &rtc->memory, DS1302_IO_MEM_SIZE);
}

void ds1302_write(ds1302_t* rtc)
{
    uint8_t cmd = make_cmd_byte(false, DS1302_ADDRESS_MAX, false);
    ds1302_write_data(rtc, cmd, &rtc->memory, DS1302_IO_MEM_SIZE);
}

void ds1302_datetime_get(ds1302_t* rtc, ds1302_datetime_t* datetime)
{
    datetime->seconds = rtc->memory.seconds_byte.seconds10 * 10 + rtc->memory.seconds_byte.seconds;
    datetime->minutes = rtc->memory.minutes_byte.minutes10 * 10 + rtc->memory.minutes_byte.minutes;
    if(rtc->memory.hours_byte.ampm){
        datetime->is_ampm = true;
        datetime->pm = BIT_VALUE(rtc->memory.hours_byte.hours10, 1);
        datetime->hours   = (rtc->memory.hours_byte.hours10 & 0x1) * 10 + rtc->memory.hours_byte.hours;
    }else{
        datetime->is_ampm = false;
        datetime->hours   = rtc->memory.hours_byte.hours10 * 10 + rtc->memory.hours_byte.hours;
    }
    datetime->day = rtc->memory.day_byte.day;
    datetime->date = rtc->memory.date_byte.date10 * 10 + rtc->memory.date_byte.date;
    datetime->month = rtc->memory.month_byte.month10 * 10 + rtc->memory.month_byte.month;
    datetime->year = rtc->memory.year_byte.year10 * 10 + rtc->memory.year_byte.year;
}

void ds1302_datetime_set(ds1302_t* rtc, ds1302_datetime_t* datetime)
{
    rtc->memory.seconds_byte.seconds10 = datetime->seconds / 10;
    rtc->memory.seconds_byte.seconds   = datetime->seconds % 10;
    
    rtc->memory.minutes_byte.minutes10 = datetime->minutes / 10;
    rtc->memory.minutes_byte.minutes   = datetime->minutes % 10;
    
    rtc->memory.hours_byte.ampm = datetime->is_ampm;
    if(datetime->is_ampm){
        rtc->memory.hours_byte.hours10 = datetime->hours & 0x1;
        if(datetime->pm) BIT_ON(rtc->memory.hours_byte.hours10, 1);
    }else{
        rtc->memory.hours_byte.hours10 = datetime->hours / 10;
    }
    rtc->memory.hours_byte.hours = datetime->hours % 10;
    
    rtc->memory.day_byte.day = datetime->day;
    
    rtc->memory.date_byte.date10 = datetime->date / 10;
    rtc->memory.date_byte.date   = datetime->date % 10;
    
    rtc->memory.month_byte.month10 = datetime->month / 10;
    rtc->memory.month_byte.month   = datetime->month % 10;
    
    rtc->memory.year_byte.year10 = datetime->year / 10;
    rtc->memory.year_byte.year   = datetime->year % 10;
}

bool ds1302_running(ds1302_t* rtc)
{
    return !rtc->memory.seconds_byte.clock_halt;
}

void ds1302_set_running(ds1302_t* rtc, bool running)
{
    rtc->memory.seconds_byte.clock_halt = !running;
}

void ds1302_write_running(ds1302_t* rtc)
{
    uint8_t cmd = make_cmd_byte(false, offsetof(ds1302mem_t, seconds_byte), false);
    ds1302_write_data(rtc, cmd, &rtc->memory.seconds_byte, 1);
}

bool ds1302_write_protected(ds1302_t* rtc)
{
    return rtc->memory.wp_byte.write_protect;
}

void ds1302_set_write_protected(ds1302_t* rtc, bool write_protected)
{
    rtc->memory.wp_byte.write_protect = write_protected;
}

void ds1302_write_write_protected(ds1302_t* rtc)
{
    uint8_t cmd = make_cmd_byte(false, offsetof(ds1302mem_t, wp_byte), false);
    ds1302_write_data(rtc, cmd, &rtc->memory.wp_byte, 1);
}

