/*
 * @Author: HaikaKitagawa 1031168824@qq.com
 * @Date: 2026-05-26 01:08:52
 * @LastEditors: HaikaKitagawa 1031168824@qq.com
 * @LastEditTime: 2026-05-26 01:25:09
 * @FilePath: /linux_driver/02_led/ledAPP.c
 * @Description: 
  LED灯控制应用程序
  通过命令行参数控制LED灯的开关状态
 * 
 * Copyright (c) 2026 by HaikaKitagawa, All Rights Reserved. 
 */
#include "stdio.h"
#include "unistd.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "string.h"
#include "stdlib.h"

#define LEDON           1
#define LEDOFF          0

int main(int argc, char *argv[])
{
    int fd, retvalue;
    char *filename;
    unsigned char data_buf[1];

    if(argc != 3)
    {
        printf("Error Usage!\r\n");
        return -1;
    }

    filename = argv[1];

    fd = open(filename, O_RDWR);
    if(fd < 0)
    {
        printf("Can't open file %s\r\n", filename);
        return -1;
    }

    data_buf[0] = atoi(argv[2]);

    retvalue = write(fd, data_buf, sizeof(data_buf));
    if(retvalue < 0)
    {
        printf("write data failed!\r\n");
        close(fd);
        return -1; 
    }

    retvalue = close(fd);
    if(retvalue < 0)
    {
        printf("Can't close file %s\r\n", filename);
        return -1;
    }
    return 0;
}