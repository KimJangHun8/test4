#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../../INC/datastore.h"
#include "mbuff.h"
#include "common_utils.h"

extern volatile S_SYSTEM_DATA *myData;

int OpenSharedMemory(int clear)
{
	myData = (volatile S_SYSTEM_DATA *) mbuff_alloc("myData",
		sizeof(S_SYSTEM_DATA));
	if(myData == NULL) {
		printf("mbuff_alloc failed\n");
		return -1;
	}

    if(myData == ((S_SYSTEM_DATA *)-1)) {
		printf("Can not create shared memory\n");
		return -2;
    }

	if(clear == 1) {
		memset((S_SYSTEM_DATA *)myData, 0x00, sizeof(S_SYSTEM_DATA));
		printf("Sizeof S_SYSTEM_DATA %d bytes\n", sizeof(S_SYSTEM_DATA));
	}
	return 0;
}

void CloseSharedMemory(void)
{
	mbuff_free("myData", (void*)myData);
}
/*
int my_system(const char *) {
	int pid, status;

	if(command == 0) return 1;
	
	pid = fork();
	if(pid == -1) {
		return -1;
	} else if(pid == 0) {
		char *argv[4];

		argv[0] = "sh";
		argv[1] = "-c";
		argv[2] = command;
		argv[3] = 0;
		execve("/bin/sh", argv, environ);
		exit(127);
	}

	do {
		if(waitpid(pid, &status, 0) == -1) {
			if(errno != EINTR) return -1;
		} else {
			return status;
		}
	} while(1);
}*/
