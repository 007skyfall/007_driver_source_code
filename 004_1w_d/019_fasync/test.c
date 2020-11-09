#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/select.h>
#include <signal.h>

#include <string.h>
char buf[128] = {0};
int fd;

void handle_sighandler_f(int signo)
{
	if(signo == SIGIO){
		memset(buf,0,sizeof(buf));
		read(fd,buf,sizeof(buf));
		printf("data = %s\n",buf);
	}

}

int main(int argc, const char *argv[])
{
	
	fd = open("hello",O_RDWR);
	if(fd == -1){
		perror("open file error");
		return -1;
	}
	
	signal(SIGIO,handle_sighandler_f);
	
	fcntl(fd,F_SETFL,fcntl(fd,F_GETFL)|FASYNC);
	
	fcntl(fd,F_SETOWN,getpid());

	while(1);
	
	return 0;
}
