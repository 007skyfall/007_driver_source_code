#ifndef __MYIOCTL_H__
#define __MYIOCTL_H__

#define type 'k'

#define LED2_OP _IOWR(type,1,int)
#define LED3_OP _IOWR(type,2,int)

#endif

