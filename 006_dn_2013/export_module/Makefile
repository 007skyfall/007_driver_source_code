#!/bin/bash
$(warning KERNELRELEASE = $(KERNELRELEASE))

ifeq ($(KERNELRELEASE),)

ifeq ($(PLATFORM),PC)

#内核的源码路径, ?= 条件赋值, uname -r 得到内核版本号
KERNELDIR ?=  /lib/modules/$(shell uname -r)/build	
else
KERNELDIR ?=  /home/mint/itop/linux_3.0
endif
# := 立即赋值, 得到当前的绝对路径
PWD := $(shell pwd)


# -C 切换工作路径, $(MAKE) =  make
modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions Module* modules*

.PHONY: modules clean

else
	  # 生成模块
       obj-m := export_module.o 
	
endif

