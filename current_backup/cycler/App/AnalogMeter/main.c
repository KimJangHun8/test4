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
#include "cali.h"
#include "main.h"

volatile S_SYSTEM_DATA *myData;
volatile S_ANALOG_METER  *myPs; //my process : AnalogMeter
char	psName[PROCESS_NAME_SIZE];

int main(void)
{
	int rtn;
	struct timeval tv;
	long scan_period1, scan_period2;
	fd_set rfds;
	if(Initialize() < 0) return 0;
   	
	if(myPs->config.functionType == 0) {
		scan_period1 = 0;
		scan_period2 = 180000;
	} else {
		scan_period1 = 0;
		scan_period2 = 450000;
	}
	while(myData->AppControl.signal[APP_SIG_ANALOG_METER_PROCESS] == P1) {
		if(ComPortInitialize() < 0) {
// 130606 khk w				
			sleep(3);
			continue;
		}
	while(myData->AppControl.signal[APP_SIG_ANALOG_METER_PROCESS] == P1) {
		tv.tv_sec = scan_period1;
		tv.tv_usec = scan_period2;
		FD_ZERO(&rfds);
		if(myPs->config.ttyS_fd > 0)
			FD_SET(myPs->config.ttyS_fd, &rfds);
		
//		rtn = Parsing_SerialEvent();

		rtn = select(myPs->config.ttyS_fd+1, &rfds, NULL, NULL, &tv);
		//userlog(DEBUG_LOG, psName, "event %d\n", rtn); //kjgd
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
		} else if(rtn == 0) {
			AnalogMeter_Control(); 
			if(ComPortStateCheck() < 0) {
				rtn = ComPortInitialize();
				if(rtn < 0) break;
				myPs->config.ttyS_fd = rtn;
				continue;
			}
			rtn = Parsing_SerialEvent() ;
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
	
	myPs = &(myData->AnalogMeter);

	Init_SystemMemory();

	memset((char *)&psName[0], 0x00, 16);
	strcpy(psName, "Analog");
	
	if(Read_AnalogMeter_Config() < 0) return -2;
	if(Read_TempArray_A() < 0) return -3;
	if(Read_AnalogMeter_CaliData() < 0) return -4;
#ifdef _TEMP_CALI  
	if(Read_AnalogMeter_CaliData_2() < 0) return -5;
#endif
	if(Read_AnalogMeter_Ambient_CaliData() < 0) return -6;
	if(Read_AnalogMeter_gas_CaliData() < 0) return -7;
	
/*	rtn = ComPortInitialize(myPs->config.comPort, myPs->config.comBps);
	if(rtn < 0) return -4;
	myPs->config.ttyS_fd = rtn;*/
	myData->AppControl.signal[APP_SIG_ANALOG_METER_PROCESS] = P1;
	
	if(myData->AppControl.config.debugType != P0) return 0;	//hun_211020
	
	//hun_211019
	if(myPs->config.countMeter > 0){
		myPs->signal[ANALOG_METER_SIG_MEASURE] = P1;
	}else if(myPs->config.ambientModuleCount > 0){
		myPs->signal[ANALOG_METER_SIG_MEASURE] = P100;
	}else if(myPs->config.gasModuleCount > 0){
		myPs->signal[ANALOG_METER_SIG_MEASURE] = P200;
	}

//	110621 oys w : For AnalogMeter Process Kill (restart)
	myData->serialCheckTime = myData->mData.misc.timer_1sec;
	return 0;
}

void AnalogMeter_Control(void)
{
	Check_Message();
	Check_Signal();
	if(myPs->misc.auto_cali_flag == P1){
		analog_cali_auto();
	}
}

void Check_Signal(void)
{
	int debug = 0;
	switch(myPs->signal[ANALOG_METER_SIG_INITIALIZE]) {
		case P1:
			send_cmd_initialize(1, 1, 1);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P2:
			send_cmd_initialize(2, 1, 1);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P3:
			send_cmd_initialize(1, 1, 2);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P4:
			send_cmd_initialize(2, 1, 2);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P5:
			send_cmd_initialize(1, 1, 3);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P6:
			send_cmd_initialize(2, 1, 3);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P7:
			send_cmd_initialize(1, 1, 4);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P8:
			send_cmd_initialize(2, 1, 4);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P9:
			send_cmd_initialize(1, 1, 5);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P10:
			send_cmd_initialize(2, 1, 5);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P11:
			send_cmd_initialize(1, 1, 6);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P12:
			send_cmd_initialize(2, 1, 6);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P13:
			send_cmd_initialize(1, 1, 7);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P14:
			send_cmd_initialize(2, 1, 7);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P15:
			send_cmd_initialize(1, 1, 8);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P16:
			send_cmd_initialize(2, 1, 8);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P17:		//170319
			send_cmd_initialize(1, 1, 9);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P18:
			send_cmd_initialize(2, 1, 9);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P19:
			send_cmd_initialize(1, 1, 10);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P20:		//170319
			send_cmd_initialize(2, 1, 10);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P21:
			send_cmd_initialize(1, 2, 1);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P22:
			send_cmd_initialize(2, 2, 1);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P23:
			send_cmd_initialize(1, 2, 2);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P24:
			send_cmd_initialize(2, 2, 2);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P25:
			send_cmd_initialize(1, 2, 3);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P26:
			send_cmd_initialize(2, 2, 3);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P27:
			send_cmd_initialize(1, 2, 4);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P28:
			send_cmd_initialize(2, 2, 4);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P29:
			send_cmd_initialize(1, 2, 5);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P30:
			send_cmd_initialize(2, 2, 5);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P31:
			send_cmd_initialize(1, 2, 6);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P32:
			send_cmd_initialize(2, 2, 6);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P33:
			send_cmd_initialize(1, 2, 7);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P34:
			send_cmd_initialize(2, 2, 7);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P35:
			send_cmd_initialize(1, 2, 8);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P36:
			send_cmd_initialize(2, 2, 8);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P37:		//170319 add
			send_cmd_initialize(1, 2, 9);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P38:		
			send_cmd_initialize(2, 2, 9);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P39:
			send_cmd_initialize(1, 2, 10);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P40:		//170319 add
			send_cmd_initialize(2, 2, 10);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P41:
			send_cmd_initialize(1, 3, 1);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P42:
			send_cmd_initialize(2, 3, 1);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P43:
			send_cmd_initialize(1, 3, 2);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P44:
			send_cmd_initialize(2, 3, 2);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P45:
			send_cmd_initialize(1, 3, 3);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P46:
			send_cmd_initialize(2, 3, 3);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P47:
			send_cmd_initialize(1, 3, 4);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P48:
			send_cmd_initialize(2, 3, 4);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P49:
			send_cmd_initialize(1, 3, 5);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P50:
			send_cmd_initialize(2, 3, 5);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P51:
			send_cmd_initialize(1, 3, 6);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P52:
			send_cmd_initialize(2, 3, 6);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P53:
			send_cmd_initialize(1, 3, 7);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P54:
			send_cmd_initialize(2, 3, 7);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P55:
			send_cmd_initialize(1, 3, 8);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P56:
			send_cmd_initialize(2, 3, 8);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P57:			//170319
			send_cmd_initialize(1, 3, 9);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P58:
			send_cmd_initialize(2, 3, 9);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P59:
			send_cmd_initialize(1, 3, 10);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P60:
			send_cmd_initialize(2, 3, 10);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P61:
			send_cmd_initialize(1, 4, 1);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P62:
			send_cmd_initialize(2, 4, 1);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P63:
			send_cmd_initialize(1, 4, 2);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P64:
			send_cmd_initialize(2, 4, 2);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P65:
			send_cmd_initialize(1, 4, 3);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P66:
			send_cmd_initialize(2, 4, 3);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P67:
			send_cmd_initialize(1, 4, 4);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P68:
			send_cmd_initialize(2, 4, 4);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P69:
			send_cmd_initialize(1, 4, 5);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P70:
			send_cmd_initialize(2, 4, 5);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P71:
			send_cmd_initialize(1, 4, 6);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P72:
			send_cmd_initialize(2, 4, 6);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P73:
			send_cmd_initialize(1, 4, 7);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P74:
			send_cmd_initialize(2, 4, 7);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P75:
			send_cmd_initialize(1, 4, 8);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P76:
			send_cmd_initialize(2, 4, 8);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P77:		//170319
			send_cmd_initialize(1, 4, 9);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P78:
			send_cmd_initialize(2, 4, 9);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P79:
			send_cmd_initialize(1, 4, 10);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P80:
			send_cmd_initialize(2, 4, 10);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P81:
			send_cmd_initialize(1, 5, 1);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P82:
			send_cmd_initialize(2, 5, 1);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P83:
			send_cmd_initialize(1, 5, 2);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P84:
			send_cmd_initialize(2, 5, 2);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P85:
			send_cmd_initialize(1, 5, 3);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P86:
			send_cmd_initialize(2, 5, 3);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P87:
			send_cmd_initialize(1, 5, 4);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P88:
			send_cmd_initialize(2, 5, 4);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P89:
			send_cmd_initialize(1, 5, 5);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P90:
			send_cmd_initialize(2, 5, 5);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P91:
			send_cmd_initialize(1, 5, 6);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P92:
			send_cmd_initialize(2, 5, 6);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P93:
			send_cmd_initialize(1, 5, 7);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P94:
			send_cmd_initialize(2, 5, 7);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P95:
			send_cmd_initialize(1, 5, 8);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P96:
			send_cmd_initialize(2, 5, 8);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P97:		//170319 add
			send_cmd_initialize(1, 5, 9);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P98:
			send_cmd_initialize(2, 5, 9);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P99:
			send_cmd_initialize(1, 5, 10);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P100:
			send_cmd_initialize(2, 5, 10);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P101:
			send_cmd_initialize(1, 6, 1);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P102:
			send_cmd_initialize(2, 6, 1);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P103:
			send_cmd_initialize(1, 6, 2);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P104:
			send_cmd_initialize(2, 6, 2);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P105:
			send_cmd_initialize(1, 6, 3);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P106:
			send_cmd_initialize(2, 6, 3);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P107:
			send_cmd_initialize(1, 6, 4);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P108:
			send_cmd_initialize(2, 6, 4);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P109:
			send_cmd_initialize(1, 6, 5);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P110:
			send_cmd_initialize(2, 6, 5);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P111:
			send_cmd_initialize(1, 6, 6);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P112:
			send_cmd_initialize(2, 6, 6);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P113:
			send_cmd_initialize(1, 6, 7);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P114:
			send_cmd_initialize(2, 6, 7);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P115:
			send_cmd_initialize(1, 6, 8);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P116:
			send_cmd_initialize(2, 6, 8);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P117:		//170319 add
			send_cmd_initialize(1, 6, 9);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P118:
			send_cmd_initialize(2, 6, 9);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P119:
			send_cmd_initialize(1, 6, 10);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P120:
			send_cmd_initialize(2, 6, 10);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P121:
			send_cmd_initialize(1, 7, 1);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P122:
			send_cmd_initialize(2, 7, 1);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P123:
			send_cmd_initialize(1, 7, 2);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P124:
			send_cmd_initialize(2, 7, 2);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P125:
			send_cmd_initialize(1, 7, 3);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P126:
			send_cmd_initialize(2, 7, 3);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P127:
			send_cmd_initialize(1, 7, 4);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P128:
			send_cmd_initialize(2, 7, 4);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P129:
			send_cmd_initialize(1, 7, 5);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P130:
			send_cmd_initialize(2, 7, 5);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P131:
			send_cmd_initialize(1, 7, 6);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P132:
			send_cmd_initialize(2, 7, 6);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P133:
			send_cmd_initialize(1, 7, 7);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P134:
			send_cmd_initialize(2, 7, 7);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P135:
			send_cmd_initialize(1, 7, 8);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P136:
			send_cmd_initialize(2, 7, 8);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P137:			//170319 add
			send_cmd_initialize(1, 7, 9);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P138:
			send_cmd_initialize(2, 7, 8);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P139:
			send_cmd_initialize(1, 7, 10);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P140:
			send_cmd_initialize(2, 7, 10);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P141:
			send_cmd_initialize(1, 8, 1);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P142:
			send_cmd_initialize(2, 8, 1);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P143:
			send_cmd_initialize(1, 8, 2);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P144:
			send_cmd_initialize(2, 8, 2);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P145:
			send_cmd_initialize(1, 8, 3);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P146:
			send_cmd_initialize(2, 8, 3);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P147:
			send_cmd_initialize(1, 8, 4);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P148:
			send_cmd_initialize(2, 8, 4);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P149:
			send_cmd_initialize(1, 8, 5);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P150:
			send_cmd_initialize(2, 8, 5);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P151:
			send_cmd_initialize(1, 8, 6);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P152:
			send_cmd_initialize(2, 8, 6);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P153:
			send_cmd_initialize(1, 8, 7);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P154:
			send_cmd_initialize(2, 8, 7);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P155:
			send_cmd_initialize(1, 8, 8);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P156:
			send_cmd_initialize(2, 8, 8);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P157:			//170319 add
			send_cmd_initialize(1, 8, 9);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P158:
			send_cmd_initialize(2, 8, 9);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P159:
			send_cmd_initialize(1, 8, 10);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P160:
			send_cmd_initialize(2, 8, 10);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P161:		//170319 add
			send_cmd_initialize(1, 9, 1);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P162:
			send_cmd_initialize(2, 9, 1);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P163:
			send_cmd_initialize(1, 9, 2);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P164:
			send_cmd_initialize(2, 9, 2);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P165:
			send_cmd_initialize(1, 9, 3);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P166:
			send_cmd_initialize(2, 9, 3);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P167:
			send_cmd_initialize(1, 9, 4);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P168:
			send_cmd_initialize(2, 9, 4);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P169:
			send_cmd_initialize(1, 9, 5);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P170:
			send_cmd_initialize(2, 9, 5);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P171:
			send_cmd_initialize(1, 9, 6);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P172:
			send_cmd_initialize(2, 9, 6);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P173:
			send_cmd_initialize(1, 9, 7);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P174:
			send_cmd_initialize(2, 9, 7);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P175:
			send_cmd_initialize(1, 9, 8);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P176:
			send_cmd_initialize(2, 9, 8);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P177:			
			send_cmd_initialize(1, 9, 9);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P178:
			send_cmd_initialize(2, 9, 9);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P179:
			send_cmd_initialize(1, 9, 10);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P180:
			send_cmd_initialize(2, 9, 10);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P181:		//170319 add
			send_cmd_initialize(1, 10, 1);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P182:
			send_cmd_initialize(2, 10, 1);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P183:
			send_cmd_initialize(1, 10, 2);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P184:
			send_cmd_initialize(2, 10, 2);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P185:
			send_cmd_initialize(1, 10, 3);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P186:
			send_cmd_initialize(2, 10, 3);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P187:
			send_cmd_initialize(1, 10, 4);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P188:
			send_cmd_initialize(2, 10, 4);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P189:
			send_cmd_initialize(1, 10, 5);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P190:
			send_cmd_initialize(2, 10, 5);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P191:
			send_cmd_initialize(1, 10, 6);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P192:
			send_cmd_initialize(2, 10, 6);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P193:
			send_cmd_initialize(1, 10, 7);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P194:
			send_cmd_initialize(2, 10, 7);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P195:
			send_cmd_initialize(1, 10, 8);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P196:
			send_cmd_initialize(2, 10, 8);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P197:			
			send_cmd_initialize(1, 10, 9);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P198:
			send_cmd_initialize(2, 10, 9);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		case P199:
			send_cmd_initialize(1, 10, 10);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE]++;
			break;
		case P200:
			send_cmd_initialize(2, 10, 10);
			myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P0;
			break;
		default:	break;
	}

	if(myPs->config.functionType == 0) {
		switch(myPs->signal[ANALOG_METER_SIG_MEASURE]) {
			case P1:
				myPs->signal[ANALOG_METER_SIG_MEASURE] = P11;
				send_cmd_request(0, 1);
				break;
			case P2:
				myPs->signal[ANALOG_METER_SIG_MEASURE] = P12;
				send_cmd_request(0, 2);
				break;
			case P3:
				myPs->signal[ANALOG_METER_SIG_MEASURE] = P13;
				send_cmd_request(0, 3);
				break;
			case P4:
				myPs->signal[ANALOG_METER_SIG_MEASURE] = P14;
				send_cmd_request(0, 4);
				break;
			case P5:
				myPs->signal[ANALOG_METER_SIG_MEASURE] = P15;
				send_cmd_request(0, 5);
				break;
			case P6:
				myPs->signal[ANALOG_METER_SIG_MEASURE] = P16;
				send_cmd_request(0, 6);
				break;
			case P7:
				myPs->signal[ANALOG_METER_SIG_MEASURE] = P17;
				send_cmd_request(0, 7);
				break;
			case P8:
				myPs->signal[ANALOG_METER_SIG_MEASURE] = P18;
				send_cmd_request(0, 8);
				break;
			case P9:		//170319 add
				myPs->signal[ANALOG_METER_SIG_MEASURE] = P19;
				send_cmd_request(0, 9);
				break;
			case P10:		//170319 add
				myPs->signal[ANALOG_METER_SIG_MEASURE] = P20;
				send_cmd_request(0, 10);
				break;

			case P11:
				myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] += 1;
				if(myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] >= 3) {
//					userlog(DEBUG_LOG, psName, "measure error : %d, %d\n",
//						(int)myPs->signal[ANALOG_METER_SIG_MEASURE],
//						(int)myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR]);
					memset((char *)&myPs->rcvPacket, 0,
						sizeof(S_ANALOG_METER_RCV_PACKET));
					memset((char *)&myPs->rcvCmd, 0,
						sizeof(S_ANALOG_METER_RCV_COMMAND));
					myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] = 0;
					//myPs->signal[ANALOG_METER_SIG_MEASURE] = P1;
					//module read error next module request
					//120927 kji
					next_module_request();
				}
				break;
			case P12:
				myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] += 1;
				if(myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] >= 3) {
//					userlog(DEBUG_LOG, psName, "measure error : %d, %d\n",
//						(int)myPs->signal[ANALOG_METER_SIG_MEASURE],
//						(int)myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR]);
					memset((char *)&myPs->rcvPacket, 0,
						sizeof(S_ANALOG_METER_RCV_PACKET));
					memset((char *)&myPs->rcvCmd, 0,
						sizeof(S_ANALOG_METER_RCV_COMMAND));
					myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] = 0;
				//	myPs->signal[ANALOG_METER_SIG_MEASURE] = P2;
					//module read error next module request
					//120927 kji
					next_module_request();
				}
				break;
			case P13:
				myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] += 1;
				if(myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] >= 3) {
//					userlog(DEBUG_LOG, psName, "measure error : %d, %d\n",
//						(int)myPs->signal[ANALOG_METER_SIG_MEASURE],
//						(int)myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR]);
					memset((char *)&myPs->rcvPacket, 0,
						sizeof(S_ANALOG_METER_RCV_PACKET));
					memset((char *)&myPs->rcvCmd, 0,
						sizeof(S_ANALOG_METER_RCV_COMMAND));
					myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] = 0;
				//	myPs->signal[ANALOG_METER_SIG_MEASURE] = P3;
					//module read error next module request
					//120927 kji
					next_module_request();
				}
				break;
			case P14:
				myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] += 1;
				if(myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] >= 3) {
//					userlog(DEBUG_LOG, psName, "measure error : %d, %d\n",
//						(int)myPs->signal[ANALOG_METER_SIG_MEASURE],
//						(int)myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR]);
					memset((char *)&myPs->rcvPacket, 0,
						sizeof(S_ANALOG_METER_RCV_PACKET));
					memset((char *)&myPs->rcvCmd, 0,
						sizeof(S_ANALOG_METER_RCV_COMMAND));
					myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] = 0;
				//	myPs->signal[ANALOG_METER_SIG_MEASURE] = P4;
					//module read error next module request
					//120927 kji
					next_module_request();
				}
				break;
			case P15:
				myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] += 1;
				if(myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] >= 3) {
//					userlog(DEBUG_LOG, psName, "measure error : %d, %d\n",
//						(int)myPs->signal[ANALOG_METER_SIG_MEASURE],
//						(int)myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR]);
					memset((char *)&myPs->rcvPacket, 0,
						sizeof(S_ANALOG_METER_RCV_PACKET));
					memset((char *)&myPs->rcvCmd, 0,
						sizeof(S_ANALOG_METER_RCV_COMMAND));
					myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] = 0;
				//	myPs->signal[ANALOG_METER_SIG_MEASURE] = P5;
					//module read error next module request
					//120927 kji
					next_module_request();
				}
				break;
			case P16:
				myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] += 1;
				if(myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] >= 3) {
//					userlog(DEBUG_LOG, psName, "measure error : %d, %d\n",
//						(int)myPs->signal[ANALOG_METER_SIG_MEASURE],
//						(int)myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR]);
					memset((char *)&myPs->rcvPacket, 0,
						sizeof(S_ANALOG_METER_RCV_PACKET));
					memset((char *)&myPs->rcvCmd, 0,
						sizeof(S_ANALOG_METER_RCV_COMMAND));
					myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] = 0;
			//		myPs->signal[ANALOG_METER_SIG_MEASURE] = P6;
					//module read error next module request
					//120927 kji
					next_module_request();
				}
				break;
			case P17:
				myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] += 1;
				if(myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] >= 3) {
//					userlog(DEBUG_LOG, psName, "measure error : %d, %d\n",
//						(int)myPs->signal[ANALOG_METER_SIG_MEASURE],
//						(int)myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR]);
					memset((char *)&myPs->rcvPacket, 0,
						sizeof(S_ANALOG_METER_RCV_PACKET));
					memset((char *)&myPs->rcvCmd, 0,
						sizeof(S_ANALOG_METER_RCV_COMMAND));
					myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] = 0;
				//	myPs->signal[ANALOG_METER_SIG_MEASURE] = P7;
					//module read error next module request
					//120927 kji
					next_module_request();
				}
				break;
			case P18:
				myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] += 1;
				if(myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] >= 3) {
//					userlog(DEBUG_LOG, psName, "measure error : %d, %d\n",
//						(int)myPs->signal[ANALOG_METER_SIG_MEASURE],
//						(int)myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR]);
					memset((char *)&myPs->rcvPacket, 0,
						sizeof(S_ANALOG_METER_RCV_PACKET));
					memset((char *)&myPs->rcvCmd, 0,
						sizeof(S_ANALOG_METER_RCV_COMMAND));
					myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] = 0;
				//	myPs->signal[ANALOG_METER_SIG_MEASURE] = P8;
					//module read error next module request
					//120927 kji
					next_module_request();
				}
				break;
			case P19:			//170319 add
				myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] += 1;
				if(myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] >= 3) {
//					userlog(DEBUG_LOG, psName, "measure error : %d, %d\n",
//						(int)myPs->signal[ANALOG_METER_SIG_MEASURE],
//						(int)myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR]);
					memset((char *)&myPs->rcvPacket, 0,
						sizeof(S_ANALOG_METER_RCV_PACKET));
					memset((char *)&myPs->rcvCmd, 0,
						sizeof(S_ANALOG_METER_RCV_COMMAND));
					myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] = 0;
				//	myPs->signal[ANALOG_METER_SIG_MEASURE] = P8;
					//module read error next module request
					//120927 kji
					next_module_request();
				}
				break;
			case P20:			//170319 add
				myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] += 1;
				if(myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] >= 3) {
//					userlog(DEBUG_LOG, psName, "measure error : %d, %d\n",
//						(int)myPs->signal[ANALOG_METER_SIG_MEASURE],
//						(int)myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR]);
					memset((char *)&myPs->rcvPacket, 0,
						sizeof(S_ANALOG_METER_RCV_PACKET));
					memset((char *)&myPs->rcvCmd, 0,
						sizeof(S_ANALOG_METER_RCV_COMMAND));
					myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] = 0;
				//	myPs->signal[ANALOG_METER_SIG_MEASURE] = P8;
					//module read error next module request
					//120927 kji
					next_module_request();
				}
				break;
			case P100:	//ambient Temp
				myPs->signal[ANALOG_METER_SIG_MEASURE] = P110;
				send_cmd_request(0, (int)myPs->config.ambientModuleNo);
				break;
			case P101:	//ambient Temp
				myPs->signal[ANALOG_METER_SIG_MEASURE] = P111;
				send_cmd_request(0, (int)myPs->config.ambientModuleNo+1);
				break;
			case P110:			
				myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] += 1;
				if(myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] >= 3) {
//					userlog(DEBUG_LOG, psName, "measure error : %d, %d\n",
//						(int)myPs->signal[ANALOG_METER_SIG_MEASURE],
//						(int)myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR]);
					memset((char *)&myPs->rcvPacket, 0,
						sizeof(S_ANALOG_METER_RCV_PACKET));
					memset((char *)&myPs->rcvCmd, 0,
						sizeof(S_ANALOG_METER_RCV_COMMAND));
					myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] = 0;
				//	myPs->signal[ANALOG_METER_SIG_MEASURE] = P8;
					//module read error next module request
					//120927 kji
					next_module_request();
				}
				break;
			case P111:			
				myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] += 1;
				if(myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] >= 3) {
//					userlog(DEBUG_LOG, psName, "measure error : %d, %d\n",
//						(int)myPs->signal[ANALOG_METER_SIG_MEASURE],
//						(int)myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR]);
					memset((char *)&myPs->rcvPacket, 0,
						sizeof(S_ANALOG_METER_RCV_PACKET));
					memset((char *)&myPs->rcvCmd, 0,
						sizeof(S_ANALOG_METER_RCV_COMMAND));
					myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] = 0;
				//	myPs->signal[ANALOG_METER_SIG_MEASURE] = P8;
					//module read error next module request
					//120927 kji
					next_module_request();
				}
				break;
			case P200:	//gas
				myPs->signal[ANALOG_METER_SIG_MEASURE] = P205;
				send_cmd_request(0, 45);
				break;
			case P201:	//gas
				myPs->signal[ANALOG_METER_SIG_MEASURE] = P206;
				send_cmd_request(0, 45);
				break;
			case P205:	//hun_211019
				myPs->signal[ANALOG_METER_COUNT] += 1;
				if(myPs->signal[ANALOG_METER_COUNT] >= 2){
					myPs->signal[ANALOG_METER_COUNT] = 0;
					myPs->signal[ANALOG_METER_SIG_MEASURE] = P210;
					send_cmd_request(0, (int)myPs->config.gasModuleNo);
					//send_cmd_request(0, 41);
				}
				break;
			case P206:	//hun_211019
				myPs->signal[ANALOG_METER_COUNT] += 1;
				if(myPs->signal[ANALOG_METER_COUNT] >= 2){
					myPs->signal[ANALOG_METER_COUNT] = 0;
					myPs->signal[ANALOG_METER_SIG_MEASURE] = P211;
					send_cmd_request(0, (int)myPs->config.gasModuleNo+1);
					//send_cmd_request(0, 42);
				}
				break;
			case P210:		
				myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] += 1;
				if(myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] >= 3) {
//					userlog(DEBUG_LOG, psName, "measure error : %d, %d\n",
//						(int)myPs->signal[ANALOG_METER_SIG_MEASURE],
//						(int)myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR]);
					memset((char *)&myPs->rcvPacket, 0,
						sizeof(S_ANALOG_METER_RCV_PACKET));
					memset((char *)&myPs->rcvCmd, 0,
						sizeof(S_ANALOG_METER_RCV_COMMAND));
					myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] = 0;
				//	myPs->signal[ANALOG_METER_SIG_MEASURE] = P8;
					//module read error next module request
					//120927 kji
					next_module_request();
				}
				break;
			case P211:		
				myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] += 1;
				if(myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] >= 3) {
//					userlog(DEBUG_LOG, psName, "measure error : %d, %d\n",
//						(int)myPs->signal[ANALOG_METER_SIG_MEASURE],
//						(int)myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR]);
					memset((char *)&myPs->rcvPacket, 0,
						sizeof(S_ANALOG_METER_RCV_PACKET));
					memset((char *)&myPs->rcvCmd, 0,
						sizeof(S_ANALOG_METER_RCV_COMMAND));
					myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] = 0;
				//	myPs->signal[ANALOG_METER_SIG_MEASURE] = P8;
					//module read error next module request
					//120927 kji
					next_module_request();
				}
				break;
			default:	break;
		}
	} else {
		switch(myPs->signal[ANALOG_METER_SIG_MEASURE]) {
			case P1:
				send_cmd_open(1);
				myPs->signal[ANALOG_METER_SIG_MEASURE] = P11;
				break;
			case P2:
				send_cmd_open(2);
				myPs->signal[ANALOG_METER_SIG_MEASURE] = P12;
				break;
			case P3:
				send_cmd_open(3);
				myPs->signal[ANALOG_METER_SIG_MEASURE] = P13;
				break;
			case P4:
				send_cmd_open(4);
				myPs->signal[ANALOG_METER_SIG_MEASURE] = P14;
				break;
			case P5:
				send_cmd_open(5);
				myPs->signal[ANALOG_METER_SIG_MEASURE] = P15;
				break;
			case P6:
				send_cmd_open(6);
				myPs->signal[ANALOG_METER_SIG_MEASURE] = P16;
				break;
			case P7:
				send_cmd_open(7);
				myPs->signal[ANALOG_METER_SIG_MEASURE] = P17;
				break;
			case P8:
				send_cmd_open(8);
				myPs->signal[ANALOG_METER_SIG_MEASURE] = P18;
				break;
			case P9:		//170319 add
				send_cmd_open(9);
				myPs->signal[ANALOG_METER_SIG_MEASURE] = P19;
				break;
			case P10:		//170319 add
				send_cmd_open(10);
				myPs->signal[ANALOG_METER_SIG_MEASURE] = P20;
				break;

			case P11:
			case P12:
			case P13:
			case P14:
			case P15:
			case P16:
			case P17:
			case P18:
			case P19:		//170319 add
			case P20:
				myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] += 1;
				if(myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] >= 3) {
					if(debug == 1) {
						userlog(DEBUG_LOG, psName, "measure error : %d, %d\n",
							(int)myPs->signal[ANALOG_METER_SIG_MEASURE],
							(int)myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR]);
					}
					memset((char *)&myPs->rcvPacket, 0,
						sizeof(S_ANALOG_METER_RCV_PACKET));
					memset((char *)&myPs->rcvCmd, 0,
						sizeof(S_ANALOG_METER_RCV_COMMAND));
					myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] = 0;
					myPs->signal[ANALOG_METER_SIG_MEASURE] -= P10;
				}
				break;
			case P21:
				send_cmd_request(0, 1);
				myPs->signal[ANALOG_METER_SIG_MEASURE] = P31;
				break;
			case P22:
				send_cmd_request(0, 2);
				myPs->signal[ANALOG_METER_SIG_MEASURE] = P32;
				break;
			case P23:
				send_cmd_request(0, 3);
				myPs->signal[ANALOG_METER_SIG_MEASURE] = P33;
				break;
			case P24:
				send_cmd_request(0, 4);
				myPs->signal[ANALOG_METER_SIG_MEASURE] = P34;
				break;
			case P25:
				send_cmd_request(0, 5);
				myPs->signal[ANALOG_METER_SIG_MEASURE] = P35;
				break;
			case P26:
				send_cmd_request(0, 6);
				myPs->signal[ANALOG_METER_SIG_MEASURE] = P36;
				break;
			case P27:
				send_cmd_request(0, 7);
				myPs->signal[ANALOG_METER_SIG_MEASURE] = P37;
				break;
			case P28:
				send_cmd_request(0, 8);
				myPs->signal[ANALOG_METER_SIG_MEASURE] = P38;
				break;
			case P29:		//170319 add
				send_cmd_request(0, 9);
				myPs->signal[ANALOG_METER_SIG_MEASURE] = P39;
				break;
			case P30:		//170319 add
				send_cmd_request(0, 10);
				myPs->signal[ANALOG_METER_SIG_MEASURE] = P40;
				break;

			case P31:
			case P32:
			case P33:
			case P34:
			case P35:
			case P36:
			case P37:
			case P38:
			case P39:		//170319 add
			case P40:
				myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] += 1;
				if(myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] >= 3) {
					if(debug == 1) {
						userlog(DEBUG_LOG, psName, "measure error : %d, %d\n",
							(int)myPs->signal[ANALOG_METER_SIG_MEASURE],
							(int)myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR]);
					}
					memset((char *)&myPs->rcvPacket, 0,
						sizeof(S_ANALOG_METER_RCV_PACKET));
					memset((char *)&myPs->rcvCmd, 0,
						sizeof(S_ANALOG_METER_RCV_COMMAND));
					myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] = 0;
					myPs->signal[ANALOG_METER_SIG_MEASURE] -= P10;
				}
				break;
			case P41:
				send_cmd_close(1);
				myPs->signal[ANALOG_METER_SIG_MEASURE] = P51;
				break;
			case P42:
				send_cmd_close(2);
				myPs->signal[ANALOG_METER_SIG_MEASURE] = P52;
				break;
			case P43:
				send_cmd_close(3);
				myPs->signal[ANALOG_METER_SIG_MEASURE] = P53;
				break;
			case P44:
				send_cmd_close(4);
				myPs->signal[ANALOG_METER_SIG_MEASURE] = P54;
				break;
			case P45:
				send_cmd_close(5);
				myPs->signal[ANALOG_METER_SIG_MEASURE] = P55;
				break;
			case P46:
				send_cmd_close(6);
				myPs->signal[ANALOG_METER_SIG_MEASURE] = P56;
				break;
			case P47:
				send_cmd_close(7);
				myPs->signal[ANALOG_METER_SIG_MEASURE] = P57;
				break;
			case P48:
				send_cmd_close(8);
				myPs->signal[ANALOG_METER_SIG_MEASURE] = P58;
				break;
			case P49:			//170319 add
				send_cmd_close(9);
				myPs->signal[ANALOG_METER_SIG_MEASURE] = P58;
				break;
			case P50:			//170319 add
				send_cmd_close(10);
				myPs->signal[ANALOG_METER_SIG_MEASURE] = P58;
				break;

			case P51:
			case P52:
			case P53:
			case P54:
			case P55:
			case P56:
			case P57:
			case P58:
				myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] += 1;
				if(myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] >= 3) {
					if(debug == 1) {
						userlog(DEBUG_LOG, psName, "measure error : %d, %d\n",
							(int)myPs->signal[ANALOG_METER_SIG_MEASURE],
							(int)myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR]);
					}
					memset((char *)&myPs->rcvPacket, 0,
						sizeof(S_ANALOG_METER_RCV_PACKET));
					memset((char *)&myPs->rcvCmd, 0,
						sizeof(S_ANALOG_METER_RCV_COMMAND));
					myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] = 0;
					myPs->signal[ANALOG_METER_SIG_MEASURE] -= P10;
				}
				break;
			/*case P61:
				send_cmd_close(1);
				myPs->signal[ANALOG_METER_SIG_MEASURE] = P71;
				break;
			case P62:
				send_cmd_close(2);
				myPs->signal[ANALOG_METER_SIG_MEASURE] = P72;
				break;
			case P63:
				send_cmd_close(3);
				myPs->signal[ANALOG_METER_SIG_MEASURE] = P73;
				break;
			case P64:
				send_cmd_close(4);
				myPs->signal[ANALOG_METER_SIG_MEASURE] = P74;
				break;
			case P71:
			case P72:
			case P73:
			case P74:
				myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] += 1;
				if(myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] >= 3) {
					memset((char *)&myPs->rcvPacket, 0,
						sizeof(S_ANALOG_METER_RCV_PACKET));
					memset((char *)&myPs->rcvCmd, 0,
						sizeof(S_ANALOG_METER_RCV_COMMAND));
					myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] = 0;
					myPs->signal[ANALOG_METER_SIG_MEASURE] -= P10;
				}
				break;*/
			default:	break;
		}
	}
}

void Close_Process(void)
{
	if(myPs->config.ttyS_fd > 0) {
		if(closetty(myPs->config.ttyS_fd) < 0) {
			userlog(DEBUG_LOG, psName, "ttyS %d close error\n",
				myPs->config.ttyS_fd);
		}
	}
	
	myData->AppControl.signal[APP_SIG_ANALOG_METER_PROCESS] = P3;	

	Close_SystemMemory();
}
