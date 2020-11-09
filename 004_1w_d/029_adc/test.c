#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

int main(int argc, const char *argv[])
{
	int buf;
	int fd = open("/dev/adc",O_RDWR);
	if(fd < 0)
    {
		perror("open");
	}

    while(1)
    {
		if(read(fd ,&buf, sizeof(buf)) < 0)
        {
			perror("read");
		}

		printf("adcdat :%f V\n",buf*1.8/4095);
	}
    
	return 0;
}

