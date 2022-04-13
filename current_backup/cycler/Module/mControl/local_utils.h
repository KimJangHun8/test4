#ifndef __LOCAL_UTILS_H__
#define __LOCAL_UTILS_H__

void	Initialize(void);
void	InitSystemMemory(void);

void	getTime_RTC(void);		//210316 lyhw
void	timer_1sec_increment(void);
void	shift_slot_increment(void);
void	sens_count_increment(void);
void	sens_ch_ad_count_increment(int, int);
void	sens_ch_ad_count_increment_default(int, int);
void	sens_ch_ad_count_increment_1(void);
void	module_runningTime(int, long, long);

//void	localtime(const time_t *, struct tm *);
//131228 oys w : real time add
void	Sync_RTC(void);

void	CheckWDT(void);
void	EnableWDT(void);
void	DisableWDT(void);
void	RefreshWDT(void);
void 	Init_RT_CAN(void);
void 	Close_RT_CAN(void);

#endif
