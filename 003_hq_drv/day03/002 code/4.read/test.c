#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
 #include <unistd.h>

#define PATH "/dev/demo"

int main(int argc, const char *argv[])
{
	int fd ;
	char use_buf[64] = {0};
	fd = open(PATH,O_RDWR);
	if(fd < 0){
		perror("open");
	}
	if(read(fd,use_buf,64)){
		perror("read");
	}
	printf("use_buf:%s\n",use_buf);
	close(fd);
	return 0;
}
