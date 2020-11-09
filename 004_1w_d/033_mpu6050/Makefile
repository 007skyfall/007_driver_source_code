#KERNELDIR:= /lib/modules/$(shell uname -r)/build/
KERNELDIR:= /home/ubuntu/itop/linux_4.14
PWD:=$(shell pwd)
all:
	make -C $(KERNELDIR) M=$(PWD) modules
clean:
	make -C $(KERNELDIR) M=$(PWD) clean
obj-m:=mpu6050.o
install:
	cp *.ko /mnt/share
