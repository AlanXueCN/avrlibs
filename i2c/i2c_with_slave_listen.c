#include "i2c.h"
#include "buffer/buffer.h"
#include "bits/bits.h"
#include <avr/interrupt.h>
#include <util/twi.h>
#include <string.h>

#ifndef F_CPU
#warning i2c: F_CPU is not defined. Defaulting to 1MHz.
#define F_CPU 1000000
#endif

#define F_CPU_KHZ (F_CPU / 1000)
#define F_CPU_DIV16_KHZ (F_CPU_KHZ / 16)


#define I2C_ACK  1
#define I2C_NACK 0

#define I2C_RESPONSE_ALLOW 1
#define I2C_RESPONSE_DENY  0

#define I2C_SLAVE_DATA_ERR_VALUE 0xff

/**
 * Группа из буфера адреса и буфера данных.
 */
typedef struct _I2C_Data {
    buffer_t address_buffer;
    buffer_t data_buffer;
}i2c_data_t;

/**
 * Инициализирует данные для передачи/приёма.
 * @param i2c_data Данные для передачи/приёма.
 * @param rom_address буфер адреса в устройстве.
 * @param rom_address_size размер буфера адреса.
 * @param data данные для передачи/приёма.
 * @param data_size размер данных.
 */
ALWAYS_INLINE static void i2c_data_init(i2c_data_t* i2c_data, void* rom_address, i2c_size_t rom_address_size, void* data, i2c_size_t data_size)
{
    buffer_init(&i2c_data->address_buffer, rom_address, rom_address_size);
    buffer_init(&i2c_data->data_buffer, data, data_size);
}

/**
 * Получает количество переданных или полученных данных.
 * @param data Данные.
 * @return Количество переданных или полученных данных.
 */
ALWAYS_INLINE static i2c_size_t i2c_data_bytes_transmitted(i2c_data_t* data)
{
    return data->address_buffer.pos + data->data_buffer.pos;
}

/**
 * Получает флаг наличия адреса.
 * @param data Данные.
 * @return  Флаг наличия адреса.
 */
ALWAYS_INLINE static bool i2c_data_has_address(i2c_data_t* data)
{
    return data->address_buffer.ptr != NULL;
}

/**
 * Получает флаг наличия следующего байта в буфере с учётом адреса страницы.
 * @param data Данные.
 * @return Флаг наличия следующего байта в буфере.
 */
static bool i2c_data_buffer_has_next(i2c_data_t* data)
{
    if(data->address_buffer.pos != 0){
        i2c_page_address_t* pg_addr = (i2c_page_address_t*) data->address_buffer.ptr;
        if(pg_addr != NULL){
            switch(pg_addr->size){
                case I2C_PAGE_ADDRESS_SIZE_8:
                    return BUFFER_POS_HAS_NEXT((data->data_buffer.pos + pg_addr->address.addr8.value),
                                               data->data_buffer.size);
                case I2C_PAGE_ADDRESS_SIZE_16:
                    return BUFFER_POS_HAS_NEXT((pg_addr->address.addr16.value + data->data_buffer.pos),
                                               data->data_buffer.size);
#ifdef I2C_NEED_PAGE_ADDRESS_32BIT
                case I2C_PAGE_ADDRESS_SIZE_32:
                    return BUFFER_POS_HAS_NEXT((pg_addr->address.addr32.value + data->data_buffer.pos),
                                               data->data_buffer.size);
#endif
            }
        }
    }
    return buffer_has_next(&data->data_buffer);
}

/**
 * Получает флаг нахождения указателя в конце буфера с учётом адреса страницы.
 * @param data Данные.
 * @return Флаг нахождения указателя в конце буфера.
 */
static bool i2c_data_buffer_at_end(i2c_data_t* data)
{
    if(data->address_buffer.pos != 0){
        i2c_page_address_t* pg_addr = (i2c_page_address_t*) data->address_buffer.ptr;
        if(pg_addr != NULL){
            switch(pg_addr->size){
                case I2C_PAGE_ADDRESS_SIZE_8:
                    return BUFFER_POS_AT_END((data->data_buffer.pos + pg_addr->address.addr8.value),
                                               data->data_buffer.size);
                case I2C_PAGE_ADDRESS_SIZE_16:
                    return BUFFER_POS_AT_END((pg_addr->address.addr16.value + data->data_buffer.pos),
                                               data->data_buffer.size);
#ifdef I2C_NEED_PAGE_ADDRESS_32BIT
                case I2C_PAGE_ADDRESS_SIZE_32:
                    return BUFFER_POS_AT_END((pg_addr->address.addr32.value + data->data_buffer.pos),
                                               data->data_buffer.size);
#endif
            }
        }
    }
    return buffer_at_end(&data->data_buffer);
}

/**
 * Получает флаг нахождения указателя перед концом буфера с учётом адреса страницы.
 * @param data Данные.
 * @return Флаг нахождения указателя перед концом буфера.
 */
static bool i2c_data_buffer_at_last(i2c_data_t* data)
{
    if(data->address_buffer.pos != 0){
        i2c_page_address_t* pg_addr = (i2c_page_address_t*) data->address_buffer.ptr;
        if(pg_addr != NULL){
            switch(pg_addr->size){
                case I2C_PAGE_ADDRESS_SIZE_8:
                    return BUFFER_POS_AT_LAST((data->data_buffer.pos + pg_addr->address.addr8.value),
                                               data->data_buffer.size);
                case I2C_PAGE_ADDRESS_SIZE_16:
                    return BUFFER_POS_AT_LAST((pg_addr->address.addr16.value + data->data_buffer.pos),
                                               data->data_buffer.size);
#ifdef I2C_NEED_PAGE_ADDRESS_32BIT
                case I2C_PAGE_ADDRESS_SIZE_32:
                    return BUFFER_POS_AT_LAST((pg_addr->address.addr32.value + data->data_buffer.pos),
                                               data->data_buffer.size);
#endif
            }
        }
    }
    return buffer_at_last(&data->data_buffer);
}

/**
 * Получает байт из буфера в текущей позиции.
 * @param data Данные.
 * @return Байт из буфера в текущей позиции.
 */
static uint8_t i2c_data_buffer_get(i2c_data_t* data)
{
    if(data->address_buffer.pos != 0){
        i2c_page_address_t* pg_addr = (i2c_page_address_t*) data->address_buffer.ptr;
        if(pg_addr != NULL){
            switch(pg_addr->size){
                case I2C_PAGE_ADDRESS_SIZE_8:
                    return BUFFER_POS_GET(data->data_buffer.ptr,
                            (data->data_buffer.pos + pg_addr->address.addr8.value));
                case I2C_PAGE_ADDRESS_SIZE_16:
                    return BUFFER_POS_GET(data->data_buffer.ptr,
                            (pg_addr->address.addr16.value + data->data_buffer.pos));
#ifdef I2C_NEED_PAGE_ADDRESS_32BIT
                case I2C_PAGE_ADDRESS_SIZE_32:
                    return BUFFER_POS_GET(data->data_buffer.ptr,
                            (pg_addr->address.addr32.value + data->data_buffer.pos));
#endif
            }
        }
    }
    return buffer_get(&data->data_buffer);
}

/**
 * Получает байт из буфера в текущей позиции, инкрементируя указатель.
 * @param data Данные.
 * @return Байт из буфера в текущей позиции, инкрементируя указатель.
 */
static uint8_t i2c_data_buffer_get_next(i2c_data_t* data)
{
    uint8_t byte = i2c_data_buffer_get(data);
    data->data_buffer.pos ++;
    return byte;
}

/**
 * Устанавливает байт в буфере в текущей позиции.
 * @param data Данные.
 */
static void i2c_data_buffer_set(i2c_data_t* data, uint8_t byte)
{
    if(data->address_buffer.pos != 0){
        i2c_page_address_t* pg_addr = (i2c_page_address_t*) data->address_buffer.ptr;
        if(pg_addr != NULL){
            switch(pg_addr->size){
                case I2C_PAGE_ADDRESS_SIZE_8:
                    BUFFER_POS_SET(data->data_buffer.ptr,
                            (data->data_buffer.pos + pg_addr->address.addr8.value), byte);
                    break;
                case I2C_PAGE_ADDRESS_SIZE_16:
                    BUFFER_POS_SET(data->data_buffer.ptr,
                            (pg_addr->address.addr16.value + data->data_buffer.pos), byte);
                    break;
#ifdef I2C_NEED_PAGE_ADDRESS_32BIT
                case I2C_PAGE_ADDRESS_SIZE_32:
                    BUFFER_POS_SET(data->data_buffer.ptr,
                            (pg_addr->address.addr32.value + data->data_buffer.pos), byte);
                    break;
#endif
            }
        }
    }
    buffer_set(&data->data_buffer, byte);
}

/**
 * Устанавливает байт в буфере в текущей позиции, инкрементируя указатель.
 * @param data Данные.
 */
static void i2c_data_buffer_set_next(i2c_data_t* data, uint8_t byte)
{
    i2c_data_buffer_set(data, byte);
    data->data_buffer.pos ++;
}

/**
 * Получает Получает оставшееся число байт данных до конца.
 * @param data Данные.
 * @return Оставшееся число байт.
 */
static uint8_t i2c_data_buffer_remain(i2c_data_t* data)
{
    if(data->address_buffer.pos != 0){
        i2c_page_address_t* pg_addr = (i2c_page_address_t*) data->address_buffer.ptr;
        if(pg_addr != NULL){
            switch(pg_addr->size){
                case I2C_PAGE_ADDRESS_SIZE_8:
                    return data->data_buffer.size - (data->data_buffer.pos + pg_addr->address.addr8.value);
                case I2C_PAGE_ADDRESS_SIZE_16:
                    return data->data_buffer.size - (pg_addr->address.addr16.value + data->data_buffer.pos);
#ifdef I2C_NEED_PAGE_ADDRESS_32BIT
                case I2C_PAGE_ADDRESS_SIZE_32:
                    return data->data_buffer.size - (pg_addr->address.addr32.value + data->data_buffer.pos);
#endif
            }
        }
    }
    return buffer_remain(&data->data_buffer);
}

ALWAYS_INLINE static void i2c_data_addr_buffer_reset(i2c_data_t* data)
{
    /*if(data->address_buffer.pos != 0){
        i2c_page_address_t* pg_addr = (i2c_page_address_t*) data->address_buffer.ptr;
        if(pg_addr != NULL){
            switch(pg_addr->size){
                case I2C_PAGE_ADDRESS_SIZE_8:
                    pg_addr->address.addr8.value = 0;
                    break;
                case I2C_PAGE_ADDRESS_SIZE_16:
                    pg_addr->address.addr16.value = 0;
                    break;
#ifdef I2C_NEED_PAGE_ADDRESS_32BIT
                case I2C_PAGE_ADDRESS_SIZE_32:
                    pg_addr->address.addr32.value = 0;
                    break;
#endif
            }
        }
    }*/
    buffer_reset(&data->address_buffer);
}

#define I2C_MASTER_IO_DIRECTION_WRITE 0
#define I2C_MASTER_IO_DIRECTION_READ 1

/**
 * Данные мастера.
 */
typedef struct _I2C_Master_Data {
    i2c_data_t data;
    i2c_address_t device;
    uint8_t io_direction;
}i2c_master_data_t;

/**
 * Данные слейва.
 */
typedef struct _I2C_Slave_Data {
    i2c_data_t data;
}i2c_slave_data_t;

/**
 * Состояние шины i2c.
 */
typedef struct _I2C_State{
    i2c_callback_t callback;
    i2c_transfer_id_t transfer_id;
    i2c_status_t status;
    err_t error;
    bool listening;
    bool interrupted;
    i2c_master_data_t master;
    i2c_slave_data_t slave;
}i2c_state_t;

/**
 * Текущее состояние шины i2c.
 */
static i2c_state_t _i2c_state;


/**
 * Создаёт старт на шине.
 */
ALWAYS_INLINE static void i2c_do_start(void)
{
    TWCR = (1 << TWINT) |
           (_i2c_state.listening << TWEA) |
           (1 << TWSTA) |
           (1 << TWEN)  |
           (1 << TWIE);
}

/**
 * Создаёт стоп на шине.
 */
ALWAYS_INLINE static void i2c_do_stop(void)
{
    TWCR = (1 << TWINT) |
           (_i2c_state.listening << TWEA) |
           (1 << TWSTO) |
           (1 << TWEN)  |
           (1 << TWIE);
}

/**
 * Слушает шину в ожидании обращения.
 */
ALWAYS_INLINE static void i2c_do_listen(void)
{
    TWCR = (1 << TWINT) |
           (_i2c_state.listening << TWEA) |
           (1 << TWEN)  |
           (1 << TWIE);
}

/**
 * Указывает TWI произвести приём/передачу следующего байта.
 * @param ack управление битом ACK.
 */
ALWAYS_INLINE static void i2c_do_rw_byte(uint8_t ack)
{
    TWCR = (1 << TWINT) |
           (ack << TWEA)|
           (1 << TWEN)  |
           (1 << TWIE);
}

/**
 * Устанавливает код ошибки шины i2c.
 * @param err Код ошибки.
 * @return Код ошибки.
 */
ALWAYS_INLINE static err_t i2c_set_error(err_t err)
{
    _i2c_state.error = err;
    return err;
}

/**
 * Устанавливает статус шины i2c.
 * @param status Статус.
 * @return Статус.
 */
ALWAYS_INLINE static i2c_status_t i2c_set_status(i2c_status_t status)
{
    _i2c_state.status = status;
    return status;
}

void i2c_init(void)
{
    memset(&_i2c_state, 0x0, sizeof(i2c_state_t));
    i2c_do_listen();
}

err_t i2c_set_freq(uint16_t freq)
{
    if(freq > F_CPU_DIV16_KHZ) return i2c_set_error(E_I2C_INVALID_FREQ);
    
    uint16_t twbr = ((uint16_t)F_CPU_KHZ / freq - 16) / 2;
    uint8_t twps = 0;
    uint8_t ps_val = 1;
    
    while(twbr > 255){
        twps ++;
        ps_val *= 4;
        twbr /= ps_val;
    }
    
    if(twps > 0x3 /* 0b11 */) return i2c_set_error(E_I2C_INVALID_FREQ);
    
    TWBR = (uint8_t)twbr;
    TWSR = twps;
    
    return i2c_set_error(E_NO_ERROR);
}

void i2c_set_address(i2c_address_t address, bool bcast_enabled)
{
    TWAR = (address << TWA0) | ((uint8_t)bcast_enabled << TWGCE);
}

i2c_status_t i2c_status(void)
{
    return _i2c_state.status;
}

err_t i2c_error(void)
{
    return _i2c_state.error;
}

i2c_size_t i2c_master_bytes_transmitted(void)
{
    return i2c_data_bytes_transmitted(&_i2c_state.master.data);
}

i2c_size_t i2c_slave_bytes_transmitted(void)
{
    return i2c_data_bytes_transmitted(&_i2c_state.slave.data);
}

bool i2c_is_busy(void)
{
    switch(_i2c_state.status){
        case I2C_STATUS_READING:
        case I2C_STATUS_WRITING:
            return true;
        default:
            break;
    }
    return false;//!(TWCR & (1<<TWINT));
}

i2c_callback_t i2c_callback(void)
{
    return _i2c_state.callback;
}

void i2c_set_callback(i2c_callback_t callback)
{
    _i2c_state.callback = callback;
}

i2c_transfer_id_t i2c_transfer_id(void)
{
    return _i2c_state.transfer_id;
}

void i2c_set_transfer_id(i2c_transfer_id_t id)
{
    _i2c_state.transfer_id = id;
}

ALWAYS_INLINE static bool i2c_m_addr_buffer_has_next(void)
{
    return buffer_has_next(&_i2c_state.master.data.address_buffer);
}

ALWAYS_INLINE static bool i2c_m_data_buffer_has_next(void)
{
    return buffer_has_next(&_i2c_state.master.data.data_buffer);
}

ALWAYS_INLINE static uint8_t i2c_m_addr_buffer_get_next(void)
{
    return buffer_get_next(&_i2c_state.master.data.address_buffer);
}

ALWAYS_INLINE static uint8_t i2c_m_data_buffer_get_next(void)
{
    return buffer_get_next(&_i2c_state.master.data.data_buffer);
}

ALWAYS_INLINE static void i2c_m_addr_buffer_reset(void)
{
    buffer_reset(&_i2c_state.master.data.address_buffer);
}

ALWAYS_INLINE static void i2c_m_data_buffer_reset(void)
{
    buffer_reset(&_i2c_state.master.data.data_buffer);
}

ALWAYS_INLINE static bool i2c_m_addr_buffer_at_last(void)
{
    return buffer_at_last(&_i2c_state.master.data.address_buffer);
}

ALWAYS_INLINE static bool i2c_m_data_buffer_at_last(void)
{
    return buffer_at_last(&_i2c_state.master.data.data_buffer);
}

ALWAYS_INLINE static void i2c_m_addr_buffer_set_next(uint8_t byte)
{
    buffer_set_next(&_i2c_state.master.data.address_buffer, byte);
}

ALWAYS_INLINE static void i2c_m_data_buffer_set_next(uint8_t byte)
{
    buffer_set_next(&_i2c_state.master.data.data_buffer, byte);
}

ALWAYS_INLINE static bool i2c_s_addr_buffer_has_next(void)
{
    return buffer_has_next(&_i2c_state.slave.data.address_buffer);
}

ALWAYS_INLINE static bool i2c_s_data_buffer_has_next(void)
{
    return i2c_data_buffer_has_next(&_i2c_state.slave.data);
}

ALWAYS_INLINE static uint8_t i2c_s_data_buffer_get_next(void)
{
    return i2c_data_buffer_get_next(&_i2c_state.slave.data);
}

ALWAYS_INLINE static void i2c_s_addr_buffer_reset(void)
{
    i2c_data_addr_buffer_reset(&_i2c_state.slave.data);
}

ALWAYS_INLINE static void i2c_s_data_buffer_reset(void)
{
    buffer_reset(&_i2c_state.slave.data.data_buffer);
}

ALWAYS_INLINE static bool i2c_s_data_buffer_at_end(void)
{
    return i2c_data_buffer_at_end(&_i2c_state.slave.data);
}

ALWAYS_INLINE static bool i2c_s_addr_buffer_at_last(void)
{
    return buffer_at_last(&_i2c_state.slave.data.address_buffer);
}

ALWAYS_INLINE static bool i2c_s_data_buffer_at_last(void)
{
    return i2c_data_buffer_at_last(&_i2c_state.slave.data);
}

ALWAYS_INLINE static void i2c_s_addr_buffer_set_next(uint8_t byte)
{
    buffer_set_next(&_i2c_state.slave.data.address_buffer, byte);
}

ALWAYS_INLINE static void i2c_s_data_buffer_set_next(uint8_t byte)
{
    i2c_data_buffer_set_next(&_i2c_state.slave.data, byte);
}

static bool i2c_s_buffers_has_more_one(void)
{
    if(i2c_s_addr_buffer_has_next()){
        if(i2c_s_data_buffer_has_next()) return true;
        return !i2c_s_addr_buffer_at_last();
    }else if(i2c_s_data_buffer_has_next()){
        return !i2c_s_data_buffer_at_last();
    }
    return false;
}

ALWAYS_INLINE static void i2c_next_byte(uint8_t ack)
{
    i2c_do_rw_byte(ack);
}

ALWAYS_INLINE static void i2c_mt_next_byte(void)
{
    i2c_next_byte(_i2c_state.listening);
}

ALWAYS_INLINE static void i2c_end(i2c_event_t event)
{
    if(_i2c_state.callback) _i2c_state.callback(event);
}

static void i2c_m_end(i2c_event_t event)
{
    i2c_do_stop();
    i2c_end(event);
}

static void i2c_s_end(i2c_event_t event)
{
    if(_i2c_state.interrupted){
        _i2c_state.interrupted = false;
        i2c_do_start();
    }else{
        i2c_do_listen();
    }
    i2c_end(event);
}

/**
 * Настраивает шину i2c в режим мастер.
 * @param device адрес устройства.
 * @param page_address адрес в устройстве.
 * @param data данные.
 * @param data_size размер данных.
 * @return Код ошибки.
 */
err_t i2c_master_setup_rw(i2c_address_t device, i2c_page_address_t* page_address, void* data, i2c_size_t data_size)
{
    if(i2c_is_busy()) return i2c_set_error(E_I2C_BUS_BUSY);
    if(data == NULL) return i2c_set_error(E_NULL_POINTER);
    if(data_size == 0) return i2c_set_error(E_INVALID_VALUE);
    
    void *safe_rom_address = NULL;
    i2c_size_t safe_rom_address_size = 0;
    
    if(page_address != NULL){
        if(page_address->size > I2C_PAGE_ADDRESS_SIZE_MAX){
            return i2c_set_error(E_OUT_OF_RANGE);
        }
        if(page_address->size == I2C_PAGE_ADDRESS_SIZE_INVALID){
            return i2c_set_error(E_INVALID_VALUE);
        }
        safe_rom_address = (void*)page_address;
        safe_rom_address_size = page_address->size;
    }
    
    i2c_data_init(&_i2c_state.master.data, safe_rom_address, safe_rom_address_size, data, data_size);
    
    _i2c_state.master.device = device;
    
    return i2c_set_error(E_NO_ERROR);
}

/**
 * Настраивает шину i2c в режим слейв.
 * @param page_address адрес в устройстве.
 * @param data данные.
 * @param data_size размер данных.
 * @return Код ошибки.
 */
err_t i2c_slave_setup_rw(i2c_page_address_t* page_address, void* data, i2c_size_t data_size)
{
    if(i2c_is_busy()) return i2c_set_error(E_I2C_BUS_BUSY);
    if(data == NULL) return i2c_set_error(E_NULL_POINTER);
    if(data_size == 0) return i2c_set_error(E_INVALID_VALUE);
    
    void *safe_rom_address = NULL;
    i2c_size_t safe_rom_address_size = 0;
    
    if(page_address != NULL){
        if(page_address->size > I2C_PAGE_ADDRESS_SIZE_MAX){
            return i2c_set_error(E_OUT_OF_RANGE);
        }
        if(page_address->size == I2C_PAGE_ADDRESS_SIZE_INVALID){
            return i2c_set_error(E_INVALID_VALUE);
        }
        safe_rom_address = (void*)page_address;
        safe_rom_address_size = page_address->size;
    }
    
    i2c_data_init(&_i2c_state.slave.data, safe_rom_address, safe_rom_address_size, (void*)data, data_size);
    
    return i2c_set_error(E_NO_ERROR);
}

err_t i2c_master_read(i2c_address_t device, void* data, i2c_size_t data_size)
{
    return i2c_master_read_at(device, NULL, data, data_size);
}

err_t i2c_master_read_at(i2c_address_t device, const i2c_page_address_t* page_address, void* data, i2c_size_t data_size)
{
    err_t err = i2c_master_setup_rw(device, (i2c_page_address_t*)page_address, data, data_size);
    
    if(err == E_NO_ERROR){
        _i2c_state.master.io_direction = I2C_MASTER_IO_DIRECTION_READ;
        i2c_do_start();
    }
    
    return err;
}

err_t i2c_master_write(i2c_address_t device, const void* data, i2c_size_t data_size)
{
    return i2c_master_write_at(device, NULL, data, data_size);
}

err_t i2c_master_write_at(i2c_address_t device, const i2c_page_address_t* page_address, const void* data, i2c_size_t data_size)
{
    err_t err = i2c_master_setup_rw(device, (i2c_page_address_t*)page_address, (void*)data, data_size);
    
    if(err == E_NO_ERROR){
        _i2c_state.master.io_direction = I2C_MASTER_IO_DIRECTION_WRITE;
        i2c_do_start();
    }
    
    return err;
}

err_t i2c_slave_listen(i2c_page_address_t* page_address, void* data, i2c_size_t data_size)
{
    err_t err = i2c_slave_setup_rw(page_address, data, data_size);
    
    if(err == E_NO_ERROR){
        _i2c_state.listening = true;
        i2c_do_listen();
    }
    
    return err;
}

void i2c_slave_end_listening(void)
{
    _i2c_state.listening = false;
    if(!i2c_is_busy()){
        i2c_do_listen();
    }
}

/*static uint8_t _twi_status = TW_NO_INFO;

uint8_t i2c_twi_status(void)
{
    return _twi_status;
}*/

/**
 * Прерывание аппаратуры I2C (TWI).
 * Содержит конечный автомат с реакцией на события.
 */
ISR(TWI_vect)
{
    //_twi_status = TW_STATUS;
    switch(TW_STATUS){
        //Нет информации.
        case TW_NO_INFO:
            //Не делаем ничего.
            break;
        //Ошибка шины.
        case TW_BUS_ERROR:
            //Установим ошибку.
            i2c_set_error(E_I2C_BUS_ERROR);
            //Установим состояние.
            i2c_set_status(I2C_STATUS_IDLE);
            //Закончим передачу.
            i2c_end(I2C_EVENT_ERROR);
            break;
        //Master
        //Transmitter
        //Мастер послал START на шину.
        case TW_START:
            //сбросим буферы.
            i2c_m_addr_buffer_reset();
            i2c_m_data_buffer_reset();
            //break нету, идём дальше!
        //Мастер послал на шину повторный старт.
        case TW_REP_START:
            //Если нужно передавать данные или адрес.
            if(_i2c_state.master.io_direction == I2C_MASTER_IO_DIRECTION_WRITE ||
                    i2c_m_addr_buffer_has_next()){
                //Установим флаг записи.
                TWDR = (_i2c_state.master.device << 1) | TW_WRITE;
            }else{//I2C_MASTER_IO_DIRECTION_READ
                //Иначе - чтения.
                TWDR = (_i2c_state.master.device << 1) | TW_READ;
            }
            //Следующий байт.
            i2c_mt_next_byte();
            break;
        //Мастер передал SLA+W.
        case TW_MT_SLA_ACK:
            //Установим статус, что передаём данные.
            i2c_set_status(I2C_STATUS_WRITING);
            //Если есть адрес.
            if(i2c_m_addr_buffer_has_next()){
                //Передадим очередной байт.
                TWDR = i2c_m_addr_buffer_get_next();
                //Следующий байт.
                i2c_mt_next_byte();
            //Иначе если есть данные.
            }else if(i2c_m_data_buffer_has_next()){
                //Передадим очередной байт.
                TWDR = i2c_m_data_buffer_get_next();
                //Следующий байт.
                i2c_mt_next_byte();
            //Иначе завершим передачу.
            }else{
                //Установим отсутствие ошибки.
                i2c_set_error(E_NO_ERROR);
                //Установим статус.
                i2c_set_status(I2C_STATUS_IDLE);
                //Остановим передачу.
                i2c_m_end(I2C_EVENT_MASTER_DATA_WRITED);
            }
            break;
        //Мастер получил NACK после передачи адреса - устройство недоступно.
        case TW_MT_SLA_NACK:
        case TW_MR_SLA_NACK:
            //Установим ошибку.
            i2c_set_error(E_I2C_DEVICE_NOT_RESPONDING);
            //Установим состояние.
            i2c_set_status(I2C_STATUS_IDLE);
            //Остановим передачу.
            i2c_m_end(I2C_EVENT_ERROR);
            break;
        //Мастер получил ACK после передачи байта.
        case TW_MT_DATA_ACK:
            //Если мы передаём адрес.
            if(_i2c_state.master.io_direction == I2C_MASTER_IO_DIRECTION_READ){
                //Если адрес передан не весь.
                if(i2c_m_addr_buffer_has_next()){
                    //Передадим очередной байт.
                    TWDR = i2c_m_addr_buffer_get_next();
                    //Следующий байт.
                    i2c_mt_next_byte();
                }else{
                    //Иначе сформируем повторный старт.
                    i2c_do_start();
                }
            //Иначе мы записываем данные и адрес.
            }else{//I2C_MASTER_IO_DIRECTION_WRITE
                //Если нужно передать адрес.
                if(i2c_m_addr_buffer_has_next()){
                    //Передадим очередной байт.
                    TWDR = i2c_m_addr_buffer_get_next();
                    //Следующий байт.
                    i2c_mt_next_byte();
                //Иначе если нужно передать данные.
                }else if(i2c_m_data_buffer_has_next()){
                    //Передадим очередной байт.
                    TWDR = i2c_m_data_buffer_get_next();
                    //Следующий байт.
                    i2c_mt_next_byte();
                }else{
                    //Иначе мы всё передали.
                    //Установим отсутствие ошибки.
                    i2c_set_error(E_NO_ERROR);
                    //И статус переданных данных.
                    i2c_set_status(I2C_STATUS_DATA_WRITED);
                    //И сформируем стоп.
                    i2c_m_end(I2C_EVENT_MASTER_DATA_WRITED);
                }
            }
            break;
        //Мастер получил NACK после передачи очередного байта.
        case TW_MT_DATA_NACK:
            //Если не передали весь адрес.
            if(i2c_m_addr_buffer_has_next()){
                //Естановим ошибку.
                i2c_set_error(E_I2C_DEVICE_DATA_NACK);
                //Установим статус.
                i2c_set_status(I2C_STATUS_IDLE);
            //Иначе если не передали все данные.
            }else if(i2c_m_data_buffer_has_next()){
                //Установим ошибку.
                i2c_set_error(E_I2C_DEVICE_DATA_NACK);
                //Установим статус.
                i2c_set_status(I2C_STATUS_DATA_WRITED);
            //Иначе - всё передано, ошибки нет.
            }else{
                //Установим отсутствие ошибки.
                i2c_set_error(E_NO_ERROR);
                //Установим статус.
                i2c_set_status(I2C_STATUS_DATA_WRITED);
            }
            //В любом случае больше ничего не передать - сформируем стоп.
            i2c_m_end(I2C_EVENT_MASTER_DATA_WRITED);
            break;
        //Мастер потерял шину, другой мастер обратился к слейву.
        case TW_MT_ARB_LOST:
        //case TW_MR_ARB_LOST:
            //Установим ошибку потери приоритета.
            i2c_set_error(E_I2C_BUS_ARBITRATION_LOST);
            //Сформируем старт как только освободиться шина.
            i2c_do_start();
            //i2c_do_listen();
            break;
        //Receiver
        //Мастер получил ACK на перданный SLA+R.
        case TW_MR_SLA_ACK:
            //Установим статус, что принимаем данные.
            i2c_set_status(I2C_STATUS_READING);
            //Если есть данные для приёма.
            if(i2c_m_data_buffer_has_next()){
                //Если нужно принять только один байт.
                if(i2c_m_data_buffer_at_last()){
                    //Примем его и ответим NACK.
                    i2c_next_byte(I2C_NACK);
                }else{
                    //Иначе примем и запросим ещё.
                    i2c_next_byte(I2C_ACK);
                }
            //Иначе завершим приём.
            }else{
                //Установим отсутствие ошибки.
                i2c_set_error(E_NO_ERROR);
                //Установим статус.
                i2c_set_status(I2C_STATUS_IDLE);
                //Остановим передачу.
                i2c_m_end(I2C_EVENT_MASTER_DATA_READED);
            }
            break;
        //Мастер принял байт и ответил ACK.
        case TW_MR_DATA_ACK:
            //Поместим принятый байт в буфер.
            i2c_m_data_buffer_set_next(TWDR);
            //Если осталось принять только один байт.
            if(i2c_m_data_buffer_at_last()){
                //Примем его и ответим NACK.
                i2c_next_byte(I2C_NACK);
            }else{
                //Иначе примем и запросим ещё.
                i2c_next_byte(I2C_ACK);
            }
            break;
        //Мастер принял байт данных и ответил NACK.
        case TW_MR_DATA_NACK:
            //Поместим принятый байт в буфер.
            i2c_m_data_buffer_set_next(TWDR);
            //Установим отсутствие ошибки.
            i2c_set_error(E_NO_ERROR);
            //Установим статус.
            i2c_set_status(I2C_STATUS_DATA_READED);
            //Сформируем стоп.
            i2c_m_end(I2C_EVENT_MASTER_DATA_READED);
            break;
        //Slave
        //Transmitter
        //Мастер потерял шину,
        //но ответил ACK на свой адрес в SLA+R,
        //став слейвом.
        case TW_ST_ARB_LOST_SLA_ACK:
            //Обозначим что нас прервали.
            _i2c_state.interrupted = true;
            //break нет - продолжаем.
        //Слейв ответил на свой адрес в SLA+R.
        case TW_ST_SLA_ACK:
            //Установим статус, что передаём данные.
            i2c_set_status(I2C_STATUS_WRITING);
            //Если есть что передавать.
            if(i2c_s_data_buffer_has_next()){
                //Возьмём следующий байт данных.
                TWDR = i2c_s_data_buffer_get_next();
                //Если больше нечего передавать.
                if(i2c_s_data_buffer_at_end()){
                    //Передадим байт и должны получить NACK.
                    i2c_next_byte(I2C_NACK);
                }else{
                    //Иначе передадим байт и должны получить ACK.
                    i2c_next_byte(I2C_ACK);
                }
            //Иначе в буфере нет больше байт.
            }else{
                //Установим ошибку выхода за пределы.
                i2c_set_error(E_OUT_OF_RANGE);
                //И пошлём ноль.
                TWDR = I2C_SLAVE_DATA_ERR_VALUE;
                //И должны получить NACK.
                i2c_next_byte(I2C_NACK);
            }
            break;
        //Слейв передал очередной байт и получил ACK.
        case TW_ST_DATA_ACK:
            //Если есть что передавать.
            if(i2c_s_data_buffer_has_next()){
                //Возьмём следующий байт данных.
                TWDR = i2c_s_data_buffer_get_next();
                //Если больше нечего передавать.
                if(i2c_s_data_buffer_at_end()){
                    //Передадим байт и должны получить NACK.
                    i2c_next_byte(I2C_NACK);
                }else{
                    //Иначе передадим байт и должны получить ACK.
                    i2c_next_byte(I2C_ACK);
                }
            //Иначе в буфере нет больше байт.
            }else{
                //Установим ошибку выхода за пределы.
                i2c_set_error(E_OUT_OF_RANGE);
                //И пошлём ноль.
                TWDR = I2C_SLAVE_DATA_ERR_VALUE;
                //И должны получить NACK.
                i2c_next_byte(I2C_NACK);
            }
            break;
        //Слейв отправил очередной байт и получил NACK.
        case TW_ST_DATA_NACK:
            //Передача окончена.
            //Установим отсутствие ошибки.
            i2c_set_error(E_NO_ERROR);
            //Установим статус.
            i2c_set_status(I2C_STATUS_DATA_WRITED);
            //Закончим передачу.
            i2c_s_end(I2C_EVENT_SLAVE_DATA_WRITED);
            break;
        //Слейв отправил последний байт и получил ACK.
        case TW_ST_LAST_DATA:
            //Больше байт у нас нет.
            //Установим ошибку переполнения буфера.
            i2c_set_error(E_OUT_OF_RANGE);
            //Установим статус.
            i2c_set_status(I2C_STATUS_DATA_WRITED);
            //Закончим передачу.
            i2c_s_end(I2C_EVENT_SLAVE_DATA_WRITED);
            break;
        //Receiver
        //Мастер потерял шину, ответив ACK
        //на собственный или широковещательный адрес
        //в SLA+W.
        case TW_SR_ARB_LOST_SLA_ACK:
        case TW_SR_ARB_LOST_GCALL_ACK:
            //Обозначим что нас прервали.
            _i2c_state.interrupted = true;
            //break нет - продолжаем.
        //Слейв ответил ACK
        //на собственный или широковещательный адрес
        //в SLA+W.
        case TW_SR_SLA_ACK:
        case TW_SR_GCALL_ACK:
            //Установим статус что читаем данные.
            i2c_set_status(I2C_STATUS_READING);
            
            //Сбросим буферы.
            i2c_s_addr_buffer_reset();
            i2c_s_data_buffer_reset();
            
            //Если в буферах более одного байта.
            if(i2c_s_buffers_has_more_one()){
                //Получим байт и ответим ACK.
                i2c_next_byte(I2C_ACK);
            }else{
                //Иначе получим байт и ответим NACK.
                i2c_next_byte(I2C_NACK);
            }
            break;
        //Получили байт и ответили ACK.
        case TW_SR_DATA_ACK:
        case TW_SR_GCALL_DATA_ACK:
            //Если адрес ещё не принят.
            if(i2c_s_addr_buffer_has_next()){
                //Поместим очередной байт.
                i2c_s_addr_buffer_set_next(TWDR);
                //Если в буферах есть более одного байта.
                if(i2c_s_buffers_has_more_one()){
                    //Примем очередной байт и ответим ACK.
                    i2c_next_byte(I2C_ACK);
                }else{
                    //Иначе примем очередной байт и ответим NACK.
                    i2c_next_byte(I2C_NACK);
                }
            //Иначе если данные ещё не приняты.
            }else if(i2c_s_data_buffer_has_next()){
                //Поместим очередной байт.
                i2c_s_data_buffer_set_next(TWDR);
                //Если в буфере данных есть более одного байта.
                if(i2c_s_data_buffer_at_last()){
                    //Примем очередной байт и ответим ACK.
                    i2c_next_byte(I2C_NACK);
                }else{
                    //Иначе примем очередной байт и ответим NACK.
                    i2c_next_byte(I2C_ACK);
                }
            //Иначе все данные получены.
            }else{
                //Установим ошибку выхода за границы.
                i2c_set_error(E_OUT_OF_RANGE);
                //И примем очередной байт, ответив NACK.
                i2c_next_byte(I2C_NACK);
            }
            break;
        //Получили байт и ответили NACK.
        case TW_SR_DATA_NACK:
        case TW_SR_GCALL_DATA_NACK:
            //Если адрес ещё не принят.
            if(i2c_s_addr_buffer_has_next()){
                //Поместим очередной байт.
                i2c_s_addr_buffer_set_next(TWDR);
                //Установим отсутствие ошибки.
                i2c_set_error(E_NO_ERROR);
                //Установим статус.
                i2c_set_status(I2C_STATUS_IDLE);
            //Если данные ещё не приняты.
            }else if(i2c_s_data_buffer_has_next()){
                //Поместим очередной байт.
                i2c_s_data_buffer_set_next(TWDR);
                //Установим отсутствие ошибки.
                i2c_set_error(E_NO_ERROR);
                //Установим статус.
                i2c_set_status(I2C_STATUS_DATA_READED);
            }
            //Закончим передачу.
            i2c_s_end(I2C_EVENT_SLAVE_DATA_READED);
            break;
        //Слейв получил стоп или повторный старт.
        case TW_SR_STOP:
            //Продолжим слушать шину.
            i2c_do_listen();
            break;
        default:
            break;
    }
}
