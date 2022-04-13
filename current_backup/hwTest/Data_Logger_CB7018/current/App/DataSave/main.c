#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include "../../INC/datastore.h"
#include "userlog.h"
#include "common_utils.h"
#include "local_utils.h"
#include "message.h"
#include "main.h"

volatile S_SYSTEM_DATA	*myData;
volatile S_DATA_SAVE	*myPs; //my process : DataSave
char psName[16];
FILE *fpM;
	
int main(void)
{
    int rtn;
    struct timeval tv;
    fd_set rfds;
    
	if(Initialize() < 0) return 0;

    while(myData->AppControl.signal[APP_SIG_DATA_SAVE_PROCESS] == PHASE1) {
    	tv.tv_sec = 0;
		tv.tv_usec = 100000;
		FD_ZERO(&rfds);

		rtn = select(0, &rfds, NULL, NULL, &tv);
		if(rtn == 0) {
			DataSave_Control();
		} else {
		}
    }
	CloseProcess();
    return 0;
}

int Initialize(void)
{
	if(OpenSharedMemory(0) < 0) return -1;
	
	myPs = &(myData->DataSave);
	
	InitSharedMemory();
	
	memset((char *)&psName[0], 0x00, 16);
	strcpy(psName, "DataSave");

	if(Read_DataSave_Config() < 0) return -2;

	myData->AppControl.signal[APP_SIG_DATA_SAVE_PROCESS] = PHASE1;
	return 0;
}

void DataSave_Control(void)
{
	MessageCheck();
	SignalCheck();
}

void SignalCheck(void)
{
	char fileName[128];

	if(myPs->signal[DATASAVE_SIG_SAVED_FILE_DELETE] == PHASE1) {
		myPs->signal[DATASAVE_SIG_SAVED_FILE_DELETE] = PHASE0;
		memset(fileName, 0x00, 128);
		sprintf(fileName, "%s", "rm -rf ./monitoringData/*");
		system(fileName);
		sleep(1);
	}
}

void CloseProcess(void)
{
    myData->AppControl.signal[APP_SIG_DATA_SAVE_PROCESS] = PHASE3;
	
	CloseSharedMemory();
}
