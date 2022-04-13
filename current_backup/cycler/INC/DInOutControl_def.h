#ifndef __DINOUTCONTROL_DEF_H__
#define __DINOUTCONTROL_DEF_H__

#define MAX_DIGITAL_IN_BYTE			32
#define MAX_DIGITAL_OUT_BYTE		32
#define MAX_DIGITAL_INPUT			(MAX_DIGITAL_IN_BYTE * 8)
#define MAX_DIGITAL_OUTPUT			(MAX_DIGITAL_OUT_BYTE * 8)

//dio signal
#define DIO_SIG_POWER_SWITCH		0
#define DIO_SIG_MAIN_EMG			1
#define DIO_SIG_SUB_EMG				2
#define DIO_SIG_POWER_FAIL			3
#define DIO_SIG_SMPS_FAIL1			4
#define DIO_SIG_SMPS_FAIL2			5
#define DIO_SIG_REMOTE_SMPS1		6
#define DIO_SIG_REMOTE_SMPS2		7
#define DIO_SIG_CALI_CHARGE_RELAY			8
#define DIO_SIG_CALI_DISCHARGE_RELAY			9
#define DIO_SIG_BUZZER_END			10
#define DIO_SIG_START_SW			11

//20170622 sch Modify ChamberType Not Use
#define DIO_SIG_CHAMBER_ERROR1			12
#define DIO_SIG_CHAMBER_ERROR2			13
#define DIO_SIG_CHAMBER_ERROR3			14
#define DIO_SIG_CHAMBER_ERROR4			15
#define DIO_SIG_CHAMBER_ERROR5			16
#define DIO_SIG_CHAMBER_ERROR6			17
#define DIO_SIG_CHAMBER_ERROR7			18
#define DIO_SIG_CHAMBER_ERROR8			19
#define DIO_SIG_CHAMBER_ERROR9			20
#define DIO_SIG_CHAMBER_ERROR10			21
#define DIO_SIG_CHAMBER_ERROR11			22
#define DIO_SIG_CHAMBER_ERROR12			23
#define DIO_SIG_CHAMBER_ERROR13			24
#define DIO_SIG_CHAMBER_ERROR14			25
#define DIO_SIG_CHAMBER_ERROR15			26
#define DIO_SIG_CHAMBER_ERROR16			27

#define DIO_SIG_OVP					30
#define DIO_SIG_CCC					31
#define DIO_SIG_OTP					32

//160701 SCH
#define DIO_SIG_DOOR_OPEN_FAIL1		33
#define	DIO_SIG_DOOR_OPEN_FAIL2		34
#define	DIO_SIG_SMOKE_FAIL			35

//151023
#define DIO_SIG_DC_FAN_FAIL1		36
#define DIO_SIG_DC_FAN_FAIL2		37

#ifdef _JIG_TYPE_1
#define DIO_SIG_JIG_START			38
#define DIO_SIG_JIG_STOP			39
#define DIO_SIG_JIG_EMG				41
#define DIO_SIG_JIG_RIGHT_LOWER		42
#define DIO_SIG_JIG_LEFT_LOWER		43
#define DIO_SIG_JIG_LIMIT			44
#define DIO_SIG_JIG_RIGHT_UPPER		45
#define DIO_SIG_JIG_LEFT_UPPER		46

#endif

#define DIO_SIG_DOOR_OPEN_FAIL3		47
#define DIO_SIG_NONE_FAIL			50		//191001
#define	DIO_SIG_PCU_INV_FAIL1		51
#define	DIO_SIG_PCU_INV_FAIL2		52
#define	DIO_SIG_PCU_INV_FAIL3		53
#define	DIO_SIG_PCU_INV_FAIL4		54
#define	DIO_SIG_PCU_INV_FAIL5		55
#define	DIO_SIG_PCU_INV_FAIL6		56
#define	DIO_SIG_PCU_INV_FAIL7		57
#define	DIO_SIG_PCU_INV_FAIL8		58
#define	DIO_SIG_PCU_INV_FAIL9		59
#define	DIO_SIG_PCU_INV_FAIL10		60

//dout signal
#define O_SIG_LAMP_RUN				0
#define O_SIG_LAMP_STOP				1
#define O_SIG_CALI_RELAY			2
#define O_SIG_CALI_RELAY2			3

#define O_SIG_REMOTE_SMPS1			0
#define O_SIG_REMOTE_SMPS2			1
#define O_SIG_RUN_LED				2
#define O_SIG_FAN_RELAY				3
#define O_SIG_POWER_OFF				4
#define O_SIG_FAULT_LAMP1			9
#define O_SIG_FAULT_LAMP2			10
#define O_SIG_FAULT_LAMP3			11
#define O_SIG_FAULT_LAMP4			12

#define LAMP_GREEN					1
#define LAMP_YELLOW					2
#define LAMP_RED					3
#define LAMP_BUZZER_ON				4
#define LAMP_BUZZER_OFF				5

#define FAN_ON						1
#define FAN_OFF						2
//internal input
#define I_IN_POWER_SWITCH			0
#define I_IN_MAIN_EMG				1
#define I_IN_AC_POWER_FAIL			2
#define I_IN_UPS_BATTERY_FAIL		3
#define I_IN_NO_POWER_OFF			4
#define I_IN_SMPS_FAIL1				5
#define I_IN_SMPS_FAIL2				6
#define I_IN_SMPS_FAIL3				7

#ifdef _JIG_TYPE_0
#define I_IN_BUZZER_STOP			8
#endif
#ifdef _JIG_TYPE_1
#define I_IN_JIG_START				8
#define I_IN_JIG_STOP				9
#define I_IN_JIG_EMG				10
#define I_IN_JIG_RIGHT_LOWER		11
#define I_IN_JIG_LEFT_LOWER			12
#define I_IN_JIG_LIMIT				13
#define I_IN_JIG_RIGHT_UPPER		14
#define I_IN_JIG_LEFT_UPPER			15
#endif

//20170622 sch Modify ChamberType Not Use
#define I_IN_CHAMBER_ERROR1				8
#define I_IN_CHAMBER_ERROR2				9
#define I_IN_CHAMBER_ERROR3				10
#define I_IN_CHAMBER_ERROR4				11
#define I_IN_CHAMBER_ERROR5				12
#define I_IN_CHAMBER_ERROR6				13
#define I_IN_CHAMBER_ERROR7				14
#define I_IN_CHAMBER_ERROR8				15
#define I_IN_CHAMBER_ERROR9				16
#define I_IN_CHAMBER_ERROR10			17
#define I_IN_CHAMBER_ERROR11			18
#define I_IN_CHAMBER_ERROR12			19
#define I_IN_CHAMBER_ERROR13			20
#define I_IN_CHAMBER_ERROR14			21
#define I_IN_CHAMBER_ERROR15			22
#define I_IN_CHAMBER_ERROR16			23
//151023 for dc_Fan_fail / 160701 SCH Fix
#define I_IN_LOWSPEC_DIO_FAIL1			24	//DC_FAN_FAIL1 & DOOR_OPEN1
#define I_IN_LOWSPEC_DIO_FAIL2			25	//DC_FAN_FAIL2 & DOOR_OPEN2
#define I_IN_LOWSPEC_DIO_FAIL3			26	//SMOKE_SENSOR
//Low Spec Chamber Error / F_CHAMBER_TYPE == 2
#define I_IN_CHAMBER_ERROR17			26
#define I_IN_CHAMBER_ERROR18			27
#define I_IN_CHAMBER_ERROR19			28
#define I_IN_CHAMBER_ERROR20			29
#define I_IN_CHAMBER_ERROR21			30
#define I_IN_CHAMBER_ERROR22			31
#define I_IN_DIO_DOOR_FAIL1				32	//180822 add
#define I_IN_DIO_DOOR_FAIL2				33	//180822 add
#define I_IN_DIO_DOOR_FAIL3				34	//180822 add
//190103 add 
#define I_IN_DOOR_OPEN_FAIL1			36	
#define I_IN_DOOR_OPEN_FAIL2			37	
#define I_IN_SMOKE_SENSOR_FAIL1			38	
#define I_IN_SMOKE_SENSOR_FAIL2			39	

//external input

//internal output
#ifdef _CYCLER
#define I_OUT_REMOTE_SMPS1			0
#define I_OUT_REMOTE_SMPS2			1
#define I_OUT_RUN_LED				2
#define I_OUT_FAN_RELAY				3
#define I_OUT_SPARE4				4
#define I_OUT_SPARE5				5
#define I_OUT_POWER_OFF				6
#define I_OUT_SPARE7				7 //Power Off

#if CYCLER_TYPE == LINEAR_CYC || CYCLER_TYPE == CAN_CYC  
	#define I_OUT_CALI_CHARGE_RELAY		8
	#define I_OUT_CALI_DISCHARGE_RELAY	9
#else
//180611 lyhw for digital Inverter I/O
	#define I_OUT_FAN1					8
	#define I_OUT_FAN2					9
	#define I_OUT_FAN3					10
	#define I_OUT_FAN4					11
	#define I_OUT_FAN5					12
	#define I_OUT_FAN6					13
	#define I_OUT_FAN7					14
	#define I_OUT_FAN8					15
#endif

#ifdef _JIG_TYPE_0
#define I_OUT_MULTI_RUN				10
#define I_OUT_MULTI_STOP			11
#define I_OUT_MULTI_PAUSE			12
#define I_OUT_STATE_RUN				16
#define I_OUT_STATE_STOP			17
#define I_OUT_STATE_PAUSE			18
#define I_OUT_STATE_BUZZER			19
#define I_OUT_END_BUZZER			20
#define I_OUT_MAIN_MC_OFF			21	//220214_hun
#endif

#ifdef _JIG_TYPE_1
#define I_OUT_JIG_PASS_LAMP			16
#define I_OUT_JIG_FAIL_LAMP			17
#define I_OUT_JIG_START_LAMP		18
#define I_OUT_JIG_STOP_LAMP			19
#define I_OUT_JIG_BUZZER			20
#define I_OUT_JIG_SOL				21
#endif

#ifdef _JIG_TYPE_3
#define I_OUT_STATE1_PAUSE			16
#define I_OUT_STATE1_STOP			17
#define I_OUT_STATE1_RUN			18
#define I_OUT_STATE1_BUZZER			19
#define I_OUT_CALI_CHARGE_RELAY		20
#define I_OUT_CALI_DISCHARGE_RELAY	21
#define I_OUT_CALI_VOLTAGE1_RELAY	22
#define I_OUT_CALI_VOLTAGE2_RELAY	23
#endif

#if CYCLER_TYPE == CAN_CYC
#define I_OUT_CAN1_0				24
#define I_OUT_CAN1_1				25
#define I_OUT_CAN1_2				26
#define I_OUT_CAN1_3				27
#define I_OUT_CAN1_4				28
#define I_OUT_CAN1_5				29
#define I_OUT_CAN1_6				30
#define I_OUT_CAN1_7				31
#define I_OUT_CAN2_0				32
#define I_OUT_CAN2_1				33
#define I_OUT_CAN2_2				34
#define I_OUT_CAN2_3				35
#define I_OUT_CAN2_4				36
#define I_OUT_CAN2_5				37
#define I_OUT_CAN2_6				38
#define I_OUT_CAN2_7				39
#endif

#endif
#ifdef _PACK_CYCLER
#define I_OUT_REMOTE_SMPS1			0
#define I_OUT_REMOTE_SMPS2			1
#define I_OUT_SPARE2				2
#define I_OUT_FAN_RELAY				3
#define I_OUT_RUN_LED				4
#define I_OUT_SPARE5				5
#define I_OUT_POWER_OFF				6
#define I_OUT_SPARE7				7 //Power Off
#define I_OUT_CALI_CHARGE_RELAY		8
#define I_OUT_CALI_DISCHARGE_RELAY	9
#endif

//external output
#define E_OUT_LAMP_RUN				0
#define E_OUT_LAMP_STOP				1
#define E_OUT_CALI_RELAY			2
#define E_OUT_BUZZER				3
#define E_OUT_CALI_RELAY2			4

/*
#define I_OUT_STATE2_STOP			20
#define I_OUT_STATE2_PAUSE			21
#define I_OUT_STATE2_RUN			22
#define I_OUT_STATE2_BUZZER			23
*/
//#define I_OUT_FIRE_1				24	
//#define I_OUT_FIRE_2				28

/*
#define I_OUT_INV1_RESET			16
#define I_OUT_INV1_RUN				17
#define I_OUT_INV2_RESET			20
#define I_OUT_INV2_RUN				21
#define I_OUT_INV3_RESET			24
#define I_OUT_INV3_RUN				25
#define I_OUT_INV4_RESET			28
#define I_OUT_INV4_RUN				29
#define I_OUT_INV5_RESET			32
#define I_OUT_INV5_RUN				33
#define I_OUT_INV6_RESET			36
#define I_OUT_INV6_RUN				37
#define I_OUT_INV7_RESET			48
#define I_OUT_INV7_RUN				49
#define I_OUT_INV8_RESET			52
#define I_OUT_INV8_RUN				53
*/
#define I_IN_INV1_FLT1				9
#define I_IN_INV1_FLT2				10
#define I_IN_INV1_FLT3				11
#define I_IN_INV2_FLT1				13
#define I_IN_INV2_FLT2				14
#define I_IN_INV2_FLT3				15
#define I_IN_INV3_FLT1				17
#define I_IN_INV3_FLT2				18
#define I_IN_INV3_FLT3				19
#define I_IN_INV4_FLT1				21
#define I_IN_INV4_FLT2				22
#define I_IN_INV4_FLT3				23
#define I_IN_INV5_FLT1				25
#define I_IN_INV5_FLT2				26
#define I_IN_INV5_FLT3				27
#define I_IN_INV6_FLT1				29
#define I_IN_INV6_FLT2				30
#define I_IN_INV6_FLT3				31
#define I_IN_INV7_FLT1				33
#define I_IN_INV7_FLT2				34
#define I_IN_INV7_FLT3				35
#define I_IN_INV8_FLT1				37
#define I_IN_INV8_FLT2				38
#define I_IN_INV8_FLT3				39

#define I_IN_SMOKE_SENS				3
#define I_IN_FIRE_SENS1				16
#define I_IN_FIRE_SENS2				20

#define DIO_SIG_SMOKE				12
#define DIO_SIG_FIRE				13

#endif
