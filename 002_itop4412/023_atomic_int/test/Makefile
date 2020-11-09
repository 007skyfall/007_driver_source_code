CC= /home/mint/itop/arm-2009q3/bin/arm-none-linux-gnueabi-gcc
OBJ=open_atomic_int_one
OBJS=open_atomic_int_one.c 
CFLAGS= -Wall -g -static
$(OBJ):$(OBJS)
	$(CC) $(CFLAGS) $^ -o $@
$*.o:$%.c
	$(CC) $(CFLAGS) -c $< -o $@ 
.PHONY:clean
clean:
	$(RM) *.o open_atomic_int_one  open_atomic_int_two

.PHONY:two
two:
	cp open_atomic_int_one open_atomic_int_two