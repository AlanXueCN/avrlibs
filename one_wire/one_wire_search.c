#include "one_wire_search.h"


/**
 * Получает линейный номер бита в структуре one_wire_t,
 * соотвествующий порядковому номеру передаваемого бита.
 * @param bit_n Номер принимаемого бита.
 * @return Номер бита в структуре one_wire_t.
 */
/*static uint8_t get_linear_bit_n(uint8_t bit_n)
{
    uint8_t byte_n = bit_n >> 3;
    //uint8_t byte_bit_n = bit_n & 0x7;
    switch(byte_n){
        //Передача family code.
        case 0:
            return (bit_n & 0x7);//(byte_bit_n);
        //Передача crc.
        case 7:
            return (56 + (bit_n & 0x7));//(56 + byte_bit_n);
        //Передача serial.
        default:
            return bit_n;//(((byte_n) << 3) + byte_bit_n);
    }
}*/

err_t one_wire_search_roms(one_wire_t* ow, one_wire_rom_id_t* roms,
                           uint8_t roms_count, uint8_t* roms_found,
                           uint8_t max_attempts)
{
    //Текущий ROM.
    uint8_t cur_rom = 0;
    //бит, временная переменная
    uint8_t tmp_bit = 0;
    //Первый бит ответа.
    uint8_t bit0 = 0;
    //Второй бит ответа.
    uint8_t bit1 = 0;
    //Результат "сравнения" бит.
    uint8_t cmp = 0;
    //Итератор бит.
    uint8_t i_bit = 0;
    //Линейное значение передаваемого бита.
    uint8_t bit_l = 0;
    //Начальное значение позоций коллизий.
    #define COLLIS_POS_INITIAL 255
    //Последняя коллизия.
    uint8_t last_collis = COLLIS_POS_INITIAL;
    //Последняя коллизия на предыдущем проходе.
    uint8_t prev_collis = COLLIS_POS_INITIAL;
    //Контрольная сумма.
    uint8_t crc = 0;
    //Переменная для отладки.
    //uint8_t debug = 0;
    //Текущее число оставшихся попыток.
    uint8_t cur_attempts = max_attempts;
    
    *roms_found = 0;
    
    for(; cur_rom < roms_count;){
        //Если нет устройств на шине - нет смысла их искать (с) КЭП.
        if(!one_wire_reset(ow)){
            return E_ONE_WIRE_DEVICES_NOT_FOUND;
        }
        //Пошлём команду поиска.
        one_wire_send_cmd(ow, ONE_WIRE_CMD_SEARCH_ROM);
        
        //Нужно считать 64 бита.
        for(i_bit = 0; i_bit < 64; i_bit ++){
            
            //Получим номер бита в структуре ROM.
            bit_l = i_bit;//get_linear_bit_n(i_bit);
            
            //Считаем два бита, определяющих текущий бит в ромах устройств.
            bit1 = one_wire_read_bit(ow);
            bit0 = one_wire_read_bit(ow);
            
            cmp = (bit1 << 1) | bit0;
            
            //Оба бита равны нулю.
            //Часть устройств имеют 0, а часть 1 в этой позиции.
            if(cmp == 0x0){
                //Если текущая позиция меньше позиции предыдущей коллизии.
                if(i_bit < prev_collis){
                    //Получим бит предыдущего ROM, или 0.
                    if(cur_rom == 0) tmp_bit = 0;
                    else tmp_bit = bits_value((uint8_t*)&roms[cur_rom - 1], bit_l);
                    
                    //Установим его же значение.
                    bits_set_value((uint8_t*)&roms[cur_rom], bit_l, tmp_bit);
                    one_wire_write_bit(ow, tmp_bit);
                    
                    //Если бит не переключался, тогда зафиксируем коллизию.
                    if(tmp_bit == 0) last_collis = i_bit;
                //Если мы на позиции предыдущей коллизии.
                }else if(i_bit == prev_collis){
                    //Устанавливаем бит в 1.
                    bits_on((uint8_t*)&roms[cur_rom], bit_l);
                    one_wire_write_bit(ow, 1);
                //Если текущая позиция после позиции предыдущей коллизии.
                }else{ //if(i_bit > prev_collis)
                    //Устанавливаем бит в 0.
                    bits_off((uint8_t*)&roms[cur_rom], bit_l);
                    one_wire_write_bit(ow, 0);
                    //Зафиксируем коллизию.
                    last_collis = i_bit;
                }
            }
            //Второй бит равен единице.
            //Все устройства имеют 0 в этой позиции.
            else if(cmp == 0x1){
                bits_off((uint8_t*)&roms[cur_rom], bit_l);
                one_wire_write_bit(ow, 0);
            }
            //Первый бит равен единице.
            //Все устройства имеют 1 в этой позиции.
            else if(cmp == 0x2){
                bits_on((uint8_t*)&roms[cur_rom], bit_l);
                one_wire_write_bit(ow, 1);
            }
            //Оба бита равны единице.
            //Нет устройств.
            else{//0x3:
                //Если исчерпали попытки.
                if(cur_attempts == 0){
                    //Выход.
                    return E_ONE_WIRE_SEARCH_LOGIC_ERROR;
                }
                //Иначе попробуем ещё раз.
                cur_attempts --;
                break;
            }
        }
        //Если поиск устройства окончен.
        if(i_bit == 64){
            //Проверим контрольную сумму.
            crc = one_wire_calc_crc((const void*)&roms[cur_rom], 0x7);//1 байт family + 6 байт serial
            //Если совпадают, то поиск успешен.
            if(crc == roms[cur_rom].crc){
                //Увеличим число найденных ROM.
                (*roms_found) ++;
                //Перейдём к следующему ROM.
                cur_rom ++;
                //Обновим значение позиции предыдущей коллизии.
                prev_collis = last_collis;
                //сбросим число попыток.
                cur_attempts = max_attempts;
                //Если коллизий небыло, т.е. найдено последнее устройство - выход.
                if(last_collis == COLLIS_POS_INITIAL){
                    break;
                }
            }else{
                //Если исчерпали попытки.
                if(cur_attempts == 0){
                    //Выход.
                    return E_ONE_WIRE_INVALID_CRC;
                }
                //Иначе попробуем ещё раз.
                cur_attempts --;
            }
        }
        //Сбросим значение последней текущей коллизии.
        last_collis = COLLIS_POS_INITIAL;
    }
    
    return E_NO_ERROR;
}
