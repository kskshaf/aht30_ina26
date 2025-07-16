#ifndef _DEVICE_H
#define _DEVICE_H

#define IIC_DEVICE_PORT_1 "/dev/i2c-3"        /**< iic device name */
#define IIC_DEVICE_PORT_2 "/dev/i2c-4"        /**< iic device name */
#define IIC_DEVICE_ADDR 0x38

static uint8_t inited_1;
static uint8_t inited_2;

static int gs_fd_1;                           /**< iic handle */
static int gs_fd_2;                           /**< iic handle */

#endif