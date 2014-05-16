#include "uart.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>
#include "bits/bits.h"
#include "buffer/circular_buffer.h"
#include "utils/utils.h"


#ifndef F_CPU
#warning uart: F_CPU is not defined. Defaulting to 1MHz.
#define F_CPU 1000000UL
#endif

// Предварительно вычисленные значения частоты.
#define F_CPU_HHZ (F_CPU / 100)
#define F_CPU_HHZ_DIV16 (F_CPU_HHZ / 16)
#define F_CPU_HHZ_DIV8 (F_CPU_HHZ / 8)

//! Максимально возвожное значение в регистрах UBRRH:UBRRL.
#define UART_UBRR_MAX 4095
//! Порог значения UBRR, после которого используется U2X.
#define UART_2X_UBRR_MIN 8

//! Зарезервированные/максимальные значения.
//! Зарезервированное значение режима контроля чётности.
#define UART_PARITY_MODE_RESERVED 1
//! Максимальное значение режима контроля чётности.
#define UART_PARITY_MODE_MAX      UART_PARITY_MODE_ODD
//! Максимальное значение числа стоп-бит.
#define UART_STOP_BITS_MAX        UART_STOP_BITS_2

#ifdef UART_DISABLE_ALL_INTERRUPTS
#define __uart_rx_interrupts_save_disable() __interrupts_save_disable()
#define __uart_rx_interrupts_restore() __interrupts_restore()
#define __uart_tx_interrupts_save_disable() __interrupts_save_disable()
#define __uart_tx_interrupts_restore() __interrupts_restore()
#else
//! Сохраняет и запрещает прерывания чтения UART.
#define __uart_rx_interrupts_save_disable()\
                register uint8_t __saved_rxcie_ucsrb = BIT_RAW_VALUE(UCSRB, RXCIE);\
                BIT_OFF(UCSRB, RXCIE)
//! Восстанавливает значение прерывания чтения UART.
#define __uart_rx_interrupts_restore()\
                UCSRB |= __saved_rxcie_ucsrb
//! Сохраняет и запрещает прерывания записи UART.
#define __uart_tx_interrupts_save_disable()\
                register uint8_t __saved_udrie_ucsrb = BIT_RAW_VALUE(UCSRB, UDRIE);\
                BIT_OFF(UCSRB, UDRIE)
//! Восстанавливает значение прерывания записи UART.
#define __uart_tx_interrupts_restore()\
                UCSRB |= __saved_udrie_ucsrb
#endif

//! Структура состояния UART.
typedef struct _UsartState{
    circular_buffer_t write_buffer;
    circular_buffer_t read_buffer;
    bool data_overrun;
    uart_callback_t on_receive_callback;
}uart_state_t;

//! Состояние UART.
static uart_state_t state;


ISR(USART_UDRE_vect)
{
    uint8_t data;
    
    if(circular_buffer_get(&state.write_buffer, &data)){
        UDR = data;
    }else{
        BIT_OFF(UCSRB, UDRIE);
    }
}

ISR(USART_RXC_vect)
{
    uint8_t data;
    
    data = UDR;
    
    if(!circular_buffer_put(&state.read_buffer, data)){
        state.data_overrun = true;
    }
    if(state.on_receive_callback) state.on_receive_callback();
}

ISR(USART_TXC_vect)
{
}

err_t uart_init(uint16_t hbaud, uart_parity_mode_t parity, uart_stop_bits_t stop_bits)
{
    if(parity == UART_PARITY_MODE_RESERVED || parity > UART_PARITY_MODE_MAX){
        return E_INVALID_VALUE;
    }
    if(stop_bits > UART_STOP_BITS_MAX) return E_INVALID_VALUE;
    
    UCSRA = 0; // U2X = 0; MPCM = 0;
    //Запрет прерываний и приёмника/передатчика,
    //Установка 2го бита размера символа в 0.
    UCSRB = 0;
    
    err_t err = uart_set_baud(hbaud);
    if(err != E_NO_ERROR) return err;
    
    //Асинхронный режим, 8 бит на символ.
    UCSRC = (1 << URSEL) //Выбор регистра UCSRC
            | (((parity >> 1) & 0x1) << UPM1) //Контроль чётности, 1й бит.
            | ((parity & 0x1) << UPM0) //Контроль чётности, 0й бит.
            | ((stop_bits & 0x1) << USBS) //Число стоп-бит.
            | (1 << UCSZ1) | (1 << UCSZ0); // 8 бит на символ.
    
    memset(&state, 0x0, sizeof(uart_state_t));
    
    return E_NO_ERROR;
}

err_t uart_set_baud(uint16_t hbaud)
{
    bool u2x = false;
    uint16_t ubrr = F_CPU_HHZ_DIV16 / hbaud;
    
    if(ubrr <= UART_2X_UBRR_MIN){
        u2x = true;
        ubrr = F_CPU_HHZ_DIV8 / hbaud;
    }
    
    if(ubrr > UART_UBRR_MAX || ubrr == 0){
        return E_UART_INVALID_BAUD;
    }
    
    ubrr --;
    
    UBRRH = ubrr >> 8;
    UBRRL = ubrr & 0xff;
    
    if(u2x) BIT_ON(UCSRA, U2X);
    
    return E_NO_ERROR;
}

err_t uart_set_read_buffer(uint8_t* ptr, size_t size)
{
    if(ptr == NULL) return E_NULL_POINTER;
    if(size == 0) return E_INVALID_VALUE;
    
    circular_buffer_init(&state.read_buffer, ptr, size);
    
    return E_NO_ERROR;
}

err_t uart_set_write_buffer(uint8_t* ptr, size_t size)
{
    if(ptr == NULL) return E_NULL_POINTER;
    if(size == 0) return E_INVALID_VALUE;
    
    circular_buffer_init(&state.write_buffer, ptr, size);
    
    return E_NO_ERROR;
}

uart_callback_t uart_receive_callback(void)
{
    return state.on_receive_callback;
}

void uart_set_receive_callback(uart_callback_t callback)
{
    __uart_rx_interrupts_save_disable();
    
    state.on_receive_callback = callback;
    
    __uart_rx_interrupts_restore();
}

bool uart_transmitter_enabled(void)
{
    return BIT_VALUE(UCSRB, TXEN);
}

void uart_transmitter_set_enabled(bool enabled)
{
    //BIT_SET(UCSRB, TXCIE, enabled);
    BIT_SET(UCSRB, TXEN, enabled);
}

bool uart_receiver_enabled(void)
{
    return BIT_VALUE(UCSRB, RXEN);
}

void uart_receiver_set_enabled(bool enabled)
{
    BIT_SET(UCSRB, RXCIE, enabled);
    BIT_SET(UCSRB, RXEN, enabled);
}

bool uart_parity_error(void)
{
    return BIT_VALUE(UCSRA, PE);
}

bool uart_data_overrun_error(void)
{
    return BIT_VALUE(UCSRA, DOR) || state.data_overrun;
}

bool uart_frame_error(void)
{
    return BIT_VALUE(UCSRA, FE);
}

void uart_flush(void)
{
    while(circular_buffer_avail_size(&state.write_buffer) != 0);
}

size_t uart_data_avail(void)
{
    return circular_buffer_avail_size(&state.read_buffer);
}

size_t uart_put(uint8_t data)
{
    size_t res;
    
    if(!circular_buffer_valid(&state.write_buffer) ||
       !uart_transmitter_enabled()) return 0;
    
    while(circular_buffer_free_size(&state.write_buffer) == 0);
    
    __uart_tx_interrupts_save_disable();
    
    res = circular_buffer_put(&state.write_buffer, data);
    
    __uart_tx_interrupts_restore();
    
    if(res != 0){
        BIT_ON(UCSRB, UDRIE);
    }
    
    return res;
}

size_t uart_get(uint8_t* data)
{
    size_t res;
    
    if(!circular_buffer_valid(&state.read_buffer) ||
       !uart_receiver_enabled()) return 0;
    
    __uart_rx_interrupts_save_disable();
    
    res = circular_buffer_get(&state.read_buffer, data);
    
    __uart_rx_interrupts_restore();
    
    return res;
}

size_t uart_write(const void* data, size_t size)
{
    if(size == 0 || data == NULL) return 0;
    if(!circular_buffer_valid(&state.write_buffer) ||
       !uart_transmitter_enabled()) return 0;
    
    size_t res_size = 0;
    size_t n;
    
    do{

        do{
            n = MIN(circular_buffer_free_size(&state.write_buffer), size);
        }while(n == 0);

        __uart_tx_interrupts_save_disable();

        n = circular_buffer_write(&state.write_buffer, data, n);

        __uart_tx_interrupts_restore();

        if(n != 0){
            BIT_ON(UCSRB, UDRIE);
        }else{
            break;
        }
        
        res_size += n;
        data += n;
        size -= n;
    
    }while(size > 0);
    
    return res_size;
}

size_t uart_read(void* data, size_t size)
{
    if(size == 0 || data == NULL) return 0;
    if(!circular_buffer_valid(&state.read_buffer) ||
       !uart_receiver_enabled()) return 0;
    
    size_t res_size = 0;
    
    __uart_rx_interrupts_save_disable();
    
    res_size = circular_buffer_read(&state.read_buffer, data, size);
    
    __uart_rx_interrupts_restore();
    
    if(res_size != 0){
        state.data_overrun = false;
    }
    
    return res_size;
}
