#ifndef __MODULECONTROL_DEF_H__
#define __MODULECONTROL_DEF_H__

//module state
#define	M_IDLE						0x00
#define	M_SELFTEST					0x01
#define M_STANDBY					0x02
#define M_RUN						0x03
#define M_FAIL						0x04

//channel state
#define	C_IDLE						0x00
#define	C_STANDBY					0x01
#define C_RUN						0x02
#define C_PAUSE						0x03
#define C_CALI						0x04
#define C_CALI_UPDATE				0x05	//180813 lyh add

//step type
#define STEP_IDLE					0
#define	STEP_CHARGE					1
#define	STEP_DISCHARGE				2
#define	STEP_REST					3
#define	STEP_OCV					4
#define	STEP_Z						5
#define	STEP_END					6
#define STEP_ADV_CYCLE				7
#define STEP_LOOP					8
#define STEP_USER_PATTERN			9
#define STEP_BALANCE				10
#define STEP_USER_MAP				11
#define STEP_SHORT					12 //140407 oys add
#define STEP_PARALLEL_CYCLE			13 //kjg_180416
#define	STEP_ACIR					14 //220124 hun

//CMD V DIRECTION
#define	CMD_V_IDLE					0
#define	CMD_V_PLUS					1
#define CMD_V_MINUS					2

//step mode
#define MODE_IDLE					0
#define	CCCV						1
#define	CC							2
#define	CV							3
#define DC							4
#define AC							5
#define CP							6
#define CCP							7
#define CR							8
#define CPCV						9
#define CPP							10	//180611 lyhw
#define C_RATE						11	//210402

//170501 oys add Cycle Efficiency End
#define	CAPACITY_END				0
#define WATTHOUR_END				1
#define DCIR_END					2

//180611 lyhw
//step pcu mode
#define PCU_RESET					0
#define PCU_CC						1
#define PCU_CCP						2
#define PCU_CV						3
#define PCU_CCCV					4
#define PCU_CP						5
#define PCU_CPP						6
#define PCU_CR						7
#define PCU_CPCV					10
//semi switch state
#define SEMI_IDLE					0
#define SEMI_PRE					1
#define SEMI_V_P					2
#define SEMI_V_N					3
#define SEMI_REST					4

//210511 LJS FOR LGES END GOTO ADD
#define UNUSED						0
#define TIME						1
#define VOLTAGE						2
#define CURRENT						3
#define AMPAREHOUR					4
#define WATTHOUR					5
#define TEMP						6

//module signal
#define M_SIG_MODULE_PROCESS		0
#define M_SIG_LAMP_RUN				1
#define M_SIG_LAMP_STOP				2
#define M_SIG_CALI_RELAY			3
#define M_SIG_REMOTE_SMPS1			4
#define M_SIG_LAMP_PAUSE			5

#define M_SIG_RUN_LED				6
#define M_SIG_FAN_RELAY				7
#define M_SIG_POWER_OFF				8
#define M_SIG_SMPS_FAULT			9
#define M_SIG_OT_FAULT				10
#define M_SIG_SEND_ERROR_CODE		11
#define M_SIG_TEST_SEND_SAVE_DATA	12
#define M_SIG_CALI_RELAY2			13
#define M_SIG_NET_CHECK				14
#define M_SIG_CALI_CHARGE_RELAY		15
#define M_SIG_CALI_DISCHARGE_RELAY	16
#define M_SIG_SMPS_FAULT1			17
#define M_SIG_OT_FAULT1				18
#define M_SIG_REMOTE_SMPS_DELAY		19
#define M_SIG_FAN_DELAY				20
#define M_SIG_INV_FAULT				21
#define M_SIG_END_BUZZER			22
#define M_SIG_LAMP_BUZZER			23
#define M_SIG_INV_AC_FAULT			24
#define M_SIG_MAIN_MC_OFF			25		//220214_hun

//160701 SCH Add
#define M_SIG_SMOKE_FAULT			31
#define	M_SIG_DOOR_OPEN_FAULT		32

//181108 GUI TO SBC SHUTDOWN
#define	M_SIG_GUI_TO_SBC_SHUTDOWN	33

#ifdef _JIG_TYPE_1
#define M_SIG_JIG_PASS_LAMP			25
#define M_SIG_JIG_FAIL_LAMP			26
#define M_SIG_JIG_START_LAMP		27
#define M_SIG_JIG_STOP_LAMP			28
#define M_SIG_JIG_BUZZER			29
#define M_SIG_JIG_SOL				30
#endif

//180611 lyhw add dig
#define M_SIG_INV_POWER				31
#define M_SIG_INV1_RUN				32
#define M_SIG_INV2_RUN				33
#define M_SIG_INV3_RUN				34
#define M_SIG_INV4_RUN				35
#define M_SIG_INV5_RUN				36
#define M_SIG_INV6_RUN				37
#define M_SIG_INV7_RUN				38
#define M_SIG_INV8_RUN				39
#define M_SIG_INV_POWER1			40
#define M_SIG_INV_POWER_CAN			41	//201208
//#define M_SIG_FIRE					40 //1700808 add for INV 8EA
//180927 add lyhw
#define M_SIG_CALI_VOLTAGE2_RELAY	50	
#define M_SIG_CALI_CHARGE2_RELAY	51	
#define M_SIG_CALI_DISCHARGE2_RELAY	52	
#define M_SIG_TEMP_FAULT1			53
#define M_SIG_TEMP_FAULT2			54	
//210308 lyhw for CAN JIG I_OUT
#if CYCLER_TYPE == CAN_CYC
#define M_SIG_CAN1_0				55
#define M_SIG_CAN1_1				56
#define M_SIG_CAN1_2				57
#define M_SIG_CAN1_3				58
#define M_SIG_CAN1_4				59
#define M_SIG_CAN1_5				60
#define M_SIG_CAN1_6				61
#define M_SIG_CAN1_7				62
#define M_SIG_CAN2_0				63
#define M_SIG_CAN2_1				64
#define M_SIG_CAN2_2				65
#define M_SIG_CAN2_3				66
#define M_SIG_CAN2_4				67
#define M_SIG_CAN2_5				68
#define M_SIG_CAN2_6				69
#define M_SIG_CAN2_7				70
#endif
#define M_SIG_MAIN_CAN_ERR_TOTAL	71

//board signal
#define B_SIG_RESET_LATCHED			0
#define B_SIG_HW_FAULT_CLEAR		1
#define B_SIG_MAIN_CAN_COMM_ERROR	2

//channel signal
#define	C_SIG_SELFTEST_START		0
#define C_SIG_SELFTEST_END			1
#define C_SIG_RUN					2
#define C_SIG_PAUSE					3
#define C_SIG_CONTINUE				4
#define C_SIG_NEXTSTEP				5
#define C_SIG_STOP					6
#define C_SIG_RESET					7
#define C_SIG_CALI					9
#define C_SIG_CALI_CHECK			10
#define C_SIG_SAVED_FILE_DELETE		11
#define C_SIG_FAIL_USER_PATTERN_READ	12

#define C_SIG_CALI_POINT			13
#define C_SIG_CALI_PHASE			14
#define C_SIG_METER_REPLY			15
#define C_SIG_METER_ERROR			16
#define C_SIG_OUT_SWITCH			17
#define C_SIG_V_RANGE				18
#define C_SIG_I_RANGE				19
#define C_SIG_C_D_SELECT			20
#define C_SIG_OT_FAULT				21
#define C_SIG_SMPS_FAULT			22
#define C_SIG_CALI_NORMAL_RESULT_SAVED	23
#define C_SIG_CALI_CHECK_RESULT_SAVED	24
#define C_SIG_FORCE_POWER			25
#define C_SIG_MAIN_EMG				26
#define C_SIG_SUB_EMG				27
#define C_SIG_AC_POWER				28
#define C_SIG_UPS_BATTERY			29
#define C_SIG_HW_FAULT				30
#define C_SIG_V_CMD_OUTPUT			31
#define C_SIG_I_CMD_OUTPUT			32
#define C_SIG_TRIGGER				33
#define C_SIG_ACTIVE_SWITCH			34
#define C_SIG_RANGE_SWITCH			35
#define C_SIG_CP_DCR_FLAG			36
#define C_SIG_LIMIT_ERROR			37
#define C_SIG_CHAMBER_ERROR			38
#define C_SIG_PARALLEL_SWITCH		39
#define C_SIG_TERMINAL_QUIT			40
#define C_SIG_INV_FAULT				41
#define C_SIG_FAIL_USER_MAP_READ	42
#define C_SIG_VL_PLUS_FAULT         43
#define C_SIG_VL_MINUS_FAULT        44
#define C_SIG_CH_V_LEVEL_FAULT      45
#define C_SIG_OV_FAULT              46
#define C_SIG_OC_FAULT              47
#define C_SIG_OT_COMPARE_FAULT      48
#define C_SIG_METER_HIGH_FAULT      49
#define C_SIG_METER_LOW_FAULT       50
#define C_SIG_JIG_ERROR				51
#define C_SIG_TEMP_CONNECT_ERROR	52
#define C_SIG_NETWORK_CONNECT_ERROR	53
#define C_SIG_SHORT_TESTER_ERROR	54		//140407
#define C_SIG_DC_FAN_FAIL			55		//151023
#define C_SIG_CALIMETER2_REPLY		56 //20160229
#define C_SIG_CALIMETER2_ERROR		57 //20160229
#define C_SIG_DOOR_OPEN_FAIL		58	//160701 SCH
#define C_SIG_SMOKE_FAIL			59	//160701 SCH
#define C_SIG_LIMIT_CURRENT			60	//20171024 sch add
#define C_SIG_LIMIT_VOLTAGE			61	//20171024 sch add
#define C_SIG_IEC_START				62	//20171227 add
#define C_SIG_OUT_SWITCH_ON			63
#define C_SIG_OUT_SWITCH_OFF		64
#define C_SIG_PARALLEL_SWITCH_ON	65
#define C_SIG_PARALLEL_SWITCH_OFF	66
#define C_SIG_DIGITAL_INV_STOP		67	//180620 add
#define C_SIG_DIGITAL_INV_RUN		68  //180620 add
#define C_SIG_DIGITAL_INV_RESET		69  //180620 add
#define C_SIG_D_CALI_UPDATE			70  //180620 add
//#define C_SIG_INV_POWER1			75	//180829 add
//20190605 KHK---------------------------
#define C_SIG_RANGE_SWITCH_ON		71
#define C_SIG_RANGE_SWITCH_OFF		72
#define C_SIG_MAIN_CAN_COMM_ERROR	73
#define C_SIG_INV_CAN_COMM_ERROR	74
#define C_SIG_IO_CAN_COMM_ERROR		75
//---------------------------------------
#define C_SIG_SHUTDOWN				80	//181108
#define C_SIG_M_RECIPE_CONNECT_ERROR	81
#define C_SIG_READ_FAIL_SOC_TRACKING_FILE	82 //210609
#define C_SIG_DLL_STOP				83 //210817
#define C_SIG_DLL_PAUSE				84 //210817
#define	C_SIG_OVP_PAUSE				85 //210901 LJS
#define	C_SIG_OTP_PAUSE				86 //210901 LJS
#define	C_SIG_OVP_GROUP_PAUSE		87 //211018 HUN
#define	C_SIG_OTP_GROUP_PAUSE		88 //211018 HUN
#define C_SIG_SELECT_CH_CONNECT_ERROR	90 //211022
#define	C_SIG_EQUATION_CALC_ERROR	91	//211111

//watchdog timer bits
#define WDT_BIT_ERROR				0xFE
#define WDT_BIT_UNREADY				0x00
#define WDT_BIT_READY				0x01
#define WDT_BIT_TEST				0x02
#define WDT_BIT_CLEAR				0x01

//sbc watchdog timer(hs_6637)
#define WDT_ADDR_ENABLE_HS_6637		0x443
#define WDT_ADDR_REFRESH_HS_6637	0x443
#define WDT_ADDR_DISABLE_HS_6637	0x045

//sbc watchdog timer(web_6580)
#define WDT_ADDR_ENABLE_WEB_6580	0x543
#define WDT_ADDR_REFRESH_WEB_6580	0x543
#define WDT_ADDR_DISABLE_WEB_6580	0x543

//sbc watchdog timer(hs_4020)
#define WDT_ADDR_ENABLE_HS_4020		0x500

//sbc watchdog timer(wafer_e669)
#define WDT_ADDR_ENABLE_WAFER_E669	0x443
#define WDT_ADDR_REFRESH_WAFER_E669	0x443
#define WDT_ADDR_DISABLE_WAFER_E669	0x043

//realtime define
#define RTTASK_CYCLE_PERIOD			10		//100mS
#define	RTTASK_5000MS				(RTTASK_CYCLE_PERIOD * 50)
#define RTTASK_2000MS				(RTTASK_CYCLE_PERIOD * 20)
#define RTTASK_1500MS				(RTTASK_CYCLE_PERIOD * 15)
#define RTTASK_1000MS				(RTTASK_CYCLE_PERIOD * 10)
#define RTTASK_500MS				(RTTASK_CYCLE_PERIOD * 5)

//sbc type
#define HS_6637						1
#define WEB_6580					2
#define HS_4020						3
#define WAFER_E669					4
#define WAFER_MARK					5
#define EM104_A5362					6
//system type
#define SYSTEM_FORMATION			0
#define SYSTEM_IR_OCV				1
#define SYSTEM_AGING				2
#define SYSTEM_GRADER				3
#define SYSTEM_SELECTOR				4
#define SYSTEM_OCV					5
#define SYSTEM_CYCLER				6

//hardware spec
#define L_5V_3A						0
#define L_6V_6A						1
#define L_5V_2A						2
#define L_5V_200A					3
#define L_5V_10mA					4
#define L_5V_5A						5
#define L_5V_30A					6
#define L_2V_60A					7
#define L_2V_100A					8
#define L_5V_5A_2					9
#define L_5V_50A					10
#define L_5V_20A					11
#define L_50V_50A					12
#define L_5V_10A					13
#define L_2V_1A						14
#define L_20V_25A					15
#define L_5V_100A					16
#define L_5V_200A_R2				17
#define L_5V_100A_R2				18
#define L_5V_30A_R1					19
#define L_5V_100A_R1				20
#define L_5V_2A_R1					21
#define L_5V_500A_R1				22
#define L_5V_200A_R4				23
#define L_5V_100A_R1_EG				24
#define L_5V_150A_R1				25
#define L_5V_250A_R1				26
#define L_5V_50A_R1					27
#define L_5V_1000A_R1				28
#define L_5V_300A_R1				29
#define L_20V_10A_R1				30
#define L_20V_50A_R2				31
#define L_5V_50A_R2					32
#define L_5V_2A_R2					33
#define L_5V_500A_R2				34
#define L_5V_200A_R3				35
#define L_10V_5A_R2					36
#define L_5V_300A_R3				37
#define L_5V_1000A_R3				38
#define L_5V_150A_R3				39
#define L_5V_50A_R2_1				40
#define L_5V_100A_R2_1				41
#define L_20V_5A_R1					42
#define L_5V_4A_R2					43
#define L_20V_300A_R2				44
#define L_5V_120A_R3				45
#define L_5V_220A_R2				46
#define L_5V_5A_R2					47
#define L_5V_10_30A_R2				48
#define L_5V_65A_R3					49
#define L_5V_500mA_R2				50
#define L_5V_250A_R2				51
#define L_5V_100mA_R2				52
#define L_3V_200A_R2				53
#define L_16V_200A_R2				54
#define L_20V_110A_R2				55
#define L_10V_50A_R2				56
#define L_5V_400A_R3				57
#define L_5V_20A_R3					58
#define L_6V_6A_R1					59
#define L_6V_60A_R2					60
#define L_5V_65A_R3_1				61
#define L_6V_60A_R2_P				62
#define L_5V_150A_R3_AD2			63
#define L_5V_200A_R3_P_AD2			64
#define L_5V_200A_R3_P				65
#define L_5V_6A_R3					66
#define L_20V_300A_R2_1				67
#define L_5V_600A_10A				68
#define L_20V_50A_R2_1				69
#define L_5V_1A_R2					70
#define L_10V_500A_R2				71
#define L_30V_20A_R1_AD2			72
#define L_30V_5A_R1_AD2				73
#define L_5V_60A_R2_1				74
#define L_5V_10A_R3					75
#define L_60V_100A_R1_AD2			76
#define L_15V_100A_R3_AD2			77
#define L_5V_150A_R2_P				78	//TOSHIBA(ORIX) SPEC
#define L_5V_200A_1CH_JIG			79	//
#define L_MULTI						80	//4CH MULTI BD SPEC
#define L_5V_1A_R3					81
#define L_5V_10A_R3_NEW				82
#define L_5V_30A_R3_HYUNDAI			83	
#define L_40V_300A_R2				84
#define L_30V_40A_R2				85
#define L_5V_50A_R2_P				86
#define L_30V_40A_R2_P_AD2			87
#define L_MAIN_REV11				88
#define L_8CH_MAIN_AD2_P			89
#define L_5V_20A_R3_NEW				90	//32CH MAIN BD USE SPEC (20A)
#define L_5V_500A_R3_1				91
#define L_30V_40A_R2_OT_20			92
#define L_5V_500mA_2uA_R4			93	//210510 lyhw for 2uA
#define L_20V_6A_R3					94	//210813 ljsw for LGES 20V6A haksun
#define C_5V_CYCLER_CAN				95
#define S_5V_200A					100
#define S_5V_200A_75A_15A_AD2		101
#define	DC_DIGITAL_SPEC				200	//180611
#define	DC_5V_150A_PARA				201	//1000 -> 201
#define	DC_5V_CYCLER_NEW			202

//max current
#define	L_1A						1000000
#define	L_5A						5000000
#define	L_10A						10000000
#define	L_50A						50000000
#define	L_100A						100000000
#define	L_300A						300000000
#define	L_500A						500000000
#define	L_1000A						1000000000

//max voltage
#define L_10V						10000000

//current 180602
#define	C_1MA						1000
#define	C_10MA						10000
#define	C_100MA						100000

//capacity type
#define CAPACITY_AMPARE_HOURS		0
#define CAPACITY_CAPACITANCE		1

//range define
#define RANGE0						0 //range off
#define RANGE1						1
#define RANGE2						2
#define RANGE3						3
#define RANGE4						4

#define HIGH_RANGE					RANGE0
#define MIDDLE_RANGE				RANGE1
#define LOW_RANGE					RANGE2
#define BOTTOM_RANGE				RANGE3

//Run Define
#define RUN_OFF						0
#define RUN_ON						1
//#define RUN_CHARGE				1
//#define RUN_DISCHARGE				2

//Scan Period
#define	RT_SCAN_PERIOD_100mS		0
#define	RT_SCAN_PERIOD_50mS			1
#define	RT_SCAN_PERIOD_25mS			2
#define	RT_SCAN_PERIOD_20mS			3
#define	RT_SCAN_PERIOD_100mS_2		4
#define	RT_SCAN_PERIOD_10mS			5
//#define	RT_SCAN_PERIOD_100mS_CAN	6

//#define SCAN_PERIOD_VI_ADC			100	//100ms
//#define SCAN_PERIOD_CH_CONTROL		100	//100ms
//#define SCAN_PERIOD_MODULE_CONTROL	100	//100ms

#define	IO_INPUT			0
#define	IO_OUTPUT			1
#define IO_EXPEND			2

//Interface B'd Address Define
// Input
#define IO_IN_1					0
#define IO_IN_2					1
#define IO_IN_3					2
#define IO_IN_4					3
#define IO_IN_5					4
#define IO_IN_6					5
#define IO_IN_7					6
#define IO_IN_8					7
#define IO_IN_9					8
#define IO_IN_10				9
#define IO_IN_11				10
#define IO_IN_12				11
#define IO_IN_13				12
#define IO_IN_14				13
#define IO_IN_15				14
#define IO_IN_16				15
#define IO_IN_17				16
#define IO_IN_18				17
#define IO_IN_19				18
#define IO_IN_20				19
#define IO_IN_21				20
#define IO_IN_22				21
#define IO_IN_23				22
#define IO_IN_24				23
#define IO_IN_25				24
#define IO_IN_26				25
#define IO_IN_27				26
#define IO_IN_28				27
#define IO_IN_29				28
#define IO_IN_30				29
#define IO_IN_31				30
#define IO_IN_32				31

// Output
#define IO_OUT_1				0
#define IO_OUT_2				1
#define IO_OUT_3				2
#define IO_OUT_4				3
#define IO_OUT_5				4
#define IO_OUT_6				5
#define IO_OUT_7				6
#define IO_OUT_8				7
#define IO_OUT_9				8
#define IO_OUT_10				9
#define IO_OUT_11				10
#define IO_OUT_12				11
#define IO_OUT_13				12
#define IO_OUT_14				13
#define IO_OUT_15				14
#define IO_OUT_16				15
#define IO_OUT_17				16
#define IO_OUT_18				17
#define IO_OUT_19				18
#define IO_OUT_20				19
#define IO_OUT_21				20
#define IO_OUT_22				21
#define IO_OUT_23				22
#define IO_OUT_24				23
#define IO_OUT_25				24
#define IO_OUT_26				25
#define IO_OUT_27				26
#define IO_OUT_28				27
#define IO_OUT_29				28
#define IO_OUT_30				29
#define IO_OUT_31				30
#define IO_OUT_32				31

// Extend
#define EXT_1				0
#define EXT_2				1
#define EXT_3				2
#define EXT_4				3
#define EXT_5				4
#define EXT_6				5
#define EXT_7				6
#define EXT_8				7
#define EXT_9				8
#define EXT_10				9
#define EXT_11				10
#define EXT_12				11
#define EXT_13				1

#define EXT_14				13
#define EXT_15				14
#define EXT_16				15
#define EXT_17				16
#define EXT_18				17
#define EXT_19				18
#define EXT_20				19
#define EXT_21				20
#define EXT_22				21
#define EXT_23				22
#define EXT_24				23
#define EXT_25				24
#define EXT_26				25
#define EXT_27				26
#define EXT_28				27
#define EXT_29				28
#define EXT_30				29
#define EXT_31				30
#define EXT_32				31

//Main Board Address Define
#define BASE_ADDR			0
#define ADDR_STEP			1
#define	ADDR_DIV			2
#define	AD_BYTE				3
#define	AD_RC				4
#define	AD_CS				5
#define	AD_BUSY				6
#define	DA_H_BYTE			7
#define	DAV_L				8
#define	DAI_L				9
#define	AUX_DA_SYNC			10
#define	AUX_DA_H			11
#define	AUX_DA_L			12
#define	MUX_EN				13
#define	MUX_CS1				14
#define MUX_CS2				15
#define HW_RD				16
#define HW_WR				17
#define RUN_CS				18
#define RANGE_V_CS			19
#define RANGE_I_CS			20
#define STATUS_LED			21
#define BD_OT				22
#define PS_REM_CS			23
#define DA_BD_ENABLE		24
#define FAD_CS				25
#define PARALLEL_CS			26
#define CD_CS1				27
#define CD_CS2				28
#define FAD_END_CS			29
#define FAD_CH				30
#define PS_FAIL_CS			31

#endif
