/**
 * @file button.h
 * Библиотека для работы с кнопками.
 */

#ifndef BUTTON_H
#define	BUTTON_H

#include <stdint.h>
#include <stdbool.h>
#include "ports/ports.h"
#include "errors/errors.h"
#include "counter/counter.h"
#include "defs/defs.h"


//! Логические уровни нажатия кнопки.
#define BUTTON_LEVEL_LO 0
#define BUTTON_LEVEL_HI 1

//! Тип уровня кнопки.
typedef uint8_t button_level_t;

/**
 * Структура кнопки.
 */
typedef struct _Button {
    //Пин.
    pin_t pin;
    //Время нажатия.
    counter_t press_time;
    //Логический уровень нажатой кнопки.
    button_level_t pressed_level;
    //состояние.
    uint8_t state;
}button_t;

//! Состояния кнопки.
#define BUTTON_STATE_RELEASED  0
#define BUTTON_STATE_RELEASING 1
#define BUTTON_STATE_PRESSING  2
#define BUTTON_STATE_PRESSED   3


/**
 * Инициализирует кнопку.
 * Нужен инициализированный системный таймер перед вызовом!
 * @param button Кнопка.
 * @param port Порт кнопки.
 * @param pin_n Номер пина кнопки.
 * @param pressed_level Логический уровень сигнала при нажатии кнопки.
 * @param need_pullup Необходимость подтяжки.
 * @return Код ошибки.
 */
extern err_t button_init(button_t* button, uint8_t port, uint8_t pin_n, button_level_t pressed_level, bool need_pullup);

/**
 * Проверяет состояние кнопки.
 * @param button Кнопка.
 * @return true, если состояние изменилось, иначе false;
 */
extern bool button_check(button_t* button);

/**
 * Получает флаг нажатости кнопки.
 * @param button Кнопка.
 * @return Флаг нажатости кнопки.
 */
extern bool button_pressed(button_t* button);

#endif	/* BUTTON_H */

