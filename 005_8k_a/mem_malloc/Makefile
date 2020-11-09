ifeq ($(KERNELRELEASE),)

KERNELDIR ?=/home/student/linux-2.6.32.2
PWD := $(shell pwd)

modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

modules_install:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules_install

clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.markers *.mod.c *.mod.o *.symvers .tmp_versions

.PHONY: modules modules_install clean

else
    obj-m := mem_malloc.o
endif

