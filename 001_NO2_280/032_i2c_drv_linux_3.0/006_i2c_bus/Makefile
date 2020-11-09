KERN_DIR = /home/ubuntu/itop/linux_3.0

all:
	make -C $(KERN_DIR) M=`pwd` modules 

clean:
	make -C $(KERN_DIR) M=`pwd` modules clean
	rm -rf modules.order

#obj-m	+= at24cxx_dev.o
#obj-m	+= at24cxx_drv.o
obj-m	+= itop4412_i2c_bus.o
install:
	cp *.ko /mnt/share
