【1】IO模型
     1，阻塞式IO
	    最常见，最简单，效率最低的一种IO模型，系统默认设置的函数，一般为阻塞函数。
		 读阻塞：getchar,read,fgets,....
		 写阻塞: write , send, fputs ....
		 其他阻塞：accept, connect
	 
	 2，非阻塞式IO
	    需要cpu不断轮询，使用较少，需要设置非阻塞标志位。
	 
	 3，IO多路复用
	    允许同一个进程，进行多路输入输出操作。
	 
	 4，信号驱动IO
	    异步io模型，类似于中断的形式。
	 
【2】阻塞式IO
     
    定义“等待队列头”
	wait_queue_head_t  my_queue;
	
	初始化“等待队列头”
	init_waitqueue_head(&my_queue);
	
	定义等待队列
	DECLARE_WAITQUEUE(name, tsk)
	添加/移除等待队列
	void  fastcall  add_wait_queue(wait_queue_head_t *q, wait_queue_t *wait);
	void  fastcall  remove_wait_queue(wait_queue_head_t *q, wait_queue_t *wait);

	/// 将引起阻塞的进程，打包成节点，插入等待队列中去。
	等待事件 
	wait_event(queue, condition)  // 阻塞过程不能被打断
	wait_event_interruptible(queue, condition)  // 阻塞过程中，可以被打断， queue 等待队列头，condition 唤醒条件，可以是一个表达式
	wait_event_timeout(queue, condition, timeout)
	wait_event_interruptible_timeout(queue, condition, timeout)

	唤醒队列
	void wake_up(wait_queue_head_t *queue);
	void wake_up_interruptible(wait_queue_head_t *queue);

////////////////////////////////////////////////////////////////////////////////
   struct __wait_queue_head {
		spinlock_t		lock;
		struct list_head	task_list;  // 只要包含了此结构体，那么当前节点，就问内核循环双链表节点
   };
  
  typedef struct __wait_queue_head wait_queue_head_t;
  
////////////////////////////////////////////////////////////////////////////////
  #define init_waitqueue_head(q)				\
	do {						\
		static struct lock_class_key __key;	\
							\
		__init_waitqueue_head((q), #q, &__key);	\
	} while (0) 
  
   ---->
  
    void __init_waitqueue_head(wait_queue_head_t *q, const char *name, struct lock_class_key *key)
	{
		INIT_LIST_HEAD(&q->task_list);   // 对队列的头进行初始化
	}
/////////////////////////////////////////////////////////////////////////////////////////////
	#define wait_event_interruptible(wq, condition)				\
	({									\
		int __ret = 0;							\
		if (!(condition))						\
			__ret = __wait_event_interruptible(wq, condition);	\
		__ret;								\
	})

  ------>
  #define __wait_event_interruptible(wq, condition)			\
	      ___wait_event(wq, condition, TASK_INTERRUPTIBLE, 0, 0,		\
		      schedule())   // schedule 调度函数
			  
  ------>
  #define ___wait_event(wq, condition, state, exclusive, ret, cmd)	\
	({									\				\
	   // 等待队列的节点
		wait_queue_t __wait;						\
		
		// 为节点进行初始化
		INIT_LIST_HEAD(&__wait.task_list);				\
	
	    // 死循环
		for (;;) {							\
			long __int = prepare_to_wait_event(&wq, &__wait, state);\
										\
			// 条件为真，跳出for循环
			if (condition)						\
				break;						\
			
			// 接收到中断信号，也会跳出 for循环
			if (___wait_is_interruptible(state) && __int) {		\
				__ret = __int;					\
				if (exclusive) {				\
					abort_exclusive_wait(&wq, &__wait,	\
								 state, NULL);	\
					goto __out;				\
				}						\
				break;						\
			}							\
										\
			cmd;							\  /// schedule() ，放弃cpu的调度权
		}								\
		finish_wait(&wq, &__wait);					\  // 结束睡眠
	__out:	__ret;								\
	})
	  
/////////////////////////////////////////////////////////////////////////////////////  
  /// 将进程打包成此节点
  typedef struct __wait_queue wait_queue_t;
   struct __wait_queue {
		unsigned int		flags;
		void			*private;  // 存储进程相关信息
		wait_queue_func_t	func;  // 回调函数
		struct list_head	task_list;  // 循环双链表的节点
	};

////////////////////////////////////////////////////////////////////////////////////////
long __int = prepare_to_wait_event(&wq, &__wait, state);\
											
  long prepare_to_wait_event(wait_queue_head_t *q, wait_queue_t *wait, int state)
{
	unsigned long flags;

	wait->private = current;  //  current 当前的进程
	wait->func = autoremove_wake_function;  // 回调函数

	if (list_empty(&wait->task_list)) {
		if (wait->flags & WQ_FLAG_EXCLUSIVE)
			__add_wait_queue_tail(q, wait);  // 添加到尾部
		else
			__add_wait_queue(q, wait);  // 头插法
	}
	set_current_state(state);   // 设置进程的状态

	return 0;
}
  
 ////////////////////////////////////////////////////////////////////////////////////
void finish_wait(wait_queue_head_t *q, wait_queue_t *wait)
{
	unsigned long flags;

	// 将进程设为运行态
	__set_current_state(TASK_RUNNING);
	
	if (!list_empty_careful(&wait->task_list)) {
		list_del_init(&wait->task_list);   /// 将阻塞的进程从等待队列中移除
	}
} 
  
///////////////////////////////////////////////////////////////////////////////////////  
  #define wake_up(x)			__wake_up(x, TASK_NORMAL, 1, NULL)
  
  ---->
  void __wake_up(wait_queue_head_t *q, unsigned int mode,
			int nr_exclusive, void *key)
	{
		。。。
		__wake_up_common(q, mode, nr_exclusive, 0, key);
	}
  
  ---->
  static void __wake_up_common(wait_queue_head_t *q, unsigned int mode,
			int nr_exclusive, int wake_flags, void *key)
	{
		wait_queue_t *curr, *next;

		// 遍历等待队列
		list_for_each_entry_safe(curr, next, &q->task_list, task_list) {
			unsigned flags = curr->flags;

			/// 执行回调函数
			if (curr->func(curr, mode, wake_flags, key) &&
					(flags & WQ_FLAG_EXCLUSIVE) && !--nr_exclusive)
				break;
		}
	}
  

【3】内核循环双链表
   // 循环双链表的定义
   struct list_head {
		struct list_head *next, *prev;
   };

   // 初始化
   static inline void INIT_LIST_HEAD(struct list_head *list)
	{
		list->next = list;
		list->prev = list;
	}
	
   // 插入节点
   static inline void list_add(struct list_head *new, struct list_head *head)
	{
		__list_add(new, head, head->next);
	}

    // 尾插法
	static inline void list_add_tail(struct list_head *new, struct list_head *head)
	{
		__list_add(new, head->prev, head);
	}

	// 删除节点
    static inline void list_del(struct list_head *entry)
	{
		__list_del(entry->prev, entry->next);
		entry->next = LIST_POISON1;   // 指向一个特定内存地址
		entry->prev = LIST_POISON2;
	}

	// 遍历
	#define list_for_each(pos, head) \
	for (pos = (head)->next; pos != (head); pos = pos->next)

     list_for_each(pos, head) 
	 {
	     ....
	 }

【4】非阻塞式IO  
    应用程序：
	     1，设置非阻塞标志位
		    文件打开时设置非阻塞标志位：
		    fd = open("",  O_NONBLOCK | ...);

            使用fcntl 也可以设置非阻塞标志位：
			int fcntl(int fd, int cmd, ... /* arg */ );

			int flags = fcntl(fd, F_GETFL);
			flags |= O_NONBLOCK;
			fcntl(fd, F_SETFL, flags);

    驱动程序：
	    
	struct file {
    ....
	unsigned int 		f_flags;   // 存储了文件的标志位信息
	
	}

       if(counter == 0)
 49     {
 51         if(filep->f_flags & O_NONBLOCK)
 52         {
 53             return -EAGAIN;
 54         }
 55 
 56         if(wait_event_interruptible(wq, counter != 0))
 57         {
 58             return -ERESTARTSYS;
 59         }
 60                                                                                                                   
 61     }

【5】多路IO复用
     解决多个输入输出阻塞问题.

	if(当前的文件描述符资源是否可用)
	{
     fgets();
    }
	if(当前的文件描述符资源是否可用)
	{
     read();	 
    }
	
	应用程序：
	
	int select(int nfds, fd_set *readfds, fd_set *writefds,
                  fd_set *exceptfds, struct timeval *timeout);
	功能：允许一个进程监听多个文件描述符集合，直到有一个或者多个文件描述符准备就绪，
	      那么函数立刻返回，否则一直阻塞等待下去。
	参数：nfds  最大文件描述符数加1
	      readfds  读集合
		  writefds 写集合
		  exceptfds  其他异常的集合
		  timeout  超时值
	返回值：成功 准备就绪的文件描述符的个数
	        出错 -1
			没有考虑 timeout 情况， timeout 值为NULL
			
	注意：当函数返回时，内核会将所有没有准备就绪的文件描述符，从集合中清除掉。

    void FD_CLR(int fd, fd_set *set);  // 将fd 从 集合中清除掉
    int  FD_ISSET(int fd, fd_set *set);  // 判断 fd 是否在集合中
    void FD_SET(int fd, fd_set *set);   // 将fd 添加到集合中
    void FD_ZERO(fd_set *set);  // 清空 集合
  
	  
	驱动实现：
    unsigned int (*poll) (struct file * filep, struct poll_table_struct * wait);	
	
	1. 在一个或多个可指示查询状态变化的等待队列上调用 poll_wait. 如果没有文件描述符可用作 I/O, 
	   内核使这个进程在等待队列上等待所有的传递给系统调用的文件描述符. 

	2. 返回一个位掩码, 描述可能不必阻塞就立刻进行的操作.
	   POLLIN  数据可读
	   POLLRDNORM  读数据正常
	   POLLOUT  数据可写
	   POLLWRNORM  写数据数据
	   POLLERR  出错
	   
	通过poll_wait可以向驱动向poll_table结构添加一个等待队列。
    
	static inline void poll_wait(struct file * filp, wait_queue_head_t * wait_address, poll_table *p)
	{
		if (p && p->_qproc && wait_address)
			p->_qproc(filp, wait_address, p);
	}
	功能：将进程相关的信息（文件描述符信息），打包成节点，添加到等待队列中。在将此等待队列，添加到poll_table
	      表中，内核可通过poll_table 表来查询文件描述符的状态变化。
		  
	/// 它不是一个阻塞函数
	  
	// poll_table表的定义
	typedef struct poll_table_struct {
		poll_queue_proc _qproc;
		unsigned long _key;
	} poll_table;
	  
	typedef void (*poll_queue_proc)(struct file *, wait_queue_head_t *, struct poll_table_struct *);
	  
	///////////////////////////////////////////////////////////////////////////////////////////////////

     select() --> sys_select() --> ....  --> demo_poll() --> poll_wait()  ---> .... ---> 硬件		
	  
【6】信号驱动IO
     异步通知的意思是：一旦设备就绪，则主动通知应用程序，这样应用程序根本就不需要查询设备状态，
	                   这一点非常类似于硬件上“中断”的概念，比较准确的称谓是“信号驱动的异步I/O”。
					   
	实现信号驱动io，依赖于异步队列。
	
	void handler()
	{
	    read();
	}
    
	signal(SIGIO, handler);
	
	while(1)
	{
		printf("+++++++++++++++++++=\n");
		sleep(1);
	}
	
    驱动中实现：
	    int (*fasync) (int, struct file *, int);

	  
	/////////////////////////////////////////////////////////////////////////////////////////////
    
    应用程序：
           1，设置异步标志位  O_ASYNC
                  open, fcntl

              int  flags = fcntl(fd, F_GETFL);
                   flags |= O_ASYNC;
             	fcntl(fd, F_SETFL, flags);   
	  
	       2，设置文件的属主
	           fcntl(fd, F_SETOWN, getpid());
	  
	       3，注册SIGIO 信号
		      signal(SIGIO, handler);
			  
	驱动程序：
	   // 驱动中，只需要实现此函数即可，（只需要调用 fasync_helper()）
	   int (*fasync) (int fd, struct file * filep, int mode);
	   
	   int fasync_helper(int fd, struct file * filp, int on, struct fasync_struct **fapp)
	   功能：向字符设备驱动，注册一个异步队列
	   参数：fd   文件描述符
	        filep  文件指针
			on    开关 ， 添加 或者 移除, 由应用程序中，O_ASYNC 异步标志位决定，设置时即为真，
			fapp  异步队列
	   返回值：出错 负数的错误码
	           0    代表没有任何改变
			   正数，  添加或者删除
	   含义：将文件信息fd，进程号，打包成节点，添加到异步队列中去。
	   
	   void kill_fasync(struct fasync_struct **fp, int sig, int band)
	   功能：给指定的进程发送信号
	   参数：fp  异步队列的首地址
	         sig  信号
			 band  POLLIN  读事件
			       POLLOUT  写事件
				   POLLRDNORM  ...
	   
	  
	///////////////////////////////////////////////////////////////////////////////////////////  
	  
	  struct fasync_struct {
		spinlock_t		fa_lock;
		int			magic;
		int			fa_fd;  // 文件描述符
		struct fasync_struct	*fa_next; /* 单链表 */
		struct file		*fa_file;  // 文件信息指针
		struct rcu_head		fa_rcu;
	};
		  
	/////////////////////////////////////////////////////////////////////////////////////////////  
	  
	int fasync_helper(int fd, struct file * filp, int on, struct fasync_struct **fapp)
	{
		if (!on)
			return fasync_remove_entry(filp, fapp);  // 删除节点
		return fasync_add_entry(fd, filp, fapp);   // 添加节点
	}  
		  
	---->

    static int fasync_add_entry(int fd, struct file *filp, struct fasync_struct **fapp)
	{
		struct fasync_struct *new;

		// 开辟一个新的节点
		new = fasync_alloc();
		// 插入新节点
		if (fasync_insert_entry(fd, filp, fapp, new)) {
			fasync_free(new);
			return 0;
		}
		return 1;
	}	
	
    ------->
    struct fasync_struct *fasync_insert_entry(int fd, struct file *filp, struct fasync_struct **fapp, struct fasync_struct *new)
	{
	   struct fasync_struct *fa, **fp;

	   // 对新节点进行初始化
		new->magic = FASYNC_MAGIC;
		new->fa_file = filp;
		new->fa_fd = fd;
		new->fa_next = *fapp;
		
		rcu_assign_pointer(*fapp, new);
		filp->f_flags |= FASYNC;

		return fa;
	}	
	  
	///////////////////////////////////////////////////////////////////////////////////////////////////////////

    struct file {

	  struct fown_struct	f_owner;	
      ...
	}

	---->
	
	struct fown_struct {
		rwlock_t lock;          /* protects pid, uid, euid fields */
		struct pid *pid;	/* 进程pid */
		enum pid_type pid_type;	/* Kind of process group SIGIO should be sent to */
		kuid_t uid, euid;	/* uid/euid of process setting the owner */
		int signum;		/* posix.1b rt signal to be delivered on IO */
	};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   echo "31245" >> /dev/demo   ---> demo_open()  , demo_write()























	 