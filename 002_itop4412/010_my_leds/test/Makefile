CC= /home/mint/itop/arm-2009q3/bin/arm-none-linux-gnueabi-gcc
STRIP=/home/mint/itop/arm-2009q3/bin/arm-none-linux-gnueabi-strip
OBJ=test_my_led
OBJS=test_my_led.c 
CFLAGS= -Wall -g -static
$(OBJ):$(OBJS)
	$(CC) $(CFLAGS) $^ -o $@
	$(STRIP) $@
$*.o:$%.c
	$(CC) $(CFLAGS) -c $< -o $@ 
.PHONY:clean
clean:
	$(RM) *.o test_my_led
