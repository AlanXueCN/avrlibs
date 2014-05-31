#include "i2c.h"
#include "buffer/buffer.h"
#include "bits/bits.h"
#include "utils/utils.h"
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

//! Сохраняет и запрещает прерывания чтения UART.
#define __i2c_interrupts_save_disable()\
                register uint8_t __saved_twcr_twie = BIT_RAW_VALUE(TWCR, TWIE);\
                BIT_OFF(TWCR, TWIE)
//! Восстанавливает значение прерывания чтения UART.
#define __i2c_interrupts_restore()\
                TWCR |= __saved_twcr_twie

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
 * Состояние шины i2c.
 */
typedef struct _I2C_State{
    //! Каллбэк.
    i2c_callback_t callback;
    
    //! Каллбэк передачи/приёма ведомым.
    i2c_slave_callback_t slave_callback;
    
    //Идентификатор передачи.
    i2c_transfer_id_t transfer_id;
    
    //! Статус.
    i2c_status_t status;
    
    //! Флаг реагирования на свой адрес.
    bool listening;
    
    //! Флаг прерванной передачи/приёма потерей приоритета.
    bool interrupted;
    
    //! Флаг наличия передачи.
    bool has_transfer;
    
    //! Данные для передачи/приёма ведущим.
    i2c_master_data_t master;
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
 * Устанавливает статус шины i2c.
 * @param status Статус.
 * @return Статус.
 */
ALWAYS_INLINE static void i2c_set_status(i2c_status_t status)
{
    _i2c_state.status = status;
}

err_t i2c_init(uint16_t freq)
{
    err_t err = i2c_set_freq(freq);
    if(err != E_NO_ERROR) return err;
    
    memset(&_i2c_state, 0x0, sizeof(i2c_state_t));
    
    i2c_do_listen();
    
    return E_NO_ERROR;
}

err_t i2c_set_freq(uint16_t freq)
{
    if(freq > F_CPU_DIV16_KHZ) return E_I2C_INVALID_FREQ;
    
    uint16_t twbr = ((uint16_t)F_CPU_KHZ / freq - 16) / 2;
    uint8_t twps = 0;
    uint8_t ps_val = 1;
    
    while(twbr > 255){
        twps ++;
        ps_val *= 4;
        twbr /= ps_val;
    }
    
    if(twps > 0x3 /* 0b11 */) return E_I2C_INVALID_FREQ;
    
    TWBR = (uint8_t)twbr;
    TWSR = twps;
    
    return E_NO_ERROR;
}

void i2c_set_address(i2c_address_t address, bool bcast_enabled)
{
    TWAR = (address << TWA0) | ((uint8_t)bcast_enabled << TWGCE);
}

i2c_status_t i2c_status(void)
{
    return _i2c_state.status;
}

bool i2c_interrupted(void)
{
    return _i2c_state.interrupted;
}

bool i2c_busy(void)
{
    switch(_i2c_state.status){
        case I2C_STATUS_READING:
        case I2C_STATUS_WRITING:
            return true;
        default:
            break;
    }
    return _i2c_state.has_transfer;
}

void i2c_wait(void)
{
    WAIT_WHILE_TRUE(i2c_busy());
}

i2c_callback_t i2c_callback(void)
{
    return _i2c_state.callback;
}

void i2c_set_callback(i2c_callback_t callback)
{
    _i2c_state.callback = callback;
}

i2c_slave_callback_t i2c_slave_callback(void)
{
    return _i2c_state.slave_callback;
}

void i2c_set_slave_callback(i2c_slave_callback_t callback)
{
    _i2c_state.slave_callback = callback;
}

i2c_transfer_id_t i2c_transfer_id(void)
{
    return _i2c_state.transfer_id;
}

void i2c_set_transfer_id(i2c_transfer_id_t id)
{
    _i2c_state.transfer_id = id;
}

i2c_size_t i2c_master_bytes_transmitted(void)
{
    return i2c_data_bytes_transmitted(&_i2c_state.master.data);
}

/*
 * Алиасы. 
 */

ALWAYS_INLINE static void i2c_m_addr_buffer_reset(void)
{
    buffer_reset(&_i2c_state.master.data.address_buffer);
}

ALWAYS_INLINE static void i2c_m_data_buffer_reset(void)
{
    buffer_reset(&_i2c_state.master.data.data_buffer);
}

ALWAYS_INLINE static void i2c_m_buffers_reset(void)
{
    i2c_m_addr_buffer_reset();
    i2c_m_data_buffer_reset();
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

ALWAYS_INLINE static void i2c_next_byte(uint8_t ack)
{
    i2c_do_rw_byte(ack);
}

ALWAYS_INLINE static void i2c_mt_next_byte(void)
{
    i2c_next_byte(_i2c_state.listening);
}

/**
 * Посредник, вызывает каллбэк ведомого.
 * @param data Полученные данные, или NULL.
 * @return флаг возможности ещё принимать данные.
 */
ALWAYS_INLINE static bool i2c_on_slave_read(uint8_t* data)
{
    if(_i2c_state.slave_callback) return _i2c_state.slave_callback(I2C_READ, data);
    return false;
}

/**
 * Посредник, вызывает каллбэк ведомого.
 * @param data Данные для передачи.
 * @return флаг возможности ещё передавать данные.
 */
ALWAYS_INLINE static bool i2c_on_slave_write(uint8_t* data)
{
    if(_i2c_state.slave_callback) return _i2c_state.slave_callback(I2C_WRITE, data);
    return false;
}

/**
 * Посредник, вызывает каллбэк, если он не NULL.
 */
ALWAYS_INLINE static void i2c_end(void)
{
    // Передача окончина.
    _i2c_state.has_transfer = _i2c_state.interrupted;
    if(_i2c_state.callback) _i2c_state.callback();
}

/**
 * Обработчик окончания приёма/передачи ведущим.
 */
static void i2c_m_end(void)
{
    // Обозначим конец передачи.
    i2c_end();
    
    // Если не была инициирована ещё одна передача.
    if(!_i2c_state.has_transfer){
        // Пошлём стоп.
        i2c_do_stop();
    }
}

/**
 * Обработчик окончания приёма/передачи ведомым.
 */
static void i2c_s_end(void)
{
    // Обозначим конец передачи/приёма.
    i2c_end();
    
    // Если передача прервана.
    if(_i2c_state.interrupted){
        _i2c_state.interrupted = false;
        // Запустим новую.
        i2c_do_start();
    // Иначе.
    }else{
        // Если не была инициирована ещё одна передача.
        if(!_i2c_state.has_transfer){
            //Будем слушать дальше.
            i2c_do_listen();
        }
    }
}

/**
 * Настраивает шину i2c в режим мастер.
 * @param device адрес устройства.
 * @param page_address адрес в устройстве.
 * @param data данные.
 * @param data_size размер данных.
 * @return Код ошибки.
 */
err_t i2c_master_setup_rw(i2c_address_t device, void* page_address, size_t page_address_size, void* data, i2c_size_t data_size)
{
    if(i2c_is_busy()) return E_BUSY;
    if(data == NULL) return E_NULL_POINTER;
    if(data_size == 0) return E_INVALID_VALUE;
    
    if(page_address == NULL){
        page_address_size = 0;
    }else{
        if(page_address_size == 0) return E_INVALID_VALUE;
    }
    
    i2c_data_init(&_i2c_state.master.data, page_address, page_address_size, data, data_size);
    
    _i2c_state.master.device = device;
    
    _i2c_state.has_transfer = true;
    
    return E_NO_ERROR;
}

err_t i2c_master_read(i2c_address_t device, void* data, i2c_size_t data_size)
{
    return i2c_master_read_at(device, NULL, 0, data, data_size);
}

err_t i2c_master_read_at(i2c_address_t device, const void* page_address, size_t page_address_size, void* data, i2c_size_t data_size)
{
    err_t err = i2c_master_setup_rw(device, (void*)page_address, page_address_size, data, data_size);
    
    if(err == E_NO_ERROR){
        _i2c_state.master.io_direction = I2C_MASTER_IO_DIRECTION_READ;
        i2c_do_start();
    }
    
    return err;
}

err_t i2c_master_write(i2c_address_t device, const void* data, i2c_size_t data_size)
{
    return i2c_master_write_at(device, NULL, 0, data, data_size);
}

err_t i2c_master_write_at(i2c_address_t device, const void* page_address, size_t page_address_size, const void* data, i2c_size_t data_size)
{
    err_t err = i2c_master_setup_rw(device, (void*)page_address, page_address_size, (void*)data, data_size);
    
    if(err == E_NO_ERROR){
        _i2c_state.master.io_direction = I2C_MASTER_IO_DIRECTION_WRITE;
        i2c_do_start();
    }
    
    return err;
}

void i2c_slave_listen(void)
{
    _i2c_state.listening = true;
    if(!i2c_is_busy()){
        i2c_do_listen();
    }
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
}/**/

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
            i2c_set_status(I2C_STATUS_IDLE);
            //Закончим передачу.
            i2c_end();
            break;
        //Ошибка шины.
        case TW_BUS_ERROR:
            //Установим состояние.
            i2c_set_status(I2C_STATUS_BUS_ERROR);
            //Закончим передачу.
            i2c_end();
            break;
        //Master
        //Transmitter
        //Мастер послал START на шину.
        case TW_START:
            //сбросим буферы.
            i2c_m_buffers_reset();
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
                //Установим статус.
                i2c_set_status(I2C_STATUS_DATA_WRITED);
                //Остановим передачу.
                i2c_m_end();
            }
            break;
        //Мастер получил NACK после передачи адреса - устройство недоступно.
        case TW_MT_SLA_NACK:
        case TW_MR_SLA_NACK:
            //Установим состояние.
            i2c_set_status(I2C_STATUS_NOT_RESPONDING);
            //Остановим передачу.
            i2c_m_end();
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
                    //И статус переданных данных.
                    i2c_set_status(I2C_STATUS_DATA_WRITED);
                    //И сформируем стоп.
                    i2c_m_end();
                }
            }
            break;
        //Мастер получил NACK после передачи очередного байта.
        case TW_MT_DATA_NACK:
            //Если не передали все данные.
            if(i2c_m_data_buffer_has_next()){
                //Установим статус.
                i2c_set_status(I2C_STATUS_REJECTED);
            //Иначе - всё передано, ошибки нет.
            }else{
                //Установим статус.
                i2c_set_status(I2C_STATUS_DATA_WRITED);
            }
            //В любом случае больше ничего не передать - сформируем стоп.
            i2c_m_end();
            break;
        //Мастер потерял шину, другой мастер обратился к слейву.
        case TW_MT_ARB_LOST:
        //case TW_MR_ARB_LOST:
            //Установим ошибку потери приоритета.
            i2c_set_status(I2C_STATUS_ARBITRATION_LOST);
            //Установим флаг прерванной передачи.
            _i2c_state.interrupted = true;
            //сбросим буферы.
            i2c_m_buffers_reset();
            //Обозначим конец передачи.
            i2c_end();
            //Сформируем старт как только освободится шина.
            i2c_do_start();
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
                //Установим статус.
                i2c_set_status(I2C_STATUS_DATA_READED);
                //Остановим передачу.
                i2c_m_end();
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
            //Установим статус.
            i2c_set_status(I2C_STATUS_DATA_READED);
            //Сформируем стоп.
            i2c_m_end();
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
            i2c_set_status(I2C_STATUS_SLAVE_WRITING);
            //Обозначим начало передачи.
            i2c_on_slave_write(NULL);
            {
                //Байт данных.
                uint8_t data = I2C_DATA_DEFAULT_VALUE;
                //Получим байт и доступность данных.
                bool has_more = i2c_on_slave_write(&data);
                //Запишем данные.
                TWDR = data;
                //Передадим.
                i2c_next_byte(has_more ? I2C_ACK : I2C_NACK);
            }
            break;
        //Слейв передал очередной байт и получил ACK.
        case TW_ST_DATA_ACK:
            {
                //Байт данных.
                uint8_t data = I2C_DATA_DEFAULT_VALUE;
                //Получим байт и доступность данных.
                bool has_more = i2c_on_slave_write(&data);
                //Запишем данные.
                TWDR = data;
                //Передадим.
                i2c_next_byte(has_more ? I2C_ACK : I2C_NACK);
            }
            break;
        //Слейв отправил очередной байт и получил NACK.
        case TW_ST_DATA_NACK:
        //Слейв отправил последний байт и получил ACK.
        case TW_ST_LAST_DATA:
            //Передача окончена.
            //Или больше байт у нас нет.
            //Установим статус.
            i2c_set_status(I2C_STATUS_SLAVE_DATA_WRITED);
            //Закончим передачу.
            i2c_s_end();
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
            i2c_set_status(I2C_STATUS_SLAVE_READING);
            {
                //Получим доступность более одного байта данных.
                bool has_more = i2c_on_slave_read(NULL);
                //Следующий байт.
                i2c_next_byte(has_more ? I2C_ACK : I2C_NACK);
            }
            break;
        //Получили байт и ответили ACK.
        case TW_SR_DATA_ACK:
        case TW_SR_GCALL_DATA_ACK:
            {
                //Байт данных.
                uint8_t data = TWDR;
                //Получим байт и доступность данных.
                bool has_more = i2c_on_slave_read(&data);
                //Следующий байт.
                i2c_next_byte(has_more ? I2C_ACK : I2C_NACK);
            }
            break;
        //Получили байт и ответили NACK.
        case TW_SR_DATA_NACK:
        case TW_SR_GCALL_DATA_NACK:
            {
                //Байт данных.
                uint8_t data = TWDR;
                //Получим байт.
                i2c_on_slave_read(&data);
                //Установим статус.
                i2c_set_status(I2C_STATUS_SLAVE_DATA_READED);
                //Закончим передачу.
                i2c_s_end();
            }
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
