#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include "../../INC/datastore.h"
#include "userlog.h"
#include "message.h"
#include "common_utils.h"
#include "network.h"
#include "local_utils.h"

extern volatile S_SYSTEM_DATA *myData;
extern volatile S_EXT_CLIENT *myPs;
extern char psName[PROCESS_NAME_SIZE];

int Initialize(void)
{
	if(Open_SystemMemory(0) < 0) return -1;
	
	myPs = &(myData->ExtClient);
	
	Init_SystemMemory();
	
	if(Read_ExtClient_Config() < 0) return -2;
	
	myData->AppControl.signal[myPs->misc.psSignal] = P1;
	return 0;
}

void Init_SystemMemory(void)
{
	int i;
	
	memset((char *)&psName[0], 0, PROCESS_NAME_SIZE);
	strcpy(psName, "Ext");
    myPs->misc.psSignal = APP_SIG_EXT_CLIENT_PROCESS;

	for(i=0; i < MAX_SIGNAL; i++) {
		myPs->signal[i] = P0;
	}
	
	myPs->netTimer = myData->mData.misc.timer_1sec;
	myPs->pingTimer = myData->mData.misc.timer_1sec;
	myPs->pingCount = 0;

	memset((char *)&myPs->rcvCmd, 0x00, sizeof(S_EXT_CLIENT_RCV_COMMAND));
	memset((char *)&myPs->rcvPacket, 0x00, sizeof(S_EXT_CLIENT_RCV_PACKET));
	
	myPs->misc.cmd_serial = 0;
	myPs->misc.processPointer = (int)&myData;
}

int	Read_ExtClient_Config(void)
{
    int tmp;
	char temp[20], buf[12], fileName[128];
    FILE *fp;

	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/cycler_data/config/parameter/ExtClient_Config");
	// /root/cycler_data/config/parameter/ExtClient_Config
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "ExtClient_Config file read error\n");
		system("cp ../Config_backup/ExtClient_Config /root/cycler_data/config/parameter");
		userlog(DEBUG_LOG, psName, "ExtClient_Config file copy\n");
		if((fp = fopen(fileName, "r")) == NULL) {
			return -1;
		}
	}

    tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	memset(temp, 0x00, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	memcpy((char *)&myPs->config.ipAddr[0], (char *)&temp[0],
		sizeof(char)*16);
		
    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.sendPort = atoi(buf);

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.receivePort = atoi(buf);

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.networkPort = atoi(buf);

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.replyTimeout = atoi(buf);

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.retryCount = atoi(buf);

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.netTimeout = atoi(buf);

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.pingTimeout = atoi(buf);

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.CmdSendLog = (unsigned char)atoi(buf);

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.CmdRcvLog = (unsigned char)atoi(buf);

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.CmdSendLog_Hex = (unsigned char)atoi(buf);

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.CmdRcvLog_Hex = (unsigned char)atoi(buf);

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.CommCheckLog = (unsigned char)atoi(buf);

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.send_monitor_data_interval = (unsigned long)atol(buf);

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.send_save_data_interval = (unsigned long)atol(buf);

	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myPs->config.protocol_version = (unsigned int)atoi(buf);
    	
	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	//1:pause - network fail, 2:run - network fail
	myPs->config.state_change = (unsigned int)atoi(buf);

	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myPs->config.crc_type = (unsigned char)atoi(buf);
    	
    fclose(fp);
	return 0;
}

void StateChange_Pause(int num)
{
	int bd, ch;
	int idx,sendNum;

	if(myPs->config.state_change == 1) {
		myPs->signal[EXT_SIG_NET_CONNECTED] = P0;
		for(bd=0; bd < myData->mData.config.installedBd; bd++) {
			for(ch=0; ch < myData->mData.config.installedCh; ch++) {
				if(myData->bData[bd].cData[ch].op.state == C_RUN) {
					myData->bData[bd].cData[ch].signal[C_SIG_NETWORK_CONNECT_ERROR] = P1;
				//	myData->bData[bd].cData[ch].signal[C_SIG_PAUSE] = P1;
					idx = myData->save_msg[0].read_idx[bd][ch];
					if(myData->save_msg[0].val[idx][bd][ch].chData.resultIndex < MAX_RETRANS_DATA) {
						sendNum = myData->save_msg[0].
							val[idx][bd][ch].chData.resultIndex;
					} else { 
						sendNum = MAX_RETRANS_DATA;
					}
					if(idx < sendNum) {
						myData->save_msg[0].read_idx[bd][ch] += 
								(MAX_SAVE_MSG - MAX_RETRANS_DATA);
						myData->save_msg[0].count[bd][ch] += sendNum;

					} else {
						myData->save_msg[0].read_idx[bd][ch] -= sendNum;
						myData->save_msg[0].count[bd][ch] += sendNum;

					}
				}
			}
		}
	} else if(myPs->config.state_change == 2) {
//110402 kji w
	//	send_msg(EXT_TO_MODULE, MSG_EXT_MODULE_SAVE_MSG_FLAG, 0, 1); //stop
	}

	close(myPs->config.network_socket);
	userlog(DEBUG_LOG, psName, "network communication error : %d\n", num);
}

char get_ch_state(int bd, int ch, int state)
{
	char	ret;

	switch(state){
		case C_IDLE:
			ret = '1';
			break;
		case C_STANDBY:
			if(myData->bData[bd].cData[ch].op.type == STEP_END){
				if(myData->bData[bd].cData[ch].ChAttribute.opType == P1){
					if(myPs->misc.rcvRecipeFlag[bd][ch] == P1){
						ret = '3';//ready
					}else{
						ret = '7';//end
					}
				}else{
					if(myData->bData[bd].cData[ch-1].ChAttribute.opType == P1){
						ret = '8';//parallel
					}else{
						if(myPs->misc.rcvRecipeFlag[bd][ch] == P1){
							ret = '3';//ready
						}else{
							ret = '7';//end
						}
					}
				}
			}else{
				if(myData->bData[bd].cData[ch].ChAttribute.opType == P1){
					if(myPs->misc.rcvRecipeFlag[bd][ch] == P1){
						ret = '3';//ready
					}else{
						if(myData->bData[bd].cData[ch].op.code == C_FAULT_STOP_CMD){
							ret = '6';//stop
						}else{
							ret = '8';//parallel
						}
					}
				}else{
					if(myPs->misc.rcvRecipeFlag[bd][ch] == P1){
						if(myData->bData[bd].cData[ch-1].ChAttribute.opType == P1){
							ret = '8';//parallel
						}else{
							ret = '3';//ready
						}
					}else{
						if(myData->bData[bd].cData[ch].op.code == C_FAULT_STOP_CMD){
							if(myData->bData[bd].cData[ch-1].ChAttribute.opType == P1){
								ret = '8';//parallel
							}else{
								ret = '6';//stop
							}
						}else{
							if(myData->bData[bd].cData[ch-1].ChAttribute.opType == P1){
								ret = '8';//parallel
							}else{
								ret = '2'; //standby
							}
						}
					}
				}
			}
			break;
		case C_RUN:
			if(myData->bData[bd].cData[ch].ChAttribute.opType == P1){
				ret = '4';//run
			}else{
				if(myData->bData[bd].cData[ch-1].ChAttribute.opType == P1){
					ret = '8';//parallel
				}else{
					ret = '4';//run
				}
			}
			break;
		case C_PAUSE:
			if(myData->bData[bd].cData[ch].ChAttribute.opType == P1){
				ret = '5';//pasue
			}else{
				if(myData->bData[bd].cData[ch-1].ChAttribute.opType == P1){
					ret = '8';//parallel
				}else{
					ret = '5';//pause
				}
			}
			break;
		default:
			ret = '0';
			break;
	}

	return ret;
}


char get_ch_stepType(int bdno, int chno, int type)
{
	char	ret;

	switch(type){
		case STEP_ADV_CYCLE:
			ret = '1';
			break;
		case STEP_CHARGE:
			ret = '2';
			break;
		case STEP_DISCHARGE:
			ret = '3';
			break;
		case STEP_REST:
			ret = '4';
			break;
		case STEP_OCV:
			ret = '5';
			break;
		case STEP_Z:
			ret = '6';
			break;
		case STEP_LOOP:
			ret = '7';
			break;
		case STEP_END:
			ret = '8';
			break;
		default:
			ret = '0';
			break;
	}

	return ret;
}

char convert_ch_stepType(int bdno, int chno, int type)
{
	char	ret;

	switch(type){
		case 1:
			ret = STEP_ADV_CYCLE;
			break;
		case 2:
			ret = STEP_CHARGE;
			break;
		case 3:
			ret = STEP_DISCHARGE;
			break;
		case 4:
			ret = STEP_REST;
			break;
		case 5:
			ret = STEP_OCV;
			break;
		case 6:
			ret = STEP_Z;
			break;
		case 7:
			ret = STEP_LOOP;
			break;
		case 8:
			ret = STEP_END;
			break;
		default:
			ret = STEP_IDLE;
			break;
	}

	return ret;
}



char get_ch_mode(int bdno, int chno, int mode)
{
	char	ret;

	switch(mode){
		case CCCV:
			ret = '1';
			break;
		case CC:
			ret = '2';
			break;
		case CV:
			ret = '3';
			break;
		case CP:
			ret = '4';
			break;
		case CPCV:
			ret = '5';
			break;
		case DC:
			ret = '6';
			break;
		default:
			ret = '0';
			break;
	}

	return ret;
}

char convert_ch_mode(int bdno, int chno, int mode)
{
	char	ret;

	switch(mode){
		case 1:
			ret = CCCV;
			break;
		case 2:
			ret = CC;
			break;
		case 3:
			ret = CV;
			break;
		case 4:
			ret = CP;
			break;
		case 5:
			ret = CPCV;
			break;
		case 6:
			ret = DC;
			break;
		default:
			ret = 0;
			break;
	}

	return ret;
}



int get_ch_code(int bdno, int chno, int code)
{
	int ret; 
//channel code
	switch(code){
		case C_END_TIME:
			ret = 101;
			break;
		case C_END_VOLTAGE:
			ret = 102;
			break;
		case C_END_CURRENT:
			ret = 103;
			break;
		case C_END_CAPACITY:
			ret = 104;
			break;
		case C_END_OCV:
			ret = 109;
			break;
/*C_END_STEP					
C_END_STOP_CMD				
C_END_PAUSE_CMD				
C_END_CHECK					
C_END_NEXTSTEP_CMD			
C_END_Z					
C_END_ADV_CYCLE				
C_END_LOOP					
C_END_DELTA_V				
C_END_SOC					
C_END_TEMP					

C_STOP_CODE_START			
C_STOP_TIME					
C_STOP_VOLTAGE				
C_STOP_CURRENT				
C_STOP_CAPACITY				
C_STOP_POWER				
C_STOP_WATTHOUR				
C_TEMP_WAIT_TIME			
C_END_SHORT				
*/
		case C_END_POWER:
			ret = 105;
			break;
		case C_END_WATTHOUR:
			ret = 106;
			break;
/*		
C_END_INTEGRAL_CAPACITY		
C_END_INTEGRAL_WATTHOUR		
C_END_CYCLE_TIME			
C_END_CVTIME			

C_FAULT_CHECK_CODE_START	 //check step fault code
C_FAULT_CHECK_UPPER_OCV		
C_FAULT_CHECK_LOWER_OCV	
C_FAULT_CHECK_UPPER_VOLTAGE	
C_FAULT_CHECK_LOWER_VOLTAGE	
C_FAULT_CHECK_UPPER_CURRENT	
C_FAULT_CHECK_LOWER_CURRENT	
C_FAULT_CHECK_CONTACT_BAD	
C_FAULT_CHECK_CONTACT_BAD2	
C_FAULT_CHECK_CONTACT_BAD3	
C_FAULT_CHECK_BAD_CELL		
C_FAULT_CHECK_BAD_CELL2		
C_FAULT_CHECK_BAD_CELL3		
C_FAULT_CHECK_SHORT			
C_FAULT_ERROR_NO			
C_FAULT_ERROR_YES			
*/

//C_FAULT_SOFT_CODE_START		//soft fault code
		case C_FAULT_UPPER_VOLTAGE:
			ret = 501;
			break;
		case C_FAULT_LOWER_VOLTAGE:
			ret = 502;
			break;
/*C_FAULT_UPPER_DELTA_VOLTAGE	
C_FAULT_LOWER_DELTA_VOLTAGE	
C_FAULT_UPPER_COMP_VOLTAGE1	
C_FAULT_LOWER_COMP_VOLTAGE1	
C_FAULT_UPPER_OCV			
C_FAULT_LOWER_OCV			
*/
		case C_FAULT_UPPER_CURRENT:
			ret = 503;
			break;
		case C_FAULT_LOWER_CURRENT:
			ret = 504;
			break;
/*		
C_FAULT_UPPER_DELTA_CURRENT	
C_FAULT_LOWER_DELTA_CURRENT	
C_FAULT_UPPER_COMP_CURRENT	
C_FAULT_LOWER_COMP_CURRENT	
*/
		case C_FAULT_UPPER_CAPACITY:
			ret = 505;
			break;
		case C_FAULT_LOWER_CAPACITY:
			ret = 506;
			break;
/*		
C_FAULT_UPPER_DELTA_CAPACITY	
C_FAULT_LOWER_DELTA_CAPACITY	
C_FAULT_UPPER_COMP_CAPACITY	
C_FAULT_LOWER_COMP_CAPACITY	
C_FAULT_UPPER_IMPEDANCE		
C_FAULT_LOWER_IMPEDANCE		
C_FAULT_UPPERTIME			
C_FAULT_LOWERTIME			
*/
		case C_FAULT_STOP_CMD:
			ret = 107;
			break;
		case C_FAULT_PAUSE_CMD:
			ret = 108;
			break;
/*		
C_FAULT_NEXTSTEP_CMD		
C_FAULT_UPPER_COMP_VOLTAGE2	
C_FAULT_LOWER_COMP_VOLTAGE2	
C_FAULT_UPPER_COMP_VOLTAGE3	
C_FAULT_LOWER_COMP_VOLTAGE3	
C_FAULT_UPPER_CC_TIME	
C_FAULT_WORK_ERROR			
C_FAULT_UPPER_COMP_CURRENT2	
C_FAULT_LOWER_COMP_CURRENT2	
C_FAULT_UPPER_COMP_CURRENT3	
C_FAULT_LOWER_COMP_CURRENT3	
C_FAULT_UPPER_CH_TEMP		
C_FAULT_LOWER_CH_TEMP		
C_FAULT_USER_PATTERN_READ	
C_FAULT_LIMIT_ERROR			
C_FAULT_CHAMBER_ERROR		
C_FAULT_INVERTER			
C_FAULT_TERMINAL_QUIT		
C_FAULT_COMPARE_LINE_PLUS 	
C_FAULT_COMPARE_LINE_MINUS 	
C_FAULT_COMPARE_LINE_FLT_CH	
C_FAULT_COMPARE_UPPER_VOLTAGE	
C_FAULT_COMPARE_UPPER_CURRENT	
C_FAULT_COMPARE_OVER_TEMP	
C_FAULT_COMPARE_METER_HIGH	
C_FAULT_COMPARE_METER_LOW	
C_FAULT_JIG_ERROR			
C_FAULT_TEMP_CONNECT_ERROR	
C_FAULT_NETWORK_CONNECT_ERROR	
C_FAULT_SHORT_TESTER_ERROR	
C_FAULT_SHORT_DISCONNECTED	
C_FAULT_CYCLER_SINGLE		
C_FAULT_SHORT_TESTER_SINGLE	
C_FAULT_INTER_LOCK_MODE
*/
//C_FAULT_HARD_CODE_START		//hard_fault
		case C_FAULT_CH_V_FAIL:
			ret = 601;
			break;
		case C_FAULT_CH_I_FAIL:
			ret = 602;
			break;
/*		
C_FAULT_OT					
C_FAULT_SMPS				
C_FAULT_FORCE_POWER			
C_FAULT_AC_POWER			
C_FAULT_UPS_BATTERY			
C_FAULT_MAIN_EMG			
C_FAULT_SUB_EMG				
C_FAULT_NETWORK_COMM		
C_FAULT_PANEL_HIGH			
C_FAULT_PANEL_LOW			

//aux Fault, End code
C_END_AUX_TEMP				
C_END_AUX_TEMP_UPPER		
C_END_AUX_TEMP_LOWER		
C_END_AUX_VOLTAGE			
C_END_AUX_VOLTAGE_UPPER		
C_END_AUX_VOLTAGE_LOWER		
C_FAULT_AUX_TEMP_UPPER		
C_FAULT_AUX_TEMP_LOWER		
C_FAULT_AUX_VOLTAGE_UPPER	
C_FAULT_AUX_VOLTAGE_LOWER	

//current userMap Temp end code
C_END_USERMAP_OVER_TEMP		

//151023 add for dc_fan_fail
C_FAULT_DC_FAN				

//160125 lyh add for rolling mode
C_FAULT_ROLLING				
C_END_ROLLING				

//151214 oys add : SEC Cycle Capacity Efficiency End
C_STOP_CAPACITY_SOC			
C_FAULT_CAPACITY_SOC	
*/
	default:
		ret = 0;
		break;
	}
	return ret;
}

/*
void Convert_testCond(int bd, int ch, int parallel, char *cmd)
{
	int i, rangeI, type, mode, rtn;
	long tmp, refV, refP, refI;
	unsigned char	buf[20];

	//testcond update not init
	memset((char *)&myData->mData.testCond[bd][ch], 0x00
					, sizeof(S_TEST_CONDITION));
	rtn = 0;
	for( i = 0; i < 4; i++){
		switch(i){
			case 0:
				type = STEP_ADV_CYCLE;
				mode = MODE_IDLE;
				break;
			case 1: //Step
				rtn = 1;
				break;
			case 2:
				type = STEP_LOOP;
				mode = MODE_IDLE;
				break;
			case 3:
				type = STEP_END;
				mode = MODE_IDLE;
				break;
			default:
				break;
		}
		myData->mData.testCond[bd][ch].step[i].type = type;
		myData->mData.testCond[bd][ch].step[i].stepNo = i;
		myData->mData.testCond[bd][ch].step[i].mode = mode;
		if(rtn == 1){
			memset(buf, 0x00, sizeof(buf));
			sprintf(buf, (char *)&cmd->step.stepType
					, sizeof(cmd->step.stepType));
			tmp = atol(buf); 
			myData->mData.testCond[bd][ch].step[i].type
				   = (char)convert_ch_stepType(bd, ch, (int)tmp);

			myData->mData.testCond[bd][ch].step[i].stepNo = i;

			memset(buf, 0x00, sizeof(buf));
			sprintf(buf, (char *)&cmd->step.stepMode
					, sizeof(cmd->step.stepMode));
			tmp = atol(buf); 
			myData->mData.testCond[bd][ch].step[i].mode
				   = (char)convert_ch_mode(bd, ch, (int)tmp);

			memset(buf, 0x00, sizeof(buf));
			sprintf(buf, (char *)&cmd->step.Vref, sizeof(cmd->step.Vref));
			refV = atol(buf) * 1000; 
			
			myData->mData.testCond[bd][ch].step[i].refV = refV;

			memset(buf, 0x00, sizeof(buf));
			sprintf(buf, (char *)&cmd->step.Pref, sizeof(cmd->step.Pref));
			refP = atol(buf); 

			memset(buf, 0x00, sizeof(buf));
			sprintf(buf, (char *)&cmd->step.Iref, sizeof(cmd->step.Iref));
			refI = atol(buf) * 1000; 
			if(parallel){
				if(myData->mData.testCond[bd][ch].step[i].mode == CP) {
					myData->mData.testCond[bd][ch].step[i].refP = 
						(long)refP / 2 ; 
				} else {
					refI =  (long)refI / 2;
				}
			}else{
				myData->mData.testCond[bd][ch].step[i].refP = refP;
				myData->mData.testCond[bd][ch].step[i].refI = refI;
			}
			if(refI <= myData->mData.config.maxCurrent[3]) {
				rangeI = RANGE4 - 1;
			} else if(refI <= myData->mData.config.maxCurrent[2]) {
				rangeI = RANGE3 - 1;
			} else if(refI <= myData->mData.config.maxCurrent[1]) {
				rangeI = RANGE2 - 1;
			} else {
				rangeI = RANGE1 - 1;
			}
			if((rangeI+1) > (int)myData->mData.config.rangeI) {
				rangeI = (int)myData->mData.config.rangeI - 1;
			}
			myData->mData.testCond[bd][ch].step[i].rangeI = (unsigned char)rangeI;

			memset(buf, 0x00, sizeof(buf));
			sprintf(buf, (char *)&cmd->step.end_time_day, sizeof(cmd->step.end_time_day));
			tmp = atol(buf) * 24 * 3600 * 100; 

			memset(buf, 0x00, sizeof(buf));
			sprintf(buf, (char *)&cmd->step.end_time_sec, sizeof(cmd->step.end_time_sec));
			tmp += atol(buf) * 100; 

			myData->mData.testCond[bd][ch].step[i].endT = tmp;

			memset(buf, 0x00, sizeof(buf));
			sprintf(buf, (char *)&cmd->step.endV, sizeof(cmd->step.endV));
			tmp = atol(buf) * 1000; 
			myData->mData.testCond[bd][ch].step[i].endV = tmp;

			memset(buf, 0x00, sizeof(buf));
			sprintf(buf, (char *)&cmd->step.endI, sizeof(cmd->step.endI));
			tmp = atol(buf) * 1000; 
			myData->mData.testCond[bd][ch].step[i].endI = tmp;

			memset(buf, 0x00, sizeof(buf));
			sprintf(buf, (char *)&cmd->step.endC, sizeof(cmd->step.endC));
			tmp = atol(buf)*1000; 
			myData->mData.testCond[bd][ch].step[i].endC = tmp;

			memset(buf, 0x00, sizeof(buf));
			sprintf(buf, (char *)&cmd->step.endW, sizeof(cmd->step.endW));
			tmp = atol(buf); 
			myData->mData.testCond[bd][ch].step[i].endP = tmp;

			memset(buf, 0x00, sizeof(buf));
			sprintf(buf, (char *)&cmd->step.endWh, sizeof(cmd->step.endWh));
			tmp = atol(buf); 
			myData->mData.testCond[bd][ch].step[i].endWh = tmp;

			memset(buf, 0x00, sizeof(buf));
			sprintf(buf, (char *)&cmd->step.highV, sizeof(cmd->step.highV));
			tmp = atol(buf) * 1000; 
			myData->mData.testCond[bd][ch].step[i].faultUpperV = tmp;

			memset(buf, 0x00, sizeof(buf));
			sprintf(buf, (char *)&cmd->step.lowV, sizeof(cmd->step.lowV));
			tmp = atol(buf) * 1000; 
			myData->mData.testCond[bd][ch].step[i].faultLowerV = tmp;

			memset(buf, 0x00, sizeof(buf));
			sprintf(buf, (char *)&cmd->step.highI, sizeof(cmd->step.highI));
			tmp = atol(buf) * 1000; 
			myData->mData.testCond[bd][ch].step[i].faultUpperI = tmp;
					
			memset(buf, 0x00, sizeof(buf));
			sprintf(buf, (char *)&cmd->step.lowI, sizeof(cmd->step.lowI));
			tmp = atol(buf) * 1000; 
			myData->mData.testCond[bd][ch].step[i].faultLowerI = tmp;

			memset(buf, 0x00, sizeof(buf));
			sprintf(buf, (char *)&cmd->step.highC, sizeof(cmd->step.highC));
			tmp = atol(buf)*1000; 
			myData->mData.testCond[bd][ch].step[i].faultUpperC = tmp;

			memset(buf, 0x00, sizeof(buf));
			sprintf(buf, (char *)&cmd->step.lowC, sizeof(cmd->step.lowC));
			tmp = atol(buf)*1000; 
			myData->mData.testCond[bd][ch].step[i].faultLowerC = tmp;

			memset(buf, 0x00, sizeof(buf));
			sprintf(buf, (char *)&cmd->step.recode_time_day, sizeof(cmd->step.recode_time_day));
			tmp = atol(buf) * 24 * 3600 * 100; 

			memset(buf, 0x00, sizeof(buf));
			sprintf(buf, (char *)&cmd->step.recode_time_sec, sizeof(cmd->step.recode_time_sec));
			tmp += atol(buf) * 100; 

			memset(buf, 0x00, sizeof(buf));
			sprintf(buf, (char *)&cmd->step.recode_time_mSec, sizeof(cmd->step.recode_time_mSec));
			tmp += atol(buf)/10;//10mS = 1

			myData->mData.testCond[bd][ch].step[i].saveDt = tmp;

			memset(buf, 0x00, sizeof(buf));
			sprintf(buf, (char *)&cmd->step.deltaV, sizeof(cmd->step.deltaV));
			tmp = atol(buf) * 1000; 
			myData->mData.testCond[bd][ch].step[i].saveDv = tmp;

			memset(buf, 0x00, sizeof(buf));
			sprintf(buf, (char *)&cmd->step.deltaI, sizeof(cmd->step.deltaI));
			tmp = atol(buf) * 1000; 
			myData->mData.testCond[bd][ch].step[i].saveDi = tmp;
		}
	}

	if(parallel){
		memcpy((char *)&myData->mData.testCond[bd][ch+1],
			(char *)&myData->mData.testCond[bd][ch], sizeof(S_TEST_CONDITION));
	}
}

*/
