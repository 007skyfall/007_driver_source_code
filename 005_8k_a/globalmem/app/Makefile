KERNELDIR ?=/home/student/linux-2.6.32.2

all: globalmem_test 

globalmem_test : globalmem_test.c
	#arm-linux-gcc -I$(KERNELDIR) -s -Wl,-warn-common --static -o $@ $^
	arm-linux-gcc -I$(KERNELDIR) -o $@ $^

clean :
	rm globalmem_test
