#include "lcd0108.h"
#include "defs/defs.h"
#include <util/delay.h>
#include "bits/bits.h"
#include "utils/utils.h"

//! Максимальный адрес в одном контроллере.
#define LCD0108_HALF_ADDRESS_MAX    63
//! Ширина памяти одного контроллера.
#define LCD0108_HALF_WIDTH          64

//! Задержка для установки R/W и RS значений.
//#define LCD0108_RW_RS_SETUP_TIME_US 0
//! Время полупериода цикла чтения/записи бита.
#define LCD0108_CYCLE_HALF_TIME_US  0.45
//! Число попыток чтения флага занятости
//! (Время ожидания = число попыток * 
//! (LCD0108_RW_RS_SETUP_TIME_US + LCD0108_CYCLE_HALF_TIME_US * 2))
//! Максимальное время выполнения операции 1.53 мс,
//! Возьмём удвоенное время.
//! Максимальное время выполнения.
#define LCD0108_MAX_EXECUTION_TIME_US 1000
//! Максимальное число попыток.
#ifdef LCD0108_RW_RS_SETUP_TIME_US
#define LCD0108_WAIT_BUSY_MAX_COUNT ((LCD0108_MAX_EXECUTION_TIME_US * 2) / (LCD0108_RW_RS_SETUP_TIME_US + LCD0108_CYCLE_HALF_TIME_US * 2))
#else
#define LCD0108_WAIT_BUSY_MAX_COUNT ((LCD0108_MAX_EXECUTION_TIME_US * 2) / (LCD0108_CYCLE_HALF_TIME_US * 2))
#endif


//! Команды LCD.
//! Включение/выключение дисплея/курсора/моргания.
#define LCD0108_CMD_DISPLAY_ON_OFF           0x3e
//! Вкл/Выкл дисплея.
#define LCD0108_ON_OFF_DISPLAY_BIT           0

//! Установка страницы.
#define LCD0108_CMD_SET_START_LINE           0xc0
//! Маска X.
#define LCD0108_START_LINE_MASK              0x3f

//! Установка страницы.
#define LCD0108_CMD_SET_PAGE                 0xb8
//! Маска X.
#define LCD0108_PAGE_MASK                    0x7

//! Установка адреса.
#define LCD0108_CMD_SET_ADDRESS              0x40
//! Маска адреса.
#define LCD0108_ADDRESS_MASK                 0x3f

//! Маска флага занятости.
#define LCD0108_BUSY_FLAG_MASK         0x80

/**
 * Записывает данные в шину LCD.
 * @param lcd LCD.
 * @param data Байт данных.
 */
ALWAYS_INLINE static void lcd0108_write_to_bus(lcd0108_t* lcd, uint8_t data)
{
    pin_on(&lcd->pin_e);
    port_set_value(&lcd->port_data, data);
    _delay_us(LCD0108_CYCLE_HALF_TIME_US);
    pin_off(&lcd->pin_e);
}

/**
 * Записывает байт данных в LCD.
 * @param lcd LCD.
 * @param byte Байт данных.
 */
static void lcd0108_write_byte(lcd0108_t* lcd, uint8_t byte)
{
    //Подождём время установки значений RW/RS.
#ifdef LCD0108_RW_RS_SETUP_TIME_US
    _delay_us(LCD0108_RW_RS_SETUP_TIME_US);
#endif
    lcd0108_write_to_bus(lcd, byte);
}

/**
 * Читает данные из шины LCD.
 * @param lcd LCD.
 * @return Байт данных.
 */
ALWAYS_INLINE static uint8_t lcd0108_read_from_bus(lcd0108_t* lcd)
{
    pin_on(&lcd->pin_e);
    _delay_us(LCD0108_CYCLE_HALF_TIME_US);
    
    uint8_t data = port_get_value(&lcd->port_data);
    
    pin_off(&lcd->pin_e);
    
    return data;
}

/**
 * Читает байт данных из LCD.
 * @param lcd LCD.
 * @return Байт данных.
 */
static uint8_t lcd0108_read_byte(lcd0108_t* lcd)
{
    uint8_t byte = 0;
    
    //Подождём время установки значений RW/RS.
#ifdef LCD0108_RW_RS_SETUP_TIME_US
    _delay_us(LCD0108_RW_RS_SETUP_TIME_US);
#endif
    byte = lcd0108_read_from_bus(lcd);
    
    return byte;
}

/**
 * Активирует контроллеры.
 * @param lcd LCD.
 * @param cs0 Активация первого контроллера.
 * @param cs1 Активация второго контроллера.
 */
static void lcd0108_select_chips(lcd0108_t* lcd, bool cs0, bool cs1)
{
    if(lcd->inverse_cs){
        cs0 = !cs0;
        cs1 = !cs1;
    }
    //Первый контроллер.
    pin_set_value(&lcd->pin_cs0, cs0);
    //Второй контроллер.
    pin_set_value(&lcd->pin_cs1, cs1);
}

/**
 * Начинает запись данных в LCD.
 * @param lcd LCD.
 * @param is_data Пересылаются данные, а не команда.
 * @param cs0 Активация первого контроллера.
 * @param cs1 Активация второго контроллера.
 */
static void lcd0108_begin_write(lcd0108_t* lcd, bool is_data, bool cs0, bool cs1)
{
    //Контроллеры.
    lcd0108_select_chips(lcd, cs0, cs1);
    //Установим значение пина RS.
    pin_set_value(&lcd->pin_rs, is_data);
    //Сбросим пин RW.
    pin_off(&lcd->pin_rw);
    //Настроим шину на выход.
    port_set_out(&lcd->port_data);
}


/**
 * Начинает чтение из LCD.
 * @param lcd LCD.
 * @param is_data Пересылаются данные, а не команда.
 * @param cs0 Активация первого контроллера.
 * @param cs1 Активация второго контроллера.
 */
static void lcd0108_begin_read(lcd0108_t* lcd, bool is_data, bool cs0, bool cs1)
{
    //Контроллеры.
    lcd0108_select_chips(lcd, cs0, cs1);
    //Установим значение пина RS.
    pin_set_value(&lcd->pin_rs, is_data);
    //Установим пин RW.
    pin_on(&lcd->pin_rw);
}

/**
 * Завершает обмен данными с LCD.
 * @param lcd LCD.
 */
static void lcd0108_end(lcd0108_t* lcd)
{
    //Сбросим все пины.
    lcd0108_select_chips(lcd, false, false);
    pin_off(&lcd->pin_e);
    pin_off(&lcd->pin_rs);
    pin_off(&lcd->pin_rw);
    //Настроем шину на вход.
    port_set_in(&lcd->port_data);
    //Включим подтяжку.
    port_pullup_enable(&lcd->port_data);
}

/**
 * Посылает команду в LCD.
 * @param lcd LCD.
 * @param cmd Команда.
 * @param cs0 Активация первого контроллера.
 * @param cs1 Активация второго контроллера.
 */
static err_t lcd0108_write_cmd(lcd0108_t* lcd, uint8_t cmd, bool cs0, bool cs1)
{
    if(!lcd0108_wait(lcd)) return E_BUSY;
    
    lcd0108_begin_write(lcd, false, cs0, cs1);
    lcd0108_write_byte(lcd, cmd);
    lcd0108_end(lcd);
    
    return E_NO_ERROR;
}

/**
 * Посылает данные в LCD.
 * @param lcd LCD.
 * @param data Байт данных.
 * @param cs0 Активация первого контроллера.
 * @param cs1 Активация второго контроллера.
 */
static err_t lcd0108_write_data(lcd0108_t* lcd, uint8_t data, bool cs0, bool cs1)
{
    if(!lcd0108_wait(lcd)) return E_BUSY;
    
    lcd0108_begin_write(lcd, true, cs0, cs1);
    lcd0108_write_byte(lcd, data);
    lcd0108_end(lcd);
    
    return E_NO_ERROR;
}

/**
 * Читает данные из LCD.
 * @param lcd LCD.
 * @param cs0 Активация первого контроллера.
 * @param cs1 Активация второго контроллера.
 * @return Адрес.
 */
static err_t lcd0108_read_data(lcd0108_t* lcd, uint8_t* data, bool cs0, bool cs1)
{
    if(!lcd0108_wait(lcd)) return E_BUSY;
    
    lcd0108_begin_read(lcd, true, cs0, cs1);
    *data = lcd0108_read_byte(lcd);
    lcd0108_end(lcd);
    
    return E_NO_ERROR;
}

/**
 * Читает статус LCD.
 * @param lcd LCD.
 * @param cs0 Активация первого контроллера.
 * @param cs1 Активация второго контроллера.
 * @return Статус.
 */
static uint8_t lcd0108_read_status(lcd0108_t* lcd, bool cs0, bool cs1)
{
    lcd0108_begin_read(lcd, false, cs0, cs1);
    uint8_t res = lcd0108_read_byte(lcd);
    lcd0108_end(lcd);
    
    return res;
}

bool lcd0108_busy(lcd0108_t* lcd)
{
    if(lcd0108_read_status(lcd, true, false) & LCD0108_BUSY_FLAG_MASK) return true;
    return (lcd0108_read_status(lcd, false, true) & LCD0108_BUSY_FLAG_MASK) != 0;
}

bool lcd0108_wait(lcd0108_t* lcd)
{
    uint16_t i;
    for(i = 0; i < LCD0108_WAIT_BUSY_MAX_COUNT; i ++){
        if(!lcd0108_busy(lcd)) return true;
    }
    return false;
}

err_t lcd0108_init(lcd0108_t* lcd,
                    bool inverse_cs, uint8_t data_port,
                    uint8_t reset_port, uint8_t reset_pin_n,
                    uint8_t cs0_port, uint8_t cs0_pin_n,
                    uint8_t cs1_port, uint8_t cs1_pin_n,
                    uint8_t rs_port, uint8_t rs_pin_n,
                    uint8_t rw_port, uint8_t rw_pin_n,
                    uint8_t e_port, uint8_t e_pin_n)
{
    lcd->inverse_cs = inverse_cs;
    
    err_t err = port_init(&lcd->port_data, data_port);
    if(err != E_NO_ERROR) return err;
    
    err = pin_init(&lcd->pin_reset, reset_port, reset_pin_n);
    if(err != E_NO_ERROR) return err;
    
    err = pin_init(&lcd->pin_cs0, cs0_port, cs0_pin_n);
    if(err != E_NO_ERROR) return err;
    
    err = pin_init(&lcd->pin_cs1, cs1_port, cs1_pin_n);
    if(err != E_NO_ERROR) return err;
    
    err = pin_init(&lcd->pin_rs, rs_port, rs_pin_n);
    if(err != E_NO_ERROR) return err;
    
    err = pin_init(&lcd->pin_rw, rw_port, rw_pin_n);
    if(err != E_NO_ERROR) return err;
    
    err = pin_init(&lcd->pin_e, e_port, e_pin_n);
    if(err != E_NO_ERROR) return err;
    
    // Настроим пины управления на выход.
    pin_set_out(&lcd->pin_cs0);
    pin_set_out(&lcd->pin_cs1);
    pin_set_out(&lcd->pin_rw);
    pin_set_out(&lcd->pin_rs);
    pin_set_out(&lcd->pin_e);
    // Пин ресета.
    pin_set_in(&lcd->pin_reset);
    // Включим подтяжку.
    pin_pullup_enable(&lcd->pin_reset);
    
    // Установим состояние после окончания обмена данными.
    lcd0108_end(lcd);

    return lcd0108_wait(lcd) ? E_NO_ERROR : E_BUSY;
}

err_t lcd0108_control(lcd0108_t* lcd, bool display_on)
{
    uint8_t cmd = LCD0108_CMD_DISPLAY_ON_OFF;
    BIT_SET(cmd, LCD0108_ON_OFF_DISPLAY_BIT, display_on);
    return lcd0108_write_cmd(lcd, cmd, true, true);
}

err_t lcd0108_set_start_line(lcd0108_t* lcd, uint8_t start_line)
{
    uint8_t cmd = LCD0108_CMD_SET_START_LINE;
    cmd |= (start_line & LCD0108_START_LINE_MASK);
    return lcd0108_write_cmd(lcd, cmd, true, true);
}

/**
 * Устанавливает страницу.
 * @param lcd LCD.
 * @param page Страница.
 * @param cs0 Активация первого контроллера.
 * @param cs1 Активация второго контроллера.
 */
static err_t lcd0108_set_page(lcd0108_t* lcd, uint8_t page, bool cs0, bool cs1)
{
    uint8_t cmd = LCD0108_CMD_SET_PAGE;
    cmd |= (page & LCD0108_PAGE_MASK);
    return lcd0108_write_cmd(lcd, cmd, cs0, cs1);
}

/**
 * Устанавливает адрес.
 * @param lcd LCD.
 * @param address Адрес.
 * @param cs0 Активация первого контроллера.
 * @param cs1 Активация второго контроллера.
 */
static err_t lcd0108_set_address(lcd0108_t* lcd, uint8_t address, bool cs0, bool cs1)
{
    uint8_t cmd = LCD0108_CMD_SET_ADDRESS;
    cmd |= (address & LCD0108_ADDRESS_MASK);
    return lcd0108_write_cmd(lcd, cmd, cs0, cs1);
}

/**
 * Устанавливает страницу и адрес в странице LCD.
 * @param lcd LCD.
 * @param page Страница.
 * @param address Адрес.
 * @return Код ошибки.
 */
/*static err_t lcd0108_goto(lcd0108_t* lcd, uint8_t page, uint8_t address)
{
    bool cs1 = false;
    if(address > LCD0108_HALF_ADDRESS_MAX){
        cs1 = true;
        address -= LCD0108_HALF_WIDTH;
    }
    bool cs0 = !cs1;
    
    err_t err = lcd0108_set_page(lcd, page, cs0, cs1);
    if(err != E_NO_ERROR) return err;
    
    err = lcd0108_set_address(lcd, address, cs0, cs1);
    if(err != E_NO_ERROR) return err;
    
    return E_NO_ERROR;
}*/

err_t lcd0108_rw(lcd0108_t* lcd, bool is_read, uint8_t page, uint8_t address, uint8_t* data, size_t size)
{
    if(page > LCD0108_PAGE_MAX || address > LCD0108_ADDRESS_MAX) return E_OUT_OF_RANGE;
    
    err_t err = E_NO_ERROR;
    
    bool page_sel = false;
    bool addr_sel = false;
    bool cs0 = false;
    bool cs1 = false;
    
    size_t i;
    for(i = 0; i < size; i ++){
        if(!addr_sel){
            if(address > LCD0108_HALF_ADDRESS_MAX){
                cs0 = false; cs1 = true;
                address -= LCD0108_HALF_WIDTH;
            }else{
                cs0 = true; cs1 = false;
            }
            err = lcd0108_set_address(lcd, address, cs0, cs1);
            if(err != E_NO_ERROR) return err;
            
            addr_sel = true;
        }
        if(!page_sel){
            err = lcd0108_set_page(lcd, page, true, true);
            if(err != E_NO_ERROR) return err;
            
            page_sel = true;
        }
        
        if(is_read) lcd0108_read_data(lcd, &data[i], cs0, cs1);
        else        lcd0108_write_data(lcd, data[i], cs0, cs1);
        
        address ++;
        
        if(address > LCD0108_HALF_ADDRESS_MAX){
            
            addr_sel = false;
            
            if(cs1){
                page_sel = false;
                
                page ++;
                address = 0;
            }
        }
        
        if(page > LCD0108_PAGE_MAX) break;
    }
    
    return E_NO_ERROR;
}

err_t lcd0108_rw_block(lcd0108_t* lcd, bool is_read, uint8_t page, uint8_t address, uint8_t* data, uint8_t width, uint8_t height, uint8_t data_line_size)
{
    if(page > LCD0108_PAGE_MAX || address > LCD0108_ADDRESS_MAX) return E_OUT_OF_RANGE;
    if(width == 0 || height == 0) return E_INVALID_VALUE;
    
    uint8_t page_end = height + page;
    page_end = MIN(LCD0108_PAGES_HEIGHT, page_end);
    
    uint8_t avail_width = LCD0108_WIDTH - address;
    avail_width = MIN(width, avail_width);
    
    err_t err;
    uint8_t i;
    for(i = page; i < page_end; i ++){
        err = lcd0108_rw(lcd, is_read, i, address, data, avail_width);
        if(err != E_NO_ERROR) return err;
        
        data += data_line_size;
    }
    
    return E_NO_ERROR;
}

err_t lcd0108_write(lcd0108_t* lcd, uint8_t page, uint8_t address, const uint8_t* data, size_t size)
{
    return lcd0108_rw(lcd, false, page, address, (uint8_t*)data, size);
}

err_t lcd0108_write_block(lcd0108_t* lcd, uint8_t page, uint8_t address, const uint8_t* data, uint8_t width, uint8_t height, uint8_t data_line_size)
{
    return lcd0108_rw_block(lcd, false, page, address, (uint8_t*)data, width, height, data_line_size);
}

err_t lcd0108_read(lcd0108_t* lcd, uint8_t page, uint8_t address, const uint8_t* data, size_t size)
{
    return lcd0108_rw(lcd, true, page, address, (uint8_t*)data, size);
}

err_t lcd0108_read_block(lcd0108_t* lcd, uint8_t page, uint8_t address, const uint8_t* data, uint8_t width, uint8_t height, uint8_t data_line_size)
{
    return lcd0108_rw_block(lcd, true, page, address, (uint8_t*)data, width, height, data_line_size);
}

err_t lcd0108_clear(lcd0108_t* lcd)
{
    err_t err;
    
    uint8_t page, address;
    
    for(page = 0; page <= LCD0108_PAGE_MAX; page ++){
        
        err = lcd0108_set_address(lcd, 0, true, true);
        if(err != E_NO_ERROR) return err;
        
        err = lcd0108_set_page(lcd, page, true, true);
        if(err != E_NO_ERROR) return err;
        
        for(address = 0; address <= LCD0108_HALF_ADDRESS_MAX; address ++){
            err = lcd0108_write_data(lcd, 0, true, true);
        }
    }
    
    return E_NO_ERROR;
}
