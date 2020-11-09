#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/select.h>
#include <string.h>

char buf[128] = {0};

int main(int argc, const char *argv[])
{
	int i,ret;
	int fd,max_fd;
	fd_set rfds;

	for(i=1; i<argc; i++){
		fd = open(argv[i],O_RDWR);
		if(fd == -1){
			perror("open file error");
			return -1;
		}
	}
	max_fd = fd;
	while(1){
		FD_ZERO(&rfds);
		for(i=3;i<=max_fd;i++){
			FD_SET(i,&rfds);
		}
		ret = select(max_fd+1,&rfds,NULL,NULL,NULL);
		if(ret == -1){
			perror("select");
			return -1;
		}
		for(i=3; i<=max_fd;i++){
			if(FD_ISSET(i,&rfds)){
				memset(buf,0,sizeof(buf));
				read(i,buf,sizeof(buf));
				printf("fd = %d,data = %s\n",i,buf);
			}
		}
	}
	return 0;
}

