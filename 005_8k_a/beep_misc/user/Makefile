KERNELDIR ?=/opt/linux-2.6.34
all: beep_test 

beep_test : main.c
	#arm-linux-gcc -I$(KERNELDIR) -s -Wl,-warn-common --static -o $@ $^
	arm-linux-gcc -I$(KERNELDIR) -o $@ $^

clean :
	rm beep_test
