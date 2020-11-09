/*

i2c@138D0000 {
 #address-cells = <1>;
 #size-cells = <0>;
 samsung,i2c-sda-delay = <100>;
 samsung,i2c-max-bus-freq = <20000>;
 pinctrl-0 = <&i2c7_bus>;
 pinctrl-names = "default";
 status = "okay";

 mpu6050@68 {
         compatible = "samsung,mpu6050";
         reg = <0x68>;
         };
 };
*/
	

#include <linux/module.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/device.h>

MODULE_LICENSE("GPL");

#define DRV_DEBUG
#ifdef  DRV_DEBUG
#define drv_pr(...)	do{ printk("\n\r [font] " __VA_ARGS__); }while(0)
#else
#define drv_pr(...)
#endif



/**********************mpu6050reg********************************/
#define SMPLRT_DIV 0x19 //采样率分频，典型值： 0x07(125Hz) */  
#define CONFIG 0x1A // 低通滤波频率，典型值： 0x06(5Hz) */  
#define GYRO_CONFIG 0x1B // 陀螺仪自检及测量范围，典型值： 0x18(不自检，2000deg/s) */  
#define ACCEL_CONFIG 0x1C // 加速计自检、测量范围及高通滤波频率，典型值： 0x01(不自检， 2G， 5Hz) */  
#define ACCEL_XOUT_H 0x3B // 存储最近的 X 轴、 Y 轴、 Z 轴加速度感应器的测量值 */  
#define ACCEL_XOUT_L 0x3C  
#define ACCEL_YOUT_H 0x3D  
#define ACCEL_YOUT_L 0x3E  
#define ACCEL_ZOUT_H 0x3F  
#define ACCEL_ZOUT_L 0x40  
#define TEMP_OUT_H 0x41 // 存储的最近温度传感器的测量值 */  
#define TEMP_OUT_L 0x42  
#define GYRO_XOUT_H 0x43 // 存储最近的 X 轴、 Y 轴、 Z 轴陀螺仪感应器的测量值 */  
#define GYRO_XOUT_L 0x44  
#define GYRO_YOUT_H 0x45  
#define GYRO_YOUT_L 0x46  
#define GYRO_ZOUT_H 0x47  
#define GYRO_ZOUT_L 0x48  
#define PWR_MGMT_1 0x6B // 电源管理，典型值： 0x00(正常启用) */  
#define WHO_AM_I 0x75 //IIC 地址寄存器(默认数值 0x68，只读) */  
/*****************************************************************/

struct i2c_client *golbal_client;
int major = 0;
struct class *cls;
struct device *dev;
char kbuf[6] = {0};

static int myi2c_write_reg(char reg,char data)
{
	int ret;
	char w_buf[] = {reg,data};
	struct i2c_msg w_msg[] = {
		[0] = {
			.addr  = golbal_client->addr,
			.flags = 0,
			.len   = 2,
			.buf   = w_buf,
			
		},
		
	};
	
	ret = i2c_transfer(golbal_client->adapter,w_msg, ARRAY_SIZE(w_msg));
	if(ret != 1){
		drv_pr("write msg send error\n");
		return -EAGAIN;
		
	}
	return 0;
}

static  int myi2c_read_reg(char reg)
{
	int ret;
	char val;
	char r_buf[] = {reg};
	struct i2c_msg r_msg[] = {
		[0] = {
			.addr  = golbal_client->addr,
			.flags = 0,
			.len   = 1,
			.buf   = r_buf,
			
		},
		[1] = {
			.addr  = golbal_client->addr,
			.flags = 1,
			.len   = 1,
			.buf   = &val,
			
		},
		
	};
	ret = i2c_transfer(golbal_client->adapter,r_msg, ARRAY_SIZE(r_msg));
	if(ret != 2){
		drv_pr("read msg send error\n");
		return -EAGAIN;
		
	}
	
	return val;
}

static int mpu6050_open(struct inode *inode, struct file *file)
{
	drv_pr("%s:%s:%d\n",__FILE__,__func__,__LINE__);
	
	return 0;
}

static ssize_t mpu6050_read(struct file *file, char __user *ubuf, size_t size, loff_t *offs)
{
	int ret;
	//printk("%s:%s:%d\n",__FILE__,__func__,__LINE__);

	//读取数据
	kbuf[0] = myi2c_read_reg(ACCEL_XOUT_H);
	kbuf[1] = myi2c_read_reg(ACCEL_XOUT_L);
	kbuf[2] = myi2c_read_reg(ACCEL_YOUT_H);
	kbuf[3] = myi2c_read_reg(ACCEL_YOUT_L); 
	kbuf[4] = myi2c_read_reg(ACCEL_ZOUT_H);
	kbuf[5] = myi2c_read_reg(ACCEL_ZOUT_L);
	
	//将数据拷贝到用户空间
	if(size > sizeof(kbuf)) size = sizeof(kbuf);

	ret = copy_to_user(ubuf,(void *)kbuf,size);
	if(ret){
		printk("copy data to user error\n");
		return ret;
	}
	
	return size;
}

static int mpu6050_close(struct inode *inode, struct file *file)
{
	drv_pr("%s:%s:%d\n",__FILE__,__func__,__LINE__);

	return 0;
}

static const struct file_operations fops = {
	.open = mpu6050_open,
	.read = mpu6050_read,
	.release = mpu6050_close,
};

static int mpu6050_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int ret;
	golbal_client = client;
	drv_pr("mpu6050 probe...\n");
	
	ret = myi2c_read_reg(0x75);
	if(ret != 0x68){
		drv_pr("mpu6050 access data error\n");
		return -EAGAIN;
	}
	drv_pr("i2c read WHO_AM_I success.\n");

	//初始化mpu6050
	myi2c_write_reg(PWR_MGMT_1,0x40);  //复位mpu6050
	myi2c_write_reg(PWR_MGMT_1,0x01);  //唤醒mpu6050
	myi2c_write_reg(GYRO_CONFIG,0x18);  //设置陀螺仪土2000/s
	myi2c_write_reg(ACCEL_CONFIG,0x01);
	myi2c_write_reg(SMPLRT_DIV,0x07);
	myi2c_write_reg(CONFIG,0x06);

	//1.注册字符设备驱动
	major = register_chrdev(0,"mpu6050",&fops);
	if(major <= 0){
		drv_pr("reigster char device error\n");
		return -EAGAIN;
	}
	
	//2.自动创建设备节点
	cls = class_create(THIS_MODULE,"mpu6050");
	if(IS_ERR(cls)){
		drv_pr("create class errror\n");
		return PTR_ERR(cls);
	}

	dev = device_create(cls,NULL,MKDEV(major,0),NULL,"mpu6050");
	if(IS_ERR(dev)){
		drv_pr("device class errror\n");
		return PTR_ERR(dev);
	}
	
	return 0;
}

static int mpu6050_remove(struct i2c_client *client)
{
	drv_pr("mpu6050 remove...\n");
	device_destroy(cls,MKDEV(major,0));
	class_destroy(cls);
	unregister_chrdev(major,"mpu6050");
	
	return 0;

}

static const struct of_device_id mpu6050_of_match_table[] = {
	{.compatible = "samsung,mpu6050",},
	{},
};
static const struct i2c_device_id mpu6050_id_table[] = {
	{"hello",},
	{},
};

static struct i2c_driver mpu6050_driver = {
	.probe = mpu6050_probe,
	.remove = mpu6050_remove,
	.driver = {
		.name = "hello",
		.owner = THIS_MODULE,
		.of_match_table = mpu6050_of_match_table,
	},
	.id_table = mpu6050_id_table, 
	//按照设备树写的时候，这个成员必须被填充，否则匹配不上

};

module_i2c_driver(mpu6050_driver);

