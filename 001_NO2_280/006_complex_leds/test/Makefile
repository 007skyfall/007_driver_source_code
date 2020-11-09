CC= arm-none-linux-gnueabi-gcc
STRIP=arm-none-linux-gnueabi-strip
OBJ=complex_leds_test
OBJS=complex_leds_test.c 
CFLAGS= -Wall -g -static
$(OBJ):$(OBJS)
	$(CC) $(CFLAGS) $^ -o $@
	$(STRIP) $@
$*.o:$%.c
	$(CC) $(CFLAGS) -c $< -o $@ 
.PHONY:clean
clean:
	$(RM) *.o $(OBJ)
install:
	cp $(OBJ) /mnt/share
