
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>


#define LED2    "/dev/led2"
#define LED3    "/dev/led3"
#define HOME    "/dev/button1"
#define BACK    "/dev/button2"
#define UP      "/dev/button3"
#define DOWN    "/dev/button4"


/*
* ./interrupt_key_led_test <1|2|3|4>
*/
/*
*   home /dev/button1
*   back /dev/button2
*   up   /dev/button3
*   down /dev/button4
*/
void perr_exit(const char *s)
{
    perror(s);
    exit(-1);
}

int main(int argc, const char *argv[])
{
	int key_led_arry_fd[4] = { 0 };
    
	char  key_val;
/*
*   led_cmd=1 LED2===ON
*   led_cmd=2 LED2===OFF 
*   led_cmd=3 LED3===ON
*   led_cmd=4 LED3===OFF 
*/

    int cmd = 0;
	if(argc != 2)
	{
        printf("Usage:\n");
        printf(" %s <1|2|3|4> \n", argv[0]);
        exit(1);
    }
    cmd = atoi(argv[1]);
 
	key_led_arry_fd[0] = open(HOME, O_RDWR);
	if (key_led_arry_fd[0] < 0)
        perr_exit("home button open error");
    
	key_led_arry_fd[1] = open(BACK, O_RDWR);
	if (key_led_arry_fd[1] < 0)
        perr_exit("back button open error");
    
	key_led_arry_fd[2] = open(UP, O_RDWR);
	if (key_led_arry_fd[2] < 0)
        perr_exit("up button open error");
    
	key_led_arry_fd[3] = open(DOWN, O_RDWR);
	if (key_led_arry_fd[3] < 0)
        perr_exit("down button open error");

for(;;){
            switch(cmd)
            {
                case 1:
                         read(key_led_arry_fd[0], &key_val, 1);
                         printf("cmd = %d\n",cmd);
                         break;
                case 2:
                        read(key_led_arry_fd[1], &key_val, 1);
                        printf("cmd = %d\n",cmd);
                        break;
                
                case 3:
                         read(key_led_arry_fd[2], &key_val, 1);
                         printf("cmd = %d\n",cmd);
                         break;
                case 4:
                         read(key_led_arry_fd[3], &key_val, 1);
                         printf("cmd = %d\n",cmd);
                         break;
                
                default:
                         printf("invalid value!\n");
                         break;

            }
        } 

    close(key_led_arry_fd[0]);
    close(key_led_arry_fd[1]);
    close(key_led_arry_fd[2]);
    close(key_led_arry_fd[3]);
    
	return 0;
}

