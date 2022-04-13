#ifndef __DAQ_CLIENT_STR_H__
#define __DAQ_CLIENT_STR_H__

#include "SysDefine.h"
#include "DAQ_Client_def.h"

typedef struct s_daq_client_config_tag {
	short int			ChArray2[MAX_CH_PER_MODULE];
    char				ipAddr[16];

	int					sendPort;
	int					receivePort;
	int					networkPort; //common port

    int     			send_socket;
    int     			receive_socket;
    int     			network_socket; //common socket

	int					replyTimeout;
	int					retryCount;
	int					pingTimeout;
	int					netTimeout;
	
	unsigned char		CmdSendLog;
	unsigned char		CmdRcvLog;
	unsigned char		CmdSendLog_Hex;
	unsigned char		CmdRcvLog_Hex;
	
	unsigned char		CommCheckLog;
	unsigned char		workMode;
	short int			groupNo;

	unsigned char		gCtrlMode;
	unsigned char		useStkCrane;
	unsigned char		workType;
	unsigned char		equipType;

	short int			groupId;
	short int			monitoringInterval;
	short int			sensorDataInterval;
	short int			reserved1;

	unsigned char		autoProcess;
	unsigned char		trayCodeReadType;
	unsigned char		useTempLimitFlag;
	unsigned char		useGasLimitFlag;
	long				maxTempLimit;
	long				maxGasLimit;

	unsigned int		state_change;
	unsigned long		sended_heart_beat_interval;
	
	unsigned char		CaliPowerFanOff;	//SBC_FAN1
	unsigned char		reserved[3];		//SBC_FAN1
} S_DAQ_CLIENT_CONFIG;

typedef struct s_daq_client_misc_tag {
	unsigned long		timer;
	unsigned long		network_timer;
	unsigned long		cmd_serial;

	unsigned short		stepCount;
	unsigned short		gradeCount;
	unsigned short		convert_step1;
	unsigned short		convert_step2;

	unsigned short		tmpGroupState;
	unsigned short		tmpCode;
	unsigned short		tmpSensorState[8];

	char				Vted_Version[6]; // Major 2, Minor 2, Patch 2
	unsigned char		Version_Update; // 0:Ok, 1: Update
	unsigned char		reserved;

	int					VtedNo;
	int					psSignal;
	int					chInGroup;
	int					processPointer;
	unsigned long		sended_heart_beat_time;
	unsigned long		sended_monitor_data_time;
	
} S_DAQ_CLIENT_MISC;

typedef struct s_daq_client_th_table_tag {
	unsigned char		th_type;
	unsigned char		bias_type;
	short int			th_data_max_index;
	
	float				Vref; //mV
	float				R1; //ohm
	float				R2; //ohm
	float				R25; //lyh_20220107
	float				Beta; //lyh_20220107

	float				T_R[MAX_TH_DATA][2]; //temp, ohm
	float				V_TH[MAX_TH_DATA]; //mV
	
} S_DAQ_CLIENT_TH_TABLE;

typedef struct s_daq_client_rcv_packet_tag {
	int					usedBufSize;

	int					rcvCount;
	int					rcvStartPoint[MAX_DAQ_CLIENT_PACKET_COUNT];
	int					rcvSize[MAX_DAQ_CLIENT_PACKET_COUNT];
	char				rcvPacketBuf[MAX_DAQ_CLIENT_PACKET_LENGTH];

	int					parseCount;
	int					parseStartPoint[MAX_DAQ_CLIENT_PACKET_COUNT];
} S_DAQ_CLIENT_RCV_PACKET;

typedef struct s_daq_client_rcv_command_tag {
	int					cmdBufSize;
	char				cmd[MAX_DAQ_CLIENT_PACKET_LENGTH];
	char				cmdBuf[MAX_DAQ_CLIENT_PACKET_LENGTH];
	char				tmpBuf[MAX_DAQ_CLIENT_PACKET_LENGTH];
	int					cmdFail;
	int					cmdSize;

	int					rcvCmdIndex;
	char				rcvCmd[MAX_DAQ_CLIENT_PACKET_LENGTH];

	unsigned char		rcvCmdCompleteFlag;
	unsigned char		rcvCmdRestFlag;
	unsigned char		reserved1[2];
} S_DAQ_CLIENT_RCV_COMMAND;

typedef struct s_daq_client_retry_data_tag {
	int					seqno;
	int					replyCmd;
	int					count;
	int					size;
	char				buf[MAX_DAQ_CLIENT_PACKET_LENGTH];
} S_DAQ_CLIENT_RETRY_DATA;

typedef struct s_daq_client_reply_tag {
	int					timer_run;
	unsigned long		timer;
	S_DAQ_CLIENT_RETRY_DATA		retry;
} S_DAQ_CLIENT_REPLY;

typedef struct s_daq_client_cmd_header_tag {
	unsigned int		cmd_size;
	unsigned short		group_id;
	unsigned short		cmd_id;
} S_DAQ_CLIENT_CMD_HEADER;


typedef struct s_daq_client_rcv_cmd_check_tag {
	S_DAQ_CLIENT_CMD_HEADER		header;
	unsigned short		totalGroup;
	unsigned short		chCount;
	unsigned char		module_id[10];
	unsigned char		group;
	unsigned char		reserved;
} S_DAQ_CLIENT_RCV_CMD_CHECK;


typedef struct s_daq_send_cmd_run_tag { //4301
	S_DAQ_CLIENT_CMD_HEADER		header;
} S_DAQ_SEND_CMD_RUN;

typedef struct s_daq_send_cmd_stop_tag { //4303
	S_DAQ_CLIENT_CMD_HEADER		header;
} S_DAQ_SEND_CMD_STOP;

typedef struct s_daq_rcv_cmd_temp_data_tag { //4302
	S_DAQ_CLIENT_CMD_HEADER		header;
	long				index;
	long				data[256];
//	long				data[32]; //lyh_test
} S_DAQ_RCV_CMD_TEMP_DATA;

typedef struct s_daq_client_rcv_cmd_response_tag {
	S_DAQ_CLIENT_CMD_HEADER		header;
	unsigned short		cmd_id;
	unsigned short		code;
} S_DAQ_CLIENT_RCV_CMD_RESPONSE;

typedef struct s_daq_client_rcv_cmd_comm_check_tag {
	S_DAQ_CLIENT_CMD_HEADER		header;
} S_DAQ_CLIENT_RCV_CMD_COMM_CHECK;

typedef struct s_daq_client_rcv_cmd_comm_check_reply_tag {
	S_DAQ_CLIENT_CMD_HEADER		header;
	char				result[4];
	char				sended_cmd[4];
} S_DAQ_CLIENT_RCV_CMD_COMM_CHECK_REPLY;

typedef struct s_daq_client_send_cmd_module_info_tag {
	S_DAQ_CLIENT_CMD_HEADER		header;
	unsigned short		totalGroup;
	unsigned short		installedCh;
	unsigned char		group;
	unsigned char		reserved[3];
} S_DAQ_CLIENT_SEND_CMD_MODULE_INFO;


typedef struct s_daq_client_send_cmd_response_tag {
	S_DAQ_CLIENT_CMD_HEADER		header;
	unsigned short		cmd_id;
	unsigned short		code;
} S_DAQ_CLIENT_SEND_CMD_RESPONSE;

typedef struct s_daq_client_send_cmd_comm_check_reply_tag {
	S_DAQ_CLIENT_CMD_HEADER		header;
	char				object_id[4];
	char				result[4];
	char				sended_cmd[4];
} S_DAQ_CLIENT_SEND_CMD_COMM_CHECK_REPLY;

typedef struct s_daq_client_send_cmd_comm_check_tag {
	S_DAQ_CLIENT_CMD_HEADER		header;
} S_DAQ_CLIENT_SEND_CMD_COMM_CHECK;

typedef struct s_daq_client_send_cmd_heart_beat_tag {
	S_DAQ_CLIENT_CMD_HEADER		header;
	unsigned char		group;
	unsigned char		reserved[3];	
} S_DAQ_CLIENT_SEND_CMD_HEART_BEAT;

typedef struct s_daq_client_tag {
	unsigned char		state;
	unsigned char		code;
	unsigned char		vted_type;  //0: Not use, 1: Use
	unsigned char		reserved;

	long				temp_data[256]; //  7-2Line CH = 25EA
	
	float				temp_data_R[256]; //lyh_20220107
	float				temp_data_log[256]; //lyh_20220107
	
	long				temp[256]; //lyh_20220107

	unsigned char		signal[MAX_SIGNAL];

	S_DAQ_CLIENT_CONFIG			config;
	S_DAQ_CLIENT_MISC			misc;
	S_DAQ_CLIENT_TH_TABLE		th_table;

	S_DAQ_CLIENT_RCV_PACKET		rcvPacket;
	S_DAQ_CLIENT_RCV_COMMAND	rcvCmd;

	S_DAQ_CLIENT_REPLY			reply;
	unsigned long		pingTimer;
	unsigned long		netTimer;
	
	//S_DAQ_CLIENT_TEST_CONDITION	testCond;
	//S_DAQ_CLIENT_RECIPE_CONDITION	recipeCond[4];
} S_DAQ_CLIENT;
#endif
