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

volatile S_SYSTEM_DATA *myData;
volatile S_COOLING_CONTROL *myPs; //my process : CoolingControl
char psName[PROCESS_NAME_SIZE];

int main(void)
{
    int	rtn;
    struct timeval tv;
    fd_set rfds;
	
	if(Initialize() < 0) return 0;	
	
    while(myData->AppControl.signal[myPs->misc.psSignal] == P1) {
		myData->cooling_connected = P0;
		myPs->signal[COOLING_SIG_NET_CONNECTED] = P0;
		if(InitNetwork() < 0) continue;
		
		while(myData->AppControl.signal[myPs->misc.psSignal] == P1) {
	    	tv.tv_sec = 0;
			tv.tv_usec = 200000;
		   	//tv.tv_usec = 50000; //50mS						
		   	//tv.tv_usec = 100000; //100mS						
			//tv.tv_usec = 200000; //200ms
			
		    FD_ZERO(&rfds);
		    if(myPs->config.network_socket > 0){ 
				FD_SET(myPs->config.network_socket, &rfds);
			}

			rtn = select(myPs->config.network_socket+1,
							&rfds, NULL, NULL, &tv);
			if(rtn > 0) {
				if(FD_ISSET(myPs->config.network_socket, &rfds) == 1) {
					if(NetworkPacket_Receive() < 0) {
						rtn = -1;
					} else {
						rtn = Parsing_NetworkEvent();
					}
				}
			} else if(rtn == 0) {
				rtn = Cooling_Control();
			}
	    }
	}

	Close_Process();
    return 0;
}

int Cooling_Control(void)
{
	int rtn;

	//Check_Message();
	Check_Signal();	
	
	rtn = Check_NetworkState();

	return rtn;
}

void Check_Signal(void)
{
	//myPs->misc.CoolingNo = 1;	
	myPs->data[1].command_sv = 25;	//set val test
	switch(myPs->signal[COOLING_SIG_PROCESS]){
		case P1:
			send_cmd_request(myPs->misc.CoolingNo);	
			myPs->signal[COOLING_SIG_PROCESS]++;
			break;
		case P2:
			if(myPs->data[myPs->misc.CoolingNo].command_sv * 10
			== myPs->data[myPs->misc.CoolingNo].read_sv){
				myPs->data[myPs->misc.CoolingNo].send_sv_flag = P0;
				myPs->data[myPs->misc.CoolingNo].send_error_cnt = 0;
			}else{
				if(myPs->data[myPs->misc.CoolingNo].send_sv_flag == P1){
					if(myPs->data[myPs->misc.CoolingNo].send_error_cnt++ >= 5){
						userlog(DEBUG_LOG, psName, "send error\n");
						myPs->data[myPs->misc.CoolingNo].send_error_cnt = 5;
						myData->AppControl.signal[myPs->misc.psSignal] = P2;
					}
				}
				send_cmd_set_value(myPs->misc.CoolingNo);
			}
			myPs->signal[COOLING_SIG_PROCESS] = P1;
			myPs->misc.CoolingNo++;	
			if(myPs->misc.CoolingNo > myPs->config.installed_cooling){
				myPs->misc.CoolingNo = 1;
			}
			break;
		default:
			break;
	}
}


void Close_Process(void)
{
	if(myPs->config.network_socket > 0) {
		close(myPs->config.network_socket);
	}
	
	myData->AppControl.signal[myPs->misc.psSignal] = P3;

	Close_SystemMemory();
}

