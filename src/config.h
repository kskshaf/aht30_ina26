#ifndef CONFIG_H
#define CONFIG_H

#define IIC_DEVICE_PORT_1 "/dev/i2c-3"        /**< iic device name */
#define IIC_DEVICE_PORT_2 "/dev/i2c-4"        /**< iic device name */
#define IIC_DEVICE_ADDR 0x38

#define Temp_HIGH    45
#define Temp_BACK    42

#define KEEP_FILE    "/tmp/aht30_keep"
#define DATA_FILE_1  "/tmp/aht30_data_1"
#define DATA_FILE_2  "/tmp/aht30_data_2"
#define LOG_FILE     "/var/log/aht30/aht30.log"

#endif