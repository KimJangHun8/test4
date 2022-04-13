#ifdef __KERNEL__
#include <rtl.h>
#else
#include <stdlib.h>
#endif
#include <mbuff.h>
#include "../../INC/datastore.h"
#include "common_utils.h"

extern volatile S_SYSTEM_DATA *myData;

void Unload_Module(void)
{
#ifdef __KERNEL__
#else
	system("./Unload_mControl");
#endif
}

void Load_Module(void)
{
#ifdef __KERNEL__
#else
	system("insmod -f /usr/src/rtlinux-3.2-pre2/drivers/mbuff/mbuff.o");
#endif
}

int Open_SystemMemory(int clear)
{
	myData = (volatile S_SYSTEM_DATA *) mbuff_alloc("myData",
		sizeof(S_SYSTEM_DATA));
	if(myData == NULL) {
#ifdef __KERNEL__
		rtl_printf("mbuff_alloc failed\n");
#else
		printf("mbuff_alloc failed\n");
#endif
		return -1;
	}

    if(myData == ((S_SYSTEM_DATA *)-1)) {
#ifdef __KERNEL__
		rtl_printf("Can not create system memory\n");
#else
		printf("Can not create system memory\n");
#endif
		return -2;
    }

	if(clear == 1) {
		memset((S_SYSTEM_DATA *)myData, 0, sizeof(S_SYSTEM_DATA));
#ifdef __KERNEL__
		rtl_printf("Sizeof S_SYSTEM_DATA %d bytes\n", sizeof(S_SYSTEM_DATA));
#else
		printf("Sizeof S_SYSTEM_DATA %d bytes\n", sizeof(S_SYSTEM_DATA));
#endif
	}
	return 0;
}

void Close_SystemMemory(void)
{
	mbuff_free("myData", (void*)myData);
}

void Close_mbuff(int pointer)
{
	char *sysData;

	sysData = (char *)pointer;
	mbuff_free("myData", (void *)sysData);
}
