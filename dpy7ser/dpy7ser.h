/**
 * @file dpy7ser.h
 * Библиотека для работы с 7-сегментными индикаторами, соединёнными через сдвиговые регистры.
 */

#ifndef DPY7SER_H
#define	DPY7SER_H

#include <stdint.h>
#include <stdbool.h>
#include "ports/ports.h"
#include "errors/errors.h"
#include "display_7seg/display_7seg.h"


/**
 * Структура дисплея.
 * Статическая индикация.
 * Управление.
 */
typedef struct _Display_7seg_Ser {
    //Пин данных регистра.
    pin_t pin_data;
    //Пин синхронизации регистра.
    pin_t pin_clk;
    //Пин фиксации регистра.
    pin_t pin_swap;
    //Пин ресета регистра.
    //pin_t pin_reset;
    //Полярность селектора индикатора - true - если управление осуществляется по плюсу, иначе false.
    bool ind_polarity;
    //Порядок бит.
    bool bits_order;
} dpy7ser_t;

/**
 * Вополняет инициализацию структуры дисплея и портов.
 * @param dpy Дисплей.
 * @param pin_data_port_n Номер порта пина данных регистра.
 * @param pin_data_n Номер пина данных регистра.
 * @param pin_clk_port_n Номер порта пина синхронизации регистра.
 * @param pin_clk_n Номер пина синхронизации регистра.
 * @param pin_swap_port_n Номер порта пина фиксации регистра.
 * @param pin_swap_n Номер пина фиксации регистра.
 //* @param pin_reset_port_n Номер порта пина ресета регистра.
 //* @param pin_reset_n Номер пина ресета регистра.
 * @param ind_polarity Полярность селектора индикатора.
 * @param bits_order Порядок бит, true если A соотвествует старший бит, иначе false.
 * @return Код ошибки.
 */
extern err_t dpy7ser_init(dpy7ser_t* dpy,
                        uint8_t pin_data_port_n,
                        uint8_t pin_data_n,
                        uint8_t pin_clk_port_n,
                        uint8_t pin_clk_n,
                        uint8_t pin_swap_port_n,
                        uint8_t pin_swap_n,
                        //uint8_t pin_reset_port_n,
                        //uint8_t pin_reset_n,
                        bool ind_polarity,
                        bool bits_order);

/**
 * Отображает на дисплее массив значений.
 * @param dpy Дисплей.
 * @param offset Смещение относительно начала.
 * @param values Значения.
 * @param count Число значений.
 */
extern void dpy7ser_write(dpy7ser_t* dpy, const value7seg_t* values, uint8_t count);

/**
 * Записывает в регистр очередной байт данных.
 * @param dpy Дисплей.
 * @param value Данные.
 */
extern void dpy7ser_put(dpy7ser_t* dpy, value7seg_t value);


#endif	/* DPY7SER_H */

