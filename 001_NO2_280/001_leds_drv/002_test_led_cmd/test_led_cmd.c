#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

//  ./test_led_cmd  <2|3> <0|1>

#define LED2 2
#define LED3 3

int g_val = 100;
int g_led2_fd,g_led3_fd;

int led_control(int * cmd)
{
	if (*cmd == 1)
	{
		g_val  = 1;
	}
	else if(*cmd == 0)
	{
		g_val = 0;
	}
	else
	{
		printf("invalid argc!\n");
		return -1;
	}
	
	printf("g_val = %d\n",g_val);
    
	return 0;
} 

int main(int argc, const char *argv[])
{
	int num = 0;
	int cmd = 0;
    int ret;
    
	if (argc != 3)
	{
		printf("Usage :\n");
		printf("%s <2|3> <0|1>\n", argv[0]);
		return 0;
	}
    
	num = atoi(argv[1]);
	cmd = atoi(argv[2]);
    
	g_led2_fd = open("/dev/led2", O_RDWR);
	if (g_led2_fd < 0)
	{
		printf("can't open led2! \n");
	}

	g_led3_fd = open("/dev/led3", O_RDWR);
	if (g_led3_fd < 0)
	{
		printf("can't open led3!\n");
	}
	
	for(;;)
	{
			switch (num)
			{
				case LED2:
				ret = led_control(&cmd);
		        if(0 == ret)
		        {
		    		write(g_led2_fd, &g_val, 4);
		    		break;
		        }

				case LED3:
		        ret = led_control(&cmd);
		        if(0 == ret)
		        {
		            write(g_led3_fd, &g_val, 4);
		            break;
		        }

				default:
				printf("the %s is invalid!\n",argv[1]);
				break;
			}
			
			sleep(1);
	}

	close(g_led2_fd);
	close(g_led3_fd);

	return 0;
}

