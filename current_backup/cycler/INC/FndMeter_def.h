#ifndef __FNDMETER_DEF_H__
#define __FNDMETER_DEF_H__

//packet
#define	MAX_FND_METER_PACKET_LENGTH	(256*20)
#define MAX_FND_METER_PACKET_COUNT	128

//cmd
#define	FND_METER_SEND_CMD_REQUEST	0x01
#define FND_METER_SEND_CMD_TEMP		0x02
#define FND_METER_SEND_CMD_CONTROL	0x03

//AnalogMeter Signal
#define FND_METER_SIG_INITIALIZE		0
#define FND_METER_SIG_MEASURE		1
#define FND_METER_SIG_MEASURE_ERROR	2
#define FND_METER_SIG_COMM_BUS_ENABLE 3

//readType
#define READ_T_K						0
#define READ_V							1
#define READ_I							2
#define READ_T_T						3
#define MAX_METER_COUNT					10
#define MAX_FND_METER_CH				8

#endif
