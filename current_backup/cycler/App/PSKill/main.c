#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <asm/io.h>
#include "../../INC/datastore.h"
#include "common_utils.h"
#include "userlog.h"
#include "main.h"

volatile S_SYSTEM_DATA *myData;
		
int main(void)
{
	if(Open_SystemMemory(0) < 0) return -1;

	send_msg(PSKILL_TO_MODULE,MSG_PSKILL_MODULE_EXIT,0,0);

	printf("PSKILL : start killing process \n");

	usleep(100000);
	
	Close_SystemMemory();
    
	return 0;
}
