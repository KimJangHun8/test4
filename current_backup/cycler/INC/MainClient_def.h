#ifndef __MAINCLIENT_DEF_H__
#define __MAINCLIENT_DEF_H__

//packet
#define	MAX_MAIN_PACKET_LENGTH			(1024*512)
#define MAX_MAIN_PACKET_COUNT			128

// etc define
#define MAX_MAIN_CMD_SERIAL				100000000

//testcond define
#define MAX_STEP						250
#define MAX_CYCLE						84
#define MAX_SUB_STEP					10
#define MAX_COMP_POINT					3
#define MAX_GRADE_ITEM					4
#define MAX_GRADE_STEP					10
#define MAX_STEP_USER_PATTERN			10
#define MAX_OCV_TABLE_ROW				30
#define MAX_OCV_TABLE_COL				2
#define MAX_DATA_TABLE_ROW				30
#define MAX_DATA_TABLE_COL				30
#define MAX_RETRANS_DATA				31
#define MAX_CHK_VI_POINT				3

#if VENDER == 3		//190207
	#define MAX_USER_PATTERN_DATA			50000
#else 
	#define MAX_USER_PATTERN_DATA			20000
#endif
//command - machine : host
#define MAIN_CMD_TO_SBC_RESPONSE				0x00000001
#define MAIN_CMD_TO_SBC_SHUTDOWN				0x00000002
#define MAIN_CMD_TO_SBC_INITIALIZE				0x00000003
#define MAIN_CMD_TO_SBC_COMM_CHECK				0x00000004
#define MAIN_CMD_TO_SBC_COMM_CHECK_REPLY		0x00010004
#define MAIN_CMD_TO_SBC_MODULE_INFO_REQUEST		0x00000006
#define MAIN_CMD_TO_SBC_AUX_INFO_REQUEST		0x00000007
#define MAIN_CMD_TO_SBC_AUX_SET					0x00000008
#define MAIN_CMD_TO_SBC_CH_ATTRIBUTE_SET		0x00000009
#define MAIN_CMD_TO_SBC_CH_ATTRIBUTE_REQUEST	0x0000000A
#define MAIN_CMD_TO_SBC_CAN_SET					0x0000000B
#define MAIN_CMD_TO_SBC_CAN_INFO_REQUEST		0x0000000C
#define MAIN_CMD_TO_SBC_CH_COMPDATA_SET			0x0000000D
#define MAIN_CMD_TO_SBC_CH_COMPDATA_REQUEST		0x0000000E
#define MAIN_CMD_TO_SBC_LIMIT_USER_VI_SET		0x00000010	//20171008 sch add
#define MAIN_CMD_TO_SBC_LIMIT_USER_VI_REQUEST	0x00000011	//20171008 sch add
#define MAIN_CMD_TO_SBC_BUZZER_OFF				0x00000012  //190225 add
#define MAIN_CMD_TO_SBC_BUZZER_ON				0x00000013  //190225 add
#define MAIN_CMD_TO_SBC_DIO_CONTROL				0x00000014 	//190901 add
#define MAIN_CMD_TO_SBC_CALIMETER_SET			0x00000015 	//190901 lyhw
#define MAIN_CMD_TO_SBC_PARAMETER_CONTROL		0x00000016 	//190901 lyhw
#define MAIN_CMD_TO_SBC_TEMP_CALI_INFO_REQUEST	0x00000017 	//200102
#define MAIN_CMD_TO_SBC_TEMP_CALI_BACKUP_READ	0x00000018 	//200102
#define MAIN_CMD_TO_SBC_HWFAULT_CONFIG_SET		0x00000019 	//210204 lyhw
#define MAIN_CMD_TO_SBC_HWFAULT_CONFIG_REQUEST	0x00000020 	//210204 lyhw
#define MAIN_CMD_TO_SBC_CHAMBER_CH_NO_SET		0x00000021	//210415 hun
#define MAIN_CMD_TO_SBC_CHAMBER_CH_NO_REQUEST	0x00000022  //210415 hun
#define MAIN_CMD_TO_SBC_SW_FAULT_CONFIG_SET		0x00000023	//210415 hun
#define MAIN_CMD_TO_SBC_SW_FAULT_CONFIG_REQUEST	0x00000024  //210415 hun
#define	MAIN_CMD_TO_SBC_ACIR_VALUE				0x00000025	//220124 hun
#define MAIN_CMD_TO_SBC_CHAMBER_WAIT_RELEASE	0x00000026	//211022
#define MAIN_CMD_TO_SBC_LGES_FAULT_CONFIG_SET	0x00000027 	//211025 hun
#define MAIN_CMD_TO_SBC_LGES_FAULT_CONFIG_REQUEST	0x00000028 	//211025 hun
#define MAIN_CMD_TO_SBC_SDI_CC_CV_HUMP_CONFIG_SET	0x00000029 	//211125 hun
#define MAIN_CMD_TO_SBC_SDI_CC_CV_HUMP_CONFIG_REQUEST	0x00000030 	//211125 hun
#define MAIN_CMD_TO_SBC_SDI_PAUSE_SAVE_CONFIG_SET	0x00000031 	//211209 hun
#define MAIN_CMD_TO_SBC_SDI_PAUSE_SAVE_CONFIG_REQUEST	0x00000032 	//211209 hun
#define MAIN_CMD_TO_SBC_DYSON_MAINTENANCE_CONFIG_SET	0x00000033 	//220118 jsh
#define MAIN_CMD_TO_SBC_DYSON_MAINTENANCE_CONFIG_REQUEST	0x00000034 	//220118 jsh

#define MAIN_CMD_TO_SBC_RUN						0x00001000
#define MAIN_CMD_TO_SBC_STOP					0x00001001
#define MAIN_CMD_TO_SBC_PAUSE					0x00001002
#define MAIN_CMD_TO_SBC_CONTINUE				0x00001003
#define MAIN_CMD_TO_SBC_CLEAR					0x00001004
#define MAIN_CMD_TO_SBC_NEXT_STEP				0x00001005

#define MAIN_CMD_TO_SBC_MODULE_SET_DATA			0x00001012
#define MAIN_CMD_TO_SBC_TESTCOND_START			0x00001020
#define MAIN_CMD_TO_SBC_TESTCOND_STEP			0x00001021
#define MAIN_CMD_TO_SBC_TESTCOND_END			0x00001022
#define MAIN_CMD_TO_SBC_TESTCOND_SAFETY			0x00001023
#define MAIN_CMD_TO_SBC_STEP_COND_REQUEST		0x00001024
#define MAIN_CMD_TO_SBC_STEP_COND_UPDATE		0x00001025
#define MAIN_CMD_TO_SBC_SAFETY_COND_REQUEST		0x00001026
#define MAIN_CMD_TO_SBC_SAFETY_COND_UPDATE		0x00001027
#define MAIN_CMD_TO_SBC_RESET_RESERVED_CMD		0x00001028
#define MAIN_CMD_TO_SBC_TESTCOND_USER_PATTERN	0x00001029 // add <--20071106
#define MAIN_CMD_TO_SBC_SET_MEASURE_DATA		0X0000102A // add <--20071106
#define MAIN_CMD_TO_SBC_CH_INIT					0X0000102B // add <--20071106
#define MAIN_CMD_TO_SBC_TESTCOND_USER_MAP		0x0000102C // add <--20111215
#define MAIN_CMD_TO_SBC_CH_RECOVERY				0x0000102D // 20180716 sch
#define MAIN_CMD_TO_SBC_CALI_METER_CONNECT		0x00001030
#define MAIN_CMD_TO_SBC_CALI_START				0x00001032
#define MAIN_CMD_TO_SBC_CALI_UPDATE				0x00001035
#define MAIN_CMD_TO_SBC_CALI_NOT_UPDATE			0x00001036 	//180611 add for DIG
#define MAIN_CMD_TO_SBC_SET_SWELLING_DATA		0X00001037 	// add <--20071106
#define MAIN_CMD_TO_SBC_TESTCOND_TRACKING_FILE	0x00001040 	// add <--20210609
#define MAIN_CMD_TO_SBC_REAL_TIME_REPLY			0x00001050 	// add <--20131228
// 150512 oys add : chData Backup, Restore
#define MAIN_CMD_TO_SBC_CH_DATA_BACKUP			0x00001051 	// add <--20150512
#define MAIN_CMD_TO_SBC_CH_DATA_RESTORE			0x00001052 	// add <--20150512
#define MAIN_CMD_TO_SBC_GOTO_COUNT_RECOVERY		0x00001054 	// 20180716 sch
#define MAIN_CMD_TO_SBC_GUI_VERSION_INFO		0x00001055 	// 20190829 oys
#define MAIN_CMD_TO_SBC_PARAMETER_UPDATE		0x00001060 	//190801 lyhw
#define MAIN_CMD_TO_SBC_CELL_DIAGNOSIS_PAUSE	0x00001061 	//210806_kjc
#define MAIN_CMD_TO_SBC_CELL_DIAGNOSIS_STOP		0x00001062 	//210806_kjc
#define MAIN_CMD_TO_SBC_CELL_DIAGNOSIS_ALARM	0x00001063 	//210806_kjc
#define MAIN_CMD_TO_SBC_TEMP_CALI_POINT_SET		0x00001065 	//200102
#define MAIN_CMD_TO_SBC_TEMP_CALI_START			0x00001066 	//200102 
#define MAIN_CMD_TO_SBC_TEMP_CALI_UPDATE		0x00001067 	//200102 
#define MAIN_CMD_TO_SBC_HWFAULT_REQUEST			0x00001070 	//hun_200219
#define MAIN_CMD_TO_SBC_GOTO_COUNT_REQUEST		0x00001071	//hun_210929
#define MAIN_CMD_TO_SBC_GOTO_COUNT_DATA			0x00001072	//hun_210929

#define MAIN_CMD_TO_SBC_CH_END_DATA_RECOVERY	0x00001080 	//hun_200219
#define MAIN_CMD_TO_SBC_CH_CONVERT_ADV_CYCLE_STEP 0x00001090 	//hun_200219
#define	MAIN_CMD_TO_SBC_FAULT_CHANNEL_CODE		0x00001092 	//210417 LJS
#define	MAIN_CMD_TO_SBC_FAULT_ALARM_REQUEST		0x00001093  //210417 LJS
#define	MAIN_CMD_TO_SBC_SET_GAS_MEASURE_DATA	0x00001094  //210923 lyhw
#define	MAIN_CMD_TO_SBC_FAULT_GUI_CODE			0x00001095	//210917 LJS
#define	MAIN_CMD_TO_SBC_CHAMBER_FAULT			0x00001096	//220214_hun

#define	MAIN_CMD_TO_SBC_SCHEDULE_INFO			0x00004001  //hun_210824

#define MAIN_CMD_TO_PC_RESPONSE						0x00000001
#define MAIN_CMD_TO_PC_COMM_CHECK					0x00000004
#define MAIN_CMD_TO_PC_EMG_STATUS					0x00000005
#define MAIN_CMD_TO_PC_COMM_CHECK_REPLY				0x00010004
#define MAIN_CMD_TO_PC_MODULE_INFO_REPLY			0x00010006
#define MAIN_CMD_TO_PC_AUX_INFO_REPLY				0x00010007
#define MAIN_CMD_TO_PC_CH_ATTRIBUTE_REPLY			0x0001000A
#define MAIN_CMD_TO_PC_CH_COMPDATA_REPLY			0x0001000E
#define MAIN_CMD_TO_PC_CAN_INFO_REPLY				0x0001000C 	//120206 kji
#define MAIN_CMD_TO_PC_JIG_STATUS					0x0001000D 	//120206 kji
#define MAIN_CMD_TO_PC_LIMIT_USER_VI_REPLY			0x00010011	//171008 sch
#define MAIN_CMD_TO_PC_HWFAULT_CONFIG_REPLY			0x00010012	//171008 sch 
#define MAIN_CMD_TO_PC_CHAMBER_CH_NO_REPLY			0x00010013	//210415 hun
#define MAIN_CMD_TO_PC_SW_FAULT_CONFIG_REPLY		0x00010014	//210415 hun
#define MAIN_CMD_TO_PC_LGES_FAULT_CONFIG_REPLY		0x00010015	//211025 hun 
#define MAIN_CMD_TO_PC_SDI_CC_CV_HUMP_CONFIG_REPLY	0x00010016	//211125 hun 
#define MAIN_CMD_TO_PC_SDI_PAUSE_SAVE_CONFIG_REPLY	0x00010017	//211209 hun 
#define MAIN_CMD_TO_PC_DYSON_MAINTENANCE_CONFIG_REPLY	0x00010018	//220118 jsh 

#define MAIN_CMD_TO_PC_CH_DATA						0x00001010
#define MAIN_CMD_TO_PC_STATE_DATA					0x00001011
#define MAIN_CMD_TO_PC_CH_PULSE_DATA				0x00001013
#define MAIN_CMD_TO_PC_AUX_CH_DATA					0x00001014
#define MAIN_CMD_TO_PC_CH_10MS_DATA					0x00001015
#define MAIN_CMD_TO_PC_CH_PULSE_IEC_DATA			0x00001016	//171221

#define MAIN_CMD_TO_PC_STEP_COND_REPLY				0x00011024
#define MAIN_CMD_TO_PC_SAFETY_COND_REPLY			0x00011026
#define MAIN_CMD_TO_PC_SET_MEASURE_DATA				0X0000102A 	//071105
#define MAIN_CMD_TO_PC_CALI_METER_CONNECT_REPLY		0x00011030
#define MAIN_CMD_TO_PC_CALI_START_REPLY				0x00011032
#define MAIN_CMD_TO_PC_CALI_NORMAL_RESULT			0x00001033
#define MAIN_CMD_TO_PC_CALI_CHECK_RESULT			0x00001034
#define MAIN_CMD_TO_PC_REAL_TIME_REQUEST			0x00011050 	//131228
// 150512 oys add : chData Backup, Restore
#define MAIN_CMD_TO_PC_CH_DATA_BACKUP_REPLY			0x00011051 	//150512
#define MAIN_CMD_TO_PC_CH_DATA_RESTORE_REPLY		0x00011052 	//150512
// 160510 oys add : LGC Cycle Capacity Efficiency End
#define MAIN_CMD_TO_PC_STEP_REF_I_UPDATE			0x00011053 	//160510
#define MAIN_CMD_TO_PC_GOTO_COUNT_UPDATE			0x00011054 	//180716 sch
#define MAIN_CMD_TO_PC_GUI_VERSION_INFO_REPLY		0x00011055  //190916 oys
#define MAIN_CMD_TO_PC_TEMP_CALI_INFO_REPLY			0x00011060  //200102
#define MAIN_CMD_TO_PC_TEMP_CALI_BACKUP_READ_REPLY	0x00011061  //200102
#define MAIN_CMD_TO_PC_HWFAULT_REPLY				0x00011070 	//hun_200219
#define MAIN_CMD_TO_PC_GOTO_COUNT					0x00011071	//hun_210929
#define	MAIN_CMD_TO_PC_FAULT_ALARM_REPLY			0x00011093  //210417 LJS

// command error
#define MAIN_CMD_ID_ERROR		1
#define MAIN_SIZE_ERROR			2
#define MAIN_CHECK_SUM_ERROR	3
#define MAIN_BODY_SIZE_ERROR	4
#define MAIN_DIRECTION_ERROR	5
#define MAIN_BOX_ID_ERROR		6
#define MAIN_OBJECT_ID_ERROR	7
#define MAIN_PROCESS_ID_ERROR	8
#define MAIN_ETC_ERROR			9

//host in define
#define MACHINE_FORMATION			1
#define MACHINE_HOST				2
#define MACHINE_IR_OCV				3
#define MACHINE_OCV					4
#define MACHINE_AGING				5
#define MACHINE_CYCLER				6
#define EP_MODEL_ID_LENGTH			12

/* end pc <-> sbc command */

/* start response code */
//command
#define EP_CD_NACK						0x0000
#define EP_CD_ACK						0x0001
#define EP_CD_TIMEOUT					0x0002
#define EP_CD_SIZE_MISMATCH				0x0003
#define EP_CD_RX_BUF_OVERFLOW			0x0004
#define EP_CD_TX_BUF_OVERFLOW			0x0005

#define EP_CD_UNKNOWN_CMD				0x0011
#define EP_CD_MODULE_ID_ERROR			0x0012
#define EP_CD_UNKNOWN_CODE				0x0013
#define EP_CD_GROUP_ID_ERROR			0x0014
#define EP_CD_UNKNOWN_STEP_TYPE			0x0015
#define EP_CD_TEST_CONDITION_UNRCV		0x0016
#define EP_CD_COMPFLAG_IS_DISABLE		0x0017
#define EP_CD_CH_ID_ERROR				0x0018

#define EP_CD_SEQ_NO_ERROR				0x0
#define EP_CD_TEST_HEADER_UNRCV			0x0
#define EP_CD_TEST_STEP_UNRCV			0x0
#define EP_CD_TEST_STEP_COUNT_ERROR		0x0
#define EP_CD_TEST_STEP_ERROR			0x0
#define EP_CD_TYPE_MISMATCH				0x0
#define EP_CD_RANGE_MISMATCH			0x0
#define EP_CD_MODE_MISMATCH				0x0

#define EP_CD_DATA_SIZE_ERROR			0x0
#define EP_CD_CALI_RANGE_MISMATCH		0x0
#define EP_CD_TOTAL_CH_ERROR			0x0
#define EP_CD_PACKET_ID_ERROR			0x0

//group
#define EP_CD_GP_ISNT_IDLE				0x0100
#define EP_CD_GP_ISNT_STANDBY_OR_END	0x0101
#define EP_CD_GP_ISNT_RUN_OR_PAUSE		0x0102
#define EP_CD_GP_ISNT_RUN				0x0103
#define EP_CD_GP_ISNT_PAUSE				0x0104
#define EP_CD_GP_IS_ALL_FAULT			0x0105
#define EP_CD_GP_ISNT_IDLE_STANDBY		0x0106
#define EP_CD_GP_ISNT_IDLE_STANDBY_RUN	0x0107
#define EP_CD_GP_ISNT_STANDBY_END_RUN	0x0108
#define EP_CD_GP_ISNT_STANDBY_RUN		0x0109
#define EP_CD_GP_TOTAL_CH_ERROR			0x0

//channel
#define EP_CD_CH_ISNT_RUN				0x0200
#define EP_CD_CH_STATE_ERROR			0x0
#define EP_CD_CH_ISNT_STANDBY			0x0
                                    	
//jig                               	
#define EP_CD_JG_ISNT_READY				0x0300
#define EP_CD_JG_IS_LOCAL				0x0301
#define EP_CD_DOOR_IS_OPEN				0x0302
#define EP_CD_NVRAM_DISABLE				0x0

//system
#define EP_CD_FILE_WRITE_ERROR			0x0400
/* end response code */

// MainClient Signal
#define MAIN_SIG_NET_CONNECTED				0
#define MAIN_SIG_REST_END					1
#define MAIN_SIG_AC_POWER_FAIL_A			2
#define MAIN_SIG_SEND_RESULT_1_POWER_FAIL	3
#define MAIN_SIG_SEND_RESULT_2_POWER_FAIL	4
#define MAIN_SIG_CALCURATE_CHECK_RESULT		5
#define MAIN_SIG_SEND_CHECK_RESULT			6
#define MAIN_SIG_SEND_CMD_COMM_CHECK_TIMER	7
#define MAIN_SIG_SEND_ERROR_CODE			8
#define MAIN_SIG_TEST_HEADER_RCV			9
#define MAIN_SIG_TEST_STEP_RCV				10
#define MAIN_SIG_METER_CONNECT_REPLY		11
#define MAIN_SIG_NO_CONNECTION_RETRY		12
#define MAIN_SIG_TEST_STEP_USER_PATTERN_RCV		13
#define MAIN_SIG_SEND_REAL_TIME_REQUEST		14
//190225
#define MAIN_SIG_LAMP_BUZZER				15
#define MAIN_SIG_PARAMETER_UPDATE			16
#define MAIN_SIG_DISCONNECT_DAY				17	//210111 ljs

//180602 lyhw add for Meter_offset
#if CYCLER_TYPE == LINEAR_CYC || CYCLER_TYPE == CAN_CYC  
	#if SHUNT_R_RCV == 2
		#define MAX_OFFSET_POINT					10
	#else
		#define MAX_OFFSET_POINT					8
	#endif
#else
	#if SHUNT_R_RCV == 2
		#define MAX_OFFSET_POINT					10
	#else
		#define MAX_OFFSET_POINT					4
	#endif
#endif

#endif
