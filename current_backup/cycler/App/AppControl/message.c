#include <time.h>
#include <string.h>
#include "../../INC/datastore.h"
#include "userlog.h"
#include "common_utils.h"
#include "local_utils.h"
#include "message.h"

extern volatile S_SYSTEM_DATA *myData;
extern volatile S_APP_CONTROL *myPs;
extern char psName[PROCESS_NAME_SIZE];

void Check_Message(void)
{
	rcv_msg(MAIN_TO_APP);
	rcv_msg(MODULE_TO_APP);
	rcv_msg(DATASAVE_TO_APP);
	rcv_msg(METER_TO_APP);
	rcv_msg(EXT_TO_APP);
	rcv_msg(MODULE_TO_APP_VALUE);
	rcv_msg(COOLING_TO_APP);
}

void rcv_msg(int fromPs)
{
	int idx, msg, ch, val, rtn;
	int bd;
	long event_val;
	
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
		case MAIN_TO_APP:
			rtn = msgParsing_Main_to_App(msg, ch, val, idx);
			break;
		case EXT_TO_APP:
			rtn = msgParsing_Ext_to_App(msg, ch, val, idx);
			break;
		case MODULE_TO_APP:
			rtn = msgParsing_Module_to_App(msg, ch, val, idx);
			break;
		case METER_TO_APP:
			rtn = msgParsing_Meter_to_App(msg, ch, val, idx);
			break;
		case MODULE_TO_APP_VALUE:	//20190527
			bd = myData->msg[fromPs].msg_val[idx].bd;
			event_val = myData->msg[fromPs].msg_val[idx].event_val;
			
			rtn = msgParsing_Module_to_App_value(msg, ch, val, idx, 
															bd, event_val); 
			break;
		case COOLING_TO_APP:
			rtn = msgParsing_Cooling_to_App(msg, ch, val, idx);
			break;
		default:
			userlog(DEBUG_LOG, psName,
				"Msg Direction UnKnown : %d %d %d\n", msg, ch, val);
			break;
	}
}

int msgParsing_Main_to_App(int msg, int ch, int val, int idx)
{
	int rtn=0;

	switch(msg) {
		case MSG_MAIN_APP_PROCESS_KILL:
			userlog(DEBUG_LOG, psName, "Kill Process MainClient 1\n");
			if(myData->MainClient.misc.processPointer > 0) {
				Close_mbuff(myData->MainClient.misc.processPointer);
			}
			Kill_Process("MainClient");
			break;
		case MSG_MAIN_APP_WRITE_CHATTRIBUTE:
			rtn = Write_ChAttribute();
			break;
		case MSG_MAIN_APP_WRITE_CHCOMPDATA:
			rtn = Write_ChCompData();
			break;
		case MSG_MAIN_APP_WRITE_AUX_SET_DATA:
			rtn = Write_AuxSetData();
			break;
		//20171008 ach add
		case MSG_MAIN_APP_WRITE_LIMIT_USER_VI:
			rtn = Write_LimitUserVI();
			break;
		case MSG_MAIN_APP_PARAMETER_UPDATE:
			rtn = Read_Parameter_Update();
			break;
		case MSG_MAIN_APP_WRITE_HWFAULT_CONFIG:
			rtn = Write_HwFault_Config(); //210204 lyhw
			break;
		case MSG_MAIN_APP_WRITE_CHAMBER_CH_NO:
			rtn = Write_ChamberChNo();
			break;
		case MSG_MAIN_APP_WRITE_SW_FAULT_CONFIG:
			rtn = Write_SwFault_Config(); //210418 hun
			break;
		case MSG_MAIN_APP_WRITE_LGES_FAULT_CONFIG:
			#ifdef _LGES
			rtn = Write_LGES_Fault_Config(); //211025 hun
			#endif
			break;
		case MSG_MAIN_APP_WRITE_SDI_CC_CV_HUMP_CONFIG:
			#ifdef _SDI
			rtn = Write_SDI_CC_CV_hump_Config(); //211025 hun
			#endif
			break;
		case MSG_MAIN_APP_WRITE_SDI_PAUSE_SAVE_CONFIG:
			#ifdef _SDI
			rtn = Write_SDI_Pause_save_Config(); //211025 hun
			#endif
			break;
		case MSG_MAIN_APP_WRITE_DYSON_MAINTENANCE_CONFIG:
			rtn = Write_DYSON_Maintenance_Config(); //220121 jsh
			break;

		default:
			userlog(DEBUG_LOG, psName,
				"Main to App Msg Unknown : %d %d %d\n", msg, ch, val);
			break;
	}
	return rtn;
}

int msgParsing_Ext_to_App(int msg, int ch, int val, int idx)
{
	int rtn=0;

	switch(msg) {
		case MSG_EXT_APP_PROCESS_KILL:
			userlog(DEBUG_LOG, psName, "Kill Process ExtClient 1\n");
			if(myData->ExtClient.misc.processPointer > 0) {
				Close_mbuff(myData->ExtClient.misc.processPointer);
			}
			Kill_Process("ExtClient");
			break;
		case MSG_EXT_APP_WRITE_CHATTRIBUTE:
			rtn = Write_ChAttribute();
			break;
		case MSG_EXT_APP_WRITE_CHCOMPDATA:
			rtn = Write_ChCompData();
			break;
		case MSG_EXT_APP_WRITE_AUX_SET_DATA:
			rtn = Write_AuxSetData();
			break;
		default:
			userlog(DEBUG_LOG, psName,
				"Ext to App Msg Unknown : %d %d %d\n", msg, ch, val);
			break;
	}
	return rtn;
}

int msgParsing_Module_to_App(int msg, int ch, int val, int idx)
{
	int rtn=0, i;
	time_t the_time;

	switch(msg) {
		case MSG_MODULE_APP_POWER_SWITCH:
			(void)time(&the_time);
			myPs->misc.quitDelayTime = the_time;
			if(ch == 0) {
				userlog(DEBUG_LOG, psName, "power switch detect : normal\n");
				myPs->signal[APP_SIG_QUIT] = P1;
			} else if(ch == 1) {
				userlog(DEBUG_LOG, psName, "power switch detect : M_RUNNING\n");
			} else if(ch == 2) {
				userlog(DEBUG_LOG, psName, "power switch detect : force\n");
				myPs->signal[APP_SIG_QUIT] = P2;
			} else if(ch == 3) {
				userlog(DEBUG_LOG, psName, "terminal key input : quit\n");
				myPs->signal[APP_SIG_QUIT] = P3;
			} else if(ch == 4) {
				userlog(DEBUG_LOG, psName, "terminal key input : halt\n");
				myPs->signal[APP_SIG_QUIT] = P4;
			}
			if(myData->AnalogMeter.config.countMeter > 0) {
				send_msg(APP_TO_METER2, MSG_APP_METER2_MEASURE, 1, 1);
			}
			break;
		case MSG_MODULE_APP_EMG:
			(void)time(&the_time);
			myPs->misc.quitDelayTime = the_time;
			//ch == 0 : force power switch
			if(ch == 1) {
				userlog(DEBUG_LOG, psName, "main emergency button\n");
				myPs->signal[APP_SIG_QUIT] = P5;
			} else if(ch == 2) {
				userlog(DEBUG_LOG, psName, "sub emergency button\n");
				myPs->signal[APP_SIG_QUIT] = P6;
			} else if(ch == 3) {
				userlog(DEBUG_LOG, psName, "ac power fail\n");
				myPs->signal[APP_SIG_QUIT] = P7;
			} else if(ch == 4) {
				userlog(DEBUG_LOG, psName, "ups battery fail\n");
				myPs->signal[APP_SIG_QUIT] = P8;
			} else if(ch == 5) {
				userlog(DEBUG_LOG, psName, "control smps fail\n");
				myPs->signal[APP_SIG_QUIT] = P9;
			} else if(ch == 6) {
				userlog(DEBUG_LOG, psName, "OT fail %d\n", val);
				myPs->signal[APP_SIG_QUIT] = P10;
			} else if(ch == 7) {
				userlog(DEBUG_LOG, psName, "main smps fail %d\n", val);
				myPs->signal[APP_SIG_QUIT] = P10;
			} else if(ch == 8) {
				userlog(DEBUG_LOG, psName, "inverter fail %d\n", val);
				myPs->signal[APP_SIG_QUIT] = P10;
			} else if(ch == 9) {
				userlog(DEBUG_LOG, psName, "voltage plus level fail %d\n",val);
			} else if(ch == 10) {
				userlog(DEBUG_LOG, psName, "voltage minus level fail %d\n",val);
			} else if(ch == 11) {
				userlog(DEBUG_LOG, psName, "ch voltage level fail %d\n",val);
			} else if(ch == 12) {
				userlog(DEBUG_LOG, psName, "OV fail %d\n",val);
			} else if(ch == 13) {
				userlog(DEBUG_LOG, psName, "OC fail %d\n",val);
			} else if(ch == 14) {
				userlog(DEBUG_LOG, psName, "OT_COMPARE fail %d\n",val);
			} else if(ch == 15) {
				userlog(DEBUG_LOG, psName, "meter high fail %d\n",val);
				myPs->signal[APP_SIG_QUIT] = P11;
			} else if(ch == 16) {
				userlog(DEBUG_LOG, psName, "meter low fail %d\n",val);
				myPs->signal[APP_SIG_QUIT] = P11;
			} else if(ch == 17) {
				userlog(DEBUG_LOG, psName, "chamber error %d\n",val);
				myPs->signal[APP_SIG_QUIT] = P12;
			} else if(ch == 18) {
				userlog(DEBUG_LOG, psName, "Smps Fault 7V %d\n",val);
			} else if(ch == 19) {
				userlog(DEBUG_LOG, psName, "Smps Fault 3V %d\n",val);
			} else if(ch == 20) {
				userlog(DEBUG_LOG, psName, "Ch OVP Detect %d\n",val);
				myPs->signal[APP_SIG_QUIT] = P13;
			} else if(ch == 21) {
				userlog(DEBUG_LOG, psName, "Ch OTP Detect %d\n",val);
				myPs->signal[APP_SIG_QUIT] = P14;
			} else if(ch == 22) {
				userlog(DEBUG_LOG, psName, "Fail Ch Detect %d\n",val);
				myPs->signal[APP_SIG_QUIT] = P15;
			} else if(ch == 23) {
				userlog(DEBUG_LOG, psName, "DC_FAN_FAIL1 %d\n",val);
			} else if(ch == 24) {
				userlog(DEBUG_LOG, psName, "DC_FAN_FAIL2 %d\n",val);
			} else if(ch == 25) {
				userlog(DEBUG_LOG, psName, "DOOR_OPEN_FAIL1 %d\n",val);
			} else if(ch == 26) {
				userlog(DEBUG_LOG, psName, "DOOR_OPEN_FAIL2 %d\n",val);
			} else if(ch == 27) {
				userlog(DEBUG_LOG, psName, "SMOKE_SENSOR_FAIL %d\n",val);
				myPs->signal[APP_SIG_QUIT] = P16;
			} else if(ch == 28) {
				userlog(DEBUG_LOG, psName, "OT Fail Pause %dCh\n",val);
			} else if(ch == 29) { //20181108
#if CYCLER_TYPE == LINEAR_CYC || CYCLER_TYPE == CAN_CYC
				userlog(DEBUG_LOG, psName, "GUI SHUTDOWN SIGNAL %d\n",val);
				myPs->signal[APP_SIG_QUIT] = P17;
#else
				userlog(DEBUG_LOG, psName, "DOOR_OPEN_FAIL3 %d\n",val);
#endif
			} else if(ch == 30) { //190123 add
				userlog(DEBUG_LOG, psName, "SMOKE_SENSOR_FAIL2 %d\n",val);
				myPs->signal[APP_SIG_QUIT] = P16;
			} else if(ch == 31) {
				userlog(DEBUG_LOG, psName,
					"[CAN][%d]MAIN BD CAN COMM ERROR FAULT\n", val);
			} else if(ch == 32) {
				userlog(DEBUG_LOG, psName,
					"[CAN][%d]INV CAN COMM ERROR FAULT[%x] VDC[%d]mV\n",
					val,
					myData->CAN.invsavefault[val].code,
					myData->CAN.invsavefault[val].vdc);
			} else if(ch == 33) {
				userlog(DEBUG_LOG, psName,
					"[CAN]IO BD CAN COMM ERROR FAULT\n");
			} else if(ch == 34) {
				userlog(DEBUG_LOG, psName,
					"[CAN][%d]INV ERROR FAULT[%x] VDC[%d]mV Step[%d]\n",
					val,
					myData->CAN.invsavefault[val].code,
					myData->CAN.invsavefault[val].vdc,
					myData->CAN.invsavefault[val].step);
			} else if(ch == 35){	//210625
				userlog(DEBUG_LOG, psName, "momentary power fail\n");
			} else if(ch == 36){	//220113
				userlog(DEBUG_LOG, psName, 
					"MAIN BD CAN COMM ERROR TOTAL [ShutDown]\n");
				myPs->signal[APP_SIG_QUIT] = P18;
			} else if(ch == 37){	//220204 jsh
				userlog(DEBUG_LOG, psName, "DOOR_OPEN_FAIL4_SHUTDOWN\n");
				myPs->signal[APP_SIG_QUIT] = P19;
			} else if(ch == 38){	//220214_hun
				userlog(DEBUG_LOG, psName, "CHAMBER_FAULT_SHUTDOWN code:[%d]\n",val);
				myPs->signal[APP_SIG_QUIT] = P20;
			} 
			if(myPs->signal[APP_SIG_QUIT] != P0){
				if(myData->AnalogMeter.config.countMeter > 0) {
					send_msg(APP_TO_METER2, MSG_APP_METER2_MEASURE, 1, 1);
				}
			}
			break;
		case MSG_MODULE_APP_CH_EMG:
			if(ch == C_FAULT_HARD_VENTING){	//220322_hun
				userlog(DEBUG_LOG, psName, "HARD_VENTING_ShutDown chNo:[%d]\n",val);
				myPs->signal[APP_SIG_QUIT] = P21;
			}
			if(myPs->signal[APP_SIG_QUIT] != P0){
				if(myData->AnalogMeter.config.countMeter > 0) {
					send_msg(APP_TO_METER2, MSG_APP_METER2_MEASURE, 1, 1);
				}
			}
			break;
		case MSG_MODULE_APP_PROCESS_KILL:
			userlog(DEBUG_LOG, psName, "Kill Process MainClient 2\n");
			if(myData->MainClient.misc.processPointer > 0) {
				Close_mbuff(myData->MainClient.misc.processPointer);
			}
			Kill_Process("MainClient");
			break;
		case MSG_MODULE_APP_AUX_CALI:
			if(ch == 1) {
				userlog(DEBUG_LOG, psName, "Module Aux Cali Low Point\n");
				if(val == 0) {
					for(i = 0;i<myData->mData.config.installedAuxV;i++) {
						userlog(DEBUG_LOG, psName, "ch : %d , val : %ld \n",i
						,myData->daq.tmp_low.ch_vsens[i]);
					}
				} else {
					for(i = (val-1)*16;i<(val * 16);i++) {
						if(i >= myData->mData.config.installedAuxV)
							return rtn;
						userlog(DEBUG_LOG, psName, "ch : %d , val : %ld \n",i
						,myData->daq.tmp_low.ch_vsens[i]);
					}
				}
			} else if(ch == 2) {
				userlog(DEBUG_LOG, psName, "Module Aux Cali High Point\n");
				if(val == 0) {
					for(i = 0;i<myData->mData.config.installedAuxV;i++) {
						userlog(DEBUG_LOG, psName, "ch : %d , val : %ld \n",i
						,myData->daq.tmp_high.ch_vsens[i]);
					}
				} else {
					for(i = (val-1)*16;i<(val * 16);i++) {
						if(i >= myData->mData.config.installedAuxV)
							return rtn;
						userlog(DEBUG_LOG, psName, "ch : %d , val : %ld \n",i
						,myData->daq.tmp_high.ch_vsens[i]);
					}
				}
			} else if(ch == 3) {
				for(i = 0;i<myData->mData.config.installedAuxV;i++) {
					if(myData->daq.misc.caliFlag[i] && 0x03 == 0x03) {
						userlog(DEBUG_LOG, psName, "ch:%d,A:%f,AD_B : %f\n",i
						,myData->daq.tmpCali.AD_A[i],myData->daq.tmpCali.AD_B[i]);
						myData->daq.misc.caliFlag[i] = 0;
					}
				}
				rtn = Write_AuxCaliData();
				userlog(DEBUG_LOG, psName, "Module Aux Cali Update\n");
			}
			break;
		case MSG_MODULE_APP_D_CALI_UPDATE:
				userlog(DEBUG_LOG, psName, "D Cali Update [%d]ch\n", ch); 
			break;	
		default:
			userlog(DEBUG_LOG, psName,
				"Module to App Msg Unknown : %d %d %d\n", msg, ch, val);
			break;
	}
	return rtn;
}

int msgParsing_Module_to_App_value(int msg, int ch, int val, int idx, int bd, long event_val)
{
	int rtn=0;
	time_t the_time;

	switch(msg) {
		case MSG_MODULE_APP_POWER_SWITCH:
			(void)time(&the_time);
			myPs->misc.quitDelayTime = the_time;
			break;
		case MSG_MODULE_APP_EMG:
			(void)time(&the_time);
			myPs->misc.quitDelayTime = the_time;
			//ch == 0 : force power switch
			if(ch == 1) {
				userlog(DEBUG_LOG, psName, "OVP Detect Ch:[%d] Value [%ld]uV\n",val,event_val);
				userlog(DEBUG_LOG, psName, "System Halt\n");
				myPs->signal[APP_SIG_QUIT] = P13;
			} else if(ch == 2) {
				userlog(DEBUG_LOG, psName, "Ch OTP Detect Ch[%d] Value [%ld]mT\n",val,event_val);
				myPs->signal[APP_SIG_QUIT] = P14;
			} else if(ch == 3) {
				userlog(DEBUG_LOG, psName, "Fail Ch Detect Ch[%d] Value [%ld]uA\n",val, event_val);
				myPs->signal[APP_SIG_QUIT] = P15;
			}	
			if(myPs->signal[APP_SIG_QUIT] != P0){
				if(myData->AnalogMeter.config.countMeter > 0) {
					send_msg(APP_TO_METER2, MSG_APP_METER2_MEASURE, 1, 1);
				}
			}
			break;
		case MSG_MODULE_APP_PROCESS_KILL:
			break;
		case MSG_MODULE_APP_AUX_CALI:
			break;
		default:
			userlog(DEBUG_LOG, psName,
				"Module to App Msg Unknown : %d %d %d\n", msg, ch, val);
			break;
	}
	return rtn;
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

int msgParsing_Meter_to_App(int msg, int ch, int val, int idx)
{
	int rtn=0;

	switch(msg) {
		default:
			userlog(DEBUG_LOG, psName,
				"Meter to App Msg Unknown : %d %d %d\n", msg, ch, val);
			break;
	}
	return rtn;
}

//220314 jws add
int msgParsing_Cooling_to_App(int msg, int ch, int val, int idx)
{
	int rtn = 0;

	switch(msg){
		case MSG_COOLING_APP_PROCESS_KILL:
			userlog(DEBUG_LOG, psName, "Kill Process CoolingControl\n");
			if(myData->CoolingControl.misc.processPointer > 0){
				Close_mbuff(myData->CoolingControl.misc.processPointer);
			}
			Kill_Process("CoolingControl");
			break;
		default:
			userlog(DEBUG_LOG, psName,
				"Main to App Msg Unknown : %d %d %d\n", msg, ch, val);
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

