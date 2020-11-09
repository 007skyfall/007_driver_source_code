#KERNELDIR:= /lib/modules/$(shell uname -r)/build/
KERNELDIR:= /home/ubuntu/itop/linux_4.14
PWD:=$(shell pwd)
all:
	make -C $(KERNELDIR) M=$(PWD) modules
	arm-none-linux-gnueabi-gcc -o test test.c
clean:
	make -C $(KERNELDIR) M=$(PWD) clean
	-@rm -rf test
obj-m:=mpu6050.o
install:
	cp *.ko  test  /mnt/share
