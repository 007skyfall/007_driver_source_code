CC=/home/mint/itop/gcc-4.6.2/fsl-linaro-toolchain/bin/arm-none-linux-gnueabi-gcc 
OBJ=test_mpu6050
OBJS=test_mpu6050.c 
CFLAGS= -Wall -g -static 
$(OBJ):$(OBJS)
	$(CC) $(CFLAGS) $^ -o $@
$*.o:$%.c
	$(CC) $(CFLAGS) -c $< -o $@ 
.PHONY:clean
clean:
	$(RM) *.o test_mpu6050
