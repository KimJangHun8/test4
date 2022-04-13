#ifndef __FAULTCOND_H__
#define __FAULTCOND_H__
//Main Fault
void	cFaultCondHard(int, int);
void	cFaultCondHard_P(int, int);
void	cFaultCondHard_P2(int, int);
void	cFaultCondSoft(int, int);
void	cFaultCondSoft_P(int, int);
void	cFaultCondSoft_P2(int, int);
void	cFaultCond_Aux(int, int);
int		cFaultCond_Aux_step(int, int, int, long, int);
void	cFaultCond_Aux_P(int, int);
int		cFaultCond_Aux_step_P(int, int, int, long, int);
void	DropV_Charge_p(int, int);				//210914 LJS
void	DropV_DisCharge_p(int, int); 			//210914 LJS
void	DropV_Charge(int, int);					//210914 LJS
void	DropV_DisCharge(int, int);				//210914 LJS

//HW Fault
int		SAMSUNG_SDI_safety(int, int, int, int);				//20190905 oys add
int		cvFault_Check(int, int, int, int, long);			//210204 lyhw add

//SW Fault
int		tempFaultCheck(int, int, unsigned long, unsigned char); //add <--20150827 oys
int		tempPauseCheck(int, int, unsigned long, unsigned char); //20190311
int		Fault_Check_default_LG(int, int);					//20180314 sch add
int		Fault_Check_Change_VI(int, int, unsigned long);		//190901 lyhw
int		Fault_Check_Gas_Data(int, int, unsigned long);		//210923 lyhw
int		Fault_Check_Rest_Voltage(int, int);					//210518 hunw

int		code_convert(int, int);		//211025 hun
void	Fault_Value_Check(long,long,long,long,long,long,long,long,long,long);
int		errorCodeCheck_CAN(int, int, int);					//210126 lyhw
int 	SDI_CC_CV_hump_Check(int, int, int, int);	//211125_hun
int		Group_Error_Fault_Check(int, int, unsigned long);	//220214 ljs
#endif
