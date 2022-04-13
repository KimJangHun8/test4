#ifndef MAIN_BD_H
#define MAIN_BD_H


typedef struct _mInputValue
{
	int refreshFlag;
	unsigned char smpsFail7V;
	unsigned char smpsFail3V;
	unsigned char ot1;
	unsigned char ot2;
}mInputValue;

//Main mother
void Test_Main_Mother(void);

//fpga ad2 type
void Test_FPGA_8Ch(void);
void Test_FPGA_16Ch(void);
void Test_FPGA_32Ch(void);

//cpld ad2 type
void Test_CPLD_8Ch_AD2(void);
void Test_CPLD_16Ch_AD2(void);
void Test_CPLD_32Ch_AD2(void);

//cpld ad1 type
void Test_CPLD_8Ch_AD1(void);
void Test_CPLD_16Ch_AD1(void);
void Test_CPLD_32Ch_AD1(void);

//backplan type
void Test_Backplan_64Ch(void);
void Test_Backplan_32Ch(void);

// AD/DA
//main bd
void Test_32Ch_AD2_AD_DA(int installedCh, int refVal, int bdType);
void Test_32Ch_AD1_AD_DA(int installedCh, int refVal, int bdType);

//backplan
void Test_Backplan_AD2_AD_DA(int installedCh, int refVal, int bdType);

//only AD
void Test_32Ch_AD2_AD(int installedCh, int bdType);
void Test_32Ch_AD1_AD(int installedCh, int bdType);
void Test_Backplan_AD2_AD(int installedCh, int bdType);

//main mother fault 
void Draw_Fault_State(mInputValue* input);
void Read_Fault_Value(mInputValue* input);
void Init_Fault_Value(mInputValue* input);
int Compare_Board_Fault_Value(mInputValue* org, mInputValue* other);

//print data
void Print_32Ch_AD_Data(float* vArray, int current, int installedCh);
void Print_64Ch_AD_Data(float* vArray, int current, int installedCh);


#endif
