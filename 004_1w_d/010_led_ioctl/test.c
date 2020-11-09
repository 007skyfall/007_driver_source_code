#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdlib.h>
#include "myioctl.h"

#define NODE 		"/dev/leds"
#define UNIT_US		1000
#define ON 			1
#define OFF			0
int g_fd;

void led_blink(int cmd, int ms)
{
	ioctl(g_fd, cmd, ON);
	usleep(ms * UNIT_US);

	ioctl(g_fd, cmd, OFF);
	usleep(ms * UNIT_US);
	
	return ;
}

int main(int argc, const char *argv[])
{
	int ms = 0;
	if(argc != 2)
	{
		printf("Usage:\n");
		printf("%s ms\n", argv[0]);
		printf("rg: %s 10\n", argv[0]);
		exit(1);
	}
	ms = atoi(argv[1]);
	
	g_fd = open(NODE, O_RDWR);
	if(g_fd == -1){
		perror("open error");
		return -1;
	}
	
	while(1)
	{
		led_blink(LED2_OP, ms);
		led_blink(LED3_OP, ms);
	}
	
	close(g_fd);
	
	return 0;
}
