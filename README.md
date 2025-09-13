# aht30_ina226
使用 aht30 & ina226 还有 Linux 系统(开发板)来监测家里的温湿度和 UPS 电池状态。
<br>
数据默认写入`/tmp/aht30_ina226_data`和`/tmp/aht30_data_2`，数据格式：
```bash
T=32.51
H=75.6
V=8.291
# 错误时清空，写入 'ERROR'
```

### 参考/使用的项目:
- https://github.com/libdriver/aht30 
- https://github.com/libdriver/ina226
- https://github.com/rxi/log.c

### 使用:
```bash
# 程序默认在 root 用户下使用
# 配置详见 config.h
# IIC_DEVICE_PORT_1 对应 /tmp/aht30_ina226_data
# IIC_DEVICE_PORT_2 对应 /tmp/aht30_data_2
# aht30 和 ina226 共用一个串口
# send_mail.sh 见 https://github.com/kskshaf/im1281b_home_power
make
./aht30_ina226
```
