/*
 * @Author: HaikaKitagawa a1031168824@gmail.com
 * @Date: 2026-05-28 15:42:26
 * @LastEditors: HaikaKitagawa a1031168824@gmail.com
 * @LastEditTime: 2026-05-28 16:45:28
 * @FilePath: /linux_driver/03_newchrled/newchrled.c
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
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#define NEWCHRLED_CNT 1
#define NEWCHRLED_NAME "newchrled"

// #define LED_MAJOR       200
// #define LED_NAME        "led"

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

// LED设备结构体
struct newchrled_dev {
    dev_t devid;
    struct cdev cdev;
    struct class *class;
    struct device *device;
    int major;
    int minor;
} newchrled;

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

    if (newchrled.major) {
        newchrled.devid = MKDEV(newchrled.major, 0);
        retvalue = register_chrdev_region(newchrled.devid, NEWCHRLED_CNT, 
                                            NEWCHRLED_NAME);
        if (retvalue < 0) {
            pr_err("cannot register %s char driver {ret=%d}\n", NEWCHRLED_NAME, retvalue);
            goto fail_map;
        }
    }
    else 
    {
        retvalue = alloc_chrdev_region(&newchrled.devid, 0, NEWCHRLED_CNT,
                                            NEWCHRLED_NAME);
        if (retvalue <0)
        {
            pr_err("%s Couldn;t alloc_chrdev_region, ret=%d\n", NEWCHRLED_NAME, retvalue);
            goto fail_map;
        }
        newchrled.major = MAJOR(newchrled.devid);
        newchrled.minor = MINOR(newchrled.devid);
    }

    printk("newchrled major=%d, minor=%d\r\n", newchrled.major, newchrled.minor);

    newchrled.cdev.owner = THIS_MODULE;
    cdev_init(&newchrled.cdev, &led_fops);

    retvalue = cdev_add(&newchrled.cdev, newchrled.devid, NEWCHRLED_CNT);

    if (retvalue < 0)
    {
        goto del_unregister;
    }

    newchrled.class = class_create(THIS_MODULE, NEWCHRLED_NAME);
    if (IS_ERR(newchrled.class)) {
        goto del_cdev;
    }

    newchrled.device = device_create(newchrled.class, NULL, newchrled.devid, 
                                        NULL, NEWCHRLED_NAME);
    if (IS_ERR(newchrled.device)) {
        goto destroy_class;
    }

    return 0;

destroy_class:
    class_destroy(newchrled.class);

del_cdev:
    cdev_del(&newchrled.cdev);

del_unregister:
    unregister_chrdev_region(newchrled.devid, NEWCHRLED_CNT);

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
    
    cdev_del(&newchrled.cdev);
    unregister_chrdev_region(newchrled.devid, NEWCHRLED_CNT);
    device_destroy(newchrled.class, newchrled.devid);
    class_destroy(newchrled.class);
}

module_init(led_init);
module_exit(led_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("HaikaKitagawa");
MODULE_INFO(intree, "Y");