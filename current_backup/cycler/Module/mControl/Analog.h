#ifndef __ANALOG_H__
#define __ANALOG_H__

void	AnalogValue_Input_100mS(int);
void	AnalogValue_Input_100mS_Ch(int);
void	AnalogValue_Input_100mS_Ch_2(int);
void	AnalogValue_Input_100mS_Ch_AD2_64Ch(int);
void	AnalogValue_Input_50mS(int);
void	AnalogValue_Input_50mS_Ch(int);
void	AnalogValue_Input_50mS_Ch_AD2(int);
void	AnalogValue_Input_50mS_Ch_AD2_2uA(int);	//210510 lyhw
void	AnalogValue_Input_25mS_Ch_AD2(int);
void	AnalogValue_Input_20mS_Ch_AD2(int);
void	AnalogValue_Input_20mS_Ch_AD2_FAD(int);
void	AnalogValue_Input_20mS_Pack(int);
void	AnalogValue_Input_10mS_Ch_AD2(int);
void 	AnalogValue_Input_100mS_CAN(int);	//20190605 KHK
void 	AnalogValue_Input_50mS_CAN(int);	//210204 lyhw

void  	GetADValue(int, int);
void  	GetADValue_Ch(int, int, int);
void  	GetADValue_Ch_Default(int, int, int);
void  	GetADValue_Ch_2(int, int, int);
void  	GetADValue_Ch_AD2(int, int, int, int);
void  	GetADValue_Ch_AD2_2uA(int, int, int, int);	//210510 lyhw
void  	GetADValue_Ch_AD2p(int, int, int, int);
void  	GetADValue_Ch_Pack(int, int, int);

void 	ReadADData(int, int, int, int, int);
void 	ReadADData_Default(int, int, int, int, int);
void 	ReadADData_2(int, int, int, int, int);
void 	ReadADData_AD2(int, int, int, int, int);
void 	ReadADData_Pack(int, int, int, int);

void 	SetMux(int, int);
void 	SetMux_Off(int, int, int);
void 	SetMux_1( int, int, int);
void 	SetMux_2( int, int, int);
void 	SetMux_3( int, int, int);
void 	SetMux_4( int, int, int);
void 	SetMux_5( int, int, int);
void 	SetMux_6( int, int, int);
void 	SetMux_7( int, int, int);
void 	SetMux_8( int, int, int);
void 	SetMux_9( int, int, int);
void 	SetMux_10( int, int, int);
void 	SetMux_11( int, int); 		//Pack Hardware
void 	SetMux_12( int, int, int);	//for MULTI BD  PMS add
void 	SetMux_13( int, int, int);
void 	SetMux_14( int, int, int);
void 	SetMux_15( int, int, int);	//210510 lyhw

void	CalChAD_Filter(int);
void	CalChAD_Filter_Ch(int, int, int);
void	CalChAD_Filter_Ch_Default(int, int, int);
void	CalChAD_Filter_Ch_2(int, int, int);
void	CalChAD_Filter_Ch_AD2(int, int, int);
void	CalChAD_Filter_Ch_CAN(int, int, int);//20190605 KHK

void	CalSourceAD_Filter(int, int);
void	CalSourceAD_Filter_Ch(int, int, int);
void	CalSourceAD_Filter_Ch_AD2(int, int, int);

void	ConvertAD_Data(int, int, int, double);
void	ConvertAD_Data_CAN(int, int, int, double);

void  	CalChAverage(int);
void  	CalChAverage_Ch(int, int, int);
void  	CalChAverage_Ch_AD2(int, int, int);
void  	CalChAverage_Ch_AD2p(int, int, int);
void  	CalChAverage_Ch_CAN(int, int);//20190605 KHK

void  	CalSourceAverage(int);
void  	CalSourceRingBuffer(int);
void  	CalibratorSource(int);
int		cFindADCaliPoint(int, int, double, int, int);
int		cFindADCaliPoint_Default(int, int, double, int, int);
int		cFindADCaliPoint_2(int, int, double, int, int);
void   	cCalculate_Voltage(int, int);
void   	cCalculate_Voltage_2(int, int);
void   	cCalculate_Current(int, int);
void   	cCalculate_Current_p(int, int, int, int);
void   	cCalculate_Capacity(int, int);
void   	cCalculate_Watt(int, int);
void 	SWAP(double *, double *);
void	cSimulation(int, int, int, int, int);
//void  	CalSourceAverage2(int);
//void  	CalSourceAverage3a(int);
//void  	CalSourceAverage3b(int);
//void  	CalSourceAverage4(int, int);
#endif
