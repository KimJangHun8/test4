#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../../INC/datastore.h"
#include "userlog.h"
#include "local_utils.h"
#include "message.h"

extern volatile S_SYSTEM_DATA	*myData;
extern volatile S_DATA_SAVE		*myPs;
extern char psName[PROCESS_NAME_SIZE];

void Check_Message(void)
{
	rcv_msg(APP_TO_DATASAVE);
	rcv_msg(MAIN_TO_DATASAVE);
	rcv_msg(MODULE_TO_DATASAVE);
	rcv_msg(METER_TO_DATASAVE);
	rcv_msg(EXT_TO_DATASAVE);
	rcv_msg(MODULE_TO_DATASAVE_PATTERN);
}

void rcv_msg(int fromPs)
{
	int idx, msg, ch, val, rtn, num;
	
	if(myData->msg[fromPs].write_idx == myData->msg[fromPs].read_idx)
		return;

	myData->msg[fromPs].read_idx++;
	if(myData->msg[fromPs].read_idx >= MAX_MSG)
		myData->msg[fromPs].read_idx = 0;
	
	idx = myData->msg[fromPs].read_idx;
	msg = myData->msg[fromPs].msg_val[idx].msg;
	ch = myData->msg[fromPs].msg_val[idx].ch;
	val = myData->msg[fromPs].msg_val[idx].val;
	num = myData->msg[fromPs].msg_val[idx].num;
	
	switch(fromPs) {
		case APP_TO_DATASAVE:
			rtn = msgParsing_App_to_DataSave(msg, ch, val, idx);
			break;
		case MAIN_TO_DATASAVE:
			rtn = msgParsing_Main_to_DataSave(msg, ch, val, idx);
			break;
		case EXT_TO_DATASAVE:
			rtn = msgParsing_Ext_to_DataSave(msg, ch, val, idx);
			break;
		case MODULE_TO_DATASAVE:
			rtn = msgParsing_Module_to_DataSave(msg, ch, val, idx);
			break;
		case METER_TO_DATASAVE:
			rtn = msgParsing_Meter_to_DataSave(msg, ch, val, idx);
			break;
		case MODULE_TO_DATASAVE_PATTERN:	//191120 lyhw
			rtn = msgParsing_Module_to_DataSave_Ptn(msg, ch, val, idx, num);
			break;
		default:
			userlog(DEBUG_LOG, psName,
				"Msg Direction UnKnown : %d %d %d\n", msg, ch, val);
			break;
	}
}

int msgParsing_App_to_DataSave(int msg, int ch, int val, int idx)
{
	int rtn=0;

	switch(msg) {
		default:
			userlog(DEBUG_LOG, psName,
				"App to DataSave Msg Unknown : %d %d %d\n", msg, ch, val);
			break;
	}
	return rtn;
}

int msgParsing_Main_to_DataSave(int msg, int ch, int val, int idx)
{
	int rtn=0;
#ifdef _TRACKING_MODE
	int realCh, bd;
#endif

	switch(msg) {
		case MSG_MAIN_DATASAVE_SAVED_FILE_DELETE:
			myPs->signal[DATASAVE_SIG_SAVED_FILE_DELETE_IDX]
				= (unsigned char)idx;
			myPs->signal[DATASAVE_SIG_SAVED_FILE_DELETE] = P1;
			break;
		case MSG_MAIN_DATASAVE_RCVED_CONTINUE_CMD:
			myPs->signal[DATASAVE_SIG_RCVED_CONTINUE_CMD_IDX]
				= (unsigned char)idx;
			myPs->signal[DATASAVE_SIG_RCVED_CONTINUE_CMD] = P1;
			break;
#ifdef _TRACKING_MODE
		case MSG_MAIN_DATASAVE_READ_USER_SOC_TRACKING: //210609
			realCh = ch;
			bd = myData->CellArray1[realCh].bd;
			ch = myData->CellArray1[realCh].ch;
			rtn = Read_SOC_Tracking_File(0, realCh, val, 1);	
			if(rtn < 0){
				send_msg(DATASAVE_TO_MODULE,
					MSG_DATASAVE_MODULE_READ_FAIL_USER_SOC, bd, ch);
			}else{
				send_msg(DATASAVE_TO_MODULE,
					MSG_DATASAVE_MODULE_READ_SUCESS_USER_SOC, bd, ch);
			}
			break;
#endif		

		default:
			userlog(DEBUG_LOG, psName,
				"Main to DataSave Msg Unknown : %d %d %d\n", msg, ch, val);
			break;
	}
	return rtn;
}

int msgParsing_Ext_to_DataSave(int msg, int ch, int val, int idx)
{
	int rtn=0;

	switch(msg) {
		case MSG_EXT_DATASAVE_SAVED_FILE_DELETE:
			myPs->signal[DATASAVE_SIG_SAVED_FILE_DELETE_IDX]
				= (unsigned char)idx;
			myPs->signal[DATASAVE_SIG_SAVED_FILE_DELETE] = P1;
			break;
		case MSG_EXT_DATASAVE_RCVED_CONTINUE_CMD:
			myPs->signal[DATASAVE_SIG_RCVED_CONTINUE_CMD_IDX]
				= (unsigned char)idx;
			myPs->signal[DATASAVE_SIG_RCVED_CONTINUE_CMD] = P1;
			break;
		default:
			userlog(DEBUG_LOG, psName,
				"Ext to DataSave Msg Unknown : %d %d %d\n", msg, ch, val);
			break;
	}
	return rtn;
}

int msgParsing_Module_to_DataSave(int msg, int ch, int val, int idx)
{
	int rtn=0, bd, realCh;

	switch(msg) {
		case MSG_MODULE_DATASAVE_CALI_NORMAL_RESULT_SAVE:
			bd = ch; ch = val;
			//kjgw CaliCheckDataSave(bd, ch);
			send_msg(DATASAVE_TO_MAIN,
				MSG_DATASAVE_MAIN_CALI_NORMAL_RESULT_SEND, bd, ch);
			send_msg(DATASAVE_TO_MODULE,
				MSG_DATASAVE_MODULE_CALI_NORMAL_RESULT_SAVED, bd, ch);
			break;
		case MSG_MODULE_DATASAVE_CALI_CHECK_RESULT_SAVE:
			bd = ch; ch = val;
			//kjgw CaliCheckDataSave(bd, ch);
			send_msg(DATASAVE_TO_MAIN,
				MSG_DATASAVE_MAIN_CALI_CHECK_RESULT_SEND, bd, ch);
			send_msg(DATASAVE_TO_MODULE,
				MSG_DATASAVE_MODULE_CALI_CHECK_RESULT_SAVED, bd, ch);
			break;
		case MSG_MODULE_DATASAVE_READ_USER_PATTERN:
			realCh = ch;
			bd = myData->CellArray1[realCh].bd;
			ch = myData->CellArray1[realCh].ch;	
			if(myData->mData.config.function[F_PATTERN_CH_SAVE] == P1){
				if(myData->mData.config.function[F_PATTERN_FTP] == P1)
					rtn = Read_User_Pattern_NoUserData(realCh, val, 1); 
				else
					rtn = Read_User_Pattern_NoUserData(realCh, val, 0); 
			} else {
				if(myData->mData.config.function[F_PATTERN_FTP] == P1)
					rtn = Read_User_Pattern(realCh, val, 1); 
				else
					rtn = Read_User_Pattern(realCh, val, 0); 
			}
			if(rtn < 0){
				send_msg(DATASAVE_TO_MODULE,
					MSG_DATASAVE_MODULE_READ_FAIL_USER_PATTERN, bd, ch);
			}else{
				send_msg(DATASAVE_TO_MODULE,
					MSG_DATASAVE_MODULE_READ_SUCESS_USER_PATTERN, bd, ch);
			}
			break;
		case MSG_MODULE_DATASAVE_READ_USER_MAP:
			realCh = ch;
			bd = myData->CellArray1[realCh].bd;
			ch = myData->CellArray1[realCh].ch;
			rtn = Read_User_Map(realCh, val); 
			if(rtn < 0){
				send_msg(DATASAVE_TO_MODULE,
					MSG_DATASAVE_MODULE_READ_FAIL_USER_MAP, bd, ch);
			}else{
				send_msg(DATASAVE_TO_MODULE,
					MSG_DATASAVE_MODULE_READ_SUCESS_USER_MAP, bd, ch);
			}
			break;
#ifdef _TRACKING_MODE
		case MSG_MODULE_DATASAVE_READ_USER_SOC_TRACKING: //210609
			realCh = ch;
			bd = myData->CellArray1[realCh].bd;
			ch = myData->CellArray1[realCh].ch;
			rtn = Read_SOC_Tracking_File(0, realCh, val, 1);	
			if(rtn < 0){
				send_msg(DATASAVE_TO_MODULE,
					MSG_DATASAVE_MODULE_READ_FAIL_USER_SOC, bd, ch);
			}else{
				send_msg(DATASAVE_TO_MODULE,
					MSG_DATASAVE_MODULE_READ_SUCESS_USER_SOC, bd, ch);
			}
			break;
#endif		
		default:
			userlog(DEBUG_LOG, psName,
				"Module to DataSave Msg Unknown : %d %d %d\n", msg, ch, val);
			break;
	}
	return rtn;
}

int msgParsing_Module_to_DataSave_Ptn(int msg,int ch,int val,int idx,int num)
{	//191120 lyhw
	int rtn=0, bd, realCh;

	if(myData->mData.config.function[F_PATTERN_FTP] != P1){
	  	userlog(DEBUG_LOG, psName,
				"Check parameter FUNCTION Pattern FTP user flag \n");
		rtn = -1;
	  	return rtn;
	}
	
	switch(msg) {
		case MSG_READ_USER_PATTERN_1:
			realCh = ch;
			bd = myData->CellArray1[realCh].bd;
			ch = myData->CellArray1[realCh].ch;
		//	if(myData->mData.config.function[F_PATTERN_FTP] == P1){
			rtn = Read_User_Pattern_1(realCh, val, 1, num); 
		//	}
			if(rtn < 0){
				send_msg(DATASAVE_TO_MODULE,
					MSG_DATASAVE_MODULE_READ_FAIL_USER_PATTERN, bd, ch);
			}else{
				send_msg(DATASAVE_TO_MODULE,
					MSG_DATASAVE_MODULE_READ_SUCESS_USER_PATTERN, bd, ch);
			}
			break;
		case MSG_READ_USER_PATTERN_2:
			realCh = ch;
			bd = myData->CellArray1[realCh].bd;
			ch = myData->CellArray1[realCh].ch;
	//		if(myData->mData.config.function[F_PATTERN_FTP] == P1){
			rtn = Read_User_Pattern_2(realCh, val, 1, num); 
		//	}
			if(rtn < 0){
				send_msg(DATASAVE_TO_MODULE,
					MSG_DATASAVE_MODULE_READ_FAIL_USER_PATTERN, bd, ch);
			}else{
				send_msg(DATASAVE_TO_MODULE,
					MSG_DATASAVE_MODULE_READ_SUCESS_USER_PATTERN, bd, ch);
			}
			break;
		default:
			userlog(DEBUG_LOG, psName,
			"Module to DataSave Msg Unknown ptn : %d %d %d %d\n", msg, ch, val, num);
			break;
	}
	return rtn;
}

int msgParsing_Meter_to_DataSave(int msg, int ch, int val, int idx)
{
	int rtn=0;

	switch(msg) {
		default:
			userlog(DEBUG_LOG, psName,
				"Meter to DataSave Msg Unknown : %d %d %d\n", msg, ch, val);
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

