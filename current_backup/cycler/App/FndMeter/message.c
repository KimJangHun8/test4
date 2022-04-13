#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../../INC/datastore.h"
#include "userlog.h"
#include "serial.h"
#include "message.h"

extern volatile S_SYSTEM_DATA *myData;
extern volatile S_FND_METER  *myPs;
extern char psName[PROCESS_NAME_SIZE];

void Check_Message(void)
{
//	rcv_msg(APP_TO_METER2);
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
		default:
			userlog(DEBUG_LOG, psName,
				"App to Meter2 Msg Unknown : %d %d %d\n", msg, ch, val);
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
