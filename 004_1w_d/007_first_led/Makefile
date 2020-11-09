#KERNELDIR:= /lib/modules/$(shell uname -r)/build/
KERNELDIR:= /home/mint/itop/linux_3.0
PWD:=$(shell pwd)
all:
	make -C $(KERNELDIR) M=$(PWD) modules
	/home/mint/itop/arm-2009q3/bin/arm-none-linux-gnueabi-gcc test.c -o test
clean:
	make -C $(KERNELDIR) M=$(PWD) clean
	-@rm -r test
obj-m:=myled.o

