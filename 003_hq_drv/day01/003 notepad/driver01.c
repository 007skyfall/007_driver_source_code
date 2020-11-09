【1】模块的加载，卸载
    1、显示指定加载，卸载
	   static int __init demo_init(void)
	  {                                                                                                                 
	      printk("%s,%d\n", __func__, __LINE__);
	  
	      return 0;
	  }
	  
	  static void __exit demo_exit(void)
	  {
	      printk("%s,%d\n", __func__, __LINE__);
	  }
	  
	  module_init(demo_init);
	  module_exit(demo_exit);

	 2、 隐式指定加载，卸载
	    int __init init_module(void)
	  {
	      printk("%s,%d\n", __func__, __LINE__);
	  
	      return 0;
	  }
	  
	  void __exit cleanup_module(void)
	  {
	      printk("%s,%d\n", __func__, __LINE__);
	  }             
	 
【2】模块命令
     1、加载模块
	    sudo insmod demo.ko
		
	 2、卸载模块
	   sudo rmmod demo
	   
	 3、查看打印信息
	   sudo dmesg -c
	   
	 4、查看内核中的模块信息
	   lsmod
	   Module                  Size  Used by
		demo                   12428  0 
       模块名称                 大小  使用者0
	   
	 5、查看内核模块信息
	   cat  /proc/modules
	   
	   demo 		12428    	0 			- Live 		0x00000000 (O)
	   vmwgfx 		102138 		2 			- Live 		0x00000000
       模块名称  	大小  		使用计数  	活着
	   
	 6、查看模块中的描述信息
	  modinfo demo.ko
	  
		filename:       demo.ko
		description:    This is a simple driver test.
		author:         farsight
		license:        GPL
		srcversion:     BCEC0948369108DA0793922
		depends:        
		vermagic:       3.2.0-29-generic-pae SMP mod_unload modversions 686 

【3】模块加载卸载过程
     显示指定加载卸载过程：
     1、加载	   
	    sudo insmod demo.ko   ---> module_init(demo_init) ---> demo_init(void) { .... }
		
	 2、卸载
	    sudo rmmod demo  --->  module_exit(demo_exit);  -->  demo_exit(void) { ... }
		
	 隐式指定加载卸载过程：
	 缺省加载卸载时，名字是固定的
	 1、加载 
	    sudo insmod demo.ko --> init_module();
		
	 2、卸载
	    sudo  rmmod demo -->  cleanup_module();
		
【4】模块传参
    module_param(参数名,参数类型,参数读/写权限)
		
	module_param_array(name,type,num,perm); // 数组名称， 元素类型，元素个数，权限
	num需要加上&符号，否则编译报错。
	
	module_param_string(ids, ids, sizeof(ids), 0);
	module_param_string(传递参数时变量名称, 数组名, 数组大小, 权限);
	                    传参名称（在进行加载模块时写上的参数名称），数组名称（定义时的数组名称），数组大小，权限
	MODULE_PARM_DESC(变量名称, 对变量的描述信息);
						
【5】导出符号表
	Linux 2.6的“/proc/kallsyms”文件对应着内核符号表，它记录了符号以及符号所在的内存地址。

	模块可以使用如下宏导出符号到内核符号表：
	EXPORT_SYMBOL(符号名);
	EXPORT_SYMBOL_GPL(符号名);
	Module.symvers  存储了导出符号的信息

	1、先编译export.c ，生成符号表Module.symvers 拷贝到 used目录，再编译used.c
	2、先加载 export.ko , 再加载 used.ko
	3、先卸载 used, 再卸载 export
	
	cat /proc/kallsyms | grep " o"   // 查看符号表
	
【6】字符设备驱动框架
	驱动注册与初始化
	主设备号和次设备号
	设备名

	设备操作集合struct  file_operations
	对设备的具体操作

	注册字符设备
	关联设备号和设备操作集合

	1、设备号
	    主设备号，表示的是同一类型的设备
		次设备号，同一类型设备的数量
		#define MINORBITS	20
		#define MINORMASK	((1U << MINORBITS) - 1)

		#define MAJOR(dev)	((unsigned int) ((dev) >> MINORBITS))
		#define MINOR(dev)	((unsigned int) ((dev) & MINORMASK))
		#define MKDEV(ma,mi)	(((ma) << MINORBITS) | (mi))  // 生成一个设备号
		
		// 主设备，占高12位
		// 次设备号，占低20位
		ma << 20 | mi
		
	2、字符设备
	   struct cdev {
			struct kobject kobj;  //基类
			struct module *owner;  // 模块的拥有者，一般为THIS_MODULE
			const struct file_operations *ops; // 字符设备操作方法集合
			struct list_head list;  // 内核循环双链表节点
			dev_t dev;  // 设备号
			unsigned int count;  // 次设备的数量
		};
				
	 3、字符设备操作集合
      struct file_operations {
		struct module *owner;	
		ssize_t (*read) (struct file *, char __user *, size_t, loff_t *);
		ssize_t (*write) (struct file *, const char __user *, size_t, loff_t *);
		unsigned int (*poll) (struct file *, struct poll_table_struct *);
		long (*unlocked_ioctl) (struct file *, unsigned int, unsigned long);
		int (*open) (struct inode *, struct file *);
		int (*release) (struct inode *, struct file *);
		......
	  };	 
		
     4、将设备号注册到内核中
      int register_chrdev_region(dev_t from, unsigned count, const char *name)
      功能：......
      参数：from  设备号
            count 次设备的数量
            name  设备号的名称
      返回值：成功0，出错 -1
	  
【7】结构体成员赋值
	 struct node {
		int val;
		int (*func)(void);  // 函数指针，指向的函数，没有形参，返回值为int
	 };

	 // 一种方式
     struct  node  m;
     m.val = 100;	
     m.func = show;	 
	 
	 // 另外一种
	 struct node m1 = {100, show};
	 
	 struct node m2 = {100, , , , , ....,  show};
	 
	 // 另外一种
	 struct node m3 = {.val=100, .func = show};
	 
	 --->
	 struct node m3 = {
			.val=100, 
	        .func = show,   // 最后一个','  可加，可不加
	 };

【8】字符设备驱动框架
	设备驱动可以分为下面三类
	1、字符设备驱动
	2、块设备驱动
	3、网络设备驱动
	
【9】内核的Makefile
   // 向终端输出一个变量的值， $(waring AA)
  1 $(warning KERNELRELEASE = $(KERNELRELEASE))
  2 
  3 ifeq ($(KERNELRELEASE),)
  4 
  5 #内核的源码路径, ?= 条件赋值, uname -r 得到内核版本号, shell 在makefile 中，执行一条shell命令
  6 KERNELDIR ?= /lib/modules/$(shell uname -r)/build  
  7 
  8 # := 立即赋值, 得到当前的绝对路径
  9 PWD := $(shell pwd)
 10 
 11 
 12 # -C 切换工作路径, $(MAKE) =  make
 13 modules:
 14     $(MAKE) -C $(KERNELDIR) M=$(PWD) modules
 15 
 16 clean:
 17     rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions Module* modules*
 18 // 对伪目标的申明
 19 .PHONY: modules clean
 20 
 21 else
 22       # 生成模块                                                                                                  
 23        obj-m := demo.o 
 24     
 25 endif
 26 

 /// makefile 的调用过程
 
    make  --->  调用 内核的Makefile  ， 实现KERERLESE 赋值 
	                       -->  调用 当前绝对路径下的Makefile ，  demo.c  ---> demo.o
						   -->  调用 当前绝对路径下的Makefile ,   demo.o  ---> demo.ko
						   
			     离开内核的makefile			   

		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
	   
	   
	   
	 