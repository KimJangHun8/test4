#ifndef __SYSDEFINE_H__
#define __SYSDEFINE_H__

//system define
#define MAX_SUM						(2147483647L - 147483647L)
									// 7FFFFFFFh - 8CA6BFFh = 2000000000d
#define MAX_CH_PER_MODULE			64

//#if CAN_TYPE == 1
#if CYCLER_TYPE == CAN_CYC
	#define MAX_BD_PER_MODULE			16
	#define	MAX_CH_PER_BD				4
#else
	#define MAX_BD_PER_MODULE			2
	#define	MAX_CH_PER_BD				64
#endif

//System Type Define
#define FORMATION					0
#define IROCV						1
#define AGING						2
#define GRADER						3
#define SELECTOR					4
#define OCV							5
#define CYCLER_LINEAR				6
#define CYCLER_PCU					7
#define CYCLER_CAN					8

#if CYCLER_TYPE == DIGITAL_CYC
	#define MAX_SIGNAL					128
#else
	#define MAX_SIGNAL					96
#endif

#define MAX_FUNCTION				64
#define MAX_TEMP_CH					200	//140324 oys add//170619 sch modify
#define MAX_TEMP_POINT				5	//200102 add
#define MAX_TEST_VALUE				20
#define MAX_AUX_DATA				256
#define MAX_AUX_VOLT_DATA			128
#define MAX_AUX_TEMP_DATA			128
#define MAX_AUX_TYPE				2
#define MAX_AUX_NAME_SIZE			128
#define MAX_CAN_DATA				512
#define MAX_CAN_TYPE				2
#define MAX_CAN_NAME_SIZE			128
#define MAX_NOMAL_CODE				255

#define MAX_AD_INPUT				16
#define MAX_TH_STEP					50
#define MAX_TYPE					2
#define MAX_RANGE					4
#define MAX_CALI_POINT				15
#define MAX_REF_CALI_POINT			3
#define MAX_TOTAL_RUNTIME			1987200000
#define ONE_DAY_RUNTIME				8640000
#define MAX_AUX_FUNCTION			10
#define MAX_CH_AUX_DATA				5		//190807 pthw
#define MAX_CH_PRESSURE_DATA		4		//210316 lyhw

#if	VENDER == 2		//SDI
#define MAX_CH_THICKNESS_DATA		4		//211025 hun
#else 
#define MAX_CH_THICKNESS_DATA		2		//210316 lyhw
#endif

#define MAX_SOC_TRACKING_DATA		30      // 20 -> 30 rewrite 210720

#define MAX_SHUNT_SERIAL_LENGTH		12

#ifdef _PACK_CYCLER
#define MAX_AD_COUNT				3
#endif

#ifdef _CYCLER
	#if CYCLER_TYPE == DIGITAL_CYC
		#define MAX_AD_COUNT				7
	#else
		#define MAX_AD_COUNT				8
	#endif
#endif

#define	MAX_ERROR_CNT_TEMP			3	//190430
#define MAX_FILTER_AD_COUNT			MAX_AD_COUNT
#define MAX_TMP_AD_COUNT			MAX_AD_COUNT
#define MAX_INV_NUM					8
#define MAX_ERROR_CNT_P				5	//180611
#define MAX_INV_ERR_CNT				2	//200905
#define MAX_ERROR_CNT_SEQ			3	//180810
#define MAX_ERROR_CNT_SEQ_PATTERN	5	//191007
//#define MAX_SEQ_NO_RETRY			3	//180820
#define MAX_SEQ_NO_RETRY			5	//180828
#define MAX_SOURCE_SENS_COUNT		600

#if CYCLER_TYPE == DIGITAL_CYC
	#define MAX_ERROR_CNT				7
#else 
	#define MAX_ERROR_CNT				3
#endif

#define MAX_SLAVE_CH				3
#define PROCESS_NAME_SIZE			16
#define MAX_PROCESS_NUMBER			16

#define MAX_HW_FAULT_NUM			32
#define MAX_FUNCTION_NUM			32
#define MAX_CHAMBERMOTION_NUM		16
#define MAX_C_RATE_STEP_NO			10
#define MAX_FAULT_NUM				10
#define MAX_COMP_GOTO				5
#define MAX_COOLING_COUNT			4	//2220302 jws add

#define	SAFETY						2000
#define	STEP_SAFETY					1000

//log files
#define DEBUG_LOG					0
#define MAIN_LOG					1
#define METER_LOG					2
#define METER2_LOG					3
#define EXT_LOG						4

//phase define
#define P0							0
#define P1							1
#define P2							2
#define P3							3
#define P4							4
#define P5							5
#define P6							6
#define P7							7
#define P8							8
#define P9							9
#define P10							10
#define P11							11
#define P12							12
#define P13							13
#define P14							14
#define P15							15
#define P16							16
#define P17							17
#define P18							18
#define P19							19
#define P20							20
#define P21							21
#define P22							22
#define P23							23
#define P24							24
#define P25							25
#define P26							26
#define P27							27
#define P28							28
#define P29							29
#define P30							30
#define P31							31
#define P32							32
#define P33							33
#define P34							34
#define P35							35
#define P36							36
#define P37							37
#define P38							38
#define P39							39
#define P40							40
#define P41							41
#define P42							42
#define P43							43
#define P44							44
#define P45							45
#define P46							46
#define P47							47
#define P48							48
#define P49							49
#define P50							50
#define P51							51
#define P52							52
#define P53							53
#define P54							54
#define P55							55
#define P56							56
#define P57							57
#define P58							58
#define P59							59
#define P60							60
#define P61							61
#define P62							62
#define P63							63
#define P64							64
#define P65							65
#define P66							66
#define P67							67
#define P68							68
#define P69							69
#define P70							70
#define P71							71
#define P72							72
#define P73							73
#define P74							74
#define P75							75
#define P76							76
#define P77							77
#define P78							78
#define P79							79
#define P80							80
#define P81							81
#define P82							82
#define P83							83
#define P84							84
#define P85							85
#define P86							86
#define P87							87
#define P88							88
#define P89							89
#define P90							90
#define P91							91
#define P92							92
#define P93							93
#define P94							94
#define P95							95
#define P96							96
#define P97							97
#define P98							98
#define P99							99
#define P100						100
#define P101						101
#define P102						102
#define P103						103
#define P104						104
#define P105						105
#define P106						106
#define P107						107
#define P108						108
#define P109						109
#define P110						110
#define P111						111
#define P112						112
#define P113						113
#define P114						114
#define P115						115
#define P116						116
#define P117						117
#define P118						118
#define P119						119
#define P120						120
#define P121						121
#define P122						122
#define P123						123
#define P124						124
#define P125						125
#define P126						126
#define P127						127
#define P128						128
#define P129						129
#define P130						130
#define P131						131
#define P132						132
#define P133						133
#define P134						134
#define P135						135
#define P136						136
#define P137						137
#define P138						138
#define P139						139
#define P140						140
#define P141						141
#define P142						142
#define P143						143
#define P144						144
#define P145						145
#define P146						146
#define P147						147
#define P148						148
#define P149						149
#define P150						150
#define P151						151
#define P152						152
#define P153						153
#define P154						154
#define P155						155
#define P156						156
#define P157						157
#define P158						158
#define P159						159
#define P160						160
#define P161						161
#define P162						162
#define P163						163
#define P164						164
#define P165						165
#define P166						166
#define P167						167
#define P168						168
#define P169						169
#define P170						170
#define P171						171
#define P172						172
#define P173						173
#define P174						174
#define P175						175
#define P176						176
#define P177						177
#define P178						178
#define P179						179
#define P180						180
#define P181						181
#define P182						182
#define P183						183
#define P184						184
#define P185						185
#define P186						186
#define P187						187
#define P188						188
#define P189						189
#define P190						190
#define P191						191
#define P192						192
#define P193						193
#define P194						194
#define P195						195
#define P196						196
#define P197						197
#define P198						198
#define P199						199
#define P200						200
#define P201						201
#define P202						202
#define P203						203
#define P204						204
#define P205						205
#define P206						206
#define P207						207
#define P208						208
#define P209						209
#define P210						210
#define P211						211
#define P212						212
#define P213						213
#define P214						214
#define P215						215

#define ON							1
#define OFF							0

#define JIG_INIT					0
#define JIG_STANDBY					1
#define JIG_PAUSE					2
#define JIG_SIGNAL1					3
#define JIG_SIGNAL2					4
#define JIG_SIGNAL3					5
#define JIG_SIGNAL4					6

//cmd variable define
#define ACK							0x06
#define NACK						0x15
#define REPLY_NO					0x00
#define REPLY_YES					0x01
#define SEQNUM_NONE					0x00
#define SEQNUM_AUTO					0x01

//measure range
#define	MICRO						0
#define	NANO						1

//ad type
#define COMM_TYPE					0

//da type
#define DAC_712						0
#define DAC_7741					1

//MainBdType
#define CPLD_TYPE					0
#define FPGA_TYPE					1

//grade item
#define GRADE_NONE					0x00
#define GRADE_V						0x01
#define GRADE_CAPACITY				0x02
#define GRADE_Z						0x03
#define GRADE_CURRENT				0x04
#define GRADE_TIME					0x05
#define GRADE_FARAD					0x06
#define GRADE_TEMP					0x07
#define GRADE_POWER					0x08
#define GRADE_WATTHOUR				0x09

//grade cond
#define GRADE_AND					0x26 //&
#define GRADE_OR					0x2B //+
//#define GRADE_AND					'&'
//#define GRADE_OR					'+'

//grade proc
#define GRADE_STOP					1
#define GRADE_PAUSE					2

//save flag
#define SAVE_FLAG_MONITORING_DATA	0
#define SAVE_FLAG_SAVING_END		1
#define SAVE_FLAG_SAVING_TIME		2
#define SAVE_FLAG_SAVING_DELTA_V	3
#define SAVE_FLAG_SAVING_DELTA_I	4
#define SAVE_FLAG_SAVING_DELTA_T	5 //temperature
#define SAVE_FLAG_SAVING_DELTA_P	6
#define SAVE_FLAG_SAVING_ETC		7
#define SAVE_FLAG_SAVING_PAUSE		8	//190901 lyhw

#define V_CH_FAIL					10	//10=1.0% -> 5000mV:50mV
#define I_CH_FAIL					20	//20=2.0% -> 5000mA:100mA

//channel code
#define C_CODE_IDLE					0
#define C_END_CODE_START			64 //cell end code
#define C_END_TIME					64
#define C_END_VOLTAGE				65
#define C_END_CURRENT				66
#define C_END_CAPACITY				67
#define C_END_OCV					68
#define C_END_STEP					69
#define C_END_STOP_CMD				70
#define C_END_PAUSE_CMD				71
#define C_END_CHECK					72
#define C_END_NEXTSTEP_CMD			73
#define C_END_Z						74
#define C_END_ADV_CYCLE				75
#define C_END_LOOP					76
#define C_END_DELTA_V				77
#define C_END_SOC					78
#define C_END_TEMP					79

#define C_STOP_CODE_START			80 //cell stop code
#define C_STOP_TIME					80
#define C_STOP_VOLTAGE				81
#define C_STOP_CURRENT				82
#define C_STOP_CAPACITY				83
#define C_STOP_POWER				84
#define C_STOP_WATTHOUR				85
#define C_TEMP_WAIT_TIME			86
#define C_STEP_SYNC_WAIT_TIME		88
#define C_END_SHORT					87 //cell end code continue
#define	C_ACIR_WAIT_TIME			88
#define C_END_ACIR					89
#define C_END_POWER					90
#define C_END_WATTHOUR				91
#define C_END_INTEGRAL_CAPACITY		92
#define C_END_INTEGRAL_WATTHOUR		93
#define C_END_CYCLE_TIME			94
#define C_END_CVTIME				95 //CVRUNTIME end

#define C_FAULT_CHECK_CODE_START	96 //check step fault code
#define	C_FAULT_CHECK_UPPER_OCV		96
#define	C_FAULT_CHECK_LOWER_OCV		97
#define	C_FAULT_CHECK_UPPER_VOLTAGE	98
#define	C_FAULT_CHECK_LOWER_VOLTAGE	99
#define	C_FAULT_CHECK_UPPER_CURRENT	100
#define	C_FAULT_CHECK_LOWER_CURRENT	101
#define C_FAULT_CHECK_CONTACT_BAD	102
#define C_FAULT_CHECK_CONTACT_BAD2	103
#define C_FAULT_CHECK_CONTACT_BAD3	104
#define C_FAULT_CHECK_BAD_CELL		105
#define C_FAULT_CHECK_BAD_CELL2		106
#define C_FAULT_CHECK_BAD_CELL3		107
#define C_FAULT_CHECK_SHORT			108
#define C_FAULT_ERROR_NO			109
#define C_FAULT_ERROR_YES			110
#define C_FAULT_VOLTAGE_DROP		111	//20180627 sch add for drop voltage

#define C_FAULT_SOFT_CODE_START		128 //soft fault code
#define	C_FAULT_UPPER_VOLTAGE		128
#define	C_FAULT_LOWER_VOLTAGE		129
//#define C_FAULT_UPPER_DELTA_VOLTAGE	130
//#define C_FAULT_LOWER_DELTA_VOLTAGE	131
#define C_FAULT_LIMIT_CURRENT		130	//20171008 sch modify
#define C_FAULT_END_LIMIT_CURRENT	131	//20171008 sch modify
#define C_FAULT_UPPER_COMP_VOLTAGE1	132
#define C_FAULT_LOWER_COMP_VOLTAGE1	133
#define C_FAULT_UPPER_OCV			134
#define C_FAULT_LOWER_OCV			135
#define	C_FAULT_UPPER_CURRENT		136
#define C_FAULT_LOWER_CURRENT		137
//#define C_FAULT_UPPER_DELTA_CURRENT	138
//#define C_FAULT_LOWER_DELTA_CURRENT	139
#define C_FAULT_LIMIT_VOLTAGE		138	//20171008 sch modify
#define C_FAULT_END_LIMIT_VOLTAGE	139	//20171008 sch modify
#define C_FAULT_UPPER_COMP_CURRENT	140
#define C_FAULT_LOWER_COMP_CURRENT	141
#define C_FAULT_UPPER_CAPACITY		142
#define	C_FAULT_LOWER_CAPACITY		143
#define C_FAULT_CHARGE_DELTA_CAPACITY		144
#define	C_FAULT_DISCHARGE_DELTA_CAPACITY	145
#define C_FAULT_UPPER_COMP_CAPACITY	146
#define	C_FAULT_LOWER_COMP_CAPACITY	147
#define C_FAULT_UPPER_IMPEDANCE		148
#define C_FAULT_LOWER_IMPEDANCE		149
#define C_FAULT_UPPERTIME			150
#define C_FAULT_LOWERTIME			151
#define C_FAULT_STOP_CMD			152
#define C_FAULT_PAUSE_CMD			153
#define C_FAULT_NEXTSTEP_CMD		154
#define C_FAULT_UPPER_COMP_VOLTAGE2	155
#define C_FAULT_LOWER_COMP_VOLTAGE2	156
#define C_FAULT_UPPER_COMP_VOLTAGE3	157
#define C_FAULT_LOWER_COMP_VOLTAGE3	158
#define C_FAULT_UPPER_CC_TIME		159
#define C_FAULT_WORK_ERROR			160
#define C_FAULT_UPPER_COMP_CURRENT2	161
#define C_FAULT_LOWER_COMP_CURRENT2	162
#define C_FAULT_UPPER_COMP_CURRENT3	163
#define C_FAULT_LOWER_COMP_CURRENT3	164
#define C_FAULT_UPPER_CH_TEMP		165
#define C_FAULT_LOWER_CH_TEMP		166
#define C_FAULT_USER_PATTERN_READ	167 // add <-20071118
#define C_FAULT_LIMIT_ERROR			168
#define C_FAULT_CHAMBER_ERROR		169
#define C_FAULT_INVERTER			170
#define C_FAULT_TERMINAL_QUIT		171
#define C_FAULT_COMPARE_LINE_PLUS 	172
#define C_FAULT_COMPARE_LINE_MINUS 	173
#define C_FAULT_COMPARE_LINE_FLT_CH	174
#define	C_FAULT_COMPARE_UPPER_VOLTAGE	175
#define	C_FAULT_COMPARE_UPPER_CURRENT	176
#define	C_FAULT_COMPARE_OVER_TEMP	177
#define C_FAULT_COMPARE_METER_HIGH	178
#define C_FAULT_COMPARE_METER_LOW	179
#define C_FAULT_JIG_ERROR			180
#define C_FAULT_TEMP_CONNECT_ERROR	181
#define C_FAULT_NETWORK_CONNECT_ERROR	182
#define C_FAULT_SHORT_TESTER_ERROR	183 // oys add <-20140407
#define C_FAULT_SHORT_DISCONNECTED	184 // oys add <-20140407
#define C_FAULT_CYCLER_SINGLE		185 // oys add <-20140407
#define C_FAULT_SHORT_TESTER_SINGLE	186 // oys add <-20140407
#define C_FAULT_INTER_LOCK_MODE		187 // oys add <-20140407
#define	C_FAULT_GUI_SAFETY			188	// lyh add <-20190225

#define	C_CYCLE_P_STEP_WAIT			190	// lyh add <-20190306
#define	C_FAULT_OVEN_USE			191	// lyh add <-20190318
#define C_FAULT_RUN_TIME			192	//hun_200219
#define C_FAULT_SOH					193	//hun_200219
#define C_FAULT_CHANGE_DELTA_V		194	//hun_200430

#define C_PAUSE_UPPER_TEMP_CH		195	// lyh add <-20190311
#define	C_PAUSE_LOWER_TEMP_CH		196	// lyh add <-20190311
#define C_FAULT_CHANGE_V			197 // lyh add <-20190607
#define C_FAULT_CHK_VOLTAGE			198 // lyh add <-20190607
#define C_FAULT_CHK_CURRENT			199 // lyh add <-20190607
#define C_END_SOE					200	// pth add <-20190214 
#define	C_FAULT_CAPACITY_RETAIN		201	// kjc add <-20210414
#define C_FAULT_CAPACITY_EFFICIENCY 202	// kjc_add <-20210425
#define C_FAULT_READ_TRACKING_FILE	203	// 210609
#define C_FAULT_SELECT_CH_NETWORK_ERROR 204	// 210913

#define C_FAULT_HARD_CODE_START		205 //hard_fault
//20190618 oys add ====================
#define C_FAULT_MAIN_CAN_COMM_ERROR 205
#define C_FAULT_INV_CAN_COMM_ERROR	206
#define C_FAULT_IO_CAN_COMM_ERROR	207
//=====================================
#define C_FAULT_CH_V_FAIL			208
#define C_FAULT_CH_I_FAIL			209
#define C_FAULT_OT					210
#define C_FAULT_SMPS				211
#define C_FAULT_FORCE_POWER			212
#define C_FAULT_AC_POWER			213
#define C_FAULT_UPS_BATTERY			214
#define C_FAULT_MAIN_EMG			215
#define C_FAULT_SUB_EMG				216
#define C_FAULT_NETWORK_COMM		217
#define C_FAULT_PANEL_HIGH			218
#define C_FAULT_PANEL_LOW			219
//aux Fault, End code
#define C_END_AUX_TEMP				220
#define C_END_AUX_TEMP_UPPER		221
#define C_END_AUX_TEMP_LOWER		222
#define C_END_AUX_VOLTAGE			223
#define C_END_AUX_VOLTAGE_UPPER		224
#define C_END_AUX_VOLTAGE_LOWER		225
#define C_FAULT_AUX_TEMP_UPPER		226
#define C_FAULT_AUX_TEMP_LOWER		227
#define C_FAULT_AUX_VOLTAGE_UPPER	228
#define C_FAULT_AUX_VOLTAGE_LOWER	229

//current userMap Temp end code
#define C_END_USERMAP_OVER_TEMP		230			

//151023 add for dc_fan_fail
#define	C_FAULT_DC_FAN				231

//151214 oys add : SEC Cycle Capacity Efficiency End
#define C_STOP_CAPACITY_SOC			232
#define C_FAULT_CAPACITY_SOC		233

//160510 oys add : LGC, SDI Cycle Capacity Efficiency End
#define C_END_CYCLE_CAPACITY_SOC	234
#define C_END_CYCLE_CAPACITY		235
#define C_END_CYCLE_WH_SOC			236
#define C_END_CYCLE_DCIR_SOC		237
#define C_FAULT_WH_SOC				238
#define C_FAULT_DCIR_SOC			239

//180104 sch modify for 170518 lyh cycle V end
#define C_END_CYCLE_VOLTAGE			240
#define C_STOP_CYCLE_VOLTAGE		241

//160701 SCH
#define	C_FAULT_DOOR_OPEN			242
#define	C_FAULT_SMOKE				243

//170215 SCH add for DeltaV/I
#define	C_FAULT_DELTA_V				244
#define	C_FAULT_DELTA_I				245
#define	C_STOP_DELTA_V				246
#define	C_STOP_DELTA_I				247

//170501 oys add AH, WH, DCIR Efficiency End
#define C_END_AH_SOC				248
#define C_END_WH_SOC				249
#define C_END_DCIR_SOC				250

//170718 oys add
#define C_FAULT_GRADE				251
#define C_STOP_GRADE				252
//171123 oys add
#define C_FAULT_CH_GROUP			253

//181108
#define C_FAULT_SHUTDOWN			254
//200414 ljs
#define	C_END_T_V_UPPER				255
#define	C_END_T_V_LOWER				256

//180611 lyh PCU Error
#define P_CD_FAULT_HVU_INPUT_POWER	260
#define P_CD_FAULT_OVP				261
#define P_CD_FAULT_OCP				262
#define P_CD_FAULT_OTP				263
#define P_CD_FAULT_PV_OVP			264
#define P_CD_FAULT_HW_OVP			265
#define P_CD_FAULT_HW_OCP			266
#define P_CD_FAULT_BD_OT			267
#define P_CD_FAULT_B_POWER			268
#define P_CD_FAULT_BALANCE			269
#define P_CD_FAULT_PARALLEL_RELAY	270
#define P_CD_FAULT_EXT_RELAY		271
#define P_CD_FAULT_SCI_ERR			273
#define P_CD_FAULT_UNKOWN_ERR		274
#define P_CD_FAULT_MODE_ERR			275
#define P_CD_FAULT_SEQ_NO			277
//#define C_FAULT_SMOKE				280
//#define C_FAULT_FIRE				281

//180625 lyhw Digital INV Error
#define P_INV_FAULT_LAG_SHORT		280
#define P_INV_FAULT_OVER_CURRENT	281
#define P_INV_FAULT_OVER_VOLTAGE	282
#define P_INV_FAULT_PRECHARGE_FAIL	283
#define P_INV_FAULT_OVER_CURRENT2	284
#define P_INV_FAULT_CAN_ERR			285
#define P_INV_FAULT_OVER_LOAD		286
#define P_INV_FAULT_OVER_HEAT		287
#define P_INV_FAULT_LOW_VOLTAGE		289
#define	P_INV_FAULT_AC_LOW_VOLTAGE	290
#define P_INV_FAULT_RESET1			291
#define	P_INV_FAULT_RESET2			292
#define P_INV_FAULT_AC_INPUT_FAIL	293
#define P_INV_FAULT_AC_OVER_VOLT	294
#define P_INV_FAULT_HDC_ERROR		295
#define P_INV_STANDBY				296		//180904 add 
#define P_INV_FAULT_ETC				297		//200904 notUseError

//210204 lyhw for can error code
#define C_FAULT_CH_CAN_HARD_ERR		302
#define C_FAULT_CH_CAN_LOU_ERR		303
#define C_FAULT_CH_CAN_OTP_ERR		304
#define C_FAULT_CH_CAN_VOL_OUT_ERR	306
#define C_FAULT_CH_CAN_BUS_ERR		307
//210204 lyhw
#define	C_FAULT_CV_VOLTAGE			310
#define	C_FAULT_CV_CURRENT			311
//210402 
#define	C_END_CYCLE_STD_CAPACITY_STEP_CAPACITY	312
#define	C_END_CYCLE_STD_CAPACITY_STEP_CURRENT	313
#define	C_FAULT_C_RATE_CALC_ERROR	314

//210806_kjc
#define C_FAULT_CELL_DIAGNOSIS_PAUSE	315
#define C_FAULT_CELL_DIAGNOSIS_STOP		316
#define C_FAULT_CELL_DIAGNOSIS_ALARM	317
//211014 
#define C_FAULT_EQUATION_CALC_ERROR		318

//210304 lyhw panda inverter error code
#define P_INV_DC_FAULT_OVER_TIME		320
#define P_INV_DC_FAULT_BUS_OVER_V		321
#define P_INV_DC_FAULT_BUS_LOW_V		322
#define P_INV_DC_FAULT_OVER_V			323
#define P_INV_DC_FAULT_LOW_V			324
#define P_INV_DC_FAULT_OVER_CURRENT		325
#define P_INV_DC_FAULT_OVER_LOAD		326
#define P_INV_DC_FAULT_LOCK				327
#define P_INV_DC_FAULT_SOFT_SHORT		328
#define P_INV_DC_FAULT_DIP_SWITCH		329
#define P_INV_DC_FAULT_PFC_ERR			330
//210304 panda AC error code
#define P_INV_AC_FAULT_INPUT_V			340
#define P_INV_AC_FAULT_INPUT_FRQ		341
#define P_INV_AC_FAULT_INPUT_CURRENT	342
#define P_INV_AC_FAULT_PFC_BUS_V		343
#define P_INV_AC_FAULT_OVER_TIME		344
#define P_INV_AC_FAULT_OVER_LOAD		345
#define P_INV_AC_FAULT_OVER_HEAT		346
#define P_INV_AC_FAULT_LOCK				347

//210412 LJS for MASTER RECIPE
#define C_FAULT_M_RECIPE_CONNECT_ERROR		350
#define C_FAULT_CABLE_LINE_CHECK			351
#define C_MASTER_RECIPE_WAIT_TIME			352
#define	C_FAULT_MASTER_RECIPE_VOLTAGE_ERR   353
#define C_FAULT_MASTER_RECIPE_CURRENT_ERR   354
#define C_FAULT_MASTER_RECIPE_TIME_ERR 		355
#define C_FAULT_MASTER_RECIPE_TEMP_ERR 		356
#define C_FAULT_MASTER_RECIPE_GROUP_ERR		357
//211025 lyhw for Ulsan JigPress Timeout fault
#define C_FAULT_JIGPRESS_TIMEOUT_ERR		360
//210923 lyhw
#define C_FAULT_UPPER_GAS_TVOC				361
#define C_FAULT_LOWER_GAS_TVOC				362
#define C_FAULT_UPPER_GAS_ECO2				363
#define C_FAULT_LOWER_GAS_ECO2				364
#define C_END_GAS_TVOC						365
#define C_END_GAS_ECO2						366

//211125 hun
#define	C_FAULT_CHARGE_CC_VOLTAGE_HUMP		380
#define	C_FAULT_DISCHARGE_CC_VOLTAGE_HUMP	381
#define C_FAULT_CHARGE_CV_CURRENT_HUMP		382
#define C_FAULT_DISCHARGE_CV_CURRENT_HUMP	383
//211012 kjc
#define C_FAULT_CV_CURRENT_HUMP			386
//210428 hun
#define C_FAULT_GAS_VOLTAGE_MIN			400
#define C_FAULT_GAS_VOLTAGE_MAX			401
#define	C_FAULT_REST_VOLTAGE_ERROR		402
#define C_FAULT_AMBIENT_TEMP_MAX		404

//220322_hun
#define C_FAULT_SOFT_VENTING			405
#define C_FAULT_HARD_VENTING			406

//210901 LJS
#define	C_FAULT_GUI_AMBIENT_TEMP_ERROR	403
#define	C_FAULT_GUI_UPPER_VOLTAGE		412
#define	C_FAULT_OVP_ERROR				413
#define	C_FAULT_GUI_UPPER_TEMP			416	
#define	C_FAULT_OTP_ERROR				417
#define	C_FAULT_GUI_LOWER_VOLTAGE		418
#define	C_FAULT_OVP_GROUP_ERROR			419
#define	C_FAULT_OTP_GROUP_ERROR			420
#define	C_FAULT_GUI_CHAMBER_DI			421
//210126 LJS
#define	C_FAULT_GUI_CHAMBER_AMBIENT_ERR	422
//220214 LJS
#define C_FAULT_GROUP_VOLTAGE_ERROR		423
#define C_FAULT_GROUP_TIME_ERROR		424

#define	C_FAULT_STEP_UPPER_VOLTAGE		1128
#define	C_FAULT_COMM_UPPER_VOLTAGE		2128
#define	C_FAULT_STEP_LOWER_VOLTAGE		1129
#define	C_FAULT_COMM_LOWER_VOLTAGE		2129
#define	C_FAULT_STEP_UPPER_OCV			1134
#define	C_FAULT_COMM_UPPER_OCV			2134
#define	C_FAULT_STEP_LOWER_OCV			1135
#define	C_FAULT_COMM_LOWER_OCV			2135
#define	C_FAULT_STEP_UPPER_CURRENT		1136
#define	C_FAULT_COMM_UPPER_CURRENT		2136
#define C_FAULT_STEP_LOWER_CURRENT		1137
#define C_FAULT_COMM_LOWER_CURRENT		2137
#define C_FAULT_STEP_UPPER_CAPACITY		1142
#define C_FAULT_COMM_UPPER_CAPACITY		2142
#define	C_FAULT_STEP_LOWER_CAPACITY		1143
#define	C_FAULT_COMM_LOWER_CAPACITY		2143
#define C_FAULT_STEP_UPPER_CH_TEMP		1165
#define C_FAULT_COMM_UPPER_CH_TEMP		2165
#define C_FAULT_STEP_LOWER_CH_TEMP		1166
#define C_FAULT_COMM_LOWER_CH_TEMP		2166

//ch error count
#define C_CNT_CH_I_FAIL				0
#define C_CNT_CH_V_FAIL				1
#define C_CNT_OVP					2
#define C_CNT_OCP					3
#define C_CNT_UPPER_VOLTAGE			4
#define C_CNT_LOWER_VOLTAGE			5
#define C_CNT_UPPER_CURRENT			6
#define C_CNT_LOWER_CURRENT			7
#define C_CNT_UPPER_CAPACITY		8
#define C_CNT_LOWER_CAPACITY		9
#define C_CNT_UPPER_OCV				10
#define C_CNT_LOWER_OCV				11
#define C_CNT_UPPER_TEMP			12
#define C_CNT_LOWER_TEMP			13
#define C_CNT_CONTACT_BAD			14
#define C_CNT_CONTACT_BAD2			15
#define C_CNT_CONTACT_BAD3			16
#define C_CNT_BAD_CELL				17
#define C_CNT_BAD_CELL2				18
#define C_CNT_BAD_CELL3				19
#define C_CNT_SHORT					20
#define C_CNT_UPPER_VOLTAGE_STEP	21
#define C_CNT_LOWER_VOLTAGE_STEP	22
#define C_CNT_UPPER_CURRENT_STEP	23
#define C_CNT_LOWER_CURRENT_STEP	24
#define C_CNT_UPPER_CAPACITY_STEP	25
#define C_CNT_LOWER_CAPACITY_STEP	26
#define C_CNT_UPPER_TEMP_STEP		27
#define C_CNT_LOWER_TEMP_STEP		28
#define C_CNT_UPPER_OCV_STEP		29
#define C_CNT_LOWER_OCV_STEP		30
//170215 SCH add for DeltaV/I
#define C_CNT_DELTA_V_STEP			31
#define C_CNT_DELTA_I_STEP			32
//20180314 sch add for LimitV
#define C_CNT_LIMIT_V				33
//	#define C_CNT_PCU_FAULT				31
#define C_CNT_PCU_FAULT				34		//180611 lyh add for digital
#define C_CNT_DROP_V				35
#define C_CNT_PCU_INV_FLT1			36
#define C_CNT_PCU_INV_FLT2			37
#define C_CNT_PCU_INV_FLT3			38
#define C_CNT_PCU_INV_FLT4			39
#define C_CNT_PCU_INV_FLT5			40
#define C_CNT_PCU_INV_FLT6			41
#define C_CNT_PCU_INV_FLT7			42
#define C_CNT_PCU_INV_FLT8			43
#define C_CNT_PCU_SEQ_FAULT			44		//180725 lyh add for digital
#define C_CNT_UPPER_TEMP_PAUSE_STEP	45		//190311 lyh add
#define C_CNT_LOWER_TEMP_PAUSE_STEP	46		//190311 lyh add
#define C_CNT_UPPER_AUX_TEMP		47
#define C_CNT_LOWER_AUX_TEMP		48
#define C_CNT_UPPER_AUX_TEMP_STEP	49
#define C_CNT_LOWER_AUX_TEMP_STEP	50
#define C_CNT_PCU_INPUT_FAULT		51		//191024 lyhw
#define C_CNT_FAULT_DELTA_V			52		//200504 hun add
//210204 lyhw
#define C_CNT_CV_FAULT_V			53		
#define C_CNT_CV_FAULT_I			54		
//210204 lyhw
#define C_CNT_INV_AC_1				55
#define C_CNT_INV_AC_2				56
#define C_CNT_INV_AC_3				57
#define C_CNT_INV_AC_4				58
#define C_CNT_INV_AC_5				59
#define C_CNT_INV_AC_6				60
#define C_CNT_INV_AC_7				61
#define C_CNT_INV_AC_8				62
#define C_CNT_UPPER_GAS_TVOC_STEP	65		//210923 lyhw
#define C_CNT_LOWER_GAS_TVOC_STEP	66
#define C_CNT_UPPER_GAS_ECO2_STEP	67
#define C_CNT_LOWER_GAS_ECO2_STEP	68

#define	C_CNT_INV_FAULT_CAN			70		//220112 lyhw

//fail code
#define FAIL_NONE					0
#define FAIL_HOLD					255
#define M_FAIL_AC_POWER_SHORT		1
#define M_FAIL_AC_POWER_LONG		2
#define M_FAIL_UPS_BATTERY			3
#define M_FAIL_MAIN_EMG				4
#define M_FAIL_SUB_EMG				5
#define M_FAIL_SMPS					6
#define M_FAIL_OT					7
#define M_FAIL_FORCE_POWER			8
#define M_FAIL_CPU_WATCHDOG			9
#define M_FAIL_CALI_METER_COMM_ERROR	10
#define M_FAIL_CALIBRATOR_COMM_ERROR	11
#define M_FAIL_AD_PART				12 
#define M_FAIL_TERMINAL_QUIT		13
#define M_FAIL_TERMINAL_HALT		14

//20170622 sch Modify ChamberType Not Use
#define M_FAIL_CHAMBER_ERROR1		15
#define M_FAIL_CHAMBER_ERROR2		16
#define M_FAIL_CHAMBER_ERROR3		17
#define M_FAIL_CHAMBER_ERROR4		18
#define M_FAIL_CHAMBER_ERROR5		19
#define M_FAIL_CHAMBER_ERROR6		20
#define M_FAIL_CHAMBER_ERROR7		21
#define M_FAIL_CHAMBER_ERROR8		22
#define M_FAIL_CHAMBER_ERROR9		23
#define M_FAIL_CHAMBER_ERROR10		24
#define M_FAIL_CHAMBER_ERROR11		25
#define M_FAIL_CHAMBER_ERROR12		26
#define M_FAIL_CHAMBER_ERROR13		27
#define M_FAIL_CHAMBER_ERROR14		28
#define M_FAIL_CHAMBER_ERROR15		29
#define M_FAIL_CHAMBER_ERROR16		30

#define M_FAIL_DC_CHAMBER_FAULT1	31	//191001 lyhw
#define M_FAIL_DC_CHAMBER_FAULT2	32	//191001 lyhw
#define M_FAIL_DC_CHAMBER_FAULT3	33	//191001 lyhw
#define M_FAIL_DC_CHAMBER_FAULT4	34	//191001 lyhw
#define M_FAIL_DC_CHAMBER_FAULT5	35	//191001 lyhw
#define M_FAIL_DC_CHAMBER_FAULT6	36	//191001 lyhw

#ifdef _JIG_TYPE_1
#define M_FAIL_JIG_EMG				15
#define M_FAIL_JIG_RIGHT_LOWER		16
#define M_FAIL_JIG_LEFT_LOWER		17
#define M_FAIL_JIG_LIMIT_ERROR		18
#define M_FAIL_JIG_RIGHT_UPPER		19
#define M_FAIL_JIG_LEFT_UPPER		20
#endif

#define M_FAIL_INV					45
//111215 unusual end 
#define M_FAIL_UNUSUAL_END			47
//111215 meter error
#define M_FAIL_PANEL_METER_LOW_ERROR	48
#define M_FAIL_PANEL_METER_HIGH_ERROR	49
//
//20151115 khk ass
#define M_FAIL_OVP						52
#define M_FAIL_OTP						53
#define M_FAIL_CCC						54

#define M_FAIL_DC_FAN				55
#define M_FAIL_DOOR_OPEN1			56	//160701 SCH
#define M_FAIL_DOOR_OPEN2			57	//160701 SCH
#define M_FAIL_SMOKE				58	//160701 SCH
#define M_FAIL_DOOR_OPEN3			59	//160701 SCH

// Digital Inverter
/*
#define M_FAIL_INV_DC				52
#define M_FAIL_INV_AC				53
#define M_FAIL_INV_TEMP				54
*/
#define M_FAIL_INV_DC				60	//180611 lyh for digital
#define M_FAIL_INV_AC				61	//180611 lyh for digital
#define M_FAIL_INV_TEMP				62	//180611 lyh for digital
#define M_FAIL_DOOR_OPEN4			63	//220124 jsh

#define M_FAIL_INV_GROUP1			70	//180709 lyh for digital
#define M_FAIL_INV_GROUP2			71	//180709 lyh for digital
#define M_FAIL_INV_GROUP3			72	//180709 lyh for digital
#define M_FAIL_INV_GROUP4			73	//180709 lyh for digital
#define M_FAIL_INV_GROUP5			74	//180709 lyh for digital
#define M_FAIL_INV_GROUP6			75	//180709 lyh for digital
#define M_FAIL_INV_GROUP7			76	//180709 lyh for digital
#define M_FAIL_INV_GROUP8			77	//180709 lyh for digital
#define M_FAIL_INV_GROUP9			78	//180709 lyh for digital
#define M_FAIL_INV_GROUP10			79	//180709 lyh for digital
#define M_FAIL_INV_GROUP11			80	//180709 lyh for digital
#define M_FAIL_INV_GROUP12			81	//180709 lyh for digital
#define M_FAIL_INV_GROUP13			82	//180709 lyh for digital
#define M_FAIL_INV_GROUP14			83	//180709 lyh for digital
#define M_FAIL_INV_GROUP15			84	//180709 lyh for digital
#define M_FAIL_INV_GROUP16			85	//180709 lyh for digital

#define M_FAIL_SMOKE2				90 	//190123 add

//20190605 KHK----------------
#define M_FAIL_MAIN_CAN_COMM_ERROR	91
#define M_FAIL_INV_CAN_COMM_ERROR	92
#define M_FAIL_IO_CAN_COMM_ERROR	93
//-----------------------------
#define M_FAIL_MAIN_CAN_ERR_TOTAL	94	//220117 lyhw
#define	M_FAIL_PCU_HVU_INPUT_ERROR	95	//191118 lyw

#define M_FAIL_CHAMBER_SYNC_TIME_OVER 96	//kjc_210413


//STATUS LED
#define LED_OFF			0x00
#define LED_CHARGE 		0x01
#define	LED_DISCHARGE 	0x02
#define LED_REST		0x04
#define LED_FAULT		0x08

//Data define of ID index
#define PS_STATE				0x00
#define	PS_VOLTAGE				0x01
#define PS_CURRENT				0x02
#define PS_CAPACITY				0x03
#define PS_IMPEDANCE			0x04
#define PS_CODE					0x05
#define PS_STEP_TIME			0x06
#define PS_TOT_TIME				0x07
#define PS_GRADE_CODE			0x08
#define PS_STEP_NO				0x09
#define PS_WATT					0x0A
#define	PS_WATT_HOUR			0x0B
#define PS_TEMPERATURE			0x0C
#define PS_PRESSURE				0x0D
#define PS_STEP_TYPE			0x0E
#define PS_CUR_CYCLE			0x0F
#define PS_TOT_CYCLE			0x10
#define PS_TEST_NAME			0x11
#define PS_SCHEDULE_NAME		0x12
#define PS_CHANNEL_NO			0x13
#define PS_MODULE_NO			0x14
#define PS_LOT_NO				0x15
#define PS_DATA_SEQ				0x16
#define PS_AVG_CURRENT			0x17
#define PS_AVG_VOLTAGE          0x18
#define PS_CAPACITY_SUM			0x19
#define PS_CHARGE_CAP			0x1A
#define PS_DISCHARGE_CAP		0x1B
#define PS_METER_DATA			0x1C
#define PS_START_TIME			0x1D
#define PS_END_TIME				0x1E
#define PS_C_RATE				0x1F	//20190802 add
#define PS_SWELLING_DATA		0x20	//210318 lyhw
#define PS_GAS_MEASURE_DATA		0x21	//210923 lyhw

//Save Condition
#define NORMAL_COND				0
#define END_COND				1
#define FAULT_COND				2
#define SAVE_START				3
#define SAVE_DELTA_T			4
#define SAVE_DELTA_V			5
#define SAVE_DELTA_I			6
#define SAVE_DELTA_TEMP			7
#define SAVE_PULSE_T			8
#define END_COND_GOTO_LOOP		9

//Aux Value Control
#define	COMP_NONE						0
#define	COMP_LESS_THAN				 	1 //<
#define	COMP_LESS_THAN_OR_EQUAL_TO		2 //<=
#define	COMP_EQUAL_TO					3 //=
#define	COMP_GREATER_THAN				4 //>
#define	COMP_GREATER_THAN_OR_EQUAL_TO	5 //>=
#define	COMP_NOT_EQUAL_TO				6 //!=

//aux_function_division
#define AUX_V_FUNC_DIV_NONE				0
#define AUX_V_FUNC_DIV_ALL				1 //UPPER_OR_LOWER
#define AUX_V_FUNC_DIV_UPPER			2
#define AUX_V_FUNC_DIV_LOWER			3
#define AUX_TEMP_FUNC_DIV_NONE			0
#define AUX_TEMP_FUNC_DIV_ALL			1
#define AUX_TEMP_FUNC_DIV_UPPER			2
#define AUX_TEMP_FUNC_DIV_LOWER			3
#define AUX_FUNC_DIV_START				2001
#define AUX_FUNC_DIV_END				3000

//HwFault_Config define
#define HW_FAULT_OVP					0
#define HW_FAULT_CCC					1
#define HW_FAULT_OTP					2
#define HW_FAULT_DROP_V_1				3	//Charge Drop voltage
#define HW_FAULT_DROP_V_2				4	//DisChrage Drop Voltage
#define HW_FAULT_CV_VOLTAGE				5	
#define HW_FAULT_CV_CURRENT				6

//SwFault_Config define
//210428 hun
#define CHAMBER_GAS_VOLTAGE_MIN			0
#define CHAMBER_GAS_VOLTAGE_MAX			1
#define REST_CHECK_START_TIME			2
#define REST_START_COMPARE_VOLTAGE  	3
#define REST_COMPARE_VOLTAGE_DELTA_V1 	4
#define REST_COMPARE_VOLTAGE_DELTA_V2 	5
#define REST_FAULT_CHECK_COUNT			6
#define GAS_CHECK_TIME					7
#define AMBIENT_TEMP_MAX				8
#define AMBIENT_TEMP_MAX_TIME			9
#define AMBIENT_TEMP_DIFF				10
#define AMBIENT_TEMP_DIFF_TIME			11
#define SOFT_VENTING_COUNT				12
#define SOFT_VENTING_VALUE				13
#define HARD_VENTING_VALUE				14

//FUNCTION define
#define	F_OVP						0
#define	F_CCC						1
#define	F_OTP						2
#define	F_MAIN_BD_OT				3
#define	F_HW_FAULT_COND				4
#define	F_SW_FAULT_COND				5
#define	F_MINUS_CELL				6
#define	F_I_OFFSET_CALI				7
#define F_SEMI_SWITCH_TYPE			8
#define F_CHAMBER_TEMP_WAIT			9
#define F_SDI_MES_USE				10
#define F_OT_PAUSE					11
#define F_CHAMBER_ERR_PROC			12
#define F_SENS_COUNT_TYPE			13
#define F_V_SENS_REM_NO				14
#define F_I_SENS_REM_NO				15
#define F_DELTA_V_I					16
#define F_CHAMBER_TYPE				17
#define F_PATTERN_FTP				18
#define F_SBC_RECOVERY				19
#define F_PAUSE_DATA_SAVE			20
#define F_CHANGE_VI_CHECK			21
#define F_DCR_TYPE					22
#define F_PATTERN_PROCESS			23
#define F_PATTERN_CH_SAVE			24
#define F_DISCONNECT_DAY			25
#endif
