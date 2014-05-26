 /**
 * @file int0.h
 * Библиотека для работы с внешним прерыванием 0.
 */

#ifndef INT0_H
#define	INT0_H

#include <stdint.h>
#include <stdbool.h>
#include "ports/ports.h"
#include "errors/errors.h"

//! Контроль чувствительности к событиям внешнего прерывания 0.
//! Срабатывание при низком уровне.
#define INT0_SENSE_CONTROL_LOW_LEVEL        0
//! Срабатывание при изменении уровня.
#define INT0_SENSE_CONTROL_LOGICAL_CHANGE   1
//! Срабатывание при падающем фронте.
#define INT0_SENSE_CONTROL_FALLING_EDGE     2
//! Срабатывание при возрастающем фронте.
#define INT0_SENSE_CONTROL_RISING_EDGE      3
//! Тип контроля чувствительности.
typedef uint8_t int0_sense_control_t;

//! Тип каллбэка внешнего прерывания.
typedef void (*int0_callback_t)(void);

/**
 * Инициализирует внешнее прерывание на пине.
 * @param port_n Номер порта.
 * @param pin_n Номер пина порта.
 * @param need_pullup Необходимость подтяжки.
 * @return Код ошибки.
 */
extern err_t int0_init(uint8_t port_n, uint8_t pin_n, bool need_pullup);

/**
 * Получает пин.
 * @return Пин.
 */
extern pin_t* int0_pin(void);

/**
 * Получает каллбэк.
 * @return Каллбэк.
 */
extern int0_callback_t int0_callback(void);

/**
 * Устанавливает каллбэк.
 * @param callback Кллбэк.
 */
extern void int0_set_callback(int0_callback_t callback);

/**
 * Получает чувствительность к событиям.
 * @return Чувствительность к событиям.
 */
extern int0_sense_control_t int0_sense_control(void);

/**
 * Устанавливает чувствительность к событиям.
 * @param sense_control Чувствительность к событиям.
 * @return Код ошибки.
 */
extern err_t int0_set_sense_control(int0_sense_control_t sense_control);

/**
 * Получает флаг разрешённости внешнего прерывания.
 * @return Разрешённость внешнего прерывания.
 */
extern bool int0_enabled(void);

/**
 * Разрешает внешнее прерывание.
 */
extern void int0_enable(void);

/**
 * Запрещает внешнее прерывание.
 */
extern void int0_disable(void);

#endif	/* INT0_H */

