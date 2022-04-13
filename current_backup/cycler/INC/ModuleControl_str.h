#ifndef __MODULECONTROL_STR_H__
#define __MODULECONTROL_STR_H__

#include "ModuleControl_def.h"

#define _C_LGC_5V_600A_10A

typedef union u_adda_tag {
    short int 			val;
    unsigned char		byte[2];
} U_ADDA;

typedef struct s_pcu_state_data_tag {	//180611 lyhw for digital
	unsigned short int	errState : 4;
	unsigned short int	timeout  : 4;
	unsigned short int	checkSum : 4;
	unsigned short int	reserved1 	 : 2;
	unsigned short int	cmdSending	 : 1;
	unsigned short int	reserved2 	 : 1;
} S_PCU_STATE_DATA;

typedef union u_pcu_state_data_tag {
	S_PCU_STATE_DATA	data;
	unsigned short		pcu_state;
} U_PCU_STATE_DATA;

typedef struct s_ch_do_tag {  //LGC_5V_600A_6A
	unsigned char	o1 				: 1;
	unsigned char	o2 				: 1;
	unsigned char	o3 				: 1;
	unsigned char	o4 				: 1;
	unsigned char	o5			    : 1;
	unsigned char	o6 				: 1;
	unsigned char	o7 				: 1;
	unsigned char	o8 				: 1;
} S_CH_DO;

typedef union {
	S_CH_DO	io;
	unsigned char ch_io;
} S_CH_DO_DATA;

typedef struct s_ch_di_tag {
	unsigned char	i1 : 1;
	unsigned char	i2 : 1;
	unsigned char	i3 : 1;
	unsigned char	i4 : 1;
	unsigned char	i5 : 1;
	unsigned char	i6 : 1;
	unsigned char	i7 : 1;
	unsigned char	i8 : 1;
} S_CH_DI;

typedef union {
	S_CH_DI	io;
	unsigned char ch_io;
} S_CH_DI_DATA;
#if MACHINE_TYPE == 1
//140407 oys add start
typedef struct s_ch_short_do_tag {  //SDI 50V20A
	unsigned char	ready1 	: 1;
	unsigned char	run1 	: 1;
	unsigned char	com1	: 1;
	unsigned char	alram1 	: 1;
	unsigned char	ready2	: 1;
	unsigned char	run2 	: 1;
	unsigned char	com2 	: 1;
	unsigned char	alram2 	: 1;
} S_CH_SHORT_DO;

typedef union {
	S_CH_SHORT_DO	io;
	unsigned char ch_io;
} S_CH_SHORT_DO_DATA;

typedef struct s_ch_short_di_tag {
	unsigned char	ready 	: 1;
	unsigned char	run		: 1;
	unsigned char	com		: 1;
	unsigned char	alram 	: 1;
	unsigned char	spare1	: 1;
	unsigned char	spare2 	: 1;
	unsigned char	spare3 	: 1;
	unsigned char	spare4 	: 1;
} S_CH_SHORT_DI;

typedef union {
	S_CH_SHORT_DI	io;
	unsigned char ch_io;
} S_CH_SHORT_DI_DATA;

typedef struct s_ch_relay_do_tag {
	unsigned char	v_relay1 	: 1;
	unsigned char	v_relay2	: 1;
	unsigned char	i_relay1	: 1;
	unsigned char	i_relay2	: 1;
	unsigned char	spare1		: 1;
	unsigned char	spare2 		: 1;
	unsigned char	spare3 		: 1;
	unsigned char	spare4 		: 1;
} S_CH_RELAY_DO;

typedef union {
	S_CH_RELAY_DO	io;
	unsigned char ch_io;
} S_CH_RELAY_DO_DATA;
//add end
#endif

typedef struct s_ch_misc_tag {
	unsigned char		tmpState;
	char				CAN_PreCmd;
	unsigned char		canNextStepFlag;	//220117 jws add
	unsigned char		preDataType;
	
	unsigned short		tmpCode;
	unsigned char		semiSwitchState;
	unsigned char		delta_flag;

	unsigned char		tmpPhase;
	unsigned char		userPatternFlag;
	unsigned char		tempDir;
	unsigned char		nextDelay;
	
	long				startV;
	long				maxV;
	long				minV;
	long				maxI;
	long				minI;
	long				startT;

	long				maxT;
	long				minT;
// 150126 lyh add for rest Delta V
	long				compareV;
// 141208 oys add : SDI MES VER4	
	long				cycleMaxV;
	long				cycleMinV;
	long				cycleSumAvgT;
	long				cycleStartV;
	long				cycleAvgDischargeV;

	long				cycleAvgDischargeI;
	unsigned long		step_count;
	unsigned long		cycleStepCount;
	unsigned long		cycleDischargeStepCount;

	unsigned char		mes_data_flag;
	unsigned char		gradeProcFlag; //170728 oys add
	unsigned char		chGroupCheckFlag; //171125 oys add
	unsigned char		cycle_p_flag;		//190308 lyh add

	unsigned long		checkSentTime;
	short int			pulseDataCount;
	short int			save10msDataCount;

   	unsigned long		saveDt;
   	unsigned long		saveDv;
   	unsigned long		saveDi;
   	unsigned long		pulseDt;
	unsigned long		userPatternRunTime;

   	long				saveDtemp;
	long				ocv;
	long				actualCapacity;
	long				actualWattHour; //20190214 add

   	long				adValue[4][MAX_AD_COUNT];
   	long				sumCapacity;
   	long				seedCapacity;
   	long				sumWattHour;
   	long				seedWattHour;
	double				meanSumVolt;
	double				meanSumCurr;
	double				meanSumTemp;
	unsigned long		meanCount;

	unsigned long		advCycleStep;
	unsigned long		advCycle;
	unsigned long		advStepNo;
	unsigned long		currentCycle;
	unsigned long		totalCycle;

	unsigned short		cycleNo;
	unsigned char		parallel_cycle_phase; //kjg_180521
	unsigned char		parallel_sensFlag; //kjg_180523

	unsigned short		gotoCycleCount[MAX_STEP];

	long				charge_integralCap[MAX_CYCLE]; //pjy add for toshiba
	long				discharge_integralCap[MAX_CYCLE]; //pjy add for toshiba

	unsigned short		fbCountV_H;
	unsigned short		fbCountV_L;
	unsigned short		fbCountV_M;
	unsigned short		fbCountI_H;
	unsigned short		fbCountI_L;
	unsigned short		fbCountI_M;

	long				fbSumV_H;
	long				fbSumV_L;
	long				fbSumV_M;
	long				fbSumI_H;
	long				fbSumI_L;
	long				fbSumI_M;
	long				fbV;
	long				fbI;

	unsigned long		cvTime;
	unsigned long		ccTime;
	unsigned long		cycle_Charge_cvTime;
	unsigned long		cycle_Charge_ccTime;
	unsigned long		cycle_Discharge_cvTime;
	unsigned long		cycle_Discharge_ccTime;
	long				tmpVsens;
	long				tmpBufVsens[MAX_TMP_AD_COUNT];
	long				tmpCpVsens;
	long				tmpBufIsens[MAX_TMP_AD_COUNT];
	long				tmpIsens;
   	long				tmpWatt;

	long				tmpIsens2; //kjg_180521
	long				Isens2; //kjg_180521

	long				c_v1;
	long				c_v2;
	unsigned long		c_t1;
	unsigned long		c_t2;
	unsigned short		d_count;
	unsigned short		d_count_iec;
	unsigned short		d_flag;
	unsigned short		d_flag2;
	long				d_v[100];
	long				d_t[100];
	long				d_i[100];
	long				d_v_iec[MAX_PULSE_DATA_IEC];
	long				d_t_iec[MAX_PULSE_DATA_IEC];
	long				d_i_iec[MAX_PULSE_DATA_IEC];

	long				d_voltage;
	
	unsigned short		d_count2;	//220322_hun
	long				d_v2;	//220322_hun

	//20180206 sch add for capacitance iec & maxwell
	unsigned long		delta_t_iec[2];
	long				delta_v_iec[2];
	long				delta_w_iec[2];
	double				delta_sumI_iec;
	unsigned int		delta_cnt_iec;
	unsigned char		delta_flag_iec;
	unsigned char		cycle_p_sch_flag;	//190318 add lyhw
	unsigned char		grade_flag;
	unsigned char		reserved_cmd_flag;	//200317 add lyhw

	unsigned long		delta_t_maxwell[2];
	long				delta_v_maxwell[2];
	double				delta_sumI_maxwell;
	unsigned int		delta_cnt_maxwell;
	
	unsigned char		delta_flag_maxwell;
	unsigned char		restart_inv;
	unsigned char		equation_calc_err_flag;	//211111
	unsigned char		equation_range_flag;	//211111
	
	long				sensSumV[MAX_FILTER_AD_COUNT];
	long				sensSumI[MAX_FILTER_AD_COUNT];

	double				pid_ui1[MAX_TYPE];
	double				pid_error1[MAX_TYPE];

	long				cmd_v;
	long				cmd_i;
	short int			cmd_v_div;
	short int			cmd_i_div;
	short int			cmd_v_range;
	short int			cmd_i_range;
	
	unsigned short		sensCount;
	unsigned char		sensCountFlag;
	unsigned char		sensBufCount;
	
	unsigned char		sensBufCountFlag;
	unsigned char		statusLed;	
	unsigned char		preRangeI;
	unsigned char		reserved3;
	
	unsigned char		errCnt[MAX_SIGNAL];
	
	long				preVref;
	long				preIref;
	long				groupTemp;
	unsigned long		chamberStepNo;
	int					userPatternCnt;

	unsigned long		endCycleTime;
	unsigned long		endCycleTimeGotoStep;
	unsigned long		cycleRunTime;
	//120818 kji SDI mes cycle data
	long				sumChargeCCAh;
	long				seedChargeCCAh;
	long				chargeCCAh;
	long				cycleSumChargeCCAh;
	long				sumChargeCVAh;
	long				seedChargeCVAh;
	long				chargeCVAh;
	long				cycleSumChargeCVAh;
	long				sumDischargeCCAh;
	long				seedDischargeCCAh;
	long				dischargeCCAh;
	long				cycleSumDischargeCCAh;
	long				sumDischargeCVAh;
	long				seedDischargeCVAh;
	long				dischargeCVAh;
	long				cycleSumDischargeCVAh;
	long				sumChargeCCCVAh;
	long				seedChargeCCCVAh;
	long				chargeCCCVAh;
	long				sumDischargeCCCVAh;
	long				seedDischargeCCCVAh;
	long				dischargeCCCVAh;
	
	long				lastRestVsens;
	long				sumDischargeAmpareHour;
	long				seedDischargeAmpareHour;
	long				sumChargeAmpareHour;
	long				seedChargeAmpareHour;
	long				integralCapacity;
	long				sumintegralCapacity;
	long				seedintegralCapacity;
	long				integralWattHour;
	long				sumintegralWattHour;
	long				seedintegralWattHour;
	long				sumDischargeWattHour;
	long				sumChargeWattHour;
	long				cycleSumChargeWatt;
	long				cycleSumDischargeWatt;
	long				cycleSumDischargeWattHour;
	long				cycleSumChargeWattHour;
	long				cycleSumChargeAmpareHour;
	long				cycleSumDischargeAmpareHour;
	long				seedDischargeWattHour;
	long				seedChargeWattHour;
	long				endIntegralC;
	long				endIntegralWh;
	unsigned long		endIntegralCGotoStep;
	unsigned long		endIntegralWhGotoStep;
	long				endIntegralCFlag;
	long				endIntegralWhFlag;
	long				userPatternCntFlag;
	float				stdev_i;
	long				saveDtConfig;

	unsigned char		cmdV_dir;
	unsigned char		start;
	unsigned char		waitFlag;
	unsigned char       patternPhase;

	unsigned long		fadTotalCycle;
	unsigned long		fadStepNo;
	long				fadTimer;
	float				fadGainV;
	float				fadOffsetV;
	float				fadGainI;
	float				fadOffsetI;
//110215 kji	
	unsigned short		nDCRTime1_iec;
	unsigned short		nDCRTime2_iec;
	unsigned short		nAfterIncludeCnt_iec;
	unsigned short		nAfterExcludeCnt_iec;

	unsigned char		integralTGotoCheck;
	unsigned char		integralCGotoCheck;
	unsigned char		integralWhGotoCheck;
	unsigned char		integralInit;
//110402 kji	
	
	long				caliCheckSum;
	double				caliCheckSum1;
	
	unsigned char		setCaliNum;
	unsigned char		checkCaliNum;
	unsigned char		relayStartFlag;
	unsigned char		caliCheckPoint;
	
	unsigned char		fadFlag;
	unsigned char		virRangePhase;
	unsigned char		virRangeReservedNo;
	unsigned char		userMapFlag;	
	long				soc;
	long				startSoc;
	long				testRefI;
	long				testRefV;
	long				simVsens;
	long				simIsens;
	long				testCondUpdate;
	long				saveZ;
	
	unsigned char		refFlag;
	char				chamberWaitFlag;
	unsigned char		cvFlag;
	unsigned char		cvFaultCheckFlag;		//210204 lyhw
	
	int					saveCount;
	unsigned long		hw_fault_ovp;
	unsigned long		hw_fault_ccc;
	unsigned long		hw_fault_otp;
	unsigned long		hw_fault_temp;
	//151214 oys w : SEC Cycle Capacity Efficiency End
	unsigned char		completeFlag;
	unsigned char		standardC_Flag;
	unsigned char		standardP_Flag;
	unsigned char		standardZ_Flag;
	unsigned char		endC_std_type;
	unsigned char		endP_std_type;
	unsigned char		endZ_std_type;
	unsigned char		feedback_start;
	unsigned char		pause_flag;
	unsigned char		efficiency_pause_flag;
	unsigned char		stepSyncFlag;
	unsigned char		tempWaitType;
	
	//160510 oys w : Cycle Capacity Efficiency End
	unsigned long		standardC;
	unsigned long		standardP;
	unsigned long		standardZ;
	unsigned long		cycleSumC;
	unsigned long		cycleSumP;
	unsigned long		cycleEndC;
	//170518 lyh add
	unsigned long		cycleEndV;
	long				refTemp_backup;	//20171212 sch add

	unsigned short		socCheckCount;
	unsigned short		socCountNo[3];
	
	unsigned short		endC_std_sel;
	unsigned short		endP_std_sel;
	unsigned short		endZ_std_sel;
	unsigned short		endC_std_cycleCount;
	unsigned short		endP_std_cycleCount;
	unsigned short		endZ_std_cycleCount;
	unsigned short		ahEndRatio[MAX_STEP];
	unsigned short		whEndRatio[MAX_STEP];

	unsigned long		feedback_delayTime;

	unsigned short		changeRefI[MAX_STEP]; //MAX_STEP : 250

	unsigned long		extStepNo;
	unsigned long		extCurrentCycle;
	unsigned long		extTotalCycle;
	
	unsigned char		chGroupNo; //170105 oys add
	unsigned char		chamberNo;	//171116 oys add
	unsigned short		stepNo_pause;
	
	unsigned short		cycleNo_pause;
	unsigned short		Standard_Cycle; //210423 LJS
	long				limit_current_timeout;	//20171008 sch add
	long				deltaV_timeout;	//20180918 pth add
	long				deltaI_timeout;	//20180918 pth add

	long				parallel_val2; 	//kjg_180530
		
	unsigned short int	inv_errCode[MAX_INV_NUM];
	unsigned short int	inv_errCode_AC[MAX_INV_NUM];	//210304
	unsigned char		inv_errFlag[MAX_INV_NUM];
	
	unsigned char		send_pcu_seq_no;	//180817
	unsigned char		temp_wait_flag_cnt;	//181012
	unsigned char		receive_pcu_seq_no;	//180817
	unsigned char		endFlag;	//180903
			
	unsigned long		seq_no_cnt;	//180817
	unsigned long		invTimer;	//180817
	unsigned long		pauseRunTime;	//190901 lyhw
	
	long				Pre_change_V;	//20190607 lyhw
	long				change_V;		//20190607 lyhw
	unsigned long		change_V_timer; //20190607 lyhw
	unsigned long		save10msDt;     //20190722 oys add
	
	long				chk_V_lower[MAX_CHK_VI_POINT];	//190901 lyhw
	long				chk_V_upper[MAX_CHK_VI_POINT];	//190901 lyhw
	unsigned long		chk_V_time[MAX_CHK_VI_POINT];	//190901 lyhw
	
	long				chk_I_lower[MAX_CHK_VI_POINT];	//190901 lyhw
	long				chk_I_upper[MAX_CHK_VI_POINT];	//190901 lyhw
	unsigned long		chk_I_time[MAX_CHK_VI_POINT];	//190901 lyhw
	
	long				can_read_v;
	long				can_read_i;

	unsigned short int	can_read_errCnt;
	unsigned char		can_read_update_flag;
	unsigned char		can_error;

	unsigned char		can_range_state;
	unsigned char		can_run_state;
	unsigned char		cRateUseFlag;	//190801 oys add
	unsigned char		userDataNo;		//191029 oys add

	int					can_inv_errflag; 	//220112 lyhw
	int					reserved2;			//220112 lyhw

	long				chAuxTemp[MAX_CH_AUX_DATA]; 	//190807 pthw
	long				chAuxVoltage[MAX_CH_AUX_DATA];  //190807 pthw
	unsigned char		chAuxTCnt; 						//190807 pthw
	unsigned char		chAuxVCnt; 						//190807 pthw
	unsigned char		chAuxVCheckNum; 				//190807 pthw
	unsigned char		chAuxTCheckNum; 				//190807 pthw
	
//190826 oys add : User Select Cycle Step Charge Capacity, Discharge Capacity
	long				sel_Cyc_C_Cap[MAX_STEP];		//190826 oys add
	long				sel_Cyc_D_Cap[MAX_STEP];		//190826 oys add
//190826 oys add : Voltage Check Time
	unsigned long		pre_v_chk_time;					//190826 oys add
	long				pre_chk_v;						//190826 oys add
//190706 lyhw
	long				AuxTemp_max;	//190706
	long				AuxTemp_min;	//190706
	unsigned long		pattern_point_runTime;
	long				pattern_cross;

//191119 lyhw
	unsigned char		userPattern_ReadFlag;		
	unsigned char		StepPattern_TotalNum;
	unsigned char		StepPattern_ReadNum;
	unsigned char		pattern_change_flag;
	
	unsigned char		ac_fail_flag;
	unsigned char		preMode;		//210126 lyhw
	unsigned char		tempFaultCount;
	unsigned char		tempFaultChamberNo;

	//210316 lyhw
	long				chPressure[MAX_CH_PRESSURE_DATA];
	long				chThickness[MAX_CH_THICKNESS_DATA];
	//210318 lyhw
	long                sumAccumulated_Capacity;
	long                sumAccumulated_WattHour;
	long                seedAccumulated_Capacity;
	long                seedAccumulated_WattHour;
	long                Accumulated_Capacity;
	long                Accumulated_WattHour;
	long				fault_deltaV;	//hun_200430_add
	long				Master_recipe_deltaV;	//LJS 210412
	unsigned char       MasterFlag;		//LJS 210417
	unsigned char		deltaV_cnt;		//LJS 210417
	unsigned char		cmd_send_flag;	//LJS 210420
	unsigned char		reserved8;		//LJS 210420
	long				FaultVal[MAX_FAULT_NUM];	//LJS 210420
	long				FaultValFlag;
#ifdef _ULSAN_SDI_SAFETY
	unsigned long		humpComp_T;
	unsigned long		humpCheck_T;
	
	long				humpComp_I;
	long				humpCheck_I;

	unsigned long		cvTime_Ulsan;
#endif
	unsigned char		Fault_flag;	  	//LJS 210417
	unsigned char		reserved4[3]; 	//LJS 210417
	unsigned short		ch_fault_code;	//LJS 210417
	unsigned short		reserved5;		//LJS 210417
#if CAPACITY_CONTROL == 1
	long				CycleCapaCount;
	long				CycleCapacity;
	long				C_Rate_stepCapacity[MAX_STEP];
	long				C_Rate_Calc_Capacity[10];
	long				End_Capa_Calc_Capacity[2];
	long				End_Current_Calc_Capacity[2];
	long				Sum_C_Rate_Calc_Capacity;
	long				Sum_End_Capa_Capacity;
	long				Sum_End_Current_Capacity;
	unsigned char		c_rate_fault_flag[MAX_STEP];
	unsigned char		reserved13[2];
#endif
	long				ambient_check_flag;
	long				ambient_check_time;
	long				ambientTemp;
	long				gasVoltage;
	long				std_gasVoltage;	//220322_hun
	long				soft_venting_count; //2203222_hun
	long				gas_check_flag;
	long				gas_check_time;
#if CHAMBER_TEMP_HUMIDITY == 1	//kjc_210425
	long				humi;	//hun_210227
	unsigned long		cham_check_time_pre;
	unsigned long		cham_check_time_new;
	unsigned long		cham_check_time_1sec;
	char				cham_check_time_flag;
	char				reserved8[3];
	long				efficiency_Ah[2];
	long				loopStepNo;
	long				dischargeStepNo;
	float				calc_retain_Ah;
	int					chSyncFlag;
	int					chamber_temp_humid_check;
	long				faultEfficiencyAh;
	long				faultRetainAh;
	long				chargeAccAh;	//acc = accumulation
	long				dischargeAccAh;
#endif
	//210428 hun
	unsigned char		restCheckStartFlag;
	unsigned char		restCheckFlag;
	unsigned char		restCheckNum;
	char				reserved12;
	long				restCheck_preV[20];
	long				restFaultCount;
#ifdef _TRACKING_MODE
	long 				sum_AmpareHourSOC;
	long 				sum_CycleAmpareHourSOC;
	unsigned char		socTrackingStep;
	unsigned char		file_sucess_flag;
	unsigned char		file_fail_flag;
	unsigned char		sohTrackingStep; //211022
#endif
#ifdef _EXTERNAL_CONTROL
	unsigned char		chControl;
	unsigned char		chPause;
	unsigned char		chCV;
	unsigned char		reserved9;
	
	int					external_return;
	//int					chAlarm;
#endif
#if GAS_DATA_CONTROL == 1
	long				gas_eCo2;
	long				gas_Temp;
	long				gas_AH;
	long				gas_Baseline;
	long				gas_TVOC;
	long				gas_Ethanol;
	long				gas_H2;
#endif
	long				limit_voltage_timeout;	//210831 LJS
	unsigned char		ovp_ChamberNo;		//210904 LJS
	unsigned char		ovp_ChNo;		//210904 LJS
	unsigned char		ovp_fault_signal;
	unsigned char		code_seperate_flag;
	unsigned char		otp_ChamberNo;		//210904 LJS
	unsigned char		otp_ChNo;		//210904 LJS
	unsigned char		otp_fault_signal;
	unsigned char		reservoed1;
	long				Drop_maxV; 			//210914 LJS
	long				Drop_minV;			//210915 LJS
	//211022
	unsigned char		connect_check_flag; 
	unsigned char		chamberNoWaitFlag;	
	unsigned char		reserved11[2];		
	int					inv_auto_continue_timer; //211122_hun
	int					inv_auto_continue_count; //211122_hun
	long				charge_cc_hump_start_voltage;	//211125_hun	
	long				charge_cv_hump_start_current;	//211125_hun
	long				charge_cc_hump_start_time;	//211125_hun	
	long				charge_cv_hump_start_time;	//211125_hun
	long				discharge_cc_hump_start_voltage;	//211125_hun	
	long				discharge_cv_hump_start_current;	//211125_hun
	long				discharge_cc_hump_start_time;	//211125_hun	
	long				discharge_cv_hump_start_time;	//211125_hun
	long				charge_cc_hump_flag; //211125_hun
	long				charge_cv_hump_flag; //211125_hun
	long				discharge_cc_hump_flag; //211125_hun
	long				discharge_cv_hump_flag; //211125_hun
	long				acir_wait_flag;
	long				acir_rcv_flag;
	long				acir;
	long				acir_voltage;
	long				groupAvgVsens;			//220203_hun
	long				group_StartVoltage_flag; //220203_hun
	unsigned long		groupEndTime;			//220203_hun
	long				endState;				//220203_hun
	unsigned long		Std_Time;				//220214 ljs
	unsigned char		Fault_Check_flag;		//220214 ljs
	unsigned char		reserved13[3];			//220214 ljs
	long				DCR_PreV;				//220221 ljs
} S_CH_MISC;

//190418 lyh add
typedef struct s_ch_pcu_misc_tag {
	unsigned char		Rcv_mode;
	unsigned char		Rcv_state;
	unsigned char		Rcv_err_code;
	unsigned char		reserved1;
		
	unsigned char		Rcv_response;		//190418 ljs add
	unsigned char		Rcv_ch_relay;		//190418 ljs add
	unsigned char		Rcv_parallel_relay;	//190418 ljs add
	unsigned char		InvState;	
} S_CH_PCU_MISC;

typedef struct s_ch_ccv_tag {
	unsigned int		index;
	unsigned int		count;
	long				ad_ccv[10];
	long				ad_cci[10];
	long				avg_v;
	long				avg_i;
} S_CH_CCV;

typedef struct s_ch_step_info_tag {
	long				refV;
	long				refI;
	long				refP;
	long				refTemp;
	unsigned char		rangeV;
	unsigned char		rangeI;
	unsigned char		mode;
	unsigned char		type;
	unsigned long		advStepNo;
	unsigned long		saveDt;
} S_CH_STEP_INFO;

typedef struct s_ch_op_data_tag {
   	unsigned char		state;
   	unsigned char		phase;
	unsigned short		code;
   	unsigned char		preType;
	unsigned char		reseved1[3];

   	unsigned char		type;
   	unsigned char		mode;
	unsigned char		select;
	unsigned char		grade;

	unsigned char		rangeV;
	unsigned char		rangeI;
	unsigned char		reservedCmd;	//0:normal, 1:stop, 2:pause
	unsigned char		virRangeReservedNo; // 1: run, 2~FF : reserve, 0 : Don't Use

   	unsigned long		totalRunTime;
   	unsigned long		runTime;
   	unsigned long		checkDelayTime;
   	unsigned long		stepNo;

   	long				Vsens;
	long				meanVolt;
   	long				Isens;
   	long				meanCurr;
   	long				watt;
   	long				wattHour;
	long				ampareHour;
	long				charge_ampareHour;
	long				discharge_ampareHour;
	long				charge_wattHour;
	long				discharge_wattHour;
	long				integral_ampareHour;
	long				integral_WattHour;
   	long				capacitance;
   	long				capacitance_iec;
   	long				capacitance_maxwell;
   	long 				z;
	long				temp;
   	long				meanTemp;
	long				temp1;
	long				resultIndex;   	
   	long				totalRunTime_carry;
	long				restoreTimer;
	
	unsigned char		semiPreType;
	unsigned char		reserved[2];
	unsigned char		changeV_Dt_cnt;	//hun_200430
	#ifdef _TRACKING_MODE
	float				SOC;
	float				rptSOC;
	float				SOH; //211022
	float				rptSOH; //211022
	long				ampareHour_SOC;
	long				ampareHour_SOH; //211022
	long				limitCurrent_Upper;
	long				limitCurrent_Lower;
	unsigned long		charging_counter;
	unsigned long		charging_counter_rpt_soc;
	float				link_rptSOC;
	float				current_rptSOC;
	unsigned char		crate_flag;
	unsigned char		reserved2[3];
	#endif
	long				z_100mS;				//220322 hun
	long				z_1S;					//220322 hun
	long				z_5S;					//220322 hun
	long				z_30S;					//220322 hun
	long				z_60S;					//220322 hun
} S_CH_OP_DATA;

typedef struct s_ch_attrubute_tag {
	unsigned char		chNo_master;
	unsigned char		chNo_slave[MAX_SLAVE_CH];
	unsigned char		opType;
	unsigned char		reserved1[3];
} S_CH_ATTRIBUTE;

typedef struct s_ch_compdata_tag {
	unsigned char		useFlag;
	unsigned char		reserved1[3];
	long				compPlus;
	long				compMinus;
} S_CH_COMPDATA;

//211025_hun
typedef struct s_lges_fault_config_tag {
	long				ovp_check_time;
	long				otp_check_time;
	long				drop_v_charge_start_time;
	long				drop_v_discharge_start_time;
	long				drop_v_charge_check_time;
	long				drop_v_discharge_check_time;
	long				cv_voltage_start_time;
	long				cv_current_start_time;
	long				ovp_pause_flag;
	long				otp_pause_flag;
	long				limit_time_v;	//FAULT_CONFIG_VERSION = 1
} S_LGES_FAULT;

typedef struct s_sdi_cc_cv_hump_config_tag {
	long				charge_voltage;
	long				charge_current;
	long				charge_cc_start_time;
	long				charge_cv_start_time;
	long				charge_cc_period_time;
	long				charge_cv_period_time;
	long				discharge_voltage;
	long				discharge_current;
	long				discharge_cc_start_time;
	long				discharge_cv_start_time;
	long				discharge_cc_period_time;
	long				discharge_cv_period_time;
} S_SDI_CC_CV_HUMP;	

typedef struct s_sdi_pause_save_config_tag {
	long				pause_end_time;
	long				pause_period_time;
}S_SDI_PAUSE_SAVE;

typedef struct s_dyson_maintenance_config_tag {
	long				door_pause_flag;
}S_DYSON_MAINTENANCE;

//20171008
typedef struct s_ch_limit_user_vi_tag {
	unsigned char		limit_use_I;
	unsigned char		limit_action_I;
	unsigned char		limit_Ch_I;
	unsigned char		reserved1;
	long				limit_current;
	unsigned long		limit_time_I;
	unsigned char		limit_use_V;
	unsigned char		limit_action_V;
	unsigned char		limit_Ch_V;
	unsigned char		reserved2;
	long				limit_voltage;
} S_LIMIT_USER_VI;

typedef struct s_ch_data_tag {
	unsigned long		check1;
	S_CH_OP_DATA		op;
	S_CH_OP_DATA		opSave;
	S_CH_CCV			ccv[2];
   	S_CH_MISC			misc;
	S_CH_PCU_MISC		pcu_misc;
	S_CH_ATTRIBUTE		ChAttribute;
	S_CH_COMPDATA     	ChCompData;
   	unsigned char		inv_power;
   	unsigned char		reserved[3];
   	unsigned char		signal[MAX_SIGNAL];
	unsigned long		check2;		
} S_CH_DATA;

typedef struct s_board_source_tag {
	long				adValue[4][MAX_AD_COUNT];
	long				sensSumV[MAX_FILTER_AD_COUNT];
	long				sourceV;
	long				calSourceV;
	long				sensSumI[MAX_FILTER_AD_COUNT];
	long				sourceI;
	long				calSourceI;
} S_BD_SOURCE;

typedef struct s_board_source2_tag {
	long				sumV[MAX_SOURCE_SENS_COUNT];
	long				sourceV;
	long				sumI[MAX_SOURCE_SENS_COUNT];
	long				sourceI;
} S_BD_SOURCE2;

typedef struct s_board_misc_tag {
	S_BD_SOURCE			source[4];
	S_BD_SOURCE2		source2[4];
	
	double				Vsource_AD_a;
	double				Vsource_AD_b;
	double				Vsource_AD_a_N;
	double				Vsource_AD_b_N;
	double				Isource_AD_a;
	double				Isource_AD_b;
	double				Isource_AD_a_N;
	double				Isource_AD_b_N;
} S_BD_MISC;


typedef struct	s_board_data_tag {
	unsigned char		outputSwitch[MAX_CH_PER_BD/8];
	unsigned char		outputTrigger[MAX_CH_PER_BD/8];
	unsigned char		C_D_Select[MAX_CH_PER_BD/8];
	unsigned char		readOtFault[MAX_CH_PER_BD/8];
	unsigned char		readHwFault[MAX_CH_PER_BD/8];
	unsigned char		rangeSelectV[MAX_RANGE][MAX_CH_PER_BD/8];
	unsigned char		rangeSelectI[MAX_RANGE][MAX_CH_PER_BD/8];
	unsigned char		parallelSwitch[MAX_CH_PER_BD/8];
	unsigned char		statusLed[MAX_CH_PER_BD/4];
   	unsigned char		signal[MAX_SIGNAL];
	S_CH_DO_DATA		ch_do[MAX_CH_PER_BD];
	S_CH_DI_DATA		ch_di[MAX_CH_PER_BD];
#if MACHINE_TYPE == 1
	S_CH_SHORT_DO_DATA	ch_short_do[MAX_CH_PER_BD];
	S_CH_SHORT_DI_DATA	ch_short_di[MAX_CH_PER_BD];
	S_CH_RELAY_DO_DATA	ch_relay_do[MAX_CH_PER_BD];
#endif
		
	S_BD_MISC			misc;

    S_CH_DATA			cData[MAX_CH_PER_BD];
} S_BD_DATA;

typedef struct s_module_misc_tag {
	unsigned int		mainSlot;
	unsigned int		shiftSlot;
	
	unsigned long		timer_1sec;
	long long			timer_usec;
	
	unsigned int		timer_1sec_count;
	unsigned int		sensCount;
	
	unsigned int		sourceSensCount;

	unsigned char		sensCountFlag;
	unsigned char		sourceSensCountFlag;
	unsigned char		virRangeUseFlag;
	unsigned char		virOutputSwitch;
	
	int					processPointer;
	unsigned int		virRangeUseCh;
	
	unsigned int		virRangeUseCnt;
	
	unsigned long		rt_scan_time;
	unsigned long		dio_scan_time;
	unsigned long		endBuzzerCount;
	unsigned long		invertFailWait;
	long long			buzzerFlag1;
	long long			buzzerFlag2;
	
	int					outFlag;
	int					reserved;
	unsigned long		timer_mSec;
	unsigned long		safety_fault_timer;
	
	unsigned char		d_cali_update_ch;	//180813 lyh add
	unsigned char		usingUserDataFlag[MAX_CH_PER_BD];	//191029 oys add
	//unsigned char		reserved1[4];
	unsigned long		disconnect_timer;
	
	//210316 lyhw
	unsigned long		slot_periodic;	
	unsigned long		slot_tic_timer;	
	unsigned long		realTime_sec;	
	
	long 				ambientTemp[10];
	long				gasVoltage[8];
	long				ambientTemp_org[10];
	long				gasVoltage_org[8];
	int					daq_slot;
} S_MODULE_MISC;

typedef struct s_module_config_tag {
	unsigned int		installedBd;
	unsigned int		installedCh;
	unsigned int		chPerBd;
	unsigned short		installedTemp;
	unsigned short		installedAuxV;
	unsigned short		installedCAN;
	unsigned char		sendAuxFlag;
	unsigned short		reserved1[5];

	unsigned short 		rangeV;
	unsigned short		rangeI;

	long				maxVoltage[MAX_RANGE];
	long				minVoltage[MAX_RANGE];
	long				maxCurrent[MAX_RANGE];
	long				minCurrent[MAX_RANGE];

	unsigned char		ratioVoltage;
	unsigned char		ratioCurrent;
	unsigned char		rt_scan_type;
	unsigned char		reserved2[5];

	unsigned char		watchdogFlag;
	unsigned char		ADC_type;
	unsigned char		hwSpec;
	unsigned char		capacityType;

	unsigned int		totalJig;
	unsigned int		bdInJig[16];
	float				shunt[MAX_RANGE];
	float				gain[MAX_RANGE];
	float				adAmp;
	float				voltageAmp;
	float				currentAmp;
	unsigned char		auto_v_cali;
	unsigned char		parallelMode; //kjg_180521 2:parallel_cycle use
	unsigned char		DAC_type; //add 20100723
	unsigned char		ADC_num; //add 20100723
	float				AD_offset; //add 20100723
	unsigned char		SoftFeedbackFlag;
	unsigned char		MainBdType;
	unsigned char		FadBdUse;
	unsigned char		reserved3;
	unsigned char		range[5][4];
	unsigned long		tempBoundary; // add <--20071106
	unsigned long		reserved4[3];
	long				hwFaultConfig[MAX_HW_FAULT_NUM];
	unsigned char		function[MAX_FUNCTION_NUM];
	unsigned char		ChamberMotion[MAX_CHAMBERMOTION_NUM];
	unsigned char		ChamberDioUse; 
	unsigned char		ChamberPerUnit;
	unsigned char		ChamberPerSigNum;
	unsigned char		reserved5;
	unsigned char		TempFaultDioUse; //210121
	unsigned char		reserved6[3];
   	long				TempFaultMinT;	
   	long				TempFaultMaxT;	
   	long				TempFaultCheckTime;	
	S_LIMIT_USER_VI		LimitVI;  //20171008 sch add
	long				swFaultConfig[20];	//210428 hun
	S_LGES_FAULT		LGES_fault_config;	//hun_211025
	int					ambient2; //INC/LGES_PatchNote.txt Check
	int					code_buzzer_on; //INC/LGES_PatchNote.txt Check
	int					inv_fault_count; //INC/SDI_PatchNote.txt Check
	int					cc_cv_hump_flag; //INC/SDI_PatchNote.txt Check
	int					sdi_pause_save_flag; //INC/SDI_PatchNote.txt Check
	S_SDI_CC_CV_HUMP	SDI_cc_cv_hump;	//hun_211025
	S_SDI_PAUSE_SAVE	SDI_pause_save;	//hun_211209
	S_DYSON_MAINTENANCE	DYSON_maintenance; //jsh_220118
	int					dyson_maintenance_flag; //jsh_220124
	int					Semi_Autojig_DCR_Calculate; //210218 LJS
	int					Auto_Update;	//220408_hun
} S_MODULE_CONFIG;

typedef struct s_module_pcu_config_tag {	//180611 lyh add for digital
	unsigned char		portPerCh;
	unsigned char		installedInverter;
	unsigned char		invPerCh;
	unsigned char		parallel_inv_ch;
	
	double				hallCT;
	float				caliV_ohm;
	float				caliV_amp;
	unsigned short		errCnt[32];

	unsigned short		pcuCaliUse;
	unsigned char		powerNo;
	unsigned char		inverterType;	//210304 for jiwoo, panda

	unsigned short		setCaliNum;
	unsigned short		cali_delay_time;
} S_MODULE_PCU_CONFIG;

typedef struct s_test_cond_header_tag {
	unsigned char		totalStep;
	unsigned char		reserved1[3];
	long				reserved2[2];
} S_TEST_COND_HEADER;

typedef struct s_test_cond_check_tag {
	unsigned char		compFlag;
	unsigned char		reserved1[3];
    long				ocvUpper;
    long				ocvLower;
    long				Vref;
    long				Iref;
    long				endT;
	long				faultUpperV;
	long				faultLowerV;
	long				faultUpperI;
	long				faultLowerI;
    long				faultDv1;		//ocv ~ ccv
	long				faultDv2;		//ccv
	long				reserved2[2];
} S_TEST_COND_CHECK;

typedef struct s_test_cond_grade_step_tag {
	unsigned char		gradeCode;
	unsigned char		item;
	unsigned char		gradeProc;
	unsigned char		reserved;
	long				lowerValue;
	long				upperValue;
} S_TEST_COND_GRADE_STEP;

typedef struct s_test_cond_grade_tag {
	unsigned char		item;
	unsigned char		gradeStepCount;
	short int			reserved1;

	S_TEST_COND_GRADE_STEP	gradeStep[MAX_GRADE_STEP];
} S_TEST_COND_GRADE;

//210602 LJS
#ifdef _END_COMPARE_GOTO
typedef struct s_test_cond_endgoto_tag{
	long			type;
	unsigned long	value;
	long			sign;
	long			gotoStepNo;
} S_TEST_END_COMP_GOTO;
#endif

typedef struct s_test_cond_step_tag {
	unsigned char		type;
	unsigned char		mode;
	unsigned char		rangeI;
	unsigned char		testEnd;

	unsigned char		subStep;
	unsigned char		useSocFlag;
	unsigned char		rangeV;
	unsigned char		jigPressTimeOutFlag;	//211025 lyhw

	unsigned long		stepNo;

	long				refV;
	long				refI;
	long				refP;
	long				refR;
	long				refTemp; // add <--20071106

	unsigned long		endT;
	unsigned long		endT_CV;
	long				endV;
	long				endV_L;
	long				refV_L;
	long				refV_H;
	long				refI_L;
	long				refI_H;
	long				endI;
	long				endC;						//capacity
	long				endP;						//Power
	long				endWh;						//WattHour
	long				endDeltaV;
	long				startTemp; //add <--20071106
	long				endTemp; //add <--20071106
	//101028 kji add
	char				tempType;
	char				tempDir;
	unsigned short		endTempGoto;
	unsigned short		endSocGoto;
	short int			endSoc;
	long				socStepCap;		//20190214 add
	unsigned short		SocSoeFlag;		//20190214 add
	long				socStepPower;	//add <--20170501 oys
	long				socStepZ;		//add <--20170501 oys
	unsigned short int	socCapStepNo;
	unsigned short int	reserved2[2];
	long				faultUpperTemp_restart;
	long				faultLowerTemp_restart;
	//long				endDv;
	//unsigned long		endDVT;
	//long				endDi;
	//unsigned long		endDIT;
	//unsigned long		endCompT[EP_COMP_POINT];
	//long				endCompV[EP_COMP_POINT];

	unsigned long		saveDt;						//delta time
	long				saveDv;
	long				saveDi;
	long				saveDtemp;
	long				saveDp;

	long				faultUpperV;
	long				faultLowerV;
	long				faultUpperI;
	long				faultLowerI;
	long				faultUpperC;
	long				faultLowerC;
	long				faultUpperZ;
	long				faultLowerZ;
	long				faultUpperTemp;
	long				faultLowerTemp;
	
	long				pauseUpperTemp;		//190311 lyh add
	long				pauseLowerTemp;     //190311 lyh add
	//170215 SCH add for DeltaV/I
	long				faultDeltaV;
	long				faultDeltaI;
	unsigned long		faultDeltaV_T;
	unsigned long		faultDeltaI_T;
	
	long				capacitance_v1;		//v1 for capacitance
	long				capacitance_v2;		//v2 for capacitance
	unsigned long		z_t1;				//t1 for dcr
	unsigned long		z_t2;				//t2 for dcr
	unsigned long		lc_t1;				//t1 for leakage current
	unsigned long		lc_t2;				//t2 for leakage current

	unsigned long		advCycleCount;
	unsigned long		advGotoStep;
	unsigned long		totalCycleCount;
	unsigned long		totalGotoStep;

	unsigned short		gotoCycleCount;
	unsigned short		reserved3;

	unsigned long		stopT;

	long				stopV;
	long				stopI;
	long				stopC;				//capacity
	long				stopP;				//Power
	long				stopWh;				//WattHour
	
	unsigned short		endVGoto;
	unsigned short		endIntegralCGoto;

	unsigned short		endIGoto;
	unsigned short		endIntegralWhGoto;

	unsigned short		endTGoto;
	unsigned short		reserved4;

	unsigned short		endCGoto;
	unsigned short		reserved5;

	unsigned short		endTCVGoto;
	unsigned short		reserved6;

	unsigned char		gotoFlag;
	unsigned char		gotoStep;
	unsigned char		integralTFlag;
	unsigned char		integralCFlag;
	unsigned char		integralWhFlag;
	unsigned char		cycleEndStepSave;
	unsigned char		integralInit;
	unsigned char		balanceStepCheck;
	long				cycleNo;
	long				chamber_dev; //210422
	long				reserved7[3]; //210422 4->3

	//long				startRefV;			//for Battery Z
	//long				startRefI;			//for Battery Z
	//unsigned long		startT;				//for Battery Z
	//unsigned long		waitT;				//for Battery Z
	
	unsigned char		integralCapFlag;	//pjy add for toshiba
	//170501 oys add
	unsigned char		endZeroVoltageFlag;
	unsigned short		stepNo_pause;
	unsigned short		cycleNo_pause;
	short int			endP_soc;
	short int			endZ_soc;
	unsigned char		endP_std_sel;
	unsigned char		endZ_std_sel;
	unsigned char		endP_std_type;
	unsigned char		endZ_std_type;

	unsigned short		endC_std_cycleCount;
	unsigned short		endP_std_cycleCount;
	unsigned short		endZ_std_cycleCount;
	unsigned short		reserved8;

	unsigned char		endC_proc_type;
	unsigned char		endP_proc_type;
	unsigned char		endZ_proc_type;
	unsigned char		noTempWaitFlag;
	unsigned short		reduce_ratio_P;
#if END_V_COMPARE_GOTO == 1
	long				endVGoto_upper;
	long				endVGoto_lower;
	unsigned short		endVupper_GotoStep;
	unsigned short		endVlower_GotoStep;
#endif
	//add end
#if AUX_CONTROL == 1
	#if NETWORK_VERSION >= 4103
	short int			auxType[MAX_AUX_FUNCTION];
	// 0: Not Use, 1: Temp, 2: Voltage
	unsigned char		auxCompareType[MAX_AUX_FUNCTION];
	// 0: Not Use, 1: <, 2: <=, 3: ==, 4: >, 5: >=, 6: !=
	unsigned char		reserved9[2];

	short int			auxGoto[MAX_AUX_FUNCTION];
	long				endAuxValue[MAX_AUX_FUNCTION];
	#endif
#endif
#if CHANGE_VI_CHECK	==	1		//190901 lyhw
	long				change_V_lower;	
	long				change_V_upper;	
	long				change_V;		
	unsigned long		change_V_time;	
	
	long				chk_V_lower[MAX_CHK_VI_POINT];	
	long				chk_V_upper[MAX_CHK_VI_POINT];	
	unsigned long		chk_V_time[MAX_CHK_VI_POINT];	
	
	long				chk_I_lower[MAX_CHK_VI_POINT];	
	long				chk_I_upper[MAX_CHK_VI_POINT];	
	unsigned long		chk_I_time[MAX_CHK_VI_POINT];	
	long				faultRunTime;	//hun_200219
#endif
#ifdef _SDI_SAFETY_V1 //211112 hun w
	long				faultRunTime;	//hun_200219
#endif
#ifdef _ULSAN_SDI_SAFETY
	long							humpSet_T;
	long							humpSet_I;
#endif
	S_TEST_COND_GRADE	grade[MAX_GRADE_ITEM];
#ifdef _END_COMPARE_GOTO
	S_TEST_END_COMP_GOTO endCompGoto[MAX_COMP_GOTO]; //210602 LJS
#endif
#if CAPACITY_CONTROL == 1
	short int			endCycleCapaRate[2];
	short int			endCycleCurrentRate[2];
	unsigned short int	endCycleCapaStepNo[2];
	unsigned short int	endCycleCurrentStepNo[2];
	long				endCycleCapaGoto;
	long				endCycleCurrentGoto;
	unsigned char		endCycleCapaSign[2];
	unsigned char		endCycleCurrentSign[2];
	int					UseCheckCapaFlag;
	long				CycleCapaCount;
	short int			C_Rate_Persent[10];
	unsigned short int	C_Rate_stepNo[10];
	unsigned char		C_Rate_Sign[10];
	unsigned char		reserved10[2];
#endif
//cham : chamber, sync : syncronization
//T : tiem, humid : humidity, dev : deviation
#if CHAMBER_TEMP_HUMIDITY == 1		//kjc_210412
	unsigned long				cham_sync_T;
	long						cham_temp;
	long						cham_humid;
	long						ch_temp;
	short int					cham_temp_dev;							
	short int					cham_humid_dev;
	short int					ch_temp_dev;
	short int					reserved8;
	int							cham_temp_sig;
	int							cham_humid_sig;
	int							ch_temp_sig;
	int							reserved11;
#endif
#ifdef _TRACKING_MODE
	long						rptsoc;
	long						soc;
	unsigned long				reserved14[2]; // 211124 rewrite
	unsigned long				crate_factor;
	unsigned char				schedule_link_flag;
	unsigned char				crate_flag;
	unsigned char				SOC_Tracking_flag;
	unsigned char				SOH_Tracking_flag; //211022
	unsigned char				trackingMode_flag;
	unsigned char				reserved12[3];
	unsigned short int			rptSOC;
	unsigned short int			rptSOH; //211022
	long						limitCurrent_Lower;
	long						limitCurrent_Upper;
#endif
#if GAS_DATA_CONTROL == 1		//210923 lyhw
	long						endGasTVOC;
	long						endGasECo2;
	unsigned short				endGasTVOC_Goto;
	unsigned short				endGasECo2_Goto;

	long						faultUpper_GasTVOC;
	long						faultLower_GasTVOC;
	long						faultUpper_GasECo2;
	long						faultLower_GasECo2;
#endif
#ifdef _EQUATION_CURRENT 		//211111
	unsigned char				equation_current_flag;
	unsigned char				reserved13[3];
	long						Max_Current_EQU;
	long						variable[4];
#endif
#ifdef _GROUP_ERROR		//220203 hun
	long						group_StartVoltage;
	long						group_CheckTime;
	long						group_DeltaVoltage;
	long						group_EndFaultTime;
#endif
} S_TEST_COND_STEP;

typedef struct s_test_cond_safety_tag {
	long				faultLowerV;
	long				faultUpperV;
	long				faultLowerI;
	long				faultUpperI;
	long				faultLowerC;
	long				faultUpperC;
	long				faultLowerTemp;
	long				faultUpperTemp;
// 191002 oys add start : SDI safety process
	unsigned short		changeV_Dv;
	unsigned short		changeV_Dt;
	unsigned short		deltaV_Dv;
	unsigned short		deltaV_Dt;
// 1921002 oys add end
#ifdef _SDI
	#ifdef _SDI_SAFETY_V0 //211112 hun w
	long				reserved1[2];
	#endif
	#ifdef _SDI_SAFETY_V1 //211112 hun w
	long				faultSOH;	//hun_200219
	long				fault_deltaDv; //hun_200430
	#endif
	#ifdef _SDI_SAFETY_V2 //211112 hun w
	long				Master_Recipe_V;//LJS 210415
	long				reserved1[5];	//LJS 210415
	#endif
#endif
#ifndef _SDI
	#if CHAMBER_TEMP_HUMIDITY == 1
		long				capacityEfficiency;
		long				capacityRetain;
	#else
		long				reserved1[2];
	#endif
#endif
#ifdef _TRACKING_MODE
	long				rptsoc;
	long				soc;
	long				rptsoh; //211022
	long				soh; //211022
	unsigned long		reserved2[2]; //211124 rewrite
	unsigned long		crate_factor;
	unsigned char		schedule_link_flag;
	unsigned char		schedule_link_flag2; //211022
	unsigned char		reserved[2];
#endif
} S_TEST_COND_SAFETY;

typedef struct s_test_cond_reserved_tag {
	unsigned char		reserved_cmd;	//0:normal, 1:stop, 2:pause
	unsigned char		select_run;		//0:normal, 1:select_run
	unsigned char		reserved1[2];
	unsigned long		reserved_stepNo;
	unsigned long		reserved_cycleNo;
	unsigned long		select_stepNo;
	unsigned long		select_cycleNo;
	unsigned long		select_advCycleStep;
} S_TEST_COND_RESERVED;

typedef struct s_user_pattern_data_tag{
	long				stepNo;
	long				length;
	long				type;
	long				time;
	long				data;
#if USER_PATTERN_500 == 1
	long				totalNumber;	//191201 lyhw
	long				Number;			//191201 lyhw
#endif
} S_USER_PATTERN_DATA;

typedef struct s_ch_dcir_val_tag{
	float	runTime[MAX_PULSE_DATA_IEC];
	float	Vsens[MAX_PULSE_DATA_IEC];
	float	Isens[MAX_PULSE_DATA_IEC];
} S_CH_DCIR_VAL;

typedef struct s_test_cond_user_pattern_data_tag{
	long				time;
	long				data;
} S_TEST_COND_USER_PATTERN_DATA;

typedef struct s_test_cond_user_pattern_tag{
	long				stepNo;
	long				length;
	long				type;
#if USER_PATTERN_500 == 1
	long				totalNumber;	//191201 lyhw
	long				Number;
#endif
	S_TEST_COND_USER_PATTERN_DATA data[MAX_USER_PATTERN_DATA];
} S_TEST_COND_USER_PATTERN;

typedef struct s_test_cond_user_map_tag{
	long						stepNo;
	short int					type;
	short int					mode;
	long						maxCapacity;
	long						renewalTime;
	char						ocvTableRow;
	char						ocvTableCol;
	char						dataTableRow;
	char						dataTableCol;
	long						ocvTable[MAX_OCV_TABLE_ROW][MAX_OCV_TABLE_COL];
	long						dataTable[MAX_DATA_TABLE_ROW][MAX_DATA_TABLE_COL];
} S_TEST_COND_USER_MAP;

#ifdef _TRACKING_MODE
typedef struct s_test_cond_soc_tracking_data_tag{
	unsigned short int			temp_num;
	unsigned short int			soc_num;
	long						maxI;
	long						minI;
	long						temp[MAX_SOC_TRACKING_DATA];
	long						SOC[MAX_SOC_TRACKING_DATA];
	long						limit_current[MAX_SOC_TRACKING_DATA][MAX_SOC_TRACKING_DATA];
	double						tracking_data_A[MAX_SOC_TRACKING_DATA][MAX_SOC_TRACKING_DATA];
	double						tracking_data_B[MAX_SOC_TRACKING_DATA][MAX_SOC_TRACKING_DATA];
} S_TEST_COND_SOC_TRACKING_DATA;
#endif

typedef struct s_test_condition_tag {
	S_TEST_COND_HEADER			header;
    S_TEST_COND_STEP			step[MAX_STEP];
	S_TEST_COND_SAFETY			safety;
	S_TEST_COND_RESERVED		reserved;
	S_TEST_COND_USER_PATTERN	userPattern;
	S_TEST_COND_USER_MAP		userMap;
#if USER_PATTERN_500 == 1
	S_TEST_COND_USER_PATTERN 	userPatternBuf;
#endif
#ifdef _TRACKING_MODE
	S_TEST_COND_SOC_TRACKING_DATA SOC_tracking[2];
#endif
} S_TEST_CONDITION;

typedef struct s_module_addr_tag	{
	int		inputAddrNo;
	int		outputAddrNo;
	int		expendAddrNo;
	int		interface[3][32];
	int		main[32];
}	S_MODULE_ADDR;

typedef struct s_module_fault_tag	{
	unsigned char		SMPS7_5V[8];
	unsigned char		SMPS3_3V[8];
	unsigned char		OT[8];
	// lyh add for Main rev 11 Fault
	unsigned long		PS_ADDR_7V[8];
	unsigned char		PS_BIT_7V[8];
	unsigned long		PS_ADDR_3V[8];
	unsigned char		PS_BIT_3V[8];
	unsigned long		ADDR_OT[4];
	unsigned char		BIT_OT[4];
	unsigned char		PS_ACTIVECH_7V[8];
	unsigned char		PS_ACTIVECH_3V[8];
}	S_MODULE_FAULT;

typedef struct s_module_fault_board_tag	{
	unsigned char		useFlag;
	unsigned char		active;
	unsigned char		bit;
}	S_MODULE_FAULT_BOARD;


typedef struct s_module_data_tag {
    unsigned char		state;
    unsigned char		phase;
    unsigned char		code;
	unsigned char		cali_meas_type;
	unsigned int		cali_hallCT;
	unsigned long		cali_meas_time;
	
	unsigned char		signal[MAX_SIGNAL];
	unsigned char		reserved1[3];
	unsigned char		mainStateCheckFlag;	//210113 ljs
	long				runningTime[7][200];
//131228 oys w : real time add
#if REAL_TIME == 1
	long				real_time[7];
#endif
	
	S_MODULE_MISC		misc;
	S_MODULE_CONFIG		config;
	S_MODULE_ADDR		addr;
	S_MODULE_FAULT		fault;
	S_MODULE_FAULT_BOARD	board_fault[4][3];
	S_MODULE_PCU_CONFIG	pcu_config;		//180611 lyh add for digital

    S_TEST_CONDITION	testCond[MAX_BD_PER_MODULE][MAX_CH_PER_BD];
} S_MODULE_DATA;

typedef struct s_aux_set_data_tag {
    unsigned char       chNo;           //1base : machine channel number
    unsigned char       reserved1[3];
    short int           auxChNo;        //1base : aux channel number
    short int           auxType;        //0:temperature, 1:v
    char                name[MAX_AUX_NAME_SIZE];
    long                fault_upper;
    long                fault_lower;
    long                end_upper;
    long                end_lower;
    short int           function_div1;
    short int           function_div2;
    short int           function_div3;
    short int           reserved2;
} S_AUX_SET_DATA;
	#if AUX_CONTROL == 1
		#if NETWORK_VERSION >= 4103
		typedef struct s_aux_misc_tag {
			unsigned char	endCnt[MAX_AUX_DATA][MAX_AUX_FUNCTION];
			unsigned char	auxEndCnt[MAX_AUX_DATA];
			unsigned char	faultCnt[MAX_AUX_DATA];
			unsigned char	auxGoto[MAX_BD_PER_MODULE][MAX_CH_PER_BD];
			unsigned char	reserved;
		} S_AUX_MISC;
		#endif
	#endif
#endif
