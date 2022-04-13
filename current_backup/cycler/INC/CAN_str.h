#ifndef __CAN_STR_H__
#define __CAN_STR_H__

#include "CAN_def.h"

//---------------Inverter Structure----------------//

typedef struct s_can_inv_err_format{
	unsigned char lagShortCur 	 :1;//0'bit
	unsigned char overCurrent  	 :1;//1'bit
	unsigned char overVoltage    :1;//2'bit
	unsigned char preChargeFail  :1;//3'bit
	unsigned char overCurrent2 	 :1;//4'bit
	unsigned char CanComError    :1;//5'bit
	unsigned char overLoad	     :1;//6'bit
	unsigned char overHeat	     :1;//7'bit
	unsigned char reserved1	     :1;//8'bit
	unsigned char lowVoltage     :1;//9'bit
	unsigned char reserved2	     :1;//10'bit
	unsigned char reset1	     :1;//11'bit
	unsigned char reset2	     :1;//12'bit
	unsigned char acInputFail    :1;//13'bit
	unsigned char reserved3	     :1;//14'bit
	unsigned char hdcFail	     :1;//15'bit
} S_CAN_INV_ERR_FORMAT;

typedef union u_can_inv_err_header{
	unsigned short all;
	S_CAN_INV_ERR_FORMAT fault;
} U_CAN_INV_ERR_HEADER;	

typedef struct s_can_inv_cmd_format{
	unsigned char id;
	unsigned char inv0 :2;
	unsigned char inv1 :2;
	unsigned char inv2 :2;
	unsigned char inv3 :2;
	unsigned char reserved[6];
} S_CAN_INV_CMD_FORMAT;

typedef union u_can_inv_send_msg{
	unsigned char byte[8];
	S_CAN_INV_CMD_FORMAT data;
} U_CAN_INV_SEND_MSG;	

typedef struct s_can_inv_state_format{
	unsigned char cpuReady  :1;
	unsigned char stop	    :1;
	unsigned char preCharge :1;
	unsigned char acc       :1;
	unsigned char reach     :1;
	unsigned char steady    :1;
	unsigned char fail      :1;
	unsigned char dummy     :1;
}S_CAN_INV_STATE_FORMAT;

typedef struct s_can_inv_receive_format{
	unsigned char id       :3;//0'byte
	unsigned char none0	   :5;//0'byte
	unsigned char cmd      :2;//1'byte 
	unsigned char none1	   :6;//1'byte 
	S_CAN_INV_STATE_FORMAT state;//2'byte
	unsigned char dc_volt[2];//3,4'byte
	unsigned char fault[2];//5,6'byte
	unsigned char reserved;//7'byte
} S_CAN_INV_RECEIVE_FORMAT;

typedef struct s_can_inv_fault_save_format{
	unsigned short code;
	unsigned char step;
	unsigned char reserved;
	long		  vdc;
} S_CAN_INV_FAULT_SAVE_FORMAT;

typedef struct s_can_communication_format{
	char		   StateFlag;
	char		   code;
	char		   reserved[2];
	long		   pingOutCnt;
} S_CAN_COMMUNICATION_FORMAT;

typedef union u_can_inv_receive_msg{
	unsigned char byte[8];
	S_CAN_INV_RECEIVE_FORMAT data;
} U_CAN_INV_RECEIVE_MSG;	

typedef struct s_can_inv_state{
	S_CAN_COMMUNICATION_FORMAT comm;
	U_CAN_INV_ERR_HEADER faultFull;//2Byte
	S_CAN_INV_STATE_FORMAT state;//1Byte
	unsigned char  	sendCmd;//1Byte
	unsigned char  	receiveCmd;
	unsigned char  	code;
	char		   	reserved;//for byte alignment
	unsigned char   signal[MAX_CAN_SIGNAL];
	long		   vdc;
} S_CAN_INV_STATE;


//---------------IO BD Structure-----------------//
typedef struct s_can_io_state{
	S_CAN_COMMUNICATION_FORMAT comm;
	unsigned char  inputValue[CAN_IO_INPUT_CNT]; //5Byte
	unsigned char  outputValue[CAN_IO_OUTPUT_CNT]; //2Byte
	unsigned char  code;
	unsigned char	signal[16];
} S_CAN_IO_STATE;

//---------------Main BD Structure-----------------//
typedef struct s_can_main_state{
	S_CAN_COMMUNICATION_FORMAT comm;
} S_CAN_MAIN_STATE;

typedef struct s_can_config_tag {
	unsigned short		installedCAN;
	unsigned char		canUseFlag;
	unsigned char		installedInverter;

	unsigned char		functionType[MAX_CAN_PORT];
	unsigned char		canPort[MAX_CAN_PORT];
	unsigned char		commType[MAX_CAN_PORT];
	unsigned char		CmdSendLog[MAX_CAN_PORT];
	unsigned int		canBps[MAX_CAN_PORT];
	unsigned long		canCommTimeOut[MAX_CAN_PORT];
	unsigned char		CmdRcvLog[MAX_CAN_PORT];
	unsigned char		canInsDevNum[MAX_CAN_PORT];
	unsigned char		chInInv[MAX_INVERTER_NUM];
	unsigned char		canInBd[MAX_BD_PER_MODULE];
} S_CAN_CONFIG;

typedef struct rt_can_msg_tag {
	unsigned int		id;
	unsigned char		type;
	unsigned char		length;
	unsigned char		reserved1[2];
	unsigned char		data[8];
	long long			time; //kjg_120619
} S_RT_CAN_MSG;

typedef struct s_can_full_cmd_format{
	unsigned char channel;
	unsigned char relay :2;
	unsigned char range :2;
	unsigned char command :4;
	unsigned char v_ref[2];
	unsigned char i_ref[4];
} S_CAN_FULL_CMD_FORMAT;

typedef union u_can_full_send_msg{
	unsigned char byte[8];
	S_CAN_FULL_CMD_FORMAT data;
} U_CAN_FULL_SEND_MSG;	

typedef struct s_can_each_cmd_format{
	unsigned char channel;
	unsigned char command;
	unsigned char run_range;
	unsigned char reserved;
	unsigned char ref[4];
} S_CAN_EACH_CMD_FORMAT;

typedef struct s_can_main_ch_fault_msg_tag {
	unsigned short		write_idx;
	unsigned short		read_idx;
	unsigned long		runTime[MAX_CH_PER_MODULE];
	unsigned char		bd[MAX_CH_PER_MODULE];
	unsigned char		ch[MAX_CH_PER_MODULE];
	unsigned char		code[MAX_CH_PER_MODULE];
	unsigned char		stepType[MAX_CH_PER_MODULE];
} S_CAN_MAIN_CH_FAULT_MSG;

typedef union u_can_each_send_msg{
	unsigned char byte[8];
	S_CAN_EACH_CMD_FORMAT data;
} U_CAN_EACH_SEND_MSG;	

typedef struct s_can_receive_format{
	unsigned char errcode :4;
	unsigned char channel :4;
	unsigned char run	  :2;
	unsigned char range	  :2;
	unsigned char command :4;
	unsigned char read_v[3];
	unsigned char read_i[3];
} S_CAN_RECEIVE_FORMAT;

typedef union u_can_receive_msg{
	unsigned char byte[8];
	S_CAN_RECEIVE_FORMAT data;
} U_CAN_RECEIVE_MSG;	

typedef union u_can_ad_change{
	unsigned char byte[3];
	unsigned int val;
} U_CAN_AD_CHANGE;

typedef struct s_can_tag {
	S_CAN_CONFIG				config;
	S_CAN_MAIN_STATE			main[MAX_BD_PER_MODULE];
	S_CAN_MAIN_CH_FAULT_MSG		ChFaultMsg;
	S_CAN_INV_STATE     		inverter[MAX_INVERTER_NUM];
	S_CAN_INV_FAULT_SAVE_FORMAT invsavefault[MAX_INVERTER_NUM];
	S_CAN_IO_STATE		 		io;
	unsigned long				inv_run_delay;
} S_CAN;

#endif
