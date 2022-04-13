#include <asm/io.h>
#include <rtl_core.h>
#include "../../INC/datastore.h"
#include "message.h"
#include "BoardControl.h"

extern S_SYSTEM_DATA *myData;
extern S_MODULE_DATA *myPs;

void BoardControl(void)
{
#if CYCLER_TYPE == DIGITAL_CYC
	return;	//180611 add
#endif
	if(myData->AppControl.config.systemType == CYCLER_CAN) return; //190829 add
	if(myData->AppControl.config.debugType != P0) return;
	if(myData->dio.config.dio_Control_Flag == P0) return;

	if(myData->AppControl.config.versionNo >= 20110701){
		switch(myPs->config.hwSpec) {
			case L_5V_1A_R3:
			case L_5V_10A_R3_NEW:
			case L_5V_500mA_2uA_R4:
				ReadHardwareFault_Common_21();
				break;
			case L_MAIN_REV11:
				ReadHardwareFault_Common_22();
				break;
			case L_5V_60A_R2_1:
			case L_5V_20A_R3_NEW:
				ReadHardwareFault_Common_23();
				break;
			default:
//				ReadHardwareFault_Common_20();
				break;
		}
	}else{
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
			case L_5V_20A:
			case L_5V_50A:
				ReadHardwareFault_Common_1();
				break;
			case S_5V_200A:
				ReadHardwareFault_Common_2();
				break;
			case L_2V_100A:
			case L_5V_100A_R2:
				ReadHardwareFault_Common_3();
				break;
			case L_50V_50A:
				ReadHardwareFault_Common_4();
				break;
			case L_20V_25A:
				ReadHardwareFault_Common_5();
				break;
			case L_5V_30A_R1:
			case L_5V_50A_R2: 
			case L_5V_65A_R3: 
			case L_5V_50A_R2_1: 
				ReadHardwareFault_Common_6();
				break;
			case L_5V_2A_R1:
			case L_5V_2A_R2:
			case L_5V_4A_R2:
	//		case L_5V_6A_R3:
	//		case L_6V_6A_R1:
	//		case L_5V_100mA_R2:
			case L_5V_500mA_R2:
			case L_5V_5A_R2:
			case L_5V_1A_R2:
			case L_10V_5A_R2:
				ReadHardwareFault_Common_7();
				break;
			case L_6V_60A_R2:
			case L_6V_60A_R2_P:
			case L_5V_10A_R3:
				ReadHardwareFault_Common_8();
				break;
			default:	break;
		}
	}

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
		case L_5V_20A:
		case L_5V_50A:
		case L_50V_50A:
		case L_20V_25A:
		case L_5V_30A_R1:
		case L_5V_50A_R2:
		case L_5V_50A_R2_1:
		case L_5V_65A_R3: 
			WriteHardwareFault_Common_1();
			break;
		case L_5V_2A_R1:
		case L_5V_50A_R2_P:
			PS_Remote_1();
			break;
		default:	break;
	}
}
/*
void BoardControl(void)
{
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
		case L_5V_20A:
		case L_5V_50A:
			ReadHardwareFault_Common_1();
			break;
		case S_5V_200A:
			ReadHardwareFault_Common_2();
			break;
		case L_2V_100A:
		case L_5V_100A_R2:
			ReadHardwareFault_Common_3();
			break;
		case L_50V_50A:
			ReadHardwareFault_Common_4();
			break;
		case L_20V_25A:
			ReadHardwareFault_Common_5();
			break;
		case L_5V_30A_R1:
		case L_5V_50A_R2: 
		case L_5V_65A_R3: 
		case L_5V_50A_R2_1: 
			ReadHardwareFault_Common_6();
			break;
		case L_5V_2A_R1:
		case L_5V_2A_R2:
		case L_5V_4A_R2:
//		case L_5V_6A_R3:
//		case L_6V_6A_R1:
//		case L_5V_100mA_R2:
		case L_5V_500mA_R2:
		case L_5V_5A_R2:
		case L_5V_1A_R2:
		case L_10V_5A_R2:
			ReadHardwareFault_Common_7();
			break;
		case L_6V_60A_R2:
		case L_6V_60A_R2_P:
		case L_5V_10A_R3:
			ReadHardwareFault_Common_8();
			break;
		default:	break;
	}

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
		case L_5V_20A:
		case L_5V_50A:
		case L_50V_50A:
		case L_20V_25A:
		case L_5V_30A_R1:
		case L_5V_50A_R2:
		case L_5V_50A_R2_1:
		case L_5V_65A_R3: 
			WriteHardwareFault_Common_1();
			break;
		case L_5V_2A_R1:
			PS_Remote_1();
			break;
		default:	break;
	}
}
*/
void ReadHardwareFault_Common_1(void)
{
	int bd, ch, rtn;
	int addr, base_addr, hw_rd;
	unsigned char flag;

	if(myData->dio.delayTimer < myData->dio.config.dioDelay) return;
	if(myPs->signal[M_SIG_RUN_LED] == P0) return;

	base_addr = myPs->addr.main[BASE_ADDR];
	hw_rd = myPs->addr.main[HW_RD];

	addr = base_addr + hw_rd;
	flag = inb(addr);

	rtn = 0;
	if((flag & 0x01) != 0) rtn = 1; //reset latched
	for(bd=0; bd < myPs->config.installedBd; bd++) {
		if(rtn == 0) myData->bData[bd].signal[B_SIG_RESET_LATCHED] = P0;
		else myData->bData[bd].signal[B_SIG_RESET_LATCHED] = P1;
	}

	rtn = 0;
	if((flag & 0x02) == 0) rtn = 1; //smps fault
	if(rtn != 0) {
		if(myPs->signal[M_SIG_SMPS_FAULT] == P0) {
			myPs->signal[M_SIG_SMPS_FAULT] = P1;
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

	if(myPs->config.hwSpec == L_5V_10mA) {
		return;
	}

	for(bd=0; bd < myPs->config.installedBd; bd++) {
		rtn = 0;
		switch(bd) {
			case 0: if((flag & 0x04) != 0) rtn = 1; break;
			case 1: if((flag & 0x08) != 0) rtn = 2; break;
			case 2: if((flag & 0x10) != 0) rtn = 3; break;
			case 3: if((flag & 0x20) != 0) rtn = 4; break;
			case 4: if((flag & 0x40) != 0) rtn = 5; break;
		}
		if(rtn != 0) { //ot fault
			if(myPs->signal[M_SIG_OT_FAULT] == P0) {
				myPs->signal[M_SIG_OT_FAULT] = P1;
				myPs->code = M_FAIL_OT;
				send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 6, rtn);
				send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
					(int)myPs->code, rtn);
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
	}
}

void ReadHardwareFault_Common_2(void)
{
	int bd, ch, rtn;
	int addr, base_addr, hw_rd;
	unsigned char flag, flag2;

	if(myData->dio.delayTimer < myData->dio.config.dioDelay) return;
	if(myPs->signal[M_SIG_RUN_LED] == P0) return;

	bd = 0;
	rtn = 0;
	base_addr = myPs->addr.main[BASE_ADDR];
	hw_rd = myPs->addr.main[HW_RD];

	addr = base_addr + hw_rd;
	flag = inb(addr);
	flag2 = inb(addr + 1);

	for(ch=0; ch < myPs->config.chPerBd; ch++) {
		if((myPs->config.chPerBd * bd + ch) > (myPs->config.installedCh-1)) {
			continue;
		}
		switch(ch) {
			case 0: if((flag & 0x01) == 0) rtn = 1; break;
			case 1: if((flag & 0x02) == 0) rtn = 1; break;
			case 2: if((flag & 0x04) == 0) rtn = 1; break;
			case 3: if((flag & 0x08) == 0) rtn = 1; break;
			case 4: if((flag & 0x10) == 0) rtn = 1; break;
			case 5: if((flag & 0x20) == 0) rtn = 1; break;
			case 6: if((flag & 0x40) == 0) rtn = 1; break;
			case 7: if((flag & 0x80) == 0) rtn = 1; break;
			case 8: if((flag2 & 0x01) == 0) rtn = 2; break;
			case 9: if((flag2 & 0x02) == 0) rtn = 2; break;
			case 10: if((flag2 & 0x04) == 0) rtn = 2; break;
			case 11: if((flag2 & 0x08) == 0) rtn = 2; break;
			case 12: if((flag2 & 0x10) == 0) rtn = 2; break;
			case 13: if((flag2 & 0x20) == 0) rtn = 2; break;
			case 14: if((flag2 & 0x40) == 0) rtn = 2; break;
			case 15: if((flag2 & 0x80) == 0) rtn = 2; break;
		}
	}

	if(rtn != 0) { //ot fault
		if(myPs->signal[M_SIG_OT_FAULT] == P0) {
			myPs->signal[M_SIG_OT_FAULT] = P1;
			myPs->code = M_FAIL_OT;
			send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 6, rtn);
			send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
				(int)myPs->code, rtn);
			for(ch=0; ch < myPs->config.chPerBd; ch++) {
				if((myPs->config.chPerBd * bd + ch)
					> (myPs->config.installedCh-1)) {
					continue;
				}
				if(myData->bData[bd].cData[ch].op.state == C_RUN
					&& myData->bData[bd].cData[ch]
					.signal[C_SIG_OT_FAULT] == P0) {
					myData->bData[bd].cData[ch].signal[C_SIG_OT_FAULT] = P1;
				}
			}
		}
	}

	bd = 0;
	rtn = 0;
/*	baseAddr = BD_ADDR + COM_BD_HW_FAULT_IN_ADDR_1;
	flag = inb(baseAddr);
	flag2 = inb(baseAddr + 1);

	for(ch=0; ch < myPs->config.chPerBd; ch++) {
		switch(ch) {
			case 0: if((flag & 0x01) != 0) rtn = 1; break;
			case 1: if((flag & 0x02) != 0) rtn = 1; break;
			case 2: if((flag & 0x04) != 0) rtn = 1; break;
			case 3: if((flag & 0x08) != 0) rtn = 1; break;
			case 4: if((flag & 0x10) != 0) rtn = 1; break;
			case 5: if((flag & 0x20) != 0) rtn = 1; break;
			case 6: if((flag & 0x40) != 0) rtn = 1; break;
			case 7: if((flag & 0x80) != 0) rtn = 1; break;
			case 8: if((flag2 & 0x01) != 0) rtn = 2; break;
			case 9: if((flag2 & 0x02) != 0) rtn = 2; break;
			case 10: if((flag2 & 0x04) != 0) rtn = 2; break;
			case 11: if((flag2 & 0x08) != 0) rtn = 2; break;
			case 12: if((flag2 & 0x10) != 0) rtn = 2; break;
			case 13: if((flag2 & 0x20) != 0) rtn = 2; break;
			case 14: if((flag2 & 0x40) != 0) rtn = 2; break;
			case 15: if((flag2 & 0x80) != 0) rtn = 2; break;
		}
	}*/

	if(rtn != 0) { //hw fault
		/*if(myPs->signal[M_SIG_OT_FAULT] == P0) {
			myPs->signal[M_SIG_OT_FAULT] = P1;
			myPs->code = M_FAIL_OT;
			send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 6, rtn);
			send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
				(int)myPs->code, rtn);*/
			for(ch=0; ch < myPs->config.chPerBd; ch++) {
				if((myPs->config.chPerBd * bd + ch)
					> (myPs->config.installedCh-1)) {
					continue;
				}
				if(myData->bData[bd].cData[ch].op.state == C_RUN
					&& myData->bData[bd].cData[ch]
					.signal[C_SIG_HW_FAULT] == P0) {
					myData->bData[bd].cData[ch].signal[C_SIG_HW_FAULT] = P1;
				}
			}
		//}
	}
}

void ReadHardwareFault_Common_3(void)
{
	int bd, ch, rtn;
	int addr, base_addr, bd_ot;
	unsigned char flag, flag2;

	if(myData->dio.delayTimer < myData->dio.config.dioDelay) return;
	if(myPs->signal[M_SIG_RUN_LED] == P0) return;

	bd = 0;
	rtn = 0;
	base_addr = myPs->addr.main[BASE_ADDR];
	bd_ot = myPs->addr.main[BD_OT];

	addr = base_addr + bd_ot;
	flag = inb(addr);
	flag2 = inb(addr + 1);

	for(ch=0; ch < myPs->config.chPerBd; ch++) {
		if((myPs->config.chPerBd * bd + ch) > (myPs->config.installedCh-1)) {
			continue;
		}
		switch(ch) {
			case 0: if((flag & 0x01) != 0) rtn = 1; break;
			case 1: if((flag & 0x02) != 0) rtn = 1; break;
			case 2: if((flag & 0x04) != 0) rtn = 1; break;
			case 3: if((flag & 0x08) != 0) rtn = 1; break;
			case 4: if((flag & 0x10) != 0) rtn = 1; break;
			case 5: if((flag & 0x20) != 0) rtn = 1; break;
			case 6: if((flag & 0x40) != 0) rtn = 1; break;
			case 7: if((flag & 0x80) != 0) rtn = 1; break;
			case 8: if((flag2 & 0x01) != 0) rtn = 2; break;
			case 9: if((flag2 & 0x02) != 0) rtn = 2; break;
			case 10: if((flag2 & 0x04) != 0) rtn = 2; break;
			case 11: if((flag2 & 0x08) != 0) rtn = 2; break;
			case 12: if((flag2 & 0x10) != 0) rtn = 2; break;
			case 13: if((flag2 & 0x20) != 0) rtn = 2; break;
			case 14: if((flag2 & 0x40) != 0) rtn = 2; break;
			case 15: if((flag2 & 0x80) != 0) rtn = 2; break;
		}
	}

	if(rtn != 0) { //ot fault
		if(myPs->signal[M_SIG_OT_FAULT] == P0) {
			myPs->signal[M_SIG_OT_FAULT] = P1;
			myPs->code = M_FAIL_OT;
			send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 6, rtn);
			send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
				(int)myPs->code, rtn);
			for(ch=0; ch < myPs->config.chPerBd; ch++) {
				if((myPs->config.chPerBd * bd + ch)
					> (myPs->config.installedCh-1)) {
					continue;
				}
				if(myData->bData[bd].cData[ch].op.state == C_RUN
					&& myData->bData[bd].cData[ch]
					.signal[C_SIG_OT_FAULT] == P0) {
					myData->bData[bd].cData[ch].signal[C_SIG_OT_FAULT] = P1;
				}
			}
		}
	}

	bd = 0;
	rtn = 0;
/*	baseAddr = BD_ADDR + COM_BD_HW_FAULT_IN_ADDR_1;
	flag = inb(baseAddr);
	flag2 = inb(baseAddr + 1);

	for(ch=0; ch < myPs->config.chPerBd; ch++) {
		switch(ch) {
			case 0: if((flag & 0x01) != 0) rtn = 1; break;
			case 1: if((flag & 0x02) != 0) rtn = 1; break;
			case 2: if((flag & 0x04) != 0) rtn = 1; break;
			case 3: if((flag & 0x08) != 0) rtn = 1; break;
			case 4: if((flag & 0x10) != 0) rtn = 1; break;
			case 5: if((flag & 0x20) != 0) rtn = 1; break;
			case 6: if((flag & 0x40) != 0) rtn = 1; break;
			case 7: if((flag & 0x80) != 0) rtn = 1; break;
			case 8: if((flag2 & 0x01) != 0) rtn = 2; break;
			case 9: if((flag2 & 0x02) != 0) rtn = 2; break;
			case 10: if((flag2 & 0x04) != 0) rtn = 2; break;
			case 11: if((flag2 & 0x08) != 0) rtn = 2; break;
			case 12: if((flag2 & 0x10) != 0) rtn = 2; break;
			case 13: if((flag2 & 0x20) != 0) rtn = 2; break;
			case 14: if((flag2 & 0x40) != 0) rtn = 2; break;
			case 15: if((flag2 & 0x80) != 0) rtn = 2; break;
		}
	}*/

	if(rtn != 0) { //hw fault
		/*if(myPs->signal[M_SIG_OT_FAULT] == P0) {
			myPs->signal[M_SIG_OT_FAULT] = P1;
			myPs->code = M_FAIL_OT;
			send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 6, rtn);
			send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
				(int)myPs->code, rtn);*/
			for(ch=0; ch < myPs->config.chPerBd; ch++) {
				if((myPs->config.chPerBd * bd + ch)
					> (myPs->config.installedCh-1)) {
					continue;
				}
				if(myData->bData[bd].cData[ch].op.state == C_RUN
					&& myData->bData[bd].cData[ch]
					.signal[C_SIG_HW_FAULT] == P0) {
					myData->bData[bd].cData[ch].signal[C_SIG_HW_FAULT] = P1;
				}
			}
		//}
	}
}

void ReadHardwareFault_Common_4(void)
{
	int bd, ch, rtn;
	int	addr, base_addr, hw_rd;
	unsigned char flag;

	if(myData->dio.delayTimer < myData->dio.config.dioDelay) return;
	if(myPs->signal[M_SIG_FAN_RELAY] == P0) return;

	base_addr = myPs->addr.main[BASE_ADDR];
	hw_rd = myPs->addr.main[HW_RD];
	addr = base_addr + hw_rd;
	flag = inb(addr);

	rtn = 0;
	if((flag & 0x01) != 0) rtn = 1; //reset latched
	for(bd=0; bd < myPs->config.installedBd; bd++) {
		if(rtn == 0) myData->bData[bd].signal[B_SIG_RESET_LATCHED] = P0;
		else myData->bData[bd].signal[B_SIG_RESET_LATCHED] = P1;
	}

	rtn = 0;
	if((flag & 0x02) == 0) rtn = 1; //smps fault
	if(rtn != 0) {
		if(myPs->signal[M_SIG_SMPS_FAULT] == P0) {
			myPs->signal[M_SIG_SMPS_FAULT] = P1;
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

	for(ch=0; ch < myPs->config.installedCh; ch++) {
		rtn = 0;
		switch(ch) {
			case 0: if((flag & 0x04) != 0) rtn = 1; break;
			case 1: if((flag & 0x08) != 0) rtn = 2; break;
			case 2: if((flag & 0x10) != 0) rtn = 3; break;
			case 3: if((flag & 0x20) != 0) rtn = 4; break;
		}
		if(rtn != 0) { //ot fault
			if(myPs->signal[M_SIG_OT_FAULT] == P0) {
				myPs->signal[M_SIG_OT_FAULT] = P1;
				myPs->signal[M_SIG_RUN_LED] = P0;
				myPs->signal[M_SIG_REMOTE_SMPS1] = P0;
				myPs->signal[M_SIG_FAN_RELAY] = P0;
				myPs->signal[M_SIG_POWER_OFF] = P1;
				myPs->code = M_FAIL_OT;
				send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 6, rtn);
				send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
					(int)myPs->code, rtn);
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
	}
}

void ReadHardwareFault_Common_5(void)
{
	int bd, ch, rtn;
	int	addr, base_addr, hw_rd;
	unsigned char flag=0x00;

	if(myData->dio.delayTimer < myData->dio.config.dioDelay) return;
	if(myPs->signal[M_SIG_FAN_RELAY] == P0) return;

	base_addr = myPs->addr.main[BASE_ADDR];
	hw_rd = myPs->addr.main[HW_RD];
	addr = base_addr + hw_rd;
	flag = inb(addr);

	rtn = 0;
	if((flag & 0x01) != 0) rtn = 1; //reset latched
	for(bd=0; bd < myPs->config.installedBd; bd++) {
		if(rtn == 0) myData->bData[bd].signal[B_SIG_RESET_LATCHED] = P0;
		else myData->bData[bd].signal[B_SIG_RESET_LATCHED] = P1;
	}

	rtn = 0;
	if((flag & 0x02) != 0) rtn = 1; //OT fault
	if(rtn != 0) {
		if(myPs->signal[M_SIG_OT_FAULT] == P0) {
			myPs->signal[M_SIG_OT_FAULT] = P1;
			myPs->signal[M_SIG_RUN_LED] = P0;
			myPs->signal[M_SIG_REMOTE_SMPS1] = P0;
			myPs->signal[M_SIG_FAN_RELAY] = P0;
			myPs->signal[M_SIG_POWER_OFF] = P1;
			myPs->code = M_FAIL_OT;
			send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 6, rtn);
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
	}
}


void ReadHardwareFault_Common_6(void)
{
	int bd, ch;
	int	addr, base_addr, hw_rd;
	unsigned char flag=0x00;

	if(myData->dio.delayTimer < myData->dio.config.dioDelay) return;
	if(myPs->signal[M_SIG_FAN_RELAY] == P0) return;

	base_addr = myPs->addr.main[BASE_ADDR];
	hw_rd = myPs->addr.main[HW_RD];
	addr = base_addr + hw_rd;
	flag = inb(addr);

	if((flag & 0x01) != 0x00) { //OT fault
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

	if((flag & 0x20) != 0x20) { //control smps fault
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
						>= myPs->config.installedCh) {
						continue;
					}
					if(myData->bData[bd].cData[ch].op.state == C_RUN){
						if(myData->bData[bd].cData[ch]
							.signal[C_SIG_SMPS_FAULT] == P0) {
							send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
								(int)myPs->code, flag);
							myData->bData[bd].cData[ch]
								.signal[C_SIG_SMPS_FAULT] = P1;
						}
					}
				}
			}
		}
		return;
	}

	if((flag & 0x1E) != 0x1E) {//main smps fault 
		for(bd=0; bd < myPs->config.installedBd; bd++) {
			for(ch=0; ch < myPs->config.chPerBd; ch++) {
				if((myPs->config.chPerBd * bd + ch)
					>= myPs->config.installedCh) {
					continue;
				}
				if(myData->bData[bd].cData[ch].op.state == C_RUN){
					if(myData->bData[bd].cData[ch]
						.signal[C_SIG_SMPS_FAULT] == P0) {
						send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
							(int)myPs->code, flag);
						myData->bData[bd].cData[ch]
							.signal[C_SIG_SMPS_FAULT] = P1;
					}
				}
			}
		}
	}
}

void ReadHardwareFault_Common_7(void)
{
	int bd, ch, rtn = 0;
	int	addr=0, base_addr, hw_rd, addr_step;
	unsigned char flag=0x00;

	if(myData->dio.delayTimer < myData->dio.config.dioDelay) return;
	if(myPs->signal[M_SIG_FAN_RELAY] == P0) return;

	for(bd=0; bd < myPs->config.installedBd; bd++) {
		base_addr = myPs->addr.main[BASE_ADDR];
		addr_step = myPs->addr.main[ADDR_STEP];
		hw_rd = myPs->addr.main[HW_RD];
		addr = base_addr + addr_step * bd + hw_rd;
		flag = inb(addr);

		if((flag & 0x01) != 0x00) { //OT fault
			rtn = 1; 
			break;
		}
		if((flag & 0x02) != 0x02) { //control smps fault
			rtn = 2;
			break;
		}
	}

	if(rtn == 1){
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
						>= myPs->config.installedCh) continue;
				}
				if(myData->bData[bd].cData[ch].op.state == C_RUN){
					if(myData->bData[bd].cData[ch]
						.signal[C_SIG_OT_FAULT] == P0) {
						myData->bData[bd].cData[ch]
							.signal[C_SIG_OT_FAULT] = P1;
					}
				}
			}
		}
		return;
	}

	if(rtn == 2){
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
						>= myPs->config.installedCh) continue;
					if(myData->bData[bd].cData[ch].op.state == C_RUN){
						if(myData->bData[bd].cData[ch]
							.signal[C_SIG_SMPS_FAULT] == P0) {
							send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
								(int)myPs->code, flag);
							myData->bData[bd].cData[ch]
								.signal[C_SIG_SMPS_FAULT] = P1;
						}
					}
				}
			}
		}
	}
}

void ReadHardwareFault_Common_8(void)
{
	int bd, ch;
	int	addr, base_addr, hw_rd;
	unsigned char flag=0x00;

	if(myData->dio.delayTimer < myData->dio.config.dioDelay) return;
	if(myPs->signal[M_SIG_FAN_RELAY] == P0) return;

	base_addr = myPs->addr.main[BASE_ADDR];
	hw_rd = myPs->addr.main[HW_RD];
	addr = base_addr + hw_rd;
	flag = inb(addr);

	if((flag & 0x1E) != 0x1E) {//main smps fault 
		for(bd=0; bd < myPs->config.installedBd; bd++) {
			for(ch=0; ch < myPs->config.chPerBd; ch++) {
				if((myPs->config.chPerBd * bd + ch)
					>= myPs->config.installedCh) {
					continue;
				}
				if(myData->bData[bd].cData[ch].op.state == C_RUN){
					if(myData->bData[bd].cData[ch]
						.signal[C_SIG_SMPS_FAULT] == P0) {
						send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
							(int)myPs->code, flag);
						myData->bData[bd].cData[ch]
							.signal[C_SIG_SMPS_FAULT] = P1;
					}
				}
			}
		}
	}
}
/*	date: 2011. 7. 6					writer : pjy
 *	discription:
 *	1.checkFault(until installedBd)
 *		1.1 OT Check
 *		1.2 Main SMPS Fault Check
 *		1.3 Control SMPS Check
 *	2.OT fault service sequence
 *	3.Main SMPS fault service sequence
 *	4.Control SMPS fault service sequence
 */ 
void ReadHardwareFault_Common_20(void)
{
	int bd, ch, i;
	int	addr, base_addr, hw_rd, bd_step;
	unsigned char flag=0x00, otFlag=0, mainFlag=0, controlFlag=0, value=0;

	if(myData->dio.delayTimer < myData->dio.config.dioDelay) return;
	if(myPs->signal[M_SIG_FAN_RELAY] == P0) return;

	base_addr = myPs->addr.main[BASE_ADDR];
	bd_step = myPs->addr.main[ADDR_STEP];
	hw_rd = myPs->addr.main[HW_RD];

	//1. check fault (OT, Main SMPS, Control SMPS)
	for(bd = 0; bd < myData->mData.config.installedBd; bd++){
		addr = base_addr + bd_step * bd + hw_rd;
		flag = inb(addr);
		for(i = 0; i < 3; i++){
			if(myPs->board_fault[bd][i].useFlag != 0){
			   	if(myPs->board_fault[bd][i].active != 0){
					if((flag & myPs->board_fault[bd][i].bit) != 0){
						value = 1;
					}
				}else{
					if((flag & myPs->board_fault[bd][i].bit) 
						!= myPs->board_fault[bd][i].bit){
						value = 1;
					}	
				}
			}
			if(value != 0){
				switch(i){
					case 0: 
						otFlag = 1;
						break;
					case 1:
						mainFlag = 1;
						break;
					case 2:
						controlFlag = 1;
						break;
					default:
						break;
				}
			}
		}
	}
	//2. ot fault
	if(otFlag == 1){
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
	//3. main smps fault
	if(mainFlag == 1){
		if(myPs->signal[M_SIG_SMPS_FAULT] == P0){
			myPs->signal[M_SIG_SMPS_FAULT] = P1;
			myPs->signal[M_SIG_RUN_LED] = P0;
			myPs->signal[M_SIG_REMOTE_SMPS1] = P0;
			myPs->signal[M_SIG_FAN_RELAY] = P0;
			myPs->signal[M_SIG_POWER_OFF] = P1;
			myPs->code = M_FAIL_SMPS;
			send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 7, flag);
			send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
				(int)myPs->code, 1);
			for(bd=0; bd < myPs->config.installedBd; bd++) {
				for(ch=0; ch < myPs->config.chPerBd; ch++) {
					if((myPs->config.chPerBd * bd + ch)
						>= myPs->config.installedCh) continue;
					if(myData->bData[bd].cData[ch].op.state == C_RUN){
						if(myData->bData[bd].cData[ch]
							.signal[C_SIG_SMPS_FAULT] == P0) {
							send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
								(int)myPs->code, flag);
							myData->bData[bd].cData[ch]
								.signal[C_SIG_SMPS_FAULT] = P1;
						}
					}
				}
			}
		}
		return;
	}	
	//4. control smps fault
	if(controlFlag == 1){
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
						>= myPs->config.installedCh) {
						continue;
					}
					if(myData->bData[bd].cData[ch].op.state == C_RUN){
						if(myData->bData[bd].cData[ch]
							.signal[C_SIG_SMPS_FAULT] == P0) {
							send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
								(int)myPs->code, flag);
							myData->bData[bd].cData[ch]
								.signal[C_SIG_SMPS_FAULT] = P1;
						}
					}
				}
			}
		}
		return;
	}
}
/*	date: 2012. 5. 22										writer : pms
 	discription:
	
	ADR      7   |   6   |   5   |   4   |   3   |   2   |   1   |   0   |
	0x627   OT_4 |  OT_3 |  OT_2 |  OT_1 | C5V_F |CP15V_F|CM15V_F|PS_FAIL|
	0x628  7PS_F4| 7PS_F3| 7PS_F2| 7PS_F1| 3PS_F4| 3PS_F3| 3PS_F2| 3PS_F1|

	7PS_Fx  : 7V SMPS FAIL number			3PS_Fx  : 3V SMPS FAIL number
	OT_x    : Over Temperature number		C5V_F   : 5V Control SMPS FAIL
	CM15V_F : -15V Control SMPS FAIL		CP15V_F : +15V Control SMPS FAIL
	PS_FAIL : Power SMPS FAIL

	sequence:
 	1.checkFault(unit installedBd)
 		1.1 OT Check
 		1.2 Main SMPS Fault Check
 		1.3 Control SMPS Check
 	2.OT fault service sequence
 	3.Main SMPS fault service sequence
 	4.Control SMPS fault service sequence
 */ 
#if 1
void ReadHardwareFault_Common_21(void)
{
	int bd, ch;
	int	addr, base_addr, hw_rd, bd_step;
	unsigned char flag=0x00;
	unsigned char mainSmpsFail= 0x00;
	unsigned char hwFail= 0x00;
	int otFlag = 0;
	int mainFlag = 0;
	int controlFlag = 0;

	if(myData->dio.delayTimer < myData->dio.config.dioDelay) return;
	if(myPs->signal[M_SIG_FAN_RELAY] == P0) return;

	base_addr = myPs->addr.main[BASE_ADDR];
	bd_step = myPs->addr.main[ADDR_STEP];
	hw_rd = myPs->addr.main[HW_RD];
						
	
	//1. check fault
	for(bd = 0; bd < myData->mData.config.installedBd; bd++){	
		
		addr = base_addr + bd_step * bd + hw_rd -1;
		hwFail = inb(addr);
		
		addr = base_addr + bd_step * bd + hw_rd;
		mainSmpsFail = inb(addr);
		
		//1.1 OT Check(if the use flag define 1)
		if(myPs->board_fault[bd][0].useFlag != 0){
			//1.1.1 active high check
		   	if(myPs->board_fault[bd][0].active != 0){
				if((hwFail & myPs->board_fault[bd][0].bit) != 0){
					otFlag = 1;
				}
			//1.1.2 active low check
			}else{
				if((hwFail & myPs->board_fault[bd][0].bit) 
					!= myPs->board_fault[bd][0].bit){
					otFlag = 1;
				}	
			}
		}
		//1.2 main smps fault check
		if(myPs->board_fault[bd][1].useFlag == 1){
			//1.2.1 active high check
			if(myPs->board_fault[bd][1].active == 1){
				if((mainSmpsFail & myPs->board_fault[bd][1].bit) != 0){
					mainFlag = 1;
				}
			//1.2.2 active low check
			}else{
				if((mainSmpsFail & myPs->board_fault[bd][1].bit)
					!= myPs->board_fault[bd][1].bit){
					mainFlag = 1;
				}
			}
		}
		//1.3 control smps fault check
		if(myPs->board_fault[bd][2].useFlag != 0){
			//1.3.1 active high check
			if(myPs->board_fault[bd][2].active != 0){
				if((hwFail & myPs->board_fault[bd][2].bit) != 0){
					controlFlag = 1;
				}
			//1.3.2 active low check
			}else{
				if((hwFail & myPs->board_fault[bd][2].bit) 
					!= myPs->board_fault[bd][2].bit){
					controlFlag = 1;
				}
			}
		}
	}
	//2. ot fault
	if(otFlag == 1){
		if(myPs->signal[M_SIG_OT_FAULT] == P0) {
			myPs->signal[M_SIG_OT_FAULT] = P1;
			myPs->signal[M_SIG_RUN_LED] = P0;
			myPs->signal[M_SIG_REMOTE_SMPS1] = P0;
			myPs->signal[M_SIG_FAN_RELAY] = P0;
			myPs->signal[M_SIG_POWER_OFF] = P1;
			myPs->code = M_FAIL_OT;
			send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 6, (hwFail && 0x0f)) ;
			send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
				(int)myPs->code, hwFail && 0x0f);
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
	//3. main smps fault
	if(mainFlag == 1){
		if(myPs->signal[M_SIG_SMPS_FAULT] == P0 &&
		  myPs->signal[M_SIG_MAIN_MC_OFF] == P0){	//220214_hun 
		  //M_SIG_SHUTDOWN_DOOR_FAULT signal 은 다이슨 도어 오픈시 MC OFF 기능 때문에 추가 됨
			myPs->signal[M_SIG_SMPS_FAULT] = P1;
			myPs->signal[M_SIG_RUN_LED] = P0;
			myPs->signal[M_SIG_REMOTE_SMPS1] = P0;
			myPs->signal[M_SIG_FAN_RELAY] = P0;
			myPs->signal[M_SIG_POWER_OFF] = P1;
			myPs->code = M_FAIL_SMPS;
			send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 7, mainSmpsFail);
			send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
				(int)myPs->code, 1);
			for(bd=0; bd < myPs->config.installedBd; bd++) {
				for(ch=0; ch < myPs->config.chPerBd; ch++) {
					if((myPs->config.chPerBd * bd + ch)
						>= myPs->config.installedCh) continue;
					if(myData->bData[bd].cData[ch].op.state == C_RUN){
						if(myData->bData[bd].cData[ch]
							.signal[C_SIG_SMPS_FAULT] == P0) {
							send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
								(int)myPs->code, flag);
							myData->bData[bd].cData[ch]
								.signal[C_SIG_SMPS_FAULT] = P1;
						}
					}
				}
			}
		}
		return;
	}	
	//4. control smps fault
	if(controlFlag == 1){
		if(myPs->signal[M_SIG_SMPS_FAULT] == P0) {
			myPs->signal[M_SIG_SMPS_FAULT] = P1;
			myPs->signal[M_SIG_RUN_LED] = P0;
			myPs->signal[M_SIG_REMOTE_SMPS1] = P0;
			myPs->signal[M_SIG_FAN_RELAY] = P0;
			myPs->signal[M_SIG_POWER_OFF] = P1;
			myPs->code = M_FAIL_SMPS;
			send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 5, (hwFail ^ 0xf0));
			send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
				(int)myPs->code, 0);
			for(bd=0; bd < myPs->config.installedBd; bd++) {
				for(ch=0; ch < myPs->config.chPerBd; ch++) {
					if((myPs->config.chPerBd * bd + ch)
						>= myPs->config.installedCh) {
						continue;
					}
					if(myData->bData[bd].cData[ch].op.state == C_RUN){
						if(myData->bData[bd].cData[ch]
							.signal[C_SIG_SMPS_FAULT] == P0) {
							send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
								(int)myPs->code, flag);
							myData->bData[bd].cData[ch]
								.signal[C_SIG_SMPS_FAULT] = P1;
						}
					}
				}
			}
		}
		return;
	}
}
#endif

/*	date: 2015. 4.22	
 *	writer : lyh
 *	MAIN_REV11 - 7V, 3.3 SMPS fail Check
 *			   - OT fail Check 
*/	
#if 1
void ReadHardwareFault_Common_22(void)
{
	int addr, bd, ch;	
	int i,j=0, k=0, z = 0;
	unsigned short flag7v[8]={0}, flag3v[8]={0};
	unsigned short checkOT[4]={0}, check7v=0, check3v=0;
	unsigned char bitOT[4]={0}, bit7v[8]={0}, bit3v[8]={0};
	unsigned char activech[8]={0};
	static unsigned short pre7v[8]={0}, pre3v[8]={0};
	static unsigned short preOT[4]={0};
	static unsigned short sendFlag, sendFlag2, sendFlag3;
	unsigned short tmpFlag = 0, tmpFlag2 = 0, compareValue;
	unsigned short chPause = 0, chPause1[2] = {0}, chFlag = 0;
	int startCh, endCh;
		
	if(myData->dio.delayTimer < myData->dio.config.dioDelay) return;
	if(myData->AppControl.config.debugType == P1) return;
	if(myPs->signal[M_SIG_FAN_RELAY] == P0) return;
	
	// 1. OT Fault Check
	for(i=0; i<4; i++){
		addr = myPs->fault.ADDR_OT[i];
		if(addr == 0)
			continue;
		checkOT[i] = inb(addr);
		bitOT[i] = myPs->fault.BIT_OT[i];
		
		// 1-1. OT Fault Check event
		if((checkOT[i] & bitOT[i]) != bitOT[i]){
			if(myPs->signal[M_SIG_OT_FAULT] == P0 || preOT[i] != checkOT[i]) {
				preOT[i] = checkOT[i];
				chFlag = 1;		//20170530 sch add
		
				//150806 lyh add for Ot Fault
				tmpFlag = ~checkOT[i] & bitOT[i];
				if(myPs->config.installedBd > 1){
					tmpFlag2 = tmpFlag & 0xF0;
					tmpFlag = tmpFlag & 0x0F;

					switch(tmpFlag){
						case 0x01:
						case 0x02:
						case 0x03:
							tmpFlag = 0x01;
							break;
						case 0x04:
						case 0x08:
						case 0x0C:
							tmpFlag = 0x02;
							break;
						case 0x05:
						case 0x06:
						case 0x07:
						case 0x0A:
						case 0x0B:
						case 0x0D:
						case 0x0E:
						case 0x0F:
							tmpFlag = 0x03;
							break;
						default :
							tmpFlag = 0x00;
							break;
					}
					switch(tmpFlag2){
						case 0x10:
						case 0x20:
						case 0x30:
							tmpFlag2 = 0x04;
							break;
						case 0x40:
						case 0x80:
						case 0xC0:
							tmpFlag2 = 0x08;
							break;
						case 0x50:
						case 0x60:
						case 0x70:
						case 0x90:
						case 0xA0:
						case 0xB0:
						case 0xD0:
						case 0xE0:
						case 0xF0:
							tmpFlag2 = 0x0C;
							break;
						default :
							tmpFlag2 = 0x00;
							break;
					}
					tmpFlag = tmpFlag | tmpFlag2;
					//20170530 sch modify
					if(myPs->config.maxCurrent[0] < 20000000){
							sendFlag = 0x01 << i;
					}else{
						sendFlag = sendFlag & ~(0x0f << (4*i));
						sendFlag = sendFlag | (tmpFlag << (4*i));
					}
				}else{
					sendFlag = sendFlag & ~(bitOT[i] << (8*i));
					sendFlag = sendFlag | (tmpFlag << (8*i));
				}
				myPs->code = M_FAIL_OT;
				send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
					(int)myPs->code, sendFlag);
			}
			//150810 OT Ch Pause
			//20170530 sch modify
			if(myData->mData.config.function[F_OT_PAUSE] == P1){
				chPause = ~checkOT[i] & bitOT[i];
				if(i < 2){
					chPause1[0] = chPause << (8*i);
				}else{
					chPause1[1] = chPause << (8*(i-2));
				}
			}
			//1-2. OT fault Halt
			if(myData->mData.config.function[F_OT_PAUSE] == P0){
				if((inb(0x602) & 0x10) == P0) {
					myPs->signal[M_SIG_RUN_LED] = P0;
					myPs->signal[M_SIG_REMOTE_SMPS1] = P0;			
					myPs->signal[M_SIG_POWER_OFF] = P1;
					myPs->signal[M_SIG_FAN_RELAY] = P0;
					send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 6, sendFlag);
				}
				myPs->signal[M_SIG_OT_FAULT] = P1;
				myPs->signal[M_SIG_OT_FAULT1] = P1;
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
				bd = 0;
			}else{ //1-3. 150811 lyh add for OT Fault Ch Pause
				myPs->signal[M_SIG_OT_FAULT] = P1;
				myPs->signal[M_SIG_OT_FAULT1] = P1;
				//20170530 sch modify
				for(j= 0; j < myPs->config.installedCh; j++){
					bd = j / myPs->config.chPerBd;
					ch = j % myPs->config.chPerBd;
					compareValue = 0x01 << ch ;
					if((compareValue & chPause1[bd]) == compareValue){ 
						if(myPs->config.installedBd > 1){
							for(z = 0; z < 2; z++){
								if(myData->bData[bd].cData[ch+z].op.state 
									== C_RUN && myData->bData[bd].cData[ch+z]
										.signal[C_SIG_OT_FAULT] == P0) {
										myData->bData[bd].cData[ch+z]
										.signal[C_SIG_OT_FAULT] = P1;
								}
							}
						}else{
							if(myData->bData[bd].cData[ch].op.state 
								== C_RUN && myData->bData[bd].cData[ch]
									.signal[C_SIG_OT_FAULT] == P0) {
									myData->bData[bd].cData[ch]
									.signal[C_SIG_OT_FAULT] = P1;
							}
						}
						if(chFlag == 1)
							send_msg(MODULE_TO_APP, 
										MSG_MODULE_APP_EMG, 28, j+1);
					}
				}
				j = 0;
				bd = 0;
			}
		}else{
			preOT[i] = checkOT[i];
			chFlag = 0;

			if(myPs->config.installedBd > 1){
				sendFlag = sendFlag & ~(0x0f << (4*i));
			}else{
				sendFlag = sendFlag & ~(bitOT[i] << (8*i));
			}
		}
	}
	//2. 7V Smps Fault Check
	
	for(i=0; i<8; i++){
		addr = myPs->fault.PS_ADDR_7V[i];
		if(addr == 0)
			continue;
		flag7v[i] = inb(addr);
		activech[i] = myPs->fault.PS_ACTIVECH_7V[i];
		bit7v[i] = myPs->fault.PS_BIT_7V[i];
		
		//2-1. PS BIT Not use 
		if(pre7v[i] != 0){
			flag7v[j] = flag7v[i];
			activech[j] = activech[i];
			bit7v[j] = bit7v[i];
			j++;
		}
	
	}
	//2-2. 7V Smps Fault event 	
	for(i=0; i<8; i++){
		addr = myPs->fault.PS_ADDR_7V[i];
		bit7v[i] = myPs->fault.PS_BIT_7V[i];
		if(addr == 0 || bit7v[i] == 0)
			continue;
		if((flag7v[i] & bit7v[i]) != 0){
			if(myPs->signal[M_SIG_SMPS_FAULT] == P0 || pre7v[i] != flag7v[i]){
				pre7v[i] = flag7v[i];	
				sendFlag2 |= 0x01 << i;
				myPs->code = M_FAIL_SMPS;
				send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 18, sendFlag2);
				send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
							(int)myPs->code, sendFlag2);
			}
			myPs->signal[M_SIG_SMPS_FAULT] = P1;
			check7v = i;
			
			//2-3. 7V Fault Ch serch
			startCh = 0;
			endCh = 0;
			bd = 0;

			if(check7v == 0){
				startCh = 0;
				endCh = activech[0];
			}else{
				for(j=1; j<=check7v ; j++){
					startCh += activech[i-1];
	 				endCh = activech[i] + startCh;
				}	
			}
			//2-4. 7V Smps Fault event service routine
			for(k = startCh; k< endCh; k++){
				if(startCh >= myPs->config.chPerBd){
					bd = startCh/myPs->config.chPerBd;
					ch = k - (myPs->config.chPerBd * bd) ;
				}else{
					bd = 0;
					ch = k;
				}
				if((myPs->config.chPerBd * bd + ch)
					> (myPs->config.installedCh)){
					continue;
				}
				if(myData->bData[bd].cData[ch].op.state == C_RUN
					&& myData->bData[bd].cData[ch]
					.signal[C_SIG_SMPS_FAULT] == P0){
					myData->bData[bd].cData[ch]
						.signal[C_SIG_SMPS_FAULT] = P1;
				}	
			}
		}else{
			sendFlag2 &= ~(0x01 << i);
			pre7v[i] = flag7v[i];
		}
	}
	
	//3. 3.3V Smps Fault Check
	for(i=0; i<8; i++){
		addr = myPs->fault.PS_ADDR_3V[i];
		if(addr == 0)
			continue;
		flag3v[i] = inb(addr);
		activech[i] = myPs->fault.PS_ACTIVECH_3V[i];
		bit3v[i] = myPs->fault.PS_BIT_3V[i];
		
		//3-1. PS BIT Not use 
		if(pre3v[i] != 0){
			flag3v[j] = flag3v[i];
			activech[j] = activech[i];
			bit3v[j] = bit3v[i];
			j++;
		}
	
	}
	//3-2. 3V Smps Fault event 	
	for(i=0; i<8; i++){
		addr = myPs->fault.PS_ADDR_3V[i];
		bit3v[i] = myPs->fault.PS_BIT_3V[i];
		if(addr == 0 || bit3v[i] == 0)
			continue;
		if((flag3v[i] & bit3v[i]) != 0){
			if(myPs->signal[M_SIG_SMPS_FAULT1] == P0 || pre3v[i] != flag3v[i]){
				pre3v[i] = flag3v[i];	
				sendFlag3 |= 0x0100 << i;
				myPs->code = M_FAIL_SMPS;
				send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 19, sendFlag3);
				send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
							(int)myPs->code, sendFlag3);
			}
			myPs->signal[M_SIG_SMPS_FAULT1] = P1;
			check3v = i;
			
			//3-3. 7V Fault Ch serch
			startCh = 0;
			endCh = 0;
			bd = 0;

			if(check3v == 0){
				startCh = 0;
				endCh = activech[0];
			}else{
				for(j=1; j<=check3v ; j++){
					startCh += activech[i-1];
	 				endCh = activech[i] + startCh;
				}	
			}
			//3-4. 3V Smps Fault event service routine
			for(k = startCh; k< endCh; k++){
				if(startCh >= myPs->config.chPerBd){
					bd = startCh/myPs->config.chPerBd;
					ch = k - (myPs->config.chPerBd * bd) ;
				}else{
					bd = 0;
					ch = k;
				}
				if((myPs->config.chPerBd * bd + ch)
					> (myPs->config.installedCh)){
					continue;
				}
				if(myData->bData[bd].cData[ch].op.state == C_RUN
					&& myData->bData[bd].cData[ch]
					.signal[C_SIG_SMPS_FAULT] == P0){
					myData->bData[bd].cData[ch]
						.signal[C_SIG_SMPS_FAULT] = P1;
				}	
			}
		}else{
			sendFlag3 &= ~(0x0100 << i);
			pre3v[i] = flag3v[i];
		}
	}
					
}

#endif


/*	L_NEW
	141216 oys Modify : MAX 32CH Fault Check
 
#if 1
void ReadHardwareFault_Common_22(void)
{
	int bd, ch;
	int	addr, base_addr;
	unsigned short flag, flag1, flag2, flag3, sendFlag = 0;
	unsigned char ot, smps, compareValue1, compareValue2;
	static unsigned char preSMPS[2]={0},preOT[4]={0};
	unsigned char otFlag[4] = {0};
	int chPerSMPS;
	int smpsCount;
	int fault7Flag[8] = {0};
	int max, i, j, k, type;
	int start;
	int bd_step, hw_rd;

	if(myData->dio.delayTimer < myData->dio.config.dioDelay) return;
	if(myPs->signal[M_SIG_FAN_RELAY] == P0) return;

	base_addr = myPs->addr.main[BASE_ADDR];
	bd_step = myPs->addr.main[ADDR_STEP];
	hw_rd = myPs->addr.main[PS_FAIL_CS];
	//1. fault check	
	for(bd = 0; bd < myPs->config.installedBd; bd++){	
		if(bd == 0){
			//1-1.OT check (1 - 8ch)
			addr = base_addr + bd_step * bd + hw_rd + 2;	
			flag = inb(addr);
			ot = myPs->fault.OT[0];
	
			if((flag & ot) != ot) { //OT fault
				if(myPs->signal[M_SIG_OT_FAULT] == P0 || preOT[0] != flag ) {
					preOT[0] = flag;
					//100908 oys OT signal send
				//	sendFlag = ~flag & ot;
					sendFlag = 0x01;
					otFlag[0] = 1;
				}
			//	return;
			} else {
				myPs->signal[M_SIG_OT_FAULT] = P0;
				preOT[0] = flag;
				otFlag[0] = 0;
			}
			//1-2.OT check (9 - 16ch)
			addr = base_addr + bd_step * bd + hw_rd + 5;
			flag = inb(addr);
			ot = myPs->fault.OT[1];
			if((flag & ot) != ot) { //OT fault
				if(myPs->signal[M_SIG_OT_FAULT1] == P0 || preOT[1] != flag) {
					preOT[1] = flag;
					//100908 oys OT signal send
				//	sendFlag |= (~flag & ot) << 8;
					sendFlag = 0x02;
					otFlag[1] = 1;
				}
				//	return;
			} else {
				myPs->signal[M_SIG_OT_FAULT1] = P0;
				preOT[1] = flag;
				otFlag[1] = 0;
			}
		}
		if(bd == 1){
			//1-3.OT check (17 - 24ch)
			addr = base_addr + bd_step * bd + hw_rd + 2;	
			flag = inb(addr);
			ot = myPs->fault.OT[2];
	
			if((flag & ot) != ot) { //OT fault
				if(myPs->signal[M_SIG_OT_FAULT] == P0 || preOT[2] != flag ) {
					preOT[2] = flag;
					//100908 oys OT signal send
				//	sendFlag = ~flag & ot;
					sendFlag = 0x04;
					otFlag[2] = 1;
				}
			//	return;
			} else {
				myPs->signal[M_SIG_OT_FAULT] = P0;
				preOT[2] = flag;
				otFlag[2] = 0;
			}
			//1-4.OT check (25 - 32ch)
			addr = base_addr + bd_step * bd + hw_rd + 5;
			flag = inb(addr);
			ot = myPs->fault.OT[3];
			if((flag & ot) != ot) { //OT fault
				if(myPs->signal[M_SIG_OT_FAULT1] == P0 || preOT[3] != flag) {
					preOT[3] = flag;
					//100908 oys OT signal send
				//	sendFlag |= (~flag & ot) << 8;
					sendFlag = 0x08;
					otFlag[3] = 1;
				}
				//	return;
			} else {
				myPs->signal[M_SIG_OT_FAULT1] = P0;
				preOT[3] = flag;
				otFlag[3] = 0;
			}
		}
	}
	if(myPs->code == M_FAIL_OT){
		for(i=0; i<4; i++){
			otFlag[i] = 0;
		}
	}
	//2. OT fault event service routine
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
		bd = 0;
	}
	//3. 7V Main SMPS Fault check
	bd = 0;
	flag = 0;
	flag1 = 0;
	flag2 = 0;
	flag3 = 0;
	if(myPs->config.installedCh > 8){	
		addr = base_addr + bd_step * bd + hw_rd;
		flag = inb(addr) & 0x03;

//		addr = base_addr + bd_step * bd + hw_rd + 3;
//		flag1 = inb(addr) & 0x03;
	} else {
		addr = base_addr + bd_step * bd + hw_rd;
		flag = inb(addr);
	}
	if(myPs->config.installedBd == 2){
		addr = base_addr + bd_step * 1 + hw_rd;
		flag2 = inb(addr) & 0x03;

//		addr = base_addr + bd_step * 1 + hw_rd + 3;
//		flag3 = inb(addr) & 0x03;
	}
	//lyh w 140220 for MAX 32ch	
	flag = (flag + (flag1 << 2) + (flag2 << 2) + (flag3 << 2));
	

	//oys modify
	if(myPs->config.installedBd == 2){
		smps = myPs->fault.SMPS7_5V[0] + (myPs->fault.SMPS7_5V[1] << 2);
	}else{
		smps = myPs->fault.SMPS7_5V[0];
	}

	// 3-1. 7V smps fail
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
	//3-2 나누어 떨어질때(new)
		if(type == 0){
			if(myPs->signal[M_SIG_SMPS_FAULT] == P0
					|| preSMPS[0] != flag) {
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
			chPerSMPS = (myPs->config.installedCh) / smpsCount;
			
			for(i = 0; i < 8; i++){
				if(fault7Flag[i] == 1){
					start = chPerSMPS * i;
					max = chPerSMPS * (i+1);
					bd = (i * chPerSMPS) / (myPs->config.chPerBd);
					ch = (i*chPerSMPS)%(myPs->config.chPerBd);
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
						}
					k++;	
						}
				}
			}
	// 3-3 나누어 떨어지지 않을 때.
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
		bd = 0;
	} else {
		myPs->signal[M_SIG_SMPS_FAULT] = P0;	//7VSmps(new)
	}
	//4. 3.3V Main SMPS Fault check
	bd = 0;
	flag = 0;
	flag1 = 0;
	flag2 = 0;
	flag3 = 0;

	if(myPs->config.installedCh > 8){	
		addr = base_addr + bd_step * bd + hw_rd + 1;
		flag = inb(addr) & 0x03;

//		addr = base_addr + bd_step * bd + hw_rd + 4;
//		flag1 = inb(addr) & 0x03;
	}else{
		addr = base_addr + bd_step * bd + hw_rd + 1;
		flag = inb(addr);
	}
	if(myPs->config.installedBd == 2){
		addr = base_addr + bd_step * 1 + hw_rd + 1;
		flag2 = inb(addr) & 0x03;

//		addr = base_addr + bd_step * 1 + hw_rd + 4;
//		flag3 = inb(addr) & 0x03;
	}
	//lyh w 140220 for MAX 32ch	
	flag = (flag + (flag1 << 2) + (flag2 << 2) + (flag3 << 2));
	

	//oys modify
	if(myPs->config.installedBd == 2){
		smps = myPs->fault.SMPS3_3V[0] + (myPs->fault.SMPS3_3V[1] << 2);
	}else{
		smps = myPs->fault.SMPS3_3V[0];
	}
	// 4-1. 3.3V smps fail
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
	// 4-2 나누어 떨어질때(new)
		if( type == 0){	
			if(myPs->signal[M_SIG_SMPS_FAULT1] == P0 || preSMPS[1] != flag) {						
				preSMPS[1] = flag;
				sendFlag = (flag & smps) << 8;
				myPs->code = M_FAIL_SMPS;
				send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
						(int)myPs->code, sendFlag);
			}
//			myPs->code = M_FAIL_SMPS;
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
			chPerSMPS = (myPs->config.installedCh) / smpsCount;
			
			for(i = 0; i < 8; i++){
				if(fault7Flag[i] == 1){
					start = chPerSMPS * i;
					max = chPerSMPS * (i+1);
					bd = (i * chPerSMPS) / (myPs->config.chPerBd);
					ch = (i*chPerSMPS)%(myPs->config.chPerBd);
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
						}
					k++;	
						}
				}
			}
	// 4-3 나누어 떨어지지 않을때
		}else{		     
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
					bd = 0;
				}	
				
			} else {
				myPs->signal[M_SIG_SMPS_FAULT1] = P0;
				preSMPS[1] = flag;
			}
		}
		bd = 0;
	} else {
		myPs->signal[M_SIG_SMPS_FAULT1] = P0;	//3.3V SMPS Fail
	}
}
#endif
*/

/*	date: 2011. 7. 6					writer : pjy
 *	discription:
 *	1.checkFault(until installedBd)
 *		1.1 OT Check
 *		1.2 Main SMPS Fault Check
 *		1.3 Control SMPS Check
 *	2.OT fault service sequence
 *	3.Main SMPS fault service sequence
 *	4.Control SMPS fault service sequence
 */ 
void ReadHardwareFault_Common_23(void)
{
	int bd, ch, i;
	int	addr, base_addr, hw_rd, bd_step;
	unsigned char flag=0x00;
	int otFlag = 0;
	int mainFlag = 0;
	int controlFlag = 0;
	static unsigned char preSMPS[5]={0};

	if(myData->dio.delayTimer < myData->dio.config.dioDelay) return;
	if(myPs->signal[M_SIG_FAN_RELAY] == P0) return;

	base_addr = myPs->addr.main[BASE_ADDR];
	bd_step = myPs->addr.main[ADDR_STEP];
	hw_rd = myPs->addr.main[HW_RD];
	//1. check fault
	for(bd = 0; bd < myData->mData.config.installedBd; bd++){
		addr = base_addr + bd_step * bd + hw_rd;
		flag = inb(addr);
		//1.1 ot check(if the use flag define 1)
		if(myPs->board_fault[bd][0].useFlag != 0){
			//1.1.1 active high check
		   	if(myPs->board_fault[bd][0].active != 0){
				if((flag & myPs->board_fault[bd][0].bit) != 0){
					otFlag = 1;
				}
			//1.1.2 active low check
			}else{
				if((flag & myPs->board_fault[bd][0].bit) 
					!= myPs->board_fault[bd][0].bit){
					otFlag = 1;
				}	
			}
		}
	//1.2 main smps fault check
	if(myPs->board_fault[bd][1].useFlag == 1){
		//1.2.1 active high check
		if(myPs->board_fault[bd][1].active == 1){
			if(bd == 0){
				if((flag & 0xa) == 0xa){
					if(myPs->signal[M_SIG_SMPS_FAULT] == P0 && preSMPS[0] != flag){
						
						myPs->code = M_FAIL_SMPS;
						send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
							(int)myPs->code, 12);
						myPs->signal[M_SIG_SMPS_FAULT] = P1;
					}
					preSMPS[0] = flag;

					for(i=0; i<32; i++){
						if(myData->bData[bd].cData[i].op.state == C_RUN
							&& myData->bData[bd].cData[i]
							.signal[C_SIG_SMPS_FAULT] == P0){
								myData->bData[bd].cData[i]
							.signal[C_SIG_SMPS_FAULT] = P1;
						}
					}
				}else{
					myPs->signal[M_SIG_SMPS_FAULT] = P0;
					//20170615 sch add
					preSMPS[0] = flag;
				}
				if((flag & 0xa) == 0x2){
					if(myPs->signal[M_SIG_SMPS_FAULT] == P0 && preSMPS[1] != flag){
						
						myPs->code = M_FAIL_SMPS;
						send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
							(int)myPs->code, 1);
						myPs->signal[M_SIG_SMPS_FAULT] = P1;
					}
					preSMPS[1] = flag;

					for(i=0; i<16; i++){
						if(myData->bData[bd].cData[i].op.state == C_RUN
							&& myData->bData[bd].cData[i]
							.signal[C_SIG_SMPS_FAULT] == P0){
								myData->bData[bd].cData[i]
							.signal[C_SIG_SMPS_FAULT] = P1;
						}
					}
				}else{
					myPs->signal[M_SIG_SMPS_FAULT] = P0;
					//20170615 sch add
					preSMPS[1] = flag;
				}
				if((flag & 0xa) == 0x8){
					if(myPs->signal[M_SIG_SMPS_FAULT] == P0 && preSMPS[2] != flag){
						myPs->code = M_FAIL_SMPS;
						send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
							(int)myPs->code, 2);
						myPs->signal[M_SIG_SMPS_FAULT] = P1;
					}
					preSMPS[2] = flag;

					for(i=16; i<32; i++){
						if(myData->bData[bd].cData[i].op.state == C_RUN
							&& myData->bData[bd].cData[i]
							.signal[C_SIG_SMPS_FAULT] == P0){
								myData->bData[bd].cData[i]
							.signal[C_SIG_SMPS_FAULT] = P1;
						}
					}
				}else{
					myPs->signal[M_SIG_SMPS_FAULT] = P0;
					//20170615 sch add
					preSMPS[2] = flag;
				}
			}
			else if(bd ==1){
				if((flag & 0xa) == 0xa){
					if(myPs->signal[M_SIG_SMPS_FAULT] == P0 && preSMPS[3] != flag){
						myPs->code = M_FAIL_SMPS;
						send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
							(int)myPs->code, 34);
						myPs->signal[M_SIG_SMPS_FAULT] = P1;
					}
					preSMPS[3] = flag;

					for(i=0; i<32; i++){
						if(myData->bData[bd].cData[i].op.state == C_RUN
							&& myData->bData[bd].cData[i]
							.signal[C_SIG_SMPS_FAULT] == P0){
								myData->bData[bd].cData[i]
							.signal[C_SIG_SMPS_FAULT] = P1;
						}
					}
				}else{
					myPs->signal[M_SIG_SMPS_FAULT] = P0;
					//20170615 sch add
					preSMPS[3] = flag;
				}
				if((flag & 0xa) == 0x2){
					if(myPs->signal[M_SIG_SMPS_FAULT] == P0 && preSMPS[4] != flag){
						myPs->code = M_FAIL_SMPS;
						send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
							(int)myPs->code, 3);
						myPs->signal[M_SIG_SMPS_FAULT] = P1;
					}
					preSMPS[4] = flag;

					for(i=0; i<16; i++){
						if(myData->bData[bd].cData[i].op.state == C_RUN
							&& myData->bData[bd].cData[i]
							.signal[C_SIG_SMPS_FAULT] == P0){
								myData->bData[bd].cData[i]
							.signal[C_SIG_SMPS_FAULT] = P1;
						}
					}
				}else{
					myPs->signal[M_SIG_SMPS_FAULT] = P0;
					//20170615 sch add
					preSMPS[4] = flag;
				}
				if((flag & 0xa) == 0x8){
					if(myPs->signal[M_SIG_SMPS_FAULT] == P0 && preSMPS[5] != flag){
						myPs->code = M_FAIL_SMPS;
						send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
							(int)myPs->code, 4);
						myPs->signal[M_SIG_SMPS_FAULT] = P1;
					}
					preSMPS[5] = flag;

					for(i=16; i<32; i++){
						if(myData->bData[bd].cData[i].op.state == C_RUN
							&& myData->bData[bd].cData[i]
							.signal[C_SIG_SMPS_FAULT] == P0){
								myData->bData[bd].cData[i]
							.signal[C_SIG_SMPS_FAULT] = P1;
						}
					}
				}else{
					myPs->signal[M_SIG_SMPS_FAULT] = P0;
					//20170615 sch add
					preSMPS[6] = flag;
				}
			}
			//1.2.2 active low check
			}else{
			if(bd == 0){
				if((flag & 0xa) == 0){
					if(myPs->signal[M_SIG_SMPS_FAULT] == P0 && preSMPS[0] != flag){
						
						myPs->code = M_FAIL_SMPS;
						send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
							(int)myPs->code, 12);
						myPs->signal[M_SIG_SMPS_FAULT] = P1;
					}
					preSMPS[0] = flag;

					for(i=0; i<32; i++){
						if(myData->bData[bd].cData[i].op.state == C_RUN
							&& myData->bData[bd].cData[i]
							.signal[C_SIG_SMPS_FAULT] == P0){
								myData->bData[bd].cData[i]
							.signal[C_SIG_SMPS_FAULT] = P1;
						}
					}
				}else{
					myPs->signal[M_SIG_SMPS_FAULT] = P0;
					//20170615 sch add
					preSMPS[0] = flag;
				}
				if((flag & 0xa) == 0x8){
					if(myPs->signal[M_SIG_SMPS_FAULT] == P0 && preSMPS[1] != flag){
						
						myPs->code = M_FAIL_SMPS;
						send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
							(int)myPs->code, 1);
						myPs->signal[M_SIG_SMPS_FAULT] = P1;
					}
					preSMPS[1] = flag;

					for(i=0; i<16; i++){
						if(myData->bData[bd].cData[i].op.state == C_RUN
							&& myData->bData[bd].cData[i]
							.signal[C_SIG_SMPS_FAULT] == P0){
								myData->bData[bd].cData[i]
							.signal[C_SIG_SMPS_FAULT] = P1;
						}
					}
				}else{
					myPs->signal[M_SIG_SMPS_FAULT] = P0;
					//20170615 sch add
					preSMPS[1] = flag;
				}
				if((flag & 0xa) == 0x2){
					if(myPs->signal[M_SIG_SMPS_FAULT] == P0 && preSMPS[2] != flag){
						myPs->code = M_FAIL_SMPS;
						send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
							(int)myPs->code, 2);
						myPs->signal[M_SIG_SMPS_FAULT] = P1;
					}
					preSMPS[2] = flag;

					for(i=16; i<32; i++){
						if(myData->bData[bd].cData[i].op.state == C_RUN
							&& myData->bData[bd].cData[i]
							.signal[C_SIG_SMPS_FAULT] == P0){
								myData->bData[bd].cData[i]
							.signal[C_SIG_SMPS_FAULT] = P1;
						}
					}
				}else{
					myPs->signal[M_SIG_SMPS_FAULT] = P0;
					//20170615 sch add
					preSMPS[2] = flag;
				}
			}
			else if(bd ==1){
				if((flag & 0xa) == 0){
					if(myPs->signal[M_SIG_SMPS_FAULT] == P0 && preSMPS[3] != flag){
						myPs->code = M_FAIL_SMPS;
						send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
							(int)myPs->code, 34);
						myPs->signal[M_SIG_SMPS_FAULT] = P1;
					}
					preSMPS[3] = flag;

					for(i=0; i<32; i++){
						if(myData->bData[bd].cData[i].op.state == C_RUN
							&& myData->bData[bd].cData[i]
							.signal[C_SIG_SMPS_FAULT] == P0){
								myData->bData[bd].cData[i]
							.signal[C_SIG_SMPS_FAULT] = P1;
						}
					}
				}else{
					myPs->signal[M_SIG_SMPS_FAULT] = P0;
					//20170615 sch add
					preSMPS[3] = flag;
				}
				if((flag & 0xa) == 0x8){
					if(myPs->signal[M_SIG_SMPS_FAULT] == P0 && preSMPS[4] != flag){
						myPs->code = M_FAIL_SMPS;
						send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
							(int)myPs->code, 3);
						myPs->signal[M_SIG_SMPS_FAULT] = P1;
					}
					preSMPS[4] = flag;

					for(i=0; i<16; i++){
						if(myData->bData[bd].cData[i].op.state == C_RUN
							&& myData->bData[bd].cData[i]
							.signal[C_SIG_SMPS_FAULT] == P0){
								myData->bData[bd].cData[i]
							.signal[C_SIG_SMPS_FAULT] = P1;
						}
					}
				}else{
					myPs->signal[M_SIG_SMPS_FAULT] = P0;
					//20170615 sch add
					preSMPS[4] = flag;
				}
				if((flag & 0xa) == 0x2){
					if(myPs->signal[M_SIG_SMPS_FAULT] == P0 && preSMPS[5] != flag){
						myPs->code = M_FAIL_SMPS;
						send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
							(int)myPs->code, 4);
						myPs->signal[M_SIG_SMPS_FAULT] = P1;
					}
					preSMPS[5] = flag;

					for(i=16; i<32; i++){
						if(myData->bData[bd].cData[i].op.state == C_RUN
							&& myData->bData[bd].cData[i]
							.signal[C_SIG_SMPS_FAULT] == P0){
								myData->bData[bd].cData[i]
							.signal[C_SIG_SMPS_FAULT] = P1;
						}
					}
				}else{
					myPs->signal[M_SIG_SMPS_FAULT] = P0;
					//20170615 sch add
					preSMPS[5] = flag;
				}
			}
		}		
	}

		 //1.3 control smps fault check
		if(myPs->board_fault[bd][2].useFlag != 0){
			//1.3.1 active high check
			if(myPs->board_fault[bd][2].active != 0){
				if((flag & myPs->board_fault[bd][2].bit) != 0){
					controlFlag = 1;
				}
			//1.3.2 active low check
			}else{
				if((flag & myPs->board_fault[bd][2].bit) 
					!= myPs->board_fault[bd][2].bit){
					controlFlag = 1;
				}
			}
		}
	}
	//2. ot fault
	if(otFlag == 1){
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
	//3. main smps fault
	if(mainFlag == 1){
		if(myPs->signal[M_SIG_SMPS_FAULT] == P0){
			myPs->signal[M_SIG_SMPS_FAULT] = P1;
			myPs->signal[M_SIG_RUN_LED] = P0;
			myPs->signal[M_SIG_REMOTE_SMPS1] = P0;
			myPs->signal[M_SIG_FAN_RELAY] = P0;
			myPs->signal[M_SIG_POWER_OFF] = P1;
			myPs->code = M_FAIL_SMPS;
			send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 7, flag);
			send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
				(int)myPs->code, 1);
			for(bd=0; bd < myPs->config.installedBd; bd++) {
				for(ch=0; ch < myPs->config.chPerBd; ch++) {
					if((myPs->config.chPerBd * bd + ch)
						>= myPs->config.installedCh) continue;
					if(myData->bData[bd].cData[ch].op.state == C_RUN){
						if(myData->bData[bd].cData[ch]
							.signal[C_SIG_SMPS_FAULT] == P0) {
							send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
								(int)myPs->code, flag);
							myData->bData[bd].cData[ch]
								.signal[C_SIG_SMPS_FAULT] = P1;
						}
					}
				}
			}
		}
		return;
	}	
	//4. control smps fault
	if(controlFlag == 1){
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
						>= myPs->config.installedCh) {
						continue;
					}
					if(myData->bData[bd].cData[ch].op.state == C_RUN){
						if(myData->bData[bd].cData[ch]
							.signal[C_SIG_SMPS_FAULT] == P0) {
							send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
								(int)myPs->code, flag);
							myData->bData[bd].cData[ch]
								.signal[C_SIG_SMPS_FAULT] = P1;
						}
					}
				}
			}
		}
		return;
	}
}

void WriteHardwareFault_Common_1(void)
{
	int rtn=0, bd;
	int addr, base_addr, hw_wr, ps_rem_cs;

	for(bd=0; bd < myPs->config.installedBd; bd++) {
		if(myData->bData[bd].signal[B_SIG_HW_FAULT_CLEAR] == P1) {
			myData->bData[bd].signal[B_SIG_HW_FAULT_CLEAR] = P0;
			rtn = 1;
		}
	}

	base_addr = myPs->addr.main[BASE_ADDR];
	hw_wr = myPs->addr.main[HW_WR];
	ps_rem_cs = myPs->addr.main[PS_REM_CS];

	addr = base_addr + hw_wr;
	switch((int)myPs->config.hwSpec){
   		case L_5V_30A_R1:
			break;
		default:
			if(rtn != 0) {
				outb(0x01, addr);
				outb(0x00, addr);
			}
			break;
	}

	addr = base_addr + ps_rem_cs;
	switch((int)myPs->config.hwSpec){
		case L_5V_50A:
		case L_50V_50A:
		case L_5V_100A: 
		case L_5V_20A:
		case L_5V_30A_R1:
		case L_5V_50A_R2:
		case L_5V_65A_R3:
		case L_5V_50A_R2_1:
			if(myPs->signal[M_SIG_REMOTE_SMPS1] == P1) {
				outb(0xFF, addr);
			} else {
				outb(0x00, addr);
			}
			break;
	}
}

void PS_Remote_1(void)
{
	int bd;
	int addr, base_addr, addr_step, ps_rem_cs;

	base_addr = myPs->addr.main[BASE_ADDR];
	addr_step = myPs->addr.main[ADDR_STEP];
	ps_rem_cs = myPs->addr.main[PS_REM_CS];

	for(bd=0; bd < myPs->config.installedBd; bd++) {
		addr = base_addr + addr_step * bd + ps_rem_cs;
		switch((int)myPs->config.hwSpec){
			case L_5V_2A_R1:
			case L_5V_50A_R2_P:
				if(myPs->signal[M_SIG_REMOTE_SMPS1] == P1) {
					outb(0xFF, addr);
				} else {
					outb(0x00, addr);
				}
				break;
		}
	}
}
