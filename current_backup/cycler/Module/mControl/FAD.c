#include <asm/io.h>
#include <rtl_core.h>
#include <pthread.h>
#include <math.h>
#include "../../INC/datastore.h"
#include "FAD.h"
#include "Analog.h"

extern S_SYSTEM_DATA    *myData;
extern S_MODULE_DATA	*myPs;

void FadEndCheck(int ch)
{
	int bd, addr, base_addr, addr_step, fad_end_cs;

	if(myPs->config.FadBdUse == P0) return;

	base_addr = myPs->addr.main[BASE_ADDR];
	addr_step = myPs->addr.main[ADDR_STEP];
	for(bd = 0;bd < myPs->config.installedBd; bd++) {
		//171227 add
		if(myData->bData[bd].cData[ch].misc.d_flag2 == 1){
			myData->bData[bd].cData[ch].misc.d_flag2 = 2;
			Fad_Read_IEC(bd, ch, 0);
		}

		if(myData->bData[bd].cData[ch].misc.fadFlag == 0) continue;
		addr = base_addr + addr_step * bd;
		addr = addr + fad_end_cs + (ch / 8);

		if(myData->bData[bd].cData[ch].misc.fadFlag == 1) {
			myData->bData[bd].cData[ch].misc.fadTimer -=
				myPs->misc.dio_scan_time;
			if(myData->bData[bd].cData[ch].misc.fadTimer <= 0) {
				myData->bData[bd].cData[ch].misc.fadFlag = 0;
				Fad_Read(bd, ch, 1);
			}
		} else if(myData->bData[bd].cData[ch].misc.fadFlag == 4 ) {
			myData->bData[bd].cData[ch].misc.fadFlag = 2;
			Fad_Read(bd, ch, 2);
		} else if(myData->bData[bd].cData[ch].misc.fadFlag == 2 ) {
			myData->bData[bd].cData[ch].misc.fadFlag = 0;
			Fad_Read(bd, ch, 2);
		}
	}
}

void Fad_Read(int bd, int ch, int flag)
{
	int addr, base_addr, addr_step, fad_ch, i, rangeI, point=0;
	long adV[150],adI[150],adRefV[3],adRefI[3], val;
	double ref_v_a, ref_v_a_N, ref_v_b, ref_v_b_N;
	double ref_i_a, ref_i_a_N, ref_i_b, ref_i_b_N;
	double sumV, sumI; 
	unsigned char readData[1000];
	char *tmp1;
	U_ADDA ADValue;
	
	long msg = 2, idx, type;
	double tmp, sendVal, valV, valI;

	base_addr = myPs->addr.main[BASE_ADDR];
	addr_step = myPs->addr.main[ADDR_STEP];
	fad_ch = myPs->addr.main[FAD_CH] + ch;

	addr = base_addr + addr_step * bd;
	addr = addr + fad_ch;
	
	base_addr = 0x700; //jyk
	addr = base_addr + addr_step * bd + ch;

	for(i = 0; i < 700 ;i++) {
		readData[i] = (unsigned char)inb(addr);
	}
	if(flag == 2) return;// start data not save
	
	tmp1 = &readData[5];
	for(i = 0; i<150 ; i++) {
		ADValue.byte[1] = *tmp1++;
		ADValue.byte[0] = *tmp1++;
		adV[i] = (long)ADValue.val; //read V
		ADValue.byte[1] = *tmp1++;
		ADValue.byte[0] = *tmp1++;
		adI[i] = (long)ADValue.val; //read I
	}
	
	for(i = 0; i<3 ;i++) {
		ADValue.byte[1] = *tmp1++;
		ADValue.byte[0] = *tmp1++;
		adRefV[i] = (long)ADValue.val; //read refV
	}
	
	for(i = 0; i<3 ;i++) {
		ADValue.byte[1] = *tmp1++;
		ADValue.byte[0] = *tmp1++;
		adRefI[i] = (long)ADValue.val; //read refI
	}
	
	switch(myPs->config.hwSpec) {
	    case L_30V_5A_R1_AD2:
	        val = 18604927;
	        break;
	    case L_60V_100A_R1_AD2:
	        val = 37598231;
	        break;
	    default:
 	       val = myData->mData.config.maxVoltage[0];
        break;
	}

	ref_v_a = (double)(val - 0)	/ (double)(adRefV[0] - adRefV[2]);
	ref_v_b = val - (double)(adRefV[0] * ref_v_a);
	
	ref_v_a_N = (double)(0.0 - (-val)) / (double)(adRefV[2] - adRefV[1]);
	ref_v_b_N = (double)(0.0) - (double)(adRefV[2] * ref_v_a_N);

	if(myData->bData[bd].cData[ch].op.state == C_CALI){
		rangeI = myData->cali.tmpCond[bd][ch].range;
	}else{
		rangeI = myData->FADM.pulse_ad[bd][ch].rangeI;
	}
	val = myData->mData.config.maxCurrent[rangeI];
	
	switch(myPs->config.hwSpec) {
	    case L_5V_150A_R3_AD2: //khkw 20130709
		case L_8CH_MAIN_AD2_P:
			adRefI[0] = 28600; 
			adRefI[1] = -28600;
			adRefI[2] = 0;
			break;
		default:
			break;
	}
	ref_i_a = (double)(val - 0) / (double)(adRefI[0] - adRefI[2]);
	ref_i_b = val - (double)(adRefI[0] * ref_i_a);

	ref_i_a_N = (double)(0.0 - (-val)) / (double)(adRefI[2] - adRefI[1]);
	ref_i_b_N = (double)(0.0) - (double)(adRefI[2] * ref_i_a_N);
	
	sumV = sumI = 0.0;
	if(myData->bData[bd].cData[ch].ChAttribute.opType == P1){
		for(i=0; i<150; i++){
			myData->FADM.Isens[i] = 0;
		}
	}
	for(i= 0 ;i< 150; i++) {
		if(i == 0) type = 0;
		else if(i == 149) type = 2;
		else type = 1;
	   	
		if(myData->bData[bd].cData[ch].ChAttribute.opType == P1){
			idx = i;
		}else if(myData->bData[bd].cData[ch].ChAttribute.opType == P0
				&& myData->bData[bd].cData[ch].ChAttribute.chNo_master == P0){
			idx = myData->pulse_msg[msg][bd][ch-1].write_idx;
			idx++;
		}else{
			idx = myData->pulse_msg[msg][bd][ch].write_idx;
			idx++;
		}
		if(idx >= MAX_PULSE_MSG) idx = 0;
		
		if(myData->bData[bd].cData[ch].ChAttribute.opType == P1){
		}else if(myData->bData[bd].cData[ch].ChAttribute.opType == P0
				&& myData->bData[bd].cData[ch].ChAttribute.chNo_master == P0){
			myData->pulse_msg[msg][bd][ch-1].val[idx].type = type;
			myData->pulse_msg[msg][bd][ch-1].val[idx].runTime = (i * 2) * (-1);
		}else{
			myData->pulse_msg[msg][bd][ch].val[idx].type = type;
			myData->pulse_msg[msg][bd][ch].val[idx].runTime = (i * 2) * (-1);
		}
		if(adV[i] > 0)
			tmp = adV[i] * ref_v_a + ref_v_b;
		else
			tmp = adV[i] * ref_v_a_N + ref_v_b_N;

		if(myData->bData[bd].cData[ch].op.state == C_CALI){
			if(myData->bData[bd].cData[ch].signal[C_SIG_CALI_PHASE] == P0){
				sendVal = tmp;
			}else{
				point = cFindADCaliPoint(bd, ch, (long)tmp, 0, 0);
				sendVal = tmp 
					* myData->cali.tmpData_fad[bd][ch].AD_A[0][0][point]
					- myData->cali.tmpData_fad[bd][ch].AD_B[0][0][point];
			}
		}else{
			point = cFindADCaliPoint(bd, ch, (long)tmp, 0, 0);
			sendVal = tmp 
				* myData->cali.data_fad[bd][ch].AD_A[0][0][point]
				- myData->cali.data_fad[bd][ch].AD_B[0][0][point];
		}
		if(i >= 100){
			sumV += sendVal;
		}

		valV = sendVal;
		

		if(adI[i] > 0)
			tmp = adI[i] * ref_i_a + ref_i_b;
		else
			tmp = adI[i] * ref_i_a_N + ref_i_b_N;

//		tmp = tmp * myData->bData[bd].cData[ch].misc.fadGainI 
//					+ myData->bData[bd].cData[ch].misc.fadOffsetI; 
		
		if(myData->bData[bd].cData[ch].op.state == C_CALI){
			rangeI = myData->cali.tmpCond[bd][ch].range;
			if(myData->bData[bd].cData[ch].signal[C_SIG_CALI_PHASE] == P20){
				sendVal = tmp;
			}else{
				point = cFindADCaliPoint(bd, ch, (long)tmp, 1, rangeI);
				sendVal = tmp 
					* myData->cali.tmpData_fad[bd][ch].AD_A[1][rangeI][point]
					+ myData->cali.tmpData_fad[bd][ch].AD_B[1][rangeI][point];
			}
		}else{
			point = cFindADCaliPoint(bd, ch, (long)tmp, 1, rangeI);
			sendVal = tmp 
				* myData->cali.data_fad[bd][ch].AD_A[1][rangeI][point]
				+ myData->cali.data_fad[bd][ch].AD_B[1][rangeI][point];
		}

		if(i >= 100){
			sumI += sendVal;
		}

		valI = sendVal;

		if(myData->bData[bd].cData[ch].op.state == C_CALI){
			if(i == 149){
				myData->cali.orgAD_fad[0] = sumV / 50.0;
				myData->cali.orgAD_fad[1] = sumI / 50.0;
			}
		}

		if(myData->bData[bd].cData[ch].op.state == C_CALI){
				sendVal =  valV;
		}else{
			if(fabs(valI) >= fabs(myPs->config.minCurrent[rangeI] * 0.3)){
				sendVal =  valV - fabs(valI/(double)myPs->config.minCurrent[rangeI])*myData->FADM.config.fad_offset[rangeI];  
				// V = adv - (adi/min current) * offsetV;
			}else{
				sendVal =  valV;
			}
		}

		if(myData->bData[bd].cData[ch].ChAttribute.opType == P1){
			myData->FADM.Isens[i] = (long)valI;
		}else if(myData->bData[bd].cData[ch].ChAttribute.opType == P0
				&& myData->bData[bd].cData[ch].ChAttribute.chNo_master == P0){
			myData->pulse_msg[msg][bd][ch-1].val[idx].Vsens = (long)sendVal;
			myData->pulse_msg[msg][bd][ch-1].val[idx].Isens
				= myData->FADM.Isens[i] + (long)valI;

			myData->pulse_msg[msg][bd][ch-1].val[idx].totalCycle
				= myData->bData[bd].cData[ch-1].misc.fadTotalCycle;
			myData->pulse_msg[msg][bd][ch-1].val[idx].stepNo
				= myData->bData[bd].cData[ch-1].misc.fadStepNo;

			myData->pulse_msg[msg][bd][ch-1].val[idx].capacity = 0;
			myData->pulse_msg[msg][bd][ch-1].val[idx].wattHour = 0;
			
			myData->pulse_msg[msg][bd][ch-1].write_idx = idx;
			myData->pulse_msg[msg][bd][ch-1].count++;
		}else{

			myData->pulse_msg[msg][bd][ch].val[idx].Vsens = (long)sendVal;
			myData->pulse_msg[msg][bd][ch].val[idx].Isens = (long)valI;

			myData->pulse_msg[msg][bd][ch].val[idx].totalCycle
				= myData->bData[bd].cData[ch].misc.fadTotalCycle;
			myData->pulse_msg[msg][bd][ch].val[idx].stepNo
				= myData->bData[bd].cData[ch].misc.fadStepNo;

			myData->pulse_msg[msg][bd][ch].val[idx].capacity = 0;
			myData->pulse_msg[msg][bd][ch].val[idx].wattHour = 0;
			
			myData->pulse_msg[msg][bd][ch].write_idx = idx;
			myData->pulse_msg[msg][bd][ch].count++;
		}
	}	
}

void Fad_Read_IEC(int bd, int ch, int flag)
{
	int i;
	long idx;
///	long type;
	int msg =2;
	int data_num = MAX_PULSE_DATA_IEC;
			
	for(i= 0 ;i< data_num; i++) {
		if(myData->bData[bd].cData[ch].ChAttribute.opType == P1){
			idx = i;
		}else if(myData->bData[bd].cData[ch].ChAttribute.opType == P0
				&& myData->bData[bd].cData[ch].ChAttribute.chNo_master == P0){
			idx = myData->pulse_msg_iec[msg][bd][ch-1].write_idx;
			idx++;
		}else{
			idx = myData->pulse_msg_iec[msg][bd][ch].write_idx;
			idx++;
		}
		if(idx > MAX_PULSE_DATA_IEC) idx = 0;
//		if(idx >= MAX_MSG_RING) idx = 0;
//		if(idx >= MAX_PULSE_MSG) idx = 0;
		if(myData->bData[bd].cData[ch].ChAttribute.opType == P1){
		}else if(myData->bData[bd].cData[ch].ChAttribute.opType == P0
				&& myData->bData[bd].cData[ch].ChAttribute.chNo_master == P0){
			myData->pulse_msg_iec[msg][bd][ch-1].val[idx].totalCycle
				= myData->bData[bd].cData[ch-1].misc.totalCycle;
//				= myData->bData[bd].cData[ch-1].misc.fadTotalCycle;
			myData->pulse_msg_iec[msg][bd][ch-1].val[idx].stepNo
				= myData->bData[bd].cData[ch-1].op.stepNo;
			myData->pulse_msg_iec[msg][bd][ch-1].val[idx].runTime 
				= myData->bData[bd].cData[ch-1].misc.d_t_iec[i];
			myData->pulse_msg_iec[msg][bd][ch-1].val[idx].Vsens 
				= myData->bData[bd].cData[ch-1].misc.d_v_iec[i];
			myData->pulse_msg_iec[msg][bd][ch-1].val[idx].Isens 
				= myData->bData[bd].cData[ch-1].misc.d_i_iec[i];
			
			myData->pulse_msg_iec[msg][bd][ch-1].write_idx = idx;
			myData->pulse_msg_iec[msg][bd][ch-1].count++;
		}else{
			myData->pulse_msg_iec[msg][bd][ch].val[idx].type = 0;
			myData->pulse_msg_iec[msg][bd][ch].val[idx].totalCycle
				= myData->bData[bd].cData[ch].misc.totalCycle;
//				= myData->bData[bd].cData[ch].misc.fadTotalCycle;
			myData->pulse_msg_iec[msg][bd][ch].val[idx].stepNo
				= myData->bData[bd].cData[ch].op.stepNo;
			myData->pulse_msg_iec[msg][bd][ch].val[idx].runTime 
				= myData->bData[bd].cData[ch].misc.d_t_iec[i];
			myData->pulse_msg_iec[msg][bd][ch].val[idx].Vsens 
				= myData->bData[bd].cData[ch].misc.d_v_iec[i];
			myData->pulse_msg_iec[msg][bd][ch].val[idx].Isens 
				= myData->bData[bd].cData[ch].misc.d_i_iec[i];
			
			myData->pulse_msg_iec[msg][bd][ch].write_idx = idx;
			myData->pulse_msg_iec[msg][bd][ch].count++;
		}
	}
	
	if(myData->bData[bd].cData[ch].ChAttribute.opType == P0
				&& myData->bData[bd].cData[ch].ChAttribute.chNo_master == P0){
		data_num = myData->bData[bd].cData[ch-1].misc.nAfterIncludeCnt_iec-1;
	}else{
		data_num = myData->bData[bd].cData[ch].misc.nAfterIncludeCnt_iec-1;
	}
}
