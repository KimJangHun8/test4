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
volatile S_DAQ_CLIENT *myPs; //my process : SubClient[?]
char psName[PROCESS_NAME_SIZE];

int main(void)
{
    int	rtn;
    struct timeval tv;
    fd_set rfds;

	if(Initialize() < 0) return 0;	

    while(myData->AppControl.signal[myPs->misc.psSignal] == P1) {
		myPs->signal[DAQ_CLIENT_SIG_NET_CONNECTED] = P0;

		if(InitNetwork() < 0){
		   	continue; //kjgw shutdown process
		}

		while(myData->AppControl.signal[myPs->misc.psSignal] == P1) {
	    	tv.tv_sec = 0;
		    tv.tv_usec = 300000;
		   // tv.tv_usec = 500000;
		    FD_ZERO(&rfds);
		    if(myPs->config.network_socket > 0)
				FD_SET(myPs->config.network_socket, &rfds);
					
	//		rtn = Parsing_NetworkEvent();
			
			rtn = select(myPs->config.network_socket+1,
				&rfds, NULL, NULL, &tv);
			if(rtn > 0) {
				if(FD_ISSET(myPs->config.network_socket, &rfds) == 1) {
					if(NetworkPacket_Receive() < 0) {
						rtn = -1;
					} else {
						rtn = Parsing_NetworkEvent();
						if(rtn < 0) break;
					}
				}
			} else if(rtn == 0) {
				rtn = DAQ_Client_Control();
				if(rtn < 0) {
					break;
				}
			} else {
			}
	    }
	}

	Close_Process();
    return 0;
}

int DAQ_Client_Control(void)
{
	int rtn;

	Check_Signal();
	rtn = Check_NetworkState();
	
	return rtn;
}

void Check_Signal(void)
{
	if(myPs->signal[DAQ_CLIENT_SIG_NET_CONNECTED] == P1){
		if(myPs->signal[DAQ_CLIENT_SIG_START_FLAG] == P0) {
			myPs->signal[DAQ_CLIENT_SIG_START_FLAG] = P1;
			userlog(DEBUG_LOG, psName, "DAQ_CLIENT TEMP CHECK START\n");
			send_cmd_run(0);
		}
	}

}

void Close_Process(void)
{
	send_cmd_stop(0);
	if(myPs->config.network_socket > 0) {
		close(myPs->config.network_socket);
	}
	
	myData->AppControl.signal[myPs->misc.psSignal] = P3;
	Close_SystemMemory();
}
