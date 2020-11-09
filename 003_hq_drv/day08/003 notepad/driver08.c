【1】设备驱动模型
     将设备与驱动进行分离。将硬件信息与软件信息进行分离。结构更加清晰，移植更加方便。
	 
	 排序算法：
	     main()
		 {
			int a[N] = {};
			
			for()
			{
				for()
				{
				   a[] ....;
				}
			}
		 
		 }
		 
		 int sort(int a[], int n)
		 {
		     ....
		 }
		 
		 main()
		 {
			int a[N] = {};
			
			sort();
		 
		 }
		
		数据 与 算法的分离。
		 
【2】sysfs 文件系统
    sysfs类似proc文件系统的特殊文件系统
	用于将系统中的设备组织成层次结构，并向用户模式程序提供详细的内核数据结构信息
	
	sysfs的挂载
	mount -t sysfs sysfs /sys
	
	sysfs的信息来源是kobject层次结构
	   读一个sysfs文件，就是动态的从kobject结构提取信息，生成文件
	   kobject层次结构就是linux的设备模型
		 
	struct person{
		char name[N];
		int year;
		int month;
		int day;
	};

    struct student {
		struct person p;
	    int stuno;
	};	
	
	struct teacher{
		struct person p;
	    int teacherno;
	};	
    
	Kobject 对象：
		Kobject是组成设备模型的基本结构,类似于C++中的基类
		它嵌入于更大的对象中--用来描述设备模型的组件，如bus,devices, drivers 都是典型的容器
		这些容器就是通过kobject连接起来了，形成了一个树状结构。这个树状结构就与/sys向对应
		每个在内核中注册的kobject对象都对应于sysfs文件系统中的一个目录
    
	Kset:
	    Kset   kobject的集合。kobject通过kset组织成层次化的结构，kset是具有相同类型的kobject的集合
	 	 
【3】设备驱动模型
    1，总线可以是物理存在的，也可以是虚拟的。内核中对应的结构为struct bus_type。
		struct bus_type {
			const char		*name;   // 总线的名称
			const char		*dev_name;  // 设备的名称
			struct device		*dev_root;  // 设备的指针
			int (*match)(struct device *dev, struct device_driver *drv);  // 设备与驱动匹配函数，当你注册设备或者驱动时，此函数会被回调
			int (*probe)(struct device *dev);  // 探测函数，当设备与驱动匹配成功时，会回调此函数
			int (*remove)(struct device *dev);  // 移除函数
		};
		
	2，设备是连接到某条物理或者虚拟总线上的对象。可能是真正物理对象，也可能是虚拟对象。内核中对应的结构为struct device。
	   struct device {
			struct device		*parent;  // 父亲节点指针
			struct kobject kobj;   // 所有设备的基类
			struct bus_type	*bus;		/* 设备挂载到那条总线上 */
			struct device_driver *driver;	/*设备驱动 */
			void		*platform_data;	/*  平台设备的数据 */
	
			struct device_node	*of_node; /* 设备树节点 */
			dev_t			devt;	/* 设备号 */
			struct list_head	devres_head;  // 循环双链表
			struct class		*class;
			void	(*release)(struct device *dev);  // 释放函数
			....
		};

    3，驱动是用来和设备通信的软件程序。驱动可以从设备中获取数据，也可以把相应数据发给设备进行处理。
	   内核中对应的结构为struct device_driver	
	   
		struct device_driver {
			const char		*name;  // 驱动的名称，可以用来进行和设备按名字匹配
			struct bus_type		*bus;  // 总线的类型
			struct module		*owner;  // THIS_MODULE,
			const struct of_device_id	*of_match_table;  // 设备树匹配表
			int (*probe) (struct device *dev);  // 探测函数，当设备与驱动匹配成功，就回调此函数
			int (*remove) (struct device *dev);  // 移除函数
			struct driver_private *p;
			....
		};	   
		 
【4】platform 平台虚拟总线
     1，总线类型 platform_bus_type
	 
	   struct bus_type platform_bus_type = {
			.name		= "platform",  // 总线的名称
			.dev_groups	= platform_dev_groups,
			.match		= platform_match,  // 匹配函数
			.uevent		= platform_uevent,
			.pm		= &platform_dev_pm_ops,
		};
	 2，平台设备
		struct platform_device {
			const char	*name;  // 设备的名称，用来进行匹配的名称
			int		id;
			bool		id_auto;
			struct device	dev;  // 设备的基类
			u32		num_resources;  // 硬件资源的数量
			struct resource	*resource;  // 硬件资源
			const struct platform_device_id	*id_entry;  // id_table 匹配方式
            ...
		};
		
     3，平台驱动
		struct platform_driver {
			int (*probe)(struct platform_device *);   // 探测函数，一旦设备与驱动匹配成功，就回调此函数
			int (*remove)(struct platform_device *);  // 移除函数
			struct device_driver driver;  // 驱动的基类
			const struct platform_device_id *id_table;  // id_table 匹配表
			....
		};

【5】platform  的 API接口
    int platform_device_register(struct platform_device *pdev)
	功能：注册平台设备
	参数：pdev   设备的地址
	返回值：成功 0， 出错 负数错误码

	void platform_device_unregister(struct platform_device *pdev)
	功能：取消设备的注册
	参数：平台设备的地址
	返回值：无
	
	#define platform_driver_register(drv)   __platform_driver_register(drv, THIS_MODULE)
	功能：注册一个驱动
	参数：drv  平台驱动
	返回值：成功0，出错 负数的错误码

	int __platform_driver_register(struct platform_driver *,
					struct module *);
	
	void platform_driver_unregister(struct platform_driver * pdriver);
	功能：取消一个驱动的注册
	参数：drv  平台驱动
	返回值：成功0，出错 负数的错误码

【6】硬件资源结构体
     struct resource {
		resource_size_t start;  // 起始地址
		resource_size_t end;   //结束地址
		const char *name;
		unsigned long flags;　// 地址的标志位
		struct resource *parent, *sibling, *child;
	};

	#define IORESOURCE_IO		0x00000100	/* PCI/ISA I/O ports */
	#define IORESOURCE_MEM		0x00000200   // 内存
	#define IORESOURCE_REG		0x00000300	/* 寄存器地址 */
	#define IORESOURCE_IRQ		0x00000400   // 中断标志位
	#define IORESOURCE_DMA		0x00000800   // DMA 标志位
	#define IORESOURCE_BUS		0x00001000   // 总线标志位

【7】获取硬件资源信息
     
	/**
	 * platform_get_resource - get a resource for a device
	 * @dev: platform device
	 * @type: resource type
	 * @num: resource index
	 */
	struct resource *platform_get_resource(struct platform_device *dev,
						   unsigned int type, unsigned int num)	
    功能：从平台设备中获取资源
	参数：dev  平台设备的地址
	      type  地址的标志位 
		  num   资源的索引下标
    返回值：资源的指针
	
【8】匹配方式
    1，按照名称进行匹配

	2，按照id_table匹配表，进行匹配	
       多个设备匹配，同一个驱动
    
		struct platform_driver {
			....
			const struct platform_device_id *id_table;  // id_table 匹配表匹配
		};
		-->
	   struct platform_device_id {
			char name[PLATFORM_NAME_SIZE];   // 按名称进行匹配
			kernel_ulong_t driver_data;
		};


      struct platform_device_id  ids[] = {
 45     {.name = "demo0", },
 46     {.name = "demo1", },
 47     {/*Nothing to be done.*/},    // 匹配表的结束标志
 48  };                                                                                                                
 49 
    匹配的优先级：
	    设备树  >  id_table 表匹配  >  按照名称匹配

 【9】匹配的过程
     static int platform_match(struct device *dev, struct device_driver *drv)
	 {
		struct platform_device *pdev = to_platform_device(dev);
		struct platform_driver *pdrv = to_platform_driver(drv);

		/* 设备树匹配 */  
		if (of_driver_match_device(dev, drv))
			return 1;

		/* id table 表进行匹配  */
		if (pdrv->id_table)
			return platform_match_id(pdrv->id_table, pdev) != NULL;

		/* 按照名称进行匹配 */
	 	return (strcmp(pdev->name, drv->name) == 0);
	 }

     --->
	 
	 static const struct platform_device_id *platform_match_id(
			const struct platform_device_id *id,   // 匹配表的结构体指针
			struct platform_device *pdev)   // 设备地址
		{
		   // (id + 0) -> name   取出的整个名称，name = "demo0"
			while (id->name[0]) {
				if (strcmp(pdev->name, id->name) == 0) {
					pdev->id_entry = id;
					return id;
				}
				id++;  // 数组指针，向前移动 (id+1)
			}
			return NULL;
		}
	
【10】对platform驱动进行再次升级
     module_platform_driver(pdriver);   // 替换了加载，卸载函数

     #define module_platform_driver(__platform_driver) \
       	 module_driver(__platform_driver, platform_driver_register, platform_driver_unregister)
		 
	---->

     #define module_driver(__driver, __register, __unregister, ...) \                                                 
1204 static int __init __driver##_init(void) \   /// pdriver_init   
1205 { \
1206     return __register(&(__driver) , ##__VA_ARGS__); \
1207 } \
1208 module_init(__driver##_init); \
1209 static void __exit __driver##_exit(void) \
1210 { \
1211     __unregister(&(__driver) , ##__VA_ARGS__); \
1212 } \
1213 module_exit(__driver##_exit);
	
	##　：　将两个字符串拼接起来
	
【11】字符设备驱动框架 与 platform架构的关系






















	








	 
		 
		 
		 
		 
		 
		 
		 