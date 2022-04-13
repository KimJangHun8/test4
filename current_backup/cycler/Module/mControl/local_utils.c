#include <asm/io.h>
#include <rtl_core.h>
#include <unistd.h>
#include <time.h>
#include "../rt_can/rt_can.h" //rt_can
#include "../../INC/datastore.h"
#include "message.h"
#include "rtTask.h"
#include "local_utils.h"
#include "DAQ.h"

extern S_SYSTEM_DATA *myData;
extern S_MODULE_DATA *myPs;

//131228 oys w : real time add
#if REAL_TIME == 1
struct tm
{
	int tm_sec;		//Seconds.		[0-60]	(1 leap second)
	int tm_min;		//Minutes.		[-059]
	int tm_hour;	//Hours.		[0-23]
	int tm_mday;	//Day.			[1-31]
	int tm_mon;		//Month.		[0-11]
	int	tm_year;	//Year-1900.
	int tm_wday;	//Day of week.	[0-6]
	int tm_yday;	//Day in year.	[0-365]
	int tm_isdst;	//DST.			[-1/0/1]
	
	long int tm_gmtoff;		//we don't care, we count from GMT
	const char *tm_zone;	//we don't care, we count from GMT
};
#define SPD 24*60*60
//add end
#endif

void Initialize(void)
{
	myPs = &(myData->mData);
	InitSystemMemory();

	outb(0x01, 0x601); //reset Enable
//	CheckWDT();//khkw
	EnableWDT();
	
	myData->AppControl.signal[APP_SIG_MODULE_CONTROL_PROCESS] = P1;
//	131228 oys w : real_time_sync
#if REAL_TIME == 1
	myData->MainClient.signal[MAIN_SIG_SEND_REAL_TIME_REQUEST] = P1;
#endif
}

void InitSystemMemory(void)
{
	int bd, ch, inv, i, j, addr, base_addr, addr_step, fad_ch, installedInv;
	long long flag;

	myPs->misc.mainSlot = 0;
	myPs->misc.shiftSlot = 0;
	myPs->misc.timer_1sec_count = 0;
	myPs->misc.timer_1sec = 0;
	myPs->misc.timer_usec = 0;
	myData->daq.misc.daq_read_delay = 0;
	myPs->misc.realTime_sec = 0; 
	myPs->misc.slot_tic_timer = 0; 	//msec 210316

	myPs->misc.sensCount = 0;
	myPs->misc.sensCountFlag = P0;
	myPs->misc.sourceSensCount = 0;
	myPs->misc.sourceSensCountFlag = P0;
	myPs->misc.processPointer = (int)&myData;

	installedInv = myData->CAN.config.installedInverter;

	myPs->code = FAIL_NONE;
	for(i=0; i < MAX_SIGNAL; i++) {
		myPs->signal[i] = P0;
	}
	//180611 add for digital INV Power
	switch(myPs->config.hwSpec){
		case DC_5V_150A_PARA:
			myPs->signal[M_SIG_INV_POWER] = P1;
			break;
		case DC_5V_CYCLER_NEW:
			myPs->signal[M_SIG_INV_POWER1] = P1;
			break;
		case C_5V_CYCLER_CAN:	//210204 lyhw
			myPs->signal[M_SIG_INV_POWER_CAN] = P1;
			break;
		default:
			break;	
	}

	myPs->signal[M_SIG_RUN_LED] = P1;
	myPs->signal[M_SIG_REMOTE_SMPS1] = P1;  
	myPs->signal[M_SIG_FAN_RELAY] = P1;

	for(bd=0; bd < MAX_BD_PER_MODULE; bd++) {
		for(ch=0; ch < MAX_CH_PER_BD; ch++) {
			myData->bData[bd].cData[ch].misc.sensCount = 0;
			myData->bData[bd].cData[ch].misc.sensCountFlag = P0;
		}
		//MAIN BD CAN COMM ERROR CHECK INITIALIZE
		myData->CAN.main[bd].comm.pingOutCnt = 0; //20190527 KHK
		myData->CAN.main[bd].comm.StateFlag = P0; //20190527 KHK
		myData->bData[bd].signal[B_SIG_MAIN_CAN_COMM_ERROR] = P0;
	}
	
	//INVERTER BD CAN COMM ERROR CHECK INITIALIZE
	for(inv=0; inv < installedInv; inv++) {
		myData->CAN.inverter[inv].signal[CAN_SIG_INV_CAN_COMM_ERROR] = P0;
		myData->CAN.inverter[inv].comm.pingOutCnt = 0;
		myData->CAN.inverter[inv].comm.StateFlag = 0;
	}

	//IO BD CAN COMM ERROR CHECK INITIALIZE
	myData->CAN.io.signal[CAN_SIG_IO_CAN_COMM_ERROR] = P0;
	myData->CAN.io.comm.pingOutCnt = 0;
	myData->CAN.io.comm.StateFlag = 0;
	
	flag = 1;
	for(i=0; i < myPs->config.installedBd; i++) {
		myData->bData[i].signal[B_SIG_HW_FAULT_CLEAR] = P1;
		for(j=0; j < myPs->config.chPerBd; j++) {
			if(myData->bData[i].cData[j].op.state == C_IDLE
				|| myData->bData[i].cData[j].op.state == C_STANDBY
				|| myData->bData[i].cData[j].op.state == C_CALI) {
				myData->bData[i].cData[j].op.state = C_IDLE;
				myData->bData[i].cData[j].op.phase = P0;
			}
			if(myData->bData[i].cData[j].op.state == C_PAUSE) {
				if(myData->bData[i].cData[j].op.code != C_FAULT_PAUSE_CMD) {
					myPs->misc.buzzerFlag1 |= flag; 
				}
			}
			flag = flag << 1;
		}
	}

	for(i=0; i < 7; i++) {
		for(j=0; j < 200; j++) {
			if(i == 3 || i == 5) myPs->runningTime[i][j] = 100000;
			else myPs->runningTime[i][j] = 0;
		}
	}

	switch(myPs->config.rt_scan_type) {
		case RT_SCAN_PERIOD_10mS:
			myPs->misc.rt_scan_time = 1;
			myPs->misc.dio_scan_time = 10;
			break;
		case RT_SCAN_PERIOD_20mS:
			myPs->misc.rt_scan_time = 2;
			myPs->misc.dio_scan_time = 20;
			break;
		case RT_SCAN_PERIOD_25mS:
			myPs->misc.rt_scan_time = 2;
			myPs->misc.dio_scan_time = 20;
			break;
		case RT_SCAN_PERIOD_50mS:
			myPs->misc.rt_scan_time = 5;
			myPs->misc.dio_scan_time = 50;
			break;
		default:
			myPs->misc.rt_scan_time = 10;
			myPs->misc.dio_scan_time = 100;
			break;
	}
	
	if(myPs->config.FadBdUse == 1) {
		base_addr = myPs->addr.main[BASE_ADDR];
		addr_step = myPs->addr.main[ADDR_STEP];

		for(bd=0; bd < myPs->config.installedBd; bd++) {
			for(i=0; i < myPs->config.chPerBd; i++) {
				fad_ch = myPs->addr.main[FAD_CH] + i;
				addr = base_addr + (bd * addr_step) + fad_ch;
			//	outb(0x00,addr);
				myData->bData[bd].cData[i].misc.fadFlag = 4;
			}
		}
	}

	//compare bd error clear
	if(myPs->config.hwSpec == L_5V_150A_R2_P) {
		outb(0xFF, 0x666);
		outb(0xFF, 0x6A6);
	}
	
	//210316 Get System time
	getTime_RTC();
}

void timer_1sec_increment(void)
{
	myPs->misc.timer_1sec_count++;
	myPs->misc.timer_mSec += myPs->misc.rt_scan_time * 10;

	switch(myPs->config.rt_scan_type) {
		case RT_SCAN_PERIOD_10mS:
			if(myPs->misc.timer_1sec_count >= 100) {
				myPs->misc.timer_1sec_count = 0;
				myPs->misc.timer_1sec++;
			}
			break;
		case RT_SCAN_PERIOD_20mS:
			if(myPs->misc.timer_1sec_count >= 50) {
				myPs->misc.timer_1sec_count = 0;
				myPs->misc.timer_1sec++;
			}
			break;
		case RT_SCAN_PERIOD_25mS:
			if(myPs->misc.timer_1sec_count >= 50) {
				myPs->misc.timer_1sec_count = 0;
				myPs->misc.timer_1sec++;
			}
			break;
		case RT_SCAN_PERIOD_50mS:
			if(myPs->misc.timer_1sec_count >= 20) {
				myPs->misc.timer_1sec_count = 0;
				myPs->misc.timer_1sec++;
			}
			break;
		default:
			if(myPs->misc.timer_1sec_count >= 10) {
				myPs->misc.timer_1sec_count = 0;
				myPs->misc.timer_1sec++;
			}
			break;
	}/*
	if(myPs->config.rt_scan_type == RT_SCAN_PERIOD_25mS)
	{
		if(myPs->misc.rt_scan_time == 2)
			myPs->misc.rt_scan_time = 3;
		else if(myPs->misc.rt_scan_time == 3)
			myPs->misc.rt_scan_time = 2;
	}*/
}

void shift_slot_increment(void)
{
	switch(myPs->config.hwSpec) {
		case L_5V_3A:
		case L_6V_6A:
		case L_5V_2A:
		case L_5V_200A:
		case L_5V_5A:
		case L_5V_30A:
		case L_2V_60A:
		case L_2V_100A:
		case S_5V_200A:
		case L_5V_5A_2:
		case L_5V_50A:
			if(myPs->config.ADC_type == MICRO) {
				myPs->misc.shiftSlot += 4;
				if(myPs->misc.shiftSlot >= 16) myPs->misc.shiftSlot = 0;
			} else { //common adc
				myPs->misc.shiftSlot += 4;
				if(myPs->misc.shiftSlot >= 16) myPs->misc.shiftSlot = 0;
			}
			break;
		default: //L_5V_10mA
			if(myPs->config.ADC_type == MICRO) {
				myPs->misc.shiftSlot += 8;
				if(myPs->misc.shiftSlot >= 32) myPs->misc.shiftSlot = 0;
			} else { //common adc
				myPs->misc.shiftSlot += 8;
				if(myPs->misc.shiftSlot >= 32) myPs->misc.shiftSlot = 0;
			}
			break;
	}
}

void sens_count_increment(void)
{
	myPs->misc.sensCount++;
	if(myPs->misc.sensCount >= MAX_FILTER_AD_COUNT) {
		myPs->misc.sensCount = 0;
	}

	if(myPs->misc.timer_1sec >= 2) {
		myPs->misc.sourceSensCount++;
		if(myPs->misc.sourceSensCount >= MAX_SOURCE_SENS_COUNT) {
			myPs->misc.sourceSensCount = 0;
			myPs->misc.sourceSensCountFlag = P1;
		}
	}
}

void sens_ch_ad_count_increment(int bd, int ch)
{ //kjg_180521
	int parallel_ch;

	if(myData->bData[bd].cData[ch].misc.parallel_cycle_phase == P50) {
		if((ch % 2) == 0) parallel_ch = ch + 1;
		else parallel_ch = ch - 1;

		sens_ch_ad_count_increment_default(bd, ch);
		sens_ch_ad_count_increment_default(bd, parallel_ch);
	} else {
		sens_ch_ad_count_increment_default(bd, ch);
	}
}

//20181030 modify filtering
void sens_ch_ad_count_increment_default(int bd, int ch)
{
	int tmpCount, flag;

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
	tmpCount = MAX_FILTER_AD_COUNT;

#if CYCLER_TYPE == LINEAR_CYC || CYCLER_TYPE == CAN_CYC
	myData->bData[bd].cData[ch].misc.sensCount++;
#endif
#if CYCLER_TYPE == DIGITAL_CYC
	if(myData->bData[bd].cData[ch].op.runTime > 3){	//180717 Check after 30mS
		myData->bData[bd].cData[ch].misc.sensCount++;
	}
#endif
	if(myData->mData.config.function[F_SENS_COUNT_TYPE] == P0) {
		if(myData->bData[bd].cData[ch].misc.sensCount >= tmpCount) {
			myData->bData[bd].cData[ch].misc.sensCount = 0;
		}
	} else if(myData->mData.config.function[F_SENS_COUNT_TYPE] == P1) {
		if(myData->bData[bd].cData[ch].misc.sensCount >= tmpCount) {
			myData->bData[bd].cData[ch].misc.sensCount = 0;
			flag = P1;
		}
	} else if(myData->mData.config.function[F_SENS_COUNT_TYPE] == P2) {
		if(flag == P0) {	
			if(myData->bData[bd].cData[ch].misc.sensCount >= tmpCount) {
				myData->bData[bd].cData[ch].misc.sensCount = 0;
				flag = P1;
			}
		}
		if(flag == P1) {	
			if(myData->bData[bd].cData[ch].misc.sensCount >= tmpCount) {
				myData->bData[bd].cData[ch].misc.sensCount = 0;
			}
		}
	} else {
		if(myData->bData[bd].cData[ch].misc.sensCount >= tmpCount) {
			myData->bData[bd].cData[ch].misc.sensCount = 0;
			flag = P1;
		}
	}

	if(flag >= P1) {
		myData->bData[bd].cData[ch].misc.sensBufCount++;
		if(myData->bData[bd].cData[ch].misc.sensBufCount >= tmpCount) {
			myData->bData[bd].cData[ch].misc.sensBufCount = 0;
			myData->bData[bd].cData[ch].misc.sensBufCountFlag = P1;
		}
	}
	myData->bData[bd].cData[ch].misc.sensCountFlag = flag;
}
/*
void sens_ch_ad_count_increment(int bd, int ch)
{
	myData->bData[bd].cData[ch].misc.sensCount++;
	// 101001 kji ad filter
	//	if(myData->bData[bd].cData[ch].misc.sensCount >= MAX_FILTER_AD_COUNT){
	//25ms 인경우 rt_scan_time이 2->3->2로 변하기때문에
	//rt_scan_time 변수 사용
//	if(myData->bData[bd].cData[ch].misc.sensCount >= 
//			(MAX_FILTER_AD_COUNT / myPs->misc.rt_scan_time)){
	// 150723 khk add for filltering			
	if(myData->bData[bd].cData[ch].misc.sensCount >= MAX_FILTER_AD_COUNT){
		myData->bData[bd].cData[ch].misc.sensCountFlag = P1;
		myData->bData[bd].cData[ch].misc.sensCount = 0;
	}
//	if(myData->bData[bd].cData[ch].misc.sensCountFlag == P1) {
//		if(myData->bData[bd].cData[ch].misc.sensCount >= MAX_FILTER_AD_COUNT){
//			myData->bData[bd].cData[ch].misc.sensCount = 0;
//			myData->bData[bd].cData[ch].misc.sensCountFlag = P2;
//		}
//	}
//	if(myData->bData[bd].cData[ch].misc.sensCountFlag == P2) {
//		if(myData->bData[bd].cData[ch].misc.sensCount >= MAX_FILTER_AD_COUNT){
//			myData->bData[bd].cData[ch].misc.sensCount = 0;
//		}
//	}
	if(myData->bData[bd].cData[ch].misc.sensCountFlag >= P1) {
		myData->bData[bd].cData[ch].misc.sensBufCount++;
		if(myData->bData[bd].cData[ch].misc.sensBufCount >= MAX_TMP_AD_COUNT){
			myData->bData[bd].cData[ch].misc.sensBufCount = 0;
			myData->bData[bd].cData[ch].misc.sensBufCountFlag = P1;
		}
	}
}
*/
void sens_ch_ad_count_increment_1(void)
{
	int bd, ch, i;

	for(i=0; i < myPs->config.installedCh; i++) {
		bd = i / myPs->config.chPerBd;
		ch = i % myPs->config.chPerBd;

		myData->bData[bd].cData[ch].misc.sensCount++;
		if(myData->bData[bd].cData[ch].misc.sensCount >= MAX_FILTER_AD_COUNT) {
			myData->bData[bd].cData[ch].misc.sensCount = 0;
			myData->bData[bd].cData[ch].misc.sensCountFlag = P1;
		}
	}
}

void module_runningTime(int index, long val1, long val2)
{
	if(myPs->misc.timer_1sec <= 4) return;

	myPs->runningTime[0][index] = val1;
	myPs->runningTime[1][index] = val2;

	if(myPs->runningTime[0][index] > myPs->runningTime[2][0]) {
		myPs->runningTime[2][0] = myPs->runningTime[0][index];
		myPs->runningTime[2][1] = (long)index;
	}
	if(myPs->runningTime[0][index] > myPs->runningTime[6][index]) {
		myPs->runningTime[6][index] = myPs->runningTime[0][index];
	}
	if(myPs->runningTime[0][index] < myPs->runningTime[3][0]) {
		myPs->runningTime[3][0] = myPs->runningTime[0][index];
		myPs->runningTime[3][1] = (long)index;
	}

	if(myPs->runningTime[1][index] > myPs->runningTime[4][0]) {
		myPs->runningTime[4][0] = myPs->runningTime[1][index];
		myPs->runningTime[4][1] = (long)index;
	}

	if(myPs->runningTime[1][index] < myPs->runningTime[5][0]) {
		myPs->runningTime[5][0] = myPs->runningTime[1][index];
		myPs->runningTime[5][1] = (long)index;
	}
}

//131228 oys w : real time add
#if REAL_TIME == 1
void make_localtime(long timepr, struct tm *r)
{	//210316 lyhw
	int j;
	time_t i;
	time_t timep;
	extern struct timezone sys_tz;
	const unsigned int __spm[12] = {
		0,
		(31),
		(31+28),
		(31+28+31),
		(31+28+31+30),
		(31+28+31+30+31),
		(31+28+31+30+31+30),
		(31+28+31+30+31+30+31),
		(31+28+31+30+31+30+31+31),
		(31+28+31+30+31+30+31+31+30),
		(31+28+31+30+31+30+31+31+30+31),
		(31+28+31+30+31+30+31+31+30+31+30),
	};
	const unsigned int __spm2[12] = {
		0,
		(31),
		(31+29),
		(31+29+31),
		(31+29+31+30),
		(31+29+31+30+31),
		(31+29+31+30+31+30),
		(31+29+31+30+31+30+31),
		(31+29+31+30+31+30+31+31),
		(31+29+31+30+31+30+31+31+30),
		(31+29+31+30+31+30+31+31+30+31),
		(31+29+31+30+31+30+31+31+30+31+30),
	};
	register time_t work;

	timep = (timepr) - (sys_tz.tz_minuteswest * 60);
 	work = timep % (SPD);
	r->tm_sec = work % 60;
	work /= 60;
	r->tm_min = work % 60;
	r->tm_hour = work / 60;
	work = timep / (SPD);
	r->tm_wday = (4 + work) % 7;
	for (i=1970; ; ++i) {
		register time_t k = (!(i%4) && ((i%100) || !(i%400))) ? 366 : 365;
		if (work > k) {
			work -= k;
			j = (int)k;
		} else {
			j = (int)k;
			break;
		}
	}
	r->tm_year = i - 1900;
	if(j == 366) {
		for (i=11; i && __spm2[i] > work; --i) ;
			r->tm_mon = i;
			r->tm_mday = work - __spm2[i] + 1;
	} else {
		for (i=11; i && __spm[i] > work; --i) ;
			r->tm_mon = i;
			r->tm_mday = work - __spm[i] + 1;
	}
	if((r->tm_mon+1) == 12 && r->tm_mday == 32) {
		r->tm_year += 1;
		r->tm_mon = 0;
		r->tm_mday = 1;
	}
}

void localtime(const time_t *timepr, struct tm *r)
{
	int j;
	time_t i;
	time_t timep;
	extern struct timezone sys_tz;
	const unsigned int __spm[12] = {
		0,
		(31),
		(31+28),
		(31+28+31),
		(31+28+31+30),
		(31+28+31+30+31),
		(31+28+31+30+31+30),
		(31+28+31+30+31+30+31),
		(31+28+31+30+31+30+31+31),
		(31+28+31+30+31+30+31+31+30),
		(31+28+31+30+31+30+31+31+30+31),
		(31+28+31+30+31+30+31+31+30+31+30),
	};
	const unsigned int __spm2[12] = {
		0,
		(31),
		(31+29),
		(31+29+31),
		(31+29+31+30),
		(31+29+31+30+31),
		(31+29+31+30+31+30),
		(31+29+31+30+31+30+31),
		(31+29+31+30+31+30+31+31),
		(31+29+31+30+31+30+31+31+30),
		(31+29+31+30+31+30+31+31+30+31),
		(31+29+31+30+31+30+31+31+30+31+30),
	};
	register time_t work;

	timep = (*timepr) - (sys_tz.tz_minuteswest * 60);
 	work = timep % (SPD);
	r->tm_sec = work % 60;
	work /= 60;
	r->tm_min = work % 60;
	r->tm_hour = work / 60;
	work = timep / (SPD);
	r->tm_wday = (4 + work) % 7;
	for (i=1970; ; ++i) {
		register time_t k = (!(i%4) && ((i%100) || !(i%400))) ? 366 : 365;
		if (work > k) {
			work -= k;
			j = (int)k;
		} else {
			j = (int)k;
			break;
		}
	}
	r->tm_year = i - 1900;
	if(j == 366) {
		for (i=11; i && __spm2[i] > work; --i) ;
			r->tm_mon = i;
			r->tm_mday = work - __spm2[i] + 1;
	} else {
		for (i=11; i && __spm[i] > work; --i) ;
			r->tm_mon = i;
			r->tm_mday = work - __spm[i] + 1;
	}
	if((r->tm_mon+1) == 12 && r->tm_mday == 32) {
		r->tm_year += 1;
		r->tm_mon = 0;
		r->tm_mday = 1;
	}
}

void Sync_RTC(void)
{
//	struct timeval tv;
	struct tm realTime;

	if(myPs->misc.slot_tic_timer > 999){
		myPs->misc.realTime_sec += 1;
		myPs->misc.slot_tic_timer = 0;
	}

	make_localtime(myPs->misc.realTime_sec, &realTime);
//	do_gettimeofday(&tv);
//	localtime(&tv.tv_sec, &realTime);
	
	//printk("(currentTime:%.04d-%.02d-%.02d %.02d:%.02d:%.02d)\n",
	//	realTime.tm_year+1900, realTime.tm_mon+1, realTime.tm_mday,
	//	realTime.tm_hour, realTime.tm_min, realTime.tm_sec);
	//	
//	myPs->real_time[0] = 0; //kjg (long)(tv.tv_usec + 500) / 1000; //msec
	myPs->real_time[0] = (long)myPs->misc.slot_tic_timer; //msec
	myPs->real_time[1] = (long)realTime.tm_sec; //sec
	myPs->real_time[2] = (long)realTime.tm_min; //min
	myPs->real_time[3] = (long)realTime.tm_hour; //hour

	myPs->real_time[4] = (long)realTime.tm_mday; //day
	myPs->real_time[5] = (long)realTime.tm_mon + 1; //month
	myPs->real_time[6] = (long)realTime.tm_year + 1900; //year
}

#endif
void getTime_RTC(void)
{
	struct timeval tv;

	do_gettimeofday(&tv);
	myPs->misc.slot_tic_timer = (long)(tv.tv_usec + 500) / 1000;
	myPs->misc.realTime_sec = (long)tv.tv_sec;
}
//add_end

void EnableWDT(void)
{
	if(myPs->config.watchdogFlag == P0) return;
	if(myData->AppControl.config.systemType == CYCLER_CAN){
		outb(0x03, 0x601);
	} else {
		outb(0x01, 0x605);
	}
}

void DisableWDT(void)
{
	if(myPs->config.watchdogFlag == P0) return;
	if(myData->AppControl.config.systemType == CYCLER_CAN){
		outb(0x01, 0x601);
	} else {
		outb(0x00, 0x605);
	}
}


void RefreshWDT(void)
{
	if(myPs->config.watchdogFlag == P0) return;
	if(myData->AppControl.config.systemType == CYCLER_CAN){
		outb(0x00, 0x602);
	} else {
		outb(0x00, 0x606);
	}
}


void Init_RT_CAN(void)
{
/*	int i, rtn;

	if(myData->CAN.config.canUseFlag == P0) return;

	for(i=0; i < myData->CAN.config.installedCAN; i++) {
		rtn = rt_can_setup(i, 2); //kjg_180405
		rtl_printf("rt_can_setup open ch:%d, rtn:%d\n", i, rtn);
	}*/
}

void Close_RT_CAN(void)
{
/*	int i, rtn;

	if(myData->CAN.config.canUseFlag == P0) return;

	for(i=0; i < myData->CAN.config.installedCAN; i++) {
		rtn = rt_can_setup(i, -2); //kjg_180405
		rtl_printf("rt_can_setup close ch:%d, rtn:%d\n", i, rtn);
	}*/
}


/*
void EnableWDT(void)
{
	unsigned char wdt;

	if(myPs->config.watchdogFlag == P0) return;

//	outb(WDT_BIT_READY, WDT_ADDR_WRITE);

	switch(myData->AppControl.config.sbcType) {
		case HS_6637:
			wdt = inb(WDT_ADDR_ENABLE_HS_6637);
			break;
		case WEB_6580:
			outb(0x05, WDT_ADDR_ENABLE_WEB_6580); //5sec
			break;
		case HS_4020:
			break;
		case WAFER_E669:
			outb(0x05, WDT_ADDR_ENABLE_WAFER_E669); //5sec
			wdt = inb(WDT_ADDR_ENABLE_WAFER_E669);
			break;
	}
}

void DisableWDT(void)
{
	unsigned char wdt;

	if(myPs->config.watchdogFlag == P0) return;

	switch(myData->AppControl.config.sbcType) {
		case HS_6637:
			wdt = inb(WDT_ADDR_DISABLE_HS_6637);
			break;
		case WEB_6580:
			outb(0x00, WDT_ADDR_DISABLE_WEB_6580);
			break;
		case HS_4020:
			break;
		case WAFER_E669:
			wdt = inb(WDT_ADDR_DISABLE_WAFER_E669);
			break;
	}
//	outb(WDT_BIT_UNREADY, WDT_ADDR_WRITE);
}

void RefreshWDT(void)
{
	unsigned char wdt;

	if(myPs->config.watchdogFlag == P0) return;

	switch(myData->AppControl.config.sbcType) {
		case HS_6637:
			wdt = inb(WDT_ADDR_REFRESH_HS_6637);
			break;
		case WEB_6580:
			outb(0x05, WDT_ADDR_REFRESH_WEB_6580); //5sec
			break;
		case HS_4020:
			break;
		case WAFER_E669:
			wdt = inb(WDT_ADDR_REFRESH_WAFER_E669);
			break;
	}
}

void CheckWDT(void)
{
	unsigned char wdt;
	int	addr, base_addr, hw_rd, hw_wr;

	if(myPs->config.watchdogFlag == P0) return;
	base_addr = myPs->addr.main[BASE_ADDR];
	hw_rd = myPs->addr.main[HW_RD];
	hw_wr = myPs->addr.main[HW_WR];

	addr = base_addr + hw_rd;
	wdt = inb(addr) & WDT_BIT_ERROR;
	if(wdt) {
		if(myPs->signal[M_SIG_SEND_ERROR_CODE] == P0) {
			myPs->signal[M_SIG_SEND_ERROR_CODE] = P12;
			myPs->code = M_FAIL_CPU_WATCHDOG;
			send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS,
				(int)myPs->code, 0);
		}
		addr = base_addr + hw_wr;
		outb(WDT_BIT_CLEAR, addr);
		outb(0x00, addr);
	}
}
*/
