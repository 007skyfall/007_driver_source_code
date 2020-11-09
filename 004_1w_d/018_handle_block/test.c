#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define NODE "/dev/non_block_io"

char buf[128] = {0};

int main(int argc, const char *argv[])
{
	int fd;
	fd = open(NODE, O_RDWR);
	if(fd == -1){
		perror("open error");
		return -1;
	}
	while(1){
		read(fd,buf,sizeof(buf));
		printf("user:data = %s\n",buf);
	}
    
	close(fd);

    return 0;
}
