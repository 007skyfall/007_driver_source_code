CC= gcc
STRIP= strip
OBJ=test_bit
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
	cp $(OBJ) /mnt/share
