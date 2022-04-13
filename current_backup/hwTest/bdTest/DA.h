#ifndef _DA_H
#define _DA_H

#define DAC_712			0
#define DAC_7741		1
#define CPLD_TYPE		0
#define FPGA_TYPE		1
#define BACKPLAN_TYPE	2
#define HIGH			1
#define LOW				0

typedef struct _da_addr{
	int runAddr;
	int voltageAddr;
	int currentAddr;
	int daEnableAddr;
	int daHighAddr;
	int daLowAddr;
	int bdType;
	int dacType;
}DaAddr;

union DAData{
		char		data[2];
		short int	val;
};

//				addresss setting
void Setup_Main_Bd_DA_CPLD_Address(DaAddr* daAddressMap);

void Setup_Main_Bd_DA_FPGA_Address(DaAddr* daAddressMap);

void Setup_Backplan_Bd_DA_Address(DaAddr* daAddressMap);

void Setup_Mother_Bd_DA_Address(DaAddr* daAddressMap);


//				main bd type
//V output
void Output_Voltage_Channel(DaAddr* daAddressMap, int ch, float refVal);

void Output_Voltage_All_Channel
					(DaAddr* daAddressMap, int installedCh, float refVal);

//I output
void Output_Current_Channel(DaAddr* daAddressMap, int ch, float refVal);

void Output_Current_All_Channel
					(DaAddr* daAddressMap, int installedCh, float refVal);


//				backplan type
//V output
void Output_Voltage_Channel_For_Backplan_32Ch
					(DaAddr* daAddressMap, int ch, float refVal);

void Output_Voltage_Channel_For_Backplan_64Ch
					(DaAddr* daAddressMap, int ch, float refVal);

void Output_Voltage_All_Channel_For_Backplan_32Ch
					(DaAddr* daAddressMap, int installedCh, float refVal);
					
void Output_Voltage_All_Channel_For_Backplan_64Ch
					(DaAddr* daAddressMap, int installedCh, float refVal);

//I output
void Output_Current_Channel_For_Backplan_32Ch
					(DaAddr* daAddressMap, int ch, float refVal);

void Output_Current_Channel_For_Backplan_64Ch
					(DaAddr* daAddressMap, int ch, float refVal);

void Output_Current_All_Channel_For_Backplan_32Ch
					(DaAddr* daAddressMap, int installedCh, float refVal);

void Output_Current_All_Channel_For_Backplan_64Ch
					(DaAddr* daAddressMap, int installedCh, float refVal);


								
//저전류(mother type)
//전압
void Output_Voltage_Channel_For_Mother
					(DaAddr* daAddressMap, int ch, float refVal);

void Output_Voltage_All_Channel_For_Mother
					(DaAddr* daAddressMap, int ch, float refVal);

//전류
void Output_Current_Channel_For_Mother
					(DaAddr* daAddressMap, int ch, float refVal);

void Output_Current_All_Channel_For_Mother
					(DaAddr* daAddressMap, int installedCh, float refVal);


#endif
