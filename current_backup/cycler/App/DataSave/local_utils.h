#ifndef __LOCAL_UTILS_H__
#define __LOCAL_UTILS_H__

void	Init_SystemMemory(void);
int		Read_DataSave_Config(void);
int		Open_ResultData_1(int);
int		Open_ResultData_2(int);
void	Save_ResultData_1(void);
int		Save_ResultData_2(int);
void	Save_PulseData_1(void);
int		Save_PulseData_2(int);
int		Save_PulseData_IEC(int);	//171227 add
void	Save_10mS_Data_1(void);
int		Save_10mS_Data_2(int, int);
//171211 lyh add
int		Read_User_Pattern(const int, const int, const int);
//191120 lyhw
int		Read_User_Pattern_1(const int, const int, const int, const int);
int		Read_User_Pattern_2(const int, const int, const int, const int);
int		Read_User_Map(const int, const int);
//210609
long	Calculate_Crate_Value(unsigned char, int);
int		Read_SOC_Tracking_File(const int, const int, const int, const int);
//20200427
int		Read_User_Pattern_NoUserData(const int, const int, const int);
#endif
