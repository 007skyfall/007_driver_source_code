KERNELDIR ?=/home/student/linux-2.6.32.2/include
all: sem_test

sem_test : main.c
	#arm-linux-gcc -I$(KERNELDIR) -s -Wl,-warn-common --static -o $@ $^
	arm-linux-gcc -I$(KERNELDIR) -o $@ $^

clean :
	rm sem_test
