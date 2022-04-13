#include <rtl_core.h>
#include <asm/io.h>
#include "../../INC/datastore.h"
#include "message.h"
#include "DInOutControl.h"
#include "PCU.h"

extern S_SYSTEM_DATA *myData;
extern S_MODULE_DATA *myPs;

void DInOutControl(int index)
{
	switch(index) {
		case 0:
			DInScan();
			Check_OVP();
			break;
		case 1:
			DIn_FlagCheck();
			DOut_FlagCheck();
			Check_OTP();
			break;
		case 2:
			DOutScan();
			Check_CCC();
			break;
		case 3:
//			ThermalSensor(); //external temperature
			break;
		default: break;
	}
}

void DInScan(void)
{
	unsigned char flag, input, useFlag, tmpInputNum;
	int tmp, inByte, signalNo, position, addr = 0, sensCount, active, idx = 0;
	
	if(myData->dio.delayTimer < myData->dio.config.dioDelay) {
		myData->dio.delayTimer
			+= myPs->misc.dio_scan_time;
		return;
	}
	if(myData->dio.config.dio_Control_Flag == P0) return;
	if(myData->AppControl.config.debugType != P0) return;
			
	sensCount = myData->dio.config.sensCount;

	if(myData->AppControl.config.systemType == CYCLER_CAN) {
		tmpInputNum = myPs->addr.inputAddrNo + CAN_IO_INPUT_CNT;
	} else {
		tmpInputNum = myPs->addr.inputAddrNo;
	}

	for(inByte=0; inByte < tmpInputNum; inByte++) {
		if(inByte >= MAX_DIGITAL_INPUT) return;
		if(inByte < myPs->addr.inputAddrNo) {
			addr = myPs->addr.interface[IO_INPUT][inByte];
			if(addr == 0) continue;
		}
		if(inByte >= myPs->addr.inputAddrNo &&
			myData->AppControl.config.systemType == CYCLER_CAN) { //Read CAN IO 
			idx = inByte - myPs->addr.inputAddrNo;
			input = myData->CAN.io.inputValue[idx];
		} else {
			input = inb(addr);
		}
		
		flag = 0x01;
		for(position=0; position < 8; position++) {
			signalNo = inByte * 8 + position;
			if(myData->AppControl.config.versionNo >= 20090602) {
				active = (int)myData->dio.din.inActiveType[signalNo];
				useFlag = myData->dio.din.inUseFlag[signalNo];
				tmp = DInCheck(signalNo, input & flag, active, sensCount);
				if(useFlag == P1){
					if(tmp == P1) myData->dio.din.inFlag[signalNo] = P1;
					else myData->dio.din.inFlag[signalNo] = P0;
				}else if(useFlag == P2){	//190731
					if(tmp == P1) myData->dio.din.inFlag[signalNo] = P2;
					else myData->dio.din.inFlag[signalNo] = P0;
				}
			} else {
				switch(signalNo){
					case I_IN_SMPS_FAIL1:
					case I_IN_SMPS_FAIL2:
						tmp = DInCheck(signalNo, input & flag, 0, sensCount); 
						break;
					default:
						tmp = DInCheck(signalNo, input & flag, 1, sensCount);
						break;
				}
				if(tmp == P1) myData->dio.din.inFlag[signalNo] = P1;
				else myData->dio.din.inFlag[signalNo] = P0;
			}
			flag = flag << 1;
		}
	}
}

int DInCheck(int pos, int comp, int pn, int count)
{
	int rtn, flag1, flag2;
	rtn = P0;

	switch(pn) {
		case 0:
			flag1 = P0;
			flag2 = P1;
			break;
		case 1:
			flag1 = P1;
			flag2 = P0;
			break;
		case 2:
			flag1 = P0;
			flag2 = P1;
			break;
		default:
			flag1 = P1;
			flag2 = P0;
			break;
	}

    if(comp != 0) {
		if(myData->dio.din.inCountFlag[pos] == 0) {
			myData->dio.din.inCount[pos]++;
    		if(myData->dio.din.inCount[pos] >= count) {
	    		myData->dio.din.inCountFlag[pos] = 1;
				myData->dio.din.inCount[pos] = 0;
				rtn = flag1;
			} else rtn = flag2;
		} else {
			myData->dio.din.inCount[pos] = 0;
			if(myData->dio.din.inCountFlag[pos] == 1) rtn = flag1;
			if(pn == 3) rtn = flag2;
		}
   	} else {
		if(myData->dio.din.inCountFlag[pos] == 1) {
			myData->dio.din.inCount[pos]++;
			if(myData->dio.din.inCount[pos] >= count) {
				myData->dio.din.inCountFlag[pos] = 0;
				myData->dio.din.inCount[pos] = 0;
				rtn = flag2;
			} else {
				rtn = flag1;
				if(pn == 3) rtn = flag2;
			}
		} else {
			myData->dio.din.inCount[pos] = 0;
			if(myData->dio.din.inCountFlag[pos] == 0) rtn = flag2;
		}
	}
	return rtn;
}

void DIn_FlagCheck(void)
{
	if(myData->dio.delayTimer < myData->dio.config.dioDelay) return;
	if(myData->AppControl.config.debugType != P0) return;
	
	if(DIn_FlagCheck_MainEMG() < 0) return;
	if(DIn_FlagCheck_PowerSwitch() < 0) return;
	if(DIn_FlagCheck_PowerFail() < 0) return;
	if(DIn_FlagCheck_SMPSFail() < 0) return;
	
	
	if(myData->mData.config.function[F_CHAMBER_TYPE] != P0){
		if(myData->mData.config.ChamberDioUse == P1){
			DIn_FlagCheck_ChamberFail2();
		}else{
			DIn_FlagCheck_ChamberFail();
		}
	}

	switch(myPs->config.hwSpec) {
		case L_5V_250A_R2:
			DIn_Limit_Check();
			break;
		case L_5V_150A_R2_P:
			DIn_Buzzer_Stop();
			break;
		case L_5V_200A_1CH_JIG:
			DIn_Jig_Check();
			break;
		case DC_5V_CYCLER_NEW:
			DIn_Door_Open_FAIL();
			if(myData->mData.config.function[F_CHAMBER_TYPE] != 2){
				DIn_Smoke_FAIL();
			}
			break;
		default:
			DIn_Door_Open_FAIL1();
			DIn_Door_Open_FAIL2();
			if(myData->mData.config.function[F_CHAMBER_TYPE] != 2){
				DIn_Smoke_FAIL();
			}
			break;
	}

	if(myData->mData.config.function[F_CHANGE_VI_CHECK] == 2){ //191031
		//add for NorthVolta OPCUA Nomal State Send
		EMG_Signal_Check();	//191001 lyhw
	}
}

int DIn_FlagCheck_MainEMG(void)
{
	int rtn=0, bd, ch;
	unsigned char flag;

	flag = Read_InPoint(I_IN_MAIN_EMG);
			
	if(flag == P1){
		if(myData->dio.signal[DIO_SIG_MAIN_EMG] == P0) {
#if CYCLER_TYPE == DIGITAL_CYC
			//180611 add for digtal
			myPs->signal[M_SIG_INV_POWER] = P100;
			myPs->signal[M_SIG_INV_POWER1] = P100;
#endif
			//210303 Inverter off 
			if(myData->AppControl.config.systemType == CYCLER_CAN){ 
				myPs->signal[M_SIG_INV_POWER_CAN] = P100;
			}

			if((inb(0x602) & 0x10) == P0) {
				myPs->signal[M_SIG_RUN_LED] = P0;
				myPs->signal[M_SIG_REMOTE_SMPS1] = P0;
				myPs->signal[M_SIG_FAN_RELAY] = P0;
				myPs->signal[M_SIG_POWER_OFF] = P1;
				//main emg.
				send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 1, 0);
			}
			
			myPs->code = M_FAIL_MAIN_EMG;
			myData->dio.signal[DIO_SIG_MAIN_EMG] = P1;
			send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
				(int)myPs->code, 0);
			for(bd=0; bd < myPs->config.installedBd; bd++) {
				for(ch=0; ch < myPs->config.chPerBd; ch++) {
					if((myPs->config.chPerBd * bd + ch)
						> (myPs->config.installedCh-1)) {
						continue;
					}
					if(myData->bData[bd].cData[ch].op.state == C_RUN) {
						myData->bData[bd].cData[ch]
							.signal[C_SIG_MAIN_EMG] = P1;
					}
				}
			}
		}
		rtn = -1;
	} else {
		myData->dio.signal[DIO_SIG_MAIN_EMG] = P0;
	}
	return rtn;
}

int DIn_FlagCheck_PowerSwitch(void)
{
	int rtn=0, bd, ch;
	unsigned char	flag;

	flag = Read_InPoint(I_IN_POWER_SWITCH);

	if(flag == P0){
		if(myData->dio.signal[DIO_SIG_POWER_SWITCH] == P1){
			myData->dio.signal[DIO_SIG_POWER_SWITCH] = P0;
		}
		myData->dio.powerSwitchTimer = 0;
		return rtn;
	}

	myData->dio.powerSwitchTimer += myPs->misc.dio_scan_time;

	if(myPs->state == M_RUN) {
		if(myData->dio.powerSwitchTimer
			>= myData->dio.config.powerSwitchTimeout) { //3sec
			if(myData->dio.signal[DIO_SIG_POWER_SWITCH] < P2) {
				myData->dio.signal[DIO_SIG_POWER_SWITCH] = P2;
			}
		} else {
			if(myData->dio.signal[DIO_SIG_POWER_SWITCH] == P0) {
				myData->dio.signal[DIO_SIG_POWER_SWITCH] = P1;
				//warnning before power off
				send_msg(MODULE_TO_APP, MSG_MODULE_APP_POWER_SWITCH, 1, 0);
				BuzzerSet(5, 300, 300);
			}
		}
		if(myData->dio.signal[DIO_SIG_POWER_SWITCH] == P2) {
			myData->dio.signal[DIO_SIG_POWER_SWITCH] = P3;		
				
#if CYCLER_TYPE == DIGITAL_CYC
			//180611 add for digtal
			myPs->signal[M_SIG_INV_POWER] = P100;
			myPs->signal[M_SIG_INV_POWER1] = P100;
#endif
			//210303 Inverter off 
			if(myData->AppControl.config.systemType == CYCLER_CAN){ 
				myPs->signal[M_SIG_INV_POWER_CAN] = P100;
			}

			myPs->signal[M_SIG_RUN_LED] = P0;
			myPs->signal[M_SIG_REMOTE_SMPS1] = P0;
			myPs->signal[M_SIG_FAN_RELAY] = P0;
			myPs->signal[M_SIG_POWER_OFF] = P1;
			myPs->code = M_FAIL_FORCE_POWER;
			//forcing power off
			send_msg(MODULE_TO_APP, MSG_MODULE_APP_POWER_SWITCH, 2, 0);
			send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
				(int)myPs->code, 0);
			for(bd=0; bd < myPs->config.installedBd; bd++) {
				for(ch=0; ch < myPs->config.chPerBd; ch++) {
					if((myPs->config.chPerBd * bd + ch)
						> (myPs->config.installedCh-1)) {
						continue;
					}
					if(myData->bData[bd].cData[ch].op.state == C_RUN) {
						myData->bData[bd].cData[ch]
							.signal[C_SIG_FORCE_POWER] = P1;
					}
				}
			}
			rtn = -1;
		}
	} else {
		if(myData->dio.powerSwitchTimer
			>= myData->dio.config.powerSwitchTimeout) { //3sec
			if(myData->dio.signal[DIO_SIG_POWER_SWITCH] == P0) {
				myData->dio.signal[DIO_SIG_POWER_SWITCH] = P3;
				
#if CYCLER_TYPE == DIGITAL_CYC
				//180611 add for digtal
				myPs->signal[M_SIG_INV_POWER] = P100;
				myPs->signal[M_SIG_INV_POWER1] = P100;
#endif
				//210303 Inverter off 
				if(myData->AppControl.config.systemType == CYCLER_CAN){ 
					myPs->signal[M_SIG_INV_POWER_CAN] = P100;
				}
				
				myPs->signal[M_SIG_RUN_LED] = P0;
				myPs->signal[M_SIG_REMOTE_SMPS1] = P0;
				myPs->signal[M_SIG_FAN_RELAY] = P0;
				myPs->signal[M_SIG_POWER_OFF] = P1;
				//normal power off
				send_msg(MODULE_TO_APP, MSG_MODULE_APP_POWER_SWITCH, 0, 0);
			}
			rtn = -1;
		}
	}
	return rtn;
}

int DIn_FlagCheck_PowerFail(void)
{
	int rtn=0, bd, ch;
	unsigned char flag1, flag2;

	flag1 = (unsigned char)Read_InPoint(I_IN_AC_POWER_FAIL);
	flag2 = P0;		//flag2 = (unsigned char)Read_InPoint(I_IN_UPS_BATTERY_FAIL);
	if((flag1 == P1 && flag2 == P1) || (flag1 == P1 && flag2 == P0)) {
		myData->dio.powerFailTimer += myPs->misc.dio_scan_time;
		if(myData->dio.signal[DIO_SIG_POWER_FAIL] == P0) {
			if(myData->dio.powerFailTimer
				>= myData->dio.config.powerFailTimeout1) { //0sec
			//	if(myPs->code != M_FAIL_SMPS){
					myData->dio.signal[DIO_SIG_POWER_FAIL] = P1;
					//to Pause : short power fail
					myPs->code = M_FAIL_AC_POWER_SHORT;
					send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 35, 0);
					send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
						(int)myPs->code, 0);
					for(bd=0; bd < myPs->config.installedBd; bd++) {
						for(ch=0; ch < myPs->config.chPerBd; ch++) {
							if((myPs->config.chPerBd * bd + ch)
								> (myPs->config.installedCh-1)) {
								continue;
							}
							if(myData->bData[bd].cData[ch].op.state == C_RUN) {
								myData->bData[bd].cData[ch]
									.signal[C_SIG_AC_POWER] = P1;
							}
							if(myData->bData[bd].cData[ch].op.code == C_FAULT_INVERTER) {
								myData->bData[bd].cData[ch].op.code = C_FAULT_AC_POWER;
							}
						}
					}
					rtn = -1;
			//	}
			}
		} else if(myData->dio.signal[DIO_SIG_POWER_FAIL] == P1) {
			if(myData->dio.powerFailTimer
				>= myData->dio.config.powerFailTimeout2) { //10sec
				
#if CYCLER_TYPE == DIGITAL_CYC
				//180611 add for digtal
				myPs->signal[M_SIG_INV_POWER] = P100;
				myPs->signal[M_SIG_INV_POWER1] = P100;
#endif
				//210303 Inverter off 
				if(myData->AppControl.config.systemType == CYCLER_CAN){ 
					myPs->signal[M_SIG_INV_POWER_CAN] = P100;
				}

				myData->dio.signal[DIO_SIG_POWER_FAIL] = P2;
				myPs->signal[M_SIG_RUN_LED] = P0;
				myPs->signal[M_SIG_REMOTE_SMPS1] = P0;
				myPs->signal[M_SIG_FAN_RELAY] = P0;
				myPs->signal[M_SIG_POWER_OFF] = P1;
				//to Shutdown : long power fail
				myPs->code = M_FAIL_AC_POWER_LONG;
				send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 3, 0);
				send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
					(int)myPs->code, 0);
				rtn = -1;
			}

			if(flag2 == P1) {
				myData->dio.upsBatteryFailTimer 
					+= myPs->misc.dio_scan_time;
				if(myData->dio.upsBatteryFailTimer
					>= myData->dio.config.upsBatteryFailTimeout) { //2sec
					
#if CYCLER_TYPE == DIGITAL_CYC
					//180903 add for digtal
					myPs->signal[M_SIG_INV_POWER] = P100;
					myPs->signal[M_SIG_INV_POWER1] = P100;
#endif
					//210303 Inverter off 
					if(myData->AppControl.config.systemType == CYCLER_CAN){ 
						myPs->signal[M_SIG_INV_POWER_CAN] = P100;
					}

					myData->dio.signal[DIO_SIG_POWER_FAIL] = P2;
					myPs->signal[M_SIG_RUN_LED] = P0;
					myPs->signal[M_SIG_REMOTE_SMPS1] = P0;
					myPs->signal[M_SIG_FAN_RELAY] = P0;
					myPs->signal[M_SIG_POWER_OFF] = P1;
					//to Shutdown : battery fail
					myPs->code = M_FAIL_UPS_BATTERY;
					send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 4, 0);
					send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
						(int)myPs->code, 0);
					for(bd=0; bd < myPs->config.installedBd; bd++) {
						for(ch=0; ch < myPs->config.chPerBd; ch++) {
							if((myPs->config.chPerBd * bd + ch)
								> (myPs->config.installedCh-1)) {
								continue;
							}
							if(myData->bData[bd].cData[ch].op.state == C_RUN) {
								myData->bData[bd].cData[ch]
									.signal[C_SIG_UPS_BATTERY] = P1;
							}
						}
					}
					rtn = -1;
				}
			}
		} else if(myData->dio.signal[DIO_SIG_POWER_FAIL] == P2) {
			rtn = -1;
		}
	} else if(flag1 == P0 && flag2 == P1) { //battery fail
		myData->dio.upsBatteryFailTimer 
			+= myPs->misc.dio_scan_time;
		if(myData->dio.signal[DIO_SIG_POWER_FAIL] == P0) {
			if(myData->dio.upsBatteryFailTimer
				>= myData->dio.config.upsBatteryFailTimeout) { //2sec
				if(myPs->code != M_FAIL_SMPS){
#if CYCLER_TYPE == DIGITAL_CYC
					myPs->signal[M_SIG_INV_POWER] = P100;
					myPs->signal[M_SIG_INV_POWER1] = P100;
#endif
					//210303 Inverter off 
					if(myData->AppControl.config.systemType == CYCLER_CAN){ 
						myPs->signal[M_SIG_INV_POWER_CAN] = P100;
					}

					myData->dio.signal[DIO_SIG_POWER_FAIL] = P2;
					myPs->signal[M_SIG_RUN_LED] = P0;
					myPs->signal[M_SIG_REMOTE_SMPS1] = P0;
					myPs->signal[M_SIG_FAN_RELAY] = P0;
					myPs->signal[M_SIG_POWER_OFF] = P1;
					//to Shutdown : battery fail
					myPs->code = M_FAIL_UPS_BATTERY;
					send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 4, 0);
					send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
						(int)myPs->code, 0);
					for(bd=0; bd < myPs->config.installedBd; bd++) {
						for(ch=0; ch < myPs->config.chPerBd; ch++) {
							if((myPs->config.chPerBd * bd + ch)
								> (myPs->config.installedCh-1)) {
								continue;
							}
							if(myData->bData[bd].cData[ch].op.state == C_RUN) {
								myData->bData[bd].cData[ch]
									.signal[C_SIG_UPS_BATTERY] = P1;
							}
						}
					} rtn = -1;
				}
			}
		} else if(myData->dio.signal[DIO_SIG_POWER_FAIL] == P2) {
			rtn = -1;
		}
	} else {
		if(myData->dio.signal[DIO_SIG_POWER_FAIL] <= P2) {
			myData->dio.signal[DIO_SIG_POWER_FAIL] = 0;
		}
		myData->dio.powerFailTimer = 0;
		myData->dio.upsBatteryFailTimer = 0;
	}

	return rtn;
}

int DIn_FlagCheck_SMPSFail(void)
{
	int rtn = 0;

	if(myData->dio.signal[DIO_SIG_POWER_SWITCH] != P0
		||myPs->code == M_FAIL_TERMINAL_QUIT
		||myPs->code == M_FAIL_TERMINAL_HALT)	return rtn;

	switch(myPs->config.hwSpec) {
		case L_20V_25A:
		case L_5V_10_30A_R2:
			rtn = SMPS_FAIL_1();
			break;
		case L_5V_200A_R2:
		case L_5V_100A_R1:
		case L_5V_100A_R1_EG:
		case S_5V_200A:
		case L_2V_100A:
		case L_5V_100A_R2:
		case L_5V_2A_R1:
		case L_5V_200A_R4:
		case L_5V_500A_R2:
		case L_5V_200A_R3:
		case L_5V_150A_R3_AD2:
		case L_30V_20A_R1_AD2:
		case L_30V_5A_R1_AD2:	
		case L_60V_100A_R1_AD2:	//LS Mtron
		case L_15V_100A_R3_AD2:
		case L_5V_200A_R3_P:
		case L_5V_200A_R3_P_AD2:
		case L_5V_400A_R3:
		case L_5V_220A_R2:
		case L_20V_110A_R2:
		case L_20V_50A_R2_1:
		case L_5V_120A_R3:
		case L_20V_300A_R2:
		case L_40V_300A_R2:
		case L_30V_40A_R2:
		case L_30V_40A_R2_OT_20:
		case L_30V_40A_R2_P_AD2:
		case L_5V_100A_R2_1:
		case L_5V_150A_R3:
		case L_5V_300A_R3:
		case L_5V_250A_R2:
		case L_20V_300A_R2_1:
		case L_5V_1000A_R3:
		case L_3V_200A_R2:
		case L_16V_200A_R2:
		case L_10V_50A_R2:
		case S_5V_200A_75A_15A_AD2:
		case L_5V_60A_R2_1:
		case L_5V_150A_R2_P:
		case L_MAIN_REV11:
		case L_8CH_MAIN_AD2_P:
		case L_5V_20A_R3_NEW:
		case L_5V_500A_R3_1:
		case DC_5V_150A_PARA:					//180611 add for digital
		case DC_5V_CYCLER_NEW:					//180611 add for digital
		case L_5V_30A_R3_HYUNDAI:               //20190731 add
		case C_5V_CYCLER_CAN:					//201229 lyhw Check
		case L_20V_6A_R3: 	//210813 ljsw LGES 20V6A
			rtn = SMPS_FAIL_2();
			break;
		case L_MULTI:							//add for Multi
			rtn = SMPS_FAIL_4();
			break;
		default: 
			break;
	}
	return rtn;
}

int SMPS_FAIL_1(void)
{
	int rtn=0, bd, ch;
	unsigned char flag;

	flag = (unsigned char)Read_InPoint(I_IN_SMPS_FAIL1);
	if(flag == P1) {
		if(myData->dio.signal[DIO_SIG_SMPS_FAIL1] == P0) {
			myData->dio.signal[DIO_SIG_SMPS_FAIL1] = P1;
			myPs->signal[M_SIG_RUN_LED] = P0;
			myPs->signal[M_SIG_REMOTE_SMPS1] = P0;
			myPs->signal[M_SIG_FAN_RELAY] = P0;
			myPs->signal[M_SIG_POWER_OFF] = P1;
			myPs->code = M_FAIL_SMPS;
			send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 5, 0);
			send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
				(int)myPs->code, 0);
			for(bd=0; bd < myPs->config.installedBd; bd++) {
				for(ch=0; ch < myPs->config.chPerBd; ch++) {
					if((myPs->config.chPerBd * bd + ch)
						> (myPs->config.installedCh-1)) {
						continue;
					}
					if(myData->bData[bd].cData[ch].op.state == C_RUN) {
						myData->bData[bd].cData[ch]
							.signal[C_SIG_SMPS_FAULT] = P1;
					}
				}
			}
			rtn = -1;
		}
	} else {
		myData->dio.signal[DIO_SIG_SMPS_FAIL1] = P0;
	}

	flag = (unsigned char)Read_InPoint(I_IN_SMPS_FAIL2);
	if(flag == P1) {
		if(myData->dio.signal[DIO_SIG_SMPS_FAIL2] == P0) {
			myData->dio.signal[DIO_SIG_SMPS_FAIL2] = P1;
			myPs->signal[M_SIG_RUN_LED] = P0;
			myPs->signal[M_SIG_REMOTE_SMPS1] = P0;
			myPs->signal[M_SIG_FAN_RELAY] = P0;
			myPs->signal[M_SIG_POWER_OFF] = P1;
			myPs->code = M_FAIL_SMPS;
			send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 7, 0);
			send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
				(int)myPs->code, 0);
			for(bd=0; bd < myPs->config.installedBd; bd++) {
				for(ch=0; ch < myPs->config.chPerBd; ch++) {
					if((myPs->config.chPerBd * bd + ch)
						> (myPs->config.installedCh-1)) {
						continue;
					}
					if(myData->bData[bd].cData[ch].op.state == C_RUN) {
						myData->bData[bd].cData[ch]
							.signal[C_SIG_SMPS_FAULT] = P1;
					}
				}
			}
			rtn = -1;
		}
	} else {
		myData->dio.signal[DIO_SIG_SMPS_FAIL2] = P0;
	}
	return rtn;
}

int SMPS_FAIL_2(void)
{
	int rtn=0, bd, ch;
	unsigned char flag;

	flag = (unsigned char)Read_InPoint(I_IN_SMPS_FAIL1);
	if(flag == P1) {
#if CYCLER_TYPE == DIGITAL_CYC
		myPs->signal[M_SIG_INV_POWER] = P100;
		myPs->signal[M_SIG_INV_POWER1] = P100;
#endif
		//210303 Inverter off 
		if(myData->AppControl.config.systemType == CYCLER_CAN){ 
			myPs->signal[M_SIG_INV_POWER_CAN] = P100;
		}

		if(myData->dio.signal[DIO_SIG_SMPS_FAIL1] == P0) {
			myData->dio.signal[DIO_SIG_SMPS_FAIL1] = P1;
			myPs->signal[M_SIG_RUN_LED] = P0;
			myPs->signal[M_SIG_REMOTE_SMPS1] = P0;
			myPs->signal[M_SIG_FAN_RELAY] = P0;
			myPs->signal[M_SIG_POWER_OFF] = P1;
			myPs->code = M_FAIL_SMPS;
			send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 5, 0);
			send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
				(int)myPs->code, 0);
			for(bd=0; bd < myPs->config.installedBd; bd++) {
				for(ch=0; ch < myPs->config.chPerBd; ch++) {
					if((myPs->config.chPerBd * bd + ch)
						> (myPs->config.installedCh-1)) {
						continue;
					}
					if(myData->bData[bd].cData[ch].op.state == C_RUN) {
						myData->bData[bd].cData[ch]
							.signal[C_SIG_SMPS_FAULT] = P1;
					}
				}
			}
			rtn = -1;
		}
	} else {
		myData->dio.signal[DIO_SIG_SMPS_FAIL1] = P0;
	}
	return rtn;
}

int SMPS_FAIL_3(void)
{
	int rtn=0, bd, ch;
	unsigned char flag;

	flag = (unsigned char)Read_InPoint(I_IN_SMPS_FAIL1);
	if(flag == P1) {
		if(myData->dio.signal[DIO_SIG_SMPS_FAIL1] == P0) {
			myData->dio.signal[DIO_SIG_SMPS_FAIL1] = P1;
			myPs->signal[M_SIG_RUN_LED] = P0;
			myPs->signal[M_SIG_REMOTE_SMPS1] = P0;
			myPs->signal[M_SIG_FAN_RELAY] = P0;
			myPs->signal[M_SIG_POWER_OFF] = P1;
			myPs->code = M_FAIL_SMPS;
			send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 5, flag);
			send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
				(int)myPs->code, 0);
			for(bd=0; bd < myPs->config.installedBd; bd++) {
				for(ch=0; ch < myPs->config.chPerBd; ch++) {
					if((myPs->config.chPerBd * bd + ch)
						> (myPs->config.installedCh-1)) {
						continue;
					}
					if(myData->bData[bd].cData[ch].op.state == C_RUN) {
						myData->bData[bd].cData[ch]
							.signal[C_SIG_SMPS_FAULT] = P1;
					}
				}
			}
			rtn = -1;
		}
	} else {
		myData->dio.signal[DIO_SIG_SMPS_FAIL1] = P0;
	}
	return rtn;
}
//pms add for Multi
int SMPS_FAIL_4(void)
{
	int rtn=0, bd, ch;
	unsigned char flag, flag2, flag3;

	flag = (unsigned char)Read_InPoint(I_IN_SMPS_FAIL1);
	flag2 = (unsigned char)Read_InPoint(I_IN_SMPS_FAIL2);
	flag3 = (unsigned char)Read_InPoint(I_IN_SMPS_FAIL3);
	if(flag == P1) {
		if(myData->dio.signal[DIO_SIG_SMPS_FAIL1] == P0) {
			myData->dio.signal[DIO_SIG_SMPS_FAIL1] = P1;
			myPs->signal[M_SIG_RUN_LED] = P0;
			myPs->signal[M_SIG_REMOTE_SMPS1] = P0;
			myPs->signal[M_SIG_FAN_RELAY] = P0;
			myPs->signal[M_SIG_POWER_OFF] = P1;
			myPs->code = M_FAIL_SMPS;
			send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 5, 0);
			send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
				(int)myPs->code, 0);
			for(bd=0; bd < myPs->config.installedBd; bd++) {
				for(ch=0; ch < myPs->config.chPerBd; ch++) {
					if((myPs->config.chPerBd * bd + ch)
						> (myPs->config.installedCh-1)) {
						continue;
					}
					if(myData->bData[bd].cData[ch].op.state == C_RUN) {
						myData->bData[bd].cData[ch]
							.signal[C_SIG_SMPS_FAULT] = P1;
					}
				}
			}
			rtn = -1;
		}
	}else if(flag2 == P1){ 		
		if(myData->dio.signal[DIO_SIG_SMPS_FAIL1] == P0) {
			myData->dio.signal[DIO_SIG_SMPS_FAIL1] = P1;
			myPs->signal[M_SIG_RUN_LED] = P0;
			myPs->signal[M_SIG_REMOTE_SMPS1] = P0;
			myPs->signal[M_SIG_FAN_RELAY] = P0;
			myPs->signal[M_SIG_POWER_OFF] = P1;
			myPs->code = M_FAIL_SMPS;
			send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 5, 0);
			send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
				(int)myPs->code, 0);
			for(bd=0; bd < myPs->config.installedBd; bd++) {
				for(ch=0; ch < myPs->config.chPerBd; ch++) {
					if((myPs->config.chPerBd * bd + ch)
						> (myPs->config.installedCh-1)) {
						continue;
					}
					if(myData->bData[bd].cData[ch].op.state == C_RUN) {
						myData->bData[bd].cData[ch]
							.signal[C_SIG_SMPS_FAULT] = P1;
					}
				}
			}
			rtn = -1;
		}
	}else if(flag3 == P1){		
		if(myData->dio.signal[DIO_SIG_SMPS_FAIL1] == P0) {
			myData->dio.signal[DIO_SIG_SMPS_FAIL1] = P1;
			myPs->signal[M_SIG_RUN_LED] = P0;
			myPs->signal[M_SIG_REMOTE_SMPS1] = P0;
			myPs->signal[M_SIG_FAN_RELAY] = P0;
			myPs->signal[M_SIG_POWER_OFF] = P1;
			myPs->code = M_FAIL_SMPS;
			send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 5, 0);
			send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
				(int)myPs->code, 0);
			for(bd=0; bd < myPs->config.installedBd; bd++) {
				for(ch=0; ch < myPs->config.chPerBd; ch++) {
					if((myPs->config.chPerBd * bd + ch)
						> (myPs->config.installedCh-1)) {
						continue;
					}
					if(myData->bData[bd].cData[ch].op.state == C_RUN) {
						myData->bData[bd].cData[ch]
							.signal[C_SIG_SMPS_FAULT] = P1;
					}
				}
			}
			rtn = -1;
		}
	}else {
		myData->dio.signal[DIO_SIG_SMPS_FAIL1] = P0;
	}
	return rtn;
}//end of add

//20170622 sch Modify Not Use ChamberType
void DIn_FlagCheck_ChamberFail(void)
{
	int i, j,  bd, ch, cham_err_no = 0;
	unsigned char flag = 0, Chamber_type, Chamber_err_proc;
		
	if(myData->AppControl.config.debugType != P0) return;
	Chamber_type = myPs->config.function[F_CHAMBER_TYPE];
	Chamber_err_proc = myPs->config.function[F_CHAMBER_ERR_PROC];

	switch(Chamber_type){	//190731 lyhw
		case P1:
			cham_err_no = 16;
			break;
		case P2:
			cham_err_no = 6;
			break;
		case P3:	
			cham_err_no = 16;
			break;
		default:
			break;
	}
	
	/*
	if(myData->mData.config.function[F_CHAMBER_TYPE] == 1){
		cham_err_no = 16;
	}else if(myData->mData.config.function[F_CHAMBER_TYPE] == 2){
		cham_err_no = 6;
	}else if(myData->mData.config.function[F_CHAMBER_TYPE] == 3){
		//190731 lyhw for digital
		cham_err_no = 16;
	}*/

	for(i=0;i < cham_err_no ;i++) {
		if(Chamber_type == P1){
			flag = (unsigned char)Read_InPoint(I_IN_CHAMBER_ERROR1 + i);
		}else if(Chamber_type == P2){
			flag = (unsigned char)Read_InPoint(I_IN_CHAMBER_ERROR17 + i);
		}
		
		if(flag == P1){
			//Error Chamber Use Ch Pause Process
			if(Chamber_err_proc == P1 || Chamber_err_proc == P3){
				if(myData->dio.signal[DIO_SIG_CHAMBER_ERROR1 + i] == P0) {
					myData->dio.signal[DIO_SIG_CHAMBER_ERROR1 + i] = P1;
					myPs->code = M_FAIL_CHAMBER_ERROR1 + i;
					send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
						(int)myPs->code, 0);
					for(j=0; j < MAX_CH_PER_MODULE; j++) {
						if(myData->ChamArray[j].number1 == 0)
							continue;
						if(i == myData->ChamArray[j].number1-1) {
							bd = myData->ChamArray[j].bd;
							ch = myData->ChamArray[j].ch;
							if((myPs->config.chPerBd * bd + ch)
								> (myPs->config.installedCh-1)) {
								continue;
							}
							if(myData->bData[bd].cData[ch].op.state == C_RUN) {
								myData->bData[bd].cData[ch]
									.signal[C_SIG_CHAMBER_ERROR] = P1;
							}
						}
					}
				}
				return;
			} else {
				if(myData->dio.signal[DIO_SIG_CHAMBER_ERROR1 + i] == P0) {
					myData->dio.signal[DIO_SIG_CHAMBER_ERROR1 + i] = P1;
					myPs->code = M_FAIL_CHAMBER_ERROR1 + i;
					if(myData->mData.config.ChamberMotion[i] == 1) {
						#if CYCLER_TYPE == DIGITAL_CYC
						myPs->signal[M_SIG_INV_POWER] = P100;
						myPs->signal[M_SIG_INV_POWER1] = P100;
						#endif
						//210303 Inverter off 
						if(myData->AppControl.config.systemType == CYCLER_CAN){ 
							myPs->signal[M_SIG_INV_POWER_CAN] = P100;
						}

						myPs->signal[M_SIG_RUN_LED] = P0;
						myPs->signal[M_SIG_REMOTE_SMPS1] = P0;
						myPs->signal[M_SIG_FAN_RELAY] = P0;
						myPs->signal[M_SIG_POWER_OFF] = P1;
						send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 17, 0);
					}
					send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
						(int)myPs->code, 0);
					for(bd=0; bd < myPs->config.installedBd; bd++) {
						for(ch=0; ch < myPs->config.chPerBd; ch++) {
							if((myPs->config.chPerBd * bd + ch)
								> (myPs->config.installedCh-1)) {
								continue;
							}
							if(myData->bData[bd].cData[ch].op.state == C_RUN) {
								myData->bData[bd].cData[ch]
									.signal[C_SIG_CHAMBER_ERROR] = P1;
							}
						}
					}
				} else {
					for(bd=0; bd < myPs->config.installedBd; bd++) {
						for(ch=0; ch < myPs->config.chPerBd; ch++) {
							if((myPs->config.chPerBd * bd + ch)
								> (myPs->config.installedCh-1)) {
								continue;
								}
							if(myData->bData[bd].cData[ch].op.state == C_RUN) {
								myData->bData[bd].cData[ch]
									.signal[C_SIG_CHAMBER_ERROR] = P1;
							}
						}
					}
				}
				return;
			}
		}else if(flag == P2){	//190731 lyhw
			if(Chamber_err_proc == P3){
				if(myData->dio.signal[DIO_SIG_CHAMBER_ERROR1 + i] == P0) {
					myData->dio.signal[DIO_SIG_CHAMBER_ERROR1 + i] = P1;
					myPs->code = M_FAIL_CHAMBER_ERROR1 + i;
					if(myData->mData.config.ChamberMotion[i] == 1) {
						#if CYCLER_TYPE == DIGITAL_CYC
						myPs->signal[M_SIG_INV_POWER] = P100;
						myPs->signal[M_SIG_INV_POWER1] = P100;
						#endif
						//210303 Inverter off 
						if(myData->AppControl.config.systemType == CYCLER_CAN){ 
							myPs->signal[M_SIG_INV_POWER_CAN] = P100;
						}

						myPs->signal[M_SIG_RUN_LED] = P0;
						myPs->signal[M_SIG_REMOTE_SMPS1] = P0;
						myPs->signal[M_SIG_FAN_RELAY] = P0;
						myPs->signal[M_SIG_POWER_OFF] = P1;
						send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 17, 0);
					}
					send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
						(int)myPs->code, 0);
					for(bd=0; bd < myPs->config.installedBd; bd++) {
						for(ch=0; ch < myPs->config.chPerBd; ch++) {
							if((myPs->config.chPerBd * bd + ch)
								> (myPs->config.installedCh-1)) {
								continue;
							}
							if(myData->bData[bd].cData[ch].op.state == C_RUN) {
								myData->bData[bd].cData[ch]
									.signal[C_SIG_CHAMBER_ERROR] = P1;
							}
						}
					}
				} else {
					for(bd=0; bd < myPs->config.installedBd; bd++) {
						for(ch=0; ch < myPs->config.chPerBd; ch++) {
							if((myPs->config.chPerBd * bd + ch)
								> (myPs->config.installedCh-1)) {
								continue;
								}
							if(myData->bData[bd].cData[ch].op.state == C_RUN) {
								myData->bData[bd].cData[ch]
									.signal[C_SIG_CHAMBER_ERROR] = P1;
							}
						}
					}
				}
				return;
			}
		}else{
			myData->dio.signal[DIO_SIG_CHAMBER_ERROR1 + i] = P0;
		}
	}
}

void DIn_FlagCheck_ChamberFail2(void)
{
	unsigned char flag = 0, Total_SigNum, chamber_Motion, value1;
	int i, j, bd, ch, chamberNo;

	if(myData->mData.config.function[F_CHAMBER_TYPE] == P0) return;
	
	Total_SigNum = myPs->config.ChamberPerUnit * myPs->config.ChamberPerSigNum;
	bd = ch = chamberNo = 0;

	for(i = 0; i < Total_SigNum; i++) {
		if(myData->mData.config.function[F_CHAMBER_TYPE] == 1){
			flag = (unsigned char)Read_InPoint(I_IN_CHAMBER_ERROR1 + i);
		}else if(myData->mData.config.function[F_CHAMBER_TYPE] == 2){
			flag = (unsigned char)Read_InPoint(I_IN_CHAMBER_ERROR17 + i);
		}
		
		if(flag == P1 && myPs->config.function[F_CHAMBER_ERR_PROC] == P1){
			value1 = i / myPs->config.ChamberPerSigNum;

			if(myData->dio.signal[DIO_SIG_CHAMBER_ERROR1 + i] == P0){
				myData->dio.signal[DIO_SIG_CHAMBER_ERROR1 + i] = P1;
				//210805 lyhw
				myPs->code = M_FAIL_DC_CHAMBER_FAULT1
						+ (i - (myPs->config.ChamberPerSigNum * value1));

				//2.Chamber Error Unit Shut Down.
				chamberNo = value1 + 1;
				chamber_Motion = myData->mData.config.ChamberMotion[i];
				switch(chamber_Motion){
					case 0:
						for(j=0; j < MAX_CH_PER_MODULE; j++) {
							if(myData->ChamArray[j].number1 == 0)	continue;
							if(chamberNo == myData->ChamArray[j].number1){
								bd = myData->ChamArray[j].bd;
								ch = myData->ChamArray[j].ch;
								if((myPs->config.chPerBd * bd + ch)
									> (myPs->config.installedCh-1)) {
									continue;
								}
								if(myData->bData[bd].cData[ch].op.state
															== C_RUN){
									myData->bData[bd].cData[ch]
									.signal[C_SIG_CHAMBER_ERROR] = P1;
								}
							}
						}
						break;
					case 1:
						#if CYCLER_TYPE == DIGITAL_CYC
						myPs->signal[M_SIG_INV_POWER] = P100;
						myPs->signal[M_SIG_INV_POWER1] = P100;
						#endif
						//210303 Inverter off 
						if(myData->AppControl.config.systemType == CYCLER_CAN){ 
							myPs->signal[M_SIG_INV_POWER_CAN] = P100;
						}
						myPs->signal[M_SIG_RUN_LED] = P0;
						myPs->signal[M_SIG_REMOTE_SMPS1] = P0;
						myPs->signal[M_SIG_FAN_RELAY] = P0;
						myPs->signal[M_SIG_POWER_OFF] = P1;
						send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 17, 0);
						for(j=0; j < MAX_CH_PER_MODULE; j++){
							if(myData->ChamArray[j].number1 == 0)	continue;
							bd = myData->ChamArray[j].bd;
							ch = myData->ChamArray[j].ch;
							if(myData->bData[bd].cData[ch].op.state == C_RUN){
								myData->bData[bd].cData[ch]
										.signal[C_SIG_CHAMBER_ERROR] = P1;
							}
						}
						break;
					default:
						break;
				}
				send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
					(int)myPs->code, chamberNo);
			}else if(myData->dio.signal[DIO_SIG_CHAMBER_ERROR1 + i] == P1){
				chamberNo = value1 + 1;
				for(j = 0; j < MAX_CH_PER_MODULE; j++) {
					if(myData->ChamArray[j].number1 == 0)	continue;
					if(chamberNo == myData->ChamArray[j].number1){
						bd = myData->ChamArray[j].bd;
						ch = myData->ChamArray[j].ch;
						if(myData->bData[bd].cData[ch].op.state == C_RUN){
							myData->dio.signal[DIO_SIG_CHAMBER_ERROR1 + i] = P0;
						}
					}
				}
			}
		//210805 lyhw not use
		//	return;
		}else{
			myData->dio.signal[DIO_SIG_CHAMBER_ERROR1 + i] = P0;
		}
	}
}

/*
void DIn_FlagCheck_ChamberFail(void)
{
#ifdef _CHAMBERTYPE_1
	int i, bd, ch;
	unsigned char flag;
	for(i=0;i<10;i++) {
		flag = Read_InPoint(I_IN_CHAMBER_DOOR_OPEN + i);
		if(flag == P1){
			if(myData->dio.signal[DIO_SIG_CHAMBER_DOOR_OPEN + i] == P0) {
				myData->dio.signal[DIO_SIG_CHAMBER_DOOR_OPEN + i] = P1;
				myPs->code = M_FAIL_CHAMBER_DOOR_OPEN + i;
				send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
					(int)myPs->code, 0);
				for(bd=0; bd < myPs->config.installedBd; bd++) {
					for(ch=0; ch < myPs->config.chPerBd; ch++) {
						if((myPs->config.chPerBd * bd + ch)
							> (myPs->config.installedCh-1)) {
							continue;
						}
						if(myData->bData[bd].cData[ch].op.state == C_RUN) {
							myData->bData[bd].cData[ch]
								.signal[C_SIG_CHAMBER_ERROR] = P1;
						}
					}
				}
			} else {
				for(bd=0; bd < myPs->config.installedBd; bd++) {
					for(ch=0; ch < myPs->config.chPerBd; ch++) {
						if((myPs->config.chPerBd * bd + ch)
							> (myPs->config.installedCh-1)) {
							continue;
						}
						if(myData->bData[bd].cData[ch].op.state == C_RUN) {
							myData->bData[bd].cData[ch]
								.signal[C_SIG_CHAMBER_ERROR] = P1;
						}
					}
				}
			}
			return;
		} else {
			myData->dio.signal[DIO_SIG_CHAMBER_DOOR_OPEN + i] = P0;
		}
	}
#endif
#ifdef _CHAMBERTYPE_2
	int i, bd, ch;
	unsigned char flag;
	for(i=0;i<12;i++) {
		flag = Read_InPoint(I_IN_CHAMBER_MAINPOWER + i);
		if(flag == P1){
			if(myData->dio.signal[DIO_SIG_CHAMBER_MAINPOWER + i] == P0) {
				myData->dio.signal[DIO_SIG_CHAMBER_MAINPOWER + i] = P1;
				myPs->code = M_FAIL_CHAMBER_MAINPOWER + i;
				send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
					(int)myPs->code, 0);
				for(bd=0; bd < myPs->config.installedBd; bd++) {
					for(ch=0; ch < myPs->config.chPerBd; ch++) {
						if((myPs->config.chPerBd * bd + ch)
							> (myPs->config.installedCh-1)) {
							continue;
						}
						if(myData->bData[bd].cData[ch].op.state == C_RUN) {
							myData->bData[bd].cData[ch]
								.signal[C_SIG_CHAMBER_ERROR] = P1;
						}
					}
				}
			} else {
				for(bd=0; bd < myPs->config.installedBd; bd++) {
					for(ch=0; ch < myPs->config.chPerBd; ch++) {
						if((myPs->config.chPerBd * bd + ch)
							> (myPs->config.installedCh-1)) {
							continue;
							}
						if(myData->bData[bd].cData[ch].op.state == C_RUN) {
							myData->bData[bd].cData[ch]
								.signal[C_SIG_CHAMBER_ERROR] = P1;
						}
					}
				}
			}
			return;
		} else {
			myData->dio.signal[DIO_SIG_CHAMBER_MAINPOWER + i] = P0;
		}
	}
#endif
#ifdef _CHAMBERTYPE_3
	int i, bd, ch;
	unsigned char flag;
	for(i=0;i<16;i++) {
		flag = Read_InPoint(I_IN_CHAMBER_DOOR_OPEN + i);
		if(flag == P1){
			if(myData->dio.signal[DIO_SIG_CHAMBER_DOOR_OPEN + i] == P0) {
				myData->dio.signal[DIO_SIG_CHAMBER_DOOR_OPEN + i] = P1;
				myPs->code = M_FAIL_CHAMBER_DOOR_OPEN + (i%8);
				send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
					(int)myPs->code, 0);
				for(bd=0; bd < myPs->config.installedBd; bd++) {
					for(ch=0; ch < myPs->config.chPerBd; ch++) {
						if((myPs->config.chPerBd * bd + ch)
							> (myPs->config.installedCh-1)) {
							continue;
						}
						if(myData->bData[bd].cData[ch].op.state == C_RUN) {
							myData->bData[bd].cData[ch]
								.signal[C_SIG_CHAMBER_ERROR] = P1;
						}
					}
				}
			} else {
				for(bd=0; bd < myPs->config.installedBd; bd++) {
					for(ch=0; ch < myPs->config.chPerBd; ch++) {
						if((myPs->config.chPerBd * bd + ch)
							> (myPs->config.installedCh-1)) {
							continue;
							}
						if(myData->bData[bd].cData[ch].op.state == C_RUN) {
							myData->bData[bd].cData[ch]
								.signal[C_SIG_CHAMBER_ERROR] = P1;
						}
					}
				}
			}
			return;
		} else {
			myData->dio.signal[DIO_SIG_CHAMBER_DOOR_OPEN + i] = P0;
		}
	}
#endif
#ifdef _CHAMBERTYPE_4
	int i, bd, ch;
	unsigned char flag;
	for(i=0;i<8;i++) {
		flag = Read_InPoint(I_IN_CHAMBER_RUN_STOP + i);
		if(flag == P1){
			if(myData->dio.signal[DIO_SIG_CHAMBER_RUN_STOP + i] == P0) {
				myData->dio.signal[DIO_SIG_CHAMBER_RUN_STOP + i] = P1;
				myPs->code = M_FAIL_CHAMBER_RUN_STOP + i;
				if(myPs->code == M_FAIL_CHAMBER_FIRE_ERROR) {
					myPs->signal[M_SIG_RUN_LED] = P0;
					myPs->signal[M_SIG_REMOTE_SMPS1] = P0;
					myPs->signal[M_SIG_FAN_RELAY] = P0;
					myPs->signal[M_SIG_POWER_OFF] = P1;
					send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 17, 0);
				}
				send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
					(int)myPs->code, 0);
				for(bd=0; bd < myPs->config.installedBd; bd++) {
					for(ch=0; ch < myPs->config.chPerBd; ch++) {
						if((myPs->config.chPerBd * bd + ch)
							> (myPs->config.installedCh-1)) {
							continue;
						}
						if(myData->bData[bd].cData[ch].op.state == C_RUN) {
							myData->bData[bd].cData[ch]
								.signal[C_SIG_CHAMBER_ERROR] = P1;
						}
					}
				}
			} else {
				for(bd=0; bd < myPs->config.installedBd; bd++) {
					for(ch=0; ch < myPs->config.chPerBd; ch++) {
						if((myPs->config.chPerBd * bd + ch)
							> (myPs->config.installedCh-1)) {
							continue;
							}
						if(myData->bData[bd].cData[ch].op.state == C_RUN) {
							myData->bData[bd].cData[ch]
								.signal[C_SIG_CHAMBER_ERROR] = P1;
						}
					}
				}
			}
			return;
		} else {
			myData->dio.signal[DIO_SIG_CHAMBER_RUN_STOP + i] = P0;
		}
	}
#endif
#ifdef _CHAMBERTYPE_5
	int i, bd, ch;
	unsigned char flag;
	for(i=0;i<8;i++) {
		flag = Read_InPoint(I_IN_CHAMBER_ERROR1 + i);
		if(flag == P1){
			if(myData->dio.signal[DIO_SIG_CHAMBER_ERROR1 + i] == P0) {
				myData->dio.signal[DIO_SIG_CHAMBER_ERROR1 + i] = P1;
				myPs->code = M_FAIL_CHAMBER_ERROR1 + i;
				myPs->signal[M_SIG_RUN_LED] = P0;
				myPs->signal[M_SIG_REMOTE_SMPS1] = P0;
				myPs->signal[M_SIG_FAN_RELAY] = P0;
				myPs->signal[M_SIG_POWER_OFF] = P1;
				send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 17, 0);
				send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
					(int)myPs->code, 0);
				for(bd=0; bd < myPs->config.installedBd; bd++) {
					for(ch=0; ch < myPs->config.chPerBd; ch++) {
						if((myPs->config.chPerBd * bd + ch)
							> (myPs->config.installedCh-1)) {
							continue;
						}
						if(myData->bData[bd].cData[ch].op.state == C_RUN) {
							myData->bData[bd].cData[ch]
								.signal[C_SIG_CHAMBER_ERROR] = P1;
						}
					}
				}
			} else {
				for(bd=0; bd < myPs->config.installedBd; bd++) {
					for(ch=0; ch < myPs->config.chPerBd; ch++) {
						if((myPs->config.chPerBd * bd + ch)
							> (myPs->config.installedCh-1)) {
							continue;
							}
						if(myData->bData[bd].cData[ch].op.state == C_RUN) {
							myData->bData[bd].cData[ch]
								.signal[C_SIG_CHAMBER_ERROR] = P1;
						}
					}
				}
			}
			return;
		} else {
			myData->dio.signal[DIO_SIG_CHAMBER_ERROR1 + i] = P0;
		}
	}
#endif
// 160120 oys add : Error Chamber Use Ch Pause or Unit Shutdown
#ifdef _CHAMBERTYPE_6
	int i, j, bd, ch;
	unsigned char flag;

	for(i=0;i<16;i++) {
		flag = Read_InPoint(I_IN_CHAMBER_ERROR1 + i);

		if(flag == P1){
			//Error Chamber Use Ch Pause Process
			if(myPs->config.function[F_CHAMBER_ERR_PROC] == P1)
			{
				if(myData->dio.signal[DIO_SIG_CHAMBER_ERROR1 + i] == P0) {
					myData->dio.signal[DIO_SIG_CHAMBER_ERROR1 + i] = P1;
					myPs->code = M_FAIL_CHAMBER_ERROR;
					send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
						(int)myPs->code, 0);
					for(j=0; j < MAX_CH_PER_MODULE; j++) {
						if(myData->ChamArray[j].number1 == 0)
							continue;
						if(i == myData->ChamArray[j].number1-1) {
							bd = myData->ChamArray[j].bd;
							ch = myData->ChamArray[j].ch;
							if((myPs->config.chPerBd * bd + ch)
								> (myPs->config.installedCh-1)) {
								continue;
							}
							if(myData->bData[bd].cData[ch].op.state == C_RUN) {
								myData->bData[bd].cData[ch]
									.signal[C_SIG_CHAMBER_ERROR] = P1;
							}
						}
					}
				}
				return;
			//Unit Shutdown Process
			} else {
				if(myData->dio.signal[DIO_SIG_CHAMBER_ERROR1 + i] == P0) {
					myData->dio.signal[DIO_SIG_CHAMBER_ERROR1 + i] = P1;
					myPs->code = M_FAIL_CHAMBER_ERROR;
					myPs->signal[M_SIG_RUN_LED] = P0;
					myPs->signal[M_SIG_REMOTE_SMPS1] = P0;
					myPs->signal[M_SIG_FAN_RELAY] = P0;
					myPs->signal[M_SIG_POWER_OFF] = P1;
					send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 17, 0);
					send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
						(int)myPs->code, 0);
					for(bd=0; bd < myPs->config.installedBd; bd++) {
						for(ch=0; ch < myPs->config.chPerBd; ch++) {
							if((myPs->config.chPerBd * bd + ch)
								> (myPs->config.installedCh-1)) {
								continue;
							}
							if(myData->bData[bd].cData[ch].op.state == C_RUN) {
								myData->bData[bd].cData[ch]
									.signal[C_SIG_CHAMBER_ERROR] = P1;
							}
						}
					}
				} else {
					for(bd=0; bd < myPs->config.installedBd; bd++) {
						for(ch=0; ch < myPs->config.chPerBd; ch++) {
							if((myPs->config.chPerBd * bd + ch)
								> (myPs->config.installedCh-1)) {
								continue;
								}
							if(myData->bData[bd].cData[ch].op.state == C_RUN) {
								myData->bData[bd].cData[ch]
									.signal[C_SIG_CHAMBER_ERROR] = P1;
							}
						}
					}
				}
				return;
			}
		} else {
			myData->dio.signal[DIO_SIG_CHAMBER_ERROR1 + i] = P0;
		}
	}
#endif
#ifdef _CHAMBERTYPE_7
	int i, bd, ch;
	unsigned char flag;
	for(i=0;i<8;i++) {
		flag = Read_InPoint(I_IN_CHAMBER_DOOR_OPEN + i);
		if(flag == P1){
			if(myData->dio.signal[DIO_SIG_CHAMBER_DOOR_OPEN + i] == P0) {
				myData->dio.signal[DIO_SIG_CHAMBER_DOOR_OPEN + i] = P1;
				myPs->code = M_FAIL_CHAMBER_DOOR_OPEN + i;
				if(myPs->code == M_FAIL_CHAMBER_OVER_TEMP || myPs->code == M_FAIL_CHAMBER_AC_FAIL || myPs->code == M_FAIL_CHAMBER_FIRE_ERROR ) {
					myPs->signal[M_SIG_RUN_LED] = P0;
					myPs->signal[M_SIG_REMOTE_SMPS1] = P0;
					myPs->signal[M_SIG_FAN_RELAY] = P0;
					myPs->signal[M_SIG_POWER_OFF] = P1;
					send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 17, 0);
				}
				send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
					(int)myPs->code, 0);
				for(bd=0; bd < myPs->config.installedBd; bd++) {
					for(ch=0; ch < myPs->config.chPerBd; ch++) {
						if((myPs->config.chPerBd * bd + ch)
							> (myPs->config.installedCh-1)) {
							continue;
						}
						if(myData->bData[bd].cData[ch].op.state == C_RUN) {
							myData->bData[bd].cData[ch]
								.signal[C_SIG_CHAMBER_ERROR] = P1;
						}
					}
				}
			} else {
				for(bd=0; bd < myPs->config.installedBd; bd++) {
					for(ch=0; ch < myPs->config.chPerBd; ch++) {
						if((myPs->config.chPerBd * bd + ch)
							> (myPs->config.installedCh-1)) {
							continue;
							}
						if(myData->bData[bd].cData[ch].op.state == C_RUN) {
							myData->bData[bd].cData[ch]
								.signal[C_SIG_CHAMBER_ERROR] = P1;
						}
					}
				}
			}
			return;
		} else {
			myData->dio.signal[DIO_SIG_CHAMBER_DOOR_OPEN + i] = P0;
		}
	}
#endif

}
*/
/*
int DIn_FlagCheck_ChamberFail1(void)
{
	int rtn=0, i, bd, ch;
	unsigned char flag;

	for(i=0;i<10;i++) {
		flag = Read_InPoint(I_IN_CHAMBER_MAINPOWER + i);
		if(flag == P1){
			if(myData->dio.signal[DIO_SIG_CHAMBER_MAINPOWER + i] == P0) {
				myData->dio.signal[DIO_SIG_CHAMBER_MAINPOWER + i] = P1;
				myPs->code = M_FAIL_CHAMBER_MAINPOWER + i;
				send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
					(int)myPs->code, 0);
			//	if(myPs->code == M_FAIL_CHAMBER_UV_ERROR){}
			//	else if(myPs->code == M_FAIL_CHAMBER_SMOKE_ERROR){}
				for(bd=0; bd < myPs->config.installedBd; bd++) {
					for(ch=0; ch < myPs->config.chPerBd; ch++) {
						if((myPs->config.chPerBd * bd + ch)
							> (myPs->config.installedCh-1)) {
							continue;
						}
						if(myData->bData[bd].cData[ch].op.state == C_RUN) {
							myData->bData[bd].cData[ch]
								.signal[C_SIG_CHAMBER_ERROR] = P1;
						}
					}
				}
			} else {
			//	if(myPs->code == M_FAIL_CHAMBER_UV_ERROR){}
			//	else if(myPs->code == M_FAIL_CHAMBER_SMOKE_ERROR){}
				for(bd=0; bd < myPs->config.installedBd; bd++) {
					for(ch=0; ch < myPs->config.chPerBd; ch++) {
						if((myPs->config.chPerBd * bd + ch)
							> (myPs->config.installedCh-1)) {
							continue;
						}
						if(myData->bData[bd].cData[ch].op.state == C_RUN) {
							myData->bData[bd].cData[ch]
								.signal[C_SIG_CHAMBER_ERROR] = P1;
						}
					}
				}
			}
		} else {
			myData->dio.signal[DIO_SIG_CHAMBER_MAINPOWER + i] = P0;
		}
	}
	return rtn;
}
*/
void DIn_Limit_Check()
{
#ifdef _CHAMBERTYPE_1
	int bd , ch;
	unsigned char  flag;

	for(ch=0; ch < myPs->config.installedCh;ch++) {
		flag = (unsigned char)Read_InPoint(I_IN_LIMIT_ERROR1 + (ch/2));
		bd = ch / myPs->config.chPerBd;
		if(flag == P1)
			myData->bData[bd].cData[ch].signal[C_SIG_LIMIT_ERROR] = P1;
		else
			myData->bData[bd].cData[ch].signal[C_SIG_LIMIT_ERROR] = P0;
	}
#endif
}

void DIn_Buzzer_Stop()
{
#ifdef _JIG_TYPE_0
	int bd,ch,i;
	long long flag;
	unsigned char flag1;
	flag = 1;
	flag1 = (unsigned char)Read_InPoint(I_IN_BUZZER_STOP);
	if(flag1 == 1 && myPs->signal[M_SIG_LAMP_BUZZER] == P1) {
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
#endif
}

void DIn_Jig_Check(void)
{
#ifdef _JIG_TYPE_1
	int i, bd, ch;
	unsigned char flag;
	flag = Read_InPoint(I_IN_JIG_START);
	if(flag == P1) {
		if(myData->dio.signal[DIO_SIG_JIG_START] == P0) {
			myData->dio.signal[DIO_SIG_JIG_START] = P1;
			if(myPs->state == M_STANDBY) {
				myPs->signal[M_SIG_LAMP_RUN] = P1;
				send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_JIG_STATUS,
					ON, 0);
			}
	 	}		
	} else {
	  myData->dio.signal[DIO_SIG_JIG_START] = P0;
	}

	flag = Read_InPoint(I_IN_JIG_STOP);
	if(flag == P1) {
		if(myData->dio.signal[DIO_SIG_JIG_STOP] == P0) {
			myData->dio.signal[DIO_SIG_JIG_STOP] = P1;
			for(bd=0; bd < myPs->config.installedBd; bd++) {
				for(ch=0; ch < myPs->config.chPerBd; ch++) {
					if((myPs->config.chPerBd * bd + ch)
						> (myPs->config.installedCh-1)) {
						continue;
					}
					if(myData->bData[bd].cData[ch].op.state == C_RUN) {
						myData->bData[bd].cData[ch].op.state = C_IDLE;
						myData->bData[bd].cData[ch].op.phase = P0;
					}
				}
			}
	 	}		
	} else {
	  myData->dio.signal[DIO_SIG_JIG_STOP] = P0;
	}

	for(i=0;i<4;i++) {
		flag = Read_InPoint(I_IN_JIG_EMG + i);
		if(flag == P1){
			if(myData->dio.signal[DIO_SIG_JIG_EMG + i] == P0) {
				myData->dio.signal[DIO_SIG_JIG_EMG + i] = P1;
				for(bd=0; bd < myPs->config.installedBd; bd++) {
					for(ch=0; ch < myPs->config.chPerBd; ch++) {
						if((myPs->config.chPerBd * bd + ch)
							> (myPs->config.installedCh-1)) {
							continue;
						}
						if(myData->bData[bd].cData[ch].op.state == C_RUN) {
							myData->bData[bd].cData[ch]
								.signal[C_SIG_JIG_ERROR] = P1;
							myPs->signal[M_SIG_JIG_BUZZER]  = P1;
							myPs->code = M_FAIL_JIG_EMG + i;
							send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
								(int)myPs->code, 0);
						}
					}
				}
			} else {
				for(bd=0; bd < myPs->config.installedBd; bd++) {
					for(ch=0; ch < myPs->config.chPerBd; ch++) {
						if((myPs->config.chPerBd * bd + ch)
							> (myPs->config.installedCh-1)) {
							continue;
						}
						if(myData->bData[bd].cData[ch].op.state == C_RUN) {
							myData->bData[bd].cData[ch]
								.signal[C_SIG_JIG_ERROR] = P1;
						}
					}
				}
			}
//			return;
		} else {
			myData->dio.signal[DIO_SIG_JIG_EMG + i] = P0;
		}
	}

	flag = Read_InPoint(I_IN_JIG_RIGHT_UPPER);
	if(flag == P1) {
		myData->dio.signal[DIO_SIG_JIG_RIGHT_UPPER] = P1;
	} else {
		myData->dio.signal[DIO_SIG_JIG_RIGHT_UPPER] = P0;
	}

	flag = Read_InPoint(I_IN_JIG_LEFT_UPPER);
	if(flag == P1) {
		myData->dio.signal[DIO_SIG_JIG_LEFT_UPPER] = P1;
	} else {
		myData->dio.signal[DIO_SIG_JIG_LEFT_UPPER] = P0;
	}
#endif
}

void DOut_FlagCheck(void)
{
	switch(myPs->config.hwSpec) {
		case L_5V_3A:
		case L_6V_6A:
		case L_5V_2A:
		case L_5V_200A:
		case L_5V_10mA:
		case L_2V_100A:
		case L_5V_100A_R2:
		case S_5V_200A:
			DOut_Flag_1();
			break;
		case L_5V_5A:
		case L_5V_30A:
		case L_2V_60A:
		case L_5V_5A_2:
		case L_5V_50A:
		case L_5V_100A:
			DOut_Flag_2();
			break;
		case L_20V_25A:
		case L_5V_2A_R2:
		case L_5V_4A_R2:
		case L_5V_6A_R3:
		case L_5V_100mA_R2:
		case L_5V_500mA_R2:
		case L_5V_10_30A_R2:
		case L_5V_5A_R2:
		case L_5V_1A_R2:
		case L_10V_5A_R2:
		case L_5V_10A_R3_NEW:
		case L_5V_1A_R3:
		case L_5V_500mA_2uA_R4:
			DOut_Flag_3();
			break;
		case L_5V_50A_R2:
		case L_5V_65A_R3:
			DOut_Flag_4();
			break;
		case S_5V_200A_75A_15A_AD2:
			DOut_Flag_5();
			break;
		case L_5V_500A_R2:
		case L_30V_5A_R1_AD2:
		case L_60V_100A_R1_AD2:	//LS Mtron
		case L_5V_150A_R2_P:
		case L_30V_40A_R2:
		case L_30V_40A_R2_OT_20:
		case L_30V_40A_R2_P_AD2:
		case L_MAIN_REV11:
		case L_20V_6A_R3: 	//210813 ljsw LGES 20V6A
			DOut_Flag_6();
			break;
		case L_5V_200A_1CH_JIG:
			DOut_Flag_7();
			break;
		case L_MULTI: //20181127 Add
		   	DOut_Flag_8();
			break;	
		case DC_5V_150A_PARA:			//180611 add for digital
			DOut_Flag_9();
			break;
		case DC_5V_CYCLER_NEW:			//180611 add for digital
			DOut_Flag_10();
			break;
		case C_5V_CYCLER_CAN:	//210308 lyhw
			DOut_Flag_11();
			break;
		default:	
			DOut_Flag_Default();
			break;
	}
}

void DOut_Flag_1(void)
{

	if(myPs->signal[M_SIG_LAMP_RUN] == P1)
		myData->dio.dout.outFlag[E_OUT_LAMP_RUN] = P1;
	else myData->dio.dout.outFlag[E_OUT_LAMP_RUN] = P0;

	if(myPs->signal[M_SIG_LAMP_STOP] == P1)
		myData->dio.dout.outFlag[E_OUT_LAMP_STOP] = P1;
	else myData->dio.dout.outFlag[E_OUT_LAMP_STOP] = P0;

	if(myPs->signal[M_SIG_CALI_RELAY] == P1)
		myData->dio.dout.outFlag[E_OUT_CALI_RELAY] = P1;
	else myData->dio.dout.outFlag[E_OUT_CALI_RELAY] = P0;

	if(myPs->signal[M_SIG_CALI_RELAY2] == P1)
		myData->dio.dout.outFlag[E_OUT_CALI_RELAY2] = P1;
	else myData->dio.dout.outFlag[E_OUT_CALI_RELAY2] = P0;

	if(myPs->signal[M_SIG_REMOTE_SMPS1] == P1
		&& myData->dio.delayTimer >= 300) { //300mS
		myData->dio.dout.outFlag[I_OUT_REMOTE_SMPS1] = P1;
		myData->dio.dout.outFlag[I_OUT_REMOTE_SMPS1+1] = P1;
	} else {
		myData->dio.dout.outFlag[I_OUT_REMOTE_SMPS1] = P0;
		myData->dio.dout.outFlag[I_OUT_REMOTE_SMPS1+1] = P0;
	}

	if(myPs->signal[M_SIG_RUN_LED] == P1)
		myData->dio.dout.outFlag[I_OUT_RUN_LED] = P1;
	else myData->dio.dout.outFlag[I_OUT_RUN_LED] = P0;
	
	if(myPs->signal[M_SIG_FAN_RELAY] == P1)
		myData->dio.dout.outFlag[I_OUT_FAN_RELAY] = P1;
	else myData->dio.dout.outFlag[I_OUT_FAN_RELAY] = P0;
	
	if(myPs->signal[M_SIG_POWER_OFF] == P1)
		myData->dio.dout.outFlag[I_OUT_POWER_OFF] = P1;
	else myData->dio.dout.outFlag[I_OUT_POWER_OFF] = P0;
}

void DOut_Flag_2(void)
{
	int tmp;

	tmp = FaultLamp_Control();
	if(tmp < 0) {
		if(myPs->signal[M_SIG_CALI_RELAY] == P1)
			myData->dio.dout.outFlag[E_OUT_CALI_RELAY] = P1;
		else myData->dio.dout.outFlag[E_OUT_CALI_RELAY] = P0;

		if(myPs->signal[M_SIG_CALI_RELAY2] == P1)
			myData->dio.dout.outFlag[E_OUT_CALI_RELAY2] = P1;
		else myData->dio.dout.outFlag[E_OUT_CALI_RELAY2] = P0;

		if(myData->dio.dout.outFlag[E_OUT_CALI_RELAY] == P1)
			myData->dio.dout.outSignal[O_SIG_CALI_RELAY] = P1;
		else myData->dio.dout.outSignal[O_SIG_CALI_RELAY] = P0;

		if(myData->dio.dout.outFlag[E_OUT_CALI_RELAY2] == P1)
			myData->dio.dout.outSignal[O_SIG_CALI_RELAY2] = P1;
		else myData->dio.dout.outSignal[O_SIG_CALI_RELAY2] = P0;
	}
	
	if(myPs->signal[M_SIG_REMOTE_SMPS1] == P1
		&& myData->dio.delayTimer >= 300) //300mS
		myData->dio.dout.outFlag[I_OUT_REMOTE_SMPS1] = P1;
	else myData->dio.dout.outFlag[I_OUT_REMOTE_SMPS1] = P0;

	if(myPs->signal[M_SIG_RUN_LED] == P1)
		myData->dio.dout.outFlag[I_OUT_RUN_LED] = P1;
	else myData->dio.dout.outFlag[I_OUT_RUN_LED] = P0;
	
	if(myPs->signal[M_SIG_FAN_RELAY] == P1)
		myData->dio.dout.outFlag[I_OUT_FAN_RELAY] = P1;
	else myData->dio.dout.outFlag[I_OUT_FAN_RELAY] = P0;
	
	if(myPs->signal[M_SIG_POWER_OFF] == P1)
		myData->dio.dout.outFlag[I_OUT_POWER_OFF] = P1;
	else myData->dio.dout.outFlag[I_OUT_POWER_OFF] = P0;
}

void DOut_Flag_3(void)
{
	
    Dout_TowerLamp_Flag();

	if(myPs->signal[M_SIG_REMOTE_SMPS1] == P1
		&& myData->dio.delayTimer >= 300) { //300mS
		myData->dio.dout.outFlag[I_OUT_REMOTE_SMPS1] = P1;
		myData->dio.dout.outFlag[I_OUT_REMOTE_SMPS1+1] = P1;
	} else {
		myData->dio.dout.outFlag[I_OUT_REMOTE_SMPS1] = P0;
		myData->dio.dout.outFlag[I_OUT_REMOTE_SMPS1+1] = P0;
	}

	if(myPs->signal[M_SIG_RUN_LED] == P1)
		myData->dio.dout.outFlag[I_OUT_RUN_LED] = P1;
	else myData->dio.dout.outFlag[I_OUT_RUN_LED] = P0;
	
	if(myPs->signal[M_SIG_FAN_RELAY] == P1)
		myData->dio.dout.outFlag[I_OUT_FAN_RELAY] = P1;
	else myData->dio.dout.outFlag[I_OUT_FAN_RELAY] = P0;
	
	if(myPs->signal[M_SIG_POWER_OFF] == P1)
		myData->dio.dout.outFlag[I_OUT_POWER_OFF] = P1;
	else myData->dio.dout.outFlag[I_OUT_POWER_OFF] = P0;

	if(myPs->signal[M_SIG_CALI_RELAY] == P1)
		myData->dio.dout.outFlag[I_OUT_CALI_CHARGE_RELAY] = P1;
	else myData->dio.dout.outFlag[I_OUT_CALI_CHARGE_RELAY] = P0;

	//DYSON Aa¨¬n ¥ì¥ì¨úi Open ¨öA MC OFF ¡¾a¢¥E
	//220214_hun
#ifdef _JIG_TYPE_0
	if(myPs->signal[M_SIG_MAIN_MC_OFF] == P1)
		myData->dio.dout.outFlag[I_OUT_MAIN_MC_OFF] = P1;
	else myData->dio.dout.outFlag[I_OUT_MAIN_MC_OFF] = P0;
#endif
}

void DOut_Flag_4(void)
{ //kji 080821
	if(myPs->signal[M_SIG_RUN_LED] == P1)
		myData->dio.dout.outFlag[I_OUT_RUN_LED] = P1;
	else myData->dio.dout.outFlag[I_OUT_RUN_LED] = P0;
	
	if(myPs->signal[M_SIG_FAN_RELAY] == P1)
		myData->dio.dout.outFlag[I_OUT_FAN_RELAY] = P1;
	else myData->dio.dout.outFlag[I_OUT_FAN_RELAY] = P0;
	
	if(myPs->signal[M_SIG_POWER_OFF] == P1)
		myData->dio.dout.outFlag[I_OUT_POWER_OFF] = P1;
	else myData->dio.dout.outFlag[I_OUT_POWER_OFF] = P0;
}

void DOut_Flag_5(void)
{
	int i = 0, addr1;

	if(myPs->signal[M_SIG_RUN_LED] == P1)
		myData->dio.dout.outFlag[I_OUT_RUN_LED] = P1;
	else myData->dio.dout.outFlag[I_OUT_RUN_LED] = P0;

	addr1 = myPs->addr.interface[IO_EXPEND][9];
	if(myPs->signal[M_SIG_FAN_RELAY] == P1) {
		//25ms
		switch(myPs->signal[M_SIG_FAN_DELAY]) {
			case P0:
				outb(0x01, addr1);
				myPs->signal[M_SIG_FAN_DELAY]++;
				break;
			case P2:
				outb(0x03, addr1);
				myPs->signal[M_SIG_FAN_DELAY]++;
				break;
			case P4:
				outb(0x07, addr1);
				myPs->signal[M_SIG_FAN_DELAY]++;
				break;
			case P6:
				outb(0x0F, addr1);
				myPs->signal[M_SIG_FAN_DELAY] = P50;
				break;
			case P50:
				break;
			default:
				myPs->signal[M_SIG_FAN_DELAY]++;
				break;
		}
	} else {
		outb(0x00, addr1);
		myPs->signal[M_SIG_FAN_DELAY] = P0;
	}
	
	if(myPs->signal[M_SIG_POWER_OFF] == P1)
		myData->dio.dout.outFlag[I_OUT_POWER_OFF] = P1;
	else myData->dio.dout.outFlag[I_OUT_POWER_OFF] = P0;

	i = 2;
	if(i == 1){
		if(myPs->signal[M_SIG_CALI_RELAY] == P1)
			myData->dio.dout.outFlag[I_OUT_CALI_CHARGE_RELAY] = P1;
		else myData->dio.dout.outFlag[I_OUT_CALI_CHARGE_RELAY] = P0;
	}else if(i == 2){
		if(myPs->signal[M_SIG_CALI_CHARGE_RELAY] == P1)
			myData->dio.dout.outFlag[I_OUT_CALI_CHARGE_RELAY] = P1;
		else myData->dio.dout.outFlag[I_OUT_CALI_CHARGE_RELAY] = P0;

		if(myPs->signal[M_SIG_CALI_DISCHARGE_RELAY] == P1)
			myData->dio.dout.outFlag[I_OUT_CALI_DISCHARGE_RELAY] = P1;
		else myData->dio.dout.outFlag[I_OUT_CALI_DISCHARGE_RELAY] = P0;
	}
}

void DOut_Flag_6(void)
{
	int i = 0;
	//101104 kji w
#ifdef _JIG_TYPE_0

	Dout_TowerLamp_Flag();
	 
#ifdef _CHAMBERTYPE_4
	if(myPs->code == M_FAIL_CHAMBER_FIRE_ERROR ) { 
		myData->dio.dout.outFlag[I_OUT_STATE_PAUSE] = P1;
		myData->dio.dout.outFlag[I_OUT_STATE_BUZZER] = P1;
	}
#endif

	if(myPs->config.hwSpec == L_20V_6A_R3){
		if(myPs->signal[M_SIG_REMOTE_SMPS1] == P1
			&& myData->dio.delayTimer >= 300){
			myData->dio.dout.outFlag[I_OUT_REMOTE_SMPS1] = P1;
			myData->dio.dout.outFlag[I_OUT_REMOTE_SMPS1 + 1] = P1;
		}else{
			myData->dio.dout.outFlag[I_OUT_REMOTE_SMPS1] = P0;
			myData->dio.dout.outFlag[I_OUT_REMOTE_SMPS1 + 1] = P0;
		}
	}
	
	if(myPs->signal[M_SIG_END_BUZZER] == P1) {
		if(myPs->misc.endBuzzerCount == 0) //step_end
		{
			myPs->misc.endBuzzerCount = 2000; // 2sec
			myData->dio.dout.outFlag[I_OUT_END_BUZZER] = P1; //buzzer of
		} else if(myPs->misc.endBuzzerCount <= 
					myPs->misc.dio_scan_time) { //buzzer off
			myPs->signal[M_SIG_END_BUZZER] = P0;
			myPs->misc.endBuzzerCount = 0;
		} else { //buzzer on
			myPs->misc.endBuzzerCount -= myPs->misc.dio_scan_time;
		}
	}
	else myData->dio.dout.outFlag[I_OUT_END_BUZZER] = P0;
	
#endif	

	if(myPs->signal[M_SIG_RUN_LED] == P1)
		myData->dio.dout.outFlag[I_OUT_RUN_LED] = P1;
	else myData->dio.dout.outFlag[I_OUT_RUN_LED] = P0;

	if(myPs->signal[M_SIG_FAN_RELAY] == P1)
		myData->dio.dout.outFlag[I_OUT_FAN_RELAY] = P1;
	else myData->dio.dout.outFlag[I_OUT_FAN_RELAY] = P0;
	
	if(myPs->signal[M_SIG_POWER_OFF] == P1)
		myData->dio.dout.outFlag[I_OUT_POWER_OFF] = P1;
	else myData->dio.dout.outFlag[I_OUT_POWER_OFF] = P0;

	i = 1;
	if(i == 1){
		if(myPs->signal[M_SIG_CALI_RELAY] == P1)
			myData->dio.dout.outFlag[I_OUT_CALI_CHARGE_RELAY] = P1;
		else myData->dio.dout.outFlag[I_OUT_CALI_CHARGE_RELAY] = P0;
	}else if(i == 2){
		if(myPs->signal[M_SIG_CALI_CHARGE_RELAY] == P1)
			myData->dio.dout.outFlag[I_OUT_CALI_CHARGE_RELAY] = P1;
		else myData->dio.dout.outFlag[I_OUT_CALI_CHARGE_RELAY] = P0;

		if(myPs->signal[M_SIG_CALI_DISCHARGE_RELAY] == P1)
			myData->dio.dout.outFlag[I_OUT_CALI_DISCHARGE_RELAY] = P1;
		else myData->dio.dout.outFlag[I_OUT_CALI_DISCHARGE_RELAY] = P0;
	}
}

void DOut_Flag_7(void)
{
	int i = 0;
	//101104 kji w
#ifdef _JIG_TYPE_1
	if(myPs->signal[M_SIG_JIG_PASS_LAMP] == P1)
		myData->dio.dout.outFlag[I_OUT_JIG_PASS_LAMP] = P1;
	else myData->dio.dout.outFlag[I_OUT_JIG_PASS_LAMP] = P0;

	if(myPs->signal[M_SIG_JIG_FAIL_LAMP] == P1)
		myData->dio.dout.outFlag[I_OUT_JIG_FAIL_LAMP] = P1;
	else myData->dio.dout.outFlag[I_OUT_JIG_FAIL_LAMP] = P0;
	
	if(myPs->signal[M_SIG_LAMP_RUN] == P1)
		myData->dio.dout.outFlag[I_OUT_JIG_START_LAMP] = P1;
	else myData->dio.dout.outFlag[I_OUT_JIG_START_LAMP] = P0;

	if(myPs->signal[M_SIG_LAMP_STOP] == P1)
		myData->dio.dout.outFlag[I_OUT_JIG_STOP_LAMP] = P1;
	else myData->dio.dout.outFlag[I_OUT_JIG_STOP_LAMP] = P0;
	/*
	if(myPs->signal[M_SIG_JIG_SOL] == P1) {
		myData->dio.dout.outFlag[I_OUT_JIG_START_LAMP] = P1;
		myData->dio.dout.outFlag[I_OUT_JIG_STOP_LAMP] = P0;
	} else {
		myData->dio.dout.outFlag[I_OUT_JIG_START_LAMP] = P0;
		myData->dio.dout.outFlag[I_OUT_JIG_STOP_LAMP] = P1;
	}
	*/
	if(myPs->signal[M_SIG_JIG_BUZZER] == P1) {
		if(myPs->misc.endBuzzerCount == 0) //step_end
		{
			myPs->misc.endBuzzerCount = 1500; // 2sec
			myData->dio.dout.outFlag[I_OUT_JIG_BUZZER] = P1; //buzzer of
		} else if(myPs->misc.endBuzzerCount <= 
					myPs->misc.dio_scan_time) { //buzzer off
			myPs->signal[M_SIG_JIG_BUZZER] = P0;
			myPs->misc.endBuzzerCount = 0;
		} else { //buzzer on
			myPs->misc.endBuzzerCount -= myPs->misc.dio_scan_time;
		}
	} else myData->dio.dout.outFlag[I_OUT_JIG_BUZZER] = P0;

	if(myPs->signal[M_SIG_JIG_SOL] == P1)
		myData->dio.dout.outFlag[I_OUT_JIG_SOL] = P1;
	else myData->dio.dout.outFlag[I_OUT_JIG_SOL] = P0;
#endif	

	if(myPs->signal[M_SIG_RUN_LED] == P1)
		myData->dio.dout.outFlag[I_OUT_RUN_LED] = P1;
	else myData->dio.dout.outFlag[I_OUT_RUN_LED] = P0;

	if(myPs->signal[M_SIG_FAN_RELAY] == P1)
		myData->dio.dout.outFlag[I_OUT_FAN_RELAY] = P1;
	else myData->dio.dout.outFlag[I_OUT_FAN_RELAY] = P0;
	
	if(myPs->signal[M_SIG_POWER_OFF] == P1)
		myData->dio.dout.outFlag[I_OUT_POWER_OFF] = P1;
	else myData->dio.dout.outFlag[I_OUT_POWER_OFF] = P0;

	i = 1;
	if(i == 1){
		if(myPs->signal[M_SIG_CALI_RELAY] == P1)
			myData->dio.dout.outFlag[I_OUT_CALI_CHARGE_RELAY] = P1;
		else myData->dio.dout.outFlag[I_OUT_CALI_CHARGE_RELAY] = P0;
	}else if(i == 2){
		if(myPs->signal[M_SIG_CALI_CHARGE_RELAY] == P1)
			myData->dio.dout.outFlag[I_OUT_CALI_CHARGE_RELAY] = P1;
		else myData->dio.dout.outFlag[I_OUT_CALI_CHARGE_RELAY] = P0;

		if(myPs->signal[M_SIG_CALI_DISCHARGE_RELAY] == P1)
			myData->dio.dout.outFlag[I_OUT_CALI_DISCHARGE_RELAY] = P1;
		else myData->dio.dout.outFlag[I_OUT_CALI_DISCHARGE_RELAY] = P0;
	}
}

void DOut_Flag_8(void) //20181127 Add
{	

	int i = 0;

    Dout_TowerLamp_Flag_MultiBd();
	
	if(myPs->signal[M_SIG_RUN_LED] == P1)
		myData->dio.dout.outFlag[I_OUT_RUN_LED] = P1;
	else myData->dio.dout.outFlag[I_OUT_RUN_LED] = P0;

	if(myPs->signal[M_SIG_FAN_RELAY] == P1)
		myData->dio.dout.outFlag[I_OUT_FAN_RELAY] = P1;
	else myData->dio.dout.outFlag[I_OUT_FAN_RELAY] = P0;
	
	if(myPs->signal[M_SIG_POWER_OFF] == P1)
		myData->dio.dout.outFlag[I_OUT_POWER_OFF] = P1;
	else myData->dio.dout.outFlag[I_OUT_POWER_OFF] = P0;

	i = 1;
	if(i == 1){
		if(myPs->signal[M_SIG_CALI_RELAY] == P1)
			myData->dio.dout.outFlag[I_OUT_CALI_CHARGE_RELAY] = P1;
		else myData->dio.dout.outFlag[I_OUT_CALI_CHARGE_RELAY] = P0;
	}else if(i == 2){
		if(myPs->signal[M_SIG_CALI_CHARGE_RELAY] == P1)
			myData->dio.dout.outFlag[I_OUT_CALI_CHARGE_RELAY] = P1;
		else myData->dio.dout.outFlag[I_OUT_CALI_CHARGE_RELAY] = P0;

		if(myPs->signal[M_SIG_CALI_DISCHARGE_RELAY] == P1)
			myData->dio.dout.outFlag[I_OUT_CALI_DISCHARGE_RELAY] = P1;
		else myData->dio.dout.outFlag[I_OUT_CALI_DISCHARGE_RELAY] = P0;
	}
}


void DOut_Flag_Default(void)
{	

	int i = 0;

    Dout_TowerLamp_Flag();
	
	if(myPs->signal[M_SIG_RUN_LED] == P1)
		myData->dio.dout.outFlag[I_OUT_RUN_LED] = P1;
	else myData->dio.dout.outFlag[I_OUT_RUN_LED] = P0;

	if(myPs->signal[M_SIG_FAN_RELAY] == P1)
		myData->dio.dout.outFlag[I_OUT_FAN_RELAY] = P1;
	else myData->dio.dout.outFlag[I_OUT_FAN_RELAY] = P0;
	
	if(myPs->signal[M_SIG_POWER_OFF] == P1)
		myData->dio.dout.outFlag[I_OUT_POWER_OFF] = P1;
	else myData->dio.dout.outFlag[I_OUT_POWER_OFF] = P0;

	i = 1;
	if(i == 1){
		if(myPs->signal[M_SIG_CALI_RELAY] == P1)
			myData->dio.dout.outFlag[I_OUT_CALI_CHARGE_RELAY] = P1;
		else myData->dio.dout.outFlag[I_OUT_CALI_CHARGE_RELAY] = P0;
	}else if(i == 2){
		if(myPs->signal[M_SIG_CALI_CHARGE_RELAY] == P1)
			myData->dio.dout.outFlag[I_OUT_CALI_CHARGE_RELAY] = P1;
		else myData->dio.dout.outFlag[I_OUT_CALI_CHARGE_RELAY] = P0;

		if(myPs->signal[M_SIG_CALI_DISCHARGE_RELAY] == P1)
			myData->dio.dout.outFlag[I_OUT_CALI_DISCHARGE_RELAY] = P1;
		else myData->dio.dout.outFlag[I_OUT_CALI_DISCHARGE_RELAY] = P0;
	}
}

void DOut_Flag_9(void)
{	//180611 add for digital
#if CYCLER_TYPE == DIGITAL_CYC
	int i = 0;
	if(myPs->signal[M_SIG_RUN_LED] == P1)
		myData->dio.dout.outFlag[I_OUT_RUN_LED] = P1;
	else myData->dio.dout.outFlag[I_OUT_RUN_LED] = P0;
	
	if(myPs->signal[M_SIG_FAN_RELAY] == P1){
		myData->dio.dout.outFlag[I_OUT_FAN_RELAY] = P1;
		myData->dio.dout.outFlag[I_OUT_FAN1] = P1;
		myData->dio.dout.outFlag[I_OUT_FAN2] = P1;
		myData->dio.dout.outFlag[I_OUT_FAN3] = P1;
		myData->dio.dout.outFlag[I_OUT_FAN4] = P1;
	//	myData->dio.dout.outFlag[I_OUT_FAN5] = P1;
	//	myData->dio.dout.outFlag[I_OUT_FAN6] = P1;
	//	myData->dio.dout.outFlag[I_OUT_FAN7] = P1;
	//	myData->dio.dout.outFlag[I_OUT_FAN8] = P1;
	}else{ 
		myData->dio.dout.outFlag[I_OUT_FAN_RELAY] = P0;
		myData->dio.dout.outFlag[I_OUT_FAN1] = P0;
		myData->dio.dout.outFlag[I_OUT_FAN2] = P0;
		myData->dio.dout.outFlag[I_OUT_FAN3] = P0;
		myData->dio.dout.outFlag[I_OUT_FAN4] = P0;
	//	myData->dio.dout.outFlag[I_OUT_FAN5] = P0;
	//	myData->dio.dout.outFlag[I_OUT_FAN6] = P0;
	//	myData->dio.dout.outFlag[I_OUT_FAN7] = P0;
	//	myData->dio.dout.outFlag[I_OUT_FAN8] = P0;
	}
	
	if(myPs->signal[M_SIG_POWER_OFF] == P1)
		myData->dio.dout.outFlag[I_OUT_POWER_OFF] = P1;
	else myData->dio.dout.outFlag[I_OUT_POWER_OFF] = P0;


	if(myPs->signal[M_SIG_LAMP_RUN] == P1){
		myData->dio.dout.outFlag[I_OUT_STATE1_RUN] = P1;
	//	myData->dio.dout.outFlag[I_OUT_STATE2_RUN] = P0;
	}else if(myPs->signal[M_SIG_LAMP_RUN] == P2){
	//	myData->dio.dout.outFlag[I_OUT_STATE2_RUN] = P1;
		myData->dio.dout.outFlag[I_OUT_STATE1_RUN] = P0;
	}else if(myPs->signal[M_SIG_LAMP_RUN] == P3){
		myData->dio.dout.outFlag[I_OUT_STATE1_RUN] = P1;
	//	myData->dio.dout.outFlag[I_OUT_STATE2_RUN] = P1;
	}else{
		myData->dio.dout.outFlag[I_OUT_STATE1_RUN] = P0;
	//	myData->dio.dout.outFlag[I_OUT_STATE2_RUN] = P0;
	}
	
	if(myPs->signal[M_SIG_LAMP_STOP] == P1){
		myData->dio.dout.outFlag[I_OUT_STATE1_STOP] = P1;
	//	myData->dio.dout.outFlag[I_OUT_STATE2_STOP] = P0;
	}else if(myPs->signal[M_SIG_LAMP_STOP] == P2){
	//	myData->dio.dout.outFlag[I_OUT_STATE2_STOP] = P1;
		myData->dio.dout.outFlag[I_OUT_STATE1_STOP] = P0;
	}else if(myPs->signal[M_SIG_LAMP_STOP] == P3){
		myData->dio.dout.outFlag[I_OUT_STATE1_STOP] = P1;
	//	myData->dio.dout.outFlag[I_OUT_STATE2_STOP] = P1;
	}else{
		myData->dio.dout.outFlag[I_OUT_STATE1_STOP] = P0;
	//	myData->dio.dout.outFlag[I_OUT_STATE2_STOP] = P0;
	}
	
	if(myPs->signal[M_SIG_LAMP_PAUSE] == P1){
		myData->dio.dout.outFlag[I_OUT_STATE1_PAUSE] = P1;
	//	myData->dio.dout.outFlag[I_OUT_STATE2_PAUSE] = P0;
	}else if(myPs->signal[M_SIG_LAMP_PAUSE] == P2){
		myData->dio.dout.outFlag[I_OUT_STATE1_PAUSE] = P0;
	//	myData->dio.dout.outFlag[I_OUT_STATE2_PAUSE] = P1;
	}else if(myPs->signal[M_SIG_LAMP_PAUSE] == P3){
		myData->dio.dout.outFlag[I_OUT_STATE1_PAUSE] = P1;
	//	myData->dio.dout.outFlag[I_OUT_STATE2_PAUSE] = P1;
	}else{
		myData->dio.dout.outFlag[I_OUT_STATE1_PAUSE] = P0;
	//	myData->dio.dout.outFlag[I_OUT_STATE2_PAUSE] = P0;
	}
	
	if(myPs->signal[M_SIG_LAMP_BUZZER] == P1){
		myData->dio.dout.outFlag[I_OUT_STATE1_BUZZER] = P1;
	//	myData->dio.dout.outFlag[I_OUT_STATE2_BUZZER] = P0;
	}else if(myPs->signal[M_SIG_LAMP_BUZZER] == P2){
		myData->dio.dout.outFlag[I_OUT_STATE1_BUZZER] = P0;
	//	myData->dio.dout.outFlag[I_OUT_STATE2_BUZZER] = P1;
	}else if(myPs->signal[M_SIG_LAMP_BUZZER] == P3){
		myData->dio.dout.outFlag[I_OUT_STATE1_BUZZER] = P1;
	//	myData->dio.dout.outFlag[I_OUT_STATE2_BUZZER] = P1;
	}else{
		myData->dio.dout.outFlag[I_OUT_STATE1_BUZZER] = P0;
	//	myData->dio.dout.outFlag[I_OUT_STATE2_BUZZER] = P0;
	}
	
	i = 3;
	if(i == 1){
		if(myPs->signal[M_SIG_CALI_RELAY] == P1)
			myData->dio.dout.outFlag[I_OUT_CALI_CHARGE_RELAY] = P1;
		else myData->dio.dout.outFlag[I_OUT_CALI_CHARGE_RELAY] = P0;
	}else if(i == 2){
		if(myPs->signal[M_SIG_CALI_CHARGE_RELAY] == P1)
			myData->dio.dout.outFlag[I_OUT_CALI_CHARGE_RELAY] = P1;
		else myData->dio.dout.outFlag[I_OUT_CALI_CHARGE_RELAY] = P0;

		if(myPs->signal[M_SIG_CALI_DISCHARGE_RELAY] == P1)
			myData->dio.dout.outFlag[I_OUT_CALI_DISCHARGE_RELAY] = P1;
		else myData->dio.dout.outFlag[I_OUT_CALI_DISCHARGE_RELAY] = P0;
	}
#endif
}

void DOut_Flag_10(void)
{	//180611 add for digital
#if CYCLER_TYPE == DIGITAL_CYC
	int i = 0;
	if(myPs->signal[M_SIG_RUN_LED] == P1)
		myData->dio.dout.outFlag[I_OUT_RUN_LED] = P1;
	else myData->dio.dout.outFlag[I_OUT_RUN_LED] = P0;
	
	if(myData->mData.config.TempFaultDioUse == 0){
		if(myPs->signal[M_SIG_FAN_RELAY] == P1){
			myData->dio.dout.outFlag[I_OUT_FAN_RELAY] = P1;
			myData->dio.dout.outFlag[I_OUT_FAN1] = P1;
			myData->dio.dout.outFlag[I_OUT_FAN2] = P1;
			myData->dio.dout.outFlag[I_OUT_FAN3] = P1;
			myData->dio.dout.outFlag[I_OUT_FAN4] = P1;
			myData->dio.dout.outFlag[I_OUT_FAN5] = P1;
			myData->dio.dout.outFlag[I_OUT_FAN6] = P1;
			myData->dio.dout.outFlag[I_OUT_FAN7] = P1;
			myData->dio.dout.outFlag[I_OUT_FAN8] = P1;
		}else{ 
			myData->dio.dout.outFlag[I_OUT_FAN_RELAY] = P0;
			myData->dio.dout.outFlag[I_OUT_FAN1] = P0;
			myData->dio.dout.outFlag[I_OUT_FAN2] = P0;
			myData->dio.dout.outFlag[I_OUT_FAN3] = P0;
			myData->dio.dout.outFlag[I_OUT_FAN4] = P0;
			myData->dio.dout.outFlag[I_OUT_FAN5] = P0;
			myData->dio.dout.outFlag[I_OUT_FAN6] = P0;
			myData->dio.dout.outFlag[I_OUT_FAN7] = P0;
			myData->dio.dout.outFlag[I_OUT_FAN8] = P0;
		}
	}else{ 
		if(myPs->signal[M_SIG_FAN_RELAY] == P1){ //Fan Signal
			myData->dio.dout.outFlag[I_OUT_FAN_RELAY] = P1;
		//	myData->dio.dout.outFlag[I_OUT_FAN1] = P1;
		//	myData->dio.dout.outFlag[I_OUT_FAN2] = P1;
			myData->dio.dout.outFlag[I_OUT_FAN3] = P1;
			myData->dio.dout.outFlag[I_OUT_FAN4] = P1;
			myData->dio.dout.outFlag[I_OUT_FAN5] = P1;
			myData->dio.dout.outFlag[I_OUT_FAN6] = P1;
			myData->dio.dout.outFlag[I_OUT_FAN7] = P1;
			myData->dio.dout.outFlag[I_OUT_FAN8] = P1;
		}else{ 
			myData->dio.dout.outFlag[I_OUT_FAN_RELAY] = P0; 
		//	myData->dio.dout.outFlag[I_OUT_FAN1] = P0;
		//	myData->dio.dout.outFlag[I_OUT_FAN2] = P0;
			myData->dio.dout.outFlag[I_OUT_FAN3] = P0;
			myData->dio.dout.outFlag[I_OUT_FAN4] = P0;
			myData->dio.dout.outFlag[I_OUT_FAN5] = P0;
			myData->dio.dout.outFlag[I_OUT_FAN6] = P0;
			myData->dio.dout.outFlag[I_OUT_FAN7] = P0;
			myData->dio.dout.outFlag[I_OUT_FAN8] = P0;
		}
		
		if(myPs->signal[M_SIG_TEMP_FAULT1] == P1) //Fire extinguisher Signal
			myData->dio.dout.outFlag[I_OUT_FAN1] = P1;
		else
			myData->dio.dout.outFlag[I_OUT_FAN1] = P0;
		if(myPs->signal[M_SIG_TEMP_FAULT2] == P1)
			myData->dio.dout.outFlag[I_OUT_FAN2] = P1;
		else
			myData->dio.dout.outFlag[I_OUT_FAN2] = P0;
	}
	
	if(myPs->signal[M_SIG_POWER_OFF] == P1)
		myData->dio.dout.outFlag[I_OUT_POWER_OFF] = P1;
	else myData->dio.dout.outFlag[I_OUT_POWER_OFF] = P0;

	if(myPs->signal[M_SIG_LAMP_STOP] == P1)
		myData->dio.dout.outFlag[I_OUT_STATE1_STOP] = P1;
	else myData->dio.dout.outFlag[I_OUT_STATE1_STOP] = P0;
	
	if(myPs->signal[M_SIG_LAMP_PAUSE] == P1)
		myData->dio.dout.outFlag[I_OUT_STATE1_PAUSE] = P1;
	else myData->dio.dout.outFlag[I_OUT_STATE1_PAUSE] = P0;
	
	if(myPs->signal[M_SIG_LAMP_RUN] == P1)
		myData->dio.dout.outFlag[I_OUT_STATE1_RUN] = P1;
	else myData->dio.dout.outFlag[I_OUT_STATE1_RUN] = P0;
	
	if(myPs->signal[M_SIG_LAMP_BUZZER] == P1)
		myData->dio.dout.outFlag[I_OUT_STATE1_BUZZER] = P1;
	else myData->dio.dout.outFlag[I_OUT_STATE1_BUZZER] = P0;

	#ifdef _JIG_TYPE_3
	//180927 add for Smps Cali
	if(myPs->signal[M_SIG_CALI_VOLTAGE2_RELAY] == P1){
		myData->dio.dout.outFlag[I_OUT_CALI_VOLTAGE1_RELAY] = P1;
		myData->dio.dout.outFlag[I_OUT_CALI_VOLTAGE2_RELAY] = P1;
	}else{
		myData->dio.dout.outFlag[I_OUT_CALI_VOLTAGE1_RELAY] = P0;
		myData->dio.dout.outFlag[I_OUT_CALI_VOLTAGE2_RELAY] = P0;
	}
	
	if(myPs->signal[M_SIG_CALI_CHARGE2_RELAY] == P1)
		myData->dio.dout.outFlag[I_OUT_CALI_CHARGE_RELAY] = P1;
	else myData->dio.dout.outFlag[I_OUT_CALI_CHARGE_RELAY] = P0;
	

	if(myPs->signal[M_SIG_CALI_DISCHARGE2_RELAY] == P1)
		myData->dio.dout.outFlag[I_OUT_CALI_DISCHARGE_RELAY] = P1;
	else myData->dio.dout.outFlag[I_OUT_CALI_DISCHARGE_RELAY] = P0;
	#endif
	i = 3;
#endif
}

void DOut_Flag_11(void)
{	//210308 lyhw for can	
#if CYCLER_TYPE == CAN_CYC
	int i = 0, index;

    Dout_TowerLamp_Flag();
	
	if(myPs->signal[M_SIG_RUN_LED] == P1)
		myData->dio.dout.outFlag[I_OUT_RUN_LED] = P1;
	else myData->dio.dout.outFlag[I_OUT_RUN_LED] = P0;

	if(myPs->signal[M_SIG_FAN_RELAY] == P1)
		myData->dio.dout.outFlag[I_OUT_FAN_RELAY] = P1;
	else myData->dio.dout.outFlag[I_OUT_FAN_RELAY] = P0;
	
	if(myPs->signal[M_SIG_POWER_OFF] == P1)
		myData->dio.dout.outFlag[I_OUT_POWER_OFF] = P1;
	else myData->dio.dout.outFlag[I_OUT_POWER_OFF] = P0;

	/* 210308 lyhw for CAN JIG IO CONTROL
	 * I_OUT_CALI_CHARGE_REALY == I_OUT_CAN2_0
	 * I_OUT_CALI_DISCHARGE_REALY == I_OUT_CAN2_1
	 */
	i = 1;
	if(i == 1){
		if(myPs->signal[M_SIG_CALI_RELAY] == P1){
			myPs->signal[M_SIG_CAN2_0] = P1;
		}else{ 
			myPs->signal[M_SIG_CAN2_0] = P0;
		}
	}else if(i == 2){
		if(myPs->signal[M_SIG_CALI_CHARGE_RELAY] == P1){
			myPs->signal[M_SIG_CAN2_0] = P1;
		}else{ 
			myPs->signal[M_SIG_CAN2_0] = P0;
		}

		if(myPs->signal[M_SIG_CALI_DISCHARGE_RELAY] == P1){
			myPs->signal[M_SIG_CAN2_1] = P1;
		}else{ 
			myPs->signal[M_SIG_CAN2_1] = P0;
		}
	}

	for(index = 0; index < 16; index++){
		if(myPs->signal[M_SIG_CAN1_0 + index] == P1)
			myData->dio.dout.outFlag[I_OUT_CAN1_0 + index] = P1;
		else myData->dio.dout.outFlag[I_OUT_CAN1_0 + index] = P0;
	}

#endif
}

void DOutScan(void)
{
	unsigned char flag, useFlag, tmpOutputNum;
	int outByte, bit, signalNo, addr = 0, idx = 0;
	
	if(myData->dio.config.dio_Control_Flag == P0) return;
	
	if(myData->AppControl.config.systemType == CYCLER_CAN) {
		tmpOutputNum = myPs->addr.outputAddrNo + CAN_IO_OUTPUT_CNT;
	} else {
		tmpOutputNum = myPs->addr.outputAddrNo;
	}
	
	for(outByte = 0; outByte < tmpOutputNum; outByte++) {
		if(outByte >= MAX_DIGITAL_OUTPUT) break;
		if(outByte < myPs->addr.outputAddrNo) {
			addr = myPs->addr.interface[IO_OUTPUT][outByte];
			if(addr == 0) continue;
		}	
		
		flag = 0x01;
		
		for(bit=0; bit < 8; bit++) {
			signalNo = outByte * 8 + bit;
			if(myData->AppControl.config.versionNo >= 20090602) {
				useFlag = myData->dio.dout.outUseFlag[signalNo];
				if(useFlag == P1) {
					if(myData->dio.dout.outFlag[signalNo] == P1) {
						myData->dio.dout.outBit[outByte] |= flag;
					} else {
						myData->dio.dout.outBit[outByte] &= ~flag;
					}
				}
			} else {
				if(myData->dio.dout.outFlag[signalNo] == P1) {
					myData->dio.dout.outBit[outByte] |= flag;
				} else {
					myData->dio.dout.outBit[outByte] &= ~flag;
				}
			}
			flag = flag << 1;
		}
		
		//210308 Check CRC Model JIG IO Output  
		if(outByte >= myPs->addr.outputAddrNo &&
			myData->AppControl.config.systemType == CYCLER_CAN) { //CAN IO Send 
			idx = outByte - myPs->addr.outputAddrNo;
			myData->CAN.io.outputValue[idx] =
				myData->dio.dout.outBit[outByte];
		} else {
			outb(myData->dio.dout.outBit[outByte], addr);
		}
	}
}

unsigned char Read_InPoint(int signal)
{
	if(myData->dio.din.inFlag[signal] == P1) return P1;
	else if(myData->dio.din.inFlag[signal] == P2) return P2;
	else return P0;
}
		
void Select_OutPoint(int signal, int value)
{
	myData->dio.dout.outFlag[signal] = (unsigned char)value;
}

void Clear_OutPort(void)
{
}

void BuzzerSet(int count, int OnTimer, int OffTimer)
{
	myData->dio.buzzerCount = (unsigned char)count;
	myData->dio.buzzerOnTimer = (unsigned long)OnTimer;
	myData->dio.buzzerOffTimer = (unsigned long)OffTimer;

	myData->dio.tmpBuzzerCount = 0;
	myData->dio.tmpBuzzerOnTimer = 0;
	myData->dio.tmpBuzzerOffTimer = 0;
	myData->dio.signal[DIO_SIG_BUZZER_END] = P0;
}

void BuzzerControl(void)
{
	if(myData->dio.buzzerOnTimer == 0 && myData->dio.buzzerOffTimer == 0) {
		myData->dio.dout.outFlag[E_OUT_BUZZER] = P0;
		return;
	}
	
	if(myData->dio.buzzerCount == 0) {
		if(myData->dio.tmpBuzzerCount == 0) {
			if(myData->dio.tmpBuzzerOnTimer >= myData->dio.buzzerOnTimer) {
				myData->dio.tmpBuzzerCount++;
				myData->dio.tmpBuzzerOnTimer = 0;
			} else {
				myData->dio.tmpBuzzerOnTimer
					+= myPs->misc.dio_scan_time;
				myData->dio.dout.outFlag[E_OUT_BUZZER] = P1;
			}
		} else {
			if(myData->dio.tmpBuzzerOffTimer >= myData->dio.buzzerOffTimer) {
				myData->dio.tmpBuzzerCount = 0;
				myData->dio.tmpBuzzerOffTimer = 0;
			} else {
				myData->dio.tmpBuzzerOffTimer
					+= myPs->misc.dio_scan_time;
				myData->dio.dout.outFlag[E_OUT_BUZZER] = P0;
			}
		}
	} else {
		if(myData->dio.tmpBuzzerCount >= myData->dio.buzzerCount*2) {
			myData->dio.buzzerCount = 0;
			myData->dio.buzzerOnTimer = 0;
			myData->dio.buzzerOffTimer = 0;
			myData->dio.signal[DIO_SIG_BUZZER_END] = P1;
		} else {
			if((myData->dio.tmpBuzzerCount % 2) == 0) {
				myData->dio.tmpBuzzerOnTimer
					+= myPs->misc.dio_scan_time;
				if(myData->dio.tmpBuzzerOnTimer >= myData->dio.buzzerOnTimer) {
					myData->dio.tmpBuzzerOnTimer = 0;
					myData->dio.tmpBuzzerCount++;
				}
				myData->dio.dout.outFlag[E_OUT_BUZZER] = P1;
			} else {
				myData->dio.tmpBuzzerOffTimer
					+= myPs->misc.dio_scan_time;
				if(myData->dio.tmpBuzzerOffTimer
					>= myData->dio.buzzerOffTimer) {
					myData->dio.tmpBuzzerOffTimer = 0;
					myData->dio.tmpBuzzerCount++;
				}
				myData->dio.dout.outFlag[E_OUT_BUZZER] = P0;
			}
		}
	}
}

void ThermalSensor(void)
{/*
	int i, j, tSlot;
	long temp, maxTemp, minTemp;
	U_ADDA	ADValue;

	temp = 0;
	tSlot = myData->tempData.tSlot;
	if(tSlot < MAX_AD_INPUT) {
		for(i=0; i < 5; i++) {
			outb(0x00, TEMP_AD_ADDR+1);
			for(j=0; j < 2100; j++) {}
			ADValue.byte[1] = (unsigned char)inb(TEMP_AD_ADDR+1);
			ADValue.byte[0] = (unsigned char)inb(TEMP_AD_ADDR);
			temp += ADValue.val;
		}
		myData->tempData.value[tSlot] = temp/5;
		outb((unsigned char)(tSlot+1), TEMP_MUX_ADDR);
	} else if(tSlot == MAX_AD_INPUT) {
		for(i=0; i < MAX_AD_INPUT; i++) {
			temp = myData->tempData.value[i];
			if(i < 15) { // temp sensor
				for(j=0; j < MAX_TH_STEP; j++) {
					if(myData->thData.VTH[j] < temp) {
						if(j == (MAX_TH_STEP-1)) {
							myData->tempData.Th[i] = (myData->thData.TH_A[j] *
								temp) / 10000L + myData->thData.TH_B[j];
							myData->tempData.Th[i] *= 10;
						} else continue;
					} else {
						myData->tempData.Th[i] = (myData->thData.TH_A[j] * temp)
							/ 10000L + myData->thData.TH_B[j];
						myData->tempData.Th[i] *= 10;
						break;
					}
				}
			} else { //smoke sensor
			}
		}
	} else {
		//temperature
		maxTemp = myData->tempData.Th[0];
		minTemp = maxTemp;
		for(i=0; i < 5; i++) {
			if(maxTemp < myData->tempData.Th[i])
				maxTemp = myData->tempData.Th[i];
			if(minTemp > myData->tempData.Th[i])
				minTemp = myData->tempData.Th[i];
		}
		//myData->gData[0].maxJigTemp = (int)maxTemp;
		//myData->gData[0].deltaJigTemp = (int)(maxTemp - minTemp);
	}
	tSlot++;
	if(tSlot > (MAX_AD_INPUT+1)) tSlot = 0;
	myData->tempData.tSlot = tSlot; */
}

int FaultLamp_Control(void)
{
	unsigned char k;
	int i, j, count, maxFault, faultCount, bd, ch;

	maxFault = 1;
	if(myPs->config.hwSpec == L_5V_5A
		|| myPs->config.hwSpec == L_5V_5A_2) {
		count = 8;
	} else if(myPs->config.hwSpec == L_5V_30A) {
		count = 10;
	} else {
		count = 1;
	}

	for(i=0; i < myPs->config.installedCh; i++) {
		bd = myData->CellArray1[i].bd;
		ch = myData->CellArray1[i].ch;
		if(myData->bData[bd].cData[ch].op.state == C_CALI) return -1;
	}

	faultCount = 0;
	for(i=0; i < myPs->config.installedCh; i++) {
		bd = myData->CellArray1[i].bd;
		ch = myData->CellArray1[i].ch;
		if(myData->bData[bd].cData[ch].op.code >= C_FAULT_CHECK_CODE_START
			&& myData->bData[bd].cData[ch].op.code != C_FAULT_STOP_CMD
			&& myData->bData[bd].cData[ch].op.code != C_FAULT_PAUSE_CMD
			&& myData->bData[bd].cData[ch].op.code != C_FAULT_NEXTSTEP_CMD) {
			faultCount++;
		}
		if((i+1)%count == 0) {
			j = i / count;
			if(faultCount >= maxFault) {
				k = P1;
			} else {
				k = P0;
			}
			switch(j) {
				case 0:
					myData->dio.dout.outSignal[O_SIG_FAULT_LAMP1] = k;
					break;
				case 1:
					myData->dio.dout.outSignal[O_SIG_FAULT_LAMP1+2] = k;
					break;
				case 2:
					myData->dio.dout.outSignal[O_SIG_FAULT_LAMP1+1] = k;
					break;
				case 3:
					myData->dio.dout.outSignal[O_SIG_FAULT_LAMP1+3] = k;
					break;
			}
			faultCount = 0;
		}
	}
	return 0;
}

void Check_OVP(void)
{
	if(myData->mData.config.LGES_fault_config.ovp_pause_flag == 1){
		Check_OVP_1();
	}else{
		Check_OVP_Default();
	}
}

void Check_OVP_Default(void)
{
	unsigned short chNo = 0, bdNo = 0;
	int bd, ch, diff, count;
	long event_val = 0;

	if(myData->mData.config.function[F_OVP] == P0) return;
	if(myData->AppControl.config.debugType != P0) return;
			
	if(myData->dio.delayTimer < myData->dio.config.dioDelay) {
		for(bd=0; bd < myPs->config.installedBd; bd++) {
			for(ch=0; ch < myPs->config.chPerBd; ch++) {
				if((myPs->config.chPerBd * bd + ch)
					> (myPs->config.installedCh - 1)) continue;

				myData->bData[bd].cData[ch].misc.hw_fault_ovp
					= myData->mData.misc.timer_1sec;
			}
		}

		return;
	}
	
	count = 0;
	for(bd=0; bd < myPs->config.installedBd; bd++) {
		for(ch=0; ch < myPs->config.chPerBd; ch++) {
			if((myPs->config.chPerBd * bd + ch)
				> (myPs->config.installedCh - 1)) continue;

			if(myData->bData[bd].cData[ch].op.Vsens 
				>= myData->mData.config.hwFaultConfig[HW_FAULT_OVP]) {
				diff = myData->mData.misc.timer_1sec
					- myData->bData[bd].cData[ch].misc.hw_fault_ovp;
				if(diff >= 20) { //20Sec
					count++;
				}
				//190527
				event_val = (long)myData->bData[bd].cData[ch].op.Vsens;
				bdNo = bd;
				chNo = (myPs->config.chPerBd * bd) + (ch+1);
			} else {
				myData->bData[bd].cData[ch].misc.hw_fault_ovp
					= myData->mData.misc.timer_1sec;
			}
		}
	}

	if(count > 0) {
		if(myData->dio.signal[DIO_SIG_OVP] == P0) {
			myData->dio.signal[DIO_SIG_OVP] = P1;
			if((inb(0x602) & 0x10) == P0) { //190701
				
#if CYCLER_TYPE == DIGITAL_CYC
				//180701 add
				myPs->signal[M_SIG_INV_POWER] = P100;
				myPs->signal[M_SIG_INV_POWER1] = P100;
#endif
				//210303 Inverter off 
				if(myData->AppControl.config.systemType == CYCLER_CAN){ 
					myPs->signal[M_SIG_INV_POWER_CAN] = P100;
				}
				myPs->signal[M_SIG_RUN_LED] = P0;
				myPs->signal[M_SIG_REMOTE_SMPS1] = P0;
				myPs->signal[M_SIG_FAN_RELAY] = P0;
				myPs->signal[M_SIG_POWER_OFF] = P1;
				send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 20, chNo);//halt
			}
			send_msg_2(MODULE_TO_APP_VALUE, MSG_MODULE_APP_EMG,
				1, chNo, bdNo, event_val); //SBC Log
			myPs->code = M_FAIL_OVP;	
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
}

void Check_OVP_1(void)
{
	unsigned short chNo = 0;
	int bd = 0, ch = 0, bd1 = 0, ch1 = 0, index, i;
	long ovp_time;
	char fault_bd[MAX_BD_PER_MODULE];
	char fault_ch[MAX_CH_PER_BD];
	long event_val[MAX_BD_PER_MODULE][MAX_CH_PER_BD];


	if(myData->mData.config.function[F_OVP] == P0) return;
	if(myData->AppControl.config.debugType != P0) return;

	memset(fault_bd, 0, sizeof fault_bd);
	memset(fault_ch, 0, sizeof fault_ch);
	memset(event_val, 0, sizeof event_val);
	
	ovp_time = myData->mData.config.LGES_fault_config.ovp_check_time / 100;
			
	if(myData->dio.delayTimer < myData->dio.config.dioDelay){
		for(bd=0; bd < myPs->config.installedBd; bd++){
			for(ch=0; ch < myPs->config.chPerBd; ch++){
				if((myPs->config.chPerBd * bd + ch)
					> (myPs->config.installedCh - 1)) continue;
				myData->bData[bd].cData[ch].misc.hw_fault_ovp
					= myData->mData.misc.timer_1sec;
			}
		}
		return;
	}
	
	for(bd=0; bd < myPs->config.installedBd; bd++){
		for(ch=0; ch < myPs->config.installedCh; ch++){
			if((myPs->config.chPerBd * bd + ch)
				> (myPs->config.installedCh - 1)) continue;
			index = (myPs->config.chPerBd * bd) + ch;
			myData->bData[bd].cData[ch].misc.ovp_ChamberNo 
				= myData->ChamberChNo[index].number1;
			if(myData->bData[bd].cData[ch].op.Vsens 
				>= myData->mData.config.hwFaultConfig[HW_FAULT_OVP]){
				if(myData->bData[bd].cData[ch].misc.ovp_fault_signal == 0){
					if((myData->mData.misc.timer_1sec 
						- myData->bData[bd].cData[ch].misc.hw_fault_ovp) >= ovp_time){
						if(myData->bData[bd].cData[ch].op.state == C_STANDBY){
							//halt
							event_val[bd][ch] = (long)myData->bData[bd].cData[ch].op.Vsens;
							chNo = (myPs->config.chPerBd * bd) + (ch+1);
							fault_bd[bd] = 1;
							fault_ch[ch] = 1;
							if(myData->dio.signal[DIO_SIG_OVP] == P0){
								myData->dio.signal[DIO_SIG_OVP] = P1;
							}
						}else{
							//pause
							myData->bData[bd].cData[ch].misc.ovp_fault_signal = 1;
						}
					}
				}
			}else{
				myData->bData[bd].cData[ch].misc.hw_fault_ovp
					= myData->mData.misc.timer_1sec;
				myData->bData[bd].cData[ch].misc.ovp_fault_signal = 0;
			}
		}
	}
	if(myData->dio.signal[DIO_SIG_OVP] == P1) {
		myData->dio.signal[DIO_SIG_OVP] = P2;
		if((inb(0x602) & 0x10) == P0) { //190701
			
#if CYCLER_TYPE == DIGITAL_CYC
			//180701 add
			myPs->signal[M_SIG_INV_POWER] = P100;
			myPs->signal[M_SIG_INV_POWER1] = P100;
#endif
			//210303 Inverter off 
			if(myData->AppControl.config.systemType == CYCLER_CAN){ 
				myPs->signal[M_SIG_INV_POWER_CAN] = P100;
			}
			myPs->signal[M_SIG_RUN_LED] = P0;
			myPs->signal[M_SIG_REMOTE_SMPS1] = P0;
			myPs->signal[M_SIG_FAN_RELAY] = P0;
			myPs->signal[M_SIG_POWER_OFF] = P1;
			send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 20, chNo);//halt
		}

		myPs->code = M_FAIL_OVP;	
		send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
				(int)myPs->code, chNo);
		for(bd=0; bd < myPs->config.installedBd; bd++) {
			for(ch=0; ch < myPs->config.chPerBd; ch++) {
				if((myPs->config.chPerBd * bd + ch)
					> (myPs->config.installedCh - 1)) continue;
				if(fault_bd[bd] == 1 && fault_ch[ch] == 1){
					myData->bData[bd].cData[ch].signal[C_SIG_OVP_PAUSE] = P1;
					send_msg_2(MODULE_TO_APP_VALUE, MSG_MODULE_APP_EMG,
					1, ch, bd, event_val[bd][ch]); //SBC Log
				}else{
					myData->bData[bd].cData[ch].signal[C_SIG_OVP_GROUP_PAUSE] = P1;
				}
			}
		}
	}


	for(bd=0; bd < myPs->config.installedBd; bd++){
		for(ch=0; ch < myPs->config.installedCh; ch++){
			if(myData->bData[bd].cData[ch].misc.ovp_fault_signal == 1){
				myData->bData[bd].cData[ch].misc.ovp_fault_signal = 0;
				for(i = 0; i< myData->mData.config.installedCh; i++){
					if(myData->bData[bd].cData[ch].misc.ovp_ChamberNo 
							== myData->ChamberChNo[i].number1){
						bd1 = myData->ChamberChNo[i].bd;
						ch1 = myData->ChamberChNo[i].ch;
						if((myPs->config.chPerBd * bd + ch)
							> (myPs->config.installedCh-1)){
							continue;
						}
						if(bd == bd1 && ch == ch1){
							if(myData->bData[bd1].cData[ch1].op.state == C_RUN){
								myData->bData[bd1].cData[ch1].signal[C_SIG_OVP_PAUSE] = P1;
							}
						}else{
							if(myData->bData[bd1].cData[ch1].op.state == C_RUN){
								myData->bData[bd1].cData[ch1].signal[C_SIG_OVP_GROUP_PAUSE] = P1;
							}
						}
					}
				}
			}
		}
	}
}

void Check_OTP(void)
{
	if(myData->mData.config.LGES_fault_config.otp_pause_flag == 1){
		Check_OTP_1();	
	}else{
		Check_OTP_Default();
	}
}

void Check_OTP_Default(void)
{
	unsigned short chNo = 0, bdNo = 0;
	int bd, ch, diff, count;
	long event_val = 0;

	if(myData->mData.config.function[F_OTP] == P0) return;
	if(myData->AppControl.config.debugType != P0) return;

	if(myData->dio.delayTimer < myData->dio.config.dioDelay) {
		for(bd=0; bd < myPs->config.installedBd; bd++) {
			for(ch=0; ch < myPs->config.chPerBd; ch++) {
				if((myPs->config.chPerBd * bd + ch)
					> (myPs->config.installedCh - 1)) continue;

				myData->bData[bd].cData[ch].misc.hw_fault_otp
					= myData->mData.misc.timer_1sec;
			}
		}

		return;
	}

	count = 0;
	for(bd=0; bd < myPs->config.installedBd; bd++) {
		for(ch=0; ch < myPs->config.chPerBd; ch++) {
			if((myPs->config.chPerBd * bd + ch)
				> (myPs->config.installedCh - 1)) continue;

			if((myData->bData[bd].cData[ch].op.temp
				< TEMP_CONNECT_ERROR_VALUE)
				&& (myData->bData[bd].cData[ch].op.temp1
				< TEMP_CONNECT_ERROR_VALUE)) {
				if(myData->bData[bd].cData[ch].op.temp 
					>= myData->mData.config.hwFaultConfig[HW_FAULT_OTP]) {
					diff = myData->mData.misc.timer_1sec
						- myData->bData[bd].cData[ch].misc.hw_fault_otp;
					if(diff >= 20) { //20Sec
						count++;
					}
					//20190527
					event_val = (long)myData->bData[bd].cData[ch].op.temp;
					bdNo = bd;
					chNo = (myPs->config.chPerBd * bd) + (ch + 1);
				} else {
					myData->bData[bd].cData[ch].misc.hw_fault_otp
						= myData->mData.misc.timer_1sec;
				}
			}
		}
	}

	if(count > 0){
		if(myData->dio.signal[DIO_SIG_OTP] == P0) {
			myData->dio.signal[DIO_SIG_OTP] = P1;
			if((inb(0x602) & 0x10) == P0) { //190701
#if CYCLER_TYPE == DIGITAL_CYC
				//180701 add
				myPs->signal[M_SIG_INV_POWER] = P100;
				myPs->signal[M_SIG_INV_POWER1] = P100;
#endif	
				//210303 Inverter off 
				if(myData->AppControl.config.systemType == CYCLER_CAN){ 
					myPs->signal[M_SIG_INV_POWER_CAN] = P100;
				}
				myPs->signal[M_SIG_RUN_LED] = P0;
				myPs->signal[M_SIG_REMOTE_SMPS1] = P0;
				myPs->signal[M_SIG_FAN_RELAY] = P0;
				myPs->signal[M_SIG_POWER_OFF] = P1;
				send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 21, chNo); //halt
			}
			send_msg_2(MODULE_TO_APP_VALUE, MSG_MODULE_APP_EMG,
				2, chNo, bdNo, event_val); //SBC Log
			myPs->code = M_FAIL_OTP;
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
}

void Check_OTP_1(void)
{
	long otp_time;
	int bd = 0, ch = 0, bd1 = 0, ch1 = 0, index, i;

	if(myData->mData.config.function[F_OTP] == P0) return;
	if(myData->AppControl.config.debugType != P0) return;

	otp_time = myData->mData.config.LGES_fault_config.otp_check_time / 100;

	if(myData->dio.delayTimer < myData->dio.config.dioDelay){
		for(bd=0; bd < myPs->config.installedBd; bd++){
			for(ch=0; ch < myPs->config.chPerBd; ch++){
				if((myPs->config.chPerBd * bd + ch)
					> (myPs->config.installedCh - 1)) continue;

				myData->bData[bd].cData[ch].misc.hw_fault_otp
					= myData->mData.misc.timer_1sec;
			}
		}

		return;
	}
	
	for(bd=0; bd < myPs->config.installedBd; bd++){
		for(ch=0; ch < myPs->config.chPerBd; ch++){
			if((myPs->config.chPerBd * bd + ch)
				> (myPs->config.installedCh - 1)) continue;
				
			index = (myPs->config.chPerBd * bd + ch);
			myData->bData[bd].cData[ch].misc.otp_ChamberNo 
				= myData->ChamberChNo[index].number1;

			if((myData->bData[bd].cData[ch].op.temp
				< TEMP_CONNECT_ERROR_VALUE)
				&& (myData->bData[bd].cData[ch].op.temp1
				< TEMP_CONNECT_ERROR_VALUE)){
				if(myData->bData[bd].cData[ch].op.temp 
					>= myData->mData.config.hwFaultConfig[HW_FAULT_OTP]){
					if((myData->mData.misc.timer_1sec
						- myData->bData[bd].cData[ch].misc.hw_fault_otp) >= otp_time){
						//pause
						myData->bData[bd].cData[ch].misc.otp_fault_signal = 1;
					}
				}else{
					myData->bData[bd].cData[ch].misc.hw_fault_otp
						= myData->mData.misc.timer_1sec;
					myData->bData[bd].cData[ch].misc.otp_fault_signal = 0;
				}
			}
		}
	}
	
	for(bd=0; bd < myPs->config.installedBd; bd++){
		for(ch=0; ch < myPs->config.installedCh; ch++){
			if(myData->bData[bd].cData[ch].misc.otp_fault_signal == 1){
				for(i = 0; i< myData->mData.config.installedCh; i++){
					if(myData->bData[bd].cData[ch].misc.otp_ChamberNo 
							== myData->ChamberChNo[i].number1){
						bd1 = myData->ChamberChNo[i].bd;
						ch1 = myData->ChamberChNo[i].ch;
						if((myPs->config.chPerBd * bd + ch)
							> (myPs->config.installedCh-1)){
							continue;
						}
						if(bd == bd1 && ch == ch1){
							if(myData->bData[bd1].cData[ch1].op.state == C_RUN){
								myData->bData[bd1].cData[ch1].signal[C_SIG_OTP_PAUSE] = P1;
							}
						}else{
							if(myData->bData[bd1].cData[ch1].op.state == C_RUN){
								myData->bData[bd1].cData[ch1].signal[C_SIG_OTP_GROUP_PAUSE] = P1;
							}
						}
					}
				}
			}
		}
	}
}

void Check_CCC(void)
{
	unsigned short chNo = 0, bdNo = 0, clear;
	int bd, ch, diff, count, parallel_ch;
	long event_val = 0;

	if(myData->mData.config.function[F_CCC] == P0) return;
	if(myData->AppControl.config.debugType != P0) return;

	if(myData->dio.delayTimer < myData->dio.config.dioDelay) {
		for(bd=0; bd < myPs->config.installedBd; bd++) {
			for(ch=0; ch < myPs->config.chPerBd; ch++) {
				if((myPs->config.chPerBd * bd + ch)
					> (myPs->config.installedCh-1)) continue;

				myData->bData[bd].cData[ch].misc.hw_fault_ccc
					= myData->mData.misc.timer_1sec;
			}
		}

		return;
	}

	count = 0;
	for(bd=0; bd < myPs->config.installedBd; bd++) {
		for(ch=0; ch < myPs->config.chPerBd; ch++) {
			if((myPs->config.chPerBd * bd + ch)
				> (myPs->config.installedCh-1)) continue;

			if((ch % 2) == 0) parallel_ch = ch + 1;
			else parallel_ch = ch - 1;

			clear = 1;
			if(myData->bData[bd].cData[ch].op.state != C_RUN
				&& myData->bData[bd].cData[ch].op.state != C_CALI) {
				if(labs(myData->bData[bd].cData[ch].op.Isens)
					>= myData->mData.config.hwFaultConfig[HW_FAULT_CCC]) {
					if(myData->bData[bd].cData[parallel_ch].misc
						.parallel_cycle_phase == P50) { //kjg_180521
						clear = 1;
					} else {
						clear = 0;
					}
				}
			}

			if(clear == 1) {
				myData->bData[bd].cData[ch].misc.hw_fault_ccc
					= myData->mData.misc.timer_1sec;
			} else {
				diff = myData->mData.misc.timer_1sec
					- myData->bData[bd].cData[ch].misc.hw_fault_ccc;
				if(diff >= 20) count++; //20Sec
				//20190527
				event_val = (long)myData->bData[bd].cData[ch].op.Isens;
				bdNo = bd;
				chNo = (myPs->config.chPerBd * bd) + (ch+1);
			}
		}
	}

	if(count > 0) {
		if(myData->dio.signal[DIO_SIG_CCC] == P0) {
			myData->dio.signal[DIO_SIG_CCC] = P1;
			if((inb(0x602) & 0x10) == P0) { //190701
#if CYCLER_TYPE == DIGITAL_CYC
				//180701 add
				myPs->signal[M_SIG_INV_POWER] = P100;
				myPs->signal[M_SIG_INV_POWER1] = P100;
#endif	
				//210303 Inverter off 
				if(myData->AppControl.config.systemType == CYCLER_CAN){ 
					myPs->signal[M_SIG_INV_POWER_CAN] = P100;
				}
				myPs->signal[M_SIG_RUN_LED] = P0;
				myPs->signal[M_SIG_REMOTE_SMPS1] = P0;
				myPs->signal[M_SIG_FAN_RELAY] = P0;
				myPs->signal[M_SIG_POWER_OFF] = P1;
				send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 22, chNo); //halt
			}
			send_msg_2(MODULE_TO_APP_VALUE, MSG_MODULE_APP_EMG,
				3, chNo, bdNo, event_val); //SBC Log
			myPs->code = M_FAIL_CCC;
			send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
				(int)myPs->code, chNo);

			for(bd=0; bd < myPs->config.installedBd; bd++) {
				for(ch=0; ch < myPs->config.chPerBd; ch++) {
					if((myPs->config.chPerBd * bd + ch)
						> (myPs->config.installedCh-1)) continue;

					myData->bData[bd].cData[ch].signal[C_SIG_PAUSE] = P1;
				}
			}
		}
	}
}

//151023 Addr_Interface USE Input(addr) = 603
int DIn_FlagCheck_DC_FAN_FAIL1(void)
{
	int rtn=0, bd, ch;
	unsigned char flag;

	flag = (unsigned char)Read_InPoint(I_IN_LOWSPEC_DIO_FAIL1);
	
	if(flag == P1) {
		if(myData->dio.signal[DIO_SIG_DC_FAN_FAIL1] == P0) {
			myData->dio.signal[DIO_SIG_DC_FAN_FAIL1] = P1;
			myPs->code = M_FAIL_DC_FAN;
			send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 23, 1);
			send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
				(int)myPs->code, 1);

			for(bd=0; bd < myPs->config.installedBd; bd++) {
				for(ch=0; ch < myPs->config.chPerBd; ch++) {
					if((myPs->config.chPerBd * bd + ch)
						> (myPs->config.installedCh-1)) {
						continue;
					}
					if(myData->bData[bd].cData[ch].op.state == C_RUN) {
						myData->bData[bd].cData[ch]
							.signal[C_SIG_DC_FAN_FAIL] = P1;
					}
				}
			}
			rtn = -1;
		}
	} else {
		myData->dio.signal[DIO_SIG_DC_FAN_FAIL1] = P0;
	}
	return rtn;
}

int DIn_FlagCheck_DC_FAN_FAIL2(void)
{
	int rtn=0, bd, ch;
	unsigned char flag;

	flag = (unsigned char)Read_InPoint(I_IN_LOWSPEC_DIO_FAIL2);
	
	if(flag == P1) {
		if(myData->dio.signal[DIO_SIG_DC_FAN_FAIL2] == P0) {
			myData->dio.signal[DIO_SIG_DC_FAN_FAIL2] = P1;
			myPs->code = M_FAIL_DC_FAN;
			send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 24, 2);
			send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
				(int)myPs->code, 2);
			
			for(bd=0; bd < myPs->config.installedBd; bd++) {
				for(ch=0; ch < myPs->config.chPerBd; ch++) {
					if((myPs->config.chPerBd * bd + ch)
						> (myPs->config.installedCh-1)) {
						continue;
					}
					if(myData->bData[bd].cData[ch].op.state == C_RUN) {
						myData->bData[bd].cData[ch]
							.signal[C_SIG_DC_FAN_FAIL] = P1;
					}
				}
			}
			rtn = -1;
		}
	} else {
		myData->dio.signal[DIO_SIG_DC_FAN_FAIL2] = P0;
	}
	return rtn;
}
//160701 SCH Add for Door Open
int DIn_Door_Open_FAIL1(void)
{
	int  bd, ch, rtn = 0;
	unsigned short sendFlag = 0;
	unsigned char flag = 0, flag1, flag2;
	
	switch(myPs->config.hwSpec){
		case L_5V_1A_R3:
		case L_5V_10A_R3_NEW:		
			flag = (unsigned char)Read_InPoint(I_IN_LOWSPEC_DIO_FAIL1);
			if(flag == P1){ //20191118
				for(bd=0; bd < myPs->config.installedBd; bd++) {
					for(ch=0; ch < myPs->config.chPerBd; ch++) {
						if((myPs->config.chPerBd * bd + ch)
							> myPs->config.installedCh-1) {
							continue;
						}
						if(myData->bData[bd].cData[ch].op.state == C_RUN) {
							myData->dio.signal[DIO_SIG_DOOR_OPEN_FAIL1] = P0;
						}
					}
				}
			}
			break;
		case DC_5V_CYCLER_NEW:
			flag = (unsigned char)Read_InPoint(I_IN_LOWSPEC_DIO_FAIL3+6);
			break;
		case C_5V_CYCLER_CAN: //210406
			flag1 = (unsigned char)Read_InPoint(I_IN_DOOR_OPEN_FAIL1);
		    flag2 = myData->CAN.io.inputValue[0];
			if(flag1 == 1 && flag == 2) flag = 1;
			if(flag == P1){ 
				for(bd=0; bd < myPs->config.installedBd; bd++) {
					for(ch=0; ch < myPs->config.chPerBd; ch++) {
						if((myPs->config.chPerBd * bd + ch)
							> myPs->config.installedCh-1) {
							continue;
						}
						if(myData->bData[bd].cData[ch].op.state == C_RUN) {
							myData->dio.signal[DIO_SIG_DOOR_OPEN_FAIL1] = P0;
						}
					}
				}
			}
			break;	
		default:
			flag = (unsigned char)Read_InPoint(I_IN_DOOR_OPEN_FAIL1);
			if(flag == P1){
				for(bd=0; bd < myPs->config.installedBd; bd++) {
					for(ch=0; ch < myPs->config.chPerBd; ch++) {
						if((myPs->config.chPerBd * bd + ch)
							> myPs->config.installedCh-1) {
							continue;
						}
						if(myData->bData[bd].cData[ch].op.state == C_RUN) {
							myData->dio.signal[DIO_SIG_DOOR_OPEN_FAIL1] = P0;
						}
					}
				}
			}
			break;
	}
	
	if(flag == P1) {
		if(myData->dio.signal[DIO_SIG_DOOR_OPEN_FAIL1] == P0) {
			myData->dio.signal[DIO_SIG_DOOR_OPEN_FAIL1] = P1;
			if(myData->mData.config.dyson_maintenance_flag == 1){
				myPs->code = M_FAIL_DOOR_OPEN1;
				rtn = DYSON_Door_Open(myPs->code,25,1);
			}else{
				myPs->code = M_FAIL_DOOR_OPEN1;
				send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 25, 1);
				send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
					(int)myPs->code, sendFlag);
			
				for(bd=0; bd < myPs->config.installedBd; bd++) {
					for(ch=0; ch < myPs->config.chPerBd; ch++) {
						if((myPs->config.chPerBd * bd + ch)
							> myPs->config.installedCh-1) {
							continue;
						}
						if(myData->bData[bd].cData[ch].op.state == C_RUN) {
							myData->bData[bd].cData[ch]
								.signal[C_SIG_DOOR_OPEN_FAIL] = P1;
						}
					}
				}
				rtn = -1;
			}
		}
	} else {
		myData->dio.signal[DIO_SIG_DOOR_OPEN_FAIL1] = P0;
	}
	return rtn;		
}

int DIn_Door_Open_FAIL2(void)
{
	int  bd, ch, rtn = 0;
	unsigned short sendFlag = 0;
	unsigned char flag;

	switch(myPs->config.hwSpec){
		case L_5V_1A_R3:
		case L_5V_10A_R3_NEW:		
			flag = (unsigned char)Read_InPoint(I_IN_LOWSPEC_DIO_FAIL2);
			if(flag == P1){ //20191118
				for(bd=0; bd < myPs->config.installedBd; bd++) {
					for(ch=0; ch < myPs->config.chPerBd; ch++) {
						if((myPs->config.chPerBd * bd + ch)
							> myPs->config.installedCh-1) {
							continue;
						}
						if(myData->bData[bd].cData[ch].op.state == C_RUN) {
							myData->dio.signal[DIO_SIG_DOOR_OPEN_FAIL2] = P0;
						}
					}
				}
			}
			break;
		case DC_5V_CYCLER_NEW:
			flag = (unsigned char)Read_InPoint(I_IN_LOWSPEC_DIO_FAIL3+7);
			break;
		default:
			flag = (unsigned char)Read_InPoint(I_IN_DOOR_OPEN_FAIL2);
			if(flag == P1){
				for(bd=0; bd < myPs->config.installedBd; bd++) {
					for(ch=0; ch < myPs->config.chPerBd; ch++) {
						if((myPs->config.chPerBd * bd + ch)
							> myPs->config.installedCh-1) {
							continue;
						}
						if(myData->bData[bd].cData[ch].op.state == C_RUN) {
							myData->dio.signal[DIO_SIG_DOOR_OPEN_FAIL2] = P0;
						}
					}
				}
			}
			break;
	}
		
	if(flag == P1) {
		if(myData->dio.signal[DIO_SIG_DOOR_OPEN_FAIL2] == P0) {
			myData->dio.signal[DIO_SIG_DOOR_OPEN_FAIL2] = P1;
			if(myData->mData.config.dyson_maintenance_flag == 1){
				myPs->code = M_FAIL_DOOR_OPEN2;
				rtn = DYSON_Door_Open(myPs->code,26,2);
			}else{
				myPs->code = M_FAIL_DOOR_OPEN2;
				send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 26, 2);
				send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
					(int)myPs->code, sendFlag);
				
				for(bd=0; bd < myPs->config.installedBd; bd++) {
					for(ch=0; ch < myPs->config.chPerBd; ch++) {
						if((myPs->config.chPerBd * bd + ch)
							> myPs->config.installedCh-1) {
							continue;
						}
						if(myData->bData[bd].cData[ch].op.state == C_RUN) {
							myData->bData[bd].cData[ch]
								.signal[C_SIG_DOOR_OPEN_FAIL] = P1;
						}
					}
				}
				rtn = -1;
			}
		}
	} else {
		myData->dio.signal[DIO_SIG_DOOR_OPEN_FAIL2] = P0;
	}
	return rtn;		
}

int DIn_Door_Open_FAIL(void)
{	//180822 add
	int  bd, ch, rtn = 0;
	unsigned short sendFlag = 0;
	unsigned char flag1, flag2, flag3;

	switch(myPs->config.hwSpec){
		case DC_5V_CYCLER_NEW:
			flag1 = (unsigned char)Read_InPoint(I_IN_DIO_DOOR_FAIL1);
			flag2 = (unsigned char)Read_InPoint(I_IN_DIO_DOOR_FAIL2);
			flag3 = (unsigned char)Read_InPoint(I_IN_DIO_DOOR_FAIL3);
			break;
		default:
			flag1 = (unsigned char)Read_InPoint(I_IN_DIO_DOOR_FAIL1);
			flag2 = (unsigned char)Read_InPoint(I_IN_DIO_DOOR_FAIL2);
			flag3 = (unsigned char)Read_InPoint(I_IN_DIO_DOOR_FAIL3);
			break;
	}
	
	if(flag1 == P1 || flag2 == P1 || flag3 == P3){
		for(bd=0; bd < myPs->config.installedBd; bd++){
			for(ch=0; ch < myPs->config.chPerBd; ch++){
				if((myPs->config.chPerBd * bd + ch) 
					> myPs->config.installedCh-1){					
						continue;
				}
				if(myData->bData[bd].cData[ch].op.state == C_RUN) {
					myData->dio.signal[DIO_SIG_DOOR_OPEN_FAIL1] = P0;
					myData->dio.signal[DIO_SIG_DOOR_OPEN_FAIL2] = P0;
					myData->dio.signal[DIO_SIG_DOOR_OPEN_FAIL3] = P0;
				}
			}
		}
	}
	
	if(flag1 == P1){
		if(myData->dio.signal[DIO_SIG_DOOR_OPEN_FAIL1] == P0) {
			myData->dio.signal[DIO_SIG_DOOR_OPEN_FAIL1] = P1;
			if(myData->mData.config.dyson_maintenance_flag == 1){
				myPs->code = M_FAIL_DOOR_OPEN1;
				rtn = DYSON_Door_Open(myPs->code,25,3);
			}else{
				myPs->code = M_FAIL_DOOR_OPEN1;
				send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 25, 3);
				send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
					(int)myPs->code, sendFlag);
				
				for(bd=0; bd < myPs->config.installedBd; bd++) {
					for(ch=0; ch < myPs->config.chPerBd; ch++) {
						if((myPs->config.chPerBd * bd + ch)
							> myPs->config.installedCh-1) {
							continue;
						}
						if(myData->bData[bd].cData[ch].op.state == C_RUN) {
							myData->bData[bd].cData[ch]
								.signal[C_SIG_DOOR_OPEN_FAIL] = P1;
						}
					}
				}
				rtn = -1;
			}
		}
	} else {
		myData->dio.signal[DIO_SIG_DOOR_OPEN_FAIL1] = P0;
	}

	if(flag2 == P1){
		if(myData->dio.signal[DIO_SIG_DOOR_OPEN_FAIL2] == P0) {
			myData->dio.signal[DIO_SIG_DOOR_OPEN_FAIL2] = P1;
			if(myData->mData.config.dyson_maintenance_flag == 1){
				myPs->code = M_FAIL_DOOR_OPEN1;
				rtn = DYSON_Door_Open(myPs->code,26,3);
			}else{
				myPs->code = M_FAIL_DOOR_OPEN1;
				send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 26, 3);
				send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
					(int)myPs->code, sendFlag);
				
				for(bd=0; bd < myPs->config.installedBd; bd++) {
					for(ch=0; ch < myPs->config.chPerBd; ch++) {
						if((myPs->config.chPerBd * bd + ch)
							> myPs->config.installedCh-1) {
							continue;
							}
						if(myData->bData[bd].cData[ch].op.state == C_RUN) {
							myData->bData[bd].cData[ch]
								.signal[C_SIG_DOOR_OPEN_FAIL] = P1;
						}
					}
				}
				rtn = -1;
			}
		}
	} else {
		myData->dio.signal[DIO_SIG_DOOR_OPEN_FAIL2] = P0;
	}

	if(flag3 == P1){
		if(myData->dio.signal[DIO_SIG_DOOR_OPEN_FAIL3] == P0) {
			myData->dio.signal[DIO_SIG_DOOR_OPEN_FAIL3] = P1;
			if(myData->mData.config.dyson_maintenance_flag == 1){
				myPs->code = M_FAIL_DOOR_OPEN3;
				rtn = DYSON_Door_Open(myPs->code,29,3);
			}else{
				myPs->code = M_FAIL_DOOR_OPEN3;
				send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 29, 3);
				send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
					(int)myPs->code, sendFlag);
				
				for(bd=0; bd < myPs->config.installedBd; bd++) {
					for(ch=0; ch < myPs->config.chPerBd; ch++) {
						if((myPs->config.chPerBd * bd + ch)
							> myPs->config.installedCh-1) {
							continue;
						}
						if(myData->bData[bd].cData[ch].op.state == C_RUN) {
							myData->bData[bd].cData[ch]
								.signal[C_SIG_DOOR_OPEN_FAIL] = P1;
						}
					}
				}
				rtn = -1;
			}
		}
	} else {
		myData->dio.signal[DIO_SIG_DOOR_OPEN_FAIL3] = P0;
	}
	return rtn;		
}

int DYSON_Door_Open(int code, int val1, int val2) //220127 jsh
{
	int bd, ch; 
	unsigned short sendFlag = 0;

	if(myData->mData.config.DYSON_maintenance.door_pause_flag == 0){
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
		myPs->code = M_FAIL_DOOR_OPEN4;
		myPs->signal[M_SIG_MAIN_MC_OFF] = P1;	//220214_hun
		send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 37, 0); 
		send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
			(int)myPs->code, sendFlag);
		for(bd=0; bd < myPs->config.installedBd; bd++) {
			for(ch=0; ch < myPs->config.chPerBd; ch++) {
				if((myPs->config.chPerBd * bd + ch)
					> (myPs->config.installedCh - 1)) continue;
					myData->bData[bd].cData[ch].signal[C_SIG_PAUSE] = P1;
			}
		}
	}else{
		myPs->code = code;
		myPs->signal[M_SIG_MAIN_MC_OFF] = P0;	//220214_hun
		send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, val1, val2);
		send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
			(int)myPs->code, sendFlag);
		
		for(bd=0; bd < myPs->config.installedBd; bd++) {
			for(ch=0; ch < myPs->config.chPerBd; ch++) {
				if((myPs->config.chPerBd * bd + ch)
					> myPs->config.installedCh-1) {
					continue;
					}
				if(myData->bData[bd].cData[ch].op.state == C_RUN) {
					myData->bData[bd].cData[ch]
						.signal[C_SIG_DOOR_OPEN_FAIL] = P1;
				}
			}
		}
	}
	return -1;
}


//160701 SCH Add for Smoke Sensor
int	DIn_Smoke_FAIL(void)
{
	int  bd, ch, rtn = 0;
	unsigned short sendFlag = 0;
	unsigned char flag1 = 0, flag2 = 0;
	
	switch(myPs->config.hwSpec){
		case L_5V_1A_R3:
		case L_5V_10A_R3_NEW:		
			flag1 = (unsigned char)Read_InPoint(I_IN_LOWSPEC_DIO_FAIL3);
			break;
		default:
	//		flag = (unsigned char)Read_InPoint(I_IN_LOWSPEC_DIO_FAIL2+12);
			flag1 = (unsigned char)Read_InPoint(I_IN_SMOKE_SENSOR_FAIL1);
			flag2 = (unsigned char)Read_InPoint(I_IN_SMOKE_SENSOR_FAIL2);
			
			if(flag1 == P1 || flag2 == P1){
				for(bd=0; bd < myPs->config.installedBd; bd++) {
					for(ch=0; ch < myPs->config.chPerBd; ch++) {
						if((myPs->config.chPerBd * bd + ch)
							> myPs->config.installedCh-1) {
							continue;
						}
						if(myData->bData[bd].cData[ch].op.state == C_RUN) {
							myData->dio.signal[DIO_SIG_SMOKE_FAIL] = P0;
						}
					}
				}
			}
			break;
	}

//	flag = (unsigned char)Read_InPoint(I_IN_LOWSPEC_DIO_FAIL3);
	
	if(flag1 == P1 || flag2 == P1) {
		if(myData->dio.signal[DIO_SIG_SMOKE_FAIL] == P0) {
#if CYCLER_TYPE == DIGITAL_CYC
			//180611 add for digtal
			myPs->signal[M_SIG_INV_POWER] = P100;
			myPs->signal[M_SIG_INV_POWER1] = P100;
#endif
			//210303 Inverter off 
			if(myData->AppControl.config.systemType == CYCLER_CAN){ 
				myPs->signal[M_SIG_INV_POWER_CAN] = P100;
			}

			if((inb(0x602) & 0x10) == P0) {
				myPs->signal[M_SIG_RUN_LED] = P0;
				myPs->signal[M_SIG_REMOTE_SMPS1] = P0;
				myPs->signal[M_SIG_FAN_RELAY] = P0;
				myPs->signal[M_SIG_POWER_OFF] = P1;
				if(flag1 == P1) {
					send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 27, 1);
				}
				if(flag2 == P1) {
					send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 30, 2);
				}
			}
			if(flag1 == P1) {
				myPs->code = M_FAIL_SMOKE;
			}
			if(flag2 == P1) {
				myPs->code = M_FAIL_SMOKE2;
			}
			myData->dio.signal[DIO_SIG_SMOKE_FAIL] = P1;
			send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
					(int)myPs->code, sendFlag);
			for(bd=0; bd < myPs->config.installedBd; bd++) {
				for(ch=0; ch < myPs->config.chPerBd; ch++) {
					if((myPs->config.chPerBd * bd + ch)
						> (myPs->config.installedCh-1)) {
						continue;
					}
					if(myData->bData[bd].cData[ch].op.state == C_RUN) {
						myData->bData[bd].cData[ch]
							.signal[C_SIG_SMOKE_FAIL] = P1;
					}
				#if DIO_TYPE == 4 //Hungary SDI Smoke EMG -> Buzzer On
				outb(0x10, 0x610);		
				#endif
				}
			}
			rtn = -1;
		}
	} else {
		myData->dio.signal[DIO_SIG_SMOKE_FAIL] = P0;
	}
	return rtn;		
}

void EMG_Signal_Check(void)
{	//191001 lyhw
	int i, rtn = 0;
	
	for(i=0; i < MAX_SIGNAL; i++){
		if(i == DIO_SIG_NONE_FAIL) continue;	
		
		if(myData->dio.signal[i] == P1){ 
			rtn = P1; 
			myData->dio.signal[DIO_SIG_NONE_FAIL] = P0;
			break;
		}else{	
			continue;
		}
	}
	
	if(myData->MainClient.signal[MAIN_SIG_NET_CONNECTED] == P2){
		if(rtn == P0 && myData->dio.signal[DIO_SIG_NONE_FAIL] == P0){
			myData->dio.signal[DIO_SIG_NONE_FAIL] = P1;
			myPs->code = FAIL_NONE;
			send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS, 
									(int)myPs->code, 0);
		}
	}
	return;
}

int Dout_TowerLamp_Flag(void)
{
	int rtn = 0;
#ifdef _JIG_TYPE_0
	unsigned char useflag;
    useflag = myData->dio.dout.outUseFlag[I_OUT_STATE_RUN];
	if(useflag == P1)
	{ 
		if(myPs->signal[M_SIG_LAMP_RUN] == P1)
			myData->dio.dout.outFlag[I_OUT_STATE_RUN] = P1;
		else myData->dio.dout.outFlag[I_OUT_STATE_RUN] = P0;
	
		if(myPs->signal[M_SIG_LAMP_STOP] == P1)
			myData->dio.dout.outFlag[I_OUT_STATE_STOP] = P1;
		else myData->dio.dout.outFlag[I_OUT_STATE_STOP] = P0;
		
		if(myPs->signal[M_SIG_LAMP_PAUSE] == P1)
			myData->dio.dout.outFlag[I_OUT_STATE_PAUSE] = P1;
		else myData->dio.dout.outFlag[I_OUT_STATE_PAUSE] = P0;

		if(myPs->signal[M_SIG_LAMP_BUZZER] == P1){
			myData->dio.dout.outFlag[I_OUT_STATE_BUZZER] = P1;
		}else{
			myData->dio.dout.outFlag[I_OUT_STATE_BUZZER] = P0;	
		}
		if(myData->mData.config.code_buzzer_on == 1){
			if(myPs->code == M_FAIL_OVP){
				myData->dio.dout.outFlag[I_OUT_STATE_PAUSE] = P1;
				myData->dio.dout.outFlag[I_OUT_STATE_BUZZER] = P1;
			}			
		}else{
			if(myPs->code == M_FAIL_MAIN_EMG || 
				myPs->code == M_FAIL_OT || 
				myPs->code == M_FAIL_PANEL_METER_HIGH_ERROR || 
				myPs->code == M_FAIL_PANEL_METER_LOW_ERROR || 
				myPs->code == M_FAIL_AC_POWER_LONG ||
			    myPs->code == M_FAIL_DOOR_OPEN4 ) { 
					myData->dio.dout.outFlag[I_OUT_STATE_PAUSE] = P1;
					myData->dio.dout.outFlag[I_OUT_STATE_BUZZER] = P1;
			}
		}
	}
#endif
	return rtn;

}

int Dout_TowerLamp_Flag_MultiBd(void) //20181127 Add
{
	int rtn = 0;
#ifdef _JIG_TYPE_0
	unsigned char useflag;

    useflag = myData->dio.dout.outUseFlag[I_OUT_MULTI_RUN];
	if(useflag == P1)
	{ 
		if(myPs->signal[M_SIG_LAMP_RUN] == P1)
			myData->dio.dout.outFlag[I_OUT_MULTI_RUN] = P1;
		else myData->dio.dout.outFlag[I_OUT_MULTI_RUN] = P0;
	
		if(myPs->signal[M_SIG_LAMP_STOP] == P1)
			myData->dio.dout.outFlag[I_OUT_MULTI_STOP] = P1;
		else myData->dio.dout.outFlag[I_OUT_MULTI_STOP] = P0;
		
		if(myPs->signal[M_SIG_LAMP_PAUSE] == P1)
			myData->dio.dout.outFlag[I_OUT_MULTI_PAUSE] = P1;
		else myData->dio.dout.outFlag[I_OUT_MULTI_PAUSE] = P0;

		if(myPs->code == M_FAIL_MAIN_EMG || 
			myPs->code == M_FAIL_OT || 
			myPs->code == M_FAIL_PANEL_METER_HIGH_ERROR || 
			myPs->code == M_FAIL_PANEL_METER_LOW_ERROR || 
			myPs->code == M_FAIL_AC_POWER_LONG ) { 
			myData->dio.dout.outFlag[I_OUT_MULTI_PAUSE] = P1;
		}
	}
#endif
	return rtn;

}

