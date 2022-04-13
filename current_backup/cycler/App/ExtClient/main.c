#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include "../../INC/datastore.h"
#include "userlog.h"
#include "common_utils.h"
#include "local_utils.h"
#include "message.h"
#include "com_io.h"
#include "com_socket.h"
#include "network.h"
#include "main.h"

volatile S_SYSTEM_DATA	*myData;
volatile S_EXT_CLIENT 	*myPs;
char psName[PROCESS_NAME_SIZE];

int main(void)
{
    int	rtn;
    struct timeval tv;
    fd_set rfds;

	if(Initialize() < 0) return 0;	
	
    while(myData->AppControl.signal[myPs->misc.psSignal] == P1) {
		myPs->signal[EXT_SIG_NET_CONNECTED] = P0;
		myPs->sended_monitor_data_time 
			= myData->mData.misc.timer_mSec;
		myPs->sended_save_data_time 
			= myData->mData.misc.timer_mSec;
		if(InitNetwork() < 0) continue;

		while(myData->AppControl.signal[myPs->misc.psSignal] == P1) {
	    	tv.tv_sec = 0;
		    tv.tv_usec = 100000; //100mSec

		    FD_ZERO(&rfds);
		    if(myPs->config.network_socket > 0)
				FD_SET(myPs->config.network_socket, &rfds);

			rtn = select(myPs->config.network_socket+1,
				&rfds, NULL, NULL, &tv);
			if(rtn > 0) {
				if(FD_ISSET(myPs->config.network_socket, &rfds) == 1) {
					if(NetworkPacket_Receive() < 0) {
						close(myPs->config.network_socket);
						rtn = -1;
					} else {
						rtn = Parsing_NetworkEvent();
					}
				}
				if(rtn < 0){
					StateChange_Pause(1);
					break;
				}
			} else if(rtn == 0) {
				rtn = ExtClient_Control();
				if(rtn < 0){
					StateChange_Pause(2);
					break;
				}
				rtn = Parsing_NetworkEvent();
				if(rtn < 0){
					StateChange_Pause(3);
					break;
				}
			}
		}
	}
	Close_Process();
    return 0;
}

int ExtClient_Control(void)
{
	int rtn = 0;
	Check_Message();
	rtn = Check_Signal();
	if(rtn < 0) return rtn;
	rtn = Check_NetworkState();
	return rtn;
}

int Check_Signal(void)
{
	int rtn=0;
	
	if(myPs->signal[EXT_SIG_NET_CONNECTED] == P1) {
		send_cmd_module_info();
		myPs->signal[EXT_SIG_NET_CONNECTED] = P2;
		myPs->pingTimer = myData->mData.misc.timer_1sec;
		myPs->pingCount = 0;
	}else if(myPs->signal[EXT_SIG_NET_CONNECTED] == P2) {
		rtn = send_cmd_ch_data();
	}
	return rtn;
}


void Close_Process(void)
{
	if(myPs->config.network_socket > 0) {
		close(myPs->config.network_socket);
	}
	
	myData->AppControl.signal[myPs->misc.psSignal] = P3;

	Close_SystemMemory();
}
