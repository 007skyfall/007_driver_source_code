#include <linux/module.h>
#include <linux/ioport.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/skbuff.h>
#include <linux/spinlock.h>
#include <linux/crc32.h>
#include <linux/mii.h>
#include <linux/ethtool.h>
#include <linux/dm9000.h>
#include <linux/delay.h>
#include <linux/irq.h>
#include <linux/slab.h>

#include <asm/delay.h>
#include <asm/irq.h>
#include <asm/io.h>


//1.定义结构体
static struct net_device *virt_net;
int count=0;
int virt_net_open(struct net_device *dev)
{
	printk("%s:%s:%d\n",__FILE__,__func__,__LINE__);
	return 0;
}
int virt_net_stop(struct net_device *dev)
{
	printk("%s:%s:%d\n",__FILE__,__func__,__LINE__);
	return 0;

}
netdev_tx_t virt_net_start_xmit(struct sk_buff *skb,struct net_device *net_dev)
{
	count++;
	net_dev->stats.tx_packets = count; 
	net_dev->stats.tx_bytes = skb->len;
	printk("%s:%s:%d\n",__FILE__,__func__,__LINE__);

	return NETDEV_TX_OK;
}
int virt_net_do_ioctl(struct net_device *dev,struct ifreq *ifr, int cmd)
{
	printk("%s:%s:%d\n",__FILE__,__func__,__LINE__);
	return 0;

}

static struct net_device_ops netdev_ops = {
	.ndo_open = virt_net_open,
	.ndo_stop = virt_net_stop,
	.ndo_start_xmit = virt_net_start_xmit,
	.ndo_do_ioctl = virt_net_do_ioctl,
};

static int __init virt_net_init(void)
{
	int ret;
	//2.分配结构体
	virt_net = alloc_netdev(0,"virt0",ether_setup);
	if(virt_net == NULL){
		printk("alloc memory fail\n");
		return -ENOMEM;
	}

	//3.设置
	virt_net->netdev_ops = &netdev_ops;
	//4.注册
	ret = register_netdev(virt_net);
	if(ret){
		printk("register net fail\n");
		return -EAGAIN;
	}
	return 0;
}
static void __exit virt_net_exit(void)
{
	//注销
	unregister_netdev(virt_net);
}
module_init(virt_net_init);
module_exit(virt_net_exit);
MODULE_LICENSE("GPL");


