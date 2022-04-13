#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../../INC/datastore.h"
#include "userlog.h"
#include "serial.h"
#include "message.h"

extern volatile S_SYSTEM_DATA *myData;
extern volatile S_FADM *myPs;
extern char psName[PROCESS_NAME_SIZE];

void Check_Message(void)
{
	rcv_msg(APP_TO_METER2);
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
	int rtn=0, comm_idx, i, tmp, use_ch;
	unsigned long chFlag;

	switch(msg) {
		case MSG_APP_METER2_MEASURE:
			comm_idx = myPs->comm_buffer.write_idx;
			comm_idx++;
			if(comm_idx >= MAX_FADM_USE_CH) comm_idx = 0;

			use_ch = 0;
			for(i=0; i < myData->mData.config.installedCh; i++) {
				tmp = i / 32;
				chFlag = 0x00000001;
				chFlag = chFlag << (i % 32);
				chFlag = chFlag & myData->msg[APP_TO_METER2]
					.msg_ch_flag[idx].bit_32[tmp];
				if(chFlag != 0) {
					use_ch = i;
					break;
				}
			}
			myPs->comm_buffer.use_ch[comm_idx] = use_ch;
			myPs->comm_buffer.use_totalCycle[comm_idx] = ch;
			myPs->comm_buffer.use_stepNo[comm_idx] = val;
			myPs->comm_buffer.write_idx = comm_idx;
			break;
		case MSG_APP_METER2_REQUEST:
			send_cmd_request(ch, val);
			break;
		case MSG_APP_METER2_COMM_CHECK:
			send_cmd_comm_check(ch, val);
			break;
		case MSG_APP_METER2_WR_BASE_ADDR:
			send_cmd_wr_base_addr(ch, val);
			break;
		default:
			userlog(DEBUG_LOG, psName,
				"App to Meter2 Msg Unknown : %d %d %d\n", msg, ch, val);
			break;
	}
	return rtn;
}

int msgParsing_Module_to_Meter2(int msg, int ch, int val, int idx)
{
	int rtn=0, comm_idx, i, tmp, use_ch;
	unsigned long chFlag;

	switch(msg) {
		case MSG_MODULE_METER2_MEASURE:
			comm_idx = myPs->comm_buffer.write_idx;
			comm_idx++;
			if(comm_idx >= MAX_FADM_USE_CH) comm_idx = 0;

			use_ch = 0;
			for(i=0; i < myData->mData.config.installedCh; i++) {
				tmp = i / 32;
				chFlag = 0x00000001;
				chFlag = chFlag << (i % 32);
				chFlag = chFlag & myData->msg[MODULE_TO_METER2]
					.msg_ch_flag[idx].bit_32[tmp];
				if(chFlag != 0) {
					use_ch = i;
					break;
				}
			}
			myPs->comm_buffer.use_ch[comm_idx] = use_ch;
			myPs->comm_buffer.use_totalCycle[comm_idx] = ch;
			myPs->comm_buffer.use_stepNo[comm_idx] = val;
			myPs->comm_buffer.write_idx = comm_idx;
			break;
		default:
			userlog(DEBUG_LOG, psName,
				"Module to Meter2 Msg Unknown : %d %d %d\n", msg, ch, val);
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

void send_fadm_pulse_msg(int channel)
{
	int i;

	for(i=0; i < 300; i++) {
		send_fadm_pulse_msg_2(channel, i);
	}
}

void send_fadm_pulse_msg_2(int channel, int index)
{
	int bd, ch, msg, idx, type;

	msg = 2;

	bd = myData->CellArray1[channel].bd;
	ch = myData->CellArray1[channel].ch;

	if(myData->pulse_msg[msg][bd][ch].flag == 1) { //send pulse_msg stop
		return;
	}

	if(index == 0) type = 0;
	else if(index == 299) type = 2;
	else type = 1;

	idx = myData->pulse_msg[msg][bd][ch].write_idx;
	idx++;
	if(idx >= MAX_PULSE_MSG) idx = 0;

	myData->pulse_msg[msg][bd][ch].val[idx].type = type;
	myData->pulse_msg[msg][bd][ch].val[idx].runTime = (index * 2) * (-1);
	myData->pulse_msg[msg][bd][ch].val[idx].Vsens
		= myPs->pulse_ad[bd][ch].value[0][index];
	myData->pulse_msg[msg][bd][ch].val[idx].Isens
		= myPs->pulse_ad[bd][ch].value[1][index];

	myData->pulse_msg[msg][bd][ch].val[idx].totalCycle
		= (long)myPs->misc.use_totalCycle;
	myData->pulse_msg[msg][bd][ch].val[idx].stepNo
		= (long)myPs->misc.use_stepNo;
	myData->pulse_msg[msg][bd][ch].val[idx].capacity = 0;
	myData->pulse_msg[msg][bd][ch].val[idx].wattHour = 0;

	myData->pulse_msg[msg][bd][ch].write_idx = idx;
	myData->pulse_msg[msg][bd][ch].count++;
}
