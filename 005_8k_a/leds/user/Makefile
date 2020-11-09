KERNELDIR ?=/home/student/linux-2.6.32.2
all: leds_test 

leds_test : leds_test.c
	arm-linux-gcc -I$(KERNELDIR) -o $@ $^

clean :
	rm leds_test
