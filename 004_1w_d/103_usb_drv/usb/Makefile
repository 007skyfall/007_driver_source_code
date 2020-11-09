ifneq ($(KERNELRELEASE),)
	obj-m := usbmouse.o
else
	KERNELDIR := /home/linux/kernel/linux-3.14/
	PWD       := $(shell pwd)

modules:
	make -C $(KERNELDIR) M=$(PWD) modules
	cp *.ko ~/rootfs/
endif

clean:
	rm -rf *.o *.symvers *.order *.ko *.mod.c *.markers .tmp_versions .*.cmd
