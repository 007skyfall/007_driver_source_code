KERN_PATH:=/lib/modules/$(shell uname -r )/build

all:
	make -C $(KERN_PATH) M=$(shell pwd) modules 
clean:
	make -C $(KERN_PATH) M=$(shell pwd)  clean
obj-m:=demo.o

