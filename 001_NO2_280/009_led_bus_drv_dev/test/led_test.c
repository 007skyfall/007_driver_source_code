
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define LED2 "/dev/led2"
#define LED3 "/dev/led3"

/*
*   ./led_test <2|3> <on|off> 
 */
int led_ctl(char * led_status, int *val)
{
    if (strcmp(led_status, "on") == 0)
	{
		*val  = 1;
	}
	else if(strcmp(led_status, "off") == 0)
	{
		*val = 0;
	}
    else
    {
        return -1;
    }

    return 0;
}
 
int main(int argc, char **argv)
{
	int led2_fd, led3_fd;
	int val = -1;
    int led_num = 0;
    char led_status[32] = { 0 };

    if (argc != 3)
	{
		printf("Usage :\n");
		printf("%s <2|3> <on|off>\n", argv[0]);
		return -1;
	}
    
    led_num = atoi(argv[1]);   
    strcpy(led_status,argv[2]);


    led2_fd = open(LED2, O_RDWR);
	if (led2_fd < 0)
	{
		printf("can't open led2!\n");
        return -1;
	}

    led3_fd = open(LED3, O_RDWR);
	if (led3_fd < 0)
	{
		printf("can't open led3!\n");
        return -1;
	}

    switch (led_num)
    {
        case 2:
            led_ctl(led_status, &val);
            write(led2_fd, &val, 4);
            sleep(3);
            break;

        case 3:
            led_ctl(led_status, &val);
            write(led3_fd, &val, 4);
            sleep(3);
            break;
        
        default:
            printf("invalid args!\n");
            return -1;
            break;
    }

    close(led2_fd);
    close(led3_fd);
    
    return 0;
}
