KERNELDIR ?=/home/student/linux-2.6.32.2
all: memdev_write memdev_read

memdev_read : memdev_read.c
	arm-linux-gcc -I$(KERNELDIR) -o $@ $^

memdev_write : memdev_write.c
	arm-linux-gcc -I$(KERNELDIR) -o $@ $^

clean :
	rm memdev_read memdev_write
