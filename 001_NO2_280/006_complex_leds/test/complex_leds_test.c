#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

/*
*  	./complex_leds_test <1|2|3> <on|off>
*/
#define LEDS 	"/dev/leds"
#define LED2 	"/dev/led2"
#define LED3 	"/dev/led3"

void print_usage(const char *file)
{
    printf("Usage:\n");
    printf("%s <1|2|3> <on>\n",file);
    printf("eg. \n");
    printf("%s  1		 on\n",  file);
    printf("%s  2		 on\n",  file);

	return ;
}

int main(int argc, const char *argv[])
{
    int fd;
    char filename[32] = { 0 };
    char val;
	int led_num;

    if (argc != 3)
    {
        print_usage(argv[0]);
        return 0;
    }
	led_num = atoi(argv[1]);
	switch(led_num)
		{
			case 1:
				strcpy(filename, LEDS);
				break;
			
			case 2:
				strcpy(filename, LED2);
				break;
			
			case 3:
				strcpy(filename, LED3);
				break;

			default:
				break;		
		}
	
    fd = open(filename, O_RDWR);
    if (fd < 0)
    {
        printf("error, can't open %s\n", filename);
        return -1;
    }

    if (0 == strcmp("on", argv[2]))
    {
        // 亮灯
        val = 0;
        write(fd, &val, 1);
    }
    else if (0 == strcmp("off", argv[2]))
    {
        // 灭灯
        val = 1;
        write(fd, &val, 1);
    }
    else
    {
        print_usage(argv[0]);
        return -1;
    }

	close(fd);
    
    return 0;
}

