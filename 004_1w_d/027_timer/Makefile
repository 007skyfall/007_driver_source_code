#KERNELDIR:= /lib/modules/$(shell uname -r)/build/
KERNELDIR:= /home/ubuntu/itop/linux_4.14
#KERNELDIR:= /home/ubuntu/Hi3519V101_SDK_V1.0.4.0/osdrv/opensource/kernel/linux-3.18.y
PWD:=$(shell pwd)
all:
	make -C $(KERNELDIR) M=$(PWD) modules
clean:
	make -C $(KERNELDIR) M=$(PWD) clean
obj-m:=timer_gpio.o
install:
	cp *.ko /mnt/share
