$(warning KERNELRELEASE = $(KERNELRELEASE))

ifeq ($(KERNELRELEASE),)

#KERNELDIR ?= /lib/modules/$(shell uname -r)/build 
KERNELDIR ?= /home/linux/system/linux-3.14-fs4412/
PWD := $(shell pwd)

modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules


clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions modules* Module*

.PHONY: modules clean

else
    obj-m := fs4412_led.o
endif

