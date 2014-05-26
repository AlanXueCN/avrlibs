 /**
 * @file int1.h
 * Библиотека для работы с внешним прерыванием 1.
 */

#ifndef INT1_H
#define	INT1_H

#include <stdint.h>
#include <stdbool.h>
#include "ports/ports.h"
#include "errors/errors.h"

//! Контроль чувствительности к событиям внешнего прерывания 1.
//! Срабатывание при низком уровне.
#define INT1_SENSE_CONTROL_LOW_LEVEL        0
//! Срабатывание при изменении уровня.
#define INT1_SENSE_CONTROL_LOGICAL_CHANGE   1
//! Срабатывание при падающем фронте.
#define INT1_SENSE_CONTROL_FALLING_EDGE     2
//! Срабатывание при возрастающем фронте.
#define INT1_SENSE_CONTROL_RISING_EDGE      3
//! Тип контроля чувствительности.
typedef uint8_t int1_sense_control_t;

//! Тип каллбэка внешнего прерывания.
typedef void (*int1_callback_t)(void);

/**
 * Инициализирует внешнее прерывание на пине.
 * @param port_n Номер порта.
 * @param pin_n Номер пина порта.
 * @param need_pullup Необходимость подтяжки.
 * @return Код ошибки.
 */
extern err_t int1_init(uint8_t port_n, uint8_t pin_n, bool need_pullup);

/**
 * Получает пин.
 * @return Пин.
 */
extern pin_t* int1_pin(void);

/**
 * Получает каллбэк.
 * @return Каллбэк.
 */
extern int1_callback_t int1_callback(void);

/**
 * Устанавливает каллбэк.
 * @param callback Кллбэк.
 */
extern void int1_set_callback(int1_callback_t callback);

/**
 * Получает чувствительность к событиям.
 * @return Чувствительность к событиям.
 */
extern int1_sense_control_t int1_sense_control(void);

/**
 * Устанавливает чувствительность к событиям.
 * @param sense_control Чувствительность к событиям.
 * @return Код ошибки.
 */
extern err_t int1_set_sense_control(int1_sense_control_t sense_control);

/**
 * Получает флаг разрешённости внешнего прерывания.
 * @return Разрешённость внешнего прерывания.
 */
extern bool int1_enabled(void);

/**
 * Разрешает внешнее прерывание.
 */
extern void int1_enable(void);

/**
 * Запрещает внешнее прерывание.
 */
extern void int1_disable(void);

#endif	/* INT1_H */

