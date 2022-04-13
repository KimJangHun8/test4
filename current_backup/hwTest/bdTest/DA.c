#include <asm/io.h>
#include "DA.h"


/*----------------------------------------------------------------------------- 	
	date: 2013. 8. 22										writer : pms
 	discription:
	
	ADR     80   |   40  |   20  |   10  |   8   |   4   |   2   |   1   |
	0x620	Ch8	 |   Ch7 |   Ch6 |	 Ch5 |	 Ch4 |	 Ch3 |	 Ch2 |   Ch1 |
	0x623					High   Value
	0x624  DA_EN8| DA_EN7|DA_EN6 |DA_EN5 |DA_EN4 |DA_EN3 |DA_EN2 |DA_EN1 |
	0x62f  		 |	 	 |		 |		 |		 | I_CS  | V_CS	 | 		 |
 
 	This is Address Setting for CPLD Type board
	you can use DAC 712 or 7741, depend on the dacType setting.
 
-----------------------------------------------------------------------------*/ 
void Setup_Main_Bd_DA_CPLD_Address(DaAddr* daAddressMap)
{
	daAddressMap->runAddr = 0x62f;
	daAddressMap->voltageAddr = 0x02;
	daAddressMap->currentAddr = 0x04;
	daAddressMap->daEnableAddr = 0x624;
	daAddressMap->daHighAddr = 0x623;
	daAddressMap->daLowAddr = 0x620;
	daAddressMap->bdType = CPLD_TYPE;
	daAddressMap->dacType = DAC_712;
}

/*----------------------------------------------------------------------------- 	
	date: 2013. 8. 22										writer : pms
 	discription:
	
	ADR     80   |   40  |   20  |   10  |   8   |   4   |   2   |   1   |
	0x620	Ch8	 |   Ch7 |   Ch6 |	 Ch5 |	 Ch4 |	 Ch3 |	 Ch2 |   Ch1 |
	0x623					High   Value
	0x624  DA_EN8| DA_EN7|DA_EN6 |DA_EN5 |DA_EN4 |DA_EN3 |DA_EN2 |DA_EN1 |
	0x63f  		 |	 	 |		 |		 |		 | I_CS  | V_CS	 | 		 
 
 	This is Address Setting for FPGA Type board
	you can use DAC 712 or 7741, depend on the dacType setting.
 
-----------------------------------------------------------------------------*/ 

void Setup_Main_Bd_DA_FPGA_Address(DaAddr* daAddressMap)
{
	daAddressMap->runAddr = 0x63f;
	daAddressMap->voltageAddr = 0x02;
	daAddressMap->currentAddr = 0x04;
	daAddressMap->daEnableAddr = 0x624;
	daAddressMap->daHighAddr = 0x623;
	daAddressMap->daLowAddr = 0x620;
	daAddressMap->bdType = FPGA_TYPE;
	daAddressMap->dacType = DAC_712;
}

/*----------------------------------------------------------------------------- 	
	date: 2013. 8. 22										writer : pms
 	discription:
	
	ADR     80   |   40  |   20  |   10  |   8   |   4   |   2   |   1   |
	0x620	Ch8	 |   Ch7 |   Ch6 |	 Ch5 |	 Ch4 |	 Ch3 |	 Ch2 |   Ch1 |
	0x623					High   Value
	0x624  DA_EN8| DA_EN7|DA_EN6 |DA_EN5 |DA_EN4 |DA_EN3 |DA_EN2 |DA_EN1 |
	0x63f  		 |	 	 |		 |		 |		 | I_CS  | V_CS	 | 		 
 
 	This is Address Setting for Backplan Type board
	you can use DAC 712 or 7741, depend on the dacType setting.
 

-----------------------------------------------------------------------------*/ 
void Setup_Backplan_Bd_DA_Address(DaAddr* daAddressMap)
{
	daAddressMap->runAddr = 0x63f;
	daAddressMap->voltageAddr = 0x02;
	daAddressMap->currentAddr = 0x04;
	daAddressMap->daEnableAddr = 0x624;
	daAddressMap->daHighAddr = 0x623;
	daAddressMap->daLowAddr = 0x620;
	daAddressMap->bdType = FPGA_TYPE;
	daAddressMap->dacType = DAC_7741;
}

/*----------------------------------------------------------------------------- 	
	date: 2013. 8. 22										writer : pms
 	discription:
	
	ADR     80   |   40  |   20  |   10  |   8   |   4   |   2   |   1   |
	0x620	Ch8	 |   Ch7 |   Ch6 |	 Ch5 |	 Ch4 |	 Ch3 |	 Ch2 |   Ch1 |
	0x623					High   Value
	0x624  DA_EN8| DA_EN7|DA_EN6 |DA_EN5 |DA_EN4 |DA_EN3 |DA_EN2 |DA_EN1 |
	0x62f  		 |	 	 |		 |		 |		 | I_CS  | V_CS	 | 		 
 
 	This is Address Setting for Mother Type board
	you can use DAC 712 or 7741, depend on the dacType setting.
 
-----------------------------------------------------------------------------*/ 

void Setyp_Mother_Bd_DA_Address(DaAddr* daAddressMap)
{
	daAddressMap->runAddr = 0x62f;
	daAddressMap->voltageAddr = 0x02;
	daAddressMap->currentAddr = 0x04;
	daAddressMap->daEnableAddr = 0x624;
	daAddressMap->daHighAddr = 0x623;
	daAddressMap->daLowAddr = 0x620;
	daAddressMap->bdType = CPLD_TYPE;
	daAddressMap->dacType = DAC_712;
}

/*----------------------------------------------------------------------------- 	
 	date: 2013. 8. 22										writer : pms

parameter
	daAddressMap 	: runAddress
	ch	 	: channel number ( 1 base)
	refVal	: output value

discription:
 	
		1. da bd enable
 		2. high value
 		3. V cs enable
 		4. low value
 		5. V cs disable
 		6. da bd disable
-----------------------------------------------------------------------------*/ 
void Output_Voltage_Channel(DaAddr* daAddressMap, int ch, float refVal)
{
	short int tmpVal;
	union DAData outputVal;
	int realCh;
	int daOutputVal;

	realCh = ch - 1;

	tmpVal = (short int)((29491.0 * refVal)/(9.0));
	if(daAddressMap->dacType == DAC_7741){
		outputVal.val = (short int)tmpVal ^ 0x8000;
	}else{
		outputVal.val = (short int)tmpVal;
	}
	daOutputVal = 0x01 << (realCh /8);
	if(daAddressMap->bdType == FPGA_TYPE){
		outb(~daOutputVal, daAddressMap->daEnableAddr);		//DA_BD_EN
	}else{
		outb(daOutputVal, daAddressMap->daEnableAddr);			
	}
	outb(outputVal.data[HIGH], daAddressMap->daHighAddr);	//REF_HIGH
	outb(daAddressMap->voltageAddr, daAddressMap->runAddr);	//V_CS_EN
	outb(outputVal.data[LOW], daAddressMap->daLowAddr + (realCh %8));//REF_LOW
	outb(0x00, daAddressMap->runAddr);								//V_CS_DIS
	if(daAddressMap->bdType == FPGA_TYPE){					//DA_BD_DIS
		outb( 0xff, daAddressMap->daEnableAddr);
	}else{
		outb( 0x00, daAddressMap->daEnableAddr);
	}
	
}
/*----------------------------------------------------------------------------- 	
 	date: 2013. 8. 22										writer : pms

parameter
	daAddressMap 	: runAddress
	ch	 	: channel number ( 1 base)
	refVal	: output value
   	
discription:
 		
		1. da bd enable
 		2. high value
 		3. I cs enable
 		4. low value
 		5. I cs disable
 		6. da bd disable

-----------------------------------------------------------------------------*/ 
void Output_Current_Channel(DaAddr* daAddressMap, int ch, float refVal)
{
	short int tmpVal;
	union DAData outputVal;
	int realCh;
	int daOutputVal;

	realCh = ch - 1;

	tmpVal = (short int)((29491.0 * refVal)/(9.0));
	if(daAddressMap->dacType == DAC_7741){
		outputVal.val = (short int)tmpVal ^ 0x8000;
	}else{
		outputVal.val = (short int)tmpVal;
	}
	daOutputVal = 0x01 << (realCh /8);
	if(daAddressMap->bdType == FPGA_TYPE){
		outb(~daOutputVal, daAddressMap->daEnableAddr);		//DA_BD_EN
	}else{
		outb(daOutputVal, daAddressMap->daEnableAddr);			
	}
	outb(outputVal.data[HIGH], daAddressMap->daHighAddr);	//REF_HIGH
	outb(daAddressMap->currentAddr, daAddressMap->runAddr);	//I_CS_EN
	outb(outputVal.data[LOW], daAddressMap->daLowAddr + (realCh %8));//REF_LOW
	outb(0x00, daAddressMap->runAddr);								//I_CS_DIS
	if(daAddressMap->bdType == FPGA_TYPE){					//DA_BD_DIS
		outb( 0xff, daAddressMap->daEnableAddr);
	}else{
		outb( 0x00, daAddressMap->daEnableAddr);
	}
}

/*----------------------------------------------------------------------------- 	
 	date: 2013. 8. 22										writer : pms

parameter
	daAddressMap 	: runAddress
	ch	 	: channel number ( 1 base)
	refVal	: output value

discription:
 	
		1. da bd enable
 		2. high value
 		3. V cs enable
 		4. low value
 		5. V cs disable
 		6. da bd disable
-----------------------------------------------------------------------------*/ 
void Output_Voltage_Channel_For_Backplan_32Ch(DaAddr* daAddressMap, int ch, float refVal)
{
	short int tmpVal;
	union DAData outputVal;
	int realCh;
	int daOutputVal;

	realCh = ch - 1;

	tmpVal = (short int)((29491.0 * refVal)/(9.0));
	if(daAddressMap->dacType == DAC_7741){
		outputVal.val = (short int)tmpVal ^ 0x8000;
	}else{
		outputVal.val = (short int)tmpVal;
	}
	daOutputVal = 0x01 << ((realCh /8) * 2);
	if(daAddressMap->bdType == FPGA_TYPE){
		outb(~daOutputVal, daAddressMap->daEnableAddr);		//DA_BD_EN
	}else{
		outb(daOutputVal, daAddressMap->daEnableAddr);			
	}
	outb(outputVal.data[HIGH], daAddressMap->daHighAddr);	//REF_HIGH
	outb(daAddressMap->voltageAddr, daAddressMap->runAddr);	//V_CS_EN
	outb(outputVal.data[LOW], daAddressMap->daLowAddr + (realCh %8));//REF_LOW
	outb(0x00, daAddressMap->runAddr);						//V_CS_DIS
	if(daAddressMap->bdType == FPGA_TYPE){					//DA_BD_DIS
		outb( 0xff, daAddressMap->daEnableAddr);
	}else{
		outb( 0x00, daAddressMap->daEnableAddr);
	}
	
}
/*----------------------------------------------------------------------------- 	
 	date: 2013. 8. 22										writer : pms

parameter
	daAddressMap 	: runAddress
	ch	 	: channel number ( 1 base)
	refVal	: output value

discription:
 	
		1. da bd enable
 		2. high value
 		3. V cs enable
 		4. low value
 		5. V cs disable
 		6. da bd disable
-----------------------------------------------------------------------------*/ 


void Output_Current_Channel_For_Backplan_32Ch(DaAddr* daAddressMap, int ch, float refVal)
{
	short int tmpVal;
	union DAData outputVal;
	int realCh;
	int daOutputVal;

	realCh = ch - 1;

	tmpVal = (short int)((29491.0 * refVal)/(9.0));
	if(daAddressMap->dacType == DAC_7741){
		outputVal.val = (short int)tmpVal ^ 0x8000;
	}else{
		outputVal.val = (short int)tmpVal;
	}
	daOutputVal = 0x01 << ((realCh /8)*2);
	if(daAddressMap->bdType == FPGA_TYPE){
		outb(~daOutputVal, daAddressMap->daEnableAddr);		//DA_BD_EN
	}else{
		outb(daOutputVal, daAddressMap->daEnableAddr);			
	}
	outb(outputVal.data[HIGH], daAddressMap->daHighAddr);	//REF_HIGH
	outb(daAddressMap->currentAddr, daAddressMap->runAddr);	//I_CS_EN
	outb(outputVal.data[LOW], daAddressMap->daLowAddr + (realCh %8));//REF_LOW
	outb(0x00, daAddressMap->runAddr);						//I_CS_DIS
	if(daAddressMap->bdType == FPGA_TYPE){					//DA_BD_DIS
		outb( 0xff, daAddressMap->daEnableAddr);
	}else{
		outb( 0x00, daAddressMap->daEnableAddr);
	}
}
/*----------------------------------------------------------------------------- 	
 	date: 2013. 8. 22										writer : pms

parameter
	daAddressMap 	: runAddress
	ch	 	: channel number ( 1 base)
	refVal	: output value

discription:
 	
		1. da bd enable
 		2. high value
 		3. V cs enable
 		4. low value
 		5. V cs disable
 		6. da bd disable
-----------------------------------------------------------------------------*/ 
void Output_Voltage_Channel_For_Backplan_64Ch(DaAddr* daAddressMap, int ch, float refVal)
{
	short int tmpVal;
	union DAData outputVal;
	int realCh;
	int daOutputVal;

	realCh = ch - 1;

	tmpVal = (short int)((29491.0 * refVal)/(9.0));
	if(daAddressMap->dacType == DAC_7741){
		outputVal.val = (short int)tmpVal ^ 0x8000;
	}else{
		outputVal.val = (short int)tmpVal;
	}
	daOutputVal = 0x01 << (realCh /8);
	if(daAddressMap->bdType == FPGA_TYPE){
		outb(~daOutputVal, daAddressMap->daEnableAddr);		//DA_BD_EN
	}else{
		outb(daOutputVal, daAddressMap->daEnableAddr);			
	}
	outb(outputVal.data[HIGH], daAddressMap->daHighAddr);	//REF_HIGH
	outb(daAddressMap->voltageAddr, daAddressMap->runAddr);	//V_CS_EN
	outb(outputVal.data[LOW], daAddressMap->daLowAddr + (realCh %8));//REF_LOW
	outb(0x00, daAddressMap->runAddr);						//V_CS_DIS
	if(daAddressMap->bdType == FPGA_TYPE){					//DA_BD_DIS
		outb( 0xff, daAddressMap->daEnableAddr);
	}else{
		outb( 0x00, daAddressMap->daEnableAddr);
	}
	
}
/*----------------------------------------------------------------------------- 	
 	date: 2013. 8. 22										writer : pms

parameter
	daAddressMap 	: runAddress
	ch	 	: channel number ( 1 base)
	refVal	: output value

discription:
 	
		1. da bd enable
 		2. high value
 		3. V cs enable
 		4. low value
 		5. V cs disable
 		6. da bd disable
-----------------------------------------------------------------------------*/ 
void Output_Current_Channel_For_Backplan_64Ch(DaAddr* daAddressMap, int ch, float refVal)
{
	short int tmpVal;
	union DAData outputVal;
	int realCh;
	int daOutputVal;

	realCh = ch - 1;

	tmpVal = (short int)((29491.0 * refVal)/(9.0));
	if(daAddressMap->dacType == DAC_7741){
		outputVal.val = (short int)tmpVal ^ 0x8000;
	}else{
		outputVal.val = (short int)tmpVal;
	}
	daOutputVal = 0x01 << (realCh /8);
	if(daAddressMap->bdType == FPGA_TYPE){
		outb(~daOutputVal, daAddressMap->daEnableAddr);		//DA_BD_EN
	}else{
		outb(daOutputVal, daAddressMap->daEnableAddr);			
	}
	outb(outputVal.data[HIGH], daAddressMap->daHighAddr);	//REF_HIGH
	outb(daAddressMap->currentAddr, daAddressMap->runAddr);	//I_CS_EN
	outb(outputVal.data[LOW], daAddressMap->daLowAddr + (realCh %8));//REF_LOW
	outb(0x00, daAddressMap->runAddr);						//I_CS_DIS
	if(daAddressMap->bdType == FPGA_TYPE){					//DA_BD_DIS
		outb( 0xff, daAddressMap->daEnableAddr);
	}else{
		outb( 0x00, daAddressMap->daEnableAddr);
	}
}

/*----------------------------------------------------------------------------- 	
 	date: 2013. 8. 22										writer : pms

parameter
	daAddressMap 	: runAddress
	ch	 	: channel number ( 1 base)
	refVal	: output value

discription:
 	it is for main bd, all channel voltage output by refVal	
-----------------------------------------------------------------------------*/ 

void Output_Voltage_All_Channel(DaAddr* daAddressMap, int installedCh, float refVal)
{
	int ch;

	for( ch = 1; ch <= installedCh; ch++){
		Output_Voltage_Channel(daAddressMap, ch, refVal);
	}
}
/*----------------------------------------------------------------------------- 	
 	date: 2013. 8. 22										writer : pms

parameter
	daAddressMap 	: runAddress
	ch	 	: channel number ( 1 base)
	refVal	: output value

discription:

 	it is for main bd, all channel current output by refVal	
-----------------------------------------------------------------------------*/ 

void Output_Current_All_Channel(DaAddr* daAddressMap, int installedCh, float refVal)
{
	int ch;

	for( ch = 1; ch <= installedCh; ch++){
		Output_Current_Channel(daAddressMap, ch, refVal);
	}
}
/*----------------------------------------------------------------------------- 	
 	date: 2013. 8. 22										writer : pms

parameter
	daAddressMap 	: runAddress
	ch	 	: channel number ( 1 base)
	refVal	: output value

discription:
 	it is for backplan 32ch  bd, all channel voltage output by refVal	
 -----------------------------------------------------------------------------*/ 


void Output_Voltage_All_Channel_For_Backplan_32Ch
(DaAddr* daAddressMap, int installedCh, float refVal)
{
	int ch;

	for( ch = 1; ch <= installedCh; ch++){
		Output_Voltage_Channel_For_Backplan_32Ch(daAddressMap, ch, refVal);
	}
}

/*----------------------------------------------------------------------------- 	
 	date: 2013. 8. 22										writer : pms

parameter
	daAddressMap 	: runAddress
	ch	 	: channel number ( 1 base)
	refVal	: output value

discription:
 	
 	it is for backplan 32ch  bd, all channel current output by refVal	
-----------------------------------------------------------------------------*/ 

void Output_Current_All_Channel_For_Backplan_32Ch
(DaAddr* daAddressMap, int installedCh, float refVal)
{
	int ch;

	for( ch = 1; ch <= installedCh; ch++){
		Output_Current_Channel_For_Backplan_32Ch(daAddressMap, ch, refVal);
	}
}

/*----------------------------------------------------------------------------- 	
 	date: 2013. 8. 22										writer : pms

parameter
	daAddressMap 	: runAddress
	ch	 	: channel number ( 1 base)
	refVal	: output value

discription:
 	
 	it is for backplan 64ch  bd, all channel voltage output by refVal	
-----------------------------------------------------------------------------*/ 

void Output_Voltage_All_Channel_For_Backplan_64Ch
(DaAddr* daAddressMap, int installedCh, float refVal)
{
	int ch;

	for( ch = 1; ch <= installedCh; ch++){
		Output_Voltage_Channel_For_Backplan_64Ch(daAddressMap, ch, refVal);
	}
}

/*----------------------------------------------------------------------------- 	
 	date: 2013. 8. 22										writer : pms

parameter
	daAddressMap 	: runAddress
	ch	 	: channel number ( 1 base)
	refVal	: output value

discription:
 	
 	it is for backplan 64ch  bd, all channel current output by refVal	
-----------------------------------------------------------------------------*/ 

void Output_Current_All_Channel_For_Backplan_64Ch
(DaAddr* daAddressMap, int installedCh, float refVal)
{
	int ch;

	for( ch = 1; ch <= installedCh; ch++){
		Output_Current_Channel_For_Backplan_64Ch(daAddressMap, ch, refVal);
	}
}

