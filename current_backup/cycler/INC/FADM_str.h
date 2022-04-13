#ifndef __FADM_STR_H__
#define __FADM_STR_H__

#include "FADM_def.h"

typedef struct s_fadm_misc_tag {
	int					processPointer;
	int					use_totalCycle;
	int					use_stepNo;
} S_FADM_MISC;

typedef struct s_fadm_config_tag {
	int					comPort;
	int					comBps;
	int					ttyS_fd;
	unsigned char		CmdSendLog;
	unsigned char		CmdRcvLog;
	unsigned char		CmdSendLog_Hex;
	unsigned char		CmdRcvLog_Hex;
	unsigned char		CommCheckLog;
	unsigned char		countMeter;
	unsigned char		readType;
	unsigned char		useFlag;
	float				fad_offset[4];
} S_FADM_CONFIG;

typedef struct s_fadm_rcv_packet_tag {
	int					usedBufSize;

	int					rcvCount;
	int					rcvStartPoint[MAX_FADM_PACKET_COUNT];
	int					rcvSize[MAX_FADM_PACKET_COUNT];
	char				rcvPacketBuf[MAX_FADM_PACKET_LENGTH];

	int					parseCount;
	int					parseStartPoint[MAX_FADM_PACKET_COUNT];
} S_FADM_RCV_PACKET;

typedef struct s_fadm_rcv_command_tag {
	int					cmdBufSize;
	char				cmd[MAX_FADM_PACKET_LENGTH];
	char				cmdBuf[MAX_FADM_PACKET_LENGTH];
	char				tmpBuf[MAX_FADM_PACKET_LENGTH];
	int					cmdFail;
	int					cmdSize;

	int					rcvCmdIndex;
	char				rcvCmd[MAX_FADM_PACKET_LENGTH];
	unsigned char		rcvCmdCompleteFlag;
	unsigned char		rcvCmdRestFlag;
	unsigned char		reserved1;
	unsigned char		reserved2;
} S_FADM_RCV_COMMAND;

typedef struct s_fadm_cmd_header_tag {
	char				stx;
	char				base_addr1;
	char				base_addr2;
	char				base_addr3;
	char				id_addr1;
	char				id_addr2;
	char				id_addr3;
} S_FADM_CMD_HEADER;

typedef struct s_fadm_rcv_cmd_answer_tag {
	char				data[128];
} S_FADM_RCV_CMD_ANSWER;

typedef struct s_fadm_send_cmd_tag {
	char				data[32];
} S_FADM_SEND_CMD;

typedef struct s_fadm_pulse_ad_org_tag {
	short int			value[2][300];
	short int			ref[2][3][20];
	long				avg_ref[2][3];
} S_FADM_PULSE_AD_ORG;

typedef struct s_fadm_pulse_ad_tag {
	long				value[2][300];
	double				ref_v_a;
	double				ref_v_b;
	double				ref_v_a_N;
	double				ref_v_b_N;
	double				ref_i_a;
	double				ref_i_b;
	double				ref_i_a_N;
	double				ref_i_b_N;
	long				ref[2][3];
	long				rangeI;
} S_FADM_PULSE_AD;

typedef struct s_fadm_comm_buffer_tag {
	int					write_idx;
	int					read_idx;
	int					use_ch[MAX_FADM_USE_CH];
	int					use_totalCycle[MAX_FADM_USE_CH];
	int					use_stepNo[MAX_FADM_USE_CH];
	int					use_flag;
} S_FADM_COMM_BUFFER;

typedef struct s_fadm_tag {
	S_FADM_MISC			misc;
	S_FADM_CONFIG		config;
	
	S_FADM_RCV_PACKET	rcvPacket;
	S_FADM_RCV_COMMAND	rcvCmd;

	unsigned char		signal[MAX_SIGNAL];

    S_FADM_PULSE_AD_ORG	pulse_ad_org[MAX_BD_PER_MODULE][MAX_CH_PER_BD];
	S_FADM_PULSE_AD		pulse_ad[MAX_BD_PER_MODULE][MAX_CH_PER_BD];
	long				Isens[MAX_PULSE_MSG];
	S_FADM_COMM_BUFFER	comm_buffer;
} S_FADM;

#endif
