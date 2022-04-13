#ifndef __CALIMETER_STR_H__
#define __CALIMETER_STR_H__

#include "CaliMeter_def.h"

typedef struct s_cali_misc_tag {
	int					processPointer;
} S_CALI_MISC;

typedef struct s_cali_meter_config_tag {
	int					comPort;
	int					comBps;
	int					ttyS_fd;
	unsigned char		CmdSendLog;
	unsigned char		CmdRcvLog;
	unsigned char		CmdSendLog_Hex;
	unsigned char		CmdRcvLog_Hex;
	unsigned char		CommCheckLog;
	unsigned char		readType;
	unsigned char		measureI;
	unsigned char		Shunt_Sel_Calibrator;
	long				I_offset;

	int					Lan_Use;			//add <--161220
	char				CaliMeterIP[16];	//add <--161220
	int					MeterSock_Port;		//add <--161220
} S_CALI_METER_CONFIG;

typedef struct s_cmd_cali_meter_set_tag {	//add <--190901 lyhw
	unsigned char		comPort;
	unsigned char		Type;
	unsigned char		commType;
	unsigned char		reserved1;

	char				IP[16];
	long				Port;
	long				reserved2[2];
} S_CMD_CALI_METER_SET;

typedef struct s_shunt_select_calibrator_config_tag {
	float				Shunt[MAX_RANGE];
	unsigned char		Shunt_Range_Select[MAX_RANGE];
	char				SerialNo[MAX_RANGE][20];
} S_SHUNT_SELECT_CALIBRATOR_CONFIG;


typedef struct s_cali_meter_rcv_packet_tag {
	int					usedBufSize;

	int					rcvCount;
	int					rcvStartPoint[MAX_CALI_METER_PACKET_COUNT];
	int					rcvSize[MAX_CALI_METER_PACKET_COUNT];
	char				rcvPacketBuf[MAX_CALI_METER_PACKET_LENGTH];

	int					parseCount;
	int					parseStartPoint[MAX_CALI_METER_PACKET_COUNT];
} S_CALI_METER_RCV_PACKET;

typedef struct s_cali_meter_rcv_command_tag {
	int					cmdBufSize;
	char				cmd[MAX_CALI_METER_PACKET_LENGTH];
	char				cmdBuf[MAX_CALI_METER_PACKET_LENGTH];
	char				tmpBuf[MAX_CALI_METER_PACKET_LENGTH];
	int					cmdFail;
	int					cmdSize;

	int					rcvCmdIndex;
	char				rcvCmd[MAX_CALI_METER_PACKET_LENGTH];
	unsigned char		rcvCmdCompleteFlag;
	unsigned char		rcvCmdRestFlag;
	unsigned char		reserved1;
	unsigned char		reserved2;
} S_CALI_METER_RCV_COMMAND;

typedef struct s_cali_meter_cmd_header_tag {
	char				sign1;
	char				digit1;
	char				dot;
	char				digit2[8];
	char				exponent;
	char				sign2;
	char				digit3[2];
	char				cr;
	char				nl;
} S_CALI_METER_CMD_HEADER;

typedef struct s_cali_meter_rcv_cmd_answer_tag {
	S_CALI_METER_CMD_HEADER	header;
} S_CALI_METER_RCV_CMD_ANSWER;

typedef struct s_cali_meter_send_cmd_initialize_tag {
	char				data[24];
} S_CALI_METER_SEND_CMD_INITIALIZE;

typedef struct s_cali_meter_send_cmd_request_tag {
	char				data[8];
} S_CALI_METER_SEND_CMD_REQUEST;

typedef struct s_cali_meter_tag {
	S_CALI_MISC			misc;
	S_CALI_METER_CONFIG	config;
	S_SHUNT_SELECT_CALIBRATOR_CONFIG caliConfig;
	
	S_CALI_METER_RCV_PACKET		rcvPacket;
	S_CALI_METER_RCV_COMMAND	rcvCmd;
	S_CMD_CALI_METER_SET		rcvMeterSet;	//190901 lyhw
	
	unsigned char		signal[MAX_SIGNAL];

	double				value;
	double				orgValue;
//	float				shunt_mOhm;
	double				shunt_mOhm;
	double				hallCT_ratio; //180525 add for hallct
	double				avgValue;	  //180611 lyhw 
	long				caliType;
	int					receivedBd;
	int					receivedCh;
	int					autoCaliUseFlag;	//pms edit for auto cali
	int					shuntValue[4][3];	//end of edit	
	unsigned char		offset_val;			//kjc_200309
} S_CALI_METER;
#endif
