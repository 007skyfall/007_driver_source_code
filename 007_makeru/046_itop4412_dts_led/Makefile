$(warning KERNELRELEASE = $(KERNELRELEASE))

ifeq ($(KERNELRELEASE),)

#内核的源码路径, ?= 条件赋值, uname -r 得到内核版本号
#KERNELDIR ?= /lib/modules/$(shell uname -r)/build  
KERNELDIR ?= /home/mint/itop/linux_4.14.02

# := 立即赋值, 得到当前的绝对路径
PWD := $(shell pwd)


# -C 切换工作路径, $(MAKE) =  make
modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko .cache.mk *.mod.c .tmp_versions Module* modules*

.PHONY: modules clean

else
	  # 生成模块
       obj-m := itop4412_dts_led.o
	
endif

