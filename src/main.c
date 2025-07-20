#include "includes.h"
#include "driver_aht30.h"
#include "driver_ina226_basic.h"
#include "config.h"
#include "exit.h"
#include "send_mail.h"
#include "aht30_2.h"
#include "ina226.h"

int main(void)
{
    static int gs_fd_1; // iic handle
    uint8_t inited_1 = 0;

    int overH_task_lock = 0; // 过热锁定
    int tempN_task_lock = 1; // 常温锁定
    int read_retry = 0;

    float temperature_main = 0, humidity_main = 0;

    // 设置 locale
    setlocale(LC_ALL, "zh_CN.UTF-8");

    // 设置信号处理
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    // 创建 KEEP_FILE
    FILE *fp_keep = fopen(KEEP_FILE, "w");
    if(fp_keep == NULL)
    {
        log_error("创建 KEEP_FILE 失败!");
        return -1;
    }
    fclose(fp_keep);

    // 打开日志文件
    FILE *fp_log = fopen(LOG_FILE, "a");
    if(fp_log == NULL)
    {
        log_error("打开日志文件失败!");
        return -1;
    }
    log_add_fp(fp_log, LOG_INFO);

    log_info("==============启动主进程==============");

    /* aht30_1 init */
    if (aht30_init(&gs_fd_1, IIC_DEVICE_PORT_1, &inited_1, AHT30_DEVICE_ADDR) != 0)
    {
        log_error("aht30_1 初始化失败");
        return -1;
    }

    // 创建 ina226 & aht30_2 线程
    if((ina226_thread_handle(0) != 0) || (aht30_2_thread_handle(0) != 0)) {
        // 回收线程
        (void)ina226_thread_handle(1);
        (void)aht30_2_thread_handle(1);

        return -1;
    }

    // 等待其他线程的 keep_running
    usleep(500*1000);

    /* loop */
    while(keep_running && !check_keep_file())
    {
        /* read temperature and humidity */
        while(aht30_read_temperature_humidity(gs_fd_1, &temperature_main, &humidity_main, inited_1) != 0)
        {
            // 错误时写入"ERROR"
            FILE *fp = fopen(DATA_FILE_1, "w");
            if (fp != NULL) {
                fprintf(fp, "ERROR");
                fclose(fp);
            } else {
                log_error("打开 %s 失败", DATA_FILE_1);
            }

            // 重试大于5次退出
            if(read_retry >= 5)
            {
                log_error("重试超过5次, 退出进程");

                // 发送邮件
                send_mail(12);

                // 清理退出
                (void)aht30_deinit(gs_fd_1, inited_1);
                (void)aht30_2_thread_handle(1);
                (void)ina226_thread_handle(1);
                cleanup_files();

                return -1;
            }

            // 尝试重新初始化
            if (aht30_init(&gs_fd_1, IIC_DEVICE_PORT_1, &inited_1, AHT30_DEVICE_ADDR) != 0)
            {
                log_error("aht30_1 初始化失败");
            }

            log_warn("接收超时, 正在重试: %d", read_retry+1);
            read_retry++;
        }
        read_retry = 0;

        // 写入文件
        FILE *fp = fopen(DATA_FILE_1, "w");
        if (fp != NULL) {
            fprintf(fp, "V=%.3f\nT=%.2f\nH=%.1f\n", ina226_voltage, temperature_main, humidity_main);
            fclose(fp);
        } else {
            log_error("打开 %s 失败", DATA_FILE_1);
            break;
        }

        // 过热或温度恢复正常时发送邮件
        if(temperature_main > Temp_HIGH)
        {
            if(!overH_task_lock)
            {
                log_warn("警告：当前温度为 %.2f°C!!!", temperature_main);
                send_mail(30);
            }
            overH_task_lock = 1;
            tempN_task_lock = 0;
        }
        else
        {
            if(!tempN_task_lock && (temperature_main < Temp_BACK))
            {
                log_info("温度已降低到 %.2f°C", temperature_main);
                send_mail(31);
                tempN_task_lock = 1;
            }
            overH_task_lock = 0;
        }

        /* delay 1000ms */
        usleep(1000*1000);
    }

    /* deinit */
    (void)aht30_deinit(gs_fd_1, inited_1);

    // 回收 ina226 & aht30_2 线程
    if((ina226_thread_handle(1) != 0) || (aht30_2_thread_handle(1) != 0)) {
        usleep(100*1000); // 等待线程退出？
        return -1;
    }

    cleanup_files();
    log_info("==============退出主进程==============");
    return 0;
}
