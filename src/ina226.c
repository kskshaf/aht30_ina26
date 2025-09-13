#include "ina226.h"
#include "driver_ina226_basic.h"
#include "send_mail.h"

pthread_t    ina226_thread;
volatile int ina226_thread_running = 0;
volatile float ina226_voltage = 0.0f;

/* ina226 监测线程 */
void* ina226_handler(void* arg) {
    double r = 0.1;
    ina226_address_t addr = INA226_ADDRESS_0;
    uint8_t res;
    float mV;

    int lowV_task_lock  = 0;   // 欠压锁定
    int highV_task_lock = 0;   // 过压锁定
    int offV_task_lock  = 0;   // 缺电锁定
    int norM_task_lock  = 1;   // 常态锁定

    int vl_count = 0;          // 欠压计数
    int vh_count = 0;          // 过压计数
    int of_count = 0;          // 缺电计数
    int nm_count = 0;          // 常态计数

    int read_retry = 0;        // 读取重试计数

    /* basic init */
    res = ina226_basic_init(addr, r);
    if (res != 0)
    {
        pthread_detach(ina226_thread);   // 线程分离
        ina226_thread_running = 0;
        keep_running = 0;

        log_error("ina226 初始化失败, 退出 ina226 线程");
        send_mail(11);

        pthread_exit(NULL);
    }

    /* delay 200ms */
    usleep(200*1000);

    /* loop */
    while(keep_running && !check_keep_file())
    {
        /* read data */
        while(ina226_voltage_read(&mV) != 0)
        {
            // 错误时将数值归为 -100
            ina226_voltage = -100.0f;

            // 重试大于5次退出
            if(read_retry >= 5)
            {
                pthread_detach(ina226_thread);   // 线程分离
                ina226_thread_running = 0;

                log_error("重试超过5次, 退出 ina226 线程");
                send_mail(11);

                // 清理退出
                (void)ina226_basic_deinit();
                pthread_exit(NULL);
            }

            // 尝试重新初始化
            if (ina226_basic_init(addr, r) != 0)
            {
                log_error("ina226 初始化失败");
            }

            log_warn("接收超时, 正在重试: %d", read_retry+1);
            read_retry++;
            usleep(1000*1000);
        }
        read_retry = 0;

        // 更新数值
        ina226_voltage = (volatile float)mV / 1000.0f;

        // 欠压、过压、缺电处理
        if (mV < Voltage_LOW && mV > Voltage_OFF) {
            vl_count++;
            if(vl_count > 2) {
                if(!lowV_task_lock) {
                    log_warn("警告：当前电池电压为 %.3fV!!!", mV / 1000.0f);

                    if(system("shutdown 10") != 0) {
                        log_error("系统异常?");
                    } else {
                        log_info("设置为10分钟后关机");
                    }

                    send_mail(20);

                    lowV_task_lock  = 1;
                    highV_task_lock = 0;
                    offV_task_lock  = 0;
                    norM_task_lock  = 0;
                }
                vl_count = vh_count = of_count = nm_count = 0;
            }
        } else if (mV > Voltage_HIGH) {
            vh_count++;
            if(vh_count > 2) {
                if(!highV_task_lock) {
                    log_warn("警告：当前电池电压为 %.3fV!!!", mV / 1000.0f);
                    send_mail(21);

                    lowV_task_lock  = 0;
                    highV_task_lock = 1;
                    offV_task_lock  = 0;
                    norM_task_lock  = 0;
                }
                vl_count = vh_count = of_count = nm_count = 0;
            }
        } else if (mV < Voltage_OFF) {
            of_count++;
            if(of_count > 1) {
                if(!offV_task_lock) {
                    log_warn("警告：当前电池电压为 %.3fV!!!", mV / 1000.0f);
                    send_mail(22);

                    lowV_task_lock  = 0;
                    highV_task_lock = 0;
                    offV_task_lock  = 1;
                    norM_task_lock  = 0;
                }
                vl_count = vh_count = of_count = nm_count = 0;
            }
        } else {
            nm_count++;
            if(nm_count > 2) {
                if(!norM_task_lock) {
                    log_info("当前电池电压已恢复正常: %.3fV!!!", mV / 1000.0f);

                    if(lowV_task_lock == 1) {
                        if(system("shutdown -c") != 0) {
                            log_error("系统异常?");
                        } else {
                            log_info("已取消关机");
                        }
                    }

                    send_mail(23);

                    lowV_task_lock  = 0;
                    highV_task_lock = 0;
                    offV_task_lock  = 0;
                    norM_task_lock  = 1;
                }
                vl_count = vh_count = of_count = nm_count = 0;
            }
        }

        usleep(500*1000);
    }

    (void)ina226_basic_deinit();
    log_info("退出 ina226 线程");

    pthread_exit(NULL);
}

// 创建或回收 ina226 线程
// 0: 创建; 1: 回收
int ina226_thread_handle(int handle)
{
    int res;

    // 创建线程
    if(!handle) {
        res = pthread_create(&ina226_thread, NULL, ina226_handler, NULL);
        if(res != 0) {
            ina226_thread_running = 0;
            log_error("创建 ina226 线程失败: %d", res);
            return 1;
        } else {
            ina226_thread_running = 1;
            log_info("创建 ina226 线程成功");
        }
    } else {
        if(ina226_thread_running != 0)
        {
            res = pthread_join(ina226_thread, NULL);
            if(res != 0) {
                log_error("退出 ina226 线程失败: %d", res);
                return 1;
            } else {
                ina226_thread_running = 0;
                log_info("退出 ina226 线程成功");
            }
        }
    }
    return 0;
}
