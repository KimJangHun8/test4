#ifndef __COOLINGCONTROL_DEF_H__
#define __COOLINGCONTROL_DEF_H__

//packet
#define	MAX_COOLING_PACKET_LENGTH			(1024*512)
#define MAX_COOLING_PACKET_COUNT			128
//signal define
#define COOLING_SIG_NET_CONNECTED		0
#define COOLING_SIG_PROCESS				1

#define COOLING_DISCONNECT				0
#define COOLING_STANDBY					1
#define COOLING_RUNNING					2

#endif
