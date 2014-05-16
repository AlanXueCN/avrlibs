/**
 * @file uart.h
 * Библиотека для работы с UART.
 */

#ifndef UART_H
#define	UART_H

#include "errors/errors.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

//! Коды ошибок USART
#define E_UART  (E_USER + 50)
#define E_UART_INVALID_BAUD (E_UART + 1)

//! Режим контроля чётности.
//! Без контроля чётности.
#define UART_PARITY_MODE_NONE          0
//! Синоним предыдущего режима.
#define UART_PARITY_MODE_DISABLED      0
//! Флаг чётного числа.
#define UART_PARITY_MODE_EVEN          2
//! Флаг нечётного числа.
#define UART_PARITY_MODE_ODD           3
//! Тип режима контроля чётности.
typedef uint8_t uart_parity_mode_t;

//! Число стоп-битов.
//! Один стоп-бит.
#define UART_STOP_BITS_1       0
//! Два стоп-бита.
#define UART_STOP_BITS_2       1
//! Тип числа стоп-битов.
typedef uint8_t uart_stop_bits_t;

//! Тип каллбэка.
typedef void (*uart_callback_t)(void);

/**
 * Инициализирует UART.
 * @param hbaud Скорость передачи, гектабод.
 * @param parity Режим контроля чётности.
 * @param bits Число стоп-бит.
 * @return Код ошибки.
 */
extern err_t uart_init(uint16_t hbaud, uart_parity_mode_t parity, uart_stop_bits_t stop_bits);

/**
 * Устанавливает значение скорости передачи.
 * @param hbaud Скорость передачи, гектабод.
 * @return Код ошибки.
 */
extern err_t uart_set_baud(uint16_t hbaud);

/**
 * Устанавливает буфер для чтения UART.
 * @param ptr Указатель на буфер.
 * @param size Размер буфера.
 * @return Код ошибки.
 */
extern err_t uart_set_read_buffer(uint8_t* ptr, size_t size);

/**
 * Устанавливает буфер для записи UART.
 * @param ptr Указатель на буфер.
 * @param size Размер буфера.
 * @return Код ошибки.
 */
extern err_t uart_set_write_buffer(uint8_t* ptr, size_t size);

/**
 * Получает каллбэк получения данных.
 * @return Каллбэк получения данных.
 */
extern uart_callback_t uart_receive_callback(void);

/**
 * Устанавливает каллбэк получения данных.
 * @param callback Каллбэк получения данных.
 */
extern void uart_set_receive_callback(uart_callback_t callback);

/**
 * Получает флаг разрешения передатчика UART.
 * @return Флаг разрешения передатчика UART.
 */
extern bool uart_transmitter_enabled(void);

/**
 * Разрешает или запрещает передатчик UART.
 * @param enabled Флаг разрешения передатчика UART.
 */
extern void uart_transmitter_set_enabled(bool enabled);

/**
 * Получает флаг разрешения приёмника UART.
 * @return Флаг разрешения приёмника UART.
 */
extern bool uart_receiver_enabled(void);

/**
 * Разрешает или запрещает приёмника UART.
 * @param enabled Флаг разрешения приёмника UART.
 */
extern void uart_receiver_set_enabled(bool enabled);

/**
 * Получает флаг ошибки чётности.
 * @return Флаг ошибки чётности.
 */
extern bool uart_parity_error(void);

/**
 * Получает флаг ошибки переполнения.
 * @return Флаг ошибки переполнения.
 */
extern bool uart_data_overrun_error(void);

/**
 * Получает флаг ошибки кадра.
 * @return Флаг ошибки кадра.
 */
extern bool uart_frame_error(void);

/**
 * Ждёт окончания передачи данных по UART.
 */
extern void uart_flush(void);

/**
 * Получает количество принятых и не считанных байт данных.
 * @return Количество принятых и не считанных байт данных.
 */
extern size_t uart_data_avail(void);

/**
 * Копирует байт данных для передачи по UART.
 * При необходимости ждёт освобождения буфера.
 * @param data Байт данных.
 * @return Размер скопированных данных.
 */
extern size_t uart_put(uint8_t data);

/**
 * Получает асинхронно принятый по UART байт данных.
 * @param data Байт данных.
 * @return Размер скопированных данных.
 */
extern size_t uart_get(uint8_t* data);

/**
 * Копирует данные в буфер для асинхронной передачи по UART.
 * Если буфера нехватает, ждёт освобождения.
 * @param data Данные.
 * @param size Размер данных.
 * @return Размер скопированных для передачи байт.
 */
extern size_t uart_write(const void* data, size_t size);

/**
 * Получает асинхронно принятые по UART данные.
 * @param data Буфер для данных.
 * @param size Размер буфера.
 * @return Размер скопированных в буфер данных.
 */
extern size_t uart_read(void* data, size_t size);


#ifdef UART_STDIO

#include <stdio.h>

/**
 * Записывает символ в UART.
 * @param c Символ.
 * @param stream Поток ввода/вывода, не используется.
 * @return Записываемый символ или EOF, в случае ошибки.
 */
static int uart_putc(int c, FILE* stream)
{
    uint8_t data = (uint8_t)c;
    if(data == '\n'){
        if(uart_put('\r') == 0) return EOF;
    }
    if(uart_put(data) == 0) return EOF;
    return c;
}

/**
 * Считывает символ из UART.
 * @param stream Поток ввода/вывода, не используется.
 * @return Считанный символ, или EOF, если нечего считывать.
 */
static int uart_getc(FILE* stream)
{
    uint8_t data;
    if(uart_get(&data)){
        return (int)data;
    }
    return EOF;
}

static FILE uart_stdio = FDEV_SETUP_STREAM(uart_putc, uart_getc, _FDEV_SETUP_RW);

/**
 * Устанавливает потоки чтения/зиписи на UART.
 */
static void uart_setup_stdio(void)
{
    stdout = &uart_stdio;
    stdin = &uart_stdio;
}

#endif

#endif	/* UART_H */

