#include "driver_aht30.h"
#include "device.h"
#include "exit.h"

#define Temp_HIGH 45


/* 另一传感器的监测线程 */
pthread_t    aht30_2_thread;
volatile int aht30_2_thread_running = 0;
volatile float temperature_2 = 0, humidity_2 = 0;

void* aht30_2_handler(void* arg) {
    pthread_detach(aht30_2_thread);   // 线程分离
    log_info(" ");

    static int gs_fd_2; // iic handle
    static uint8_t inited_2;

    /* aht30_2 init */
    if (aht30_init(&gs_fd_2, IIC_DEVICE_PORT_2, &inited_2, IIC_DEVICE_ADDR) != 0)
    {
        log_error("aht30_2 init failed.");
        return 1;
    }

    while(keep_running && !check_keep_file()) {

        if (aht30_read_temperature_humidity(gs_fd_2, &temperature_2, &humidity_2, inited_2) != 0)
        {
            /* deinit aht30 and close bus */
            (void)aht30_deinit(gs_fd_2, inited_2);
        }

        printf("====================================\n");
        printf("aht30_2: 温度 %.2fC, 湿度 %.1f%%\n", temperature_2, humidity_2);

        usleep(5000*1000);
    }

    (void)aht30_deinit(gs_fd_2, inited_2);
    log_info("退出 aht30_2 线程");
    aht30_2_thread_running = 0;

    pthread_exit(NULL);
}


int main(void)
{
    // 设置 locale
    setlocale(LC_ALL, "zh_CN.UTF-8");
    // 设置信号处理
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    static int gs_fd_1; // iic handle
    static uint8_t inited_1;
    int thread_res;

    float temperature_1 = 0, humidity_1 = 0;

    /* aht30_1 init */
    if (aht30_init(&gs_fd_1, IIC_DEVICE_PORT_1, &inited_1, IIC_DEVICE_ADDR) != 0)
    {
        log_error("aht30_1 init failed.");
        return 1;
    }

    // 创建温度异常线程
    if(!aht30_2_thread_running)
    {
        thread_res = pthread_create(&aht30_2_thread, NULL, aht30_2_handler, NULL);
        if(thread_res != 0)
        {
            aht30_2_thread_running = 0;
            log_error("创建 aht30_2 线程失败: %d", thread_res);
        }
        else
        {
            aht30_2_thread_running = 1;
            log_info("创建 aht30_2 线程成功");
        }
    }

    /* loop */
    while(keep_running && !check_keep_file())
    {
        /* read temperature and humidity */
        if (aht30_read_temperature_humidity(gs_fd_1, &temperature_1, &humidity_1, inited_1) != 0)
        {
            /* deinit aht30 and close bus */
            (void)aht30_deinit(gs_fd_1, inited_1);
        }
        printf("====================================\n");
        printf("aht30_1: 温度 %.2fC, 湿度 %.1f%%\n", temperature_1, humidity_1);

        /* delay 1000ms */
        usleep(1000*1000);
    }

    /* deinit */
    (void)aht30_deinit(gs_fd_1, inited_1);
    return 0;
}
