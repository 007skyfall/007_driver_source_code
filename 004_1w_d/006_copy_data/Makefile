KERNELDIR:= /lib/modules/$(shell uname -r)/build/
#KERNELDIR:= /home/mint/itop/linux_3.0
PWD:=$(shell pwd)
all:
	make -C $(KERNELDIR) M=$(PWD) modules
	gcc test.c -o test
clean:
	make -C $(KERNELDIR) M=$(PWD) clean
	-@rm -rf test
obj-m:=mycdev.o

