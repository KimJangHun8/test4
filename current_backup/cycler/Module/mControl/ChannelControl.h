#ifndef __CHANNELCONTROL_H__
#define __CHANNELCONTROL_H__

void	ChannelControl_Ch(int, int);
void	ChannelControl_Ch_Default(int, int);

void	cStep(int, int);
void	cStopCond(int, int);
void	cEndCond(int, int);
int		cEndCond_Ocv(int, int, int, unsigned long);
int		cEndCond_Rest(int, int, int, unsigned long);
int		cEndCond_Charge(int, int, int, unsigned long);
int		cEndCond_Discharge(int, int, int, unsigned long);
int		cEndCond_Z(int, int, int, unsigned long);
int		cEndCond_User_Pattern(int, int, int, unsigned long);
int		cEndCond_User_Map(int, int, int, unsigned long);
int		cEndCond_Short(int, int, int, unsigned long); // add <- 20140407 oys
int		cEndCond_Acir(int, int, int, unsigned long);	//220124 hun
int		cEndCond_Rest_P(int, int, int, unsigned long);
int		cEndCond_Charge_P(int, int, int, unsigned long);
int		cEndCond_Discharge_P(int, int, int, unsigned long);
int		cEndCond_User_Pattern_P(int, int, int, unsigned long);
int		cEndCond_Z_P(int, int, int, unsigned long);
int		cEndCond_Acir_P(int, int, int, unsigned long);	//220124 hun
int		cEndCond_Cycle_Common(int, int , int);
int		cEndCond_FaultCheck(int, int, int, unsigned long);	//add <- 20200309

int		tempEndCheck(int, int, int, unsigned long); //add <--20160129 oys
void	cShortDioSignal(int, int); // add <- 20140407 oys
void 	cShortModeControl(int);	// add <- 20170501 oys

void	cIdle(int, int);
void	cStandby(int, int);
void	cPause(int, int);
void	cStepCharge(int, int);
void	cStepDischarge(int, int);
void	cStepRest(int, int);
void	cStepOcv(int, int);
void	cStepZ(int, int);
void	cStepAcir(int, int);	//220124 hun
void	cStepDefault(int, int);
void	cNextStep(int, int);
void	cStepAdvCycle(int, int);
void	cStepParallelCycle(int, int);
void	cStepLoop(int, int, int);
int		cGoto_Type_Check(int, int); // add <--20141029 oys
void	cStepEnd(int, int);
void	cStepUserPattern(int, int); // add <--20071106
void	cStepBalance(int, int); // add <--20071106
void	cStepShort(int, int); // add <--20140407 oys
void	cStepGoto(int, int); // add <--20080130
void	cStepUserMap(int, int); // add <--20111215 kji
int		AdvStepFlag(int, int);

void	cCali(int, int);
int 	cCalibrationV(int, int, int);
int		cCalibrationCheckV(int, int, int);
int 	cCalibrationI(int, int, int);
int		cCalibrationCheckI(int, int, int);
void	cCalculate_CaliData(int, int);
void	cCalculate_CaliData_1(int, int);
void	cCalculate_CaliData_2(int, int);
void	cCalculate_CaliData_3(int, int);	//180611 add for digital
void	cCalculate_CaliData_CAN(int, int);	//190828 oys add
void	cCali_Ch_Select(int, int, int);
void	cCali_Ch_Select_auto_2(int, int);
void	cCali_Ch_Select_CAN(int, int, int);		//210308 lyhw
double 	return_meter_value(int, int, int);		//20160229
double 	return_calimeter2_value(int, int, int);	//20160229
void 	data_gathering_cali(int, int, int, int, int); 	//20160229
void 	data_gathering_check(int, int, int, int, int); 	//20160229
void 	data_gathering_cali_meter2(int, int, int, int, int); 	//20160229
void 	data_gathering_check_meter2(int, int, int, int, int); 	//20160229

void	cSemiSwitch(int, int);
void	cSemiSwitch_L1(int, int);
void	cSemiSwitch_L2(int, int);
void	cSemiSwitch_L3(int, int);
void	cSemiSwitch_L4(int, int);
void	cSemiSwitch_L5(int, int);
void	cSemiSwitch_L6(int, int);
void	cSemiSwitch_L7(int, int);	//210601 lyhw
void	cSemiSwitch_S1(int, int);
void	cSemiSwitch_S2(int, int);
void	cSemiSwitch_p(int, int, int);
void	cActiveSwitch(void);
void	cActiveSwitch_L1(void);

void	cSemiSwitch_Cali(int, int, unsigned char);
void	cSemiSwitch_Start(int, int, int);
void	cSemiSwitch_Charge(int, int, int);
void	cSemiSwitch_Discharge(int, int, int);
void	cSemiSwitch_Pattern(int, int, int);
void	cSemiSwitch_Rest(int, int, unsigned long, int);
unsigned char cSemi_Rest_Check(int, int);
unsigned long cSemi_Rest_StepNo_Check(int, int);
void	cSemiSwitch_End(int, int, int);

int		cFindDACaliPoint(int, int, long, int, int);
int		cFindDACaliPoint_1(int, int, long, int, int);
int		cFindDACaliPoint_2(int, int, long, int, int);
int		cFindDACaliPoint_p(int, int, long, int, int);
void    cCalCmdV(int, int, long, int, int);
void    cCalCmdV_Default(int, int, long, int, int);//20190605 KHK
void    cCalCmdV_p(int, int, long, int, int);
long    cCalCmdV_CAN(int, int, long, int, int, int);//20190605 KHK
void    cCalCmdI(int, int, long, int, int);
void    cCalCmdI_Default(int, int, long, int, int);//20190605 KHK
void    cCalCmdI_p(int, int, long, int, int);
long    cCalCmdI_CAN(int, int, long, int, int, int);//20190605 KHK
double	SelectShunt(int, long, double);
double	ConvertAmpRate(int, double, double,int);
void	cCalCmdLimitV(int, int, long, int);

int		cFailCodeCheck(int, int);
int		cFailCodeCheck_parallel_master(int, int); //220317
int		cFailCodeCheck_parallel_slave(int, int); //220317
void	cNextStepCheck(int, int);
void 	cCycle_p_ch_check(int, int);		//190318 add
//kjgw void	cSoftPID(int, int, long, long, long);
long	cCompareTime(int, int, long, long, long);
void	cSoftFeedback(int, int, long, long, long);
void	cSoftFeedback_1(int, int, long, long, long);
void	cSoftFeedback_2(int, int, long, long, long);
void	cSoftFeedback_3(int, int, long, long, long);
void	cSoftFeedback_3p(int, int, long, long, long);
void	cSoftFeedback_4(int, int, long, long, long);
void	cSoftFeedback_CAN(int, int, long, long, long);	//210126 lyhw
void	cSoftFeedback_SOC(int, int, long, long, long); //210609
void	cSoftFeedback_EQU(int, int, long, long, long); //211111
void	cCalculate_Capacitance(int, int, unsigned long);
void	cCalculate_Capacitance_IEC(int, int, unsigned long);
void	cCalculate_Capacitance_Maxwell(int, int, unsigned long);
void	cCalculate_DCR(int, int, unsigned long);
void	cCalculate_DCR_IEC(int, int, unsigned long); //20171227
void	cCalculate_DCR_1(int, int, unsigned long);
void	cCalculate_DCR_2(int, int, unsigned long);
void	cCalculate_DCR_2_SK(int, int, unsigned long);
void	cCalculate_DCR_3(int, int, unsigned long);
void	cCalculate_DCR_3_1(int, int, unsigned long);
void	cCalculate_DCR_4(int, int, unsigned long);	//20200106
void	cCalculate_DCR_5(int, int, unsigned long);	//220322_hun
void	Calculate_Impedance(int, int, int);	//220322_hun

void	RangeSelectV(int);
void	RangeSelectV_Ch(int, int);

void	RangeSelectI(int);
void	RangeSelectI_Ch(int, int);
void	RangeSelectI_Ch_Default(int, int);//20190605 KHK
void	RangeSelectI_Ch_CAN(int, int);//20190605 KHK
void	RangeSelectI_1(int);
void	RangeSelectI_2(int);
void	RangeSelectI_3(int);
void	RangeSelectI_4(int);
void	RangeSelectI_4_Ch(int, int);
void	RangeSelectI_5(int);
void	RangeSelectI_6(int);
void	RangeSelectI_6_Ch(int, int);
void	RangeSelectI_7(int);
void	RangeSelectI_7_Ch(int, int);
void	RangeSelectI_8(int);
void	RangeSelectI_8_Ch(int, int);
void	RangeSelectI_9(int);
void	RangeSelectI_9_Ch(int,int);
void	RangeSelectI_10(int);
void	RangeSelectI_10_Ch(int,int);
void	RangeSelectI_11_Ch(int,int);
void	RangeSelectI_12_Ch(int,int);
void	RangeSelectI_20_Ch(int,int);
void	RangeSelectI_21_Ch(int,int);
void	RangeSelectI_22_Ch(int,int);
void	RangeSelectI_23_Ch(int,int);
void	RangeSelectI_24_Ch(int,int);
void	RangeSelectI_25_Ch(int,int);	//210512 lyhw for 2uA

void	OutputSwitch_OnOff(int);
void	OutputSwitch_OnOff_Ch(int, int);
void	OutputSwitch_OnOff_Ch_Default(int, int);
void	OutputSwitch_OnOff_Ch_CAN(int, int);
void	OutputSwitch_1(int);
void	OutputSwitch_2(int);
void	OutputSwitch_3(int);
void	OutputSwitch_4(int);
void	OutputSwitch_5(int);
void	OutputSwitch_4_Ch(int, int);
void	OutputSwitch_5_Ch(int, int);
void	OutputSwitch_6_Ch(int, int);
void	OutputSwitch_7_Ch(int, int);
void	OutputSwitch_8_Ch(int, int);
void	OutputSwitch_9_Ch(int, int);		//pms add for Multi bd
void	OutputSwitch_10_Ch(int, int);
void	OutputSwitch_11_Ch(int, int);		//210512 lyhw

//120206 kji 
void	CompWriteCh(int, int);

void	ParallelSwitch_OnOff_Ch(int, int);
void	ParallelSwitch_1_Ch(int, int);
void	ParallelSwitch_2_Ch(int, int);
void	ParallelSwitch_3_Ch(int, int);
void	ParallelSwitch_4_Ch(int, int);
void	ParallelSwitch_5_Ch(int, int);
void	ParallelSwitch_5p_Ch(int, int); //kjg_180510
void	ParallelSwitch_6_Ch(int, int);
void	ParallelSwitch_7_Ch(int, int);
void	OutputTrigger(int);
void	OutputTrigger_Ch(int, int);

void	Status_LED_OnOff(int);
void	Status_LED_OnOff_1(int);
void	Status_LED_OnOff_2(int);
void	ReadOtFault(int);
void	ReadHwFault(int);
void	ReadHwFault_Ch(int, int); //Pack Hardware

void	cC_D_Select(int, int, int);
void	cV_Range_Select(int, int, int);
void	cI_Range_Select_1(int, int, int);
void	cI_Range_Select_2(int, int, int);
void	cI_Range_Select(int, int, int);
void	V_Cmd_Output(int);
void	V_Cmd_Output_Ch(int, int);
void	V_Cmd_Output_1(int);
void	V_Cmd_Output_2(int);
void	V_Cmd_Output_2_Ch(int, int);
void	I_Cmd_Output(int);
void	I_Cmd_Output_Ch(int, int);
void	I_Cmd_Output_1(int);
void	I_Cmd_Output_2(int);
void	I_Cmd_Output_2_Ch(int, int);

unsigned char cGrading(int, int, int, unsigned char);
unsigned char cGrading_Check(int, int, int, int, unsigned char);

//int		SelectHwSpec(void);
int		SelectHwSpec(int, int);
void	initCh(int, int);
S_CH_STEP_INFO step_info(int, int);
S_USER_PATTERN_DATA pattern_info(int, int);
int 	find_pattern_time(int, int, long);
int 	find_pattern_time_Default(int, int, long);	//20200108
int 	find_pattern_time_2(int, int, long);	//20200108
int 	temp_wait_flag_check(int, int);
int		acir_wait_flag_check(int, int); //220124 hun
void	ref_output(int, int, long, long, int, int, int, int);
void	ref_output_p(int, int, int, long, long, int, int, int, int);//20190605 KHK
void	user_pattern_data(int, int);
void	Send_Read_UserPattern(int, int, unsigned long, int);	//191120
int		Save_Condition(int, int, unsigned long);

double	calculate_gain(long, long , long, long);
long	calculate_offset(long, long , double);
void	user_map_data(int ,int );
void	user_map_info(int ,int );
int		cEndCond_Aux(int, int);
int		cEndCond_Aux_step(int, int, int, long, int);
int		aux_end_code_check(int, int, int, int, long, long);
int		aux_end_code_check2(int, int, int, int, long, long);

void	DIO_BD_Control(int, int);
void	EfficiencyEnd(int, int, unsigned long, unsigned char);//20170501 oys add

int		read_user_map_ocvTable(int, int);					//20180611 add for digital

long	Convert_C_rate_to_I_value(int, int, long);			//20190801 oys add

int 	deltaCapacityCheck(int, int, int, unsigned long); 	//20190826 oys add
long	cFind_SOC_Tracking_Current(int, int); 				//210609
long	cFind_SOC_Tracking_Current_pattern(int, int, long); //210609
long	cFind_SOV_Tracking_Current(int, int); 				//210720
long	cFind_SOV_Tracking_Current_pattern(int, int, long); //210720
long	cFind_SOH_Tracking_Current(int, int); 				//211022
long	cFind_equation_current(int, int); 					//211111
int		gotoStepCheck(int, int);				//210511 LJS
int 	return_check_value(long, long, long);   //210511 LJS
long	Calculate_C_Rate_Capacity(int, int, int);
long	Calculate_End_Capacity(int, int, int, int);
void	cCalculate_DCR_2_Semi_Autojig(int, int, unsigned long); //220221 LJS
#endif
