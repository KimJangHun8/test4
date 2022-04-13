#ifndef __RTTASK_H__
#define __RTTASK_H__

void  	*rt_task(void *);
void  	*rt_task1(void *);		//180611 add for digital
void  	*rt_task2(void *);		//210523 add for can
int		init_module(void);
void	cleanup_module(void);
void	rt_slot_100mS(int);
void	rt_slot_100mS_2(int);
void	rt_slot_50mS(int);
void	rt_slot_100mS_AD2(int);
void	rt_slot_100mS_AD2_64Ch(int);
void	rt_slot_100mS_2(int);
void	rt_slot_100mS_CAN(int);
void	rt_slot_50mS_CAN(int);
void	rt_slot_50mS_AD2(int);
void	rt_slot_50mS_AD2_2uA(int);	//210510 lyhw
void	rt_slot_25mS_AD2(int);
void	rt_slot_20mS_AD2(int);
void	rt_slot_20mS_AD2_FAD(int);
void	rt_slot_20mS_Pack(int);
void	rt_slot_10mS_AD2(int);
void 	rt_slot_10mS(int);		//180611 add for digital
void	rt_slot_20mS(int);		//180611 add for digital
void 	rt_slot_100mS_3(int);	//180611 add for digital
#endif
