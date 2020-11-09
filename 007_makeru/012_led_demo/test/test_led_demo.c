#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, const char *argv[])
{
	int fd;
	fd = open("/dev/test_led_demo" , O_RDWR);
	if(fd < 0)
	{
		perror("open");
		return fd;
	}

	sleep(10);
	close(fd);

return 0;
}
