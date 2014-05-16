/**
 * @file ports.h
 * Библиотека для работы с портами ввода-вывода.
 */

#ifndef _PORTS_H_
#define _PORTS_H_

#include <stdint.h>
#include <avr/io.h>
#include "errors/errors.h"
#include "defs/defs.h"
#include "bits/bits.h"

/*
 * Номера портов.
 */
#ifdef PORTA
#define PORT_A 0
#endif

#ifdef PORTB
#define PORT_B 1
#endif

#ifdef PORTC
#define PORT_C 2
#endif

#ifdef PORTD
#define PORT_D 3
#endif

#ifdef PORTE
#define PORT_E 4
#endif

#ifdef PORTF
#define PORT_F 5
#endif

/*
 * Коды ошибок.
 */
#define E_PORTS                         E_USER
#define E_PORTS_INVALID_PORT_NUMBER     (E_PORTS + 1)
#define E_PORTS_INVALID_PORT_OUT        (E_PORTS + 2)
#define E_PORTS_INVALID_PORT_DDR        (E_PORTS + 3)
#define E_PORTS_INVALID_PORT_IN         (E_PORTS + 4)

#define E_PORTS_INVALID_PIN_NUMBER      (E_PORTS + 5)
#define E_PORTS_INVALID_PINS_COUNT      (E_PORTS + 6)
#define E_PORTS_INVALID_PINS_OFFSET     (E_PORTS + 7)

/**
 * Структура порта.
 */
typedef struct _Port {
    //! Адрес регистра вывода.
    volatile uint8_t* out;
    //! Адрес регистра ввода.
    volatile uint8_t* in;
    //! Адрес регистра конфигурации.
    volatile uint8_t* ddr;
} port_t;

/**
 * Структура пина порта.
 */
typedef struct _Pin {
    //! Порт.
    port_t port;
    //! Номер пина.
    uint8_t pin_n;
    //! Маска.
    uint8_t _port_mask;
} pin_t;

/**
 * Структура диапазона
 * подряд идущих
 * пинов порта.
 */
typedef struct _Pin_range {
    //! Порт.
    port_t port;
    //! Число пинов.
    uint8_t count;
    //! Смещение относительно нулевого пина порта.
    uint8_t offset;
    //! Маска порта.
    uint8_t _port_mask;
}pin_range_t;

/**
 * Возвращает адрес регистра вывода для заданного порта
 * @param port_n Номер порта.
 * @return Адрес регистра вывода.
 */
extern volatile uint8_t* port_get_out(uint8_t port_n);

/**
 * Возвращает адрес регистра ввода для заданного порта
 * @param port_n Номер порта.
 * @return Адрес регистра ввода.
 */
extern volatile uint8_t* port_get_in(uint8_t port_n);

/**
 * Возвращает адрес регистра конфигурации для заданного порта
 * @param port_n Номер порта.
 * @return Адрес регистра конфигурации.
 */
extern volatile uint8_t* port_get_ddr(uint8_t port_n);

/**
 * Получает значение регистров для заданного порта.
 * @param port Порт.
 * @param port_n Номер порта.
 * @return Код ошибки.
 */
extern err_t port_init(port_t* port, uint8_t port_n);

/**
 * Устанавливает режим вывода для порта.
 * @param port Порт.
 */
ALWAYS_INLINE static void port_set_out(port_t* port)
{
    *port->ddr = 0xff;
}

/**
 * Устанавливает режим ввода для порта.
 * @param port Порт.
 */
ALWAYS_INLINE static void port_set_in(port_t* port)
{
    *port->ddr = 0x0;
}

/**
 * Разрешает включение подтяжки.
 */
ALWAYS_INLINE static void port_pullup_enable_all(void)
{
    BIT_OFF(SFIOR, PUD);
}

/**
 * Запрещает включение подтяжки.
 */
ALWAYS_INLINE static void port_pullup_disable_all(void)
{
    BIT_ON(SFIOR, PUD);
}

/**
 * Разрешает подтяжку на порту.
 * @param port Порт.
 */
ALWAYS_INLINE static void port_pullup_enable(port_t* port)
{
    *port->out = 0xff;
}

/**
 * Запрещает подтяжку на порту.
 * @param port Порт.
 */
ALWAYS_INLINE static void port_pullup_disable(port_t* port)
{
    *port->out = 0x0;
}

/**
 * Устанавливает значение на порту.
 * @param port Порт.
 * @param value Значение.
 */
ALWAYS_INLINE static void port_set_value(port_t* port, uint8_t value)
{
    *port->out = value;
}

/**
 * Получение значения на порту.
 * @param port Порт.
 * @return Значение на порту.
 */
ALWAYS_INLINE static uint8_t port_get_value(port_t* port)
{
    return *port->in;
}

/**
 * Получает значения регистров для заданного пина порта.
 * @param pin Пин порта.
 * @param port_n Номер порта.
 * @param pin_n Номер пина.
 * @return Код ошибки.
 */
extern err_t pin_init(pin_t* pin, uint8_t port_n, uint8_t pin_n);

/**
 * Устанавливает режим вывода для пина порта.
 * @param pin Пин порта.
 */
ALWAYS_INLINE static void pin_set_out(pin_t* pin)
{
    BIT_ON_MASK(*pin->port.ddr, pin->_port_mask);
}

/**
 * Устанавливает режим ввода для пина порта.
 * @param pin Пин порта.
 */
ALWAYS_INLINE static void pin_set_in(pin_t* pin)
{
    BIT_OFF_MASK(*pin->port.ddr, pin->_port_mask);
}

/**
 * Разрешает подтяжку на пине порта.
 * @param pin Пин порта.
 */
ALWAYS_INLINE static void pin_pullup_enable(pin_t* pin)
{
    BIT_ON_MASK(*pin->port.out, pin->_port_mask);
}

/**
 * Запрещает подтяжку на пине порта.
 * @param pin Пин порта.
 */
ALWAYS_INLINE static void pin_pullup_disable(pin_t* pin)
{
    BIT_OFF_MASK(*pin->port.out, pin->_port_mask);
}

/**
 * Зажигает пин порта.
 * @param pin Пин порта.
 */
ALWAYS_INLINE static void pin_on(pin_t* pin)
{
    BIT_ON_MASK(*pin->port.out, pin->_port_mask);
}

/**
 * Гасит пин порта.
 * @param pin Пин порта.
 */
ALWAYS_INLINE static void pin_off(pin_t* pin)
{
    BIT_OFF_MASK(*pin->port.out, pin->_port_mask);
}

/**
 * Изменяет пин порта.
 * @param pin Пин порта.
 */
ALWAYS_INLINE static void pin_toggle(pin_t* pin)
{
    BIT_TOGGLE_MASK(*pin->port.out, pin->_port_mask);
}

/**
 * Устанавливает значение на пине порта.
 * @param pin Пин порта.
 * @param value Значение, имеет знавение только нулевой бит.
 */
ALWAYS_INLINE static void pin_set_value(pin_t* pin, uint8_t value)
{
    BIT_SET_MASK(*pin->port.out, pin->_port_mask, value);
}

/**
 * Получение значения на пине порта.
 * @param pin Пин порта.
 * @return Значение на пине порта.
 */
ALWAYS_INLINE static uint8_t pin_get_value(pin_t* pin)
{
    return BIT_TEST_VALUE_MASK(*pin->port.in, pin->_port_mask);
}

/**
 * Получает значения регистров и маски для
 * заданного диапазона пинов порта.
 * @param pin_range Диапазон пинов порта.
 * @param port_n Номер порта.
 * @param pins_offset Смещение пинов порта.
 * @param pins_count Число пинов порта.
 * @return Код ошибки.
 */
err_t pin_range_init(pin_range_t* pin_range, uint8_t port_n, uint8_t pins_offset, uint8_t pins_count);

/**
 * Устанавливает режим вывода для диапазона пинов порта.
 * @param pin_range Диапазон пинов порта.
 */
ALWAYS_INLINE static void pin_range_set_out(pin_range_t* pin_range)
{
    BIT_ON_MASK(*pin_range->port.ddr, pin_range->_port_mask);
}

/**
 * Устанавливает режим ввода для диапазона пинов порта.
 * @param pin_range Диапазон пинов порта.
 */
ALWAYS_INLINE static void pin_range_set_in(pin_range_t* pin_range)
{
    BIT_OFF_MASK(*pin_range->port.ddr, pin_range->_port_mask);
}

/**
 * Разрешает подтяжку на диапазоне пинов порта.
 * @param pin_range Диапазон пинов порта.
 */
ALWAYS_INLINE static void pin_range_pullup_enable(pin_range_t* pin_range)
{
    BIT_ON_MASK(*pin_range->port.out, pin_range->_port_mask);
}

/**
 * Запрещает подтяжку на диапазоне пинов порта.
 * @param pin_range Диапазон пинов порта.
 */
ALWAYS_INLINE static void pin_range_pullup_disable(pin_range_t* pin_range)
{
    BIT_OFF_MASK(*pin_range->port.out, pin_range->_port_mask);
}

/**
 * Разрешает подтяжку на заданных пинах
 * диапазона пинов порта.
 * @param pin_range Диапазон пинов порта.
 * @param pullups Значения подтяжек.
 */
ALWAYS_INLINE static void pin_range_pullup_set(pin_range_t* pin_range, uint8_t pullups)
{
    register uint8_t port_value = *pin_range->port.out & ~pin_range->_port_mask;
    port_value |= (pullups << pin_range->offset) & pin_range->_port_mask;
    *pin_range->port.out = port_value;
}

/**
 * Зажигает диапазон пинов порта.
 * @param pin_range Диапазон пинов порта.
 */
ALWAYS_INLINE static void pin_range_on(pin_range_t* pin_range)
{
    BIT_ON_MASK(*pin_range->port.out, pin_range->_port_mask);
}

/**
 * Гасит диапазон пинов порта.
 * @param pin_range Диапазон пинов порта.
 */
ALWAYS_INLINE static void pin_range_off(pin_range_t* pin_range)
{
    BIT_OFF_MASK(*pin_range->port.out, pin_range->_port_mask);
}

/**
 * Устанавливает значение на диапазоне пинов порта.
 * @param pin_range Диапазон пинов порта.
 * @param value Значение.
 */
ALWAYS_INLINE static void pin_range_set_value(pin_range_t* pin_range, uint8_t value)
{
    register uint8_t port_value = *pin_range->port.out & ~pin_range->_port_mask;
    port_value |= (value << pin_range->offset) & pin_range->_port_mask;
    *pin_range->port.out = port_value;
}

/**
 * Получение значения на диапазоне пинов порта.
 * @param pin_range Диапазон пинов порта.
 * @return Значение.
 */
ALWAYS_INLINE static uint8_t pin_range_get_value(pin_range_t* pin_range)
{
    return (*pin_range->port.in & pin_range->_port_mask) >> pin_range->offset;
}

#endif  //_PORTS_H_
