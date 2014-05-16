/**
 * @file dpy7par.h
 * Библиотека для работы с 7-сегментными индикатороми, соединёнными через дешифратор.
 */

#ifndef DPY7PAR_H
#define	DPY7PAR_H

#include <stdint.h>
#include <stdbool.h>
#include "ports/ports.h"
#include "errors/errors.h"
#include "display_7seg/display_7seg.h"


//Максимальное число индикаторов.
#define DPY7_PAR_MAX_INDICATORS 16

/**
 * Структура дисплея.
 * Посегментная индикация.
 * Управление сегментами через дешифратор на sel_seg_pins
 * и общими выводами индикаторов через sel_ind_pins{lo,hi}.
 */
typedef struct _Display_7seg_Par {
    //Диапазон пинов порта - селектор сегментов индикаторов.
    pin_range_t sel_seg_pins;
    //Диапазон пинов порта - селектор индикатора, младшие значения.
    pin_range_t sel_ind_pins_lo;
    //Диапазон пинов порта - селектор индикатора, старшие значения.
    pin_range_t sel_ind_pins_hi;
    //Полярность селектора индикатора - true - если управление осуществляется по плюсу, иначе false.
    bool ind_polarity;
    //Интервал задержки между переключением индикаторов.
    uint8_t wait_time_ms;
    //Число индикаторов.
    uint8_t _inds_count;
} dpy7par_t;

/**
 * Вополняет инициализацию структуры дисплея и портов.
 * @param dpy Дисплей.
 * @param sel_seg_port_n Номер порта селектора сегментов.
 * @param sel_seg_pins_offset Смещение пинов порта селектора сегментов.
 * @param sel_seg_pins_count Число пинов порта селектора сегментов.
 * @param sel_ind_port_n_lo Номер порта селектора индикаторов, младшие значения.
 * @param sel_ind_pins_offset_lo Смещение пинов порта селектора индикаторов, младшие значения.
 * @param sel_ind_pins_count_lo Число пинов порта селектора индикаторов, младшие значения.
 * @param sel_ind_port_n_hi Номер порта селектора индикаторов, старшие значения.
 * @param sel_ind_pins_offset_hi Смещение пинов порта селектора индикаторов, старшие значения.
 * @param sel_ind_pins_count_hi Число пинов порта селектора индикаторов, старшие значения.
 * @param ind_polarity Полярность селектора индикатора.
 * @param wait_ms Интервал задержки между переключением индикаторов.
 * @return Код ошибки.
 */
extern err_t dpy7par_init(dpy7par_t* dpy,
                        uint8_t sel_seg_port_n,
                        uint8_t sel_seg_pins_offset,
                        uint8_t sel_seg_pins_count,
                        uint8_t sel_ind_port_n_lo,
                        uint8_t sel_ind_pins_offset_lo,
                        uint8_t sel_ind_pins_count_lo,
                        uint8_t sel_ind_port_n_hi,
                        uint8_t sel_ind_pins_offset_hi,
                        uint8_t sel_ind_pins_count_hi,
                        bool ind_polarity,
                        uint8_t wait_ms);

/**
 * Очищает дисплей.
 * @param dpy Дисплей.
 */
extern void dpy7par_clear(dpy7par_t* dpy);

/**
 * Отображает на дисплее массив значений.
 * @param dpy Дисплей.
 * @param offset Смещение относительно начала.
 * @param values Значения.
 * @param count Число значений.
 */
extern void dpy7par_write(dpy7par_t* dpy, uint8_t offset, const value7seg_t* values, uint8_t count);


#endif	/* DPY7PAR_H */

