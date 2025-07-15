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
 * @file      main.c
 * @brief     main source file
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

//#include "driver_aht30_basic.h"
#include "driver_aht30.h"
#include <math.h>
#include <getopt.h>
#include <stdlib.h>
#include "device.h"

/**
 * @brief     main function
 * @param[in] argc arg numbers
 * @param[in] **argv arg address
 * @return    status code
 *             - 0 success
 * @note      none
 */
int main(int argc, char **argv)
{
    //uint8_t res;
    uint32_t times = 50;

    float temperature_1;
    uint8_t humidity_1;

    float temperature_2;
    uint8_t humidity_2;

    /* aht30_1 init */
    if (aht30_init(&gs_fd_1, IIC_DEVICE_PORT_1, &inited_1, IIC_DEVICE_ADDR) != 0)
    {
        log_error("aht30_1 init failed.");
        return 1;
    }

    /* aht30_2 init */
    if (aht30_init(&gs_fd_2, IIC_DEVICE_PORT_2, &inited_2, IIC_DEVICE_ADDR) != 0)
    {
        log_error("aht30_2 init failed.");
        return 1;
    }

    /* loop */
    for (uint32_t i = 0; i < times; i++)
    {
        /* read temperature and humidity */
        if (aht30_read_temperature_humidity(gs_fd_1, &temperature_1, &humidity_1, inited_1) != 0)
        {
            /* deinit aht30 and close bus */
            (void)aht30_deinit(gs_fd_1, inited_1);
        }
        printf("count: %d/%d.\n", (uint32_t)(i + 1), (uint32_t)times);
        printf("====================================\n");
        printf("aht30_1: temperature is %0.2fC.\n", temperature_1);
        printf("aht30_1: humidity is %d%%.\n", humidity_1);

        if(i % 10 == 0)
        {
            if (aht30_read_temperature_humidity(gs_fd_2, &temperature_2, &humidity_2, inited_2) != 0)
            {
                /* deinit aht30 and close bus */
                (void)aht30_deinit(gs_fd_2, inited_2);
            }

            printf("====================================\n");
            printf("aht30_2: temperature is %0.2fC.\n", temperature_2);
            printf("aht30_2: humidity is %d%%.\n\n", humidity_2);
        }
        else
        {
            printf("\n");
        }

        /* delay 1000ms */
        usleep(1000*1000);
    }

    /* deinit */
    (void)aht30_deinit(gs_fd_1, inited_1);
    (void)aht30_deinit(gs_fd_2, inited_2);
    return 0;
}
