#ifndef __MODULECONTROL_H__
#define __MODULECONTROL_H__

void	ModuleControl(void);
void	mSelfTest(void);
void 	mSignalCheck(void);
void	GroupControl(void);
void	chStepSync(int, int);
void	groupErrorCheck(int, int);	//220203_hun
void	groupErrorCheck_EndTime(int, int);	//220223_LJS
void	PS_ON_1(void);
void	PS_ON_2(void);
void	PS_ON_3(void);
void	PS_ON_4(void);
void	GUI_TO_SBC_SHUTDOWN(void);
void	Fault_Check(void);
void	Fault_Check_CAN_1(void);
void	Fault_Check_1(void);
void	Fault_Check_2(void);
void	Fault_Check_3(void);
void	Fault_Check_4(void);
void	Fault_Check_5(void);
void	Fault_Check_6(void);
void	Fault_Check_7(void);
void	Fault_Check_8(void);
void	Fault_Check_9(void);
void	Fault_Check_10(void);
void	Fault_Check_11(void);
void	Fault_Check_11_OT_20(void);
void	Fault_Check_12(void);
void	GUI_TO_SBC_BUZZER_CONTROL(void);
void	Fault_Check_13(void);			//180611 add for digital
void	mInv_Signal(void);				//180611 add for digital
void	PCU_INV_Signal(void);			//180611 add for digital
//void 	Fault_Check_default_LG(void);
void 	Inverter_Signal_CAN(void);//CAN Type
void 	GUI_cmd_Buzzer_stop(void);		//181026 add lyhw
void	mainState_Connection_Check(void);
//hun_210824
void	AnalogMeter_Cali(void);
void 	AuxTemp_ChData_Mapping(void);
int 	analog_cali_calc_2(int);
int		temp_cali_data_select(void);	//211124_hun
void 	Ch_Code_Check(void);		//220322_hun
#endif
