KERN_DIR = /home/ubuntu/itop/linux_3.0

all:
	make -C $(KERN_DIR) M=`pwd` modules 
	arm-none-linux-gnueabi-gcc i2c_test.c -o i2c_test
clean:
	make -C $(KERN_DIR) M=`pwd` modules clean
	rm -rf modules.order
	rm -rf i2c_test

obj-m	+= itop_i2c_dev.o
obj-m	+= itop_i2c_drv.o
install:
	cp *.ko  i2c_test /mnt/share
