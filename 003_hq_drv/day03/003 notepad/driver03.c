=======================================================================================================================
										linux设备驱动IO操作
【1】字符设备的高级操作
		字符设备框架程序编写流程：
		1、模块三要素
		2、完成字符设备框架相关的代码cdev
				手动创建设备节点：mknod /dev/xxx     c/b		主设备号  次设备号
		                                 char/block
			Makefile的编写
				
				make -C	/lib/modules/3.5.0-23-generic/build(ubuntu) M=$(shell pwd) modules/clean

				  -C :该模块要插入的内核的Makefile的路径
				  M=:待编译的模块的路径
	/*
	(0)字符设备驱动另一种方式
	(1)读写	
	(2)控制（32bits被分为四部分）
	*/
【2】自动创建设备节点[/proc/sys/kernel/hotplug]--->mdev/udev
		#include <linux/device.h>
		1、struct class *class_create(struct module *owner, const char *name)  /sys/class/name
			功能：创建对应的class
			参数：owner：指代对应的模块，一般用 THIS_MODULE
				  name：对应的class的名称--->/sys/class/demo_class
			返回值：成功： struct class对应的指针  失败：错误指针或NULL
			
			IS_ERR(void *ptr)：用来识别检测对应的错误指针   错误：非0   正确：0
			PTR_ERR(void *ptr):将错误指针直接转成对应的错误码
			
			#define class_create(owner, name)		\
			({						\
				static struct lock_class_key __key;	\
				__class_create(owner, name, &__key);	\
			})
		2、void class_destroy(struct class *cls)
			功能：销毁对应的class
			参数：cls：对应的struct class的指针对象
			返回值：无
			
		3、struct device *device_create(struct class *class, struct device *parent,dev_t devt, void *drvdata, const char *fmt, ...)
			功能：创建对应的device
			参数：class：设备所归属的类
				 parent：父设备  如果没有 则写NULL
				  devt:该设备的设备号
				  drvdata：私有数据  无的话直接写NULL
				  fmt:该设备节点的命名格式  /dev/xxx  
				       eg:%s%d
				  ...:根据fmt的具体形式决定 
				      “demo” , i
			返回值：成功：struct device 对应的指针  失败：错误指针或NULL
		
		4、void device_destroy(struct class *class, dev_t devt)
			功能：销毁对应的device
			参数：class：设备所归属的类
				  devt:该设备的设备号
			返回值：无
		
=======================================================================================================================	
										Linux设备驱动内存分配
【1】内存
内存分配函数都依赖于内核中一个构件：内存管理 
linux下对内存管理总体上可以分为两大类：（1）物理内存管理 （2）虚拟内存管理
【2】物理内存
	1、物理内存中内存相关结构体
			内存节点node
				1.1、引入此概念的原因:
						内存管理模型：
							uma:一致性内存管理模型
							numa:非一致性内存管理模型
				/*数据结构struct pglist_data*/
				
			内存区域zone：
				概念：内存区域是内存节点中的一个概念  
				分区：主要分为高端内存区  低端内存区（dma,normal...）
				/*
				  结构体：struct  zone
				  区域的类型：enum zone_type
				*/
				  
			内存页page：
				物理内存管理的最小单元：4k   
	
	2、物理内存中管理机制：	
		2.1、页面级内存管理（页面分配器）：-->page
					(1)页面分配器函数的核心函数：
						__alloc_pages分配连续的若干页、alloc_page分配一页				->高端内存区   -->__free_page
						__get_free_pages分配连续的若干页，get_free_page分配一页		->低端内存区   -->free_page
						get_zeroed_page：分配内存页，并完成分配空间的请0操作
						
					分配区间取决于gfp_mask标志:	GFP_KERNEL,GFP_ATOMIC
					
		2.2、给予页面管理之上的slab管理：小内存
			slab思想：现将内存按页或者若干页分配出来，之后将其分配成若干相等的小的内存区；
			slab分配器限制：分配低端内存
			典型函数：
				static inline void *kmalloc(size_t s, gfp_t gfp)//ps :kzalloc
				功能：分配相关内存区  最大只能分配 128k
				参数：s：分配渠道与大小
					  gfp:分配区间的标志  GFP_KERNEL
				返回值 成功：内存区首地址   失败：NULL
				
				分配的虚拟地址空间连续，并且该虚拟空间对应的物理地址也是连续的；
			
				static inline void kfree(void *p)
				功能：释放相关内存区
				参数：p：分配内存的首地址
				返回值：无
		
			
【3】虚拟内存管理	
	1、vmalloc / vfree
	void *vmalloc(unsigned long size)
	功能：分配相关内存区
	参数：size：大小
	返回值：成功：内存区的地址  失败：NULL
	
	(1)分配的地址空间较大
	(2)分配的虚拟地址空间连续，并且该虚拟空间对应的物理地址 并不一定 是连续的；
	
	void vfree(const void *addr)
	功能：释放相关内存区
	参数：addr：待释放内存区的首地址
	返回值：无
	
	2、linux内核虚拟地址空间构成
	/*
	   4g------------------------------------
							4k
		 -----------------------------------
			       固定内存映射区 4m-4k
	     -----------------------------------
		           高端内存映射区 4m
		 ------------------------------------
					NULL 8K(保护)
		 ------------------------------------vmalloc  end
				 vmalloc内存区120m-8m-8k
		 ------------------------------------vmalloc  start
					vmalloc offset 8m
		 ------------------------------------
					物理内存映射区896M
	   3g------------------------------------  物理内存 3g 偏移   4k + 3g
						用户空间
	   0g------------------------------------
	*/
	
	
	
【4】与硬件操作相关
		1、各个地址的概念
			（1）物理地址：就是硬件芯片手册中能够查找到的实际的地址；
			（2）逻辑地址：就是程序代码经过反汇编之后，能够直观看到的地址；
			（3）虚拟地址：程序员能够操作的地址；
			
				因为linux内核经过内存管理单元，将地址进行了相关的映射处理，故程序员如果说想
			操作某一硬件的话，那么务必现将该硬件所涉及的相关物理地址--->虚拟地址后才可以进行
			相关逻辑处理（配置相关寄存器）；
			
		2、相关函数API:
		（1）ioremap:实现物理地址到虚拟地址的转换
			static inline void __iomem *ioremap(phys_addr_t offset, unsigned long size)
			功能：实现物理地址到虚拟地址的映射
			参数：offset：物理地址
			      size：映射的空间大小
			返回值：成功：映射成功后的虚拟地址   失败：NULL
		
		（2）iounmap:解除地址映射
			static inline void iounmap(void __iomem *addr)
			功能：解除对应的地址映射关系
			参数：addr：虚拟地址
			返回值：无
			
		（3）对映射过来的相关虚拟地址进行读写操作
			readl
				static inline u32 readl(const volatile void __iomem *addr)
				功能：读取寄存器中的相关数据
				参数：addr：虚拟地址
				返回值：该寄存器中的配置数据  
				
			writel
				void writel (unsigned int value, unsigned int ptr)
				功能：向寄存器中写入待配置的数据
				参数：value：待配置的数据
					  ptr：虚拟地址
				返回值：无
				/*
					#define writel(b,addr) __raw_writel(__cpu_to_le32(b),addr)
				*/
		
			
【5】程序编写：led灯驱动
		流程：	
			1、分析硬件原理图
				想点亮led5  ---->  gpf3_5   ---> 亮灯：+   灭灯：-
			
			2、分析该硬件所需配置的寄存器	
				物理地址：
					con:0x114001e0	[20-23]   1(output)
					dat:0x114001e4  [5]		  1亮   0灭
					
			3、搭建模块框架 + cdev 
					流程：	
						3.1、模块三要素
						3.2、完成字符设备相关的操作
						补充：
							static inline int register_chrdev(unsigned int major, const char *name,const struct file_operations *fops)
							功能：实现字符设备框架的搭建
							参数：major：主设备号   major = 0 自动申请
								  name：设备名称
								  fops：操作方法集合
							返回值：如果major=0 则返回申请得到的主设备号   反之返回0
							
							static inline void unregister_chrdev(unsigned int major, const char *name)
							功能：销毁字符设备框架
							参数：major：主设备号  
								  name：设备名称
							返回值：无
`				4、硬件逻辑操作
							地址映射
							硬件配置
							逻辑操作
				5、撤销模块
			
			
			对于内核模块的编写，编译及加载步骤：
				（1）内核模块编写
					1.1 模块三要素
					1.2 在入口函数（及相关操作方法集合）中完成对应的字符设备框架搭建 
						+ 相关硬件资源的申请 ，映射 ， 配置 + 硬件逻辑操作
					1.3 在出口函数中完成入口函数的反操作
				（2）编译
					注意事项：
						2.1  编译模块的内核源码必须是已经被编译完成的
						2.2  在编译模块Makefile的文件中需要更改的内容主要有两个：
							（1）内核源码路径
							（2）待编译文件的名称	
				（3）加载
					3.1 首先将编译好的 xxx.ko 文件拷贝至网络文件系统 “/source/rootfs”
					3.2 启动开发板 ，开发板会自动挂载nfs,将/source/rootfs中的各种文件展现出来
					3.3 执行模块相关的操作 eg:insmod rmmod ....
					3.4 如果说有应用程序要调用底层驱动的函数，那么在pc端应该编译好其应用测试文件，
						  并将其编译好的可执行文件拷贝至 “/source/rootfs”， 在执行即可
			
=======================================================================================================================
									Linux设备驱动中的并发与竟态
【1】并发竟态
1、概念来源？
	概念：linux操作系统允许多进程 多cpu并发执行 ， 多个并发源对同一资源 同时 进行访问 会造成竟态；
	竟态：并发处理单元对 同一资源访问  ，造成该资源数据紊乱的一种状态；
	
	为了确保共享资源的访问安全的措施：
		互斥：强调排他性
		同步：强调顺序
	
	补充概念：	
		临界区：访问共享资源的程序（代码段）
		临界资源：共享资源，被并发执行单元 同时访问的资源
		并发源：多进程， 多cpu...

2、竟态的产生的情况：
	为了防止产生竟态，应该怎么办？
	
	linux内核中产生竟态的几种场景：
		单cpu:
			进程和进程间抢占；
			进程和中断间的抢占；
		多cpu：对称多处理器
			smp
		
	
3、解决方式：
	单cpu:--->中断屏蔽概念及使用场景：
							在临界区访问共享资源之前，将该cpu的中断屏蔽掉，访问完
							共享资源之后在开启中断；
						why:因为进程调度，和中断都依赖于中断机制；
					函数API：
					
					local_irq_disable();	/*中断屏蔽*/
					...
					critical section		/*临界区处理*/
					...
					local_irq_enable();		/*中断开启*/
	
					应用注意事项：使用时间应该尽量短，在此期间不能调用引起调度或者睡眠的函数；
	
	
	--------->自旋锁概念及使用场景：	
						目的：保护共享资源
						核心思想：在系统中维护一个全局变量“v”  v=1表征上锁了  v = 0解锁了
							
							上锁v=1	->	访问共享资源	->	解锁 v=0 让给其他的并发源去获取资源并执行
						
						上锁过程：“read-test-set” 自旋锁是一个忙等锁
						数据类型：
							typedef struct spinlock {
								union {
									struct raw_spinlock rlock;
								};
							} spinlock_t;
							
						函数API(初始化，上锁，解锁):
						#include <linux/spinlock.h>
						
						(1)void  spin_lock_init(spinlock_t *lock)
							功能：初始化自旋锁
							参数：lock：自旋锁的指针对象
							返回值：无
						(2)static inline void spin_lock(spinlock_t *lock)
							功能：自旋锁上锁
							参数：lock：自旋锁的指针对象
							返回值：无
						(3)static inline void spin_unlock(spinlock_t *lock)
							功能：自旋锁解锁
							参数：lock：自旋锁的指针对象
							返回值：无
						
						程序编写：使用自旋锁实现设备只能被一个进程打开
						程序编写流程：
						应用注意事项：上锁期间时间尽量短，不能睡眠，忙等锁			
						
	--------->信号量：类似于自旋锁，在资源不能获取到时，直接去睡眠
					应用注意事项：只能运行域进程上下文；
					结构体：
						#include <linux/semaphore.h>
						struct semaphore {
							raw_spinlock_t		lock;
							unsigned int		count;//表征该资源可以被count个资源进行访问
						}；
						
					函数(初始化，获取，释放)：
						（1）static inline void sema_init(struct semaphore *sem, int val)
							功能：初始化信号量
							参数：sem：信号量的指针对象
								  val:填充sem->count
							返回值：无
						（2）int down_trylock(struct semaphore *sem)
							功能：获取信号量
							参数：sem：信号量的指针对象
							返回值：0：成功  1：失败
						（3）static noinline void up(struct semaphore *sem);
							功能：释放信号量
							参数：sem：信号量的指针对象
							返回值：无
						
						
					程序编写：使用信号量实现设备只能被一个进程打开
					程序编写流程：
						struct semaphore sem;
						（1）在入口函数中初始化信号量
								sema_init(&sem,1);
						（2）在底层open中尝试获取信号量
								if(down_trylock(&sem)){
									return -EBUSY;
								}
						（3）在底层release中释放掉占用的信号量
								up(&sem);
				
					PS:互斥锁mutex:就是信号量的val被设置为1的特殊情况而已
					结构体：struct mutex 

					函数API(初始化，上锁，解锁):
					void mutex_init（struct mutex *lock）
					void  mutex_lock(struct mutex *lock)
					void  __sched mutex_unlock(struct mutex *lock)
					int __sched mutex_trylock(struct mutex *lock)
					 Returns 1 if the mutex has been acquired successfully, and 0 on contention.
				    程序编写：使用互斥体实现设备只能被一个进程打开
					程序编写流程：
						
						
	------>原子变量：
					定义：将多步操作在cpu上执行变为一步；
					函数API(初始化，获取，释放)：
					
						结构体：typedef struct {
									int counter;
								} atomic_t;
								
						初始化：
							ATOMIC_INIT(i)	
							初始化原子变量的值
						
						原子变量的获取并进行测试，验证其是否真正的获取到该变量：
							#define atomic_dec_and_test(v)	(atomic_sub_return(1, v) == 0)
							
							counter = 1
																1
							static inline int atomic_sub_return(int i, atomic_t *v)
							{
								int val
								val = v->counter;//val = 0
								v->counter = val -= i;  //v->counter = val = 1

								return val;
							}
							
						原子变量的释放：
							static inline void atomic_inc(atomic_t *v)
							
							
					
					程序编写：使用原子变量实现设备只能被一个进程打开	
					程序编写流程：
						（1）定义或初始化对应的原子变量
						（2）在执行open时，获取到对应的原子变量
						（3）在release时，将对应的原子变量释放
				
作业1：		   
	总结各种互斥并发机制的使用场景及使用流程？								  










			