#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/usb/input.h>
#include <linux/hid.h>
#include <linux/usb.h>

#if 0
#define MOUSE_VENDOR 0x093a
#define MOUSE_PRODUCT 0x2521
#endif
#define MOUSE_VENDOR 0x1d57
#define MOUSE_PRODUCT 0xfa60

int len;
struct urb *urb;
static char *usb_buf;
static dma_addr_t usb_buf_phys;
static struct input_dev *mouse_dev;
static void usb_mouse_irq(struct urb *urb)
{
	#if 0
	int i,cnt=0;
	++cnt;
	printk("data cnt %d:",cnt);
	for(i=0; i<6; i++){
		printk("%02x ",usb_buf[i]);
	}
	printk("\n");
	#endif
	if(usb_buf[0] == 1){
		input_report_key(mouse_dev,KEY_L,1);
		input_sync(mouse_dev);
	}else if(usb_buf[0] == 2){
		input_report_key(mouse_dev,KEY_S,1);
		input_sync(mouse_dev);
	}else if(usb_buf[0] == 4){
		input_report_key(mouse_dev,KEY_ENTER,1);
		input_sync(mouse_dev);
	}
	input_report_key(mouse_dev,KEY_L,0);
	input_report_key(mouse_dev,KEY_S,0);
	input_report_key(mouse_dev,KEY_ENTER,0);
	input_sync(mouse_dev);
	usb_submit_urb(urb,GFP_KERNEL);
}

int mouse_probe(struct usb_interface *intf,
		  const struct usb_device_id *id)
{
	struct usb_device *dev = interface_to_usbdev(intf);
	struct usb_host_interface * interface;
	struct usb_endpoint_descriptor *endpoint;
	unsigned int pipe;
	int ret;

	interface = intf->cur_altsetting;
	endpoint = &interface->endpoint[0].desc;

	//初始化输入子系统
	//1.1分配接口体
	mouse_dev = input_allocate_device();
	//1.2设置结构体
		//1.2.1设置事件类型
		set_bit(EV_KEY,mouse_dev->evbit);
		//1.2.2设置上报的事件
		set_bit(KEY_L,mouse_dev->keybit);
		set_bit(KEY_S,mouse_dev->keybit);
		set_bit(KEY_ENTER,mouse_dev->keybit);
	//1.3注册结构体
	ret = input_register_device(mouse_dev);
	if(ret){
		printk("register input is fail.\n");
		return -EAGAIN;
	}
	
	//源
	pipe = usb_rcvintpipe(dev,endpoint->bEndpointAddress);
	//长度
	len = endpoint->wMaxPacketSize;
	//目的
	usb_buf = usb_alloc_coherent(dev, len, GFP_ATOMIC, &usb_buf_phys);

	//1.分配urb
	urb = usb_alloc_urb(0, GFP_KERNEL);
	
	//2.urb的初始化
	usb_fill_int_urb(urb,dev,pipe,usb_buf,len,usb_mouse_irq,NULL,endpoint->bInterval);
	urb->transfer_dma = usb_buf_phys;
	urb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;
	
	//3.urb的数据提交
	usb_submit_urb(urb,GFP_KERNEL);
	
	return 0;
}

void mouse_distconnect(struct usb_interface *intf)
{
	printk("%s:%s:%d\n",__FILE__,__func__,__LINE__);
}


static struct usb_device_id usb_mouse_id_table [] = {
//	{USB_DEVICE(MOUSE_VENDOR, MOUSE_PRODUCT), },
	{ USB_INTERFACE_INFO(USB_INTERFACE_CLASS_HID, USB_INTERFACE_SUBCLASS_BOOT,
		USB_INTERFACE_PROTOCOL_MOUSE) },
	{ }
};

MODULE_DEVICE_TABLE(usb, usb_mouse_id_table);


static struct usb_driver mouse_dri = {
	.name = "2.4G usb mouse",
	.probe = mouse_probe,
	.disconnect = mouse_distconnect,
	.id_table = usb_mouse_id_table,
	
};

static int __init mouse_init(void)
{
	usb_register(&mouse_dri);
	return 0;
}
static void __exit mouse_exit(void)
{
	usb_deregister(&mouse_dri);
}

module_init(mouse_init);
module_exit(mouse_exit);
MODULE_LICENSE("GPL");
