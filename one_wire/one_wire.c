#include "one_wire.h"
#include <avr/interrupt.h>
#include <util/crc16.h>
#include "utils/utils.h"
#include "utils/delay.h"
#include "bits/bits.h"
#include "defs/defs.h"


#define ONE_WIRE_RESET_PULSE_MIN_US     480
#define ONE_WIRE_RESET_PULSE_MAX_US     640
#define ONE_WIRE_RESET_PULSE_US         500

#define ONE_WIRE_BUS_REACTION_MIN_US    15
#define ONE_WIRE_BUS_REACTION_MAX_US    60
#define ONE_WIRE_BUS_REACTION_US        30

#define ONE_WIRE_PRESENCE_PULSE_MIN_US  60
#define ONE_WIRE_PRESENCE_PULSE_MAX_US  240
#define ONE_WIRE_PRESENCE_PULSE_US      240

#define ONE_WIRE_FRAMES_SEPARATOR_MIN_US 1
#define ONE_WIRE_FRAMES_SEPARATOR_MAX_US 255
#define ONE_WIRE_FRAMES_SEPARATOR_US    2

#define ONE_WIRE_FRAME_HEAD_US          15

#define ONE_WIRE_DELAY_CORRECTION_US    (3)

#define ONE_WIRE_FRAME_BEGIN_MIN_US     1
#define ONE_WIRE_FRAME_BEGIN_MAX_US     ONE_WIRE_FRAME_HEAD_US
#define ONE_WIRE_FRAME_BEGIN_US         2

#define ONE_WIRE_FRAME_RW_US            60


/**
 * Настраивает порт шины 1-wire на вывод.
 * @param ow Шина 1-wire
 */
ALWAYS_INLINE static void one_wire_set_out(one_wire_t* ow)
{
    pin_set_out(&ow->pin);
}

/**
 * Настраивает порт шины 1-wire на ввод.
 * @param ow Шина 1-wire
 */
ALWAYS_INLINE static void one_wire_set_in(one_wire_t* ow)
{
    pin_set_in(&ow->pin);
}

/**
 * Устанавливает высокий уровень на шине 1-wire.
 * @param ow Шина 1-wire
 */
ALWAYS_INLINE static void one_wire_in_set_hi(one_wire_t* ow)
{
    pin_pullup_enable(&ow->pin);
}

/**
 * Устанавливает низкий уровень на шине 1-wire.
 * @param ow Шина 1-wire
 */
ALWAYS_INLINE static void one_wire_in_set_lo(one_wire_t* ow)
{
    pin_pullup_disable(&ow->pin);
}

/**
 * Устанавливает высокий уровень на шине 1-wire.
 * @param ow Шина 1-wire
 */
ALWAYS_INLINE static void one_wire_out_set_hi(one_wire_t* ow)
{
    pin_on(&ow->pin);
}

/**
 * Устанавливает низкий уровень на шине 1-wire.
 * @param ow Шина 1-wire
 */
ALWAYS_INLINE static void one_wire_out_set_lo(one_wire_t* ow)
{
    pin_off(&ow->pin);
}

/**
 * Начинает кадр передачи данных на шине 1-wire.
 * @param ow Шина 1-wire
 */
ALWAYS_INLINE static void one_wire_frame_begin(one_wire_t* ow)
{
    delay_us8(ONE_WIRE_FRAMES_SEPARATOR_US);
    one_wire_in_set_lo(ow);
    one_wire_set_out(ow);
}

/**
 * Заканчивает кадр передачи данных на шине 1-wire.
 * @param ow Шина 1-wire
 */
ALWAYS_INLINE static void one_wire_frame_end(one_wire_t* ow)
{
    one_wire_set_in(ow);
    one_wire_in_set_hi(ow);
}

/**
 * Устанавливает логическое значение на шине 1-wire.
 */
ALWAYS_INLINE static void one_wire_set_value(one_wire_t* ow, uint8_t value)
{
    pin_set_value(&ow->pin, value);
}

/**
 * Возвращает текущее логическое значение на шине 1-wire.
 * @param ow Шина 1-wire
 */
ALWAYS_INLINE static uint8_t one_wire_get_value(one_wire_t* ow)
{
    return pin_get_value(&ow->pin);
}

err_t one_wire_init(one_wire_t* ow, uint8_t port_n, uint8_t pin_n)
{
    err_t res = pin_init(&ow->pin, port_n, pin_n);
    if(res != E_NO_ERROR) return res;
    
    one_wire_frame_end(ow);
    
    return E_NO_ERROR;
}

uint8_t one_wire_reset(one_wire_t* ow)
{
    __interrupts_save_disable();
    one_wire_frame_begin(ow);
    
    delay_us16(ONE_WIRE_RESET_PULSE_US);
    
    one_wire_frame_end(ow);
    
    delay_us8(ONE_WIRE_BUS_REACTION_US);
    
    uint8_t presence = one_wire_get_value(ow);
    
    delay_us8(ONE_WIRE_PRESENCE_PULSE_US);
    
    __interrupts_restore();
    
    return BIT0_NOT(presence);
}

void one_wire_write_bit(one_wire_t* ow, uint8_t bit)
{
    __interrupts_save_disable();
    one_wire_frame_begin(ow);
    
    delay_us8(ONE_WIRE_FRAME_BEGIN_US);
    
    one_wire_set_value(ow, bit);
    
    delay_us8(ONE_WIRE_FRAME_RW_US - ONE_WIRE_FRAME_BEGIN_US);
    
    one_wire_frame_end(ow);
    __interrupts_restore();
}

uint8_t one_wire_read_bit(one_wire_t* ow)
{
    __interrupts_save_disable();
    one_wire_frame_begin(ow);
    
    delay_us8(ONE_WIRE_FRAME_BEGIN_US);
    
    one_wire_frame_end(ow);
    
    delay_us8(ONE_WIRE_FRAME_HEAD_US - ONE_WIRE_FRAME_BEGIN_US - ONE_WIRE_DELAY_CORRECTION_US);
            
    uint8_t value = one_wire_get_value(ow);
    
    delay_us8(ONE_WIRE_FRAME_RW_US - ONE_WIRE_FRAME_HEAD_US + ONE_WIRE_DELAY_CORRECTION_US);
    
    __interrupts_restore();
    
    return value;
}

void one_wire_write_byte(one_wire_t* ow, uint8_t byte)
{
    uint8_t i = 0;
    for(; i < 8; i ++){
        one_wire_write_bit(ow, byte & 0x1);
        byte >>= 1;
    }
}

uint8_t one_wire_read_byte(one_wire_t* ow)
{
    uint8_t byte = 0;
    uint8_t i = 7;
    for(;;){
        if(one_wire_read_bit(ow)) byte |= 0x80;
        if(i == 0) break;
        i --;
        byte >>= 1;
    }
    return byte;
}

void one_wire_write(one_wire_t* ow, const void* data, one_wire_size_t size)
{
    one_wire_size_t i = 0;
    for(; i < size; i ++){
        one_wire_write_byte(ow, ((const uint8_t*)data)[i]);
    }
}

void one_wire_read(one_wire_t* ow, void* data, one_wire_size_t size)
{
    one_wire_size_t i = 0;
    for(; i < size; i ++){
        ((uint8_t*)data)[i] = one_wire_read_byte(ow);
    }
}

uint8_t one_wire_calc_crc(const void* data, one_wire_size_t size)
{
    one_wire_size_t i = 0;
    uint8_t crc = 0;
    for(i = 0; i < size; i ++){
        crc = _crc_ibutton_update(crc, ((const uint8_t*)data)[i]);
    }
    return crc;
}

err_t one_wire_read_rom(one_wire_t* ow, one_wire_rom_id_t* rom)
{
    uint8_t crc = 0;
    //uint8_t i = 0;
    
    //if(!one_wire_reset(ow)) return E_ONE_WIRE_DEVICES_NOT_FOUND;
    
    one_wire_send_cmd(ow, ONE_WIRE_CMD_READ_ROM);
    
    /*rom->family_code = one_wire_read_byte(ow);
    for(i = 0; i < ONE_WIRE_SERIAL_LEN; i ++){
        one_wire_write_byte(ow, rom->serial[i]);
    }
    rom->crc = one_wire_read_byte(ow);*/
    one_wire_read(ow, rom, sizeof(one_wire_rom_id_t));
    
    crc = one_wire_calc_crc((const void*)rom, 0x7);
    if(crc != rom->crc) return E_ONE_WIRE_INVALID_CRC;
    
    return E_NO_ERROR;
}

err_t one_wire_match_rom(one_wire_t* ow, one_wire_rom_id_t* rom)
{
    //uint8_t i = 0;
    
    //if(!one_wire_reset(ow)) return E_ONE_WIRE_DEVICES_NOT_FOUND;
    
    one_wire_send_cmd(ow, ONE_WIRE_CMD_MATCH_ROM);
    
    /*one_wire_write_byte(ow, rom->family_code);
    for(i = 0; i < ONE_WIRE_SERIAL_LEN; i ++){
        one_wire_write_byte(ow, rom->serial[i]);
    }
    one_wire_write_byte(ow, rom->crc);*/
    one_wire_write(ow, rom, sizeof(one_wire_rom_id_t));
    
    return E_NO_ERROR;
}

err_t one_wire_skip_rom(one_wire_t* ow)
{
    //if(!one_wire_reset(ow)) return E_ONE_WIRE_DEVICES_NOT_FOUND;
    
    one_wire_send_cmd(ow, ONE_WIRE_CMD_SKIP_ROM);
    
    return E_NO_ERROR;
}
