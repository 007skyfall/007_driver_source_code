/*********************************************************************************************
#####         上海嵌入式家园-开发板商城         #####
#####                    www.embedclub.com                        #####
#####             http://embedclub.taobao.com               #####

* File：	main.c
* Author:	Hanson
* Desc：	main.c : test no semaphore driver
* History:	May 16th 2011
*********************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

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
		printf("usage: nosem_test number\n");
		exit(1); 
	}
	dev_fd = open("/dev/nosem",O_RDWR);
	if ( dev_fd == -1 ) {
		printf("Can't open file /dev/nosem\n");
		exit(1);
	}
	memset(wr_buf, input, sizeof(wr_buf));
	/*print out 100 line number, there are 60 one every line*/
	while(1)
	{
		lseek(dev_fd,0,SEEK_SET);
		write(dev_fd,wr_buf,sizeof(wr_buf));		
		cnt ++;
		if(cnt > 100)break;
		usleep(8);
	}
	close(dev_fd);
	return 0;
}
