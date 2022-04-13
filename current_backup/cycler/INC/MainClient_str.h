#ifndef __MAINCLIENT_STR_H__
#define __MAINCLIENT_STR_H__

#include "MainClient_def.h"

typedef struct s_main_config_tag {
   	char		ipAddr[16];
	int			sendPort;
	int			receivePort;
	int			networkPort; //common port
	int			networkPort2; //common port
	int			networkPort3; //common port

   	int    		send_socket;
   	int    		receive_socket;
   	int    		network_socket; //common socket
   	int    		network_socket2; //common socket
   	int    		network_socket3; //common socket

	int			replyTimeout;
	int			retryCount;
	int			pingTimeout;
	int			netTimeout;
	
	unsigned char		CmdSendLog;
	unsigned char		CmdRcvLog;
	unsigned char		CmdSendLog_Hex;
	unsigned char		CmdRcvLog_Hex;
	
	unsigned char		CommCheckLog;
	unsigned char		reserved[3];
	
	unsigned long		send_monitor_data_interval;
	unsigned long		send_save_data_interval;
	unsigned int		protocol_version;
	unsigned int		state_change;
} S_MAIN_CONFIG;

typedef struct s_main_rcv_packet_tag {
	int					usedBufSize;

	int					rcvCount;
	int					rcvStartPoint[MAX_MAIN_PACKET_COUNT];
	int					rcvSize[MAX_MAIN_PACKET_COUNT];
	char				rcvPacketBuf[MAX_MAIN_PACKET_LENGTH];

	int					parseCount;
	int					parseStartPoint[MAX_MAIN_PACKET_COUNT];
} S_MAIN_RCV_PACKET;

typedef struct s_main_rcv_command_tag {
	int					cmdBufSize;
	char				cmd[MAX_MAIN_PACKET_LENGTH];
	char				cmdBuf[MAX_MAIN_PACKET_LENGTH];
	char				tmpBuf[MAX_MAIN_PACKET_LENGTH];
	int					cmdFail;
	int					cmdSize;

	int					rcvCmdIndex;
	char				rcvCmd[MAX_MAIN_PACKET_LENGTH];
	unsigned char		rcvCmdCompleteFlag;
	unsigned char		rcvCmdRestFlag;
	unsigned char		reserved1[2];
} S_MAIN_RCV_COMMAND;

typedef struct s_main_retry_data_tag {
	int					seqno;
	int					replyCmd;
	int					count;
	int					size;
	char				buf[MAX_MAIN_PACKET_LENGTH];
} S_MAIN_RETRY_DATA;

typedef struct s_main_reply_tag {
	int					timer_run;
	unsigned long		timer;
	S_MAIN_RETRY_DATA	retry;
} S_MAIN_REPLY;

typedef struct s_main_cmd_header_tag {
	unsigned long		cmd_id;
	unsigned long		cmd_serial;
	unsigned short		reserved1;
	unsigned short		reserved2;
	unsigned long		chFlag[2];
	unsigned long		body_size;
} S_MAIN_CMD_HEADER;

typedef struct s_main_test_cond_header_tag {
	unsigned char		totalStep;
	unsigned char		saveDtConfig;
	unsigned char		waitFlag;
	unsigned char		stepSyncFlag;
	unsigned char		tempWaitType;
	unsigned char		cycle_p_sch_flag;	//190318 add lyh
	unsigned char		cRateUseFlag;		//190801 oys add
	unsigned char		MasterFlag;			//LJS 210415
	long				reserved2;
#ifdef _EXTERNAL_CONTROL
	long				external_flag;		//hun_210723
#endif
#ifdef _TRACKING_MODE 
	unsigned char		connect_check_flag;
	unsigned char		reserved3[3];
#endif
} S_MAIN_TEST_COND_HEADER;

typedef struct s_main_adp_test_cond_safety_tag {
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
#ifdef _SDI	//sdi use
	#ifdef _SDI_SAFETY_V0
	long				reserved1[2];
	#endif
	#ifdef _SDI_SAFETY_V1
	long				faultSOH;		//hun_200219
	long				fault_deltaDv; 	//hun_200430
	#endif
	#ifdef _SDI_SAFETY_V2
	long				Master_Recipe_V;//LJS 210415
	long				reserved1[5];	//LJS 210415
	#endif
#endif
#ifndef _SDI //not sdi
	#if CHAMBER_TEMP_HUMIDITY == 1		//kjc_210525
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
	unsigned long		reserved2[2]; //211124 rewirte
	unsigned long		crate_factor;
	unsigned char		schedule_link_flag;
	unsigned char		schedule_link_flag_2; //211022
	unsigned char		reserved[2];
#endif
} S_MAIN_ADP_TEST_COND_SAFETY;

typedef struct s_main_adp_test_step_header_tag {
	int							type;
	unsigned char				stepNo;
	unsigned char				mode;
	unsigned char				testEnd;
	unsigned char				subStep;
	unsigned char				useSocFlag;
	unsigned char				reserved1[3];
} S_MAIN_ADP_TEST_STEP_HEADER;

typedef struct s_main_adp_test_step_reference_tag {
	long						refV; // x 100
	long						refI; // x 100
	short int					refTemp; // x 1000 add <--20071106
	char						integralCapFlag;
	char						endZeroVoltageFlag;	//<--20170626 oys modify
	unsigned long				endT;
	long						endV;
	long						endI;
	long						endC;
	long						GotoCondition;
	
	unsigned short				endTGoto;
	unsigned short				endIntegralCGoto;

	unsigned short				endVGoto;
	unsigned short				endIntegralWhGoto;

	unsigned short				endIGoto;
	unsigned char				jigPressTimeOutFlag;	//211025 lyhw
	unsigned char				reserved1;

	unsigned short				endCGoto;
	unsigned short				chamber_dev; //210422 add

	long						cycleCount;
	long						endDeltaV;
	short int					endSoc;
	unsigned short int			socCapStepNo;
	long						endWatt;
	long						endWattHour;
	short int					startTemp; // x 1000 add <--20071106
	//101028 kji add
	char						tempType;
	char						tempDir;
	short int					endTemp;  // x 1000 add <--20071106
	unsigned short 				endTempGoto;
	unsigned short				gotoCycleCount;
	#if NETWORK_VERSION > 4101
		#if VENDER != 2 //NOT SDI
		unsigned long				endT_CV;
		unsigned short				endTCVGoto;
		unsigned char				cycleEndStepSave;
		unsigned char				integralInit;
		#endif
	#endif
	unsigned short				endSocGoto;
	//170501 oys add
	#if VENDER == 3
	unsigned short				stepNo_pause;
	unsigned short				cycleNo_pause;

	short int					endP_soc;
	short int					endZ_soc;
	//standard Cycle or Step select value, standard Step type select value.
	unsigned short				endP_std_sel;
	unsigned short				endZ_std_sel;

	unsigned char				endP_std_type;
	unsigned char				endZ_std_type;
	unsigned char				endC_proc_type;
	unsigned char				endP_proc_type;
	unsigned char				endZ_proc_type;
	unsigned char				noTempWaitFlag;
	unsigned short				reduce_ratio_P;

	long						refV_L;
	long						refI_L;

	unsigned short				endC_std_cycleCount;
	unsigned short				endP_std_cycleCount;
	unsigned short				endZ_std_cycleCount;

	unsigned short				SocSoeFlag; //20190214
	long						reserved4;
	#endif
	#if AUX_CONTROL == 1
		#if NETWORK_VERSION >= 4103
		short int					auxType[MAX_AUX_FUNCTION];
		// 0: Not Use, 1: Temp, 2: Voltage
		unsigned char				auxCompareType[MAX_AUX_FUNCTION];
		// 0: Not Use, 1: <, 2: <=, 3: ==, 4: >, 5: >=, 6: !=
		unsigned char				reserved5[2];
		short int					auxGoto[MAX_AUX_FUNCTION];
		long						endAuxValue[MAX_AUX_FUNCTION];
		#endif
	#endif
	//210402 Cycle & Step Capacity End / Cycle & Step Current End / Cycle & Step C-Rate
	#if CAPACITY_CONTROL == 1
	short int					endCycleCapaRate[2]; 
	short int					endCycleCurrentRate[2]; 
	unsigned short int			endCycleCapaStepNo[2];
	unsigned short int			endCycleCurrentStepNo[2];
	long						endCycleCapaGoto; 
	long						endCycleCurrentGoto; 
	unsigned char				endCycleCapaSign[2];
	unsigned char				endCycleCurrentSign[2];
	int							UseCheckCapaFlag; 
	long						CycleCapaCount; 
	short int					C_Rate_Persent[10]; 
	unsigned short int			C_Rate_stepNo[10]; 
	unsigned char				C_Rate_Sign[10];
	unsigned char				reserved6[2];
	#endif
//cham : chamber, sync : synchronization
//T : Time, humid : humidity, dev : deviation
#if CHAMBER_TEMP_HUMIDITY == 1	//kjc_210412
	unsigned long				cham_sync_T;
	long						cham_temp;
	long						cham_humid;
	long						ch_temp;
	short int					cham_temp_dev;							
	short int					cham_humid_dev;
	short int					ch_temp_dev;
	short int					reserved6;
	char						cham_temp_sig;
	char						cham_humid_sig;
	char						ch_temp_sig;
	char						reserved7;
#endif	
#ifdef _TRACKING_MODE 
	unsigned char				SOC_Tracking_flag;
	unsigned char				SOH_Tracking_flag; //211022
	unsigned char				reserved6[1];
	unsigned char				trackingMode_flag;
	unsigned short int			rptSOC;
	unsigned short int			rptSOH; //211022
	unsigned short int			endSOC_branch;
	unsigned short int			reserved7;
	long						limitCurrent_Lower;
#endif
#if GAS_DATA_CONTROL == 1		//210923 lyhw
	long						endGasTVOC;
	long						endGasECo2;
	unsigned short				endGasTVOC_Goto;
	unsigned short				endGasECo2_Goto;
#endif
#ifdef _EQUATION_CURRENT		//211111
	unsigned char				equation_current_flag;
	unsigned char				reserved8[3];
	long						Max_Current_EQU;
	long						variable[4];
#endif
} S_MAIN_ADP_TEST_STEP_REFERENCE;

typedef struct s_main_adp_test_step_user_map { //add <-- 20071106
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
} S_MAIN_ADP_TEST_COND_USER_MAP;

typedef struct s_main_adp_test_step_user_pattern { //add <-- 20071106
	long						stepNo;
	long						length; //Data length change dynamical
	long						type; 	//data type
	char						UseFTP; //191122 not use
	char						reserved[3];
} S_MAIN_ADP_TEST_COND_USER_PATTERN;

typedef struct s_main_adp_test_step_user_pattern_data { // add <-- 20071106
	long						time;
	long						data;
} S_MAIN_ADP_TEST_COND_USER_PATTERN_DATA;

typedef struct s_main_adp_test_step_tracking_file { //210609
	long						stepNo;
	long						type; // 1:charge 2:discharge
	char						UseSOC;
	char						orgCh;
	char						reserved[2];
} S_MAIN_ADP_TEST_COND_TRACKING_FILE;

typedef struct s_main_adp_test_comp_cond_tag {
	long						lowerValue;
	long						upperValue;
	unsigned long				time;
} S_MAIN_ADP_TEST_COMP_COND;

typedef struct s_main_adp_test_delta_cond_tag {
	long						lowerValue;
	long						upperValue;
	unsigned long				time;
} S_MAIN_ADP_TEST_DELTA_COND;

//20190607 add for hyundae
typedef struct s_main_adp_test_change_v_cond_tag {
	long						lowerValue;
	long						upperValue;
	long						changeValue;
	unsigned long				time;
} S_MAIN_ADP_TEST_CHANGE_V_COND;

typedef struct s_main_adp_test_chk_vi_cond_tag {
	long						lowerValue[MAX_CHK_VI_POINT];
	long						upperValue[MAX_CHK_VI_POINT];
	unsigned long				time[MAX_CHK_VI_POINT];
} S_MAIN_ADP_TEST_CHK_VI_COND;
//20190607 add end 

typedef struct s_main_adp_testp_record_cond_tag {
	unsigned long				time;
	long						deltaV;
	long						deltaI;
	long						deltaT; //temperature
	long						deltaP;
	long						reserved;
} S_MAIN_ADP_TEST_RECORD_COND;

typedef struct s_main_adp_test_edlc_cond_tag {
	long						capacitanceV1;
	long						capacitanceV2;
	unsigned long				startT_Z;
	unsigned long				endT_Z;
	unsigned long				startT_LC;
	unsigned long				endT_LC;
} S_MAIN_ADP_TEST_EDLC_COND;

typedef struct s_main_adp_test_grade_step_tag {
	unsigned char				gradeCode;
	unsigned char				item;
	unsigned char				gradeProc;
	unsigned char				reserved;
	long						lowerValue;	//equal and upper(lowerValue <= x)
	long						upperValue;	//only lower(upperValue > x)
} S_MAIN_ADP_TEST_GRADE_STEP;

typedef struct s_main_adp_test_grade_cond_tag {
	unsigned char				item;
	unsigned char				gradeStepCount;
	short int					reserved1;

	S_MAIN_ADP_TEST_GRADE_STEP	gradeStep[MAX_GRADE_STEP];
} S_MAIN_ADP_TEST_GRADE_COND;

//210602 LJS 
#ifdef _END_COMPARE_GOTO
typedef struct s_main_end_comp_goto_tag {
	long				type;
	unsigned long		value;
	long				sign;
	long				gotoStepNo;
}S_MAIN_TEST_END_COMP_GOTO;
#endif

typedef struct s_main_adp_test_cond_step_tag {
	S_MAIN_ADP_TEST_STEP_HEADER		header;
	S_MAIN_ADP_TEST_STEP_REFERENCE	reference[MAX_SUB_STEP];
	S_MAIN_ADP_TEST_COMP_COND		compV[MAX_COMP_POINT];
	S_MAIN_ADP_TEST_COMP_COND		compI[MAX_COMP_POINT];
	S_MAIN_ADP_TEST_DELTA_COND		deltaV;
	S_MAIN_ADP_TEST_DELTA_COND		deltaI;
	S_MAIN_ADP_TEST_RECORD_COND		record;
	S_MAIN_ADP_TEST_EDLC_COND		edlc;
	S_MAIN_ADP_TEST_GRADE_COND		grade[MAX_GRADE_ITEM];
#ifdef _END_COMPARE_GOTO
	S_MAIN_TEST_END_COMP_GOTO		endCompGoto[MAX_COMP_GOTO]; //210602 LJS
#endif
	long							faultUpperV;
	long							faultLowerV;
	long							faultUpperI;
	long							faultLowerI;
	long							faultUpperC;
	long							faultLowerC;
	long							faultUpperZ;
	long							faultLowerZ;
	long							faultUpperTemp;
	long							faultLowerTemp;
	short int						faultUpperTemp_restart;
	short int						faultLowerTemp_restart;
	short int						pauseUpperTemp;
	short int						pauseLowerTemp;
#ifdef _SDI_SAFETY_V1
	long							faultRunTime;	//hun_200219
	long							reserved2[4];
#endif
#ifdef _ULSAN_SDI_SAFETY	//kjc_211008
	long							humpSet_T;
	long							humpSet_I;
#endif
#if CHANGE_VI_CHECK == 1
	S_MAIN_ADP_TEST_CHANGE_V_COND	changeV;
	S_MAIN_ADP_TEST_CHK_VI_COND		chk_V;
	S_MAIN_ADP_TEST_CHK_VI_COND		chk_I;
	long							faultRunTime;	//200703 ljs
	long							reserved1[3];
#endif
#if END_V_COMPARE_GOTO == 1
	long							endVGoto_upper;
	long							endVGoto_lower;
	unsigned short					endVupper_GotoStep;
	unsigned short					endVlower_GotoStep;
#endif
#ifdef _TRACKING_MODE 
	unsigned char					crate_flag;
	unsigned char					reserved[3];
#endif
#if GAS_DATA_CONTROL == 1			//210923 lyhw
	long							faultUpper_GasTVOC;
	long							faultLower_GasTVOC;
	long							faultUpper_GasECo2;
	long							faultLower_GasECo2;
#endif
#ifdef _GROUP_ERROR		//220203 hun
	long							group_StartVoltage;	
	long							group_CheckTime;	
	long							group_DeltaVoltage;	
	long							group_EndFaultTime;	
#endif
} S_MAIN_ADP_TEST_COND_STEP;

typedef struct s_main_test_condition_tag {
	S_MAIN_TEST_COND_HEADER		header;
	S_MAIN_ADP_TEST_COND_SAFETY	safety;
   	S_MAIN_ADP_TEST_COND_STEP	step[MAX_STEP];
	short int			stepCount;
} S_MAIN_TEST_CONDITION;

typedef struct s_main_response_tag {
	int					cmd;
	int					code;
} S_MAIN_RESPONSE;

typedef struct s_main_rcv_cmd_module_info_request_tag {
	S_MAIN_CMD_HEADER	header;
} S_MAIN_RCV_CMD_MODULE_INFO_REQUEST;

typedef struct s_main_rcv_cmd_module_set_data_tag {
	S_MAIN_CMD_HEADER	header;
	unsigned char		connection_retry;
	unsigned char		line_mode;
	unsigned char		control_mode;
	unsigned char		working_mode;
	unsigned int		auto_report_interval;
	unsigned int		data_save_interval;
	unsigned int		reserved1[16];
} S_MAIN_RCV_CMD_MODULE_SET_DATA;

typedef struct s_main_cmd_control_tag {
	long				stepNo;
	long				cycleNo;
	char				buzzerControl; //190225 add
	char				reserved2[3];
	long				reserved1[5];
} S_MAIN_CMD_CONTROL;

typedef struct s_main_rcv_cmd_run_tag {
	S_MAIN_CMD_HEADER	header;
	S_MAIN_CMD_CONTROL	control;
} S_MAIN_RCV_CMD_RUN;

typedef struct s_main_rcv_cmd_stop_tag {
	S_MAIN_CMD_HEADER	header;
	S_MAIN_CMD_CONTROL	control;
} S_MAIN_RCV_CMD_STOP;

typedef struct s_main_rcv_cmd_pause_tag {
	S_MAIN_CMD_HEADER	header;
	S_MAIN_CMD_CONTROL	control;
} S_MAIN_RCV_CMD_PAUSE;

typedef struct s_main_rcv_cmd_continue_tag {
	S_MAIN_CMD_HEADER	header;
} S_MAIN_RCV_CMD_CONTINUE;

typedef struct s_main_rcv_cmd_chamber_wait_release_tag { //211022
	S_MAIN_CMD_HEADER	header;
} S_MAIN_RCV_CMD_CHAMBER_WAIT_RELEASE;

typedef struct s_main_rcv_cmd_init_tag {
	S_MAIN_CMD_HEADER	header;
} S_MAIN_RCV_CMD_INIT;

typedef struct s_main_rcv_cmd_next_step_tag {
	S_MAIN_CMD_HEADER	header;
} S_MAIN_RCV_CMD_NEXT_STEP;

//181108 GUI TO SBC SHUTDOWN
typedef struct s_main_rcv_cmd_shutdown_tag {
	S_MAIN_CMD_HEADER	header;
	S_MAIN_CMD_CONTROL	control;
} S_MAIN_RCV_CMD_SHUTDOWN;

typedef struct s_main_rcv_cmd_parameter_update_tag{	//180801 lyhw 
	S_MAIN_CMD_HEADER	header;
//	unsigned char		reserved[4];
} S_MAIN_RCV_CMD_PARAMETER_UPDATE;


// 150512 oys add start : chData Backup, Restore
typedef struct s_main_rcv_cmd_ch_data_backup_tag {
	S_MAIN_CMD_HEADER	header;
} S_MAIN_RCV_CMD_CH_DATA_BACKUP;

typedef struct s_main_rcv_cmd_ch_data_restore_tag {
	S_MAIN_CMD_HEADER	header;
} S_MAIN_RCV_CMD_CH_DATA_RESTORE;
// 150512 oys add end : chData Backup, Restore

//190829 oys add : GUI VERSION INFO
typedef struct s_main_rcv_cmd_gui_version_info_tag {
	S_MAIN_CMD_HEADER	header;
} S_MAIN_RCV_CMD_GUI_VERSION_INFO;

typedef struct s_main_rcv_cmd_reset_tag {
	S_MAIN_CMD_HEADER	header;
} S_MAIN_RCV_CMD_RESET;

typedef struct s_main_rcv_cmd_testcond_start_tag {
	S_MAIN_CMD_HEADER	header;
	S_MAIN_TEST_COND_HEADER	testCondHeader;
} S_MAIN_RCV_CMD_TESTCOND_START;

typedef struct s_main_rcv_cmd_testcond_safety_tag {
	S_MAIN_CMD_HEADER	header;
	S_MAIN_ADP_TEST_COND_SAFETY	safety;
} S_MAIN_RCV_CMD_TESTCOND_SAFETY;

typedef struct s_main_rcv_cmd_testcond_step_tag {
	S_MAIN_CMD_HEADER	header;
	S_MAIN_ADP_TEST_COND_STEP	testCondStep;
} S_MAIN_RCV_CMD_TESTCOND_STEP;

typedef struct s_main_rcv_cmd_testcond_user_pattern_tag {// add <--20071106
	S_MAIN_CMD_HEADER	header;
	S_MAIN_ADP_TEST_COND_USER_PATTERN	userPattern;
} S_MAIN_RCV_CMD_TESTCOND_USER_PATTERN;

typedef struct s_main_rcv_cmd_testcond_map_tag {// add <--20111215
	S_MAIN_CMD_HEADER	header;
	S_MAIN_ADP_TEST_COND_USER_MAP userMap;
} S_MAIN_RCV_CMD_TESTCOND_USER_MAP;

#ifdef _TRACKING_MODE 
typedef struct s_main_rcv_cmd_testcond_tracking_mode_tag {
	S_MAIN_CMD_HEADER	header;
	S_MAIN_ADP_TEST_COND_TRACKING_FILE	tracking;
} S_MAIN_RCV_CMD_TESTCOND_TRACKING_FILE;
#endif

typedef struct s_main_rcv_cmd_testcond_end_tag {
	S_MAIN_CMD_HEADER	header;
} S_MAIN_RCV_CMD_TESTCOND_END;

typedef struct s_main_rcv_cmd_step_cond_request_tag {
	S_MAIN_CMD_HEADER	header;
	long				stepNo;
	long				reserved1[3];
} S_MAIN_RCV_CMD_STEP_COND_REQUEST;

typedef struct s_main_rcv_cmd_step_cond_update_tag {
	S_MAIN_CMD_HEADER	header;
} S_MAIN_RCV_CMD_STEP_COND_UPDATE;

typedef struct s_main_rcv_cmd_safety_cond_request_tag {
	S_MAIN_CMD_HEADER	header;
	long				stepNo;
	long				reserved1[3];
} S_MAIN_RCV_CMD_SAFETY_COND_REQUEST;

typedef struct s_main_rcv_cmd_safety_cond_update_tag {
	S_MAIN_CMD_HEADER	header;
} S_MAIN_RCV_CMD_SAFETY_COND_UPDATE;

typedef struct s_main_rcv_cmd_reset_reserved_cmd_tag {
	S_MAIN_CMD_HEADER	header;
} S_MAIN_RCV_CMD_RESET_RESERVED_CMD;

typedef struct s_main_rcv_cmd_response_tag {
	S_MAIN_CMD_HEADER	header;
	S_MAIN_RESPONSE		response;
} S_MAIN_RCV_CMD_RESPONSE;

typedef struct s_main_rcv_cmd_cali_meter_connect_tag {
	S_MAIN_CMD_HEADER	header;
	long				type;
} S_MAIN_RCV_CMD_CALI_METER_CONNECT;

typedef struct s_main_rcv_cmd_set_measure_data_tag {//add <--20071107
	S_MAIN_CMD_HEADER	header;
	unsigned short int	id;
	unsigned short int	type;
	long				data;
	unsigned char		chamberNo;
	unsigned char		reserved[3];
#if CHAMBER_TEMP_HUMIDITY == 1
	long				humi;	//hun_210227
	long				ch_temp[16];
#endif
} S_MAIN_RCV_CMD_SET_MEASURE_DATA;

typedef struct s_main_rcv_cmd_set_swelling_data_tag {	//210316 lyhw
	S_MAIN_CMD_HEADER	header;
	unsigned short int	id;
	unsigned short int	reserved1;
	long				Pressure[MAX_CH_PRESSURE_DATA];
	long				Thickness[MAX_CH_THICKNESS_DATA];
} S_MAIN_RCV_CMD_SET_SWELLING_DATA;

//210923 lyhw for lges gas control
typedef struct s_main_rcv_cmd_set_gas_measure_data_tag {
	S_MAIN_CMD_HEADER	header;
	unsigned short int	id;
	unsigned short int	reserved1;
	long	eCo2;
	long	Temp;
	long	AH;
	long	Baseline;
	long	TVOC;
	long	Ethanol;
	long	H2;
} S_MAIN_RCV_CMD_SET_GAS_MEASURE_DATA;

#pragma pack(1)
typedef struct s_main_cali_point_tag {
	unsigned char		setPointNum;
	unsigned char		checkPointNum;
	unsigned char		setCaliNum;
	unsigned char		checkCaliNum;
	long				setPoint[MAX_CALI_POINT];
	long				checkPoint[MAX_CALI_POINT];
#if SHUNT_R_RCV >= 1
	double				shuntValue;
	unsigned char		shuntSerialNo[MAX_SHUNT_SERIAL_LENGTH];
#endif
#if SHUNT_R_RCV == 2	//180515 add for hallct
	double				shuntValue2;
	double				meter_offset[MAX_OFFSET_POINT];
#endif
} S_MAIN_CALI_POINT;
#pragma pack()
typedef struct s_main_cali_tmp_cond_tag {
	int					type;
	int					range;
	int					mode;
//140701 nam w : actual measure
#if SHUNT_R_RCV >= 1
	int					meas;
	int					hallCT;
	unsigned long		interval;
#endif
	S_MAIN_CALI_POINT	point;
} S_MAIN_CALI_TMP_COND;

typedef struct s_main_rcv_cmd_cali_start_tag {
	S_MAIN_CMD_HEADER		header;
	S_MAIN_CALI_TMP_COND	tmpCond;
} S_MAIN_RCV_CMD_CALI_START;

typedef struct s_main_rcv_cmd_cali_update_tag {
	S_MAIN_CMD_HEADER	header;
} S_MAIN_RCV_CMD_CALI_UPDATE;

typedef struct s_main_rcv_cmd_cali_not_update_tag {	//180611 lyhw 
	S_MAIN_CMD_HEADER	header;
} S_MAIN_RCV_CMD_CALI_NOT_UPDATE;

typedef struct s_main_rcv_cmd_cali_lan_connect_tag {
	S_MAIN_CMD_HEADER	header;			//add <--161220 lyh
} S_MAIN_RCV_CMD_CALI_LAN_CONNECT;

#ifdef _TEMP_CALI 
typedef struct s_main_rcv_cmd_temp_cali_point_set_tag { //200102 add
	S_MAIN_CMD_HEADER	header;
	unsigned char		setPointCount;
	unsigned char		reserved1[3];
	long				setTempPoint[MAX_TEMP_POINT];
} S_MAIN_RCV_CMD_TEMP_CALI_POINT_SET;

typedef struct s_main_rcv_cmd_temp_cali_start_tag { //200102 add
	S_MAIN_CMD_HEADER	header;
	unsigned char		setPointNo;
	unsigned char		caliFlagCount;
	unsigned char		reserved1[2];
	
	unsigned char		temp_caliFlag[MAX_TEMP_CH];
	long				setTempValue;
}S_MAIN_RCV_CMD_TEMP_CALI_START;
typedef struct s_main_rcv_cmd_temp_cali_info_request_tag {
	S_MAIN_CMD_HEADER	header;
} S_MAIN_RCV_CMD_TEMP_CALI_INFO_REQUEST;

typedef struct s_main_send_cmd_temp_cali_info_reply_tag {
	S_MAIN_CMD_HEADER	header;
	short 				tempNo;
	short 				setPointCount;
	short 				reserved1[2];
	long				setTempPoint[MAX_TEMP_POINT];
} S_MAIN_SEND_CMD_TEMP_CALI_INFO_REPLY;

typedef struct s_main_rcv_cmd_temp_cali_backup_read_tag {	
	S_MAIN_CMD_HEADER	header;
} S_MAIN_RCV_CMD_TEMP_CALI_BACKUP_READ;

typedef struct s_main_send_cmd_temp_cali_backup_read_reply_tag {	
	S_MAIN_CMD_HEADER	header;
	unsigned char		response; // 0:NG, 1:OK
	unsigned char		reserved1[3];
} S_MAIN_SEND_CMD_TEMP_CALI_BACKUP_READ_REPLY;
#endif

typedef struct s_main_rcv_cmd_sensor_limit_set_tag {
	S_MAIN_CMD_HEADER	header;
	long				smokeUseFlag;
	long				smokeUpper;
} S_MAIN_RCV_CMD_SENSOR_LIMIT_SET;

typedef struct s_main_rcv_cmd_comm_check_tag {
	S_MAIN_CMD_HEADER	header;
} S_MAIN_RCV_CMD_COMM_CHECK;

typedef struct s_main_rcv_cmd_buzzer_off_tag { //190225 add
	S_MAIN_CMD_HEADER	header;
} S_MAIN_RCV_CMD_BUZZER_OFF;

typedef struct s_main_rcv_cmd_buzzer_on_tag { //190225 add
	S_MAIN_CMD_HEADER	header;
} S_MAIN_RCV_CMD_BUZZER_ON;

typedef struct s_main_rcv_cmd_dio_control_tag {			//190901 lyhw
	S_MAIN_CMD_HEADER	header;
	unsigned char		LampColor;
	unsigned char		FanSignal;
	char				reserved1[2];
	long				reserved2;
} S_MAIN_RCV_CMD_DIO_CONTROL;

typedef struct s_main_rcv_cmd_parameter_control_tag {	//190901 lyhw
	S_MAIN_CMD_HEADER	header;
	long				reserved[4];
} S_MAIN_RCV_CMD_PARAMETER_CONTROL;

typedef struct s_main_calimeter_set_tag {	//190901 lyhw
	unsigned char		comPort;
	unsigned char		Type;
	unsigned char		commType;
	unsigned char		reserved1;
	
	char				IP[16];
	long				Port;
	long				reserved2[2];
} S_MAIN_CALIMETER_SET;

typedef struct s_main_rcv_cmd_calimeter_set_tag {		//190901 lyhw
	S_MAIN_CMD_HEADER		header;
	S_MAIN_CALIMETER_SET	caliMeterSet;
} S_MAIN_RCV_CMD_CALIMETER_SET;

typedef struct s_main_rcv_cmd_comm_check_reply_tag {
	S_MAIN_CMD_HEADER	header;
	char				result[2];
	char				sended_cmd[4];
} S_MAIN_RCV_CMD_COMM_CHECK_REPLY;
//131228 oys w : real time add
#if REAL_TIME == 1
typedef struct s_main_rcv_cmd_real_time_reply_tag {
	S_MAIN_CMD_HEADER	header;
	char				real_time[24];
} S_MAIN_RCV_CMD_REAL_TIME_REPLY;
#endif
#ifdef _EXTERNAL_CONTROL
typedef struct s_main_rcv_cmd_cell_diagnosis_alarm_tag{
	S_MAIN_CMD_HEADER	header;
} S_MAIN_RCV_CMD_CELL_DIAGNOSIS_ALARM;

typedef struct s_main_rcv_cmd_schedule_info_tag{
	S_MAIN_CMD_HEADER	header;
	int					chIndex;
	unsigned char		pathName[260];
	unsigned char		scheduleName[128];
	unsigned char		serial[64];
	unsigned char		userId[32];
	unsigned char		descript[128];
	unsigned char		taryNo[64];
} S_MAIN_RCV_CMD_SCHEDULE_INFO;
#endif

#ifdef __LG_VER1__
typedef struct s_main_ch_data_tag {
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
} S_MAIN_CH_DATA;
#endif

#ifdef __SDI_MES_VER4__
typedef struct s_main_ch_data_tag {
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
	#ifdef _ACIR	//220124
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
} S_MAIN_CH_DATA;
#endif

typedef struct s_main_send_cmd_ch_data_tag {
	S_MAIN_CMD_HEADER	header;
	S_MAIN_CH_DATA		chData[MAX_CH_PER_MODULE];
} S_MAIN_SEND_CMD_CH_DATA;

//20180716 sch add for SBC Recovery 
typedef struct s_main_rcv_cmd_ch_recovery_tag {
	S_MAIN_CMD_HEADER	header;
	S_MAIN_CH_DATA		chData;
} S_MAIN_RCV_CMD_CH_RECOVERY;

typedef struct s_main_cmd_goto_count_recovery_tag {
	S_MAIN_CMD_HEADER	header;
	unsigned short ch;
	unsigned short LoopCount;
	unsigned short totalCycle;
	unsigned short gotoCycleCount[99];
} S_MAIN_CMD_GOTO_COUNT_RECOVERY;
//20180716 add end

typedef struct s_main_rcv_cmd_goto_count_request_tag {
	S_MAIN_CMD_HEADER	header;
	int					ch;
} S_MAIN_RCV_CMD_GOTO_COUNT_REQUEST;

typedef struct s_main_send_cmd_goto_count_tag {
	S_MAIN_CMD_HEADER	header;
	int			 		ch;
	unsigned short 		gotoCycleCount[MAX_STEP];
} S_MAIN_SEND_CMD_GOTO_COUNT;

typedef struct s_main_rcv_cmd_goto_count_data_tag {
	S_MAIN_CMD_HEADER	header;
	int					ch;
	unsigned short 		gotoCycleCount[MAX_STEP];
} S_MAIN_RCV_CMD_GOTO_COUNT_DATA;

//201010_SK_DATA_RECOVERY_s
//210916 SDI_DATA_Add
typedef struct s_main_rcv_cmd_ch_end_tag {
	short int			stepNo;
#if VENDER == 3
	long				capacity;
	long                Farad;	
	long				wattHour;
	long				z;
#endif	
#if VENDER == 2
	long				sel_Cyc_C_Cap;	
	long				sel_Cyc_D_Cap;
#endif	
} S_MAIN_CH_END_DATA;

typedef struct s_main_rcv_cmd_ch_end_datarecovery_tag {
	S_MAIN_CMD_HEADER	header;
	unsigned char		ch;
	unsigned char		reserved[3];
	int					total_step;
	S_MAIN_CH_END_DATA	chData[MAX_STEP];
} S_MAIN_RCV_CMD_CH_END_DATA_RECOVERY;

typedef struct s_main_rcv_cmd_ch_convert_adv_cycle_step_tag {
	S_MAIN_CMD_HEADER	header;
	unsigned char		ch;
	unsigned char		reserved[3];
} S_MAIN_RCV_CMD_CH_CONVERT_ADV_CYCLE_STEP;

//201010_SK_DATA_RECOVERY_e

typedef struct s_main_sned_cmd_sensor_data_tag {
	S_MAIN_CMD_HEADER	header;
	long				auxTemp[MAX_AUX_TEMP_DATA];
#if NETWORK_VERSION >= 4103
	long				auxVoltage[MAX_AUX_VOLT_DATA];
#endif
} S_MAIN_SEND_CMD_SENSOR_DATA;

typedef struct s_main_ch_attribute_tag {
	unsigned char		chNo_master;	//1base
	unsigned char		chNo_slave[3];	//1base
	unsigned char		opType;			//0:independent, 1:parallel
	unsigned char		reserved1[3];
} S_MAIN_CH_ATTRIBUTE;

typedef struct s_main_rcv_cmd_ch_attribute_set_tag {
	S_MAIN_CMD_HEADER		header;
	S_MAIN_CH_ATTRIBUTE	attr[MAX_CH_PER_MODULE];
} S_MAIN_RCV_CMD_CH_ATTRIBUTE_SET;

typedef struct s_main_rcv_cmd_ch_attribute_request_tag {
	S_MAIN_CMD_HEADER		header;
} S_MAIN_RCV_CMD_CH_ATTRIBUTE_REQUEST;

typedef struct s_main_send_cmd_ch_attribute_reply_tag {
	S_MAIN_CMD_HEADER		header;
	S_MAIN_CH_ATTRIBUTE	attr[MAX_CH_PER_MODULE];
} S_MAIN_SEND_CMD_CH_ATTRIBUTE_REPLY;

typedef struct s_main_chamber_ch_no_tag {
	unsigned char		chamberNo;	//1base
	unsigned char		hw_no;	//1base
	unsigned char		bd;			
	unsigned char		ch;
} S_MAIN_CHAMBER_CH_NO;

typedef struct s_main_rcv_cmd_chamber_ch_no_set_tag {
	S_MAIN_CMD_HEADER		header;
	S_MAIN_CHAMBER_CH_NO	attr[MAX_CH_PER_MODULE];
} S_MAIN_RCV_CMD_CHAMBER_CH_NO_SET;

typedef struct s_main_rcv_cmd_chamber_ch_no_request_tag {
	S_MAIN_CMD_HEADER		header;
} S_MAIN_RCV_CMD_CHAMBER_CH_NO_REQUEST;

typedef struct s_main_send_cmd_chamber_ch_no_reply_tag {
	S_MAIN_CMD_HEADER		header;
	S_MAIN_CHAMBER_CH_NO	attr[MAX_CH_PER_MODULE];
} S_MAIN_SEND_CMD_CHAMBER_CH_NO_REPLY;

//210428 hun
typedef struct s_main_swfault_config_tag {
	long		chamber_gas_voltage_min;
	long		chamber_gas_voltage_max;
	long		rest_check_start_time;
	long		rest_start_compare_voltage;
	long		rest_compare_voltage_delta_v1;	
	long		rest_compare_voltage_delta_v2;
	long		rest_fault_check_count;	
	long		gasCheckTime;
	long		ambient_temp_max;
	long		ambient_temp_max_checkTime;
	long		ambient_temp_diff;
	long		ambient_temp_diff_checkTime;
	long		soft_venting_count;
	long		soft_venting_value;
	long		hard_venting_value;
	long		reserved[5];
} S_MAIN_SW_FAULT_CONFIG;

typedef struct s_main_rcv_cmd_swfault_config_set_tag {
	S_MAIN_CMD_HEADER		header;
	S_MAIN_SW_FAULT_CONFIG	sw_fault_config;
} S_MAIN_RCV_CMD_SW_FAULT_CONFIG_SET;

typedef struct s_main_rcv_cmd_swfault_config_request_tag {
	S_MAIN_CMD_HEADER		header;
} S_MAIN_RCV_CMD_SW_FAULT_CONFIG_REQUEST;

typedef struct s_main_send_cmd_swfault_config_reply_tag {
	S_MAIN_CMD_HEADER		header;
	S_MAIN_SW_FAULT_CONFIG	sw_fault_config;
} S_MAIN_SEND_CMD_SW_FAULT_CONFIG_REPLY;


//20171008 sch add
typedef struct s_main_limit_user_vi_tag {
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
} S_MAIN_LIMIT_USER_VI;

typedef struct s_main_rcv_cmd_limit_user_vi_set_tag {
	S_MAIN_CMD_HEADER		header;
	S_MAIN_LIMIT_USER_VI	LimitVI;
} S_MAIN_RCV_CMD_LIMIT_USER_VI_SET;

typedef struct s_main_rcv_cmd_limit_user_vi_reply_tag {
	S_MAIN_CMD_HEADER		header;
	S_MAIN_LIMIT_USER_VI	LimitVI;
} S_MAIN_SEND_CMD_LIMIT_USER_VI_REPLY;

typedef struct s_main_rcv_cmd_limit_user_vi_request_tag {
	S_MAIN_CMD_HEADER		header;
} S_MAIN_RCV_CMD_LIMIT_USER_VI_REQUEST;

//210204 lyhw
typedef struct s_main_hwfault_config_tag {
	long			ovpVal;
	long			cccVal;
	long			otpVal;
	long			dropV_Charge;
	long			dropV_Discharge;
	long			cvFaultV;
	long			cvFaultI;
	long			reserved[2];
} S_MAIN_HWFAULT_CONFIG;

typedef struct s_main_rcv_cmd_hwfault_config_set_tag {
	S_MAIN_CMD_HEADER		header;
	S_MAIN_HWFAULT_CONFIG	hwFaultConfig;
} S_MAIN_RCV_CMD_HWFAULT_CONFIG_SET;

typedef struct s_main_rcv_cmd_hwfault_config_request_tag {
	S_MAIN_CMD_HEADER		header;
} S_MAIN_RCV_CMD_HWFAULT_CONFIG_REQUEST;

typedef struct s_main_send_cmd_hwfault_config_reply_tag {
	S_MAIN_CMD_HEADER		header;
	S_MAIN_HWFAULT_CONFIG	hwFaultConfig;
} S_MAIN_SEND_CMD_HWFAULT_CONFIG_REPLY;

//211025 hun
typedef struct s_main_lges_fault_config_tag {
	#if FAULT_CONFIG_VERSION >= 1
	long	ovp_check_time;
	long	otp_check_time;	
	long	drop_v_charge_start_time;	
	long	drop_v_discharge_start_time;	
	long	drop_v_charge_check_time;	
	long	drop_v_discharge_check_time;
	long	cv_voltage_start_time;
	long	cv_current_start_time;
	long	ovp_pause_flag;
	long	otp_pause_flag;
	long	limit_time_v;
	#endif
} S_MAIN_LGES_FAULT_CONFIG;

typedef struct s_main_rcv_cmd_lges_fault_config_set_tag {
	S_MAIN_CMD_HEADER			header;
	S_MAIN_LGES_FAULT_CONFIG	faultConfig;
} S_MAIN_RCV_CMD_LGES_FAULT_CONFIG_SET;

typedef struct s_main_rcv_cmd_lges_fault_config_request_tag {
	S_MAIN_CMD_HEADER		header;
} S_MAIN_RCV_CMD_LGES_FAULT_CONFIG_REQUEST;

typedef struct s_main_send_cmd_lges_fault_config_reply_tag {
	S_MAIN_CMD_HEADER		header;
	S_MAIN_LGES_FAULT_CONFIG	faultConfig;
} S_MAIN_SEND_CMD_LGES_FAULT_CONFIG_REPLY;

//211125 hun
typedef struct s_main_sdi_cc_cv_hump_config_tag {
	long	charge_voltage;
	long	charge_current;
	long	charge_cc_start_time;
	long	charge_cv_start_time;
	long	charge_cc_period_time;
	long	charge_cv_period_time;
	long	discharge_voltage;
	long	discharge_current;
	long	discharge_cc_start_time;
	long	discharge_cv_start_time;
	long	discharge_cc_period_time;
	long	discharge_cv_period_time;
} S_MAIN_SDI_CC_CV_HUMP_CONFIG;

typedef struct s_main_rcv_cmd_sdi_cc_cv_hump_config_set_tag {
	S_MAIN_CMD_HEADER			header;
	S_MAIN_SDI_CC_CV_HUMP_CONFIG	cc_cv_hump_config;
} S_MAIN_RCV_CMD_SDI_CC_CV_HUMP_CONFIG_SET;

typedef struct s_main_rcv_cmd_sdi_cc_cv_hump_config_request_tag {
	S_MAIN_CMD_HEADER		header;
} S_MAIN_RCV_CMD_SDI_CC_CV_HUMP_CONFIG_REQUEST;

typedef struct s_main_send_cmd_sdi_cc_cv_hump_config_reply_tag {
	S_MAIN_CMD_HEADER		header;
	S_MAIN_SDI_CC_CV_HUMP_CONFIG	cc_cv_hump_config;
} S_MAIN_SEND_CMD_SDI_CC_CV_HUMP_CONFIG_REPLY;

//211209 hun
typedef struct s_main_sdi_pause_save_config_tag {
	long	pause_end_time;
	long	pause_period_time;
} S_MAIN_SDI_PAUSE_SAVE_CONFIG;

typedef struct s_main_rcv_cmd_sdi_pause_save_config_set_tag {
	S_MAIN_CMD_HEADER			header;
	S_MAIN_SDI_PAUSE_SAVE_CONFIG	pause_save_config;
} S_MAIN_RCV_CMD_SDI_PAUSE_SAVE_CONFIG_SET;

typedef struct s_main_rcv_cmd_sdi_pause_save_config_request_tag {
	S_MAIN_CMD_HEADER		header;
} S_MAIN_RCV_CMD_SDI_PAUSE_SAVE_CONFIG_REQUEST;

typedef struct s_main_send_cmd_sdi_pause_save_config_reply_tag {
	S_MAIN_CMD_HEADER		header;
	S_MAIN_SDI_PAUSE_SAVE_CONFIG	pause_save_config;
} S_MAIN_SEND_CMD_SDI_PAUSE_SAVE_CONFIG_REPLY;

typedef struct s_main_rcv_cmd_chamber_fault_tag {
	S_MAIN_CMD_HEADER			header;
	long 	faultCode;			
	long 	pauseFlag;		// 0 - Pause , 1 - ShutDown
}S_MAIN_RCV_CMD_CHAMBER_FAULT;

typedef struct s_main_dyson_maintenance_config_tag {
	long	door_pause_flag;
} S_MAIN_DYSON_MAINTENANCE_CONFIG;

typedef struct s_main_rcv_cmd_dyson_maintenance_config_set_tag {
	S_MAIN_CMD_HEADER			header;
	S_MAIN_DYSON_MAINTENANCE_CONFIG		maintenanceConfig;
} S_MAIN_RCV_CMD_DYSON_MAINTENANCE_CONFIG_SET;

typedef struct s_main_rcv_cmd_dyson_maintenance_config_request_tag {
	S_MAIN_CMD_HEADER			header;
} S_MAIN_RCV_CMD_DYSON_MAINTENANCE_CONFIG_REQUEST;

typedef struct s_main_send_cmd_dyson_maintenance_config_reply_tag {
	S_MAIN_CMD_HEADER			header;
	S_MAIN_DYSON_MAINTENANCE_CONFIG		maintenanceConfig;
} S_MAIN_SEND_CMD_DYSON_MAINTENANCE_CONFIG_REPLY;

typedef struct s_main_rcv_cmd_recipe_channel_code_tag {
	S_MAIN_CMD_HEADER		header;
	unsigned short				Faultcode;
	unsigned short				reserved;
} S_MAIN_RCV_CMD_FAULT_CHANNEL_CODE;

typedef struct s_main_rcv_cmd_fault_alarm_tag {
	S_MAIN_CMD_HEADER		header;
} S_MAIN_RCV_CMD_FAULT_ALARM_REQUEST;

typedef struct s_main_fault_alarm_tag {
	long		faultval[MAX_FAULT_NUM];
} S_MAIN_SDI_FAULT_ALARM;
	
typedef struct s_main_send_cmd_fault_alarm_tag {
	S_MAIN_CMD_HEADER		header;
	S_MAIN_SDI_FAULT_ALARM	fault_alarm;
} S_MAIN_SEND_CMD_FAULT_ALARM_REPLY;

typedef struct s_main_ch_compdata_tag {
	unsigned char		useFlag;
	unsigned char		reserved1[3];
	long				compPlus;
	long				compMinus;
} S_MAIN_CH_COMPDATA;

typedef struct s_main_rcv_cmd_ch_compdata_set_tag {
	S_MAIN_CMD_HEADER		header;
	S_MAIN_CH_COMPDATA		compData[MAX_CH_PER_MODULE];
} S_MAIN_RCV_CMD_CH_COMPDATA_SET;

typedef struct s_main_rcv_cmd_ch_compdata_request_tag {
	S_MAIN_CMD_HEADER		header;
} S_MAIN_RCV_CMD_CH_COMPDATA_REQUEST;

typedef struct s_main_send_cmd_ch_compdata_reply_tag {
	S_MAIN_CMD_HEADER		header;
	S_MAIN_CH_COMPDATA		compData[MAX_CH_PER_MODULE];
} S_MAIN_SEND_CMD_CH_COMPDATA_REPLY;

typedef struct s_main_aux_set_data_tag {
	unsigned char		chNo;			//1base : machine channel number
	unsigned char		reserved1[3];
	short int			auxChNo;		//1base : aux channel number
	short int			auxType;		//0:temperature, 1:v
	char				name[MAX_AUX_NAME_SIZE];
	long				fault_upper;
	long				fault_lower;
	long				end_upper;
	long				end_lower;

	short int			function_div1;
	short int			function_div2;
	short int			function_div3;
	short int			reserved2;
} S_MAIN_AUX_SET_DATA;

typedef struct s_main_rcv_cmd_aux_set_tag {
	S_MAIN_CMD_HEADER		header;
	S_MAIN_AUX_SET_DATA	auxSetData[MAX_AUX_DATA];
} S_MAIN_RCV_CMD_AUX_SET;

typedef struct s_main_can_common_data_tag {
	unsigned char		can_baudrate;	//0:125K, 1:250K, 2:500K, 3:1M, 4:User
	unsigned char		extended_id;	//0:unused, 1:used
	unsigned char		reserved1[2];
	long				controller_canID;
	long				mask[2];
	long				filter[6];
	float				cell_cv_value;
	long				reserved2[2];
} S_MAIN_CAN_COMMON_DATA;

typedef struct s_main_can_normal_data_tag {
	unsigned char		canType;		//0:unused, 1:master, 2:slave
	unsigned char		byte_order;		//0:intel, 1:motolora
	unsigned char		data_type;	//0:unsigned, 1:signed, 2:float, 3:string
	unsigned char		cell_cv_flag;

	float				factor;
	short int			startBit;
	short int			bitCount;

	long				canID;			//hex value
	char				name[MAX_CAN_NAME_SIZE];
	float				fault_upper;
	float				fault_lower;
	float				end_upper;
	float				end_lower;
	long				reserved1;
} S_MAIN_CAN_NORMAL_DATA;

typedef struct s_main_can_set_data_tag {
	S_MAIN_CAN_COMMON_DATA	commonData[MAX_CH_PER_MODULE][MAX_CAN_TYPE];
	S_MAIN_CAN_NORMAL_DATA	normalData[MAX_CH_PER_MODULE][MAX_CAN_DATA];
} S_MAIN_CAN_SET_DATA;

typedef struct s_main_rcv_cmd_can_set_tag {
	S_MAIN_CMD_HEADER		header;
	S_MAIN_CAN_SET_DATA	canSetData;
} S_MAIN_RCV_CMD_CAN_SET;

typedef struct s_main_rcv_cmd_aux_info_request_tag {
	S_MAIN_CMD_HEADER		header;
} S_MAIN_RCV_CMD_AUX_INFO_REQUEST;

typedef struct s_main_send_cmd_aux_info_reply_tag {
	S_MAIN_CMD_HEADER		header;
	short int			installedTemp;
	short int			installedAuxV;
#if NETWORK_VERSION >= 4103
	short int			reserved1[2];
	S_MAIN_AUX_SET_DATA	auxSetData[MAX_AUX_DATA];
#endif
} S_MAIN_SEND_CMD_AUX_INFO_REPLY;

typedef struct s_main_rcv_cmd_can_info_request_tag {
	S_MAIN_CMD_HEADER		header;
} S_MAIN_RCV_CMD_CAN_INFO_REQUEST;

typedef struct s_main_send_cmd_can_info_reply_tag {
	S_MAIN_CMD_HEADER		header;
	short int			canDataCount[MAX_CH_PER_MODULE];
	S_MAIN_CAN_SET_DATA	canSetData;
} S_MAIN_SEND_CMD_CAN_INFO_REPLY;

typedef struct s_main_aux_data_tag {
	short int			auxChNo;		//1base
	short int			auxType;		//0:temperature, 1:v
	long				val;
} S_MAIN_AUX_DATA;

typedef union u_main_can_val_tag {
	unsigned long		ul_val[2];
	long				l_val[2];
	float				f_val[2];
	unsigned char		uc_val[8];
	char				c_val[8];
} U_MAIN_CAN_VAL;

typedef struct s_main_can_data_tag {
	unsigned char		canType;		//0:unused, 1:master, 2:slave
	unsigned char		data_type;	//0:unsigned, 1:signed, 2:float, 3:string
	unsigned char		reserved1[2];

	U_MAIN_CAN_VAL		val;
} S_MAIN_CAN_DATA;

typedef struct s_main_send_cmd_ch_data2_tag {
	S_MAIN_CMD_HEADER		header;
	S_MAIN_CH_DATA		chData[MAX_CH_PER_MODULE];
	S_MAIN_AUX_DATA		auxData[MAX_AUX_DATA];
	S_MAIN_CAN_DATA		canData[MAX_CAN_DATA];
} S_MAIN_SEND_CMD_CH_DATA2;

typedef struct s_main_ch_pulse_val_tag {
	long				runTime;
	long				Vsens;
	long				Isens;
	long				capacity;
	long				wattHour;
} S_MAIN_CH_PULSE_VAL;

typedef struct s_main_send_cmd_ch_pulse_data_tag {
	S_MAIN_CMD_HEADER	header;
	long				totalCycle;
	long				stepNo;
	long				dataCount;
	S_MAIN_CH_PULSE_VAL	val[MAX_PULSE_MSG];
} S_MAIN_SEND_CMD_CH_PULSE_DATA;

typedef struct s_main_send_cmd_ch_pulse_iec_data_tag {
	S_MAIN_CMD_HEADER	header;
	long				totalCycle;
	long				stepNo;
	long				dataCount;
	S_MAIN_CH_PULSE_VAL	val[MAX_PULSE_MSG];
} S_MAIN_SEND_CMD_CH_PULSE_IEC_DATA;

typedef struct s_main_send_cmd_ch_10ms_data_tag {
	S_MAIN_CMD_HEADER	header;
	long				dataCount;
	S_MAIN_CH_DATA		chData[MAX_10MS_MSG];
} S_MAIN_SEND_CMD_CH_10MS_DATA;

typedef struct s_main_module_info_tag {
	unsigned int		group_id;
	unsigned int		systemType;
	unsigned int		protocol_version;
	char				modelName[128];
	unsigned int		osVersion;
	unsigned short int	voltage_range;
	unsigned short int	current_range;
	unsigned int		voltage_spec[5];
	unsigned int		current_spec[5];
#ifdef _SDI_SAFETY_V2
	unsigned char		TempUseUnit;
	unsigned char		reserved3[7];
#else
	unsigned char		reserved1[8];
#endif
	unsigned short int	installedBd;
	unsigned short int	chPerBd;
	unsigned int		installedCh;
	unsigned int		totalJig;
	unsigned int		BdinJig[16];
	int					moduleConfig;
#if VERSION_DETAIL_SHOW == 1 
	char				program_version[18];
#else
	char				program_version[8];	
#endif
	int					reserved2;	
} S_MAIN_MODULE_INFO;

typedef struct s_main_send_cmd_module_info_reply_tag {
	S_MAIN_CMD_HEADER	header;
	S_MAIN_MODULE_INFO	md_info;
} S_MAIN_SEND_CMD_MODULE_INFO_REPLY;

//hun_200219_s
typedef struct s_main_rcv_cmd_module_hwfault_request_tag {
	S_MAIN_CMD_HEADER	header;
} S_MAIN_RCV_CMD_MODULE_HWFAULT_REQUEST;

typedef struct s_main_module_hwfault_tag {
	long	Drop_V_Charge;
	long	Drop_V_DisCharge;
	long	reserved[10];
} S_MAIN_MODULE_HWFAULT;

typedef struct s_main_send_cmd_hwfault_reply_tag {
	S_MAIN_CMD_HEADER	header;
	S_MAIN_MODULE_HWFAULT	Hwfault;
} S_MAIN_SEND_CMD_MODULE_HWFAULT_REPLY;
//hun_200219_e

typedef struct s_main_send_cmd_response_tag {
	S_MAIN_CMD_HEADER	header;
	S_MAIN_RESPONSE		response;
} S_MAIN_SEND_CMD_RESPONSE;

typedef struct s_main_send_cmd_meter_connect_reply_tag {
	S_MAIN_CMD_HEADER	header;
	long				state;
} S_MAIN_SEND_CMD_METER_CONNECT_REPLY;

typedef struct s_main_send_cmd_cali_start_reply_tag {
	S_MAIN_CMD_HEADER	header;
	S_MAIN_RESPONSE		response;
} S_MAIN_SEND_CMD_CALI_START_REPLY;

#pragma pack(1)
typedef struct s_main_cali_normal_result_tag {
	int					type;
	unsigned char		range;
	unsigned char		setPointNum;
	unsigned char		checkPointNum;
	unsigned char		ch;
	long				setPointAD[MAX_CALI_POINT];
	long				setPointDVM[MAX_CALI_POINT];
	long				checkPointAD[MAX_CALI_POINT];
	long				checkPointDVM[MAX_CALI_POINT];
#if SHUNT_R_RCV >= 1
	double				shuntValue;
	unsigned char		shuntSerialNo[MAX_SHUNT_SERIAL_LENGTH];
#endif
#if SHUNT_R_RCV == 2	//180515 add for hallct
	double				shuntValue2;
	double				meter_offset[MAX_OFFSET_POINT];
#endif
#ifdef _SK_CALI_TYPE
	long				set_meterValue[MAX_CALI_POINT];	
	long				check_meterValue[MAX_CALI_POINT];	
#endif
} S_MAIN_CALI_NORMAL_RESULT;
#pragma pack()
#pragma pack(1)
typedef struct s_main_cali_check_result_tag {
	int					type;
	unsigned char		range;
	unsigned char		reserved1;
	unsigned char		checkPointNum;
	unsigned char		ch;
	long				checkPointAD[MAX_CALI_POINT];
	long				checkPointDVM[MAX_CALI_POINT];
#if SHUNT_R_RCV >= 1
	double				shuntValue;
	unsigned char		shuntSerialNo[MAX_SHUNT_SERIAL_LENGTH];
#endif
#if SHUNT_R_RCV == 2	//180515 add for hallct 
	double				shuntValue2;
	double				meter_offset[MAX_OFFSET_POINT];
#endif
#ifdef _SK_CALI_TYPE
	long				check_meterValue[MAX_CALI_POINT];	
#endif
} S_MAIN_CALI_CHECK_RESULT;
#pragma pack()
typedef struct s_main_send_cmd_cali_normal_result_tag {
	S_MAIN_CMD_HEADER	header;
	S_MAIN_CALI_NORMAL_RESULT	result;
} S_MAIN_SEND_CMD_CALI_NORMAL_RESULT;

typedef struct s_main_send_cmd_cali_check_result_tag {
	S_MAIN_CMD_HEADER	header;
	S_MAIN_CALI_CHECK_RESULT	result;
} S_MAIN_SEND_CMD_CALI_CHECK_RESULT;

typedef struct s_main_send_cmd_comm_check_reply_tag {
	S_MAIN_CMD_HEADER	header;
} S_MAIN_SEND_CMD_COMM_CHECK_REPLY;

typedef struct s_main_send_cmd_comm_check_tag {
	S_MAIN_CMD_HEADER	header;
} S_MAIN_SEND_CMD_COMM_CHECK;

typedef struct s_main_send_cmd_emg_status_tag {
	S_MAIN_CMD_HEADER	header;
	long				code;
	long				val;
} S_MAIN_SEND_CMD_EMG_STATUS;

typedef struct s_main_send_cmd_jig_status_tag {
	S_MAIN_CMD_HEADER	header;
} S_MAIN_SEND_CMD_JIG_STATUS;

typedef struct s_main_send_cmd_step_cond_reply_tag {
	S_MAIN_CMD_HEADER	header;
	S_MAIN_ADP_TEST_COND_STEP	testCondStep;
} S_MAIN_SEND_CMD_STEP_COND_REPLY;

// 160510 oys add
typedef struct s_main_send_cmd_step_ref_i_update_tag {
	S_MAIN_CMD_HEADER	header;
	long changeRefI[MAX_STEP];
} S_MAIN_SEND_CMD_STEP_REF_I_UPDATE;
//add end
//
typedef struct s_main_send_cmd_safety_cond_reply_tag {
	S_MAIN_CMD_HEADER	header;
	S_MAIN_ADP_TEST_COND_SAFETY	safety;
} S_MAIN_SEND_CMD_SAFETY_COND_REPLY;
//131228 oys w : real time add
#if REAL_TIME == 1
typedef struct s_main_send_cmd_real_time_request_tag {
	S_MAIN_CMD_HEADER	header;
} S_MAIN_SEND_CMD_REAL_TIME_REQUEST;
#endif

// 150512 oys add start : chData Backup, Restore
typedef struct s_main_send_cmd_ch_data_backup_reply_tag {
	S_MAIN_CMD_HEADER	header;
	unsigned int		flag;
} S_MAIN_SEND_CMD_CH_DATA_BACKUP_REPLY;

typedef struct s_main_send_cmd_ch_data_restore_reply_tag {
	S_MAIN_CMD_HEADER	header;
	unsigned int		flag;
} S_MAIN_SEND_CMD_CH_DATA_RESTORE_REPLY;
// 150512 oys add end : chData Backup, Restore

typedef struct s_main_send_cmd_set_measure_data_tag {//add <--20071107
	S_MAIN_CMD_HEADER	header;
	unsigned short int	id;
	unsigned short int	type;
	long				data;
	long				reserved;
} S_MAIN_SEND_CMD_SET_MEASURE_DATA;

//220124 hun_s
typedef struct s_main_rcv_cmd_acir_value_tag {
	S_MAIN_CMD_HEADER	header;
	int					ch;
	long				voltage;
	long				acir;
} S_MAIN_RCV_CMD_ACIR_VALUE;
//220124 hun_e

typedef struct s_main_misc_tag {
	unsigned long		timer;
	unsigned long		network_timer;
	unsigned long		cmd_serial;
	int					psSignal;
	int					processPointer;
// 131228 oys w : real time add
#if REAL_TIME == 1
	long				sent_real_time_request[3];
#endif
} S_MAIN_MISC;

typedef struct s_main_ch_pulse_data_tag {
	long				totalCycle;
	long				stepNo;
	long				dataCount;
	S_PULSE_MSG_VAL		val[MAX_PULSE_MSG];
} S_MAIN_CH_PULSE_DATA;

typedef struct s_main_client_tag {
	S_MAIN_MISC			misc;
	S_MAIN_CONFIG		config;
	
	S_MAIN_RCV_PACKET	rcvPacket;
	S_MAIN_RCV_COMMAND	rcvCmd;

	S_MAIN_REPLY		reply;
	unsigned long		pingTimer;
	unsigned long		netTimer;
	unsigned long		chDataTimer;
	
	S_MAIN_TEST_CONDITION	testCond;
	S_MAIN_CH_PULSE_DATA	chPulseData[MAX_CH_PER_MODULE];
	S_MAIN_CH_PULSE_DATA	chPulseData_iec[MAX_CH_PER_MODULE]; //171227
	S_MAIN_CH_PULSE_DATA	fadmPulseData[MAX_CH_PER_MODULE];
	
	unsigned char		signal[MAX_SIGNAL];
	unsigned long		sended_monitor_data_time;
} S_MAIN_CLIENT;

#endif
