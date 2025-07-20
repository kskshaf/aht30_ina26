#include "aht30_2.h"
#include "driver_aht30.h"
#include "send_mail.h"

pthread_t    aht30_2_thread;
volatile int aht30_2_thread_running = 0;

/* aht30_2 监测线程 */
void* aht30_2_handler(void* arg) {
    log_info("启动 aht30_2 线程");

    static int gs_fd_2; // iic handle
    uint8_t inited_2;

    int read_retry_2 = 0;

    float temperature_2 = 0, humidity_2 = 0;

    /* aht30_2 init */
    if (aht30_init(&gs_fd_2, IIC_DEVICE_PORT_2, &inited_2, AHT30_DEVICE_ADDR) != 0)
    {
        pthread_detach(aht30_2_thread);   // 线程分离
        aht30_2_thread_running = 0;
        keep_running = 0;

        log_error("aht30_2: 初始化失败");

        // 发送邮件
        send_mail(12);

        log_error("退出 aht30_2 线程");
        pthread_exit(NULL);
    }

    while(keep_running && !check_keep_file()) {

        while(aht30_read_temperature_humidity(gs_fd_2, &temperature_2, &humidity_2, inited_2) != 0)
        {
            // 错误时写入"ERROR"
            FILE *fp = fopen(DATA_FILE_2, "w");
            if (fp != NULL) {
                fprintf(fp, "ERROR");
                fclose(fp);
            } else {
                log_error("打开 %s 失败", DATA_FILE_2);
            }

            // 重试超过5次退出
            if(read_retry_2 >= 5)
            {
                pthread_detach(aht30_2_thread);   // 线程分离
                aht30_2_thread_running = 0;

                // 发送邮件
                send_mail(12);

                // 清理
                (void)aht30_deinit(gs_fd_2, inited_2);

                log_error("aht30_2: 重试超过5次, 退出线程");
                pthread_exit(NULL);
            }

            // 尝试重新初始化
            if (aht30_init(&gs_fd_2, IIC_DEVICE_PORT_2, &inited_2, AHT30_DEVICE_ADDR) != 0)
            {
                log_error("aht30_2: 初始化失败");
            }

            log_warn("接收超时, 正在重试: %d", read_retry_2+1);
            read_retry_2++;
            usleep(2000*1000);
        }
        read_retry_2 = 0;

        // 写入文件
        FILE *fp = fopen(DATA_FILE_2, "w");
        if (fp != NULL) {
            fprintf(fp, "T=%.2f\nH=%.1f\n", temperature_2, humidity_2);
            fclose(fp);
        } else {
            log_error("打开 %s 失败", DATA_FILE_2);
            break;
        }

        usleep(2000*1000);
    }

    (void)aht30_deinit(gs_fd_2, inited_2);
    log_info("退出 aht30_2 线程");

    pthread_exit(NULL);
}

// 创建或回收 aht30_2 线程
// 0: 创建; 1: 回收
int aht30_2_thread_handle(int handle)
{
    int res;

    // 创建线程
    if(!handle) {
        res = pthread_create(&aht30_2_thread, NULL, aht30_2_handler, NULL);
        if(res != 0) {
            aht30_2_thread_running = 0;
            log_error("创建 aht30_2 线程失败: %d", res);
            return 1;
        } else {
            aht30_2_thread_running = 1;
            log_info("创建 aht30_2 线程成功");
        }
    } else {
        if(aht30_2_thread_running != 0)
        {
            res = pthread_join(aht30_2_thread, NULL);
            if(res != 0) {
                log_error("退出 aht30_2 线程失败: %d", res);
                return 1;
            } else {
                aht30_2_thread_running = 0;
                log_info("退出 aht30_2 线程成功");
            }
        }
    }
    return 0;
}