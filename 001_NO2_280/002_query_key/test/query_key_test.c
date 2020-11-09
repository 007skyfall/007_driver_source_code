
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

/* 
*   ./query_key_test 
*/

int main(int argc, const char *argv[])
{
	int fd, n;
	unsigned int key_val;
    unsigned int key_vals[5] = { 0 };
	int cnt = 0;

    n = sizeof(key_vals) / sizeof(key_vals[0]); 
    
	fd = open("/dev/button", O_RDWR);
	if (fd < 0)
	{
		printf("can't open!\n");
		return -1;
	}

	while (1)
	{
		read(fd, key_vals, sizeof(key_vals));
        
        if(!key_vals[0] || !key_vals[1] || !key_vals[2] || !key_vals[3] || !key_vals[4])
        {
            printf("%04d key pressed: %d:%d:%d:%d:%d \n", cnt++, key_vals[0], key_vals[1], key_vals[2], key_vals[3], key_vals[4]);
        }
        
//		read(fd, &key_val, 4);
//		if (!key_val)
//		{
//			printf("%04d key pressed: %d\n", cnt++, key_val);
//		}
//		sleep(1);
	}
	
	return 0;
}

