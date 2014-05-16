#include "dpy7par.h"
#include "utils/utils.h"
#include "utils/delay.h"
#include "bits/bits.h"


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
                        uint8_t wait_ms)
{
    err_t res = pin_range_init(&dpy->sel_seg_pins, sel_seg_port_n, sel_seg_pins_offset, sel_seg_pins_count);
    if(res != E_NO_ERROR) return res;
    
    res = pin_range_init(&dpy->sel_ind_pins_lo, sel_ind_port_n_lo, sel_ind_pins_offset_lo, sel_ind_pins_count_lo);
    if(res != E_NO_ERROR) return res;
    
    res = pin_range_init(&dpy->sel_ind_pins_hi, sel_ind_port_n_hi, sel_ind_pins_offset_hi, sel_ind_pins_count_hi);
    if(res != E_NO_ERROR) return res;
    
    dpy->ind_polarity = ind_polarity;
    dpy->wait_time_ms = wait_ms;
    dpy->_inds_count = sel_ind_pins_count_lo + sel_ind_pins_count_hi;
    
    pin_range_set_out(&dpy->sel_seg_pins);
    pin_range_set_out(&dpy->sel_ind_pins_lo);
    pin_range_set_out(&dpy->sel_ind_pins_hi);
    
    return E_NO_ERROR;
}

ALWAYS_INLINE static void dpy7par_clear_value(dpy7par_t* dpy)
{
    register uint8_t port_val = dpy->ind_polarity ? DPY7_CODE_NONE : ~DPY7_CODE_NONE;
    pin_range_set_value(&dpy->sel_ind_pins_lo, port_val);
    pin_range_set_value(&dpy->sel_ind_pins_hi, port_val);
}

void dpy7par_clear(dpy7par_t* dpy)
{
    dpy7par_clear_value(dpy);
    delay_ms8(dpy->wait_time_ms);
}

void dpy7par_write(dpy7par_t* dpy, uint8_t offset, const value7seg_t* values, uint8_t count)
{
    //Маска сегмента
    uint8_t mask = 1;
    
    //Маска сегментов для всех значений.
    uint16_t segments_mask = 0;
    uint8_t segments_mask_lo = 0;
    uint8_t segments_mask_hi = 0;
    
    //Итераторы.
    uint8_t i_bits = 0;
    uint8_t i_values = 0;
    //Маска бита и номером i_values.
    uint16_t i_values_mask = 1;
    
    //Если смещение больше числа индикаторов - не выводим ничего.
    if(offset > dpy->_inds_count) return;
    //Если число значений больше числа индикаторов - отмекаем лишние.
    if(count > dpy->_inds_count) count = dpy->_inds_count;
    
    //Высчитаем общую длину выводимой строки
    uint8_t offset_and_count = offset + count;
    //Если она больше числа индикаторов
    if(offset_and_count > dpy->_inds_count){
        //Отсечём лишние значения.
        count -= offset;
    }
    
    //Цикл по всем сегментам.
    for(mask = 1, i_bits = 0; i_bits < 8; i_bits ++, mask <<= 1){;
        //Цикл по всем знакам.
        for(i_values_mask = 1, segments_mask = 0, i_values = 0;
                i_values < count; i_values ++, i_values_mask <<= 1){
            //if(BIT_TEST_MASK(values[i_values], mask)) BIT_ON(segments_mask, i_values);
            //Если сегмент присутствует в маске, то добавить позицию индикатора в маску.
            if(BIT_TEST_MASK(values[i_values], mask)) BIT_ON_MASK(segments_mask, i_values_mask);
        }
        //Если есть что отображать.
        if(segments_mask){
            //Если управляем по противоположной полярности.
            if(!dpy->ind_polarity){
                //Инвертируем маску.
                segments_mask = ~segments_mask;
            }
            dpy7par_clear_value(dpy);
            
            segments_mask_lo = (uint8_t)segments_mask;
            segments_mask_hi = (uint8_t)(segments_mask >> dpy->sel_ind_pins_lo.count);
            
            pin_range_set_value(&dpy->sel_seg_pins, i_bits);
            pin_range_set_value(&dpy->sel_ind_pins_lo, segments_mask_lo);
            pin_range_set_value(&dpy->sel_ind_pins_hi, segments_mask_hi);
            
            delay_ms8(dpy->wait_time_ms);
        }
    }
}

