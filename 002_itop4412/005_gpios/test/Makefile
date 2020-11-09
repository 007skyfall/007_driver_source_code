src = $(wildcard *.c)
targets = $(patsubst %.c, %, $(src))

CC = /home/mint/itop/arm-2009q3/bin/arm-none-linux-gnueabi-gcc 
STRIP= /home/mint/itop/arm-2009q3/bin/arm-none-linux-gnueabi-strip
CFLAGS = -Wall -g 

all:$(targets)

$(targets):%:%.c
	$(CC) $< -o $@ $(CFLAGS)
	$(STRIP) $@

.PHONY:clean all
clean:
	-rm -rf $(targets) 
