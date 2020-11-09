#include <stdio.h>
#include <string.h>

int main()
{
	FILE *fp0 = NULL;
	char Buf[128];
	
	/*初始化Buf*/
	strcpy(Buf,"memdev_sem test!");
	
	/*打开设备文件*/
	fp0 = fopen("/dev/memdev0","r+");
	if (fp0 == NULL)
	{
		perror("Open Memdev0 Error!\n");
		return -1;
	}
	
	while (1) {
		/*写入设备*/
		fwrite(Buf, sizeof(Buf), 1, fp0);
	
		/*重新定位文件位置（思考没有该指令，会有何后果)*/
		fseek(fp0,0,SEEK_SET);	
	
		/*检测结果*/
		printf("Write Buf: %s\n",Buf);
	}
	
	return 0;	

}
