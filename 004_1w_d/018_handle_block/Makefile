#KERNELDIR:= /lib/modules/$(shell uname -r)/build/
KERNELDIR:= /home/ubuntu/itop/linux_3.0
PWD:=$(shell pwd)
all:
	make -C $(KERNELDIR) M=$(PWD) modules
	gcc test.c -o test -Wall
	gcc fork_rw.c -o fork_rw -Wall
clean:
	make -C $(KERNELDIR) M=$(PWD) clean  
	-@rm test fork_rw
obj-m:=handle_block.o

