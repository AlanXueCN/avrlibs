#include "lcd44780.h"
#include "defs/defs.h"
#include "utils/utils.h"
#include "utils/delay.h"
#include "bits/bits.h"
#include "errno.h"


//! Ширина шины данных LCD в битах.
//! 8 бит.
#define LCD44780_DATA_WIDTH_FULL_BITS 8
//! 4 бита.
#define LCD44780_DATA_WIDTH_HALF_BITS 4
//! Маска для половины шины.
#define LCD44780_DATA_WIDTH_HALF_MASK 0xf

//! Задержка для установки R/W и RS значений.
#define LCD44780_RW_RS_SETUP_TIME_US 1
//! Время полупериода цикла чтения/записи бита.
#define LCD44780_CYCLE_HALF_TIME_US 1
//! Число попыток чтения флага занятости
//! (Время ожидания = число попыток * 
//! (LCD44780_RW_RS_SETUP_TIME_US + LCD44780_CYCLE_HALF_TIME_US * 2))
//! Максимальное время выполнения операции 1.53 мс,
//! Возьмём удвоенное время.
//! Максимальное время выполнения.
#define LCD44780_MAX_EXECUTION_TIME_US 1530
//! Максимальное число попыток.
#define LCD44780_WAIT_BUSY_MAX_COUNT ((LCD44780_MAX_EXECUTION_TIME_US * 2) / (LCD44780_RW_RS_SETUP_TIME_US + LCD44780_CYCLE_HALF_TIME_US * 2))


//! Команды LCD.
//! Очистка.
#define LCD44780_CMD_CLEAR                      1

//! Домой.
#define LCD44780_CMD_HOME                       2

//! Режим движения курсора / дисплея.
#define LCD44780_CMD_ENTRY_MODE                 4
//! Номер бита режима движения курсора.
#define LCD44780_ENTRY_MODE_CURSOR_BIT          1
//! Номер бита режима движения дисплея.
#define LCD44780_ENTRY_MODE_DISPLAY_BIT         0

//! Включение/выключение дисплея/курсора/моргания.
#define LCD44780_CMD_DISPLAY_ON_OFF             8
//! Вкл/Выкл дисплея.
#define LCD44780_ON_OFF_DISPLAY_BIT             2
//! Вкл/Выкл курсора.
#define LCD44780_ON_OFF_CURSOR_BIT              1
//! Вкл/Выкл моргания курсора.
#define LCD44780_ON_OFF_CURSOR_BLINK_BIT        0

//! Сдвиг курсора / дисплея.
#define LCD44780_CMD_CURSOR_DISPLAY_SHIFT       16
//! Бит сдвига курсора/дисплея.
#define LCD44780_SHIFT_DISPLAY_CURSOR_BIT       3
//! Бит сдвига влево/вправо.
#define LCD44780_SHIFT_RIGHT_LEFT_BIT           2

//! Конфигурация дисплея.
#define LCD44780_CMD_FUNCTION_SET               32
//! Ширина шины.
#define LCD44780_FUNCTION_SET_DATA_WIDTH_BIT    4
//! Число линий.
#define LCD44780_FUNCTION_SET_LINE_NUMBER_BIT   3
//! Тип шрифта
#define LCD44780_FUNCTION_SET_FONT_TYPE_BIT     2

//! Установка адреса знакогенератора.
#define LCD44780_CMD_SET_CGRAM_ADDRESS  64
//! Маска адреса.
#define LCD44780_CGRAM_ADDRESS_MASK     0x3f

//! Установка адреса видеопамяти.
#define LCD44780_CMD_SET_DDRAM_ADDRESS  128
//! Маска адреса.
#define LCD44780_DDRAM_ADDRESS_MASK     0x7f

//! Верхние 4 бита команды конфигурации дисплея,
//! предназначенной для перевода его на 4 битную шину.
#define LCD44780_FUNCTION_SET_DATA_WIDTH_4BIT   0x2

//! Маска флага занятости.
#define LCD44780_BUSY_FLAG_MASK         0x80
#define LCD44780_ADDRESS_MASK           0x7f

/**
 * Записывает данные в шину LCD.
 * @param lcd LCD.
 * @param data Байт данных.
 */
static void lcd44780_write_to_bus(lcd44780_t* lcd, uint8_t data)
{
    pin_on(&lcd->pin_e);
    pin_range_set_value(&lcd->pins_data, data);
    delay_us8(LCD44780_CYCLE_HALF_TIME_US);
    pin_off(&lcd->pin_e);
}

/**
 * Записывает байт данных в LCD.
 * @param lcd LCD.
 * @param byte Байт данных.
 */
static void lcd44780_write_byte(lcd44780_t* lcd, uint8_t byte)
{
    //Подождём время установки значений RW/RS.
    delay_us8(LCD44780_RW_RS_SETUP_TIME_US);
    //Если шина 4 бит.
    if(lcd->data_bus_width == LCD44780_DATA_BUS_WIDTH_4BIT){
        lcd44780_write_to_bus(lcd, (byte >> LCD44780_DATA_WIDTH_HALF_BITS));
        delay_us8(LCD44780_CYCLE_HALF_TIME_US);
        lcd44780_write_to_bus(lcd, byte & LCD44780_DATA_WIDTH_HALF_MASK);
    }else{
        lcd44780_write_to_bus(lcd, byte);
    }
}

/**
 * Читает данные из шины LCD.
 * @param lcd LCD.
 * @return Байт данных.
 */
static uint8_t lcd44780_read_from_bus(lcd44780_t* lcd)
{
    pin_on(&lcd->pin_e);
    delay_us8(LCD44780_CYCLE_HALF_TIME_US);
    
    uint8_t data = pin_range_get_value(&lcd->pins_data);
    
    pin_off(&lcd->pin_e);
    
    return data;
}

/**
 * Читает байт данных из LCD.
 * @param lcd LCD.
 * @return Байт данных.
 */
static uint8_t lcd44780_read_byte(lcd44780_t* lcd)
{
    uint8_t byte = 0;
    
    //Подождём время установки значений RW/RS.
    delay_us8(LCD44780_RW_RS_SETUP_TIME_US);
    //Если шина 4 бит.
    if(lcd->data_bus_width == LCD44780_DATA_BUS_WIDTH_4BIT){
        byte  = lcd44780_read_from_bus(lcd) << LCD44780_DATA_WIDTH_HALF_BITS;
        delay_us8(LCD44780_CYCLE_HALF_TIME_US);
        byte |= lcd44780_read_from_bus(lcd);
    }else{
        byte = lcd44780_read_from_bus(lcd);
    }
    
    return byte;
}

/**
 * Начинает запись данных в LCD.
 * @param lcd LCD.
 * @param is_data Пересылаются данные, а не команда.
 */
static void lcd44780_begin_write(lcd44780_t* lcd, bool is_data)
{
    //Установим значение пина RS.
    pin_set_value(&lcd->pin_rs, is_data);
    //Сбросим пин RW.
    pin_off(&lcd->pin_rw);
    //Настроим шину на выход.
    pin_range_set_out(&lcd->pins_data);
}


/**
 * Начинает чтение из LCD.
 * @param lcd LCD.
 * @param is_data Пересылаются данные, а не команда.
 */
static void lcd44780_begin_read(lcd44780_t* lcd, bool is_data)
{
    //Установим значение пина RS.
    pin_set_value(&lcd->pin_rs, is_data);
    //Установим пин RW.
    pin_on(&lcd->pin_rw);
}

/**
 * Завершает обмен данными с LCD.
 * @param lcd LCD.
 */
static void lcd44780_end(lcd44780_t* lcd)
{
    //Сбросим все пины.
    pin_off(&lcd->pin_e);
    pin_off(&lcd->pin_rs);
    pin_off(&lcd->pin_rw);
    //Настроем шину на вход.
    pin_range_set_in(&lcd->pins_data);
    //Включим подтяжку.
    pin_range_pullup_enable(&lcd->pins_data);
}

/**
 * Посылает команду в LCD.
 * @param lcd LCD.
 * @param cmd Команда.
 */
static err_t lcd44780_write_cmd(lcd44780_t* lcd, uint8_t cmd)
{
    if(!lcd44780_wait(lcd)) return E_BUSY;
    
    lcd44780_begin_write(lcd, false);
    lcd44780_write_byte(lcd, cmd);
    lcd44780_end(lcd);
    
    return E_NO_ERROR;
}

/**
 * Посылает данные в LCD.
 * @param lcd LCD.
 * @param data Байт данных.
 */
static err_t lcd44780_write_data(lcd44780_t* lcd, uint8_t data)
{
    if(!lcd44780_wait(lcd)) return E_BUSY;
    
    lcd44780_begin_write(lcd, true);
    lcd44780_write_byte(lcd, data);
    lcd44780_end(lcd);
    
    return E_NO_ERROR;
}

/**
 * Читает команду из LCD.
 * Устанавливает переменную errno.
 * @param lcd LCD.
 * @return Значение адреса + флаг занятости.
 */
/*static uint8_t lcd44780_read_cmd(lcd44780_t* lcd)
{
    errno = E_NO_ERROR;
    if(!lcd44780_wait(lcd)){
        errno = E_BUSY;
        return 0;
    }
    lcd44780_begin_read(lcd, false);
    uint8_t res = lcd44780_read_byte(lcd);
    lcd44780_end(lcd);
    
    return res;
}*/

/**
 * Читает данные из LCD.
 * Устанавливает переменную errno.
 * @param lcd LCD.
 * @return Адрес.
 */
static uint8_t lcd44780_read_data(lcd44780_t* lcd)
{
    errno = E_NO_ERROR;
    if(!lcd44780_wait(lcd)){
        errno = E_BUSY;
        return 0;
    }
    lcd44780_begin_read(lcd, true);
    uint8_t res = lcd44780_read_byte(lcd);
    lcd44780_end(lcd);
    
    return res;
}

static uint8_t lcd44780_read_bf_address(lcd44780_t* lcd)
{
    lcd44780_begin_read(lcd, false);
    uint8_t bf_address = lcd44780_read_byte(lcd);
    lcd44780_end(lcd);
    
    return bf_address;
}

bool lcd44780_busy(lcd44780_t* lcd)
{
    return (lcd44780_read_bf_address(lcd) & LCD44780_BUSY_FLAG_MASK) != 0;
}

bool lcd44780_wait(lcd44780_t* lcd)
{
    uint16_t i;
    for(i = 0; i < LCD44780_WAIT_BUSY_MAX_COUNT; i ++){
        if(!lcd44780_busy(lcd)) return true;
    }
    return false;
}

uint8_t lcd44780_address(lcd44780_t* lcd)
{
    return lcd44780_read_bf_address(lcd) & LCD44780_ADDRESS_MASK;
}

/**
 * Переводит дисплей на 4 битную шину данных.
 * @param lcd LCD.
 */
static void lcd44780_switch_to_4bit(lcd44780_t* lcd)
{
    lcd44780_begin_write(lcd, false);
    delay_us8(LCD44780_RW_RS_SETUP_TIME_US);
    lcd44780_write_to_bus(lcd, LCD44780_FUNCTION_SET_DATA_WIDTH_4BIT);
    lcd44780_end(lcd);
}


err_t lcd44780_init(lcd44780_t* lcd,
                    uint8_t data_port, uint8_t data_offset,
                    lcd44780_data_width_t data_bus_width,
                    uint8_t rs_port, uint8_t rs_pin_n,
                    uint8_t rw_port, uint8_t rw_pin_n,
                    uint8_t e_port, uint8_t e_pin_n)
{
    if(data_bus_width > LCD44780_DATA_BUS_WIDTH_MAX_BIT) return E_INVALID_VALUE;
    
    uint8_t pins_count = (data_bus_width == LCD44780_DATA_BUS_WIDTH_4BIT) ? LCD44780_DATA_WIDTH_HALF_BITS : LCD44780_DATA_WIDTH_FULL_BITS;
    
    lcd->data_bus_width = data_bus_width;
    
    if(data_offset + pins_count > 8) return E_OUT_OF_RANGE;
    
    err_t err = pin_range_init(&lcd->pins_data, data_port, data_offset, pins_count);
    if(err != E_NO_ERROR) return err;
    
    err = pin_init(&lcd->pin_rs, rs_port, rs_pin_n);
    if(err != E_NO_ERROR) return err;
    
    err = pin_init(&lcd->pin_rw, rw_port, rw_pin_n);
    if(err != E_NO_ERROR) return err;
    
    err = pin_init(&lcd->pin_e, e_port, e_pin_n);
    if(err != E_NO_ERROR) return err;
    
    // Настроим пины управления на выход.
    pin_set_out(&lcd->pin_rw);
    pin_set_out(&lcd->pin_rs);
    pin_set_out(&lcd->pin_e);
    
    // Установим состояние после окончания обмена данными.
    lcd44780_end(lcd);
    
    if(lcd->data_bus_width == LCD44780_DATA_BUS_WIDTH_4BIT) lcd44780_switch_to_4bit(lcd);
    
    return lcd44780_wait(lcd) ? E_NO_ERROR : E_BUSY;
}

err_t lcd44780_clear(lcd44780_t* lcd)
{
    return lcd44780_write_cmd(lcd, LCD44780_CMD_CLEAR);
}

err_t lcd44780_home(lcd44780_t* lcd)
{
    return lcd44780_write_cmd(lcd, LCD44780_CMD_HOME);
}

err_t lcd44780_entry_mode(lcd44780_t* lcd, lcd44780_entry_mode_t cursor_mode, lcd44780_entry_mode_t display_mode)
{
    uint8_t cmd = LCD44780_CMD_ENTRY_MODE;
    BIT_SET(cmd, LCD44780_ENTRY_MODE_CURSOR_BIT, cursor_mode);
    BIT_SET(cmd, LCD44780_ENTRY_MODE_DISPLAY_BIT, display_mode);
    return lcd44780_write_cmd(lcd, cmd);
}

err_t lcd44780_control(lcd44780_t* lcd, bool display_on, bool cursor_on, bool cursor_blink_on)
{
    uint8_t cmd = LCD44780_CMD_DISPLAY_ON_OFF;
    BIT_SET(cmd, LCD44780_ON_OFF_DISPLAY_BIT, display_on);
    BIT_SET(cmd, LCD44780_ON_OFF_CURSOR_BIT, cursor_on);
    BIT_SET(cmd, LCD44780_ON_OFF_CURSOR_BLINK_BIT, cursor_blink_on);
    return lcd44780_write_cmd(lcd, cmd);
}

err_t lcd44780_shift(lcd44780_t* lcd, lcd44780_shift_entry_t shift_entry, lcd44780_shift_direction_t shift_direction)
{
    uint8_t cmd = LCD44780_CMD_CURSOR_DISPLAY_SHIFT;
    BIT_SET(cmd, LCD44780_SHIFT_DISPLAY_CURSOR_BIT, shift_entry);
    BIT_SET(cmd, LCD44780_SHIFT_RIGHT_LEFT_BIT, shift_direction);
    return lcd44780_write_cmd(lcd, cmd);
}

err_t lcd44780_configure(lcd44780_t* lcd, lcd44780_line_number_t line_number, lcd44780_font_type_t font_type)
{
    uint8_t cmd = LCD44780_CMD_FUNCTION_SET;
    BIT_SET(cmd, LCD44780_FUNCTION_SET_DATA_WIDTH_BIT, lcd->data_bus_width);
    BIT_SET(cmd, LCD44780_FUNCTION_SET_LINE_NUMBER_BIT, line_number);
    BIT_SET(cmd, LCD44780_FUNCTION_SET_FONT_TYPE_BIT, font_type);
    return lcd44780_write_cmd(lcd, cmd);
}

err_t lcd44780_set_cgram(lcd44780_t* lcd, uint8_t address)
{
    uint8_t cmd = LCD44780_CMD_SET_CGRAM_ADDRESS;
    cmd |= (address & LCD44780_CGRAM_ADDRESS_MASK);
    return lcd44780_write_cmd(lcd, cmd);
}

err_t lcd44780_set_ddram(lcd44780_t* lcd, uint8_t address)
{
    uint8_t cmd = LCD44780_CMD_SET_DDRAM_ADDRESS;
    cmd |= (address & LCD44780_DDRAM_ADDRESS_MASK);
    return lcd44780_write_cmd(lcd, cmd);
}

err_t lcd44780_putc(lcd44780_t* lcd, char c)
{
    return lcd44780_write_data(lcd, (uint8_t)c);
}

char lcd44780_getc(lcd44780_t* lcd)
{
    return (char)lcd44780_read_data(lcd);
}

err_t lcd44780_puts(lcd44780_t* lcd, const char* s)
{
    err_t err = E_NO_ERROR;
    while(*s){
        err = lcd44780_putc(lcd, *s ++);
        if(err != E_NO_ERROR) return err;
    }
    return E_NO_ERROR;
}

err_t lcd44780_write(lcd44780_t* lcd, const char* s, uint8_t size)
{
    err_t err = E_NO_ERROR;
    uint8_t i;
    for(i = 0; i < size; i ++){
        err = lcd44780_putc(lcd, s[i]);
        if(err != E_NO_ERROR) return err;
    }
    return E_NO_ERROR;
}
