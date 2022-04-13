#ifndef __METER_STR_H__
#define __METER_STR_H__

#include "SysDefine.h"
#include "Meter_def.h"

typedef struct s_meter_serialConfig_tag {
	int					comPort;
	int					comBps;
	int					ttyS_fd;
	unsigned char		CmdSendLog;
	unsigned char		CmdRcvLog;
	unsigned char		CmdSendLog_Hex;
	unsigned char		CmdRcvLog_Hex;
	unsigned char		CommCheckLog;
	unsigned char		reserved1[3];
	int					saveInterval;
} S_METER_SERIAL_CONFIG;

typedef struct s_meter_rcv_packet_tag {
	int					usedBufSize;

	int					rcvCount;
	int					rcvStartPoint[MAX_METER_PACKET_COUNT];
	int					rcvSize[MAX_METER_PACKET_COUNT];
	char				rcvPacketBuf[MAX_METER_PACKET_LENGTH];

	int					parseCount;
	int					parseStartPoint[MAX_METER_PACKET_COUNT];
} S_METER_RCV_PACKET;

typedef struct s_meter_rcv_command_tag {
	int					cmdBufSize;
	char				cmd[MAX_METER_PACKET_LENGTH];
	char				cmdBuf[MAX_METER_PACKET_LENGTH];
	char				tmpBuf[MAX_METER_PACKET_LENGTH];
	int					cmdFail;
	int					cmdSize;

	int					rcvCmdIndex;
	char				rcvCmd[MAX_METER_PACKET_LENGTH];
	unsigned char		rcvCmdCompleteFlag;
	unsigned char		rcvCmdRestFlag;
	unsigned char		reserved1;
	unsigned char		reserved2;
} S_METER_RCV_COMMAND;

typedef struct s_meter1_cmd_header_tag {
	char				sign1;
	char				digit1;
	char				dot;
	char				digit2[8];
	char				exponent;
	char				sign2;
	char				digit3[2];
	char				cr;
	char				nl;
} S_METER1_CMD_HEADER;

typedef struct s_meter1_rcv_cmd_answer_tag {
	S_METER1_CMD_HEADER	header;
} S_METER1_RCV_CMD_ANSWER;

typedef struct s_meter1_send_cmd_initialize_tag {
	char				data[24];
} S_METER1_SEND_CMD_INITIALIZE;

typedef struct s_meter1_send_cmd_request_tag {
	char				data[8];
} S_METER1_SEND_CMD_REQUEST;
/*
//for Agilent 34401A
typedef struct s_meter2_cmd_header_tag {
	char				sign1;
	char				digit1;
	char				dot;
	char				digit2[8];
	char				exponent;
	char				sign2;
	char				digit3[2];
	char				cr;
	char				nl;
} S_METER2_CMD_HEADER;

typedef struct s_meter2_rcv_cmd_answer_tag {
	S_METER2_CMD_HEADER	header;
} S_METER2_RCV_CMD_ANSWER;

typedef struct s_meter2_send_cmd_initialize_tag {
	char				data[24];
} S_METER2_SEND_CMD_INITIALIZE;

typedef struct s_meter2_send_cmd_request_tag {
	char				data[8];
} S_METER2_SEND_CMD_REQUEST;
*/
//for ducksung
typedef struct s_meter2_cmd_header_tag {
	unsigned char		stx;
	unsigned char		addr;
	unsigned char		cmd;
	unsigned char		body_size;
} S_METER2_CMD_HEADER;

typedef struct s_meter2_rcv_cmd_answer_tag {
	S_METER2_CMD_HEADER	header;
	unsigned char		body[8];
	unsigned char		bcc;
	unsigned char		etx;
	unsigned char		reserved1;
	unsigned char		reserved2;
} S_METER2_RCV_CMD_ANSWER;

typedef struct s_meter2_send_cmd_request_tag {
	unsigned char		data[8];
} S_METER2_SEND_CMD_REQUEST;

typedef struct s_meter_tag {
	S_METER_SERIAL_CONFIG	config;
	
	S_METER_RCV_PACKET	rcvPacket;
	S_METER_RCV_COMMAND	rcvCmd;

	unsigned char		signal[MAX_SIGNAL];

	long				start_saveTime;
	long				saveTime;
	long				value;
} S_METER;

#endif
