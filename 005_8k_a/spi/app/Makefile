CROSS=arm-linux-

all: spi_test
.PHONY:  spi_test 

spi_test: spi_test.c
	$(CROSS)gcc -o spi_test spi_test.c

clean:
	rm -rf   *.o *~