#include <asm/io.h>
#include <rtl_core.h>
#include <math.h>
#include "../../INC/datastore.h"
#include "rtTask.h"
#include "ChannelControl.h"
#include "message.h"
#include "DAQ.h"

extern S_SYSTEM_DATA *myData;
extern S_MODULE_DATA *myPs;

void Check_Message(void)
{
	rcv_msg(APP_TO_MODULE);
	rcv_msg(MAIN_TO_MODULE);
	rcv_msg(DATASAVE_TO_MODULE);
	rcv_msg(METER_TO_MODULE);
	rcv_msg(CALIMETER2_TO_MODULE);
	rcv_msg(PSKILL_TO_MODULE);
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
		case APP_TO_MODULE:
			rtn = msgParsing_App_to_Module(msg, ch, val, idx);
			break;
		case MAIN_TO_MODULE:
			rtn = msgParsing_MainClient_to_Module(msg, ch, val, idx);
			break;
		case DATASAVE_TO_MODULE:
			rtn = msgParsing_DataSave_to_Module(msg, ch, val, idx);
			break;
		case METER_TO_MODULE:
			rtn = msgParsing_Meter_to_Module(msg, ch, val, idx);
			break;
		case CALIMETER2_TO_MODULE:
			rtn = msgParsing_CaliMeter2_to_Module(msg, ch, val, idx);
			break;
		case PSKILL_TO_MODULE:
			rtn = msgParsing_PSKill_to_Module(msg, ch, val, idx);
			break;
		default: break;
	}
}
int msgParsing_PSKill_to_Module(int msg, int ch, int val, int idx)
{
	int rtn, bd1, ch1;
	rtn = 0;
	switch(msg) {
		case MSG_PSKILL_MODULE_EXIT:
			send_msg(MODULE_TO_APP, MSG_MODULE_APP_POWER_SWITCH, 3, 0);
			myPs->signal[M_SIG_RUN_LED] = P0;
			myPs->signal[M_SIG_REMOTE_SMPS1] = P0;
			myPs->signal[M_SIG_FAN_RELAY] = P0;
			//180611 add for digital
#if CYCLER_TYPE == DIGITAL_CYC
			myPs->signal[M_SIG_INV_POWER] = P100;
			myPs->signal[M_SIG_INV_POWER1] = P100;
#endif
			//210303 Inverter Off
			if(myData->AppControl.config.systemType == CYCLER_CAN){
				myPs->signal[M_SIG_INV_POWER_CAN] = P100;
			}
			
			myPs->code = M_FAIL_TERMINAL_QUIT;
			for(bd1=0; bd1 < myPs->config.installedBd; bd1++) {
		    	for(ch1=0; ch1 < myPs->config.chPerBd; ch1++) {
					if((myPs->config.chPerBd * bd1 + ch1)
					        >= myPs->config.installedCh) {
						continue;
					}
					if(myData->bData[bd1].cData[ch1].op.state == C_RUN
						    && myData->bData[bd1].cData[ch1]
							.signal[C_SIG_TERMINAL_QUIT] == P0) {
						myData->bData[bd1].cData[ch1]
								.signal[C_SIG_TERMINAL_QUIT] = P1;
				    } else if(myData->bData[bd1].cData[ch1].op.state == C_CALI)
					{
						myData->bData[bd1].cData[ch1].op.state = C_IDLE;
						myData->bData[bd1].cData[ch1].op.phase = P0;
					}	
				}
			}
			break;
		default: break;
	}
	return rtn;
}
int msgParsing_App_to_Module(int msg, int ch, int val, int idx)
{
	int rtn, bd, bd1, ch1;
	
	rtn = 0;
	switch(msg) {
		case MSG_APP_MODULE_EXIT:
			if(ch == 0) { //quit
				send_msg(MODULE_TO_APP, MSG_MODULE_APP_POWER_SWITCH, 3, 0);
				//180611 add for digital
#if CYCLER_TYPE == DIGITAL_CYC
				myPs->signal[M_SIG_INV_POWER] = P100;
				myPs->signal[M_SIG_INV_POWER1] = P100;
#endif
				//210303 Inverter Off
				if(myData->AppControl.config.systemType == CYCLER_CAN){
					myPs->signal[M_SIG_INV_POWER_CAN] = P100;
				}
		
				myPs->signal[M_SIG_RUN_LED] = P0;
				myPs->signal[M_SIG_REMOTE_SMPS1] = P0;
				myPs->signal[M_SIG_FAN_RELAY] = P0;
				myPs->code = M_FAIL_TERMINAL_QUIT;
			} else { //halt
				send_msg(MODULE_TO_APP, MSG_MODULE_APP_POWER_SWITCH, 4, 0);
				//180611 add for digital
#if CYCLER_TYPE == DIGITAL_CYC
				myPs->signal[M_SIG_INV_POWER] = P100;
				myPs->signal[M_SIG_INV_POWER1] = P100;
#endif
				//210303 Inverter Off
				if(myData->AppControl.config.systemType == CYCLER_CAN){
					myPs->signal[M_SIG_INV_POWER_CAN] = P100;
				}
			
				myPs->signal[M_SIG_RUN_LED] = P0;
				myPs->signal[M_SIG_REMOTE_SMPS1] = P0;
				myPs->signal[M_SIG_FAN_RELAY] = P0;
				myPs->signal[M_SIG_POWER_OFF] = P1;
				myPs->code = M_FAIL_TERMINAL_HALT;
			}
			for(bd1=0; bd1 < myPs->config.installedBd; bd1++) {
		    	for(ch1=0; ch1 < myPs->config.chPerBd; ch1++) {
					if((myPs->config.chPerBd * bd1 + ch1)
					        >= myPs->config.installedCh) {
						continue;
					}
					if(myData->bData[bd1].cData[ch1].op.state == C_RUN
						    && myData->bData[bd1].cData[ch1]
							.signal[C_SIG_TERMINAL_QUIT] == P0) {
						myData->bData[bd1].cData[ch1]
								.signal[C_SIG_TERMINAL_QUIT] = P1;
				    } else if(myData->bData[bd1].cData[ch1].op.state == C_CALI)
					{
						myData->bData[bd1].cData[ch1].op.state = C_IDLE;
						myData->bData[bd1].cData[ch1].op.phase = P0;
				    }
				}
			}
			break;
		case MSG_APP_MODULE_CH_CALI:
			bd = ch / myPs->config.chPerBd;
			ch = ch % myPs->config.chPerBd;
			myData->bData[bd].cData[ch].signal[C_SIG_CALI] = P1;
			myData->bData[bd].cData[ch].signal[C_SIG_CALI_POINT] = P0;
			if(myData->cali.tmpCond[bd][ch].type == 0) { //voltage
				if(myData->cali.tmpCond[bd][ch].mode == CALI_MODE_NORMAL) {
					myData->bData[bd].cData[ch].signal[C_SIG_CALI_PHASE] = P0;
				} else if(myData->cali.tmpCond[bd][ch].mode
					== CALI_MODE_CHECK) {
#if CYCLER_TYPE == DIGITAL_CYC
					myData->bData[bd].cData[ch]
							.signal[C_SIG_CALI_PHASE] = P70; //180611
#endif
#if CYCLER_TYPE == LINEAR_CYC || CYCLER_TYPE == CAN_CYC
					myData->bData[bd].cData[ch]
							.signal[C_SIG_CALI_PHASE] = P10;
#endif
				}
			} else { //current
				if(myData->cali.tmpCond[bd][ch].mode == CALI_MODE_NORMAL) {
#if CYCLER_TYPE == DIGITAL_CYC
					myData->bData[bd].cData[ch]
							.signal[C_SIG_CALI_PHASE] = P80; //180611
#endif
#if CYCLER_TYPE == LINEAR_CYC || CYCLER_TYPE == CAN_CYC
					myData->bData[bd].cData[ch]
							.signal[C_SIG_CALI_PHASE] = P20;
#endif
				} else if(myData->cali.tmpCond[bd][ch].mode
					== CALI_MODE_CHECK) {
#if CYCLER_TYPE == DIGITAL_CYC
					myData->bData[bd].cData[ch]
							.signal[C_SIG_CALI_PHASE] = P90; //180611
#endif
#if CYCLER_TYPE == LINEAR_CYC || CYCLER_TYPE == CAN_CYC
					myData->bData[bd].cData[ch]
							.signal[C_SIG_CALI_PHASE] = P30;
#endif
				}
			}
			break;
		case MSG_APP_MODULE_SAVE_START:
			myPs->signal[M_SIG_TEST_SEND_SAVE_DATA] = (unsigned char)(ch+1);
			break;
		case MSG_APP_MODULE_AUX_CALI_LOW:
				DAQ_Cali_Read(ch , val, 1);
			break;
		case MSG_APP_MODULE_AUX_CALI_HIGH:
				DAQ_Cali_Read(ch , val, 2);
			break;
		case MSG_APP_MODULE_AUX_CALI_UPDATE:
				DAQ_Cali_Update();
			break;
		case MSG_APP_MODULE_SAVE_STOP:
			myPs->signal[M_SIG_TEST_SEND_SAVE_DATA] = P0;
			break;
		case MSG_APP_MODULE_D_CALI_UPDATE:	//180813 lyh add
			myData->mData.misc.d_cali_update_ch = 0;
			for(bd1=0; bd1 < myPs->config.installedBd; bd1++) {
		    	for(ch1=0; ch1 < myPs->config.chPerBd; ch1++) {
					myData->bData[bd1].cData[ch1].op.state = C_CALI_UPDATE;
					myData->bData[bd1].cData[ch1]
							.signal[C_SIG_D_CALI_UPDATE] = P1;
					myData->bData[bd1].cData[ch1]
							.signal[C_SIG_CALI_PHASE] = P0;
				}
			}
			break;
		default: break;
	}
	return rtn;
}

int msgParsing_MainClient_to_Module(int msg, int ch, int val, int idx)
{
	int rtn, bd = 0, tmp;
	int bd1 = 0, ch1 = 0;
	
	rtn = 0;
	switch(msg) {
		case MSG_MAIN_MODULE_CH_CALI:
			bd = ch / myPs->config.chPerBd;
			ch = ch % myPs->config.chPerBd;
			myData->bData[bd].cData[ch].signal[C_SIG_CALI] = P1;
			myData->bData[bd].cData[ch].signal[C_SIG_CALI_POINT] = P0;
			switch(val) {
				case 0: //voltage cali
					myData->bData[bd].cData[ch].signal[C_SIG_CALI_PHASE]
						= P0;
					myData->bData[bd].cData[ch].signal[C_SIG_I_RANGE] = RANGE1;
					break;
				case 1: //current cali
#if CYCLER_TYPE == DIGITAL_CYC	//180611
					myData->bData[bd].cData[ch]
							.signal[C_SIG_CALI_PHASE] = P80;
#endif
#if CYCLER_TYPE == LINEAR_CYC || CYCLER_TYPE == CAN_CYC
					myData->bData[bd].cData[ch]
							.signal[C_SIG_CALI_PHASE] = P20;
#endif
					break;
				case 2: //voltage cali check
#if CYCLER_TYPE == DIGITAL_CYC	//180611
					myData->bData[bd].cData[ch]
							.signal[C_SIG_CALI_PHASE] = P70;
#endif
#if CYCLER_TYPE == LINEAR_CYC || CYCLER_TYPE == CAN_CYC
					myData->bData[bd].cData[ch]
							.signal[C_SIG_CALI_PHASE] = P10;
#endif
					myData->bData[bd].cData[ch].signal[C_SIG_I_RANGE] = RANGE1;
					break;
				case 3: //current cali check
#if CYCLER_TYPE == DIGITAL_CYC	//180611
					myData->bData[bd].cData[ch]
							.signal[C_SIG_CALI_PHASE] = P90;
#endif
#if CYCLER_TYPE == LINEAR_CYC || CYCLER_TYPE == CAN_CYC
					myData->bData[bd].cData[ch]
							.signal[C_SIG_CALI_PHASE] = P30;
#endif
					break;
			}
			break;
		case MSG_MAIN_MODULE_CALI_RELAY:
			if(ch == 0) { //voltage
				myData->mData.signal[M_SIG_CALI_RELAY] = P0;
				myData->mData.signal[M_SIG_CALI_RELAY2] = P0;
			} else { //current
				switch(myPs->config.hwSpec) {
					case L_5V_3A:
					case L_6V_6A:
					case L_5V_2A:
					case L_5V_200A:
					case L_5V_10mA:
					case L_5V_5A:
					case L_5V_30A:
					case L_2V_60A:
					case L_5V_5A_2:
						if(val == 0) { //RANGE1
							myData->mData.signal[M_SIG_CALI_RELAY] = P1;
							myData->mData.signal[M_SIG_CALI_RELAY2] = P1;
						} else if(val == 1) { //RANGE2
							myData->mData.signal[M_SIG_CALI_RELAY] = P1;
							myData->mData.signal[M_SIG_CALI_RELAY2] = P0;
						}
						break;
					case L_2V_100A:
					case S_5V_200A:
					case L_5V_50A:
					case L_5V_2A_R1:
						if(val == 0) { //RANGE1
							myData->mData.signal[M_SIG_CALI_RELAY] = P1;
							myData->mData.signal[M_SIG_CALI_RELAY2] = P1;
						} else if(val == 1) { //RANGE2
							myData->mData.signal[M_SIG_CALI_RELAY] = P1;
							myData->mData.signal[M_SIG_CALI_RELAY2] = P1;
						}
						break;
					default:	break;
				}
			}
			break;
		case MSG_MAIN_MODULE_SAVE_MSG_FLAG:
			//ch:msg, val:0-run,1-stop
			tmp = ch;
			myData->save_msg[ch].flag = val;
			if(val == 0) {
				myPs->signal[M_SIG_NET_CHECK] = P0;
			} else if(val == 1) {
				myPs->signal[M_SIG_NET_CHECK] = P1;
			}
			for(bd=0; bd < MAX_BD_PER_MODULE; bd++) {
				for(ch=0; ch < MAX_CH_PER_BD; ch++) {
					myData->pulse_msg[tmp][bd][ch].flag = val;
				}
			}
			break;
		case MSG_MAIN_MODULE_SHUTDOWN_FLAG:
			if(ch == 1){
				myPs->signal[M_SIG_GUI_TO_SBC_SHUTDOWN] = P1;
			} else {
				myPs->signal[M_SIG_GUI_TO_SBC_SHUTDOWN] = P0;
			}
			break;
		case MSG_MAIN_MODULE_CHAMBER_FAULT:
			if(ch == 0){	//pause
				for(bd1=0; bd1 < myPs->config.installedBd; bd1++) {
					for(ch1=0; ch1 < myPs->config.chPerBd; ch1++) {
						if((myPs->config.chPerBd * bd + ch) 
							> myPs->config.installedCh-1) { 
							continue;
						}
						if(myData->bData[bd1].cData[ch1].op.state == C_RUN){
							myData->bData[bd1].cData[ch1].signal[C_SIG_CHAMBER_ERROR] = P1;
						}
					}
				}
			}else{ //shutdown
#if CYCLER_TYPE == DIGITAL_CYC
				myPs->signal[M_SIG_INV_POWER] = P100;
				myPs->signal[M_SIG_INV_POWER1] = P100;
#endif
				if(myData->AppControl.config.systemType == CYCLER_CAN){ 
					myPs->signal[M_SIG_INV_POWER_CAN] = P100;
				}
				myPs->signal[M_SIG_RUN_LED] = P0;
				myPs->signal[M_SIG_REMOTE_SMPS1] = P0;
				myPs->signal[M_SIG_FAN_RELAY] = P0;
				myPs->signal[M_SIG_POWER_OFF] = P1;
				#ifdef _DYSON
				myPs->signal[M_SIG_MAIN_MC_OFF] = P1;	//220214_hun
				#endif
				send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 38, ch); 
				for(bd1=0; bd1 < myPs->config.installedBd; bd1++) {
					for(ch1=0; ch1 < myPs->config.chPerBd; ch1++) {
						if((myPs->config.chPerBd * bd1 + ch1)
							> (myPs->config.installedCh - 1)) continue;
							myData->bData[bd1].cData[ch1].signal[C_SIG_PAUSE] = P1;
					}
				}
			}	
			break;
		default: break;
	}
	return rtn;
}

int msgParsing_DataSave_to_Module(int msg, int ch, int val, int idx)
{
	unsigned char slave_ch;
	int rtn, i, bd = 0, tmp;
	unsigned long chFlag;
	
	rtn = 0;
	switch(msg) {
		case MSG_DATASAVE_MODULE_CH_RUN:
			for(i=0; i < myPs->config.installedCh; i++) {
				tmp = i / 32;
				chFlag = 0x00000001;
				chFlag = chFlag << (i % 32);
				chFlag = chFlag & myData->msg[DATASAVE_TO_MODULE]
					.msg_ch_flag[idx].bit_32[tmp];
				if(chFlag != 0) {
					bd = myData->CellArray1[i].bd;
					ch = myData->CellArray1[i].ch;
					
					myData->bData[bd].cData[ch].signal[C_SIG_RUN] = P1;
					
					if(myData->bData[bd].cData[ch].ChAttribute.opType == P1) {
						slave_ch = myData->bData[bd].cData[ch]
							.ChAttribute.chNo_slave[0] - 1; //1base
						myData->bData[bd].cData[slave_ch].signal[C_SIG_RUN]=P1;
					}
				}
			}
			break;
		case MSG_DATASAVE_MODULE_CALI_NORMAL_RESULT_SAVED:
			bd = ch; ch = val;
			myData->bData[bd].cData[ch].signal[C_SIG_CALI_NORMAL_RESULT_SAVED]
				= P1;
			break;
		case MSG_DATASAVE_MODULE_CALI_CHECK_RESULT_SAVED:
			bd = ch; ch = val;
			myData->bData[bd].cData[ch].signal[C_SIG_CALI_CHECK_RESULT_SAVED]
				= P1;
			break;
		case MSG_DATASAVE_MODULE_READ_SUCESS_USER_PATTERN:
			bd = ch; ch = val;
			myData->bData[bd].cData[ch].misc.userPatternFlag = P1;
			break;
		case MSG_DATASAVE_MODULE_READ_FAIL_USER_PATTERN:
			bd = ch; ch = val;
	        myData->bData[bd].cData[ch].signal[C_SIG_FAIL_USER_PATTERN_READ] = P1;
			break;
		case MSG_DATASAVE_MODULE_READ_SUCESS_USER_MAP:
			bd = ch; ch = val;
			myData->bData[bd].cData[ch].misc.userMapFlag = P1;
			break;
		case MSG_DATASAVE_MODULE_READ_FAIL_USER_MAP:
			bd = ch; ch = val;
	        myData->bData[bd].cData[ch].signal[C_SIG_FAIL_USER_MAP_READ] = P1;
			break;
		case MSG_DATASAVE_MODULE_CH_CONTINUE:
			for(i=0; i < myPs->config.installedCh; i++) {
				tmp = i / 32;
				chFlag = 0x00000001;
				chFlag = chFlag << (i % 32);
				chFlag = chFlag & myData->msg[DATASAVE_TO_MODULE]
					.msg_ch_flag[idx].bit_32[tmp];
				if(chFlag != 0) {
					bd = myData->CellArray1[i].bd;
					ch = myData->CellArray1[i].ch;
					myData->bData[bd].cData[ch].signal[C_SIG_CONTINUE] = P1;
				}
			}
			break;
#ifdef _TRACKING_MODE
		case MSG_DATASAVE_MODULE_READ_FAIL_USER_SOC:
			bd = ch; ch = val;
			myData->bData[bd].cData[ch].misc.file_fail_flag = P1; //210720
			break;
		case MSG_DATASAVE_MODULE_READ_SUCESS_USER_SOC:
			bd = ch; ch = val;
			myData->bData[bd].cData[ch].misc.file_sucess_flag = P1;
			break;
#endif
		default: break;
	}
	return rtn;
}

int msgParsing_Meter_to_Module(int msg, int ch, int val, int idx)
{
	int rtn, bd;
	
	rtn = 0;
	switch(msg) {
		case MSG_METER_MODULE_REQUEST_REPLY:
			bd = ch;
			ch = val;
			myData->bData[bd].cData[ch].signal[C_SIG_METER_REPLY] = P1;
			break;
		default: break;
	}
	return rtn;
}

int msgParsing_CaliMeter2_to_Module(int msg, int ch, int val, int idx) 
//20160229
{
	int rtn, bd;
	
	rtn = 0;
	switch(msg) {
		case MSG_METER_MODULE_REQUEST_REPLY:
			bd = ch;
			ch = val;
			myData->bData[bd].cData[ch].signal[C_SIG_CALIMETER2_REPLY] = P1;
			break;
		default: break;
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
	myData->msg[toPs].msg_val[idx].ch = ch;    //Fault Number
	myData->msg[toPs].msg_val[idx].val = val;  //real Ch
	myData->msg[toPs].write_idx = idx;
}

void send_msg_2(int toPs, int msg, int ch, int val, int bd, long event_val)
{
	int idx;
	
	idx = myData->msg[toPs].write_idx;
	idx++;
	if(idx >= MAX_MSG) idx = 0;
	
	myData->msg[toPs].msg_val[idx].msg = msg;
	myData->msg[toPs].msg_val[idx].ch = ch;    //Fault Number
	myData->msg[toPs].msg_val[idx].val = val;  //real Ch
	myData->msg[toPs].write_idx = idx;
	myData->msg[toPs].msg_val[idx].bd = bd;    
	myData->msg[toPs].msg_val[idx].event_val = event_val;  //real Value
}

void send_msg_ptn(int toPs, int msg, int ch, int val, int num)
{
	int idx;
	
	idx = myData->msg[toPs].write_idx;
	idx++;
	if(idx >= MAX_MSG) idx = 0;
	
	myData->msg[toPs].msg_val[idx].msg = msg;
	myData->msg[toPs].msg_val[idx].ch = ch;    //Fault Number
	myData->msg[toPs].msg_val[idx].val = val;  //real Ch
	myData->msg[toPs].write_idx = idx;
	myData->msg[toPs].msg_val[idx].num = num;    
}

void send_msg_ch_flag(int toPs, char *ch_flag)
{
	int idx;
	
	idx = myData->msg[toPs].write_idx;
	idx++;
	if(idx >= MAX_MSG) idx = 0;
	
	memcpy((char *)&myData->msg[toPs].msg_ch_flag[idx].bit_32,
		(char *)ch_flag, sizeof(S_MSG_CH_FLAG));
}

void send_save_msg(int bd, int ch, unsigned long saveDt, int type)
{
	if(myData->bData[bd].cData[ch].ChAttribute.chNo_master == P0) {
		myData->bData[bd].cData[ch].op.resultIndex++;
		myData->bData[bd].cData[ch-1].op.resultIndex++;
		myData->bData[bd].cData[ch].opSave = myData->bData[bd].cData[ch].op;
		myData->bData[bd].cData[ch-1].opSave = myData->bData[bd].cData[ch-1].op;
	} else if(myData->bData[bd].cData[ch].ChAttribute.opType == P0) {
		myData->bData[bd].cData[ch].op.resultIndex++;
		myData->bData[bd].cData[ch].opSave = myData->bData[bd].cData[ch].op;
	}
	
	if(myData->DataSave.config.save_data_type == P1) {
		if(myData->bData[bd].cData[ch].ChAttribute.chNo_master == P0) {
			if(myData->DataSave.config.monitoringData_saveFlag == P1) {
				send_10ms_msg_2(bd, ch-1, saveDt, type, 0); //MainClient
			}
			if(myData->DataSave.config.resultData_saveFlag == P1) {
#if CYCLER_TYPE == DIGITAL_CYC	//191204
				send_10ms_msg_2(bd, ch-1, saveDt, type, 1); //DataSave
#endif
#if CYCLER_TYPE == LINEAR_CYC || CYCLER_TYPE == CAN_CYC
				if(myPs->config.chPerBd <= 8) {
					send_10ms_msg_2(bd, ch-1, saveDt, type, 1); //DataSave
				}
#endif
			}
		} else if(myData->bData[bd].cData[ch].ChAttribute.opType == P0) {
			if(myData->DataSave.config.monitoringData_saveFlag == P1) {
				send_10ms_msg_2(bd, ch, saveDt, type, 0); //MainClient
			}

			if(myData->DataSave.config.resultData_saveFlag == P1) {
#if CYCLER_TYPE == DIGITAL_CYC
				send_10ms_msg_2(bd, ch, saveDt, type, 1); //DataSave
#endif
#if CYCLER_TYPE == LINEAR_CYC || CYCLER_TYPE == CAN_CYC
				if(myPs->config.chPerBd <= 8) {
					send_10ms_msg_2(bd, ch, saveDt, type, 1); //DataSave
				}
#endif
			}
		}
		
		if(myData->bData[bd].cData[ch].ChAttribute.chNo_master == P0) {
			if(type == 0){
				myData->bData[bd].cData[ch-1].misc.save10msDataCount++;
				myData->bData[bd].cData[ch].misc.save10msDataCount++;
			}else if (type == 1) {
				myData->bData[bd].cData[ch-1].misc.save10msDataCount = 0;
				myData->bData[bd].cData[ch].misc.save10msDataCount = 0;
			}
		} else if(myData->bData[bd].cData[ch].ChAttribute.opType == P0) {
			if(type == 0){
				myData->bData[bd].cData[ch].misc.save10msDataCount++;
			}else if (type == 1) {
				myData->bData[bd].cData[ch].misc.save10msDataCount = 0;
			}
		}
	} else {
		if(myData->bData[bd].cData[ch].ChAttribute.chNo_master == P0) {
			if(myData->DataSave.config.monitoringData_saveFlag == P1) {
				send_save_msg_2(bd, ch-1, saveDt, type, 0); //MainClient
			}
			if(myData->DataSave.config.resultData_saveFlag == P1) {
				send_save_msg_2(bd, ch-1, saveDt, type, 1); //DataSave
			}
		} else if(myData->bData[bd].cData[ch].ChAttribute.opType == P0) {
			if(myData->DataSave.config.monitoringData_saveFlag == P1) {
				send_save_msg_2(bd, ch, saveDt, type, 0); //MainClient
			}
			if(myData->DataSave.config.resultData_saveFlag == P1) {
				send_save_msg_2(bd, ch, saveDt, type, 1); //DataSave
			}
		}
	}
}

void send_save_msg_aux(int bd, int ch, int idx, int msg)
{
	int i , map;

	// 130226 oys w : AuxDataSave
	if(myData->mData.config.installedTemp > 0)
	{
		for (i=0; i < myData->mData.config.installedTemp; i++)
		{
			myData->save_msg[msg].auxData[idx][i].auxChNo = i+1;
			if(i < 100){
				myData->save_msg[msg].auxData[idx][i].val
					= myData->AnalogMeter.temp[i].temp;
			}else if(i >= 100){
				myData->save_msg[msg].auxData[idx][i].val
					= myData->AnalogMeter2.temp[i-100].temp;
			}
		}
	}
	if(myData->mData.config.installedAuxV > 0)
	{
		for (i=MAX_AUX_TEMP_DATA;
			i < myData->mData.config.installedAuxV+MAX_AUX_TEMP_DATA; i++)
		{
			map = myData->daq.misc.map[i-MAX_AUX_TEMP_DATA];
			myData->save_msg[msg].auxData[idx][i].auxChNo
				= i-(MAX_AUX_TEMP_DATA-1);
			myData->save_msg[msg].auxData[idx][i].val
				= myData->daq.op.ch_vsens[map];
		}
	}
}

#ifdef __LG_VER1__
void send_save_msg_2(int bd, int ch, unsigned long saveDt, int type, int msg)
{
	unsigned char stepType,slave_ch = 0;
	unsigned short advStepNo;
	int i, idx, dataType, save100ms_Limit;
	int aux_i;

	aux_i = i = 0;
	stepType = myData->bData[bd].cData[ch].op.type;
	advStepNo = myData->bData[bd].cData[ch].misc.advStepNo;

	//190124 add lyhw
	if(myData->AppControl.config.versionNo >= 20150402){
		save100ms_Limit = myData->DataSave.config.save_100ms_time;
	}else{
	//	save100ms_Limit = 6000;
		save100ms_Limit = myData->DataSave.config.save_100ms_time; //201015
	}
/*
	if(type == 0) {
		if(saveDt == 0 || saveDt >= RTTASK_1000MS) {
			dataType = 0; //Vsens, Isens
		} else {
			dataType = 1; //tmpVsens, tmpIsens
		}
	} else {
		dataType = 1; //tmpVsens, tmpIsens
	}
*/
	if(myData->bData[bd].cData[ch].op.type == STEP_USER_PATTERN){
		dataType = 1; //tmpVsens, tmpIsens
	}else if(myData->bData[bd].cData[ch].op.type == STEP_USER_MAP){
		dataType = 1; //tmpVsens, tmpIsens
	}else if(myData->bData[bd].cData[ch].op.mode == CP){
		dataType = 1; //tmpVsens, tmpIsens
	}else if(myData->bData[bd].cData[ch].op.phase == P0){
		dataType = myData->bData[bd].cData[ch].misc.preDataType; //210217
	}else if(myData->bData[bd].cData[ch].op.runTime > 0 &&
		myData->bData[bd].cData[ch].op.runTime < 100){
		dataType = 1; //tmpVsens, tmpIsens
	}else{
		dataType = 0;
	}

	if(myData->bData[bd].cData[ch].op.type == STEP_LOOP) type = 2;
	
	if(myData->save_msg[msg].flag == 1) { //send save_msg stop
		return;
	}
	
	idx = myData->save_msg[msg].write_idx[bd][ch];
	idx++;
	
	if(idx >= MAX_SAVE_MSG) idx = 0;
	
	if(msg == 1){
		if(myData->mData.config.installedTemp > 0
			|| myData->mData.config.installedAuxV > 0){
			send_save_msg_aux(bd, ch, idx, msg); //auxDataSave
		}
	}

	myData->save_msg[msg].val[idx][bd][ch].chData.resultIndex
		= myData->bData[bd].cData[ch].op.resultIndex;
	myData->save_msg[msg].val[idx][bd][ch].chData.state
		= myData->bData[bd].cData[ch].op.state;
	myData->save_msg[msg].val[idx][bd][ch].chData.type
		= myData->bData[bd].cData[ch].op.type;
	myData->save_msg[msg].val[idx][bd][ch].chData.mode
		= myData->bData[bd].cData[ch].op.mode;
	
	if(myData->bData[bd].cData[ch].ChAttribute.opType == P1)
	{
		slave_ch = myData->bData[bd].cData[ch]
			.ChAttribute.chNo_slave[0] - 1 ;  //1base
	}
	if(dataType == 0) {
		if(myData->bData[bd].cData[ch].ChAttribute.opType == P1)
		{
			if(CYCLER_TYPE == DIGITAL_CYC){		//210317 lyhw
				if((saveDt < RTTASK_1000MS) 
				&&(myData->bData[bd].cData[ch].op.runTime < save100ms_Limit)){ 
					myData->bData[bd].cData[ch].misc.preDataType = 1; 
					myData->save_msg[msg].val[idx][bd][ch].chData.Vsens
					= myData->bData[bd].cData[ch].misc.tmpVsens;
				} else {
					myData->bData[bd].cData[ch].misc.preDataType = 0; 
					myData->save_msg[msg].val[idx][bd][ch].chData.Vsens
						= myData->bData[bd].cData[ch].op.Vsens;
				}
			}else{
				//190124 lyhw
				if((saveDt < RTTASK_1000MS) 
				&&(myData->bData[bd].cData[ch].op.runTime < save100ms_Limit)){ 
					myData->bData[bd].cData[ch].misc.preDataType = 1; 
					myData->save_msg[msg].val[idx][bd][ch].chData.Vsens
					= myData->bData[bd].cData[slave_ch].misc.tmpVsens;
				} else {
					myData->bData[bd].cData[ch].misc.preDataType = 0; 
					myData->save_msg[msg].val[idx][bd][ch].chData.Vsens
						= myData->bData[bd].cData[slave_ch].op.Vsens;
				}
			}
			myData->save_msg[msg].val[idx][bd][ch].chData.Isens
				= myData->bData[bd].cData[ch].op.Isens
					+ myData->bData[bd].cData[slave_ch].op.Isens;
			myData->save_msg[msg].val[idx][bd][ch].chData.watt
				= myData->bData[bd].cData[ch].op.watt
					+ myData->bData[bd].cData[slave_ch].op.watt;
		} else {
			if(myData->bData[bd].cData[ch].op.phase == P0){	
				//210303 for 0s data
				if(myData->bData[bd].cData[ch].misc.preDataType == P0){
					myData->save_msg[msg].val[idx][bd][ch].chData.Vsens
						= myData->bData[bd].cData[ch].op.Vsens;
					myData->save_msg[msg].val[idx][bd][ch].chData.Isens //191118
						= myData->bData[bd].cData[ch].op.Isens;
				}else{
					myData->save_msg[msg].val[idx][bd][ch].chData.Vsens
						= myData->bData[bd].cData[ch].misc.tmpVsens;
					myData->save_msg[msg].val[idx][bd][ch].chData.Isens //191118
						= myData->bData[bd].cData[ch].misc.tmpIsens;
				}
			}else{
				if((saveDt < RTTASK_1000MS) 
				&&(myData->bData[bd].cData[ch].op.runTime < save100ms_Limit)){ 
					myData->bData[bd].cData[ch].misc.preDataType = 1; 
					myData->save_msg[msg].val[idx][bd][ch].chData.Vsens
						= myData->bData[bd].cData[ch].misc.tmpVsens;
					myData->save_msg[msg].val[idx][bd][ch].chData.Isens //191118
						= myData->bData[bd].cData[ch].misc.tmpIsens;
				} else {
					myData->bData[bd].cData[ch].misc.preDataType = 0; 
					myData->save_msg[msg].val[idx][bd][ch].chData.Vsens
						= myData->bData[bd].cData[ch].op.Vsens;
					myData->save_msg[msg].val[idx][bd][ch].chData.Isens //191118
						= myData->bData[bd].cData[ch].op.Isens;
				}
			}
		//	myData->save_msg[msg].val[idx][bd][ch].chData.Isens
		//		= myData->bData[bd].cData[ch].op.Isens;
			myData->save_msg[msg].val[idx][bd][ch].chData.watt
				= myData->bData[bd].cData[ch].op.watt;
		}
	} else {
		myData->bData[bd].cData[ch].misc.preDataType = 1; 
		if(myData->bData[bd].cData[ch].ChAttribute.opType == P1)
		{
			if(CYCLER_TYPE == DIGITAL_CYC){		//210317 lyhw
				myData->save_msg[msg].val[idx][bd][ch].chData.Vsens
					= myData->bData[bd].cData[ch].misc.tmpVsens;
			}else{
				myData->save_msg[msg].val[idx][bd][ch].chData.Vsens
					= myData->bData[bd].cData[slave_ch].misc.tmpVsens;
			}
			myData->save_msg[msg].val[idx][bd][ch].chData.Isens
				= myData->bData[bd].cData[ch].misc.tmpIsens
					+ myData->bData[bd].cData[slave_ch].misc.tmpIsens;
			myData->save_msg[msg].val[idx][bd][ch].chData.watt
				= myData->bData[bd].cData[ch].misc.tmpWatt
					+ myData->bData[bd].cData[slave_ch].misc.tmpWatt;

		} else {
			myData->save_msg[msg].val[idx][bd][ch].chData.Vsens
				= myData->bData[bd].cData[ch].misc.tmpVsens;
			myData->save_msg[msg].val[idx][bd][ch].chData.Isens
				= myData->bData[bd].cData[ch].misc.tmpIsens;
			myData->save_msg[msg].val[idx][bd][ch].chData.watt
				= myData->bData[bd].cData[ch].misc.tmpWatt;
		}
	}

	myData->save_msg[msg].val[idx][bd][ch].chData.select
		= myData->bData[bd].cData[ch].op.select;
	myData->save_msg[msg].val[idx][bd][ch].chData.code
		= myData->bData[bd].cData[ch].op.code;
	myData->save_msg[msg].val[idx][bd][ch].chData.stepNo
		= (unsigned char)myData->bData[bd].cData[ch].op.stepNo;

	if(myData->bData[bd].cData[ch].ChAttribute.opType == P1)
	{
		myData->save_msg[msg].val[idx][bd][ch].chData.wattHour
			= myData->bData[bd].cData[ch].op.wattHour
			+ myData->bData[bd].cData[slave_ch].op.wattHour;
		myData->save_msg[msg].val[idx][bd][ch].chData.avgI
			= myData->bData[bd].cData[ch].op.meanCurr
			+ myData->bData[bd].cData[slave_ch].op.meanCurr;
		myData->save_msg[msg].val[idx][bd][ch].chData.z
			= myData->bData[bd].cData[slave_ch].op.z;
		myData->save_msg[msg].val[idx][bd][ch].chData.capacity
			= myData->bData[bd].cData[ch].op.ampareHour
			+ myData->bData[bd].cData[slave_ch].op.ampareHour;
		//120818 kji SDI mes cycle data
		if(myData->mData.config.function[F_SDI_MES_USE] == P1){
			if(myData->bData[bd].cData[ch].op.type == STEP_LOOP){
				myData->save_msg[msg].val[idx][bd][ch].chData.Vsens
					= myData->bData[bd].cData[slave_ch].misc.lastRestVsens;
				myData->save_msg[msg].val[idx][bd][ch].chData.capacity
					= myData->bData[bd].cData[ch].misc.chargeCCAh
					+ myData->bData[bd].cData[slave_ch].misc.chargeCCAh;
				myData->save_msg[msg].val[idx][bd][ch].chData.z
					= myData->bData[bd].cData[ch].misc.chargeCVAh
					+ myData->bData[bd].cData[slave_ch].misc.chargeCVAh;
			}
		}
	} else {
#if CHAMBER_TEMP_HUMIDITY == 1	//kjc_210426
	if(type == 2){ 
		myData->save_msg[msg].val[idx][bd][ch].chData.watt
			= myData->bData[bd].cData[ch].misc.dischargeAccAh;
		myData->save_msg[msg].val[idx][bd][ch].chData.wattHour
			= myData->bData[bd].cData[ch].misc.faultEfficiencyAh;
		myData->save_msg[msg].val[idx][bd][ch].chData.avgI
			= myData->bData[bd].cData[ch].op.meanCurr;
		myData->save_msg[msg].val[idx][bd][ch].chData.z
			= myData->bData[bd].cData[ch].op.z;
		myData->save_msg[msg].val[idx][bd][ch].chData.capacity
			= myData->bData[bd].cData[ch].misc.chargeAccAh;
	}else{
		myData->save_msg[msg].val[idx][bd][ch].chData.wattHour
			= myData->bData[bd].cData[ch].op.wattHour;
		myData->save_msg[msg].val[idx][bd][ch].chData.avgI
			= myData->bData[bd].cData[ch].op.meanCurr;
		myData->save_msg[msg].val[idx][bd][ch].chData.z
			= myData->bData[bd].cData[ch].op.z;
		myData->save_msg[msg].val[idx][bd][ch].chData.capacity
			= myData->bData[bd].cData[ch].op.ampareHour;
	}
#else
		myData->save_msg[msg].val[idx][bd][ch].chData.wattHour
			= myData->bData[bd].cData[ch].op.wattHour;
		myData->save_msg[msg].val[idx][bd][ch].chData.avgI
			= myData->bData[bd].cData[ch].op.meanCurr;
		myData->save_msg[msg].val[idx][bd][ch].chData.z
			= myData->bData[bd].cData[ch].op.z;
		myData->save_msg[msg].val[idx][bd][ch].chData.capacity
			= myData->bData[bd].cData[ch].op.ampareHour;
#endif
	}

	myData->save_msg[msg].val[idx][bd][ch].chData.runTime
		= myData->bData[bd].cData[ch].op.runTime;
	myData->save_msg[msg].val[idx][bd][ch].chData.totalRunTime
		= myData->bData[bd].cData[ch].op.totalRunTime;
	myData->save_msg[msg].val[idx][bd][ch].chData.temp
			= myData->bData[bd].cData[ch].op.temp;
	myData->save_msg[msg].val[idx][bd][ch].chData.reservedCmd
		= myData->bData[bd].cData[ch].op.reservedCmd;

	myData->save_msg[msg].val[idx][bd][ch].chData.totalCycle
		= myData->bData[bd].cData[ch].misc.totalCycle;
	myData->save_msg[msg].val[idx][bd][ch].chData.currentCycle
		= myData->bData[bd].cData[ch].misc.currentCycle;
	myData->save_msg[msg].val[idx][bd][ch].chData.gotoCycleCount
		= myData->bData[bd].cData[ch].misc.gotoCycleCount[advStepNo];
	
	myData->save_msg[msg].val[idx][bd][ch].chData.avgV
		= myData->bData[bd].cData[ch].op.meanVolt;
		
/*
	if(myData->bData[bd].cData[ch].op.select == SAVE_FLAG_SAVING_END
		&& (myData->bData[bd].cData[ch].op.type == STEP_CHARGE
		|| myData->bData[bd].cData[ch].op.type == STEP_DISCHARGE
		|| myData->bData[bd].cData[ch].op.type == STEP_Z
		|| myData->bData[bd].cData[ch].op.type == STEP_OCV
		|| myData->bData[bd].cData[ch].op.type == STEP_REST)) {
		stepNo = (int)myData->bData[bd].cData[ch].op.stepNo - 1;
		item = myPs->testCond[bd][ch].step[stepNo].grade[0].item;
		if(item == GRADE_V) {
			val = myData->save_msg[msg].val[idx][bd][ch].chData.Vsens;
			myData->save_msg[msg].val[idx][bd][ch].chData.grade =
				(unsigned char)GradeCodeCheck(bd, ch, stepNo, val);
		} else if(item == GRADE_CAPACITY) {
			val = myData->save_msg[msg].val[idx][bd][ch].chData.capacity;
			myData->save_msg[msg].val[idx][bd][ch].chData.grade =
				(unsigned char)GradeCodeCheck(bd, ch, stepNo, val);
		} else if(item == GRADE_Z) {
			val = myData->save_msg[msg].val[idx][bd][ch].chData.z;
			myData->save_msg[msg].val[idx][bd][ch].chData.grade =
				(unsigned char)GradeCodeCheck(bd, ch, stepNo, val);
		} else {
			myData->save_msg[msg].val[idx][bd][ch].chData.grade = 0;
		}
	}*/
	myData->save_msg[msg].val[idx][bd][ch].chData.grade 
		= myData->bData[bd].cData[ch].op.grade;
	myData->save_msg[msg].write_idx[bd][ch] = idx;
	myData->save_msg[msg].count[bd][ch]++;
	// 090903 jyk
#if NETWORK_VERSION >= 4102	
	#if VENDER != 2 //NOT SDI
	if(myData->bData[bd].cData[ch].ChAttribute.opType == P1)
	{
		myData->save_msg[msg].val[idx][bd][ch].chData.IntegralAmpareHour = 
			myData->bData[bd].cData[ch].op.integral_ampareHour
			+ myData->bData[bd].cData[slave_ch].op.integral_ampareHour;
		myData->save_msg[msg].val[idx][bd][ch].chData.IntegralWattHour =
			myData->bData[bd].cData[ch].op.integral_WattHour
			+ myData->bData[bd].cData[slave_ch].op.integral_WattHour;
		myData->save_msg[msg].val[idx][bd][ch].chData.ChargeAmpareHour =
			myData->bData[bd].cData[ch].op.charge_ampareHour
			+ myData->bData[bd].cData[slave_ch].op.charge_ampareHour;
		myData->save_msg[msg].val[idx][bd][ch].chData.ChargeWattHour =
			myData->bData[bd].cData[ch].op.charge_wattHour
			+ myData->bData[bd].cData[slave_ch].op.charge_wattHour;
		myData->save_msg[msg].val[idx][bd][ch].chData.DischargeAmpareHour =
			myData->bData[bd].cData[ch].op.discharge_ampareHour
			+ myData->bData[bd].cData[slave_ch].op.discharge_ampareHour;
		myData->save_msg[msg].val[idx][bd][ch].chData.DischargeWattHour =
			myData->bData[bd].cData[ch].op.discharge_wattHour
			+ myData->bData[bd].cData[slave_ch].op.discharge_wattHour;
		if(myData->bData[bd].cData[ch].misc.cvTime > 
				myData->bData[bd].cData[slave_ch].misc.cvTime) {
			myData->save_msg[msg].val[idx][bd][ch].chData.cvTime = 
				myData->bData[bd].cData[ch].misc.cvTime ;
		} else {
			myData->save_msg[msg].val[idx][bd][ch].chData.cvTime = 
				myData->bData[bd].cData[slave_ch].misc.cvTime ;
		}
	} else {
		myData->save_msg[msg].val[idx][bd][ch].chData.IntegralAmpareHour
			= myData->bData[bd].cData[ch].op.integral_ampareHour;
		myData->save_msg[msg].val[idx][bd][ch].chData.IntegralWattHour
			= myData->bData[bd].cData[ch].op.integral_WattHour;
		myData->save_msg[msg].val[idx][bd][ch].chData.ChargeAmpareHour
			= myData->bData[bd].cData[ch].op.charge_ampareHour;
		myData->save_msg[msg].val[idx][bd][ch].chData.ChargeWattHour
			= myData->bData[bd].cData[ch].op.charge_wattHour;
		myData->save_msg[msg].val[idx][bd][ch].chData.DischargeAmpareHour
			= myData->bData[bd].cData[ch].op.discharge_ampareHour;
		myData->save_msg[msg].val[idx][bd][ch].chData.DischargeWattHour
			= myData->bData[bd].cData[ch].op.discharge_wattHour;
		myData->save_msg[msg].val[idx][bd][ch].chData.cvTime
			= myData->bData[bd].cData[ch].misc.cvTime;
	}
	// SKI Data Restore hun_201010
	#ifdef _AC_FAIL_RECOVERY 
	myData->save_msg[msg].val[idx][bd][ch].chData.advCycle
		= myData->bData[bd].cData[ch].misc.advCycle;
	myData->save_msg[msg].val[idx][bd][ch].chData.advCycleStep
		= myData->bData[bd].cData[ch].misc.advCycleStep;
	myData->save_msg[msg].val[idx][bd][ch].chData.cycleRunTime
		= myData->bData[bd].cData[ch].misc.cycleRunTime;
	myData->save_msg[msg].val[idx][bd][ch].chData.seedintegralCapacity
		= myData->bData[bd].cData[ch].misc.seedintegralCapacity;
	myData->save_msg[msg].val[idx][bd][ch].chData.sumintegralCapacity
		= myData->bData[bd].cData[ch].misc.sumintegralCapacity;
	myData->save_msg[msg].val[idx][bd][ch].chData.seedChargeAmpareHour
		= myData->bData[bd].cData[ch].misc.seedChargeAmpareHour;
	myData->save_msg[msg].val[idx][bd][ch].chData.sumChargeAmpareHour
		= myData->bData[bd].cData[ch].misc.sumChargeAmpareHour;
	myData->save_msg[msg].val[idx][bd][ch].chData.seedDischargeAmpareHour
		= myData->bData[bd].cData[ch].misc.seedDischargeAmpareHour;
	myData->save_msg[msg].val[idx][bd][ch].chData.sumDischargeAmpareHour
		= myData->bData[bd].cData[ch].misc.sumDischargeAmpareHour;
	myData->save_msg[msg].val[idx][bd][ch].chData.seedintegralWattHour
		= myData->bData[bd].cData[ch].misc.seedintegralWattHour;
	myData->save_msg[msg].val[idx][bd][ch].chData.sumintegralWattHour
		= myData->bData[bd].cData[ch].misc.sumintegralWattHour;
	myData->save_msg[msg].val[idx][bd][ch].chData.seedChargeWattHour
		= myData->bData[bd].cData[ch].misc.seedChargeWattHour;
	myData->save_msg[msg].val[idx][bd][ch].chData.sumChargeWattHour
		= myData->bData[bd].cData[ch].misc.sumChargeWattHour;
	myData->save_msg[msg].val[idx][bd][ch].chData.seedDischargeWattHour
		= myData->bData[bd].cData[ch].misc.seedDischargeWattHour;
	myData->save_msg[msg].val[idx][bd][ch].chData.sumDischargeWattHour
		= myData->bData[bd].cData[ch].misc.sumDischargeWattHour;
	myData->save_msg[msg].val[idx][bd][ch].chData.standardC
		= myData->bData[bd].cData[ch].misc.standardC;
	myData->save_msg[msg].val[idx][bd][ch].chData.standardP
		= myData->bData[bd].cData[ch].misc.standardP;			
	myData->save_msg[msg].val[idx][bd][ch].chData.standardZ
		= myData->bData[bd].cData[ch].misc.standardZ;			
	myData->save_msg[msg].val[idx][bd][ch].chData.cycleSumC
		= myData->bData[bd].cData[ch].misc.cycleSumC;
	myData->save_msg[msg].val[idx][bd][ch].chData.cycleSumP
		= myData->bData[bd].cData[ch].misc.cycleSumP;						
	myData->save_msg[msg].val[idx][bd][ch].chData.cycleEndC
		= myData->bData[bd].cData[ch].misc.cycleEndC;						
	myData->save_msg[msg].val[idx][bd][ch].chData.pattern_change_flag
		= myData->bData[bd].cData[ch].misc.pattern_change_flag;		
	myData->save_msg[msg].val[idx][bd][ch].chData.chGroupNo
		= myData->bData[bd].cData[ch].misc.chGroupNo;		
	myData->save_msg[msg].val[idx][bd][ch].chData.tempDir
		= myData->bData[bd].cData[ch].misc.tempDir;		
	#endif

		#if PROGRAM_VERSION1 == 0
			#if PROGRAM_VERSION2 >= 1
	if(myData->bData[bd].cData[ch].ChAttribute.opType == P1) {
		myData->save_msg[msg].val[idx][bd][ch].chData.Farad
			= myData->bData[bd].cData[ch].op.capacitance
			+ myData->bData[bd].cData[slave_ch].op.capacitance;
	} else {
		myData->save_msg[msg].val[idx][bd][ch].chData.Farad
			= myData->bData[bd].cData[ch].op.capacitance;
	}
	myData->save_msg[msg].val[idx][bd][ch].chData.totalRunTime_carry
		= myData->bData[bd].cData[ch].op.totalRunTime_carry;
	myData->save_msg[msg].val[idx][bd][ch].chData.cycleNo
		= myData->bData[bd].cData[ch].misc.cycleNo;
//	120109 oys w : One Ch -> Temp Module Multi Ch Input
	if(myData->mData.config.ambient2 == 0){	//INC/LGES_Patch_Note.txt Check
		myData->save_msg[msg].val[idx][bd][ch].chData.temp1
			= myData->bData[bd].cData[ch].op.temp1;
	}else if(myData->mData.config.ambient2 != 0){
		myData->save_msg[msg].val[idx][bd][ch].chData.temp1
			= myData->bData[bd].cData[ch].misc.ambientTemp;
	}
			#endif
			#if EDLC_TYPE == 1
	//20160229 khk add start
	myData->save_msg[msg].val[idx][bd][ch].chData.c_t1
		= myData->bData[bd].cData[ch].misc.c_t1;	
	myData->save_msg[msg].val[idx][bd][ch].chData.c_v1
		= myData->bData[bd].cData[ch].misc.c_v1;	
	myData->save_msg[msg].val[idx][bd][ch].chData.c_t2
		= myData->bData[bd].cData[ch].misc.c_t2;	
	myData->save_msg[msg].val[idx][bd][ch].chData.c_v2
		= myData->bData[bd].cData[ch].misc.c_v2;	
	//20160229 khk add end
	//20180206 sch add start
	myData->save_msg[msg].val[idx][bd][ch].chData.capacitance_iec
		= myData->bData[bd].cData[ch].op.capacitance_iec;	
	myData->save_msg[msg].val[idx][bd][ch].chData.capacitance_maxwell
		= myData->bData[bd].cData[ch].op.capacitance_maxwell;	
			#endif
			#if PROGRAM_VERSION2 >= 2
	if(myData->bData[bd].cData[ch].ChAttribute.opType == P1)
	{
		myData->save_msg[msg].val[idx][bd][ch].chData.chargeCCAh
			= myData->bData[bd].cData[ch].misc.chargeCCAh
			+ myData->bData[bd].cData[slave_ch].misc.chargeCCAh;
		myData->save_msg[msg].val[idx][bd][ch].chData.chargeCVAh
			= myData->bData[bd].cData[ch].misc.chargeCVAh
			+ myData->bData[bd].cData[slave_ch].misc.chargeCVAh;
		myData->save_msg[msg].val[idx][bd][ch].chData.dischargeCCAh
			= myData->bData[bd].cData[ch].misc.dischargeCCAh
			+ myData->bData[bd].cData[slave_ch].misc.dischargeCCAh;
		myData->save_msg[msg].val[idx][bd][ch].chData.dischargeCVAh
			= myData->bData[bd].cData[ch].misc.dischargeCVAh
			+ myData->bData[bd].cData[slave_ch].misc.dischargeCVAh;
		myData->save_msg[msg].val[idx][bd][ch].chData.z_100mS
			= myData->bData[bd].cData[slave_ch].op.z_100mS;
		myData->save_msg[msg].val[idx][bd][ch].chData.z_1S
			= myData->bData[bd].cData[slave_ch].op.z_1S;
		myData->save_msg[msg].val[idx][bd][ch].chData.z_5S
			= myData->bData[bd].cData[slave_ch].op.z_5S;
		myData->save_msg[msg].val[idx][bd][ch].chData.z_30S
			= myData->bData[bd].cData[slave_ch].op.z_30S;
		myData->save_msg[msg].val[idx][bd][ch].chData.z_60S
			= myData->bData[bd].cData[slave_ch].op.z_60S;
	}else{
		myData->save_msg[msg].val[idx][bd][ch].chData.chargeCCAh
			= myData->bData[bd].cData[ch].misc.chargeCCAh;
		myData->save_msg[msg].val[idx][bd][ch].chData.chargeCVAh
			= myData->bData[bd].cData[ch].misc.chargeCVAh;
		myData->save_msg[msg].val[idx][bd][ch].chData.dischargeCCAh
			= myData->bData[bd].cData[ch].misc.dischargeCCAh;
		myData->save_msg[msg].val[idx][bd][ch].chData.dischargeCVAh
			= myData->bData[bd].cData[ch].misc.dischargeCVAh;
		myData->save_msg[msg].val[idx][bd][ch].chData.z_100mS
			= myData->bData[bd].cData[ch].op.z_100mS;
		myData->save_msg[msg].val[idx][bd][ch].chData.z_1S
			= myData->bData[bd].cData[ch].op.z_1S;
		myData->save_msg[msg].val[idx][bd][ch].chData.z_5S
			= myData->bData[bd].cData[ch].op.z_5S;
		myData->save_msg[msg].val[idx][bd][ch].chData.z_30S
			= myData->bData[bd].cData[ch].op.z_30S;
		myData->save_msg[msg].val[idx][bd][ch].chData.z_60S
			= myData->bData[bd].cData[ch].op.z_60S;
	}
			#endif
		#endif
		#if PROGRAM_VERSION1 > 0
	if(myData->bData[bd].cData[ch].op.type != STEP_END) {
		if(myData->save_msg[msg].val[idx][bd][ch].chData.Vsens
				> myData->bData[bd].cData[ch].misc.maxV) {
			myData->bData[bd].cData[ch].misc.maxV = 
				myData->save_msg[msg].val[idx][bd][ch].chData.Vsens;
		}
		if(myData->save_msg[msg].val[idx][bd][ch].chData.Vsens
				< myData->bData[bd].cData[ch].misc.minV) {
			myData->bData[bd].cData[ch].misc.minV = 
				myData->save_msg[msg].val[idx][bd][ch].chData.Vsens;
		}
		if(myData->save_msg[msg].val[idx][bd][ch].chData.temp
				> myData->bData[bd].cData[ch].misc.maxT) {
			myData->bData[bd].cData[ch].misc.maxT = 
				myData->save_msg[msg].val[idx][bd][ch].chData.temp;
		}
		if(myData->save_msg[msg].val[idx][bd][ch].chData.temp
				< myData->bData[bd].cData[ch].misc.minT) {
			myData->bData[bd].cData[ch].misc.minT = 
				myData->save_msg[msg].val[idx][bd][ch].chData.temp;
		}
	}
	
	if(myData->bData[bd].cData[ch].ChAttribute.opType == P1)
		myData->save_msg[msg].val[idx][bd][ch].chData.Farad
			= myData->bData[bd].cData[ch].op.capacitance
			+ myData->bData[bd].cData[slave_ch].op.capacitance;
	}else{
		myData->save_msg[msg].val[idx][bd][ch].chData.Farad
			= myData->bData[bd].cData[ch].op.capacitance
	}
	myData->save_msg[msg].val[idx][bd][ch].chData.totalRunTime_carry
		= myData->bData[bd].cData[ch].op.totalRunTime_carry;
	myData->save_msg[msg].val[idx][bd][ch].chData.cycleNo
		= myData->bData[bd].cData[ch].misc.cycleNo;
//	120109 oys w : One Ch -> Temp Module Multi Ch Input
	myData->save_msg[msg].val[idx][bd][ch].chData.temp1
		= myData->bData[bd].cData[ch].op.temp1;
	myData->save_msg[msg].val[idx][bd][ch].chData.startVoltage
		= myData->bData[bd].cData[ch].misc.startV;
	myData->save_msg[msg].val[idx][bd][ch].chData.maxVoltage
		= myData->bData[bd].cData[ch].misc.maxV;
	myData->save_msg[msg].val[idx][bd][ch].chData.minVoltage
		= myData->bData[bd].cData[ch].misc.minV;
	myData->save_msg[msg].val[idx][bd][ch].chData.startTemp
		= myData->bData[bd].cData[ch].misc.startT;
	myData->save_msg[msg].val[idx][bd][ch].chData.maxTemp
		= myData->bData[bd].cData[ch].misc.maxT;
	myData->save_msg[msg].val[idx][bd][ch].chData.minTemp
		= myData->bData[bd].cData[ch].misc.minT;	
		#endif	
	#endif

	//131228 oys w : real time add
	#if REAL_TIME == 1
	myData->save_msg[msg].val[idx][bd][ch].chData.realDate
		= myData->mData.real_time[6] * 10000	//year
		+ myData->mData.real_time[5] * 100		//month
		+ myData->mData.real_time[4];			//day
	myData->save_msg[msg].val[idx][bd][ch].chData.realClock
		= myData->mData.real_time[3] * 10000000	//hour
		+ myData->mData.real_time[2] * 100000	//min
		+ myData->mData.real_time[1] * 1000		//sec
		+ myData->mData.real_time[0];			//msec
	#endif
	#ifdef _CH_CHAMBER_DATA		//210316 lyhw
	myData->save_msg[msg].val[idx][bd][ch].chData.Chamber_Temp =
			myData->bData[bd].cData[ch].misc.groupTemp;
	#endif
	
	#if CHAMBER_TEMP_HUMIDITY == 1	//210227_hun
	myData->save_msg[msg].val[idx][bd][ch].chData.Chamber_Temp 
		= myData->bData[bd].cData[ch].misc.groupTemp;
	myData->save_msg[msg].val[idx][bd][ch].chData.humi
		= myData->bData[bd].cData[ch].misc.humi;
	myData->save_msg[msg].val[idx][bd][ch].chData.chargeAccAh
		= myData->bData[bd].cData[ch].misc.chargeAccAh;
	myData->save_msg[msg].val[idx][bd][ch].chData.dischargeAccAh
		= myData->bData[bd].cData[ch].misc.dischargeAccAh;
	myData->save_msg[msg].val[idx][bd][ch].chData.EfficiencyAh
		= myData->bData[bd].cData[ch].misc.faultEfficiencyAh;
	#endif
	#if VENDER == 1 && CH_AUX_DATA == 1			//190807 pthw
	for(aux_i = 0; aux_i < MAX_CH_AUX_DATA; aux_i++){
		myData->save_msg[msg].val[idx][bd][ch].chData.ch_AuxTemp[aux_i]
			= myData->bData[bd].cData[ch].misc.chAuxTemp[aux_i];
		myData->save_msg[msg].val[idx][bd][ch].chData.ch_AuxVoltage[aux_i]
			= myData->bData[bd].cData[ch].misc.chAuxVoltage[aux_i];
	}
	#endif
	#if VENDER == 3 				//20200629 ONLY SK
		myData->save_msg[msg].val[idx][bd][ch].chData.Chamber_Temp =
			myData->bData[bd].cData[ch].misc.groupTemp;
	#endif
	#ifdef _CH_SWELLING_DATA 
	for(i = 0; i < MAX_CH_PRESSURE_DATA; i++){
		myData->save_msg[msg].val[idx][bd][ch].chData.PressureData[i] =
				myData->bData[bd].cData[ch].misc.chPressure[i];
	}
	for(i = 0; i < MAX_CH_THICKNESS_DATA; i++){
		myData->save_msg[msg].val[idx][bd][ch].chData.ThicknessData[i] =
				myData->bData[bd].cData[ch].misc.chThickness[i];
	}
	#endif
	#if CH_SWELLING_DATA == 1			//210316 NV use lyhw
	for(i = 0; i < MAX_CH_PRESSURE_DATA; i++){
		myData->save_msg[msg].val[idx][bd][ch].chData.PressureData[i] =
				myData->bData[bd].cData[ch].misc.chPressure[i];
	}
	for(i = 0; i < MAX_CH_THICKNESS_DATA; i++){
		myData->save_msg[msg].val[idx][bd][ch].chData.ThicknessData[i] =
				myData->bData[bd].cData[ch].misc.chThickness[i];
	}
	#endif
	//hun_210723
	#ifdef _EXTERNAL_CONTROL
	myData->save_msg[msg].val[idx][bd][ch].chData.chControl
	 = myData->bData[bd].cData[ch].misc.chControl;
	myData->save_msg[msg].val[idx][bd][ch].chData.chPause
	 = myData->bData[bd].cData[ch].misc.chPause;
	myData->save_msg[msg].val[idx][bd][ch].chData.chCV
	 = myData->bData[bd].cData[ch].misc.cvFlag;
	myData->save_msg[msg].val[idx][bd][ch].chData.external_return
	 = myData->bData[bd].cData[ch].misc.external_return;
	#endif
	 /*	노스볼트 사용으로 인해 주석 처리 by 김장훈
	//210318 lyhw
	//just Send Data, not Save Data
	#ifdef _GUI_OPCUA_TYPE
		myData->save_msg[msg].val[idx][bd][ch].chData.Acc_Capacity =
			myData->bData[bd].cData[ch].misc.Accumulated_Capacity;
		myData->save_msg[msg].val[idx][bd][ch].chData.Acc_WattHour =
			myData->bData[bd].cData[ch].misc.Accumulated_WattHour;
	#endif
	*/
	#ifdef _AMBIENT_GAS_FLAG 	//hun_211013
		myData->save_msg[msg].val[idx][bd][ch].chData.ambientTemp
			= myData->bData[bd].cData[ch].misc.ambientTemp;
		myData->save_msg[msg].val[idx][bd][ch].chData.gasVoltage
			= myData->bData[bd].cData[ch].misc.gasVoltage;
	#endif
	#ifdef _TRACKING_MODE
		myData->save_msg[msg].val[idx][bd][ch].chData.SOC =
			myData->bData[bd].cData[ch].op.SOC;
		myData->save_msg[msg].val[idx][bd][ch].chData.RPT_SOC =
			myData->bData[bd].cData[ch].op.ampareHour_SOC;
		myData->save_msg[msg].val[idx][bd][ch].chData.SOH = //211022
			myData->bData[bd].cData[ch].op.SOH;
		myData->save_msg[msg].val[idx][bd][ch].chData.RPT_SOH =
			myData->bData[bd].cData[ch].op.ampareHour_SOH;
		myData->save_msg[msg].val[idx][bd][ch].chData.socRefStep =
			myData->bData[bd].cData[ch].misc.socTrackingStep;
		myData->save_msg[msg].val[idx][bd][ch].chData.sohRefStep =
			myData->bData[bd].cData[ch].misc.sohTrackingStep;
		myData->save_msg[msg].val[idx][bd][ch].chData.chamberNoWaitFlag =
			myData->bData[bd].cData[ch].misc.chamberNoWaitFlag;
	#endif
	#if GAS_DATA_CONTROL == 1 //210923 lyhw
		myData->save_msg[msg].val[idx][bd][ch].chData.gas_eCo2
			= myData->bData[bd].cData[ch].misc.gas_eCo2;
		myData->save_msg[msg].val[idx][bd][ch].chData.gas_Temp
			= myData->bData[bd].cData[ch].misc.gas_Temp;
		myData->save_msg[msg].val[idx][bd][ch].chData.gas_AH
			= myData->bData[bd].cData[ch].misc.gas_AH;
		myData->save_msg[msg].val[idx][bd][ch].chData.gas_Baseline
			= myData->bData[bd].cData[ch].misc.gas_Baseline;
		myData->save_msg[msg].val[idx][bd][ch].chData.gas_TVOC
			= myData->bData[bd].cData[ch].misc.gas_TVOC;
		myData->save_msg[msg].val[idx][bd][ch].chData.gas_Ethanol
			= myData->bData[bd].cData[ch].misc.gas_Ethanol;
		myData->save_msg[msg].val[idx][bd][ch].chData.gas_H2
			= myData->bData[bd].cData[ch].misc.gas_H2;
	#endif
	#if VENDER == 3 && CH_AUX_DATA == 1			//211027
	for(aux_i = 0; aux_i < MAX_CH_AUX_DATA; aux_i++){
		myData->save_msg[msg].val[idx][bd][ch].chData.ch_AuxTemp[aux_i]
			= myData->bData[bd].cData[ch].misc.chAuxTemp[aux_i];
		myData->save_msg[msg].val[idx][bd][ch].chData.ch_AuxVoltage[aux_i]
			= myData->bData[bd].cData[ch].misc.chAuxVoltage[aux_i];
	}
	#endif
#endif
}
#endif

#ifdef __SDI_MES_VER4__
void send_save_msg_2(int bd, int ch, unsigned long saveDt, int type, int msg)
{
	unsigned char stepType, slave_ch = 0;
	unsigned short advStepNo;
	int idx, dataType;
	int save100ms_Limit;
	#ifdef _CH_SWELLING_DATA 		//211025 hun
	int i;
	#endif

	stepType = myData->bData[bd].cData[ch].op.type;
	advStepNo = myData->bData[bd].cData[ch].misc.advStepNo;
	//190124 add lyhw
	if(myData->AppControl.config.versionNo >= 20150402){
		save100ms_Limit = myData->DataSave.config.save_100ms_time;
	}else{
		save100ms_Limit = 6000;
	}

	if(myData->bData[bd].cData[ch].op.type == STEP_USER_PATTERN)
		dataType = 1; //tmpVsens, tmpIsens
	else if(myData->bData[bd].cData[ch].op.type == STEP_USER_MAP)
		dataType = 1; //tmpVsens, tmpIsens
	else if(myData->bData[bd].cData[ch].op.mode == CP)
		dataType = 1; //tmpVsens, tmpIsens
	else if(myData->bData[bd].cData[ch].op.runTime < 100)
		dataType = 1; //tmpVsens, tmpIsens
	else
		dataType = 0;
	
	if(myPs->config.hwSpec == L_5V_200A_1CH_JIG) {
		dataType = 1; //tmpVsens, tmpIsens
	}
	if(myData->save_msg[msg].flag == 1) { //send save_msg stop
		return;
	}
	
	idx = myData->save_msg[msg].write_idx[bd][ch];
	idx++;
	
	if(idx >= MAX_SAVE_MSG) idx = 0;

	if(msg == 1){
		if(myData->mData.config.installedTemp > 0
			|| myData->mData.config.installedAuxV > 0){
			send_save_msg_aux(bd, ch, idx, msg); //auxDataSave
		}
	}
	
	myData->save_msg[msg].val[idx][bd][ch].chData.resultIndex
		= myData->bData[bd].cData[ch].op.resultIndex;
	myData->save_msg[msg].val[idx][bd][ch].chData.state
		= myData->bData[bd].cData[ch].op.state;
	myData->save_msg[msg].val[idx][bd][ch].chData.type
		= myData->bData[bd].cData[ch].op.type;
	myData->save_msg[msg].val[idx][bd][ch].chData.mode
		= myData->bData[bd].cData[ch].op.mode;
	myData->save_msg[msg].val[idx][bd][ch].chData.select
		= myData->bData[bd].cData[ch].op.select;
//	myData->save_msg[msg].val[idx][bd][ch].chData.grade
//		= myData->bData[bd].cData[ch].op.grade;
	myData->save_msg[msg].val[idx][bd][ch].chData.cvFlag 
		= myData->bData[bd].cData[ch].misc.cvFlag;

//	일반형 Cycler에서 안쓰는 항목들.
	myData->save_msg[msg].val[idx][bd][ch].chData.switchState[0]
		= 0;
	myData->save_msg[msg].val[idx][bd][ch].chData.switchState[1]
		= 0;
	myData->save_msg[msg].val[idx][bd][ch].chData.chamber_control
		= 0;
	myData->save_msg[msg].val[idx][bd][ch].chData.record_index
		= 0;
	myData->save_msg[msg].val[idx][bd][ch].chData.input_val
		= 0;
	myData->save_msg[msg].val[idx][bd][ch].chData.output_val
		= 0;
	myData->save_msg[msg].val[idx][bd][ch].chData.Vinput
		= 0;
	myData->save_msg[msg].val[idx][bd][ch].chData.Vpower
		= 0;
	myData->save_msg[msg].val[idx][bd][ch].chData.Vbus
		= 0;
//

	myData->save_msg[msg].val[idx][bd][ch].chData.code
		= (short int)myData->bData[bd].cData[ch].op.code;
	myData->save_msg[msg].val[idx][bd][ch].chData.stepNo
		= (short int)myData->bData[bd].cData[ch].op.stepNo;

	if(myData->bData[bd].cData[ch].ChAttribute.opType == P1)
	{
		slave_ch = myData->bData[bd].cData[ch]
			.ChAttribute.chNo_slave[0] - 1;  //1base
	}
	if(dataType == 0) {
		if(myData->bData[bd].cData[ch].ChAttribute.opType == P1){
			/*	
			if((myData->mData.config.rt_scan_type == RT_SCAN_PERIOD_100mS)
				&& (saveDt < RTTASK_1000MS)) {
				myData->save_msg[msg].val[idx][bd][ch].chData.Vsens
					= myData->bData[bd].cData[slave_ch].misc.tmpVsens;
			}*/
			//190124 lyhw
			if((saveDt < RTTASK_1000MS) 
				&&(myData->bData[bd].cData[ch].op.runTime < save100ms_Limit)){ 
				myData->save_msg[msg].val[idx][bd][ch].chData.Vsens
					= myData->bData[bd].cData[slave_ch].misc.tmpVsens;
			} else {
				myData->save_msg[msg].val[idx][bd][ch].chData.Vsens
					= myData->bData[bd].cData[slave_ch].op.Vsens;
			}
			myData->save_msg[msg].val[idx][bd][ch].chData.Isens
				= myData->bData[bd].cData[ch].op.Isens
					+ myData->bData[bd].cData[slave_ch].op.Isens;
		}else{
			/*
			if((myData->mData.config.rt_scan_type == RT_SCAN_PERIOD_100mS)
				&& (saveDt < RTTASK_1000MS)) {
				myData->save_msg[msg].val[idx][bd][ch].chData.Vsens
					= myData->bData[bd].cData[ch].misc.tmpVsens;
			}*/
			//190124 lyhw
			if((saveDt < RTTASK_1000MS) 
				&&(myData->bData[bd].cData[ch].op.runTime < save100ms_Limit)){ 
				myData->save_msg[msg].val[idx][bd][ch].chData.Vsens
					= myData->bData[bd].cData[ch].misc.tmpVsens;
			} else {
				myData->save_msg[msg].val[idx][bd][ch].chData.Vsens
					= myData->bData[bd].cData[ch].op.Vsens;
			}
			myData->save_msg[msg].val[idx][bd][ch].chData.Isens
				= myData->bData[bd].cData[ch].op.Isens;
		}
	}else{
		if(myData->bData[bd].cData[ch].ChAttribute.opType == P1) {
			myData->save_msg[msg].val[idx][bd][ch].chData.Vsens
				= myData->bData[bd].cData[slave_ch].misc.tmpVsens;
			myData->save_msg[msg].val[idx][bd][ch].chData.Isens
				= myData->bData[bd].cData[ch].misc.tmpIsens
					+ myData->bData[bd].cData[slave_ch].misc.tmpIsens;
		} else {
			myData->save_msg[msg].val[idx][bd][ch].chData.Vsens
				= myData->bData[bd].cData[ch].misc.tmpVsens;
			myData->save_msg[msg].val[idx][bd][ch].chData.Isens
				= myData->bData[bd].cData[ch].misc.tmpIsens;
		}
	}
//	스텝 진행시간
	if(myData->bData[bd].cData[ch].op.runTime >= ONE_DAY_RUNTIME){
		myData->save_msg[msg].val[idx][bd][ch].chData.runTime_day
			= myData->bData[bd].cData[ch].op.runTime / ONE_DAY_RUNTIME;	
		myData->save_msg[msg].val[idx][bd][ch].chData.runTime
			= myData->bData[bd].cData[ch].op.runTime % ONE_DAY_RUNTIME;	
	}else{
		myData->save_msg[msg].val[idx][bd][ch].chData.runTime_day
			= 0;	
		myData->save_msg[msg].val[idx][bd][ch].chData.runTime
			= myData->bData[bd].cData[ch].op.runTime;	
	}
//	누적 총 진행시간
	if(myData->bData[bd].cData[ch].op.totalRunTime >= ONE_DAY_RUNTIME){
		myData->save_msg[msg].val[idx][bd][ch].chData.totalRunTime_carry
			= myData->bData[bd].cData[ch].op.totalRunTime / ONE_DAY_RUNTIME;	
		myData->save_msg[msg].val[idx][bd][ch].chData.totalRunTime
			= myData->bData[bd].cData[ch].op.totalRunTime % ONE_DAY_RUNTIME;
	}else{
		myData->save_msg[msg].val[idx][bd][ch].chData.totalRunTime_carry
			= 0;	
		myData->save_msg[msg].val[idx][bd][ch].chData.totalRunTime
			= myData->bData[bd].cData[ch].op.totalRunTime;
	}
//	CC 시간
	if(myData->bData[bd].cData[ch].misc.ccTime >= ONE_DAY_RUNTIME){
		myData->save_msg[msg].val[idx][bd][ch].chData.ccTime_day
			= myData->bData[bd].cData[ch].misc.ccTime / ONE_DAY_RUNTIME;	
		myData->save_msg[msg].val[idx][bd][ch].chData.ccTime
			= myData->bData[bd].cData[ch].misc.ccTime % ONE_DAY_RUNTIME;	
	}else{
		myData->save_msg[msg].val[idx][bd][ch].chData.ccTime_day
			= 0;	
		myData->save_msg[msg].val[idx][bd][ch].chData.ccTime
			= myData->bData[bd].cData[ch].misc.ccTime;	
	}
//	CV 시간
	if(myData->bData[bd].cData[ch].misc.cvTime >= ONE_DAY_RUNTIME){
		myData->save_msg[msg].val[idx][bd][ch].chData.cvTime_day
			= myData->bData[bd].cData[ch].misc.cvTime / ONE_DAY_RUNTIME;	
		myData->save_msg[msg].val[idx][bd][ch].chData.cvTime
			= myData->bData[bd].cData[ch].misc.cvTime % ONE_DAY_RUNTIME;	
	}else{
		myData->save_msg[msg].val[idx][bd][ch].chData.cvTime_day
			= 0;	
		myData->save_msg[msg].val[idx][bd][ch].chData.cvTime
			= myData->bData[bd].cData[ch].misc.cvTime;	
	}

	myData->save_msg[msg].val[idx][bd][ch].chData.z
		= myData->bData[bd].cData[ch].op.z;

	myData->save_msg[msg].val[idx][bd][ch].chData.temp[0][0]
		= myData->bData[bd].cData[ch].op.temp;
	myData->save_msg[msg].val[idx][bd][ch].chData.temp[0][3]
		= myData->bData[bd].cData[ch].op.meanTemp;

	myData->save_msg[msg].val[idx][bd][ch].chData.reservedCmd
		= myData->bData[bd].cData[ch].op.reservedCmd;

	myData->save_msg[msg].val[idx][bd][ch].chData.gotoCycleCount
		= myData->bData[bd].cData[ch].misc.gotoCycleCount[advStepNo];
	myData->save_msg[msg].val[idx][bd][ch].chData.totalCycle
		= myData->bData[bd].cData[ch].misc.totalCycle;
	myData->save_msg[msg].val[idx][bd][ch].chData.currentCycle
		= myData->bData[bd].cData[ch].misc.currentCycle;
	
	myData->save_msg[msg].val[idx][bd][ch].chData.avgV
		= myData->bData[bd].cData[ch].op.meanVolt;
	myData->save_msg[msg].val[idx][bd][ch].chData.avgI
		= myData->bData[bd].cData[ch].op.meanCurr;
	myData->save_msg[msg].write_idx[bd][ch] = idx;
	myData->save_msg[msg].count[bd][ch]++;

	if(myData->bData[bd].cData[ch].ChAttribute.opType == P1){
		myData->save_msg[msg].val[idx][bd][ch].chData.IntegralAmpareHour
			= myData->bData[bd].cData[ch].op.integral_ampareHour
			+ myData->bData[bd].cData[slave_ch].op.integral_ampareHour;
		myData->save_msg[msg].val[idx][bd][ch].chData.IntegralWattHour
			= myData->bData[bd].cData[ch].op.integral_WattHour
			+ myData->bData[bd].cData[slave_ch].op.integral_WattHour;
		myData->save_msg[msg].val[idx][bd][ch].chData.ChargeAmpareHour
			= myData->bData[bd].cData[ch].op.charge_ampareHour
			+ myData->bData[bd].cData[slave_ch].op.charge_ampareHour;
		myData->save_msg[msg].val[idx][bd][ch].chData.ChargeWattHour
			= myData->bData[bd].cData[ch].op.charge_wattHour
			+ myData->bData[bd].cData[slave_ch].op.charge_wattHour;
		myData->save_msg[msg].val[idx][bd][ch].chData.DischargeAmpareHour
			= myData->bData[bd].cData[ch].op.discharge_ampareHour
			+ myData->bData[bd].cData[slave_ch].op.discharge_ampareHour;
		myData->save_msg[msg].val[idx][bd][ch].chData.DischargeWattHour
			= myData->bData[bd].cData[ch].op.discharge_wattHour
			+ myData->bData[bd].cData[slave_ch].op.discharge_wattHour;

		myData->save_msg[msg].val[idx][bd][ch].chData.charge_cc_ampare_hour
			= myData->bData[bd].cData[ch].misc.chargeCCAh
			+ myData->bData[bd].cData[slave_ch].misc.chargeCCAh;
		myData->save_msg[msg].val[idx][bd][ch].chData.charge_cv_ampare_hour
			= myData->bData[bd].cData[ch].misc.chargeCVAh
			+ myData->bData[bd].cData[slave_ch].misc.chargeCVAh;
		myData->save_msg[msg].val[idx][bd][ch].chData.discharge_cc_ampare_hour
			= myData->bData[bd].cData[ch].misc.dischargeCCAh
			+ myData->bData[bd].cData[slave_ch].misc.dischargeCCAh;
		myData->save_msg[msg].val[idx][bd][ch].chData.discharge_cv_ampare_hour
			= myData->bData[bd].cData[ch].misc.dischargeCVAh
			+ myData->bData[bd].cData[slave_ch].misc.dischargeCVAh;

		if(stepType == STEP_CHARGE){
			myData->save_msg[msg].val[idx][bd][ch].chData.DischargeWatt = 0;
			if(dataType == 0){
				myData->save_msg[msg].val[idx][bd][ch].chData.ChargeWatt
					= myData->bData[bd].cData[ch].op.watt
					+ myData->bData[bd].cData[slave_ch].op.watt;
			}else{
				myData->save_msg[msg].val[idx][bd][ch].chData.ChargeWatt
					= myData->bData[bd].cData[ch].misc.tmpWatt
					+ myData->bData[bd].cData[slave_ch].misc.tmpWatt;
			}
		}else if(stepType == STEP_DISCHARGE){
			myData->save_msg[msg].val[idx][bd][ch].chData.ChargeWatt = 0;
			if(dataType == 0){
				myData->save_msg[msg].val[idx][bd][ch].chData.DischargeWatt
					= myData->bData[bd].cData[ch].op.watt
					+ myData->bData[bd].cData[slave_ch].op.watt;
			}else{
				myData->save_msg[msg].val[idx][bd][ch].chData.DischargeWatt
					= myData->bData[bd].cData[ch].misc.tmpWatt
					+ myData->bData[bd].cData[slave_ch].misc.tmpWatt;
			}
		}else{
			myData->save_msg[msg].val[idx][bd][ch].chData.ChargeWatt
				= 0;
			myData->save_msg[msg].val[idx][bd][ch].chData.DischargeWatt
				= 0;
		}
	}else{
		myData->save_msg[msg].val[idx][bd][ch].chData.IntegralAmpareHour
			= myData->bData[bd].cData[ch].op.integral_ampareHour;
		myData->save_msg[msg].val[idx][bd][ch].chData.IntegralWattHour
			= myData->bData[bd].cData[ch].op.integral_WattHour;
		myData->save_msg[msg].val[idx][bd][ch].chData.ChargeAmpareHour
			= myData->bData[bd].cData[ch].op.charge_ampareHour;
		myData->save_msg[msg].val[idx][bd][ch].chData.ChargeWattHour
			= myData->bData[bd].cData[ch].op.charge_wattHour;
		myData->save_msg[msg].val[idx][bd][ch].chData.DischargeAmpareHour
			= myData->bData[bd].cData[ch].op.discharge_ampareHour;
		myData->save_msg[msg].val[idx][bd][ch].chData.DischargeWattHour
			= myData->bData[bd].cData[ch].op.discharge_wattHour;
			

		myData->save_msg[msg].val[idx][bd][ch].chData.charge_cc_ampare_hour
			= myData->bData[bd].cData[ch].misc.chargeCCAh;
		myData->save_msg[msg].val[idx][bd][ch].chData.charge_cv_ampare_hour
			= myData->bData[bd].cData[ch].misc.chargeCVAh;
		myData->save_msg[msg].val[idx][bd][ch].chData.discharge_cc_ampare_hour
			= myData->bData[bd].cData[ch].misc.dischargeCCAh;
		myData->save_msg[msg].val[idx][bd][ch].chData.discharge_cv_ampare_hour
			= myData->bData[bd].cData[ch].misc.dischargeCVAh;

		if(stepType == STEP_CHARGE){
			myData->save_msg[msg].val[idx][bd][ch].chData.DischargeWatt = 0;
			if(dataType == 0){
				myData->save_msg[msg].val[idx][bd][ch].chData.ChargeWatt
					= myData->bData[bd].cData[ch].op.watt;
			}else{
				myData->save_msg[msg].val[idx][bd][ch].chData.ChargeWatt
					= myData->bData[bd].cData[ch].misc.tmpWatt;
			}
		}else if(stepType == STEP_DISCHARGE){
			myData->save_msg[msg].val[idx][bd][ch].chData.ChargeWatt = 0;
			if(dataType == 0){
				myData->save_msg[msg].val[idx][bd][ch].chData.DischargeWatt
					= myData->bData[bd].cData[ch].op.watt;
			}else{
				myData->save_msg[msg].val[idx][bd][ch].chData.DischargeWatt
					= myData->bData[bd].cData[ch].misc.tmpWatt;
			}
		}else{
			myData->save_msg[msg].val[idx][bd][ch].chData.ChargeWatt
				= 0;
			myData->save_msg[msg].val[idx][bd][ch].chData.DischargeWatt
				= 0;
		}
	}
	
	myData->save_msg[msg].val[idx][bd][ch].chData.startVoltage
		= myData->bData[bd].cData[ch].misc.startV;
	myData->save_msg[msg].val[idx][bd][ch].chData.step_count
		= myData->bData[bd].cData[ch].misc.step_count;

	if(myData->bData[bd].cData[ch].op.type != STEP_END) {
		if(myData->save_msg[msg].val[idx][bd][ch].chData.Vsens
				> myData->bData[bd].cData[ch].misc.maxV) {
			myData->bData[bd].cData[ch].misc.maxV = 
				myData->save_msg[msg].val[idx][bd][ch].chData.Vsens;
		}
		if(myData->save_msg[msg].val[idx][bd][ch].chData.Vsens
				< myData->bData[bd].cData[ch].misc.minV) {
			myData->bData[bd].cData[ch].misc.minV = 
				myData->save_msg[msg].val[idx][bd][ch].chData.Vsens;
		}
		if(myData->save_msg[msg].val[idx][bd][ch].chData.Vsens
				> myData->bData[bd].cData[ch].misc.cycleMaxV) {
			myData->bData[bd].cData[ch].misc.cycleMaxV = 
				myData->save_msg[msg].val[idx][bd][ch].chData.Vsens;
		}
		if(myData->save_msg[msg].val[idx][bd][ch].chData.Vsens
				< myData->bData[bd].cData[ch].misc.cycleMinV) {
			myData->bData[bd].cData[ch].misc.cycleMinV = 
				myData->save_msg[msg].val[idx][bd][ch].chData.Vsens;
		}
		if(myData->bData[bd].cData[ch].op.temp
				< myData->bData[bd].cData[ch].misc.minT) {
			myData->bData[bd].cData[ch].misc.minT = 
				myData->bData[bd].cData[ch].op.temp;
		}
		myData->save_msg[msg].val[idx][bd][ch].chData.temp[0][1]
			= myData->bData[bd].cData[ch].misc.minT;
		if(myData->bData[bd].cData[ch].op.temp
				> myData->bData[bd].cData[ch].misc.maxT) {
			myData->bData[bd].cData[ch].misc.maxT = 
				myData->bData[bd].cData[ch].op.temp;
		}
		myData->save_msg[msg].val[idx][bd][ch].chData.temp[0][2]
			= myData->bData[bd].cData[ch].misc.maxT;
	}	
	myData->save_msg[msg].val[idx][bd][ch].chData.maxVoltage
			= myData->bData[bd].cData[ch].misc.maxV;
	myData->save_msg[msg].val[idx][bd][ch].chData.minVoltage
			= myData->bData[bd].cData[ch].misc.minV;

	if(myData->bData[bd].cData[ch].op.select == SAVE_FLAG_SAVING_END
		&& myData->bData[bd].cData[ch].misc.mes_data_flag == 1){
		if(stepType == STEP_LOOP){
			myData->save_msg[msg].val[idx][bd][ch].chData.ChargeWatt
				= myData->bData[bd].cData[ch].misc.cycleSumChargeWatt;
			myData->save_msg[msg].val[idx][bd][ch].chData.DischargeWatt
				= myData->bData[bd].cData[ch].misc.cycleSumDischargeWatt;	
			myData->save_msg[msg].val[idx][bd][ch].chData.ChargeAmpareHour
				= myData->bData[bd].cData[ch].misc.cycleSumChargeAmpareHour;
			myData->save_msg[msg].val[idx][bd][ch].chData.DischargeAmpareHour
				= myData->bData[bd].cData[ch].misc.cycleSumDischargeAmpareHour;
			myData->save_msg[msg].val[idx][bd][ch].chData.ChargeWattHour
				= myData->bData[bd].cData[ch].misc.cycleSumChargeWattHour;
			myData->save_msg[msg].val[idx][bd][ch].chData.DischargeWattHour
				= myData->bData[bd].cData[ch].misc.cycleSumDischargeWattHour;
//	충전 CC용량	
			myData->save_msg[msg].val[idx][bd][ch].chData.charge_cc_ampare_hour
				= myData->bData[bd].cData[ch].misc.cycleSumChargeCCAh;
//	충전 CV용량	
			myData->save_msg[msg].val[idx][bd][ch].chData.charge_cv_ampare_hour
				= myData->bData[bd].cData[ch].misc.cycleSumChargeCVAh;
//	방전 CC용량
			myData->save_msg[msg].val[idx][bd][ch].chData.discharge_cc_ampare_hour
				= myData->bData[bd].cData[ch].misc.cycleSumDischargeCCAh;
//	방전 CV용량
			myData->save_msg[msg].val[idx][bd][ch].chData.discharge_cv_ampare_hour
				= myData->bData[bd].cData[ch].misc.cycleSumDischargeCVAh;
	
			if(myData->bData[bd].cData[ch].misc.cycleRunTime
					>= ONE_DAY_RUNTIME){
				myData->save_msg[msg].val[idx][bd][ch].chData.runTime_day
					= myData->bData[bd].cData[ch].misc.cycleRunTime
						/ ONE_DAY_RUNTIME;
				myData->save_msg[msg].val[idx][bd][ch].chData.runTime
					= myData->bData[bd].cData[ch].misc.cycleRunTime
						% ONE_DAY_RUNTIME;
			}else{
				myData->save_msg[msg].val[idx][bd][ch].chData.runTime_day
					= 0;
				myData->save_msg[msg].val[idx][bd][ch].chData.runTime
					= myData->bData[bd].cData[ch].misc.cycleRunTime;
			}

			if(myData->bData[bd].cData[ch].misc.cycleStepCount != 0){
				myData->save_msg[msg].val[idx][bd][ch].chData.temp[0][3]
					= myData->bData[bd].cData[ch].misc.cycleSumAvgT
					/ myData->bData[bd].cData[ch].misc.cycleStepCount;
			}
			if(myData->bData[bd].cData[ch].misc.cycleDischargeStepCount != 0){
				myData->save_msg[msg].val[idx][bd][ch].chData.avgV
					= myData->bData[bd].cData[ch].misc.cycleAvgDischargeV
					/ myData->bData[bd].cData[ch].misc.cycleDischargeStepCount;
				myData->save_msg[msg].val[idx][bd][ch].chData.avgI
					= myData->bData[bd].cData[ch].misc.cycleAvgDischargeI
					/ myData->bData[bd].cData[ch].misc.cycleDischargeStepCount;
			}

			myData->save_msg[msg].val[idx][bd][ch].chData.startVoltage
				= myData->bData[bd].cData[ch].misc.cycleStartV;

			myData->save_msg[msg].val[idx][bd][ch].chData.maxVoltage
				= myData->bData[bd].cData[ch].misc.cycleMaxV;
			myData->save_msg[msg].val[idx][bd][ch].chData.minVoltage
				= myData->bData[bd].cData[ch].misc.cycleMinV;

// 사이클 충전 CC 시간
			if(myData->bData[bd].cData[ch].misc.cycle_Charge_ccTime
					>= ONE_DAY_RUNTIME){
				myData->save_msg[msg].val[idx][bd][ch].chData.ccTime_day
					= myData->bData[bd].cData[ch].misc.cycle_Charge_ccTime
						/ ONE_DAY_RUNTIME;	
				myData->save_msg[msg].val[idx][bd][ch].chData.ccTime
					= myData->bData[bd].cData[ch].misc.cycle_Charge_ccTime
						% ONE_DAY_RUNTIME;	
			}else{
				myData->save_msg[msg].val[idx][bd][ch].chData.ccTime_day
					= 0;	
				myData->save_msg[msg].val[idx][bd][ch].chData.ccTime
					= myData->bData[bd].cData[ch].misc.cycle_Charge_ccTime;	
			}
// 사이클 충전 CV 시간
			if(myData->bData[bd].cData[ch].misc.cycle_Charge_cvTime
					>= ONE_DAY_RUNTIME){
				myData->save_msg[msg].val[idx][bd][ch].chData.cvTime_day
					= myData->bData[bd].cData[ch].misc.cycle_Charge_cvTime
						/ ONE_DAY_RUNTIME;	
				myData->save_msg[msg].val[idx][bd][ch].chData.cvTime
					= myData->bData[bd].cData[ch].misc.cycle_Charge_cvTime
						% ONE_DAY_RUNTIME;	
			}else{
				myData->save_msg[msg].val[idx][bd][ch].chData.cvTime_day
					= 0;	
				myData->save_msg[msg].val[idx][bd][ch].chData.cvTime
					= myData->bData[bd].cData[ch].misc.cycle_Charge_cvTime;	
			}
		}else{
			if(stepType == STEP_CHARGE){
				if(myData->bData[bd].cData[ch].ChAttribute.opType == P1){
					myData->bData[bd].cData[ch].misc.cycleSumChargeWatt
						+= (myData->bData[bd].cData[ch].op.watt
						+ myData->bData[bd].cData[slave_ch].op.watt);
					myData->bData[bd].cData[ch].misc.cycleSumChargeAmpareHour
						+= (myData->bData[bd].cData[ch].op.charge_ampareHour
						+ myData->bData[bd].cData[slave_ch].op.charge_ampareHour);					
					myData->bData[bd].cData[ch].misc.cycleSumChargeWattHour
						+= (myData->bData[bd].cData[ch].op.charge_wattHour
						+ myData->bData[bd].cData[slave_ch].op.charge_wattHour);
					myData->bData[bd].cData[ch].misc.cycleSumChargeCCAh
						+= myData->bData[bd].cData[ch].misc.chargeCCAh
						+ myData->bData[bd].cData[slave_ch].misc.chargeCCAh;
					myData->bData[bd].cData[ch].misc.cycleSumChargeCVAh
						+= myData->bData[bd].cData[ch].misc.chargeCVAh
						+ myData->bData[bd].cData[slave_ch].misc.chargeCVAh;
				}else{
					myData->bData[bd].cData[ch].misc.cycleSumChargeWatt
						+= myData->bData[bd].cData[ch].op.watt;
					myData->bData[bd].cData[ch].misc.cycleSumChargeAmpareHour
						+= myData->bData[bd].cData[ch].op.charge_ampareHour;
					myData->bData[bd].cData[ch].misc.cycleSumChargeWattHour
						+= myData->bData[bd].cData[ch].op.charge_wattHour;
					myData->bData[bd].cData[ch].misc.cycleSumChargeCCAh
						+= myData->bData[bd].cData[ch].misc.chargeCCAh;
					myData->bData[bd].cData[ch].misc.cycleSumChargeCVAh
						+= myData->bData[bd].cData[ch].misc.chargeCVAh;
				}
			}else if(stepType == STEP_DISCHARGE){
				if(myData->bData[bd].cData[ch].ChAttribute.opType == P1){
					myData->bData[bd].cData[ch].misc.cycleSumDischargeWatt
						+= (myData->bData[bd].cData[ch].op.watt
						+ myData->bData[bd].cData[slave_ch].op.watt);

					myData->bData[bd].cData[ch].misc.cycleSumDischargeAmpareHour
					+= (myData->bData[bd].cData[ch].op.discharge_ampareHour
					+ myData->bData[bd].cData[slave_ch].op.discharge_ampareHour);
					myData->bData[bd].cData[ch].misc.cycleSumDischargeWattHour
					+= (myData->bData[bd].cData[ch].op.discharge_wattHour
					+ myData->bData[bd].cData[slave_ch].op.discharge_wattHour);
					myData->bData[bd].cData[ch].misc.cycleSumDischargeCCAh
						+= myData->bData[bd].cData[ch].misc.dischargeCCAh
						+ myData->bData[bd].cData[slave_ch].misc.dischargeCCAh;
					myData->bData[bd].cData[ch].misc.cycleSumDischargeCVAh
						+= myData->bData[bd].cData[ch].misc.dischargeCVAh
						+ myData->bData[bd].cData[slave_ch].misc.dischargeCVAh;
				}else{
					myData->bData[bd].cData[ch].misc.cycleSumDischargeWatt
						+= myData->bData[bd].cData[ch].op.watt;
					myData->bData[bd].cData[ch].misc.cycleSumDischargeAmpareHour
						+= myData->bData[bd].cData[ch].op.discharge_ampareHour;
					myData->bData[bd].cData[ch].misc.cycleSumDischargeWattHour
						+= myData->bData[bd].cData[ch].op.discharge_wattHour;
					myData->bData[bd].cData[ch].misc.cycleSumDischargeCCAh
						+= myData->bData[bd].cData[ch].misc.dischargeCCAh;
					myData->bData[bd].cData[ch].misc.cycleSumDischargeCVAh
						+= myData->bData[bd].cData[ch].misc.dischargeCVAh;
				}
				myData->bData[bd].cData[ch].misc.cycleAvgDischargeV
					+= myData->bData[bd].cData[ch].op.meanVolt;
				myData->bData[bd].cData[ch].misc.cycleAvgDischargeI
					+= myData->bData[bd].cData[ch].op.meanCurr;	
			}
			myData->bData[bd].cData[ch].misc.cycleSumAvgT
				+= myData->bData[bd].cData[ch].op.meanTemp;
			
			myData->bData[bd].cData[ch].misc.mes_data_flag = 0;
		}
	}
	#ifdef _CH_CHAMBER_DATA 	//211025 hun
	myData->save_msg[msg].val[idx][bd][ch].chData.Chamber_Temp =
			myData->bData[bd].cData[ch].misc.groupTemp;
	#endif
	#ifdef _CH_SWELLING_DATA 		//211025 hun
	for(i = 0; i < MAX_CH_PRESSURE_DATA; i++){
		myData->save_msg[msg].val[idx][bd][ch].chData.PressureData[i] =
				myData->bData[bd].cData[ch].misc.chPressure[i];
	}
	for(i = 0; i < MAX_CH_THICKNESS_DATA; i++){
		myData->save_msg[msg].val[idx][bd][ch].chData.ThicknessData[i] =
				myData->bData[bd].cData[ch].misc.chThickness[i];
	}
	#endif

	#ifdef _ACIR //220124 hun
	myData->save_msg[msg].val[idx][bd][ch].chData.acir_voltage
	 = myData->bData[bd].cData[ch].misc.acir_voltage;
	myData->save_msg[msg].val[idx][bd][ch].chData.acir
	 = myData->bData[bd].cData[ch].misc.acir;
	#endif

	// SDI Data Restore hun_201010
	#ifdef _AC_FAIL_RECOVERY
	myData->save_msg[msg].val[idx][bd][ch].chData.advCycle
		= myData->bData[bd].cData[ch].misc.advCycle;
	myData->save_msg[msg].val[idx][bd][ch].chData.advCycleStep
		= myData->bData[bd].cData[ch].misc.advCycleStep;
	myData->save_msg[msg].val[idx][bd][ch].chData.cycleRunTime
		= myData->bData[bd].cData[ch].misc.cycleRunTime;
	myData->save_msg[msg].val[idx][bd][ch].chData.seedintegralCapacity
		= myData->bData[bd].cData[ch].misc.seedintegralCapacity;
	myData->save_msg[msg].val[idx][bd][ch].chData.sumintegralCapacity
		= myData->bData[bd].cData[ch].misc.sumintegralCapacity;
	myData->save_msg[msg].val[idx][bd][ch].chData.seedChargeAmpareHour
		= myData->bData[bd].cData[ch].misc.seedChargeAmpareHour;
	myData->save_msg[msg].val[idx][bd][ch].chData.sumChargeAmpareHour
		= myData->bData[bd].cData[ch].misc.sumChargeAmpareHour;
	myData->save_msg[msg].val[idx][bd][ch].chData.seedDischargeAmpareHour
		= myData->bData[bd].cData[ch].misc.seedDischargeAmpareHour;
	myData->save_msg[msg].val[idx][bd][ch].chData.sumDischargeAmpareHour
		= myData->bData[bd].cData[ch].misc.sumDischargeAmpareHour;
	myData->save_msg[msg].val[idx][bd][ch].chData.seedintegralWattHour
		= myData->bData[bd].cData[ch].misc.seedintegralWattHour;
	myData->save_msg[msg].val[idx][bd][ch].chData.sumintegralWattHour
		= myData->bData[bd].cData[ch].misc.sumintegralWattHour;
	myData->save_msg[msg].val[idx][bd][ch].chData.seedChargeWattHour
		= myData->bData[bd].cData[ch].misc.seedChargeWattHour;
	myData->save_msg[msg].val[idx][bd][ch].chData.sumChargeWattHour
		= myData->bData[bd].cData[ch].misc.sumChargeWattHour;
	myData->save_msg[msg].val[idx][bd][ch].chData.seedDischargeWattHour
		= myData->bData[bd].cData[ch].misc.seedDischargeWattHour;
	myData->save_msg[msg].val[idx][bd][ch].chData.sumDischargeWattHour
		= myData->bData[bd].cData[ch].misc.sumDischargeWattHour;
	myData->save_msg[msg].val[idx][bd][ch].chData.standardC
		= myData->bData[bd].cData[ch].misc.standardC;
	myData->save_msg[msg].val[idx][bd][ch].chData.standardP
		= myData->bData[bd].cData[ch].misc.standardP;			
	myData->save_msg[msg].val[idx][bd][ch].chData.standardZ
		= myData->bData[bd].cData[ch].misc.standardZ;			
	myData->save_msg[msg].val[idx][bd][ch].chData.cycleSumC
		= myData->bData[bd].cData[ch].misc.cycleSumC;
	myData->save_msg[msg].val[idx][bd][ch].chData.cycleSumP
		= myData->bData[bd].cData[ch].misc.cycleSumP;						
	myData->save_msg[msg].val[idx][bd][ch].chData.cycleEndC
		= myData->bData[bd].cData[ch].misc.cycleEndC;						
	myData->save_msg[msg].val[idx][bd][ch].chData.pattern_change_flag
		= myData->bData[bd].cData[ch].misc.pattern_change_flag;			
	myData->save_msg[msg].val[idx][bd][ch].chData.chGroupNo
		= myData->bData[bd].cData[ch].misc.chGroupNo;		
	myData->save_msg[msg].val[idx][bd][ch].chData.tempDir
		= myData->bData[bd].cData[ch].misc.tempDir;		
			
	myData->save_msg[msg].val[idx][bd][ch].chData.seedChargeCCCVAh
		= myData->bData[bd].cData[ch].misc.seedChargeCCCVAh;
	myData->save_msg[msg].val[idx][bd][ch].chData.sumChargeCCCVAh
		= myData->bData[bd].cData[ch].misc.sumChargeCCCVAh;
	myData->save_msg[msg].val[idx][bd][ch].chData.seedChargeCCAh
		= myData->bData[bd].cData[ch].misc.seedChargeCCAh;
	myData->save_msg[msg].val[idx][bd][ch].chData.sumChargeCCAh
		= myData->bData[bd].cData[ch].misc.sumChargeCCAh;
	myData->save_msg[msg].val[idx][bd][ch].chData.seedDischargeCCCVAh
		= myData->bData[bd].cData[ch].misc.seedDischargeCCCVAh;
	myData->save_msg[msg].val[idx][bd][ch].chData.sumDischargeCCCVAh
		= myData->bData[bd].cData[ch].misc.sumDischargeCCCVAh;
	myData->save_msg[msg].val[idx][bd][ch].chData.seedDischargeCCAh
		= myData->bData[bd].cData[ch].misc.seedDischargeCCAh;
	myData->save_msg[msg].val[idx][bd][ch].chData.sumDischargeCCAh
		= myData->bData[bd].cData[ch].misc.sumDischargeCCAh;
	myData->save_msg[msg].val[idx][bd][ch].chData.seedDischargeCVAh
		= myData->bData[bd].cData[ch].misc.seedDischargeCVAh;
	myData->save_msg[msg].val[idx][bd][ch].chData.sumDischargeCVAh
		= myData->bData[bd].cData[ch].misc.sumDischargeCVAh;		
	myData->save_msg[msg].val[idx][bd][ch].chData.seedChargeCVAh
		= myData->bData[bd].cData[ch].misc.seedChargeCVAh;
	myData->save_msg[msg].val[idx][bd][ch].chData.sumChargeCVAh
		= myData->bData[bd].cData[ch].misc.sumChargeCVAh;
	myData->save_msg[msg].val[idx][bd][ch].chData.chargeCCAh
		= myData->bData[bd].cData[ch].misc.chargeCCAh;
	myData->save_msg[msg].val[idx][bd][ch].chData.chargeCVAh
		= myData->bData[bd].cData[ch].misc.chargeCVAh;		
	myData->save_msg[msg].val[idx][bd][ch].chData.chargeCCCVAh
		= myData->bData[bd].cData[ch].misc.chargeCCCVAh;		
	myData->save_msg[msg].val[idx][bd][ch].chData.dischargeCCAh
		= myData->bData[bd].cData[ch].misc.dischargeCCAh;
	myData->save_msg[msg].val[idx][bd][ch].chData.dischargeCVAh
		= myData->bData[bd].cData[ch].misc.dischargeCVAh;		
	myData->save_msg[msg].val[idx][bd][ch].chData.dischargeCCCVAh
		= myData->bData[bd].cData[ch].misc.dischargeCCCVAh;							
	myData->save_msg[msg].val[idx][bd][ch].chData.cycleSumChargeWatt
		= myData->bData[bd].cData[ch].misc.cycleSumChargeWatt;
	myData->save_msg[msg].val[idx][bd][ch].chData.cycleSumChargeWattHour
		= myData->bData[bd].cData[ch].misc.cycleSumChargeWattHour;
	myData->save_msg[msg].val[idx][bd][ch].chData.cycleSumChargeAmpareHour
		= myData->bData[bd].cData[ch].misc.cycleSumChargeAmpareHour;
	myData->save_msg[msg].val[idx][bd][ch].chData.cycleStepCount
		= myData->bData[bd].cData[ch].misc.cycleStepCount;
	myData->save_msg[msg].val[idx][bd][ch].chData.cycleSumAvgT
		= myData->bData[bd].cData[ch].misc.cycleSumAvgT;
	myData->save_msg[msg].val[idx][bd][ch].chData.sel_Cyc_C_Cap
		= myData->bData[bd].cData[ch].misc.sel_Cyc_C_Cap[advStepNo];
	myData->save_msg[msg].val[idx][bd][ch].chData.sel_Cyc_D_Cap
		= myData->bData[bd].cData[ch].misc.sel_Cyc_D_Cap[advStepNo];
	#endif
	
	// 131228 oys w : real time add
	#if REAL_TIME == 1
	myData->save_msg[msg].val[idx][bd][ch].chData.realDate
		= myData->mData.real_time[6] * 10000	//year
		+ myData->mData.real_time[5] * 100		//month
		+ myData->mData.real_time[4];			//day
	myData->save_msg[msg].val[idx][bd][ch].chData.realClock
		= myData->mData.real_time[3] * 10000000	//hour
		+ myData->mData.real_time[2] * 100000	//min
		+ myData->mData.real_time[1] * 1000		//sec
		+ myData->mData.real_time[0];			//msec
	#endif
}
#endif

void send_pulse_msg(int bd, int ch, int type)
{
	if(myData->bData[bd].cData[ch].ChAttribute.chNo_master == P0) {
		send_pulse_msg_2(bd, ch-1, type, 0); //MainClient
	//	send_pulse_msg_2(bd, ch, type, 1); //DataSave
	} else if(myData->bData[bd].cData[ch].ChAttribute.opType == P0) {
		send_pulse_msg_2(bd, ch, type, 0); //MainClient
	//	send_pulse_msg_2(bd, ch, type, 1); //DataSave
	}
}

void send_pulse_msg_2(int bd, int ch, int type, int msg)
{	
	unsigned char slave_ch = 0;
	int idx;
	
	if(myData->pulse_msg[msg][bd][ch].flag == 1) { //send pulse_msg stop
		return;
	}
	if(myData->bData[bd].cData[ch].ChAttribute.opType == P1)
	{
		slave_ch = myData->bData[bd].cData[ch]
			.ChAttribute.chNo_slave[0] - 1;		//1base
	}

	idx = myData->pulse_msg[msg][bd][ch].write_idx;
	idx++;
	if(idx >= MAX_PULSE_MSG) idx = 0;
	myData->pulse_msg[msg][bd][ch].val[idx].type = type;
	myData->pulse_msg[msg][bd][ch].val[idx].runTime
		= (long)myData->bData[bd].cData[ch].op.runTime;
	myData->pulse_msg[msg][bd][ch].val[idx].totalCycle
		= (long)myData->bData[bd].cData[ch].misc.totalCycle;
	myData->pulse_msg[msg][bd][ch].val[idx].stepNo
		= (long)myData->bData[bd].cData[ch].op.stepNo;

	if(myData->bData[bd].cData[ch].ChAttribute.opType == P1)
	{
		myData->pulse_msg[msg][bd][ch].val[idx].Vsens
			= myData->bData[bd].cData[slave_ch].misc.tmpVsens;
		myData->pulse_msg[msg][bd][ch].val[idx].Isens
			= myData->bData[bd].cData[ch].misc.tmpIsens
				+ myData->bData[bd].cData[slave_ch].misc.tmpIsens;
		myData->pulse_msg[msg][bd][ch].val[idx].wattHour
			= myData->bData[bd].cData[ch].op.wattHour
				+ myData->bData[bd].cData[slave_ch].op.wattHour;
		myData->pulse_msg[msg][bd][ch].val[idx].capacity
			= myData->bData[bd].cData[ch].op.ampareHour
				+ myData->bData[bd].cData[slave_ch].op.ampareHour;
	}else{
		myData->pulse_msg[msg][bd][ch].val[idx].Vsens
			= myData->bData[bd].cData[ch].misc.tmpVsens;
		myData->pulse_msg[msg][bd][ch].val[idx].Isens
			= myData->bData[bd].cData[ch].misc.tmpIsens;
		myData->pulse_msg[msg][bd][ch].val[idx].wattHour
			= myData->bData[bd].cData[ch].op.wattHour;
		myData->pulse_msg[msg][bd][ch].val[idx].capacity
			= myData->bData[bd].cData[ch].op.ampareHour;
	}

	myData->pulse_msg[msg][bd][ch].write_idx = idx;

	myData->pulse_msg[msg][bd][ch].count++;
}

#ifdef __LG_VER1__
void send_10ms_msg_2(int bd, int ch, unsigned long saveDt, int type, int msg)
{
	unsigned char stepType, slave_ch = 0;
	unsigned short advStepNo;
	int idx, dataType, i;
	
	stepType = myData->bData[bd].cData[ch].op.type;
	advStepNo = myData->bData[bd].cData[ch].misc.advStepNo;

	if(myData->bData[bd].cData[ch].misc.save10msDataCount == 0){
		for(i=0; i<MAX_10MS_MSG; i++){
			memset((char *)&myData->save10ms_msg[msg][bd][ch].chData[i],
			0x00, sizeof(S_SAVE_MSG_CH_DATA));
		}
		myData->save10ms_msg[msg][bd][ch].write_idx = 0;
	}
	if(myData->bData[bd].cData[ch].op.type == STEP_OCV
		&& myData->bData[bd].cData[ch].op.runTime == 0)
		myData->save10ms_msg[msg][bd][ch].write_idx = 0;

	if(myData->bData[bd].cData[ch].op.type == STEP_USER_PATTERN){
		dataType = 1; //tmpVsens, tmpIsens
	}else if(myData->bData[bd].cData[ch].op.type == STEP_USER_MAP){
		dataType = 1; //tmpVsens, tmpIsens
	}else if(myData->bData[bd].cData[ch].op.mode == CP){
		dataType = 1; //tmpVsens, tmpIsens
	}else if(myData->bData[bd].cData[ch].op.runTime < 100){
#if CYCLER_TYPE == DIGITAL_CYC
		dataType = 0;
#endif
#if CYCLER_TYPE == LINEAR_CYC || CYCLER_TYPE == CAN_CYC
		dataType = 1; //tmpVsens, tmpIsens
#endif
	}else{
		dataType = 0;
	}
	
	if(myPs->config.hwSpec == L_5V_200A_1CH_JIG) {
		dataType = 1; //tmpVsens, tmpIsens
	}

	idx = myData->save10ms_msg[msg][bd][ch].write_idx;

	if(msg == 1){
		if(myData->mData.config.installedTemp > 0
			|| myData->mData.config.installedAuxV > 0){
			send_save_msg_aux(bd, ch, idx, msg); //auxDataSave
		}
	}

	myData->save10ms_msg[msg][bd][ch].chData[idx].resultIndex
		= myData->bData[bd].cData[ch].op.resultIndex;
	myData->save10ms_msg[msg][bd][ch].chData[idx].state
		= myData->bData[bd].cData[ch].op.state;
	myData->save10ms_msg[msg][bd][ch].chData[idx].type
		= myData->bData[bd].cData[ch].op.type;
	myData->save10ms_msg[msg][bd][ch].chData[idx].mode
		= myData->bData[bd].cData[ch].op.mode;
	
	if(myData->bData[bd].cData[ch].ChAttribute.opType == P1)
	{
		slave_ch = myData->bData[bd].cData[ch]
			.ChAttribute.chNo_slave[0] - 1;  //1base
	}
	if(dataType == 0) {
		if(myData->bData[bd].cData[ch].ChAttribute.opType == P1)
		{
			if((myData->mData.config.rt_scan_type == RT_SCAN_PERIOD_100mS)
				&& (saveDt < RTTASK_1000MS)) {
				myData->save10ms_msg[msg][bd][ch].chData[idx].Vsens
					= myData->bData[bd].cData[slave_ch].misc.tmpVsens;
			} else {
				myData->save10ms_msg[msg][bd][ch].chData[idx].Vsens
					= myData->bData[bd].cData[slave_ch].op.Vsens;
			}
			myData->save10ms_msg[msg][bd][ch].chData[idx].Isens
				= myData->bData[bd].cData[ch].op.Isens
					+ myData->bData[bd].cData[slave_ch].op.Isens;
			myData->save10ms_msg[msg][bd][ch].chData[idx].watt
				= myData->bData[bd].cData[ch].op.watt
					+ myData->bData[bd].cData[slave_ch].op.watt;
		} else {
			if((myData->mData.config.rt_scan_type == RT_SCAN_PERIOD_100mS)
				&& (saveDt < RTTASK_1000MS)) {
				myData->save10ms_msg[msg][bd][ch].chData[idx].Vsens
					= myData->bData[bd].cData[ch].misc.tmpVsens;
			} else {
				myData->save10ms_msg[msg][bd][ch].chData[idx].Vsens
					= myData->bData[bd].cData[ch].op.Vsens;
			}
			myData->save10ms_msg[msg][bd][ch].chData[idx].Isens
				= myData->bData[bd].cData[ch].op.Isens;
			myData->save10ms_msg[msg][bd][ch].chData[idx].watt
				= myData->bData[bd].cData[ch].op.watt;
		}
	} else {
		if(myData->bData[bd].cData[ch].ChAttribute.opType == P1)
		{
			myData->save10ms_msg[msg][bd][ch].chData[idx].Vsens
				= myData->bData[bd].cData[slave_ch].misc.tmpVsens;
			myData->save10ms_msg[msg][bd][ch].chData[idx].Isens
				= myData->bData[bd].cData[ch].misc.tmpIsens
					+ myData->bData[bd].cData[slave_ch].misc.tmpIsens;
			myData->save10ms_msg[msg][bd][ch].chData[idx].watt
				= myData->bData[bd].cData[ch].misc.tmpWatt
					+ myData->bData[bd].cData[slave_ch].misc.tmpWatt;

		} else {
			myData->save10ms_msg[msg][bd][ch].chData[idx].Vsens
				= myData->bData[bd].cData[ch].misc.tmpVsens;
			myData->save10ms_msg[msg][bd][ch].chData[idx].Isens
				= myData->bData[bd].cData[ch].misc.tmpIsens;
			myData->save10ms_msg[msg][bd][ch].chData[idx].watt
				= myData->bData[bd].cData[ch].misc.tmpWatt;
		}
	}

	myData->save10ms_msg[msg][bd][ch].chData[idx].select
		= myData->bData[bd].cData[ch].op.select;
	myData->save10ms_msg[msg][bd][ch].chData[idx].code
		= myData->bData[bd].cData[ch].op.code;
	myData->save10ms_msg[msg][bd][ch].chData[idx].stepNo
		= (unsigned char)myData->bData[bd].cData[ch].op.stepNo;

	if(myData->bData[bd].cData[ch].ChAttribute.opType == P1)
	{
		myData->save10ms_msg[msg][bd][ch].chData[idx].wattHour
			= myData->bData[bd].cData[ch].op.wattHour
			+ myData->bData[bd].cData[slave_ch].op.wattHour;
		myData->save10ms_msg[msg][bd][ch].chData[idx].avgI
			= myData->bData[bd].cData[ch].op.meanCurr
			+ myData->bData[bd].cData[slave_ch].op.meanCurr;
		myData->save10ms_msg[msg][bd][ch].chData[idx].z
			= myData->bData[bd].cData[slave_ch].op.z;
		myData->save10ms_msg[msg][bd][ch].chData[idx].capacity
			= myData->bData[bd].cData[ch].op.ampareHour
			+ myData->bData[bd].cData[slave_ch].op.ampareHour;
		//120818 kji SDI mes cycle data
		if(myData->mData.config.function[F_SDI_MES_USE] == P1){
			if(myData->bData[bd].cData[ch].op.type == STEP_LOOP){
				myData->save10ms_msg[msg][bd][ch].chData[idx].Vsens
					= myData->bData[bd].cData[slave_ch].misc.lastRestVsens;
				myData->save10ms_msg[msg][bd][ch].chData[idx].capacity
					= myData->bData[bd].cData[ch].misc.chargeCCAh
					+ myData->bData[bd].cData[slave_ch].misc.chargeCCAh;
				myData->save10ms_msg[msg][bd][ch].chData[idx].z
					= myData->bData[bd].cData[ch].misc.chargeCVAh
					+ myData->bData[bd].cData[slave_ch].misc.chargeCVAh;
			}
		}
	} else {
		myData->save10ms_msg[msg][bd][ch].chData[idx].wattHour
			= myData->bData[bd].cData[ch].op.wattHour;
		myData->save10ms_msg[msg][bd][ch].chData[idx].avgI
			= myData->bData[bd].cData[ch].op.meanCurr;
		myData->save10ms_msg[msg][bd][ch].chData[idx].z
			= myData->bData[bd].cData[ch].op.z;
		myData->save10ms_msg[msg][bd][ch].chData[idx].capacity
			= myData->bData[bd].cData[ch].op.ampareHour;
		//120818 kji SDI mes cycle data
		if(myData->mData.config.function[F_SDI_MES_USE] == P1){
			if(myData->bData[bd].cData[ch].op.type == STEP_LOOP){
				myData->save10ms_msg[msg][bd][ch].chData[idx].Vsens
					= myData->bData[bd].cData[slave_ch].misc.lastRestVsens;
				myData->save10ms_msg[msg][bd][ch].chData[idx].capacity
					= myData->bData[bd].cData[ch].misc.chargeCCAh;
				myData->save10ms_msg[msg][bd][ch].chData[idx].z
					= myData->bData[bd].cData[ch].misc.chargeCVAh;
			}
		}
	}

	myData->save10ms_msg[msg][bd][ch].chData[idx].runTime
		= myData->bData[bd].cData[ch].op.runTime;
	myData->save10ms_msg[msg][bd][ch].chData[idx].totalRunTime
		= myData->bData[bd].cData[ch].op.totalRunTime;
	myData->save10ms_msg[msg][bd][ch].chData[idx].temp
			= myData->bData[bd].cData[ch].op.temp;
	myData->save10ms_msg[msg][bd][ch].chData[idx].reservedCmd
		= myData->bData[bd].cData[ch].op.reservedCmd;

	myData->save10ms_msg[msg][bd][ch].chData[idx].totalCycle
		= myData->bData[bd].cData[ch].misc.totalCycle;
	myData->save10ms_msg[msg][bd][ch].chData[idx].currentCycle
		= myData->bData[bd].cData[ch].misc.currentCycle;
	myData->save10ms_msg[msg][bd][ch].chData[idx].gotoCycleCount
		= myData->bData[bd].cData[ch].misc.gotoCycleCount[advStepNo];
	
	myData->save10ms_msg[msg][bd][ch].chData[idx].avgV
		= myData->bData[bd].cData[ch].op.meanVolt;
	
	myData->save10ms_msg[msg][bd][ch].chData[idx].grade 
		= myData->bData[bd].cData[ch].op.grade;
	
	// 090903 jyk
#if NETWORK_VERSION > 4101	
	#if VENDER != 2 //NOT SDI
	if(myData->bData[bd].cData[ch].ChAttribute.opType == P1)
	{
		myData->save10ms_msg[msg][bd][ch].chData[idx].IntegralAmpareHour = 
			myData->bData[bd].cData[ch].op.integral_ampareHour
			+ myData->bData[bd].cData[slave_ch].op.integral_ampareHour;
		myData->save10ms_msg[msg][bd][ch].chData[idx].IntegralWattHour =
			myData->bData[bd].cData[ch].op.integral_WattHour
			+ myData->bData[bd].cData[slave_ch].op.integral_WattHour;
		myData->save10ms_msg[msg][bd][ch].chData[idx].ChargeAmpareHour =
			myData->bData[bd].cData[ch].op.charge_ampareHour
			+ myData->bData[bd].cData[slave_ch].op.charge_ampareHour;
		myData->save10ms_msg[msg][bd][ch].chData[idx].ChargeWattHour =
			myData->bData[bd].cData[ch].op.charge_wattHour
			+ myData->bData[bd].cData[slave_ch].op.charge_wattHour;
		myData->save10ms_msg[msg][bd][ch].chData[idx].DischargeAmpareHour =
			myData->bData[bd].cData[ch].op.discharge_ampareHour
			+ myData->bData[bd].cData[slave_ch].op.discharge_ampareHour;
		myData->save10ms_msg[msg][bd][ch].chData[idx].DischargeWattHour =
			myData->bData[bd].cData[ch].op.discharge_wattHour
			+ myData->bData[bd].cData[slave_ch].op.discharge_wattHour;
		if(myData->bData[bd].cData[ch].misc.cvTime > 
				myData->bData[bd].cData[slave_ch].misc.cvTime) {
			myData->save10ms_msg[msg][bd][ch].chData[idx].cvTime = 
				myData->bData[bd].cData[ch].misc.cvTime ;
		} else {
			myData->save10ms_msg[msg][bd][ch].chData[idx].cvTime = 
				myData->bData[bd].cData[slave_ch].misc.cvTime ;
		}
	} else {
		myData->save10ms_msg[msg][bd][ch].chData[idx].IntegralAmpareHour
			= myData->bData[bd].cData[ch].op.integral_ampareHour;
		myData->save10ms_msg[msg][bd][ch].chData[idx].IntegralWattHour
			= myData->bData[bd].cData[ch].op.integral_WattHour;
		myData->save10ms_msg[msg][bd][ch].chData[idx].ChargeAmpareHour
			= myData->bData[bd].cData[ch].op.charge_ampareHour;
		myData->save10ms_msg[msg][bd][ch].chData[idx].ChargeWattHour
			= myData->bData[bd].cData[ch].op.charge_wattHour;
		myData->save10ms_msg[msg][bd][ch].chData[idx].DischargeAmpareHour
			= myData->bData[bd].cData[ch].op.discharge_ampareHour;
		myData->save10ms_msg[msg][bd][ch].chData[idx].DischargeWattHour
			= myData->bData[bd].cData[ch].op.discharge_wattHour;
		myData->save10ms_msg[msg][bd][ch].chData[idx].cvTime
			= myData->bData[bd].cData[ch].misc.cvTime;
	}
		#if PROGRAM_VERSION1 == 0
			#if PROGRAM_VERSION2 >= 1
	if(myData->bData[bd].cData[ch].ChAttribute.opType == P1) {
		myData->save10ms_msg[msg][bd][ch].chData[idx].Farad
			= myData->bData[bd].cData[ch].op.capacitance
			+ myData->bData[bd].cData[slave_ch].op.capacitance;
	} else {
		myData->save10ms_msg[msg][bd][ch].chData[idx].Farad
			= myData->bData[bd].cData[ch].op.capacitance;
	}
	myData->save10ms_msg[msg][bd][ch].chData[idx].totalRunTime_carry
		= myData->bData[bd].cData[ch].op.totalRunTime_carry;
	myData->save10ms_msg[msg][bd][ch].chData[idx].cycleNo
		= myData->bData[bd].cData[ch].misc.cycleNo;
//	120109 oys w : One Ch -> Temp Module Multi Ch Input
	myData->save10ms_msg[msg][bd][ch].chData[idx].temp1
		= myData->bData[bd].cData[ch].op.temp1;
			#endif
	/*		
			#if EDLC_TYPE == 1
	//20160229 khk add start
	myData->save10ms_msg[msg].val[idx][bd][ch].chData.c_t1
		= myData->bData[bd].cData[ch].misc.c_t1;	
	myData->save10ms_msg[msg].val[idx][bd][ch].chData.c_v1
		= myData->bData[bd].cData[ch].misc.c_v1;	
	myData->save10ms_msg[msg].val[idx][bd][ch].chData.c_t2
		= myData->bData[bd].cData[ch].misc.c_t2;	
	myData->save10ms_msg[msg].val[idx][bd][ch].chData.c_v2
		= myData->bData[bd].cData[ch].misc.c_v2;	
	//20160229 khk add end
			#endif
	*/
			#if PROGRAM_VERSION2 >= 2
	if(myData->bData[bd].cData[ch].ChAttribute.opType == P1)
	{
		myData->save10ms_msg[msg][bd][ch].chData[idx].chargeCCAh
			= myData->bData[bd].cData[ch].misc.chargeCCAh
			+ myData->bData[bd].cData[slave_ch].misc.chargeCCAh;
		myData->save10ms_msg[msg][bd][ch].chData[idx].chargeCVAh
			= myData->bData[bd].cData[ch].misc.chargeCVAh
			+ myData->bData[bd].cData[slave_ch].misc.chargeCVAh;
		myData->save10ms_msg[msg][bd][ch].chData[idx].dischargeCCAh
			= myData->bData[bd].cData[ch].misc.dischargeCCAh
			+ myData->bData[bd].cData[slave_ch].misc.dischargeCCAh;
		myData->save10ms_msg[msg][bd][ch].chData[idx].dischargeCVAh
			= myData->bData[bd].cData[ch].misc.dischargeCVAh
			+ myData->bData[bd].cData[slave_ch].misc.dischargeCVAh;
	}else{
		myData->save10ms_msg[msg][bd][ch].chData[idx].chargeCCAh
			= myData->bData[bd].cData[ch].misc.chargeCCAh;
		myData->save10ms_msg[msg][bd][ch].chData[idx].chargeCVAh
			= myData->bData[bd].cData[ch].misc.chargeCVAh;
		myData->save10ms_msg[msg][bd][ch].chData[idx].dischargeCCAh
			= myData->bData[bd].cData[ch].misc.dischargeCCAh;
		myData->save10ms_msg[msg][bd][ch].chData[idx].dischargeCVAh
			= myData->bData[bd].cData[ch].misc.dischargeCVAh;
	}
			#endif
		#endif
		#if PROGRAM_VERSION1 > 0
		if(myData->bData[bd].cData[ch].op.type != STEP_END) {
			if(myData->save10ms_msg[msg][bd][ch].chData[idx].Vsens
					> myData->bData[bd].cData[ch].misc.maxV) {
				myData->bData[bd].cData[ch].misc.maxV = 
					myData->save10ms_msg[msg][bd][ch].chData[idx].Vsens;
			}
			if(myData->save10ms_msg[msg][bd][ch].chData[idx].Vsens
					< myData->bData[bd].cData[ch].misc.minV) {
				myData->bData[bd].cData[ch].misc.minV = 
					myData->save10ms_msg[msg][bd][ch].chData[idx].Vsens;
			}
			if(myData->save10ms_msg[msg][bd][ch].chData[idx].temp
					> myData->bData[bd].cData[ch].misc.maxT) {
				myData->bData[bd].cData[ch].misc.maxT = 
					myData->save10ms_msg[msg][bd][ch].chData[idx].temp;
			}
			if(myData->save10ms_msg[msg][bd][ch].chData[idx].temp
					< myData->bData[bd].cData[ch].misc.minT) {
				myData->bData[bd].cData[ch].misc.minT = 
					myData->save10ms_msg[msg][bd][ch].chData[idx].temp;
			}
		}
		if(myData->bData[bd].cData[ch].ChAttribute.opType == P1) {
			myData->save10ms_msg[msg][bd][ch].chData[idx].Farad
				= myData->bData[bd].cData[ch].op.capacitance
				+ myData->bData[bd].cData[slave_ch].op.capacitance;
		} else {
			myData->save10ms_msg[msg][bd][ch].chData[idx].Farad
				= myData->bData[bd].cData[ch].op.capacitance
		}
		myData->save10ms_msg[msg][bd][ch].chData[idx].totalRunTime_carry
			= myData->bData[bd].cData[ch].op.totalRunTime_carry;
		myData->save10ms_msg[msg][bd][ch].chData[idx].cycleNo
			= myData->bData[bd].cData[ch].misc.cycleNo;
//	120109 oys w : One Ch -> Temp Module Multi Ch Input
		myData->save10ms_msg[msg][bd][ch].chData[idx].temp1
			= myData->bData[bd].cData[ch].op.temp1;
		myData->save10ms_msg[msg][bd][ch].chData[idx].startVoltage
			= myData->bData[bd].cData[ch].misc.startV;
		myData->save10ms_msg[msg][bd][ch].chData[idx].maxVoltage
			= myData->bData[bd].cData[ch].misc.maxV;
		myData->save10ms_msg[msg][bd][ch].chData[idx].minVoltage
			= myData->bData[bd].cData[ch].misc.minV;
		myData->save10ms_msg[msg][bd][ch].chData[idx].startTemp
			= myData->bData[bd].cData[ch].misc.startT;
		myData->save10ms_msg[msg][bd][ch].chData[idx].maxTemp
			= myData->bData[bd].cData[ch].misc.maxT;
		myData->save10ms_msg[msg][bd][ch].chData[idx].minTemp
			= myData->bData[bd].cData[ch].misc.minT;	
		#endif	
	#endif
	// 131228 oys w : real time add
	#if REAL_TIME == 1
	myData->save10ms_msg[msg][bd][ch].chData[idx].realDate
		= myData->mData.real_time[6] * 10000	//year
		+ myData->mData.real_time[5] * 100		//month
		+ myData->mData.real_time[4];			//day
	myData->save10ms_msg[msg][bd][ch].chData[idx].realClock
		= myData->mData.real_time[3] * 10000000	//hour
		+ myData->mData.real_time[2] * 100000	//min
		+ myData->mData.real_time[1] * 1000		//sec
		+ myData->mData.real_time[0];			//msec
	#endif
#endif
	if(type == 1){	// mem copy
		for(i=0; i < idx+1; i++){
			memset((char *)&myData->save10ms_msg[msg+2][bd][ch].chData[i],
			0x00, sizeof(S_SAVE_MSG_CH_DATA));
			memcpy((char *)&myData->save10ms_msg[msg+2][bd][ch].chData[i],
				(char *)&myData->save10ms_msg[msg][bd][ch].chData[i],
				sizeof(S_SAVE_MSG_CH_DATA));
			memset((char *)&myData->save10ms_msg[msg][bd][ch].chData[i],
			0x00, sizeof(S_SAVE_MSG_CH_DATA));
		}
		myData->save10ms_msg[msg+2][bd][ch].flag = 1;
		myData->save10ms_msg[msg+2][bd][ch].write_idx = idx;
	}

//	myData->save10ms_msg[msg][bd][ch].flag = type;
	myData->save10ms_msg[msg][bd][ch].write_idx++;
//	myData->save10ms_msg[msg+2][bd][ch].count++;

	if(myData->bData[bd].cData[ch].op.type == STEP_END)
		myData->save10ms_msg[msg][bd][ch].write_idx = 0;
}
#endif

#ifdef __SDI_MES_VER4__
void send_10ms_msg_2(int bd, int ch, unsigned long saveDt, int type, int msg)
{
	unsigned char stepType, slave_ch = 0;
	unsigned short advStepNo;
	int idx, dataType, i;
	
	stepType = myData->bData[bd].cData[ch].op.type;
	advStepNo = myData->bData[bd].cData[ch].misc.advStepNo;

	if(myData->bData[bd].cData[ch].misc.save10msDataCount == 0){
		for(i=0; i<MAX_10MS_MSG; i++){
			memset((char *)&myData->save10ms_msg[msg][bd][ch].chData[i],
			0x00, sizeof(S_SAVE_MSG_CH_DATA));
		}
		myData->save10ms_msg[msg][bd][ch].write_idx = 0;
	}
	if(myData->bData[bd].cData[ch].op.type == STEP_OCV
		&& myData->bData[bd].cData[ch].op.runTime == 0)
		myData->save10ms_msg[msg][bd][ch].write_idx = 0;

	if(myData->bData[bd].cData[ch].op.type == STEP_USER_PATTERN)
		dataType = 1; //tmpVsens, tmpIsens
	else if(myData->bData[bd].cData[ch].op.type == STEP_USER_MAP)
		dataType = 1; //tmpVsens, tmpIsens
	else if(myData->bData[bd].cData[ch].op.mode == CP)
		dataType = 1; //tmpVsens, tmpIsens
	else if(myData->bData[bd].cData[ch].op.runTime < 100)
		dataType = 1; //tmpVsens, tmpIsens
	else
		dataType = 0;

	idx = myData->save10ms_msg[msg][bd][ch].write_idx;

	if(msg == 1){
		if(myData->mData.config.installedTemp > 0
			|| myData->mData.config.installedAuxV > 0){
			send_save_msg_aux(bd, ch, idx, msg); //auxDataSave
		}
	}

	myData->save10ms_msg[msg][bd][ch].chData[idx].resultIndex
		= myData->bData[bd].cData[ch].op.resultIndex;
	myData->save10ms_msg[msg][bd][ch].chData[idx].state
		= myData->bData[bd].cData[ch].op.state;
	myData->save10ms_msg[msg][bd][ch].chData[idx].type
		= myData->bData[bd].cData[ch].op.type;
	myData->save10ms_msg[msg][bd][ch].chData[idx].mode
		= myData->bData[bd].cData[ch].op.mode;
	myData->save10ms_msg[msg][bd][ch].chData[idx].select
		= myData->bData[bd].cData[ch].op.select;
	myData->save10ms_msg[msg][bd][ch].chData[idx].cvFlag 
		= myData->bData[bd].cData[ch].misc.cvFlag;

//	일반형 Cycler에서 안쓰는 항목들.
	myData->save10ms_msg[msg][bd][ch].chData[idx].switchState[0] = 0;
	myData->save10ms_msg[msg][bd][ch].chData[idx].switchState[1] = 0;
	myData->save10ms_msg[msg][bd][ch].chData[idx].chamber_control = 0;
	myData->save10ms_msg[msg][bd][ch].chData[idx].record_index = 0;
	myData->save10ms_msg[msg][bd][ch].chData[idx].input_val	= 0;
	myData->save10ms_msg[msg][bd][ch].chData[idx].output_val = 0;
	myData->save10ms_msg[msg][bd][ch].chData[idx].Vinput = 0;
	myData->save10ms_msg[msg][bd][ch].chData[idx].Vpower = 0;
	myData->save10ms_msg[msg][bd][ch].chData[idx].Vbus	= 0;
//

	myData->save10ms_msg[msg][bd][ch].chData[idx].code
		= (short int)myData->bData[bd].cData[ch].op.code;
	myData->save10ms_msg[msg][bd][ch].chData[idx].stepNo
		= (short int)myData->bData[bd].cData[ch].op.stepNo;

	if(myData->bData[bd].cData[ch].ChAttribute.opType == P1)
	{
		slave_ch = myData->bData[bd].cData[ch]
			.ChAttribute.chNo_slave[0] - 1;  //1base
	}
	if(dataType == 0) {
		if(myData->bData[bd].cData[ch].ChAttribute.opType == P1){
			if((myData->mData.config.rt_scan_type == RT_SCAN_PERIOD_100mS)
				&& (saveDt < RTTASK_1000MS)) {
				myData->save10ms_msg[msg][bd][ch].chData[idx].Vsens
					= myData->bData[bd].cData[slave_ch].misc.tmpVsens;
			} else {
				myData->save10ms_msg[msg][bd][ch].chData[idx].Vsens
					= myData->bData[bd].cData[slave_ch].op.Vsens;
			}
			myData->save10ms_msg[msg][bd][ch].chData[idx].Isens
				= myData->bData[bd].cData[ch].op.Isens
					+ myData->bData[bd].cData[slave_ch].op.Isens;
		}else{
			if((myData->mData.config.rt_scan_type == RT_SCAN_PERIOD_100mS)
				&& (saveDt < RTTASK_1000MS)) {
				myData->save10ms_msg[msg][bd][ch].chData[idx].Vsens
					= myData->bData[bd].cData[ch].misc.tmpVsens;
			} else {
				myData->save10ms_msg[msg][bd][ch].chData[idx].Vsens
					= myData->bData[bd].cData[ch].op.Vsens;
			}
			myData->save10ms_msg[msg][bd][ch].chData[idx].Isens
				= myData->bData[bd].cData[ch].op.Isens;
		}
	}else{
		if(myData->bData[bd].cData[ch].ChAttribute.opType == P1) {
			myData->save10ms_msg[msg][bd][ch].chData[idx].Vsens
				= myData->bData[bd].cData[slave_ch].misc.tmpVsens;
			myData->save10ms_msg[msg][bd][ch].chData[idx].Isens
				= myData->bData[bd].cData[ch].misc.tmpIsens
					+ myData->bData[bd].cData[slave_ch].misc.tmpIsens;
		} else {
			myData->save10ms_msg[msg][bd][ch].chData[idx].Vsens
				= myData->bData[bd].cData[ch].misc.tmpVsens;
			myData->save10ms_msg[msg][bd][ch].chData[idx].Isens
				= myData->bData[bd].cData[ch].misc.tmpIsens;
		}
	}
//	스텝 진행시간
	if(myData->bData[bd].cData[ch].op.runTime >= ONE_DAY_RUNTIME){
		myData->save10ms_msg[msg][bd][ch].chData[idx].runTime_day
			= myData->bData[bd].cData[ch].op.runTime / ONE_DAY_RUNTIME;	
		myData->save10ms_msg[msg][bd][ch].chData[idx].runTime
			= myData->bData[bd].cData[ch].op.runTime % ONE_DAY_RUNTIME;	
	}else{
		myData->save10ms_msg[msg][bd][ch].chData[idx].runTime_day
			= 0;	
		myData->save10ms_msg[msg][bd][ch].chData[idx].runTime
			= myData->bData[bd].cData[ch].op.runTime;	
	}
//	누적 총 진행시간
	if(myData->bData[bd].cData[ch].op.totalRunTime >= ONE_DAY_RUNTIME){
		myData->save10ms_msg[msg][bd][ch].chData[idx].totalRunTime_carry
			= myData->bData[bd].cData[ch].op.totalRunTime / ONE_DAY_RUNTIME;	
		myData->save10ms_msg[msg][bd][ch].chData[idx].totalRunTime
			= myData->bData[bd].cData[ch].op.totalRunTime % ONE_DAY_RUNTIME;
	}else{
		myData->save10ms_msg[msg][bd][ch].chData[idx].totalRunTime_carry
			= 0;	
		myData->save10ms_msg[msg][bd][ch].chData[idx].totalRunTime
			= myData->bData[bd].cData[ch].op.totalRunTime;
	}
//	CC 시간
	if(myData->bData[bd].cData[ch].misc.ccTime >= ONE_DAY_RUNTIME){
		myData->save10ms_msg[msg][bd][ch].chData[idx].ccTime_day
			= myData->bData[bd].cData[ch].misc.ccTime / ONE_DAY_RUNTIME;	
		myData->save10ms_msg[msg][bd][ch].chData[idx].ccTime
			= myData->bData[bd].cData[ch].misc.ccTime % ONE_DAY_RUNTIME;	
	}else{
		myData->save10ms_msg[msg][bd][ch].chData[idx].ccTime_day
			= 0;	
		myData->save10ms_msg[msg][bd][ch].chData[idx].ccTime
			= myData->bData[bd].cData[ch].misc.ccTime;	
	}
//	CV 시간
	if(myData->bData[bd].cData[ch].misc.cvTime >= ONE_DAY_RUNTIME){
		myData->save10ms_msg[msg][bd][ch].chData[idx].cvTime_day
			= myData->bData[bd].cData[ch].misc.cvTime / ONE_DAY_RUNTIME;	
		myData->save10ms_msg[msg][bd][ch].chData[idx].cvTime
			= myData->bData[bd].cData[ch].misc.cvTime % ONE_DAY_RUNTIME;	
	}else{
		myData->save10ms_msg[msg][bd][ch].chData[idx].cvTime_day
			= 0;	
		myData->save10ms_msg[msg][bd][ch].chData[idx].cvTime
			= myData->bData[bd].cData[ch].misc.cvTime;	
	}

	myData->save10ms_msg[msg][bd][ch].chData[idx].z
		= myData->bData[bd].cData[ch].op.z;

	myData->save10ms_msg[msg][bd][ch].chData[idx].temp[0][0]
		= myData->bData[bd].cData[ch].op.temp;
	myData->save10ms_msg[msg][bd][ch].chData[idx].temp[0][3]
		= myData->bData[bd].cData[ch].op.meanTemp;

	myData->save10ms_msg[msg][bd][ch].chData[idx].reservedCmd
		= myData->bData[bd].cData[ch].op.reservedCmd;

	myData->save10ms_msg[msg][bd][ch].chData[idx].gotoCycleCount
		= myData->bData[bd].cData[ch].misc.gotoCycleCount[advStepNo];
	myData->save10ms_msg[msg][bd][ch].chData[idx].totalCycle
		= myData->bData[bd].cData[ch].misc.totalCycle;
	myData->save10ms_msg[msg][bd][ch].chData[idx].currentCycle
		= myData->bData[bd].cData[ch].misc.currentCycle;
	
	myData->save10ms_msg[msg][bd][ch].chData[idx].avgV
		= myData->bData[bd].cData[ch].op.meanVolt;
	myData->save10ms_msg[msg][bd][ch].chData[idx].avgI
		= myData->bData[bd].cData[ch].op.meanCurr;
//	myData->save_msg[msg].write_idx[bd][ch] = idx;
//	myData->save_msg[msg].count[bd][ch]++;

	if(myData->bData[bd].cData[ch].ChAttribute.opType == P1){
		myData->save10ms_msg[msg][bd][ch].chData[idx].IntegralAmpareHour
			= myData->bData[bd].cData[ch].op.integral_ampareHour
			+ myData->bData[bd].cData[slave_ch].op.integral_ampareHour;
		myData->save10ms_msg[msg][bd][ch].chData[idx].IntegralWattHour
			= myData->bData[bd].cData[ch].op.integral_WattHour
			+ myData->bData[bd].cData[slave_ch].op.integral_WattHour;
		myData->save10ms_msg[msg][bd][ch].chData[idx].ChargeAmpareHour
			= myData->bData[bd].cData[ch].op.charge_ampareHour
			+ myData->bData[bd].cData[slave_ch].op.charge_ampareHour;
		myData->save10ms_msg[msg][bd][ch].chData[idx].ChargeWattHour
			= myData->bData[bd].cData[ch].op.charge_wattHour
			+ myData->bData[bd].cData[slave_ch].op.charge_wattHour;
		myData->save10ms_msg[msg][bd][ch].chData[idx].DischargeAmpareHour
			= myData->bData[bd].cData[ch].op.discharge_ampareHour
			+ myData->bData[bd].cData[slave_ch].op.discharge_ampareHour;
		myData->save10ms_msg[msg][bd][ch].chData[idx].DischargeWattHour
			= myData->bData[bd].cData[ch].op.discharge_wattHour
			+ myData->bData[bd].cData[slave_ch].op.discharge_wattHour;

		myData->save10ms_msg[msg][bd][ch].chData[idx].charge_cc_ampare_hour
			= myData->bData[bd].cData[ch].misc.chargeCCAh
			+ myData->bData[bd].cData[slave_ch].misc.chargeCCAh;
		myData->save10ms_msg[msg][bd][ch].chData[idx].charge_cv_ampare_hour
			= myData->bData[bd].cData[ch].misc.chargeCVAh
			+ myData->bData[bd].cData[slave_ch].misc.chargeCVAh;
		myData->save10ms_msg[msg][bd][ch].chData[idx].discharge_cc_ampare_hour
			= myData->bData[bd].cData[ch].misc.dischargeCCAh
			+ myData->bData[bd].cData[slave_ch].misc.dischargeCCAh;
		myData->save10ms_msg[msg][bd][ch].chData[idx].discharge_cv_ampare_hour
			= myData->bData[bd].cData[ch].misc.dischargeCVAh
			+ myData->bData[bd].cData[slave_ch].misc.dischargeCVAh;

		if(stepType == STEP_CHARGE){
			myData->save10ms_msg[msg][bd][ch].chData[idx].DischargeWatt = 0;
			if(dataType == 0){
				myData->save10ms_msg[msg][bd][ch].chData[idx].ChargeWatt
					= myData->bData[bd].cData[ch].op.watt
					+ myData->bData[bd].cData[slave_ch].op.watt;
			}else{
				myData->save10ms_msg[msg][bd][ch].chData[idx].ChargeWatt
					= myData->bData[bd].cData[ch].misc.tmpWatt
					+ myData->bData[bd].cData[slave_ch].misc.tmpWatt;
			}
		}else if(stepType == STEP_DISCHARGE){
			myData->save10ms_msg[msg][bd][ch].chData[idx].ChargeWatt = 0;
			if(dataType == 0){
				myData->save10ms_msg[msg][bd][ch].chData[idx].DischargeWatt
					= myData->bData[bd].cData[ch].op.watt
					+ myData->bData[bd].cData[slave_ch].op.watt;
			}else{
				myData->save10ms_msg[msg][bd][ch].chData[idx].DischargeWatt
					= myData->bData[bd].cData[ch].misc.tmpWatt
					+ myData->bData[bd].cData[slave_ch].misc.tmpWatt;
			}
		}else{
			myData->save10ms_msg[msg][bd][ch].chData[idx].ChargeWatt
				= 0;
			myData->save10ms_msg[msg][bd][ch].chData[idx].DischargeWatt
				= 0;
		}
	}else{
		myData->save10ms_msg[msg][bd][ch].chData[idx].IntegralAmpareHour
			= myData->bData[bd].cData[ch].op.integral_ampareHour;
		myData->save10ms_msg[msg][bd][ch].chData[idx].IntegralWattHour
			= myData->bData[bd].cData[ch].op.integral_WattHour;
		myData->save10ms_msg[msg][bd][ch].chData[idx].ChargeAmpareHour
			= myData->bData[bd].cData[ch].op.charge_ampareHour;
		myData->save10ms_msg[msg][bd][ch].chData[idx].ChargeWattHour
			= myData->bData[bd].cData[ch].op.charge_wattHour;
		myData->save10ms_msg[msg][bd][ch].chData[idx].DischargeAmpareHour
			= myData->bData[bd].cData[ch].op.discharge_ampareHour;
		myData->save10ms_msg[msg][bd][ch].chData[idx].DischargeWattHour
			= myData->bData[bd].cData[ch].op.discharge_wattHour;
			

		myData->save10ms_msg[msg][bd][ch].chData[idx].charge_cc_ampare_hour
			= myData->bData[bd].cData[ch].misc.chargeCCAh;
		myData->save10ms_msg[msg][bd][ch].chData[idx].charge_cv_ampare_hour
			= myData->bData[bd].cData[ch].misc.chargeCVAh;
		myData->save10ms_msg[msg][bd][ch].chData[idx].discharge_cc_ampare_hour
			= myData->bData[bd].cData[ch].misc.dischargeCCAh;
		myData->save10ms_msg[msg][bd][ch].chData[idx].discharge_cv_ampare_hour
			= myData->bData[bd].cData[ch].misc.dischargeCVAh;

		if(stepType == STEP_CHARGE){
			myData->save10ms_msg[msg][bd][ch].chData[idx].DischargeWatt = 0;
			if(dataType == 0){
				myData->save10ms_msg[msg][bd][ch].chData[idx].ChargeWatt
					= myData->bData[bd].cData[ch].op.watt;
			}else{
				myData->save10ms_msg[msg][bd][ch].chData[idx].ChargeWatt
					= myData->bData[bd].cData[ch].misc.tmpWatt;
			}
		}else if(stepType == STEP_DISCHARGE){
			myData->save10ms_msg[msg][bd][ch].chData[idx].ChargeWatt = 0;
			if(dataType == 0){
				myData->save10ms_msg[msg][bd][ch].chData[idx].DischargeWatt
					= myData->bData[bd].cData[ch].op.watt;
			}else{
				myData->save10ms_msg[msg][bd][ch].chData[idx].DischargeWatt
					= myData->bData[bd].cData[ch].misc.tmpWatt;
			}
		}else{
			myData->save10ms_msg[msg][bd][ch].chData[idx].ChargeWatt
				= 0;
			myData->save10ms_msg[msg][bd][ch].chData[idx].DischargeWatt
				= 0;
		}
	}
	
	myData->save10ms_msg[msg][bd][ch].chData[idx].startVoltage
		= myData->bData[bd].cData[ch].misc.startV;
	myData->save10ms_msg[msg][bd][ch].chData[idx].step_count
		= myData->bData[bd].cData[ch].misc.step_count;

	if(myData->bData[bd].cData[ch].op.type != STEP_END) {
		if(myData->save10ms_msg[msg][bd][ch].chData[idx].Vsens
				> myData->bData[bd].cData[ch].misc.maxV) {
			myData->bData[bd].cData[ch].misc.maxV = 
				myData->save10ms_msg[msg][bd][ch].chData[idx].Vsens;
		}
		if(myData->save10ms_msg[msg][bd][ch].chData[idx].Vsens
				< myData->bData[bd].cData[ch].misc.minV) {
			myData->bData[bd].cData[ch].misc.minV = 
				myData->save10ms_msg[msg][bd][ch].chData[idx].Vsens;
		}
		if(myData->save10ms_msg[msg][bd][ch].chData[idx].Vsens
				> myData->bData[bd].cData[ch].misc.cycleMaxV) {
			myData->bData[bd].cData[ch].misc.cycleMaxV = 
				myData->save10ms_msg[msg][bd][ch].chData[idx].Vsens;
		}
		if(myData->save10ms_msg[msg][bd][ch].chData[idx].Vsens
				< myData->bData[bd].cData[ch].misc.cycleMinV) {
			myData->bData[bd].cData[ch].misc.cycleMinV = 
				myData->save10ms_msg[msg][bd][ch].chData[idx].Vsens;
		}
		if(myData->bData[bd].cData[ch].op.temp
				< myData->bData[bd].cData[ch].misc.minT) {
			myData->bData[bd].cData[ch].misc.minT = 
				myData->bData[bd].cData[ch].op.temp;
		}
		myData->save10ms_msg[msg][bd][ch].chData[idx].temp[0][1]
			= myData->bData[bd].cData[ch].misc.minT;
		if(myData->bData[bd].cData[ch].op.temp
				> myData->bData[bd].cData[ch].misc.maxT) {
			myData->bData[bd].cData[ch].misc.maxT = 
				myData->bData[bd].cData[ch].op.temp;
		}
		myData->save10ms_msg[msg][bd][ch].chData[idx].temp[0][2]
			= myData->bData[bd].cData[ch].misc.maxT;
	}	
	myData->save10ms_msg[msg][bd][ch].chData[idx].maxVoltage
			= myData->bData[bd].cData[ch].misc.maxV;
	myData->save10ms_msg[msg][bd][ch].chData[idx].minVoltage
			= myData->bData[bd].cData[ch].misc.minV;

	if(myData->bData[bd].cData[ch].op.select == SAVE_FLAG_SAVING_END
		&& myData->bData[bd].cData[ch].misc.mes_data_flag == 1){
		if(stepType == STEP_LOOP){
			myData->save10ms_msg[msg][bd][ch].chData[idx].ChargeWatt
				= myData->bData[bd].cData[ch].misc.cycleSumChargeWatt;
			myData->save10ms_msg[msg][bd][ch].chData[idx].DischargeWatt
				= myData->bData[bd].cData[ch].misc.cycleSumDischargeWatt;	
			myData->save10ms_msg[msg][bd][ch].chData[idx].ChargeAmpareHour
				= myData->bData[bd].cData[ch].misc.cycleSumChargeAmpareHour;
			myData->save10ms_msg[msg][bd][ch].chData[idx].DischargeAmpareHour
				= myData->bData[bd].cData[ch].misc.cycleSumDischargeAmpareHour;
			myData->save10ms_msg[msg][bd][ch].chData[idx].ChargeWattHour
				= myData->bData[bd].cData[ch].misc.cycleSumChargeWattHour;
			myData->save10ms_msg[msg][bd][ch].chData[idx].DischargeWattHour
				= myData->bData[bd].cData[ch].misc.cycleSumDischargeWattHour;
//	충전 CC용량	
			myData->save10ms_msg[msg][bd][ch].chData[idx].charge_cc_ampare_hour
				= myData->bData[bd].cData[ch].misc.cycleSumChargeCCAh;
//	충전 CV용량	
			myData->save10ms_msg[msg][bd][ch].chData[idx].charge_cv_ampare_hour
				= myData->bData[bd].cData[ch].misc.cycleSumChargeCVAh;
//	방전 CC용량
			myData->save10ms_msg[msg][bd][ch].chData[idx].discharge_cc_ampare_hour
				= myData->bData[bd].cData[ch].misc.cycleSumDischargeCCAh;
//	방전 CV용량
			myData->save10ms_msg[msg][bd][ch].chData[idx].discharge_cv_ampare_hour
				= myData->bData[bd].cData[ch].misc.cycleSumDischargeCVAh;
	
			if(myData->bData[bd].cData[ch].misc.cycleRunTime
					>= ONE_DAY_RUNTIME){
				myData->save10ms_msg[msg][bd][ch].chData[idx].runTime_day
					= myData->bData[bd].cData[ch].misc.cycleRunTime
						/ ONE_DAY_RUNTIME;
				myData->save10ms_msg[msg][bd][ch].chData[idx].runTime
					= myData->bData[bd].cData[ch].misc.cycleRunTime
						% ONE_DAY_RUNTIME;
			}else{
				myData->save10ms_msg[msg][bd][ch].chData[idx].runTime_day
					= 0;
				myData->save10ms_msg[msg][bd][ch].chData[idx].runTime
					= myData->bData[bd].cData[ch].misc.cycleRunTime;
			}

			if(myData->bData[bd].cData[ch].misc.cycleStepCount != 0){
				myData->save10ms_msg[msg][bd][ch].chData[idx].temp[0][3]
					= myData->bData[bd].cData[ch].misc.cycleSumAvgT
					/ myData->bData[bd].cData[ch].misc.cycleStepCount;
			}
			if(myData->bData[bd].cData[ch].misc.cycleDischargeStepCount != 0){
				myData->save10ms_msg[msg][bd][ch].chData[idx].avgV
					= myData->bData[bd].cData[ch].misc.cycleAvgDischargeV
					/ myData->bData[bd].cData[ch].misc.cycleDischargeStepCount;
				myData->save10ms_msg[msg][bd][ch].chData[idx].avgI
					= myData->bData[bd].cData[ch].misc.cycleAvgDischargeI
					/ myData->bData[bd].cData[ch].misc.cycleDischargeStepCount;
			}

			myData->save10ms_msg[msg][bd][ch].chData[idx].startVoltage
				= myData->bData[bd].cData[ch].misc.cycleStartV;

			myData->save10ms_msg[msg][bd][ch].chData[idx].maxVoltage
				= myData->bData[bd].cData[ch].misc.cycleMaxV;
			myData->save10ms_msg[msg][bd][ch].chData[idx].minVoltage
				= myData->bData[bd].cData[ch].misc.cycleMinV;

// 사이클 충전 CC 시간
			if(myData->bData[bd].cData[ch].misc.cycle_Charge_ccTime
					>= ONE_DAY_RUNTIME){
				myData->save10ms_msg[msg][bd][ch].chData[idx].ccTime_day
					= myData->bData[bd].cData[ch].misc.cycle_Charge_ccTime
						/ ONE_DAY_RUNTIME;	
				myData->save10ms_msg[msg][bd][ch].chData[idx].ccTime
					= myData->bData[bd].cData[ch].misc.cycle_Charge_ccTime
						% ONE_DAY_RUNTIME;	
			}else{
				myData->save10ms_msg[msg][bd][ch].chData[idx].ccTime_day
					= 0;	
				myData->save10ms_msg[msg][bd][ch].chData[idx].ccTime
					= myData->bData[bd].cData[ch].misc.cycle_Charge_ccTime;	
			}
// 사이클 충전 CV 시간
			if(myData->bData[bd].cData[ch].misc.cycle_Charge_cvTime
					>= ONE_DAY_RUNTIME){
				myData->save10ms_msg[msg][bd][ch].chData[idx].cvTime_day
					= myData->bData[bd].cData[ch].misc.cycle_Charge_cvTime
						/ ONE_DAY_RUNTIME;	
				myData->save10ms_msg[msg][bd][ch].chData[idx].cvTime
					= myData->bData[bd].cData[ch].misc.cycle_Charge_cvTime
						% ONE_DAY_RUNTIME;	
			}else{
				myData->save10ms_msg[msg][bd][ch].chData[idx].cvTime_day
					= 0;	
				myData->save10ms_msg[msg][bd][ch].chData[idx].cvTime
					= myData->bData[bd].cData[ch].misc.cycle_Charge_cvTime;	
			}
		}else{
			if(stepType == STEP_CHARGE){
				if(myData->bData[bd].cData[ch].ChAttribute.opType == P1){
					myData->bData[bd].cData[ch].misc.cycleSumChargeWatt
						+= (myData->bData[bd].cData[ch].op.watt
						+ myData->bData[bd].cData[slave_ch].op.watt);
					myData->bData[bd].cData[ch].misc.cycleSumChargeAmpareHour
						+= (myData->bData[bd].cData[ch].op.charge_ampareHour
						+ myData->bData[bd].cData[slave_ch].op.charge_ampareHour);					
					myData->bData[bd].cData[ch].misc.cycleSumChargeWattHour
						+= (myData->bData[bd].cData[ch].op.charge_wattHour
						+ myData->bData[bd].cData[slave_ch].op.charge_wattHour);
					myData->bData[bd].cData[ch].misc.cycleSumChargeCCAh
						+= myData->bData[bd].cData[ch].misc.chargeCCAh
						+ myData->bData[bd].cData[slave_ch].misc.chargeCCAh;
					myData->bData[bd].cData[ch].misc.cycleSumChargeCVAh
						+= myData->bData[bd].cData[ch].misc.chargeCVAh
						+ myData->bData[bd].cData[slave_ch].misc.chargeCVAh;
				}else{
					myData->bData[bd].cData[ch].misc.cycleSumChargeWatt
						+= myData->bData[bd].cData[ch].op.watt;
					myData->bData[bd].cData[ch].misc.cycleSumChargeAmpareHour
						+= myData->bData[bd].cData[ch].op.charge_ampareHour;
					myData->bData[bd].cData[ch].misc.cycleSumChargeWattHour
						+= myData->bData[bd].cData[ch].op.charge_wattHour;
					myData->bData[bd].cData[ch].misc.cycleSumChargeCCAh
						+= myData->bData[bd].cData[ch].misc.chargeCCAh;
					myData->bData[bd].cData[ch].misc.cycleSumChargeCVAh
						+= myData->bData[bd].cData[ch].misc.chargeCVAh;
				}
			}else if(stepType == STEP_DISCHARGE){
				if(myData->bData[bd].cData[ch].ChAttribute.opType == P1){
					myData->bData[bd].cData[ch].misc.cycleSumDischargeWatt
						+= (myData->bData[bd].cData[ch].op.watt
						+ myData->bData[bd].cData[slave_ch].op.watt);

					myData->bData[bd].cData[ch].misc.cycleSumDischargeAmpareHour
					+= (myData->bData[bd].cData[ch].op.discharge_ampareHour
					+ myData->bData[bd].cData[slave_ch].op.discharge_ampareHour);
					myData->bData[bd].cData[ch].misc.cycleSumDischargeWattHour
					+= (myData->bData[bd].cData[ch].op.discharge_wattHour
					+ myData->bData[bd].cData[slave_ch].op.discharge_wattHour);
					myData->bData[bd].cData[ch].misc.cycleSumDischargeCCAh
						+= myData->bData[bd].cData[ch].misc.dischargeCCAh
						+ myData->bData[bd].cData[slave_ch].misc.dischargeCCAh;
					myData->bData[bd].cData[ch].misc.cycleSumDischargeCVAh
						+= myData->bData[bd].cData[ch].misc.dischargeCVAh
						+ myData->bData[bd].cData[slave_ch].misc.dischargeCVAh;
				}else{
					myData->bData[bd].cData[ch].misc.cycleSumDischargeWatt
						+= myData->bData[bd].cData[ch].op.watt;
					myData->bData[bd].cData[ch].misc.cycleSumDischargeAmpareHour
						+= myData->bData[bd].cData[ch].op.discharge_ampareHour;
					myData->bData[bd].cData[ch].misc.cycleSumDischargeWattHour
						+= myData->bData[bd].cData[ch].op.discharge_wattHour;
					myData->bData[bd].cData[ch].misc.cycleSumDischargeCCAh
						+= myData->bData[bd].cData[ch].misc.dischargeCCAh;
					myData->bData[bd].cData[ch].misc.cycleSumDischargeCVAh
						+= myData->bData[bd].cData[ch].misc.dischargeCVAh;
				}
				myData->bData[bd].cData[ch].misc.cycleAvgDischargeV
					+= myData->bData[bd].cData[ch].op.meanVolt;
				myData->bData[bd].cData[ch].misc.cycleAvgDischargeI
					+= myData->bData[bd].cData[ch].op.meanCurr;	
			}
			myData->bData[bd].cData[ch].misc.cycleSumAvgT
				+= myData->bData[bd].cData[ch].op.meanTemp;
			
			myData->bData[bd].cData[ch].misc.mes_data_flag = 0;
		}
	}
	// 131228 oys w : real time add
	#if REAL_TIME == 1
	myData->save10ms_msg[msg][bd][ch].chData[idx].realDate
		= myData->mData.real_time[6] * 10000	//year
		+ myData->mData.real_time[5] * 100		//month
		+ myData->mData.real_time[4];			//day
	myData->save10ms_msg[msg][bd][ch].chData[idx].realClock
		= myData->mData.real_time[3] * 10000000	//hour
		+ myData->mData.real_time[2] * 100000	//min
		+ myData->mData.real_time[1] * 1000		//sec
		+ myData->mData.real_time[0];			//msec
	#endif

	if(type == 1){	//mem copy
		for(i=0; i < idx+1; i++){
			memset((char *)&myData->save10ms_msg[msg+2][bd][ch].chData[i],
			0x00, sizeof(S_SAVE_MSG_CH_DATA));
			memcpy((char *)&myData->save10ms_msg[msg+2][bd][ch].chData[i],
				(char *)&myData->save10ms_msg[msg][bd][ch].chData[i],
				sizeof(S_SAVE_MSG_CH_DATA));
			memset((char *)&myData->save10ms_msg[msg][bd][ch].chData[i],
			0x00, sizeof(S_SAVE_MSG_CH_DATA));
		}
		myData->save10ms_msg[msg+2][bd][ch].flag = 1;
		myData->save10ms_msg[msg+2][bd][ch].write_idx = idx;
	}

//	myData->save10ms_msg[msg][bd][ch].flag = type;
	myData->save10ms_msg[msg][bd][ch].write_idx++;
//	myData->save10ms_msg[msg+2][bd][ch].count++;

	if(myData->bData[bd].cData[ch].op.type == STEP_END)
		myData->save10ms_msg[msg][bd][ch].write_idx = 0;
}
#endif
