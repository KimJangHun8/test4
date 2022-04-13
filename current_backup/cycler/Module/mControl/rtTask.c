#include <linux/module.h>
#include <rtl.h>
#include <time.h>
#include <pthread.h>
#include <asm/io.h>
#include <mbuff.h>
#include "../rt_can/rt_can.h" //rt_can
#include "../../INC/datastore.h"
#include "common_utils.h"
#include "local_utils.h"
#include "message.h"
#include "ModuleControl.h"
#include "BoardControl.h"
#include "ChannelControl.h"
#include "PCU_Control.h"
#include "Analog.h"
#include "DInOutControl.h"
#include "DAQ.h"
#include "FAD.h"
#include "rtTask.h"
#include "CAN.h"

pthread_t thread;
S_SYSTEM_DATA *myData;
S_MODULE_DATA *myPs;
S_TEST_CONDITION *myTestCond;	//190901 lyhw

void *rt_task(void *arg) 
{
    int	mainSlot, rt_tic;
	long long ht0, ht1, ht2;
	struct sched_param p;

//	Initialize();

	switch(myPs->config.hwSpec) {
		case L_5V_20A:
			rt_tic = 500000; //0.5mS
			break;
		case S_5V_200A_75A_15A_AD2:
			rt_tic = 1000000; //2mS
			break;
		default:
			rt_tic = 1000000; //1mS
			break;
	}
	p.sched_priority = 1;
	pthread_setschedparam(pthread_self(), SCHED_FIFO, &p);
	pthread_make_periodic_np(pthread_self(), gethrtime(),rt_tic);
	pthread_setfp_np(pthread_self(), 1);
	myPs->misc.slot_periodic = (unsigned long)(rt_tic / 1000000); 	//210316
	
	while(1) {
		ht0 = gethrtime();
		mainSlot = myPs->misc.mainSlot++;
		myPs->misc.timer_usec++ ;

		switch(myPs->config.rt_scan_type) {
			case RT_SCAN_PERIOD_10mS: //140520 oys w : rt_scan_type_10mS	
				rt_slot_10mS_AD2(mainSlot);
				break;
			case RT_SCAN_PERIOD_20mS:
				switch(myPs->config.hwSpec) { //150413 lyh add for FAD
					case L_5V_150A_R3_AD2:
					case L_8CH_MAIN_AD2_P:
						rt_slot_20mS_AD2_FAD(mainSlot);
						break;
					case L_5V_600A_10A: //Pack Cycler
						rt_slot_20mS_Pack(mainSlot);
						break;
					default:
						rt_slot_20mS_AD2(mainSlot);
						break;
				}
				break;
			case RT_SCAN_PERIOD_25mS:
				rt_slot_25mS_AD2(mainSlot);
				break;
			case RT_SCAN_PERIOD_50mS:
				switch(myPs->config.hwSpec) { 
					case L_5V_500mA_2uA_R4:
						rt_slot_50mS_AD2_2uA(mainSlot);
						break;
					default:
						if(myPs->config.ADC_num >= 2){					
							rt_slot_50mS_AD2(mainSlot);					
						}else{
							rt_slot_50mS(mainSlot);
						}
						break;
				}
				break;
			case RT_SCAN_PERIOD_100mS_2:
				rt_slot_100mS_2(mainSlot);
				break;
			default:
				if(myPs->config.ADC_num >= 2){
					if(myPs->config.chPerBd > 32){
						rt_slot_100mS_AD2_64Ch(mainSlot);
					}else{
						rt_slot_100mS_AD2(mainSlot);
					}
				}else{
					rt_slot_100mS(mainSlot);
				}
				break;
		}
#if REAL_TIME == 1
		Sync_RTC();	// 131228 oys w : real time add
#endif
		cActiveSwitch();
		//210316 lyhw
		myPs->misc.slot_tic_timer += myPs->misc.slot_periodic;

		ht1 = gethrtime();
		pthread_wait_np();
		ht2 = gethrtime();
		module_runningTime(mainSlot, (long)(ht1-ht0), (long)(ht2-ht0));
    }

	return 0;
}

void *rt_task1(void *arg) 
{	//180611 add for digital
    int	mainSlot, rt_tic;
	long long ht0, ht1, ht2;
	struct sched_param p;

	//191118 lyhw -> intialize change init_module
	//Initialize();

	switch(myPs->config.hwSpec) {
		default:
			rt_tic = 1000000; //1mS
			break;
	}

	p.sched_priority = 1;
	pthread_setschedparam(pthread_self(), SCHED_FIFO, &p);
	pthread_make_periodic_np(pthread_self(), gethrtime(),rt_tic);
	pthread_setfp_np(pthread_self(), 1);
	myPs->misc.slot_periodic = (unsigned long)(rt_tic / 1000000); 	//210316
	
	while(1) {
		ht0 = gethrtime();
		mainSlot = myPs->misc.mainSlot++;
		myPs->misc.timer_usec++ ;

		switch(myPs->config.rt_scan_type){
			case RT_SCAN_PERIOD_10mS:
				rt_slot_10mS(mainSlot);
				break;
			case RT_SCAN_PERIOD_20mS:
				rt_slot_20mS(mainSlot);
				break;
			default:
				rt_slot_100mS_3(mainSlot);
				break;
		}
#if REAL_TIME == 1
		Sync_RTC();	// 131228 oys w : real time add
#endif
		ht1 = gethrtime();
		pthread_wait_np();
		ht2 = gethrtime();
		myPs->misc.slot_tic_timer += myPs->misc.slot_periodic;	//210316

		module_runningTime(mainSlot, (long)(ht1-ht0), (long)(ht2-ht0));
    }
	return 0;
}

void *rt_task2(void *arg) 
{	//210523 add for canType Cycler
    int	mainSlot, rt_tic;
	long long ht0, ht1, ht2;
	struct sched_param p;

	switch(myPs->config.hwSpec) {
		default:
			rt_tic = 1000000; //1mS
			break;
	}

	p.sched_priority = 1;
	pthread_setschedparam(pthread_self(), SCHED_FIFO, &p);
	pthread_make_periodic_np(pthread_self(), gethrtime(),rt_tic);
	pthread_setfp_np(pthread_self(), 1);
	myPs->misc.slot_periodic = (unsigned long)(rt_tic / 1000000); 	//210316
	
	while(1) {
		ht0 = gethrtime();
		mainSlot = myPs->misc.mainSlot++;
		myPs->misc.timer_usec++ ;
	
		switch(myPs->config.rt_scan_type) {
			case RT_SCAN_PERIOD_50mS:
				rt_slot_50mS_CAN(mainSlot);
				break;
			default:
				rt_slot_100mS_CAN(mainSlot);
				break;
		}
#if REAL_TIME == 1
		Sync_RTC();	// 131228 oys w : real time add
#endif
		ht1 = gethrtime();
		pthread_wait_np();
		ht2 = gethrtime();
		myPs->misc.slot_tic_timer += myPs->misc.slot_periodic;	//210316

		module_runningTime(mainSlot, (long)(ht1-ht0), (long)(ht2-ht0));
    }
	return 0;
}

void rt_slot_100mS(int mainSlot)
{
	if(mainSlot == 0 ){
	}else if(mainSlot >= 1 && mainSlot <= 72) { // 72 slot
    	AnalogValue_Input_100mS_Ch(mainSlot-1);
	}else if(mainSlot >=73 && mainSlot <= 76){ //  4 slot
	} else if(mainSlot >= 77 && mainSlot <= 80) { // 4 slot
		CalSourceAverage(mainSlot-77);
		CalSourceRingBuffer(mainSlot-77);
		CalibratorSource(mainSlot-77);
	} else if(mainSlot == 81) {
   		ModuleControl();
		GroupControl();
#if REAL_TIME == 1
//		Sync_RTC();	// 131228 oys w : real time add
#endif
	} else if(mainSlot == 82) {
		BoardControl();
	} else if(mainSlot >= 83 && mainSlot <= 93) { // 11 slot
	} else if(mainSlot >= 94 && mainSlot <= 97) { // 4 slot
		DInOutControl(mainSlot-94);
	} else if(mainSlot == 98) {
		RefreshWDT();
		Check_Message();
	} else if(mainSlot == 99) {
		sens_count_increment();
		timer_1sec_increment();
		shift_slot_increment();
		myPs->misc.mainSlot = 0;
	}
}

void	rt_slot_100mS_2(int mainSlot)
{
	if(mainSlot == 0 ){
	}else if(mainSlot >= 1 && mainSlot <= 136) {
    	AnalogValue_Input_100mS_Ch_2(mainSlot-1);
	}else if(mainSlot >=137 && mainSlot <= 140){
		CalSourceAverage(mainSlot-137);
		CalSourceRingBuffer(mainSlot-137);
		CalibratorSource(mainSlot-137);
	} else if(mainSlot == 141) {
   		ModuleControl();
		GroupControl();
#if REAL_TIME == 1
//		Sync_RTC();	// 131228 oys w : real time add
#endif		
	} else if(mainSlot == 142) {
		BoardControl();
	} else if(mainSlot >= 194 && mainSlot <= 197) {
		DInOutControl(mainSlot-194);
	} else if(mainSlot == 198) {
		Check_Message();
	} else if(mainSlot == 199) {
		sens_count_increment();
		timer_1sec_increment();
		shift_slot_increment();
		myPs->misc.mainSlot = 0;
	} else {
			//nothing else
	}
}

void	rt_slot_100mS_AD2_64Ch(int mainSlot)
{
	//int addr , base_addr , addr_step, ad_start, bd;

	if(mainSlot == 0 ){
		DAQ_Start();
	}else if(mainSlot >= 1 && mainSlot <= 68) { // 68 slot
    	AnalogValue_Input_100mS_Ch_AD2_64Ch(mainSlot-1);
	} else if(mainSlot >= 69 && mainSlot <= 72) { // 4 slot
		CalSourceAverage(mainSlot-69);
		CalSourceRingBuffer(mainSlot-69);
		CalibratorSource(mainSlot-69);
	} else if(mainSlot >= 45 && mainSlot <= 76) { // 32 slot
		if(myPs->config.FadBdUse == P1) //FAD BD USE
			FadEndCheck(mainSlot-45); //FAD END CHECK and DATA SEND
	} else if(mainSlot >= 73 && mainSlot <= 80) { // 4 slot
	} else if(mainSlot == 81) {
   		ModuleControl();
		GroupControl();
#if REAL_TIME == 1
//		Sync_RTC();	// 131228 oys w : real time add
#endif
	} else if(mainSlot == 82) {
		BoardControl();
	} else if(mainSlot >= 83 && mainSlot <= 93) { // 11 slot
		if(mainSlot == 83)	DAQ_Read(0);
		else if(mainSlot == 84)	DAQ_Read(1);
		else if(mainSlot == 85)	DAQ_Read(2);
		else if(mainSlot == 86)	DAQ_Read(3);
	} else if(mainSlot >= 94 && mainSlot <= 97) { // 4 slot
		DInOutControl(mainSlot-94);
	} else if(mainSlot == 98) {
		RefreshWDT();
		Check_Message();
	} else if(mainSlot == 99) {
/*		if(myPs->config.MainBdType == FPGA_TYPE) {
			base_addr = myPs->addr.main[BASE_ADDR];
			addr_step = myPs->addr.main[ADDR_STEP];
			ad_start = myPs->addr.main[AD_RC];
			for(bd = 0;bd < myPs->config.installedBd; bd++) {
				addr = base_addr + (addr_step * bd);
				outb(0x00 , addr+ad_start);
			}
		}*/
		sens_count_increment();
		timer_1sec_increment();
		shift_slot_increment();
		myPs->misc.mainSlot = 0;
	}
}

void	rt_slot_100mS_AD2(int mainSlot)
{
	//int addr , base_addr , addr_step, ad_start, bd;

	if(mainSlot == 0 ){
		DAQ_Start();
	}else if(mainSlot >= 1 && mainSlot < 37) { // 36 slot
    	AnalogValue_Input_50mS_Ch_AD2(mainSlot-1);
	}else if(mainSlot == 37) {
	} else if(mainSlot >= 38 && mainSlot < 41) { // 4 slot
	} else if(mainSlot >= 41 && mainSlot < 45) { // 4 slot
		CalSourceAverage(mainSlot-41);
		CalSourceRingBuffer(mainSlot-41);
		CalibratorSource(mainSlot-41);
	} else if(mainSlot >= 45 && mainSlot < 77) { // 32 slot
		if(myPs->config.FadBdUse == P1) //FAD BD USE
			FadEndCheck(mainSlot-45); //FAD END CHECK and DATA SEND
	} else if(mainSlot >= 77 && mainSlot < 81) { // 4 slot
	} else if(mainSlot == 81) {
   		ModuleControl();
		GroupControl();
#if REAL_TIME == 1
//		Sync_RTC();	// 131228 oys w : real time add
#endif
	} else if(mainSlot == 82) {
		BoardControl();
	} else if(mainSlot >= 83 && mainSlot < 94) { // 11 slot
		if(mainSlot == 83)	DAQ_Read(0);
		else if(mainSlot == 84)	DAQ_Read(1);
		else if(mainSlot == 85)	DAQ_Read(2);
		else if(mainSlot == 86)	DAQ_Read(3);
	} else if(mainSlot >= 94 && mainSlot < 98) { // 4 slot
		DInOutControl(mainSlot-94);
	} else if(mainSlot == 98) {
		RefreshWDT();
		Check_Message();
	} else if(mainSlot == 99) {
/*		if(myPs->config.MainBdType == FPGA_TYPE) {
			base_addr = myPs->addr.main[BASE_ADDR];
			addr_step = myPs->addr.main[ADDR_STEP];
			ad_start = myPs->addr.main[AD_RC];
			for(bd = 0;bd < myPs->config.installedBd; bd++) {
				addr = base_addr + (addr_step * bd);
				outb(0x00 , addr+ad_start);
			}
		}*/
		sens_count_increment();
		timer_1sec_increment();
		shift_slot_increment();
		myPs->misc.mainSlot = 0;
	}
}

void rt_slot_100mS_CAN(int mainSlot)
{
	int slot;

	if(mainSlot >= 0 && mainSlot < 32) {
		slot = mainSlot;
		if(slot % 4 == 0){
			CAN_Control(CAN_MAIN_LINK_CHECK, (slot/4));
			CAN_Control(CAN_MAIN_LINK_CHECK, ((slot+32)/4));
		}
	}else if(mainSlot >= 32 && mainSlot < 64) {
		slot = mainSlot - 32;
		if(slot % 4 == 0){
			CAN_Control(CAN_DATA_READ, (slot/4));
			CAN_Control(CAN_DATA_READ, (slot+32)/4);
		}
		AnalogValue_Input_100mS_CAN(slot);
		AnalogValue_Input_100mS_CAN(slot + 32);
	}else if(mainSlot >= 64 && mainSlot < 80) {
		slot = mainSlot - 64;
		if(slot % 4 == 0){
			CAN_Control(CAN_INV_LINK_CHECK, (slot/4));
			CAN_Control(CAN_IO_LINK_CHECK, (slot/4));
		}
		if(slot % 4 == 3){
			CAN_Control(CAN_DATA_READ, (slot/4));
		}
	} else if(mainSlot == 94) {
   		ModuleControl();
		GroupControl();
		BoardControl();
	} else if(mainSlot >= 95 && mainSlot < 99) {
		slot = mainSlot - 95;
		DInOutControl(slot % 4);
	} else if(mainSlot == 99) {
#if REAL_TIME == 1
//		Sync_RTC();
#endif
		RefreshWDT();
		Check_Message();
		sens_count_increment();
		timer_1sec_increment();
		shift_slot_increment();
		myPs->misc.mainSlot = 0;
	}
}

void rt_slot_50mS_CAN(int mainSlot)
{	//210203 add for 50mS Can Test;
	int slot;

	if(mainSlot >= 0 && mainSlot < 8) {
		slot = mainSlot;
		CAN_Control(CAN_MAIN_LINK_CHECK, slot);			//1~8 BD
		CAN_Control(CAN_MAIN_LINK_CHECK, (slot + 8));	//8~16 BD
		if(mainSlot < 4){
			DInOutControl(mainSlot % 4);
		}
	}else if(mainSlot == 8){
		slot = mainSlot - 8;
		CAN_Control(CAN_INV_LINK_CHECK, slot);
	}else if(mainSlot == 9){
		slot = mainSlot - 9;
		CAN_Control(CAN_IO_LINK_CHECK, slot);
	}else if(mainSlot >= 10 && mainSlot < 20){		//10Slot

	}else if(mainSlot >= 20 && mainSlot < 28){
		slot = mainSlot - 20;
		CAN_Control(CAN_DATA_READ, slot);
		CAN_Control(CAN_DATA_READ, (slot+8));
	}else if(mainSlot >= 29 && mainSlot < 45){ 
		slot = mainSlot - 29;
		AnalogValue_Input_50mS_CAN(slot);
		AnalogValue_Input_50mS_CAN(slot + 16);
		AnalogValue_Input_50mS_CAN(slot + 32);
		AnalogValue_Input_50mS_CAN(slot + 48);
	}else if(mainSlot == 45){
		ModuleControl();
		GroupControl();
		BoardControl();
	}else if(mainSlot >= 46 && mainSlot < 48){

	}else if(mainSlot == 48){
		RefreshWDT();
		Check_Message();
	}else if(mainSlot == 49){
#if REAL_TIME == 1
//		Sync_RTC();
#endif
		sens_count_increment();
		timer_1sec_increment();
		shift_slot_increment();
		myPs->misc.mainSlot = 0;

	}
}
	
//org 100 mS
/*
void	rt_slot_100mS(int mainSlot)
{
	if(mainSlot == 0 ){
	}else if(mainSlot >= 1 && mainSlot <= 72) { // 72 slot
    	AnalogValue_Input_100mS(mainSlot-1);
	}else if(mainSlot >=73 && mainSlot <= 76){ //  4 slot
		CalChAD_Filter(mainSlot-73);
		CalChAverage(mainSlot-73);
	} else if(mainSlot >= 77 && mainSlot <= 80) { // 4 slot
		CalSourceAverage(mainSlot-77);
		CalSourceRingBuffer(mainSlot-77);
		CalibratorSource(mainSlot-77);
	} else if(mainSlot == 81) {
   		ModuleControl();
		GroupControl();
	} else if(mainSlot == 82) {
		BoardControl();
		sens_ch_ad_count_increment();
	} else if(mainSlot >= 83 && mainSlot <= 93) { // 11 slot
    	ChannelControl_100mS(mainSlot-83);
	} else if(mainSlot >= 94 && mainSlot <= 97) { // 4 slot
		DInOutControl(mainSlot-94);
	} else if(mainSlot == 98) {
		RefreshWDT();
		Check_Message();
	} else if(mainSlot == 99) {
		sens_count_increment();
		timer_1sec_increment();
		shift_slot_increment();
		myPs->misc.mainSlot = 0;
	}
}
*/
/*
void	rt_slot_50mS(int mainSlot)
{
	if(mainSlot >= 0 && mainSlot <= 39) { // 40 Slot
    	AnalogValue_Input_50mS(mainSlot);
	}else if(mainSlot >= 40 && mainSlot <= 41){ //2slot
		CalChAD_Filter(mainSlot-40);
		CalChAverage(mainSlot-40);
		CalChAD_Filter(mainSlot-40+2);
		CalChAverage(mainSlot-40+2);
//	} else if(mainSlot >= 42 && mainSlot <= 43) {//2slot
		CalSourceAverage(mainSlot-40);
		CalSourceRingBuffer(mainSlot-40);
		CalibratorSource(mainSlot-40);
		CalSourceAverage(mainSlot-40+2);
		CalSourceRingBuffer(mainSlot-40+2);
		CalibratorSource(mainSlot-40+2);
	} else if(mainSlot == 42) {
   		ModuleControl();
		GroupControl();
		BoardControl();
		sens_ch_ad_count_increment_1();
	} else if(mainSlot >= 43 && mainSlot <= 46) {// 4 slot
    	ChannelControl_50mS(mainSlot-43);
	} else if(mainSlot >= 47 && mainSlot <= 48) { //2 slot
		DInOutControl(mainSlot-47);
		DInOutControl(mainSlot-47+1);
	} else if(mainSlot == 49) {
		RefreshWDT();
		Check_Message();
		sens_count_increment();
		timer_1sec_increment();
		shift_slot_increment();
		myPs->misc.mainSlot = 0;
	}
}
*/

void rt_slot_50mS(int mainSlot)
{
	if(mainSlot >= 0 && mainSlot <= 39) { // 40 slot
		AnalogValue_Input_50mS_Ch(mainSlot);
	} else if(mainSlot >= 40 && mainSlot <= 43) { // 4 slot
		CalSourceAverage(mainSlot-40);
		CalSourceRingBuffer(mainSlot-40);
		CalibratorSource(mainSlot-40);
	} else if(mainSlot == 44) {
   		ModuleControl();
		GroupControl();
#if REAL_TIME == 1
//		Sync_RTC();	// 131228 oys w : real time add
#endif
		BoardControl();
		RefreshWDT();
		Check_Message();
	} else if(mainSlot >= 45 && mainSlot <= 48) { // 4 slot
		DInOutControl(mainSlot-45);
	} else if(mainSlot == 49) {
		sens_count_increment();
		timer_1sec_increment();
		shift_slot_increment();
		myPs->misc.mainSlot = 0;
	}
}
void rt_slot_50mS_AD2(int mainSlot)
{
//	int addr , base_addr , addr_step, ad_start, bd;
	if(mainSlot >= 0 && mainSlot <= 35) { // 40 slot
		if(mainSlot == 0)
		{
			DAQ_Start();
		}
		AnalogValue_Input_50mS_Ch_AD2(mainSlot);
	} else if(mainSlot >= 36 && mainSlot <= 39) { // 4 slot
		CalSourceAverage(mainSlot-36);
		CalSourceRingBuffer(mainSlot-36);
		CalibratorSource(mainSlot-36);
	} else if(mainSlot >= 40 && mainSlot <= 42) { // 4 slot
		if(mainSlot == 40)	DAQ_Read(0);
		else if(mainSlot == 41)	DAQ_Read(1);
	} else if(mainSlot == 43) {
   		ModuleControl();
		GroupControl();
#if REAL_TIME == 1
//		Sync_RTC();	// 131228 oys w : real time add
#endif
		BoardControl();
		RefreshWDT();
		Check_Message();
	} else if(mainSlot >= 44 && mainSlot <= 47) { // 4 slot
		DInOutControl(mainSlot-44);
	} else if(mainSlot == 48) {
		DAQ_Read(2);
	} else if(mainSlot == 49) {
	/*	if(myPs->config.MainBdType == FPGA_TYPE) {
			base_addr = myPs->addr.main[BASE_ADDR];
			addr_step = myPs->addr.main[ADDR_STEP];
			ad_start = myPs->addr.main[AD_RC];
			for(bd = 0;bd < myPs->config.installedBd; bd++) {
				addr = base_addr + (addr_step * bd);
				outb(0x00 , addr+ad_start);
			}
		}*/
		DAQ_Read(3);
		sens_count_increment();
		timer_1sec_increment();
		shift_slot_increment();
		myPs->misc.mainSlot = 0;
	}
}

void rt_slot_50mS_AD2_2uA(int mainSlot)
{	//210510 add for 500mA/2uA Type Cycler
	if(mainSlot >= 0 && mainSlot <= 35) { // 40 slot
		if(mainSlot == 0)
		{
			DAQ_Start();
		}
		AnalogValue_Input_50mS_Ch_AD2_2uA(mainSlot);
	} else if(mainSlot >= 36 && mainSlot <= 39) { // 4 slot
		CalSourceAverage(mainSlot-36);
		CalSourceRingBuffer(mainSlot-36);
		CalibratorSource(mainSlot-36);
	} else if(mainSlot >= 40 && mainSlot <= 42) { // 4 slot
		if(mainSlot == 40)	DAQ_Read(0);
		else if(mainSlot == 41)	DAQ_Read(1);
	} else if(mainSlot == 43) {
   		ModuleControl();
		GroupControl();
		BoardControl();
		RefreshWDT();
		Check_Message();
	} else if(mainSlot >= 44 && mainSlot <= 47) { // 4 slot
		DInOutControl(mainSlot-44);
	} else if(mainSlot == 48) {
		DAQ_Read(2);
	} else if(mainSlot == 49) {
		DAQ_Read(3);
		sens_count_increment();
		timer_1sec_increment();
		shift_slot_increment();
		myPs->misc.mainSlot = 0;
	}
}

void rt_slot_25mS_AD2(int mainSlot)
{
	//DAQ Test Check!!
	int i;	

	if(mainSlot >= 0 && mainSlot <= 18) { 
		if(mainSlot == 0) DAQ_Start();

		AnalogValue_Input_25mS_Ch_AD2(mainSlot);

		if(mainSlot == 16)	DAQ_Read(0);
		else if(mainSlot == 17)	DAQ_Read(1);
		else if(mainSlot == 18)	DAQ_Read(2);
	} else if(mainSlot == 19) {
		for(i=0; i < 4; i++) {
			CalSourceAverage(i);
			CalSourceRingBuffer(i);
			CalibratorSource(i);
		}

		for(i=0; i < 4; i++) {
			DInOutControl(i);
		}
   		ModuleControl();
		GroupControl();

#if REAL_TIME == 1
//		Sync_RTC();	//131228 oys w : real time add
#endif

		BoardControl();

		RefreshWDT();
		Check_Message();
		DAQ_Read(3);

		sens_count_increment();
		timer_1sec_increment();
		shift_slot_increment();
		myPs->misc.mainSlot = 0;
	}
}

void rt_slot_20mS_AD2(int mainSlot)
{
//	int addr , base_addr , addr_step, ad_start, bd;
	if(mainSlot >= 0 && mainSlot <= 11) { // 12 slot
		if(mainSlot == 0)
		{
			DAQ_Start();
		}
		AnalogValue_Input_20mS_Ch_AD2(mainSlot);
	} else if(mainSlot >= 12 && mainSlot <= 15) { // 4 slot
		CalSourceAverage(mainSlot-12);
		CalSourceRingBuffer(mainSlot-12);
		CalibratorSource(mainSlot-12);
		DInOutControl(mainSlot-12);
	} else if(mainSlot == 16) {
   		ModuleControl();
		GroupControl();
#if REAL_TIME == 1
//		Sync_RTC();	// 131228 oys w : real time add
#endif
		DAQ_Read(0);	//211116_hun
	} else if(mainSlot == 17) {
		BoardControl();
		DAQ_Read(1);	//211116_hun
	} else if(mainSlot == 18) {
		RefreshWDT();
		Check_Message();
		DAQ_Read(2);	//211116_hun
	} else if(mainSlot == 19) {
		DAQ_Read(3);	//211116_hun
		sens_count_increment();
		timer_1sec_increment();
		shift_slot_increment();
		myPs->misc.mainSlot = 0;
		/*if(myPs->config.MainBdType == FPGA_TYPE) {
			base_addr = myPs->addr.main[BASE_ADDR];
			addr_step = myPs->addr.main[ADDR_STEP];
			ad_start = myPs->addr.main[AD_RC];
			for(bd = 0;bd < myPs->config.installedBd; bd++) {
				addr = base_addr + (addr_step * bd);
				outb(0x00 , addr+ad_start);
			}
		}*/
	}
}

void	rt_slot_20mS_AD2_FAD(int mainSlot)
{
	//DAQ Test Check!!
	if(mainSlot >= 0 && mainSlot <= 10) { // 11 slot
		if(mainSlot >= 1 && mainSlot <= 5){
			DInOutControl(mainSlot-1);
		}
		if(mainSlot == 0)
		{
			DAQ_Start();
		}
		if(mainSlot == 1){
			ModuleControl();
			GroupControl();
#if REAL_TIME == 1
//			Sync_RTC();	// 131228 oys w : real time add
#endif
		} else if(mainSlot == 2){
			BoardControl();
		}
		AnalogValue_Input_20mS_Ch_AD2_FAD(mainSlot);
		if(mainSlot == 10){
			CalSourceAverage(0);
			CalSourceRingBuffer(0);
			CalibratorSource(0);
		}
	}else if(mainSlot >= 11 && mainSlot < 19){ //8slot
		if(myPs->config.FadBdUse == P1) //FAD BD USE
			FadEndCheck(mainSlot-11); //FAD END CHECK and DATA SEND
			
		if(mainSlot == 16) DAQ_Read(0);	//211116_hun
		else if(mainSlot == 17) DAQ_Read(1);	//211116_hun
		else if(mainSlot == 18) DAQ_Read(2);	//211116_hun
	}else if(mainSlot == 19){
		RefreshWDT();
		Check_Message();
		DAQ_Read(3);	//211116_hun
		sens_count_increment();
		timer_1sec_increment();
		myPs->misc.mainSlot = 0;
	}
}

void	rt_slot_20mS_Pack(int mainSlot)
{
	if(mainSlot >= 0 && mainSlot <= 11) { // 12 slot
		if(mainSlot == 0)
		{
			DAQ_Start();
		}
		AnalogValue_Input_20mS_Pack(mainSlot);
} else if(mainSlot >= 12 && mainSlot <= 15) { // 4 slot
		CalSourceAverage(mainSlot-12);
		CalSourceRingBuffer(mainSlot-12);
		CalibratorSource(mainSlot-12);
		DInOutControl(mainSlot-12);
	} else if(mainSlot == 16) {
   		ModuleControl();
		GroupControl();
		DAQ_Read(0);	
#if REAL_TIME == 1
//		Sync_RTC();	// 131228 oys w : real time add
#endif
	} else if(mainSlot == 17) {
		BoardControl();
		DAQ_Read(1);	
	} else if(mainSlot == 18) {
		RefreshWDT();
		Check_Message();
		DAQ_Read(2);	
	} else if(mainSlot == 19) {
		DAQ_Read(3);	
		sens_count_increment();
		timer_1sec_increment();
		shift_slot_increment();
		myPs->misc.mainSlot = 0;
	}
}

//140520 oys w: rt_scan_time_10mS
void rt_slot_10mS_AD2(int mainSlot)
{
	int i;
	if(mainSlot >= 0 && mainSlot <= 7) { // 8 slot
		if(mainSlot == 0)
		{
			DAQ_Start();
		}
		AnalogValue_Input_10mS_Ch_AD2(mainSlot);
	} else if(mainSlot == 8) {
		AnalogValue_Input_10mS_Ch_AD2(mainSlot);
		for(i = 0; i < 4; i++){
			DInOutControl(i);
		}
		Check_Message();
   		ModuleControl();
		GroupControl();
		if(myPs->misc.daq_slot == 0) DAQ_Read(0);
		else if(myPs->misc.daq_slot == 1) DAQ_Read(2);
	} else if(mainSlot == 9) {
		AnalogValue_Input_10mS_Ch_AD2(mainSlot);
#if REAL_TIME == 1
//		Sync_RTC();	// 131228 oys w : real time add
#endif
		BoardControl();
		RefreshWDT();
		for(i = 0; i <MAX_BD_PER_MODULE; i++){
			CalSourceAverage(i);
			CalSourceRingBuffer(i);
			CalibratorSource(i);
		}
		if(myPs->misc.daq_slot == 0) DAQ_Read(1);
		else if(myPs->misc.daq_slot == 1) DAQ_Read(3);
		sens_count_increment();
		timer_1sec_increment();
		shift_slot_increment();
		myPs->misc.daq_slot++;
		if(myPs->misc.daq_slot == 2) myPs->misc.daq_slot = 0;
		myPs->misc.mainSlot = 0;
	}
}

void rt_slot_10mS(int mainSlot)
{	//180611 add for digital
	if(mainSlot >= 0 && mainSlot < 8) { // 8 slot
		if(mainSlot == 0) DAQ_Start();	//211116_hun
		PCU_Control(0, mainSlot);
		PCU_Control(0, mainSlot+8);
		PCU_Control(0, mainSlot+16);
		PCU_Control(0, mainSlot+24);
		if(mainSlot < 4){
			DInOutControl(mainSlot % 4);
		}
	} else{ 
		if(mainSlot == 8) {
   			ModuleControl();
			RefreshWDT();
			Check_Message();
			if(myPs->misc.daq_slot == 0) DAQ_Read(0);
			else if(myPs->misc.daq_slot == 1) DAQ_Read(2);
		} else if(mainSlot == 9) {
#if REAL_TIME == 1
			//Sync_RTC();	// 131228 oys w : real time add
#endif
			if(myPs->misc.daq_slot == 0) DAQ_Read(1);
			else if(myPs->misc.daq_slot == 1) DAQ_Read(3);
			sens_count_increment();
			timer_1sec_increment();
			shift_slot_increment();
			myPs->misc.daq_slot++;
			if(myPs->misc.daq_slot == 2) myPs->misc.daq_slot = 0;
			myPs->misc.mainSlot = 0;
		}
	}
}

void rt_slot_20mS(int mainSlot)
{	//200425 lyhw
	//DAQ Test Check!!
	if(mainSlot == 0){
		outb(0x02, 0x600);	//0~32 BD Select
		DAQ_Read(0);	
	}else if(mainSlot >= 1 && mainSlot < 9) { // 8 slot
		mainSlot = mainSlot - 1;
		PCU_Control(0, mainSlot+32);
		PCU_Control(0, mainSlot+40);
		PCU_Control(0, mainSlot+48);
		PCU_Control(0, mainSlot+56);
		if(mainSlot < 4){
			DInOutControl(mainSlot % 4);
		}
	}else if(mainSlot == 9){
		outb(0x01, 0x600);	//33 ~64 Extend BD Select
		DAQ_Read(1);	
	}else if(mainSlot >= 10 && mainSlot < 18){
		mainSlot = mainSlot - 10;
		PCU_Control(0, mainSlot);
		PCU_Control(0, mainSlot+8);
		PCU_Control(0, mainSlot+16);
		PCU_Control(0, mainSlot+24);
	}else{
		if(mainSlot == 18) {
   			ModuleControl();
			RefreshWDT();
			Check_Message();
			DAQ_Read(2);	//211116_hun
		} else if(mainSlot == 19) {
			DAQ_Read(3);	//211116_hun
#if REAL_TIME == 1
		//	Sync_RTC();	// 131228 oys w : real time add
#endif
			sens_count_increment();
			timer_1sec_increment();
			shift_slot_increment();
			myPs->misc.mainSlot = 0;
		}
	}
}

void rt_slot_100mS_3(int mainSlot)
{	//180611 add for digital
	if(mainSlot >= 0 && mainSlot < 16) { // 16 slot
		PCU_Control(0, mainSlot);
		PCU_Control(0, mainSlot+16);
	} else{ 
		if(mainSlot == 16) {
   			ModuleControl();
			GroupControl();
		}else if(mainSlot == 17) {
			BoardControl();
		}else if(mainSlot == 18) {
			RefreshWDT();
			Check_Message();
		} else if(mainSlot == 19) {
#if REAL_TIME == 1
		//	Sync_RTC();	// 131228 oys w : real time add
#endif
			sens_count_increment();
			timer_1sec_increment();
			shift_slot_increment();
			myPs->misc.mainSlot = 0;
		}
		DInOutControl(mainSlot-16);
	}
}

int init_module(void)
{
	int i, hwPort, bps, id_type, sjw = 0, group = 0;

	if(Open_SystemMemory(0) < 0) return -1;

	Initialize();

	if(myData->CAN.config.canUseFlag == P1){
		Can_Group_Select(group);
		//used_rt_can
		for(i=0; i < myData->CAN.config.installedCAN; i++) {
			if(myData->CAN.config.canPort[i] == 0){//no use port not setting
				continue;
			}
			hwPort = myData->CAN.config.canPort[i] - 1;
			bps = myData->CAN.config.canBps[i];
			id_type = myData->CAN.config.commType[i];
			rt_can_set_param(hwPort, bps, sjw, id_type, 0);
		}
	}
	
#if CYCLER_TYPE == DIGITAL_CYC //180611 lyh Digital Cycler
	return pthread_create(&thread, NULL, rt_task1, 0);
#elif CYCLER_TYPE == CAN_CYC
	return pthread_create(&thread, NULL, rt_task2, 0);
#else
	return pthread_create(&thread, NULL, rt_task, 0);
#endif
}

void cleanup_module(void)
{
	pthread_delete_np(thread);

	DisableWDT();
	Clear_OutPort();
	myData->AppControl.signal[APP_SIG_MODULE_CONTROL_PROCESS] = P3;

	Close_SystemMemory();
}
