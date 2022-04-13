#ifndef __PCU_CONTROL_H__
#define __PCU_CONTROL_H__

void	PCU_Control(int, int);
void	pulse_data_1sec(int, int);
void	pStep(int, int);
void	pIdle(int, int);
void	pStandby(int, int);
void	pPause(int, int);
void	pStepCharge(int, int);
void	pStepDischarge(int, int);
void	pStepRest(int, int);
void	pStepOcv(int, int);
void	pStepZ(int, int);
void	pStepUserPattern(int, int);
void	pStepBalance(int, int); 
void	pStepGoto(int, int);
void	pStepUserMap(int, int);

void	pCali(int, int);
void	pCaliUpdate(int, int);					//180813 lyhw
int 	pCalibrationV(int, int, int);
int		pCalibrationCheckV(int, int, int);
int 	pCalibrationI(int, int, int);
int		pCalibrationCheckI(int, int, int);
long    pCalCmdV(int, int, long, int, int);
long    pCalCmdI(int, int, long, int, int);
void    pcu_ref_output(int, int, unsigned char, unsigned char, long, long, int, int, int, int);
void	pCalChAverage(int, int);
void	pConvert_data(int, int, int);
void	pcu_user_pattern_data(int, int);
void 	pFaultCheck(int, int);
void	pFaultCondHard_P(int, int);				//191017
void	pFaultCondHard(int, int);				//191017

void	PCU_Inverter_Control(int, int);			//180627
void 	PCU_Inverter_Fault(int, int);	
void 	PCU_Inverter_Fault_DC(int, int);		//210304 lyhw
void 	PCU_Inverter_Fault_AC(int, int);		//210304 lyhw

void	CH_Inverter_Signal(int, int);			//180830
void 	pCali_Ch_Select_auto_3(int ,int, int);	//180726
void	PCU_State_Check(int, int);				//190424
int		pcu_temp_wait_flag_check(int, int);		//190624
void	PCU_INV_State(int, int);				//200417 lyhw
void	pFaultCond_Common(int, int);			//200905 lyhw
void	pStepCCCV_Check(int, int, long, long); //210507
int 	chamber_temp_humidity_check(int, int, unsigned long);	//kjc_210413
#endif
