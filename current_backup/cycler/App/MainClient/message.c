#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../../INC/datastore.h"
#include "userlog.h"
#include "network.h"
#include "message.h"

extern volatile S_SYSTEM_DATA *myData;
extern volatile S_MAIN_CLIENT *myPs;
extern char psName[PROCESS_NAME_SIZE];

void Check_Message(void)
{
	rcv_msg(APP_TO_MAIN);
	rcv_msg(MODULE_TO_MAIN);
	rcv_msg(DATASAVE_TO_MAIN);
	rcv_msg(METER_TO_MAIN);
	rcv_msg(METER2_TO_MAIN);
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
		case APP_TO_MAIN:
			rtn = msgParsing_App_to_Main(msg, ch, val, idx);
			break;
		case MODULE_TO_MAIN:
			rtn = msgParsing_Module_to_Main(msg, ch, val, idx);
			break;
		case DATASAVE_TO_MAIN:
			rtn = msgParsing_DataSave_to_Main(msg, ch, val, idx);
			break;
		case METER_TO_MAIN:
			rtn = msgParsing_Meter_to_Main(msg, ch, val, idx);
			break;
		case METER2_TO_MAIN:
			rtn = msgParsing_Meter2_to_Main(msg, ch, val, idx);
			break;
		default:
			userlog(DEBUG_LOG, psName,
				"Msg Direction UnKnown : %d %d %d\n", msg, ch, val);
			break;
	}
}

int msgParsing_App_to_Main(int msg, int ch, int val, int idx)
{
	int rtn=0;

	switch(msg) {
		default:
			userlog(DEBUG_LOG, psName,
				"App to Main Msg Unknown : %d %d %d\n", msg, ch, val);
			break;
	}
	return rtn;
}

int msgParsing_Module_to_Main(int msg, int ch, int val, int idx)
{
	int rtn=0;

	switch(msg) {
		case MSG_MODULE_MAIN_EMG_STATUS:
			send_cmd_emg_status(ch, val);
			break;
		case MSG_MODULE_MAIN_JIG_STATUS:
			send_cmd_jig_status();
			break;
		//160510 oys add
		case MSG_MODULE_MAIN_STEP_COND_UPDATE:
			send_cmd_step_cond_update(ch, val);
			break;
		//add end
//#ifdef __LG_VER1__ hun_210929
		//20180717 sch add
		case MSG_MODULE_MAIN_GOTO_COUNT_UPDATE:
			send_cmd_goto_count_update(ch, val);
			break;
		//add end
//#endif
		default:
			userlog(DEBUG_LOG, psName,
				"Module to Main Msg Unknown : %d %d %d\n", msg, ch, val);
			break;
	}
	return rtn;
}

int msgParsing_DataSave_to_Main(int msg, int ch, int val, int idx)
{
	int rtn=0, bd;

	switch(msg) {
		case MSG_DATASAVE_MAIN_CALI_NORMAL_RESULT_SEND:
			bd = ch; ch = val;
			send_cmd_cali_normal_result(bd, ch);
			break;
		case MSG_DATASAVE_MAIN_CALI_CHECK_RESULT_SEND:
			bd = ch; ch = val;
			send_cmd_cali_check_result(bd, ch);
			break;
		default:
			userlog(DEBUG_LOG, psName,
				"DataSave to Main Msg Unknown : %d %d %d\n", msg, ch, val);
			break;
	}
	return rtn;
}

int msgParsing_Meter_to_Main(int msg, int ch, int val, int idx)
{
	int rtn=0;

	switch(msg) {
		case MSG_METER_MAIN_INITIALIZE_REPLY:
			myPs->signal[MAIN_SIG_METER_CONNECT_REPLY] = P1;
			break;
		default:
			userlog(DEBUG_LOG, psName,
				"Meter to Main Msg Unknown : %d %d %d\n", msg, ch, val);
			break;
	}
	return rtn;
}

int msgParsing_Meter2_to_Main(int msg, int ch, int val, int idx)
{
	int rtn=0;

	switch(msg) {
#ifdef _TEMP_CALI  
		case MSG_METER2_MAIN_TEMP_CALI_BACKUP_READ_REPLY:
			send_cmd_temp_cali_backup_read_reply(val);
			break;
#endif
		default:
			userlog(DEBUG_LOG, psName,
				"Meter2 to Main Msg Unknown : %d %d %d\n", msg, ch, val);
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
