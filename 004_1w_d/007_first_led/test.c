#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

char buf[128] = {1,0};

int main(int argc, const char *argv[])
{
	int fd;
	fd = open("/dev/myled",O_RDWR);
	if(fd == -1){
		perror("open /dev/mycdev error");
		return -1;
	}
	while(1){
		buf[1] = buf[1]?0:1;
		write(fd,buf,sizeof(buf));
		sleep(1);
	}
	close(fd);
	return 0;
}
