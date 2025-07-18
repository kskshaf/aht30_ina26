#include "driver_aht30.h"
#include "config.h"
#include "exit.h"

/* aht30_2 监测线程 */
pthread_t    aht30_2_thread;
volatile int aht30_2_thread_running = 0;

void* aht30_2_handler(void* arg) {
    log_info("启动 aht30_2 线程");

    static int gs_fd_2; // iic handle
    uint8_t inited_2;

    int thread_res;
    int read_retry_2 = 0;

    float temperature_2 = 0, humidity_2 = 0;

    /* aht30_2 init */
    if (aht30_init(&gs_fd_2, IIC_DEVICE_PORT_2, &inited_2, IIC_DEVICE_ADDR) != 0)
    {
        pthread_detach(aht30_2_thread);   // 线程分离
        aht30_2_thread_running = 0;

        log_error("aht30_2: 初始化失败");

        // 发送邮件
        thread_res = system("/root/send_mail.sh 12");
        if(thread_res != 0) log_error("邮件发送异常!");

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
                thread_res = system("/root/send_mail.sh 12");
                if(thread_res != 0) log_error("邮件发送异常!");

                // 清理
                (void)aht30_deinit(gs_fd_2, inited_2);

                log_error("aht30_2: 重试超过5次, 退出线程");
                pthread_exit(NULL);
            }

            // 尝试重新初始化
            if (aht30_init(&gs_fd_2, IIC_DEVICE_PORT_2, &inited_2, IIC_DEVICE_ADDR) != 0)
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

int main(void)
{
    static int gs_fd_1; // iic handle
    uint8_t inited_1 = 0;

    int thread_res;
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
    if(log_add_fp(fp_log, LOG_INFO) != 0)
    {
        log_error("打开日志文件失败!");
        return -1;
    }

    log_info("==============启动主进程==============");

    /* aht30_1 init */
    if (aht30_init(&gs_fd_1, IIC_DEVICE_PORT_1, &inited_1, IIC_DEVICE_ADDR) != 0)
    {
        log_error("aht30_1 初始化失败");
        return -1;
    }

    // 创建 aht30_2 线程
    if(aht30_2_thread_handle(0) != 0) return -1;

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
                thread_res = system("/root/send_mail.sh 12");
                if(thread_res != 0) log_error("邮件发送异常!");

                // 清理退出
                (void)aht30_deinit(gs_fd_1, inited_1);
                (void)aht30_2_thread_handle(1);
                cleanup_files();

                return -1;
            }

            // 尝试重新初始化
            if (aht30_init(&gs_fd_1, IIC_DEVICE_PORT_1, &inited_1, IIC_DEVICE_ADDR) != 0)
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
            fprintf(fp, "T=%.2f\nH=%.1f\n", temperature_main, humidity_main);
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
                thread_res = system("/root/send_mail.sh 30");
                if(thread_res != 0) log_error("邮件发送异常!");
            }
            overH_task_lock = 1;
            tempN_task_lock = 0;
        }
        else
        {
            if(!tempN_task_lock && (temperature_main < Temp_BACK))
            {
                log_info("温度已降低到 %.2f°C", temperature_main);
                thread_res = system("/root/send_mail.sh 31");
                if(thread_res != 0) log_error("邮件发送异常!");
                tempN_task_lock = 1;
            }
            overH_task_lock = 0;
        }

        /* delay 1000ms */
        usleep(1000*1000);
    }

    /* deinit */
    (void)aht30_deinit(gs_fd_1, inited_1);

    // 回收 aht30_2 线程
    if(aht30_2_thread_handle(1) != 0) return -1;

    cleanup_files();
    log_info("==============退出主进程==============");
    return 0;
}
