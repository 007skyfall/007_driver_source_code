#include <linux/module.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/timer.h>
#include <linux/genhd.h>
#include <linux/hdreg.h>
#include <linux/ioport.h>
#include <linux/init.h>
#include <linux/wait.h>
#include <linux/blkdev.h>
#include <linux/mutex.h>
#include <linux/blkpg.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/gfp.h>
#include <linux/vmalloc.h>
#include <asm/uaccess.h>
#include <asm/dma.h>



#define BLK_NAME "ramblock"
#define RAM_SIZE (2*1024*1024)
//1.分配gendisk结构体
static struct gendisk *ramdisk;
static struct request_queue *ram_q;
char * buf;
int major;
spinlock_t lock;

int ramblock_open(struct block_device *blk_dev, fmode_t mod)
{
	printk("%s:%s:%d\n",__FILE__,__func__,__LINE__);
	return 0;
}
int ramblock_release (struct gendisk *disk, fmode_t mod)
{
	printk("%s:%s:%d\n",__FILE__,__func__,__LINE__);
	return 0;

}
int ramblock_getgeo(struct block_device *blk_dev, struct hd_geometry *hd)
{
	hd->heads = 4; //4
	hd->cylinders = 20;
	hd->sectors = RAM_SIZE/4/20/512;
	printk("%s:%s:%d\n",__FILE__,__func__,__LINE__);
	return 0;
}

//队列处理函数
void handle_request_fn_proc(struct request_queue *q)
{
	struct request *req;
	req = blk_fetch_request(q);
	while (req) {
		unsigned block = blk_rq_pos(req)*512;
		unsigned count = blk_rq_cur_sectors(req)*512;
		if(rq_data_dir(req) == READ){
			printk("read data ++++++++\n");
			memcpy(req->buffer,buf+block,count);
		}else{
			printk("write data ++++++++\n");
			memcpy(buf+block,req->buffer,count);
		}
		
		if (!__blk_end_request_cur(req,0))
		req = blk_fetch_request(q);

	}
}
static struct block_device_operations fops = {
	.open = ramblock_open,
	.getgeo = ramblock_getgeo,
	.release = ramblock_release,
};

static int __init demo_init(void)
{
	//2.为结构体分配空间
	ramdisk = alloc_disk(8);
	if(ramdisk == NULL){
		printk("alloc memory fail\n");
		return -ENOMEM;
	}

	//3.分配设备号
	major = register_blkdev(0,BLK_NAME);
	if(major < 0){
		printk("register blk num fail\n");
		return -EAGAIN;
	}

	//4.分配队列
	spin_lock_init(&lock);
	ram_q = blk_init_queue(handle_request_fn_proc,&lock);
	if (!ram_q){
		printk("blk_init_queue fail\n");
		return -EAGAIN;
	}

	//5.结构体成员的初始化
	ramdisk->major = major;
	ramdisk->first_minor = 0;
	sprintf(ramdisk->disk_name, BLK_NAME);
	ramdisk->fops = &fops;
	ramdisk->queue = ram_q;
	set_capacity(ramdisk, RAM_SIZE/512);

	//分配虚拟的内存
	buf = vmalloc(RAM_SIZE);
	if(buf == NULL){
		printk("alloc memory fail\n");
		return -ENOMEM;
	}
	
	//6.注册
	add_disk(ramdisk);
	return 0;
}
static void __exit demo_exit(void)
{
	//注销
	blk_cleanup_queue(ram_q);
	unregister_blkdev(major, BLK_NAME);
	put_disk(ramdisk);
	vfree(buf);
}
module_init(demo_init);
module_exit(demo_exit);
MODULE_LICENSE("GPL");


