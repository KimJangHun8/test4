#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include "../../INC/datastore.h"
#include "userlog.h"
#include "common_utils.h"
#include "local_utils.h"
#include "message.h"
#include "com_io.h"
#include "comm.h"
#include "serial.h"
#include "main.h"

volatile S_SYSTEM_DATA *myData;
volatile S_METER  		*myPs; //my process : Meter1
char	psName[16];

int main(void)
{
	int rtn;
	struct timeval tv;
	fd_set rfds;
	
	if(Initialize() < 0) return 0;
   	
	while(myData->AppControl.signal[APP_SIG_METER1_PROCESS] == PHASE1) {
		tv.tv_sec = 0;
		tv.tv_usec = 100000;
		FD_ZERO(&rfds);
		if(myPs->config.ttyS_fd > 0)
			FD_SET(myPs->config.ttyS_fd, &rfds);
		
		rtn = Parsing_SerialEvent();

		rtn = select(myPs->config.ttyS_fd+1, &rfds, NULL, NULL, &tv);
		//userlog(DEBUG_LOG, psName, "event %d\n", rtn); //kjgd
		if(rtn > 0) {
			if(FD_ISSET(myPs->config.ttyS_fd, &rfds) == 1) {
				if(SerialPacket_Receive() < 0) {
					rtn = ComPortInitialize(myPs->config.comPort,
						myPs->config.comBps);
					if(rtn < 0) break;
					myPs->config.ttyS_fd = rtn;
					continue;
				}
				rtn = Parsing_SerialEvent();
			}
		} else if(rtn == 0) {
			Meter_Control();
			if(ComPortStateCheck() < 0) {
				rtn = ComPortInitialize(myPs->config.comPort,
					myPs->config.comBps);
				if(rtn < 0) break;
				myPs->config.ttyS_fd = rtn;
				continue;
			}
			rtn = Parsing_SerialEvent();
		} else {
		}
	}

	CloseProcess();
	return 0;
}

int	Initialize(void)
{
	int rtn;
	
	if(OpenSharedMemory(0) < 0) return -1;
	
	myPs = &(myData->Meter1);

	InitSharedMemory();

	memset((char *)&psName[0], 0x00, 16);
	strcpy(psName, "Meter1");
	
	if(Read_Meter_Config() < 0) return -2;
	
	rtn = ComPortInitialize(myPs->config.comPort, myPs->config.comBps);
	if(rtn < 0) return -3;
	myPs->config.ttyS_fd = rtn;

	myData->AppControl.signal[APP_SIG_METER1_PROCESS] = PHASE1;
	return 0;
}

void Meter_Control(void)
{
	MessageCheck();
	SignalCheck();
}

void SignalCheck(void)
{
	long diff;
	time_t the_time;

	switch(myPs->signal[METER_SIG_INITIALIZE]) {
		case PHASE1:
			send_cmd_initialize(1);
			myPs->signal[METER_SIG_INITIALIZE]++;
			break;
		case PHASE2:
			send_cmd_initialize(2);
			myPs->signal[METER_SIG_INITIALIZE]++;
			break;
		case PHASE3:
			send_cmd_initialize(3);
			myPs->signal[METER_SIG_INITIALIZE]++;
			break;
		case PHASE4:
			send_cmd_initialize(4);
			myPs->signal[METER_SIG_INITIALIZE] = PHASE0;
			break;
		case PHASE11:
			send_cmd_initialize(1);
			myPs->signal[METER_SIG_INITIALIZE]++;
			break;
		case PHASE12:
			send_cmd_initialize(2);
			myPs->signal[METER_SIG_INITIALIZE]++;
			break;
		case PHASE13:
			send_cmd_initialize(5);
			//kjgw send_cmd_initialize(3);
			myPs->signal[METER_SIG_INITIALIZE]++;
			break;
		case PHASE14:
			send_cmd_initialize(4);
			myPs->signal[METER_SIG_INITIALIZE] = PHASE0;
			break;
		default:	break;
	}

	switch(myPs->signal[METER_SIG_LOG_START]) {
		case PHASE1:
			(void)time(&the_time);
			myPs->saveTime = the_time;
			myPs->start_saveTime = the_time;
			myPs->signal[METER_SIG_LOG_START] = PHASE2;
			break;
		case PHASE2:
			if(myPs->signal[METER_SIG_LOG_STOP] == PHASE1) {
				myPs->signal[METER_SIG_LOG_STOP] = PHASE0;
				myPs->signal[METER_SIG_LOG_START] = PHASE0;
				myPs->signal[METER_SIG_REQUEST_PHASE] = PHASE0;
				break;
			}
			(void)time(&the_time);
			diff = the_time - myPs->saveTime;
			if(diff >= (long)(myPs->config.saveInterval / 1000)) {
				myPs->saveTime = the_time;
				myPs->signal[METER_SIG_LOG_START] = PHASE3;
				myPs->signal[METER_SIG_REQUEST_PHASE] = PHASE1;
				send_cmd_request();
			}
			break;
		case PHASE3:
			if(myPs->signal[METER_SIG_REQUEST_PHASE] == PHASE2) {
				myPs->signal[METER_SIG_LOG_START] = PHASE2;
				myPs->signal[METER_SIG_REQUEST_PHASE] = PHASE0;
				send_msg(METER1_TO_DATASAVE, MSG_METER1_DATASAVE_MONITOR_DATA,
					(int)(myPs->saveTime - myPs->start_saveTime),
					(int)myPs->value);
			}
			break;
		default: break;
	}
}

void CloseProcess(void)
{
	if(myPs->config.ttyS_fd > 0) {
		if(closetty(myPs->config.ttyS_fd) < 0) {
			userlog(DEBUG_LOG, psName, "ttyS %d close error\n",
				myPs->config.ttyS_fd);
		}
	}
	
	myData->AppControl.signal[APP_SIG_METER1_PROCESS] = PHASE3;	

	CloseSharedMemory();
}
