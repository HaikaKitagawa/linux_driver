/*
 * @Author: Haika 1031168824@qq.com
 * @Date: 2026-05-23 18:18:17
 * @LastEditors: HaikaKitagawa 1031168824@qq.com
 * @LastEditTime: 2026-05-23 19:08:24
 * @FilePath: /linux_driver/01_chrdevbase/chrdevbase.c
 * @Description: A char device example.
 * 
 * Copyright (c) 2026 by Haika, All Rights Reserved. 
 */

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/init.h>
#include <linux/module.h>

#define CHARDEVBASE_MAJOR 200 /* 预设的主设备号 */
#define CHARDEVBASE_NAME "chardevbase" /* 设备名称 */

