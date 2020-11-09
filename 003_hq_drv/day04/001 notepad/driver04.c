【1】ioctl中生成命令码
    生成命令码：
      _IO(type,nr)         //  生成一个命令码，type 代表的是一个幻数 ， 填充一个字符， 可以是任意的字符
                           //  nr  代表的是一个自然数， 范围在 0 - 255  	  
	  _IOR(type,nr,size)    // 读命令字 ， size  表示的是  数据的类型
	  _IOW(type,nr,size)    // 写命令字
      _IOWR(type,nr,size)   // 读写命令字
	  
	
	// 通过4个有意义的位来进行描述的。
	#define _IOC_NRBITS     8
	#define _IOC_TYPEBITS   8
	#define _IOC_SIZEBITS   14
	#define _IOC_DIRBITS    2

	#define _IOC_NRMASK     ((1 << _IOC_NRBITS)-1)
	#define _IOC_TYPEMASK   ((1 << _IOC_TYPEBITS)-1)
	#define _IOC_SIZEMASK   ((1 << _IOC_SIZEBITS)-1)
	#define _IOC_DIRMASK    ((1 << _IOC_DIRBITS)-1)

	#define _IOC_NRSHIFT    0
	#define _IOC_TYPESHIFT  (_IOC_NRSHIFT+_IOC_NRBITS)   // 0 + 8
	#define _IOC_SIZESHIFT  (_IOC_TYPESHIFT+_IOC_TYPEBITS)  // 8 + 8
	#define _IOC_DIRSHIFT   (_IOC_SIZESHIFT+_IOC_SIZEBITS)   //   14 + 8 + 8
	
	#define _IOC_NONE       0U
	#define _IOC_WRITE      1U
	#define _IOC_READ       2U

	#define _IOC(dir,type,nr,size) (((dir)  << _IOC_DIRSHIFT) | ((type) << _IOC_TYPESHIFT) | 
	                                 ((nr)   << _IOC_NRSHIFT) | ((size) << _IOC_SIZESHIFT))
             // dir << 30   [31:30]
		     // type << 8   [15:8]    // 
             // nr  << 0    [7:0]
             // size << 16	[29:16]		 
									 
									 
	/* used to create numbers */
	#define _IO(type,nr)	        _IOC(_IOC_NONE,(type),(nr),0)
	#define _IOR(type,nr,size)      _IOC(_IOC_READ,(type),(nr),sizeof(size))
	#define _IOW(type,nr,size)      _IOC(_IOC_WRITE,(type),(nr),sizeof(size))
	#define _IOWR(type,nr,size)     _IOC(_IOC_READ|_IOC_WRITE,(type),(nr),sizeof(size))
			
【2】自动申请注册设备号，自动创建设备节点
     1，自动申请注册设备号
	    int alloc_chrdev_region(dev_t *dev, unsigned baseminor, unsigned count,
			const char *name)
		功能：...
		参数：dev  存储自动申请注册的设备号
		      baseminor  次设备号
			  count   次设备的数量
			  name    设备号的名称  /// cat /proc/devices
	    返回值：成功0，出错负数的错误码

		#define MINORBITS	20
		#define MINORMASK	((1U << MINORBITS) - 1)  // 1 << 20  - 1 ==>  1<<1  ， 10
		                                             // 1 0000 0000 0000 0000 0000  -1  = fffff

		#define MAJOR(dev)	((unsigned int) ((dev) >> MINORBITS))  // devno >> 20
		#define MINOR(dev)	((unsigned int) ((dev) & MINORMASK))   // devno & 0xfffff
		#define MKDEV(ma,mi)	(((ma) << MINORBITS) | (mi)   // ma << 20 | mi
		
	2， 动态申请分配一个cdev结构体
	    struct cdev *cdev_alloc(void)

    3，自动创建设备节点
	
	#define class_create(owner, name)		\
	({						\
		__class_create(owner, name, &__key);	\
	})
	功能：创建一个类
	参数：owner  THIS_MODULE
	      name   类的名称
	返回值：类的首地址	  
    struct class *__class_create(struct module *owner, const char *name,
			     struct lock_class_key *key)
				 
	void class_destroy(struct class *cls)
	功能：销毁一个类
	
	// mknod  
	struct device *device_create(struct class *class, struct device *parent,
			     dev_t devt, void *drvdata, const char *fmt, ...)
	功能：自动创建设备节点
	参数：class  类
	      parent  父亲节点，一般为NULL
		  devt  设备号
		  drvdata  添加到设备中的数据，一般为NULL
		  fmt    设备节点的名称  // 此名称必须和open中的路径名称一致
		  ...    可变参数  // 与 fmt 结合组成一个新的字符串
	返回值：成功 设备的地址，出错 ERR_PTR

    printf("hell %s", str);

	void device_destroy(struct class *class, dev_t devt)
	功能：移除一个设备节点
	参数：class  类
	      devt   设备号
		  
【3】struct inode  在内核中，用此结构体静态的来表示一个文件的属性信息
     当用户执行 mknod  命令时， mknod /dev/hello  c 250 0,  创建了一个设备文件 /dev/hello ,
	 那么内核中，就会创建一个struct inode 的结构体。
	 一个文件对应唯一的一个inode结构体。

	struct inode {
			umode_t			i_mode;  // 文件打开方式
			unsigned short		i_opflags;
			kuid_t			i_uid;   // 用户id
			kgid_t			i_gid;    // 组id
			unsigned int		i_flags;  // 标志位信息
			const struct inode_operations	*i_op;  // inode 操作集合
			struct super_block	*i_sb;
			struct address_space	*i_mapping;
			unsigned long		i_ino;  // inode号码
			
			union {
				const unsigned int i_nlink;
				unsigned int __i_nlink;  // 链接数
			};
			dev_t			i_rdev;   // 设备号
			loff_t			i_size;
			struct timespec		i_atime;  // 访问时间
			struct timespec		i_mtime;  // 修改时间
			struct timespec		i_ctime; // 创建时间
	
			const struct file_operations	*i_fop;	/* 文件操作集合指针 */
			struct list_head	i_devices;  // 它也是一个循环双链表的节点
			union {
				struct pipe_inode_info	*i_pipe;
				struct block_device	*i_bdev;
				struct cdev		*i_cdev;   // 字符设备对象的指针
			};
			void			*i_private; /* fs or device private pointer */
			.....
		};		  

【4】struct file 结构体， 它也表示的是文件属性信息，动态表示的。
     应用程序每调用一次 open 函数，内核中就会创建一个 struct file 的结构体。
	 
	 struct file {
			union {
				struct llist_node	fu_llist;
				struct rcu_head 	fu_rcuhead;
			} f_u;
			struct path		f_path;   // 路径名称
			struct inode		*f_inode;	/* cached value ， inode 指针 */
			const struct file_operations	*f_op;  // 文件操作集合

			unsigned int 		f_flags;  // 文件的操作标识位
			fmode_t			f_mode;  // 打开方式
			loff_t			f_pos;
			struct fown_struct	f_owner;  // 文件拥有者
			/* needed for tty driver, and maybe others */
			void			*private_data;
			struct address_space	*f_mapping;  // 地址映射
	
		} __attribute__((aligned(4)));	


    open("/dev/hello", ....)   ---> struct file  |  
	                                             | ----> inode  ---> 真正的文件
	open("/dev/hello", ....)   ---> struct file  |
	
	*.c.swp

【5】应用程序 如何调到驱动中的程序。
    open() --> .... --> demo_open();
	
	
	// struct Inode  组成了一个循环双链表， devno ， cdev指针
	struct inode  <--->   struct inode  <--->  struct inode  <---->  
	           

    // struct cdev 也组成了一个循环双链表, devno , cdev地址
	struct cdev <---> struct cdev <--->  struct cdev <---> struct cdev
	
    // 两条链表按照 devno 的值，进行匹配，一旦匹配成功，将cdev地址 赋值给 cdev指针

【6】点亮一个LED 灯 
    1，查看原理图
	   LED5  --->  GPF3_5     [23:20]

    2，三星手册
	   GPF3CON  0x114001E0     
       GPF3DAT  0x114001E4
	   
	3，led 灯的驱动
	
	4, 修改 Makefile 中，内核的源码路径
	
【7】地址映射
    void __iomem *ioremap(unsigned long paddr, unsigned long size)
	功能：将物理地址，映射为虚拟地址
	参数：paddr   物理地址
	      size    大小
	返回值：映射后的虚拟地址
	        出错 NULL
			
	void iounmap(const void __iomem *addr)
	功能：解除映射关系
	参数：addr  隐射的虚拟地址
	
【8】对寄存器进行读写操作
    inline u32 readl(const volatile void __iomem *addr)
	 功能：将寄存器中数据读出
	 参数：addr  虚拟的寄存器地址
	 返回值：寄存器的值
	 
    inline void writel(u32 b, volatile void __iomem *addr)	
	功能：对寄存器进行写操作
	参数：b    值
	      addr  地址
	
【9】流水灯，作业
     
    while(1)
	{
	    ioctl(fd, LED_ON , &NUM);
		
		ioctl(fd, LED_OFF , &NUM);
	}	
	
    	















	 

    











		
