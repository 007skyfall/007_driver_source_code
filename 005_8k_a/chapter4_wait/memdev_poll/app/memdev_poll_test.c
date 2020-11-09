/*********************************************************************************************
#####         上海嵌入式家园-开发板商城         #####
#####                    www.embedclub.com                        #####
#####             http://embedclub.taobao.com               #####

* File：	memdev_test.c
* Author:	Hanson
* Desc： A test program in userspace   
    This example is to introduce the ways to use "select"
     and driver poll
* History:	May 16th 2011
*********************************************************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>

main()
{
  int fd, num;
  char Buf[128]="memdev_poll test!";
  fd_set rfds,wfds;
  
  /*以非阻塞方式打开/dev/memdev0设备文件*/
  fd = open("/dev/memdev0", O_RDWR | O_NONBLOCK);
  if (fd !=  - 1)
  {
	while (1)
    	{
      		FD_ZERO(&rfds);
      		FD_ZERO(&wfds);
      		FD_SET(fd, &rfds);
      		FD_SET(fd, &wfds);

      		select(fd + 1, &rfds, &wfds, NULL, NULL);
      		/*数据可获得*/
      		if (FD_ISSET(fd, &rfds))
      		{
      			printf("Poll monitor:can be read\n");
			lseek(fd, 0, SEEK_SET);
			read(fd, Buf, sizeof(Buf));
			printf("Read Buf: %s\n", Buf);
			sleep(1);
      		}
      		/*数据可写入*/
      		if (FD_ISSET(fd, &wfds))
      		{
      			printf("Poll monitor:can be written\n");
			lseek(fd, 0, SEEK_SET);
			write(fd, Buf, sizeof(Buf));
			printf("Write Buf: %s\n", Buf);
			sleep(1);
      		}      
   	}
  }
  else
  {
    printf("Device open failure\n");
  }
}
