CC= arm-none-linux-gnueabi-gcc
STRIP=arm-none-linux-gnueabi-strip
OBJ=char_dev_test
OBJS=*.c 
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
	cp  $(OBJ)   /mnt/share
