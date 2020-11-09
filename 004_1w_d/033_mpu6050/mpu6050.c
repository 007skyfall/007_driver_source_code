/*
//ËÆæÂ§áÊ†ë‰ø°ÊÅØ

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

static int mpu6050_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	drv_pr("mpu6050 probe...\n");
	
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
	//∞¥’’…Ë±∏ ˜–¥µƒ ±∫Ú£¨’‚∏ˆ≥…‘±±ÿ–Î±ªÃÓ≥‰£¨∑Ò‘Ú∆•≈‰≤ª…œ

};

module_i2c_driver(mpu6050_driver);

