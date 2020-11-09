【1】模拟有名管道通信
    1， 2个应用程序，分别打开一个设备文件，一个负责读消息，一个负责写消息.
	    char buf[128] = {};
	
	2，驱动中利用循环双链表，建立一个队列，队列结点可以定义如下：
	    struct mymsg {
		  char buf[128];
		  struct list_head list;
		}
    
	3,应用程序调用 write  ---> demo_write() --> 将消息入队
	
	4,另外一个应用程序调用 read  ---> demo_read() --> 将消息出队

	5，分配队列的节点函数
	static __always_inline void *kmalloc(size_t size, gfp_t flags)
    功能：...
	参数：size  大小
	      flags  GFP_KERNEL  ， 在内核空间分配，可以被中断

    #include <linux/list.h>   // 双链表头文件
	
【2】流水灯
    1，应用程序
	   while(1)
	   {
	       ioctl(fd, LED_ON, &NUM);
		   sleep(1);
		   ioctl(fd, LED_OFF, &NUM);
		   ...
	   }
	2, 驱动中实现 ioctl 函数
	
	 long demo_ioctl(struct file * filp, unsigned int cmd, unsigned long arg)
	 {
	     int num = ....;
		 
	     switch(cmd)
		 {
		       case LED_ON:
			   fs4412_led_on(num);
			   break;
			   case LED_OFF:
			   fs4412_led_off(num);
			   break;
			   ....
		 }
	 }
	
【3】int (*mmap) (struct file *, struct vm_area_struct *);	
     功能：用户将设备内存区映射到进程的地址空间，直接操作该物理内存提高效率。
	 
	 驱动中提供该方法用于支持用户mmap操作

    1，应用程序
	   #include <sys/mman.h>

       void *mmap(void *addr, size_t length, int prot, int flags,
                  int fd, off_t offset);
	   功能：将物理地址映射为虚拟地址
	   参数：addr  虚拟起始地址，一般为NULL，由系统自己分配
	         length  映射长度
			 prot      PROT_EXEC  Pages may be executed.
					   PROT_READ  Pages may be read.
					   PROT_WRITE Pages may be written.
					   PROT_NONE  Pages may not be accessed.
			 flags   MAP_SHARED  多个进程共享
			         MAP_PRIVATE  私有
			 fd      文件描述符
			 offset  偏移量
	   返回值：成功 映射后的地址，出错  MAP_FAILED

       int munmap(void *addr, size_t length);
       功能：解除映射关系
	   参数：addr  映射后的地址
	         length  长度
	
	2，驱动层面，实现mmap 函数，只需要重新建立页表映射即可
	int (*mmap) (struct file * filp, struct vm_area_struct * vma);
	功能：filp  ...
	      vma   ...
	
	int remap_pfn_range(struct vm_area_struct *vma, unsigned long virt_addr, 
	                    unsigned long pfn, unsigned long size, pgprot_t prot); 
    只需要在mmap中实现，此函数即可
	返回值：成功 0，出错 负数的错误码

	/**
	 * remap_pfn_range - remap kernel memory to userspace
	 * @vma: user vma to map to  用户要映射的虚拟地址区域
	 * @addr: target user address to start at  虚拟地址起始空间
	 * @pfn: physical address of kernel memory  内核空间对应的物理地址，页编号
	 * @size: size of map area  映射大小
	 * @prot: page protection flags for this mapping  页保护标志位
	 *
	 *  Note: this is only safe if the mm semaphore is held when called.
	 */
	
	
	3，虚拟地址结构体
	struct vm_area_struct {
		unsigned long vm_start;		/* 虚拟地址，起始地址. */
		unsigned long vm_end;		/* 虚拟地址，结束地址. */

		pgprot_t vm_page_prot;		/* 虚拟内存的访问权限. */
		unsigned long vm_flags;		/* Flags, see mm.h. */

		/* Function pointers to deal with this struct. */
		const struct vm_operations_struct *vm_ops;

		/* Information about our backing store: */
		unsigned long vm_pgoff;		/* Offset (within vm_file) in PAGE_SIZE
						   units, *not* PAGE_CACHE_SIZE */
		struct file * vm_file;		/* File we map to (can be NULL). */
		void * vm_private_data;		/* was vm_pte (shared mem) */
	};
		
【4】并发与竞态
  并发（concurrency）指的是多个执行单元同时、并行被执行，而并发的执行单元对共享资源
  （硬件资源和软件上的全局变量、静态变量等）的访问则很容易导致竞态（race conditions）
		
  竞态产生的情况：
   1-- 对称多处理器（SMP）的多个CPU

   2-- 单CPU内进程与抢占它的进程

   3-- 中断（硬中断、软中断、Tasklet、底半部）与进程之间
       中断可以打断正在执行的进程，如果中断处理程序访问进程正在访问的资源，则竞态也会发生。
	   
   互斥机制
	1，中断屏蔽
	2，原子操作
	3，自旋锁
	4，信号量

【5】中断屏蔽
	local_irq_disable() /* 屏蔽中断 */
	. . .
	critical section /* 临界区， 不能有耗时操作*/
	. . .
	local_irq_enable()  /* 开中断*/
	
	local_irq_disable()和local_irq_enable()都只能禁止和使能本CPU内的中断，
	因此，并不能解决SMP多CPU引发的竞态。
	
【6】原子操作
     对原子变量的值的操作计算，是原子的，不可在分割的。
	 
	实现方法参见include/asm/atomic.h
	
	整型原子操作
	设置原子变量的值
	void atomic_set(atomic_t *v, int i); /* 设置原子变量的值为i */
	atomic_t v = ATOMIC_INIT(0); /* 定义原子变量v并初始化为0 */
	
	获取原子变量的值
	atomic_read(atomic_t *v);  /* 返回原子变量的值*/
	
	原子变量加/减
	void atomic_add(int i, atomic_t *v); /* 原子变量增加i */
	void atomic_sub(int i, atomic_t *v); /* 原子变量减少i */
	
	原子变量自增/自减
	void atomic_inc(atomic_t *v);    /* 原子变量增加1 */
	void atomic_dec(atomic_t *v);    /* 原子变量减少1 */

    操作并测试
	int atomic_inc_and_test(atomic_t *v);
	int atomic_dec_and_test(atomic_t *v);
	int atomic_sub_and_test(int i, atomic_t *v);
	上述操作对原子变量执行自增、自减和减操作后（注意没有加）测试其是否为0，为0返回true，否则返回false。
	操作并返回
	int atomic_add_return(int i, atomic_t *v);
	int atomic_sub_return(int i, atomic_t *v);
	int atomic_inc_return(atomic_t *v);
	int atomic_dec_return(atomic_t *v);
	上述操作对原子变量进行加/减和自增/自减操作，并返回新的值
	
【7】自旋锁
     自旋锁是一个忙等待锁，锁期间不能有引起睡眠的函数存在。要求自旋锁保护的临界区的执行代码
	 时间越快越好。
	 
	 自旋锁可能导致系统死锁
     自旋锁锁定期间不能调用可能引起进程调度的函数


	定义自旋锁
	spinlock_t  lock;
	
	初始化自旋锁
	spin_lock_init(&lock)
	
	获得自旋锁
	spin_lock(&lock)
	如果能够立即获得锁，它就马上返回，否则，它将自旋在那里
	
	spin_trylock(&lock)
	该宏尝试获得自旋锁lock，如果能立即获得锁，它获得锁并返回真，否则立即返回假
	
	释放自旋锁
	spin_unlock(&lock)
		
【8】信号量
	信号量是内核中用来保护临界资源的一种
	互斥机制

	相关头文件 <linux/semaphore.h>

	定义信号量
	struct semaphore sem;

	初始化信号量
	void  sema_init(struct semaphore *sem, int val);
	
	信号量的获取
	void  down(struct semaphore * sem);  
	int  down_interruptible(struct semaphore * sem);  // 申请资源，如果没有资源，阻塞睡眠
	int  down_trylock(struct semaphore * sem);

	信号量的释放
	void up(struct semaphore * sem);
	
	

【9】互斥体

    使用方法及场合和信号量一致， 理解上同应用层中互斥锁一致。
	
	初始化
	struct mutex my_mutex;
	mutex_init(&my_mutex);
	
	获取互斥体
	void inline _ _sched mutex_lock(struct mutex *lock);
	
	释放互斥体
	void __sched mutex_unlock(struct mutex *lock);

   
	
	
    	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	