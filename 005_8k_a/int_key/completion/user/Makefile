KERNELDIR ?=/opt/linux-2.6.34/include
all: key_test 

key_test : key_test.c
	#arm-linux-gcc -I$(KERNELDIR) -s -Wl,-warn-common --static -o $@ $^
	arm-linux-gcc -I$(KERNELDIR) -o $@ $^
clean :
	rm key_test
