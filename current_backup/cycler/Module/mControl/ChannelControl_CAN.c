#include <asm/io.h>
#include <math.h>
#include <rtl_core.h>
#include <pthread.h>
#include "../../INC/datastore.h"
#include "message.h"
#include "Analog.h"
#include "ChannelControl.h"
#include "ChannelControl_CAN.h"
#include "local_utils.h"
#include "CAN.h"
#include "FaultCond.h"

extern S_SYSTEM_DATA *myData;
extern S_MODULE_DATA *myPs;    
extern S_TEST_CONDITION *myTestCond;	//190901 lyhw
extern S_CH_DATA *myCh;

void ChannelControl_Ch_CAN(int bd, int ch)
{
	if(myData->AppControl.config.systemType != CYCLER_CAN) return;
	if(bd >= myPs->config.installedBd) return;
	if(ch >= myPs->config.chPerBd) return;

	myCh = &(myData->bData[bd].cData[ch]);

	CAN_Inverter_Control(bd, ch);		//220112 lyhw
	CAN_FaultCond_Common(bd, ch);		//220112 lyhw

	if(myCh->ChAttribute.chNo_master == P0) { 
		if(myCh->op.runTime % 10 == 0) {
			cFaultCondSoft_P(bd, ch); 
			cFaultCondHard_P(bd, ch); 		
#if AUX_CONTROL == 1
	#if NETWORK_VERSION >= 4103
			cFaultCond_Aux_P(bd, ch); //20190802
	#endif
#endif
		}
	} else if(myCh->ChAttribute.opType == P0) { //kjiw
		//general ch check
		if(myCh->op.runTime % 10 == 0) {
			cFaultCondSoft(bd, ch);
			cFaultCondHard(bd, ch); 

#if AUX_CONTROL == 1
	#if NETWORK_VERSION >= 4103
			cFaultCond_Aux(bd, ch);
	#endif
#endif
		}
	}

	cStep_CAN(bd, ch);
	cStopCond(bd, ch);
	cEndCond(bd, ch);

	RangeSelectI_Ch_CAN(bd, ch);
	OutputSwitch_OnOff_Ch_CAN(bd, ch);
	/*
	RangeSelectV_Ch(bd, ch);
	//180404 add for parallel Range Select
	if(myCh->ChAttribute.chNo_master == P0) {
		RangeSelectI_Ch(bd, ch - 1);
		RangeSelectI_Ch(bd, ch);
	} else {
		if(myCh->ChAttribute.opType == P0) {
			RangeSelectI_Ch(bd, ch);
		}
	}
	ParallelSwitch_OnOff_Ch(bd, ch);
	OutputSwitch_OnOff_Ch(bd, ch);
	OutputTrigger_Ch(bd, ch);
	Status_LED_OnOff(bd);
	CompWriteCh(bd, ch);
	*/
}

void cStep_CAN(int bd, int ch)
{
	myCh = &(myData->bData[bd].cData[ch]);

	switch(myCh->op.state) {
   		case C_IDLE:	cIdle_CAN(bd, ch);		break;
   		case C_STANDBY:	cStandby_CAN(bd, ch);	break;
		case C_CALI: 	cCali(bd, ch);			break;
		case C_PAUSE:	cPause_CAN(bd, ch);		break;
		case C_RUN:
			//111125 oys w : totalRunTime increase			
			if(myCh->op.totalRunTime >= MAX_TOTAL_RUNTIME) {
				myCh->op.totalRunTime_carry += 1;
				myCh->op.totalRunTime = 0;
			}

			switch(myCh->op.type) {
				case STEP_CHARGE:		cStepCharge_CAN(bd, ch);	break;
				case STEP_DISCHARGE:	cStepDischarge_CAN(bd, ch);	break;
				case STEP_REST:			cStepRest_CAN(bd, ch);		break;
				case STEP_OCV:			cStepOcv_CAN(bd, ch);		break;
				case STEP_Z:			cStepZ_CAN(bd, ch);			break;
				case STEP_USER_PATTERN:	cStepUserPattern_CAN(bd, ch);	break;
				case STEP_USER_MAP:		cStepUserMap_CAN(bd, ch);		break;
				case STEP_BALANCE:		cStepBalance_CAN(bd, ch);		break;
				case STEP_ACIR:			cStepAcir(bd, ch);				break;
				default: 				cStepDefault(bd, ch);			break;
			}
			break;
    	default:
			break;
	}
}

void cIdle_CAN(int bd, int ch)
{
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
			myCh->op.select = 0;
    		myCh->op.grade = 0;
	       	myCh->op.stepNo = 0;
			myCh->op.meanVolt = 0;
			myCh->op.meanCurr = 0;
        	myCh->op.watt = 0;
	       	myCh->op.wattHour = 0;
   		  	myCh->op.ampareHour = 0;
   		  	myCh->op.capacitance = 0;
			//20180206 sch
			myCh->op.capacitance_iec = 0;	
			myCh->op.capacitance_maxwell = 0;	
	       	myCh->op.z = 0;
			myCh->misc.saveZ = 0;
			myCh->op.resultIndex = 0;
			//141208 oys SDI MES VER4 data
			myCh->misc.step_count = 0;
			myCh->op.meanTemp = 0;
			//190618 oys add
			myCh->misc.can_read_v = 0;
			myCh->misc.can_read_i = 0;
			myCh->misc.can_read_errCnt = 0;
			myCh->misc.can_error = 0;
			//200317 lyhw
			myCh->misc.reserved_cmd_flag = 0;
			myCh->misc.preMode = 0;
			myCh->misc.can_inv_errflag = 0;
			myCh->misc.canNextStepFlag = P0;
		    myCh->op.phase = P1;
    		break;
		case P1:
			cSemiSwitch_CAN(bd, ch);
		    myCh->op.phase = P2;
			break;
		case P2:	//220113 lyhw
			if(myPs->signal[M_SIG_INV_POWER_CAN] == P99){
				myPs->signal[M_SIG_INV_POWER_CAN] = P100;
			}
		    myCh->op.phase = P3;
			break;
		case P3:
			if(myPs->misc.timer_1sec >= 2) { //2sec
				myCh->opSave = myCh->op;
	    	    myCh->op.state = C_STANDBY;
	        	myCh->op.phase = P0;
			}
			break;
		default: break;
	}
}

void cStandby_CAN(int bd, int ch)
{
	int i, j=0, type, parallel_ch, k=0, Inv_rtn = 0;
	long refTemp, groupTemp, refTemp_backup;
	unsigned long delay_time, startStepNo, advStepNo, saveDt;
	
	myCh = &(myData->bData[bd].cData[ch]);
    advStepNo = myCh->misc.advStepNo;
	saveDt = myPs->testCond[bd][ch].step[advStepNo].saveDt;
	
	if((ch % 2) == 0) parallel_ch = ch + 1;
	else parallel_ch = ch - 1;

	switch(myCh->op.phase) {
		case P0:
			myCh->op.checkDelayTime = 0;
			myCh->op.phase = P1;
		case P1:
			if(myCh->op.select == SAVE_FLAG_SAVING_END
				&& myCh->op.code != C_END_STEP) {
				if(myData->DataSave.config.save_data_type == P1) {
					myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
					if(myCh->op.checkDelayTime > 100) { 
						myCh->op.checkDelayTime = 0;
						if(myCh->ChAttribute.opType == P0) {
							send_save_msg(bd, ch, saveDt, 1);
						}
						myCh->op.phase = P2;
					}else{
						break;
					}
				} else {
					if(myCh->ChAttribute.opType == P0) {
						send_save_msg(bd, ch, saveDt, 1);
					}
					myCh->op.phase = P2;
				}
			} else {
				myCh->op.phase = P2;
			}
		case P2:
			for(i=0; i < MAX_SIGNAL; i++){ 
				if(i == C_SIG_OUT_SWITCH 
					|| i == C_SIG_OUT_SWITCH_ON
					|| i == C_SIG_OUT_SWITCH_OFF
					|| i == C_SIG_RANGE_SWITCH
					|| i == C_SIG_RANGE_SWITCH_ON
					|| i == C_SIG_RANGE_SWITCH_OFF){ 
					continue;
				}else{ 
					myCh->signal[i] = P0;
				}
			}
			
			for(i=0; i < MAX_STEP; i++) {
				myCh->misc.gotoCycleCount[i] = P0;
				myCh->misc.ahEndRatio[i] = 100; //default : 100
				myCh->misc.whEndRatio[i] = 100; //default : 100
				myCh->misc.sel_Cyc_C_Cap[i] = 0;
				myCh->misc.sel_Cyc_D_Cap[i] = 0;
			}

			for(i=0; i < MAX_CYCLE; i++) { //pjy add for toshiba
				myCh->misc.charge_integralCap[i] = 0;
				myCh->misc.discharge_integralCap[i] = 0;
			}

			myCh->op.rangeV = 0;
			myCh->op.rangeI = 0;
			if(myData->AppControl.config.debugType == P0) {
				myCh->op.preType = STEP_IDLE;
			}
			myCh->op.semiPreType = STEP_IDLE;
			myCh->misc.semiSwitchState = SEMI_IDLE;

			//210127 add for Ch init
			myCh->op.phase = P9;
			break;
		case P3:
			if(myData->AppControl.loadProcess[LOAD_EXTCLIENT] == P1) {
				if(myCh->op.type == STEP_END) {
				} else {
					myCh->op.type = 0;
				}
			} else {
				myCh->op.z = 0;
				myCh->op.type = 0;
			}

			//170215 SCH add for DeltaV/I
			if(myCh->op.code == C_STOP_DELTA_V 
				|| myCh->op.code == C_STOP_DELTA_I
				|| myCh->op.code == C_FAULT_END_LIMIT_CURRENT
				|| myCh->op.code == C_FAULT_END_LIMIT_VOLTAGE) {
			} else {
    			myCh->op.code = C_CODE_IDLE;
			}
 			//myCh->op.code = C_CODE_IDLE;

			myCh->op.type = 0;
			myCh->op.mode = 0;
    		myCh->op.totalRunTime = 0;
   			myCh->op.totalRunTime_carry = 0;
			myCh->op.select = 0;
			myCh->op.grade = 0;
	       	myCh->op.stepNo = 0;
	       	myCh->op.runTime = 0;
   			myCh->misc.cvFlag = 0;
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
			//20180206
			myCh->op.capacitance_iec = 0;	
			myCh->op.capacitance_maxwell = 0;	
	       	myCh->op.z = 0;
			myCh->misc.saveZ = 0;
			myCh->op.resultIndex = 0;
			myCh->misc.preRangeI = 99;
			myCh->misc.pulseDataCount = 0;
			myCh->misc.save10msDataCount = 0;
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
			myCh->misc.actualWattHour = 0; //20190214 add
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
			myCh->misc.relayStartFlag = 0; //110402 kji	
			//151214 oys
			myCh->misc.completeFlag = 0;
			myCh->misc.standardC_Flag = 0;
			myCh->misc.standardP_Flag = 0;
			myCh->misc.standardZ_Flag = 0;
			//160510 oys	
			myCh->misc.socCheckCount = 0;
			myCh->misc.socCountNo[0] = 0;
			myCh->misc.socCountNo[1] = 0;
			myCh->misc.socCountNo[2] = 0;
			myCh->misc.cycleEndC = 0;
			myCh->misc.standardC = 0;
			myCh->misc.standardP = 0;
			myCh->misc.standardZ = 0;
			myCh->misc.cycleSumC = 0;
			myCh->misc.cycleSumP = 0;
			myCh->misc.endC_std_type = 0;
			myCh->misc.endP_std_type = 0;
			myCh->misc.endZ_std_type = 0;
			myCh->misc.endC_std_sel = 0;
			myCh->misc.endP_std_sel = 0;
			myCh->misc.endZ_std_sel = 0;
			myCh->misc.endC_std_cycleCount = 0;
			myCh->misc.endP_std_cycleCount = 0;
			myCh->misc.endZ_std_cycleCount = 0;
			myCh->misc.pause_flag = P0;
			myCh->misc.efficiency_pause_flag = P0;
			
			myCh->misc.chamberWaitFlag = P0;
			myCh->misc.cycle_p_flag = P0;			//190308 lyhw add
			myCh->misc.cycle_p_sch_flag = P0;		//190318 lyhw add

			//111215 detail calibration add
			myCh->misc.caliCheckPoint = 0;
			myCh->misc.caliCheckSum = 0;
			myCh->misc.caliCheckSum1 = 0;
			
			//141208 oys SDI MES VER4 data
			myCh->misc.cycleMaxV = 0;
			myCh->misc.cycleMinV = 0;
			myCh->op.meanTemp = 0;
			myCh->misc.meanSumTemp = 0;
			myCh->misc.cycleSumChargeAmpareHour = 0;
			myCh->misc.cycleSumDischargeAmpareHour = 0;
			myCh->misc.sumChargeCCCVAh = 0;
			myCh->misc.seedChargeCCCVAh = 0;
			myCh->misc.chargeCCCVAh = 0;
			myCh->misc.chargeCCAh = 0;
			myCh->misc.chargeCVAh = 0;
			myCh->misc.sumDischargeCCCVAh = 0;
			myCh->misc.seedDischargeCCCVAh = 0;
			myCh->misc.dischargeCCCVAh = 0;
			myCh->misc.dischargeCCAh = 0;
			myCh->misc.dischargeCVAh = 0;
			myCh->misc.cycleSumChargeCCAh = 0;
			myCh->misc.cycleSumChargeCVAh = 0;
			myCh->misc.cycleSumDischargeCCAh = 0;
			myCh->misc.cycleSumDischargeCVAh = 0;
			myCh->misc.cycleSumChargeWatt = 0;
			myCh->misc.cycleSumDischargeWatt = 0;
			myCh->misc.cycleSumChargeWattHour = 0;
			myCh->misc.cycleSumDischargeWattHour = 0;
			myCh->misc.cycleStepCount = 0;
			myCh->misc.cycleDischargeStepCount = 0;
			myCh->misc.mes_data_flag = 0;
			myCh->misc.cycleStartV = 0;
			myCh->misc.step_count = 0;
			myCh->misc.ccTime = 0;
			myCh->misc.cycle_Charge_ccTime = 0;
			myCh->misc.cycle_Charge_cvTime = 0;
			myCh->misc.cycle_Discharge_ccTime = 0;
			myCh->misc.cycle_Discharge_cvTime = 0;
	
			myCh->misc.cycleAvgDischargeV = 0;
			myCh->misc.cycleAvgDischargeI = 0;
			myCh->misc.cycleSumAvgT = 0;
			//170105 oys
			myCh->misc.chGroupNo = 0;
			myCh->misc.chGroupCheckFlag = 0;
			myCh->misc.chamberNo = 0;
			myCh->misc.stepSyncFlag = 0;

			myCh->misc.feedback_start = P0;
			//170728 oys
			myCh->misc.gradeProcFlag = P0;
			//20171212 sch add
			myCh->misc.refTemp_backup = 999000; 
			//20171227 add
			myCh->misc.nAfterIncludeCnt_iec = 0;

			myCh->misc.parallel_cycle_phase = P0; //kjg_180521
			myCh->misc.parallel_sensFlag = P0; //kjg_180523
			//kjg_180524
			if(myData->bData[bd].cData[parallel_ch].misc.parallel_cycle_phase
				!= P50) {
				myData->bData[bd].cData[parallel_ch].misc.sensCount = 0;
				myData->bData[bd].cData[parallel_ch].misc.sensCountFlag = P0;
				myData->bData[bd].cData[parallel_ch].misc.sensBufCount = 0;
				myData->bData[bd].cData[parallel_ch].misc.sensBufCountFlag = P0;
			}
			myCh->misc.grade_flag = 0; //20190513 add

			//20180206 sch add for capacitance iec & maxwell
			myCh->misc.delta_t_iec[0] = 0;
			myCh->misc.delta_t_iec[1] = 0;
			myCh->misc.delta_v_iec[0] = 0;
			myCh->misc.delta_v_iec[1] = 0;
			myCh->misc.delta_w_iec[0] = 0;
			myCh->misc.delta_w_iec[1] = 0;
			myCh->misc.delta_flag_iec = P0;
			myCh->misc.delta_sumI_iec = 0;
			myCh->misc.delta_cnt_iec = 0;

			myCh->misc.delta_t_maxwell[0] = 0;
			myCh->misc.delta_t_maxwell[1] = 0;
			myCh->misc.delta_v_maxwell[0] = 0;
			myCh->misc.delta_v_maxwell[1] = 0;
			myCh->misc.delta_flag_maxwell = P0;
			myCh->misc.delta_sumI_maxwell = 0;
			myCh->misc.delta_cnt_maxwell = 0;
			//20190607 add
			myCh->misc.Pre_change_V = 0; 
			myCh->misc.change_V = 0; 
			myCh->misc.change_V_timer = 0;
			//20190719 oys add
			myCh->misc.save10msDt = 0;
			//190801 oys
			myCh->misc.cRateUseFlag = 0;
			//190829 oys
			myCh->misc.pre_v_chk_time = 0;
			myCh->op.changeV_Dt_cnt = 0;	//hun_200430
			myCh->misc.pre_chk_v = 0;
			myCh->misc.deltaV_timeout = 0;
			myCh->misc.deltaI_timeout = 0;
			//191029 oys
			myCh->misc.userDataNo = 0;
			myCh->misc.pattern_point_runTime = 0;
			myCh->misc.pattern_cross = 0;
			//200430 hun
			#ifdef _SDI_SAFETY_V1	
			myCh->misc.fault_deltaV = 0;
			#endif
			#ifdef _SDI_SAFETY_V2	
			myCh->misc.Master_recipe_deltaV = 0;
			#endif
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
			
			memset((char *)&myCh->misc.d_t, 0, sizeof(long)*100);
			memset((char *)&myCh->misc.d_v, 0, sizeof(long)*100);
			memset((long *)&myCh->misc.d_t_iec, 0x00, sizeof(long)*MAX_PULSE_DATA_IEC);
			memset((long *)&myCh->misc.d_v_iec, 0x00, sizeof(long)*MAX_PULSE_DATA_IEC);
			memset((long *)&myCh->misc.d_i_iec, 0x00, sizeof(long)*MAX_PULSE_DATA_IEC);
			memset((char *)&myCh->ccv, 0, sizeof(S_CH_CCV) * 2);
			//210128
			myCh->misc.preMode = 0;
			myCh->misc.can_inv_errflag = 0;
    		myCh->op.phase = P4;
			break;
		case P4:
			myCh->op.checkDelayTime = 0;
			myCh->signal[C_SIG_OUT_SWITCH_ON] = P0;
			myCh->signal[C_SIG_OUT_SWITCH_OFF] = P0;
			myCh->opSave = myCh->op;
			myCh->op.phase = P5;
			break;
		case P5:
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
			//210105 lyhw
			if(myPs->signal[M_SIG_INV_POWER_CAN] == P6){
				cSemiSwitch_CAN(bd, ch);
			}

			if(myCh->signal[C_SIG_RUN] == P1) {
				myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
				Inv_rtn = INV_StateCheck();		//220113 lyhw
				if(Inv_rtn != P1 && myCh->op.checkDelayTime <= 6000) return;
				myCh->op.checkDelayTime = 0;
				
				myCh->signal[C_SIG_RUN] = P0;
				myCh->op.code = C_CODE_IDLE; //20160810

				#ifdef _JIG_TYPE_1 //120206 kji
				if(myPs->config.hwSpec == L_5V_200A_1CH_JIG) {
					if(myData->dio.signal[DIO_SIG_JIG_RIGHT_UPPER] == P1) {
						myPs->code = M_FAIL_JIG_RIGHT_UPPER;
						send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
							(int)myPs->code, 0);
						myPs->signal[M_SIG_JIG_BUZZER] = P1;
						myCh->op.state = C_STANDBY;
						myCh->op.phase = P0;
						return;
					} else if(myData->dio.signal[DIO_SIG_JIG_LEFT_UPPER] == P1){
						myPs->code = M_FAIL_JIG_LEFT_UPPER;
						send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
							(int)myPs->code, 0);
						myPs->signal[M_SIG_JIG_BUZZER] = P1;
						myCh->op.state = C_STANDBY;
						myCh->op.phase = P0;
						return;
					} else if(myData->dio.signal[DIO_SIG_JIG_LIMIT] == P1) {
						myPs->code = M_FAIL_JIG_LIMIT_ERROR;
						send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
							(int)myPs->code, 0);
						myPs->signal[M_SIG_JIG_BUZZER] = P1;
						myCh->op.state = C_STANDBY;
						myCh->op.phase = P0;
						return;
					} else {
						myPs->signal[M_SIG_JIG_PASS_LAMP] = P0;		 
						myPs->signal[M_SIG_JIG_FAIL_LAMP] = P0;		 
						myPs->signal[M_SIG_JIG_SOL] = P1;
					}
				}
				#endif

				if(myPs->testCond[bd][ch].reserved.select_run == 0) {
					myCh->op.type = myPs->testCond[bd][ch].step[0].type;
					myCh->op.mode = myPs->testCond[bd][ch].step[0].mode;
					i = 1;
				} else {
					j = (int)myPs->testCond[bd][ch].reserved.select_stepNo - 1;
					if(j < 0) j = 0;
					// 120427 oys w : select cycle & step process
					// 170630 oys modify
					myCh->op.type = myPs->testCond[bd][ch].step[j].type;
					myCh->op.mode = myPs->testCond[bd][ch].step[j].mode;
					myCh->op.stepNo = myPs->testCond[bd][ch].step[j].stepNo;
					myCh->misc.advStepNo = (unsigned long)j;
					i = 2;
				}
			} else if(myCh->signal[C_SIG_RESET] == P1) {
				myCh->signal[C_SIG_RESET] = P0;
				//kjg_180521 myCh->op.state = C_STANDBY;
				i = 1;
			} else if(myCh->signal[C_SIG_CALI] == P1) {
				myCh->signal[C_SIG_CALI] = P0;
				if(myPs->config.hwSpec < S_5V_200A) { //linear
				} else { //switching
					cV_Range_Select(bd, ch, 0);
					cI_Range_Select(bd, ch, 0);
				}
				i = 3;
			} else if(myCh->signal[C_SIG_PAUSE] == P1) {
				myCh->signal[C_SIG_PAUSE] = P0;
				myCh->signal[C_SIG_RANGE_SWITCH] = P0;
				myCh->signal[C_SIG_OUT_SWITCH] = P0;
				myCh->signal[C_SIG_I_RANGE] = P0;			//151105 oys w
			}

			if(i == 1) {
				myCh->op.phase = P6;
				myCh->misc.advCycle = 0;
				myCh->misc.advCycleStep = 0;
				myCh->misc.advStepNo = 0;
				myCh->misc.chamberStepNo = myCh->misc.advStepNo;
				myCh->misc.currentCycle = 0;
				myCh->misc.totalCycle = 0;
				type = myPs->testCond[bd][ch].step[1].type;

				if(myCh->ChAttribute.opType == P1) {
					myCh->signal[C_SIG_PARALLEL_SWITCH] = P1;
				}

				if(myPs->config.hwSpec < S_5V_200A) { //linear
					if(myData->bData[bd].cData[parallel_ch].misc
						.parallel_cycle_phase == P50) k = 0; //kjg_180529
					else k = 1;
				} else k = 0;

				if(k == 1) {
					if(type == STEP_REST) {
						if(myPs->testCond[bd][ch].step[1].endT != 0) {
							if(myPs->testCond[bd][ch].step[1].endT >= 6000) {
								myCh->signal[C_SIG_OUT_SWITCH] = P0;
								myCh->signal[C_SIG_RANGE_SWITCH] = P0;
							} else {
								myCh->misc.relayStartFlag = 0;
								type = myPs->testCond[bd][ch].step[2].type;
								cSemiSwitch_Start(bd, ch, type);
							}
						} else {
							myCh->signal[C_SIG_OUT_SWITCH] = P0;
							myCh->signal[C_SIG_RANGE_SWITCH] = P0;
						}
					} else if(type == STEP_USER_PATTERN) {
						cSemiSwitch_Start(bd, ch, type);
						myCh->misc.relayStartFlag = 3;
					} else if(type == STEP_OCV) { 
						type = myPs->testCond[bd][ch].step[2].type;
						cSemiSwitch_Start(bd, ch, type);
						myCh->misc.relayStartFlag = 0;
					} else if(type == STEP_SHORT) {
						myCh->misc.relayStartFlag = 0;
					} else {
						cSemiSwitch_Start(bd, ch, type);
						myCh->misc.relayStartFlag = 1;
					}
				}

				if(myData->mData.config.function[F_CHAMBER_TEMP_WAIT] == P1) {
					if(myPs->testCond[bd][ch].step[1].refTemp != 999000) {
						myCh->misc.relayStartFlag = 100; //chamber temp wait
					}
				}
			} else if(i == 2) {
				myCh->op.phase = P6;
				type = myPs->testCond[bd][ch].step[j].type;

				if(myData->bData[bd].cData[parallel_ch].misc
					.parallel_cycle_phase == P50) k = 0; //kjg_180529
				else k = 1;
				if(k == 1) cSemiSwitch_Start(bd, ch, type);

				if(myCh->ChAttribute.opType == P1) {
					myCh->signal[C_SIG_PARALLEL_SWITCH] = P1;
				}

				if(myPs->config.hwSpec < S_5V_200A) { //linear
					if(myData->bData[bd].cData[parallel_ch].misc
						.parallel_cycle_phase == P50) k = 0; //kjg_180529
					else k = 1;
				} else k = 0;

				if(k == 1) {
					if(type == STEP_REST) {
						if(myPs->testCond[bd][ch].step[j].endT != 0) {
							if(myPs->testCond[bd][ch].step[j].endT >= 6000) {
								myCh->signal[C_SIG_OUT_SWITCH] = P0;
								myCh->signal[C_SIG_RANGE_SWITCH] = P0;
							} else {
								myCh->misc.relayStartFlag = 0;
							}
						} else {
							myCh->signal[C_SIG_OUT_SWITCH] = P0;
							myCh->signal[C_SIG_RANGE_SWITCH] = P0;
						}
					} else if(type == STEP_USER_PATTERN) {
						cSemiSwitch_Start(bd, ch, type);
						myCh->misc.relayStartFlag = 3;
					} else {
						cSemiSwitch_Start(bd, ch, type);
						myCh->misc.relayStartFlag = 2;
					}
				}

				if(myData->mData.config.function[F_CHAMBER_TEMP_WAIT] == P1) {
					refTemp = myPs->testCond[bd][ch].step[j].refTemp;
					groupTemp = myCh->misc.groupTemp;
					//20171212 sch add
					refTemp_backup = myCh->misc.refTemp_backup;

					switch(type) {
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
							if(refTemp != 999000) {
								if(refTemp == groupTemp
									|| refTemp == refTemp_backup) {
									myCh->misc.tempDir = P0;
								} else if(refTemp > groupTemp) {
									myCh->misc.tempDir = P1;
								} else {
									myCh->misc.tempDir = P2;
								}

								//chamber temp wait
								myCh->misc.relayStartFlag = 100;
							}
							break;
						default:
							myCh->misc.tempDir = P0;
							break;
					}
				}

				j = (int)myPs->testCond[bd][ch].reserved.select_cycleNo;
				if(myCh->op.type == STEP_ADV_CYCLE) j -= 1;	//190705
				if(j < 0) j = 0;
				myCh->misc.advCycle = (unsigned long)j;
				myCh->misc.currentCycle = (unsigned long)j;
				myCh->misc.totalCycle = (unsigned long)j;
				myCh->misc.advCycleStep
					= myPs->testCond[bd][ch].reserved.select_advCycleStep;
			} else if(i == 3) {
				myCh->op.phase = P8;
			}

			myCh->ccv[0].avg_v = myCh->misc.tmpVsens;
			if(myCh->ChAttribute.opType == P0) {
				//kjgw_180521 [ch-1]?
				myCh->ccv[0].avg_v = myData->bData[bd].cData[ch-1].ccv[0].avg_v;
			}
			myCh->ccv[0].avg_i = myCh->misc.tmpIsens;
			break;
		case P6:
			startStepNo = 1;
			if(myCh->misc.relayStartFlag == 2) {
				startStepNo = (unsigned long)myPs->testCond[bd][ch]
					.reserved.select_stepNo - 1;
			}

			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
			delay_time = 200;

			if(myData->bData[bd].cData[parallel_ch].misc
				.parallel_cycle_phase == P50) k = 0; //kjg_180529
			else k = 1;

			if(myCh->misc.relayStartFlag == 0){
				myCh->misc.relayStartFlag = 0;
			}else{
				if(myCh->op.checkDelayTime >= 50){ 
					myCh->signal[C_SIG_V_RANGE]
						= myPs->testCond[bd][ch].step[startStepNo]
							.rangeV + 1;
					myCh->signal[C_SIG_I_RANGE]
						= myPs->testCond[bd][ch].step[startStepNo]
							.rangeI + 1;
					myCh->signal[C_SIG_RANGE_SWITCH] = P1;
				}
				if(myCh->op.checkDelayTime >= 100){
					if(myCh->misc.relayStartFlag > 0){
						if(myCh->misc.relayStartFlag < 3) { //111008 kji
							myCh->signal[C_SIG_OUT_SWITCH] = P1;
						}
						myCh->misc.relayStartFlag = 0;
					}
				}
			}

			if(myCh->op.checkDelayTime >= delay_time) {
				myCh->op.state = C_RUN;
				myCh->op.phase = P0;
				myCh->op.checkDelayTime = 0;
			}
			break;
		case P8:
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
			delay_time = 200;
			if(myCh->op.checkDelayTime >= delay_time) {
				myCh->op.state = C_CALI;
				myCh->op.phase = P0;
				myCh->op.checkDelayTime = 0;
			}
			break;
		case P9:
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
			if(myCh->op.checkDelayTime == 10){
				cSemiSwitch_CAN(bd, ch);
			}
			if(myCh->op.checkDelayTime == 20) {
				myCh->signal[C_SIG_RANGE_SWITCH] = P0;
			}
			if(myCh->op.checkDelayTime >= 30) {
				myCh->signal[C_SIG_OUT_SWITCH] = P0;
				myCh->op.checkDelayTime = 0;
				myCh->op.phase = P3;
			}
			break;
		default:
			break;
	}
}

void cPause_CAN(int bd, int ch)
{
	unsigned char flag1 = 0, flag2 = 0, Inv_rtn = 0;
	int parallel_ch, mode, sum_ch, pauseSaveDt, pauseSaveEndT = 0;
   //	int inv, chInInv;
	long type, j;
	unsigned long advStepNo, saveDt, i=0;
	long long flag3;
	S_CH_STEP_INFO step;

	flag3 = 0x01;
	for(j=0; j < myPs->config.chPerBd; j++) {
	    if(j == ch) break;
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
	mode = step.mode;
	
	if((ch % 2) == 0) parallel_ch = ch + 1;
	else parallel_ch = ch - 1;

	switch(myCh->op.phase) {
		case P0:
			myCh->misc.preMode = 0;
			myCh->op.checkDelayTime = 0;
			myCh->misc.can_inv_errflag = 0;
			myCh->op.phase = P1;
			break;
		case P1:
			if(myData->DataSave.config.save_data_type == P1) {
				myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
				if(myCh->op.checkDelayTime == 100) {
					//200317 lyhw for 10mS End Data Save
					if(myCh->misc.reserved_cmd_flag == 0){
						if(myCh->ChAttribute.opType == P0) {
							send_save_msg(bd, ch, saveDt, 1);
						}
					}else{
						myCh->misc.reserved_cmd_flag = 0;
					}
				}
				if(myCh->op.checkDelayTime >= 200) {
					myCh->op.checkDelayTime = 0;
					myCh->op.phase = P2;
				}
			} else {
				myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
				if(myCh->op.checkDelayTime == 10) {
					cSemiSwitch_CAN(bd, ch);	//210105
				}
				if(myCh->op.checkDelayTime == 20) {
					myCh->signal[C_SIG_RANGE_SWITCH] = P0;
				}
				if(myCh->op.checkDelayTime == 30) {
					myCh->signal[C_SIG_OUT_SWITCH] = P0;
				}
				if(myCh->op.checkDelayTime >= 50){
					myCh->op.checkDelayTime = 0;
					myCh->op.phase = P2;
				}
			}
			break;
		case P2:
			if(myPs->config.parallelMode == P2) { //kjg_180528
				if(myData->bData[bd].cData[parallel_ch].misc
					.parallel_cycle_phase == P50) {
				} else {
					sens_ch_ad_count_increment(bd, ch);
				}
			} else {
				sens_ch_ad_count_increment(bd, ch);
			}

			sum_ch = 0;
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
								//111215 kji w
								if(myCh->ChAttribute.opType == P1) {
								    myData->bData[bd].cData[ch+1].
								        misc.errCnt[C_CNT_UPPER_TEMP] = 0;
								    myData->bData[bd].cData[ch+1].
								        signal[C_SIG_CONTINUE] = P1;
								}
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
								//111215 kji w
								if(myCh->ChAttribute.opType == P1) {
								    myData->bData[bd].cData[ch+1].
								    	misc.errCnt[C_CNT_UPPER_TEMP] = 0;
								    myData->bData[bd].cData[ch+1].
										signal[C_SIG_CONTINUE] = P1;
								}
								myCh->misc.errCnt[C_CNT_LOWER_TEMP] = 0;
								myCh->signal[C_SIG_CONTINUE] = P1;
							}
						}
					}
				}
			}

			//210114 add / 190901 lyhw	
			if(myData->mData.config.function[F_PAUSE_DATA_SAVE] == P1){
				pauseSaveDt = myData->DataSave.config.pause_data_time;
				if(pauseSaveDt != 0 
					&& myCh->op.code != C_FAULT_PAUSE_CMD
					&& myCh->op.code != C_PAUSE_UPPER_TEMP_CH
					&& myCh->op.code != C_PAUSE_LOWER_TEMP_CH){
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


			if(myCh->signal[C_SIG_STOP] == P1) {
				myCh->misc.pauseRunTime = 0;
				myCh->signal[C_SIG_STOP] = P0;
				if(myCh->op.code == C_FAULT_PAUSE_CMD
					&& myCh->opSave.code != C_FAULT_PAUSE_CMD) {
				} else {
					myCh->op.select = SAVE_FLAG_SAVING_END;
				}
				//190318 add
				if(myData->mData.config.parallelMode == P2){
					cCycle_p_ch_check(bd, ch);
				}
				myCh->op.code = C_FAULT_STOP_CMD;				
				send_save_msg(bd, ch, saveDt, 0);
				myCh->op.state = C_STANDBY;
				myCh->op.phase = P0;
				
			} else if(myCh->signal[C_SIG_PAUSE] == P1) {
				myCh->misc.pauseRunTime = 0;
				myCh->signal[C_SIG_PAUSE] = P0;
			} else if(myCh->signal[C_SIG_CONTINUE] == P1) {
				myCh->op.checkDelayTime += myPs->misc.rt_scan_time;	
				Inv_rtn = INV_StateCheck();		//220113 lyhw
				if(Inv_rtn != P1 && myCh->op.checkDelayTime <= 6000) return;
				myCh->op.checkDelayTime = 0;
			
				//211209 hun	
				myCh->misc.pauseRunTime = 0;
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

				myCh->misc.std_gasVoltage = myCh->misc.gasVoltage;	//220322_hun
				myCh->misc.pre_v_chk_time = myCh->op.runTime;	//kjc_200426
				myCh->misc.pre_chk_v = myCh->op.Vsens;			//kjc_200426
				#ifdef _SDI_SAFETY_V1	
				myCh->misc.fault_deltaV = myCh->op.Vsens;		//hun_200430
				#endif
				#ifdef _SDI_SAFETY_V2	
				myCh->misc.Master_recipe_deltaV = 0;		//hun_200430
				#endif
				myCh->op.changeV_Dt_cnt = 0;	//hun_200430
				//hun_211125
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
				//210303 need to check
				//return if bad
				/*
				if(myData->CAN.config.canPort[INV_CAN_PORT] != 0) {
					sum_ch = ch + myPs->config.chPerBd * bd;
					chInInv = 0;
					for(inv= 0; inv < myData->CAN.config.installedInverter; inv++){
						chInInv += myData->CAN.config.chInInv[inv];
						if(inv == 0){
							if(myPs->signal[M_SIG_INV_POWER_CAN] == P0){
								myData->CAN.inverter[inv].code = INV_CODE_NONE;
								myPs->signal[M_SIG_INV_POWER_CAN] = P1;
								myCh->misc.restart_inv = inv;
							}
						}else{	
							if(sum_ch >= (chInInv - myData->CAN.config.chInInv[inv])
								&&(sum_ch < chInInv)){
								if(myPs->signal[M_SIG_INV_POWER_CAN] == P0){
									myData->CAN.inverter[inv].code = INV_CODE_NONE;
									myPs->signal[M_SIG_INV_POWER_CAN] = P1;
									myCh->misc.restart_inv = inv;
								}
							}
						}
						
						inv = myCh->misc.restart_inv;
						if(myData->CAN.inverter[inv].code != INV_CODE_NONE){
							myCh->signal[C_SIG_CONTINUE] = P0;
							return;
						}
						if(myPs->signal[M_SIG_INV_POWER_CAN] != P10) return;
			
					}
				}*/

				myCh->signal[C_SIG_CONTINUE] = P0;
				if(myCh->misc.parallel_cycle_phase == P2
					|| myCh->misc.parallel_cycle_phase == P12) { //kjg_180530
				} else {
					cSemiSwitch_Start(bd,ch,type);
				}

				if(myCh->op.code == C_FAULT_PAUSE_CMD
					&& myCh->opSave.code != C_FAULT_PAUSE_CMD) {
					if(myCh->misc.parallel_cycle_phase == P2
						|| myCh->misc.parallel_cycle_phase == P12) {
						//kjg_180530
					} else {
					}
	       			myCh->op.checkDelayTime = 0;
					myCh->op.phase = P6;
				} else if(myCh->op.code == C_FAULT_USER_PATTERN_READ) {
					if(myCh->misc.parallel_cycle_phase == P2
						|| myCh->misc.parallel_cycle_phase == P12) {
						//kjg_180530
					} else {
						if(myCh->op.type == STEP_REST) {
							myCh->signal[C_SIG_OUT_SWITCH] = P0;
							if(myCh->ChAttribute.opType == P1) {
								myCh->signal[C_SIG_PARALLEL_SWITCH] = P1;
							}
						}
					}

	       			myCh->op.checkDelayTime = 0;
					myCh->op.phase = P3;
				} else {
					if(myCh->misc.parallel_cycle_phase == P2
						|| myCh->misc.parallel_cycle_phase == P12) {
						//kjg_180530
					} else {
						if(myCh->op.type == STEP_REST
							|| myCh->op.type == STEP_SHORT) {
							myCh->signal[C_SIG_OUT_SWITCH] = P0;
							if(myCh->ChAttribute.opType == P1) {
								myCh->signal[C_SIG_PARALLEL_SWITCH] = P1;
							}
						}
					}

	       			myCh->op.checkDelayTime = 0;
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
					myCh->misc.feedback_start = P0;
					myCh->op.phase = P4;
				}

				if(myCh->misc.parallel_cycle_phase == P2
					|| myCh->misc.parallel_cycle_phase == P12) {
					//kjg_180530
				} else {
					if(myCh->op.type == STEP_USER_PATTERN) {
						if(myCh->op.code != C_FAULT_USER_PATTERN_READ) {
							cSemiSwitch_Pattern(bd, ch, 0);
							i = myCh->misc.userPatternCnt;
							if(myData->mData.testCond[bd][ch].userPattern
								.data[i].data > 0) {
								myCh->misc.cmdV_dir = CMD_V_PLUS;
							} else {
								myCh->misc.cmdV_dir = CMD_V_MINUS;
							}
						}
					}
				}

				myCh->ccv[0].avg_v = myCh->misc.tmpVsens;
				myCh->ccv[0].avg_i = myCh->misc.tmpIsens;
			} else if(myCh->signal[C_SIG_NEXTSTEP] == P1) {
				myCh->op.checkDelayTime += myPs->misc.rt_scan_time;	
				Inv_rtn = INV_StateCheck();		//220113 lyhw
				if(Inv_rtn != P1 && myCh->op.checkDelayTime <= 6000) return;
				
				myCh->op.checkDelayTime = 0;
				myCh->misc.pauseRunTime = 0;
				myCh->misc.canNextStepFlag = P1;
				myCh->signal[C_SIG_NEXTSTEP] = P0;
				//kjg_180530
				if(myCh->misc.parallel_cycle_phase == P2 //kjg_180524
					|| myCh->misc.parallel_cycle_phase == P12) break; 

				myCh->op.checkDelayTime = 0;
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

				//141030 oys add : loop step -> next step cmd process	
				if(type == STEP_LOOP) {
					myCh->op.phase = P4;
				} else {
					myCh->op.phase = P5;
				}
			}
			break;
		case P3:
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
			if(myCh->op.checkDelayTime >= 100) {
				myCh->op.checkDelayTime = 0;
				myCh->op.state = myCh->misc.tmpState;
				myCh->op.code = myCh->misc.tmpCode;
				if(myCh->misc.parallel_cycle_phase == P2 //kjg_180524
					|| myCh->misc.parallel_cycle_phase == P12) { //kjg_180530
					myCh->op.phase = P5;
				} else {
					myCh->op.phase = P0;
				}
			}
			break;
		case P4:
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
			if(myCh->op.type != STEP_REST
				&& myCh->op.type != STEP_USER_PATTERN){
				if(myCh->op.checkDelayTime == 10) {
					cSemiSwitch_CAN(bd, ch);
				}
				if(myCh->op.checkDelayTime == 20) {
					myCh->signal[C_SIG_RANGE_SWITCH] = P1;
				}
				if(myCh->op.checkDelayTime == 30) {
					myCh->signal[C_SIG_OUT_SWITCH] = P1;
				}
			}
			
			if(myCh->op.checkDelayTime >= 200) {
				myCh->op.checkDelayTime = 0;
				myCh->op.state = myCh->misc.tmpState;
				myCh->op.code = myCh->misc.tmpCode;
				if(myCh->op.type == STEP_USER_PATTERN) {
					cSemiSwitch_Pattern(bd, ch, 1);
				}
				if(myCh->misc.waitFlag == P1) //pause waitFlag
				{
					myCh->misc.waitFlag = P0;
					myCh->op.phase = P0;
				}
				if(myCh->misc.chamberWaitFlag == -1) {
					myCh->misc.chamberWaitFlag = P0;
					myCh->op.phase = P0;
				} else {
					if(myCh->misc.parallel_cycle_phase == P2 //kjg_180524
						|| myCh->misc.parallel_cycle_phase == P12) {//kjg_180530
						myCh->op.phase = P5;
					} else {
						myCh->op.phase = P10;
					}
				}
			}
			break;
		case P5:
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
			if(myCh->op.checkDelayTime >= 200) {
				//170728 oys add
				myCh->misc.gradeProcFlag = P0;
				myCh->op.runTime = 0;
				//end add
				myCh->op.checkDelayTime = 0;
				myCh->op.state = C_RUN;
				myCh->op.phase = P0;
				myCh->misc.advStepNo++;
				advStepNo = myCh->misc.advStepNo;
				myCh->op.type = myPs->testCond[bd][ch].step[advStepNo].type;
				myCh->op.mode = myPs->testCond[bd][ch].step[advStepNo].mode;
				myCh->op.stepNo = myPs->testCond[bd][ch].step[advStepNo].stepNo;
			}
			break;
		case P6:
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
			if(myCh->op.checkDelayTime >= 200) {
				myCh->op.state = myCh->misc.tmpState;
				myCh->op.code = myCh->misc.tmpCode;
				myCh->misc.advStepNo++;
				advStepNo = myCh->misc.advStepNo;
				myCh->op.type = myPs->testCond[bd][ch].step[advStepNo].type;
				myCh->op.mode = myPs->testCond[bd][ch].step[advStepNo].mode;
				myCh->op.stepNo
					= myPs->testCond[bd][ch].step[advStepNo].stepNo;
				//myCh->signal[C_SIG_TRIGGER] = P1;
				myCh->op.checkDelayTime = 0;
				myCh->op.phase = P0;
			}
			break;
		case P7:
			break;
		default: break;
	}
}

void cStepCharge_CAN(int bd, int ch)
{
    int	rtn, rangeI, rangeV, div, parallel_ch, mode;
    long val1, val2, val3, refV, type;
	unsigned long advStepNo, saveDt;
	double tmp1;
	S_CH_STEP_INFO step;

	myCh = &(myData->bData[bd].cData[ch]);

	step = step_info(bd, ch);

	tmp1 = 0.0;
	div = 5;
	type = step.type;
	val1 = step.refV;
	val2 = step.refI;
	val3 = step.refP;
	rangeI = (int)step.rangeI;
	rangeV = (int)step.rangeV;
	saveDt = step.saveDt;
	advStepNo = step.advStepNo;
	mode = step.mode;

	refV = myPs->config.maxVoltage[rangeV] * 1.8;
	refV = val1;	//110428 kji

	switch(myPs->config.hwSpec) {
		case C_5V_CYCLER_CAN:
			refV = myPs->config.maxVoltage[rangeV];
			break;
		default:
			refV = val1;
			if(myData->mData.config.rt_scan_type > 0) {
				refV = myPs->config.maxVoltage[rangeV] * 1.8;
			}
			break;
	}
	
	//kjg_180521
	if((ch % 2) == 0) parallel_ch = ch + 1;
	else parallel_ch = ch - 1;

    switch(myCh->op.phase) {
		case P0:
			initCh(bd, ch);

			rtn = temp_wait_flag_check(bd, ch);
			if(rtn == 0) {
				cSemiSwitch_Start(bd, ch, type);
				break;
			}
			myCh->misc.step_count++;
			myCh->misc.cycleStepCount++;
			// 111212 oys w : cycleNo
			myCh->misc.cycleNo = myPs->testCond[bd][ch].step[advStepNo].cycleNo;

		   	myCh->op.checkDelayTime = 0;
			myCh->misc.startV = myCh->op.Vsens;
			myCh->misc.maxV = myCh->op.Vsens;
			myCh->misc.minV = myCh->op.Vsens;
			myCh->misc.Pre_change_V = myCh->op.Vsens;	//20190607
			myCh->misc.pre_chk_v = myCh->op.Vsens;		//20190829
			#ifdef _SDI_SAFETY_V1	
			myCh->misc.fault_deltaV = myCh->op.Vsens;	//hun_200430
			#endif
			myCh->misc.startT = myCh->op.temp;
			myCh->misc.maxT = myCh->op.temp;
			myCh->misc.minT = myCh->op.temp;
			myCh->misc.limit_current_timeout = myCh->op.runTime;
			myCh->misc.deltaV_timeout = myCh->op.runTime;
			myCh->misc.deltaI_timeout = myCh->op.runTime;
			//120315 kji 0 sec data option
			//170501 oys modify
			if(myData->DataSave.config.zero_sec_data_save == P1) {
				if(myCh->ChAttribute.opType == P0) {
					if(myCh->ChAttribute.chNo_master == 0) {
						myData->bData[bd].cData[ch-1].op.Isens = 0;
						myData->bData[bd].cData[ch-1].misc.tmpIsens = 0;
						myData->bData[bd].cData[ch-1].op.select
							= SAVE_FLAG_SAVING_TIME;
					}
					myCh->op.Isens = 0;
					myCh->misc.tmpIsens = 0;
					myCh->op.select = SAVE_FLAG_SAVING_TIME;
					send_save_msg(bd, ch, 0, 0);
				}
			}

			rtn = SelectHwSpec_CAN(bd, ch);
			myCh->misc.preMode = mode;	//210128
			if(rtn == 1) {
				ref_output_CAN(bd, ch, val1, val2, div, rangeV, rangeI, mode);
				if(myCh->ChAttribute.opType == P1) {
					ref_output_CAN(bd, ch+1, val1, val2, div, rangeV, rangeI, mode);
					myCh = &(myData->bData[bd].cData[ch]);
				}
				myCh->op.phase = P50;
			} else if(rtn == 2) {
				myCh->misc.cmd_v = refV;
				myCh->misc.cmd_v_div = (short int)div;
				myCh->misc.cmd_v_range = (short int)rangeV;
				myCh->misc.cmd_i = val2;
				myCh->misc.cmd_i_div = (short int)div;
				myCh->misc.cmd_i_range = (short int)rangeI;
				cSemiSwitch_Rest(bd, ch, 0, 0);
				myCh->signal[C_SIG_OUT_SWITCH] = P0;
				myCh->op.phase = P4;
			} else if(rtn == 3) {
				//210323 lyhw
				myCh->signal[C_SIG_OUT_SWITCH] = P0;
				myCh->op.phase = P3;
			} else if(rtn == 4) {
				cSemiSwitch(bd, ch);
				if(myCh->misc.parallel_cycle_phase == P50) { //kjg_180521
					cSemiSwitch_p(bd, ch, parallel_ch);
				}
				myCh->op.phase = P50;
			} else if(rtn == 5) {
				if(myCh->misc.start != 1) cSemiSwitch_Charge(bd, ch, 0);
				myCh->op.phase = P2;
			} else if(rtn == 6) { //100mS
				if(myCh->misc.start != 1) cSemiSwitch_Charge(bd, ch, 0);
				myCh->op.phase = P1;
			} else if(rtn == 7) { //switching same range
				myCh->misc.cmd_v = refV;
				myCh->misc.cmd_v_div = (short int)div;
				myCh->misc.cmd_v_range = (short int)rangeV;
				myCh->misc.cmd_i = val2;
				myCh->misc.cmd_i_div = (short int)div;
				myCh->misc.cmd_i_range = (short int)rangeI;
				cSemiSwitch_Rest(bd, ch, 0, 0);
				myCh->signal[C_SIG_OUT_SWITCH] = P0;
				myCh->op.phase = P6;
			}else if (rtn == 8){
				myCh->op.phase = P5;
			}
			break;
		case P1: //100mS
			myCh->op.checkDelayTime = 0;
			myCh->misc.sensCount = 0;
			myCh->misc.sensCountFlag = P0;
			myCh->misc.sensBufCount = 0;
			myCh->misc.sensBufCountFlag = P0;
			ref_output_CAN(bd, ch, val1, val2, div, rangeV, rangeI, mode);
			if(myCh->ChAttribute.opType == P1) {
				ref_output_CAN(bd,ch+1,val1,val2,div,rangeV,rangeI,mode);
				myCh = &(myData->bData[bd].cData[ch]);
			}
			myCh->op.phase = P50;
			break;
		case P2:
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
			if(myCh->op.checkDelayTime == 10) {
				if(myCh->misc.start != 1) cSemiSwitch_Charge(bd, ch, 1);
			}
			if(myCh->op.checkDelayTime >= 50) {
				myCh->op.checkDelayTime = 0;
				myCh->op.phase = P1;
			}
			break;
		case P3:	//210104 lyhw
			if(myCh->op.checkDelayTime == 0) cSemiSwitch_Charge(bd, ch, 0);
			if(myCh->op.checkDelayTime >= 10) {
				myCh->signal[C_SIG_RANGE_SWITCH] = P1;
			}
			if(myCh->op.checkDelayTime >= 20){
				myCh->signal[C_SIG_OUT_SWITCH] = P1;
			}

			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
			if(myCh->op.checkDelayTime >= 50){
				myCh->op.checkDelayTime = 0;
				myCh->op.phase = P1;
			}
			break;
		case P4:
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
			if(myCh->op.checkDelayTime == 10) {
				cSemiSwitch(bd, ch);
				if(myCh->misc.parallel_cycle_phase == P50) { //kjg_180521
					cSemiSwitch_p(bd, ch, parallel_ch);
				}
			} else if(myCh->op.checkDelayTime > 27) {
				myCh->op.checkDelayTime = 0;
				myCh->signal[C_SIG_OUT_SWITCH] = P1;
				myCh->signal[C_SIG_V_CMD_OUTPUT] = P1;
				myCh->op.phase = P5;
			}
			break;
		case P5:
			//210323 lyhw
			myCh->signal[C_SIG_OUT_SWITCH] = P0;
			myCh->op.checkDelayTime = 0;
			myCh->op.phase = P3;
			break;
		case P6:
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
			if(myCh->op.checkDelayTime <= 2) {
				cSemiSwitch(bd, ch);
				if(myCh->misc.parallel_cycle_phase == P50) { //kjg_180521
					cSemiSwitch_p(bd, ch, parallel_ch);
				}
				myCh->signal[C_SIG_V_CMD_OUTPUT] = P1;
				myCh->signal[C_SIG_OUT_SWITCH] = P1;
			} else if(myCh->op.checkDelayTime >= 4) {
				myCh->op.checkDelayTime = 0;
				myCh->signal[C_SIG_I_CMD_OUTPUT] = P1;
				myCh->op.phase = P50;
			} 
			break;
		case P10:
			//SKI_hun_201010
			if(myCh->misc.ac_fail_flag == P1){
				myCh->op.phase = P15;
			}else if(myCh->misc.ac_fail_flag == P0){			
				rtn = SelectHwSpec_CAN(bd , ch);
				if(rtn == 1 || rtn == 3 || rtn == 4 || rtn == 5 || rtn == 6) {
					myCh->misc.fbV = 0;
					myCh->misc.fbI = 0;
					myCh->misc.ocv = myCh->op.Vsens;
					myCh->misc.pid_ui1[0] = 0.0;
					myCh->misc.pid_ui1[1] = 0.0;
					myCh->misc.pid_error1[0] = 0.0;
					myCh->misc.pid_error1[1] = 0.0;
					myCh->signal[C_SIG_V_RANGE] = myCh->op.rangeV + 1;
					myCh->signal[C_SIG_I_RANGE] = myCh->op.rangeI + 1;

					myCh->op.phase = P11;
				} else if(rtn == 2 || rtn == 7) {
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
					myCh->signal[C_SIG_V_RANGE] = rangeV + 1;
					myCh->signal[C_SIG_I_RANGE] = rangeI + 1;
					cSemiSwitch_Rest(bd, ch, 0, 0);
					myCh->op.phase = P13;
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
				ref_output_CAN(bd, ch, val1, val2, div, rangeV, rangeI, mode);
				if(myCh->misc.parallel_cycle_phase == P50) { //kjg_180521
					ref_output_p(bd, ch, parallel_ch, val1, val2, div,
						rangeV, rangeI, mode);
				}
				if(myCh->ChAttribute.opType == P1) {
					ref_output_CAN(bd, ch + 1, val1, val2, div, rangeV, rangeI, mode);
					myCh = &(myData->bData[bd].cData[ch]);
				}
			} else {
				ref_output_CAN(bd, ch, val1, val2, div, rangeV, rangeI, mode);
				if(myCh->misc.parallel_cycle_phase == P50) { //kjg_180521
					ref_output_p(bd, ch, parallel_ch, val1, val2, div,
						rangeV, rangeI, mode);
				}
				if(myCh->ChAttribute.opType == P1) {
					ref_output_CAN(bd, ch + 1, val1, val2, div, rangeV, rangeI, mode);
					myCh = &(myData->bData[bd].cData[ch]);
				}
			}
		//	myCh->op.phase = P50;
			myCh->op.phase = P40;
			break;
		case P13:
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
			if(myCh->op.checkDelayTime == 10) {
				cSemiSwitch(bd, ch);
				if(myCh->misc.parallel_cycle_phase == P50) { //kjg_180521
					cSemiSwitch_p(bd, ch, parallel_ch);
				}
			} else if(myCh->op.checkDelayTime >= 30) {
				myCh->misc.sensCount = 0;
				myCh->misc.sensCountFlag = P0;
				myCh->misc.sensBufCount = 0;
				myCh->misc.sensBufCountFlag = P0;
		
				myCh->op.checkDelayTime = 0;
				myCh->signal[C_SIG_OUT_SWITCH] = P1;
				myCh->signal[C_SIG_V_CMD_OUTPUT] = P1;
				myCh->signal[C_SIG_I_CMD_OUTPUT] = P1;
				myCh->op.phase = P50;
			}
			break;
		case P14:
			cSemiSwitch_Charge(bd, ch, 4);
			myCh->op.phase = P11;
			break;
		//SKI_hun_201010
		case P15:
			myCh->misc.ac_fail_flag = P0;
			rtn = temp_wait_flag_check(bd, ch);
			if(rtn == 0) {
				cSemiSwitch_Start(bd, ch, type);
				break;
			}
			myCh->misc.saveDt = myCh->op.runTime;
			
			rtn = SelectHwSpec_CAN(bd , ch);
			if(rtn == 1 || rtn == 3 || rtn == 4 || rtn == 5 || rtn == 6) {
				myCh->misc.fbV = 0;
				myCh->misc.fbI = 0;
				myCh->misc.ocv = myCh->op.Vsens;
				myCh->misc.pid_ui1[0] = 0.0;
				myCh->misc.pid_ui1[1] = 0.0;
				myCh->misc.pid_error1[0] = 0.0;
				myCh->misc.pid_error1[1] = 0.0;
				myCh->signal[C_SIG_V_RANGE] = myCh->op.rangeV + 1;
				myCh->signal[C_SIG_I_RANGE] = myCh->op.rangeI + 1;
			
				myCh->op.phase = P11;
			} else if(rtn == 2 || rtn == 7) {
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
				myCh->signal[C_SIG_V_RANGE] = rangeV + 1;
				myCh->signal[C_SIG_I_RANGE] = rangeI + 1;
				cSemiSwitch_Rest(bd, ch, 0, 0);
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
		case P40:
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
			if(myData->mData.config.function[F_SEMI_SWITCH_TYPE] == P1
					|| myCh->ChAttribute.opType == 1
					|| myCh->ChAttribute.chNo_master == 0) {
				myCh->op.checkDelayTime = 0;			//200320 lyh
				myCh->op.phase = P50;
			}else{
				//201109_pty
				if(labs(val2) <= myPs->config.maxCurrent[rangeI] * 0.1){ 
					if(myCh->misc.cvFlag == 0) {
						if(labs(myCh->op.Isens) >= labs(val2*0.8)){
							myCh->op.checkDelayTime = 0;
							myCh->op.phase = P50;
						} else {
							if(myCh->op.checkDelayTime >= 1500) {
								myCh->op.checkDelayTime = 0;
								myCh->op.phase = P50;
							}
							break;
						}
					} else {
						myCh->op.checkDelayTime = 0;
						myCh->op.phase = P50;
					}
				}else{
					myCh->op.checkDelayTime = 0;
					myCh->op.phase = P50;
				}//end
			}
		case P50:
			if(myPs->config.SoftFeedbackFlag == P0){
				cStepCCCV_Check_CAN(bd, ch, val1, val2); //220204
			}
			myCh->misc.start = 0;
	    	myCh->op.runTime += myPs->misc.rt_scan_time;
			myCh->op.totalRunTime += myPs->misc.rt_scan_time;
			myCh->misc.cycleRunTime += myPs->misc.rt_scan_time;
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;

			sens_ch_ad_count_increment(bd, ch);

			if(myCh->op.runTime >= 30) {
				myCh->misc.meanCount++;
				if(myCh->misc.meanCount <= 8) {
					myCh->misc.meanSumVolt += (myCh->misc.tmpVsens / 100);
					myCh->misc.meanSumCurr += (myCh->misc.tmpIsens / 100);
					myCh->misc.meanSumTemp += (myCh->op.temp / 100);
				} else {
					myCh->misc.meanSumVolt += (myCh->op.Vsens / 100);
					myCh->misc.meanSumCurr += (myCh->op.Isens / 100);
					myCh->misc.meanSumTemp += (myCh->op.temp / 100);
				}
				myCh->op.meanVolt = (long)(myCh->misc.meanSumVolt
					/ myCh->misc.meanCount) * 100;
				myCh->op.meanCurr = (long)(myCh->misc.meanSumCurr
					/ myCh->misc.meanCount) * 100;
				myCh->op.meanTemp = (long)(myCh->misc.meanSumTemp
					/ myCh->misc.meanCount) * 100;
			}

			if(myCh->op.checkDelayTime == myPs->misc.rt_scan_time) {
				myCh->misc.cmd_v = val1;
				myCh->misc.cmd_v_div = (short int)div;
				myCh->misc.cmd_v_range = (short int)rangeV;
				myCh->signal[C_SIG_V_CMD_OUTPUT] = P1;
			}

			if(myCh->signal[C_SIG_STOP] == P1) {
				myCh->op.code = C_FAULT_STOP_CMD;
				myCh->op.select = SAVE_FLAG_SAVING_END;
				myCh->signal[C_SIG_STOP] = P0;
				cSemiSwitch_Rest(bd, ch, advStepNo, 0);
				myCh->op.phase = P100;
			} else if(myCh->signal[C_SIG_PAUSE] == P1) {
				//cmd, soft fault
				myCh->misc.tmpState = myCh->op.state;
				myCh->misc.tmpCode = myCh->op.code;
				myCh->op.code = C_FAULT_PAUSE_CMD;
				myCh->opSave = myCh->op;
				myCh->signal[C_SIG_PAUSE] = P0;
				cSemiSwitch_Rest(bd, ch, advStepNo, 0);
				myCh->op.phase = P100;
			} else {
				if(cFailCodeCheck(bd, ch) >= 0) {
					cSoftFeedback(bd, ch, val1, val2, val3);
				}

				if(myCh->misc.cvFlag == P1) {
					myCh->misc.cvTime += myPs->misc.rt_scan_time;
					myCh->misc.cycle_Charge_cvTime += myPs->misc.rt_scan_time;
				} else {
					myCh->misc.ccTime += myPs->misc.rt_scan_time;
					myCh->misc.cycle_Charge_ccTime += myPs->misc.rt_scan_time;
				}
			}

			//191010
			if(myCh->op.checkDelayTime < 100){
				myCh->misc.Pre_change_V = myCh->misc.tmpVsens;
				//hun_200409_s
				myCh->misc.maxV = myCh->misc.tmpVsens;
				myCh->misc.startV = myCh->misc.tmpVsens;
				myCh->misc.minV = myCh->misc.tmpVsens;
				myCh->misc.maxI = myCh->misc.tmpIsens;
				myCh->misc.minI = myCh->misc.tmpIsens;
				//hun_200409_e
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
				cNextStepCheck(bd, ch);
			}
			break;
		case P100:
			break;
		default: break;
    }
}

void cStepDischarge_CAN(int bd, int ch)
{
	int rtn, rangeV, rangeI, div = 5, parallel_ch, mode;
    long val1, val2, val3, refV, refI,type;
	unsigned long advStepNo, saveDt;
	double tmp1;
	S_CH_STEP_INFO step;

	myCh = &(myData->bData[bd].cData[ch]);

	tmp1 = 0.0;
	
	step = step_info(bd, ch);

	type = step.type;
	val1 = step.refV;
	val2 = step.refI;
	val3 = step.refP;
	rangeI = (int)step.rangeI;
	rangeV = (int)step.rangeV;
	saveDt = step.saveDt;
	advStepNo = step.advStepNo;
	mode = step.mode;

	refV = myPs->config.maxVoltage[rangeV]*(-1.8);
	refI = -180000;
	refV = val1;	//110428 kji

	switch(myPs->config.hwSpec) {
		case C_5V_CYCLER_CAN:
			refV = myPs->config.maxVoltage[rangeV]*0.2;
			break;
		default:
			refV = val1;
			if(myData->mData.config.rt_scan_type > 0) {
				refV = myPs->config.maxVoltage[rangeV] * (-1.8);
			}
			break;
	}

	//kjg_180521
	if((ch % 2) == 0) parallel_ch = ch + 1;
	else parallel_ch = ch - 1;

    switch(myCh->op.phase) {
		case P0:
			initCh(bd, ch);

			rtn = temp_wait_flag_check(bd, ch);
			if(rtn == 0) {
				cSemiSwitch_Start(bd, ch, type);
				break;
			}
			myCh->misc.step_count++;
			myCh->misc.cycleStepCount++;
			myCh->misc.cycleDischargeStepCount++; //kjgw_180530
			// 111212 oys w : cycleNo
			myCh->misc.cycleNo = myPs->testCond[bd][ch].step[advStepNo].cycleNo;

		   	myCh->op.checkDelayTime = 0;
			myCh->misc.startV = myCh->op.Vsens;
			myCh->misc.maxV = myCh->op.Vsens;
			myCh->misc.minV = myCh->op.Vsens;
			myCh->misc.pre_chk_v = myCh->op.Vsens;		//20190829
			#ifdef _SDI_SAFETY_V1	
			myCh->misc.fault_deltaV = myCh->op.Vsens;	//hun_200430
			#endif
			#ifdef _SDI_SAFETY_V2	
			myCh->misc.Master_recipe_deltaV = myCh->op.Vsens;	//hun_200430
			#endif
			myCh->misc.startT = myCh->op.temp;
			myCh->misc.maxT = myCh->op.temp;
			myCh->misc.minT = myCh->op.temp;
			myCh->misc.limit_current_timeout = myCh->op.runTime;
			myCh->misc.deltaV_timeout = myCh->op.runTime;
			myCh->misc.deltaI_timeout = myCh->op.runTime;
			//120315 kji 0 sec data option

			//170501 oys modify
			if(myData->DataSave.config.zero_sec_data_save == P1) {
				if(myCh->ChAttribute.opType == P0) {
					if(myCh->ChAttribute.chNo_master == 0) {
						myData->bData[bd].cData[ch-1].op.Isens = 0;
						myData->bData[bd].cData[ch-1].misc.tmpIsens = 0;
						myData->bData[bd].cData[ch-1].op.select
							= SAVE_FLAG_SAVING_TIME;
					}
					myCh->op.Isens = 0;
					myCh->misc.tmpIsens = 0;
					myCh->op.select = SAVE_FLAG_SAVING_TIME;
					send_save_msg(bd, ch, 0, 0);
				}
			}

			rtn = SelectHwSpec_CAN(bd , ch);
			myCh->misc.preMode = mode;	//210128
			if(rtn == 1) {
				ref_output_CAN(bd, ch, val1, val2, div, rangeV, rangeI, mode);
				if(myCh->ChAttribute.opType == P1) {
					ref_output_CAN(bd,ch+1, val1, val2, div, rangeV, rangeI, mode);
					myCh = &(myData->bData[bd].cData[ch]);
				}
				myCh->op.phase = P50;
			} else if(rtn == 2) {
				myCh->misc.cmd_v = 0;
				myCh->misc.cmd_v_div = (short int)div;
				myCh->misc.cmd_v_range = (short int)rangeV;
				myCh->misc.cmd_i = val2;
				myCh->misc.cmd_i_div = (short int)div;
				myCh->misc.cmd_i_range = (short int)rangeI;
				myCh->signal[C_SIG_OUT_SWITCH] = P0;
				cSemiSwitch_Rest(bd, ch, 0, 0);
				myCh->op.phase = P4;
			} else if(rtn == 3) {	//210323 lyhw
				myCh->signal[C_SIG_OUT_SWITCH] = P0;
				myCh->op.phase = P3;
			} else if(rtn == 4) {
				cSemiSwitch(bd, ch);
				if(myCh->misc.parallel_cycle_phase == P50) { //kjg_180521
					cSemiSwitch_p(bd, ch, parallel_ch);
				}
				myCh->op.phase = P50;
			} else if(rtn == 5) {
				if(myCh->misc.start != 1)	cSemiSwitch_CAN(bd, ch);
				myCh->op.phase = P3;
			} else if(rtn == 6) {
				if(myCh->misc.start != 1)	cSemiSwitch_CAN(bd, ch);
				myCh->op.phase = P3;
			} else if(rtn == 7) { //switching same range
				myCh->misc.cmd_v = 0;
				myCh->misc.cmd_v_div = (short int)div;
				myCh->misc.cmd_v_range = (short int)rangeV;
				myCh->misc.cmd_i = val2;
				myCh->misc.cmd_i_div = (short int)div;
				myCh->misc.cmd_i_range = (short int)rangeI;
				myCh->signal[C_SIG_OUT_SWITCH] = P0;
				cSemiSwitch_Rest(bd, ch, 0, 0);
				myCh->op.phase = P6;
			}else if(rtn == 8){
				myCh->op.phase = P5;
			}
			break;
		case P1:
			myCh->op.checkDelayTime = 0;
			myCh->misc.sensCount = 0;
			myCh->misc.sensCountFlag = P0;
			myCh->misc.sensBufCount = 0;
			myCh->misc.sensBufCountFlag = P0;
			ref_output_CAN(bd, ch, val1, val2, div, rangeV, rangeI, mode);
			if(myCh->ChAttribute.opType == P1) {
				ref_output_CAN(bd,ch+1,val1,val2,div,rangeV,rangeI,mode);
				myCh = &(myData->bData[bd].cData[ch]);
			}
			myCh->op.phase = P50;
			break;
		case P2:
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
			if(myCh->op.checkDelayTime == 10) {
				if(myCh->misc.start != 1)	cSemiSwitch_CAN(bd, ch);
			}
			if(myCh->op.checkDelayTime >= 50) {
				if(myCh->misc.start != 1) 	cSemiSwitch_CAN(bd, ch);
				myCh->op.checkDelayTime = 0;
				myCh->op.phase = P1;
			}
			break;
		case P3:	//210104 lyhw
			if(myCh->op.checkDelayTime == 0) cSemiSwitch_CAN(bd, ch);
			if(myCh->op.checkDelayTime >= 10) {
				myCh->signal[C_SIG_RANGE_SWITCH] = P1;
			}
			if(myCh->op.checkDelayTime >= 20){
				myCh->signal[C_SIG_OUT_SWITCH] = P1;
			}

			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
			if(myCh->op.checkDelayTime >= 50){ 
				myCh->op.checkDelayTime = 0;
				myCh->op.phase = P1;
			}
			break;
		case P4:
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
			if(myCh->op.checkDelayTime == 10) {
				cSemiSwitch(bd, ch);
				if(myCh->misc.parallel_cycle_phase == P50) { //kjg_180521
					cSemiSwitch_p(bd, ch, parallel_ch);
				}
			}else if(myCh->op.checkDelayTime > 27) {
				myCh->op.checkDelayTime = 0;
				myCh->signal[C_SIG_OUT_SWITCH] = P1;
				myCh->signal[C_SIG_V_CMD_OUTPUT] = P1;
				myCh->op.phase = P5;
			}
			break;
		case P5:	
	//		cSemiSwitch_CAN(bd, ch);
			//210323 lyhw
			myCh->signal[C_SIG_OUT_SWITCH] = P0;
			myCh->op.checkDelayTime = 0;
			myCh->op.phase = P3;
			break;
		case P6:
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
			if(myCh->op.checkDelayTime <= 2) {
				cSemiSwitch(bd, ch);
				if(myCh->misc.parallel_cycle_phase == P50) { //kjg_180521
					cSemiSwitch_p(bd, ch, parallel_ch);
				}
			} else if(myCh->op.checkDelayTime >= 4) {
				myCh->op.checkDelayTime = 0;
				myCh->signal[C_SIG_OUT_SWITCH] = P1;
				myCh->signal[C_SIG_I_CMD_OUTPUT] = P1;
				myCh->signal[C_SIG_V_CMD_OUTPUT] = P1;
				myCh->op.phase = P50;
			} 
			break;
		case P10:
			//SKI_hun_201010
			if(myCh->misc.ac_fail_flag == P1){
				myCh->op.phase = P15;				
			}else if(myCh->misc.ac_fail_flag == P0){
				rtn = SelectHwSpec_CAN(bd , ch);
				if(rtn == 1
					|| rtn == 3
					|| rtn == 4
					|| rtn == 5
					|| rtn == 6) {
				//	cSemiSwitch_Discharge(bd, ch, 0);
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
				} else if(rtn == 2 || rtn == 7) {
					myCh->misc.fbV = 0;
					myCh->misc.fbI = 0;
					myCh->misc.ocv = myCh->op.Vsens;
					myCh->misc.pid_ui1[0] = 0.0;
					myCh->misc.pid_ui1[1] = 0.0;
					myCh->misc.pid_error1[0] = 0.0;
					myCh->misc.pid_error1[1] = 0.0;

					myCh->misc.cmd_v = 0;
					myCh->misc.cmd_v_div = (short int)div;
					myCh->misc.cmd_v_range = (short int)rangeV;
					myCh->misc.cmd_i = val2;
					myCh->misc.cmd_i_div = (short int)div;
					myCh->misc.cmd_i_range = (short int)rangeI;
					myCh->signal[C_SIG_V_RANGE] = rangeV+1;
					myCh->signal[C_SIG_I_RANGE] = rangeI+1;
					cSemiSwitch_Rest(bd,ch,0,0);
					myCh->op.phase = P13;
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
				ref_output_CAN(bd, ch, val1, val2, div, rangeV, rangeI, mode);
				if(myCh->misc.parallel_cycle_phase == P50) { //kjg_180521
					ref_output_p(bd, ch, parallel_ch, val1, val2, div,
						rangeV, rangeI, mode);
				}
				myCh->signal[C_SIG_IEC_START] = P1;	//171227 add dis
				if(myCh->ChAttribute.opType == P1)
				{
					ref_output_CAN(bd, ch+1, val1, val2, div, rangeV, rangeI, mode);
					myCh->signal[C_SIG_IEC_START] = P1;	//171227 add
					myCh = &(myData->bData[bd].cData[ch]);
				}
			} else {
				ref_output_CAN(bd, ch, val1, val2, div, rangeV, rangeI, mode);
				if(myCh->misc.parallel_cycle_phase == P50) { //kjg_180521
					ref_output_p(bd, ch, parallel_ch, val1, val2, div,
						rangeV, rangeI, mode);
				}
				if(myCh->ChAttribute.opType == P1)
				{
					ref_output_CAN(bd, ch+1, val1, val2, div, rangeV, rangeI, mode);
					myCh = &(myData->bData[bd].cData[ch]);
				}
			}
		//	myCh->op.phase = P50;
			myCh->op.phase = P40;
			break;
		case P13:
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
			if(myCh->op.checkDelayTime == 10) {
				cSemiSwitch(bd, ch);
				if(myCh->misc.parallel_cycle_phase == P50) { //kjg_180521
					cSemiSwitch_p(bd, ch, parallel_ch);
				}
			} else if(myCh->op.checkDelayTime >= 30) {
				myCh->misc.sensCount = 0;
				myCh->misc.sensCountFlag = P0;
				myCh->misc.sensBufCount = 0;
				myCh->misc.sensBufCountFlag = P0;
		
				myCh->op.checkDelayTime = 0;
				myCh->signal[C_SIG_OUT_SWITCH] = P1;
				myCh->signal[C_SIG_V_CMD_OUTPUT] = P1;
				myCh->signal[C_SIG_I_CMD_OUTPUT] = P1;
				myCh->op.phase = P50;
			}
			break;
		//SKI_hun_201010
		case P15:
			myCh->misc.ac_fail_flag = P0;
			rtn = temp_wait_flag_check(bd, ch);
			if(rtn == 0) {
				cSemiSwitch_Start(bd, ch, type);
				break;
			}
			myCh->misc.saveDt = myCh->op.runTime;
			
			rtn = SelectHwSpec_CAN(bd , ch);
			if(rtn == 1
				|| rtn == 3
				|| rtn == 4
				|| rtn == 5
				|| rtn == 6) {
			//	cSemiSwitch_Discharge(bd, ch, 0);
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
			} else if(rtn == 2 || rtn == 7) {
				myCh->misc.fbV = 0;
				myCh->misc.fbI = 0;
				myCh->misc.ocv = myCh->op.Vsens;
				myCh->misc.pid_ui1[0] = 0.0;
				myCh->misc.pid_ui1[1] = 0.0;
				myCh->misc.pid_error1[0] = 0.0;
				myCh->misc.pid_error1[1] = 0.0;

				myCh->misc.cmd_v = 0;
				myCh->misc.cmd_v_div = (short int)div;
				myCh->misc.cmd_v_range = (short int)rangeV;
				myCh->misc.cmd_i = val2;
				myCh->misc.cmd_i_div = (short int)div;
				myCh->misc.cmd_i_range = (short int)rangeI;
				myCh->signal[C_SIG_V_RANGE] = rangeV+1;
				myCh->signal[C_SIG_I_RANGE] = rangeI+1;
				cSemiSwitch_Rest(bd,ch,0,0);
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
		case P40:
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
			if(myData->mData.config.function[F_SEMI_SWITCH_TYPE] == P1
					|| myCh->ChAttribute.opType == 1 
					|| myCh->ChAttribute.chNo_master == 0) {
				myCh->op.checkDelayTime = 0;			//200320 lyhw
				myCh->op.phase = P50;
			}else{
				//201109_pth
				if(labs(val2) <= myPs->config.maxCurrent[rangeI] * 0.1){
					if(myCh->misc.cvFlag == 0) {
						if(labs(myCh->op.Isens) >= labs(val2*0.8)){
							myCh->op.checkDelayTime = 0;
							myCh->op.phase = P50;
						} else {
							if(myCh->op.checkDelayTime >= 1500) {
								myCh->op.checkDelayTime = 0;
								myCh->op.phase = P50;
							}
							break;
						}
					} else {
						myCh->op.checkDelayTime = 0;
						myCh->op.phase = P50;
					}
				}else{
					myCh->op.checkDelayTime = 0;
					myCh->op.phase = P50;
				}//end
			}
		case P50:
			if(myPs->config.SoftFeedbackFlag == P0){
				cStepCCCV_Check_CAN(bd, ch, val1, val2); //220204
			}
			myCh->misc.start = 0;
	    	myCh->op.runTime += myPs->misc.rt_scan_time;
			myCh->op.totalRunTime += myPs->misc.rt_scan_time;
			myCh->misc.cycleRunTime += myPs->misc.rt_scan_time;
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;

			sens_ch_ad_count_increment(bd, ch);

			if(myCh->op.runTime >= 30) {
				myCh->misc.meanCount++;
				if(myCh->misc.meanCount <= 8) {
					myCh->misc.meanSumVolt += (myCh->misc.tmpVsens / 100);
					myCh->misc.meanSumCurr += (myCh->misc.tmpIsens / 100);
					myCh->misc.meanSumTemp += (myCh->op.temp / 100);
				} else {
					myCh->misc.meanSumVolt += (myCh->op.Vsens / 100);
					myCh->misc.meanSumCurr += (myCh->op.Isens / 100);
					myCh->misc.meanSumTemp += (myCh->op.temp / 100);
				}
				myCh->op.meanVolt = (long)(myCh->misc.meanSumVolt
					/ myCh->misc.meanCount) * 100;
				myCh->op.meanCurr = (long)(myCh->misc.meanSumCurr
					/ myCh->misc.meanCount) * 100;
				myCh->op.meanTemp = (long)(myCh->misc.meanSumTemp
					/ myCh->misc.meanCount) * 100;
			}

			if(myCh->op.checkDelayTime == myPs->misc.rt_scan_time) {
				myCh->misc.cmd_v = val1;
				myCh->misc.cmd_v_div = (short int)div;
				myCh->misc.cmd_v_range = (short int)rangeV;
				myCh->signal[C_SIG_V_CMD_OUTPUT] = P1;
			}

			if(myCh->signal[C_SIG_STOP] == P1) {
				myCh->op.code = C_FAULT_STOP_CMD;
				myCh->op.select = SAVE_FLAG_SAVING_END;
				myCh->signal[C_SIG_STOP] = P0;
				cSemiSwitch_Rest(bd, ch, advStepNo, 0);
				myCh->op.phase = P100;
			} else if(myCh->signal[C_SIG_PAUSE] == P1) {
				//cmd, soft fault
				myCh->misc.tmpState = myCh->op.state;
				myCh->misc.tmpCode = myCh->op.code;
				myCh->op.code = C_FAULT_PAUSE_CMD;
				myCh->opSave = myCh->op;
				myCh->signal[C_SIG_PAUSE] = P0;
				cSemiSwitch_Rest(bd, ch, advStepNo, 0);
				myCh->op.phase = P100;
			} else {
				if(cFailCodeCheck(bd, ch) >= 0) {
					cSoftFeedback(bd, ch, val1, val2, val3);
				}

				if(myCh->misc.cvFlag == P1) {
					myCh->misc.cvTime += myPs->misc.rt_scan_time;
					myCh->misc.cycle_Discharge_cvTime
						+= myPs->misc.rt_scan_time;
				} else {
					myCh->misc.ccTime += myPs->misc.rt_scan_time;
					myCh->misc.cycle_Discharge_ccTime
						+= myPs->misc.rt_scan_time;
				}
			}

			//191010
			if(myCh->op.checkDelayTime < 100){
				myCh->misc.Pre_change_V = myCh->misc.tmpVsens;
				//hun_200409_s
				myCh->misc.maxV = myCh->misc.tmpVsens;
				myCh->misc.startV = myCh->misc.tmpVsens;
				myCh->misc.minV = myCh->misc.tmpVsens;
				myCh->misc.maxI = myCh->misc.tmpIsens;
				myCh->misc.minI = myCh->misc.tmpIsens;
				//hun_200409_e
			}
	    	if(myCh->op.checkDelayTime == 100) {
				myCh->misc.maxI = myCh->op.Isens;
				myCh->misc.minI = myCh->op.Isens;
			}

			if(myCh->ChAttribute.opType == P0) {
				cCalculate_Capacitance(bd, ch, advStepNo);
			//20180206 sch add
			#if EDLC_TYPE == 1
				cCalculate_Capacitance_IEC(bd, ch, advStepNo);
				cCalculate_Capacitance_Maxwell(bd, ch, advStepNo);
			#endif
				cCalculate_DCR(bd, ch, advStepNo);
			}

			myCh->ccv[0].avg_v = myCh->misc.tmpVsens;
			myCh->ccv[0].avg_i = myCh->misc.tmpIsens;

			if(myCh->op.phase == P100) {
				cNextStepCheck(bd, ch);
			}
			break;
		case P100:
			break;
		default: break;
    }
}

void cStepRest_CAN(int bd, int ch)
{
	int i, rtn = 0, rangeI, rangeV,type, mode;
	unsigned long advStepNo, saveDt;
	S_CH_STEP_INFO step;

	myCh = &(myData->bData[bd].cData[ch]);

	step = step_info(bd, ch);
	type = step.type;
    advStepNo = step.advStepNo;
	rangeV = step.rangeV;
	rangeI = step.rangeI;
	saveDt = step.saveDt;
	mode = step.mode;

    switch(myCh->op.phase) {
		case P0:
			initCh(bd, ch);	
			rtn = temp_wait_flag_check(bd, ch);
			if(rtn == 0) {
				//20180305 sch modify
				cSemiSwitch_Rest(bd, ch, advStepNo, 0);
				break;
			}
			myCh->misc.step_count++;
			myCh->misc.cycleStepCount++;
			// 111212 oys w : cycleNo
			myCh->misc.cycleNo = myPs->testCond[bd][ch].step[advStepNo].cycleNo;

			myCh->op.checkDelayTime = 0;
			myCh->misc.startV = myCh->op.Vsens;
			myCh->misc.maxV = myCh->op.Vsens;
			myCh->misc.minV = myCh->op.Vsens;
			myCh->misc.startT = myCh->op.temp;
			myCh->misc.maxT = myCh->op.temp;
			myCh->misc.minT = myCh->op.temp;
			//150126 lyh add for rest Delta V
			myCh->misc.compareV = myCh->op.Vsens;
			//120315 kji 0 sec data option
			//170501 oys modify
			if(myData->DataSave.config.zero_sec_data_save == P1) {
				if(myCh->ChAttribute.opType == P0) {
					if(myCh->ChAttribute.chNo_master == 0) {
						myData->bData[bd].cData[ch-1].op.Isens = 0;
						myData->bData[bd].cData[ch-1].misc.tmpIsens = 0;
						myData->bData[bd].cData[ch-1].op.select
							= SAVE_FLAG_SAVING_TIME;
					}
					myCh->op.Isens = 0;
					myCh->misc.tmpIsens = 0;
					myCh->op.select = SAVE_FLAG_SAVING_TIME;
					send_save_msg(bd, ch, 0, 0);
				}
			}

			rtn = SelectHwSpec_CAN(bd , ch);
			myCh->misc.preMode = mode;	//210128
			myCh->op.phase = P2;
			/* 210323 lyhw for Relay Off
			if(rtn == 2) {
				myCh->signal[C_SIG_OUT_SWITCH] = P0;
	    		myCh->op.phase = P50;
			}else if(rtn == 8){			//201208 check lyhw
	    		myCh->op.phase = P2;
			}else{
				cSemiSwitch_CAN(bd, ch);
	    		myCh->op.phase = P50;
			}*/
			break;
		case P1:
			myCh->op.select = SAVE_FLAG_SAVING_TIME;
			if(myCh->ChAttribute.opType == P0) {
				send_save_msg(bd, ch, 0, 0);
			}
	    	myCh->op.phase = P50;
			break;
		case P2:
			cSemiSwitch_CAN(bd, ch);
			myCh->op.phase = P3;
			break;
		case P3:
			myCh->signal[C_SIG_OUT_SWITCH] = P0;
			myCh->op.phase = P50;
			break;
		case P10:
			//SIK_hun_201010_s
			if(myCh->misc.ac_fail_flag == P1){
				myCh->op.phase = P15;
			}else if(myCh->misc.ac_fail_flag == P0){
				myCh->op.runTime += myPs->misc.rt_scan_time;
				myCh->op.totalRunTime += myPs->misc.rt_scan_time;
				myCh->op.phase = P50;
			}
	    	break;
		case P15:
			myCh->misc.ac_fail_flag = P0;
			rtn = temp_wait_flag_check(bd, ch);
			if(rtn == 0) {
				cSemiSwitch_Start(bd, ch, type);
				break;
			}
			myCh->misc.saveDt = myCh->op.runTime;
			//SKI_hun_201010_e
			myCh->op.runTime += myPs->misc.rt_scan_time;
			myCh->op.totalRunTime += myPs->misc.rt_scan_time;
			myCh->op.phase = P50;
			break;
		case P50:
			myCh->misc.start = 0;
			if(myCh->misc.nextDelay < P3 || myCh->misc.nextDelay == P5) {
	    		myCh->op.runTime += myPs->misc.rt_scan_time;
				myCh->op.totalRunTime += myPs->misc.rt_scan_time;
				myCh->misc.cycleRunTime += myPs->misc.rt_scan_time;
			}
			
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
			sens_ch_ad_count_increment(bd, ch);

			//191010
			if(myCh->op.checkDelayTime < 100){
				myCh->misc.Pre_change_V = myCh->misc.tmpVsens;
			}
			if(myCh->op.runTime >= 30) {
				myCh->misc.meanCount++;
				if(myCh->misc.meanCount <= 8) {
					myCh->misc.meanSumVolt += (myCh->misc.tmpVsens / 100);
					myCh->misc.meanSumCurr += (myCh->misc.tmpIsens / 100);
					myCh->misc.meanSumTemp += (myCh->op.temp / 100);
				} else {
					myCh->misc.meanSumVolt += (myCh->op.Vsens / 100);
					myCh->misc.meanSumCurr += (myCh->op.Isens / 100);
					myCh->misc.meanSumTemp += (myCh->op.temp / 100);
				}
				myCh->op.meanVolt = (long)(myCh->misc.meanSumVolt
					/ myCh->misc.meanCount) * 100;
				myCh->op.meanCurr = (long)(myCh->misc.meanSumCurr
					/ myCh->misc.meanCount) * 100;
				myCh->op.meanTemp = (long)(myCh->misc.meanSumTemp
					/ myCh->misc.meanCount) * 100;
			}
			if(myCh->signal[C_SIG_STOP] == P1) {
				myCh->op.code = C_FAULT_STOP_CMD;
				myCh->op.select = SAVE_FLAG_SAVING_END;
				myCh->signal[C_SIG_STOP] = P0;
				myCh->op.phase = P100;
			} else if(myCh->signal[C_SIG_PAUSE] == P1) {
				myCh->misc.tmpState = myCh->op.state;
				myCh->misc.tmpCode = myCh->op.code;
				myCh->op.code = C_FAULT_PAUSE_CMD;
				myCh->opSave = myCh->op;
				myCh->signal[C_SIG_PAUSE] = P0;
				myCh->op.phase = P100;
			} else {
				i = cFailCodeCheck(bd, ch);
			}
			
			if(myPs->config.hwSpec == L_5V_150A_R3_AD2
				|| myPs->config.hwSpec == L_8CH_MAIN_AD2_P){
				cCalculate_DCR(bd, ch, advStepNo);
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

void cStepOcv_CAN(int bd, int ch)
{
	int i, rangeV, rangeI, rtn = 0, mode;
	unsigned long advStepNo, saveDt;

	S_CH_STEP_INFO step;
	myCh = &(myData->bData[bd].cData[ch]);
	step = step_info(bd, ch);

    advStepNo = step.advStepNo;
	rangeV = step.rangeV;
	rangeI = step.rangeI;
	saveDt = step.saveDt;
	mode = step.mode;

    switch(myCh->op.phase) {
		case P0:
			myCh->misc.step_count++;
			myCh->misc.cycleStepCount++;
			// 111212 oys w : cycleNo
			myCh->misc.cycleNo = myPs->testCond[bd][ch].step[advStepNo].cycleNo;

			initCh(bd, ch);
			myCh->misc.startV = myCh->op.Vsens;
			myCh->misc.startT = myCh->op.temp;
			myCh->misc.maxT = myCh->op.temp;
			myCh->misc.minT = myCh->op.temp;

			rtn = SelectHwSpec_CAN(bd , ch);
			myCh->misc.preMode = mode;	//210128
			if(rtn == 2) {
				myCh->signal[C_SIG_OUT_SWITCH] = P0;
				myCh->op.phase = P50;
			}else if(rtn == 8){ //201208 check lyhw
				myCh->op.phase = P1;
			}else{
				//cSemiSwitch_CAN(bd, ch);
				myCh->misc.start = 0;
		    	myCh->op.phase = P50;
			}
			break;
		case P1:
			cSemiSwitch_CAN(bd, ch);	//210105
			myCh->op.phase = P2;
			break;
		case P2:
			myCh->signal[C_SIG_OUT_SWITCH] = P0;
			myCh->op.phase = P50;
			break;
		case P10:
			//SIK_hun_201010_s
			if(myCh->misc.ac_fail_flag == P1){
				myCh->op.phase = P15;
			}else if(myCh->misc.ac_fail_flag == P0){
				cSemiSwitch_Rest(bd, ch, advStepNo, 0);
				myCh->misc.start = 0;
		    	myCh->op.phase = P50;
			}
		case P15:
			myCh->misc.ac_fail_flag = P0;
			cSemiSwitch_Rest(bd, ch, advStepNo, 0);
			//SKI_hun_201010_e
			myCh->misc.start = 0;
			myCh->op.phase = P50;
	    	break;
		case P50:
	    	myCh->op.runTime += myPs->misc.rt_scan_time;
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
			sens_ch_ad_count_increment(bd, ch);
			if(myCh->op.runTime >= 30) {
				myCh->misc.meanCount++;
				if(myCh->misc.meanCount <= 8) {
					myCh->misc.meanSumVolt += (myCh->misc.tmpVsens / 100);
					myCh->misc.meanSumCurr += (myCh->misc.tmpIsens / 100);
					myCh->misc.meanSumTemp += (myCh->op.temp / 100);
				} else {
					myCh->misc.meanSumVolt += (myCh->op.Vsens / 100);
					myCh->misc.meanSumCurr += (myCh->op.Isens / 100);
					myCh->misc.meanSumTemp += (myCh->op.temp / 100);
				}
				myCh->op.meanVolt = (long)(myCh->misc.meanSumVolt
					/ myCh->misc.meanCount) * 100;
				myCh->op.meanCurr = (long)(myCh->misc.meanSumCurr
					/ myCh->misc.meanCount) * 100;
				myCh->op.meanTemp = (long)(myCh->misc.meanSumTemp
					/ myCh->misc.meanCount) * 100;
			}
			//191010
			if(myCh->op.checkDelayTime < 100){
				myCh->misc.Pre_change_V = myCh->misc.tmpVsens;
			}
			if(myCh->signal[C_SIG_STOP] == P1) {
				myCh->op.code = C_FAULT_STOP_CMD;
				myCh->op.select = SAVE_FLAG_SAVING_END;
				myCh->signal[C_SIG_STOP] = P0;
				myCh->op.phase = P100;
			} else if(myCh->signal[C_SIG_PAUSE] == P1) {
				myCh->misc.tmpState = myCh->op.state;
				myCh->misc.tmpCode = myCh->op.code;
				myCh->op.code = C_FAULT_PAUSE_CMD;
				myCh->opSave = myCh->op;
				myCh->signal[C_SIG_PAUSE] = P0;
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

void cStepZ_CAN(int bd, int ch)
{
	int rtn, rangeV, rangeI, div = 5, parallel_ch, mode;
    long val1, val2, val3, refV, refI, type;
	unsigned long advStepNo, saveDt;
	double tmp1, tmp2;
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
	val1 = step.refV;
	val2 = step.refI;
	val3 = step.refP;
	mode = step.mode;

	refV = myPs->config.maxVoltage[rangeV] * (-1.8);
	refI = -180000;
	//110428 kji 
	refV = val1;

	if(myData->mData.config.rt_scan_type > 0){
		refV = myPs->config.maxVoltage[rangeV]*(-1.8);
	}
	refV = myPs->config.maxVoltage[rangeV]*0.2;

	//kjg_180521
	if((ch % 2) == 0) parallel_ch = ch + 1;
	else parallel_ch = ch - 1;

    switch(myCh->op.phase) {
		case P0:
			initCh(bd, ch);
			rtn = temp_wait_flag_check(bd, ch);
			if(rtn == 0) {
				cSemiSwitch_Start(bd, ch, type);
				break;
			}
			myCh->misc.step_count++;
			myCh->misc.cycleStepCount++;
			// 111212 oys w : cycleNo
			myCh->misc.cycleNo = myPs->testCond[bd][ch].step[advStepNo].cycleNo;

			myCh->op.checkDelayTime = 0;
			myCh->misc.startV = myCh->op.Vsens;
			myCh->misc.maxV = myCh->op.Vsens;
			myCh->misc.minV = myCh->op.Vsens;
			myCh->misc.startT = myCh->op.temp;
			myCh->misc.maxT = myCh->op.temp;
			myCh->misc.minT = myCh->op.temp;
			//120315 kji 0 sec data option
			//170501 oys modify
			if(myData->DataSave.config.zero_sec_data_save == P1) {
				if(myCh->ChAttribute.opType == P0) {
					if(myCh->ChAttribute.chNo_master == 0) {
						myData->bData[bd].cData[ch-1].op.Isens = 0;
						myData->bData[bd].cData[ch-1].misc.tmpIsens = 0;
						myData->bData[bd].cData[ch-1].op.select
							= SAVE_FLAG_SAVING_TIME;
					}
					myCh->op.Isens = 0;
					myCh->misc.tmpIsens = 0;
					myCh->op.select = SAVE_FLAG_SAVING_TIME;
					send_save_msg(bd, ch, 0, 0);
				}
			}

			rtn = SelectHwSpec_CAN(bd , ch);
			myCh->misc.preMode = mode;	//210128
			if(rtn == 1) {
				ref_output_CAN(bd, ch, refV, val2, div, rangeV, rangeI, mode);
				if(myCh->ChAttribute.opType == P1){
					ref_output_CAN(bd, ch+1, refV, val2, div, rangeV, rangeI, mode);
					myCh = &(myData->bData[bd].cData[ch]);
				}
		    	myCh->op.phase = P50;
			} else if(rtn == 2) {
				myCh->misc.cmd_v = 0;
				myCh->misc.cmd_v_div = (short int)div;
				myCh->misc.cmd_v_range = (short int)rangeV;
				myCh->misc.cmd_i = val2;
				myCh->misc.cmd_i_div = (short int)div;
				myCh->misc.cmd_i_range = (short int)rangeI;
				myCh->signal[C_SIG_OUT_SWITCH] = P0;
				cSemiSwitch_Rest(bd, ch,0 ,0);
	    		myCh->op.phase = P5;
			} else if(rtn == 3) {	//210323 lyhw
				myCh->signal[C_SIG_OUT_SWITCH] = P0;
			//	if(myCh->misc.start != 1) cSemiSwitch_CAN(bd, ch);
				myCh->op.phase = P7;
			} else if(rtn == 4) {
				cSemiSwitch(bd, ch);
				if(myCh->misc.parallel_cycle_phase == P50) { //kjg_180521
					cSemiSwitch_p(bd, ch, parallel_ch);
				}
		    	myCh->op.phase = P50;
			} else if(rtn == 5) {
				cSemiSwitch_CAN(bd, ch);
	    		myCh->op.phase = P2;
			} else if(rtn == 6) {
				cSemiSwitch_CAN(bd, ch);
				myCh->op.checkDelayTime = 0;
	    		myCh->op.phase = P1;
			} else if(rtn == 7) {
				myCh->misc.cmd_v = 0;
				myCh->misc.cmd_v_div = (short int)div;
				myCh->misc.cmd_v_range = (short int)rangeV;
				myCh->misc.cmd_i = val2;
				myCh->misc.cmd_i_div = (short int)div;
				myCh->misc.cmd_i_range = (short int)rangeI;
				cSemiSwitch_Rest(bd, ch,0 ,0);
	    		myCh->op.phase = P6;
			}else if(rtn == 8){ 
				myCh->op.phase = P8;
			}
			break;
		case P1:
			myCh->misc.sensCount = 0;
			myCh->op.checkDelayTime = 0;
			myCh->misc.sensCountFlag = P0;
			myCh->misc.sensBufCount = 0;
			myCh->misc.sensBufCountFlag = P0;
			ref_output_CAN(bd, ch, refV, val2, div, rangeV, rangeI, mode);
			if(myCh->ChAttribute.opType == P1){
				ref_output_CAN(bd, ch+1, refV, val2, div, rangeV, rangeI, mode);
				myCh = &(myData->bData[bd].cData[ch]);
			}
		   	myCh->op.phase = P50;
			break;
		case P2:
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
			if(myCh->op.checkDelayTime >= 100) {
				myCh->op.checkDelayTime = 0;
				if(myCh->misc.start != 1)
					cSemiSwitch_Discharge(bd, ch, 1);
				if(myCh->ChAttribute.opType == P1)
				{
					myCh->signal[C_SIG_TRIGGER] = P1;
//					myData->bData[bd].cData[ch+1].signal[C_SIG_TRIGGER] = P1;
				}else if(myData->bData[bd].cData[ch]
						.ChAttribute.opType == P0
						&& myData->bData[bd].cData[ch]
						.ChAttribute.chNo_master == P0){
				}else{
					myCh->signal[C_SIG_TRIGGER] = P1;
				}
				myCh->op.phase = P1;
			}
			break;
		case P3:
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
			if(myCh->op.checkDelayTime >= 10) {
				myCh->op.checkDelayTime = 0;
				if(myCh->misc.start != 1)
					cSemiSwitch_Discharge(bd, ch, 1);
		    	myCh->op.phase = P4;
			}
			break;
		case P4:
			myCh->misc.sensCount = 0;
			myCh->misc.sensCountFlag = P0;
			myCh->misc.sensBufCount = 0;
			myCh->misc.sensBufCountFlag = P0;
			rtn = SelectHwSpec_CAN(bd , ch);
			if(rtn == 4) {
				cSemiSwitch(bd, ch);
				if(myCh->misc.parallel_cycle_phase == P50) { //kjg_180521
					cSemiSwitch_p(bd, ch, parallel_ch);
				}
			} else {
				ref_output_CAN(bd, ch, refV, val2, div, rangeV, rangeI, mode);
				if(myCh->misc.parallel_cycle_phase == P50) { //kjg_180521
					ref_output_p(bd, ch, parallel_ch, refV, val2, div,
						rangeV, rangeI, mode);
				}
				if(myCh->ChAttribute.opType == P1)
				{
					ref_output_CAN(bd, ch+1, refV, val2, div, rangeV, rangeI, mode);
					myCh = &(myData->bData[bd].cData[ch]);
				}
			}
	    	myCh->op.phase = P50;
			break;
		case P5:
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
			if(myCh->op.checkDelayTime == 10) {
				cSemiSwitch(bd, ch);
				if(myCh->misc.parallel_cycle_phase == P50) { //kjg_180521
					cSemiSwitch_p(bd, ch, parallel_ch);
				}
			}else if(myCh->op.checkDelayTime >= 30) {
				myCh->op.checkDelayTime = 0;
				myCh->signal[C_SIG_OUT_SWITCH] = P1;
				myCh->signal[C_SIG_I_CMD_OUTPUT] = P1;
				myCh->signal[C_SIG_V_CMD_OUTPUT] = P1;
				myCh->op.phase = P50;
			}
			break;
		case P6:
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
			if(myCh->op.checkDelayTime <= 2) {
				cSemiSwitch(bd, ch);
				if(myCh->misc.parallel_cycle_phase == P50) { //kjg_180521
					cSemiSwitch_p(bd, ch, parallel_ch);
				}
			}else if(myCh->op.checkDelayTime >= 4) {
				myCh->op.checkDelayTime = 0;
				myCh->signal[C_SIG_OUT_SWITCH] = P1;
				myCh->signal[C_SIG_I_CMD_OUTPUT] = P1;
				myCh->signal[C_SIG_V_CMD_OUTPUT] = P1;
				myCh->op.phase = P50;
			}
			break;
		case P7:	//210126 add for can Step Z
			if(myCh->op.checkDelayTime == 0) cSemiSwitch_CAN(bd, ch);
			if(myCh->op.checkDelayTime >= 10) {
				myCh->signal[C_SIG_RANGE_SWITCH] = P1;
			}
			if(myCh->op.checkDelayTime >= 20){
				myCh->signal[C_SIG_OUT_SWITCH] = P1;
			}

			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
			if(myCh->op.checkDelayTime >= 50){ 
				myCh->op.checkDelayTime = 0;
				myCh->op.phase = P1;
			}
			break;
		case P8:
			//210323 lyhw
			myCh->signal[C_SIG_OUT_SWITCH] = P0;
			myCh->op.checkDelayTime = 0;
			myCh->op.phase = P7;
			break;
		case P10:
		//SKI_hun_201010
			if(myCh->misc.ac_fail_flag == P1){
				myCh->op.phase = P15;
			}else if(myCh->misc.ac_fail_flag == P0){
				rtn = SelectHwSpec_CAN(bd , ch);
				if(rtn == 1
					|| rtn == 3
					|| rtn == 4
					|| rtn == 5
					|| rtn == 6) {
					cSemiSwitch_Discharge(bd, ch, 0);
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
				} else if(rtn == 2 || rtn == 7) {
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
					cSemiSwitch_Rest(bd,ch,0,0);
					myCh->op.phase = P13;
				}
			}
	    	break;
		case P11:
			cSemiSwitch_Discharge(bd, ch, 1);
			myCh->op.phase = P12;
			break;
		case P12:
			myCh->misc.sensCount = 0;
			myCh->misc.sensCountFlag = P0;
			myCh->misc.sensBufCount = 0;
			myCh->misc.sensBufCountFlag = P0;
			myCh->op.checkDelayTime = 0;
			if(myPs->config.capacityType == CAPACITY_CAPACITANCE) {
				ref_output_CAN(bd, ch, val1, val2, div, rangeV, rangeI, mode);
				if(myCh->misc.parallel_cycle_phase == P50) { //kjg_180521
					ref_output_p(bd, ch, parallel_ch, val1, val2, div,
						rangeV, rangeI, mode);
				}
				if(myCh->ChAttribute.opType == P1)
				{
					ref_output_CAN(bd, ch+1, val1, val2, div, rangeV, rangeI, mode);
					myCh = &(myData->bData[bd].cData[ch]);
				}
			} else {
				ref_output_CAN(bd, ch, val1, val2, div, rangeV, rangeI, mode);
				if(myCh->misc.parallel_cycle_phase == P50) { //kjg_180521
					ref_output_p(bd, ch, parallel_ch, val1, val2, div,
						rangeV, rangeI, mode);
				}
				if(myCh->ChAttribute.opType == P1)
				{
					ref_output_CAN(bd, ch+1, val1, val2, div, rangeV, rangeI, mode);
					myCh = &(myData->bData[bd].cData[ch]);
				}
			}
			myCh->op.phase = P50;
			break;
		case P13:
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
			if(myCh->op.checkDelayTime == 10) {
				cSemiSwitch(bd, ch);
				if(myCh->misc.parallel_cycle_phase == P50) { //kjg_180521
					cSemiSwitch_p(bd, ch, parallel_ch);
				}
			} else if(myCh->op.checkDelayTime >= 30) {
				myCh->misc.sensCount = 0;
				myCh->misc.sensCountFlag = P0;
				myCh->misc.sensBufCount = 0;
				myCh->misc.sensBufCountFlag = P0;
		
				myCh->op.checkDelayTime = 0;
				myCh->signal[C_SIG_OUT_SWITCH] = P1;
				myCh->signal[C_SIG_V_CMD_OUTPUT] = P1;
				myCh->signal[C_SIG_I_CMD_OUTPUT] = P1;
				myCh->op.phase = P50;
			}
			break;
		//SKI_hun_201010
		case P15:
			myCh->misc.ac_fail_flag = P0;
			rtn = temp_wait_flag_check(bd, ch);
			if(rtn == 0) {
				cSemiSwitch_Start(bd, ch, type);
				break;
			}
			myCh->misc.saveDt = myCh->op.runTime;
			rtn = SelectHwSpec_CAN(bd , ch);
			if(rtn == 1
				|| rtn == 3
				|| rtn == 4
				|| rtn == 5
				|| rtn == 6) {
				cSemiSwitch_Discharge(bd, ch, 0);
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
			} else if(rtn == 2 || rtn == 7) {
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
				cSemiSwitch_Rest(bd,ch,0,0);
				myCh->op.phase = P13;
			}
			break;
		case P20: //111130 kji run relay check 
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
			if(myCh->op.checkDelayTime >= 100) {
				myCh->op.checkDelayTime = 0;
				myCh->op.phase = P0;
			}
			break;
		case P50:
			myCh->misc.start = 0;
	    	myCh->op.runTime += myPs->misc.rt_scan_time;
			myCh->op.totalRunTime += myPs->misc.rt_scan_time;
			myCh->misc.cycleRunTime += myPs->misc.rt_scan_time;
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;

			sens_ch_ad_count_increment(bd, ch);

			if(myCh->op.runTime >= 30) {
				myCh->misc.meanCount++;
				if(myCh->misc.meanCount <= 8) {
					myCh->misc.meanSumVolt += (myCh->misc.tmpVsens / 100);
					myCh->misc.meanSumCurr += (myCh->misc.tmpIsens / 100);
					myCh->misc.meanSumTemp += (myCh->op.temp / 100);
				} else {
					myCh->misc.meanSumVolt += (myCh->op.Vsens / 100);
					myCh->misc.meanSumCurr += (myCh->op.Isens / 100);
					myCh->misc.meanSumTemp += (myCh->op.temp / 100);
				}
				myCh->op.meanVolt = (long)(myCh->misc.meanSumVolt
					/ myCh->misc.meanCount) * 100;
				myCh->op.meanCurr = (long)(myCh->misc.meanSumCurr
					/ myCh->misc.meanCount) * 100;
				myCh->op.meanTemp = (long)(myCh->misc.meanSumTemp
					/ myCh->misc.meanCount) * 100;
			}

			if(myCh->op.checkDelayTime == myPs->misc.rt_scan_time) {
				myCh->misc.cmd_v = val1;
				myCh->misc.cmd_v_div = (short int)div;
				myCh->misc.cmd_v_range = (short int)rangeV;
				myCh->signal[C_SIG_V_CMD_OUTPUT] = P1;
			}

			if(myCh->signal[C_SIG_STOP] == P1) {
				myCh->op.code = C_FAULT_STOP_CMD;
				myCh->op.select = SAVE_FLAG_SAVING_END;
				myCh->signal[C_SIG_STOP] = P0;
				cSemiSwitch_Rest(bd, ch, advStepNo, 0);
				myCh->op.phase = P100;
			} else if(myCh->signal[C_SIG_PAUSE] == P1) {
				myCh->misc.tmpState = myCh->op.state;
				myCh->misc.tmpCode = myCh->op.code;
				myCh->op.code = C_FAULT_PAUSE_CMD;
				myCh->opSave = myCh->op;
				myCh->signal[C_SIG_PAUSE] = P0;
				cSemiSwitch_Rest(bd, ch, advStepNo, 0);
				myCh->op.phase = P100;
			} else {
				if(cFailCodeCheck(bd, ch) >= 0) {
					cSoftFeedback(bd, ch, val1, val2, val3);
				}
			}

			//191010
			if(myCh->op.checkDelayTime < 100){
				myCh->misc.Pre_change_V = myCh->misc.tmpVsens;
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

			if(myCh->op.phase == P100) {
				cNextStepCheck(bd, ch);
			}
			break;
		case P100:
			break;
		default: break;
    }
}

void cStepUserPattern_CAN(int bd, int ch)
{
	unsigned char slave_flag = 1;
    int	rtn, realCh = 0, rangeV, rangeI, cnt, div = 5, mode;
    long val1, val2, val3, deltaT, length;
	unsigned long advStepNo, saveDt;
	double tmp1;
	int i = 0;
	S_CH_STEP_INFO step;

	myCh = &(myData->bData[bd].cData[ch]);
	myTestCond = &(myPs->testCond[bd][ch]);

	val1 = val2 = val3 = 0;
	tmp1 = 0.0;

	length = myData->mData.testCond[bd][ch].userPattern.length;

	step = step_info(bd, ch);

    advStepNo = step.advStepNo;
	rangeV = step.rangeV;
	rangeI = step.rangeI;
	saveDt = step.saveDt;
	val1 = step.refV;
	val2 = step.refI;
	val3 = step.refP;
	mode = step.mode;

    switch(myCh->op.phase) {
		case P0:
			myCh->misc.limit_current_timeout = myCh->op.runTime;
			realCh = myPs->config.chPerBd * bd + ch;
			send_msg(MODULE_TO_DATASAVE
					, MSG_MODULE_DATASAVE_READ_USER_PATTERN, realCh, advStepNo);
			myCh->misc.userPatternFlag = P0;
			myCh->op.checkDelayTime = 0;
			initCh(bd, ch);
			
			rtn = SelectHwSpec_CAN(bd , ch);
			myCh->misc.preMode = mode;	//210128
			if(rtn == 8){	//needCheck
				myCh->op.phase = P1;
			}else{
				//210323 lyhw
				myCh->signal[C_SIG_OUT_SWITCH] = P0;
				myCh->op.phase = P2;
			}
			break;
		case P1:
		//	cSemiSwitch_CAN(bd, ch);
			myCh->signal[C_SIG_OUT_SWITCH] = P0;
			myCh->op.phase = P2;
			break;
		case P2:
			if(myCh->ChAttribute.opType == P1){
				if(myData->bData[bd].cData[ch+1].misc.userPatternFlag == P1){
					slave_flag = P1;
				} else {
					slave_flag = P0;
				}
			}else{
				slave_flag = P1;
			}

			if((myCh->misc.userPatternFlag == P1) && (slave_flag == P1)){
				rtn = temp_wait_flag_check(bd, ch);
				if(rtn == 0) {
					cSemiSwitch_Rest(bd, ch, advStepNo, 0);
					break;
				}
				myCh->misc.step_count++;
				myCh->misc.cycleStepCount++;
				// 111212 oys w : cycleNo
				myCh->misc.cycleNo = myPs->testCond[bd][ch].step[advStepNo].cycleNo;

				myCh->misc.userPatternFlag = P0;
				myCh->op.checkDelayTime = 0;
				myCh->misc.startV = myCh->op.Vsens;
				myCh->misc.maxV = myCh->op.Vsens;
				myCh->misc.minV = myCh->op.Vsens;
				myCh->misc.startT = myCh->op.temp;
				myCh->misc.maxT = myCh->op.temp;
				myCh->misc.minT = myCh->op.temp;
				myCh->misc.limit_current_timeout = myCh->op.runTime;
				//170501 oys modify
				if(myData->DataSave.config.zero_sec_data_save == P1) {
					if(myCh->ChAttribute.opType == P0) {
						if(myCh->ChAttribute.chNo_master == 0) {
							myData->bData[bd].cData[ch-1].op.Isens = 0;
							myData->bData[bd].cData[ch-1].misc.tmpIsens = 0;
							myData->bData[bd].cData[ch-1].op.select
								= SAVE_FLAG_SAVING_TIME;
						}
						myCh->op.Isens = 0;
						myCh->misc.tmpIsens = 0;
						myCh->op.select = SAVE_FLAG_SAVING_TIME;
						send_save_msg(bd, ch, 0, 0);
					}
				}

				myCh->signal[C_SIG_V_RANGE] = rangeV + 1;
				myCh->signal[C_SIG_I_RANGE] = rangeI + 1;
				if(myCh->ChAttribute.opType == P1) {
					myCh->signal[C_SIG_PARALLEL_SWITCH] = P1;
				}
				myCh->op.phase = P3;
				myCh->op.checkDelayTime = 0; //131220
			}else{
			  	if(myCh->signal[C_SIG_STOP] == P1) {
					myCh->op.code = C_FAULT_STOP_CMD;
					myCh->op.select = SAVE_FLAG_SAVING_END;
					myCh->signal[C_SIG_STOP] = P0;
					myCh->op.phase = P100;
				} else if(myCh->signal[C_SIG_PAUSE] == P1) {
					//cmd, soft fault
					myCh->misc.tmpState = myCh->op.state;
					myCh->misc.tmpCode = myCh->op.code;
					myCh->op.code = C_FAULT_PAUSE_CMD;
					myCh->opSave = myCh->op;
					myCh->signal[C_SIG_PAUSE] = P0;
					myCh->op.phase = P100;
				}else if(myCh->signal[C_SIG_NEXTSTEP] == P1) {
					myCh->signal[C_SIG_NEXTSTEP] = P0;
					myCh->op.code = C_FAULT_NEXTSTEP_CMD;
					myCh->op.select = SAVE_FLAG_SAVING_END;
					myCh->op.phase = P100;
				} else {
					cFailCodeCheck(bd, ch);
				}
			}
			break;
		case P3:
			//210323 lyhw
			if(myCh->op.checkDelayTime == 0)  cSemiSwitch_CAN(bd, ch);

			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
			if(myCh->op.checkDelayTime >= 10) {
				myCh->signal[C_SIG_RANGE_SWITCH] = P1;	//131220
				myCh->misc.userPatternRunTime = myCh->op.runTime;
				myCh->op.checkDelayTime = 0;
				myCh->op.phase = P4;
			}
			break;
		case P4:
			rtn = SelectHwSpec_CAN(bd , ch);
			if(rtn == 1) {
			//	user_pattern_data_CAN(bd, ch);
				myCh->op.phase = P6;
			} else if(rtn == 2 || rtn == 7) {
				cSemiSwitch_Rest(bd, ch, 0, 0);
				myCh->op.phase = P7;
			} else if(rtn == 3) {
				myCh->op.phase = P6;
			} else if(rtn == 4) {
				myCh->op.phase = P6;
			} else if(rtn == 5) {
				myCh->op.phase = P6;
			} else if(rtn == 6) {
				myCh->op.phase = P6;
			} else if(rtn == 8) {	//201208 check lyhw
				myCh->op.phase = P6;
			}
			break;
		case P5:
			myCh->misc.sensCount = 0;
			myCh->misc.sensCountFlag = P0;
			myCh->misc.sensBufCount = 0;
			myCh->misc.sensBufCountFlag = P0;
			myCh->misc.userPatternRunTime = myCh->op.runTime;
			user_pattern_data_CAN(bd, ch);
			myCh->op.phase = P50;
			break;
		case P6:
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
			if(myCh->op.checkDelayTime == 10) {
				myCh->signal[C_SIG_OUT_SWITCH] = P1;
			}
			if(myCh->op.checkDelayTime >= 30) {
				myCh->op.checkDelayTime = 0;
				myCh->op.phase = P5;
			}
			break;
		case P7:
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
			if(myCh->op.checkDelayTime == 10) {
				cV_Range_Select(bd, ch, rangeV+1);
				cI_Range_Select(bd, ch, rangeI+1);
			} else if(myCh->op.checkDelayTime > 25) {
				myCh->op.checkDelayTime = 0;
				user_pattern_data_CAN(bd, ch);
				myCh->op.phase = P8;
			}
			break;
		case P8:
			myCh->signal[C_SIG_OUT_SWITCH] = P1;
			myCh->signal[C_SIG_V_CMD_OUTPUT] = P1;
			myCh->op.phase = P9;
			break;
		case P9:
			myCh->misc.sensCount = 0;
			myCh->misc.sensCountFlag = P0;
			myCh->misc.sensBufCount = 0;
			myCh->misc.sensBufCountFlag = P0;
			myCh->signal[C_SIG_I_CMD_OUTPUT] = P1;
			myCh->misc.userPatternRunTime = myCh->op.runTime;
			myCh->op.phase = P50;
			break;
		case P10:
			//SKI_hun_201010
			realCh = myPs->config.chPerBd * bd + ch;
			if(myCh->misc.ac_fail_flag == P1){
				send_msg(MODULE_TO_DATASAVE
					, MSG_MODULE_DATASAVE_READ_USER_PATTERN, realCh, advStepNo);
				myCh->op.phase = P15;
			}else if(myCh->misc.ac_fail_flag == P0){
				rtn = SelectHwSpec_CAN(bd , ch);
				if(rtn == 1
					|| rtn == 3
					|| rtn == 4
					|| rtn == 5
					|| rtn == 6
					|| rtn == 8) {
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
				} else if(rtn == 2 || rtn == 7) {
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
					myCh->op.phase = P12;
				}
			}
			break;
		case P11:
			user_pattern_data_CAN(bd, ch);
			myCh->op.phase = P50;
			break;
		case P12:
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
			if(myCh->op.checkDelayTime == 10) {
				cV_Range_Select(bd, ch, rangeV+1);
				cI_Range_Select(bd, ch, rangeI+1);
			} else if(myCh->op.checkDelayTime > 25) {
				myCh->op.checkDelayTime = 0;
				user_pattern_data_CAN(bd, ch);
				myCh->op.phase = P13;
			}
			break;
		case P13:
			myCh->signal[C_SIG_OUT_SWITCH] = P1;
			if(myPs->config.hwSpec > DC_DIGITAL_SPEC){//180611 
				myCh->signal[C_SIG_OUT_SWITCH_ON] = P1;
			}
			myCh->signal[C_SIG_V_CMD_OUTPUT] = P1;
			myCh->op.phase = P14;
			break;
		case P14:
			myCh->signal[C_SIG_I_CMD_OUTPUT] = P1;
			myCh->op.phase = P50;
			break;
		//SKI_hun_201010
		case P15:
			if(myCh->misc.userPatternFlag == P1){
				myCh->misc.ac_fail_flag = P0;
				length = myTestCond->userPattern.length;
				myCh->misc.saveDt = myCh->op.runTime;
				for(i = 0 ; i < length ; i++){ 
					if(myTestCond->userPattern.data[i].time >= myCh->op.runTime){
						myCh->misc.userPatternCnt = i;
						break;
					}
				}
				rtn = SelectHwSpec_CAN(bd , ch);
				if(rtn == 1
					|| rtn == 3
					|| rtn == 4
					|| rtn == 5
					|| rtn == 6
					|| rtn == 8) {
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
				} else if(rtn == 2 || rtn == 7) {
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
					myCh->op.phase = P12;
				}
			}
			break;
		case P50:
			if(myCh->misc.patternPhase == P0) {
	   	 		myCh->op.runTime += myPs->misc.rt_scan_time;
				myCh->op.totalRunTime += myPs->misc.rt_scan_time;
				myCh->misc.cycleRunTime += myPs->misc.rt_scan_time;
				myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
				myCh->misc.pattern_point_runTime += myPs->misc.rt_scan_time;
				sens_ch_ad_count_increment(bd, ch);
			} else if(myCh->misc.patternPhase == P1) {
				cC_D_Select(bd, ch, P0);
				myCh->signal[C_SIG_V_CMD_OUTPUT] = P1;
				myCh->signal[C_SIG_OUT_SWITCH] = P1;
				if(myPs->config.hwSpec > DC_DIGITAL_SPEC){//180611 
					myCh->signal[C_SIG_OUT_SWITCH_ON] = P1;
				}
				myCh->misc.patternPhase++; 
			} else if(myCh->misc.patternPhase == P2) {
				myCh->signal[C_SIG_I_CMD_OUTPUT] = P1;
				myCh->misc.patternPhase = P0; 
			} else if(myCh->misc.patternPhase == P11) {
				cC_D_Select(bd, ch, P1);
				myCh->misc.patternPhase++; 
			} else if(myCh->misc.patternPhase == P12) {
				myCh->signal[C_SIG_OUT_SWITCH] = P1;
				myCh->signal[C_SIG_V_CMD_OUTPUT] = P1;
				myCh->signal[C_SIG_I_CMD_OUTPUT] = P1;
				myCh->misc.patternPhase = P0; 
			} else {
				myCh->misc.patternPhase = P0; 
			}

			if(myCh->op.runTime >= 30) {
				myCh->misc.meanCount++;
				if(myCh->misc.meanCount <= 8) {
					myCh->misc.meanSumVolt += (myCh->misc.tmpVsens / 100);
					myCh->misc.meanSumCurr += (myCh->misc.tmpIsens / 100);
					myCh->misc.meanSumTemp += (myCh->op.temp / 100);
				} else {
					myCh->misc.meanSumVolt += (myCh->op.Vsens / 100);
					myCh->misc.meanSumCurr += (myCh->op.Isens / 100);
					myCh->misc.meanSumTemp += (myCh->op.temp / 100);
				}
				myCh->op.meanVolt = (long)(myCh->misc.meanSumVolt
					/ myCh->misc.meanCount) * 100;
				myCh->op.meanCurr = (long)(myCh->misc.meanSumCurr
					/ myCh->misc.meanCount) * 100;
				myCh->op.meanTemp = (long)(myCh->misc.meanSumTemp
					/ myCh->misc.meanCount) * 100;
			}

			if(myCh->signal[C_SIG_STOP] == P1) {
				myCh->op.code = C_FAULT_STOP_CMD;
				myCh->op.select = SAVE_FLAG_SAVING_END;
				myCh->signal[C_SIG_STOP] = P0;
				cSemiSwitch_Rest(bd, ch, advStepNo, 0);
				myCh->op.phase = P100;
			} else if(myCh->signal[C_SIG_PAUSE] == P1) {
				//cmd, soft fault
				myCh->misc.tmpState = myCh->op.state;
				myCh->misc.tmpCode = myCh->op.code;
				myCh->op.code = C_FAULT_PAUSE_CMD;
				myCh->opSave = myCh->op;
				myCh->signal[C_SIG_PAUSE] = P0;
				cSemiSwitch_Rest(bd, ch, advStepNo, 0);
				myCh->op.phase = P100;
			} else {
				if(cFailCodeCheck(bd, ch) >= 0) {
					cSoftFeedback(bd, ch, val1, val2, val3);
				}
			}

			//201208 CanType Not Use ?
			/*
			if(myCh->misc.refFlag == P1) {
				myCh->misc.refFlag = P0;
				cCalCmdV(bd, ch, val1, div, rangeV);
			}*/

			//191010
			if(myCh->op.checkDelayTime < 100){
				myCh->misc.Pre_change_V = myCh->misc.tmpVsens;
			}
	    	if(myCh->op.checkDelayTime == 100) {
				myCh->misc.maxI = myCh->op.Isens;
				myCh->misc.minI = myCh->op.Isens;
			}

			if(myCh->op.phase == P100) {
				myCh->ccv[0].avg_v = myCh->misc.tmpVsens;
				myCh->ccv[0].avg_i = myCh->misc.tmpIsens;
				cNextStepCheck(bd, ch);
			} else {
			    deltaT = ((myCh->op.runTime / 10) * 10)
					- (myCh->misc.userPatternRunTime
	    			- myPs->misc.rt_scan_time);
				cnt = find_pattern_time(bd, ch, deltaT);
				if(myCh->misc.userPatternCnt != cnt) {	
					myCh->misc.userPatternCnt = cnt;
					myCh->misc.sensCount = 0;
					myCh->misc.sensCountFlag = 0;
					myCh->misc.pattern_point_runTime = 0;
					user_pattern_data_CAN(bd, ch);
				}
			}
			break;
		case P100:
			break;
		default: break;
    }
}

void cStepUserMap_CAN(int bd, int ch)
{
    int	rtn, realCh = 0, rangeV, rangeI, cnt, div = 5, parallel_ch, mode;
    long val1, val2, val3, deltaT;
	unsigned long advStepNo, saveDt;
	double tmp1;
	S_CH_STEP_INFO step;

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
	val3 = step.refP;
	mode = step.mode;

	//kjg_180521
	if((ch % 2) == 0) parallel_ch = ch + 1;
	else parallel_ch = ch - 1;

    switch(myCh->op.phase) {
		case P0:
			rtn = temp_wait_flag_check(bd, ch);
			if(rtn == 0) {
				cSemiSwitch_Rest(bd, ch, advStepNo, 0);
				break;
			}
			myCh->misc.step_count++;
			myCh->misc.cycleStepCount++;
			// 111212 oys w : cycleNo
			myCh->misc.cycleNo = myPs->testCond[bd][ch].step[advStepNo].cycleNo;
			myCh->misc.limit_current_timeout = myCh->op.runTime;
//			rtn = temp_wait_flag_check(bd, ch);
//			if(rtn == 0) break;
			cSemiSwitch(bd, ch);
			if(myCh->misc.parallel_cycle_phase == P50) { //kjg_180521
				cSemiSwitch_p(bd, ch, parallel_ch);
			}
			realCh = myPs->config.chPerBd * bd + ch;
			send_msg(MODULE_TO_DATASAVE
					, MSG_MODULE_DATASAVE_READ_USER_MAP, realCh, advStepNo);
			myCh->misc.userMapFlag = P0;
			myCh->op.checkDelayTime = 0;
			myCh->op.phase = P1;
			initCh(bd, ch);
			break;
		case P1:
			if(myCh->misc.userMapFlag == P1){
				rtn = temp_wait_flag_check(bd, ch);
				if(rtn == 0) {
					cSemiSwitch_Rest(bd, ch, advStepNo, 0);
					break;
				}
				myCh->misc.preMode = mode;	//210128
				myCh->misc.userMapFlag = P0;
				myCh->op.checkDelayTime = 0;
				myCh->misc.startV = myCh->op.Vsens;
				myCh->misc.maxV = myCh->op.Vsens;
				myCh->misc.minV = myCh->op.Vsens;
				myCh->misc.startT = myCh->op.temp;
				myCh->misc.maxT = myCh->op.temp;
				myCh->misc.minT = myCh->op.temp;
	//			myCh->misc.limit_current_timeout= myData->mData.misc.timer_1sec;
				myCh->misc.limit_current_timeout = myCh->op.runTime;
				//120315 kji 0 sec data option
				//170501 oys modify
				if(myData->DataSave.config.zero_sec_data_save == P1) {
					if(myCh->ChAttribute.opType == P0) {
						if(myCh->ChAttribute.chNo_master == 0) {
							myData->bData[bd].cData[ch-1].op.Isens = 0;
							myData->bData[bd].cData[ch-1].misc.tmpIsens = 0;
							myData->bData[bd].cData[ch-1].op.select
								= SAVE_FLAG_SAVING_TIME;
						}
						myCh->op.Isens = 0;
						myCh->misc.tmpIsens = 0;
						myCh->op.select = SAVE_FLAG_SAVING_TIME;
						send_save_msg(bd, ch, 0, 0);
					}
				}
				myCh->op.phase = P3;
			}else{
			  	if(myCh->signal[C_SIG_STOP] == P1) {
					myCh->op.code = C_FAULT_STOP_CMD;
					myCh->op.select = SAVE_FLAG_SAVING_END;
					myCh->signal[C_SIG_STOP] = P0;
					myCh->op.phase = P100;
				} else if(myCh->signal[C_SIG_PAUSE] == P1) {
					//cmd, soft fault
					myCh->misc.tmpState = myCh->op.state;
					myCh->misc.tmpCode = myCh->op.code;
					myCh->op.code = C_FAULT_PAUSE_CMD;
					myCh->opSave = myCh->op;
					myCh->signal[C_SIG_PAUSE] = P0;
					myCh->op.phase = P100;
				}else if(myCh->signal[C_SIG_NEXTSTEP] == P1) {
					myCh->signal[C_SIG_NEXTSTEP] = P0;
					myCh->op.code = C_FAULT_NEXTSTEP_CMD;
					myCh->op.select = SAVE_FLAG_SAVING_END;
					myCh->op.phase = P100;
				} else {
					cFailCodeCheck(bd, ch);
				}
			}
			break;
		case P2:
			myCh->op.checkDelayTime 
				+= myPs->misc.rt_scan_time;
			if(myCh->op.checkDelayTime >= 20) {
				cSemiSwitch_Pattern(bd, ch, 1);
				myCh->op.checkDelayTime = 0;
				myCh->op.phase = P3;
			}
			break;
		case P3:
			rtn = SelectHwSpec_CAN(bd , ch);
			myCh->misc.soc = read_user_map_ocvTable(bd, ch);
			myCh->misc.startSoc = myCh->misc.soc;
			rtn = 3;
			if(rtn == 1) {
				myCh->op.phase = P5;
			} else if(rtn == 2 || rtn == 7) {
				cSemiSwitch_Rest(bd, ch, 0, 0);
				myCh->op.phase = P6;
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
				myCh->signal[C_SIG_OUT_SWITCH] = P1;
				if(myPs->config.hwSpec > DC_DIGITAL_SPEC){//180611 
					myCh->signal[C_SIG_OUT_SWITCH_ON] = P1;
				}
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
		case P6:
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;
			if(myCh->op.checkDelayTime == 10) {
				cV_Range_Select(bd, ch, rangeV+1);
				cI_Range_Select(bd, ch, rangeI+1);
			} else if(myCh->op.checkDelayTime > 25) {
				myCh->op.checkDelayTime = 0;
				user_pattern_data_CAN(bd, ch);
				myCh->op.phase = P7;
			}
			break;
		case P7:
			myCh->signal[C_SIG_OUT_SWITCH] = P1;
			if(myPs->config.hwSpec > DC_DIGITAL_SPEC){//180611 
				myCh->signal[C_SIG_OUT_SWITCH_ON] = P1;
			}
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
			break;
		case P10:
			rtn = SelectHwSpec_CAN(bd , ch);
			if(rtn == 1
				|| rtn == 3
				|| rtn == 4
				|| rtn == 5
				|| rtn == 6) {
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
			} else if(rtn == 2 || rtn == 7) {
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
				myCh->op.phase = P12;
			}
			break;
		case P11:
			user_map_data(bd,ch);
			myCh->op.phase = P50;
			break;
		case P12:
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
			if(myPs->config.hwSpec > DC_DIGITAL_SPEC){//180611 
				myCh->signal[C_SIG_OUT_SWITCH_ON] = P1;
			}
			myCh->signal[C_SIG_V_CMD_OUTPUT] = P1;
			myCh->op.phase = P14;
			break;
		case P14:
			myCh->signal[C_SIG_I_CMD_OUTPUT] = P1;
			myCh->op.phase = P50;
			break;
		case P50:
			myCh->misc.start = 0;
	    	myCh->op.runTime += myPs->misc.rt_scan_time;
			myCh->op.totalRunTime += myPs->misc.rt_scan_time;
			myCh->misc.cycleRunTime += myPs->misc.rt_scan_time;
			myCh->op.checkDelayTime += myPs->misc.rt_scan_time;

			sens_ch_ad_count_increment(bd, ch);

			if(myCh->op.runTime % myData->mData.testCond[bd][ch].
				userMap.renewalTime == 0) {
				myCh->misc.soc = myCh->misc.startSoc
					+ (long)((double)(myCh->op.ampareHour)
					/ (double)(myData->mData.testCond[bd][ch]
					.userMap.maxCapacity) * 100000);
				user_map_data(bd,ch);
			}

			if(myCh->op.runTime >= 30) {
				myCh->misc.meanCount++;
				if(myCh->misc.meanCount <= 8) {
					myCh->misc.meanSumVolt += (myCh->misc.tmpVsens / 100);
					myCh->misc.meanSumCurr += (myCh->misc.tmpIsens / 100);
					myCh->misc.meanSumTemp += (myCh->op.temp / 100);
				} else {
					myCh->misc.meanSumVolt += (myCh->op.Vsens / 100);
					myCh->misc.meanSumCurr += (myCh->op.Isens / 100);
					myCh->misc.meanSumTemp += (myCh->op.temp / 100);
				}
				myCh->op.meanVolt = (long)(myCh->misc.meanSumVolt
					/ myCh->misc.meanCount) * 100;
				myCh->op.meanCurr = (long)(myCh->misc.meanSumCurr
					/ myCh->misc.meanCount) * 100;
				myCh->op.meanTemp = (long)(myCh->misc.meanSumTemp
					/ myCh->misc.meanCount) * 100;
			}

			if(myCh->signal[C_SIG_STOP] == P1) {
				myCh->op.code = C_FAULT_STOP_CMD;
				myCh->op.select = SAVE_FLAG_SAVING_END;
				myCh->signal[C_SIG_STOP] = P0;
				cSemiSwitch_Rest(bd, ch, advStepNo, 0);
				myCh->op.phase = P100;
			} else if(myCh->signal[C_SIG_PAUSE] == P1) {
				//cmd, soft fault
				myCh->misc.tmpState = myCh->op.state;
				myCh->misc.tmpCode = myCh->op.code;
				myCh->op.code = C_FAULT_PAUSE_CMD;
				myCh->opSave = myCh->op;
				myCh->signal[C_SIG_PAUSE] = P0;
				cSemiSwitch_Rest(bd, ch, advStepNo, 0);
				myCh->op.phase = P100;
			} else {
				if(cFailCodeCheck(bd, ch) >= 0) {
					cSoftFeedback(bd, ch, val1, val2, val3);
				}
			}

/* 20120126 kji value move phase 0
	    	if(myCh->op.checkDelayTime == 100) {
				myCh->misc.maxV = myCh->op.Vsens;
				myCh->misc.minV = myCh->op.Vsens;
				myCh->misc.maxI = myCh->op.Isens;
				myCh->misc.minI = myCh->op.Isens;
			}
*/
			//191010
			if(myCh->op.checkDelayTime < 100){
				myCh->misc.Pre_change_V = myCh->misc.tmpVsens;
			}
	    	if(myCh->op.checkDelayTime == 100) {
				myCh->misc.maxI = myCh->op.Isens;
				myCh->misc.minI = myCh->op.Isens;
			}

			if(myCh->op.phase == P100) {
				myCh->ccv[0].avg_v = myCh->misc.tmpVsens;
				myCh->ccv[0].avg_i = myCh->misc.tmpIsens;
				cNextStepCheck(bd, ch);
			} else {
			    deltaT = ((myCh->op.runTime / 10) * 10)
					- (myCh->misc.userPatternRunTime
					- myPs->misc.rt_scan_time);
				cnt = find_pattern_time(bd, ch, deltaT);
				if(myCh->misc.userPatternCnt != cnt) {	
					myCh->misc.userPatternCnt = cnt;
					user_pattern_data_CAN(bd, ch);
				}
			}
			break;
		case P100:
			break;
		default:
			break;
    }
}


void cStepBalance_CAN(int bd, int ch)
{
	unsigned long refV , endV , advStepNo , mode;

	myCh = &(myData->bData[bd].cData[ch]);
	advStepNo = myCh->misc.advStepNo;
	refV = myPs->testCond[bd][ch].step[advStepNo].refV;
	endV = myPs->testCond[bd][ch].step[advStepNo].endV;
	mode = myPs->testCond[bd][ch].step[advStepNo].mode;
	
	switch(myCh->op.phase) {
		case P0:
			initCh(bd, ch);
	 	   	myCh->op.phase = P1;
			break;
		case P1:
			switch(mode){
				case CCCV:
				case CV:
					if(myCh->op.Vsens > refV){ //Voltage >  refV  Discharge
						myCh->op.type = STEP_DISCHARGE;
						myCh->op.phase = P0;
						if(myPs->testCond[bd][ch].step[advStepNo].refI > 0)
							myPs->testCond[bd][ch].step[advStepNo].refI *= -1;
	 					myPs->testCond[bd][ch].step[advStepNo].type = 
								STEP_DISCHARGE;
					} else {
						myCh->op.type = STEP_CHARGE;
						myCh->op.phase = P0;
						if(myPs->testCond[bd][ch].step[advStepNo].refI < 0)
							myPs->testCond[bd][ch].step[advStepNo].refI *= -1;
	 					myPs->testCond[bd][ch].step[advStepNo].type = 
								STEP_CHARGE;
					}
					break;
				case CC:  //cc mode 
					if(endV == 0) {
						if(myCh->op.Vsens > refV){ //Voltage >  refV  Discharge
							myCh->op.type = STEP_DISCHARGE;
							myCh->op.phase = P0;
							if(myPs->testCond[bd][ch].step[advStepNo].refI > 0)
								myPs->testCond[bd][ch].step[advStepNo].refI *= -1;
	 						myPs->testCond[bd][ch].step[advStepNo].type = 
									STEP_DISCHARGE;
						} else {
							myCh->op.type = STEP_CHARGE;
							myCh->op.phase = P0;
							if(myPs->testCond[bd][ch].step[advStepNo].refI < 0)
								myPs->testCond[bd][ch].step[advStepNo].refI *= -1;
	 						myPs->testCond[bd][ch].step[advStepNo].type = 
									STEP_CHARGE;
						}
					} else {
						if(myCh->op.Vsens > endV){ //Voltage >  refV  Discharge
							myCh->op.type = STEP_DISCHARGE;
							myCh->op.phase = P0;
							if(myPs->testCond[bd][ch].step[advStepNo].refI > 0)
								myPs->testCond[bd][ch].step[advStepNo].refI *= -1;
	 						myPs->testCond[bd][ch].step[advStepNo].type = 
									STEP_DISCHARGE;
	 						myPs->testCond[bd][ch].step[advStepNo].refV =
								   endV - 100000;	
						} else {
							myCh->op.type = STEP_CHARGE;
							myCh->op.phase = P0;
							if(myPs->testCond[bd][ch].step[advStepNo].refI < 0)
								myPs->testCond[bd][ch].step[advStepNo].refI *= -1;
	 						myPs->testCond[bd][ch].step[advStepNo].type = 
									STEP_CHARGE;
	 						myPs->testCond[bd][ch].step[advStepNo].refV =
								   endV + 100000;	
						}
					}
					break;
				case CP:  //cp mode 
					if(endV == 0) {
						if(myCh->op.Vsens > refV){ //Voltage >  refV  Discharge
							myCh->op.type = STEP_DISCHARGE;
							myCh->op.phase = P0;
							if(myPs->testCond[bd][ch].step[advStepNo].refP < 0)
								myPs->testCond[bd][ch].step[advStepNo].refP *= -1;
	 						myPs->testCond[bd][ch].step[advStepNo].type = 
									STEP_DISCHARGE;
						} else {
							myCh->op.type = STEP_CHARGE;
							myCh->op.phase = P0;
							if(myPs->testCond[bd][ch].step[advStepNo].refP < 0)
								myPs->testCond[bd][ch].step[advStepNo].refP *= -1;
	 						myPs->testCond[bd][ch].step[advStepNo].type = 
									STEP_CHARGE;
						}
					} else {
						if(myCh->op.Vsens > endV){ //Voltage >  refV  Discharge
							myCh->op.type = STEP_DISCHARGE;
							myCh->op.phase = P0;
							if(myPs->testCond[bd][ch].step[advStepNo].refP < 0)
								myPs->testCond[bd][ch].step[advStepNo].refP *= -1;
	 						myPs->testCond[bd][ch].step[advStepNo].type = 
									STEP_DISCHARGE;
	 						myPs->testCond[bd][ch].step[advStepNo].refV =
								   endV - 100000;	
	 						myPs->testCond[bd][ch].step[advStepNo].refI *= -1;
						} else {
							myCh->op.type = STEP_CHARGE;
							myCh->op.phase = P0;
							if(myPs->testCond[bd][ch].step[advStepNo].refP < 0)
								myPs->testCond[bd][ch].step[advStepNo].refP *= -1;
	 						myPs->testCond[bd][ch].step[advStepNo].type = 
									STEP_CHARGE;
	 						myPs->testCond[bd][ch].step[advStepNo].refV =
								   endV + 100000;	
						}
					}
					break;
				}
			break;
		default:
			break;
	}
}

void cStepDefault_CAN(int bd, int ch)
{
	myCh = &(myData->bData[bd].cData[ch]);

	switch(myCh->op.type) {
		case STEP_ADV_CYCLE:
			cStepAdvCycle(bd, ch);
			break;
		case STEP_PARALLEL_CYCLE: //kjg_180521
			cStepParallelCycle(bd, ch);
			break;
		case STEP_LOOP:
			cStepLoop(bd, ch, 0);
			break;
		case STEP_END:
			cStepEnd(bd, ch);
			break;
		default:
			break;
	}
}

//20190605 KHK----------------------------------------------------
void cSemiSwitch_CAN(int bd, int ch)
{
	int tmp_vRef, tmp_iRef;	
	char can_cmd;

	//201229 lyhw
	myCh->signal[C_SIG_OUT_SWITCH] = P2;
	myCh->signal[C_SIG_RANGE_SWITCH] = P2;
		
	can_cmd = CAN_COMMAND_RESET;

	tmp_vRef = 0;
   	tmp_iRef = 0;	
	
	myData->bData[bd].cData[ch].misc.preVref = tmp_vRef;
	myData->bData[bd].cData[ch].misc.preIref = tmp_iRef;

	bCan_Full_Ref_Cmd_Out(can_cmd, bd, ch, tmp_vRef, tmp_iRef);
}

int SelectHwSpec_CAN(int bd, int ch)
{
	int rtn = 1;

	myCh = &(myData->bData[bd].cData[ch]);

	if(myCh->misc.preMode == CP 
		|| myCh->misc.preMode == CR
		|| myCh->misc.preMode == CPCV){
		return 8;
	}else{
		if(myCh->misc.start == 1){ 
			rtn = 1;
		}else{
			if(myCh->misc.preRangeI != myCh->op.rangeI){
				rtn = 3;
			}
		}
	}

	//220117 jws add
	if(myCh->misc.canNextStepFlag == P1){
		myCh->misc.canNextStepFlag = P0;
		return 3;
	}

	if(myCh->op.preType == STEP_REST) {
		myCh->misc.preRangeI = myCh->op.rangeI;
		return 1;
	} else if(myCh->op.preType == STEP_OCV
		|| myCh->op.preType == STEP_USER_PATTERN) {
		myCh->misc.preRangeI = myCh->op.rangeI;
		return 8;
	} else {
		if(myCh->op.rangeI == myCh->misc.preRangeI){ 
			myCh->misc.preMode = myCh->op.mode;
			return 1;
		}
	}

	//kjgw_180521
	myCh->misc.preRangeI = myCh->op.rangeI; 

	return rtn;
}

void ref_output_CAN(int bd, int ch, long Vref, long Iref, int div, int rangeV, int rangeI, int mode)
{
	long tmp_vRef = 0, tmp_iRef = 0, can_cmd = 0;

	tmp_vRef = cCalCmdV_CAN(bd, ch, Vref, div, rangeV, 1);
	tmp_iRef = cCalCmdI_CAN(bd, ch, Iref, div, rangeI, 1);
	
	if(Iref == 0)	mode = CC;
	else	mode = CCCV;

	if(tmp_vRef < 0) {
		tmp_vRef = 0;
	}

	switch(mode){
		case CC:
			can_cmd = CAN_COMMAND_CC;
			break;
		case CCCV:
		case CV:
		case DC:
		case AC:
			can_cmd = CAN_COMMAND_CCCV;
			break;
		case CP:
		case CCP:
			can_cmd = CAN_COMMAND_CCCV;
			break;
		case CR:
			can_cmd = CAN_COMMAND_CCCV;
			break;
		default:
			can_cmd = CAN_COMMAND_RESET;
			break;
	}
	
	myData->bData[bd].cData[ch].misc.preVref = tmp_vRef;
	myData->bData[bd].cData[ch].misc.preIref = tmp_iRef;

//	printk("Time %ld.%ld :%dch pre vout %ld, after vout %ld, pre iout %ld, after iout %ld rangeV %d rangeI %d\n",myPs->real_time[1], myPs->real_time[2], ch, Vref, tmp_vRef, Iref, tmp_iRef, rangeV, rangeI);
	//sungdel

	bCan_Full_Ref_Cmd_Out(can_cmd, bd, ch, (int)tmp_vRef, (long)tmp_iRef);
}

void user_pattern_data_CAN(int bd, int ch)
{
	int div = 5, rangeV, rangeI, mode;
	long val1, val2, refV, maxI;
//	int parallel_ch;
	S_USER_PATTERN_DATA pattern;
	S_CH_STEP_INFO step;

	myCh = &(myData->bData[bd].cData[ch]);

	pattern = pattern_info(bd, ch);
	step = step_info(bd, ch);

	val1 = step.refV;
	val2 = step.refI;
	rangeV = step.rangeV;
	rangeI = step.rangeI;
	mode = step.mode;
	maxI = myPs->config.maxCurrent[rangeI];

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

	refV = val1;
	switch(myPs->config.hwSpec) {
		default:
			refV = val1;
			break;
	}

	if(myCh->ChAttribute.opType == P0){
		ref_output_CAN(bd, ch, val1, val2, div, rangeV, rangeI, mode);
		if(myCh->ChAttribute.chNo_master == P0){
			ref_output_CAN(bd, ch-1, val1, val2, div, rangeV, rangeI, mode);
			myCh = &(myData->bData[bd].cData[ch]);
		}
		//180531 lyhw
		/*
		if(myCh->misc.parallel_cycle_phase == P50){
			if((ch%2) == 0) parallel_ch = ch + 1;
			else parallel_ch = ch - 1;
			ref_output_p(bd, ch, parallel_ch, val1, val2, div, rangeV, rangeI, mode);
		}
		*/
	}
}

void CAN_Inverter_Control(int bd,int ch)
{	//220112 lyhw
	if(CYCLER_TYPE != CAN_CYC) return;
	if(myData->mData.signal[M_SIG_INV_POWER_CAN] <= P10) return;		

	CAN_Inverter_Fault(bd, ch);
}

void CAN_Inverter_Fault(int bd, int ch)
{	//220112 lyhw
	int inv, installedInv;
	
	myCh = &(myData->bData[bd].cData[ch]);

	if(myCh->op.state != C_RUN) return;
	if(myCh->op.phase != P50) return;

	installedInv = myData->CAN.config.installedInverter;
		
	for(inv = 0; inv < installedInv; inv++){
		if(myData->CAN.inverter[inv].state.steady != 0x01){
			if(myCh->misc.can_inv_errflag == 0){
				myCh->misc.can_inv_errflag = 1;
				myData->bData[bd].cData[ch].misc.tmpCode
						= myData->bData[bd].cData[ch].op.code;

				switch(myData->CAN.invsavefault[inv].code){
					case P_INV_LAG_SHORT:		//0x0001
						myCh->op.code = P_INV_FAULT_LAG_SHORT;
						break;
					case P_INV_OVER_CURRENT:	//0x0002
						myCh->op.code = P_INV_FAULT_OVER_CURRENT;
						break;
					case P_INV_OVER_VOLTAGE:	//0x0004
						myCh->op.code = P_INV_FAULT_OVER_VOLTAGE;
						break;
					case P_INV_PRECHARGE_FAIL:	//0x0008
						myCh->op.code = P_INV_FAULT_PRECHARGE_FAIL;
						break;
					case P_INV_OVER_CURRENT2:	//0x0010
						myCh->op.code = P_INV_FAULT_OVER_CURRENT2;
						break;
					case P_INV_CAN_ERR:			//0x0020
						myCh->op.code = P_INV_FAULT_CAN_ERR;
						break;
					case P_INV_OVER_LOAD:		//0x0040
						myCh->op.code = P_INV_FAULT_OVER_LOAD;
						break;
					case P_INV_OVER_HEAT:		//0x0080
						myCh->op.code = P_INV_FAULT_OVER_HEAT;
						break;
					case P_INV_LOW_VOLTAGE:		//0x0200
						myCh->op.code = P_INV_FAULT_LOW_VOLTAGE;
						break;
					case P_INV_AC_LOW_VOLTAGE:	//0x0400
						myCh->op.code = P_INV_FAULT_AC_LOW_VOLTAGE;
						break;
					case P_INV_RESET1:			//0x0800
						myCh->op.code = P_INV_FAULT_RESET1;
						break;
					case P_INV_RESET2:			//0x1000
						myCh->op.code = P_INV_FAULT_RESET2;
						break;
					case P_INV_AC_INPUT_FAIL:	//0x2000
						myCh->op.code = P_INV_FAULT_AC_INPUT_FAIL;
						break;
					case P_INV_AC_OVER_VOLT:	//0x4000
						myCh->op.code = P_INV_FAULT_AC_OVER_VOLT;
						break;
					case P_INV_HDC_ERROR:		//0x8000
						myCh->op.code = P_INV_FAULT_HDC_ERROR;
						break;
					default:
						myCh->op.code = P_INV_FAULT_ETC;
						break;
					
				}
				myCh->misc.errCnt[C_CNT_INV_FAULT_CAN] = P1;
				break;
			}
		}else{
			myCh->misc.errCnt[C_CNT_INV_FAULT_CAN] = 0;
			myCh->misc.can_inv_errflag = 0;
		}
	}
}

void CAN_FaultCond_Common(int bd,int ch)		
{	//220112 lyhw
	int rtn = 0;
	unsigned long saveDt;
	S_CH_STEP_INFO step;

	myCh = &(myData->bData[bd].cData[ch]);

	if(myCh->op.state != C_RUN) return;
	if(myCh->op.phase != P50) return;

	saveDt = step.saveDt;

	if(myCh->misc.errCnt[C_CNT_INV_FAULT_CAN] > 0){
		myCh->misc.errCnt[C_CNT_INV_FAULT_CAN] = 0;
		myCh->misc.tmpState = myCh->op.state;
		rtn = FAULT_COND;
	}
	
	if(rtn == FAULT_COND){
		myCh->op.select = SAVE_FLAG_SAVING_ETC;
		send_save_msg(bd, ch, saveDt, 0);
		if(myCh->misc.tempWaitType == P0){
			myCh->misc.chGroupNo = 0;
			myCh->misc.stepSyncFlag = 0;
		}
		myCh->op.phase = P100;
		cNextStepCheck(bd, ch);
	}
}

int INV_StateCheck(void)
{	//220113 lyhw

	if(myPs->signal[M_SIG_INV_POWER_CAN] == P10) return 1;

	if(myPs->signal[M_SIG_INV_POWER_CAN] == P99){
		myPs->signal[M_SIG_INV_POWER_CAN] = P100;
	}
	return 0;
}

void cStepCCCV_Check_CAN(int bd, int ch, long val1, long val2)
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
					myCh->misc.cvFlag = P1;
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
					myCh->misc.cvFlag = P1;
   					myCh->misc.cvFaultCheckFlag = P1; //210204
				}
			}
			break;
		default:
			break;
	}
}

