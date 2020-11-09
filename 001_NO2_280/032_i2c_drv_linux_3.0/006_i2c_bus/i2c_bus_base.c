
MODULE_LICENSE("GPL");

static int itop4412_i2c_xfer(struct i2c_adapter *adap,struct i2c_msg *msgs, int num)
{

	return 0;
}

static u32 itop4412_i2c_func(struct i2c_adapter *adap)
{
	return I2C_FUNC_I2C | I2C_FUNC_SMBUS_EMUL | I2C_FUNC_PROTOCOL_MANGLING;
}

static const struct i2c_algorithm itop4412_i2c_algo = {
//	.smbus_xfer     = ,
	.master_xfer	= itop4412_i2c_xfer,
	.functionality	= itop4412_i2c_func,
};

/* 1. ∑÷≈‰/…Ë÷√i2c_adapter
 */
static struct i2c_adapter itop4412_i2c_adapter = {
 .name			 = "itop",
 .algo			 = &itop4412_i2c_algo,
 .owner 		 = THIS_MODULE,
};

static int __init itop_i2c_bus_init(void)
{
	/* 2. ◊¢≤·i2c_adapter */

	i2c_add_adapter(&itop4412_i2c_adapter);
	
	return 0;
}

static void __exit itop_i2c_bus_exit(void)
{
	i2c_del_adapter(&itop4412_i2c_adapter);

	return ;
}

module_init(itop_i2c_bus_init);
module_exit(itop_i2c_bus_init);

