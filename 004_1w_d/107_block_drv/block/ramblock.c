#include <linux/init.h>
#include <linux/module.h>
#include <linux/blkdev.h>
#include <linux/genhd.h>
#include <linux/slab.h>
#include <linux/hdreg.h>
#include <linux/vmalloc.h>

#define BLK_NAME "ramblk"
#define BLK_SIZE (1*1024*1024)
static struct gendisk *ramblk;
static int major;
struct request_queue *q;
spinlock_t lock;
void * ram_buf;

int ramblk_open(struct block_device * blk_dev, fmode_t mode)
{
	printk("%s:%s:%d\n",__FILE__,__func__,__LINE__);
	return 0;
}
int ramblk_release(struct gendisk * disk, fmode_t mode)
{
	printk("%s:%s:%d\n",__FILE__,__func__,__LINE__);
	return 0;
}
int ramblk_getgeo(struct block_device *blk_dev, struct hd_geometry *hd)
{
	printk("%s:%s:%d\n",__FILE__,__func__,__LINE__);
	hd->heads = 2;           //有多少面
	hd->cylinders = 20;   //多少环
	hd->sectors = BLK_SIZE/2/20/512;//一环内有多少个扇区
	return 0;
}

static struct block_device_operations ramblk_fops = {
	.open    = ramblk_open,
	.release = ramblk_release,
	.getgeo  = ramblk_getgeo,
};

void request_ramblk_proc(struct request_queue *q)
{
	struct request *req;
	//队列处理函数
 
	req = blk_fetch_request(q);//如果队列不为空，取出来一个处理
	while (req) {
		unsigned block = blk_rq_pos(req)*512;   //获取偏移值
		unsigned count = blk_rq_cur_sectors(req)*512;  //获取大小
		if(rq_data_dir(req) == READ){
			printk("read data--------\n");
            memcpy(req->buffer,ram_buf+block,count); //读数据
		}else{
			printk("write data++++++++\n");

            memcpy(ram_buf+block,req->buffer,count); //写数据
		}
		//结束请求
		if (!__blk_end_request_cur(req, 0))
			req = blk_fetch_request(q);
	}
}

static int __init ram_block_init(void)
{
	//1.分配gendisk结构体
	ramblk = alloc_disk(8);
	if(ramblk == NULL){
		printk("alloc gendisk memory fail\n");
		return -EINVAL;
	}
	
	//2.结构体的初始化
	//2.1申请块设备号
	major = register_blkdev(0,BLK_NAME);
	if(major <= 0){
		printk("get blk device num fail\n");
		return -EAGAIN;
	}
	
	//2.2申请队列
	spin_lock_init(&lock);
	q = blk_init_queue(request_ramblk_proc, &lock);
	if(q == NULL){
		printk("alloc blk queue fail\n");
		return -EAGAIN;
	}

	//2.3结构体的填充
	ramblk->major = major;
	ramblk->first_minor = 0;
	sprintf(ramblk->disk_name, BLK_NAME);
	ramblk->fops = &ramblk_fops;
	ramblk->queue = q;
	set_capacity(ramblk, BLK_SIZE/512); //设置容量

	//分配一块虚拟的内存空间
	ram_buf = vmalloc(BLK_SIZE);
	if(ram_buf == NULL){
		printk("alloc ram_buf memory fail\n");
		return -EINVAL;
	}
	//3.注册
	add_disk(ramblk);
	
	return 0;
}
static void __exit ram_block_exit(void)
{

	//1.清除队列
	blk_cleanup_queue(q);
	
	//2.注销设备号
	unregister_blkdev(major,BLK_NAME);
	//3.注销
	del_gendisk(ramblk);
	//4.释放内存
	vfree(ram_buf);

}

module_init(ram_block_init);
module_exit(ram_block_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("farsight");
