#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/slab.h>
#include <asm/io.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/of_gpio.h>
#include <linux/miscdevice.h>

MODULE_LICENSE("GPL");

#if 0
struct resource *res0;
struct resource *res1;

void __iomem * gpl2con;
void __iomem * gpl2dat;

#endif

int gpio_pin[2] = {-1};

static int demo_probe(struct platform_device * pdevice) // 探测函数，一旦设备与驱动匹配成功，就回调此函数
{
	unsigned int dat = 0;
	struct device_node *node = pdevice->dev.of_node;
    int ret;
    
    printk("led init\n");
    
    gpio_pin[0] = of_get_named_gpio(node, "gpios1", 0);
    gpio_pin[1] = of_get_named_gpio(node, "gpios2", 0);
    
    ret = gpio_request(gpio_pin[0], "led2");
    if(ret != 0)
    {
        printk("gpio_pin[0] request %d failed.", gpio_pin[0]);
        return ret;
    }
	ret = gpio_request(gpio_pin[1], "led3");
    if (gpio_pin[1] < 0)
        printk("gpio_pin[1] is not available \n");
    if(ret != 0)
    {
        printk("gpio_pin[1] request %d failed.", gpio_pin[1]);
        return ret;
    }
    printk("gpio_pin[0] is %d\n",gpio_pin[0]);
    printk("gpio_pin[1] is %d\n",gpio_pin[1]);
    
    
    gpio_free(gpio_pin[0]);
    gpio_free(gpio_pin[1]);
    
    gpio_direction_output(gpio_pin[0],0);
    gpio_set_value(gpio_pin[0], 1);
    
    gpio_direction_output(gpio_pin[1],0);
    gpio_set_value(gpio_pin[1], 1);
	printk("probe ok!\n");
#if 0
	res0 = platform_get_resource(pdevice, IORESOURCE_MEM, 0);
	if(res0 == NULL)
	{
		printk("Failed to platform_get_resource.\n");
		return -1;
	}
	printk("res0:%#x , %#x\n", (unsigned int)res0->start, (unsigned int)res0->end);

	res1 = platform_get_resource(pdevice, IORESOURCE_MEM, 1);
	if(res1 == NULL)
	{
		printk("Failed to platform_get_resource.\n");
		return -1;
	}
	printk("res1:%#x , %#x\n", (unsigned int)res1->start, (unsigned int)res1->end);

	gpl2con = ioremap(res0->start, 4);
	if(gpl2con == NULL)
	{
		printk("Failed to ioremap.\n");
		return -1;
	}

	gpl2dat = ioremap(res1->start, 4);
	if(gpl2dat == NULL)
	{
		printk("Failed to ioremap.\n");
		return -1;	
	}
#endif

#if 0
	// 设置输出模式
	dat = readl(gpl2con);
	dat = (dat & (~(0xf << 0)))|(0x1 << 0);
	writel(dat, gpl2con);

	dat = readl(gpl2dat);
	dat = dat|(0x1 << 0);
	writel(dat, gpl2dat);
#endif

	return 0;
}

static int demo_remove(struct platform_device * pdevice)  // 移除函数
{
	unsigned int dat = 0;
	printk("%s,%d\n", __func__, __LINE__);
	gpio_set_value(gpio_pin[0], 0);
    gpio_set_value(gpio_pin[1], 0);
#if 0
	dat = readl(gpl2dat);
	dat = dat&(~(0x1 << 0));
	writel(dat, gpl2dat);

	iounmap(gpl2con);
	iounmap(gpl2dat);

#endif
	return 0;
}

struct of_device_id  idts[] = {
	{.compatible="leds_test"},
	{/*Nothing to be done*/},
};

struct platform_driver pdriver = {
	.probe = demo_probe,
	.remove = demo_remove,
	.driver = {
		.name = "itop4412_dts_led",
		.of_match_table = idts,
	},
};

module_platform_driver(pdriver);
