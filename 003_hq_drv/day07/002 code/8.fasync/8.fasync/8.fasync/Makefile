KERN:=/lib/modules/$(shell uname -r)/build

all:
	make -C $(KERN) M=$(shell pwd) modules
clean:
	make -C $(KERN) M=$(shell pwd) clean

obj-m:=demo.o


