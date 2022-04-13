#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../../INC/datastore.h"
#include "userlog.h"
#include "serial.h"
#include "message.h"
#include "cali.h"
#include "local_utils.h"

extern volatile S_SYSTEM_DATA *myData;
extern volatile S_ANALOG_METER  *myPs;
extern char psName[PROCESS_NAME_SIZE];

void Check_Message(void)
{
	rcv_msg(APP_TO_METER2);
	rcv_msg(MAIN_TO_METER2);
	rcv_msg(MODULE_TO_METER2);
}

void rcv_msg(int fromPs)
{
	int idx, msg, ch, val, rtn;
	
	if(myData->msg[fromPs].write_idx == myData->msg[fromPs].read_idx)
		return;

	myData->msg[fromPs].read_idx++;
	if(myData->msg[fromPs].read_idx >= MAX_MSG)
		myData->msg[fromPs].read_idx = 0;
	
	idx = myData->msg[fromPs].read_idx;
	msg = myData->msg[fromPs].msg_val[idx].msg;
	ch = myData->msg[fromPs].msg_val[idx].ch;
	val = myData->msg[fromPs].msg_val[idx].val;
	
	switch(fromPs) {
		case APP_TO_METER2:
			rtn = msgParsing_App_to_Meter2(msg, ch, val, idx);
			break;
		case MAIN_TO_METER2:
			rtn = msgParsing_Main_to_Meter2(msg, ch, val, idx);
			break;
		case MODULE_TO_METER2:
			rtn = msgParsing_Module_to_Meter2(msg, ch, val, idx);
			break;
		default:
			userlog(DEBUG_LOG, psName,
				"Msg Direction UnKnown : %d %d %d\n", msg, ch, val);
			break;
	}
}

int msgParsing_App_to_Meter2(int msg, int ch, int val, int idx)
{
	int rtn=0 ;

	switch(msg) {
		case MSG_APP_METER2_INITIALIZE:
			if(ch == 1) {
				if(val == 1) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P1;
				} else if(val == 2) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P3;
				} else if(val == 3) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P5;
				} else if(val == 4) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P7;
				} else if(val == 5) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P9;
				} else if(val == 6) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P11;
				} else if(val == 7) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P13;
				} else if(val == 8) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P15;
				} else if(val == 9) {	//170319 add
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P17;
				} else if(val == 10) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P19;
				}
			} else if(ch == 2) {
				if(val == 1) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P21;
				} else if(val == 2) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P23;
				} else if(val == 3) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P25;
				} else if(val == 4) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P27;
				} else if(val == 5) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P29;
				} else if(val == 6) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P31;
				} else if(val == 7) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P33;
				} else if(val == 8) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P35;
				} else if(val == 9) {	//170319 add
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P37;
				} else if(val == 10) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P39;
				}

			} else if(ch == 3) {
				if(val == 1) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P41;
				} else if(val == 2) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P43;
				} else if(val == 3) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P45;
				} else if(val == 4) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P47;
				} else if(val == 5) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P49;
				} else if(val == 6) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P51;
				} else if(val == 7) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P53;
				} else if(val == 8) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P55;
				} else if(val == 9) {	//170319 add
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P57;
				} else if(val == 10) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P59;
				}
			} else if(ch == 4) {
				if(val == 1) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P61;
				} else if(val == 2) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P63;
				} else if(val == 3) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P65;
				} else if(val == 4) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P67;
				} else if(val == 5) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P69;
				} else if(val == 6) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P71;
				} else if(val == 7) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P73;
				} else if(val == 8) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P75;
				} else if(val == 9) {		//170319
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P77;
				} else if(val == 10) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P79;
				}
			} else if(ch == 5) {
				if(val == 1) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P81;
				} else if(val == 2) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P83;
				} else if(val == 3) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P85;
				} else if(val == 4) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P87;
				} else if(val == 5) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P89;
				} else if(val == 6) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P91;
				} else if(val == 7) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P93;
				} else if(val == 8) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P95;
				} else if(val == 9) {	//170319 add
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P97;
				} else if(val == 10) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P99;
				}
			} else if(ch == 6) {
				if(val == 1) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P101;
				} else if(val == 2) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P103;
				} else if(val == 3) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P105;
				} else if(val == 4) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P107;
				} else if(val == 5) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P109;
				} else if(val == 6) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P111;
				} else if(val == 7) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P113;
				} else if(val == 8) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P115;
				} else if(val == 9) {	//170319
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P117;
				} else if(val == 10) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P119;
				}
			} else if(ch == 7) {
				if(val == 1) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P121;
				} else if(val == 2) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P123;
				} else if(val == 3) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P125;
				} else if(val == 4) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P127;
				} else if(val == 5) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P129;
				} else if(val == 6) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P131;
				} else if(val == 7) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P133;
				} else if(val == 8) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P135;
				} else if(val == 9) {	//170319
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P137;
				} else if(val == 10) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P139;
				}
			} else if(ch == 8) {
				if(val == 1) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P141;
				} else if(val == 2) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P143;
				} else if(val == 3) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P145;
				} else if(val == 4) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P147;
				} else if(val == 5) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P149;
				} else if(val == 6) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P151;
				} else if(val == 7) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P153;
				} else if(val == 8) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P155;
				} else if(val == 9) {	//170319
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P157;
				} else if(val == 10) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P159;
				}
			} else if(ch == 9) {		//170319
				if(val == 1) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P161;
				} else if(val == 2) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P163;
				} else if(val == 3) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P165;
				} else if(val == 4) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P167;
				} else if(val == 5) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P169;
				} else if(val == 6) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P171;
				} else if(val == 7) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P173;
				} else if(val == 8) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P175;
				} else if(val == 9) {	//170319
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P177;
				} else if(val == 10) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P179;
				}
			} else if(ch == 10) {
				if(val == 1) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P181;
				} else if(val == 2) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P183;
				} else if(val == 3) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P185;
				} else if(val == 4) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P187;
				} else if(val == 5) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P189;
				} else if(val == 6) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P191;
				} else if(val == 7) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P193;
				} else if(val == 8) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P195;
				} else if(val == 9) {	//170319
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P197;
				} else if(val == 10) {
					myPs->signal[ANALOG_METER_SIG_INITIALIZE] = P199;
				}
			}
			break;
		case MSG_APP_METER2_REQUEST:
			send_cmd_request(ch, val);
			break;
		case MSG_APP_METER2_MEASURE:
			if(ch == 0) {
				if(val == 1) {
					myPs->signal[ANALOG_METER_SIG_MEASURE] = P1;
				} else if(val == 2) {
					myPs->signal[ANALOG_METER_SIG_MEASURE] = P2;
				} else if(val == 3) {
					myPs->signal[ANALOG_METER_SIG_MEASURE] = P3;
				} else if(val == 4) {
					myPs->signal[ANALOG_METER_SIG_MEASURE] = P4;
				//2011 3 14 pjy add
				} else if(val == 5) {
					myPs->signal[ANALOG_METER_SIG_MEASURE] = P5;
				} else if(val == 6) {
					myPs->signal[ANALOG_METER_SIG_MEASURE] = P6;
				} else if(val == 7) {
					myPs->signal[ANALOG_METER_SIG_MEASURE] = P7;
				} else if(val == 8) {
					myPs->signal[ANALOG_METER_SIG_MEASURE] = P8;
				} else if(val == 9) {	//170319
					myPs->signal[ANALOG_METER_SIG_MEASURE] = P9;
				} else if(val == 10) {
					myPs->signal[ANALOG_METER_SIG_MEASURE] = P10;
				}
			} else {
				myPs->signal[ANALOG_METER_SIG_MEASURE] = P0;
			}
			break;
		case MSG_APP_METER2_CALI_L:
			analog_cali_read(ch, val, 0);
			break;
		case MSG_APP_METER2_CALI_H:
			analog_cali_read(ch, val, 1);
			break;
		case MSG_APP_METER2_CALI_WRITE:
			analog_cali_calc(ch);
			break;
		case MSG_APP_METER2_CALI_UPDATE:
			analog_cali_update();
			break;
		case MSG_APP_METER2_CALI_AUTO:
			myPs->misc.auto_cali_flag = P1;
			myPs->misc.phase = P1;
			myPs->misc.normal_count = val;
			break;
		default:
			userlog(DEBUG_LOG, psName,
				"App to Meter2 Msg Unknown : %d %d %d\n", msg, ch, val);
			break;
	}
	return rtn;
}

int msgParsing_Module_to_Meter2(msg, ch, val, idx)
{		
	int rtn;

	switch(msg) {
#ifdef _TEMP_CALI  
		case MSG_MODULE_METER2_WRITE_TEMP_CALI_DATA:
			userlog(DEBUG_LOG, psName, "Module To Meter2 Temp Write\n");
			Write_AnalogMeter_CaliData_2();
			break;
#endif
		default:
			userlog(DEBUG_LOG, psName,
				"Main to Meter2 Msg Unknown : %d %d %d\n", msg, ch, val);
			break;
	}
	return rtn;		
}

int msgParsing_Main_to_Meter2(msg, ch, val, idx)
{		
	int rtn;
#ifdef _TEMP_CALI  
	int complete_flag;
#endif

	switch(msg) {
#ifdef _TEMP_CALI  
		case MSG_MAIN_METER2_WRITE_TEMP_CALI_DATA:
			Write_AnalogMeter_CaliData_2();
			break;
		case MSG_MAIN_METER2_TEMP_CALI_START:
			myPs->temp_cali.signal[ANALOG_METER_SIG_CALI_NORMAL] = P1;
			myPs->temp_cali.pointNo = val;
			userlog(DEBUG_LOG, psName, "TEMP_CALI_START sig[%d] pointNo[%d]\n", 
			myPs->temp_cali.signal[ANALOG_METER_SIG_CALI_NORMAL],
			myPs->temp_cali.pointNo);
			break;
		case MSG_MAIN_METER2_TEMP_CALI_BACKUP_READ:
			rtn = Read_AnalogMeter_CaliData_2();
			if(rtn < 0) {
				userlog(DEBUG_LOG, psName, "CALI_TEMP Read error %d\n", rtn);
				complete_flag = 0; //NG
			} else {
				userlog(DEBUG_LOG, psName, "CALI_TEMP Read Success %d\n", rtn);
				complete_flag = 1; //OK
				send_msg(METER2_TO_MAIN,
				  MSG_METER2_MAIN_TEMP_CALI_BACKUP_READ_REPLY,0, complete_flag);
			}	
			break;
		case MSG_MAIN_METER2_TEMP_CALI_CLEAR:
			rtn = Write_AnalogMeter_CaliData_Default();
			if(rtn < 0) {
				userlog(DEBUG_LOG, psName, "CALI_T Clear Fail %d\n", rtn);
			} else {
				userlog(DEBUG_LOG, psName, "CALI_T Clear Success %d\n", rtn);
			}
			if(myData->mData.config.installedTemp > 100){
				rtn = Write_AnalogMeter2_CaliData_Default();
				if(rtn < 0) {
					userlog(DEBUG_LOG, psName, "CALI_T_2 Clear Fail %d\n", rtn);
				} else {
					userlog(DEBUG_LOG, psName, "CALI_T_2 Clear Success %d\n", rtn);
				}
			}	
			break;
		
#endif
		default:
			userlog(DEBUG_LOG, psName,
				"Main to Meter2 Msg Unknown : %d %d %d\n", msg, ch, val);
			break;
	}
	return rtn;		
}

void send_msg(int toPs, int msg, int ch, int val)
{
	int idx;
	
	idx = myData->msg[toPs].write_idx;
	idx++;
	if(idx >= MAX_MSG) idx = 0;
	
	myData->msg[toPs].msg_val[idx].msg = msg;
	myData->msg[toPs].msg_val[idx].ch = ch;
	myData->msg[toPs].msg_val[idx].val = val;
	myData->msg[toPs].write_idx = idx;
}

void send_msg_ch_flag(int toPs, char *ch_flag)
{
	int idx;
	
	idx = myData->msg[toPs].write_idx;
	idx++;
	if(idx >= MAX_MSG) idx = 0;
	
	memcpy((char *)&myData->msg[toPs].msg_ch_flag[idx].bit_32[0],
		(char *)ch_flag, sizeof(S_MSG_CH_FLAG));
}
