#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/input.h>

static struct input_dev *fspad_input;
static int __init fspad_input_init(void)
{
	int ret;
	//结构体的分配
	fspad_input = input_allocate_device();
	if(fspad_input == NULL){
		printk("alloc input memory is fail.\n");
		return -ENOMEM;
	}
	//设置事件类型
	set_bit(EV_KEY,fspad_input->evbit);
	
	//设置事件的值
	set_bit(KEY_1,fspad_input->keybit);
	set_bit(KEY_2,fspad_input->keybit);
	set_bit(KEY_3,fspad_input->keybit);
	set_bit(KEY_4,fspad_input->keybit);
	set_bit(KEY_POWER,fspad_input->keybit);

	//结构体的注册
	ret = input_register_device(fspad_input);
	if(ret){
		printk("register input is fail.\n");
		return -EAGAIN;
	}
	
	return 0;
}
static void __exit fspad_input_exit(void)
{
	input_unregister_device(fspad_input);
	input_free_device(fspad_input);
}
module_init(fspad_input_init);
module_exit(fspad_input_exit);
MODULE_LICENSE("GPL");
