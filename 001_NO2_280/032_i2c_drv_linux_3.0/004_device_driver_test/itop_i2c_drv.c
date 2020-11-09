#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include "mpu_cmd.h"

/*3.2 对iic从设备的配置工作*/
#define SAMPLE_RATE_DIV   		0X19
#define CONFIGURATION     		0X1A
#define GYRO_CONFIGURATION      0X1B
#define ACCEL_CONFIGURATION     0X1C
#define POWER_MANAGER_1         0X6B
/*数据寄存器*/
#define ACCEL_X_H      0X3B
#define ACCEL_X_L      0X3C
#define ACCEL_Y_H      0X3D
#define ACCEL_Y_L      0X3E
#define ACCEL_Z_H      0X3F
#define ACCEL_Z_L      0X40
#define TEMP_H        0X41
#define TEMP_L        0X42
#define GYRO_X_H      0X43
#define GYRO_X_L      0X44
#define GYRO_Y_H      0X45
#define GYRO_Y_L      0X46
#define GYRO_Z_H      0X47
#define GYRO_Z_L      0X48

MODULE_LICENSE("GPL");

#define DRV_DEBUG
#ifdef  DRV_DEBUG
#define drv_pr(...)	do{ printk("\n\r [font] " __VA_ARGS__); }while(0)
#else
#define drv_pr(...)
#endif

static int major;
static struct class 		*class 				= NULL;
static struct i2c_client 	*mpu_client			= NULL;

union mpu6050_data data;
/*3.1 对iic从设备的读写操作函数的封装*/
void  mpu6050_write_reg(struct i2c_client *client,unsigned char reg,unsigned char  data)
{
	int ret;	
	unsigned char tx[2] = {reg ,data};
	struct i2c_msg msgs[] = {
		[0] = { .addr  =client->addr , .flags = 0, .len = ARRAY_SIZE(tx) , .buf =tx }
	};

	ret = i2c_transfer(client->adapter,msgs,ARRAY_SIZE(msgs));
	if(ret < 0){
		printk("i2c_transfer fail...%s,%d\n",__func__,__LINE__);
	}
}
unsigned char mpu6050_read_reg(struct i2c_client *client,unsigned char reg)
{
	int ret;
	unsigned char tx = reg;
	unsigned char rx = 0;
	struct i2c_msg msgs[] = {
		[0] = {.addr = client->addr, .flags = 0 ,  .len = 1, .buf = &tx},
		[1] = {.addr = client->addr, .flags = I2C_M_RD,  .len = 1, .buf = &rx},
	};
	ret = i2c_transfer(client->adapter,msgs,ARRAY_SIZE(msgs));
	if(ret < 0){
		printk("i2c_transfer fail...%s,%d\n",__func__,__LINE__);
	}
	return rx;
}

int mpu_open(struct inode *inode, struct file *file)
{
	printk("%s,%d\n",__func__,__LINE__);
	return 0;
}
int mpu_release(struct inode *inode, struct file *file)
{
	printk("%s,%d\n",__func__,__LINE__);
	return 0;
}

long mpu_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	switch(cmd){
		case ACCEL:
			data.acc.x  = mpu6050_read_reg(mpu_client,ACCEL_X_H)<<8;
			data.acc.x |= mpu6050_read_reg(mpu_client,ACCEL_X_L);
			data.acc.y  = mpu6050_read_reg(mpu_client,ACCEL_Y_H)<<8;
			data.acc.y |= mpu6050_read_reg(mpu_client,ACCEL_Y_L);
			data.acc.z  = mpu6050_read_reg(mpu_client,ACCEL_Z_H)<<8;
			data.acc.z |= mpu6050_read_reg(mpu_client,ACCEL_Z_L);
			break;
		case GYRO:
			data.gyro.x  = mpu6050_read_reg(mpu_client,GYRO_X_H)<<8;
			data.gyro.x |= mpu6050_read_reg(mpu_client,GYRO_X_L);
			data.gyro.y  = mpu6050_read_reg(mpu_client,GYRO_Y_H)<<8;
			data.gyro.y |= mpu6050_read_reg(mpu_client,GYRO_Y_L);
			data.gyro.z  = mpu6050_read_reg(mpu_client,GYRO_Z_H)<<8;
			data.gyro.z |= mpu6050_read_reg(mpu_client,GYRO_Z_L);
			break;
		case TEMP:
			data.temp  = mpu6050_read_reg(mpu_client,TEMP_H)<<8;
			data.temp |= mpu6050_read_reg(mpu_client,TEMP_L);
			break;
		default:
			printk("default ....%s,%d\n",__func__,__LINE__);
			break;
	}
	if(copy_to_user((void*)arg , &data ,sizeof(data))){
		printk("copy_to_user ...%s,%d\n",__func__,__LINE__);
	}
	//printk("%s,%d\n",__func__,__LINE__);
	return 0;
}


static struct file_operations itop_i2c_fops = {
	.owner			 = THIS_MODULE,
	.unlocked_ioctl  = mpu_ioctl,
	.open			 = mpu_open,
	.release 		 = mpu_release,
};

static int __devinit itop_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	mpu_client = client;
		
	printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
	
	mpu6050_write_reg(client,POWER_MANAGER_1,0X00);
	mpu6050_write_reg(client,SAMPLE_RATE_DIV,0X07);
	mpu6050_write_reg(client,CONFIGURATION,0X00);
	mpu6050_write_reg(client,GYRO_CONFIGURATION,0X18);/*-2000   +2000*/
	mpu6050_write_reg(client,ACCEL_CONFIGURATION,0X18);/* -16g 16g*/
	major = register_chrdev(0, "itop_i2c", &itop_i2c_fops);
	class = class_create(THIS_MODULE, "itop_i2c");
	device_create(class, NULL, MKDEV(major, 0), NULL, "itop_i2c"); /* /dev/itop_i2c */
	
	return 0;
}

static int __devexit itop_i2c_remove(struct i2c_client *client)
{
	drv_pr("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);

	device_destroy(class, MKDEV(major, 0));
	class_destroy(class);
	unregister_chrdev(major, "itop_i2c");
		
	return 0;
}

static const struct i2c_device_id itop_i2c_id_table[] = {
	{ "at24c04", 0 },
	{ "mpu6050", 0 },
	{}
};


/* 1. 分配/设置i2c_driver */
static struct i2c_driver itop_i2c_driver = {
	.driver	= {
		.name	= "itop",
		.owner	= THIS_MODULE,
	},
	.probe		= itop_i2c_probe,
	.remove		= __devexit_p(itop_i2c_remove),
	.id_table	= itop_i2c_id_table,
};

static int __init itop_i2c_drv_init(void)
{
	/* 2. 注册i2c_driver */
	i2c_add_driver(&itop_i2c_driver);
	
	return 0;
}

static void __exit itop_i2c_drv_exit(void)
{
	i2c_del_driver(&itop_i2c_driver);

	return ;
}

module_init(itop_i2c_drv_init);
module_exit(itop_i2c_drv_exit);

