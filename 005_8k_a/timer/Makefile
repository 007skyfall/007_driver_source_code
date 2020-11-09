ifeq ($(KERNELRELEASE),)

KERNELDIR ?=/lib/modules/$(shell uname -r)/build
#KERNELDIR ?=/usr/src/kernels/2.6.18-164.el5-xen-i686
PWD := $(shell pwd)

modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

modules_install:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules_install

clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.order *.markers *.mod.c *.mod.o *.symvers .tmp_versions

.PHONY: modules modules_install clean

else
    obj-m := timer.o
endif