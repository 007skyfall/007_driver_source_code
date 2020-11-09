/*
//设备树信息

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

MODULE_LICENSE("GPL");

#define DRV_DEBUG
#ifdef  DRV_DEBUG
#define drv_pr(...)	do{ printk("\n\r [font] " __VA_ARGS__); }while(0)
#else
#define drv_pr(...)
#endif

struct i2c_client *golbal_client;

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

static int myi2c_read_reg(char reg)
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
	
	return 0;
}

static int mpu6050_remove(struct i2c_client *client)
{
	drv_pr("mpu6050 remove...\n");

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

