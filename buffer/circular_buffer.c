#include "circular_buffer.h"
#include <string.h>
#include "utils/utils.h"


void circular_buffer_init(circular_buffer_t* buffer, uint8_t* ptr, size_t size)
{
    buffer->ptr = ptr;
    buffer->put = 0;
    buffer->get = 0;
    buffer->size = size;
    buffer->count = 0;
}

void circular_buffer_reset(circular_buffer_t* buffer)
{
    buffer->put = 0;
    buffer->get = 0;
    buffer->count = 0;
}

bool circular_buffer_valid(circular_buffer_t* buffer)
{
    return /* buffer->size != 0 && */ buffer->ptr != NULL;
}

size_t circular_buffer_size(circular_buffer_t* buffer)
{
    return buffer->size;
}

size_t circular_buffer_free_size(circular_buffer_t* buffer)
{
    return buffer->size - buffer->count;
}

size_t circular_buffer_avail_size(circular_buffer_t* buffer)
{
    return buffer->count;
}

size_t circular_buffer_put(circular_buffer_t* buffer, uint8_t data)
{
    if(buffer->count == buffer->size) return 0;
    
    buffer->ptr[buffer->put] = data;
    
    CYCLIC_INC(buffer->put, 0, buffer->size);
    
    buffer->count ++;
    
    return 1;
}

size_t circular_buffer_get(circular_buffer_t* buffer, uint8_t* data)
{
    if(buffer->count == 0) return 0;
    
    *data = buffer->ptr[buffer->get];
    
    CYCLIC_INC(buffer->get, 0, buffer->size);
    
    buffer->count --;
    
    return 1;
}

size_t circular_buffer_peek(circular_buffer_t* buffer, uint8_t* data)
{
    if(buffer->count == 0) return 0;
    
    *data = buffer->ptr[buffer->get];
    
    return 1;
}

size_t circular_buffer_write(circular_buffer_t* buffer, const uint8_t* data, size_t size)
{
    size_t n, part1;//, part2;
    
    // Если места недостаточно или размер данных равен 0 - возврат 0.
    if(buffer->size - buffer->count < size || size == 0) return 0;
    
    // Если индекс записи больше индекса чтения.
    if(buffer->put >= buffer->get){
        // Вычислим размер свободных мест в буфере.
        part1 = buffer->size - buffer->put;
        //part2 = buffer->get;
        // Если нехватает - возврат 0.
        //if(part1 + part2 < size) return 0;
        
        // Первая часть данных.
        n = MIN(part1, size);
        // Счётчик.
        buffer->count += n;
        // Скопируем.
        memcpy(buffer->ptr + buffer->put, data, n);
        
        // Если израсходовали всю первую часть.
        if(n == part1){
            // Позицию на 0.
            buffer->put = 0;
            // Вычислим остаток данных для записи.
            n = size - part1;
            // Если записали не всё.
            if(n > 0){
                // Счётчик.
                buffer->count += n;
                
                // Скопируем.
                memcpy(buffer->ptr /* + buffer->put */, data + part1, n);
                
                // Увеличим позицию.
                //CYCLIC_ADD(buffer->put, n, 0, buffer->size);
                buffer->put += n;
            }
        // Иначе.
        }else{
            // Увеличим позицию.
            //CYCLIC_ADD(buffer->put, n, 0, buffer->size);
            buffer->put += n;
        }
    // Иначе.
    }else{
        // Вычислим свободное место в буфере.
        //part1 = buffer->size - buffer->count;
        // Если нехватает - возврат 0.
        //if(part1 < size) return 0;
        // Скопируем.
        memcpy(buffer->ptr + buffer->put, data, size);
        
        // Увеличим позицию.
        //CYCLIC_ADD(buffer->put, size, 0, buffer->size);
        buffer->put += size;
        buffer->count += size;
    }
    
    return size;
}

size_t circular_buffer_read(circular_buffer_t* buffer, uint8_t* data, size_t size)
{
    size_t part1, part2, n;
    size_t res_size = 0;
    
    // Если в буфере нет данных, или размер данных равен 0 - возврат 0.
    if(buffer->count == 0 || size == 0) return 0;
    
    // Если индекс чтения больше индекса записи.
    if(buffer->get >= buffer->put){
        
        // Вычислим размер свободных мест в буфере.
        part1 = buffer->size - buffer->get;
        part2 = buffer->put;
        
        // Первая часть данных.
        n = MIN(part1, size);
        // Счётчик.
        res_size += n;
        buffer->count -= n;
        // Скопируем.
        memcpy(data, buffer->ptr + buffer->get, n);
        
        // Если израсходовали всю первую часть.
        if(n == part1){
            // Позицию на 0.
            buffer->get = 0;
            // Вычислим остаток данных для записи.
            n = size - part1;
            // Если считали не всё.
            if(n > 0){
                // Оставшееся число данных для чтения.
                n = MIN (n, part2);
                // Счётчик.
                res_size += n;
                buffer->count -= n;
                // Скопируем.
                memcpy(data + part1, buffer->ptr /* + buffer->get */, n);
                
                // Увеличим позицию.
                //CYCLIC_ADD(buffer->get, n, 0, buffer->size);
                buffer->get += n;
            }
        // Иначе.
        }else{
            // Увеличим позицию.
            //CYCLIC_ADD(buffer->get, n, 0, buffer->size);
            buffer->get += n;
        }
    // Иначе.
    }else{
        // Вычислим свободное место в буфере.
        part1 = buffer->count;
        
        // Минимальный размер.
        n = MIN(part1, size);
        // Счётчик.
        res_size += n;
        buffer->count -= n;
        // Скопируем.
        memcpy(data, buffer->ptr + buffer->get, n);
        
        // Увеличим позицию.
        //CYCLIC_ADD(buffer->get, n, 0, buffer->size);
        buffer->get += n;
    }
    
    return res_size;
}

