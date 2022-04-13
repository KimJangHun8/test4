#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "../../INC/datastore.h"
#include "userlog.h"
#include "common_utils.h"
#include "local_utils.h"
#include "message.h"
#include "StandardInput.h"
#include "main.h"

volatile S_SYSTEM_DATA *myData;
volatile S_APP_CONTROL *myPs; //my process : AppControl
char psName[16];

int main(int argc, char *argv[])
{
    int		rtn;
	char	path[128];
    struct	timeval tv;
    fd_set	rfds;
	
	if(argc != 3) {
		printf("System loader start fail %d\n", argc);
		return 0;
	}
	
	rtn = atoi(argv[1]); //boot on start compare (1:boot, force:0)
	
	memset(path, 0x00, sizeof path);
	strcpy(path, argv[2]);

	rtn = Initialize(rtn, path);
	if(rtn < 0) {
		if(rtn <= -10) {
			userlog(DEBUG_LOG, psName, "System initialize fail %d\n", rtn);
		} else if(rtn < 0) {
			printf("System initialize fail %d\n", rtn);
		}
		CloseSharedMemory();
		system("rmmod mbuff");
		return 0;
	}

    while(myPs->signal[APP_SIG_APP_CONTROL_PROCESS] == PHASE1) {
	   	tv.tv_sec = 0;
	    tv.tv_usec = 200000;
	    FD_ZERO(&rfds);
	   	FD_SET(0, &rfds);
			
		rtn = select(1, &rfds, NULL, NULL, &tv);
		//userlog(DEBUG_LOG, psName, "event %d\n", rtn); //kjgd
		if(rtn != 0) {
			StandardInput_Receive();
		} else {
			AppControl();
		}
	}
	CloseAppControl();
    return 0;
}

int Initialize(int bootOnStartComp, char *path)
{
	ProcessKill("DataSave", path);
	ProcessKill("CaliMeter1", path);
	ProcessKill("CaliMeter2", path);

	if(OpenSharedMemory(1) < 0) return -1;

	myPs = &(myData->AppControl);

	if(LoadSharedMemory("sharedMemory", path) < 0)
		return -2; //saved shared memory data load
	InitSharedMemory();

	memset((char *)&myPs->config.projectPath[0], 0x00, 128);
	strcpy((char *)&myPs->config.projectPath[0], path);

	memset((char *)&psName[0], 0x00, 16);
	strcpy(psName, "App");

	InitLogfile("DEBUG_LOG", 0);
	InitLogfile("METER1_LOG", 1);
	InitLogfile("METER2_LOG", 2);

	if(Read_AppControl_Config(bootOnStartComp) < 0) return -10;
	
	if(ProcessLoad("Load_DataSave", "./", APP_SIG_DATA_SAVE_PROCESS) < 0) {
		CloseProcess("DataSave", APP_SIG_DATA_SAVE_PROCESS);
		return -21;
	}
	if(ProcessLoad("Load_Meter1", "./", APP_SIG_METER1_PROCESS) < 0) {
		CloseProcess("DataSave", APP_SIG_DATA_SAVE_PROCESS);
		CloseProcess("Meter1", APP_SIG_METER1_PROCESS);
		return -22;
	}
	if(ProcessLoad("Load_Meter2", "./", APP_SIG_METER2_PROCESS) < 0) {
		CloseProcess("DataSave", APP_SIG_DATA_SAVE_PROCESS);
		CloseProcess("Meter1", APP_SIG_METER1_PROCESS);
		CloseProcess("Meter2", APP_SIG_METER2_PROCESS);
		return -23;
	}
	
	myPs->signal[APP_SIG_APP_CONTROL_PROCESS] = PHASE1;
	userlog(DEBUG_LOG, psName, "System start\n");

	return 0;
}

void AppControl(void)
{
	MessageCheck();
	SignalCheck();
	ProcessCheck();
}

void SignalCheck(void)
{
	long diff;
	time_t the_time;

	if(myPs->signal[APP_SIG_QUIT] != PHASE0) {
		(void)time(&the_time);
		diff = the_time - myPs->misc.quitDelayTime;
		if(diff > 2) { //2sec
			myPs->signal[APP_SIG_APP_CONTROL_PROCESS] = PHASE2;
			if(myPs->signal[APP_SIG_QUIT] == PHASE1 //normal power off
				|| myPs->signal[APP_SIG_QUIT] == PHASE2 //force power off
				|| myPs->signal[APP_SIG_QUIT] == PHASE4 //terminal halt
				|| myPs->signal[APP_SIG_QUIT] == PHASE5 //main emg.
				|| myPs->signal[APP_SIG_QUIT] == PHASE6 //sub emg.
				|| myPs->signal[APP_SIG_QUIT] == PHASE7 //ac power fail
				|| myPs->signal[APP_SIG_QUIT] == PHASE8 //ups battery fail
				|| myPs->signal[APP_SIG_QUIT] == PHASE9 //smps fail
				|| myPs->signal[APP_SIG_QUIT] == PHASE10) { //OT fail
				myPs->signal[APP_SIG_APP_CONTROL_PROCESS] = PHASE10; //halt
			}
		}
		if(myPs->signal[APP_SIG_QUIT] == PHASE3) {
			myPs->signal[APP_SIG_APP_CONTROL_PROCESS] = PHASE2;
		} //terminal quit
	}
}

void CloseAppControl(void)
{
	CloseProcess("DataSave", APP_SIG_DATA_SAVE_PROCESS);
	CloseProcess("Meter1", APP_SIG_METER1_PROCESS);
	CloseProcess("Meter2", APP_SIG_METER2_PROCESS);
	
	SaveSharedMemory();

	if(myPs->signal[APP_SIG_APP_CONTROL_PROCESS] == PHASE10) {
		userlog(DEBUG_LOG, psName, "system shutdown\n");
		system("cd /");
		system("update -s1");
		system("shutdown -h now");
	} else {
		userlog(DEBUG_LOG, psName, "system quit\n");
		system("cd /");
		system("update -s1");
	}

	CloseSharedMemory();
	system("rmmod mbuff");
	system("./rmrtl");
}
