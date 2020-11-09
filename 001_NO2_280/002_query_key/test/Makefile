CC= arm-none-linux-gnueabi-gcc
STRIP=arm-none-linux-gnueabi-strip
OBJ=query_key_test
OBJS=query_key_test.c 
CFLAGS= -Wall -g -static
$(OBJ):$(OBJS)
	$(CC) $(CFLAGS) $^ -o $@
	$(STRIP) $@
$*.o:$%.c
	$(CC) $(CFLAGS) -c $< -o $@ 
.PHONY:clean
clean:
	$(RM) *.o query_key_test
install:
	cp $(OBJ) /mnt/share
