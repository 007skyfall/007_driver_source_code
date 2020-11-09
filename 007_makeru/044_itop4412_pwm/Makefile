$(warning KERNELRELEASE = $(KERNELRELEASE))

ifeq ($(KERNELRELEASE),)

KERNELDIR ?= /home/mint/itop/linux_4.14.02
PWD := $(shell pwd)

modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules


clean:
	rm -rf *.o *~ core .depend .*.cmd .cache.mk *.ko *.mod.c .tmp_versions modules* Module*

.PHONY: modules clean

else
    obj-m := itop4412_pwm.o
endif

