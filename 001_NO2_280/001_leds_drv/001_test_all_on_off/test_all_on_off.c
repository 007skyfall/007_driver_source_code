#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>


/*  ./test_all_on_off  on
*   ./test_all_on_off off
*/
int main(int argc, const char *argv[])
{
	int led2_fd, led3_fd;
	int val = 1;
	
	if (argc != 2)
	{
		printf("Usage :\n");
		printf("%s <on|off>\n", argv[0]);
		return 0;
	}

    led2_fd = open("/dev/led2", O_RDWR);
	if (led2_fd < 0)
	{
		printf("can't open led2! \n");
        exit(1);
	}

	led3_fd = open("/dev/led3", O_RDWR);
	if (led3_fd < 0)
	{
		printf("can't open led3!\n");
        exit(1);
	}

	if (strcmp(argv[1], "on") == 0)
	{
		val = 1;
	}
	else
	{
		val = 0;
	}
	
	write(led2_fd, &val, 4);
	write(led3_fd, &val, 4);
	sleep(3);
	
	close(led2_fd);
	close(led3_fd);

	return 0;
}
