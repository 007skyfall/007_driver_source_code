#include <stdio.h>

#define DATA 0x12345678

int main(int argc, const char *argv[])
{
	unsigned long a = DATA;
	// a &= ~(0xf<<(1*4)) | (0x01<<(1*4));
	unsigned long b = (0xf <<(1*4));
	unsigned long c = ~b;
	unsigned long d = c | (0x01 <<(1*4));
	unsigned long e = d | DATA;
    
    printf("a = %02x\n",a); 
	printf("b = %02x\n",b);
	printf("c = %02x\n",c);
	printf("d = %02x\n",d);
	printf("e = %02x\n",e);

#if 0

a = 12345678
b = f0
c = ffffff0f
d = ffffff1f
e = ffffff7f

#endif

	return 0;
}
