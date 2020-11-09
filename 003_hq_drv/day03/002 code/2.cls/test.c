#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


int main(int argc, const char *argv[])
{
	int fd;
	fd = open("/dev/demo" , O_RDWR);
	if(fd < 0){
		perror("open");
	}

	close(fd);
	return 0;
}
