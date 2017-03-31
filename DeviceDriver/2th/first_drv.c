
//一个最简单的驱动程序至少要包含以下三个头文件
#include <linux/module.h>		//所有内核模块都必须包含这个头文件
#include <linux/kernel.h>		//使用内核优先级时要包含
#include <linux/device.h>	//一些初始化的函数 如:modole_init

#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/arch/regs-gpio.h>
#include <asm/hardware.h>

static struct class *firstdrv_class;
static struct class_device	*firstdrv_class_dev;

volatile unsigned long *gpbcon = NULL;
volatile unsigned long *gpbdat = NULL;

	/*
	 * 定义驱动级的open 函数
	 *
	 */
static int first_drv_open(struct inode *inode, struct file *file)
{
	//printk("first_drv_open\n");
	/*
	 * LED1 LED2 LED3 LED4  分别对应GPIOB5 GPIOB5 GPIOB7 GPIOB8
	 * 配置 LED1 LED2 LED3 LED4 为输出
	 */
	*gpbcon &= ~((0x3<<(5*2)) | (0x3<<(6*2)) | (0x3<<(7*2)) | (0x3<<(8*2)));
	*gpbcon |= ((0x1<<(5*2)) | (0x1<<(6*2)) | (0x1<<(7*2)) | (0x1<<(8*2)));

	return 0;
}


//定义驱动级的write函数
static ssize_t first_drv_write(struct file *file, const char __user *buf, size_t count, loff_t * ppos)
{
	int val;

	//printk("first_drv_write\n");

	copy_from_user(&val, buf, count); //	copy_to_user();

	if (val == 1)
	{
		// 清0
		*gpbdat &= ~((1<<5) | (1<<6) | (1<<7) | (1<<8));
	}
	else
	{
		// 置1
		*gpbdat |= (1<<5) | (1<<6) | (1<<7) | (1<<8);
	}

	return 0;
}

	//定义file_operations 结构体
	//该结构体声明位于/linux-2.6.22.6/include/linux/fs.h 中
static struct file_operations first_drv_fops = {
    .owner  =   THIS_MODULE,    /* 这是一个宏，指向编译模块自动创建的__this_module变量 */
    .open   =   first_drv_open,
		.write	=		first_drv_write,
};


int major;

//模块加载函数
static int first_drv_init(void)
{
	major = register_chrdev(0, "first_drv", &first_drv_fops); //注册

	firstdrv_class = class_create(THIS_MODULE, "firstdrv");

	firstdrv_class_dev = class_device_create(firstdrv_class, NULL, MKDEV(major, 0), NULL, "xyz"); /* /dev/xyz */

	gpbcon = (volatile unsigned long *)ioremap(0x56000010, 16);
	gpbdat = gpbcon + 1;

	return 0;
}

//模块卸载函数
static void first_drv_exit(void)
{
	unregister_chrdev(major, "first_drv"); // 卸载

	class_device_unregister(firstdrv_class_dev);
	class_destroy(firstdrv_class);
	iounmap(gpbcon);
}

//这两个宏定义声明模块的加载和卸载函数
//这两个的定义位于/linux-2.6.22.x/include/linux/init.h中
//模块注册
module_init(first_drv_init);
module_exit(first_drv_exit);


//模块许可声明
MODULE_LICENSE("GPL");