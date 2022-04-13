#ifndef __MESSAGE_STR_H__
#define __MESSAGE_STR_H__

#include "Message_def.h"

typedef struct s_msg_val_tag {
	int					msg;
	int					ch;
	int					val;
	int					bd; 			//20190527
	
	long				event_val;		//20190527
	int					num;			//191120
	int					reserved1;
	long				reserved2;  //20190527
} S_MSG_VAL;

typedef struct s_msg_ch_flag_tag {
	unsigned long		bit_32[8];
} S_MSG_CH_FLAG;

typedef struct s_msg_tag {
	int					write_idx;
	int					read_idx;
	S_MSG_VAL			msg_val[MAX_MSG];
	S_MSG_CH_FLAG		msg_ch_flag[MAX_MSG];
} S_MSG;

#ifdef __LG_VER1__
typedef struct s_save_ch_data_tag {
	unsigned char		ch;								
	unsigned char		state;
	unsigned char		type;
	unsigned char		mode;
	unsigned char		select;
#if PROGRAM_VERSION2 <= 1
	#if VENDER == 3 //210419 rewrite
		#ifdef _TRACKING_MODE 
		unsigned char		reserved1;
		short int			code;
		short int			reserved6;
		#else
		unsigned char		code;
		#endif
	#else
	unsigned char		code;
	#endif
#else
	unsigned char		reserved1;
#endif
	unsigned char		stepNo;
	unsigned char		grade;
	//8byte
	long				Vsens;
	long				Isens;
	//16byte
	long				capacity;
	long				watt;
	long				wattHour;
	unsigned long		runTime;
	//32byte
	unsigned long		totalRunTime;
	long				z;	
	long				temp;
	unsigned char		reservedCmd;	//0:normal, 1:stop, 2:pause
	unsigned char		virRangeReservedNo;
	unsigned short		gotoCycleCount;
	//48byte
	unsigned long		totalCycle;	
	unsigned long		currentCycle;
	long				avgV;
	long				avgI;
	//64byte
	long				resultIndex;
	//68byte
#if NETWORK_VERSION > 4101
	#if VENDER != 2 //NOT SDI
	long                IntegralAmpareHour;
	long                IntegralWattHour;
	long                ChargeAmpareHour;
	//80byte
	long                ChargeWattHour;
	long                DischargeAmpareHour;
	long                DischargeWattHour;
	long                cvTime;
	//96byte
		#if PROGRAM_VERSION1 == 0
			#if PROGRAM_VERSION2 >= 1
			long				Farad;	
			char				totalRunTime_carry;	//111125 oys
			char				fileIndex;	//20180716 sch
			unsigned short		cycleNo;
			long                temp1;
			//108byte
			#endif
			#if EDLC_TYPE == 1
			//	20160229 khk add start			
			long                c_v1;
			long                c_v2;
			long                c_t1;
			long                c_t2;
			// 20160229 khk add end	
			// 20180206 sch add start
			long                capacitance_iec;	
			long                capacitance_maxwell;	
			// 20180206 sch add end
			#endif
			#if PROGRAM_VERSION2 >= 2
			long				chargeCCAh;
			long				chargeCVAh;
			long				dischargeCCAh;
			long				dischargeCVAh;
			short int			code;
			short int			reserved2;
			long				z_100mS;	//hun_220322
			long				z_1S;		//hun_220322
			long				z_5S;		//hun_220322
			long				z_30S;		//hun_220322
			long				z_60S;		//hun_220322
			/* 노스볼트에만 사용으로 인해 주석 처리 by 김장훈
			long				Chamber_Temp;	 	//200715 lyhw
			long				Acc_Capacity;		//200318 lyhw
			long				Acc_WattHour;		//200318 lyhw			
			long				reserved3[2];
			*/
			#endif	
			#ifdef _AMBIENT_GAS_FLAG	//hun_211013
			long				ambientTemp;
			long				gasVoltage;
			#endif
			#if REAL_TIME == 1
			//131228 oys w : real time add
			long				realDate;
			long				realClock;
			#endif
			//hun_220322_s
			#if RESERVED == 0
			#elif RESERVED == 1
			long				Reserved[10];
			#endif
			//hun_220322_e
			#if	VENDER == 1 && CH_AUX_DATA == 1	//190807 pthw
			long	ch_AuxTemp[MAX_CH_AUX_DATA];
			long	ch_AuxVoltage[MAX_CH_AUX_DATA];
			#endif
			#ifdef _EXTERNAL_CONTROL		//210316 lyhw
			unsigned char 		chControl;
			unsigned char		chPause;
			unsigned char		chCV;
			unsigned char		reserved6;
			int					external_return;
			//int				chAlarm;
			#endif
			#ifdef _CH_SWELLING_DATA
			long	PressureData[MAX_CH_PRESSURE_DATA];
			long	ThicknessData[MAX_CH_THICKNESS_DATA];
			#endif
			#if CH_SWELLING_DATA == 1 //210316 NV Use lyhw
			long	PressureData[MAX_CH_PRESSURE_DATA];
			long	ThicknessData[MAX_CH_THICKNESS_DATA];
			#endif
			#if CHAMBER_TEMP_HUMIDITY == 1
			long				humi;				//hun_210227
			long				chargeAccAh;
			long				dischargeAccAh;
			long				EfficiencyAh;
			#endif
		#endif
		#if PROGRAM_VERSION1 > 0
		long                Farad;	
		char				totalRunTime_carry;	//111125 oys
		long                temp1;
		unsigned short		cycleNo;
		long				startVoltage;
		long				maxVoltage;
		long				minVoltage;
		long				startTemp;
		//124byte
		long				maxTemp;
		long				minTemp;
		//132byte
			#if REAL_TIME == 1
			long				realDate;
			long				realClock;
			long				reserved4[9];
			#endif
		#endif	
//add end
	#endif
	#if VENDER == 3 //20200629 ONLY SK
	long				Chamber_Temp;
	long				reserved[3];
	#endif
	#ifdef _AC_FAIL_RECOVERY 
	//Data Restore
	long				advCycle;
	unsigned long		advCycleStep;
	unsigned long		cycleRunTime;				//CycleStep
	long				seedintegralCapacity;
	long				sumintegralCapacity;
	long				seedintegralWattHour;
	long				sumintegralWattHour;
	long				seedChargeAmpareHour;
	long				sumChargeAmpareHour;
	long				seedDischargeAmpareHour;
	long				sumDischargeAmpareHour;
	long				seedChargeWattHour;
	long				sumChargeWattHour;
	long				seedDischargeWattHour;
	long				sumDischargeWattHour;
	unsigned long		standardC;					//step
	unsigned long		standardP;					//step
	unsigned long		standardZ;					//step
	unsigned long		cycleSumC;					//step
	unsigned long		cycleSumP;					//step
	unsigned long		cycleEndC;					//step
	unsigned char		pattern_change_flag;
	unsigned char		chGroupNo;
	unsigned char		tempDir;
	unsigned char		reserved5;
	#endif
	#ifdef _TRACKING_MODE 
	long	SOC;
	long	RPT_SOC;
	long	SOH;		//211022
	long	RPT_SOH;	//211022
	unsigned char		socRefStep;
	unsigned char		sohRefStep;
	unsigned char		chamberNoWaitFlag;
	unsigned char		reserved11[1];
	#endif
	#if GAS_DATA_CONTROL == 1
	long	gas_eCo2;
	long	gas_Temp;
	long	gas_AH;
	long	gas_Baseline;
	long	gas_TVOC;
	long	gas_Ethanol;
	long	gas_H2;	
	#endif
	#if VENDER == 3 && CH_AUX_DATA == 1
	long	ch_AuxTemp[MAX_CH_AUX_DATA];
	long	ch_AuxVoltage[MAX_CH_AUX_DATA];
	#endif
#endif
} S_SAVE_MSG_CH_DATA;
#endif

#ifdef __SDI_MES_VER4__
typedef struct s_save_ch_data_tag {
	unsigned char		ch;
	unsigned char		state;
	unsigned char		type;
	unsigned char		mode;

	unsigned char		select;
//	unsigned char		grade;
	unsigned char		cvFlag;	// 210417 LJS
	unsigned char		switchState[2];

	short int			code;
	short int			stepNo;
	
	long				Vsens;
	long				Isens;
	long				ChargeAmpareHour;
	long				DischargeAmpareHour;
//	long				watt;
	long				ChargeWatt;
	long				DischargeWatt;
	long				ChargeWattHour;
	long				DischargeWattHour;

	unsigned long		runTime_day;
	unsigned long		runTime;
	unsigned long		totalRunTime_carry;
	unsigned long		totalRunTime;
	long				z;
	long				temp[3][4];// normal, min, max, avg

	unsigned char		chamber_control;
	unsigned char		record_index;
	unsigned char		input_val;
	unsigned char		output_val;
	
	unsigned char		reservedCmd;	//0:normal, 1:stop, 2:pause
	unsigned char		virRangeReservedNo;
	unsigned short		gotoCycleCount;

	unsigned long		totalCycle;
	unsigned long		currentCycle;

	long				avgV;
	long				avgI;
	long				resultIndex;

	unsigned long		cvTime_day;
	unsigned long       cvTime;
	
	long				realDate;
	long				realClock;

	long				Vinput;
	long				Vpower;
	long				Vbus;

	long                IntegralAmpareHour;
	long                IntegralWattHour;
	
	unsigned long		ccTime_day;
	unsigned long		ccTime;

	long				charge_cc_ampare_hour;
	long				charge_cv_ampare_hour;
	long				discharge_cc_ampare_hour;
	long				discharge_cv_ampare_hour;
	long				startVoltage;
	unsigned long		step_count;
	long				maxVoltage;
	long				minVoltage;
	#ifdef _CH_CHAMBER_DATA
	long				Chamber_Temp;	 //211025 hun
	#endif
	#ifdef _CH_SWELLING_DATA
	long				PressureData[MAX_CH_PRESSURE_DATA];
	long				ThicknessData[MAX_CH_THICKNESS_DATA];
	#endif
	#ifdef _ACIR		//220124 hun
	long				acir_voltage;
	long				acir;
	#endif	
	#ifdef _AC_FAIL_RECOVERY
	//Data Restore
	long				advCycle;
	unsigned long		advCycleStep;
	unsigned long		cycleRunTime;
	long				seedintegralCapacity;
	long				sumintegralCapacity;
	long				seedintegralWattHour;
	long				sumintegralWattHour;
	long				seedChargeAmpareHour;
	long				sumChargeAmpareHour;
	long				seedDischargeAmpareHour;
	long				sumDischargeAmpareHour;
	long				seedChargeWattHour;
	long				sumChargeWattHour;
	long				seedDischargeWattHour;
	long				sumDischargeWattHour;
	unsigned long		standardC;					//step
	unsigned long		standardP;					//step
	unsigned long		standardZ;					//step
	unsigned long		cycleSumC;					//step
	unsigned long		cycleSumP;					//step
	unsigned long		cycleEndC;					//step
	unsigned char		pattern_change_flag;
	unsigned char		chGroupNo;
	unsigned char		tempDir;
	unsigned char		reserved5;
	long				sumChargeCCAh;				//step
	long				seedChargeCCAh;				//step
	long				sumChargeCVAh;				//step
	long				seedChargeCVAh;				//step
	long				sumChargeCCCVAh;			//step
	long				seedChargeCCCVAh;			//step
	long				sumDischargeCCAh;			//step
	long				seedDischargeCCAh;			//step
	long				sumDischargeCVAh;			//step
	long				seedDischargeCVAh;			//step
	long				sumDischargeCCCVAh;			//step
	long				seedDischargeCCCVAh;		//step
	long				chargeCCAh;					//step
	long				chargeCVAh;					//step
	long				chargeCCCVAh;				//step
	long				dischargeCCAh;				//step
	long				dischargeCVAh;				//step
	long				dischargeCCCVAh;			//step
	long				cycleSumChargeWatt;			//step
	long				cycleSumChargeWattHour;		//step
	long				cycleSumChargeAmpareHour;	//step
	long				cycleSumDischargeWatt;		//step
	long				cycleSumDischargeWattHour;	//step
	long				cycleSumDischargeAmpareHour;//step

	unsigned long		cycleStepCount;
	long				cycleSumAvgT;
	
	long				sel_Cyc_C_Cap;	
	long				sel_Cyc_D_Cap;
	int					fileIndex;	//20180716 sch
	#endif
} S_SAVE_MSG_CH_DATA;
#endif

typedef struct s_save_msg_aux_data_tag {
	short int			auxChNo;
	short int			auxType;
	long				val;
} S_SAVE_MSG_AUX_DATA;

typedef struct s_save_msg_val_tag {
	S_SAVE_MSG_CH_DATA chData;
} S_SAVE_MSG_VAL;
typedef struct s_save_msg_tag {
	int					flag;
	int					write_idx[MAX_BD_PER_MODULE][MAX_CH_PER_BD];
	int					read_idx[MAX_BD_PER_MODULE][MAX_CH_PER_BD];
	int					count[MAX_BD_PER_MODULE][MAX_CH_PER_BD];
	S_SAVE_MSG_VAL		val[MAX_SAVE_MSG][MAX_BD_PER_MODULE][MAX_CH_PER_BD];
	S_SAVE_MSG_AUX_DATA auxData[MAX_SAVE_MSG][MAX_AUX_DATA];// 130226 oys add
} S_SAVE_MSG;

typedef struct s_pulse_msg_val_tag {
	int					type;
	long				runTime;
	long				Vsens;
	long				Isens;
	long				totalCycle;
	long				stepNo;
	long				capacity;
	long				wattHour;
} S_PULSE_MSG_VAL;

typedef struct s_pulse_msg_tag {
	int					flag;
	int					write_idx;
	int					read_idx;
	int					count;
	S_PULSE_MSG_VAL		val[MAX_PULSE_MSG];
	S_SAVE_MSG_CH_DATA	chData[MAX_10MS_MSG];
} S_PULSE_MSG;

typedef struct s_save10ms_msg_tag {
	int					flag;
	int					write_idx;
	int					read_idx;
	int					count;
//	S_PULSE_MSG_VAL		val[MAX_PULSE_MSG];
	S_SAVE_MSG_CH_DATA	chData[MAX_10MS_MSG];
} S_10MS_MSG;
#endif
