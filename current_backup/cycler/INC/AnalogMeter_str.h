#ifndef __ANALOGMETER_STR_H__
#define __ANALOGMETER_STR_H__

#include "AnalogMeter_def.h"

typedef struct s_analog_misc_tag {
	int					processPointer;
	long 				caliPoint[MAX_METER_COUNT][2];
	long 				caliFlag[MAX_METER_COUNT];
	long				org_temp[MAX_METER_COUNT][MAX_ANALOG_METER_CH];
	long				cali_point1[MAX_METER_COUNT][MAX_ANALOG_METER_CH];
	long				cali_point2[MAX_METER_COUNT][MAX_ANALOG_METER_CH];
	long				cali_point3[MAX_METER_COUNT][MAX_ANALOG_METER_CH];
	long				cali_point4[MAX_METER_COUNT][MAX_ANALOG_METER_CH];
	long				measure_offset[MAX_METER_COUNT][MAX_ANALOG_METER_CH];
	float				measure_gain[MAX_METER_COUNT][MAX_ANALOG_METER_CH];
	long 				ambientCaliPoint[MAX_METER_COUNT][2];
	long				ambientCaliPoint1[MAX_METER_COUNT][MAX_ANALOG_METER_CH];
	long				ambientCaliPoint2[MAX_METER_COUNT][MAX_ANALOG_METER_CH];
	long				ambient_offset[MAX_METER_COUNT][MAX_ANALOG_METER_CH];
	float				ambient_gain[MAX_METER_COUNT][MAX_ANALOG_METER_CH];
	long 				ambient_caliFlag[MAX_METER_COUNT];
	long 				gasCaliPoint[MAX_METER_COUNT][2];
	long				gasCaliPoint1[MAX_METER_COUNT][MAX_ANALOG_METER_CH];
	long				gasCaliPoint2[MAX_METER_COUNT][MAX_ANALOG_METER_CH];
	long				gas_offset[MAX_METER_COUNT][MAX_ANALOG_METER_CH];
	float				gas_gain[MAX_METER_COUNT][MAX_ANALOG_METER_CH];
	long 				gas_caliFlag[MAX_METER_COUNT];
	long				phase;
	long				auto_cali_flag;
	long				check_count;
	long				normal_count;
} S_ANALOG_MISC;

//hun_210729
typedef struct s_analog_misc2_tag {
	long				tempData[100];	
	long				checkCount;
	int					endflag;
} S_ANALOG_MISC2;

typedef struct s_analog_cali_tag {
	long 				caliFlag2[MAX_METER_COUNT][8];
	long				measure_offset2[MAX_METER_COUNT][8][MAX_ANALOG_METER_CH];
	float				measure_gain2[MAX_METER_COUNT][8][MAX_ANALOG_METER_CH];
} S_ANALOG_CALI;	

typedef struct s_analog_meter_config_tag {
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
	unsigned char		multiNum;
	
	unsigned char		functionType;
	unsigned char		commType2;
	unsigned char		autoStart;
	unsigned char		connectionCheck;
	
	unsigned char		checkSumFlag;
	unsigned char		chPerModule;
	unsigned char		tempNonDisplay;
	unsigned char		auxStartCh;

	unsigned char		auxControlFlag;
	unsigned char		ambientModuleNo;
	unsigned char		ambientModuleCount;;
	unsigned char		gasModuleNo;

	unsigned char		chPerModule2;
	unsigned char		gasModuleCount;
	unsigned char		reserved[2];
	
	int					comBps2;
	
	long				measure_offset[MAX_METER_COUNT][MAX_ANALOG_METER_CH];
	float				measure_gain[MAX_METER_COUNT][MAX_ANALOG_METER_CH];
	long				ambient_offset[MAX_METER_COUNT][MAX_ANALOG_METER_CH];
	float				ambient_gain[MAX_METER_COUNT][MAX_ANALOG_METER_CH];
	long				gas_offset[MAX_METER_COUNT][MAX_ANALOG_METER_CH];
	float				gas_gain[MAX_METER_COUNT][MAX_ANALOG_METER_CH];
} S_ANALOG_METER_CONFIG;

typedef struct s_analog_cali_config_tag {
	long				measure_offset2[MAX_METER_COUNT][8][MAX_ANALOG_METER_CH];
	float				measure_gain2[MAX_METER_COUNT][8][MAX_ANALOG_METER_CH];

	unsigned char		useRange;

} S_ANALOG_CALI_CONFIG;

typedef struct s_analog_meter_rcv_packet_tag {
	int					usedBufSize;

	int					rcvCount;
	int					rcvStartPoint[MAX_ANALOG_METER_PACKET_COUNT];
	int					rcvSize[MAX_ANALOG_METER_PACKET_COUNT];
	char				rcvPacketBuf[MAX_ANALOG_METER_PACKET_LENGTH];

	int					parseCount;
	int					parseStartPoint[MAX_ANALOG_METER_PACKET_COUNT];
} S_ANALOG_METER_RCV_PACKET;

typedef struct s_analog_temp_tag {
	long				temp;
	long				temp1;
} S_ANALOG_TEMP;

typedef struct s_analog_meter_rcv_command_tag {
	int					cmdBufSize;
	char				cmd[MAX_ANALOG_METER_PACKET_LENGTH];
	char				cmdBuf[MAX_ANALOG_METER_PACKET_LENGTH];
	char				tmpBuf[MAX_ANALOG_METER_PACKET_LENGTH];
	int					cmdFail;
	int					cmdSize;

	int					rcvCmdIndex;
	char				rcvCmd[MAX_ANALOG_METER_PACKET_LENGTH];
	unsigned char		rcvCmdCompleteFlag;
	unsigned char		rcvCmdRestFlag;
	unsigned char		reserved1;
	unsigned char		reserved2;
} S_ANALOG_METER_RCV_COMMAND;

typedef struct s_analog_meter_cmd_header_tag {
	char				stx;
	char				addr1;
	char				addr2;
} S_ANALOG_METER_CMD_HEADER;

typedef struct s_analog_meter_rcv_cmd_answer_tag {
	char						data[100];
} S_ANALOG_METER_RCV_CMD_ANSWER;

typedef struct s_analog_meter_rcv_cmd_answer2_tag {
    char                Status;
    char                SP1;
    char                AnalogChannel;
    char                CH[2];
    char                Alarm[4];
    char                Units[6];
    char                Sign;
    char                Data[5];
    char                E;
    char                Plus_Minus;
    char                Exponent[2];
    char                A_CR;
    char                LF;
} S_ANALOG_METER_RCV_CMD_ANSWER2;

typedef struct s_analog_meter_send_cmd_initialize_tag {
	char				data[20];
} S_ANALOG_METER_SEND_CMD_INITIALIZE;

typedef struct s_analog_meter_send_cmd_request_tag {
	char				data[15];
} S_ANALOG_METER_SEND_CMD_REQUEST;

typedef struct s_analog_meter_cali_data_tag {
 	unsigned char		signal[MAX_SIGNAL];	
 	unsigned char		pointNo;	
 	unsigned char		reserved[3];
} S_ANALOG_CALI_DATA;

typedef struct s_analog_meter_tag {
	S_ANALOG_MISC				misc;
	S_ANALOG_MISC2				misc2;
	S_ANALOG_TEMP				temp[MAX_AUX_TEMP_DATA];
	S_ANALOG_METER_CONFIG		config;
	
	S_ANALOG_METER_RCV_PACKET	rcvPacket;
	S_ANALOG_METER_RCV_COMMAND	rcvCmd;

	S_ANALOG_CALI				cali;
	S_ANALOG_CALI_CONFIG		cali_config;
	S_ANALOG_CALI_DATA			temp_cali;

	unsigned char				signal[MAX_SIGNAL];

	long						value;
	int							receivedBd;
	int							receivedCh;
	long						chData_AuxT[MAX_AUX_TEMP_DATA];	//190807
} S_ANALOG_METER;

#endif
