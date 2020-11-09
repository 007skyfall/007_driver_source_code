KERNELDIR:= /lib/modules/$(shell uname -r)/build/
#KERNELDIR:= /home/mint/itop/linux_3.0
PWD:=$(shell pwd)
all:
	make -C $(KERNELDIR) M=$(PWD) modules
clean:
	make -C $(KERNELDIR) M=$(PWD) clean
obj-m:=used_symbol.o

