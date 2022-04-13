#ifndef _RANGE_H
#define _RANGE_H
#include "BitMacro.h"

#define MAX_RANGE 4
#define RANGE1 0
#define	RANGE2 1
#define RANGE3 2
#define RANGE4 3

#define CPLD_TYPE 0
#define FPGA_TYPE 1
#define BACKPLAN_TYPE 2

#define USE 1
#define DONT_USE 0

typedef struct _range{
	int		addr;
	int		runAddr;
	int		range1;
	int		range2;
	int		range3;
	int		range4;
	int		useFlag[4];
	int		installedCh;
	int		baseAddr;
}Range;

// 				addresss setting
void Setup_Main_BD_CPLD_Range_Address
					(Range* rangeAddress, int chNumber);
void Setup_Main_BD_FPGA_Range_Address
					(Range* rangeAddress, int chNumber);
void Setup_Backplan_BD_FPGA_Range_Address
					(Range* rangeAddress, int chNumber);


void Test_Main_BD_Range(int installedCh, int bdType);
void Test_Parallel();
void Turn_On_All_Range(Range* rangeAddress);
void Turn_Off_All_Range(Range* rangeAddress);
void Turn_On_Range(Range* rangeAddress);
void Turn_On_Range_Reverse(Range* rangeAddress);

//				backplan  32Ch function
void Turn_On_All_Range_Backplan_32Ch(Range* rangeAddress);
void Turn_Off_All_Range_Backplan_32Ch(Range* rangeAddress);
void Turn_On_Range_Backplan_32Ch(Range* rangeAddress);


#endif
