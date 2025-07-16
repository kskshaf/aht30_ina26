/**
 * Copyright (c) 2015 - present LibDriver All rights reserved
 * 
 * The MIT License (MIT)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE. 
 *
 * @file      driver_aht30.c
 * @brief     driver aht30 source file
 * @version   1.0.0
 * @author    Shifeng Li
 * @date      2023-11-30
 *
 * <h3>history</h3>
 * <table>
 * <tr><th>Date        <th>Version  <th>Author      <th>Description
 * <tr><td>2023/11/30  <td>1.0      <td>Shifeng Li  <td>first upload
 * </table>
 */

#include "driver_aht30.h"
#include "iic.h"

void delay_ms(uint32_t ms)
{
    usleep(ms * 1000);
}

/**
 * @brief chip address definition
 */
#define AHT30_ADDRESS             0x70        /**< iic device address */

/**
 * @brief     calculate the crc
 * @param[in] *data pointer to a data buffer
 * @param[in] len length of data
 * @return    crc
 * @note      none
 */
static uint8_t a_aht30_calc_crc(uint8_t *data, uint8_t len)
{
    uint8_t i;
    uint8_t byte;
    uint8_t crc = 0xFF;
    
    for (byte = 0; byte < len; byte++)          /* len times */
    {
        crc ^= data[byte];                      /* xor byte */
        for (i = 8; i > 0; --i)                 /* one byte */
        {
            if ((crc & 0x80) != 0)              /* if high*/
            {
                crc = (crc << 1) ^ 0x31;        /* xor 0x31 */
            }
            else
            {
                crc = crc << 1;                 /* skip */
            }
        }
    }
    
    return crc;                                 /* return crc */
}

/**
 * @brief     reset the register
 * @param[in] *handle pointer to an aht30 handle structure
 * @param[in] addr reset register
 * @return    status code
 *            - 0 success
 *            - 1 reset failed
 * @note      none
 */
static uint8_t a_aht30_jh_reset_reg(int g_handle, uint8_t addr)
{
    uint8_t buf[3];
    uint8_t regs[3];

    buf[0] = addr;                                     /* set the addr */
    buf[1] = 0x00;                                     /* set 0x00 */
    buf[2] = 0x00;                                     /* set 0x00 */

    if (iic_write_cmd(g_handle, AHT30_ADDRESS, buf, 3) != 0)  /* write the command */
    {
        return 1;
    }
    delay_ms(5);

    if (iic_read_cmd(g_handle, AHT30_ADDRESS, regs, 3) != 0)  /* read regs */
    {
        return 1;
    }
    delay_ms(10);

    buf[0] = 0xB0 | addr;                              /* set addr */
    buf[1] = regs[1];                                  /* set regs[1] */
    buf[2] = regs[2];                                  /* set regs[2] */
    if (iic_write_cmd(g_handle, AHT30_ADDRESS, buf, 3) != 0)
    {
        return 1;
    }
    
    return 0;
}

/**
 * @brief     initialize the chip
 * @param[in] *handle pointer to an aht30 handle structure
 * @return    status code
 *            - 0 success
 *            - 1 iic failed
 *            - 2 handle is NULL
 *            - 3 linked functions is NULL
 *            - 4 read status failed
 *            - 5 reset reg failed
 * @note      none
 */
uint8_t aht30_init(int *g_handle, char *i2c_port, uint8_t *inited, int dev_addr)
{
    uint8_t status;

    if (iic_init(i2c_port, g_handle, dev_addr) != 0)       /* iic init */
    {
        log_error("iic init failed.");
        return 1;
    }
    delay_ms(200);

    if (iic_read_cmd(*g_handle, AHT30_ADDRESS, &status, 1) != 0)           /* read the status */
    {
        log_error("read status failed.");
        iic_deinit(*g_handle);
        return 4;
    }

    if ((status & 0x18) != 0x18)                                         /* check the status */
    {
        if (a_aht30_jh_reset_reg(*g_handle, 0x1B) != 0)                   /* reset the 0x1B */
        {
            log_error("reset reg failed.");
            iic_deinit(*g_handle);
            return 5;
        }
        if (a_aht30_jh_reset_reg(*g_handle, 0x1C) != 0)                   /* reset the 0x1C */
        {
            log_error("reset reg failed.");
            iic_deinit(*g_handle);
            return 5;
        }
        if (a_aht30_jh_reset_reg(*g_handle, 0x1E) != 0)                   /* reset the 0x1E */
        {
            log_error("reset reg failed.");
            iic_deinit(*g_handle);
            return 5;
        }
    }
    delay_ms(10);
    *inited = 1;                                                        /* flag finish initialization */

    return 0;                                                          /* success return 0 */
}

/**
 * @brief     close the chip
 * @param[in] *handle pointer to an aht30 handle structure
 * @return    status code
 *            - 0 success
 *            - 1 deinit failed
 *            - 2 handle is NULL
 *            - 3 handle is not initialized
 * @note      none
 */
uint8_t aht30_deinit(int g_handle, uint8_t inited)
{
    if (inited != 1)                                       /* check handle initialization */
    {
        return 3;                                                  /* return error */
    }

    if (iic_deinit(g_handle) != 0)                                 /* iic deinit */
    {
        log_error("iic deinit failed.");        /* iic deinit failed */
        return 1;                                                  /* return error */
    }
    inited = 0;                                            /* set closed flag */
    
    return 0;                                                      /* success return 0 */
}

/**
 * @brief      read the temperature and humidity data
 * @param[in]  *handle pointer to an aht30 handle structure
 * @param[out] *temperature_raw pointer to a raw temperature buffer
 * @param[out] *temperature_s pointer to a converted temperature buffer
 * @param[out] *humidity_raw pointer to a raw humidity buffer
 * @param[out] *humidity_s pointer to a converted humidity buffer
 * @return     status code
 *             - 0 success
 *             - 1 read temperature humidity failed
 *             - 2 handle is NULL
 *             - 3 handle is not initialized
 *             - 4 data is not ready
 *             - 5 crc is error
 * @note       none
 */
uint8_t aht30_read_temperature_humidity(int g_handle, float *temperature_s, float *humidity_s, uint8_t inited)
{
    uint8_t buf[7];
    uint32_t temperature_raw;
    uint32_t humidity_raw;

    if (inited != 1)                                                  /* check handle initialization */
    {
        return 3;
    }

    buf[0] = 0xAC;                                                    /* set the addr */
    buf[1] = 0x33;                                                    /* set 0x33 */
    buf[2] = 0x00;                                                    /* set 0x00 */
    if (iic_write_cmd(g_handle, AHT30_ADDRESS, buf, 3) != 0)          /* write the command */
    {
        log_error("sent command failed.");                      /* sent command failed */
        return 1;
    }
    delay_ms(85);                                                     /* delay 85ms */

    if (iic_read_cmd(g_handle, AHT30_ADDRESS, buf, 7) != 0)           /* read data */
    {
        log_error("read data failed.");
        return 1;
    }
    if ((buf[0] & 0x80) != 0)                                         /* check busy */
    {
        log_error("data is not ready.");
        return 4;
    }
    if (a_aht30_calc_crc(buf, 6) != buf[6])                           /* check the crc */
    {
        log_error("crc is error.");
        return 5;
    }

    humidity_raw = (((uint32_t)buf[1]) << 16) |
                    (((uint32_t)buf[2]) << 8) |
                    (((uint32_t)buf[3]) << 0);                        /* set the humidity */
    humidity_raw = humidity_raw >> 4;                                 /* right shift 4 */
    *humidity_s = (float)humidity_raw
                            / 1048576.0f * 100.0f;                   /* convert the humidity */

    temperature_raw = (((uint32_t)buf[3]) << 16) |
                       (((uint32_t)buf[4]) << 8) |
                       (((uint32_t)buf[5]) << 0);                     /* set the temperature */
    temperature_raw = temperature_raw & 0xFFFFF;                      /* cut the temperature part */
    *temperature_s = (float)temperature_raw 
                             / 1048576.0f * 200.0f
                             - 50.0f;                                 /* right shift 4 */
    
    return 0;                                                         /* success return 0 */
}
