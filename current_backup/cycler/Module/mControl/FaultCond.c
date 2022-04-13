#include <asm/io.h>
#include <math.h>
#include <rtl_core.h>
#include <pthread.h>
#include "../../INC/datastore.h"
#include "message.h"
#include "Analog.h"
#include "FAD.h"
#include "ChannelControl.h"
#include "ChannelControl_CAN.h"
#include "PCU_Control.h"
#include "PCU.h"
#include "local_utils.h"
#include "CAN.h"
#include "FaultCond.h"

extern S_SYSTEM_DATA *myData;
extern S_MODULE_DATA *myPs;    
extern S_TEST_CONDITION *myTestCond;	//190901 lyhw
extern S_CH_DATA *myCh;


void cFaultCondHard(int bd, int ch)
{
	unsigned char type, mode, checkFlag;
	int rangeV, rangeI, rtn = 0, inv_p;
	float tmpV, tmp, val3 = 0;
    long val1, val2, maxV, maxI, minI, delta, refP;
	unsigned long advStepNo, saveDt;

	S_CH_STEP_INFO step;

	if(myData->mData.config.function[F_HW_FAULT_COND] == P0) return;

	if(myData->bData[bd].cData[ch].misc.parallel_cycle_phase == P50) {
		cFaultCondHard_P2(bd, ch); //kjg_180521
		return;
	}
	
	myCh = &(myData->bData[bd].cData[ch]);
	myTestCond = &(myPs->testCond[bd][ch]);

	if(myCh->op.state != C_RUN) return;
	if(myCh->misc.patternPhase > 10) return;
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

	if(type == STEP_REST) {
		rangeI = myCh->signal[C_SIG_I_RANGE] - 1;
		if(rangeI < RANGE0)
			rangeI = RANGE0;
	}

	maxV = myPs->config.maxVoltage[rangeV];
	maxI = myPs->config.maxCurrent[rangeI];
	minI = myPs->config.minCurrent[rangeI];

	if(labs(val2) > maxI * 0.1){ // 10%
		checkFlag = 1;
	}else{
#if CYCLER_TYPE == DIGITAL_CYC
		checkFlag = 1;
#else
		checkFlag = 2;
#endif
	}
	
	//20180410 add for CP mode	
	if(mode == CP || mode == CPCV) {
		refP = step.refP * 1000;
		if(type == STEP_CHARGE){
		//	val3 = (float)refP / (float)val1;
			val3 = (float)refP / (float)myCh->op.Vsens; //191031
		}else{
		//	val3 = ((float)refP / (float)myCh->misc.startV) * -1;
			val3 = ((float)refP / (float)myCh->op.Vsens) * -1; //191031
		} 
		val3 *= 1000000;
	}

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
		
	switch(type) {
		case STEP_REST:
			if(myCh->op.checkDelayTime < 180) break;
			if(labs(myCh->misc.tmpIsens) > maxI * 0.5) { 
				//case of op-amp control fail
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
			
#if CYCLER_TYPE == DIGITAL_CYC
			//180726 add for digital
			if(myCh->misc.errCnt[C_CNT_PCU_SEQ_FAULT]>= MAX_ERROR_CNT_SEQ){
				myCh->misc.errCnt[C_CNT_PCU_SEQ_FAULT] = 0;
				myCh->misc.tmpState = myCh->op.state;
				myCh->misc.tmpCode = myCh->op.code;
				myCh->op.code = P_CD_FAULT_SEQ_NO;
				rtn = FAULT_COND;
				break;
			}
			//180611 add for digital
			if(myCh->misc.errCnt[C_CNT_PCU_FAULT]>= MAX_ERROR_CNT_P){
				myCh->misc.errCnt[C_CNT_PCU_FAULT] = 0;
				myCh->misc.tmpState = myCh->op.state;
				rtn = FAULT_COND;
				break;
			}
#endif			
			break;
		case STEP_CHARGE:
			if(myCh->op.checkDelayTime < 180) break;
			if(checkFlag == 1){
				if(mode == CC || mode == CV || mode == CCCV || mode == C_RATE) {
					if((val1 - myCh->op.Vsens) > maxV * 0.001) {
						if(myCh->op.Isens < val2 * 0.6) {
							myCh->misc.errCnt[C_CNT_CONTACT_BAD]++;
							if(myCh->misc.errCnt[C_CNT_CONTACT_BAD]
								>= MAX_ERROR_CNT) {
								myCh->misc.errCnt[C_CNT_CONTACT_BAD] = 0;
								myCh->misc.tmpState = myCh->op.state;
								myCh->misc.tmpCode = myCh->op.code;
								if(myCh->op.checkDelayTime < 300) {
									myCh->op.code = C_FAULT_CH_I_FAIL;
								} else {
									myCh->op.code = C_FAULT_CH_V_FAIL;
								}
								rtn = FAULT_COND;
								break;
							}
						} else {
							myCh->misc.errCnt[C_CNT_CONTACT_BAD] = 0;
						}
					}
				}
			}else{
				if(mode == CC || mode == CV || mode == CCCV || mode == C_RATE) {
					if((val1 - myCh->op.Vsens) > maxV * 0.001) {
						if(myCh->misc.feedback_delayTime > 100 //1Sec
							|| myCh->op.checkDelayTime > 2000){ //20 Sec
							if(myCh->op.Isens < val2 * 0.6) {
								myCh->misc.errCnt[C_CNT_CONTACT_BAD]++;
								if(myCh->misc.errCnt[C_CNT_CONTACT_BAD]
									>= MAX_ERROR_CNT) {
									myCh->misc.errCnt[C_CNT_CONTACT_BAD] = 0;
									myCh->misc.tmpState = myCh->op.state;
									myCh->misc.tmpCode = myCh->op.code;
									if(myCh->op.checkDelayTime < 300) {
										myCh->op.code = C_FAULT_CH_I_FAIL;
									} else {
										myCh->op.code = C_FAULT_CH_V_FAIL;
									}
									rtn = FAULT_COND;
									break;
								}
							} else {
								myCh->misc.errCnt[C_CNT_CONTACT_BAD] = 0;
							}
						}
					}
				}
			}

			//20180410 add for Charge CP mode
			if(mode == CP || mode == CPCV) {
				if((val1 - myCh->op.Vsens) > maxV * 0.001) {
					if(myCh->op.Isens < val3 * 0.6) {
						myCh->misc.errCnt[C_CNT_CONTACT_BAD]++; //191031
						if(myCh->misc.errCnt[C_CNT_CONTACT_BAD]
							>= MAX_ERROR_CNT) {
							myCh->misc.errCnt[C_CNT_CONTACT_BAD] = 0;
							myCh->misc.tmpState = myCh->op.state;
							myCh->misc.tmpCode = myCh->op.code;
							if(myCh->op.checkDelayTime < 300) {
								myCh->op.code = C_FAULT_CH_I_FAIL;
							} else {
								myCh->op.code = C_FAULT_CH_V_FAIL;
							}
							rtn = FAULT_COND;
							break;
						}
					}else{
						myCh->misc.errCnt[C_CNT_CONTACT_BAD] = 0;
					}
				}
			}

			tmp = (float)(maxI*0.04);
			if((myCh->op.Isens - val2) >= (long)tmp) {
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

	//		tmp = (float)(maxV*0.02);
			tmp = (float)(maxV*0.01); //210621
			if((myCh->op.Vsens - val1) >= (long)tmp) {
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
#if FAULT_CONFIG_VERSION < 1	
			//20180627 sch add for Charge drop Voltage
			if(myData->mData.config.hwFaultConfig[HW_FAULT_DROP_V_1] != 0){
				if((myCh->misc.maxV - myCh->op.Vsens) 
					>= myData->mData.config.hwFaultConfig[HW_FAULT_DROP_V_1]){
					myCh->misc.errCnt[C_CNT_DROP_V]++;
					if(myCh->misc.errCnt[C_CNT_DROP_V] >= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_DROP_V] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_VOLTAGE_DROP;
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_DROP_V] = 0;
				}
			}
#endif

#if CYCLER_TYPE == DIGITAL_CYC
			//180726 add for digital
			if(myCh->misc.errCnt[C_CNT_PCU_SEQ_FAULT]>= MAX_ERROR_CNT_SEQ){
				myCh->misc.errCnt[C_CNT_PCU_SEQ_FAULT] = 0;
				myCh->misc.tmpState = myCh->op.state;
				myCh->misc.tmpCode = myCh->op.code;
				myCh->op.code = P_CD_FAULT_SEQ_NO;
				rtn = FAULT_COND;
				break;
			}
			//180611 add for digital
			if(myCh->misc.errCnt[C_CNT_PCU_FAULT]>= MAX_ERROR_CNT_P){
				myCh->misc.errCnt[C_CNT_PCU_FAULT] = 0;
				myCh->misc.tmpState = myCh->op.state;
				rtn = FAULT_COND;
				break;
			}
#endif
			//20171123 sch for NoCell
			if(myCh->op.runTime < 300){
				tmpV = (float)(myCh->misc.startV);
				if(abs(myCh->op.Vsens - (long)tmpV) >= val1*0.8) {
					myCh->misc.tmpState = myCh->op.state;
					myCh->misc.tmpCode = myCh->op.code;
					myCh->op.code = C_FAULT_CH_V_FAIL;
					rtn = FAULT_COND;
					break;
				}else{
				}
			}
#if CAPACITY_CONTROL == 1
			//210402 c_rate calculation error [Single Charge]
			if(myCh->misc.c_rate_fault_flag[advStepNo] == P1){ 
				myCh->misc.tmpState = myCh->op.state;
				myCh->misc.tmpCode = myCh->op.code;
				myCh->op.code = C_FAULT_C_RATE_CALC_ERROR;
				rtn = FAULT_COND;
				break;
			}
#endif
#ifdef _EQUATION_CURRENT	//211111
			if(myCh->misc.equation_calc_err_flag == P1){
				myCh->misc.equation_calc_err_flag = P0;
				myCh->misc.tmpState = myCh->op.state;
				myCh->misc.tmpCode = myCh->op.code;
				myCh->op.code = C_FAULT_EQUATION_CALC_ERROR;
				rtn = FAULT_COND;
				break;
			}
#endif
			
			rtn = SAMSUNG_SDI_safety(bd, ch, rtn, type);
			//210204 charge lyhw
			rtn = cvFault_Check(bd, ch, rtn, type, val1);

			break;
		case STEP_DISCHARGE:
			if(myCh->op.checkDelayTime < 180) break;
			if(checkFlag == 1){
				if(mode == CC
					|| mode == CV
					|| mode == CCCV
					|| mode == C_RATE){
					if((myCh->op.Vsens - val1) > maxV * 0.001){
						if(myCh->op.Isens > val2 * 0.6) {
							myCh->misc.errCnt[C_CNT_CONTACT_BAD]++;
							if(myCh->misc.errCnt[C_CNT_CONTACT_BAD]>= MAX_ERROR_CNT){
								myCh->misc.errCnt[C_CNT_CONTACT_BAD] = 0;
								myCh->misc.tmpState = myCh->op.state;
								myCh->misc.tmpCode = myCh->op.code;
								//100908 pjy
								if(myCh->op.checkDelayTime < 300) {
									myCh->op.code = C_FAULT_CH_I_FAIL;
								} else {
									myCh->op.code = C_FAULT_CH_V_FAIL;
								}
								rtn = FAULT_COND;
								break;
							}
						}else{
							myCh->misc.errCnt[C_CNT_CONTACT_BAD] = 0;
						}
					}
				}
			}else{
				if(mode == CC
					|| mode == CV
					|| mode == CCCV
					|| mode == C_RATE){
					if((myCh->op.Vsens - val1) > maxV * 0.001){
						if(myCh->misc.feedback_delayTime > 100 //1Sec
							|| myCh->op.checkDelayTime > 2000){// 20Sec
							if(myCh->op.Isens > val2 * 0.6) {
								myCh->misc.errCnt[C_CNT_CONTACT_BAD]++;
								if(myCh->misc.errCnt[C_CNT_CONTACT_BAD]>= MAX_ERROR_CNT){
									myCh->misc.errCnt[C_CNT_CONTACT_BAD] = 0;
									myCh->misc.tmpState = myCh->op.state;
									myCh->misc.tmpCode = myCh->op.code;
									//100908 pjy
									if(myCh->op.checkDelayTime < 300) {
										myCh->op.code = C_FAULT_CH_I_FAIL;
									} else {
										myCh->op.code = C_FAULT_CH_V_FAIL;
									}
									rtn = FAULT_COND;
									break;
								}
							}else{
								myCh->misc.errCnt[C_CNT_CONTACT_BAD] = 0;
							}
						}
					}
				}
			}

			//20180410 add for Discharge CP mode
			if(mode == CP || mode == CPCV) {
				if((myCh->op.Vsens - val1) > maxV * 0.001) {
					if(myCh->op.Isens > val3 * 0.6) {
						myCh->misc.errCnt[C_CNT_CONTACT_BAD]++; //191031
						if(myCh->misc.errCnt[C_CNT_CONTACT_BAD]
							>= MAX_ERROR_CNT) {
							myCh->misc.errCnt[C_CNT_CONTACT_BAD] = 0;
							myCh->misc.tmpState = myCh->op.state;
							myCh->misc.tmpCode = myCh->op.code;
							if(myCh->op.checkDelayTime < 300) {
								myCh->op.code = C_FAULT_CH_I_FAIL;
							} else {
								myCh->op.code = C_FAULT_CH_V_FAIL;
							}
							rtn = FAULT_COND;
							break;
						}
					}else{
						myCh->misc.errCnt[C_CNT_CONTACT_BAD] = 0;
					}
				}
			}
			tmp = (float)(maxI*0.04);
			if((val2 - myCh->op.Isens) >= (long)tmp) {
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

//			tmp = (float)(maxV*0.02);
			tmp = (float)(maxV*0.01); //210621
			if((val1 - myCh->op.Vsens) >= (long)tmp) {
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
#if FAULT_CONFIG_VERSION < 1	
			//20180627 sch add for DisCharge drop Voltage
			if(myData->mData.config.hwFaultConfig[HW_FAULT_DROP_V_2] != 0){
				if((myCh->op.Vsens - myCh->misc.minV) 
					>= myData->mData.config.hwFaultConfig[HW_FAULT_DROP_V_2]){
					myCh->misc.errCnt[C_CNT_DROP_V]++;
					if(myCh->misc.errCnt[C_CNT_DROP_V] >= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_DROP_V] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_VOLTAGE_DROP;
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_DROP_V] = 0;
				}
			}
#endif
#if CYCLER_TYPE == DIGITAL_CYC
			//180726 add for digital
			if(myCh->misc.errCnt[C_CNT_PCU_SEQ_FAULT]>= MAX_ERROR_CNT_SEQ){
				myCh->misc.errCnt[C_CNT_PCU_SEQ_FAULT] = 0;
				myCh->misc.tmpState = myCh->op.state;
				myCh->misc.tmpCode = myCh->op.code;
				myCh->op.code = P_CD_FAULT_SEQ_NO;
				rtn = FAULT_COND;
				break;
			}
			//180611 add for digital
			if(myCh->misc.errCnt[C_CNT_PCU_FAULT]>= MAX_ERROR_CNT_P){
				myCh->misc.errCnt[C_CNT_PCU_FAULT] = 0;
				myCh->misc.tmpState = myCh->op.state;
				rtn = FAULT_COND;
				break;
			}
#endif
#if CAPACITY_CONTROL == 1
			//210402 c_rate calculation error [Single DisCharge]
			if(myCh->misc.c_rate_fault_flag[advStepNo] == P1){ 
				myCh->misc.tmpState = myCh->op.state;
				myCh->misc.tmpCode = myCh->op.code;
				myCh->op.code = C_FAULT_C_RATE_CALC_ERROR;
				rtn = FAULT_COND;
				break;
			}
#endif
			
			rtn = SAMSUNG_SDI_safety(bd, ch, rtn, type);
			//210204 discharge lyhw
			rtn = cvFault_Check(bd, ch, rtn, type, val1);

			break;
		case STEP_Z: 
			if(myCh->op.checkDelayTime < 180) break;
			tmp = (float)(maxI*0.04);
			if((val2 - myCh->op.Isens) >= (long)tmp) {
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

//			tmp = (float)(maxV*0.02);
			tmp = (float)(maxV*0.01); //210621
			if((val1 - myCh->op.Vsens) >= (long)tmp) {
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
		
#if CYCLER_TYPE == DIGITAL_CYC
			//180726 add for digital
			if(myCh->misc.errCnt[C_CNT_PCU_SEQ_FAULT]>= MAX_ERROR_CNT_SEQ){
				myCh->misc.errCnt[C_CNT_PCU_SEQ_FAULT] = 0;
				myCh->misc.tmpState = myCh->op.state;
				myCh->misc.tmpCode = myCh->op.code;
				myCh->op.code = P_CD_FAULT_SEQ_NO;
				rtn = FAULT_COND;
				break;
			}
			//180611 add for digital
			if(myCh->misc.errCnt[C_CNT_PCU_FAULT]>= MAX_ERROR_CNT_P){
				myCh->misc.errCnt[C_CNT_PCU_FAULT] = 0;
				myCh->misc.tmpState = myCh->op.state;
				rtn = FAULT_COND;
				break;
			}
#endif
			break;
		case STEP_USER_PATTERN:
			if(myCh->op.checkDelayTime < 180) break;
/*
			if(val2 > 0){
				delta = myCh->misc.tmpIsens - val2;
			}else if(val2 < 0){
				delta = val2 - myCh->misc.tmpIsens;
			}else {
				delta = maxI;
			}
			if(delta >= (long)(maxI*0.04)) {
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
			*/
			if(mode == CC) {
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
				
				if(val2 > 0){
					delta = myCh->op.Vsens - val1;
				}else if(val2 < 0){
					delta = val1 - myCh->op.Vsens;
				}else {
					delta = 0;
				}
//				if(delta >= (long)(maxV * 0.02)) {
				if(delta >= (long)(maxV * 0.01)) { //210621
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
			
#if CYCLER_TYPE == DIGITAL_CYC
			//180726 add for digital
			if(myCh->misc.errCnt[C_CNT_PCU_SEQ_FAULT]>= MAX_ERROR_CNT_SEQ){
				myCh->misc.errCnt[C_CNT_PCU_SEQ_FAULT] = 0;
				myCh->misc.tmpState = myCh->op.state;
				myCh->misc.tmpCode = myCh->op.code;
				myCh->op.code = P_CD_FAULT_SEQ_NO;
				rtn = FAULT_COND;
				break;
			}	
			//180611 add for digital
			if(myCh->misc.errCnt[C_CNT_PCU_FAULT]>= MAX_ERROR_CNT_P){
				myCh->misc.errCnt[C_CNT_PCU_FAULT] = 0;
				myCh->misc.tmpState = myCh->op.state;
				rtn = FAULT_COND;
				break;
			}
#endif
			break;
		case STEP_USER_MAP:
			if(myCh->op.checkDelayTime < 180) break;
			tmp = (float)(maxI*0.04);
			if((labs(myCh->op.Isens) - labs(val2)) >= (long)tmp
				&& val2 != 0) {		
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
			
			if(val2 > 0){
				delta = myCh->op.Vsens - val1;
			}else if(val2 < 0){
				delta = val1 - myCh->op.Vsens;
			}else{
				delta = 0;
			}
//			if(delta >= (long)(maxV * 0.02)) {
			if(delta >= (long)(maxV * 0.01)) { //210621
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
			
#if CYCLER_TYPE == DIGITAL_CYC
			//180726 add for digital
			if(myCh->misc.errCnt[C_CNT_PCU_SEQ_FAULT]>= MAX_ERROR_CNT_SEQ){
				myCh->misc.errCnt[C_CNT_PCU_SEQ_FAULT] = 0;
				myCh->misc.tmpState = myCh->op.state;
				myCh->misc.tmpCode = myCh->op.code;
				myCh->op.code = P_CD_FAULT_SEQ_NO;
				rtn = FAULT_COND;
				break;
			}
			//180611 add for digital
			if(myCh->misc.errCnt[C_CNT_PCU_FAULT]>= MAX_ERROR_CNT_P){
				myCh->misc.errCnt[C_CNT_PCU_FAULT] = 0;
				myCh->misc.tmpState = myCh->op.state;
				rtn = FAULT_COND;
				break;
			}
#endif
			break;
		default: 
			break;
	}
	
	//210126 lyhw
	if(myData->AppControl.config.systemType == CYCLER_CAN){
		rtn = errorCodeCheck_CAN(bd, ch, rtn);
		
	}
	rtn = SDI_CC_CV_hump_Check(bd, ch, rtn, type);

	if(rtn == FAULT_COND) { //fault
		myCh->op.select = SAVE_FLAG_SAVING_ETC;
		send_save_msg(bd, ch, saveDt, 0);
		//170106 oys add
		if(myCh->misc.tempWaitType == P0){
			myCh->misc.chGroupNo = 0;
			//20180417 add
			myCh->misc.stepSyncFlag = P0;
			myCh->misc.endState = P0;		//220203_hun
			myCh->misc.groupAvgVsens = 0;	//220203_hun
			myCh->misc.group_StartVoltage_flag = 0;
		}
		//190318 lyhw
		if(myData->mData.config.parallelMode == P2) { 
			cCycle_p_ch_check(bd, ch);
		}
		myCh->op.phase = P100;
		cNextStepCheck(bd, ch);
	}else if(rtn == 1){	//200504 hun add
		myCh->op.select = SAVE_FLAG_SAVING_ETC;
		send_save_msg(bd, ch, saveDt, 0);
	}
}

void cFaultCondHard_P(int bd, int ch)
{
	unsigned char type, mode, checkFlag;
	int rangeV, rangeI, rtn = 0, inv_p;
	float tmpV, tmp, val3 = 0;
    long val1, val2, maxV, maxI, minI, delta, m_Delta, refP;
	long m_opIsens, m_tmpIsens, m_Vsens, m_Temp;
	unsigned long advStepNo, saveDt;

	S_CH_STEP_INFO step;

	if(myData->mData.config.function[F_HW_FAULT_COND] == P0) return;

	myCh = &(myData->bData[bd].cData[ch]);
	myTestCond = &(myPs->testCond[bd][ch]);

	if(myCh->op.state != C_RUN) return;
	if(myCh->misc.patternPhase != 0) return;
	if(myCh->op.phase != P50) return;
	
	//master Temp
	m_Temp = myData->bData[bd].cData[ch-1].op.temp;
	//master Vsens
	m_Vsens = myData->bData[bd].cData[ch-1].op.Vsens;
	//Isens = master Isens + slave Isens
	m_tmpIsens = myData->bData[bd].cData[ch-1].misc.tmpIsens;
	m_opIsens =  myData->bData[bd].cData[ch-1].op.Isens;
				
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

	if(type == STEP_REST) {
		rangeI = myCh->signal[C_SIG_I_RANGE] - 1;
		if(rangeI < RANGE0) rangeI = RANGE0;
	}
	maxV = myPs->config.maxVoltage[rangeV];
	maxI = myPs->config.maxCurrent[rangeI];
	minI = myPs->config.minCurrent[rangeI];

	if(labs(val2) >= maxI * 0.1) { //10%
		checkFlag = 1;
	} else {
#if CYCLER_TYPE == DIGITAL_CYC	//180903 add for Digital Fault
		checkFlag = 1;
#else
		checkFlag = 2;
#endif
	}

	//20180410 add for CP mode
	if(mode == CP || mode == CPCV) {
		refP = step.refP * 1000;
		if(type == STEP_CHARGE) {
	//		val3 = (float)refP / (float)val1;
			val3 = (float)refP / (float)m_Vsens; //191031
		} else {
	//		val3 = ((float)refP / (float)myCh->misc.startV) * (-1);
			val3 = ((float)refP / (float)m_Vsens) * (-1);
		} 
		val3 *= 1000000;
	}
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
	if(m_opIsens > myData->bData[bd].cData[ch-1].misc.maxI) {
		myData->bData[bd].cData[ch-1].misc.maxI = m_opIsens;
	//	myCh->misc.maxI = m_opIsens;
	}
	if(m_opIsens < myData->bData[bd].cData[ch-1].misc.minI) {
		myData->bData[bd].cData[ch-1].misc.minI = m_opIsens;
	//	myCh->misc.minI = m_opIsens;
	}
	//210204 lyhw
	if(myCh->op.Isens > myCh->misc.maxI) {
		myCh->misc.maxI = myCh->op.Isens;
	}
	if(myCh->op.Isens < myCh->misc.minI) {
		myCh->misc.minI = myCh->op.Isens;
	}

	switch(type) {
		case STEP_REST:
			if(myCh->op.checkDelayTime < 180) break;

			if((labs(myCh->misc.tmpIsens) > maxI * 0.5)
				|| (labs(m_tmpIsens) > maxI * 0.5)) { 
				//case of op-amp control fail
				myCh->misc.errCnt[C_CNT_CH_I_FAIL]++;
				if(myCh->misc.errCnt[C_CNT_CH_I_FAIL]>= MAX_ERROR_CNT) {
					myCh->misc.errCnt[C_CNT_CH_I_FAIL] = 0;
					myCh->misc.tmpState = myCh->op.state;
					myCh->misc.tmpCode = myCh->op.code;
					myCh->op.code = C_FAULT_CH_I_FAIL;
					rtn = FAULT_COND;
					break;
				}
			} else {
				myCh->misc.errCnt[C_CNT_CH_I_FAIL] = 0;
			}
			
#if CYCLER_TYPE == DIGITAL_CYC
			//180726 add for digital
			if(myCh->misc.errCnt[C_CNT_PCU_SEQ_FAULT]>= MAX_ERROR_CNT_SEQ){
				myCh->misc.errCnt[C_CNT_PCU_SEQ_FAULT] = 0;
				myCh->misc.tmpState = myCh->op.state;
				myCh->misc.tmpCode = myCh->op.code;
				myCh->op.code = P_CD_FAULT_SEQ_NO;
				rtn = FAULT_COND;
				break;
			}
			//180611 add for digital
			if(myCh->misc.errCnt[C_CNT_PCU_FAULT]>= MAX_ERROR_CNT_P){
				myCh->misc.errCnt[C_CNT_PCU_FAULT] = 0;
				myCh->misc.tmpState = myCh->op.state;
				rtn = FAULT_COND;
				break;
			}
#endif
			break;
		case STEP_CHARGE:
			if(myCh->op.checkDelayTime < 180) break;

			if(checkFlag == 1) {
				if(mode == CC || mode == CV || mode == CCCV || mode == C_RATE) {
					if(((val1 - myCh->op.Vsens) > maxV * 0.001)
						|| ((val1 - m_Vsens) > maxV * 0.001)) {
						if((myCh->op.Isens < val2 * 0.6)
							|| (m_opIsens < val2 * 0.6)) {
							myCh->misc.errCnt[C_CNT_CONTACT_BAD]++;
							if(myCh->misc.errCnt[C_CNT_CONTACT_BAD]
								>= MAX_ERROR_CNT) {
								myCh->misc.errCnt[C_CNT_CONTACT_BAD] = 0;
								myCh->misc.tmpState = myCh->op.state;
								myCh->misc.tmpCode = myCh->op.code;
								//100908 pjy
								if(myCh->op.checkDelayTime < 300) {
									myCh->op.code = C_FAULT_CH_I_FAIL;
								} else {
									myCh->op.code = C_FAULT_CH_V_FAIL;
								}
								rtn = FAULT_COND;
								break;
							}
						} else {
							myCh->misc.errCnt[C_CNT_CONTACT_BAD] = 0;
						}
					}
				}
			} else {
				if(mode == CC || mode == CV || mode == CCCV || C_RATE) {
					if(((val1 - myCh->op.Vsens) > maxV * 0.001)
						|| ((val1 - m_Vsens) > maxV * 0.001)) {
						if(myCh->misc.feedback_delayTime > 100 //1Sec
							|| myCh->op.checkDelayTime > 2000) { //20Sec
							if((myCh->op.Isens < val2 * 0.6)
								|| (m_opIsens < val2 * 0.6)) {
								myCh->misc.errCnt[C_CNT_CONTACT_BAD]++;
								if(myCh->misc.errCnt[C_CNT_CONTACT_BAD]
									>= MAX_ERROR_CNT) {
									myCh->misc.errCnt[C_CNT_CONTACT_BAD] = 0;
									myCh->misc.tmpState = myCh->op.state;
									myCh->misc.tmpCode = myCh->op.code;
									//100908 pjy
									if(myCh->op.checkDelayTime < 300) {
										myCh->op.code = C_FAULT_CH_I_FAIL;
									} else {
										myCh->op.code = C_FAULT_CH_V_FAIL;
									}
									rtn = FAULT_COND;
									break;
								}
							} else {
								myCh->misc.errCnt[C_CNT_CONTACT_BAD] = 0;
							}
						}
					}
				}
			}

			//20180410 add for parallel Charge CP mode
			if(mode == CP || mode == CPCV) {
				if(((val1 - myCh->op.Vsens) > maxV * 0.001)
					|| ((val1 - m_Vsens) > maxV * 0.001)) {
					if((myCh->op.Isens < val3 * 0.6)
						|| (m_opIsens < val3 *0.6)) {
						myCh->misc.errCnt[C_CNT_CONTACT_BAD]++; //191031
						if(myCh->misc.errCnt[C_CNT_CONTACT_BAD]
							>= MAX_ERROR_CNT) {
							myCh->misc.errCnt[C_CNT_CONTACT_BAD] = 0;
							myCh->misc.tmpState = myCh->op.state;
							myCh->misc.tmpCode = myCh->op.code;
							if(myCh->op.checkDelayTime < 300) {
								myCh->op.code = C_FAULT_CH_I_FAIL;
							} else {
								myCh->op.code = C_FAULT_CH_V_FAIL;
							}
							rtn = FAULT_COND;
							break;
						}
					} else {
						myCh->misc.errCnt[C_CNT_CONTACT_BAD] = 0;
					}
				}
			}

			tmp = (float)(maxI * 0.02);
			if(((myCh->op.Isens - (val2)) >= (long)tmp)
				|| ((m_opIsens - (val2)) >= (long)tmp)) {
				myCh->misc.errCnt[C_CNT_CH_I_FAIL]++;
				if(myCh->misc.errCnt[C_CNT_CH_I_FAIL] >= MAX_ERROR_CNT) {
					myCh->misc.errCnt[C_CNT_CH_I_FAIL] = 0;
					myCh->misc.tmpState = myCh->op.state;
					myCh->misc.tmpCode = myCh->op.code;
					myCh->op.code = C_FAULT_CH_I_FAIL;
					rtn = FAULT_COND;
					break;
				}
			} else {
				myCh->misc.errCnt[C_CNT_CH_I_FAIL] = 0;
			}

//			tmp = (float)(maxV * 0.02);
			tmp = (float)(maxV * 0.01); //210621
			if(((myCh->op.Vsens - val1) >= (long)tmp)
				|| ((m_Vsens - val1) >= (long)tmp)) {
				myCh->misc.errCnt[C_CNT_CH_V_FAIL]++;
				if(myCh->misc.errCnt[C_CNT_CH_V_FAIL] >= MAX_ERROR_CNT) {
					myCh->misc.errCnt[C_CNT_CH_V_FAIL] = 0;
					myCh->misc.tmpState = myCh->op.state;
					myCh->misc.tmpCode = myCh->op.code;
					myCh->op.code = C_FAULT_CH_V_FAIL;
					rtn = FAULT_COND;
					break;
				}
			} else {
				myCh->misc.errCnt[C_CNT_CH_V_FAIL] = 0;
			}
#if FAULT_CONFIG_VERSION < 1	
			//20180627 sch add for Charge drop Voltage for parallel
			if(myData->mData.config.hwFaultConfig[HW_FAULT_DROP_V_1] != 0){
				if((myCh->misc.maxV - m_Vsens) 
					>= myData->mData.config.hwFaultConfig[HW_FAULT_DROP_V_1]){
					myCh->misc.errCnt[C_CNT_DROP_V]++;
					if(myCh->misc.errCnt[C_CNT_DROP_V] >= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_DROP_V] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_VOLTAGE_DROP;
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_DROP_V] = 0;
				}
			}
#endif
#if CYCLER_TYPE == DIGITAL_CYC
			//180726 add for digital
			if(myCh->misc.errCnt[C_CNT_PCU_SEQ_FAULT]>= MAX_ERROR_CNT_SEQ){
				myCh->misc.errCnt[C_CNT_PCU_SEQ_FAULT] = 0;
				myCh->misc.tmpState = myCh->op.state;
				myCh->misc.tmpCode = myCh->op.code;
				myCh->op.code = P_CD_FAULT_SEQ_NO;
				rtn = FAULT_COND;
				break;
			}
			//180611 add for digital
			if(myCh->misc.errCnt[C_CNT_PCU_FAULT]>= MAX_ERROR_CNT_P){
				myCh->misc.errCnt[C_CNT_PCU_FAULT] = 0;
				myCh->misc.tmpState = myCh->op.state;
				rtn = FAULT_COND;
				break;
			}
#endif
			//20171123 sch for NoCell
			if(myCh->op.runTime < 300) {
				tmpV = (float)(myCh->misc.startV);
				if((abs(myCh->op.Vsens - (long)tmpV) >= (val1 * 0.8))
					|| (abs(m_Vsens - (long)tmpV) >= (val1 * 0.8))) {
					myCh->misc.tmpState = myCh->op.state;
					myCh->misc.tmpCode = myCh->op.code;
					myCh->op.code = C_FAULT_CH_V_FAIL;
					rtn = FAULT_COND;
					break;
				} else {
				}
			}
#if CAPACITY_CONTROL == 1
			//210402 c_rate calculation error [Parallel Charge]
			if(myCh->misc.c_rate_fault_flag[advStepNo] == P1){ 
				myCh->misc.tmpState = myCh->op.state;
				myCh->misc.tmpCode = myCh->op.code;
				myCh->op.code = C_FAULT_C_RATE_CALC_ERROR;
				rtn = FAULT_COND;
				break;
			}
#endif
			
			rtn = SAMSUNG_SDI_safety(bd, ch, rtn, type);
			//210204 charge parallel lyhw
			rtn = cvFault_Check(bd, ch, rtn, type, val1);

			break;
		case STEP_DISCHARGE:
			if(myCh->op.checkDelayTime < 180) break;

			if(checkFlag == 1) {
				if(mode == CC || mode == CV || mode == CCCV || C_RATE) {
					if(((myCh->op.Vsens - val1) > maxV * 0.001) || 
						((m_Vsens - val1) > maxV * 0.001)){
						if((myCh->op.Isens > val2 * 0.6) ||
							(m_opIsens > val2 * 0.6)) {
							myCh->misc.errCnt[C_CNT_CONTACT_BAD]++;
							if(myCh->misc.errCnt[C_CNT_CONTACT_BAD]>= MAX_ERROR_CNT){
								myCh->misc.errCnt[C_CNT_CONTACT_BAD] = 0;
								myCh->misc.tmpState = myCh->op.state;
								myCh->misc.tmpCode = myCh->op.code;
								//100908 pjy
								if(myCh->op.checkDelayTime < 300) {
									myCh->op.code = C_FAULT_CH_I_FAIL;
								} else {
									myCh->op.code = C_FAULT_CH_V_FAIL;
								}
								rtn = FAULT_COND;
								break;
							}
						}else{
							myCh->misc.errCnt[C_CNT_CONTACT_BAD] = 0;
						}
					}
				}
			}else{
				if(mode == CC
					|| mode == CV
					|| mode == CCCV
					|| mode == C_RATE){
					if(((myCh->op.Vsens - val1) > maxV * 0.001) || 
						((m_Vsens - val1) > maxV * 0.001)){
						if(myCh->misc.feedback_delayTime > 100 //1Sec
							|| myCh->op.checkDelayTime > 2000){// 20Sec
							if((myCh->op.Isens > val2 * 0.6) ||
								(m_opIsens > val2 * 0.6)) {
								myCh->misc.errCnt[C_CNT_CONTACT_BAD]++;
								if(myCh->misc.errCnt[C_CNT_CONTACT_BAD]>= MAX_ERROR_CNT){
									myCh->misc.errCnt[C_CNT_CONTACT_BAD] = 0;
									myCh->misc.tmpState = myCh->op.state;
									myCh->misc.tmpCode = myCh->op.code;
									//100908 pjy
									if(myCh->op.checkDelayTime < 300) {
										myCh->op.code = C_FAULT_CH_I_FAIL;
									} else {
										myCh->op.code = C_FAULT_CH_V_FAIL;
									}
									rtn = FAULT_COND;
									break;
								}
							}else{
								myCh->misc.errCnt[C_CNT_CONTACT_BAD] = 0;
							}
						}
					}
				}
			}
		
			//20180410 add for parallel Discharge CP mode
			if(mode == CP || mode == CPCV) {
				if(((myCh->op.Vsens - val1) > maxV * 0.001) ||
					((m_Vsens - val1) > maxV * 0.001)) {
					if((myCh->op.Isens > val3 * 0.6) ||
						(m_opIsens > val3 *0.6)) {
						myCh->misc.errCnt[C_CNT_CONTACT_BAD]++; //191031
						if(myCh->misc.errCnt[C_CNT_CONTACT_BAD]
							>= MAX_ERROR_CNT) {
							myCh->misc.errCnt[C_CNT_CONTACT_BAD] = 0;
							myCh->misc.tmpState = myCh->op.state;
							myCh->misc.tmpCode = myCh->op.code;
							if(myCh->op.checkDelayTime < 300) {
								myCh->op.code = C_FAULT_CH_I_FAIL;
							} else {
								myCh->op.code = C_FAULT_CH_V_FAIL;
							}
							rtn = FAULT_COND;
							break;
						}
					}else{
						myCh->misc.errCnt[C_CNT_CONTACT_BAD] = 0;
					}
				}
			}

			tmp = (float)(maxI*0.02);
			if(((val2 - myCh->op.Isens) >= (long)tmp) ||
				((val2 - m_opIsens) >= (long)tmp)) {
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

//			tmp = (float)(maxV*0.02);
			tmp = (float)(maxV * 0.01); //210621
			if(((val1 - myCh->op.Vsens) >= (long)tmp) ||
				((val1 - m_Vsens) >= (long)tmp)) {
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
#if FAULT_CONFIG_VERSION < 1	
			//20180627 sch add for DisCharge drop Voltage Paralllel
			if(myData->mData.config.hwFaultConfig[HW_FAULT_DROP_V_2] != 0){
				if((m_Vsens - myCh->misc.minV)
					>= myData->mData.config.hwFaultConfig[HW_FAULT_DROP_V_2]){
					myCh->misc.errCnt[C_CNT_DROP_V]++;
					if(myCh->misc.errCnt[C_CNT_DROP_V] >= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_DROP_V] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_VOLTAGE_DROP;
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_DROP_V] = 0;
				}
			}
#endif
#if CYCLER_TYPE == DIGITAL_CYC
			//180726 add for digital
			if(myCh->misc.errCnt[C_CNT_PCU_SEQ_FAULT]>= MAX_ERROR_CNT_SEQ){
				myCh->misc.errCnt[C_CNT_PCU_SEQ_FAULT] = 0;
				myCh->misc.tmpState = myCh->op.state;
				myCh->misc.tmpCode = myCh->op.code;
				myCh->op.code = P_CD_FAULT_SEQ_NO;
				rtn = FAULT_COND;
				break;
			}
			//180611 add for digital
			if(myCh->misc.errCnt[C_CNT_PCU_FAULT]>= MAX_ERROR_CNT_P){
				myCh->misc.errCnt[C_CNT_PCU_FAULT] = 0;
				myCh->misc.tmpState = myCh->op.state;
				rtn = FAULT_COND;
				break;
			}
#endif
#if CAPACITY_CONTROL == 1
			//210308 c_rate calculation error [Parallel DisCharge]
			if(myCh->misc.c_rate_fault_flag[advStepNo] == P1){ 
				myCh->misc.tmpState = myCh->op.state;
				myCh->misc.tmpCode = myCh->op.code;
				myCh->op.code = C_FAULT_C_RATE_CALC_ERROR;
				rtn = FAULT_COND;
				break;
			}
#endif		
			rtn = SAMSUNG_SDI_safety(bd, ch, rtn, type);
			//210204 discharge Parallel lyhw
			rtn = cvFault_Check(bd, ch, rtn, type, val1);

			break;
		case STEP_Z: 
			if(myCh->op.checkDelayTime < 180) break;
			tmp = (float)(maxI*0.08);
			if(((val2 - myCh->op.Isens) >= (long)tmp) ||
				((val2 - m_opIsens) >= (long)tmp)) {
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

	//		tmp = (float)(maxV*0.02);
			tmp = (float)(maxV*0.01); //210618
			if(((val1 - myCh->op.Vsens) >= (long)tmp) ||
				((val1 - m_Vsens) >= (long)tmp)) {
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
			
#if CYCLER_TYPE == DIGITAL_CYC
			//180726 add for digital
			if(myCh->misc.errCnt[C_CNT_PCU_SEQ_FAULT]>= MAX_ERROR_CNT_SEQ){
				myCh->misc.errCnt[C_CNT_PCU_SEQ_FAULT] = 0;
				myCh->misc.tmpState = myCh->op.state;
				myCh->misc.tmpCode = myCh->op.code;
				myCh->op.code = P_CD_FAULT_SEQ_NO;
				rtn = FAULT_COND;
				break;
			}			
			//180611 add for digital
			if(myCh->misc.errCnt[C_CNT_PCU_FAULT]>= MAX_ERROR_CNT_P){
				myCh->misc.errCnt[C_CNT_PCU_FAULT] = 0;
				myCh->misc.tmpState = myCh->op.state;
				rtn = FAULT_COND;
				break;
			}
#endif
			break;
		case STEP_USER_PATTERN:
			if(myCh->op.checkDelayTime < 180) break;
			//20180314 sch modify
			if(mode == CC || mode == CCCV || mode == C_RATE) {
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
				/*if(val2 != 0){
					delta = labs(myCh->misc.tmpIsens) - labs(val2);
					m_Delta = labs(m_tmpIsens) - labs(val2);
				}else{
					delta = 0;
					m_Delta = 0;
				}
				*/
			
				if(((val1 - myCh->op.Vsens) > maxV * 0.001) ||
					((val1 - m_Vsens) > maxV * 0.001)){
					if((labs(delta) >= (long)(maxI*0.04)) ||
						(labs(m_Delta) >= (long)(maxI*0.04))) {
						myCh->misc.errCnt[C_CNT_CH_I_FAIL]++;
					//	if(myCh->misc.errCnt[C_CNT_CH_I_FAIL]>= MAX_ERROR_CNT){
						if(myCh->misc.errCnt[C_CNT_CH_I_FAIL] >= 10){
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
//				if((delta >= (long)(maxV * 0.02)) ||
//					(m_Delta >= (long)(maxV * 0.02))) {				
				if((delta >= (long)(maxV * 0.01)) ||
					(m_Delta >= (long)(maxV * 0.01))) { //210621
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
			
#if CYCLER_TYPE == DIGITAL_CYC
			//180726 add for digital
			if(myCh->misc.errCnt[C_CNT_PCU_SEQ_FAULT]>= MAX_ERROR_CNT_SEQ){
				myCh->misc.errCnt[C_CNT_PCU_SEQ_FAULT] = 0;
				myCh->misc.tmpState = myCh->op.state;
				myCh->misc.tmpCode = myCh->op.code;
				myCh->op.code = P_CD_FAULT_SEQ_NO;
				rtn = FAULT_COND;
				break;
			}
			//180611 add for digital
			if(myCh->misc.errCnt[C_CNT_PCU_FAULT]>= MAX_ERROR_CNT_P){
				myCh->misc.errCnt[C_CNT_PCU_FAULT] = 0;
				myCh->misc.tmpState = myCh->op.state;
				rtn = FAULT_COND;
				break;
			}
#endif
			break;
		default: 
			break;
	}
		
	rtn = SDI_CC_CV_hump_Check(bd, ch, rtn, type);

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
			myData->bData[bd].cData[ch-1].misc.endState = 0;		//220223_hun
			myData->bData[bd].cData[ch-1].misc.groupAvgVsens = 0; 	//220223_hun
			myData->bData[bd].cData[ch-1].misc.group_StartVoltage_flag = 0;
		}
		myCh->op.phase = P100;
		cNextStepCheck(bd, ch-1);
		myCh = &(myData->bData[bd].cData[ch]);
		cNextStepCheck(bd, ch);
	}else if(rtn == 1){	//200504 hun add
		myCh->op.select = SAVE_FLAG_SAVING_ETC;
		send_save_msg(bd, ch, saveDt, 0);
	}
}

void cFaultCondHard_P2(int bd, int ch)
{ //kjg_180521
	unsigned char type, mode, checkFlag;
	int rangeV, rangeI, rtn = 0, parallel_ch;
	float tmp, val3 = 0;
    long val1, val2, maxV, maxI, minI, delta, refP;
	unsigned long advStepNo, saveDt;
	S_CH_STEP_INFO step;

	if((ch % 2) == 0) parallel_ch = ch + 1;
	else parallel_ch = ch - 1;

	myCh = &(myData->bData[bd].cData[ch]);

	if(myCh->op.state != C_RUN) return;
	if(myCh->misc.patternPhase > 10) return;
	if(myCh->op.phase != P50) return;

	step = step_info(bd, ch);

	advStepNo = step.advStepNo;
	rangeV = step.rangeV;
	rangeI = step.rangeI;
	saveDt = step.saveDt;
	val1 = step.refV;
	val2 = step.refI * 2;
	type = step.type;
	mode = step.mode;

	if(type == STEP_REST) {
		rangeI = myCh->signal[C_SIG_I_RANGE] - 1;
		if(rangeI < RANGE0) rangeI = RANGE0;
	}

	maxV = myPs->config.maxVoltage[rangeV];
	maxI = myPs->config.maxCurrent[rangeI] * 2;
	minI = myPs->config.minCurrent[rangeI] * 2;

	if(labs(val2) > (maxI * 0.1)) { //10%
		checkFlag = 1;
	} else {
		checkFlag = 2;
	}
	
	//20180410 add for CP mode	
	if(mode == CP || mode == CPCV) {
		refP = step.refP * 1000 * 2;
		if(type == STEP_CHARGE) {
		//	val3 = (float)refP / (float)val1;
			val3 = ((float)refP / (float)myCh->op.Vsens); 
		} else {
		//	val3 = ((float)refP / (float)myCh->misc.startV) * (-1);
			val3 = ((float)refP / (float)myCh->op.Vsens) * (-1); 
		} 
		val3 *= 1000000;
	}
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

	
	switch(type) {
		case STEP_REST:
			if(myCh->op.checkDelayTime < 180) break;

			if(labs(myCh->misc.tmpIsens) > (maxI * 0.5)) {
				//case of op-amp control fail
				myCh->misc.errCnt[C_CNT_CH_I_FAIL]++;
				if(myCh->misc.errCnt[C_CNT_CH_I_FAIL] >= MAX_ERROR_CNT) {
					myCh->misc.errCnt[C_CNT_CH_I_FAIL] = 0;
					myCh->misc.tmpState = myCh->op.state;
					myCh->misc.tmpCode = myCh->op.code;
					myCh->op.code = C_FAULT_CH_I_FAIL;
					rtn = FAULT_COND;
					break;
				}
			} else {
				myCh->misc.errCnt[C_CNT_CH_I_FAIL] = 0;
			}
			break;
		case STEP_CHARGE:
			if(myCh->op.checkDelayTime < 180) break;

			if(checkFlag == 1) {
				if(mode == CC || mode == CV || mode == CCCV || mode == C_RATE) {
					if((val1 - myCh->op.Vsens) > maxV * 0.001) {
						if(myCh->op.Isens < val2 * 0.6) {
							myCh->misc.errCnt[C_CNT_CONTACT_BAD]++;
							if(myCh->misc.errCnt[C_CNT_CONTACT_BAD]
								>= MAX_ERROR_CNT) {
								myCh->misc.errCnt[C_CNT_CONTACT_BAD] = 0;
								myCh->misc.tmpState = myCh->op.state;
								myCh->misc.tmpCode = myCh->op.code;
								if(myCh->op.checkDelayTime < 300) {
									myCh->op.code = C_FAULT_CH_I_FAIL;
								} else {
									myCh->op.code = C_FAULT_CH_V_FAIL;
								}
								rtn = FAULT_COND;
								break;
							}
						} else {
							myCh->misc.errCnt[C_CNT_CONTACT_BAD] = 0;
						}
					}
				}
			} else {
				if(mode == CC || mode == CV || mode == CCCV || mode == C_RATE) {
					if((val1 - myCh->op.Vsens) > maxV * 0.001) {
						if(myCh->misc.feedback_delayTime > 100 //1Sec
							|| myCh->op.checkDelayTime > 2000) { //20Sec
							if(myCh->op.Isens < val2 * 0.6) {
								myCh->misc.errCnt[C_CNT_CONTACT_BAD]++;
								if(myCh->misc.errCnt[C_CNT_CONTACT_BAD]
									>= MAX_ERROR_CNT) {
									myCh->misc.errCnt[C_CNT_CONTACT_BAD] = 0;
									myCh->misc.tmpState = myCh->op.state;
									myCh->misc.tmpCode = myCh->op.code;
									if(myCh->op.checkDelayTime < 300) {
										myCh->op.code = C_FAULT_CH_I_FAIL;
									} else {
										myCh->op.code = C_FAULT_CH_V_FAIL;
									}
									rtn = FAULT_COND;
									break;
								}
							} else {
								myCh->misc.errCnt[C_CNT_CONTACT_BAD] = 0;
							}
						}
					}
				}
			}

			//20180410 add for Charge CP mode
			if(mode == CP || mode == CPCV) {
				if((val1 - myCh->op.Vsens) > maxV * 0.001) {
					if(myCh->op.Isens < val3 * 0.6) {
						myCh->misc.errCnt[C_CNT_CONTACT_BAD]++; //191031
						if(myCh->misc.errCnt[C_CNT_CONTACT_BAD]
							>= MAX_ERROR_CNT) {
							myCh->misc.errCnt[C_CNT_CONTACT_BAD] = 0;
							myCh->misc.tmpState = myCh->op.state;
							myCh->misc.tmpCode = myCh->op.code;
							if(myCh->op.checkDelayTime < 300) {
								myCh->op.code = C_FAULT_CH_I_FAIL;
							} else {
								myCh->op.code = C_FAULT_CH_V_FAIL;
							}
							rtn = FAULT_COND;
							break;
						}
					} else {
						myCh->misc.errCnt[C_CNT_CONTACT_BAD] = 0;
					}
				}
			}

			tmp = (float)maxI * 0.04;
			if((myCh->op.Isens - val2) >= (long)tmp) {
				myCh->misc.errCnt[C_CNT_CH_I_FAIL]++;
				if(myCh->misc.errCnt[C_CNT_CH_I_FAIL] >= MAX_ERROR_CNT) {
					myCh->misc.errCnt[C_CNT_CH_I_FAIL] = 0;
					myCh->misc.tmpState = myCh->op.state;
					myCh->misc.tmpCode = myCh->op.code;
					myCh->op.code = C_FAULT_CH_I_FAIL;
					rtn = FAULT_COND;
					break;
				}
			} else {
				myCh->misc.errCnt[C_CNT_CH_I_FAIL] = 0;
			}

			tmp = (float)maxV * 0.02;
			if((myCh->op.Vsens - val1) >= (long)tmp) {
				myCh->misc.errCnt[C_CNT_CH_V_FAIL]++;
				if(myCh->misc.errCnt[C_CNT_CH_V_FAIL]>= MAX_ERROR_CNT) {
					myCh->misc.errCnt[C_CNT_CH_V_FAIL] = 0;
					myCh->misc.tmpState = myCh->op.state;
					myCh->misc.tmpCode = myCh->op.code;
					myCh->op.code = C_FAULT_CH_V_FAIL;
					rtn = FAULT_COND;
					break;
				}
			} else {
				myCh->misc.errCnt[C_CNT_CH_V_FAIL] = 0;
			}
			
			//20171123 sch for NoCell
			if(myCh->op.runTime < 300) {
				tmp = (float)(myCh->misc.startV);
				if(abs((myCh->op.Vsens - (long)tmp)) >= (val1 * 0.8)) {
					myCh->misc.tmpState = myCh->op.state;
					myCh->misc.tmpCode = myCh->op.code;
					myCh->op.code = C_FAULT_CH_V_FAIL;
					rtn = FAULT_COND;
					break;
				}
			}
			
			rtn = SAMSUNG_SDI_safety(bd, ch, rtn, type);

			break;
		case STEP_DISCHARGE:
			if(myCh->op.checkDelayTime < 180) break;

			if(checkFlag == 1){
				if(mode == CC || mode == CV || mode == CCCV || mode == C_RATE) {
					if((myCh->op.Vsens - val1) > (maxV * 0.001)) {
						if(myCh->op.Isens > (val2 * 0.6)) {
							myCh->misc.errCnt[C_CNT_CONTACT_BAD]++;
							if(myCh->misc.errCnt[C_CNT_CONTACT_BAD]
								>= MAX_ERROR_CNT) {
								myCh->misc.errCnt[C_CNT_CONTACT_BAD] = 0;
								myCh->misc.tmpState = myCh->op.state;
								myCh->misc.tmpCode = myCh->op.code;
								//100908 pjy
								if(myCh->op.checkDelayTime < 300) {
									myCh->op.code = C_FAULT_CH_I_FAIL;
								} else {
									myCh->op.code = C_FAULT_CH_V_FAIL;
								}
								rtn = FAULT_COND;
								break;
							}
						} else {
							myCh->misc.errCnt[C_CNT_CONTACT_BAD] = 0;
						}
					}
				}
			} else {
				if(mode == CC || mode == CV || mode == CCCV || mode == C_RATE) {
					if((myCh->op.Vsens - val1) > (maxV * 0.001)) {
						if(myCh->misc.feedback_delayTime > 100 //1Sec
							|| myCh->op.checkDelayTime > 2000) { //20Sec
							if(myCh->op.Isens > (val2 * 0.6)) {
								myCh->misc.errCnt[C_CNT_CONTACT_BAD]++;
								if(myCh->misc.errCnt[C_CNT_CONTACT_BAD]
									>= MAX_ERROR_CNT) {
									myCh->misc.errCnt[C_CNT_CONTACT_BAD] = 0;
									myCh->misc.tmpState = myCh->op.state;
									myCh->misc.tmpCode = myCh->op.code;
									//100908 pjy
									if(myCh->op.checkDelayTime < 300) {
										myCh->op.code = C_FAULT_CH_I_FAIL;
									} else {
										myCh->op.code = C_FAULT_CH_V_FAIL;
									}
									rtn = FAULT_COND;
									break;
								}
							} else {
								myCh->misc.errCnt[C_CNT_CONTACT_BAD] = 0;
							}
						}
					}
				}
			}

			//20180410 add for Discharge CP mode
			if(mode == CP || mode == CPCV) {
				if((myCh->op.Vsens - val1) > (maxV * 0.001)) {
					if(myCh->op.Isens > (val3 * 0.6)) {
						myCh->misc.errCnt[C_CNT_CONTACT_BAD]++; //191031
						if(myCh->misc.errCnt[C_CNT_CONTACT_BAD]
							>= MAX_ERROR_CNT) {
							myCh->misc.errCnt[C_CNT_CONTACT_BAD] = 0;
							myCh->misc.tmpState = myCh->op.state;
							myCh->misc.tmpCode = myCh->op.code;
							if(myCh->op.checkDelayTime < 300) {
								myCh->op.code = C_FAULT_CH_I_FAIL;
							} else {
								myCh->op.code = C_FAULT_CH_V_FAIL;
							}
							rtn = FAULT_COND;
							break;
						}
					} else {
						myCh->misc.errCnt[C_CNT_CONTACT_BAD] = 0;
					}
				}
			}

			tmp = (float)maxI * 0.04;
			if((val2 - myCh->op.Isens) >= (long)tmp) {
				myCh->misc.errCnt[C_CNT_CH_I_FAIL]++;
				if(myCh->misc.errCnt[C_CNT_CH_I_FAIL] >= MAX_ERROR_CNT) {
					myCh->misc.errCnt[C_CNT_CH_I_FAIL] = 0;
					myCh->misc.tmpState = myCh->op.state;
					myCh->misc.tmpCode = myCh->op.code;
					myCh->op.code = C_FAULT_CH_I_FAIL;
					rtn = FAULT_COND;
					break;
				}
			} else {
				myCh->misc.errCnt[C_CNT_CH_I_FAIL] = 0;
			}

			tmp = (float)maxV * 0.02;
			if((val1 - myCh->op.Vsens) >= (long)tmp) {
				myCh->misc.errCnt[C_CNT_CH_V_FAIL]++;
				if(myCh->misc.errCnt[C_CNT_CH_V_FAIL] >= MAX_ERROR_CNT) {
					myCh->misc.errCnt[C_CNT_CH_V_FAIL] = 0;
					myCh->misc.tmpState = myCh->op.state;
					myCh->misc.tmpCode = myCh->op.code;
					myCh->op.code = C_FAULT_CH_V_FAIL;
					rtn = FAULT_COND;
					break;
				}
			} else {
				myCh->misc.errCnt[C_CNT_CH_V_FAIL] = 0;
			}

			rtn = SAMSUNG_SDI_safety(bd, ch, rtn, type);
			break;
		case STEP_Z: 
			if(myCh->op.checkDelayTime < 180) break;

			tmp = (float)maxI * 0.04;
			if((val2 - myCh->op.Isens) >= (long)tmp) {
				myCh->misc.errCnt[C_CNT_CH_I_FAIL]++;
				if(myCh->misc.errCnt[C_CNT_CH_I_FAIL]>= MAX_ERROR_CNT) {
					myCh->misc.errCnt[C_CNT_CH_I_FAIL] = 0;
					myCh->misc.tmpState = myCh->op.state;
					myCh->misc.tmpCode = myCh->op.code;
					myCh->op.code = C_FAULT_CH_I_FAIL;
					rtn = FAULT_COND;
					break;
				}
			} else {
				myCh->misc.errCnt[C_CNT_CH_I_FAIL] = 0;
			}

			tmp = (float)maxV * 0.02;
			if((val1 - myCh->op.Vsens) >= (long)tmp) {
				myCh->misc.errCnt[C_CNT_CH_V_FAIL]++;
				if(myCh->misc.errCnt[C_CNT_CH_V_FAIL] >= MAX_ERROR_CNT) {
					myCh->misc.errCnt[C_CNT_CH_V_FAIL] = 0;
					myCh->misc.tmpState = myCh->op.state;
					myCh->misc.tmpCode = myCh->op.code;
					myCh->op.code = C_FAULT_CH_V_FAIL;
					rtn = FAULT_COND;
					break;
				}
			} else {
				myCh->misc.errCnt[C_CNT_CH_V_FAIL] = 0;
			}
			break;
		case STEP_USER_PATTERN:
			if(myCh->op.checkDelayTime < 180) break;
/*
			if(val2 > 0){
				delta = myCh->misc.tmpIsens - val2;
			} else if(val2 < 0) {
				delta = val2 - myCh->misc.tmpIsens;
			} else {
				delta = maxI;
			}
			if(delta >= (long)(maxI * 0.04)) {
				myCh->misc.errCnt[C_CNT_CH_I_FAIL]++;
				if(myCh->misc.errCnt[C_CNT_CH_I_FAIL] >= MAX_ERROR_CNT) {
					myCh->misc.errCnt[C_CNT_CH_I_FAIL] = 0;
					myCh->misc.tmpState = myCh->op.state;
					myCh->misc.tmpCode = myCh->op.code;
					myCh->op.code = C_FAULT_CH_I_FAIL;
					rtn = FAULT_COND;
					break;
				}
			} else {
				myCh->misc.errCnt[C_CNT_CH_I_FAIL] = 0;
			}*/

			if(mode == CC) {
				tmp = (float)maxI * 0.04;
				if((labs(myCh->op.Isens) - labs(val2)) >= (long)tmp) {
					myCh->misc.errCnt[C_CNT_CH_I_FAIL]++;
					if(myCh->misc.errCnt[C_CNT_CH_I_FAIL] >= MAX_ERROR_CNT) {
						myCh->misc.errCnt[C_CNT_CH_I_FAIL] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_CH_I_FAIL;
						rtn = FAULT_COND;
						break;
					}
				} else {
					myCh->misc.errCnt[C_CNT_CH_I_FAIL] = 0;
				}
				
				if(val2 > 0) {
					delta = myCh->op.Vsens - val1;
				} else if(val2 < 0) {
					delta = val1 - myCh->op.Vsens;
				} else {
					delta = 0;
				}
				if(delta >= (long)(maxV * 0.02)) {
					myCh->misc.errCnt[C_CNT_CH_V_FAIL]++;
					if(myCh->misc.errCnt[C_CNT_CH_V_FAIL] >= MAX_ERROR_CNT) {
						myCh->misc.errCnt[C_CNT_CH_V_FAIL] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_CH_V_FAIL;
						rtn = FAULT_COND;
						break;
					}
				} else {
					myCh->misc.errCnt[C_CNT_CH_V_FAIL] = 0;
				}
			}
			break;
		case STEP_USER_MAP:
			if(myCh->op.checkDelayTime < 180) break;

			tmp = (float)maxI * 0.04;
			if((labs(myCh->op.Isens) - labs(val2)) >= (long)tmp
				&& val2 != 0) {		
				myCh->misc.errCnt[C_CNT_CH_I_FAIL]++;
				if(myCh->misc.errCnt[C_CNT_CH_I_FAIL] >= MAX_ERROR_CNT) {
					myCh->misc.errCnt[C_CNT_CH_I_FAIL] = 0;
					myCh->misc.tmpState = myCh->op.state;
					myCh->misc.tmpCode = myCh->op.code;
					myCh->op.code = C_FAULT_CH_I_FAIL;
					rtn = FAULT_COND;
					break;
				}
			} else {
				myCh->misc.errCnt[C_CNT_CH_I_FAIL] = 0;
			}
			
			if(val2 > 0) {
				delta = myCh->op.Vsens - val1;
			} else if(val2 < 0) {
				delta = val1 - myCh->op.Vsens;
			} else {
				delta = 0;
			}
			if(delta >= (long)(maxV * 0.02)) {
				myCh->misc.errCnt[C_CNT_CH_V_FAIL]++;
				if(myCh->misc.errCnt[C_CNT_CH_V_FAIL] >= MAX_ERROR_CNT) {
					myCh->misc.errCnt[C_CNT_CH_V_FAIL] = 0;
					myCh->misc.tmpState = myCh->op.state;
					myCh->misc.tmpCode = myCh->op.code;
					myCh->op.code = C_FAULT_CH_V_FAIL;
					rtn = FAULT_COND;
					break;
				}
			} else {
				myCh->misc.errCnt[C_CNT_CH_V_FAIL] = 0;
			}
			break;
		default: 
			break;
	}

	if(rtn == FAULT_COND) { //fault
		myCh->op.select = SAVE_FLAG_SAVING_ETC;
		send_save_msg(bd, ch, saveDt, 0);

		//170106 oys add
		if(myCh->misc.tempWaitType == P0) {
			myCh->misc.chGroupNo = 0;
			//20180417 add
			myCh->misc.stepSyncFlag = P0;
			myCh->misc.endState = P0;		//220203_hun
			myCh->misc.groupAvgVsens = 0;	//220203_hun
			myCh->misc.group_StartVoltage_flag = 0;
		}
		//190318 lyhw
		if(myData->mData.config.parallelMode == P2) { 
			cCycle_p_ch_check(bd, ch);
		}
		myCh->op.phase = P100;
		cNextStepCheck(bd, ch);
	}else if(rtn == 1){	//200504 hun add
		myCh->op.select = SAVE_FLAG_SAVING_ETC;
		send_save_msg(bd, ch, saveDt, 0);
	}
}

void cFaultCondSoft(int bd, int ch)
{
	unsigned char type;
    int rtn=0, rangeV, rangeI, diff, mode;
	int	i, index, tempBD, tempCH, count, fault_Count; //210205
//  int	diff, LimitFlagI = 0, LimitFlagV = 0, LimitBd, LimitCh;
	long capacity, refV, refI, maxV, maxI;
	long cvtVal, faultUpperI_com, faultLowerI_com;
	long faultUpperI_step, faultLowerI_step;
	unsigned long advStepNo, saveDt;
	S_CH_STEP_INFO step;

	if(myData->mData.config.function[F_SW_FAULT_COND] == P0) return;

	if(myData->bData[bd].cData[ch].misc.parallel_cycle_phase == P50) {
		cFaultCondSoft_P2(bd, ch); //kjg_180521
		return;
	}

	myCh = &(myData->bData[bd].cData[ch]);
	myTestCond = &(myPs->testCond[bd][ch]);

	if(myCh->op.state != C_RUN) return;
	if(myCh->misc.patternPhase != 0) return;
	if(myCh->op.phase != P50) return;

	step = step_info(bd, ch);

	advStepNo = step.advStepNo;
	saveDt = step.saveDt;
	type = step.type;

	refV = step.refV;
	refI = step.refI;
	rangeV = step.rangeV;
	rangeI = step.rangeI;
	mode = step.mode;

	maxV = myPs->config.maxVoltage[rangeV];
	maxI = myPs->config.maxCurrent[rangeI];
	
	//190801 oys add start : Convert C-rate to I value
	faultUpperI_com = myPs->testCond[bd][ch].safety.faultUpperI;
	cvtVal = Convert_C_rate_to_I_value(bd, ch, faultUpperI_com);
	if(cvtVal != 0)
		faultUpperI_com = cvtVal;

	faultLowerI_com = myPs->testCond[bd][ch].safety.faultLowerI;
	cvtVal = Convert_C_rate_to_I_value(bd, ch, faultLowerI_com);
	if(cvtVal != 0)
		faultLowerI_com = cvtVal;
	
	faultUpperI_step = myPs->testCond[bd][ch].step[advStepNo].faultUpperI;
	cvtVal = Convert_C_rate_to_I_value(bd, ch, faultUpperI_step);
	if(cvtVal != 0)
		faultUpperI_step = cvtVal;
	
	faultLowerI_step = myPs->testCond[bd][ch].step[advStepNo].faultLowerI;
	cvtVal = Convert_C_rate_to_I_value(bd, ch, faultLowerI_step);
	if(cvtVal != 0)
		faultLowerI_step = cvtVal;
	//add end
	
	if(myPs->config.capacityType == CAPACITY_AMPARE_HOURS) {
		capacity = myCh->op.ampareHour;
	} else if(myPs->config.capacityType == CAPACITY_CAPACITANCE) {
		capacity = myCh->op.capacitance;
	} else {
		capacity = 0;
	}
	switch(type) {
		case STEP_OCV:
			if(myPs->testCond[bd][ch].safety.faultUpperV != 0) {
				if(myCh->op.Vsens
					>= myPs->testCond[bd][ch].safety.faultUpperV) {
					myCh->misc.errCnt[C_CNT_UPPER_OCV]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_OCV]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_UPPER_OCV] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_OCV;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); 
						//safety
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_UPPER_OCV] = 0;
				}
			}
			if(myPs->testCond[bd][ch].safety.faultLowerV != 0) {
			   	if(myCh->op.Vsens
					<= myPs->testCond[bd][ch].safety.faultLowerV) {
					myCh->misc.errCnt[C_CNT_LOWER_OCV]++;
					if(myCh->misc.errCnt[C_CNT_LOWER_OCV]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_LOWER_OCV] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_LOWER_OCV;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); 
						//safety
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_LOWER_OCV] = 0;
				}
			}

			if(myPs->testCond[bd][ch].safety.faultUpperTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 0);
				if(rtn == FAULT_COND)
					break;
			}
			if(myPs->testCond[bd][ch].safety.faultLowerTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 1);
				if(rtn == FAULT_COND)
					break;
			}
			if(myPs->testCond[bd][ch].step[advStepNo].faultUpperV != 0) {
				if(myCh->op.Vsens >=
					myPs->testCond[bd][ch].step[advStepNo].faultUpperV) {
					myCh->misc.errCnt[C_CNT_UPPER_OCV_STEP]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_OCV_STEP]>=MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_UPPER_OCV_STEP] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_OCV;
						myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); 
						//step_safety
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_UPPER_OCV_STEP] = 0;
				}
			}
			if(myPs->testCond[bd][ch].step[advStepNo].faultLowerV != 0) {
			   	if(myCh->op.Vsens <=
					myPs->testCond[bd][ch].step[advStepNo].faultLowerV) {
					myCh->misc.errCnt[C_CNT_LOWER_OCV_STEP]++;
					if(myCh->misc.errCnt[C_CNT_LOWER_OCV_STEP]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_LOWER_OCV_STEP] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_LOWER_OCV;
						myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); 
						//step_safety
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_LOWER_OCV_STEP] = 0;
				}
			}
			if(myPs->testCond[bd][ch].step[advStepNo].faultUpperTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 2);
				if(rtn == FAULT_COND)
					break;
			}
			if(myPs->testCond[bd][ch].step[advStepNo].faultLowerTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 3);
				if(rtn == FAULT_COND)
					break;
			}
			//190311 lyh add
			if(myTestCond->step[advStepNo].pauseUpperTemp != 0) {
				rtn = tempPauseCheck(bd, ch, advStepNo, 0);
				if(rtn == FAULT_COND)
					break;
			}
			if(myTestCond->step[advStepNo].pauseLowerTemp != 0) {
				rtn = tempPauseCheck(bd, ch, advStepNo, 1);
				if(rtn == FAULT_COND)
					break;
			}
			//20180314 sch modify
			if(myPs->config.LimitVI.limit_use_I != 0 
				|| myPs->config.LimitVI.limit_use_V != 0){
				rtn = Fault_Check_default_LG(bd, ch);
				if(rtn == FAULT_COND)	break;
			}
			//19090l lyhw
			if(myPs->config.function[F_CHANGE_VI_CHECK] != 0){
				rtn = Fault_Check_Change_VI(bd, ch, advStepNo);
				if(rtn == FAULT_COND)	break;
			}
			//210923 lyhw add for lges gas control
			if(GAS_DATA_CONTROL == 1){
				rtn = Fault_Check_Gas_Data(bd, ch, advStepNo);
				if(rtn == FAULT_COND)	break;
			}
			break;
		case STEP_REST:
			if(myPs->testCond[bd][ch].safety.faultUpperV != 0) {
				if(myCh->op.Vsens
					>= myPs->testCond[bd][ch].safety.faultUpperV) {
					myCh->misc.errCnt[C_CNT_UPPER_OCV]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_OCV]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_UPPER_OCV] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_OCV;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); //safety
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_UPPER_OCV] = 0;
				}
			}
			if(myPs->testCond[bd][ch].safety.faultLowerV != 0) {
			   	if(myCh->op.Vsens
					<= myPs->testCond[bd][ch].safety.faultLowerV) {
					myCh->misc.errCnt[C_CNT_LOWER_OCV]++;
					if(myCh->misc.errCnt[C_CNT_LOWER_OCV]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_LOWER_OCV] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_LOWER_OCV;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); //safety
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_LOWER_OCV] = 0;
				}
			}
			if(myPs->testCond[bd][ch].safety.faultUpperTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 0);
				if(rtn == FAULT_COND)
					break;
			}
			if(myPs->testCond[bd][ch].safety.faultLowerTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 1);
				if(rtn == FAULT_COND)
					break;
			}
			if(myPs->testCond[bd][ch].step[advStepNo].faultUpperV != 0) {
				if(myCh->op.Vsens >=
					myPs->testCond[bd][ch].step[advStepNo].faultUpperV) {
					myCh->misc.errCnt[C_CNT_UPPER_OCV_STEP]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_OCV_STEP]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_UPPER_OCV_STEP] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_OCV;
						myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); //step_safety
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_UPPER_OCV_STEP] = 0;
				}
			}
			if(myPs->testCond[bd][ch].step[advStepNo].faultLowerV != 0) {
			   	if(myCh->op.Vsens <=
					myPs->testCond[bd][ch].step[advStepNo].faultLowerV) {
					myCh->misc.errCnt[C_CNT_LOWER_OCV_STEP]++;
					if(myCh->misc.errCnt[C_CNT_LOWER_OCV_STEP]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_LOWER_OCV_STEP] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_LOWER_OCV;
						myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); //step_safety
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_LOWER_OCV_STEP] = 0;
				}
			}
			if(myPs->testCond[bd][ch].step[advStepNo].faultUpperTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 2);
				if(rtn == FAULT_COND)
					break;
			}
			if(myPs->testCond[bd][ch].step[advStepNo].faultLowerTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 3);
				if(rtn == FAULT_COND)
					break;
			}
			//190311 lyh add
			if(myTestCond->step[advStepNo].pauseUpperTemp != 0) {
				rtn = tempPauseCheck(bd, ch, advStepNo, 0);
				if(rtn == FAULT_COND)
					break;
			}
			if(myTestCond->step[advStepNo].pauseLowerTemp != 0) {
				rtn = tempPauseCheck(bd, ch, advStepNo, 1);
				if(rtn == FAULT_COND)
					break;
			}
			//20180314 sch modify
			if(myPs->config.LimitVI.limit_use_I != 0 
				|| myPs->config.LimitVI.limit_use_V != 0){
				rtn = Fault_Check_default_LG(bd, ch);
				if(rtn == FAULT_COND)
					break;
			}
			//19090l lyhw
			if(myPs->config.function[F_CHANGE_VI_CHECK] != 0){
				rtn = Fault_Check_Change_VI(bd, ch, advStepNo);
				if(rtn == FAULT_COND)	break;
			}
			//210428_hun
			if(myData->mData.config.swFaultConfig[REST_CHECK_START_TIME] != 0){
				rtn = Fault_Check_Rest_Voltage(bd, ch);
				if(rtn == FAULT_COND)	break;
			}
			//210923 lyhw add for lges gas control
			if(GAS_DATA_CONTROL == 1){
				rtn = Fault_Check_Gas_Data(bd, ch, advStepNo);
				if(rtn == FAULT_COND)	break;
			}
			break;
    	case STEP_CHARGE:
			//100908 pjy temp fault check without delay
			if(myPs->testCond[bd][ch].safety.faultUpperTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 0);
				if(rtn == FAULT_COND)
					break;
			}
			if(myPs->testCond[bd][ch].safety.faultLowerTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 1);
				if(rtn == FAULT_COND)
					break;
			}
			if(myPs->testCond[bd][ch].step[advStepNo].faultUpperTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 2);
				if(rtn == FAULT_COND)
					break;
			}
			if(myPs->testCond[bd][ch].step[advStepNo].faultLowerTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 3);
				if(rtn == FAULT_COND)
					break;
			}
			//190311 lyh add
			if(myTestCond->step[advStepNo].pauseUpperTemp != 0) {
				rtn = tempPauseCheck(bd, ch, advStepNo, 0);
				if(rtn == FAULT_COND)
					break;
			}
			if(myTestCond->step[advStepNo].pauseLowerTemp != 0) {
				rtn = tempPauseCheck(bd, ch, advStepNo, 1);
				if(rtn == FAULT_COND)
					break;
			}
		
			//hun_200219_s
			#ifdef _SDI_SAFETY_V1	
			if(myTestCond->step[advStepNo].faultRunTime != 0) {
				if(myCh->op.runTime >= myTestCond->step[advStepNo].faultRunTime){
					myCh->misc.tmpState = myCh->op.state;
					myCh->misc.tmpCode = myCh->op.code;
					myCh->op.code = C_FAULT_RUN_TIME;
					Fault_Value_Check(myCh->misc.advStepNo, 
					myCh->op.runTime,
					myPs->testCond[bd][ch].step[advStepNo].faultRunTime,
					0, 0, 0, 0, 0, 0, 0);
					rtn = FAULT_COND;
					break;
				}
			}
			#endif
			//hun_200219_e
			//200706
			#if CHANGE_VI_CHECK == 1
			if(myTestCond->step[advStepNo].faultRunTime != 0) {
				if(myCh->op.runTime >= myTestCond->step[advStepNo].faultRunTime){
					myCh->misc.tmpState = myCh->op.state;
					myCh->misc.tmpCode = myCh->op.code;
					myCh->op.code = C_FAULT_RUN_TIME;
					rtn = FAULT_COND;
					break;
				}
			}
			#endif
			//kjc_211028
			#ifdef _ULSAN_SDI_SAFETY
			if(myTestCond->step[advStepNo].humpSet_T != 0 
				&& myTestCond->step[advStepNo].humpSet_I != 0){
				if(myCh->misc.cvFlag == P1){
					if(myCh->misc.cvTime_Ulsan <= 100){
						myCh->misc.humpComp_T = myCh->op.runTime;
						myCh->misc.humpComp_I = myCh->op.Isens;

						myCh->misc.humpCheck_T = myCh->misc.humpComp_T 
							+ myTestCond->step[advStepNo].humpSet_T;
					}
					if(myCh->misc.cvTime_Ulsan > 100){
						if(myCh->op.runTime >= myCh->misc.humpCheck_T){
							myCh->misc.humpCheck_I
								= myCh->op.Isens - myCh->misc.humpComp_I;

							myCh->misc.humpComp_T = myCh->op.runTime;
							myCh->misc.humpComp_I = myCh->op.Isens;

							myCh->misc.humpCheck_T = myCh->misc.humpComp_T 
								+ myTestCond->step[advStepNo].humpSet_T;

							if(myCh->misc.humpCheck_I 
								>= myTestCond->step[advStepNo].humpSet_I){
								myCh->misc.tmpState = myCh->op.state;
								myCh->misc.tmpCode = myCh->op.code;
								myCh->misc.cvFlag = 0;
								myCh->misc.cvTime_Ulsan = 0;
								myCh->op.code = C_FAULT_CV_CURRENT_HUMP;
								rtn = FAULT_COND;
								break;
							}
						}
					}
				}
			}
			#endif
			
			if(myPs->testCond[bd][ch].safety.faultUpperV != 0) {
				if(myCh->op.Vsens
					>= myPs->testCond[bd][ch].safety.faultUpperV) {
					myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_VOLTAGE;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); //safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Vsens,
						myPs->testCond[bd][ch].safety.faultUpperV,
					   	0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE] = 0;
				}
			}
			if(myPs->testCond[bd][ch].safety.faultLowerV != 0) {
				if(myCh->op.Vsens
					<= myPs->testCond[bd][ch].safety.faultLowerV) {
					myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE]++;
					if(myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_LOWER_VOLTAGE;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); //safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Vsens,
						myPs->testCond[bd][ch].safety.faultLowerV,
					   	0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE] = 0;
				}
			}
			if(myCh->op.checkDelayTime < 180) break;
			if(faultUpperI_com != 0) {
				if(myCh->op.Isens >= faultUpperI_com) {
					myCh->misc.errCnt[C_CNT_UPPER_CURRENT]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_CURRENT]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_UPPER_CURRENT] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_CURRENT;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); //safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Isens,
						faultUpperI_com, 0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_UPPER_CURRENT] = 0;
				}
			}
			if(faultLowerI_com != 0) {
				if(myCh->op.Isens <= faultLowerI_com) {
					myCh->misc.errCnt[C_CNT_LOWER_CURRENT]++;
					if(myCh->misc.errCnt[C_CNT_LOWER_CURRENT]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_LOWER_CURRENT] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_LOWER_CURRENT;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); //safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Isens,
						faultLowerI_com, 0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_LOWER_CURRENT] = 0;
				}
			}
			if(myPs->testCond[bd][ch].safety.faultUpperC != 0) {
				if(capacity >= myPs->testCond[bd][ch].safety.faultUpperC) {
					myCh->misc.errCnt[C_CNT_UPPER_CAPACITY]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_CAPACITY]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_UPPER_CAPACITY] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_CAPACITY;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); //safety
						Fault_Value_Check(myCh->misc.advStepNo, capacity,
						myPs->testCond[bd][ch].safety.faultUpperC,
					   	0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_UPPER_CAPACITY] = 0;
				}
			}
			if(myPs->testCond[bd][ch].step[advStepNo].faultUpperV != 0) {
				if(myCh->op.Vsens >=
					myPs->testCond[bd][ch].step[advStepNo].faultUpperV) {
					myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE_STEP]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE_STEP]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE_STEP] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_VOLTAGE;
						myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); //step_safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Vsens,
						myPs->testCond[bd][ch].step[advStepNo].faultUpperV,
					   	0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE_STEP] = 0;
				}
			}
			if(myPs->testCond[bd][ch].step[advStepNo].faultLowerV != 0) {
				if(myCh->op.Vsens <=
					myPs->testCond[bd][ch].step[advStepNo].faultLowerV) {
					myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE_STEP]++;
					if(myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE_STEP]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE_STEP] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_LOWER_VOLTAGE;
						myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); //step_safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Vsens,
						myPs->testCond[bd][ch].step[advStepNo].faultLowerV,
					   	0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE_STEP] = 0;
				}
			}
			if(faultUpperI_step != 0) {
				if(myCh->op.Isens >= faultUpperI_step) {
					myCh->misc.errCnt[C_CNT_UPPER_CURRENT_STEP]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_CURRENT_STEP]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_UPPER_CURRENT_STEP] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_CURRENT;
						myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); //step_safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Isens,
						faultUpperI_step, 0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_UPPER_CURRENT_STEP] = 0;
				}
			}
			if((refV - myCh->op.Vsens) > (maxV * 0.001)){		//kjc_200612
				if(faultLowerI_step != 0) {
					if(myCh->op.Isens <= faultLowerI_step) {
						myCh->misc.errCnt[C_CNT_LOWER_CURRENT_STEP]++;
						if(myCh->misc.errCnt[C_CNT_LOWER_CURRENT_STEP]>= MAX_ERROR_CNT){
							myCh->misc.errCnt[C_CNT_LOWER_CURRENT_STEP] = 0;
							myCh->misc.tmpState = myCh->op.state;
							myCh->misc.tmpCode = myCh->op.code;
							myCh->op.code = C_FAULT_LOWER_CURRENT;
							myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); //step_safety
							Fault_Value_Check(myCh->misc.advStepNo, 
							myCh->op.Isens, faultLowerI_step,
						   	0, 0, 0, 0, 0, 0, 0);
							rtn = FAULT_COND;
							break;
						}
					}else{
						myCh->misc.errCnt[C_CNT_LOWER_CURRENT_STEP] = 0;
					}
				}
			}
			
			if(myPs->testCond[bd][ch].step[advStepNo].faultUpperC != 0) {
				if(capacity >=
					myPs->testCond[bd][ch].step[advStepNo].faultUpperC) {
					myCh->misc.errCnt[C_CNT_UPPER_CAPACITY_STEP]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_CAPACITY_STEP]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_UPPER_CAPACITY_STEP] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_CAPACITY;
						myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); //step_safety
						Fault_Value_Check(myCh->misc.advStepNo, capacity,
						myPs->testCond[bd][ch].step[advStepNo].faultUpperC,
					   	0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_UPPER_CAPACITY_STEP] = 0;
				}
			}	
			//170215 SCH add for DeltaV/I CHARGE, 180918 rewirte
			if(myPs->testCond[bd][ch].step[advStepNo].faultDeltaV != 0 
				&& myPs->testCond[bd][ch].step[advStepNo].faultDeltaV_T !=0){
				if(myData->mData.config.function[F_DELTA_V_I] != 0){
//					if(labs(refI - myCh->op.Isens) < maxI*0.001){
						if(labs(myCh->misc.maxV - myCh->op.Vsens) >= 
							myPs->testCond[bd][ch].step[advStepNo].faultDeltaV){
							diff = myCh->op.runTime - myCh->misc.deltaV_timeout;
							if(diff >= myPs->testCond[bd][ch].step[advStepNo].faultDeltaV_T){
								myCh->misc.deltaV_timeout = myCh->op.runTime;
								myCh->misc.tmpState = myCh->op.state;
								myCh->misc.tmpCode = myCh->op.code;
								if(myData->mData.config.function[F_DELTA_V_I] == 1){
									myCh->op.code = C_STOP_DELTA_V;
									rtn = FAULT_COND;
									//rtn = END_COND;
								}else if(myData->mData.config.function[F_DELTA_V_I] == 2){
									myCh->op.code = C_FAULT_DELTA_V;
									rtn = FAULT_COND;
								}
								break;
							}
						}else{	
							myData->bData[bd].cData[ch].misc.deltaV_timeout
								= myData->bData[bd].cData[ch].op.runTime;
						}
//					}
				}
			} else if(myPs->testCond[bd][ch].step[advStepNo].faultDeltaV != 0){
					if(myData->mData.config.function[F_DELTA_V_I] != 0){
						if(labs(refI - myCh->op.Isens) < maxI*0.001){
							if(labs(myCh->misc.maxV - myCh->op.Vsens) >= 
								myPs->testCond[bd][ch].step[advStepNo].faultDeltaV){
								myCh->misc.errCnt[C_CNT_DELTA_V_STEP] ++;
								if(myCh->misc.errCnt[C_CNT_DELTA_V_STEP] >
															MAX_ERROR_CNT){
									myCh->misc.errCnt[C_CNT_DELTA_V_STEP] = 0;
									myCh->misc.tmpState = myCh->op.state;
									myCh->misc.tmpCode = myCh->op.code;
									if(myData->mData.config.function[F_DELTA_V_I] == 1){
										myCh->op.code = C_STOP_DELTA_V;
										rtn = FAULT_COND;
										//rtn = END_COND;
									}else if(myData->mData.config.function[F_DELTA_V_I] == 2){
										myCh->op.code = C_FAULT_DELTA_V;
										rtn = FAULT_COND;
									}
									break;
								}
							}else{	
								myCh->misc.errCnt[C_CNT_DELTA_V_STEP] = 0;
							}
						}
					}
				}

			//current
			if(myPs->testCond[bd][ch].step[advStepNo].faultDeltaI != 0 
				&& myPs->testCond[bd][ch].step[advStepNo].faultDeltaI_T !=0){
				if(myData->mData.config.function[F_DELTA_V_I] != 0){
					if(labs(refV - myCh->op.Vsens) < maxV*0.001){
						if(labs(myCh->op.Isens - myCh->misc.minI) >= 
							myPs->testCond[bd][ch].step[advStepNo].faultDeltaI){
							diff = myCh->op.runTime - myCh->misc.deltaI_timeout;
							if(diff >= myPs->testCond[bd][ch].step[advStepNo].faultDeltaI_T){
								myCh->misc.deltaI_timeout = myCh->op.runTime;
								myCh->misc.tmpState = myCh->op.state;
								myCh->misc.tmpCode = myCh->op.code;
								if(myData->mData.config.function[F_DELTA_V_I] == 1){
									myCh->op.code = C_STOP_DELTA_I;
									rtn = FAULT_COND;
									//rtn = END_COND;
								}else if(myData->mData.config.function[F_DELTA_V_I] == 2){
									myCh->op.code = C_FAULT_DELTA_I;
									rtn = FAULT_COND;
								}
								break;
							}
						}else{	
							myData->bData[bd].cData[ch].misc.deltaI_timeout
									= myData->bData[bd].cData[ch].op.runTime;
						}
					}
				}
			} else if(myPs->testCond[bd][ch].step[advStepNo].faultDeltaI != 0){
				if(myData->mData.config.function[F_DELTA_V_I] != 0){
					if(labs(refV - myCh->op.Vsens) < maxV*0.001){
						if(labs(myCh->op.Isens - myCh->misc.minI) >= 
							myPs->testCond[bd][ch].step[advStepNo].faultDeltaI){
							myCh->misc.errCnt[C_CNT_DELTA_I_STEP] ++;
							if(myCh->misc.errCnt[C_CNT_DELTA_I_STEP] >
														MAX_ERROR_CNT){
								myCh->misc.errCnt[C_CNT_DELTA_I_STEP] = 0;
								myCh->misc.tmpState = myCh->op.state;
								myCh->misc.tmpCode = myCh->op.code;
								if(myData->mData.config.function[F_DELTA_V_I] == 1){
									myCh->op.code = C_STOP_DELTA_I;
									rtn = FAULT_COND;
									//rtn = END_COND;
								}else if(myData->mData.config.function[F_DELTA_V_I] == 2){
									myCh->op.code = C_FAULT_DELTA_I;
									rtn = FAULT_COND;
								}
								break;
							}
						}else{	
							myCh->misc.errCnt[C_CNT_DELTA_I_STEP] = 0;
						}
					}
				}
			}
			
			//20180314 sch modify
			if(myPs->config.LimitVI.limit_use_I != 0 
				|| myPs->config.LimitVI.limit_use_V != 0){
				rtn = Fault_Check_default_LG(bd, ch);
				if(rtn == FAULT_COND)
					break;
			}
			//19090l lyhw
			if(myPs->config.function[F_CHANGE_VI_CHECK] != 0){
				rtn = Fault_Check_Change_VI(bd, ch, advStepNo);
				if(rtn == FAULT_COND)	break;
			}
			//210923 lyhw add for lges gas control
			if(GAS_DATA_CONTROL == 1){
				rtn = Fault_Check_Gas_Data(bd, ch, advStepNo);
				if(rtn == FAULT_COND)	break;
			}
#ifdef _GROUP_ERROR
			if(myCh->op.Vsens >= myPs->testCond[bd][ch].step[advStepNo].group_StartVoltage){
				myCh->misc.group_StartVoltage_flag = P1;
			}
#endif
			break;
		case STEP_DISCHARGE:
			//100908 pjy temp fault check without delay
			if(myPs->testCond[bd][ch].safety.faultUpperTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 0);
				if(rtn == FAULT_COND)
					break;
			}
			if(myPs->testCond[bd][ch].safety.faultLowerTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 1);
				if(rtn == FAULT_COND)
					break;
			}
			if(myPs->testCond[bd][ch].step[advStepNo].faultUpperTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 2);
				if(rtn == FAULT_COND)
					break;
			}
			if(myPs->testCond[bd][ch].step[advStepNo].faultLowerTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 3);
				if(rtn == FAULT_COND)
					break;
			}
			//190311 lyh add
			if(myTestCond->step[advStepNo].pauseUpperTemp != 0) {
				rtn = tempPauseCheck(bd, ch, advStepNo, 0);
				if(rtn == FAULT_COND)
					break;
			}
			if(myTestCond->step[advStepNo].pauseLowerTemp != 0) {
				rtn = tempPauseCheck(bd, ch, advStepNo, 1);
				if(rtn == FAULT_COND)
					break;
			}

			//hun_200219_s
			#ifdef _SDI_SAFETY_V1	
			if(myTestCond->step[advStepNo].faultRunTime != 0) {
				if(myCh->op.runTime >= myTestCond->step[advStepNo].faultRunTime){
					myCh->misc.tmpState = myCh->op.state;
					myCh->misc.tmpCode = myCh->op.code;
					myCh->op.code = C_FAULT_RUN_TIME;
					Fault_Value_Check(myCh->misc.advStepNo, 
					myCh->op.runTime,
					myPs->testCond[bd][ch].step[advStepNo].faultRunTime,
					0, 0, 0, 0, 0, 0, 0);
					rtn = FAULT_COND;
					break;
				}
			}
			#endif
			#ifdef _SDI_SAFETY_V2	
			//210417 LJS for MASTER RECIPE
			if(myCh->misc.MasterFlag == 1 || myCh->misc.MasterFlag == 3){
				if(myPs->testCond[bd][ch].safety.Master_Recipe_V != 0 &&
							myCh->misc.deltaV_cnt == 0){
					if(myCh->misc.Master_recipe_deltaV - myCh->op.Vsens >= 
						myPs->testCond[bd][ch].safety.Master_Recipe_V){
						myCh->misc.deltaV_cnt = 1;
					}
				}
				if(myCh->misc.deltaV_cnt == 0 && myCh->op.runTime >= 100){
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_CABLE_LINE_CHECK;
						rtn = FAULT_COND;
						break;
				}
			}
			
			#endif
			//hun_200219_e
			//200706
			#if CHANGE_VI_CHECK == 1
			if(myTestCond->step[advStepNo].faultRunTime != 0) {
				if(myCh->op.runTime >= myTestCond->step[advStepNo].faultRunTime){
					myCh->misc.tmpState = myCh->op.state;
					myCh->misc.tmpCode = myCh->op.code;
					myCh->op.code = C_FAULT_RUN_TIME;
					rtn = FAULT_COND;
					break;
				}
			}
			#endif
			//kjc_211028
			#ifdef _ULSAN_SDI_SAFETY
			if(myTestCond->step[advStepNo].humpSet_T != 0 
				&& myTestCond->step[advStepNo].humpSet_I != 0){
				if(myCh->misc.cvFlag == P1){
					if(myCh->misc.cvTime_Ulsan <= 100){
						myCh->misc.humpComp_T = myCh->op.runTime;
						myCh->misc.humpComp_I = myCh->op.Isens;

						myCh->misc.humpCheck_T = myCh->misc.humpComp_T 
							+ myTestCond->step[advStepNo].humpSet_T;
					}
					if(myCh->misc.cvTime_Ulsan > 100){
						if(myCh->op.runTime >= myCh->misc.humpCheck_T){
							myCh->misc.humpCheck_I
								= myCh->misc.humpComp_I - myCh->op.Isens;

							myCh->misc.humpComp_T = myCh->op.runTime;
							myCh->misc.humpComp_I = myCh->op.Isens;

							myCh->misc.humpCheck_T = myCh->misc.humpComp_T 
								+ myTestCond->step[advStepNo].humpSet_T;

							if(myCh->misc.humpCheck_I 
								>= myTestCond->step[advStepNo].humpSet_I){
								myCh->misc.tmpState = myCh->op.state;
								myCh->misc.tmpCode = myCh->op.code;
								myCh->misc.cvFlag = 0;
								myCh->misc.cvTime_Ulsan = 0;
								myCh->op.code = C_FAULT_CV_CURRENT_HUMP;
								rtn = FAULT_COND;
								break;
							}
						}
					}
				}
			}
			#endif

			if(myPs->testCond[bd][ch].safety.faultUpperV != 0) {
				if(myCh->op.Vsens
					>= myPs->testCond[bd][ch].safety.faultUpperV) {
					myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_VOLTAGE;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); //safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Vsens,
						myPs->testCond[bd][ch].safety.faultUpperV,
					   	0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE] = 0;
				}
			}
			if(myPs->testCond[bd][ch].safety.faultLowerV != 0) {
				if(myCh->op.Vsens
					<= myPs->testCond[bd][ch].safety.faultLowerV) {
					myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE]++;
					if(myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_LOWER_VOLTAGE;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); //safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Vsens,
						myPs->testCond[bd][ch].safety.faultLowerV,
					   	0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE] = 0;
				}
			}
			if(myCh->op.checkDelayTime < 180) break;
			if(faultUpperI_com != 0) {
				if(myCh->op.Isens <= faultUpperI_com * (-1)) {
					myCh->misc.errCnt[C_CNT_UPPER_CURRENT]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_CURRENT]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_UPPER_CURRENT] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_CURRENT;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); //safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Isens,
						faultUpperI_com * (-1), 0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_UPPER_CURRENT] = 0;
				}
			}
			if(faultLowerI_com != 0) {
				if(myCh->op.Isens >= faultLowerI_com * (-1)) {
					myCh->misc.errCnt[C_CNT_LOWER_CURRENT]++;
					if(myCh->misc.errCnt[C_CNT_LOWER_CURRENT]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_LOWER_CURRENT] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_LOWER_CURRENT;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); //safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Isens,
						faultLowerI_com * (-1), 0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_LOWER_CURRENT] = 0;
				}
			}
			if(myPs->testCond[bd][ch].safety.faultUpperC != 0) {
				if(capacity >= myPs->testCond[bd][ch].safety.faultUpperC) {
					myCh->misc.errCnt[C_CNT_UPPER_CAPACITY]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_CAPACITY]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_UPPER_CAPACITY] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_CAPACITY;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); //safety
						Fault_Value_Check(myCh->misc.advStepNo, capacity,
						myPs->testCond[bd][ch].safety.faultUpperC,
					   	0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_UPPER_CAPACITY] = 0;
				}
			}
			if(myPs->testCond[bd][ch].step[advStepNo].faultUpperV != 0) {
				if(myCh->op.Vsens >=
					myPs->testCond[bd][ch].step[advStepNo].faultUpperV) {
					myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE_STEP]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE_STEP]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE_STEP] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_VOLTAGE;
						myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); //step_safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Vsens,
						myPs->testCond[bd][ch].step[advStepNo].faultUpperV,
					   	0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE_STEP] = 0;
				}
			}
			if(myPs->testCond[bd][ch].step[advStepNo].faultLowerV != 0) {
				if(myCh->op.Vsens <=
					myPs->testCond[bd][ch].step[advStepNo].faultLowerV) {
					myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE_STEP]++;
					if(myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE_STEP]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE_STEP] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_LOWER_VOLTAGE;
						myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); //step_safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Vsens,
						myPs->testCond[bd][ch].step[advStepNo].faultLowerV,
					   	0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE_STEP] = 0;
				}
			}
			if(faultUpperI_step != 0) {
				if(myCh->op.Isens <= faultUpperI_step) {
					myCh->misc.errCnt[C_CNT_UPPER_CURRENT_STEP]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_CURRENT_STEP]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_UPPER_CURRENT_STEP] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_CURRENT;
						myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); //step_safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Isens,
						faultUpperI_step, 0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_UPPER_CURRENT_STEP] = 0;
				}
			}
			if((myCh->op.Vsens - refV) > (maxV * 0.001)){		//kjc_200612
				if(faultLowerI_step != 0) {
					if(myCh->op.Isens >= faultLowerI_step) {
						myCh->misc.errCnt[C_CNT_LOWER_CURRENT_STEP]++;
						if(myCh->misc.errCnt[C_CNT_LOWER_CURRENT_STEP]>= MAX_ERROR_CNT){
							myCh->misc.errCnt[C_CNT_LOWER_CURRENT_STEP] = 0;
							myCh->misc.tmpState = myCh->op.state;
							myCh->misc.tmpCode = myCh->op.code;
							myCh->op.code = C_FAULT_LOWER_CURRENT;
							myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); //step_safety
							Fault_Value_Check(myCh->misc.advStepNo, 
							myCh->op.Isens, faultLowerI_step,
						   	0, 0, 0, 0, 0, 0, 0);
							rtn = FAULT_COND;
							break;
						}
					}else{
						myCh->misc.errCnt[C_CNT_LOWER_CURRENT_STEP] = 0;
					}
				}
			}
			if(myPs->testCond[bd][ch].step[advStepNo].faultUpperC != 0) {
				if(capacity >=
					myPs->testCond[bd][ch].step[advStepNo].faultUpperC) {
					myCh->misc.errCnt[C_CNT_UPPER_CAPACITY_STEP]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_CAPACITY_STEP]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_UPPER_CAPACITY_STEP] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_CAPACITY;
						myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); //step_safety
						Fault_Value_Check(myCh->misc.advStepNo, capacity,
						myPs->testCond[bd][ch].step[advStepNo].faultUpperC,
					   	0, 0, 0, 0, 0, 0 ,0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_UPPER_CAPACITY_STEP] = 0;
				}
			}	
			//170215 SCH add for DeltaV/I DISCHARGE, 180918 rewrite
			if(myPs->testCond[bd][ch].step[advStepNo].faultDeltaV != 0 
				&& myPs->testCond[bd][ch].step[advStepNo].faultDeltaV_T !=0){
				if(myData->mData.config.function[F_DELTA_V_I] != 0){
//					if(labs(refI - myCh->op.Isens) < maxI*0.001){
						if(labs(myCh->op.Vsens - myCh->misc.minV) >= 
							myPs->testCond[bd][ch].step[advStepNo].faultDeltaV){
							diff = myCh->op.runTime - myCh->misc.deltaV_timeout;
							if(diff >= myPs->testCond[bd][ch].step[advStepNo].faultDeltaV_T){
								myCh->misc.deltaV_timeout = myCh->op.runTime;
								myCh->misc.tmpState = myCh->op.state;
								myCh->misc.tmpCode = myCh->op.code;
								if(myData->mData.config.function[F_DELTA_V_I] == 1){
									myCh->op.code = C_STOP_DELTA_V;
									rtn = FAULT_COND;
									//rtn = END_COND;
								}else if(myData->mData.config.function[F_DELTA_V_I] == 2){
									myCh->op.code = C_FAULT_DELTA_V;
									rtn = FAULT_COND;
								}
								break;
							}
						}else{	
							myData->bData[bd].cData[ch].misc.deltaV_timeout
									= myData->bData[bd].cData[ch].op.runTime;
						}
//					}
				}
			} else if(myPs->testCond[bd][ch].step[advStepNo].faultDeltaV != 0){
					if(myData->mData.config.function[F_DELTA_V_I] != 0){
						if(labs(refI - myCh->op.Isens) < maxI*0.001){
							if(labs(myCh->op.Vsens - myCh->misc.minV) >= 
								myPs->testCond[bd][ch].step[advStepNo].faultDeltaV){
								myCh->misc.errCnt[C_CNT_DELTA_V_STEP] ++;
								if(myCh->misc.errCnt[C_CNT_DELTA_V_STEP] >
															MAX_ERROR_CNT){
									myCh->misc.errCnt[C_CNT_DELTA_V_STEP] = 0;
									myCh->misc.tmpState = myCh->op.state;
									myCh->misc.tmpCode = myCh->op.code;
									if(myData->mData.config.function[F_DELTA_V_I] == 1){
										myCh->op.code = C_STOP_DELTA_V;
										rtn = FAULT_COND;
										//rtn = END_COND;
									}else if(myData->mData.config.function[F_DELTA_V_I] == 2){
										myCh->op.code = C_FAULT_DELTA_V;
										rtn = FAULT_COND;
									}
									break;
								}
							}else{	
								myCh->misc.errCnt[C_CNT_DELTA_V_STEP] = 0;
							}
						}
					}
				}

			//current
			if(myPs->testCond[bd][ch].step[advStepNo].faultDeltaI != 0 
				&& myPs->testCond[bd][ch].step[advStepNo].faultDeltaI_T !=0){
				if(myData->mData.config.function[F_DELTA_V_I] != 0){
					if(labs(refV - myCh->op.Vsens) < maxV*0.001){
						if(labs(myCh->misc.maxI - myCh->op.Isens) >= 
							myPs->testCond[bd][ch].step[advStepNo].faultDeltaI){
							diff = myCh->op.runTime - myCh->misc.deltaI_timeout;
							if(diff >= myPs->testCond[bd][ch].step[advStepNo].faultDeltaI_T){
								myCh->misc.deltaI_timeout = myCh->op.runTime;
								myCh->misc.tmpState = myCh->op.state;
								myCh->misc.tmpCode = myCh->op.code;
								if(myData->mData.config.function[F_DELTA_V_I] == 1){
									myCh->op.code = C_STOP_DELTA_I;
									rtn = FAULT_COND;
									//rtn = END_COND;
								}else if(myData->mData.config.function[F_DELTA_V_I] == 2){
									myCh->op.code = C_FAULT_DELTA_I;
									rtn = FAULT_COND;
								}
								break;
							}
						}else{	
							myData->bData[bd].cData[ch].misc.deltaI_timeout
									= myData->bData[bd].cData[ch].op.runTime;
						}
					}
				}
			} else if(myPs->testCond[bd][ch].step[advStepNo].faultDeltaI != 0){
					if(myData->mData.config.function[F_DELTA_V_I] != 0){
						if(labs(refV - myCh->op.Vsens) < maxV*0.001){
							if(labs(myCh->misc.maxI - myCh->op.Isens) >= 
								myPs->testCond[bd][ch].step[advStepNo].faultDeltaI){
								myCh->misc.errCnt[C_CNT_DELTA_I_STEP] ++;
								if(myCh->misc.errCnt[C_CNT_DELTA_I_STEP] >
															MAX_ERROR_CNT){
									myCh->misc.errCnt[C_CNT_DELTA_I_STEP] = 0;
									myCh->misc.tmpState = myCh->op.state;
									myCh->misc.tmpCode = myCh->op.code;
									if(myData->mData.config.function[F_DELTA_V_I] == 1){
										myCh->op.code = C_STOP_DELTA_I;
										rtn = FAULT_COND;
										//rtn = END_COND;
									}else if(myData->mData.config.function[F_DELTA_V_I] == 2){
										myCh->op.code = C_FAULT_DELTA_I;
										rtn = FAULT_COND;
									}
									break;
								}
							}else{	
								myCh->misc.errCnt[C_CNT_DELTA_I_STEP] = 0;
							}
						}
					}
				}

			//19090l lyhw
			if(myPs->config.function[F_CHANGE_VI_CHECK] != 0){
				rtn = Fault_Check_Change_VI(bd, ch, advStepNo);
				if(rtn == FAULT_COND)	break;
			}
			//210923 lyhw add for lges gas control
			if(GAS_DATA_CONTROL == 1){
				rtn = Fault_Check_Gas_Data(bd, ch, advStepNo);
				if(rtn == FAULT_COND)	break;
			}
#ifdef _GROUP_ERROR
			if(myCh->op.Vsens <= myPs->testCond[bd][ch].step[advStepNo].group_StartVoltage){
				myCh->misc.group_StartVoltage_flag = P1;
			}
#endif
/*
			//20180314 sch modify
			if(myPs->config.LimitVI.limit_use_I != 0){
				rtn = Fault_Check_default_LG(bd, ch);
				if(rtn == FAULT_COND)
					break;
			}
			if(myPs->config.LimitVI.limit_use_V != 0){
				rtn = Fault_Check_default_LG(bd, ch);
				if(rtn == FAULT_COND)
					break;
			}
*/
			break;
		case STEP_Z:
			//100908 pjy temp fault check without delay
			if(myPs->testCond[bd][ch].safety.faultUpperTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 0);
				if(rtn == FAULT_COND)
					break;
			}
			if(myPs->testCond[bd][ch].safety.faultLowerTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 1);
				if(rtn == FAULT_COND)
					break;
			}
			if(myPs->testCond[bd][ch].step[advStepNo].faultUpperTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 2);
				if(rtn == FAULT_COND)
					break;
			}
			if(myPs->testCond[bd][ch].step[advStepNo].faultLowerTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 3);
				if(rtn == FAULT_COND)
					break;
			}
			//190311 lyh add
			if(myTestCond->step[advStepNo].pauseUpperTemp != 0) {
				rtn = tempPauseCheck(bd, ch, advStepNo, 0);
				if(rtn == FAULT_COND)
					break;
			}
			if(myTestCond->step[advStepNo].pauseLowerTemp != 0) {
				rtn = tempPauseCheck(bd, ch, advStepNo, 1);
				if(rtn == FAULT_COND)
					break;
			}
			if(myPs->testCond[bd][ch].safety.faultUpperV != 0) {
				if(myCh->op.Vsens
					>= myPs->testCond[bd][ch].safety.faultUpperV) {
					myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_VOLTAGE;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); //safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Vsens,
						myPs->testCond[bd][ch].safety.faultUpperV,
					   	0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE] = 0;
				}
			}
			if(myPs->testCond[bd][ch].safety.faultLowerV != 0) {
				if(myCh->op.Vsens
					<= myPs->testCond[bd][ch].safety.faultLowerV) {
					myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE]++;
					if(myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_LOWER_VOLTAGE;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); //safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Vsens,
						myPs->testCond[bd][ch].safety.faultLowerV,
					   	0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE] = 0;
				}
			}
			if(myCh->op.checkDelayTime < 180) break;
			if(faultUpperI_com != 0) {
				if(myCh->op.Isens <= faultUpperI_com * (-1)) {
					myCh->misc.errCnt[C_CNT_UPPER_CURRENT]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_CURRENT]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_UPPER_CURRENT] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_CURRENT;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); //safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Isens,
						faultUpperI_com * (-1), 0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_UPPER_CURRENT] = 0;
				}
			}
			if(faultLowerI_com != 0) {
				if(myCh->op.Isens >= faultLowerI_com * (-1)) {
					myCh->misc.errCnt[C_CNT_LOWER_CURRENT]++;
					if(myCh->misc.errCnt[C_CNT_LOWER_CURRENT]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_LOWER_CURRENT] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_LOWER_CURRENT;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); //safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Isens,
						faultLowerI_com * (-1), 0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_LOWER_CURRENT] = 0;
				}
			}
			if(myPs->testCond[bd][ch].step[advStepNo].faultUpperV != 0) {
				if(myCh->op.Vsens >=
					myPs->testCond[bd][ch].step[advStepNo].faultUpperV) {
					myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE_STEP]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE_STEP]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE_STEP] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_VOLTAGE;
						myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); //step_safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Vsens,
						myPs->testCond[bd][ch].step[advStepNo].faultUpperV,
					   	0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE_STEP] = 0;
				}
			}
			if(myPs->testCond[bd][ch].step[advStepNo].faultLowerV != 0) {
				if(myCh->op.Vsens <=
					myPs->testCond[bd][ch].step[advStepNo].faultLowerV) {
					myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE_STEP]++;
					if(myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE_STEP]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE_STEP] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_LOWER_VOLTAGE;
						myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); //step_safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Vsens,
						myPs->testCond[bd][ch].step[advStepNo].faultLowerV,
					   	0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE_STEP] = 0;
				}
			}
			if(faultUpperI_step != 0) {
				if(myCh->op.Isens <= faultUpperI_step) {
					myCh->misc.errCnt[C_CNT_UPPER_CURRENT_STEP]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_CURRENT_STEP]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_UPPER_CURRENT_STEP] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_CURRENT;
						myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); //step_safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Isens,
						faultUpperI_step, 0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_UPPER_CURRENT_STEP] = 0;
				}
			}
			if((myCh->op.Vsens - refV) > (maxV * 0.001)){		//kjc_200612
				if(faultLowerI_step != 0) {
					if(myCh->op.Isens >= faultLowerI_step) {
						myCh->misc.errCnt[C_CNT_LOWER_CURRENT_STEP]++;
						if(myCh->misc.errCnt[C_CNT_LOWER_CURRENT_STEP]>= MAX_ERROR_CNT){
							myCh->misc.errCnt[C_CNT_LOWER_CURRENT_STEP] = 0;
							myCh->misc.tmpState = myCh->op.state;
							myCh->misc.tmpCode = myCh->op.code;
							myCh->op.code = C_FAULT_LOWER_CURRENT;
							myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); //step_safety
							Fault_Value_Check(myCh->misc.advStepNo, 
							myCh->op.Isens, faultLowerI_step,
						   	0, 0, 0, 0, 0, 0, 0);
							rtn = FAULT_COND;
							break;
						}
					}else{
						myCh->misc.errCnt[C_CNT_LOWER_CURRENT_STEP] = 0;
					}
				}
			}
			//20180314 sch modify
			if(myPs->config.LimitVI.limit_use_I != 0 
				|| myPs->config.LimitVI.limit_use_V != 0){
				rtn = Fault_Check_default_LG(bd, ch);
				if(rtn == FAULT_COND)
					break;
			}
			//19090l lyhw
			if(myPs->config.function[F_CHANGE_VI_CHECK] != 0){
				rtn = Fault_Check_Change_VI(bd, ch, advStepNo);
				if(rtn == FAULT_COND)	break;
			}
			//210923 lyhw add for lges gas control
			if(GAS_DATA_CONTROL == 1){
				rtn = Fault_Check_Gas_Data(bd, ch, advStepNo);
				if(rtn == FAULT_COND)	break;
			}
			break;
		case STEP_USER_PATTERN:
			//100908 pjy temp fault check without delay
			if(myPs->testCond[bd][ch].safety.faultUpperTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 0);
				if(rtn == FAULT_COND)
					break;
			}
			if(myPs->testCond[bd][ch].safety.faultLowerTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 1);
				if(rtn == FAULT_COND)
					break;
			}
			if(myPs->testCond[bd][ch].step[advStepNo].faultUpperTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 2);
				if(rtn == FAULT_COND)
					break;
			}
			if(myPs->testCond[bd][ch].step[advStepNo].faultLowerTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 3);
				if(rtn == FAULT_COND)
					break;
			}
			//190311 lyh add
			if(myTestCond->step[advStepNo].pauseUpperTemp != 0) {
				rtn = tempPauseCheck(bd, ch, advStepNo, 0);
				if(rtn == FAULT_COND)
					break;
			}
			if(myTestCond->step[advStepNo].pauseLowerTemp != 0) {
				rtn = tempPauseCheck(bd, ch, advStepNo, 1);
				if(rtn == FAULT_COND)
					break;
			}
			if(myPs->testCond[bd][ch].safety.faultUpperV != 0) {
				if(myCh->op.Vsens
					>= myPs->testCond[bd][ch].safety.faultUpperV) {
					myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_VOLTAGE;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); //safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Vsens,
						myPs->testCond[bd][ch].safety.faultUpperV,
					   	0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE] = 0;
				}
			}
			if(myPs->testCond[bd][ch].safety.faultLowerV != 0) {
				if(myCh->op.Vsens
					<= myPs->testCond[bd][ch].safety.faultLowerV) {
					myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE]++;
					if(myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_LOWER_VOLTAGE;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); //safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Vsens,
						myPs->testCond[bd][ch].safety.faultLowerV,
					   	0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE] = 0;
				}
			}
			if(myCh->op.checkDelayTime < 180) break;
			if(faultUpperI_com != 0) {
				if(myCh->op.Isens >= faultUpperI_com) {
					myCh->misc.errCnt[C_CNT_UPPER_CURRENT]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_CURRENT]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_UPPER_CURRENT] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_CURRENT;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); //safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Isens,
						faultUpperI_com, 0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_UPPER_CURRENT] = 0;
				}
			}
			if(faultLowerI_com != 0) {
				if(myCh->op.Isens <= faultLowerI_com) {
					myCh->misc.errCnt[C_CNT_LOWER_CURRENT]++;
					if(myCh->misc.errCnt[C_CNT_LOWER_CURRENT]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_LOWER_CURRENT] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_LOWER_CURRENT;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); //safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Isens,
						faultLowerI_com, 0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_LOWER_CURRENT] = 0;
				}
			}
			if(myPs->testCond[bd][ch].safety.faultUpperC != 0) {
				if(capacity >= myPs->testCond[bd][ch].safety.faultUpperC) {
					myCh->misc.errCnt[C_CNT_UPPER_CAPACITY]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_CAPACITY]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_UPPER_CAPACITY] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_CAPACITY;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); //safety
						Fault_Value_Check(myCh->misc.advStepNo, capacity,
						myPs->testCond[bd][ch].safety.faultUpperC,
					   	0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_UPPER_CAPACITY] = 0;
				}
			}
			if(myPs->testCond[bd][ch].step[advStepNo].faultUpperV != 0) {
				if(myCh->op.Vsens >=
					myPs->testCond[bd][ch].step[advStepNo].faultUpperV) {
					myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE_STEP]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE_STEP]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE_STEP] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_VOLTAGE;
						myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); //step_safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Vsens,
						myPs->testCond[bd][ch].step[advStepNo].faultUpperV,
					   	0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE_STEP] = 0;
				}
			}
			if(myPs->testCond[bd][ch].step[advStepNo].faultLowerV != 0) {
				if(myCh->op.Vsens <=
					myPs->testCond[bd][ch].step[advStepNo].faultLowerV) {
					myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE_STEP]++;
					if(myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE_STEP]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE_STEP] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_LOWER_VOLTAGE;
						myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); //step_safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Vsens,
						myPs->testCond[bd][ch].step[advStepNo].faultLowerV,
					   	0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE_STEP] = 0;
				}
			}
			if(faultUpperI_step != 0) {
				if(myCh->op.Isens >= faultUpperI_step) {
					myCh->misc.errCnt[C_CNT_UPPER_CURRENT_STEP]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_CURRENT_STEP]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_UPPER_CURRENT_STEP] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_CURRENT;
						myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); //step_safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Isens,
						faultUpperI_step, 0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_UPPER_CURRENT_STEP] = 0;
				}
			}
			if(faultLowerI_step != 0) {
				if(myCh->op.Isens <= faultLowerI_step) {
					myCh->misc.errCnt[C_CNT_LOWER_CURRENT_STEP]++;
					if(myCh->misc.errCnt[C_CNT_LOWER_CURRENT_STEP]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_LOWER_CURRENT_STEP] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_LOWER_CURRENT;
						myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); //step_safety
						Fault_Value_Check(myCh->misc.advStepNo, 
						myCh->op.Isens, faultLowerI_step,
					   	0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_LOWER_CURRENT_STEP] = 0;
				}
			}
			if(myPs->testCond[bd][ch].step[advStepNo].faultUpperC != 0) {
				if(capacity >=
					myPs->testCond[bd][ch].step[advStepNo].faultUpperC) {
					myCh->misc.errCnt[C_CNT_UPPER_CAPACITY_STEP]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_CAPACITY_STEP]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_UPPER_CAPACITY_STEP] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_CAPACITY;
						myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); //step_safety
						Fault_Value_Check(myCh->misc.advStepNo, capacity,
						myPs->testCond[bd][ch].step[advStepNo].faultUpperC,
					   	0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_UPPER_CAPACITY_STEP] = 0;
				}
			}
			#if CHANGE_VI_CHECK == 1
			if(myTestCond->step[advStepNo].faultRunTime != 0) {
				if(myCh->op.runTime >= myTestCond->step[advStepNo].faultRunTime){
					myCh->misc.tmpState = myCh->op.state;
					myCh->misc.tmpCode = myCh->op.code;
					myCh->op.code = C_FAULT_RUN_TIME;
					rtn = FAULT_COND;
					break;
				}
			}
			#endif
			//170215 SCH add for DeltaV/I USER_PATTERN
/*			if(myPs->testCond[bd][ch].step[advStepNo].faultDeltaV != 0){
				if(myData->mData.config.function[F_DELTA_V_I] == 0){
				
				}else{
					if(labs(myCh->misc.maxV - myCh->op.Vsens) >= 
						myPs->testCond[bd][ch].step[advStepNo].faultDeltaV){
						myCh->misc.errCnt[C_CNT_DELTA_V_STEP]++;
						if(myCh->misc.errCnt[C_CNT_DELTA_V_STEP] > 
													MAX_ERROR_CNT){
							myCh->misc.errCnt[C_CNT_DELTA_V_STEP] = 0;
							myCh->misc.tmpState = myCh->op.state;
							myCh->misc.tmpCode = myCh->op.code;
							if(myData->mData.config.function[F_DELTA_V_I] == 1){
								myCh->op.code = C_STOP_DELTA_V;
								rtn = FAULT_COND;
								//rtn = END_COND;
							}else if(myData->mData.config.function[F_DELTA_V_I] == 2){
								myCh->op.code = C_FAULT_DELTA_V;
								rtn = FAULT_COND;
							}
							break;
						}
					}else{	
						myCh->misc.errCnt[C_CNT_DELTA_V_STEP] = 0;
					}
				}
			}
			if(myPs->testCond[bd][ch].step[advStepNo].faultDeltaI != 0){
				if(myData->mData.config.function[F_DELTA_V_I] == 0){
				
				}else{
					if(labs(myCh->misc.maxI - myCh->op.Isens) >= 
						myPs->testCond[bd][ch].step[advStepNo].faultDeltaI){
						myCh->misc.errCnt[C_CNT_DELTA_V_STEP]++;
						if(myCh->misc.errCnt[C_CNT_DELTA_V_STEP] > 
													MAX_ERROR_CNT){
							myCh->misc.errCnt[C_CNT_DELTA_I_STEP] = 0;
							myCh->misc.tmpState = myCh->op.state;
							myCh->misc.tmpCode = myCh->op.code;
							if(myData->mData.config.function[F_DELTA_V_I] == 1){
								myCh->op.code = C_STOP_DELTA_I;
								rtn = FAULT_COND;
								//rtn = END_COND;
							}else if(myData->mData.config.function[F_DELTA_V_I] == 2){
								myCh->op.code = C_FAULT_DELTA_I;
								rtn = FAULT_COND;
							}
							break;
						}
					}else{	
						myCh->misc.errCnt[C_CNT_DELTA_V_STEP] = 0;
					}
				}
			}
*/
			//20180314 sch modify
			if(myPs->config.LimitVI.limit_use_I != 0 
				|| myPs->config.LimitVI.limit_use_V != 0){
				rtn = Fault_Check_default_LG(bd, ch);
				if(rtn == FAULT_COND)
					break;
			}
			//19090l lyhw
			if(myPs->config.function[F_CHANGE_VI_CHECK] != 0){
				rtn = Fault_Check_Change_VI(bd, ch, advStepNo);
				if(rtn == FAULT_COND)	break;
			}
			//210923 lyhw add for lges gas control
			if(GAS_DATA_CONTROL == 1){
				rtn = Fault_Check_Gas_Data(bd, ch, advStepNo);
				if(rtn == FAULT_COND)	break;
			}
			break;
		case STEP_USER_MAP:
			//100908 pjy temp fault check without delay
			if(myPs->testCond[bd][ch].safety.faultUpperTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 0);
				if(rtn == FAULT_COND)
					break;
			}
			if(myPs->testCond[bd][ch].safety.faultLowerTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 1);
				if(rtn == FAULT_COND)
					break;
			}
			if(myPs->testCond[bd][ch].step[advStepNo].faultUpperTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 2);
				if(rtn == FAULT_COND)
					break;
			}
			if(myPs->testCond[bd][ch].step[advStepNo].faultLowerTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 3);
				if(rtn == FAULT_COND)
					break;
			}
			//190311 lyh add
			if(myTestCond->step[advStepNo].pauseUpperTemp != 0) {
				rtn = tempPauseCheck(bd, ch, advStepNo, 0);
				if(rtn == FAULT_COND)
					break;
			}
			if(myTestCond->step[advStepNo].pauseLowerTemp != 0) {
				rtn = tempPauseCheck(bd, ch, advStepNo, 1);
				if(rtn == FAULT_COND)
					break;
			}
			if(myPs->testCond[bd][ch].safety.faultUpperV != 0) {
				if(myCh->op.Vsens
					>= myPs->testCond[bd][ch].safety.faultUpperV) {
					myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_VOLTAGE;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); //safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Vsens,
						myPs->testCond[bd][ch].safety.faultUpperV,
					   	0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE] = 0;
				}
			}
			if(myPs->testCond[bd][ch].safety.faultLowerV != 0) {
				if(myCh->op.Vsens
					<= myPs->testCond[bd][ch].safety.faultLowerV) {
					myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE]++;
					if(myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_LOWER_VOLTAGE;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); //safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Vsens,
						myPs->testCond[bd][ch].safety.faultLowerV,
					   	0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE] = 0;
				}
			}
			if(myCh->op.checkDelayTime < 180) break;
			if(faultUpperI_com != 0) {
				if(myCh->op.Isens >= faultUpperI_com) {
					myCh->misc.errCnt[C_CNT_UPPER_CURRENT]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_CURRENT]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_UPPER_CURRENT] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_CURRENT;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); //safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Isens,
						faultUpperI_com, 0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_UPPER_CURRENT] = 0;
				}
			}
			if(faultLowerI_com != 0) {
				if(myCh->op.Isens <= faultLowerI_com) {
					myCh->misc.errCnt[C_CNT_LOWER_CURRENT]++;
					if(myCh->misc.errCnt[C_CNT_LOWER_CURRENT]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_LOWER_CURRENT] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_LOWER_CURRENT;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); //safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Isens,
						faultLowerI_com, 0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_LOWER_CURRENT] = 0;
				}
			}
			if(myPs->testCond[bd][ch].safety.faultUpperC != 0) {
				if(labs(capacity) >= myPs->testCond[bd][ch].safety.faultUpperC) {
					myCh->misc.errCnt[C_CNT_UPPER_CAPACITY]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_CAPACITY]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_UPPER_CAPACITY] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_CAPACITY;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); //safety
						Fault_Value_Check(myCh->misc.advStepNo, capacity,
						myPs->testCond[bd][ch].safety.faultUpperC,
					   	0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_UPPER_CAPACITY] = 0;
				}
			}
			if(myPs->testCond[bd][ch].step[advStepNo].faultUpperV != 0) {
				if(myCh->op.Vsens >=
					myPs->testCond[bd][ch].step[advStepNo].faultUpperV) {
					myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE_STEP]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE_STEP]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE_STEP] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_VOLTAGE;
						myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); //step_safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Vsens,
						myPs->testCond[bd][ch].step[advStepNo].faultUpperV,
					   	0, 0, 0, 0, 0, 0 ,0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE_STEP] = 0;
				}
			}
			if(myPs->testCond[bd][ch].step[advStepNo].faultLowerV != 0) {
				if(myCh->op.Vsens <=
					myPs->testCond[bd][ch].step[advStepNo].faultLowerV) {
					myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE_STEP]++;
					if(myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE_STEP]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE_STEP] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_LOWER_VOLTAGE;
						myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); //step_safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Vsens,
						myPs->testCond[bd][ch].step[advStepNo].faultLowerV,
					   	0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE_STEP] = 0;
				}
			}
			if(faultUpperI_step != 0) {
				if(myCh->op.Isens >= faultUpperI_step) {
					myCh->misc.errCnt[C_CNT_UPPER_CURRENT_STEP]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_CURRENT_STEP]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_UPPER_CURRENT_STEP] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_CURRENT;
						myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); //step_safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Isens,
						faultUpperI_step, 0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_UPPER_CURRENT_STEP] = 0;
				}
			}
			if(faultLowerI_step != 0) {
				if(myCh->op.Isens <= faultLowerI_step) {
					myCh->misc.errCnt[C_CNT_LOWER_CURRENT_STEP]++;
					if(myCh->misc.errCnt[C_CNT_LOWER_CURRENT_STEP]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_LOWER_CURRENT_STEP] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_LOWER_CURRENT;
						myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); //step_safety
						Fault_Value_Check(myCh->misc.advStepNo, 
						myCh->op.Isens, faultLowerI_step, 0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_LOWER_CURRENT_STEP] = 0;
				}
			}
			if(myPs->testCond[bd][ch].step[advStepNo].faultUpperC != 0) {
				if(labs(capacity) >=
					myPs->testCond[bd][ch].step[advStepNo].faultUpperC) {
					myCh->misc.errCnt[C_CNT_UPPER_CAPACITY_STEP]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_CAPACITY_STEP]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_UPPER_CAPACITY_STEP] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_CAPACITY;
						myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); //step_safety
						Fault_Value_Check(myCh->misc.advStepNo, capacity,
						myPs->testCond[bd][ch].step[advStepNo].faultUpperC,
					   	0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_UPPER_CAPACITY_STEP] = 0;
				}
			}
/*			//170215 SCH add for DeltaV/I USER_MAP
			if(myPs->testCond[bd][ch].step[advStepNo].faultDeltaV != 0){
				if(myData->mData.config.function[F_DELTA_V_I] == 0){
				
				}else{
					if(labs(myCh->misc.maxV - myCh->op.Vsens) >= 
						myPs->testCond[bd][ch].step[advStepNo].faultDeltaV){
						myCh->misc.errCnt[C_CNT_DELTA_V_STEP]++;
						if(myCh->misc.errCnt[C_CNT_DELTA_V_STEP] > 
													MAX_ERROR_CNT){
							myCh->misc.errCnt[C_CNT_DELTA_V_STEP] = 0;
							myCh->misc.tmpState = myCh->op.state;
							myCh->misc.tmpCode = myCh->op.code;
							if(myData->mData.config.function[F_DELTA_V_I] == 1){
								myCh->op.code = C_STOP_DELTA_V;
								rtn = FAULT_COND;
								//rtn = END_COND;
							}else if(myData->mData.config.function[F_DELTA_V_I] == 2){
								myCh->op.code = C_FAULT_DELTA_V;
								rtn = FAULT_COND;
							}
							break;
						}
					}else{	
						myCh->misc.errCnt[C_CNT_DELTA_V_STEP] = 0;
					}
				}
			}
			if(myPs->testCond[bd][ch].step[advStepNo].faultDeltaI != 0){
				if(myData->mData.config.function[F_DELTA_V_I] == 0){
				
				}else{
					if(labs(myCh->misc.maxI - myCh->op.Isens) >= 
						myPs->testCond[bd][ch].step[advStepNo].faultDeltaI){
						myCh->misc.errCnt[C_CNT_DELTA_V_STEP]++;
						if(myCh->misc.errCnt[C_CNT_DELTA_V_STEP] > 
													MAX_ERROR_CNT){
							myCh->misc.errCnt[C_CNT_DELTA_I_STEP] = 0;
							myCh->misc.tmpState = myCh->op.state;
							myCh->misc.tmpCode = myCh->op.code;
							if(myData->mData.config.function[F_DELTA_V_I] == 1){
								myCh->op.code = C_STOP_DELTA_I;
								rtn = FAULT_COND;
								//rtn = END_COND;
							}else if(myData->mData.config.function[F_DELTA_V_I] == 2){
								myCh->op.code = C_FAULT_DELTA_I;
								rtn = FAULT_COND;
							}
							break;
						}
					}else{	
						myCh->misc.errCnt[C_CNT_DELTA_V_STEP] = 0;
					}
				}
			}
*/
			//20180314 sch modify
			if(myPs->config.LimitVI.limit_use_I != 0 
				|| myPs->config.LimitVI.limit_use_V != 0){
				rtn = Fault_Check_default_LG(bd, ch);
				if(rtn == FAULT_COND)
					break;
			}
			//19090l lyhw
			if(myPs->config.function[F_CHANGE_VI_CHECK] != 0){
				rtn = Fault_Check_Change_VI(bd, ch, advStepNo);
				if(rtn == FAULT_COND)	break;
			}
			//210923 lyhw add for lges gas control
			if(GAS_DATA_CONTROL == 1){
				rtn = Fault_Check_Gas_Data(bd, ch, advStepNo);
				if(rtn == FAULT_COND)	break;
			}
			break;
#if MACHINE_TYPE == 1
		case STEP_SHORT:
			if(myPs->testCond[bd][ch].safety.faultUpperTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 0);
				if(rtn == FAULT_COND)
					break;
			}
			if(myPs->testCond[bd][ch].safety.faultLowerTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 1);
				if(rtn == FAULT_COND)
					break;
			}
			break;
#endif
		default: 
			break;
	}
	//20171024
#ifdef _USER_VI
	if(myData->mData.config.LimitVI.limit_use_I != 0){
		if(myCh->signal[C_SIG_LIMIT_CURRENT] == P1) { 
			myCh->signal[C_SIG_LIMIT_CURRENT] = P0;
			myCh->misc.tmpState = myCh->op.state;
			myCh->misc.tmpCode = myCh->op.code;
			if(myPs->config.LimitVI.limit_action_I == P0){
				myCh->op.code = C_FAULT_LIMIT_CURRENT;
			}else if(myPs->config.LimitVI.limit_action_I == P1){
				myCh->op.code = C_FAULT_END_LIMIT_CURRENT;
			}
			rtn = FAULT_COND;
		}
	}
	if(myData->mData.config.LimitVI.limit_use_V != 0){
		if(myCh->signal[C_SIG_LIMIT_VOLTAGE] == P1) { //20171024 sch add
			myCh->signal[C_SIG_LIMIT_VOLTAGE] = P0;
			myCh->misc.tmpState = myCh->op.state;
			myCh->misc.tmpCode = myCh->op.code;
			if(myPs->config.LimitVI.limit_action_V == P0){
				myCh->op.code = C_FAULT_LIMIT_VOLTAGE;
			}else if(myPs->config.LimitVI.limit_action_V == P1){
				myCh->op.code = C_FAULT_END_LIMIT_VOLTAGE;
			}
			rtn = FAULT_COND;
		}
	}
#endif

	//210428_hun
	//gasVoltage [uV]
	//GAS_VOLTAGE_MIN [uV] GA_VOLTAGE_MAX [uV]
	//GAS_CHECK_TIME [sec]
	if(rtn != FAULT_COND && myData->mData.config.swFaultConfig[CHAMBER_GAS_VOLTAGE_MIN] != 0){
		if(myCh->misc.gasVoltage < myData->mData.config.swFaultConfig[CHAMBER_GAS_VOLTAGE_MIN]) {
			if(myCh->misc.gas_check_flag == 0){
				myCh->misc.gas_check_time = myCh->op.runTime;	
				myCh->misc.gas_check_flag = 1;
			}else if(myCh->misc.gas_check_flag == 1){
				if(labs(myCh->misc.gas_check_time - myCh->op.runTime) >
					myData->mData.config.swFaultConfig[GAS_CHECK_TIME] * 100){
					myCh->misc.tmpState = myCh->op.state;
					myCh->misc.tmpCode = myCh->op.code;
					myCh->op.code = C_FAULT_GAS_VOLTAGE_MIN;
					rtn = FAULT_COND;
				}
			}
		}else if(myCh->misc.gasVoltage > myData->mData.config.swFaultConfig[CHAMBER_GAS_VOLTAGE_MIN]) {
			myCh->misc.gas_check_flag = 0;
			myCh->misc.gas_check_time = myCh->op.runTime;	
		}
	}
	if(rtn != FAULT_COND && myData->mData.config.swFaultConfig[CHAMBER_GAS_VOLTAGE_MAX] != 0){
		if(myCh->misc.gasVoltage > myData->mData.config.swFaultConfig[CHAMBER_GAS_VOLTAGE_MAX]) { 
			if(myCh->misc.gas_check_flag == 0){
				myCh->misc.gas_check_time = myCh->op.runTime;	
				myCh->misc.gas_check_flag = 1;
			}else if(myCh->misc.gas_check_flag == 1){
				if(labs(myCh->misc.gas_check_time - myCh->op.runTime) >
					myData->mData.config.swFaultConfig[GAS_CHECK_TIME] * 100){
					myCh->misc.tmpState = myCh->op.state;
					myCh->misc.tmpCode = myCh->op.code;
					myCh->op.code = C_FAULT_GAS_VOLTAGE_MAX;
					rtn = FAULT_COND;
				}
			}
		}else if(myCh->misc.gasVoltage < myData->mData.config.swFaultConfig[CHAMBER_GAS_VOLTAGE_MAX]) {
			myCh->misc.gas_check_flag = 0;
			myCh->misc.gas_check_time = myCh->op.runTime;	
		}
	}
	if(rtn != FAULT_COND && myData->mData.config.swFaultConfig[SOFT_VENTING_COUNT] != 0){
		if(myCh->misc.gasVoltage - myCh->misc.std_gasVoltage >= myData->mData.config.swFaultConfig[SOFT_VENTING_VALUE]){
			myCh->misc.soft_venting_count++;
			myCh->misc.std_gasVoltage = myCh->misc.gasVoltage;
		}else{
			myCh->misc.soft_venting_count = 0;
		}
		if(myCh->misc.soft_venting_count >= myData->mData.config.swFaultConfig[SOFT_VENTING_COUNT]){
			myCh->misc.tmpState = myCh->op.state;
			myCh->misc.tmpCode = myCh->op.code;
			myCh->op.code = C_FAULT_SOFT_VENTING;
			rtn = FAULT_COND;
		}
	}
	if(rtn != FAULT_COND && myData->mData.config.swFaultConfig[HARD_VENTING_VALUE] != 0){
		if(myCh->misc.gasVoltage - myCh->misc.std_gasVoltage >= myData->mData.config.swFaultConfig[HARD_VENTING_VALUE]){
			myCh->misc.tmpState = myCh->op.state;
			myCh->misc.tmpCode = myCh->op.code;
			myCh->op.code = C_FAULT_HARD_VENTING;
			rtn = FAULT_COND;
		}
	}

	if(rtn != FAULT_COND && myData->mData.config.swFaultConfig[AMBIENT_TEMP_MAX] != 0){
		if(myCh->misc.ambientTemp > myData->mData.config.swFaultConfig[AMBIENT_TEMP_MAX]) { 
			if(myCh->misc.ambient_check_flag == 0){
				myCh->misc.ambient_check_time = myCh->op.runTime;	
				myCh->misc.ambient_check_flag = 1;
			}else if(myCh->misc.ambient_check_flag == 1){
				if(labs(myCh->misc.ambient_check_time - myCh->op.runTime) >
					myData->mData.config.swFaultConfig[AMBIENT_TEMP_MAX_TIME] * 100){
					myCh->misc.tmpState = myCh->op.state;
					myCh->misc.tmpCode = myCh->op.code;
					myCh->op.code = C_FAULT_AMBIENT_TEMP_MAX;
					rtn = FAULT_COND;
				}
			}
		}else if(myCh->misc.ambientTemp < myData->mData.config.swFaultConfig[AMBIENT_TEMP_MAX]) {
			myCh->misc.ambient_check_flag = 0;
			myCh->misc.ambient_check_time = myCh->op.runTime;	
		}
	}

	//210303 add
	if(myData->mData.config.TempFaultDioUse != 0){
		fault_Count = count = 0;
		for(i=0; i < myCh->misc.chAuxTCnt; i++){
			if((myCh->misc.chAuxTemp[i]
				< TEMP_CONNECT_ERROR_VALUE)){
				if((myCh->misc.chAuxTemp[i]
					>= myData->mData.config.TempFaultMinT) &&
					myCh->misc.chAuxTemp[i] 
					<= myData->mData.config.TempFaultMaxT){
							count++;
				}
			}
		}
		if(count > 1) myCh->misc.tempFaultCount = P1;
		if(myCh->misc.tempFaultCount == P1){
			myCh->misc.tempFaultCount = P0;
			diff = myCh->op.runTime - myCh->misc.hw_fault_temp;
			if(diff >= myData->mData.config.TempFaultCheckTime){
				myCh->misc.hw_fault_temp = myCh->op.runTime;
				index = (myPs->config.chPerBd * bd + ch);
				myCh->misc.tempFaultChamberNo = myData->ChamArray[index].number1;
				for(i=0; i < MAX_CH_PER_MODULE; i++) {
					if(myData->ChamArray[i].number1 == 0)	continue;
					if(myCh->misc.tempFaultChamberNo == myData->ChamArray[i].number1){
						tempBD = myData->ChamArray[i].bd;
						tempCH = myData->ChamArray[i].ch;
						if((myPs->config.chPerBd * tempBD + tempCH)
							> (myPs->config.installedCh-1)) continue;
						if(myData->bData[tempBD].cData[tempCH].op.state == C_RUN){
							myData->bData[tempBD].cData[tempCH].signal[C_SIG_PAUSE] = P1;
#if CYCLER_TYPE == DIGITAL_CYC
						switch(myCh->misc.tempFaultChamberNo){
							case 1:
								myPs->signal[M_SIG_TEMP_FAULT1] = P1;
								rtn = FAULT_COND;
								break;
							case 2:
								myPs->signal[M_SIG_TEMP_FAULT2] = P1;
								rtn = FAULT_COND;
								break;
							default:
								break;
						}
#endif	
						}
					}
				}	
			}
		}else{
			myCh->misc.hw_fault_temp = myCh->op.runTime;
			switch(myCh->misc.tempFaultChamberNo){
				case 1:
					myPs->signal[M_SIG_TEMP_FAULT1] = P0;
					break;
				case 2:
					myPs->signal[M_SIG_TEMP_FAULT2] = P0;
					break;
				default:
					break;
			}
		}
	}
	//LJS 210417 for MASTER RECIPE
	if(myData->bData[bd].cData[ch].misc.Fault_flag != 0){
		myData->bData[bd].cData[ch].misc.Fault_flag = 0;
		switch(myCh->misc.ch_fault_code){
			case C_FAULT_MASTER_RECIPE_VOLTAGE_ERR://SDI
			case C_FAULT_MASTER_RECIPE_CURRENT_ERR://SDI
			case C_FAULT_MASTER_RECIPE_TIME_ERR://SDI
			case C_FAULT_MASTER_RECIPE_TEMP_ERR://SDI
			case C_FAULT_MASTER_RECIPE_GROUP_ERR://SDI
			case C_FAULT_GUI_AMBIENT_TEMP_ERROR://LGES
			case C_FAULT_GUI_UPPER_VOLTAGE://LGES
			case C_FAULT_GUI_UPPER_TEMP://LGES
			case C_FAULT_GUI_LOWER_VOLTAGE://LGES
			case C_FAULT_GUI_CHAMBER_DI://LGES
				myCh->misc.tmpState = myCh->op.state;
				myCh->misc.tmpCode = myCh->op.code;
				myCh->op.code = myCh->misc.ch_fault_code;
				rtn = FAULT_COND;
				break;
			default:	//SDI
				myCh->misc.tmpState = myCh->op.state;
				myCh->misc.tmpCode = myCh->op.code;
				myCh->op.code = myCh->misc.ch_fault_code;
				rtn = FAULT_COND;
				break;
		}	
	}
	
//220214 LJS For LGES semi-auto jig PLC safety Condition
#ifdef _GROUP_ERROR
	if(myCh->misc.Fault_Check_flag == 0){
		if(myCh->misc.groupAvgVsens != 0 && myCh->misc.stepSyncFlag != 0){
			if(myTestCond->step[advStepNo].group_DeltaVoltage != 0){
				if(labs(myCh->misc.groupAvgVsens - myCh->op.Vsens) >=
						myTestCond->step[advStepNo].group_DeltaVoltage){
					myCh->misc.Std_Time = myCh->op.runTime;
					myCh->misc.Fault_Check_flag = 1;
				}
			}
		}
	}
	if(myCh->misc.Fault_Check_flag == 1){
		rtn = Group_Error_Fault_Check(bd, ch, advStepNo);
	}
	
	if(myTestCond->step[advStepNo].group_EndFaultTime != 0 &&
			myCh->misc.endState == P2){
		if(myCh->op.runTime >= myCh->misc.groupEndTime + 
			 myTestCond->step[advStepNo].group_EndFaultTime){
			myCh->misc.tmpState = myCh->op.state;
			myCh->misc.tmpCode = myCh->op.code;
			myCh->op.code = C_FAULT_GROUP_TIME_ERROR;
			rtn = FAULT_COND;
		}	
	}
#endif
	
	if(rtn == FAULT_COND) { //fault
		myCh->op.select = SAVE_FLAG_SAVING_ETC;
		send_save_msg(bd, ch, saveDt, 0);
		
		//170106 oys add
		if(myCh->misc.tempWaitType == P0){
			myCh->misc.chGroupNo = 0;
			//20180417 add
			myCh->misc.stepSyncFlag = P0;
			myCh->misc.endState = P0;		//220203_hun
			myCh->misc.groupAvgVsens = 0;	//220203_hun
			myCh->misc.group_StartVoltage_flag = 0;
		}
		//190318 lyhw
		if(myData->mData.config.parallelMode == P2) { 
			cCycle_p_ch_check(bd, ch);
		}
		myCh->op.phase = P100;
		cNextStepCheck(bd, ch);
	}
}

void cFaultCondSoft_P(int bd, int ch)
{
	unsigned char type;
    int rtn=0, rangeV, rangeI, mode;
	long capacity, refV, refI, maxV, maxI;
	long cvtVal, faultUpperI_com, faultLowerI_com, faultUpperI_step, faultLowerI_step;
	unsigned long advStepNo, saveDt;
	S_CH_STEP_INFO step;

	if(myData->mData.config.function[F_SW_FAULT_COND] == P0) return;

	myCh = &(myData->bData[bd].cData[ch]);
	myTestCond = &(myPs->testCond[bd][ch]);

	if(myCh->op.state != C_RUN) return;
	if(myCh->misc.patternPhase != 0) return;
	if(myCh->op.phase != P50) return;

	step = step_info(bd, ch);

	advStepNo = step.advStepNo;
	saveDt = step.saveDt;
	type = step.type;
	
	refV = step.refV;
	refI = step.refI;
	rangeV = step.rangeV;
	rangeI = step.rangeI;
	mode = step.mode;

	maxV = myPs->config.maxVoltage[rangeV];
	maxI = myPs->config.maxCurrent[rangeI];

	//190801 oys add start : Convert C-rate to I value
	faultUpperI_com = myPs->testCond[bd][ch].safety.faultUpperI;
	cvtVal = Convert_C_rate_to_I_value(bd, ch, faultUpperI_com);
	if(cvtVal != 0)
		faultUpperI_com = cvtVal;

	faultLowerI_com = myPs->testCond[bd][ch].safety.faultLowerI;
	cvtVal = Convert_C_rate_to_I_value(bd, ch, faultLowerI_com);
	if(cvtVal != 0)
		faultLowerI_com = cvtVal;
	
	faultUpperI_step = myPs->testCond[bd][ch].step[advStepNo].faultUpperI;
	cvtVal = Convert_C_rate_to_I_value(bd, ch, faultUpperI_step);
	if(cvtVal != 0)
		faultUpperI_step = cvtVal;
	
	faultLowerI_step = myPs->testCond[bd][ch].step[advStepNo].faultLowerI;
	cvtVal = Convert_C_rate_to_I_value(bd, ch, faultLowerI_step);
	if(cvtVal != 0)
		faultLowerI_step = cvtVal;
	//add end

	if(myPs->config.capacityType == CAPACITY_AMPARE_HOURS) {
		capacity = myCh->op.ampareHour
				+ myData->bData[bd].cData[ch-1].op.ampareHour;
	} else if(myPs->config.capacityType == CAPACITY_CAPACITANCE) {
		capacity = myCh->op.capacitance
				+ myData->bData[bd].cData[ch-1].op.capacitance;
	} else {
		capacity = 0;
	}
				
	switch(type) {
		case STEP_OCV:
			if(myPs->testCond[bd][ch].safety.faultUpperV != 0) {
				if(myCh->op.Vsens
					>= myPs->testCond[bd][ch].safety.faultUpperV) {
					myCh->misc.errCnt[C_CNT_UPPER_OCV]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_OCV]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_UPPER_OCV] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_OCV;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); 
						//safety
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_UPPER_OCV] = 0;
				}
			}
			if(myPs->testCond[bd][ch].safety.faultLowerV != 0) {
			  	if(myCh->op.Vsens
					<= myPs->testCond[bd][ch].safety.faultLowerV) {
					myCh->misc.errCnt[C_CNT_LOWER_OCV]++;
					if(myCh->misc.errCnt[C_CNT_LOWER_OCV]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_LOWER_OCV] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_LOWER_OCV;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); 
						//safety
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_LOWER_OCV] = 0;
				}
			}

			if(myPs->testCond[bd][ch].safety.faultUpperTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 0);
				if(rtn == FAULT_COND)
					break;
			}
			if(myPs->testCond[bd][ch].safety.faultLowerTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 1);
				if(rtn == FAULT_COND)
					break;
			}
			if(myPs->testCond[bd][ch].step[advStepNo].faultUpperV != 0) {
				if(myCh->op.Vsens 
					>= myPs->testCond[bd][ch].step[advStepNo].faultUpperV) {
					myCh->misc.errCnt[C_CNT_UPPER_OCV_STEP]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_OCV_STEP]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_UPPER_OCV_STEP] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_OCV;
						myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); 
						//step_safety
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_UPPER_OCV_STEP] = 0;
				}
			}
			if(myPs->testCond[bd][ch].step[advStepNo].faultLowerV != 0) {
			   	if(myCh->op.Vsens
					<= myPs->testCond[bd][ch].step[advStepNo].faultLowerV) {
					myCh->misc.errCnt[C_CNT_LOWER_OCV_STEP]++;
					if(myCh->misc.errCnt[C_CNT_LOWER_OCV_STEP]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_LOWER_OCV_STEP] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_LOWER_OCV;
						myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); 
						//step_safety
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_LOWER_OCV_STEP] = 0;
				}
			}
			if(myPs->testCond[bd][ch].step[advStepNo].faultUpperTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 2);
				if(rtn == FAULT_COND)
					break;
			}
			if(myPs->testCond[bd][ch].step[advStepNo].faultLowerTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 3);
				if(rtn == FAULT_COND)
					break;
			}
			//20180314 sch modify
			if(myPs->config.LimitVI.limit_use_I != 0 
				|| myPs->config.LimitVI.limit_use_V != 0){
				rtn = Fault_Check_default_LG(bd, ch);
				if(rtn == FAULT_COND)
					break;
			}
			//19090l lyhw
			if(myPs->config.function[F_CHANGE_VI_CHECK] != 0){
				rtn = Fault_Check_Change_VI(bd, ch, advStepNo);
				if(rtn == FAULT_COND)	break;
			}
			/*
			if(myPs->config.LimitVI.limit_use_V != 0){
				rtn = Fault_Check_default_LG(bd, ch);
				if(rtn == FAULT_COND)
					break;
			}*/
			break;
		case STEP_REST:
			if(myPs->testCond[bd][ch].safety.faultUpperV != 0) {
				if(myCh->op.Vsens
					>= myPs->testCond[bd][ch].safety.faultUpperV) {
					myCh->misc.errCnt[C_CNT_UPPER_OCV]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_OCV]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_UPPER_OCV] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_OCV;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); 
						//safety
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_UPPER_OCV] = 0;
				}
			}
			if(myPs->testCond[bd][ch].safety.faultLowerV != 0) {
			   	if(myCh->op.Vsens
					<= myPs->testCond[bd][ch].safety.faultLowerV) {
					myCh->misc.errCnt[C_CNT_LOWER_OCV]++;
					if(myCh->misc.errCnt[C_CNT_LOWER_OCV]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_LOWER_OCV] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_LOWER_OCV;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); 
						//safety
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_LOWER_OCV] = 0;
				}
			}
			if(myPs->testCond[bd][ch].safety.faultUpperTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 0);
				if(rtn == FAULT_COND)
					break;
			}
			if(myPs->testCond[bd][ch].safety.faultLowerTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 1);
				if(rtn == FAULT_COND)
					break;
			}
			if(myPs->testCond[bd][ch].step[advStepNo].faultUpperV != 0) {
				if(myCh->op.Vsens 
					>= myPs->testCond[bd][ch].step[advStepNo].faultUpperV) {
					myCh->misc.errCnt[C_CNT_UPPER_OCV_STEP]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_OCV_STEP]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_UPPER_OCV_STEP] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_OCV;
						myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); 
						//step_safety
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_UPPER_OCV_STEP] = 0;
				}
			}
			if(myPs->testCond[bd][ch].step[advStepNo].faultLowerV != 0) {
			   	if(myCh->op.Vsens
					<= myPs->testCond[bd][ch].step[advStepNo].faultLowerV) {
					myCh->misc.errCnt[C_CNT_LOWER_OCV_STEP]++;
					if(myCh->misc.errCnt[C_CNT_LOWER_OCV_STEP]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_LOWER_OCV_STEP] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_LOWER_OCV;
						myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); 
						//step_safety
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_LOWER_OCV_STEP] = 0;
				}
			}
			if(myPs->testCond[bd][ch].step[advStepNo].faultUpperTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 2);
				if(rtn == FAULT_COND)
					break;
			}
			if(myPs->testCond[bd][ch].step[advStepNo].faultLowerTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 3);
				if(rtn == FAULT_COND)
					break;
			}
			//190311 lyh add
			if(myTestCond->step[advStepNo].pauseUpperTemp != 0) {
				rtn = tempPauseCheck(bd, ch, advStepNo, 0);
				if(rtn == FAULT_COND)
					break;
			}
			if(myTestCond->step[advStepNo].pauseLowerTemp != 0) {
				rtn = tempPauseCheck(bd, ch, advStepNo, 1);
				if(rtn == FAULT_COND)
					break;
			}
			//20180314 sch modify
			if(myPs->config.LimitVI.limit_use_I != 0 
				|| myPs->config.LimitVI.limit_use_V != 0){
				rtn = Fault_Check_default_LG(bd, ch);
				if(rtn == FAULT_COND)
					break;
			}
			//19090l lyhw
			if(myPs->config.function[F_CHANGE_VI_CHECK] != 0){
				rtn = Fault_Check_Change_VI(bd, ch, advStepNo);
				if(rtn == FAULT_COND)	break;
			}
			/*
			if(myPs->config.LimitVI.limit_use_V != 0){
				rtn = Fault_Check_default_LG(bd, ch);
				if(rtn == FAULT_COND)
					break;
			}*/
			if(myData->mData.config.swFaultConfig[REST_CHECK_START_TIME] != 0){
				rtn = Fault_Check_Rest_Voltage(bd, ch);
				if(rtn == FAULT_COND)	break;
			}
			break;
    	case STEP_CHARGE:
			//100908 pjy temp fault check without delay
			if(myPs->testCond[bd][ch].safety.faultUpperTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 0);
				if(rtn == FAULT_COND)
					break;
			}
			if(myPs->testCond[bd][ch].safety.faultLowerTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 1);
				if(rtn == FAULT_COND)
					break;
			}
			if(myPs->testCond[bd][ch].step[advStepNo].faultUpperTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 2);
				if(rtn == FAULT_COND)
					break;
			}
			if(myPs->testCond[bd][ch].step[advStepNo].faultLowerTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 3);
				if(rtn == FAULT_COND)
					break;
			}
			//190311 lyh add
			if(myTestCond->step[advStepNo].pauseUpperTemp != 0) {
				rtn = tempPauseCheck(bd, ch, advStepNo, 0);
				if(rtn == FAULT_COND)
					break;
			}
			if(myTestCond->step[advStepNo].pauseLowerTemp != 0) {
				rtn = tempPauseCheck(bd, ch, advStepNo, 1);
				if(rtn == FAULT_COND)
					break;
			}

			//hun_211228_s 병렬 부분 누락으로 인해 추가
			#ifdef _SDI_SAFETY_V1	
			if(myTestCond->step[advStepNo].faultRunTime != 0) {
				if(myCh->op.runTime >= myTestCond->step[advStepNo].faultRunTime){
					myCh->misc.tmpState = myCh->op.state;
					myCh->misc.tmpCode = myCh->op.code;
					myCh->op.code = C_FAULT_RUN_TIME;
					Fault_Value_Check(myCh->misc.advStepNo, 
					myCh->op.runTime,
					myPs->testCond[bd][ch].step[advStepNo].faultRunTime,
					0, 0, 0, 0, 0, 0, 0);
					rtn = FAULT_COND;
					break;
				}
			}
			#endif
			#if CHANGE_VI_CHECK == 1
			if(myTestCond->step[advStepNo].faultRunTime != 0) {
				if(myCh->op.runTime >= myTestCond->step[advStepNo].faultRunTime){
					myCh->misc.tmpState = myCh->op.state;
					myCh->misc.tmpCode = myCh->op.code;
					myCh->op.code = C_FAULT_RUN_TIME;
					rtn = FAULT_COND;
					break;
				}
			}
			#endif
			#ifdef _ULSAN_SDI_SAFETY
			if(myTestCond->step[advStepNo].humpSet_T != 0 
				&& myTestCond->step[advStepNo].humpSet_I != 0){
				if(myCh->misc.cvFlag == P1){
					if(myCh->misc.cvTime_Ulsan <= 100){
						myCh->misc.humpComp_T = myCh->op.runTime;
						myCh->misc.humpComp_I = myCh->op.Isens + myData->bData[bd].cData[ch-1].op.Isens;

						myCh->misc.humpCheck_T = myCh->misc.humpComp_T 
							+ myTestCond->step[advStepNo].humpSet_T;
					}
					if(myCh->misc.cvTime_Ulsan > 100){
						if(myCh->op.runTime >= myCh->misc.humpCheck_T){
							myCh->misc.humpCheck_I
								= (myCh->op.Isens + myData->bData[bd].cData[ch-1].op.Isens) 
									- myCh->misc.humpComp_I;
							myCh->misc.humpComp_T = myCh->op.runTime;
							myCh->misc.humpComp_I = myCh->op.Isens + myData->bData[bd].cData[ch-1].op.Isens;

							myCh->misc.humpCheck_T = myCh->misc.humpComp_T 
								+ myTestCond->step[advStepNo].humpSet_T;

							if(myCh->misc.humpCheck_I 
								>= myTestCond->step[advStepNo].humpSet_I){
								myCh->misc.tmpState = myCh->op.state;
								myCh->misc.tmpCode = myCh->op.code;
								myCh->misc.cvFlag = 0;
								myCh->misc.cvTime_Ulsan = 0;
								myCh->op.code = C_FAULT_CV_CURRENT_HUMP;
								rtn = FAULT_COND;
								break;
							}
						}
					}
				}
			}
			#endif
			//hun_211228_e 병렬 부분 누락으로 인해 추가

			if(myPs->testCond[bd][ch].safety.faultUpperV != 0) {
				if(myCh->op.Vsens
					>= myPs->testCond[bd][ch].safety.faultUpperV) {
					myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_VOLTAGE;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); //safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Vsens,
						myPs->testCond[bd][ch].safety.faultUpperV,
					   	0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE] = 0;
				}
			}
			if(myPs->testCond[bd][ch].safety.faultLowerV != 0) {
				if(myCh->op.Vsens
					<= myPs->testCond[bd][ch].safety.faultLowerV) {
					myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE]++;
					if(myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_LOWER_VOLTAGE;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); //safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Vsens,
						myPs->testCond[bd][ch].safety.faultLowerV,
					   	0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE] = 0;
				}
			}
			if(myCh->op.checkDelayTime < 180) break;
			if(faultUpperI_com != 0) {
				//slave ch + master ch
				if(myCh->op.Isens + myData->bData[bd].cData[ch-1].op.Isens 
					>= faultUpperI_com) {
					myCh->misc.errCnt[C_CNT_UPPER_CURRENT]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_CURRENT]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_UPPER_CURRENT] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_CURRENT;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); //safety
						Fault_Value_Check(myCh->misc.advStepNo, 
						myCh->op.Isens + myData->bData[bd].cData[ch-1].op.Isens,
						faultUpperI_com, 0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_UPPER_CURRENT] = 0;
				}
			}
			if(faultLowerI_com != 0) {
				//slave ch + master ch
				if(myCh->op.Isens + myData->bData[bd].cData[ch-1].op.Isens 
					<= faultLowerI_com) {
					myCh->misc.errCnt[C_CNT_LOWER_CURRENT]++;
					if(myCh->misc.errCnt[C_CNT_LOWER_CURRENT]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_LOWER_CURRENT] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_LOWER_CURRENT;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); //safety
						Fault_Value_Check(myCh->misc.advStepNo,
						myCh->op.Isens + myData->bData[bd].cData[ch-1].op.Isens,
						faultLowerI_com * (-1), 0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_LOWER_CURRENT] = 0;
				}
			}
			if(myPs->testCond[bd][ch].safety.faultUpperC != 0) {
				if(capacity >= myPs->testCond[bd][ch].safety.faultUpperC) {
					myCh->misc.errCnt[C_CNT_UPPER_CAPACITY]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_CAPACITY]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_UPPER_CAPACITY] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_CAPACITY;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); //safety
						Fault_Value_Check(myCh->misc.advStepNo, capacity,
						myPs->testCond[bd][ch].safety.faultUpperC,
					   	0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_UPPER_CAPACITY] = 0;
				}
			}
			if(myPs->testCond[bd][ch].step[advStepNo].faultUpperV != 0) {
				if(myCh->op.Vsens 
					>= myPs->testCond[bd][ch].step[advStepNo].faultUpperV) {
					myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE_STEP]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE_STEP]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE_STEP] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_VOLTAGE;
						myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); //step_safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Vsens,
						myPs->testCond[bd][ch].step[advStepNo].faultUpperV,
					   	0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE_STEP] = 0;
				}
			}
			if(myPs->testCond[bd][ch].step[advStepNo].faultLowerV != 0) {
				if(myCh->op.Vsens 
					<= myPs->testCond[bd][ch].step[advStepNo].faultLowerV) {
					myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE_STEP]++;
					if(myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE_STEP]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE_STEP] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_LOWER_VOLTAGE;
						myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); //step_safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Vsens,
						myPs->testCond[bd][ch].step[advStepNo].faultLowerV,
					   	0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE_STEP] = 0;
				}
			}
			if(faultUpperI_step != 0) {
				//slave ch + master ch
				if(myCh->op.Isens + myData->bData[bd].cData[ch-1].op.Isens 
					>= faultUpperI_step) {
					myCh->misc.errCnt[C_CNT_UPPER_CURRENT_STEP]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_CURRENT_STEP]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_UPPER_CURRENT_STEP] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_CURRENT;
						myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); //step_safety
						Fault_Value_Check(myCh->misc.advStepNo, 
						myCh->op.Isens + myData->bData[bd].cData[ch-1].op.Isens,
						faultUpperI_step, 0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_UPPER_CURRENT_STEP] = 0;
				}
			}
			if((refV - myData->bData[bd].cData[ch-1].op.Vsens) > (maxV * 0.001)){		//kjc_200612
				if(faultLowerI_step != 0) {
					//slave ch + master ch
					if(myCh->op.Isens + myData->bData[bd].cData[ch-1].op.Isens 
						<= faultLowerI_step) {
						myCh->misc.errCnt[C_CNT_LOWER_CURRENT_STEP]++;
						if(myCh->misc.errCnt[C_CNT_LOWER_CURRENT_STEP]>= MAX_ERROR_CNT){
							myCh->misc.errCnt[C_CNT_LOWER_CURRENT_STEP] = 0;
							myCh->misc.tmpState = myCh->op.state;
							myCh->misc.tmpCode = myCh->op.code;
							myCh->op.code = C_FAULT_LOWER_CURRENT;
							myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); //step_safety
							Fault_Value_Check(myCh->misc.advStepNo,
							myCh->op.Isens + myData->bData[bd].cData[ch-1].op.
							Isens,faultLowerI_step, 0, 0, 0, 0, 0, 0, 0);
							rtn = FAULT_COND;
							break;
						}
					}else{
						myCh->misc.errCnt[C_CNT_LOWER_CURRENT_STEP] = 0;
					}
				}
			}
			if(myPs->testCond[bd][ch].step[advStepNo].faultUpperC != 0) {
				if(capacity >=
					myPs->testCond[bd][ch].step[advStepNo].faultUpperC) {
					myCh->misc.errCnt[C_CNT_UPPER_CAPACITY_STEP]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_CAPACITY_STEP]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_UPPER_CAPACITY_STEP] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_CAPACITY;
						myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); //step_safety
						Fault_Value_Check(myCh->misc.advStepNo, capacity,
						myPs->testCond[bd][ch].step[advStepNo].faultUpperC,
					   	0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_UPPER_CAPACITY_STEP] = 0;
				}
			}
			//20180314 sch modify
			if(myPs->config.LimitVI.limit_use_I != 0 
				|| myPs->config.LimitVI.limit_use_V != 0){
				rtn = Fault_Check_default_LG(bd, ch);
				if(rtn == FAULT_COND)
					break;
			}
			//19090l lyhw
			if(myPs->config.function[F_CHANGE_VI_CHECK] != 0){
				rtn = Fault_Check_Change_VI(bd, ch, advStepNo);
				if(rtn == FAULT_COND)	break;
			}
#ifdef _GROUP_ERROR
			if(myCh->op.Vsens >= myPs->testCond[bd][ch].step[advStepNo].group_StartVoltage){
				myCh->misc.group_StartVoltage_flag = P1;
			}
#endif
			break;
		case STEP_DISCHARGE:
			//100908 pjy temp fault check without delay
			if(myPs->testCond[bd][ch].safety.faultUpperTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 0);
				if(rtn == FAULT_COND)
					break;
			}
			if(myPs->testCond[bd][ch].safety.faultLowerTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 1);
				if(rtn == FAULT_COND)
					break;
			}
			if(myPs->testCond[bd][ch].step[advStepNo].faultUpperTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 2);
				if(rtn == FAULT_COND)
					break;
			}
			if(myPs->testCond[bd][ch].step[advStepNo].faultLowerTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 3);
				if(rtn == FAULT_COND)
					break;
			}
			//190311 lyh add
			if(myTestCond->step[advStepNo].pauseUpperTemp != 0) {
				rtn = tempPauseCheck(bd, ch, advStepNo, 0);
				if(rtn == FAULT_COND)
					break;
			}
			if(myTestCond->step[advStepNo].pauseLowerTemp != 0) {
				rtn = tempPauseCheck(bd, ch, advStepNo, 1);
				if(rtn == FAULT_COND)
					break;
			}
			
			//hun_211228_s 병렬 부분 누락으로 인해 추가
			#ifdef _SDI_SAFETY_V1	
			if(myTestCond->step[advStepNo].faultRunTime != 0) {
				if(myCh->op.runTime >= myTestCond->step[advStepNo].faultRunTime){
					myCh->misc.tmpState = myCh->op.state;
					myCh->misc.tmpCode = myCh->op.code;
					myCh->op.code = C_FAULT_RUN_TIME;
					Fault_Value_Check(myCh->misc.advStepNo, 
					myCh->op.runTime,
					myPs->testCond[bd][ch].step[advStepNo].faultRunTime,
					0, 0, 0, 0, 0, 0, 0);
					rtn = FAULT_COND;
					break;
				}
			}
			#endif
			#ifdef _SDI_SAFETY_V2	
			//210417 LJS for MASTER RECIPE
			if(myCh->misc.MasterFlag == 1 || myCh->misc.MasterFlag == 3){
				if(myPs->testCond[bd][ch].safety.Master_Recipe_V != 0 &&
							myCh->misc.deltaV_cnt == 0){
					if(myCh->misc.Master_recipe_deltaV - myCh->op.Vsens >= 
						myPs->testCond[bd][ch].safety.Master_Recipe_V){
						myCh->misc.deltaV_cnt = 1;
					}
				}
				if(myCh->misc.deltaV_cnt == 0 && myCh->op.runTime >= 100){
					myCh->misc.tmpState = myCh->op.state;
					myCh->misc.tmpCode = myCh->op.code;
					myCh->op.code = C_FAULT_CABLE_LINE_CHECK;
					rtn = FAULT_COND;
					break;
				}
			}
			
			#endif
			#if CHANGE_VI_CHECK == 1
			if(myTestCond->step[advStepNo].faultRunTime != 0) {
				if(myCh->op.runTime >= myTestCond->step[advStepNo].faultRunTime){
					myCh->misc.tmpState = myCh->op.state;
					myCh->misc.tmpCode = myCh->op.code;
					myCh->op.code = C_FAULT_RUN_TIME;
					rtn = FAULT_COND;
					break;
				}
			}
			#endif
			#ifdef _ULSAN_SDI_SAFETY
			if(myTestCond->step[advStepNo].humpSet_T != 0 
				&& myTestCond->step[advStepNo].humpSet_I != 0){
				if(myCh->misc.cvFlag == P1){
					if(myCh->misc.cvTime_Ulsan <= 100){
						myCh->misc.humpComp_T = myCh->op.runTime;
						myCh->misc.humpComp_I = myCh->op.Isens + myData->bData[bd].cData[ch-1].op.Isens;

						myCh->misc.humpCheck_T = myCh->misc.humpComp_T 
							+ myTestCond->step[advStepNo].humpSet_T;
					}
					if(myCh->misc.cvTime_Ulsan > 100){
						if(myCh->op.runTime >= myCh->misc.humpCheck_T){
							myCh->misc.humpCheck_I = myCh->misc.humpComp_I
							 - (myCh->op.Isens + myData->bData[bd].cData[ch-1].op.Isens);

							myCh->misc.humpComp_T = myCh->op.runTime;
							myCh->misc.humpComp_I = myCh->op.Isens + myData->bData[bd].cData[ch-1].op.Isens;

							myCh->misc.humpCheck_T = myCh->misc.humpComp_T + myTestCond->step[advStepNo].humpSet_T;

							if(myCh->misc.humpCheck_I >= myTestCond->step[advStepNo].humpSet_I){
								myCh->misc.tmpState = myCh->op.state;
								myCh->misc.tmpCode = myCh->op.code;
								myCh->misc.cvFlag = 0;
								myCh->misc.cvTime_Ulsan = 0;
								myCh->op.code = C_FAULT_CV_CURRENT_HUMP;
								rtn = FAULT_COND;
								break;
							}
						}
					}
				}
			}
			#endif
			//hun_211228_e 병렬 부분 누락으로 인해 추가
			
			if(myPs->testCond[bd][ch].safety.faultUpperV != 0) {
				if(myCh->op.Vsens
					>= myPs->testCond[bd][ch].safety.faultUpperV) {
					myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_VOLTAGE;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); //safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Vsens,
						myPs->testCond[bd][ch].safety.faultUpperV,
					   	0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE] = 0;
				}
			}
			if(myPs->testCond[bd][ch].safety.faultLowerV != 0) {
				if(myCh->op.Vsens
					<= myPs->testCond[bd][ch].safety.faultLowerV) {
					myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE]++;
					if(myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_LOWER_VOLTAGE;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); //safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Vsens,
						myPs->testCond[bd][ch].safety.faultLowerV,
					   	0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE] = 0;
				}
			}
			if(myCh->op.checkDelayTime < 180) break;
			if(faultUpperI_com != 0) {
				//slave ch + master ch
				if(myCh->op.Isens + myData->bData[bd].cData[ch-1].op.Isens 
					<= faultUpperI_com * (-1)) {
					myCh->misc.errCnt[C_CNT_UPPER_CURRENT]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_CURRENT]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_UPPER_CURRENT] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_CURRENT;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); //safety
						Fault_Value_Check(myCh->misc.advStepNo, 
						myCh->op.Isens + myData->bData[bd].cData[ch-1].op.Isens,
						faultUpperI_com * (-1), 0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_UPPER_CURRENT] = 0;
				}
			}
			if(faultLowerI_com != 0) {
				//slave ch + master ch
				if(myCh->op.Isens + myData->bData[bd].cData[ch-1].op.Isens 
					>= faultLowerI_com * (-1)) {
					myCh->misc.errCnt[C_CNT_LOWER_CURRENT]++;
					if(myCh->misc.errCnt[C_CNT_LOWER_CURRENT]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_LOWER_CURRENT] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_LOWER_CURRENT;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); //safety
						Fault_Value_Check(myCh->misc.advStepNo,
						myCh->op.Isens + myData->bData[bd].cData[ch-1].op.Isens,
						faultLowerI_com * (-1), 0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_LOWER_CURRENT] = 0;
				}
			}
			if(myPs->testCond[bd][ch].safety.faultUpperC != 0) {
				if(capacity >= myPs->testCond[bd][ch].safety.faultUpperC) {
					myCh->misc.errCnt[C_CNT_UPPER_CAPACITY]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_CAPACITY]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_UPPER_CAPACITY] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_CAPACITY;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); //safety
						Fault_Value_Check(myCh->misc.advStepNo, capacity,
						myPs->testCond[bd][ch].safety.faultUpperC,
					   	0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_UPPER_CAPACITY] = 0;
				}
			}
			if(myPs->testCond[bd][ch].step[advStepNo].faultUpperV != 0) {
				if(myCh->op.Vsens 
					>= myPs->testCond[bd][ch].step[advStepNo].faultUpperV) {
					myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE_STEP]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE_STEP]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE_STEP] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_VOLTAGE;
						myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); //step_safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Vsens,
						myPs->testCond[bd][ch].step[advStepNo].faultUpperV,
					   	0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE_STEP] = 0;
				}
			}
			if(myPs->testCond[bd][ch].step[advStepNo].faultLowerV != 0) {
				if(myCh->op.Vsens 
					<= myPs->testCond[bd][ch].step[advStepNo].faultLowerV) {
					myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE_STEP]++;
					if(myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE_STEP]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE_STEP] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_LOWER_VOLTAGE;
						myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); //step_safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Vsens,
						myPs->testCond[bd][ch].step[advStepNo].faultLowerV,
					   	0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE_STEP] = 0;
				}
			}
			if(faultUpperI_step != 0) {
				//slave ch + master ch
				if(myCh->op.Isens + myData->bData[bd].cData[ch-1].op.Isens 
					<= faultUpperI_step) {
					myCh->misc.errCnt[C_CNT_UPPER_CURRENT_STEP]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_CURRENT_STEP]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_UPPER_CURRENT_STEP] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_CURRENT;
						myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); //step_safety
						Fault_Value_Check(myCh->misc.advStepNo, 
						myCh->op.Isens + myData->bData[bd].cData[ch-1].op.Isens,
						faultUpperI_step, 0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_UPPER_CURRENT_STEP] = 0;
				}
			}
			if((myData->bData[bd].cData[ch-1].op.Vsens - refV) > (maxV * 0.001)){		//kjc_200612
				if(faultLowerI_step != 0) {
					//slave ch + master ch
					if(myCh->op.Isens + myData->bData[bd].cData[ch-1].op.Isens 
						>= faultLowerI_step) {
						myCh->misc.errCnt[C_CNT_LOWER_CURRENT_STEP]++;
						if(myCh->misc.errCnt[C_CNT_LOWER_CURRENT_STEP]>= MAX_ERROR_CNT){
							myCh->misc.errCnt[C_CNT_LOWER_CURRENT_STEP] = 0;
							myCh->misc.tmpState = myCh->op.state;
							myCh->misc.tmpCode = myCh->op.code;
							myCh->op.code = C_FAULT_LOWER_CURRENT;
							myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); //step_safety
							Fault_Value_Check(myCh->misc.advStepNo,
							myCh->op.Isens + myData->bData[bd].cData[ch-1].op.
							Isens, faultLowerI_step, 0, 0, 0, 0, 0, 0, 0);
							rtn = FAULT_COND;
							break;
						}
					}else{
						myCh->misc.errCnt[C_CNT_LOWER_CURRENT_STEP] = 0;
					}
				}
			}
			if(myPs->testCond[bd][ch].step[advStepNo].faultUpperC != 0) {
				if(capacity >=
					myPs->testCond[bd][ch].step[advStepNo].faultUpperC) {
					myCh->misc.errCnt[C_CNT_UPPER_CAPACITY_STEP]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_CAPACITY_STEP]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_UPPER_CAPACITY_STEP] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_CAPACITY;
						myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); //step_safety
						Fault_Value_Check(myCh->misc.advStepNo, capacity,
						myPs->testCond[bd][ch].step[advStepNo].faultUpperC,
					   	0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_UPPER_CAPACITY_STEP] = 0;
				}
			}
			//19090l lyhw
			if(myPs->config.function[F_CHANGE_VI_CHECK] != 0){
				rtn = Fault_Check_Change_VI(bd, ch, advStepNo);
				if(rtn == FAULT_COND)	break;
			}
			/*
			//20180314 sch modify
			if(myPs->config.LimitVI.limit_use_I != 0){
				rtn = Fault_Check_default_LG(bd, ch);
				if(rtn == FAULT_COND)
					break;
			}
			if(myPs->config.LimitVI.limit_use_V != 0){
				rtn = Fault_Check_default_LG(bd, ch);
				if(rtn == FAULT_COND)
					break;
			}
*/
#ifdef _GROUP_ERROR
			if(myCh->op.Vsens <= myPs->testCond[bd][ch].step[advStepNo].group_StartVoltage){
				myCh->misc.group_StartVoltage_flag = P1;
			}
#endif
			break;
		case STEP_Z:
			//100908 pjy temp fault check without delay
			if(myPs->testCond[bd][ch].safety.faultUpperTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 0);
				if(rtn == FAULT_COND)
					break;
			}
			if(myPs->testCond[bd][ch].safety.faultLowerTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 1);
				if(rtn == FAULT_COND)
					break;
			}
			if(myPs->testCond[bd][ch].step[advStepNo].faultUpperTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 2);
				if(rtn == FAULT_COND)
					break;
			}
			if(myPs->testCond[bd][ch].step[advStepNo].faultLowerTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 3);
				if(rtn == FAULT_COND)
					break;
			}
			//190311 lyh add
			if(myTestCond->step[advStepNo].pauseUpperTemp != 0) {
				rtn = tempPauseCheck(bd, ch, advStepNo, 0);
				if(rtn == FAULT_COND)
					break;
			}
			if(myTestCond->step[advStepNo].pauseLowerTemp != 0) {
				rtn = tempPauseCheck(bd, ch, advStepNo, 1);
				if(rtn == FAULT_COND)
					break;
			}
			if(myPs->testCond[bd][ch].safety.faultUpperV != 0) {
				if(myCh->op.Vsens
					>= myPs->testCond[bd][ch].safety.faultUpperV) {
					myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_VOLTAGE;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); //safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Vsens,
						myPs->testCond[bd][ch].safety.faultUpperV,
					   	0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE] = 0;
				}
			}
			if(myPs->testCond[bd][ch].safety.faultLowerV != 0) {
				if(myCh->op.Vsens
					<= myPs->testCond[bd][ch].safety.faultLowerV) {
					myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE]++;
					if(myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_LOWER_VOLTAGE;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); //safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Vsens,
						myPs->testCond[bd][ch].safety.faultLowerV,
					   	0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE] = 0;
				}
			}
			if(myCh->op.checkDelayTime < 180) break;
			if(faultUpperI_com != 0) {
				//slave ch + master ch
				if(myCh->op.Isens + myData->bData[bd].cData[ch-1].op.Isens 
					<= faultUpperI_com * (-1)) {
					myCh->misc.errCnt[C_CNT_UPPER_CURRENT]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_CURRENT]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_UPPER_CURRENT] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_CURRENT;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); //safety
						Fault_Value_Check(myCh->misc.advStepNo, 
						myCh->op.Isens + myData->bData[bd].cData[ch-1].op.Isens,
						faultUpperI_com * (-1), 0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_UPPER_CURRENT] = 0;
				}
			}
			if(faultLowerI_com != 0) {
				//slave ch + master ch
				if(myCh->op.Isens + myData->bData[bd].cData[ch-1].op.Isens 
					>= faultLowerI_com * (-1)) {
					myCh->misc.errCnt[C_CNT_LOWER_CURRENT]++;
					if(myCh->misc.errCnt[C_CNT_LOWER_CURRENT]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_LOWER_CURRENT] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_LOWER_CURRENT;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); //safety
						Fault_Value_Check(myCh->misc.advStepNo,
						myCh->op.Isens + myData->bData[bd].cData[ch-1].op.Isens,
						faultLowerI_com * (-1), 0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_LOWER_CURRENT] = 0;
				}
			}
			if(myPs->testCond[bd][ch].step[advStepNo].faultUpperV != 0) {
				if(myCh->op.Vsens 
					>= myPs->testCond[bd][ch].step[advStepNo].faultUpperV) {
					myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE_STEP]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE_STEP]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE_STEP] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_VOLTAGE;
						myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); //step_safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Vsens,
						myPs->testCond[bd][ch].step[advStepNo].faultUpperV,
					   	0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE_STEP] = 0;
				}
			}
			if(myPs->testCond[bd][ch].step[advStepNo].faultLowerV != 0) {
				if(myCh->op.Vsens 
					<= myPs->testCond[bd][ch].step[advStepNo].faultLowerV) {
					myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE_STEP]++;
					if(myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE_STEP]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE_STEP] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_LOWER_VOLTAGE;
						myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); //step_safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Vsens,
						myPs->testCond[bd][ch].step[advStepNo].faultLowerV,
					   	0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE_STEP] = 0;
				}
			}
			if(faultUpperI_step != 0) {
				//slave ch + master ch
				if(myCh->op.Isens + myData->bData[bd].cData[ch-1].op.Isens 
					<= faultUpperI_step) {
					myCh->misc.errCnt[C_CNT_UPPER_CURRENT_STEP]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_CURRENT_STEP]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_UPPER_CURRENT_STEP] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_CURRENT;
						myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); //step_safety
						Fault_Value_Check(myCh->misc.advStepNo, 
						myCh->op.Isens + myData->bData[bd].cData[ch-1].op.Isens,
						faultUpperI_step, 0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_UPPER_CURRENT_STEP] = 0;
				}
			}
			if((myData->bData[bd].cData[ch-1].op.Vsens - refV) > (maxV * 0.001)){		//kjc_200612
				if(faultLowerI_step != 0) {
					//slave ch + master ch
					if(myCh->op.Isens + myData->bData[bd].cData[ch-1].op.Isens 
						>= faultLowerI_step) {
						myCh->misc.errCnt[C_CNT_LOWER_CURRENT_STEP]++;
						if(myCh->misc.errCnt[C_CNT_LOWER_CURRENT_STEP]>= MAX_ERROR_CNT){
							myCh->misc.errCnt[C_CNT_LOWER_CURRENT_STEP] = 0;
							myCh->misc.tmpState = myCh->op.state;
							myCh->misc.tmpCode = myCh->op.code;
							myCh->op.code = C_FAULT_LOWER_CURRENT;
							myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); //step_safety
							Fault_Value_Check(myCh->misc.advStepNo,
							myCh->op.Isens + myData->bData[bd].cData[ch-1].op.
							Isens, faultLowerI_step, 0, 0, 0, 0, 0, 0, 0);
							rtn = FAULT_COND;
							break;
						}
					}else{
						myCh->misc.errCnt[C_CNT_LOWER_CURRENT_STEP] = 0;
					}
				}
			}
			//20180314 sch modify
			if(myPs->config.LimitVI.limit_use_I != 0 
				|| myPs->config.LimitVI.limit_use_V != 0){
				rtn = Fault_Check_default_LG(bd, ch);
				if(rtn == FAULT_COND)
					break;
			}
			//19090l lyhw
			if(myPs->config.function[F_CHANGE_VI_CHECK] != 0){
				rtn = Fault_Check_Change_VI(bd, ch, advStepNo);
				if(rtn == FAULT_COND)	break;
			}
			/*
			if(myPs->config.LimitVI.limit_use_V != 0){
				rtn = Fault_Check_default_LG(bd, ch);
				if(rtn == FAULT_COND)
					break;
			}*/
			break;
		case STEP_USER_PATTERN:
			//100908 pjy temp fault check without delay
			if(myPs->testCond[bd][ch].safety.faultUpperTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 0);
				if(rtn == FAULT_COND)
					break;
			}
			if(myPs->testCond[bd][ch].safety.faultLowerTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 1);
				if(rtn == FAULT_COND)
					break;
			}
			if(myPs->testCond[bd][ch].step[advStepNo].faultUpperTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 2);
				if(rtn == FAULT_COND)
					break;
			}
			if(myPs->testCond[bd][ch].step[advStepNo].faultLowerTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 3);
				if(rtn == FAULT_COND)
					break;
			}
			//190311 lyh add
			if(myTestCond->step[advStepNo].pauseUpperTemp != 0) {
				rtn = tempPauseCheck(bd, ch, advStepNo, 0);
				if(rtn == FAULT_COND)
					break;
			}
			if(myTestCond->step[advStepNo].pauseLowerTemp != 0) {
				rtn = tempPauseCheck(bd, ch, advStepNo, 1);
				if(rtn == FAULT_COND)
					break;
			}
			if(myPs->testCond[bd][ch].safety.faultUpperV != 0) {
				if(myCh->op.Vsens
					>= myPs->testCond[bd][ch].safety.faultUpperV) {
					myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_VOLTAGE;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); //safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Vsens,
						myPs->testCond[bd][ch].safety.faultUpperV,
					   	0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE] = 0;
				}
			}
			if(myPs->testCond[bd][ch].safety.faultLowerV != 0) {
				if(myCh->op.Vsens
					<= myPs->testCond[bd][ch].safety.faultLowerV) {
					myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE]++;
					if(myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_LOWER_VOLTAGE;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); //safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Vsens,
						myPs->testCond[bd][ch].safety.faultLowerV,
					   	0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE] = 0;
				}
			}
			if(myCh->op.checkDelayTime < 180) break;
			if(faultUpperI_com != 0) {
				//slave ch + master ch
				if(myCh->op.Isens + myData->bData[bd].cData[ch-1].op.Isens 
					>= faultUpperI_com) {
					myCh->misc.errCnt[C_CNT_UPPER_CURRENT]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_CURRENT]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_UPPER_CURRENT] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_CURRENT;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); //safety
						Fault_Value_Check(myCh->misc.advStepNo, 
						myCh->op.Isens + myData->bData[bd].cData[ch-1].op.Isens,
						faultUpperI_com, 0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_UPPER_CURRENT] = 0;
				}
			}
			if(faultLowerI_com != 0) {
				//slave ch + master ch
				if(myCh->op.Isens + myData->bData[bd].cData[ch-1].op.Isens 
					<= faultLowerI_com) {
					myCh->misc.errCnt[C_CNT_LOWER_CURRENT]++;
					if(myCh->misc.errCnt[C_CNT_LOWER_CURRENT]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_LOWER_CURRENT] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_LOWER_CURRENT;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); //safety
						Fault_Value_Check(myCh->misc.advStepNo,
						myCh->op.Isens + myData->bData[bd].cData[ch-1].op.Isens,
						faultLowerI_com, 0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_LOWER_CURRENT] = 0;
				}
			}
			if(myPs->testCond[bd][ch].safety.faultUpperC != 0) {
				if(capacity >= myPs->testCond[bd][ch].safety.faultUpperC) {
					myCh->misc.errCnt[C_CNT_UPPER_CAPACITY]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_CAPACITY]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_UPPER_CAPACITY] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_CAPACITY;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); //safety
						Fault_Value_Check(myCh->misc.advStepNo, capacity,
						myPs->testCond[bd][ch].safety.faultUpperC,
					   	0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_UPPER_CAPACITY] = 0;
				}
			}
			if(myPs->testCond[bd][ch].step[advStepNo].faultUpperV != 0) {
				if(myCh->op.Vsens
					>= myPs->testCond[bd][ch].step[advStepNo].faultUpperV) {
					myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE_STEP]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE_STEP]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE_STEP] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_VOLTAGE;
						myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); //step_safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Vsens,
						myPs->testCond[bd][ch].step[advStepNo].faultUpperV,
					   	0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE_STEP] = 0;
				}
			}
			if(myPs->testCond[bd][ch].step[advStepNo].faultLowerV != 0) {
				if(myCh->op.Vsens 
					<= myPs->testCond[bd][ch].step[advStepNo].faultLowerV) {
					myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE_STEP]++;
					if(myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE_STEP]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE_STEP] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_LOWER_VOLTAGE;
						myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); //step_safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Vsens,
						myPs->testCond[bd][ch].step[advStepNo].faultLowerV,
					   	0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE_STEP] = 0;
				}
			}
			if(faultUpperI_step != 0) {
				//slave ch + master ch
				if(myCh->op.Isens + myData->bData[bd].cData[ch-1].op.Isens 
					>= faultUpperI_step) {
					myCh->misc.errCnt[C_CNT_UPPER_CURRENT_STEP]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_CURRENT_STEP]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_UPPER_CURRENT_STEP] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_CURRENT;
						myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); //step_safety
						Fault_Value_Check(myCh->misc.advStepNo, 
						myCh->op.Isens + myData->bData[bd].cData[ch-1].op.Isens,
						faultUpperI_step, 0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_UPPER_CURRENT_STEP] = 0;
				}
			}
			if(faultLowerI_step != 0) {
				//slave ch + master ch
				if(myCh->op.Isens + myData->bData[bd].cData[ch-1].op.Isens 
					<= faultLowerI_step) {
					myCh->misc.errCnt[C_CNT_LOWER_CURRENT_STEP]++;
					if(myCh->misc.errCnt[C_CNT_LOWER_CURRENT_STEP]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_LOWER_CURRENT_STEP] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_LOWER_CURRENT;
						myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); //step_safety
						Fault_Value_Check(myCh->misc.advStepNo,
						myCh->op.Isens + myData->bData[bd].cData[ch-1].op.Isens,
						faultLowerI_step, 0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_LOWER_CURRENT_STEP] = 0;
				}
			}
			if(myPs->testCond[bd][ch].step[advStepNo].faultUpperC != 0) {
				if(capacity >=
					myPs->testCond[bd][ch].step[advStepNo].faultUpperC) {
					myCh->misc.errCnt[C_CNT_UPPER_CAPACITY_STEP]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_CAPACITY_STEP]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_UPPER_CAPACITY_STEP] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_CAPACITY;
						myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); //step_safety
						Fault_Value_Check(myCh->misc.advStepNo, capacity,
						myPs->testCond[bd][ch].step[advStepNo].faultUpperC,
					   	0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_UPPER_CAPACITY_STEP] = 0;
				}
			}
			//hun_211228_s 병렬 부분 누락으로 인해 추가
			#if CHANGE_VI_CHECK == 1
			if(myTestCond->step[advStepNo].faultRunTime != 0) {
				if(myCh->op.runTime >= myTestCond->step[advStepNo].faultRunTime){
					myCh->misc.tmpState = myCh->op.state;
					myCh->misc.tmpCode = myCh->op.code;
					myCh->op.code = C_FAULT_RUN_TIME;
					rtn = FAULT_COND;
					break;
				}
			}
			#endif
			//hun_211228_e 병렬 부분 누락으로 인해 추가

			//20180314 sch modify
			if(myPs->config.LimitVI.limit_use_I != 0 
				|| myPs->config.LimitVI.limit_use_V != 0){
				rtn = Fault_Check_default_LG(bd, ch);
				if(rtn == FAULT_COND)
					break;
			}
			//19090l lyhw
			if(myPs->config.function[F_CHANGE_VI_CHECK] != 0){
				rtn = Fault_Check_Change_VI(bd, ch, advStepNo);
				if(rtn == FAULT_COND)	break;
			}
			/*
			if(myPs->config.LimitVI.limit_use_V != 0){
				rtn = Fault_Check_default_LG(bd, ch);
				if(rtn == FAULT_COND)
					break;
			}*/
			break;
		case STEP_USER_MAP:
			//100908 pjy temp fault check without delay
			if(myPs->testCond[bd][ch].safety.faultUpperTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 0);
				if(rtn == FAULT_COND)
					break;
			}
			if(myPs->testCond[bd][ch].safety.faultLowerTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 1);
				if(rtn == FAULT_COND)
					break;
			}
			if(myPs->testCond[bd][ch].step[advStepNo].faultUpperTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 2);
				if(rtn == FAULT_COND)
					break;
			}
			if(myPs->testCond[bd][ch].step[advStepNo].faultLowerTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 3);
				if(rtn == FAULT_COND)
					break;
			}
			//190311 lyh add
			if(myTestCond->step[advStepNo].pauseUpperTemp != 0) {
				rtn = tempPauseCheck(bd, ch, advStepNo, 0);
				if(rtn == FAULT_COND)
					break;
			}
			if(myTestCond->step[advStepNo].pauseLowerTemp != 0) {
				rtn = tempPauseCheck(bd, ch, advStepNo, 1);
				if(rtn == FAULT_COND)
					break;
			}
			if(myPs->testCond[bd][ch].safety.faultUpperV != 0) {
				if(myCh->op.Vsens
					>= myPs->testCond[bd][ch].safety.faultUpperV) {
					myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_VOLTAGE;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); //safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Vsens,
						myPs->testCond[bd][ch].safety.faultUpperV,
					   	0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE] = 0;
				}
			}
			if(myPs->testCond[bd][ch].safety.faultLowerV != 0) {
				if(myCh->op.Vsens
					<= myPs->testCond[bd][ch].safety.faultLowerV) {
					myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE]++;
					if(myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_LOWER_VOLTAGE;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); //safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Vsens,
						myPs->testCond[bd][ch].safety.faultLowerV,
					   	0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE] = 0;
				}
			}
			if(myCh->op.checkDelayTime < 180) break;
			if(faultUpperI_com != 0) {
				//slave ch + master ch
				if(myCh->op.Isens + myData->bData[bd].cData[ch-1].op.Isens 
					>= faultUpperI_com) {
					myCh->misc.errCnt[C_CNT_UPPER_CURRENT]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_CURRENT]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_UPPER_CURRENT] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_CURRENT;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); //safety
						Fault_Value_Check(myCh->misc.advStepNo, 
						myCh->op.Isens + myData->bData[bd].cData[ch-1].op.Isens,
						faultUpperI_com, 0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_UPPER_CURRENT] = 0;
				}
			}
			if(faultLowerI_com != 0) {
				//slave ch + master ch
				if(myCh->op.Isens + myData->bData[bd].cData[ch-1].op.Isens 
					<= faultLowerI_com) {
					myCh->misc.errCnt[C_CNT_LOWER_CURRENT]++;
					if(myCh->misc.errCnt[C_CNT_LOWER_CURRENT]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_LOWER_CURRENT] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_LOWER_CURRENT;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); //safety
						Fault_Value_Check(myCh->misc.advStepNo,
						myCh->op.Isens + myData->bData[bd].cData[ch-1].op.Isens,
						faultLowerI_com, 0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_LOWER_CURRENT] = 0;
				}
			}
			if(myPs->testCond[bd][ch].safety.faultUpperC != 0) {
				if(labs(capacity) >= myPs->testCond[bd][ch].safety.faultUpperC) {
					myCh->misc.errCnt[C_CNT_UPPER_CAPACITY]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_CAPACITY]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_UPPER_CAPACITY] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_CAPACITY;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); //safety
						Fault_Value_Check(myCh->misc.advStepNo, capacity,
						myPs->testCond[bd][ch].safety.faultUpperC,
					   	0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_UPPER_CAPACITY] = 0;
				}
			}
			if(myPs->testCond[bd][ch].step[advStepNo].faultUpperV != 0) {
				if(myCh->op.Vsens
					>= myPs->testCond[bd][ch].step[advStepNo].faultUpperV) {
					myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE_STEP]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE_STEP]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE_STEP] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_VOLTAGE;
						myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); //step_safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Vsens,
						myPs->testCond[bd][ch].step[advStepNo].faultUpperV,
					   	0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE_STEP] = 0;
				}
			}
			if(myPs->testCond[bd][ch].step[advStepNo].faultLowerV != 0) {
				if(myCh->op.Vsens 
					<= myPs->testCond[bd][ch].step[advStepNo].faultLowerV) {
					myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE_STEP]++;
					if(myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE_STEP]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE_STEP] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_LOWER_VOLTAGE;
						myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); //step_safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Vsens,
						myPs->testCond[bd][ch].step[advStepNo].faultLowerV,
					   	0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE_STEP] = 0;
				}
			}
			if(faultUpperI_step != 0) {
				//slave ch + master ch
				if(myCh->op.Isens + myData->bData[bd].cData[ch-1].op.Isens 
					>= faultUpperI_step) {
					myCh->misc.errCnt[C_CNT_UPPER_CURRENT_STEP]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_CURRENT_STEP]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_UPPER_CURRENT_STEP] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_CURRENT;
						myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); //step_safety
						Fault_Value_Check(myCh->misc.advStepNo, 
						myCh->op.Isens + myData->bData[bd].cData[ch-1].op.Isens,
						faultUpperI_step, 0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_UPPER_CURRENT_STEP] = 0;
				}
			}
			if(faultLowerI_step != 0) {
				//slave ch + master ch
				if(myCh->op.Isens + myData->bData[bd].cData[ch-1].op.Isens 
					<= faultLowerI_step) {
					myCh->misc.errCnt[C_CNT_LOWER_CURRENT_STEP]++;
					if(myCh->misc.errCnt[C_CNT_LOWER_CURRENT_STEP]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_LOWER_CURRENT_STEP] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_LOWER_CURRENT;
						myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); //step_safety
						Fault_Value_Check(myCh->misc.advStepNo,
						myCh->op.Isens + myData->bData[bd].cData[ch-1].op.Isens,
						faultLowerI_step, 0, 0, 0, 0, 0, 0 ,0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_LOWER_CURRENT_STEP] = 0;
				}
			}
			if(myPs->testCond[bd][ch].step[advStepNo].faultUpperC != 0) {
				if(labs(capacity) >=
					myPs->testCond[bd][ch].step[advStepNo].faultUpperC) {
					myCh->misc.errCnt[C_CNT_UPPER_CAPACITY_STEP]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_CAPACITY_STEP]>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_UPPER_CAPACITY_STEP] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_CAPACITY;
						myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); //step_safety
						Fault_Value_Check(myCh->misc.advStepNo, capacity,
						myPs->testCond[bd][ch].step[advStepNo].faultUpperC,
					   	0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_UPPER_CAPACITY_STEP] = 0;
				}
			}
			//20180314 sch modify
			if(myPs->config.LimitVI.limit_use_I != 0 
				|| myPs->config.LimitVI.limit_use_V != 0){
				rtn = Fault_Check_default_LG(bd, ch);
				if(rtn == FAULT_COND)
					break;
			}
			//19090l lyhw
			if(myPs->config.function[F_CHANGE_VI_CHECK] != 0){
				rtn = Fault_Check_Change_VI(bd, ch, advStepNo);
				if(rtn == FAULT_COND)	break;
			}
			/*
			if(myPs->config.LimitVI.limit_use_V != 0){
				rtn = Fault_Check_default_LG(bd, ch);
				if(rtn == FAULT_COND)
					break;
			}*/
			break;
		case STEP_BALANCE:
			break;
		default: 
			break;
	}

#ifdef _USER_VI
	if(myData->mData.config.LimitVI.limit_use_I != 0){
		if(myCh->signal[C_SIG_LIMIT_CURRENT] == P1) { 
			myCh->signal[C_SIG_LIMIT_CURRENT] = P0;
			myCh->misc.tmpState = myCh->op.state;
			myCh->misc.tmpCode = myCh->op.code;
			if(myPs->config.LimitVI.limit_action_I == P0){
				myCh->op.code = C_FAULT_LIMIT_CURRENT;
			}else if(myPs->config.LimitVI.limit_action_I == P1){
				myCh->op.code = C_FAULT_END_LIMIT_CURRENT;
			}
			rtn = FAULT_COND;
		}
	}
	if(myData->mData.config.LimitVI.limit_use_V != 0){
		if(myCh->signal[C_SIG_LIMIT_VOLTAGE] == P1) { //20171024 sch add
			myCh->signal[C_SIG_LIMIT_VOLTAGE] = P0;
			myCh->misc.tmpState = myCh->op.state;
			myCh->misc.tmpCode = myCh->op.code;
			if(myPs->config.LimitVI.limit_action_V == P0){
				myCh->op.code = C_FAULT_LIMIT_VOLTAGE;
			}else if(myPs->config.LimitVI.limit_action_V == P1){
				myCh->op.code = C_FAULT_END_LIMIT_VOLTAGE;
			}
			rtn = FAULT_COND;
		}
	}
#endif

	//hun_211228_s 병렬 부분 누락으로 인해 추가
	//gasVoltage [uV]
	//GAS_VOLTAGE_MIN [uV] GA_VOLTAGE_MAX [uV]
	//GAS_CHECK_TIME [sec]
	if(rtn != FAULT_COND && myData->mData.config.swFaultConfig[CHAMBER_GAS_VOLTAGE_MIN] != 0){
		if(myCh->misc.gasVoltage < myData->mData.config.swFaultConfig[CHAMBER_GAS_VOLTAGE_MIN]) {
			if(myCh->misc.gas_check_flag == 0){
				myCh->misc.gas_check_time = myCh->op.runTime;	
				myCh->misc.gas_check_flag = 1;
			}else if(myCh->misc.gas_check_flag == 1){
				if(labs(myCh->misc.gas_check_time - myCh->op.runTime) >
					myData->mData.config.swFaultConfig[GAS_CHECK_TIME] * 100){
					myCh->misc.tmpState = myCh->op.state;
					myCh->misc.tmpCode = myCh->op.code;
					myCh->op.code = C_FAULT_GAS_VOLTAGE_MIN;
					rtn = FAULT_COND;
				}
			}
		}else if(myCh->misc.gasVoltage > myData->mData.config.swFaultConfig[CHAMBER_GAS_VOLTAGE_MIN]) {
			myCh->misc.gas_check_flag = 0;
			myCh->misc.gas_check_time = myCh->op.runTime;	
		}
	}
	if(rtn != FAULT_COND && myData->mData.config.swFaultConfig[CHAMBER_GAS_VOLTAGE_MAX] != 0){
		if(myCh->misc.gasVoltage > myData->mData.config.swFaultConfig[CHAMBER_GAS_VOLTAGE_MAX]) { 
			if(myCh->misc.gas_check_flag == 0){
				myCh->misc.gas_check_time = myCh->op.runTime;	
				myCh->misc.gas_check_flag = 1;
			}else if(myCh->misc.gas_check_flag == 1){
				if(labs(myCh->misc.gas_check_time - myCh->op.runTime) >
					myData->mData.config.swFaultConfig[GAS_CHECK_TIME] * 100){
					myCh->misc.tmpState = myCh->op.state;
					myCh->misc.tmpCode = myCh->op.code;
					myCh->op.code = C_FAULT_GAS_VOLTAGE_MAX;
					rtn = FAULT_COND;
				}
			}
		}else if(myCh->misc.gasVoltage < myData->mData.config.swFaultConfig[CHAMBER_GAS_VOLTAGE_MAX]) {
			myCh->misc.gas_check_flag = 0;
			myCh->misc.gas_check_time = myCh->op.runTime;	
		}
	}
	if(rtn != FAULT_COND && myData->mData.config.swFaultConfig[SOFT_VENTING_COUNT] != 0){
		if(myCh->misc.gasVoltage - myCh->misc.std_gasVoltage >= myData->mData.config.swFaultConfig[SOFT_VENTING_VALUE]){
			myCh->misc.soft_venting_count++;
			myCh->misc.std_gasVoltage = myCh->misc.gasVoltage;
		}else{
			myCh->misc.soft_venting_count = 0;
		}
		if(myCh->misc.soft_venting_count >= myData->mData.config.swFaultConfig[SOFT_VENTING_COUNT]){
			myCh->misc.tmpState = myCh->op.state;
			myCh->misc.tmpCode = myCh->op.code;
			myCh->op.code = C_FAULT_SOFT_VENTING;
			rtn = FAULT_COND;
		}
	}
	if(rtn != FAULT_COND && myData->mData.config.swFaultConfig[HARD_VENTING_VALUE] != 0){
		if(myCh->misc.gasVoltage - myCh->misc.std_gasVoltage >= myData->mData.config.swFaultConfig[HARD_VENTING_VALUE]){
			myCh->misc.tmpState = myCh->op.state;
			myCh->misc.tmpCode = myCh->op.code;
			myCh->op.code = C_FAULT_HARD_VENTING;
			rtn = FAULT_COND;
		}
	}

	if(rtn != FAULT_COND && myData->mData.config.swFaultConfig[AMBIENT_TEMP_MAX] != 0){
		if(myCh->misc.ambientTemp > myData->mData.config.swFaultConfig[AMBIENT_TEMP_MAX]) { 
			if(myCh->misc.ambient_check_flag == 0){
				myCh->misc.ambient_check_time = myCh->op.runTime;	
				myCh->misc.ambient_check_flag = 1;
			}else if(myCh->misc.ambient_check_flag == 1){
				if(labs(myCh->misc.ambient_check_time - myCh->op.runTime) >
					myData->mData.config.swFaultConfig[AMBIENT_TEMP_MAX_TIME] * 100){
					myCh->misc.tmpState = myCh->op.state;
					myCh->misc.tmpCode = myCh->op.code;
					myCh->op.code = C_FAULT_AMBIENT_TEMP_MAX;
					rtn = FAULT_COND;
				}
			}
		}else if(myCh->misc.ambientTemp < myData->mData.config.swFaultConfig[AMBIENT_TEMP_MAX]) {
			myCh->misc.ambient_check_flag = 0;
			myCh->misc.ambient_check_time = myCh->op.runTime;	
		}
	}
	
	if(myData->bData[bd].cData[ch].misc.Fault_flag != 0){
		myData->bData[bd].cData[ch].misc.Fault_flag = 0;
		switch(myCh->misc.ch_fault_code){
			case C_FAULT_MASTER_RECIPE_VOLTAGE_ERR://SDI
			case C_FAULT_MASTER_RECIPE_CURRENT_ERR://SDI
			case C_FAULT_MASTER_RECIPE_TIME_ERR://SDI
			case C_FAULT_MASTER_RECIPE_TEMP_ERR://SDI
			case C_FAULT_MASTER_RECIPE_GROUP_ERR://SDI
			case C_FAULT_GUI_AMBIENT_TEMP_ERROR://LGES
			case C_FAULT_GUI_UPPER_VOLTAGE://LGES
			case C_FAULT_GUI_UPPER_TEMP://LGES
			case C_FAULT_GUI_LOWER_VOLTAGE://LGES
			case C_FAULT_GUI_CHAMBER_DI://LGES
				myCh->misc.tmpState = myCh->op.state;
				myCh->misc.tmpCode = myCh->op.code;
				myCh->op.code = myCh->misc.ch_fault_code;
				rtn = FAULT_COND;
				break;
			default:	//SDI
				myCh->misc.tmpState = myCh->op.state;
				myCh->misc.tmpCode = myCh->op.code;
				myCh->op.code = myCh->misc.ch_fault_code;
				rtn = FAULT_COND;
				break;
		}	
	}
	//hun_211228_e 병렬 부분 누락으로 인해 추가
//220214 LJS For LGES semi-auto jig PLC safety Condition
#ifdef _GROUP_ERROR
	if(myCh->misc.Fault_Check_flag == 0){
		if(myCh->misc.groupAvgVsens != 0 && myCh->misc.stepSyncFlag != 0){
			if(myTestCond->step[advStepNo].group_DeltaVoltage != 0){
				if(labs(myCh->misc.groupAvgVsens - myCh->op.Vsens) >=
						myTestCond->step[advStepNo].group_DeltaVoltage){
					myCh->misc.Std_Time = myCh->op.runTime;
					myCh->misc.Fault_Check_flag = 1;
				}
			}
		}
	}
	if(myCh->misc.Fault_Check_flag == 1){
		rtn = Group_Error_Fault_Check(bd, ch, advStepNo);
	}
	
	if(myTestCond->step[advStepNo].group_EndFaultTime != 0 &&
			myCh->misc.endState == P2){
		if(myCh->op.runTime >= myCh->misc.groupEndTime + 
			 myTestCond->step[advStepNo].group_EndFaultTime){
			myCh->misc.tmpState = myCh->op.state;
			myCh->misc.tmpCode = myCh->op.code;
			myCh->op.code = C_FAULT_GROUP_TIME_ERROR;
			rtn = FAULT_COND;
		}	
	}
#endif

	if(rtn == FAULT_COND) { //fault
		myData->bData[bd].cData[ch-1].op.code = myCh->op.code;
		myData->bData[bd].cData[ch-1].misc.tmpCode = myCh->misc.tmpCode;
		myData->bData[bd].cData[ch-1].misc.tmpState = myCh->misc.tmpState;
		myData->bData[bd].cData[ch-1].op.phase = P100;
		
		myCh->op.select = SAVE_FLAG_SAVING_ETC;
		myData->bData[bd].cData[ch-1].op.select = myCh->op.select;
		send_save_msg(bd, ch, saveDt, 0);
		myCh = &(myData->bData[bd].cData[ch]);
		myCh->op.phase = P100;
		
//		myData->bData[bd].cData[ch-1].op.code = myCh->op.code;
//		myData->bData[bd].cData[ch-1].misc.tmpCode = myCh->misc.tmpCode;
//		myData->bData[bd].cData[ch-1].misc.tmpState = myCh->misc.tmpState;
//		myData->bData[bd].cData[ch-1].op.phase = P100;
		//170106 oys add
		if(myData->bData[bd].cData[ch-1].misc.tempWaitType == P0){
			myData->bData[bd].cData[ch-1].misc.chGroupNo = 0;
			//20180417 add
			myData->bData[bd].cData[ch-1].misc.stepSyncFlag = 0;
			myData->bData[bd].cData[ch-1].misc.endState = 0;		//220223_hun
			myData->bData[bd].cData[ch-1].misc.groupAvgVsens = 0; 	//220223_hun
			myData->bData[bd].cData[ch-1].misc.group_StartVoltage_flag = 0;
		}
		cNextStepCheck(bd, ch-1);
		myCh = &(myData->bData[bd].cData[ch]);
		cNextStepCheck(bd, ch);
	}
}

void cFaultCondSoft_P2(int bd, int ch)
{
	unsigned char type;
    int rtn=0, rangeV, rangeI, mode;
	long capacity, refV, refI, maxV, maxI;
	long cvtVal, faultUpperI_com, faultLowerI_com, faultUpperI_step, faultLowerI_step;
	unsigned long advStepNo, saveDt;
	S_CH_STEP_INFO step;

	myCh = &(myData->bData[bd].cData[ch]);
	myTestCond = &(myPs->testCond[bd][ch]);

	if(myCh->op.state != C_RUN) return;
	if(myCh->misc.patternPhase != 0) return;
	if(myCh->op.phase != P50) return;

	step = step_info(bd, ch);

	advStepNo = step.advStepNo;
	saveDt = step.saveDt;
	type = step.type;

	refV = step.refV;
	refI = step.refI * 2;
	rangeV = step.rangeV;
	rangeI = step.rangeI;
	mode = step.mode;

	maxV = myPs->config.maxVoltage[rangeV];
	maxI = myPs->config.maxCurrent[rangeI] * 2;

	//190801 oys add start : Convert C-rate to I value
	faultUpperI_com = myPs->testCond[bd][ch].safety.faultUpperI;
	cvtVal = Convert_C_rate_to_I_value(bd, ch, faultUpperI_com);
	if(cvtVal != 0)
		faultUpperI_com = cvtVal;

	faultLowerI_com = myPs->testCond[bd][ch].safety.faultLowerI;
	cvtVal = Convert_C_rate_to_I_value(bd, ch, faultLowerI_com);
	if(cvtVal != 0)
		faultLowerI_com = cvtVal;
	
	faultUpperI_step = myPs->testCond[bd][ch].step[advStepNo].faultUpperI;
	cvtVal = Convert_C_rate_to_I_value(bd, ch, faultUpperI_step);
	if(cvtVal != 0)
		faultUpperI_step = cvtVal;
	
	faultLowerI_step = myPs->testCond[bd][ch].step[advStepNo].faultLowerI;
	cvtVal = Convert_C_rate_to_I_value(bd, ch, faultLowerI_step);
	if(cvtVal != 0)
		faultLowerI_step = cvtVal;
	//add end

	if(myPs->config.capacityType == CAPACITY_AMPARE_HOURS) {
		capacity = myCh->op.ampareHour;
	} else if(myPs->config.capacityType == CAPACITY_CAPACITANCE) {
		capacity = myCh->op.capacitance;
	} else {
		capacity = 0;
	}

	switch(type) {
		case STEP_OCV:
			if(myPs->testCond[bd][ch].safety.faultUpperV != 0) {
				if(myCh->op.Vsens
					>= myPs->testCond[bd][ch].safety.faultUpperV) {
					myCh->misc.errCnt[C_CNT_UPPER_OCV]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_OCV] >= MAX_ERROR_CNT) {
						myCh->misc.errCnt[C_CNT_UPPER_OCV] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_OCV;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); 
						//safety
						rtn = FAULT_COND;
						break;
					}
				} else {
					myCh->misc.errCnt[C_CNT_UPPER_OCV] = 0;
				}
			}

			if(myPs->testCond[bd][ch].safety.faultLowerV != 0) {
			   	if(myCh->op.Vsens
					<= myPs->testCond[bd][ch].safety.faultLowerV) {
					myCh->misc.errCnt[C_CNT_LOWER_OCV]++;
					if(myCh->misc.errCnt[C_CNT_LOWER_OCV] >= MAX_ERROR_CNT) {
						myCh->misc.errCnt[C_CNT_LOWER_OCV] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_LOWER_OCV;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); 
						//safety
						rtn = FAULT_COND;
						break;
					}
				} else {
					myCh->misc.errCnt[C_CNT_LOWER_OCV] = 0;
				}
			}

			if(myPs->testCond[bd][ch].safety.faultUpperTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 0);
				if(rtn == FAULT_COND) break;
			}
			if(myPs->testCond[bd][ch].safety.faultLowerTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 1);
				if(rtn == FAULT_COND) break;
			}

			if(myPs->testCond[bd][ch].step[advStepNo].faultUpperV != 0) {
				if(myCh->op.Vsens >=
					myPs->testCond[bd][ch].step[advStepNo].faultUpperV) {
					myCh->misc.errCnt[C_CNT_UPPER_OCV_STEP]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_OCV_STEP]
						>= MAX_ERROR_CNT) {
						myCh->misc.errCnt[C_CNT_UPPER_OCV_STEP] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_OCV;
						myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); 
						//step_safety
						rtn = FAULT_COND;
						break;
					}
				} else {
					myCh->misc.errCnt[C_CNT_UPPER_OCV_STEP] = 0;
				}
			}

			if(myPs->testCond[bd][ch].step[advStepNo].faultLowerV != 0) {
			   	if(myCh->op.Vsens <=
					myPs->testCond[bd][ch].step[advStepNo].faultLowerV) {
					myCh->misc.errCnt[C_CNT_LOWER_OCV_STEP]++;
					if(myCh->misc.errCnt[C_CNT_LOWER_OCV_STEP]
					>= MAX_ERROR_CNT) {
						myCh->misc.errCnt[C_CNT_LOWER_OCV_STEP] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_LOWER_OCV;
						myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); 
						//step_safety
						rtn = FAULT_COND;
						break;
					}
				} else {
					myCh->misc.errCnt[C_CNT_LOWER_OCV_STEP] = 0;
				}
			}

			if(myPs->testCond[bd][ch].step[advStepNo].faultUpperTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 2);
				if(rtn == FAULT_COND) break;
			}
			if(myPs->testCond[bd][ch].step[advStepNo].faultLowerTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 3);
				if(rtn == FAULT_COND) break;
			}
			//190311 lyh add
			if(myTestCond->step[advStepNo].pauseUpperTemp != 0) {
				rtn = tempPauseCheck(bd, ch, advStepNo, 0);
				if(rtn == FAULT_COND)
					break;
			}
			if(myTestCond->step[advStepNo].pauseLowerTemp != 0) {
				rtn = tempPauseCheck(bd, ch, advStepNo, 1);
				if(rtn == FAULT_COND)
					break;
			}

			//20180314 sch modify
			if(myPs->config.LimitVI.limit_use_I != 0 
				|| myPs->config.LimitVI.limit_use_V != 0) {
				rtn = Fault_Check_default_LG(bd, ch);
				if(rtn == FAULT_COND) break;
			}
			break;
		case STEP_REST:
			if(myPs->testCond[bd][ch].safety.faultUpperV != 0) {
				if(myCh->op.Vsens
					>= myPs->testCond[bd][ch].safety.faultUpperV) {
					myCh->misc.errCnt[C_CNT_UPPER_OCV]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_OCV] >= MAX_ERROR_CNT) {
						myCh->misc.errCnt[C_CNT_UPPER_OCV] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_OCV;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); 
						//safety
						rtn = FAULT_COND;
						break;
					}
				} else {
					myCh->misc.errCnt[C_CNT_UPPER_OCV] = 0;
				}
			}

			if(myPs->testCond[bd][ch].safety.faultLowerV != 0) {
			   	if(myCh->op.Vsens
					<= myPs->testCond[bd][ch].safety.faultLowerV) {
					myCh->misc.errCnt[C_CNT_LOWER_OCV]++;
					if(myCh->misc.errCnt[C_CNT_LOWER_OCV] >= MAX_ERROR_CNT) {
						myCh->misc.errCnt[C_CNT_LOWER_OCV] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_LOWER_OCV;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); 
						//safety
						rtn = FAULT_COND;
						break;
					}
				} else {
					myCh->misc.errCnt[C_CNT_LOWER_OCV] = 0;
				}
			}

			if(myPs->testCond[bd][ch].safety.faultUpperTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 0);
				if(rtn == FAULT_COND) break;
			}
			if(myPs->testCond[bd][ch].safety.faultLowerTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 1);
				if(rtn == FAULT_COND) break;
			}

			if(myPs->testCond[bd][ch].step[advStepNo].faultUpperV != 0) {
				if(myCh->op.Vsens >=
					myPs->testCond[bd][ch].step[advStepNo].faultUpperV) {
					myCh->misc.errCnt[C_CNT_UPPER_OCV_STEP]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_OCV_STEP]
						>= MAX_ERROR_CNT) {
						myCh->misc.errCnt[C_CNT_UPPER_OCV_STEP] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_OCV;
						myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); 
						//step_safety
						rtn = FAULT_COND;
						break;
					}
				} else {
					myCh->misc.errCnt[C_CNT_UPPER_OCV_STEP] = 0;
				}
			}

			if(myPs->testCond[bd][ch].step[advStepNo].faultLowerV != 0) {
			   	if(myCh->op.Vsens <=
					myPs->testCond[bd][ch].step[advStepNo].faultLowerV) {
					myCh->misc.errCnt[C_CNT_LOWER_OCV_STEP]++;
					if(myCh->misc.errCnt[C_CNT_LOWER_OCV_STEP]
						>= MAX_ERROR_CNT) {
						myCh->misc.errCnt[C_CNT_LOWER_OCV_STEP] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_LOWER_OCV;
						myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); 
						//step_safety
						rtn = FAULT_COND;
						break;
					}
				} else {
					myCh->misc.errCnt[C_CNT_LOWER_OCV_STEP] = 0;
				}
			}

			if(myPs->testCond[bd][ch].step[advStepNo].faultUpperTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 2);
				if(rtn == FAULT_COND) break;
			}
			if(myPs->testCond[bd][ch].step[advStepNo].faultLowerTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 3);
				if(rtn == FAULT_COND) break;
			}
			//190311 lyh add
			if(myTestCond->step[advStepNo].pauseUpperTemp != 0) {
				rtn = tempPauseCheck(bd, ch, advStepNo, 0);
				if(rtn == FAULT_COND)
					break;
			}
			if(myTestCond->step[advStepNo].pauseLowerTemp != 0) {
				rtn = tempPauseCheck(bd, ch, advStepNo, 1);
				if(rtn == FAULT_COND)
					break;
			}

			//20180314 sch modify
			if(myPs->config.LimitVI.limit_use_I != 0 
				|| myPs->config.LimitVI.limit_use_V != 0) {
				rtn = Fault_Check_default_LG(bd, ch);
				if(rtn == FAULT_COND) break;
			}
			break;
    	case STEP_CHARGE:
			//100908 pjy temp fault check without delay
			if(myPs->testCond[bd][ch].safety.faultUpperTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 0);
				if(rtn == FAULT_COND) break;
			}
			if(myPs->testCond[bd][ch].safety.faultLowerTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 1);
				if(rtn == FAULT_COND) break;
			}

			if(myPs->testCond[bd][ch].step[advStepNo].faultUpperTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 2);
				if(rtn == FAULT_COND) break;
			}
			if(myPs->testCond[bd][ch].step[advStepNo].faultLowerTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 3);
				if(rtn == FAULT_COND) break;
			}
			//190311 lyh add
			if(myTestCond->step[advStepNo].pauseUpperTemp != 0) {
				rtn = tempPauseCheck(bd, ch, advStepNo, 0);
				if(rtn == FAULT_COND)
					break;
			}
			if(myTestCond->step[advStepNo].pauseLowerTemp != 0) {
				rtn = tempPauseCheck(bd, ch, advStepNo, 1);
				if(rtn == FAULT_COND)
					break;
			}
			//hun_211228_s 병렬 부분 누락으로 인해 추가
			#ifdef _SDI_SAFETY_V1	
			if(myTestCond->step[advStepNo].faultRunTime != 0) {
				if(myCh->op.runTime >= myTestCond->step[advStepNo].faultRunTime){
					myCh->misc.tmpState = myCh->op.state;
					myCh->misc.tmpCode = myCh->op.code;
					myCh->op.code = C_FAULT_RUN_TIME;
					Fault_Value_Check(myCh->misc.advStepNo, 
					myCh->op.runTime,
					myPs->testCond[bd][ch].step[advStepNo].faultRunTime,
					0, 0, 0, 0, 0, 0, 0);
					rtn = FAULT_COND;
					break;
				}
			}
			#endif
			#if CHANGE_VI_CHECK == 1
			if(myTestCond->step[advStepNo].faultRunTime != 0) {
				if(myCh->op.runTime >= myTestCond->step[advStepNo].faultRunTime){
					myCh->misc.tmpState = myCh->op.state;
					myCh->misc.tmpCode = myCh->op.code;
					myCh->op.code = C_FAULT_RUN_TIME;
					rtn = FAULT_COND;
					break;
				}
			}
			#endif
			#ifdef _ULSAN_SDI_SAFETY
			if(myTestCond->step[advStepNo].humpSet_T != 0 
				&& myTestCond->step[advStepNo].humpSet_I != 0){
				if(myCh->misc.cvFlag == P1){
					if(myCh->misc.cvTime_Ulsan <= 100){
						myCh->misc.humpComp_T = myCh->op.runTime;
						myCh->misc.humpComp_I = myCh->op.Isens;

						myCh->misc.humpCheck_T = myCh->misc.humpComp_T 
							+ myTestCond->step[advStepNo].humpSet_T;
					}
					if(myCh->misc.cvTime_Ulsan > 100){
						if(myCh->op.runTime >= myCh->misc.humpCheck_T){
							myCh->misc.humpCheck_I
								= myCh->op.Isens - myCh->misc.humpComp_I;

							myCh->misc.humpComp_T = myCh->op.runTime;
							myCh->misc.humpComp_I = myCh->op.Isens;

							myCh->misc.humpCheck_T = myCh->misc.humpComp_T 
								+ myTestCond->step[advStepNo].humpSet_T;

							if(myCh->misc.humpCheck_I 
								>= myTestCond->step[advStepNo].humpSet_I){
								myCh->misc.tmpState = myCh->op.state;
								myCh->misc.tmpCode = myCh->op.code;
								myCh->misc.cvFlag = 0;
								myCh->misc.cvTime_Ulsan = 0;
								myCh->op.code = C_FAULT_CV_CURRENT_HUMP;
								rtn = FAULT_COND;
								break;
							}
						}
					}
				}
			}
			#endif
			//hun_211228_e 병렬 부분 누락으로 인해 추가

			if(myPs->testCond[bd][ch].safety.faultUpperV != 0) {
				if(myCh->op.Vsens
					>= myPs->testCond[bd][ch].safety.faultUpperV) {
					myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE]
						>= MAX_ERROR_CNT) {
						myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_VOLTAGE;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); //safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Vsens,
						myPs->testCond[bd][ch].safety.faultUpperV,
					   	0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				} else {
					myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE] = 0;
				}
			}

			if(myPs->testCond[bd][ch].safety.faultLowerV != 0) {
				if(myCh->op.Vsens
					<= myPs->testCond[bd][ch].safety.faultLowerV) {
					myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE]++;
					if(myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE]
						>= MAX_ERROR_CNT) {
						myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_LOWER_VOLTAGE;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); //safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Vsens,
						myPs->testCond[bd][ch].safety.faultLowerV,
					   	0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				} else {
					myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE] = 0;
				}
			}

			if(myCh->op.checkDelayTime < 180) break;

			if(faultUpperI_com != 0) {
				if(myCh->op.Isens >= faultUpperI_com) {
					myCh->misc.errCnt[C_CNT_UPPER_CURRENT]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_CURRENT]
						>= MAX_ERROR_CNT) {
						myCh->misc.errCnt[C_CNT_UPPER_CURRENT] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_CURRENT;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); //safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Isens,
						faultUpperI_com, 0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				} else {
					myCh->misc.errCnt[C_CNT_UPPER_CURRENT] = 0;
				}
			}

			if(faultLowerI_com != 0) {
				if(myCh->op.Isens <= faultLowerI_com) {
					myCh->misc.errCnt[C_CNT_LOWER_CURRENT]++;
					if(myCh->misc.errCnt[C_CNT_LOWER_CURRENT]
						>= MAX_ERROR_CNT) {
						myCh->misc.errCnt[C_CNT_LOWER_CURRENT] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_LOWER_CURRENT;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); //safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Isens,
						faultLowerI_com, 0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				} else {
					myCh->misc.errCnt[C_CNT_LOWER_CURRENT] = 0;
				}
			}

			if(myPs->testCond[bd][ch].safety.faultUpperC != 0) {
				if(capacity >= myPs->testCond[bd][ch].safety.faultUpperC) {
					myCh->misc.errCnt[C_CNT_UPPER_CAPACITY]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_CAPACITY]
						>= MAX_ERROR_CNT) {
						myCh->misc.errCnt[C_CNT_UPPER_CAPACITY] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_CAPACITY;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); //safety
						Fault_Value_Check(myCh->misc.advStepNo, capacity,
						myPs->testCond[bd][ch].safety.faultUpperC,
					   	0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				} else {
					myCh->misc.errCnt[C_CNT_UPPER_CAPACITY] = 0;
				}
			}

			if(myPs->testCond[bd][ch].step[advStepNo].faultUpperV != 0) {
				if(myCh->op.Vsens >=
					myPs->testCond[bd][ch].step[advStepNo].faultUpperV) {
					myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE_STEP]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE_STEP]
						>= MAX_ERROR_CNT) {
						myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE_STEP] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_VOLTAGE;
						myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); //step_safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Vsens,
						myPs->testCond[bd][ch].step[advStepNo].faultUpperV,
					   	0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				} else {
					myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE_STEP] = 0;
				}
			}

			if(myPs->testCond[bd][ch].step[advStepNo].faultLowerV != 0) {
				if(myCh->op.Vsens <=
					myPs->testCond[bd][ch].step[advStepNo].faultLowerV) {
					myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE_STEP]++;
					if(myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE_STEP]
						>= MAX_ERROR_CNT) {
						myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE_STEP] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_LOWER_VOLTAGE;
						myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); //step_safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Vsens,
						myPs->testCond[bd][ch].step[advStepNo].faultLowerV,
					   	0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				} else {
					myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE_STEP] = 0;
				}
			}

			if(faultUpperI_step != 0) {
				if(myCh->op.Isens >= faultUpperI_step) {
					myCh->misc.errCnt[C_CNT_UPPER_CURRENT_STEP]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_CURRENT_STEP]
						>= MAX_ERROR_CNT) {
						myCh->misc.errCnt[C_CNT_UPPER_CURRENT_STEP] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_CURRENT;
						myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); //step_safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Isens,
						faultUpperI_step, 0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				} else {
					myCh->misc.errCnt[C_CNT_UPPER_CURRENT_STEP] = 0;
				}
			}

			if((refV - myCh->op.Vsens) > (maxV * 0.001)){		//kjc_200612
				if(faultLowerI_step != 0) {
					if(myCh->op.Isens <= faultLowerI_step) {
						myCh->misc.errCnt[C_CNT_LOWER_CURRENT_STEP]++;
						if(myCh->misc.errCnt[C_CNT_LOWER_CURRENT_STEP]
							>= MAX_ERROR_CNT) {
							myCh->misc.errCnt[C_CNT_LOWER_CURRENT_STEP] = 0;
							myCh->misc.tmpState = myCh->op.state;
							myCh->misc.tmpCode = myCh->op.code;
							myCh->op.code = C_FAULT_LOWER_CURRENT;
							myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); //step_safety
							Fault_Value_Check(myCh->misc.advStepNo, 
							myCh->op.Isens, faultLowerI_step,
						   	0, 0, 0, 0, 0, 0, 0);
							rtn = FAULT_COND;
							break;
						}
					} else {
						myCh->misc.errCnt[C_CNT_LOWER_CURRENT_STEP] = 0;
					}
				}
			}
			if(myPs->testCond[bd][ch].step[advStepNo].faultUpperC != 0) {
				if(capacity >=
					myPs->testCond[bd][ch].step[advStepNo].faultUpperC) {
					myCh->misc.errCnt[C_CNT_UPPER_CAPACITY_STEP]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_CAPACITY_STEP]
						>= MAX_ERROR_CNT) {
						myCh->misc.errCnt[C_CNT_UPPER_CAPACITY_STEP] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_CAPACITY;
						myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); //step_safety
						Fault_Value_Check(myCh->misc.advStepNo, capacity,
						myPs->testCond[bd][ch].step[advStepNo].faultUpperC,
					   	0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				} else {
					myCh->misc.errCnt[C_CNT_UPPER_CAPACITY_STEP] = 0;
				}
			}

			//170215 SCH add for DeltaV/I CHARGE
			if(myPs->testCond[bd][ch].step[advStepNo].faultDeltaV != 0) {
				if(myData->mData.config.function[F_DELTA_V_I] != 0) {
					if(labs(refI - myCh->op.Isens) < (maxI * 0.001)) {
						if(labs(myCh->misc.maxV - myCh->op.Vsens)
							>= myPs->testCond[bd][ch].step[advStepNo]
							.faultDeltaV) {
							myCh->misc.errCnt[C_CNT_DELTA_V_STEP]++;
							if(myCh->misc.errCnt[C_CNT_DELTA_V_STEP]
								> MAX_ERROR_CNT) {
								myCh->misc.errCnt[C_CNT_DELTA_V_STEP] = 0;
								myCh->misc.tmpState = myCh->op.state;
								myCh->misc.tmpCode = myCh->op.code;
								if(myData->mData.config.function[F_DELTA_V_I]
									== 1) {
									myCh->op.code = C_STOP_DELTA_V;
									rtn = FAULT_COND;
								} else if(myData->mData.config
									.function[F_DELTA_V_I] == 2) {
									myCh->op.code = C_FAULT_DELTA_V;
									rtn = FAULT_COND;
								}
								break;
							}
						} else {
							myCh->misc.errCnt[C_CNT_DELTA_V_STEP] = 0;
						}
					}
				}
			}

			if(myPs->testCond[bd][ch].step[advStepNo].faultDeltaI != 0) {
				if(myData->mData.config.function[F_DELTA_V_I] != 0) {
					if(labs(refV - myCh->op.Vsens) < (maxV * 0.001)) {
						if(labs(myCh->misc.maxI - myCh->op.Isens)
							>= myPs->testCond[bd][ch].step[advStepNo]
							.faultDeltaI) {
							myCh->misc.errCnt[C_CNT_DELTA_V_STEP]++;
							if(myCh->misc.errCnt[C_CNT_DELTA_V_STEP]
								> MAX_ERROR_CNT) {
								myCh->misc.errCnt[C_CNT_DELTA_I_STEP] = 0;
								myCh->misc.tmpState = myCh->op.state;
								myCh->misc.tmpCode = myCh->op.code;
								if(myData->mData.config.function[F_DELTA_V_I]
									== 1) {
									myCh->op.code = C_STOP_DELTA_I;
									rtn = FAULT_COND;
								} else if(myData->mData.config
									.function[F_DELTA_V_I] == 2) {
									myCh->op.code = C_FAULT_DELTA_I;
									rtn = FAULT_COND;
								}
								break;
							}
						} else {
							myCh->misc.errCnt[C_CNT_DELTA_V_STEP] = 0;
						}
					}
				}
			}

			//20180314 sch modify
			if(myPs->config.LimitVI.limit_use_I != 0 
				|| myPs->config.LimitVI.limit_use_V != 0) {
				rtn = Fault_Check_default_LG(bd, ch);
				if(rtn == FAULT_COND)
					break;
			}
			break;
		case STEP_DISCHARGE:
			//100908 pjy temp fault check without delay
			if(myPs->testCond[bd][ch].safety.faultUpperTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 0);
				if(rtn == FAULT_COND) break;
			}
			if(myPs->testCond[bd][ch].safety.faultLowerTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 1);
				if(rtn == FAULT_COND) break;
			}

			if(myPs->testCond[bd][ch].step[advStepNo].faultUpperTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 2);
				if(rtn == FAULT_COND) break;
			}
			if(myPs->testCond[bd][ch].step[advStepNo].faultLowerTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 3);
				if(rtn == FAULT_COND) break;
			}
			//190311 lyh add
			if(myTestCond->step[advStepNo].pauseUpperTemp != 0) {
				rtn = tempPauseCheck(bd, ch, advStepNo, 0);
				if(rtn == FAULT_COND)
					break;
			}
			if(myTestCond->step[advStepNo].pauseLowerTemp != 0) {
				rtn = tempPauseCheck(bd, ch, advStepNo, 1);
				if(rtn == FAULT_COND)
					break;
			}
			//hun_211228_s 병렬 부분 누락으로 인해 추가
			#ifdef _SDI_SAFETY_V1	
			if(myTestCond->step[advStepNo].faultRunTime != 0) {
				if(myCh->op.runTime >= myTestCond->step[advStepNo].faultRunTime){
					myCh->misc.tmpState = myCh->op.state;
					myCh->misc.tmpCode = myCh->op.code;
					myCh->op.code = C_FAULT_RUN_TIME;
					Fault_Value_Check(myCh->misc.advStepNo, 
					myCh->op.runTime,
					myPs->testCond[bd][ch].step[advStepNo].faultRunTime,
					0, 0, 0, 0, 0, 0, 0);
					rtn = FAULT_COND;
					break;
				}
			}
			#endif
			#ifdef _SDI_SAFETY_V2	
			if(myCh->misc.MasterFlag == 1 || myCh->misc.MasterFlag == 3){
				if(myPs->testCond[bd][ch].safety.Master_Recipe_V != 0 &&
							myCh->misc.deltaV_cnt == 0){
					if(myCh->misc.Master_recipe_deltaV - myCh->op.Vsens >= 
						myPs->testCond[bd][ch].safety.Master_Recipe_V){
						myCh->misc.deltaV_cnt = 1;
					}
				}
				if(myCh->misc.deltaV_cnt == 0 && myCh->op.runTime >= 100){
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_CABLE_LINE_CHECK;
						rtn = FAULT_COND;
						break;
				}
			}
			
			#endif
			#if CHANGE_VI_CHECK == 1
			if(myTestCond->step[advStepNo].faultRunTime != 0) {
				if(myCh->op.runTime >= myTestCond->step[advStepNo].faultRunTime){
					myCh->misc.tmpState = myCh->op.state;
					myCh->misc.tmpCode = myCh->op.code;
					myCh->op.code = C_FAULT_RUN_TIME;
					rtn = FAULT_COND;
					break;
				}
			}
			#endif
			#ifdef _ULSAN_SDI_SAFETY
			if(myTestCond->step[advStepNo].humpSet_T != 0 
				&& myTestCond->step[advStepNo].humpSet_I != 0){
				if(myCh->misc.cvFlag == P1){
					if(myCh->misc.cvTime_Ulsan <= 100){
						myCh->misc.humpComp_T = myCh->op.runTime;
						myCh->misc.humpComp_I = myCh->op.Isens;

						myCh->misc.humpCheck_T = myCh->misc.humpComp_T 
							+ myTestCond->step[advStepNo].humpSet_T;
					}
					if(myCh->misc.cvTime_Ulsan > 100){
						if(myCh->op.runTime >= myCh->misc.humpCheck_T){
							myCh->misc.humpCheck_I
								= myCh->misc.humpComp_I - myCh->op.Isens;

							myCh->misc.humpComp_T = myCh->op.runTime;
							myCh->misc.humpComp_I = myCh->op.Isens;

							myCh->misc.humpCheck_T = myCh->misc.humpComp_T 
								+ myTestCond->step[advStepNo].humpSet_T;

							if(myCh->misc.humpCheck_I 
								>= myTestCond->step[advStepNo].humpSet_I){
								myCh->misc.tmpState = myCh->op.state;
								myCh->misc.tmpCode = myCh->op.code;
								myCh->misc.cvFlag = 0;
								myCh->misc.cvTime_Ulsan = 0;
								myCh->op.code = C_FAULT_CV_CURRENT_HUMP;
								rtn = FAULT_COND;
								break;
							}
						}
					}
				}
			}
			#endif
			//hun_211228_e 병렬 부분 누락으로 인해 추가

			if(myPs->testCond[bd][ch].safety.faultUpperV != 0) {
				if(myCh->op.Vsens
					>= myPs->testCond[bd][ch].safety.faultUpperV) {
					myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE]
						>= MAX_ERROR_CNT) {
						myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_VOLTAGE;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); //safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Vsens,
						myPs->testCond[bd][ch].safety.faultUpperV,
					   	0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				} else {
					myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE] = 0;
				}
			}

			if(myPs->testCond[bd][ch].safety.faultLowerV != 0) {
				if(myCh->op.Vsens
					<= myPs->testCond[bd][ch].safety.faultLowerV) {
					myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE]++;
					if(myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE]
						>= MAX_ERROR_CNT) {
						myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_LOWER_VOLTAGE;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); //safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Vsens,
						myPs->testCond[bd][ch].safety.faultLowerV,
					   	0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				} else {
					myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE] = 0;
				}
			}

			if(myCh->op.checkDelayTime < 180) break;

			if(faultUpperI_com != 0) {
				if(myCh->op.Isens <= faultUpperI_com * (-1)) {
					myCh->misc.errCnt[C_CNT_UPPER_CURRENT]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_CURRENT]
						>= MAX_ERROR_CNT) {
						myCh->misc.errCnt[C_CNT_UPPER_CURRENT] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_CURRENT;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); //safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Isens,
						faultUpperI_com * (-1), 0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				} else {
					myCh->misc.errCnt[C_CNT_UPPER_CURRENT] = 0;
				}
			}

			if(faultLowerI_com != 0) {
				if(myCh->op.Isens >= faultLowerI_com * (-1)) {
					myCh->misc.errCnt[C_CNT_LOWER_CURRENT]++;
					if(myCh->misc.errCnt[C_CNT_LOWER_CURRENT]
						>= MAX_ERROR_CNT) {
						myCh->misc.errCnt[C_CNT_LOWER_CURRENT] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_LOWER_CURRENT;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); //safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Isens,
						faultLowerI_com * (-1), 0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				} else {
					myCh->misc.errCnt[C_CNT_LOWER_CURRENT] = 0;
				}
			}

			if(myPs->testCond[bd][ch].safety.faultUpperC != 0) {
				if(capacity >= myPs->testCond[bd][ch].safety.faultUpperC) {
					myCh->misc.errCnt[C_CNT_UPPER_CAPACITY]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_CAPACITY]
						>= MAX_ERROR_CNT) {
						myCh->misc.errCnt[C_CNT_UPPER_CAPACITY] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_CAPACITY;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); //safety
						Fault_Value_Check(myCh->misc.advStepNo, capacity,
						myPs->testCond[bd][ch].safety.faultUpperC,
					   	0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				} else {
					myCh->misc.errCnt[C_CNT_UPPER_CAPACITY] = 0;
				}
			}

			if(myPs->testCond[bd][ch].step[advStepNo].faultUpperV != 0) {
				if(myCh->op.Vsens >=
					myPs->testCond[bd][ch].step[advStepNo].faultUpperV) {
					myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE_STEP]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE_STEP]
						>= MAX_ERROR_CNT) {
						myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE_STEP] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_VOLTAGE;
						myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); //step_safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Vsens,
						myPs->testCond[bd][ch].step[advStepNo].faultUpperV,
					   	0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				} else {
					myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE_STEP] = 0;
				}
			}

			if(myPs->testCond[bd][ch].step[advStepNo].faultLowerV != 0) {
				if(myCh->op.Vsens <=
					myPs->testCond[bd][ch].step[advStepNo].faultLowerV) {
					myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE_STEP]++;
					if(myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE_STEP]
						>= MAX_ERROR_CNT) {
						myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE_STEP] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_LOWER_VOLTAGE;
						myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); //step_safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Vsens,
						myPs->testCond[bd][ch].step[advStepNo].faultLowerV,
					   	0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				} else {
					myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE_STEP] = 0;
				}
			}

			if(faultUpperI_step != 0) {
				if(myCh->op.Isens <= faultUpperI_step) {
					myCh->misc.errCnt[C_CNT_UPPER_CURRENT_STEP]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_CURRENT_STEP]
						>= MAX_ERROR_CNT) {
						myCh->misc.errCnt[C_CNT_UPPER_CURRENT_STEP] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_CURRENT;
						myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); //step_safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Isens,
						faultUpperI_step, 0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				} else {
					myCh->misc.errCnt[C_CNT_UPPER_CURRENT_STEP] = 0;
				}
			}

			if((myCh->op.Vsens - refV) > (maxV * 0.001)){		//kjc_200612
				if(faultLowerI_step != 0) {
					if(myCh->op.Isens >= faultLowerI_step) {
						myCh->misc.errCnt[C_CNT_LOWER_CURRENT_STEP]++;
						if(myCh->misc.errCnt[C_CNT_LOWER_CURRENT_STEP]
							>= MAX_ERROR_CNT) {
							myCh->misc.errCnt[C_CNT_LOWER_CURRENT_STEP] = 0;
							myCh->misc.tmpState = myCh->op.state;
							myCh->misc.tmpCode = myCh->op.code;
							myCh->op.code = C_FAULT_LOWER_CURRENT;
							myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); //step_safety
							Fault_Value_Check(myCh->misc.advStepNo, 
							myCh->op.Isens, faultLowerI_step,
						   	0, 0, 0, 0, 0, 0, 0);
							rtn = FAULT_COND;
							break;
						}
					} else {
						myCh->misc.errCnt[C_CNT_LOWER_CURRENT_STEP] = 0;
					}
				}
			}

			if(myPs->testCond[bd][ch].step[advStepNo].faultUpperC != 0) {
				if(capacity >=
					myPs->testCond[bd][ch].step[advStepNo].faultUpperC) {
					myCh->misc.errCnt[C_CNT_UPPER_CAPACITY_STEP]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_CAPACITY_STEP]
						>= MAX_ERROR_CNT) {
						myCh->misc.errCnt[C_CNT_UPPER_CAPACITY_STEP] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_CAPACITY;
						myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); //step_safety
						Fault_Value_Check(myCh->misc.advStepNo, capacity,
						myPs->testCond[bd][ch].step[advStepNo].faultUpperC,
					   	0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				} else {
					myCh->misc.errCnt[C_CNT_UPPER_CAPACITY_STEP] = 0;
				}
			}

			//170215 SCH add for DeltaV/I DISCHARGE
			if(myPs->testCond[bd][ch].step[advStepNo].faultDeltaV != 0) {
				if(myData->mData.config.function[F_DELTA_V_I] != 0) {
					if(labs(refI - myCh->op.Isens) < (maxI * 0.001)) {
						if(labs(myCh->op.Vsens - myCh->misc.minV )
							>= myPs->testCond[bd][ch].step[advStepNo]
							.faultDeltaV) {
							myCh->misc.errCnt[C_CNT_DELTA_V_STEP]++;
							if(myCh->misc.errCnt[C_CNT_DELTA_V_STEP]
								> MAX_ERROR_CNT) {
								myCh->misc.errCnt[C_CNT_DELTA_V_STEP] = 0;
								myCh->misc.tmpState = myCh->op.state;
								myCh->misc.tmpCode = myCh->op.code;
								if(myData->mData.config.function[F_DELTA_V_I]
									== 1) {
									myCh->op.code = C_STOP_DELTA_V;
									rtn = FAULT_COND;
								} else if(myData->mData.config
									.function[F_DELTA_V_I] == 2) {
									myCh->op.code = C_FAULT_DELTA_V;
									rtn = FAULT_COND;
								}
								break;
							}
						} else {
							myCh->misc.errCnt[C_CNT_DELTA_V_STEP] = 0;
						}
					}
				}
			}

			if(myPs->testCond[bd][ch].step[advStepNo].faultDeltaI != 0) {
				if(myData->mData.config.function[F_DELTA_V_I] != 0) {
					if(labs(refV - myCh->op.Vsens) < (maxV * 0.001)) {
						if(labs(myCh->op.Isens - myCh->misc.minI)
							>= myPs->testCond[bd][ch].step[advStepNo]
							.faultDeltaI) {
							myCh->misc.errCnt[C_CNT_DELTA_V_STEP]++;
							if(myCh->misc.errCnt[C_CNT_DELTA_V_STEP]
								> MAX_ERROR_CNT) {
								myCh->misc.errCnt[C_CNT_DELTA_I_STEP] = 0;
								myCh->misc.tmpState = myCh->op.state;
								myCh->misc.tmpCode = myCh->op.code;
								if(myData->mData.config.function[F_DELTA_V_I]
									== 1) {
									myCh->op.code = C_STOP_DELTA_I;
									rtn = FAULT_COND;
								} else if(myData->mData.config
									.function[F_DELTA_V_I] == 2) {
									myCh->op.code = C_FAULT_DELTA_I;
									rtn = FAULT_COND;
								}
								break;
							}
						} else {
							myCh->misc.errCnt[C_CNT_DELTA_V_STEP] = 0;
						}
					}
				}
			}
			break;
		case STEP_Z:
			//100908 pjy temp fault check without delay
			if(myPs->testCond[bd][ch].safety.faultUpperTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 0);
				if(rtn == FAULT_COND) break;
			}
			if(myPs->testCond[bd][ch].safety.faultLowerTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 1);
				if(rtn == FAULT_COND) break;
			}

			if(myPs->testCond[bd][ch].step[advStepNo].faultUpperTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 2);
				if(rtn == FAULT_COND) break;
			}
			if(myPs->testCond[bd][ch].step[advStepNo].faultLowerTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 3);
				if(rtn == FAULT_COND) break;
			}
			//190311 lyh add
			if(myTestCond->step[advStepNo].pauseUpperTemp != 0) {
				rtn = tempPauseCheck(bd, ch, advStepNo, 0);
				if(rtn == FAULT_COND)
					break;
			}
			if(myTestCond->step[advStepNo].pauseLowerTemp != 0) {
				rtn = tempPauseCheck(bd, ch, advStepNo, 1);
				if(rtn == FAULT_COND)
					break;
			}

			if(myPs->testCond[bd][ch].safety.faultUpperV != 0) {
				if(myCh->op.Vsens
					>= myPs->testCond[bd][ch].safety.faultUpperV) {
					myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE]
						>= MAX_ERROR_CNT) {
						myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_VOLTAGE;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); //safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Vsens,
						myPs->testCond[bd][ch].safety.faultUpperV,
					   	0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				} else {
					myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE] = 0;
				}
			}

			if(myPs->testCond[bd][ch].safety.faultLowerV != 0) {
				if(myCh->op.Vsens
					<= myPs->testCond[bd][ch].safety.faultLowerV) {
					myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE]++;
					if(myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE]
						>= MAX_ERROR_CNT) {
						myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_LOWER_VOLTAGE;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); //safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Vsens,
						myPs->testCond[bd][ch].safety.faultLowerV,
					   	0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				} else {
					myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE] = 0;
				}
			}

			if(myCh->op.checkDelayTime < 180) break;

			if(faultUpperI_com != 0) {
				if(myCh->op.Isens <= faultUpperI_com * (-1)) {
					myCh->misc.errCnt[C_CNT_UPPER_CURRENT]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_CURRENT]
						>= MAX_ERROR_CNT) {
						myCh->misc.errCnt[C_CNT_UPPER_CURRENT] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_CURRENT;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); //safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Isens,
						faultUpperI_com * (-1), 0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				} else {
					myCh->misc.errCnt[C_CNT_UPPER_CURRENT] = 0;
				}
			}

			if(faultLowerI_com != 0) {
				if(myCh->op.Isens >= faultLowerI_com * (-1)) {
					myCh->misc.errCnt[C_CNT_LOWER_CURRENT]++;
					if(myCh->misc.errCnt[C_CNT_LOWER_CURRENT]
						>= MAX_ERROR_CNT) {
						myCh->misc.errCnt[C_CNT_LOWER_CURRENT] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_LOWER_CURRENT;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); //safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Isens,
						faultLowerI_com * (-1), 0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				} else {
					myCh->misc.errCnt[C_CNT_LOWER_CURRENT] = 0;
				}
			}

			if(myPs->testCond[bd][ch].step[advStepNo].faultUpperV != 0) {
				if(myCh->op.Vsens >=
					myPs->testCond[bd][ch].step[advStepNo].faultUpperV) {
					myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE_STEP]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE_STEP]
						>= MAX_ERROR_CNT) {
						myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE_STEP] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_VOLTAGE;
						myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); //step_safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Vsens,
						myPs->testCond[bd][ch].step[advStepNo].faultUpperV,
					   	0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				} else {
					myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE_STEP] = 0;
				}
			}

			if(myPs->testCond[bd][ch].step[advStepNo].faultLowerV != 0) {
				if(myCh->op.Vsens <=
					myPs->testCond[bd][ch].step[advStepNo].faultLowerV) {
					myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE_STEP]++;
					if(myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE_STEP]
						>= MAX_ERROR_CNT) {
						myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE_STEP] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_LOWER_VOLTAGE;
						myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); //step_safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Vsens,
						myPs->testCond[bd][ch].step[advStepNo].faultLowerV,
					   	0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				} else {
					myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE_STEP] = 0;
				}
			}

			if(faultUpperI_step != 0) {
				if(myCh->op.Isens <= faultUpperI_step) {
					myCh->misc.errCnt[C_CNT_UPPER_CURRENT_STEP]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_CURRENT_STEP]
						>= MAX_ERROR_CNT) {
						myCh->misc.errCnt[C_CNT_UPPER_CURRENT_STEP] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_CURRENT;
						myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); //step_safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Isens,
						faultUpperI_step, 0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				} else {
					myCh->misc.errCnt[C_CNT_UPPER_CURRENT_STEP] = 0;
				}
			}

			if((myCh->op.Vsens - refV) > (maxV * 0.001)){		//kjc_200612
				if(faultLowerI_step != 0) {
					if(myCh->op.Isens >= faultLowerI_step) {
						myCh->misc.errCnt[C_CNT_LOWER_CURRENT_STEP]++;
						if(myCh->misc.errCnt[C_CNT_LOWER_CURRENT_STEP]
							>= MAX_ERROR_CNT) {
							myCh->misc.errCnt[C_CNT_LOWER_CURRENT_STEP] = 0;
							myCh->misc.tmpState = myCh->op.state;
							myCh->misc.tmpCode = myCh->op.code;
							myCh->op.code = C_FAULT_LOWER_CURRENT;
							myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); //step_safety
							Fault_Value_Check(myCh->misc.advStepNo, 
							myCh->op.Isens, faultLowerI_step,
						   	0, 0, 0, 0, 0, 0, 0);
							rtn = FAULT_COND;
							break;
						}
					} else {
						myCh->misc.errCnt[C_CNT_LOWER_CURRENT_STEP] = 0;
					}
				}
			}

			//20180314 sch modify
			if(myPs->config.LimitVI.limit_use_I != 0 
				|| myPs->config.LimitVI.limit_use_V != 0) {
				rtn = Fault_Check_default_LG(bd, ch);
				if(rtn == FAULT_COND)
					break;
			}
			break;
		case STEP_USER_PATTERN:
			//100908 pjy temp fault check without delay
			if(myPs->testCond[bd][ch].safety.faultUpperTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 0);
				if(rtn == FAULT_COND) break;
			}
			if(myPs->testCond[bd][ch].safety.faultLowerTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 1);
				if(rtn == FAULT_COND) break;
			}

			if(myPs->testCond[bd][ch].step[advStepNo].faultUpperTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 2);
				if(rtn == FAULT_COND) break;
			}
			if(myPs->testCond[bd][ch].step[advStepNo].faultLowerTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 3);
				if(rtn == FAULT_COND) break;
			}
			//190311 lyh add
			if(myTestCond->step[advStepNo].pauseUpperTemp != 0) {
				rtn = tempPauseCheck(bd, ch, advStepNo, 0);
				if(rtn == FAULT_COND)
					break;
			}
			if(myTestCond->step[advStepNo].pauseLowerTemp != 0) {
				rtn = tempPauseCheck(bd, ch, advStepNo, 1);
				if(rtn == FAULT_COND)
					break;
			}

			if(myPs->testCond[bd][ch].safety.faultUpperV != 0) {
				if(myCh->op.Vsens
					>= myPs->testCond[bd][ch].safety.faultUpperV) {
					myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE]
						>= MAX_ERROR_CNT) {
						myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_VOLTAGE;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); //safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Vsens,
						myPs->testCond[bd][ch].safety.faultUpperV,
					   	0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				} else {
					myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE] = 0;
				}
			}

			if(myPs->testCond[bd][ch].safety.faultLowerV != 0) {
				if(myCh->op.Vsens
					<= myPs->testCond[bd][ch].safety.faultLowerV) {
					myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE]++;
					if(myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE]
						>= MAX_ERROR_CNT) {
						myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_LOWER_VOLTAGE;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); //safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Vsens,
						myPs->testCond[bd][ch].safety.faultLowerV,
					   	0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				} else {
					myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE] = 0;
				}
			}

			if(myCh->op.checkDelayTime < 180) break;

			if(faultUpperI_com != 0) {
				if(myCh->op.Isens >= faultUpperI_com) {
					myCh->misc.errCnt[C_CNT_UPPER_CURRENT]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_CURRENT]
						>= MAX_ERROR_CNT) {
						myCh->misc.errCnt[C_CNT_UPPER_CURRENT] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_CURRENT;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); //safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Isens,
						faultUpperI_com, 0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				} else {
					myCh->misc.errCnt[C_CNT_UPPER_CURRENT] = 0;
				}
			}

			if(faultLowerI_com != 0) {
				if(myCh->op.Isens <= faultLowerI_com) {
					myCh->misc.errCnt[C_CNT_LOWER_CURRENT]++;
					if(myCh->misc.errCnt[C_CNT_LOWER_CURRENT]
						>= MAX_ERROR_CNT) {
						myCh->misc.errCnt[C_CNT_LOWER_CURRENT] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_LOWER_CURRENT;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); //safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Isens,
						faultLowerI_com, 0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				} else {
					myCh->misc.errCnt[C_CNT_LOWER_CURRENT] = 0;
				}
			}

			if(myPs->testCond[bd][ch].safety.faultUpperC != 0) {
				if(capacity >= myPs->testCond[bd][ch].safety.faultUpperC) {
					myCh->misc.errCnt[C_CNT_UPPER_CAPACITY]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_CAPACITY]
						>= MAX_ERROR_CNT) {
						myCh->misc.errCnt[C_CNT_UPPER_CAPACITY] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_CAPACITY;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); //safety
						Fault_Value_Check(myCh->misc.advStepNo, capacity,
						myPs->testCond[bd][ch].safety.faultUpperC,
					   	0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				} else {
					myCh->misc.errCnt[C_CNT_UPPER_CAPACITY] = 0;
				}
			}

			if(myPs->testCond[bd][ch].step[advStepNo].faultUpperV != 0) {
				if(myCh->op.Vsens >=
					myPs->testCond[bd][ch].step[advStepNo].faultUpperV) {
					myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE_STEP]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE_STEP]
						>= MAX_ERROR_CNT) {
						myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE_STEP] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_VOLTAGE;
						myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); //step_safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Vsens,
						myPs->testCond[bd][ch].step[advStepNo].faultUpperV,
					   	0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				} else {
					myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE_STEP] = 0;
				}
			}

			if(myPs->testCond[bd][ch].step[advStepNo].faultLowerV != 0) {
				if(myCh->op.Vsens <=
					myPs->testCond[bd][ch].step[advStepNo].faultLowerV) {
					myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE_STEP]++;
					if(myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE_STEP]
						>= MAX_ERROR_CNT) {
						myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE_STEP] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_LOWER_VOLTAGE;
						myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); //step_safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Vsens,
						myPs->testCond[bd][ch].step[advStepNo].faultLowerV,
					   	0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				} else {
					myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE_STEP] = 0;
				}
			}

			if(faultUpperI_step != 0) {
				if(myCh->op.Isens >= faultUpperI_step) {
					myCh->misc.errCnt[C_CNT_UPPER_CURRENT_STEP]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_CURRENT_STEP]
						>= MAX_ERROR_CNT) {
						myCh->misc.errCnt[C_CNT_UPPER_CURRENT_STEP] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_CURRENT;
						myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); //step_safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Isens,
						faultUpperI_step, 0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				} else {
					myCh->misc.errCnt[C_CNT_UPPER_CURRENT_STEP] = 0;
				}
			}

			if(faultLowerI_step != 0) {
				if(myCh->op.Isens <= faultLowerI_step) {
					myCh->misc.errCnt[C_CNT_LOWER_CURRENT_STEP]++;
					if(myCh->misc.errCnt[C_CNT_LOWER_CURRENT_STEP]
						>= MAX_ERROR_CNT) {
						myCh->misc.errCnt[C_CNT_LOWER_CURRENT_STEP] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_LOWER_CURRENT;
						myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); //step_safety
						Fault_Value_Check(myCh->misc.advStepNo, 
						myCh->op.Isens, faultLowerI_step, 0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				} else {
					myCh->misc.errCnt[C_CNT_LOWER_CURRENT_STEP] = 0;
				}
			}

			if(myPs->testCond[bd][ch].step[advStepNo].faultUpperC != 0) {
				if(capacity
					>= myPs->testCond[bd][ch].step[advStepNo].faultUpperC) {
					myCh->misc.errCnt[C_CNT_UPPER_CAPACITY_STEP]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_CAPACITY_STEP]
						>= MAX_ERROR_CNT) {
						myCh->misc.errCnt[C_CNT_UPPER_CAPACITY_STEP] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_CAPACITY;
						myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); //step_safety
						Fault_Value_Check(myCh->misc.advStepNo, capacity,
						myPs->testCond[bd][ch].step[advStepNo].faultUpperC,
					   	0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				} else {
					myCh->misc.errCnt[C_CNT_UPPER_CAPACITY_STEP] = 0;
				}
			}
			//hun_211228_s 병렬 부분 누락으로 인해 추가
			#if CHANGE_VI_CHECK == 1
			if(myTestCond->step[advStepNo].faultRunTime != 0) {
				if(myCh->op.runTime >= myTestCond->step[advStepNo].faultRunTime){
					myCh->misc.tmpState = myCh->op.state;
					myCh->misc.tmpCode = myCh->op.code;
					myCh->op.code = C_FAULT_RUN_TIME;
					rtn = FAULT_COND;
					break;
				}
			}
			#endif

			//20180314 sch modify
			if(myPs->config.LimitVI.limit_use_I != 0 
				|| myPs->config.LimitVI.limit_use_V != 0) {
				rtn = Fault_Check_default_LG(bd, ch);
				if(rtn == FAULT_COND) break;
			}
			break;
		case STEP_USER_MAP:
			//100908 pjy temp fault check without delay
			if(myPs->testCond[bd][ch].safety.faultUpperTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 0);
				if(rtn == FAULT_COND) break;
			}
			if(myPs->testCond[bd][ch].safety.faultLowerTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 1);
				if(rtn == FAULT_COND) break;
			}

			if(myPs->testCond[bd][ch].step[advStepNo].faultUpperTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 2);
				if(rtn == FAULT_COND) break;
			}
			if(myPs->testCond[bd][ch].step[advStepNo].faultLowerTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 3);
				if(rtn == FAULT_COND) break;
			}
			//190311 lyh add
			if(myTestCond->step[advStepNo].pauseUpperTemp != 0) {
				rtn = tempPauseCheck(bd, ch, advStepNo, 0);
				if(rtn == FAULT_COND)
					break;
			}
			if(myTestCond->step[advStepNo].pauseLowerTemp != 0) {
				rtn = tempPauseCheck(bd, ch, advStepNo, 1);
				if(rtn == FAULT_COND)
					break;
			}

			if(myPs->testCond[bd][ch].safety.faultUpperV != 0) {
				if(myCh->op.Vsens
					>= myPs->testCond[bd][ch].safety.faultUpperV) {
					myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE]
						>= MAX_ERROR_CNT) {
						myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_VOLTAGE;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); //safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Vsens,
						myPs->testCond[bd][ch].safety.faultUpperV,
					   	0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				} else {
					myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE] = 0;
				}
			}

			if(myPs->testCond[bd][ch].safety.faultLowerV != 0) {
				if(myCh->op.Vsens
					<= myPs->testCond[bd][ch].safety.faultLowerV) {
					myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE]++;
					if(myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE]
						>= MAX_ERROR_CNT) {
						myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_LOWER_VOLTAGE;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); //safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Vsens,
						myPs->testCond[bd][ch].safety.faultLowerV,
					   	0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				} else {
					myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE] = 0;
				}
			}

			if(myCh->op.checkDelayTime < 180) break;

			if(faultUpperI_com != 0) {
				if(myCh->op.Isens >= faultUpperI_com) {
					myCh->misc.errCnt[C_CNT_UPPER_CURRENT]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_CURRENT]
						>= MAX_ERROR_CNT) {
						myCh->misc.errCnt[C_CNT_UPPER_CURRENT] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_CURRENT;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); //safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Isens,
						faultUpperI_com, 0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				} else {
					myCh->misc.errCnt[C_CNT_UPPER_CURRENT] = 0;
				}
			}

			if(faultLowerI_com != 0) {
				if(myCh->op.Isens <= faultLowerI_com) {
					myCh->misc.errCnt[C_CNT_LOWER_CURRENT]++;
					if(myCh->misc.errCnt[C_CNT_LOWER_CURRENT]
						>= MAX_ERROR_CNT) {
						myCh->misc.errCnt[C_CNT_LOWER_CURRENT] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_LOWER_CURRENT;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); //safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Isens,
						faultLowerI_com, 0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				} else {
					myCh->misc.errCnt[C_CNT_LOWER_CURRENT] = 0;
				}
			}

			if(myPs->testCond[bd][ch].safety.faultUpperC != 0) {
				if(labs(capacity)
					>= myPs->testCond[bd][ch].safety.faultUpperC) {
					myCh->misc.errCnt[C_CNT_UPPER_CAPACITY]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_CAPACITY]
						>= MAX_ERROR_CNT) {
						myCh->misc.errCnt[C_CNT_UPPER_CAPACITY] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_CAPACITY;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); //safety
						Fault_Value_Check(myCh->misc.advStepNo, capacity,
						myPs->testCond[bd][ch].safety.faultUpperC,
					   	0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				} else {
					myCh->misc.errCnt[C_CNT_UPPER_CAPACITY] = 0;
				}
			}

			if(myPs->testCond[bd][ch].step[advStepNo].faultUpperV != 0) {
				if(myCh->op.Vsens
					>= myPs->testCond[bd][ch].step[advStepNo].faultUpperV) {
					myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE_STEP]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE_STEP]
						>= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE_STEP] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_VOLTAGE;
						myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); //step_safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Vsens,
						myPs->testCond[bd][ch].step[advStepNo].faultUpperV,
					   	0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				} else {
					myCh->misc.errCnt[C_CNT_UPPER_VOLTAGE_STEP] = 0;
				}
			}

			if(myPs->testCond[bd][ch].step[advStepNo].faultLowerV != 0) {
				if(myCh->op.Vsens
					<= myPs->testCond[bd][ch].step[advStepNo].faultLowerV) {
					myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE_STEP]++;
					if(myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE_STEP]
						>= MAX_ERROR_CNT) {
						myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE_STEP] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_LOWER_VOLTAGE;
						myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); //step_safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Vsens,
						myPs->testCond[bd][ch].step[advStepNo].faultLowerV,
					   	0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				} else {
					myCh->misc.errCnt[C_CNT_LOWER_VOLTAGE_STEP] = 0;
				}
			}

			if(faultUpperI_step != 0) {
				if(myCh->op.Isens >= faultUpperI_step) {
					myCh->misc.errCnt[C_CNT_UPPER_CURRENT_STEP]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_CURRENT_STEP]
						>= MAX_ERROR_CNT) {
						myCh->misc.errCnt[C_CNT_UPPER_CURRENT_STEP] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_CURRENT;
						myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); //step_safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.Isens,
						faultUpperI_step, 0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				} else {
					myCh->misc.errCnt[C_CNT_UPPER_CURRENT_STEP] = 0;
				}
			}

			if(faultLowerI_step != 0) {
				if(myCh->op.Isens <= faultLowerI_step) {
					myCh->misc.errCnt[C_CNT_LOWER_CURRENT_STEP]++;
					if(myCh->misc.errCnt[C_CNT_LOWER_CURRENT_STEP]
						>= MAX_ERROR_CNT) {
						myCh->misc.errCnt[C_CNT_LOWER_CURRENT_STEP] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_LOWER_CURRENT;
						myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); //step_safety
						Fault_Value_Check(myCh->misc.advStepNo, 
						myCh->op.Isens, faultLowerI_step, 0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				} else {
					myCh->misc.errCnt[C_CNT_LOWER_CURRENT_STEP] = 0;
				}
			}

			if(myPs->testCond[bd][ch].step[advStepNo].faultUpperC != 0) {
				if(labs(capacity)
					>= myPs->testCond[bd][ch].step[advStepNo].faultUpperC) {
					myCh->misc.errCnt[C_CNT_UPPER_CAPACITY_STEP]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_CAPACITY_STEP]
						>= MAX_ERROR_CNT) {
						myCh->misc.errCnt[C_CNT_UPPER_CAPACITY_STEP] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_CAPACITY;
						myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); //step_safety
						Fault_Value_Check(myCh->misc.advStepNo, capacity,
						myPs->testCond[bd][ch].step[advStepNo].faultUpperC,
					   	0, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
						break;
					}
				} else {
					myCh->misc.errCnt[C_CNT_UPPER_CAPACITY_STEP] = 0;
				}
			}

			//20180314 sch modify
			if(myPs->config.LimitVI.limit_use_I != 0 
				|| myPs->config.LimitVI.limit_use_V != 0) {
				rtn = Fault_Check_default_LG(bd, ch);
				if(rtn == FAULT_COND) break;
			}
			break;
#if MACHINE_TYPE == 1
		case STEP_SHORT:
			if(myPs->testCond[bd][ch].safety.faultUpperTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 0);
				if(rtn == FAULT_COND) break;
			}
			if(myPs->testCond[bd][ch].safety.faultLowerTemp != 0) {
				rtn = tempFaultCheck(bd, ch, advStepNo, 1);
				if(rtn == FAULT_COND) break;
			}
			break;
#endif
		default: 
			break;
	}
	//hun_211228_s 병렬 부분 누락으로 인해 추가
	//gasVoltage [uV]
	//GAS_VOLTAGE_MIN [uV] GA_VOLTAGE_MAX [uV]
	//GAS_CHECK_TIME [sec]
	if(rtn != FAULT_COND && myData->mData.config.swFaultConfig[CHAMBER_GAS_VOLTAGE_MIN] != 0){
		if(myCh->misc.gasVoltage < myData->mData.config.swFaultConfig[CHAMBER_GAS_VOLTAGE_MIN]) {
			if(myCh->misc.gas_check_flag == 0){
				myCh->misc.gas_check_time = myCh->op.runTime;	
				myCh->misc.gas_check_flag = 1;
			}else if(myCh->misc.gas_check_flag == 1){
				if(labs(myCh->misc.gas_check_time - myCh->op.runTime) >
					myData->mData.config.swFaultConfig[GAS_CHECK_TIME] * 100){
					myCh->misc.tmpState = myCh->op.state;
					myCh->misc.tmpCode = myCh->op.code;
					myCh->op.code = C_FAULT_GAS_VOLTAGE_MIN;
					rtn = FAULT_COND;
				}
			}
		}else if(myCh->misc.gasVoltage > myData->mData.config.swFaultConfig[CHAMBER_GAS_VOLTAGE_MIN]) {
			myCh->misc.gas_check_flag = 0;
			myCh->misc.gas_check_time = myCh->op.runTime;	
		}
	}
	if(rtn != FAULT_COND && myData->mData.config.swFaultConfig[CHAMBER_GAS_VOLTAGE_MAX] != 0){
		if(myCh->misc.gasVoltage > myData->mData.config.swFaultConfig[CHAMBER_GAS_VOLTAGE_MAX]) { 
			if(myCh->misc.gas_check_flag == 0){
				myCh->misc.gas_check_time = myCh->op.runTime;	
				myCh->misc.gas_check_flag = 1;
			}else if(myCh->misc.gas_check_flag == 1){
				if(labs(myCh->misc.gas_check_time - myCh->op.runTime) >
					myData->mData.config.swFaultConfig[GAS_CHECK_TIME] * 100){
					myCh->misc.tmpState = myCh->op.state;
					myCh->misc.tmpCode = myCh->op.code;
					myCh->op.code = C_FAULT_GAS_VOLTAGE_MAX;
					rtn = FAULT_COND;
				}
			}
		}else if(myCh->misc.gasVoltage < myData->mData.config.swFaultConfig[CHAMBER_GAS_VOLTAGE_MAX]) {
			myCh->misc.gas_check_flag = 0;
			myCh->misc.gas_check_time = myCh->op.runTime;	
		}
	}
	if(rtn != FAULT_COND && myData->mData.config.swFaultConfig[SOFT_VENTING_COUNT] != 0){
		if(myCh->misc.gasVoltage - myCh->misc.std_gasVoltage >= myData->mData.config.swFaultConfig[SOFT_VENTING_VALUE]){
			myCh->misc.soft_venting_count++;
			myCh->misc.std_gasVoltage = myCh->misc.gasVoltage;
		}else{
			myCh->misc.soft_venting_count = 0;
		}
		if(myCh->misc.soft_venting_count >= myData->mData.config.swFaultConfig[SOFT_VENTING_COUNT]){
			myCh->misc.tmpState = myCh->op.state;
			myCh->misc.tmpCode = myCh->op.code;
			myCh->op.code = C_FAULT_SOFT_VENTING;
			rtn = FAULT_COND;
		}
	}
	if(rtn != FAULT_COND && myData->mData.config.swFaultConfig[HARD_VENTING_VALUE] != 0){
		if(myCh->misc.gasVoltage - myCh->misc.std_gasVoltage >= myData->mData.config.swFaultConfig[HARD_VENTING_VALUE]){
			myCh->misc.tmpState = myCh->op.state;
			myCh->misc.tmpCode = myCh->op.code;
			myCh->op.code = C_FAULT_HARD_VENTING;
			rtn = FAULT_COND;
		}
	}

	if(rtn != FAULT_COND && myData->mData.config.swFaultConfig[AMBIENT_TEMP_MAX] != 0){
		if(myCh->misc.ambientTemp > myData->mData.config.swFaultConfig[AMBIENT_TEMP_MAX]) { 
			if(myCh->misc.ambient_check_flag == 0){
				myCh->misc.ambient_check_time = myCh->op.runTime;	
				myCh->misc.ambient_check_flag = 1;
			}else if(myCh->misc.ambient_check_flag == 1){
				if(labs(myCh->misc.ambient_check_time - myCh->op.runTime) >
					myData->mData.config.swFaultConfig[AMBIENT_TEMP_MAX_TIME] * 100){
					myCh->misc.tmpState = myCh->op.state;
					myCh->misc.tmpCode = myCh->op.code;
					myCh->op.code = C_FAULT_AMBIENT_TEMP_MAX;
					rtn = FAULT_COND;
				}
			}
		}else if(myCh->misc.ambientTemp < myData->mData.config.swFaultConfig[AMBIENT_TEMP_MAX]) {
			myCh->misc.ambient_check_flag = 0;
			myCh->misc.ambient_check_time = myCh->op.runTime;	
		}
	}
	
	if(myData->bData[bd].cData[ch].misc.Fault_flag != 0){
		myData->bData[bd].cData[ch].misc.Fault_flag = 0;
		switch(myCh->misc.ch_fault_code){
			case C_FAULT_MASTER_RECIPE_VOLTAGE_ERR://SDI
			case C_FAULT_MASTER_RECIPE_CURRENT_ERR://SDI
			case C_FAULT_MASTER_RECIPE_TIME_ERR://SDI
			case C_FAULT_MASTER_RECIPE_TEMP_ERR://SDI
			case C_FAULT_MASTER_RECIPE_GROUP_ERR://SDI
			case C_FAULT_GUI_AMBIENT_TEMP_ERROR://LGES
			case C_FAULT_GUI_UPPER_VOLTAGE://LGES
			case C_FAULT_GUI_UPPER_TEMP://LGES
			case C_FAULT_GUI_LOWER_VOLTAGE://LGES
			case C_FAULT_GUI_CHAMBER_DI://LGES
				myCh->misc.tmpState = myCh->op.state;
				myCh->misc.tmpCode = myCh->op.code;
				myCh->op.code = myCh->misc.ch_fault_code;
				rtn = FAULT_COND;
				break;
			default:	//SDI
				myCh->misc.tmpState = myCh->op.state;
				myCh->misc.tmpCode = myCh->op.code;
				myCh->op.code = myCh->misc.ch_fault_code;
				rtn = FAULT_COND;
				break;
		}	
	}
	//hun_211228_e 병렬 부분 누락으로 인해 추가

	//20171024
#ifdef _USER_VI
	if(myData->mData.config.LimitVI.limit_use_I != 0) {
		if(myCh->signal[C_SIG_LIMIT_CURRENT] == P1) { 
			myCh->signal[C_SIG_LIMIT_CURRENT] = P0;
			myCh->misc.tmpState = myCh->op.state;
			myCh->misc.tmpCode = myCh->op.code;
			if(myPs->config.LimitVI.limit_action_I == P0) {
				myCh->op.code = C_FAULT_LIMIT_CURRENT;
			} else if(myPs->config.LimitVI.limit_action_I == P1) {
				myCh->op.code = C_FAULT_END_LIMIT_CURRENT;
			}
			rtn = FAULT_COND;
		}
	}

	if(myData->mData.config.LimitVI.limit_use_V != 0) {
		if(myCh->signal[C_SIG_LIMIT_VOLTAGE] == P1) { //20171024 sch add
			myCh->signal[C_SIG_LIMIT_VOLTAGE] = P0;
			myCh->misc.tmpState = myCh->op.state;
			myCh->misc.tmpCode = myCh->op.code;
			if(myPs->config.LimitVI.limit_action_V == P0) {
				myCh->op.code = C_FAULT_LIMIT_VOLTAGE;
			} else if(myPs->config.LimitVI.limit_action_V == P1) {
				myCh->op.code = C_FAULT_END_LIMIT_VOLTAGE;
			}
			rtn = FAULT_COND;
		}
	}
#endif

	if(rtn == FAULT_COND) { //fault
		myCh->op.select = SAVE_FLAG_SAVING_ETC;
		send_save_msg(bd, ch, saveDt, 0);

		//170106 oys add
		if(myCh->misc.tempWaitType == P0){
			myCh->misc.chGroupNo = 0;
			//20180417 add
			myCh->misc.stepSyncFlag = P0;
			myCh->misc.endState = P0;		//220203_hun
			myCh->misc.groupAvgVsens = 0;	//220203_hun
			myCh->misc.group_StartVoltage_flag = 0;
		}
		//190318 lyhw
		if(myData->mData.config.parallelMode == P2){
			cCycle_p_ch_check(bd, ch);
		}
		myCh->op.phase = P100;
		cNextStepCheck(bd, ch);
	}
}

#if AUX_CONTROL == 1
	#if NETWORK_VERSION >= 4103
void cFaultCond_Aux(int bd, int ch)
{
	unsigned long advStepNo;
	int i, j, auxNo, map, rtn;
	long val;

	rtn = NORMAL_COND;
	
	if(myCh->op.state != C_RUN) return;
	if(myCh->misc.patternPhase != 0) return;

	if(myPs->config.installedTemp == 0
		&& myPs->config.installedAuxV == 0) return;

	auxNo = myPs->config.installedTemp + myPs->config.installedAuxV;
	if(auxNo > MAX_AUX_DATA) auxNo = MAX_AUX_DATA;

	advStepNo = myCh->misc.advStepNo;

	for(i=0; i < auxNo; i++) {
		if((ch+1) != myData->auxSetData[i].chNo) continue;

		j = myData->auxSetData[i].auxChNo - 1;

		if(myData->auxSetData[i].auxType == 0) { //aux temperature
			val = myData->AnalogMeter.temp[j].temp;	
			
			if(cFaultCond_Aux_step(bd, ch, advStepNo, val, i) > NORMAL_COND) 
				return;	
		}else if(myData->auxSetData[i].auxType == 1) { //aux voltage
			map = myData->daq.misc.map[j];
			if(map < 0) continue;
			val = myData->daq.op.ch_vsens[map];
			
			if(cFaultCond_Aux_step(bd, ch, advStepNo, val, i) > NORMAL_COND)
				return;
			
		}
	}
}

int cFaultCond_Aux_step(int bd, int ch, int advStepNo, long val_1, int aux_idx)
{
	long factor, rtn, val_2, val_3;
	unsigned long saveDt;

	S_CH_STEP_INFO step;

	saveDt = step.saveDt;

	rtn = NORMAL_COND;

	val_2 = myData->auxSetData[aux_idx].fault_upper;
	val_3 = myData->auxSetData[aux_idx].fault_lower;
	
	if(myData->auxSetData[aux_idx].auxType == 0) { //temperature
		factor = 1;
	} else { //voltage
		factor = 1000;
	}
	if(val_2 != 0){
		if(val_1 >= val_2)
		{
			myData->auxMisc.faultCnt[aux_idx]++;
			if(myData->auxMisc.faultCnt[aux_idx] >= MAX_ERROR_CNT) {
				myData->auxMisc.faultCnt[aux_idx] = 0;
				myCh->misc.tmpState = myCh->op.state;
				myCh->misc.tmpCode = myCh->op.code;
				rtn = 100;
			}
		}
		
	}
	if(val_3 != 0){
		if(val_1 <= val_3)
		{
			myData->auxMisc.faultCnt[aux_idx]++;
			if(myData->auxMisc.faultCnt[aux_idx] >= MAX_ERROR_CNT) {
				myData->auxMisc.faultCnt[aux_idx] = 0;
				myCh->misc.tmpState = myCh->op.state;
				myCh->misc.tmpCode = myCh->op.code;
				rtn = 200;
			}
		}
	}

	if(myData->auxSetData[aux_idx].auxType == 0) { //temperature
		if(rtn == 100){
			myCh->op.code = C_FAULT_AUX_TEMP_UPPER;
			rtn = FAULT_COND;
		}
		if(rtn == 200){
			myCh->op.code = C_FAULT_AUX_TEMP_LOWER;
			rtn = FAULT_COND;
		}
	}else{
		if(rtn == 100){
			myCh->op.code = C_FAULT_AUX_VOLTAGE_UPPER;
			rtn = FAULT_COND;
		}
		if(rtn == 200){
			myCh->op.code = C_FAULT_AUX_VOLTAGE_LOWER;
			rtn = FAULT_COND;
		}
	}
	if(rtn == FAULT_COND) { //fault
		myCh->op.select = SAVE_FLAG_SAVING_ETC;
		send_save_msg(bd, ch, saveDt, 0);
		myCh->op.phase = P100;
		cNextStepCheck(bd, ch);
	}
	return rtn;
}		
void cFaultCond_Aux_P(int bd, int ch)
{
	unsigned long advStepNo;
	int i, j, auxNo, map, rtn;
	long val;

	rtn = NORMAL_COND;
	
	if(myCh->op.state != C_RUN) return;
	if(myCh->misc.patternPhase != 0) return;

	if(myPs->config.installedTemp == 0
		&& myPs->config.installedAuxV == 0) return;

	auxNo = myPs->config.installedTemp + myPs->config.installedAuxV;
	if(auxNo > MAX_AUX_DATA) auxNo = MAX_AUX_DATA;

	advStepNo = myCh->misc.advStepNo;

	for(i=0; i < auxNo; i++) {
		if((ch+1) != myData->auxSetData[i].chNo) continue;

		j = myData->auxSetData[i].auxChNo - 1;

		if(myData->auxSetData[i].auxType == 0) { //aux temperature
			val = myData->AnalogMeter.temp[j].temp;	
			
			if(cFaultCond_Aux_step_P(bd, ch, advStepNo, val, i) > NORMAL_COND) 
				return;	
		}else if(myData->auxSetData[i].auxType == 1) { //aux voltage
			map = myData->daq.misc.map[j];
			if(map < 0) continue;
			val = myData->daq.op.ch_vsens[map];
			
			if(cFaultCond_Aux_step_P(bd, ch, advStepNo, val, i) > NORMAL_COND)
				return;
			
		}
	}
}

int cFaultCond_Aux_step_P(int bd, int ch, int advStepNo, long val_1,int aux_idx)
{
	long factor, rtn, val_2, val_3;
	unsigned long saveDt;

	S_CH_STEP_INFO step;

	saveDt = step.saveDt;

	rtn = NORMAL_COND;

	val_2 = myData->auxSetData[aux_idx].fault_upper;
	val_3 = myData->auxSetData[aux_idx].fault_lower;
	
	if(myData->auxSetData[aux_idx].auxType == 0) { //temperature
		factor = 1;
	} else { //voltage
		factor = 1000;
	}
	if(val_2 != 0){
		if(val_1 >= val_2)
		{
			myData->auxMisc.faultCnt[aux_idx]++;
			if(myData->auxMisc.faultCnt[aux_idx] >= MAX_ERROR_CNT) {
				myData->auxMisc.faultCnt[aux_idx] = 0;
				myCh->misc.tmpState = myCh->op.state;
				myCh->misc.tmpCode = myCh->op.code;
				rtn = 100;
			}
		}
		
	}
	if(val_3 != 0){
		if(val_1 <= val_3)
		{
			myData->auxMisc.faultCnt[aux_idx]++;
			if(myData->auxMisc.faultCnt[aux_idx] >= MAX_ERROR_CNT) {
				myData->auxMisc.faultCnt[aux_idx] = 0;
				myCh->misc.tmpState = myCh->op.state;
				myCh->misc.tmpCode = myCh->op.code;
				rtn = 200;
			}
		}
	}

	if(myData->auxSetData[aux_idx].auxType == 0) { //temperature
		if(rtn == 100){
			myCh->op.code = C_FAULT_AUX_TEMP_UPPER;
			rtn = FAULT_COND;
		}
		if(rtn == 200){
			myCh->op.code = C_FAULT_AUX_TEMP_LOWER;
			rtn = FAULT_COND;
		}
	}else{
		if(rtn == 100){
			myCh->op.code = C_FAULT_AUX_VOLTAGE_UPPER;
			rtn = FAULT_COND;
		}
		if(rtn == 200){
			myCh->op.code = C_FAULT_AUX_VOLTAGE_LOWER;
			rtn = FAULT_COND;
		}
	}
	if(rtn == FAULT_COND) { //fault
		myData->bData[bd].cData[ch-1].op.code = myCh->op.code;
		myData->bData[bd].cData[ch-1].misc.tmpCode = myCh->misc.tmpCode;
		myData->bData[bd].cData[ch-1].misc.tmpState = myCh->misc.tmpState;
		myData->bData[bd].cData[ch-1].op.phase = P100;
		
		myCh->op.select = SAVE_FLAG_SAVING_ETC;
		myData->bData[bd].cData[ch-1].op.select = myCh->op.select;
		send_save_msg(bd, ch, saveDt, 0);
		myCh = &(myData->bData[bd].cData[ch]);
		myCh->op.phase = P100;
		
		cNextStepCheck(bd, ch-1);
		myCh = &(myData->bData[bd].cData[ch]);
		cNextStepCheck(bd, ch);
	}
	return rtn;
}		
	#endif
#endif

//2109091 LJS
void DropV_Charge_p(int bd, int ch)
{
    long m_Vsens;
	long Check_Period_C_P, diagnostic_Point_C_P;
	unsigned char type;
	int rtn = 0;
	unsigned long saveDt;
	
	S_CH_STEP_INFO step;

	if(myData->mData.config.function[F_HW_FAULT_COND] == P0) return;
	if(myData->mData.config.LGES_fault_config.drop_v_charge_check_time < 1) return;
	if(myData->bData[bd].cData[ch].op.type == STEP_DISCHARGE) return;
	
	diagnostic_Point_C_P
	 = myData->mData.config.LGES_fault_config.drop_v_charge_start_time;	

	myCh = &(myData->bData[bd].cData[ch]);
	myTestCond = &(myPs->testCond[bd][ch]);
	
	if(myCh->op.state != C_RUN) return;
	if(myCh->op.phase != P50) return;
	if(myCh->op.checkDelayTime < diagnostic_Point_C_P) return;

	Check_Period_C_P
	 //= myData->mData.config.LGES_fault_config.drop_v_charge_check_time * 10;	
	 = myData->mData.config.LGES_fault_config.drop_v_charge_check_time;	//Count	
	
	//master Vsens
	m_Vsens = myData->bData[bd].cData[ch-1].op.Vsens;

	step = step_info(bd, ch);

	saveDt = step.saveDt;
	type = step.type;
	
	if(myCh->misc.Drop_maxV == 0){
		myCh->misc.Drop_maxV = m_Vsens;
	}
	
	if(m_Vsens > myData->bData[bd].cData[ch-1].misc.Drop_maxV) {
		myData->bData[bd].cData[ch-1].misc.Drop_maxV = m_Vsens;
		myCh->misc.Drop_maxV = m_Vsens;
	}

	switch(type) {
		case STEP_CHARGE:
			if(myData->mData.config.hwFaultConfig[HW_FAULT_DROP_V_1] != 0){
				if((myCh->misc.Drop_maxV - m_Vsens) 
					>= myData->mData.config.hwFaultConfig[HW_FAULT_DROP_V_1]){
					myCh->misc.errCnt[C_CNT_DROP_V]++;
					if(myCh->misc.errCnt[C_CNT_DROP_V] >= Check_Period_C_P){
						myCh->misc.errCnt[C_CNT_DROP_V] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_VOLTAGE_DROP;
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_DROP_V] = 0;
				}
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
		if(myCh->misc.tempWaitType == P0){
			myCh->misc.chGroupNo = 0;
			//20180417 add
			myCh->misc.stepSyncFlag = P0;
			myCh->misc.endState = P0;		//220203_hun
			myCh->misc.groupAvgVsens = 0;	//220203_hun
			myCh->misc.group_StartVoltage_flag = 0;
		}
		myCh->op.phase = P100;
		cNextStepCheck(bd, ch-1);
		myCh = &(myData->bData[bd].cData[ch]);
		cNextStepCheck(bd, ch);
	}else if(rtn == 1){	//200504 hun add , END_COND
		myCh->op.select = SAVE_FLAG_SAVING_ETC;
		send_save_msg(bd, ch, saveDt, 0);
	}
}

void DropV_DisCharge_p(int bd, int ch)
{
    long m_Vsens;
	long Check_Period_D_P, diagnostic_Point_D_P;
	unsigned char type;
	int rtn = 0;
	unsigned long saveDt;
	
	S_CH_STEP_INFO step;

	if(myData->mData.config.function[F_HW_FAULT_COND] == P0) return;
	if(myData->mData.config.LGES_fault_config.drop_v_discharge_check_time < 1) return;
	if(myData->bData[bd].cData[ch].op.type == STEP_CHARGE) return;
	
	diagnostic_Point_D_P
	 = myData->mData.config.LGES_fault_config.drop_v_discharge_start_time;	

	myCh = &(myData->bData[bd].cData[ch]);
	myTestCond = &(myPs->testCond[bd][ch]);
	
	if(myCh->op.state != C_RUN) return;
	if(myCh->op.phase != P50) return;
	if(myCh->op.checkDelayTime < diagnostic_Point_D_P) return;

	Check_Period_D_P
	 //= myData->mData.config.LGES_fault_config.drop_v_charge_check_time * 10;	
	 = myData->mData.config.LGES_fault_config.drop_v_charge_check_time; //Count	

	//master Vsens
	m_Vsens = myData->bData[bd].cData[ch-1].op.Vsens;

	step = step_info(bd, ch);

	saveDt = step.saveDt;
	type = step.type;
	
	if(myCh->misc.Drop_minV == 0){
		myCh->misc.Drop_minV = m_Vsens;
	}
	
	if(m_Vsens < myData->bData[bd].cData[ch-1].misc.Drop_minV) {
		myData->bData[bd].cData[ch-1].misc.Drop_minV = m_Vsens;
		myCh->misc.Drop_minV = m_Vsens;
	}

	switch(type) {
		case STEP_DISCHARGE:
			if(myCh->op.checkDelayTime % Check_Period_D_P != 0) break;
			if(myData->mData.config.hwFaultConfig[HW_FAULT_DROP_V_2] != 0){
				if((m_Vsens - myCh->misc.minV) 
					>= myData->mData.config.hwFaultConfig[HW_FAULT_DROP_V_2]){
					myCh->misc.errCnt[C_CNT_DROP_V]++;
					if(myCh->misc.errCnt[C_CNT_DROP_V] >= Check_Period_D_P){
						myCh->misc.errCnt[C_CNT_DROP_V] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_VOLTAGE_DROP;
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_DROP_V] = 0;
				}
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
		if(myCh->misc.tempWaitType == P0){
			myCh->misc.chGroupNo = 0;
			//20180417 add
			myCh->misc.stepSyncFlag = P0;
			myCh->misc.endState = P0;		//220203_hun
			myCh->misc.groupAvgVsens = 0;	//220203_hun
			myCh->misc.group_StartVoltage_flag = 0;
		}
		myCh->op.phase = P100;
		cNextStepCheck(bd, ch-1);
		myCh = &(myData->bData[bd].cData[ch]);
		cNextStepCheck(bd, ch);
	}else if(rtn == 1){	//200504 hun add , END_COND
		myCh->op.select = SAVE_FLAG_SAVING_ETC;
		send_save_msg(bd, ch, saveDt, 0);
	}
}

void DropV_Charge(int bd, int ch)
{
	long diagnostic_Point_C, Check_Period_C;
	unsigned char type;
	int rtn = 0;
	unsigned long saveDt;
		
	S_CH_STEP_INFO step;
	
	if(myData->mData.config.function[F_HW_FAULT_COND] == P0) return;
	if(myData->mData.config.LGES_fault_config.drop_v_charge_check_time < 1) return;
	if(myData->bData[bd].cData[ch].op.type == STEP_DISCHARGE) return;
	
	myCh = &(myData->bData[bd].cData[ch]);
	myTestCond = &(myPs->testCond[bd][ch]);

	diagnostic_Point_C
	 = myData->mData.config.LGES_fault_config.drop_v_charge_start_time;	

	if(myCh->op.state != C_RUN) return;
	if(myCh->op.phase != P50) return;
	if(myCh->op.checkDelayTime < diagnostic_Point_C) return;
	
	Check_Period_C
	 //= myData->mData.config.LGES_fault_config.drop_v_charge_check_time * 10;
	 = myData->mData.config.LGES_fault_config.drop_v_charge_check_time;	//Count

	step = step_info(bd, ch);

	saveDt = step.saveDt;
	type = step.type;
	
	if(myCh->misc.Drop_maxV == 0){
		myCh->misc.Drop_maxV = myCh->op.Vsens;
	}
	
	if(myCh->op.Vsens > myCh->misc.Drop_maxV) {
		myCh->misc.Drop_maxV = myCh->op.Vsens;
	}

	switch(type) {
		case STEP_CHARGE:
			if(myData->mData.config.hwFaultConfig[HW_FAULT_DROP_V_1] != 0){
				if((myCh->misc.Drop_maxV - myCh->op.Vsens) 
					>= myData->mData.config.hwFaultConfig[HW_FAULT_DROP_V_1]){
					myCh->misc.errCnt[C_CNT_DROP_V]++;
					if(myCh->misc.errCnt[C_CNT_DROP_V] >= Check_Period_C){
						myCh->misc.errCnt[C_CNT_DROP_V] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_VOLTAGE_DROP;
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_DROP_V] = 0;
				}
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
			myCh->misc.endState = P0;		//220203_hun
			myCh->misc.groupAvgVsens = 0;	//220203_hun
			myCh->misc.group_StartVoltage_flag = 0;
		}
		myCh->op.phase = P100;
		cNextStepCheck(bd, ch);
	}else if(rtn == 1){	//200504 hun add , END_COND
		myCh->op.select = SAVE_FLAG_SAVING_ETC;
		send_save_msg(bd, ch, saveDt, 0);
	}
}

void DropV_DisCharge(int bd, int ch)
{
	long diagnostic_Point_D, Check_Period_D;
	unsigned char type;
	int rtn = 0;
	unsigned long saveDt;
		
	S_CH_STEP_INFO step;

	if(myData->mData.config.function[F_HW_FAULT_COND] == P0) return;
	if(myData->mData.config.LGES_fault_config.drop_v_discharge_check_time < 1) return;
	if(myData->bData[bd].cData[ch].op.type == STEP_CHARGE) return;
	
	myCh = &(myData->bData[bd].cData[ch]);
	myTestCond = &(myPs->testCond[bd][ch]);
	
	diagnostic_Point_D 
	 = myData->mData.config.LGES_fault_config.drop_v_discharge_start_time;	
	if(myCh->op.state != C_RUN) return;
	if(myCh->op.phase != P50) return;
	if(myCh->op.checkDelayTime < diagnostic_Point_D) return;
	
	Check_Period_D
	 //= myData->mData.config.LGES_fault_config.drop_v_discharge_check_time * 10;
	 = myData->mData.config.LGES_fault_config.drop_v_discharge_check_time;	//Count

	step = step_info(bd, ch);

	saveDt = step.saveDt;
	type = step.type;
	
	if(myCh->misc.Drop_minV == 0){
		myCh->misc.Drop_minV = myCh->op.Vsens;
	}
	
	if(myCh->op.Vsens < myCh->misc.Drop_minV) {
		myCh->misc.Drop_minV = myCh->op.Vsens;
	}

	switch(type) {
		case STEP_DISCHARGE:
			if(myData->mData.config.hwFaultConfig[HW_FAULT_DROP_V_2] != 0){
				if((myCh->op.Vsens - myCh->misc.Drop_minV) 
					>= myData->mData.config.hwFaultConfig[HW_FAULT_DROP_V_2]){
					myCh->misc.errCnt[C_CNT_DROP_V]++;
					if(myCh->misc.errCnt[C_CNT_DROP_V] >= Check_Period_D){
						myCh->misc.errCnt[C_CNT_DROP_V] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_VOLTAGE_DROP;
						rtn = FAULT_COND;
						break;
					}
				}else{
					myCh->misc.errCnt[C_CNT_DROP_V] = 0;
				}
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
			myCh->misc.endState = P0;		//220203_hun
			myCh->misc.groupAvgVsens = 0;	//220203_hun
			myCh->misc.group_StartVoltage_flag = 0;
		}
		myCh->op.phase = P100;
		cNextStepCheck(bd, ch);
	}else if(rtn == 1){	//200504 hun add , END_COND
		myCh->op.select = SAVE_FLAG_SAVING_ETC;
		send_save_msg(bd, ch, saveDt, 0);
	}
}

//190829 oys add : SAMSUNG SDI Safety Condition
int SAMSUNG_SDI_safety(int bd, int ch, int rtn, int stepType) 
{
	unsigned long diff_v, diff_t, changeV_Dt, changeV_Dv, deltaV_Dv, deltaV_Dt;
	unsigned long maxI, rangeI, deltaI_Di;
	long m_Vsens = 0, m_Isens = 0;
	#ifdef _SDI_SAFETY_V1	
	unsigned long fault_deltaDv;	//hun_200430
	#endif

	if(VENDER != 2) return rtn;

	if(myCh->ChAttribute.chNo_master == P0) {
		m_Vsens = myData->bData[bd].cData[ch-1].op.Vsens;
		m_Isens = myData->bData[bd].cData[ch-1].op.Isens;
	}
//===================================
//		checkDt = 6000*3; 	// 3min
//		checkDv = 1000;		// 1mV
//		checkDt = 10; 		// 100ms
//		checkDv = 100000;	// 100mV
//===================================
	rangeI = myCh->op.rangeI;
	maxI = myData->mData.config.maxCurrent[rangeI];
	//changeV_Dt = (unsigned long)myPs->testCond[bd][ch].safety.changeV_Dt;
	changeV_Dt = (unsigned long)myPs->testCond[bd][ch].safety.changeV_Dt * 10;
	changeV_Dv = (unsigned long)myPs->testCond[bd][ch].safety.changeV_Dv * 1000;
	//deltaV_Dt = (unsigned long)myPs->testCond[bd][ch].safety.deltaV_Dt;
	deltaV_Dt = (unsigned long)myPs->testCond[bd][ch].safety.deltaV_Dt * 10;
	deltaV_Dv = (unsigned long)myPs->testCond[bd][ch].safety.deltaV_Dv * 1000;
	deltaI_Di = (unsigned long)maxI * 0.001;

	//SAMSUNG SDI VOLTAGE CHANGE ERROR CHECK	
	#ifdef _SDI_SAFETY_V1	
	fault_deltaDv = (unsigned long)myPs->testCond[bd][ch].safety.fault_deltaDv * 1000;	//hun_200430
	//hun_200504_s	
	if(fault_deltaDv != 0){
		if(myCh->misc.cvFlag == P0) {
			if(myCh->op.checkDelayTime < 300){		//hun_201010
				myCh->misc.fault_deltaV = myCh->op.Vsens;
			}else{
				if(labs(myCh->misc.fault_deltaV - myCh->op.Vsens) >= fault_deltaDv){
					myCh->misc.errCnt[C_CNT_FAULT_DELTA_V]++;
					if(myCh->misc.errCnt[C_CNT_FAULT_DELTA_V] >= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_FAULT_DELTA_V] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_CHANGE_DELTA_V;
						Fault_Value_Check(myCh->misc.advStepNo, 
						myCh->op.Vsens, myCh->misc.fault_deltaV,
						fault_deltaDv, 0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
					}else if(myCh->misc.errCnt[C_CNT_FAULT_DELTA_V] != 0 &&
						myCh->misc.errCnt[C_CNT_FAULT_DELTA_V] < MAX_ERROR_CNT){
						rtn = 1;
					}
				}else{
					myCh->misc.fault_deltaV = myCh->op.Vsens;
					myCh->misc.errCnt[C_CNT_FAULT_DELTA_V] = 0;
				}
			}
		}
	}
	//hun_200504_e
	#endif
	// SAMSUNG SDI VOLTAGE NOT CHANGE CHECK
	// hun_200426_re
	if(changeV_Dt != 0 && changeV_Dv != 0) {
		if(myCh->ChAttribute.chNo_master == P0) {
			if(myData->bData[bd].cData[ch-1].misc.cvFlag == P0) {
				diff_v = labs(m_Vsens -
						myData->bData[bd].cData[ch-1].misc.pre_chk_v);
				if(diff_v >= changeV_Dv){
					myCh->op.changeV_Dt_cnt = 1;	//hun_200430
				}
				if(myData->bData[bd].cData[ch-1].op.runTime > 180) {
					diff_t = myData->bData[bd].cData[ch-1].op.runTime
								- myData->bData[bd].cData[ch-1]
									.misc.pre_v_chk_time;
					if(diff_t >= changeV_Dt) {
						myData->bData[bd].cData[ch-1].misc.pre_v_chk_time
							= myData->bData[bd].cData[ch-1].op.runTime;
						if(myCh->op.changeV_Dt_cnt == 0) {	//hun_200430
							myCh->misc.tmpState = myCh->op.state;
							myCh->misc.tmpCode = myCh->op.code;
							myCh->op.code = C_FAULT_CHANGE_V;
							Fault_Value_Check(myCh->misc.advStepNo, 
							m_Vsens, myData->bData[bd].cData[ch-1].misc.pre_chk_v,
						   	myData->bData[bd].cData[ch-1].op.runTime,
						   	myData->bData[bd].cData[ch-1].misc.pre_v_chk_time,
						   	changeV_Dv, changeV_Dt, 0, 0, 0);
							rtn = FAULT_COND;
						}else if(myCh->op.changeV_Dt_cnt != 0){	//hun_200430
							myCh->op.changeV_Dt_cnt = 0;
						}
						myData->bData[bd].cData[ch-1].misc.pre_chk_v = m_Vsens;
					}
				}
			}
		} else if(myCh->ChAttribute.opType == P0) {
			if(myCh->misc.cvFlag == P0) {
				diff_v = labs(myCh->op.Vsens - myCh->misc.pre_chk_v);
				if(diff_v >= changeV_Dv){
					myCh->op.changeV_Dt_cnt = 1;	//hun_200430
				}
				if(myCh->op.runTime > 180) {
					diff_t = myCh->op.runTime - myCh->misc.pre_v_chk_time;
					if(diff_t >= changeV_Dt) {
						myCh->misc.pre_v_chk_time = myCh->op.runTime;
						if(myCh->op.changeV_Dt_cnt == 0) {	//hun_200430
							myCh->misc.tmpState = myCh->op.state;
							myCh->misc.tmpCode = myCh->op.code;
							myCh->op.code = C_FAULT_CHANGE_V;
							Fault_Value_Check(myCh->misc.advStepNo, 
							myCh->op.Vsens, myCh->misc.pre_chk_v,
						   	myCh->op.runTime, myCh->misc.pre_v_chk_time,
						   	changeV_Dv, changeV_Dt, 0, 0, 0);
							rtn = FAULT_COND;
						}else if(myCh->op.changeV_Dt_cnt != 0){	//hun_200430
							myCh->op.changeV_Dt_cnt = 0;
						}
						myCh->misc.pre_chk_v = myCh->op.Vsens;
					}
				}
			}
		}
	}
	// SAMSUNG SDI DELTA V CHECK
	if(deltaV_Dt != 0 && deltaV_Dv != 0) {
		if(myCh->ChAttribute.chNo_master == P0) {
			switch(stepType) {
				case STEP_CHARGE:
					if(myData->bData[bd].cData[ch-1].misc.cvFlag == P0) {
						if((myData->bData[bd].cData[ch-1].misc.maxV - m_Vsens)
							>= deltaV_Dv){
								
							diff_t = myData->bData[bd].cData[ch-1].op.runTime
									- myData->bData[bd].cData[ch-1]
										.misc.deltaV_timeout;
							if(diff_t >= deltaV_Dt){
								myData->bData[bd].cData[ch-1]
									.misc.deltaV_timeout
									= myData->bData[bd].cData[ch-1].op.runTime;
								myCh->misc.tmpState = myCh->op.state;
								myCh->misc.tmpCode =  myCh->op.code;
								myCh->op.code = C_FAULT_DELTA_V;
								rtn = FAULT_COND;
								break;
							}
						}else{	
							myData->bData[bd].cData[ch-1].misc.deltaV_timeout
								= myData->bData[bd].cData[ch-1].op.runTime;
						}
					} else {
						if(labs(m_Isens - myData->bData[bd].cData[ch-1]
											.misc.minI) >= deltaI_Di) {
							diff_t = myData->bData[bd].cData[ch-1].op.runTime
										- myData->bData[bd].cData[ch-1]
											.misc.deltaV_timeout;
							if(diff_t >= deltaV_Dt){
								myData->bData[bd].cData[ch-1]
									.misc.deltaV_timeout
									= myData->bData[bd].cData[ch-1].op.runTime;
								myCh->misc.tmpState = myCh->op.state;
								myCh->misc.tmpCode =  myCh->op.code;
								myCh->op.code = C_FAULT_DELTA_I;
								rtn = FAULT_COND;
								break;
							}
						}else{	
							myData->bData[bd].cData[ch-1].misc.deltaV_timeout
								= myData->bData[bd].cData[ch-1].op.runTime;
						}
					}
					break;
				case STEP_DISCHARGE:
					if(myData->bData[bd].cData[ch-1].misc.cvFlag == P0) {
						if((m_Vsens - myData->bData[bd].cData[ch-1].misc.minV)
										>= deltaV_Dv){
							diff_t = myData->bData[bd].cData[ch-1].op.runTime
										- myData->bData[bd].cData[ch-1]
											.misc.deltaV_timeout;
							if(diff_t >= deltaV_Dt){
								myData->bData[bd].cData[ch-1]
									.misc.deltaV_timeout
									= myData->bData[bd].cData[ch-1].op.runTime;
								myCh->misc.tmpState = myCh->op.state;
								myCh->misc.tmpCode =  myCh->op.code;
								myCh->op.code = C_FAULT_DELTA_V;
								rtn = FAULT_COND;
								break;
							}
						}else{
							myData->bData[bd].cData[ch-1].misc.deltaV_timeout
								= myData->bData[bd].cData[ch-1].op.runTime;
						}
					} else {
						if(labs(myData->bData[bd].cData[ch-1].misc.maxI
								- m_Isens) >= deltaI_Di){
							diff_t = myData->bData[bd].cData[ch-1].op.runTime
										- myData->bData[bd].cData[ch-1]
											.misc.deltaV_timeout;
							if(diff_t >= deltaV_Dt){
								myData->bData[bd].cData[ch-1]
									.misc.deltaV_timeout
									= myData->bData[bd].cData[ch-1].op.runTime;
								myCh->misc.tmpState = myCh->op.state;
								myCh->misc.tmpCode =  myCh->op.code;
								myCh->op.code = C_FAULT_DELTA_I;
								rtn = FAULT_COND;
								break;
							}
						}else{
							myData->bData[bd].cData[ch-1].misc.deltaV_timeout
								= myData->bData[bd].cData[ch-1].op.runTime;
						}
					}
					break;
				default:
					break;
			}
		} else if(myCh->ChAttribute.opType == P0) {
			switch(stepType) {
				case STEP_CHARGE:
					if(myCh->misc.cvFlag == P0) {
						if((myCh->misc.maxV - myCh->op.Vsens) >= deltaV_Dv){
							diff_t = myCh->op.runTime - myCh->misc.deltaV_timeout;
							if(diff_t >= deltaV_Dt){
								myCh->misc.deltaV_timeout = myCh->op.runTime;
								myCh->misc.tmpState = myCh->op.state;
								myCh->misc.tmpCode = myCh->op.code;
								myCh->op.code = C_FAULT_DELTA_V;
								rtn = FAULT_COND;
								break;
							}
						}else{
							myCh->misc.deltaV_timeout = myCh->op.runTime;
						}
					} else {
						if(labs(myCh->op.Isens - myCh->misc.minI) >= deltaI_Di){
							diff_t = myCh->op.runTime - myCh->misc.deltaV_timeout;
							if(diff_t >= deltaV_Dt){
								myCh->misc.deltaV_timeout = myCh->op.runTime;
								myCh->misc.tmpState = myCh->op.state;
								myCh->misc.tmpCode = myCh->op.code;
								myCh->op.code = C_FAULT_DELTA_I;
								rtn = FAULT_COND;
								break;
							}
						}else{	
							myCh->misc.deltaV_timeout = myCh->op.runTime;
						}
					}
					break;
				case STEP_DISCHARGE:
					if(myCh->misc.cvFlag == P0) {
						if((myCh->op.Vsens - myCh->misc.minV) >= deltaV_Dv){
							diff_t = myCh->op.runTime - myCh->misc.deltaV_timeout;
							if(diff_t >= deltaV_Dt){
								myCh->misc.deltaV_timeout = myCh->op.runTime;
								myCh->misc.tmpState = myCh->op.state;
								myCh->misc.tmpCode = myCh->op.code;
								myCh->op.code = C_FAULT_DELTA_V;
								rtn = FAULT_COND;
								break;
							}
						}else{	
							myCh->misc.deltaV_timeout = myCh->op.runTime;
						}
					} else {
						if(labs(myCh->misc.maxI - myCh->op.Isens) >= deltaI_Di){
							diff_t = myCh->op.runTime - myCh->misc.deltaV_timeout;
							if(diff_t >= deltaV_Dt){
								myCh->misc.deltaV_timeout = myCh->op.runTime;
								myCh->misc.tmpState = myCh->op.state;
								myCh->misc.tmpCode = myCh->op.code;
								myCh->op.code = C_FAULT_DELTA_I;
								rtn = FAULT_COND;
								break;
							}
						}else{
							myCh->misc.deltaV_timeout = myCh->op.runTime;
						}
					}
					break;
				default:
					break;
			}
		}
	}
	
	return rtn;
}

int cvFault_Check(int bd, int ch, int rtn, int stepType, long refV)
{	//210204 lyhw
	long m_Vsens, m_Isens, cvFaultV_val, cvFaultI_val;
	long chVsens, chIsens, chMaxI, chMinI;
	long cvFaultV_time, cvFaultI_time;
		
	if(myCh->misc.cvFaultCheckFlag != P1) return rtn;
	chVsens = chIsens = chMaxI = chMinI = 0;

	cvFaultV_val = myData->mData.config.hwFaultConfig[HW_FAULT_CV_VOLTAGE];
	cvFaultI_val = myData->mData.config.hwFaultConfig[HW_FAULT_CV_CURRENT];
	cvFaultV_time 
		= myData->mData.config.LGES_fault_config.cv_voltage_start_time;
	cvFaultI_time
		= myData->mData.config.LGES_fault_config.cv_current_start_time;
			
	if(myCh->ChAttribute.chNo_master == P0) {
		m_Vsens = myData->bData[bd].cData[ch-1].op.Vsens;
		m_Isens = myData->bData[bd].cData[ch-1].op.Isens;
		chVsens = m_Vsens;
		chIsens = (myCh->op.Isens + myData->bData[bd].cData[ch-1].op.Isens);
		chMaxI = (myCh->misc.maxI + myData->bData[bd].cData[ch-1].misc.maxI);
		chMinI = (myCh->misc.minI + myData->bData[bd].cData[ch-1].misc.minI);
	}else if(myCh->ChAttribute.opType == P0) {
		chVsens = myCh->op.Vsens;
		chIsens = myCh->op.Isens;
		chMaxI = myCh->misc.maxI;
		chMinI = myCh->misc.minI;
	}

	//1. Check for cv Voltage Drop
	if(cvFaultV_val != 0){
		switch(stepType){
			case STEP_CHARGE:
				if(myCh->misc.cvTime < cvFaultV_time) break;
				if(labs(refV - chVsens) >= cvFaultV_val){
					myCh->misc.errCnt[C_CNT_CV_FAULT_V]++;
					if(myCh->misc.errCnt[C_CNT_CV_FAULT_V] >= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_CV_FAULT_V] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_CV_VOLTAGE;
						rtn = FAULT_COND;
						return rtn;
					}
				}else{
					myCh->misc.errCnt[C_CNT_CV_FAULT_V] = 0;
				}
				break;
			case STEP_DISCHARGE:
				if(myCh->misc.cvTime < cvFaultV_time) break;
				if(labs(chVsens - refV) >= cvFaultV_val){
					myCh->misc.errCnt[C_CNT_CV_FAULT_V]++;
					if(myCh->misc.errCnt[C_CNT_CV_FAULT_V] >= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_CV_FAULT_V] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_CV_VOLTAGE;
						rtn = FAULT_COND;
						return rtn;
					}
				}else{
					myCh->misc.errCnt[C_CNT_CV_FAULT_V] = 0;
				}
				break;
			default:
				break;
		}
	}
	//2. Check for cv Current Drop
	if(cvFaultI_val != 0){
		switch(stepType){
			case STEP_CHARGE:
				if(myCh->misc.cvTime < cvFaultI_time) break;
				if((chIsens - chMinI) >= cvFaultI_val){
					myCh->misc.errCnt[C_CNT_CV_FAULT_I]++;
					if(myCh->misc.errCnt[C_CNT_CV_FAULT_I] >= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_CV_FAULT_I] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_CV_CURRENT;
						rtn = FAULT_COND;
						return rtn;
					}
				}else{
					myCh->misc.errCnt[C_CNT_CV_FAULT_I] = 0;
				}
				break;
			case STEP_DISCHARGE:
				if(myCh->misc.cvTime < cvFaultI_time) break;
				if((chMaxI - chIsens) >= cvFaultI_val){
					myCh->misc.errCnt[C_CNT_CV_FAULT_I]++;
					if(myCh->misc.errCnt[C_CNT_CV_FAULT_I] >= MAX_ERROR_CNT){
						myCh->misc.errCnt[C_CNT_CV_FAULT_I] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_CV_CURRENT;
						rtn = FAULT_COND;
						return rtn;
					}
				}else{
					myCh->misc.errCnt[C_CNT_CV_FAULT_I] = 0;
				}
				break;
			default:
				break;
		}
	}
	return rtn;
}

int tempFaultCheck(int bd, int ch, unsigned long advStepNo, unsigned char type)
{		
    int rtn=0;

	if(myData->bData[bd].cData[ch].ChAttribute.chNo_master == P0) {
		ch = ch-1;
	}
	
	if(myData->AnalogMeter.config.tempNonDisplay == P0){ 
		switch(type){
			case P0:	//safety fault Upper Temperature
				if((myData->bData[bd].cData[ch].op.temp
					>= myPs->testCond[bd][ch].safety.faultUpperTemp)
					|| (myData->AnalogMeter.config.multiNum == 2
					&& (myData->bData[bd].cData[ch].op.temp1
					>= myPs->testCond[bd][ch].safety.faultUpperTemp))){
					myCh->misc.errCnt[C_CNT_UPPER_TEMP]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_TEMP]
													>= MAX_ERROR_CNT_TEMP*4){
						myCh->misc.errCnt[C_CNT_UPPER_TEMP] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_CH_TEMP;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); //safety
						Fault_Value_Check(myCh->misc.advStepNo, myCh->op.temp,
						myTestCond->safety.faultUpperTemp,
					   	myTestCond->step[advStepNo].faultUpperTemp,
					   	0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
					}
				}else{
					myCh->misc.errCnt[C_CNT_UPPER_TEMP] = 0;
				}
				break;
			case P1:	//safety fault Lower Temperature
				if((myData->bData[bd].cData[ch].op.temp
					<= myPs->testCond[bd][ch].safety.faultLowerTemp)
					|| (myData->AnalogMeter.config.multiNum == 2
					&& (myData->bData[bd].cData[ch].op.temp1
					<= myPs->testCond[bd][ch].safety.faultLowerTemp))){
					myCh->misc.errCnt[C_CNT_LOWER_TEMP]++;
					if(myCh->misc.errCnt[C_CNT_LOWER_TEMP] 
													>= MAX_ERROR_CNT_TEMP*4){
						myCh->misc.errCnt[C_CNT_LOWER_TEMP] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_LOWER_CH_TEMP;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); //safety
						Fault_Value_Check(myCh->misc.advStepNo, 
						myData->bData[bd].cData[ch].op.temp,
						myTestCond->safety.faultLowerTemp,
						myTestCond->step[advStepNo].faultLowerTemp,
					   	0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
					}
				}else{
					myCh->misc.errCnt[C_CNT_LOWER_TEMP] = 0;
				}
				break;
			case P2:	//safety step fault Upper Temperature
				if((myData->bData[bd].cData[ch].op.temp
					>= myPs->testCond[bd][ch].step[advStepNo].faultUpperTemp)
					|| (myData->AnalogMeter.config.multiNum == 2
					&& (myData->bData[bd].cData[ch].op.temp1
					>= myPs->testCond[bd][ch].step[advStepNo].faultUpperTemp))){
					myCh->misc.errCnt[C_CNT_UPPER_TEMP_STEP]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_TEMP_STEP] 
													>= MAX_ERROR_CNT_TEMP*4){
						myCh->misc.errCnt[C_CNT_UPPER_TEMP_STEP] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_CH_TEMP;
						myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); //step_safety
						Fault_Value_Check(myCh->misc.advStepNo, 
						myData->bData[bd].cData[ch].op.temp,
						myTestCond->safety.faultUpperTemp,
						myTestCond->step[advStepNo].faultUpperTemp,
					   	0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
					}
				}else{
					myCh->misc.errCnt[C_CNT_UPPER_TEMP_STEP] = 0;
				}
				break;
			case P3:	//safety step fault Lower Temperature
				if((myData->bData[bd].cData[ch].op.temp
					<= myPs->testCond[bd][ch].step[advStepNo].faultLowerTemp)
					|| (myData->AnalogMeter.config.multiNum == 2
					&& (myData->bData[bd].cData[ch].op.temp1
					<= myPs->testCond[bd][ch].step[advStepNo].faultLowerTemp))){
					myCh->misc.errCnt[C_CNT_LOWER_TEMP_STEP]++;
					if(myCh->misc.errCnt[C_CNT_LOWER_TEMP_STEP] 
													>= MAX_ERROR_CNT_TEMP*4){
						myCh->misc.errCnt[C_CNT_LOWER_TEMP_STEP] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_LOWER_CH_TEMP;
						myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); //step_safety
						Fault_Value_Check(myCh->misc.advStepNo, 
						myData->bData[bd].cData[ch].op.temp,
						myTestCond->safety.faultLowerTemp,
						myTestCond->step[advStepNo].faultLowerTemp,
					    0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
					}
				}else{
					myCh->misc.errCnt[C_CNT_LOWER_TEMP_STEP] = 0;
				}
				break;
			default: break;
		}		
	}
		
	if(myData->AnalogMeter.config.auxControlFlag == P1 
		&& myData->mData.config.installedTemp > 0){
		switch(type){
			case P0:	//safety Aux Temp fault Upper Temperature
				if(myData->bData[bd].cData[ch].misc.AuxTemp_max
					>= myPs->testCond[bd][ch].safety.faultUpperTemp){
					myCh->misc.errCnt[C_CNT_UPPER_AUX_TEMP]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_AUX_TEMP]
												>= MAX_ERROR_CNT_TEMP*4){
					myCh->misc.errCnt[C_CNT_UPPER_AUX_TEMP] = 0;
					myCh->misc.tmpState = myCh->op.state;
					myCh->misc.tmpCode = myCh->op.code;
					myCh->op.code = C_FAULT_UPPER_CH_TEMP;
					myCh->op.code = code_convert(myCh->op.code, SAFETY); //safety
					Fault_Value_Check(myCh->misc.advStepNo, 
					myData->bData[bd].cData[ch].misc.AuxTemp_max,
					myTestCond->safety.faultUpperTemp,
					myTestCond->step[advStepNo].faultUpperTemp,
				   	0, 0, 0, 0, 0, 0);
					rtn = FAULT_COND;
					}
				}else{
					myCh->misc.errCnt[C_CNT_UPPER_AUX_TEMP] = 0;
				}
				break;
			case P1:	//safety Aux Temp fault Lower Temperature
				if(myData->bData[bd].cData[ch].misc.AuxTemp_min
					<= myPs->testCond[bd][ch].safety.faultLowerTemp){
					myCh->misc.errCnt[C_CNT_LOWER_AUX_TEMP]++;
					if(myCh->misc.errCnt[C_CNT_LOWER_AUX_TEMP] 
													>= MAX_ERROR_CNT_TEMP*4){
						myCh->misc.errCnt[C_CNT_LOWER_AUX_TEMP] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_LOWER_CH_TEMP;
						myCh->op.code = code_convert(myCh->op.code, SAFETY); //safety
						Fault_Value_Check(myCh->misc.advStepNo, 
						myData->bData[bd].cData[ch].misc.AuxTemp_min,
						myTestCond->safety.faultLowerTemp,
					   	myTestCond->step[advStepNo].faultLowerTemp,
					   	0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
					}
				}else{
					myCh->misc.errCnt[C_CNT_LOWER_AUX_TEMP] = 0;
				}
				break;
			case P2:	//safety step fault Upper Temperature
				if(myData->bData[bd].cData[ch].misc.AuxTemp_max
					>= myPs->testCond[bd][ch].step[advStepNo].faultUpperTemp){
					myCh->misc.errCnt[C_CNT_UPPER_AUX_TEMP_STEP]++;
					if(myCh->misc.errCnt[C_CNT_UPPER_AUX_TEMP_STEP] 
													>= MAX_ERROR_CNT_TEMP*4){
						myCh->misc.errCnt[C_CNT_UPPER_AUX_TEMP_STEP] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_UPPER_CH_TEMP;
						myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); //step_safety
						Fault_Value_Check(myCh->misc.advStepNo, 
						myData->bData[bd].cData[ch].misc.AuxTemp_max,
						myTestCond->safety.faultUpperTemp,
						myTestCond->step[advStepNo].faultUpperTemp,
					    0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
					}
				}else{
					myCh->misc.errCnt[C_CNT_UPPER_AUX_TEMP_STEP] = 0;
				}
				break;
			case P3:	//safety step fault Lower Temperature
				if(myData->bData[bd].cData[ch].misc.AuxTemp_min
					<= myPs->testCond[bd][ch].step[advStepNo].faultLowerTemp){
					myCh->misc.errCnt[C_CNT_LOWER_AUX_TEMP_STEP]++;
					if(myCh->misc.errCnt[C_CNT_LOWER_AUX_TEMP_STEP] 
													>= MAX_ERROR_CNT_TEMP*4){
						myCh->misc.errCnt[C_CNT_LOWER_AUX_TEMP_STEP] = 0;
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_LOWER_CH_TEMP;
						myCh->op.code = code_convert(myCh->op.code, STEP_SAFETY); //step_safety
						Fault_Value_Check(myCh->misc.advStepNo, 
						myData->bData[bd].cData[ch].misc.AuxTemp_min,
						myTestCond->safety.faultLowerTemp,
						myTestCond->step[advStepNo].faultLowerTemp,
					    0, 0, 0, 0, 0, 0);
						rtn = FAULT_COND;
					}
				}else{
					myCh->misc.errCnt[C_CNT_LOWER_AUX_TEMP_STEP] = 0;
				}
				break;
			default: break;
		}
	}
	
	return rtn;
}

int tempPauseCheck(int bd, int ch, unsigned long advStepNo, unsigned char type)
{	//20190311 lyhw		
    int rtn = 0;

	myTestCond = &(myPs->testCond[bd][ch]);

	if(myData->bData[bd].cData[ch].ChAttribute.chNo_master == P0) {
		ch = ch-1;
	}

	switch(type){
		case P0:	//safety step fault Upper Temperature
			if((myData->bData[bd].cData[ch].op.temp
				>= myTestCond->step[advStepNo].pauseUpperTemp)
				|| (myData->AnalogMeter.config.multiNum == 2
					&& (myData->bData[bd].cData[ch].op.temp1
					>= myTestCond->step[advStepNo].pauseUpperTemp))){
				myCh->misc.errCnt[C_CNT_UPPER_TEMP_PAUSE_STEP]++;
				if(myCh->misc.errCnt[C_CNT_UPPER_TEMP_PAUSE_STEP] 
												>= MAX_ERROR_CNT_TEMP * 4){
					myCh->misc.errCnt[C_CNT_UPPER_TEMP_PAUSE_STEP] = 0;
					myCh->misc.tmpState = myCh->op.state;
					myCh->misc.tmpCode = myCh->op.code;
					myCh->op.code = C_PAUSE_UPPER_TEMP_CH;
					rtn = FAULT_COND;
				}
			}else{
				myCh->misc.errCnt[C_CNT_UPPER_TEMP_PAUSE_STEP] = 0;
			}
			break;
		case P1:	//safety step fault Lower Temperature
			if((myData->bData[bd].cData[ch].op.temp
				<= myTestCond->step[advStepNo].pauseLowerTemp)
				|| (myData->AnalogMeter.config.multiNum == 2
					&& (myData->bData[bd].cData[ch].op.temp1
					<= myTestCond->step[advStepNo].pauseLowerTemp))){
				myCh->misc.errCnt[C_CNT_LOWER_TEMP_PAUSE_STEP]++;
				if(myCh->misc.errCnt[C_CNT_LOWER_TEMP_PAUSE_STEP] 
												>= MAX_ERROR_CNT_TEMP * 4){
					myCh->misc.errCnt[C_CNT_LOWER_TEMP_PAUSE_STEP] = 0;
					myCh->misc.tmpState = myCh->op.state;
					myCh->misc.tmpCode = myCh->op.code;
					myCh->op.code = C_PAUSE_LOWER_TEMP_CH;
					rtn = FAULT_COND;
				}
			}else{
				myCh->misc.errCnt[C_CNT_LOWER_TEMP_PAUSE_STEP] = 0;
			}
			break;
		default: break;
	}		
	return rtn;
}

//20180314 sch modify
int Fault_Check_default_LG(int bd, int ch)
{
   	int	rtn = 0;
	#ifdef _USER_VI
	int rangeV, rangeI, diff_I, diff_V, mode;
	int LimitBD = 0;
	int	LimitCH = 0;
	long maxV, maxI, type, LimitI=0, LimitV=0, SensI=0, SensV=0;

	S_CH_STEP_INFO step;
	
	step = step_info(bd, ch);

	rangeV = step.rangeV;
	rangeI = step.rangeI;
	type = step.type;
	mode = step.mode;

	maxV = myPs->config.maxVoltage[rangeV];
	maxI = myPs->config.maxCurrent[rangeI];
		
	if(myCh->ChAttribute.chNo_master == P0){
		SensI = myData->bData[bd].cData[ch].op.Isens
				+ myData->bData[bd].cData[ch-1].op.Isens;
		SensV = myData->bData[bd].cData[ch].op.Vsens;
		LimitI = myPs->config.LimitVI.limit_current - maxI*0.004;
		LimitV = myPs->config.LimitVI.limit_voltage;
	}else if(myCh->ChAttribute.opType == P0){ 
		SensI = myData->bData[bd].cData[ch].op.Isens;
		SensV = myData->bData[bd].cData[ch].op.Vsens;
		LimitI = myPs->config.LimitVI.limit_current - maxI*0.002;
		LimitV = myPs->config.LimitVI.limit_voltage;
	}

	if(type == STEP_DISCHARGE){
		SensI = labs(SensI);
	}
	if(myPs->config.LimitVI.limit_use_I != 0){
		if(SensI >= LimitI){
			diff_I = myCh->op.runTime - myCh->misc.limit_current_timeout;
			if(diff_I >= myPs->config.LimitVI.limit_time_I){
				myCh->misc.limit_current_timeout = myCh->op.runTime;
				rtn = FAULT_COND;
				if(myPs->config.LimitVI.limit_Ch_I == P0){
					if(myCh->op.state == C_RUN 
						&& myCh->signal[C_SIG_LIMIT_CURRENT] == P0){
						myCh->signal[C_SIG_LIMIT_CURRENT] = P1;
					}
				}else if(myPs->config.LimitVI.limit_Ch_I == P1){
					for(LimitBD=0; LimitBD<myPs->config.installedBd; LimitBD++){
						for(LimitCH=0; LimitCH<myPs->config.installedCh; LimitCH++){
							if((myPs->config.chPerBd * LimitBD + LimitCH)
								>= myPs->config.installedCh){
								continue;
							}
							if(myData->bData[LimitBD].cData[LimitCH].op.state 
								== C_RUN && myData->bData[LimitBD]
								.cData[LimitCH].signal[C_SIG_LIMIT_CURRENT] 
								== P0){
								myData->bData[LimitBD].cData[LimitCH]
										.signal[C_SIG_LIMIT_CURRENT] = P1;
							}
						}
					}
				}
			}
		}else{
			myData->bData[bd].cData[ch].misc.limit_current_timeout 
					= myData->bData[bd].cData[ch].op.runTime;
		}
	}
	
	if(myPs->config.LimitVI.limit_use_V != 0){
		if(SensV >= LimitV){
			//210831 LJS
			diff_V = myCh->op.runTime - myCh->misc.limit_voltage_timeout;
			if(diff_V >= myPs->config.LGES_fault_config.limit_time_v){
				myCh->misc.limit_voltage_timeout = myCh->op.runTime;
				rtn = FAULT_COND;
				if(myPs->config.LimitVI.limit_Ch_V == P0){
					if(myCh->op.state == C_RUN 
						&& myCh->signal[C_SIG_LIMIT_VOLTAGE] == P0){
						myCh->signal[C_SIG_LIMIT_VOLTAGE] = P1;
					}
				}else if(myPs->config.LimitVI.limit_Ch_V == P1){
					for(LimitBD=0; LimitBD<myPs->config.installedBd; LimitBD++){
						for(LimitCH=0; LimitCH<myPs->config.installedCh; 
																	LimitCH++){
							if((myPs->config.chPerBd * LimitBD + LimitCH)
								>= myPs->config.installedCh){
								continue;
							}
							if(myData->bData[LimitBD].cData[LimitCH].op.state 
								== C_RUN && myData->bData[LimitBD]
								.cData[LimitCH].signal[C_SIG_LIMIT_VOLTAGE] 
								== P0){
								myData->bData[LimitBD].cData[LimitCH]
										.signal[C_SIG_LIMIT_VOLTAGE] = P1;
							}
						}
					}
				}
			}
		}else{
			//210831 LJS
			myData->bData[bd].cData[ch].misc.limit_voltage_timeout
					= myData->bData[bd].cData[ch].op.runTime;
		}
	}
	#endif
	return rtn;
}

int Fault_Check_Change_VI(int bd, int ch, unsigned long advStepNo)
{	//190901 lyhw
	int rtn = 0;
#if CHANGE_VI_CHECK == 1
	int i;
	unsigned char type, F_Type, change_V_flag = 0;
	long V_upper, V_lower, change_V, SensI = 0, SensV =0;

	myCh = &(myData->bData[bd].cData[ch]);
	myTestCond = &(myPs->testCond[bd][ch]);

	type = myCh->op.type;
	
	myCh->misc.change_V_timer = myTestCond->step[advStepNo].change_V_time;
	V_upper = myTestCond->step[advStepNo].change_V_upper;
	V_lower = myTestCond->step[advStepNo].change_V_lower;
	F_Type = myData->mData.config.function[F_CHANGE_VI_CHECK];
	
	if(myCh->ChAttribute.chNo_master == 0){
		SensV = myData->bData[bd].cData[ch].op.Vsens;
		SensI = myData->bData[bd].cData[ch].op.Isens
				+ myData->bData[bd].cData[ch-1].op.Isens;
	}else if(myCh->ChAttribute.opType == 0){
		SensV = myData->bData[bd].cData[ch].op.Vsens;
		SensI = myData->bData[bd].cData[ch].op.Isens;
	}
	
	if(type == STEP_DISCHARGE){
		SensI = labs(SensI);
	}
	
	switch(F_Type){
		case P1:	//Hyundae
			if(type != STEP_CHARGE) break;		//just Charge Step
			//1. Check Change V Value
			if(myCh->misc.change_V_timer != 0){
				if(myCh->op.runTime % myCh->misc.change_V_timer == 0){
					myCh->misc.change_V = SensV - myCh->misc.Pre_change_V;
					myCh->misc.Pre_change_V = SensV;

					if(myTestCond->step[advStepNo].change_V != 0){
						if(myCh->misc.change_V 
									<= myTestCond->step[advStepNo].change_V){
							change_V_flag = P1;
						}else{
							change_V_flag = P0;
						}
					}
				}else{
					change_V_flag = P0;
				}
			}
			
			if(change_V_flag == P1){
				if(V_upper != 0 && V_lower != 0){
					if((SensV >= V_upper) && (SensV <= V_lower)){
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_CHANGE_V;
						rtn = FAULT_COND;
						return rtn;
					}
				}else if(V_upper != 0 && V_lower == 0){
					if(SensV >= V_upper){
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_CHANGE_V;
						rtn = FAULT_COND;
						return rtn;
					}	
				}else if(V_upper == 0 && V_lower != 0){
					if(SensV <= V_lower){
						myCh->misc.tmpState = myCh->op.state;
						myCh->misc.tmpCode = myCh->op.code;
						myCh->op.code = C_FAULT_CHANGE_V;
						rtn = FAULT_COND;
						return rtn;
					}
				}
			}
			
			for(i=0; i < MAX_CHK_VI_POINT; i++){
				if(myTestCond->step[advStepNo].chk_V_time[i] != 0){
					myCh->misc.chk_V_time[i]
						= myTestCond->step[advStepNo].chk_V_time[i];
					myCh->misc.chk_V_upper[i]
						= myTestCond->step[advStepNo].chk_V_upper[i];
					myCh->misc.chk_V_lower[i]
						= myTestCond->step[advStepNo].chk_V_lower[i];
				}else{
					myCh->misc.chk_V_time[i] = 0;
					myCh->misc.chk_V_upper[i] = 0;
					myCh->misc.chk_V_lower[i] = 0;
				}
		
				if(myCh->misc.chk_V_time[i] != 0){
					if(myCh->misc.chk_V_time[i] <= myCh->op.runTime){
						if((myCh->op.runTime - myCh->misc.chk_V_time[i])<=100){
							if((myCh->misc.chk_V_upper[i] != 0)
								&& (myCh->misc.chk_V_lower[i] != 0)){
								if((myCh->misc.chk_V_upper[i] <= SensV)
								|| (myCh->misc.chk_V_lower[i] >= SensV)){
									myCh->misc.tmpState = myCh->op.state;
									myCh->misc.tmpCode = myCh->op.code;
									myCh->op.code = C_FAULT_CHK_VOLTAGE;
									rtn = FAULT_COND;
									return rtn;
								}		
							}else if((myCh->misc.chk_V_upper[i] != 0) 
								&& (myCh->misc.chk_V_lower[i] == 0)){ 
								if(myCh->misc.chk_V_upper[i] <= SensV){
									myCh->misc.tmpState = myCh->op.state;
									myCh->misc.tmpCode = myCh->op.code;
									myCh->op.code = C_FAULT_CHK_VOLTAGE;
									rtn = FAULT_COND;
									return rtn;
								}
							}else if(myCh->misc.chk_V_upper[i] == 0
								&& (myCh->misc.chk_V_lower[i] != 0)){//191023
								if(myCh->misc.chk_V_lower[i] >= SensV){
									myCh->misc.tmpState = myCh->op.state;
									myCh->misc.tmpCode = myCh->op.code;
									myCh->op.code = C_FAULT_CHK_VOLTAGE;
									rtn = FAULT_COND;
									return rtn;
								}
							}else{
								myCh->misc.chk_V_upper[i] = 0;
								myCh->misc.chk_V_lower[i] = 0;
							}	
						}
					}
				}else{
					myCh->misc.chk_V_time[i] = 0;
				}
			}
	
			for(i=0; i < MAX_CHK_VI_POINT; i++){
				if(myTestCond->step[advStepNo].chk_I_time[i] != 0){
					myCh->misc.chk_I_time[i]
						= myTestCond->step[advStepNo].chk_I_time[i];
					myCh->misc.chk_I_upper[i]
						= myTestCond->step[advStepNo].chk_I_upper[i];
					myCh->misc.chk_I_lower[i]
						= myTestCond->step[advStepNo].chk_I_lower[i];
				}else{
					myCh->misc.chk_I_time[i] = 0;
					myCh->misc.chk_I_upper[i] = 0;
					myCh->misc.chk_I_lower[i] = 0;
				}
		
				if((myCh->misc.cvFlag == P1) 
								&& (myCh->misc.chk_I_time[i] != 0)){
					if(myCh->misc.chk_I_time[i] <= myCh->misc.cvTime){
						if((myCh->misc.cvTime - myCh->misc.chk_I_time[i])<=100){
							if((myCh->misc.chk_I_upper[i] != 0)
								&& (myCh->misc.chk_I_lower[i] != 0)){
								if((myCh->misc.chk_I_upper[i] <= SensI)//191023
									||(myCh->misc.chk_I_lower[i] >= SensI)){
									myCh->misc.tmpState = myCh->op.state;
									myCh->misc.tmpCode = myCh->op.code;
									myCh->op.code = C_FAULT_CHK_CURRENT;
									rtn = FAULT_COND;
									return rtn;
								}		
							}else if((myCh->misc.chk_I_upper[i] != 0) 
								&& (myCh->misc.chk_I_lower[i] == 0)){ 
								if(myCh->misc.chk_I_upper[i] <= SensI){
									myCh->misc.tmpState = myCh->op.state;
									myCh->misc.tmpCode = myCh->op.code;
									myCh->op.code = C_FAULT_CHK_CURRENT;
									rtn = FAULT_COND;
									return rtn;
								}
							}else if(myCh->misc.chk_I_upper[i] == 0 //191023
								&& (myCh->misc.chk_I_lower[i] != 0)){
								if(myCh->misc.chk_I_lower[i] >= SensI){
									myCh->misc.tmpState = myCh->op.state;
									myCh->misc.tmpCode = myCh->op.code;
									myCh->op.code = C_FAULT_CHK_CURRENT;
									rtn = FAULT_COND;
									return rtn;
								}
							}else{
								myCh->misc.chk_I_upper[i] = 0;
								myCh->misc.chk_I_lower[i] = 0;
							}
						}
					}
				}else{
					myCh->misc.chk_I_time[i] = 0;
				}
			}
			break;
		case P2:	//LG
			if(myCh->misc.change_V_timer == 0) break;
			if(myTestCond->step[advStepNo].change_V == 0) break;
			if(myCh->op.checkDelayTime <= 100) break;
			if(type == STEP_OCV) break;
			
			if(myCh->op.runTime % myCh->misc.change_V_timer == 0){
				myCh->misc.change_V = SensV - myCh->misc.Pre_change_V;
				myCh->misc.Pre_change_V = SensV;
				change_V = labs(myCh->misc.change_V);
				
				if(myTestCond->step[advStepNo].change_V <= change_V){
					change_V_flag = P1;
				}else{
					change_V_flag = P0;
				}
			}else{
				change_V_flag = P0;
			}
			
			if(change_V_flag == P1){
				myCh->misc.tmpState = myCh->op.state;
				myCh->misc.tmpCode = myCh->op.code;
				myCh->op.code = C_FAULT_CHANGE_V;
				rtn = FAULT_COND;
				return rtn;
			}else{
				rtn = 0;
				return rtn;
			}
			break;
		default:
			break;
	}
#endif
	return rtn;
}

//210923 lyhw add for lges gas data control
int Fault_Check_Gas_Data(int bd, int ch, unsigned long advStepNo)
{	
	int rtn = 0;
#if GAS_DATA_CONTROL == 1
	myCh = &(myData->bData[bd].cData[ch]);
	myTestCond = &(myPs->testCond[bd][ch]);
	
	if(myTestCond->step[advStepNo].faultUpper_GasTVOC != 0){
		if(myCh->misc.gas_TVOC
			>= myTestCond->step[advStepNo].faultUpper_GasTVOC){
			myCh->misc.errCnt[C_CNT_UPPER_GAS_TVOC_STEP]++;
			if(myCh->misc.errCnt[C_CNT_UPPER_GAS_TVOC_STEP] >= MAX_ERROR_CNT){
				myCh->misc.errCnt[C_CNT_UPPER_GAS_TVOC_STEP] = 0;
				myCh->misc.tmpState = myCh->op.state;
				myCh->misc.tmpCode = myCh->op.code;
				myCh->op.code = C_FAULT_UPPER_GAS_TVOC;
				rtn = FAULT_COND;
			}
		}else{
			myCh->misc.errCnt[C_CNT_UPPER_GAS_TVOC_STEP] = 0;
		}
	}
	
	if(myTestCond->step[advStepNo].faultLower_GasTVOC != 0){
		if(myCh->misc.gas_TVOC
			<= myTestCond->step[advStepNo].faultLower_GasTVOC){
			myCh->misc.errCnt[C_CNT_LOWER_GAS_TVOC_STEP]++;
			if(myCh->misc.errCnt[C_CNT_LOWER_GAS_TVOC_STEP] >= MAX_ERROR_CNT){
				myCh->misc.errCnt[C_CNT_LOWER_GAS_TVOC_STEP] = 0;
				myCh->misc.tmpState = myCh->op.state;
				myCh->misc.tmpCode = myCh->op.code;
				myCh->op.code = C_FAULT_LOWER_GAS_TVOC;
				rtn = FAULT_COND;
			}
		}else{
			myCh->misc.errCnt[C_CNT_LOWER_GAS_TVOC_STEP] = 0;
		}
	}
	
	if(myTestCond->step[advStepNo].faultUpper_GasECo2 != 0){
		if(myCh->misc.gas_eCo2
			>= myTestCond->step[advStepNo].faultUpper_GasECo2){
			myCh->misc.errCnt[C_CNT_UPPER_GAS_ECO2_STEP]++;
			if(myCh->misc.errCnt[C_CNT_UPPER_GAS_ECO2_STEP] >= MAX_ERROR_CNT){
				myCh->misc.errCnt[C_CNT_UPPER_GAS_ECO2_STEP] = 0;
				myCh->misc.tmpState = myCh->op.state;
				myCh->misc.tmpCode = myCh->op.code;
				myCh->op.code = C_FAULT_UPPER_GAS_ECO2;
				rtn = FAULT_COND;
			}
		}else{
			myCh->misc.errCnt[C_CNT_UPPER_GAS_ECO2_STEP] = 0;
		}
	}

	if(myTestCond->step[advStepNo].faultLower_GasECo2 != 0){
		if(myCh->misc.gas_eCo2
			<= myTestCond->step[advStepNo].faultLower_GasECo2){
			myCh->misc.errCnt[C_CNT_LOWER_GAS_ECO2_STEP]++;
			if(myCh->misc.errCnt[C_CNT_LOWER_GAS_ECO2_STEP] >= MAX_ERROR_CNT){
				myCh->misc.errCnt[C_CNT_LOWER_GAS_ECO2_STEP] = 0;
				myCh->misc.tmpState = myCh->op.state;
				myCh->misc.tmpCode = myCh->op.code;
				myCh->op.code = C_FAULT_LOWER_GAS_ECO2;
				rtn = FAULT_COND;
			}
		}else{
			myCh->misc.errCnt[C_CNT_LOWER_GAS_ECO2_STEP] = 0;
		}
	}
#endif
	return rtn;
}

int Fault_Check_Rest_Voltage(int bd, int ch)
{	//210518 hunw
	int rtn, restFaultCount, CheckNum;
   	unsigned long tmpVal; 
	unsigned long restStart_CheckTime;
	long	restCompareV, restCompareV_D_1, restCompareV_D_2;
	
	myCh = &(myData->bData[bd].cData[ch]);
	
	tmpVal = CheckNum = rtn = 0;
	//CheckNum = rtn = 0;
	restStart_CheckTime 
		= myData->mData.config.swFaultConfig[REST_CHECK_START_TIME] * 100;
	restCompareV 
		= myData->mData.config.swFaultConfig[REST_START_COMPARE_VOLTAGE];
	restCompareV_D_1 
		= myData->mData.config.swFaultConfig[REST_COMPARE_VOLTAGE_DELTA_V1];
	restCompareV_D_2 
		= myData->mData.config.swFaultConfig[REST_COMPARE_VOLTAGE_DELTA_V2];
	restFaultCount 
		=  myData->mData.config.swFaultConfig[REST_FAULT_CHECK_COUNT];
	
	
	if(restStart_CheckTime < 200) return rtn; 
	
	if(myCh->misc.restCheckStartFlag == 0){
		if(myCh->op.checkDelayTime >= (restStart_CheckTime - 200)){
			CheckNum = myCh->misc.restCheckNum;
			myCh->misc.restCheck_preV[CheckNum] = myCh->op.Vsens;
			myCh->misc.restCheckNum++;
			if(myCh->misc.restCheckNum >= 20){
				myCh->misc.restCheckStartFlag = P1;
				myCh->misc.restCheckNum = 0;
			}
		}
	}


	
	if(myCh->misc.restCheckStartFlag == P1){
		CheckNum = myCh->misc.restCheckNum;

		tmpVal = labs(myCh->op.Vsens - myCh->misc.restCheck_preV[CheckNum]); 
		myCh->misc.restCheck_preV[CheckNum] = myCh->op.Vsens;
		
		myCh->misc.restCheckNum++;
		if(myCh->misc.restCheckNum >= 20)	myCh->misc.restCheckNum = 0;
		
		if(myCh->misc.restCheckFlag == 0){
			if(myCh->op.Vsens >= restCompareV){
				myCh->misc.restCheckFlag = 1;
			}else{
				myCh->misc.restCheckFlag = 2;
			}
		}
				
		if(myCh->misc.restCheckFlag == 1){
			if(tmpVal >= restCompareV_D_1){
				myCh->misc.restFaultCount++;
			}
		}
		
		if(myCh->misc.restCheckFlag == 2){
			if(tmpVal >= restCompareV_D_2){
				myCh->misc.restFaultCount++;
			}
		}
	}
	
	if(myCh->misc.restFaultCount > restFaultCount){
		myCh->misc.tmpState = myCh->op.state;
		myCh->misc.tmpCode = myCh->op.code;
		myCh->op.code = C_FAULT_REST_VOLTAGE_ERROR;
		rtn = FAULT_COND;
	}

	return rtn;
}

void Fault_Value_Check(long val1, long val2, long val3, long val4, long val5, long val6, long val7, long val8, long val9, long val10)
{
#ifdef _SDI_SAFETY_V2	
 	myCh->misc.FaultValFlag = 1;
	myCh->misc.FaultVal[0] = val1;
 	myCh->misc.FaultVal[1] = val2;
 	myCh->misc.FaultVal[2] = val3;
 	myCh->misc.FaultVal[3] = val4;
 	myCh->misc.FaultVal[4] = val5;
 	myCh->misc.FaultVal[5] = val6;
 	myCh->misc.FaultVal[6] = val7;
 	myCh->misc.FaultVal[7] = val8;
 	myCh->misc.FaultVal[8] = val9;
 	myCh->misc.FaultVal[9] = val10;
#endif
}

int code_convert(int code, int flag)
{
	#ifdef _CH_CODE_CONVERT
	//SAFETY : 2000
	//STEP_SAFETY : 1000
	switch(flag){
		case SAFETY:
			code = code + SAFETY;
			break;
		case STEP_SAFETY:
			code = code + STEP_SAFETY;
			break;
		default:
			break;
	}
	#endif
	return code;
}

//210126 lyhw for Can ch errorCode Check
int errorCodeCheck_CAN(int bd, int ch, int rtn) 
{
	int errorCode;
	if(CYCLER_TYPE != 2) return rtn;
	//210126 lyhw Check
	if(rtn == FAULT_COND) return rtn;
	
	myCh = &(myData->bData[bd].cData[ch]);

	errorCode = myCh->misc.can_error;
	
	switch(errorCode){
		case CAN_MAIN_CH_HARD_ERR:
			myCh->misc.tmpState = myCh->op.state;
			myCh->misc.tmpCode = myCh->op.code;
			myCh->op.code = C_FAULT_CH_CAN_HARD_ERR;
			rtn = FAULT_COND;
			break;
		case CAN_MAIN_CH_LOU_ERR:
			myCh->misc.tmpState = myCh->op.state;
			myCh->misc.tmpCode = myCh->op.code;
			myCh->op.code = C_FAULT_CH_CAN_LOU_ERR;
			rtn = FAULT_COND;
			break;
		case CAN_MAIN_CH_OTP_ERR:
			myCh->misc.tmpState = myCh->op.state;
			myCh->misc.tmpCode = myCh->op.code;
			myCh->op.code = C_FAULT_CH_CAN_OTP_ERR;
			rtn = FAULT_COND;
			break;
		case CAN_MAIN_CH_VOL_OUT_ERR:
			myCh->misc.tmpState = myCh->op.state;
			myCh->misc.tmpCode = myCh->op.code;
			myCh->op.code = C_FAULT_CH_CAN_VOL_OUT_ERR;
			rtn = FAULT_COND;
			break;
		case CAN_MAIN_CH_BUS_ERR:
			myCh->misc.tmpState = myCh->op.state;
			myCh->misc.tmpCode = myCh->op.code;
			myCh->op.code = C_FAULT_CH_CAN_BUS_ERR;
			rtn = FAULT_COND;
			break;
		default:
			break;
	}
	
	return rtn;
}

int SDI_CC_CV_hump_Check(int bd, int ch, int rtn, int stepType)
{
	long sum_Isens = 0;
	long charge_voltage = 0;
	long charge_current = 0;
	long charge_cc_start_time = 0;
	long charge_cv_start_time = 0;
	long charge_cc_period_time = 0;
	long charge_cv_period_time = 0;
	long discharge_voltage = 0;
	long discharge_current = 0;
	long discharge_cc_start_time = 0;
	long discharge_cv_start_time = 0;
	long discharge_cc_period_time = 0;
	long discharge_cv_period_time = 0;

	if(VENDER != 2) return rtn;
	if(rtn == FAULT_COND) return rtn;
	if(myData->mData.config.cc_cv_hump_flag == 0) return rtn;
	
	if(myCh->ChAttribute.chNo_master == P0) {
		sum_Isens = myCh->op.Isens + myData->bData[bd].cData[ch-1].op.Isens;
	}

	charge_voltage = myData->mData.config.SDI_cc_cv_hump.charge_voltage * 1000; //(mV)
	charge_current = myData->mData.config.SDI_cc_cv_hump.charge_current; //(uA)
	discharge_voltage = myData->mData.config.SDI_cc_cv_hump.discharge_voltage * 1000;	//(mV)
	discharge_current = myData->mData.config.SDI_cc_cv_hump.discharge_current;	//(uA)
	charge_cc_start_time = myData->mData.config.SDI_cc_cv_hump.charge_cc_start_time * 100;	//(sec)
	charge_cv_start_time = myData->mData.config.SDI_cc_cv_hump.charge_cv_start_time * 100;	//(sec)
	charge_cc_period_time = myData->mData.config.SDI_cc_cv_hump.charge_cc_period_time * 100;	//(sec)
	charge_cv_period_time = myData->mData.config.SDI_cc_cv_hump.charge_cv_period_time * 100;	//(sec)
	discharge_cc_start_time = myData->mData.config.SDI_cc_cv_hump.discharge_cc_start_time * 100;	//(sec)
	discharge_cv_start_time = myData->mData.config.SDI_cc_cv_hump.discharge_cv_start_time * 100;	//(sec)
	discharge_cc_period_time = myData->mData.config.SDI_cc_cv_hump.discharge_cc_period_time * 100;	//(sec)
	discharge_cv_period_time = myData->mData.config.SDI_cc_cv_hump.discharge_cv_period_time * 100;	//(sec)
	
	if(myCh->ChAttribute.chNo_master == P0) {
		switch(stepType) {
			case STEP_CHARGE:
				if(myData->bData[bd].cData[ch-1].misc.cvFaultCheckFlag == P0) { //CC
					if(charge_cc_period_time != 0){
						if(myCh->misc.charge_cc_hump_flag == P0){
							if(charge_cc_start_time == 0){
								myCh->misc.charge_cc_hump_flag = P1;
								myCh->misc.charge_cc_hump_start_time = myCh->misc.ccTime;	
								myCh->misc.charge_cc_hump_start_voltage = myCh->op.Vsens;	
							}else if(myCh->misc.ccTime - myCh->misc.charge_cc_hump_start_time >= charge_cc_start_time){
								myCh->misc.charge_cc_hump_flag = P1;
								myCh->misc.charge_cc_hump_start_time = myCh->misc.ccTime;	
								myCh->misc.charge_cc_hump_start_voltage = myCh->op.Vsens;	
							}
						}
						if(myCh->misc.charge_cc_hump_flag == P1){ 
							if(myCh->misc.ccTime - myCh->misc.charge_cc_hump_start_time >= charge_cc_period_time){
								myCh->misc.charge_cc_hump_start_time = myCh->misc.ccTime;	
								if(myCh->misc.charge_cc_hump_start_voltage - myCh->op.Vsens >= charge_voltage){
									myCh->misc.tmpState = myCh->op.state;
									myCh->misc.tmpCode = myCh->op.code;
									myCh->op.code = C_FAULT_CHARGE_CC_VOLTAGE_HUMP;
									Fault_Value_Check(myCh->misc.charge_cc_hump_start_time,charge_cc_period_time,
									 myCh->misc.charge_cc_hump_start_voltage,myCh->op.Vsens,charge_voltage,0,0,0,0,0);
									rtn = FAULT_COND;
									return rtn;
								}else{
									myCh->misc.charge_cc_hump_start_voltage = myCh->op.Vsens;	
								}
							}
						}
					}	
				}else if(myData->bData[bd].cData[ch-1].misc.cvFaultCheckFlag == P1) { //CV
					if(charge_cv_period_time != 0){
						if(myCh->misc.charge_cv_hump_flag == P0){
							if(charge_cv_start_time == 0){ 
								myCh->misc.charge_cv_hump_flag = P1;
								myCh->misc.charge_cv_hump_start_time = myCh->misc.cvTime;	
								myCh->misc.charge_cv_hump_start_current = sum_Isens;	
							}else if(myCh->misc.cvTime - myCh->misc.charge_cv_hump_start_time >= charge_cv_start_time){
								myCh->misc.charge_cv_hump_flag = P1;
								myCh->misc.charge_cv_hump_start_time = myCh->misc.cvTime;	
								myCh->misc.charge_cv_hump_start_current = sum_Isens;	
							}
						}
						if(myCh->misc.charge_cv_hump_flag == P1){
							if(myCh->misc.cvTime - myCh->misc.charge_cv_hump_start_time >= charge_cv_period_time){
								myCh->misc.charge_cv_hump_start_time = myCh->misc.cvTime;	
								if(myCh->misc.charge_cv_hump_start_current - sum_Isens <= charge_current){
									myCh->misc.tmpState = myCh->op.state;
									myCh->misc.tmpCode = myCh->op.code;
									myCh->op.code = C_FAULT_CHARGE_CV_CURRENT_HUMP;
									Fault_Value_Check(myCh->misc.charge_cv_hump_start_time,charge_cv_period_time,
									 myCh->misc.charge_cv_hump_start_current,myCh->op.Isens,charge_current,0,0,0,0,0);
									rtn = FAULT_COND;
									return rtn;
								}else{
									myCh->misc.charge_cv_hump_start_current = sum_Isens;	
								}
							}
						}
					}
				}
				break;
			case STEP_DISCHARGE:
				if(myData->bData[bd].cData[ch-1].misc.cvFaultCheckFlag == P0) { //CC
					if(discharge_cc_period_time != 0){
						if(myCh->misc.discharge_cc_hump_flag == P0){
							if(discharge_cc_start_time == 0){
								myCh->misc.discharge_cc_hump_flag = P1;
								myCh->misc.discharge_cc_hump_start_time = myCh->misc.ccTime;	
								myCh->misc.discharge_cc_hump_start_voltage = myCh->op.Vsens;	
							}else if(myCh->misc.ccTime - myCh->misc.discharge_cc_hump_start_time >= discharge_cc_start_time){
								myCh->misc.discharge_cc_hump_flag = P1;
								myCh->misc.discharge_cc_hump_start_time = myCh->misc.ccTime;	
								myCh->misc.discharge_cc_hump_start_voltage = myCh->op.Vsens;	
							}
						}
						if(myCh->misc.discharge_cc_hump_flag == P1){
							if(myCh->misc.ccTime - myCh->misc.discharge_cc_hump_start_time >= discharge_cc_period_time){
								myCh->misc.discharge_cc_hump_start_time = myCh->misc.ccTime;	
								if(myCh->misc.discharge_cc_hump_start_voltage - myCh->op.Vsens <= discharge_voltage){
									myCh->misc.tmpState = myCh->op.state;
									myCh->misc.tmpCode = myCh->op.code;
									myCh->op.code = C_FAULT_DISCHARGE_CC_VOLTAGE_HUMP;
									Fault_Value_Check(myCh->misc.discharge_cc_hump_start_time,discharge_cc_period_time,
									 myCh->misc.discharge_cc_hump_start_voltage,myCh->op.Vsens,discharge_voltage,0,0,0,0,0);
									rtn = FAULT_COND;
									return rtn;
								}else{
									myCh->misc.discharge_cc_hump_start_voltage = myCh->op.Vsens;	
								}
							}
						}
					}	
				}else if(myData->bData[bd].cData[ch-1].misc.cvFaultCheckFlag == P1) { //CV
					if(discharge_cv_period_time != 0){
						if(myCh->misc.discharge_cv_hump_flag == P0){
							if(discharge_cv_start_time == 0){ 
								myCh->misc.discharge_cv_hump_flag = P1;
								myCh->misc.discharge_cv_hump_start_time = myCh->misc.cvTime;	
								myCh->misc.discharge_cv_hump_start_current = sum_Isens;	
							}else if(myCh->misc.cvTime - myCh->misc.discharge_cv_hump_start_time >= discharge_cv_start_time){
								myCh->misc.discharge_cv_hump_flag = P1;
								myCh->misc.discharge_cv_hump_start_time = myCh->misc.cvTime;	
								myCh->misc.discharge_cv_hump_start_current = sum_Isens;	
							}
						}
						if(myCh->misc.discharge_cv_hump_flag == P1){
							if(myCh->misc.cvTime - myCh->misc.discharge_cv_hump_start_time >= discharge_cv_period_time){
								myCh->misc.discharge_cv_hump_start_time = myCh->misc.cvTime;	
								if(myCh->misc.discharge_cv_hump_start_current - sum_Isens >= discharge_current){
									myCh->misc.tmpState = myCh->op.state;
									myCh->misc.tmpCode = myCh->op.code;
									myCh->op.code = C_FAULT_DISCHARGE_CV_CURRENT_HUMP;
									Fault_Value_Check(myCh->misc.discharge_cv_hump_start_time,discharge_cv_period_time,
									 myCh->misc.discharge_cv_hump_start_current,myCh->op.Isens,discharge_current,0,0,0,0,0);
									rtn = FAULT_COND;
									return rtn;
								}else{
									myCh->misc.discharge_cv_hump_start_current = sum_Isens;	
								}
							}
						}
					}
				}
				break;
			default:
				break;
		}
	}else if(myCh->ChAttribute.opType == P0) {
		switch(stepType) {
			case STEP_CHARGE:
				if(myCh->misc.cvFaultCheckFlag == P0) { //CC
					if(charge_cc_period_time != 0){
						if(myCh->misc.charge_cc_hump_flag == P0){
							if(charge_cc_start_time == 0){
								myCh->misc.charge_cc_hump_flag = P1;
								myCh->misc.charge_cc_hump_start_time = myCh->misc.ccTime;	
								myCh->misc.charge_cc_hump_start_voltage = myCh->op.Vsens;	
							}else if(myCh->misc.ccTime - myCh->misc.charge_cc_hump_start_time >= charge_cc_start_time){
								myCh->misc.charge_cc_hump_flag = P1;
								myCh->misc.charge_cc_hump_start_time = myCh->misc.ccTime;	
								myCh->misc.charge_cc_hump_start_voltage = myCh->op.Vsens;	
							}
						}
						if(myCh->misc.charge_cc_hump_flag == P1){
							if(myCh->misc.ccTime - myCh->misc.charge_cc_hump_start_time >= charge_cc_period_time){
								myCh->misc.charge_cc_hump_start_time = myCh->misc.ccTime;	
								if(myCh->misc.charge_cc_hump_start_voltage - myCh->op.Vsens >= charge_voltage){
									myCh->misc.tmpState = myCh->op.state;
									myCh->misc.tmpCode = myCh->op.code;
									myCh->op.code = C_FAULT_CHARGE_CC_VOLTAGE_HUMP;
									Fault_Value_Check(myCh->misc.charge_cc_hump_start_time,charge_cc_period_time,
									 myCh->misc.charge_cc_hump_start_voltage,myCh->op.Vsens,charge_voltage,0,0,0,0,0);
									rtn = FAULT_COND;
									return rtn;
								}else{
									myCh->misc.charge_cc_hump_start_voltage = myCh->op.Vsens;	
								}
							}
						}
					}	
				}else if(myCh->misc.cvFaultCheckFlag == P1) { //CV
					if(charge_cv_period_time != 0){
						if(myCh->misc.charge_cv_hump_flag == P0){
							if(charge_cv_start_time == 0){ 
								myCh->misc.charge_cv_hump_flag = P1;
								myCh->misc.charge_cv_hump_start_time = myCh->misc.cvTime;	
								myCh->misc.charge_cv_hump_start_current = myCh->op.Isens;	
							}else if(myCh->misc.cvTime - myCh->misc.charge_cv_hump_start_time >= charge_cv_start_time){
								myCh->misc.charge_cv_hump_flag = P1;
								myCh->misc.charge_cv_hump_start_time = myCh->misc.cvTime;	
								myCh->misc.charge_cv_hump_start_current = myCh->op.Isens;	
							}
						}
						if(myCh->misc.charge_cv_hump_flag == P1){
							if(myCh->misc.cvTime - myCh->misc.charge_cv_hump_start_time >= charge_cv_period_time){
								myCh->misc.charge_cv_hump_start_time = myCh->misc.cvTime;	
								if(myCh->misc.charge_cv_hump_start_current - myCh->op.Isens <= charge_current){
									myCh->misc.tmpState = myCh->op.state;
									myCh->misc.tmpCode = myCh->op.code;
									myCh->op.code = C_FAULT_CHARGE_CV_CURRENT_HUMP;
									Fault_Value_Check(myCh->misc.charge_cv_hump_start_time,charge_cv_period_time,
									 myCh->misc.charge_cv_hump_start_current,myCh->op.Isens,charge_current,0,0,0,0,0);
									rtn = FAULT_COND;
									return rtn;
								}else{
									myCh->misc.charge_cv_hump_start_current = myCh->op.Isens;	
								}
							}
						}
					}
				}
				break;
			case STEP_DISCHARGE:
				if(myCh->misc.cvFaultCheckFlag == P0) { //CC
					if(discharge_cc_period_time != 0){
						if(myCh->misc.discharge_cc_hump_flag == P0){
							if(discharge_cc_start_time == 0){
								myCh->misc.discharge_cc_hump_flag = P1;
								myCh->misc.discharge_cc_hump_start_time = myCh->misc.ccTime;	
								myCh->misc.discharge_cc_hump_start_voltage = myCh->op.Vsens;	
							}else if(myCh->misc.ccTime - myCh->misc.discharge_cc_hump_start_time >= discharge_cc_start_time){
								myCh->misc.discharge_cc_hump_flag = P1;
								myCh->misc.discharge_cc_hump_start_time = myCh->misc.ccTime;	
								myCh->misc.discharge_cc_hump_start_voltage = myCh->op.Vsens;	
							}
						}
						if(myCh->misc.discharge_cc_hump_flag == P1){
							if(myCh->misc.ccTime - myCh->misc.discharge_cc_hump_start_time >= discharge_cc_period_time){
								myCh->misc.discharge_cc_hump_start_time = myCh->misc.ccTime;	
								if(myCh->misc.discharge_cc_hump_start_voltage - myCh->op.Vsens <= discharge_voltage){
									myCh->misc.tmpState = myCh->op.state;
									myCh->misc.tmpCode = myCh->op.code;
									myCh->op.code = C_FAULT_DISCHARGE_CC_VOLTAGE_HUMP;
									Fault_Value_Check(myCh->misc.discharge_cc_hump_start_time,discharge_cc_period_time,
									 myCh->misc.discharge_cc_hump_start_voltage,myCh->op.Vsens,discharge_voltage,0,0,0,0,0);
									rtn = FAULT_COND;
									return rtn;
								}else{
									myCh->misc.discharge_cc_hump_start_voltage = myCh->op.Vsens;	
								}	
							}
						}
					}	
				}else if(myCh->misc.cvFaultCheckFlag == P1) { //CV
					if(discharge_cv_period_time != 0){
						if(myCh->misc.discharge_cv_hump_flag == P0){
							if(discharge_cv_start_time == 0){ 
								myCh->misc.discharge_cv_hump_flag = P1;
								myCh->misc.discharge_cv_hump_start_time = myCh->misc.cvTime;	
								myCh->misc.discharge_cv_hump_start_current = myCh->op.Isens;	
							}else if(myCh->misc.cvTime - myCh->misc.discharge_cv_hump_start_time >= discharge_cv_start_time){
								myCh->misc.discharge_cv_hump_flag = P1;
								myCh->misc.discharge_cv_hump_start_time = myCh->misc.cvTime;	
								myCh->misc.discharge_cv_hump_start_current = myCh->op.Isens;	
							}
						}
						if(myCh->misc.discharge_cv_hump_flag == P1){
							if(myCh->misc.cvTime - myCh->misc.discharge_cv_hump_start_time >= discharge_cv_period_time){
								myCh->misc.discharge_cv_hump_start_time = myCh->misc.cvTime;	
								if(myCh->misc.discharge_cv_hump_start_current - myCh->op.Isens >= discharge_current){
									myCh->misc.tmpState = myCh->op.state;
									myCh->misc.tmpCode = myCh->op.code;
									myCh->op.code = C_FAULT_DISCHARGE_CV_CURRENT_HUMP;
									Fault_Value_Check(myCh->misc.discharge_cv_hump_start_time,discharge_cv_period_time,
									 myCh->misc.discharge_cv_hump_start_current,myCh->op.Isens,discharge_current,0,0,0,0,0);
									rtn = FAULT_COND;
									return rtn;
								}else{
									myCh->misc.discharge_cv_hump_start_current = myCh->op.Isens;	
								}
							}
						}
					}
				}
				break;
			default:
				break;
		}
	}
	return rtn;
}

//220214 LJS For LGES semi-auto jig PLC safety Condition
#ifdef _GROUP_ERROR
int Group_Error_Fault_Check(int bd, int ch, unsigned long advStepNo)
{
	int rtn = 0;
	if(myCh->misc.stepSyncFlag == 0) return rtn;
	
	if(myTestCond->step[advStepNo].group_CheckTime != 0){
		if(myCh->op.runTime - myCh->misc.Std_Time >= 
				myTestCond->step[advStepNo].group_CheckTime){
			if(labs(myCh->misc.groupAvgVsens - myCh->op.Vsens) >= 
					myTestCond->step[advStepNo].group_DeltaVoltage){
				myCh->misc.tmpState = myCh->op.state;
				myCh->misc.tmpCode = myCh->op.code;
				myCh->op.code = C_FAULT_GROUP_VOLTAGE_ERROR;
				rtn = FAULT_COND;
			}else{
				myCh->misc.Fault_Check_flag = 0;
			}
		}
	}
	return rtn;	
}
#endif
