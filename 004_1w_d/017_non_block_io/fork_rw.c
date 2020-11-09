#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define NODE "/dev/non_block_io"

char buf[128] = "i am child process.....";

int main(int argc, const char *argv[])
{
	int fd;
	pid_t pid;
	fd = open(NODE, O_RDWR);
	if(fd == -1){
		perror("open error");
		return -1;
	}

	pid = fork();
	if(pid < 0){
		perror("fork error");
		return -1;
	}else if(pid == 0){
		while(1){
			sleep(2);
			write(fd,buf,sizeof(buf));
		}
	}else{
		while(1){
			memset(buf,0,sizeof(buf));
			read(fd,buf,sizeof(buf));
			printf("user:data = %s\n",buf);
		}
	}

	close(fd);
    
	return 0;
}

