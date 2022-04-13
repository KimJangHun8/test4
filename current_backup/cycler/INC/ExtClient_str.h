#ifndef __EXTCLIENT_STR_H__
#define __EXTCLIENT_STR_H__

#include "ExtClient_def.h"

//typedef unsigned char		BYTE;
//typedef unsigned short	WORD;
//typedef unsigned long		DWORD;

// HOST <=> CLIENT Command Format define
typedef struct s_ext_client_cmd_header_tag {
	char				cmdid[3];
	char				moduleNo;
	char				chNo[2];
	char				reply;
	char				data_size[3];
} S_EXT_CLIENT_CMD_HEADER;

typedef struct s_exit_client_send_cmd_heartbeat_tag {
	S_EXT_CLIENT_CMD_HEADER		header;
	unsigned char				CRC[2];
} S_EXT_CLIENT_SEND_CMD_HEARTBEAT;

typedef struct s_exit_client_rcv_cmd_run_tag {
	S_EXT_CLIENT_CMD_HEADER		header;
	unsigned char				CRC[2];
} S_EXT_CLIENT_RCV_CMD_RUN;

typedef struct s_exit_client_rcv_cmd_nextstep_tag {
	S_EXT_CLIENT_CMD_HEADER		header;
	unsigned char				CRC[2];
} S_EXT_CLIENT_RCV_CMD_NEXTSTEP;

typedef struct s_exit_client_rcv_cmd_pause_tag {
	S_EXT_CLIENT_CMD_HEADER		header;
	unsigned char				CRC[2];
} S_EXT_CLIENT_RCV_CMD_PAUSE;

typedef struct s_exit_client_rcv_cmd_continue_tag {
	S_EXT_CLIENT_CMD_HEADER		header;
	unsigned char				CRC[2];
} S_EXT_CLIENT_RCV_CMD_CONTINUE;

typedef struct s_exit_client_rcv_cmd_stop_tag {
	S_EXT_CLIENT_CMD_HEADER		header;
	unsigned char				CRC[2];
} S_EXT_CLIENT_RCV_CMD_STOP;

typedef struct s_exit_client_rcv_cmd_init_tag {
	S_EXT_CLIENT_CMD_HEADER		header;
	unsigned char				CRC[2];
} S_EXT_CLIENT_RCV_CMD_INIT;

typedef struct s_exit_client_rcv_cmd_unknown_cmd_tag {
	S_EXT_CLIENT_CMD_HEADER		header;
	unsigned char				CRC[2];
} S_EXT_CLIENT_RCV_CMD_UNKNOWN_CMD;

typedef struct s_exit_client_send_cmd_unknown_tag {
	S_EXT_CLIENT_CMD_HEADER		header;
	unsigned char				CRC[2];
} S_EXT_CLIENT_SEND_CMD_UNKNOWN;

typedef struct s_exit_client_rcv_cmd_crc_error_tag {
	S_EXT_CLIENT_CMD_HEADER		header;
	unsigned char				CRC[2];
} S_EXT_CLIENT_RCV_CMD_CRC_ERROR;

typedef struct s_exit_client_rcv_cmd_data_size_error_tag {
	S_EXT_CLIENT_CMD_HEADER		header;
	unsigned char				CRC[2];
} S_EXT_CLIENT_RCV_CMD_DATA_SIZE_ERROR;

typedef struct s_exit_client_send_cmd_data_size_error_tag {
	S_EXT_CLIENT_CMD_HEADER		header;
	unsigned char				CRC[2];
} S_EXT_CLIENT_SEND_CMD_DATA_SIZE_ERROR;

typedef struct s_exit_client_send_cmd_response_tag {
	S_EXT_CLIENT_CMD_HEADER		header;
	unsigned char				CRC[2];
} S_EXT_CLIENT_SEND_CMD_RESPONSE;

typedef struct s_exit_client_rcv_cmd_response_tag {
	S_EXT_CLIENT_CMD_HEADER		header;
	unsigned char				CRC[2];
} S_EXT_CLIENT_RCV_CMD_RESPONSE;

typedef struct s_exit_client_module_info_tag {
	char	protocol_version[5];
	char	moduleNo[2];
	char	chNo[2];
	char	voltage_range;
	char	current_range;
	char	voltage_range1[7];
	char	voltage_range2[7];
	char	voltage_range3[7];
	char	voltage_range4[7];
	char	current_range1[7];
	char	current_range2[7];
	char	current_range3[7];
	char	current_range4[7];
} S_EXT_CLIENT_MODULE_INFO;

typedef struct s_exit_client_send_cmd_module_info_tag {
	S_EXT_CLIENT_CMD_HEADER		header;
	S_EXT_CLIENT_MODULE_INFO 	md_info;
	unsigned char				CRC[2];
} S_EXT_CLIENT_SEND_CMD_MODULE_INFO;

typedef struct s_ext_client_parallel_tag {
	char	chNo[2];
	char	parallel_count[2];
	char	set;
} S_EXT_CLIENT_PARALLEL;

typedef struct s_ext_client_rcv_cmd_parallel_tag {
	S_EXT_CLIENT_CMD_HEADER		header;
	S_EXT_CLIENT_PARALLEL	parallel;
	unsigned char				CRC[2];
} S_EXT_CLIENT_RCV_CMD_PARALLEL;

typedef struct s_ext_client_send_cmd_trouble_tag {
	S_EXT_CLIENT_CMD_HEADER		header;
	char	code[3];
	unsigned char				CRC[2];
} S_EXT_CLIENT_SEND_CMD_TROUBLE;

typedef struct s_exit_client_rcv_cmd_trouble_tag {
	S_EXT_CLIENT_CMD_HEADER		header;
	char	code[3];
	unsigned char				CRC[2];
} S_EXT_CLIENT_RCV_CMD_TROUBLE;

typedef struct s_ext_client_step_data_tag {
	char	stepNo[3];
	char	stepType;
	char	stepMode;
	char	Vref[7];
	char	Iref[7];
	char	Pref[7];
	char	end_time_day[3];
	char	end_time_hour[2];
	char	end_time_min[2];
	char	end_time_sec[2];
	char	endV[7];
	char	endI[7];
	char	endC[7];
	char	endW[7];
	char	endWh[7];
	char	highV[7];
	char	lowV[7];
	char	highI[7];
	char	lowI[7];
	char	highC[7];
	char	lowC[7];
	char	recode_time_day[3];
	char	recode_time_hour[2];
	char	recode_time_min[2];
	char	recode_time_sec[2];
	char	recode_time_mSec[3];
	char	deltaV[7];
	char	deltaI[7];
	char	current_cycle_no[6];
	char	total_cycle_no[6];
} S_EXT_CLIENT_STEP_DATA;

typedef struct s_ext_client_rcv_cmd_step_data_tag {
	S_EXT_CLIENT_CMD_HEADER		header;
	S_EXT_CLIENT_STEP_DATA		step;
	unsigned char				CRC[2];
} S_EXT_CLIENT_RCV_CMD_STEP_DATA;

typedef struct s_ext_client_channel_data_tag {
	char	chNo[2];
	char	state;
	char	stepType;
	char	stepMode;
	char	code[3];
	char	dataType;
	char	stepNo[3];
	char	Vsens[7];
	char	Isens[7];
	char	capacity[7];
	char	watt[7];
	char	wattHour[7];
	char	z[7];
	char	step_time_day[3];
	char	step_time_hour[2];
	char	step_time_min[2];
	char	step_time_sec[2];
	char	step_time_mSec[3];
	char	currentCycleNo[6];
	char	totalCycleNo[6];
	char	saveSequence[10];
	char	temp[7];
} S_EXT_CLIENT_CHANNEL_DATA;

typedef struct s_ext_client_send_cmd_channel_data_tag {
	S_EXT_CLIENT_CMD_HEADER		header;
	S_EXT_CLIENT_CHANNEL_DATA	cData;
	unsigned char				CRC[2];
} S_EXT_CLIENT_SEND_CMD_CHANNEL_DATA;

typedef struct s_ext_client_misc_tag {
	unsigned long		timer;
	unsigned long		network_timer;
	unsigned long		cmd_serial;
	int					psSignal;
	int					processPointer;
	char				rcvRecipeFlag[MAX_BD_PER_MODULE][MAX_CH_PER_BD];
} S_EXT_CLIENT_MISC;

typedef struct s_ext_client_config_tag {
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
	unsigned char		crc_type;
	unsigned char		reserved[2];
	
	unsigned long		send_monitor_data_interval;
	unsigned long		send_save_data_interval;
	unsigned int		protocol_version;
	unsigned int		state_change;
} S_EXT_CLIENT_CONFIG;

typedef struct s_ext_rcv_packet_tag {
	int				usedBufSize;
	int				rcvCount;
	int				rcvStartPoint[MAX_MAIN_PACKET_COUNT];
	int				rcvSize[MAX_MAIN_PACKET_COUNT];
	char			rcvPacketBuf[MAX_MAIN_PACKET_LENGTH];
	int				parseCount;
	int				parseStartPoint[MAX_MAIN_PACKET_COUNT];
} S_EXT_CLIENT_RCV_PACKET;

typedef struct s_ext_rcv_command_tag {
	int				cmdBufSize;
	char			cmd[MAX_MAIN_PACKET_LENGTH];
	char			cmdBuf[MAX_MAIN_PACKET_LENGTH];
	char			tmpBuf[MAX_MAIN_PACKET_LENGTH];
	int				cmdFail;
	int				cmdSize;
	int				rcvCmdIndex;
	char			rcvCmd[MAX_MAIN_PACKET_LENGTH];

	unsigned char	rcvCmdCompleteFlag;
	unsigned char	rcvCmdRestFlag;
	unsigned char	reserved1[2];
} S_EXT_CLIENT_RCV_COMMAND;

typedef struct s_extClient_tag {
	S_EXT_CLIENT_MISC	misc;
	S_EXT_CLIENT_CONFIG			config;
	S_EXT_CLIENT_RCV_PACKET		rcvPacket;
	S_EXT_CLIENT_RCV_COMMAND	rcvCmd;
	unsigned long		pingTimer;
	unsigned long		netTimer;
	unsigned long		chDataTimer;
	unsigned short int	pingCount;
	unsigned short int	reserved;
	unsigned char		signal[MAX_SIGNAL];
	unsigned long		sended_monitor_data_time;
	unsigned long		sended_save_data_time;
} S_EXT_CLIENT; 

#endif
