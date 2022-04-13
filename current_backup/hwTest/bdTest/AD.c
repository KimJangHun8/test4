#include "AD.h"
#include <asm/io.h>


/*----------------------------------------------------------------------------- 	
	date: 2013. 8. 22										writer : pms
 	discription:

	 It's Address Setting,
	basically FPGA type board have two AD, CPLD type divided into two type, 
	one have two AD, other only one AD, so you should check AD count. 

	you can use DAC 712 or 7741, it's depend on the DAC Type setting.
	but backplan type board only use 7741 type DA.
 
-----------------------------------------------------------------------------*/ 
void Setup_Main_Bd_AD2_FPGA_Address(AdAddr* addressMap)
{	
	addressMap->baseAddr = 0x620;
	addressMap->adStart = 0x01;
	addressMap->voltageAddr = 0x04;
	addressMap->currentAddr = 0x01;
	addressMap->muxEnableAddr = 0x05;
	addressMap->muxAddr = 0x06;
	addressMap->chAddr = 0x07;
	addressMap->adCount = AD_NUM2;
	addressMap->bdType = FPGA_TYPE;
}
void Setup_Main_Bd_AD2_CPLD_Address(AdAddr* addressMap)
{
	addressMap->baseAddr = 0x620;
	addressMap->adStart = 0x01;
	addressMap->voltageAddr = 0x04;
	addressMap->currentAddr = 0x01;
	addressMap->muxEnableAddr = 0x05;
	addressMap->muxAddr = 0x06;
	addressMap->chAddr = 0x07;
	addressMap->adCount = AD_NUM2;
	addressMap->bdType = CPLD_TYPE;
}

void Setup_Main_Bd_AD1_CPLD_Address(AdAddr* addressMap)
{
	addressMap->baseAddr = 0x620;
	addressMap->adStart = 0x01;
	addressMap->voltageAddr = 0x01;
	addressMap->currentAddr = 0x01;
	addressMap->muxEnableAddr = 0x05;
	addressMap->muxAddr = 0x06;
	addressMap->chAddr = 0x07;
	addressMap->adCount = AD_NUM1;
	addressMap->bdType = CPLD_TYPE;
}

void Setup_Backplan_AD_Address(AdAddr* addressMap)
{
	addressMap->baseAddr = 0x620;
	addressMap->adStart = 0x01;
	addressMap->voltageAddr = 0x04;
	addressMap->currentAddr = 0x01;
	addressMap->muxEnableAddr = 0x07;
	addressMap->muxAddr = 0x06;
	addressMap->chAddr = 0x05;
	addressMap->adCount = AD_NUM2;
	addressMap->bdType = FPGA_TYPE;
}

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
void Set_Mux(AdAddr* addressMap, int type, int ch)
{
	int bd, tmp;
	int muxEnVal, muxAddrVal, chAddrVal;
	switch(type){
		case 0: //Channel Select
			bd = ch / 8;
			muxEnVal = 0x01;
			muxEnVal = muxEnVal << bd;
			muxAddrVal = 0x00;
			chAddrVal = (ch % 8) & 0x07;
			break;
		case 1://Ref Source Select
			muxEnVal = 0x30;
			ch = ( ch % 4) & 0x03;
			muxAddrVal = ch;
			tmp = ch << 2;
			muxAddrVal |= tmp;
			break;
		default: break;
	}
	//capacitor discharge
	outb(0x30, (addressMap->baseAddr + addressMap->muxEnableAddr));
	outb(0x3F, (addressMap->baseAddr + addressMap->muxAddr));
	outb(0x00, (addressMap->baseAddr + addressMap->chAddr));

	usleep(100);

	outb(muxEnVal, 	 (addressMap->baseAddr + addressMap->muxEnableAddr));
	outb(muxAddrVal, (addressMap->baseAddr + addressMap->muxAddr));
	outb(chAddrVal,  (addressMap->baseAddr + addressMap->chAddr));
}

void Set_Mux_AD1(AdAddr* addressMap, int type, int ch)
{
	int bd;
    unsigned char muxVal1=0x00, muxVal2=0x00, muxVal3=0x00;
	//bits-   7   |   6   |   5   |   4   |   3   |   2   |   1   |   0   |
	//mux1-   X   | M_EN3 | M_EN2 | M_EN1 | CH_EN4| CH_EN3| CH_EN2| CH_EN1|
	//mux2-   X   |   X   | M_A6  | M_A5  | M_A4  | M_A3  | M_A2  | M_A1  |
	//mux3-   X   |   X   |   X   |   X   |   X   | C_A3  | C_A2  | C_A1  |

		switch(type) {
		case 0: //V
			bd = ch / 8;
			muxVal1 = 0x01;
			muxVal1 = muxVal1 << bd;
			muxVal1 |= 0x40;
			muxVal2 = 0x00;//Voltage
			muxVal3 = (ch%8) & 0x07;//Channel Select
			break;
		case 1: //I
			bd = ch / 8;
			muxVal1 = 0x01;
			muxVal1 = muxVal1 << bd;
			muxVal1 |= 0x40;
			muxVal2 = 0x10;//Current
			muxVal3 = (ch%8) & 0x07;//Channel Select
			break;
		case 2: //sourceV
			muxVal1 = 0x70;
			ch = (ch % 4) & 0x03;
			muxVal2 = ch;
			break;
		case 3: //sourceI
			muxVal1 = 0x70;
			ch = (ch % 4) & 0x03;
			muxVal2 = ch << 2;
			muxVal2 |= 0x10;
			break;
		default: break;
	}

	//capacitor discharge
	outb(0x70, addressMap->baseAddr + addressMap->muxEnableAddr);
	outb(0x3F, addressMap->baseAddr + addressMap->muxAddr);
	outb(0x00, addressMap->baseAddr + addressMap->chAddr);
	
	usleep(100);
	

	outb(muxVal1, addressMap->baseAddr + addressMap->muxEnableAddr);
	outb(muxVal2, addressMap->baseAddr + addressMap->muxAddr);
	outb(muxVal3, addressMap->baseAddr + addressMap->chAddr);

}



void Set_Mux_Backplan_64Ch(AdAddr* addressMap, int type, int ch)
{
	int bd, tmp;
	int muxEnVal, chAddrVal, chEnVal;
	switch(type){
		//Ch
		case 0:
			bd = ch / 8;
			chEnVal = 0x01;
			chEnVal = chEnVal << bd;
			muxEnVal = 0x00;
			chAddrVal = (ch % 8) & 0x07;	//Ch Select
			break;
		//Source
		case 1:
			muxEnVal = 0x30;
			chAddrVal = 0x00;
			chEnVal = 0x00;
			ch = (ch % 4) & 0x03;
			muxEnVal |= ch;
			tmp = ch << 2;
			muxEnVal |= tmp;
			break;
		default: break;
	}
	//capacitor discharge
	outb(0x00, (addressMap->baseAddr +  addressMap->chAddr));	//Ch disable
	outb(0x0f, (addressMap->baseAddr + addressMap->muxEnableAddr));	//Mux address set
	outb(0x3f, (addressMap->baseAddr + addressMap->muxEnableAddr)); //Mux enable
	usleep(10);
	outb( 0x00, (addressMap->baseAddr + addressMap->muxEnableAddr));//Mux disable
	
	outb(chAddrVal, (addressMap->baseAddr + addressMap->muxAddr)); 	//Ch select
	outb(muxEnVal, (addressMap->baseAddr + addressMap->muxEnableAddr)); //Mux address set
	outb(chEnVal, (addressMap->baseAddr + addressMap->chAddr)); //ChMux enable
}

void Set_Mux_Backplan_32Ch(AdAddr* addressMap, int type, int ch)
{
	int bd, tmp;
	int muxEnVal, chAddrVal, chEnVal;
	switch(type){
		//Ch
		case 0:
			bd = (ch / 8) *2;
			chEnVal = 0x01;
			chEnVal = chEnVal << bd;
			muxEnVal = 0x00;
			chAddrVal = (ch % 8) & 0x07;	//Ch Select
			break;
		//Source
		case 1:
			muxEnVal = 0x30;
			chAddrVal = 0x00;
			chEnVal = 0x00;
			ch = (ch % 4) & 0x03;
			muxEnVal |= ch;
			tmp = ch << 2;
			muxEnVal |= tmp;
			break;
		default: break;
	}
	//capacitor discharge
	outb(0x00, (addressMap->baseAddr +  addressMap->chAddr));	//Ch disable
	outb(0x0f, (addressMap->baseAddr + addressMap->muxEnableAddr));	//Mux address set
	outb(0x3f, (addressMap->baseAddr + addressMap->muxEnableAddr)); //Mux enable
	usleep(10);
	outb( 0x00, (addressMap->baseAddr + addressMap->muxEnableAddr));//Mux disable
	
	outb(chAddrVal, (addressMap->baseAddr + addressMap->muxAddr)); 	//Ch select
	outb(muxEnVal, (addressMap->baseAddr + addressMap->muxEnableAddr)); //Mux address set
	outb(chEnVal, (addressMap->baseAddr + addressMap->chAddr)); //ChMux enable

}

void Read_32Ch_AD2_Data(AdAddr* addressMap, int installedCh, float* array)
{
	int ch;
	union ADData vVal, iVal;
	//Ch AD
	for( ch = 0; ch < installedCh; ch++){
		Set_Mux(addressMap, 0, ch);									//Ch select
		usleep(2000);
		outb(0x00, addressMap->baseAddr + addressMap->adStart);	//AD start
		usleep(2000);
		vVal.data[HIGH] = inb(addressMap->baseAddr + addressMap->voltageAddr);
		vVal.data[LOW] = inb(addressMap->baseAddr + addressMap->voltageAddr - 1);//Read V
		array[ch] = (float)vVal.val * 10/ 32767;

		iVal.data[HIGH] = inb(addressMap->baseAddr + addressMap->currentAddr);
		iVal.data[LOW] = inb(addressMap->baseAddr + addressMap->currentAddr - 1);//Read I
		array[ch + 32] = (float)iVal.val * 10 / 32767;
	}
	//Ref AD
	for( ch = 0; ch < 4; ch++){
		Set_Mux(addressMap, 1, ch);
		usleep(2000);
		outb(0x00, addressMap->baseAddr + addressMap->adStart);
		usleep(2000);
		vVal.data[HIGH] = inb(addressMap->baseAddr + addressMap->voltageAddr);
		usleep(2000);
		vVal.data[LOW] = inb(addressMap->baseAddr + addressMap->voltageAddr - 1);//Read Vref
		array[ch + 64] = (float)vVal.val * 10/ 32767;
		
		usleep(2000);
		iVal.data[HIGH] = inb(addressMap->baseAddr + addressMap->currentAddr);
		usleep(2000);
		iVal.data[LOW] = inb(addressMap->baseAddr + addressMap->currentAddr - 1);//Read Iref
		array[ch + 68] = (float)iVal.val * 10/ 32767;
	}
	

}
void Read_32Ch_AD1_Data(AdAddr* addressMap, int installedCh, float* array)
{
	int ch;
	union ADData vVal, iVal;
	//Ch AD
	for( ch = 0; ch < installedCh; ch++){
		Set_Mux_AD1(addressMap, 0, ch);									//Ch select
		usleep(2000);
		outb(0x00, addressMap->baseAddr + addressMap->adStart);	//AD start
		usleep(2000);
		vVal.data[HIGH] = inb(addressMap->baseAddr + addressMap->voltageAddr);
		vVal.data[LOW] = inb(addressMap->baseAddr + addressMap->voltageAddr - 1);//Read V
		array[ch] = (float)vVal.val * 10/ 32767;
	}
	for(ch = 0; ch < installedCh; ch++){
		Set_Mux_AD1(addressMap, 1, ch);
		usleep(2000);
		outb(0x00, addressMap->baseAddr + addressMap->adStart);	//AD start
		iVal.data[HIGH] = inb(addressMap->baseAddr + addressMap->currentAddr);
		iVal.data[LOW] = inb(addressMap->baseAddr + addressMap->currentAddr - 1);//Read I
		array[ch + 32] = (float)iVal.val * 10 / 32767;
	}
	//Ref AD
	for( ch = 0; ch < 4; ch++){
		Set_Mux_AD1(addressMap, 2, ch);
		usleep(2000);
		outb(0x00, addressMap->baseAddr + addressMap->adStart);
		usleep(2000);
		vVal.data[HIGH] = inb(addressMap->baseAddr + addressMap->voltageAddr);
		usleep(2000);
		vVal.data[LOW] = inb(addressMap->baseAddr + addressMap->voltageAddr - 1);//Read Vref
		array[ch + 64] = (float)vVal.val * 10/ 32767;
	}
	for( ch = 0; ch < 4; ch++){
		Set_Mux_AD1(addressMap, 3, ch);	
		outb(0x00, addressMap->baseAddr + addressMap->adStart);
		usleep(2000);
		iVal.data[HIGH] = inb(addressMap->baseAddr + addressMap->currentAddr);
		usleep(2000);
		iVal.data[LOW] = inb(addressMap->baseAddr + addressMap->currentAddr - 1);//Read Iref
		array[ch + 68] = (float)iVal.val * 10/ 32767;
	}
	
}
void Read_Backplan_32Ch_AD2_Data(AdAddr* addressMap, int installedCh, float* array)
{
	int ch;
	union ADData vVal, iVal;
	//Ch AD
	for( ch = 0; ch < installedCh; ch++){
		Set_Mux_Backplan_32Ch(addressMap, 0, ch);									//Ch select
		usleep(2000);
		outb(0x00, addressMap->baseAddr + addressMap->adStart);	//AD start
		usleep(2000);
		vVal.data[HIGH] = inb(addressMap->baseAddr + addressMap->voltageAddr);
		vVal.data[LOW] = inb(addressMap->baseAddr + addressMap->voltageAddr - 1);//Read V
		array[ch] = (float)vVal.val * 10/ 32767;

		iVal.data[HIGH] = inb(addressMap->baseAddr + addressMap->currentAddr);
		iVal.data[LOW] = inb(addressMap->baseAddr + addressMap->currentAddr - 1);//Read I
		array[ch + 32] = (float)iVal.val * 10 / 32767;
	}
	//Ref AD
	for( ch = 0; ch < 4; ch++){
		Set_Mux_Backplan_32Ch(addressMap, 1, ch);
		usleep(2000);
		outb(0x00, addressMap->baseAddr + addressMap->adStart);
		usleep(2000);
		vVal.data[HIGH] = inb(addressMap->baseAddr + addressMap->voltageAddr);
		usleep(2000);
		vVal.data[LOW] = inb(addressMap->baseAddr + addressMap->voltageAddr - 1);//Read Vref
		array[ch + 64] = (float)vVal.val * 10/ 32767;
		
		usleep(2000);
		iVal.data[HIGH] = inb(addressMap->baseAddr + addressMap->currentAddr);
		usleep(2000);
		iVal.data[LOW] = inb(addressMap->baseAddr + addressMap->currentAddr - 1);//Read Iref
		array[ch + 68] = (float)iVal.val * 10/ 32767;
	}
	

}
void Read_Backplan_64Ch_AD2_Data(AdAddr* addressMap, int installedCh, float* array)
{
	int ch;
	union ADData vVal, iVal;
	//Ch AD
	for( ch = 0; ch < installedCh; ch++){
		Set_Mux_Backplan_64Ch(addressMap, 0, ch);									//Ch select
		usleep(2000);
		outb(0x00, addressMap->baseAddr + addressMap->adStart);	//AD start
		usleep(2000);
		vVal.data[HIGH] = inb(addressMap->baseAddr + addressMap->voltageAddr);
		vVal.data[LOW] = inb(addressMap->baseAddr + addressMap->voltageAddr - 1);//Read V
		array[ch] = (float)vVal.val * 10/ 32767;

		iVal.data[HIGH] = inb(addressMap->baseAddr + addressMap->currentAddr);
		iVal.data[LOW] = inb(addressMap->baseAddr + addressMap->currentAddr - 1);//Read I
		array[ch+64] = (float)iVal.val * 10 / 32767;
	}
	//Ref AD
	for( ch = 0; ch < 4; ch++){
		Set_Mux_Backplan_64Ch(addressMap, 1, ch);
		usleep(2000);
		outb(0x00, addressMap->baseAddr + addressMap->adStart);
		usleep(2000);
		vVal.data[HIGH] = inb(addressMap->baseAddr + addressMap->voltageAddr);
		usleep(2000);
		vVal.data[LOW] = inb(addressMap->baseAddr + addressMap->voltageAddr - 1);//Read Vref
		array[ch + 128] = (float)vVal.val * 10/ 32767;
		
		usleep(2000);
		iVal.data[HIGH] = inb(addressMap->baseAddr + addressMap->currentAddr);
		usleep(2000);
		iVal.data[LOW] = inb(addressMap->baseAddr + addressMap->currentAddr - 1);//Read Iref
		array[ch + 132] = (float)iVal.val * 10/ 32767;
	}
	

}
