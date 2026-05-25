/*
 * @Author: HaikaKitagawa 1031168824@qq.com
 * @Date: 2026-05-25 22:00:22
 * @LastEditors: HaikaKitagawa 1031168824@qq.com
 * @LastEditTime: 2026-05-26 01:47:13
 * @FilePath: /linux_driver/02_led/led.c
 * @Description: 
  Linux内核模块：LED灯控制
  通过GPIO接口控制LED灯的开关状态
 * 
 * Copyright (c) 2026 by HaikaKitagawa, All Rights Reserved. 
 */

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <asm/uaccess.h>
#include <linux/fs.h>
#include <asm/io.h>

#define LED_MAJOR       200
#define LED_NAME        "led"

#define LEDON           1
#define LEDOFF          0

// #define PMU_GRF_BASE    0xFF660000
#define GPIO1_IOC_BASE    0xFF660000
#define GPIO1_IOC_GPIO1B_IOMUX_SEL_0    (GPIO1_IOC_BASE + 0x0028)

#define GPIO1_IOC_GPIO1B_DS_1           (GPIO1_IOC_BASE + 0x0154)

#define GPIO1_BASE      0xFF870000
#define GPIO1_SWPORT_DR_L  (GPIO1_BASE + 0x0000)
#define GPIO1_SWPORT_DDR_L (GPIO1_BASE + 0x0008)

static void __iomem *GPIO1_IOC_GPIO1B_IOMUX_SEL_0_PI;
static void __iomem *GPIO1_IOC_GPIO1B_DS_1_PI;
static void __iomem *GPIO1_SWPORT_DR_L_PI;
static void __iomem *GPIO1_SWPORT_DDR_L_PI;

/**
 * @description: 切换LED状态
 * @param {u8} sta LED状态，LEDON表示开，LEDOFF表示关
 * @return {*}
 */
void led_switch(u8 sta)
{
    u32 val = 0;
    if(sta == LEDON) 
    {
        val = readl(GPIO1_SWPORT_DR_L_PI);
        val &= ~(0x8 << (0 + 8));
        val |= ((0x8 << (16 + 8)) | (0x8 << (0 + 8)));
        writel(val, GPIO1_SWPORT_DR_L_PI);
    }
    else if(sta == LEDOFF)
    {
        val = readl(GPIO1_SWPORT_DR_L_PI);
        val &= ~(0x8 << (0 + 8));
        val |= ((0x8 << (16 + 8)));
        writel(val, GPIO1_SWPORT_DR_L_PI);
    }
}

/**
 * @description: 重新映射GPIO寄存器
 * @return {*}
 */
void led_remap(void)
{
    GPIO1_IOC_GPIO1B_IOMUX_SEL_0_PI = ioremap(GPIO1_IOC_GPIO1B_IOMUX_SEL_0, 4);
    GPIO1_IOC_GPIO1B_DS_1_PI = ioremap(GPIO1_IOC_GPIO1B_DS_1, 4);
    GPIO1_SWPORT_DR_L_PI = ioremap(GPIO1_SWPORT_DR_L, 4);
    GPIO1_SWPORT_DDR_L_PI = ioremap(GPIO1_SWPORT_DDR_L, 4);
}

/**
 * @description: 取消GPIO寄存器映射
 * @return {*}
 */
void led_unmap(void)
{
    iounmap(GPIO1_IOC_GPIO1B_IOMUX_SEL_0_PI);
    iounmap(GPIO1_IOC_GPIO1B_DS_1_PI);
    iounmap(GPIO1_SWPORT_DR_L_PI);
    iounmap(GPIO1_SWPORT_DDR_L_PI);
}

/**
 * @description: 读取LED状态
 * @return {*}
 */
static ssize_t led_read(struct file *filp, char __user *buf,
                        size_t cnt, loff_t *offt)
{
    return 0;
}

/**
 * @description: 写入LED状态
 * @return {*}
 */
static ssize_t led_write(struct file *filp, const char __user *buf,
                        size_t cnt, loff_t *offt)
{
    int retvalue;
    unsigned char data_buf[1];
    unsigned char led_status;

    retvalue = copy_from_user(data_buf, buf, cnt);
    if(retvalue < 0)
    {
        printk("kernel write failed!\r\n");
        return -EFAULT;
    }

    led_status = data_buf[0];
    if(led_status == LEDON)
    {
        led_switch(LEDON);
    }
    else if(led_status == LEDOFF)
    {
        led_switch(LEDOFF);
    }
    return 0;
}

/**
 * @description: 释放LED设备
 * @param {inode} *inode
 * @param {file} *filp
 * @return {*}
 */
static int led_release(struct inode *inode, struct file *filp)
{
    return 0;
}

/**
 * @description: 打开LED设备
 * @param {inode} *inode
 * @param {file} *filp
 * @return {*}
 */
static int led_open(struct inode *inode, struct file *filp)
{
    return 0;
}

/**
 * @description: LED设备文件操作结构体
 * @return {*}
 */
static struct file_operations led_fops = {
    .owner = THIS_MODULE,
    .read = led_read,
    .write = led_write,
    .open = led_open,
    .release = led_release,
};

/**
 * @description: 初始化LED设备
 * @return {*}
 */
static int __init led_init(void)
{
    int retvalue = 0;
    u32 val = 0;

    led_remap();

    // 配置GPIO1B3的IOMUX为GPIO功能
    val = readl(GPIO1_IOC_GPIO1B_IOMUX_SEL_0_PI);
    val &= ~(0xF000 << 0);
    val |= ((0xF000 << 16) | (0x0 << 0));
    writel(val, GPIO1_IOC_GPIO1B_IOMUX_SEL_0_PI);

    // 配置GPIO1B3的驱动能力
    val = readl(GPIO1_IOC_GPIO1B_DS_1_PI);
    val &= ~(0x3F00 << 0);
    val |= ((0x3F00 << 16) | (0x0F00 << 0));
    writel(val, GPIO1_IOC_GPIO1B_DS_1_PI);

    // 配置GPIO1B3为输出模式
    val = readl(GPIO1_SWPORT_DDR_L_PI);
    val &= ~(0x8 << (0 + 8));
    val |= ((0x8 << (16 + 8)) | (0x8 << (0 + 8)));
    writel(val, GPIO1_SWPORT_DDR_L_PI);

    // 默认关闭LED灯
    val = readl(GPIO1_SWPORT_DR_L_PI);
    val &= ~(0x8 << (0 + 8));
    val |= (0x8 << (16 + 8));
    writel(val, GPIO1_SWPORT_DR_L_PI);

    retvalue = register_chrdev(LED_MAJOR, LED_NAME, &led_fops);
    if(retvalue < 0)
    {
        printk("register led failed!\r\n");
        goto fail_map;
    }
    return 0;

    fail_map:
        led_unmap();
        return -EIO;
}

/**
 * @description: 退出LED设备
 * @return {*}
 */
static void __exit led_exit(void)
{
    led_unmap();
    unregister_chrdev(LED_MAJOR, LED_NAME);
}

module_init(led_init);
module_exit(led_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("HaikaKitagawa");
MODULE_INFO(intree, "Y");