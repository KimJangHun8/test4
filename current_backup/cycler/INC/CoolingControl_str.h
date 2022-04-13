#ifndef __COOLINGCONTROL_STR_H__
#define __COOLINGCONTROL_STR_H__

#include "CoolingControl_def.h"

typedef struct s_cooling_config_tag {
   	char		ipAddr[16];
	int			sendPort;
	int			receivePort;
	int			networkPort; //common port

   	int    		send_socket;
   	int    		receive_socket;
   	int    		network_socket; //common socket

	int			replyTimeout;
	int			retryCount;
	int			pingTimeout;
	int			netTimeout;
	
	unsigned char		CmdSendLog;
	unsigned char		CmdRcvLog;
	unsigned char		CmdSendLog_Hex;
	unsigned char		CmdRcvLog_Hex;
	
	unsigned char		CommCheckLog;
	unsigned char		monitoring_StartNo;
	unsigned char		reserved[2];
	
	unsigned long		send_monitor_data_interval;
	unsigned long		send_save_data_interval;
	unsigned int		installed_cooling;
	//unsigned int		protocol_version;
} S_COOLING_CONFIG;

typedef struct s_cooling_rcv_packet_tag {
	int					usedBufSize;

	int					rcvCount;
	int					rcvStartPoint[MAX_COOLING_PACKET_COUNT];
	int					rcvSize[MAX_COOLING_PACKET_COUNT];
	char				rcvPacketBuf[MAX_COOLING_PACKET_LENGTH];

	int					parseCount;
	int					parseStartPoint[MAX_COOLING_PACKET_COUNT];
} S_COOLING_RCV_PACKET;

typedef struct s_cooling_rcv_command_tag {
	int					cmdBufSize;
	char				cmd[MAX_COOLING_PACKET_LENGTH];
	char				cmdBuf[MAX_COOLING_PACKET_LENGTH];
	char				tmpBuf[MAX_COOLING_PACKET_LENGTH];
	int					cmdFail;
	int					cmdSize;

	int					rcvCmdIndex;
	char				rcvCmd[MAX_COOLING_PACKET_LENGTH];
	unsigned char		rcvCmdCompleteFlag;
	unsigned char		rcvCmdRestFlag;
	unsigned char		reserved1[2];
} S_COOLING_RCV_COMMAND;

typedef struct s_cooling_retry_data_tag {
	int					seqno;
	int					replyCmd;
	int					count;
	int					size;
	char				buf[MAX_COOLING_PACKET_LENGTH];
} S_COOLING_RETRY_DATA;

typedef struct s_cooling_reply_tag {
	int					timer_run;
	unsigned long		timer;
	S_COOLING_RETRY_DATA	retry;
} S_COOLING_REPLY;

//아래 CMD 확인 필요 
#pragma pack(1)
typedef struct s_cooling_cmd_header_tag {
	unsigned char		stx;
	unsigned char		addr[2];
	unsigned char		cmd[4];
} S_COOLING_CMD_HEADER;
#pragma pack()

typedef struct s_cooling_misc_tag {
	
	unsigned long		timer;
	unsigned long		network_timer;
	unsigned long		cmd_serial;	
	unsigned char		psSignal;
	unsigned char		maxGroup;
	//unsigned char		set_flag;
	
	unsigned char		CoolingNo;
	unsigned char		processPointer;
	unsigned char		cmd_id;
	unsigned char		state;
	
	unsigned char		phase[MAX_COOLING_COUNT];
	unsigned char		reserved[2];

	//int					refTemp[MAX_CHAMBER_COUNT];
	//int					checkTimer;	//hun_210625
} S_COOLING_MISC;

typedef struct s_cooling_data_tag {
//	unsigned char		chamber_id;
//	unsigned char		use_flag;
	unsigned char		retry_cnt;
//	char				send_flag;

	unsigned char		error_state;
//	unsigned char		CHAMBER_RUN;
//	unsigned char		FIX_MODE;
//	unsigned char		PROG_MODE;
	
	unsigned char		phase;
	unsigned char		Status; 	//200829
	int					read_pv;	//process value
	int					read_sv;	//set value
	double				command_sv;	//set value
	unsigned char		send_sv_flag;
	unsigned char		send_error_cnt;
	unsigned char		send_sv_ng_cnt;
	unsigned char		send_pv_ng_cnt;
	unsigned char		reserved;
//	int					TEMP_NPV;
//	int					TEMP_NSP;
//	int					NOWSTS;
//	int					OTHERSTS;
	
//	int					DIN;
	
//	int					PROC_TIME_H;
//	int					PROC_TIME_M;
//	int					PROC_TIME_S;
//	int					retry_timer;
	
//	unsigned char		signal[MAX_SIGNAL];
//	unsigned char		reserved;
} S_COOLING_DATA;

typedef struct s_chamber_flag_tag {
	unsigned char		flag;
	unsigned char		count;
	unsigned char		reserved[2];
} S_CHAMBER_FLAG;

typedef struct s_chamber_use_count_tag {
	unsigned char		count;
	unsigned char		check_count;
	char				chamUpdateCount;
	char				chamUpdateFlag;
	int					stepNo;
} S_CHAMBER_USE_COUNT;

typedef struct s_chamber_client_tag {
	S_COOLING_MISC			misc;
	S_COOLING_CONFIG		config;
	
	S_COOLING_RCV_PACKET	rcvPacket;
	S_COOLING_RCV_COMMAND	rcvCmd;

	S_COOLING_REPLY			reply;
	unsigned long			pingTimer;
	unsigned long			netTimer;	
	//주석 부분 확인 필요
	S_COOLING_DATA			data[MAX_COOLING_COUNT];
	//S_CHAMBER_FLAG			flag[MAX_CH_PER_MODULE];
	//S_CHAMBER_USE_COUNT		useCount[MAX_COOLING_COUNT];
	
	//unsigned long			sended_set_data_time[MAX_CHAMBER_COUNT];

	unsigned char			signal[MAX_SIGNAL];
} S_COOLING_CONTROL;

#endif
