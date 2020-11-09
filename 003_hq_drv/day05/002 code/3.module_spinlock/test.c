#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#define  N  128

int main(int argc, const char *argv[])
{
	int fd;
	int ret = 0;
	fd = open("/dev/demo", O_RDWR);
	if(fd < 0)
	{
		perror("Failed to open.");
		return -1;
	}
	else
	{
		printf("open success.\n");
	}

	getchar();



	close(fd);

	return 0;
}
