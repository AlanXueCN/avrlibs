/**
 * @file gyro6050.h
 * Библиотека для работы с гироскопом MPU-6050.
 */

#ifndef GYRO6050_H
#define	GYRO6050_H

#include "errors/errors.h"
#include "i2c/i2c.h"
#include "fixed/fixed16.h"

//! Адреса i2c гироскопа.
//! Пин AD0 подтянут к земле.
#define GYRO6050_I2C_ADDRESS0 0x68
//! Пин AD0 подтянут к питанию.
#define GYRO6050_I2C_ADDRESS1 0x69

//! Идентификатор передачи i2c по умолчанию.
#define GYRO6050_DEFAULT_I2C_TRANSFER_ID GYRO6050_I2C_ADDRESS0

//! Частота вывода данных.
//! Частота вывода данных акселерометра.
#define GYRO6050_ACCEL_OUTPUT_RATE_KHZ      1
//! Частота вывода данных гироскопа.
#define GYRO6050_GYRO_OUTPUT_RATE_KHZ       8
//! Частота вывода данных гироскопа с DLPF.
#define GYRO6050_GYRO_DLPF_OUTPUT_RATE_KHZ  1


//! Фильтр низких частот.
//! Акселерометр 260 Hz, гироскоп 256 Hz.
#define GYRO6050_DLPF_A260HZ_G256HZ     0
//! Акселерометр 184 Hz, гироскоп 188 Hz.
#define GYRO6050_DLPF_A184HZ_G188HZ     1
//! Акселерометр 94 Hz, гироскоп 98 Hz.
#define GYRO6050_DLPF_A94HZ_G98HZ       2
//! Акселерометр 44 Hz, гироскоп 42 Hz.
#define GYRO6050_DLPF_A44HZ_G42HZ       3
//! Акселерометр 21 Hz, гироскоп 20 Hz.
#define GYRO6050_DLPF_A21HZ_G20HZ       4
//! Акселерометр 10 Hz, гироскоп 10 Hz.
#define GYRO6050_DLPF_A10HZ_G10HZ       5
//! Акселерометр 5 Hz, гироскоп 5 Hz.
#define GYRO6050_DLPF_A5HZ_G5HZ         6
//! Тип фильтра низких частот.
typedef uint8_t gyro6050_dlpf_t;


//! Диапазон данных гироскопа.
//! +-250 degrees per second
#define GYRO6050_GYRO_SCALE_RANGE_250_DPS       0
//! +-500 degrees per second
#define GYRO6050_GYRO_SCALE_RANGE_500_DPS       1
//! +-1000 degrees per second
#define GYRO6050_GYRO_SCALE_RANGE_1000_DPS      2
//! +-2000 degrees per second
#define GYRO6050_GYRO_SCALE_RANGE_2000_DPS      3
//! Тип диапазона данных гироскопа.
typedef uint8_t gyro6050_gyro_scale_range_t;

//! Диапазон данных акселерометра.
//! +-2g
#define GYRO6050_ACCEL_SCALE_RANGE_2G           0
//! +-4g
#define GYRO6050_ACCEL_SCALE_RANGE_4G           1
//! +-8g
#define GYRO6050_ACCEL_SCALE_RANGE_8G           2
//! +-16g
#define GYRO6050_ACCEL_SCALE_RANGE_16G          3
//! Тип диапазона данных гироскопа.
typedef uint8_t gyro6050_accel_scale_range_t;

//! Конфигурация пина сигнала о прерываниях.
//! Уровень активного сигнала - высокий.
#define GYRO6050_INT_PIN_PIN_LEVEL_HI               0
//! Уровень активного сигнала - низкий.
#define GYRO6050_INT_PIN_LEVEL_LO                   128
//! Тип выхода - push-pull.
#define GYRO6050_INT_PIN_OUT_PUSH_PULL              0
//! Тип выхода - открытый коллектор.
#define GYRO6050_INT_PIN_OUT_OPEN_DRAIN             64
//! Продолжительность сигнала - 50 us.
#define GYRO6050_INT_PIN_PULSE_50US                 0
//! Продолжительность сигнала - пока не очищен флаг прерывания.
#define GYRO6050_INT_PIN_PULSE_UNTIL_CLEARED        32
//! Очистка флага прерывания - только при чтении регистра статуса.
#define GYRO6050_INT_PIN_CLEAN_ON_READ_STATUS       0
//! Очистка флага прерывания - при любом чтении.
#define GYRO6050_INT_PIN_CLEAN_ON_ANY_READ_OP       16
//! Уровень активного сигнала FSYNC - высокий.
#define GYRO6050_INT_PIN_FSYNC_LEVEL_HI             0
//! Уровень активного сигнала FSYNC - низкий.
#define GYRO6050_INT_PIN_FSYNC_LEVEL_LO             8
//! Генерация сигнала на пине FSYNC - выключена.
#define GYRO6050_INT_PIN_FSYNC_DISABLED             0
//! Генерация сигнала на пине FSYNC - включена.
#define GYRO6050_INT_PIN_FSYNC_ENABLED              4
//! Получение доступа к AUX i2c - запрещено.
#define GYRO6050_INT_PIN_I2C_BYPASS_DISABLED        0
//! Получение доступа к AUX i2c - разрешено.
#define GYRO6050_INT_PIN_I2C_BYPASS_ENABLED         2
//! Тип конфигурации пина сигнала прерываниях.
typedef uint8_t gyro6050_int_pin_conf_t;

//! Конфигурация сигнала о прерываниях.
//! Прерывание от детектора движения.
#define GYRO6050_INT_ON_MOTION_DETECTION        64
//! Прерывание по переполнению буфера FIFO.
#define GYRO6050_INT_ON_FIFO_OVERFLOW           16
//! Прерывание при событии i2c.
#define GYRO6050_INT_ON_I2C_EVENT               8
//! Прерывание по готовности данных.
#define GYRO6050_INT_ON_DATA_READY              1
//! Тип конфигурации сигнала о прерываниях.
typedef uint8_t gyro6050_int_conf_t;

//! Максимальный вес угла по данным акселерометра.
#define GYRO6050_ACCEL_ANGLE_WEIGHT_MAX         100

/**
 * Каллбэк i2c гироскопа.
 * @return Флаг обработки события.
 */
extern bool gyro6050_i2c_callback(void);

/**
 * Инициализирует данные гироскопа.
 * Идентификатор пердачи по умолчанию.
 * @param address Адрес устройства на шине i2c.
 * @return Код ошибки.
 */
extern err_t gyro6050_init(i2c_address_t address);

/**
 * Получает флаг завершения текущего действия.
 * @return Флаг завершения текущего действия.
 */
extern bool gyro6050_done(void);

/**
 * Ждёт завершения текущей операции.
 */
extern err_t gyro6050_wait(void);

/**
 * Получает флаг наличия выполняемой операции.
 * @return Флаг наличия выполняемой операции.
 */
extern bool gyro6050_busy(void);

/**
 * Получает код ошибки последней операции.
 * @return Код ошибки.
 */
extern err_t gyro6050_error(void);

/**
 * Получает идентификатор передачи i2c.
 * @return Идентификатор передачи i2c.
 */
extern i2c_transfer_id_t gyro6050_i2c_transfer_id(void);

/**
 * Устанавливает идентификатор передачи i2c.
 * @param transfer_id Идентификатор передачи i2c.
 */
extern void gyro6050_i2c_set_transfer_id(i2c_transfer_id_t transfer_id);

/**
 * Считывает делитель частоты вывода из гироскопа.
 * Данные действительны до следующей операции с гироскопом.
 * @return Код ошибки.
 */
extern err_t gyro6050_read_rate_divisor(void);

/**
 * Получает считанное значение делителя частоты вывода гироскопа.
 * @return Считанное значение делителя частоты вывода гироскопа.
 */
extern uint8_t gyro6050_rate_divisor(void);

/**
 * Устанавливает делитель частоты вывода гироскопа.
 * @param divisor Делитель.
 * @return Код ошибки.
 */
extern err_t gyro6050_set_rate_divisor(uint8_t divisor);

/**
 * Считывает значение фильтра низких частот..
 * Данные действительны до следующей операции с гироскопом.
 * @return Код ошибки.
 */
extern err_t gyro6050_read_dlpf(void);

/**
 * Получает считанное значение фильтра низких частот.
 * @return Считанное значение фильтра низких частот.
 */
extern gyro6050_dlpf_t gyro6050_dlpf(void);

/**
 * Устанавливает значение фильтра низких частот.
 * @param dlpf Значение фильтра низких частот.
 * @return Код ошибки.
 */
extern err_t gyro6050_set_dlpf(gyro6050_dlpf_t dlpf);

/**
 * Считывает значение диапазона данных гироскопа.
 * Данные действительны до следующей операции с гироскопом.
 * @return Код ошибки.
 */
extern err_t gyro6050_read_gyro_scale_range(void);

/**
 * Получает считанное значение диапазона данных гироскопа.
 * @return Считанное значение диапазона данных гироскопа.
 */
extern gyro6050_gyro_scale_range_t gyro6050_gyro_scale_range(void);

/**
 * Устанавливает значение диапазона данных гироскопа.
 * @param range Значение диапазона данных гироскопа.
 * @return Код ошибки.
 */
extern err_t gyro6050_set_gyro_scale_range(gyro6050_gyro_scale_range_t range);

/**
 * Считывает значение диапазона данных акселерометра.
 * Данные действительны до следующей операции с гироскопом.
 * @return Код ошибки.
 */
extern err_t gyro6050_read_accel_scale_range(void);

/**
 * Получает считанное значение диапазона данных акселерометра.
 * @return Считанное значение диапазона данных акселерометра.
 */
extern gyro6050_accel_scale_range_t gyro6050_accel_scale_range(void);

/**
 * Устанавливает значение диапазона данных акселерометра.
 * @param range Значение диапазона данных акселерометра.
 * @return Код ошибки.
 */
extern err_t gyro6050_set_accel_scale_range(gyro6050_accel_scale_range_t range);

/**
 * Считывает .
 * Данные действительны до следующей операции с гироскопом.
 * @return Код ошибки.
 */
//extern err_t gyro6050_read_(void);

/**
 * Получает считанное значение .
 * @return Считанное значение .
 */
//extern uint8_t gyro6050_(void);

/**
 * Устанавливает .
 * @param data.
 * @return Код ошибки.
 */
//extern err_t gyro6050_set_(uint8_t data);

/**
 * Конфигурирует пин сигнала о прерываниях.
 * @param conf Конфигурация пина.
 * @return Код ошибки.
 */
extern err_t gyro6050_int_pin_configure(gyro6050_int_pin_conf_t conf);

/**
 * Считывает конфигурацию пина сигнала о прерываниях.
 * @return Код ошибки.
 */
extern err_t gyro6050_read_int_pin_config(void);

/**
 * Получает считанную конфигурацию пина сигнала о прерываниях.
 * @return Конфигурацию пина сигнала о прерываниях.
 */
extern gyro6050_int_pin_conf_t gyro6050_int_pin_config(void);

/**
 * Конфигурирует сигнал о прерываниях.
 * @param conf Конфигурация сигнала.
 * @return Код ошибки.
 */
extern err_t gyro6050_int_configure(gyro6050_int_conf_t conf);

/**
 * Считывает конфигурацию сигнала о прерываниях.
 * @return Код ошибки.
 */
extern err_t gyro6050_read_int_config(void);

/**
 * Получает считанную конфигурацию сигнала о прерываниях.
 * @return Конфигурацию сигнала о прерываниях.
 */
extern gyro6050_int_conf_t gyro6050_int_config(void);

/**
 * Пробуждает гироскоп.
 * @return Код ошибки.
 */
extern err_t gyro6050_wakeup(void);

/**
 * Начинает калибровку гироскопа.
 */
extern void gyro6050_begin_calibration(void);

/**
 * Читает данные гироскопа для калибровки.
 * @return Код ошибки.
 */
extern err_t gyro6050_calibration_read(void);

/**
 * Вычисляет данные для калибровки.
 */
extern void gyro6050_calibrate(void);

/**
 * Получает число калибровочных данных.
 * @return Число калибровочных данных.
 */
extern uint8_t gyro6050_calibrations_count(void);

/**
 * Завершает калибровку и вычисляет калибровочные данные.
 */
extern void gyro6050_end_calibration(void);

/**
 * Считывает данные из гироскопа.
 * @return Код ошибки.
 */
extern err_t gyro6050_read(void);

/**
 * Получает вес угла по данным акселерометра.
 * Диапазон от 0 до 100 включительно.
 * @return Вес угла по данным акселерометра.
 */
extern uint8_t gyro6050_accel_angle_weight(void);

/**
 * Устанавливает вес угла по данным акселерометра.
 * Диапазон от 0 до 100 включительно.
 * @param weight Вес угла по данным акселерометра.
 */
extern void gyro6050_set_accel_angle_weight(uint8_t weight);

/**
 * Вычисляет ориентацию по полученным данным.
 */
extern void gyro6050_calculate(void);

/**
 * Получает температуру гироскопа.
 * @return Температура гироскопа.
 */
extern fixed10_6_t gyro6050_temp(void);

/**
 * Получает ускорение по оси X.
 * @return Ускорение по оси X.
 */
extern fixed10_6_t gyro6050_accel_x(void);

/**
 * Получает ускорение по оси Y.
 * @return Ускорение по оси Y.
 */
extern fixed10_6_t gyro6050_accel_y(void);

/**
 * Получает ускорение по оси Z.
 * @return Ускорение по оси Z.
 */
extern fixed10_6_t gyro6050_accel_z(void);

/**
 * Получает угол поворота вокруг оси X.
 * @return Угол поворота вокруг оси X.
 */
extern fixed10_6_t gyro6050_angle_x(void);

/**
 * Получает угол поворота вокруг оси Y.
 * @return Угол поворота вокруг оси Y.
 */
extern fixed10_6_t gyro6050_angle_y(void);

/**
 * Получает угол поворота вокруг оси Z.
 * @return Угол поворота вокруг оси Z.
 */
extern fixed10_6_t gyro6050_angle_z(void);

#endif	/* GYRO6050_H */

