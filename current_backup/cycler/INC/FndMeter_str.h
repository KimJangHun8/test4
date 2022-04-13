#ifndef __FNDMETER_STR_H__
#define __FNDMETER_STR_H__

#include "FndMeter_def.h"

typedef struct s_fnd_meter_cmd_header_tag {
	char				stx;
	char				addr1;
	char				addr2;
} S_FND_METER_CMD_HEADER;
typedef struct s_fnd_meter_config_tag {
	int					comPort;
	int					comBps;
	int					ttyS_fd;
	unsigned char		CmdSendLog;
	unsigned char		CmdRcvLog;
	unsigned char		CmdSendLog_Hex;
	unsigned char		CmdRcvLog_Hex;
	unsigned char		CommCheckLog;
	
	unsigned char		commType2;
	unsigned char		autoStart;
	unsigned char		installCh;
} S_FND_METER_CONFIG;

typedef struct s_fnd_meter_send_cmd_request_tag {
	char				data[16];
} S_FND_METER_SEND_CMD;

typedef struct s_fnd_meter_rcv_command_tag {
	int					cmdBufSize;
	char				cmd[MAX_FND_METER_PACKET_LENGTH];
	char				cmdBuf[MAX_FND_METER_PACKET_LENGTH];
	char				tmpBuf[MAX_FND_METER_PACKET_LENGTH];
	int					cmdFail;
	int					cmdSize;

	int					rcvCmdIndex;
	char				rcvCmd[MAX_FND_METER_PACKET_LENGTH];
	unsigned char		rcvCmdCompleteFlag;
	unsigned char		rcvCmdRestFlag;
	unsigned char		reserved1;
	unsigned char		reserved2;
} S_FND_METER_RCV_COMMAND;

typedef struct s_fnd_meter_rcv_packet_tag {
	int					usedBufSize;

	int					rcvCount;
	int					rcvStartPoint[MAX_FND_METER_PACKET_COUNT];
	int					rcvSize[MAX_FND_METER_PACKET_COUNT];
	char				rcvPacketBuf[MAX_FND_METER_PACKET_LENGTH];

	int					parseCount;
	int					parseStartPoint[MAX_FND_METER_PACKET_COUNT];
} S_FND_METER_RCV_PACKET;

typedef struct s_fnd_misc_tag {
	int					processPointer;
} S_FND_MISC;

typedef struct s_fnd_meter_tag {
	S_FND_MISC		misc;
	unsigned char		signal[MAX_SIGNAL];
	int					processPointer;
	S_FND_METER_CONFIG	config;
	S_FND_METER_RCV_PACKET	rcvPacket;
	S_FND_METER_RCV_COMMAND	rcvCmd;
} S_FND_METER;


#endif
