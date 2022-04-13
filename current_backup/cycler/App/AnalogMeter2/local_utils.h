#ifndef __LOCAL_UTILS_H__
#define __LOCAL_UTILS_H__

void	Init_SystemMemory(void);
int		Read_AnalogMeter_Config(void);
int		Read_AnalogMeter_CaliConfig(void);
int		Read_AnalogMeter_CaliData(void);
int		Write_AnalogMeter_Config(void);
int		Write_AnalogMeter_CaliData(void);
int		Read_TempArray_A(void);
#endif
