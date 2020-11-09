
CC= /home/mint/itop/arm-2009q3/bin/arm-none-linux-gnueabi-gcc
OBJ=test_module_rw_update
OBJS=test_module_rw_update.c 
CFLAGS= -Wall -g -static
$(OBJ):$(OBJS)
	$(CC) $(CFLAGS) $^ -o $@
$*.o:$%.c
	$(CC) $(CFLAGS) -c $< -o $@ 
.PHONY:clean
clean:
	$(RM) *.o test_module_rw_update
