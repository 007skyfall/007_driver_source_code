=======================================================================================================================
									Linux设备驱动中的阻塞与非阻塞I/O
									
1、字符设备IO模型
	同步阻塞io模型：
	同步非阻塞io模型：
	异步阻塞io模型：	
	/*
		异步非阻塞io模型：
	*/				
	异步信号驱动io（异步通知）：

	[1]同步阻塞io概念：系统调用执行时（read）,其条件不满足（buf中无对应的数据） ，系统调用（read）
					   处于一个阻塞等待的状态（休眠），直到条件满足时（buf中有数据），
					   才能继续去执行；
			
		等待队列： 进程 + 队列的这样一种组合
		等待队列头:将不满足条件的系统调用所对应的进程挂载在对应的等待队列中；
		
		底层驱动实现对应的同步阻塞io的相关参数及API（初始化，睡眠，唤醒）:
		相关对象以及函数API:
			typedef struct __wait_queue_head wait_queue_head_t;
			#include <linux/wait.h>
			(1)void init_waitqueue_head（wait_queue_head_t *q）
				功能：初始化等待队列头
				参数：q：等待队列头的指针对象
				返回值：无
				
			(2)int wait_event_interruptible(wait_queue_head_t q,int condition)//可被信号打断
				功能：添加对应的等待队列
				参数：q：等待队列头
				     condition：条件
				返回值： The function will return -ERESTARTSYS if it was interrupted by a signal and 0 if @condition evaluated to true.
						当该函数被信号意外打断的情况下应该返回return -ERESTARTSYS
						如果condition为真，直接返回0
		
			(3)void wake_up(wait_queue_head_t *q)
				功能：唤醒对应的等待队列
				参数：q：等待队列头的指针对象
				返回值：无
				
				
				gcc read.c   -fno-stack-protector -o read
		【*】程序编写：同步阻塞IO
				（1）模块三要素
				（2）实现字符框架相关
				（3）在入口函数中实现等待队列头的初始化
				（4）read条件不满足，直接去睡眠等待，将其加入到对应的等待队列中wait_event_interruptible
				（5）当天剑满足时，将等待队列唤醒
					if(条件不满足){
							if（wait_event_interrupt( wq ， flags)）
								return -ERESTARTSYS;
					}
					copy_to_user
	
	
	
	
	[2]同步非阻塞io概念：系统调用执行时（read）,其条件不满足（buf中无对应的数据） ，直接报错返回
	
		相关参数或标志：在app上open时，加上O_NONBLOCK非阻塞标志即可
		【*】程序编写：同步非阻塞IO
			struct  file->f_flags 
			
	[3]异步阻塞io概念：类似于同步阻塞io，只不过是其阻塞的是一组文件描述符；
				
				底层实现:select--->底层poll
					unsigned int (*poll) (struct file *, struct poll_table_struct *);
						底层poll函数的实现--->poll_wait-->将对应的请求->poll_table
							(1)实现struct file_operations->poll
							(2)在poll函数中，调用poll_wait函数，将对应的请求放到poll表
							(3)poll函数返回具体的掩码【表征其条件是否满足ps:是否可读可写】
								POLLIN:用户空间可以去读取相关数据
								POLLOUT:用户空间可以去输出、写入相关数据
						
					static inline void poll_wait(struct file * filp, wait_queue_head_t * wait_address, poll_table *p)
					
					
				app实现：[select/poll]
					 #include <poll.h>

					int poll(struct pollfd *fds, nfds_t nfds, int timeout);
					功能：等待一组文件描述符中准备好文件描述符执行输入输出
					参数：fds:struct pollfd 结构体指针，在fds参数中指定要监视的文件描述符集
									  nfds：监控的文件描述符的个数
							timeout：超时时间设置[0:立即返回不阻塞  >0:等待指定毫秒数 其他:永远等待]
					返回值：成功：>0       超时：=0       出错：-errno

					struct pollfd {
					   int   fd;         /*待监测的文件描述符*/
					   short events;     /*待监测的事件 */
					   short revents;    /*返回监测到的事件 */
				   };
					测试程序编写思路：
						（1）打开
						（2）填充待检测文件描述符集合 struct pollfd的结构体数组
						（3）poll函数检测，检测各个文件描述符事件的返回值
				   
				
		   【*】程序编写：非阻塞读(轮询app)[多路复用IO]
		   
		   
		   
			 
	[4]异步非阻塞io概念：
		读写操作立即返回，用户程序的读写请求将放入一个请求队列中由设备在后台异步完成，
		当设备完成了本次读写操作时，将通过信号或者回调函数的方式通知用户程序。【块、网络】

		
		异步通知：
		
			 底层实现：	int (*fasync) (int, struct file *, int);--->调用fasync_helper:添加请求
				底层实现思路：
					（1）完成struct file_operations->fasync
					（2）在fasync函数中调用fasync_helper将应用层的请求添加
					（3）当底层发现有满足该请求的情况出现时，会自动发送信号  kill_fasync-->eg:SIGIO
					（4）触发应用层与该信号相关的的处理函数去执行
			 
			 
			 应用层实现：	
			【*】程序编写：异步通知(fasync)	
				（1）打开
				（2）设置该进程与文件描述符的联系 fcntl
				（3）设置异步通知标志 fcntl：先获取标志 之后或上  FASYNC  之后再设置即可
				（4）绑定信号已及其处理函数 signal--->SIGIO
			
				#include <unistd.h>
				#include <fcntl.h>

					int fcntl(int fd, int cmd, ... /* arg */ );
					cmd:
						F_SETOWN:绑定进程
						F_GETFL:获取文件打开标志
						F_SETFL:设备文件标志
						

=======================================================================================================================
											Linux虚拟总线platform框架
													
1、宏观：总线、设备、驱动
	为什么引入platform？主要用在哪呢？如何联系设备与驱动？
	    引入总线设备驱动：
		
	[1]总线 
		[1.1]总线结构体	:struct bus_type
		[1.2]相关API:bus_register/bus_unregister		
		[1.3]注册成功现象:/sys/bus/xxxx
		
	[2]设备
		[1.1]设备结构体	struct device
		[1.2]相关API:device_register/device_unregister	
		[1.3]注册成功现象:/sys/bus/devices/xxxx
	
	[3]驱动	
		[1.1]驱动结构体struct device_driver
		[1.2]相关API:driver_register/driver_unregister
		[1.3]注册成功现象:/sys/bus/drivers/xxxx

2、虚拟总线platform框架
	platform的基于:总线、设备、驱动
	虚拟总线
	platform原理？【绘图】
	[1]platform总线
		[1.1]总线结构体	：struct bus_type platform_bus_type{...};		
	
	[2]platform设备 + 资源
		[1.1]设备结构体
			struct platform_device {
				const char	*name;//设备名称
				int		id;//序号  -1
				struct device	dev;
				u32		num_resources;//记录当前设备的硬件资源 mem,irq的个数
				struct resource	*resource;//记录设备的的资源
			};
			
			struct resource {
				unsigned long flags;//资源的类型  ：IORESOURCE_MEM ,IORESOURCE_IRQ
				resource_size_t start;
				resource_size_t end;
			};
			
			struct device {
				void	(*release)(struct device *dev);
			};
			
			当platform_device或者platform_driver	注册	时，自动执行 probe	函数；
			当platform_device或者platform_driver	注销	时，自动执行 remove	函数；
			当platfrom_device注销时，要额外调用 release 函数；
			
		[1.2]相关API
			#include <linux/pltaform_device.h>
			int platform_device_register(struct platform_device *pdev)
			功能：注册platform_device
			参数：pdev：struct platform_device的指针对象
			返回值：成功：0  失败：errno
			
			void platform_device_unregister(struct platform_device *pdev)
			功能：注销platform_device
			参数：pdev：struct platform_device的指针对象
			返回值：无
			
			流程（platform_device端一般用于提供设备信息）：
				（1）模块三要素
				（2）定义并填充对应的platform_device结构体
				（3）注册、注销即可	
		[1.3]注册成功现象
		
		
	[3]platform驱动	
		[1.1]驱动结构体		
			struct platform_driver {
				int (*probe)(struct platform_device *);//platform_device与platform_driver匹配成功，自动执行
				int (*remove)(struct platform_device *);//platform_device与platform_driver任意一方卸载掉时，自动执行
				struct device_driver driver;//驱动对象
				const struct platform_device_id *id_table;//platform_driver所支持的设备名称表
			};

			struct platform_device_id {
				char name[PLATFORM_NAME_SIZE];//名称
			};
			struct device_driver {
				const char		*name;//名称
				struct module		*owner;//默认填充THIS_MODULE
				const struct of_device_id	*of_match_table;//设备树匹配
			};
			
			struct of_device_id
			{
				char	compatible[128];//设备树匹配信息
			};

					platform_device与platform_driver匹配成功的规则：
									platform_device                                platform_driver
									device_tree                                     of_match_table
									.name											.id_table				
									.name 											drivre.name
									
		[1.2]相关API
			（1）int platform_driver_register（struct platform_driver *drv）
				功能：注册platform_driver
				参数：drv：struct platform_driver的指针对象
				返回值：成功：0  失败：errno
						#define platform_driver_register(drv) \
							__platform_driver_register(drv, THIS_MODULE)

			（2）void platform_driver_unregister(struct platform_driver *drv)
				功能：注销platform_driver
				参数：drv：struct platform_driver的指针对象
				返回值：无	
			
		流程：
			（1）模块三要素
			（2）platfrom_drievr定义 + 填充
			（3）注册、注销
				
		[1.3]注册成功现象	
		
	[3]对于platform_device而言主要是提供对应的设备资源（struct resource）
		驱动端获取设备的硬件信息
		struct resource *platform_get_resource(struct platform_device *dev,unsigned int type, unsigned int num)
		功能：获取设备资源
		参数：dev：设备对象
		      type：设备资源的类型  :struct resouce->flags IORESOURCE_MEM/IRQ
			  num：设备资源序号
		返回值：成功：struct resource指针对象  失败：NULL
	
	[4]platform框架驱动编写：框架
	
	总结：
		
		
		
		
		
		
		
		
	
	
	