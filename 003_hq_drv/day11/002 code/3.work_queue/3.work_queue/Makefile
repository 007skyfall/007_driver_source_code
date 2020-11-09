#KERN:=/lib/modules/$(shell uname -r)/build
KERN:=/home/wangj/tftpboot/system/fs4412/linux-3.14


all:
	make -C $(KERN) M=$(shell pwd) modules
clean:
	make -C $(KERN) M=$(shell pwd) clean

obj-m:=demo.o


