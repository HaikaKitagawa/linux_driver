/*
 * @Author: HaikaKitagawa 1031168824@qq.com
 * @Date: 2026-05-23 18:18:17
 * @LastEditors: HaikaKitagawa 1031168824@qq.com
 * @LastEditTime: 2026-05-25 00:44:01
 * @FilePath: /linux_driver/01_chrdevbase/chrdevbase.c
 * @Description: Linux字符设备驱动程序示例代码，演示了如何实现一个简单的字符设备驱动，包括设备的打开、读取、写入和释放操作。该驱动程序注册了一个字符设备，并提供了相应的文件操作函数，以便用户空间程序可以通过设备文件与内核进行交互。
 * 
 * Copyright (c) 2026 by HaikaKitagawa, All Rights Reserved. 
 */


#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
// #include <linux/ide.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/uaccess.h>
#include <linux/module.h>

#define CHRDEVBASE_MAJOR 200 /* 预设的主设备号 */
#define CHRDEVBASE_NAME "chrdevbase" /* 设备名称 */

static char read_buf[100]; /* 读缓冲区 */
static char write_buf[100]; /* 写缓冲区 */
static char kerneldata[] = {"kernel data!"}; /* 内核数据 */

/**
 * @description: 打开设备
 * @param {inode} *inode 
 * @param {file} *filp 
 * @return {*}
 */
static int chrdevbase_open(struct inode *inode, struct file *filp)
{
    // printk("chrdevbase open!\r\n");
    return 0;
}

/**
 * @description: 读取设备数据
 * @param {file} *filp 要打开的设备文件
 * @param {char __user} *buf 返回给用户空间的缓冲区
 * @param {size_t} cnt 要读取的字节数
 * @param {loff_t} *offt 相对于文件首地址的偏移量
 * @return {*}  读取的字节数，如果为负值则表示读取失败
 */
static ssize_t chrdevbase_read(struct file *filp, char __user *buf, 
                                size_t cnt, loff_t *offt)
{
    int retvalue = 0;

    /*向用户空间发送数据*/
    memcpy(read_buf, kerneldata, sizeof(kerneldata)); /* 将内核数据复制到读缓冲区 */
    retvalue = copy_to_user(buf, read_buf, sizeof(read_buf)); /* 将读缓冲区的数据复制到用户空间 */
    if (retvalue == 0)
    {
        printk("kernel senddata ok!\r\n");
    }
    else
    {
        printk("kernel senddata failed!\r\n");
    }

    return 0;
}

/**
 * @description: 写入设备数据
 * @param {file} *filp 要打开的设备文件
 * @param {const char __user} *buf 要写入的数据缓冲区
 * @param {size_t} cnt 要写入的字节数
 * @param {loff_t} *offt 相对于文件首地址的偏移量
 * @return {*}  写入的字节数，如果为负值则表示写入失败
 */
static ssize_t chrdevbase_write(struct file *filp, const char __user *buf,
                                size_t cnt, loff_t *offt)
{
    int retvalue = 0;

    /* 从用户空间接收数据 */
    retvalue = copy_from_user(write_buf, buf, cnt);
    if (retvalue == 0)
    {
        printk("kernel receivedata ok! data = %s\r\n", write_buf);
    }
    else
    {
        printk("kernel receivedata failed!\r\n");
    }

    return 0;
}

/**
 * @description: 释放设备
 * @param {inode} *inode 要打开的设备文件
 * @param {file} *filp 要打开的设备文件
 * @return {*}
 */
static int chrdevbase_release(struct inode *inode, struct file *filp)
{
    // printk("chrdevbase release!\r\n");
    return 0;
}

/* 设备操作函数结构体 */
static struct file_operations chrdevbase_fops = {
    .owner = THIS_MODULE,
    .open = chrdevbase_open,
    .read = chrdevbase_read,
    .write = chrdevbase_write,
    .release = chrdevbase_release,
};

/**
 * @description: 驱动入口函数
 * @return {*}
 */
static int __init chrdevbase_init(void)
{
    int retvalue = 0;
    /* 注册字符设备驱动 */
    retvalue = register_chrdev(CHRDEVBASE_MAJOR, CHRDEVBASE_NAME,
                                &chrdevbase_fops);
    if (retvalue < 0)
    {
        printk("chrdevbase driver register failed!\r\n");
    }
    printk("chrdevbase_init() \r\n");
    return 0;
}

/**
 * @description: 驱动出口函数
 * @return {*}
 */
static void __exit chrdevbase_exit(void)
{
    /* 注销字符设备驱动 */
    unregister_chrdev(CHRDEVBASE_MAJOR, CHRDEVBASE_NAME);
    printk("chrdevbase_exit() \r\n");
}

module_init(chrdevbase_init);
module_exit(chrdevbase_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("HaikaKitagawa");
MODULE_INFO(intree, "Y");
