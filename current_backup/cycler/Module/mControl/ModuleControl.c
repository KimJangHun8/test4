#include <rtl_core.h>
#include <asm/io.h>
#include "../../INC/datastore.h"
#include "message.h"
#include "ModuleControl.h"
#include "DInOutControl.h"
#include "CAN.h"

extern S_SYSTEM_DATA *myData;
extern S_MODULE_DATA *myPs;

void ModuleControl(void)
{
	int i, bd = 0, ch = 0, cnt;
	int chPause = 0,idx,sendNum;
	long long flag, compare1;
	char compare2, caliFlag = 0;
	unsigned long ac_fail_flag = 0; //201106_kjc
#ifdef _GROUP_ERROR
	unsigned long advStepNo;		//220223 LJS
#endif
	int standbyFlag = 0;			//220408
	
	AnalogMeter_Cali();	//hun_210824
	mSignalCheck();
	mainState_Connection_Check();

    switch(myPs->state) {
		case M_IDLE:
			myPs->state = M_STANDBY;
	    	myPs->phase = P0;
			myPs->signal[M_SIG_LAMP_RUN] = P0;
			myPs->signal[M_SIG_LAMP_STOP] = P1;
	    	break;
		case M_STANDBY:
			flag = 1;
			compare2 = 0;
			for(i=0; i < myPs->config.installedCh; i++) {
				bd = i / myPs->config.chPerBd;
				ch = i % myPs->config.chPerBd;
				//20101104 pjy
				//check channels condition			
				if(myData->bData[bd].cData[ch].op.state == C_PAUSE &&
				myData->bData[bd].cData[ch].op.code != C_PAUSE_UPPER_TEMP_CH &&
				myData->bData[bd].cData[ch].op.code != C_PAUSE_LOWER_TEMP_CH &&
				myData->bData[bd].cData[ch].op.code != C_FAULT_PAUSE_CMD &&
				myData->bData[bd].cData[ch].op.code != C_FAULT_NEXTSTEP_CMD &&
				myData->bData[bd].cData[ch].op.code != P_INV_STANDBY){
						compare1 = myPs->misc.buzzerFlag1 & flag;
						if(compare1 == 0){
							if(myData->mData.config.code_buzzer_on == 1){ //마곡 LGES 특정 코드만 Buzzer On 기능 추가
								if(myPs->code == M_FAIL_OVP || myData->bData[bd].cData[ch].op.code == C_FAULT_COMM_UPPER_VOLTAGE ||
								   myData->bData[bd].cData[ch].op.code == C_FAULT_COMM_LOWER_VOLTAGE ||
								   myData->bData[bd].cData[ch].op.code == C_FAULT_COMM_UPPER_OCV ||
								   myData->bData[bd].cData[ch].op.code == C_FAULT_COMM_LOWER_OCV ||
								   myData->bData[bd].cData[ch].op.code == C_FAULT_COMM_UPPER_CURRENT ||
								   myData->bData[bd].cData[ch].op.code == C_FAULT_COMM_LOWER_CURRENT ||
								   myData->bData[bd].cData[ch].op.code == C_FAULT_COMM_UPPER_CAPACITY ||
								   myData->bData[bd].cData[ch].op.code == C_FAULT_COMM_LOWER_CAPACITY){
									compare2 = 1;
								}else{
									compare2 = 0;
								}
							}else if(myData->mData.config.dyson_maintenance_flag == 1){ //DYSON maintenance 모드 시 Buzzer Off
							if(myPs->code == FAIL_NONE || myPs->code == M_FAIL_DOOR_OPEN1 
								|| myPs->code == M_FAIL_DOOR_OPEN2 || myPs->code == M_FAIL_DOOR_OPEN3){
									compare2 = 0;
								}else{
									compare2 = 1;
								}
							}
							else{
								compare2 = 1;
							}	
						}		
						chPause = 1;
				} else {
					myPs->misc.buzzerFlag1 &= ~flag;
				}
				if(myData->bData[bd].cData[ch].op.state == C_RUN) {
					myPs->state = M_RUN;
					myPs->phase = P0;
					//break;
				}
				if(myData->bData[bd].cData[ch].op.state == C_CALI) {
					caliFlag = 1;
				}
				flag = flag << 1;

				//201106_kjc
				if(myData->bData[bd].cData[ch].misc.ac_fail_flag == P1){
					ac_fail_flag ++;
				}
			}
			myPs->signal[M_SIG_LAMP_RUN] = P0;
			//190607 lyhw / 191118 rewrite
			if(myData->mData.config.function[F_CHANGE_VI_CHECK] != P1){ 
				myPs->signal[M_SIG_LAMP_STOP] = P1; //yellow
			}
			if(caliFlag && myData->mData.misc.timer_1sec%2)
				myPs->signal[M_SIG_LAMP_STOP] = P0;
			if(chPause == 1){
				if(ac_fail_flag != 0){	//201106_kjc
					myPs->signal[M_SIG_LAMP_PAUSE] = P0;
					myPs->signal[M_SIG_LAMP_BUZZER] = P0;
				}else{
					myPs->signal[M_SIG_LAMP_PAUSE] = P1;
					if(compare2){
						myPs->signal[M_SIG_LAMP_BUZZER] = P1;
					}else{
						myPs->signal[M_SIG_LAMP_BUZZER] = P0;
					}
				}
			}else{
				myPs->misc.buzzerFlag1 = 0;
				myPs->signal[M_SIG_LAMP_PAUSE] = P0;
				myPs->signal[M_SIG_LAMP_BUZZER] = P0;
				//190607 lyhw
				if(myData->mData.config.function[F_CHANGE_VI_CHECK] == P1){ 
					myPs->signal[M_SIG_LAMP_STOP] = P1; //yellow
				}
				//200818 HYUNDAI yellow off
				if(myData->MainClient.signal[MAIN_SIG_LAMP_BUZZER] == P2){
					myPs->signal[M_SIG_LAMP_STOP] = P0;
				}
			}
	   		break;
		case M_RUN:
	    	cnt = 0;
			chPause = 0;
			flag = 1;
			compare2 = 0;
			for(i=0; i < myPs->config.installedCh; i++) {
				bd = i / myPs->config.chPerBd;
				ch = i % myPs->config.chPerBd;
				//170105 oys add : ch step sync process
				if(myData->bData[bd].cData[ch].misc.stepSyncFlag == P1){
					chStepSync(bd, ch);
					#ifdef _GROUP_ERROR
					groupErrorCheck_EndTime(bd, ch);
					advStepNo = myData->bData[bd].cData[ch].misc.advStepNo;
					if(myPs->testCond[bd][ch].step[advStepNo].group_StartVoltage != 0 && myData->bData[bd].cData[ch].op.phase == P50){
						groupErrorCheck(bd, ch);
					}
					#endif
				}
				if(myData->bData[bd].cData[ch].op.state != C_RUN) cnt++;
				//20101104 pjy
				//check channels condition			
				if(myData->bData[bd].cData[ch].op.state == C_PAUSE &&
				myData->bData[bd].cData[ch].op.code != C_PAUSE_UPPER_TEMP_CH &&
				myData->bData[bd].cData[ch].op.code != C_PAUSE_LOWER_TEMP_CH &&
				myData->bData[bd].cData[ch].op.code != C_FAULT_PAUSE_CMD &&
				myData->bData[bd].cData[ch].op.code != C_FAULT_NEXTSTEP_CMD &&
				myData->bData[bd].cData[ch].op.code != P_INV_STANDBY){
					compare1 = myPs->misc.buzzerFlag1 & flag;
					if(compare1 == 0){
						if(myData->mData.config.code_buzzer_on == 1){ //마곡 LGES 특정 코드만 Buzzer On 기능 추가
							if(myPs->code == M_FAIL_OVP || myData->bData[bd].cData[ch].op.code == C_FAULT_COMM_UPPER_VOLTAGE ||
							   myData->bData[bd].cData[ch].op.code == C_FAULT_COMM_LOWER_VOLTAGE ||
							   myData->bData[bd].cData[ch].op.code == C_FAULT_COMM_UPPER_OCV ||
							   myData->bData[bd].cData[ch].op.code == C_FAULT_COMM_LOWER_OCV ||
							   myData->bData[bd].cData[ch].op.code == C_FAULT_COMM_UPPER_CURRENT ||
							   myData->bData[bd].cData[ch].op.code == C_FAULT_COMM_LOWER_CURRENT ||
							   myData->bData[bd].cData[ch].op.code == C_FAULT_COMM_UPPER_CAPACITY ||
							   myData->bData[bd].cData[ch].op.code == C_FAULT_COMM_LOWER_CAPACITY){
								compare2 = 1;
							}else{
								compare2 = 0;
							}
						}else if(myData->mData.config.dyson_maintenance_flag == 1){ //DYSON maintenance 모드 시 Buzzer Off
							if(myPs->code == FAIL_NONE || myPs->code == M_FAIL_DOOR_OPEN1 
								|| myPs->code == M_FAIL_DOOR_OPEN2 || myPs->code == M_FAIL_DOOR_OPEN3){
								compare2 = 0;
							}else{
								compare2 = 1;
							}
						}
						else{
							compare2 = 1;
						}
					}	
					chPause = 1;
				} else {
					myPs->misc.buzzerFlag1 &= ~flag;
				}	
				
				if(myData->bData[bd].cData[ch].op.state == C_RUN) {
					myPs->state = M_RUN;
					myPs->phase = P0;
					//break;
				}
				if(myData->bData[bd].cData[ch].op.state == C_CALI) {
					caliFlag = 1;
				}
				flag = flag << 1;
			}
			if(cnt >= myPs->config.installedCh) {
				myPs->state = M_STANDBY;
				myPs->phase = P0;
			}
			
			//190607 lyhw 
			if(myData->mData.config.function[F_CHANGE_VI_CHECK] != P1){ //20191107
				myPs->signal[M_SIG_LAMP_RUN] = P1;
			}
			myPs->signal[M_SIG_LAMP_STOP] = P0;
			if(caliFlag && myData->mData.misc.timer_1sec%2)
				myPs->signal[M_SIG_LAMP_STOP] = P1;
			//if channel condition is 
			//C_PAUSE  step, the Module's tower lamp will be 
			//turn on the red lamp

			if(chPause == 1){
				//190607 lyhw /191118 rewrite 
				if(myData->mData.config.function[F_CHANGE_VI_CHECK] == P1){ 
					myPs->signal[M_SIG_LAMP_RUN] = P0;
				}
				myPs->signal[M_SIG_LAMP_PAUSE] = P1;
				if(compare2){
					myPs->signal[M_SIG_LAMP_BUZZER] = P1;
				}else{
					myPs->signal[M_SIG_LAMP_BUZZER] = P0;
				}
			}else{
				myPs->misc.buzzerFlag1 = 0;
				myPs->signal[M_SIG_LAMP_PAUSE] = P0;
				myPs->signal[M_SIG_LAMP_BUZZER] = P0;
				//190607 lyhw 
				if(myData->mData.config.function[F_CHANGE_VI_CHECK] == P1){ 
					myPs->signal[M_SIG_LAMP_RUN] = P1;
				}
			}
					
			cnt = (int)(myPs->misc.timer_1sec - myData->MainClient.netTimer);
			if(cnt >= myData->MainClient.config.netTimeout
				&& myPs->signal[M_SIG_NET_CHECK] == P0) {
				myPs->signal[M_SIG_NET_CHECK] = P1;
				myData->MainClient.signal[MAIN_SIG_NET_CONNECTED] = P0;
				if(myData->MainClient.config.state_change == 1) {
					for(i=0; i < myPs->config.installedCh; i++) {
						bd = i / myPs->config.chPerBd;
						ch = i % myPs->config.chPerBd;
						if(myData->bData[bd].cData[ch].op.state == C_RUN) {
					//		myData->bData[bd].cData[ch].signal[C_SIG_PAUSE]
					//			= P1;
							myData->bData[bd].cData[ch].
								signal[C_SIG_NETWORK_CONNECT_ERROR] = P1;
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
				} else if(myData->MainClient.config.state_change == 2) {
					myData->save_msg[0].flag = 1; //send save_msg stop
					send_msg(MODULE_TO_APP, MSG_MODULE_APP_PROCESS_KILL, 0, 0);
				}
			}
	    	break;
		default: break;
    }
	//220408 hun
    if(myPs->state == M_STANDBY) {
		for(i=0; i < myPs->config.installedCh; i++) {
			bd = i / myPs->config.chPerBd;
			ch = i % myPs->config.chPerBd;
			if(myData->bData[bd].cData[ch].op.state != C_STANDBY){
				standbyFlag++;
			}
		}	
		if(standbyFlag == 0){
			myPs->config.Auto_Update = 1;
		}else{
			myPs->config.Auto_Update = 0;
		}
	}
}

void mSignalCheck(void)
{
	int p_ch, bd, ch;

	if(myPs->signal[M_SIG_TEST_SEND_SAVE_DATA] != P0) { //khh test
		p_ch = (int)myPs->signal[M_SIG_TEST_SEND_SAVE_DATA] - 1;
		bd = myData->CellArray1[p_ch].bd;
		ch = myData->CellArray1[p_ch].ch;
		send_save_msg_2(bd, ch, 0, 0, 1);
	}
	
	switch(myPs->config.hwSpec){
		case L_5V_100A:
			PS_ON_1();
			break;
		case L_5V_200A_R2:
		case L_5V_30A_R1:
		case L_5V_100A_R1:
		case L_5V_100A_R1_EG:
		case L_5V_2A_R1:
		case L_5V_500A_R1:
		case L_5V_500A_R2:
		case L_5V_200A_R4:
		case L_5V_200A_R3:
		case L_5V_150A_R3_AD2:
		case L_5V_200A_R3_P:
		case L_5V_200A_R3_P_AD2:
		case L_5V_400A_R3:
		case L_3V_200A_R2:
		case L_16V_200A_R2:
		case L_5V_250A_R2:
		case L_20V_300A_R2_1:
		case L_5V_220A_R2:
		case L_20V_110A_R2:
		case L_20V_50A_R2_1:
		case L_5V_100A_R2_1:
		case L_5V_120A_R3:
		case L_20V_300A_R2:
		case L_40V_300A_R2:
		case L_30V_40A_R2:
		case L_30V_40A_R2_OT_20:
		case L_30V_40A_R2_P_AD2:
		case L_5V_150A_R3:
		case L_5V_150A_R1:
		case L_5V_250A_R1:
		case L_5V_50A_R1:
		case L_5V_1000A_R1:
		case L_5V_1000A_R3:
		case L_5V_300A_R1:
		case L_5V_300A_R3:
		case L_10V_500A_R2:
		case L_20V_10A_R1:
		case L_20V_5A_R1:
		case L_20V_50A_R2:
		case L_10V_50A_R2:
		case L_5V_20A_R3:
		case L_5V_10A_R3:
		case L_6V_60A_R2:
		case L_6V_60A_R2_P:
		case L_5V_600A_10A: //Pack Hardware
		case L_30V_20A_R1_AD2:	//LG innotek
		case L_30V_5A_R1_AD2:	
		case L_60V_100A_R1_AD2:	//LS Mtron
		case L_15V_100A_R3_AD2:
		case L_5V_60A_R2_1:
		case L_5V_150A_R2_P:
		case L_5V_200A_1CH_JIG:
		case L_MAIN_REV11:
		case L_8CH_MAIN_AD2_P:
		case L_5V_20A_R3_NEW:
		case L_5V_500A_R3_1:
		case L_5V_30A_R3_HYUNDAI:
		case L_20V_6A_R3: 	//210813 ljsw LGES 20V6A
			PS_ON_2();
			GUI_TO_SBC_SHUTDOWN(); 		 //181108
			GUI_TO_SBC_BUZZER_CONTROL(); //190225 
			break;
		case S_5V_200A_75A_15A_AD2:
			PS_ON_3();
			break;
		case L_MULTI:
			PS_ON_4();
			GUI_TO_SBC_SHUTDOWN(); //190121
			GUI_TO_SBC_BUZZER_CONTROL(); //190225
			break;
		case DC_5V_150A_PARA:		//180611 add for digital
			mInv_Signal();
			break;
		case DC_5V_CYCLER_NEW:		//180611 add for digital
			PCU_INV_Signal();
			GUI_cmd_Buzzer_stop();
			break;
		case L_5V_10A_R3_NEW: 		//191118
		case L_5V_1A_R3:	 		//191118
		case L_5V_500mA_2uA_R4:		//210510 lyhw
			GUI_TO_SBC_SHUTDOWN(); 
			GUI_TO_SBC_BUZZER_CONTROL(); 
			break;
		case C_5V_CYCLER_CAN:
			Inverter_Signal_CAN();
			break;
		default:
			break;
	}

	Fault_Check();
	Ch_Code_Check();	//220322_hun
}

void PS_ON_1(void)
{
	int addr, base_addr, hw_wr, ps_rem_cs;

	base_addr = myPs->addr.main[BASE_ADDR];
	hw_wr = myPs->addr.main[HW_WR];
	ps_rem_cs = myPs->addr.main[PS_REM_CS];
	addr = base_addr + ps_rem_cs;

	if(myPs->signal[M_SIG_REMOTE_SMPS1] == P1) {
		outb(0xFF, addr);
	} else {
		outb(0x00, addr);
	}
}

void PS_ON_2(void)
{
	int addr1, addr2;

	addr1 = myPs->addr.interface[IO_EXPEND][0];
	addr2 = myPs->addr.interface[IO_EXPEND][1];

	if(myPs->signal[M_SIG_REMOTE_SMPS1] == P1) {
		outb(0xFF, addr1);
		outb(0xFF, addr2);
	} else {
		outb(0x00, addr1);
		outb(0x00, addr2);
	}
}

void PS_ON_3(void)
{
	unsigned char flag1, flag2;
	int rd_addr1, rd_addr2, wr_addr1;

	rd_addr1 = myPs->addr.interface[IO_EXPEND][6];
	rd_addr2 = myPs->addr.interface[IO_EXPEND][7];
	wr_addr1 = myPs->addr.interface[IO_EXPEND][8];

	//25ms
	if(myPs->signal[M_SIG_REMOTE_SMPS1] == P1) {
		switch(myPs->signal[M_SIG_REMOTE_SMPS_DELAY]) {
			/*case P0:
				outb(0x55, wr_addr1);
				break;
			case P2:
				outb(0x00, wr_addr1);
				break;*/
			case P4:
				flag1 = inb(rd_addr1);
				flag2 = inb(rd_addr2);
//				if(flag1 == 0 && flag2 == 0) {
				if(1){
					outb(0x02, wr_addr1);
					myPs->signal[M_SIG_REMOTE_SMPS_DELAY]++;
				} else {
					myPs->signal[M_SIG_REMOTE_SMPS_DELAY] = P100;
				}
				break;
			case P8:
				flag1 = inb(rd_addr1);
				flag2 = inb(rd_addr2);
//				if(flag1 == 0 && flag2 == 0) {
				if(1){
					outb(0x0A, wr_addr1);
					myPs->signal[M_SIG_REMOTE_SMPS_DELAY]++;
				} else {
					myPs->signal[M_SIG_REMOTE_SMPS_DELAY] = P100;
				}
				break;
			case P12:
				flag1 = inb(rd_addr1);
				flag2 = inb(rd_addr2);
//				if(flag1 == 0 && flag2 == 0) {
				if(1){
					outb(0x2A, wr_addr1);
					myPs->signal[M_SIG_REMOTE_SMPS_DELAY]++;
				} else {
					myPs->signal[M_SIG_REMOTE_SMPS_DELAY] = P100;
				}
				break;
			case P16:
				flag1 = inb(rd_addr1);
				flag2 = inb(rd_addr2);
//				if(flag1 == 0 && flag2 == 0) {
				if(1){
					outb(0xAA, wr_addr1);
					myPs->signal[M_SIG_REMOTE_SMPS_DELAY]++;
				} else {
					myPs->signal[M_SIG_REMOTE_SMPS_DELAY] = P100;
				}
				break;
//			case P50:
//				break;
			case P100:
				break;
			default:
				myPs->signal[M_SIG_REMOTE_SMPS_DELAY]++;
				break;
		}
	} else {
		myPs->signal[M_SIG_REMOTE_SMPS_DELAY] = P0;
		outb(0x55, wr_addr1);
	}
}
//pms add for multi bd
void PS_ON_4(void)
{
	int addr, base_addr, run_cs;
	base_addr = myPs->addr.main[BASE_ADDR];	//base addr 0x620
	run_cs = myPs->addr.main[RUN_CS];		//run cs 0xC
	addr = base_addr + run_cs;								// 0x62C

	//SMPS, FAN on
	if(myPs->signal[M_SIG_REMOTE_SMPS1] == P1){
		myPs->misc.outFlag |= 0xF0;
		outb(myPs->misc.outFlag, addr);
	//SMPS, FAN off
	}else{
		myPs->misc.outFlag &= ~0xF0;
		outb(myPs->misc.outFlag, addr);
	}
}//end of add

void GUI_TO_SBC_SHUTDOWN(void) //220209
{
	if(myPs->signal[M_SIG_GUI_TO_SBC_SHUTDOWN] == P1) {
#if CYCLER_TYPE == DIGITAL_CYC
		myPs->signal[M_SIG_INV_POWER] = P100;
		myPs->signal[M_SIG_INV_POWER1] = P100;
#endif
		if(myData->AppControl.config.systemType == CYCLER_CAN){ 
			myPs->signal[M_SIG_INV_POWER_CAN] = P100;
		}
		myPs->signal[M_SIG_GUI_TO_SBC_SHUTDOWN] = P0;
		myPs->signal[M_SIG_RUN_LED] = P0;
		myPs->signal[M_SIG_REMOTE_SMPS1] = P0;
		myPs->signal[M_SIG_FAN_RELAY] = P0;
		myPs->signal[M_SIG_POWER_OFF] = P1;
		send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 29, 0);
	}
		
}

void Fault_Check(void)
{
	if(myData->dio.config.dio_Control_Flag == P0) return;
	
	switch(myPs->config.hwSpec){
/*		case L_5V_100A:
			Fault_Check_1();
			break;
		case L_5V_200A_R2:
			Fault_Check_2();
			break;
		case L_5V_100A_R1:
			Fault_Check_3();
			break;
		case L_5V_500A_R1:
			Fault_Check_4();
			break;
		case L_5V_200A_R4:
		case L_5V_100A_R1_EG:
			Fault_Check_5();
			break;
		case L_5V_150A_R1:
			Fault_Check_6();
			break;
*/	
		case L_5V_100A:
		case L_5V_200A_R2:
		case L_5V_200A_R4:
		case L_5V_200A_R3:
		case L_5V_400A_R3:
		case L_3V_200A_R2:
		case L_16V_200A_R2:
		case L_5V_220A_R2:
		case L_20V_110A_R2:
		case L_20V_50A_R2_1:
		case L_5V_120A_R3:
		case L_20V_300A_R2:
		case L_40V_300A_R2:
		case L_30V_40A_R2:
		case L_30V_40A_R2_P_AD2:
		case L_5V_100A_R2_1:
		case L_5V_150A_R3:
		case L_5V_100A_R1_EG:
		case L_5V_150A_R1:
		case L_5V_250A_R1:
		case L_5V_50A_R1:
		case L_5V_500A_R1:
		case L_5V_500A_R2:
		case L_5V_1000A_R1:
		case L_5V_1000A_R3:
		case L_5V_300A_R1:
		case L_5V_300A_R3:
		case L_10V_500A_R2:
		case L_5V_10_30A_R2:
		case L_20V_10A_R1:
		case L_20V_5A_R1:
		case L_20V_50A_R2:
		case L_10V_50A_R2:
		case L_5V_200A_R3_P:
		case L_5V_250A_R2:
		case L_20V_300A_R2_1:
		case L_5V_150A_R3_AD2:
		case L_5V_200A_R3_P_AD2:
		case L_30V_20A_R1_AD2:
		case L_30V_5A_R1_AD2:	
		case L_60V_100A_R1_AD2:	//LS Mtron
		case L_15V_100A_R3_AD2:
		case L_5V_60A_R2_1:
		case L_5V_10A_R3:
		case L_6V_60A_R2:
		case L_5V_200A_1CH_JIG:
		case L_MULTI:
		case L_8CH_MAIN_AD2_P:
		case L_5V_20A_R3_NEW:
		case L_5V_500A_R3_1:
		case L_5V_30A_R3_HYUNDAI:
		case L_20V_6A_R3: 	//210813 ljsw LGES 20V6A
			Fault_Check_11();
//			Fault_Check_default_LG();
			break;
		case S_5V_200A_75A_15A_AD2:
			Fault_Check_9();
			break;
		case DC_5V_150A_PARA:		//180611 add for digital
			Fault_Check_13();
			break;
		case L_5V_100A_R1:
			Fault_Check_10();
//			Fault_Check_default_LG();
			break;
		case L_5V_150A_R2_P:
			Fault_Check_11();
			Fault_Check_12();
//			Fault_Check_default_LG();
			break;
		case L_30V_40A_R2_OT_20:
			Fault_Check_11();
			Fault_Check_11_OT_20();
			break;
		case C_5V_CYCLER_CAN:
			Fault_Check_CAN_1();
			break;
		default:
//			Fault_Check_default_LG();
			break;
	}
}

//190621 oys add start : CAN COMM ERROR -> CHANNEL SIGNAL CHANGE
void Fault_Check_CAN_1(void)
{
	int bd, ch, inv, start, end, i;
	int canCommErrCnt;
	
	if(myData->AppControl.config.debugType != P0) return;
	if(myData->CAN.config.canUseFlag == P0) return;

	canCommErrCnt = 0;
	//1.MAIN BOARD CAN COMMUNICATION ERROR
	for(bd=0; bd < myPs->config.installedBd; bd++) {
		if(myData->bData[bd].signal[B_SIG_MAIN_CAN_COMM_ERROR] == P1) {
			//220113 lyhw
			canCommErrCnt++;
			for(ch=0; ch < myPs->config.chPerBd; ch++) {
				if((myPs->config.chPerBd * bd + ch)
					> (myPs->config.installedCh-1)) {
					continue;
				}

				if(myData->bData[bd].cData[ch].op.state == C_RUN
					&& myData->bData[bd].cData[ch]
					.signal[C_SIG_MAIN_CAN_COMM_ERROR] == P0) {
					myData->bData[bd].cData[ch]
						.signal[C_SIG_MAIN_CAN_COMM_ERROR] = P1;
				}
			}
		}
	}
	//1.1 Main Board can comm error shutdown sequence	
	
	if(canCommErrCnt == myPs->config.installedBd 
		&& myPs->signal[M_SIG_INV_POWER_CAN] == P99){
		if(myPs->signal[M_SIG_MAIN_CAN_ERR_TOTAL] == P0){
			myPs->signal[M_SIG_MAIN_CAN_ERR_TOTAL] = P1;
			myPs->signal[M_SIG_RUN_LED] = P0;
			myPs->signal[M_SIG_REMOTE_SMPS1] = P0;
			myPs->signal[M_SIG_FAN_RELAY] = P0;
			myPs->signal[M_SIG_POWER_OFF] = P1;
			send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 36, 0);
			send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
				M_FAIL_MAIN_CAN_ERR_TOTAL, 0);
			if(myPs->signal[M_SIG_INV_POWER_CAN] == P10){
				myPs->signal[M_SIG_INV_POWER_CAN] = P100;
			}
		}
	}

	//2.INVERTER CAN COMMUNICATION ERROR
	for(inv=0;inv < myData->CAN.config.installedInverter; inv++) {
		if(myData->CAN.inverter[inv].signal[CAN_SIG_INV_CAN_COMM_ERROR] == P1) {
			if(inv == 0) {
				start = 0;
				end = myData->CAN.config.chInInv[inv];
			} else {
				start = myData->CAN.config.chInInv[inv-1] * inv;
				end = myData->CAN.config.chInInv[inv] + start;
			}
			for(i = start; i < end; i++) {
				bd = i / myPs->config.chPerBd;
				ch = i % myPs->config.chPerBd;
				if(myData->bData[bd].cData[ch].op.state == C_RUN
					&& myData->bData[bd].cData[ch]
					.signal[C_SIG_INV_CAN_COMM_ERROR] == P0) {
					myData->bData[bd].cData[ch]
						.signal[C_SIG_INV_CAN_COMM_ERROR] = P1;
				}
			}
		}
	}
	//3.IO BOARD CAN COMMUNICATION ERROR
}

void Fault_Check_1(void)
{
	int bd, ch, rtn;
	int	addr, base_addr, hw_rd;
	unsigned char flag;

	if(myData->dio.delayTimer < myData->dio.config.dioDelay) return;
	if(myPs->signal[M_SIG_FAN_RELAY] == P0) return;

	base_addr = myPs->addr.interface[IO_EXPEND][1];
	addr = base_addr;
	flag = inb(addr);

	rtn = 0;
	if((flag & 0x0F) != 0x0F) rtn = 1; //OT fault
	if(rtn != 0) {
		if(myPs->signal[M_SIG_OT_FAULT] == P0) {
			myPs->signal[M_SIG_OT_FAULT] = P1;
			myPs->signal[M_SIG_RUN_LED] = P0;
			myPs->signal[M_SIG_REMOTE_SMPS1] = P0;
			myPs->signal[M_SIG_FAN_RELAY] = P0;
			myPs->signal[M_SIG_POWER_OFF] = P1;
			myPs->code = M_FAIL_OT;
			send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 6, flag);
			send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
				(int)myPs->code, rtn);
			for(bd=0; bd < myPs->config.installedBd; bd++) {
				for(ch=0; ch < myPs->config.chPerBd; ch++) {
					if((myPs->config.chPerBd * bd + ch)
						> (myPs->config.installedCh-1)) {
						continue;
					}
					if(myData->bData[bd].cData[ch].op.state == C_RUN
						&& myData->bData[bd].cData[ch]
						.signal[C_SIG_OT_FAULT] == P0) {
						myData->bData[bd].cData[ch]
							.signal[C_SIG_OT_FAULT] = P1;
					}
				}
			}
		}
		return;
	}

	base_addr = myPs->addr.main[BASE_ADDR];
	hw_rd = myPs->addr.main[HW_RD];
	addr = base_addr + hw_rd;
	flag = inb(addr);

	rtn = 0;
	if((flag & 0x07) != 0x07) rtn = 1; //smps fault
	if(rtn != 0) {
		if(myPs->signal[M_SIG_SMPS_FAULT] == P0) {
			myPs->signal[M_SIG_SMPS_FAULT] = P1;
			myPs->signal[M_SIG_RUN_LED] = P0;
			myPs->signal[M_SIG_REMOTE_SMPS1] = P0;
			myPs->signal[M_SIG_FAN_RELAY] = P0;
			myPs->signal[M_SIG_POWER_OFF] = P1;
			myPs->code = M_FAIL_SMPS;
			send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 7, flag);
			send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
				(int)myPs->code, 0);
			for(bd=0; bd < myPs->config.installedBd; bd++) {
				for(ch=0; ch < myPs->config.chPerBd; ch++) {
					if((myPs->config.chPerBd * bd + ch)
						> (myPs->config.installedCh-1)) {
						continue;
					}
					if(myData->bData[bd].cData[ch].op.state == C_RUN
						&& myData->bData[bd].cData[ch]
						.signal[C_SIG_SMPS_FAULT] == P0) {
						myData->bData[bd].cData[ch]
							.signal[C_SIG_SMPS_FAULT] = P1;
					}
				}
			}
		}
	}
}

void Fault_Check_2(void)
{
	int bd, ch;
	int	addr, base_addr;
	unsigned short flag, bit;

	if(myData->dio.delayTimer < myData->dio.config.dioDelay) return;
	if(myPs->signal[M_SIG_FAN_RELAY] == P0) return;

	base_addr = myPs->addr.interface[IO_EXPEND][3];
	addr = base_addr;
	flag = inb(addr);

	if((flag & 0x1F) != 0x1F) { //OT fault
		if(myPs->signal[M_SIG_OT_FAULT] == P0) {
			myPs->signal[M_SIG_OT_FAULT] = P1;
			myPs->signal[M_SIG_RUN_LED] = P0;
			myPs->signal[M_SIG_REMOTE_SMPS1] = P0;
			myPs->signal[M_SIG_FAN_RELAY] = P0;
			myPs->signal[M_SIG_POWER_OFF] = P1;
			myPs->code = M_FAIL_OT;
			send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 6, flag);
			send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
				(int)myPs->code, flag);
			for(bd=0; bd < myPs->config.installedBd; bd++) {
				for(ch=0; ch < myPs->config.chPerBd; ch++) {
					if((myPs->config.chPerBd * bd + ch)
						>= myPs->config.installedCh) {
					continue;
					}
					if(myData->bData[bd].cData[ch].op.state == C_RUN
						&& myData->bData[bd].cData[ch]
						.signal[C_SIG_OT_FAULT] == P0) {
						myData->bData[bd].cData[ch]
							.signal[C_SIG_OT_FAULT] = P1;
					}
				}
			}
		}
		return;
	}

	base_addr = myPs->addr.interface[IO_EXPEND][2];
	addr = base_addr;
	flag = inb(addr);

	if((flag & 0x1F) != 0) {//smps fault 7.5V
		for(bd=0; bd < myPs->config.installedBd; bd++) {
			bit = 0x0001;
			for(ch=0; ch < myPs->config.chPerBd; ch++) {
				if((myPs->config.chPerBd * bd + ch)
					>= myPs->config.installedCh) {
					continue;
				}
				if((flag & (bit << ch)) != 0){
					if(myData->bData[bd].cData[ch].op.state == C_RUN){
						if(myData->bData[bd].cData[ch]
							.signal[C_SIG_SMPS_FAULT] == P0) {
							myData->bData[bd].cData[ch]
								.signal[C_SIG_SMPS_FAULT] = P1;
						}
					}
				}
			}
		}
	}

	base_addr = myPs->addr.interface[IO_EXPEND][4];
	addr = base_addr;
	flag = inb(addr);

	if((flag & 0x1F) != 0) { //smps fault 3.3V
		for(bd=0; bd < myPs->config.installedBd; bd++) {
			bit = 0x0001;
			for(ch=0; ch < myPs->config.chPerBd; ch++) {
				if((myPs->config.chPerBd * bd + ch)
					>= myPs->config.installedCh) {
					continue;
				}
				if((flag & (bit << ch)) != 0){
					if(myData->bData[bd].cData[ch].op.state == C_RUN){
						if(myData->bData[bd].cData[ch]
							.signal[C_SIG_SMPS_FAULT] == P0) {
							myData->bData[bd].cData[ch]
								.signal[C_SIG_SMPS_FAULT] = P1;
						}
					}
				}
			}
		}
	}
}

void Fault_Check_3(void)
{
	int bd, ch;
	int	addr, base_addr;
	unsigned short flag;

	if(myData->dio.delayTimer < myData->dio.config.dioDelay) return;
	if(myPs->signal[M_SIG_FAN_RELAY] == P0) return;

	base_addr = myPs->addr.interface[IO_EXPEND][3];
	addr = base_addr;
	flag = inb(addr);

	if((flag & 0x7F) != 0x7F) { //OT fault
		if(myPs->signal[M_SIG_OT_FAULT] == P0) {
			myPs->signal[M_SIG_OT_FAULT] = P1;
			myPs->signal[M_SIG_RUN_LED] = P0;
			myPs->signal[M_SIG_REMOTE_SMPS1] = P0;
			myPs->signal[M_SIG_FAN_RELAY] = P0;
			myPs->signal[M_SIG_POWER_OFF] = P1;
			myPs->code = M_FAIL_OT;
			send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 6, flag);
			send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
				(int)myPs->code, flag);
			for(bd=0; bd < myPs->config.installedBd; bd++) {
				for(ch=0; ch < myPs->config.chPerBd; ch++) {
					if((myPs->config.chPerBd * bd + ch)
						>= myPs->config.installedCh) {
					continue;
					}
					if(myData->bData[bd].cData[ch].op.state == C_RUN
						&& myData->bData[bd].cData[ch]
						.signal[C_SIG_OT_FAULT] == P0) {
						myData->bData[bd].cData[ch]
							.signal[C_SIG_OT_FAULT] = P1;
					}
				}
			}
		}
		return;
	}

	base_addr = myPs->addr.interface[IO_EXPEND][5];
	addr = base_addr;
	flag = inb(addr);

	if((flag & 0x7F) != 0x7F) { //OT fault
		if(myPs->signal[M_SIG_OT_FAULT] == P0) {
			myPs->signal[M_SIG_OT_FAULT] = P1;
			myPs->signal[M_SIG_RUN_LED] = P0;
			myPs->signal[M_SIG_REMOTE_SMPS1] = P0;
			myPs->signal[M_SIG_FAN_RELAY] = P0;
			myPs->signal[M_SIG_POWER_OFF] = P1;
			myPs->code = M_FAIL_OT;
			send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 6, flag);
			send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
				(int)myPs->code, flag);
			for(bd=0; bd < myPs->config.installedBd; bd++) {
				for(ch=0; ch < myPs->config.chPerBd; ch++) {
					if((myPs->config.chPerBd * bd + ch)
						>= myPs->config.installedCh) {
					continue;
					}
					if(myData->bData[bd].cData[ch].op.state == C_RUN
						&& myData->bData[bd].cData[ch]
						.signal[C_SIG_OT_FAULT] == P0) {
						myData->bData[bd].cData[ch]
							.signal[C_SIG_OT_FAULT] = P1;
					}
				}
			}
		}
		return;
	}

	base_addr = myPs->addr.interface[IO_EXPEND][2];
	addr = base_addr;
	flag = inb(addr);

	if((flag & 0x7F) != 0) {//smps fault 7.5V
		if(myPs->signal[M_SIG_SMPS_FAULT] == P0) {
			myPs->signal[M_SIG_SMPS_FAULT] = P1;
			myPs->signal[M_SIG_RUN_LED] = P0;
			myPs->signal[M_SIG_REMOTE_SMPS1] = P0;
			myPs->signal[M_SIG_FAN_RELAY] = P0;
			myPs->signal[M_SIG_POWER_OFF] = P1;
			myPs->code = M_FAIL_SMPS;
			send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 7, flag);
			send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
				(int)myPs->code, 0);
			for(bd=0; bd < myPs->config.installedBd; bd++) {
				for(ch=0; ch < myPs->config.chPerBd; ch++) {
					if((myPs->config.chPerBd * bd + ch)
						> (myPs->config.installedCh-1)) {
						continue;
					}
					if(myData->bData[bd].cData[ch].op.state == C_RUN
						&& myData->bData[bd].cData[ch]
						.signal[C_SIG_SMPS_FAULT] == P0) {
						myData->bData[bd].cData[ch]
							.signal[C_SIG_SMPS_FAULT] = P1;
					}
				}
			}
		}
		return;
	}

	base_addr = myPs->addr.interface[IO_EXPEND][4];
	addr = base_addr;
	flag = inb(addr);

	if((flag & 0x7F) != 0) { //smps fault 3.3V
		if(myPs->signal[M_SIG_SMPS_FAULT] == P0) {
			myPs->signal[M_SIG_SMPS_FAULT] = P1;
			myPs->signal[M_SIG_RUN_LED] = P0;
			myPs->signal[M_SIG_REMOTE_SMPS1] = P0;
			myPs->signal[M_SIG_FAN_RELAY] = P0;
			myPs->signal[M_SIG_POWER_OFF] = P1;
			myPs->code = M_FAIL_SMPS;
			send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 7, flag);
			send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
				(int)myPs->code, 0);
			for(bd=0; bd < myPs->config.installedBd; bd++) {
				for(ch=0; ch < myPs->config.chPerBd; ch++) {
					if((myPs->config.chPerBd * bd + ch)
						> (myPs->config.installedCh-1)) {
						continue;
					}
					if(myData->bData[bd].cData[ch].op.state == C_RUN
						&& myData->bData[bd].cData[ch]
						.signal[C_SIG_SMPS_FAULT] == P0) {
						myData->bData[bd].cData[ch]
							.signal[C_SIG_SMPS_FAULT] = P1;
					}
				}
			}
		}
		return;
	}
}

void Fault_Check_4(void)
{
	int bd, ch;
	int	addr, base_addr;
	unsigned short flag, bit;

	if(myData->dio.delayTimer < myData->dio.config.dioDelay) return;
	if(myPs->signal[M_SIG_FAN_RELAY] == P0) return;

	base_addr = myPs->addr.interface[IO_EXPEND][3];
	addr = base_addr;
	flag = inb(addr);

	if((flag & 0x0F) != 0x0F) { //OT fault
		if(myPs->signal[M_SIG_OT_FAULT] == P0) {
			myPs->signal[M_SIG_OT_FAULT] = P1;
			myPs->signal[M_SIG_RUN_LED] = P0;
			myPs->signal[M_SIG_REMOTE_SMPS1] = P0;
			myPs->signal[M_SIG_FAN_RELAY] = P0;
			myPs->signal[M_SIG_POWER_OFF] = P1;
			myPs->code = M_FAIL_OT;
			send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 6, flag);
			send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
				(int)myPs->code, flag);
			for(bd=0; bd < myPs->config.installedBd; bd++) {
				for(ch=0; ch < myPs->config.chPerBd; ch++) {
					if((myPs->config.chPerBd * bd + ch)
						>= myPs->config.installedCh) {
					continue;
					}
					if(myData->bData[bd].cData[ch].op.state == C_RUN
						&& myData->bData[bd].cData[ch]
						.signal[C_SIG_OT_FAULT] == P0) {
						myData->bData[bd].cData[ch]
							.signal[C_SIG_OT_FAULT] = P1;
					}
				}
			}
		}
		return;
	}

	base_addr = myPs->addr.interface[IO_EXPEND][2];
	addr = base_addr;
	flag = inb(addr);

	if((flag & 0x0F) != 0) {//smps fault 7.5V
		for(bd=0; bd < myPs->config.installedBd; bd++) {
			bit = 0x0001;
			for(ch=0; ch < myPs->config.chPerBd; ch++) {
				if((myPs->config.chPerBd * bd + ch)
					>= myPs->config.installedCh) {
					continue;
				}
				if((flag & (bit << ch)) != 0){
					if(myData->bData[bd].cData[ch].op.state == C_RUN){
						if(myData->bData[bd].cData[ch]
							.signal[C_SIG_SMPS_FAULT] == P0) {
							myData->bData[bd].cData[ch]
								.signal[C_SIG_SMPS_FAULT] = P1;
						}
					}
				}
			}
		}
	}

	base_addr = myPs->addr.interface[IO_EXPEND][4];
	addr = base_addr;
	flag = inb(addr);

	if((flag & 0x0F) != 0) { //smps fault 3.3V
		for(bd=0; bd < myPs->config.installedBd; bd++) {
			bit = 0x0001;
			for(ch=0; ch < myPs->config.chPerBd; ch++) {
				if((myPs->config.chPerBd * bd + ch)
					>= myPs->config.installedCh) {
					continue;
				}
				if((flag & (bit << ch)) != 0){
					if(myData->bData[bd].cData[ch].op.state == C_RUN){
						if(myData->bData[bd].cData[ch]
							.signal[C_SIG_SMPS_FAULT] == P0) {
							myData->bData[bd].cData[ch]
								.signal[C_SIG_SMPS_FAULT] = P1;
						}
					}
				}
			}
		}
	}
}

void Fault_Check_5(void)
{
	int bd, ch;
	int	addr, base_addr;
	unsigned short flag, bit;

	if(myData->dio.delayTimer < myData->dio.config.dioDelay) return;
	if(myPs->signal[M_SIG_FAN_RELAY] == P0) return;

	base_addr = myPs->addr.interface[IO_EXPEND][3];
	addr = base_addr;
	flag = inb(addr);

	if((flag & 0xFF) != 0xFF) { //OT fault
		if(myPs->signal[M_SIG_OT_FAULT] == P0) {
			myPs->signal[M_SIG_OT_FAULT] = P1;
			myPs->signal[M_SIG_RUN_LED] = P0;
			myPs->signal[M_SIG_REMOTE_SMPS1] = P0;
			myPs->signal[M_SIG_FAN_RELAY] = P0;
			myPs->signal[M_SIG_POWER_OFF] = P1;
			myPs->code = M_FAIL_OT;
			send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 6, flag);
			send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
				(int)myPs->code, flag);
			for(bd=0; bd < myPs->config.installedBd; bd++) {
				for(ch=0; ch < myPs->config.chPerBd; ch++) {
					if((myPs->config.chPerBd * bd + ch)
						>= myPs->config.installedCh) {
					continue;
					}
					if(myData->bData[bd].cData[ch].op.state == C_RUN
						&& myData->bData[bd].cData[ch]
						.signal[C_SIG_OT_FAULT] == P0) {
						myData->bData[bd].cData[ch]
							.signal[C_SIG_OT_FAULT] = P1;
					}
				}
			}
		}
		return;
	}

	base_addr = myPs->addr.interface[IO_EXPEND][2];
	addr = base_addr;
	flag = inb(addr);

	if((flag & 0xFF) != 0) {//smps fault 7.5V
		for(bd=0; bd < myPs->config.installedBd; bd++) {
			bit = 0x0001;
			for(ch=0; ch < myPs->config.chPerBd; ch++) {
				if((myPs->config.chPerBd * bd + ch)
					>= myPs->config.installedCh) {
					continue;
				}
				if((flag & (bit << ch)) != 0){
					if(myData->bData[bd].cData[ch].op.state == C_RUN){
						if(myData->bData[bd].cData[ch]
							.signal[C_SIG_SMPS_FAULT] == P0) {
							myData->bData[bd].cData[ch]
								.signal[C_SIG_SMPS_FAULT] = P1;
						}
					}
				}
			}
		}
	}

	base_addr = myPs->addr.interface[IO_EXPEND][4];
	addr = base_addr;
	flag = inb(addr);

	if((flag & 0xFF) != 0) { //smps fault 3.3V
		for(bd=0; bd < myPs->config.installedBd; bd++) {
			bit = 0x0001;
			for(ch=0; ch < myPs->config.chPerBd; ch++) {
				if((myPs->config.chPerBd * bd + ch)
					>= myPs->config.installedCh) {
					continue;
				}
				if((flag & (bit << ch)) != 0){
					if(myData->bData[bd].cData[ch].op.state == C_RUN){
						if(myData->bData[bd].cData[ch]
							.signal[C_SIG_SMPS_FAULT] == P0) {
							myData->bData[bd].cData[ch]
								.signal[C_SIG_SMPS_FAULT] = P1;
						}
					}
				}
			}
		}
	}
}

void Fault_Check_6(void)
{
	int bd, ch;
	int	addr, base_addr;
	unsigned short flag;

	if(myData->dio.delayTimer < myData->dio.config.dioDelay) return;
	if(myPs->signal[M_SIG_FAN_RELAY] == P0) return;

	base_addr = myPs->addr.interface[IO_EXPEND][3];
	addr = base_addr;
	flag = inb(addr);

	if((flag & 0xFF) != 0xFF) { //OT fault
		if(myPs->signal[M_SIG_OT_FAULT] == P0) {
			myPs->signal[M_SIG_OT_FAULT] = P1;
			myPs->signal[M_SIG_RUN_LED] = P0;
			myPs->signal[M_SIG_REMOTE_SMPS1] = P0;
			myPs->signal[M_SIG_FAN_RELAY] = P0;
			myPs->signal[M_SIG_POWER_OFF] = P1;
			myPs->code = M_FAIL_OT;
			send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 6, flag);
			send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
				(int)myPs->code, flag);
			for(bd=0; bd < myPs->config.installedBd; bd++) {
				for(ch=0; ch < myPs->config.chPerBd; ch++) {
					if((myPs->config.chPerBd * bd + ch)
						>= myPs->config.installedCh) {
					continue;
					}
					if(myData->bData[bd].cData[ch].op.state == C_RUN
						&& myData->bData[bd].cData[ch]
						.signal[C_SIG_OT_FAULT] == P0) {
						myData->bData[bd].cData[ch]
							.signal[C_SIG_OT_FAULT] = P1;
					}
				}
			}
		}
		return;
	}

	base_addr = myPs->addr.interface[IO_EXPEND][5];
	addr = base_addr;
	flag = inb(addr);

	if((flag & 0xFF) != 0xFF) { //OT fault
		if(myPs->signal[M_SIG_OT_FAULT] == P0) {
			myPs->signal[M_SIG_OT_FAULT] = P1;
			myPs->signal[M_SIG_RUN_LED] = P0;
			myPs->signal[M_SIG_REMOTE_SMPS1] = P0;
			myPs->signal[M_SIG_FAN_RELAY] = P0;
			myPs->signal[M_SIG_POWER_OFF] = P1;
			myPs->code = M_FAIL_OT;
			send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 6, flag);
			send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
				(int)myPs->code, flag);
			for(bd=0; bd < myPs->config.installedBd; bd++) {
				for(ch=0; ch < myPs->config.chPerBd; ch++) {
					if((myPs->config.chPerBd * bd + ch)
						>= myPs->config.installedCh) {
					continue;
					}
					if(myData->bData[bd].cData[ch].op.state == C_RUN
						&& myData->bData[bd].cData[ch]
						.signal[C_SIG_OT_FAULT] == P0) {
						myData->bData[bd].cData[ch]
							.signal[C_SIG_OT_FAULT] = P1;
					}
				}
			}
		}
		return;
	}


	base_addr = myPs->addr.interface[IO_EXPEND][2];
	addr = base_addr;
	flag = inb(addr);

	if((flag & 0x0F) != 0) {//smps fault 7.5V
		if(myPs->signal[M_SIG_SMPS_FAULT] == P0) {
			myPs->signal[M_SIG_SMPS_FAULT] = P1;
			myPs->signal[M_SIG_RUN_LED] = P0;
			myPs->signal[M_SIG_REMOTE_SMPS1] = P0;
			myPs->signal[M_SIG_FAN_RELAY] = P0;
			myPs->signal[M_SIG_POWER_OFF] = P1;
			myPs->code = M_FAIL_SMPS;
			send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 7, flag);
			send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
				(int)myPs->code, 0);
			for(bd=0; bd < myPs->config.installedBd; bd++) {
				for(ch=0; ch < myPs->config.chPerBd; ch++) {
					if((myPs->config.chPerBd * bd + ch)
						> (myPs->config.installedCh-1)) {
						continue;
					}
					if(myData->bData[bd].cData[ch].op.state == C_RUN
						&& myData->bData[bd].cData[ch]
						.signal[C_SIG_SMPS_FAULT] == P0) {
						myData->bData[bd].cData[ch]
							.signal[C_SIG_SMPS_FAULT] = P1;
					}
				}
			}
		}
		return;
	}

	base_addr = myPs->addr.interface[IO_EXPEND][4];
	addr = base_addr;
	flag = inb(addr);

	if((flag & 0x0F) != 0) { //smps fault 3.3V
		if(myPs->signal[M_SIG_SMPS_FAULT] == P0) {
			myPs->signal[M_SIG_SMPS_FAULT] = P1;
			myPs->signal[M_SIG_RUN_LED] = P0;
			myPs->signal[M_SIG_REMOTE_SMPS1] = P0;
			myPs->signal[M_SIG_FAN_RELAY] = P0;
			myPs->signal[M_SIG_POWER_OFF] = P1;
			myPs->code = M_FAIL_SMPS;
			send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 7, flag);
			send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
				(int)myPs->code, 0);
			for(bd=0; bd < myPs->config.installedBd; bd++) {
				for(ch=0; ch < myPs->config.chPerBd; ch++) {
					if((myPs->config.chPerBd * bd + ch)
						> (myPs->config.installedCh-1)) {
						continue;
					}
					if(myData->bData[bd].cData[ch].op.state == C_RUN
						&& myData->bData[bd].cData[ch]
						.signal[C_SIG_SMPS_FAULT] == P0) {
						myData->bData[bd].cData[ch]
							.signal[C_SIG_SMPS_FAULT] = P1;
					}
				}
			}
		}
		return;
	}
}

void Fault_Check_7(void)
{
	int bd, ch;
	int	addr, base_addr;
	unsigned short flag;
	unsigned char ot, smps;

	if(myData->dio.delayTimer < myData->dio.config.dioDelay) return;
	if(myPs->signal[M_SIG_FAN_RELAY] == P0) return;

	base_addr = myPs->addr.interface[IO_EXPEND][3];
	if(base_addr){
		addr = base_addr;
		flag = inb(addr);
		ot = myPs->fault.OT[0];

		if((flag & ot) != ot) { //OT fault
			if(myPs->signal[M_SIG_OT_FAULT] == P0) {
				myPs->signal[M_SIG_OT_FAULT] = P1;
				myPs->signal[M_SIG_RUN_LED] = P0;
				myPs->signal[M_SIG_REMOTE_SMPS1] = P0;
				myPs->signal[M_SIG_FAN_RELAY] = P0;
				myPs->signal[M_SIG_POWER_OFF] = P1;
				myPs->code = M_FAIL_OT;
				send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 6, flag);
				send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
					(int)myPs->code, flag);
				for(bd=0; bd < myPs->config.installedBd; bd++) {
					for(ch=0; ch < myPs->config.chPerBd; ch++) {
						if((myPs->config.chPerBd * bd + ch)
							>= myPs->config.installedCh) {
						continue;
						}
						if(myData->bData[bd].cData[ch].op.state == C_RUN
							&& myData->bData[bd].cData[ch]
							.signal[C_SIG_OT_FAULT] == P0) {
							myData->bData[bd].cData[ch]
								.signal[C_SIG_OT_FAULT] = P1;
						}
					}
				}
			}
			return;
		}
	}
	base_addr = myPs->addr.interface[IO_EXPEND][5];
	if(base_addr){
		addr = base_addr;
		flag = inb(addr);
		ot = myPs->fault.OT[1];

		if((flag & ot) != ot) { //OT fault
			if(myPs->signal[M_SIG_OT_FAULT] == P0) {
				myPs->signal[M_SIG_OT_FAULT] = P1;
				myPs->signal[M_SIG_RUN_LED] = P0;
				myPs->signal[M_SIG_REMOTE_SMPS1] = P0;
				myPs->signal[M_SIG_FAN_RELAY] = P0;
				myPs->signal[M_SIG_POWER_OFF] = P1;
				myPs->code = M_FAIL_OT;
				send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 6, flag);
				send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
					(int)myPs->code, flag);
				for(bd=0; bd < myPs->config.installedBd; bd++) {
					for(ch=0; ch < myPs->config.chPerBd; ch++) {
						if((myPs->config.chPerBd * bd + ch)
							>= myPs->config.installedCh) {
						continue;
						}
						if(myData->bData[bd].cData[ch].op.state == C_RUN
							&& myData->bData[bd].cData[ch]
							.signal[C_SIG_OT_FAULT] == P0) {
							myData->bData[bd].cData[ch]
								.signal[C_SIG_OT_FAULT] = P1;
						}
					}
				}
			}
			return;
		}
	}
	base_addr = myPs->addr.interface[IO_EXPEND][2];
	if(base_addr){
		addr = base_addr;
		flag = inb(addr);
		smps = myPs->fault.SMPS7_5V[0];

		if((flag & smps) != 0) {//smps fault 7.5V
			if(myPs->signal[M_SIG_SMPS_FAULT] == P0) {
				myPs->signal[M_SIG_SMPS_FAULT] = P1;
				myPs->signal[M_SIG_RUN_LED] = P0;
				myPs->signal[M_SIG_REMOTE_SMPS1] = P0;
				myPs->signal[M_SIG_FAN_RELAY] = P0;
				myPs->signal[M_SIG_POWER_OFF] = P1;
				myPs->code = M_FAIL_SMPS;
				send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 7, flag);
				send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
					(int)myPs->code, flag);
				for(bd=0; bd < myPs->config.installedBd; bd++) {
					for(ch=0; ch < myPs->config.chPerBd; ch++) {
						if((myPs->config.chPerBd * bd + ch)
							> (myPs->config.installedCh-1)) {
							continue;
						}
						if(myData->bData[bd].cData[ch].op.state == C_RUN
							&& myData->bData[bd].cData[ch]
							.signal[C_SIG_SMPS_FAULT] == P0) {
							myData->bData[bd].cData[ch]
								.signal[C_SIG_SMPS_FAULT] = P1;
						}
					}
				}
			}
			return;
		}
	}

	base_addr = myPs->addr.interface[IO_EXPEND][4];
	if(base_addr){
		addr = base_addr;
		flag = inb(addr);
		smps = myPs->fault.SMPS3_3V[0];

		if((flag & smps) != 0) { //smps fault 3.3V
			if(myPs->signal[M_SIG_SMPS_FAULT] == P0) {
				myPs->signal[M_SIG_SMPS_FAULT] = P1;
				myPs->signal[M_SIG_RUN_LED] = P0;
				myPs->signal[M_SIG_REMOTE_SMPS1] = P0;
				myPs->signal[M_SIG_FAN_RELAY] = P0;
				myPs->signal[M_SIG_POWER_OFF] = P1;
				myPs->code = M_FAIL_SMPS;
				send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 7, flag);
				send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
					(int)myPs->code, flag);
				for(bd=0; bd < myPs->config.installedBd; bd++) {
					for(ch=0; ch < myPs->config.chPerBd; ch++) {
						if((myPs->config.chPerBd * bd + ch)
							> (myPs->config.installedCh-1)) {
							continue;
						}
						if(myData->bData[bd].cData[ch].op.state == C_RUN
							&& myData->bData[bd].cData[ch]
							.signal[C_SIG_SMPS_FAULT] == P0) {
							myData->bData[bd].cData[ch]
								.signal[C_SIG_SMPS_FAULT] = P1;
						}
					}
				}
			}
			return;
		}
	}
}

void Fault_Check_8(void)
{
	int bd, ch;
	int	addr, base_addr;
	unsigned short flag,sendFlag = 0;
	unsigned char ot, smps;
	static unsigned char preSMPS[2]={0},preOT[2]={0};
	unsigned char otFlag[2] = {0};
	
	if(myData->dio.delayTimer < myData->dio.config.dioDelay) return;
	if(myPs->signal[M_SIG_FAN_RELAY] == P0) return;

	base_addr = myPs->addr.interface[IO_EXPEND][3];
	if(base_addr){
		addr = base_addr;
		flag = inb(addr);
		ot = myPs->fault.OT[0];

		if((flag & ot) != ot) { //OT fault
			if(myPs->signal[M_SIG_OT_FAULT] == P0 || preOT[0] != flag ) {
				preOT[0] = flag;
				//100908 oys OT signal send
				sendFlag = ~flag & ot;
				otFlag[0] = 1;
			}
		//	return;
		} else {
			myPs->signal[M_SIG_OT_FAULT] = P0;
			preOT[0] = flag;
			otFlag[0] = 0;
		}
	}
	base_addr = myPs->addr.interface[IO_EXPEND][5];
	if(base_addr){
		addr = base_addr;
		flag = inb(addr);
		ot = myPs->fault.OT[1];

		if((flag & ot) != ot) { //OT fault
			if(myPs->signal[M_SIG_OT_FAULT1] == P0 || preOT[1] != flag) {
				preOT[1] = flag;
				//100908 oys OT signal send
				sendFlag |= (~flag & ot) << 8;
				otFlag[1] = 1;
			}
		//	return;
		} else {
			myPs->signal[M_SIG_OT_FAULT1] = P0;
			preOT[1] = flag;
			otFlag[1] = 0;
		}
	}
	if(otFlag[0] || otFlag[1]){
		if((inb(0x602) & 0x10) == P0) {
			myPs->signal[M_SIG_RUN_LED] = P0;
			myPs->signal[M_SIG_REMOTE_SMPS1] = P0;
			myPs->signal[M_SIG_FAN_RELAY] = P0;
			myPs->signal[M_SIG_POWER_OFF] = P1;
			send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 6, sendFlag);
		}
		myPs->code = M_FAIL_OT;
		myPs->signal[M_SIG_OT_FAULT] = P1;
		myPs->signal[M_SIG_OT_FAULT1] = P1;
		send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
			(int)myPs->code, sendFlag);
		for(bd=0; bd < myPs->config.installedBd; bd++) {
			for(ch=0; ch < myPs->config.chPerBd; ch++) {
				if((myPs->config.chPerBd * bd + ch)
					>= myPs->config.installedCh) {
				continue;
				}
				if(myData->bData[bd].cData[ch].op.state == C_RUN
					&& myData->bData[bd].cData[ch]
					.signal[C_SIG_OT_FAULT] == P0) {
					myData->bData[bd].cData[ch]
					.signal[C_SIG_OT_FAULT] = P1;
				}
			}
		}
	}
	base_addr = myPs->addr.interface[IO_EXPEND][2];
	if(base_addr){
		addr = base_addr;
		flag = inb(addr);
		smps = myPs->fault.SMPS7_5V[0];

		if((flag & smps) != 0) {//smps fault 7.5V
			if(myPs->signal[M_SIG_SMPS_FAULT] == P0 || preSMPS[0] != flag) {
				preSMPS[0] = flag;
				sendFlag = flag & smps;
				if((inb(0x602) & 0x10) == P0) {
					myPs->signal[M_SIG_RUN_LED] = P0;
					myPs->signal[M_SIG_REMOTE_SMPS1] = P0;
					myPs->signal[M_SIG_FAN_RELAY] = P0;
					myPs->signal[M_SIG_POWER_OFF] = P1;
					myPs->code = M_FAIL_SMPS;
					send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 7, sendFlag);
				}
				myPs->code = M_FAIL_SMPS;
				myPs->signal[M_SIG_SMPS_FAULT] = P1;
				send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
					(int)myPs->code, sendFlag);
				for(bd=0; bd < myPs->config.installedBd; bd++) {
					for(ch=0; ch < myPs->config.chPerBd; ch++) {
						if((myPs->config.chPerBd * bd + ch)
							> (myPs->config.installedCh-1)) {
							continue;
						}
						if(myData->bData[bd].cData[ch].op.state == C_RUN
							&& myData->bData[bd].cData[ch]
							.signal[C_SIG_SMPS_FAULT] == P0) {
							myData->bData[bd].cData[ch]
								.signal[C_SIG_SMPS_FAULT] = P1;
						}
					}
				}
			}
		//	return;
		} else {
			myPs->signal[M_SIG_SMPS_FAULT] = P0;
			preSMPS[0] = flag;
		}
	}

	base_addr = myPs->addr.interface[IO_EXPEND][4];
	if(base_addr){
		addr = base_addr;
		flag = inb(addr);
		smps = myPs->fault.SMPS3_3V[0];
		if((flag & smps) != 0) { //smps fault 3.3V
			if(myPs->signal[M_SIG_SMPS_FAULT1] == P0 || preSMPS[1] != flag) {
				preSMPS[1] = flag;
				sendFlag = (flag & smps) << 8;
				if((inb(0x602) & 0x10) == P0) {
					myPs->signal[M_SIG_RUN_LED] = P0;
					myPs->signal[M_SIG_REMOTE_SMPS1] = P0;
					myPs->signal[M_SIG_FAN_RELAY] = P0;
					myPs->signal[M_SIG_POWER_OFF] = P1;
					send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 8, sendFlag);
				}
				myPs->code = M_FAIL_SMPS;
				myPs->signal[M_SIG_SMPS_FAULT1] = P1;
				send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
					(int)myPs->code, sendFlag);
				for(bd=0; bd < myPs->config.installedBd; bd++) {
					for(ch=0; ch < myPs->config.chPerBd; ch++) {
						if((myPs->config.chPerBd * bd + ch)
							> (myPs->config.installedCh-1)) {
							continue;
						}
						if(myData->bData[bd].cData[ch].op.state == C_RUN
							&& myData->bData[bd].cData[ch]
							.signal[C_SIG_SMPS_FAULT] == P0) {
							myData->bData[bd].cData[ch]
								.signal[C_SIG_SMPS_FAULT] = P1;
						}
					}
				}
			}
		//	return;
		} else {
			myPs->signal[M_SIG_SMPS_FAULT1] = P0;
			preSMPS[1] = flag;
		}
	}
}


void Fault_Check_9(void)
{
	unsigned char	flag1, flag2, flag3=0, flag4=0;
	unsigned short sendFlag;
	int rd_addr1, rd_addr2, wr_addr1, bd, ch;

	if(myData->dio.delayTimer < myData->dio.config.dioDelay) return;

	rd_addr1 = myPs->addr.interface[IO_EXPEND][6];
	rd_addr2 = myPs->addr.interface[IO_EXPEND][7];
	wr_addr1 = myPs->addr.interface[IO_EXPEND][8];
	if(myPs->signal[M_SIG_INV_AC_FAULT] == P11) { //UPS signal wait	
		myPs->misc.invertFailWait += myPs->misc.dio_scan_time; //ups signal input goto p0
		if(myData->dio.signal[DIO_SIG_POWER_FAIL] >= P1) { //power fail
			myPs->misc.invertFailWait = 0;
			myPs->signal[M_SIG_INV_AC_FAULT] = P0;
		} else if(myPs->misc.invertFailWait > 5000) { //non power fail
			myPs->signal[M_SIG_INV_AC_FAULT] = P12;
			myPs->signal[M_SIG_INV_FAULT] = P0;
			myPs->misc.invertFailWait = 0;
		}
	}else if(myPs->signal[M_SIG_INV_AC_FAULT] == P12) { //INV RUN
		myPs->signal[M_SIG_REMOTE_SMPS1] = P1;
		myPs->signal[M_SIG_INV_AC_FAULT] = P13;
	}else if(myPs->signal[M_SIG_INV_AC_FAULT] == P13) { //CH RUN WAIT
		myPs->misc.invertFailWait += myPs->misc.dio_scan_time;
		if(myPs->misc.invertFailWait == 2000){
			myPs->signal[M_SIG_INV_FAULT] = P10; //run wait, fault shutdown
		}
		if(myPs->misc.invertFailWait > 3000){
			myPs->misc.invertFailWait = 0;
			myPs->signal[M_SIG_INV_AC_FAULT] = P14;
		}
	}else if(myPs->signal[M_SIG_INV_AC_FAULT] == P14) { //CH RUN
		if(myPs->misc.invertFailWait == 0) {
			for(bd=0; bd < myPs->config.installedBd; bd++) {
				for(ch=0; ch < 4; ch++) {
					if(myData->bData[bd].cData[ch].op.state == C_PAUSE
						&& myData->bData[bd].cData[ch].op.code == C_FAULT_INVERTER) {
						myData->bData[bd].cData[ch].signal[C_SIG_CONTINUE] = P1;
					}
				}
			}
		}
		myPs->misc.invertFailWait += myPs->misc.dio_scan_time;
		if(myPs->misc.invertFailWait > 1000){
			myPs->misc.invertFailWait = 0;
			myPs->signal[M_SIG_INV_AC_FAULT] = P15;
		}
	}else if(myPs->signal[M_SIG_INV_AC_FAULT] == P15) { //CH RUN
		if(myPs->misc.invertFailWait == 0) {
			for(bd=0; bd < myPs->config.installedBd; bd++) {
				for(ch=4; ch < 8; ch++) {
					if(myData->bData[bd].cData[ch].op.state == C_PAUSE
						&& myData->bData[bd].cData[ch].op.code == C_FAULT_INVERTER) {
						myData->bData[bd].cData[ch].signal[C_SIG_CONTINUE] = P1;
					}
				}
			}
		}
		myPs->misc.invertFailWait += myPs->misc.dio_scan_time;
		if(myPs->misc.invertFailWait > 1000){
			myPs->misc.invertFailWait = 0;
			myPs->signal[M_SIG_INV_AC_FAULT] = P16;
		}
	}else if(myPs->signal[M_SIG_INV_AC_FAULT] == P16) { //CH RUN
		if(myPs->misc.invertFailWait == 0) {
			for(bd=0; bd < myPs->config.installedBd; bd++) {
				for(ch=8; ch < 12; ch++) {
					if(myData->bData[bd].cData[ch].op.state == C_PAUSE
						&& myData->bData[bd].cData[ch].op.code == C_FAULT_INVERTER) {
						myData->bData[bd].cData[ch].signal[C_SIG_CONTINUE] = P1;
					}
				}
			}
		}
		myPs->misc.invertFailWait += myPs->misc.dio_scan_time;
		if(myPs->misc.invertFailWait > 1000){
			myPs->misc.invertFailWait = 0;
			myPs->signal[M_SIG_INV_AC_FAULT] = P17;
		}
	}else if(myPs->signal[M_SIG_INV_AC_FAULT] == P17) { //CH RUN
		if(myPs->misc.invertFailWait == 0) {
			for(bd=0; bd < myPs->config.installedBd; bd++) {
				for(ch=12; ch < 16; ch++) {
					if(myData->bData[bd].cData[ch].op.state == C_PAUSE
						&& myData->bData[bd].cData[ch].op.code == C_FAULT_INVERTER) {
						myData->bData[bd].cData[ch].signal[C_SIG_CONTINUE] = P1;
					}
				}
			}
		}
		myPs->misc.invertFailWait += myPs->misc.dio_scan_time;
		if(myPs->misc.invertFailWait > 1000){
				myPs->misc.invertFailWait = 0;
				myPs->signal[M_SIG_INV_AC_FAULT] = P0;
				myPs->signal[M_SIG_INV_FAULT] = P0;
		}
	}else if(myPs->signal[M_SIG_INV_AC_FAULT] == P18) { // ALL CH PAUSE
		for(bd=0; bd < myPs->config.installedBd; bd++) {
			for(ch=0; ch < myPs->config.chPerBd; ch++) {
				if((myPs->config.chPerBd * bd + ch)
					> (myPs->config.installedCh-1)) {
					continue;
				}		
				if(myData->bData[bd].cData[ch].op.state == C_RUN
					&& myData->bData[bd].cData[ch].signal[C_SIG_INV_FAULT] == P0) {
					myData->bData[bd].cData[ch].signal[C_SIG_INV_FAULT] = P1;
				}
			}
		}
	}
	if(myPs->signal[M_SIG_REMOTE_SMPS_DELAY] != P50 
	 &&myPs->signal[M_SIG_REMOTE_SMPS_DELAY] != P100) return;

	flag1 = inb(rd_addr1);
	flag2 = inb(rd_addr2);
	
	if(myPs->config.installedCh == 16) {
		flag3 = 0xAA;
		flag4 = 0xAA;
	} else if(myPs->config.installedCh == 4) {
		flag3 = 0x0A;
		flag4 = 0x00;
	}
	if(((flag1 & flag3) != 0) // INV OT,DC Fault 
		|| ((flag2 & flag4) != 0)) {
		if(myPs->signal[M_SIG_INV_FAULT] == P0) {
			myPs->signal[M_SIG_INV_FAULT] = P1;
			myPs->signal[M_SIG_RUN_LED] = P0;
			myPs->signal[M_SIG_REMOTE_SMPS1] = P0;
			myPs->signal[M_SIG_FAN_RELAY] = P0;
			myPs->signal[M_SIG_POWER_OFF] = P1;
			myPs->code = M_FAIL_INV;
			sendFlag = (flag2 << 8 ) | flag1;
			send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 8, sendFlag);
			send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
				(int)myPs->code, sendFlag);
			for(bd=0; bd < myPs->config.installedBd; bd++) {
				for(ch=0; ch < myPs->config.chPerBd; ch++) {
					if((myPs->config.chPerBd * bd + ch)
						> (myPs->config.installedCh-1)) {
						continue;
					}
					if(myData->bData[bd].cData[ch].op.state == C_RUN
						&& myData->bData[bd].cData[ch]
						.signal[C_SIG_INV_FAULT] == P0) {
						myData->bData[bd].cData[ch]
							.signal[C_SIG_INV_FAULT] = P1;
					}
				}
			}
		}
		return;
	}
	if(myPs->config.installedCh == 16) {
		flag3 = 0x44;
		flag4 = 0x44;
	} else if(myPs->config.installedCh == 4) {
		flag3 = 0x04;
		flag4 = 0x00;
	}
	if(((flag1 & flag3) != 0) // INV AC Fault
		|| ((flag2 & flag4) != 0)) {
		if(myPs->signal[M_SIG_INV_FAULT] == P0) {
			myPs->misc.invertFailWait = 0;
			printk("****** AC_FAULT Short off ******\n");
			myPs->signal[M_SIG_INV_FAULT] = P1;
			myPs->signal[M_SIG_INV_AC_FAULT] = P11;
			myPs->signal[M_SIG_REMOTE_SMPS1] = P0;
			for(bd=0; bd < myPs->config.installedBd; bd++) {
				for(ch=0; ch < myPs->config.chPerBd; ch++) {
					if((myPs->config.chPerBd * bd + ch)
						> (myPs->config.installedCh-1)) {
						continue;
					}
					if(myData->bData[bd].cData[ch].op.state == C_RUN
						&& myData->bData[bd].cData[ch]
						.signal[C_SIG_INV_FAULT] == P0) {
						myData->bData[bd].cData[ch]
							.signal[C_SIG_INV_FAULT] = P1;
					}
				}
			}
		}
		if(myPs->signal[M_SIG_INV_FAULT] == P10) {
			printk("****** AC_FAULT Shutdown process ******\n");
			myPs->signal[M_SIG_INV_FAULT] = P1;
			myPs->signal[M_SIG_INV_AC_FAULT] = P18; // ALL CH PAUSE
			myPs->signal[M_SIG_RUN_LED] = P0;
			myPs->signal[M_SIG_REMOTE_SMPS1] = P0;
			myPs->signal[M_SIG_FAN_RELAY] = P0;
			myPs->signal[M_SIG_POWER_OFF] = P1;
			myPs->code = M_FAIL_INV;
			sendFlag = (flag2 << 8 ) | flag1;
			send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 8, sendFlag);
			send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
				(int)myPs->code, sendFlag);
			for(bd=0; bd < myPs->config.installedBd; bd++) {
				for(ch=0; ch < myPs->config.chPerBd; ch++) {
					if((myPs->config.chPerBd * bd + ch)
						> (myPs->config.installedCh-1)) {
						continue;
					}
					if(myData->bData[bd].cData[ch].op.state == C_RUN
						&& myData->bData[bd].cData[ch]
						.signal[C_SIG_INV_FAULT] == P0) {
						myData->bData[bd].cData[ch]
							.signal[C_SIG_INV_FAULT] = P1;
					}
				}
			}
		}
		return;
	}
}

/*Date				:2010	11	18
 *Editor			:pjy
 *Name				:Fault_Check_10
 *parameter			:none
 *returnValue		:none
 *function description
 *16채널 이상의 오티 신호를 사용하기 위해서 610, 611, 612, 613번지를 오티로 사용 *할경우오티 신호를 검사하는 함수를 추가적으로 만들어주었다
 * 
 *Control BD rev 01일경우 적용가능
 */ 
void Fault_Check_10(void)
{
	int bd, ch;
	int	addr, base_addr;
	unsigned short flag,sendFlag = 0;
	unsigned char ot, smps;
	static unsigned char preSMPS[2]={0},preOT[4]={0};
	unsigned char otFlag[4] = {0};
	
	if(myData->dio.delayTimer < myData->dio.config.dioDelay) return;
	if(myPs->signal[M_SIG_FAN_RELAY] == P0) return;

	//OT Signal Check
	
	//610
	base_addr = myPs->addr.interface[IO_EXPEND][2];
	if(base_addr){
		addr = base_addr;
		flag = inb(addr);
		ot = myPs->fault.OT[0];

		if((flag & ot) != ot) { //OT fault
			if(myPs->signal[M_SIG_OT_FAULT] == P0 || preOT[0] != flag ) {
			//if(myPs->signal[M_SIG_OT_FAULT] == P0) {
				preOT[0] = flag;
				//100908 oys OT signal send
				sendFlag = ~flag & ot;
				otFlag[0] = 1;
			}
		//	return;
		} else {
			myPs->signal[M_SIG_OT_FAULT] = P0;
			preOT[0] = flag;
			otFlag[0] = 0;
		}
	}
	//611
	base_addr = myPs->addr.interface[IO_EXPEND][3];
	if(base_addr){
		addr = base_addr;
		flag = inb(addr);
		ot = myPs->fault.OT[1];

		if((flag & ot) != ot) { //OT fault
			if(myPs->signal[M_SIG_OT_FAULT] == P0 || preOT[1] != flag ) {
			//if(myPs->signal[M_SIG_OT_FAULT] == P0) {
				preOT[1] = flag;
				//100908 oys OT signal send
				sendFlag = ~flag & ot;
				otFlag[1] = 1;
			}
		//	return;
		} else {
			myPs->signal[M_SIG_OT_FAULT] = P0;
			preOT[1] = flag;
			otFlag[1] = 0;
		}
	}
	//612
	base_addr = myPs->addr.interface[IO_EXPEND][4];
	if(base_addr){
		addr = base_addr;
		flag = inb(addr);
		ot = myPs->fault.OT[2];

		if((flag & ot) != ot) { //OT fault
			if(myPs->signal[M_SIG_OT_FAULT] == P0 || preOT[2] != flag ) {
		//	if(myPs->signal[M_SIG_OT_FAULT] == P0 ) {
				preOT[2] = flag;
				//100908 oys OT signal send
				sendFlag = ~flag & ot;
				otFlag[2] = 1;
			}
		//	return;
		} else {
			myPs->signal[M_SIG_OT_FAULT] = P0;
			preOT[2] = flag;
			otFlag[2] = 0;
		}
	}

	base_addr = myPs->addr.interface[IO_EXPEND][5];
	if(base_addr){
		addr = base_addr;
		flag = inb(addr);
		ot = myPs->fault.OT[3];

		if((flag & ot) != ot) { //OT fault
			if(myPs->signal[M_SIG_OT_FAULT1] == P0 || preOT[3] != flag) {
		//	if(myPs->signal[M_SIG_OT_FAULT] == P0) {
				preOT[3] = flag;
				//100908 oys OT signal send
				sendFlag |= (~flag & ot) << 8;
				otFlag[3] = 1;
			}
		//	return;
		} else {
			myPs->signal[M_SIG_OT_FAULT1] = P0;
			preOT[3] = flag;
			otFlag[3] = 0;
		}
	}
	if(otFlag[0] || otFlag[1] || otFlag[2] || otFlag[3]){
		if((inb(0x602) & 0x10) == P0) {
			myPs->signal[M_SIG_RUN_LED] = P0;
			myPs->signal[M_SIG_REMOTE_SMPS1] = P0;
			myPs->signal[M_SIG_FAN_RELAY] = P0;
			myPs->signal[M_SIG_POWER_OFF] = P1;
			send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 6, sendFlag);
		}
		myPs->code = M_FAIL_OT;
		myPs->signal[M_SIG_OT_FAULT] = P1;
		myPs->signal[M_SIG_OT_FAULT1] = P1;
		send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
			(int)myPs->code, sendFlag);
		
		for(bd=0; bd < myPs->config.installedBd; bd++) {
			for(ch=0; ch < myPs->config.chPerBd; ch++) {
				if((myPs->config.chPerBd * bd + ch)
					>= myPs->config.installedCh) {
				continue;
				}
				if(myData->bData[bd].cData[ch].op.state == C_RUN
					&& myData->bData[bd].cData[ch]
					.signal[C_SIG_OT_FAULT] == P0) {
					myData->bData[bd].cData[ch]
					.signal[C_SIG_OT_FAULT] = P1;
				}
			}
		}
	}
	base_addr = myPs->addr.interface[IO_EXPEND][2];
	if(base_addr){
		addr = base_addr;
		flag = inb(addr);
		smps = myPs->fault.SMPS7_5V[0];

		if((flag & smps) != 0) {//smps fault 7.5V
			if(myPs->signal[M_SIG_SMPS_FAULT] == P0 || preSMPS[0] != flag) {
				preSMPS[0] = flag;
				sendFlag = flag & smps;
				if((inb(0x602) & 0x10) == P0) {
					myPs->signal[M_SIG_RUN_LED] = P0;
					myPs->signal[M_SIG_REMOTE_SMPS1] = P0;
					myPs->signal[M_SIG_FAN_RELAY] = P0;
					myPs->signal[M_SIG_POWER_OFF] = P1;
					myPs->code = M_FAIL_SMPS;
					send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 7, sendFlag);
				}
				myPs->code = M_FAIL_SMPS;
				myPs->signal[M_SIG_SMPS_FAULT] = P1;
				send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
					(int)myPs->code, sendFlag);
				for(bd=0; bd < myPs->config.installedBd; bd++) {
					for(ch=0; ch < myPs->config.chPerBd; ch++) {
						if((myPs->config.chPerBd * bd + ch)
							> (myPs->config.installedCh-1)) {
							continue;
						}
						if(myData->bData[bd].cData[ch].op.state == C_RUN
							&& myData->bData[bd].cData[ch]
							.signal[C_SIG_SMPS_FAULT] == P0) {
							myData->bData[bd].cData[ch]
								.signal[C_SIG_SMPS_FAULT] = P1;
						}
					}
				}
			}
		//	return;
		} else {
			myPs->signal[M_SIG_SMPS_FAULT] = P0;
			preSMPS[0] = flag;
		}
	}

	base_addr = myPs->addr.interface[IO_EXPEND][4];
	if(base_addr){
		addr = base_addr;
		flag = inb(addr);
		smps = myPs->fault.SMPS3_3V[0];
		if((flag & smps) != 0) { //smps fault 3.3V
			if(myPs->signal[M_SIG_SMPS_FAULT1] == P0 || preSMPS[1] != flag) {
				preSMPS[1] = flag;
				sendFlag = (flag & smps) << 8;
				if((inb(0x602) & 0x10) == P0) {
					myPs->signal[M_SIG_RUN_LED] = P0;
					myPs->signal[M_SIG_REMOTE_SMPS1] = P0;
					myPs->signal[M_SIG_FAN_RELAY] = P0;
					myPs->signal[M_SIG_POWER_OFF] = P1;
					send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 8, sendFlag);
				}
				myPs->code = M_FAIL_SMPS;
				myPs->signal[M_SIG_SMPS_FAULT1] = P1;
				send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
					(int)myPs->code, sendFlag);
				for(bd=0; bd < myPs->config.installedBd; bd++) {
					for(ch=0; ch < myPs->config.chPerBd; ch++) {
						if((myPs->config.chPerBd * bd + ch)
							> (myPs->config.installedCh-1)) {
							continue;
						}
						if(myData->bData[bd].cData[ch].op.state == C_RUN
							&& myData->bData[bd].cData[ch]
							.signal[C_SIG_SMPS_FAULT] == P0) {
							myData->bData[bd].cData[ch]
								.signal[C_SIG_SMPS_FAULT] = P1;
						}
					}
				}
			}
		//	return;
		} else {
			myPs->signal[M_SIG_SMPS_FAULT1] = P0;
			preSMPS[1] = flag;
		}
	}
}
/*
 *Date				:2011	10	4
 *Editor			:pjy
 *Name				:Fault_Check_11
 *parameter			:none
 *returnValue		:none
 *function description
 *
 * Main SMPS, OT Fail신호시 해당 채널만 일시 정지 시켜주는 기능
 * Control BD rev 01일경우 적용가능
*/ 
void Fault_Check_11(void)
{
	int bd, ch;
	int	addr, base_addr, base_addr2;
	unsigned short ot, smps, flag, sendFlag = 0, compareValue1, compareValue2;
	static unsigned short preSMPS[2]={0},preOT[1]={0};
	int chPerSMPS7;
	int chPerSMPS3;
	int chPerOT;
	int smpsCount;
	int otCount;
	int fault7Flag[8] = {0};
	int otFlag[16] = {0};
	int max, i, j, k, type;
	int start;
	
	int otShutDownFlag = 1;	

	if(myData->dio.delayTimer < myData->dio.config.dioDelay) return;
	if(myPs->signal[M_SIG_FAN_RELAY] == P0) return;

	// 140123 oys modify : OT Fault
	// 1. OT Fault check
	base_addr = myPs->addr.interface[IO_EXPEND][3];
	base_addr2 = myPs->addr.interface[IO_EXPEND][5];
	if(base_addr){
		// addr 0x611
		addr = base_addr;
		flag = inb(addr);
		// addr 0x613
		addr = base_addr2;
		flag = flag + (inb(addr) << 8);
		ot = myPs->fault.OT[0] + (myPs->fault.OT[1] << 8);
		//1,1. OT Fault 
		if((flag & ot) != ot) { //OT fault
			compareValue1 = 0x01;
			otCount = 0;
			for(i=0; i<16; i++){
				compareValue2 = compareValue1 << i;
				if((compareValue2 & ot) == compareValue2){
					otCount++;
				}
			}
			if(myPs->config.installedCh >= otCount){
				type = myPs->config.installedCh % otCount;
			}else{
				type = otCount % myPs->config.installedCh;
			}

			if(otShutDownFlag == P1)
				type = 1;

			//1,1,1. 나누어 떨어질때
			if(type == 0){
				// installedCh >= OT SIGNAL
				if(myPs->config.installedCh >= otCount){
					if(myPs->signal[M_SIG_OT_FAULT] == P0 || preOT[0] != flag ) {
						preOT[0] = flag;
						//100908 oys OT signal send
						sendFlag = ~flag & ot;
						myPs->code = M_FAIL_OT;
						send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
							(int)myPs->code, sendFlag);
					}
					myPs->signal[M_SIG_OT_FAULT] = P1;
					compareValue1 = 0x01;
					for(i=0; i<16; i++){
						compareValue2 = compareValue1 << i;
						if((flag & compareValue2) == compareValue2){
							otFlag[i] = 0;
						}else{
							otFlag[i] = 1;
						}
					}
					chPerOT = (myPs->config.installedCh) / otCount;
					for(i=0; i<otCount; i++){
						if(otFlag[i] == 1){
							start = chPerOT * i;
							max = chPerOT * (i+1);
							bd = (i * chPerOT) / (myPs->config.chPerBd);
							ch = (i * chPerOT) % (myPs->config.chPerBd);
							k = 0;
							for(j=start; j<max; j++){
								if((myPs->config.chPerBd * bd + ch)
									> (myPs->config.installedCh-1)){
									continue;
								}
								if(myData->bData[bd].cData[ch+k].op.state == C_RUN
									&& myData->bData[bd].cData[ch+k]
									.signal[C_SIG_OT_FAULT] == P0){
									myData->bData[bd].cData[ch+k]
									.signal[C_SIG_OT_FAULT] = P1;
									if(myData->bData[bd].cData[ch]
										.ChAttribute.chNo_master == P0) {
											myData->bData[bd].cData[ch+k-1]
												.signal[C_SIG_OT_FAULT] = P1;
									} else if(myData->bData[bd].cData[ch]
												.ChAttribute.opType == P1) {
											myData->bData[bd].cData[ch+k+1]
												.signal[C_SIG_OT_FAULT] = P1;
									}	
								}
								k++;	
							}
						}
					}
				}else{ //installedCh < OT SIGNAL
					if(myPs->signal[M_SIG_OT_FAULT] == P0 || preOT[0] != flag ) {
						preOT[0] = flag;
						//100908 oys OT signal send
						sendFlag = ~flag & ot;
						myPs->code = M_FAIL_OT;
						send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
							(int)myPs->code, sendFlag);
					}
					myPs->signal[M_SIG_OT_FAULT] = P1;
					compareValue1 = 0x01;
					for(i=0; i<16; i++){
						compareValue2 = compareValue1 << i;
						if((flag & compareValue2) == compareValue2){
							otFlag[i] = 0;
						}else{
							otFlag[i] = 1;
						}
					}
					chPerOT = otCount / myPs->config.installedCh;
					for(i=0; i<otCount; i++){
						if(otFlag[i] == 1){
							bd = 0;
							ch = i / chPerOT;
							if(myData->bData[bd].cData[ch].op.state == C_RUN
								&& myData->bData[bd].cData[ch]
								.signal[C_SIG_OT_FAULT] == P0){
								myData->bData[bd].cData[ch]
								.signal[C_SIG_OT_FAULT] = P1;
								if(myData->bData[bd].cData[ch]
									.ChAttribute.chNo_master == P0) {
										myData->bData[bd].cData[ch-1]
											.signal[C_SIG_OT_FAULT] = P1;
								} else if(myData->bData[bd].cData[ch]
											.ChAttribute.opType == P1) {
										myData->bData[bd].cData[ch+1]
											.signal[C_SIG_OT_FAULT] = P1;
								}	
							}
						}
					}	
				}	
			//1,1,2. 나누어 떨어지지 않을때(Shutdown)
			}else{
				if(myPs->signal[M_SIG_OT_FAULT] == P0 || preOT[0] != flag ) {
					preOT[0] = flag;
					//100908 oys OT signal send
					sendFlag = ~flag & ot;
					if((inb(0x602) & 0x10) == P0) {
						myPs->signal[M_SIG_RUN_LED] = P0;
						myPs->signal[M_SIG_REMOTE_SMPS1] = P0;
						myPs->signal[M_SIG_FAN_RELAY] = P0;
						myPs->signal[M_SIG_POWER_OFF] = P1;
						send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 6, sendFlag);
					}
					myPs->code = M_FAIL_OT;
					myPs->signal[M_SIG_OT_FAULT] = P1;
					send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
						(int)myPs->code, sendFlag);
					for(bd=0; bd < myPs->config.installedBd; bd++) {
						for(ch=0; ch < myPs->config.chPerBd; ch++) {
							if((myPs->config.chPerBd * bd + ch)
								>= myPs->config.installedCh) {
								continue;
							}
							if(myData->bData[bd].cData[ch].op.state == C_RUN
								&& myData->bData[bd].cData[ch]
								.signal[C_SIG_OT_FAULT] == P0) {
								myData->bData[bd].cData[ch]
								.signal[C_SIG_OT_FAULT] = P1;
							}
						}
					}
				}else{
					myPs->signal[M_SIG_OT_FAULT] = P0;
					preOT[0] = flag;
					otFlag[0] = 0;
				}
			}
		//OT FAULT
		} else {
			myPs->signal[M_SIG_OT_FAULT] = P0;
		}
	}
	//2. 7V Main SMPS Fault check
	base_addr = myPs->addr.interface[IO_EXPEND][2];
	if(base_addr){
		addr = base_addr;
		flag = inb(addr);
		smps = myPs->fault.SMPS7_5V[0];
		//2,1. 7V smps fail
		if((flag & smps) != 0) {//smps fault 7.5V
			compareValue1 = 0x01;
			smpsCount = 0;
			for( i = 0; i < 8; i++){
				compareValue2 = compareValue1 << i;
				if((compareValue2 & smps) == compareValue2){
					smpsCount++;
				}
			}
			type = myPs->config.installedCh % smpsCount;
		//2,1,1 나누어 떨어질때(new)
			if( type == 0){	
				if(myPs->signal[M_SIG_SMPS_FAULT] == P0 || preSMPS[0] != flag) {
					preSMPS[0] = flag;
					sendFlag = flag & smps;
					myPs->code = M_FAIL_SMPS;
					send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
						(int)myPs->code, sendFlag);
				}
	//			myPs->code = M_FAIL_SMPS;
				myPs->signal[M_SIG_SMPS_FAULT] = P1;
				compareValue1 = 0x01;
				for( i = 0; i < 8; i++){
					compareValue2 = compareValue1 << i;
					if(( flag & compareValue2) == compareValue2){
						fault7Flag[i] = 1;
					}else{
						fault7Flag[i] = 0;
					}
				}
				chPerSMPS7 = (myPs->config.installedCh) / smpsCount;
				
				for(i = 0; i < 8; i++){
					if(fault7Flag[i] == 1){
						start = chPerSMPS7 * i;
						max = chPerSMPS7 * (i+1);
						bd = (i * chPerSMPS7) / (myPs->config.chPerBd);
						ch = (i*chPerSMPS7)%(myPs->config.chPerBd);
						k= 0;
						for(j = start; j< max; j++){
							if((myPs->config.chPerBd * bd + ch)
								> (myPs->config.installedCh-1)){
								continue;
							}
							if(myData->bData[bd].cData[ch+k].op.state == C_RUN
								&& myData->bData[bd].cData[ch+k]
								.signal[C_SIG_SMPS_FAULT] == P0){
								myData->bData[bd].cData[ch+k]
									.signal[C_SIG_SMPS_FAULT] = P1;
								if(myData->bData[bd].cData[ch]
									.ChAttribute.chNo_master == P0) {
										myData->bData[bd].cData[ch+k-1]
											.signal[C_SIG_SMPS_FAULT] = P1;
								} else if(myData->bData[bd].cData[ch]
											.ChAttribute.opType == P1) {
										myData->bData[bd].cData[ch+k+1]
											.signal[C_SIG_SMPS_FAULT] = P1;
								}	
							}
							k++;
						}
					}
				}
			//2,1,2. 나누어 떨어지지 않을때(Shutdown)
			}else{		
				if((flag & smps) != 0) {//smps fault 7.5V
					if(myPs->signal[M_SIG_SMPS_FAULT] == P0 || preSMPS[0] != flag) {
						preSMPS[0] = flag;
						sendFlag = flag & smps;
						if((inb(0x602) & 0x10) == P0) {
							myPs->signal[M_SIG_RUN_LED] = P0;
							myPs->signal[M_SIG_REMOTE_SMPS1] = P0;
							myPs->signal[M_SIG_FAN_RELAY] = P0;
							myPs->signal[M_SIG_POWER_OFF] = P1;
							myPs->code = M_FAIL_SMPS;
							send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 7, sendFlag);
						}
						myPs->code = M_FAIL_SMPS;
						myPs->signal[M_SIG_SMPS_FAULT] = P1;
						send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
							(int)myPs->code, sendFlag);
						for(bd=0; bd < myPs->config.installedBd; bd++) {
							for(ch=0; ch < myPs->config.chPerBd; ch++) {
								if((myPs->config.chPerBd * bd + ch)
									> (myPs->config.installedCh-1)) {
									continue;
								}	
							if(myData->bData[bd].cData[ch].op.state == C_RUN
								&& myData->bData[bd].cData[ch]
								.signal[C_SIG_SMPS_FAULT] == P0) {
								myData->bData[bd].cData[ch]
									.signal[C_SIG_SMPS_FAULT] = P1;
								}
							}
						}
					}
				}else {
					myPs->signal[M_SIG_SMPS_FAULT] = P0;
					preSMPS[0] = flag;
				}
			}
		//7VSmps(new)
		} else {
			myPs->signal[M_SIG_SMPS_FAULT] = P0;
		}
	}	
	//3. 3.3V Main SMPS Fault check
	base_addr = myPs->addr.interface[IO_EXPEND][4];
	if(base_addr){
		addr = base_addr;
		flag = inb(addr);
		smps = myPs->fault.SMPS3_3V[0];
		//3,1. 3.3V smps fail
		if((flag & smps) != 0) {//smps fault 3.3V
			compareValue1 = 0x01;
			smpsCount = 0;
			for( i = 0; i < 8; i++){
				compareValue2 = compareValue1 << i;
				if((compareValue2 & smps) == compareValue2){
					smpsCount++;
				}
			}
			type = myPs->config.installedCh % smpsCount;
			//3,1,1. 나누어 떨어질때(new)
			if( type == 0){	
				if(myPs->signal[M_SIG_SMPS_FAULT1] == P0 || preSMPS[1] != flag) {						
					preSMPS[1] = flag;
					sendFlag = (flag & smps) << 8;
					myPs->code = M_FAIL_SMPS;
					send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
							(int)myPs->code, sendFlag);
				}
//				myPs->code = M_FAIL_SMPS;
				myPs->signal[M_SIG_SMPS_FAULT1] = P1;

				compareValue1 = 0x01;
				for( i = 0; i < 8; i++){
					compareValue2 = compareValue1 << i;
					if(( flag & compareValue2) == compareValue2){
						fault7Flag[i] = 1;
					}else{
						fault7Flag[i] = 0;
					}
				}
				chPerSMPS3 = (myPs->config.installedCh) / smpsCount;
				
				for(i = 0; i < 8; i++){
					if(fault7Flag[i] == 1){
						start = chPerSMPS3 * i;
						max = chPerSMPS3 * (i+1);
						bd = (i * chPerSMPS3) / (myPs->config.chPerBd);
						ch = (i*chPerSMPS3)%(myPs->config.chPerBd);
						k= 0;
						for(j = start; j< max; j++){
							if((myPs->config.chPerBd * bd + ch)
								> (myPs->config.installedCh-1)){
								continue;
							}
							if(myData->bData[bd].cData[ch+k].op.state == C_RUN
								&& myData->bData[bd].cData[ch+k]
								.signal[C_SIG_SMPS_FAULT] == P0){
								myData->bData[bd].cData[ch+k]
									.signal[C_SIG_SMPS_FAULT] = P1;
								if(myData->bData[bd].cData[ch]
									.ChAttribute.chNo_master == P0) {
										myData->bData[bd].cData[ch+k-1]
											.signal[C_SIG_SMPS_FAULT] = P1;
								} else if(myData->bData[bd].cData[ch]
											.ChAttribute.opType == P1) {
										myData->bData[bd].cData[ch+k+1]
											.signal[C_SIG_SMPS_FAULT] = P1;
								}	
							}
							k++;	
						}
					}
				}
			//3,1,2. 나누어 떨어지지 안을때
			}else{		     
				addr = base_addr;
				flag = inb(addr);
				smps = myPs->fault.SMPS3_3V[0];
				if((flag & smps) != 0) { //smps fault 3.3V
					if(myPs->signal[M_SIG_SMPS_FAULT1] == P0 || preSMPS[1] != flag) {
						preSMPS[1] = flag;
						sendFlag = (flag & smps) << 8;
						if((inb(0x602) & 0x10) == P0) {
							myPs->signal[M_SIG_RUN_LED] = P0;
							myPs->signal[M_SIG_REMOTE_SMPS1] = P0;
							myPs->signal[M_SIG_FAN_RELAY] = P0;
							myPs->signal[M_SIG_POWER_OFF] = P1;
							send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 8, sendFlag);
						}
						myPs->code = M_FAIL_SMPS;
						myPs->signal[M_SIG_SMPS_FAULT1] = P1;
						send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
							(int)myPs->code, sendFlag);
						for(bd=0; bd < myPs->config.installedBd; bd++) {
							for(ch=0; ch < myPs->config.chPerBd; ch++) {
								if((myPs->config.chPerBd * bd + ch)
									> (myPs->config.installedCh-1)) {
								continue;
								}
								if(myData->bData[bd].cData[ch].op.state == C_RUN
									&& myData->bData[bd].cData[ch]
									.signal[C_SIG_SMPS_FAULT] == P0) {
										myData->bData[bd].cData[ch]
										.signal[C_SIG_SMPS_FAULT] = P1;
								}
							}
						}
					}	
				} else {
					myPs->signal[M_SIG_SMPS_FAULT1] = P0;
					preSMPS[1] = flag;
				}
			}
		//3.3V SMPS Fail
		} else {
			myPs->signal[M_SIG_SMPS_FAULT1] = P0;
		}
	}
}

//20181115 Module Cycler 32ch OT 16 - 20 Add 
void Fault_Check_11_OT_20(void)
{
	int bd, ch;
	int	addr, base_addr;
	unsigned short ot, flag, sendFlag = 0, compareValue1, compareValue2;
	static unsigned short preOT[1]={0};
	int chPerOT;
	int otCount;
	int otFlag[8] = {0};
	int max, i, j, k, type;
	int start;
	
	int otShutDownFlag = 1;	

	if(myData->dio.delayTimer < myData->dio.config.dioDelay) return;
	if(myPs->signal[M_SIG_FAN_RELAY] == P0) return;

	// 140123 oys modify : OT Fault
	// 1. OT Fault check
	base_addr = myPs->addr.interface[IO_EXPEND][2];
	if(base_addr){
		// addr 0x610
		addr = base_addr;
		flag = inb(addr);
		ot = myPs->fault.OT[2];

		//1,1. OT Fault 
		if((flag & ot) != ot) { //OT fault
			compareValue1 = 0x01;
			otCount = 0;
			for(i=0; i<8; i++){
				compareValue2 = compareValue1 << i;
				if((compareValue2 & ot) == compareValue2){
					otCount++;
				}
			}
			if(myPs->config.installedCh >= otCount){
				type = myPs->config.installedCh % otCount;
			}else{
				type = otCount % myPs->config.installedCh;
			}

			if(otShutDownFlag == P1)
				type = 1;

			//1,1,1. 나누어 떨어질때
			if(type == 0){
				// installedCh >= OT SIGNAL
				if(myPs->config.installedCh >= otCount){
					if(myPs->signal[M_SIG_OT_FAULT] == P0 || preOT[0] != flag ) {
						preOT[0] = flag;
						//100908 oys OT signal send
						sendFlag = ~flag & ot;
						myPs->code = M_FAIL_OT;
						send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
							(int)myPs->code, sendFlag);
					}
					myPs->signal[M_SIG_OT_FAULT] = P1;
					compareValue1 = 0x01;
					for(i=0; i<8; i++){
						compareValue2 = compareValue1 << i;
						if((flag & compareValue2) == compareValue2){
							otFlag[i] = 0;
						}else{
							otFlag[i] = 1;
						}
					}
					chPerOT = (myPs->config.installedCh) / otCount;
					for(i=0; i<otCount; i++){
						if(otFlag[i] == 1){
							start = chPerOT * i;
							max = chPerOT * (i+1);
							bd = (i * chPerOT) / (myPs->config.chPerBd);
							ch = (i * chPerOT) % (myPs->config.chPerBd);
							k = 0;
							for(j=start; j<max; j++){
								if((myPs->config.chPerBd * bd + ch)
									> (myPs->config.installedCh-1)){
									continue;
								}
								if(myData->bData[bd].cData[ch+k].op.state == C_RUN
									&& myData->bData[bd].cData[ch+k]
									.signal[C_SIG_OT_FAULT] == P0){
									myData->bData[bd].cData[ch+k]
									.signal[C_SIG_OT_FAULT] = P1;
									if(myData->bData[bd].cData[ch]
										.ChAttribute.chNo_master == P0) {
											myData->bData[bd].cData[ch+k-1]
												.signal[C_SIG_OT_FAULT] = P1;
									} else if(myData->bData[bd].cData[ch]
												.ChAttribute.opType == P1) {
											myData->bData[bd].cData[ch+k+1]
												.signal[C_SIG_OT_FAULT] = P1;
									}	
								}
								k++;	
							}
						}
					}
				}else{ //installedCh < OT SIGNAL
					if(myPs->signal[M_SIG_OT_FAULT] == P0 || preOT[0] != flag ) {
						preOT[0] = flag;
						//100908 oys OT signal send
						sendFlag = ~flag & ot;
						myPs->code = M_FAIL_OT;
						send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
							(int)myPs->code, sendFlag);
					}
					myPs->signal[M_SIG_OT_FAULT] = P1;
					compareValue1 = 0x01;
					for(i=0; i<8; i++){
						compareValue2 = compareValue1 << i;
						if((flag & compareValue2) == compareValue2){
							otFlag[i] = 0;
						}else{
							otFlag[i] = 1;
						}
					}
					chPerOT = otCount / myPs->config.installedCh;
					for(i=0; i<otCount; i++){
						if(otFlag[i] == 1){
							bd = 0;
							ch = i / chPerOT;
							if(myData->bData[bd].cData[ch].op.state == C_RUN
								&& myData->bData[bd].cData[ch]
								.signal[C_SIG_OT_FAULT] == P0){
								myData->bData[bd].cData[ch]
								.signal[C_SIG_OT_FAULT] = P1;
								if(myData->bData[bd].cData[ch]
									.ChAttribute.chNo_master == P0) {
										myData->bData[bd].cData[ch-1]
											.signal[C_SIG_OT_FAULT] = P1;
								} else if(myData->bData[bd].cData[ch]
											.ChAttribute.opType == P1) {
										myData->bData[bd].cData[ch+1]
											.signal[C_SIG_OT_FAULT] = P1;
								}	
							}
						}
					}	
				}	
			//1,1,2. 나누어 떨어지지 않을때(Shutdown)
			}else{
				if(myPs->signal[M_SIG_OT_FAULT] == P0 || preOT[0] != flag ) {
					preOT[0] = flag;
					//100908 oys OT signal send
					sendFlag = ~flag & ot;
					if((inb(0x602) & 0x10) == P0) {
						myPs->signal[M_SIG_RUN_LED] = P0;
						myPs->signal[M_SIG_REMOTE_SMPS1] = P0;
						myPs->signal[M_SIG_FAN_RELAY] = P0;
						myPs->signal[M_SIG_POWER_OFF] = P1;
						send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 6, sendFlag);
					}
					myPs->code = M_FAIL_OT;
					myPs->signal[M_SIG_OT_FAULT] = P1;
					send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
						(int)myPs->code, sendFlag);
					for(bd=0; bd < myPs->config.installedBd; bd++) {
						for(ch=0; ch < myPs->config.chPerBd; ch++) {
							if((myPs->config.chPerBd * bd + ch)
								>= myPs->config.installedCh) {
								continue;
							}
							if(myData->bData[bd].cData[ch].op.state == C_RUN
								&& myData->bData[bd].cData[ch]
								.signal[C_SIG_OT_FAULT] == P0) {
								myData->bData[bd].cData[ch]
								.signal[C_SIG_OT_FAULT] = P1;
							}
						}
					}
				}else{
					myPs->signal[M_SIG_OT_FAULT] = P0;
					preOT[0] = flag;
					otFlag[0] = 0;
				}
			}
		//OT FAULT
		} else {
			myPs->signal[M_SIG_OT_FAULT] = P0;
		}
	}
}

///// 120207 oys w : for TOSHIBA VS Compare BD /////////////////////////////////

void Fault_Check_12(void)
{
	int bd, ch, i;
	int	addr, base_addr;
	int sendFlag;
	unsigned short flag, flag1,flag2;

	if(myData->dio.delayTimer < myData->dio.config.dioDelay) return;
	if(myPs->signal[M_SIG_FAN_RELAY] == P0) return;

	// fault Check
	for(i=0; i < 6; i++){
		base_addr = myPs->addr.interface[IO_EXPEND][7+i];
		if(base_addr){
			addr = base_addr;
			flag = inb(addr);
			if(myPs->config.chPerBd > 8) {
				flag = flag + (inb(addr + 0x40) << 8);
			}
			if(flag != 0x00) {
				bd = 0;
				flag1 = 0x01;
				for(ch=0; ch < myPs->config.chPerBd; ch++) {
					if((flag & flag1) != 0x00){
						if(myData->bData[bd].cData[ch].op.state == C_RUN
							&& myData->bData[bd].cData[ch]
							.signal[C_SIG_VL_PLUS_FAULT+i] == P0) {
							myData->bData[bd].cData[ch]
								.signal[C_SIG_VL_PLUS_FAULT+i] = P1;
							if(myData->bData[bd].cData[ch]
								.ChAttribute.chNo_master == P0) {
									myData->bData[bd].cData[ch-1]
										.signal[C_SIG_VL_PLUS_FAULT+i] = P1;
							} else if(myData->bData[bd].cData[ch]
								.ChAttribute.opType == P1) {
									myData->bData[bd].cData[ch+1]
										.signal[C_SIG_VL_PLUS_FAULT+i] = P1;
							}	
						}
					}
					flag1 = (flag1 << 0x01);
				}
			}
		}
	}
	// METER_HIGH fault
	sendFlag = 0;
	base_addr = myPs->addr.interface[IO_EXPEND][13];
	if(base_addr){
		addr = base_addr;
		flag = inb(addr);
		if(myPs->config.chPerBd > 8) {
			flag = flag + (inb(addr + 0x40) << 8);
		}
		if(flag != 0x00) {
			myPs->signal[M_SIG_RUN_LED] = P0;
			myPs->signal[M_SIG_REMOTE_SMPS1] = P0;
			myPs->signal[M_SIG_FAN_RELAY] = P0;
			myPs->signal[M_SIG_POWER_OFF] = P1;
			myPs->code = M_FAIL_PANEL_METER_HIGH_ERROR;
			flag2 = 0x01;
			for(i=0;i<myPs->config.chPerBd;i++) {
				if(flag & flag2) {
					sendFlag += i+1;
				}
				flag2 = flag2 << 1;
			}
			send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 15, sendFlag);
			send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
				(int)myPs->code, sendFlag);
			for(bd=0; bd < myPs->config.installedBd; bd++) {
				for(ch=0; ch < myPs->config.chPerBd; ch++) {
					if((myPs->config.chPerBd * bd + ch)
						>= myPs->config.installedCh) {
					continue;
					}
					if(myData->bData[bd].cData[ch].op.state == C_RUN
						&& myData->bData[bd].cData[ch]
						.signal[C_SIG_METER_HIGH_FAULT] == P0) {
						myData->bData[bd].cData[ch]
							.signal[C_SIG_METER_HIGH_FAULT] = P1;
					}
				}
			}
			return;
		}
	}
	// METER_LOW fault
	base_addr = myPs->addr.interface[IO_EXPEND][14];
	if(base_addr){
		addr = base_addr;
		flag = inb(addr);
		if(myPs->config.chPerBd > 8) {
			flag = flag + (inb(addr + 0x40) << 8);
		}
		if(flag != 0x00) {
			myPs->signal[M_SIG_RUN_LED] = P0;
			myPs->signal[M_SIG_REMOTE_SMPS1] = P0;
			myPs->signal[M_SIG_FAN_RELAY] = P0;
			myPs->signal[M_SIG_POWER_OFF] = P1;
			myPs->code = M_FAIL_PANEL_METER_LOW_ERROR;
			flag2 = 0x01;
			for(i=0;i<myPs->config.chPerBd;i++) {
				if(flag & flag2) {
					sendFlag += i+1;
				}
				flag2 = flag2 << 1;
			}
			send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 16, sendFlag);
			send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
				(int)myPs->code, sendFlag);
			for(bd=0; bd < myPs->config.installedBd; bd++) {
				for(ch=0; ch < myPs->config.chPerBd; ch++) {
					if((myPs->config.chPerBd * bd + ch)
						>= myPs->config.installedCh) {
					continue;
					}
					if(myData->bData[bd].cData[ch].op.state == C_RUN
						&& myData->bData[bd].cData[ch]
						.signal[C_SIG_METER_LOW_FAULT] == P0) {
						myData->bData[bd].cData[ch]
							.signal[C_SIG_METER_LOW_FAULT] = P1;
					}
				}
			}
			return;
		}
	}
}
//20171024 sch add
/*
void Fault_Check_default_LG(void)
{
	int rangeV, rangeI, bd, ch, diff;
	long maxV, maxI, SensI = 0, LimitI = 0;
		
	S_CH_STEP_INFO step;

	rangeV = step.rangeV;
	rangeI = step.rangeI;
	maxV = myPs->config.maxVoltage[rangeV];
	maxI = myPs->config.maxCurrent[rangeI];
	

//	if(VENDER != 1)	return;
	if(USER_VI != 1) return;

	if(myData->dio.delayTimer < myData->dio.config.dioDelay){ 
		for(bd=0; bd < myPs->config.installedBd; bd++) {
			for(ch=0; ch < myPs->config.chPerBd; ch++) {
				if((myPs->config.chPerBd * bd + ch)
					>= myPs->config.installedCh) {
				continue;
				}
					myData->bData[bd].cData[ch].misc.limit_current_timeout
						= myData->bData[bd].cData[ch].op.runTime;
			}
		}
			return;
	}

	if(myPs->config.LimitVI.limit_use_I != 0){
		for(bd = 0; bd < myPs->config.installedBd; bd++){
			for(ch = 0; ch < myPs->config.installedCh; ch++){
				if(myData->bData[bd].cData[ch].ChAttribute.chNo_master == P0){
					SensI = myData->bData[bd].cData[ch].op.Isens
							+ myData->bData[bd].cData[ch-1].op.Isens;
					LimitI = myPs->config.LimitVI.limit_current - maxI*0.004;
				}else{
					SensI = myData->bData[bd].cData[ch].op.Isens;
					LimitI = myPs->config.LimitVI.limit_current - maxI*0.002;
				}
				if(SensI >= LimitI){
					diff = myData->bData[bd].cData[ch].op.runTime
					- myData->bData[bd].cData[ch].misc.limit_current_timeout;
					if(diff >= myPs->config.LimitVI.limit_time_I){
						myData->bData[bd].cData[ch].misc.limit_current_timeout
						= myData->bData[bd].cData[ch].op.runTime;
						if(myPs->config.LimitVI.limit_Ch_I == P0){
							if(myData->bData[bd].cData[ch].op.state == C_RUN
								&& myData->bData[bd].cData[ch]
								.signal[C_SIG_LIMIT_CURRENT] == P0) {
								myData->bData[bd].cData[ch]
								.signal[C_SIG_LIMIT_CURRENT] = P1;
							}
						}else if(myPs->config.LimitVI.limit_Ch_I == P1){
							for(bd=0; bd < myPs->config.installedBd; bd++) {
								for(ch=0; ch < myPs->config.chPerBd; ch++) {
									if((myPs->config.chPerBd * bd + ch)
										>= myPs->config.installedCh) {
									continue;
									}
									if(myData->bData[bd].cData[ch].op.state 
										== C_RUN && myData->bData[bd].cData[ch]
										.signal[C_SIG_LIMIT_CURRENT] == P0) {
										myData->bData[bd].cData[ch]
											.signal[C_SIG_LIMIT_CURRENT] = P1;
									}
								}
							}
			//				return;
						}
					}
				}else{
					myData->bData[bd].cData[ch].misc.limit_current_timeout
						= myData->bData[bd].cData[ch].op.runTime;
				}
			}
		}
	//	return;
	}
	if(myPs->config.LimitVI.limit_use_V != 0){
		for(bd = 0; bd < myPs->config.installedBd; bd++){
			for(ch = 0; ch < myPs->config.installedCh; ch++){
				if(myData->bData[bd].cData[ch].op.Vsens >=
						myPs->config.LimitVI.limit_voltage){
					if(myPs->config.LimitVI.limit_Ch_V == P0){
						if(myData->bData[bd].cData[ch].op.state == C_RUN
							&& myData->bData[bd].cData[ch]
							.signal[C_SIG_LIMIT_VOLTAGE] == P0) {
							myData->bData[bd].cData[ch]
							.signal[C_SIG_LIMIT_VOLTAGE] = P1;
						}
					}else if(myPs->config.LimitVI.limit_Ch_V == P1){
						for(bd=0; bd < myPs->config.installedBd; bd++) {
							for(ch=0; ch < myPs->config.chPerBd; ch++) {
								if((myPs->config.chPerBd * bd + ch)
									>= myPs->config.installedCh) {
								continue;
								}
								if(myData->bData[bd].cData[ch].op.state 
									== C_RUN && myData->bData[bd].cData[ch]
									.signal[C_SIG_LIMIT_VOLTAGE] == P0) {
									myData->bData[bd].cData[ch]
										.signal[C_SIG_LIMIT_VOLTAGE] = P1;
								}
							}
						}
			//				return;
					}
				}else{
				}
			}
		}
	//	return;
	}
}
////////////////////////////////////////////////////////////////////////////////
*/
void Fault_Check_13(void)
{	//180611 add for digital
	unsigned char	flag1=0, flag2=0;
	unsigned short sendFlag;
	int rd_addr1, bd, ch, i, index=0;

	if(myData->dio.delayTimer < myData->dio.config.dioDelay) return;

	rd_addr1 = myPs->addr.interface[IO_INPUT][1];

	if(myPs->signal[M_SIG_INV_POWER] < P10
	 || myPs->signal[M_SIG_INV_POWER] >= P100) return;

	flag1 = Read_InPoint(I_IN_INV1_FLT1);
	
	for(i=0; i < myPs->pcu_config.installedInverter - 1; i++){
		index = i * 4;
	//	flag1 = flag1 | Read_InPoint(I_IN_INV2_FLT1 + index);
		flag2 |= Read_InPoint(I_IN_INV2_FLT1 + index) << (i + 1);
	}
	flag1 += flag2;
	
	if(flag1 != P0){ // INV DC Fault 
		if(myPs->signal[M_SIG_INV_FAULT] == P0) {
			myPs->signal[M_SIG_INV_FAULT] = P1;
			myPs->signal[M_SIG_RUN_LED] = P0;
			myPs->signal[M_SIG_REMOTE_SMPS1] = P0;
			myPs->signal[M_SIG_FAN_RELAY] = P0;
			myPs->signal[M_SIG_POWER_OFF] = P1;
			if(myPs->signal[M_SIG_INV_POWER] == P10){
				myPs->signal[M_SIG_INV_POWER] = P100;
				myPs->signal[M_SIG_INV_POWER1] = P100;
			}
			myPs->code = M_FAIL_INV_DC;
			sendFlag = flag1;
			printk("****** INV DC_FAULT Short off ******\n");
			send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 1, sendFlag);
			send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
				(int)myPs->code, sendFlag);
			for(bd=0; bd < myPs->config.installedBd; bd++) {
				for(ch=0; ch < myPs->config.chPerBd; ch++) {
					if((myPs->config.chPerBd * bd + ch)
						> (myPs->config.installedCh-1)) {
						continue;
					}
					if(myData->bData[bd].cData[ch].op.state == C_RUN
						&& myData->bData[bd].cData[ch]
						.signal[C_SIG_INV_FAULT] == P0) {
						myData->bData[bd].cData[ch]
							.signal[C_SIG_INV_FAULT] = P1;
					}
				}
			}
		}
		return;
	}

	flag1 = Read_InPoint(I_IN_INV1_FLT2);
	
	for(i=0; i < myPs->pcu_config.installedInverter - 1; i++){
		index = i * 4;
	//	flag1 = flag1 | Read_InPoint(I_IN_INV2_FLT2 + index);
		flag2 |= Read_InPoint(I_IN_INV2_FLT2 + index) << (i + 1);
	}

	flag1 += flag2;

	if(flag1 != P0){ // INV AC Fault
		if(myPs->signal[M_SIG_INV_FAULT] == P0) {
			printk("****** INV AC_FAULT Short off ******\n");
			myPs->signal[M_SIG_INV_FAULT] = P1;
			myPs->signal[M_SIG_REMOTE_SMPS1] = P0;
			myPs->signal[M_SIG_RUN_LED] = P0;
			myPs->signal[M_SIG_FAN_RELAY] = P0;
			myPs->signal[M_SIG_POWER_OFF] = P1;
			if(myPs->signal[M_SIG_INV_POWER] == P10){
				myPs->signal[M_SIG_INV_POWER] = P100;
				myPs->signal[M_SIG_INV_POWER1] = P100;
			}
			myPs->code = M_FAIL_INV_AC;
			sendFlag = flag1;
			send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 1, sendFlag);
			send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
				(int)myPs->code, sendFlag);
			for(bd=0; bd < myPs->config.installedBd; bd++) {
				for(ch=0; ch < myPs->config.chPerBd; ch++) {
					if((myPs->config.chPerBd * bd + ch)
						> (myPs->config.installedCh-1)) {
						continue;
					}
					if(myData->bData[bd].cData[ch].op.state == C_RUN
						&& myData->bData[bd].cData[ch]
						.signal[C_SIG_INV_FAULT] == P0) {
						myData->bData[bd].cData[ch]
							.signal[C_SIG_INV_FAULT] = P1;
					}
				}
			}
		}
		return;
	}

	flag1 = Read_InPoint(I_IN_INV1_FLT3);
	
	for(i=0; i < myPs->pcu_config.installedInverter - 1; i++){
		index = i * 4;
	//	flag1 = flag1 | Read_InPoint(I_IN_INV2_FLT2 + index);
		flag2 |= Read_InPoint(I_IN_INV2_FLT3 + index) << (i + 1);
	}
	
	flag1 += flag2;

	if(flag1 != P0){ // INV OT Fault 
		if(myPs->signal[M_SIG_INV_FAULT] == P0) {
			myPs->signal[M_SIG_INV_FAULT] = P1;
			myPs->signal[M_SIG_RUN_LED] = P0;
			myPs->signal[M_SIG_REMOTE_SMPS1] = P0;
			myPs->signal[M_SIG_FAN_RELAY] = P0;
			myPs->signal[M_SIG_POWER_OFF] = P1;
			if(myPs->signal[M_SIG_INV_POWER] == P10){
				myPs->signal[M_SIG_INV_POWER] = P100;
				myPs->signal[M_SIG_INV_POWER1] = P100;
			}
			myPs->code = M_FAIL_INV_TEMP;
			sendFlag = flag1;
			printk("****** INV OVER TEMP Short off ******\n");
			send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 1, sendFlag);
			send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
				(int)myPs->code, sendFlag);
			for(bd=0; bd < myPs->config.installedBd; bd++) {
				for(ch=0; ch < myPs->config.chPerBd; ch++) {
					if((myPs->config.chPerBd * bd + ch)
						> (myPs->config.installedCh-1)) {
						continue;
					}
					if(myData->bData[bd].cData[ch].op.state == C_RUN
						&& myData->bData[bd].cData[ch]
						.signal[C_SIG_INV_FAULT] == P0) {
						myData->bData[bd].cData[ch]
							.signal[C_SIG_INV_FAULT] = P1;
					}
				}
			}
		}
		return;
	}
}

//170105 oys add : ch step sync process
void chStepSync(int bd, int ch)
{
	int bd_1=0, ch_1=0, cnt=0, i;
	unsigned char chamberWaitFlag;

	chamberWaitFlag = myData->bData[bd].cData[ch].misc.chamberWaitFlag;
	if(chamberWaitFlag <= P1) return;
	if(chamberWaitFlag >= P20) return;

	if(chamberWaitFlag >= P10 
		&& chamberWaitFlag <= P12){
		for(i=0; i < myPs->config.installedCh; i++) {
			bd_1 = i / myPs->config.chPerBd;
			ch_1 = i % myPs->config.chPerBd;
			if(myData->bData[bd].cData[ch].misc.chGroupNo ==
				myData->bData[bd_1].cData[ch_1].misc.chGroupNo) {
				if(myData->bData[bd_1].cData[ch_1].misc.chamberWaitFlag
					<= P1){
					cnt++;
				}
			}
		}

		//20180417 modify
		if(cnt == 0) {
			myData->bData[bd].cData[ch].misc.chamberWaitFlag += P10;
		}
	}
}
//add end

#ifdef _GROUP_ERROR
void groupErrorCheck(int bd, int ch)
{
	int bd_1=0, ch_1=0, cnt=0, cnt2=0, i;
	long sum = 0;
	int sum_cnt = 0;
	unsigned long advStepNo;

	advStepNo = myData->bData[bd].cData[ch].misc.advStepNo;

	if(myData->bData[bd].cData[ch].misc.chGroupNo != 0){
		for(i=0; i < myPs->config.installedCh; i++) {
			bd_1 = i / myPs->config.chPerBd;
			ch_1 = i % myPs->config.chPerBd;
			if(myData->bData[bd].cData[ch].misc.chGroupNo ==
				myData->bData[bd_1].cData[ch_1].misc.chGroupNo) {
				cnt++;
				if(myData->bData[bd_1].cData[ch_1].misc.group_StartVoltage_flag == P1){
					cnt2++;
				}
			}
		}
		if(cnt != 0 && cnt2 == cnt) { //When all channels reach the set voltage
			for(i=0; i < myPs->config.installedCh; i++) {
				bd_1 = i / myPs->config.chPerBd;
				ch_1 = i % myPs->config.chPerBd;
				if(myData->bData[bd].cData[ch].misc.chGroupNo ==
					myData->bData[bd_1].cData[ch_1].misc.chGroupNo) {
					sum += myData->bData[bd_1].cData[ch_1].op.Vsens;
				   	//Accumulation to caculate the average voltage
					sum_cnt++; //Check the same number of chGroupNo
				}
			}
			myData->bData[bd].cData[ch].misc.groupAvgVsens = (sum / sum_cnt);
		   	//Calculate the average voltage
			for(i=0; i < myPs->config.installedCh; i++) {
				bd_1 = i / myPs->config.chPerBd;
				ch_1 = i % myPs->config.chPerBd;
				if(myData->bData[bd].cData[ch].misc.chGroupNo ==
					myData->bData[bd_1].cData[ch_1].misc.chGroupNo) {
					myData->bData[bd_1].cData[ch_1].misc.groupAvgVsens
					 = myData->bData[bd].cData[ch].misc.groupAvgVsens;
				   	//Put the average voltage value into the channel
				}
			}
		}
	}
}

void groupErrorCheck_EndTime(int bd, int ch)
{
	int bd_1=0, ch_1=0, i;

	if(myData->bData[bd].cData[ch].misc.chGroupNo != 0){
		if(myData->bData[bd].cData[ch].misc.endState == P1){
			for(i=0; i < myPs->config.installedCh; i++) {
				bd_1 = i / myPs->config.chPerBd;
				ch_1 = i % myPs->config.chPerBd;
				if(myData->bData[bd].cData[ch].misc.chGroupNo ==
					myData->bData[bd_1].cData[ch_1].misc.chGroupNo) {
					myData->bData[bd_1].cData[ch_1].misc.endState = P2;
					myData->bData[bd_1].cData[ch_1].misc.groupEndTime 
					  = myData->bData[bd].cData[ch].misc.groupEndTime; 
				}
			}
		}
	}
}
#endif

void GroupControl(void)
{
}

void GUI_TO_SBC_BUZZER_CONTROL()
{
	int bd,ch,i;
	long long flag;
	flag = 1;

	if(myData->MainClient.signal[MAIN_SIG_LAMP_BUZZER] < P1) return;

	if(myData->MainClient.signal[MAIN_SIG_LAMP_BUZZER] == P1) {
		//1.BUZZER_OFF
		myData->MainClient.signal[MAIN_SIG_LAMP_BUZZER] = P0;
		for(i=0; i < myPs->config.installedCh; i++) {
			bd = i / myPs->config.chPerBd;
			ch = i % myPs->config.chPerBd;
			if(myData->bData[bd].cData[ch].op.state == C_PAUSE) {
				if(myData->bData[bd].cData[ch].op.code != C_FAULT_PAUSE_CMD &&
				myData->bData[bd].cData[ch].op.code != C_FAULT_NEXTSTEP_CMD){
					myPs->misc.buzzerFlag1 |= flag;
				}
			}
			flag = flag << 1;
		} 
	}else if(myData->MainClient.signal[MAIN_SIG_LAMP_BUZZER] == P2) {
		myPs->misc.safety_fault_timer += myPs->misc.rt_scan_time;	
		if(myPs->misc.safety_fault_timer >= 200){		//after 2 Sec
			for(i=0; i < myPs->config.installedCh; i++) {
				bd = i / myPs->config.chPerBd;
				ch = i % myPs->config.chPerBd;
				if(myData->bData[bd].cData[ch].op.state == C_PAUSE &&
					myData->bData[bd].cData[ch].op.code == C_FAULT_PAUSE_CMD){
					//190225 add for Buuzer ON
					myData->bData[bd].cData[ch].op.code = C_FAULT_GUI_SAFETY;
				}
			}
			
			myPs->misc.safety_fault_timer = 0;
			myData->MainClient.signal[MAIN_SIG_LAMP_BUZZER] = P0;
		}
	}
}

void mInv_Signal(void)
{	//180611 add for digital
	int inv1_state=0, diff,i, index = 0;
    int flag1=0, flag3=0xff, addr1, inv_run;

	flag1 = Read_InPoint(I_IN_INV1_FLT1) && Read_InPoint(I_IN_INV1_FLT2)
			&& Read_InPoint(I_IN_INV1_FLT3);

	for(i=0; i < myPs->pcu_config.installedInverter; i++){
		index = i * 4;
		flag1 = flag1 && Read_InPoint(I_IN_INV2_FLT1 + index) 
			&& Read_InPoint(I_IN_INV2_FLT2 + index) 
			&& Read_InPoint(I_IN_INV2_FLT3 + index);
	}

	addr1 = myPs->addr.interface[IO_INPUT][1];
	
	if(flag1 == P0){
		inv1_state = 0;
	}else{
		inv1_state = 1;
		flag3 = flag3 & inb(addr1); 
	}
	inv_run = M_SIG_INV1_RUN;

	switch(myPs->signal[M_SIG_INV_POWER]){
		case P0:
			myPs->misc.invertFailWait = myPs->misc.timer_1sec;
//			myPs->signal[M_SIG_INV_POWER]++;
			break;
		case P1:
			for(i=0; i < myPs->pcu_config.installedInverter; i++){
				myPs->signal[inv_run + i] = P1;
			}
			myPs->misc.invertFailWait = myPs->misc.timer_1sec;
			myPs->signal[M_SIG_INV_POWER]++;
			break;
		case P2:
			diff = myPs->misc.timer_1sec - myPs->misc.invertFailWait;
			if(diff < 20) break;
			myPs->misc.invertFailWait = myPs->misc.timer_1sec;
			myPs->signal[M_SIG_INV_POWER]++;
			break;
		case P3:
			diff = myPs->misc.timer_1sec - myPs->misc.invertFailWait;
			if(diff < 15) break;
			myPs->misc.invertFailWait = myPs->misc.timer_1sec;
			myPs->signal[M_SIG_INV_POWER] = P10;
			break;
		case P10:
			if(inv1_state == 1){
				if(myData->dio.signal[DIO_SIG_POWER_FAIL] == P0
					&&  myData->dio.signal[DIO_SIG_MAIN_EMG] == P0){
					myPs->code = M_FAIL_INV;
					myPs->signal[M_SIG_INV_POWER] = P100;
					send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 0, flag3);
					send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
						(int)myPs->code, flag3);
				}
			}
			break;
		case P100:
			for(i=0; i < myPs->pcu_config.installedInverter; i++){
				myPs->signal[inv_run + i] = P0;
			}
			myPs->signal[M_SIG_INV_POWER]++;
			break;
		case P101:
			myPs->signal[M_SIG_INV_POWER]++;
			break;
		default:
			break;
	}
}

void PCU_INV_Signal(void)
{	//180620 add for digital
	int i ,bd, diff , cnt, invCh = 0;
	unsigned char useFlag = 0;
	
	bd = 0;
	
	switch(myPs->signal[M_SIG_INV_POWER1]){
		case P1:
			for(i=0; i < myPs->config.chPerBd ; i++){
				myData->bData[bd].cData[i].inv_power = 0;
			}
			myPs->misc.invertFailWait = myPs->misc.timer_1sec;
			myPs->signal[M_SIG_INV_POWER1]++;
			break;
		case P2:
			diff = myPs->misc.timer_1sec - myPs->misc.invertFailWait;
			if(diff < 5) break;
			myPs->signal[M_SIG_INV_POWER1]++;
			break;
		case P3:
			for(i=0; i < myPs->pcu_config.installedInverter; i++){
				useFlag = myData->dio.din.pcu_inUseFlag[i];
				if(useFlag == P1){
					invCh = i * myPs->pcu_config.invPerCh;
					myData->bData[bd].cData[invCh].inv_power = P1;
				}
			}
			myPs->misc.invertFailWait = myPs->misc.timer_1sec;
			myPs->signal[M_SIG_INV_POWER1]++;
			break;
		case P4:
			cnt = 0;
			for(i=0; i < myPs->pcu_config.installedInverter; i++){
				invCh = i * myPs->pcu_config.invPerCh;
				if(myData->bData[bd].cData[invCh].inv_power == P10){
					cnt++;
				}
			}
			if(cnt == myPs->pcu_config.installedInverter){
				myPs->signal[M_SIG_INV_POWER1] = P10;
			}
			
			diff = myPs->misc.timer_1sec - myPs->misc.invertFailWait;
			if(diff >= 40){
				myPs->signal[M_SIG_INV_POWER1] = P10;
			}
		//	myPs->signal[M_SIG_INV_POWER1]++;
			break;
		case P10:
			break;
		case P100:
			for(i=0; i < myPs->pcu_config.installedInverter; i++){
				useFlag = myData->dio.din.pcu_inUseFlag[i];
				if(useFlag == P1){
					invCh = i * myPs->pcu_config.invPerCh;
					myData->bData[bd].cData[invCh].inv_power = P100;
				}
			}
			myPs->signal[M_SIG_INV_POWER1] = P101;
			break;
		case P101:
			cnt = 0;
			for(i=0; i < myPs->pcu_config.installedInverter; i++){
				useFlag = myData->dio.din.pcu_inUseFlag[i];
				if(useFlag == P1){
					invCh = i * myPs->pcu_config.invPerCh;
					//191024 add
					if(myData->bData[bd].cData[invCh].inv_power == P99){
						myData->bData[bd].cData[invCh].inv_power = P0;
					}

					if(myData->bData[bd].cData[invCh].inv_power == P0){
						cnt++;
					}
				}
			}
			if(cnt == myPs->pcu_config.installedInverter){
				myPs->signal[M_SIG_INV_POWER1] = P0;
			}
			break;
		default:
			break;
	}
}

void Inverter_Signal_CAN(void)//CAN Type
{
   	int inv_state, inv, installedInv;
	int i, bd, ch;
//	int start, end;
	long diff = 0.0;
	char tmp_val = 0, inv_cmd = 0;
	
	if(myData->CAN.config.canPort[INV_CAN_PORT] == 0) return;
	
	installedInv = myData->CAN.config.installedInverter;
	
	if(myPs->signal[M_SIG_REMOTE_SMPS1] == P1){
		if(myPs->signal[M_SIG_INV_POWER_CAN] == P0){
			myPs->signal[M_SIG_INV_POWER_CAN] = P1;
		}
	}
	
	if(myPs->signal[M_SIG_REMOTE_SMPS1] == P0){
		if(myPs->signal[M_SIG_INV_POWER_CAN] == P10){
			myPs->signal[M_SIG_INV_POWER_CAN] = P100;
		}
	}
	
	for(inv = 0; inv < installedInv; inv++){
		if(myData->AppControl.config.debugType != P0) break;
		if(myData->CAN.inverter[inv].signal[CAN_SIG_INV_CAN_COMM_ERROR] == P1){
			if(myPs->code == M_FAIL_MAIN_EMG){
				myPs->signal[M_SIG_INV_POWER_CAN] = P100;
				myData->CAN.inverter[inv]
						.signal[CAN_SIG_INV_CAN_COMM_ERROR] = P2;
				//P0->P2 Send trouble only one
			}else{
				myData->CAN.inverter[inv].code = M_FAIL_INV;
				myPs->code = myData->CAN.inverter[inv].code;
				send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 34, inv);
				send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS, myPs->code, inv);
				myPs->signal[M_SIG_INV_POWER_CAN] = P100;
				myData->CAN.inverter[inv]
						.signal[CAN_SIG_INV_CAN_COMM_ERROR] = P2;
				//P0->P2 Send trouble only one
			}
			return;
		}
	}	

	switch(myPs->signal[M_SIG_INV_POWER_CAN]){
		case P0:	//inverter Error state
			myData->CAN.inv_run_delay = myPs->misc.timer_1sec;
			break;
		case P1:	//send inverter rest command
			for(inv = 0; inv < installedInv; inv++){
				tmp_val = INV_CMD_RESET;
				myData->CAN.inverter[inv].sendCmd = INV_CMD_RESET;
				inv_cmd |= (tmp_val << (inv * 2));
			}
			//send inverter reset command
			can_inv_send_cmd(inv_cmd, inv);
			myPs->signal[M_SIG_INV_POWER_CAN]++;
			break;
		case P2:	//wait reset delay process time
			myData->CAN.inv_run_delay = myPs->misc.timer_1sec;
			myPs->signal[M_SIG_INV_POWER_CAN]++;
			break;
		case P3:	//send inverter run command
			diff = myPs->misc.timer_1sec - myData->CAN.inv_run_delay;
			if(diff < (myData->dio.config.dioDelay / 1000)) break;
			myData->CAN.inv_run_delay = myPs->misc.timer_1sec;

			for(inv = 0; inv < installedInv; inv++){
				//save last cmd
				tmp_val = INV_CMD_RUN;
				myData->CAN.inverter[inv].sendCmd = INV_CMD_RUN;
				inv_cmd |= (tmp_val << (inv * 2));
			}
			//send inverter run command
			can_inv_send_cmd(inv_cmd ,inv);
			myPs->signal[M_SIG_INV_POWER_CAN] = P4;
			break;
		case P4:
			//210316 add for ch Code 
			for(i = 0; i < myPs->config.installedCh; i++){
				bd = myData->CellArray1[i].bd;
				ch = myData->CellArray1[i].ch;
				myData->bData[bd].cData[ch].op.code = P_INV_STANDBY;
			}
			myPs->signal[M_SIG_INV_POWER_CAN] = P5;
			break;
		case P5:
			diff = myPs->misc.timer_1sec - myData->CAN.inv_run_delay;
	//		if(diff < (myData->dio.config.dioDelay / 1000)) break;
			if(diff < 10) break;

			if(myData->AppControl.config.debugType != P0){
				myPs->signal[M_SIG_INV_POWER_CAN] = P6;
				break;
			}

			//inverter state is ok
			inv_state = P1;
			/*
			for(inv = 0; inv < installedInv; inv++){
				if(myData->CAN.inverter[inv].state.steady != 0x1){
					//inverter state is not ok
					inv_state = P0;
					myData->CAN.inverter[inv].code = M_FAIL_INV;
					myPs->code = myData->CAN.inverter[inv].code;
					send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 34, inv);
					send_msg(MODULE_TO_MAIN, 
							MSG_MODULE_MAIN_EMG_STATUS, myPs->code, inv);
					//send inv fault signal
					myData->CAN.invsavefault[inv].code
						= myData->CAN.inverter[inv].faultFull.all;
					myData->CAN.invsavefault[inv].step
						= myPs->signal[M_SIG_INV_POWER_CAN];
					myData->CAN.invsavefault[inv].vdc
						= myData->CAN.inverter[inv].vdc;

					//Channel code change
					if(inv == 0) {
						start = 0;
						end = myData->CAN.config.chInInv[inv];
					}else{
						start = myData->CAN.config.chInInv[inv-1] * inv;
						end = myData->CAN.config.chInInv[inv] + start;
					}
					for(i= start; i < end; i++){
						bd = i / myData->mData.config.chPerBd;
						ch = i % myData->mData.config.chPerBd;
						if(myData->bData[bd].cData[ch].op.state == C_RUN) {
							myData->bData[bd].cData[ch]
								.signal[C_SIG_INV_FAULT] = P1;
						}
					}
					myPs->signal[M_SIG_INV_POWER_CAN] = P100;
				}
			}*/

			if(inv_state == P1){	//inverter is steady
				myPs->signal[M_SIG_INV_POWER_CAN] = P6;
			}
			break;
		case P6:
			//210316 add for ch tmpCode
			for(i = 0; i < myPs->config.installedCh; i++){
				bd = myData->CellArray1[i].bd;
				ch = myData->CellArray1[i].ch;

				myData->bData[bd].cData[ch].op.code 
						= myData->bData[bd].cData[ch].misc.tmpCode;
			}
			myPs->signal[M_SIG_INV_POWER_CAN] = P10;
			break;
		case P10:
			//inverter state is ok
			inv_state = P0;
			if(myData->AppControl.config.debugType != P0)	break;

			for(inv = 0; inv < installedInv; inv++){
				if(myData->CAN.inverter[inv].state.fail == 0x1){
					inv_state = P1; //inverter state is not ok
					if(myData->dio.signal[DIO_SIG_POWER_FAIL] == P0
						&& myData->dio.signal[DIO_SIG_MAIN_EMG] == P0){
						myData->CAN.inverter[inv].code = M_FAIL_INV;
						myPs->code = myData->CAN.inverter[inv].code;
						send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 34, inv);
						send_msg(MODULE_TO_MAIN, 
							MSG_MODULE_MAIN_EMG_STATUS, myPs->code, inv);
						//send inv fault signal
						myData->CAN.invsavefault[inv].code
							= myData->CAN.inverter[inv].faultFull.all;
						myData->CAN.invsavefault[inv].step
							= myPs->signal[M_SIG_INV_POWER_CAN];
						myData->CAN.invsavefault[inv].vdc
							= myData->CAN.inverter[inv].vdc;

						//Channel code change
						/*
						if(inv == 0) {
							start = 0;
							end = myData->CAN.config.chInInv[inv];
						}else{
							start = myData->CAN.config.chInInv[inv-1] * inv;
							end = myData->CAN.config.chInInv[inv] + start;
						}
						for(i=start; i < end; i++){
							bd = i / myData->mData.config.chPerBd;
							ch = i % myData->mData.config.chPerBd;
							if(myData->bData[bd].cData[ch].op.state == C_RUN) {
								myData->bData[bd].cData[ch]
									.signal[C_SIG_INV_FAULT] = P1;
							}
						}*/
					}
				//	myPs->signal[M_SIG_INV_POWER_CAN] = P100;
					myPs->signal[M_SIG_INV_POWER_CAN] = P99;
				}
			}
			break;
		case P99:	//inverter Fault stay State
			break;
		case P100://inverter stop
			for(inv = 0; inv < installedInv; inv++){
				tmp_val = INV_CMD_STOP;
				myData->CAN.inverter[inv].sendCmd = INV_CMD_STOP;
				inv_cmd |= (tmp_val << (inv * 2));
			}
			can_inv_send_cmd(inv_cmd, inv);	//send inverter stop 
			myPs->signal[M_SIG_INV_POWER_CAN]++;
			break;
		case P101:
			for(inv = 0; inv < installedInv; inv++){
				tmp_val = INV_CMD_RESET;
				myData->CAN.inverter[inv].sendCmd = INV_CMD_RESET;
				inv_cmd |= (tmp_val << (inv * 2));
			}
			can_inv_send_cmd(inv_cmd, inv);	//send inverter reset 
			myPs->signal[M_SIG_INV_POWER_CAN]++;
			break;
		case P102:
			myPs->signal[M_SIG_INV_POWER_CAN] = P0;
			break;
		default:
			break;
	}
}

void GUI_cmd_Buzzer_stop()
{
	int bd,ch,i;
	long long flag;
	flag = 1;
	if(myPs->signal[M_SIG_LAMP_BUZZER] == P2) {
		for(i=0; i < myPs->config.installedCh; i++) {
			bd = i / myPs->config.chPerBd;
			ch = i % myPs->config.chPerBd;
			if(myData->bData[bd].cData[ch].op.state == C_PAUSE) {
				if(myData->bData[bd].cData[ch].op.code != C_FAULT_PAUSE_CMD &&
					myData->bData[bd].cData[ch].op.code != C_FAULT_NEXTSTEP_CMD){
					myPs->misc.buzzerFlag1 |= flag; 
				}
			}
			flag = flag << 1;
		}	
	}
}

void mainState_Connection_Check(void)
{
	int bd,ch,i;
	int idx,sendNum;
	int Day;
	
	if(myData->mData.config.function[F_DISCONNECT_DAY] == P0) return;
	if(myData->mData.mainStateCheckFlag == P0){
		myPs->misc.disconnect_timer = myPs->misc.timer_1sec;
		return;	
	}

	Day = myData->mData.config.function[F_DISCONNECT_DAY];

	if(myData->mData.mainStateCheckFlag == P1){
		if(((myPs->misc.timer_1sec - myPs->misc.disconnect_timer) / (ONE_DAY_RUNTIME / 100)) >= Day){
			for(i=0; i < myPs->config.installedCh; i++) {
				bd = i / myPs->config.chPerBd;
				ch = i % myPs->config.chPerBd;
				if(myData->bData[bd].cData[ch].op.state == C_RUN){
					myData->bData[bd].cData[ch].
						signal[C_SIG_NETWORK_CONNECT_ERROR] = P1;
					idx = myData->save_msg[0].read_idx[bd][ch];
					if(myData->save_msg[0].val[idx][bd][ch].chData.resultIndex < MAX_RETRANS_DATA) {
						sendNum = myData->save_msg[0].
							val[idx][bd][ch].chData.resultIndex;
					}else { 
						sendNum = MAX_RETRANS_DATA;
					}
					if(idx < sendNum) {
						myData->save_msg[0].read_idx[bd][ch] += 
								(MAX_SAVE_MSG - MAX_RETRANS_DATA);
						myData->save_msg[0].count[bd][ch] += sendNum;

					}else {
						myData->save_msg[0].read_idx[bd][ch] -= sendNum;
						myData->save_msg[0].count[bd][ch] += sendNum;

					}
				}
			}
		}
	}else{
		myData->mData.mainStateCheckFlag = P0;
		myPs->misc.disconnect_timer = myPs->misc.timer_1sec;
	}
	if(myData->MainClient.signal[MAIN_SIG_NET_CONNECTED] == P2){
		myData->mData.mainStateCheckFlag = P0;
		myPs->misc.disconnect_timer = myPs->misc.timer_1sec;
	}
}
//hun_210824
void AnalogMeter_Cali(void)
{
	int chamberNo = 0, bd1 = 0, ch1 = 0;
	int i = 0;
	int totalCount = 0 ,count = 0 ,count1 = 0;
	int bd = 0, ch = 0;
	int temp_bd = 0, temp_ch = 0, index = 0, index2 = 0;
	int max = 0,min = 0,j = 0,k = 0,ch_aux = 0,bd_aux = 0;
	int temp_cali_data_use = 0; //211124_hun
#ifdef _TEMP_CALI 
	unsigned char pointNo = 0, setPointCount;
	int tempCali_cnt = 0;
	int rtn = 0;
	float val1;
#endif	
	//211124_hun
	temp_cali_data_use = temp_cali_data_select();
	
	if(myData->AppControl.loadProcess[LOAD_ANALOGMETER] == P0 &&
		myData->AppControl.loadProcess[LOAD_ANALOGMETER2] == P0){
		return;
	}
	if(myData->AppControl.loadProcess[LOAD_ANALOGMETER] == P0 &&
		myData->AppControl.loadProcess[LOAD_ANALOGMETER2] == P1){
		return;
	}
	if(myData->AppControl.loadProcess[LOAD_ANALOGMETER] == P1){
		count = myData->AnalogMeter.config.chPerModule * myData->AnalogMeter.config.countMeter;
	}
	if(myData->AppControl.loadProcess[LOAD_ANALOGMETER2] == P1){
		count1 = myData->AnalogMeter2.config.chPerModule * myData->AnalogMeter2.config.countMeter;
	}
	totalCount = count + count1;
	
	for(i = 0 ; i < totalCount ; i++) {
		if(i < 100) {
			temp_ch = i % myData->AnalogMeter.config.chPerModule;
			temp_bd = i / myData->AnalogMeter.config.chPerModule;
		}else if(i >= 100) {
			temp_ch = (i - 100) % myData->AnalogMeter2.config.chPerModule;
			temp_bd = (i - 100) / myData->AnalogMeter2.config.chPerModule;
		}
		if(temp_cali_data_use == 0){	//CALI_T use	
			if(myData->AnalogMeter.config.multiNum > 1){
				if(i % 2 == 0){
					index = (int)myData->TempArray1[i/myData->AnalogMeter.config.multiNum].number1 - 1; //temp_monitor_no 
					index2 = (int)myData->TempArray1[i/myData->AnalogMeter.config.multiNum].number2; //hw_no 
				}else if(i % 2 == 1){
					index = (int)myData->TempArray1[(i-1)/myData->AnalogMeter.config.multiNum].number1 - 1; //temp_monitor_no 
					index2 = (int)myData->TempArray1[(i-1)/myData->AnalogMeter.config.multiNum].number2; //hw_no 
				}
				if(index < myData->mData.config.installedCh){
					bd = (int)myData->TempArray1[index].bd;
					ch = (int)myData->TempArray1[index].ch;
					//temp 10000 do upper not save
					if((myData->AnalogMeter.misc2.tempData[i] <= 10000000)&&
						(myData->AnalogMeter.misc2.tempData[i] >= -10000000)) {
						if(myData->AnalogMeter.config.auxControlFlag == P1){
							if(myData->mData.config.installedTemp > 0){
								if(index2 != 0){
									myData->AnalogMeter.temp[i].temp
										= (long)(myData->AnalogMeter.misc2.tempData[i]
										* myData->AnalogMeter.config.measure_gain[temp_bd][temp_ch])
										+ myData->AnalogMeter.config.measure_offset[temp_bd][temp_ch];
								}else{
									myData->AnalogMeter.temp[i].temp = 0;
								}
							}else{
								if(index2 != 0){
									myData->bData[bd].cData[ch].op.temp
										= (long)(myData->AnalogMeter.misc2.tempData[i]
										* myData->AnalogMeter.config.measure_gain[temp_bd][temp_ch])
										+ myData->AnalogMeter.config.measure_offset[temp_bd][temp_ch];
									myData->AnalogMeter.temp[i].temp
										= myData->bData[bd].cData[ch].op.temp;
								}else{
									myData->bData[bd].cData[ch].op.temp = 0;
									myData->AnalogMeter.temp[i].temp = 0;
								}
							}
						}else{
							if((i+1) % 2 == 0){
								if(index2 != 0){
									myData->bData[bd].cData[ch].op.temp1
										= (long)(myData->AnalogMeter.misc2.tempData[i]
										* myData->AnalogMeter.config.measure_gain[temp_bd][temp_ch])
										+ myData->AnalogMeter.config.measure_offset[temp_bd][temp_ch];
									myData->AnalogMeter.temp[i].temp1
										= myData->bData[bd].cData[ch].op.temp1;
								}else{
									myData->bData[bd].cData[ch].op.temp1 = 0;
									myData->AnalogMeter.temp[i].temp1 = 0;
								}
							}else{
								if(index2 != 0){
									myData->bData[bd].cData[ch].op.temp
										= (long)(myData->AnalogMeter.misc2.tempData[i]
										* myData->AnalogMeter.config.measure_gain[temp_bd][temp_ch])
										+ myData->AnalogMeter.config.measure_offset[temp_bd][temp_ch];
									myData->AnalogMeter.temp[i].temp
										= myData->bData[bd].cData[ch].op.temp;
								}else{
									myData->bData[bd].cData[ch].op.temp = 0;
									myData->AnalogMeter.temp[i].temp = 0;
								}
							}
						}
	
						if(myData->AnalogMeter.config.connectionCheck == P1) {
							if(((myData->bData[bd].cData[ch].op.temp
								> TEMP_CONNECT_ERROR_VALUE) 
								|| (myData->bData[bd].cData[ch].op.temp1
								> TEMP_CONNECT_ERROR_VALUE))
								|| (myData->bData[bd].cData[ch].op.temp
								< TEMP_CONNECT_ERROR_VALUE*(-1))
								|| (myData->bData[bd].cData[ch].op.temp1
								< TEMP_CONNECT_ERROR_VALUE*(-1))) {
								if(myData->bData[bd].cData[ch].op.state == C_RUN) {
									if(myData->bData[bd].cData[ch].ChAttribute.opType == P1) {
										myData->bData[bd].cData[ch].signal
											[C_SIG_TEMP_CONNECT_ERROR] = P1;
										myData->bData[bd].cData[ch+1].signal
											[C_SIG_TEMP_CONNECT_ERROR] = P1;
									} else {
										myData->bData[bd].cData[ch].signal
											[C_SIG_TEMP_CONNECT_ERROR] = P1;
									}
								}
							} else { 
								myData->bData[bd].cData[ch].signal
									[C_SIG_TEMP_CONNECT_ERROR] = P0;
							}
						}
					}
				}else if(myData->mData.config.installedTemp > 0){
					if(i < 100){
						if(index2 != 0){
							myData->AnalogMeter.temp[i].temp
							= (long)(myData->AnalogMeter.misc2.tempData[i]
								* myData->AnalogMeter.config.measure_gain[temp_bd][temp_ch])
								+ myData->AnalogMeter.config.measure_offset[temp_bd][temp_ch];
						}else{
							myData->AnalogMeter.temp[i].temp = 0;
						}
					}else if(i >= 100){
						if(index2 != 0){
							myData->AnalogMeter2.temp[i-100].temp
							= (long)(myData->AnalogMeter2.misc2.tempData[i-100]
								* myData->AnalogMeter2.config.measure_gain[temp_bd][temp_ch])
								+ myData->AnalogMeter2.config.measure_offset[temp_bd][temp_ch];
						}else{
							myData->AnalogMeter2.temp[i-100].temp = 0;
						}
					}
				}
			}else{				
				index = (int)myData->TempArray1[i].number1 - 1; //temp_monitor_no
				index2 = (int)myData->TempArray1[i].number2; //temp_hw_no
				
				if(index < myData->mData.config.installedCh){
					bd = (int)myData->TempArray1[index].bd;
					ch = (int)myData->TempArray1[index].ch;
					//temp 10000 do upper not save
					if((myData->AnalogMeter.misc2.tempData[i] <= 10000000)&&
						(myData->AnalogMeter.misc2.tempData[i] >= -10000000)) {
						if(myData->AnalogMeter.config.auxControlFlag == P1){
							if(myData->mData.config.installedTemp > 0){
								if(index2 != 0){
									myData->AnalogMeter.temp[i].temp
										= (long)(myData->AnalogMeter.misc2.tempData[i]
										* myData->AnalogMeter.config.measure_gain[temp_bd][temp_ch])
										+ myData->AnalogMeter.config.measure_offset[temp_bd][temp_ch];
								}else{
									myData->AnalogMeter.temp[i].temp = 0;
								}
							}else{
								if(index2 != 0){
									myData->bData[bd].cData[ch].op.temp
										= (long)(myData->AnalogMeter.misc2.tempData[i]
										* myData->AnalogMeter.config.measure_gain[temp_bd][temp_ch])
										+ myData->AnalogMeter.config.measure_offset[temp_bd][temp_ch];
									myData->AnalogMeter.temp[i].temp
										= myData->bData[bd].cData[ch].op.temp;
								}else{
									myData->bData[bd].cData[ch].op.temp = 0;
									myData->AnalogMeter.temp[i].temp = 0;
								}
							}
						}else{
							if(index2 != 0){
								myData->bData[bd].cData[ch].op.temp
									= (long)(myData->AnalogMeter.misc2.tempData[i]
									* myData->AnalogMeter.config.measure_gain[temp_bd][temp_ch])
									+ myData->AnalogMeter.config.measure_offset[temp_bd][temp_ch];
								myData->AnalogMeter.temp[i].temp
									= myData->bData[bd].cData[ch].op.temp;
							}else{
								myData->bData[bd].cData[ch].op.temp = 0;
								myData->AnalogMeter.temp[i].temp = 0;
							}
						}
						if(myData->AnalogMeter.config.connectionCheck == P1) {
							if(myData->bData[bd].cData[ch].ChAttribute.chNo_master != P0) {	
								if(myData->bData[bd].cData[ch].op.temp
									> TEMP_CONNECT_ERROR_VALUE 
									|| myData->bData[bd].cData[ch].op.temp 
									< TEMP_CONNECT_ERROR_VALUE*(-1)) {
									if(myData->bData[bd].cData[ch].op.state == C_RUN) {
										if(myData->bData[bd].cData[ch].ChAttribute.opType == P1) {
											myData->bData[bd].cData[ch].signal
												[C_SIG_TEMP_CONNECT_ERROR] = P1;
											myData->bData[bd].cData[ch+1].signal
												[C_SIG_TEMP_CONNECT_ERROR] = P1;
										} else {
											myData->bData[bd].cData[ch].signal
												[C_SIG_TEMP_CONNECT_ERROR] = P1;
										}
									}
								} else {
									myData->bData[bd].cData[ch].signal
										[C_SIG_TEMP_CONNECT_ERROR] = P0;
								}
							}
						}
					}
				}else if(myData->mData.config.installedTemp > 0){
					if(i < 100){
						if(index2 != 0){
						myData->AnalogMeter.temp[i].temp
								= (long)(myData->AnalogMeter.misc2.tempData[i]
								* myData->AnalogMeter.config.measure_gain[temp_bd][temp_ch])
								+ myData->AnalogMeter.config.measure_offset[temp_bd][temp_ch];
							if(myData->mData.config.ambient2 != 0){	//INC/LGES_Patch_note.txt Check
								if(i >= 30 && i < 34){
									myData->mData.misc.ambientTemp[i-30] = myData->AnalogMeter.temp[i].temp;
								}
								for(k = 0 ; k < myData->mData.config.installedCh; k++){
									if(myData->ChamberChNo[k].number1 != 0 &&
										myData->ChamberChNo[k].number1 <= 10){
										chamberNo = myData->ChamberChNo[k].number1;
										bd1 = myData->ChamberChNo[k].bd;
										ch1 = myData->ChamberChNo[k].ch;
										myData->bData[bd1].cData[ch1].misc.ambientTemp
										 = myData->mData.misc.ambientTemp[chamberNo-1];
									}
								}
							}
						}else{
							myData->AnalogMeter.temp[i].temp = 0;
						}
					}else if(i >= 100){
						if(index2 != 0){
							myData->AnalogMeter2.temp[i-100].temp
							= (long)(myData->AnalogMeter2.misc2.tempData[i-100]
								* myData->AnalogMeter2.config.measure_gain[temp_bd][temp_ch])
								+ myData->AnalogMeter2.config.measure_offset[temp_bd][temp_ch];
						}else{
							myData->AnalogMeter2.temp[i-100].temp = 0;
						}
					}
				}
			}
		}
	#ifdef _TEMP_CALI 
		pointNo = myData->AnalogMeter.temp_cali.pointNo;
		if(myData->AnalogMeter.temp_cali.signal[ANALOG_METER_SIG_CALI_NORMAL] == P1) {
			if(myData->temp_cali.data.temp_caliFlag[pointNo][i] == 1) {
				if(i < 100){
					myData->temp_cali.data.setTempValue[pointNo][i] 				
						= myData->AnalogMeter.misc2.tempData[i];		
				}else if(i >= 100){
					myData->temp_cali.data.setTempValue[pointNo][i] 
						= myData->AnalogMeter2.misc2.tempData[i - 100];				
				}
				//if(i==0) myData->temp_cali.data.temp_caliFlag[pointNo][i] = 0;
				tempCali_cnt++;
				myData->temp_cali.point.caliFlagCount 
						= myData->temp_cali.point.caliFlagCount -1;
			}
			if(myData->temp_cali.point.caliFlagCount == 0) {
				myData->AnalogMeter.temp_cali.signal[ANALOG_METER_SIG_CALI_NORMAL] = P2;
			}
		}
		if(i < 100){
			val1 = (float)myData->AnalogMeter.misc2.tempData[i];
		}else if(i >= 100){
			val1 = (float)myData->AnalogMeter2.misc2.tempData[i - 100];
		}
		if(temp_cali_data_use == 1){	//CALI_TEMP use	
			setPointCount = myData->temp_cali.point.setPointCount;
			if(setPointCount == 2) {
				if(i < 100){
					myData->AnalogMeter.temp[i].temp
						 = (float)myData->AnalogMeter.misc2.tempData[i]
							* myData->temp_cali.measure.gain[0][i]
							+ myData->temp_cali.measure.offset[0][i];			
				}else if(i >= 100){
					myData->AnalogMeter2.temp[i-100].temp
						 = (float)myData->AnalogMeter2.misc2.tempData[i - 100]
							* myData->temp_cali.measure.gain[0][i]
							+ myData->temp_cali.measure.offset[0][i];			
				}
			} else {
				if(val1 < myData->temp_cali.data.setTempValue[1][i]) {
					if(i < 100){
						myData->AnalogMeter.temp[i].temp
							 = (float)myData->AnalogMeter.misc2.tempData[i]
								* myData->temp_cali.measure.gain[0][i]
								+ myData->temp_cali.measure.offset[0][i];
					}else if(i >= 100){
						myData->AnalogMeter2.temp[i-100].temp
							 = (float)myData->AnalogMeter2.misc2.tempData[i - 100]
								* myData->temp_cali.measure.gain[0][i]
								+ myData->temp_cali.measure.offset[0][i];
					}
				} else {
					if(setPointCount == 3) {
						if(i < 100){
							myData->AnalogMeter.temp[i].temp
								 = (float)myData->AnalogMeter.misc2.tempData[i]
									* myData->temp_cali.measure.gain[1][i]
									+ myData->temp_cali.measure.offset[1][i];
						}else if(i >= 100){
							myData->AnalogMeter2.temp[i-100].temp
								 = (float)myData->AnalogMeter2.misc2.tempData[i - 100]
									* myData->temp_cali.measure.gain[1][i]
									+ myData->temp_cali.measure.offset[1][i];
						}
					} else if(setPointCount == 4) {
						if(val1 >= myData->temp_cali.data.setTempValue[2][i]) {
							if(i < 100){
								myData->AnalogMeter.temp[i].temp
									 = (float)myData->AnalogMeter.misc2.tempData[i]
										* myData->temp_cali.measure.gain[2][i]
										+ myData->temp_cali.measure.offset[2][i];
							}else if(i >= 100){
								myData->AnalogMeter2.temp[i-100].temp
									 = (float)myData->AnalogMeter2.misc2.tempData[i - 100]
										* myData->temp_cali.measure.gain[2][i]
										+ myData->temp_cali.measure.offset[2][i];
							}
						} else {
							if(i < 100){
								myData->AnalogMeter.temp[i].temp
									 = (float)myData->AnalogMeter.misc2.tempData[i]
										* myData->temp_cali.measure.gain[1][i]
										+ myData->temp_cali.measure.offset[1][i];
							}else if(i >= 100){
								myData->AnalogMeter2.temp[i-100].temp
									 = (float)myData->AnalogMeter2.misc2.tempData[i - 100]
										* myData->temp_cali.measure.gain[1][i]
										+ myData->temp_cali.measure.offset[1][i];
							}
						}
					} else {
						if(val1 >= myData->temp_cali.data.setTempValue[3][i]) {
							if(i < 100){
								myData->AnalogMeter.temp[i].temp
									 = (float)myData->AnalogMeter.misc2.tempData[i]
										* myData->temp_cali.measure.gain[3][i]
										+ myData->temp_cali.measure.offset[3][i];
							}else if(i >= 100){
								myData->AnalogMeter2.temp[i-100].temp
									 = (float)myData->AnalogMeter2.misc2.tempData[i - 100]
										* myData->temp_cali.measure.gain[3][i]
										+ myData->temp_cali.measure.offset[3][i];
							}
						} else if((val1 < myData->temp_cali.data.setTempValue[3][i]) 
								&& (val1 >= myData->temp_cali.data.setTempValue[2][i])) {
							if(i < 100){
								myData->AnalogMeter.temp[i].temp
									 = (float)myData->AnalogMeter.misc2.tempData[i]
										* myData->temp_cali.measure.gain[2][i]
										+ myData->temp_cali.measure.offset[2][i];
							}else if(i >= 100){
								myData->AnalogMeter2.temp[i-100].temp
									 = (float)myData->AnalogMeter2.misc2.tempData[i - 100]
										* myData->temp_cali.measure.gain[2][i]
										+ myData->temp_cali.measure.offset[2][i];
							}
						} else {
							if(i < 100){
								myData->AnalogMeter.temp[i].temp
									 = (float)myData->AnalogMeter.misc2.tempData[i]
										* myData->temp_cali.measure.gain[1][i]
										+ myData->temp_cali.measure.offset[1][i];
							}else if(i >= 100){
								myData->AnalogMeter2.temp[i-100].temp
									 = (float)myData->AnalogMeter2.misc2.tempData[i - 100]
										* myData->temp_cali.measure.gain[1][i]
										+ myData->temp_cali.measure.offset[1][i];
							}
						}
					}
				}
			}
		}
		if(myData->AnalogMeter.config.multiNum > 1){
			if(i % 2 == 0){
				index = (int)myData->TempArray1[i/myData->AnalogMeter.config.multiNum].number1 - 1; //temp_monitor_no 
				index2 = (int)myData->TempArray1[i/myData->AnalogMeter.config.multiNum].number2; //hw_no 
			}else if(i % 2 == 1){
				index = (int)myData->TempArray1[(i-1)/myData->AnalogMeter.config.multiNum].number1 - 1; //temp_monitor_no 
				index2 = (int)myData->TempArray1[(i-1)/myData->AnalogMeter.config.multiNum].number2; //hw_no 
			}
			if(index < myData->mData.config.installedCh){
				bd = (int)myData->TempArray1[index].bd;
				ch = (int)myData->TempArray1[index].ch;
				//temp 10000 do upper not save
				if((myData->AnalogMeter.misc2.tempData[i] <= 10000000)&&
					(myData->AnalogMeter.misc2.tempData[i] >= -10000000)) {
					if(myData->AnalogMeter.config.auxControlFlag == P1){
						if(myData->mData.config.installedTemp > 0){
							if(index2 == 0){
								myData->AnalogMeter.temp[i].temp = 0;
							}
						}else{
							if(index2 != 0){
								myData->bData[bd].cData[ch].op.temp = myData->AnalogMeter.temp[i].temp;
							}else{
								myData->bData[bd].cData[ch].op.temp = 0;
								myData->AnalogMeter.temp[i].temp = 0;
							}
						}
					}else{
						if((i+1) % 2 == 0){
							if(index2 != 0){
								myData->bData[bd].cData[ch].op.temp = myData->AnalogMeter.temp[i].temp1;
							}else{
								myData->bData[bd].cData[ch].op.temp1 = 0;
								myData->AnalogMeter.temp[i].temp1 = 0;
							}
						}else{
							if(index2 != 0){
								myData->bData[bd].cData[ch].op.temp = myData->AnalogMeter.temp[i].temp;
							}else{
								myData->bData[bd].cData[ch].op.temp = 0;
								myData->AnalogMeter.temp[i].temp = 0;
							}
						}
					}

					if(myData->AnalogMeter.config.connectionCheck == P1) {
						if(((myData->bData[bd].cData[ch].op.temp
							> TEMP_CONNECT_ERROR_VALUE) 
							|| (myData->bData[bd].cData[ch].op.temp1
							> TEMP_CONNECT_ERROR_VALUE))
							|| (myData->bData[bd].cData[ch].op.temp
							< TEMP_CONNECT_ERROR_VALUE*(-1))
							|| (myData->bData[bd].cData[ch].op.temp1
							< TEMP_CONNECT_ERROR_VALUE*(-1))) {
							if(myData->bData[bd].cData[ch].op.state == C_RUN) {
								if(myData->bData[bd].cData[ch].ChAttribute.opType == P1) {
									myData->bData[bd].cData[ch].signal
										[C_SIG_TEMP_CONNECT_ERROR] = P1;
									myData->bData[bd].cData[ch+1].signal
										[C_SIG_TEMP_CONNECT_ERROR] = P1;
								} else {
									myData->bData[bd].cData[ch].signal
										[C_SIG_TEMP_CONNECT_ERROR] = P1;
								}
							}
						} else { 
							myData->bData[bd].cData[ch].signal
								[C_SIG_TEMP_CONNECT_ERROR] = P0;
						}
					}
				}
			}
		}else {
			index = (int)myData->TempArray1[i].number1 - 1; //temp_monitor_no
			index2 = (int)myData->TempArray1[i].number2; //temp_hw_no
			if(index < myData->mData.config.installedCh){
				bd = (int)myData->TempArray1[index].bd;
				ch = (int)myData->TempArray1[index].ch;
				if(i < 100){
					if(index2 != 0){
						myData->bData[bd].cData[ch].op.temp 
							= myData->AnalogMeter.temp[i].temp;
					}else{
						myData->bData[bd].cData[ch].op.temp = 0;
						myData->AnalogMeter.temp[i].temp = 0;
					}
				}else if(i >= 100){
					if(index2 != 0){
							myData->bData[bd].cData[ch].op.temp 
						= myData->AnalogMeter2.temp[i-100].temp;
					}else{
							myData->bData[bd].cData[ch].op.temp = 0;
						myData->AnalogMeter2.temp[i-100].temp = 0;
					}
				}
				if(myData->AnalogMeter.config.connectionCheck == P1) {
					if(myData->bData[bd].cData[ch].ChAttribute.chNo_master != P0) {	
						if(myData->bData[bd].cData[ch].op.temp
							> TEMP_CONNECT_ERROR_VALUE 
							|| myData->bData[bd].cData[ch].op.temp 
							< TEMP_CONNECT_ERROR_VALUE*(-1)) {
							if(myData->bData[bd].cData[ch].op.state == C_RUN) {
								if(myData->bData[bd].cData[ch].ChAttribute.opType == P1) {
									myData->bData[bd].cData[ch].signal
										[C_SIG_TEMP_CONNECT_ERROR] = P1;
									myData->bData[bd].cData[ch+1].signal
										[C_SIG_TEMP_CONNECT_ERROR] = P1;
								} else {
									myData->bData[bd].cData[ch].signal
										[C_SIG_TEMP_CONNECT_ERROR] = P1;
								}
							}
						} else {
							myData->bData[bd].cData[ch].signal
								[C_SIG_TEMP_CONNECT_ERROR] = P0;
						}
					}
				}
			}
		}
		if(myData->AnalogMeter.temp_cali.signal[ANALOG_METER_SIG_CALI_NORMAL] == P2) {
			rtn = analog_cali_calc_2(pointNo);
			send_msg(MODULE_TO_METER2, MSG_MODULE_METER2_WRITE_TEMP_CALI_DATA, pointNo, 0);			
			myData->AnalogMeter.temp_cali.signal[ANALOG_METER_SIG_CALI_NORMAL] = P0;
		}	
#endif	
	
		max = -10000000;
		min = 10000000;
		if(myData->AnalogMeter.config.auxControlFlag == P1){
			if(myData->mData.config.installedTemp > 0){
				j = 0;
				for(k=0; k<myData->mData.config.installedTemp; k++){
					if(myData->auxSetData[k].chNo == ch+1){
						if(k < 100){
							if(max < myData->AnalogMeter.temp[k].temp)
								max = myData->AnalogMeter.temp[k].temp;
							if(min > myData->AnalogMeter.temp[k].temp)
								min = myData->AnalogMeter.temp[k].temp;				
						}else if(k >= 100){
							if(max < myData->AnalogMeter2.temp[k-100].temp)
								max = myData->AnalogMeter2.temp[k-100].temp;
							if(min > myData->AnalogMeter2.temp[k-100].temp)
								min = myData->AnalogMeter2.temp[k-100].temp;
						}
						j++;
						if(myData->auxSetData[k].chNo != 0){
							ch_aux = myData->auxSetData[k].chNo - 1;
							if(ch_aux < myData->mData.config.installedCh){
								bd_aux = 0;
							}else{
								bd_aux = 1;
							}
							myData->bData[bd_aux].cData[ch_aux]
									.misc.AuxTemp_max = max;
							myData->bData[bd_aux].cData[ch_aux]
									.misc.AuxTemp_min = min;
						}
					}
				}

				if(myData->AnalogMeter.config.tempNonDisplay == P1){
					if(index < myData->mData.config.installedCh){
						myData->bData[bd].cData[ch].op.temp = 0;
						myData->bData[bd].cData[ch].op.temp1 = 0;
					}
				}else{
					if(index < myData->mData.config.installedCh){
						myData->bData[bd].cData[ch].op.temp = max;
						myData->bData[bd].cData[ch].op.temp1 = min;
					}
				}
			}
			if(j == 0 && index < myData->mData.config.installedCh){
				myData->bData[bd].cData[ch].op.temp = 0;
				myData->bData[bd].cData[ch].op.temp1 = 0;
			}
		}else{
			if(myData->AnalogMeter.config.tempNonDisplay == P1 
				&& index < myData->mData.config.installedCh){
					myData->bData[bd].cData[ch].op.temp = 0;
					myData->bData[bd].cData[ch].op.temp1 = 0;
			}
		}
	}

#if CH_AUX_DATA == 1
	if(myData->mData.config.installedTemp > 0){
		AuxTemp_ChData_Mapping();
	}
#endif
}

//hun_210824
#ifdef _TEMP_CALI 
int analog_cali_calc_2(int setPointNo)
{
	unsigned char point_count;
//	int i ,rtn, countMeter, chPerModule, tempNo;
	int i ,rtn, tempNo;
	long temp_ref1, temp_ref2, temp_value1, temp_value2;
	float value_diff = 0;
	float ratio, offset;
	int installedTemp = 0;
	
	rtn = 0;
	installedTemp = myData->mData.config.installedTemp;
//	temp_ref1 = temp_ref2 = 0;
//	temp_value1 = temp_value2 = 0;
	point_count = myData->temp_cali.point.setPointCount - 1;
//	countMeter = myPs->config.countMeter;
//	chPerModule = myPs->config.chPerModule;
//	tempNo = countMeter * chPerModule;
	if(installedTemp == 0){
		tempNo = myData->mData.config.installedCh;	
	}else if(installedTemp != 0){
		tempNo = myData->mData.config.installedTemp;
	}
	if(setPointNo != 0) {
		temp_ref1 = myData->temp_cali
					.point.setTempPoint[setPointNo - 1] * 1000;
		temp_ref2 = myData->temp_cali
					.point.setTempPoint[setPointNo] * 1000;
		for(i=0; i < tempNo; i++) {
			temp_value1 = myData->temp_cali.data
										.setTempValue[setPointNo-1][i];
			temp_value2 = myData->temp_cali.data
										.setTempValue[setPointNo][i];
			if(temp_value1 == temp_value2) {
				value_diff = 0.1;
			} else {
				value_diff = ((float)temp_value2 - (float)temp_value1);
			}
			ratio = ((float)temp_ref2 - (float)temp_ref1) /((float)value_diff);
		   	offset = ((float)temp_ref1) - ((float)ratio * (float)temp_value1);	

			if(temp_value1 == 99999000) {
				myData->temp_cali.measure.gain[setPointNo-1][i] = 1;
				myData->temp_cali.measure.offset[setPointNo-1][i] = 0;
			} else {
				if(myData->temp_cali.data.temp_caliFlag[setPointNo][i] == 1) {
					myData->temp_cali.measure.gain[setPointNo-1][i] = ratio;
					myData->temp_cali.measure.offset[setPointNo-1][i] = offset;
				}else if(myData->temp_cali.data.temp_caliFlag[setPointNo][i] == 0) {
					myData->temp_cali.measure.gain[setPointNo-1][i] 
						= myData->temp_cali.measure.gain[setPointNo-1][i];
					myData->temp_cali.measure.offset[setPointNo-1][i] 
						= myData->temp_cali.measure.offset[setPointNo-1][i];				
				}

			}
		}				
	}

	if(setPointNo != 0 && setPointNo < point_count) {
		temp_ref1 = myData->temp_cali
					.point.setTempPoint[setPointNo] * 1000;
		temp_ref2 = myData->temp_cali
					.point.setTempPoint[setPointNo+1] * 1000;
		for(i=0; i < tempNo; i++) {
			temp_value1 = myData->temp_cali.data
										.setTempValue[setPointNo][i];
			temp_value2 = myData->temp_cali.data
										.setTempValue[setPointNo+1][i];
			if(temp_value1 == temp_value2) {
				value_diff = 0.1;
			} else {
				value_diff = ((float)temp_value2 - (float)temp_value1);
			}
			ratio = ((float)temp_ref2 - (float)temp_ref1) / ((float)value_diff);
		   	offset = (float)temp_ref1 - ((float)ratio * (float)temp_value1);	
			
			if(temp_value2 == 99999000) {
				myData->temp_cali.measure.gain[setPointNo][i] = 1;
				myData->temp_cali.measure.offset[setPointNo][i] = 0;
			} else {
				if(myData->temp_cali.data.temp_caliFlag[setPointNo][i] == 1) {
					myData->temp_cali.measure.gain[setPointNo][i] = ratio;
					myData->temp_cali.measure.offset[setPointNo][i] = offset;
				}else if(myData->temp_cali.data.temp_caliFlag[setPointNo][i] == 0) {
					myData->temp_cali.measure.gain[setPointNo][i] 
						= myData->temp_cali.measure.gain[setPointNo][i];
					myData->temp_cali.measure.offset[setPointNo][i] 
						= myData->temp_cali.measure.offset[setPointNo][i];				
				}
			}	
		}
	}

	return 0;
}
#endif

#if CH_AUX_DATA == 1
void AuxTemp_ChData_Mapping()
{
	int i, j, k;
	unsigned char  bd, ch, checkNum;
	unsigned short installedAuxT;

	installedAuxT = myData->mData.config.installedTemp;
			
	for(i = 0 ; i < myData->mData.config.installedCh; i++){
		bd = myData->CellArray1[i].bd;
		ch = myData->CellArray1[i].ch;
		myData->bData[bd].cData[ch].misc.chAuxTCnt = 0;
		myData->bData[bd].cData[ch].misc.chAuxTCheckNum = 0;
		checkNum = 0;
		for(j = 0; j < installedAuxT; j++){
			if(myData->auxSetData[j].auxType == 0 
				&& myData->auxSetData[j].chNo == ch + 1){
				if(j < 100){
					myData->bData[bd].cData[ch].misc.chAuxTemp[checkNum] 
						= myData->AnalogMeter.temp[j].temp;	
				}else if(j >= 100){
					myData->bData[bd].cData[ch].misc.chAuxTemp[checkNum] 
						= myData->AnalogMeter2.temp[j-100].temp;
				}
				checkNum++;
				if(checkNum >= MAX_CH_AUX_DATA) checkNum = 0;
			}
		}
		if(checkNum != 0){
			for(k = checkNum ; k < MAX_CH_AUX_DATA ; k++){
				myData->bData[bd].cData[ch].misc.chAuxTemp[k] = 0; 
			}
		}
	}
}
#endif
int temp_cali_data_select(void)
{
	#ifdef _TEMP_CALI
	if(myData->AnalogMeter.config.measure_gain[0][0] == 1 &&
	 myData->AnalogMeter.config.measure_offset[0][0] == 0){
		return 1; //CALI_TEMP use
	}else{
		return 0; //CALI_T use
	}
	#else
		return 0; //CALI_T use
	#endif
}

//220322_hun
void Ch_Code_Check(void)	
{
	int i = 0, bd_set = 0, ch_set = 0;
	int bd = 0, ch = 0;
	int shutdown = 0;
	int chNo = 0;

	for(i = 0 ; i < myData->mData.config.installedCh; i++){
		bd = myData->CellArray1[i].bd;
		ch = myData->CellArray1[i].ch;
		if(myData->bData[bd].cData[ch].op.code == C_FAULT_HARD_VENTING){
			bd_set = bd;
			ch_set = ch;
			shutdown = 1;
			break;
		}
	}
	if(shutdown == 1){
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
		myPs->code = myData->bData[bd_set].cData[ch_set].op.code;
		chNo = (myPs->config.chPerBd * bd_set) + (ch_set + 1);
		send_msg(MODULE_TO_APP, MSG_MODULE_APP_CH_EMG, myPs->code, chNo); 
		send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
			(int)myPs->code, chNo);
		for(bd=0; bd < myPs->config.installedBd; bd++) {
			for(ch=0; ch < myPs->config.chPerBd; ch++) {
				if((myPs->config.chPerBd * bd + ch)
					> (myPs->config.installedCh - 1)) continue;
					myData->bData[bd].cData[ch].signal[C_SIG_PAUSE] = P1;
			}
		}
	}
}
