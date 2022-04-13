#ifndef __SYSDEFINE_H__
#define __SYSDEFINE_H__

// system define
#define MAX_SUM						(2147483647L - 147483647L)
									// 7FFFFFFFh - 8CA6BFFh = 2000000000d
#define MAX_SIGNAL					64
#define MAX_TEST_VALUE				16

// log files
#define DEBUG_LOG					0
#define METER1_LOG					1
#define METER2_LOG					2

// sbc type
#define HS_6637						0x01
#define WEB_6580					0x02
#define HS_4020						0x03
#define WAFER_E669					0x04

// phase define
#define PHASE0						0
#define PHASE1						1
#define PHASE2						2
#define PHASE3						3
#define PHASE4						4
#define PHASE5						5
#define PHASE6						6
#define PHASE7						7
#define PHASE8						8
#define PHASE9						9
#define PHASE10						10
#define PHASE11						11
#define PHASE12						12
#define PHASE13						13
#define PHASE14						14
#define PHASE15						15
#define PHASE16						16
#define PHASE17						17
#define PHASE18						18
#define PHASE19						19
#define PHASE20						20
#define PHASE21						21
#define PHASE22						22
#define PHASE23						23
#define PHASE24						24
#define PHASE25						25
#define PHASE26						26
#define PHASE27						27
#define PHASE28						28
#define PHASE29						29
#define PHASE30						30
#define PHASE31						31
#define PHASE32						32
#define PHASE33						33
#define PHASE34						34
#define PHASE35						35
#define PHASE36						36
#define PHASE37						37
#define PHASE38						38
#define PHASE39						39
#define PHASE40						40
#define PHASE50						50
#define PHASE51						51
#define PHASE52						52
#define PHASE53						53
#define PHASE54						54
#define PHASE55						55
#define PHASE56						56
#define PHASE57						57
#define PHASE58						58
#define PHASE59						59
#define PHASE100					100
#define PHASE101					101
#define PHASE102					102
#define PHASE103					103
#define PHASE104					104
#define PHASE105					105
#define PHASE106					106
#define PHASE107					107
#define PHASE108					108
#define PHASE109					109

// cmd variable define
#define STX							0x02
#define ETX							0x03
#define ACK							0x06
#define NACK						0x15
#define SPACE						0x20
#define LF							0x0A
#define CR							0x0D
#define REPLY_NO					0x00
#define REPLY_YES					0x01
#define SEQNUM_NONE					0x00
#define SEQNUM_AUTO					0x01

/* start fail code */
#define FAIL_NONE					0x00
#define FAIL_HOLD					0xFF
#define M_FAIL_AC_POWER				0x01
#define M_FAIL_UPS_BATTERY			0x02
#define M_FAIL_MAIN_EMG				0x03
#define M_FAIL_SUB_EMG				0x04
#define M_FAIL_SMPS					0x05
#define M_FAIL_OT					0x06
#define M_FAIL_FORCE_POWER			0x07
#define M_FAIL_POWER_SWITCH			0x08
#define B_FAIL_TEMP_UPPER			0x20	//main board fail
#define B_FAIL_ADC					0x21
/* end fail code */

/* start calibration meter command */
#define CALI_METER_NONE				0
#define CALI_METER_INIT				1
#define CALI_METER_READ				2
/* end calibration meter command */

/* start message define */
#define MAX_MSG_RING				20
#define MAX_MSG						64
#define MAX_SAVE_MSG				60

// message direction
#define APP_TO_DATASAVE				0
#define APP_TO_METER1				1
#define APP_TO_METER2				2
#define DATASAVE_TO_APP				3
#define DATASAVE_TO_METER1			4
#define DATASAVE_TO_METER2			5
#define METER1_TO_APP				6
#define METER1_TO_DATASAVE			7
#define METER1_TO_METER2			8
#define METER2_TO_APP				9
#define METER2_TO_DATASAVE			10
#define METER2_TO_METER1			11

//AppControl to DataSave 0
#define MSG_APP_DATASAVE_LOG_CLEAR	1

//AppControl to Meter1 1
#define MSG_APP_METER1_INITIALIZE	1
#define MSG_APP_METER1_REQUEST		2
#define MSG_APP_METER1_LOG_START	3
#define MSG_APP_METER1_LOG_STOP		4

//AppControl to Meter2 2
#define MSG_APP_METER2_INITIALIZE	1
#define MSG_APP_METER2_REQUEST		2
#define MSG_APP_METER2_LOG_START	3
#define MSG_APP_METER2_LOG_STOP		4

//Meter1 to DataSave 7
#define MSG_METER1_DATASAVE_MONITOR_DATA	1

//Meter2 to DataSave 10
#define MSG_METER2_DATASAVE_MONITOR_DATA	1
#define MSG_METER2_DATASAVE_MONITOR_DATA2	2

#endif
