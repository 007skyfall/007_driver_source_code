【1】字符设备驱动框架
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
		#define MKDEV(ma,mi)	(((ma) << MINORBITS) | (mi))  // 生成一个设备号，宏函数
		
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
				ssize_t (*read) (struct file *, char __user *, size_t, loff_t *);  // 读
				ssize_t (*write) (struct file *, const char __user *, size_t, loff_t *); // 写
				unsigned int (*poll) (struct file *, struct poll_table_struct *);  // 多路IO复用
				long (*unlocked_ioctl) (struct file *, unsigned int, unsigned long);  // 对设备属性进行设置 或者 获取的函数
				int (*open) (struct inode *, struct file *);  // 打开文件
				int (*release) (struct inode *, struct file *);  // 关闭文件
				int (*fasync) (int, struct file *, int);  // 信号驱动IO
				......
		};
		
4、将设备号注册到内核中
      int register_chrdev_region(dev_t from, unsigned count, const char *name)
      功能：......
      参数：from  设备号
            count 次设备的数量
            name  设备号的名称   //  cat  /proc/devices
      返回值：成功0，出错 -1
	  
5、对字符设备对象进行初始化
	  void cdev_init(struct cdev *cdev, const struct file_operations *fops)
	  功能：......
	  参数：cdev  字符设备对象
	        fops  字符设备对象操作方法集合
			
6、将cdev注册进内核
       int cdev_add(struct cdev *p, dev_t dev, unsigned count)
       功能：......
       参数：p  字符设备对象地址
             dev  设备号
             count  次设备的数量
       返回值：成功 0，出错 负数的错误码			 
	  
7、注销设备号（取消设备号的注册）
	    void unregister_chrdev_region(dev_t from, unsigned count)
		功能：......
		参数：from  设备号
		      count 次设备号的数量
			  
8、移除一个字符设备对象
	    void cdev_del(struct cdev *p)
		
9、创建设备节点文件（将设备号与文件名称进行绑定）
       sudo   mknod   /dev/hello    c        250        0
	                 设备节点名称  字符设备  主设备号  次设备号
10、查看系统中设备号的使用情况
	   cat  /proc/devices
	 
【2】字符设备完成读写功能
     
	//  read(fd,  buf,  N);
	ssize_t (*read) (struct file *, char __user *, size_t, loff_t *);
	功能：实现读函数功能
	参数：filep  文件指针
	      buffer  应用程序传递下来的地址
	ssize_t (*write) (struct file *, const char __user *, size_t, loff_t *);		  
	功能：实现写函数功能
		
	  
	unsigned long copy_from_user(void *to, const void __user *from, unsigned long n)
    功能：将用户空间的数据拷贝到内核空间来。
    参数：to  	目的地址，内核空间
          from 	源地址，用户空间
          n   	要拷贝字节数
    返回值：成功0，出错非0
	
	unsigned long copy_to_user(void __user *to, const void *from, unsigned long n)
	功能：将内核空间的数据拷贝到用户空间
	参数：to  	目的地址
	      from 	源地址
		  n  	 大小
	返回值：成功 0，出错 剩余未拷贝字节数	
	
【3】struct inode 结构体
     struct inode 结构体，用来静态的表示一个文件。存储了文件的各种属性信息。
	 当执行mknod 命令时，内核中会创建一个struct inode 结构体，此结构体就来表示一个设备文件。
	 mknod  /dev/hello  c  250  0
    
	struct inode {
		umode_t			i_mode;  // 打开方式
		kuid_t			i_uid;   // 用户id
		kgid_t			i_gid;   // 组id
		unsigned int		i_flags; // 标志位
		unsigned long		i_ino;  // 文件的唯一标识
		union {
			const unsigned int i_nlink;
			unsigned int __i_nlink;   // 链接数
		};
		dev_t			i_rdev;  // 设备号
		loff_t			i_size;  // 大小
		struct timespec		i_atime;
		struct timespec		i_mtime;
		struct timespec		i_ctime;  //时间属性信息
	　　．．．．
		const struct file_operations	*i_fop;	// 文件操作集合指针
		struct list_head	i_devices;  // 循环双链表结点
		union {
			......
			struct cdev		*i_cdev;  // 字符设备对象指针
		};
		void			*i_private; /* fs or device private pointer */
		......
	};

【4】struct file 结构体
     它也是文件的属性信息，表示的是动态的属性信息.
	 当应用程序，每调用一次open，那么内核中就会生成一个struct file的结构体。
	 
	 struct file {
			struct path		f_path;  // 文件路径
			struct inode		*f_inode;	/* cached value */
			const struct file_operations	*f_op; // 文件操作集合
			unsigned int 		f_flags;  // 文件标志位信息
			fmode_t			f_mode;  // 操作方式
			loff_t			f_pos;
			struct fown_struct	f_owner;
			u64			f_version;
			void			*private_data;
	     ......
	};

    open()  ---> struct file  |
	open()  ---> struct file  |  ---> struct inode  ---> 文件本身
	open()  ---> struct file  |

【5】ioctl操作（对设备文件的属性，进行设置或者获取）
     long (*unlocked_ioctl)(struct file*, unsigned int cmd, unsigned long arg)
	 
	 驱动程序一般需支持通过ioctl实现各种控制与参数设置，如串口可设置波特率等多参数
		功能：。。。
    参数：filep  文件指针
          cmd    命令字 ，自己制造的
          arg    应用程序，第3个可变参数，传递进来的地址
    返回值：成功0，出错 -1		  
	 semctl， shmctl， msgctl
	 
	 int ioctl(int d, int request, ...);
     功能：对设备的属性进行设置或者获取
	 参数：d 文件描述符
	       request  命令码
		   .....可变参数
	 返回值：成功0，出错 -1
	 
     生成命令码：
	#define _IOC_NRBITS     8
	#define _IOC_TYPEBITS   8
	#define _IOC_SIZEBITS   14
	#define _IOC_DIRBITS    2

	#define _IOC_NRMASK     ((1 << _IOC_NRBITS)-1)
	#define _IOC_TYPEMASK   ((1 << _IOC_TYPEBITS)-1)
	#define _IOC_SIZEMASK   ((1 << _IOC_SIZEBITS)-1)
	#define _IOC_DIRMASK    ((1 << _IOC_DIRBITS)-1)

	#define _IOC_NRSHIFT    0
	#define _IOC_TYPESHIFT  (_IOC_NRSHIFT+_IOC_NRBITS)  // 0 + 8
	#define _IOC_SIZESHIFT  (_IOC_TYPESHIFT+_IOC_TYPEBITS) // 8 + 8
	#define _IOC_DIRSHIFT   (_IOC_SIZESHIFT+_IOC_SIZEBITS)  //16 + 14

	/*
	 * Direction bits.
	 */
	#define _IOC_NONE       0U
	#define _IOC_WRITE      1U
	#define _IOC_READ       2U

	#define _IOC(dir,type,nr,size) (((dir)  << _IOC_DIRSHIFT) | ((type) << _IOC_TYPESHIFT) | ((nr)   << _IOC_NRSHIFT) | ((size) << _IOC_SIZESHIFT))
    dir << 30 | type << 8 | nr << 0 | size << 16
	dir : [31:30]  // 命令码的方向
	size: [29:16]  // 数据的大小
	type: [15:8]   // 幻数，8位，一般用字符来表示
	nr  : [7:0]    // 8位的自然数
	
	/* used to create numbers */
	#define _IO(type,nr)	    	_IOC(_IOC_NONE,(type),(nr),0)
	#define _IOR(type,nr,size)      _IOC(_IOC_READ,(type),(nr),sizeof(size))
	#define _IOW(type,nr,size)      _IOC(_IOC_WRITE,(type),(nr),sizeof(size))
	#define _IOWR(type,nr,size)     _IOC(_IOC_READ|_IOC_WRITE,(type),(nr),sizeof(size))
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	



	 


















	
	
		  
	  
	  
	  
	  
	  
	  
	  
	  
	  
	  
	  
	  
	  
	  
	  
	  
	  
	  
	  
	  
	  
	  
	  
	  
	  
	  
	  
	  