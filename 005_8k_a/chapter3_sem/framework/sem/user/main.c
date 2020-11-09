/*
 * main.c : test semaphore driver
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>


int main(int argc, char *argv[])
{
	int i = 0;
	int dev_fd;
	char wr_buf[60];
	int cnt = 0;
	int input = 0;

	if(argc > 1)
	{
		input = atoi(argv[1]);
	}else{
		printf("usage: sem_test num\n");
		exit(1);
	}
	dev_fd = open("/dev/sem",O_RDWR);
	if ( dev_fd == -1 ) {
		printf("Cann't open file /dev/sem\n");
		exit(1);
	}
	memset(wr_buf, input, sizeof(wr_buf));
	/*print out 100 line input, there are 60 one every line*/
	while(1)
	{
		write(dev_fd,wr_buf,sizeof(wr_buf));
		cnt ++;
		if(cnt > 100)break;
		usleep(8);
	}
	close(dev_fd);
	return 0;
}
