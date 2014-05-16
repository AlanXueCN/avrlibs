#include "spi.h"
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "ports/ports.h"
#include "bits/bits.h"
#include "buffer/buffer.h"
#include "defs/defs.h"

//! Максимальные значения.
//! Максимальное значение делителя тактирования SPI.
#define SPI_CLOCK_RATE_MAX SPI_CLOCK_RATE_2X_64
//! Максимальное значение режима SPI.
#define SPI_MODE_MAX SPI_MODE_MASTER
//! Максимальное значение порядка бит.
#define SPI_DATA_ORDER_MAX SPI_DATA_ORDER_LSB_FIRST
//! Максимальное значение полярности синхроимпульса.
#define SPI_CLOCK_POLARITY_MAX SPI_CLOCK_POLARITY_LOW
//! Максимальное значение фазы чтения/установки.
#define SPI_CLOCK_PHASE_MAX SPI_CPHA_LEADING_SETUP_TRAILING_SAMPLE
// //! Максимальное значение

//! Режимы передачи данных.
//! Одновременно передача и приём.
#define SPI_TRANSFER_MODE_READ_WRITE 0
//! Передача.
#define SPI_TRANSFER_MODE_WRITE 1
//! Приём.
#define SPI_TRANSFER_MODE_READ 2
//! Передача, а затем приём.
#define SPI_TRANSFER_MODE_WRITE_THEN_READ 3
//! Тип режима передачи.
typedef uint8_t spi_transfer_mode_t;

//! Сохраняет и запрещает прерывания чтения UART.
#define __spi_interrupts_save_disable()\
                register uint8_t __saved_spcr_spie = BIT_RAW_VALUE(SPCR, SPIE);\
                BIT_OFF(SPCR, SPIE)
//! Восстанавливает значение прерывания чтения UART.
#define __spi_interrupts_restore()\
                SPCR |= __saved_spcr_spie


typedef struct _SpiMasterData{
    buffer_t tx_buffer;
    buffer_t rx_buffer;
    spi_transfer_mode_t mode;
} spi_master_data_t;

//! Структура состояния SPI.
typedef struct _SpiState{
    pin_t mosi_pin;
    pin_t miso_pin;
    pin_t sck_pin;
    pin_t ss_pin;
    
    spi_mode_t mode;
    
    spi_state_t state;
    
    spi_transfer_id_t transfer_id;
    
    spi_master_callback_t master_callback;
    spi_slave_callback_t slave_callback;
    
    spi_master_data_t master;
} spi_t;


static spi_t spi;



ALWAYS_INLINE static bool spi_m_rx_has_next(void)
{
    return buffer_valid(&spi.master.rx_buffer) && buffer_has_next(&spi.master.rx_buffer);
}

static bool spi_m_rx_next(void)
{
    if(buffer_valid(&spi.master.rx_buffer) && spi_m_rx_has_next()){
        buffer_set_next(&spi.master.rx_buffer, SPDR);
        return true;
    }
    return false;
}

ALWAYS_INLINE static bool spi_m_tx_has_next(void)
{
    return buffer_valid(&spi.master.tx_buffer) && buffer_has_next(&spi.master.tx_buffer);
}

static bool spi_m_tx_next(void)
{
    if(buffer_valid(&spi.master.tx_buffer) && spi_m_tx_has_next()){
        SPDR = buffer_get_next(&spi.master.tx_buffer);
        return true;
    }
    return false;
}

ALWAYS_INLINE static void spi_end(spi_state_t state)
{
    spi.state = state;
    if(spi.master_callback) spi.master_callback();
}

/**
 * Настраивает пины SPI соответственно режиму.
 */
static void spi_setup_pins(void)
{
    if(BIT_TEST(SPCR, MSTR)){
        pin_set_out(&spi.mosi_pin);
        pin_set_out(&spi.sck_pin);
        pin_pullup_enable(&spi.ss_pin); pin_set_in(&spi.ss_pin);
    }else{
        pin_set_out(&spi.miso_pin);
    }
}

ISR(SPI_STC_vect)
{
    // Master.
    if(BIT_TEST(SPCR, MSTR)){
        switch(spi.master.mode){
            case SPI_TRANSFER_MODE_READ_WRITE:
                spi_m_rx_next();
                if(spi_m_rx_has_next()){
                    if(!spi_m_tx_next()){
                        SPDR = SPI_DATA_DEFAULT;
                    }
                }else{
                    if(!spi_m_tx_next())
                        spi_end(SPI_STATE_DATA_TRANSFERED);
                }
                break;
            case SPI_TRANSFER_MODE_WRITE:
                if(!spi_m_tx_next())
                    spi_end(SPI_STATE_DATA_WRITED);
                break;
            case SPI_TRANSFER_MODE_READ:
                spi_m_rx_next();
                if(spi_m_rx_has_next()){
                    SPDR = SPI_DATA_DEFAULT;
                }else{
                    spi_end(SPI_STATE_DATA_READED);
                }
                break;
            case SPI_TRANSFER_MODE_WRITE_THEN_READ:
                if(!spi_m_tx_next()){
                    if(spi.state == SPI_STATE_DATA_READING){
                        spi_m_rx_next();
                    }else{
                        spi.state = SPI_STATE_DATA_READING;
                    }
                    if(spi_m_rx_has_next()){
                        SPDR = SPI_DATA_DEFAULT;
                    }else{
                        spi_end(SPI_STATE_DATA_READED);
                    }
                }
                break;
        }
    //Slave.
    }else{
        // Если мы слейв.
        if(spi.mode != SPI_MODE_MASTER){
            spi.state = SPI_STATE_SLAVE_DATA_TRANSFERING;
            if(spi.slave_callback) SPDR = spi.slave_callback(SPDR);
            else SPDR = SPI_DATA_DEFAULT;
        // Иначе мы были мастером.
        }else{
            // Теперь мы слейв.
            spi.mode = SPI_MODE_SLAVE;
            // Перенастроим пины.
            spi_setup_pins();
            // Если была прервана передача.
            spi_end(spi_busy() ? SPI_STATE_INTERRUPTED : SPI_STATE_LOW_PRIORITY);
        }
    }
}

err_t spi_init(uint8_t spi_port, uint8_t mosi_pin, uint8_t miso_pin,
               uint8_t sck_pin, uint8_t ss_pin,
               spi_mode_t mode, spi_clock_rate_t clock_rate)
{
    if(clock_rate > SPI_CLOCK_RATE_MAX) return E_INVALID_VALUE;
    if(mode > SPI_MODE_MAX) return E_INVALID_VALUE;
    
    memset(&spi, 0x0, sizeof(spi_t));
    
    err_t err = pin_init(&spi.mosi_pin, spi_port, mosi_pin);
    if(err != E_NO_ERROR) return err;
    
    err = pin_init(&spi.miso_pin, spi_port, miso_pin);
    if(err != E_NO_ERROR) return err;
    
    err = pin_init(&spi.sck_pin, spi_port, sck_pin);
    if(err != E_NO_ERROR) return err;
    
    err = pin_init(&spi.ss_pin, spi_port, ss_pin);
    if(err != E_NO_ERROR) return err;
    
    if(BIT_TEST(clock_rate, 2)){
        BIT_ON(SPSR, SPI2X);
    }else{
        BIT_OFF(SPSR, SPI2X);
    }
    
    spi.mode = mode;
    
    SPCR = BIT(SPIE) |
           BIT(SPE) |
           BIT_BYVAL(MSTR, mode) |
           BIT_BYVAL(SPR1, (clock_rate >> 1) & 0x1) |
           BIT_BYVAL(SPR0, clock_rate & 0x1);
    
    spi_setup_pins();
    
    return E_NO_ERROR;
}

/*
spi_clock_rate_t spi_clock_rate(void)
{
    return (BIT_RAW_VALUE(SPSR, 0) << 2) | (SPCR & BIT_MAKE_MASK(2, 0));
}

err_t spi_set_clock_rate(spi_clock_rate_t clock_rate)
{
    if(clock_rate > SPI_CLOCK_RATE_MAX) return E_INVALID_VALUE;
    
    if(BIT_TEST(clock_rate, 2)){
        BIT_ON(SPSR, SPI2X);
    }else{
        BIT_OFF(SPSR, SPI2X);
    }
    
    BIT_SET(SPCR, SPR0, clock_rate & 0x1);
    BIT_SET(SPCR, SPR0, (clock_rate >> 1) & 0x1);
    
    return E_NO_ERROR;
}
*/

spi_mode_t spi_mode(void)
{
    return BIT_VALUE(SPCR, MSTR);
}

err_t spi_set_mode(spi_mode_t mode)
{
    if(mode > SPI_MODE_MAX) return E_INVALID_VALUE;
    
    spi.mode = mode;
    
    BIT_SET(SPCR, MSTR, mode);
    
    spi_setup_pins();
    
    return E_NO_ERROR;
}

spi_data_order_t spi_data_order(void)
{
    return BIT_VALUE(SPCR, DORD);
}

err_t spi_set_data_order(spi_data_order_t data_order)
{
    if(data_order > SPI_DATA_ORDER_MAX) return E_INVALID_VALUE;
    
    BIT_SET(SPCR, DORD, data_order);
    
    return E_NO_ERROR;
}

spi_clock_polarity_t spi_clock_polarity(void)
{
    return BIT_VALUE(SPCR, CPOL);
}

err_t spi_set_clock_polarity(spi_clock_polarity_t clock_polarity)
{
    if(clock_polarity > SPI_DATA_ORDER_MAX) return E_INVALID_VALUE;
    
    BIT_SET(SPCR, CPOL, clock_polarity);
    
    return E_NO_ERROR;
}

spi_clock_phase_t spi_clock_phase(void)
{
    return BIT_VALUE(SPCR, CPHA);
}

err_t spi_set_clock_phase(spi_clock_phase_t clock_phase)
{
    if(clock_phase > SPI_DATA_ORDER_MAX) return E_INVALID_VALUE;
    
    BIT_SET(SPCR, CPHA, clock_phase);
    
    return E_NO_ERROR;
}

spi_state_t spi_state(void)
{
    return spi.state;
}

bool spi_busy(void)
{
    switch(spi.state){
        case SPI_STATE_DATA_TRANSFERING:
        case SPI_STATE_DATA_WRITING:
        case SPI_STATE_DATA_READING:
            return true;
    }
    return pin_get_value(&spi.ss_pin) == 0;
}

spi_transfer_id_t spi_transfer_id(void)
{
    return spi.transfer_id;
}

void spi_set_transfer_id(spi_transfer_id_t id)
{
    spi.transfer_id = id;
}

spi_master_callback_t spi_master_callback(void)
{
    return spi.master_callback;
}

void spi_set_master_callback(spi_master_callback_t callback)
{
    __spi_interrupts_save_disable();
    spi.master_callback = callback;
    __spi_interrupts_restore();
}

spi_slave_callback_t spi_slave_callback(void)
{
    return spi.slave_callback;
}

void spi_set_slave_callback(spi_slave_callback_t callback)
{
    __spi_interrupts_save_disable();
    spi.slave_callback = callback;
    __spi_interrupts_restore();
}

err_t spi_transfer(const void* tx_data, void* rx_data, size_t size)
{
    if(spi_busy()) return E_BUSY;
    if(tx_data == NULL && rx_data == NULL) return E_NULL_POINTER;
    if(size == 0) return E_INVALID_VALUE;
    
    spi.state = SPI_STATE_DATA_TRANSFERING;
    
    spi.master.mode = SPI_TRANSFER_MODE_READ_WRITE;
    buffer_init(&spi.master.tx_buffer, (uint8_t*)tx_data, size);
    buffer_init(&spi.master.rx_buffer, (uint8_t*)rx_data, size);
    
    if(!spi_m_tx_next()){
        SPDR = SPI_DATA_DEFAULT;
    }
    
    if(BIT_TEST(SPSR, WCOL)){
        spi.state = SPI_STATE_IDLE;
        return E_BUSY;
    }
    
    return E_NO_ERROR;
}

err_t spi_write(const void* data, size_t size)
{
    if(spi_busy()) return E_BUSY;
    if(data == NULL) return E_NULL_POINTER;
    if(size == 0) return E_INVALID_VALUE;
    
    spi.state = SPI_STATE_DATA_WRITING;
    
    spi.master.mode = SPI_TRANSFER_MODE_WRITE;
    buffer_init(&spi.master.tx_buffer, (uint8_t*)data, size);
    
    if(!spi_m_tx_next()){
        spi.state = SPI_STATE_IDLE;
        return E_INVALID_VALUE;
    }
    
    if(BIT_TEST(SPSR, WCOL)){
        spi.state = SPI_STATE_IDLE;
        return E_BUSY;
    }
    
    return E_NO_ERROR;
}

err_t spi_read(void* data, size_t size)
{
    if(spi_busy()) return E_BUSY;
    if(data == NULL) return E_NULL_POINTER;
    if(size == 0) return E_INVALID_VALUE;
    
    spi.state = SPI_STATE_DATA_READING;
    
    spi.master.mode = SPI_TRANSFER_MODE_READ;
    buffer_init(&spi.master.rx_buffer, (uint8_t*)data, size);
    
    SPDR = SPI_DATA_DEFAULT;
    
    if(BIT_TEST(SPSR, WCOL)){
        spi.state = SPI_STATE_IDLE;
        return E_BUSY;
    }
    
    return E_NO_ERROR;
}

err_t spi_write_then_read(const void* tx_data, size_t tx_size, void* rx_data, size_t rx_size)
{
    if(spi_busy()) return E_BUSY;
    if(tx_data == NULL || rx_data == NULL) return E_NULL_POINTER;
    if(tx_size == 0 || rx_size == 0) return E_INVALID_VALUE;
    
    spi.state = SPI_STATE_DATA_WRITING;
    
    spi.master.mode = SPI_TRANSFER_MODE_WRITE_THEN_READ;
    buffer_init(&spi.master.tx_buffer, (uint8_t*)tx_data, tx_size);
    buffer_init(&spi.master.rx_buffer, (uint8_t*)rx_data, rx_size);
    
    if(!spi_m_tx_next()){
        spi.state = SPI_STATE_IDLE;
        return E_INVALID_VALUE;
    }
    
    if(BIT_TEST(SPSR, WCOL)){
        spi.state = SPI_STATE_IDLE;
        return E_BUSY;
    }
    
    return E_NO_ERROR;
}
