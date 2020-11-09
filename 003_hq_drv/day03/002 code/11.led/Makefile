#KERN_PATH:=/lib/modules/$(shell uname -r )/build
KERN_PATH:=/home/wangj/tftpboot/system/fs4412/linux-3.14

all:
	make -C $(KERN_PATH) M=$(shell pwd) modules 
clean:
	make -C $(KERN_PATH) M=$(shell pwd)  clean
obj-m:=demo.o

