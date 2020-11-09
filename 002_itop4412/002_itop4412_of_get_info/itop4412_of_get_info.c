#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_gpio.h>

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("SKYFALL");
MODULE_DESCRIPTION("itop4412_of_get_info");

#define DRIVER_NAME "seedling"

static int leds_probe(struct platform_device * pdev)
{
    struct device_node *node = pdev->dev.of_node;
    printk("node name is %s!\n",node->name);
    printk("node fullname is %s!\n",node->full_name);
    
    struct property *comprop = NULL; 
    comprop = of_find_property(node,"compatible",NULL);
    printk("comprop name is %s!\n",comprop->name);
    printk("comprop value is %s!\n",comprop->value);

    comprop = of_find_property(node,"status",NULL);
    printk("comprop name is %s!\n",comprop->name);
    printk("comprop value is %s!\n",comprop->value);
    
    printk(KERN_ALERT "probe init\n");
    
    return 0;
}

static int leds_remove(struct platform_device * pdev)
{
    printk(KERN_ALERT "Goodbye, curel world, this is remove\n");
    
    return 0;
}

static const struct of_device_id of_leds_dt_match[] = {
    {.compatible = DRIVER_NAME},
    {},
};

MODULE_DEVICE_TABLE(of,of_leds_dt_match);

static struct platform_driver leds_driver = {
    .probe  = leds_probe,
    .remove = leds_remove,
    .driver = {
        .name = DRIVER_NAME,
        .owner = THIS_MODULE,
        .of_match_table = of_leds_dt_match,
        },
};

static int __init leds_init(void)
{
    printk(KERN_ALERT "leds_init!\n");
    printk("%s,%d\n",__func__,__LINE__);

    platform_driver_register(&leds_driver);

    return 0;
}

static void __exit leds_exit(void)
{
    printk(KERN_ALERT "leds_exit!\n");
    printk("%s,%d\n",__func__,__LINE__);

    platform_driver_unregister(&leds_driver);

    return ;
}

module_init(leds_init);
module_exit(leds_exit);