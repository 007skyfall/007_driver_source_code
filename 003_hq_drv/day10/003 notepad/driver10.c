【1】中断的上下半部
     在中断期间运行，不能执行有可能睡眠的操作，不能同用户空间交换数据，不能调用schedule函数放弃调度。
	 
	 实现中断处理有一个原则，就是尽可能快地处理并返回。冗长计算处理工作应该交给tasklet或任务队列在安全的时间内进行。

【2】中断上下半部机制
     两个半部的理念：
       解决既要中断执行快，又要做的事情多的矛盾。

	 下半部的机制：
		1，软中断
		2，Tasklet
		3，工作队列

	
	1，工作队列：
	定义一个工作队列 
	struct work_struct my_wq; 

	定义一个处理函数 
	void  my_wq_func(struct work_struct *work)

	初始化工作队列并将其与处理函数绑定 
	INIT_WORK(&my_wq, my_wq_func); 

	调度工作队列执行 
	schedule_work(&my_wq); 
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	struct work_struct {
		atomic_long_t data;
		struct list_head entry;  // 内核双链表节点
		work_func_t func;  // 耗时任务的处理函数
	};
	// 耗时任务的处理函数
	typedef void (*work_func_t)(struct work_struct *work);
	
	
	2，tasklet  
	 tasklet 结构体
		struct tasklet_struct
		{
			struct tasklet_struct *next;
			unsigned long state;
			atomic_t count;
			void (*func)(unsigned long);
			unsigned long data;
		};
		
	定义一个处理函数 
	void my_tasklet_func(unsigned  long); 
	
	定义一个tasklet结构my_tasklet，与my_tasklet_func(data)函数相关联 
	DECLARE_TASKLET(my_tasklet, my_tasklet_func, data);
	
	调度tasklet 
	tasklet_schedule(&my_tasklet); 

	区别：
	    工作队列的使用方法和tasklet非常相似 
		tasklet运行于中断上下文 
		工作队列运行于进程上下文 
		tasklet处理函数中不能睡眠，而工作队列处理函数中允许睡眠 

【3】内核中的时间流
     设备驱动程序需要获得时间信息以及定时服务，包括获取高低分辨率的时间。
	 
	 内核中关于时间的一个重要概念jiffies
	 jiffies每隔一个固定时间就会增加1，这个固定间隔由定时器中断来实现
	 每秒钟产生多少个定时器中断，由在<linux/param.h>中定义的宏HZ确定

	 jiffies 单位 是 长整形数据，它代表的是系统开机到现在所经过的时间。
	 
	 程序的延迟执行：
		驱动程序为了让硬件有足够的事件完成一些任务，常常需要将特定的代码延后一段时间来执行
		
	长延迟定义为：
	    延迟时间多于若干个jiffy，实现长延迟可以用查询jiffies的方法：
		time_before(jiffies, new_jiffies);  time_after(new_jiffies, jiffies);

	短延迟定义为：延迟时间接近或短于一个jiffy
		调用udelay和mdelay，忙等待函数，大量消耗CPU时间
		函数udelay使用软件循环来延迟指定数目的微秒数
		函数mdelay使用udelay嵌套来实现更长的毫秒级延迟

【4】使用jiffies 来实现 内核定时器
    定义一个名为my_timer的定时器
	struct timer_list  my_timer; 

	初始化定时器 
	void  init_timer(struct timer_list * timer);
	mytimer.function = my_function;
	mytimer.expires =  jiffies + HZ;

	增加定时器
	void  add_timer(struct timer_list * timer);

	删除定时器 
	int  del_timer(struct timer_list * timer);   
	
	修改定时器
	int mod_timer(struct timer_list *timer, unsigned long expires);
	
	//////////////////////////////////////////////////////////////////////////////////////////////
	struct timer_list {
		struct list_head entry;
		unsigned long expires;  // 期望时间值
		struct tvec_base *base;

		void (*function)(unsigned long);  // 时间到达时，会回调此函数
		unsigned long data;  // 给回调函数传递参数
	};

【5】linux内存分配
	按页(page)分配 _ _get_free_pages () 
		调用者指定所需整页的阶数作为参数之一来请求
		所申请的页将会被加入内核空间
		所分配的物理RAM空间是连续的
	kmalloc
		除了是请求精确字节的内存外，与按页分配相同
	vmalloc
		除了物理内存不一定连续外，与kmalloc同

	
	unsigned long  __get_free_page(int gfp_mask)
	unsigned long  __get_free_pages(int gfp_mask, unsiged long order)
	所分配的内存在物理上是连续的
	“__ “表示在返回页时并不将其清0
	该函数可分配多个页并返回分配内存的首地址，分配的页数为2order，分配的页也不清零。order允许的最大值是10（即1024页）或者 11（即2048页），依赖于具体的硬件平台。
	
	void free_page(unsigned long addr)
	void free_pages(unsigned long addr, unsigned long order)
	必须自己释放内存
	在模块卸载时内核并不清楚其拥有的页

	一页的大小 ， 一般情况写 为 4K
	
	gfp_mask 标志位：
	    GFP_ATOMIC     原子操作标志位
		GFP_KERNEL     空间从内核中申请
		GFP_USER       用户空间，开辟内存
	
	内核管理物理内存方式在底层是通过页来实现的
	内核为了满足任意数量的内存使用请求，创建了由各种不同固定大小的内存块组成的内存对象池
	当kmalloc申请内存空间时，内核就会将一个刚好足够大的空闲内存块分配出来。比如要申请100字节，
	   kmalloc就会返回一个128字节的内存块
	在4k字节大小页面的系统上，分配的最小内存块是32字节，最大128k字节

	////////////////////////////////////////////////////////////////////////////////////////////////////////
	struct cdev *cdev_alloc(void)
	{
		struct cdev *p = kzalloc(sizeof(struct cdev), GFP_KERNEL);
		return p;
	}
    	
	--->
	
	static inline void *kzalloc(size_t size, gfp_t flags)
	{
		return kmalloc(size, flags | __GFP_ZERO);   // 开辟连续的空间（物理内存连续）
	}
	
	--->
	static __always_inline void *kmalloc(size_t size, gfp_t flags)
	
	如果需要连续的大块内存区域，就需要在内核引导时分配
	在内核引导时预留的方式跳过了linux内存管理，直接保留需要的内存区域
	在内核空间对物理内存进行映射后访问

【6】I2C 驱动框架
     1，I2C协议
	    主机 与 从机 使用 两根线连起来的。
		SCL   时钟线
		SDA   数据线
		
		起始信号：SCL 高电平期间，SDA 下降沿触发
		结束信号：SCL 高电平期间，SDA 上升沿触发
		
		// I2C的协议帧
	   S Slave Address 7bits R/W A DATA(1Byte) A P
       起始信号	从机地址（7位）读/写 应答信号  数据（1个字节） 应答信号  停止信号
	   
	   0  ：写标志位
	   1  ：读标志位
	   
     2，三星手册关于I2C章节
	   Multi-master I2C-bus control register – I2CCON     // 控制寄存器
	 Multi-master I2C-bus control/status register – I2CSTAT   // 状态寄存器
	 Multi-master I2C-bus Tx/Rx data shift register – I2CDS  // 数据传输寄存器
	 Multi-master I2C-bus address register – I2CADD   // 地址寄存器
	 
	   三星	i2c控制器，三星厂家自己编写控制器驱动。
	 
	 3，I2C设备 MPU6050
	     SCL5
		 SDA5
		 GYRO_INT    中断管脚
	 
	     MPU6050  陀螺仪， 重力加速度，角速度，温度
		 
		 芯片的地址 0x68
		
		
【7】I2C 总线架构模型
     1，总线
	    struct bus_type i2c_bus_type = {
			.name		= "i2c",   // 总线名称
			.match		= i2c_device_match,  // 匹配函数
			.probe		= i2c_device_probe,   // 探测函数
			.remove		= i2c_device_remove,  // 移除函数
			.shutdown	= i2c_device_shutdown,
			.pm		= &i2c_device_pm_ops,
		};
	 
	 2，设备
	 struct i2c_client {
			unsigned short flags;		/* div., see below		*/
			unsigned short addr;		/* mpu6050 从机的芯片地址，由芯片厂商提供	*/
			char name[I2C_NAME_SIZE];   //　设备的名称
			struct i2c_adapter *adapter;	/* i2c适配器	*/
			struct device dev;		/* 设备的基类	*/
			int irq;			/* 硬件的中断号	*/
			struct list_head detected;  // 设备链表
	  };
			 
	 3，驱动
     struct i2c_driver {
			unsigned int class;
			/* Standard driver model interfaces */
			int (*probe)(struct i2c_client *, const struct i2c_device_id *);  // 探测函数，当设备与驱动匹配成功时，被回调
			int (*remove)(struct i2c_client *);  // 移除函数
			struct device_driver driver;   // 驱动的基类
			const struct i2c_device_id *id_table;  // i2c 的匹配表
		
			int (*detect)(struct i2c_client *, struct i2c_board_info *);
			const unsigned short *address_list;
			struct list_head clients;
		};		
	 
	 4，I2C 适配器
		struct i2c_adapter {
			struct module *owner;
			unsigned int class;		  /* classes to allow probing for */
			const struct i2c_algorithm *algo; /* 访问总线的算法 */
			void *algo_data;
			
			int timeout;			/* 超时时间 */
			int retries;           // 重传次数
			struct device dev;		/* the adapter device */
			int nr;
			char name[48];
			struct list_head userspace_clients;

			struct i2c_bus_recovery_info *bus_recovery_info;
		};
	 
	 5，I2C 消息的传递
	 struct i2c_algorithm {
		int (*master_xfer)(struct i2c_adapter *adap, struct i2c_msg *msgs,
				   int num);  // i2c 此函数指针，由三星控制器驱动实现。
		/* To determine what the adapter supports */
		u32 (*functionality) (struct i2c_adapter *);
	};
		 
	6，i2c 消息 
	 struct i2c_msg {
		__u16 addr;	/* 从机地址			*/
		__u16 flags;   // 标志位
	#define I2C_M_TEN		0x0010	/* this is a ten bit chip address */
	#define I2C_M_RD		0x0001	/* 1 读 ， 0  写 */
	#define I2C_M_STOP		0x8000	/* if I2C_FUNC_PROTOCOL_MANGLING */
	#define I2C_M_NOSTART		0x4000	/* if I2C_FUNC_NOSTART */
	#define I2C_M_REV_DIR_ADDR	0x2000	/* if I2C_FUNC_PROTOCOL_MANGLING */
	#define I2C_M_IGNORE_NAK	0x1000	/* if I2C_FUNC_PROTOCOL_MANGLING */
	#define I2C_M_NO_RD_ACK		0x0800	/* if I2C_FUNC_PROTOCOL_MANGLING */
	#define I2C_M_RECV_LEN		0x0400	/* length will be first received byte */
		__u16 len;		/* 消息的长度				*/
		__u8 *buf;		/* 数据的正文			*/
	};
	 
	 7，i2c驱动中如何传递消息
	 int i2c_transfer(struct i2c_adapter *adap, struct i2c_msg *msgs, int num)
	 功能：实现 主机与从机之前的数据传递
	 参数：adap  适配器
	       msgs  消息
		   num   消息的个数
	 返回值：成功0，出错 -1
	 
	 int i2c_transfer(struct i2c_adapter *adap, struct i2c_msg *msgs, int num)
	{
		int ret;
		
		if (adap->algo->master_xfer) {
		
			ret = __i2c_transfer(adap, msgs, num);  // 负责消息的传递
			return ret;
		} else {
			dev_dbg(&adap->dev, "I2C level transfers not supported\n");
			return -EOPNOTSUPP;
		}
	}
	 
	---->
	
	int __i2c_transfer(struct i2c_adapter *adap, struct i2c_msg *msgs, int num)
	{
		unsigned long orig_jiffies;
		int ret, try;

		/* Retry automatically on arbitration loss */
		orig_jiffies = jiffies;
		for (ret = 0, try = 0; try <= adap->retries; try++) {
			ret = adap->algo->master_xfer(adap, msgs, num);  // 负责传递消息
			...
		}

		return ret;
	}
	 
	---->
    。。。
	
【8】MPU6050 驱动编程
    1，查看原理图 
	     SCL5
		 SDA5
		 GYRO_INT    中断管脚  ， EXINT27   , GPX3_3
	 
	     MPU6050  陀螺仪， 重力加速度，角速度，温度
		 
		 芯片的地址 0x68
	 
	 2，修改设备树，添加 MPU6050设备节点信息
	    i2c@138B0000 {
314         samsung,i2c-sda-delay = <100>;
315         samsung,i2c-max-bus-freq = <20000>;
316         pinctrl-0 = <&i2c5_bus>;
317         pinctrl-names = "default";
318         status = "okay";   // 状态配置， okay 打开， disable 关闭，默认是关闭的
319 
320         mpu6050-3-axis@68 {
321             compatible ="invensense,mpu6050";   // 设备树匹配表
322             reg = <0x68>;   // mpu6050 的从机地址
323             interrupt-parent = <&gpx3>;  // 中断父亲
324             interrupts = <3 2>;    // 基于父亲，描述中断源
325         };
326     };

	 3，编译设备树文件
	    make  dtbs
		
	 4，拷贝设备树文件，到/tftpboot 目录下
	 
	 5，进行mpu6050驱动编程
	
	
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

              应用程序
    ------------------------------
   		  字符设备驱动框架
	------------------------------
	      i2c设备驱动框架
    ------------------------------
	      mpu6050 设备驱动
    ------------------------------
	      三星i2c控制器驱动
	------------------------------
	      mpu6050 的硬件
	
	
	
	
	
