#include "ssi.h"
#include "defs/defs.h"
#include "utils/utils.h"
#include "utils/delay.h"


//#define SSI_DISABLE_INTERRUPTS

#ifdef SSI_DISABLE_INTERRUPTS
#include <avr/interrupt.h>
#endif



//! Максимальное значение.
#define SSI_BIT_ORDER_MAX SSI_BIT_ORDER_LSB_TO_MSB
#define SSI_PULLUP_MAX SSI_PULLUP_VCC
#define SSI_BIT_READ_AT_EDGE_MAX SSI_BIT_READ_AT_FALLING_EDGE


/**
 * Устанавливает высокий уровень на пине.
 * @param pullup Тип подтяжки.
 * @param pin Пин.
 */
static void ssi_set_hi(ssi_pullup_t pullup, pin_t* pin)
{
    if(pullup == SSI_PULLUP_VCC){
        pin_set_in(pin);
        pin_pullup_enable(pin);
    }else{
        pin_set_out(pin);
        pin_on(pin);
    }
}

/**
 * Устанавливает низкий уровень на пине.
 * @param pullup Тип подтяжки.
 * @param pin Пин.
 */
static void ssi_set_lo(ssi_pullup_t pullup, pin_t* pin)
{
    if(pullup == SSI_PULLUP_VCC){
        pin_pullup_disable(pin);
        pin_set_out(pin);
    }else{
        pin_off(pin);
        pin_set_in(pin);
    }
}

/**
 * Устанавливает высокий уровень на шине SDA.
 * @param ssi Последовательная шина.
 */
ALWAYS_INLINE static void ssi_sda_hi(ssi_t* ssi)
{
    ssi_set_hi(ssi->sda_pullup, &ssi->pin_sda);
}

/**
 * Устанавливает низкий уровень на шине SDA.
 * @param ssi Последовательная шина.
 */
ALWAYS_INLINE static void ssi_sda_lo(ssi_t* ssi)
{
    ssi_set_lo(ssi->sda_pullup, &ssi->pin_sda);
}

/**
 * Устанавливает высокий уровень на шине SCL.
 * @param ssi Последовательная шина.
 */
ALWAYS_INLINE static void ssi_scl_hi(ssi_t* ssi)
{
    ssi_set_hi(ssi->scl_pullup, &ssi->pin_scl);
}

/**
 * Устанавливает низкий уровень на шине SCL.
 * @param ssi Последовательная шина.
 */
ALWAYS_INLINE static void ssi_scl_lo(ssi_t* ssi)
{
    ssi_set_lo(ssi->scl_pullup, &ssi->pin_scl);
}

/**
 * Настраивает шину SDA на запись.
 * @param ssi Последовательная шина.
 */
ALWAYS_INLINE static void ssi_sda_setup_write(ssi_t* ssi)
{
    ssi_sda_lo(ssi);
}

/**
 * Настраивает шину SDA на чтение.
 * @param ssi Последовательная шина.
 */
ALWAYS_INLINE static void ssi_sda_setup_read(ssi_t* ssi)
{
    pin_set_in(&ssi->pin_sda);
    pin_pullup_disable(&ssi->pin_sda);
}

void ssi_begin(ssi_t* ssi)
{
    ssi_scl_lo(ssi);
    //ssi_sda_lo(ssi);
}

void ssi_end(ssi_t* ssi)
{
    pin_set_in(&ssi->pin_scl);
    pin_pullup_disable(&ssi->pin_scl);
    pin_set_in(&ssi->pin_sda);
    pin_pullup_disable(&ssi->pin_sda);
}

err_t ssi_init(ssi_t* ssi,
                uint8_t sda_port_n, uint8_t sda_pin_n, ssi_pullup_t sda_pullup,
                uint8_t scl_port_n, uint8_t scl_pin_n, ssi_pullup_t scl_pullup,
                ssi_bit_order_t bit_order, ssi_bit_read_pos_t bit_read_pos,
                uint8_t half_period_time_us)
{
    if(bit_order > SSI_BIT_ORDER_MAX) return E_INVALID_VALUE;
    if(sda_pullup > SSI_PULLUP_MAX) return E_INVALID_VALUE;
    if(scl_pullup > SSI_PULLUP_MAX) return E_INVALID_VALUE;
    if(bit_read_pos > SSI_BIT_READ_AT_EDGE_MAX) return E_INVALID_VALUE;
    
    err_t err = pin_init(&ssi->pin_sda, sda_port_n, sda_pin_n);
    if(err != E_NO_ERROR) return err;
    
    err = pin_init(&ssi->pin_scl, scl_port_n, scl_pin_n);
    if(err != E_NO_ERROR) return err;
    
    ssi->bit_order = bit_order;
    ssi->sda_pullup = sda_pullup;
    ssi->scl_pullup = scl_pullup;
    
    ssi->half_period_time_us = half_period_time_us;
    ssi->bit_read_pos = bit_read_pos;
    
    ssi_end(ssi);
    
    return E_NO_ERROR;
}

void ssi_write_bit(ssi_t* ssi, uint8_t bit)
{
#ifdef SSI_DISABLE_INTERRUPTS
    // Сохраним состояние и запретим прерывания.
    __interrupts_save_disable();
#endif
    // Бит данных.
    if(bit) ssi_sda_hi(ssi);
    else ssi_sda_lo(ssi);
    
    // Цикл чтения/записи.
    // Поднимим SCL
    ssi_scl_hi(ssi);
    // Подождём полупериод.
    delay_us8(ssi->half_period_time_us);
    // Опустим SCL.
    ssi_scl_lo(ssi);
    // Опустим SDA.
    ssi_sda_lo(ssi);
    // Подождём полупериод.
    delay_us8(ssi->half_period_time_us);
#ifdef SSI_DISABLE_INTERRUPTS
    // Восстановим прерывания.
    __interrupts_restore();
#endif
}

/**
 * Записывает последний бит команды чтения в шину.
 * @param ssi Последовательный интерфейс.
 * @param bit Бит, имеет значние лишь равенство или не равенство нулю.
 */
static void ssi_write_last_read_cmd_bit(ssi_t* ssi, uint8_t bit)
{
#ifdef SSI_DISABLE_INTERRUPTS
    // Сохраним состояние и запретим прерывания.
    __interrupts_save_disable();
#endif
    // Бит данных.
    if(bit) ssi_sda_hi(ssi);
    else ssi_sda_lo(ssi);
    
    // Цикл чтения/записи.
    // Поднимим SCL
    ssi_scl_hi(ssi);
    // Подождём полупериод.
    delay_us8(ssi->half_period_time_us);
    // Сперва установим SDA на чтение.
    ssi_sda_setup_read(ssi);
    //Если чтение происходит при восходящем фронте синхроимпульса
    if(ssi->bit_read_pos == SSI_BIT_READ_AT_RISING_EDGE){
        // Опустим SCL.
        ssi_scl_lo(ssi);
        // Подождём полупериод.
        delay_us8(ssi->half_period_time_us);
    }
#ifdef SSI_DISABLE_INTERRUPTS
    // Восстановим прерывания.
    __interrupts_restore();
#endif
}

/**
 * Записывает байт в шину.
 * @param ssi Последовательный интерфейс.
 * @param byte Байт данных.
 */
static void ssi_write_byte_impl(ssi_t* ssi, uint8_t byte)
{
    uint8_t i;
    if(ssi->bit_order == SSI_BIT_ORDER_MSB_TO_LSB){
        for(i = 0; i < 8; i ++){
            ssi_write_bit(ssi, byte & 0x80);
            if(i < 7){
                byte <<= 1;
            }
        }
    }else{
        for(i = 0; i < 8; i ++){
            ssi_write_bit(ssi, byte & 0x1);
            if(i < 7){
                byte >>= 1;
            }
        }
    }
}

/**
 * Записывает байт команды чтения в шину.
 * @param ssi Последовательный интерфейс.
 * @param byte Байт данных.
 */
static void ssi_write_read_cmd_byte_impl(ssi_t* ssi, uint8_t byte)
{
    uint8_t i;
    if(ssi->bit_order == SSI_BIT_ORDER_MSB_TO_LSB){
        for(i = 0; i < 8; i ++){
            if(i < 7){
                ssi_write_bit(ssi, byte & 0x80);
                byte <<= 1;
            }else{
                ssi_write_last_read_cmd_bit(ssi, byte & 0x80);
            }
        }
    }else{
        for(i = 0; i < 8; i ++){
            if(i < 7){
                ssi_write_bit(ssi, byte & 0x1);
                byte >>= 1;
            }else{
                ssi_write_last_read_cmd_bit(ssi, byte & 0x1);
            }
        }
    }
}

/**
 * Читает бит из шины после поднятия синхроимпульса.
 * @param ssi Последовательный интерфейс.
 * @return Прочитанный бит.
 */
ALWAYS_INLINE static uint8_t ssi_read_bit_rising(ssi_t* ssi)
{
#ifdef SSI_DISABLE_INTERRUPTS
    // Сохраним состояние и запретим прерывания.
    __interrupts_save_disable();
#endif
    // Рузультат.
    uint8_t bit = 0;
    
    // Цикл чтения/записи.
    // Поднимим SCL
    ssi_scl_hi(ssi);
    // Подождём полупериод.
    delay_us8(ssi->half_period_time_us);
    // Считаем значение SDA.
    bit = pin_get_value(&ssi->pin_sda);
    // Опустим SCL.
    ssi_scl_lo(ssi);
    // Подождём полупериод.
    delay_us8(ssi->half_period_time_us);
#ifdef SSI_DISABLE_INTERRUPTS
    // Восстановим прерывания.
    __interrupts_restore();
#endif
    return bit;
}

/**
 * Читает бит из шины после падения синхроимпульса.
 * @param ssi Последовательный интерфейс.
 * @return Прочитанный бит.
 */
ALWAYS_INLINE static uint8_t ssi_read_bit_falling(ssi_t* ssi)
{
#ifdef SSI_DISABLE_INTERRUPTS
    // Сохраним состояние и запретим прерывания.
    __interrupts_save_disable();
#endif
    // Рузультат.
    uint8_t bit = 0;
    
    // Цикл чтения/записи.
    // Поднимим SCL
    ssi_scl_hi(ssi);
    // Подождём полупериод.
    delay_us8(ssi->half_period_time_us);
    // Опустим SCL.
    ssi_scl_lo(ssi);
    // Подождём полупериод.
    delay_us8(ssi->half_period_time_us);
    // Считаем значение SDA.
    bit = pin_get_value(&ssi->pin_sda);
#ifdef SSI_DISABLE_INTERRUPTS
    // Восстановим прерывания.
    __interrupts_restore();
#endif
    return bit;
}

uint8_t ssi_read_bit(ssi_t* ssi)
{
    if(ssi->bit_read_pos == SSI_BIT_READ_AT_RISING_EDGE) return ssi_read_bit_rising(ssi);
    return ssi_read_bit_falling(ssi);
}

/**
 * Записывает байт в шину.
 * @param ssi Последовательный интерфейс.
 * @param byte Байт данных.
 */
static uint8_t ssi_read_byte_impl(ssi_t* ssi)
{
    // Рузультат.
    uint8_t byte = 0;
    
    uint8_t i;
    if(ssi->bit_order == SSI_BIT_ORDER_MSB_TO_LSB){
        for(i = 0; i < 8; i ++){
            if(ssi_read_bit(ssi)) byte |= 0x1;
            if(i < 7){
                byte <<= 1;
            }
        }
    }else{
        for(i = 0; i < 8; i ++){
            if(ssi_read_bit(ssi)) byte |= 0x80;
            if(i < 7){
                byte >>= 1;
            }
        }
    }
    
    return byte;
}

void ssi_write_byte(ssi_t* ssi, uint8_t byte)
{
    ssi_sda_setup_write(ssi);
    
    ssi_write_byte_impl(ssi, byte);
}

void ssi_write(ssi_t* ssi, const void* data, ssi_size_t size)
{
    ssi_sda_setup_write(ssi);
    
    uint8_t i;
    for(i = 0; i < size; i ++){
        ssi_write_byte_impl(ssi, ((const uint8_t*)data)[i]);
    }
}

uint8_t ssi_read_byte(ssi_t* ssi)
{
    ssi_sda_setup_read(ssi);
    
    uint8_t byte = ssi_read_byte_impl(ssi);
    
    return byte;
}

void ssi_read(ssi_t* ssi, void* data, ssi_size_t size)
{
    ssi_sda_setup_read(ssi);
    
    uint8_t i;
    for(i = 0; i < size; i ++){
        ((uint8_t*)data)[i] = ssi_read_byte_impl(ssi);
    }
}

void ssi_cmd_read(ssi_t* ssi, uint8_t cmd, void* data, ssi_size_t size)
{
    ssi_sda_setup_write(ssi);
    
    ssi_write_read_cmd_byte_impl(ssi, cmd);
    
    ssi_read(ssi, data, size);
}
