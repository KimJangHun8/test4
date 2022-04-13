#ifndef __CHANNELCONTROL_CAN_H__
#define __CHANNELCONTROL_CAN_H__

void 	ChannelControl_Ch_CAN(int, int);
void 	cStep_CAN(int, int);
void 	cIdle_CAN(int, int);
void 	cStandby_CAN(int, int);
void 	cStepCharge_CAN(int, int);
void 	cStepDischarge_CAN(int, int);
void 	cStepRest_CAN(int, int);
void 	cStepUserPattern_CAN(int, int);
void 	cStepUserMap_CAN(int, int);
void 	cStepBalance_CAN(int, int);
void 	cStepZ_CAN(int, int);
void 	cStepOcv_CAN(int, int);
void 	cPause_CAN(int, int);
void 	cStepDefault_CAN(int, int);
int		SelectHwSpec_CAN(int, int);
void 	cSemiSwitch_CAN(int, int);
void 	user_pattern_data_CAN(int, int);
void	ref_output_CAN(int, int, long, long, int, int, int, int);//20190605 KHK
void 	CAN_Inverter_Control(int, int);	//220112 lyhw
void	CAN_Inverter_Fault(int, int);	//220112 lyhw
void	CAN_FaultCond_Common(int, int);	//220112 lyhw
int		INV_StateCheck(void);			//220113 lyhw
void	cStepCCCV_Check_CAN(int, int, long, long);	//220204

#endif
