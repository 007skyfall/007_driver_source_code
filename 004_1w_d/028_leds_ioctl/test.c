#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdlib.h>
#include "myioctl.h"

#define LED2 		"/dev/leds_drv0"
#define LED3		"/dev/leds_drv1"

#define UNIT_US		1000
#define ON 			1
#define OFF			0
int led2_fd, led3_fd;

void led_blink(int cmd, int arg)
{
	ioctl(led2_fd, cmd, arg);
	usleep(200 * UNIT_US);

	ioctl(led2_fd, cmd, !arg);
	usleep(200* UNIT_US);
	
	return ;
}

int main(int argc, const char *argv[])
{
	int led_status = 0;
	if(argc != 2)
	{
		printf("Usage:\n");
		printf("%s cmd\n", argv[0]);
		printf("eg: %s <0|1>\n", argv[0]);
		exit(1);
	}
	led_status = atoi(argv[1]);
	
	led2_fd = open(LED2, O_RDWR);
	if(led2_fd == -1){
		perror("open led2_fd error");
		return -1;
	}

	led3_fd = open(LED3, O_RDWR);
	if(led3_fd == -1){
		perror("open led3_fd error");
		return -1;
	}
	
	while(1)
	{
		led_blink(LED2_OP, led_status);
		led_blink(LED3_OP, led_status);

	}
	
	close(led2_fd);
	close(led3_fd);
	
	return 0;
}

