#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/of_irq.h>
#include <linux/workqueue.h>
#include <linux/spinlock.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/input/mt.h>

#define MAX_TOUCH_POINT 5

struct ts_data{
	struct input_dev *ts_input;
	struct i2c_client *client;
	struct work_struct ts_work;
	spinlock_t ts_spinlock;
	int irq_num;
	unsigned int init_base_addr;
	unsigned int *GPX0CON;
	int data_base_addr;
	int touch_num;
	int data_buf[40];
};

struct ts_data *ts;

unsigned char ts_cfg_info[106] = {
	0x12,0x10,0x0E,0x0C,0x0A,0x08,0x06,0x04,0x02,0x00,0xE0,0x00,0xD0,0x00,
	0xC0,0x00,0xB0,0x00,0xA0,0x00,0x90,0x00,0x80,0x00,0x70,0x00,0x60,0x00,
	0x50,0x00,0x40,0x00,0x30,0x00,0x20,0x00,0x10,0x00,0x00,0x00,0xF0,0x00,
	0x1B,0x03,0x00,0x00,0x00,0x16,0x16,0x16,0x0F,0x0F,0x0A,0x45,0x32,0x05,
	0x03,0x00,0x05,0x00,0x04,0x58,0x02,0x00,0x00,0x34,0x2C,0x37,0x2E,0x00,
	0x00,0x14,0x14,0x01,0x0A,0x00,0x00,0x00,0x00,0x00,0x14,0x10,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x14,0x1F,0x18,
	0x1A,0x00,0x00,0x00,0x00,0x00,0x00,0x01
};

char gt818_i2c_read(int reg)
{
	char ret,val;
	char reg_l = reg & 0xff;
	char reg_h = (reg >> 8) & 0xff;
	char w_buf[] = {reg_h,reg_l}; 
	struct i2c_msg read_reg[] = {
		[0] = {
			.addr = ts->client->addr,
			.flags = 0, //先写
			.len = 2,
			.buf = w_buf,
		},
		[1] = {
			.addr = ts->client->addr,
			.flags = I2C_M_RD, //后写
			.len = 1,
			.buf = &val,
		}
	};
	ret = i2c_transfer(ts->client->adapter,read_reg,sizeof(read_reg)/sizeof(read_reg[0]));
	if(ret != 2){
		printk("transfer i2c msg is fail.\n");
		return -EAGAIN;
	}
	return val;
	
}

int gt818_i2c_write(int reg,char val)
{
	int ret;
	char reg_l = reg & 0xff;
	char reg_h = (reg >> 8) & 0xff;
	char w_buf[] = {reg_h,reg_l,val}; 
	struct i2c_msg write_reg[] = {
		[0] = {
			.addr = ts->client->addr,
			.flags = 0, 
			.len = 3,
			.buf = w_buf,
		},
	};
	ret = i2c_transfer(ts->client->adapter,write_reg,sizeof(write_reg)/sizeof(write_reg[0]));
	if(ret != 1){
		printk("transfer i2c msg is fail ret = %d.\n",ret);
		return -EAGAIN;
	}
	return 0;

}

int gt818_write_cmd(int reg)
{
	int ret;
	char reg_l = reg & 0xff;
	char reg_h = (reg >> 8) & 0xff;
	char w_buf[] = {reg_h,reg_l}; 
	struct i2c_msg write_reg[] = {
		[0] = {
			.addr = ts->client->addr,
			.flags = 0, 
			.len = 2,
			.buf = w_buf,
		},
	};
	ret = i2c_transfer(ts->client->adapter,write_reg,sizeof(write_reg)/sizeof(write_reg[0]));
	if(ret != 1){
		printk("transfer i2c msg is fail.\n");
		printk("ret = %d\n",ret);
		return -EAGAIN;
	}
	return 0;
}

int gt818_write_end_cmd(void)
{
	int ret,retry=0;
	while(retry++ < 3){
		ret = gt818_write_cmd(0x8000);
		if(ret == 0){
			return 0;
		}else{
			mdelay(10);
			printk("i2c test is retry...\n");
		}
	}
	return -EAGAIN;
}

void ts_interrupt_disable(void)
{
	unsigned long flags;
	spin_lock_irqsave(&ts->ts_spinlock, flags);
	disable_irq_nosync(ts->irq_num);
	spin_unlock_irqrestore(&ts->ts_spinlock, flags);
}

void ts_interrupt_enable(void)
{
	unsigned long flags;
	spin_lock_irqsave(&ts->ts_spinlock, flags);
	enable_irq(ts->irq_num);
	spin_unlock_irqrestore(&ts->ts_spinlock, flags);
}

static irqreturn_t ts_interrupt(int irq, void *dev_id)
{
	ts_interrupt_disable();
	schedule_work(&ts->ts_work);
	return IRQ_HANDLED;
}

static void ts_work_handle(struct work_struct *work)
{
	int ret,i;
	ts->data_base_addr = 0x721;
	ret = gt818_i2c_read(0x712);  //读取工作模式，状态，有多少个触点
	gt818_write_end_cmd(); 

	ts->touch_num = (ret & 0xf);

	for(i=0; i<(ts->touch_num*8); i++){
		ts->data_base_addr += 1;
		ts->data_buf[i] = gt818_i2c_read(ts->data_base_addr);  //读数据
		gt818_write_end_cmd(); 
	}

	//用于内核数据显示
	for(i=0; i<ts->touch_num; i++){
		printk("touch%d:(%d,%d) ",(i+1),((ts->data_buf[2+i*8] << 8)|(ts->data_buf[1+i*8])),\
			((ts->data_buf[4+i*8] << 8)|(ts->data_buf[3+i*8])));
		printk("  touch pressure:(%d)\n",((ts->data_buf[6+i*8] << 8)|(ts->data_buf[5+i*8])));
	}

	//用于事件上报
	for(i=0; i<ts->touch_num; i++){
		input_report_abs(ts->ts_input,ABS_MT_SLOT,i+1);
		input_report_abs(ts->ts_input,ABS_MT_TRACKING_ID,i+1);
		input_report_abs(ts->ts_input,ABS_MT_POSITION_X,((ts->data_buf[2+i*8] << 8)|(ts->data_buf[1+i*8])));
		input_report_abs(ts->ts_input,ABS_MT_POSITION_Y,((ts->data_buf[4+i*8] << 8)|(ts->data_buf[3+i*8])));
		input_report_abs(ts->ts_input,ABS_MT_PRESSURE,((ts->data_buf[6+i*8] << 8)|(ts->data_buf[5+i*8])));
	}
	input_sync(ts->ts_input);

	ts_interrupt_enable();
}

int init_ts_input(void)
{
	int ret;
	//分配空间
	ts->ts_input = input_allocate_device();
	if(ts->ts_input == NULL){
		printk("input alloc memory is fail.\n");
		return -ENOMEM;
	}

	//设置事件类型
	set_bit(EV_ABS,ts->ts_input->evbit);
	set_bit(EV_KEY,ts->ts_input->evbit);
	//设置上报的事件值
	set_bit(BTN_TOUCH,ts->ts_input->keybit);
	input_mt_init_slots(ts->ts_input,MAX_TOUCH_POINT,0);

	//单点事件
	input_set_abs_params(ts->ts_input,ABS_X,0,1024,0,0); 
	input_set_abs_params(ts->ts_input,ABS_Y,0,600,0,0);
	input_set_abs_params(ts->ts_input,ABS_PRESSURE,0,255,0,0);

	//多点事件
	input_set_abs_params(ts->ts_input,ABS_MT_TOUCH_MAJOR,0,255,0,0);
	input_set_abs_params(ts->ts_input,ABS_MT_WIDTH_MAJOR,0,255,0,0);
	input_set_abs_params(ts->ts_input,ABS_MT_POSITION_X,0,1024,0,0);
	input_set_abs_params(ts->ts_input,ABS_MT_POSITION_Y,0,600,0,0);
	input_set_abs_params(ts->ts_input,ABS_MT_TRACKING_ID,0,255,0,0);
	input_set_abs_params(ts->ts_input,ABS_MT_PRESSURE,0,255,0,0);
	
	//输入子系统的注册
	ret = input_register_device(ts->ts_input);
	if(ret){
		printk("register input is fail.\n");
		return -EAGAIN;
	}
	return 0;
}

int gt818_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int ret,i;
	struct device_node *node;
	
	printk("%s:%s:%d\n",__FILE__,__func__,__LINE__);

	ts = kzalloc(sizeof(*ts),GFP_KERNEL);
	if(ts == NULL){
		printk("struct ts is alloc fail.\n");
		return -ENOMEM;
	}
	ts->client = client;
	ts->init_base_addr = 0x6a1;
	
	//1.设置复位管脚，上电一直处于复位状态，置为1清除复位
	ts->GPX0CON= ioremap(0x11000c00,8);
	*(ts->GPX0CON) &= ~(0xf<<12);
	*(ts->GPX0CON) |= (1<<12);    //设置gpx0_3的方向为输出
	*(ts->GPX0CON+1) |= (1<<3); //设置gpx0_3输出的结果为1
	mdelay(20);

	//2.写前缀和后缀
	gt818_write_end_cmd();   //结束上一次的通讯
	
	//3.检测一下模式是否为自动模式
	ret = gt818_i2c_read(0x692);  //查看工作模式
	printk("work mode = 0x%02x \n",ret);
	gt818_write_end_cmd();  
	
	//4.触摸屏寄存器的初始化
	for(i=0; i<106; i++){
		ts->init_base_addr += 1;
		ret = gt818_i2c_write(ts->init_base_addr,ts_cfg_info[i]);
		gt818_write_end_cmd();  
		if(ret < 0){
			printk("init reg is fail.\n");
			return -EAGAIN;
		}
	}
	
	spin_lock_init(&ts->ts_spinlock);        //初始化自旋锁
	INIT_WORK(&ts->ts_work, ts_work_handle);//初始化工作队列
	//分配输入子系统
	init_ts_input();
	
	//5.注册中断
	node = of_find_node_by_path("/ts-gt818");
	ts->irq_num = irq_of_parse_and_map(node,0);
	printk("irq_num = %d\n",ts->irq_num);
	ret = request_irq(ts->irq_num,ts_interrupt,IRQF_TRIGGER_RISING,"ts-interrupt",NULL);
	if(ret){
		printk("request irq is fail.\n");
		return -EAGAIN;
	}
	
	return 0;
}

int gt818_remove(struct i2c_client *client)
{
	printk("%s:%s:%d\n",__FILE__,__func__,__LINE__);
	free_irq(ts->irq_num,NULL);
	kfree(ts);
	return 0;
}

static struct of_device_id gt818_of_match[] = {
	{ .compatible = "ts-gt818",},
	{},
};
MODULE_DEVICE_TABLE(of, gt818_of_match);

static const struct i2c_device_id gt818_i2c_id[] = {
	{ "ts-gt818",0x5d },
	{ }
};
MODULE_DEVICE_TABLE(i2c, gt818_i2c_id);

static struct i2c_driver gt818_dri = {
	.driver = {
		   .name = "ts-gt818",
		   .owner = THIS_MODULE,
		   .of_match_table = gt818_of_match,
	},
	.probe = gt818_probe,
	.remove = gt818_remove,
	.id_table = gt818_i2c_id,
};

static int __init ts_gt818_init(void)
{
	return i2c_add_driver(&gt818_dri);
}

static void __exit ts_gt818_exit(void)
{
	i2c_del_driver(&gt818_dri);
}

module_init(ts_gt818_init);
module_exit(ts_gt818_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("design by dzs(daizs_jt@farsight.com.cn)");
