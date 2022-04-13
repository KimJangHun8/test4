#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/time.h>
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
volatile S_FND_METER  *myPs; //my process : FndMeter
char	psName[PROCESS_NAME_SIZE];

int main(void)
{
	int rtn;
	struct timeval tv;
	fd_set rfds;
	if(Initialize() < 0) return 0;
   	
	while(myData->AppControl.signal[APP_SIG_FND_METER_PROCESS] == P1) {
		if(ComPortInitialize() < 0) continue;
	while(myData->AppControl.signal[APP_SIG_FND_METER_PROCESS] == P1) {
		tv.tv_sec = 0;
		tv.tv_usec = 100000;
		FD_ZERO(&rfds);
		if(myPs->config.ttyS_fd > 0)
			FD_SET(myPs->config.ttyS_fd, &rfds);
		
//		rtn = Parsing_SerialEvent();

		rtn = select(myPs->config.ttyS_fd+1, &rfds, NULL, NULL, &tv);
//		userlog(DEBUG_LOG, psName, "event %d\n", rtn); //kjgd
		if(rtn > 0) {
			if(FD_ISSET(myPs->config.ttyS_fd, &rfds) == 1) {
				if(SerialPacket_Receive() < 0) {
					rtn = ComPortInitialize();
					if(rtn < 0) break;
					myPs->config.ttyS_fd = rtn;
					continue;
				} else {
					rtn = Parsing_SerialEvent();
				}
			}
	//		FndMeter_Control(); 
		} else if(rtn == 0) {
			FndMeter_Control(); 
/*			if(ComPortStateCheck() < 0) {
				rtn = ComPortInitialize();
				if(rtn < 0) break;
				myPs->config.ttyS_fd = rtn;
				continue;
			}
			rtn = Parsing_SerialEvent() ;
			*/
		} else {
		}
	}
	}
	Close_Process();
	return 0;
}

int	Initialize(void)
{
//	int rtn;
	
	if(Open_SystemMemory(0) < 0) return -1;
	
	myPs = &(myData->FndMeter);

	Init_SystemMemory();

	memset((char *)&psName[0], 0x00, 16);
	strcpy(psName, "Fnd");
	
	if(Read_FndMeter_Config() < 0) return -2;
	
	myData->AppControl.signal[APP_SIG_FND_METER_PROCESS] = P1;
	myPs->signal[FND_METER_SIG_MEASURE] = P1;
	return 0;
}

void FndMeter_Control(void)
{
	Check_Message();
	Check_Signal();
}

void Check_Signal(void)
{
//	int bd, ch, type;

	switch(myPs->signal[FND_METER_SIG_MEASURE]) {
		case P1:
			myPs->signal[FND_METER_SIG_MEASURE] = P2;
			send_cmd_request(0, 0, 1);
			break;
		case P2:
			myPs->signal[FND_METER_SIG_MEASURE] = P3;
			send_cmd_request(0, 0, 2);
			break;
		case P3:
			myPs->signal[FND_METER_SIG_MEASURE] = P4;
			send_cmd_request(0, 0, 3);
			break;
		case P4:
			myPs->signal[FND_METER_SIG_MEASURE] = P5;
			send_cmd_request(0, 0, 4);
			break;
		case P5:
			if(myPs->config.installCh > 1)
				myPs->signal[FND_METER_SIG_MEASURE] = P6;
			else
				myPs->signal[FND_METER_SIG_MEASURE] = P1;

			break;
		case P6:
			myPs->signal[FND_METER_SIG_MEASURE] = P7;
			send_cmd_request(0, 1, 1);
			break;
		case P7:
			myPs->signal[FND_METER_SIG_MEASURE] = P8;
			send_cmd_request(0, 1, 2);
			break;
		case P8:
			myPs->signal[FND_METER_SIG_MEASURE] = P9;
			send_cmd_request(0, 1, 3);
			break;
		case P9:
			myPs->signal[FND_METER_SIG_MEASURE] = P10;
			send_cmd_request(0, 1, 4);
			break;
		case P10:
			if(myPs->config.installCh > 2)
				myPs->signal[FND_METER_SIG_MEASURE] = P11;
			else
				myPs->signal[FND_METER_SIG_MEASURE] = P1;
			break;
		case P11:
			myPs->signal[FND_METER_SIG_MEASURE] = P12;
			send_cmd_request(1, 0, 1);
			break;
		case P12:
			myPs->signal[FND_METER_SIG_MEASURE] = P13;
			send_cmd_request(1, 0, 2);
			break;
		case P13:
			myPs->signal[FND_METER_SIG_MEASURE] = P14;
			send_cmd_request(1, 0, 3);
			break;
		case P14:
			myPs->signal[FND_METER_SIG_MEASURE] = P15;
			send_cmd_request(1, 0, 4);
			break;
		case P15:
			if(myPs->config.installCh > 3)
				myPs->signal[FND_METER_SIG_MEASURE] = P16;
			else
				myPs->signal[FND_METER_SIG_MEASURE] = P1;
			break;
		case P16:
			myPs->signal[FND_METER_SIG_MEASURE] = P17;
			send_cmd_request(1, 1, 1);
			break;
		case P17:
			myPs->signal[FND_METER_SIG_MEASURE] = P18;
			send_cmd_request(1, 1, 2);
			break;
		case P18:
			myPs->signal[FND_METER_SIG_MEASURE] = P19;
			send_cmd_request(1, 1, 3);
			break;
		case P19:
			myPs->signal[FND_METER_SIG_MEASURE] = P20;
			send_cmd_request(1, 1, 4);
			break;
		case P20:
			myPs->signal[FND_METER_SIG_MEASURE] = P1;
			break;

	}
/*	
	for(bd = 0; bd < 2; bd++){
		for(ch = 0; ch < 2;ch++){
			for(type = 1; type < 5; type++){
				send_cmd_request(bd, ch, type);
			}
		}
	}
*/	
}

void Close_Process(void)
{
	if(myPs->config.ttyS_fd > 0) {
		if(closetty(myPs->config.ttyS_fd) < 0) {
			userlog(DEBUG_LOG, psName, "ttyS %d close error\n",
				myPs->config.ttyS_fd);
		}
	}
	
	myData->AppControl.signal[APP_SIG_FND_METER_PROCESS] = P3;	

	Close_SystemMemory();
}
