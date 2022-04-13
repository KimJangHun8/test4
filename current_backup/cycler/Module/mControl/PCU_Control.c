#include <asm/io.h>
#include <math.h>
#include <rtl_core.h>
#include <pthread.h>
#include "../../INC/datastore.h"
#include "message.h"
#include "ChannelControl.h"
#include "PCU_Control.h"
#include "local_utils.h"
#include "PCU.h"
#include "Analog.h"
#include "FaultCond.h"

extern S_SYSTEM_DATA *myData;
extern S_MODULE_DATA *myPs;    
extern S_TEST_CONDITION	*myTestCond;	//190901 lyhw
extern S_CH_DATA *myCh;

int shiftTable_64Ch[64][2] = {
	{ 0, 0}, { 0, 2}, { 0, 4}, { 0, 6}, { 0, 1}, { 0, 3}, { 0, 5}, { 0, 7},
	{ 0, 8}, { 0, 10}, { 0, 12}, { 0, 14}, { 0, 9}, { 0, 11}, { 0, 13}, { 0, 15},
	{ 0, 16}, { 0, 18}, { 0, 20}, { 0, 22}, { 0, 17}, { 0, 19}, { 0, 21}, { 0, 23},
	{ 0, 24}, { 0, 26}, { 0, 28}, { 0, 30}, { 0, 25}, { 0, 27}, { 0, 29}, { 0, 31},
	{ 0, 32}, { 0, 34}, { 0, 36}, { 0, 38}, { 0, 33}, { 0, 35}, { 0, 37}, { 0, 39},
	{ 0, 40}, { 0, 42}, { 0, 44}, { 0, 46}, { 0, 41}, { 0, 43}, { 0, 45}, { 0, 47},
	{ 0, 48}, { 0, 50}, { 0, 52}, { 0, 54}, { 0, 49}, { 0, 51}, { 0, 53}, { 0, 55},
	{ 0, 56}, { 0, 58}, { 0, 60}, { 0, 62}, { 0, 57}, { 0, 59}, { 0, 61}, { 0, 63},
};	//200407 lyhw add for 64ch

int shiftTable_32Ch[32][2] = {
	{ 0, 0}, { 0, 2}, { 0, 4}, { 0, 6}, { 0, 1}, { 0, 3}, { 0, 5}, { 0, 7},
	{ 0, 8}, { 0, 10}, { 0, 12}, { 0, 14}, { 0, 9}, { 0, 11}, { 0, 13}, { 0, 15},
	{ 0, 16}, { 0, 18}, { 0, 20}, { 0, 22}, { 0, 17}, { 0, 19}, { 0, 21}, { 0, 23},
	{ 0, 24}, { 0, 26}, { 0, 28}, { 0, 30}, { 0, 25}, { 0, 27}, { 0, 29}, { 0, 31},
};

int shiftTable_24[24][2] = {
	{ 0, 0}, { 0, 2}, { 0, 4}, { 0, 6}, { 0, 1}, { 0, 3}, { 0, 5}, { 0, 7},
	{ 0, 8}, { 0, 10}, { 0, 12}, { 0, 14}, { 0, 9}, { 0, 11}, { 0, 13}, { 0, 15},
	{ 0, 16}, { 0, 18}, { 0, 20}, { 0, 22}, { 0, 17}, { 0, 19}, { 0, 21}, { 0, 23},
};

int shiftTable_16Ch[16][2] = {
	{ 0, 0}, { 0, 8}, { 0, 2}, { 0,10}, { 0, 4}, { 0, 12}, { 0, 6}, { 0, 14},
	{ 0, 1}, { 0, 9}, { 0, 3}, { 0,11}, { 0, 5}, { 0, 13}, { 0, 7}, { 0, 15}
};

int shiftTable_8Ch[8][2] = {
	{ 0, 0}, { 0, 2}, { 0, 4}, { 0, 6}, { 0, 1}, { 0, 3}, { 0, 5}, { 0, 7},
};
int shiftTable_6Ch[6][2] = {
	{ 0, 0}, { 0, 2}, { 0, 4},
	{ 0, 1}, { 0, 3}, { 0, 5}
};

void PCU_Control(int bd, int slot)
{ 
	int type, ch;

	if(myPs->config.chPerBd > 32){	//200407 lyhw 64Ch
		slot = slot % 64;
		type = shiftTable_64Ch[slot][0];
		ch = shiftTable_64Ch[slot][1];
	}else{	
		slot = slot % 32;
		type = shiftTable_32Ch[slot][0];
		ch = shiftTable_32Ch[slot][1];
	}

	if(bd >= myPs->config.installedBd) return;
	if(ch >= myPs->config.chPerBd) return;

	myCh = &(myData->bData[bd].cData[ch]);

	PCU_State_Check(bd, ch);					//190424 lyhw
	PCU_Inverter_Control(bd, ch);				//180626 lyhw

	pCalChAverage(bd, ch);	
	pFaultCond_Common(bd, ch);					//20905 lyhw

	if(myCh->ChAttribute.chNo_master == P0){	//parallel slave Ch
		if(myCh->op.type == STEP_USER_PATTERN
			&& myData->mData.config.rt_scan_type == RT_SCAN_PERIOD_10mS){
			pFaultCondHard_P(bd, ch);				//check 10ms
			if(myCh->op.runTime % 10 == 0) {		//check 100ms
				cFaultCondSoft_P(bd, ch);		
			}
		}else{
			if(myCh->op.runTime % 10 == 0) {		//check 100ms
				cFaultCondSoft_P(bd, ch);		
				cFaultCondHard_P(bd, ch);
			}
		}
	}else if(myCh->ChAttribute.opType == P0){	//general Ch
		if(myCh->op.type == STEP_USER_PATTERN
			&& myData->mData.config.rt_scan_type == RT_SCAN_PERIOD_10mS){	
			pFaultCondHard(bd, ch);					//check 10ms
			if(myCh->op.runTime % 10 == 0) {		//check 100ms
				cFaultCondSoft(bd, ch);	
			}
		}else{
			if(myCh->op.runTime % 10 == 0) {		//check 100ms
				cFaultCondSoft(bd, ch);	
				cFaultCondHard(bd, ch);
			}
		}
	}

#if FAULT_CONFIG_VERSION >= 1	
	//210901 LJS
	if(myCh->ChAttribute.chNo_master == P0){
		if(myCh->op.runTime % 10 == 0) {
			DropV_Charge_p(bd, ch);
			DropV_DisCharge_p(bd, ch);
		}
	}else if(myCh->ChAttribute.opType == P0){
		if(myCh->op.runTime % 10 == 0) {
			DropV_Charge(bd, ch);
			DropV_DisCharge(bd, ch);
		}
	}
#endif

	sens_ch_ad_count_increment(bd,ch);
	pFaultCheck(bd, ch);
	pStep(bd, ch);
	cStopCond(bd, ch);
	cEndCond(bd, ch);
	PCU_Relay_OnOff(bd, ch);
	if(myCh->ChAttribute.chNo_master == P0){
		PCU_ParallelSwitch_OnOff(bd, ch);
	}
//	pulse_data_1sec(bd, ch);
//	PCU_OutputTrigger(bd, ch);
}

#ifndef __SDI_MES_VER4__
void pulse_data_1sec(int bd, int ch)
{
	int time_send=0, flag, idx, i, msg=0;
	long capacity;
	unsigned long time, advStepNo;

	myCh = &(myData->bData[bd].cData[ch]);
	flag = myData->pulse_msg[0][bd][ch].flag;
	idx = myData->pulse_msg[msg][bd][ch].write_idx;
	advStepNo = myCh->misc.advStepNo;	//181611

	if(myCh->op.state != C_RUN){
		if(flag == 3){
			  myData->pulse_msg[0][bd][ch].flag = 2;	
			  return;
		}
		return;
	}
	/*
	if(myCh->op.phase == P50 || myCh->op.phase == P1){
	}else{
		if(flag == 3){
			  myData->pulse_msg[0][bd][ch].flag = 2;	
			  return;
		}
		return;
	}
*/
	if(flag == 0) myData->pulse_msg[0][bd][ch].flag = 1;

	if(idx > 99) idx = 0;

	myData->pulse_msg[0][bd][ch].write_idx = idx;
	myData->pulse_msg[0][bd][ch].chData[idx].ch = ch + 1;
	myData->pulse_msg[0][bd][ch].chData[idx].select = SAVE_FLAG_MONITORING_DATA;
	myData->pulse_msg[0][bd][ch].chData[idx].state = myCh->op.state;
	myData->pulse_msg[0][bd][ch].chData[idx].type = myCh->op.type;
	myData->pulse_msg[0][bd][ch].chData[idx].mode = myCh->op.mode;
	myData->pulse_msg[0][bd][ch].chData[idx].code = myCh->op.phase;
	myData->pulse_msg[0][bd][ch].chData[idx].stepNo = myCh->op.stepNo;
	myData->pulse_msg[0][bd][ch].chData[idx].grade = myCh->op.grade;
	myData->pulse_msg[0][bd][ch].chData[idx].Vsens = myCh->op.Vsens;
	myData->pulse_msg[0][bd][ch].chData[idx].Isens = myCh->op.Isens;
	if(myPs->config.capacityType == CAPACITY_AMPARE_HOURS){
		capacity = myCh->op.ampareHour;
	}else if(myPs->config.capacityType == CAPACITY_CAPACITANCE){
		capacity = myCh->op.capacitance;
	}else{
		capacity = 0;
	}
	myData->pulse_msg[0][bd][ch].chData[idx].capacity = capacity;
	myData->pulse_msg[0][bd][ch].chData[idx].watt = myCh->op.watt;
	myData->pulse_msg[0][bd][ch].chData[idx].wattHour = myCh->op.wattHour;
	myData->pulse_msg[0][bd][ch].chData[idx].runTime = myCh->op.runTime;
	myData->pulse_msg[0][bd][ch].chData[idx].totalRunTime 
			= myCh->op.totalRunTime;
	myData->pulse_msg[0][bd][ch].chData[idx].z = myCh->op.z;
	myData->pulse_msg[0][bd][ch].chData[idx].temp = myCh->op.temp;
	myData->pulse_msg[0][bd][ch].chData[idx].reservedCmd = myCh->op.reservedCmd;
	myData->pulse_msg[0][bd][ch].chData[idx].totalCycle 
			= myCh->misc.totalCycle;
	myData->pulse_msg[0][bd][ch].chData[idx].currentCycle 
			= myCh->misc.currentCycle;
	myData->pulse_msg[0][bd][ch].chData[idx].gotoCycleCount 
			= myCh->misc.gotoCycleCount[advStepNo];
		//	= myCh->misc.gotoCycleCount; 180611
	myData->pulse_msg[0][bd][ch].chData[idx].avgV = myCh->op.meanVolt;
	myData->pulse_msg[0][bd][ch].chData[idx].avgI = myCh->op.meanCurr;
#if NETWORK_VERSION > 4101
	#if VENDER != 2 //NOT SDI
	myData->pulse_msg[0][bd][ch].chData[idx].IntegralAmpareHour
			= myCh->op.integral_ampareHour;
	myData->pulse_msg[0][bd][ch].chData[idx].IntegralWattHour
			= myCh->op.integral_WattHour;
	myData->pulse_msg[0][bd][ch].chData[idx].ChargeAmpareHour
			= myCh->op.charge_ampareHour;
	myData->pulse_msg[0][bd][ch].chData[idx].ChargeWattHour
			= myCh->op.charge_wattHour;
	myData->pulse_msg[0][bd][ch].chData[idx].DischargeAmpareHour
			= myCh->op.discharge_ampareHour;
	myData->pulse_msg[0][bd][ch].chData[idx].DischargeWattHour
			= myCh->op.discharge_wattHour;
	myData->pulse_msg[0][bd][ch].chData[idx].cvTime = myCh->misc.cvTime;
		#if PROGRAM_VERSION1 == 0
			#if PROGRAM_VERSION2 >= 1
	myData->pulse_msg[0][bd][ch].chData[idx].Farad = myCh->op.capacitance;
	myData->pulse_msg[0][bd][ch].chData[idx].totalRunTime_carry
			= myCh->op.totalRunTime_carry;
	myData->pulse_msg[0][bd][ch].chData[idx].cycleNo = myCh->misc.cycleNo;
	myData->pulse_msg[0][bd][ch].chData[idx].temp1 = myCh->op.temp1;
			#endif
		#endif
		#if PROGRAM_VERSION1 > 0
	myData->pulse_msg[0][bd][ch].chData[idx].Farad = myCh->op.capacitance;
	myData->pulse_msg[0][bd][ch].chData[idx].totalRunTime_carry
			= myCh->op.totalRunTime_carry;
	myData->pulse_msg[0][bd][ch].chData[idx].temp1 = myCh->op.temp1;
	myData->pulse_msg[0][bd][ch].chData[idx].cycleNo = myCh->misc.cycleNo;
	myData->pulse_msg[0][bd][ch].chData[idx].startVoltage = myCh->misc.startV;
	myData->pulse_msg[0][bd][ch].chData[idx].maxVoltage = myCh->misc.maxV;
	myData->pulse_msg[0][bd][ch].chData[idx].minVoltage = myCh->misc.minV;
	myData->pulse_msg[0][bd][ch].chData[idx].startTemp = myCh->misc.startT;
	myData->pulse_msg[0][bd][ch].chData[idx].maxTemp = myCh->misc.maxT;
	myData->pulse_msg[0][bd][ch].chData[idx].minTemp = myCh->misc.minT;
		#endif
	#endif
#endif
	time = myData->pulse_msg[0][bd][ch].chData[idx].totalRunTime / 100;
   	if(time){
		time = myData->pulse_msg[0][bd][ch].chData[idx].totalRunTime % 100;
		if(time){
			time_send = 1;
		}
		time_send = 0;
	}

	if(idx == 99 || time_send == 1){
		for(i=0;i<idx+1;i++){
			memcpy((char *)&myData->pulse_msg[1][bd][ch].chData[i],
					(char *)&myData->pulse_msg[0][bd][ch].chData[i],
					sizeof(S_SAVE_MSG_CH_DATA));
			memset((char *)&myData->pulse_msg[0][bd][ch].chData[i],
						0x00, sizeof(S_SAVE_MSG_CH_DATA));
		}
		myData->pulse_msg[1][bd][ch].flag = 1;
		myData->pulse_msg[0][bd][ch].flag = 3;
		myData->pulse_msg[0][bd][ch].write_idx = 0;
		myData->pulse_msg[1][bd][ch].write_idx = idx + 1;
		time_send = 0;
	}else{
		myData->pulse_msg[0][bd][ch].write_idx++;
	}
}
#endif

void pStep(int bd, int ch)
{
	myCh = &(myData->bData[bd].cData[ch]);
	
	switch(myCh->op.state) {
   		case C_IDLE:	pIdle(bd, ch);		break;
   		case C_STANDBY:	pStandby(bd, ch);	break;
		case C_CALI: 	pCali(bd, ch);		break;
		case C_PAUSE:	pPause(bd, ch);		break;
		case C_CALI_UPDATE: pCaliUpdate(bd, ch); break;
		case C_RUN:
			// 111125 oys w : totalRunTime increase			
			if(myCh->op.totalRunTime >= MAX_TOTAL_RUNTIME){
				myCh->op.totalRunTime_carry += 1;
				myCh->op.totalRunTime = 0;
			}
			//181202 NOU use for Temp wait
	//		myCh->op.totalRunTime += myPs->misc.rt_scan_time;
			switch(myCh->op.type) {
				case STEP_CHARGE:		pStepCharge(bd, ch);		break;
				case STEP_DISCHARGE:	pStepDischarge(bd, ch);		break;
				case STEP_REST:			pStepRest(bd, ch);			break;
				case STEP_OCV:			pStepOcv(bd, ch);			break;
				case STEP_Z:			pStepZ(bd, ch);				break;
				case STEP_USER_PATTERN:	pStepUserPattern(bd, ch);	break;
				case STEP_USER_MAP:		pStepUserMap(bd, ch);		break;
				case STEP_BALANCE:		cStepBalance(bd, ch);		break;
				case STEP_ACIR:			cStepAcir(bd, ch);			break;
				default: //STEP_ADV_CYCLE, STEP_LOOP, STEP_END
					cStepDefault(bd, ch);
					break;
			}
			break;
    	default: break;
	}
}

void pIdle(int bd, int ch)
{
	int base_addr, addr_step, addr1, addr, ch_div;
	int inv_div, inv_num, inv_num1;

	base_addr = myPs->addr.main[BASE_ADDR];
	addr_step = myPs->addr.main[ADDR_STEP];
	ch_div = myPs->pcu_config.portPerCh;
	if(ch < 32){
	addr1 = base_addr + addr_step * (ch / ch_div);
	} else{
	addr1 = base_addr + addr_step * ((ch-32) / ch_div);
	}
	myCh = &(myData->bData[bd].cData[ch]);
	
	switch(myCh->op.phase) {
		case P0:
   			myCh->op.code = C_CODE_IDLE;
			myCh->op.type = 0;
			myCh->op.mode = 0;
   			myCh->op.totalRunTime = 0;
   			myCh->op.totalRunTime_carry = 0;
   			myCh->misc.cycleRunTime = 0;
   			myCh->misc.cycleNo = 0;
	       	myCh->op.runTime = 0;
	       	myCh->op.checkDelayTime = 0;
    		myCh->op.grade = 0;
	       	myCh->op.stepNo = 0;
			myCh->op.meanVolt = 0;
			myCh->op.meanCurr = 0;
        	myCh->op.watt = 0;
	       	myCh->op.wattHour = 0;
   		  	myCh->op.ampareHour = 0;
   		  	myCh->op.capacitance = 0;
	       	myCh->op.z = 0;
			myCh->misc.saveZ = 0;
			myCh->op.resultIndex = 0;
			myCh->misc.seq_no_cnt = 0;			//180822 add
			myCh->misc.temp_wait_flag_cnt = 0;
			myCh->misc.step_count = 0;		//kjc_211023
			myCh->misc.cycleStepCount = 0;	//kjc_211023
			//1807017 add lyh
			memset((char *)&myCh->misc.errCnt, 0x00, MAX_SIGNAL);
			ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, CMD_PCU_RESET, 0,0);//Reset
		/*	addr= addr1 + CREG_CTRL_REG;
			if(!(SREG16_read(addr) & 0x0001)){
				SREG16_write(addr, 0x0001); // auto enable
			}*/
		    myCh->op.phase++;
    		break;
		case P1:
			addr= addr1 + CREG_CTRL_REG;
			if(!(SREG16_read(addr) & 0x0001)){
				SREG16_write(addr, 0x0001); // auto enable
			}
		    myCh->op.phase++;
			break;
		case P2:
			ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, CMD_PCU_MODE_CC, 0,0);//CC0
		    myCh->op.phase++;
			break;
		case P3:
			myCh->signal[C_SIG_OUT_SWITCH] = P0;
			myCh->signal[C_SIG_OUT_SWITCH_OFF] = P1;
		    myCh->op.phase++;
			break;
		case P5:
/*			addr= addr1 + CREG_CTRL_HVU1_STATE;
			if(!(SREG16_read(addr) & 0x0001)){ //if HVU Error then reset
				ch_send_cmd(bd, ch, ADDR_HVU_CLASS_MULTICAST, CMD_PCU_RESET, 0,0);//HVU OFF
			}*/ // khkw 20131014
			//181011
			myPs->signal[M_SIG_CALI_VOLTAGE2_RELAY] = P0;
			myPs->signal[M_SIG_CALI_CHARGE2_RELAY] = P0;
			myPs->signal[M_SIG_CALI_DISCHARGE2_RELAY] = P0;
		    myCh->op.phase++;
			break;
		case P8:
			if(myPs->signal[M_SIG_INV_POWER1] == P10){
				inv_div = myPs->pcu_config.invPerCh;
				inv_num1 = ch / inv_div;
				inv_num = inv_num1 * inv_div;
				if(myData->bData[bd].cData[inv_num].inv_power == P0){
					myData->bData[bd].cData[inv_num].inv_power = P1;
				}
			}
		    myCh->op.phase++;
			break;
		case P10:
			myCh->opSave
				= myCh->op;
	        myCh->op.state = C_STANDBY;
	       	myCh->op.phase = P0;
			break;
		default: 
		    myCh->op.phase++;
			break;
	}
}

void pStandby(int bd, int ch)
{
	int i, j=0, type;
	int inv_div, inv_num, inv_num1;
	unsigned long delay_time, startStepNo=1, saveDt, advStepNo;
	long refTemp, groupTemp, refTemp_backup;
//	unsigned long relay_state;
	int base_addr, addr_step, addr1, addr, ch_div;

	myCh = &(myData->bData[bd].cData[ch]);
	
	base_addr = myPs->addr.main[BASE_ADDR];
	addr_step = myPs->addr.main[ADDR_STEP];
	ch_div = myPs->pcu_config.portPerCh;
	if(ch < 32){
	addr1 = base_addr + addr_step * (ch / ch_div);
	} else{
	addr1 = base_addr + addr_step * ((ch-32) / ch_div);
	}
	inv_div = myPs->pcu_config.invPerCh;
	
	//191204 lyhw
	advStepNo = myCh->misc.advStepNo;
	saveDt = myPs->testCond[bd][ch].step[advStepNo].saveDt;
	
	switch(myCh->op.phase) {
		case P0:
			for(i=0; i < MAX_SIGNAL; i++) myCh->signal[i] = P0;
			for(i=0; i < MAX_STEP; i++) 
				myCh->misc.gotoCycleCount[i] = P0;
			//pjy add for toshiba
			for(i =0; i< MAX_CYCLE; i++){
				myCh->misc.charge_integralCap[i] = 0;
				myCh->misc.discharge_integralCap[i] = 0;
			}//end of add
			memset((char *)&myCh->misc.errCnt, 0x00, MAX_SIGNAL);
			myCh->op.rangeV = 0;
			myCh->op.rangeI = 0;
			myCh->op.preType = STEP_IDLE;
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
			if(myCh->op.checkDelayTime == 10){
				ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, 
												CMD_PCU_MODE_CC, 0,0); //CC0
				if(myData->CaliMeter.config.Shunt_Sel_Calibrator == P3){
					outb(0x00, 0x601);		//180805 auto Cali Relay off
					outb(0x00, 0x602);
			//		outb(0x00, 0x611);		//180921 add Smps Relay off
				}
			}else if(myCh->op.checkDelayTime == 100){
				ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, 
												CMD_PCU_RESET, 0,0); //Reset
				//190712 oys add : End Step save wait delay
				if(myCh->op.select == SAVE_FLAG_SAVING_END
								&& myCh->op.code != C_END_STEP) {
					if(myData->DataSave.config.save_data_type == P1) {
						send_save_msg(bd, ch, saveDt, 1);
					}else{	//hun_210817
						if(myCh->ChAttribute.opType == P0){
							send_save_msg(bd, ch, saveDt, 1);
						}
					}
				}
			}else if(myCh->op.checkDelayTime == 200){
				myCh->signal[C_SIG_PARALLEL_SWITCH_OFF] = P1;
			}else if(myCh->op.checkDelayTime >= 300){
				myCh->op.checkDelayTime = 0;
				myCh->signal[C_SIG_OUT_SWITCH] = P0;
				myCh->signal[C_SIG_OUT_SWITCH_OFF] = P1;
				myCh->op.phase = P1;
			}
			break;
		case P1:
			if(myCh->op.code != P_CD_FAULT_SEQ_NO){
    			myCh->op.code = C_CODE_IDLE;
			}
			myCh->op.type = 0;
			myCh->op.mode = 0;
    		myCh->op.totalRunTime = 0;
   			myCh->op.totalRunTime_carry = 0;
			myCh->op.grade = 0;
	       	myCh->op.stepNo = 0;
	       	myCh->op.runTime = 0;
   			myCh->misc.cycleRunTime = 0;
   			myCh->misc.cycleNo = 0;
	       	myCh->op.checkDelayTime = 0;
			myCh->op.meanVolt = 0;
			myCh->op.meanCurr = 0;
        	myCh->op.watt = 0;
	       	myCh->op.wattHour = 0;
   		  	myCh->op.ampareHour = 0;
   		  	myCh->op.charge_ampareHour = 0;
   		  	myCh->op.discharge_ampareHour = 0;
   		  	myCh->op.charge_wattHour = 0;
			myCh->op.discharge_wattHour = 0;
			myCh->op.capacitance = 0;
	       	myCh->op.z = 0;
			myCh->misc.saveZ = 0;
			myCh->op.resultIndex = 0;
			myCh->op.preType = STEP_IDLE;
			myCh->misc.preRangeI = 99;

			myCh->misc.pulseDataCount = 0;
			myCh->misc.delta_flag = P0;
   		   	myCh->misc.saveDt = 0;
   		   	myCh->misc.saveDv = 0;
   		   	myCh->misc.saveDi = 0;
   		   	myCh->misc.saveDtemp = 0;
			myCh->misc.meanCount = 0;
			myCh->misc.meanSumVolt = 0;
			myCh->misc.meanSumCurr = 0;
			myCh->misc.ocv = 0;
			myCh->misc.actualCapacity = 0;
			myCh->misc.sumCapacity = 0;
			myCh->misc.seedCapacity = 0;
			myCh->misc.sumChargeAmpareHour = 0;
			myCh->misc.sumDischargeAmpareHour = 0;
			myCh->misc.sumChargeWattHour = 0;
			myCh->misc.sumDischargeWattHour = 0;
			myCh->misc.seedChargeWattHour = 0;
			myCh->misc.seedDischargeWattHour = 0;
			myCh->misc.seedChargeAmpareHour = 0;
			myCh->misc.seedDischargeAmpareHour = 0;
        	myCh->misc.tmpWatt = 0;
			myCh->misc.sumWattHour = 0;
			myCh->misc.seedWattHour = 0;
			myCh->misc.maxV = 0;
			myCh->misc.minV = 0;
			myCh->misc.startV = 0;
			myCh->misc.startT = 0;
			myCh->misc.maxT = 0;
			myCh->misc.minT = 0;
			myCh->misc.maxI = 0;
			myCh->misc.minI = 0;
			myCh->misc.c_v1 = 0;
			myCh->misc.c_v2 = 0;
			myCh->misc.c_t1 = 0;
			myCh->misc.c_t2 = 0;
			myCh->misc.start = 1;
			myCh->misc.fbCountV_H = 0;
			myCh->misc.fbCountI_H = 0;
			myCh->misc.fbSumI_H = 0;
			myCh->misc.fbSumV_H = 0;
			myCh->misc.fbCountV_L = 0;
			myCh->misc.fbCountI_L = 0;
			myCh->misc.fbSumI_L = 0;
			myCh->misc.fbSumV_L = 0;
			myCh->misc.fbCountV_M = 0;
			myCh->misc.fbCountI_M = 0;
			myCh->misc.fbSumI_M = 0;
			myCh->misc.fbSumV_M = 0;
			myCh->misc.sensCount = 0;
			myCh->misc.userPatternCnt = 0;
			myCh->ccv[0].index = 0;
			myCh->misc.sensCountFlag = P0;
			myCh->misc.sensBufCount = 0;
			myCh->misc.sensBufCountFlag = P0;
			myCh->misc.tempDir = P0;
			myCh->misc.chamberStepNo = 0;
			myCh->misc.advStepNo = 0;
			myCh->misc.fbV = 0;
			myCh->misc.fbI = 0;
			myCh->misc.cvTime = 0;
			myCh->misc.pid_ui1[0] = 0.0;
			myCh->misc.pid_ui1[1] = 0.0;
			myCh->misc.pid_error1[0] = 0.0;
			myCh->misc.pid_error1[1] = 0.0;
			myCh->misc.relayStartFlag = 0;
		//	myCh->misc.gotoCycleCount = 1; //1base
			//111215 detail calibration add
			myCh->misc.caliCheckPoint = 0;
			myCh->misc.caliCheckSum = 0;
			myCh->misc.caliCheckSum1 = 0;
			myCh->misc.send_pcu_seq_no = 0;
			myCh->misc.receive_pcu_seq_no = 0;	//180817 add
			myCh->op.semiPreType = 0;			//180905 add
			myCh->misc.pauseRunTime = 0;		//190901 lyhw
			
			myCh->misc.chamberWaitFlag = P0;
			myCh->misc.chGroupNo = 0;			//180624 add
			myCh->misc.chamberNo = 0;			//180624 add
			myCh->misc.stepSyncFlag = 0;		//180624 add
			myCh->misc.refTemp_backup = 999000; //181202 add
			//191029 oys
			myCh->misc.userDataNo = 0;
			//191119 lyhw
			myCh->misc.userPattern_ReadFlag = 0;
			myCh->misc.pattern_change_flag = 0;
			myCh->misc.StepPattern_TotalNum = 0;
			myCh->misc.StepPattern_ReadNum = 0;
			myCh->misc.cvFaultCheckFlag = 0;		//210204
			myCh->misc.hw_fault_temp = 0;
			//210318 lyhw
			myCh->misc.sumAccumulated_Capacity = 0;
			myCh->misc.seedAccumulated_Capacity = 0;
			myCh->misc.Accumulated_Capacity = 0;
			myCh->misc.sumAccumulated_WattHour = 0;
			myCh->misc.seedAccumulated_WattHour = 0;
			myCh->misc.Accumulated_WattHour = 0;
			//210521 ljsw
			myCh->misc.restCheckFlag = 0;
			myCh->misc.restFaultCount = 0;
			myCh->misc.restCheckStartFlag = 0;
			myCh->misc.restCheckNum = 0;
			#if CHAMBER_TEMP_HUMIDITY == 1	//kjc_210415
			myCh->misc.cham_check_time_flag = 0;
			myCh->misc.cham_check_time_new = 0;
			myCh->misc.cham_check_time_1sec = 0;
			myCh->misc.efficiency_Ah[0] = 0;
			myCh->misc.efficiency_Ah[1] = 0;
			myCh->misc.loopStepNo = 0;
			myCh->misc.calc_retain_Ah = 0;
			myCh->misc.chargeAccAh = 0;
			myCh->misc.dischargeAccAh = 0;
			myCh->misc.faultEfficiencyAh = 0;
			#endif
			#ifdef _SDI_SAFETY_V1
			myCh->misc.fault_deltaV = 0;
			#endif
			#ifdef _SDI_SAFETY_V2
			myCh->misc.Master_recipe_deltaV = 0;
			#endif	
			#ifdef _ULSAN_SDI_SAFETY
			myCh->misc.humpCheck_T = 0;
			myCh->misc.humpCheck_I = 0;
			myCh->misc.humpComp_T = 0;
			myCh->misc.humpComp_I = 0;
			#endif
			#ifdef _EXTERNAL_CONTROL
			myCh->misc.chControl = 0;	//hun_210723
			myCh->misc.chPause = 0;		//hun_210723
			myCh->misc.external_return = 0;
			myCh->misc.external_return = 0x00001000;
			myCh->misc.chControl = 0;	//hun_210723
			#endif

			myCh->misc.Drop_maxV = 0;	//210916 LJS
			myCh->misc.Drop_minV = 0;	//210916 LJS
			
			myCh->misc.step_count = 0;		//kjc_211023
			myCh->misc.cycleStepCount = 0;	//kjc_211023

			//hun_211125
			myCh->misc.charge_cc_hump_flag = P0;
			myCh->misc.charge_cv_hump_flag = P0;
			myCh->misc.charge_cc_hump_start_time = 0;
			myCh->misc.charge_cv_hump_start_time = 0;
			myCh->misc.charge_cc_hump_start_voltage = 0;
			myCh->misc.charge_cv_hump_start_current = 0;

			myCh->misc.discharge_cc_hump_flag = P0;
			myCh->misc.discharge_cv_hump_flag = P0;
			myCh->misc.discharge_cc_hump_start_time = 0;
			myCh->misc.discharge_cv_hump_start_time = 0;
			myCh->misc.discharge_cc_hump_start_voltage = 0;
			myCh->misc.discharge_cv_hump_start_current = 0;
			
			//220222 LJS
			myCh->misc.endState = P0;
			myCh->misc.groupEndTime = 0;
			myCh->misc.groupAvgVsens = 0;
			myCh->misc.Fault_Check_flag = 0;
			myCh->misc.Std_Time = 0;

			memset((char *)&myCh->misc.d_t, 0x00, sizeof(long)*100);
			memset((char *)&myCh->misc.d_v, 0x00, sizeof(long)*100);
			memset((char *)&myCh->ccv, 0x00, sizeof(S_CH_CCV)*2);
    		myCh->op.phase = P2;
			break;
		case P2:
			//190627 lyhw add
			/*
			if(myCh->ChAttribute.chNo_master == P0){
				if(myCh->pcu_misc.Rcv_parallel_relay == P1){
					myCh->signal[C_SIG_PARALLEL_SWITCH_OFF] = P1;
				}
			}	
			*/
			myCh->opSave = myCh->op;
			myCh->op.phase = P3;
			break;
		case P3:
			inv_num1 = ch / inv_div;
			inv_num = inv_num1 * inv_div;
			if(myData->bData[bd].cData[inv_num].inv_power == P7){
				//191024 add INPUT Error Clear
				ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, 
												CMD_PCU_RESET, 0,0); //Reset
			}
			
			i = myCh->ccv[0].index;
			myCh->ccv[0].index++;
			if(myCh->ccv[0].count != 10) {
				myCh->ccv[0].count = myCh->ccv[0].index;
			}
			if(myCh->ccv[0].index >= 10) {
				myCh->ccv[0].index = 0;
				myCh->ccv[0].count = 10;
			}
			myCh->ccv[0].ad_ccv[i] = myCh->misc.tmpVsens;
			myCh->ccv[0].ad_cci[i] = myCh->misc.tmpIsens;

			i = 0;
			if(myCh->signal[C_SIG_RUN] == P1) {
				myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
				if(myCh->op.checkDelayTime == 10){
					ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, 
												CMD_PCU_RESET, 0,0); //Reset
					memset((char *)&myCh->misc.errCnt, 0x00, MAX_SIGNAL);
				}else if(myCh->op.checkDelayTime == 11){
					addr= addr1 + CREG_CTRL_REG;
					if(!(SREG16_read(addr) & 0x0001)){
						SREG16_write(addr, 0x0001); // auto enable
					}
				}else{
					if(myCh->op.checkDelayTime < 50) return;
				}
				//180830 add
				inv_num1 = ch / inv_div;
				inv_num = inv_num1 * inv_div;
				if(myData->bData[bd].cData[inv_num].inv_power == P0){
					myData->bData[bd].cData[inv_num].inv_power = P1;
				}
				//20020905 lyhw	
				if(myData->bData[bd].cData[inv_num].inv_power != P10
					&& myCh->op.checkDelayTime <= 6000) return;
				
				ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, 
										CMD_PCU_RESET, 0,0); //Reset
				
				myCh->op.checkDelayTime = 0;
				myCh->signal[C_SIG_RUN] = P0;
				if(myPs->testCond[bd][ch].reserved.select_run == 0) {
					myCh->op.type = myPs->testCond[bd][ch].step[0].type;
					myCh->op.mode = myPs->testCond[bd][ch].step[0].mode;

					i = 1;
				} else {
					j = (int)myPs->testCond[bd][ch].reserved.select_stepNo - 1;
					if(j < 0){
						j = 0;
					}
					// 120427 oys w : select cycle & step process
					if(myPs->testCond[bd][ch].step[j].type == P7){
						myCh->op.type = myPs->testCond[bd][ch].step[j+1].type;
						myCh->op.mode = myPs->testCond[bd][ch].step[j+1].mode;
						myCh->op.stepNo = myPs->testCond[bd][ch].step[j+1].stepNo;
						myCh->misc.advStepNo = (unsigned long)j + 1;
						i = 2;
					} else {
						myCh->op.type = myPs->testCond[bd][ch].step[j].type;
						myCh->op.mode = myPs->testCond[bd][ch].step[j].mode;
						myCh->op.stepNo = myPs->testCond[bd][ch].step[j].stepNo;
						myCh->misc.advStepNo = (unsigned long)j;
						i = 2;
					}
				}
			} else if(myCh->signal[C_SIG_RESET] == P1) {
				myCh->signal[C_SIG_RESET] = P0;
				myCh->op.state = C_IDLE;
				myCh->op.phase = P0;
				break;
			} else if(myCh->signal[C_SIG_CALI] == P1) {
				//180830 add
				inv_num1 = ch / inv_div;
				inv_num = inv_num1 * inv_div;
				if(myData->bData[bd].cData[inv_num].inv_power == P0){
					myData->bData[bd].cData[inv_num].inv_power = P1;
				}
				myCh->signal[C_SIG_CALI] = P0;
				myCh->signal[C_SIG_OUT_SWITCH] = P0;
				myCh->signal[C_SIG_OUT_SWITCH_OFF] = P1;
				i = 3;
			}

			if(i == 1) {
    			myCh->op.code = C_CODE_IDLE;
				myCh->op.phase = P4;
				myCh->misc.advCycle = 0;
				myCh->misc.advCycleStep = 0;
				myCh->misc.advStepNo = 0;
				myCh->misc.chamberStepNo = myCh->misc.advStepNo;
				myCh->misc.currentCycle = 0;
				myCh->misc.totalCycle = 0;
				type = myPs->testCond[bd][ch].step[1].type;
				if(type == STEP_REST) {
					if(myPs->testCond[bd][ch].step[1].endT != 0) {
						myCh->signal[C_SIG_OUT_SWITCH] = P0; //201015
						myCh->signal[C_SIG_OUT_SWITCH_OFF] = P1;
						myCh->signal[C_SIG_RANGE_SWITCH] = P0;
						myCh->misc.relayStartFlag = 0;
					}
					/*		
						if(myPs->testCond[bd][ch].step[1].endT >= 6000) {
							myCh->signal[C_SIG_OUT_SWITCH] = P0;
							myCh->signal[C_SIG_OUT_SWITCH_OFF] = P1;
							myCh->signal[C_SIG_RANGE_SWITCH] = P0;
						} else {
							myCh->misc.relayStartFlag = 1;
						}
					} else {
						myCh->signal[C_SIG_OUT_SWITCH] = P0;
						myCh->signal[C_SIG_OUT_SWITCH_OFF] = P1;
						myCh->signal[C_SIG_RANGE_SWITCH] = P0;
					}
					*/
				}else if(type == STEP_USER_PATTERN) { //111008 kji
					myCh->misc.relayStartFlag = 3;
				}else if(type == STEP_OCV) {
					myCh->misc.relayStartFlag = 1;
					type = myPs->testCond[bd][ch].step[2].type;
					if(type == STEP_REST) {
						if(myPs->testCond[bd][ch].step[2].endT != 0) {
						myCh->signal[C_SIG_OUT_SWITCH] = P0; //201015
						myCh->signal[C_SIG_OUT_SWITCH_OFF] = P1;
						myCh->signal[C_SIG_RANGE_SWITCH] = P0;
						myCh->misc.relayStartFlag = 0;
						}
						/*		
							if(myPs->testCond[bd][ch].step[2].endT >= 6000) {
								myCh->signal[C_SIG_OUT_SWITCH] = P0;
								myCh->signal[C_SIG_OUT_SWITCH_OFF] = P1;
								myCh->signal[C_SIG_RANGE_SWITCH] = P0;
							} else {
								myCh->misc.relayStartFlag = 2;
							}
						} else {
							myCh->signal[C_SIG_OUT_SWITCH] = P0;
							myCh->signal[C_SIG_OUT_SWITCH_OFF] = P1;
							myCh->signal[C_SIG_RANGE_SWITCH] = P0;
						}
						*/
					}
				} else {
					myCh->misc.relayStartFlag = 1;
				}
				//180628 lyhw 0 -> 999000
				//181202 add for chamber Temp Wait
				if(myData->mData.config.function[F_CHAMBER_TEMP_WAIT] == P1){
					if(myPs->testCond[bd][ch].step[1].refTemp != 999000) {
						myCh->misc.relayStartFlag = 100; //chamber temp wait
					}
				}
			} else if(i == 2) {
				myCh->op.phase = P4;
//				myCh->misc.advStepNo = (unsigned long)j;
				type = myPs->testCond[bd][ch].step[j].type;
				if(type == STEP_REST) {
					if(myPs->testCond[bd][ch].step[j].endT != 0) {
						myCh->signal[C_SIG_OUT_SWITCH] = P0; //201015
						myCh->signal[C_SIG_OUT_SWITCH_OFF] = P1;
						myCh->signal[C_SIG_RANGE_SWITCH] = P0;
						myCh->misc.relayStartFlag = 0;
					}
					/*		
						if(myPs->testCond[bd][ch].step[j].endT >= 6000) {
							myCh->signal[C_SIG_OUT_SWITCH] = P0;
							myCh->signal[C_SIG_OUT_SWITCH_OFF] = P1;
							myCh->signal[C_SIG_RANGE_SWITCH] = P0;
						} else {
							myCh->misc.relayStartFlag = 2;
						}
					} else {
						myCh->signal[C_SIG_OUT_SWITCH] = P0;
						myCh->signal[C_SIG_OUT_SWITCH_OFF] = P1;
						myCh->signal[C_SIG_RANGE_SWITCH] = P0;
					}
				*/
				} else if(type == STEP_USER_PATTERN) { //111008 kji
					myCh->misc.relayStartFlag = 3;
				} else {
					myCh->misc.relayStartFlag = 2;
				}

				//181202 add for Chamber Temp Wait
				if(myData->mData.config.function[F_CHAMBER_TEMP_WAIT] == P1){
					refTemp = myPs->testCond[bd][ch].step[j].refTemp;
					groupTemp = myCh->misc.groupTemp;
					//21071212 sch add
					refTemp_backup = myCh->misc.refTemp_backup;

					switch(type){
						case STEP_CHARGE:
						case STEP_DISCHARGE:
						case STEP_OCV:
						case STEP_REST:
						case STEP_Z:
						case STEP_USER_PATTERN:
						case STEP_BALANCE:
#if MACHINE_TYPE == 1
						case STEP_SHORT:
#endif
							if(refTemp != 999000){
								if(refTemp == groupTemp 
									|| refTemp == refTemp_backup){
									myCh->misc.tempDir = P0;
								}else if(refTemp > groupTemp){
									myCh->misc.tempDir = P1;
								}else{
									myCh->misc.tempDir = P2;
								}
								//chamber temp wiat
								myCh->misc.relayStartFlag = 100;
							}
							break;
						default:
							myCh->misc.tempDir = P0;
							break;	
					}
				}

				j = (int)myPs->testCond[bd][ch].reserved.select_cycleNo;
				myCh->misc.advCycle = (unsigned long)j;
				myCh->misc.currentCycle = (unsigned long)j;
				myCh->misc.totalCycle = (unsigned long)j;
				myCh->misc.advCycleStep
					= myPs->testCond[bd][ch].reserved.select_advCycleStep;
			} else if(i == 3) {
				myCh->op.phase = P7;
			}
			myCh->ccv[0].avg_v = myCh->misc.tmpVsens;
			myCh->ccv[0].avg_i = myCh->misc.tmpIsens;
			break;
		case P4:
			if(myCh->op.checkDelayTime == 10){  
				myCh->op.checkDelayTime = 0;
				if(myCh->ChAttribute.chNo_master == P0){
					myCh->signal[C_SIG_PARALLEL_SWITCH_ON] = P1;
					myCh->op.phase = P5;
				}else{
					myCh->op.phase = P5;
				}
			}
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
			break;
		case P5:	//180830
			inv_num1 = ch / inv_div;
			inv_num = inv_num1 * inv_div;
			if(myData->bData[bd].cData[inv_num].inv_power == P10){
				myCh->op.checkDelayTime = 0;
				myCh->op.phase = P6;
			}else if(myData->bData[bd].cData[inv_num].inv_power == P99){
				//191024 lyhw
				send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS
								, M_FAIL_PCU_HVU_INPUT_ERROR, ch + 1);
				myCh->op.state = C_STANDBY;
				myCh->op.phase = P0;
			}
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
			if(myCh->op.checkDelayTime >= 6000){  //after 60 sec
				myCh->op.state = C_STANDBY;
				myCh->op.phase = P0;
			}
			break;
		case P6:
			if(myCh->misc.relayStartFlag == 2){
				startStepNo = (unsigned long)myPs->testCond[bd][ch]
												.reserved.select_stepNo - 1;
			}
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
			//110402
			if(myCh->misc.relayStartFlag > 0) {
				if(myCh->op.checkDelayTime >= 100) {
					if(myCh->misc.relayStartFlag < 3) {
						myCh->signal[C_SIG_OUT_SWITCH_ON] = P1;
						myCh->signal[C_SIG_OUT_SWITCH] = P1;
						myCh->signal[C_SIG_RANGE_SWITCH] = P1;
						myCh->signal[C_SIG_V_RANGE]
							= myPs->testCond[bd][ch].step[startStepNo].rangeV+1;
						myCh->signal[C_SIG_I_RANGE]
							= myPs->testCond[bd][ch].step[startStepNo].rangeI+1;
						//181127 lyhw add for Relay Retry
						myCh->op.phase = P10;
						myCh->op.checkDelayTime = 0;
					}else if(myCh->misc.relayStartFlag == 100){
						myCh->op.phase = P10;
						myCh->op.checkDelayTime = 0;
					}
					myCh->misc.relayStartFlag = 0;
				}
			}else if(myCh->misc.relayStartFlag == 0){
				myCh->op.phase = P10;
				myCh->op.checkDelayTime = 0;
			}
							
			if(myData->AppControl.config.debugType != P0){
				myCh->op.phase = P11;
			}
		//	if(myCh->op.checkDelayTime >= delay_time) {	//180515 lyhw check
		//		if((myCh->op.checkDelayTime - 200) < 10*ch) break;
		//			myCh->op.phase = P50;
		//			myCh->op.checkDelayTime = 0;
		//	}
				/*	
				if(myCh->ChAttribute.chNo_master == P0){	//180719 lyhw
					myCh->op.state = C_RUN;
					myCh->op.phase = P0;
					myCh->op.checkDelayTime = 0;
					myData->bData[bd].cData[ch-1].op.state = C_RUN;
					myData->bData[bd].cData[ch-1].op.phase = P0;
					myData->bData[bd].cData[ch-1].op.checkDelayTime = 0;
				}else if(myCh->ChAttribute.opType == P0){
					myCh->op.state = C_RUN;
					myCh->op.phase = P0;
					myCh->op.checkDelayTime = 0;
				}*/
			break;
		case P7:	//180830 C_CALI Inv_phase Check
			inv_num1 = ch / inv_div;
			inv_num = inv_num1 * inv_div;
			if(myData->bData[bd].cData[inv_num].inv_power == P10){
				myCh->op.checkDelayTime = 0;
				myCh->op.phase = P8;
			}
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
			if(myCh->op.checkDelayTime >= 6000){  //after 60 sec
				myCh->op.state = C_STANDBY;
				myCh->op.phase = P0;
			}
			break;
		case P8:	//180830 C_CALI Start
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
			delay_time = 200;
			if(myCh->op.checkDelayTime >= delay_time) {
				myCh->op.state = C_CALI;
				myCh->op.phase = P0;
				myCh->op.checkDelayTime = 0;
			}
			break;
		case P10: 	//181127 add for Relay ON Check
			if(myCh->misc.send_pcu_seq_no != myCh->misc.receive_pcu_seq_no){
				myCh->misc.seq_no_cnt++;		 
				if(myCh->misc.seq_no_cnt >= MAX_SEQ_NO_RETRY){
					myCh->op.code = P_CD_FAULT_SEQ_NO;
					myCh->op.state = C_STANDBY;
					myCh->misc.seq_no_cnt = 0;
					myCh->op.phase = P0;
				}else{
					myCh->signal[C_SIG_OUT_SWITCH_ON] = P1;
				}
			}else{
				myCh->misc.seq_no_cnt = 0;	
				myCh->op.phase = P11;
			}
			break;
		case P11:	//181127 
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
			delay_time = 200;
			if(myCh->op.checkDelayTime >= delay_time) {	//180515 lyhw check
				if((myCh->op.checkDelayTime - 200) < 10*ch) break;
					myCh->op.phase = P50;
					myCh->op.checkDelayTime = 0;
			}
			break;
		case P50:		//180906 add for CC0 and Run
			if(myCh->op.checkDelayTime == 0){
				if(myData->bData[bd].cData[ch].ChAttribute.opType != P1){
					ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, 
														CMD_PCU_MODE_CC, 0,0);
				}
			}else if(myCh->op.checkDelayTime >= 10){
				if(myCh->ChAttribute.chNo_master == P0){	//180719 lyhw
					myCh->op.state = C_RUN;
					myCh->op.phase = P0;
					myCh->op.checkDelayTime = 0;
					myData->bData[bd].cData[ch-1].op.state = C_RUN;
					myData->bData[bd].cData[ch-1].op.phase = P0;
					myData->bData[bd].cData[ch-1].op.checkDelayTime = 0;
				}else if(myCh->ChAttribute.opType == P0){
					myCh->op.state = C_RUN;
					myCh->op.phase = P0;
					myCh->op.checkDelayTime = 0;
				}
			}
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
			break;
		default: break;
	}
}

void pPause(int bd, int ch)
{
	unsigned long advStepNo, saveDt, j, i=0;
	long type;
	unsigned char flag1=0,flag2=0;
	long long   flag3;
	int base_addr, addr_step, addr1, addr, ch_div, pauseSaveDt, pauseSaveEndT = 0;
	int inv_div, inv_p, inv_num, inv_num1;

	S_CH_STEP_INFO step;

	base_addr = myPs->addr.main[BASE_ADDR];
	addr_step = myPs->addr.main[ADDR_STEP];
	ch_div = myPs->pcu_config.portPerCh;
	if(ch < 32){
		addr1 = base_addr + addr_step * (ch / ch_div);
	} else{
		addr1 = base_addr + addr_step * ((ch-32) / ch_div);
	}
	
	inv_div = myPs->pcu_config.invPerCh;
	inv_p = myPs->pcu_config.parallel_inv_ch;
	inv_num = ch / inv_div;

	flag3 = 0x01;
	for(j=0;j<myPs->config.chPerBd;j++) {
	    if(j == ch)
	        break;
	    flag3 = flag3 << 1;
	}
	flag1 = flag3;
	flag3 = flag3 >> 8;
	flag2 = flag3;
	

	myCh = &(myData->bData[bd].cData[ch]);
	step = step_info(bd, ch);
	
	advStepNo = step.advStepNo;
	saveDt = step.saveDt;
	type = step.type;
	switch(myCh->op.phase) {
		case P0:
			switch(myPs->config.hwSpec){
				case L_2V_100A:
				case L_5V_50A:
				case S_5V_200A:
				case S_5V_200A_75A_15A_AD2:
					myCh->signal[C_SIG_V_RANGE] = RANGE0;
					myCh->signal[C_SIG_I_RANGE] = RANGE0;
					break;
				default:
					break;
			}
			myCh->misc.semiSwitchState = SEMI_PRE;
			myCh->signal[C_SIG_RANGE_SWITCH] = P0;
			myCh->signal[C_SIG_OUT_SWITCH] = P0;
			ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, CMD_PCU_MODE_CC, 0,0);
		//	myCh->signal[C_SIG_OUT_SWITCH_OFF] = P1;
			myCh->op.checkDelayTime = 0;
			myCh->misc.seq_no_cnt = 0;			//180822 
			myCh->misc.cvFaultCheckFlag = 0;		//210204
			//210521 ljsw
			myCh->misc.restCheckFlag = 0;
			myCh->misc.restFaultCount = 0;
			myCh->misc.restCheckStartFlag = 0;
			myCh->misc.restCheckNum = 0;
			myCh->op.phase = P1;
			break;
		case P1:
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
			if(myData->DataSave.config.save_data_type == P1){
				if(myCh->op.checkDelayTime == 100){		//191204 lyhw
					if(myCh->misc.reserved_cmd_flag == 0){			
						send_save_msg(bd, ch, saveDt, 1);
					}else{
						myCh->misc.reserved_cmd_flag = 0;
					}
				}
			}
			
			if(myCh->op.checkDelayTime == 10){
				myCh->signal[C_SIG_OUT_SWITCH_OFF] = P1;
			}else if(myCh->op.checkDelayTime >= 200){
				myCh->op.checkDelayTime = 0;
				//181226 lyhw
				if(myCh->ChAttribute.chNo_master == P0){
					myCh->signal[C_SIG_PARALLEL_SWITCH_OFF] = P1;
					myCh->op.phase = P2;
				}else{
					myCh->op.phase = P2;
				}
			}
			break;
		case P2:
			inv_num1 = ch / inv_div;
			inv_num = inv_num1 * inv_div;
			if(myData->bData[bd].cData[inv_num].inv_power == P7){
				//191024 add INPUT Error Clear
				ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, 
												CMD_PCU_RESET, 0,0); //Reset
			}
			//111215 kji w
			if(myCh->ChAttribute.chNo_master != P0) {
				if(myCh->op.code == C_FAULT_UPPER_CH_TEMP ||
					myCh->op.code == C_PAUSE_UPPER_TEMP_CH) {
					if(myPs->testCond[bd][ch].step[advStepNo].
							faultUpperTemp_restart != 0) {
						if(myPs->testCond[bd][ch].step[advStepNo].
							faultUpperTemp_restart >= myCh->op.temp) {
							myCh->misc.errCnt[C_CNT_UPPER_TEMP]++;
							if(myCh->misc.errCnt[C_CNT_UPPER_TEMP]>= 
											MAX_ERROR_CNT*4){
								myCh->misc.errCnt[C_CNT_UPPER_TEMP] = 0;
								myCh->signal[C_SIG_CONTINUE] = P1;
							}
						}
					}
				} else if(myCh->op.code == C_FAULT_LOWER_CH_TEMP ||
							myCh->op.code == C_PAUSE_LOWER_TEMP_CH) {
					if(myPs->testCond[bd][ch].step[advStepNo].
							faultLowerTemp_restart != 0) {
						if(myPs->testCond[bd][ch].step[advStepNo].
							faultLowerTemp_restart <= myCh->op.temp) {
							myCh->misc.errCnt[C_CNT_LOWER_TEMP]++;
							if(myCh->misc.errCnt[C_CNT_LOWER_TEMP]>= 
											MAX_ERROR_CNT*4){
								myCh->misc.errCnt[C_CNT_LOWER_TEMP] = 0;
								myCh->signal[C_SIG_CONTINUE] = P1;
							}
						}
					}
				}
			}
			addr = addr1 + CREG_CTRL_REG;
			if(!(SREG16_read(addr) & 0x0001)){
				SREG16_write(addr, 0x0001); // auto enable
			}
			//220322_hun
			if(myData->mData.config.swFaultConfig[HARD_VENTING_VALUE] != 0){
				if(myCh->misc.std_gasVoltage >= myData->mData.config.swFaultConfig[HARD_VENTING_VALUE]){
					myCh->op.code = C_FAULT_HARD_VENTING;
				}
			}
			//220104 hun 
			//일시정지 중 Ambient온도 이상일 경우 채널 Code 변경
			if(myData->mData.config.swFaultConfig[AMBIENT_TEMP_MAX] != 0){
				if(myCh->misc.ambientTemp > myData->mData.config.swFaultConfig[AMBIENT_TEMP_MAX]){
					if(myCh->misc.ambient_check_flag == 0){
						myCh->misc.ambient_check_time = myCh->op.runTime;
						myCh->misc.ambient_check_flag = 1;
					}else if(myCh->misc.ambient_check_flag == 1){
						myCh->misc.ambient_check_time += myPs->misc.rt_scan_time;
						if(labs(myCh->misc.ambient_check_time - myCh->op.runTime) >
							myData->mData.config.swFaultConfig[AMBIENT_TEMP_MAX_TIME] * 100){
							myCh->op.code = C_FAULT_AMBIENT_TEMP_MAX;
						}
					}
				}else if(myCh->misc.ambientTemp < myData->mData.config.swFaultConfig[AMBIENT_TEMP_MAX]){
					myCh->misc.ambient_check_flag = 0;
					myCh->misc.ambient_check_time = myCh->op.runTime;
				}
			}

			//190901 lyhw	
			if(myData->mData.config.function[F_PAUSE_DATA_SAVE] == P1){
				pauseSaveDt = myData->DataSave.config.pause_data_time;
				if(pauseSaveDt != 0 
					&& myCh->op.code != C_FAULT_PAUSE_CMD
					&& myCh->op.code != C_PAUSE_UPPER_TEMP_CH
					&& myCh->op.code != C_PAUSE_LOWER_TEMP_CH
					&& myCh->op.code != P_INV_STANDBY){
					myCh->misc.pauseRunTime += myPs->misc.rt_scan_time; 
					if(myCh->misc.pauseRunTime % pauseSaveDt == 0){
						myCh->misc.pauseRunTime = 0;
						if(myCh->ChAttribute.opType == P0) {
							myCh->op.select = SAVE_FLAG_SAVING_PAUSE;
							if(myCh->ChAttribute.chNo_master == 0){
								myData->bData[bd].cData[ch-1].op.select 
									= myCh->op.select;
							}
							send_save_msg(bd, ch, saveDt, 0);
						}
					}
				}
			}
			//211209 hun	
			if(myData->mData.config.sdi_pause_save_flag == P1){
				pauseSaveEndT = myData->mData.config.SDI_pause_save.pause_end_time * 100;
				pauseSaveDt = myData->mData.config.SDI_pause_save.pause_period_time * 100;
				if(pauseSaveDt != 0 && pauseSaveEndT != 0
					&& myCh->op.code != C_FAULT_PAUSE_CMD ){
					myCh->misc.pauseRunTime += myPs->misc.rt_scan_time;
					if(myCh->misc.pauseRunTime <= pauseSaveEndT){
						if(myCh->misc.pauseRunTime % pauseSaveDt == 0){
							if(myCh->ChAttribute.opType == P0) {
								myCh->op.select = SAVE_FLAG_SAVING_TIME;
								if(myCh->ChAttribute.chNo_master == 0){
									myData->bData[bd].cData[ch-1].op.select 
									= myCh->op.select;
								}
								send_save_msg(bd, ch, saveDt, 0);
							}
						}
					}
				}
			}

			//hun_211122_test
			if(myData->mData.config.inv_fault_count != 0 &&
				myCh->misc.inv_auto_continue_count < myData->mData.config.inv_fault_count){
				if(myCh->op.code == P_INV_FAULT_LAG_SHORT || 
					myCh->op.code == P_INV_FAULT_OVER_CURRENT ||
					myCh->op.code == P_INV_FAULT_OVER_VOLTAGE || 
					myCh->op.code == P_INV_FAULT_PRECHARGE_FAIL ||
					myCh->op.code == P_INV_FAULT_OVER_CURRENT2 ||
					myCh->op.code == P_INV_FAULT_CAN_ERR ||
					myCh->op.code == P_INV_FAULT_OVER_LOAD ||
					myCh->op.code == P_INV_FAULT_OVER_HEAT ||
					myCh->op.code == P_INV_FAULT_LOW_VOLTAGE ||
					myCh->op.code == P_INV_FAULT_AC_LOW_VOLTAGE ||
					myCh->op.code == P_INV_FAULT_RESET1 ||
					myCh->op.code == P_INV_FAULT_RESET2 ||
					myCh->op.code == P_INV_FAULT_AC_INPUT_FAIL ||
					myCh->op.code == P_INV_FAULT_AC_OVER_VOLT ||
					myCh->op.code == P_INV_FAULT_HDC_ERROR ||
					myCh->op.code == P_INV_FAULT_ETC){
					myCh->misc.inv_auto_continue_timer += myPs->misc.rt_scan_time;
					if(myCh->misc.inv_auto_continue_timer >= 500){	//5sec
						myCh->misc.inv_auto_continue_timer = 0;
						myCh->misc.inv_auto_continue_count += 1;
						myCh->signal[C_SIG_CONTINUE] = P1;
					}
				}
			}
			if(myCh->signal[C_SIG_STOP] == P1) {
				memset((char *)&myCh->misc.errCnt, 0x00, MAX_SIGNAL);
				myCh->signal[C_SIG_STOP] = P0;
				if(myCh->op.code == C_FAULT_PAUSE_CMD
					&& myCh->opSave.code != C_FAULT_PAUSE_CMD) {
				} else {
					if(myCh->ChAttribute.opType == P0) {
						myCh->op.select = SAVE_FLAG_SAVING_END;
						if(myCh->ChAttribute.chNo_master == 0){
							myData->bData[bd].cData[ch-1].op.select 
								= myCh->op.select;
						}
					//	send_save_msg(bd, ch, saveDt, 0);
					}
				}
				//hun_210917
				myCh->op.code = C_FAULT_STOP_CMD;

				myCh->op.state = C_STANDBY;
				myCh->op.phase = P0;
				myCh->misc.pauseRunTime = 0;	//211209_hun
			} else if(myCh->signal[C_SIG_CONTINUE] == P1) {
				myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
				if(myCh->op.checkDelayTime == 10){
					ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, CMD_PCU_RESET, 0,0); //Reset
				}
				if(myCh->op.checkDelayTime < 20) return;
				//200904 add
				inv_num1 = ch / inv_div;
				inv_num = inv_num1 * inv_div;
				if(myData->bData[bd].cData[inv_num].inv_power == P0){
					myData->bData[bd].cData[inv_num].inv_power = P1;
				}
				if(myData->bData[bd].cData[inv_num].inv_power != P10
					&& myCh->op.checkDelayTime <= 6000) return;
				
				ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, 
										CMD_PCU_RESET, 0,0); //Reset
				
				if(myCh->op.type == STEP_CHARGE){
					if(myCh->ChAttribute.opType == P0) {
						myCh->misc.Drop_maxV = myCh->op.Vsens;	//210916 LJS
					}else if(myCh->ChAttribute.chNo_master == 0){
						myCh->misc.Drop_maxV 
							= myData->bData[bd].cData[ch-1].op.Vsens;
					}
				}else if(myCh->op.type == STEP_DISCHARGE){
					if(myCh->ChAttribute.opType == P0) {
						myCh->misc.Drop_minV = myCh->op.Vsens;	//210916 LJS
					}else if(myCh->ChAttribute.chNo_master == 0){
						myCh->misc.Drop_minV 
							= myData->bData[bd].cData[ch-1].op.Vsens;
					}
				}
				//211209 hun	
				if(myData->mData.config.sdi_pause_save_flag == P1){
					if(myCh->ChAttribute.opType == P0) {
						myCh->op.select = SAVE_FLAG_SAVING_TIME;
						if(myCh->ChAttribute.chNo_master == 0){
							myData->bData[bd].cData[ch-1].op.select 
							= myCh->op.select;
						}
						send_save_msg(bd, ch, saveDt, 0);
					}
				}

				myCh->signal[C_SIG_CONTINUE] = P0;
				myCh->misc.std_gasVoltage = myCh->misc.gasVoltage;	//220322_hun
				myCh->misc.pauseRunTime = 0;	//211209_hun
				myCh->op.checkDelayTime = 0;
				#ifdef _SDI_SAFETY_V1
				myCh->misc.fault_deltaV = myCh->op.Vsens;
				#endif
				#ifdef _SDI_SAFETY_V2
				myCh->misc.Master_recipe_deltaV = 0;
				#endif	
				//211125_hun
				if(myCh->op.type == STEP_CHARGE){
					myCh->misc.charge_cc_hump_flag = P0;	
					myCh->misc.charge_cv_hump_flag = P0;	
					myCh->misc.charge_cc_hump_start_time = myCh->misc.ccTime;
					myCh->misc.charge_cv_hump_start_time = myCh->misc.cvTime;
				}else if(myCh->op.type == STEP_DISCHARGE){
					myCh->misc.discharge_cc_hump_flag = P0;	
					myCh->misc.discharge_cv_hump_flag = P0;	
					myCh->misc.discharge_cc_hump_start_time = myCh->misc.ccTime;
					myCh->misc.discharge_cv_hump_start_time = myCh->misc.cvTime;
				}
				
				memset((char *)&myCh->misc.errCnt, 0x00, MAX_SIGNAL);		
				if(myCh->op.code == C_FAULT_PAUSE_CMD
					&& myCh->opSave.code != C_FAULT_PAUSE_CMD) {
					myCh->signal[C_SIG_RANGE_SWITCH] = P1;
					myCh->op.phase = P6;
				}else if(myCh->op.code == C_FAULT_USER_PATTERN_READ){
					if(myCh->op.type == STEP_REST) {
						myCh->signal[C_SIG_OUT_SWITCH] = P0;
						myCh->signal[C_SIG_OUT_SWITCH_OFF] = P1;
					} else {
						myCh->signal[C_SIG_RANGE_SWITCH] = P1;
					}
					myCh->op.phase = P3;
				} else {
					myCh->misc.fbCountV_H = 0;
					myCh->misc.fbCountI_H = 0;
					myCh->misc.fbSumI_H = 0;
					myCh->misc.fbSumV_H = 0;
					myCh->misc.fbCountV_L = 0;
					myCh->misc.fbCountI_L = 0;
					myCh->misc.fbSumI_L = 0;
					myCh->misc.fbSumV_L = 0;
					myCh->misc.fbCountV_M = 0;
					myCh->misc.fbCountI_M = 0;
					myCh->misc.fbSumI_M = 0;
					myCh->misc.fbSumV_M = 0;
					myCh->misc.sensCount = 0;
					myCh->misc.sensCountFlag = P0;
					myCh->misc.sensBufCount = 0;
					myCh->misc.sensBufCountFlag = P0;
					myCh->misc.fbV = 0;
					myCh->misc.fbI = 0;
					myCh->op.phase = P4;
				}
				if(myCh->op.type == STEP_USER_PATTERN){
					if(myCh->op.code != C_FAULT_USER_PATTERN_READ){
						i = myCh->misc.userPatternCnt;
						if(myData->mData.testCond[bd][ch].userPattern.data[i].data> 0){
							myCh->misc.cmdV_dir = CMD_V_PLUS;
						}else{
							myCh->misc.cmdV_dir = CMD_V_MINUS;
						}
					}
				}
				myCh->ccv[0].avg_v = myCh->misc.tmpVsens;
				myCh->ccv[0].avg_i = myCh->misc.tmpIsens;
			} else if(myCh->signal[C_SIG_NEXTSTEP] == P1) {
				myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
				if(myCh->op.checkDelayTime == 10){
					ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, CMD_PCU_RESET, 0,0); //Reset
				}
				if(myCh->op.checkDelayTime < 20) return;

				//200904 add
				inv_num1 = ch / inv_div;
				inv_num = inv_num1 * inv_div;
				if(myData->bData[bd].cData[inv_num].inv_power == P0){
					myData->bData[bd].cData[inv_num].inv_power = P1;
				}
				if(myData->bData[bd].cData[inv_num].inv_power != P10
					&& myCh->op.checkDelayTime <= 6000) return;	
				
				ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, 
										CMD_PCU_RESET, 0,0); //Reset
				myCh->signal[C_SIG_NEXTSTEP] = P0;
				myCh->misc.pauseRunTime = 0;	//211209_hun
				myCh->op.checkDelayTime = 0;
				memset((char *)&myCh->misc.errCnt, 0x00, MAX_SIGNAL);
				myCh->signal[C_SIG_OUT_SWITCH] = P1;
			//	myCh->signal[C_SIG_OUT_SWITCH_ON] = P1;
				myCh->misc.saveDt = myCh->op.runTime;
				if(myCh->op.code == C_FAULT_PAUSE_CMD
					&& myCh->opSave.code != C_FAULT_PAUSE_CMD) {
				} else {
					myCh->op.code = C_FAULT_NEXTSTEP_CMD;
					//121205 oys add
					if(myCh->op.type != STEP_LOOP) {
						myCh->op.select = SAVE_FLAG_SAVING_END;
						if(myCh->ChAttribute.opType == P0) {
							if(myCh->ChAttribute.chNo_master == 0){
								myData->bData[bd].cData[ch-1].op.select
									= myCh->op.select;
							}
							send_save_msg(bd, ch, saveDt, 0);
						}
					}
				}
				if(type == STEP_LOOP){
					if(myCh->op.code == C_FAULT_CAPACITY_EFFICIENCY){
						myCh->op.phase = P5;
					}else{
						myCh->op.phase = P4;
					}
				}else{
					myCh->op.phase = P5;
				}
			}else if(myCh->signal[C_SIG_RESET] == P1) {
				memset((char *)&myCh->misc.errCnt, 0x00, MAX_SIGNAL);
				memset((char *)&myCh->misc.inv_errCode, 0x00, MAX_INV_NUM);
				memset((char *)&myCh->misc.inv_errCode_AC, 0x00, MAX_INV_NUM);
				memset((char *)&myCh->misc.inv_errFlag, 0x00, MAX_INV_NUM);
				myCh->signal[C_SIG_RESET] = P0;
				myCh->op.state = C_IDLE;
				myCh->op.phase = P0;
				myCh->misc.pauseRunTime = 0;	//211209_hun
			}
	      	myCh->op.checkDelayTime = 0;
			break;
		case P3:
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
			if(myCh->op.checkDelayTime == 50){
				myCh->signal[C_SIG_OUT_SWITCH] = P1;
				//181226 lyhw
				if(myData->mData.config.parallelMode == P1){
					if(myCh->ChAttribute.chNo_master == P0){
						myCh->signal[C_SIG_PARALLEL_SWITCH_ON] = P1;
					}
				}
			}
			if(myCh->op.checkDelayTime >= 150){
				myCh->op.checkDelayTime = 0;
				myCh->op.phase = P7;
			}
			break;
		case P4:
			inv_num1 = ch / inv_div;	//181218
			inv_num = inv_num1 * inv_div;
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
			if(myCh->op.checkDelayTime == 100){
				if(myCh->op.type == STEP_REST) {
					myCh->signal[C_SIG_OUT_SWITCH] = P0;
					myCh->signal[C_SIG_OUT_SWITCH_OFF] = P1;
				} else {
					myCh->signal[C_SIG_RANGE_SWITCH] = P1;
					myCh->signal[C_SIG_OUT_SWITCH] = P1;
				} //khk test 20141008
			}else if(myCh->op.checkDelayTime == 200){
				//181226 lyhw
				if(myData->mData.config.parallelMode == P1){
					if(myCh->ChAttribute.chNo_master == P0){
						myCh->signal[C_SIG_PARALLEL_SWITCH_ON] = P1;
					}
				}
			}else{
				if(myData->bData[bd].cData[inv_num].inv_power == P10){
					if(myCh->op.checkDelayTime >=300){
						myCh->op.checkDelayTime = 0;
						myCh->op.phase = P8;
					}
				}else if(myData->bData[bd].cData[inv_num].inv_power == P99){
					//191024 lyhw
					send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS
								, M_FAIL_PCU_HVU_INPUT_ERROR, ch +1);
					myCh->op.state = C_PAUSE;
					myCh->op.phase = P0;
				}else if(myCh->op.checkDelayTime >= 6000){	//181214	
					myCh->op.state = C_PAUSE;
					myCh->op.phase = P0;
				}
			}
			break;
		case P5:	//next Step signal
			inv_num1 = ch / inv_div;	//181218
			inv_num = inv_num1 * inv_div;
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
			if(myCh->op.checkDelayTime == 50){
				myCh->signal[C_SIG_OUT_SWITCH] = P1;
				//181226 lyhw
				if(myData->mData.config.parallelMode == P1){
					if(myCh->ChAttribute.chNo_master == P0){
						myCh->signal[C_SIG_PARALLEL_SWITCH_ON] = P1;
					}
				}
			//	myCh->signal[C_SIG_OUT_SWITCH_ON] = P1;
			}else{
				if(myData->bData[bd].cData[inv_num].inv_power == P10){
					if(myCh->op.checkDelayTime >= 150){
						myCh->op.checkDelayTime = 0;
						myCh->op.phase = P9;
					}
				}else if(myCh->op.checkDelayTime >= 6000){	//181214	
					myCh->op.state = C_PAUSE;
					myCh->op.phase = P0;
				}
			}
			break;
		case P6:
			inv_num1 = ch / inv_div;	//181218
			inv_num = inv_num1 * inv_div;
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
			if(myCh->op.checkDelayTime == 50){
				myCh->signal[C_SIG_OUT_SWITCH] = P1;
				//181226 lyhw
				if(myData->mData.config.parallelMode == P1){
					if(myCh->ChAttribute.chNo_master == P0){
						myCh->signal[C_SIG_PARALLEL_SWITCH_ON] = P1;
					}
				}
			}else{
				if(myData->bData[bd].cData[inv_num].inv_power == P10){
					if(myCh->op.checkDelayTime >= 150){
						myCh->misc.advStepNo++;
						advStepNo = myCh->misc.advStepNo;
						myCh->op.type 
							= myPs->testCond[bd][ch].step[advStepNo].type;
						myCh->op.mode 
							= myPs->testCond[bd][ch].step[advStepNo].mode;
						myCh->op.stepNo
							= myPs->testCond[bd][ch].step[advStepNo].stepNo;
						myCh->op.checkDelayTime = 0;
						myCh->op.phase = P7;
					}
				}else if(myCh->op.checkDelayTime >= 6000){	//181214	
					myCh->op.state = C_PAUSE;
					myCh->op.phase = P0;
				}
			}
			break;
		case P7:   //Continue Signal
			inv_num1 = ch / inv_div;
			inv_num = inv_num1 * inv_div;
			
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;

			if(myCh->op.checkDelayTime >= 6000){	//after 60 sec	
				myCh->op.state = C_PAUSE;
				myCh->op.phase = P0;
			}
			
			if(myData->bData[bd].cData[inv_num].inv_power == P10){
				if(myCh->op.type == STEP_REST) {
					if(myCh->signal[C_SIG_OUT_SWITCH_OFF] != P3){
						myCh->signal[C_SIG_OUT_SWITCH_OFF] = P1;
					}
				}else{	//181214 lyh w
					if(myCh->signal[C_SIG_OUT_SWITCH_ON] != P3){
						myCh->signal[C_SIG_OUT_SWITCH_ON] = P1;
					}else{
						if(myCh->misc.send_pcu_seq_no
								!= myCh->misc.receive_pcu_seq_no){
							myCh->misc.seq_no_cnt++;
							if(myCh->misc.seq_no_cnt < MAX_SEQ_NO_RETRY){
								myCh->signal[C_SIG_OUT_SWITCH_ON] = P1;
							}
						}else{
							myCh->misc.seq_no_cnt = 0;
						}
					}	
				}
				if(myCh->op.checkDelayTime == 20){
					if(myData->bData[bd].cData[ch].ChAttribute.opType != P1){
						ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST,
									CMD_PCU_MODE_CC, 0 ,0);
					}
				}	
				if(myCh->op.checkDelayTime > 30){	
					myCh->op.state = myCh->misc.tmpState;
					myCh->op.code = myCh->misc.tmpCode;
					myCh->op.checkDelayTime = 0;
					myCh->op.phase = P0;
				}
			}
			break;
		case P8:	//Continue 181217
			inv_num1 = ch / inv_div;
			inv_num = inv_num1 * inv_div;
			if(myData->bData[bd].cData[inv_num].inv_power == P10){
				if(myCh->op.type == STEP_REST) {
					if(myCh->signal[C_SIG_OUT_SWITCH_OFF] != P3){
						myCh->signal[C_SIG_OUT_SWITCH_OFF] = P1;
					}
				}else{		//181214 add
					if(myCh->signal[C_SIG_OUT_SWITCH_ON] != P3){
						myCh->signal[C_SIG_OUT_SWITCH_ON] = P1;
					}else{
						if(myCh->misc.send_pcu_seq_no
								!= myCh->misc.receive_pcu_seq_no){
							myCh->misc.seq_no_cnt++;
							if(myCh->misc.seq_no_cnt < MAX_SEQ_NO_RETRY){
								myCh->signal[C_SIG_OUT_SWITCH_ON] = P1;
							}
						}else{
							myCh->misc.seq_no_cnt = 0;
						}
					}
				}
				if(myCh->op.checkDelayTime == 20){
					if(myData->bData[bd].cData[ch].ChAttribute.opType != P1){
						ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST,
									CMD_PCU_MODE_CC, 0 ,0);
					}
				}	
				if(myCh->op.checkDelayTime > 30){	//after 60 Sec
					myCh->op.state = myCh->misc.tmpState;
					myCh->op.code = myCh->misc.tmpCode;
					myCh->op.checkDelayTime = 0;
				
					if(myCh->misc.waitFlag == P1) //pause waitFlag
					{
						myCh->misc.waitFlag = P0;
						myCh->op.phase = P0;
					}
					if(myCh->misc.chamberWaitFlag == -1) {
						myCh->misc.chamberWaitFlag = P0;
						myCh->op.phase = P0;
					} else {
						myCh->op.phase = P10;
					}
				}
			}
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
			if(myCh->op.checkDelayTime >= 6000){	//after 60 sec	
				myCh->op.state = C_PAUSE;
				myCh->op.phase = P0;
			}
			break;
		case P9:	//next signal
			inv_num1 = ch / inv_div;
			inv_num = inv_num1 * inv_div;
			if(myData->bData[bd].cData[inv_num].inv_power == P10){
				if(myCh->op.checkDelayTime == 10){	//after 60 sec	
					myCh->signal[C_SIG_OUT_SWITCH_ON] = P1;
				}else{
					if(myCh->op.checkDelayTime > 30){	//after 60 Sec
						myCh->op.state = C_RUN;
						myCh->op.phase = P0;
						myCh->op.runTime = 0;
						myCh->op.code = myCh->misc.tmpCode;
						myCh->op.checkDelayTime = 0;

						myCh->misc.advStepNo++;
						advStepNo = myCh->misc.advStepNo;
						myCh->op.type 
							= myPs->testCond[bd][ch].step[advStepNo].type;
						myCh->op.mode 
							= myPs->testCond[bd][ch].step[advStepNo].mode;
						myCh->op.stepNo 
							= myPs->testCond[bd][ch].step[advStepNo].stepNo;
					}
				}
			}
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
			if(myCh->op.checkDelayTime >= 6000){	//after 60 Sec
				myCh->op.state = C_PAUSE;
				myCh->op.phase = P0;
			}
			break;
		default: break;
	}
}

void pStepCharge(int bd, int ch)
{
    int	rtn, rangeI, rangeV, div;
    long val1, val2, val3, refV;
	unsigned long advStepNo, saveDt;
	unsigned char	type, mode;
	double tmp1;
	S_CH_STEP_INFO step;

	myCh = &(myData->bData[bd].cData[ch]);
	step = step_info(bd, ch);

	val1 = val2 = val3 = 0;
	tmp1 = 0.0;
	div = 5;
	type = step.type;
	val1 = step.refV;
	val2 = step.refI;
	
	rangeI = (int)step.rangeI;
	rangeV = (int)step.rangeV;
	saveDt = step.saveDt;
	advStepNo = step.advStepNo;
	mode = step.mode;
	type = step.type;
	
//	if(mode == CP)
//		val1 = step.refP;
	if(mode == CPCV || mode == CP)
		val2 = step.refP;

	switch(myPs->config.hwSpec) {
		default:
			refV = val1;
			break;
	}
						
    switch(myCh->op.phase) {
		case P0:
			initCh(bd, ch);
#if CHAMBER_TEMP_HUMIDITY == 1	//kjc_210419
			rtn = chamber_temp_humidity_check(ch, bd, advStepNo);
#else
			rtn = pcu_temp_wait_flag_check(bd, ch);
#endif
		//	rtn = temp_wait_flag_check(bd, ch);
			//181202 add for Chamber Temp Wait
			if(rtn == 0){
				if(myCh->misc.temp_wait_flag_cnt == 0){
					myCh->misc.temp_wait_flag_cnt++;
					if(myCh->ChAttribute.opType == P0){
						ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, CMD_PCU_MODE_CC, 0,0);
					}
				}
				break;
			}
			myCh->misc.step_count ++;		//kjc_211023
			myCh->misc.cycleStepCount ++;	//kjc_211023
			myCh->misc.endState = P0;
			myCh->misc.groupEndTime = 0;
			myCh->misc.cycleNo = myPs->testCond[bd][ch].step[advStepNo].cycleNo;
			myCh->misc.temp_wait_flag_cnt = 0;
			myCh->op.checkDelayTime = 0;
			myCh->misc.seq_no_cnt = 0;			//180822 add
			myCh->misc.maxV = myCh->misc.tmpVsens;	//180722
			myCh->misc.startV = myCh->misc.tmpVsens;
			myCh->misc.minV = myCh->misc.tmpVsens;
			myCh->misc.maxI = myCh->misc.tmpIsens;
			myCh->misc.minI = myCh->misc.tmpIsens;
			myCh->misc.startT = myCh->op.temp;
			myCh->misc.maxT = myCh->op.temp;
			myCh->misc.minT = myCh->op.temp;
			//190107 add for limit V
			myCh->misc.limit_current_timeout = myCh->op.runTime;
			myCh->misc.hw_fault_temp = myCh->op.runTime;
			myCh->misc.DCR_PreV = myCh->op.Vsens;	//220221 LJS
			//181202 add for Temp Wait Zero Data
			if(myData->DataSave.config.zero_sec_data_save == P1){
				if(myCh->ChAttribute.opType == P0){
					myCh->op.select = SAVE_FLAG_SAVING_TIME;
					if(myCh->ChAttribute.chNo_master == P0){
						myData->bData[bd].cData[ch-1].op.Isens = 0;
						myData->bData[bd].cData[ch-1].misc.tmpIsens = 0;
						myData->bData[bd].cData[ch-1].op.select 
							= myCh->op.select;
					}
					myCh->op.Isens = 0;
					myCh->misc.tmpIsens = 0;
					send_save_msg(bd, ch, 0, 0);
				}
			}
			//120315 kji 0 sec data option
			
			//201015
			if(myCh->signal[C_SIG_OUT_SWITCH_ON] != P3){
				myCh->signal[C_SIG_OUT_SWITCH] = P1;
				myCh->signal[C_SIG_OUT_SWITCH_ON] = P1;
				rtn = 3;
			}else{
				rtn = SelectHwSpec(bd , ch);
			}
			
			if(rtn == 1) {	//180726 lyhw
				myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
				if(myCh->op.checkDelayTime >= 0) {
					myCh->op.checkDelayTime = 0;
					myCh->misc.sensCount = 0;
					myCh->misc.sensCountFlag = P0;
					myCh->misc.sensBufCount = 0;
					myCh->misc.sensBufCountFlag = P0;
					if(myPs->config.capacityType == CAPACITY_CAPACITANCE) {
						pcu_ref_output(bd, ch, type, mode, 
								val1, val2, div, div, rangeV, rangeI);
					} else {
						pcu_ref_output(bd, ch, type, mode, 
								refV, val2, div, div, rangeV, rangeI);
					}
					myCh->op.phase = P50;
				}
			} else if(rtn == 2) {
				ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, 
											CMD_PCU_MODE_CC, 0,0);
				myCh->op.phase = P1;
			}else if(rtn == 3){ //201015
				myCh->op.checkDelayTime = 0;
				myCh->misc.sensCount = 0;
				myCh->misc.sensCountFlag = P0;
				myCh->misc.sensBufCount = 0;
				myCh->misc.sensBufCountFlag = P0;
				myCh->op.phase = P4;
			}else if(rtn == 4){ //210621
				myCh->op.phase = P1;
			}else{
				myCh->op.phase = P1;
			}
			break;
		case P1: 
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
			//180615
			if(myCh->op.checkDelayTime >= 0) {
				myCh->op.checkDelayTime = 0;
				myCh->misc.sensCount = 0;
				myCh->misc.sensCountFlag = P0;
				myCh->misc.sensBufCount = 0;
				myCh->misc.sensBufCountFlag = P0;
				if(myPs->config.capacityType == CAPACITY_CAPACITANCE) {
					pcu_ref_output(bd, ch, type, mode, 
						val1, val2, div, div, rangeV, rangeI);
				} else {
					pcu_ref_output(bd, ch, type, mode, 
							refV, val2, div, div, rangeV, rangeI);
				}
				myCh->op.phase = P50;
			}
			break;
		case P2:
			myCh->op.checkDelayTime = 0;
			myCh->op.phase = P1;
			break;
		case P3:
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
			if(myCh->op.checkDelayTime >= 10) {
				myCh->op.checkDelayTime = 0;
				myCh->op.phase = P1;
			}
			break;
		case P4:
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
			if(myCh->op.checkDelayTime == 40) { //201015
				ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, 
											CMD_PCU_MODE_CC, 0,0);
			}else if(myCh->op.checkDelayTime >= 50) {
				myCh->op.phase = P1;
			}
			break;
		case P5:
			myCh->signal[C_SIG_I_CMD_OUTPUT] = P1;
			myCh->op.phase = P15;
			break;
		case P6:
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
			if(myCh->op.checkDelayTime <= 2) {
				ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, 
											CMD_PCU_MODE_CC, 0,0);
				myCh->signal[C_SIG_OUT_SWITCH] = P1;
				myCh->signal[C_SIG_OUT_SWITCH_ON] = P1;
			} else if(myCh->op.checkDelayTime >= 200) {
				myCh->op.checkDelayTime = 0;
				myCh->op.phase = P15;
			} 
			break;
		case P7:
			if(myData->bData[bd].cData[ch-1].op.phase == P50 &&
				myData->bData[bd].cData[ch-1].op.state == C_RUN){
				myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
				if(myCh->op.checkDelayTime >= 200){
					if(ch==1)
					myCh->op.checkDelayTime = 0;
					myCh->op.phase = P9;
				}
			}
			if(myData->bData[bd].cData[ch-1].op.state != C_RUN){
				myCh->op.phase = P8;
			}
			break;
		case P8:
			if(myPs->config.capacityType == CAPACITY_CAPACITANCE) {
				pcu_ref_output(bd, ch, type, mode, 
								val1, val2, div, div, rangeV, rangeI);
			} else {
				pcu_ref_output(bd, ch, type, mode, 
								refV, val2, div, div, rangeV, rangeI);
			}
			myCh->op.phase = P15;
			break;
		case P9:
			if(myPs->config.capacityType == CAPACITY_CAPACITANCE) {
				pcu_ref_output(bd, ch, type, mode, 
								val1, val2, div, div, rangeV, rangeI);
			} else {
				pcu_ref_output(bd, ch, type, mode, 
								refV, val2, div, div, rangeV, rangeI);
			}
			myCh->op.phase = P15;
			break;
		case P10:
			if(myCh->misc.ac_fail_flag == P1){
				myCh->op.phase = P18; //210419
			} else if(myCh->misc.ac_fail_flag == P0) { 
				rtn = SelectHwSpec(bd , ch);
				if(rtn == 1 || rtn == 2 || rtn == 3 
				|| rtn == 4 || rtn == 5 || rtn == 6) {
					myCh->misc.fbV = 0;
					myCh->misc.fbI = 0;
					myCh->misc.ocv = myCh->op.Vsens;
					myCh->misc.pid_ui1[0] = 0.0;
					myCh->misc.pid_ui1[1] = 0.0;
					myCh->misc.pid_error1[0] = 0.0;
					myCh->misc.pid_error1[1] = 0.0;
					myCh->signal[C_SIG_V_RANGE] = myCh->op.rangeV+1;
					myCh->signal[C_SIG_I_RANGE] = myCh->op.rangeI+1;
					myCh->op.phase = P11;
				} else if(rtn == 7) {
					myCh->misc.fbV = 0;
					myCh->misc.fbI = 0;
					myCh->misc.ocv = myCh->op.Vsens;
					myCh->misc.pid_ui1[0] = 0.0;
					myCh->misc.pid_ui1[1] = 0.0;
					myCh->misc.pid_error1[0] = 0.0;
					myCh->misc.pid_error1[1] = 0.0;
					myCh->op.checkDelayTime = 0;

					myCh->misc.cmd_v = refV;
					myCh->misc.cmd_v_div = (short int)div;
					myCh->misc.cmd_v_range = (short int)rangeV;
					myCh->misc.cmd_i = val2;
					myCh->misc.cmd_i_div = (short int)div;
					myCh->misc.cmd_i_range = (short int)rangeI;
					myCh->signal[C_SIG_V_RANGE] = rangeV+1;
					myCh->signal[C_SIG_I_RANGE] = rangeI+1;
			//		if(myCh->ChAttribute.chNo_master != P0)		//180108
					ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, 
													CMD_PCU_MODE_CC, 0,0);
					myCh->op.phase = P13;
				}
			}
			myCh->op.checkDelayTime = 0;
			break;
		case P11:
			myCh->op.phase = P12;
			break;
		case P12:
			myCh->misc.sensCount = 0;
			myCh->misc.sensCountFlag = P0;
			myCh->misc.sensBufCount = 0;
			myCh->misc.sensBufCountFlag = P0;
			myCh->op.checkDelayTime = 0;
			if(myPs->config.capacityType == CAPACITY_CAPACITANCE) {
				pcu_ref_output(bd, ch, type, mode, 
								val1, val2, div, div, rangeV, rangeI);
			} else {
				pcu_ref_output(bd, ch, type, mode, 
								refV, val2, div, div, rangeV, rangeI);
		//		pcu_ref_output(bd, ch, type, mode, 
		//						val1, val2, div, div, rangeV, rangeI);
			}
			myCh->op.phase = P30;
			break;
		case P13:
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
			if(myCh->op.checkDelayTime == 10) {
				ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, CMD_PCU_MODE_CC, 0,0);
			} else if(myCh->op.checkDelayTime >= 30) {
				myCh->misc.sensCount = 0;
				myCh->misc.sensCountFlag = P0;
				myCh->misc.sensBufCount = 0;
				myCh->misc.sensBufCountFlag = P0;
		
				myCh->op.checkDelayTime = 0;
				myCh->signal[C_SIG_OUT_SWITCH] = P1;
				myCh->signal[C_SIG_OUT_SWITCH_ON] = P1;
				myCh->signal[C_SIG_V_CMD_OUTPUT] = P1;
				myCh->signal[C_SIG_I_CMD_OUTPUT] = P1;
				myCh->op.phase = P50;
			}
			break;
		case P14:
			ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, CMD_PCU_MODE_CC, 0,0);
			myCh->op.phase = P11;
			break;
		case P15:
			//170315 lyh add for 100mS Data
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
			if(myCh->op.checkDelayTime >= 10) {
				myCh->op.checkDelayTime = 0;
				myCh->op.phase = P50;
			}
			break;
		case P18: //210419
			myCh->misc.ac_fail_flag = P0;
			rtn = pcu_temp_wait_flag_check(bd, ch);
			if(rtn == 0){
				if(myCh->misc.temp_wait_flag_cnt == 0){
					myCh->misc.temp_wait_flag_cnt++;
					if(myCh->ChAttribute.opType == P0){
						ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, CMD_PCU_MODE_CC, 0,0);
					}
				}
				break;
			}
			myCh->misc.saveDt = myCh->op.runTime;
			rtn = SelectHwSpec(bd , ch);
			if(rtn == 1 || rtn == 2 || rtn == 3 
			|| rtn == 4 || rtn == 5 || rtn == 6) {
				myCh->misc.fbV = 0;
				myCh->misc.fbI = 0;
				myCh->misc.ocv = myCh->op.Vsens;
				myCh->misc.pid_ui1[0] = 0.0;
				myCh->misc.pid_ui1[1] = 0.0;
				myCh->misc.pid_error1[0] = 0.0;
				myCh->misc.pid_error1[1] = 0.0;
				myCh->signal[C_SIG_V_RANGE] = myCh->op.rangeV+1;
				myCh->signal[C_SIG_I_RANGE] = myCh->op.rangeI+1;
				myCh->op.phase = P11;
			} else if(rtn == 7) {
				myCh->misc.fbV = 0;
				myCh->misc.fbI = 0;
				myCh->misc.ocv = myCh->op.Vsens;
				myCh->misc.pid_ui1[0] = 0.0;
				myCh->misc.pid_ui1[1] = 0.0;
				myCh->misc.pid_error1[0] = 0.0;
				myCh->misc.pid_error1[1] = 0.0;
				myCh->op.checkDelayTime = 0;
				
				myCh->misc.cmd_v = refV;
				myCh->misc.cmd_v_div = (short int)div;
				myCh->misc.cmd_v_range = (short int)rangeV;
				myCh->misc.cmd_i = val2;
				myCh->misc.cmd_i_div = (short int)div;
				myCh->misc.cmd_i_range = (short int)rangeI;
				myCh->signal[C_SIG_V_RANGE] = rangeV+1;
				myCh->signal[C_SIG_I_RANGE] = rangeI+1;
			//	if(myCh->ChAttribute.chNo_master != P0)		//180108
				ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, 
												CMD_PCU_MODE_CC, 0,0);
				myCh->op.phase = P13;
			}
			break;
		case P20:
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
			if(myCh->op.checkDelayTime >= 100) {
				myCh->op.checkDelayTime = 0;
				myCh->op.phase = P0;
			}
			break;
		case P30:
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
			if(myCh->op.checkDelayTime >= 20) {
				myCh->op.checkDelayTime = 0;
				myCh->op.phase = P50;
			}
			break;
		case P49:
			myCh->op.phase = P15;
			break;
		case P50:
			pStepCCCV_Check(bd, ch, val1, val2); //210507
			//180817 add for charge seq error retry
			if(myCh->ChAttribute.opType == P0){		//200120 lyhw
				if(myCh->misc.send_pcu_seq_no != myCh->misc.receive_pcu_seq_no){
					myCh->misc.seq_no_cnt++;		//180822 
					if(myCh->misc.seq_no_cnt < MAX_SEQ_NO_RETRY){
						if(myPs->config.capacityType == CAPACITY_CAPACITANCE) {
							pcu_ref_output(bd, ch, type, mode, 
									val1, val2, div, div, rangeV, rangeI);
						} else {
							pcu_ref_output(bd, ch, type, mode, 
									refV, val2, div, div, rangeV, rangeI);
						}
					}
				}else{
					myCh->misc.seq_no_cnt = 0;		//180822 
				}
			}
			
			myCh->misc.start = 0;
	    	myCh->op.runTime += myPs->misc.rt_scan_time;
			myCh->op.totalRunTime += myPs->misc.rt_scan_time;	//181202
			myCh->misc.cycleRunTime += myPs->misc.rt_scan_time;
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
			
			if(myCh->op.checkDelayTime <= 100){	//180722 add 
				myCh->misc.Pre_change_V = myCh->misc.tmpVsens;	//190901
				myCh->misc.maxV = myCh->misc.tmpVsens;
				myCh->misc.startV = myCh->misc.tmpVsens;
				myCh->misc.minV = myCh->misc.tmpVsens;
				myCh->misc.maxI = myCh->misc.tmpIsens;
				myCh->misc.minI = myCh->misc.tmpIsens;
			}
			
			if(myCh->op.runTime >= 30) {
				myCh->misc.meanCount++;
				if(myCh->misc.meanCount <= 8) {
					myCh->misc.meanSumVolt += (myCh->misc.tmpVsens / 100);
					myCh->misc.meanSumCurr += (myCh->misc.tmpIsens / 100);
				} else {
					myCh->misc.meanSumVolt += (myCh->op.Vsens / 100);
					myCh->misc.meanSumCurr += (myCh->op.Isens / 100);
				}
				myCh->op.meanVolt = (long)(myCh->misc.meanSumVolt
					/ myCh->misc.meanCount) * 100;
				myCh->op.meanCurr = (long)(myCh->misc.meanSumCurr
					/ myCh->misc.meanCount) * 100;
			}

			if(myCh->signal[C_SIG_STOP] == P1) {
				myCh->op.code = C_FAULT_STOP_CMD;
				myCh->op.select = SAVE_FLAG_SAVING_END;
				if(myCh->ChAttribute.opType == P0) {
					if(myCh->ChAttribute.chNo_master == 0){
						myData->bData[bd].cData[ch-1].op.code
							= myCh->op.code;
						myData->bData[bd].cData[ch-1].op.select 
							= myCh->op.select;
					}
					send_save_msg(bd, ch, saveDt, 0);
				}
				myCh->signal[C_SIG_STOP] = P0;
				ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, CMD_PCU_MODE_CC, 0,0);
				myCh->op.phase = P100;
			} else if(myCh->signal[C_SIG_PAUSE] == P1) {
				//cmd, soft fault
				myCh->misc.tmpState = myCh->op.state;
				myCh->misc.tmpCode = myCh->op.code;
				myCh->op.code = C_FAULT_PAUSE_CMD;
				myCh->opSave = myCh->op;
				myCh->signal[C_SIG_PAUSE] = P0;
				ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, CMD_PCU_MODE_CC, 0,0);
				myCh->op.phase = P100;
			} else if(myCh->signal[C_SIG_DLL_STOP] == P1) {
				myCh->op.code = C_FAULT_CELL_DIAGNOSIS_STOP;
				myCh->op.select = SAVE_FLAG_SAVING_END;
				if(myCh->ChAttribute.opType == P0) {
					if(myCh->ChAttribute.chNo_master == 0){
						myData->bData[bd].cData[ch-1].op.code
							= myCh->op.code;
						myData->bData[bd].cData[ch-1].op.select 
							= myCh->op.select;
					}
					//send_save_msg(bd, ch, saveDt, 0);
				}
				myCh->signal[C_SIG_DLL_STOP] = P0;
				ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, CMD_PCU_MODE_CC, 0,0);
				myCh->op.phase = P100;
			} else {
				if(cFailCodeCheck(bd, ch) >= 0) {
				//	cSoftFeedback(bd, ch, val1, val2, val3);
				}
				//180904
				//if(myCh->misc.cvFlag == P1){	
				if(myCh->misc.cvFaultCheckFlag == P1){ //211125_hun
					myCh->misc.cvTime += myPs->misc.rt_scan_time;
				}else{
					myCh->misc.ccTime += myPs->misc.rt_scan_time;
				}
			}

	    	if(myCh->op.checkDelayTime == 100) {
				myCh->misc.maxI = myCh->op.Isens;
				myCh->misc.minI = myCh->op.Isens;
			}
			if(myCh->ChAttribute.opType == P0) {
				cCalculate_Capacitance(bd, ch, advStepNo);
				cCalculate_DCR(bd, ch, advStepNo);
			}
			
			myCh->ccv[0].avg_v = myCh->misc.tmpVsens;
			myCh->ccv[0].avg_i = myCh->misc.tmpIsens;
			if(myCh->op.phase == P100) {
				ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, CMD_PCU_MODE_CC, 0,0);
				cNextStepCheck(bd, ch);
			}
			break;
		case P100:
			break;
		default: break;
    }
}

void pStepDischarge(int bd, int ch)
{
	int rtn;
    long val1, val2, val3, refV;
//	long diff;
	unsigned long advStepNo, saveDt;
	int rangeV, rangeI;
	unsigned char type, mode;
	double tmp1;
	int div = 5;

	S_CH_STEP_INFO step;

	myCh = &(myData->bData[bd].cData[ch]);
	val1 = val2 = val3 = 0;
	tmp1 = 0.0;
	
	step = step_info(bd, ch);

	type = step.type;
	mode = step.mode;
	val1 = step.refV;
	val2 = step.refI;
	rangeI = (int)step.rangeI;
	rangeV = (int)step.rangeV;
	saveDt = step.saveDt;
	advStepNo = step.advStepNo;
	
//	if(mode == CP)
//		val1 = step.refP;
	if(mode == CPCV || mode == CP)
		val2 = step.refP;

	switch(myPs->config.hwSpec){
		default:
			refV = val1;
			break;
	}

    switch(myCh->op.phase) {
		case P0:
			initCh(bd, ch);
#if CHAMBER_TEMP_HUMIDITY == 1	//kjc_210419
			rtn = chamber_temp_humidity_check(ch, bd, advStepNo);
#else
			rtn = pcu_temp_wait_flag_check(bd, ch);
#endif
		//	rtn = temp_wait_flag_check(bd, ch);
			if(rtn == 0){	//181202 add 
				if(myCh->misc.temp_wait_flag_cnt == 0){
					myCh->misc.temp_wait_flag_cnt++;
					if(myCh->ChAttribute.opType == P0){
						ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, CMD_PCU_MODE_CC, 0,0);
					}
				}
				break;
			}
			myCh->misc.step_count ++;		//kjc_211023
			myCh->misc.cycleStepCount ++;	//kjc_211023
			myCh->misc.endState = P0;
			myCh->misc.groupEndTime = 0;
			myCh->misc.cycleNo = myPs->testCond[bd][ch].step[advStepNo].cycleNo;
			myCh->misc.temp_wait_flag_cnt = 0;
			myCh->op.checkDelayTime = 0;
			myCh->misc.seq_no_cnt = 0;		//180822 add
			myCh->misc.maxV = myCh->misc.tmpVsens; //180722
			myCh->misc.startV = myCh->misc.tmpVsens;
			myCh->misc.minV = myCh->misc.tmpVsens;
			myCh->misc.maxI = myCh->misc.tmpIsens;
			myCh->misc.minI = myCh->misc.tmpIsens;
			#ifdef _SDI_SAFETY_V1
			myCh->misc.fault_deltaV = myCh->op.Vsens;
			#endif
			#ifdef _SDI_SAFETY_V2
			myCh->misc.Master_recipe_deltaV = myCh->op.Vsens;
			#endif	
			myCh->misc.startT = myCh->op.temp;
			myCh->misc.maxT = myCh->op.temp;
			myCh->misc.minT = myCh->op.temp;
			//190107 add for limit V
			myCh->misc.limit_current_timeout = myCh->op.runTime;
			myCh->misc.hw_fault_temp = myCh->op.runTime;
			myCh->misc.DCR_PreV = myCh->op.Vsens;	//220221 LJS
			//181202 add for Temp Wait Zero Data
			if(myData->DataSave.config.zero_sec_data_save == P1){
				if(myCh->ChAttribute.opType == P0){
					myCh->op.select = SAVE_FLAG_SAVING_TIME;
					if(myCh->ChAttribute.chNo_master == P0){
						myData->bData[bd].cData[ch-1].op.Isens = 0;
						myData->bData[bd].cData[ch-1].misc.tmpIsens = 0;
						myData->bData[bd].cData[ch-1].op.select 
							= myCh->op.select;
					}
					myCh->op.Isens = 0;
					myCh->misc.tmpIsens = 0;
					send_save_msg(bd, ch, 0, 0);
				}
				/*
				myCh->op.Isens = 0;
				myCh->misc.tmpIsens = 0;
				myCh->op.select = SAVE_FLAG_SAVING_TIME;
				if(myData->bData[bd].cData[ch].ChAttribute.chNo_master == P0){
					send_save_msg(bd, ch-1, 0, 0);
				}else{
					if(myData->bData[bd].cData[ch].ChAttribute.opType == P0){
						send_save_msg(bd, ch, 0, 0);
					}
				}*/
			}
			//120315 kji 0 sec data option	
			//201015
			if(myCh->signal[C_SIG_OUT_SWITCH_ON] != P3){
				myCh->signal[C_SIG_OUT_SWITCH] = P1;
				myCh->signal[C_SIG_OUT_SWITCH_ON] = P1;
				rtn = 3;
			}else{
				rtn = SelectHwSpec(bd , ch);
			}
			
			if(rtn == 1) {		//180726 add
				myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
				if(myCh->op.checkDelayTime >= 0) {
					myCh->op.checkDelayTime = 0;
					myCh->misc.sensCount = 0;
					myCh->misc.sensCountFlag = P0;
					myCh->misc.sensBufCount = 0;
					myCh->misc.sensBufCountFlag = P0;
					if(myPs->config.capacityType == CAPACITY_CAPACITANCE) {
					//	if(val1 < myCh->misc.tmpVsens){
							pcu_ref_output(bd, ch, type, mode, 
									val1, val2, div, div, rangeV, rangeI);
					//	}else{
					//		ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, 
					//									CMD_PCU_MODE_CC, 0,0);
					//		myCh->misc.endFlag = P1;
					//	}
					} else {
					//	if(val1 < myCh->misc.tmpVsens){
							pcu_ref_output(bd, ch, type, mode, 
									val1, val2, div, div, rangeV, rangeI);
					//	}else{
					//		ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, 
					//									CMD_PCU_MODE_CC, 0,0);
					//		myCh->misc.endFlag = P1;
					//	}
					}
					myCh->op.phase = P50;
				}
			} else if(rtn == 2) {
				ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, 
												CMD_PCU_MODE_CC, 0,0);
				myCh->op.phase = P1;
			}else if(rtn == 3){ //201015
				//201015
				myCh->op.checkDelayTime = 0;
				myCh->misc.sensCount = 0;
				myCh->misc.sensCountFlag = P0;
				myCh->misc.sensBufCount = 0;
				myCh->misc.sensBufCountFlag = P0;
				myCh->op.phase = P4;
			}else if(rtn == 4){ //210621
				myCh->op.phase = P1;
			}else{
				myCh->op.phase = P1;
			}
			break;
		case P1:
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
			//180615
			if(myCh->op.checkDelayTime >= 0) {
				myCh->op.checkDelayTime = 0;
				myCh->misc.sensCount = 0;
				myCh->misc.sensCountFlag = P0;
				myCh->misc.sensBufCount = 0;
				myCh->misc.sensBufCountFlag = P0;
				if(myPs->config.capacityType == CAPACITY_CAPACITANCE) {
					pcu_ref_output(bd, ch, type, mode, 
								val1, val2, div, div, rangeV, rangeI);
				} else {
					pcu_ref_output(bd, ch, type, mode, 
								val1, val2, div, div, rangeV, rangeI);
				}
				myCh->op.phase = P50;
			}
			break;
		case P2:
			myCh->op.phase = P1;
			myCh->op.checkDelayTime = 0;
			break;
		case P3:
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
			if(myCh->op.checkDelayTime >= 10) {
				myCh->op.checkDelayTime = 0;
				myCh->op.phase = P1;
			}
			break;
		case P4:
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
			if(myCh->op.checkDelayTime == 40) { //201015
				ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, 
											CMD_PCU_MODE_CC, 0,0);
			}else if(myCh->op.checkDelayTime >= 50) {
				myCh->op.phase = P1;
			}
			break;
		case P5:
			myCh->signal[C_SIG_I_CMD_OUTPUT] = P1;
			myCh->op.phase = P15;
			break;
		case P6:
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
			if(myCh->op.checkDelayTime >= 4) {
				myCh->op.checkDelayTime = 0;
				myCh->signal[C_SIG_OUT_SWITCH] = P1;
		//		if(myCh->ChAttribute.chNo_master != P0)		//180108
				myCh->signal[C_SIG_OUT_SWITCH_ON] = P1;
				myCh->signal[C_SIG_I_CMD_OUTPUT] = P1;
				myCh->signal[C_SIG_V_CMD_OUTPUT] = P1;
				myCh->op.phase = P15;
			} 
			break;
		case P10:
			if(myCh->misc.ac_fail_flag == P1){ //210419
				myCh->op.phase = P18;
			} else if(myCh->misc.ac_fail_flag == P0) {
				rtn = SelectHwSpec(bd , ch);
				if(rtn == 1
					|| rtn == 2
					|| rtn == 3
					|| rtn == 4
					|| rtn == 5
					|| rtn == 6
					|| rtn == 7) {
					myCh->signal[C_SIG_V_RANGE] = rangeV+1;
					myCh->signal[C_SIG_I_RANGE] = rangeI+1;

					myCh->misc.fbV = 0;
					myCh->misc.fbI = 0;
					myCh->misc.ocv = myCh->op.Vsens;
					myCh->misc.pid_ui1[0] = 0.0;
					myCh->misc.pid_ui1[1] = 0.0;
					myCh->misc.pid_error1[0] = 0.0;
					myCh->misc.pid_error1[1] = 0.0;
					myCh->op.phase = P11;
				}
			}
			break;
		case P11:
			myCh->op.phase = P12;
			break;
		case P12:
			myCh->misc.sensCount = 0;
			myCh->misc.sensCountFlag = P0;
			myCh->misc.sensBufCount = 0;
			myCh->misc.sensBufCountFlag = P0;
			myCh->op.checkDelayTime = 0;
			if(myPs->config.capacityType == CAPACITY_CAPACITANCE) {
				pcu_ref_output(bd, ch, type, mode, val1, val2, div, div, rangeV, rangeI);
			} else {
				pcu_ref_output(bd, ch, type, mode, val1, val2, div, div, rangeV, rangeI);
			}
			myCh->op.phase = P30;
			break;
		case P15:
			//170315 lyh add for 100mS Data
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
			if(myCh->op.checkDelayTime >= 10) {
				myCh->op.checkDelayTime = 0;
				myCh->op.phase = P50;
			}
			break;
		case P18: //210419
			myCh->misc.ac_fail_flag = P0;
			rtn = pcu_temp_wait_flag_check(bd, ch);
			if(rtn == 0){
				if(myCh->misc.temp_wait_flag_cnt == 0){
					myCh->misc.temp_wait_flag_cnt++;
					if(myCh->ChAttribute.opType == P0){
						ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, CMD_PCU_MODE_CC, 0,0);
					}
				}
				break;
			}
			myCh->misc.saveDt = myCh->op.runTime;
			rtn = SelectHwSpec(bd , ch);
			if(rtn == 1 || rtn == 2 || rtn == 3 
			|| rtn == 4 || rtn == 5 || rtn == 6) {
				myCh->misc.fbV = 0;
				myCh->misc.fbI = 0;
				myCh->misc.ocv = myCh->op.Vsens;
				myCh->misc.pid_ui1[0] = 0.0;
				myCh->misc.pid_ui1[1] = 0.0;
				myCh->misc.pid_error1[0] = 0.0;
				myCh->misc.pid_error1[1] = 0.0;
				myCh->signal[C_SIG_V_RANGE] = myCh->op.rangeV+1;
				myCh->signal[C_SIG_I_RANGE] = myCh->op.rangeI+1;
				myCh->op.phase = P11;
			} else if(rtn == 7) {
				myCh->misc.fbV = 0;
				myCh->misc.fbI = 0;
				myCh->misc.ocv = myCh->op.Vsens;
				myCh->misc.pid_ui1[0] = 0.0;
				myCh->misc.pid_ui1[1] = 0.0;
				myCh->misc.pid_error1[0] = 0.0;
				myCh->misc.pid_error1[1] = 0.0;
				myCh->op.checkDelayTime = 0;
				
				myCh->misc.cmd_v = refV;
				myCh->misc.cmd_v_div = (short int)div;
				myCh->misc.cmd_v_range = (short int)rangeV;
				myCh->misc.cmd_i = val2;
				myCh->misc.cmd_i_div = (short int)div;
				myCh->misc.cmd_i_range = (short int)rangeI;
				myCh->signal[C_SIG_V_RANGE] = rangeV+1;
				myCh->signal[C_SIG_I_RANGE] = rangeI+1;
			//	if(myCh->ChAttribute.chNo_master != P0)		//180108
				ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, 
												CMD_PCU_MODE_CC, 0,0);
				myCh->op.phase = P13;
			}
			break;
		case P20:
			//111130 kji run relay check 
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
			if(myCh->op.checkDelayTime >= 100) {
				myCh->op.checkDelayTime = 0;
				myCh->op.phase = P0;
			}
			break;
		case P30:
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
			if(myCh->op.checkDelayTime >= 20) {
				myCh->op.checkDelayTime = 0;
				myCh->op.phase = P50;
			}
			break;
		case P49:
			myCh->op.phase = P50;
		//	myCh->op.phase = P15;
			break;
		case P50:
			pStepCCCV_Check(bd, ch, val1, val2); //210507
			//180817 dischage seq error retry
			if(myCh->ChAttribute.opType == P0){		//200120 lyhw
				if(myCh->misc.send_pcu_seq_no != myCh->misc.receive_pcu_seq_no){
					myCh->misc.seq_no_cnt++;		//180822 
					if(myCh->misc.seq_no_cnt < MAX_SEQ_NO_RETRY){
						if(myPs->config.capacityType == CAPACITY_CAPACITANCE) {
							pcu_ref_output(bd, ch, type, mode, 
										val1, val2, div, div, rangeV, rangeI);
						} else {
							pcu_ref_output(bd, ch, type, mode, 
										val1, val2, div, div, rangeV, rangeI);
						}
					}
				}else{
					myCh->misc.seq_no_cnt = 0;		//180822 
				}
			}
			
			myCh->misc.start = 0;
	    	myCh->op.runTime += myPs->misc.rt_scan_time;
			myCh->op.totalRunTime += myPs->misc.rt_scan_time;	//181202
	   		myCh->misc.cycleRunTime += myPs->misc.rt_scan_time;
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
			
			if(myCh->op.checkDelayTime < 100){	//180722
				myCh->misc.Pre_change_V = myCh->misc.tmpVsens;	//190901
				myCh->misc.maxV = myCh->misc.tmpVsens;
				myCh->misc.startV = myCh->misc.tmpVsens;
				myCh->misc.minV = myCh->misc.tmpVsens;
				myCh->misc.maxI = myCh->misc.tmpIsens;
				myCh->misc.minI = myCh->misc.tmpIsens;
			}
			if(myCh->op.runTime >= 30) {
				myCh->misc.meanCount++;
				if(myCh->misc.meanCount <= 8) {
					myCh->misc.meanSumVolt += (myCh->misc.tmpVsens / 100);
					myCh->misc.meanSumCurr += (myCh->misc.tmpIsens / 100);
				} else {
					myCh->misc.meanSumVolt += (myCh->op.Vsens / 100);
					myCh->misc.meanSumCurr += (myCh->op.Isens / 100);
				}
				myCh->op.meanVolt = (long)(myCh->misc.meanSumVolt
					/ myCh->misc.meanCount) * 100;
				myCh->op.meanCurr = (long)(myCh->misc.meanSumCurr
					/ myCh->misc.meanCount) * 100;
			}
/*			if(myCh->op.checkDelayTime == myPs->misc.rt_scan_time) {
//				pcu_ref_output(bd, ch, type, mode, val1, val2, div, div, rangeV, rangeI);
			}*/
			if(myCh->signal[C_SIG_STOP] == P1) {
				myCh->op.code = C_FAULT_STOP_CMD;
				myCh->op.select = SAVE_FLAG_SAVING_END;
				if(myCh->ChAttribute.opType == P0) {
					if(myCh->ChAttribute.chNo_master == 0){
						myData->bData[bd].cData[ch-1].op.code
							= myCh->op.code;
						myData->bData[bd].cData[ch-1].op.select 
							= myCh->op.select;
					}
					send_save_msg(bd, ch, saveDt, 0);
				}
				myCh->signal[C_SIG_STOP] = P0;
				ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, CMD_PCU_MODE_CC, 0,0);
				myCh->op.phase = P100;
			} else if(myCh->signal[C_SIG_PAUSE] == P1) {
				//cmd, soft fault
				myCh->misc.tmpState = myCh->op.state;
				myCh->misc.tmpCode = myCh->op.code;
				myCh->op.code = C_FAULT_PAUSE_CMD;
				myCh->opSave
					= myCh->op;
				myCh->signal[C_SIG_PAUSE] = P0;
				ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, CMD_PCU_MODE_CC, 0,0);
				myCh->op.phase = P100;
			} else if(myCh->signal[C_SIG_DLL_STOP] == P1) {
				myCh->op.code = C_FAULT_CELL_DIAGNOSIS_STOP;
				myCh->op.select = SAVE_FLAG_SAVING_END;
				if(myCh->ChAttribute.opType == P0) {
					if(myCh->ChAttribute.chNo_master == 0){
						myData->bData[bd].cData[ch-1].op.code
							= myCh->op.code;
						myData->bData[bd].cData[ch-1].op.select 
							= myCh->op.select;
					}
					//send_save_msg(bd, ch, saveDt, 0);
				}
				myCh->signal[C_SIG_DLL_STOP] = P0;
				ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, CMD_PCU_MODE_CC, 0,0);
				myCh->op.phase = P100;
			} else {
				if(cFailCodeCheck(bd, ch) >= 0) {
				//	cSoftFeedback(bd, ch, val1, val2, val3);
				}
				//180904
				//if(myCh->misc.cvFlag == P1){	
				if(myCh->misc.cvFaultCheckFlag == P1){ //211125_hun
					myCh->misc.cvTime += myPs->misc.rt_scan_time;
				}else{
					myCh->misc.ccTime += myPs->misc.rt_scan_time;
				}
			}

	    	if(myCh->op.checkDelayTime == 100) {
				myCh->misc.maxI = myCh->op.Isens;
				myCh->misc.minI = myCh->op.Isens;
			}
			if(myCh->ChAttribute.opType == P0){
				cCalculate_Capacitance(bd, ch, advStepNo);
				cCalculate_DCR(bd, ch, advStepNo);
			}
			myCh->ccv[0].avg_v = myCh->misc.tmpVsens;
			myCh->ccv[0].avg_i = myCh->misc.tmpIsens;
			if(myCh->op.phase == P100){
				ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, CMD_PCU_MODE_CC, 0,0);
				cNextStepCheck(bd, ch);
			}
			break;
		case P100:
			break;
		default: break;
    }
}

void pStepRest(int bd, int ch)
{
	int i, rtn = 0;
	unsigned long advStepNo, saveDt;
	int rangeI, rangeV,type;

	S_CH_STEP_INFO step;

	myCh = &(myData->bData[bd].cData[ch]);
	step = step_info(bd, ch);
	type = step.type;
    advStepNo = step.advStepNo;
	rangeV = step.rangeV;
	rangeI = step.rangeI;
	saveDt = step.saveDt;
    switch(myCh->op.phase) {
		case P0:
			initCh(bd, ch);	
#if CHAMBER_TEMP_HUMIDITY == 1	//kjc_210419
			rtn = chamber_temp_humidity_check(ch, bd, advStepNo);
#else
			rtn = pcu_temp_wait_flag_check(bd, ch);
#endif
		//	rtn = temp_wait_flag_check(bd, ch);
			if(rtn == 0){	//181202 add 
				if(myCh->misc.temp_wait_flag_cnt == 0){
					myCh->misc.temp_wait_flag_cnt++;
					if(myCh->ChAttribute.opType == P0){
						ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, CMD_PCU_MODE_CC, 0,0);
					}
				}
				break;
			}
			myCh->misc.step_count ++;		//kjc_211023
			myCh->misc.cycleStepCount ++;	//kjc_211023
			myCh->misc.endState = P0;
			myCh->misc.groupEndTime = 0;
			// 111212 oys w : cycleNo
			myCh->misc.cycleNo = myPs->testCond[bd][ch].step[advStepNo].cycleNo;
			myCh->misc.temp_wait_flag_cnt = 0;
			myCh->op.checkDelayTime = 0;
			myCh->misc.seq_no_cnt = 0;		//180822 add
			myCh->misc.maxV = myCh->misc.tmpVsens; //180722
			myCh->misc.startV = myCh->misc.tmpVsens;
			myCh->misc.minV = myCh->misc.tmpVsens;
			myCh->misc.maxI = myCh->misc.tmpIsens;
			myCh->misc.minI = myCh->misc.tmpIsens;
			myCh->misc.startT = myCh->op.temp;
			myCh->misc.maxT = myCh->op.temp;
			myCh->misc.minT = myCh->op.temp;
			myCh->misc.hw_fault_temp = myCh->op.runTime;
			//181202 add for Temp Wait Zero Data
			if(myData->DataSave.config.zero_sec_data_save == P1){
				if(myCh->ChAttribute.opType == P0){
					myCh->op.select = SAVE_FLAG_SAVING_TIME;
					if(myCh->ChAttribute.chNo_master == P0){
						myData->bData[bd].cData[ch-1].op.Isens = 0;
						myData->bData[bd].cData[ch-1].misc.tmpIsens = 0;
						myData->bData[bd].cData[ch-1].op.select 
							= myCh->op.select;
					}
					myCh->op.Isens = 0;
					myCh->misc.tmpIsens = 0;
					send_save_msg(bd, ch, 0, 0);
				}
				/*
				myCh->op.Isens = 0;
				myCh->misc.tmpIsens = 0;
				myCh->op.select = SAVE_FLAG_SAVING_TIME;
				if(myData->bData[bd].cData[ch].ChAttribute.chNo_master == P0){
					send_save_msg(bd, ch-1, 0, 0);
				}else{
					if(myData->bData[bd].cData[ch].ChAttribute.opType == P0){
						send_save_msg(bd, ch, 0, 0);
					}
				}*/
			}
			//120315 kji 0 sec data option
			rtn = SelectHwSpec(bd , ch);
			if(rtn == 2) {
				myCh->signal[C_SIG_OUT_SWITCH] = P0;
				myCh->signal[C_SIG_OUT_SWITCH_OFF] = P1;
			}
			//180727 add for parallel rest CC0
			if(myData->bData[bd].cData[ch].ChAttribute.opType != P1){
				ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, CMD_PCU_MODE_CC, 0,0);
			}
	    	myCh->op.phase = P3;
			break;
		case P1:
			myCh->op.select = SAVE_FLAG_SAVING_TIME;
			send_save_msg(bd, ch, 0, 0);
	    	myCh->op.phase = P50;
			break;
		case P2:
			if(myData->bData[bd].cData[ch].ChAttribute.opType != P1){
				ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, CMD_PCU_MODE_CC, 0,0);
		   		myCh->op.phase = P3;
			}
			break;
		case P3:
			//170221 lyh add for Relay Check			
			if(myPs->testCond[bd][ch].step[advStepNo].endT != 0){
				if(myPs->testCond[bd][ch].step[advStepNo].endT >= 6000){
					myCh->signal[C_SIG_OUT_SWITCH_OFF] = P1;
				}
			}
	    	myCh->op.phase = P50;
			break;
		case P10:
			if(myCh->misc.ac_fail_flag == P1){
				myCh->op.phase = P18;
			} else if(myCh->misc.ac_fail_flag == P0) {
	    		myCh->op.runTime += myPs->misc.rt_scan_time;
				myCh->op.totalRunTime += myPs->misc.rt_scan_time;	//181202
	    		myCh->op.phase = P50;
			}
	    	break;
		case P18:
			myCh->misc.ac_fail_flag = P0;
			rtn = pcu_temp_wait_flag_check(bd, ch);
			if(rtn == 0){	//181202 add 
				if(myCh->misc.temp_wait_flag_cnt == 0){
					myCh->misc.temp_wait_flag_cnt++;
					if(myCh->ChAttribute.opType == P0){
						ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, CMD_PCU_MODE_CC, 0,0);
					}
				}
				break;
			}
			myCh->misc.saveDt = myCh->op.runTime;
	    	myCh->op.runTime += myPs->misc.rt_scan_time;
			myCh->op.totalRunTime += myPs->misc.rt_scan_time;	//181202
	    	myCh->op.phase = P50;
			break;
		case P50:
			//180817 add Rest seq error retry
			if(myCh->ChAttribute.opType == P0){		//200120 lyhw
				if(myCh->misc.send_pcu_seq_no != myCh->misc.receive_pcu_seq_no){
					myCh->misc.seq_no_cnt++;		//180822 
					if(myCh->misc.seq_no_cnt < MAX_SEQ_NO_RETRY){
						ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, 
													CMD_PCU_MODE_CC, 0,0);
					}
				}else{
					myCh->misc.seq_no_cnt = 0;		//180822 
				}
			}
			
			myCh->misc.start = 0;
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;

			if(myCh->op.checkDelayTime <= 100){
				myCh->misc.Pre_change_V = myCh->misc.tmpVsens; //190901
				myCh->misc.maxV = myCh->misc.tmpVsens; //180722
				myCh->misc.startV = myCh->misc.tmpVsens;
				myCh->misc.minV = myCh->misc.tmpVsens;
				myCh->misc.maxI = myCh->misc.tmpIsens;
				myCh->misc.minI = myCh->misc.tmpIsens;
			}
			if(myCh->misc.nextDelay < P3 || myCh->misc.nextDelay == P5) {
	    		myCh->op.runTime += myPs->misc.rt_scan_time;
				myCh->op.totalRunTime += myPs->misc.rt_scan_time;	//181202
				myCh->misc.cycleRunTime += myPs->misc.rt_scan_time;
			}
			
			if(myCh->op.runTime >= 30) {
				myCh->misc.meanCount++;
				if(myCh->misc.meanCount <= 8) {
					myCh->misc.meanSumVolt += (myCh->misc.tmpVsens / 100);
					myCh->misc.meanSumCurr += (myCh->misc.tmpIsens / 100);
				} else {
					myCh->misc.meanSumVolt += (myCh->op.Vsens / 100);
					myCh->misc.meanSumCurr += (myCh->op.Isens / 100);
				}
				myCh->op.meanVolt = (long)(myCh->misc.meanSumVolt
					/ myCh->misc.meanCount) * 100;
				myCh->op.meanCurr = (long)(myCh->misc.meanSumCurr
					/ myCh->misc.meanCount) * 100;
			}
						
			if(myCh->signal[C_SIG_STOP] == P1) {
				myCh->op.code = C_FAULT_STOP_CMD;
				myCh->op.select = SAVE_FLAG_SAVING_END;
				if(myCh->ChAttribute.opType == P0) {
					if(myCh->ChAttribute.chNo_master == 0){
						myData->bData[bd].cData[ch-1].op.code
							= myCh->op.code;
						myData->bData[bd].cData[ch-1].op.select 
							= myCh->op.select;
					}
					send_save_msg(bd, ch, saveDt, 0);
				}
				myCh->signal[C_SIG_STOP] = P0;
				myCh->op.phase = P100;
			} else if(myCh->signal[C_SIG_PAUSE] == P1) {
				myCh->misc.tmpState = myCh->op.state;
				myCh->misc.tmpCode = myCh->op.code;
				myCh->op.code = C_FAULT_PAUSE_CMD;
				myCh->opSave = myCh->op;
				myCh->signal[C_SIG_PAUSE] = P0;
				myCh->op.phase = P100;
			} else if(myCh->signal[C_SIG_DLL_STOP] == P1) {
				myCh->op.code = C_FAULT_CELL_DIAGNOSIS_STOP;
				myCh->op.select = SAVE_FLAG_SAVING_END;
				if(myCh->ChAttribute.opType == P0) {
					if(myCh->ChAttribute.chNo_master == 0){
						myData->bData[bd].cData[ch-1].op.code
							= myCh->op.code;
						myData->bData[bd].cData[ch-1].op.select 
							= myCh->op.select;
					}
					//send_save_msg(bd, ch, saveDt, 0);
				}
				myCh->signal[C_SIG_DLL_STOP] = P0;
				myCh->op.phase = P100;
			} else {
				i = cFailCodeCheck(bd, ch);
			}
			
			myCh->ccv[0].avg_v = myCh->misc.tmpVsens;
			myCh->ccv[0].avg_i = myCh->misc.tmpIsens;
			if(myCh->op.phase == P100){
				cNextStepCheck(bd, ch);
			}
	    	break;
		case P100:
			break;
		default: break;
    }
}

void pStepOcv(int bd, int ch)
{
	int i, rangeV, rangeI, rtn = 0;
	unsigned long advStepNo, saveDt;

	S_CH_STEP_INFO step;
	myCh = &(myData->bData[bd].cData[ch]);
	step = step_info(bd, ch);

    advStepNo = step.advStepNo;
	rangeV = step.rangeV;
	rangeI = step.rangeI;
	saveDt = step.saveDt;

    switch(myCh->op.phase) {
		case P0:
			myCh->misc.step_count ++;		//kjc_211023
			myCh->misc.cycleStepCount ++;	//kjc_211023
			myCh->misc.endState = P0;
			myCh->misc.groupEndTime = 0;
			// 111212 oys w : cycleNo
			myCh->misc.cycleNo = myPs->testCond[bd][ch].step[advStepNo].cycleNo;

			initCh(bd, ch);
			myCh->misc.maxV = myCh->misc.tmpVsens; //180722
			myCh->misc.startV = myCh->misc.tmpVsens;
			myCh->misc.minV = myCh->misc.tmpVsens;
			myCh->misc.maxI = myCh->misc.tmpIsens;
			myCh->misc.minI = myCh->misc.tmpIsens;
		/*	myCh->misc.maxV = myCh->op.Vsens;
			myCh->misc.startV = myCh->op.Vsens;
			myCh->misc.minV = myCh->op.Vsens;*/
			myCh->misc.startT = myCh->op.temp;
			myCh->misc.maxT = myCh->op.temp;
			myCh->misc.minT = myCh->op.temp;
			//181202 add for Temp Wait Zero Data
			if(myData->DataSave.config.zero_sec_data_save == P1){
				if(myCh->ChAttribute.opType == P0){
					myCh->op.select = SAVE_FLAG_SAVING_TIME;
					if(myCh->ChAttribute.chNo_master == P0){
						myData->bData[bd].cData[ch-1].op.Isens = 0;
						myData->bData[bd].cData[ch-1].misc.tmpIsens = 0;
						myData->bData[bd].cData[ch-1].op.select 
							= myCh->op.select;
					}
					myCh->op.Isens = 0;
					myCh->misc.tmpIsens = 0;
					send_save_msg(bd, ch, 0, 0);
					
				}
				/*
				myCh->op.Isens = 0;
				myCh->misc.tmpIsens = 0;
				myCh->op.select = SAVE_FLAG_SAVING_TIME;
				if(myData->bData[bd].cData[ch].ChAttribute.chNo_master == P0){
					send_save_msg(bd, ch-1, 0, 0);
				}else{
					if(myData->bData[bd].cData[ch].ChAttribute.opType == P0){
						send_save_msg(bd, ch, 0, 0);
					}
				}*/
			}
			rtn = SelectHwSpec(bd , ch);
			if(rtn == 2) {
		//	if(rtn == 2) {		//180726
				myCh->signal[C_SIG_OUT_SWITCH] = P0;
				myCh->signal[C_SIG_OUT_SWITCH_OFF] = P1;
			}
			ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, CMD_PCU_MODE_CC, 0,0);
			myCh->misc.start = 0;
	    	myCh->op.phase = P50;
			break;
		case P10:
			if(myCh->misc.ac_fail_flag == P1){ //210419
				myCh->op.phase = P18;
			} else if(myCh->misc.ac_fail_flag == P0) {
				ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, CMD_PCU_MODE_CC, 0,0);
				myCh->misc.start = 0;
	    		myCh->op.phase = P50;
			}
	    	break;
		case P18: //210419
			myCh->misc.ac_fail_flag = P0;
			ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, CMD_PCU_MODE_CC, 0,0);
			myCh->misc.start = 0;
	    	myCh->op.phase = P50;
	    	break;
		case P50:
			if(myCh->op.checkDelayTime < 100){
				myCh->misc.Pre_change_V = myCh->misc.tmpVsens; //190901
				myCh->misc.maxV = myCh->misc.tmpVsens; //180722
				myCh->misc.startV = myCh->misc.tmpVsens;
				myCh->misc.minV = myCh->misc.tmpVsens;
				myCh->misc.maxI = myCh->misc.tmpIsens;
				myCh->misc.minI = myCh->misc.tmpIsens;
			}
	    	myCh->op.runTime += myPs->misc.rt_scan_time;
//			myCh->misc.cycleRunTime += myPs->misc.rt_scan_time;
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
			if(myCh->op.runTime >= 30) {
				myCh->misc.meanCount++;
				if(myCh->misc.meanCount <= 8) {
					myCh->misc.meanSumVolt += (myCh->misc.tmpVsens / 100);
					myCh->misc.meanSumCurr += (myCh->misc.tmpIsens / 100);
				} else {
					myCh->misc.meanSumVolt += (myCh->op.Vsens / 100);
					myCh->misc.meanSumCurr += (myCh->op.Isens / 100);
				}
				myCh->op.meanVolt = (long)(myCh->misc.meanSumVolt
					/ myCh->misc.meanCount) * 100;
				myCh->op.meanCurr = (long)(myCh->misc.meanSumCurr
					/ myCh->misc.meanCount) * 100;
			}
			if(myCh->signal[C_SIG_STOP] == P1) {
				myCh->op.code = C_FAULT_STOP_CMD;
				myCh->op.select = SAVE_FLAG_SAVING_END;
				if(myCh->ChAttribute.opType == P0) {
					if(myCh->ChAttribute.chNo_master == 0){
						myData->bData[bd].cData[ch-1].op.code
							= myCh->op.code;
						myData->bData[bd].cData[ch-1].op.select 
							= myCh->op.select;
					}
					send_save_msg(bd, ch, saveDt, 0);
				}
				myCh->signal[C_SIG_STOP] = P0;
				myCh->op.phase = P100;
			} else if(myCh->signal[C_SIG_PAUSE] == P1) {
				myCh->misc.tmpState = myCh->op.state;
				myCh->misc.tmpCode = myCh->op.code;
				myCh->op.code = C_FAULT_PAUSE_CMD;
				myCh->opSave
					= myCh->op;
				myCh->signal[C_SIG_PAUSE] = P0;
				myCh->op.phase = P100;
			} else if(myCh->signal[C_SIG_DLL_STOP] == P1) {
				myCh->op.code = C_FAULT_CELL_DIAGNOSIS_STOP;
				myCh->op.select = SAVE_FLAG_SAVING_END;
				if(myCh->ChAttribute.opType == P0) {
					if(myCh->ChAttribute.chNo_master == 0){
						myData->bData[bd].cData[ch-1].op.code
							= myCh->op.code;
						myData->bData[bd].cData[ch-1].op.select 
							= myCh->op.select;
					}
					//send_save_msg(bd, ch, saveDt, 0);
				}
				myCh->signal[C_SIG_DLL_STOP] = P0;
				myCh->op.phase = P100;
			} else {
				i = cFailCodeCheck(bd, ch);
			}

			myCh->ccv[0].avg_v = myCh->misc.tmpVsens;
			myCh->ccv[0].avg_i = myCh->misc.tmpIsens;
			if(myCh->op.phase == P100){
				cNextStepCheck(bd, ch);
			}
	    	break;
		case P100:
			break;
		default: break;
    }
}

void pStepZ(int bd, int ch)
{
	int rtn;
    long val1, val2, val3, refV;
//	long diff;
	unsigned long advStepNo, saveDt;
	int rangeV, rangeI;
	double tmp1, tmp2;
	unsigned char	type, mode;
	int div = 5;

	S_CH_STEP_INFO step;

	myCh = &(myData->bData[bd].cData[ch]);
	step = step_info(bd, ch);

	val1 = val2 = val3 = 0;
	tmp1 = tmp2 = 0.0;

    advStepNo = step.advStepNo;
	rangeV = step.rangeV;
	rangeI = step.rangeI;
	saveDt = step.saveDt;
	type = step.type;
	mode = step.mode;

	val1 = step.refV;
	val2 = step.refI;
	refV = val1;
	switch(myPs->config.hwSpec){
		default:
			refV = val1;
			break;
	}
    switch(myCh->op.phase) {
		case P0:
			initCh(bd, ch);
#if CHAMBER_TEMP_HUMIDITY == 1	//kjc_210419
			rtn = chamber_temp_humidity_check(ch, bd, advStepNo);
#else
			rtn = pcu_temp_wait_flag_check(bd, ch);
#endif
		//	rtn = temp_wait_flag_check(bd, ch);
			if(rtn == 0) {
				if(myCh->misc.temp_wait_flag_cnt == 0){
					myCh->misc.temp_wait_flag_cnt++;
					if(myCh->ChAttribute.opType == P0){
						ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, CMD_PCU_MODE_CC, 0,0);
					}
				}
				break;
			}
			myCh->misc.step_count ++;		//kjc_211023
			myCh->misc.cycleStepCount ++;	//kjc_211023
			myCh->misc.endState = P0;
			myCh->misc.groupEndTime = 0;
			// 111212 oys w : cycleNo
			myCh->misc.cycleNo = myPs->testCond[bd][ch].step[advStepNo].cycleNo;
			myCh->misc.temp_wait_flag_cnt = 0;
			myCh->op.checkDelayTime = 0;
			myCh->misc.seq_no_cnt = 0;
			myCh->misc.maxV = myCh->misc.tmpVsens; //180722
			myCh->misc.startV = myCh->misc.tmpVsens;
			myCh->misc.minV = myCh->misc.tmpVsens;
			myCh->misc.maxI = myCh->misc.tmpIsens;
			myCh->misc.minI = myCh->misc.tmpIsens;
		/*	myCh->misc.maxV = myCh->op.Vsens;
			myCh->misc.startV = myCh->op.Vsens;
			myCh->misc.minV = myCh->op.Vsens; */
			myCh->misc.startT = myCh->op.temp;
			myCh->misc.maxT = myCh->op.temp;
			myCh->misc.minT = myCh->op.temp;
			myCh->misc.hw_fault_temp = myCh->op.runTime;
			//181202 add for Temp Wait Zero Data
			if(myData->DataSave.config.zero_sec_data_save == P1){
				if(myCh->ChAttribute.opType == P0){
					myCh->op.select = SAVE_FLAG_SAVING_TIME;
					if(myCh->ChAttribute.chNo_master == P0){
						myData->bData[bd].cData[ch-1].op.Isens = 0;
						myData->bData[bd].cData[ch-1].misc.tmpIsens = 0;
						myData->bData[bd].cData[ch-1].op.select 
							= myCh->op.select;
					}
					myCh->op.Isens = 0;
					myCh->misc.tmpIsens = 0;
					send_save_msg(bd, ch, 0, 0);
				}
			}
			
			//201015
			if(myCh->signal[C_SIG_OUT_SWITCH_ON] != P3){
				myCh->signal[C_SIG_OUT_SWITCH] = P1;
				myCh->signal[C_SIG_OUT_SWITCH_ON] = P1;
				rtn = 8;
			}else{
				rtn = SelectHwSpec(bd , ch);
			}
			
			if(rtn == 1) {
				if(myData->FADM.config.useFlag == P1 ||
					myData->mData.config.FadBdUse == 1){
					myCh->op.checkDelayTime = 0;
					myCh->signal[C_SIG_TRIGGER] = P1;
		    		myCh->op.phase = P1;
				}else{
					if(myPs->config.capacityType == CAPACITY_CAPACITANCE) {
						pcu_ref_output(bd, ch, type, mode, 
									val1, val2, div, div, rangeV, rangeI);
					} else {
						pcu_ref_output(bd, ch, type, mode, 
									val1, val2, div, div, rangeV, rangeI);
					}
		    		myCh->op.phase = P50;
				}
			} else if(rtn == 2) {
				ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, CMD_PCU_MODE_CC, 0,0);
				myCh->misc.cmd_v = 0;
				myCh->misc.cmd_v_div = (short int)div;
				myCh->misc.cmd_v_range = (short int)rangeV;
				myCh->misc.cmd_i = val2;
				myCh->misc.cmd_i_div = (short int)div;
				myCh->misc.cmd_i_range = (short int)rangeI;
				myCh->signal[C_SIG_OUT_SWITCH] = P0;
				myCh->signal[C_SIG_OUT_SWITCH_OFF] = P1;
	    		myCh->op.phase = P5;
			} else if(rtn == 3) {
				ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, CMD_PCU_MODE_CC, 0,0);
				myCh->signal[C_SIG_TRIGGER] = P1;
	    		myCh->op.phase = P1;
			} else if(rtn == 4) {
				if(myData->FADM.config.countMeter > 0){
		    		myCh->op.phase = P3;
				}else{
		    		myCh->op.phase = P50;
				}
			} else if(rtn == 5) {
				ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, CMD_PCU_MODE_CC, 0,0);
	    		myCh->op.phase = P2;
			} else if(rtn == 6) {
				ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, CMD_PCU_MODE_CC, 0,0);
				myCh->op.checkDelayTime = 0;
				myCh->signal[C_SIG_TRIGGER] = P1;
	    		myCh->op.phase = P1;
			} else if(rtn == 7) {
				ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, CMD_PCU_MODE_CC, 0,0);
				myCh->misc.cmd_v = 0;
				myCh->misc.cmd_v_div = (short int)div;
				myCh->misc.cmd_v_range = (short int)rangeV;
				myCh->misc.cmd_i = val2;
				myCh->misc.cmd_i_div = (short int)div;
				myCh->misc.cmd_i_range = (short int)rangeI;
	    		myCh->op.phase = P6;
			} else if(rtn == 8) { //201015
				myCh->op.checkDelayTime = 0;
				myCh->misc.sensCount = 0;
				myCh->misc.sensCountFlag = P0;
				myCh->misc.sensBufCount = 0;
				myCh->misc.sensBufCountFlag = P0;
	    		myCh->op.phase = P5;
			}
			break;
		case P1:
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
			if((myData->FADM.config.useFlag == P1 || 
				myData->mData.config.FadBdUse == 1) &&
					myCh->op.checkDelayTime < 10) {
			} else {
				myCh->misc.sensCount = 0;
				myCh->op.checkDelayTime = 0;
				myCh->misc.sensCountFlag = P0;
				myCh->misc.sensBufCount = 0;
				myCh->misc.sensBufCountFlag = P0;
				if(myPs->config.capacityType == CAPACITY_CAPACITANCE) {
					pcu_ref_output(bd, ch, type, mode, val1, val2, div, div, rangeV, rangeI);
				} else {
					pcu_ref_output(bd, ch, type, mode, refV, val2, div, div, rangeV, rangeI);
				}
	
				//0sec save LGC 5V6A 100716 kji
				if(myPs->config.hwSpec == L_5V_6A_R3){
					myCh->op.select = SAVE_FLAG_SAVING_TIME;
					send_save_msg(bd, ch, 0, 0);
				}
		    	myCh->op.phase = P50;
			}
			break;
		case P2:
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
			if(myCh->op.checkDelayTime >= 50) {
				myCh->op.checkDelayTime = 0;
				myCh->signal[C_SIG_TRIGGER] = P1;
				myCh->op.phase = P1;
			}
			break;
		case P3:
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
			if(myCh->op.checkDelayTime >= 10) {
				myCh->op.checkDelayTime = 0;
		    	myCh->op.phase = P4;
			}
			break;
		case P4:
			myCh->misc.sensCount = 0;
			myCh->misc.sensCountFlag = P0;
			myCh->misc.sensBufCount = 0;
			myCh->misc.sensBufCountFlag = P0;
			rtn = SelectHwSpec(bd , ch);
			if(rtn == 4) {
			} else {
				pcu_ref_output(bd, ch, type, mode, refV, val2, div, div, rangeV, rangeI);
			}
	    	myCh->op.phase = P50;
			break;
		case P5:
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
			if(myCh->op.checkDelayTime == 40) { //201015
				ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, 
											CMD_PCU_MODE_CC, 0,0);
			}else if(myCh->op.checkDelayTime >= 50) {
				myCh->op.phase = P4;
			}
			break;
		case P6:
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
			if(myCh->op.checkDelayTime >= 4) {
				myCh->op.checkDelayTime = 0;
				myCh->signal[C_SIG_OUT_SWITCH] = P1;
		//		if(myCh->ChAttribute.chNo_master != P0) 	//181008
				myCh->signal[C_SIG_OUT_SWITCH_ON] = P1;
				myCh->signal[C_SIG_I_CMD_OUTPUT] = P1;
				myCh->signal[C_SIG_V_CMD_OUTPUT] = P1;
				myCh->op.phase = P50;
			}
			break;
		case P10:
			if(myCh->misc.ac_fail_flag == P1){
				myCh->op.phase = P18;
			} else if(myCh->misc.ac_fail_flag == P0) {
				rtn = SelectHwSpec(bd , ch);
				if(rtn == 1
					|| rtn == 2
					|| rtn == 3
					|| rtn == 4
					|| rtn == 5
					|| rtn == 6
					|| rtn == 7) {
					ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, CMD_PCU_MODE_CC, 0,0);
					myCh->misc.ocv = myCh->op.Vsens;
					myCh->signal[C_SIG_V_RANGE] = rangeV+1;
					myCh->signal[C_SIG_I_RANGE] = rangeI+1;
	
					myCh->misc.fbV = 0;
					myCh->misc.fbI = 0;
					myCh->misc.ocv = myCh->op.Vsens;
					myCh->misc.pid_ui1[0] = 0.0;
					myCh->misc.pid_ui1[1] = 0.0;
					myCh->misc.pid_error1[0] = 0.0;
					myCh->misc.pid_error1[1] = 0.0;
					myCh->op.phase = P11;
/*				} else if(rtn == 2 || rtn == 7) {
					myCh->misc.ocv = myCh->op.Vsens;
	
					myCh->misc.fbV = 0;
					myCh->misc.fbI = 0;
	
					myCh->misc.cmd_v = 0;
					myCh->misc.cmd_v_div = (short int)div;
					myCh->misc.cmd_v_range = (short int)rangeV;
					myCh->misc.cmd_i = val2;
					myCh->misc.cmd_i_div = (short int)div;
					myCh->misc.cmd_i_range = (short int)rangeI;
					myCh->signal[C_SIG_V_RANGE] = rangeV+1;
					myCh->signal[C_SIG_I_RANGE] = rangeI+1;
					ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, CMD_PCU_MODE_CC, 0,0);
					myCh->op.phase = P13;*/
				}
			}
	    	break;
		case P11:
			myCh->op.phase = P12;
			break;
		case P12:
			myCh->misc.sensCount = 0;
			myCh->misc.sensCountFlag = P0;
			myCh->misc.sensBufCount = 0;
			myCh->misc.sensBufCountFlag = P0;
			myCh->op.checkDelayTime = 0;
			if(myPs->config.capacityType == CAPACITY_CAPACITANCE) {
				pcu_ref_output(bd, ch, type, mode, val1, val2, div, div, rangeV, rangeI);
			} else {
				pcu_ref_output(bd, ch, type, mode, val1, val2, div, div, rangeV, rangeI);
			}
			myCh->op.phase = P30;
			break;
/*		case P13:
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
			if(myCh->op.checkDelayTime == 10) {
				ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, CMD_PCU_MODE_CC, 0,0);
			} else if(myCh->op.checkDelayTime >= 30) {
				myCh->misc.sensCount = 0;
				myCh->misc.sensCountFlag = P0;
				myCh->misc.sensBufCount = 0;
				myCh->misc.sensBufCountFlag = P0;
		
				myCh->op.checkDelayTime = 0;
				myCh->signal[C_SIG_OUT_SWITCH] = P1;
				myCh->signal[C_SIG_OUT_SWITCH_ON] = P1;
				myCh->signal[C_SIG_V_CMD_OUTPUT] = P1;
				myCh->signal[C_SIG_I_CMD_OUTPUT] = P1;
				myCh->op.phase = P50;
			}
			break;*/
		case P18:
			myCh->misc.ac_fail_flag = P0;
			rtn = pcu_temp_wait_flag_check(bd, ch);
		//	rtn = temp_wait_flag_check(bd, ch);
			if(rtn == 0) {
				if(myCh->misc.temp_wait_flag_cnt == 0){
					myCh->misc.temp_wait_flag_cnt++;
					if(myCh->ChAttribute.opType == P0){
						ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, CMD_PCU_MODE_CC, 0,0);
					}
				}
				break;
			}
			myCh->misc.saveDt = myCh->op.runTime;
			rtn = SelectHwSpec(bd , ch);
			if(rtn == 1
				|| rtn == 2
				|| rtn == 3
				|| rtn == 4
				|| rtn == 5
				|| rtn == 6
				|| rtn == 7) {
				ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, CMD_PCU_MODE_CC, 0,0);
				myCh->misc.ocv = myCh->op.Vsens;
				myCh->signal[C_SIG_V_RANGE] = rangeV+1;
				myCh->signal[C_SIG_I_RANGE] = rangeI+1;
	
				myCh->misc.fbV = 0;
				myCh->misc.fbI = 0;
				myCh->misc.ocv = myCh->op.Vsens;
				myCh->misc.pid_ui1[0] = 0.0;
				myCh->misc.pid_ui1[1] = 0.0;
				myCh->misc.pid_error1[0] = 0.0;
				myCh->misc.pid_error1[1] = 0.0;
				myCh->op.phase = P11;
/*			} else if(rtn == 2 || rtn == 7) {
				myCh->misc.ocv = myCh->op.Vsens;
	
				myCh->misc.fbV = 0;
				myCh->misc.fbI = 0;

				myCh->misc.cmd_v = 0;
				myCh->misc.cmd_v_div = (short int)div;
				myCh->misc.cmd_v_range = (short int)rangeV;
				myCh->misc.cmd_i = val2;
				myCh->misc.cmd_i_div = (short int)div;
				myCh->misc.cmd_i_range = (short int)rangeI;
				myCh->signal[C_SIG_V_RANGE] = rangeV+1;
				myCh->signal[C_SIG_I_RANGE] = rangeI+1;
				ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, CMD_PCU_MODE_CC, 0,0);
				myCh->op.phase = P13;*/
			}
			break;
		case P20: //111130 kji run relay check 
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
			if(myCh->op.checkDelayTime >= 100) {
				myCh->op.checkDelayTime = 0;
				myCh->op.phase = P0;
			}
			break;
		case P30:
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
			if(myCh->op.checkDelayTime >= 20) {
				myCh->op.checkDelayTime = 0;
				myCh->op.phase = P50;
			}
			break;
		case P50:
			//180817 stepZ  seq error retry
			if(myCh->ChAttribute.opType == P0){		//200120 lyhw
				if(myCh->misc.send_pcu_seq_no != myCh->misc.receive_pcu_seq_no){
					myCh->misc.seq_no_cnt++;		//180822 
					if(myCh->misc.seq_no_cnt < MAX_SEQ_NO_RETRY){
						if(myPs->config.capacityType == CAPACITY_CAPACITANCE) {
							pcu_ref_output(bd, ch, type, mode, 
										val1, val2, div, div, rangeV, rangeI);
						} else {
							pcu_ref_output(bd, ch, type, mode, 
										val1, val2, div, div, rangeV, rangeI);
						}
					}
				}else{
					myCh->misc.seq_no_cnt = 0;		//180822 
				}
			}
		
			myCh->misc.start = 0;
	    	myCh->op.runTime += myPs->misc.rt_scan_time;
			myCh->op.totalRunTime += myPs->misc.rt_scan_time;	//181202
			myCh->misc.cycleRunTime += myPs->misc.rt_scan_time;
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
			
			if(myCh->op.checkDelayTime < 100){
				myCh->misc.Pre_change_V = myCh->misc.tmpVsens; //190901
				myCh->misc.maxV = myCh->misc.tmpVsens; //180722
				myCh->misc.startV = myCh->misc.tmpVsens;
				myCh->misc.minV = myCh->misc.tmpVsens;
				myCh->misc.maxI = myCh->misc.tmpIsens;
				myCh->misc.minI = myCh->misc.tmpIsens;
			}
			if(myCh->op.runTime >= 30) {
				myCh->misc.meanCount++;
				if(myCh->misc.meanCount <= 8) {
					myCh->misc.meanSumVolt += (myCh->misc.tmpVsens / 100);
					myCh->misc.meanSumCurr += (myCh->misc.tmpIsens / 100);
				} else {
					myCh->misc.meanSumVolt += (myCh->op.Vsens / 100);
					myCh->misc.meanSumCurr += (myCh->op.Isens / 100);
				}
				myCh->op.meanVolt = (long)(myCh->misc.meanSumVolt
					/ myCh->misc.meanCount) * 100;
				myCh->op.meanCurr = (long)(myCh->misc.meanSumCurr
					/ myCh->misc.meanCount) * 100;
			}
			if(myCh->op.checkDelayTime == myPs->misc.rt_scan_time) {
				pcu_ref_output(bd, ch, type, mode, val1, val2, div, div, rangeV, rangeI);
			}
			if(myCh->signal[C_SIG_STOP] == P1) {
				myCh->op.code = C_FAULT_STOP_CMD;
				myCh->op.select = SAVE_FLAG_SAVING_END;
				if(myCh->ChAttribute.opType == P0) {
					if(myCh->ChAttribute.chNo_master == 0){
						myData->bData[bd].cData[ch-1].op.code
							= myCh->op.code;
						myData->bData[bd].cData[ch-1].op.select 
							= myCh->op.select;
					}
					send_save_msg(bd, ch, saveDt, 0);
				}
				myCh->signal[C_SIG_STOP] = P0;
				ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, CMD_PCU_MODE_CC, 0,0);
				myCh->op.phase = P100;
			} else if(myCh->signal[C_SIG_PAUSE] == P1) {
				myCh->misc.tmpState = myCh->op.state;
				myCh->misc.tmpCode = myCh->op.code;
				myCh->op.code = C_FAULT_PAUSE_CMD;
				myCh->opSave
					= myCh->op;
				myCh->signal[C_SIG_PAUSE] = P0;
				ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, CMD_PCU_MODE_CC, 0,0);
				myCh->op.phase = P100;
			} else if(myCh->signal[C_SIG_DLL_STOP] == P1) {
				myCh->op.code = C_FAULT_CELL_DIAGNOSIS_STOP;
				myCh->op.select = SAVE_FLAG_SAVING_END;
				if(myCh->ChAttribute.opType == P0) {
					if(myCh->ChAttribute.chNo_master == 0){
						myData->bData[bd].cData[ch-1].op.code
							= myCh->op.code;
						myData->bData[bd].cData[ch-1].op.select 
							= myCh->op.select;
					}
					//send_save_msg(bd, ch, saveDt, 0);
				}
				myCh->signal[C_SIG_DLL_STOP] = P0;
				ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, CMD_PCU_MODE_CC, 0,0);
				myCh->op.phase = P100;
			} else {
				if(cFailCodeCheck(bd, ch) >= 0) {
					cSoftFeedback(bd, ch, val1, val2, val3);
				}
			}

	    	if(myCh->op.checkDelayTime == 100) {
				myCh->misc.maxI = myCh->op.Isens;
				myCh->misc.minI = myCh->op.Isens;
			}
//			cCalculate_Capacitance(bd, ch, advStepNo);
			if(myCh->ChAttribute.opType == P0)
				cCalculate_DCR(bd, ch, advStepNo);

			myCh->ccv[0].avg_v = myCh->misc.tmpVsens;
			myCh->ccv[0].avg_i = myCh->misc.tmpIsens;
			if(myCh->op.phase == P100){
				cNextStepCheck(bd, ch);
			}
			break;
		case P100:
			break;
		default: break;
    }
}

void pStepUserMap(int bd, int ch)
{
    int	rtn, realCh = 0;
	int	rangeV, rangeI, cnt;
    long val1, val2, val3, deltaT;
	unsigned long advStepNo, saveDt;
	double tmp1;

	S_CH_STEP_INFO	step;

	myCh = &(myData->bData[bd].cData[ch]);
	val1 = val2 = val3 = 0;
	tmp1 = 0.0;


	step = step_info(bd, ch);
    advStepNo = step.advStepNo;
	rangeV = step.rangeV;
	rangeI = step.rangeI;
	saveDt = step.saveDt;
	val1 = step.refV;
	val2 = step.refI;
    switch(myCh->op.phase) {
		case P0:
			realCh = myPs->config.chPerBd * bd + ch;
			send_msg(MODULE_TO_DATASAVE
					, MSG_MODULE_DATASAVE_READ_USER_MAP, realCh, advStepNo);
			myCh->misc.userMapFlag = P0;
			myCh->op.checkDelayTime = 0;
			myCh->op.phase = P1;
			//190107 add for limit V
			myCh->misc.limit_current_timeout = myCh->op.runTime;
			myCh->misc.hw_fault_temp = myCh->op.runTime;
			initCh(bd, ch);
			break;
		case P1:
			if(myCh->misc.userMapFlag == P1){
#if CHAMBER_TEMP_HUMIDITY == 1	//kjc_210419
				rtn = chamber_temp_humidity_check(ch, bd, advStepNo);
#else
				rtn = pcu_temp_wait_flag_check(bd, ch);
#endif
			//	rtn = temp_wait_flag_check(bd, ch);
				if(rtn == 0) {
					if(myCh->misc.temp_wait_flag_cnt == 0){
						myCh->misc.temp_wait_flag_cnt++;
						ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, CMD_PCU_MODE_CC, 0,0);
					}
					break;
				}
				myCh->misc.step_count ++;		//kjc_211023
				myCh->misc.cycleStepCount ++;	//kjc_211023
				myCh->misc.endState = P0;
				myCh->misc.groupEndTime = 0;
				// 111212 oys w : cycleNo
				myCh->misc.cycleNo = myPs->testCond[bd][ch].step[advStepNo].cycleNo;
				myCh->misc.userMapFlag = P0;
				myCh->misc.temp_wait_flag_cnt = 0;
				myCh->op.checkDelayTime = 0;
				myCh->misc.maxV = myCh->misc.tmpVsens; //180722
				myCh->misc.startV = myCh->misc.tmpVsens;
				myCh->misc.minV = myCh->misc.tmpVsens;
				myCh->misc.maxI = myCh->misc.tmpIsens;
				myCh->misc.minI = myCh->misc.tmpIsens;
			/*	myCh->misc.maxV = myCh->op.Vsens;
				myCh->misc.startV = myCh->op.Vsens;
				myCh->misc.minV = myCh->op.Vsens;*/
				myCh->misc.startT = myCh->op.temp;
				myCh->misc.maxT = myCh->op.temp;
				myCh->misc.minT = myCh->op.temp;
				//190107 add for limit V
				myCh->misc.limit_current_timeout = myCh->op.runTime;
				myCh->misc.hw_fault_temp = myCh->op.runTime;
				//181202 add for Temp Wait Zero Data
				if(myData->DataSave.config.zero_sec_data_save == P1){
					if(myCh->ChAttribute.opType == P0){
						myCh->op.select = SAVE_FLAG_SAVING_TIME;
						if(myCh->ChAttribute.chNo_master == P0){
							myData->bData[bd].cData[ch-1].op.Isens = 0;
							myData->bData[bd].cData[ch-1].misc.tmpIsens = 0;
							myData->bData[bd].cData[ch-1].op.select 
								= myCh->op.select;
						}
						myCh->op.Isens = 0;
						myCh->misc.tmpIsens = 0;
						send_save_msg(bd, ch, 0, 0);
					}
					/*
					myCh->op.Isens = 0;
					myCh->misc.tmpIsens = 0;
					myCh->op.select = SAVE_FLAG_SAVING_TIME;
					if(myData->bData[bd].cData[ch].ChAttribute.chNo_master == P0){
						send_save_msg(bd, ch-1, 0, 0);
					}else{
						if(myData->bData[bd].cData[ch].ChAttribute.opType == P0){
							send_save_msg(bd, ch, 0, 0);
						}
					}*/
				}
				myCh->op.phase = P2; //201015
			}else{
			  	if(myCh->signal[C_SIG_STOP] == P1) {
					myCh->op.code = C_FAULT_STOP_CMD;
					myCh->op.select = SAVE_FLAG_SAVING_END;
					if(myCh->ChAttribute.opType == P0) {
						if(myCh->ChAttribute.chNo_master == 0){
							myData->bData[bd].cData[ch-1].op.code
								= myCh->op.code;
							myData->bData[bd].cData[ch-1].op.select 
								= myCh->op.select;
						}
						send_save_msg(bd, ch, saveDt, 0);
					}
					myCh->signal[C_SIG_STOP] = P0;
					myCh->op.phase = P100;
				} else if(myCh->signal[C_SIG_PAUSE] == P1) {
					//cmd, soft fault
					myCh->misc.tmpState = myCh->op.state;
					myCh->misc.tmpCode = myCh->op.code;
					myCh->op.code = C_FAULT_PAUSE_CMD;
					myCh->opSave
						= myCh->op;
					myCh->signal[C_SIG_PAUSE] = P0;
					myCh->op.phase = P100;
				} else if(myCh->signal[C_SIG_DLL_STOP] == P1) {
					myCh->op.code = C_FAULT_CELL_DIAGNOSIS_STOP;
					myCh->op.select = SAVE_FLAG_SAVING_END;
					if(myCh->ChAttribute.opType == P0) {
						if(myCh->ChAttribute.chNo_master == 0){
							myData->bData[bd].cData[ch-1].op.code
								= myCh->op.code;
							myData->bData[bd].cData[ch-1].op.select 
								= myCh->op.select;
						}
						//send_save_msg(bd, ch, saveDt, 0);
					}
					myCh->signal[C_SIG_DLL_STOP] = P0;
					myCh->op.phase = P100;
				}else if(myCh->signal[C_SIG_NEXTSTEP] == P1) {
					myCh->signal[C_SIG_NEXTSTEP] = P0;
					myCh->op.code = C_FAULT_NEXTSTEP_CMD;
					myCh->op.select = SAVE_FLAG_SAVING_END;
					if(myCh->ChAttribute.opType == P0) {
						if(myCh->ChAttribute.chNo_master == 0){
							myData->bData[bd].cData[ch-1].op.code
								= myCh->op.code;
							myData->bData[bd].cData[ch-1].op.select 
								= myCh->op.select;
						}
						send_save_msg(bd, ch, saveDt, 0);
					}
					myCh->op.phase = P100;
				} else {
					cFailCodeCheck(bd, ch);
				}
			}
			break;
		case P2:
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time; //201015
			if(myCh->op.checkDelayTime >= 10) {
				if(myCh->signal[C_SIG_OUT_SWITCH_ON] != P3){
					myCh->signal[C_SIG_OUT_SWITCH] = P1;
					myCh->signal[C_SIG_OUT_SWITCH_ON] = P1;
				}
			}else if(myCh->op.checkDelayTime >= 20) {
				myCh->op.checkDelayTime = 0;
				myCh->op.phase = P3;
			}
			break;
		case P3:
			rtn = SelectHwSpec(bd , ch);
			myCh->misc.soc = read_user_map_ocvTable(bd, ch);
			myCh->misc.startSoc = myCh->misc.soc;
			rtn = 3;
			if(rtn == 1) {
				myCh->op.phase = P5;
			} else if(rtn == 2 || rtn == 7) {
//				myCh->op.phase = P6;
				myCh->op.phase = P5;
			} else if(rtn == 3) {
				myCh->op.phase = P5;
			} else if(rtn == 4) {
				myCh->op.phase = P5;
			} else if(rtn == 5) {
				myCh->op.phase = P5;
			} else if(rtn == 6) {
				myCh->op.phase = P5;
			}
			break;
		case P4:
			myCh->misc.sensCount = 0;
			myCh->misc.sensCountFlag = P0;
			myCh->misc.sensBufCount = 0;
			myCh->misc.sensBufCountFlag = P0;
			user_map_data(bd,ch);
			myCh->op.phase = P50;
			break;
		case P5:
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
			if(myCh->op.checkDelayTime == 20) {
				ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, CMD_PCU_MODE_CC, 0,0); //201015
			//	myCh->signal[C_SIG_OUT_SWITCH] = P1;
			//	myCh->signal[C_SIG_OUT_SWITCH_ON] = P1;
			}
			if(myCh->op.checkDelayTime >= 50) {
				myCh->op.checkDelayTime = 0;
				myCh->op.phase = P4;
				//0sec save LGC 5V6A 100716 kji
				if(myPs->config.hwSpec == L_5V_6A_R3)
				{
					myCh->op.select =  SAVE_FLAG_SAVING_TIME;
					send_save_msg(bd, ch, 0, 0);
				}
			}
			break;
/*		case P6:
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
			if(myCh->op.checkDelayTime == 10) {
				cV_Range_Select(bd, ch, rangeV+1);
				cI_Range_Select(bd, ch, rangeI+1);
			} else if(myCh->op.checkDelayTime > 25) {
				myCh->op.checkDelayTime = 0;
				user_pattern_data(bd, ch);
				myCh->op.phase = P7;
			}
			break;
		case P7:
			myCh->signal[C_SIG_OUT_SWITCH] = P1;
			myCh->signal[C_SIG_OUT_SWITCH_ON] = P1;
			myCh->signal[C_SIG_V_CMD_OUTPUT] = P1;
			myCh->op.phase = P8;
			break;
		case P8:
			myCh->misc.sensCount = 0;
			myCh->misc.sensCountFlag = P0;
			myCh->misc.sensBufCount = 0;
			myCh->misc.sensBufCountFlag = P0;
			myCh->signal[C_SIG_I_CMD_OUTPUT] = P1;
			myCh->misc.userPatternRunTime
				= myCh->op.runTime;
			myCh->op.phase = P50;
			break;*/
		case P10:
			rtn = SelectHwSpec(bd , ch);
			if(rtn == 1
				|| rtn == 2
				|| rtn == 3
				|| rtn == 4
				|| rtn == 5
				|| rtn == 6
				|| rtn == 7) {
				myCh->misc.ocv = myCh->op.Vsens;
				myCh->signal[C_SIG_V_RANGE] = rangeV+1;
				myCh->signal[C_SIG_I_RANGE] = rangeI+1;

				myCh->misc.fbV = 0;
				myCh->misc.fbI = 0;
				myCh->misc.ocv = myCh->op.Vsens;
				myCh->misc.pid_ui1[0] = 0.0;
				myCh->misc.pid_ui1[1] = 0.0;
				myCh->misc.pid_error1[0] = 0.0;
				myCh->misc.pid_error1[1] = 0.0;
				myCh->op.phase = P11;
/*			} else if(rtn == 2 || rtn == 7) {
				myCh->signal[C_SIG_V_RANGE] = rangeV+1;
				myCh->signal[C_SIG_I_RANGE] = rangeI+1;

				myCh->misc.fbV = 0;
				myCh->misc.fbI = 0;
				myCh->misc.ocv = myCh->op.Vsens;
				myCh->misc.pid_ui1[0] = 0.0;
				myCh->misc.pid_ui1[1] = 0.0;
				myCh->misc.pid_error1[0] = 0.0;
				myCh->misc.pid_error1[1] = 0.0;

				myCh->misc.cmd_v = val1;
				myCh->misc.cmd_v_div = (short int)div;
				myCh->misc.cmd_v_range = (short int)rangeV;
				myCh->misc.cmd_i = val2;
				myCh->misc.cmd_i_div = (short int)div;
				myCh->misc.cmd_i_range = (short int)rangeI;
				myCh->op.checkDelayTime = 0;
				myCh->op.phase = P12;*/
			}
			break;
		case P11:
			user_map_data(bd,ch);
			myCh->op.phase = P50;
			break;
/*		case P12:
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
			if(myCh->op.checkDelayTime == 10) {
				cV_Range_Select(bd, ch, rangeV+1);
				cI_Range_Select(bd, ch, rangeI+1);
			} else if(myCh->op.checkDelayTime > 25) {
				myCh->op.checkDelayTime = 0;
				user_map_data(bd,ch);
				myCh->op.phase = P13;
			}
			break;
		case P13:
			myCh->signal[C_SIG_OUT_SWITCH] = P1;
			myCh->signal[C_SIG_OUT_SWITCH_ON] = P1;
			myCh->signal[C_SIG_V_CMD_OUTPUT] = P1;
			myCh->op.phase = P14;
			break;
		case P14:
			myCh->signal[C_SIG_I_CMD_OUTPUT] = P1;
			myCh->op.phase = P50;
			break;*/
		case P50:
			if(myCh->op.checkDelayTime < 100){
				myCh->misc.Pre_change_V = myCh->misc.tmpVsens; //190901
				myCh->misc.maxV = myCh->misc.tmpVsens; //180722
				myCh->misc.startV = myCh->misc.tmpVsens;
				myCh->misc.minV = myCh->misc.tmpVsens;
				myCh->misc.maxI = myCh->misc.tmpIsens;
				myCh->misc.minI = myCh->misc.tmpIsens;
			}
			myCh->misc.start = 0;
	    	myCh->op.runTime += myPs->misc.rt_scan_time;
			myCh->op.totalRunTime += myPs->misc.rt_scan_time;	//181202
			myCh->misc.cycleRunTime += myPs->misc.rt_scan_time;
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
			if(myCh->op.runTime % myData->mData.testCond[bd][ch].
						userMap.renewalTime == 0) {
				myCh->misc.soc = myCh->misc.startSoc + (long)((double)(myCh->op.ampareHour)/
								(double)(myData->mData.testCond[bd][ch].
									userMap.maxCapacity) * 100000);
				user_map_data(bd,ch);

			}
			if(myCh->op.runTime >= 30) {
				myCh->misc.meanCount++;
				if(myCh->misc.meanCount <= 8) {
					myCh->misc.meanSumVolt += (myCh->misc.tmpVsens / 100);
					myCh->misc.meanSumCurr += (myCh->misc.tmpIsens / 100);
				} else {
					myCh->misc.meanSumVolt += (myCh->op.Vsens / 100);
					myCh->misc.meanSumCurr += (myCh->op.Isens / 100);
				}
				myCh->op.meanVolt = (long)(myCh->misc.meanSumVolt
					/ myCh->misc.meanCount) * 100;
				myCh->op.meanCurr = (long)(myCh->misc.meanSumCurr
					/ myCh->misc.meanCount) * 100;
			}
			if(myCh->signal[C_SIG_STOP] == P1) {
				myCh->op.code = C_FAULT_STOP_CMD;
				myCh->op.select = SAVE_FLAG_SAVING_END;
				if(myCh->ChAttribute.opType == P0) {
					if(myCh->ChAttribute.chNo_master == 0){
						myData->bData[bd].cData[ch-1].op.code
							= myCh->op.code;
						myData->bData[bd].cData[ch-1].op.select 
							= myCh->op.select;
					}
					send_save_msg(bd, ch, saveDt, 0);
				}
				myCh->signal[C_SIG_STOP] = P0;
				ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, CMD_PCU_MODE_CC, 0,0);
				myCh->op.phase = P100;
			} else if(myCh->signal[C_SIG_PAUSE] == P1) {
				//cmd, soft fault
				myCh->misc.tmpState = myCh->op.state;
				myCh->misc.tmpCode = myCh->op.code;
				myCh->op.code = C_FAULT_PAUSE_CMD;
				myCh->opSave
					= myCh->op;
				myCh->signal[C_SIG_PAUSE] = P0;
				ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, CMD_PCU_MODE_CC, 0,0);
				myCh->op.phase = P100;
			} else if(myCh->signal[C_SIG_DLL_STOP] == P1) {
				myCh->op.code = C_FAULT_CELL_DIAGNOSIS_STOP;
				myCh->op.select = SAVE_FLAG_SAVING_END;
				if(myCh->ChAttribute.opType == P0) {
					if(myCh->ChAttribute.chNo_master == 0){
						myData->bData[bd].cData[ch-1].op.code
							= myCh->op.code;
						myData->bData[bd].cData[ch-1].op.select 
							= myCh->op.select;
					}
					//send_save_msg(bd, ch, saveDt, 0);
				}
				myCh->signal[C_SIG_DLL_STOP] = P0;
				ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, CMD_PCU_MODE_CC, 0,0);
				myCh->op.phase = P100;
			} else {
				if(cFailCodeCheck(bd, ch) >= 0) {
					cSoftFeedback(bd, ch, val1, val2, val3);
				}
			}
	    	if(myCh->op.checkDelayTime == 100) {
				myCh->misc.maxI = myCh->op.Isens;
				myCh->misc.minI = myCh->op.Isens;
			}
			if(myCh->op.phase == P100){
				myCh->ccv[0].avg_v = myCh->misc.tmpVsens;
				myCh->ccv[0].avg_i = myCh->misc.tmpIsens;
				cNextStepCheck(bd, ch);
				break;
			}else{
			    deltaT = ((myCh->op.runTime/10)*10)
						- myCh->misc.userPatternRunTime;
				cnt = find_pattern_time(bd, ch, deltaT);
				if(myCh->misc.userPatternCnt != cnt)
				{	
					myCh->misc.userPatternCnt = cnt;
					user_pattern_data(bd, ch);
				}
			}
			break;
		case P100:
			break;
		default: break;
    }
}

void pStepUserPattern(int bd, int ch)
{
    int	rtn, realCh = 0, num;
	int	rangeV, rangeI, cnt;
    long val1, val2, val3, deltaT, length;
	unsigned long advStepNo, saveDt;
	double tmp1;
	unsigned char slave_flag = 1;
	int i = 0; //210419

	S_CH_STEP_INFO	step;

	myCh = &(myData->bData[bd].cData[ch]);
	val1 = val2 = val3 = 0;
	tmp1 = 0.0;
	num = 0;

	length = myData->mData.testCond[bd][ch].userPattern.length;

	step = step_info(bd, ch);
    advStepNo = step.advStepNo;
	rangeV = step.rangeV;
	rangeI = step.rangeI;
	saveDt = step.saveDt;
	val1 = step.refV;
	val2 = step.refI;
    switch(myCh->op.phase) {
		case P0:
			//190107 add for limit V
			myCh->misc.limit_current_timeout = myCh->op.runTime;
			myCh->misc.hw_fault_temp = myCh->op.runTime;
			ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, CMD_PCU_MODE_CC, 0,0);
			
			realCh = myPs->config.chPerBd * bd + ch;
			//191120 lyhw
#if USER_PATTERN_500 == 1 
			myCh->misc.StepPattern_ReadNum = 0;
			num = ++myCh->misc.StepPattern_ReadNum;
			Send_Read_UserPattern(bd, ch, advStepNo, num);
#else
			send_msg(MODULE_TO_DATASAVE
			, MSG_MODULE_DATASAVE_READ_USER_PATTERN, realCh, advStepNo);
#endif
			myCh->misc.userPatternFlag = P0;
			myCh->op.checkDelayTime = 0;
			myCh->op.phase = P1;
			initCh(bd, ch);
			break;
		case P1:
			if(myCh->ChAttribute.opType == P1){	//180108
				if(myData->bData[bd].cData[ch+1].misc.userPatternFlag == P1){
					slave_flag = P1;
				}else{
					slave_flag = P0;
				}
			}else{
				slave_flag = P1;		//Master Ch
			}	
			
			if((myCh->misc.userPatternFlag == P1)&&(slave_flag == P1)) {
#if CHAMBER_TEMP_HUMIDITY == 1	//kjc_210419
				rtn = chamber_temp_humidity_check(ch, bd, advStepNo);
#else
				rtn = pcu_temp_wait_flag_check(bd, ch);
#endif
			//	rtn = temp_wait_flag_check(bd, ch);
				if(rtn == 0) {
					//181212 User Pattern
					if(myCh->misc.temp_wait_flag_cnt == 0){
						myCh->misc.temp_wait_flag_cnt++;
						ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, CMD_PCU_MODE_CC, 0,0);
					}
					break;
				}
				myCh->misc.step_count ++;		//kjc_211023
				myCh->misc.cycleStepCount ++;	//kjc_211023
				myCh->misc.endState = P0;
				myCh->misc.groupEndTime = 0;
				// 111212 oys w : cycleNo
				myCh->misc.cycleNo = myPs->testCond[bd][ch].step[advStepNo].cycleNo;
				myCh->misc.userPatternFlag = P0;
				myCh->op.checkDelayTime = 0;
				myCh->misc.temp_wait_flag_cnt = 0;
				myCh->misc.maxV = myCh->misc.tmpVsens; //180722
				myCh->misc.startV = myCh->misc.tmpVsens;
				myCh->misc.minV = myCh->misc.tmpVsens;
				myCh->misc.maxI = myCh->misc.tmpIsens;
				myCh->misc.minI = myCh->misc.tmpIsens;
			/*	myCh->misc.maxV = myCh->op.Vsens;
				myCh->misc.startV = myCh->op.Vsens;
				myCh->misc.minV = myCh->op.Vsens;*/
				myCh->misc.startT = myCh->op.temp;
				myCh->misc.maxT = myCh->op.temp;
				myCh->misc.minT = myCh->op.temp;
				//190107 add for limit V
				myCh->misc.limit_current_timeout = myCh->op.runTime;
				myCh->misc.hw_fault_temp = myCh->op.runTime;
				//181202 add for Temp Wait Zero Data
				if(myData->DataSave.config.zero_sec_data_save == P1){
					if(myCh->ChAttribute.opType == P0){
						myCh->op.select = SAVE_FLAG_SAVING_TIME;
						if(myCh->ChAttribute.chNo_master == P0){
							myData->bData[bd].cData[ch-1].op.Isens = 0;
							myData->bData[bd].cData[ch-1].misc.tmpIsens = 0;
							myData->bData[bd].cData[ch-1].op.select 
								= myCh->op.select;
						}
						myCh->op.Isens = 0;
						myCh->misc.tmpIsens = 0;
						send_save_msg(bd, ch, 0, 0);
						
					}
					/*
					myCh->op.Isens = 0;
					myCh->misc.tmpIsens = 0;
					myCh->op.select = SAVE_FLAG_SAVING_TIME;
					if(myData->bData[bd].cData[ch].ChAttribute.chNo_master == P0){
						send_save_msg(bd, ch-1, 0, 0);
					}else{
						if(myData->bData[bd].cData[ch].ChAttribute.opType == P0){
							send_save_msg(bd, ch, 0, 0);
						}
					}*/
				}
				myCh->signal[C_SIG_RANGE_SWITCH] = P1;
				myCh->signal[C_SIG_V_RANGE] = rangeV+1;
				myCh->signal[C_SIG_I_RANGE] = rangeI+1;
				myCh->op.phase = P2;
			}else{
			  	if(myCh->signal[C_SIG_STOP] == P1) {
					myCh->op.code = C_FAULT_STOP_CMD;
					myCh->op.select = SAVE_FLAG_SAVING_END;
					if(myCh->ChAttribute.opType == P0) {
						if(myCh->ChAttribute.chNo_master == 0){
							myData->bData[bd].cData[ch-1].op.code
								= myCh->op.code;
							myData->bData[bd].cData[ch-1].op.select 
								= myCh->op.select;
						}
						send_save_msg(bd, ch, saveDt, 0);
					}
					myCh->signal[C_SIG_STOP] = P0;
					myCh->op.phase = P100;
				} else if(myCh->signal[C_SIG_PAUSE] == P1) {
					//cmd, soft fault
					myCh->misc.tmpState = myCh->op.state;
					myCh->misc.tmpCode = myCh->op.code;
					myCh->op.code = C_FAULT_PAUSE_CMD;
					myCh->opSave
						= myCh->op;
					myCh->signal[C_SIG_PAUSE] = P0;
					myCh->op.phase = P100;
				} else if(myCh->signal[C_SIG_DLL_STOP] == P1) {
					myCh->op.code = C_FAULT_CELL_DIAGNOSIS_STOP;
					myCh->op.select = SAVE_FLAG_SAVING_END;
					if(myCh->ChAttribute.opType == P0) {
						if(myCh->ChAttribute.chNo_master == 0){
							myData->bData[bd].cData[ch-1].op.code
								= myCh->op.code;
							myData->bData[bd].cData[ch-1].op.select 
								= myCh->op.select;
						}
						//send_save_msg(bd, ch, saveDt, 0);
					}
					myCh->signal[C_SIG_DLL_STOP] = P0;
					myCh->op.phase = P100;
				}else if(myCh->signal[C_SIG_NEXTSTEP] == P1) {
					myCh->signal[C_SIG_NEXTSTEP] = P0;
					myCh->op.code = C_FAULT_NEXTSTEP_CMD;
					myCh->op.select = SAVE_FLAG_SAVING_END;
					if(myCh->ChAttribute.opType == P0) {
						if(myCh->ChAttribute.chNo_master == 0){
							myData->bData[bd].cData[ch-1].op.code
								= myCh->op.code;
							myData->bData[bd].cData[ch-1].op.select 
								= myCh->op.select;
						}
						send_save_msg(bd, ch, saveDt, 0);
					}
					myCh->op.phase = P100;
				} else {
					cFailCodeCheck(bd, ch);
				}
			}
			break;
		case P2:
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
			if(myCh->op.checkDelayTime >= 20) {
				//201015
				if(myCh->signal[C_SIG_OUT_SWITCH_ON] != P3){
					myCh->signal[C_SIG_OUT_SWITCH] = P1;
					myCh->signal[C_SIG_OUT_SWITCH_ON] = P1;
				}
				myCh->op.checkDelayTime = 0;
				myCh->op.phase = P3;
			}
			break;
		case P3:
			rtn = SelectHwSpec(bd , ch);
			if(rtn == 1) {
				myCh->op.phase = P5;
			} else if(rtn == 2 || rtn == 7) {
//				myCh->op.phase = P6;
				myCh->op.phase = P5;
			} else if(rtn == 3) {
				myCh->op.phase = P5;
			} else if(rtn == 4) {
				myCh->op.phase = P5;
			} else if(rtn == 5) {
				myCh->op.phase = P5;
			} else if(rtn == 6) {
				myCh->op.phase = P5;
			}
			break;
		case P4:
			myCh->misc.sensCount = 0;
			myCh->misc.sensCountFlag = P0;
			myCh->misc.sensBufCount = 0;
			myCh->misc.sensBufCountFlag = P0;
//			myCh->misc.userPatternRunTime = myCh->op.runTime;
			pcu_user_pattern_data(bd, ch);
			myCh->op.phase = P50;
			myCh->op.checkDelayTime = 0;
			break;
		case P5:
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
			if(myCh->op.checkDelayTime == 40) { //201015
				ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, CMD_PCU_MODE_CC, 0,0);
			//	myCh->signal[C_SIG_OUT_SWITCH] = P1;
			//	myCh->signal[C_SIG_OUT_SWITCH_ON] = P1;
			}
			if(myCh->op.checkDelayTime >= 50) {
				myCh->op.checkDelayTime = 0;
				myCh->op.phase = P4;
				//0sec save LGC 5V6A 100716 kji
				if(myPs->config.hwSpec == L_5V_6A_R3)
				{
					myCh->op.select =  SAVE_FLAG_SAVING_TIME;
					send_save_msg(bd, ch, 0, 0);
				}
			}
			break;
/*		case P6:
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
			if(myCh->op.checkDelayTime == 10) {
				cV_Range_Select(bd, ch, rangeV+1);
				cI_Range_Select(bd, ch, rangeI+1);
			} else if(myCh->op.checkDelayTime > 25) {
				myCh->op.checkDelayTime = 0;
				pcu_user_pattern_data(bd, ch);
				myCh->op.phase = P7;
			}
			break;
		case P7:
			myCh->signal[C_SIG_OUT_SWITCH] = P1;
			myCh->signal[C_SIG_OUT_SWITCH_ON] = P1;
			myCh->signal[C_SIG_V_CMD_OUTPUT] = P1;
			myCh->op.phase = P8;
			break;
		case P8:
			myCh->misc.sensCount = 0;
			myCh->misc.sensCountFlag = P0;
			myCh->misc.sensBufCount = 0;
			myCh->misc.sensBufCountFlag = P0;
			myCh->signal[C_SIG_I_CMD_OUTPUT] = P1;
			myCh->misc.userPatternRunTime
				= myCh->op.runTime;
			myCh->op.phase = P50;
			break;*/
		case P10:
			realCh = myPs->config.chPerBd * bd + ch;
			if(myCh->misc.ac_fail_flag == P1){ //210419
				send_msg(MODULE_TO_DATASAVE
					, MSG_MODULE_DATASAVE_READ_USER_PATTERN, realCh, advStepNo);
				myCh->op.phase = P18;
			}else if(myCh->misc.ac_fail_flag == P0){
				rtn = SelectHwSpec(bd , ch);
				if(rtn == 1
					|| rtn == 2
					|| rtn == 3
					|| rtn == 4
					|| rtn == 5
					|| rtn == 6
					|| rtn == 7) {
					myCh->misc.ocv = myCh->op.Vsens;
					myCh->signal[C_SIG_V_RANGE] = rangeV+1;
					myCh->signal[C_SIG_I_RANGE] = rangeI+1;

					myCh->misc.fbV = 0;
					myCh->misc.fbI = 0;
					myCh->misc.ocv = myCh->op.Vsens;
					myCh->misc.pid_ui1[0] = 0.0;
					myCh->misc.pid_ui1[1] = 0.0;
					myCh->misc.pid_error1[0] = 0.0;
					myCh->misc.pid_error1[1] = 0.0;
					myCh->op.phase = P11; //210624
				}
			}
			break;
		case P11:
			pcu_user_pattern_data(bd, ch);
			myCh->op.phase = P50;
			myCh->op.checkDelayTime = 0;
			break;
		case P12:
			//170315 lyh add for 100mS Data
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
			if(myCh->op.checkDelayTime >= 10) {
				myCh->op.checkDelayTime = 0;
				myCh->op.phase = P50;
			}
			break;
		case P18: //210419
			if(myCh->misc.userPatternFlag == P1){
				myCh->misc.ac_fail_flag = P0;
				length = myTestCond->userPattern.length;
				myCh->misc.saveDt = myCh->op.runTime;
				for(i = 0 ; i < length ; i++ ){
					if(myTestCond->userPattern.data[i].time>=myCh->op.runTime){
						myCh->misc.userPatternCnt = i;
						break;
					}
				}
				rtn = SelectHwSpec(bd , ch);
				if(rtn == 1
					|| rtn == 2
					|| rtn == 3
					|| rtn == 4
					|| rtn == 5
					|| rtn == 6
					|| rtn == 7) {
					myCh->misc.ocv = myCh->op.Vsens;
					myCh->signal[C_SIG_V_RANGE] = rangeV+1;
					myCh->signal[C_SIG_I_RANGE] = rangeI+1;

					myCh->misc.fbV = 0;
					myCh->misc.fbI = 0;
					myCh->misc.ocv = myCh->op.Vsens;
					myCh->misc.pid_ui1[0] = 0.0;
					myCh->misc.pid_ui1[1] = 0.0;
					myCh->misc.pid_error1[0] = 0.0;
					myCh->misc.pid_error1[1] = 0.0;
					myCh->op.phase = P11;
				}
			}
			break;
		case P50:
			//180817 add for UserPattern seq error retry
			if(myCh->ChAttribute.opType == P0){		//200120 lyhw
				if(myCh->misc.userPatternCnt != P0){
					if(myCh->misc.send_pcu_seq_no != myCh->misc.receive_pcu_seq_no){
						myCh->misc.seq_no_cnt++;		//180822 
						if(myCh->misc.seq_no_cnt < MAX_SEQ_NO_RETRY){
							pcu_user_pattern_data(bd, ch);
						//	break;		//200120 need check
						}
					}else{
						myCh->misc.seq_no_cnt = 0;		//180822 
					}
				}
			}
			
			if(myCh->op.checkDelayTime < 100){
				myCh->misc.Pre_change_V = myCh->misc.tmpVsens;	//190901
				myCh->misc.maxV = myCh->misc.tmpVsens; //180722
				myCh->misc.startV = myCh->misc.tmpVsens;
				myCh->misc.minV = myCh->misc.tmpVsens;
				myCh->misc.maxI = myCh->misc.tmpIsens;
				myCh->misc.minI = myCh->misc.tmpIsens;
			}
	   	 	myCh->op.runTime += myPs->misc.rt_scan_time;
			myCh->op.totalRunTime += myPs->misc.rt_scan_time;	//181202
			myCh->misc.cycleRunTime += myPs->misc.rt_scan_time;
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
//			if(myCh->misc.patternPhase == P0) {
			//	myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
/*			} else if(myCh->misc.patternPhase == P1) {
				cC_D_Select(bd, ch, P0);
				myCh->signal[C_SIG_V_CMD_OUTPUT] = P1;
				myCh->signal[C_SIG_OUT_SWITCH] = P1;
				myCh->signal[C_SIG_OUT_SWITCH_ON] = P1;
				myCh->misc.patternPhase++; 
			} else if(myCh->misc.patternPhase == P2) {
				myCh->signal[C_SIG_I_CMD_OUTPUT] = P1;
				myCh->misc.patternPhase = P0; 
			} else if(myCh->misc.patternPhase == P11) {
				cC_D_Select(bd, ch, P1);
				myCh->misc.patternPhase++; 
			} else if(myCh->misc.patternPhase == P12) {
				myCh->signal[C_SIG_OUT_SWITCH] = P1;
				myCh->signal[C_SIG_OUT_SWITCH_ON] = P1;
				myCh->signal[C_SIG_V_CMD_OUTPUT] = P1;
				myCh->signal[C_SIG_I_CMD_OUTPUT] = P1;
				myCh->misc.patternPhase = P0; 
			} else {
				myCh->misc.patternPhase = P0; 
			}*/
			if(myCh->op.runTime >= 30) {
				myCh->misc.meanCount++;
				if(myCh->misc.meanCount <= 8) {
					myCh->misc.meanSumVolt += (myCh->misc.tmpVsens / 100);
					myCh->misc.meanSumCurr += (myCh->misc.tmpIsens / 100);
				} else {
					myCh->misc.meanSumVolt += (myCh->op.Vsens / 100);
					myCh->misc.meanSumCurr += (myCh->op.Isens / 100);
				}
				myCh->op.meanVolt = (long)(myCh->misc.meanSumVolt
					/ myCh->misc.meanCount) * 100;
				myCh->op.meanCurr = (long)(myCh->misc.meanSumCurr
					/ myCh->misc.meanCount) * 100;
			}
			if(myCh->signal[C_SIG_STOP] == P1) {
				myCh->op.code = C_FAULT_STOP_CMD;
				myCh->op.select = SAVE_FLAG_SAVING_END;
				if(myCh->ChAttribute.opType == P0) {
					if(myCh->ChAttribute.chNo_master == 0){
						myData->bData[bd].cData[ch-1].op.code
							= myCh->op.code;
						myData->bData[bd].cData[ch-1].op.select 
							= myCh->op.select;
					}
					send_save_msg(bd, ch, saveDt, 0);
				}
				myCh->signal[C_SIG_STOP] = P0;
				ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, CMD_PCU_MODE_CC, 0,0);
				myCh->op.phase = P100;
			} else if(myCh->signal[C_SIG_PAUSE] == P1) {
				//cmd, soft fault
				myCh->misc.tmpState = myCh->op.state;
				myCh->misc.tmpCode = myCh->op.code;
				myCh->op.code = C_FAULT_PAUSE_CMD;
				myCh->opSave
					= myCh->op;
				myCh->signal[C_SIG_PAUSE] = P0;
				ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, CMD_PCU_MODE_CC, 0,0);
				myCh->op.phase = P100;
			} else if(myCh->signal[C_SIG_DLL_STOP] == P1) {
				myCh->op.code = C_FAULT_CELL_DIAGNOSIS_STOP;
				myCh->op.select = SAVE_FLAG_SAVING_END;
				if(myCh->ChAttribute.opType == P0) {
					if(myCh->ChAttribute.chNo_master == 0){
						myData->bData[bd].cData[ch-1].op.code
							= myCh->op.code;
						myData->bData[bd].cData[ch-1].op.select 
							= myCh->op.select;
					}
					//send_save_msg(bd, ch, saveDt, 0);
				}
				myCh->signal[C_SIG_DLL_STOP] = P0;
				ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, CMD_PCU_MODE_CC, 0,0);
				myCh->op.phase = P100;
			} else {
				if(cFailCodeCheck(bd, ch) >= 0) {
					cSoftFeedback(bd, ch, val1, val2, val3);
				}
			}
			if(myCh->misc.refFlag == P1) {
				myCh->misc.refFlag = P0;
//				cCalCmdV(bd, ch, val1, div, rangeV);
			}
	    	if(myCh->op.checkDelayTime == 100) {
				myCh->misc.maxI = myCh->op.Isens;
				myCh->misc.minI = myCh->op.Isens;
			}
			if(myCh->op.phase == P100){
				myCh->ccv[0].avg_v = myCh->misc.tmpVsens;
				myCh->ccv[0].avg_i = myCh->misc.tmpIsens;
				cNextStepCheck(bd, ch);
				break;
			}else{
			    deltaT = myCh->op.runTime
						- (myCh->misc.userPatternRunTime
						  	- myPs->misc.rt_scan_time);
				cnt = find_pattern_time(bd, ch, deltaT);
				if(myCh->misc.userPatternCnt != cnt){
					myCh->misc.userPatternCnt = cnt;
					pcu_user_pattern_data(bd, ch);
				}
			}
			
#if USER_PATTERN_500 == P1
			if(myCh->op.checkDelayTime > 300){
				if(myCh->misc.StepPattern_TotalNum > 1){
					if(myCh->misc.StepPattern_ReadNum == 0){
						myCh->misc.StepPattern_ReadNum = 1;
						num = ++myCh->misc.StepPattern_ReadNum;
						if(num > myCh->misc.StepPattern_TotalNum){
							myCh->misc.StepPattern_ReadNum = 1;
							num = myCh->misc.StepPattern_ReadNum;
						}
					
						Send_Read_UserPattern(bd, ch, advStepNo, num);
					}else{
						if(myCh->misc.userPattern_ReadFlag == 1){
							myCh->misc.userPattern_ReadFlag = 0;
							num = ++myCh->misc.StepPattern_ReadNum;
							if(num > myCh->misc.StepPattern_TotalNum){
								myCh->misc.StepPattern_ReadNum = 1;
								num = myCh->misc.StepPattern_ReadNum;
							}
							
							Send_Read_UserPattern(bd, ch, advStepNo, num);
						}
					}
				}
			}
#endif
//20170317 KHK ---------------------------
			/*  deltaT = myCh->op.runTime
						- myCh->misc.userPatternRunTime;
				cnt = find_pattern_time(bd, ch, deltaT);
			//	if(myCh->misc.userPatternCnt != cnt)
			//	{	
			//		myCh->misc.userPatternCnt = cnt;
			//		pcu_user_pattern_data(bd, ch);
			//	}
				if(myCh->op.runTime % 5 == 0){
					myCh->misc.userPatternCnt = cnt;
					pcu_user_pattern_data(bd, ch);
				}
			}*/
//20170317  -------------------------------
			break;
		case P100:
			break;
		default: break;
    }
}

long pCalCmdV(int bd, int ch, long value, int div, int range)
{
    int point=0;
    double tmp=0;
	int type = 0;

	myCh = &(myData->bData[bd].cData[ch]);
	myCh->misc.preVref = value;
	
	if(myData->mData.cali_meas_type == MEAS) div = 5;

	if(div > 0 && div <= 10) {
		point = cFindDACaliPoint(bd, ch, value, type, range);
		if((range+1) == RANGE1) {
			switch(myPs->config.hwSpec) {
				default:
					if(myCh->op.state == C_CALI 
						&& myData->mData.cali_meas_type != MEAS) {
						tmp = (double)value
							* myData->cali.tmpData[bd][ch]
							.DA_A[type][range][point]
							+ myData->cali.tmpData[bd][ch]
							.DA_B[type][range][point];
					} else {
						//180212 
						if(myCh->ChAttribute.chNo_master == P0){
							tmp = (double)value;
						}else{
							tmp = (double)value
								* myData->cali.data[bd][ch]
								.DA_A[type][range][point]
								+ myData->cali.data[bd][ch]
								.DA_B[type][range][point];
						}
					}
					break;
			}
		} else if((range+1) == RANGE2) {
			if(myCh->op.state == C_CALI 
					&& myData->mData.cali_meas_type != MEAS){
				tmp = (double)value
					* myData->cali.tmpData[bd][ch]
					.DA_A[type][range][point]
					+ myData->cali.tmpData[bd][ch]
					.DA_B[type][range][point];
			} else {
				if(myCh->ChAttribute.chNo_master == P0){
					tmp = (double)value;
				}else{
					tmp = (double)value
						* myData->cali.data[bd][ch]
						.DA_A[type][range][point]
						+ myData->cali.data[bd][ch]
						.DA_B[type][range][point];
				}
			}
		} else if((range+1) == RANGE3) {
			if(myCh->op.state == C_CALI
					&& myData->mData.cali_meas_type != MEAS){
				tmp = (double)value
					* myData->cali.tmpData[bd][ch]
					.DA_A[type][range][point]
					+ myData->cali.tmpData[bd][ch]
					.DA_B[type][range][point];
			} else {
				if(myCh->ChAttribute.chNo_master == P0){
					tmp = (double)value;
				}else{
					tmp = (double)value
						* myData->cali.data[bd][ch]
						.DA_A[type][range][point]
						+ myData->cali.data[bd][ch]
						.DA_B[type][range][point];
				}
			}
		} else { //range4
			if(myCh->op.state == C_CALI
					&& myData->mData.cali_meas_type != MEAS) {
				tmp = (double)value
					* myData->cali.tmpData[bd][ch]
					.DA_A[type][range][point]
					+ myData->cali.tmpData[bd][ch]
					.DA_B[type][range][point];
			} else {
				if(myCh->ChAttribute.chNo_master == P0){
					tmp = (double)value;
				}else{
					tmp = (double)value
						* myData->cali.data[bd][ch]
						.DA_A[type][range][point]
						+ myData->cali.data[bd][ch]
						.DA_B[type][range][point];
				}
			}
		}
	} else if(div > 10){
		div -= 10;
		if((range+1) == RANGE1) {
			tmp = (double)value;
		} else if((range+1) == RANGE2) {
			tmp = (double)value;
		} else if((range+1) == RANGE3) {
			tmp = (double)value;
		} else { //range4
			tmp = (double)value;
		}
	}else	tmp = 0;

	if(myPs->config.ratioVoltage == MICRO) { //1uV
		tmp *= 0.001;
	}else{ //1nV
		tmp *= 0.000001;
	}
	return (long)tmp;
}

long pCalCmdI(int bd, int ch, long value, int div, int range)
{
    int point, dir;
	double tmp=0;

	int type = 1;

	myCh = &(myData->bData[bd].cData[ch]);
	myCh->misc.preIref = value;
	dir = value;
	if(myData->mData.cali_meas_type == MEAS)	div = 5;

	if(div > 0 && div <= 10) {
		point = cFindDACaliPoint(bd, ch, value, type, range);
		//180726 add MEAS
		if(myCh->op.state == C_CALI && myData->mData.cali_meas_type != MEAS) {
		 	tmp = (double)value
				* myData->cali.tmpData[bd][ch].DA_A[type][range][point]
				+ myData->cali.tmpData[bd][ch].DA_B[type][range][point];
		} else {
			if(myCh->ChAttribute.chNo_master == P0){		//180212
		 		tmp = (double)value;
			}else{
		 		tmp = (double)value
					* myData->cali.data[bd][ch].DA_A[type][range][point]
					+ myData->cali.data[bd][ch].DA_B[type][range][point];
			}
		}
	}else if(div > 10){
		div -=10;
		tmp = (double)value;
	}else tmp = 0;
	if(myPs->config.ratioVoltage == MICRO) { //1uA
		tmp *= 0.001;
	}else{ //1nA
		tmp *= 0.000001;
	}
	return (long)tmp;
}

void pCali(int bd, int ch)
{
	int base_addr, addr_step, addr1, addr, ch_div;
	int rtn, type, range, point, runTime, idx;
	int cmd;
	long offset, calidata, rangePoint, pointVal;
	//calibration io
	int realCh;
	unsigned char caliVal=0, tmpVal=0;
	unsigned long relay_state;
	
	addr = 0x607;
	realCh = (bd * myPs->config.chPerBd) + ch;
	tmpVal = realCh / 8;
	caliVal = tmpVal << 4;
	caliVal |= realCh % 8;
	outb(caliVal , 0x607);

	type = myData->cali.tmpCond[bd][ch].type;
	range = myData->cali.tmpCond[bd][ch].range;
	point = (int)myCh->signal[C_SIG_CALI_POINT];
	
    switch(myCh->signal[C_SIG_CALI_PHASE]) {
		case P0: //voltage cali
			myPs->signal[M_SIG_CALI_RELAY] = P0;
			myData->CaliMeter.caliType = CALI_V;
			rtn = pCalibrationV(bd, ch, point); //da_v, ad_v;
			if(rtn > 0) {
				myCh->op.phase = P0;
				if(point != (int)myData->cali.tmpCond[bd][ch]
					.point[type][range].setPointNum-1) {
   	 				myCh->signal[C_SIG_CALI_POINT]++;
					ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, 
							CMD_PCU_CALI_V_DATA_CNT, range,
			(int)myData->cali.tmpCond[bd][ch].point[type][range].setPointNum);
				} else {
					myCh->signal[C_SIG_CALI_POINT] = P0;
   	 				myCh->signal[C_SIG_CALI_PHASE] = P1;
					myCh->signal[C_SIG_OUT_SWITCH] = P0;
				//	myCh->signal[C_SIG_OUT_SWITCH_OFF] = P1;
					myCh->signal[C_SIG_OUT_SWITCH_OFF] = P9;
					if(myData->mData.cali_meas_type == MEAS){
   	 					myCh->signal[C_SIG_CALI_PHASE] = P91;
					}
				}
			}
			break;
		case P1:
			cCalculate_CaliData(bd, ch);
			myCh->signal[C_SIG_CALI_PHASE] = P2;
			break;
		case P62:
			if(type == P0){
   	 			myCh->signal[C_SIG_CALI_PHASE] = P70;
			}else{ 	//180726 check lyhw
   	 			myCh->signal[C_SIG_CALI_PHASE] = P90;
			}
			break;
		case P70: //voltage cali check
			myPs->signal[M_SIG_CALI_RELAY] = P0;
			myData->CaliMeter.caliType = CALI_V;
			rtn = pCalibrationCheckV(bd, ch, point);
			if(rtn > 0) {
				myCh->op.phase = P0;
				if(point != (int)myData->cali.tmpCond[bd][ch]
					.point[type][range].checkPointNum-1) {
					myCh->signal[C_SIG_CALI_POINT]++;
				} else {
					if(myData->cali.tmpCond[bd][ch].mode == CALI_MODE_NORMAL) {
						send_msg(MODULE_TO_DATASAVE,
							MSG_MODULE_DATASAVE_CALI_NORMAL_RESULT_SAVE,
							bd, ch);
					} else if(myData->cali.tmpCond[bd][ch].mode
						== CALI_MODE_CHECK) {
						send_msg(MODULE_TO_DATASAVE,
							MSG_MODULE_DATASAVE_CALI_CHECK_RESULT_SAVE, bd, ch);
					}
					myCh->signal[C_SIG_CALI_PHASE] = P71;
				}
			}
			break;
		case P71: //wait check data save to file
			if(myCh->signal[C_SIG_CALI_NORMAL_RESULT_SAVED] == P1) {
				myCh->signal[C_SIG_CALI_NORMAL_RESULT_SAVED] = P0;
				//20180812 sch
				if(myData->mData.cali_meas_type == MEAS){
   	 				myCh->signal[C_SIG_CALI_PHASE] = P73;
				}else{
					myCh->signal[C_SIG_CALI_PHASE] = P72;
				}
			}
			if(myCh->signal[C_SIG_CALI_CHECK_RESULT_SAVED] == P1) {
				myCh->signal[C_SIG_CALI_CHECK_RESULT_SAVED] = P0;
				//20180812 sch
				if(myData->mData.cali_meas_type == MEAS){
   	 				myCh->signal[C_SIG_CALI_PHASE] = P73;
				}else{
					myCh->signal[C_SIG_CALI_PHASE] = P72;
				}
			}
			break;
		case P72:
			if(myCh->signal[C_SIG_CALI_NORMAL_RESULT_SAVED] == P2
				|| myData->CaliMeter.config.Shunt_Sel_Calibrator == P3){
				//180801 add Sel_Cali 
				ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, 
							CMD_PCU_CALI_DATA_SAVE, 1, 0);
   	 			myCh->signal[C_SIG_CALI_PHASE]++;
			}else if(myCh->signal[C_SIG_CALI_NORMAL_RESULT_SAVED] == P3) {
				ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, 
							CMD_PCU_CALI_DATA_SAVE, 0, 0);
   	 			myCh->signal[C_SIG_CALI_PHASE]++;
			}
			break;
		case P73:
   	 		myCh->signal[C_SIG_CALI_PHASE]++;
			break;
		case P74:
   	 		myCh->signal[C_SIG_CALI_PHASE]++;
			break;
		case P75: //180713 add lyhw
			myCh->op.phase = P0;
			myCh->op.state = C_STANDBY;
			myCh->signal[C_SIG_OUT_SWITCH] = P0;
			myCh->signal[C_SIG_OUT_SWITCH_OFF] = P1;
			myCh->signal[C_SIG_RANGE_SWITCH] = P0;
			//180927 add lyhw
			myPs->signal[M_SIG_CALI_VOLTAGE2_RELAY] = P0;
			break;
		case P80: //current cali
			myPs->signal[M_SIG_CALI_RELAY] = P1;
			myData->CaliMeter.caliType = CALI_I;
			rtn = pCalibrationI(bd, ch, point); //da_i, ad_i;
			if(rtn > 0) {
				myCh->op.phase = P0;
				if(point != (int)myData->cali.tmpCond[bd][ch]
					.point[type][range].setPointNum-1) {
					myCh->signal[C_SIG_CALI_POINT]++;
				} else {
					myCh->signal[C_SIG_CALI_POINT] = P0;
					myCh->op.runTime = 0;
    				myCh->signal[C_SIG_CALI_PHASE] = P81;
					myCh->misc.semiSwitchState = SEMI_PRE;
					ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, CMD_PCU_MODE_CC, 0,0);
					ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, 
							CMD_PCU_CALI_V_DATA_CNT, range,
			(int)myData->cali.tmpCond[bd][ch].point[type][range].setPointNum);
					if(myData->mData.cali_meas_type == MEAS){
    					myCh->signal[C_SIG_CALI_PHASE] = P91;
					}
				}
			}
			break;
		case P81:
	    	myCh->op.runTime += myPs->misc.rt_scan_time;
			runTime = 100;
			if(myCh->op.runTime >= runTime){
				myPs->signal[M_SIG_CALI_CHARGE_RELAY] = P0;
				myPs->signal[M_SIG_CALI_DISCHARGE_RELAY] = P0;
				myCh->signal[C_SIG_OUT_SWITCH] = P0;
				myCh->signal[C_SIG_OUT_SWITCH_OFF] = P1;
				myCh->op.runTime = 0; 
				myCh->signal[C_SIG_CALI_PHASE] = P82;
				//180927 lyhw
			//	myPs->signal[M_SIG_CALI_VOLTAGE2_RELAY] = P0;
			//	myPs->signal[M_SIG_CALI_CHARGE2_RELAY] = P0;
			//	myPs->signal[M_SIG_CALI_DISCHARGE2_RELAY] = P0;
			}
			break;
		case P82:
			cCalculate_CaliData(bd, ch);
			myCh->signal[C_SIG_CALI_PHASE] = P2;
			break;
		case P90: //current cali check
			myPs->signal[M_SIG_CALI_RELAY] = P1;
			myData->CaliMeter.caliType = CALI_I;
			rtn = pCalibrationCheckI(bd, ch, point);
			if(rtn > 0) {
				myCh->op.phase = P0;
				if(point != (int)myData->cali.tmpCond[bd][ch]
					.point[type][range].checkPointNum-1) {
					myCh->signal[C_SIG_CALI_POINT]++;
				} else {
					if(myData->cali.tmpCond[bd][ch].mode == CALI_MODE_NORMAL) {
						send_msg(MODULE_TO_DATASAVE,
							MSG_MODULE_DATASAVE_CALI_NORMAL_RESULT_SAVE,
							bd, ch);
					} else if(myData->cali.tmpCond[bd][ch].mode
						== CALI_MODE_CHECK) {
						send_msg(MODULE_TO_DATASAVE,
							MSG_MODULE_DATASAVE_CALI_CHECK_RESULT_SAVE, bd, ch);
					}
					myCh->signal[C_SIG_CALI_PHASE] = P91;
					ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, CMD_PCU_MODE_CC, 0,0);
				}
			}
			break;
		case P91: //wait check data save to file
			if(myData->mData.cali_meas_type == MEAS){	//180726
				myCh->op.runTime = 0;
				myCh->signal[C_SIG_CALI_PHASE] = P92;
			}
			if(myCh->signal[C_SIG_CALI_NORMAL_RESULT_SAVED] == P1) {
				myCh->signal[C_SIG_CALI_NORMAL_RESULT_SAVED] = P0;
				myCh->signal[C_SIG_CALI_PHASE] = P92;
			}
			if(myCh->signal[C_SIG_CALI_CHECK_RESULT_SAVED] == P1) {
				myCh->signal[C_SIG_CALI_CHECK_RESULT_SAVED] = P0;
				myCh->op.runTime = 0;
				myCh->signal[C_SIG_CALI_PHASE] = P92;
			}
			break;
		case P92:
	    	myCh->op.runTime += myPs->misc.rt_scan_time;
			runTime = 100;
			if(myCh->op.runTime >= runTime){
				myCh->op.runTime =0;
				myPs->signal[M_SIG_CALI_RELAY] = P0;
				myPs->signal[M_SIG_CALI_CHARGE_RELAY] = P0;
				myPs->signal[M_SIG_CALI_DISCHARGE_RELAY] = P0;
				//20180812 sch
				if(myData->mData.cali_meas_type == MEAS){
   	 				myCh->signal[C_SIG_CALI_PHASE] = P94;
				}else{
					myCh->signal[C_SIG_CALI_PHASE] = P93;
				}
			}
			break;
		case P93:
			if(myCh->signal[C_SIG_CALI_NORMAL_RESULT_SAVED] == P2//OK
				|| myData->CaliMeter.config.Shunt_Sel_Calibrator == P3){
				//180801 add Sel_Cali 
				myCh->signal[C_SIG_CALI_NORMAL_RESULT_SAVED] = P0;
				ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, 
						CMD_PCU_CALI_DATA_SAVE, 1, 0);
   	 			myCh->signal[C_SIG_CALI_PHASE]++;
			}else if(myCh->signal[C_SIG_CALI_NORMAL_RESULT_SAVED] == P3) {//NG
				myCh->signal[C_SIG_CALI_NORMAL_RESULT_SAVED] = P0;
				ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, 
						CMD_PCU_CALI_DATA_SAVE, 0, 0);
   	 			myCh->signal[C_SIG_CALI_PHASE]++;
			}else if(myData->mData.cali_meas_type == MEAS){	//180726
   	 			myCh->signal[C_SIG_CALI_PHASE]++;
			}
			break;
		case P94:
   	 		myCh->signal[C_SIG_CALI_PHASE]++;
			break;
		case P95:
			myPs->signal[M_SIG_CALI_RELAY] = P0;
			myPs->signal[M_SIG_CALI_RELAY] = P0;
			myCh->op.phase = P0;
			myCh->op.state = C_STANDBY;
			myCh->signal[C_SIG_OUT_SWITCH] = P0;
			myCh->signal[C_SIG_OUT_SWITCH_OFF] = P1;
			myCh->signal[C_SIG_RANGE_SWITCH] = P0;
			//181025 lyhw
			myPs->signal[M_SIG_CALI_VOLTAGE2_RELAY] = P0;
			myPs->signal[M_SIG_CALI_CHARGE2_RELAY] = P0;
			myPs->signal[M_SIG_CALI_DISCHARGE2_RELAY] = P0;
			break;
		default:
			base_addr = myPs->addr.main[BASE_ADDR];
			addr_step = myPs->addr.main[ADDR_STEP];
			ch_div = myPs->pcu_config.portPerCh;
			if(ch < 32){
				addr1 = base_addr + addr_step * (ch / ch_div);
			} else{
				addr1 = base_addr + addr_step * ((ch-32) / ch_div);
			}
			addr = addr1 + CREG_CTRL_CCU1_STATE + (ch % ch_div);

			relay_state = SREG8_read2(addr) & 0x02;
			if(relay_state == P2){
				myData->bData[bd].cData[ch].signal[C_SIG_OUT_SWITCH_OFF] = P1;
				break;
			}
			
			addr = addr1 + CREG_CTRL_STATE;
			idx = myCh->signal[C_SIG_CALI_PHASE]-2;
			// real phase P2~P31 -> idx P0~29
			if(idx < P30){//P0~P29
				if(idx < P15){
					type = 0;
					cmd = CMD_PCU_CALI_V_DATA;
				}else{		
					type = 1;
					idx -= 15;
					cmd = CMD_PCU_CALI_I_DATA;
				}

				if(myData->CaliMeter.caliType == CALI_V){
					if(idx < P15){
						offset = (long)(myData->cali.tmpData[bd][ch].AD_B[type][range][idx] / 100.0);
						calidata = (long)(myData->cali.tmpData[bd][ch].AD_A[type][range][idx] * 1000000) + ((idx + 1) * 10000000);
					}else{
						offset = (long)(myData->cali.data[bd][ch].AD_B[type][range][idx] / 100.0);
						calidata = (long)(myData->cali.data[bd][ch].AD_A[type][range][idx] * 1000000) + ((idx + 1) * 10000000);
					}
				}else{
					if(idx < P15){
						offset = (long)(myData->cali.tmpData[bd][ch].AD_B[type][range][idx] / 100.0);
						calidata = (long)(myData->cali.tmpData[bd][ch].AD_A[type][range][idx] * 1000000) + ((idx + 1) * 10000000);
					}else{
						offset = (long)(myData->cali.tmpData[bd][ch].AD_B[type][range][idx] / 100.0);
						calidata = (long)(myData->cali.tmpData[bd][ch].AD_A[type][range][idx] * 1000000) + ((idx + 1) * 10000000);
					}
				}
				ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, cmd, offset, calidata);

	 			while(!(SREG16_read(addr) & 0x4000))break;
   		 		myCh->signal[C_SIG_CALI_PHASE]++;
				break;
			}else if(idx < P60){//P30~P59
				if(idx < P45){
					type = 0;
					idx -= 30;
					cmd = CMD_PCU_CALI_V_RANGE_POINT;
				}else{
					type = 1;
					idx -= 45;
					cmd = CMD_PCU_CALI_I_RANGE_POINT;
				}
				if(myData->CaliMeter.caliType == CALI_V){
					if(idx < P45){
						pointVal = myData->cali.tmpCond[bd][ch].point[type][range].setPoint[idx] / 1000;
					}else{
						pointVal = myData->cali.data[bd][ch].point[type][range].setPoint[idx] / 1000;
					}
					if(range == 0) range += 1;
					rangePoint = (range * 100) + (idx + 1);
				}else{
					if(idx < P45){
						pointVal = myData->cali.tmpCond[bd][ch].point[type][range].setPoint[idx] / 1000;
					}else{
						pointVal = myData->cali.tmpCond[bd][ch].point[type][range].setPoint[idx] / 1000;
					}
					if(range == 0) range += 1;
					rangePoint = (range * 100) + (idx + 1);
				}
				ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, cmd, rangePoint, pointVal);
	 			while(!(SREG16_read(addr) & 0x4000))break;
   		 		myCh->signal[C_SIG_CALI_PHASE]++;
				break;
			}
			break;
	}
}

void pCaliUpdate(int bd, int ch)
{
	int base_addr, addr_step, addr1, addr, ch_div;
	int realCh, type, range, point, idx, cmd;
	long offset, calidata, rangePoint, pointVal;
	unsigned long relay_state;
	
	realCh = (bd * myPs->config.chPerBd) + ch + 1;

	type = myData->cali.tmpCond[bd][ch].type;
	range = myData->cali.tmpCond[bd][ch].range;
	point = (int)myCh->signal[C_SIG_CALI_POINT];
	
    switch(myCh->signal[C_SIG_CALI_PHASE]) {
		case P0: //voltage cali
			if(myData->mData.misc.d_cali_update_ch != 0) return;
			if(myCh->signal[C_SIG_D_CALI_UPDATE] == P1){
				myCh->signal[C_SIG_D_CALI_UPDATE] = P0;
				myData->mData.misc.d_cali_update_ch = realCh;
				myCh->signal[C_SIG_CALI_PHASE] = P2;
			}
			break;
		case P62:
   	 		myCh->signal[C_SIG_CALI_PHASE] = P70;
			break;
		case P70:
   	 		myCh->signal[C_SIG_CALI_PHASE]++;
			break;
		case P71:
   	 		myCh->signal[C_SIG_CALI_PHASE]++;
			break;
		case P72:
			//180801 add Sel_Cali 
			ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, 
										CMD_PCU_CALI_DATA_SAVE, 1, 0);
   	 		myCh->signal[C_SIG_CALI_PHASE]++;
			break;
		case P73:
   	 		myCh->signal[C_SIG_CALI_PHASE]++;
			myCh->op.checkDelayTime = 0;
			break;
		case P74:
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
			if(myCh->op.checkDelayTime >= 100){
				myCh->op.checkDelayTime = 0;
				send_msg(MODULE_TO_APP, 
							MSG_MODULE_APP_D_CALI_UPDATE, realCh, 0);
   	 			myCh->signal[C_SIG_CALI_PHASE]++;
			}
			break;
		case P75: //180713 add lyhw
			myCh->op.phase = P0;
			myCh->op.state = C_STANDBY;
			myData->mData.misc.d_cali_update_ch = 0;
			break;
		default:
			base_addr = myPs->addr.main[BASE_ADDR];
			addr_step = myPs->addr.main[ADDR_STEP];
			ch_div = myPs->pcu_config.portPerCh;
			if(ch < 32){
				addr1 = base_addr + addr_step * (ch / ch_div);
			} else{
				addr1 = base_addr + addr_step * ((ch-32) / ch_div);
			}
			addr = addr1 + CREG_CTRL_CCU1_STATE + (ch % ch_div);

			relay_state = SREG8_read2(addr) & 0x02;
			if(relay_state == P2){
				myData->bData[bd].cData[ch].signal[C_SIG_OUT_SWITCH_OFF] = P1;
				break;
			}
			
			addr = addr1 + CREG_CTRL_STATE;
			idx = myCh->signal[C_SIG_CALI_PHASE]-2;
			// real phase P2~P31 -> idx P0~29
			if(idx < P30){//P0~P29
				if(idx < P15){
					type = 0;
					cmd = CMD_PCU_CALI_V_DATA;
				}else{		
					type = 1;
					idx -= 15;
					cmd = CMD_PCU_CALI_I_DATA;
				}
				offset = (long)(myData->cali.data[bd][ch]
					.AD_B[type][range][idx] / 100.0);
				calidata = (long)(myData->cali.data[bd][ch]
					.AD_A[type][range][idx] * 1000000) + ((idx + 1) * 10000000);
				ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, 
													cmd, offset, calidata);
	 			while(!(SREG16_read(addr) & 0x4000))break;
   		 		myCh->signal[C_SIG_CALI_PHASE]++;
				break;
			}else if(idx < P60){//P30~P59
				if(idx < P45){
					type = 0;
					idx -= 30;
					cmd = CMD_PCU_CALI_V_RANGE_POINT;
				}else{
					type = 1;
					idx -= 45;
					cmd = CMD_PCU_CALI_I_RANGE_POINT;
				}
				pointVal = myData->cali.data[bd][ch]
								.point[type][range].setPoint[idx] / 1000;
				if(range == 0) range += 1;
				rangePoint = (range * 100) + (idx + 1);
				
				ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, 
												cmd, rangePoint, pointVal);
	 			while(!(SREG16_read(addr) & 0x4000))break;
   		 		myCh->signal[C_SIG_CALI_PHASE]++;
				break;
			}
			break;
	}
}

int pCalibrationV(int bd, int ch, int point)
{
	int rtn, type, rangeV, rangeI, div, mode;
	long val1, val2;
	float ohm, amp;

	rtn = 0;
	type = 0;
	rangeV = myData->cali.tmpCond[bd][ch].range;
	rangeI = 0;
	div = 15;
	mode = CCCV;

	val1 = myData->cali.tmpCond[bd][ch].point[type][rangeV].setPoint[point];
	if(val1 > myPs->config.maxVoltage[rangeV]) {
		val1 = myPs->config.maxVoltage[rangeV];
	} else if(val1 < myPs->config.minVoltage[rangeV]) {
		val1 = myPs->config.minVoltage[rangeV];
	}

	//5V/10mA (Load : 1Kohm 1/4W) 5mA
	//6V/6A (Load : 10ohm 10W) 600mA
	//Linear 5V/200A (Load : 10ohm 10W) 500mA
	//Switching 5V/200A (Load : 4.7ohm/4EA=1.175ohm 40W) 4255mA
	//2V/100A (Load : 4.7ohm/4EA=1.175ohm 40W) 1702mA
	//5V/50A (Load : 4.7ohm/2EA=2.35ohm 20W) 2128mA
	//50V/50A (Load : 200ohm 1EA)
	
	if(rangeV == 0) {
		if(val1 > 0){
			val2 = myPs->config.maxCurrent[rangeI];
		}else{
			val2 = myPs->config.minCurrent[rangeI];
		}
	}else{
		val2 = myPs->config.minCurrent[rangeI];
	}
#if SHUNT_R_RCV == 1		//180705 add for shunt
	myData->CaliMeter.shunt_mOhm 
		= myData->cali.tmpCond[bd][ch].point[type][rangeV].shuntValue;
#else
	myData->CaliMeter.shunt_mOhm = myPs->pcu_config.caliV_ohm;
#endif
	ohm = myData->CaliMeter.shunt_mOhm;
	amp = myPs->pcu_config.caliV_amp;
	
//	myData->CaliMeter.shunt_mOhm = ohm;
	val2 = (val1 / ohm) * amp;			//JYG_141016

    switch(myCh->op.phase) {
		case P0:
			if(myData->CaliMeter.config.Shunt_Sel_Calibrator == 3){
				pCali_Ch_Select_auto_3(bd, ch, point);
			}
			myPs->signal[M_SIG_CALI_RELAY] = P0;
			myCh->signal[C_SIG_OUT_SWITCH] = P1;
	//		if(myCh->ChAttribute.chNo_master != P0){		//180108
			myCh->signal[C_SIG_OUT_SWITCH_ON] = P1;
	//		}
			myCh->op.phase = P1;
			myCh->misc.cmd_v = val1;		//200115
			myCh->op.runTime = 0;
			myCh->misc.caliCheckPoint = 0;
			myCh->misc.caliCheckSum = 0;
			myCh->misc.caliCheckSum1 = 0;
			break;
		case P1:
			myCh->signal[C_SIG_V_RANGE] = (unsigned char)(rangeV+1);
			myCh->signal[C_SIG_I_RANGE] = (unsigned char)(rangeI+1);
			myCh->signal[C_SIG_RANGE_SWITCH] = P1;
			myCh->op.runTime += myPs->misc.rt_scan_time;
			if(myCh->op.runTime >= RTTASK_1500MS) {
				myCh->op.runTime = 0;
				myCh->op.phase = P2;
			}
			break;
		case P2:
			myCh->op.phase = P3;
			break;
		case P3:
			pcu_ref_output(bd, ch, STEP_IDLE, mode, val1, val2, div, div, rangeV, rangeI);
			myCh->op.runTime = 0;
			myCh->op.phase = P4;
			break;
		case P4:
	    	myCh->op.runTime += myPs->misc.rt_scan_time;
			if(myCh->op.runTime >= myPs->pcu_config.cali_delay_time) {
				if(myData->AppControl.config.debugType != P0) {
				} else {
					send_msg(MODULE_TO_METER, MSG_MODULE_METER_REQUEST, bd, ch);
				}
				myCh->op.phase = P10;
				myCh->op.runTime = 0;
				if(myData->mData.cali_meas_type == MEAS){
					myCh->op.phase = P6;
				}
			}
			break;
		case P6:
	    	myCh->op.runTime += myPs->misc.rt_scan_time;
			if(myCh->op.runTime % 100 == 0){
				send_msg(MODULE_TO_METER, MSG_MODULE_METER_REQUEST, bd, ch);
				if(myCh->signal[C_SIG_METER_REPLY] == P1) {
					myCh->signal[C_SIG_METER_REPLY] = P0;

					myData->cali.tmpData[bd][ch]
						.set_ad[type][rangeV][0]
						= (double)myCh->op.Vsens;
					myData->cali.tmpData[bd][ch]
						.set_meter[type][rangeV][0]
						= return_meter_value(bd, ch, type);

					send_msg(MODULE_TO_DATASAVE,
						MSG_MODULE_DATASAVE_CALI_NORMAL_RESULT_SAVE,
						bd, ch);
					myCh->op.phase = P6;
					rtn = 0;
					if(myCh->op.runTime >= myData->mData.cali_meas_time){
						rtn = 1;
						break;
					}
				} else if(myCh->signal[C_SIG_METER_ERROR] == P1) {
					myCh->signal[C_SIG_METER_ERROR] = P0;
					myCh->signal[C_SIG_OUT_SWITCH] = P0;
					if(myPs->config.hwSpec > DC_DIGITAL_SPEC){	//180611 add
						myCh->signal[C_SIG_OUT_SWITCH_OFF] = P1;
					}	
					myCh->signal[C_SIG_V_RANGE] = RANGE0;
					myCh->signal[C_SIG_I_RANGE] = RANGE0;
					myCh->op.state = C_STANDBY;
					myCh->op.phase = P0;
				}
			}
			break;
		case P10: // meter, ad value read
			if(myData->AppControl.config.debugType != P0) {
				myCh->signal[C_SIG_METER_REPLY] = P0;
				myData->cali.tmpData[bd][ch].set_ad[type][rangeV][point]
					= myData->cali.orgAD[type];
				myData->cali.tmpData[bd][ch].set_meter[type][rangeV][point]
					= myData->CaliMeter.value / 100.0;
				myCh->op.phase = P20;
			} else {	//180805 lyhw
				data_gathering_cali(bd, ch , type, rangeV, point);
			}

			/*	if(myCh->signal[C_SIG_METER_REPLY] == P1) {
					myCh->signal[C_SIG_METER_REPLY] = P0;
					//111215 detail calibration add
					myCh->misc.caliCheckPoint++;
					myCh->misc.caliCheckSum += myData->cali.orgAD[type];
					myCh->misc.caliCheckSum1 +=	 
						 myData->CaliMeter.value / 100.0;
					if(myCh->misc.caliCheckPoint >= myPs->pcu_config.setCaliNum) {
						myData->cali.tmpData[bd][ch].set_ad[type][rangeV][point]
							= (double)(myCh->misc.caliCheckSum / 
											myPs->pcu_config.setCaliNum);
						myData->cali.tmpData[bd][ch].set_meter
							[type][rangeV][point] = (double)
							(myCh->misc.caliCheckSum1 / myPs->pcu_config.setCaliNum);
						myCh->op.phase = P20;
						myCh->misc.caliCheckPoint = 0;
						myCh->misc.caliCheckSum = 0;
						myCh->misc.caliCheckSum1 = 0;
					}else{
						myCh->op.phase = P4;
						myCh->op.runTime = 0;
					}

					myData->cali.tmpData[bd][ch].set_ad[type][rangeV][point]
						= myData->cali.orgAD[type];
					myData->cali.tmpData[bd][ch].set_meter[type][rangeV][point]
						= myData->CaliMeter.value / 100.0;
					myCh->op.phase = P20;
				} else if(myCh->signal[C_SIG_METER_ERROR] == P1) {
					myCh->signal[C_SIG_METER_ERROR] = P0;
					myCh->signal[C_SIG_V_RANGE] = RANGE0;
					myCh->signal[C_SIG_I_RANGE] = RANGE0;
					myCh->op.phase = P25;
				}
			}*/
			break;
		case P20:
			myCh->signal[C_SIG_V_RANGE] = RANGE1;
			ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, CMD_PCU_MODE_CC, 0,0);//CC0
			myCh->op.phase = P21;
			break;
		case P21:
//			myCh->signal[C_SIG_OUT_SWITCH] = P0;
//			myCh->signal[C_SIG_OUT_SWITCH_OFF] = P1;
			myCh->op.phase = P22;
			break;
		case P22:
			myCh->op.phase = P23;
			break;
		case P23:
			myCh->op.phase = P24;
			break;
		case P24:
	    	myCh->op.runTime += myPs->misc.rt_scan_time;
			if(myCh->op.runTime >= RTTASK_5000MS) {
				myCh->op.runTime = 0;
				myCh->op.phase = P30;
			}
			break;
		case P25:
			ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, CMD_PCU_MODE_CC, 0,0);//CC0
			myCh->op.phase = P26;
			break;
		case P26:
	    	myCh->op.runTime += myPs->misc.rt_scan_time;
//			if(myCh->op.runTime >= RTTASK_1500MS*2) {
			if(myCh->op.runTime >= myPs->pcu_config.cali_delay_time) {
				myCh->op.runTime = 0;
				myCh->signal[C_SIG_OUT_SWITCH] = P0;
				myCh->signal[C_SIG_OUT_SWITCH_OFF] = P1;
				myCh->op.state = C_STANDBY;
				myCh->op.phase = P0;
			}
			break;
		case P30:
			rtn = 1;
			break;
		default: break;
	}
	return rtn;
}

int pCalibrationCheckV(int bd, int ch, int point)
{
	int rtn, type, rangeV, rangeI, div;
	long val1, val2;
	unsigned char mode;
	float ohm, amp;

	rtn = 0;
	type = 0;
	rangeV = myData->cali.tmpCond[bd][ch].range;
	rangeI = 0;
	div = 15;
	mode = CCCV;

	val1 = myData->cali.tmpCond[bd][ch].point[type][rangeV].checkPoint[point];
	if(val1 > myPs->config.maxVoltage[rangeV]) {
		val1 = myPs->config.maxVoltage[rangeV];
	} else if(val1 < myPs->config.minVoltage[rangeV]) {
		val1 = myPs->config.minVoltage[rangeV];
	}

	//5V/10mA (Load : 1Kohm 1/4W) 5mA
	//6V/6A (Load : 10ohm 10W) 600mA
	//Linear 5V/200A (Load : 10ohm 10W) 500mA
	//Switching 5V/200A (Load : 4.7ohm/4EA=1.175ohm 40W) 4255mA
	//2V/100A (Load : 4.7ohm/4EA=1.175ohm 40W) 1702mA
	//5V/50A (Load : 4.7ohm/2EA=2.35ohm 20W) 2128mA
	//50V/50A (Load : 200ohm 1EA)
	if(rangeV == 0){
		if(val1 > 0){
			val2 = myPs->config.maxCurrent[rangeI];
		}else{
			val2 = myPs->config.minCurrent[rangeI];
		}
	}else{
		val2 = myPs->config.minCurrent[rangeI];
	}

#if SHUNT_R_RCV == 1		//180705 add for shunt
	myData->CaliMeter.shunt_mOhm 
		= myData->cali.tmpCond[bd][ch].point[type][rangeV].shuntValue;
#else
	myData->CaliMeter.shunt_mOhm = myPs->pcu_config.caliV_ohm;
#endif
	ohm = myData->CaliMeter.shunt_mOhm;
	amp = myPs->pcu_config.caliV_amp;
	
	val2 = (val1 / ohm) * amp;			//JYG_141016

	switch(myCh->op.phase) {
		case P0:
			if(myData->CaliMeter.config.Shunt_Sel_Calibrator == 3){
				pCali_Ch_Select_auto_3(bd, ch, point);
			}
			myPs->signal[M_SIG_CALI_RELAY] = P0;
			myCh->signal[C_SIG_OUT_SWITCH] = P1;
	//		if(myCh->ChAttribute.chNo_master != P0)		//180108
	//			myCh->signal[C_SIG_OUT_SWITCH_ON] = P1;
			myCh->op.phase = P1;
			myCh->misc.cmd_v = val1;		//200115
			myCh->op.runTime = 0;
			myCh->misc.caliCheckPoint = 0;
			myCh->misc.caliCheckSum = 0;
			myCh->misc.caliCheckSum1 = 0;
			break;
		case P1:
			myCh->signal[C_SIG_V_RANGE] = (unsigned char)(rangeV+1);
			myCh->signal[C_SIG_I_RANGE] = (unsigned char)(rangeI+1);
			myCh->signal[C_SIG_RANGE_SWITCH] = P1;
	    	myCh->op.runTime += myPs->misc.rt_scan_time;
			if(myCh->op.runTime == 10) { //20200629 lyhw
				myCh->signal[C_SIG_OUT_SWITCH_ON] = P1;
			}
			if(myCh->op.runTime >= RTTASK_1500MS) {
				myCh->op.runTime = 0;
				myCh->op.phase = P2;
			}
			break;
		case P2:
			myCh->op.phase = P3;
			break;
		case P3:
			pcu_ref_output(bd, ch, STEP_IDLE, mode, val1, val2, 5, div, rangeV, rangeI);
			myCh->op.runTime = 0;
			myCh->op.phase = P4;
			break;
		case P4:
	    	myCh->op.runTime += myPs->misc.rt_scan_time;
			if(myCh->op.runTime >= myPs->pcu_config.cali_delay_time) {
				if(myData->AppControl.config.debugType != P0) {
				} else {
					send_msg(MODULE_TO_METER, MSG_MODULE_METER_REQUEST, bd, ch);
				}
				myCh->op.phase = P10;
				myCh->op.runTime = 0;
			}
			break;
		case P10: // meter, ad value read
			if(myData->AppControl.config.debugType != P0) {
				myCh->signal[C_SIG_METER_REPLY] = P0;
				myData->cali.tmpData[bd][ch].check_ad[type][rangeV][point]
					= (double)myCh->op.Vsens;
				myData->cali.tmpData[bd][ch].check_meter[type][rangeV][point]
					= myData->CaliMeter.value / 100.0;
				myCh->op.phase = P20;
			} else {	//180806 lyhw	
				data_gathering_check(bd, ch, type, rangeV, point);
			}
				/*
				if(myCh->signal[C_SIG_METER_REPLY] == P1) {
					myCh->signal[C_SIG_METER_REPLY] = P0;
//111215 detail calibration add
					myCh->misc.caliCheckPoint++;
					myCh->misc.caliCheckSum += (double)myCh->op.Vsens;
					myCh->misc.caliCheckSum1 +=	 
						 myData->CaliMeter.value / 100.0;
					if(myCh->misc.caliCheckPoint >= myCh->misc.checkCaliNum) {
						myData->cali.tmpData[bd][ch].check_ad[type][rangeV][point]
							= (double)(myCh->misc.caliCheckSum / 
											myCh->misc.checkCaliNum);
						myData->cali.tmpData[bd][ch].check_meter
							[type][rangeV][point] = (double)
							(myCh->misc.caliCheckSum1 / myCh->misc.checkCaliNum);
						myCh->op.phase = P20;
						myCh->misc.caliCheckPoint = 0;
						myCh->misc.caliCheckSum = 0;
						myCh->misc.caliCheckSum1 = 0;
					}else{
						myCh->op.phase = P4;
						myCh->op.runTime = 0;
					}*/
				/*	myData->cali.tmpData[bd][ch].check_ad[type][rangeV][point]
						= (double)myCh->op.Vsens;
					myData->cali.tmpData[bd][ch]
						.check_meter[type][rangeV][point]
						= myData->CaliMeter.value / 100.0;
					myCh->op.phase = P20;
*/

			/*	} else if(myCh->signal[C_SIG_METER_ERROR] == P1) {
					myCh->signal[C_SIG_METER_ERROR] = P0;
					myCh->signal[C_SIG_V_RANGE] = RANGE0;
					myCh->signal[C_SIG_I_RANGE] = RANGE0;
					myCh->op.phase = P25;
				}
			}*/
			break;
		case P20:
			ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, CMD_PCU_MODE_CC, 0,0);//CC0
			myCh->op.phase = P21;
			break;
		case P21:
//			myCh->signal[C_SIG_OUT_SWITCH] = P0;
//			myCh->signal[C_SIG_OUT_SWITCH_OFF] = P1;
			myCh->op.phase = P22;
			break;
		case P22:
			myCh->op.phase = P23;
			break;
		case P23:
			myCh->op.phase = P24;
			break;
		case P24:
	    	myCh->op.runTime += myPs->misc.rt_scan_time;
			if(myCh->op.runTime >= RTTASK_5000MS) {
				myCh->op.runTime = 0;
				myCh->op.phase = P30;
			}
			break;
		case P25:
			ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, CMD_PCU_MODE_CC, 0,0);//CC0
			myCh->op.phase = P26;
			break;
		case P26:
	    	myCh->op.runTime += myPs->misc.rt_scan_time;
//			if(myCh->op.runTime >= RTTASK_1500MS*2) {
			if(myCh->op.runTime >= myPs->pcu_config.cali_delay_time) {
				myCh->op.runTime = 0;
				myCh->signal[C_SIG_OUT_SWITCH] = P0;
				myCh->signal[C_SIG_OUT_SWITCH_OFF] = P1;
				myCh->op.state = C_STANDBY;
				myCh->op.phase = P0;
			}
			break;
		case P30:
			rtn = 1;
			break;
		default: break;
	}
	return rtn;
}

int pCalibrationI(int bd, int ch, int point)
{
	int rtn, type, rangeV, rangeI, div, calRelay, mode;
	long val1, val2;
	double tmp;
	rtn = 0;
	type = 1;
	rangeV = 0;
	rangeI = myData->cali.tmpCond[bd][ch].range;
	div = 15;
	mode = CC;

	val1 = myPs->config.maxVoltage[rangeV];

	val2 = myData->cali.tmpCond[bd][ch].point[type][rangeI].setPoint[point];
	
	if(val2 > myPs->config.maxCurrent[rangeI]) {
		val2 = myPs->config.maxCurrent[rangeI];
	} else if(val2 < myPs->config.minCurrent[rangeI]) {
		val2 = myPs->config.minCurrent[rangeI];
	}

	if(val2 < 0) val1 *=(-1);
	if(val1 > 0 && val2 > 0){
		calRelay = 1; //charge
	}else if(val1 <= 0 && val2 < 0){
		calRelay = 2; //discharge
	}else{
		calRelay = 0; //open
	}
	
#if SHUNT_R_RCV >= 1		//180705 add for shunt
	if(myData->mData.cali_hallCT == P1	//hun_210830
		|| myData->mData.cali_hallCT == P3){
		myData->CaliMeter.hallCT_ratio 
			= myData->cali.tmpCond[bd][ch].point[type][rangeI].shuntValue;
	}else{
		myData->CaliMeter.shunt_mOhm 
			= myData->cali.tmpCond[bd][ch].point[type][rangeI].shuntValue;
	}
#endif

    switch(myCh->op.phase) {
		case P0:
			if(myData->CaliMeter.config.Shunt_Sel_Calibrator == 3){
				pCali_Ch_Select_auto_3(bd, ch, point);
			}
			myPs->signal[M_SIG_CALI_RELAY] = P1;
			myCh->signal[C_SIG_OUT_SWITCH] = P1;
	//		myData->CaliMeter.shunt_mOhm = 1;
	//		if(myCh->ChAttribute.chNo_master != P0)		//180108
				myCh->signal[C_SIG_OUT_SWITCH_ON] = P1;
			if(calRelay == 1){
				myPs->signal[M_SIG_CALI_DISCHARGE_RELAY] = P0;
			}else if(calRelay == 2){
				myPs->signal[M_SIG_CALI_CHARGE_RELAY] = P0;
			}else{
				myPs->signal[M_SIG_CALI_CHARGE_RELAY] = P0;
				myPs->signal[M_SIG_CALI_DISCHARGE_RELAY] = P0;
			}
			myCh->op.phase = P1;
			myCh->misc.cmd_i = val2;		//200115
			myCh->op.runTime = 0;
			myCh->misc.caliCheckPoint = 0;
			myCh->misc.caliCheckSum = 0;
			myCh->misc.caliCheckSum1 = 0;
			break;
		case P1:
			myCh->signal[C_SIG_V_RANGE] = (unsigned char)(rangeV+1);
			myCh->signal[C_SIG_I_RANGE] = (unsigned char)(rangeI+1);
			myCh->signal[C_SIG_RANGE_SWITCH] = P1;
			myCh->op.preType = STEP_IDLE;
	    	myCh->op.runTime += myPs->misc.rt_scan_time;
			if(myCh->op.runTime >= RTTASK_1500MS) {
				ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, CMD_PCU_MODE_CC, 0,0);//CC0
				myCh->op.runTime = 0;
				myCh->op.phase = P2;
			}else{
				if(calRelay == 1){
					myPs->signal[M_SIG_CALI_CHARGE_RELAY] = P1;
				}else if(calRelay == 2){
					myPs->signal[M_SIG_CALI_DISCHARGE_RELAY] = P1;
				}else{
					myPs->signal[M_SIG_CALI_CHARGE_RELAY] = P0;
					myPs->signal[M_SIG_CALI_DISCHARGE_RELAY] = P0;
				}
			}
			break;
		case P2:
			//190618 lyhw add 
	    	myCh->op.runTime += myPs->misc.rt_scan_time;
			if(myCh->op.runTime >= 20) {
				myCh->op.runTime = 0;
				myCh->op.phase = P3;
			}
			break;
		case P3:
			pcu_ref_output(bd, ch, STEP_IDLE, mode, val1, val2, div, div, rangeV, rangeI);
			myCh->op.runTime = 0;
			myCh->op.phase = P4;
			break;
		case P4:
//	    	myCh->op.runTime += 40;
//			if(myCh->op.runTime >= 1000) {
	    	myCh->op.runTime += myPs->misc.rt_scan_time;
			if(myCh->op.runTime >= myPs->pcu_config.cali_delay_time) {
				send_msg(MODULE_TO_METER, MSG_MODULE_METER_REQUEST, bd, ch);
				myCh->op.phase = P10;
				myCh->op.runTime = 0;
				if(myData->mData.cali_meas_type == MEAS){	//180730 lyhw
					myCh->op.phase = P7;
				}
			}
			break;
		case P5:
			if(myCh->signal[C_SIG_METER_REPLY] == P1) {
				myCh->signal[C_SIG_METER_REPLY] = P0;
				if(myPs->config.ratioCurrent == MICRO) { //uA
					myData->CaliMeter.avgValue 
								= myData->CaliMeter.value / 100.0;
				} else { //nA
					myData->CaliMeter.avgValue 
								= myData->CaliMeter.value * 10.0;
				}
				myCh->op.phase = P6;
				myCh->op.runTime = 0;
			}
			break;
		case P6:
	    	myCh->op.runTime += myPs->misc.rt_scan_time;
			if(myCh->op.runTime >= myPs->pcu_config.cali_delay_time) {
					send_msg(MODULE_TO_METER, MSG_MODULE_METER_REQUEST, bd, ch);
				myCh->op.phase = P10;
				myCh->op.runTime = 0;
				if(myData->mData.cali_meas_type == MEAS){
					myCh->op.phase = P7;
				}
			}
			break;
		case P7:	//180730 add for meas digital
			myCh->op.runTime += myPs->misc.rt_scan_time;
			if(myCh->op.runTime % 100 == 0){
				send_msg(MODULE_TO_METER, MSG_MODULE_METER_REQUEST, bd, ch);

				if(myCh->signal[C_SIG_METER_REPLY] == P1) {
					myCh->signal[C_SIG_METER_REPLY] = P0;

					myData->cali.tmpData[bd][ch]
						.set_ad[type][rangeI][0]
						= (double)myCh->op.Isens;
					myData->cali.tmpData[bd][ch]
						.set_meter[type][rangeI][0] 
						=  return_meter_value(bd, ch, type);

					send_msg(MODULE_TO_DATASAVE,
					MSG_MODULE_DATASAVE_CALI_NORMAL_RESULT_SAVE,
					bd, ch);
					myCh->op.phase = P7;	//180803
					rtn = 0;
					if(	myCh->op.runTime >= myData->mData.cali_meas_time){
						rtn = 1;
						break;
					}
				} else if(myCh->signal[C_SIG_METER_ERROR] == P1) {
					myCh->signal[C_SIG_METER_ERROR] = P0;
					myCh->signal[C_SIG_OUT_SWITCH] = P0;
					myCh->signal[C_SIG_V_RANGE] = RANGE0;
					myCh->signal[C_SIG_I_RANGE] = RANGE0;
					myCh->op.state = C_STANDBY;
					myCh->op.phase = P0;
				}
			}
			break;

		case P10: // meter, ad value read
			if(myData->AppControl.config.debugType != P0) { //for cali debug
				myCh->signal[C_SIG_METER_REPLY] = P0;
				myData->cali.tmpData[bd][ch].set_ad[type][rangeI][point]
					= myData->cali.orgAD[type];
				switch(myData->CaliMeter.config.measureI) {
					case MEASURE_I_3: //DCCT 600A/400mA
						tmp = myData->CaliMeter.value / 40.0 * 600.0;
						break;
					case MEASURE_I_4: //DCCT 150A/200mA
						tmp = myData->CaliMeter.value / 20.0 * 150.0;
						break;
					default: //meter DCI
						if(myPs->config.ratioCurrent == MICRO) { //uA
							tmp = myData->CaliMeter.value / 100.0;
						} else { //nA
							tmp = myData->CaliMeter.value * 10.0;
						}
				}

				myData->cali.tmpData[bd][ch].set_meter[type][rangeI][point]
					= tmp;
				myCh->op.phase = P20;
			} else {	//180805 lyhw
				data_gathering_cali(bd, ch , type, rangeV, point);
			}
			/*	if(myCh->signal[C_SIG_METER_REPLY] == P1) {
					myCh->signal[C_SIG_METER_REPLY] = P0;
					switch(myData->CaliMeter.config.measureI) {
						case MEASURE_I_3: //DCCT 600A/400mA
							tmp = myData->CaliMeter.value / 40.0 * 600.0;
							break;
						case MEASURE_I_4: //DCCT 150A/200mA
							tmp = myData->CaliMeter.value / 20.0 * 150.0;
							break;
						default: //meter DCI
							if(myPs->config.ratioCurrent == MICRO) { //uA
								tmp = myData->CaliMeter.value / 100.0;
							} else { //nA
								tmp = myData->CaliMeter.value * 10.0;
							}
							break;
					}
					//111215 detail calibration add
					myCh->misc.caliCheckPoint++;
					myCh->misc.caliCheckSum += myData->cali.orgAD[type];
					myCh->misc.caliCheckSum1 +=	 
						(tmp + (double)myData->CaliMeter.config.I_offset);
					if(myCh->misc.caliCheckPoint >= myPs->pcu_config.setCaliNum) {
						myData->cali.tmpData[bd][ch].set_ad[type][rangeI][point]
							= (double)(myCh->misc.caliCheckSum / 
											myPs->pcu_config.setCaliNum);
						myData->cali.tmpData[bd][ch].set_meter
							[type][rangeI][point] = (double)
							(myCh->misc.caliCheckSum1 / myPs->pcu_config.setCaliNum);
						myCh->op.phase = P20;
						myCh->op.runTime = 0;
						myCh->misc.caliCheckPoint = 0;
						myCh->misc.caliCheckSum = 0;
						myCh->misc.caliCheckSum1 = 0;
					}else{
						myCh->op.phase = P4;
						myCh->op.runTime = 0;
					}
				} else if(myCh->signal[C_SIG_METER_ERROR] == P1) {
					myCh->signal[C_SIG_METER_ERROR] = P0;
					myCh->signal[C_SIG_V_RANGE] = RANGE0;
					myCh->signal[C_SIG_I_RANGE] = RANGE0;
					myCh->op.runTime = 0;
					myCh->op.phase = P25;
				}
			}*/
			break;
		case P20:
			ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, CMD_PCU_MODE_CC, 0,0);//CC0
			myCh->op.phase = P21;
			break;
		case P21:
//			myCh->signal[C_SIG_OUT_SWITCH] = P0;
//			myCh->signal[C_SIG_OUT_SWITCH_OFF] = P1;
			myCh->op.phase = P30;
			break;
		case P25:
			ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, CMD_PCU_MODE_CC, 0,0);//CC0
			myCh->op.phase = P26;
			break;
		case P26:
			myCh->signal[C_SIG_OUT_SWITCH] = P0;
			myCh->signal[C_SIG_OUT_SWITCH_OFF] = P1;
			myCh->op.state = C_STANDBY;
			myCh->op.phase = P0;
			break;
		case P30:
			rtn = 1;
			break;
		default: break;
	}
	return rtn;
}

int pCalibrationCheckI(int bd, int ch, int point)
{
	int rtn, type, rangeV, rangeI, div, calRelay, mode;
	long val1, val2;
//	double tmp;

	rtn = 0;
	type = 1;
	rangeV = 0;
	rangeI = myData->cali.tmpCond[bd][ch].range;
	div = 5;
	mode = CC;

	val1 = myPs->config.maxVoltage[rangeV];

	val2 = myData->cali.tmpCond[bd][ch].point[type][rangeI].checkPoint[point];
	if(val2 > myPs->config.maxCurrent[rangeI]) {
		val2 = myPs->config.maxCurrent[rangeI];
	} else if(val2 < myPs->config.minCurrent[rangeI]) {
		val2 = myPs->config.minCurrent[rangeI];
	}

	if(val2 < 0) val1 *=(-1);
	if(val1 > 0 && val2 > 0){
		calRelay = 1; //charge
	}else if(val1 < 0 && val2 < 0){
		calRelay = 2; //discharge
	}else{
		calRelay = 0; //open
	}
#if SHUNT_R_RCV >= 1		//180705 add for shunt
	if(myData->mData.cali_hallCT == P1		//hun_210830
		|| myData->mData.cali_hallCT == P3){
		myData->CaliMeter.hallCT_ratio 
			= myData->cali.tmpCond[bd][ch].point[type][rangeI].shuntValue;
	}else{
		myData->CaliMeter.shunt_mOhm 
			= myData->cali.tmpCond[bd][ch].point[type][rangeI].shuntValue;
	}
#endif

    switch(myCh->op.phase) {
		case P0:
			if(myData->CaliMeter.config.Shunt_Sel_Calibrator == 3){
				pCali_Ch_Select_auto_3(bd, ch, point);
			}
			myPs->signal[M_SIG_CALI_RELAY] = P1;
			myCh->signal[C_SIG_OUT_SWITCH] = P1;
//			myCh->signal[C_SIG_OUT_SWITCH_ON] = P1;
			myCh->op.phase = P1;
			if(calRelay == 1){
				myPs->signal[M_SIG_CALI_DISCHARGE_RELAY] = P0;
			}else if(calRelay == 2){
				myPs->signal[M_SIG_CALI_CHARGE_RELAY] = P0;
			}else{
				myPs->signal[M_SIG_CALI_DISCHARGE_RELAY] = P0;
				myPs->signal[M_SIG_CALI_CHARGE_RELAY] = P0;
			}
			myCh->misc.cmd_i = val2;		//200115
			myCh->op.runTime = 0;
			myCh->misc.caliCheckPoint = 0;
			myCh->misc.caliCheckSum = 0;
			myCh->misc.caliCheckSum1 = 0;
			break;
		case P1:
			myCh->signal[C_SIG_V_RANGE] = (unsigned char)(rangeV+1);
			myCh->signal[C_SIG_I_RANGE] = (unsigned char)(rangeI+1);
			myCh->signal[C_SIG_RANGE_SWITCH] = P1;
			myCh->op.preType = STEP_IDLE;
	    	myCh->op.runTime += myPs->misc.rt_scan_time;
			if(myCh->op.runTime == 10) { //20200629 lyhw
				myCh->signal[C_SIG_OUT_SWITCH_ON] = P1;
			}
			if(myCh->op.runTime >= RTTASK_1500MS) {
				ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, CMD_PCU_MODE_CC, 0,0);//CC0
				myCh->op.runTime = 0;
				myCh->op.phase = P2;
			}else{
				if(calRelay == 1){
					myPs->signal[M_SIG_CALI_CHARGE_RELAY] = P1;
				}else if(calRelay == 2){
					myPs->signal[M_SIG_CALI_DISCHARGE_RELAY] = P1;
				}else{
					myPs->signal[M_SIG_CALI_CHARGE_RELAY] = P0;
					myPs->signal[M_SIG_CALI_DISCHARGE_RELAY] = P0;
				}
			}
			break;
		case P2:
			//190618 lyhw add
	    	myCh->op.runTime += myPs->misc.rt_scan_time;
			if(myCh->op.runTime >= 20) {
				myCh->op.runTime = 0;
				myCh->op.phase = P3;
			}
			break;
		case P3:
			pcu_ref_output(bd, ch, STEP_IDLE, mode, val1, val2, 15, div, rangeV, rangeI);
			myCh->op.runTime = 0;
			myCh->op.phase = P4;
	    	break;
		case P4:
	    	myCh->op.runTime += myPs->misc.rt_scan_time;
			if(myCh->op.runTime >= myPs->pcu_config.cali_delay_time) {
				if(myData->AppControl.config.debugType != P0) {
				} else {
					send_msg(MODULE_TO_METER, MSG_MODULE_METER_REQUEST, bd, ch);
				}
				myCh->op.phase = P10;
				myCh->op.runTime = 0;
			}
			break;
		case P5:
			if(myCh->signal[C_SIG_METER_REPLY] == P1) {
				myCh->signal[C_SIG_METER_REPLY] = P0;
				if(myPs->config.ratioCurrent == MICRO) { //uA
					myData->CaliMeter.avgValue = myData->CaliMeter.value / 100.0;
				} else { //nA
					myData->CaliMeter.avgValue = myData->CaliMeter.value * 10.0;
				}
			}
			myCh->op.phase = P6;
			myCh->op.runTime = 0;
			break;
		case P6:
	    	myCh->op.runTime += myPs->misc.rt_scan_time;
			if(myCh->op.runTime >= (RTTASK_1500MS)) {
				if(myData->AppControl.config.debugType != P0) {
				} else {
					send_msg(MODULE_TO_METER, MSG_MODULE_METER_REQUEST, bd, ch);
				}
				myCh->op.phase = P10;
				myCh->op.runTime = 0;
			}
			break;
		case P10:
			if(myData->AppControl.config.debugType != P0) {
			} else {	//180805 lyh
				data_gathering_check(bd, ch, type, rangeV, point);
			}
			/*
				if(myCh->signal[C_SIG_METER_REPLY] == P1) {
					myCh->signal[C_SIG_METER_REPLY] = P0;
					switch(myData->CaliMeter.config.measureI) {
						case MEASURE_I_3: //DCCT 600A/400mA
							tmp = myData->CaliMeter.value / 40.0 * 600.0;
							break;
						case MEASURE_I_4: //DCCT 150A/200mA
							tmp = myData->CaliMeter.value / 20.0 * 150.0;
							break;
						default: //meter DCI
							if(myPs->config.ratioCurrent == MICRO) { //uA
								tmp = myData->CaliMeter.value / 100.0;
							} else { //nA
								tmp = myData->CaliMeter.value * 10.0;
							}
							break;
					}
					//111215 detail calibration add
					myCh->misc.caliCheckPoint++;
					myCh->misc.caliCheckSum += (double)myCh->op.Isens;
					myCh->misc.caliCheckSum1 +=	 
						(tmp + (double)myData->CaliMeter.config.I_offset);
					if(myCh->misc.caliCheckPoint >= myCh->misc.checkCaliNum) {
						myData->cali.tmpData[bd][ch].check_ad[type][rangeI][point]
							= (double)(myCh->misc.caliCheckSum / 
											myCh->misc.checkCaliNum);
						myData->cali.tmpData[bd][ch].check_meter
							[type][rangeI][point] = (double)
							(myCh->misc.caliCheckSum1 / myCh->misc.checkCaliNum);
						myCh->op.phase = P20;
						myCh->misc.caliCheckPoint = 0;
						myCh->misc.caliCheckSum = 0;
						myCh->misc.caliCheckSum1 = 0;
					}else{
						myCh->op.phase = P4;
						myCh->op.runTime = 0;
					}
		//			myData->cali.tmpData[bd][ch]
		//				.check_meter[type][rangeI][point]
		//				= tmp + (double)myData->CaliMeter.config.I_offset;
		//			myCh->op.phase = P20;
				} else if(myCh->signal[C_SIG_METER_ERROR] == P1) {
					myCh->signal[C_SIG_METER_ERROR] = P0;
					myCh->signal[C_SIG_V_RANGE] = RANGE0;
					myCh->signal[C_SIG_I_RANGE] = RANGE0;
					myCh->op.phase = P25;
				}
			}*/
			break;
		case P20:
			ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, CMD_PCU_MODE_CC, 0,0);//CC0
			myCh->op.phase = P21;
			break;
		case P21:
		//	myCh->signal[C_SIG_OUT_SWITCH] = P0;
		//	myCh->signal[C_SIG_OUT_SWITCH_OFF] = P1;
			myCh->op.phase = P30;
			break;
		case P25:
			ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, CMD_PCU_MODE_CC, 0,0);//CC0
			myCh->op.phase = P26;
			break;
		case P26:
			myCh->signal[C_SIG_OUT_SWITCH] = P0;
			myCh->signal[C_SIG_OUT_SWITCH_OFF] = P1;
			myCh->op.state = C_STANDBY;
			myCh->op.phase = P0;
			break;
		case P30:
			rtn = 1;
			break;
		default: break;
    }
	return rtn;
}

void pcu_ref_output(int bd, int ch, unsigned char type, unsigned char mode, long par1, long par2, int div_v, int div_i, int rangeV, int rangeI)
{
	long val1, val2;

	myCh = &(myData->bData[bd].cData[ch]);

	if(myCh->op.state != C_CALI){
		if(mode == CP) mode = CPCV;		//180615
		if(mode == CC || mode == CV){ 
			mode = CCCV; 				//190107
		}
	}
	
	if(mode == CP){						//cali mode
		val1 = par1;
		val2 = 0;
	}else if(mode == CPCV){
		val1 = par2;					//WATT
		val2 = pCalCmdV(bd, ch, par1, div_v, rangeV);
	}else{
		//1. voltage Ref out
		if(mode == CC){ 
			val1 = 0;
		}else{
			val1 = pCalCmdV(bd, ch, par1, div_v, rangeV);
		}
		//2. current Ref out
		if(type == STEP_USER_PATTERN && par2 == 0){
			val2 = 0;
		}else{
			val2 = pCalCmdI(bd, ch, par2, div_i, rangeI);
		}
	}
	
	if(myCh->ChAttribute.chNo_master == P0){		//180108 output 
		ch_mode_set(bd, ch, type, mode, val1, val2);
	}else{
		if(myCh->ChAttribute.opType == P0){		
			ch_mode_set(bd, ch, type, mode, val1, val2);
		}
	}
}

void pConvert_Data(int bd, int ch, int type)
{
	int range, i, point, ch_div;
	int base_addr, addr_step, addr1, addr;
	unsigned char	mode;
	long	tmp1;
	double tmp;
	int parallel_ch;

	S_CH_STEP_INFO	step;

	step = step_info(bd, ch);
	myCh = &(myData->bData[bd].cData[ch]);

	mode = step.mode;
	base_addr = myPs->addr.main[BASE_ADDR];
    addr_step = myPs->addr.main[ADDR_STEP];
	ch_div = myPs->pcu_config.portPerCh;
	if(ch < 32){
	addr1 = base_addr + addr_step * (ch / ch_div);
	}else{
	addr1 = base_addr + addr_step * ((ch-32) / ch_div);
	}

	switch(type) {
		case 0:
			range = (int)myData->bData[bd].cData[ch].signal[C_SIG_V_RANGE];
			if(range <= 0) range = 0;
			else range -= 1;
/*
			if(myCh->ChAttribute.opType == P1) {
				addr = addr1 + CREG_CTRL_DUTV;
			}else{
				addr = addr1 + CREG_CTRL_DUTV_CH1 + ((ch % 2) * 0x03);
			}
			tmp1 = (long)(SREG16_read(addr)*100.0) + 2500000;
			tmp = (double)tmp1;
*/

			if(myCh->ChAttribute.opType == P1 || ch_div == 1) {
				addr = addr1 + CREG_CTRL_DUTVH_CH1;
			}else{
				addr = addr1 + CREG_CTRL_DUTVH_CH1 + ((ch % ch_div) * 0x04);
			}

			tmp1 = SREG16_read(addr);
			tmp1 = tmp1 << 16;

			if(myCh->ChAttribute.opType == P1 || ch_div == 1) {
				addr = addr1 + CREG_CTRL_DUTVL_CH1;
			}else{
				addr = addr1 + CREG_CTRL_DUTVL_CH1 + ((ch % ch_div) * 0x04);
			}

			tmp1 |= SREG16_read(addr);
			tmp = tmp1 * 100.0;

			i = myData->bData[bd].cData[ch].misc.sensCount;
			if(myCh->ChAttribute.chNo_master == P0){
				parallel_ch = ch - 1;
			}else{
				parallel_ch = ch;
			}
	//		point = cFindADCaliPoint(bd, ch, (long)tmp, type, range);
			point = cFindADCaliPoint(bd, parallel_ch, (long)tmp, type, range);
			if(myData->bData[bd].cData[ch].op.state == C_CALI
							&& myData->mData.cali_meas_type != MEAS){
				if(myData->bData[bd].cData[ch].signal[C_SIG_CALI_PHASE] < P70) {
					myData->bData[bd].cData[ch].misc.sensSumV[i] = (long)tmp;
				}else{
					myData->bData[bd].cData[ch].misc.sensSumV[i]
						= (long)(tmp * myData->cali.tmpData[bd][ch]
							.AD_A[type][range][point]
						+ myData->cali.tmpData[bd][ch]
							.AD_B[type][range][point]);
				}
			} else {
				myData->bData[bd].cData[ch].misc.sensSumV[i]
				= (long)(tmp
				* myData->cali.data[bd][parallel_ch].AD_A[type][range][point]
				+ myData->cali.data[bd][parallel_ch].AD_B[type][range][point]);
			}
			break;
		case 1:
			range = (int)myData->bData[bd].cData[ch].signal[C_SIG_I_RANGE];
			if(range <= 0) range = 0;
			else range -= 1;

			if(myCh->ChAttribute.opType == P1 || ch_div == 1) {
				addr = addr1 + CREG_CTRL_DUTIH_CH1;
			}else{
				addr = addr1 + CREG_CTRL_DUTIH_CH1 + ((ch % ch_div) * 0x04);
			}
			tmp1 = SREG16_read(addr);
			tmp1 = tmp1 << 16;
			if(myCh->ChAttribute.opType == P1 || ch_div == 1) {
				addr = addr1 + CREG_CTRL_DUTIL_CH1;
			}else{
				addr = addr1 + CREG_CTRL_DUTIL_CH1 + ((ch % ch_div) * 0x04);
			}
			tmp1 |= SREG16_read(addr);
			tmp = tmp1 * 100.0;

			i = myData->bData[bd].cData[ch].misc.sensCount;
			point = cFindADCaliPoint(bd, ch, (long)tmp, type, range);
			if(myData->bData[bd].cData[ch].op.state == C_CALI
							&& myData->mData.cali_meas_type != MEAS){
				if(myData->bData[bd].cData[ch].signal[C_SIG_CALI_PHASE] < P81) {
					myData->bData[bd].cData[ch].misc.sensSumI[i] = (long)tmp;
				} else {
					myData->bData[bd].cData[ch].misc.sensSumI[i]
						= (long)(tmp * myData->cali.tmpData[bd][ch]
							.AD_A[type][range][point]
						+ myData->cali.tmpData[bd][ch]
							.AD_B[type][range][point]);
				}
			} else {
				myData->bData[bd].cData[ch].misc.sensSumI[i]
					= (long)(tmp
					* myData->cali.data[bd][ch].AD_A[type][range][point]
					+ myData->cali.data[bd][ch].AD_B[type][range][point]);
				//20200123 lyhw add for Ad_offset
				if(myPs->config.capacityType == CAPACITY_AMPARE_HOURS){
					if(myData->bData[bd].cData[ch].op.type == STEP_CHARGE){
						myData->bData[bd].cData[ch].misc.sensSumI[i]
							+= (long)(myPs->config.maxCurrent[range]
							* myPs->config.AD_offset);
					}
				}
			}
			break;
		default: break;
	}
}

void pCalChAverage(int bd, int ch)
{
	pConvert_Data(bd, ch, 0);
	pConvert_Data(bd, ch, 1);
	cCalculate_Voltage(bd, ch);
	cCalculate_Current(bd, ch);
	if(myData->bData[bd].cData[ch].op.type == STEP_REST) return;
	cCalculate_Capacity(bd, ch);
	cCalculate_Watt(bd, ch);
}

void pcu_user_pattern_data(int bd, int ch)
{
	long val1, val2;
	unsigned long advStepNo;
	int div = 5, rangeV, rangeI;
	unsigned char	type, mode;

	typedef struct pulse1 {
		unsigned long	par1 : 30;
		unsigned long	par2 : 2;
	}PULSE1;

	typedef struct pulse2 {
		unsigned long	par1 : 27;
		unsigned long	par2 : 3;
		unsigned long	par3 : 2;
	}PULSE2;
	
	typedef union d_pulse {
		PULSE1	data;
		unsigned long	val;
	} D_PULSE1;
	
	typedef union d_pulse2{
		PULSE2	data;
		unsigned long	val;
	} D_PULSE2;

	S_CH_STEP_INFO	step;
	S_USER_PATTERN_DATA pattern;
	D_PULSE1	pulse1;
	D_PULSE2	pulse2;

	memset((char *)&pulse1, 0, sizeof(D_PULSE1));
	memset((char *)&pulse2, 0, sizeof(D_PULSE2));

	myCh = &(myData->bData[bd].cData[ch]);
	pattern = pattern_info(bd, ch);
	step = step_info(bd, ch);

	rangeV = step.rangeV;
	rangeI = step.rangeI;
	type = step.type;
	mode = step.mode;
    advStepNo = step.advStepNo;
	/*
	switch(step.shape){
		case P1:
		case P2:
			pulse1.data.par1 = pattern.time * 5;
			pulse1.data.par2 = step.shape;
			break;
		case P3:
			pulse2.data.par1 = step.refV_H * 0.001;
			pulse2.data.par1 = 0;
			pulse2.data.par2 = step.refI;
			pulse2.data.par3 = step.shape;
			break;
		default: break;
	}*/

	if(pattern.type == PS_CURRENT){
		val2 = step.refI;
		if(val2 >= 0){
			val1 = myData->mData.testCond[bd][ch].step[advStepNo].refV_H;
			if(val1 > myPs->config.maxVoltage[0]){
				val1 = myPs->config.maxVoltage[0];
			}else if(val1 == 0){	//190619 add
				val1 = myPs->config.maxVoltage[0];
			}
		}else{
			val1 = myData->mData.testCond[bd][ch].step[advStepNo].refV_L;
			if(val1 < myPs->config.minVoltage[0]){
				val1 = myPs->config.minVoltage[0];
			}else if(val1 == 0){	//190619 add
				val1 = myPs->config.minVoltage[0];
			}
		}
	}else{
		val2 = step.refP;
		if(val2 >= 0){
			val1 = myData->mData.testCond[bd][ch].step[advStepNo].refV_H;
			if(val1 > myPs->config.maxVoltage[0]){
				val1 = myPs->config.maxVoltage[0];
			}else if(val1 == 0){ 	//190619 add
				val1 = myPs->config.maxVoltage[0];
			}
		}else{
			val1 = myData->mData.testCond[bd][ch].step[advStepNo].refV_L;
			if(val1 < myPs->config.minVoltage[0]){
				val1 = myPs->config.minVoltage[0];
			}else if(val1 == 0){	//190619 add
				val1 = myPs->config.minVoltage[0];
			}
		}
	}
	myCh->misc.sensCount = 0;
	myCh->misc.sensCountFlag = P0;
	myCh->misc.sensBufCount = 0;
	myCh->misc.sensBufCountFlag = P0;
	myCh->misc.fbSumI_M = 0;
	myCh->misc.fbCountI_M = 0;
	myCh->misc.fbSumI_H = 0;
	myCh->misc.fbCountI_H = 0;
	myCh->misc.fbSumI_L = 0;
	myCh->misc.fbCountI_L = 0;

//	if(flag)
//		ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, CMD_PCU_MODE_CC, 0,0);
//	else
	pcu_ref_output(bd,ch,type,mode,val1,val2,div,div,rangeV,rangeI);
}

void pCali_Ch_Select_auto_3(int bd, int ch, int point)
{	//180726 add lyhw
	unsigned char caliVal=0, typeVal;
	long current_val;
	int addr, realCh, range, type, relay_ch = 0;
#if CYCLER_TYPE == LINEAR_CYC || CYCLER_TYPE == CAN_CYC
	unsigned char tmpVal=0;
#endif
	// type : 0 (voltage), type : 1 (current)
	// range : 0(HIGH), 1(MIDDLE), 2(LOW), 3(LOWLOW)
	// bits-   7   |   6   |   5   |   4   |   3   |   2   |   1   |   0   |
	// x601- D0_B8 | D0_B7 | D0_B6 | D0_B5 | D0_B3 | D0_B3 | D0_B2 | D0_B1 |
	// x602- D0_B16| D0_B15| D0_B14| D0_B13| D0_B12| D0_B11| D0_B10| D0_B9 |
	// 74HCT138 <---- active low

	typeVal = 0;
	current_val = 0;
	
	range = myData->cali.tmpCond[bd][ch].range;
	type = myData->cali.tmpCond[bd][ch].type;
	//kjc_210824
	if(ch < 32){
		if(ch > 15){
			relay_ch = ch - 16;
		}else{
			relay_ch = ch;
		}
	}else{
		if(ch > 47){
			relay_ch = ch - 48;
		}else{
			relay_ch = ch - 32;
		}
	}
	
	//180914 Use Smps Current Cali
#if CYCLER_TYPE == DIGITAL_CYC
	if(relay_ch < 16){
		switch(type){				//1. select Voltage / current
			case 0:					//2. voltage
				if(myData->mData.cali_hallCT != P2	//hun_210830
					|| myData->mData.cali_hallCT != P3){
					addr = 0x601;
					typeVal = 0x01;
					outb(typeVal, addr);
				}
				break;
			case 1:					//3. current
				addr = 0x601;
				typeVal = 0x10;
				typeVal |= 0x20;		
				outb(typeVal, addr);
				break;
			default:
				break;
		}
		//4. select cali for Ch
		if(relay_ch < 8){ 
			addr = 0x602;
			caliVal = relay_ch ^ 0x3f;
			caliVal |= 0x40;		//IC Enable
			outb(caliVal, addr);	
		}else{
			addr = 0x602;
			realCh = relay_ch % 8;
			caliVal = (realCh << 3) ^ 0x3f;
			caliVal |= 0x80;		//IC Enable
			outb(caliVal, addr);	
		}
	
		if(myData->mData.cali_hallCT == P2		//hun_210830
			|| myData->mData.cali_hallCT == P3){ 	
			switch(type){				//1. select Voltage / current
				case 0:					//2. voltage
					//181011 add
					myPs->signal[M_SIG_CALI_VOLTAGE2_RELAY] = P1;
					break;
				case 1:					//3. current
					myPs->signal[M_SIG_CALI_VOLTAGE2_RELAY] = P0;
					if(myCh->signal[C_SIG_CALI_PHASE] == P80) {
						current_val = myData->cali.tmpCond[bd][ch].
										point[type][range].setPoint[point];
					}else{
						current_val = myData->cali.tmpCond[bd][ch].
										point[type][range].checkPoint[point];
					}

					if(current_val >= 0){	//current Charge
						myPs->signal[M_SIG_CALI_CHARGE2_RELAY] = P1;
						myPs->signal[M_SIG_CALI_DISCHARGE2_RELAY] = P0;
					}else{					//current DisCharge
						myPs->signal[M_SIG_CALI_DISCHARGE2_RELAY] = P1;
						myPs->signal[M_SIG_CALI_CHARGE2_RELAY] = P0;
					}
					break;
				default:
					break;
			}
		}
	}
#else
	addr = 0x607;
	realCh = (bd * myPs->config.chPerBd) + relay_ch;
	tmpVal = realCh / 8;
	caliVal = tmpVal << 4;
	caliVal |= realCh % 8;
	outb(caliVal, addr);	
#endif
}

void PCU_State_Check(int bd, int ch)
{	//190424 lyhw
	int base_addr, addr_step, addr1, addr, ch_div;
	
	myCh = &(myData->bData[bd].cData[ch]);

	base_addr = myPs->addr.main[BASE_ADDR];
	addr_step = myPs->addr.main[ADDR_STEP];
	ch_div = myPs->pcu_config.portPerCh;
	
	if(myData->AppControl.config.debugType != P0){
		if(myCh->signal[C_SIG_OUT_SWITCH_ON] == P3){
			myCh->pcu_misc.Rcv_ch_relay = ON;
		}
		if(myCh->signal[C_SIG_OUT_SWITCH_OFF] == P3){
			myCh->pcu_misc.Rcv_ch_relay = OFF;
		}
		if(myCh->signal[C_SIG_PARALLEL_SWITCH_ON] == P3){
			myCh->pcu_misc.Rcv_parallel_relay = ON;
		}
		if(myCh->signal[C_SIG_PARALLEL_SWITCH_OFF] == P3){
			myCh->pcu_misc.Rcv_parallel_relay = OFF;
		}		
		return;
	}
	
	if(ch < 32){
		addr1 = base_addr + addr_step * (ch / ch_div);
	} else{
		addr1 = base_addr + addr_step * ((ch-32) / ch_div);
	}
	addr = addr1 + CREG_CTRL_CCU1_STATE + (ch % ch_div);
	//1. Check PCU state	
	myCh->pcu_misc.Rcv_mode = (SREG8_read1(addr) & 0xf0) >> 4;
	myCh->pcu_misc.Rcv_response = SREG8_read2(addr);

	myCh->pcu_misc.Rcv_err_code = (myCh->pcu_misc.Rcv_response >> 4) & 0x0F;
	myCh->pcu_misc.Rcv_ch_relay = (myCh->pcu_misc.Rcv_response & 0x02) >> 1;
	myCh->pcu_misc.Rcv_parallel_relay 
								= (myCh->pcu_misc.Rcv_response & 0x04) >> 2;
}

void PCU_Inverter_Control(int bd,int ch)
{	//180627 lyhw
	if(CYCLER_TYPE == LINEAR_CYC) return;
	if(CYCLER_TYPE == CAN_CYC) return;

	if(myData->AppControl.config.debugType != P0){
		myCh->inv_power = P10;
		return;
	}
	
	PCU_Inverter_OnOff(bd, ch);
	if(myPs->pcu_config.inverterType == 0){
		//for jiwoo inverter
		PCU_Inverter_Fault(bd, ch);
	}else{
		//210304 for panda inverter
		PCU_Inverter_Fault_DC(bd, ch);
		PCU_Inverter_Fault_AC(bd, ch);
	}
	CH_Inverter_Signal(bd, ch);	//180820 add
	PCU_INV_State(bd, ch);
}		

void PCU_INV_State(int bd,int ch)
{	//200417 lyhw
	unsigned char InvMaster_ch;
	int inv_num1, inv_div;

	myCh = &(myData->bData[bd].cData[ch]);

	inv_div = myPs->pcu_config.invPerCh;
	inv_num1 = ch / inv_div;
	InvMaster_ch = inv_num1 * inv_div;

	myCh->pcu_misc.InvState = myData->bData[bd].cData[InvMaster_ch].inv_power;
}

void PCU_Inverter_Fault(int bd, int ch)
{	//180626 lyhw
	int i, j, base_addr, addr_step, addr1, addr, inv_p, inv_div;
	int inv_num ,real_ch, ch_div, master_ch;
	unsigned char useFlag;
   //	run_check = 0;
	
	if(myCh->inv_power < P10) return;
	
	ch_div = myPs->pcu_config.portPerCh;
	inv_div = myPs->pcu_config.invPerCh;
	inv_p = myPs->pcu_config.parallel_inv_ch;
	inv_num = ch / inv_div;
	
	//1. Check master ch
	master_ch = ch % inv_div;
	if(master_ch != P0) return;
	//2. Inverter use flag check
	useFlag = myData->dio.din.pcu_inUseFlag[inv_num];
	if(useFlag == P0) return;
	
	base_addr = myPs->addr.main[BASE_ADDR];
	addr_step = myPs->addr.main[ADDR_STEP];
	if(ch < 32){
		addr1 = base_addr + addr_step * (ch / ch_div);
	} else{
		addr1 = base_addr + addr_step * ((ch-32) / ch_div);
	}
	
	//2. Get Inv Fault Code	
	for(i = 0; i < inv_p; i++){
		addr = addr1 + ADDR_PCU_INV_STATE_1 + i; //state inv Addr
		myCh->misc.inv_errCode[i] = SREG16_read(addr);
		//3. Check Fault
		if(myCh->misc.inv_errCode[i] != 0){
			if(myCh->misc.errCnt[C_CNT_PCU_INV_FLT1 + i] < MAX_INV_ERR_CNT){
				for(j = 0; j < inv_div; j++){
					real_ch = ch + j; 
					myData->bData[bd].cData[real_ch]
							.misc.errCnt[C_CNT_PCU_INV_FLT1 + i]++;
				}
			}else if(myCh->misc.inv_errFlag[i] == 0){
				for(j = 0; j < inv_div; j++){	//190311 add
					real_ch = ch + j; 
					if(myData->bData[bd].cData[real_ch].op.code 
														< MAX_NOMAL_CODE ){
						myData->bData[bd].cData[real_ch].misc.tmpCode 
								= myData->bData[bd].cData[real_ch].op.code;
					}
				}
				myCh->misc.inv_errFlag[i] = P1;
				myData->dio.signal[DIO_SIG_PCU_INV_FAIL1 + inv_num] = P1;
				
				switch(myCh->misc.inv_errCode[i]){
					case P_INV_LAG_SHORT:
						myCh->op.code = P_INV_FAULT_LAG_SHORT;
						break;
					case P_INV_OVER_CURRENT:
						myCh->op.code = P_INV_FAULT_OVER_CURRENT;
						break;
					case P_INV_OVER_VOLTAGE:
						myCh->op.code = P_INV_FAULT_OVER_VOLTAGE;
						break;
					case P_INV_PRECHARGE_FAIL:
						myCh->op.code = P_INV_FAULT_PRECHARGE_FAIL;
						break;
					case P_INV_OVER_CURRENT2:
						myCh->op.code = P_INV_FAULT_OVER_CURRENT2;
						break;
					case P_INV_CAN_ERR:
						myCh->op.code = P_INV_FAULT_CAN_ERR;
						break;
					case P_INV_OVER_LOAD:
						myCh->op.code = P_INV_FAULT_OVER_LOAD;
						break;
					case P_INV_OVER_HEAT:
						myCh->op.code = P_INV_FAULT_OVER_HEAT;
						break;
					case P_INV_LOW_VOLTAGE:
						myCh->op.code = P_INV_FAULT_LOW_VOLTAGE;
						break;
					case P_INV_AC_LOW_VOLTAGE:
						myCh->op.code = P_INV_FAULT_AC_LOW_VOLTAGE;
						break;
					case P_INV_RESET1:
						myCh->op.code = P_INV_FAULT_RESET1;
						break;
					case P_INV_RESET2:
						myCh->op.code = P_INV_FAULT_RESET2;
						break;
					case P_INV_AC_INPUT_FAIL:
						myCh->op.code = P_INV_FAULT_AC_INPUT_FAIL;
						break;
					case P_INV_AC_OVER_VOLT:
						myCh->op.code = P_INV_FAULT_AC_OVER_VOLT;
						break;
					case P_INV_HDC_ERROR:
						myCh->op.code = P_INV_FAULT_HDC_ERROR;
						break;
					default:
						myCh->op.code = P_INV_FAULT_ETC;
						break;
				}
				for(j = 0; j < inv_div; j++){
					real_ch = ch + j; 
					myData->bData[bd].cData[real_ch].op.code = myCh->op.code;
					myData->bData[bd].cData[real_ch]
							.misc.errCnt[C_CNT_PCU_INV_FLT1 + i]++;
				}
				myCh->inv_power = P100;
				send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS
									, M_FAIL_INV_GROUP1 + inv_num , i+1);
			}
		}else{
			for(j = 0; j < inv_div; j++){
				real_ch = ch + j;
				myData->dio.signal[DIO_SIG_PCU_INV_FAIL1 + inv_num] = P0;
				myData->bData[bd].cData[real_ch]
							.misc.errCnt[C_CNT_PCU_INV_FLT1 + i] = 0;
			}
		}
	}		
}

void PCU_Inverter_Fault_DC(int bd, int ch)
{	//210304 add for panda inverter
	int i, j, base_addr, addr_step, addr1, addr, inv_p, inv_div;
	int inv_num, real_ch, ch_div, master_ch;
	unsigned char useFlag;
	
	if(myCh->inv_power < P10) return;
	
	ch_div = myPs->pcu_config.portPerCh;
	inv_div = myPs->pcu_config.invPerCh;
	inv_p = myPs->pcu_config.parallel_inv_ch;
	inv_num = ch / inv_div;
	
	//1. Check master ch
	master_ch = ch % inv_div;
	if(master_ch != P0) return;
	
	//2. Inverter use flag check
	useFlag = myData->dio.din.pcu_inUseFlag[inv_num];
	if(useFlag == P0) return;
	
	base_addr = myPs->addr.main[BASE_ADDR];
	addr_step = myPs->addr.main[ADDR_STEP];
	if(ch < 32){
		addr1 = base_addr + addr_step * (ch / ch_div);
	} else{
		addr1 = base_addr + addr_step * ((ch-32) / ch_div);
	}
	
	//3. Get DC Inv Fault Code	
	for(i = 0; i < inv_p; i++){
		addr = addr1 + ADDR_P_INV_STATE_DC_1 + i; 	//state inv Addr
		myCh->misc.inv_errCode[i] = SREG16_read(addr);

		//3. Check Fault
		if(myCh->misc.inv_errCode[i] == 0){		//error clear
			for(j = 0; j < inv_div; j++){
				real_ch = ch + j;
				myData->dio.signal[DIO_SIG_PCU_INV_FAIL1 + inv_num] = P0;
				myData->bData[bd].cData[real_ch]
							.misc.errCnt[C_CNT_PCU_INV_FLT1 + i] = 0;
			}
		} else {
			if(myCh->misc.errCnt[C_CNT_PCU_INV_FLT1 + i] > MAX_INV_ERR_CNT){
				if(myCh->misc.inv_errFlag[i] == 0){
					myCh->misc.inv_errFlag[i] = P1;
					myData->dio.signal[DIO_SIG_PCU_INV_FAIL1 + inv_num] = P1;

					for(j = 0; j < inv_div; j++){	
						real_ch = ch + j; 
						if(myData->bData[bd].cData[real_ch].op.code 
														< MAX_NOMAL_CODE ){
							myData->bData[bd].cData[real_ch].misc.tmpCode 
								= myData->bData[bd].cData[real_ch].op.code;
						}
					}
					//210204 DC Code list
					switch(myCh->misc.inv_errCode[i]){
						case P_INV_DC_OVER_TIME:
							myCh->op.code = P_INV_DC_FAULT_OVER_TIME;
							break;
						case P_INV_DC_BUS_OVER_V:
							myCh->op.code = P_INV_DC_FAULT_BUS_OVER_V;
							break;
						case P_INV_DC_BUS_LOW_V:
							myCh->op.code = P_INV_DC_FAULT_BUS_LOW_V;
							break;
						case P_INV_DC_OVER_VOLTAGE:
							myCh->op.code = P_INV_DC_FAULT_OVER_V;
							break;
						case P_INV_DC_LOW_VOLTAGE:
							myCh->op.code = P_INV_DC_FAULT_LOW_V;
							break;
						case P_INV_DC_OVER_CURRENT:
							myCh->op.code = P_INV_DC_FAULT_OVER_CURRENT;
							break;
						case P_INV_DC_OVER_LOAD:
							myCh->op.code = P_INV_DC_FAULT_OVER_LOAD;
							break;
						case P_INV_DC_FAULT_LOCK_ERR:
							myCh->op.code = P_INV_DC_FAULT_LOCK;
							break;
						case P_INV_DC_SOFT_SHORT:
							myCh->op.code = P_INV_DC_FAULT_SOFT_SHORT;
							break;
						case P_INV_DC_DIP_SWITCH:
							myCh->op.code = P_INV_DC_FAULT_DIP_SWITCH;
							break;
						case P_INV_DC_PFC_ERR:
							myCh->op.code = P_INV_DC_FAULT_PFC_ERR;
							break;
						default:
							myCh->op.code = P_INV_FAULT_ETC;
							break;
					}

					for(j = 0; j < inv_div; j++){
						real_ch = ch + j; 
						myData->bData[bd].cData[real_ch].op.code 
								= myCh->op.code;
						myData->bData[bd].cData[real_ch]
								.misc.errCnt[C_CNT_PCU_INV_FLT1 + i]++;
					}
					myCh->inv_power = P100;
					send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS
									, M_FAIL_INV_GROUP1 + inv_num , i+1);
				}
			} else {
				for(j = 0; j < inv_div; j++){
					real_ch = ch + j; 
					myData->bData[bd].cData[real_ch]
							.misc.errCnt[C_CNT_PCU_INV_FLT1 + i]++;
				}
			}
		}
	}		
}

void PCU_Inverter_Fault_AC(int bd, int ch)
{	//210304 add for panda inverter
	int i, j, base_addr, addr_step, addr1, addr, inv_p, inv_div;
	int inv_num, real_ch, ch_div, master_ch;
	unsigned char useFlag;
	
	if(myCh->inv_power < P10) return;
	
	ch_div = myPs->pcu_config.portPerCh;
	inv_div = myPs->pcu_config.invPerCh;
	inv_p = myPs->pcu_config.parallel_inv_ch;
	inv_num = ch / inv_div;
	//210304 add for AC errorCode Start Number
	
	//1. Check master ch
	master_ch = ch % inv_div;
	if(master_ch != P0) return;
	
	//2. Inverter use flag check
	useFlag = myData->dio.din.pcu_inUseFlag[inv_num];
	if(useFlag == P0) return;
	
	base_addr = myPs->addr.main[BASE_ADDR];
	addr_step = myPs->addr.main[ADDR_STEP];
	if(ch < 32){
		addr1 = base_addr + addr_step * (ch / ch_div);
	} else{
		addr1 = base_addr + addr_step * ((ch-32) / ch_div);
	}
	
	//3. Get AC Inv Fault Code	
	for(i = 0; i < inv_p; i++){
		addr = addr1 + ADDR_P_INV_STATE_AC_1 + i; 	//state inv Addr
		myCh->misc.inv_errCode_AC[i] = SREG16_read(addr);

		//3. Check Fault
		if(myCh->misc.inv_errCode_AC[i] == 0){
			for(j = 0; j < inv_div; j++){
				real_ch = ch + j;
				myData->dio.signal[DIO_SIG_PCU_INV_FAIL1 + inv_num] = P0;
				myData->bData[bd].cData[real_ch]
							.misc.errCnt[C_CNT_INV_AC_1 + i] = 0;
			}
		} else {
			if(myCh->misc.errCnt[C_CNT_INV_AC_1 + i] > MAX_INV_ERR_CNT){
				if(myCh->misc.inv_errFlag[i] == 0){
					myCh->misc.inv_errFlag[i] = P1;
					myData->dio.signal[DIO_SIG_PCU_INV_FAIL1 + inv_num] = P1;

					for(j = 0; j < inv_div; j++){	//190311 add
						real_ch = ch + j; 
						if(myData->bData[bd].cData[real_ch].op.code 
														< MAX_NOMAL_CODE ){
							myData->bData[bd].cData[real_ch].misc.tmpCode 
								= myData->bData[bd].cData[real_ch].op.code;
						}
					}
				
					switch(myCh->misc.inv_errCode_AC[i]){
						case P_INV_AC_INPUT_VOLTAGE:
							myCh->op.code = P_INV_AC_FAULT_INPUT_V;
							break;
						case P_INV_AC_INPUT_FRQ:
							myCh->op.code = P_INV_AC_FAULT_INPUT_FRQ;
							break;
						case P_INV_AC_INPUT_CURRENT:
							myCh->op.code = P_INV_AC_FAULT_INPUT_CURRENT;
							break;
						case P_INV_AC_PFC_BUS_V:
							myCh->op.code = P_INV_AC_FAULT_PFC_BUS_V;
							break;
						case P_INV_AC_OVER_TIME:
							myCh->op.code = P_INV_AC_FAULT_OVER_TIME;
							break;
						case P_INV_AC_OVER_LOAD:
							myCh->op.code = P_INV_AC_FAULT_OVER_LOAD;
							break;
						case P_INV_AC_OVER_HEAT:
							myCh->op.code = P_INV_AC_FAULT_OVER_HEAT;
							break;
						case P_INV_AC_FAULT_LOCK_ERR:
							myCh->op.code = P_INV_AC_FAULT_LOCK;
							break;
						default:
							myCh->op.code = P_INV_FAULT_ETC;
							break;
					}

					for(j = 0; j < inv_div; j++){
						real_ch = ch + j; 
						myData->bData[bd].cData[real_ch].op.code 
								= myCh->op.code;
						myData->bData[bd].cData[real_ch]
								.misc.errCnt[C_CNT_INV_AC_1 + i]++;
					}
					
					myCh->inv_power = P100;
					send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS
										, M_FAIL_INV_GROUP1 + inv_num , i+1);
				}
			} else {
				for(j = 0; j < inv_div; j++){
					//error Count
					real_ch = ch + j; 
					myData->bData[bd].cData[real_ch]
							.misc.errCnt[C_CNT_INV_AC_1 + i]++;
				}
			}
		}	
	}		
}

void pFaultCheck(int bd, int ch)
{
	unsigned char send_seq_no, receive_seq_no, val1, pcu_mode =0;
	int base_addr, addr_step, addr1, addr, ch_div;
	int inv_num, inv_num1, inv_div;
//	int j, k, real_ch, master_ch, MainInvCh = 0; //191012 lyhw
	long saveDt;
	unsigned long advStepNo;

	S_CH_STEP_INFO step;

	step = step_info(bd, ch);
	advStepNo = step.advStepNo;
	
	myCh = &(myData->bData[bd].cData[ch]);
	base_addr = myPs->addr.main[BASE_ADDR];
	addr_step = myPs->addr.main[ADDR_STEP];
	ch_div = myPs->pcu_config.portPerCh;
	inv_div = myPs->pcu_config.invPerCh;
	if(ch < 32){
		addr1 = base_addr + addr_step * (ch / ch_div);
	} else{
		addr1 = base_addr + addr_step * ((ch-32) / ch_div);
	}
	
	if(myData->AppControl.config.debugType != P0){ 
		myCh->misc.receive_pcu_seq_no = myCh->misc.send_pcu_seq_no;
		return;
	}
	//1. Check SEQ No -- 180817 add
	addr = addr1 + CREG_CTRL_CCU1_SEQ_NO + (ch % ch_div);
	myCh->misc.receive_pcu_seq_no = (unsigned char)SREG8_read2(addr);
	
	send_seq_no = myCh->misc.send_pcu_seq_no;
	receive_seq_no = myCh->misc.receive_pcu_seq_no;
	
	if(myCh->ChAttribute.opType == P1) return;
	saveDt = myPs->testCond[bd][ch].step[advStepNo].saveDt;
	
	//2. Check PCU Mode -- 180904 add
	addr = addr1 + CREG_CTRL_CCU1_STATE + (ch % ch_div);
	pcu_mode = (SREG8_read1(addr) & 0xf0) >> 4;
	
	switch(pcu_mode){
		case PCU_CV:
			if(myCh->op.type == STEP_CHARGE
				|| myCh->op.type == STEP_DISCHARGE){
				myCh->misc.cvFlag = P1;
				#ifdef _EXTERNAL_CONTORL
				mych->misc.chCV = P1;
				#endif
//				myCh->misc.cvFaultCheckFlag = P1; //210204
			}
			break;
		default:
			break;
	}
	
	//1-1. seq no error Fault check  -- 180817 add
	if(send_seq_no != receive_seq_no){	//CNT 3
		if(myCh->op.checkDelayTime > 20 && myCh->op.state == C_RUN){
			myCh->misc.errCnt[C_CNT_PCU_SEQ_FAULT]++;
			if(myCh->ChAttribute.chNo_master == P0){
				myData->bData[bd].cData[ch-1]
						.misc.errCnt[C_CNT_PCU_SEQ_FAULT]++;
			}
		}
	}else{
		myCh->misc.errCnt[C_CNT_PCU_SEQ_FAULT] = 0;
		if(myCh->ChAttribute.chNo_master == P0){
			myData->bData[bd].cData[ch-1].misc.errCnt[C_CNT_PCU_SEQ_FAULT]=0;
		}
	}
	
	//3. Check PCU Error  - Get Ch fault
	addr = addr1 + CREG_CTRL_CCU1_STATE + (ch % ch_div);
	val1 = (SREG8_read2(addr) >> 4) & 0x0F;
	
	/*
	if(val1 == P1){			//Check Inverter Input Error
		master_ch = ch / inv_div;
		MainInvCh = master_ch * inv_div;
		if(myPs->signal[M_SIG_INV_POWER1] != P10) return;
		if(myCh->op.code == P_CD_FAULT_HVU_INPUT_POWER) return;
		if(myData->bData[bd].cData[MainInvCh].inv_power != P10
			&& myData->bData[bd].cData[MainInvCh].inv_power != P99) return;
		
		for(j = 0; j < inv_div; j++){
			real_ch = MainInvCh + j;
			myData->bData[bd].cData[real_ch]
						.misc.errCnt[C_CNT_PCU_INPUT_FAULT]++;
		}
		
		if(myCh->misc.errCnt[C_CNT_PCU_INPUT_FAULT] < MAX_ERROR_CNT_P) return;
		if(myCh->misc.errCnt[C_CNT_PCU_INPUT_FAULT] == MAX_ERROR_CNT_P){
			for(j = 0; j < inv_div; j++){
				real_ch = MainInvCh + j;
				if(myData->bData[bd].cData[real_ch].op.code < MAX_NOMAL_CODE){
					myData->bData[bd].cData[real_ch].misc.tmpCode 
							= myData->bData[bd].cData[real_ch].op.code;
				}
				myData->bData[bd].cData[real_ch].op.code 
										= P_CD_FAULT_HVU_INPUT_POWER;
			}
			//P90 = INPUT ERROR INVETER STOP
			myData->bData[bd].cData[MainInvCh].inv_power = P90;
		}
	}*/ 
	if(val1 != 0){
		if(myCh->op.state == C_PAUSE) return;
		if(myCh->op.checkDelayTime < 100 ) return;
		if(myCh->op.phase != P50) return;
		myCh->misc.errCnt[C_CNT_PCU_FAULT]++;
		if(myCh->ChAttribute.opType == P1){
			myData->bData[bd].cData[ch+1].misc.errCnt[C_CNT_PCU_FAULT]++;
		}
		if(myCh->misc.errCnt[C_CNT_PCU_FAULT] < MAX_ERROR_CNT_P) return;
		//190311 lyh add
		if(myCh->misc.errCnt[C_CNT_PCU_FAULT] == MAX_ERROR_CNT_P){
			if(myCh->op.code < MAX_NOMAL_CODE){
				myCh->misc.tmpCode = myCh->op.code;
			}
		}
		switch(val1){
			case 1:
				myCh->op.code = P_CD_FAULT_HVU_INPUT_POWER;
				if(myCh->ChAttribute.opType == P1){
					myData->bData[bd].cData[ch+1].op.code 
							= P_CD_FAULT_HVU_INPUT_POWER;
				}
				break;
			case 2:
				myCh->op.code = P_CD_FAULT_OVP;
				if(myCh->ChAttribute.opType == P1){
					myData->bData[bd].cData[ch+1].op.code = P_CD_FAULT_OVP;
				}
				break;
			case 3:
				myCh->op.code = P_CD_FAULT_OCP;
				if(myCh->ChAttribute.opType == P1){
					myData->bData[bd].cData[ch+1].op.code = P_CD_FAULT_OCP;
				}
				break;
			case 4:
				myCh->op.code = P_CD_FAULT_OTP;
				if(myCh->ChAttribute.opType == P1){
					myData->bData[bd].cData[ch+1].op.code = P_CD_FAULT_OTP;
				}
				break;
			case 5:
				myCh->op.code = P_CD_FAULT_PV_OVP;
				if(myCh->ChAttribute.opType == P1){
					myData->bData[bd].cData[ch+1].op.code 
											= P_CD_FAULT_PV_OVP;
				}
				break;
			case 6:
				myCh->op.code = P_CD_FAULT_HW_OVP;
				if(myCh->ChAttribute.opType == P1){
					myData->bData[bd].cData[ch+1].op.code 
											= P_CD_FAULT_HW_OVP;
				}
				break;
			case 7:
				myCh->op.code = P_CD_FAULT_HW_OCP;
				if(myCh->ChAttribute.opType == P1){
					myData->bData[bd].cData[ch+1].op.code 
											= P_CD_FAULT_HW_OCP;
				}
				break;
			case 8:
				myCh->op.code = P_CD_FAULT_BD_OT;
				if(myCh->ChAttribute.opType == P1){
					myData->bData[bd].cData[ch+1].op.code 
											= P_CD_FAULT_BD_OT;
				}
				break;
			case 9:
				myCh->op.code = P_CD_FAULT_B_POWER;
				if(myCh->ChAttribute.opType == P1){
					myData->bData[bd].cData[ch+1].op.code 
											= P_CD_FAULT_B_POWER;
				}
				break;
			case 10:
				myCh->op.code = P_CD_FAULT_BALANCE;
				if(myCh->ChAttribute.opType == P1){
					myData->bData[bd].cData[ch+1].op.code 
											= P_CD_FAULT_BALANCE;
				}
				break;
			case 11:
				myCh->op.code = P_CD_FAULT_PARALLEL_RELAY;
				if(myCh->ChAttribute.opType == P1){
					myData->bData[bd].cData[ch+1].op.code 
											= P_CD_FAULT_PARALLEL_RELAY;
				}
				break;
			case 12:
				myCh->op.code = P_CD_FAULT_EXT_RELAY;
				if(myCh->ChAttribute.opType == P1){
					myData->bData[bd].cData[ch+1].op.code
											= P_CD_FAULT_EXT_RELAY;
				}
				break;
			case 14:
				inv_num1 = ch / inv_div;
				inv_num = inv_num1 * inv_div;
				if(myData->bData[bd].cData[inv_num].inv_power == P10){
					myData->bData[bd].cData[inv_num].inv_power = P100;
				}
				myCh->op.code = P_CD_FAULT_SCI_ERR;
				if(myCh->ChAttribute.opType == P1){
					myData->bData[bd].cData[ch+1].op.code 
											= P_CD_FAULT_SCI_ERR;
				}
				break;
			case 13:
			case 15:
				myCh->op.code = P_CD_FAULT_UNKOWN_ERR;
				if(myCh->ChAttribute.opType == P1){
					myData->bData[bd].cData[ch+1].op.code
											= P_CD_FAULT_UNKOWN_ERR;
				}
				break;
			default:
				break;
		}
	}else{
		myCh->misc.errCnt[C_CNT_PCU_FAULT] = 0;
		if(myCh->ChAttribute.opType == P1){
			myData->bData[bd].cData[ch+1].misc.errCnt[C_CNT_PCU_FAULT] = 0;
		}
			
		/*
		master_ch = ch / inv_div;
		MainInvCh = master_ch * inv_div;			
		for(k = 0; k < inv_div; k++){
			real_ch = MainInvCh + k;
			myData->bData[bd].cData[real_ch]
						.misc.errCnt[C_CNT_PCU_INPUT_FAULT] = 0;
		}*/
	}
}

void CH_Inverter_Signal(int bd ,int ch)
{	//180830 add 
	int i, diff, invPerCh , invCh = 0;
	
	myCh = &(myData->bData[bd].cData[ch]);
	invPerCh = myPs->pcu_config.invPerCh;
	invCh = ch % invPerCh;
	if(invCh != 0) return;
	
	switch(myCh->inv_power){
		case P1:
			myCh->misc.invTimer = myPs->misc.timer_1sec;
			//180905 add 
			//200824 rewrite lyhw
			for(i = 0; i < invPerCh ; i++){
				memset((char *)&myData->bData[bd].cData[ch+i].misc.inv_errCode, 0x00, MAX_INV_NUM);
				memset((char *)&myData->bData[bd].cData[ch+i].misc.inv_errCode_AC, 0x00, MAX_INV_NUM);
				memset((char *)&myData->bData[bd].cData[ch+i].misc.inv_errFlag, 0x00, MAX_INV_NUM);
				memset((char *)&myData->bData[bd].cData[ch+i].misc.errCnt, 0x00, MAX_SIGNAL);
				myData->bData[bd].cData[ch+i].op.code = P_INV_STANDBY;
			}
			myCh->inv_power++;
			break;
		case P2:
			diff = myPs->misc.timer_1sec - myCh->misc.invTimer;
			if(diff < 5) break;		//7Sec need for PCU Communication
			//180905 add 
			for(i = 0; i < invPerCh ; i++){
				myData->bData[bd].cData[ch+i].op.code = P_INV_STANDBY;
			}
			myCh->misc.invTimer = myPs->misc.timer_1sec;
			myCh->inv_power++;
			break;
		case P3:
			myCh->signal[C_SIG_DIGITAL_INV_RESET] = P1;
			myCh->misc.invTimer = myPs->misc.timer_1sec;
			myCh->inv_power++;
			break;
		case P4:
			diff = myPs->misc.timer_1sec - myCh->misc.invTimer;
			if(diff < 8) break;
			myCh->signal[C_SIG_DIGITAL_INV_RUN] = P1;
			myCh->misc.invTimer = myPs->misc.timer_1sec;
			myCh->inv_power++;
			break;
		case P5:
			diff = myPs->misc.timer_1sec - myCh->misc.invTimer;
	//		if(diff < 15) break;
			if(diff < 20) break;		//180903
			myCh->misc.invTimer = myPs->misc.timer_1sec;
			myCh->inv_power = P6;
			break;
		case P6:
			//180905 add
			//190311 rewrite lyh
			for(i = 0; i < invPerCh; i++){
				myData->bData[bd].cData[ch+i].op.code 
						= myData->bData[bd].cData[ch+i].misc.tmpCode;
			}
			myCh->inv_power = P7;
			break;
		case P7:
			//Standby By Ch Reset Singal 
			myCh->inv_power = P8;
			break;
		case P8:
			diff = myPs->misc.timer_1sec - myCh->misc.invTimer;
			if(diff < 2) break;		//180903
			myCh->misc.invTimer = myPs->misc.timer_1sec;
			myCh->inv_power = P10;
			break;
		case P10:
			// Inverter Running
			break;
		case P90:	// PCU Input Error Stop
			myCh->misc.invTimer = myPs->misc.timer_1sec;
			myCh->inv_power = P92;
			break;
		case P92:
			diff = myPs->misc.timer_1sec - myCh->misc.invTimer;
			if(diff < 2) break;		//180903
			myCh->misc.invTimer = myPs->misc.timer_1sec;
			myCh->inv_power = P98;
			break;
		case P98:
			myData->bData[bd].cData[ch].signal[C_SIG_DIGITAL_INV_STOP] = P1;
			myCh->inv_power = P99;
			break;
		case P99:	
			//PCU_INPUT ERROR Hold
			break;
		case P100:
			myCh->misc.invTimer = myPs->misc.timer_1sec;
			myCh->inv_power = P101;
			break;
		case P101:	//191108 lyhw add for INV off 
			diff = myPs->misc.timer_1sec - myCh->misc.invTimer;
			if(diff < 1) break;
			myData->bData[bd].cData[ch].signal[C_SIG_DIGITAL_INV_STOP] = P1;
			myCh->inv_power = P0;
			break;
		default:
			break;
	}
}
int pcu_temp_wait_flag_check(int bd, int ch)
{	//190624 add lyhw
	int flag;
	unsigned char noTempWaitFlag = 0;
	long refTemp, startTemp, groupTemp, opTemp;
	unsigned long advStepNo;
	int bd_1, ch_1, i;
	int cnt = 0;

	myCh = &(myData->bData[bd].cData[ch]);
	advStepNo = myCh->misc.advStepNo;
	refTemp = myPs->testCond[bd][ch].step[advStepNo].refTemp;
	startTemp = myPs->testCond[bd][ch].step[advStepNo].startTemp;
	groupTemp = myCh->misc.groupTemp;
	opTemp = myCh->op.temp;
	noTempWaitFlag = myPs->testCond[bd][ch].step[advStepNo].noTempWaitFlag;
	flag = 0;	

	if(myCh->signal[C_SIG_STOP] == P1) {
		myCh->op.code = C_FAULT_STOP_CMD;
		myCh->signal[C_SIG_STOP] = P0;
		myCh->misc.chamberWaitFlag = -1;
		myCh->misc.chGroupNo = 0;
		myCh->op.phase = P100;
	} else if(myCh->signal[C_SIG_PAUSE] == P1) {
		myCh->misc.tmpState = myCh->op.state;
		myCh->op.code = C_FAULT_PAUSE_CMD;
		myCh->opSave = myCh->op;
		myCh->signal[C_SIG_PAUSE] = P0;
		cSemiSwitch_Rest(bd,ch,advStepNo,0);
		myCh->misc.chamberWaitFlag = -1;
		myCh->op.phase = P100;
	} else if(myCh->signal[C_SIG_DLL_STOP] == P1) {
		myCh->op.code = C_FAULT_CELL_DIAGNOSIS_STOP;
		myCh->signal[C_SIG_DLL_STOP] = P0;
		myCh->misc.chamberWaitFlag = -1;
		myCh->misc.chGroupNo = 0;
		myCh->op.phase = P100;
	}
	//140409 oys add : ChamberTempWaitFlag
	if(myData->mData.config.function[F_CHAMBER_TEMP_WAIT] == P0){
		return 1;
	}

	if(cFailCodeCheck(bd, ch) < 0) {
		myCh->misc.chamberWaitFlag = -1;
	}
	
	if(myCh->op.phase == P100) {
		cNextStepCheck(bd, ch);
		return flag;
	}
	
	//20180417 modify for step sync process
	if(myCh->misc.stepSyncFlag == P1 && myCh->misc.chamberWaitFlag >= P20){
		for(i = 0; i < myPs->config.installedCh; i++){
			bd_1 = i/myPs->config.chPerBd;
			ch_1 = i%myPs->config.chPerBd;
			if(myCh->misc.chGroupNo ==
				myData->bData[bd_1].cData[ch_1].misc.chGroupNo){
				if((myCh->misc.advStepNo != 
					myData->bData[bd_1].cData[ch_1].misc.advStepNo) ||
					(myCh->misc.totalCycle != 
					myData->bData[bd_1].cData[ch_1].misc.totalCycle)){
					if(myCh->misc.totalCycle > 
						myData->bData[bd_1].cData[ch_1].misc.totalCycle){
							cnt++;
					}else if(myCh->misc.totalCycle ==
						myData->bData[bd_1].cData[ch_1].misc.totalCycle){
						if(myCh->misc.advStepNo > 
							myData->bData[bd_1].cData[ch_1].misc.advStepNo){
								cnt++;
						}
					}
				}
			}
		}
	}
	if(cnt != 0){
		myCh->misc.chamberWaitFlag -= P10;
		cnt = 0;
	}
	//170105 oys add : ch step sync process
	if(myCh->misc.stepSyncFlag == P1){
		if(myCh->misc.chamberWaitFlag >= P20){ //chamber temp wait end run delay
			if(myCh->ChAttribute.opType == P1){
				myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
				myData->bData[bd].cData[ch+1].op.checkDelayTime = myCh->op.checkDelayTime;
			}else{
				if(myCh->ChAttribute.chNo_master != P0){
					myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
				}
			}
			if(myCh->op.type != STEP_REST) {
				myCh->signal[C_SIG_OUT_SWITCH] = P1; //run relay close
				if(myCh->signal[C_SIG_OUT_SWITCH_ON] != P3){
					myCh->signal[C_SIG_OUT_SWITCH_ON] = P1;
				}else{
					if(myCh->misc.send_pcu_seq_no != myCh->misc.receive_pcu_seq_no){
						myCh->misc.seq_no_cnt++;
						if(myCh->misc.seq_no_cnt < MAX_SEQ_NO_RETRY){
							myCh->signal[C_SIG_OUT_SWITCH_ON] = P1;
						}
					}else{
						myCh->misc.seq_no_cnt = 0;
					}
				}
			}
			if(myCh->op.checkDelayTime >= 200){ //relay time wait
				myCh->op.checkDelayTime = 0;
				myCh->op.code = myCh->misc.tmpCode;
				myCh->misc.chamberWaitFlag = P0;
				flag = 1;
				return flag;
			}
			return flag;
		}
	}else{
		if(myCh->misc.chamberWaitFlag >= P10){ //chamber temp wait end run delay
			if(myCh->ChAttribute.opType == P1){
				myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
				myData->bData[bd].cData[ch+1].op.checkDelayTime
							                     = myCh->op.checkDelayTime;
			}else{
				if(myCh->ChAttribute.chNo_master != P0){
					myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
				}
			}
			/*
			if(myCh->ChAttribute.chNo_master == P0){	//181227 lyh add
				myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
				myData->bData[bd].cData[ch-1].op.checkDelayTime 
											= myCh->op.checkDelayTime;
			}else{
				if(myCh->ChAttribute.opType == P0){
					myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
				}
			}
			*/
		
			if(myCh->op.type != STEP_REST) {
				myCh->signal[C_SIG_OUT_SWITCH] = P1; //run relay close
				if(myCh->signal[C_SIG_OUT_SWITCH_ON] != P3){
					myCh->signal[C_SIG_OUT_SWITCH_ON] = P1; 	
				}else{	//181202 add lyhw
					if(myCh->misc.send_pcu_seq_no 
									!= myCh->misc.receive_pcu_seq_no){
						myCh->misc.seq_no_cnt++;
						if(myCh->misc.seq_no_cnt < MAX_SEQ_NO_RETRY){
							myCh->signal[C_SIG_OUT_SWITCH_ON] = P1;			
						}
					}else{
						myCh->misc.seq_no_cnt = 0;
					}
				}
			}

			if(myCh->op.checkDelayTime >= 200){ //relay time wait
				myCh->op.checkDelayTime = 0;
				myCh->op.code = myCh->misc.tmpCode;
				myCh->misc.chamberWaitFlag = P0;
				flag = 1;
				return flag;
			}
			return flag;
		}
	}
	
	if(refTemp != 999000 || myCh->misc.stepSyncFlag == P1){
		if(myCh->misc.tempDir == P0 && myCh->misc.stepSyncFlag == P0){
			if(myCh->ChAttribute.opType == P1){
				myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
				myData->bData[bd].cData[ch+1].op.checkDelayTime
						                     = myCh->op.checkDelayTime;
			}else{
				if(myCh->ChAttribute.chNo_master != P0){
					myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
				}
			}
			
		/*
			if(myCh->ChAttribute.chNo_master == P0){	//181227 lyhw
				myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
				myData->bData[bd].cData[ch-1].op.checkDelayTime 
											= myCh->op.checkDelayTime;
			}else{
				if(myCh->ChAttribute.opType == P0){
					myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
				}
			}
		*/	
			if(myCh->op.type != STEP_REST) {
				if(myCh->signal[C_SIG_OUT_SWITCH_ON] != P3){
					myCh->signal[C_SIG_OUT_SWITCH_ON] = P1; 	
				}else{	//181202 add lyhw
					if(myCh->misc.send_pcu_seq_no 
									!= myCh->misc.receive_pcu_seq_no){
						myCh->misc.seq_no_cnt++;
						if(myCh->misc.seq_no_cnt < MAX_SEQ_NO_RETRY){
							myCh->signal[C_SIG_OUT_SWITCH_ON] = P1;			
						}
					}else{
						myCh->misc.seq_no_cnt = 0;
					}
				}
			}
			
			if(myCh->op.checkDelayTime >= 200){ //relay time wait
				myCh->op.checkDelayTime = 0;
				myCh->misc.chamberWaitFlag = P0;
				flag = 1;
				return flag;
			}
			return flag;
		}else{
			if(myCh->misc.chamberWaitFlag == P0) {
				myCh->misc.chamberWaitFlag = P1;
				myCh->misc.tmpCode = myCh->op.code;
			}
			myCh->signal[C_SIG_NEXTSTEP] = P0; //nextstep message disable
			myCh->signal[C_SIG_STOP] = P0; //stop message disable
			myCh->signal[C_SIG_PAUSE] = P0; //pause message disable
			myCh->signal[C_SIG_DLL_STOP] = P0;
			myCh->signal[C_SIG_DLL_PAUSE] = P0;
			myCh->op.code = C_TEMP_WAIT_TIME;
			myCh->signal[C_SIG_OUT_SWITCH] = P0; //run relay open
			
			if(myCh->signal[C_SIG_OUT_SWITCH_OFF] != P3){
				myCh->signal[C_SIG_OUT_SWITCH_OFF] = P1; 	
			}else{	//181202 add lyhw
				if(myCh->misc.send_pcu_seq_no 
								!= myCh->misc.receive_pcu_seq_no){
					myCh->misc.seq_no_cnt++;
					if(myCh->misc.seq_no_cnt < MAX_SEQ_NO_RETRY){
						myCh->signal[C_SIG_OUT_SWITCH_OFF] = P1;			
					}
				}else{
					myCh->misc.seq_no_cnt = 0;
				}
			}
			
			if(myCh->ChAttribute.opType == P0){
				if(myCh->misc.tempDir == P0){
					myCh->op.checkDelayTime = 0;
					myCh->misc.chamberWaitFlag = P10; //temp dir P0
				}else if(myCh->misc.tempDir == P1){
					if(noTempWaitFlag == P1){
						myCh->op.checkDelayTime = 0;
						myCh->misc.chamberWaitFlag = P11; //temp dir P1
					}else{
						if(groupTemp >= refTemp){
							myCh->op.checkDelayTime = 0;
							myCh->misc.chamberWaitFlag = P11; //temp dir P1
						}
					}
				}else if(myCh->misc.tempDir == P2){
					if(noTempWaitFlag == P1){
						myCh->op.checkDelayTime = 0;
						myCh->misc.chamberWaitFlag = P12; //temp dir P2 
					}else{
						if(groupTemp <= refTemp) {
							myCh->op.checkDelayTime = 0;
							myCh->misc.chamberWaitFlag = P12; //temp dir P2 
						}
					}
				}
				//190624 add lyhw
				if(myCh->ChAttribute.chNo_master == P0){
					myData->bData[bd].cData[ch-1].op.checkDelayTime
												= myCh->op.checkDelayTime;
					myData->bData[bd].cData[ch-1].misc.chamberWaitFlag
							                    = myCh->misc.chamberWaitFlag;
				}
			}
		}
		#ifdef _SDI	//Step Sync Code Change
		if(myCh->misc.chamberWaitFlag >= P10){
			if(myCh->misc.stepSyncFlag == P1){
				myCh->op.code = C_STEP_SYNC_WAIT_TIME;
			}
		}
		#endif
	}else{
		flag = 1;
		//190808 lyhw
		myCh->op.checkDelayTime = 0;
	}
	return flag;
}

void pFaultCondHard(int bd,int ch)
{
	unsigned char type, mode;
	int rangeV, rangeI, rtn = 0, inv_p, i;
	float tmp;
	long val1, val2, maxV, maxI, minI, delta, CV_Check;
	unsigned long advStepNo, saveDt;
	S_CH_STEP_INFO step;

	i = 0;

	if(myData->mData.config.function[F_HW_FAULT_COND] == P0) return;
	
	myCh = &(myData->bData[bd].cData[ch]);
	myTestCond = &(myPs->testCond[bd][ch]);

	if(myCh->op.state != C_RUN) return;
	if(myCh->op.type != STEP_USER_PATTERN) return;
	if(myCh->op.phase != P50) return;

	step = step_info(bd, ch);
	
	advStepNo = step.advStepNo;
	rangeV = step.rangeV;
	rangeI = step.rangeI;
	saveDt = step.saveDt;
	val1 = step.refV;
	val2 = step.refI;
	type = step.type;
	mode = step.mode;
	inv_p = myPs->pcu_config.parallel_inv_ch;

	maxV = myPs->config.maxVoltage[rangeV];
	maxI = myPs->config.maxCurrent[rangeI];
	minI = myPs->config.minCurrent[rangeI];

	switch(type){
		case STEP_USER_PATTERN:
			if(myCh->op.checkDelayTime < 180) break;
			if(val2 > 0){
				CV_Check = val1 - myCh->op.Vsens;
			}else if(val2 < 0){
				CV_Check = myCh->op.Vsens - val1;
			}else{
				CV_Check = 0;
			}
			
			if(mode == CC) {
				if(CV_Check > maxV * 0.001){
					tmp = (float)(maxI*0.04);
					if((labs(myCh->op.Isens) - labs(val2)) >= (long)tmp) {
						myCh->misc.errCnt[C_CNT_CH_I_FAIL]++;
						if(myCh->misc.errCnt[C_CNT_CH_I_FAIL]>= MAX_ERROR_CNT){
							myCh->misc.errCnt[C_CNT_CH_I_FAIL] = 0;
							myCh->misc.tmpState = myCh->op.state;
							myCh->misc.tmpCode = myCh->op.code;
							myCh->op.code = C_FAULT_CH_I_FAIL;
							rtn = FAULT_COND;
							break;
						}
					}else{
						myCh->misc.errCnt[C_CNT_CH_I_FAIL] = 0;
					}
				}else{
					myCh->misc.errCnt[C_CNT_CH_I_FAIL] = 0;
				}
				
				if(val2 > 0){
					delta = myCh->op.Vsens - val1;
				}else if(val2 < 0){
					delta = val1 - myCh->op.Vsens;
				}else {
					delta = 0;
				}
				
				if(delta >= (long)(maxV * 0.02)) {
					myCh->misc.errCnt[C_CNT_CH_V_FAIL]++;
					if(myCh->misc.errCnt[C_CNT_CH_V_FAIL]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_CH_V_FAIL] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_CH_V_FAIL;
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_CH_V_FAIL] = 0;
				}
			}
			
			//180726 add for digital
			if(myCh->misc.errCnt[C_CNT_PCU_SEQ_FAULT] >= MAX_ERROR_CNT_SEQ){
				myCh->misc.errCnt[C_CNT_PCU_SEQ_FAULT] = 0;
				myCh->misc.tmpState = myCh->op.state;
				myCh->misc.tmpCode = myCh->op.code;
				myCh->op.code = P_CD_FAULT_SEQ_NO;
				rtn = FAULT_COND;
				break;
			}	
			//180611 add for digital
			if(myCh->misc.errCnt[C_CNT_PCU_FAULT] >= MAX_ERROR_CNT_P){
				myCh->misc.errCnt[C_CNT_PCU_FAULT] = 0;
				myCh->misc.tmpState = myCh->op.state;
				rtn = FAULT_COND;
				break;
			}
			//191024 add for digital
			/*
			if(myCh->misc.errCnt[C_CNT_PCU_INPUT_FAULT]>= MAX_ERROR_CNT_P){
				myCh->misc.errCnt[C_CNT_PCU_INPUT_FAULT] = 0;
				myCh->misc.tmpState = myCh->op.state;
				rtn = FAULT_COND;
				break;
			}*/
			//180709 add for digital inverter fault
			/*
			for(i = 0; i < inv_p; i++){
				if(myCh->misc.errCnt[C_CNT_PCU_INV_FLT1+i] 
													>= MAX_ERROR_CNT_P){
					myCh->misc.errCnt[C_CNT_PCU_INV_FLT1 + i] = 0;
					myCh->misc.tmpState = myCh->op.state;
					rtn = FAULT_COND;
					break;
				}
			}*/
			
			if(myCh->op.temp > myCh->misc.maxT) {
				myCh->misc.maxT = myCh->op.temp;
			}
			if(myCh->op.temp < myCh->misc.minT) {
				myCh->misc.minT = myCh->op.temp;
			}
			if(myCh->op.Vsens > myCh->misc.maxV) {
				myCh->misc.maxV = myCh->op.Vsens;
			}
			if(myCh->op.Vsens < myCh->misc.minV) {
				myCh->misc.minV = myCh->op.Vsens;
			}
			if(myCh->op.Isens > myCh->misc.maxI) {
				myCh->misc.maxI = myCh->op.Isens;
			}
			if(myCh->op.Isens < myCh->misc.minI) {
				myCh->misc.minI = myCh->op.Isens;
			}
			break;
		default:
			break;
	}

	if(rtn == FAULT_COND) { //fault
		myCh->op.select = SAVE_FLAG_SAVING_ETC;
		send_save_msg(bd, ch, saveDt, 0);
		//170106 oys add
		if(myCh->misc.tempWaitType == P0){
			myCh->misc.chGroupNo = 0;
			//20180417 add
			myCh->misc.stepSyncFlag = P0;
		}
		//190318 lyhw
		if(myData->mData.config.parallelMode == P2) { 
			cCycle_p_ch_check(bd, ch);
		}
		myCh->op.phase = P100;
		cNextStepCheck(bd, ch);
	}
}

void pFaultCondHard_P(int bd,int ch)
{
	unsigned char type, mode;
	int rangeV, rangeI, rtn = 0, inv_p, i;
	long val1, val2, maxV, maxI, minI, delta;
	long m_Delta, m_opIsens, m_tmpIsens, m_Vsens, m_Temp;
	unsigned long advStepNo, saveDt;
	S_CH_STEP_INFO step;

	i = 0;
	
	if(myData->mData.config.function[F_HW_FAULT_COND] == P0) return;
	
	myCh = &(myData->bData[bd].cData[ch]);
	myTestCond = &(myPs->testCond[bd][ch]);

	if(myCh->op.state != C_RUN) return;
	if(myCh->op.type != STEP_USER_PATTERN) return;
	if(myCh->op.phase != P50) return;

	m_Temp = myData->bData[bd].cData[ch-1].op.temp;
	m_Vsens = myData->bData[bd].cData[ch-1].op.Vsens;
	m_tmpIsens = myData->bData[bd].cData[ch-1].misc.tmpIsens;
	m_opIsens = myData->bData[bd].cData[ch-1].op.Isens;

	step = step_info(bd, ch);
	
	advStepNo = step.advStepNo;
	rangeV = step.rangeV;
	rangeI = step.rangeI;
	saveDt = step.saveDt;
	val1 = step.refV;
	val2 = step.refI;
	type = step.type;
	mode = step.mode;
	inv_p = myPs->pcu_config.parallel_inv_ch;

	maxV = myPs->config.maxVoltage[rangeV];
	maxI = myPs->config.maxCurrent[rangeI];
	minI = myPs->config.minCurrent[rangeI];

	switch(type){
		case STEP_USER_PATTERN:
			if(myCh->op.checkDelayTime < 180) break;
			//20180314 sch modify
			if(mode == CC || mode == CCCV) {
				if(val2 > 0){
					delta = myCh->misc.tmpIsens - val2;
					m_Delta = m_tmpIsens - val2;
				}else if(val2 < 0){
					delta = val2 - myCh->misc.tmpIsens;
					m_Delta = val2 - m_tmpIsens;
				}else{
					delta = 0;
					m_Delta = 0;
				}
			
				if(((val1 - myCh->op.Vsens) > maxV * 0.001) ||
					((val1 - m_Vsens) > maxV * 0.001)){
					if((labs(delta) >= (long)(maxI*0.1)) ||
						(labs(m_Delta) >= (long)(maxI*0.1))) {
						myCh->misc.errCnt[C_CNT_CH_I_FAIL]++;
						if(myCh->misc.errCnt[C_CNT_CH_I_FAIL]>= MAX_ERROR_CNT){
							myCh->misc.errCnt[C_CNT_CH_I_FAIL] = 0;
							myCh->misc.tmpState = myCh->op.state;
							myCh->misc.tmpCode = myCh->op.code;
							myCh->op.code = C_FAULT_CH_I_FAIL;
							rtn = FAULT_COND;
							break;
						}
					}else{
						myCh->misc.errCnt[C_CNT_CH_I_FAIL] = 0;
					}
				}
				if(val2 > 0){
					delta = myCh->op.Vsens - val1;
					m_Delta = m_Vsens - val1;
				}else if(val2 < 0){
					delta = val1 - myCh->op.Vsens;
					m_Delta = val1 - m_Vsens;
				}else{
					delta = 0;
					m_Delta = 0;
				}
				if((delta >= (long)(maxV * 0.02)) ||
					(m_Delta >= (long)(maxV * 0.02))) {
					myCh->misc.errCnt[C_CNT_CH_V_FAIL]++;
					if(myCh->misc.errCnt[C_CNT_CH_V_FAIL]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_CH_V_FAIL] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_CH_V_FAIL;
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_CH_V_FAIL] = 0;
				}
			}
			
			//180726 add for digital
			if(myCh->misc.errCnt[C_CNT_PCU_SEQ_FAULT]
					>= MAX_ERROR_CNT_SEQ_PATTERN){
				myCh->misc.errCnt[C_CNT_PCU_SEQ_FAULT] = 0;
				myCh->misc.tmpState = myCh->op.state;
				myCh->misc.tmpCode = myCh->op.code;
				myCh->op.code = P_CD_FAULT_SEQ_NO;
				rtn = FAULT_COND;
				break;
			}
			//180611 add for digital
			if(myCh->misc.errCnt[C_CNT_PCU_FAULT] >= MAX_ERROR_CNT_P){
				myCh->misc.errCnt[C_CNT_PCU_FAULT] = 0;
				myCh->misc.tmpState = myCh->op.state;
				rtn = FAULT_COND;
				break;
			}
			//191024 add for digital
			/*
			if(myCh->misc.errCnt[C_CNT_PCU_INPUT_FAULT]>= MAX_ERROR_CNT_P){
				myCh->misc.errCnt[C_CNT_PCU_INPUT_FAULT] = 0;
				myCh->misc.tmpState = myCh->op.state;
				rtn = FAULT_COND;
				break;
			}*/
			//180709 add for digital inverter fault
			/*
			for(i = 0; i < inv_p; i++){
				if(myCh->misc.errCnt[C_CNT_PCU_INV_FLT1+i] 
													>= MAX_ERROR_CNT_P){
					myCh->misc.errCnt[C_CNT_PCU_INV_FLT1 + i] = 0;
					myCh->misc.tmpState = myCh->op.state;
					rtn = FAULT_COND;
					break;
				}
			}*/
			
			if(m_Temp > myData->bData[bd].cData[ch-1].misc.maxT) {
				myData->bData[bd].cData[ch-1].misc.maxT = m_Temp;
			}
			if(m_Temp < myData->bData[bd].cData[ch-1].misc.minT) {
				myData->bData[bd].cData[ch-1].misc.minT = m_Temp;
			}
			if(m_Vsens > myData->bData[bd].cData[ch-1].misc.maxV) {
				myData->bData[bd].cData[ch-1].misc.maxV = m_Vsens;
				myCh->misc.maxV = m_Vsens;
			}
			if(m_Vsens < myData->bData[bd].cData[ch-1].misc.minV) {
				myData->bData[bd].cData[ch-1].misc.minV = m_Vsens;
				myCh->misc.minV = m_Vsens;
			}
			break;
		default:
			break;
	}
	
	if(rtn == FAULT_COND) { //fault
		//master ch fault 
		myData->bData[bd].cData[ch-1].op.select = myCh->op.select;
		myData->bData[bd].cData[ch-1].op.code = myCh->op.code;
		myData->bData[bd].cData[ch-1].misc.tmpCode = myCh->misc.tmpCode;
		myData->bData[bd].cData[ch-1].misc.tmpState = myCh->misc.tmpState;
		myData->bData[bd].cData[ch-1].op.phase = P100;

		myCh->op.select = SAVE_FLAG_SAVING_ETC;
		send_save_msg(bd, ch, saveDt, 0);
		//170106 oys add
		if(myData->bData[bd].cData[ch-1].misc.tempWaitType == P0){
			myData->bData[bd].cData[ch-1].misc.chGroupNo = 0;
		//20180417 add
			myData->bData[bd].cData[ch-1].misc.stepSyncFlag = 0;
		}
		myCh->op.phase = P100;
		cNextStepCheck(bd, ch-1);
		myCh = &(myData->bData[bd].cData[ch]);
		cNextStepCheck(bd, ch);
	}
}

void pFaultCond_Common(int bd,int ch)
{
//	unsigned char type, mode;
	int rtn = 0, inv_p, i = 0;
	unsigned long saveDt;
	S_CH_STEP_INFO step;

	if(myData->mData.config.function[F_HW_FAULT_COND] == P0) return;
	
	myCh = &(myData->bData[bd].cData[ch]);

	if(myCh->op.state != C_RUN) return;
	if(myCh->op.phase != P50) return;

	step = step_info(bd, ch);
	
	saveDt = step.saveDt;
	inv_p = myPs->pcu_config.parallel_inv_ch;
	
	for(i = 0; i < inv_p; i++){
		if(myCh->misc.errCnt[C_CNT_PCU_INV_FLT1+i] > MAX_INV_ERR_CNT){
			myCh->misc.errCnt[C_CNT_PCU_INV_FLT1 + i] = 0;
			myCh->misc.tmpState = myCh->op.state;
			rtn = FAULT_COND;
			break;
		}
		
		if(myPs->pcu_config.inverterType == 1){
			if(myCh->misc.errCnt[C_CNT_INV_AC_1+i] > MAX_INV_ERR_CNT){
				myCh->misc.errCnt[C_CNT_INV_AC_1 + i] = 0;
				myCh->misc.tmpState = myCh->op.state;
				rtn = FAULT_COND;
				break;
			}
		}
	}

	if(rtn == FAULT_COND) { //fault
		myCh->op.select = SAVE_FLAG_SAVING_ETC;
		send_save_msg(bd, ch, saveDt, 0);
		//170106 oys add
		if(myCh->misc.tempWaitType == P0){
			myCh->misc.chGroupNo = 0;
			//20180417 add
			myCh->misc.stepSyncFlag = P0;
		}
		myCh->op.phase = P100;
		cNextStepCheck(bd, ch);
	}
}


void pStepCCCV_Check(int bd, int ch, long val1, long val2)
{
	unsigned char mode, type;
	int rangeV, rangeI;
	long tmpV=0, tmpI=0, maxV, maxI, deltaV=0, deltaI=0;
	S_CH_STEP_INFO step;

	step = step_info(bd, ch);

	rangeV = step.rangeV;
	rangeI = step.rangeI;
	type = step.type;
	mode = step.mode;

	maxV = myPs->config.maxVoltage[rangeV];
	maxI = myPs->config.maxCurrent[rangeI];

	switch(type) { //20180417 add for Cvtime
		case STEP_CHARGE:
			if(myCh->op.checkDelayTime < 100) {
				tmpV = val1 - myCh->misc.tmpVsens;
				tmpI = val2 - myCh->misc.tmpIsens;
			} else {
				tmpV = val1 - myCh->op.Vsens;
				tmpI = val2 - myCh->op.Isens;
			}

			//CC Field
			deltaV = (long)(maxV * 0.001);
			deltaI = (long)(maxI * 0.0025);

			//Check CV Flag
			if(tmpV < deltaV) {
				if(tmpI > deltaI) {
   					myCh->misc.cvFaultCheckFlag = P1; //210204
				}
			}
			break;
		case STEP_DISCHARGE:
			if(myCh->op.checkDelayTime < 100) {
				tmpV = myCh->misc.tmpVsens - val1;
				tmpI = val2 - myCh->misc.tmpIsens;
			} else {
				tmpV = myCh->op.Vsens - val1;
				tmpI = val2 - myCh->op.Isens;
			}

			//CC Field
			deltaV = (long)(maxV * 0.001);
			deltaI = (long)(maxI * 0.0025);

			//Check CV Flag
			if(tmpV < deltaV) {
				if(labs(tmpI) > deltaI) {
   					myCh->misc.cvFaultCheckFlag = P1; //210204
				}
			}
			break;
		default:
			break;
	}
}
#if CHAMBER_TEMP_HUMIDITY == 1	//kjc_210413
int chamber_temp_humidity_check(int ch, int bd, unsigned long advStepNo)
{
	unsigned long cham_sync_T, cham_check_time_pre = 0, cham_check_time_new;
	long cham_temp_ref, cham_humid_ref, ch_temp_ref;
	long cham_temp, ch_temp, cham_humid;
	long cham_temp_dev_U, cham_temp_dev_L;
	long cham_humid_dev_U, cham_humid_dev_L;
	long ch_temp_dev_U, ch_temp_dev_L;
	short int cham_temp_dev, cham_humid_dev, ch_temp_dev;
	int cham_temp_sig, cham_humid_sig, ch_temp_sig;
	int cham_temp_humid_sig;
	char cham_check_time_flag;
	int flag=0, cnt=0, i=0, bd_1=0, ch_1=0;

	if(myCh->misc.waitFlag == 0) return flag = 1;
	
	myCh = &(myData->bData[bd].cData[ch]);
	
	//present value
	cham_temp = myCh->misc.groupTemp;
	ch_temp = myCh->op.temp;
	cham_humid = myCh->misc.humi;
	
	//reference value
	cham_sync_T = myData->mData.testCond[bd][ch].step[advStepNo].cham_sync_T;
	cham_temp_ref = myData->mData.testCond[bd][ch].step[advStepNo].cham_temp;
	cham_humid_ref = myData->mData.testCond[bd][ch].step[advStepNo].cham_humid;
	ch_temp_ref = myData->mData.testCond[bd][ch].step[advStepNo].ch_temp;
	
	//devation value
	cham_temp_dev = myData->mData.testCond[bd][ch].step[advStepNo].cham_temp_dev;
	cham_humid_dev = myData->mData.testCond[bd][ch].step[advStepNo].cham_humid_dev;
	ch_temp_dev = myData->mData.testCond[bd][ch].step[advStepNo].ch_temp_dev;
	
	//using check signal value
	cham_temp_sig = myData->mData.testCond[bd][ch].step[advStepNo].cham_temp_sig;
	cham_humid_sig = myData->mData.testCond[bd][ch].step[advStepNo].cham_humid_sig;
	ch_temp_sig = myData->mData.testCond[bd][ch].step[advStepNo].ch_temp_sig;
	
	//chamber syncronization time check value
	cham_check_time_new = myCh->misc.cham_check_time_new;
	cham_check_time_flag = myCh->misc.cham_check_time_flag;	

	//chamber temperature deviation
	cham_temp_dev_U = cham_temp_ref + cham_temp_dev;
	cham_temp_dev_L = cham_temp_ref - cham_temp_dev;

	//chamber humidity deviation
	cham_humid_dev_U = cham_humid_ref + cham_humid_dev;
	cham_humid_dev_L = cham_humid_ref - cham_humid_dev;

	//channer temperature deviation
	ch_temp_dev_U = ch_temp_ref + ch_temp_dev;
	ch_temp_dev_L = ch_temp_ref - ch_temp_dev;

	cham_temp_humid_sig 
		= cham_temp_sig << 2 | cham_humid_sig << 1 | ch_temp_sig;

	if(cham_check_time_flag == 0 && cham_sync_T != 0){
		if(cham_check_time_new - cham_check_time_pre >= cham_sync_T){
			if(cham_temp_humid_sig >= 1){
				myPs->code = M_FAIL_CHAMBER_SYNC_TIME_OVER;
				send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
					(int)myPs->code, 0);
				myCh->misc.cham_check_time_flag = 1;
			}
		}
	}

	if(cham_temp_humid_sig != 0){
		if(myCh->misc.cham_check_time_flag == 0){
			myCh->misc.cham_check_time_1sec += myPs->misc.rt_scan_time;
				myCh->misc.cham_check_time_new ++;
		}
	}
	
	if(myCh->signal[C_SIG_STOP] == P1) {
		myCh->op.code = C_FAULT_STOP_CMD;
		myCh->signal[C_SIG_STOP] = P0;
		myCh->misc.chamberWaitFlag = -1;
		myCh->misc.chGroupNo = 0;
		myCh->op.phase = P100;
	} else if(myCh->signal[C_SIG_PAUSE] == P1) {
		myCh->misc.tmpState = myCh->op.state;
		myCh->op.code = C_FAULT_PAUSE_CMD;
		myCh->opSave = myCh->op;
		myCh->signal[C_SIG_PAUSE] = P0;
		cSemiSwitch_Rest(bd,ch,advStepNo,0);
		myCh->misc.chamberWaitFlag = -1;
		myCh->op.phase = P100;
	} else if(myCh->signal[C_SIG_DLL_STOP] == P1) {
		myCh->op.code = C_FAULT_CELL_DIAGNOSIS_STOP;
		myCh->signal[C_SIG_DLL_STOP] = P0;
		myCh->misc.chamberWaitFlag = -1;
		myCh->misc.chGroupNo = 0;
		myCh->op.phase = P100;
	}
	//140409 oys add : ChamberTempWaitFlag
	if(myData->mData.config.function[F_CHAMBER_TEMP_WAIT] == P0){
		return 1;
	}

	if(cFailCodeCheck(bd, ch) < 0) {
		myCh->misc.chamberWaitFlag = -1;
	}
	
	if(myCh->op.phase == P100) {
		cNextStepCheck(bd, ch);
		return flag;
	}

	if(myCh->misc.stepSyncFlag == P1 && myCh->misc.chamberWaitFlag >= P20){
		for(i = 0; i < myPs->config.installedCh; i++){
			bd_1 = i/myPs->config.chPerBd;
			ch_1 = i%myPs->config.chPerBd;
			if(myCh->misc.chGroupNo ==
				myData->bData[bd_1].cData[ch_1].misc.chGroupNo){
				if((myCh->misc.advStepNo != 
					myData->bData[bd_1].cData[ch_1].misc.advStepNo) ||
					(myCh->misc.totalCycle != 
					myData->bData[bd_1].cData[ch_1].misc.totalCycle)){
					if(myCh->misc.totalCycle > 
						myData->bData[bd_1].cData[ch_1].misc.totalCycle){
							cnt++;
					}else if(myCh->misc.totalCycle ==
						myData->bData[bd_1].cData[ch_1].misc.totalCycle){
						if(myCh->misc.advStepNo > 
							myData->bData[bd_1].cData[ch_1].misc.advStepNo){
								cnt++;
						}
					}
				}
			}
		}
	}
	if(cnt != 0){
		myCh->misc.chamberWaitFlag -= P10;
		cnt = 0;
	}

	if(myCh->misc.stepSyncFlag == P1){
		if(myCh->misc.chamberWaitFlag >= P20){
			if(myCh->ChAttribute.opType == P1){
				myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
				myData->bData[bd].cData[ch+1].op.checkDelayTime
						                     = myCh->op.checkDelayTime;
			}else{
				if(myCh->ChAttribute.chNo_master != P0){
					myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
				}
			}
			if(myCh->op.type != STEP_REST) {
				myCh->signal[C_SIG_OUT_SWITCH] = P1;	//run relay close
				if(myCh->signal[C_SIG_OUT_SWITCH_ON] != P3){
						myCh->signal[C_SIG_OUT_SWITCH_ON] = P1; 	
				}else{	//181202 add lyhw
					if(myCh->misc.send_pcu_seq_no 
									!= myCh->misc.receive_pcu_seq_no){
						myCh->misc.seq_no_cnt++;
						if(myCh->misc.seq_no_cnt < MAX_SEQ_NO_RETRY){
							myCh->signal[C_SIG_OUT_SWITCH_ON] = P1;			
						}
					}else{
							myCh->misc.seq_no_cnt = 0;
					}
				}
			}
			if(myCh->op.checkDelayTime >= 200){ //relay time wait
				myCh->op.checkDelayTime = 0;
				myCh->op.code = myCh->misc.tmpCode;
				myCh->misc.chamberWaitFlag = P0;
				myCh->misc.cham_check_time_flag = 0;
				myCh->misc.cham_check_time_new = 0;
				myCh->misc.cham_check_time_1sec = 0;
				flag = 1;
				return flag;
			}
			return flag;
		}
	 }	
	
	if(cham_temp_humid_sig != 0){
		if(myCh->misc.chamberWaitFlag == P0) {
			myCh->misc.chamberWaitFlag = P1;
			myCh->misc.tmpCode = myCh->op.code;
		}
		myCh->signal[C_SIG_NEXTSTEP] = P0; //nextstep message disable
		myCh->signal[C_SIG_STOP] = P0; //stop message disable
		myCh->signal[C_SIG_PAUSE] = P0; //pause message disable
		myCh->signal[C_SIG_DLL_STOP] = P0;
		myCh->signal[C_SIG_DLL_PAUSE] = P0;
		myCh->op.code = C_TEMP_WAIT_TIME;
		myCh->signal[C_SIG_OUT_SWITCH] = P0; //run relay open
		
		if(myCh->signal[C_SIG_OUT_SWITCH_OFF] != P3){
			myCh->signal[C_SIG_OUT_SWITCH_OFF] = P1; 	
		}else{	//181202 add lyhw
			if(myCh->misc.send_pcu_seq_no 
							!= myCh->misc.receive_pcu_seq_no){
				myCh->misc.seq_no_cnt++;
				if(myCh->misc.seq_no_cnt < MAX_SEQ_NO_RETRY){
					myCh->signal[C_SIG_OUT_SWITCH_OFF] = P1;			
				}
			}else{
				myCh->misc.seq_no_cnt = 0;
			}
		}
		switch(cham_temp_humid_sig){
			case P1 : //ch_temp
				if(ch_temp <= ch_temp_dev_U && ch_temp >= ch_temp_dev_L){
					myCh->misc.chamberWaitFlag = P10;
				} 
				break;
			case P2 : //cham_humid
				if(cham_humid <= cham_humid_dev_U && cham_humid >= cham_humid_dev_L){
					myCh->misc.chamberWaitFlag = P10;
				} 
				break;
			case P3 : //ch_temp + cham_humid
				if(ch_temp <= ch_temp_dev_U && ch_temp >= ch_temp_dev_L){
					if(cham_humid <= cham_humid_dev_U 
							&& cham_humid >= cham_humid_dev_L){
						myCh->misc.chamberWaitFlag = P11;
					} 
				}
				break;
			case P4 : //cham_temp
				if(cham_temp <= cham_temp_dev_U && cham_temp >= cham_temp_dev_L){
					myCh->misc.chamberWaitFlag = P10;
				} 
				break;
			case P5 : //cham_temp + ch_temp
				if(cham_temp <= cham_temp_dev_U && cham_temp >= cham_temp_dev_L){
					if(ch_temp <= ch_temp_dev_U && ch_temp >= ch_temp_dev_L){
						myCh->misc.chamberWaitFlag = P11;
					} 
				}
				break;
			case P6 : //cham_temp + cham_humid
				if(cham_temp <= cham_temp_dev_U && cham_temp >= cham_temp_dev_L){
					if(cham_humid <= cham_humid_dev_U 
							&& cham_humid >= cham_humid_dev_L){
						myCh->misc.chamberWaitFlag = P11;
					} 
				}
				break;
			case P7 : //cham_temp + cham_humid + ch_temp
				if(cham_temp <= cham_temp_dev_U && cham_temp >= cham_temp_dev_L){
					if(cham_humid <= cham_humid_dev_U 
							&& cham_humid >= cham_humid_dev_L){
						if(ch_temp <= ch_temp_dev_U && ch_temp >= ch_temp_dev_L){
							myCh->misc.chamberWaitFlag = P12;
						} 
					}
				}
				break;
			default:
				break;
		}
	}else{
		flag = 1;
	}
	return flag;
}
#endif
