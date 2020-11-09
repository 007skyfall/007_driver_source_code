【1】已知一个结构体成员的地址，求出此结构体的首地址

     struct node {
		int a;
		int c;
		...;
		int d;   // 知道d 的地址，求出结构体首地址。
		short e;
	 };
	 
	 container_of(ptr, type, member)
	 功能：已知一个结构体成员的地址，求出结构体的首地址
	 参数：ptr  已知的成员地址
	       type  类型 （结构体类型）
		   member 已知成员地址的名称
	 返回值：得到结构体的首地址
	 
	////////////////////////////////////////////////////////////////////////////////////////////////
    #define container_of(ptr, type, member) ({			\
											const typeof(((type *)0)->member) * __mptr = (ptr);	\
											(type *)((char *)__mptr - offsetof(type, member)); })	// 绝对地址 - 偏移量
											
	// typeof(((type *)0)->member)   , 已知一个结构体成员的名字，求出成员的类型
	 
	// 计算成员相对于结构体的偏移量, 将0地址强制类型转换为结构体的指针
       offsetof(type, member)	
	     ----> #define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)  // &  取地址 ， size_t 强制类型转换，将地址转换为大小
	 
【2】linux中的并发
    并发（concurrency）指的是多个执行单元同时、并行被执行，而并发的执行单元对共享资源
	   （硬件资源和软件上的全局变量、静态变量等）的访问则很容易导致竞态（race conditions）
	
	竞态发生情况：
	1，对称多处理器cpu 
       
	2，单CPU内进程与抢占它的进程

	3，中断（硬中断、软中断、Tasklet、底半部）与进程之间

	解决竞态的方法：
	保证对共享资源的互斥访问，所谓互斥访问是指一个执行单元在访问共享资源的时候，其他的执行单元被禁止访问。
	访问共享资源的代码区域称为临界区（critical sections），临界区需要被以某种互斥机制加以保护。
	
	互斥机制：
		1，中断屏蔽
		2，原子操作
		3，自旋锁
		4，信号量
		5, 互斥体

【3】中断屏蔽
    local_irq_disable() /* 屏蔽中断 */
		. . .
 	critical section /* 临界区*/
	    . . .
	local_irq_enable()  /* 开中断*/
	
	注意：local_irq_disable()和local_irq_enable()都只能禁止和使能本CPU内的中断，因此，并不能解决SMP多CPU引发的竞态。
	
【4】原子操作
     对原子变量值的操作，是原子进行的。

    int flags = 1;

    if(flags == 1)
	{
		flags = 0;
	}
    else
	{
	   return ... ;
	}	
	
	整型原子操作：
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
	
	#define atomic_sub_and_test(i,v)	(atomic_sub_return((i), (v)) == 0) // 真，申请到了资源；假，没有申请到资源
	
	操作并返回
	int atomic_add_return(int i, atomic_t *v);
	int atomic_sub_return(int i, atomic_t *v);
	int atomic_inc_return(atomic_t *v);
	int atomic_dec_return(atomic_t *v);
	上述操作对原子变量进行加/减和自增/自减操作，并返回新的值
	
	原子变量：
	    typedef struct {
			int counter;
		} atomic_t;
	
【5】自旋锁
	自旋锁实际上是忙等锁，自旋锁可能导致系统死锁，自旋锁锁定期间不能调用可能引起进程调度的函数。
	不能执行睡眠函数  sleep(),尽量不要做耗时操作。

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
	
	///////////////////////////////////////////////////////////////////////////////
    A进程：
	
    申请锁
      临界区代码
    释放锁	  
	
	B进程：
	
    申请锁
      临界区代码
    释放锁	 

	/// 代码是有bug
    C进程：
      临界区代码	
	//////////////////////////////////////////////////////////////////////////////
	
【6】信号量
    信号量是内核中用来保护临界资源的一种，信号量保护的临界资源，可以有睡眠，调度。
    
	互斥机制：
	相关头文件 <linux/semaphore.h>

	定义信号量
	struct semaphore sem;
	
	初始化信号量
	void  sema_init(struct semaphore *sem, int val);

	信号量的获取
	void down(struct semaphore * sem);  // 申请资源
	int  down_interruptible(struct semaphore * sem);  // 申请资源，申请资源的过程中可以被打断。 
	int  down_trylock(struct semaphore * sem);

	信号量的释放
	void up(struct semaphore * sem);

【7】互斥体
    使用方法及场合和信号量一致
	
	初始化
	struct mutex my_mutex;
	mutex_init(&my_mutex);
	
	获取互斥体
	void inline _ _sched mutex_lock(struct mutex *lock);
	
	释放互斥体
	void __sched mutex_unlock(struct mutex *lock);
	
【8】流水灯LED
    
	应用程序：
	   fd = open(,);
	   
	   while(1)
	   {
		  ioctl(fd, LED_ON , &num);
		  sleep(1);
		  ioctl(fd, LED_OFF, &num);
	   
		  num++;
		  
		  if(num == ...)
		  {
		     num = ...
		  }
	   
	   }
	
	
	驱动程序：
	
	     实现 ioctl函数
		 
    demo_ioctl()
	{
		switch(cmd)
		{
			case LED_ON:
			      fs4412_led_on(&num);
		    case LED_OFF:
			      fs4412_led_off(&num);
		}
	
	}
	
【8】PWM 蜂鸣器
     1，查看原理图

     2，查看三星寄存器器

     3，驱动
        	 
	
	
	驱动 ：
	
	   demo_ioctl()
	   {
	      switch( cmd)
		  {
		     case PWM_ON:
			      。。。。;
				  break;
		    case PWM_OFF:
			      。。。。;	  
		  
		  }
	   
	   }
	
	
	


	
	 
	 
	 
	 
	 
	 
	 
	 
	 
	 
	 
	 
	 
	 
	 
	 
	 
	 
	 
	 
	 
	 
	 