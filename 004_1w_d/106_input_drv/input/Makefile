export ARCH=arm
export CROSS_COMPILE:=arm-linux-gnueabi-
KERNELDIR:=/home/linux/fspad-733-5.0/lichee/linux-3.4/
PWD:=$(shell pwd)
all:
	make -C $(KERNELDIR) M=$(PWD) modules
clean:
	make -C $(KERNELDIR) M=$(PWD) clean

obj-m:=fspad_input.o

