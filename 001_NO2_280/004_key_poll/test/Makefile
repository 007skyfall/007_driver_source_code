CC= arm-none-linux-gnueabi-gcc
STRIP=arm-none-linux-gnueabi-strip
OBJ=key_poll_test
OBJS=key_poll_test.c 
CFLAGS= -Wall -g -static
$(OBJ):$(OBJS)
	$(CC) $(CFLAGS) $^ -o $@
	$(STRIP) $@
$*.o:$%.c
	$(CC) $(CFLAGS) -c $< -o $@ 
.PHONY:clean
clean:
	$(RM) *.o key_poll_test
install:
	cp $(OBJ) /mnt/share
