#ifndef __MAIN_H__
#define __MAIN_H__

void	Test_Print(unsigned char);
void 	DataSize_Print(void);
void	Debug_Print(void);
void	DAQ_Print(void);
void	DAQ_Print(void);
void	Addr_Print(void);
void	TimeSlot_Print(void);
void	AppControl_Print(void);
void	DataSave_Print(void);
void	CaliMeter_Print(void);
void	CaliMeter2_Print(void);
void	AnalogMeter_Print(void);
void	FADM_Print(void);
void	ModuleState_Print(void);
void	GroupState_Print(void);
void	Reference_IC_Data_Save(void);
void	BoardState_Print(void);
void	ChannelState_Print(void);
void	DIO_Print(void);
void	TestCond_Print(void);
void	CaliData_Print(void);
void 	CaliMeter_Config_Print(void);
void 	Chamber_Temp_Change_Print(void);
void 	Temp_Change_Print(void);
void 	HwFault_Config_Print(void);
void 	PCU_State_Print(void);
void	CAN_State_Print(void);
void	UserDataNo_Print(void);
void 	AC_Fail_Recovery_print(void);
void	Convert_FormToCyc_CaliData(void);
void	Read_HwFault_Config(void);
void 	CAN_JIG_IO_Change_Print(void);
int 	Read_Cycler_BdCaliData(int);
int 	Read_Form_Cali_Data(int, int); 
int 	Read_Form_Cali_Check_Data(int, int); 
int 	Write_Cycler_BdCaliData(int);
int		KeyInput(void);
int		main(void);
void 	MainClient_Config_Print(void);
void	GAS_AMBIENT_TEST(void);
#endif
