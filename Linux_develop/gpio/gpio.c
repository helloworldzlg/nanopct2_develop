/******************************************************************************

                  版权所有 (C), 2001-2011, RobotAI.club: zlg

 ******************************************************************************
  文 件 名   : gpio.c
  版 本 号   : 初稿
  作    者   : zlg
  生成日期   : 2016年5月25日
  最近修改   :
  功能描述   : 
  函数列表   :
              gpioc4_cleanup
              gpioc4_init
              gpioc4_open
              gpioc4_release
              gpioc4_setup_cdev
              io_init
              led_off
              led_on
              PC04_intHandle
  修改历史   :
  1.日    期   : 2016年5月25日
    作    者   : zlg
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 外部变量说明                                 *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 外部函数原型说明                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 全局变量                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/ioctl.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <asm/arch/hardware.h>
#include <asm/arch/gpio.h>
#include <linux/interrupt.h>
#include <asm/io.h>
#include <asm/arch/board.h>
#include <linux/cdev.h>
#include <asm/arch/gpio.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <mach/platform.h>
#include <mach/devices.h>
#include <mach/soc.h>

void led_on()
{    
     printk(KERN_NOTICE, "value is 1\n");
}

void led_off()
{    
     printk(KERN_NOTICE, "value is 0\n");
}

struct light_dev
{
    struct cdev cdev;
    unsigned char value;
};

struct light_dev *gpioc4_devp;

int gpioc4_major = 200;

MODULE_AUTHOR("zlg");
MODULE_LICENSE("Dual BSD/GPL");

static void io_init(void)
{
    /* GPIO_C4设置为input */
    NX_GPIO_SetOutputEnable(PAD_GPIO_C, 4, CFALSE);

    NX_GPIO_SetInterruptMode(PAD_GPIO_C, 4, NX_GPIO_INTMODE_HIGHLEVEL);
    NX_GPIO_SetInterruptEnable(PAD_GPIO_C, 4, CTRUE);

    return;
}

struct gpio_irq_desc
{
    int irq;
    unsigned long flags;
    char *name;
};

static struct gpio_irq_desc PC04_IRQ = {PAD_GPIO_C + 4, NX_GPIO_INTMODE_HIGHLEVEL, "PC04"};

static irqreturn_t PC04_intHandle(int irq, void *dev_id)
{
    led_on();
    return IRQ_RETVAL(IRQ_HANDLED);
}

int gpioc4_open(struct inode *inode, struct file *filp)
{
    int err;
    struct light_dev *dev;
    dev = container_of(inode->i_cdev, struct light_dev, cdev);
    filp->private_data = dev;
    io_init();
    err = request_irq(PC04_IRQ.irq, PC04_intHandle, PC04_IRQ.flags, PC04_IRQ.name, (void*)0);
    if (err) 
    {
        free_irq(PC04_IRQ.irq,(void*)0);
        return  -EBUSY;
    }
    
    return 0;
}

int gpioc4_release(struct inode *inode,struct file *filp)
{
    free_irq(PC04_IRQ.irq, (void*)0);
    return 0;    
}

struct file_operations gpioc4_fops = 
{
    .owner   = THIS_MODULE,
    .open    = gpioc4_open,
    .release = gpioc4_release,
};

static void gpioc4_setup_cdev(struct light_dev *dev,int index)
{
    int err,devno = MKDEV(gpioc4_major,index);

    cdev_init(&dev->cdev, &gpioc4_fops);
    dev->cdev.owner = THIS_MODULE;
    dev->cdev.ops   = &gpioc4_fops;

    err = cdev_add(&dev->cdev, devno,1);

    if (err)
    {
        printk(KERN_NOTICE "Error %d adding LED%d",err,index);
    }
}

int gpioc4_init(void)
{
    int result;

    dev_t dev = MKDEV(gpioc4_major, 0);
    if (gpioc4_major)
    {        
        result = register_chrdev_region(dev, 1, "PC04_IRQTest");
    }

    if (result < 0)
    {
        return result;
    }

    gpioc4_devp = kmalloc(sizeof(struct light_dev), GFP_KERNEL);
    if (!gpioc4_devp)
    {
        result = - ENOMEM;
        goto fail_malloc;
    }

    memset(gpioc4_devp, 0, sizeof(struct light_dev));
    gpioc4_setup_cdev(gpioc4_devp, 0);
    
    return 0;

    fail_malloc:unregister_chrdev_region(dev, gpioc4_devp);
    return result;    
}

void gpioc4_cleanup(void)
{
    cdev_del(&gpioc4_devp->cdev);   
    kfree(gpioc4_devp);
    unregister_chrdev_region(MKDEV(gpioc4_major,0), 1);  
}

module_init(gpioc4_init);
module_exit(gpioc4_cleanup);