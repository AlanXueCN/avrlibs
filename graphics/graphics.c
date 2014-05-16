#include "graphics.h"
#include <stddef.h>
#include <string.h>
#include <stdbool.h>
#include "bits/bits.h"
#include "utils/utils.h"


//! Максимальные значения.
#define GRAPHICS_BYTE_ALIGN_MAX    GRAPHICS_BYTE_ALIGN_VERTICAL
#define PIXEL_VALUE_MAX            PIXEL_ON
#define PAINT_MODE_MAX             PAINT_MODE_XOR
#define FILL_MODE_MAX              FILL_MODE_SOLID



static void graphics_reset_dirty(graphics_t* graphics)
{
    graphics->dirty_from.x = GRAPHICS_MAX_SIZE;
    graphics->dirty_from.y = GRAPHICS_MAX_SIZE;
    graphics->dirty_to.x = 0;
    graphics->dirty_to.y = 0;
}

static void graphics_update_dirty(graphics_t* graphics, graphics_pos_t x, graphics_pos_t y)
{
    if(graphics->dirty_from.x > x) graphics->dirty_from.x = x;
    if(graphics->dirty_from.y > y) graphics->dirty_from.y = y;
    if(graphics->dirty_to.x < x) graphics->dirty_to.x = x;
    if(graphics->dirty_to.y < y) graphics->dirty_to.y = y;
}

err_t graphics_init(graphics_t* graphics, uint8_t* buffer,
                    graphics_size_t width, graphics_size_t height,
                    graphics_byte_align_t byte_align)
{
    if(buffer == NULL) return E_NULL_POINTER;
    if(width > GRAPHICS_MAX_SIZE || height > GRAPHICS_MAX_SIZE) return E_INVALID_VALUE;
    if(byte_align > GRAPHICS_BYTE_ALIGN_MAX) return E_INVALID_VALUE;
    
    graphics->buffer = buffer;
    graphics->buffer_size = width * height / 8;
    graphics->byte_align = byte_align;
    graphics->height = height;
    graphics->width = width;
    
    graphics_reset_dirty(graphics);
    
    return E_NO_ERROR;
}

void graphics_clear(graphics_t* graphics)
{
    memset(graphics->buffer, 0x0, graphics->buffer_size);
}

graphics_size_t graphics_width_bytes(graphics_t* graphics)
{
    if(graphics->byte_align == GRAPHICS_BYTE_ALIGN_HORIZONTAL){
        return graphics->width >> 3;
    }
    return graphics->width;
}

graphics_size_t graphics_height_bytes(graphics_t* graphics)
{
    if(graphics->byte_align == GRAPHICS_BYTE_ALIGN_HORIZONTAL){
        return graphics->height;
    }
    return graphics->height >> 3;
}

void graphics_set_pixel_value(graphics_t* graphics, pixel_value_t value)
{
    if(value > PIXEL_VALUE_MAX) return;
    
    graphics->pixel_value = value;
}

void graphics_set_paint_mode(graphics_t* graphics, paint_mode_t mode)
{
    if(mode > PAINT_MODE_MAX) return;
    
    graphics->paint_mode = mode;
}

void graphics_set_fill_mode(graphics_t* graphics, fill_mode_t mode)
{
    if(mode > FILL_MODE_MAX) return;
    
    graphics->fill_mode = mode;
}

static bool graphics_get_pixel_address(graphics_t* graphics, graphics_pos_t x, graphics_pos_t y, size_t* byte_n, uint8_t* bit_n)
{
    if(x < 0 || x >= graphics->width) return false;
    if(y < 0 || y >= graphics->height) return false;
    
    if(graphics->byte_align == GRAPHICS_BYTE_ALIGN_HORIZONTAL){
        *byte_n = ((size_t)graphics->width * y + x) >> 3;
        *bit_n = (x & 0x7);
    }else{
        *byte_n = ((size_t)graphics->width * (y >> 3)) + x;
        *bit_n = (y & 0x7);
    }
    
    return true;
}

void graphics_set_pixel(graphics_t* graphics, graphics_pos_t x, graphics_pos_t y)
{
    size_t byte_n;
    uint8_t bit_n;
    
    if(!graphics_get_pixel_address(graphics, x, y,&byte_n, &bit_n)) return;
    
    switch(graphics->paint_mode){
        case PAINT_MODE_SET:
            BIT_SET(graphics->buffer[byte_n], bit_n, graphics->pixel_value);
            break;
        case PAINT_MODE_XOR:
            graphics->buffer[byte_n] ^= BIT_BYVAL(bit_n, graphics->pixel_value);
            break;
    }
    
    
    graphics_update_dirty(graphics, x, y);
}

pixel_value_t graphics_get_pixel(graphics_t* graphics, graphics_pos_t x, graphics_pos_t y)
{
    size_t byte_n;
    uint8_t bit_n;
    
    if(!graphics_get_pixel_address(graphics, x, y,&byte_n, &bit_n)) return PIXEL_OFF;
    
    return BIT_VALUE(graphics->buffer[byte_n], bit_n);
}

//#define PRINT_LINE_LOG

#ifdef PRINT_LINE_LOG
#include <stdio.h>
#endif

void graphics_line(graphics_t* graphics,
                          graphics_pos_t x_from, graphics_pos_t y_from,
                          graphics_pos_t x_to, graphics_pos_t y_to)
{
    graphics_pos_t delta_x;
    graphics_pos_t delta_y;
    
    graphics_pos_t err;
    
    int8_t dx, dy;
    
#ifdef PRINT_LINE_LOG
    graphics_pos_t tmp;
#endif
    
    graphics_pos_t i;
    
    bool x_as_y = false;
    
#ifdef PRINT_LINE_LOG
    printf("(%d, %d) -> (%d, %d)\n", x_from, y_from, x_to, y_to);
#endif
    
    /*if(x_from > x_to){
        SWAP(x_from, x_to, tmp);
        SWAP(y_from, y_to, tmp);
    }*/
    
#ifdef PRINT_LINE_LOG
    printf("(%d, %d) -> (%d, %d)\n", x_from, y_from, x_to, y_to);
#endif
    
    delta_x = x_to - x_from; delta_x = ABS(delta_x);
    delta_y = y_to - y_from; delta_y = ABS(delta_y);
    
#ifdef PRINT_LINE_LOG
    printf("delta_x: %d delta_y: %d\n", delta_x, delta_y);
#endif
    
    if(delta_y > delta_x) x_as_y = true;
    
#ifdef PRINT_LINE_LOG
    printf("x_as_y: %d\n", x_as_y);
#endif
    
    if(x_to >= x_from){
        dx = 1;
    }else{
        dx = -1;
    }
    
    if(y_to >= y_from){
        dy = 1;
    }else{
        dy = -1;
    }
    
#ifdef PRINT_LINE_LOG
    printf("dx: %d dy: %d\n", dx, dy);
#endif
    
    err = 0;
    i = 0;
    
    if(x_as_y){
    
        for(; i <= delta_y; i ++){
            
#ifdef PRINT_LINE_LOG
            printf("(%d, %d)\n", x_from, y_from);
#endif
            graphics_set_pixel(graphics, x_from, y_from);
            
            err += delta_x;
            
            if((err + err) >= delta_y){
                err -= delta_y;
                x_from += dx;
            }
            y_from += dy;
        }
        
    }else{
    
        for(; i <= delta_x; i ++){
            
#ifdef PRINT_LINE_LOG
            printf("(%d, %d)\n", x_from, y_from);
#endif
            graphics_set_pixel(graphics, x_from, y_from);
            
            err += delta_y;
            
            if((err + err) >= delta_x){
                err -= delta_x;
                y_from += dy;
            }
            x_from += dx;
        }
    
    }
}

void graphics_hline(graphics_t* graphics, graphics_pos_t y, graphics_pos_t x_from, graphics_pos_t x_to)
{
    uint8_t delta;
    
    if(x_to >= x_from){
        delta = 1;
    }else{
        delta = -1;
    }
    
    for(;;){
        graphics_set_pixel(graphics, x_from, y);
        if(x_from == x_to) break;
        x_from += delta;
    }
}

void graphics_vline(graphics_t* graphics, graphics_pos_t x, graphics_pos_t y_from, graphics_pos_t y_to)
{
    uint8_t delta;
    
    if(y_to >= y_from){
        delta = 1;
    }else{
        delta = -1;
    }
    
    for(;;){
        graphics_set_pixel(graphics, x, y_from);
        if(y_from == y_to) break;
        y_from += delta;
    }
}

/**
 * Заполняет строку рисуемой фигуры.
 * @param graphics Растр.
 * @param first_y Первая строка.
 * @param y Текущая строка.
 * @param x_from Начальная абсцисса.
 * @param x_to Конечная абсцисса.
 */
static void graphics_fill(graphics_t* graphics, graphics_pos_t first_y, graphics_pos_t y, graphics_pos_t x_from, graphics_pos_t x_to)
{
    switch(graphics->fill_mode){
        default:
        case FILL_MODE_NONE:
            break;
            case FILL_MODE_SOLID:
            graphics_hline(graphics, y, x_from, x_to);
            break;
    }
}

//#define PRINT_CIRCLE_LOG

#ifdef PRINT_CIRCLE_LOG
#include <stdio.h>
#endif

void graphics_circle(graphics_t* graphics,
                     graphics_pos_t center_x, graphics_pos_t center_y,
                     graphics_pos_t radius)
{
    graphics_pos_t x = 0;
    graphics_pos_t y = radius;
    graphics_pos_t delta = 1 - (radius + radius);
    graphics_pos_t err;
    
    graphics_pos_t old_y = y;
    
    for(;y >= 0;){
        /*
        if(x >= 0 && y >= 0) graphics_set_pixel(graphics, center_x + x, center_y + y);//(0,  r)-( r, 0)
        if(x >= 0 && y >= 1) graphics_set_pixel(graphics, center_x + x, center_y - y);//(0, -r)-( r, 0)
        if(x >= 1 && y >= 0) graphics_set_pixel(graphics, center_x - x, center_y + y);//(0,  r)-(-r, 0)
        if(x >= 1 && y >= 1) graphics_set_pixel(graphics, center_x - x, center_y - y);//(0, -r)-(-r, 0)
        */
        
        graphics_set_pixel(graphics, center_x + x, center_y + y);
        if(y > 0){
            graphics_set_pixel(graphics, center_x + x, center_y - y);
        }
        if(x > 0){
            graphics_set_pixel(graphics, center_x - x, center_y + y);
            if(y > 0){
                graphics_set_pixel(graphics, center_x - x, center_y - y);
            }
            // Заливка.
            if(old_y != y){
                graphics_fill(graphics, center_y - radius, center_y - y, center_x - x + 1, center_x + x - 1);
                if(y > 0){
                    graphics_fill(graphics, center_y - radius, center_y + y, center_x - x + 1, center_x + x - 1);
                }
            }
        }
        
        old_y = y;
        
#ifdef PRINT_CIRCLE_LOG
        printf("(%d, %d)\n", x, y);
#endif
        
        err = (delta + y + delta + y) - 1;
        if(delta < 0 && err <= 0){
            x ++;
            delta += (x + x) + 1;
            continue;
        }
        err = (delta + delta - x - x) - 1;
        if(delta > 0 && err > 0){
            y --;
            delta += 1 - (y + y);
            continue;
        }
        x ++;
        delta += (x - y + x - y + 4);
        y --;
    }
}

void graphics_square(graphics_t* graphics,
                     graphics_pos_t x_from, graphics_pos_t y_from,
                     graphics_pos_t x_to, graphics_pos_t y_to)
{
    graphics_pos_t tmp;
    
    if(x_from > x_to) SWAP(x_from, x_to, tmp);
    if(y_from > y_to) SWAP(y_from, y_to, tmp);
    
    graphics_hline(graphics, y_from, x_from, x_to);
    
    if(y_from != y_to){
        
        graphics_hline(graphics, y_to,   x_from, x_to);

        if(y_to - y_from > 1){
            y_to --; y_from ++;

            graphics_vline(graphics, x_from, y_from, y_to);
            graphics_vline(graphics, x_to,   y_from, y_to);

            if(graphics->fill_mode != FILL_MODE_NONE && (x_to - x_from > 1)){
                
                x_from ++;
                x_to --;

                tmp = y_from;

                while(y_from <= y_to){
                    graphics_fill(graphics, tmp, y_from, x_from, x_to);
                    y_from ++;
                }
            }
        }
    }
}
