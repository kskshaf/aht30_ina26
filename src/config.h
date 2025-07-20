#ifndef CONFIG_H
#define CONFIG_H

#define IIC_DEVICE_PORT_1  "/dev/i2c-2"        /**< iic device name */
#define IIC_DEVICE_PORT_2  "/dev/i2c-4"        /**< iic device name */
#define AHT30_DEVICE_ADDR  0x38
#define INA226_DEVICE_ADDR 0x40

// aht30 温度
#define Temp_HIGH    45
#define Temp_BACK    42

// ina226 电压
#define Voltage_OFF   3000.0f
#define Voltage_LOW   6300.0f
#define Voltage_HIGH  8500.0f

#define KEEP_FILE    "/tmp/aht30_ina226_keep"
#define DATA_FILE_1  "/tmp/aht30_ina226_data"
#define DATA_FILE_2  "/tmp/aht30_data_2"
#define LOG_FILE     "/var/log/o-sensors/aht30_ina226.log"

#endif