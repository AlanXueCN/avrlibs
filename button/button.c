#include "button.h"
#include "bits/bits.h"


#define BUTTON_LEVEL_MAX BUTTON_LEVEL_HI


#define BUTTON_STATE_CHANGE_TIME_MS 10

static counter_t button_state_change_ticks = 0;


err_t button_init(button_t* button, uint8_t port, uint8_t pin_n, button_level_t pressed_level, bool need_pullup)
{
    if(pressed_level > BUTTON_LEVEL_MAX) return E_INVALID_VALUE;
    
    err_t err = pin_init(&button->pin, port, pin_n);
    if(err != E_NO_ERROR) return err;
    
    button->pressed_level = pressed_level;
    
    button->state = BUTTON_STATE_RELEASED;
    
    if(button_state_change_ticks == 0){
        button_state_change_ticks = system_counter_ticks_per_sec() * BUTTON_STATE_CHANGE_TIME_MS / 1000;
    }
    
    pin_set_in(&button->pin);
    if(need_pullup) pin_pullup_enable(&button->pin);
    else pin_pullup_disable(&button->pin);
    
    return E_NO_ERROR;
}

bool button_check(button_t* button)
{
    switch(button->state){
        default:
        case BUTTON_STATE_RELEASED:
            if(pin_get_value(&button->pin) == button->pressed_level){
                button->state = BUTTON_STATE_PRESSING;
                button->press_time = system_counter_ticks();
            }
            break;
        case BUTTON_STATE_PRESSING:
            if(pin_get_value(&button->pin) != button->pressed_level){
                button->state = BUTTON_STATE_RELEASED;
            }else{
                if(system_counter_diff(&button->press_time) >= button_state_change_ticks){
                    button->state = BUTTON_STATE_PRESSED;
                    return true;
                }
            }
            break;
        case BUTTON_STATE_PRESSED:
            if(pin_get_value(&button->pin) != button->pressed_level){
                button->state = BUTTON_STATE_RELEASING;
                button->press_time = system_counter_ticks();
            }
            break;
        case BUTTON_STATE_RELEASING:
            if(pin_get_value(&button->pin) == button->pressed_level){
                button->state = BUTTON_STATE_PRESSED;
            }else{
                if(system_counter_diff(&button->press_time) >= button_state_change_ticks){
                    button->state = BUTTON_STATE_RELEASED;
                    return true;
                }
            }
            break;
    }
    return false;
}

bool button_pressed(button_t* button)
{
    return BIT_VALUE(button->state, 0);
}
