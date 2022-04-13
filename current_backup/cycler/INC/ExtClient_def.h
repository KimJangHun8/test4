#ifndef __EXTCLIENT_DEF_H__
#define __EXTCLIENT_DEF_H__

#define	MAX_EXT_CLIENT_PACKET_LENGTH		(1024*512)
#define	PROTOCOL_VERSION_1					20160111//ExtClient

// ExtClient Signal
#define EXT_SIG_NET_CONNECTED				0
#define EXT_SIG_NET_CONNECT_RETRY			1
#define EXT_SIG_TEST_STEP_RCV				2

// General Cmd
//Module Cmd
#define	SBC_TO_PC_CMD_HEARTBEAT				101
#define	PC_TO_SBC_CMD_HEARTBEAT_RESPONSE	102
#define	SBC_TO_PC_CMD_MODULE_INFO				103
#define	PC_TO_SBC_CMD_MODULE_INFO_RESPONSE		104

//Step Data Cmd
#define	PC_TO_SBC_CMD_STEP_DATA				151
#define	SBC_TO_PC_CMD_STEP_DATA_RESPONSE	152

//Control Cmd
#define	PC_TO_SBC_CMD_RUN					201
#define	SBC_TO_PC_CMD_RUN_RESPONSE			202
#define	PC_TO_SBC_CMD_STOP					203
#define	SBC_TO_PC_CMD_STOP_RESPONSE			204
#define	PC_TO_SBC_CMD_PAUSE					205
#define	SBC_TO_PC_CMD_PAUSE_RESPONSE		206
#define	PC_TO_SBC_CMD_CONTINUE				207
#define	SBC_TO_PC_CMD_CONTINUE_RESPONSE		208
#define	PC_TO_SBC_CMD_NEXTSTEP				209
#define	SBC_TO_PC_CMD_NEXTSTEP_RESPONSE		210
#define	PC_TO_SBC_CMD_INIT					211
#define	SBC_TO_PC_CMD_INIT_RESPONSE			212
#define	PC_TO_SBC_CMD_PARALLEL				213
#define	SBC_TO_PC_CMD_PARALLEL_RESPONSE		214

//Channel Data
#define	SBC_TO_PC_CMD_CH_DATA				251
#define	PC_TO_SBC_CMD_CH_DATA_RESPONSE		252

//Error Cmd
#define	SBC_TO_PC_CMD_UNKNOWN_CMD			901
#define	PC_TO_SBC_CMD_UNKNOWN_CMD			901
#define	SBC_TO_PC_CMD_UNKNOWN_CMD_RESPONSE	902
#define	PC_TO_SBC_CMD_UNKNOWN_CMD_RESPONSE	902
#define	SBC_TO_PC_CMD_CRC_ERROR				903
#define	PC_TO_SBC_CMD_CRC_ERROR				903
#define	SBC_TO_PC_CMD_CRC_ERROR_RESPONSE	904
#define	PC_TO_SBC_CMD_CRC_ERROR_RESPONSE	904
#define	SBC_TO_PC_CMD_DATA_SIZE_ERROR		905
#define	PC_TO_SBC_CMD_DATA_SIZE_ERROR		905
#define	SBC_TO_PC_CMD_TROUBLE				906
#define	PC_TO_SBC_CMD_TROUBLE				906

// ethernet protocol
#define EXT_CLIENT_STX			0x02
#define EXT_CLIENT_ETX			0x03
//
#define EXT_REPLY_YES		1	
#define EXT_REPLY_NO		0

// ethernet define
#define	MAX_PACKET_LENGTH		(1024*10+100)

// command error
#define CMD_ID_ERROR		1
#define SIZE_ERROR			2
#define CHECK_SUM_ERROR		3
#define RECIPE_BODY_ERROR	4

// reply define
#define PC_TO_SBC			0
#define SBC_TO_PC			1

//Channel Code
#define C_CD_NONE			0
#define C_CD_END_T			101
#define C_CD_END_V			102
#define C_CD_END_I			103
#define C_CD_END_C			104
#define C_CD_END_W			105
#define C_CD_END_WH			106
#define C_CD_USER_STOP		107
#define C_CD_USER_PAUSE		108
#define C_CD_OCV			109

//Channel Fault Code
#define C_CD_FAULT_UPPER_V	501
#define C_CD_FAULT_LOWER_V	502
#define C_CD_FAULT_UPPER_I	503
#define C_CD_FAULT_LOWER_I	504
#define C_CD_FAULT_UPPER_C	505
#define C_CD_FAULT_LOWER_C	506

#define C_CD_FAULT_VOLTAGE	601
#define C_CD_FAULT_CURRENT	602
#define C_CD_FAULT_OVP		603
#define C_CD_FAULT_OCP		604

#endif
