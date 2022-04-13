#include <asm/io.h>
#include <rtl_core.h>
#include <pthread.h>
#include <math.h>
#include "../../INC/datastore.h"
#include "DAQ.h"
#include "message.h"

#if CYCLER_TYPE == 1	//switching
	#define DAQ_ADDR 0x630
//#elif CYCLER_TYPE == 0	//linear
#else 	//linear, CRC
	#define DAQ_ADDR 0x7A0
#endif

extern S_SYSTEM_DATA    *myData;

/*date:2010.12.19
  editor:pjy
  description : commend to DAQ Bd, and then 
  DAQ start 64ch ad, it will take 30mS.*/
void DAQ_Start()
{
	if(myData->mData.config.installedAuxV == 0) return;
	if(myData->daq.misc.daq_read_delay != 0) return;
	//(PNE201004 DAQ B/D REV 07)
//	if(myData->daq.misc.daqCount == 0)
//	{
		outb(0x00, DAQ_ADDR);
//	}	
}
/*date:2010.12.19			 editor:pjy
  description : commend to DAQ Bd, and then 
  DAQ start 64ch ad, it will take 30mS.*/
void DAQ_Read(int state)
{
	if(myData->mData.config.installedAuxV == 0) return;

	if(state == 0){
		myData->daq.misc.daq_read_delay
		 += myData->mData.misc.rt_scan_time;
	}
//	if(myData->daq.misc.daq_read_delay < 20) return;
	//	20180831 sch modify for delay
	if(myData->daq.misc.daq_read_delay < 5) return;

	if(state == 0){
		data_read(); 
	}else if(state == 1){
		aCalDaqCh();
		aCalDaqAdAverage(0);
	}else if(state == 2){
		aCalDaqAdAverage(1);
	}else if(state == 3){
		aCalDaqAdAverage(2);
#if CH_AUX_DATA == 1
		//190807
		aDaq_ChData_Mapping();
#endif
		myData->daq.misc.daq_read_delay = 0;

	}
}
void data_read() 
{
	//(PNE201004 DAQ B/D REV 07)
	U_ADDA ADValue;				//value buffer
	int maxAdNumber = 4;		//ADC Number
	int maxMuxNumber = 4;		//MUX Number
	int maxChNumber = 8;		//Ch Number
	int adCount, muxCount, chCount;
	int count;//test code
	int tmp, ad_count;
	unsigned char readData[360];
	char *tmp1;
	ad_count = myData->daq.misc.ad_count;
	
	//for(count = 0; count < 500; count++){
	for(count = 0; count < 360; count++){	//21116_hun
		readData[count] = (unsigned char)inb(DAQ_ADDR);
	}
	tmp1 = &readData[0];
	//1. 1th 3th data don't use
	for(count = 0; count < 3; count++){
		ADValue.byte[0] = *tmp1++;
		ADValue.byte[1] = *tmp1++;		
	}	
	//2. get +5V value (4th ~ 20th)
	for(muxCount = 0; muxCount < maxMuxNumber; muxCount++){
		for(adCount = 0; adCount < maxAdNumber; adCount++){
			ADValue.byte[0] = *tmp1++;		
			ADValue.byte[1] = *tmp1++;		
	//		if(adCount < 2) {
				tmp = (adCount * 4) + muxCount;
				myData->daq.data[ad_count].ad_ref[tmp][0] = ADValue.val;
	//		}
		}
	}
	
	//3. get -5V value (21th ~ 37th)
	for(muxCount = 0; muxCount < maxMuxNumber; muxCount++){
		for(adCount = 0; adCount < maxAdNumber; adCount++){
			ADValue.byte[0] = *tmp1++;
			ADValue.byte[1] = *tmp1++;
	//		if(adCount < 2) {
				tmp = (adCount * 4) + muxCount;
				myData->daq.data[ad_count].ad_ref[tmp][1] = ADValue.val;
	//		}
		}
	}

	//4. get GND value (38th ~ 52th)
	for(muxCount = 0; muxCount < maxMuxNumber; muxCount++){
		for(adCount = 0; adCount < maxAdNumber; adCount++){
			ADValue.byte[0] = *tmp1++;
			ADValue.byte[1] = *tmp1++;
	//		if(adCount < 2) {
				tmp = (adCount * 4) + muxCount;
				myData->daq.data[ad_count].ad_ref[tmp][2] = ADValue.val;
	//		}
		}
	}

	//5. get Ch value( 53th ~ 179th)
	for(chCount = 0; chCount < maxChNumber; chCount++){
		for(muxCount = 0; muxCount < maxMuxNumber; muxCount++){
			for(adCount = 0; adCount < maxAdNumber; adCount++){
				ADValue.byte[0] = *tmp1++;
				ADValue.byte[1] = *tmp1++;
	//			if(adCount < 2) {
					tmp = (adCount * 32) + (muxCount * 8) + chCount;
					myData->daq.data[ad_count].ad_ch[tmp] = ADValue.val;
	//			}
			}
		}
	}
	//6. 180th don't use.
	ADValue.byte[0] = *tmp1++;
	ADValue.byte[1] = *tmp1++;
/*
	aCalDaqAdAverage();
	aCalDaqCh();
	myData->daq.misc.ad_count++;
	if(myData->daq.misc.ad_count == MAX_DAQ_AD_COUNT)
	{
		myData->daq.misc.ad_count = 0;
	}
*/		
}

//20180831 sch modify
void aCalDaqAdAverage(int state)
{
	int i,j,k,x,y;
	double avr;
	double sum, list[MAX_DAQ_AD_COUNT], list_ref[MAX_DAQ_AD_COUNT];

	if(state == 0){	
		for(i = 0; i < 64 ; i++)
		{
			sum = 0.0;
			for(j = 0; j < MAX_DAQ_AD_COUNT; j++)
			{
				list[j] = myData->daq.data[j].ad_ch[i];
				sum += myData->daq.data[j].ad_ch[i];
			}
			for(x = 0; x < MAX_DAQ_AD_COUNT-1; x++)
			{
				for(y = x+1; y < MAX_DAQ_AD_COUNT; y++)
				{
					if(list[y] < list[x]){
						SWAP_DAQ(&list[x], &list[y]);
					}
				}
			}
	
			avr = ((sum - list[0] - list[1] - list[3] 
				- list[MAX_DAQ_AD_COUNT-1] -  list[MAX_DAQ_AD_COUNT-2] 
				- list[MAX_DAQ_AD_COUNT-3]) / (double)(MAX_DAQ_AD_COUNT - 6));
			myData->daq.misc.ch_ad_avr[i] = (long)avr;
		}
	}else if(state == 1){
		for(i = 64 ; i < MAX_AUX_VOLT_DATA; i++)
		{
			sum = 0.0;
			for(j = 0; j < MAX_DAQ_AD_COUNT; j++)
			{
				list[j] = myData->daq.data[j].ad_ch[i];
				sum += myData->daq.data[j].ad_ch[i];
			}
			for(x = 0; x < MAX_DAQ_AD_COUNT-1; x++)
			{
				for(y = x+1; y < MAX_DAQ_AD_COUNT; y++)
				{
					if(list[y] < list[x]){
						SWAP_DAQ(&list[x], &list[y]);
					}
				}
			}
	
			avr = ((sum - list[0] - list[1] - list[3] 
				- list[MAX_DAQ_AD_COUNT-1] -  list[MAX_DAQ_AD_COUNT-2] 
				- list[MAX_DAQ_AD_COUNT-3]) / (double)(MAX_DAQ_AD_COUNT - 6));
			myData->daq.misc.ch_ad_avr[i] = (long)avr;
		}
	}else if(state == 2){
		for(k=0;k<16;k++)
		{
			for(i=0;i<3;i++)
			{
				sum = 0.0;
				for(j=0;j<MAX_DAQ_AD_COUNT;j++)
				{
					list_ref[j] = myData->daq.data[j].ad_ref[k][i];
					sum += myData->daq.data[j].ad_ref[k][i];
				}
				for(x = 0; x < MAX_DAQ_AD_COUNT-1; x++)
				{
					for(y = x+1; y < MAX_DAQ_AD_COUNT; y++)
					{
						if(list[y] < list[x]){
							SWAP_DAQ(&list[x], &list[y]);
						}
					}
				}
				avr = ((sum - list_ref[0] - list_ref[1] - list_ref[2] 
					- list_ref[MAX_DAQ_AD_COUNT-1] - list_ref[MAX_DAQ_AD_COUNT-2]
					- list_ref[MAX_DAQ_AD_COUNT-3]) / (double)(MAX_DAQ_AD_COUNT-6));
				myData->daq.misc.ref_ad_avr[k][i] = (long)avr;
			}
		}
	}
}

void aDaq_ChData_Mapping()
{	//190807
	int i, j, k;
	unsigned char bd, ch, checkNum, startNum, AuxVCnt;
	unsigned short installedAuxT, installedAuxV;

	installedAuxT = myData->mData.config.installedTemp;
	installedAuxV = myData->mData.config.installedAuxV;

	startNum = checkNum = AuxVCnt = 0;
	for(i=0 ; i < myData->mData.config.installedCh; i++){
		bd = myData->CellArray1[i].bd;
		ch = myData->CellArray1[i].ch;
		myData->bData[bd].cData[ch].misc.chAuxVCnt = 0;
		myData->bData[bd].cData[ch].misc.chAuxVCheckNum = 0;
		for(j = 0; j < installedAuxV; j++){
			if(myData->auxSetData[j + installedAuxT].auxType == P1
				&& myData->auxSetData[j + installedAuxT].chNo == ch + 1){
				myData->daq.op.chData_AuxV[checkNum] 
										= myData->daq.op.ch_vsens[j];
				checkNum++;
				myData->bData[bd].cData[ch].misc.chAuxVCnt++;
				myData->bData[bd].cData[ch].misc.chAuxVCheckNum = checkNum;
				continue;
			}
		}	
		
		for(j = checkNum; j < installedAuxV; j++){
			myData->daq.op.chData_AuxV[j] = 0;
		}
						
		AuxVCnt = myData->bData[bd].cData[ch].misc.chAuxVCnt;
		startNum = myData->bData[bd].cData[ch].misc.chAuxVCheckNum - AuxVCnt;
		
		if(AuxVCnt > MAX_CH_AUX_DATA)	AuxVCnt = MAX_CH_AUX_DATA;
		
		for(k = 0; k < MAX_CH_AUX_DATA; k++){
			if(k < AuxVCnt){
				myData->bData[bd].cData[ch].misc.chAuxVoltage[k] 
					= myData->daq.op.chData_AuxV[k + startNum];
			}else{
				myData->bData[bd].cData[ch].misc.chAuxVoltage[k] = 0;	
			}
		}
	}
}
/*
void aCalDaqAdAverage()
{
	int i,j,k;
	double avr;
	double sum,max,min;
	for(i=0;i<MAX_AUX_VOLT_DATA;i++)
	{
		sum = 0.0;
		max = myData->daq.data[0].ad_ch[i];
		min = myData->daq.data[0].ad_ch[i];
		for(j=0;j<MAX_DAQ_AD_COUNT;j++)
		{
			sum += myData->daq.data[j].ad_ch[i];
			if(myData->daq.data[j].ad_ch[i] > max)
				max = myData->daq.data[j].ad_ch[i];
			if(myData->daq.data[j].ad_ch[i] < min)
				min = myData->daq.data[j].ad_ch[i];
		}
		avr = ((sum - max - min) / (double)(MAX_DAQ_AD_COUNT - 2));
		myData->daq.misc.ch_ad_avr[i] = (long)avr;
	}
	for(k=0;k<16;k++)
	{
		for(i=0;i<3;i++)
		{
			sum = 0.0;
			max = myData->daq.data[0].ad_ref[k][i];
			min = myData->daq.data[0].ad_ref[k][i];
			for(j=0;j<MAX_DAQ_AD_COUNT;j++)
			{
				sum += myData->daq.data[j].ad_ref[k][i];
				if(myData->daq.data[j].ad_ref[k][i] > max )
					max = myData->daq.data[j].ad_ref[k][i];
				if(myData->daq.data[j].ad_ref[k][i] < min )
					min = myData->daq.data[j].ad_ref[k][i];
			}
			avr = ((sum - max - min) / (double)(MAX_DAQ_AD_COUNT - 2));
			myData->daq.misc.ref_ad_avr[k][i] = (long)avr;
		}
	}
}
*/

void aCalDaqCh()
{
	short int ad_5v,ad_5vN,ad_0v;
	int i,j,val1;
	float AD_A,AD_B,AD_A_N,AD_B_N;
	double tmp;
	switch(myData->mData.config.hwSpec) {
		case L_30V_20A_R1_AD2:
		case L_20V_300A_R2:
		case L_40V_300A_R2:
			val1 = 5000000;
			break;
		case L_30V_5A_R1_AD2:
			val1 = 25000000;
			break;
		case L_30V_40A_R2:
		case L_30V_40A_R2_OT_20:
		case L_30V_40A_R2_P_AD2:
			val1 = 5000000;
			break;
		default:
			val1 = myData->mData.config.maxVoltage[0];
			break;
	}	
	
	for(j = 0;j<16;j++) {
		ad_5v = myData->daq.misc.ref_ad_avr[j][0] ;
		ad_5vN = myData->daq.misc.ref_ad_avr[j][1] ;
		ad_0v = myData->daq.misc.ref_ad_avr[j][2] ;

		AD_A = (float)(val1) / (float)(ad_5v - ad_0v);
		AD_B = val1 - (float)(ad_5v * AD_A);

		AD_A_N = (float)(0.0 - (-val1)) / (float)(ad_0v - ad_5vN);
		AD_B_N = (float)(0.0) - (float)(ad_0v * AD_A_N);
		
		myData->daq.misc.DAQ_AD_A[j] = AD_A;
		myData->daq.misc.DAQ_AD_B[j] = AD_B;
		myData->daq.misc.DAQ_AD_A_N[j] = AD_A_N;
		myData->daq.misc.DAQ_AD_B_N[j] = AD_B_N;
		for(i=0;i<8;i++)
		{	
			tmp = (double)(myData->daq.misc.ch_ad_avr[(j*8)+i]);
			if(tmp > 0)
			{	
				myData->daq.org.ch_vsens[(j*8)+i] = (long)(tmp * AD_A + AD_B);
			} else {
				myData->daq.org.ch_vsens[(j*8)+i] = 
						(long)(tmp * AD_A_N + AD_B_N);
			}
			tmp = (myData->daq.org.ch_vsens[(j*8)+i] *
				   	myData->daq.cali.AD_A[(j*8)+i]) + 
					myData->daq.cali.AD_B[(j*8)+i];
			if(myData->AppControl.config.debugType != P0) {
				myData->daq.op.ch_vsens[(j*8)+i] = i * 10000;
			}else{
				myData->daq.op.ch_vsens[(j*8)+i] = (long)tmp;
			}
		}
	}
	myData->daq.misc.ad_count++;
	if(myData->daq.misc.ad_count == MAX_DAQ_AD_COUNT)
	{
		myData->daq.misc.ad_count = 0;
	}
}

void  DAQ_Cali_Read(int no, int val , int flag)
{
	int i;
	if(no == 0) {
		if(flag == 1) {
			for(i=0;i<MAX_AUX_VOLT_DATA;i++)
			{
				myData->daq.tmp_low.ch_vsens[i] = myData->daq.org.ch_vsens[i];
				myData->daq.misc.low_point[i] = val;
				myData->daq.misc.caliFlag[i] |= 0x01;
			}
			send_msg(MODULE_TO_APP,MSG_MODULE_APP_AUX_CALI,1,0);
		} else if(flag == 2)
		{
			for(i=0;i<MAX_AUX_VOLT_DATA;i++)
			{
				myData->daq.tmp_high.ch_vsens[i] = myData->daq.org.ch_vsens[i];
				myData->daq.misc.high_point[i] = val;
				myData->daq.misc.caliFlag[i] |= 0x02;
			}
			send_msg(MODULE_TO_APP,MSG_MODULE_APP_AUX_CALI,2,0);
		}
	} else {
		if(flag == 1) {
			for(i=(no-1)*16;i<(no * 16);i++)
			{	
				myData->daq.tmp_low.ch_vsens[i] = myData->daq.org.ch_vsens[i];
				myData->daq.misc.low_point[i] = val;
				myData->daq.misc.caliFlag[i] |= 0x01;
			}
			send_msg(MODULE_TO_APP,MSG_MODULE_APP_AUX_CALI,1,no);
		} else if(flag == 2)
		{
			for(i=(no-1)*16;i<(no * 16);i++)
			{
				myData->daq.tmp_high.ch_vsens[i] = myData->daq.org.ch_vsens[i];
				myData->daq.misc.high_point[i] = val;
				myData->daq.misc.caliFlag[i] |= 0x02;
			}
			send_msg(MODULE_TO_APP,MSG_MODULE_APP_AUX_CALI,2,no);
		}

	}
}

void DAQ_Cali_Update() 
{
	int i;
	float gain,offset;
	long x1,x2,y1,y2;
	
	for(i=0;i<MAX_AUX_VOLT_DATA;i++)
	{	
		if(myData->daq.misc.caliFlag[i] && 0x03 == 0x03) {
			y1 = myData->daq.misc.low_point[i];
			y2 = myData->daq.misc.high_point[i];
			x1 = myData->daq.tmp_low.ch_vsens[i];
			x2 = myData->daq.tmp_high.ch_vsens[i];
			gain = ((float)(y2-y1))/((float)(x2-x1));
			offset = y2 - (x2 * gain);
			myData->daq.tmpCali.AD_A[i] = gain;
			myData->daq.tmpCali.AD_B[i] = offset;
			myData->daq.cali.AD_A[i] = myData->daq.tmpCali.AD_A[i];
			myData->daq.cali.AD_B[i] = myData->daq.tmpCali.AD_B[i];
		}
	}
	send_msg(MODULE_TO_APP,MSG_MODULE_APP_AUX_CALI,3,0);
}

void SWAP_DAQ(double *x, double *y)
{
	double tmp;

	tmp = *x;
	*x = *y;
	*y = tmp;
}
