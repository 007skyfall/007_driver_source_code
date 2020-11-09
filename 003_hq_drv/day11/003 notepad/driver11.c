=======================================================================================================================
											Linux设备驱动中断子系统
1、中断
(1)中断的概念
(2)linux中断硬件层面的剖析
	PIC:program interrupt controller
				      --------		    --------
	外设一----------->|	P	 |	INT	    |   C	|
	外设二----------->|	I    |----------|	P	|
	外设三----------->|	C    |  软件    |	U	|
	...	 硬件中断号   --------		    --------
	
/*
		1.了解各个结构体之间的关系
		2.掌握中断框架程序的编写
*/		
(3)linux中断软件层次的剖析
	
	PIC的作用：中断号映射 ，中断屏蔽...
	结构体：
		struct irq_chip//描述可编程中断控制器
		struct irq_desc {//描述中断线
			struct irq_data		irq_data;	//保存软件中断号irq与chip信息
			struct irqaction	*action;	//与具体设备的中断处理的抽象
			struct module		*owner;
     	} ____cacheline_internodealigned_in_smp;

	  struct irqaction {
			irq_handler_t handler; 	// 指向中断服务程序 
			unsigned long flags; 	// 中断标志 
			//unsigned long mask; 	// 中断掩码 
			const char *name; 	// I/O设备名  /proc/interrupts
			void *dev_id; 	    // 用于共享中断时识别设备
			struct irqaction *next; // 用于共享中断时指向下一个描述符
			int irq; 		// IRQ线
	  };

	MAX:系统中的所有的中断的数量
	irq_desc[MAX]
	
	外设1 --->irq=10  ->   
	  
	函数API:
		static inline int __must_check request_irq(unsigned int irq, irq_handler_t handler, unsigned long flags,const char *name, void *dev)
		功能：申请中断号（完成了中断号 + 中断处理函数 + 触发电平等的绑定）
		参数：irq:中断
		      handler ：中断处理函数
			  flags：中断触发标志
							#define IRQF_TRIGGER_RISING	0x00000001
							#define IRQF_TRIGGER_FALLING	0x00000002
							#define IRQF_TRIGGER_HIGH	0x00000004
							#define IRQF_TRIGGER_LOW	0x00000008
			  name：中断名称
			  dev：一般用于区分共享中断  可以写为NULL
		返回值：成功 ： 0  出错 ：-errno
		
		void free_irq(unsigned int irq, void *dev_id)
		功能：释放中断号
		参数：irq:中断号
			  dev_id：一般用于区分共享中断  可以写为NULL
		返回值：无
		
		irqreturn_t (*irq_handler_t)(int, void *)//中断处理函数原型 
		enum irqreturn {
			IRQ_NONE		= (0 << 0),
			IRQ_HANDLED		= (1 << 0),
			IRQ_WAKE_THREAD	= (1 << 1),
		};
		IRQ_NONE:中断例程发现正在处理一个不是自己的设备出发的中断；
		IRQ_HANDLED：中断处理例程成功处理了自己设备的中断；
		IRQ_WAKE_THREAD：中断例程被用来唤醒一个等待在他的irq上的进程使用。

(4)设备树文档讲解
		/linux/arch/arm/boot/dts
		
		文件类型分析：  xxxx.dts xxxx.dtsi  xxxx.dtb
                        xxxx.c	 xxxx.h     a.out	
						
						struct dog{
							int age;
							char name[25];
							int weight;
							.....
						};
						
						dog{
							compatible = “生产厂商信息,exynos4412";
							键                  值        对
							reg = <>;
							interrupts = <>;
						};//节点
						struct device_node
				内核中参考文档：linux-3.14/Documentation/devicetree
				
				/ {//‘/’表征设备树的起始 ：根节点
					model = "Insignal Origen evaluation board based on Exynos4412";
					compatible = "insignal,origen4412", "samsung,exynos4412";

					memory（节点名） {//节点
						reg = <0x40000000 0x40000000>;//属性  ：键值对
							mem{
								
							};
					};

					chosen {
						bootargs ="console=ttySAC2,115200";
					};                                                                                                                                                                 

					ccc:firmware@0203F000 {
						compatible = "samsung,secure-firmware";
						reg = <0x0203F000 0x1000>;
					};
					
					
					kkk{
						
						bbb = &ccc;
						
					};
					
					
				设备树中对于键值对的语法：
					（1）字符串  //bootargs ="console=ttySAC2,115200";
					（2）32bits无符号整形数组   //reg = <0x114001e0 0x4>,<0x114001e4 0x4>;
					（3）16进制数组//mac-address = [00 0a 2d a6 55 a2];
					（4）混合型   //mix = "string",[00 0a 2d a6 ];
					（5）字符串哈希表：//compatible = "insignal,origen4412", "samsung,exynos4412"; 
		
				节点名语法：
					name         @addr
				字母/数字/-/     
					
					led-2@0x114001e0{
						
					};
					
					 gpx0: gpx0 {
						gpio-controller;
						#gpio-cells = <2>;

						interrupt-controller;//表征中断控制器，如果说有某一个中断是来自于该组，则该组可被引用
						interrupt-parent = <&gic>;
								        0           1       2         3
						interrupts = <0 16 0>, <0 17 0>, <0 18 0>, <0 19 0>,                                                                                                       
								     <0 20 0>, <0 21 0>, <0 22 0>, <0 23 0>;
                                         4         5        6          7									 
						#interrupt-cells = <2>;
					};  
					
					
					key2->gpx0_5-->gpx0
					
					key2{
						interrupt-parent = <&gpx0>;
						interrupts = < 5      2>;
									 组号  触发电平
					};
					
					1：上升沿
					2：下降沿
					4：高电平
					8：低电平
					
				
		设备树API：	
		(1)struct device_node *of_find_node_by_path(const char *path)
			功能：根据路径获取设备节点
			参数：path：该设备节点的路径 
			返回值：成功：struct device_node的指针对象  失败:NULL
		
		(2)unsigned int irq_of_parse_and_map(struct device_node *dev, int index)
			功能：根据设备节点对象及序号获取对应的中断号
			参数：dev：获取的到的设备节点
			     index：序号
			返回值：成功：软件中断号    失败:0 
		
		
		
	   *程序编写：按键中断
		
		编写流程：
			（1）原理图分析：
				gpx1_1   下降沿触发  外部中断9 
			（2）将按键相关的信息填写至设备树中
				arch/arm/boot/dts/xxx.dts
				key2{
					interrupt-parent = <&gpx1>;
					interrupts = <1 2> ,<2 2>;
				};
				
				路径：/key2
			（3）编译设备树【在内核顶层  make dtbs 】，并将生成的 xxx.dtb文件拷贝至tftpboot目录
			（4）程序编写
				【1】模块三要素
				【2】在入口函数：
						获取设备信息：struct device_node->irq------------->中断处理函数
						                                      request_irq
					出口：
						释放irq
								
-----------------------------中断底半部----------------------------------
为什么会引入底半部？
为了解决在中断时 既能快速响应其他中断的请求  又能执行处理大量的任务之间的矛盾

		中断顶半部：主要是进行中断登记，中断标志的清除；
		中断底半部：主要执行耗时的中断处理
	
	1.tasklet:中断上下文
		  结构体：
			struct tasklet_struct
			{
				void (*func)(unsigned long);//底半部处理函数
				unsigned long data;//给底半部传参
			};
			
		  函数：
				static inline void tasklet_schedule(struct tasklet_struct *t)
				功能：调度底半部处理函数
				参数：t：struct tasklet_struct指针对象
				返回值：无
		 
		 流程：
			完成tasklet_struct 的定义 + 填充 
			在中断处理函数中调度底半部
			
		 
	2.workqueue ：进程上下文，允许睡眠
		 结构体：
			struct work_struct {
				work_func_t func;//底半部处理函数
			};
			typedef void (*work_func_t)(struct work_struct *work);
			
			
		 函数API:#include <linux/workqueue.h>
		 （1）初始化
			INIT_WORK(struct work_struct *work,work_func_t func)
			#define INIT_WORK(_work, _func)						\
				do {								\
					__INIT_WORK((_work), (_func), 0);			\
				} while (0)
					
		 （2）调度
			static inline bool schedule_work(struct work_struct *work)
			功能：调度工作队列的底半部处理函数
			参数：work：struct work_struct指针对象
			返回值：成功 ：true  失败：false
			
		(3)流程：	
			【1】定义
			【2】初始化
			【3】调度
	
	
	
	定时器：
	（1）结构体
		#include <linux/timer.h>
		struct timer_list
			struct timer_list {
				unsigned long expires;//定时时间  现在时间jiffies + 需要定时的时间段  HZ = 1s
				void (*function)(unsigned long);//定时器到时后的执行语句
				unsigned long data;//给function传递的参数
			};
			
			void init_timer（struct timer_list *timer）
			功能：初始化定时器
			参数：timer：struct timer_list指针对象
			返回值：无	

			void add_timer(struct timer_list *timer)
			功能：添加定时器
			参数：timer：struct timer_list指针对象
			返回值：无
			
			int del_timer(struct timer_list *timer)
			功能：删除定时器
			参数：timer：struct timer_list指针对象
			返回值：an inactive timer returns 0, del_timer() of an active timer returns 1.
			
			int mod_timer(struct timer_list *timer, unsigned long expires)
			功能：定时器的定时时间修改
			参数：timer：struct timer_list指针对象
			      expires：定时时间
			返回值：The function returns whether it has modified a pending timer or not.
         			(ie. mod_timer() of an inactive timer returns 0, mod_timer() of an active timer returns 1.
			
			
			
			#include <linux/time.h>
			void do_gettimeofday(struct timeval *tv)
			功能：获取当前时间点
			参数：tv：struct timeval指针对象
			返回值：无
			struct timeval {
				__kernel_time_t		tv_sec;		/* seconds */
				__kernel_suseconds_t	tv_usec;/* microseconds */
			};
			
			流程：	
				（1）模块三要素
				（2）在init中获取当前系统时间
				      定时器相关的操作：
						[1]定义 + 成员填充
						[2]初始化
						[3]添加
						
				（3）在定时器处理函数中，读取当前时间，并跟之前读取的时间相比较
				（4）exit删除定时器
				
			
			
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
