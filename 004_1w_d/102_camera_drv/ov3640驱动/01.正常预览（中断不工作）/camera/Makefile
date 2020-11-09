KERNELDIR:= /home/linux/kernel/linux-3.14/
PWD := $(shell pwd)
all:
	make -C $(KERNELDIR) M=$(PWD) modules
	cp ov3640.ko ~/rootfs/
clean:
	make -C $(KERNELDIR) M=$(PWD) clean

obj-m:=ov3640.o

