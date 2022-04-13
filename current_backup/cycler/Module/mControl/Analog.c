#include <asm/io.h>
#include <rtl_core.h>
#include <pthread.h>
#include <time.h>
#include <math.h>
#include "../../INC/datastore.h"
#include "local_utils.h"
#include "Analog.h"
#include "ChannelControl.h"

extern S_SYSTEM_DATA *myData;
extern S_MODULE_DATA *myPs;
extern S_CH_DATA *myCh;

int shiftTable_100mS[72][2] = {
	{ 0, 0}, { 1, 0}, { 0, 1}, { 1, 1}, { 0, 2}, { 1, 2}, { 0, 3}, { 1, 3},
	{ 0, 4}, { 1, 4}, { 0, 5}, { 1, 5}, { 0, 6}, { 1, 6}, { 0, 7}, { 1, 7},
	{ 0, 8}, { 1, 8}, { 0, 9}, { 1, 9}, { 0,10}, { 1,10}, { 0,11}, { 1,11},
	{ 0,12}, { 1,12}, { 0,13}, { 1,13}, { 0,14}, { 1,14}, { 0,15}, { 1,15},
	{ 0,16}, { 1,16}, { 0,17}, { 1,17}, { 0,18}, { 1,18}, { 0,19}, { 1,19},
	{ 0,20}, { 1,20}, { 0,21}, { 1,21}, { 0,22}, { 1,22}, { 0,23}, { 1,23},
	{ 0,24}, { 1,24}, { 0,25}, { 1,25}, { 0,26}, { 1,26}, { 0,27}, { 1,27},
	{ 0,28}, { 1,28}, { 0,29}, { 1,29}, { 0,30}, { 1,30}, { 0,31}, { 1,31},
	{ 2, 0}, { 3, 0}, { 2, 1}, { 3, 1}, { 2, 2}, { 3, 2}, { 2, 3}, { 3, 3}
}; //for 100mS

int shiftTable_50mS[40][2] = {
	{ 0, 0}, { 1, 0}, { 0, 1}, { 1, 1}, { 0, 2}, { 1, 2}, { 0, 3}, { 1, 3},
	{ 0, 4}, { 1, 4}, { 0, 5}, { 1, 5}, { 0, 6}, { 1, 6}, { 0, 7}, { 1, 7},
	{ 0, 8}, { 1, 8}, { 0, 9}, { 1, 9}, { 0,10}, { 1,10}, { 0,11}, { 1,11},
	{ 0,12}, { 1,12}, { 0,13}, { 1,13}, { 0,14}, { 1,14}, { 0,15}, { 1,15},
	{ 2, 0}, { 3, 0}, { 2, 1}, { 3, 1}, { 2, 2}, { 3, 2}, { 2, 3}, { 3, 3}
}; //for 50mS

int shiftTable_100mS_AD2[68][2] = {
	{ 0, 0}, { 0, 1}, { 0, 2}, { 0, 3}, { 0, 4}, { 0, 5}, { 0, 6}, { 0, 7},
	{ 0, 8}, { 0, 9}, { 0,10}, { 0,11}, { 0,12}, { 0,13}, { 0,14}, { 0,15},
	{ 0,16}, { 0,17}, { 0,18}, { 0,19}, { 0,20}, { 0,21}, { 0,22}, { 0,23},
	{ 0,24}, { 0,25}, { 0,26}, { 0,27}, { 0,28}, { 0,29}, { 0,30}, { 0,31},
	{ 0,32}, { 0,33}, { 0,34}, { 0,35}, { 0,36}, { 0,37}, { 0,38}, { 0,39},
	{ 0,40}, { 0,41}, { 0,42}, { 0,43}, { 0,44}, { 0,45}, { 0,46}, { 0,47},
	{ 0,48}, { 0,49}, { 0,50}, { 0,51}, { 0,52}, { 0,53}, { 0,54}, { 0,55},
	{ 0,56}, { 0,57}, { 0,58}, { 0,59}, { 0,60}, { 0,61}, { 0,62}, { 0,63},
	{ 1, 0}, { 1, 1}, { 1, 2}, { 1, 3}
}; //for 100mS

int shiftTable_50mS_AD2[36][2] = {
	{ 0, 0}, { 0, 1}, { 0, 2}, { 0, 3}, { 0, 4}, { 0, 5}, { 0, 6}, { 0, 7},
	{ 0, 8}, { 0, 9}, { 0,10}, { 0,11}, { 0,12}, { 0,13}, { 0,14}, { 0,15},
	{ 0,16}, { 0,17}, { 0,18}, { 0,19}, { 0,20}, { 0,21}, { 0,22}, { 0,23},
	{ 0,24}, { 0,25}, { 0,26}, { 0,27}, { 0,28}, { 0,29}, { 0,30}, { 0,31},
	{ 1, 0}, { 1, 1}, { 1, 2}, { 1, 3}
}; //for 50mS

//210510 add for 500mA/2uA Type Cycler
int shiftTable_50mS_AD2_500mA[36][2] = {
	{ 0, 0}, { 0, 1}, { 0, 2}, { 0, 3}, { 0, 4}, { 0, 5}, { 0, 6}, { 0, 7},
	{ 0, 8}, { 0, 9}, { 0,10}, { 0,11}, { 0,12}, { 0,13}, { 0,14}, { 0,15},
	{ 0,32}, { 0,33}, { 0,34}, { 0,35}, { 0,36}, { 0,37}, { 0,38}, { 0,39},
	{ 0,40}, { 0,41}, { 0,42}, { 0,43}, { 0,44}, { 0,45}, { 0,46}, { 0,47},
	{ 1, 0}, { 1, 1}, { 1, 2}, { 1, 3}
}; //for 50mS

int shiftTable_50mS_AD2_2uA[36][2] = {
	{ 0,16}, { 0,17}, { 0,18}, { 0,19}, { 0,20}, { 0,21}, { 0,22}, { 0,23},
	{ 0,24}, { 0,25}, { 0,26}, { 0,27}, { 0,28}, { 0,29}, { 0,30}, { 0,31},
	{ 0,48}, { 0,49}, { 0,50}, { 0,51}, { 0,52}, { 0,53}, { 0,54}, { 0,55},
	{ 0,56}, { 0,57}, { 0,58}, { 0,59}, { 0,60}, { 0,61}, { 0,62}, { 0,63},
	{ 1, 0}, { 1, 1}, { 1, 2}, { 1, 3}
}; //for 50mS

int shiftTable_25mS_AD2[20][2] = {
	{ 0, 0}, { 0, 1}, { 0, 2}, { 0, 3}, { 0, 4}, { 0, 5}, { 0, 6}, { 0, 7},
	{ 0, 8}, { 0, 9}, { 0,10}, { 0,11}, { 0,12}, { 0,13}, { 0,14}, { 0,15},
	{ 1, 0}, { 1, 1}, { 1, 2}, { 1, 3}
}; //for 25mS

int shiftTable_20mS_AD2[12][2] = {
	{ 0, 0}, { 0, 1}, { 0, 2}, { 0, 3}, { 0, 4}, { 0, 5}, { 0, 6}, { 0, 7},
	{ 1, 0}, { 1, 1}, { 1, 2}, { 1, 3}
}; //for 20mS

int shiftTable_20mS_AD2_FAD[11][2] = {
	{ 0, 0}, { 0, 1}, { 0, 2}, { 0, 3}, { 0, 4}, { 0, 5}, { 0, 6}, { 0, 7},
	{ 1, 0}, { 1, 1}, { 1, 2}
}; //for 20mS_FAD

int shiftTable_10mS_AD2[10][2] = { //140520 oys w : rt_scan_time_10mS
	{ 0, 0}, { 0, 1}, { 0, 2}, { 0, 3}, { 0, 4}, { 0, 5}, { 0, 6}, { 0, 7},
	{ 1, 0}, { 1, 1}
}; //for 10mS

int shiftTable_100mS_2[136][2] = {
	{ 0, 0}, { 1, 0}, { 0, 1}, { 1, 1}, { 0, 2}, { 1, 2}, { 0, 3}, { 1, 3},
	{ 0, 4}, { 1, 4}, { 0, 5}, { 1, 5}, { 0, 6}, { 1, 6}, { 0, 7}, { 1, 7},
	{ 0, 8}, { 1, 8}, { 0, 9}, { 1, 9}, { 0,10}, { 1,10}, { 0,11}, { 1,11},
	{ 0,12}, { 1,12}, { 0,13}, { 1,13}, { 0,14}, { 1,14}, { 0,15}, { 1,15},
	{ 0,16}, { 1,16}, { 0,17}, { 1,17}, { 0,18}, { 1,18}, { 0,19}, { 1,19},
	{ 0,20}, { 1,20}, { 0,21}, { 1,21}, { 0,22}, { 1,22}, { 0,23}, { 1,23},
	{ 0,24}, { 1,24}, { 0,25}, { 1,25}, { 0,26}, { 1,26}, { 0,27}, { 1,27},
	{ 0,28}, { 1,28}, { 0,29}, { 1,29}, { 0,30}, { 1,30}, { 0,31}, { 1,31},
	{ 0,32}, { 1,32}, { 0,33}, { 1,33}, { 0,34}, { 1,34}, { 0,35}, { 1,35},
	{ 0,36}, { 1,36}, { 0,37}, { 1,37}, { 0,38}, { 1,38}, { 0,39}, { 1,39},
	{ 0,40}, { 1,40}, { 0,41}, { 1,41}, { 0,42}, { 1,42}, { 0,43}, { 1,43},
	{ 0,44}, { 1,44}, { 0,45}, { 1,45}, { 0,46}, { 1,46}, { 0,47}, { 1,47},
	{ 0,48}, { 1,48}, { 0,49}, { 1,49}, { 0,50}, { 1,50}, { 0,51}, { 1,51},
	{ 0,52}, { 1,52}, { 0,53}, { 1,53}, { 0,54}, { 1,54}, { 0,55}, { 1,55},
	{ 0,56}, { 1,56}, { 0,57}, { 1,57}, { 0,58}, { 1,58}, { 0,59}, { 1,59},
	{ 0,60}, { 1,60}, { 0,61}, { 1,61}, { 0,62}, { 1,62}, { 0,63}, { 1,63},
	{ 2, 0}, { 3, 0}, { 2, 1}, { 3, 1}, { 2, 2}, { 3, 2}, { 2, 3}, { 3, 3}
};

void AnalogValue_Input_100mS(int mainSlot)
{
	int slot, type, ch;

	slot = mainSlot;
	if(slot >= 72){
	 	slot = 0;
		return;
	}

	type = shiftTable_100mS[slot][0];
	ch = shiftTable_100mS[slot][1];

   	GetADValue(ch, type);
		
	slot += 1;
	if(slot >= 72) slot = 0;

	type = shiftTable_100mS[slot][0];
	ch = shiftTable_100mS[slot][1];

	SetMux(ch, type);
}

void AnalogValue_Input_50mS(int mainSlot)
{
	int slot, type, ch;

	slot = mainSlot;
	if(slot >= 40){
		slot = 0;
		return;
	}

	type = shiftTable_50mS[slot][0];
	ch = shiftTable_50mS[slot][1];

   	GetADValue(ch, type);
		
	slot += 1;
	if(slot >= 40) slot = 0;

	type = shiftTable_50mS[slot][0];
	ch = shiftTable_50mS[slot][1];

	SetMux(ch, type);
}

void AnalogValue_Input_100mS_Ch(int mainSlot)
{
	int slot, type, ch;

	slot = mainSlot;
	if(slot >= 72) slot = 0;

	type = shiftTable_100mS[slot][0];
	ch = shiftTable_100mS[slot][1];

   	GetADValue_Ch(ch, type, slot);
	
	slot += 1;
	if(slot >= 72) slot = 0;
	type = shiftTable_100mS[slot][0];
	ch = shiftTable_100mS[slot][1];

	SetMux(ch, type);
}

void AnalogValue_Input_100mS_Ch_2(int mainSlot)
{
	int slot, type, ch;

	slot = mainSlot;
	if(slot >= 136){
	 	slot = 0;
		return;
	}

	type = shiftTable_100mS_2[slot][0];
	ch = shiftTable_100mS_2[slot][1];

   	GetADValue_Ch(ch, type, slot);
	
	slot += 1;
	if(slot >= 136) slot = 0;
	type = shiftTable_100mS_2[slot][0];
	ch = shiftTable_100mS_2[slot][1];

	SetMux(ch, type);
}


void AnalogValue_Input_50mS_Ch(int mainSlot)
{
	int slot, type, ch;

	slot = mainSlot;
	if(slot >= 40){
		slot = 0;
		return;
	}

	type = shiftTable_50mS[slot][0];
	ch = shiftTable_50mS[slot][1];

   	GetADValue_Ch(ch, type, slot);
	
	slot += 1;
	if(slot >= 40) slot = 0;
	type = shiftTable_50mS[slot][0];
	ch = shiftTable_50mS[slot][1];

	SetMux(ch, type);
}

void AnalogValue_Input_100mS_Ch_AD2_64Ch(int mainSlot)
{
	int slot, type, ch;
  // 	int addr, base_addr, addr_step, ad_start;

	slot = mainSlot;
	if(slot >= 68){
		slot = 0;
	}

	type = shiftTable_100mS_AD2[slot][0];
	ch = shiftTable_100mS_AD2[slot][1];

  	GetADValue_Ch_AD2(ch, type, slot, 3);
}

void AnalogValue_Input_50mS_Ch_AD2(int mainSlot)
{
	int slot, type, ch;
  // 	int addr, base_addr, addr_step, ad_start;

	slot = mainSlot;
	if(slot >= 36){
		 slot = 0;
	}

	type = shiftTable_50mS_AD2[slot][0];
	ch = shiftTable_50mS_AD2[slot][1];

  	GetADValue_Ch_AD2(ch, type, slot, 0);
/*	
	slot += 1;
	if(slot >= 36) 	slot = 0;

	type = shiftTable_50mS_AD2[slot][0];
	ch = shiftTable_50mS_AD2[slot][1];
	
	SetMux(ch, type);
	if(myPs->config.MainBdType == FPGA_TYPE)
	{
		base_addr = myPs->addr.main[BASE_ADDR];
		addr_step = myPs->addr.main[ADDR_STEP];
		ad_start = myPs->addr.main[AD_RC];
	
		for(bd = 0; bd < myPs->config.installedBd; bd++){
			addr = base_addr + addr_step * bd;
    	   	outb(0x00, addr + ad_start);	//AD START
		}
	}*/
}

void AnalogValue_Input_50mS_Ch_AD2_2uA(int mainSlot)
{	//210510 add for 500mA/2uA Type Cycler
	int slot, type, ch;

	slot = mainSlot;
	if(slot >= 36){
		 slot = 0;
	}

	type = shiftTable_50mS_AD2[slot][0];
	ch = shiftTable_50mS_AD2[slot][1];

  	GetADValue_Ch_AD2_2uA(ch, type, slot, 0);
}

void AnalogValue_Input_25mS_Ch_AD2(int mainSlot)
{
	int slot, type, bd, ch;

	slot = mainSlot;
	if(slot >= 19) slot = 0;

	type = shiftTable_25mS_AD2[slot][0];
	ch = shiftTable_25mS_AD2[slot][1];

	if(myPs->config.parallelMode == P2) { //kjg_180521
   		GetADValue_Ch_AD2p(ch, type, slot, 1);

		for(bd=0; bd < myPs->config.installedBd; bd++) {
			if(type == 0) {
			//  190109
			//	ch--;
			//	if(ch < 0) ch = 15;
				ChannelControl_Ch(bd, ch);
			}
		}
	} else {
   		GetADValue_Ch_AD2(ch, type, slot, 1);
	}
}

void AnalogValue_Input_20mS_Ch_AD2(int mainSlot)
{
	int slot, type, ch;
//   	int addr, base_addr, addr_step, ad_start;

	slot = mainSlot;
	if(slot >= 12){
		slot = 0;
	}

	type = shiftTable_20mS_AD2[slot][0];
	ch = shiftTable_20mS_AD2[slot][1];

   	GetADValue_Ch_AD2(ch, type, slot, 2);
/*(	if(myPs->config.MainBdType == CPLD_TYPE) {
		slot += 1;
		if(slot >= 12) slot = 0;
		type = shiftTable_20mS_AD2[slot][0];
		ch = shiftTable_20mS_AD2[slot][1];
		SetMux(ch, type);
	}*/
/*	
	if(myPs->config.MainBdType == FPGA_TYPE)
	{
		base_addr = myPs->addr.main[BASE_ADDR];
		addr_step = myPs->addr.main[ADDR_STEP];
		ad_start = myPs->addr.main[AD_RC];
	
		for(bd = 0; bd < myPs->config.installedBd; bd++){
			addr = base_addr + addr_step * bd;
    	   	outb(0x00, addr + ad_start);	//AD START
		}
	}*/
}

void AnalogValue_Input_20mS_Ch_AD2_FAD(int mainSlot)
{
	int slot, type, ch;
//   	int addr, base_addr, addr_step, ad_start;

	slot = mainSlot;
	if(slot >= 11){
		slot = 0;
	}

	type = shiftTable_20mS_AD2_FAD[slot][0];
	ch = shiftTable_20mS_AD2_FAD[slot][1];

   	GetADValue_Ch_AD2(ch, type, slot, 5);
}

void AnalogValue_Input_20mS_Pack(int mainSlot)
{
	int slot, type, ch;

	slot = mainSlot;
	if(slot >= 12){
		slot = 0;
	}

	type = shiftTable_20mS_AD2[slot][0];
	ch = shiftTable_20mS_AD2[slot][1];

   	GetADValue_Ch_Pack(ch, type, slot);
	
	slot += 1;
	if(slot >= 12) slot = 0;
	type = shiftTable_20mS_AD2[slot][0];
	ch = shiftTable_20mS_AD2[slot][1];

	SetMux(ch, type);
}

void AnalogValue_Input_10mS_Ch_AD2(int mainSlot)
{ //140520 oys w : rt_scan_time_10mS
	int slot, type, ch;

	slot = mainSlot;
	if(slot >= 10){
		slot = 0;
	}
	type = shiftTable_10mS_AD2[slot][0];
	ch = shiftTable_10mS_AD2[slot][1];

	GetADValue_Ch_AD2(ch, type, slot, 4);
}

void GetADValue(int ch, int type)
{
    int	i, bd;
   	int addr, base_addr, addr_step, ad_start;
	hrtime_t abstime;

	if((type == 0 || type == 1)
		&& ch >= myPs->config.installedCh) return;

	base_addr = myPs->addr.main[BASE_ADDR];
	addr_step = myPs->addr.main[ADDR_STEP];
	ad_start = myPs->addr.main[AD_RC];
	
	for(i=0; i < MAX_AD_COUNT; i++) {
		for(bd = 0; bd < myPs->config.installedBd; bd++){
			addr = base_addr + addr_step * bd;
    	   	outb(0x00, addr + ad_start);	//AD START
		}

	    abstime = clock_gethrtime(CLOCK_REALTIME)+20000;
   	   	clock_nanosleep(CLOCK_REALTIME,TIMER_ABSTIME,hrt2ts(abstime),NULL);

		for(bd = 0; bd < myPs->config.installedBd; bd++){
			addr = base_addr + addr_step * bd;
	   		ReadADData(bd, ch, type, i, addr);
		}
	}
}

void GetADValue_Ch(int ch, int type, int slot)
{
//	if(myPs->config.ADC_type == COMM_TYPE){
		GetADValue_Ch_Default(ch, type, slot);
//	}else{
//		GetADValue_Ch_2(ch, type, slot);
//	}
}

void GetADValue_Ch_Default(int ch, int type, int slot)
{
    int	i, bd;
   	int addr, base_addr, addr_step, ad_start;
	hrtime_t abstime;

	if((type == 0 || type == 1)
		&& ch >= myPs->config.installedCh) return;

	base_addr = myPs->addr.main[BASE_ADDR];
	addr_step = myPs->addr.main[ADDR_STEP];
	ad_start = myPs->addr.main[AD_RC];
	
	for(i=0; i < MAX_AD_COUNT; i++) {
		for(bd = 0; bd < myPs->config.installedBd; bd++){
			addr = base_addr + addr_step * bd;
    	   	outb(0x00, addr + ad_start);	//AD START
		}

	    abstime = clock_gethrtime(CLOCK_REALTIME) + 20000;
   	   	clock_nanosleep(CLOCK_REALTIME,TIMER_ABSTIME,hrt2ts(abstime),NULL);

		for(bd = 0; bd < myPs->config.installedBd; bd++){
			addr = base_addr + addr_step * bd;
	   		ReadADData(bd, ch, type, i, addr);
		}
	}

/*	slot += 1;
	if(slot >= 72) slot = 0;
	mux_type = shiftTable_100mS[slot][0];
	mux_ch = shiftTable_100mS[slot][1];

	SetMux(mux_ch, mux_type);
*/
	for(bd = 0; bd < myPs->config.installedBd; bd++){
		CalChAD_Filter_Ch(bd, ch, type);
		CalChAverage_Ch(bd, ch, type);
		if(type == 1){
			ChannelControl_Ch(bd, ch);
		}
	}
}

void GetADValue_Ch_2(int ch, int type, int slot)
{
    int	i, bd;
   	int addr, base_addr, addr_step, ad_start;
	hrtime_t abstime;

	if((type == 0 || type == 1)
		&& ch >= myPs->config.installedCh) return;

	base_addr = myPs->addr.main[BASE_ADDR];
	addr_step = myPs->addr.main[ADDR_STEP];
	ad_start = myPs->addr.main[AD_RC];

	bd = ch / myPs->config.chPerBd;
	ch = ch % myPs->config.chPerBd;
	
	for(i=0; i < MAX_AD_COUNT; i++) {
		addr = base_addr;
   	   	outb(0x00, addr + ad_start);	//AD START

	    abstime = clock_gethrtime(CLOCK_REALTIME) + 20000;
   	   	clock_nanosleep(CLOCK_REALTIME,TIMER_ABSTIME,hrt2ts(abstime),NULL);

		addr = base_addr;
   		ReadADData(bd, ch, type, i, addr);
	}

	CalChAD_Filter_Ch(bd, ch, type);
	CalChAverage_Ch(bd, ch, type);
	if(type == 1){
		ChannelControl_Ch(bd, ch);
	}
}

void GetADValue_Ch_AD2(int ch, int type, int slot, int contInt)
{
   	int addr, base_addr, addr_step, ad_start, i, bd, mux_type, mux_ch;
	hrtime_t abstime;

	base_addr = myPs->addr.main[BASE_ADDR];
	addr_step = myPs->addr.main[ADDR_STEP];
	ad_start = myPs->addr.main[AD_RC];

	if(myPs->config.MainBdType == CPLD_TYPE) {
		for(i=0; i < MAX_AD_COUNT; i++) {
			for(bd=0; bd < myPs->config.installedBd; bd++) {
				addr = base_addr + addr_step * bd;
   		 	   	outb(0x00, addr + ad_start); //AD START
			}

	   		abstime = clock_gethrtime(CLOCK_REALTIME) + 20000;
   	   		clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, hrt2ts(abstime),
				NULL);

			for(bd=0; bd < myPs->config.installedBd; bd++) {
				addr = base_addr + (addr_step * bd);
	   			ReadADData_AD2(bd, ch, type, i, addr);
			}
		}
	} else if(myPs->config.MainBdType == FPGA_TYPE) {
		for(bd=0; bd < myPs->config.installedBd; bd++) {
			addr = base_addr + addr_step * bd;
	   		ReadADData_AD2(bd, ch, type, 0, addr);
		}
	}

	slot += 1;
	switch(contInt) {
		case 0:
			if(slot >= 36) slot = 0;

			mux_type = shiftTable_50mS_AD2[slot][0];
			mux_ch = shiftTable_50mS_AD2[slot][1];
	
			SetMux(mux_ch, mux_type);
			break;
		case 1:
			if(slot >= 19) slot = 0;

			mux_type = shiftTable_25mS_AD2[slot][0];
			mux_ch = shiftTable_25mS_AD2[slot][1];
	
			SetMux(mux_ch, mux_type);
			break;
		case 2:
			if(slot >= 12) slot = 0;

			mux_type = shiftTable_20mS_AD2[slot][0];
			mux_ch = shiftTable_20mS_AD2[slot][1];
	
			SetMux(mux_ch, mux_type);
			break;
		case 3:
			if(slot >= 68) slot = 0;

			mux_type = shiftTable_100mS_AD2[slot][0];
			mux_ch = shiftTable_100mS_AD2[slot][1];
	
			SetMux(mux_ch, mux_type);
			break;
		case 4:
			if(slot >=10) slot = 0;

			mux_type = shiftTable_10mS_AD2[slot][0];
			mux_ch = shiftTable_10mS_AD2[slot][1];
	
			SetMux(mux_ch, mux_type);
			break;
		case 5:
			if(slot >= 11) slot = 0;

			mux_type = shiftTable_20mS_AD2_FAD[slot][0];
			mux_ch = shiftTable_20mS_AD2_FAD[slot][1];
	
			SetMux(mux_ch, mux_type);
			break;
		default:
			break;
	}

	if((type == 0) && ch >= myPs->config.installedCh) return;

	for(bd=0; bd < myPs->config.installedBd; bd++) {
		CalChAD_Filter_Ch_AD2(bd, ch, type);
		CalChAverage_Ch_AD2(bd, ch, type);

		if(type == 0) ChannelControl_Ch(bd, ch);
	}

	if(myPs->config.MainBdType == FPGA_TYPE) {
		for(bd=0; bd < myPs->config.installedBd; bd++) {
			addr = base_addr + addr_step * bd;
    	   	outb(0x00, addr + ad_start); //AD START
		}
	}
}

void GetADValue_Ch_AD2_2uA(int ch, int type, int slot, int contInt)
{	//210510 add for 500mA/2uA Type Cycler
	unsigned char	sensCh, hwChSelect, bd, mux_type;
   	int addr, base_addr, addr_step, ad_start, mux_ch;

	base_addr = myPs->addr.main[BASE_ADDR];
	addr_step = myPs->addr.main[ADDR_STEP];
	ad_start = myPs->addr.main[AD_RC];

	for(bd = 0; bd < myPs->config.installedBd; bd++) {
		addr = base_addr + addr_step * bd;
   		ReadADData_AD2(bd, ch, type, 0, addr);
	}

	//210601 lyhw add for next Ch AD SetMux
	bd = ch / myPs->config.chPerBd;

	//210716 lyhw 1ch 2uA Seelct
	sensCh = ch + 1;
	if(slot >= 35) sensCh = 0;
	
	if(myData->bData[bd].cData[sensCh].signal[C_SIG_I_RANGE] == RANGE4){
		hwChSelect = 1;
	}else{
		hwChSelect = 0;
	}

	slot += 1;
	switch(hwChSelect) {
		case 0:
			if(slot >= 36) slot = 0;
			//500mA Spec
			mux_type = shiftTable_50mS_AD2_500mA[slot][0];
			mux_ch = shiftTable_50mS_AD2_500mA[slot][1];
			
			for(bd = 0; bd < myPs->config.installedBd; bd++){
				addr = base_addr + addr_step * bd;		
				SetMux_15(mux_type, mux_ch, addr);
			}
			break;
		case 1:
			if(slot >= 36) slot = 0;
			//2uA Spec
			mux_type = shiftTable_50mS_AD2_2uA[slot][0];
			mux_ch = shiftTable_50mS_AD2_2uA[slot][1];
			
			for(bd = 0; bd < myPs->config.installedBd; bd++){
				addr = base_addr + addr_step * bd;		
				SetMux_15(mux_type, mux_ch, addr);
			}
			break;
		default:
			break;
	}

	if((type == 0) && ch >= myPs->config.installedCh) return;

	for(bd=0; bd < myPs->config.installedBd; bd++) {
		CalChAD_Filter_Ch_AD2(bd, ch, type);
		CalChAverage_Ch_AD2(bd, ch, type);

		if(type == 0) ChannelControl_Ch(bd, ch);
	}

	for(bd=0; bd < myPs->config.installedBd; bd++) {
		addr = base_addr + addr_step * bd;
       	outb(0x00, addr + ad_start); //AD START
	}
}

void GetADValue_Ch_AD2p(int ch, int type, int slot, int contInt)
{ //kjg_180521
   	int addr, base_addr, addr_step, ad_start, i, bd, mux_type, mux_ch;
	hrtime_t abstime;

	base_addr = myPs->addr.main[BASE_ADDR];
	addr_step = myPs->addr.main[ADDR_STEP];
	ad_start = myPs->addr.main[AD_RC];

	if(myPs->config.MainBdType == CPLD_TYPE) {
		for(i=0; i < MAX_AD_COUNT; i++) {
			for(bd=0; bd < myPs->config.installedBd; bd++) {
				addr = base_addr + addr_step * bd;
   		 	   	outb(0x00, addr + ad_start); //AD START
			}

	   		abstime = clock_gethrtime(CLOCK_REALTIME) + 20000;
   	   		clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, hrt2ts(abstime),
				NULL);

			for(bd=0; bd < myPs->config.installedBd; bd++) {
				addr = base_addr + (addr_step * bd);
	   			ReadADData_AD2(bd, ch, type, i, addr);
			}
		}
	} else if(myPs->config.MainBdType == FPGA_TYPE) {
		for(bd=0; bd < myPs->config.installedBd; bd++) {
			addr = base_addr + addr_step * bd;
	   		ReadADData_AD2(bd, ch, type, 0, addr);
		}
	}

	slot += 1;
	switch(contInt) {
		case 0:
			if(slot >= 36) slot = 0;

			mux_type = shiftTable_50mS_AD2[slot][0];
			mux_ch = shiftTable_50mS_AD2[slot][1];
	
			SetMux(mux_ch, mux_type);
			break;
		case 1:
			if(slot >= 19) slot = 0;

			mux_type = shiftTable_25mS_AD2[slot][0];
			mux_ch = shiftTable_25mS_AD2[slot][1];
	
			SetMux(mux_ch, mux_type);
			break;
		case 2:
			if(slot >= 12) slot = 0;

			mux_type = shiftTable_20mS_AD2[slot][0];
			mux_ch = shiftTable_20mS_AD2[slot][1];
	
			SetMux(mux_ch, mux_type);
			break;
		case 3:
			if(slot >= 68) slot = 0;

			mux_type = shiftTable_100mS_AD2[slot][0];
			mux_ch = shiftTable_100mS_AD2[slot][1];
	
			SetMux(mux_ch, mux_type);
			break;
		case 4:
			if(slot >=10) slot = 0;

			mux_type = shiftTable_10mS_AD2[slot][0];
			mux_ch = shiftTable_10mS_AD2[slot][1];
	
			SetMux(mux_ch, mux_type);
			break;
		case 5:
			if(slot >= 11) slot = 0;

			mux_type = shiftTable_20mS_AD2_FAD[slot][0];
			mux_ch = shiftTable_20mS_AD2_FAD[slot][1];
	
			SetMux(mux_ch, mux_type);
			break;
		default:
			break;
	}

	if((type == 0) && ch >= myPs->config.installedCh) return;

	for(bd=0; bd < myPs->config.installedBd; bd++) {
		CalChAD_Filter_Ch_AD2(bd, ch, type);
		CalChAverage_Ch_AD2p(bd, ch, type);
	}

	if(myPs->config.MainBdType == FPGA_TYPE) {
		for(bd=0; bd < myPs->config.installedBd; bd++) {
			addr = base_addr + addr_step * bd;
    	   	outb(0x00, addr + ad_start); //AD START
		}
	}
}

void GetADValue_Ch_Pack(int ch, int type, int slot)
{
    int	bd, ad_cnt, ad_mux;
   	int addr, base_addr, addr_step, ad_start;
	hrtime_t abstime;

	if((type == 0)
		&& ch >= myPs->config.installedCh) return;

	base_addr = myPs->addr.main[BASE_ADDR];
	addr_step = myPs->addr.main[ADDR_STEP];
	ad_start = myPs->addr.main[AD_RC];
	
	for(ad_mux = 0; ad_mux < 4; ad_mux++){
		addr = base_addr;
		outb((unsigned char)ad_mux, addr + ad_start);	//AD START

	    abstime = clock_gethrtime(CLOCK_REALTIME) + 20000;
   	   	clock_nanosleep(CLOCK_REALTIME,TIMER_ABSTIME,hrt2ts(abstime), NULL);

		for(ad_cnt = 0; ad_cnt < MAX_AD_COUNT; ad_cnt++){
	   		ReadADData_Pack(ch, type, (int)ad_mux, ad_cnt);
		}
	}

	for(bd=0; bd < myPs->config.installedBd; bd++) {
		CalChAD_Filter_Ch_AD2(bd, ch, type);
		CalChAverage_Ch_AD2(bd, ch, type);

		if(type == 0) ChannelControl_Ch(bd, ch);
	}
}

//20190605 KHK-------------------------------
void AnalogValue_Input_100mS_CAN(int mainSlot)
{
	int bd, ch;

	bd = mainSlot / myPs->config.chPerBd; 
	ch = mainSlot % myPs->config.chPerBd;

	if(bd >= myPs->config.installedBd) return;
	if(ch >= myPs->config.chPerBd) return;

	CalChAD_Filter_Ch_CAN(bd, ch, 0);
	CalChAD_Filter_Ch_CAN(bd, ch, 1);

	CalChAverage_Ch_CAN(bd, ch);
	ChannelControl_Ch(bd, ch);
}
//20190605----------------------------------------

//210204 lyhw
void AnalogValue_Input_50mS_CAN(int mainSlot)
{
	int bd, ch;

	bd = mainSlot / myPs->config.chPerBd; 
	ch = mainSlot % myPs->config.chPerBd;

	if(bd >= myPs->config.installedBd) return;
	if(ch >= myPs->config.chPerBd) return;

	CalChAD_Filter_Ch_CAN(bd, ch, 0);
	CalChAD_Filter_Ch_CAN(bd, ch, 1);

	CalChAverage_Ch_CAN(bd, ch);
	ChannelControl_Ch(bd, ch);
}

//20190605 KHK------------------------------------
void CalChAD_Filter_Ch_CAN(int bd, int ch, int type)
{
	double sum;
	long cmp_val = 0.0;
	int ref_idx;
	
	if(bd >= myPs->config.installedBd) return;
	if(ch >= myPs->config.chPerBd) return;

	//find can reference position '0'base
	ref_idx = myData->CAN.config.canInBd[bd] - 1;
	//check just one channel comm error
	cmp_val = (myData->CAN.config.canCommTimeOut[ref_idx] * 100); //100 = 1sec;

	if(myData->bData[bd].cData[ch].misc.can_read_update_flag == P1) {
		if(type == 0){	// Voltage
			myData->bData[bd].cData[ch].misc.adValue[type][0]
				= myData->bData[bd].cData[ch].misc.can_read_v;
		}else{ 			// Current
			myData->bData[bd].cData[ch].misc.adValue[type][0]
				= myData->bData[bd].cData[ch].misc.can_read_i;
		}
		sum = myData->bData[bd].cData[ch].misc.adValue[type][0];
		ConvertAD_Data_CAN(bd, ch, type, sum);
	} else {
		//after channel comm error
		if(myData->bData[bd].cData[ch].misc.can_read_update_flag == P2) {
			myData->bData[bd].cData[ch].misc.adValue[type][0] = 0.0;
			myData->bData[bd].cData[ch].misc.can_read_errCnt = 0.0;
		//before channel comm error
		}else{
			myData->bData[bd].cData[ch].misc.can_read_errCnt
				+= myPs->misc.rt_scan_time;
			if(myData->bData[bd].cData[ch].misc.can_read_errCnt > cmp_val)
				myData->bData[bd].cData[ch].misc.can_read_update_flag = P2; 
			//occure comm error
			if(myData->bData[bd].cData[ch].op.state == C_RUN) {
				myData->bData[bd].cData[ch]
					.signal[C_SIG_MAIN_CAN_COMM_ERROR] = P1;
			}
		}
	}
}
//20190605---------------------------------------

//20190605 KHK-----------------------------------
void CalChAverage_Ch_CAN(int bd, int ch)
{
	if(bd >= myPs->config.installedBd) return;
	if(ch >= myPs->config.chPerBd) return;

	cCalculate_Voltage(bd, ch);
	cCalculate_Current(bd, ch);

	cCalculate_Capacity(bd, ch);
	cCalculate_Watt(bd, ch);
}
//20190605-------------------------------------------------


void ReadADData(int bd, int ch, int type, int sensCount, int addr)
{
//	if(myPs->config.ADC_type == COMM_TYPE){
		ReadADData_Default(bd, ch, type, sensCount, addr);
//	}else{
//		ReadADData_2(bd, ch, type, sensCount, addr);
//	}
}

void ReadADData_Default(int bd, int ch, int type, int sensCount, int addr)
{
	int	ad_data_h, ad_data_l; 
	U_ADDA ADValue;
	ad_data_h = myPs->addr.main[AD_RC];
	ad_data_l = myPs->addr.main[AD_BYTE];
	
	ADValue.byte[1] = (unsigned char)inb(addr + ad_data_h);
	ADValue.byte[0] = (unsigned char)inb(addr + ad_data_l);

	switch(type) {
		case 0: //V
			myData->bData[bd].cData[ch].misc.adValue[type][sensCount]
				= (long)ADValue.val;
			break;
		case 1: //I
			myData->bData[bd].cData[ch].misc.adValue[type][sensCount]
				= (long)ADValue.val;
			break;
		case 2: //sourceV
			myData->bData[bd].misc.source[ch].adValue[type-2][sensCount]
				= (long)ADValue.val;
			break;
		case 3: //sourceI
			myData->bData[bd].misc.source[ch].adValue[type-2][sensCount]
				= (long)ADValue.val;
			break;
		default: break;
	}
}

void ReadADData_2(int bd, int ch, int type, int sensCount, int addr)
{
	int	ad_data_h, ad_data_l, i; 
	U_ADDA ADValue;

	ad_data_h = myPs->addr.main[AD_RC];
	ad_data_l = myPs->addr.main[AD_BYTE];

	
	ADValue.byte[1] = (unsigned char)inb(addr + ad_data_h);
	ADValue.byte[0] = (unsigned char)inb(addr + ad_data_l);

	switch(type) {
		case 0: //V
			myData->bData[bd].cData[ch].misc.adValue[type][sensCount]
				= (long)ADValue.val;
			break;
		case 1: //I
			myData->bData[bd].cData[ch].misc.adValue[type][sensCount]
				= (long)ADValue.val;
			break;
		case 2: //sourceV
			for(i = 0; i < myPs->config.installedBd; i++){
				myData->bData[i].misc.source[ch].adValue[type-2][sensCount]
					= (long)ADValue.val;
			}
			break;
		case 3: //sourceI
			for(i = 0; i < myPs->config.installedBd; i++){
				myData->bData[i].misc.source[ch].adValue[type-2][sensCount]
					= (long)ADValue.val;
			}
			break;
		default: break;
	}
}

void ReadADData_AD2(int bd, int ch, int type, int sensCount, int addr)
{
	unsigned char byte[7];
	int	ad_data_h, ad_data_l; 
	U_ADDA ADValueV,ADValueI;

	ad_data_h = myPs->addr.main[AD_RC];
	ad_data_l = myPs->addr.main[AD_BYTE];

	//read ad I
	byte[0] = (unsigned char)inb(addr + ad_data_h);
	ADValueI.byte[1] = (unsigned char)byte[0];
	byte[0] = (unsigned char)inb(addr + ad_data_l);
	ADValueI.byte[0] = (unsigned char)byte[0];

	//read ad V
	byte[0] = (unsigned char)inb(addr + ad_data_h + 3);
	ADValueV.byte[1] = (unsigned char)byte[0];
	byte[0] = (unsigned char)inb(addr + ad_data_l + 3);
	ADValueV.byte[0] = (unsigned char)byte[0];

	switch(type) {
		case 0: //ch
			myData->bData[bd].cData[ch].misc.adValue[type][sensCount]
				= (long)ADValueV.val;
			myData->bData[bd].cData[ch].misc.adValue[type + 1][sensCount]
				= (long)ADValueI.val;
			break;
		case 1: //source
			myData->bData[bd].misc.source[ch].adValue[type - 1][sensCount]
				= (long)ADValueV.val;
			myData->bData[bd].misc.source[ch].adValue[type][sensCount]
				= (long)ADValueI.val;

			if(myPs->config.rt_scan_type == RT_SCAN_PERIOD_10mS) {
				if(ch == 1) {
					myData->bData[bd].misc.source[2]
						.adValue[type - 1][sensCount]
						= (myData->bData[bd].misc.source[0]
						.adValue[type - 1][sensCount]
						+ myData->bData[bd].misc.source[1]
						.adValue[type - 1][sensCount]);

					myData->bData[bd].misc.source[2].adValue[type][sensCount]
						= (myData->bData[bd].misc.source[0]
						.adValue[type][sensCount]
						+ myData->bData[bd].misc.source[1]
						.adValue[type][sensCount]);
				}
			}
			break;
		default:
			break;
	}
}

void ReadADData_Pack(int ch, int type, int ad_mux, int sensCount)
{
	int bd, j;
	int	ad_data_h, ad_data_l, addr, base_addr;
	U_ADDA ADValue[2];

	base_addr = myPs->addr.main[BASE_ADDR];
	ad_data_h = myPs->addr.main[AD_RC];
	ad_data_l = myPs->addr.main[AD_BYTE];

	if(ch < 4){
		j = 0;
		addr =  base_addr + ad_data_h;
		ADValue[j].byte[1] = (unsigned char)inb(addr + sensCount * 2);
		addr =  base_addr + ad_data_l;
		ADValue[j].byte[0] = (unsigned char)inb(addr + sensCount * 2);
	}else{
		j = 1;
		addr =  base_addr + ad_data_h + 0x08;
		ADValue[j].byte[1] = (unsigned char)inb(addr + sensCount * 2);
		addr =  base_addr + ad_data_l + 0x08;
		ADValue[j].byte[0] = (unsigned char)inb(addr + sensCount * 2);
	}

	switch(type) {
		case 0: //ch
			switch(ad_mux) {
				case 0: //voltage
				case 1: //current
					for(bd = 0; bd < myPs->config.installedBd; bd++){
						myData->bData[bd].cData[ch].misc.adValue[ad_mux][sensCount]
							= (long)ADValue[j].val;
					}
					break;
			}
			break;
		case 1: //source
			switch(ad_mux) {
				case 0: //voltage
				case 1: //current
					for(bd = 0; bd < myPs->config.installedBd; bd++){
						myData->bData[bd].misc.source[ch].adValue[ad_mux][sensCount]
						= (long)ADValue[j].val;
					}
					break;
			}
			break;
		default: break;
	}
}


void SetMux(int ch, int type)
{
    int	bd, hwSpec;
	int	addr, base_addr, addr_step;
	
	if(myPs->config.ADC_num < 2){
		if((type == 0 || type == 1) && ch >= myPs->config.installedCh) return;
	} else {
		if((type == 0) && ch >= myPs->config.installedCh){
			type = 1; ch = 0;
		}
	}

	bd = 0;

	base_addr = myPs->addr.main[BASE_ADDR];
	addr_step = myPs->addr.main[ADDR_STEP];

	if(myPs->config.ADC_type == COMM_TYPE){
		bd = ch / myPs->config.chPerBd;
		addr = base_addr + addr_step * bd;
	}else{
		addr = base_addr;
	}
	hwSpec = myPs->config.hwSpec;

	if(hwSpec == L_5V_1A_R3){
		for(bd = 0; bd < myPs->config.installedBd; bd++){
			addr = base_addr + addr_step * bd;		
			SetMux_13(type, ch, addr);
		}
		return;
	}
	
	if(hwSpec == L_5V_10A_R3_NEW){
		//add 120601 kji 5V10A Back planboard mux 
		for(bd = 0; bd < myPs->config.installedBd; bd++){
			addr = base_addr + addr_step * bd;		
			SetMux_14(type, ch, addr);
		}
		return;
	}
	if(hwSpec == L_MULTI){
		//add 140311 lyh L_MULTI mux 
		for(bd = 0; bd < myPs->config.installedBd; bd++){
			addr = base_addr + addr_step * bd;		
			SetMux_12(type, ch, addr);
		}
		return;
	}

	// rt scan type 2,3 AD2 SetMux_10
	if(myPs->config.ADC_num >= 2 || 
		myPs->config.rt_scan_type >= RT_SCAN_PERIOD_25mS)
	{
		ch = ch % myPs->config.chPerBd;
		addr = base_addr + addr_step * bd;
		for(bd = 0; bd < myPs->config.installedBd; bd++){
			addr = base_addr + addr_step * bd;		
			SetMux_10(type, ch, addr);
		}
		return;
	}
	
	switch(hwSpec){
		case L_6V_6A:
		case L_5V_5A:
		case L_5V_5A_2:
		case L_5V_50A:
			SetMux_1(type, ch, addr);
			break;
		case L_5V_20A:
			SetMux_2(type, ch, addr);
			break;
		case L_5V_30A:
			SetMux_3(type, ch, addr);
			break;
		case L_5V_10mA:
			SetMux_4(type, ch, addr);
			break;
		case L_5V_200A:
		case L_2V_100A:
		case L_5V_100A_R2:
			SetMux_5(type, ch, addr);
			break;
		case L_50V_50A:
			SetMux_6(type, ch, addr);
			break;
		case L_20V_25A:
			SetMux_7(type, ch, addr);
			break;
/*		case L_5V_100A:
		case L_5V_200A_R2:
		case L_5V_100A_R1:
		case L_5V_100A_R1_EG:
		case L_5V_30A_R1:
		case L_5V_2A_R1:
		case L_5V_2A_R2:
		case L_5V_4A_R2:
		case L_5V_500A_R1:
		case L_5V_500A_R2:
		case L_5V_200A_R4:
		case L_5V_200A_R3:
		case L_5V_150A_R1:
		case L_5V_150A_R3:
		case L_5V_250A_R1:
		case L_5V_50A_R1:
		case L_5V_1000A_R1:
		case L_5V_1000A_R3:
		case L_5V_300A_R1:
		case L_5V_300A_R3:
		case L_20V_10A_R1:
		case L_20V_5A_R1:
		case L_20V_50A_R2:
		case L_5V_50A_R2:
			for(bd = 0; bd < myPs->config.installedBd; bd++){
				addr = base_addr + addr_step * bd;		
				SetMux_8(type, ch, addr);
			}
			break;*/
		case L_10V_5A_R2:
			for(bd = 0; bd < myPs->config.installedBd; bd++){
				addr = base_addr + addr_step * bd;		
				SetMux_9(type, ch, addr);
			}
			break;
		case L_5V_150A_R3_AD2:
		case L_15V_100A_R3_AD2:
		case L_MAIN_REV11:
		case L_5V_200A_R3_P_AD2:
		case S_5V_200A_75A_15A_AD2:
		case L_8CH_MAIN_AD2_P:
			for(bd = 0; bd < myPs->config.installedBd; bd++){
				addr = base_addr + addr_step * bd;		
				SetMux_10(type, ch, addr);
			}
			break;
		case L_5V_600A_10A:
			SetMux_11(type, ch);
			break;
/*
		case L_5V_1A_R3:
			for(bd = 0; bd < myPs->config.installedBd; bd++){
				addr = base_addr + addr_step * bd;		
				SetMux_13(type, ch, addr);
			}
			break;
		case L_5V_10A_R3_NEW:
			//add 120601 kji 5V10A Back planboard mux 
			for(bd = 0; bd < myPs->config.installedBd; bd++){
				addr = base_addr + addr_step * bd;		
				SetMux_14(type, ch, addr);
			}
			break;
		case L_MULTI:
			//add 140311 lyh L_MULTI mux 
			for(bd = 0; bd < myPs->config.installedBd; bd++){
				addr = base_addr + addr_step * bd;		
				SetMux_12(type, ch, addr);
			}
			break;
*/
		default: 
			for(bd = 0; bd < myPs->config.installedBd; bd++){
				addr = base_addr + addr_step * bd;		
				SetMux_8(type, ch, addr);
			}
			break;
	}
}

void SetMux_1(int type, int ch, int addr)
{
	int bd;
	int	mux_en, mux_cs1, mux_cs2;
    unsigned char muxVal1=0x00, muxVal2=0x00;

	// bits-   7   |   6   |   5   |   4   |   3   |   2   |   1   |   0   |
	// mux1- M_A6  | M_EN3 | M_EN2 | M_EN1 | C_EN4 | C_EN3 | C_EN2 | C_EN1 |
	// mux2- M_A5  | M_A4  | M_A3  | M_A2  | M_A1  | C_A3  | C_A2  | C_A1  |
	
	mux_en = myPs->addr.main[MUX_EN];
	mux_cs1 = myPs->addr.main[MUX_CS1];
	mux_cs2 = myPs->addr.main[MUX_CS2];
	switch(type) {
		case 0: //V
			bd = ch / myPs->config.chPerBd;
			ch = ch % myPs->config.chPerBd;
			muxVal1 = 0x01;
			muxVal1 = muxVal1 << bd;
			muxVal1 |= 0x40;
			muxVal2 = ch & 0x07;
			muxVal2 |= 0x78;
			break;
		case 1: //I
			bd = ch / myPs->config.chPerBd;
			ch = ch % myPs->config.chPerBd;
			muxVal1 = 0x01;
			muxVal1 = muxVal1 << bd;
			muxVal1 |= 0x40;
			muxVal2 = ch & 0x07;
			muxVal2 |= 0xF8;
			break;
		case 2: //sourceV
			muxVal1 = 0x50;
			ch = ch << 3;
			muxVal2 = ch & 0x18;
			break;
		case 3: //sourceI
			muxVal1 = 0x60;
			ch = ch << 5;
			muxVal2 = ch & 0x60;
			muxVal2 |= 0x80;
			break;
	}

	//capacitor discharge

	outb(0xC0, (addr + mux_en));
	outb(0xF8, (addr + mux_cs1));
	outb(0xF0, (addr + mux_en));
	usleep(10);//40->10//khk 20060908

	outb(muxVal2, (addr + mux_cs1));
	outb(muxVal1, (addr + mux_en));
}

void SetMux_2(int type, int ch, int addr)
{
	int bd;
	int	mux_en, mux_cs1, mux_cs2;
    unsigned char muxVal1=0x00, muxVal2=0x00, muxVal3=0x00;

	mux_en = myPs->addr.main[MUX_EN];
	mux_cs1 = myPs->addr.main[MUX_CS1];
	mux_cs2 = myPs->addr.main[MUX_CS2];

	switch(type) {
		case 0: //V
			bd = ch / 8;
			ch = ch % 8;
			muxVal3 = 0x01;
			muxVal3 = muxVal3 << bd;
			muxVal3 |= 0x80;
			muxVal2 = ch & 0x07;//Channel Select
			muxVal1 = 0x00;//Voltage
			break;
		case 1: //I
			bd = ch / 8;
			ch = ch % 8;
			muxVal3 = 0x01;
			muxVal3 = muxVal3 << bd;
			muxVal3 |= 0x80;
			muxVal2 = ch & 0x07;//Channel Select
			muxVal1 = 0x10;//Current
			break;
		case 2: //sourceV
			muxVal3 = 0xA0;
			ch = (ch & 0x03);
			muxVal1 = ch;
			muxVal1 |= 0x00;// Voltage
			break;
		case 3: //sourceI
			muxVal3 = 0xC0;
			ch = (ch & 0x03);
			muxVal1 = (ch << 2);
			muxVal1 |= 0x10;//Current
			break;
	}
	//capacitor discharge
	outb(0x3F, (addr + mux_cs1));
	outb(0xE0, (addr + mux_en));//mux enable
	usleep(10);

	outb(muxVal2, (addr + mux_cs2));
	outb(muxVal1, (addr + mux_cs1));
	outb(muxVal3, (addr + mux_en));//mux enable
}


void SetMux_3(int type, int ch, int addr)
{
	int bd;
	int	mux_en, mux_cs1, mux_cs2;
    unsigned char muxVal1=0x00, muxVal2=0x00;

	// bits-   7   |   6   |   5   |   4   |   3   |   2   |   1   |   0   |
	// mux1- M_EN3 | M_EN2 | M_EN1 | C_EN5 | C_EN4 | C_EN3 | C_EN2 | C_EN1 |
	// mux2- M_A6  | M_A5  | M_A4  | M_A3  | M_A2  | M_A1  | C_A2  | C_A1  |
	
	mux_en = myPs->addr.main[MUX_EN];
	mux_cs1 = myPs->addr.main[MUX_CS1];
	mux_cs2 = myPs->addr.main[MUX_CS2];
	switch(type) {
		case 0: //V
			bd = ch / myPs->config.chPerBd;
			ch = ch % myPs->config.chPerBd;
			muxVal1 = 0x01;
			muxVal1 = muxVal1 << bd;
			muxVal1 |= 0x80;
			muxVal2 = ch & 0x03;
			muxVal2 |= 0x3C;
			break;
		case 1: //I
			bd = ch / myPs->config.chPerBd;
			ch = ch % myPs->config.chPerBd;
			muxVal1 = 0x01;
			muxVal1 = muxVal1 << bd;
			muxVal1 |= 0x80;
			muxVal2 = ch & 0x03;
			muxVal2 |= 0x7C;
			break;
		case 2: //sourceV
			muxVal1 = 0xA0;
			ch = ch << 2;
			muxVal2 = ch & 0x0C;
			break;
		case 3: //sourceI
			muxVal1 = 0xC0;
			ch = ch << 4;
			muxVal2 = ch & 0x30;
			muxVal2 |= 0x40;
			break;
	}
	//capacitor discharge
	outb(0x80, (addr + mux_en));
	outb(0xFC, (addr + mux_cs1));
	outb(0xE0, (addr + mux_en));
	usleep(10);//40->10 khk 20060908

	outb(muxVal2, (addr + mux_cs1));
	outb(muxVal1, (addr + mux_en));

}

void SetMux_4(int type, int ch, int addr)
{
	int bd;
	int	mux_en, mux_cs1, mux_cs2;
    unsigned char muxVal1=0x00, muxVal2=0x00, muxVal3=0x00;

	//bits-   7   |   6   |   5   |   4   |   3   |   2   |   1   |   0   |
	//mux3-   X   |   X   | M_A6  | M_A5  | M_A4  | M_A3  | M_A2  | M_A1  |
	//mux2-   X   | M_EN3 | M_EN2 | M_EN1 |   X   | C_A3  | C_A2  | C_A1  |
	//mux1- C_EN8 | C_EN7 | C_EN6 | C_EN5 | C_EN4 | C_EN3 | C_EN2 | C_EN1 |
	
	mux_en = myPs->addr.main[MUX_EN];
	mux_cs1 = myPs->addr.main[MUX_CS1];
	mux_cs2 = myPs->addr.main[MUX_CS2];

	switch(type) {
		case 0: //V
			muxVal3 = 0x00;
			muxVal2 = 0x40;
			bd = ch % 8;
			muxVal2 |= (bd & 0x07);
			bd = ch / 8;
			muxVal1 = 0x01;
			muxVal1 = muxVal1 << bd;
			break;
		case 1: //I
			muxVal3 = 0x10;
			muxVal2 = 0x40;
			bd = ch % 8;
			muxVal2 |= (bd & 0x07);
			bd = ch / 8;
			muxVal1 = 0x01;
			muxVal1 = muxVal1 << bd;
			break;
		case 2: //sourceV
			muxVal3 = (ch & 0x03);
			muxVal2 = 0x50;
			muxVal1 = 0x00;
			break;
		case 3: //sourceI
			muxVal3 = 0x10;
			ch = ch << 2;
			muxVal3 |= (ch & 0x0C);
			muxVal2 = 0x60;
			muxVal1 = 0x00;
			break;
	}

	//bits-   7   |   6   |   5   |   4   |   3   |   2   |   1   |   0   |
	//mux3-   X   |   X   | M_A6  | M_A5  | M_A4  | M_A3  | M_A2  | M_A1  |
	//mux2-   X   | M_EN3 | M_EN2 | M_EN1 |   X   | C_A3  | C_A2  | C_A1  |
	//mux1- C_EN8 | C_EN7 | C_EN6 | C_EN5 | C_EN4 | C_EN3 | C_EN2 | C_EN1 |
	
	//capacitor discharge
	outb(0xFF, (addr + mux_cs2));
	outb(0xF0, (addr + mux_cs1));
	outb(0x00, (addr + mux_en));
	usleep(10);// 40->10 khk 20060908

	outb(muxVal1, (addr + mux_en));
	outb(muxVal2, (addr + mux_cs1));
	outb(muxVal3, (addr + mux_cs2));

}

void SetMux_5(int type, int ch, int addr)
{
	int bd;
	int	mux_en, mux_cs1, mux_cs2;
    unsigned char muxVal1=0x00, muxVal2=0x00;

	// bits-   7   |   6   |   5   |   4   |   3   |   2   |   1   |   0   |
	// mux1- M_A6  |   X   |   X   | M_EN3 | M_EN2 | M_EN1 | C_EN2 | C_EN1 |
	// mux2- M_A5  | M_A4  | M_A3  | M_A2  | M_A1  | C_A3  | C_A2  | C_A1  |
	
	mux_en = myPs->addr.main[MUX_EN];
	mux_cs1 = myPs->addr.main[MUX_CS1];
	mux_cs2 = myPs->addr.main[MUX_CS2];
	switch(type) {
		case 0: //V
			bd = ch / 8;
			ch = ch % 8;
			muxVal1 = 0x01;
			muxVal1 = muxVal1 << bd;
			muxVal1 |= 0x10;
			muxVal2 = ch & 0x07;
			muxVal2 |= 0x78;
			break;
		case 1: //I
			bd = ch / 8;
			ch = ch % 8;
			muxVal1 = 0x01;
			muxVal1 = muxVal1 << bd;
			muxVal1 |= 0x10;
			muxVal2 = ch & 0x07;
			muxVal2 |= 0xF8;
			break;
		case 2: //sourceV
			muxVal1 = 0x14;
			ch = ch << 3;
			muxVal2 = ch & 0x18;
			break;
		case 3: //sourceI
			muxVal1 = 0x18;
			ch = ch << 5;
			muxVal2 = ch & 0x60;
			muxVal2 |= 0x80;
			break;
	}

	//capacitor discharge
	//outb(0x90, (addr + mux_en));
	outb(0x78, (addr + mux_cs1));
	outb(0x9C, (addr + mux_en));
	usleep(10);//40->10 khk 20060908

	outb(muxVal2, (addr + mux_cs1));
	outb(muxVal1, (addr + mux_en));
}

void SetMux_6(int type, int ch, int addr)
{
	int bd;
	int	mux_en, mux_cs1, mux_cs2;
    unsigned char muxVal1=0x00, muxVal2=0x00, muxVal3=0x00;

	//bits-   7   |   6   |   5   |   4   |   3   |   2   |   1   |   0   |
	//mux1- M_EN3 | M_EN2 | M_EN1 |   X   |   X   |   X   |   X   | CH_EN1|
	//mux2-   X   |   X   | M_A6  | M_A5  | M_A4  | M_A3  | M_A2  | M_A1  |
	//mux3-   X   |   X   |   X   |   X   |   X   |   X   | C_A2  | C_A1  |

	mux_en = myPs->addr.main[MUX_EN];
	mux_cs1 = myPs->addr.main[MUX_CS1];
	mux_cs2 = myPs->addr.main[MUX_CS2];
	switch(type) {
		case 0: //V
			bd = ch / 4;
			muxVal1 = 0x01;
			muxVal1 = muxVal1 << bd;
			muxVal1 |= 0x80;
			muxVal2 = 0x00;//Voltage
			muxVal3 = (ch%4) & 0x03;//Channel Select
			break;
		case 1: //I
			bd = ch / 4;
			muxVal1 = 0x01;
			muxVal1 = muxVal1 << bd;
			muxVal1 |= 0x80;
			muxVal2 = 0x10;//Current
			muxVal3 = (ch%4) & 0x03;//Channel Select
			break;
		case 2: //sourceV
			muxVal1 = 0xA0;
			ch = (ch % 4) & 0x03;
			muxVal2 = ch;
			break;
		case 3: //sourceI
			muxVal1 = 0xC0;
			ch = (ch % 4) & 0x03;
			muxVal2 = ch << 2;
			muxVal2 |= 0x10;
			break;
		default: break;
	}
	//capacitor discharge
	outb(0xE0, (addr + mux_en));
	outb(0x3F, (addr + mux_cs1));
	outb(0x00, (addr + mux_cs2));
	usleep(10);

	outb(muxVal1, (addr + mux_en));
	outb(muxVal2, (addr + mux_cs1));
	outb(muxVal3, (addr + mux_cs2));

}

void SetMux_7(int type, int ch, int addr)
{
	int bd;
	int	mux_en, mux_cs1, mux_cs2;
    unsigned char muxVal1=0x00, muxVal2=0x00, muxVal3=0x00;

	//bits-   7   |   6   |   5   |   4   |   3   |   2   |   1   |   0   |
	//mux1-   X   | M_EN3 | M_EN2 | M_EN1 |   X   |   X   |   X   | CH_EN1|
	//mux2-   X   |   X   |   X   | M_A5  | M_A4  | M_A3  | M_A2  | M_A1  |
	//mux3-   X   |   X   |   X   |   X   |   X   |  C_A3 | C_A2  | C_A1  |

	mux_en = myPs->addr.main[MUX_EN];
	mux_cs1 = myPs->addr.main[MUX_CS1];
	mux_cs2 = myPs->addr.main[MUX_CS2];
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
	outb(0x90, (addr + mux_en));
	outb(0x3F, (addr + mux_cs1));
	outb(0x00, (addr + mux_cs2));
	usleep(10);

	outb(muxVal1, (addr + mux_en));
	outb(muxVal2, (addr + mux_cs1));
	outb(muxVal3, (addr + mux_cs2));
}

void SetMux_8(int type, int ch, int addr)
{
	int bd;
	int	mux_en, mux_cs1, mux_cs2;
    unsigned char muxVal1=0x00, muxVal2=0x00, muxVal3=0x00;

	//bits-   7   |   6   |   5   |   4   |   3   |   2   |   1   |   0   |
	//mux1-   X   | M_EN3 | M_EN2 | M_EN1 | CH_EN4| CH_EN3| CH_EN2| CH_EN1|
	//mux2-   X   |   X   | M_A6  | M_A5  | M_A4  | M_A3  | M_A2  | M_A1  |
	//mux3-   X   |   X   |   X   |   X   |   X   | C_A3  | C_A2  | C_A1  |
	mux_en = myPs->addr.main[MUX_EN];
	mux_cs1 = myPs->addr.main[MUX_CS1];
	mux_cs2 = myPs->addr.main[MUX_CS2];
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
//120305
//set mux w kji
//enable before address setting
	//capacitor discharge
	outb(0x00, (addr + mux_en));
	outb(0x3F, (addr + mux_cs1));
	outb(0x70, (addr + mux_en));
	
	usleep(10);
	
	outb(0x00, (addr + mux_en));
	outb(muxVal3, (addr + mux_cs2));
	outb(muxVal2, (addr + mux_cs1));
	outb(muxVal1, (addr + mux_en));
}


void SetMux_9(int type, int ch, int addr)
{
	int bd;
	int	mux_en, mux_cs1, mux_cs2;
    unsigned char muxVal1=0x00, muxVal2=0x00, muxVal3=0x00;

	//bits-   7   |   6   |   5   |   4   |   3   |   2   |   1   |   0   |
	//mux1-   X   | M_EN3 | M_EN2 | M_EN1 | CH_EN4| CH_EN3| CH_EN2| CH_EN1|
	//mux2-   X   |   X   | M_A6  | M_A5  | M_A4  | M_A3  | M_A2  | M_A1  |
	//mux3-   X   |   X   |   X   |   X   |   X   | C_A3  | C_A2  | C_A1  |

	mux_en = myPs->addr.main[MUX_EN];
	mux_cs1 = myPs->addr.main[MUX_CS1];
	mux_cs2 = myPs->addr.main[MUX_CS2];
	switch(type) {
		case 0: //V
			bd = ch / 4;
			muxVal1 = 0x01;
			muxVal1 = muxVal1 << bd;
			muxVal1 |= 0x40;
			muxVal2 = 0x00;//Voltage
			muxVal3 = (ch%4) & 0x03;//Channel Select
			break;
		case 1: //I
			bd = ch / 4;
			muxVal1 = 0x01;
			muxVal1 = muxVal1 << bd;
			muxVal1 |= 0x40;
			muxVal2 = 0x10;//Current
			muxVal3 = (ch%4) & 0x03;//Channel Select
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
//	outb(0x70, (addr + mux_en));
//	outb(0x3F, (addr + mux_cs1));
//	outb(0x00, (addr + mux_cs2));
	
//	usleep(10);
	
//	outb(0x00, (addr + mux_en));
//	outb(0x00, (addr + mux_cs1));
//	outb(0x00, (addr + mux_cs2));

	outb(muxVal1, (addr + mux_en));
	outb(muxVal2, (addr + mux_cs1));
	outb(muxVal3, (addr + mux_cs2));
}

void SetMux_10(int type, int ch, int addr)
{
	int bd,tmp;
	int	mux_en, mux_cs1, mux_cs2;
    unsigned char muxVal1=0x00, muxVal2=0x00, muxVal3=0x00;

	//bits-   7   |   6   |   5   |   4   |   3   |   2   |   1   |   0   |
	//mux1-   X   | M_EN3 | M_EN2 | M_EN1 | CH_EN4| CH_EN3| CH_EN2| CH_EN1|
	//mux2-   X   |   X   | M_A6  | M_A5  | M_A4  | M_A3  | M_A2  | M_A1  |
	//mux3-   X   |   X   |   X   |   X   |   X   | C_A3  | C_A2  | C_A1  |
	mux_en = myPs->addr.main[MUX_EN];
	mux_cs1 = myPs->addr.main[MUX_CS1];
	mux_cs2 = myPs->addr.main[MUX_CS2];
	switch(type) {
		case 0: //V
			bd = ch / 8;
			muxVal1 = 0x01;
			muxVal1 = muxVal1 << bd;
			muxVal2 = 0x00; //
			muxVal3 = (ch%8) & 0x07;//Channel Select
			break;
		case 1: //sourceV
			muxVal1 = 0x30;
			ch = (ch % 4) & 0x03;
			muxVal2 = ch;
			tmp = ch << 2;
			muxVal2 |= tmp;
			break;
		default: break;
	}
	//capacitor discharge
	
	outb(0x00, (addr + mux_en));		//Mux Disable(Ch,Ref)
	outb(0x0F, (addr + mux_cs1));	//ref Add Select
	outb(muxVal3, (addr + mux_cs2));  //Ch select
	outb(0x30, (addr + mux_en));	//Mux Enable(ref)
	usleep(10);
	
//T___Mux All Off
	outb(0x00, (addr + mux_en)); //Mux Disable(Ch,Ref)
	outb(muxVal2, (addr + mux_cs1)); //ref Add select
	outb(muxVal3, (addr + mux_cs2));  //Ch select
//T___Mux All Off
	outb(muxVal1, (addr + mux_en));	//Mux Enable(Ch)
}

void SetMux_Off(int bd, int ch, int addr)
{
	int	mux_en, mux_cs1, mux_cs2;

	//bits-   7   |   6   |   5   |   4   |   3   |   2   |   1   |   0   |
	//mux1-   X   | M_EN3 | M_EN2 | M_EN1 | CH_EN4| CH_EN3| CH_EN2| CH_EN1|
	//mux2-   X   |   X   | M_A6  | M_A5  | M_A4  | M_A3  | M_A2  | M_A1  |
	//mux3-   X   |   X   |   X   |   X   |   X   | C_A3  | C_A2  | C_A1  |
	mux_en = myPs->addr.main[MUX_EN];
	mux_cs1 = myPs->addr.main[MUX_CS1];
	mux_cs2 = myPs->addr.main[MUX_CS2];
	
	outb(0x00, (addr + mux_en));
	outb(0x00, (addr + mux_cs1));
	outb(0x00, (addr + mux_cs2));

}

void SetMux_11(int type, int ch)
{
	int	mux_en, mux_cs1, mux_cs2, base_addr, addr;
    unsigned char muxVal1=0x00, muxVal2=0x00, muxVal3=0x00;

//bits-   	 7   |   6     |   5   |   4   |   3   |   2   |   1   |   0   |
//mux2-   C_A8   |   C_A7  | C_A6  | C_A5  | C_A4  | C_A3  | C_A2  | C_A1  |
//mux3-   M_A8   |   M_A7  | M_A6  | M_A5  | M_A4  | M_A3  | M_A2  | M_A1  |
	base_addr = myPs->addr.main[BASE_ADDR];
	mux_en = myPs->addr.main[MUX_EN];
	mux_cs1 = myPs->addr.main[MUX_CS1];
	mux_cs2 = myPs->addr.main[MUX_CS2];
	switch(type) {
		case 0: //V, I, TEMP, PV
			muxVal1 = 0x00;
			muxVal2 = (unsigned char)(ch % 4); //Channel Select
			muxVal3 = 0xFF;
			break;
		case 1: //sourceV, SourceI
			if(ch >=3 )return;
			muxVal1 = 0x00;
			muxVal2 = 0x00; //Channel Select
			muxVal3 = (unsigned char)(ch & 0x02);
			break;
		default: break;
	}

	addr = base_addr;

	//capacitor discharge
	outb(0xAA, (addr + mux_cs2));
	
	usleep(10);
	
	outb(muxVal2, (addr + mux_cs1));
	outb(muxVal3, (addr + mux_cs2));
}

//pms add for MULTI
void SetMux_12(int type, int ch, int addr)
{
	int bd;
//	int	mux_en, mux_cs1, mux_cs2;
	int mux_en;
//  unsigned char muxVal1=0x00, muxVal2=0x00, muxVal3=0x00;
	unsigned char muxVal1 = 0x00;
	//bits-   7   |   6   |   5   |   4   |   3   |   2   |   1   |   0   |
	//625 -   X   | M_EN1 | M_A2  | M_A1  | 	  | CH_EN1| CH_A2 | CH_A1 |
	//62A - OUT4  | OUT3  | OUT2  | OUT1  |  CH4  |  CH3  |  CH2  |  CH1  |
	
	mux_en = myPs->addr.main[MUX_EN];

	//	mux_cs1 = myPs->addr.main[MUX_CS1];
//	mux_cs2 = myPs->addr.main[MUX_CS2];
	
	switch(type) {
		case 0: //Va
			muxVal1 = 0x04;
			muxVal1 = muxVal1 + ch;
			break;	
		case 1: //sourceV
			muxVal1 = 0x40;
			bd = ch << 4;
			muxVal1 = muxVal1 + bd;
			break;
		default: break;
	}

	//capacitor discharge
	outb(0x70, (addr + mux_en));

	usleep(10);
	
	outb(muxVal1, (addr + mux_en));
//	outb(muxVal2, (addr + mux_cs1));
//	outb(muxVal3, (addr + mux_cs2));
}

//end of add
void SetMux_13(int type, int ch, int addr)
{
	int bd,tmp;
	int	mux_en, mux_cs1, mux_cs2;
    unsigned char muxVal1=0x00, muxVal2=0x00, muxVal3=0x00;

	//bits-   7   |   6   |   5   |   4   |   3   |   2   |   1   |   0   |
	//mux1- CH_EN8| CH_EN7| CH_EN6| CH_EN5| CH_EN4| CH_EN3| CH_EN2| CH_EN1|
	//mux2-   X   |   X   |   X   |   X   |   X   | CH_A3 | CH_A2 | CH_A1 |
	//mux3-   X   |   X   | M_EN2 | M_EN1 | MUX_A4| MUX_A3| MUX_A2| MUX_A1|
	mux_en = myPs->addr.main[MUX_EN];
	mux_cs1 = myPs->addr.main[MUX_CS1];
	mux_cs2 = myPs->addr.main[MUX_CS2];

	switch(type) {
		case 0: //V
			bd = ch / 8;
			muxVal1 = 0x01;
			muxVal1 = muxVal1 << bd;
			muxVal3 = 0x00; //
			muxVal2 = (ch%8) & 0x07;//Channel Select
			break;
		case 1: //sourceV
			muxVal1 = 0x00;
			muxVal2 = 0x00;
			muxVal3 = 0x30;
			ch = (ch % 4) & 0x03;
			muxVal3 |= ch;
			tmp = ch << 2;
			muxVal3 |= tmp;
			break;
		default: break;
	}

	//capacitor discharge
	outb(0x00, (addr + mux_en));		//Mux Disable(Ch,Ref)
	outb(0x0F, (addr + mux_cs2));	//Mux Enable(ref)
	outb(0x3F, (addr + mux_cs2));	//Mux Enable(ref)
	usleep(5);
	outb(0x00, (addr + mux_cs2));	//Mux Enable(ref)

	outb(muxVal2, (addr + mux_cs1));  //Ch select
	outb(muxVal3, (addr + mux_cs2)); //ref Add select
	
//T___Mux All Off
//outb(0x00, (addr + mux_en)); //Mux Disable(Ch,Ref)
//T___Mux All Off
	outb(muxVal1, (addr + mux_en));	//Mux Enable(Ch)
}

void SetMux_14(int type, int ch, int addr)
{
	int bd,tmp;
	int	mux_en, mux_cs1, mux_cs2;
    unsigned char muxVal1=0x00, muxVal2=0x00, muxVal3=0x00;

	//bits-   7   |   6   |   5   |   4   |   3   |   2   |   1   |   0   |
	//mux1- CH_EN8| CH_EN7| CH_EN6| CH_EN5| CH_EN4| CH_EN3| CH_EN2| CH_EN1|
	//mux2-   X   |   X   |   X   |   X   |   X   | CH_A3 | CH_A2 | CH_A1 |
	//mux3-   X   |   X   | M_EN2 | M_EN1 | MUX_A4| MUX_A3| MUX_A2| MUX_A1|
			
	mux_en = myPs->addr.main[MUX_EN];
	mux_cs1 = myPs->addr.main[MUX_CS1];
	mux_cs2 = myPs->addr.main[MUX_CS2];
	switch(type) {
		case 0: //V
			bd = (ch / 8) * 2;
			muxVal1 = 0x01;
			muxVal1 = muxVal1 << bd;
			muxVal3 = 0x00; //
			muxVal2 = (ch%8) & 0x07;//Channel Select
			break;
		case 1: //sourceV
			muxVal1 = 0x00;
			muxVal2 = 0x00;
			muxVal3 = 0x30;
			ch = (ch % 4) & 0x03;
			muxVal3 |= ch;
			tmp = ch << 2;
			muxVal3 |= tmp;
			break;
		default: break;
	}
	//capacitor discharge
	outb(0x00, (addr + mux_en));	//Mux Disable(Ch,Ref)
	outb(0x0F, (addr + mux_cs2));	//Mux Enable(ref)
	outb(0x3F, (addr + mux_cs2));	//Mux Enable(ref)
	usleep(10);

	outb(0x00, (addr + mux_cs2));	//Mux Enable(ref)

	outb(muxVal2, (addr + mux_cs1));  //Ch select
	
	outb(muxVal3, (addr + mux_cs2)); //ref Add select
	
//T___Mux All Off
//outb(0x00, (addr + mux_en)); //Mux Disable(Ch,Ref)
//T___Mux All Off
	outb(muxVal1, (addr + mux_en));	//Mux Enable(Ch)
}

void SetMux_15(int type, int ch, int addr)
{	//210510 add for 500mA/2uA Type Cycler
	int bd,tmp;
	int	mux_en, mux_cs1, mux_cs2;
    unsigned char muxVal1=0x00, muxVal2=0x00, muxVal3=0x00;

	//bits-   7   |   6   |   5   |   4   |   3   |   2   |   1   |   0   |
	//mux1- CH_EN8| CH_EN7| CH_EN6| CH_EN5| CH_EN4| CH_EN3| CH_EN2| CH_EN1|
	//mux2-   X   |   X   |   X   |   X   |   X   | CH_A3 | CH_A2 | CH_A1 |
	//mux3-   X   |   X   | M_EN2 | M_EN1 | MUX_A4| MUX_A3| MUX_A2| MUX_A1|
	mux_en = myPs->addr.main[MUX_EN];
	mux_cs1 = myPs->addr.main[MUX_CS1];
	mux_cs2 = myPs->addr.main[MUX_CS2];

	switch(type) {
		case 0: //V
			bd = ch / 8;
			muxVal1 = 0x01;
			muxVal1 = muxVal1 << bd;
			muxVal3 = 0x00; //
			muxVal2 = (ch%8) & 0x07;//Channel Select
			break;
		case 1: //sourceV
			muxVal1 = 0x00;
			muxVal2 = 0x00;
			muxVal3 = 0x30;
			ch = (ch % 4) & 0x03;
			muxVal3 |= ch;
			tmp = ch << 2;
			muxVal3 |= tmp;
			break;
		default: break;
	}

	//capacitor discharge
	outb(0x00, (addr + mux_en));		//Mux Disable(Ch,Ref)
	outb(0x0F, (addr + mux_cs2));		//Mux Enable(ref)
	outb(0x3F, (addr + mux_cs2));		//Mux Enable(ref)
	usleep(5);
	outb(0x00, (addr + mux_cs2));		//Mux Enable(ref)

	outb(muxVal2, (addr + mux_cs1));  	//Ch select
	outb(muxVal3, (addr + mux_cs2)); 	//ref Add select
	
//T___Mux All Off
//outb(0x00, (addr + mux_en)); //Mux Disable(Ch,Ref)
//T___Mux All Off
	outb(muxVal1, (addr + mux_en));	//Mux Enable(Ch)
}

void CalChAD_Filter(int mainSlot)
{
	int type, bd, ch, i;
	long min, max ;
	double sum;
	
	bd = mainSlot;
	if(bd >= myPs->config.installedBd) return;
	for(type = 0; type < 2; type++){
		for(ch = 0; ch <  myPs->config.chPerBd; ch++){
			sum = myData->bData[bd].cData[ch].misc.adValue[type][0];
			min = max = sum;
			sum = 0;
			for(i=0; i < MAX_AD_COUNT; i++) {
				sum += myData->bData[bd].cData[ch].misc.adValue[type][i];
				if(min > myData->bData[bd].cData[ch].misc.adValue[type][i]) {
						min = myData->bData[bd].cData[ch].misc.adValue[type][i];
				}
				if(max < myData->bData[bd].cData[ch].misc.adValue[type][i]) {
						max = myData->bData[bd].cData[ch].misc.adValue[type][i];
				}
			}
			sum = (double)(sum - min - max) / (double)(MAX_AD_COUNT - 2);
			ConvertAD_Data(bd, ch, type, sum);
		}
		CalSourceAD_Filter(type, bd);
	}
}

void CalChAD_Filter_Ch(int bd, int ch, int type)
{
//	if(myPs->config.ADC_type == COMM_TYPE){
		CalChAD_Filter_Ch_Default(bd, ch, type);
//	}else{
//		CalChAD_Filter_Ch_2(bd, ch, type);
//	}
}

void CalChAD_Filter_Ch_Default(int bd, int ch, int type)
{
	int i;
	long max,min;
	double sum;
	if(type <= 1) {
		if(bd >= myPs->config.installedBd) return;
		if(ch >= myPs->config.chPerBd) return;
	}
	sum = 0;
	max = myData->bData[bd].cData[ch].misc.adValue[type][0];
	min = max;
	for(i=0; i < MAX_AD_COUNT; i++) {
		sum += myData->bData[bd].cData[ch].misc.adValue[type][i];
		if(myData->bData[bd].cData[ch].misc.adValue[type][i] > max)
			max = myData->bData[bd].cData[ch].misc.adValue[type][i];
		if(myData->bData[bd].cData[ch].misc.adValue[type][i] < min)
			min = myData->bData[bd].cData[ch].misc.adValue[type][i];
	}
	sum = (sum - max - min) / (double)(MAX_AD_COUNT - 2);
	
	if(type == 0 || type == 1){
		ConvertAD_Data(bd, ch, type, sum);
	}else if(type == 2 || type == 3){
		CalSourceAD_Filter_Ch(bd, ch, type-2);
	}
}

void CalChAD_Filter_Ch_2(int bd, int ch, int type)
{
	int i;
	long max,min;
	double sum;
	if(type <= 1) {
		if(bd >= myPs->config.installedBd) return;
		if(ch >= myPs->config.chPerBd) return;
	}
	sum = 0;
	max = myData->bData[bd].cData[ch].misc.adValue[type][0];
	min = max;
	for(i=0; i < MAX_AD_COUNT; i++) {
		sum += myData->bData[bd].cData[ch].misc.adValue[type][i];
		if(myData->bData[bd].cData[ch].misc.adValue[type][i] > max)
			max = myData->bData[bd].cData[ch].misc.adValue[type][i];
		if(myData->bData[bd].cData[ch].misc.adValue[type][i] < min)
			min = myData->bData[bd].cData[ch].misc.adValue[type][i];
	}
	sum = (sum - max - min) / (double)(MAX_AD_COUNT - 2);
	
	if(type == 0 || type == 1){
		ConvertAD_Data(bd, ch, type, sum);
	}else if(type == 2 || type == 3){
		for(i = 0; i < myPs->config.installedBd; i++){
			CalSourceAD_Filter_Ch(i, ch, type-2);
		}
	}
}

void CalChAD_Filter_Ch_AD2(int bd, int ch, int type)
{
	int i;
	double sum, sum1;
	
	if(type == 0) {
		if(bd >= myPs->config.installedBd) return;
		if(ch >= myPs->config.chPerBd) return;
	}

	if(type == 0) { //ch
		sum = sum1 = 0.0;
		if(myPs->config.MainBdType == CPLD_TYPE) {	
			for(i=0; i < MAX_AD_COUNT; i++) {
				sum += myData->bData[bd].cData[ch].misc.adValue[type][i];
				sum1 += myData->bData[bd].cData[ch].misc.adValue[type + 1][i];
			}
			sum = sum / (double)MAX_AD_COUNT;
			sum1 = sum1 / (double)MAX_AD_COUNT;

		} else if(myPs->config.MainBdType == FPGA_TYPE) {	
			sum = myData->bData[bd].cData[ch].misc.adValue[type][0];
			sum1 = myData->bData[bd].cData[ch].misc.adValue[type + 1][0];
		}

		ConvertAD_Data(bd, ch, type, sum); //V
		ConvertAD_Data(bd, ch, type + 1, sum1); //I
	} else if(type == 1) { //source
		CalSourceAD_Filter_Ch(bd, ch, type - 1); //V
		CalSourceAD_Filter_Ch(bd, ch, type); //I
	}
}

void ConvertAD_Data(int bd, int ch, int type, double sum)
{
	int range, i, point, pointV, parallel_ch;
	double ratio=0.0, tmp, tmpV;

	if((ch % 2) == 0) { //kjg_180521
		parallel_ch = ch + 1;
	} else {
		parallel_ch = ch - 1;
	}

	switch(type) {
		case 0:
			/*kjg_180523 if(myData->bData[bd].cData[ch].misc.parallel_cycle_phase == P50) {
				range = (int)myData->bData[bd].cData[parallel_ch]
					.signal[C_SIG_V_RANGE];
			} else {
				range = (int)myData->bData[bd].cData[ch].signal[C_SIG_V_RANGE];
			}*/
			if(myData->bData[bd].cData[ch].misc.parallel_cycle_phase == P50) {
				range = (int)myData->bData[bd].cData[ch].signal[C_SIG_V_RANGE];
			} else {
				if(myData->bData[bd].cData[parallel_ch].misc
					.parallel_cycle_phase == P50) {
					range = (int)myData->bData[bd].cData[parallel_ch]
						.signal[C_SIG_V_RANGE];
				} else {
					range = (int)myData->bData[bd].cData[ch]
						.signal[C_SIG_V_RANGE];
				}
			}
			if(range <= 0) range = 0;
			else range -= 1;

			ratio = 305.1850948; //10000000uV/32767 = 305.1850948
			//voltage AD 081203 KJI
			switch(myPs->config.hwSpec) {
				case L_20V_5A_R1:
					ratio = ratio * (20.0 / 7.0);
					break;
				case L_5V_150A_R1:
					ratio /= 1.494;
					break;
				case L_10V_5A_R2:
					break;
				case S_5V_200A_75A_15A_AD2:	
					ratio = (double)ratio / myPs->config.voltageAmp 
						* myPs->config.adAmp;
					break;
				default :	
					ratio = (double)ratio / myPs->config.adAmp 
						* myPs->config.voltageAmp;
					break;
			}
			tmp = (double)sum * ratio;

			//091124 add kji auto v cali enable
			if(myData->mData.config.auto_v_cali == P1) {
				if(tmp >= 0.0) {
					tmp = tmp * myData->bData[bd].misc.Vsource_AD_a
						+ myData->bData[bd].misc.Vsource_AD_b;
				} else {
					tmp = tmp * myData->bData[bd].misc.Vsource_AD_a_N
						+ myData->bData[bd].misc.Vsource_AD_b_N;
				}
			}
			
			i = myData->bData[bd].cData[ch].misc.sensCount;
			point = cFindADCaliPoint(bd, ch, (long)tmp, type, range);

			if(myData->bData[bd].cData[ch].op.state == C_CALI
				&& myData->mData.cali_meas_type != MEAS) {
				if(myData->bData[bd].cData[ch].signal[C_SIG_CALI_PHASE] < P10) {
					myData->bData[bd].cData[ch].misc.sensSumV[i] = (long)tmp;
				} else {
					if(myData->CaliMeter.caliType == CALI_V) {
						myData->bData[bd].cData[ch].misc.sensSumV[i]
							= (long)(tmp * myData->cali.tmpData[bd][ch]
								.AD_A[type][range][point]
								+ myData->cali.tmpData[bd][ch]
								.AD_B[type][range][point]);
					} else {
						myData->bData[bd].cData[ch].misc.sensSumV[i]
							= (long)tmp;
					}
				}
			} else {
				myData->bData[bd].cData[ch].misc.sensSumV[i]
					= (long)(tmp
					* myData->cali.data[bd][ch].AD_A[type][range][point]
					+ myData->cali.data[bd][ch].AD_B[type][range][point]);
			}
			break;
		case 1:
			/*kjg_180523 if(myData->bData[bd].cData[ch].misc.parallel_cycle_phase == P50) {
				range = (int)myData->bData[bd].cData[parallel_ch]
					.signal[C_SIG_I_RANGE];
			} else {
				range = (int)myData->bData[bd].cData[ch].signal[C_SIG_I_RANGE];
			}*/
			if(myData->bData[bd].cData[ch].misc.parallel_cycle_phase == P50) {
				range = (int)myData->bData[bd].cData[ch].signal[C_SIG_I_RANGE];
			} else {
				if(myData->bData[bd].cData[parallel_ch].misc
					.parallel_cycle_phase == P50) {
					range = (int)myData->bData[bd].cData[parallel_ch]
						.signal[C_SIG_I_RANGE];
				} else {
					range = (int)myData->bData[bd].cData[ch]
						.signal[C_SIG_I_RANGE];
				}
			}
			if(range <= 0) range = 0;
			else range -= 1;

			switch(myPs->config.hwSpec) {
				case L_5V_150A_R1: //150000000uA / (32767*5428571uV/10000000uV)
					if(myPs->config.ratioCurrent == MICRO) { //uA
						if((range + 1) == RANGE1) {
							//150000000uA / (32767 * 5428571uV/10000000uV);
							ratio = 150000000.0 / (32767.0 * 0.5428571);
						}
					}
					break;
				case L_10V_5A_R2: //kji_080922
					if((range + 1) == RANGE1) {
						//5000000uA /(32767*5100000uA/10000000uA)
						ratio = 5000000.0 / (32767.0 * 0.5100000);
					} else if((range + 1) == RANGE2) {
						//500000uA /(32767*5100000uA/10000000uA)
						ratio = 500000.0 / (32767.0 * 0.5100000);
					}
					break;
				case S_5V_200A_75A_15A_AD2:
					//10000000uV/32767 = 305.1850948
					ratio = 305.1850948;
					ratio = (double)ratio / myPs->config.currentAmp;
					break;
				default:
					//current AD 081203 KJI
					if((range + 1) == RANGE1) {
						ratio = (double) myPs->config.maxCurrent[range]
							/ (32767.0 * (myPs->config.currentAmp / 100.0));
					} else if((range + 1) == RANGE2) {
						ratio = (double) myPs->config.maxCurrent[range]
							/ (32767.0 * (myPs->config.currentAmp / 100.0));
					} else if((range + 1) == RANGE3) {
						ratio = (double) myPs->config.maxCurrent[range]
							/ (32767.0 * (myPs->config.currentAmp / 100.0));
					} else if((range + 1) == RANGE4) {
						ratio = (double) myPs->config.maxCurrent[range]
							/ (32767.0 * (myPs->config.currentAmp / 100.0));
					}
					break; 
			}

			switch(myPs->config.hwSpec) {
				case L_5V_150A_R1:
					ratio /= 1.494;
					break;
				case L_10V_5A_R2:
					break;
				case L_5V_1A_R3:
				case L_5V_500mA_2uA_R4:
					break;
				default:
					ratio = (double)ratio / myPs->config.adAmp;
					break;
			}
			tmp = (double)sum * ratio;

			if(tmp >= 0.0) {
				tmp = tmp * myData->bData[bd].misc.Isource_AD_a
					+ myData->bData[bd].misc.Isource_AD_b;
			} else {
				tmp = tmp * myData->bData[bd].misc.Isource_AD_a_N
					+ myData->bData[bd].misc.Isource_AD_b_N;
			}

			switch(myPs->config.hwSpec) {
				case S_5V_200A_75A_15A_AD2:
					ratio = myPs->config.shunt[range]
						* myPs->config.maxCurrent[range]
						* myPs->config.gain[range];
					tmp = tmp / ratio * myPs->config.maxCurrent[range];
					break;
				default:
					break;
			}

			i = myData->bData[bd].cData[ch].misc.sensCount;
			point = cFindADCaliPoint(bd, ch, (long)tmp, type, range);

			if(myData->bData[bd].cData[ch].op.state == C_CALI
				&& myData->mData.cali_meas_type != MEAS) {
				if(myData->CaliMeter.caliType == CALI_V) {
					point = cFindADCaliPoint(bd, ch, (long)tmp, 4, range);
					tmp = tmp
						* myData->cali.data[bd][ch].AD_A[type][range][point]
						+ myData->cali.data[bd][ch].AD_B[type][range][point];

					if(myData->mData.config.function[F_I_OFFSET_CALI] == P1) {
						if(myData->bData[bd].cData[ch].signal[C_SIG_CALI_PHASE]
							< P10) {
						} else {
							tmpV = myData->bData[bd].cData[ch].op.Vsens;

							pointV = cFindADCaliPoint(bd, ch, (long)tmpV, 3, 0);
							tmp += (tmpV
								* myData->cali.tmpData_caliMeter2[bd][ch]
								.AD_A[type][range][pointV]
								+ myData->cali.tmpData_caliMeter2[bd][ch]
								.AD_B[type][range][pointV]);
						}
					}
					//kjg_180521 myData->bData[bd].cData[ch].misc.sensSumI[i] = (long)tmp;
				} else {
					if(myData->bData[bd].cData[ch].signal[C_SIG_CALI_PHASE]
						< P30) {
					} else {
						tmp = (tmp * myData->cali.tmpData[bd][ch]
							.AD_A[type][range][point]
						+ myData->cali.tmpData[bd][ch]
							.AD_B[type][range][point]);
					}
				}
				myData->bData[bd].cData[ch].misc.sensSumI[i] = (long)tmp;
			} else {
				if(myData->mData.config.function[F_I_OFFSET_CALI] == P1) {
					tmp = tmp 
						* myData->cali.data[bd][ch].AD_A[type][range][point]
						+ myData->cali.data[bd][ch].AD_B[type][range][point];

					tmpV = myData->bData[bd].cData[ch].op.Vsens;
					pointV = cFindADCaliPoint(bd, ch, (long)tmpV, 3, 0);

					tmpV -= myData->cali.line_impedance * tmp; //20160315khk add
					tmp += ((double)tmpV
						* myData->cali.data_caliMeter2[bd][ch]
						.AD_A[type][range][pointV]
						+ myData->cali.data_caliMeter2[bd][ch]
						.AD_B[type][range][pointV]);
				} else {
					tmp = tmp
						* myData->cali.data[bd][ch].AD_A[type][range][point]
						+ myData->cali.data[bd][ch].AD_B[type][range][point];
				}
				myData->bData[bd].cData[ch].misc.sensSumI[i] = (long)tmp;

				if(myPs->config.capacityType == CAPACITY_AMPARE_HOURS) {
					if(myData->bData[bd].cData[ch].op.type == STEP_CHARGE) {
						myData->bData[bd].cData[ch].misc.sensSumI[i]
							+= (long)(myPs->config.maxCurrent[range]
							* myPs->config.AD_offset);
					}
				}
			}
			break;
		default:
			break;
	}
}

void ConvertAD_Data_CAN(int bd, int ch, int type, double sum)
{
	int range, i, point, pointV, parallel_ch;
	double tmp, tmpV;

	if((ch % 2) == 0) { //kjg_180521
		parallel_ch = ch + 1;
	} else {
		parallel_ch = ch - 1;
	}

	switch(type) {
		case 0:
			/*kjg_180523 if(myData->bData[bd].cData[ch].misc.parallel_cycle_phase == P50) {
				range = (int)myData->bData[bd].cData[parallel_ch]
					.signal[C_SIG_V_RANGE];
			} else {
				range = (int)myData->bData[bd].cData[ch].signal[C_SIG_V_RANGE];
			}*/
			if(myData->bData[bd].cData[ch].misc.parallel_cycle_phase == P50) {
				range = (int)myData->bData[bd].cData[ch].signal[C_SIG_V_RANGE];
			} else {
				if(myData->bData[bd].cData[parallel_ch].misc
					.parallel_cycle_phase == P50) {
					range = (int)myData->bData[bd].cData[parallel_ch]
						.signal[C_SIG_V_RANGE];
				} else {
					range = (int)myData->bData[bd].cData[ch]
						.signal[C_SIG_V_RANGE];
				}
			}
			if(range <= 0) range = 0;
			else range -= 1;

			tmp = (double)sum;

			i = myData->bData[bd].cData[ch].misc.sensCount;
			point = cFindADCaliPoint(bd, ch, (long)tmp, type, range);

			if(myData->bData[bd].cData[ch].op.state == C_CALI
				&& myData->mData.cali_meas_type != MEAS) {
				if(myData->bData[bd].cData[ch].signal[C_SIG_CALI_PHASE] < P10) {
					myData->bData[bd].cData[ch].misc.sensSumV[i] = (long)tmp;
				} else {
					if(myData->CaliMeter.caliType == CALI_V) {
						myData->bData[bd].cData[ch].misc.sensSumV[i]
							= (long)(tmp * myData->cali.tmpData[bd][ch]
								.AD_A[type][range][point]
								+ myData->cali.tmpData[bd][ch]
								.AD_B[type][range][point]);
					} else {
						myData->bData[bd].cData[ch].misc.sensSumV[i]
							= (long)tmp;
					}
				}
			} else {
				myData->bData[bd].cData[ch].misc.sensSumV[i]
					= (long)(tmp
					* myData->cali.data[bd][ch].AD_A[type][range][point]
					+ myData->cali.data[bd][ch].AD_B[type][range][point]);
			}
			break;
		case 1:
			/*kjg_180523 if(myData->bData[bd].cData[ch].misc.parallel_cycle_phase == P50) {
				range = (int)myData->bData[bd].cData[parallel_ch]
					.signal[C_SIG_I_RANGE];
			} else {
				range = (int)myData->bData[bd].cData[ch].signal[C_SIG_I_RANGE];
			}*/
			if(myData->bData[bd].cData[ch].misc.parallel_cycle_phase == P50) {
				range = (int)myData->bData[bd].cData[ch].signal[C_SIG_I_RANGE];
			} else {
				if(myData->bData[bd].cData[parallel_ch].misc
					.parallel_cycle_phase == P50) {
					range = (int)myData->bData[bd].cData[parallel_ch]
						.signal[C_SIG_I_RANGE];
				} else {
					range = (int)myData->bData[bd].cData[ch]
						.signal[C_SIG_I_RANGE];
				}
			}
			if(range <= 0) range = 0;
			else range -= 1;

			tmp = (double)sum;

			i = myData->bData[bd].cData[ch].misc.sensCount;
			point = cFindADCaliPoint(bd, ch, (long)tmp, type, range);

			if(myData->bData[bd].cData[ch].op.state == C_CALI
				&& myData->mData.cali_meas_type != MEAS) {
				if(myData->CaliMeter.caliType == CALI_V) {
					point = cFindADCaliPoint(bd, ch, (long)tmp, 4, range);
					tmp = tmp
						* myData->cali.data[bd][ch].AD_A[type][range][point]
						+ myData->cali.data[bd][ch].AD_B[type][range][point];

					if(myData->mData.config.function[F_I_OFFSET_CALI] == P1) {
						if(myData->bData[bd].cData[ch].signal[C_SIG_CALI_PHASE]
							< P10) {
						} else {
							tmpV = myData->bData[bd].cData[ch].op.Vsens;

							pointV = cFindADCaliPoint(bd, ch, (long)tmpV, 3, 0);
							tmp += (tmpV
								* myData->cali.tmpData_caliMeter2[bd][ch]
								.AD_A[type][range][pointV]
								+ myData->cali.tmpData_caliMeter2[bd][ch]
								.AD_B[type][range][pointV]);
						}
					}
					//kjg_180521 myData->bData[bd].cData[ch].misc.sensSumI[i] = (long)tmp;
				} else {
					if(myData->bData[bd].cData[ch].signal[C_SIG_CALI_PHASE]
						< P30) {
					} else {
						tmp = (tmp * myData->cali.tmpData[bd][ch]
							.AD_A[type][range][point]
						+ myData->cali.tmpData[bd][ch]
							.AD_B[type][range][point]);
					}
				}
				myData->bData[bd].cData[ch].misc.sensSumI[i] = (long)tmp;
			} else {
				if(myData->mData.config.function[F_I_OFFSET_CALI] == P1) {
					tmp = tmp 
						* myData->cali.data[bd][ch].AD_A[type][range][point]
						+ myData->cali.data[bd][ch].AD_B[type][range][point];

					tmpV = myData->bData[bd].cData[ch].op.Vsens;
					pointV = cFindADCaliPoint(bd, ch, (long)tmpV, 3, 0);

					tmpV -= myData->cali.line_impedance * tmp; //20160315khk add
					tmp += ((double)tmpV
						* myData->cali.data_caliMeter2[bd][ch]
						.AD_A[type][range][pointV]
						+ myData->cali.data_caliMeter2[bd][ch]
						.AD_B[type][range][pointV]);
				} else {
					tmp = tmp
						* myData->cali.data[bd][ch].AD_A[type][range][point]
						+ myData->cali.data[bd][ch].AD_B[type][range][point];
				}
				myData->bData[bd].cData[ch].misc.sensSumI[i] = (long)tmp;

				if(myPs->config.capacityType == CAPACITY_AMPARE_HOURS) {
					if(myData->bData[bd].cData[ch].op.type == STEP_CHARGE) {
						myData->bData[bd].cData[ch].misc.sensSumI[i]
							+= (long)(myPs->config.maxCurrent[range]
							* myPs->config.AD_offset);
					}
				}
			}
			break;
		default:
			break;
	}
}

/*
void ConvertAD_Data(int bd, int ch, int type, double sum)
{
	int range, i, point;
	double ratio=0, tmp;
	switch(type) {
		case 0:
			range = (int)myData->bData[bd].cData[ch].signal[C_SIG_V_RANGE];
			if(range <= 0) range = 0;
			else range -= 1;
			ratio = 305.1850948; //10000000uV/32767 = 305.1850948
			
			switch(myPs->config.hwSpec){
				case L_50V_50A:
					ratio = (ratio*0.75)*(50.0/7.5);// AD 7.5V ==>  DA 50V
					break;
				case L_20V_25A:
					ratio = ratio*(20.0/7.0);// AD 7.V ==>  DA 20V
					break;
				case L_20V_10A_R1:
				case L_20V_50A_R2:
					ratio = ratio*(20.0/7.0);// AD 7.V ==>  AD 20V
					break;
				case L_5V_100A:
				case L_5V_100A_R1:
				case L_5V_100A_R1_EG:
				case L_5V_2A_R1:
				case L_5V_500A_R1:
				case L_5V_500A_R2:
				case L_5V_200A_R4:
				case L_5V_200A_R3:
				case L_5V_150A_R1:
				case L_5V_150A_R3:
				case L_5V_250A_R1:
				case L_5V_50A_R1:
				case L_5V_1000A_R1:
				case L_5V_1000A_R3:
				case L_5V_300A_R1:
				case L_5V_300A_R3:
				case L_5V_50A_R2:
					ratio /= 1.494;
					break;
				case L_5V_2A_R2:
				case L_10V_5A_R2:
					break;
				default:
					ratio /= 1.5;
					break;
			}
			
			tmp = (double)sum * ratio;
			if(tmp >= 0.0) {
				tmp = tmp * myData->bData[bd].misc.Vsource_AD_a
					+ myData->bData[bd].misc.Vsource_AD_b;
			} else {
				tmp = tmp * myData->bData[bd].misc.Vsource_AD_a_N
					+ myData->bData[bd].misc.Vsource_AD_b_N;
			}

			i = myData->bData[bd].cData[ch].misc.sensCount;
			point = cFindADCaliPoint(bd, ch, (long)tmp, type, range);
			if(myData->bData[bd].cData[ch].op.state == C_CALI) {
				if(myData->bData[bd].cData[ch].signal[C_SIG_CALI_PHASE] < P10) {
					myData->bData[bd].cData[ch].misc.sensSumV[i] = (long)tmp;
				} else {
					myData->bData[bd].cData[ch].misc.sensSumV[i]
						= (long)(tmp * myData->cali.tmpData[bd][ch]
							.AD_A[type][range][point]
						+ myData->cali.tmpData[bd][ch]
							.AD_B[type][range][point]);
				}
			} else {
				myData->bData[bd].cData[ch].misc.sensSumV[i]
					= (long)(tmp
					* myData->cali.data[bd][ch].AD_A[type][range][point]
					+ myData->cali.data[bd][ch].AD_B[type][range][point]);
			}
			break;
		case 1:
			range = (int)myData->bData[bd].cData[ch].signal[C_SIG_I_RANGE];
			if(range <= 0) range = 0;
			else range -= 1;
			switch(myPs->config.hwSpec) {
				case L_5V_10mA: //10000uA / (32767*5982353uV/10000000uV)
					if(myPs->config.ratioCurrent == MICRO) { //uA
						if((range+1) == RANGE1) {
							ratio = 0.5101422379;
						} else if((range+1) == RANGE2) {
							ratio = 0.05101422379;
						} else if((range+1) == RANGE3) {
							ratio = 0.005101422379;
						} else {
							ratio = 0.0005101422379;
						}
					} else { //nA
						if((range+1) == RANGE1) {
							ratio = 510.1422379;
						} else if((range+1) == RANGE2) {
							ratio = 51.01422379;
						} else if((range+1) == RANGE3) {
							ratio = 5.101422379;
						} else {
							ratio = 0.5101422379;
						}
					}
					break;
				case L_5V_2A:
				case L_5V_3A:
					ratio = 0.0;
					break;
				case L_5V_5A: //5000000uA / (32767*5610000uV/10000000uV)
				case L_6V_6A: //6000000uA / (32767*6732000uV/10000000uV)
					ratio = 272.0009757;
					break;
				case L_5V_20A:
					if((range+1) == RANGE1) {
							//20000000uA / (32767*5755555uV/10000000uV)
						ratio = 1060.4888486;
					} else if((range+1) == RANGE2) {
							//6000000uA / (32767*5698000uV/10000000uV)
						ratio = 321.3602261;
					}
					break;
				case L_5V_30A: //30000000uA / (32767*6400000uV/10000000uV)
					ratio = 1430.555132;
					break;
				case L_5V_5A_2:
					if(myPs->config.ratioCurrent == MICRO) { //uA
						if((range+1) == RANGE1) {
							//5000000uA / (32767*5610000uV/10000000uV)
							ratio = 272.0009757;
						} else if((range+1) == RANGE2) {
							//50000uA / (32767*5100000uV/10000000uV)
							ratio = 2.992010733;
						} else if((range+1) == RANGE3) {
							ratio = 0.2992010733;
						} else {
							ratio = 0.02992010733;
						}
					} else { //nA
						if((range+1) == RANGE1) {
							ratio = 272000.9757;
						} else if((range+1) == RANGE2) {
							ratio = 2992.010733;
						} else if((range+1) == RANGE3) {
							ratio = 299.2010733;
						} else {
							ratio = 29.92010733;
						}
					}
					break;
				case L_5V_200A: //200000000uA / (32767*9290909uV/10000000uV)
					ratio = 6569.542222;
					break;
				case S_5V_200A: //200000000uA / (32767*6450000uV/10000000uV)
					if(myPs->config.ratioCurrent == MICRO) { //uA
						if((range+1) == RANGE1) {
							ratio = 9463.103713;
						} else if((range+1) == RANGE2) {
							ratio = 946.3103713;
						} else if((range+1) == RANGE3) {
							ratio = 94.63103713;
						} else {
							ratio = 9.463103713;
						}
					} else { //nA
						if((range+1) == RANGE1) {
							ratio = 9463103.713;
						} else if((range+1) == RANGE2) {
							ratio = 946310.3713;
						} else if((range+1) == RANGE3) {
							ratio = 94631.03713;
						} else {
							ratio = 9463.103713;
						}
					}
					break;
				case L_2V_100A: //10000000uA / (32767*5982353uV/10000000uV)
				case L_5V_100A_R2: //10000000uA / (32767*5982353uV/10000000uV)
					if(myPs->config.ratioCurrent == MICRO) { //uA
							if((range+1) == RANGE1) {
							ratio = 5101.422379;
						} else if((range+1) == RANGE2) {
							ratio = 510.1422379;
						} else if((range+1) == RANGE3) {
							ratio = 51.01422379;
						} else {
							ratio = 5.101422379;
						}
					} else { //nA
						if((range+1) == RANGE1) {
							ratio = 5101422.379;
						} else if((range+1) == RANGE2) {
							ratio = 510142.2379;
						} else if((range+1) == RANGE3) {
							ratio = 51014.22379;
						} else {
							ratio = 5101.422379;
						}
					}
					break;
				case L_5V_50A:
					if(myPs->config.ratioCurrent == MICRO) { //uA
						if((range+1) == RANGE1) {
							//50000000uA / (32767*5982353uV/10000000uV)
							ratio = 2550.71119;
						} else if((range+1) == RANGE2) {
							//8000000uA / (32767*4785882uV/10000000uV)
							ratio = 510.1422806;
						} else if((range+1) == RANGE3) {
							ratio = 51.01422806;
						} else {
							ratio = 5.101422806;
						}
					} else { //nA
						if((range+1) == RANGE1) {
							ratio = 2550711.19;
						} else if((range+1) == RANGE2) {
							ratio = 510142.2806;
						} else if((range+1) == RANGE3) {
							ratio = 51014.22806;
						} else {
							ratio = 5101.422806;
						}
					}
					break;
				case L_50V_50A: //50000000uA / (32767*5982353uV/10000000uV)
					ratio = 2550.711190;
					break;
				case L_20V_25A: //25000000uA / (32767*6970000uV/10000000uV)
					if(myPs->config.ratioCurrent == MICRO) { //uA
						if((range+1) == RANGE1) {//25000000uA /( 32767 * 6979999uV/10000000uV);
							ratio = 25000000.0/(32767.0*0.697);
						}else if((range+1) == RANGE2) {//2500000uA /( 32767 * 6979999uV/10000000uV);
							ratio = 2500000.0/(32767.0*0.697);
						}
					}
					break;
				case L_5V_100A: //100000000uA / (32767*5040000uV/10000000uV)
					if(myPs->config.ratioCurrent == MICRO) { //uA
						if((range+1) == RANGE1) {//100000000uA /( 32767 * 5040000uV/10000000uV);
							ratio = 100000000.0/(32767.0*0.504);
						}else if((range+1) == RANGE2) {//10000000uA /( 32767 * 5040000uV/10000000uV);
							ratio = 10000000.0/(32767.0*0.504);
						}
					}
					break;
				case L_5V_200A_R2: //200000000uA / (32767*575555uV/10000000uV)
					if(myPs->config.ratioCurrent == MICRO) { //uA
						if((range+1) == RANGE1) {//200000000uA /( 32767 * 5755555uV/10000000uV);
							ratio = 200000000.0/(32767.0*0.575555);
						}else if((range+1) == RANGE2) {//20000000uA /( 32767 * 575555uV/10000000uV);
							ratio = 20000000.0/(32767.0*0.575555);
						}
					}
					break;
				case L_5V_30A_R1: //30000000uA / (32767*6009375uV/10000000uV)
					if(myPs->config.ratioCurrent == MICRO) { //uA
						if((range+1) == RANGE1) {//30000000uA /( 32767 * 6009375uV/10000000uV);
							ratio = 30000000.0/(32767.0*0.6009375);
						}
					}
					break;
				case L_5V_100A_R1: //100000000uA / (32767*6051800uV/10000000uV)
				case L_5V_100A_R1_EG: //100000000uA / (32767*6051800uV/10000000uV)
					if(myPs->config.ratioCurrent == MICRO) { //uA
						if((range+1) == RANGE1) {//100000000uA /( 32767 * 6051800uV/10000000uV);
							ratio = 100000000.0/(32767.0*0.605180);
						}
					}
					break;
				case L_5V_2A_R1: //2000000uA / (32767*7560000uV/10000000uV)
					if(myPs->config.ratioCurrent == MICRO) { //uA
						if((range+1) == RANGE1) {//200000000uA /( 32767 * 7560000uV/10000000uV);
							ratio = 2000000.0/(32767.0*0.756000);
						}
					}
					break;
				case L_5V_500A_R1: //500000000uA / (32767*9077710uV/10000000uV)
					if(myPs->config.ratioCurrent == MICRO) { //uA
						if((range+1) == RANGE1) {//500000000uA /( 32767 * 9077710uV/10000000uV);
							ratio = 500000000.0/(32767.0*0.907771);
						}
					}
					break;
				case L_5V_200A_R4: //200000000uA / (32767*9077710uV/10000000uV)
					if(myPs->config.ratioCurrent == MICRO) { //uA
						if((range+1) == RANGE1) {//200000000uA /( 32767 * 9041400uV/10000000uV);
							ratio = 200000000.0/(32767.0*0.605180);
						}else if((range+1) == RANGE2) {//100000000uA /( 32767 * 6051800uV/10000000uV);
							ratio = 100000000.0/(32767.0*0.605180);
						}else if((range+1) == RANGE3) {//50000000uA /( 32767 * 6051800uV/10000000uV);
							ratio = 50000000.0/(32767.0*0.605180);
						}else if((range+1) == RANGE4) {//8000000uA /( 32767 * 6051800uV/10000000uV);
							ratio = 8000000.0/(32767.0*0.605180);
						}
					}
					break;
				case L_5V_150A_R1: //150000000uA / (32767*5428571uV/10000000uV)
					if(myPs->config.ratioCurrent == MICRO) { //uA
						if((range+1) == RANGE1) {//150000000uA /( 32767 * 5428571uV/10000000uV);
							ratio = 150000000.0/(32767.0*0.5428571);
						}
					}
					break;
				case L_5V_250A_R1: //250000000uA / (32767*6051810uV/10000000uV)
					if(myPs->config.ratioCurrent == MICRO) { //uA
						if((range+1) == RANGE1) {//250000000uA /( 32767 * 6051810uV/10000000uV);
							ratio = 250000000.0/(32767.0*0.6051810);
						}
					}
					break;
				case L_5V_50A_R1: //50000000uA / (32767*5040000uV/10000000uV)
					if(myPs->config.ratioCurrent == MICRO) { //uA
						if((range+1) == RANGE1) {//50000000uA /( 32767 * 5040000uV/10000000uV);
							ratio = 50000000.0/(32767.0*0.5040000);
						}
					}
					break;
				case L_5V_1000A_R1: //1000000000uA / (32767*9077710uV/10000000uV)
					if(myPs->config.ratioCurrent == MICRO) { //uA
						if((range+1) == RANGE1) {//1000000000uA /( 32767 * 9077710uV/10000000uV);
							ratio = 1000000000.0/(32767.0*0.907771);
						}
					}
					break;
				case L_5V_300A_R1: //300000000uA / (32767*6051810uV/10000000uV)
					if(myPs->config.ratioCurrent == MICRO) { //uA
						if((range+1) == RANGE1) {//300000000uA /( 32767 * 6051810uV/10000000uV);
							ratio = 300000000.0/(32767.0*0.6051810);
						}
					}
					break;
				case L_20V_10A_R1:
					if((range+1) == RANGE1) {
							//10000000uA / (32767*6051807uV/10000000uV)
						ratio = 10000000.0/(32767*0.6051807);
					}
					break;
				case L_20V_50A_R2:
					if((range+1) == RANGE1) {
							//50000000uA / (32767*6051807uV/10000000uV)
						ratio = 50000000.0/(32767*0.6051807);
					} else if((range+1) == RANGE2) {
							//10000000uA / (32767*6051807uV/10000000uV)
						ratio = 10000000.0/(32767*0.6051807);
					}
					break;
				case L_5V_50A_R2: //kji_080820
					if((range+1) == RANGE1) {
							//50000000uA /(32767*5040000uA/10000000uA)
						ratio = 50000000.0/(32767*0.5040000);
					} else if((range+1) == RANGE2) {
							//5000000uA /(32767*5040000uA/10000000uA)
						ratio = 5000000.0/(32767*0.5040000);
					}
					break;
				case L_5V_2A_R2: //kji_080901
					if((range+1) == RANGE1) {
							//2000000uA /(32767*5100000uA/10000000uA)
						ratio = 2000000.0/(32767*0.5100000);
					} else if((range+1) == RANGE2) {
							//200000uA /(32767*5100000uA/10000000uA)
						ratio = 200000.0/(32767*0.5100000);
					}
					break;
				case L_5V_500A_R2:
					if((range+1) == RANGE1) {
							//500000000uA / (32767*6051807uV/10000000uV)
						ratio = 500000000.0/(32767*0.6051807);
					} else if((range+1) == RANGE2) {
							//50000000uA / (32767*6051807uV/10000000uV)
						ratio = 50000000.0/(32767*0.6051807);
					}
					break;
				case L_5V_200A_R3:
					if((range+1) == RANGE1) {
							//200000000uA / (32767*6051807uV/10000000uV)
						ratio = 200000000.0/(32767*0.6051807);
					} else if((range+1) == RANGE2) {
							//50000000uA / (32767*6051807uV/10000000uV)
						ratio = 50000000.0/(32767*0.6051807);
					} else if((range+1) == RANGE3) {
							//10000000uA / (32767*6051807uV/10000000uV)
						ratio = 10000000.0/(32767*0.6051807);
					}
					break;
				case L_5V_300A_R3:
					if((range+1) == RANGE1) {
							//300000000uA / (32767*6051807uV/10000000uV)
						ratio = 300000000.0/(32767*0.6051807);
					} else if((range+1) == RANGE2) {
							//50000000uA / (32767*6051807uV/10000000uV)
						ratio = 50000000.0/(32767*0.6051807);
					} else if((range+1) == RANGE3) {
							//10000000uA / (32767*6051807uV/10000000uV)
						ratio = 10000000.0/(32767*0.6051807);
					}
					break;
				case L_10V_5A_R2: //kji_080922
					if((range+1) == RANGE1) {
							//5000000uA /(32767*5100000uA/10000000uA)
						ratio = 5000000.0/(32767*0.5100000);
					} else if((range+1) == RANGE2) {
							//500000uA /(32767*5100000uA/10000000uA)
						ratio = 500000.0/(32767*0.5100000);
					}
					break;
				case L_5V_1000A_R3:
					if((range+1) == RANGE1) {
							//1000000000uA / (32767*6051807uV/10000000uV)
						ratio = 1000000000.0/(32767*0.6051807);
					} else if((range+1) == RANGE2) {
							//500000000uA / (32767*6051807uV/10000000uV)
						ratio = 500000000.0/(32767*0.6051807);
					} else if((range+1) == RANGE3) {
							//100000000uA / (32767*6051807uV/10000000uV)
						ratio = 100000000.0/(32767*0.6051807);
					}
					break;
				case L_5V_150A_R3:
					if((range+1) == RANGE1) {
							//150000000uA / (32767*6051807uV/10000000uV)
						ratio = 150000000.0/(32767*0.6051807);
					} else if((range+1) == RANGE2) {
							//50000000uA / (32767*6051807uV/10000000uV)
						ratio = 50000000.0/(32767*0.6051807);
					} else if((range+1) == RANGE3) {
							//10000000uA / (32767*6051807uV/10000000uV)
						ratio = 10000000.0/(32767*0.6051807);
					}
					break;

				default:	break;
			}
			switch(myPs->config.hwSpec){
				case S_5V_200A:
				case L_2V_100A:
				case L_5V_5A_2:
				case L_5V_20A:
				case L_5V_50A:
				case L_5V_200A_R2:
				case L_5V_100A_R2:
				case L_5V_30A_R1:
					ratio /= 1.5;
					break;
				case L_5V_100A:
				case L_5V_100A_R1:
				case L_5V_100A_R1_EG:
				case L_5V_2A_R1:
				case L_5V_500A_R1:
				case L_5V_500A_R2:
				case L_5V_200A_R4:
				case L_5V_200A_R3:
				case L_5V_150A_R1:
				case L_5V_150A_R3:
				case L_5V_250A_R1:
				case L_5V_50A_R1:
				case L_5V_1000A_R1:
				case L_5V_1000A_R3:
				case L_5V_300A_R1:
				case L_5V_300A_R3:
				case L_5V_50A_R2:
					ratio /= 1.494;
				default:
					break;
			}
		
			ratio = (double)ratio / myPs->config.adAmp;

			tmp = (double)sum * ratio;
			if(tmp >= 0.0) {
				tmp = tmp * myData->bData[bd].misc.Isource_AD_a
					+ myData->bData[bd].misc.Isource_AD_b;
			} else {
				tmp = tmp * myData->bData[bd].misc.Isource_AD_a_N
					+ myData->bData[bd].misc.Isource_AD_b_N;
			}
			i = myData->bData[bd].cData[ch].misc.sensCount;
			point = cFindADCaliPoint(bd, ch, (long)tmp, type, range);
			if(myData->bData[bd].cData[ch].op.state == C_CALI) {
				if(myData->bData[bd].cData[ch].signal[C_SIG_CALI_PHASE] < P30) {
					myData->bData[bd].cData[ch].misc.sensSumI[i] = (long)tmp;
				} else {
					myData->bData[bd].cData[ch].misc.sensSumI[i]
						= (long)(tmp * myData->cali.tmpData[bd][ch]
							.AD_A[type][range][point]
						+ myData->cali.tmpData[bd][ch]
							.AD_B[type][range][point]);
				}
			} else {
				myData->bData[bd].cData[ch].misc.sensSumI[i]
				= (long)(tmp
				* myData->cali.data[bd][ch].AD_A[type][range][point]
				+ myData->cali.data[bd][ch].AD_B[type][range][point]);
			}
			break;
		default: break;
	}
}
*/
void CalSourceAD_Filter(int type, int bd)
{
	int ch, i;
	double sum;//, min, max;

	for(ch=0; ch < 4; ch++) {
//		sum = (long)myData->bData[bd].misc.source[ch].adValue[type][0];
//		min = max = sum;
		sum = 0;
		for(i=0; i < MAX_AD_COUNT; i++) {
			sum += myData->bData[bd].misc.source[ch].adValue[type][i];
/*			if(min > myData->bData[bd].misc.source[ch].adValue[type][i]) {
					min = myData->bData[bd].misc.source[ch].adValue[type][i];
			}
			if(max < myData->bData[bd].misc.source[ch].adValue[type][i]) {
					max = myData->bData[bd].misc.source[ch].adValue[type][i];
			}*/
		}
//		sum = (sum - min - max) / (double)(MAX_AD_COUNT - 2);
		sum = sum / (double)MAX_AD_COUNT;
		i = myPs->misc.sensCount;
		if(type == 0){
			myData->bData[bd].misc.source[ch].sensSumV[i] = (long)sum;
		}else{
			myData->bData[bd].misc.source[ch].sensSumI[i] = (long)sum;
		}
	}
}

void CalSourceAD_Filter_Ch(int bd, int ch, int type)
{
	int i;
	double sum;

	sum = 0.0;
	if(myPs->config.MainBdType == CPLD_TYPE) {	
		for(i=0; i < MAX_AD_COUNT; i++) {
			sum += myData->bData[bd].misc.source[ch].adValue[type][i];
		}
		sum = sum / (double)MAX_AD_COUNT;
	} else if(myPs->config.MainBdType == FPGA_TYPE) {	
		sum = myData->bData[bd].misc.source[ch].adValue[type][0];
	}

	i = myPs->misc.sensCount;
	if(type == 0) {
		myData->bData[bd].misc.source[ch].sensSumV[i] = (long)sum;
		if(myPs->config.rt_scan_type == RT_SCAN_PERIOD_10mS) {
			if(ch == 1) {
				myData->bData[bd].misc.source[2].sensSumV[i]
					= myData->bData[bd].misc.source[2].adValue[type][0];
			}
		}
	} else {
		myData->bData[bd].misc.source[ch].sensSumI[i] = (long)sum;
		if(myPs->config.rt_scan_type == RT_SCAN_PERIOD_10mS) {
			if(ch == 1) {
				myData->bData[bd].misc.source[2].sensSumI[i]
					= myData->bData[bd].misc.source[2].adValue[type][0];
			}
		}
	}
}

void CalChAverage(int bd)
{
    int	ch;

	if(bd >= myPs->config.installedBd) return;

	for(ch=0; ch < myPs->config.chPerBd; ch++) {
		cCalculate_Voltage(bd, ch);
		cCalculate_Current(bd, ch);

		if(myData->bData[bd].cData[ch].op.type == STEP_REST) continue;

		cCalculate_Capacity(bd, ch);
		cCalculate_Watt(bd, ch);
	}
}

void CalChAverage_Ch(int bd, int ch, int type)
{
	if(bd >= myPs->config.installedBd) return;
	if(ch >= myPs->config.chPerBd) return;

	if(type == 0) {
		cCalculate_Voltage(bd, ch);
	} else if(type == 1) {
		cCalculate_Current(bd, ch);

		if(myData->bData[bd].cData[ch].op.type == STEP_REST) return;

		cCalculate_Capacity(bd, ch);
		cCalculate_Watt(bd, ch);
	}
}

void CalChAverage_Ch_AD2(int bd, int ch, int type)
{
	if(bd >= myPs->config.installedBd) return;
	if(ch >= myPs->config.chPerBd) return;

	if(type != 0) return;

	cCalculate_Voltage(bd, ch);
	cCalculate_Current(bd, ch);

	if(myData->bData[bd].cData[ch].op.type == STEP_REST) return;

	cCalculate_Capacity(bd, ch);
	cCalculate_Watt(bd, ch);
}

void CalChAverage_Ch_AD2p(int bd, int ch, int type)
{ //kjg_180521
	int div, parallel_ch;

	if(bd >= myPs->config.installedBd) return;
	if(ch >= myPs->config.chPerBd) return;

	if(type != 0) return;

	if((ch % 2) == 0) parallel_ch = ch + 1;
	else parallel_ch = ch - 1;

	//kjg_180523
	if(myData->bData[bd].cData[ch].misc.parallel_cycle_phase == P50) {
		if(myData->bData[bd].cData[ch].misc.parallel_sensFlag == P1) {
			myData->bData[bd].cData[ch].misc.parallel_sensFlag = P2;
			myData->bData[bd].cData[ch].misc.sensCount = 0;
			myData->bData[bd].cData[ch].misc.sensCountFlag = P0;
			myData->bData[bd].cData[ch].misc.sensBufCount = 0;
			myData->bData[bd].cData[ch].misc.sensBufCountFlag = P0;
		}

		if(myData->bData[bd].cData[parallel_ch].misc.parallel_sensFlag == P1) {
			myData->bData[bd].cData[parallel_ch].misc.parallel_sensFlag = P2;
			myData->bData[bd].cData[parallel_ch].misc.sensCount = 0;
			myData->bData[bd].cData[parallel_ch].misc.sensCountFlag = P0;
			myData->bData[bd].cData[parallel_ch].misc.sensBufCount = 0;
			myData->bData[bd].cData[parallel_ch].misc.sensBufCountFlag = P0;
		}
	} else {
		if(myData->bData[bd].cData[parallel_ch].misc.parallel_cycle_phase
			== P50) {
		} else {
			if(myData->bData[bd].cData[ch].misc.parallel_sensFlag == P2) {
				myData->bData[bd].cData[ch].misc.parallel_sensFlag = P0;
				myData->bData[bd].cData[ch].misc.sensCount = 0;
				myData->bData[bd].cData[ch].misc.sensCountFlag = P0;
				myData->bData[bd].cData[ch].misc.sensBufCount = 0;
				myData->bData[bd].cData[ch].misc.sensBufCountFlag = P0;
			}
		}
	}

	if((ch % 2) == 0) {
		if(myData->bData[bd].cData[ch].misc.parallel_cycle_phase == P50) {
			div = 1;
		} else if(myData->bData[bd].cData[parallel_ch].misc.parallel_cycle_phase
			== P50) {
			div = 2;
		} else {
			div = 0;
		}
	} else {
		if(myData->bData[bd].cData[ch].misc.parallel_cycle_phase == P50) {
			div = 3;
		} else if(myData->bData[bd].cData[parallel_ch].misc.parallel_cycle_phase
			== P50) {
			div = 4;
		} else {
			div = 0;
		}
	}

	if(div == 0) {
		cCalculate_Voltage(bd, ch);
		cCalculate_Current(bd, ch);

		if(myData->bData[bd].cData[ch].op.type == STEP_REST) return;

		cCalculate_Capacity(bd, ch);
		cCalculate_Watt(bd, ch);
	} else if(div == 1) {
		cCalculate_Voltage(bd, ch);
		cCalculate_Current_p(bd, ch, 1, parallel_ch);

		//if(myData->bData[bd].cData[ch].op.type == STEP_REST) return;

		//cCalculate_Capacity(bd, ch);
		//cCalculate_Watt(bd, ch);
	} else if(div == 2) {
		cCalculate_Voltage(bd, ch);
		cCalculate_Current_p(bd, ch, 2, parallel_ch);

		//if(myData->bData[bd].cData[parallel_ch].op.type == STEP_REST) return;

		//cCalculate_Capacity(bd, ch);
		//cCalculate_Watt(bd, ch);
	} else if(div == 3) {
		cCalculate_Voltage(bd, ch);
		cCalculate_Current_p(bd, ch, 3, parallel_ch);

		if(myData->bData[bd].cData[ch].op.type == STEP_REST) return;

		cCalculate_Capacity(bd, ch);
		cCalculate_Watt(bd, ch);
		//cCalculate_Capacity_p3(bd, ch);
		//cCalculate_Watt_p3(bd, ch);
	} else if(div == 4) {
		cCalculate_Voltage(bd, ch);
		cCalculate_Current_p(bd, ch, 4, parallel_ch);

		if(myData->bData[bd].cData[parallel_ch].op.type == STEP_REST) return;

		cCalculate_Capacity(bd, parallel_ch);
		cCalculate_Watt(bd, parallel_ch);
		//cCalculate_Capacity_p4(bd, parallel_ch);
		//cCalculate_Watt_p4(bd, parallel_ch);
	}
}

//20180410 modify
//20180717 modify data type((long->double)sum, list)
//20181030 modify filtering
//20190828 modify filtering(CP)
void cCalculate_Voltage(int bd, int ch)
{
    int	cnt, i, j, type = 0, flag;
    long tmpCount;
    double tmpV = 0, sum, list[MAX_FILTER_AD_COUNT];
	
	flag = myData->bData[bd].cData[ch].misc.sensCountFlag;
/*	
	if(myData->mData.config.function[F_SENS_COUNT_TYPE] == P2) {
		if(flag > P0) {
			switch(myPs->misc.rt_scan_time) {
				case P2:
					x = 2;
					break;
				case P5:
					x = 4;
					break;
				case P10:
				//	x = 6;
					x = 5;
					break;
				default:
				//	x = 2;
					x = 0;
					break;
			}
			tmpCount = MAX_FILTER_AD_COUNT - x;
		} else {
			tmpCount = myData->bData[bd].cData[ch].misc.sensCount+1;
		}
	} else {
		tmpCount = MAX_FILTER_AD_COUNT;
	}
*/
	if(flag > P0) {
		tmpCount = MAX_FILTER_AD_COUNT;
	} else {
		tmpCount = myData->bData[bd].cData[ch].misc.sensCount+1;
	}

	cnt = myData->bData[bd].cData[ch].misc.sensCount;

	if(flag == 0 && cnt == 0){
		for(i=1; i < MAX_FILTER_AD_COUNT; i++) {
			myData->bData[bd].cData[ch].misc.sensSumV[i] = 0;
		}
	}
	
	//190124 add lyhw
	tmpV = (double)myData->bData[bd].cData[ch].misc.sensSumV[cnt];
	myData->bData[bd].cData[ch].misc.tmpVsens = (long)tmpV;

	sum = 0;
	for(i=0; i < tmpCount; i++) {
		list[i] = (double)myData->bData[bd].cData[ch].misc.sensSumV[i];
		sum += (double)myData->bData[bd].cData[ch].misc.sensSumV[i];
	}
	
	for(i=0; i < (tmpCount - 1); i++) {
		for(j=(i + 1); j < tmpCount; j++) {
			if(list[i] < list[j]) {
				SWAP(&list[i], &list[j]);
			}
		}
	}
	
#if CYCLER_TYPE == LINEAR_CYC || CYCLER_TYPE == CAN_CYC
	if(flag > P0) {
		if(tmpCount >= 5) {
			sum = sum - list[0] - list[tmpCount-2] - list[tmpCount-1];
			tmpV = (double)sum / (tmpCount - 3);
		} else {
			tmpV = (double)sum / tmpCount;
		}
	}else{
		tmpV = (double)sum / tmpCount;
	}
#else
	tmpV = (double)sum / tmpCount;
#endif
	
	if(myData->bData[bd].cData[ch].op.state == C_CALI
		&& myData->mData.cali_meas_type != MEAS) {
		myData->cali.orgAD[type] = tmpV;
	}
	if(tmpV > 0) {
		if(tmpV / 1000000.0 >= 0.5)
			tmpV = floor(tmpV + 0.5);
	} else if(tmpV < 0) {
		if(tmpV / 1000000.0 <= -0.5)
			tmpV = ceil(tmpV - 0.5);
	}
//	bufCnt = myData->bData[bd].cData[ch].misc.sensBufCount;

//	myData->bData[bd].cData[ch].misc.tmpBufVsens[bufCnt] = (long)tmpV;
	//	= myData->bData[bd].cData[ch].misc.tmpVsens;
/*	
	sum = 0;
	tmpV = 0;
	for(i=0; i < tmpCount; i++) {
		list[i] = (double)myData->bData[bd].cData[ch].misc.tmpBufVsens[i];
		sum += (double)myData->bData[bd].cData[ch].misc.tmpBufVsens[i];
	}
	for(i=0; i < (tmpCount - 1); i++) {
		min = i;
		for(j=(i + 1); j < tmpCount; j++) {
			if(list[j] < list[i]) {
				SWAP(&list[i], &list[j]);
			}
		}
	}

	if(myData->bData[bd].cData[ch].misc.sensBufCountFlag == P1) {
		if(tmpCount > 5) {
			sum = sum - list[0] - list[1] - list[tmpCount - 2]
				- list[tmpCount - 1];
			tmpV = (double)sum / (tmpCount - 4);
		} else {
			tmpV = (double)sum / tmpCount;
		}
	} else {
		if(myData->mData.config.function[F_I_OFFSET_CALI] == P0) {
			for(i=0; i <= bufCnt; i++) {
				tmpV += (double)myData->bData[bd].cData[ch].misc.tmpBufVsens[i];
			}
			tmpV = tmpV / (double)(bufCnt + 1);
		} else {
	 	   	tmpV = myData->bData[bd].cData[ch].misc.tmpVsens;
		}
	}

	if(tmpV > 0) {
		tmpV = floor(tmpV + 0.5);
	} else if(tmpV < 0) {
		tmpV = ceil(tmpV - 0.5);
	}
*/
	myData->bData[bd].cData[ch].op.Vsens = (long)tmpV;

	if(myData->bData[bd].cData[ch].op.state == C_CALI
		&& myData->mData.cali_meas_type != MEAS) {
		myData->cali.orgAD[type] = (long)tmpV;
	}

	//120112 kji add debugmode 
	switch(myData->AppControl.config.debugType) {
		case P1:
			myData->bData[bd].cData[ch].op.Vsens = 3500000;
			myData->bData[bd].cData[ch].misc.tmpVsens = 3500000;
			myData->bData[bd].cData[ch].op.temp = 25000;
			break;
		case P2:
			cSimulation(bd, ch, 0, 0, 0);
			break;
		default: break;
	}
}

//20180410 modify
//20180717 modify data type((long->double)sum, list)
//20181030 modify filtering
//20190828 modify filtering(CP)
void cCalculate_Current(int bd, int ch)
{
	unsigned char refType;	
	int	cnt, i, j, type = 1, bufCnt, flag;
    long tmpCount, min;
	long refP, refI, mode;	
    double tmpI = 0, sum, list[MAX_FILTER_AD_COUNT];	
	S_CH_STEP_INFO	step;

	myCh = &(myData->bData[bd].cData[ch]);
	
	step = step_info(bd, ch);	

	mode = step.mode;
	refP = step.refP;
	refI = step.refI;

	flag = myData->bData[bd].cData[ch].misc.sensCountFlag;
/*
	if(myData->mData.config.function[F_SENS_COUNT_TYPE] >= P2) {
		switch(myPs->misc.rt_scan_time) {
			case P2:
				x = 2;
				break;
			case P5:
				x = 4;
				break;
			case P10:
				if(myData->mData.config.function[F_SENS_COUNT_TYPE] == P3) {
					x = 6;
				} else {
					x = 5;
				}
				break;
			default:
				x = 0;
				break;
		}
		tmpCount = MAX_FILTER_AD_COUNT - x;
	} else {
		tmpCount = MAX_FILTER_AD_COUNT;
	}
*/
	if(flag > P0) {
		tmpCount = MAX_FILTER_AD_COUNT;
	} else {
		tmpCount = myData->bData[bd].cData[ch].misc.sensCount+1;
	}

	cnt = myData->bData[bd].cData[ch].misc.sensCount;

	if(flag == 0 && cnt == 0){
		for(i=1; i < MAX_FILTER_AD_COUNT; i++) {
			myData->bData[bd].cData[ch].misc.sensSumI[i] = 0;
		}
	}	

	//190124 add lyhw
//	tmpI = (double)myData->bData[bd].cData[ch].misc.sensSumI[cnt];
//	myData->bData[bd].cData[ch].misc.tmpIsens = (long)tmpI;
	
	sum = 0;
	for(i=0; i < tmpCount; i++) {
		list[i] = (double)myData->bData[bd].cData[ch].misc.sensSumI[i];
		sum += (double)myData->bData[bd].cData[ch].misc.sensSumI[i];
	}
	
	for(i=0; i < (tmpCount - 1); i++) {
		min = i;
		for(j=(i + 1); j < tmpCount; j++) {
			if(list[i] < list[j]) {
				SWAP(&list[i], &list[j]);
			}
		}
	}
	
	//list[0] => max
	//200715 add for filltering
	refType = 0;
	if(mode == CC || mode == CCCV){
		if(refI < 0)	refType = 1;
		else			refType = 0;
	} 
	if(mode == CP || mode == CPCV){
		if(refP < 0)	refType = 1;
		else			refType = 0;
	}
			
	if(flag > P0) {
		if(tmpCount >= 5) {
			if(refType == 0){	
				sum = sum - list[0] - list[tmpCount-2] - list[tmpCount-1];
			}else{	
				sum = sum - list[0] - list[1] - list[tmpCount-1];
			}
			tmpI = (double)sum / (tmpCount - 3);
		} else {
			tmpI = (double)sum / tmpCount;
		}
	}else{
		tmpI = (double)myData->bData[bd].cData[ch].misc.sensSumI[cnt];
	}

	if(tmpI > 0) {
		if(tmpI / 1000000.0 >= 0.5)
			tmpI = floor(tmpI + 0.5);
	} else if(tmpI < 0) {
		if(tmpI / 1000000.0 <= -0.5)
			tmpI = ceil(tmpI - 0.5);
	}
	
//#if CYCLER_TYPE == DIGITAL_CYC		//hun_210907
	switch(myData->bData[bd].cData[ch].op.type) {
		case STEP_CHARGE:	if(tmpI < 0) tmpI = 0;	break;
		case STEP_DISCHARGE: if(tmpI > 0) tmpI = 0;	break;
		default: break;
	}
//#endif
	
	bufCnt = myData->bData[bd].cData[ch].misc.sensBufCount;
	flag = myData->bData[bd].cData[ch].misc.sensBufCountFlag;
	myData->bData[bd].cData[ch].misc.tmpBufIsens[bufCnt] = (long)tmpI;
	myData->bData[bd].cData[ch].misc.tmpIsens = (long)tmpI;
	/*	
 	switch(myPs->misc.rt_scan_time) {
	switch(myPs->config.rt_scan_type) { //191118
		case RT_SCAN_PERIOD_25mS:
		case RT_SCAN_PERIOD_20mS:
		case RT_SCAN_PERIOD_10mS:
			myData->bData[bd].cData[ch].misc.tmpIsens = (long)tmpI;
			break;
	}
	*/

	if(flag == 0 && bufCnt == 0){
		for(i=1; i < MAX_FILTER_AD_COUNT; i++) {
			myData->bData[bd].cData[ch].misc.tmpBufIsens[i] = 0; 
		}
	}

	sum = 0;
	tmpI = 0;
	for(i=0; i < tmpCount; i++) {
		list[i] = (double)myData->bData[bd].cData[ch].misc.tmpBufIsens[i]; 
		sum += (double)myData->bData[bd].cData[ch].misc.tmpBufIsens[i]; 
	}
	for(i=0; i < (tmpCount - 1); i++) {
		for(j=(i + 1); j < tmpCount; j++) {
			if(list[i] < list[j]) {
				SWAP(&list[i], &list[j]);
			}
		}
	}

	if(flag == P1) {
		if(tmpCount >= 5) {
			if(refType == 0){
				sum = sum - list[0] - list[tmpCount-2] - list[tmpCount-1];
			}else{
				sum = sum - list[0] - list[1] - list[tmpCount-1];
			}
			tmpI = (double)sum / (tmpCount - 3);
		} else {
			tmpI = (double)sum / tmpCount;
		}
	} else {
		if(myData->mData.config.function[F_I_OFFSET_CALI] == P1) {
		 	tmpI = myData->bData[bd].cData[ch].misc.tmpIsens;
		} else {
			for(i=0; i <= bufCnt; i++) {
				tmpI += (double)myData->bData[bd].cData[ch].misc.tmpBufIsens[i];
			}
			tmpI = tmpI / (double)(bufCnt + 1);
		}
	}

	if(tmpI > 0) {
		if(tmpI / 1000000.0 >= 0.5)
			tmpI = floor(tmpI + 0.5);
	} else if(tmpI < 0) {
		if(tmpI / 1000000.0 <= -0.5)
			tmpI = ceil(tmpI - 0.5);
	}			

	switch(myData->bData[bd].cData[ch].op.type) {
		case STEP_CHARGE:	if(tmpI < 0) tmpI = 0;		break;
		case STEP_DISCHARGE:	if(tmpI > 0) tmpI = 0;	break;
		default: break;
	}

	myData->bData[bd].cData[ch].op.Isens = (long)tmpI;

	if(myData->bData[bd].cData[ch].op.state == C_CALI
		&& myData->mData.cali_meas_type != MEAS) {
		myData->cali.orgAD[type] = (long)tmpI;
		myData->cali.orgAD_caliMeter2[type] = tmpI;
	}

	//120112 kji add debugmode 
	switch(myData->AppControl.config.debugType) {
		case P1:
			myData->bData[bd].cData[ch].op.Isens 
				= myData->bData[bd].cData[ch].misc.testRefI;
			myData->bData[bd].cData[ch].misc.tmpIsens 
				= myData->bData[bd].cData[ch].misc.testRefI;
			break;
		case P2:
			cSimulation(bd, ch, 0, 0, 1);
			break;
		default: break;
	}
}

//kjg_180521
//20180717 modify data type((long->double)sum, list)
//20181030 modify filtering
//20190828 modify filtering(CP)
void cCalculate_Current_p(int bd, int ch, int div, int parallel_ch)
{
	int	cnt, i, j, type = 1, bufCnt, flag;
    long tmpCount, min;
    double tmpI = 0, sum, list[MAX_FILTER_AD_COUNT];	

	flag = myData->bData[bd].cData[ch].misc.sensCountFlag;
/*
	if(myData->mData.config.function[F_SENS_COUNT_TYPE] >= P2) {
		switch(myPs->misc.rt_scan_time) {
			case P2:
				x = 2;
				break;
			case P5:
				x = 4;
				break;
			case P10:	
				if(myData->mData.config.function[F_SENS_COUNT_TYPE] == P3) {
					x = 6;
				} else {
					x = 5;
				}
				break;
			default:
				x = 0;
				break;
		}
		tmpCount = MAX_FILTER_AD_COUNT - x;
	} else {
		tmpCount = MAX_FILTER_AD_COUNT;
	}
*/	
	if(flag > P0) {
		tmpCount = MAX_FILTER_AD_COUNT;
	} else {
		tmpCount = myData->bData[bd].cData[ch].misc.sensCount+1;
	}

	cnt = myData->bData[bd].cData[ch].misc.sensCount;

	if(flag == 0 && cnt == 0){
		for(i=1; i < MAX_FILTER_AD_COUNT; i++) {
			myData->bData[bd].cData[ch].misc.sensSumI[i] = 0;
		}
	}

	sum = 0;
	for(i=0; i < tmpCount; i++) {
		list[i] = (double)myData->bData[bd].cData[ch].misc.sensSumI[i];
		sum += (double)myData->bData[bd].cData[ch].misc.sensSumI[i];
	}
	for(i=0; i < (tmpCount - 1); i++) {
		min = i;
		for(j=(i + 1); j < tmpCount; j++) {
			if(list[i] < list[j]) {
				SWAP(&list[i], &list[j]);
			}
		}
	}

	if(flag > P0) {
//		if(tmpCount > 5) {
		if(tmpCount >= 5) {
//			sum = sum - list[0] - list[1] - list[tmpCount - 2]
			sum = sum - list[0] - list[tmpCount - 2]
				- list[tmpCount - 1];
//			tmpI = (double)sum / (tmpCount - 4);
			tmpI = (double)sum / (tmpCount - 3);
		} else {
			tmpI = (double)sum / tmpCount;
		}
	} else {
		tmpI = (double)myData->bData[bd].cData[ch].misc.sensSumI[cnt];
	}	

	if(tmpI > 0) {
		if(tmpI / 1000000.0 >= 0.5)	
			tmpI = floor(tmpI + 0.5);
	} else if(tmpI < 0) {
		if(tmpI / 1000000.0 <= -0.5)	
			tmpI = ceil(tmpI - 0.5);
	}
	//myData->bData[bd].cData[ch].misc.tmpIsens = (long)tmpI;
	if(div == 1) {
		myData->bData[bd].cData[ch].misc.tmpIsens2 = (long)tmpI;
	} else if(div == 2) {
		myData->bData[bd].cData[ch].misc.tmpIsens2 = (long)tmpI;
	} else if(div == 3) {
		myData->bData[bd].cData[ch].misc.tmpIsens2 = (long)tmpI;
		myData->bData[bd].cData[ch].misc.tmpIsens
			= myData->bData[bd].cData[ch].misc.tmpIsens2
			+ myData->bData[bd].cData[parallel_ch].misc.tmpIsens2;
	} else if(div == 4) {
		myData->bData[bd].cData[ch].misc.tmpIsens2 = (long)tmpI;
		myData->bData[bd].cData[parallel_ch].misc.tmpIsens
			= myData->bData[bd].cData[ch].misc.tmpIsens2
			+ myData->bData[bd].cData[parallel_ch].misc.tmpIsens2;
	}
	
	bufCnt = myData->bData[bd].cData[ch].misc.sensBufCount;
	flag = myData->bData[bd].cData[ch].misc.sensBufCountFlag;

	//myData->bData[bd].cData[ch].misc.tmpBufIsens[bufCnt]
	//	 = myData->bData[bd].cData[ch].misc.tmpIsens;
	if(div == 1) {
		myData->bData[bd].cData[ch].misc.tmpBufIsens[bufCnt]
			 = myData->bData[bd].cData[ch].misc.tmpIsens2;
	} else if(div == 2) {
		myData->bData[bd].cData[ch].misc.tmpBufIsens[bufCnt]
			 = myData->bData[bd].cData[ch].misc.tmpIsens2;
	} else if(div == 3) {
		myData->bData[bd].cData[ch].misc.tmpBufIsens[bufCnt]
			 = myData->bData[bd].cData[ch].misc.tmpIsens2;
	} else if(div == 4) {
		myData->bData[bd].cData[ch].misc.tmpBufIsens[bufCnt]
			 = myData->bData[bd].cData[ch].misc.tmpIsens2;
	}

	if(flag == 0 && bufCnt == 0){
		for(i=1; i < MAX_FILTER_AD_COUNT; i++) {
			myData->bData[bd].cData[ch].misc.tmpBufIsens[i] = 0; 
		}
	}

	sum = 0;
	tmpI = 0;
	for(i=0; i < tmpCount; i++) {
		list[i] = (double)myData->bData[bd].cData[ch].misc.tmpBufIsens[i]; 
		sum += (double)myData->bData[bd].cData[ch].misc.tmpBufIsens[i]; 
	}
	for(i=0; i < (tmpCount - 1); i++) {
		for(j=(i + 1); j < tmpCount; j++) {
			if(list[i] < list[j]) {
				SWAP(&list[i], &list[j]);
			}
		}
	}

	if(flag == P1) {
	//	if(tmpCount > 5) {
		if(tmpCount >= 5) {
//			sum = sum - list[0] - list[1] - list[tmpCount - 2]
			sum = sum - list[0] - list[tmpCount - 2]
				- list[tmpCount - 1];
//			tmpI = (double)sum / (tmpCount - 4);
			tmpI = (double)sum / (tmpCount - 3);
		} else {
			tmpI = (double)sum / tmpCount;
		}
	} else {
		if(myData->mData.config.function[F_I_OFFSET_CALI] == P1) {
		 	//tmpI = myData->bData[bd].cData[ch].misc.tmpIsens;
			if(div == 1) {
		 		tmpI = myData->bData[bd].cData[ch].misc.tmpIsens2;
			} else if(div == 2) {
		 		tmpI = myData->bData[bd].cData[ch].misc.tmpIsens2;
			} else if(div == 3) {
		 		tmpI = myData->bData[bd].cData[ch].misc.tmpIsens2;
			} else if(div == 4) {
		 		tmpI = myData->bData[bd].cData[ch].misc.tmpIsens2;
			}
		} else {
			for(i=0; i <= bufCnt; i++) {
				tmpI += (double)myData->bData[bd].cData[ch].misc.tmpBufIsens[i];
			}
			tmpI = tmpI / (double)(bufCnt + 1);
		}
	}
	
	if(tmpI > 0) {
		if(tmpI / 1000000.0 >= 0.5)
			tmpI = floor(tmpI + 0.5);
	} else if(tmpI < 0) {
		if(tmpI / 1000000.0 <= -0.5)
			tmpI = ceil(tmpI - 0.5);
	}			
	
	switch(myData->bData[bd].cData[ch].op.type) {
		case STEP_CHARGE:
			if(tmpI < 0)
				tmpI = 0;
			break;
		case STEP_DISCHARGE:
			if(tmpI > 0)
				tmpI = 0;
			break;
		default: break;
	}

	//myData->bData[bd].cData[ch].op.Isens = (long)tmpI;
	if(div == 1) {
		myData->bData[bd].cData[ch].misc.Isens2 = (long)tmpI;
	} else if(div == 2) {
		myData->bData[bd].cData[ch].misc.Isens2 = (long)tmpI;
	} else if(div == 3) {
		myData->bData[bd].cData[ch].misc.Isens2 = (long)tmpI;
		myData->bData[bd].cData[ch].op.Isens
			= myData->bData[bd].cData[ch].misc.Isens2
			+ myData->bData[bd].cData[parallel_ch].misc.Isens2;
	} else if(div == 4) {
		myData->bData[bd].cData[ch].misc.Isens2 = (long)tmpI;
		myData->bData[bd].cData[parallel_ch].op.Isens
			= myData->bData[bd].cData[ch].misc.Isens2
			+ myData->bData[bd].cData[parallel_ch].misc.Isens2;
	}

	if(myData->bData[bd].cData[ch].op.state == C_CALI
		&& myData->mData.cali_meas_type != MEAS) {
		myData->cali.orgAD[type] = (long)tmpI;
		myData->cali.orgAD_caliMeter2[type] = tmpI;
	}

	//120112 kji add debugmode 
	switch(myData->AppControl.config.debugType) {
		case P1:
			myData->bData[bd].cData[ch].op.Isens 
				= myData->bData[bd].cData[ch].misc.testRefI;
			myData->bData[bd].cData[ch].misc.tmpIsens 
				= myData->bData[bd].cData[ch].misc.testRefI;
			break;
		case P2:
			cSimulation(bd, ch, div, parallel_ch, 2);
			break;
		default: break;
	}
}

void cCalculate_Capacity(int bd, int ch)
{
	long tmpI, flag;
	double acc_time;

	acc_time = 3600.0 * (10.0 / myPs->misc.rt_scan_time);
	
	if(myData->bData[bd].cData[ch].op.phase != P50) return;
	if(myData->bData[bd].cData[ch].misc.patternPhase != 0) return;
	if(myData->bData[bd].cData[ch].op.type == STEP_REST) return;

	//cal ampareHour
	//uA -> capacity 1uAh/div
	//nA -> capacity 1nAh/div
//	tmpI = myData->bData[bd].cData[ch].op.Isens;
	tmpI = myData->bData[bd].cData[ch].misc.tmpIsens / 10.0;

/*	if(myData->bData[bd].cData[ch].op.type == STEP_USER_PATTERN) {
		myData->bData[bd].cData[ch].misc.sumCapacity
			+= (long)(tmpI);
	} else {	
		myData->bData[bd].cData[ch].misc.sumCapacity
			+= labs(tmpI);
	}*/

	flag = myData->bData[bd].cData[ch].misc.endIntegralCFlag;
	if((flag == 1 && tmpI > 0) //flag = 1 Charge
		|| (flag == 2 && tmpI < 0) //flag = 2 DisCharge
		|| (flag == 3)) { //flag = 3 All
		myData->bData[bd].cData[ch].misc.sumintegralCapacity += (long)(tmpI);
	} else if(flag == 4) { //All Absolute Value Accumulate 170116 oys add
		myData->bData[bd].cData[ch].misc.sumintegralCapacity
			+= (long)labs(tmpI);
	}

	myData->bData[bd].cData[ch].op.integral_ampareHour
		= myData->bData[bd].cData[ch].misc.seedintegralCapacity
		+ (long)(myData->bData[bd].cData[ch].misc.sumintegralCapacity 
		/ acc_time);
	
	if(myData->bData[bd].cData[ch].misc.sumintegralCapacity > MAX_SUM
		|| myData->bData[bd].cData[ch].misc.sumintegralCapacity
		< ((-1) * MAX_SUM)) {
		myData->bData[bd].cData[ch].misc.seedintegralCapacity
			= myData->bData[bd].cData[ch].op.integral_ampareHour;
		myData->bData[bd].cData[ch].misc.sumintegralCapacity = 0L;
	}

 	if(tmpI > 0) { //charge_ampareHour
 		myData->bData[bd].cData[ch].misc.sumChargeAmpareHour += (long)tmpI;
 		myData->bData[bd].cData[ch].op.charge_ampareHour
			= myData->bData[bd].cData[ch].misc.seedChargeAmpareHour
			+ (long)(myData->bData[bd].cData[ch].misc.sumChargeAmpareHour
			/ acc_time);

		if(myData->bData[bd].cData[ch].misc.sumChargeAmpareHour > MAX_SUM 
			|| myData->bData[bd].cData[ch].misc.sumChargeAmpareHour 
			< ((-1) * MAX_SUM)) {
			myData->bData[bd].cData[ch].misc.seedChargeAmpareHour
				= myData->bData[bd].cData[ch].op.charge_ampareHour;
			myData->bData[bd].cData[ch].misc.sumChargeAmpareHour = 0L;
		}
	} else { //discharge_ampareHour
 		myData->bData[bd].cData[ch].misc.sumDischargeAmpareHour += labs(tmpI);
 		myData->bData[bd].cData[ch].op.discharge_ampareHour
			= myData->bData[bd].cData[ch].misc.seedDischargeAmpareHour
			+ (long)(myData->bData[bd].cData[ch].misc.sumDischargeAmpareHour
			/ acc_time);

		if(myData->bData[bd].cData[ch].misc.sumDischargeAmpareHour > MAX_SUM 
			|| myData->bData[bd].cData[ch].misc.sumDischargeAmpareHour
			< ((-1) * MAX_SUM)) {
			myData->bData[bd].cData[ch].misc.seedDischargeAmpareHour
				= myData->bData[bd].cData[ch].op.discharge_ampareHour;
			myData->bData[bd].cData[ch].misc.sumDischargeAmpareHour = 0L;
		}
	}

	if(myData->bData[bd].cData[ch].op.type == STEP_USER_PATTERN
		|| myData->bData[bd].cData[ch].op.type == STEP_USER_MAP) {
 		myData->bData[bd].cData[ch].op.ampareHour
			= (myData->bData[bd].cData[ch].op.charge_ampareHour
			- myData->bData[bd].cData[ch].op.discharge_ampareHour);
	} else {
 		myData->bData[bd].cData[ch].op.ampareHour
			= labs(myData->bData[bd].cData[ch].op.charge_ampareHour
			- myData->bData[bd].cData[ch].op.discharge_ampareHour);
	}

	//120818 kji SDI mes cycle data
	if(myData->bData[bd].cData[ch].op.type == STEP_CHARGE && tmpI > 0) {
	 	myData->bData[bd].cData[ch].misc.sumChargeCCCVAh += (long)tmpI;
		myData->bData[bd].cData[ch].misc.chargeCCCVAh
			= myData->bData[bd].cData[ch].misc.seedChargeCCCVAh
			+ (long)(myData->bData[bd].cData[ch].misc.sumChargeCCCVAh
			/ acc_time);

		if(myData->bData[bd].cData[ch].misc.sumChargeCCCVAh > MAX_SUM 
			|| myData->bData[bd].cData[ch].misc.sumChargeCCCVAh 
			< ((-1) * MAX_SUM)) {
			myData->bData[bd].cData[ch].misc.seedChargeCCCVAh
				= myData->bData[bd].cData[ch].misc.chargeCCCVAh;
			myData->bData[bd].cData[ch].misc.sumChargeCCCVAh = 0L;
		}

		if(myData->bData[bd].cData[ch].misc.cvTime == 0) {
	 		myData->bData[bd].cData[ch].misc.sumChargeCCAh += (long)tmpI;
			myData->bData[bd].cData[ch].misc.chargeCCAh
				= myData->bData[bd].cData[ch].misc.seedChargeCCAh
				+ (long)(myData->bData[bd].cData[ch].misc.sumChargeCCAh
				/ acc_time);

			if(myData->bData[bd].cData[ch].misc.sumChargeCCAh > MAX_SUM 
				|| myData->bData[bd].cData[ch].misc.sumChargeCCAh
				< ((-1) * MAX_SUM)) {
				myData->bData[bd].cData[ch].misc.seedChargeCCAh
					= myData->bData[bd].cData[ch].misc.chargeCCAh;
				myData->bData[bd].cData[ch].misc.sumChargeCCAh = 0L;
			}
		} else {
			myData->bData[bd].cData[ch].misc.chargeCVAh
				= myData->bData[bd].cData[ch].misc.chargeCCCVAh
				- myData->bData[bd].cData[ch].misc.chargeCCAh;
		}
	}

	if(myData->bData[bd].cData[ch].op.type == STEP_DISCHARGE && tmpI < 0) {
	 	myData->bData[bd].cData[ch].misc.sumDischargeCCCVAh += labs(tmpI);
		myData->bData[bd].cData[ch].misc.dischargeCCCVAh
			= myData->bData[bd].cData[ch].misc.seedDischargeCCCVAh
			+ (long)(myData->bData[bd].cData[ch].misc.sumDischargeCCCVAh
			/ acc_time);

		if(myData->bData[bd].cData[ch].misc.sumDischargeCCCVAh > MAX_SUM 
			|| myData->bData[bd].cData[ch].misc.sumDischargeCCCVAh 
			< ((-1) * MAX_SUM)) {
			myData->bData[bd].cData[ch].misc.seedDischargeCCCVAh
				= myData->bData[bd].cData[ch].misc.dischargeCCCVAh;
			myData->bData[bd].cData[ch].misc.sumDischargeCCCVAh = 0L;
		}

		if(myData->bData[bd].cData[ch].misc.cvTime == 0) {
	 		myData->bData[bd].cData[ch].misc.sumDischargeCCAh += labs(tmpI);
			myData->bData[bd].cData[ch].misc.dischargeCCAh
				= myData->bData[bd].cData[ch].misc.seedDischargeCCAh
				+ (long)(myData->bData[bd].cData[ch].misc.sumDischargeCCAh
				/ acc_time);

			if(myData->bData[bd].cData[ch].misc.sumDischargeCCAh > MAX_SUM 
				|| myData->bData[bd].cData[ch].misc.sumDischargeCCAh
				< ((-1) * MAX_SUM)) {
				myData->bData[bd].cData[ch].misc.seedDischargeCCAh
					= myData->bData[bd].cData[ch].misc.dischargeCCAh;
				myData->bData[bd].cData[ch].misc.sumDischargeCCAh = 0L;
			}
		} else {
	 		myData->bData[bd].cData[ch].misc.dischargeCVAh
				= myData->bData[bd].cData[ch].misc.dischargeCCCVAh
				- myData->bData[bd].cData[ch].misc.dischargeCCAh;
		}
	}
	//211115 pthw Accumulate_Ampare
#ifdef _GUI_OPCUA_TYPE
	myData->bData[bd].cData[ch].misc.sumAccumulated_Capacity += (long)(tmpI); //201115
	myData->bData[bd].cData[ch].misc.Accumulated_Capacity
	    = (long)(myData->bData[bd].cData[ch].misc.sumAccumulated_Capacity / acc_time);

	if(myData->bData[bd].cData[ch].misc.sumAccumulated_Capacity > MAX_SUM
	    || myData->bData[bd].cData[ch].misc.sumAccumulated_Capacity
		       < ((-1) * MAX_SUM)) {
	  myData->bData[bd].cData[ch].misc.seedAccumulated_Capacity
	        = myData->bData[bd].cData[ch].misc.Accumulated_Capacity;
	  myData->bData[bd].cData[ch].misc.sumAccumulated_Capacity = 0L;
	}
#endif
}

void cCalculate_Watt(int bd, int ch)
{
	long flag, tempI;
	double tmpI, acc_time;

	acc_time = 36000.0 * (10.0 / myPs->misc.rt_scan_time);
	
	if(myData->bData[bd].cData[ch].op.phase != P50) return;
	if(myData->bData[bd].cData[ch].op.type == STEP_REST) return;
	if(myData->bData[bd].cData[ch].misc.patternPhase != 0) return;

	//cal tmpWatt
	//uA -> watt 1mW/div
	//nA -> watt 1uW/div
 	tmpI = (myData->bData[bd].cData[ch].misc.tmpVsens / 1000.0)
		* (myData->bData[bd].cData[ch].misc.tmpIsens / 1000.0);

	if(myData->bData[bd].cData[ch].op.type == STEP_USER_PATTERN
		|| myData->bData[bd].cData[ch].op.type == STEP_USER_MAP) {
		myData->bData[bd].cData[ch].misc.tmpWatt = (long)(tmpI / 1000.0);
	} else {
		myData->bData[bd].cData[ch].misc.tmpWatt = labs(tmpI / 1000.0);
	}

	tmpI = (myData->bData[bd].cData[ch].op.Vsens / 1000.0)
		* (myData->bData[bd].cData[ch].op.Isens / 1000.0);

	if(myData->bData[bd].cData[ch].op.type == STEP_USER_PATTERN
		|| myData->bData[bd].cData[ch].op.type == STEP_USER_MAP) {
		myData->bData[bd].cData[ch].op.watt = (long)(tmpI / 1000.0);
	} else {
		myData->bData[bd].cData[ch].op.watt = labs(tmpI / 1000.0);
	}

	//cal wattHour
	//uA -> watt 1mWh/div
	//nA -> watt 1uWh/div
/*	if(myData->bData[bd].cData[ch].op.type == STEP_USER_PATTERN) {
		myData->bData[bd].cData[ch].misc.sumWattHour += (long)(tmpI / 1000.0);
	} else {
		myData->bData[bd].cData[ch].misc.sumWattHour += labs(tmpI / 1000.0);
	}*/

	tempI = myData->bData[bd].cData[ch].misc.tmpIsens;
	flag = (long)myData->bData[bd].cData[ch].misc.endIntegralWhFlag;
	if((flag == 1 && tempI > 0) //flag = 1 Charge
		|| (flag == 2 && tempI < 0) //flag = 2 DisCharge
		|| (flag == 3)) { //flag = 3 All
		myData->bData[bd].cData[ch].misc.sumintegralWattHour 
			+= (long)(tmpI / 1000.0);
	} else if(flag == 4) { //All Absolute Value Accumulate 170116 oys add
		myData->bData[bd].cData[ch].misc.sumintegralWattHour 
			+= (long)(labs(tmpI) / 1000.0);
	}
	
	myData->bData[bd].cData[ch].op.integral_WattHour 
		= myData->bData[bd].cData[ch].misc.seedintegralWattHour 
		+ (long)(myData->bData[bd].cData[ch].misc.sumintegralWattHour 
		/ acc_time);

/*	myData->bData[bd].cData[ch].op.wattHour
		= myData->bData[bd].cData[ch].misc.seedWattHour
		+ myData->bData[bd].cData[ch].misc.sumWattHour
		/ acc_time;
 	if(myData->bData[bd].cData[ch].misc.sumWattHour > MAX_SUM
	  	|| myData->bData[bd].cData[ch].misc.sumWattHour < -1*MAX_SUM) {
		myData->bData[bd].cData[ch].misc.seedWattHour
   			= myData->bData[bd].cData[ch].op.wattHour;
		myData->bData[bd].cData[ch].misc.sumWattHour = 0L;
	}*/

	if(myData->bData[bd].cData[ch].misc.sumintegralWattHour > MAX_SUM
		|| myData->bData[bd].cData[ch].misc.sumintegralWattHour
		< ((-1) * MAX_SUM)) {
		myData->bData[bd].cData[ch].misc.seedintegralWattHour
   			= myData->bData[bd].cData[ch].op.integral_WattHour;
		myData->bData[bd].cData[ch].misc.sumintegralWattHour = 0L;
	}

	if(tempI > 0) { //charge_wattHour
		myData->bData[bd].cData[ch].misc.sumChargeWattHour
			+= (long)(tmpI / 1000.0);
		myData->bData[bd].cData[ch].op.charge_wattHour
			= myData->bData[bd].cData[ch].misc.seedChargeWattHour
			+(long)(myData->bData[bd].cData[ch].misc.sumChargeWattHour
			/ acc_time);

		if(myData->bData[bd].cData[ch].misc.sumChargeWattHour > MAX_SUM
			|| myData->bData[bd].cData[ch].misc.sumChargeWattHour 
			< ((-1) * MAX_SUM)) {
			myData->bData[bd].cData[ch].misc.seedChargeWattHour
				= myData->bData[bd].cData[ch].op.charge_wattHour;
			myData->bData[bd].cData[ch].misc.sumChargeWattHour = 0L;
		}
	} else { //discharge_wattHour
		myData->bData[bd].cData[ch].misc.sumDischargeWattHour
			+= labs(tmpI / 1000.0);
		myData->bData[bd].cData[ch].op.discharge_wattHour
			= myData->bData[bd].cData[ch].misc.seedDischargeWattHour
			+ (long)(myData->bData[bd].cData[ch].misc.sumDischargeWattHour
			/ acc_time);

		if(myData->bData[bd].cData[ch].misc.sumDischargeWattHour > MAX_SUM
			|| myData->bData[bd].cData[ch].misc.sumDischargeWattHour
			< ((-1) * MAX_SUM)) {
			myData->bData[bd].cData[ch].misc.seedDischargeWattHour
				= myData->bData[bd].cData[ch].op.discharge_wattHour;
			myData->bData[bd].cData[ch].misc.sumDischargeWattHour = 0L;
		}
	}

	if(myData->bData[bd].cData[ch].op.type == STEP_USER_PATTERN
		|| myData->bData[bd].cData[ch].op.type == STEP_USER_MAP) {
		myData->bData[bd].cData[ch].op.wattHour
			= (myData->bData[bd].cData[ch].op.charge_wattHour
			- myData->bData[bd].cData[ch].op.discharge_wattHour);
	} else {
		myData->bData[bd].cData[ch].op.wattHour
			= labs(myData->bData[bd].cData[ch].op.charge_wattHour
			- myData->bData[bd].cData[ch].op.discharge_wattHour);
	}
	
	//201115 pth wAccumulated_WattHour
#ifdef _GUI_OPCUA_TYPE
   	myData->bData[bd].cData[ch].misc.sumAccumulated_WattHour += (long)(tmpI / 1000.0);
	myData->bData[bd].cData[ch].misc.Accumulated_WattHour
	     = (long)(myData->bData[bd].cData[ch].misc.sumAccumulated_WattHour / acc_time);

	if(myData->bData[bd].cData[ch].misc.sumAccumulated_WattHour > MAX_SUM
	   || myData->bData[bd].cData[ch].misc.sumAccumulated_WattHour
	   < ((-1) * MAX_SUM)) {
	   myData->bData[bd].cData[ch].misc.seedAccumulated_WattHour
	  	  = myData->bData[bd].cData[ch].misc.Accumulated_WattHour;
	   myData->bData[bd].cData[ch].misc.sumAccumulated_WattHour = 0L;
	}
#endif
}

void CalSourceAverage(int bd)
{
	int i, j, cnt;
	double temp, min, max, ratio;
	
	if(bd >= myPs->config.installedBd) return;
	
	cnt = myPs->misc.sensCount;
	for(i=0; i < 4; i++) {
		temp = myData->bData[bd].misc.source[i].sensSumV[0];
		min = temp;
		max = temp;
		temp = 0;
		if(myPs->misc.sensCountFlag == P1){
			for(j=0; j < MAX_FILTER_AD_COUNT; j++) {
				temp += myData->bData[bd].misc.source[i].sensSumV[j];
				if(min > myData->bData[bd].misc.source[i].sensSumV[j]) {
					min = myData->bData[bd].misc.source[i].sensSumV[j];
				}
				if(max < myData->bData[bd].misc.source[i].sensSumV[j]) {
					max = myData->bData[bd].misc.source[i].sensSumV[j];
				}
			}
			temp = (temp - min - max)/(double)(MAX_FILTER_AD_COUNT-2);
//			temp = (temp)/(double)(MAX_FILTER_AD_COUNT);
		}else{
			for(j=0; j <= cnt; j++) {
				temp += myData->bData[bd].misc.source[i].sensSumV[j];
			}
			temp = temp /(cnt+1);
		}
		ratio = 305.1850948;	//10000000uV/32767
		// ref Voltage AD 081203 KJI
		switch(myPs->config.hwSpec) {
			case L_20V_5A_R1:
				ratio = ratio * (20.0 / 5.0);
				break;
			case L_5V_150A_R1:
				ratio /= 1.494;
				break;
			case L_10V_5A_R2:
				break;
			case S_5V_200A_75A_15A_AD2:
				ratio = (double)ratio / myPs->config.voltageAmp
						* myPs->config.adAmp
						* ((double)myPs->config.maxVoltage[0]
						/ 5000000.0);
				break;
			default :	
				ratio = (double)ratio / myPs->config.adAmp 
						* ((double)myPs->config.maxVoltage[0] 
						/ 5000000.0);
				break;
		}
		myData->bData[bd].misc.source[i].sourceV
			= (long)(temp * ratio);
		
		temp = myData->bData[bd].misc.source[i].sensSumI[0];
		min = temp;
		max = temp;
		temp = 0;
		if(myPs->misc.sensCountFlag == P1){
			for(j=0; j < MAX_FILTER_AD_COUNT; j++) {
				temp += myData->bData[bd].misc.source[i].sensSumI[j];
				if(min > myData->bData[bd].misc.source[i].sensSumI[j]) {
					min = myData->bData[bd].misc.source[i].sensSumI[j];
				}
				if(max < myData->bData[bd].misc.source[i].sensSumI[j]) {
					max = myData->bData[bd].misc.source[i].sensSumI[j];
				}
			}
			temp = (temp - min - max)/(double)(MAX_FILTER_AD_COUNT-2);
//			temp = (temp)/(double)(MAX_FILTER_AD_COUNT);
		}else{
			for(j=0; j <= cnt; j++) {
				temp += myData->bData[bd].misc.source[i].sensSumI[j];
			}
			temp = temp /(cnt+1);
		}
	// This Coding have a some problem khk
	//ref current AD 081203 KJI
		switch(myPs->config.hwSpec){
			case L_5V_150A_R1:
				if(i == 0) temp = 32767*(0.8164286);
				else if(i == 1) temp = (-1)*32767*(0.8164286);
				else temp = 0;
				break;
			case L_10V_5A_R2:
				if(i == 0) temp = 32767*(0.510000);
				else if(i == 1) temp = (-1)*32767*(0.510000);
				else temp = 0;
				break;
			default:
				if(i == 0) temp = (double) 32767
						*(myPs->config.currentAmp * myPs->config.adAmp / 100.0);
				else if(i == 1) temp = (double) (-1)
						*32767*(myPs->config.currentAmp * myPs->config.adAmp / 100.0 );
				else temp = 0;
				break;
		}
		switch(myPs->config.hwSpec) {
			case L_5V_150A_R1: //150000000uA / (32767*5442857uV/10000000uV)
				ratio = 150000000.0/(32767.0*0.5442857);
				break;
			case L_10V_5A_R2: //2000000uA / (32767*5100000uV/10000000uV)
				ratio = 5000000.0/(32767.0*0.5100000);
				break;
			default:
				ratio = myPs->config.maxCurrent[0] 
						/(32767.0 * (myPs->config.currentAmp / 100.0));
				break;
		}
		switch(myPs->config.hwSpec) {
			case L_5V_150A_R1:
				ratio /= 1.494;
				break;
			case L_10V_5A_R2:
				break;
			default:
				ratio = (double) ratio / myPs->config.adAmp;
				break;
		}
		 myData->bData[bd].misc.source[i].sourceI
				= (long)(temp * ratio);
	}
}
/*
void CalSourceAverage(int bd)
{
	int i, j, cnt;
	double temp, min, max, ratio;
	
	if(bd >= myPs->config.installedBd) return;
	
	cnt = myPs->misc.sensCount;
	for(i=0; i < 4; i++) {
		temp = myData->bData[bd].misc.source[i].sensSumV[0];
		min = temp;
		max = temp;
		temp = 0;
		if(myPs->misc.sensCountFlag == P1){
			for(j=0; j < MAX_FILTER_AD_COUNT; j++) {
				temp += myData->bData[bd].misc.source[i].sensSumV[j];
				if(min > myData->bData[bd].misc.source[i].sensSumV[j]) {
					min = myData->bData[bd].misc.source[i].sensSumV[j];
				}
				if(max < myData->bData[bd].misc.source[i].sensSumV[j]) {
					max = myData->bData[bd].misc.source[i].sensSumV[j];
				}
			}
			temp = (temp - min - max)/(double)(MAX_FILTER_AD_COUNT-2);
		}else{
			for(j=0; j <= cnt; j++) {
				temp += myData->bData[bd].misc.source[i].sensSumV[j];
			}
			temp = temp /(cnt+1);
		}
		ratio = 305.1850948;	//10000000uV/32767
		switch(myPs->config.hwSpec){
			case L_50V_50A:
				ratio = (ratio*0.75)*(50.0/5.0);// AD 7.5V ==>  DA 50V
				break;
			case L_20V_25A:
				ratio = ratio * (20.0/5.0);// AD 5.V ==>  DA 20V
				break;
			case L_20V_10A_R1:
			case L_20V_50A_R2:
				ratio = ratio * (20.0/5.0);// AD 7.V ==>  DA 20V
				break;
			case L_5V_100A:
			case L_5V_100A_R1:
			case L_5V_100A_R1_EG:
			case L_5V_500A_R1:
			case L_5V_500A_R2:
			case L_5V_200A_R4:
			case L_5V_200A_R3:
			case L_5V_150A_R1:
			case L_5V_150A_R3:
			case L_5V_250A_R1:
			case L_5V_50A_R1:
			case L_5V_1000A_R1:
			case L_5V_1000A_R3:
			case L_5V_300A_R1:
			case L_5V_300A_R3:
			case L_5V_50A_R2:
				ratio /= 1.494;
				break;
			case L_5V_2A_R2:
			case L_10V_5A_R2:
				break;
			default:
				ratio /= 1.5;
				break;
		}

		myData->bData[bd].misc.source[i].sourceV
			= (long)(temp * ratio);
		
		temp = myData->bData[bd].misc.source[i].sensSumI[0];
		min = temp;
		max = temp;
		temp = 0;
		if(myPs->misc.sensCountFlag == P1){
			for(j=0; j < MAX_FILTER_AD_COUNT; j++) {
				temp += myData->bData[bd].misc.source[i].sensSumI[j];
				if(min > myData->bData[bd].misc.source[i].sensSumI[j]) {
					min = myData->bData[bd].misc.source[i].sensSumI[j];
				}
				if(max < myData->bData[bd].misc.source[i].sensSumI[j]) {
					max = myData->bData[bd].misc.source[i].sensSumI[j];
				}
			}
			temp = (temp - min - max)/(double)(MAX_FILTER_AD_COUNT-2);
		}else{
			for(j=0; j <= cnt; j++) {
				temp += myData->bData[bd].misc.source[i].sensSumI[j];
			}
			temp = temp /(cnt+1);
		}
// This Coding have a some problem khk
switch(myPs->config.hwSpec){
			case L_50V_50A:
				if(i == 0) temp = 32767*(0.5982353); 
				else if(i == 1) temp = (-1)*32767*(0.5982353); 
				else temp = 0;
				break;
			case L_20V_25A:
				if(i == 0) temp = 32767*(0.697); 
				else if(i == 1) temp = (-1)*32767*(0.697); 
				else temp = 0;
				break;
			case L_20V_10A_R1:
			case L_20V_50A_R2:
				if(i == 0) temp = 32767*(0.6051807); 
				else if(i == 1) temp = (-1)*32767*(0.6051807); 
				else temp = 0;
				break;
			case L_5V_100A:
			case L_5V_50A_R1:
				if(i == 0) temp = 32767*(0.752976);
				else if(i == 1) temp = (-1)*32767*(0.752976);
				else temp = 0;
				break;
			case L_5V_200A_R2:
				if(i == 0) temp = 32767*(0.863333);
				else if(i == 1) temp = (-1)*32767*(0.863333);
				else temp = 0;
				break;
			case L_5V_100A_R2:
				if(i == 0) temp = 32767*(0.5982353);
				else if(i == 1) temp = (-1)*32767*(0.5982353);
				else temp = 0;
			case L_5V_30A_R1:
				if(i == 0) temp = 32767*(0.6009375);
				else if(i == 1) temp = (-1)*32767*(0.6009375);
				else temp = 0;
				break;
			case L_5V_100A_R1:
			case L_5V_100A_R1_EG:
			case L_5V_500A_R1:
			case L_5V_500A_R2:
			case L_5V_200A_R4:
			case L_5V_200A_R3:
			case L_5V_150A_R3:
			case L_5V_250A_R1:
			case L_5V_1000A_R1:
			case L_5V_1000A_R3:
			case L_5V_300A_R1:
			case L_5V_300A_R3:
				if(i == 0) temp = 32767*(0.904140);
				else if(i == 1) temp = (-1)*32767*(0.904140);
				else temp = 0;
				break;
			case L_5V_2A_R1:
				if(i == 0) temp = 32767*(0.756000);
				else if(i == 1) temp = (-1)*32767*(0.756000);
				else temp = 0;
				break;
			case L_5V_2A_R2:
			case L_10V_5A_R2:
				if(i == 0) temp = 32767*(0.510000);
				else if(i == 1) temp = (-1)*32767*(0.510000);
				else temp = 0;
				break;
			case L_5V_150A_R1:
				if(i == 0) temp = 32767*(0.8164286);
				else if(i == 1) temp = (-1)*32767*(0.8164286);
				else temp = 0;
				break;
			case L_5V_50A_R2:
				if(i == 0) temp = 32767*(0.5040000)*(1.494);
				else if(i == 1) temp =(-1)*32767*(0.5040000)*(1.494);
				else temp = 0;
				break;
		}
//		
		switch(myPs->config.hwSpec) {
			case L_5V_10mA: //10000uA / (32767*5982353uV/10000000uV)
				if(myPs->config.ratioCurrent == MICRO) { //uA
					ratio = 0.5101422379;
				} else  { //nA
					ratio = 510.1422379;
				}
				break;
			case L_5V_2A:
			case L_5V_3A:
				ratio = 0;
				break;
			case L_5V_5A: //5000000uA / (32767*5610000uV/10000000uV)
			case L_6V_6A: //6000000uA / (32767*6732000uV/10000000uV)
				ratio = 272.0009757;
				break;
			case L_5V_20A: //20000000uA / (32767*5755555uV/10000000uV)
				ratio = 1060.4888486;
				break;
			case L_5V_30A: //30000000uA / (32767*6400000uV/10000000uV)
				ratio = 1430.555132;
				break;
			case L_5V_5A_2:
				if(myPs->config.ratioCurrent == MICRO) { //uA
					//5000000uA / (32767*5610000uV/10000000uV)
					ratio = 272.0009757;
				} else { //nA
					ratio = 0.2720009757;
				}
				break;
			case L_5V_200A: //200000000uA / (32767*9290909uV/10000000uV)
				ratio = 6569.542222;
				break;
			case S_5V_200A: //200000000uA / (32767*6450000uV/10000000uV)
				ratio = 9463.103713;
				break;
			case L_2V_100A: //100000000uA / (32767*5982353uV/10000000uV)
			case L_5V_100A_R2: //100000000uA / (32767*5982353uV/10000000uV)
				ratio = 5101.422379;
				break;
			case L_50V_50A: //50000000uA / (32767*5982353uV/10000000uV)
				ratio = 2550.71119;
				break;
			case L_20V_25A: //25000000uA / (32767*6970000uV/10000000uV)
				ratio = 1052.043155;
				break;
			case L_5V_100A: //100000000uA / (32767*5040000uV/10000000uV)
				ratio = 100000000.0/(32767.0*0.504);
				break;
			case L_5V_200A_R2: //200000000uA / (32767*575555uV/10000000uV)
				ratio = 200000000.0/(32767.0*0.575555);
				break;
			case L_5V_100A_R1: //100000000uA / (32767*605181uV/10000000uV)
			case L_5V_100A_R1_EG: //100000000uA / (32767*605181uV/10000000uV)
				ratio = 100000000.0/(32767.0*0.605181);
				break;
			case L_5V_30A_R1: //30000000uA / (32767*6009375uV/10000000uV)
				ratio = 30000000.0/(32767.0*0.6009375);
				break;
			case L_5V_2A_R1: //2000000uA / (32767*5040000uV/10000000uV)
				ratio = 2000000.0/(32767.0*0.5040000);
				break;
			case L_5V_2A_R2: //2000000uA / (32767*5100000uV/10000000uV)
				ratio = 2000000.0/(32767.0*0.5100000);
				break;
			case L_5V_500A_R1: //500000000uA / (32767*6051807uV/10000000uV)
			case L_5V_500A_R2: //500000000uA / (32767*6051807uV/10000000uV)
				ratio = 500000000.0/(32767.0*0.6051807);
				break;
			case L_5V_200A_R4: //200000000uA / (32767*6051807uV/10000000uV)
			case L_5V_200A_R3: //200000000uA / (32767*6051807uV/10000000uV)
				ratio = 200000000.0/(32767.0*0.6051807);
				break;
			case L_5V_150A_R1: //150000000uA / (32767*5442857uV/10000000uV)
				ratio = 150000000.0/(32767.0*0.5442857);
				break;
			case L_5V_150A_R3: //150000000uA / (32767*6051807uV/10000000uV)
				ratio = 150000000.0/(32767.0*0.6051807);
				break;
			case L_5V_250A_R1: //250000000uA / (32767*605181uV/10000000uV)
				ratio = 250000000.0/(32767.0*0.605181);
				break;
			case L_5V_50A_R1: //50000000uA / (32767*5040000uV/10000000uV)
				ratio = 50000000.0/(32767.0*0.5040000);
				break;
			case L_5V_1000A_R1: //1000000000uA / (32767*6051807uV/10000000uV)
			case L_5V_1000A_R3: //1000000000uA / (32767*6051807uV/10000000uV)
				ratio = 1000000000.0/(32767.0*0.6051807);
				break;
			case L_5V_300A_R1: //300000000uA / (32767*6051807uV/10000000uV)
			case L_5V_300A_R3: //300000000uA / (32767*6051807uV/10000000uV)
				ratio = 300000000.0/(32767.0*0.6051807);
				break;
			case L_20V_10A_R1: //10000000uA / (32767*6051807uV/10000000uV)
				ratio = 10000000.0/(32767.0*0.6051807);
				break;
			case L_20V_50A_R2: //50000000uA / (32767*6051807uV/10000000uV)
				ratio = 50000000.0/(32767.0*0.6051807);
				break;
			case L_5V_50A_R2: //50000000uA / (32767*5040000uV/10000000uV)
				ratio = 50000000.0/(32767.0*0.5040000);
				break;
			case L_10V_5A_R2: //2000000uA / (32767*5100000uV/10000000uV)
				ratio = 5000000.0/(32767.0*0.5100000);
				break;
			default:
				ratio = 0;
				break;
		}
		switch(myPs->config.hwSpec){
			case L_5V_2A_R2:
			case L_10V_5A_R2:
					break;
			case S_5V_200A:
			case L_2V_100A:
			case L_5V_5A_2:
			case L_5V_20A:
			case L_5V_50A:
			case L_5V_200A_R2:
				ratio /= 1.5;
				break;
			case L_5V_100A:
			case L_5V_100A_R1:
			case L_5V_100A_R1_EG:
			case L_5V_2A_R1:
			case L_5V_500A_R1:
			case L_5V_500A_R2:
			case L_5V_200A_R4:
			case L_5V_200A_R3:
			case L_5V_150A_R1:
			case L_5V_150A_R3:
			case L_5V_250A_R1:
			case L_5V_50A_R1:
			case L_5V_1000A_R1:
			case L_5V_1000A_R3:
			case L_5V_300A_R1:
			case L_5V_300A_R3:
			case L_5V_50A_R2:
				ratio /= 1.494;
				break;
		}
	
	ratio = (double) ratio / myPs->config.adAmp;
		
	myData->bData[bd].misc.source[i].sourceI
			= (long)(temp * ratio);
	}
}
*/

void CalSourceRingBuffer(int bd)
{
	int cnt, i, j, k;
	long	maxV, minV, maxI, minI, temp;
	double tempV, tempI;
	
	if(bd >= myPs->config.installedBd) return;
	
	cnt = myPs->misc.sourceSensCount;

	if(myPs->misc.sourceSensCountFlag == P0) {
		k = cnt + 1;
	} else {
		k = MAX_SOURCE_SENS_COUNT;
	}

	for(i=0; i < 4; i++) {
		myData->bData[bd].misc.source2[i].sumV[cnt]
			= myData->bData[bd].misc.source[i].sourceV;
		myData->bData[bd].misc.source2[i].sumI[cnt]
			= myData->bData[bd].misc.source[i].sourceI;

		tempV = 0;
		tempI = 0;
		maxV= myData->bData[bd].misc.source[i].sourceV;
		minV= myData->bData[bd].misc.source[i].sourceV;
		maxI= myData->bData[bd].misc.source[i].sourceI;
		minI= myData->bData[bd].misc.source[i].sourceI;

		for(j=0; j < k; j++) {
			temp = myData->bData[bd].misc.source2[i].sumV[j];
			if(maxV < temp) maxV = temp;
			if(minV > temp) minV = temp;
			tempV += (double)temp;

			temp = myData->bData[bd].misc.source2[i].sumI[j];
			if(maxI < temp) maxI = temp;
			if(minI > temp) minI = temp;
			tempI += (double)temp;
		}
		myData->bData[bd].misc.source2[i].sourceV = (long)((tempV-maxV-minV) / (k-2));
		myData->bData[bd].misc.source2[i].sourceI = (long)((tempI-maxI-minI) / (k-2));
	}
}

void CalibratorSource(int bd)
{
	int i;
	double AD_a, AD_b, val1, val2;
	
	if(bd >= myPs->config.installedBd) return;
	
	val1 = (double)myPs->config.maxVoltage[0];
	val2 = (double)myPs->config.maxCurrent[0];

	AD_a = (double)(val1 - 0.0)
		/ (double)(myData->bData[bd].misc.source2[0].sourceV
		- myData->bData[bd].misc.source2[2].sourceV);
	AD_b = val1
		- (double)myData->bData[bd].misc.source2[0].sourceV * AD_a;
	myData->bData[bd].misc.Vsource_AD_a = AD_a;
	myData->bData[bd].misc.Vsource_AD_b = AD_b;
	
	AD_a = (double)(0.0 - (-val1))
		/ (double)(myData->bData[bd].misc.source2[2].sourceV
		- myData->bData[bd].misc.source2[1].sourceV);
	AD_b = (double)(0.0)
		- (double)myData->bData[bd].misc.source2[2].sourceV * AD_a;
	myData->bData[bd].misc.Vsource_AD_a_N = AD_a;
	myData->bData[bd].misc.Vsource_AD_b_N = AD_b;

	if(myPs->config.hwSpec == S_5V_200A_75A_15A_AD2) {
		myData->bData[bd].misc.Isource_AD_a
		 = myData->bData[bd].misc.Vsource_AD_a;
		myData->bData[bd].misc.Isource_AD_b
	     = myData->bData[bd].misc.Vsource_AD_b;
		myData->bData[bd].misc.Isource_AD_a_N
		 = myData->bData[bd].misc.Vsource_AD_a_N;
		myData->bData[bd].misc.Isource_AD_b_N
	     = myData->bData[bd].misc.Vsource_AD_b_N;
	}else{
		AD_a = (double)(val2 - 0.0)
			/ (double)(myData->bData[bd].misc.source2[0].sourceI
			- myData->bData[bd].misc.source2[2].sourceI);
		AD_b = (double)val2
			- (double)myData->bData[bd].misc.source2[0].sourceI * AD_a;
		myData->bData[bd].misc.Isource_AD_a = AD_a;
		myData->bData[bd].misc.Isource_AD_b = AD_b;
	
		AD_a = (double)(0.0 - (-val2))
			/ (double)(myData->bData[bd].misc.source2[2].sourceI
			- myData->bData[bd].misc.source2[1].sourceI);
		AD_b = (double)(0.0)
			- (double)myData->bData[bd].misc.source2[2].sourceI * AD_a;
		myData->bData[bd].misc.Isource_AD_a_N = AD_a;
		myData->bData[bd].misc.Isource_AD_b_N = AD_b;
	}

	for(i=0; i < 4; i++) {
		if(myData->bData[bd].misc.source2[i].sourceV >= 0) {
			myData->bData[bd].misc.source[i].calSourceV
				= (long)((double)myData->bData[bd].misc.source2[i].sourceV
				* myData->bData[bd].misc.Vsource_AD_a
				+ myData->bData[bd].misc.Vsource_AD_b);
		} else {
			myData->bData[bd].misc.source[i].calSourceV
				= (long)((double)myData->bData[bd].misc.source2[i].sourceV
				* myData->bData[bd].misc.Vsource_AD_a_N
				+ myData->bData[bd].misc.Vsource_AD_b_N);
		}
		if(myData->bData[bd].misc.source2[i].sourceI >= 0) {
			myData->bData[bd].misc.source[i].calSourceI
				= (long)((double)myData->bData[bd].misc.source2[i].sourceI
				* myData->bData[bd].misc.Isource_AD_a
				+ myData->bData[bd].misc.Isource_AD_b);
		} else {
			myData->bData[bd].misc.source[i].calSourceI
				= (long)((double)myData->bData[bd].misc.source2[i].sourceI
				* myData->bData[bd].misc.Isource_AD_a_N
				+ myData->bData[bd].misc.Isource_AD_b_N);
		}
	}
}

int cFindADCaliPoint(int bd, int ch, double value, int type, int range)
{
	int point;
	
	if(myData->AppControl.config.systemType == CYCLER_CAN) {
		if(type == 0){
			point = cFindADCaliPoint_Default(bd, ch, value, type, range);
		}else{
			point = cFindADCaliPoint_Default(bd, ch, value, type, range);
			//0724 Test lyhw
			//point = cFindADCaliPoint_2(bd, ch, value, type, range);
		}
		return point;
	}

	switch(myPs->config.hwSpec) {
		case S_5V_200A_75A_15A_AD2:
			if(type == 0){
				point = cFindADCaliPoint_Default(bd, ch, value, type, range);
			}else{
				point = cFindADCaliPoint_2(bd, ch, value, type, range);
			}
			break;
		default:
			point = cFindADCaliPoint_Default(bd, ch, value, type, range);
			break;
	}
	return point;
}

int cFindADCaliPoint_Default(int bd, int ch, double value, int type, int range)
{
	int point, point1;
	double ad;

	if(type == 3 ){
		for(point=0; point < myData->cali.data[bd][ch].point[0][range]
			.setPointNum; point++) {
			ad = myData->cali.data[bd][ch].set_ad[0][range][point]; 
			if(ad >= value) {
				if(point <= 0) point=0;
				else point-=1;
				return point;
			}
		}
		point1 = myData->cali.data[bd][ch].point[0][range].setPointNum-1;
		ad = myData->cali.data[bd][ch].set_ad[0][range][point1]; 
		if(ad < value) {
			if(point1 <= 0) point = 0;
			else point = point1-1;
		}
	}else if(type == 4){
		for(point=0; point < myData->cali.data[bd][ch].point[1][range]
			.setPointNum; point++) {
			ad = myData->cali.data[bd][ch].set_ad[1][range][point]; 
			if(ad >= value) {
				if(point <= 0) point=0;
				else point-=1;
				return point;
			}
		}
		point1 = myData->cali.data[bd][ch].point[1][range].setPointNum-1;
		ad = myData->cali.data[bd][ch].set_ad[1][range][point1]; 
		if(ad < value) {
			if(point1 <= 0) point = 0;
			else point = point1-1;
		}
	}else{
		if(myData->bData[bd].cData[ch].op.state == C_CALI
			&& myData->mData.cali_meas_type != MEAS) {
			for(point=0; point
				< myData->cali.tmpCond[bd][ch].point[type][range].setPointNum;
				point++) {
				ad = myData->cali.tmpData[bd][ch].set_ad[type][range][point];
				if(ad >= value) {
					if(point <= 0) point = 0;
					else point -= 1;
					return point;
				}
			}
			point1 = myData->cali.tmpCond[bd][ch].point[type][range].setPointNum-1;
			ad = myData->cali.tmpData[bd][ch].set_ad[type][range][point1];
			if(ad < value) {
				if(point1 <= 0) point = 0;
				else point = point1-1;
			}
		} else {
			for(point=0; point < myData->cali.data[bd][ch].point[type][range]
				.setPointNum; point++) {
				ad = myData->cali.data[bd][ch].set_ad[type][range][point]; 
				if(ad >= value) {
					if(point <= 0) point=0;
					else point-=1;
					return point;
				}
			}
			point1 = myData->cali.data[bd][ch].point[type][range].setPointNum-1;
			ad = myData->cali.data[bd][ch].set_ad[type][range][point1]; 
			if(ad < value) {
				if(point1 <= 0) point = 0;
				else point = point1-1;
			}
		}
	}
	return point;
}

int cFindADCaliPoint_2(int bd, int ch, double value, int type, int range)
{
	int point, point1, pointNum;
	double ad;

	if(myData->bData[bd].cData[ch].op.state == C_CALI) {
		pointNum = myData->cali.tmpCond[bd][ch].point[type][range].discharge_pointNum;
		if(value < 0){
			for(point=0; point < pointNum; point++) {
				ad = myData->cali.tmpData[bd][ch].set_ad[type][range][point];
				if(ad >= value) {
					if(point <= 0) point = 0;
					else point -= 1;
					return point;
				}
			}
			point1 = pointNum-1;
			ad = myData->cali.tmpData[bd][ch].set_ad[type][range][point1];
			if(ad < value) {
				if(point1 <= 0) point = 0;
				else point = point1-1;
			}
		}else{
			for(point= pointNum+1; point < myData->cali.tmpCond[bd][ch].point[type][range].setPointNum;
				point++) {
				ad = myData->cali.tmpData[bd][ch].set_ad[type][range][point];
				if(ad >= value) {
					if(point <= 0) point = 0;
					else point = point-pointNum;
					return point;
				}
			}
			point1 = myData->cali.tmpCond[bd][ch].point[type][range].setPointNum-1;
			ad = myData->cali.tmpData[bd][ch].set_ad[type][range][point1];
			if(ad < value) {
				if(point1 <= 0) point = 0;
				else point = point1-pointNum;
			}
		}
	} else {
		pointNum = myData->cali.data[bd][ch].point[type][range]
					.discharge_pointNum;
		if(value < 0){
			for(point=0; point < pointNum; point++) {
				ad = myData->cali.data[bd][ch].set_ad[type][range][point]; 
				if(ad >= value) {
					if(point <= 0) point=0;
					else point-=1;
					return point;
				}
			}
			point1 = pointNum-1;
			ad = myData->cali.data[bd][ch].set_ad[type][range][point1]; 
			if(ad < value) {
				if(point1 <= 0) point = 0;
				else point = point1-1;
			}
		}else{
			for(point = pointNum+1; 
				point < myData->cali.data[bd][ch].point[type][range].setPointNum;
				point++) {
				ad = myData->cali.data[bd][ch].set_ad[type][range][point]; 
				if(ad >= value) {
					if(point <= 0) point=0;
					else point = point-pointNum;
					return point;
				}
			}
			point1 = myData->cali.data[bd][ch].point[type][range].setPointNum-1;
			ad = myData->cali.data[bd][ch].set_ad[type][range][point1]; 
			if(ad < value) {
				if(point1 <= 0) point = 0;
				else point = point1-pointNum;
			}
		}
	}
	return point;
}

void SWAP(double *x, double *y)
{
	double tmp;

	tmp = *x;
	*x = *y;
	*y = tmp;
}

void cSimulation(int bd, int ch, int div, int parallel_ch, int type)
{
	int stepType, preType, deltaV, deltaI;
	long testRefV, testRefI, maxV, maxI;
	static int recovery_v_t[MAX_CH_PER_BD];
	static long startV;
	
	stepType = myData->bData[bd].cData[ch].op.type;
	preType = myData->bData[bd].cData[ch].op.preType;
	testRefV = myData->bData[bd].cData[ch].misc.testRefV;
	testRefI = myData->bData[bd].cData[ch].misc.testRefI;
	
	maxV = myData->mData.config.maxVoltage[0];
	maxI = myData->mData.config.maxCurrent[0];

	deltaV =labs((((float)testRefI / (float)maxI) * 1000)
					* myPs->misc.rt_scan_time);
	deltaI =(((float)testRefI / (float)maxI) * 10000)
					* myPs->misc.rt_scan_time;

//	myData->bData[bd].cData[ch].op.temp = 25000;
	//myData->bData[bd].cData[ch].op.temp
	//	= myData->bData[bd].cData[ch].misc.groupTemp;
	
	if(myData->bData[bd].cData[ch].op.state != C_RUN) {
		switch(preType) {
			case STEP_CHARGE:
				if(recovery_v_t[ch] > 0){
					recovery_v_t[ch]--;
					if(startV < myData->bData[bd].cData[ch].misc.simVsens) {
						myData->bData[bd].cData[ch].misc.simVsens
							= myData->bData[bd].cData[ch].misc.simVsens
								- labs(deltaV/10);
						myData->bData[bd].cData[ch].op.Vsens
							= myData->bData[bd].cData[ch].misc.simVsens;
						myData->bData[bd].cData[ch].misc.tmpVsens
							= myData->bData[bd].cData[ch].misc.simVsens;
					}else{
						myData->bData[bd].cData[ch].op.Vsens
							= myData->bData[bd].cData[ch].misc.simVsens;
						myData->bData[bd].cData[ch].misc.tmpVsens
							= myData->bData[bd].cData[ch].misc.simVsens;
					}
				} else {
					myData->bData[bd].cData[ch].op.Vsens
						= myData->bData[bd].cData[ch].misc.simVsens;
					myData->bData[bd].cData[ch].misc.tmpVsens
						= myData->bData[bd].cData[ch].misc.simVsens;
				}
				break;
			case STEP_DISCHARGE:
			case STEP_Z:
				if(recovery_v_t[ch] > 0){
					recovery_v_t[ch]--;
					if(startV > myData->bData[bd].cData[ch].misc.simVsens) {
						myData->bData[bd].cData[ch].misc.simVsens
							= myData->bData[bd].cData[ch].misc.simVsens
								+ labs(deltaV/10);
						myData->bData[bd].cData[ch].op.Vsens
							= myData->bData[bd].cData[ch].misc.simVsens;
						myData->bData[bd].cData[ch].misc.tmpVsens
							= myData->bData[bd].cData[ch].misc.simVsens;
					} else {
						myData->bData[bd].cData[ch].op.Vsens
							= myData->bData[bd].cData[ch].misc.simVsens;
						myData->bData[bd].cData[ch].misc.tmpVsens
							= myData->bData[bd].cData[ch].misc.simVsens;
					}
				} else {
					myData->bData[bd].cData[ch].op.Vsens
						= myData->bData[bd].cData[ch].misc.simVsens;
					myData->bData[bd].cData[ch].misc.tmpVsens
						= myData->bData[bd].cData[ch].misc.simVsens;
				}
				break;
			default:
				myData->bData[bd].cData[ch].op.Vsens
					= myData->bData[bd].cData[ch].misc.simVsens;
				myData->bData[bd].cData[ch].misc.tmpVsens
					= myData->bData[bd].cData[ch].misc.simVsens;
				break;
		}
		return;
	}else {
		if(myData->bData[bd].cData[ch].op.runTime == 0) {
			startV = myData->bData[bd].cData[ch].misc.simVsens;
		}

		recovery_v_t[ch] = 500 * myPs->misc.rt_scan_time;
	}

	switch(type) {
		case 0:
			switch(stepType) {
				case STEP_CHARGE:
					if(myData->bData[bd].cData[ch].misc.cvFlag == P0) {
						if(testRefV
							> myData->bData[bd].cData[ch].misc.simVsens) {
				 			myData->bData[bd].cData[ch].misc.simVsens
							 += deltaV;
						} else {
							myData->bData[bd].cData[ch].misc.cvFlag = P1;
						}
					} else {
				 		myData->bData[bd].cData[ch].misc.simVsens
						 = testRefV;
					}
					break;
				case STEP_DISCHARGE:
				case STEP_Z:
					if(myData->bData[bd].cData[ch].misc.cvFlag == P0) {
						if(testRefV
							< myData->bData[bd].cData[ch].misc.simVsens) {
				 			myData->bData[bd].cData[ch].misc.simVsens
							 -= deltaV;
						} else {
							myData->bData[bd].cData[ch].misc.cvFlag = P1;
						}
					} else {
				 		myData->bData[bd].cData[ch].misc.simVsens
						 = testRefV;
					}
					break;
				case STEP_USER_PATTERN:
				case STEP_USER_MAP:
					if(testRefI > 0) {
						if(testRefV
							> myData->bData[bd].cData[ch].misc.simVsens) {
					 		myData->bData[bd].cData[ch].misc.simVsens
							 += deltaV;
						} else {
					 		myData->bData[bd].cData[ch].misc.simVsens
							 = testRefV;
						}	
					} else if (testRefI < 0) {
						if(testRefV
							< myData->bData[bd].cData[ch].misc.simVsens) {
							myData->bData[bd].cData[ch].misc.simVsens
							 -= deltaV;
						} else {
				 			myData->bData[bd].cData[ch].misc.simVsens
							 = testRefV;
						}
					} else {
						myData->bData[bd].cData[ch].op.Vsens
							= myData->bData[bd].cData[ch].misc.simVsens;
						myData->bData[bd].cData[ch].misc.tmpVsens
							= myData->bData[bd].cData[ch].misc.simVsens;
					}
					break;
				case STEP_REST:
					myData->bData[bd].cData[ch].op.Vsens
					 	= myData->bData[bd].cData[ch].misc.simVsens;
					myData->bData[bd].cData[ch].misc.tmpVsens
					 	= myData->bData[bd].cData[ch].misc.simVsens;
					break;
				default:
					break;
			}
			if(stepType != STEP_REST) {
				myData->bData[bd].cData[ch].op.Vsens
					= myData->bData[bd].cData[ch].misc.simVsens;
				myData->bData[bd].cData[ch].misc.tmpVsens
					= myData->bData[bd].cData[ch].misc.simVsens;
			}
			break;
		case 1:
		case 2:
			switch(stepType) {
				case STEP_CHARGE:
					if(myData->bData[bd].cData[ch].misc.cvFlag == P0) {
						myData->bData[bd].cData[ch].misc.simIsens
							= testRefI;
					} else {
						myData->bData[bd].cData[ch].misc.simIsens
						  -= deltaI;
					}
					break;
				case STEP_DISCHARGE:
				case STEP_Z:
					if(myData->bData[bd].cData[ch].misc.cvFlag == P0) {
						myData->bData[bd].cData[ch].misc.simIsens
							= testRefI;
					} else {
						myData->bData[bd].cData[ch].misc.simIsens
						 -= deltaI;
					}
					break;
				case STEP_USER_PATTERN:
				case STEP_USER_MAP:
					if(testRefI > 0) {
						if(testRefV - myData->bData[bd].cData[ch].misc.simVsens
							> (long)(maxV*0.001)) {
							myData->bData[bd].cData[ch].misc.simIsens
								= testRefI;
						} else {
							myData->bData[bd].cData[ch].misc.simIsens
							  -= deltaI;
						}
					} else if(testRefI < 0) {
						if(myData->bData[bd].cData[ch].misc.simVsens - testRefV
							> (long)(maxV*0.001)) {
							myData->bData[bd].cData[ch].misc.simIsens
								= testRefI;
						} else {
							myData->bData[bd].cData[ch].misc.simIsens
							 -= deltaI;
						}
					} else {
						myData->bData[bd].cData[ch].misc.simIsens = 0;
					}
					break;
				case STEP_REST:
					myData->bData[bd].cData[ch].misc.simIsens = 0;
					break;
				default:
					break;
			}
			myData->bData[bd].cData[ch].op.Isens 
				= myData->bData[bd].cData[ch].misc.simIsens;
			myData->bData[bd].cData[ch].misc.tmpIsens 
				= myData->bData[bd].cData[ch].misc.simIsens;
			break;
		default:
			break;
	}
}
