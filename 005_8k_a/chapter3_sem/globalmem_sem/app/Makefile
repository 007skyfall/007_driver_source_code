#KERNELDIR ?=/opt/linux-2.6.34
KERNELDIR ?=/lib/modules/$(shell uname -r)/build
all: memdev_test 

memdev_test : memdev_test.c
	#arm-linux-gcc -I$(KERNELDIR) -s -Wl,-warn-common --static -o $@ $^
	gcc -I$(KERNELDIR) -o $@ $^

clean :
	rm memdev_test
