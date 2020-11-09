KERNELDIR:= /lib/modules/$(shell uname -r)/build/
#KERNELDIR:= /home/ubuntu/itop/linux_3.0
PWD:=$(shell pwd)
all:
	make -C $(KERNELDIR) M=$(PWD) modules
	gcc test.c -o test -Wall
clean:
	make -C $(KERNELDIR) M=$(PWD) clean  
	-@rm test
obj-m:=my_ioctl.o

