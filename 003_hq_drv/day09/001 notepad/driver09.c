【1】设备树
     设备树并没有推翻设备驱动模型，只是对设备驱动模型进行升级，改进。
	 将 设备的信息的描述，由代码形式，转换成了 文本的形式。更加的方便，清晰易懂。上电之后，由内核对文件进行解析。
	 
	 1，设备的存储路径
	    linux-3.14-fs4412/arch/arm/boot/dts$ vi exynos4412-fs4412.dts
	 
【2】设备树驱动编程
     1，添加一个设备树节点  vi exynos4412-fs4412.dts
	    xxx{
			compatible = "fs4412,xxx";   
			a = "hello";
			b = <0x12345678>;
		};
		// 节点路径:  /xxx
		xxx ：节点的名称
		compatible ：设备树匹配表，"厂商,节点名称" ， 此字符串必须与驱动中字符串保持严格一致。
		a ：健值对名称
		"hello"  ：健值对的值
		
	 2，编译设备树，在内核顶层目录下
	    make dtbs
		
	 3，将编译好的设备树文件，拷贝/tftpboot 
	    cp  arch/arm/boot/dts/exynos4412-fs4412.dtb    /tftpboot
		
	 4，设备树的驱动编程
	 
【3】设备树驱动编程的API 函数接口
     struct device_node *of_find_node_by_path(const char *path)
     功能：通过节点的路径，查找到节点
	 参数：path  节点的路径
	 返回值：成功  设备节点的地址
	         失败   NULL
			 
	设备节点：
	struct device_node {
		const char *name;   // 节点的名称
		const char *type;
		const char *full_name;
		struct	property *properties;  // 属性健值对
		...
	};

    属性健值对：
	struct property {
		char	*name;   // 键的名称
		int	length;
		void	*value;  // 键的值
		struct property *next;  // 单链表
		unsigned long _flags;
		unsigned int unique_id;
	};

    struct property *of_find_property(const struct device_node *np,
				  const char *name,int *lenp)
    功能：已知设备节点的地址，根据键的名称，求出键的值
	参数：np    设备节点的地址
	      name   键的名称
		  lenp   键的长度
	返回值：成功 属性健值对的指针
	        出错 NULL
			
	int of_property_read_string(struct device_node *np, const char *propname,
				const char **out_string)
	功能：已知字符串属性健值对的名称，得到字符串的值
	参数：np   设备节点的地址
	      propname  属性健值对的名称
		  out_string  填充字符串的值
	返回值：成功0，出错 -1
	
	
【4】设备树匹配
     struct platform_driver {
		struct device_driver driver;
		const struct platform_device_id *id_table;
		...
	};
			
	--->
	
	struct device_driver {
		...
		const struct of_device_id	*of_match_table;
	}

    --->
	
	struct of_device_id
	{
		char	name[32];
		char	type[32];
		char	compatible[128];   // 设备树匹配表
		const void *data;
	};

【5】led灯设备树驱动编程
	1，修改设备，添加led灯节点
	     led5@114001e0 {
			compatible = "farsight,led5";   // 设备树匹配表
			reg = <0x114001e0 0x4 0x114001e4 0x4>;   // 灯的资源
		};      

	2，编译设备树文件，在内核顶层目录下
	   make dtbs
	   
	3，将设备树文件拷贝到/tftpboot 目录下
      	cp  *.dtb    /tftpboot
		
	4，led灯的驱动编程
	
【6】中断
     中断机制提供了硬件和软件之间异步传递信息的方式
	 硬件设备在发生某个事件时通过中断通知软件进行处理
	 中断实现了硬件设备按需获得处理器关注的机制，与查询方式相比可以大大节省CPU时间
	

	struct irqaction {
		irq_handler_t		handler;   // 中断处理函数
		void			*dev_id;
		void __percpu		*percpu_dev_id;
		struct irqaction	*next;
		irq_handler_t		thread_fn;
		struct task_struct	*thread;
		unsigned int		irq;  // 中断号
		unsigned int		flags;  // 标志位
		unsigned long		thread_flags;
		unsigned long		thread_mask;
		const char		*name;
		struct proc_dir_entry	*dir;
	};

    中断的申请函数：
	static inline int request_irq(unsigned int irq, irq_handler_t handler, unsigned long flags, const char *name, void *dev)
    功能：申请注册一个中断
	参数：irq  中断号码
	      handler  中断处理函数
		  flags   中断标志位
		  name    中断的名称   // cat  /proc/interrupts
		  dev     共享中断的id ， 一般为NULL
	返回值：成功 0.出错 负数的错误码

	中断处理函数：
	typedef irqreturn_t (*irq_handler_t)(int, void *);  
	// typedef irqreturn_t (*)(int, void *)  irq_handler_t   比较好理解 ，但是编译器不支持
	
	函数指针的返回值：
		/**
		 * enum irqreturn
		 * @IRQ_NONE		interrupt was not from this device，这个设备上没有产生中断
		 * @IRQ_HANDLED		interrupt was handled by this device  中断已经被此设备所处理了
		 * @IRQ_WAKE_THREAD	handler requests to wake the handler thread  将处理函数交给一个新的线程来处理
		 */
		enum irqreturn {
			IRQ_NONE		= (0 << 0),
			IRQ_HANDLED		= (1 << 0),
			IRQ_WAKE_THREAD		= (1 << 1),
		};

		typedef enum irqreturn irqreturn_t;

	中断标志位：
     #define IRQF_TRIGGER_RISING	0x00000001   // 上升沿触发
	 #define IRQF_TRIGGER_FALLING	0x00000002   // 下降沿触发
	 #define IRQF_TRIGGER_HIGH	    0x00000004   // 高电平触发
	 #define IRQF_TRIGGER_LOW	    0x00000008	 // 低电平触发
	 #define IRQF_DISABLED		    0x00000020    // 关中断
	 #define IRQF_SHARED		    0x00000080    // 共享中断标志位
	
	中断的释放：
	void free_irq(unsigned int irq, void *dev_id)
	功能：。。。
	参数：irq  中断号码
	      dev_id  共享中断的id， 一般为NULL
		  
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		  
【7】按键驱动
     1，查看原理图
        K2   UART_RING	 
		
	 2，查看核心板原理图	
		K2   UART_RING    XEINT9  GPX1_1
		
	 3，查看三星的芯片手册
	    EXINT9    SPI PORT NO     ID
		          25              57
	    在arm裸机开发中，中断号为57

        上了操作系统之后，linux不认识57号，认 SPI PORT NO 25		
	
	 4，添加按钮中断的设备节点
            fs4412-key {
				compatible = "fs4412,key";     // 设备树匹配表
				interrupt-parent = <&gpx1>;     // & 引用父亲节点 gpx1
				interrupts = <1 2>, <2 2>;     // key2 , key3 中断描述
		    };
	 
		<1 2> ： 1 代表的是引用父亲节点中的第几个元素（下标从0开始）
		         2 按键中断的触发方式  （下降沿触发方式）
		
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // gic 中断控制器节点描述
 
 13 - compatible : should be one of:   // 设备树匹配表
 14     "arm,gic-400"
 15     "arm,cortex-a15-gic"
 16     "arm,cortex-a9-gic"
 17     "arm,cortex-a7-gic"
 18     "arm,arm11mp-gic"
 19 - interrupt-controller : Identifies the node as an interrupt controller
 20 - #interrupt-cells : Specifies the number of cells needed to encode an
 21   interrupt source.  The type shall be a <u32> and the value shall be 3.
       // 使用3个元素描述一个中断
 22 
 23   The 1st cell is the interrupt type; 0 for SPI interrupts, 1 for PPI
 24   interrupts.   // 中断的类型
 25 
 26   The 2nd cell contains the interrupt number for the interrupt type.
 27   SPI interrupts are in the range [0-987].  PPI interrupts are in the
 28   range [0-15].  // 中断号
 29 
 30   The 3rd cell is the flags, encoded as follows:  // 中断触发方式
 31     bits[3:0] trigger type and level flags.
 32         1 = low-to-high edge triggered  // 上升沿
 33         2 = high-to-low edge triggered  // 下降沿
 34         4 = active high level-sensitive    // 高电平                                                                                                                                                                                                              
 35         8 = active low level-sensitive   // 低电平
 36     bits[15:8] PPI interrupt cpu mask.  Each bit corresponds to each of
 37     the 8 possible cpus attached to the GIC.  A bit set to '1' indicated
 38     the interrupt is wired to that CPU.  Only valid for PPI interrupts.
 39 
 40 - reg : Specifies base physical address(s) and size of the GIC registers. The
 41   first region is the GIC distributor register base and size. The 2nd region is
 42   the GIC cpu interface register base and size. 
 43 
 44 Optional
 45 - interrupts    : Interrupt source of the parent interrupt controller on
 46   secondary GICs, or VGIC maintenance interrupt on primary GIC (see
 47   below).
 48 
 49 - cpu-offset    : per-cpu offset within the distributor and cpu interface
 50   regions, used when the GIC doesn't have banked registers. The offset is
 51   cpu-offset * cpu-nr.
 52 
 53 Example:
 54 
 55     intc: interrupt-controller@fff11000 {
 56         compatible = "arm,cortex-a9-gic";
 57         #interrupt-cells = <3>;
 58         #address-cells = <1>;
 59         interrupt-controller;
 60         reg = <0xfff11000 0x1000>,
 61               <0xfff10100 0x100>;
 62     };
     
	 

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
         gpx1: gpx1 {
572             gpio-controller;
573             #gpio-cells = <2>;
574 
575             interrupt-controller;
576             interrupt-parent = <&gic>;   // 父亲节点为gic中断控制器
577             interrupts = <0 24 0>, <0 25 0>, <0 26 0>, <0 27 0>,
578                      <0 28 0>, <0 29 0>, <0 30 0>, <0 31 0>;
579             #interrupt-cells = <2>;   // 为儿子节点制定规则，有2个元素描述中断
580         };
		
		<0 25 0> ： 0 中断类型 SPI
		            25  中断号， 用SPI PORT NO
					0   触发方式
		
        三星的中断描述规律：
		    KEY2  GPX1_1   
		   fs4412-key {
				compatible = "fs4412,key";     // 设备树匹配表
				interrupt-parent = <&gpx1>;     // & 引用父亲节点 gpx1
				interrupts = <1 2>, <2 2>;     // key2 , key3 中断描述
		    };
		
		  中断 GPX3_4  
		
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

       5，编译设备树文件
	       make dtbs
		   
	   6，拷贝设备树文件 /tftpboot, 可以参考之前的笔记
		   ...  
		
	   7，编写按键驱动
		
		
【8】获取中断号
     int platform_get_irq(struct platform_device *dev, unsigned int num)	
	 功能：。。。
	 参数：dev  平台设备指针
	       num  资源的下标索引
	 返回值：中断号码
	 
	 




















	 
		
		