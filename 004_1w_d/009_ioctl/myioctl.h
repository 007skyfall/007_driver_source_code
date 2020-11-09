#ifndef __MYIOCTL_H__
#define __MYIOCTL_H__


#define type 'k'

#define ACCESS_INT		 _IOWR(type,0,int)
#define ACCESS_STR_W 	 _IOWR(type,1,char *)
#define ACCESS_STR_R 	 _IOWR(type,2,char *)

#endif


