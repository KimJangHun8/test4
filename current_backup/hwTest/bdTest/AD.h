#ifndef _AD_H
#define _AD_H
//boad type
#define CPLD_TYPE 		0
#define FPGA_TYPE 		1
#define BACKPLAN_TYPE	2

//ad count 
#define AD_NUM1			1
#define AD_NUM2			2

#define HIGH	1
#define LOW		0

union ADData{
		char		data[2];
		short int	val;
};


typedef struct _ad_data{
		float vData[64];
		float iData[64];
		float vRefData[4];
		float iRefData[4];
}AdData;

typedef struct _ad_addr{
		int baseAddr;
		int adStart;
		int	voltageAddr;
		int	currentAddr;
		int muxEnableAddr;
		int muxAddr;
		int chAddr;
		int adCount;
		int bdType;
}AdAddr;


//					address setting
void Setup_Main_Bd_AD2_FPGA_Address(AdAddr* addressMap);
void Setup_Main_Bd_AD2_CPLD_Address(AdAddr* addressMap);
void Setup_Main_Bd_AD1_CPLD_Address(AdAddr* addressMap);
void Setup_Backplan_AD_Address(AdAddr* addressMap);

//					read ad data
void Read_32Ch_AD2_Data
					(AdAddr* addressMap, int installedCh, float* array);
void Read_32Ch_AD1_Data
					(AdAddr* addressMap, int installedCh, float* array);
void Read_Backplan_64Ch_AD2_Data
					(AdAddr* addressMap, int installedCh, float* array);
void Read_Backplan_32Ch_AD2_Data
					(AdAddr* addressMap, int installedCh, float* array);

//					mux setting
void Set_Mux(AdAddr* addressMap, int type, int ch);
void Set_Mux_AD1(AdAddr* addressMap, int type, int ch);
void Set_Mux_Backplan_32Ch(AdAddr* addressMap, int type, int ch);
void Set_Mux_Backplan_32Ch(AdAddr* addressMap, int type, int ch);

#endif
