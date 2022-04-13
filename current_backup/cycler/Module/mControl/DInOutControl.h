#ifndef __DINOUTCONTROL_H__
#define __DINOUTCONTROL_H__

void	DInOutControl(int);
void	DInScan(void);
int		DInCheck(int, int, int, int);
void	DOutScan(void);
void	DIn_FlagCheck(void);
int		DIn_FlagCheck_PowerSwitch(void);
int		DIn_FlagCheck_MainEMG(void);
int		DIn_FlagCheck_PowerFail(void);
int		DIn_FlagCheck_SMPSFail(void);
int		DIn_FlagCheck_DC_FAN_FAIL1(void);
int		DIn_FlagCheck_DC_FAN_FAIL2(void);
void	DIn_FlagCheck_ChamberFail(void);
void	DIn_FlagCheck_ChamberFail1(void);
void	DIn_FlagCheck_ChamberFail2(void);	//191001 lyhw
void	DIn_Limit_Check(void);
void	DIn_Buzzer_Stop(void);
void	DIn_Jig_Check(void);
int		SMPS_FAIL_1(void);
int		SMPS_FAIL_2(void);
int		SMPS_FAIL_3(void);
int		SMPS_FAIL_4(void);
void	DOut_FlagCheck(void);
void	DOut_Flag_1(void);
void	DOut_Flag_2(void);
void	DOut_Flag_3(void);
void	DOut_Flag_4(void);
void	DOut_Flag_5(void);
void	DOut_Flag_6(void);
void	DOut_Flag_7(void);
void	DOut_Flag_8(void); 			//181127 Add
void	DOut_Flag_9(void);			//180611 add for digital
void	DOut_Flag_10(void);			//180611 add for digital
void	DOut_Flag_11(void);			//210308 lyhw for can
void	DOut_Flag_Default(void);
void	Clear_OutPort(void);
void	BuzzerSet(int, int, int);
void	BuzzerControl(void);
void	ThermalSensor(void);
void 	CheckHwFault(void);
int		FaultLamp_Control(void);
unsigned char Read_InPoint(int);
void	Select_OutPoint(int, int);

//20151115 khk add
void Check_OVP(void);
void Check_OTP(void);
void Check_CCC(void);

int DIn_Door_Open_FAIL1(void);
int DIn_Door_Open_FAIL2(void);
int DIn_Door_Open_FAIL(void);		//180820 add lyh 
int DIn_Smoke_FAIL(void);
int Dout_TowerLamp_Flag(void);
int Dout_TowerLamp_Flag_MultiBd(void); //20181127 Add
unsigned char Read_OutPoint(int);
void EMG_Signal_Check(void);			//191001 lyhw
void Check_OVP_Default(void);			//210901 LJS
void Check_OVP_1(void);					//210901 LJS
void Check_OTP_Default(void);			//210901 LJS
void Check_OTP_1(void);					//210901 LJS
int DYSON_Door_Open(int, int, int);		//220127 jsh
#endif
