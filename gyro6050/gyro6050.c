#include "gyro6050.h"
#include "future/future.h"
#include "utils/utils.h"
#include "counter/counter.h"
#include "cordic/cordic10_6.h"
#include <string.h>


//! Адрес делителя частоты вывода.
#define GYRO6050_SMPRT_DIV_ADDRESS      25

//! Адрес регистра конфигурации EXT_SYNC и DLPF.
#define GYRO6050_CONTROL_ADDRESS        26

//! Адрес регистра конфигурации гироскопа.
#define GYRO6050_GYRO_CONTROL_ADDRESS   27
//! Смещение значения диапазона данных гироскопа.
#define GYRO6050_GYRO_CONTROL_FS_SEL_OFFSET     3

//! Адрес регистра конфигурации акселерометра.
#define GYRO6050_ACCEL_CONTROL_ADDRESS  28
//! Смещение значения диапазона данных акселерометра.
#define GYRO6050_ACCEL_CONTROL_FS_SEL_OFFSET    3

//! Адрес регистра конфигурации пина сигнала о прерываниях.
#define GYRO6050_INT_PIN_CFG_ADDRESS    55

//! Адрес регистра конфигурации сигнала о прерываниях.
#define GYRO6050_INT_CFG_ADDRESS        56

//! Адрес начала данных
//! подряд:
//! Акселерометр -> температура -> гироскоп.
#define GYRO6050_ACCEL_TEMP_GYRO_DATA_ADDRESS      59
//! Адрес начала данных акселерометра.
#define GYRO6050_ACCEL_DATA_ADDRESS                59
//! Адрес начала данных температуры.
#define GYRO6050_TEMP_DATA_ADDRESS                 65
//! Адрес начала данных гироскопа.
#define GYRO6050_GYRO_DATA_ADDRESS                 67

//! Первый регистр управления питанием.
#define GYRO6050_PWR_MNGT_1_ADDRESS     107
//! Бит сна.
#define GYRO6050_PWR_MNGT_1_SLEEP_BIT   6

//! Второй регистр управления питанием.
#define GYRO6050_PWR_MNGT_2_ADDRESS     108


//! Максимальные значения.
#define GYRO6050_DLPF_MAX               GYRO6050_DLPF_A5HZ_G5HZ
#define GYRO6050_GYRO_SCALE_RANGE_MAX   GYRO6050_GYRO_SCALE_RANGE_2000_DPS
#define GYRO6050_ACCEL_SCALE_RANGE_MAX  GYRO6050_ACCEL_SCALE_RANGE_16G

/*
//! Направление обмена данными.
//! Чтение.
#define GYRO6050_READ   0
//! Запись.
#define GYRO6050_WRITE  1
//! Тип направления обмена данными.
typedef uint8_t gyro6050_io_direction_t;
*/

#pragma pack(push, 1)
typedef union _Int16hl{
    struct {
        int8_t l;
        int8_t h;
    } parts;
    int16_t value;
} int16hl_t;
#pragma pack(pop)

//! Сырые данные акселерометра.
#pragma pack(push, 1)
typedef struct _Gyro6050AccelData{
    int16hl_t accel_x;
    int16hl_t accel_y;
    int16hl_t accel_z;
}gyro6050_accel_data_t;
#pragma pack(pop)

//! Сырые данные температуры.
#pragma pack(push, 1)
typedef struct _Gyro6050TempData{
    int16hl_t temp;
}gyro6050_temp_data_t;
#pragma pack(pop)

//! Сырые данные гироскопа.
#pragma pack(push, 1)
typedef struct _Gyro6050GyroData{
    int16hl_t gyro_x;
    int16hl_t gyro_y;
    int16hl_t gyro_z;
}gyro6050_gyro_data_t;
#pragma pack(pop)

//! Сырые данные датчика.
#pragma pack(push, 1)
typedef struct _Gyro6050RawData{
    gyro6050_accel_data_t accel_data;
    gyro6050_temp_data_t temp_data;
    gyro6050_gyro_data_t gyro_data;
}gyro6050_raw_data_t;
#pragma pack(pop)

//! Преобразованные данные гироскопа, акселерометра и температуры.
typedef struct _Gyro6050Data{
    fixed10_6_t accel_x;
    fixed10_6_t accel_y;
    fixed10_6_t accel_z;
    
    fixed10_6_t temp;
    
    fixed10_6_t gyro_w_x;
    fixed10_6_t gyro_w_y;
    fixed10_6_t gyro_w_z;
}gyro6050_data_t;

//! Кэшированные конфигурационные данные.
typedef struct _Gyro6050CachedData{
    //! Диапазон данных гироскопа.
    gyro6050_gyro_scale_range_t gyro_scale_range;
    //! Диапазон данных акселерометра.
    gyro6050_accel_scale_range_t accel_scale_range;
    //! Делитель частоты вывода.
    uint8_t rate_divisor;
    //! dlpf.
    uint8_t dlpf;
    //! Конфигурация пина сигнала о прерываниях.
    gyro6050_int_pin_conf_t int_pin_config;
    //! Конфигурация сигнала о прерываниях.
    gyro6050_int_conf_t int_config;
}gyro6050_cached_data_t;

//! Структура состояния гироскопа.
typedef struct _Gyro5060{
    //! Адрес i2c.
    i2c_address_t i2c_address;
    //! Идентификато передачи i2c.
    i2c_transfer_id_t i2c_transfer_id;
    //! Адрес страницы i2c.
    uint8_t i2c_page_address;
    //! Будущее.
    future_t future;
    //! Состояние (текущее действие).
    uint8_t state;
    //! Вычисленные данные.
    gyro6050_data_t data;
    //! Кэшированные данные.
    gyro6050_cached_data_t cached_data;
    //! Сырые данные.
    gyro6050_raw_data_t raw_data;
    //! Сырые калиброванные данные гироскопа.
    gyro6050_gyro_data_t calibrated_gyro_data;
    //! Число калибровочных данных.
    uint8_t calibrations_count;
    //! Флаг новых данных.
    bool new_data_avail;
    //! Байт данных для обмена.
    uint8_t data_byte;
}gyro5060_t;

//! Состояние гироскопа.
static gyro5060_t gyro;

//! Константы.
// --

//! Состояния гироскопа (текущие действия).
//! Бездействие.
#define GYRO6050_STATE_IDLE                     0
//! Чтение делителя частоты.
#define GYRO6050_STATE_RATE_DIVISOR_READ        1
//! Запись делителя частоты.
#define GYRO6050_STATE_RATE_DIVISOR_WRITE       2
//! Чтение DLPF.
#define GYRO6050_STATE_DLPF_READ                3
//! Запись DLPF.
#define GYRO6050_STATE_DLPF_WRITE               4
//! Чтение диапазона данных гироскопа.
#define GYRO6050_STATE_GYRO_RANGE_READ          5
//! Запись диапазона данных гироскопа.
#define GYRO6050_STATE_GYRO_RANGE_WRITE         6
//! Чтение диапазона данных акселерометра.
#define GYRO6050_STATE_ACCEL_RANGE_READ         7
//! Запись диапазона данных акселерометра.
#define GYRO6050_STATE_ACCEL_RANGE_WRITE        8
//! Запись бита sleep.
#define GYRO6050_STATE_SLEEP_WRITE              9
//! Запись конфигурации пина сигнала о прерываниях.
#define GYRO6050_STATE_INT_PIN_CONF_WRITE       10
//! Чтение конфигурации пина сигнала о прерываниях.
#define GYRO6050_STATE_INT_PIN_CONF_READ        11
//! Запись конфигурации сигнала о прерываниях.
#define GYRO6050_STATE_INT_CONF_WRITE           12
//! Чтение конфигурации сигнала о прерываниях.
#define GYRO6050_STATE_INT_CONF_READ            13
//! Чтение данных акселерометра и гироскопа.
#define GYRO6050_STATE_DATA_READ                14
//! Чтение данных для калибровки гироскопа.
#define GYRO6050_STATE_CALIBRATION_READ         15


static void gyro6050_do(void);


bool gyro6050_i2c_callback(void)
{
    if(i2c_transfer_id() != gyro.i2c_transfer_id) return false;
    
    gyro6050_do();
    
    return true;
}

/**
 * Ждёт завершения текущей операции.
 * @return true, если шина i2c занята нами и мы дождались, иначе false.
 */
static bool gyro6050_wait_current_op(void)
{
    // Если шина занята не нами - возврат ошибки занятости.
    if(i2c_busy() && i2c_transfer_id() != gyro.i2c_transfer_id) return false;
    // Подождём выполнения предыдущей операции.
    gyro6050_wait();
    return true;
}

static void gyro6050_start(void)
{
    // Установим идентификатор передачи.
    i2c_set_transfer_id(gyro.i2c_transfer_id);
    // Запустим будущее.
    future_start(&gyro.future);
}

static void gyro6050_end(err_t err)
{
    //! Остановим будущее.
    future_finish(&gyro.future, int_to_pvoid(err));
}

static void gyro6050_do(void)
{
    i2c_status_t status = i2c_status();
    switch(gyro.state){
        default:
        case GYRO6050_STATE_IDLE:
            gyro6050_end(E_NO_ERROR);
            break;
        case GYRO6050_STATE_SLEEP_WRITE:
            if(status == I2C_STATUS_DATA_WRITED){
                gyro6050_end(E_NO_ERROR);
            }else{
                gyro6050_end(E_IO_ERROR);
            }
            break;
        case GYRO6050_STATE_INT_PIN_CONF_WRITE:
        case GYRO6050_STATE_INT_PIN_CONF_READ:
            if(status == I2C_STATUS_DATA_READED ||
               status == I2C_STATUS_DATA_WRITED){
                gyro.cached_data.int_pin_config = gyro.data_byte;
                gyro6050_end(E_NO_ERROR);
            }else{
                gyro6050_end(E_IO_ERROR);
            }
            break;
        case GYRO6050_STATE_INT_CONF_WRITE:
        case GYRO6050_STATE_INT_CONF_READ:
            if(status == I2C_STATUS_DATA_READED ||
               status == I2C_STATUS_DATA_WRITED){
                gyro.cached_data.int_config = gyro.data_byte;
                gyro6050_end(E_NO_ERROR);
            }else{
                gyro6050_end(E_IO_ERROR);
            }
            break;
        case GYRO6050_STATE_RATE_DIVISOR_READ:
        case GYRO6050_STATE_RATE_DIVISOR_WRITE:
            if(status == I2C_STATUS_DATA_READED ||
               status == I2C_STATUS_DATA_WRITED){
                gyro.cached_data.rate_divisor = gyro.data_byte;
                gyro6050_end(E_NO_ERROR);
            }else{
                gyro6050_end(E_IO_ERROR);
            }
            break;
        case GYRO6050_STATE_DLPF_READ:
        case GYRO6050_STATE_DLPF_WRITE:
            if(status == I2C_STATUS_DATA_READED ||
               status == I2C_STATUS_DATA_WRITED){
                gyro.cached_data.dlpf = gyro.data_byte & 0x7;
                gyro6050_end(E_NO_ERROR);
            }else{
                gyro6050_end(E_IO_ERROR);
            }
            break;
        case GYRO6050_STATE_GYRO_RANGE_READ:
        case GYRO6050_STATE_GYRO_RANGE_WRITE:
            if(status == I2C_STATUS_DATA_READED ||
               status == I2C_STATUS_DATA_WRITED){
                gyro.cached_data.gyro_scale_range = (gyro.data_byte >> GYRO6050_GYRO_CONTROL_FS_SEL_OFFSET) & 0x3;
                gyro6050_end(E_NO_ERROR);
            }else{
                gyro6050_end(E_IO_ERROR);
            }
            break;
        case GYRO6050_STATE_ACCEL_RANGE_READ:
        case GYRO6050_STATE_ACCEL_RANGE_WRITE:
            if(status == I2C_STATUS_DATA_READED ||
               status == I2C_STATUS_DATA_WRITED){
                gyro.cached_data.accel_scale_range = (gyro.data_byte >> GYRO6050_ACCEL_CONTROL_FS_SEL_OFFSET) & 0x3;
                gyro6050_end(E_NO_ERROR);
            }else{
                gyro6050_end(E_IO_ERROR);
            }
            break;
        case GYRO6050_STATE_DATA_READ:
            if(status == I2C_STATUS_DATA_READED){
                gyro.new_data_avail = true;
                gyro6050_end(E_NO_ERROR);
            }else{
                gyro6050_end(E_IO_ERROR);
            }
            break;
        case GYRO6050_STATE_CALIBRATION_READ:
            if(status == I2C_STATUS_DATA_READED){
                gyro.new_data_avail = true;
                gyro6050_end(E_NO_ERROR);
            }else{
                gyro6050_end(E_IO_ERROR);
            }
            break;
    }
    gyro.state = GYRO6050_STATE_IDLE;
}

static err_t gyro6050_write_data(uint8_t state, uint8_t page_address, const void* data, uint8_t data_size)
{
    gyro6050_start();
    
    gyro.state = state;
    
    gyro.i2c_page_address = page_address;
    err_t err = i2c_master_write_at(gyro.i2c_address, &gyro.i2c_page_address, 1, data, data_size);
    if(err != E_NO_ERROR){
        gyro6050_end(err);
    }
    return err;
}

static err_t gyro6050_read_data(uint8_t state, uint8_t page_address, void* data, uint8_t data_size)
{
    gyro6050_start();
    
    gyro.state = state;
    
    gyro.i2c_page_address = page_address;
    err_t err = i2c_master_read_at(gyro.i2c_address, &gyro.i2c_page_address, 1, data, data_size);
    if(err != E_NO_ERROR){
        gyro6050_end(err);
    }
    return err;
}

err_t gyro6050_init(i2c_address_t address)
{
    gyro.i2c_address = address;
    gyro.i2c_transfer_id = GYRO6050_DEFAULT_I2C_TRANSFER_ID;
    gyro.i2c_page_address = 0;
    
    gyro.state = GYRO6050_STATE_IDLE;
    
    gyro.data_byte = 0;
    
    gyro.new_data_avail = false;
    
    gyro.calibrations_count = 0;
    
    future_init(&gyro.future);
    
    memset(&gyro.calibrated_gyro_data, 0x0, sizeof(gyro6050_gyro_data_t));
    memset(&gyro.cached_data, 0x0, sizeof(gyro6050_cached_data_t));
    memset(&gyro.raw_data, 0x0, sizeof(gyro6050_raw_data_t));
    memset(&gyro.data, 0x0, sizeof(gyro6050_data_t));
    
    return E_NO_ERROR;
}

bool gyro6050_done(void)
{
    return future_done(&gyro.future);
}

err_t gyro6050_wait(void)
{
    future_wait(&gyro.future);
    return gyro6050_error();
}

bool gyro6050_busy(void)
{
    return future_running(&gyro.future);
}

err_t gyro6050_error(void)
{
    return pvoid_to_int(err_t, future_result(&gyro.future));
}

i2c_transfer_id_t gyro6050_i2c_transfer_id(void)
{
    return gyro.i2c_transfer_id;
}

void gyro6050_i2c_set_transfer_id(i2c_transfer_id_t transfer_id)
{
    gyro.i2c_transfer_id = transfer_id;
}

err_t gyro6050_read_rate_divisor(void)
{
    if(!gyro6050_wait_current_op()) return E_BUSY;
    return gyro6050_read_data(GYRO6050_STATE_RATE_DIVISOR_READ, GYRO6050_SMPRT_DIV_ADDRESS, &gyro.data_byte, 1);
}

uint8_t gyro6050_rate_divisor(void)
{
    return gyro.cached_data.rate_divisor;
}

err_t gyro6050_set_rate_divisor(uint8_t divisor)
{
    if(!gyro6050_wait_current_op()) return E_BUSY;
    gyro.data_byte = divisor;
    return gyro6050_write_data(GYRO6050_STATE_RATE_DIVISOR_WRITE, GYRO6050_SMPRT_DIV_ADDRESS, &gyro.data_byte, 1);
}

err_t gyro6050_read_dlpf(void)
{
    if(!gyro6050_wait_current_op()) return E_BUSY;
    return gyro6050_read_data(GYRO6050_STATE_DLPF_READ, GYRO6050_CONTROL_ADDRESS, &gyro.data_byte, 1);
}

uint8_t gyro6050_dlpf(void)
{
    return gyro.cached_data.dlpf;
}

err_t gyro6050_set_dlpf(gyro6050_dlpf_t dlpf)
{
    if(dlpf > GYRO6050_DLPF_MAX) return E_INVALID_VALUE;
    if(!gyro6050_wait_current_op()) return E_BUSY;
    gyro.data_byte = dlpf;
    return gyro6050_write_data(GYRO6050_STATE_DLPF_WRITE, GYRO6050_CONTROL_ADDRESS, &gyro.data_byte, 1);
}

err_t gyro6050_read_gyro_scale_range(void)
{
    if(!gyro6050_wait_current_op()) return E_BUSY;
    return gyro6050_read_data(GYRO6050_STATE_GYRO_RANGE_READ, GYRO6050_GYRO_CONTROL_ADDRESS, &gyro.data_byte, 1);
}

gyro6050_gyro_scale_range_t gyro6050_gyro_scale_range(void)
{
    return gyro.cached_data.gyro_scale_range;
}

err_t gyro6050_set_gyro_scale_range(gyro6050_gyro_scale_range_t range)
{
    if(range > GYRO6050_GYRO_SCALE_RANGE_MAX) return E_INVALID_VALUE;
    if(!gyro6050_wait_current_op()) return E_BUSY;
    gyro.data_byte = range << GYRO6050_GYRO_CONTROL_FS_SEL_OFFSET;
    return gyro6050_write_data(GYRO6050_STATE_GYRO_RANGE_WRITE, GYRO6050_GYRO_CONTROL_ADDRESS, &gyro.data_byte, 1);
}

err_t gyro6050_read_accel_scale_range(void)
{
    if(!gyro6050_wait_current_op()) return E_BUSY;
    return gyro6050_read_data(GYRO6050_STATE_ACCEL_RANGE_READ, GYRO6050_ACCEL_CONTROL_ADDRESS, &gyro.data_byte, 1);
}

gyro6050_accel_scale_range_t gyro6050_accel_scale_range(void)
{
    return gyro.cached_data.accel_scale_range;
}

err_t gyro6050_set_accel_scale_range(gyro6050_accel_scale_range_t range)
{
    if(range > GYRO6050_ACCEL_SCALE_RANGE_MAX) return E_INVALID_VALUE;
    if(!gyro6050_wait_current_op()) return E_BUSY;
    gyro.data_byte = range << GYRO6050_ACCEL_CONTROL_FS_SEL_OFFSET;
    return gyro6050_write_data(GYRO6050_STATE_ACCEL_RANGE_WRITE, GYRO6050_ACCEL_CONTROL_ADDRESS, &gyro.data_byte, 1);
}

/*
#define GYRO6050__ADDRESS 0
#define GYRO6050__MAX   0
err_t gyro6050_read_(void)
{
    if(!gyro6050_wait_current_op()) return E_BUSY;
    return gyro6050_read_data(GYRO6050_STATE_IDLE, GYRO6050__ADDRESS, &gyro.data_byte, 1);
}

uint8_t gyro6050_(void)
{
    return gyro.data_byte;
}

err_t gyro6050_set_(uint8_t data)
{
    if(data > GYRO6050__MAX) return E_INVALID_VALUE;
    if(!gyro6050_wait_current_op()) return E_BUSY;
    gyro.data_byte = data;
    return gyro6050_write_data(GYRO6050_STATE_IDLE, GYRO6050__ADDRESS, &gyro.data_byte, 1);
}
 */

err_t gyro6050_int_pin_configure(gyro6050_int_pin_conf_t conf)
{
    if(!gyro6050_wait_current_op()) return E_BUSY;
    gyro.data_byte = conf;
    return gyro6050_write_data(GYRO6050_STATE_INT_PIN_CONF_WRITE, GYRO6050_INT_PIN_CFG_ADDRESS, &gyro.data_byte, 1);
}

err_t gyro6050_read_int_pin_config(void)
{
    if(!gyro6050_wait_current_op()) return E_BUSY;
    return gyro6050_read_data(GYRO6050_STATE_INT_PIN_CONF_READ, GYRO6050_INT_PIN_CFG_ADDRESS, &gyro.data_byte, 1);
}

gyro6050_int_pin_conf_t gyro6050_int_pin_config(void)
{
    return gyro.cached_data.int_pin_config;
}

err_t gyro6050_int_configure(gyro6050_int_conf_t conf)
{
    if(!gyro6050_wait_current_op()) return E_BUSY;
    gyro.data_byte = conf;
    return gyro6050_write_data(GYRO6050_STATE_INT_CONF_WRITE, GYRO6050_INT_CFG_ADDRESS, &gyro.data_byte, 1);
}

err_t gyro6050_read_int_config(void)
{
    if(!gyro6050_wait_current_op()) return E_BUSY;
    return gyro6050_read_data(GYRO6050_STATE_INT_CONF_READ, GYRO6050_INT_CFG_ADDRESS, &gyro.data_byte, 1);
}

gyro6050_int_conf_t gyro6050_int_config(void)
{
    return gyro.cached_data.int_config;
}

err_t gyro6050_wakeup(void)
{
    if(!gyro6050_wait_current_op()) return E_BUSY;
    gyro.data_byte = 0;
    return gyro6050_write_data(GYRO6050_STATE_SLEEP_WRITE, GYRO6050_PWR_MNGT_1_ADDRESS, &gyro.data_byte, 1);
}

void gyro6050_begin_calibration(void)
{
    gyro.calibrations_count = 0;
    
    memset(&gyro.calibrated_gyro_data, 0x0, sizeof(gyro6050_gyro_data_t));
}

err_t gyro6050_calibration_read(void)
{
    if(!gyro6050_wait_current_op()) return E_BUSY;
    return gyro6050_read_data(GYRO6050_STATE_CALIBRATION_READ, GYRO6050_GYRO_DATA_ADDRESS, &gyro.raw_data.gyro_data, sizeof(gyro6050_gyro_data_t));
}

void gyro6050_calibrate(void)
{
    if(!gyro.new_data_avail) return;
    
    int8_t tmp;
    SWAP(gyro.raw_data.gyro_data.gyro_x.parts.h, gyro.raw_data.gyro_data.gyro_x.parts.l, tmp);
    SWAP(gyro.raw_data.gyro_data.gyro_y.parts.h, gyro.raw_data.gyro_data.gyro_y.parts.l, tmp);
    SWAP(gyro.raw_data.gyro_data.gyro_z.parts.h, gyro.raw_data.gyro_data.gyro_z.parts.l, tmp);
    
    gyro.calibrated_gyro_data.gyro_x.value += gyro.raw_data.gyro_data.gyro_x.value;
    gyro.calibrated_gyro_data.gyro_y.value += gyro.raw_data.gyro_data.gyro_y.value;
    gyro.calibrated_gyro_data.gyro_z.value += gyro.raw_data.gyro_data.gyro_z.value;
    
    gyro.calibrations_count ++;
    
    gyro.new_data_avail = false;
}

uint8_t gyro6050_calibrations_count(void)
{
    return gyro.calibrations_count;
}

void gyro6050_end_calibration(void)
{
    gyro.calibrated_gyro_data.gyro_x.value /= gyro.calibrations_count;
    gyro.calibrated_gyro_data.gyro_y.value /= gyro.calibrations_count;
    gyro.calibrated_gyro_data.gyro_z.value /= gyro.calibrations_count;
}

err_t gyro6050_read(void)
{
    if(!gyro6050_wait_current_op()) return E_BUSY;
    return gyro6050_read_data(GYRO6050_STATE_DATA_READ, GYRO6050_ACCEL_TEMP_GYRO_DATA_ADDRESS, &gyro.raw_data, sizeof(gyro6050_raw_data_t));
}
/**
 * Получает значение сырых данных для угловой скорости в 1 градус / с.
 * @return Значение сырых данных для угловой скорости в 1 градус / с.
 */
static int8_t gyro6050_raw_value_1dps(void)
{
    switch(gyro.cached_data.gyro_scale_range){
        default:
        case GYRO6050_GYRO_SCALE_RANGE_250_DPS:
            return (131); // 32768 / 250
        case GYRO6050_GYRO_SCALE_RANGE_500_DPS:
            return (66); // 32768 / 500
        case GYRO6050_GYRO_SCALE_RANGE_1000_DPS:
            return (33); // 32768 / 1000
        case GYRO6050_GYRO_SCALE_RANGE_2000_DPS:
            return (16); // 32768 / 2000
    }
}

/**
 * Получает значение сырых данных для ускорения в 1 g.
 * @return Значение сырых данных для ускорения в 1 g.
 */
static int16_t gyro6050_raw_value_1g(void)
{
    switch(gyro.cached_data.accel_scale_range){
        default:
        case GYRO6050_ACCEL_SCALE_RANGE_2G:
            return (16384); // 32768 / 2
        case GYRO6050_ACCEL_SCALE_RANGE_4G:
            return (8192); // 32768 / 4
        case GYRO6050_ACCEL_SCALE_RANGE_8G:
            return (4096); // 32768 / 8
        case GYRO6050_ACCEL_SCALE_RANGE_16G:
            return (2048); // 32768 / 16
    }
}

void gyro6050_calculate(void)
{
    // Если нет новых данных.
    if(!gyro.new_data_avail) return;
    
    // Временная переменная.
    int8_t tmp;
    // Сконвертируем big-endian в little-endian.
    // Данные акселерометра.
    SWAP(gyro.raw_data.accel_data.accel_x.parts.h, gyro.raw_data.accel_data.accel_x.parts.l, tmp);
    SWAP(gyro.raw_data.accel_data.accel_y.parts.h, gyro.raw_data.accel_data.accel_y.parts.l, tmp);
    SWAP(gyro.raw_data.accel_data.accel_z.parts.h, gyro.raw_data.accel_data.accel_z.parts.l, tmp);
    // Данные температуры.
    SWAP(gyro.raw_data.temp_data.temp.parts.h, gyro.raw_data.temp_data.temp.parts.l, tmp);
    // Данные гироскопа.
    SWAP(gyro.raw_data.gyro_data.gyro_x.parts.h, gyro.raw_data.gyro_data.gyro_x.parts.l, tmp);
    SWAP(gyro.raw_data.gyro_data.gyro_y.parts.h, gyro.raw_data.gyro_data.gyro_y.parts.l, tmp);
    SWAP(gyro.raw_data.gyro_data.gyro_z.parts.h, gyro.raw_data.gyro_data.gyro_z.parts.l, tmp);

    // Вычислим температуру.
    gyro.data.temp = (fixed10_6_t)fixed10_6_make_from_fract((int32_t)gyro.raw_data.temp_data.temp.value, 340) + (fixed10_6_t)fixed10_6_make_from_fract((int32_t)3653, 100);
    
    // Единичные значения.
    // Единичное ускорение.
    int16_t raw_one_g = gyro6050_raw_value_1g();
    // Единичная угловая скорость.
    int8_t raw_one_dps = gyro6050_raw_value_1dps();
    
    // Вычислим данные акселерометра.
    gyro.data.accel_x = fixed10_6_make_from_fract((int32_t)gyro.raw_data.accel_data.accel_x.value, raw_one_g);
    gyro.data.accel_y = fixed10_6_make_from_fract((int32_t)gyro.raw_data.accel_data.accel_y.value, raw_one_g);
    gyro.data.accel_z = fixed10_6_make_from_fract((int32_t)gyro.raw_data.accel_data.accel_z.value, raw_one_g);
    
    // Вычислим данные гироскопа.
    // X.
    gyro.data.gyro_w_x = fixed10_6_make_from_fract(
                (int32_t)(gyro.raw_data.gyro_data.gyro_x.value -
                          gyro.calibrated_gyro_data.gyro_x.value), raw_one_dps);
    // Y.
    gyro.data.gyro_w_y = fixed10_6_make_from_fract(
                (int32_t)(gyro.raw_data.gyro_data.gyro_y.value -
                          gyro.calibrated_gyro_data.gyro_y.value), raw_one_dps);
    // Z.
    gyro.data.gyro_w_z = fixed10_6_make_from_fract(
                (int32_t)(gyro.raw_data.gyro_data.gyro_z.value -
                          gyro.calibrated_gyro_data.gyro_z.value), raw_one_dps);
    
    // Установим флаг отсутствия новых данных.
    gyro.new_data_avail = false;
}

fixed10_6_t gyro6050_temp(void)
{
    return gyro.data.temp;
}

fixed10_6_t gyro6050_accel_x(void)
{
    return gyro.data.accel_x;
}

fixed10_6_t gyro6050_accel_y(void)
{
    return gyro.data.accel_y;
}

fixed10_6_t gyro6050_accel_z(void)
{
    return gyro.data.gyro_w_z;
}

fixed10_6_t gyro6050_gyro_w_x(void)
{
    return gyro.data.gyro_w_x;
}

fixed10_6_t gyro6050_gyro_w_y(void)
{
    return gyro.data.gyro_w_y;
}

fixed10_6_t gyro6050_gyro_w_z(void)
{
    return gyro.data.gyro_w_z;
}