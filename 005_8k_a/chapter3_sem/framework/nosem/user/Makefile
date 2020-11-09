KERNELDIR ?=/opt/linux-2.6.34/include
all: nosem_test

nosem_test : main.c
	#arm-linux-gcc -I$(KERNELDIR) -s -Wl,-warn-common --static -o $@ $^
	arm-linux-gcc -I$(KERNELDIR) -o $@ $^

clean :
	rm nosem_test 
