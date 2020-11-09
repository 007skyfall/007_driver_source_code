#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define PATH "/dev/demo"

int main(int argc, const char *argv[])
{
	
	int fd ;
	fd = open(PATH,O_RDWR);
	if(fd < 0){
		perror("open");
	}

	sleep(10);

	close(fd);
	return 0;
}

