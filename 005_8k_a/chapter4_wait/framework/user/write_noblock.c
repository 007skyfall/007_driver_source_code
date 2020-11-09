/*
 * test demo driver
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/ioctl.h>
#include <string.h>

#define BEEP_MAGIC 'k'
#define BEEP_START_CMD _IO (BEEP_MAGIC, 1)
#define BEEP_STOP_CMD _IO (BEEP_MAGIC, 2)


int main()
{
	int i = 0;
	int dev_fd;
	char wrbf[2];
	
	memset(wrbf,0x1,2);
	
	//dev_fd = open("/dev/beep",O_RDWR | O_NONBLOCK);
	dev_fd = open("/dev/beep",O_RDWR);
	
	if ( dev_fd == -1 ) {
		printf("Cann't open file /dev/beep\n");
		exit(1);
	}
	//wakeup the blocked process
	printf("write to wakeup the blocked process\n");
	write(dev_fd,wrbf,2);

	close(dev_fd);
	
	return 0;
}
