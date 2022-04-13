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
volatile S_MAIN_CLIENT *myPs; //my process : MainClient
char psName[PROCESS_NAME_SIZE];

int main(void)
{
    int	rtn;
    struct timeval tv;
    fd_set rfds;

	if(Initialize() < 0) return 0;	

    while(myData->AppControl.signal[myPs->misc.psSignal] == P1) {
		myPs->signal[MAIN_SIG_NET_CONNECTED] = P0;
		if(myPs->signal[MAIN_SIG_NO_CONNECTION_RETRY] == P1) continue;
		if(InitNetwork() < 0) continue;
		
		while(myData->AppControl.signal[myPs->misc.psSignal] == P1) {
	    	tv.tv_sec = 0;
			if(myPs->signal[MAIN_SIG_NET_CONNECTED] == P2){
				tv.tv_usec = 100000; //100mS
			}else{
				tv.tv_usec = 200000; //200mS
			}

			//120129 kji chData upload time 
			if(myPs->chDataTimer == 1)  {
				myPs->signal[MAIN_SIG_NET_CONNECTED] = P2;
				myData->mData.config.sendAuxFlag = P1;
				send_msg(MAIN_TO_MODULE, MSG_MAIN_MODULE_SAVE_MSG_FLAG, 0, 0); 
			}
			if(myPs->chDataTimer > 0 ) myPs->chDataTimer--;
			if(myPs->chDataTimer < 0 ) myPs->chDataTimer = 0;
		    FD_ZERO(&rfds);
		    if(myPs->config.network_socket > 0)
				FD_SET(myPs->config.network_socket, &rfds);

			rtn = select(myPs->config.network_socket+1,
				&rfds, NULL, NULL, &tv);
	//		userlog(DEBUG_LOG, psName, "event %d\n", rtn); //kjgd
			if(rtn > 0) {
				if(FD_ISSET(myPs->config.network_socket, &rfds) == 1) {
					if(NetworkPacket_Receive() < 0) {
						rtn = -1;
					} else {
						rtn = Parsing_NetworkEvent();
					}
				}
				if(rtn < 0) {
					StateChange_Pause(1);
					break;
				}
//				rtn = MainClient_Control();
//				if(rtn < 0) {
//					StateChange_Pause(2);
//					break;
//				}
			} else if(rtn == 0) {
				rtn = MainClient_Control();
				if(rtn < 0) {
					StateChange_Pause(2);
					break;
				}
				rtn = Parsing_NetworkEvent();
				if(rtn < 0) {
					StateChange_Pause(3);
					break;
				}
			} else {}
	    }
	}

	Close_Process();
    return 0;
}

int MainClient_Control(void)
{
	int rtn;

	Check_Message();
	Check_Signal();
	rtn = Check_NetworkState();

	return rtn;
}

void Check_Signal(void)
{
//131228 oys modify : real time
#if REAL_TIME == 1
	int rtn, count, i, j, bd, ch;
#else
	int rtn, count, i, bd, ch;
#endif
	int k;

	
	if((myData->mData.misc.timer_1sec - myPs->sended_monitor_data_time)
		>= myPs->config.send_monitor_data_interval) {

		myPs->sended_monitor_data_time = myData->mData.misc.timer_1sec;
		if(myPs->signal[MAIN_SIG_NET_CONNECTED] == P2) {
			//not save end message
			if(myData->AppControl.misc.saveFlag){
				myData->AppControl.misc.saveFlag = 0;
				send_cmd_emg_status(M_FAIL_UNUSUAL_END,0);
			}
			if(myData->mData.config.installedTemp > 0 ||
				myData->mData.config.installedAuxV > 0){
				if(myData->mData.config.sendAuxFlag == P1){
					send_cmd_sensor_data();
				}
			}

			if(myData->DataSave.config.save_data_type == P1){
				send_cmd_ch_data();	// monitoring data
				for(i=0; i < myData->mData.config.installedCh; i++) {
					send_cmd_ch_10mS_data(i); //ch save data
				}
			}else{
				rtn = send_cmd_ch_data();
				if(rtn > 0) count = 10;
				else count = 0;
				while(count) {
					rtn = send_cmd_ch_data();
					if(rtn == 0) count = 0;
					else count--;
				}
			}
			for(i=0; i < myData->mData.config.installedCh; i++) {
#if CYCLER_TYPE == DIGITAL_CYC	//180710
				rtn = send_cmd_ch_pulse_data_10mS(i);
#else
				rtn = send_cmd_ch_pulse_data(i);
#endif
				if(rtn > 0) count = 15;
				else count = 0;
				while(count) {
#if CYCLER_TYPE == DIGITAL_CYC	//180710
					rtn = send_cmd_ch_pulse_data_10mS(i);
#else
					rtn = send_cmd_ch_pulse_data(i);
#endif
					if(rtn == 0) count = 0;
					else count--;
				}
			}
			
			if(myData->FADM.config.countMeter > 0
				|| myData->mData.config.FadBdUse == 1){
				for(i=0; i < myData->mData.config.installedCh; i++) {
					rtn = send_cmd_fadm_pulse_data(i);
					if(rtn > 0)	count = 15;
					else count = 0;
					while(count) {
						rtn = send_cmd_fadm_pulse_data(i);
						if(rtn == 0) count = 0;
						else count--;
					}
				}
				//171227 add
				for(i=0; i < myData->mData.config.installedCh; i++) {
					rtn = send_cmd_ch_pulse_data_iec(i);
					if(rtn > 0) count = 15;
					else count = 0;
					while(count) {
						rtn = send_cmd_ch_pulse_data_iec(i);
						if(rtn == 0) count = 0;
						else count--;
					}
				}
			}
		}
	}
	if(myPs->signal[MAIN_SIG_SEND_ERROR_CODE] == P1) {
		myPs->signal[MAIN_SIG_SEND_ERROR_CODE] = P0;
	}
	//131228 oys w : real time add
#if REAL_TIME == 1
	if((myData->mData.real_time[4] != myPs->misc.sent_real_time_request[0])
	|| (myData->mData.real_time[5] != myPs->misc.sent_real_time_request[1])
	|| (myData->mData.real_time[6] != myPs->misc.sent_real_time_request[2]))
		{
		j = 0;
		for(i=0; i < myData->mData.config.installedCh; i++) {
			bd = i / myData->mData.config.chPerBd;
			ch = i % myData->mData.config.chPerBd;
			if(myData->bData[bd].cData[ch].op.state == C_RUN
				|| myData->bData[bd].cData[ch].op.state == C_PAUSE) {
				j++;
			}
		}
		if(j == 0) {
			if(myPs->signal[MAIN_SIG_SEND_REAL_TIME_REQUEST] == P0) {
				myPs->signal[MAIN_SIG_SEND_REAL_TIME_REQUEST] = P1;
			}
		}
	}
    if(myPs->signal[MAIN_SIG_SEND_REAL_TIME_REQUEST] == P1
		&& myPs->signal[MAIN_SIG_NET_CONNECTED] == P2) {
		myPs->signal[MAIN_SIG_SEND_REAL_TIME_REQUEST] = P0;
		myPs->misc.sent_real_time_request[0]
			= myData->mData.real_time[4];   //day
		myPs->misc.sent_real_time_request[1]
			= myData->mData.real_time[5];   //month
		myPs->misc.sent_real_time_request[2]
			= myData->mData.real_time[6];   //year
		send_cmd_real_time_request();
	}
#endif
// 150512 oys add start : chData Backup, Restore

	k = 1;
	for(i=0; i < myData->mData.config.installedCh; i++) {
		if(myData->AppControl.backupFlag[i] == P1){
			bd = myData->CellArray1[i].bd;	
			ch = myData->CellArray1[i].ch;
			if(myData->bData[bd].cData[ch].op.state == C_PAUSE){
				if(myData->AppControl.backup[i] == P1
					|| myData->AppControl.backup[i] == P2) {
					k = 1;
				}else{
					k = 0;
				}
			}
		}
	}
	if(k == 0){
		send_cmd_ch_data_backup_reply();
	}

	k = 1;
	for(i=0; i < myData->mData.config.installedCh; i++) {
		if(myData->AppControl.restoreFlag[i] == P1){
			bd = myData->CellArray1[i].bd;	
			ch = myData->CellArray1[i].ch;
			if(myData->bData[bd].cData[ch].op.state == C_PAUSE){
				if(myData->AppControl.restore[i] == P1
					|| myData->AppControl.restore[i] == P2) {
					k = 1;
				}else{
					k = 0;
				}
			}
		}
	}
	if(k == 0){
		send_cmd_ch_data_restore_reply();
	}
// 150512 oys add end : chData Backup, Restore
}

void Close_Process(void)
{
	if(myPs->config.network_socket > 0) {
		close(myPs->config.network_socket);
	}
	
	myData->AppControl.signal[myPs->misc.psSignal] = P3;

	Close_SystemMemory();
}
