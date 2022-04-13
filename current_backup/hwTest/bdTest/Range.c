#include <asm/io.h>
#include <stdlib.h>
#include "Range.h"

/*----------------------------------------------------------------------------- 	
	date : 2013							editor:pms
 
	discription:

	This function is for range test.

	1. address setting
	2. range test.
		2.1 test all main board except 32ch backplan board
	    2.2 test 32ch backplan board range 

-----------------------------------------------------------------------------*/ 
void Test_Main_BD_Range(int installedCh, int bdType)
{

	Range rangeAddress;
	Range* pRangeAddress;
	int integer;

	pRangeAddress = &rangeAddress;
	
	if(iopl(3)) exit(1);

	//1. address setting
	switch(bdType){
		case CPLD_TYPE:
			Setup_Main_BD_CPLD_Range_Address(pRangeAddress, installedCh);
			break;
		case FPGA_TYPE:
			Setup_Main_BD_FPGA_Range_Address(pRangeAddress, installedCh);
			break;
		case BACKPLAN_TYPE:
			Setup_Backplan_BD_FPGA_Range_Address(pRangeAddress, installedCh);
			break;
	}
	//2. range test.
	
		//2.1 test all main board except 32ch backplan board
	if(bdType == BACKPLAN_TYPE && installedCh < 64){
		for(integer = 0; integer < 2; integer++){
			Turn_On_All_Range_Backplan_32Ch(pRangeAddress);
			sleep(1);
			Turn_Off_All_Range_Backplan_32Ch(pRangeAddress);
			sleep(1);
		}
		Turn_On_Range_Backplan_32Ch(pRangeAddress);
	

	    //2.2 test 32ch backplan board range 
	}else{
		for(integer = 0; integer < 2; integer++){
			Turn_On_All_Range(pRangeAddress);
			sleep(1);
			Turn_Off_All_Range(pRangeAddress);
			sleep(1);
		}
		Turn_On_Range(pRangeAddress);
	}
	
}


/*----------------------------------------------------------------------------- 	
	date : 2013							editor:pms
 
	discription:

	This function is parallel test for main mother type board.
   other type board can't test, so you should check main mother type bd.

	1. all parallel led on
 	2. all parallel led off
	3. turn on led in regular sequence
-----------------------------------------------------------------------------*/ 
void Test_Parallel()
{
	
	int i;
	unsigned char outPutValue;
	if( iopl(3)) exit(1);
	
	//1. all parallel led on
	outb(0x08, 0x63f);
	usleep(2000);
	outb(0x0f, 0x620);
	usleep(2000);
	outb(0x0f, 0x621);
	usleep(500000);
	
	//2. all parallel led off
	outb(0x00, 0x620);
	usleep(2000);
	outb(0x00, 0x621);
	usleep(2000);
	outPutValue = 0x01;

	//3. turn on led in regular sequence
	for(i = 0; i < 4; i++){
		outb((outPutValue << i) , 0x620);
		usleep(500000);
	}
	outb(0x00, 0x620);
	outPutValue = 0x01;
	for(i = 0; i < 4; i++){
		outb((outPutValue << i) , 0x621);
		usleep(500000);

	}
	outb(0x00, 0x621);
	outb(0x00, 0x63f);
}
		



/*----------------------------------------------------------------------------- 	
	date : 2013							editor:pms
 
	discription:
	seting address for CPLD type
	bits-  80   |  40   |  20   |  10   |   8   |   4   |   2   |   1   |
	62f - RANGE4| RANGE3| RANGE2| RANGE1|   X   |   X   |   X   | 	Run	|
	USE	   X	|  USE	|  USE	|	X	|	X	|	X	|	X	|	USE	|

-----------------------------------------------------------------------------*/ 
void Setup_Main_BD_CPLD_Range_Address(Range* rangeAddress, int installedCh)
{

	int range;
	
	rangeAddress->addr = 0x62f;
	rangeAddress->runAddr = 0x01;
	rangeAddress->range1 = 0x10;
	rangeAddress->range2 = 0x20;
	rangeAddress->range3 = 0x40;
	rangeAddress->range4 = 0x80;
	for(range = 1; range <= MAX_RANGE; range++){
		if( range == RANGE2){
			rangeAddress->useFlag[range] = USE;
		}else if(range == RANGE3){
			rangeAddress->useFlag[range] = USE;
		}else{
			rangeAddress->useFlag[range] = DONT_USE;
		}
	}
	rangeAddress->installedCh = installedCh;
	rangeAddress->baseAddr = 0x620;
}
/*----------------------------------------------------------------------------- 	
	date : 2013							editor:pms
 
	discription:
	
	seting address for FPGA type
 	bits-  80   |  40   |  20   |  10   |   8   |   4   |   2   |   1   |
 	63f - RANGE4| RANGE3| RANGE2| RANGE1|   X   |   X   |   X   | 	Run	|
 	USE	   X	|  USE	|  USE	|	X	|	X	|	X	|	X	|	USE	|

-----------------------------------------------------------------------------*/ 
void Setup_Main_BD_FPGA_Range_Address(Range* rangeAddress, int installedCh)
{
	int range;

	rangeAddress->addr = 0x63f;
	rangeAddress->runAddr = 0x01;
	rangeAddress->range1 = 0x10;
	rangeAddress->range2 = 0x20;
	rangeAddress->range3 = 0x40;
	rangeAddress->range4 = 0x80;
	for(range = 0; range < MAX_RANGE; range++){
		if( range == RANGE2){
			rangeAddress->useFlag[range] = USE;
		}else if(range == RANGE3){
			rangeAddress->useFlag[range] = USE;		
		}else if(range == RANGE4){
			rangeAddress->useFlag[range] = USE;
		}else{
			rangeAddress->useFlag[range] = DONT_USE;
		}
	}
	rangeAddress->installedCh = installedCh;
	rangeAddress->baseAddr = 0x620;
}
/*----------------------------------------------------------------------------- 	
	date : 2013							editor:pms
 
	discription:
 	seting address for FPGA type
 	bits-  80   |  40   |  20   |  10   |   8   |   4   |   2   |   1   |
 	63f - RANGE4| RANGE3| RANGE2| RANGE1|   X   |   X   |   X   | 	Run	|
 	USE	   X	|  USE	|  USE	|	X	|	X	|	X	|	X	|	USE	|
 
-----------------------------------------------------------------------------*/ 
void Setup_Backplan_BD_FPGA_Range_Address(Range* rangeAddress, int installedCh)
{
	int range;

	rangeAddress->addr = 0x63f;
	rangeAddress->runAddr = 0x01;
	rangeAddress->range1 = 0x10;
	rangeAddress->range2 = 0x20;
	rangeAddress->range3 = 0x40;
	rangeAddress->range4 = 0x80;
	for(range = 0; range < MAX_RANGE; range++){
		if( range == RANGE2){
			rangeAddress->useFlag[range] = USE;
		}else if(range == RANGE3){
			rangeAddress->useFlag[range] = USE;		
		}else if(range == RANGE4){
			rangeAddress->useFlag[range] = USE;
		}else{
			rangeAddress->useFlag[range] = DONT_USE;
		}
	}
	rangeAddress->installedCh = installedCh;
	rangeAddress->baseAddr = 0x620;
}

/*----------------------------------------------------------------------------- 	
	date : 2013							editor:pms
 
	discription:
  
	1. all run signal on
    2. all range signal on
 
-----------------------------------------------------------------------------*/ 
void Turn_On_All_Range(Range* rangeAddress)
{
	int max, range;
	int value = 0x10;
	int i;
	
	max = rangeAddress->installedCh / 8;
	
	//1 all run signal on
	outb(rangeAddress->runAddr, rangeAddress->addr);
	for(i= 0;  i < max; i++){
		outb(0xff, rangeAddress->baseAddr + i);
	}
	outb(rangeAddress->runAddr, rangeAddress->addr);

	//2.all range signal on
	for(range = 0; range < MAX_RANGE; range++){
		outb(value , rangeAddress->addr);
		if(rangeAddress->useFlag[range] == USE){
			for(i = 0; i < max; i++){
				outb(0xff, rangeAddress->baseAddr + i);
			}
		}
		value = value << 1;
	}
	outb(0x00, rangeAddress->addr);
}
/*----------------------------------------------------------------------------- 	
	date : 2013							editor:pms
 
	discription:
  	1. all run signal on
    2. all range signal on

-----------------------------------------------------------------------------*/ 
void Turn_On_All_Range_Backplan_32Ch(Range* rangeAddress)
{
	int max, range;
	int value = 0x10;
	int i;
	
	max = rangeAddress->installedCh / 8;
	
	//1 all run signal on
	outb(rangeAddress->runAddr, rangeAddress->addr);
	for(i= 0;  i < max; i++){
		outb(0xff, rangeAddress->baseAddr + (i*2));
	}
	outb(rangeAddress->runAddr, rangeAddress->addr);

	//2.all range signal on
	for(range = 0; range < MAX_RANGE; range++){
		outb(value , rangeAddress->addr);
		if(rangeAddress->useFlag[range] == USE){
			for(i = 0; i < max; i++){
				outb(0xff, rangeAddress->baseAddr + (i*2));
			}
		}
		value = value << 1;
	}
	outb(0x00, rangeAddress->addr);
}

/*----------------------------------------------------------------------------- 	
	date : 2013							editor:pms
 
	discription:
  	1. all run signal off
    2. all range signal off

-----------------------------------------------------------------------------*/ 
void Turn_Off_All_Range(Range* rangeAddress)
{
	int max, range;
	int value = 0x10;
	int i;
	
	max = rangeAddress->installedCh / 8;
	//1 all run signal off
	outb(rangeAddress->runAddr, rangeAddress->addr);
	for(i = 0; i < max; i++){
		outb(0x00, rangeAddress->baseAddr + 2);
	}
	outb(rangeAddress->runAddr, rangeAddress->addr);

	//2.all range signal off
	for(range = 0; range < MAX_RANGE; range++){
		outb(value , rangeAddress->addr);
		if(rangeAddress->useFlag[range] == USE){
			for(i = 0; i < max; i++){
				outb(0x00, rangeAddress->baseAddr + i);
			}
		}
		value = value << 1;
	}
	outb(0x00, rangeAddress->addr);
}
/*----------------------------------------------------------------------------- 	
	date : 2013							editor:pms
 
	discription:
  	1. all run signal off
    2. all range signal off
 
-----------------------------------------------------------------------------*/ 
void Turn_Off_All_Range_Backplan_32Ch(Range* rangeAddress)
{
	int max, range;
	int value = 0x10;
	int i;
	
	max = rangeAddress->installedCh / 8;
	//1 all run signal off
	outb(rangeAddress->runAddr, rangeAddress->addr);
	for(i = 0; i < max; i++){
		outb(0x00, rangeAddress->baseAddr + (i*2));
	}
	outb(rangeAddress->runAddr, rangeAddress->addr);

	//2.all range signal off
	for(range = 0; range < MAX_RANGE; range++){
		outb(value , rangeAddress->addr);
		if(rangeAddress->useFlag[range] == USE){
			for(i = 0; i < max; i++){
				outb(0x00, rangeAddress->baseAddr + (i*2));
			}
		}
		value = value << 1;
	}
	outb(0x00, rangeAddress->addr);
}
/*----------------------------------------------------------------------------- 	
	date : 2013							editor:pms
 
	discription:
  	1. run signal on
    2. range signal on

-----------------------------------------------------------------------------*/ 
void Turn_On_Range(Range* rangeAddress)
{
	int max, range;
	int value = 0x10;
	int outputValue;
	int i, j;
	
	max = rangeAddress->installedCh / 8;
	
	//1 all run signal on
	outb(rangeAddress->runAddr, rangeAddress->addr);
	for(i= 0;  i < max; i++){
		outputValue = 0x01;
		for( j = 0; j < 8; j++){
			outb((outputValue << j), rangeAddress->baseAddr + i);
			usleep(500000);
		}
		outb(0x00, rangeAddress->baseAddr +i);
	}
	outb(rangeAddress->runAddr, rangeAddress->addr);

	
	//2.all range signal on
	for(range = 0; range < MAX_RANGE; range++){
		outb(value , rangeAddress->addr);
		if(rangeAddress->useFlag[range] == USE){
			for(i = 0; i < max; i++){
				for(j = 0; j < 8; j++){
					outb(outputValue << j , rangeAddress->baseAddr + i);
					usleep(500000);
				}
				outb(0x00, rangeAddress->baseAddr + i);
			}
		}
		value = value << 1;
	}
	outb(0x00, rangeAddress->addr);

}
/*----------------------------------------------------------------------------- 	
	date : 2013							editor:pms
 
	discription:
  	1. run signal on
    2. range signal on

-----------------------------------------------------------------------------*/ 
void Turn_On_Range_Backplan_32Ch(Range* rangeAddress)
{
	int max, range;
	int value = 0x10;
	int outputValue;
	int i, j;
	
	max = rangeAddress->installedCh / 8;
	
	//1 all run signal on
	outb(rangeAddress->runAddr, rangeAddress->addr);
	for(i= 0;  i < max; i++){
		outputValue = 0x01;
		for( j = 0; j < 8; j++){
			outb((outputValue << j), rangeAddress->baseAddr + (i*2));
			usleep(500000);
		}
		outb(0x00, rangeAddress->baseAddr +i);
	}
	outb(rangeAddress->runAddr, rangeAddress->addr);

	
	//2.all range signal on
	for(range = 0; range < MAX_RANGE; range++){
		outb(value , rangeAddress->addr);
		if(rangeAddress->useFlag[range] == USE){
			for(i = 0; i < max; i++){
				for(j = 0; j < 8; j++){
					outb(outputValue << j , rangeAddress->baseAddr + (i*2));
					usleep(500000);
				}
				outb(0x00, rangeAddress->baseAddr + i);
			}
		}
		value = value << 1;
	}
	outb(0x00, rangeAddress->addr);

}
/*----------------------------------------------------------------------------- 	
	date : 2013							editor:pms
 
	discription:

	1 all run signal on
	2.all range signal on
-----------------------------------------------------------------------------*/ 
void Turn_On_Range_Reverse(Range* rangeAddress)
{
	int max, range;
	int value = 0x10;
	int outputValue;
	int i, j;
	
	max = rangeAddress->installedCh / 8;
	
	//1 all run signal on
	outb(rangeAddress->runAddr, rangeAddress->addr);
	for(i= max;  i >= 0;  i--){
		outputValue = 0x01;
		for( j = 7; j >= 0; j--){
			outb((outputValue << j), rangeAddress->baseAddr + i);
			usleep(500000);
		}
		outb(0x00, rangeAddress->baseAddr +i);
	}
	outb(rangeAddress->runAddr, rangeAddress->addr);

	
	//2.all range signal on
	for(range = 1; range <= MAX_RANGE; range++){
		outb(value , rangeAddress->addr);
		if(rangeAddress->useFlag[range] == USE){
			for(i = max; i >= 0; i--){
				for(j = 7; j >= 0; j--){
					outb(outputValue << j , rangeAddress->baseAddr + i);
					usleep(500000);
				}
				outb(0x00, rangeAddress->baseAddr + i);
			}
		}
		value = value << 1;
	}
	outb(0x00, rangeAddress->addr);

}
/*----------------------------------------------------------------------------- 	
	date : 2013							editor:pms
 
	discription:

	
	1 all run signal on
	2.all range signal on
-----------------------------------------------------------------------------*/ 
void Turn_On_Range_Reverse_Backplan_32Ch(Range* rangeAddress)
{
	int max, range;
	int value = 0x10;
	int outputValue;
	int i, j;
	
	max = rangeAddress->installedCh / 8;
	
	//1 all run signal on
	outb(rangeAddress->runAddr, rangeAddress->addr);
	for(i= max;  i >= 0;  i--){
		outputValue = 0x01;
		for( j = 7; j >= 0; j--){
			outb((outputValue << j), rangeAddress->baseAddr + i);
			usleep(500000);
		}
		outb(0x00, rangeAddress->baseAddr +i);
	}
	outb(rangeAddress->runAddr, rangeAddress->addr);

	
	//2.all range signal on
	for(range = 1; range <= MAX_RANGE; range++){
		outb(value , rangeAddress->addr);
		if(rangeAddress->useFlag[range] == USE){
			for(i = max; i >= 0; i--){
				for(j = 7; j >= 0; j--){
					outb(outputValue << j , rangeAddress->baseAddr + (i*2));
					usleep(500000);
				}
				outb(0x00, rangeAddress->baseAddr + (i*2));
			}
		}
		value = value << 1;
	}
	outb(0x00, rangeAddress->addr);

}

