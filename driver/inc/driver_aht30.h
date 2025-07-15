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
 * @file      driver_aht30.h
 * @brief     driver aht30 header file
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

#ifndef DRIVER_AHT30_H
#define DRIVER_AHT30_H

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include "log.h"

#ifdef __cplusplus
extern "C"{
#endif

/**
 * @defgroup aht30_driver aht30 driver function
 * @brief    aht30 driver modules
 * @{
 */

/**
 * @addtogroup aht30_base_driver
 * @{
 */

/**
 * @}
 */

/**
 * @defgroup aht30_link_driver aht30 link driver function
 * @brief    aht30 link driver modules
 * @ingroup  aht30_driver
 * @{
 */

/**
 * @}
 */

/**
 * @defgroup aht30_base_driver aht30 base driver function
 * @brief    aht30 base driver modules
 * @ingroup  aht30_driver
 * @{
 */

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
uint8_t aht30_init(int *g_handle, char *i2c_port, uint8_t *inited, int dev_addr);

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
uint8_t aht30_deinit(int g_handle, uint8_t inited);

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
uint8_t aht30_read_temperature_humidity(int g_handle, float *temperature_s, uint8_t *humidity_s, uint8_t inited);

#ifdef __cplusplus
}
#endif

#endif
