#ifndef __LOCAL_UTILS_H__
#define __LOCAL_UTILS_H__

int		Initialize(void);
void	Init_SystemMemory(void);
int		Read_MainClient_Config(void);
int		Write_MainClient_Config(void);
void	CaliUpdateCh(int, int);
int		Write_BdCaliData(int);
int		Write_BdCaliData_FAD(int);
int 	Write_BdCaliData_I_Offset(int);//20160229
void	Convert_testCond(int, int, int);
long 	Convert_C_rate_to_I_value(int, int, long);
int		GradeCodeCheck(int, int, unsigned long, long);
void	StateChange_Pause(int);
void	Update_RealTime(char *);	//131228 oys w
int		Write_Shunt_Cali_Info(int); //140623 oys w
void	EquipmentInfoPrint(void);
#endif
