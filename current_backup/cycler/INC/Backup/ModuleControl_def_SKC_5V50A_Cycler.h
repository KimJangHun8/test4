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

//semi switch state
#define SEMI_IDLE					0
#define SEMI_PRE					1
#define SEMI_V_P					2
#define SEMI_V_N					3

//module signal
#define M_SIG_MODULE_PROCESS		0
#define M_SIG_LAMP_RUN				1
#define M_SIG_LAMP_STOP				2
#define M_SIG_CALI_RELAY			3
#define M_SIG_REMOTE_SMPS1			4

#define M_SIG_RUN_LED				6
#define M_SIG_FAN_RELAY				7
#define M_SIG_POWER_OFF				8
#define M_SIG_SMPS_FAULT			9
#define M_SIG_OT_FAULT				10
#define M_SIG_SEND_ERROR_CODE		11
#define M_SIG_TEST_SEND_SAVE_DATA	12
#define M_SIG_CALI_RELAY2			13
#define M_SIG_NET_CHECK				14

//board signal
#define B_SIG_RESET_LATCHED			0
#define B_SIG_HW_FAULT_CLEAR		1

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

//interface board
#define I_OUT_ADDR_1				0x604
#define I_IN_ADDR_1					0x602

//dio addr
#define D_OUT_ADDR_1				0x603
#define D_IN_ADDR_1					0x601
#define A_OUT_ADDR_1				0x611
#define A_IN_ADDR_1					0x610

//address define
#define NS_EDLC_TESTER_BD_ADDR		0x620
#define NS_EDLC_TESTER_BD_AD_ADDR_L_BYTE	0x00
#define NS_EDLC_TESTER_BD_AD_ADDR_H_BYTE	0x01
#define NS_EDLC_TESTER_BD_AD_ADDR_RC		0x01
#define NS_EDLC_TESTER_BD_AD_BUSY			0x02
#define NS_EDLC_TESTER_BD_I_IN_ADDR			0x03
#define NS_EDLC_TESTER_BD_I_OUT_ADDR		0x04
#define NS_EDLC_TESTER_BD_RUN_ADDR_1		0x05
#define NS_EDLC_TESTER_BD_RUN_ADDR_2		0x06
#define NS_EDLC_TESTER_BD_RUN_ADDR_3		0x07
#define NS_EDLC_TESTER_BD_RUN_ADDR_4		0x08
#define NS_EDLC_TESTER_BD_MUX_ADDR_1		0x09
#define NS_EDLC_TESTER_BD_MUX_ADDR_2		0x0A
#define NS_EDLC_TESTER_BD_DA_BD_ENABLE		0x0B
#define NS_EDLC_TESTER_BD_DA_ADDR_H_BYTE	0x0C
#define NS_EDLC_TESTER_BD_DA_ADDR_V1		0x0D	//0x0D~0x14
#define NS_EDLC_TESTER_BD_DA_ADDR_I1		0x15	//0x15~0x1C
#define NS_EDLC_TESTER_BD_RANGE_V_ADDR_1	0x1D	//0x1D~0x1F
#define NS_EDLC_TESTER_BD_RANGE_I_ADDR_1	0x20	//0x20~0x22

#define SWITCHING_5V200A_CYCLER_BD_ADDR		0x620
#define SWITCHING_5V200A_CYCLER_BD_I_OUT_ADDR		0x01
#define SWITCHING_5V200A_CYCLER_BD_I_IN_ADDR		0x02
#define SWITCHING_5V200A_CYCLER_BD_OT_IN_ADDR_1		0x03
#define SWITCHING_5V200A_CYCLER_BD_OT_IN_ADDR_2		0x04
#define SWITCHING_5V200A_CYCLER_BD_HW_FAULT_IN_ADDR_1	0x05
#define SWITCHING_5V200A_CYCLER_BD_HW_FAULT_IN_ADDR_2	0x06
#define SWITCHING_5V200A_CYCLER_BD_AD_ADDR_L_BYTE	0x07
#define SWITCHING_5V200A_CYCLER_BD_AD_ADDR_H_BYTE	0x08
#define SWITCHING_5V200A_CYCLER_BD_AD_ADDR_RC		0x08
#define SWITCHING_5V200A_CYCLER_BD_AD_BUSY			0x09
#define SWITCHING_5V200A_CYCLER_BD_RUN_ADDR_1		0x10
#define SWITCHING_5V200A_CYCLER_BD_RUN_ADDR_2		0x11
#define SWITCHING_5V200A_CYCLER_BD_RANGE_V_ADDR_1	0x12	//0x12~0x15
#define SWITCHING_5V200A_CYCLER_BD_RANGE_I_ADDR_1	0x12	//0x12~0x15
#define SWITCHING_5V200A_CYCLER_BD_C_D_ADDR_1		0x16
#define SWITCHING_5V200A_CYCLER_BD_C_D_ADDR_2		0x17
#define SWITCHING_5V200A_CYCLER_BD_MUX_ADDR_1		0x18
#define SWITCHING_5V200A_CYCLER_BD_MUX_ADDR_2		0x19
#define SWITCHING_5V200A_CYCLER_BD_DA_BD_ENABLE		0x20
#define SWITCHING_5V200A_CYCLER_BD_DA_ADDR_H_BYTE	0x21
#define SWITCHING_5V200A_CYCLER_BD_DA_ADDR_V1		0x22	//0x22~0x25
#define SWITCHING_5V200A_CYCLER_BD_DA_ADDR_I1		0x26	//0x26~0x29
#define SWITCHING_5V200A_CYCLER_BD_DA_ADDR_L1		0x2A	//0x2A~0x2D

#define LS_5V10MA_CYCLER_BD_ADDR	0x620
#define LS_5V10MA_CYCLER_BD_AD_ADDR_L_BYTE	0x00
#define LS_5V10MA_CYCLER_BD_AD_ADDR_H_BYTE	0x01
#define LS_5V10MA_CYCLER_BD_AD_ADDR_RC		0x01
#define LS_5V10MA_CYCLER_BD_AD_BUSY			0x02
#define LS_5V10MA_CYCLER_BD_I_IN_ADDR		0x03
#define LS_5V10MA_CYCLER_BD_I_OUT_ADDR		0x04
#define LS_5V10MA_CYCLER_BD_RUN_ADDR_1		0x05	//0x05~0x0C
#define LS_5V10MA_CYCLER_BD_RANGE_V_ADDR_1	0x30
#define LS_5V10MA_CYCLER_BD_RANGE_I_ADDR_1	0x0D	//0x0D~0x14
#define LS_5V10MA_CYCLER_BD_MUX_ADDR_1		0x15
#define LS_5V10MA_CYCLER_BD_MUX_ADDR_2		0x16
#define LS_5V10MA_CYCLER_BD_MUX_ADDR_3		0x17
#define LS_5V10MA_CYCLER_BD_DA_BD_ENABLE	0x18
#define LS_5V10MA_CYCLER_BD_DA_ADDR_H_BYTE	0x19
#define LS_5V10MA_CYCLER_BD_DA_ADDR_V1		0x1A	//0x1A~0x29
#define LS_5V10MA_CYCLER_BD_DA_ADDR_I1		0x1A	//0x1A~0x29

#define SEBANG_5V5A_CYCLER_BD_ADDR		0x620
#define SEBANG_5V5A_CYCLER_BD_AD_ADDR_L_BYTE	0x00
#define SEBANG_5V5A_CYCLER_BD_AD_ADDR_H_BYTE	0x01
#define SEBANG_5V5A_CYCLER_BD_AD_ADDR_RC		0x01
#define SEBANG_5V5A_CYCLER_BD_AD_BUSY			0x02
#define SEBANG_5V5A_CYCLER_BD_I_IN_ADDR			0x03
#define SEBANG_5V5A_CYCLER_BD_I_OUT_ADDR		0x04
#define SEBANG_5V5A_CYCLER_BD_RUN_ADDR_1		0x05
#define SEBANG_5V5A_CYCLER_BD_RUN_ADDR_2		0x06
#define SEBANG_5V5A_CYCLER_BD_RUN_ADDR_3		0x07
#define SEBANG_5V5A_CYCLER_BD_RUN_ADDR_4		0x08
#define SEBANG_5V5A_CYCLER_BD_MUX_ADDR_1		0x09
#define SEBANG_5V5A_CYCLER_BD_MUX_ADDR_2		0x0A
#define SEBANG_5V5A_CYCLER_BD_DA_BD_ENABLE		0x0B
#define SEBANG_5V5A_CYCLER_BD_DA_ADDR_H_BYTE	0x0C
#define SEBANG_5V5A_CYCLER_BD_DA_ADDR_V1		0x0D	//0x0D~0x14
#define SEBANG_5V5A_CYCLER_BD_DA_ADDR_I1		0x15	//0x15~0x1C
#define SEBANG_5V5A_CYCLER_BD_RANGE_V_ADDR_1	0x1D	//0x1D~0x1F
#define SEBANG_5V5A_CYCLER_BD_RANGE_I_ADDR_1	0x20	//0x20~0x22

#define SEBANG_5V30A_CYCLER_BD_ADDR		0x620
#define SEBANG_5V30A_CYCLER_BD_AD_ADDR_L_BYTE	0x00
#define SEBANG_5V30A_CYCLER_BD_AD_ADDR_H_BYTE	0x01
#define SEBANG_5V30A_CYCLER_BD_AD_ADDR_RC		0x01
#define SEBANG_5V30A_CYCLER_BD_AD_BUSY			0x02
#define SEBANG_5V30A_CYCLER_BD_I_IN_ADDR		0x03
#define SEBANG_5V30A_CYCLER_BD_I_OUT_ADDR		0x04
#define SEBANG_5V30A_CYCLER_BD_RUN_ADDR_1		0x05
#define SEBANG_5V30A_CYCLER_BD_RUN_ADDR_2		0x06
#define SEBANG_5V30A_CYCLER_BD_MUX_ADDR_1		0x07
#define SEBANG_5V30A_CYCLER_BD_MUX_ADDR_2		0x08
#define SEBANG_5V30A_CYCLER_BD_DA_ADDR_H_BYTE	0x09
#define SEBANG_5V30A_CYCLER_BD_DA_ADDR_V1		0x0A	//0x0A~0x13
#define SEBANG_5V30A_CYCLER_BD_DA_ADDR_I1		0x14	//0x14~0x1D
#define SEBANG_5V30A_CYCLER_BD_RANGE_V_ADDR_1	0x1E	//0x1E~0x1F
#define SEBANG_5V30A_CYCLER_BD_RANGE_I_ADDR_1	0x20	//0x20~0x21

#define HYUNDAI_2V100A_CYCLER_BD_ADDR		0x620
#define HYUNDAI_2V100A_CYCLER_BD_I_OUT_ADDR			0x01
#define HYUNDAI_2V100A_CYCLER_BD_I_IN_ADDR			0x02
#define HYUNDAI_2V100A_CYCLER_BD_OT_IN_ADDR_1		0x03
#define HYUNDAI_2V100A_CYCLER_BD_OT_IN_ADDR_2		0x04
#define HYUNDAI_2V100A_CYCLER_BD_HW_FAULT_IN_ADDR_1	0x05
#define HYUNDAI_2V100A_CYCLER_BD_HW_FAULT_IN_ADDR_2	0x06
#define HYUNDAI_2V100A_CYCLER_BD_AD_ADDR_L_BYTE		0x07
#define HYUNDAI_2V100A_CYCLER_BD_AD_ADDR_H_BYTE		0x08
#define HYUNDAI_2V100A_CYCLER_BD_AD_ADDR_RC			0x08
#define HYUNDAI_2V100A_CYCLER_BD_AD_BUSY			0x09
#define HYUNDAI_2V100A_CYCLER_BD_RUN_ADDR_1			0x10
#define HYUNDAI_2V100A_CYCLER_BD_RUN_ADDR_2			0x11
#define HYUNDAI_2V100A_CYCLER_BD_RANGE_V_ADDR_1		0x12	//0x12~0x15
#define HYUNDAI_2V100A_CYCLER_BD_RANGE_I_ADDR_1		0x12	//0x12~0x15
#define HYUNDAI_2V100A_CYCLER_BD_C_D_ADDR_1			0x16
#define HYUNDAI_2V100A_CYCLER_BD_C_D_ADDR_2			0x17
#define HYUNDAI_2V100A_CYCLER_BD_MUX_ADDR_1			0x18
#define HYUNDAI_2V100A_CYCLER_BD_MUX_ADDR_2			0x19
#define HYUNDAI_2V100A_CYCLER_BD_DA_BD_ENABLE		0x20
#define HYUNDAI_2V100A_CYCLER_BD_DA_ADDR_H_BYTE		0x21
#define HYUNDAI_2V100A_CYCLER_BD_DA_ADDR_V1			0x22	//0x22~0x25
#define HYUNDAI_2V100A_CYCLER_BD_DA_ADDR_I1			0x26	//0x26~0x29
#define HYUNDAI_2V100A_CYCLER_BD_DA_ADDR_L1			0x2A	//0x2A~0x2D

#define LS_5V5A_CYCLER_BD_ADDR		0x620
#define LS_5V5A_CYCLER_BD_AD_ADDR_L_BYTE	0x00
#define LS_5V5A_CYCLER_BD_AD_ADDR_H_BYTE	0x01
#define LS_5V5A_CYCLER_BD_AD_ADDR_RC		0x01
#define LS_5V5A_CYCLER_BD_AD_BUSY			0x02
#define LS_5V5A_CYCLER_BD_I_IN_ADDR			0x03
#define LS_5V5A_CYCLER_BD_I_OUT_ADDR		0x04
#define LS_5V5A_CYCLER_BD_RUN_ADDR_1		0
#define LS_5V5A_CYCLER_BD_RUN_ADDR_2		0
#define LS_5V5A_CYCLER_BD_RUN_ADDR_3		0
#define LS_5V5A_CYCLER_BD_RUN_ADDR_4		0
#define LS_5V5A_CYCLER_BD_MUX_ADDR_1		0x0D
#define LS_5V5A_CYCLER_BD_MUX_ADDR_2		0x0E
#define LS_5V5A_CYCLER_BD_DA_BD_ENABLE		0x0F
#define LS_5V5A_CYCLER_BD_DA_ADDR_H_BYTE	0x10
#define LS_5V5A_CYCLER_BD_DA_ADDR_V1		0x11	//0x11~0x18
#define LS_5V5A_CYCLER_BD_DA_ADDR_I1		0x19	//0x19~0x20
#define LS_5V5A_CYCLER_BD_RANGE_V_ADDR_1	0
#define LS_5V5A_CYCLER_BD_RANGE_I_ADDR_1	0x05	//0x05~0x0C

#define SK_5V50A_CYCLER_BD_ADDR		0x620
#define SK_5V50A_CYCLER_BD_AD_ADDR_L_BYTE	0x00
#define SK_5V50A_CYCLER_BD_AD_ADDR_H_BYTE	0x01
#define SK_5V50A_CYCLER_BD_AD_ADDR_RC		0x01
#define SK_5V50A_CYCLER_BD_AD_BUSY			0x02
#define SK_5V50A_CYCLER_BD_I_IN_ADDR		0x03
#define SK_5V50A_CYCLER_BD_I_OUT_ADDR		0x04
#define SK_5V50A_CYCLER_BD_RUN_ADDR_1		0
#define SK_5V50A_CYCLER_BD_RUN_ADDR_2		0
#define SK_5V50A_CYCLER_BD_RUN_ADDR_3		0
#define SK_5V50A_CYCLER_BD_RUN_ADDR_4		0
#define SK_5V50A_CYCLER_BD_MUX_ADDR_1		0x0E
#define SK_5V50A_CYCLER_BD_MUX_ADDR_2		0x0F
#define SK_5V50A_CYCLER_BD_DA_BD_ENABLE		0x10
#define SK_5V50A_CYCLER_BD_DA_ADDR_H_BYTE	0x11
#define SK_5V50A_CYCLER_BD_DA_ADDR_V1		0x12	//0x11~0x19
#define SK_5V50A_CYCLER_BD_DA_ADDR_I1		0x1A	//0x1A~0x21
#define SK_5V50A_CYCLER_BD_RANGE_V_ADDR_1	0
#define SK_5V50A_CYCLER_BD_RANGE_I_ADDR_1	0x05	//0x05~0x0C
#define SK_5V50A_CYCLER_BD_PS_REMOTE		0x0D

//#define BD_ADDR						NS_EDLC_TESTER_BD_ADDR
//#define BD_ADDR						SWITCHING_5V200A_CYCLER_BD_ADDR
//#define BD_ADDR						LS_5V10MA_CYCLER_BD_ADDR
//#define BD_ADDR						SEBANG_5V5A_CYCLER_BD_ADDR
//#define BD_ADDR						SEBANG_5V30A_CYCLER_BD_ADDR
//#define BD_ADDR						HYUNDAI_2V100A_CYCLER_BD_ADDR
//#define BD_ADDR						LS_5V5A_CYCLER_BD_ADDR
#define BD_ADDR						SK_5V50A_CYCLER_BD_ADDR

//separate adc
#define SEP_BD_ADDR_STEP			0x40
#define SEP_BD_AD_ADDR_RC			0x00
#define SEP_BD_AD_ADDR_L_BYTE		0x00
#define SEP_BD_AD_ADDR_H_BYTE		0x01
#define SEP_BD_AD_BUSY				0x02
#define SEP_BD_I_IN_ADDR			0x03
#define SEP_BD_I_OUT_ADDR			0x04
#define SEP_BD_RUN_ADDR_1			0x05
#define SEP_BD_RUN_ADDR_2			0x06
#define SEP_BD_RUN_ADDR_3			0x07
#define SEP_BD_RUN_ADDR_4			0x08
#define SEP_BD_MUX_ADDR_1			0x09
#define SEP_BD_MUX_ADDR_2			0x0A
#define SEP_BD_DA_BD_ENABLE			0x0B
#define SEP_BD_DA_ADDR_H_BYTE		0x0C
#define SEP_BD_DA_ADDR_V1			0x0D	//0x0D~0x14
#define SEP_BD_DA_ADDR_I1			0x15	//0x15~0x1C
#define SEP_BD_RANGE_V_ADDR_1		0x1D	//0x1D~0x1F
#define SEP_BD_RANGE_I_ADDR_1		0x20	//0x20~0x22
#define SEP_BD_C_D_ADDR_1			0x23
#define SEP_BD_OT_IN_ADDR_1			0x24
#define SEP_BD_HW_FAULT_IN_ADDR_1	0x25

//common adc
#define COM_BD_AD_ADDR_L_BYTE		SK_5V50A_CYCLER_BD_AD_ADDR_L_BYTE
#define COM_BD_AD_ADDR_H_BYTE		SK_5V50A_CYCLER_BD_AD_ADDR_H_BYTE
#define COM_BD_AD_ADDR_RC			SK_5V50A_CYCLER_BD_AD_ADDR_RC
#define COM_BD_AD_BUSY				SK_5V50A_CYCLER_BD_AD_BUSY
#define COM_BD_I_IN_ADDR			SK_5V50A_CYCLER_BD_I_IN_ADDR
#define COM_BD_I_OUT_ADDR			SK_5V50A_CYCLER_BD_I_OUT_ADDR
#define COM_BD_RUN_ADDR_1			SK_5V50A_CYCLER_BD_RUN_ADDR_1
#define COM_BD_MUX_ADDR_1			SK_5V50A_CYCLER_BD_MUX_ADDR_1
#define COM_BD_MUX_ADDR_2			SK_5V50A_CYCLER_BD_MUX_ADDR_2
#define COM_BD_MUX_ADDR_3			0
#define COM_BD_DA_BD_ENABLE			SK_5V50A_CYCLER_BD_DA_BD_ENABLE
#define COM_BD_DA_ADDR_H_BYTE		SK_5V50A_CYCLER_BD_DA_ADDR_H_BYTE
#define COM_BD_DA_ADDR_V1			SK_5V50A_CYCLER_BD_DA_ADDR_V1
#define COM_BD_DA_ADDR_I1			SK_5V50A_CYCLER_BD_DA_ADDR_I1
#define COM_BD_DA_ADDR_L1			0
#define COM_BD_RANGE_V_ADDR_1		SK_5V50A_CYCLER_BD_RANGE_V_ADDR_1
#define COM_BD_RANGE_I_ADDR_1		SK_5V50A_CYCLER_BD_RANGE_I_ADDR_1
#define COM_BD_C_D_ADDR_1			0
#define COM_BD_OT_IN_ADDR_1			0
#define COM_BD_HW_FAULT_IN_ADDR_1	0
#define COM_BD_PS_REMOTE			SK_5V50A_CYCLER_BD_PS_REMOTE

//watchdog timer address
#define WDT_ADDR_READ				COM_BD_I_IN_ADDR
#define WDT_ADDR_WRITE				COM_BD_I_OUT_ADDR

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
#define RTTASK_1500MS				(RTTASK_CYCLE_PERIOD * 15)
#define RTTASK_1000MS				(RTTASK_CYCLE_PERIOD * 10)

//sbc type
#define HS_6637						1
#define WEB_6580					2
#define HS_4020						3
#define WAFER_E669					4

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
#define S_5V_200A					100

//capacity type
#define CAPACITY_AMPARE_HOURS		0
#define CAPACITY_CAPACITANCE		1

//range define
#define RANGE0						0 //range off
#define RANGE1						1
#define RANGE2						2
#define RANGE3						3
#define RANGE4						4

//Scan Period
#define SCAN_PERIOD_VI_ADC			100	//100ms
#define SCAN_PERIOD_CH_CONTROL		100	//100ms
#define SCAN_PERIOD_MODULE_CONTROL	100	//100ms

#endif
