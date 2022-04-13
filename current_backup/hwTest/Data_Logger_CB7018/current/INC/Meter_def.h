#ifndef __METER_DEF_H__
#define __METER_DEF_H__

//packet
#define	MAX_METER_PACKET_LENGTH		(256*10)
#define MAX_METER_PACKET_COUNT		32

//cmd
#define	METER_SEND_CMD_REQUEST		0x01
#define METER_SEND_CMD_TEMP			0x02
#define METER_SEND_CMD_CONTROL		0x03

//Meter Signal
#define METER_SIG_INITIALIZE		0
#define METER_SIG_REQUEST_PHASE		1
#define METER_SIG_LOG_START			2
#define METER_SIG_LOG_STOP			3

#endif
