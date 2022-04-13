#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "../../INC/datastore.h"
#include "userlog.h"
#include "message.h"

extern volatile S_SYSTEM_DATA *myData;
extern volatile S_APP_CONTROL *myPs;
extern char psName[16];

void MessageCheck(void)
{
	rcv_msg(DATASAVE_TO_APP);
	rcv_msg(METER1_TO_APP);
	rcv_msg(METER2_TO_APP);
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
		case DATASAVE_TO_APP:
			rtn = msgParsing_DataSave_to_App(msg, ch, val, idx);
			break;
		case METER1_TO_APP:
			rtn = msgParsing_Meter1_to_App(msg, ch, val, idx);
			break;
		case METER2_TO_APP:
			rtn = msgParsing_Meter2_to_App(msg, ch, val, idx);
			break;
		default:
			userlog(DEBUG_LOG, psName,
				"Msg Direction UnKnown : %d %d %d\n", msg, ch, val);
			break;
	}
}

int msgParsing_DataSave_to_App(int msg, int ch, int val, int idx)
{
	int rtn=0;

	switch(msg) {
		default:
			userlog(DEBUG_LOG, psName,
				"DataSave to App Msg Unknown : %d %d %d\n", msg, ch, val);
			break;
	}
	return rtn;
}

int msgParsing_Meter1_to_App(int msg, int ch, int val, int idx)
{
	int rtn=0;

	switch(msg) {
		default:
			userlog(DEBUG_LOG, psName,
				"Meter1 to App Msg Unknown : %d %d %d\n", msg, ch, val);
			break;
	}
	return rtn;
}

int msgParsing_Meter2_to_App(int msg, int ch, int val, int idx)
{
	int rtn=0;

	switch(msg) {
		default:
			userlog(DEBUG_LOG, psName,
				"Meter2 to App Msg Unknown : %d %d %d\n", msg, ch, val);
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

