CC= /home/mint/itop/arm-2009q3/bin/arm-none-linux-gnueabi-gcc
STRIP=/home/mint/itop/arm-2009q3/bin/arm-none-linux-gnueabi-strip
OBJ=test_itop4412_pwm
OBJS=test_itop4412_pwm.c 
CFLAGS= -Wall -g -static 
$(OBJ):$(OBJS)
	$(CC) $(CFLAGS) $^ -o $@
	$(STRIP) $@
$*.o:$%.c
	$(CC) $(CFLAGS) -c $< -o $@ 
.PHONY:clean
clean:
	$(RM) *.o test_itop4412_pwm
