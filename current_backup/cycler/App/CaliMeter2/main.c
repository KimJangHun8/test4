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
#include "meter_socket.h"

volatile S_SYSTEM_DATA *myData;
volatile S_CALI_METER  *myPs; //my process : CaliMeter
char	psName[PROCESS_NAME_SIZE];

int main(void)
{
	int rtn;
	struct timeval tv;
	fd_set rfds;
	
	if(Initialize() < 0) return 0;
   	
	while(myData->AppControl.signal[APP_SIG_CALI_METER2_PROCESS] == P1) {
		//161220 add for cali to Lan
		if(myPs->config.Lan_Use == P1
						&& myPs->signal[CALI_METER_SIG_LAN_CONNECT] == P1){
			myPs->config.ttyS_fd = SetMeterSock(myPs->config.MeterSock_Port, 
				 		 			(char *)&myPs->config.CaliMeterIP);
			printf("1 : %d\n",myPs->config.ttyS_fd);
		}
		//hun_211019 232 to Lan Converter
		if(myPs->config.Lan_Use == P2){
			myPs->signal[CALI_METER_SIG_LAN_USE] = P1;
		}
		
		tv.tv_sec = 0;
		tv.tv_usec = 100000;
		
		FD_ZERO(&rfds);
		if(myPs->config.ttyS_fd > 0)
			FD_SET(myPs->config.ttyS_fd, &rfds);
		
		rtn = Parsing_SerialEvent();

		rtn = select(myPs->config.ttyS_fd+1, &rfds, NULL, NULL, &tv);
		//userlog(DEBUG_LOG, psName, "event %d\n", rtn); //kjgd
		//hun_211019
		if(rtn > 0) {
			if(FD_ISSET(myPs->config.ttyS_fd, &rfds) == 1) {
				if(SerialPacket_Receive() < 0) {
					if(myPs->config.Lan_Use == P0){
						rtn = ComPortInitialize(myPs->config.comPort,
							myPs->config.comBps);
						if(rtn < 0) break;
						myPs->config.ttyS_fd = rtn;
						continue;
					}else if(myPs->config.Lan_Use != P0){
						rtn = SetMeterSock(myPs->config.MeterSock_Port, 
				 		 			(char *)&myPs->config.CaliMeterIP);
						if(rtn < 0) break;
						myPs->config.ttyS_fd = rtn;
						continue;
					}
				}
				rtn = Parsing_SerialEvent();
			}
		} else if(rtn == 0) {
			CaliMeter_Control();

			//hun_211019
			if(myPs->config.Lan_Use == P0){
				if(ComPortStateCheck() < 0){
					rtn = ComPortInitialize(myPs->config.comPort,
					myPs->config.comBps);
					if(rtn < 0) break;
						myPs->config.ttyS_fd = rtn;
						continue;
				}
			}else if(myPs->config.Lan_Use != P0){

			}
			rtn = Parsing_SerialEvent();
		} else {
		}
	}

	Close_Process();
	return 0;
}

int	Initialize(void)
{
	int rtn;
	
	if(Open_SystemMemory(0) < 0) return -1;
	
	myPs = &(myData->CaliMeter2);

	Init_SystemMemory();

	memset((char *)&psName[0], 0x00, 16);
	strcpy(psName, "Cali");
	
	if(Read_CaliMeter_Config() < 0) return -2;
    if(myPs->config.Shunt_Sel_Calibrator == 1){
		if(Read_Shunt_Select_Calibrator_Config() < 0) return -3;
	}	
	//hun_211019
	if(myPs->config.Lan_Use == P0){
		rtn = ComPortInitialize(myPs->config.comPort, myPs->config.comBps);
		if(rtn < 0) return -3;
		myPs->config.ttyS_fd = rtn;
	}else if(myPs->config.Lan_Use != P0){
		if(myPs->config.MeterSock_Port > 0) close(myPs->config.MeterSock_Port);
		rtn = SetMeterSock(myPs->config.MeterSock_Port,(char *)&myPs->config.CaliMeterIP);
		if(rtn < 0){
			close(myPs->config.MeterSock_Port);
			userlog(DEBUG_LOG,psName,"Can not initialize network : %d\n",
				myPs->config.MeterSock_Port);
			return -4;
		}
		myPs->config.ttyS_fd = rtn;
	}
	

	myData->AppControl.signal[APP_SIG_CALI_METER2_PROCESS] = P1;
	return 0;
}

void CaliMeter_Control(void)
{
	Check_Message();
	Check_Signal();
}

void Check_Signal(void)
{
#if SHUNT_R_RCV >= 1
	if(myData->mData.cali_hallCT == 1 		//hun_210830
		|| myData->mData.cali_hallCT == 3){
		myPs->config.readType = READ_V_I;
	}else{
		myPs->config.readType = READ_V_V;	
	}
#endif
	switch(myPs->signal[CALI_METER_SIG_INITIALIZE]) {
		case P1:
			send_cmd_initialize(1);
			myPs->signal[CALI_METER_SIG_INITIALIZE]++;
			break;
		case P2:
			send_cmd_initialize(2);
			myPs->signal[CALI_METER_SIG_INITIALIZE]++;
			break;
		case P3:
			send_cmd_initialize(3);
			myPs->signal[CALI_METER_SIG_INITIALIZE]++;
			break;
		case P4:
			send_cmd_initialize(4);
			myPs->signal[CALI_METER_SIG_INITIALIZE] = P0;
			break;
		case P11:
			send_cmd_initialize(1);
			myPs->signal[CALI_METER_SIG_INITIALIZE]++;
			break;
		case P12:
			send_cmd_initialize(2);
			myPs->signal[CALI_METER_SIG_INITIALIZE]++;
			break;
		case P13:
			if(myPs->config.readType == READ_V_V) {
				send_cmd_initialize(3);
			} else if(myPs->config.readType == READ_V_I) {
				send_cmd_initialize(5);
			}
			myPs->signal[CALI_METER_SIG_INITIALIZE]++;
			break;
		case P14:
			send_cmd_initialize(4);
			myPs->signal[CALI_METER_SIG_INITIALIZE] = P0;
			break;
		case P21:
			send_cmd_initialize(1);
			myPs->signal[CALI_METER_SIG_INITIALIZE]++;
			break;
		case P22:
			send_cmd_initialize(2);
			myPs->signal[CALI_METER_SIG_INITIALIZE]++;
			break;
		case P23:
			send_cmd_initialize(3);
			myPs->signal[CALI_METER_SIG_INITIALIZE]++;
			break;
		case P24:
			send_cmd_initialize(4);
			//kjgw send_msg(CALIMETER2_TO_MAIN, MSG_METER_MAIN_INITIALIZE_REPLY, 0, 0);
			myPs->signal[CALI_METER_SIG_INITIALIZE] = P0;
			break;
		case P31:
			send_cmd_initialize(1);
			myPs->signal[CALI_METER_SIG_INITIALIZE]++;
			break;
		case P32:
			send_cmd_initialize(2);
			myPs->signal[CALI_METER_SIG_INITIALIZE]++;
			break;
		case P33:
			if(myPs->config.readType == READ_V_V) {
				send_cmd_initialize(3);
			} else if(myPs->config.readType == READ_V_I) {
				send_cmd_initialize(5);
			}
			myPs->signal[CALI_METER_SIG_INITIALIZE]++;
			break;
		case P34:
			send_cmd_initialize(4);
			//kjgw send_msg(METER2_TO_MAIN, MSG_METER_MAIN_INITIALIZE_REPLY, 0, 0);
			myPs->signal[CALI_METER_SIG_INITIALIZE] = P0;
			break;
		default:	break;
	}

	if(myPs->signal[CALI_METER_SIG_REQUEST_PHASE] == P2) {
		send_msg(CALIMETER2_TO_MODULE, MSG_METER_MODULE_REQUEST_REPLY,
			myPs->receivedBd, myPs->receivedCh);
		myPs->signal[CALI_METER_SIG_REQUEST_PHASE] = P0;
	}
}

void Close_Process(void)
{
	//hun_211019
	if(myPs->config.Lan_Use == P0){
		if(myPs->config.ttyS_fd > 0) {
			if(closetty(myPs->config.ttyS_fd) < 0) {
				userlog(DEBUG_LOG, psName, "ttyS %d close error\n",
					myPs->config.ttyS_fd);
			}
		}
	}else if(myPs->config.Lan_Use != P0){
		if(myPs->config.ttyS_fd > 0) {
			close(myPs->config.ttyS_fd);
		}
	}
	
	myData->AppControl.signal[APP_SIG_CALI_METER2_PROCESS] = P3;	

	Close_SystemMemory();
}
