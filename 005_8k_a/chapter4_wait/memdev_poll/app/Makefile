KERNELDIR ?=/home/student/linux-2.6.32.2
all: memdev_poll_test

memdev_poll_test : memdev_poll_test.c
	arm-linux-gcc -I$(KERNELDIR) -o $@ $^

clean :
	rm memdev_poll_test
