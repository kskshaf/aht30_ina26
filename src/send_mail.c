#include "send_mail.h"
#include "log.h"

void send_mail(int num)
{
    switch (num)
    {
        case 11:
            if(system("/root/send_mail.sh 11") != 0) log_error("邮件发送异常!");
            break;
        case 12:
            if(system("/root/send_mail.sh 12") != 0) log_error("邮件发送异常!");
            break;
        case 20:
            if(system("/root/send_mail.sh 20") != 0) log_error("邮件发送异常!");
            break;
        case 21:
            if(system("/root/send_mail.sh 21") != 0) log_error("邮件发送异常!");
            break;
        case 22:
            if(system("/root/send_mail.sh 22") != 0) log_error("邮件发送异常!");
            break;
        case 23:
            if(system("/root/send_mail.sh 23") != 0) log_error("邮件发送异常!");
            break;
        case 30:
            if(system("/root/send_mail.sh 30") != 0) log_error("邮件发送异常!");
            break;
        case 31:
            if(system("/root/send_mail.sh 31") != 0) log_error("邮件发送异常!");
            break;
        default:
            break;
    }
}