#include <unistd.h>

int hstop(){
	if(access("/home/lianghan/STOP",F_OK)!=-1)return 1;
	return 0;
}
