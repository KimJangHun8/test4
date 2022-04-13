#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/ioctl.h>
#include <math.h>
#include "../../INC/datastore.h"
#include "userlog.h"
#include "common_utils.h"
#include "local_utils.h"
#include "message.h"
#include "com_io.h"
#include "com_socket.h"
#include "network.h"

extern volatile S_SYSTEM_DATA *myData;
extern volatile S_MAIN_CLIENT *myPs;
extern char psName[PROCESS_NAME_SIZE];

int InitNetwork(void)
{
	EquipmentInfoPrint();

	if(myPs->config.network_socket > 0)
		close(myPs->config.network_socket);
	
    myPs->config.network_socket
		= SetClientSock(myPs->config.networkPort,
		(char *)&myPs->config.ipAddr);

    if(myPs->config.network_socket < 0) {
		close(myPs->config.network_socket);
	    userlog(DEBUG_LOG, psName, "Can not initialize network : %d\n",
			myPs->config.network_socket);
		switch(myPs->config.network_socket) {
			case -113:
				printf("%c[1;5;%dm", 27, 31); //BLINK RED
				printf("PC or MainClient_Config IP setting check please!!!%c[0m\n",27);
				break;
			case -111:
				printf("%c[1;5;%dm", 27, 31); //BLINK RED
				printf("CTSMon program run state check please!!!%c[0m\n",27);
				break;
			default: break;
		}
		return -1;
    }

	myPs->signal[MAIN_SIG_NET_CONNECTED] = P1;
	myPs->netTimer = myData->mData.misc.timer_1sec;

	userlog(DEBUG_LOG, psName, "command socket connected : %d\n",
		myPs->config.network_socket);
	printf("%c[1;%dm", 27, 32);
	printf("CTSMon Program connection success!!!%c[0m\n",27);
    return 0;
}

int NetworkPacket_Receive(void)
{
	int rcv_size, read_size, i, start, index;
	char maxPacketBuf[MAX_MAIN_PACKET_LENGTH];

	memset(maxPacketBuf, 0x00, MAX_MAIN_PACKET_LENGTH);
		
	if(ioctl(myPs->config.network_socket, FIONREAD, &rcv_size) < 0) {
		userlog(DEBUG_LOG, psName, "packet receive ioctl error\n");
		close(myPs->config.network_socket);
		return -1;
	}

	if(rcv_size > MAX_MAIN_PACKET_LENGTH) {
		userlog(DEBUG_LOG, psName, "max packet size over\n");
		read_size = readn(myPs->config.network_socket, maxPacketBuf,
					MAX_MAIN_PACKET_LENGTH);
		if(read_size != MAX_MAIN_PACKET_LENGTH)
			userlog(DEBUG_LOG, psName, "packet readn size error1\n");
		close(myPs->config.network_socket);
		return -2;
	} else if(rcv_size > (MAX_MAIN_PACKET_LENGTH
		- myPs->rcvPacket.usedBufSize)) {
		userlog(DEBUG_LOG, psName, "packet buffer overflow\n");
		read_size = readn(myPs->config.network_socket, maxPacketBuf, rcv_size);
		if(read_size != rcv_size)
			userlog(DEBUG_LOG, psName, "packet readn size error2\n");
		close(myPs->config.network_socket);
		return -3;
	} else if(rcv_size <= 0) {
		userlog(DEBUG_LOG, psName, "packet sock_rcv error %d\n", rcv_size);
		send_msg(MAIN_TO_APP, MSG_MAIN_APP_PROCESS_KILL, 0, 0);
		close(myPs->config.network_socket);
		return -4;
	} else {
		read_size = readn(myPs->config.network_socket, maxPacketBuf, rcv_size);
		if(read_size != rcv_size) {
			userlog(DEBUG_LOG, psName,
				"packet readn size error3 : %d, %d\n", read_size, rcv_size);
			close(myPs->config.network_socket);
			return -5;
		}
	}
	
	//userlog(DEBUG_LOG, psName, "recvCmd %s\n", maxPacketBuf); //kjgd

	i = myPs->rcvPacket.rcvCount;
	myPs->rcvPacket.rcvCount++;
	if(myPs->rcvPacket.rcvCount > (MAX_MAIN_PACKET_COUNT-1))
		myPs->rcvPacket.rcvCount = 0;
	
	if(i == 0) index = MAX_MAIN_PACKET_COUNT - 1;
	else index = i - 1;
	start = myPs->rcvPacket.rcvStartPoint[index]
		+ myPs->rcvPacket.rcvSize[index];
	if(start >= MAX_MAIN_PACKET_LENGTH) {
		myPs->rcvPacket.rcvStartPoint[i] = abs(start - MAX_MAIN_PACKET_LENGTH);
	} else {
		myPs->rcvPacket.rcvStartPoint[i] = start;
	}

	myPs->rcvPacket.rcvSize[i] = read_size;
	myPs->rcvPacket.usedBufSize += read_size;
	
	start = myPs->rcvPacket.rcvStartPoint[i];
	if((start + read_size) > MAX_MAIN_PACKET_LENGTH) {
		index = MAX_MAIN_PACKET_LENGTH - start;
		memcpy((char *)&myPs->rcvPacket.rcvPacketBuf[start],
			(char *)&maxPacketBuf[0], index);
		memcpy((char *)&myPs->rcvPacket.rcvPacketBuf[0],
			(char *)&maxPacketBuf[index], read_size - index);
	} else {
		memcpy((char *)&myPs->rcvPacket.rcvPacketBuf[start],
			(char *)&maxPacketBuf[0], read_size);
	}
	return 0;
}

void NetworkPacket_Parsing(void)
{
	//char debug[MAX_MAIN_PACKET_LENGTH]; //kjgd
	int i, j, k, cmdBuf_index, start_point;
	if(myPs->rcvPacket.rcvCount
		== myPs->rcvPacket.parseCount) return;
	i = myPs->rcvPacket.parseCount;
	myPs->rcvPacket.parseCount++;
	if(myPs->rcvPacket.parseCount > (MAX_MAIN_PACKET_COUNT-1))
		myPs->rcvPacket.parseCount = 0;
	
	cmdBuf_index = myPs->rcvCmd.cmdBufSize;
	myPs->rcvCmd.cmdBufSize
		+= myPs->rcvPacket.rcvSize[i];
	
	start_point = myPs->rcvPacket.parseStartPoint[i];

/*	userlog(MAIN_LOG, psName, "recvCmd1 %s:end %d %d\n", 
		myPs->rcvCmd.cmdBuf, i, start_point);
		
	memset(debug, 0x00, sizeof debug);
	memcpy((char *)&debug[0], (char *)&myPs->rcvCmd.cmdBuf[cmdBuf_index],
		myPs->rcvPacket.rcvSize[i]);
	userlog(MAIN_LOG, psName, "recvCmd2 %s:end %d %d\n",
		debug, cmdBuf_index, myPs->rcvPacket.rcvSize[i]); //kjgd */

	j = start_point + myPs->rcvPacket.rcvSize[i];
	if(j <= MAX_MAIN_PACKET_LENGTH) {
		memcpy((char *)&myPs->rcvCmd.cmdBuf[cmdBuf_index],
			(char *)&myPs->rcvPacket.rcvPacketBuf[start_point],
			myPs->rcvPacket.rcvSize[i]);
	} else {
		k = MAX_MAIN_PACKET_LENGTH - start_point;
		memcpy((char *)&myPs->rcvCmd.cmdBuf[cmdBuf_index],
			(char *)&myPs->rcvPacket.rcvPacketBuf[start_point], k);
		cmdBuf_index += k;
		start_point = 0;
		k = j - MAX_MAIN_PACKET_LENGTH;
		memcpy((char *)&myPs->rcvCmd.cmdBuf[cmdBuf_index],
			(char *)&myPs->rcvPacket.rcvPacketBuf[start_point], k);
	}
		
	start_point = myPs->rcvPacket.parseStartPoint[i]
		+ myPs->rcvPacket.rcvSize[i];
	if(start_point >= MAX_MAIN_PACKET_LENGTH) {
		j = i + 1;
		if(j >= MAX_MAIN_PACKET_COUNT) j = 0;
		myPs->rcvPacket.parseStartPoint[j]
			= abs(start_point - MAX_MAIN_PACKET_LENGTH);
	} else {
		j = i + 1;
		if(j >= MAX_MAIN_PACKET_COUNT) j = 0;
		myPs->rcvPacket.parseStartPoint[j] = start_point;
	}
		
	myPs->rcvPacket.usedBufSize -= myPs->rcvPacket.rcvSize[i];
	
/*	userlog(MAIN_LOG, psName, "recvCmd3 %d %d\n", myPs->rcvPacket.usedBufSize,
		myPs->rcvCmd.cmdBufSize); //kjgd*/
}

int NetworkCommand_Receive(void)
{
	//char debug[MAX_MAIN_PACKET_LENGTH]; //kjgd
	int cmd_size, cmdBuf_index;
	S_MAIN_CMD_HEADER header;
	if(myPs->rcvCmd.cmdBufSize < sizeof(S_MAIN_CMD_HEADER)) return -1;
	
	memcpy((char *)&header, (char *)&myPs->rcvCmd.cmdBuf[0],
		sizeof(S_MAIN_CMD_HEADER));
	cmd_size = header.body_size + sizeof(S_MAIN_CMD_HEADER);
	if(myPs->rcvCmd.cmdBufSize < cmd_size) return -2;
			
/*	userlog(MAIN_LOG, psName, "recvCmd4 %s:end %d\n",
		myPs->rcvCmd.cmdBuf, cmd_size); //kjgd*/
	memset((char *)&myPs->rcvCmd.cmd[0], 0x00, MAX_MAIN_PACKET_LENGTH);
	memcpy((char *)&myPs->rcvCmd.cmd[0],
		(char *)&myPs->rcvCmd.cmdBuf[0], cmd_size);
	myPs->rcvCmd.cmdSize = cmd_size;
	
	cmdBuf_index = cmd_size;
	myPs->rcvCmd.cmdBufSize -= cmd_size;
	cmd_size = myPs->rcvCmd.cmdBufSize;
	memset((char *)&myPs->rcvCmd.tmpBuf[0], 0x00, MAX_MAIN_PACKET_LENGTH);
	memcpy((char *)&myPs->rcvCmd.tmpBuf[0],
		(char *)&myPs->rcvCmd.cmdBuf[cmdBuf_index], cmd_size);
	memset((char *)&myPs->rcvCmd.cmdBuf[0], 0x00, MAX_MAIN_PACKET_LENGTH);
	memcpy((char *)&myPs->rcvCmd.cmdBuf[0],
		(char *)&myPs->rcvCmd.tmpBuf[0], cmd_size);
	
/*	memset(debug, 0x00, sizeof debug);
	memcpy((char *)&debug[0],
		(char *)&myPs->rcvCmd.cmdBuf[0], cmd_size);
	userlog(MAIN_LOG, psName, "recvCmd5 %s:end %d %d\n",
		debug, cmd_size, cmdBuf_index); //kjgd*/

	return 0;
}

int NetworkCommand_Parsing(void)
{
	unsigned char tmp;
	int	rtn, i;
	S_MAIN_CMD_HEADER	header;
	S_MAIN_RCV_CMD_RESPONSE	cmd;

	memset((char *)&header, 0x00, sizeof(S_MAIN_CMD_HEADER));
	memcpy((char *)&header, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_MAIN_CMD_HEADER));
	if(myPs->config.CmdRcvLog == P1) {
		if(myPs->config.CommCheckLog == P1) {
			userlog(MAIN_LOG, psName, "recvCmd %s:end\n", myPs->rcvCmd.cmd);
		} else {
			switch(header.cmd_id){
				case MAIN_CMD_TO_SBC_COMM_CHECK:
				case MAIN_CMD_TO_SBC_AUX_SET:
				case MAIN_CMD_TO_SBC_CAN_SET:
				case MAIN_CMD_TO_SBC_TESTCOND_STEP:
				case MAIN_CMD_TO_SBC_TESTCOND_USER_PATTERN:
				case MAIN_CMD_TO_SBC_SET_MEASURE_DATA:
				case MAIN_CMD_TO_SBC_SET_SWELLING_DATA:
				case MAIN_CMD_TO_SBC_SET_GAS_MEASURE_DATA:
				case MAIN_CMD_TO_SBC_TESTCOND_TRACKING_FILE:
					break;
				case MAIN_CMD_TO_SBC_RESPONSE:
					memset((char *)&cmd, 0x00, sizeof(S_MAIN_RCV_CMD_RESPONSE));
					memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
						sizeof(S_MAIN_RCV_CMD_RESPONSE));
					if(cmd.response.code != EP_CD_ACK) {
						userlog(MAIN_LOG, psName, "recvCmd %s:end\n",
							myPs->rcvCmd.cmd);
					}
					break;
				default:
					userlog(MAIN_LOG, psName, "recvCmd %s:end\n", myPs->rcvCmd.cmd);
					break;
			}
		}
	}
	
	if(myPs->config.CmdRcvLog_Hex == P1) {
		if(myPs->config.CommCheckLog == P1) {
			userlog(MAIN_LOG, psName, "recvCmd_Hex");
			for(i=0; i < myPs->rcvCmd.cmdSize; i++) {
				tmp = myPs->rcvCmd.cmd[i];
				userlog2(MAIN_LOG, psName, " %02x", tmp);
			}
			userlog2(MAIN_LOG, psName, ":end\n");
		} else {
			switch(header.cmd_id){
				case MAIN_CMD_TO_SBC_COMM_CHECK:
				case MAIN_CMD_TO_SBC_AUX_SET:
				case MAIN_CMD_TO_SBC_CAN_SET:
				case MAIN_CMD_TO_SBC_TESTCOND_STEP:
				case MAIN_CMD_TO_SBC_TESTCOND_USER_PATTERN:
				case MAIN_CMD_TO_SBC_SET_MEASURE_DATA:
				case MAIN_CMD_TO_SBC_SET_SWELLING_DATA:
				case MAIN_CMD_TO_SBC_SET_GAS_MEASURE_DATA:
				case MAIN_CMD_TO_SBC_TESTCOND_TRACKING_FILE:
					break;
				case MAIN_CMD_TO_SBC_RESPONSE:
					memset((char *)&cmd, 0x00, sizeof(S_MAIN_RCV_CMD_RESPONSE));
					memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
						sizeof(S_MAIN_RCV_CMD_RESPONSE));
					if(cmd.response.code != EP_CD_ACK) {
						userlog(MAIN_LOG, psName, "recvCmd_Hex");
						for(i=0; i < myPs->rcvCmd.cmdSize; i++) {
							tmp = myPs->rcvCmd.cmd[i];
							userlog2(MAIN_LOG, psName, " %02x", tmp);
						}
						userlog2(MAIN_LOG, psName, ":end\n");
					}
					break;
				default:
					userlog(MAIN_LOG, psName, "recvCmd_Hex");
					for(i=0; i < myPs->rcvCmd.cmdSize; i++) {
						tmp = myPs->rcvCmd.cmd[i];
						userlog2(MAIN_LOG, psName, " %02x", tmp);
					}
					userlog2(MAIN_LOG, psName, ":end\n");
					break;
			}
		}
	}
	
	rtn = CmdHeader_Check((char *)&header);
	if(rtn < 0) return -1;
	
/*	rtn = Check_ReplyCmd((char *)&header);
	if(rtn < 0) return -2;*/
//	if(header.cmd_id != 1)
//	printf("header %x\n",header.cmd_id) ;
	switch(header.cmd_id) {
		case MAIN_CMD_TO_SBC_MODULE_INFO_REQUEST:
			rtn = rcv_cmd_module_info_request();
			break;
		case MAIN_CMD_TO_SBC_MODULE_SET_DATA:
			rtn = rcv_cmd_module_set_data();
			break;
		case MAIN_CMD_TO_SBC_AUX_INFO_REQUEST:
			rtn = rcv_cmd_aux_info_request();
			break;
		case MAIN_CMD_TO_SBC_CAN_INFO_REQUEST:
			rtn = rcv_cmd_can_info_request();
			break;
		case MAIN_CMD_TO_SBC_CH_ATTRIBUTE_REQUEST:
			rtn = rcv_cmd_ch_attribute_request();
			break;
		case MAIN_CMD_TO_SBC_CHAMBER_CH_NO_REQUEST:
			rtn = rcv_cmd_chamber_ch_no_request();
			break;
		case MAIN_CMD_TO_SBC_SW_FAULT_CONFIG_REQUEST:
			rtn = rcv_cmd_sw_fault_config_request();
			break;
		case MAIN_CMD_TO_SBC_CH_COMPDATA_REQUEST:
			rtn = rcv_cmd_ch_compdata_request();
			break;
		case MAIN_CMD_TO_SBC_RUN:
			rtn = rcv_cmd_run();
			myData->mData.config.sendAuxFlag = P1;
			break;
		case MAIN_CMD_TO_SBC_STOP:
			rtn = rcv_cmd_stop();
			break;
		case MAIN_CMD_TO_SBC_PAUSE:
			rtn = rcv_cmd_pause();
			break;
		case MAIN_CMD_TO_SBC_CONTINUE:
			rtn = rcv_cmd_continue();
			break;
		case MAIN_CMD_TO_SBC_NEXT_STEP:
			rtn = rcv_cmd_next_step();
			break;
// 150512 oys add start : chData Backup, Restore
		case MAIN_CMD_TO_SBC_CH_DATA_BACKUP:
			rtn = rcv_cmd_ch_data_backup();
			break;
		case MAIN_CMD_TO_SBC_CH_DATA_RESTORE:
			rtn = rcv_cmd_ch_data_restore();
			break;
// 150512 oys add end : chData Backup, Restore
		case MAIN_CMD_TO_SBC_TESTCOND_START:
//			myData->mData.config.sendAuxFlag = P0;
			rtn = rcv_cmd_testcond_start();
			break;
		case MAIN_CMD_TO_SBC_TESTCOND_SAFETY:
			rtn = rcv_cmd_testcond_safety();
			break;
		case MAIN_CMD_TO_SBC_TESTCOND_STEP:
			rtn = rcv_cmd_testcond_step();
			break;
		case MAIN_CMD_TO_SBC_TESTCOND_END:
			rtn = rcv_cmd_testcond_end();
			break;
		case MAIN_CMD_TO_SBC_TESTCOND_USER_PATTERN: // add <-- 20071106	
			if(myData->mData.config.function[F_PATTERN_CH_SAVE] == P1){
				if(myData->mData.config.function[F_PATTERN_FTP] == P1){
					rtn = rcv_cmd_testcond_user_pattern_NoUserData(1);
				}else{
					rtn = rcv_cmd_testcond_user_pattern_NoUserData(0);
				} 	
			} else {
				if(myData->mData.config.function[F_PATTERN_FTP] == P1){
					rtn = rcv_cmd_testcond_user_pattern(1);
				}else{
					rtn = rcv_cmd_testcond_user_pattern(0);
				} 	
			}
			break;
#ifdef _TRACKING_MODE
		case MAIN_CMD_TO_SBC_TESTCOND_TRACKING_FILE:
			rtn = rcv_cmd_testcond_tracking_file(); 
			break;
#endif
#if REAL_TIME == 1
		case MAIN_CMD_TO_SBC_REAL_TIME_REPLY: // add <-- 20131228
			rtn = rcv_cmd_real_time_reply();
			break;
#endif
		case MAIN_CMD_TO_SBC_STEP_COND_REQUEST:
			rtn = rcv_cmd_step_cond_request();
			break;
		case MAIN_CMD_TO_SBC_STEP_COND_UPDATE:
			rtn = rcv_cmd_step_cond_update();
			break;
		case MAIN_CMD_TO_SBC_SAFETY_COND_REQUEST:
			rtn = rcv_cmd_safety_cond_request();
			break;
		case MAIN_CMD_TO_SBC_SAFETY_COND_UPDATE:
			rtn = rcv_cmd_safety_cond_update();
			break;
		case MAIN_CMD_TO_SBC_RESET_RESERVED_CMD:
			rtn = rcv_cmd_reset_reserved_cmd();
			break;
		case MAIN_CMD_TO_SBC_CALI_METER_CONNECT:
			rtn = rcv_cmd_cali_meter_connect();
			break;
		case MAIN_CMD_TO_SBC_CALI_START:
			rtn = rcv_cmd_cali_start();
			break;
		case MAIN_CMD_TO_SBC_CALI_UPDATE:
			rtn = rcv_cmd_cali_update();
			break;
		case MAIN_CMD_TO_SBC_CALI_NOT_UPDATE:
			rtn = rcv_cmd_cali_not_update();	//add <-- 20180707
			break;
		case MAIN_CMD_TO_SBC_RESPONSE:
			rtn = rcv_cmd_response();
			break;
		case MAIN_CMD_TO_SBC_COMM_CHECK:
			rtn = rcv_cmd_comm_check();
			break;
		case MAIN_CMD_TO_SBC_SET_MEASURE_DATA: // add <-- 20071106
			rtn = rcv_cmd_set_measure_data();
			break;
		case MAIN_CMD_TO_SBC_SET_SWELLING_DATA: 	//210316 lyhw
			rtn = rcv_cmd_set_swelling_data();
			break;
		case MAIN_CMD_TO_SBC_SET_GAS_MEASURE_DATA: 	//210923 lyhw
			rtn = rcv_cmd_set_gas_measure_data();
			break;
		case MAIN_CMD_TO_SBC_CH_ATTRIBUTE_SET: // add <-- 20090424
			rtn = rcv_cmd_ch_attribute_set();
			break;
		case MAIN_CMD_TO_SBC_CHAMBER_CH_NO_SET: // add <-- 20090424
			rtn = rcv_cmd_chamber_ch_no_set();
			break;
		case MAIN_CMD_TO_SBC_SW_FAULT_CONFIG_SET: // add <-- 20090424
			rtn = rcv_cmd_sw_fault_config_set();
			break;
		case MAIN_CMD_TO_SBC_CH_COMPDATA_SET: // add <-- 20090424
			rtn = rcv_cmd_ch_compdata_set();
			break;
		case MAIN_CMD_TO_SBC_AUX_SET: // add <-- 20090424
			rtn = rcv_cmd_aux_set();
			break;
		case MAIN_CMD_TO_SBC_CAN_SET: // add <-- 20090424
			rtn = rcv_cmd_can_set();
			break;
		case MAIN_CMD_TO_SBC_CH_INIT: // add <-- 20090424
			rtn = rcv_cmd_init();
			break;
		case MAIN_CMD_TO_SBC_TESTCOND_USER_MAP: // add <-- 20111215
			rtn = rcv_cmd_testcond_user_map();
			break;
		case MAIN_CMD_TO_SBC_BUZZER_OFF: // add <-- 20190225
			rtn = rcv_cmd_buzzer_off();
			break;
		case MAIN_CMD_TO_SBC_BUZZER_ON: // add <-- 20190225
			rtn = rcv_cmd_buzzer_on();
			break;
		case MAIN_CMD_TO_SBC_DIO_CONTROL:	//190809 lyhw
			rtn = rcv_cmd_dio_control();
			break;
		case MAIN_CMD_TO_SBC_CALIMETER_SET:
			rtn = rcv_cmd_calimeter_set();
			break;
//#ifdef __LG_VER1__ hun_210929 SDI DATA RECOVERY
		case MAIN_CMD_TO_SBC_CH_RECOVERY: // 20180717 sch 
			rtn = rcv_cmd_ch_recovery();
			break;
		case MAIN_CMD_TO_SBC_GOTO_COUNT_RECOVERY: // 20180717 sch 
			rtn = rcv_cmd_goto_count_recovery();
			break;
		case MAIN_CMD_TO_SBC_CH_END_DATA_RECOVERY:		// SKI hun_201010
			rtn = rcv_cmd_ch_end_data_recovery();
			break;
		case MAIN_CMD_TO_SBC_CH_CONVERT_ADV_CYCLE_STEP:	// SKI hun_201010
			rtn = rcv_cmd_ch_convert_adv_cycle_step();
			break;
//#endif
		case MAIN_CMD_TO_SBC_GOTO_COUNT_REQUEST:
			rtn = rcv_cmd_goto_count_request();
			break;
		case MAIN_CMD_TO_SBC_GOTO_COUNT_DATA:
			rtn = rcv_cmd_goto_count_data();
			break;
		/*case MAIN_CMD_TO_SBC_RESET:
			rtn = rcv_cmd_reset();*/
		//20171008 sch add
#ifdef _USER_VI
		case MAIN_CMD_TO_SBC_LIMIT_USER_VI_REQUEST:
			rtn = rcv_cmd_limit_user_vi_request();
			break;
		case MAIN_CMD_TO_SBC_LIMIT_USER_VI_SET: 
			rtn = rcv_cmd_limit_user_vi_set();
			break;
#endif
		case MAIN_CMD_TO_SBC_GUI_VERSION_INFO:	//20190829 oys
			rtn = rcv_cmd_gui_version_info();
			break;
			
		case MAIN_CMD_TO_SBC_SHUTDOWN: //181108
			rtn = rcv_cmd_shutdown();
			break;
		case MAIN_CMD_TO_SBC_PARAMETER_UPDATE: 	//190801 lyhw
			rtn = rcv_cmd_parameter_update();
			break;
		case MAIN_CMD_TO_SBC_HWFAULT_REQUEST:	//hun_200219
			rtn = rcv_cmd_hwfault_request();
			break;
		case MAIN_CMD_TO_SBC_FAULT_ALARM_REQUEST:
			rtn = rcv_cmd_fault_alarm_request();
			break;
		//210415 LJS
		case MAIN_CMD_TO_SBC_FAULT_CHANNEL_CODE:
			rtn = rcv_cmd_fault_channel_code();
			break;
		case MAIN_CMD_TO_SBC_FAULT_GUI_CODE:
			rtn = rcv_cmd_fault_gui_code();
			break;
#ifdef _TEMP_CALI 
		case MAIN_CMD_TO_SBC_TEMP_CALI_POINT_SET:
			rtn = rcv_cmd_temp_cali_point_set();
			break;
		case MAIN_CMD_TO_SBC_TEMP_CALI_START: 
			rtn = rcv_cmd_temp_cali_start();
			break;
		case MAIN_CMD_TO_SBC_TEMP_CALI_INFO_REQUEST:
			rtn = rcv_cmd_temp_cali_info_request();
			break;
		case MAIN_CMD_TO_SBC_TEMP_CALI_BACKUP_READ:
			rtn = rcv_cmd_temp_cali_backup_read();
			break;
#endif
		//210204
		case MAIN_CMD_TO_SBC_HWFAULT_CONFIG_SET:
			rtn = rcv_cmd_hwfault_config_set();
			break;
		case MAIN_CMD_TO_SBC_HWFAULT_CONFIG_REQUEST:
			rtn = rcv_cmd_hwfault_config_request();
			break;
#ifdef _EXTERNAL_CONTROL
		case MAIN_CMD_TO_SBC_CELL_DIAGNOSIS_PAUSE:
			rtn = rcv_cmd_dll_pause();
			break;
		case MAIN_CMD_TO_SBC_CELL_DIAGNOSIS_STOP:
			rtn = rcv_cmd_dll_stop();
			break;
		/*
		case MAIN_CMD_TO_SBC_CELL_DIAGNOSIS_ALARM:
			rtn = rcv_cmd_cell_diagnosis_alarm();
			break;
		*/
		case MAIN_CMD_TO_SBC_SCHEDULE_INFO:
			userlog(DEBUG_LOG, psName, "Not Use CMD[EXTERNAL_CONTROL]\n");
			break;
#endif
		case MAIN_CMD_TO_SBC_CHAMBER_WAIT_RELEASE: //211022
			rtn = rcv_cmd_chamber_wait_release();
			break;
		case MAIN_CMD_TO_SBC_LGES_FAULT_CONFIG_SET:	//211025 hun
			rtn = rcv_cmd_lges_fault_config_set();
			break;
		case MAIN_CMD_TO_SBC_LGES_FAULT_CONFIG_REQUEST:	//211025 hun
			rtn = rcv_cmd_lges_fault_config_request();
			break;
		case MAIN_CMD_TO_SBC_SDI_CC_CV_HUMP_CONFIG_SET:	//211125 hun
			rtn = rcv_cmd_sdi_cc_cv_hump_config_set();
			break;
		case MAIN_CMD_TO_SBC_SDI_CC_CV_HUMP_CONFIG_REQUEST:	//211125 hun
			rtn = rcv_cmd_sdi_cc_cv_hump_config_request();
			break;
		case MAIN_CMD_TO_SBC_SDI_PAUSE_SAVE_CONFIG_SET:	//211209 hun
			rtn = rcv_cmd_sdi_pause_save_config_set();
			break;
		case MAIN_CMD_TO_SBC_SDI_PAUSE_SAVE_CONFIG_REQUEST:	//211209 hun
			rtn = rcv_cmd_sdi_pause_save_config_request();
			break;
		case MAIN_CMD_TO_SBC_ACIR_VALUE:
			rtn = rcv_cmd_acir_value();
			break;
		case MAIN_CMD_TO_SBC_DYSON_MAINTENANCE_CONFIG_SET:	//220118 jsh
			rtn = rcv_cmd_dyson_maintenance_config_set();
			break;
		case MAIN_CMD_TO_SBC_DYSON_MAINTENANCE_CONFIG_REQUEST:	//220118 jsh
			rtn = rcv_cmd_dyson_maintenance_config_request();
			break;
		case MAIN_CMD_TO_SBC_CHAMBER_FAULT:	//220214_hun
			rtn = rcv_cmd_chamber_fault();
			break;
			
		default:
			userlog(DEBUG_LOG, psName, "Can't Find Cmd[%x]\n", header.cmd_id);
			rtn = -10;
			break;
	}
	return rtn;
}

int Parsing_NetworkEvent(void)
{
	int rtn=0;
	NetworkPacket_Parsing();
	while(NetworkCommand_Receive() >= 0) { //kjg_change 080125
		if(NetworkCommand_Parsing() < 0) {
			myPs->rcvCmd.cmdFail++;
			if(myPs->rcvCmd.cmdFail >= 3) {
				myPs->rcvCmd.cmdFail = 0;
				myPs->rcvCmd.cmdBufSize = 0;
				memset((char *)&myPs->rcvCmd.cmdBuf[0], 0x00,
					MAX_MAIN_PACKET_LENGTH);
				rtn = -1;
				return rtn;
			}
		} else {
			myPs->rcvCmd.cmdFail = 0;
		}
	}
	
	return rtn;
}
	
int CmdHeader_Check(char *rcvHeader)
{
	int length;

	S_MAIN_CMD_HEADER	header;
	S_MAIN_ADP_TEST_COND_USER_PATTERN userPattern;
#ifdef _TRACKING_MODE
	S_MAIN_ADP_TEST_COND_TRACKING_FILE tracking;
#endif
	
	memset((char *)&header, 0x00, sizeof(S_MAIN_CMD_HEADER));
	memcpy((char *)&header, (char *)rcvHeader, sizeof(S_MAIN_CMD_HEADER));
	
	switch(header.cmd_id) {
		case MAIN_CMD_TO_SBC_MODULE_INFO_REQUEST:
			length = sizeof(S_MAIN_RCV_CMD_MODULE_INFO_REQUEST); 
			break;
		case MAIN_CMD_TO_SBC_MODULE_SET_DATA:
			length = sizeof(S_MAIN_RCV_CMD_MODULE_SET_DATA); 
			break;
		case MAIN_CMD_TO_SBC_AUX_INFO_REQUEST:
			length = sizeof(S_MAIN_RCV_CMD_AUX_INFO_REQUEST); 
			break;
		case MAIN_CMD_TO_SBC_CAN_INFO_REQUEST:
			length = sizeof(S_MAIN_RCV_CMD_CAN_INFO_REQUEST); 
			break;
		case MAIN_CMD_TO_SBC_CH_ATTRIBUTE_REQUEST:
			length = sizeof(S_MAIN_RCV_CMD_CH_ATTRIBUTE_REQUEST); 
			break;
		case MAIN_CMD_TO_SBC_CHAMBER_CH_NO_REQUEST:
			length = sizeof(S_MAIN_RCV_CMD_CHAMBER_CH_NO_REQUEST); 
			break;
		case MAIN_CMD_TO_SBC_SW_FAULT_CONFIG_REQUEST:
			length = sizeof(S_MAIN_RCV_CMD_SW_FAULT_CONFIG_REQUEST); 
			break;
		case MAIN_CMD_TO_SBC_CH_COMPDATA_REQUEST:
			length = sizeof(S_MAIN_RCV_CMD_CH_COMPDATA_REQUEST); 
			break;
		case MAIN_CMD_TO_SBC_RUN:
			length = sizeof(S_MAIN_RCV_CMD_RUN); 
			break;
		case MAIN_CMD_TO_SBC_STOP:
			length = sizeof(S_MAIN_RCV_CMD_STOP); 
			break;
		case MAIN_CMD_TO_SBC_TESTCOND_START:
			length = sizeof(S_MAIN_RCV_CMD_TESTCOND_START); 
			break;
		case MAIN_CMD_TO_SBC_TESTCOND_SAFETY:
			length = sizeof(S_MAIN_RCV_CMD_TESTCOND_SAFETY);
			break;
		case MAIN_CMD_TO_SBC_TESTCOND_STEP:
			length = sizeof(S_MAIN_RCV_CMD_TESTCOND_STEP); 
			break;
		case MAIN_CMD_TO_SBC_TESTCOND_END:
			length = sizeof(S_MAIN_RCV_CMD_TESTCOND_END); 
			break;
		case MAIN_CMD_TO_SBC_TESTCOND_USER_PATTERN: // add <-- 20071106
			memset((char *)&userPattern, 0x00
					, sizeof(S_MAIN_ADP_TEST_COND_USER_PATTERN));
			memcpy((char *)&userPattern
					,(char *)(myPs->rcvCmd.cmd + sizeof(S_MAIN_CMD_HEADER))
					,sizeof(S_MAIN_ADP_TEST_COND_USER_PATTERN));
			if(myData->mData.config.function[F_PATTERN_FTP] == P1){
				length = sizeof(S_MAIN_RCV_CMD_TESTCOND_USER_PATTERN);
			}else{
				length = sizeof(S_MAIN_RCV_CMD_TESTCOND_USER_PATTERN)
						+sizeof(S_MAIN_ADP_TEST_COND_USER_PATTERN_DATA)
						 * userPattern.length;
			}
			break;
#ifdef _TRACKING_MODE
		case MAIN_CMD_TO_SBC_TESTCOND_TRACKING_FILE:
			memset((char *)&tracking, 0x00
					, sizeof(S_MAIN_ADP_TEST_COND_TRACKING_FILE));
			memcpy((char *)&tracking
					,(char *)(myPs->rcvCmd.cmd + sizeof(S_MAIN_CMD_HEADER))
					,sizeof(S_MAIN_ADP_TEST_COND_TRACKING_FILE));
			length = sizeof(S_MAIN_RCV_CMD_TESTCOND_TRACKING_FILE);
			break;
#endif
		case MAIN_CMD_TO_SBC_STEP_COND_REQUEST:
			length = sizeof(S_MAIN_RCV_CMD_STEP_COND_REQUEST); 
			break;
		case MAIN_CMD_TO_SBC_STEP_COND_UPDATE:
			length = sizeof(S_MAIN_RCV_CMD_STEP_COND_UPDATE); 
			break;
		case MAIN_CMD_TO_SBC_SAFETY_COND_REQUEST:
			length = sizeof(S_MAIN_RCV_CMD_SAFETY_COND_REQUEST); 
			break;
		case MAIN_CMD_TO_SBC_SAFETY_COND_UPDATE:
			length = sizeof(S_MAIN_RCV_CMD_SAFETY_COND_UPDATE); 
			break;
		case MAIN_CMD_TO_SBC_RESET_RESERVED_CMD:
			length = sizeof(S_MAIN_RCV_CMD_RESET_RESERVED_CMD); 
			break;
		case MAIN_CMD_TO_SBC_CONTINUE:
			length = sizeof(S_MAIN_RCV_CMD_CONTINUE); 
			break;
		case MAIN_CMD_TO_SBC_PAUSE:
			length = sizeof(S_MAIN_RCV_CMD_PAUSE); 
			break;
		case MAIN_CMD_TO_SBC_NEXT_STEP:
			length = sizeof(S_MAIN_RCV_CMD_NEXT_STEP); 
			break;
// 150512 oys add start : chData Backup, Restore
		case MAIN_CMD_TO_SBC_CH_DATA_BACKUP:
			length = sizeof(S_MAIN_RCV_CMD_CH_DATA_BACKUP); 
			break;
		case MAIN_CMD_TO_SBC_CH_DATA_RESTORE:
			length = sizeof(S_MAIN_RCV_CMD_CH_DATA_RESTORE); 
			break;
// 150512 oys add end : chData Backup, Restore
		case MAIN_CMD_TO_SBC_RESPONSE:
			length = sizeof(S_MAIN_RCV_CMD_RESPONSE); 
			break;
		case MAIN_CMD_TO_SBC_CALI_METER_CONNECT:
			length = sizeof(S_MAIN_RCV_CMD_CALI_METER_CONNECT); 
			break;
		case MAIN_CMD_TO_SBC_CALI_START:
			length = sizeof(S_MAIN_RCV_CMD_CALI_START); 
			break;
		case MAIN_CMD_TO_SBC_CALI_UPDATE:
			length = sizeof(S_MAIN_RCV_CMD_CALI_UPDATE); 
			break;
		case MAIN_CMD_TO_SBC_CALI_NOT_UPDATE:	//add <-- 20180707
			length = sizeof(S_MAIN_RCV_CMD_CALI_NOT_UPDATE); 
			break;
		case MAIN_CMD_TO_SBC_COMM_CHECK:
			length = sizeof(S_MAIN_RCV_CMD_COMM_CHECK); 
			break;
		case MAIN_CMD_TO_SBC_SET_MEASURE_DATA: // add <-- 20071106
			length = sizeof(S_MAIN_RCV_CMD_SET_MEASURE_DATA); 
			break;
		case MAIN_CMD_TO_SBC_SET_SWELLING_DATA: 	//210316 lyhw
			length = sizeof(S_MAIN_RCV_CMD_SET_SWELLING_DATA); 
			break;
		case MAIN_CMD_TO_SBC_SET_GAS_MEASURE_DATA: 	//210923 lyhw
			length = sizeof(S_MAIN_RCV_CMD_SET_GAS_MEASURE_DATA); 
			break;
		case MAIN_CMD_TO_SBC_CH_COMPDATA_SET:
			length = sizeof(S_MAIN_RCV_CMD_CH_COMPDATA_SET); 
			break;
		case MAIN_CMD_TO_SBC_AUX_SET:
			length = sizeof(S_MAIN_RCV_CMD_AUX_SET); 
			break;
		case MAIN_CMD_TO_SBC_CAN_SET:
			length = sizeof(S_MAIN_RCV_CMD_CAN_SET); 
			break;
		case MAIN_CMD_TO_SBC_CH_ATTRIBUTE_SET:
			length = sizeof(S_MAIN_RCV_CMD_CH_ATTRIBUTE_SET); 
			break;
		case MAIN_CMD_TO_SBC_CHAMBER_CH_NO_SET:
			length = sizeof(S_MAIN_RCV_CMD_CHAMBER_CH_NO_SET); 
			break;
		case MAIN_CMD_TO_SBC_SW_FAULT_CONFIG_SET:
			length = sizeof(S_MAIN_RCV_CMD_SW_FAULT_CONFIG_SET); 
			break;
		case MAIN_CMD_TO_SBC_CH_INIT:
			length = sizeof(S_MAIN_RCV_CMD_INIT); 
			break;
		case MAIN_CMD_TO_SBC_TESTCOND_USER_MAP: // add <-- 20071106
			length = sizeof(S_MAIN_RCV_CMD_TESTCOND_USER_MAP); 
			break;
		case MAIN_CMD_TO_SBC_BUZZER_OFF: // add <-- 20190225
			length = sizeof(S_MAIN_RCV_CMD_BUZZER_OFF); 
			break;
		case MAIN_CMD_TO_SBC_BUZZER_ON: // add <-- 20190225
			length = sizeof(S_MAIN_RCV_CMD_BUZZER_ON); 
			break;
		case MAIN_CMD_TO_SBC_DIO_CONTROL:	//add <-- 20181023
			length = sizeof(S_MAIN_RCV_CMD_DIO_CONTROL); 
			break;
		case MAIN_CMD_TO_SBC_CALIMETER_SET:	//add <-- 20181023
			length = sizeof(S_MAIN_RCV_CMD_CALIMETER_SET); 
			break;
//#ifdef __LG_VER1__ hun_210929 SDI DATA RECOVERY
		case MAIN_CMD_TO_SBC_CH_RECOVERY: // 20180717 sch
			length = sizeof(S_MAIN_RCV_CMD_CH_RECOVERY); 
			break;
		case MAIN_CMD_TO_SBC_GOTO_COUNT_RECOVERY: // 20180717 sch
			length = sizeof(S_MAIN_CMD_GOTO_COUNT_RECOVERY); 
			break;
		case MAIN_CMD_TO_SBC_CH_END_DATA_RECOVERY:	// SKI hun_201010
			length = sizeof(S_MAIN_RCV_CMD_CH_END_DATA_RECOVERY);
			break;
		case MAIN_CMD_TO_SBC_CH_CONVERT_ADV_CYCLE_STEP:	// SKI hun_201010
			length = sizeof(S_MAIN_RCV_CMD_CH_CONVERT_ADV_CYCLE_STEP);
			break;
//#endif
		case MAIN_CMD_TO_SBC_GOTO_COUNT_REQUEST:
			length = sizeof(S_MAIN_RCV_CMD_GOTO_COUNT_REQUEST);
			break;
		case MAIN_CMD_TO_SBC_GOTO_COUNT_DATA:
			length = sizeof(S_MAIN_RCV_CMD_GOTO_COUNT_DATA);
			break;
		//20171008 sch add
#ifdef _USER_VI
		case MAIN_CMD_TO_SBC_LIMIT_USER_VI_REQUEST:
			length = sizeof(S_MAIN_RCV_CMD_LIMIT_USER_VI_REQUEST);
			break;
		case MAIN_CMD_TO_SBC_LIMIT_USER_VI_SET:
			length = sizeof(S_MAIN_RCV_CMD_LIMIT_USER_VI_SET);
			break;
#endif
#if REAL_TIME == 1
		case MAIN_CMD_TO_SBC_REAL_TIME_REPLY: // add <-- 20131228
			length = sizeof(S_MAIN_RCV_CMD_REAL_TIME_REPLY); 
			break;
#endif
		case MAIN_CMD_TO_SBC_SHUTDOWN: //181108
			length = sizeof(S_MAIN_RCV_CMD_SHUTDOWN);
			break;
		case MAIN_CMD_TO_SBC_PARAMETER_UPDATE: //190801
			length = sizeof(S_MAIN_RCV_CMD_PARAMETER_UPDATE);
			break;
		case MAIN_CMD_TO_SBC_HWFAULT_REQUEST:	//hun_200219
			length = sizeof(S_MAIN_RCV_CMD_MODULE_HWFAULT_REQUEST);
			break;
		//210415 LJS
		case MAIN_CMD_TO_SBC_FAULT_ALARM_REQUEST:
			length = sizeof(S_MAIN_RCV_CMD_FAULT_ALARM_REQUEST);
			break;
		case MAIN_CMD_TO_SBC_FAULT_CHANNEL_CODE:
			length = sizeof(S_MAIN_RCV_CMD_FAULT_CHANNEL_CODE);
			break;
		case MAIN_CMD_TO_SBC_FAULT_GUI_CODE:
			length = sizeof(S_MAIN_RCV_CMD_FAULT_CHANNEL_CODE);
			break;
#ifdef _TEMP_CALI //211119_hun
		case MAIN_CMD_TO_SBC_TEMP_CALI_POINT_SET:
			length = sizeof(S_MAIN_RCV_CMD_TEMP_CALI_POINT_SET);
			break;
		case MAIN_CMD_TO_SBC_TEMP_CALI_START:
			length = sizeof(S_MAIN_RCV_CMD_TEMP_CALI_START);
			break;
		case MAIN_CMD_TO_SBC_TEMP_CALI_INFO_REQUEST:
			length = sizeof(S_MAIN_RCV_CMD_TEMP_CALI_INFO_REQUEST); 
			break;
		case MAIN_CMD_TO_SBC_TEMP_CALI_BACKUP_READ:
			length = sizeof(S_MAIN_RCV_CMD_TEMP_CALI_BACKUP_READ);
			break;
#endif			
		//210204
		case MAIN_CMD_TO_SBC_HWFAULT_CONFIG_SET:
			length = sizeof(S_MAIN_RCV_CMD_HWFAULT_CONFIG_SET);
			break;
		case MAIN_CMD_TO_SBC_HWFAULT_CONFIG_REQUEST:
			length = sizeof(S_MAIN_RCV_CMD_HWFAULT_CONFIG_REQUEST);
			break;
#ifdef _EXTERNAL_CONTROL
		case MAIN_CMD_TO_SBC_CELL_DIAGNOSIS_PAUSE:
			length = sizeof(S_MAIN_RCV_CMD_PAUSE); 
			break;
		case MAIN_CMD_TO_SBC_CELL_DIAGNOSIS_STOP:
			length = sizeof(S_MAIN_RCV_CMD_STOP); 
			break;
		case MAIN_CMD_TO_SBC_SCHEDULE_INFO:
			length = sizeof(S_MAIN_RCV_CMD_SCHEDULE_INFO); 
			break;
		/*
		case MAIN_CMD_TO_SBC_CELL_DIAGNOSIS_ALARM:
			length = sizeof(S_MAIN_RCV_CMD_CELL_DIAGNOSIS_ALARM);
			break;
		*/
#endif
		case MAIN_CMD_TO_SBC_LGES_FAULT_CONFIG_SET:
			length = sizeof(S_MAIN_RCV_CMD_LGES_FAULT_CONFIG_SET);
			break;
		case MAIN_CMD_TO_SBC_LGES_FAULT_CONFIG_REQUEST:
			length = sizeof(S_MAIN_RCV_CMD_LGES_FAULT_CONFIG_REQUEST);
			break;
		case MAIN_CMD_TO_SBC_SDI_CC_CV_HUMP_CONFIG_SET:	//211125 hun
			length = sizeof(S_MAIN_RCV_CMD_SDI_CC_CV_HUMP_CONFIG_SET);
			break;
		case MAIN_CMD_TO_SBC_SDI_CC_CV_HUMP_CONFIG_REQUEST:	//211125 hun
			length = sizeof(S_MAIN_RCV_CMD_SDI_CC_CV_HUMP_CONFIG_REQUEST);
			break;
		case MAIN_CMD_TO_SBC_SDI_PAUSE_SAVE_CONFIG_SET:	//211209 hun
			length = sizeof(S_MAIN_RCV_CMD_SDI_PAUSE_SAVE_CONFIG_SET);
			break;
		case MAIN_CMD_TO_SBC_SDI_PAUSE_SAVE_CONFIG_REQUEST:	//211209 hun
			length = sizeof(S_MAIN_RCV_CMD_SDI_PAUSE_SAVE_CONFIG_REQUEST);
			break;
		case MAIN_CMD_TO_SBC_CHAMBER_WAIT_RELEASE: //211027	
			length = sizeof(S_MAIN_RCV_CMD_CHAMBER_WAIT_RELEASE); 
			break;
		case MAIN_CMD_TO_SBC_ACIR_VALUE:
			length = sizeof(S_MAIN_RCV_CMD_ACIR_VALUE);
			break;
		case MAIN_CMD_TO_SBC_DYSON_MAINTENANCE_CONFIG_SET:	//220118 jsh
			length = sizeof(S_MAIN_RCV_CMD_DYSON_MAINTENANCE_CONFIG_SET);
			break;
		case MAIN_CMD_TO_SBC_DYSON_MAINTENANCE_CONFIG_REQUEST:	//220118 jsh
			length = sizeof(S_MAIN_RCV_CMD_DYSON_MAINTENANCE_CONFIG_REQUEST);
			break;
		case MAIN_CMD_TO_SBC_CHAMBER_FAULT:	//220214_hun
			length = sizeof(S_MAIN_RCV_CMD_CHAMBER_FAULT);
			break;
		default:
			send_cmd_unknown(header.cmd_id, MAIN_CMD_ID_ERROR);
			userlog(DEBUG_LOG, psName, "RcvCmd command id error %x\n",
				header.cmd_id);
			return -1;
	}
	
	if(length != myPs->rcvCmd.cmdSize) {
		send_cmd_unknown(header.cmd_id, MAIN_SIZE_ERROR);
		userlog(DEBUG_LOG, psName, "RcvCmd(0x%x) size error (%d : %d)\n",
			header.cmd_id, length, myPs->rcvCmd.cmdSize);
		return -2;
	}

	if(header.cmd_serial > MAX_MAIN_CMD_SERIAL) {
		send_cmd_unknown(header.cmd_id, EP_CD_SEQ_NO_ERROR);
		userlog(DEBUG_LOG, psName, "RcvCmd(0x%x) sequence no error : %d\n",
			header.cmd_id, header.cmd_serial);
		return -5;
	}
	return 0;
}

int Check_ReplyCmd(char *rcvHeader)
{
	int		rtn;
	//int		seq_no;
	S_MAIN_CMD_HEADER	header;

	memset((char *)&header, 0x00, sizeof(S_MAIN_CMD_HEADER));
	memcpy((char *)&header, (char *)rcvHeader, sizeof(S_MAIN_CMD_HEADER));
	
	rtn = 0;
	/*memset(buf, 0x00, sizeof buf);
	strncpy(buf, (char *)&header.seqno[0], sizeof(S_MAIN_CMD_HEADER));
	seqno = atoi(buf);*/
	
	if(myPs->reply.timer_run == P1) {
		if(myPs->reply.retry.replyCmd == header.cmd_id) {
			myPs->reply.timer_run = P0;
			memset((char *)&myPs->reply.retry, 0x00, sizeof(S_MAIN_RETRY_DATA));
		}
	}
	return rtn;
}

int rcv_cmd_module_info_request(void)
{
	return send_cmd_module_info_reply();
}

int rcv_cmd_module_set_data(void)
{
	int rtn;
	
	S_MAIN_RCV_CMD_MODULE_SET_DATA	cmd;

	memset((char *)&cmd, 0x00, sizeof(S_MAIN_RCV_CMD_MODULE_SET_DATA));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_MAIN_RCV_CMD_MODULE_SET_DATA));
	if(cmd.connection_retry != 0) {
		myPs->signal[MAIN_SIG_NO_CONNECTION_RETRY] = P1;
	}
/*
	if(cmd.auto_report_interval > 0){
		myPs->config.send_monitor_data_interval
			= (unsigned long)cmd.auto_report_interval;
	}
	 if(cmd.data_save_interval > 0){
		myPs->config.send_save_data_interval
			= (unsigned long)cmd.data_save_interval;
	 }
*/
//140716 oys modify
/*
	rtn = Write_MainClient_Config();
	if(rtn < 0) {
		rtn = send_cmd_response((char *)&cmd.header, EP_CD_FILE_WRITE_ERROR);
		return 0;
	}
*/	
	rtn = send_cmd_response((char *)&cmd.header, EP_CD_ACK);
	if(rtn >= 0) {
	//chdata upload timer
	//	myPs->chDataTimer = 15;
		myPs->chDataTimer = 50; //210204 rewirte
		
//20100128 kji w
//20111008 kji 
/*		if(VENDER == 3 || myPs->config.protocol_version < 4102) {
// SKE 
			myPs->signal[MAIN_SIG_NET_CONNECTED] = P2;
			myData->mData.config.sendAuxFlag = P1;
			send_msg(MAIN_TO_MODULE, MSG_MAIN_MODULE_SAVE_MSG_FLAG, 0, 0); //run
		}*/
	}

	return rtn;
}

int rcv_cmd_ch_attribute_request(void)
{
	return send_cmd_ch_attribute_reply();
}

int rcv_cmd_chamber_ch_no_request(void)
{
	return send_cmd_chamber_ch_no_reply();
}

int rcv_cmd_sw_fault_config_request(void)
{
	return send_cmd_sw_fault_config_reply();
}

int rcv_cmd_ch_compdata_request(void)
{
	return send_cmd_ch_compdata_reply();
}

int rcv_cmd_aux_info_request(void)
{
	return send_cmd_aux_info_reply();
}

int rcv_cmd_can_info_request(void)
{
	return send_cmd_can_info_reply();
}

int rcv_cmd_limit_user_vi_request(void)
{
	return send_cmd_limit_user_vi_reply();
}

int rcv_cmd_hwfault_config_request(void)
{
	return send_cmd_hwfault_config_reply();
}
int rcv_cmd_lges_fault_config_request(void)
{
	return send_cmd_lges_fault_config_reply();
}
int rcv_cmd_sdi_cc_cv_hump_config_request(void)
{
	return send_cmd_sdi_cc_cv_hump_config_reply();
}
int rcv_cmd_sdi_pause_save_config_request(void)
{
	return send_cmd_sdi_pause_save_config_reply();
}

int rcv_cmd_acir_value(void)
{
	int i = 0;
	int realCh, bd, ch;
	long voltage, acir;

	S_MAIN_RCV_CMD_ACIR_VALUE cmd;

	memset((char *)&cmd, 0x00, sizeof(S_MAIN_RCV_CMD_ACIR_VALUE));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0], sizeof(S_MAIN_RCV_CMD_ACIR_VALUE));

	realCh = cmd.ch;
	voltage = cmd.voltage;
	acir = cmd.acir;

	for(i = 0 ; i < myData->mData.config.installedCh; i++){
		if(myData->CellArray1[i].number1 == realCh){
			bd = myData->CellArray1[i].bd;
			ch = myData->CellArray1[i].ch;
		}
	}
	if(myData->bData[bd].cData[ch].op.type != STEP_ACIR){
		userlog(DEBUG_LOG, psName, "No STEP_ACIR [%d][%d] , STEP_TYPE[%d], STEP_NUMBER[%d]\n",bd,ch,
		 myData->bData[bd].cData[ch].op.type, myData->bData[bd].cData[ch].misc.advStepNo);
		return send_cmd_response((char *)&cmd.header, EP_CD_MODE_MISMATCH);
	}
	myData->bData[bd].cData[ch].misc.acir_voltage = voltage;
	myData->bData[bd].cData[ch].misc.acir = acir;
	myData->bData[bd].cData[ch].misc.acir_rcv_flag = 1;
	userlog(DEBUG_LOG, psName, "STEP_ACIR bd[%d] ch[%d] voltage : %ld  acir : %ld\n",bd,ch,
		myData->bData[bd].cData[ch].misc.acir_voltage, myData->bData[bd].cData[ch].misc.acir);
	return send_cmd_response((char *)&cmd.header, EP_CD_ACK);
}

int rcv_cmd_dyson_maintenance_config_request(void)
{
	return send_cmd_dyson_maintenance_config_reply();
}

int rcv_cmd_chamber_fault(void)
{
	S_MAIN_RCV_CMD_CHAMBER_FAULT cmd;

	memset((char *)&cmd, 0x00, sizeof(S_MAIN_RCV_CMD_CHAMBER_FAULT));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0], sizeof(S_MAIN_RCV_CMD_CHAMBER_FAULT));

	send_msg(MAIN_TO_MODULE, MSG_MAIN_MODULE_CHAMBER_FAULT, cmd.pauseFlag, cmd.faultCode);
	//pauseFlag 0 AI¸e AI½AA¤Ao, pauseFlag 1 AI¸e ¼E´U¿i
	userlog(DEBUG_LOG, psName, "ChamberFault code[%ld] pauseFlag[%ld]\n",cmd.faultCode,cmd.pauseFlag);
	
	return send_cmd_response((char *)&cmd.header, EP_CD_ACK);
}

int rcv_cmd_buzzer_off(void)
{
	S_MAIN_RCV_CMD_BUZZER_OFF cmd;

	memset((char *)&cmd, 0x00, sizeof(S_MAIN_RCV_CMD_BUZZER_OFF));
	memcpy((char *)&cmd,(char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_MAIN_RCV_CMD_BUZZER_OFF));
    
#if CYCLER_TYPE == DIGITAL_CYC
	myData->mData.signal[M_SIG_LAMP_BUZZER] = P2;
#else
	myPs->signal[MAIN_SIG_LAMP_BUZZER] = P1;
#endif
	
		return send_cmd_response((char *)&cmd.header, EP_CD_ACK);
}

int rcv_cmd_buzzer_on(void)
{
	S_MAIN_RCV_CMD_BUZZER_ON cmd;

	memset((char *)&cmd, 0x00, sizeof(S_MAIN_RCV_CMD_BUZZER_ON));
	memcpy((char *)&cmd,(char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_MAIN_RCV_CMD_BUZZER_ON));

//	myData->mData.signal[M_SIG_LAMP_BUZZER] = P3;
	myPs->signal[MAIN_SIG_LAMP_BUZZER] = P2;
	
		return send_cmd_response((char *)&cmd.header, EP_CD_ACK);
}

int rcv_cmd_dio_control(void)
{	//190901 add for dio control
	unsigned char LampColor, FanFlag;
	S_MAIN_RCV_CMD_DIO_CONTROL		cmd;
	
	memset((char *)&cmd, 0x00, sizeof(S_MAIN_RCV_CMD_DIO_CONTROL));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
							sizeof(S_MAIN_RCV_CMD_DIO_CONTROL));
	
	LampColor = cmd.LampColor;
	switch(LampColor){
		case LAMP_GREEN:
			break;
		case LAMP_YELLOW:
			break;
		case LAMP_RED:
			break;
		case LAMP_BUZZER_ON:
			break;
		case LAMP_BUZZER_OFF:
			myData->mData.signal[M_SIG_LAMP_BUZZER] = P2;
			break;	
		default:
			break;
	}
	
	FanFlag = cmd.FanSignal;
	switch(FanFlag){
		case FAN_ON:
			break;
		case FAN_OFF:
			break;	
		default:
			break;
	}
	
	return send_cmd_response((char *)&cmd.header, EP_CD_ACK);
}

int rcv_cmd_calimeter_set(void)
{	//190901 add for calimeter set
	S_MAIN_RCV_CMD_CALIMETER_SET	cmd;
	
	memset((char *)&cmd, 0x00, sizeof(S_MAIN_RCV_CMD_CALIMETER_SET));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
								sizeof(S_MAIN_RCV_CMD_CALIMETER_SET));

	memcpy((char *)&myData->CaliMeter.rcvMeterSet, 
		(char *)&cmd.caliMeterSet, sizeof(S_MAIN_CALIMETER_SET));	
	send_msg(MAIN_TO_METER, MSG_MAIN_METER_WRITE_SET, 0, 0);

	return send_cmd_response((char *)&cmd.header, EP_CD_ACK);
}

//hun_200219_s
int rcv_cmd_hwfault_request(void)
{
	return send_cmd_hwfault_reply();
}


int rcv_cmd_fault_alarm_request(void)
{
	int cmd_size, body_size, rtn;
	int i=0, j=0;
	int bd=0, ch=0;
	unsigned long chFlag, chFlag1;
	S_MAIN_RCV_CMD_FAULT_ALARM_REQUEST cmd;
	S_MAIN_SEND_CMD_FAULT_ALARM_REPLY cmd2;
	
	memcpy((char *)&cmd,(char *)&myPs->rcvCmd.cmd,
		sizeof(S_MAIN_RCV_CMD_FAULT_ALARM_REQUEST));
	
	cmd_size = sizeof(S_MAIN_SEND_CMD_FAULT_ALARM_REPLY);
	body_size = cmd_size - sizeof(S_MAIN_CMD_HEADER);
	memset((char *)&cmd2, 0, cmd_size);
	make_header((char *)&cmd2, REPLY_NO,MAIN_CMD_TO_PC_FAULT_ALARM_REPLY,
				   	SEQNUM_AUTO, body_size);
	
	chFlag = 0x00000001;
	for(i=0; i < myData->mData.config.installedCh; i++) {
		chFlag1 = (chFlag << i) & cmd.header.chFlag[i / 32];
		if(chFlag1 != 0) {
			bd = myData->CellArray1[i].bd;
			ch = myData->CellArray1[i].ch;
		//	userlog(DEBUG_LOG, psName, "fault_alarm_request [%d][%d]\n",bd,ch);
			if(myData->bData[bd].cData[ch].misc.FaultValFlag == 1){
				cmd2.header.chFlag[0] = cmd.header.chFlag[0];
				cmd2.header.chFlag[1] = cmd.header.chFlag[1];
				for(j=0; j < MAX_FAULT_NUM; j++){
					cmd2.fault_alarm.faultval[j] 
					= myData->bData[bd].cData[ch].misc.FaultVal[j];
		//			userlog(DEBUG_LOG, psName, "faultval[%d] : %d\n"
		//				,j,cmd2.fault_alarm.faultval[j]);
				}
			}
		}
	}
	
	rtn = send_command((char *)&cmd2, cmd_size,
					MAIN_CMD_TO_PC_FAULT_ALARM_REPLY);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d %d\n", rtn,
			MAIN_CMD_TO_PC_FAULT_ALARM_REPLY);
	}
	/*
	else{
		userlog(DEBUG_LOG, psName, "fault_alarm_request [%d][%d][%d][%d][%d][%d][%d]\n",
		cmd2.fault_alarm.faultval[0],cmd2.fault_alarm.faultval[1],
	   	cmd2.fault_alarm.faultval[2],cmd2.fault_alarm.faultval[3],	
		cmd2.fault_alarm.faultval[4],cmd2.fault_alarm.faultval[5], 
		cmd2.fault_alarm.faultval[6]); 
	}
	*/
	return 0;
}
//hun_200219_e
int rcv_cmd_fault_channel_code(void)
{
	int bd, ch=0, i;
	unsigned long chFlag, chFlag1;
	S_MAIN_RCV_CMD_FAULT_CHANNEL_CODE cmd;

	memcpy((char *)&cmd,(char *)&myPs->rcvCmd.cmd,
		sizeof(S_MAIN_RCV_CMD_FAULT_CHANNEL_CODE));

	chFlag = 0x00000001;
	for(i=0; i < myData->mData.config.installedCh; i++) {
		chFlag1 = (chFlag << i) & cmd.header.chFlag[i / 32];

		if(chFlag1 != 0) {
			bd = myData->CellArray1[i].bd;
			ch = myData->CellArray1[i].ch;
			if(myData->bData[bd].cData[ch].ChAttribute.opType == P1) {
				cmd.header.chFlag[i / 32]
					= cmd.header.chFlag[i / 32] | (chFlag << (i + 1));
			}
			if(myData->bData[bd].cData[ch].op.state != C_RUN) {
				return send_cmd_response((char *)&cmd.header,
					EP_CD_CH_STATE_ERROR);
			} else {
				myData->bData[bd].cData[ch].misc.Fault_flag = 1;
				myData->bData[bd].cData[ch].misc.ch_fault_code 
						= cmd.Faultcode;	
			}
		}
	}

	return send_cmd_response((char *)&cmd.header, EP_CD_ACK);
}

int rcv_cmd_fault_gui_code(void)
{
	int bd, ch=0, i;
	S_MAIN_RCV_CMD_FAULT_CHANNEL_CODE cmd;

	memcpy((char *)&cmd,(char *)&myPs->rcvCmd.cmd,
		sizeof(S_MAIN_RCV_CMD_FAULT_CHANNEL_CODE));
			
	for(i=0; i < myData->mData.config.installedCh; i++) {
		bd = myData->CellArray1[i].bd;
		ch = myData->CellArray1[i].ch;
		if(myData->bData[bd].cData[ch].op.state != C_RUN) {
		}else{
			if(myData->bData[bd].cData[ch].ChAttribute.chNo_master == P0) {
				myData->bData[bd].cData[ch].misc.Fault_flag = 1;
				myData->bData[bd].cData[ch].misc.ch_fault_code 
						= cmd.Faultcode;
			}else if(myData->bData[bd].cData[ch].ChAttribute.opType == P0){
				myData->bData[bd].cData[ch].misc.Fault_flag = 1;
				myData->bData[bd].cData[ch].misc.ch_fault_code 
						= cmd.Faultcode;
			}
		}
	}

	return send_cmd_response((char *)&cmd.header, EP_CD_ACK);
}

int send_cmd_aux_info_reply(void)
{
	int cmd_size, body_size,rtn;
	S_MAIN_SEND_CMD_AUX_INFO_REPLY cmd;
	
	cmd_size = sizeof(S_MAIN_SEND_CMD_AUX_INFO_REPLY);
	body_size = cmd_size - sizeof(S_MAIN_CMD_HEADER);
	memset((char *)&cmd, 0, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		MAIN_CMD_TO_PC_AUX_INFO_REPLY, SEQNUM_AUTO, body_size);
	
	cmd.installedTemp = (short int)myData->mData.config.installedTemp;
	cmd.installedAuxV = (short int)myData->mData.config.installedAuxV;

//	memcpy((char *)&cmd.auxSetData[0],(char *)&myData->auxSetData[0],
//		sizeof(S_MAIN_SEND_CMD_AUX_INFO_REPLY) * MAX_AUX_DATA);
	// pjy 2010. 12. 25 different size.

#if NETWORK_VERSION > 4102	
	memcpy((char *)&cmd.auxSetData[0],(char *)&myData->auxSetData[0],
		sizeof(S_MAIN_AUX_SET_DATA) * MAX_AUX_DATA);
#endif

	rtn = send_command((char *)&cmd, cmd_size, MAIN_CMD_TO_PC_AUX_INFO_REPLY);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d %d\n", rtn,
			MAIN_CMD_TO_PC_AUX_INFO_REPLY);
	}
	return 0;
}

int send_cmd_can_info_reply(void)
{
/*	int cmd_size, body_size, rtn, toPs;
	S_MAIN_SEND_CMD_CAN_INFO_REPLY cmd;
	S_MSG_VAL SendMsg;
	
	cmd_size = sizeof(S_MAIN_SEND_CMD_CAN_INFO_REPLY);
	body_size = cmd_size - sizeof(S_MAIN_CMD_HEADER);
	memset((char *)&cmd, 0, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		MAIN_CMD_TO_PC_CAN_INFO_REPLY, SEQNUM_AUTO, body_size);
	
	for(rtn=0; rtn < MAX_CH_PER_MODULE; rtn++) {
		cmd.canDataCount[rtn] = (short int)(myData->canDataCount[rtn][0]
			+ myData->canDataCount[rtn][1]);
	}

	memcpy((char *)&cmd.canSetData, (char *)&myData->canSetData,
		sizeof(S_MAIN_CAN_SET_DATA));

	rtn = send_command((char *)&cmd, cmd_size, MAIN_CMD_TO_PC_CAN_INFO_REPLY);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d %d\n", rtn,
			MAIN_CMD_TO_PC_CAN_INFO_REPLY);
	} else {
		myPs->signal[MAIN_SIG_NET_CONNECTED] = P1;
		myPs->netTimer = myData->mData.misc.timer_1sec;
		toPs = MAIN_TO_MODULE + myPs->config.groupNo;
		memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
		SendMsg.msg = MSG_COA_MODULE_SAVE_MSG_FLAG;
		send_msg(toPs, (char *)&SendMsg); //run
	}
*/
	return 0;
}

int send_cmd_ch_attribute_reply(void)
{
	int cmd_size, body_size, rtn, i, bd, ch;
	unsigned char offset=0;
	S_MAIN_SEND_CMD_CH_ATTRIBUTE_REPLY cmd;
	
	cmd_size = sizeof(S_MAIN_SEND_CMD_CH_ATTRIBUTE_REPLY);
	body_size = cmd_size - sizeof(S_MAIN_CMD_HEADER);
	memset((char *)&cmd, 0, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		MAIN_CMD_TO_PC_CH_ATTRIBUTE_REPLY, SEQNUM_AUTO, body_size);

	for(i=0; i < MAX_CH_PER_MODULE; i++) {
		if(i < myData->mData.config.installedCh) {
			offset = rand();
			bd = myData->CellArray1[i].bd;
			ch = myData->CellArray1[i].ch;
			memcpy((char *)&cmd.attr[i], 
				(char *)&myData->bData[bd].cData[ch].ChAttribute,
				sizeof(S_MAIN_CH_ATTRIBUTE));
			if(myData->AppControl.config.debugType != P0) {
				myData->bData[bd].cData[ch].misc.simVsens = 3500000 + offset;
			}
		} else {
			memset((char *)&cmd.attr[i], 0, sizeof(S_MAIN_CH_ATTRIBUTE));
		}

	}
//	memcpy((char *)&cmd.attr, (char *)&myData->ChAttribute,
//		sizeof(S_MAIN_CH_ATTRIBUTE) * MAX_CH_PER_MODULE);

	rtn = send_command((char *)&cmd, cmd_size,
		MAIN_CMD_TO_PC_CH_ATTRIBUTE_REPLY);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d %d\n", rtn,
			MAIN_CMD_TO_PC_CH_ATTRIBUTE_REPLY);
	} else {
//20100128 kji w
/*		if(myPs->config.protocol_version > 4101 || 
				 myData->mData.config.parallelMode == P1) {
			myPs->signal[MAIN_SIG_NET_CONNECTED] = P2;
			myData->mData.config.sendAuxFlag = P1;
			send_msg(MAIN_TO_MODULE, MSG_MAIN_MODULE_SAVE_MSG_FLAG, 0, 0); //run
		}*/
	}
	return 0;
}

int rcv_cmd_ch_attribute_set(void)
{
	int bd , ch;
	S_MAIN_RCV_CMD_CH_ATTRIBUTE_SET cmd;

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_MAIN_RCV_CMD_CH_ATTRIBUTE_SET));

	if(myData->mData.config.parallelMode == P2) { //kjg_180531
		return send_cmd_response((char *)&cmd.header, EP_CD_NACK);
	} else {
		for(bd=0; bd < myData->mData.config.installedBd; bd++) {
			for(ch=0; ch < myData->mData.config.chPerBd; ch++) {
				memcpy((char *)&myData->bData[bd].cData[ch].ChAttribute, 
					(char *)&cmd.attr[myData->mData.config.chPerBd * bd + ch],
					sizeof(S_MAIN_CH_ATTRIBUTE));
			}
		}

		send_msg(MAIN_TO_APP, MSG_MAIN_APP_WRITE_CHATTRIBUTE, 0, 0);

		return send_cmd_response((char *)&cmd.header, EP_CD_ACK);
	}
}

int send_cmd_chamber_ch_no_reply(void)
{
	int cmd_size, body_size, rtn, i;
	S_MAIN_SEND_CMD_CHAMBER_CH_NO_REPLY cmd;
	
	cmd_size = sizeof(S_MAIN_SEND_CMD_CHAMBER_CH_NO_REPLY);
	body_size = cmd_size - sizeof(S_MAIN_CMD_HEADER);
	memset((char *)&cmd, 0, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		MAIN_CMD_TO_PC_CHAMBER_CH_NO_REPLY, SEQNUM_AUTO, body_size);

	for(i=0; i < MAX_CH_PER_MODULE; i++) {
		cmd.attr[i].chamberNo = myData->ChamberChNo[i].number1;
		cmd.attr[i].hw_no = myData->ChamberChNo[i].number2;
		cmd.attr[i].bd = myData->ChamberChNo[i].bd;
		cmd.attr[i].ch = myData->ChamberChNo[i].ch;
	}
	//userlog(DEBUG_LOG,psName, "send_cmd_chamber_ch_no_reply\n");
	rtn = send_command((char *)&cmd, cmd_size,
		MAIN_CMD_TO_PC_CHAMBER_CH_NO_REPLY);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d %d\n", rtn,
			MAIN_CMD_TO_PC_CHAMBER_CH_NO_REPLY);
	} 
	return 0;
}

int rcv_cmd_chamber_ch_no_set(void)
{
	int i = 0;
	S_MAIN_RCV_CMD_CHAMBER_CH_NO_SET cmd;

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_MAIN_RCV_CMD_CHAMBER_CH_NO_SET));

	for(i=0; i < MAX_CH_PER_MODULE; i++) {
		myData->ChamberChNo[i].number1 = cmd.attr[i].chamberNo;
		myData->ChamberChNo[i].number2 = cmd.attr[i].hw_no; 
		myData->ChamberChNo[i].bd = cmd.attr[i].bd;
		myData->ChamberChNo[i].ch = cmd.attr[i].ch; 
	}
	userlog(DEBUG_LOG,psName, "rcv_cmd_chamber_ch_no_set\n");
	send_msg(MAIN_TO_APP, MSG_MAIN_APP_WRITE_CHAMBER_CH_NO, 0, 0);

	return send_cmd_response((char *)&cmd.header, EP_CD_ACK);
}

int send_cmd_sw_fault_config_reply(void)
{
	int cmd_size, body_size, rtn;
	S_MAIN_SEND_CMD_SW_FAULT_CONFIG_REPLY cmd;
	
	cmd_size = sizeof(S_MAIN_SEND_CMD_SW_FAULT_CONFIG_REPLY);
	body_size = cmd_size - sizeof(S_MAIN_CMD_HEADER);
	memset((char *)&cmd, 0, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		MAIN_CMD_TO_PC_SW_FAULT_CONFIG_REPLY, SEQNUM_AUTO, body_size);

	cmd.sw_fault_config.chamber_gas_voltage_min
	 = myData->mData.config.swFaultConfig[CHAMBER_GAS_VOLTAGE_MIN];
	cmd.sw_fault_config.chamber_gas_voltage_max
	 = myData->mData.config.swFaultConfig[CHAMBER_GAS_VOLTAGE_MAX];
	cmd.sw_fault_config.rest_check_start_time
	 = myData->mData.config.swFaultConfig[REST_CHECK_START_TIME];
	cmd.sw_fault_config.rest_start_compare_voltage
	 = myData->mData.config.swFaultConfig[REST_START_COMPARE_VOLTAGE];
	cmd.sw_fault_config.rest_compare_voltage_delta_v1
	 = myData->mData.config.swFaultConfig[REST_COMPARE_VOLTAGE_DELTA_V1];
	cmd.sw_fault_config.rest_compare_voltage_delta_v2
	 = myData->mData.config.swFaultConfig[REST_COMPARE_VOLTAGE_DELTA_V2];
	cmd.sw_fault_config.rest_fault_check_count
	 = myData->mData.config.swFaultConfig[REST_FAULT_CHECK_COUNT];
	cmd.sw_fault_config.gasCheckTime
	 = myData->mData.config.swFaultConfig[GAS_CHECK_TIME];
	cmd.sw_fault_config.ambient_temp_max
	 = myData->mData.config.swFaultConfig[AMBIENT_TEMP_MAX];
	cmd.sw_fault_config.ambient_temp_max_checkTime
	 = myData->mData.config.swFaultConfig[AMBIENT_TEMP_MAX_TIME];
	cmd.sw_fault_config.ambient_temp_diff
	 = myData->mData.config.swFaultConfig[AMBIENT_TEMP_DIFF];
	cmd.sw_fault_config.ambient_temp_diff_checkTime
	 = myData->mData.config.swFaultConfig[AMBIENT_TEMP_DIFF_TIME];
	cmd.sw_fault_config.soft_venting_count
	 = myData->mData.config.swFaultConfig[SOFT_VENTING_COUNT];
	cmd.sw_fault_config.soft_venting_value
	 = myData->mData.config.swFaultConfig[SOFT_VENTING_VALUE];
	cmd.sw_fault_config.hard_venting_value
	 = myData->mData.config.swFaultConfig[HARD_VENTING_VALUE];

	rtn = send_command((char *)&cmd, cmd_size,
		MAIN_CMD_TO_PC_SW_FAULT_CONFIG_REPLY);
	//userlog(DEBUG_LOG,psName, "send_cmd_sw_fault_config_reply\n");
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d %d\n", rtn,
			MAIN_CMD_TO_PC_SW_FAULT_CONFIG_REPLY);
	} 
	return 0;
}

int rcv_cmd_sw_fault_config_set(void)
{
	S_MAIN_RCV_CMD_SW_FAULT_CONFIG_SET cmd;

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_MAIN_RCV_CMD_SW_FAULT_CONFIG_SET));

	myData->mData.config.swFaultConfig[CHAMBER_GAS_VOLTAGE_MIN]
	 = cmd.sw_fault_config.chamber_gas_voltage_min;
	myData->mData.config.swFaultConfig[CHAMBER_GAS_VOLTAGE_MAX]
	 = cmd.sw_fault_config.chamber_gas_voltage_max;
	myData->mData.config.swFaultConfig[REST_CHECK_START_TIME]
	 = cmd.sw_fault_config.rest_check_start_time;
	myData->mData.config.swFaultConfig[REST_START_COMPARE_VOLTAGE]
	 = cmd.sw_fault_config.rest_start_compare_voltage;
	myData->mData.config.swFaultConfig[REST_COMPARE_VOLTAGE_DELTA_V1]
	 = cmd.sw_fault_config.rest_compare_voltage_delta_v1;
	myData->mData.config.swFaultConfig[REST_COMPARE_VOLTAGE_DELTA_V2]
	 = cmd.sw_fault_config.rest_compare_voltage_delta_v2;
	myData->mData.config.swFaultConfig[REST_FAULT_CHECK_COUNT]
	 = cmd.sw_fault_config.rest_fault_check_count;
	myData->mData.config.swFaultConfig[GAS_CHECK_TIME]
	 = cmd.sw_fault_config.gasCheckTime;
	myData->mData.config.swFaultConfig[AMBIENT_TEMP_MAX]
	 = cmd.sw_fault_config.ambient_temp_max;
	myData->mData.config.swFaultConfig[AMBIENT_TEMP_MAX_TIME]
	 = cmd.sw_fault_config.ambient_temp_max_checkTime;
	myData->mData.config.swFaultConfig[AMBIENT_TEMP_DIFF]
	 = cmd.sw_fault_config.ambient_temp_diff;
	myData->mData.config.swFaultConfig[AMBIENT_TEMP_DIFF_TIME]
	 = cmd.sw_fault_config.ambient_temp_diff_checkTime;
	myData->mData.config.swFaultConfig[SOFT_VENTING_COUNT]
	 = cmd.sw_fault_config.soft_venting_count;
	myData->mData.config.swFaultConfig[SOFT_VENTING_VALUE]
	 = cmd.sw_fault_config.soft_venting_value;
	myData->mData.config.swFaultConfig[HARD_VENTING_VALUE]
	 = cmd.sw_fault_config.hard_venting_value;

	//userlog(DEBUG_LOG,psName, "rcv_cmd_sw_fault_config_set\n");
	send_msg(MAIN_TO_APP, MSG_MAIN_APP_WRITE_SW_FAULT_CONFIG, 0, 0);

	return send_cmd_response((char *)&cmd.header, EP_CD_ACK);
}

int send_cmd_ch_compdata_reply(void)
{
	int cmd_size, body_size, rtn, i, bd, ch;
	S_MAIN_SEND_CMD_CH_COMPDATA_REPLY cmd;
	
	cmd_size = sizeof(S_MAIN_SEND_CMD_CH_COMPDATA_REPLY);
	body_size = cmd_size - sizeof(S_MAIN_CMD_HEADER);
	memset((char *)&cmd, 0, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		MAIN_CMD_TO_PC_CH_COMPDATA_REPLY, SEQNUM_AUTO, body_size);
	for(i=0;i<MAX_CH_PER_MODULE;i++) 
	{
		if(i< myData->mData.config.installedCh)
		{
			bd = myData->CellArray1[i].bd;
			ch = myData->CellArray1[i].ch;
		
			memcpy((char *)&cmd.compData[i], 
				(char *)&myData->bData[bd].cData[ch].ChCompData,
				sizeof(S_MAIN_CH_COMPDATA));
		} else {
			memset((char *)&cmd.compData[i], 0x00,sizeof(S_MAIN_CH_COMPDATA));
		}

	}
	rtn = send_command((char *)&cmd, cmd_size, MAIN_CMD_TO_PC_CH_COMPDATA_REPLY);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d %d\n", rtn,
			MAIN_CMD_TO_PC_CH_ATTRIBUTE_REPLY);
	}
	return 0;
}

int rcv_cmd_ch_compdata_set(void)
{
	int bd , ch;
	S_MAIN_RCV_CMD_CH_COMPDATA_SET cmd;

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_MAIN_RCV_CMD_CH_COMPDATA_SET));
	for(bd=0;bd<myData->mData.config.installedBd;bd++)
	{
		for(ch=0;ch<myData->mData.config.chPerBd;ch++)
		{
			memcpy((char *)&myData->bData[bd].cData[ch].ChCompData, 
				(char *)&cmd.compData[myData->mData.config.chPerBd * bd + ch],
			sizeof(S_MAIN_CH_COMPDATA));
		}
	}
	send_msg(MAIN_TO_APP, MSG_MAIN_APP_WRITE_CHCOMPDATA, 0, 0);
	return send_cmd_response((char *)&cmd.header, EP_CD_ACK);
}

int rcv_cmd_aux_set(void)
{
	S_MAIN_RCV_CMD_AUX_SET cmd;

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_MAIN_RCV_CMD_AUX_SET));

	memcpy((char *)&myData->auxSetData, (char *)&cmd.auxSetData,
		sizeof(S_MAIN_AUX_SET_DATA) * MAX_AUX_DATA);
	send_msg(MAIN_TO_APP, MSG_MAIN_APP_WRITE_AUX_SET_DATA, 0, 0);
	return send_cmd_response((char *)&cmd.header, EP_CD_ACK);
}

int rcv_cmd_can_set(void)
{
/*	int group, toPs, ch, i, j;
	unsigned long chFlag, chFlag1;
	S_MSG_VAL SendMsg;
	S_MAIN_RCV_CMD_CAN_SET cmd;

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_MAIN_RCV_CMD_CAN_SET));

	toPs = ch = 0;
	chFlag = 0x01;
	for(i=0; i < myPs->misc.chInGroup; i++) {
		j = i / 32;
		chFlag1 = (chFlag << (i % 32)) & cmd.header.chFlag[j];
		if(chFlag1 != 0) {
			ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;
			toPs = 1;
			break;
		}
	}

	if(toPs == 0) {
		return send_cmd_response((char *)&cmd.header, EP_CD_NACK);
	}

	memcpy((char *)&myData->canSetData, (char *)&cmd.canSetData,
		sizeof(S_MAIN_CAN_SET_DATA));

	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	group = myPs->config.groupNo;
	toPs = MAIN_TO_APP + group;
	SendMsg.msg = MSG_COA_APP_WRITE_CAN_SET_DATA;
	SendMsg.val[0] = group;
	SendMsg.val[1] = ch;
	send_msg(toPs, (char *)&SendMsg);
	
	return send_cmd_response((char *)&cmd.header, EP_CD_ACK);*/
	return 0;
}
//20171008 sch add
int send_cmd_limit_user_vi_reply(void)
{
	int cmd_size, body_size, rtn;
	S_MAIN_SEND_CMD_LIMIT_USER_VI_REPLY cmd;
	
	cmd_size = sizeof(S_MAIN_SEND_CMD_LIMIT_USER_VI_REPLY);
	body_size = cmd_size - sizeof(S_MAIN_CMD_HEADER);
	memset((char *)&cmd, 0, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		MAIN_CMD_TO_PC_LIMIT_USER_VI_REPLY, SEQNUM_AUTO, body_size);
			memcpy((char *)&cmd.LimitVI, 
				(char *)&myData->mData.config.LimitVI,
				sizeof(S_MAIN_LIMIT_USER_VI));

	rtn = send_command((char *)&cmd, cmd_size, MAIN_CMD_TO_PC_LIMIT_USER_VI_REPLY);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d %d\n", rtn,
			MAIN_CMD_TO_PC_LIMIT_USER_VI_REPLY);
	}
	return 0;
}

int rcv_cmd_limit_user_vi_set(void)
{
	S_MAIN_RCV_CMD_LIMIT_USER_VI_SET cmd;

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_MAIN_RCV_CMD_LIMIT_USER_VI_SET));

	memcpy((char *)&myData->mData.config.LimitVI, 
		(char *)&cmd.LimitVI,
		sizeof(S_MAIN_LIMIT_USER_VI));
	
	send_msg(MAIN_TO_APP, MSG_MAIN_APP_WRITE_LIMIT_USER_VI, 0, 0);
	return send_cmd_response((char *)&cmd.header, EP_CD_ACK);
}

int send_cmd_hwfault_config_reply(void)
{
	int cmd_size, body_size, rtn;
	S_MAIN_SEND_CMD_HWFAULT_CONFIG_REPLY cmd;
	
	cmd_size = sizeof(S_MAIN_SEND_CMD_HWFAULT_CONFIG_REPLY);
	body_size = cmd_size - sizeof(S_MAIN_CMD_HEADER);
	memset((char *)&cmd, 0, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		MAIN_CMD_TO_PC_HWFAULT_CONFIG_REPLY, SEQNUM_AUTO, body_size);

	cmd.hwFaultConfig.ovpVal
		= myData->mData.config.hwFaultConfig[HW_FAULT_OVP];
	cmd.hwFaultConfig.cccVal
		= myData->mData.config.hwFaultConfig[HW_FAULT_CCC];	
	cmd.hwFaultConfig.otpVal
		= myData->mData.config.hwFaultConfig[HW_FAULT_OTP];	
	cmd.hwFaultConfig.dropV_Charge
		= myData->mData.config.hwFaultConfig[HW_FAULT_DROP_V_1];	
	cmd.hwFaultConfig.dropV_Discharge
		= myData->mData.config.hwFaultConfig[HW_FAULT_DROP_V_2];
	cmd.hwFaultConfig.cvFaultV
		= myData->mData.config.hwFaultConfig[HW_FAULT_CV_VOLTAGE];	
	cmd.hwFaultConfig.cvFaultI
		= myData->mData.config.hwFaultConfig[HW_FAULT_CV_CURRENT];	

	rtn = send_command((char *)&cmd, cmd_size, 
					MAIN_CMD_TO_PC_HWFAULT_CONFIG_REPLY);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d %d\n", rtn,
			MAIN_CMD_TO_PC_HWFAULT_CONFIG_REPLY);
	}
	return 0;
}

int rcv_cmd_hwfault_config_set(void)
{
	S_MAIN_RCV_CMD_HWFAULT_CONFIG_SET cmd;

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_MAIN_RCV_CMD_HWFAULT_CONFIG_SET));

	// Not Use OVP
	/*
	myData->mData.config.hwFaultConfig[HW_FAULT_OVP]	
			= cmd.hwFaultConfig.ovpVal;
	*/
	if(cmd.hwFaultConfig.otpVal == 0 || cmd.hwFaultConfig.otpVal >= 150000){
		return send_cmd_response((char *)&cmd.header, EP_CD_NACK);
	}
	myData->mData.config.hwFaultConfig[HW_FAULT_CCC]	
			= cmd.hwFaultConfig.cccVal;
	myData->mData.config.hwFaultConfig[HW_FAULT_OTP]	
			= cmd.hwFaultConfig.otpVal;
	myData->mData.config.hwFaultConfig[HW_FAULT_DROP_V_1]	
			= cmd.hwFaultConfig.dropV_Charge;
	myData->mData.config.hwFaultConfig[HW_FAULT_DROP_V_2]	
			= cmd.hwFaultConfig.dropV_Discharge;
	myData->mData.config.hwFaultConfig[HW_FAULT_CV_VOLTAGE]	
			= cmd.hwFaultConfig.cvFaultV;
	myData->mData.config.hwFaultConfig[HW_FAULT_CV_CURRENT]	
			= cmd.hwFaultConfig.cvFaultI;
	
	send_msg(MAIN_TO_APP, MSG_MAIN_APP_WRITE_HWFAULT_CONFIG, 0, 0);
	return send_cmd_response((char *)&cmd.header, EP_CD_ACK);
}

//211025 hun
int rcv_cmd_lges_fault_config_set(void)
{
	S_MAIN_RCV_CMD_LGES_FAULT_CONFIG_SET cmd;

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_MAIN_RCV_CMD_LGES_FAULT_CONFIG_SET));
	
	memcpy((char *)&myData->mData.config.LGES_fault_config, 
		(char *)&cmd.faultConfig, sizeof(S_MAIN_LGES_FAULT_CONFIG));
	
	send_msg(MAIN_TO_APP, MSG_MAIN_APP_WRITE_LGES_FAULT_CONFIG, 0, 0);
	return send_cmd_response((char *)&cmd.header, EP_CD_ACK);
}

int send_cmd_lges_fault_config_reply(void)
{
	int cmd_size, body_size, rtn;
	S_MAIN_SEND_CMD_LGES_FAULT_CONFIG_REPLY cmd;
	
	cmd_size = sizeof(S_MAIN_SEND_CMD_LGES_FAULT_CONFIG_REPLY);
	body_size = cmd_size - sizeof(S_MAIN_CMD_HEADER);
	memset((char *)&cmd, 0, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		MAIN_CMD_TO_PC_LGES_FAULT_CONFIG_REPLY, SEQNUM_AUTO, body_size);
	memcpy((char *)&cmd.faultConfig, (char *)&myData->mData.config.LGES_fault_config, 
		sizeof(S_MAIN_LGES_FAULT_CONFIG));

	rtn = send_command((char *)&cmd, cmd_size, 
					MAIN_CMD_TO_PC_LGES_FAULT_CONFIG_REPLY);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d %d\n", rtn,
			MAIN_CMD_TO_PC_LGES_FAULT_CONFIG_REPLY);
	}
	return 0;
}

int rcv_cmd_sdi_cc_cv_hump_config_set(void)
{
	S_MAIN_RCV_CMD_SDI_CC_CV_HUMP_CONFIG_SET cmd;

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_MAIN_RCV_CMD_SDI_CC_CV_HUMP_CONFIG_SET));
	
	memcpy((char *)&myData->mData.config.SDI_cc_cv_hump, 
		(char *)&cmd.cc_cv_hump_config, sizeof(S_MAIN_SDI_CC_CV_HUMP_CONFIG));
	
	send_msg(MAIN_TO_APP, MSG_MAIN_APP_WRITE_SDI_CC_CV_HUMP_CONFIG, 0, 0);
	return send_cmd_response((char *)&cmd.header, EP_CD_ACK);
}

int send_cmd_sdi_cc_cv_hump_config_reply(void)
{
	int cmd_size, body_size, rtn;
	S_MAIN_SEND_CMD_SDI_CC_CV_HUMP_CONFIG_REPLY cmd;
	
	cmd_size = sizeof(S_MAIN_SEND_CMD_SDI_CC_CV_HUMP_CONFIG_REPLY);
	body_size = cmd_size - sizeof(S_MAIN_CMD_HEADER);
	memset((char *)&cmd, 0, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		MAIN_CMD_TO_PC_SDI_CC_CV_HUMP_CONFIG_REPLY, SEQNUM_AUTO, body_size);
	memcpy((char *)&cmd.cc_cv_hump_config, (char *)&myData->mData.config.SDI_cc_cv_hump, 
		sizeof(S_MAIN_SDI_CC_CV_HUMP_CONFIG));

	rtn = send_command((char *)&cmd, cmd_size, 
					MAIN_CMD_TO_PC_SDI_CC_CV_HUMP_CONFIG_REPLY);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d %d\n", rtn,
			MAIN_CMD_TO_PC_SDI_CC_CV_HUMP_CONFIG_REPLY);
	}
	return 0;
}

int rcv_cmd_sdi_pause_save_config_set(void)
{
	S_MAIN_RCV_CMD_SDI_PAUSE_SAVE_CONFIG_SET cmd;

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_MAIN_RCV_CMD_SDI_PAUSE_SAVE_CONFIG_SET));
	
	memcpy((char *)&myData->mData.config.SDI_pause_save, 
		(char *)&cmd.pause_save_config, sizeof(S_MAIN_SDI_PAUSE_SAVE_CONFIG));
	
	send_msg(MAIN_TO_APP, MSG_MAIN_APP_WRITE_SDI_PAUSE_SAVE_CONFIG, 0, 0);
	return send_cmd_response((char *)&cmd.header, EP_CD_ACK);
}

int send_cmd_sdi_pause_save_config_reply(void)
{
	int cmd_size, body_size, rtn;
	S_MAIN_SEND_CMD_SDI_PAUSE_SAVE_CONFIG_REPLY cmd;
	
	cmd_size = sizeof(S_MAIN_SEND_CMD_SDI_PAUSE_SAVE_CONFIG_REPLY);
	body_size = cmd_size - sizeof(S_MAIN_CMD_HEADER);
	memset((char *)&cmd, 0, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		MAIN_CMD_TO_PC_SDI_PAUSE_SAVE_CONFIG_REPLY, SEQNUM_AUTO, body_size);
	memcpy((char *)&cmd.pause_save_config, (char *)&myData->mData.config.SDI_pause_save, 
		sizeof(S_MAIN_SDI_PAUSE_SAVE_CONFIG));

	rtn = send_command((char *)&cmd, cmd_size, 
					MAIN_CMD_TO_PC_SDI_PAUSE_SAVE_CONFIG_REPLY);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d %d\n", rtn,
			MAIN_CMD_TO_PC_SDI_PAUSE_SAVE_CONFIG_REPLY);
	}
	return 0;
}

//jsh 220118
int rcv_cmd_dyson_maintenance_config_set(void)
{
	S_MAIN_RCV_CMD_DYSON_MAINTENANCE_CONFIG_SET cmd;

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_MAIN_RCV_CMD_DYSON_MAINTENANCE_CONFIG_SET));

	memcpy((char *)&myData->mData.config.DYSON_maintenance,
		(char *)&cmd.maintenanceConfig, sizeof(S_MAIN_DYSON_MAINTENANCE_CONFIG));

	send_msg(MAIN_TO_APP, MSG_MAIN_APP_WRITE_DYSON_MAINTENANCE_CONFIG, 0, 0);
	return send_cmd_response((char *)&cmd.header, EP_CD_ACK);
}

int send_cmd_dyson_maintenance_config_reply(void)
{
	int cmd_size, body_size, rtn;
	S_MAIN_SEND_CMD_DYSON_MAINTENANCE_CONFIG_REPLY cmd;

	cmd_size = sizeof(S_MAIN_SEND_CMD_DYSON_MAINTENANCE_CONFIG_REPLY);
	body_size = cmd_size - sizeof(S_MAIN_CMD_HEADER);
	memset((char *)&cmd, 0, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		MAIN_CMD_TO_PC_DYSON_MAINTENANCE_CONFIG_REPLY, SEQNUM_AUTO, body_size);
	memcpy((char *)&cmd.maintenanceConfig, (char *)&myData->mData.config.DYSON_maintenance,
		sizeof(S_MAIN_DYSON_MAINTENANCE_CONFIG));

	rtn = send_command((char *)&cmd, cmd_size,
					MAIN_CMD_TO_PC_DYSON_MAINTENANCE_CONFIG_REPLY);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d %d\n", rtn,
			MAIN_CMD_TO_PC_DYSON_MAINTENANCE_CONFIG_REPLY);
	}
	return 0;
}


int rcv_cmd_testcond_start(void)
{
	unsigned long chFlag, chFlag1, bd, ch;
	int i, rtn = 0;

	S_MAIN_RCV_CMD_TESTCOND_START	cmd;
	
	memset((char *)&cmd, 0x00, sizeof(S_MAIN_RCV_CMD_TESTCOND_START));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_MAIN_RCV_CMD_TESTCOND_START));
		
	//userlog(DEBUG_LOG,psName,"rcv_cmd_testcond_start\n");
	
	if(cmd.testCondHeader.totalStep > MAX_STEP) {
		return send_cmd_response((char *)&cmd.header,
			EP_CD_TEST_STEP_COUNT_ERROR);
	}

	memcpy((char *)&myPs->testCond.header,
		(char *)&cmd.testCondHeader, sizeof(S_MAIN_TEST_COND_HEADER));

	myPs->signal[MAIN_SIG_TEST_STEP_RCV] = P0;
	myPs->signal[MAIN_SIG_TEST_HEADER_RCV] = P1;

	chFlag = 0x00000001;
	for(i=0; i < myData->mData.config.installedCh; i++) {
		chFlag1 = (chFlag << i) & cmd.header.chFlag[i/32];

		if(chFlag1 != 0) {
			bd = myData->CellArray1[i].bd;
			ch = myData->CellArray1[i].ch;
			//hun_210723
			#ifdef _EXTERNAL_CONTROL
			if(myPs->testCond.header.external_flag == 1){
				myData->bData[bd].cData[ch].misc.chControl = 1;
			}else if(myPs->testCond.header.external_flag == 0){
				myData->bData[bd].cData[ch].misc.chControl = 0;
			}
			#endif
			//testcond update
			if(myData->bData[bd].cData[ch].op.state == C_PAUSE) {
				myData->bData[bd].cData[ch].misc.testCondUpdate = 1;
			} else {
				myData->bData[bd].cData[ch].misc.testCondUpdate = 0;
			}
			//210209 lyhw
			if(VENDER == 1){
				myData->bData[bd].cData[ch].misc.saveDtConfig
					= (long)cmd.testCondHeader.saveDtConfig;
			}else{
				myData->bData[bd].cData[ch].misc.saveDtConfig
					= (long)(cmd.testCondHeader.saveDtConfig * 6000);
			}
			myData->bData[bd].cData[ch].misc.waitFlag
				= cmd.testCondHeader.waitFlag;
			myData->bData[bd].cData[ch].misc.stepSyncFlag
				= cmd.testCondHeader.stepSyncFlag;
			myData->bData[bd].cData[ch].misc.tempWaitType
				= cmd.testCondHeader.tempWaitType;
			//190801 oys add : Use C-Rate value
			myData->bData[bd].cData[ch].misc.cRateUseFlag
				= cmd.testCondHeader.cRateUseFlag;
#ifdef _SDI_SAFETY_V2	
			//210417 LJS for MASTER_RECIPE
			myData->bData[bd].cData[ch].misc.MasterFlag
				= cmd.testCondHeader.MasterFlag;
			/*
			userlog(DEBUG_LOG, psName, "MasterFlag[%d][%d] : %d\n"
			,bd,ch,cmd.testCondHeader.MasterFlag);
			*/
#endif
			//190318 add for cycle_p_sch lyhw
			if(myData->mData.config.parallelMode == P2){
				myData->bData[bd].cData[ch].misc.cycle_p_sch_flag
					= cmd.testCondHeader.cycle_p_sch_flag;
			}
			if(myData->bData[bd].cData[ch].ChAttribute.opType == P1)
			{
				if(VENDER == 1){	//210209 lyhw
					myData->bData[bd].cData[ch+1].misc.saveDtConfig
						= (long)cmd.testCondHeader.saveDtConfig;
				}else{
					//180711 add
					myData->bData[bd].cData[ch+1].misc.saveDtConfig
						= (long)(cmd.testCondHeader.saveDtConfig * 6000);
				}
				myData->bData[bd].cData[ch+1].misc.waitFlag
					= cmd.testCondHeader.waitFlag;
				myData->bData[bd].cData[ch+1].misc.stepSyncFlag
					= cmd.testCondHeader.stepSyncFlag;
				myData->bData[bd].cData[ch+1].misc.tempWaitType
					= cmd.testCondHeader.tempWaitType;
				//190801 oys add : Use C-Rate value
				myData->bData[bd].cData[ch+1].misc.cRateUseFlag
					= cmd.testCondHeader.cRateUseFlag;
			}
#ifdef _TRACKING_MODE
			if(myData->bData[bd].cData[ch].ChAttribute.opType == P1){
				myData->bData[bd].cData[ch+1].misc.connect_check_flag	
					= cmd.testCondHeader.connect_check_flag;
			}else{
				myData->bData[bd].cData[ch].misc.connect_check_flag	
					= cmd.testCondHeader.connect_check_flag;
			}		
#endif
			if(myData->mData.config.function[F_PATTERN_CH_SAVE] != 0) {
				rtn = delete_user_pattern_file(i);
				rtn = delete_user_map_file(i);
			}
#ifdef _TRACKING_MODE
		rtn = delete_tracking_file(i);	
#endif 
		}
	}
	rtn = delete_user_data_file();
	
	return send_cmd_response((char *)&cmd.header, EP_CD_ACK);
}

int rcv_cmd_testcond_safety(void)
{
	S_MAIN_RCV_CMD_TESTCOND_SAFETY	cmd;
	memset((char *)&cmd, 0, sizeof(S_MAIN_RCV_CMD_TESTCOND_SAFETY));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_MAIN_RCV_CMD_TESTCOND_SAFETY));

	memcpy((char *)&myPs->testCond.safety,
		(char *)&cmd.safety, sizeof(S_MAIN_ADP_TEST_COND_SAFETY));
	//userlog(DEBUG_LOG,psName,"rcv_cmd_testcond_safety\n");
	
	return send_cmd_response((char *)&cmd.header, EP_CD_ACK);
}

int rcv_cmd_testcond_step(void)
{
	int step;
	S_MAIN_RCV_CMD_TESTCOND_STEP cmd;

	memset((char *)&cmd, 0, sizeof(S_MAIN_RCV_CMD_TESTCOND_STEP));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_MAIN_RCV_CMD_TESTCOND_STEP));
	
	if(myPs->signal[MAIN_SIG_TEST_HEADER_RCV] != P1) {
		return send_cmd_response((char *)&cmd.header,
			EP_CD_TEST_HEADER_UNRCV);
	}
	
	step = (int)myPs->signal[MAIN_SIG_TEST_STEP_RCV];
	if((step+1) > (int)myPs->testCond.header.totalStep || step < 0) {
		return send_cmd_response((char *)&cmd.header,
			EP_CD_TEST_STEP_ERROR);
	}

	memcpy((char *)&myPs->testCond.step[step],
		(char *)&cmd.testCondStep, sizeof(S_MAIN_ADP_TEST_COND_STEP));

	myPs->signal[MAIN_SIG_TEST_STEP_RCV]++;
	//userlog(DEBUG_LOG,psName,"rcv_cmd_testcond_step\n");

	return send_cmd_response((char *)&cmd.header, EP_CD_ACK);
}

//171212 add for pattern ftp
//191030 oys modify
int rcv_cmd_testcond_user_pattern(int type)
{
	int i, j, rtn = 0, userDataNo = 0, setDataNo = 0;
	int bd, ch;
	unsigned long chFlag, chFlag1;
	S_MAIN_RCV_CMD_TESTCOND_USER_PATTERN	cmd;
	
	memset((char *)&cmd, 0x00, sizeof(S_MAIN_RCV_CMD_TESTCOND_USER_PATTERN));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_MAIN_RCV_CMD_TESTCOND_USER_PATTERN));
	
	i = (int)cmd.userPattern.stepNo;
	if(i > (int)myPs->testCond.header.totalStep || i < 0) {
		return send_cmd_response((char *)&cmd.header,
			EP_CD_TEST_STEP_ERROR);
	}
	
	i = (int)cmd.userPattern.length;
	if(i > MAX_USER_PATTERN_DATA || i < 0) {
		return send_cmd_response((char *)&cmd.header,
			EP_CD_TEST_STEP_ERROR);
	}

	chFlag = 0x00000001;
	for(i=0; i < myData->mData.config.installedCh; i++) {
		chFlag1 = (chFlag << i) & cmd.header.chFlag[i/32];
		
		if(chFlag1 != 0) {
			bd = myData->CellArray1[i].bd;
			ch = myData->CellArray1[i].ch;
			
			if(myData->bData[bd].cData[ch].misc.userDataNo == 0
				&& userDataNo == 0 && setDataNo == 0) {
				//select userDataNo start
				for(j = 0; j < MAX_CH_PER_BD; j++) {
					if(myData->mData.misc.usingUserDataFlag[j] != P0){
						continue;
					} else {
						myData->mData.misc.usingUserDataFlag[j] = i+1;
						userDataNo = j+1;
						break;
					}
				}
				//end
			} else if(myData->bData[bd].cData[ch].misc.userDataNo != 0
				&& userDataNo == 0 && setDataNo == 0) {
				userDataNo = myData->bData[bd].cData[ch].misc.userDataNo;
			}
			
			myData->bData[bd].cData[ch].misc.userDataNo = userDataNo;
			if(myData->bData[bd].cData[ch].ChAttribute.opType == P1){
				myData->bData[bd].cData[ch+1].misc.userDataNo = userDataNo;
			}
			if(setDataNo == 0) {
				if(type == 0){
					if(myData->bData[bd].cData[ch].ChAttribute.opType == P1){
						rtn = make_user_pattern_file(i, userDataNo, cmd, 1);
					} else {
						rtn = make_user_pattern_file(i, userDataNo, cmd, 0);
					}
				} else if(type == 1){
					rtn = user_data_copy(i, cmd.userPattern.stepNo, userDataNo);
				}
				setDataNo++;
			}
						
			if(rtn < 0){
				return send_cmd_response((char *)&cmd.header, EP_CD_NACK);
			}
		}
	}
	return send_cmd_response((char *)&cmd.header, EP_CD_ACK);
}

//20200427 rewrite
int rcv_cmd_testcond_user_pattern_NoUserData(int type)
{
	int i, rtn = 0;
	int bd, ch;
	//111215 kji add
	int orgCh=-1;
	int orgParallelCh=-1;
	unsigned long chFlag, chFlag1;
	S_MAIN_RCV_CMD_TESTCOND_USER_PATTERN	cmd;
	
	memset((char *)&cmd, 0x00, sizeof(S_MAIN_RCV_CMD_TESTCOND_USER_PATTERN));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_MAIN_RCV_CMD_TESTCOND_USER_PATTERN));
	
	i = (int)cmd.userPattern.stepNo;
	if(i > (int)myPs->testCond.header.totalStep || i < 0) {
		return send_cmd_response((char *)&cmd.header,
			EP_CD_TEST_STEP_ERROR);
	}
	
	i = (int)cmd.userPattern.length;
	if(i > MAX_USER_PATTERN_DATA || i < 0) {
		return send_cmd_response((char *)&cmd.header,
			EP_CD_TEST_STEP_ERROR);
	}

	chFlag = 0x00000001;
	for(i=0; i < myData->mData.config.installedCh; i++) {
		chFlag1 = (chFlag << i) & cmd.header.chFlag[i/32];
		
		if(chFlag1 != 0) {
			bd = myData->CellArray1[i].bd;
			ch = myData->CellArray1[i].ch;

			if(type !=0) { //FTP
				if(myData->bData[bd].cData[ch].ChAttribute.opType == P1){
					if(orgParallelCh >= 0){ //orgch copy
						rtn = make_user_pattern_file_copy(i,orgParallelCh,cmd.userPattern.stepNo, type);
					}else{	//slave ch
						rtn = make_user_pattern_file_copy(i+1,i,cmd.userPattern.stepNo, type);
					}
					if(orgParallelCh < 0)
						orgParallelCh = i;
				} else{
					if(orgCh >= 0){
						rtn = make_user_pattern_file_copy(i,orgCh,cmd.userPattern.stepNo, type);
					}
					if(orgCh < 0)
						orgCh = i;
				}
			} else { // DEFAULT
				if(myData->bData[bd].cData[ch].ChAttribute.opType == P1){
					if(orgParallelCh >= 0) //orgch copy
						rtn = make_user_pattern_file_copy(i,orgParallelCh,cmd.userPattern.stepNo, type);
					else
						rtn = make_user_pattern_file_NoUserData(i, cmd, 1);
					//slave ch
					rtn = make_user_pattern_file_copy(i+1,i,cmd.userPattern.stepNo, type);
					if(orgParallelCh < 0)
						orgParallelCh = i;
				} else{
					if(orgCh >= 0)
						rtn = make_user_pattern_file_copy(i,orgCh,cmd.userPattern.stepNo, type);
					else
						rtn = make_user_pattern_file_NoUserData(i, cmd, 0);
					if(orgCh < 0)
						orgCh = i;
				}
			}				
			if(rtn < 0){
				return send_cmd_response((char *)&cmd.header, EP_CD_NACK);
			}
		}
	}
	return send_cmd_response((char *)&cmd.header, EP_CD_ACK);
}

//111215 kji add
int rcv_cmd_testcond_user_map(void)
{
	int i, rtn = 0, j, userDataNo = 0, setDataNo = 0;
	int bd, ch;
	unsigned long chFlag, chFlag1;
	S_MAIN_RCV_CMD_TESTCOND_USER_MAP	cmd;
	memset((char *)&cmd, 0x00, sizeof(S_MAIN_RCV_CMD_TESTCOND_USER_MAP));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_MAIN_RCV_CMD_TESTCOND_USER_MAP));
	
	i = (int)cmd.userMap.stepNo;
	j = i;
	if(i > (int)myPs->testCond.header.totalStep || i < 0) {
		userlog(DEBUG_LOG, psName, "step No %d userMap stepNo Over\n",j);
		return send_cmd_response((char *)&cmd.header,
			EP_CD_TEST_STEP_ERROR);
	}
	
	i = (int)cmd.userMap.maxCapacity;
	//140925 oys modify : current userMap
	if(cmd.userMap.mode == PS_WATT){
		if(i <= 0) {
			userlog(DEBUG_LOG, psName, "step No %d userMap maxCapacity less 0Ah\n",j);
			return send_cmd_response((char *)&cmd.header,
				EP_CD_TEST_STEP_ERROR);
		}
	}

	i = (int)cmd.userMap.ocvTableRow;
	if(i <= 0 || i > MAX_OCV_TABLE_ROW) {
		userlog(DEBUG_LOG, psName, "step No %d userMap ocv table row\n",j);
		return send_cmd_response((char *)&cmd.header,
			EP_CD_TEST_STEP_ERROR);
	}

	i = (int)cmd.userMap.ocvTableCol;
	if(i <= 0 || i > MAX_OCV_TABLE_COL) {
		userlog(DEBUG_LOG, psName, "step No %d userMap ocv table col\n",j);
		return send_cmd_response((char *)&cmd.header,
			EP_CD_TEST_STEP_ERROR);
	}
	i = (int)cmd.userMap.dataTableRow;
	if(i <= 0 || i > MAX_DATA_TABLE_ROW) {
		userlog(DEBUG_LOG, psName, "step No %d userMap data table row\n",j);
		return send_cmd_response((char *)&cmd.header,
			EP_CD_TEST_STEP_ERROR);
	}
	i = (int)cmd.userMap.dataTableCol;
	if(i <= 0 || i > MAX_DATA_TABLE_COL) {
		userlog(DEBUG_LOG, psName, "step No %d userMap data table col\n",j);
		return send_cmd_response((char *)&cmd.header,
			EP_CD_TEST_STEP_ERROR);
	}
	chFlag = 0x00000001;
	
	for(i=0; i < myData->mData.config.installedCh; i++) {
		chFlag1 = (chFlag << i) & cmd.header.chFlag[i/32];
		
		if(chFlag1 != 0) {
			bd = myData->CellArray1[i].bd;
			ch = myData->CellArray1[i].ch;
			
			if(myData->bData[bd].cData[ch].misc.userDataNo == 0
				&& userDataNo == 0 && setDataNo == 0) {
				//select userDataNo start
				for(j = 0; j < MAX_CH_PER_BD; j++) {
					if(myData->mData.misc.usingUserDataFlag[j] != P0){
						continue;
					} else {
						myData->mData.misc.usingUserDataFlag[j] = i+1;
						userDataNo = j+1;
						break;
					}
				}
				//end
			} else if(myData->bData[bd].cData[ch].misc.userDataNo != 0
				&& userDataNo == 0 && setDataNo == 0) {
				userDataNo = myData->bData[bd].cData[ch].misc.userDataNo;
			}

			myData->bData[bd].cData[ch].misc.userDataNo = userDataNo;
			if(myData->bData[bd].cData[ch].ChAttribute.opType == P1){
				myData->bData[bd].cData[ch+1].misc.userDataNo = userDataNo;
			}
			if(setDataNo == 0) {
				if(myData->bData[bd].cData[ch].ChAttribute.opType == P1){
					rtn = make_user_map_file(i, userDataNo, cmd, 1);
				} else {
					rtn = make_user_map_file(i, userDataNo, cmd, 0);
				}
				setDataNo++;
			}
						
			if(rtn < 0){
				return send_cmd_response((char *)&cmd.header, EP_CD_NACK);
			}
		}
	}
	return send_cmd_response((char *)&cmd.header, EP_CD_ACK);
}

int make_user_pattern_file(int ch, int userDataNo, S_MAIN_RCV_CMD_TESTCOND_USER_PATTERN cmd, int type)
{
	int rtn = 0, i;
	char fileName[128];
	long maxI;
	struct tm *tm;
	time_t t;
	FILE *fp;


	S_MAIN_ADP_TEST_COND_USER_PATTERN_DATA	data[MAX_USER_PATTERN_DATA];

	memset((char *)&data, 0x00
		, sizeof(S_MAIN_ADP_TEST_COND_USER_PATTERN_DATA)*MAX_USER_PATTERN_DATA);
	memcpy((char *)&data, (char *)(myPs->rcvCmd.cmd
		+ sizeof(S_MAIN_RCV_CMD_TESTCOND_USER_PATTERN))
		, sizeof(S_MAIN_ADP_TEST_COND_USER_PATTERN_DATA) * cmd.userPattern.length);
	if(cmd.userPattern.length == 0){
		userlog(DEBUG_LOG, psName, "ch%02d userPattern_step%03ld.csv file length : %d Error\n", ch+1, cmd.userPattern.length);
		return -1;
	}
	if(type)
		maxI = myData->mData.config.maxCurrent[0]*2;
	else
		maxI = myData->mData.config.maxCurrent[0];

	for(i = 0; i < cmd.userPattern.length; i++){
		switch((unsigned char)cmd.userPattern.type){
			case PS_CURRENT:
				if(labs(data[i].data) > maxI){
					if(data[i].data < 0){
						data[i].data = maxI*(-1);
					}else{
						data[i].data = maxI;
					}
				}
				break;
			default:
				break;
		}
	}

	memset(fileName, 0x00, sizeof(fileName));
	sprintf(fileName, "/root/cycler_data/userData/%02d/userPattern_step_%03ld.csv", userDataNo, cmd.userPattern.stepNo);

	fp = fopen(fileName, "w+");
	if(fp == NULL || fp < 0){
		userlog(DEBUG_LOG, psName, "ch%02d userPattern_step%03ld.csv file Open fail\n", ch+1, cmd.userPattern.stepNo);
		return -1;
	}
	time(&t);
	tm = localtime(&t);
	fprintf(fp, "Date	: %04d/%02d/%02d\n"
					,(int)tm->tm_year+1900
					, (int)tm->tm_mon+1
					,(int)tm->tm_mday);

	fprintf(fp, "Time	: %02d:%02d:%02d\n"
					,(int)tm->tm_hour
					, (int)tm->tm_min
					,(int)tm->tm_sec);

	fprintf(fp, "stepNo	: %ld\n",cmd.userPattern.stepNo);
	fprintf(fp, "length : %ld\n",cmd.userPattern.length);
	fprintf(fp, "type	: %ld\n",cmd.userPattern.type);
	fprintf(fp, "stepTime : Data\n");
	for(i = 0; i < cmd.userPattern.length; i++){
		data[i].time = (data[i].time / 10) * 10; //time 100ms
		if(type)
			fprintf(fp, "%ld : %ld\n", data[i].time,(long)(data[i].data/2));
		else
			fprintf(fp, "%ld : %ld\n", data[i].time, data[i].data);
	}
	fclose(fp);

	return rtn;
}

//20200427 rewrite
int make_user_pattern_file_NoUserData
				(int ch, S_MAIN_RCV_CMD_TESTCOND_USER_PATTERN cmd,int type)
{
	int rtn = 0, i;
	char fileName[128];
	long maxI;
	struct tm *tm;
	time_t t;
	FILE *fp;


	S_MAIN_ADP_TEST_COND_USER_PATTERN_DATA	data[MAX_USER_PATTERN_DATA];

	memset((char *)&data, 0x00
		, sizeof(S_MAIN_ADP_TEST_COND_USER_PATTERN_DATA)*MAX_USER_PATTERN_DATA);
	memcpy((char *)&data, (char *)(myPs->rcvCmd.cmd
		+ sizeof(S_MAIN_RCV_CMD_TESTCOND_USER_PATTERN))
		, sizeof(S_MAIN_ADP_TEST_COND_USER_PATTERN_DATA) * cmd.userPattern.length);
	if(cmd.userPattern.length == 0){
		userlog(DEBUG_LOG, psName, "ch%02d userPattern_step%03ld.csv file length : %d Error\n", ch+1, cmd.userPattern.length);
		return -1;
	}
	if(type)
		maxI = myData->mData.config.maxCurrent[0]*2;
	else
		maxI = myData->mData.config.maxCurrent[0];

	for(i = 0; i < cmd.userPattern.length; i++){
		switch((unsigned char)cmd.userPattern.type){
			case PS_CURRENT:
				if(labs(data[i].data) > maxI){
					if(data[i].data < 0){
						data[i].data = maxI*(-1);
					}else{
						data[i].data = maxI;
					}
				}
				break;
			default:
				break;
		}
	}

	memset(fileName, 0x00, sizeof(fileName));
	sprintf(fileName, "/root/cycler_data/pattern/ch%02d/userPattern_step_%03ld.csv",ch+1, cmd.userPattern.stepNo);

	fp = fopen(fileName, "w+");
	if(fp == NULL || fp < 0){
		userlog(DEBUG_LOG, psName, "ch%02d userPattern_step%03ld.csv file Open fail\n", ch+1, cmd.userPattern.stepNo);
		return -1;
	}
	time(&t);
	tm = localtime(&t);
	fprintf(fp, "Date	: %04d/%02d/%02d\n"
					,(int)tm->tm_year+1900
					, (int)tm->tm_mon+1
					,(int)tm->tm_mday);

	fprintf(fp, "Time	: %02d:%02d:%02d\n"
					,(int)tm->tm_hour
					, (int)tm->tm_min
					,(int)tm->tm_sec);

	fprintf(fp, "stepNo	: %ld\n",cmd.userPattern.stepNo);
	fprintf(fp, "length : %ld\n",cmd.userPattern.length);
	fprintf(fp, "type	: %ld\n",cmd.userPattern.type);
	fprintf(fp, "stepTime : Data\n");
	for(i = 0; i < cmd.userPattern.length; i++){
		data[i].time = (data[i].time / 10) * 10; //time 100ms
		if(type)
			fprintf(fp, "%ld : %ld\n", data[i].time,(long)(data[i].data/2));
		else
			fprintf(fp, "%ld : %ld\n", data[i].time, data[i].data);
	}
	fclose(fp);

	return rtn;
}

//111215 kji add
int make_user_map_file (int ch, int userDataNo, S_MAIN_RCV_CMD_TESTCOND_USER_MAP cmd,int type)
{
	int rtn = 0, i,j;
	char fileName[128];
	long maxI;
	struct tm *tm;
	time_t t;
	FILE *fp;

	if(type)
		maxI = myData->mData.config.maxCurrent[0]*2;
	else
		maxI = myData->mData.config.maxCurrent[0];
/*
	for(i = 0; i < cmd.userPattern.length; i++){
		switch((unsigned char)cmd.userPattern.type){
			case PS_CURRENT:
				if(labs(data[i].data) > maxI){
					if(data[i].data < 0){
						data[i].data = maxI*(-1);
					}else{
						data[i].data = maxI;
					}
				}
				break;
			default:
				break;
		}
	}
*/
	memset(fileName, 0x00, sizeof(fileName));
	sprintf(fileName, "/root/cycler_data/userData/%02d/userMap_step_%03ld.csv", userDataNo, cmd.userMap.stepNo);

	fp = fopen(fileName, "w+");
	if(fp == NULL || fp < 0){
		userlog(DEBUG_LOG, psName, "ch%02d userMap_step%03ld.csv file Open fail\n", ch+1, cmd.userMap.stepNo);
		return -1;
	}
	time(&t);
	tm = localtime(&t);
	fprintf(fp, "Date	: %04d/%02d/%02d\n"
					,(int)tm->tm_year+1900
					, (int)tm->tm_mon+1
					,(int)tm->tm_mday);

	fprintf(fp, "Time	: %02d:%02d:%02d\n"
					,(int)tm->tm_hour
					, (int)tm->tm_min
					,(int)tm->tm_sec);

	fprintf(fp, "stepNo	: %ld\n",cmd.userMap.stepNo);
	fprintf(fp, "type	: %d\n",cmd.userMap.type);
	fprintf(fp, "mode	: %d\n",cmd.userMap.mode);
	fprintf(fp, "renewalTime	: %ld\n",cmd.userMap.renewalTime);
	fprintf(fp, "maxCapacity	: %ld\n",cmd.userMap.maxCapacity*1000);
	fprintf(fp, "ocvTableRow : %d\n",cmd.userMap.ocvTableRow);
	fprintf(fp, "ocvTableCol : %d\n",cmd.userMap.ocvTableCol);
	fprintf(fp, "dataTableRow : %d\n",cmd.userMap.dataTableRow);
	fprintf(fp, "dataTableCol : %d\n",cmd.userMap.dataTableCol);
	fprintf(fp, "ocvTable\n");
	for(i = 0; i < cmd.userMap.ocvTableRow; i++){
		fprintf(fp, "%ld %ld\n", cmd.userMap.ocvTable[i][0]*1000,cmd.userMap.ocvTable[i][1]);
	}
	fprintf(fp, "dataTable\n");
	for(i = 0; i <= cmd.userMap.dataTableRow; i++){
		for(j = 0; j <= cmd.userMap.dataTableCol; j++){
			if(i == 0 )
				cmd.userMap.dataTable[i][j] *= 1000;
			fprintf(fp, "%ld ", cmd.userMap.dataTable[i][j]);
		}
		fprintf(fp, "\n");
	}

	fclose(fp);

	return rtn;
}

//191029 oys add
int user_data_copy(int ch, int stepNo, int dataNo)
{
	int fp;
    char cmd[128], fileName[128];

    memset(cmd,0x00,sizeof cmd);
	memset(fileName, 0x00, sizeof(fileName));
	
#if USER_PATTERN_500 == 1
	//191202
	sprintf(fileName, "/root/cycler_data/pattern/ch%02d/GUI_userPattern_step_%03d_%03d.csv", ch+1, stepNo, 1);
#else
	sprintf(fileName, "/root/cycler_data/pattern/ch%02d/GUI_userPattern_step_%03d.csv", ch+1, stepNo);
#endif
	
	if((fp = access(fileName, 0)) >= 0) {
		sprintf(cmd,"mv /root/cycler_data/pattern/ch%02d/* /root/cycler_data/userData/%02d/", ch+1, dataNo);
    	system(cmd);
	}
    return 0;
}

//20200427 rewrite
int make_user_pattern_file_copy(int ch , int ch_org, int stepNo, int type)
{
    char cmd[128];
	
	if(type != 0) {
		memset(cmd,0x00,sizeof cmd);
		sprintf(cmd,"cp -rf /root/cycler_data/pattern/ch%02d/GUI_userPattern_step_%03d.csv /root/cycler_data/pattern/ch%02d/",ch_org+1,stepNo,ch+1);
		system(cmd);
	} else {
		memset(cmd,0x00,sizeof cmd);
		sprintf(cmd,"cp -rf /root/cycler_data/pattern/ch%02d/userPattern_step_%03d.csv /root/cycler_data/pattern/ch%02d/",ch_org+1,stepNo,ch+1);
		system(cmd);
	}
    return 0;
}

int delete_user_pattern_file(int ch)
{
	int rtn = 0;
	char fileName[128];

	memset(fileName, 0x00, sizeof(fileName));
	sprintf(fileName, "rm -rf /root/cycler_data/pattern/ch%02d/*", ch+1);
	system(fileName);

	return rtn;
}

int delete_user_map_file(int ch)
{
	int rtn = 0;
	char fileName[128];

	memset(fileName, 0x00, sizeof(fileName));
	sprintf(fileName, "rm -rf /root/cycler_data/map/ch%02d/*", ch+1);
	system(fileName);

	return rtn;
}
// 191029 oys add : delete not using user data
int delete_user_data_file(void)
{
	int rtn = 0, bd, ch, i, j, flag = 0;
	char cmd[128];

	for(i = 0; i < MAX_CH_PER_BD; i++) {
		if(myData->mData.misc.usingUserDataFlag[i] != P0) {
			for(j = 0; j < myData->mData.config.installedCh; j++) {
				bd = myData->CellArray1[j].bd;
				ch = myData->CellArray1[j].ch;
				if(myData->bData[bd].cData[ch].op.state == C_RUN
					|| myData->bData[bd].cData[ch].op.state == C_PAUSE) {
					if(myData->bData[bd].cData[ch].misc.userDataNo == i+1) {
						flag++;
					}
				} else {
					myData->bData[bd].cData[ch].misc.userDataNo = 0;
				}
				if(myData->bData[bd].cData[ch].misc.testCondUpdate == 1){
					myData->bData[bd].cData[ch].misc.userDataNo = 0;
				}
			}
			if(flag == 0){
				myData->mData.misc.usingUserDataFlag[i] = P0;
				memset(cmd, 0x00, sizeof(cmd));
				sprintf(cmd, "rm -rf /root/cycler_data/userData/%02d/*", i+1);
				system(cmd);
			} else {
				flag = 0;
			}
		}else{
			memset(cmd, 0x00, sizeof(cmd));
			sprintf(cmd, "rm -rf /root/cycler_data/userData/%02d/*", i+1);
			system(cmd);
		}
	}
	return rtn;
}


#ifdef _TRACKING_MODE
int rcv_cmd_testcond_tracking_file(void)
{
	char fileName[256];
	char orgCh = -1;
	int i, bd, ch;
	long stepNo;
	unsigned long chFlag, chFlag1;
	S_MAIN_RCV_CMD_TESTCOND_TRACKING_FILE	cmd;
	
	memset((char *)&cmd, 0x00, sizeof(S_MAIN_RCV_CMD_TESTCOND_TRACKING_FILE));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_MAIN_RCV_CMD_TESTCOND_TRACKING_FILE));

	chFlag = 0x00000001;
	for(i=0; i < myData->mData.config.installedCh; i++) {
		chFlag1 = (chFlag << i) & cmd.header.chFlag[i/32];
		
		if(chFlag1 != 0) {
			bd = myData->CellArray1[i].bd;
			ch = myData->CellArray1[i].ch;
			orgCh = cmd.tracking.orgCh;		
			stepNo = cmd.tracking.stepNo;
			memset(fileName, 0, sizeof fileName);
			if(orgCh >= 0 && ch > orgCh){
				memset(fileName, 0x00, sizeof fileName);
				sprintf(fileName,"cp -rf /root/cycler_data/trackingData/ch%02d/SOC_tracking_data_step_%03ld.csv /root/cycler_data/trackingData/ch%02d/",orgCh+1, stepNo+1, ch+1);
				system(fileName);
			}
		}
	}
	return send_cmd_response((char *)&cmd.header, EP_CD_ACK);
}

int delete_tracking_file(int ch)
{
	int rtn = 0;
	char fileName[128];

	memset(fileName, 0x00, sizeof(fileName));
	sprintf(fileName, "rm -rf /root/cycler_data/trackingData/ch%02d/*", ch+1);
	system(fileName);

	return rtn;
}
#endif

// 131228 oys w : real time add
#if REAL_TIME == 1
int rcv_cmd_real_time_reply(void)
{
	S_MAIN_RCV_CMD_REAL_TIME_REPLY	cmd;

//	userlog(DEBUG_LOG, psName, "rcv_cmd_real_time\n");
	
	memset((char *)&cmd, 0x00, sizeof(S_MAIN_RCV_CMD_REAL_TIME_REPLY));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_MAIN_RCV_CMD_REAL_TIME_REPLY));
	Update_RealTime((char *)&cmd.real_time);
	sleep(1);

	myPs->misc.sent_real_time_request[0]
		= myData->mData.real_time[4];	//day
	myPs->misc.sent_real_time_request[1]
		= myData->mData.real_time[5];	//month
	myPs->misc.sent_real_time_request[2]
		= myData->mData.real_time[6];	//year

//	return send_cmd_response((char *)&cmd.header, EP_CD_ACK);
	return 0;
}
#endif
		
int rcv_cmd_set_measure_data(void)
{
	int bd, ch, i;
	unsigned long	chFlag, chFlag1;
	S_MAIN_RCV_CMD_SET_MEASURE_DATA	cmd;
	//userlog(DEBUG_LOG, psName, "rcv_cmd_set_measure_data\n");

	memset((char *)&cmd, 0x00, sizeof(S_MAIN_RCV_CMD_SET_MEASURE_DATA));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_MAIN_RCV_CMD_SET_MEASURE_DATA));
	
	chFlag = 0x00000001;
	for(i=0; i < myData->mData.config.installedCh; i++) {
		chFlag1 = (chFlag << i) & cmd.header.chFlag[i/32];

		if(chFlag1 != 0) {
			bd = myData->CellArray1[i].bd;
			ch = myData->CellArray1[i].ch;
			if(cmd.id == PS_TEMPERATURE){
				myData->bData[bd].cData[ch].misc.groupTemp
					= cmd.data;
				myData->bData[bd].cData[ch].misc.chamberNo
					= cmd.chamberNo;
				if(myData->bData[bd].cData[ch].ChAttribute.chNo_master == P0){
					myData->bData[bd].cData[ch].misc.groupTemp
						= myData->bData[bd].cData[ch-1].misc.groupTemp;
					myData->bData[bd].cData[ch].misc.chamberNo
						= myData->bData[bd].cData[ch-1].misc.chamberNo;
				}
#if CHAMBER_TEMP_HUMIDITY == 1		//hun_210227
				myData->bData[bd].cData[ch].misc.humi = cmd.humi;
				myData->bData[bd].cData[ch].op.temp = cmd.ch_temp[i];
#endif
			}
		}
	}
	return send_cmd_response((char *)&cmd.header, EP_CD_ACK);
}

int rcv_cmd_set_swelling_data(void)
{	//210316 lyhw
	int bd, ch, i, idx;
	unsigned long	chFlag, chFlag1;
	S_MAIN_RCV_CMD_SET_SWELLING_DATA	cmd;

	memset((char *)&cmd, 0x00, sizeof(S_MAIN_RCV_CMD_SET_SWELLING_DATA));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_MAIN_RCV_CMD_SET_SWELLING_DATA));
	
	chFlag = 0x00000001;
	for(i=0; i < myData->mData.config.installedCh; i++) {
		chFlag1 = (chFlag << i) & cmd.header.chFlag[i/32];

		if(chFlag1 != 0) {
			bd = myData->CellArray1[i].bd;
			ch = myData->CellArray1[i].ch;
			if(cmd.id == PS_SWELLING_DATA){
				for(idx = 0; idx < MAX_CH_PRESSURE_DATA; idx++){
					myData->bData[bd].cData[ch].misc.chPressure[idx]
						= cmd.Pressure[idx];
				}
				for(idx = 0; idx < MAX_CH_THICKNESS_DATA; idx++){
					myData->bData[bd].cData[ch].misc.chThickness[idx]
						= cmd.Thickness[idx];
				}
			}
		}
	}
	return send_cmd_response((char *)&cmd.header, EP_CD_ACK);
}

//20210923 lyhw for lges gas data
int rcv_cmd_set_gas_measure_data(void)
{	
	int	bd, ch, i;
	unsigned long	chFlag, chFlag1;
	S_MAIN_RCV_CMD_SET_GAS_MEASURE_DATA	cmd;

	memset((char *)&cmd, 0x00, sizeof(S_MAIN_RCV_CMD_SET_GAS_MEASURE_DATA));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_MAIN_RCV_CMD_SET_GAS_MEASURE_DATA));
	
	chFlag = 0x00000001;
	for(i=0; i < myData->mData.config.installedCh; i++) {
		chFlag1 = (chFlag << i) & cmd.header.chFlag[i/32];

		if(chFlag1 != 0) {
			bd = myData->CellArray1[i].bd;
			ch = myData->CellArray1[i].ch;
			#if GAS_DATA_CONTROL == 1
			if(cmd.id == PS_GAS_MEASURE_DATA){
				myData->bData[bd].cData[ch].misc.gas_eCo2 = cmd.eCo2;
				myData->bData[bd].cData[ch].misc.gas_Temp = cmd.Temp;
				myData->bData[bd].cData[ch].misc.gas_AH = cmd.AH;
				myData->bData[bd].cData[ch].misc.gas_Baseline = cmd.Baseline;
				myData->bData[bd].cData[ch].misc.gas_TVOC = cmd.TVOC;
				myData->bData[bd].cData[ch].misc.gas_Ethanol = cmd.Ethanol;
				myData->bData[bd].cData[ch].misc.gas_H2 = cmd.H2;
			}
			#endif
		}
	}
	return send_cmd_response((char *)&cmd.header, EP_CD_ACK);
}

int rcv_cmd_testcond_end(void)
{
	unsigned char chGroupNo=0, cnt=0;
	int i, ch, bd;
	unsigned long chFlag, chFlag1;
	S_MAIN_RCV_CMD_TESTCOND_END	cmd;
	
	memset((char *)&cmd, 0, sizeof(S_MAIN_RCV_CMD_TESTCOND_END));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_MAIN_RCV_CMD_TESTCOND_END));	
	//userlog(DEBUG_LOG,psName,"rcv_cmd_testcond_end\n");

	if(myPs->signal[MAIN_SIG_TEST_HEADER_RCV] != P1) {
		return send_cmd_response((char *)&cmd.header,
			EP_CD_TEST_HEADER_UNRCV);
	}
	
	if(myPs->signal[MAIN_SIG_TEST_STEP_RCV] == P0) {
		return send_cmd_response((char *)&cmd.header,
			EP_CD_TEST_STEP_UNRCV);
	}
	
	if((int)myPs->testCond.header.totalStep
		!= (int)myPs->signal[MAIN_SIG_TEST_STEP_RCV]) {
		return send_cmd_response((char *)&cmd.header,
			EP_CD_TEST_STEP_COUNT_ERROR);
	}
	
	myPs->signal[MAIN_SIG_TEST_HEADER_RCV] = P0;
	myPs->signal[MAIN_SIG_TEST_STEP_RCV] = P0;

	chGroupNo = rand();
	for(i=0; i < myData->mData.config.installedCh; i++) {
		bd = myData->CellArray1[i].bd;
		ch = myData->CellArray1[i].ch;
		if(myData->bData[bd].cData[ch].misc.chGroupNo == chGroupNo) cnt++;

		if(cnt > 0) {
			chGroupNo = rand();
			i = 0;
			//20180329 sch modify
			cnt = 0;
		}
	}

	chFlag = 0x00000001;
	for(i=0; i < myData->mData.config.installedCh; i++) {
		chFlag1 = (chFlag << i) & cmd.header.chFlag[i / 32];
		
		if(chFlag1 != 0) {
			bd = myData->CellArray1[i].bd;
			ch = myData->CellArray1[i].ch;

//170105 oys add : ch Grouping process
			if(myData->bData[bd].cData[ch].misc.chamberNo == P0) {
				myData->bData[bd].cData[ch].misc.chGroupNo = chGroupNo;
			} else {
				myData->bData[bd].cData[ch].misc.chGroupNo
					= myData->bData[bd].cData[ch].misc.chamberNo;
			}
			
			if(myData->bData[bd].cData[ch].ChAttribute.opType == P1) {
				if(myData->bData[bd].cData[ch].misc.chamberNo == P0) {
					myData->bData[bd].cData[ch+1].misc.chGroupNo = chGroupNo;
				} else {
					myData->bData[bd].cData[ch+1].misc.chGroupNo
						= myData->bData[bd].cData[ch].misc.chamberNo;
				}
			}
// add end

			if(myData->bData[bd].cData[ch].ChAttribute.opType == P1) {
				Convert_testCond(bd, ch ,1);
		
				memcpy((char *)&myData->mData.testCond[bd][ch+1],
					(char *)&myData->mData.testCond[bd][ch],
					sizeof(S_TEST_CONDITION));
			} else {
				Convert_testCond(bd, ch ,0);
			}
		}
	}
	
	return send_cmd_response((char *)&cmd.header, EP_CD_ACK);
}

int rcv_cmd_step_cond_request(void)
{
	return send_cmd_step_cond_reply();
}

int rcv_cmd_step_cond_update(void)
{
	return 0;
}

int rcv_cmd_safety_cond_request(void)
{
	return send_cmd_safety_cond_reply();
}

int rcv_cmd_safety_cond_update(void)
{
	return 0;
}

int rcv_cmd_reset_reserved_cmd(void)
{
	int bd, ch=0, i;
	unsigned long chFlag, chFlag1;
	S_MAIN_RCV_CMD_RESET_RESERVED_CMD	cmd;

	memset((char *)&cmd, 0x00, sizeof(S_MAIN_RCV_CMD_RESET_RESERVED_CMD));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_MAIN_RCV_CMD_RESET_RESERVED_CMD));
	
	chFlag = 0x00000001;
	for(i=0; i < myData->mData.config.installedCh; i++) {
		chFlag1 = (chFlag << i) & cmd.header.chFlag[i/32];

		if(chFlag1 != 0) {
			bd = myData->CellArray1[i].bd;
			ch = myData->CellArray1[i].ch;

			if(myData->bData[bd].cData[ch].ChAttribute.opType == P1){
				cmd.header.chFlag[i/32] = 
						cmd.header.chFlag[i/32] | (chFlag << (i+1));
			}
			myData->mData.testCond[bd][ch].reserved.reserved_cmd = 0;
			myData->mData.testCond[bd][ch].reserved.reserved_stepNo = 0;
			myData->mData.testCond[bd][ch].reserved.reserved_cycleNo = 0;
			myData->bData[bd].cData[ch].op.reservedCmd
				= myData->mData.testCond[bd][ch].reserved.reserved_cmd;
			myData->mData.testCond[bd][ch].reserved.select_run = 0;
			myData->mData.testCond[bd][ch].reserved.select_stepNo = 0;
			myData->mData.testCond[bd][ch].reserved.select_cycleNo = 0;
			myData->mData.testCond[bd][ch].reserved.select_advCycleStep = 0;
		}
	}
	
	return send_cmd_response((char *)&cmd.header, EP_CD_ACK);
}

int rcv_cmd_cali_meter_connect(void)
{
	S_MAIN_RCV_CMD_CALI_METER_CONNECT	cmd;

	memset((char *)&cmd, 0x00, sizeof(S_MAIN_RCV_CMD_CALI_METER_CONNECT));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_MAIN_RCV_CMD_CALI_METER_CONNECT));

	if(myData->AppControl.config.debugType != P0) {
		send_cmd_meter_connect_reply();
	} else {
		if(cmd.type == 0) { //v
			send_msg(MAIN_TO_METER, MSG_MAIN_METER_INITIALIZE, 0, 0);
			if(myData->AppControl.loadProcess[LOAD_CALIMETER2] == P1){
				send_msg(MAIN_TO_CALIMETER2, MSG_MAIN_METER_INITIALIZE, 0, 0);
			}
		} else { //i
			send_msg(MAIN_TO_METER, MSG_MAIN_METER_INITIALIZE, 0, 1);
		}
	}

	userlog(DEBUG_LOG, psName, "rcv_cmd_cali_meter_connect\n");
	return 0;
}

int rcv_cmd_cali_start(void)
{
#if SHUNT_R_RCV >= 1
	int bd, ch, i, type, range, mode, meas, hallCT;
	long interval;
#else
	int bd, ch, i, type, range, mode;
#endif
	unsigned long chFlag, chFlag1;
	S_MAIN_RCV_CMD_CALI_START	cmd;

	memset((char *)&cmd, 0, sizeof(S_MAIN_RCV_CMD_CALI_START));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_MAIN_RCV_CMD_CALI_START));

	//161220 add for Lan Cali
	if(myData->CaliMeter.config.Lan_Use == P1){
		if(myData->CaliMeter.signal[CALI_METER_SIG_LAN_CONNECT] == P0){
			myData->CaliMeter.signal[CALI_METER_SIG_LAN_CONNECT] = P1;
		}
	}
	if(myData->CaliMeter2.config.Lan_Use == P1){
		if(myData->CaliMeter2.signal[CALI_METER_SIG_LAN_CONNECT] == P0){
			myData->CaliMeter2.signal[CALI_METER_SIG_LAN_CONNECT] = P1;
		}
	}
	
#if SHUNT_R_RCV >= 1
	interval = cmd.tmpCond.interval * 100;
	meas = cmd.tmpCond.meas;
//	hallCT = cmd.tmpCond.hallCT;	//Chekc Siginal 0525	
	hallCT = (long)cmd.tmpCond.hallCT;
	if(meas != CALI && meas != MEAS) {
		return send_cmd_response((char *)&cmd.header, EP_CD_MODE_MISMATCH);
	}	
#endif	

	type = cmd.tmpCond.type;
	if(type >= MAX_TYPE || type < 0) {
		return send_cmd_response((char *)&cmd.header, EP_CD_TYPE_MISMATCH);
	}

	range = cmd.tmpCond.range;
	if(range >= MAX_RANGE || type < 0) {
		return send_cmd_response((char *)&cmd.header, EP_CD_RANGE_MISMATCH);
	}

	mode = cmd.tmpCond.mode;
	if(mode != CALI_MODE_NORMAL && mode != CALI_MODE_CHECK) {
		return send_cmd_response((char *)&cmd.header, EP_CD_MODE_MISMATCH);
	}
	
//140701 nam w : actual measure
#if SHUNT_R_RCV >= 1
	printf("\nUse HallCT : %d (1:Use)\n", cmd.tmpCond.hallCT); //use : 1
	printf("Measure Type : %d (0:Calibration, 1:Actual Measurement)\n",
			cmd.tmpCond.meas); // cali : 0 meas : 1
	meas = cmd.tmpCond.meas;
	if(meas != CALI && meas != MEAS) {
		return send_cmd_response((char *)&cmd.header, EP_CD_MODE_MISMATCH);
	}
#endif
	chFlag = 0x00000001;
	for(i=0; i < myData->mData.config.installedCh; i++) {
		chFlag1 = (chFlag << i) & cmd.header.chFlag[i/32];

		if(chFlag1 != 0) {
			bd = myData->CellArray1[i].bd;
			ch = myData->CellArray1[i].ch;
			printf("cali start bd[%d], ch[%d]\n\n", bd+1, ch+1);

			if(myData->bData[bd].cData[ch].op.state != C_STANDBY) {
				return send_cmd_response((char *)&cmd.header,
					EP_CD_CH_ISNT_STANDBY);
			}

			if(type == 0) {
				send_msg(MAIN_TO_METER, MSG_MAIN_METER_INITIALIZE, 0, 0);
				if(myData->AppControl.loadProcess[LOAD_CALIMETER2] == P1){
					send_msg(MAIN_TO_CALIMETER2, MSG_MAIN_METER_INITIALIZE, 0, 0);
				}
				send_msg(MAIN_TO_MODULE, MSG_MAIN_MODULE_CALI_RELAY, 0, range);
			} else {
				send_msg(MAIN_TO_METER, MSG_MAIN_METER_INITIALIZE, 0, 1);
				send_msg(MAIN_TO_MODULE, MSG_MAIN_MODULE_CALI_RELAY, 1, range);
			}
			myData->cali.tmpCond[bd][ch].type = type;
			myData->cali.tmpCond[bd][ch].range = range;
			myData->cali.tmpCond[bd][ch].mode = mode;
#if SHUNT_R_RCV >= 1
			myData->mData.cali_meas_type = meas;
			myData->mData.cali_hallCT = hallCT;
			myData->mData.cali_meas_time = interval;
#endif			

			memcpy((char *)&myData->cali.tmpCond[bd][ch].point[type][range],
				(char *)&cmd.tmpCond.point, sizeof(S_CALI_POINT));
//111215 detail calibration add
			myData->bData[bd].cData[ch].misc.setCaliNum 
					= cmd.tmpCond.point.setCaliNum;
			myData->bData[bd].cData[ch].misc.checkCaliNum = cmd.tmpCond.
					point.checkCaliNum;

			myData->bData[bd].cData[ch].misc.checkCaliNum = 1;
			
			if(myData->bData[bd].cData[ch].misc.setCaliNum < 1)
				myData->bData[bd].cData[ch].misc.setCaliNum = 1;
			if(myData->bData[bd].cData[ch].misc.checkCaliNum < 1)
				myData->bData[bd].cData[ch].misc.checkCaliNum = 1;

			myData->cali.tmpCond[bd][ch].point[type][range].charge_pointNum = 0;
			myData->cali.tmpCond[bd][ch].point[type][range].discharge_pointNum = 0;
					
			myData->cali.tmpData[bd][ch].caliFlag[type][range] = 0;
			memset((char *)&myData->cali.tmpData[bd][ch]
				.set_ad[type][range][0], 0, sizeof(double)*MAX_CALI_POINT);
			memset((char *)&myData->cali.tmpData[bd][ch]
				.set_meter[type][range][0], 0, sizeof(double)*MAX_CALI_POINT);
			memset((char *)&myData->cali.tmpData[bd][ch]
				.check_ad[type][range][0], 0, sizeof(double)*MAX_CALI_POINT);
			memset((char *)&myData->cali.tmpData[bd][ch]
				.check_meter[type][range][0], 0, sizeof(double)*MAX_CALI_POINT);
			memset((char *)&myData->cali.tmpData[bd][ch]
				.DA_A[type][range][0], 0, sizeof(double)*(MAX_CALI_POINT-1));
			memset((char *)&myData->cali.tmpData[bd][ch]
				.DA_B[type][range][0], 0, sizeof(double)*(MAX_CALI_POINT-1));
			memset((char *)&myData->cali.tmpData[bd][ch]
				.AD_A[type][range][0], 0, sizeof(double)*(MAX_CALI_POINT-1));
			memset((char *)&myData->cali.tmpData[bd][ch]
				.AD_B[type][range][0], 0, sizeof(double)*(MAX_CALI_POINT-1));
			memset((char *)&myData->cali.tmpData[bd][ch]
				.AD_Ratio[type][range][0], 0, sizeof(double)*2);

			send_msg(MAIN_TO_MODULE, MSG_MAIN_MODULE_CH_CALI, 
				(int)myData->CellArray1[i].number2-1, type); //nember2:hw_no
		}
	}
	
	return send_cmd_response((char *)&cmd.header, EP_CD_ACK);
}

int rcv_cmd_cali_update(void)
{
	int bd, ch, i, rtn, write_bd[MAX_BD_PER_MODULE];
	unsigned long chFlag, chFlag1;
	S_MAIN_RCV_CMD_CALI_UPDATE	cmd;

	memset((char *)&cmd, 0x00, sizeof(S_MAIN_RCV_CMD_CALI_UPDATE));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_MAIN_RCV_CMD_CALI_UPDATE));

	for(bd=0; bd < MAX_BD_PER_MODULE; bd++) {
		write_bd[bd] = 0;
	}

	chFlag = 0x00000001;
	for(i=0; i < myData->mData.config.installedCh; i++) {
		chFlag1 = (chFlag << i) & cmd.header.chFlag[i/32];
		
		if(chFlag1 != 0) {
			bd = myData->CellArray1[i].bd;
			ch = myData->CellArray1[i].ch;
			CaliUpdateCh(bd, ch);
#if CYCLER_TYPE == DIGITAL_CYC
			myData->bData[bd].cData[ch]
					.signal[C_SIG_CALI_NORMAL_RESULT_SAVED] = P2;
#endif

#if SHUNT_R_RCV == 1
			Write_Shunt_Cali_Info(bd);
#endif
			write_bd[bd] = 1;
		}
	}

	for(bd=0; bd < MAX_BD_PER_MODULE; bd++) {
		if(write_bd[bd] == 1) {
			rtn = Write_BdCaliData(bd);
			if(rtn < 0) {
				return send_cmd_response((char *)&cmd.header, EP_CD_NACK);
			}
			if(myData->mData.config.FadBdUse == P1){
				rtn = Write_BdCaliData_FAD(bd);
				if(rtn < 0) {
					return send_cmd_response((char *)&cmd.header, EP_CD_NACK);
				}
			}
			if(myData->mData.config.function[F_I_OFFSET_CALI] == P1){
				rtn = Write_BdCaliData_I_Offset(bd);
				if(rtn < 0) {
					return send_cmd_response((char *)&cmd.header, EP_CD_NACK);
				}
			}
		}
	}
	
	return send_cmd_response((char *)&cmd.header, EP_CD_ACK);
}

int rcv_cmd_cali_not_update(void)
{
	int bd, ch, i;
	unsigned long chFlag, chFlag1;
	S_MAIN_RCV_CMD_CALI_NOT_UPDATE	cmd;

	memset((char *)&cmd, 0x00, sizeof(S_MAIN_RCV_CMD_CALI_NOT_UPDATE));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_MAIN_RCV_CMD_CALI_NOT_UPDATE));

	chFlag = 0x00000001;
	for(i=0; i < myData->mData.config.installedCh; i++) {
		chFlag1 = (chFlag << i) & cmd.header.chFlag[i/32];
		
		if(chFlag1 != 0) {
			bd = myData->CellArray1[i].bd;
			ch = myData->CellArray1[i].ch;
			CaliUpdateCh(bd, ch);
			myData->bData[bd].cData[ch]
							.signal[C_SIG_CALI_NORMAL_RESULT_SAVED] = P3;
			printf("No Update Ch[%d]\n", ch+1);
		}
	}
	return send_cmd_response((char *)&cmd.header, EP_CD_ACK);
}

#ifdef _TEMP_CALI  
int rcv_cmd_temp_cali_point_set(void)
{
	unsigned char point_count;
	int i;
	long set_point[MAX_TEMP_POINT];
	S_MAIN_RCV_CMD_TEMP_CALI_POINT_SET cmd;

	memset((char *)&cmd, 0x00, sizeof(S_MAIN_RCV_CMD_TEMP_CALI_POINT_SET));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_MAIN_RCV_CMD_TEMP_CALI_POINT_SET));
		
	point_count = cmd.setPointCount;
	if((point_count < 2) || (point_count > MAX_TEMP_POINT)) {
		userlog(DEBUG_LOG, psName, "temp_setPointCount error");
		return send_cmd_response((char *)&cmd.header,EP_CD_MODE_MISMATCH);
	}		
	for(i=0; i < MAX_TEMP_POINT; i++) {
		set_point[i] = cmd.setTempPoint[i];
		if(i > 0) {
			if(set_point[i] < set_point[i-1]) {
				userlog(DEBUG_LOG, psName,"temp_setTempPoint error");
				return send_cmd_response((char *)&cmd.header, EP_CD_MODE_MISMATCH);
			}
		}
	}
	
	myData->temp_cali.point.setPointCount = cmd.setPointCount;
	userlog(DEBUG_LOG, psName, "temp_cali_point point_count: %d\n",
													cmd.setPointCount);
	for(i=0; i < MAX_TEMP_POINT; i++) {
		if(myData->AppControl.config.debugType != P0) {
			myData->temp_cali.point.setTempPoint[i] = cmd.setTempPoint[i];
			userlog(DEBUG_LOG, psName, "setTempPoint");
			userlog2(DEBUG_LOG, psName, " %ld\n",
			myData->temp_cali.point.setTempPoint[i]);
		} else {
			myData->temp_cali.point.setTempPoint[i] = cmd.setTempPoint[i];
			userlog(DEBUG_LOG, psName, "setTempPoint");
			userlog2(DEBUG_LOG, psName, " %ld\n", cmd.setTempPoint[i]);
		}
	}

	if(myData->AppControl.config.debugType != P0) {
		return send_cmd_response((char *)&cmd.header, EP_CD_ACK);	
	}
		
	userlog(DEBUG_LOG, psName, "Temp_Calibration Point Set Success\n");
	send_msg(MAIN_TO_METER2, MSG_MAIN_METER2_TEMP_CALI_CLEAR, 0, 0);
	return send_cmd_response((char *)&cmd.header, EP_CD_ACK);
}

int rcv_cmd_temp_cali_start(void)
{
	unsigned char pointNo;
	int i, j, caliFlag_count, tempNo, multiNum;
	S_MAIN_RCV_CMD_TEMP_CALI_START cmd;
	
	memset((char *)&cmd, 0x00, sizeof(S_MAIN_RCV_CMD_TEMP_CALI_START));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_MAIN_RCV_CMD_TEMP_CALI_START));	

	tempNo = myData->mData.config.installedCh;
	multiNum = myData->AnalogMeter.config.multiNum;
	
	if(multiNum != 0){
		tempNo = tempNo * multiNum;
	}
	pointNo = cmd.setPointNo;
	if(pointNo > MAX_TEMP_POINT) {
		userlog(DEBUG_LOG, psName, "temp_setPointNo error");
		return send_cmd_response((char *)&cmd.header, EP_CD_MODE_MISMATCH);
	}
				
	caliFlag_count = 0;
	for(i=0; i < MAX_TEMP_CH; i++) {
		if(cmd.temp_caliFlag[i] == 1) caliFlag_count++;
	}

	if((caliFlag_count == 0) || (caliFlag_count != cmd.caliFlagCount)) {
		userlog(DEBUG_LOG, psName, "temp_caliFlag error");
		return send_cmd_response((char *)&cmd.header, EP_CD_MODE_MISMATCH);
	}

	userlog(DEBUG_LOG,psName,"temp_cali_start setPointNo : %d\n", 
										cmd.setPointNo);
	userlog(DEBUG_LOG,psName,"caliFlagCount : %d, setTempValue : %ld\n",
					cmd.caliFlagCount, cmd.setTempValue);
	for(i=0; i < MAX_TEMP_CH; i++) {
		if(myData->AppControl.config.debugType != P0) {
			j = 0;
			myData->temp_cali.data.setTempValue[j][i]= i*10;
			myData->temp_cali.data.temp_caliFlag[j][i] = 1;
			userlog2(DEBUG_LOG, psName, "setTempValue%d(flag:%d)", i,
						myData->temp_cali.data.setTempValue[j][i]);
		} else {
			userlog2(DEBUG_LOG, psName, "%d(flag:%d) ", i+1, cmd.temp_caliFlag[i]);
		}
		if((i+1) % 10 == 0)
			userlog2(DEBUG_LOG, psName, "\n");
	}
	
	if(myData->AppControl.config.debugType != P0) {
		return send_cmd_response((char *)&cmd.header, EP_CD_ACK);
	}

	for(i=0; i < MAX_TEMP_CH; i++) {
		myData->temp_cali.data.temp_caliFlag[pointNo][i] = cmd.temp_caliFlag[i];
	}
	
	myData->temp_cali.point.caliFlagCount = cmd.caliFlagCount;
	
	send_msg(MAIN_TO_METER2, MSG_MAIN_METER2_TEMP_CALI_START,
						  	(int)myData->TempArray1[i].number2-1, pointNo);

	userlog(DEBUG_LOG, psName, "Temp_Calibration Start / pointNo : %d\n", pointNo+1);

	return send_cmd_response((char *)&cmd.header, EP_CD_ACK);
}

int rcv_cmd_temp_cali_info_request(void)
{	
	return send_cmd_temp_cali_info_reply();
}

int send_cmd_temp_cali_info_reply(void)
{	
	unsigned char multiNum;
	int cmd_size, body_size, rtn, i, tempNo;
	S_MAIN_SEND_CMD_TEMP_CALI_INFO_REPLY cmd;
	
	cmd_size = sizeof(S_MAIN_SEND_CMD_TEMP_CALI_INFO_REPLY);
	body_size = cmd_size - sizeof(S_MAIN_CMD_HEADER);
	memset((char *)&cmd, 0, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		MAIN_CMD_TO_PC_TEMP_CALI_INFO_REPLY, SEQNUM_AUTO, body_size);

	if(myData->mData.config.installedTemp != 0){
		tempNo = myData->mData.config.installedTemp;
	}else{
		tempNo = myData->mData.config.installedCh;
	}

	multiNum = myData->AnalogMeter.config.multiNum;
	
	if(multiNum != 0){
		tempNo = tempNo * multiNum;
	}
	
	cmd.tempNo = (short int)tempNo;
	cmd.setPointCount = (short int)myData->temp_cali.point.setPointCount;
	for(i=0; i <MAX_TEMP_POINT; i++) {
		cmd.setTempPoint[i] 
			= (short int)myData->temp_cali.point.setTempPoint[i]; 
	}
	
	rtn = send_command((char *)&cmd, cmd_size,
				   	MAIN_CMD_TO_PC_TEMP_CALI_INFO_REPLY);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d %d\n", rtn,
			MAIN_CMD_TO_PC_TEMP_CALI_INFO_REPLY);
	} 
	return 0;
}

int rcv_cmd_temp_cali_backup_read(void)
{
	userlog(DEBUG_LOG, psName, "rcv_cmd-temp_cali_backup_read\n");
	send_msg(MAIN_TO_METER2, MSG_MAIN_METER2_TEMP_CALI_BACKUP_READ, 0, 0);
	return 0;
}

int send_cmd_temp_cali_backup_read_reply(int response)
{
	int cmd_size, body_size, rtn;
	S_MAIN_SEND_CMD_TEMP_CALI_BACKUP_READ_REPLY cmd;
	
	cmd_size = sizeof(S_MAIN_SEND_CMD_TEMP_CALI_BACKUP_READ_REPLY);
	body_size = cmd_size - sizeof(S_MAIN_CMD_HEADER);
	memset((char *)&cmd, 0, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		MAIN_CMD_TO_PC_TEMP_CALI_BACKUP_READ_REPLY, SEQNUM_AUTO, body_size);
	cmd.response = response;
	userlog(DEBUG_LOG, psName, "send_cmd_temp_cali_backup_read_reply\n");
	
	rtn = send_command((char *)&cmd, cmd_size,
				   	MAIN_CMD_TO_PC_TEMP_CALI_BACKUP_READ_REPLY);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d %d\n", rtn,
			MAIN_CMD_TO_PC_TEMP_CALI_BACKUP_READ_REPLY);
	}
	return 0;	
}
#endif

int rcv_cmd_response(void)
{
	S_MAIN_RCV_CMD_RESPONSE	cmd;

	//kjg_180524 memset((char *)&cmd, 0, sizeof(S_MAIN_RCV_CMD_RESPONSE));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_MAIN_RCV_CMD_RESPONSE));
	
	if(cmd.response.code == EP_CD_ACK) {
		if(cmd.response.cmd == MAIN_CMD_TO_PC_COMM_CHECK) {
			myPs->netTimer = myData->mData.misc.timer_1sec;
		}
	} else {
		return -1;
	}
	return 0;
}

int rcv_cmd_run(void)
{
	unsigned char slave_ch;
	int bd, ch, i, j;
	unsigned long chFlag, chFlag1, advCycleStep;
	S_MSG_CH_FLAG ch_flag;
	S_MAIN_RCV_CMD_RUN cmd;

	//kjg_180524 memset((char *)&cmd, 0, sizeof(S_MAIN_RCV_CMD_RUN));
#if CYCLER_TYPE == DIGITAL_CYC
	memset((char *)&cmd, 0x00, sizeof(S_MAIN_RCV_CMD_RUN));
#endif
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_MAIN_RCV_CMD_RUN));
	
	chFlag = 0x00000001;
	for(i=0; i < myData->mData.config.installedCh; i++) {
		chFlag1 = (chFlag << i) & cmd.header.chFlag[i / 32];

		if(chFlag1 != 0) {
			bd = myData->CellArray1[i].bd;
			ch = myData->CellArray1[i].ch;
			if(myData->bData[bd].cData[ch].op.state != C_STANDBY) {
				return send_cmd_response((char *)&cmd.header,
					EP_CD_CH_STATE_ERROR);
			}
		}
	}

	chFlag = 0x00000001;
	for(i=0; i < myData->mData.config.installedCh; i++) {
		chFlag1 = (chFlag << i) & cmd.header.chFlag[i / 32];
		
		if(chFlag1 != 0) {
			bd = myData->CellArray1[i].bd;
			ch = myData->CellArray1[i].ch;
			if(cmd.control.stepNo > 0 && cmd.control.cycleNo > 0) {
				//reservedCmd
				myData->mData.testCond[bd][ch].reserved.select_run = 1;
				myData->mData.testCond[bd][ch].reserved.select_stepNo
					= (unsigned long)cmd.control.stepNo;
				myData->mData.testCond[bd][ch].reserved.select_cycleNo
					= (unsigned long)cmd.control.cycleNo;
				advCycleStep = 0;
				for(j=0; j < MAX_STEP; j++) {
					if(myData->mData.testCond[bd][ch].step[j].type
						== STEP_ADV_CYCLE
						|| myData->mData.testCond[bd][ch].step[j].type
						== STEP_PARALLEL_CYCLE) { //kjg_180521
						advCycleStep = (unsigned long)j;
					}
					if(j == ((int)cmd.control.stepNo - 1)) {
						break;
					}
				}
				if(j == MAX_STEP) advCycleStep = 0;
				myData->mData.testCond[bd][ch].reserved.select_advCycleStep
					= advCycleStep;
			} else { //direct
				myData->mData.testCond[bd][ch].reserved.select_run = 0;
				myData->mData.testCond[bd][ch].reserved.select_stepNo = 0;
				myData->mData.testCond[bd][ch].reserved.select_cycleNo = 0;
				myData->mData.testCond[bd][ch].reserved.select_advCycleStep = 0;
			}
			myData->mData.testCond[bd][ch].reserved.reserved_cmd = 0;
			myData->mData.testCond[bd][ch].reserved.reserved_stepNo = 0;
			myData->mData.testCond[bd][ch].reserved.reserved_cycleNo = 0;
			myData->bData[bd].cData[ch].op.reservedCmd
				= myData->mData.testCond[bd][ch].reserved.reserved_cmd;
			
			if(myData->bData[bd].cData[ch].ChAttribute.opType == P1) {
				slave_ch = myData->bData[bd].cData[ch].
					ChAttribute.chNo_slave[0] - 1;
				memcpy((char *)&myData->mData.testCond[bd][slave_ch],
					(char *)&myData->mData.testCond[bd][ch],
					sizeof(S_TEST_CONDITION));
			}
		}
	}

	ch_flag.bit_32[0] = cmd.header.chFlag[0];
	ch_flag.bit_32[1] = cmd.header.chFlag[1];
	send_msg_ch_flag(MAIN_TO_DATASAVE, (char *)&ch_flag);
	send_msg(MAIN_TO_DATASAVE, MSG_MAIN_DATASAVE_SAVED_FILE_DELETE, 0, 0);

	return send_cmd_response((char *)&cmd.header, EP_CD_ACK);
}

int rcv_cmd_dll_stop(void)
{
	int bd, ch=0, i, j;
	unsigned long chFlag, chFlag1;
	S_MAIN_RCV_CMD_STOP	cmd;

	//kjg_180524 memset((char *)&cmd, 0, sizeof(S_MAIN_RCV_CMD_STOP));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_MAIN_RCV_CMD_STOP));
	
	chFlag = 0x00000001;
	for(i=0; i < myData->mData.config.installedCh; i++) {
		chFlag1 = (chFlag << i) & cmd.header.chFlag[i / 32];

		if(chFlag1 != 0) {
			bd = myData->CellArray1[i].bd;
			ch = myData->CellArray1[i].ch;
			if(myData->bData[bd].cData[ch].ChAttribute.opType == P1) {
				cmd.header.chFlag[i / 32]
					= cmd.header.chFlag[i / 32] | (chFlag << (i + 1));
			}
			if(myData->bData[bd].cData[ch].op.state != C_RUN
				&& myData->bData[bd].cData[ch].op.state != C_PAUSE) {
				return send_cmd_response((char *)&cmd.header,
					EP_CD_CH_STATE_ERROR);
			} else {
				if(cmd.control.stepNo > 0 && cmd.control.cycleNo > 0) {
					j = 1; //reserved
				} else {
					j = 0; //direct
				}
				if(j == 0) { //direct stop
					myData->mData.testCond[bd][ch].reserved.reserved_cmd = 0;
					myData->mData.testCond[bd][ch].reserved.reserved_stepNo = 0;
					myData->mData.testCond[bd][ch].reserved.reserved_cycleNo
						= 0;
					myData->bData[bd].cData[ch].signal[C_SIG_DLL_STOP] = P1;
					userlog(DEBUG_LOG, psName, "DLL STOP bd[%d]ch[%d]\n",bd,ch);
				} else { //reserved stop
					myData->mData.testCond[bd][ch].reserved.reserved_cmd = 1;
					myData->mData.testCond[bd][ch].reserved.reserved_stepNo
						= (unsigned long)cmd.control.stepNo;
					myData->mData.testCond[bd][ch].reserved.reserved_cycleNo
						= (unsigned long)cmd.control.cycleNo;
				}
				myData->bData[bd].cData[ch].op.reservedCmd
					= myData->mData.testCond[bd][ch].reserved.reserved_cmd;
				myData->mData.testCond[bd][ch].reserved.select_run = 0;
				myData->mData.testCond[bd][ch].reserved.select_stepNo = 0;
				myData->mData.testCond[bd][ch].reserved.select_cycleNo = 0;
				myData->mData.testCond[bd][ch].reserved.select_advCycleStep = 0;
				#ifdef _EXTERNAL_CONTROL
				myData->bData[bd].cData[ch].misc.chPause = P0;
				#endif
			}
		}
	}
	
	return send_cmd_response((char *)&cmd.header, EP_CD_ACK);
}

int rcv_cmd_stop(void)
{
	int bd, ch=0, i, j;
	unsigned long chFlag, chFlag1;
	S_MAIN_RCV_CMD_STOP	cmd;

	//kjg_180524 memset((char *)&cmd, 0, sizeof(S_MAIN_RCV_CMD_STOP));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_MAIN_RCV_CMD_STOP));
	
	chFlag = 0x00000001;
	for(i=0; i < myData->mData.config.installedCh; i++) {
		chFlag1 = (chFlag << i) & cmd.header.chFlag[i / 32];

		if(chFlag1 != 0) {
			bd = myData->CellArray1[i].bd;
			ch = myData->CellArray1[i].ch;
			if(myData->bData[bd].cData[ch].ChAttribute.opType == P1) {
				cmd.header.chFlag[i / 32]
					= cmd.header.chFlag[i / 32] | (chFlag << (i + 1));
			}
			if(myData->bData[bd].cData[ch].op.state != C_RUN
				&& myData->bData[bd].cData[ch].op.state != C_PAUSE) {
				return send_cmd_response((char *)&cmd.header,
					EP_CD_CH_STATE_ERROR);
			} else {
				if(cmd.control.stepNo > 0 && cmd.control.cycleNo > 0) {
					j = 1; //reserved
				} else {
					j = 0; //direct
				}
				if(j == 0) { //direct stop
					myData->mData.testCond[bd][ch].reserved.reserved_cmd = 0;
					myData->mData.testCond[bd][ch].reserved.reserved_stepNo = 0;
					myData->mData.testCond[bd][ch].reserved.reserved_cycleNo
						= 0;
					myData->bData[bd].cData[ch].signal[C_SIG_STOP] = P1;
				} else { //reserved stop
					myData->mData.testCond[bd][ch].reserved.reserved_cmd = 1;
					myData->mData.testCond[bd][ch].reserved.reserved_stepNo
						= (unsigned long)cmd.control.stepNo;
					myData->mData.testCond[bd][ch].reserved.reserved_cycleNo
						= (unsigned long)cmd.control.cycleNo;
				}
				myData->bData[bd].cData[ch].op.reservedCmd
					= myData->mData.testCond[bd][ch].reserved.reserved_cmd;
				myData->mData.testCond[bd][ch].reserved.select_run = 0;
				myData->mData.testCond[bd][ch].reserved.select_stepNo = 0;
				myData->mData.testCond[bd][ch].reserved.select_cycleNo = 0;
				myData->mData.testCond[bd][ch].reserved.select_advCycleStep = 0;
			}
		}
	}
	
	return send_cmd_response((char *)&cmd.header, EP_CD_ACK);
}

int rcv_cmd_dll_pause(void)
{
	int bd, ch=0, i, j;
	unsigned long chFlag, chFlag1;
	S_MAIN_RCV_CMD_PAUSE cmd;

	//kjg_180524 memset((char *)&cmd, 0, sizeof(S_MAIN_RCV_CMD_PAUSE));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_MAIN_RCV_CMD_PAUSE));

	chFlag = 0x00000001;
	for(i=0; i < myData->mData.config.installedCh; i++) {
		chFlag1 = (chFlag << i) & cmd.header.chFlag[i / 32];

		if(chFlag1 != 0) {
			bd = myData->CellArray1[i].bd;
			ch = myData->CellArray1[i].ch;
			
			
			if(myData->bData[bd].cData[ch].ChAttribute.opType == P1) {
				cmd.header.chFlag[i / 32]
					= cmd.header.chFlag[i / 32] | (chFlag << (i + 1));
			}
			if(myData->bData[bd].cData[ch].op.state != C_RUN) {
				return send_cmd_response((char *)&cmd.header,
					EP_CD_CH_STATE_ERROR);
			} else {
				if(cmd.control.stepNo > 0 && cmd.control.cycleNo > 0) {
					j = 1; //reserved
				} else {
					j = 0; //direct
				}
				if(j == 0) { //direct pause
					myData->mData.testCond[bd][ch].reserved.reserved_cmd = 0;
					myData->mData.testCond[bd][ch].reserved.reserved_stepNo = 0;
					myData->mData.testCond[bd][ch].reserved.reserved_cycleNo
						= 0;
					myData->bData[bd].cData[ch].signal[C_SIG_DLL_PAUSE] = P1;
					userlog(DEBUG_LOG, psName,"DLL PAUSE bd[%d]ch[%d]\n",bd,ch);
				} else { //reserved pause
					myData->mData.testCond[bd][ch].reserved.reserved_cmd = 2;
					myData->mData.testCond[bd][ch].reserved.reserved_stepNo
						= (unsigned long)cmd.control.stepNo;
					myData->mData.testCond[bd][ch].reserved.reserved_cycleNo
						= (unsigned long)cmd.control.cycleNo;
				}
				myData->bData[bd].cData[ch].op.reservedCmd
					= myData->mData.testCond[bd][ch].reserved.reserved_cmd;
				myData->mData.testCond[bd][ch].reserved.select_run = 0;
				myData->mData.testCond[bd][ch].reserved.select_stepNo = 0;
				myData->mData.testCond[bd][ch].reserved.select_cycleNo = 0;
				myData->mData.testCond[bd][ch].reserved.select_advCycleStep = 0;
				#ifdef _EXTERNAL_CONTROL
				myData->bData[bd].cData[ch].misc.chPause = P0;
				#endif
			}
		}
	}
	
	return send_cmd_response((char *)&cmd.header, EP_CD_ACK);
}

int rcv_cmd_pause(void)
{
	int bd, ch=0, i, j;
	unsigned long chFlag, chFlag1;
	S_MAIN_RCV_CMD_PAUSE cmd;

	//kjg_180524 memset((char *)&cmd, 0, sizeof(S_MAIN_RCV_CMD_PAUSE));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_MAIN_RCV_CMD_PAUSE));

	chFlag = 0x00000001;
	for(i=0; i < myData->mData.config.installedCh; i++) {
		chFlag1 = (chFlag << i) & cmd.header.chFlag[i / 32];

		if(chFlag1 != 0) {
			bd = myData->CellArray1[i].bd;
			ch = myData->CellArray1[i].ch;
			if(myData->bData[bd].cData[ch].ChAttribute.opType == P1) {
				cmd.header.chFlag[i / 32]
					= cmd.header.chFlag[i / 32] | (chFlag << (i + 1));
			}
			if(myData->bData[bd].cData[ch].op.state != C_RUN) {
				return send_cmd_response((char *)&cmd.header,
					EP_CD_CH_STATE_ERROR);
			} else {
				if(cmd.control.stepNo > 0 && cmd.control.cycleNo > 0) {
					j = 1; //reserved
				} else {
					j = 0; //direct
				}
				if(j == 0) { //direct pause
					myData->mData.testCond[bd][ch].reserved.reserved_cmd = 0;
					myData->mData.testCond[bd][ch].reserved.reserved_stepNo = 0;
					myData->mData.testCond[bd][ch].reserved.reserved_cycleNo
						= 0;
					myData->bData[bd].cData[ch].signal[C_SIG_PAUSE] = P1;
				} else { //reserved pause
					myData->mData.testCond[bd][ch].reserved.reserved_cmd = 2;
					myData->mData.testCond[bd][ch].reserved.reserved_stepNo
						= (unsigned long)cmd.control.stepNo;
					myData->mData.testCond[bd][ch].reserved.reserved_cycleNo
						= (unsigned long)cmd.control.cycleNo;
				}
				myData->bData[bd].cData[ch].op.reservedCmd
					= myData->mData.testCond[bd][ch].reserved.reserved_cmd;
				myData->mData.testCond[bd][ch].reserved.select_run = 0;
				myData->mData.testCond[bd][ch].reserved.select_stepNo = 0;
				myData->mData.testCond[bd][ch].reserved.select_cycleNo = 0;
				myData->mData.testCond[bd][ch].reserved.select_advCycleStep = 0;
				#ifdef _EXTERNAL_CONTROL
				myData->bData[bd].cData[ch].misc.chPause = P1;
				#endif
			}
		}
	}
	
	return send_cmd_response((char *)&cmd.header, EP_CD_ACK);
}

int rcv_cmd_continue(void)
{
	int bd, ch=0, i;
	unsigned long chFlag, chFlag1;
	S_MSG_CH_FLAG ch_flag;
	S_MAIN_RCV_CMD_CONTINUE	cmd;

	//kjg_180524 memset((char *)&cmd, 0, sizeof(S_MAIN_RCV_CMD_CONTINUE));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_MAIN_RCV_CMD_CONTINUE));
	
	chFlag = 0x00000001;
	for(i=0; i < myData->mData.config.installedCh; i++) {
		chFlag1 = (chFlag << i) & cmd.header.chFlag[i / 32];
		
		if(chFlag1 != 0) {
			bd = myData->CellArray1[i].bd;
			ch = myData->CellArray1[i].ch;
			if(myData->bData[bd].cData[ch].ChAttribute.opType == P1) {
				cmd.header.chFlag[i / 32]
					= cmd.header.chFlag[i / 32] | (chFlag << (i + 1));
			}
			if(myData->bData[bd].cData[ch].op.state != C_PAUSE) {
				return send_cmd_response((char *)&cmd.header,
					EP_CD_CH_STATE_ERROR);
			} else {
				myData->mData.testCond[bd][ch].reserved.reserved_cmd = 0;
				myData->mData.testCond[bd][ch].reserved.reserved_stepNo = 0;
				myData->mData.testCond[bd][ch].reserved.reserved_cycleNo = 0;
				myData->bData[bd].cData[ch].op.reservedCmd
					= myData->mData.testCond[bd][ch].reserved.reserved_cmd;
				myData->mData.testCond[bd][ch].reserved.select_run = 0;
				myData->mData.testCond[bd][ch].reserved.select_stepNo = 0;
				myData->mData.testCond[bd][ch].reserved.select_cycleNo = 0;
				myData->mData.testCond[bd][ch].reserved.select_advCycleStep = 0;
				#ifdef _EXTERNAL_CONTROL
				myData->bData[bd].cData[ch].misc.chPause = P0;
				#endif
			}
		}
	}
	
	ch_flag.bit_32[0] = cmd.header.chFlag[0];
	ch_flag.bit_32[1] = cmd.header.chFlag[1];
	send_msg_ch_flag(MAIN_TO_DATASAVE, (char *)&ch_flag);
	send_msg(MAIN_TO_DATASAVE, MSG_MAIN_DATASAVE_RCVED_CONTINUE_CMD, 0, 0);
	
	return send_cmd_response((char *)&cmd.header, EP_CD_ACK);
}

int rcv_cmd_chamber_wait_release(void)
{
	int bd, ch=0, i;
	unsigned long chFlag, chFlag1;
	S_MAIN_RCV_CMD_CHAMBER_WAIT_RELEASE	cmd;

	//kjg_180524 memset((char *)&cmd, 0, sizeof(S_MAIN_RCV_CMD_CONTINUE));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_MAIN_RCV_CMD_CHAMBER_WAIT_RELEASE));
	
	chFlag = 0x00000001;
	for(i=0; i < myData->mData.config.installedCh; i++) {
		chFlag1 = (chFlag << i) & cmd.header.chFlag[i / 32];
		
		if(chFlag1 != 0) {
			bd = myData->CellArray1[i].bd;
			ch = myData->CellArray1[i].ch;
			if(myData->bData[bd].cData[ch].ChAttribute.opType == P1) {
				cmd.header.chFlag[i / 32]
					= cmd.header.chFlag[i / 32] | (chFlag << (i + 1));
			}
			if(myData->bData[bd].cData[ch].op.state == C_PAUSE) {
				myData->bData[bd].cData[ch].misc.chGroupNo = 0;
				myData->bData[bd].cData[ch].misc.stepSyncFlag = 0;
				myData->bData[bd].cData[ch].misc.chamberNoWaitFlag = P1;
			}
		}
	}
	
	return send_cmd_response((char *)&cmd.header, EP_CD_ACK);
}

int rcv_cmd_init(void)
{
	int bd, ch=0, i;
	unsigned char offset=0;
	unsigned long chFlag, chFlag1;
	S_MAIN_RCV_CMD_INIT	cmd;

	//kjg_180524 memset((char *)&cmd, 0, sizeof(S_MAIN_RCV_CMD_INIT));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_MAIN_RCV_CMD_INIT));
	
	chFlag = 0x00000001;
	for(i=0; i < myData->mData.config.installedCh; i++) {
		chFlag1 = (chFlag << i) & cmd.header.chFlag[i / 32];
		offset = rand();
		if(chFlag1 != 0) {
			bd = myData->CellArray1[i].bd;
			ch = myData->CellArray1[i].ch;
			if(myData->bData[bd].cData[ch].ChAttribute.opType == P1) {
				cmd.header.chFlag[i / 32]
					= cmd.header.chFlag[i / 32] | (chFlag << (i + 1));
			}
			myData->bData[bd].cData[ch].op.state = C_IDLE;
			myData->bData[bd].cData[ch].op.phase = P0;
			if(myData->AppControl.config.debugType != P0) {
				myData->bData[bd].cData[ch].misc.simVsens = 3500000 + offset;
			}
		}
	}
	
	return send_cmd_response((char *)&cmd.header, EP_CD_ACK);
}

int rcv_cmd_next_step(void)
{
	int bd, ch=0, i;
	unsigned long chFlag, chFlag1;
	S_MAIN_RCV_CMD_NEXT_STEP cmd;

	//kjg_180524 memset((char *)&cmd, 0, sizeof(S_MAIN_RCV_CMD_NEXT_STEP));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_MAIN_RCV_CMD_NEXT_STEP));
	
	chFlag = 0x00000001;
	for(i=0; i < myData->mData.config.installedCh; i++) {
		chFlag1 = (chFlag << i) & cmd.header.chFlag[i / 32];

		if(chFlag1 != 0) {
			bd = myData->CellArray1[i].bd;
			ch = myData->CellArray1[i].ch;
			if(myData->bData[bd].cData[ch].ChAttribute.opType == P1) {
				cmd.header.chFlag[i / 32]
					= cmd.header.chFlag[i / 32] | (chFlag << (i + 1));
			}
			if(myData->bData[bd].cData[ch].op.state != C_RUN
				&& myData->bData[bd].cData[ch].op.state != C_PAUSE) {
				return send_cmd_response((char *)&cmd.header,
					EP_CD_CH_STATE_ERROR);
			} else {
				myData->mData.testCond[bd][ch].reserved.reserved_cmd = 0;
				myData->mData.testCond[bd][ch].reserved.reserved_stepNo = 0;
				myData->mData.testCond[bd][ch].reserved.reserved_cycleNo = 0;
				myData->bData[bd].cData[ch].op.reservedCmd
					= myData->mData.testCond[bd][ch].reserved.reserved_cmd;
				myData->mData.testCond[bd][ch].reserved.select_run = 0;
				myData->mData.testCond[bd][ch].reserved.select_stepNo = 0;
				myData->mData.testCond[bd][ch].reserved.select_cycleNo = 0;
				myData->mData.testCond[bd][ch].reserved.select_advCycleStep = 0;
				myData->bData[bd].cData[ch].signal[C_SIG_NEXTSTEP] = P1;
			}
		}
	}
	
	return send_cmd_response((char *)&cmd.header, EP_CD_ACK);
}

int rcv_cmd_shutdown(void) //181108
{
	int bd, ch=0;
	S_MAIN_RCV_CMD_SHUTDOWN cmd;

	//kjg_180524 memset((char *)&cmd, 0, sizeof(S_MAIN_RCV_CMD_PAUSE));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_MAIN_RCV_CMD_SHUTDOWN));
		
	for(bd=0; bd < myData->mData.config.installedBd; bd++) {
		for(ch=0; ch < myData->mData.config.chPerBd; ch++) {
			if((myData->mData.config.chPerBd *bd +ch)
							> (myData->mData.config.installedCh-1)) {
					continue;
			}
			if(myData->bData[bd].cData[ch].op.state == C_RUN) {
				myData->bData[bd].cData[ch].signal[C_SIG_SHUTDOWN] = P1;
			}
		}
	}
	
	send_msg(MAIN_TO_MODULE, MSG_MAIN_MODULE_SHUTDOWN_FLAG, 1, 0);
	return send_cmd_response((char *)&cmd.header, EP_CD_ACK);
}

int rcv_cmd_parameter_update(void)
{	//190801 lyhw
	int bd, ch=0;
	unsigned char update_flag = 0;
	S_MAIN_RCV_CMD_PARAMETER_UPDATE cmd;
	
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_MAIN_RCV_CMD_PARAMETER_UPDATE));
	
	for(bd=0; bd < myData->mData.config.installedBd; bd++) {
		for(ch=0; ch < myData->mData.config.chPerBd; ch++) {
			if((myData->mData.config.chPerBd *bd +ch)
							> (myData->mData.config.installedCh-1)) {
					continue;
			}
			if(myData->bData[bd].cData[ch].op.state == C_RUN) {
				update_flag = P1;
			}
		}
	}
	
	if(update_flag == P0){
		myPs->signal[MAIN_SIG_PARAMETER_UPDATE] = P1;
		userlog(DEBUG_LOG, psName, "parameter Update start\n");
		send_msg(MAIN_TO_APP, MSG_MAIN_APP_PARAMETER_UPDATE, 0, 0);
	}else{
		myPs->signal[MAIN_SIG_PARAMETER_UPDATE] = P0;
		userlog(DEBUG_LOG, psName, "parameter Update Fail, Ch Running\n");
	}
	
	return send_cmd_response((char *)&cmd.header, EP_CD_ACK);
}

//150512 oys add start : chData Backup, Restore
int rcv_cmd_ch_data_backup(void)
{
	int bd, ch=0, i;
	unsigned long chFlag, chFlag1;
	S_MAIN_RCV_CMD_CH_DATA_BACKUP cmd;

	//kjg_180524 memset((char *)&cmd, 0, sizeof(S_MAIN_RCV_CMD_CH_DATA_BACKUP));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_MAIN_RCV_CMD_CH_DATA_BACKUP));
	
	chFlag = 0x00000001;
	for(i=0; i < myData->mData.config.installedCh; i++) {
		chFlag1 = (chFlag << i) & cmd.header.chFlag[i / 32];

		if(chFlag1 != 0) {
			bd = myData->CellArray1[i].bd;
			ch = myData->CellArray1[i].ch;
			if(myData->bData[bd].cData[ch].op.state == C_PAUSE) {
				if(myData->bData[bd].cData[ch].ChAttribute.opType == P1) {
					cmd.header.chFlag[i / 32]
						= cmd.header.chFlag[i / 32] | (chFlag << (i + 1));
				}
				myData->AppControl.backup[i] = P1;
				myData->AppControl.backupFlag[i] = P0;
				myData->bData[bd].cData[ch].misc.userDataNo = 0; //20200427 add
			}
		}
	}

	return send_cmd_response((char *)&cmd.header, EP_CD_ACK);
}

int rcv_cmd_ch_data_restore(void)
{
	int bd, ch=0, i;
	unsigned long chFlag, chFlag1;
	S_MAIN_RCV_CMD_CH_DATA_RESTORE cmd;

	//kjg_180524 memset((char *)&cmd, 0, sizeof(S_MAIN_RCV_CMD_CH_DATA_RESTORE));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_MAIN_RCV_CMD_CH_DATA_RESTORE));
	
	chFlag = 0x00000001;
	for(i=0; i < myData->mData.config.installedCh; i++) {
		chFlag1 = (chFlag << i) & cmd.header.chFlag[i / 32];

		if(chFlag1 != 0) {
			bd = myData->CellArray1[i].bd;
			ch = myData->CellArray1[i].ch;
			if(myData->bData[bd].cData[ch].op.state == C_IDLE
				|| myData->bData[bd].cData[ch].op.state == C_STANDBY) {
				if(myData->bData[bd].cData[ch].ChAttribute.opType == P1) {
					cmd.header.chFlag[i / 32]
						= cmd.header.chFlag[i / 32] | (chFlag << (i + 1));
				}
				myData->AppControl.restore[i] = P1;
				myData->AppControl.restoreFlag[i] = P0;
				myData->bData[bd].cData[ch].misc.userDataNo = 0; //20200427 add
			}
		}
	}

	return send_cmd_response((char *)&cmd.header, EP_CD_ACK);
} //150512 oys add end : chData Backup, Restore

//190829 oys add : GUI VERSION INFO
int rcv_cmd_gui_version_info(void)
{
	S_MAIN_RCV_CMD_GUI_VERSION_INFO cmd;

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_MAIN_RCV_CMD_GUI_VERSION_INFO));

	return send_cmd_response((char *)&cmd.header, EP_CD_ACK);
}

int rcv_cmd_reset(void)
{
	int bd, ch=0, i;
	unsigned long chFlag, chFlag1;
	S_MAIN_RCV_CMD_RESET cmd;

	//kjg_180524 memset((char *)&cmd, 0, sizeof(S_MAIN_RCV_CMD_RESET));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_MAIN_RCV_CMD_RESET));
	
	chFlag = 0x00000001;
	for(i=0; i < myData->mData.config.installedCh; i++) {
		chFlag1 = (chFlag << i) & cmd.header.chFlag[i / 32];

		if(chFlag1 != 0) {
			bd = myData->CellArray1[i].bd;
			ch = myData->CellArray1[i].ch;
			if(myData->bData[bd].cData[ch].ChAttribute.opType == P1) {
				cmd.header.chFlag[i / 32]
					= cmd.header.chFlag[i / 32] | (chFlag << (i + 1));
			}
			if(myData->bData[bd].cData[ch].op.state != C_STANDBY) {
				return send_cmd_response((char *)&cmd.header,
					EP_CD_CH_STATE_ERROR);
			} else {
				myData->bData[bd].cData[ch].signal[C_SIG_RESET] = P1;
			}
		}
	}
	
	return send_cmd_response((char *)&cmd.header, EP_CD_ACK);
}

int rcv_cmd_comm_check_reply(void)
{
	char buf[8];
	S_MAIN_RCV_CMD_COMM_CHECK_REPLY comm_check_reply;
	
	//kjg_180524 memset((char *)&comm_check_reply, 0,
	//	sizeof(S_MAIN_RCV_CMD_COMM_CHECK_REPLY));
	memcpy((char *)&comm_check_reply, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_MAIN_RCV_CMD_COMM_CHECK_REPLY));
	
	memset(buf, 0, sizeof buf);
	memcpy((char *)&buf[0], (char *)&comm_check_reply.result[0], 2);
	if(atoi(buf) == ACK) {
	} else {
		userlog(DEBUG_LOG, psName, "rcv_cmd_comm_check_reply result:%s\n", buf);
	}
	
	memset(buf, 0, sizeof buf);
	memcpy((char *)&buf[0], (char *)&comm_check_reply.sended_cmd[0], 4);
	if(strncmp((char *)&buf[0], "0x21", 4) == 0) {
	} else {
		userlog(DEBUG_LOG, psName,
			"rcv_cmd_comm_check_reply sended_cmd:%s\n", buf);
	}

	return 0;
}

int rcv_cmd_comm_check(void)
{
	send_cmd_comm_check_reply();
	return 0;
}

int send_cmd_response(char *rcvHeader, int code)
{
	int cmd_size, body_size, rtn;
	S_MAIN_SEND_CMD_RESPONSE cmd;
	S_MAIN_CMD_HEADER header;

	cmd_size = sizeof(S_MAIN_SEND_CMD_RESPONSE);
	body_size = cmd_size - sizeof(S_MAIN_CMD_HEADER);
	memset((char *)&cmd, 0, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		MAIN_CMD_TO_PC_RESPONSE, SEQNUM_AUTO, body_size);
	
	//kjg_180524 memset((char *)&header, 0, sizeof(S_MAIN_CMD_HEADER));
	memcpy((char *)&header, (char *)rcvHeader, sizeof(S_MAIN_CMD_HEADER));

	cmd.header.reserved1 = header.reserved1;
	cmd.header.reserved2 = header.reserved2;
	cmd.header.chFlag[0] = header.chFlag[0];
	cmd.header.chFlag[1] = header.chFlag[1];
	cmd.response.cmd = (int)header.cmd_id;
	cmd.response.code = code;
	
	rtn = send_command((char *)&cmd, cmd_size, MAIN_CMD_TO_PC_RESPONSE);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d %d\n", rtn,
			MAIN_CMD_TO_PC_RESPONSE);
	}
	return 0;
}

int send_cmd_module_info_reply(void)
{
	int cmd_size, body_size, rtn, i;
	S_MAIN_SEND_CMD_MODULE_INFO_REPLY cmd;
	
	cmd_size = sizeof(S_MAIN_SEND_CMD_MODULE_INFO_REPLY);
	body_size = cmd_size - sizeof(S_MAIN_CMD_HEADER);
	memset((char *)&cmd, 0, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		MAIN_CMD_TO_PC_MODULE_INFO_REPLY, SEQNUM_AUTO, body_size);
	
	cmd.md_info.group_id = (unsigned int)myData->AppControl.config.moduleNo;
	cmd.md_info.systemType = myData->AppControl.config.systemType;
	cmd.md_info.protocol_version = myPs->config.protocol_version;
	memcpy((char *)&cmd.md_info.modelName[0],
		(char *)&myData->AppControl.config.modelName[0], 128);
	cmd.md_info.osVersion = myData->AppControl.config.osVersion;
	cmd.md_info.voltage_range
		= (unsigned short int)myData->mData.config.rangeV;
	cmd.md_info.current_range
		= (unsigned short int)myData->mData.config.rangeI;
	for(i=0; i < MAX_RANGE; i++) {
		cmd.md_info.voltage_spec[i] = myData->mData.config.maxVoltage[i];
		cmd.md_info.current_spec[i] = myData->mData.config.maxCurrent[i];
	}
	cmd.md_info.voltage_spec[4] = 0;
	cmd.md_info.current_spec[4] = 0;

#ifdef _SDI_SAFETY_V2
	cmd.md_info.TempUseUnit = myData->AppControl.loadProcess[3];
#endif

	cmd.md_info.installedBd
		= (unsigned short int)myData->mData.config.installedBd ;
	cmd.md_info.chPerBd = (unsigned short int)myData->mData.config.chPerBd;
	cmd.md_info.installedCh = (unsigned int)myData->mData.config.installedCh;
	cmd.md_info.totalJig = myData->mData.config.totalJig;
	for(i=0; i < 16; i++) {
		cmd.md_info.BdinJig[i] = myData->mData.config.bdInJig[i];
	}

	//kjg_180521 if(myData->mData.config.parallelMode) cmd.md_info.moduleConfig |= 0x01;
	switch(myData->mData.config.parallelMode) {
		case P0:
			break;
		case P1: //kjg_180521
			cmd.md_info.moduleConfig |= 0x01;
			break;
		case P2: //kjg_180521 defined
			break;
		default:
			break;
	}

	if(myData->mData.config.installedTemp) cmd.md_info.moduleConfig |= 0x02;
	if(myData->mData.config.installedAuxV) cmd.md_info.moduleConfig |= 0x04;
	
	//110411 kji program version
//	memcpy((char *)&cmd.md_info.program_version[0], VENDER, sizeof(VENDER));
#if VERSION_DETAIL_SHOW == 1 
	cmd.md_info.program_version[0] = (MAIN_P_VER1/1000) + 48; 
	cmd.md_info.program_version[1] = ((MAIN_P_VER1%1000)/100) + 48;
	cmd.md_info.program_version[2] = (((MAIN_P_VER1%1000)%100)/10) + 48;
	cmd.md_info.program_version[3] = ((((MAIN_P_VER1%1000)%100)%10)/1) + 48;
	cmd.md_info.program_version[4] = '-';
	cmd.md_info.program_version[5] = SUB_P_VER1;
	cmd.md_info.program_version[6] = (SUB_P_VER/10)+48;
	cmd.md_info.program_version[7] = ((SUB_P_VER%10)/1)+48;
	cmd.md_info.program_version[8] = '-'; 
	cmd.md_info.program_version[9] = MAIN_R_VER1;
	cmd.md_info.program_version[10] = (MAIN_R_VER/100)+48;
	cmd.md_info.program_version[11] = ((MAIN_R_VER%100)/10)+48;
	cmd.md_info.program_version[12] = (((MAIN_R_VER%100)%10)/1)+48;
	cmd.md_info.program_version[13] = '-';
	cmd.md_info.program_version[14] = SUB_R_VER1;
	cmd.md_info.program_version[15] = (SUB_R_VER/10)+48;
	cmd.md_info.program_version[16] = ((SUB_R_VER%10)/1)+48;
	cmd.md_info.program_version[17] = 0;
#else
	cmd.md_info.program_version[0] = (VENDER / 10) + 48; 
	cmd.md_info.program_version[1] = (VENDER % 10) + 48;
	cmd.md_info.program_version[2] = '.';
	cmd.md_info.program_version[3] = (PROGRAM_VERSION1) + 48; 
	cmd.md_info.program_version[4] = (PROGRAM_VERSION2) + 48;
	cmd.md_info.program_version[5] = '.';
	cmd.md_info.program_version[6] = (PROGRAM_VERSION3) + 48; 
	cmd.md_info.program_version[7] = (PROGRAM_VERSION4) + 48;
#endif
	rtn = send_command((char *)&cmd, cmd_size,
		MAIN_CMD_TO_PC_MODULE_INFO_REPLY);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d %d\n", rtn,
			MAIN_CMD_TO_PC_MODULE_INFO_REPLY);
	}
	return 0;
}
//hun_200219_s
int send_cmd_hwfault_reply(void)
{
	int cmd_size, body_size, rtn;
	S_MAIN_SEND_CMD_MODULE_HWFAULT_REPLY cmd;
	
	cmd_size = sizeof(S_MAIN_SEND_CMD_MODULE_HWFAULT_REPLY);
	body_size = cmd_size - sizeof(S_MAIN_CMD_HEADER);
	memset((char *)&cmd, 0, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		MAIN_CMD_TO_PC_HWFAULT_REPLY, SEQNUM_AUTO, body_size);
	
	//parameter/HwFault_Config
	cmd.Hwfault.Drop_V_Charge 
		= myData->mData.config.hwFaultConfig[HW_FAULT_DROP_V_1];	
	cmd.Hwfault.Drop_V_DisCharge 
		= myData->mData.config.hwFaultConfig[HW_FAULT_DROP_V_2];	
	
	rtn = send_command((char *)&cmd, cmd_size, MAIN_CMD_TO_PC_HWFAULT_REPLY);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d %d\n", rtn,
			MAIN_CMD_TO_PC_HWFAULT_REPLY);
	}
	return 0;
}
//hun_200219_e
int send_cmd_set_measure_data(int id)
{
	int cmd_size, body_size, rtn;
	S_MAIN_SEND_CMD_SET_MEASURE_DATA cmd;
	
	cmd_size = sizeof(S_MAIN_SEND_CMD_SET_MEASURE_DATA);
	body_size = cmd_size - sizeof(S_MAIN_CMD_HEADER);
	memset((char *)&cmd, 0x00, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		MAIN_CMD_TO_PC_SET_MEASURE_DATA, SEQNUM_AUTO, body_size);
	
	cmd.id = (unsigned short)id;
	switch(cmd.id){
		case PS_TEMPERATURE:
			cmd.type = 0;
			cmd.data = 0;
			break;
	}

	rtn = send_command((char *)&cmd, cmd_size,
		MAIN_CMD_TO_PC_SET_MEASURE_DATA);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d %d\n", rtn,
			MAIN_CMD_TO_PC_SET_MEASURE_DATA);
	}
	return 0;
}

int send_cmd_ch_data(void)
{
	switch(myData->mData.config.parallelMode) {
		case P0:
		case P1:
		case P2: //kjg_180521
			return send_cmd_ch_data2();
			break;
		default:
			return 0;
			break;
	}
}

#ifdef	__LG_VER1__
int send_cmd_ch_data2(void)
{
	unsigned char slave_ch;
	unsigned short advStepNo;
	int cmd_size, body_size, rtn, bd, ch, i, j, idx, count, msg;

	S_MAIN_SEND_CMD_CH_DATA	cmd;
	cmd_size = sizeof(S_MAIN_SEND_CMD_CH_DATA);
	body_size = cmd_size - sizeof(S_MAIN_CMD_HEADER);
	memset((char *)&cmd, 0, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		MAIN_CMD_TO_PC_CH_DATA, SEQNUM_AUTO, body_size);
	
	count = msg = j = 0;
	for(i=0; i < myData->mData.config.installedCh; i++) {
		bd = myData->CellArray1[i].bd;
		ch = myData->CellArray1[i].ch;
		advStepNo = myData->bData[bd].cData[ch].misc.advStepNo;
		if(myData->bData[bd].cData[ch].ChAttribute.opType == P1){
			slave_ch = myData->bData[bd].cData[ch]
						.ChAttribute.chNo_slave[0] - 1 ;  //1base
		}
		if(myData->save_msg[msg].write_idx[bd][ch]
			== myData->save_msg[msg].read_idx[bd][ch]) {
			cmd.chData[i].ch
				= i + 1;
			cmd.chData[i].select
				= SAVE_FLAG_MONITORING_DATA;
			cmd.chData[i].state
				= myData->bData[bd].cData[ch].op.state;
			cmd.chData[i].type
				= myData->bData[bd].cData[ch].op.type;
			cmd.chData[i].mode
				= myData->bData[bd].cData[ch].op.mode;
			cmd.chData[i].code
				= myData->bData[bd].cData[ch].op.code;
			cmd.chData[i].stepNo
				= (unsigned char)myData->bData[bd].cData[ch].op.stepNo;
			cmd.chData[i].Vsens
				= myData->bData[bd].cData[ch].op.Vsens;

			if(myData->bData[bd].cData[ch].ChAttribute.opType == P1) {
				cmd.chData[i].Isens
					= myData->bData[bd].cData[ch].op.Isens
					+ myData->bData[bd].cData[slave_ch].op.Isens;
				cmd.chData[i].watt
					= myData->bData[bd].cData[ch].op.watt
					+ myData->bData[bd].cData[slave_ch].op.watt;
				cmd.chData[i].wattHour
					= myData->bData[bd].cData[ch].op.wattHour
					+ myData->bData[bd].cData[slave_ch].op.wattHour;
				cmd.chData[i].avgI
					= myData->bData[bd].cData[ch].op.meanCurr
					+ myData->bData[bd].cData[slave_ch].op.meanCurr;
				cmd.chData[i].grade
					= myData->bData[bd].cData[slave_ch].op.grade;
				cmd.chData[i].capacity 
					= myData->bData[bd].cData[ch].op.ampareHour
					+ myData->bData[bd].cData[slave_ch].op.ampareHour;
			} else {
				cmd.chData[i].watt
					= myData->bData[bd].cData[ch].op.watt;
				cmd.chData[i].wattHour
					= myData->bData[bd].cData[ch].op.wattHour;
				cmd.chData[i].Isens
					= myData->bData[bd].cData[ch].op.Isens;
				cmd.chData[i].avgI
					= myData->bData[bd].cData[ch].op.meanCurr;
				cmd.chData[i].grade
					= myData->bData[bd].cData[ch].op.grade;
				cmd.chData[i].capacity 
					= myData->bData[bd].cData[ch].op.ampareHour;
			}

			cmd.chData[i].runTime
				= myData->bData[bd].cData[ch].op.runTime;
			cmd.chData[i].totalRunTime
				= myData->bData[bd].cData[ch].op.totalRunTime;	
			cmd.chData[i].z
				= myData->bData[bd].cData[ch].op.z;
			cmd.chData[i].temp
				= myData->bData[bd].cData[ch].op.temp;
			cmd.chData[i].reservedCmd
				= myData->bData[bd].cData[ch].op.reservedCmd;
			cmd.chData[i].totalCycle
				= myData->bData[bd].cData[ch].misc.totalCycle;
			cmd.chData[i].currentCycle
				= myData->bData[bd].cData[ch].misc.currentCycle;
			cmd.chData[i].gotoCycleCount
				= myData->bData[bd].cData[ch].misc.gotoCycleCount[advStepNo];
			cmd.chData[i].avgV
				= myData->bData[bd].cData[ch].op.meanVolt;	
			
#if NETWORK_VERSION >= 4102	
	#if VENDER != 2 //NOT SDI
			if(myData->bData[bd].cData[ch].ChAttribute.opType == P1)
			{
				cmd.chData[i].IntegralAmpareHour
					= myData->bData[bd].cData[ch].op.integral_ampareHour
					+ myData->bData[bd].cData[slave_ch].op.integral_ampareHour;
				cmd.chData[i].IntegralWattHour
					= myData->bData[bd].cData[ch].op.integral_WattHour
					+ myData->bData[bd].cData[slave_ch].op.integral_WattHour;
				cmd.chData[i].ChargeAmpareHour
					= myData->bData[bd].cData[ch].op.charge_ampareHour
					+ myData->bData[bd].cData[slave_ch].op.charge_ampareHour;
				cmd.chData[i].ChargeWattHour
					= myData->bData[bd].cData[ch].op.charge_wattHour
					+ myData->bData[bd].cData[slave_ch].op.charge_wattHour;
				cmd.chData[i].DischargeAmpareHour
					= myData->bData[bd].cData[ch].op.discharge_ampareHour
					+ myData->bData[bd].cData[slave_ch].op.discharge_ampareHour;
				cmd.chData[i].DischargeWattHour
					= myData->bData[bd].cData[ch].op.discharge_wattHour
					+ myData->bData[bd].cData[slave_ch].op.discharge_wattHour;
				if(myData->bData[bd].cData[ch].misc.cvTime > 
					myData->bData[bd].cData[slave_ch].misc.cvTime) {
					cmd.chData[i].cvTime
						= myData->bData[bd].cData[ch].misc.cvTime ;
				} else {
					cmd.chData[i].cvTime
						= myData->bData[bd].cData[slave_ch].misc.cvTime ;
				}
			} else {
				cmd.chData[i].IntegralAmpareHour
					= myData->bData[bd].cData[ch].op.integral_ampareHour;
				cmd.chData[i].IntegralWattHour
					= myData->bData[bd].cData[ch].op.integral_WattHour;
				cmd.chData[i].ChargeAmpareHour
					= myData->bData[bd].cData[ch].op.charge_ampareHour;
				cmd.chData[i].ChargeWattHour
					= myData->bData[bd].cData[ch].op.charge_wattHour;
				cmd.chData[i].DischargeAmpareHour
					= myData->bData[bd].cData[ch].op.discharge_ampareHour;
				cmd.chData[i].DischargeWattHour
					= myData->bData[bd].cData[ch].op.discharge_wattHour;
				cmd.chData[i].cvTime
					= myData->bData[bd].cData[ch].misc.cvTime;
			}
		#if PROGRAM_VERSION1 == 0
			#if PROGRAM_VERSION2 >= 1
			if(myData->bData[bd].cData[ch].ChAttribute.opType == P1) {
				cmd.chData[i].Farad
					= myData->bData[bd].cData[ch].op.capacitance
					+ myData->bData[bd].cData[slave_ch].op.capacitance;
			} else {
				cmd.chData[i].Farad
					= myData->bData[bd].cData[ch].op.capacitance;
			}
			cmd.chData[i].totalRunTime_carry
				= myData->bData[bd].cData[ch].op.totalRunTime_carry;
			cmd.chData[i].cycleNo
				= myData->bData[bd].cData[ch].misc.cycleNo;
			cmd.chData[i].temp1
				= myData->bData[bd].cData[ch].op.temp1;
			#endif
			#if EDLC_TYPE == 1
			//20160229 khk add	start		
			cmd.chData[i].c_t1
				= myData->bData[bd].cData[ch].misc.c_t1;
			cmd.chData[i].c_v1
				= myData->bData[bd].cData[ch].misc.c_v1;
			cmd.chData[i].c_t2
				= myData->bData[bd].cData[ch].misc.c_t2;
			cmd.chData[i].c_v2
				= myData->bData[bd].cData[ch].misc.c_v2;
			// 20160229 khk add end
			// 20180206 sch add start
			cmd.chData[i].capacitance_iec 
				= myData->bData[bd].cData[ch].op.capacitance_iec;
			cmd.chData[i].capacitance_maxwell 
				= myData->bData[bd].cData[ch].op.capacitance_maxwell;
			// 20180206 sch add end
			#endif
			#if	PROGRAM_VERSION2 >= 2
			if(myData->bData[bd].cData[ch].ChAttribute.opType == P1)
			{
				cmd.chData[i].chargeCCAh
					= myData->bData[bd].cData[ch].misc.chargeCCAh
					+ myData->bData[bd].cData[slave_ch].misc.chargeCCAh;
				cmd.chData[i].chargeCVAh
					= myData->bData[bd].cData[ch].misc.chargeCVAh
					+ myData->bData[bd].cData[slave_ch].misc.chargeCVAh;
				cmd.chData[i].dischargeCCAh
					= myData->bData[bd].cData[ch].misc.dischargeCCAh
					+ myData->bData[bd].cData[slave_ch].misc.dischargeCCAh;
				cmd.chData[i].dischargeCVAh
					= myData->bData[bd].cData[ch].misc.dischargeCVAh
					+ myData->bData[bd].cData[slave_ch].misc.dischargeCVAh;
			} else {
				cmd.chData[i].chargeCCAh
					= myData->bData[bd].cData[ch].misc.chargeCCAh;
				cmd.chData[i].chargeCVAh
					= myData->bData[bd].cData[ch].misc.chargeCVAh;
				cmd.chData[i].dischargeCCAh
					= myData->bData[bd].cData[ch].misc.dischargeCCAh;
				cmd.chData[i].dischargeCVAh
					= myData->bData[bd].cData[ch].misc.dischargeCVAh;
			}
			
			#endif
		#endif		
		#if PROGRAM_VERSION1 > 0	// [TOSHIBA] ORIX VERSION
			if(myData->bData[bd].cData[ch].ChAttribute.opType == P1) {
				cmd.chData[i].Farad
					= myData->bData[bd].cData[ch].op.capacitance
					+ myData->bData[bd].cData[slave_ch].op.capacitance;
			} else {
				cmd.chData[i].Farad
					= myData->bData[bd].cData[ch].op.capacitance;
			}
			cmd.chData[i].totalRunTime_carry
				= myData->bData[bd].cData[ch].op.totalRunTime_carry;
			cmd.chData[i].cycleNo
				= myData->bData[bd].cData[ch].misc.cycleNo;
			cmd.chData[i].temp1
				= myData->bData[bd].cData[ch].op.temp1;
			cmd.chData[i].startVoltage
				= myData->bData[bd].cData[ch].misc.startV;
			cmd.chData[i].maxVoltage
				= myData->bData[bd].cData[ch].misc.maxV;
			cmd.chData[i].minVoltage
				= myData->bData[bd].cData[ch].misc.minV;
			cmd.chData[i].startTemp
				= myData->bData[bd].cData[ch].misc.startT;
			cmd.chData[i].maxTemp
				= myData->bData[bd].cData[ch].misc.maxT;
			cmd.chData[i].minTemp
				= myData->bData[bd].cData[ch].misc.minT;	
		#endif
	#endif
	#ifdef _AMBIENT_GAS_FLAG  //211013 hun
		cmd.chData[i].ambientTemp
		 = myData->bData[bd].cData[ch].misc.ambientTemp;
		cmd.chData[i].gasVoltage
		 = myData->bData[bd].cData[ch].misc.gasVoltage;
	#endif
	//131228 oys w : real_time add
	#if REAL_TIME == 1
		cmd.chData[i].realDate
			= myData->mData.real_time[6] * 10000	//year
			+ myData->mData.real_time[5] * 100		//month
			+ myData->mData.real_time[4];			//day
		cmd.chData[i].realClock
			= myData->mData.real_time[3] * 10000000	//hour
			+ myData->mData.real_time[2] * 100000	//min
			+ myData->mData.real_time[1] * 1000		//sec
			+ myData->mData.real_time[0] * 1;		//msec
	#endif
	#ifdef _CH_CHAMBER_DATA 
		cmd.chData[i].Chamber_Temp
			= myData->bData[bd].cData[ch].misc.groupTemp;	
	#endif
	#if VENDER == 1 && CH_AUX_DATA == 1			//190807 pthw	
		for(j = 0; j < MAX_CH_AUX_DATA; j ++){
			cmd.chData[i].ch_AuxTemp[j] 
				= myData->bData[bd].cData[ch].misc.chAuxTemp[j];
			cmd.chData[i].ch_AuxVoltage[j]	
				= myData->bData[bd].cData[ch].misc.chAuxVoltage[j];
		}
	#endif
	#if VENDER == 3 				//20200629 ONLY SK
		cmd.chData[i].Chamber_Temp
			= myData->bData[bd].cData[ch].misc.groupTemp;	
		
	#endif
	#ifdef	_CH_SWELLING_DATA
		for(j = 0; j < MAX_CH_PRESSURE_DATA; j++){
			cmd.chData[i].PressureData[j] 
				= myData->bData[bd].cData[ch].misc.chPressure[j];
		}
		for(j =0; j < MAX_CH_THICKNESS_DATA; j++){
			cmd.chData[i].ThicknessData[j]	
				= myData->bData[bd].cData[ch].misc.chThickness[j];
		}
	#endif
	#if CH_SWELLING_DATA == 1	//210316 NV Use lyhw
		for(j = 0; j < MAX_CH_PRESSURE_DATA; j++){
			cmd.chData[i].PressureData[j] 
				= myData->bData[bd].cData[ch].misc.chPressure[j];
		}
		for(j =0; j < MAX_CH_THICKNESS_DATA; j++){
			cmd.chData[i].ThicknessData[j]	
				= myData->bData[bd].cData[ch].misc.chThickness[j];
		}
	#endif
	//210318 lyhw just Send Data, not Save Data
	#ifdef _GUI_OPCUA_TYPE
		cmd.chData[i].Acc_Capacity 
			= myData->bData[bd].cData[ch].misc.Accumulated_Capacity;
		cmd.chData[i].Acc_WattHour 
			= myData->bData[bd].cData[ch].misc.Accumulated_WattHour;
	#endif
	#ifdef _TRACKING_MODE
		cmd.chData[i].SOC
			= myData->bData[bd].cData[ch].op.SOC;
		cmd.chData[i].RPT_SOC
			= myData->bData[bd].cData[ch].op.ampareHour_SOC;
		cmd.chData[i].SOH		//211022
			= myData->bData[bd].cData[ch].op.SOH;
		cmd.chData[i].RPT_SOH	//211022
			= myData->bData[bd].cData[ch].op.ampareHour_SOH;
		cmd.chData[i].socRefStep //211022
			= myData->bData[bd].cData[ch].misc.socTrackingStep;
		cmd.chData[i].sohRefStep //211022
			= myData->bData[bd].cData[ch].misc.sohTrackingStep;
		cmd.chData[i].chamberNoWaitFlag //211022
			= myData->bData[bd].cData[ch].misc.chamberNoWaitFlag;
	#endif
	#ifdef _EXTERNAL_CONTROL
		cmd.chData[i].chPause
			= myData->bData[bd].cData[ch].misc.chPause;
	#endif
	#if GAS_DATA_CONTROL == 1
		cmd.chData[i].gas_eCo2
			= myData->bData[bd].cData[ch].misc.gas_eCo2;
		cmd.chData[i].gas_Temp
			= myData->bData[bd].cData[ch].misc.gas_Temp;
		cmd.chData[i].gas_AH
			= myData->bData[bd].cData[ch].misc.gas_AH;
		cmd.chData[i].gas_Baseline
			= myData->bData[bd].cData[ch].misc.gas_Baseline;
		cmd.chData[i].gas_TVOC
			= myData->bData[bd].cData[ch].misc.gas_TVOC;
		cmd.chData[i].gas_Ethanol
			= myData->bData[bd].cData[ch].misc.gas_Ethanol;
		cmd.chData[i].gas_H2
			= myData->bData[bd].cData[ch].misc.gas_H2;
	#endif
	#if VENDER == 3 && CH_AUX_DATA == 1			//190807 pthw	
		for(j = 0; j < MAX_CH_AUX_DATA; j ++){
			cmd.chData[i].ch_AuxTemp[j] 
				= myData->bData[bd].cData[ch].misc.chAuxTemp[j];
			cmd.chData[i].ch_AuxVoltage[j]	
				= myData->bData[bd].cData[ch].misc.chAuxVoltage[j];		
		}
	#endif
#endif
		} else {
			myData->save_msg[msg].read_idx[bd][ch]++;
			if(myData->save_msg[msg].read_idx[bd][ch] >= MAX_SAVE_MSG)
				myData->save_msg[msg].read_idx[bd][ch] = 0;
			idx = myData->save_msg[msg].read_idx[bd][ch];

			if(myData->save_msg[msg].count[bd][ch] > 0)
				myData->save_msg[msg].count[bd][ch]--;
			if(myData->save_msg[msg].count[bd][ch] > count) { // 080125
				count = myData->save_msg[msg].count[bd][ch];
			}
			memcpy((char *)&cmd.chData[i],
				(char *)&myData->save_msg[msg].val[idx][bd][ch],
				sizeof(S_SAVE_MSG_VAL));

			cmd.chData[i].ch = i + 1;
		}
	}
	if(myData->DataSave.config.save_data_type != P1){
		rtn = send_command((char *)&cmd, cmd_size, MAIN_CMD_TO_PC_CH_DATA);
		if(rtn < 0) {
			userlog(DEBUG_LOG, psName, "cmd send error!!! %d %d\n", rtn,
				MAIN_CMD_TO_PC_CH_DATA);
		}
	}else{
		if(myData->save_msg[msg].write_idx[bd][ch]
			== myData->save_msg[msg].read_idx[bd][ch]) {
			rtn = send_command((char *)&cmd, cmd_size, MAIN_CMD_TO_PC_CH_DATA);
			if(rtn < 0) {
				userlog(DEBUG_LOG, psName, "cmd send error!!! %d %d\n", rtn,
					MAIN_CMD_TO_PC_CH_DATA);
			}
		}
	}
	return count;
}
#endif

#ifdef __SDI_MES_VER4__
int send_cmd_ch_data2(void)
{
	int cmd_size, body_size, rtn, bd, ch, i, idx, count, msg, stepType;
	unsigned short advStepNo;
	unsigned char slave_ch;
	#ifdef _CH_SWELLING_DATA			//211025 hun
	int j = 0;
	#endif	
	S_MAIN_SEND_CMD_CH_DATA	cmd;
	cmd_size = sizeof(S_MAIN_SEND_CMD_CH_DATA);
	body_size = cmd_size - sizeof(S_MAIN_CMD_HEADER);
	memset((char *)&cmd, 0x00, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		MAIN_CMD_TO_PC_CH_DATA, SEQNUM_AUTO, body_size);
	
	count = 0;
	msg = 0;
	for(i=0; i < myData->mData.config.installedCh; i++) {
		bd = myData->CellArray1[i].bd;
		ch = myData->CellArray1[i].ch;
		
		stepType = myData->bData[bd].cData[ch].op.type;
		advStepNo = myData->bData[bd].cData[ch].misc.advStepNo;
		if(myData->bData[bd].cData[ch].ChAttribute.opType == P1){
			slave_ch = myData->bData[bd].cData[ch]
						.ChAttribute.chNo_slave[0] - 1 ;  //1base
		}

		if(myData->save_msg[msg].write_idx[bd][ch]
			== myData->save_msg[msg].read_idx[bd][ch]) {
			cmd.chData[i].ch
				= i + 1;
			cmd.chData[i].state
				= myData->bData[bd].cData[ch].op.state;
			cmd.chData[i].type
				= myData->bData[bd].cData[ch].op.type;
			cmd.chData[i].mode
				= myData->bData[bd].cData[ch].op.mode;
			cmd.chData[i].select
				= SAVE_FLAG_MONITORING_DATA;
			//cmd.chData[i].grade
			//	= myData->bData[bd].cData[ch].op.grade;
			cmd.chData[i].cvFlag
				= myData->bData[bd].cData[ch].misc.cvFlag; // 210417 LJS
			cmd.chData[i].switchState[0]
				= 0;
			cmd.chData[i].switchState[1]
				= 0;
			cmd.chData[i].code
				= (short int)myData->bData[bd].cData[ch].op.code;
			cmd.chData[i].stepNo
				= (short int)myData->bData[bd].cData[ch].op.stepNo;
			cmd.chData[i].Vsens
				= myData->bData[bd].cData[ch].op.Vsens;
			
			if(myData->bData[bd].cData[ch].ChAttribute.opType == P1){
				cmd.chData[i].Isens
					= myData->bData[bd].cData[ch].op.Isens
					+ myData->bData[bd].cData[slave_ch].op.Isens;
				cmd.chData[i].avgI
					= myData->bData[bd].cData[ch].op.meanCurr
					+ myData->bData[bd].cData[slave_ch].op.meanCurr;
			} else {
				cmd.chData[i].Isens
					= myData->bData[bd].cData[ch].op.Isens;
				cmd.chData[i].avgI
					= myData->bData[bd].cData[ch].op.meanCurr;
			}

			if(myData->bData[bd].cData[ch].op.runTime >= ONE_DAY_RUNTIME){
				cmd.chData[i].runTime_day
					= myData->bData[bd].cData[ch].op.runTime
					/ ONE_DAY_RUNTIME;
				cmd.chData[i].runTime
					= myData->bData[bd].cData[ch].op.runTime
					% ONE_DAY_RUNTIME;
			}else{
				cmd.chData[i].runTime_day
					= 0;
				cmd.chData[i].runTime
					= myData->bData[bd].cData[ch].op.runTime;
			}
			if(myData->bData[bd].cData[ch].op.totalRunTime >= ONE_DAY_RUNTIME){
				cmd.chData[i].totalRunTime_carry
					= myData->bData[bd].cData[ch].op.totalRunTime
					/ ONE_DAY_RUNTIME;	
				cmd.chData[i].totalRunTime
					= myData->bData[bd].cData[ch].op.totalRunTime
					% ONE_DAY_RUNTIME;
			}else{
				cmd.chData[i].totalRunTime_carry
					= 0;	
				cmd.chData[i].totalRunTime
					= myData->bData[bd].cData[ch].op.totalRunTime;
			}

			cmd.chData[i].z
				= myData->bData[bd].cData[ch].op.z;
			cmd.chData[i].temp[0][0]
				= myData->bData[bd].cData[ch].op.temp;
			cmd.chData[i].temp[0][1]
				= myData->bData[bd].cData[ch].misc.minT;
			cmd.chData[i].temp[0][2]
				= myData->bData[bd].cData[ch].misc.maxT;
			cmd.chData[i].temp[0][3]
				= myData->bData[bd].cData[ch].op.meanTemp;
			cmd.chData[i].chamber_control
				= 0;
			cmd.chData[i].record_index
				= 0;
			cmd.chData[i].input_val
				= 0;
			cmd.chData[i].output_val
				= 0;
			cmd.chData[i].reservedCmd
				= myData->bData[bd].cData[ch].op.reservedCmd;
			cmd.chData[i].virRangeReservedNo
				= 0;
			cmd.chData[i].gotoCycleCount
				= myData->bData[bd].cData[ch].misc.gotoCycleCount[advStepNo];
			cmd.chData[i].totalCycle
				= myData->bData[bd].cData[ch].misc.totalCycle;
			cmd.chData[i].currentCycle
				= myData->bData[bd].cData[ch].misc.currentCycle;
			cmd.chData[i].avgV
				= myData->bData[bd].cData[ch].op.meanVolt;
			
			if(myData->bData[bd].cData[ch].ChAttribute.opType == P1)
			{
				cmd.chData[i].IntegralAmpareHour
					= myData->bData[bd].cData[ch].op.integral_ampareHour
					+ myData->bData[bd].cData[slave_ch].op.integral_ampareHour;
				cmd.chData[i].IntegralWattHour
					= myData->bData[bd].cData[ch].op.integral_WattHour
					+ myData->bData[bd].cData[slave_ch].op.integral_WattHour;
				cmd.chData[i].ChargeAmpareHour
					= myData->bData[bd].cData[ch].op.charge_ampareHour
					+ myData->bData[bd].cData[slave_ch].op.charge_ampareHour;
				cmd.chData[i].ChargeWattHour
					= myData->bData[bd].cData[ch].op.charge_wattHour
					+ myData->bData[bd].cData[slave_ch].op.charge_wattHour;
				cmd.chData[i].DischargeAmpareHour
					= myData->bData[bd].cData[ch].op.discharge_ampareHour
					+ myData->bData[bd].cData[slave_ch].op.discharge_ampareHour;
				cmd.chData[i].DischargeWattHour
					= myData->bData[bd].cData[ch].op.discharge_wattHour
					+ myData->bData[bd].cData[slave_ch].op.discharge_wattHour;
				if(stepType == STEP_CHARGE){
					cmd.chData[i].ChargeWatt
						= myData->bData[bd].cData[ch].op.watt;
						+ myData->bData[bd].cData[slave_ch].op.watt;
					cmd.chData[i].DischargeWatt
						= 0;
				}else if(stepType == STEP_DISCHARGE){
					cmd.chData[i].DischargeWatt
						= myData->bData[bd].cData[ch].op.watt;
						+ myData->bData[bd].cData[slave_ch].op.watt;
					cmd.chData[i].ChargeWatt
						= 0;
				}else{
					cmd.chData[i].ChargeWatt
						= 0;
					cmd.chData[i].DischargeWatt
						= 0;
				}
				cmd.chData[i].charge_cc_ampare_hour
					= myData->bData[bd].cData[ch].misc.chargeCCAh;
					+ myData->bData[bd].cData[slave_ch].misc.chargeCCAh;
				cmd.chData[i].charge_cv_ampare_hour
					= myData->bData[bd].cData[ch].misc.chargeCVAh;
					+ myData->bData[bd].cData[slave_ch].misc.chargeCVAh;
				cmd.chData[i].discharge_cc_ampare_hour
					= myData->bData[bd].cData[ch].misc.dischargeCCAh;
					+ myData->bData[bd].cData[slave_ch].misc.dischargeCCAh;
				cmd.chData[i].discharge_cv_ampare_hour
					= myData->bData[bd].cData[ch].misc.dischargeCVAh;
					+ myData->bData[bd].cData[slave_ch].misc.dischargeCVAh;

				if(myData->bData[bd].cData[ch].misc.cvTime > 
					myData->bData[bd].cData[slave_ch].misc.cvTime) {
					cmd.chData[i].cvTime
						= myData->bData[bd].cData[ch].misc.cvTime ;
				} else {
					cmd.chData[i].cvTime
						= myData->bData[bd].cData[slave_ch].misc.cvTime ;
				}
				if(myData->bData[bd].cData[ch].misc.ccTime > 
					myData->bData[bd].cData[slave_ch].misc.ccTime) {
					cmd.chData[i].ccTime
						= myData->bData[bd].cData[ch].misc.ccTime ;
				} else {
					cmd.chData[i].ccTime
						= myData->bData[bd].cData[slave_ch].misc.ccTime ;
				}
			} else {
				cmd.chData[i].IntegralAmpareHour
					= myData->bData[bd].cData[ch].op.integral_ampareHour;
				cmd.chData[i].IntegralWattHour
					= myData->bData[bd].cData[ch].op.integral_WattHour;
				cmd.chData[i].ChargeAmpareHour
					= myData->bData[bd].cData[ch].op.charge_ampareHour;
				cmd.chData[i].ChargeWattHour
					= myData->bData[bd].cData[ch].op.charge_wattHour;
				cmd.chData[i].DischargeAmpareHour
					= myData->bData[bd].cData[ch].op.discharge_ampareHour;
				cmd.chData[i].DischargeWattHour
					= myData->bData[bd].cData[ch].op.discharge_wattHour;
				if(stepType == STEP_CHARGE){
					cmd.chData[i].ChargeWatt 
						= myData->bData[bd].cData[ch].op.watt;
					cmd.chData[i].DischargeWatt
						= 0;
				}else if(stepType == STEP_DISCHARGE){
					cmd.chData[i].DischargeWatt 
						= myData->bData[bd].cData[ch].op.watt;
					cmd.chData[i].ChargeWatt
						= 0;
				}else{
					cmd.chData[i].ChargeWatt
						= 0;
					cmd.chData[i].DischargeWatt
						= 0;
				}
				cmd.chData[i].charge_cc_ampare_hour
					= myData->bData[bd].cData[ch].misc.chargeCCAh;
				cmd.chData[i].charge_cv_ampare_hour
					= myData->bData[bd].cData[ch].misc.chargeCVAh;
				cmd.chData[i].discharge_cc_ampare_hour
					= myData->bData[bd].cData[ch].misc.dischargeCCAh;
				cmd.chData[i].discharge_cv_ampare_hour
					= myData->bData[bd].cData[ch].misc.dischargeCVAh;
				
				if(myData->bData[bd].cData[ch].misc.cvTime >= ONE_DAY_RUNTIME){
					cmd.chData[i].cvTime_day
						= myData->bData[bd].cData[ch].misc.cvTime
						/ ONE_DAY_RUNTIME;
					cmd.chData[i].cvTime
						= myData->bData[bd].cData[ch].misc.cvTime
						% ONE_DAY_RUNTIME;
				}else{
					cmd.chData[i].cvTime_day
						= 0;
					cmd.chData[i].cvTime
						= myData->bData[bd].cData[ch].misc.cvTime;
				}
				if(myData->bData[bd].cData[ch].misc.ccTime >= ONE_DAY_RUNTIME){
					cmd.chData[i].ccTime_day
						= myData->bData[bd].cData[ch].misc.ccTime
						/ ONE_DAY_RUNTIME;
					cmd.chData[i].ccTime
						= myData->bData[bd].cData[ch].misc.ccTime
						% ONE_DAY_RUNTIME;
				}else{
					cmd.chData[i].ccTime_day
						= 0;
					cmd.chData[i].ccTime
						= myData->bData[bd].cData[ch].misc.ccTime;
				}
			}
			cmd.chData[i].startVoltage
				= myData->bData[bd].cData[ch].misc.startV;
			cmd.chData[i].step_count
				= myData->bData[bd].cData[ch].misc.step_count;
			cmd.chData[i].maxVoltage
				= myData->bData[bd].cData[ch].misc.maxV;
			cmd.chData[i].minVoltage
				= myData->bData[bd].cData[ch].misc.minV;
		//131228 oys w : real_time add
		#if REAL_TIME == 1
			cmd.chData[i].realDate
				= myData->mData.real_time[6] * 10000	//year
				+ myData->mData.real_time[5] * 100		//month
				+ myData->mData.real_time[4];			//day
			cmd.chData[i].realClock
				= myData->mData.real_time[3] * 10000000	//hour
				+ myData->mData.real_time[2] * 100000	//min
				+ myData->mData.real_time[1] * 1000		//sec
				+ myData->mData.real_time[0] * 1;		//msec
		#endif
		#ifdef _CH_CHAMBER_DATA
			cmd.chData[i].Chamber_Temp
				= myData->bData[bd].cData[ch].misc.groupTemp;	
		#endif
		#ifdef _CH_SWELLING_DATA			//211025 hun
			for(j = 0; j < MAX_CH_PRESSURE_DATA; j++){
				cmd.chData[i].PressureData[j] 
					= myData->bData[bd].cData[ch].misc.chPressure[j];
			}
			for(j =0; j < MAX_CH_THICKNESS_DATA; j++){
				cmd.chData[i].ThicknessData[j]	
					= myData->bData[bd].cData[ch].misc.chThickness[j];
			}
		#endif
			cmd.chData[i].Vinput = 0;
			cmd.chData[i].Vpower = 0;	
			cmd.chData[i].Vbus = 0;
		} else {
			myData->save_msg[msg].read_idx[bd][ch]++;
			if(myData->save_msg[msg].read_idx[bd][ch] >= MAX_SAVE_MSG)
				myData->save_msg[msg].read_idx[bd][ch] = 0;
			idx = myData->save_msg[msg].read_idx[bd][ch];

			if(myData->save_msg[msg].count[bd][ch] > 0)
				myData->save_msg[msg].count[bd][ch]--;
			if(myData->save_msg[msg].count[bd][ch] > count) { // 080125
				count = myData->save_msg[msg].count[bd][ch];
			}
			memcpy((char *)&cmd.chData[i],
				(char *)&myData->save_msg[msg].val[idx][bd][ch],
				sizeof(S_SAVE_MSG_VAL));

			cmd.chData[i].ch = i + 1;
		}
	}

	if(myData->DataSave.config.save_data_type != P1){
		rtn = send_command((char *)&cmd, cmd_size, MAIN_CMD_TO_PC_CH_DATA);
		if(rtn < 0) {
			userlog(DEBUG_LOG, psName, "cmd send error!!! %d %d\n", rtn,
				MAIN_CMD_TO_PC_CH_DATA);
		}
	}else{
		if(myData->save_msg[msg].write_idx[bd][ch]
			== myData->save_msg[msg].read_idx[bd][ch]) {
			rtn = send_command((char *)&cmd, cmd_size, MAIN_CMD_TO_PC_CH_DATA);
			if(rtn < 0) {
				userlog(DEBUG_LOG, psName, "cmd send error!!! %d %d\n", rtn,
					MAIN_CMD_TO_PC_CH_DATA);
			}
		}
	}
	return count;
}
#endif

int send_cmd_sensor_data(void)
{
	int cmd_size, body_size, rtn, i, map;
	int auxStartCh, auxStartCh2;
	S_MAIN_SEND_CMD_SENSOR_DATA	cmd;
	
	cmd_size = sizeof(S_MAIN_CMD_HEADER);
	auxStartCh = myData->AnalogMeter.config.auxStartCh; //140514 oys add
	auxStartCh2 = myData->AnalogMeter2.config.auxStartCh; //170619 sch add
	
	if(myPs->config.protocol_version > 4102) 
	{
		body_size = sizeof(long)*MAX_AUX_DATA;
	} else{
		if(myData->mData.config.installedTemp > 0)
			body_size = sizeof(long)*myData->mData.config.installedTemp;
		if(myData->mData.config.installedAuxV > 0)
			body_size = sizeof(long)*myData->mData.config.installedAuxV;
	}
	memset((char *)&cmd, 0x00, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		MAIN_CMD_TO_PC_AUX_CH_DATA, SEQNUM_AUTO, body_size);
	if(myPs->config.protocol_version > 4102) 
	{
		if(myData->mData.config.installedTemp > 0)
		{
			for(i=0; i < myData->mData.config.installedTemp; i++) {
				if(myData->AppControl.config.debugType != P0) {
			//		myData->AnalogMeter.temp[i+auxStartCh].temp = i * 1000;
			//		if(i > 99){
			//			myData->AnalogMeter2.temp[i-100+auxStartCh2].temp = i * 1000;
			//		}
				}
				//20170619 sch modify
				if(i < 100){
					cmd.auxTemp[i]
					= myData->AnalogMeter.temp[i+auxStartCh].temp;//khk test
				}else{
					cmd.auxTemp[i]
					= myData->AnalogMeter2.temp[i-100+auxStartCh2].temp;
				}
			}
		}
#if NETWORK_VERSION > 4102	
		if(myData->mData.config.installedAuxV > 0)
		{
			for(i=0; i < myData->mData.config.installedAuxV; i++) {
				map = myData->daq.misc.map[i];
				cmd.auxVoltage[i]
					= myData->daq.op.ch_vsens[map];
			}
		}
#endif
	} else{
		if(myData->mData.config.installedTemp > 0)
		{
			for(i=0; i < myData->mData.config.installedTemp; i++) {
				if(myData->AppControl.config.debugType != P0) {
					myData->AnalogMeter.temp[i+auxStartCh].temp = i * 1000;
					if(i > 99){
						myData->AnalogMeter2.temp[i-100+auxStartCh2].temp = i * 1000;
					}
				}
				//20170619 sch modify
				if(i < 100){
					cmd.auxTemp[i]
					= myData->AnalogMeter.temp[i+auxStartCh].temp;//khk test
				}else{
					cmd.auxTemp[i]
					= myData->AnalogMeter2.temp[i-100+auxStartCh2].temp;
				}
			}
		} 
		if(myData->mData.config.installedAuxV > 0)
		{
			for(i=0; i < myData->mData.config.installedAuxV; i++) {
				map = myData->daq.misc.map[i];
				cmd.auxTemp[i]
					= myData->daq.op.ch_vsens[map];
			}
		}
	}
	rtn = send_command((char *)&cmd, cmd_size+body_size, MAIN_CMD_TO_PC_AUX_CH_DATA);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send sensor data error!!! %d %d\n", rtn,
			MAIN_CMD_TO_PC_AUX_CH_DATA);
	}

	return rtn;
}

int send_cmd_ch_pulse_data(int i)
{
	int bd, ch, idx, count, msg, msgCount;
	
	count = 0;
	msg = 0;

	bd = myData->CellArray1[i].bd;
	ch = myData->CellArray1[i].ch;

	if(myData->pulse_msg[msg][bd][ch].write_idx
		== myData->pulse_msg[msg][bd][ch].read_idx) {
		return count;
	}

	myData->pulse_msg[msg][bd][ch].read_idx++;
	if(myData->pulse_msg[msg][bd][ch].read_idx >= MAX_PULSE_MSG)
		myData->pulse_msg[msg][bd][ch].read_idx = 0;
	idx = myData->pulse_msg[msg][bd][ch].read_idx;
	count = myData->pulse_msg[msg][bd][ch].count--;

	if(myData->pulse_msg[msg][bd][ch].val[idx].type == 0) {
		memset((char *)&myPs->chPulseData[i], 0, sizeof(S_MAIN_CH_PULSE_DATA));
		msgCount = myPs->chPulseData[i].dataCount++;
		myPs->chPulseData[i].totalCycle
			= myData->pulse_msg[msg][bd][ch].val[idx].totalCycle;
		myPs->chPulseData[i].stepNo
			= myData->pulse_msg[msg][bd][ch].val[idx].stepNo;
		memcpy((char *)&myPs->chPulseData[i].val[msgCount],
			(char *)&myData->pulse_msg[msg][bd][ch].val[idx],
			sizeof(S_PULSE_MSG_VAL));
				printf(" send pulse data0 : %d\n", msgCount);
	} else if(myData->pulse_msg[msg][bd][ch].val[idx].type == 1) {
		if(myPs->chPulseData[i].stepNo
			== myData->pulse_msg[msg][bd][ch].val[idx].stepNo) {
			msgCount = myPs->chPulseData[i].dataCount++;
			memcpy((char *)&myPs->chPulseData[i].val[msgCount],
				(char *)&myData->pulse_msg[msg][bd][ch].val[idx],
				sizeof(S_PULSE_MSG_VAL));
			if(msgCount == 100) {
				send_cmd_ch_pulse_data_2(i, msgCount);
				myPs->chPulseData[i].dataCount = 0;
				printf(" send pulse data1 : %d\n", msgCount);
			}
		} else {
			msgCount = myPs->chPulseData[i].dataCount;
			if(msgCount > 0) {
				send_cmd_ch_pulse_data_2(i, msgCount);
				printf(" send pulse data2 : %d\n", msgCount);
			}

			memset((char *)&myPs->chPulseData[i], 0,
				sizeof(S_MAIN_CH_PULSE_DATA));
			msgCount = myPs->chPulseData[i].dataCount++;
			myPs->chPulseData[i].totalCycle
				= myData->pulse_msg[msg][bd][ch].val[idx].totalCycle;
			myPs->chPulseData[i].stepNo
				= myData->pulse_msg[msg][bd][ch].val[idx].stepNo;
			memcpy((char *)&myPs->chPulseData[i].val[msgCount],
				(char *)&myData->pulse_msg[msg][bd][ch].val[idx],
				sizeof(S_PULSE_MSG_VAL));
		}
	} else {
		msgCount = myPs->chPulseData[i].dataCount++;
		memcpy((char *)&myPs->chPulseData[i].val[msgCount],
			(char *)&myData->pulse_msg[msg][bd][ch].val[idx],
			sizeof(S_PULSE_MSG_VAL));
		send_cmd_ch_pulse_data_2(i, msgCount);
		printf(" send pulse data3 : %d\n", msgCount);
		memset((char *)&myPs->chPulseData[i], 0, sizeof(S_MAIN_CH_PULSE_DATA));
	}
	return count;
}

void send_cmd_ch_pulse_data_2_10mS(int i, int msgCount, int msg)
{	//180710 add
	int ch, cmd_size, body_size, idx, rtn, bd =0;
	S_MAIN_SEND_CMD_CH_10MS_DATA cmd;

	cmd_size = sizeof(S_MAIN_CMD_HEADER) + sizeof(long)
				+ sizeof(S_MAIN_CH_DATA) * msgCount;
	body_size = cmd_size - sizeof(S_MAIN_CMD_HEADER);
	make_header_2((char *)&cmd, REPLY_NO,
				MAIN_CMD_TO_PC_CH_10MS_DATA, SEQNUM_AUTO, body_size, i);

	cmd.dataCount = msgCount;
	ch = i;
	for(idx = 0; idx < msgCount; idx++){
		memcpy((char *)&cmd.chData[idx],
				(char *)&myData->pulse_msg[msg][bd][ch].chData[idx],
				sizeof(S_MAIN_CH_DATA));
		memset((char *)&myData->pulse_msg[msg][bd][ch].chData[idx],
					0x00, sizeof(S_MAIN_CH_DATA)); 
	}

	rtn = send_command((char *)&cmd, cmd_size, 
						MAIN_CMD_TO_PC_CH_10MS_DATA);
	if(rtn < 0){
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d %d\n", rtn,
					MAIN_CMD_TO_PC_CH_10MS_DATA);
	}
}

int send_cmd_ch_pulse_data_10mS(int i)
{	//180710 add
	int bd, ch, count = 0, msg, msgCount = 0;

	bd = myData->CellArray1[i].bd;
	ch = myData->CellArray1[i].ch;

	if(myData->pulse_msg[1][bd][ch].flag == 1){
		msg = 1;
		msgCount = myData->pulse_msg[msg][bd][ch].write_idx;
	}else if(myData->pulse_msg[0][bd][ch].flag == 2){
		msg = 0;
		msgCount = myData->pulse_msg[msg][bd][ch].write_idx;
	}else{
		return count;
	}

	send_cmd_ch_pulse_data_2_10mS(i, msgCount, msg);

	if(msg != 1 && myData->pulse_msg[0][bd][ch].flag == 2){
		myData->pulse_msg[0][bd][ch].flag = 0;
		myData->pulse_msg[0][bd][ch].write_idx = 0;
	}
	myData->pulse_msg[msg][bd][ch].write_idx = 0;
	myData->pulse_msg[1][bd][ch].flag = 0;

	return count;
}



int send_cmd_ch_pulse_data_iec(int i)
{
	int bd, ch, idx, count, msg, msgCount;
	
	count = 0;
	msg = 2;

	bd = myData->CellArray1[i].bd;
	ch = myData->CellArray1[i].ch;

	if(myData->pulse_msg_iec[msg][bd][ch].write_idx
		== myData->pulse_msg_iec[msg][bd][ch].read_idx) {
		return count;
	}
//	userlog(DEBUG_LOG, psName, "180103 pulse_data_iec!!!\n");

	myData->pulse_msg_iec[msg][bd][ch].read_idx++;
	if(myData->pulse_msg_iec[msg][bd][ch].read_idx > MAX_PULSE_DATA_IEC)
		myData->pulse_msg_iec[msg][bd][ch].read_idx = 0;
	idx = myData->pulse_msg_iec[msg][bd][ch].read_idx;
	count = myData->pulse_msg_iec[msg][bd][ch].count--;

	if(myData->pulse_msg_iec[msg][bd][ch].val[idx].type == 0) {
		if(myPs->chPulseData_iec[i].stepNo
			== myData->pulse_msg_iec[msg][bd][ch].val[idx].stepNo) {
			msgCount = myPs->chPulseData_iec[i].dataCount++;
			//20180124 sch add for IEC
			myPs->chPulseData_iec[i].totalCycle
				= myData->pulse_msg_iec[msg][bd][ch].val[idx].totalCycle;
			myPs->chPulseData_iec[i].stepNo
				= myData->pulse_msg_iec[msg][bd][ch].val[idx].stepNo;
			
			memcpy((char *)&myPs->chPulseData_iec[i].val[msgCount],
				(char *)&myData->pulse_msg_iec[msg][bd][ch].val[idx],
				sizeof(S_PULSE_MSG_VAL));
			if(msgCount == 151) {
				send_cmd_ch_pulse_data_2_iec(i, msgCount);
				myPs->chPulseData_iec[i].dataCount = 0;
//				printf(" send pulse IEC data1 : %d\n", msgCount);
			}
		} else {  //180112
			msgCount = myPs->chPulseData_iec[i].dataCount;
			if(msgCount > 0) {
				send_cmd_ch_pulse_data_2_iec(i, msgCount);
//				printf(" send pulse IEC data2 : %d\n", msgCount);
			}

			memset((char *)&myPs->chPulseData_iec[i], 0,
				sizeof(S_MAIN_CH_PULSE_DATA));
			msgCount = myPs->chPulseData_iec[i].dataCount++;
			myPs->chPulseData_iec[i].totalCycle
				= myData->pulse_msg_iec[msg][bd][ch].val[idx].totalCycle;
			myPs->chPulseData_iec[i].stepNo
				= myData->pulse_msg_iec[msg][bd][ch].val[idx].stepNo;
			memcpy((char *)&myPs->chPulseData_iec[i].val[msgCount],
				(char *)&myData->pulse_msg_iec[msg][bd][ch].val[idx],
				sizeof(S_PULSE_MSG_VAL));
		}
	}
	return count;
}


void send_cmd_ch_pulse_data_2(int i, int msgCount)
{
	int cmd_size, body_size, cnt, rtn;
	S_MAIN_SEND_CMD_CH_PULSE_DATA	cmd;

	cmd_size = sizeof(S_MAIN_CMD_HEADER)
		+ sizeof(long) * 3
		+ sizeof(S_MAIN_CH_PULSE_VAL) * msgCount;
	body_size = cmd_size - sizeof(S_MAIN_CMD_HEADER);
	memset((char *)&cmd, 0, sizeof(S_MAIN_SEND_CMD_CH_PULSE_DATA));
	make_header_2((char *)&cmd, REPLY_NO,
		MAIN_CMD_TO_PC_CH_PULSE_DATA, SEQNUM_AUTO, body_size, i);
	
	cmd.totalCycle = myPs->chPulseData[i].totalCycle;
	cmd.stepNo = myPs->chPulseData[i].stepNo;
	cmd.dataCount = msgCount;

	for(cnt=0; cnt < msgCount; cnt++) {
		cmd.val[cnt].runTime = myPs->chPulseData[i].val[cnt].runTime;
		cmd.val[cnt].Vsens = myPs->chPulseData[i].val[cnt].Vsens;
		cmd.val[cnt].Isens = myPs->chPulseData[i].val[cnt].Isens;
		cmd.val[cnt].capacity = myPs->chPulseData[i].val[cnt].capacity;
		cmd.val[cnt].wattHour = myPs->chPulseData[i].val[cnt].wattHour;
	}

	rtn = send_command((char *)&cmd, cmd_size, MAIN_CMD_TO_PC_CH_PULSE_DATA);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d %d\n", rtn,
			MAIN_CMD_TO_PC_CH_PULSE_DATA);
	}
}

void send_cmd_ch_pulse_data_2_iec(int i, int msgCount)
{
	int bd, ch, cmd_size, body_size, cnt, rtn;
	S_MAIN_SEND_CMD_CH_PULSE_IEC_DATA	cmd;

	bd = myData->CellArray1[i].bd;
	ch = myData->CellArray1[i].ch;
	cmd_size = sizeof(S_MAIN_CMD_HEADER)
		+ sizeof(long) * 3
		+ sizeof(S_MAIN_CH_PULSE_VAL) * msgCount;
	body_size = cmd_size - sizeof(S_MAIN_CMD_HEADER);
	memset((char *)&cmd, 0, sizeof(S_MAIN_SEND_CMD_CH_PULSE_IEC_DATA));
	make_header_2((char *)&cmd, REPLY_NO,
		MAIN_CMD_TO_PC_CH_PULSE_IEC_DATA, SEQNUM_AUTO, body_size, i);
	
	cmd.totalCycle = myPs->chPulseData_iec[i].totalCycle;
	cmd.stepNo = myPs->chPulseData_iec[i].stepNo;
	cmd.dataCount = msgCount;

	for(cnt=0; cnt < msgCount; cnt++) {
		cmd.val[cnt].runTime = myPs->chPulseData_iec[i].val[cnt].runTime;
		cmd.val[cnt].Vsens = myPs->chPulseData_iec[i].val[cnt].Vsens;
		cmd.val[cnt].Isens = myPs->chPulseData_iec[i].val[cnt].Isens;
		cmd.val[cnt].capacity = myPs->chPulseData_iec[i].val[cnt].capacity;
		cmd.val[cnt].wattHour = myPs->chPulseData_iec[i].val[cnt].wattHour;
//		printf("RunTime%d : %ld",cnt,myPs->chPulseData_iec[0].val[cnt].runTime);
//		printf(" Vsens1 : %ld", myPs->chPulseData_iec[0].val[cnt].Vsens);
//		printf(" Isens1 : %ld", myPs->chPulseData_iec[0].val[cnt].Isens);
//		printf(" ca : %ld", myPs->chPulseData_iec[0].val[cnt].capacity);
//		printf(" wat : %ld\n", myPs->chPulseData_iec[0].val[cnt].wattHour);
//		printf(" Cycle : %ld\n", cmd.totalCycle);
	}

	rtn = send_command((char *)&cmd, cmd_size, 
					MAIN_CMD_TO_PC_CH_PULSE_IEC_DATA);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d %d\n", rtn,
			MAIN_CMD_TO_PC_CH_PULSE_IEC_DATA);
	}
}

int send_cmd_fadm_pulse_data(int i)
{
	int bd, ch, idx, count, msg, msgCount;
	
	count = 0;
	msg = 2;

	bd = myData->CellArray1[i].bd;
	ch = myData->CellArray1[i].ch;

	if(myData->pulse_msg[msg][bd][ch].write_idx
		== myData->pulse_msg[msg][bd][ch].read_idx) {
		return count;
	}
//	userlog(DEBUG_LOG, psName, "180103 send ch fadm pulse_data_2!!!\n");
	
	myData->pulse_msg[msg][bd][ch].read_idx++;
	if(myData->pulse_msg[msg][bd][ch].read_idx >= MAX_PULSE_MSG)
		myData->pulse_msg[msg][bd][ch].read_idx = 0;

	idx = myData->pulse_msg[msg][bd][ch].read_idx;

	if(myData->pulse_msg[msg][bd][ch].count > 0)
		myData->pulse_msg[msg][bd][ch].count--;
	count = myData->pulse_msg[msg][bd][ch].count;

	if(myData->pulse_msg[msg][bd][ch].val[idx].type == 0) {
		memset((char *)&myPs->fadmPulseData[i], 0,
			sizeof(S_MAIN_CH_PULSE_DATA));
		myPs->fadmPulseData[i].dataCount++;
		msgCount = myPs->fadmPulseData[i].dataCount;
		myPs->fadmPulseData[i].totalCycle
			= myData->pulse_msg[msg][bd][ch].val[idx].totalCycle;
		myPs->fadmPulseData[i].stepNo
			= myData->pulse_msg[msg][bd][ch].val[idx].stepNo;
		
		memcpy((char *)&myPs->fadmPulseData[i].val[msgCount-1],
			(char *)&myData->pulse_msg[msg][bd][ch].val[idx],
			sizeof(S_PULSE_MSG_VAL));
	} else if(myData->pulse_msg[msg][bd][ch].val[idx].type == 1) {
		if(myPs->fadmPulseData[i].stepNo
			== myData->pulse_msg[msg][bd][ch].val[idx].stepNo) {
			myPs->fadmPulseData[i].dataCount++;
			msgCount = myPs->fadmPulseData[i].dataCount;
			memcpy((char *)&myPs->fadmPulseData[i].val[msgCount-1],
				(char *)&myData->pulse_msg[msg][bd][ch].val[idx],
				sizeof(S_PULSE_MSG_VAL));
			if(msgCount == 100) {
				send_cmd_fadm_pulse_data_2(i, msgCount);
				myPs->fadmPulseData[i].dataCount = 0;
//				printf(" send fadm pulse data1 ch %d : %d\n",ch+1, msgCount);
			}
		} else {
			msgCount = myPs->fadmPulseData[i].dataCount;
			if(msgCount > 0) {
				send_cmd_fadm_pulse_data_2(i, msgCount);
//				printf(" send fadm pulse data2 ch %d : %d\n",ch+1, msgCount);
			}

			memset((char *)&myPs->fadmPulseData[i], 0,
				sizeof(S_MAIN_CH_PULSE_DATA));
			myPs->fadmPulseData[i].dataCount++;
			msgCount = myPs->fadmPulseData[i].dataCount;
			myPs->fadmPulseData[i].totalCycle
				= myData->pulse_msg[msg][bd][ch].val[idx].totalCycle;
			myPs->fadmPulseData[i].stepNo
				= myData->pulse_msg[msg][bd][ch].val[idx].stepNo;

			memcpy((char *)&myPs->fadmPulseData[i].val[msgCount-1],
				(char *)&myData->pulse_msg[msg][bd][ch].val[idx],
				sizeof(S_PULSE_MSG_VAL));
		}
	} else {
		myPs->fadmPulseData[i].dataCount++;
		msgCount = myPs->fadmPulseData[i].dataCount;
		memcpy((char *)&myPs->fadmPulseData[i].val[msgCount-1],
			(char *)&myData->pulse_msg[msg][bd][ch].val[idx],
			sizeof(S_PULSE_MSG_VAL));
		send_cmd_fadm_pulse_data_2(i, msgCount);
//		printf(" send fadm pulse data3 ch %d : %d\n",ch+1, msgCount);
		memset((char *)&myPs->fadmPulseData[i], 0,
			sizeof(S_MAIN_CH_PULSE_DATA));
	}
	
	return count;
}

void send_cmd_fadm_pulse_data_2(int i, int msgCount)
{
	int bd, ch, cmd_size, body_size, cnt, rtn;
	S_MAIN_SEND_CMD_CH_PULSE_DATA	cmd;

	bd = myData->CellArray1[i].bd;
	ch = myData->CellArray1[i].ch;
	
	cmd_size = sizeof(S_MAIN_CMD_HEADER)
		+ sizeof(long) * 3
		+ sizeof(S_MAIN_CH_PULSE_VAL) * msgCount;
	body_size = cmd_size - sizeof(S_MAIN_CMD_HEADER);
	memset((char *)&cmd, 0, sizeof(S_MAIN_SEND_CMD_CH_PULSE_DATA));
	make_header_2((char *)&cmd, REPLY_NO,
		MAIN_CMD_TO_PC_CH_PULSE_DATA, SEQNUM_AUTO, body_size, i);
	
	cmd.totalCycle = myPs->fadmPulseData[i].totalCycle;
	cmd.stepNo = myPs->fadmPulseData[i].stepNo;
	cmd.dataCount = msgCount;

	for(cnt=0; cnt < msgCount; cnt++) {
		cmd.val[cnt].runTime = myPs->fadmPulseData[i].val[cnt].runTime;
		cmd.val[cnt].Vsens = myPs->fadmPulseData[i].val[cnt].Vsens;
		cmd.val[cnt].Isens = myPs->fadmPulseData[i].val[cnt].Isens;
		cmd.val[cnt].capacity = myPs->fadmPulseData[i].val[cnt].capacity;
		cmd.val[cnt].wattHour = myPs->fadmPulseData[i].val[cnt].wattHour;
		//180103
//		printf(" RunTime%d : %ld",cnt, myPs->fadmPulseData[0].val[cnt].runTime);
//		printf(" Vsens : %ld", myPs->fadmPulseData[0].val[cnt].Vsens);
//		printf(" Isens : %ld", myPs->fadmPulseData[0].val[cnt].Isens);
//		printf(" capacity : %ld", myPs->fadmPulseData[0].val[cnt].capacity);
//		printf(" wattHour : %ld\n", myPs->fadmPulseData[0].val[cnt].wattHour);
	}

	rtn = send_command((char *)&cmd, cmd_size, MAIN_CMD_TO_PC_CH_PULSE_DATA);
//	userlog(DEBUG_LOG, psName, "180103 send ch fadm pulse_data_3!!!\n");
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d %d\n", rtn,
			MAIN_CMD_TO_PC_CH_PULSE_DATA);
	}
}

void send_cmd_ch_10mS_data(int i)
{
	int bd, ch, msg, msgCount;

	msg = 2;

	bd = myData->CellArray1[i].bd;
	ch = myData->CellArray1[i].ch;
	
	if(myData->DataSave.config.save_data_type == P0)
		return;
	if(myData->save10ms_msg[msg][bd][ch].flag == 1){ // Delta Save Data
		msgCount = myData->save10ms_msg[msg][bd][ch].write_idx+1;
//		msgCount = myData->save10ms_msg[msg][bd][ch].write_idx;
//		printf("MainClient : bd[%d] ch[%d] flag = %d msgCount1 = %d\n"
//				, bd+1, ch+1
//				, myData->save10ms_msg[msg][bd][ch].flag, msgCount);
	
		send_cmd_ch_10mS_data_2(i, msgCount, msg);

		myData->save10ms_msg[msg][bd][ch].write_idx = 0;
		myData->save10ms_msg[msg][bd][ch].flag = 0;
	}
}

void send_cmd_ch_10mS_data_2(int i, int msgCount, int msg)
{
	int bd, ch, cmd_size, body_size, idx, rtn;
	S_MAIN_SEND_CMD_CH_10MS_DATA cmd;

	cmd_size = sizeof(S_MAIN_CMD_HEADER) + sizeof(long)
			+ sizeof(S_MAIN_CH_DATA) * msgCount;
	body_size = cmd_size - sizeof(S_MAIN_CMD_HEADER);
	memset((char *)&cmd, 0x00, sizeof(S_MAIN_SEND_CMD_CH_10MS_DATA));
	make_header_2((char *)&cmd, REPLY_NO,
			MAIN_CMD_TO_PC_CH_10MS_DATA, SEQNUM_AUTO, body_size, i);

	cmd.dataCount = msgCount;

	bd = myData->CellArray1[i].bd;
	ch = myData->CellArray1[i].ch;

	for(idx = 0; idx < msgCount; idx++){
		memcpy((char *)&cmd.chData[idx],
				(char *)&myData->save10ms_msg[msg][bd][ch].chData[idx],
				sizeof(S_MAIN_CH_DATA));
		cmd.chData[idx].ch = i + 1;
	}

	rtn = send_command((char *)&cmd, cmd_size, MAIN_CMD_TO_PC_CH_10MS_DATA);

	if(rtn < 0){
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d %d\n", rtn,
					MAIN_CMD_TO_PC_CH_10MS_DATA);
	}
}
//160510 oys add
void send_cmd_step_cond_update(int bd, int ch)
{
	int cmd_size, body_size, rtn;
	unsigned short i, j;
	unsigned long chFlag;

	S_MAIN_SEND_CMD_STEP_REF_I_UPDATE cmd;
	
	cmd_size = sizeof(S_MAIN_SEND_CMD_STEP_REF_I_UPDATE);
	body_size = cmd_size - sizeof(S_MAIN_CMD_HEADER);
	memset((char *)&cmd, 0x00, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		MAIN_CMD_TO_PC_STEP_REF_I_UPDATE, SEQNUM_AUTO, body_size);

	i = myData->mData.config.chPerBd * bd + ch;
	i = myData->CellArray2[i].number1 - 1;
	chFlag = 0x00000001;

	chFlag = chFlag << i;
	cmd.header.chFlag[i/32] = chFlag;

	for(j = 0; j < MAX_STEP; j++) {
		if(myData->bData[bd].cData[ch].misc.changeRefI[j] == P1){
			if(myData->mData.testCond[bd][ch].step[j].mode == C_RATE){ //210308
				cmd.changeRefI[j]
					= myData->mData.testCond[bd][ch].step[j].refI;
			}else{
				cmd.changeRefI[j]
					= labs(myData->mData.testCond[bd][ch].step[j].refI);
			}
			myData->bData[bd].cData[ch].misc.changeRefI[j] = P0;
			printf("UpdateCond bd : %d ch : %d stepNo : %d refI: %ld\n", bd, ch, j+1, cmd.changeRefI[j]);
		}
	}
	rtn = send_command((char *)&cmd, cmd_size,
			MAIN_CMD_TO_PC_STEP_REF_I_UPDATE);

	if(rtn < 0){
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d %d\n", rtn,
					MAIN_CMD_TO_PC_STEP_REF_I_UPDATE);
	}
	for(i = 0; i < MAX_STEP; i++) {
		cmd.changeRefI[j] = 0;
	}
}
//add end

//#ifdef __LG_VER1__ hun_210929 SDI DATA RECOVERY
//20180717 sch add for SBC Recovery
int rcv_cmd_ch_recovery(void)
{
	int bd, ch, p_ch;
//	int log_flag = 1;
	
	S_MAIN_RCV_CMD_CH_RECOVERY	cmd;

	memset((char *)&cmd, 0x00, sizeof(S_MAIN_RCV_CMD_CH_RECOVERY));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
				sizeof(S_MAIN_RCV_CMD_CH_RECOVERY));

	//userlog(DEBUG_LOG,psName,"rcv_cmd_ch_recovery\n");
	
	p_ch = cmd.chData.ch - 1;
	bd = p_ch / myData->mData.config.chPerBd;
	ch = p_ch % myData->mData.config.chPerBd;

#if VENDER == 3
	myData->bData[bd].cData[ch].op.state = cmd.chData.state;
	myData->bData[bd].cData[ch].misc.tmpState = C_RUN;	//20210215_hun
	myData->bData[bd].cData[ch].op.state = C_PAUSE;
	myData->bData[bd].cData[ch].op.phase = P0;
	myData->bData[bd].cData[ch].op.resultIndex = cmd.chData.resultIndex;
	myData->DataSave.resultData[p_ch].resultIndex = cmd.chData.resultIndex;
	myData->bData[bd].cData[ch].op.type = cmd.chData.type;
	myData->bData[bd].cData[ch].op.mode = cmd.chData.mode;
	myData->bData[bd].cData[ch].op.code = cmd.chData.code;
	myData->bData[bd].cData[ch].op.stepNo = (unsigned long)cmd.chData.stepNo;
	myData->bData[bd].cData[ch].misc.advStepNo 
			= myData->bData[bd].cData[ch].op.stepNo - 1;
	myData->bData[bd].cData[ch].op.grade = cmd.chData.grade;
	myData->bData[bd].cData[ch].op.Vsens = cmd.chData.Vsens;
	myData->bData[bd].cData[ch].op.Isens = cmd.chData.Isens;
	myData->bData[bd].cData[ch].op.ampareHour = cmd.chData.capacity;
	myData->bData[bd].cData[ch].op.watt = cmd.chData.watt;
	myData->bData[bd].cData[ch].op.wattHour = cmd.chData.wattHour;
	myData->bData[bd].cData[ch].op.runTime = cmd.chData.runTime;
	myData->bData[bd].cData[ch].op.totalRunTime = cmd.chData.totalRunTime;
	myData->bData[bd].cData[ch].op.z = cmd.chData.z;
	myData->bData[bd].cData[ch].op.temp = cmd.chData.temp;
	myData->bData[bd].cData[ch].op.reservedCmd = cmd.chData.reservedCmd;
	myData->bData[bd].cData[ch].misc.totalCycle = cmd.chData.totalCycle;
	myData->bData[bd].cData[ch].misc.currentCycle = cmd.chData.currentCycle;
	myData->bData[bd].cData[ch].misc.advCycle = cmd.chData.currentCycle;
	myData->bData[bd].cData[ch].op.meanVolt = cmd.chData.avgV;
	myData->bData[bd].cData[ch].op.meanCurr = cmd.chData.avgI;
	#if NETWORK_VERSION >= 4102	
		#if VENDER != 2 //NOT SDI
			myData->bData[bd].cData[ch].op.integral_ampareHour
				   	= cmd.chData.IntegralAmpareHour;
			myData->bData[bd].cData[ch].op.integral_WattHour
				   	= cmd.chData.IntegralWattHour;
			myData->bData[bd].cData[ch].op.charge_ampareHour
				   	= cmd.chData.ChargeAmpareHour;
			myData->bData[bd].cData[ch].op.charge_wattHour
				   	= cmd.chData.ChargeWattHour;
			myData->bData[bd].cData[ch].op.discharge_ampareHour
				   	= cmd.chData.DischargeAmpareHour;
			myData->bData[bd].cData[ch].op.discharge_wattHour
				   	= cmd.chData.DischargeWattHour;
			myData->bData[bd].cData[ch].misc.cvTime = cmd.chData.cvTime;
			#if PROGRAM_VERSION1 == 0
				#if	PROGRAM_VERSION2 >= 1
			myData->bData[bd].cData[ch].op.capacitance = cmd.chData.Farad;
			myData->bData[bd].cData[ch].op.totalRunTime_carry
				   	= cmd.chData.totalRunTime_carry;
			myData->DataSave.resultData[p_ch].fileIndex = cmd.chData.fileIndex;
			myData->bData[bd].cData[ch].misc.cycleNo = cmd.chData.cycleNo;
			myData->bData[bd].cData[ch].op.temp1 = cmd.chData.temp1;
				#endif
				#if EDLC_TYPE == 1
			myData->bData[bd].cData[ch].misc.c_t1 = cmd.chData.c_t1;
			myData->bData[bd].cData[ch].misc.c_v1 = cmd.chData.c_v1;
			myData->bData[bd].cData[ch].misc.c_t2 = cmd.chData.c_t2;
			myData->bData[bd].cData[ch].misc.c_v2 = cmd.chData.c_v2;
			myData->bData[bd].cData[ch].op.capacitance_iec
				   	= cmd.chData.capacitance_iec;
			myData->bData[bd].cData[ch].op.capacitance_maxwell
					= cmd.chData.capacitance_maxwell;
				#endif
				#if	PROGRAM_VERSION2 >= 2
			myData->bData[bd].cData[ch].misc.chargeCCAh 
					= cmd.chData.chargeCCAh;
			myData->bData[bd].cData[ch].misc.chargeCVAh 
					= cmd.chData.chargeCVAh;
			myData->bData[bd].cData[ch].misc.dischargeCCAh 
					= cmd.chData.dischargeCCAh;
			myData->bData[bd].cData[ch].misc.dischargeCVAh 
					= cmd.chData.dischargeCVAh;
				#endif
			#endif
			#if PROGRAM_VERSION1 > 0
			myData->bData[bd].cData[ch].op.capacitance = cmd.chData.Farad;
			myData->bData[bd].cData[ch].op.totalRunTime_carry
				   	= cmd.chData.totalRunTime_carry;
			myData->DataSave.resultData[p_ch].fileIndex = cmd.chData.fileIndex;
			myData->bData[bd].cData[ch].misc.cycleNo = cmd.chData.cycleNo;
			myData->bData[bd].cData[ch].op.temp1 = cmd.chData.temp1;
				myData->bData[bd].cData[ch].misc.startV 
						= cmd.chData.startVoltage;
				myData->bData[bd].cData[ch].misc.maxV 
						= cmd.chData.maxVoltage;
				myData->bData[bd].cData[ch].misc.minV 
						= cmd.chData.minVoltage;
				myData->bData[bd].cData[ch].misc.startT 
						= cmd.chData.startTemp;
				myData->bData[bd].cData[ch].misc.maxT 
						= cmd.chData.maxTemp;
				myData->bData[bd].cData[ch].misc.minT 
						= cmd.chData.minTemp;
			#endif
		#endif
		//20201025 hun added for SK AC FAIL RECOVERY 
		#if VENDER == 3
		myData->bData[bd].cData[ch].misc.groupTemp = cmd.chData.Chamber_Temp;
		#endif
		#ifdef _AC_FAIL_RECOVERY 
		 myData->bData[bd].cData[ch].misc.advCycle
		  = cmd.chData.advCycle;
		 myData->bData[bd].cData[ch].misc.advCycleStep
		  = cmd.chData.advCycleStep;
		 myData->bData[bd].cData[ch].misc.cycleRunTime
		  = cmd.chData.cycleRunTime;
		 myData->bData[bd].cData[ch].misc.seedintegralCapacity
		  = cmd.chData.seedintegralCapacity;
		 myData->bData[bd].cData[ch].misc.sumintegralCapacity
		  = cmd.chData.sumintegralCapacity;
		 myData->bData[bd].cData[ch].misc.seedintegralWattHour
		  = cmd.chData.seedintegralWattHour;
		 myData->bData[bd].cData[ch].misc.sumintegralWattHour
		  = cmd.chData.sumintegralWattHour;
		 myData->bData[bd].cData[ch].misc.seedChargeAmpareHour
		  = cmd.chData.seedChargeAmpareHour;	 
		 myData->bData[bd].cData[ch].misc.sumChargeAmpareHour
		  = cmd.chData.sumChargeAmpareHour;
		 myData->bData[bd].cData[ch].misc.seedDischargeAmpareHour
		  = cmd.chData.seedDischargeAmpareHour;
		 myData->bData[bd].cData[ch].misc.sumDischargeAmpareHour
		  = cmd.chData.sumDischargeAmpareHour;
		 myData->bData[bd].cData[ch].misc.seedChargeWattHour
		  = cmd.chData.seedChargeWattHour;
		 myData->bData[bd].cData[ch].misc.sumChargeWattHour
	 	  = cmd.chData.sumChargeWattHour;
		 myData->bData[bd].cData[ch].misc.seedDischargeWattHour
		  = cmd.chData.seedDischargeWattHour;
		 myData->bData[bd].cData[ch].misc.sumDischargeWattHour
		  = cmd.chData.sumDischargeWattHour;

		 myData->bData[bd].cData[ch].misc.standardC
		 = cmd.chData.standardC;
		 myData->bData[bd].cData[ch].misc.standardP
		 = cmd.chData.standardP;
		 myData->bData[bd].cData[ch].misc.standardZ
		 = cmd.chData.standardZ;
		 myData->bData[bd].cData[ch].misc.cycleSumC
		 = cmd.chData.cycleSumC;
		 myData->bData[bd].cData[ch].misc.cycleSumP
		 = cmd.chData.cycleSumP;
		 myData->bData[bd].cData[ch].misc.cycleEndC
		 = cmd.chData.cycleEndC;
		 myData->bData[bd].cData[ch].misc.pattern_change_flag
		 = cmd.chData.pattern_change_flag;

		 myData->bData[bd].cData[ch].misc.chGroupNo
		 = cmd.chData.chGroupNo;
		 myData->bData[bd].cData[ch].misc.tempDir
		 = cmd.chData.tempDir;
		/*
	  	userlog(DEBUG_LOG, psName, "ch : %d\n",ch);
	  	userlog(DEBUG_LOG, psName, "stepNo : %d\n",
			myData->bData[bd].cData[ch].op.stepNo);
	  	userlog(DEBUG_LOG, psName, "advStepNo : %d\n",
			myData->bData[bd].cData[ch].misc.advStepNo);
	  	userlog(DEBUG_LOG, psName, "totalCycle : %d\n",
			myData->bData[bd].cData[ch].misc.totalCycle);
	  	userlog(DEBUG_LOG, psName, "advCycle : %d\n",
			myData->bData[bd].cData[ch].misc.advCycle);
	  	userlog(DEBUG_LOG, psName, "cycleNo : %d\n",
			myData->bData[bd].cData[ch].misc.cycleNo);
	  	userlog(DEBUG_LOG, psName, "advCycleStep : %d\n",
			myData->bData[bd].cData[ch].misc.advCycleStep);
	  	userlog(DEBUG_LOG, psName, "seedintegralCapacity : %d\n",
			myData->bData[bd].cData[ch].misc.seedintegralCapacity);
	  	userlog(DEBUG_LOG, psName, "sumintegralCapacity : %d\n",
			myData->bData[bd].cData[ch].misc.sumintegralCapacity);
	  	userlog(DEBUG_LOG, psName, "seedinetegralWattHour : %d\n",
			myData->bData[bd].cData[ch].misc.seedintegralWattHour);
	  	userlog(DEBUG_LOG, psName, "sumintegralWattHour : %d\n",
			myData->bData[bd].cData[ch].misc.sumintegralWattHour);
	  	userlog(DEBUG_LOG, psName, "seedChaargeAmpareHour : %d\n",
			myData->bData[bd].cData[ch].misc.seedChargeAmpareHour);
	  	userlog(DEBUG_LOG, psName, "sumChargeAmpareHour : %d\n",
			myData->bData[bd].cData[ch].misc.sumChargeAmpareHour);
	  	userlog(DEBUG_LOG, psName, "seedDischargeAmpareHour : %d\n",
			myData->bData[bd].cData[ch].misc.seedDischargeAmpareHour);
	  	userlog(DEBUG_LOG, psName, "sumDischargeAmpareHour : %d\n",
			myData->bData[bd].cData[ch].misc.sumDischargeAmpareHour);
	  	userlog(DEBUG_LOG, psName, "seedChargeWattHour : %d\n",
			myData->bData[bd].cData[ch].misc.seedChargeWattHour);
	  	userlog(DEBUG_LOG, psName, "sumChargeWattHour : %d\n",
			myData->bData[bd].cData[ch].misc.sumChargeWattHour);
	  	userlog(DEBUG_LOG, psName, "seedDischargeWattHour : %d\n",
			myData->bData[bd].cData[ch].misc.seedDischargeWattHour);
	  	userlog(DEBUG_LOG, psName, "sumDischargeWattHour : %d\n",
			myData->bData[bd].cData[ch].misc.sumDischargeWattHour);

	  	userlog(DEBUG_LOG, psName, "standardP : %d\n",
			 myData->bData[bd].cData[ch].misc.standardP);
	  	userlog(DEBUG_LOG, psName, "standardZ : %d\n",
			myData->bData[bd].cData[ch].misc.standardZ);
	  	userlog(DEBUG_LOG, psName, "cycleSumC : %d\n",
			myData->bData[bd].cData[ch].misc.cycleSumC);
	  	userlog(DEBUG_LOG, psName, "cycleSumP : %d\n",
			myData->bData[bd].cData[ch].misc.cycleSumP);
	  	userlog(DEBUG_LOG, psName, "cycleEndC : %d\n",
			myData->bData[bd].cData[ch].misc.cycleEndC);
	  	userlog(DEBUG_LOG, psName, "pattern_charge_falg : %d\n",
			myData->bData[bd].cData[ch].misc.pattern_change_flag);
	  	userlog(DEBUG_LOG, psName, "groupTemp : %d\n",
			myData->bData[bd].cData[ch].misc.groupTemp);
	  	userlog(DEBUG_LOG, psName, "chGroupNo : %d\n",
			myData->bData[bd].cData[ch].misc.chGroupNo);
	  	userlog(DEBUG_LOG, psName, "tempDir : %d\n\n",
			myData->bData[bd].cData[ch].misc.tempDir);
		*/
		#endif
		#ifdef _TRACKING_MODE
		myData->bData[bd].cData[ch].op.SOC
		= cmd.chData.SOC;
		myData->bData[bd].cData[ch].op.ampareHour_SOC
		= cmd.chData.RPT_SOC;
		myData->bData[bd].cData[ch].op.SOH
		= cmd.chData.SOH;
		myData->bData[bd].cData[ch].op.ampareHour_SOH
		= cmd.chData.RPT_SOH;
		myData->bData[bd].cData[ch].misc.socTrackingStep
		= cmd.chData.socRefStep;
		myData->bData[bd].cData[ch].misc.sohTrackingStep
		= cmd.chData.sohRefStep;
		myData->bData[bd].cData[ch].misc.chamberNoWaitFlag
		= cmd.chData.chamberNoWaitFlag;
		if(myData->bData[bd].cData[ch].misc.chamberNoWaitFlag == 1)	
			 myData->bData[bd].cData[ch].misc.chGroupNo = 0;
		#endif
	#endif	
#endif
#if VENDER == 2
	myData->bData[bd].cData[ch].op.state = cmd.chData.state;
	myData->bData[bd].cData[ch].misc.tmpState = C_RUN;	//20210215_hun
	myData->bData[bd].cData[ch].op.state = C_PAUSE;
	myData->bData[bd].cData[ch].op.phase = P0;
	myData->bData[bd].cData[ch].op.resultIndex = cmd.chData.resultIndex;
	myData->DataSave.resultData[p_ch].resultIndex = cmd.chData.resultIndex;
	myData->bData[bd].cData[ch].op.type = cmd.chData.type;
	myData->bData[bd].cData[ch].op.mode = cmd.chData.mode;
	myData->bData[bd].cData[ch].op.code = cmd.chData.code;
	myData->bData[bd].cData[ch].op.stepNo = (unsigned long)cmd.chData.stepNo;
	myData->bData[bd].cData[ch].misc.advStepNo 
		= myData->bData[bd].cData[ch].op.stepNo - 1;
	myData->bData[bd].cData[ch].op.Vsens = cmd.chData.Vsens;
	myData->bData[bd].cData[ch].op.Isens = cmd.chData.Isens;
	myData->bData[bd].cData[ch].op.runTime 
		= (ONE_DAY_RUNTIME * cmd.chData.runTime_day) + cmd.chData.runTime;
	myData->bData[bd].cData[ch].op.totalRunTime 
		= (ONE_DAY_RUNTIME * cmd.chData.totalRunTime_carry) + cmd.chData.totalRunTime;
	myData->bData[bd].cData[ch].op.z = cmd.chData.z;
	myData->bData[bd].cData[ch].op.reservedCmd = cmd.chData.reservedCmd;
	myData->bData[bd].cData[ch].misc.totalCycle = cmd.chData.totalCycle;
	myData->bData[bd].cData[ch].misc.currentCycle = cmd.chData.currentCycle;
	myData->bData[bd].cData[ch].misc.advCycle = cmd.chData.currentCycle;
	myData->bData[bd].cData[ch].op.meanVolt = cmd.chData.avgV;
	myData->bData[bd].cData[ch].op.meanCurr = cmd.chData.avgI;

	myData->bData[bd].cData[ch].op.charge_ampareHour 
		= cmd.chData.ChargeAmpareHour;
	myData->bData[bd].cData[ch].op.discharge_ampareHour 
		= cmd.chData.DischargeAmpareHour;
	if(myData->bData[bd].cData[ch].op.type == STEP_CHARGE){
		myData->bData[bd].cData[ch].op.watt = cmd.chData.ChargeWatt;
	}else if(myData->bData[bd].cData[ch].op.type == STEP_DISCHARGE){
		myData->bData[bd].cData[ch].op.watt = cmd.chData.DischargeWatt;
	}
	myData->bData[bd].cData[ch].op.charge_wattHour 
		= cmd.chData.ChargeWattHour;	
	myData->bData[bd].cData[ch].op.discharge_wattHour 
		= cmd.chData.DischargeWattHour;	
	myData->bData[bd].cData[ch].misc.cycleRunTime 
		= (ONE_DAY_RUNTIME * cmd.chData.runTime_day) + cmd.chData.runTime;
	myData->bData[bd].cData[ch].op.temp = cmd.chData.temp[0][0];
	myData->bData[bd].cData[ch].misc.minT = cmd.chData.temp[0][1];		
	myData->bData[bd].cData[ch].misc.maxT = cmd.chData.temp[0][2];
	myData->bData[bd].cData[ch].op.meanTemp = cmd.chData.temp[0][3];
	myData->bData[bd].cData[ch].misc.gotoCycleCount[cmd.chData.stepNo - 1] 
		= cmd.chData.gotoCycleCount;
	myData->bData[bd].cData[ch].misc.cvTime 
		= (ONE_DAY_RUNTIME * cmd.chData.cvTime_day) + cmd.chData.cvTime;
	myData->bData[bd].cData[ch].op.integral_ampareHour 
		= cmd.chData.IntegralAmpareHour;	
	myData->bData[bd].cData[ch].op.integral_WattHour 
		= cmd.chData.IntegralWattHour;
	myData->bData[bd].cData[ch].misc.ccTime 
		= (ONE_DAY_RUNTIME * cmd.chData.ccTime_day) + cmd.chData.ccTime;					

	myData->bData[bd].cData[ch].misc.chargeCCAh 
		= cmd.chData.charge_cc_ampare_hour;
	myData->bData[bd].cData[ch].misc.chargeCVAh 
		= cmd.chData.charge_cv_ampare_hour;		
	myData->bData[bd].cData[ch].misc.dischargeCCAh 
		= cmd.chData.discharge_cc_ampare_hour;
	myData->bData[bd].cData[ch].misc.dischargeCVAh 
		= cmd.chData.discharge_cv_ampare_hour;		
	myData->bData[bd].cData[ch].misc.startV 
		= cmd.chData.startVoltage;
	myData->bData[bd].cData[ch].misc.step_count 
		= cmd.chData.step_count;
	myData->bData[bd].cData[ch].misc.maxV 
		= cmd.chData.maxVoltage;
	myData->bData[bd].cData[ch].misc.minV 
		= cmd.chData.minVoltage;
	#ifdef _AC_FAIL_RECOVERY
	//Data Restore
	myData->bData[bd].cData[ch].misc.advCycle 
		= cmd.chData.advCycle;
	myData->bData[bd].cData[ch].misc.currentCycle  
		= myData->bData[bd].cData[ch].misc.advCycle;
	myData->bData[bd].cData[ch].misc.advCycleStep  
		= cmd.chData.advCycleStep;
	myData->bData[bd].cData[ch].misc.cycleRunTime  
		= cmd.chData.cycleRunTime;
	myData->bData[bd].cData[ch].misc.seedintegralCapacity 
		= cmd.chData.seedintegralCapacity;
	myData->bData[bd].cData[ch].misc.sumintegralCapacity 
		= cmd.chData.sumintegralCapacity;
	myData->bData[bd].cData[ch].misc.seedintegralWattHour 
		= cmd.chData.seedintegralWattHour;
	myData->bData[bd].cData[ch].misc.sumintegralWattHour 
		= cmd.chData.sumintegralWattHour;
	myData->bData[bd].cData[ch].misc.seedChargeAmpareHour 
		= cmd.chData.seedChargeAmpareHour;	 
	myData->bData[bd].cData[ch].misc.sumChargeAmpareHour 
		= cmd.chData.sumChargeAmpareHour;
	myData->bData[bd].cData[ch].misc.seedDischargeAmpareHour 
		= cmd.chData.seedDischargeAmpareHour;
	myData->bData[bd].cData[ch].misc.sumDischargeAmpareHour 
		= cmd.chData.sumDischargeAmpareHour;
	myData->bData[bd].cData[ch].misc.seedChargeWattHour 
		= cmd.chData.seedChargeWattHour;
	myData->bData[bd].cData[ch].misc.sumChargeWattHour 
		= cmd.chData.sumChargeWattHour;
	myData->bData[bd].cData[ch].misc.seedDischargeWattHour 
		= cmd.chData.seedDischargeWattHour;
	myData->bData[bd].cData[ch].misc.sumDischargeWattHour 
		= cmd.chData.sumDischargeWattHour;
	
	myData->bData[bd].cData[ch].misc.standardC 
		= cmd.chData.standardC;
	myData->bData[bd].cData[ch].misc.standardP 
		= cmd.chData.standardP;
	myData->bData[bd].cData[ch].misc.standardZ 
		= cmd.chData.standardZ;
	myData->bData[bd].cData[ch].misc.cycleSumC 
		= cmd.chData.cycleSumC;
	myData->bData[bd].cData[ch].misc.cycleSumP 
		= cmd.chData.cycleSumP;
	myData->bData[bd].cData[ch].misc.cycleEndC 
		= cmd.chData.cycleEndC;
	myData->bData[bd].cData[ch].misc.pattern_change_flag 
		= cmd.chData.pattern_change_flag;

	myData->bData[bd].cData[ch].misc.chGroupNo 
		= cmd.chData.chGroupNo;
	myData->bData[bd].cData[ch].misc.tempDir 
		= cmd.chData.tempDir;
	myData->bData[bd].cData[ch].misc.sumChargeCCAh 
		= cmd.chData.sumChargeCCAh;
	myData->bData[bd].cData[ch].misc.seedChargeCCAh 
		= cmd.chData.seedChargeCCAh;
	myData->bData[bd].cData[ch].misc.sumChargeCVAh 
		= cmd.chData.sumChargeCVAh;
	myData->bData[bd].cData[ch].misc.seedChargeCVAh 
		= cmd.chData.seedChargeCVAh;
	myData->bData[bd].cData[ch].misc.sumChargeCCCVAh 
		= cmd.chData.sumChargeCCCVAh;
	myData->bData[bd].cData[ch].misc.seedChargeCCCVAh 
		= cmd.chData.seedChargeCCCVAh;	
	myData->bData[bd].cData[ch].misc.sumDischargeCCAh 
		= cmd.chData.sumDischargeCCAh;
	myData->bData[bd].cData[ch].misc.seedDischargeCCAh 
		= cmd.chData.seedDischargeCCAh;
	myData->bData[bd].cData[ch].misc.sumDischargeCVAh 
		= cmd.chData.sumDischargeCVAh;
	myData->bData[bd].cData[ch].misc.seedDischargeCVAh 
		= cmd.chData.seedDischargeCVAh;
	myData->bData[bd].cData[ch].misc.sumDischargeCCCVAh 
		= cmd.chData.sumDischargeCCCVAh;
	myData->bData[bd].cData[ch].misc.seedDischargeCCCVAh 
		= cmd.chData.seedDischargeCCCVAh;
	myData->bData[bd].cData[ch].misc.chargeCCAh 
		= cmd.chData.chargeCCAh;
	myData->bData[bd].cData[ch].misc.chargeCVAh 
		= cmd.chData.chargeCVAh;
	myData->bData[bd].cData[ch].misc.chargeCCCVAh 
		= cmd.chData.chargeCCCVAh;
	myData->bData[bd].cData[ch].misc.dischargeCCAh 
		= cmd.chData.dischargeCCAh;
	myData->bData[bd].cData[ch].misc.dischargeCVAh 
		= cmd.chData.dischargeCVAh;
	myData->bData[bd].cData[ch].misc.dischargeCCCVAh 
		= cmd.chData.dischargeCCCVAh;
	myData->bData[bd].cData[ch].misc.cycleSumChargeWatt 
		= cmd.chData.cycleSumChargeWatt;
	myData->bData[bd].cData[ch].misc.cycleSumChargeWattHour 
		= cmd.chData.cycleSumChargeWattHour;
	myData->bData[bd].cData[ch].misc.cycleSumChargeAmpareHour 
		= cmd.chData.cycleSumChargeAmpareHour;
	myData->bData[bd].cData[ch].misc.cycleSumDischargeWatt 
		= cmd.chData.cycleSumDischargeWatt;
	myData->bData[bd].cData[ch].misc.cycleSumDischargeWattHour 
		= cmd.chData.cycleSumDischargeWattHour;
	myData->bData[bd].cData[ch].misc.cycleSumDischargeAmpareHour 
		= cmd.chData.cycleSumDischargeAmpareHour;

	myData->bData[bd].cData[ch].misc.cycleStepCount 
		= cmd.chData.cycleStepCount;		 
	myData->bData[bd].cData[ch].misc.cycleSumAvgT 
		= cmd.chData.cycleSumAvgT;	
	myData->DataSave.resultData[p_ch].fileIndex 
		= cmd.chData.fileIndex;
	#endif
	
//	if(log_flag == 1){
//		recovery_data(p_ch,bd,ch);	
//	}
#endif
	myData->bData[bd].cData[ch].misc.ac_fail_flag = P1;
	
	return send_cmd_response((char *)&cmd.header, EP_CD_ACK);
}
//#endif

//20201025 hun added for SK AC FAIL RECOVERY
int rcv_cmd_ch_convert_adv_cycle_step(void)
{
	int i = 0;
	int p_ch, bd, ch;
	S_MAIN_RCV_CMD_CH_CONVERT_ADV_CYCLE_STEP	cmd;

	memset((char *)&cmd, 0x00,sizeof(S_MAIN_RCV_CMD_CH_CONVERT_ADV_CYCLE_STEP));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
				sizeof(S_MAIN_RCV_CMD_CH_CONVERT_ADV_CYCLE_STEP));
	
	//userlog(DEBUG_LOG,psName,"rcv_cmd_ch_convert_adv_cycle_step\n");
	
	p_ch = cmd.ch - 1;
	bd = p_ch / myData->mData.config.chPerBd;
	ch = p_ch % myData->mData.config.chPerBd;

	for(i = myData->bData[bd].cData[ch].misc.advStepNo ; i >= 0 ; i--){
		if(myData->mData.testCond[bd][ch].step[i].type == STEP_ADV_CYCLE)
		{
			Convert_ADV_CYCLE_Step(i, bd, ch);
			break;
		}	
	}
	return send_cmd_response((char *)&cmd.header, EP_CD_ACK);
}

//20201025 hun added for SK AC FAIL RECOVERY 
int rcv_cmd_ch_end_data_recovery(void)
{
	int bd, ch, p_ch;
	int i = 0;
	int stepNo = 0;
	long total_step = 0;
	#if VENDER == 3	
	long advStepNo = 0;
	long SocSoeFlag = 0;
	unsigned long capStep = 0;
	unsigned long powerStep = 0;
	#endif
	S_MAIN_RCV_CMD_CH_END_DATA_RECOVERY	cmd;

	memset((char *)&cmd, 0x00, sizeof(S_MAIN_RCV_CMD_CH_END_DATA_RECOVERY));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
				sizeof(S_MAIN_RCV_CMD_CH_END_DATA_RECOVERY));

	//userlog(DEBUG_LOG,psName,"rcv_cmd_ch_end_data_recovery\n");

	p_ch = cmd.ch - 1;
	bd = p_ch / myData->mData.config.chPerBd;
	ch = p_ch % myData->mData.config.chPerBd;
	
	total_step = cmd.total_step;
	
	//userlog(DEBUG_LOG,psName,"ch : %d\n",ch);
	//userlog(DEBUG_LOG,psName,"total_step : %d\n",total_step);
   	

	for(i = 0 ; i < cmd.total_step ; i++){
		stepNo = cmd.chData[i].stepNo -1;
		//userlog(DEBUG_LOG,psName,"StepNo : %d\n",stepNo);
		if(stepNo >= 0){
		#if VENDER == 3
			if(myData->mData.config.capacityType == CAPACITY_AMPARE_HOURS){
				myData->mData.testCond[bd][ch].step[stepNo].socStepCap
				= cmd.chData[i].capacity;
				//userlog(DEBUG_LOG,psName,"step[%d].socStepCap(AH) : %d\n",stepNo,
					//myData->mData.testCond[bd][ch].step[stepNo].socStepCap);
			}else if(myData->mData.config.capacityType == CAPACITY_CAPACITANCE){
				myData->mData.testCond[bd][ch].step[stepNo].socStepCap
				= cmd.chData[i].Farad;
				//userlog(DEBUG_LOG,psName,"step[%d].socStepCap : %d\n",stepNo,
					//myData->mData.testCond[bd][ch].step[stepNo].socStepCap);
			}
			myData->mData.testCond[bd][ch].step[stepNo].socStepPower
				= cmd.chData[i].wattHour / 1000;
			myData->mData.testCond[bd][ch].step[stepNo].socStepZ
				= cmd.chData[i].z;
			//userlog(DEBUG_LOG,psName,"step[%d].socStepPower : %d\n",stepNo,
				//myData->mData.testCond[bd][ch].step[stepNo].socStepPower);
			//userlog(DEBUG_LOG,psName,"step[%d].socStepZ : %d\n\n",stepNo,
				//myData->mData.testCond[bd][ch].step[stepNo].socStepZ);
		#endif
		#if VENDER == 2
			myData->bData[bd].cData[ch].misc.sel_Cyc_C_Cap[stepNo]
			 = cmd.chData[i].sel_Cyc_C_Cap;
			myData->bData[bd].cData[ch].misc.sel_Cyc_D_Cap[stepNo]
			 = cmd.chData[i].sel_Cyc_D_Cap;
			userlog(DEBUG_LOG,psName,"sel_Cyc_C_Cap[%d] : %d\n",stepNo,
				myData->bData[bd].cData[ch].misc.sel_Cyc_C_Cap[stepNo]);
			userlog(DEBUG_LOG,psName,"sel_Cyc_D_Cap[%d] : %d\n",stepNo,
				myData->bData[bd].cData[ch].misc.sel_Cyc_D_Cap[stepNo]);
		#endif
		}
	}
	#if VENDER == 3
	advStepNo = myData->bData[bd].cData[ch].misc.advStepNo;
	SocSoeFlag = myData->mData.testCond[bd][ch].step[advStepNo].SocSoeFlag;
	
	if(myData->mData.testCond[bd][ch].step[advStepNo].socCapStepNo != 0){
		if(SocSoeFlag < 2){
			capStep 
			= myData->mData.testCond[bd][ch].step[advStepNo].socCapStepNo - 1;
			if(capStep < 1000){
				if(myData->mData.config.capacityType == CAPACITY_AMPARE_HOURS){
					myData->bData[bd].cData[ch].misc.actualCapacity
					= myData->mData.testCond[bd][ch].step[capStep].socStepCap;
				}else if(myData->mData.config.capacityType == CAPACITY_CAPACITANCE){
					myData->bData[bd].cData[ch].misc.actualCapacity
					= myData->mData.testCond[bd][ch].step[capStep].socStepCap;
				}
			}
		}else{
			powerStep 
			 = myData->mData.testCond[bd][ch].step[advStepNo].socCapStepNo - 1;
			myData->bData[bd].cData[ch].misc.actualWattHour
			 = myData->mData.testCond[bd][ch].step[powerStep].socStepPower;
		}
	}
	#endif
	
	return send_cmd_response((char *)&cmd.header, EP_CD_ACK);
}

//20180717 sch add for goto count recovery
int rcv_cmd_goto_count_recovery(void)
{
	int bd, ch, LoopCount, i, j;
	S_MAIN_CMD_GOTO_COUNT_RECOVERY cmd;

	//userlog(DEBUG_LOG,psName,"rcv_cmd_goto_count_recovery\n");

	memset((char *)&cmd, 0x00, sizeof(S_MAIN_CMD_GOTO_COUNT_RECOVERY));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
				sizeof(S_MAIN_CMD_GOTO_COUNT_RECOVERY));
	i = 0;
	j = 0;

	bd = (cmd.ch) / myData->mData.config.chPerBd;
	ch = (cmd.ch) % myData->mData.config.chPerBd;
	LoopCount = cmd.LoopCount;
	for(i = 0; i < MAX_STEP; i++){
		if(myData->mData.testCond[bd][ch].step[i].type == STEP_LOOP){
			myData->bData[bd].cData[ch].misc.gotoCycleCount[i]
					= cmd.gotoCycleCount[j];
			j++;
		}
		if(j > LoopCount)
			break;
	}
	return send_cmd_response((char *)&cmd.header, EP_CD_ACK);
}
//20180717 sch add for goto count recovery
int rcv_cmd_goto_count_request(void)
{
	int bd, ch, p_ch;
	S_MAIN_RCV_CMD_GOTO_COUNT_REQUEST cmd;

	memset((char *)&cmd, 0x00, sizeof(S_MAIN_RCV_CMD_GOTO_COUNT_REQUEST));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
				sizeof(S_MAIN_RCV_CMD_GOTO_COUNT_REQUEST));

	p_ch = cmd.ch - 1;
	bd = p_ch / myData->mData.config.chPerBd;
	ch = p_ch % myData->mData.config.chPerBd;
	
	//printf("rcv_cmd_goto_count_request\n");	
	send_cmd_goto_count(p_ch, bd, ch);

	return 0;
}

void send_cmd_goto_count(int p_ch, int bd, int ch)
{
	int cmd_size, body_size, rtn;
	int i;

	S_MAIN_SEND_CMD_GOTO_COUNT cmd;
	
	cmd_size = sizeof(S_MAIN_SEND_CMD_GOTO_COUNT);
	body_size = cmd_size - sizeof(S_MAIN_CMD_HEADER);
	memset((char *)&cmd, 0x00, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
				 MAIN_CMD_TO_PC_GOTO_COUNT, SEQNUM_AUTO, body_size);

	cmd.ch = p_ch + 1;
	
	//printf("send_cmd_goto_count\n");	
	for(i = 0; i < MAX_STEP; i++){
		cmd.gotoCycleCount[i] 
		 = (short)myData->bData[bd].cData[ch].misc.gotoCycleCount[i];
	//	printf("ch[%d] : gotoCycleCount[%d]\n",cmd.ch,cmd.gotoCycleCount[i]);	
	}
	rtn = send_command((char *)&cmd, cmd_size, MAIN_CMD_TO_PC_GOTO_COUNT);
	if(rtn < 0){
		userlog(DEBUG_LOG, psName, "goto cmd send error!!!! %d %d \n", rtn, 
					MAIN_CMD_TO_PC_GOTO_COUNT_UPDATE);
	}
}

int rcv_cmd_goto_count_data(void)
{
	int bd, ch, p_ch, i;
	S_MAIN_RCV_CMD_GOTO_COUNT_DATA cmd;

	memset((char *)&cmd, 0x00, sizeof(S_MAIN_RCV_CMD_GOTO_COUNT_DATA));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
				sizeof(S_MAIN_RCV_CMD_GOTO_COUNT_DATA));

	p_ch = cmd.ch - 1;
	bd = p_ch / myData->mData.config.chPerBd;
	ch = p_ch % myData->mData.config.chPerBd;
	
	//printf("rcv_cmd_goto_count_data\n");	
	for(i = 0; i < MAX_STEP; i++){
		(short)myData->bData[bd].cData[ch].misc.gotoCycleCount[i] 
		 = cmd.gotoCycleCount[i];
		//printf("ch[%d] : gotoCycleCount[%d]\n",cmd.ch,cmd.gotoCycleCount[i]);	
	}

	return send_cmd_response((char *)&cmd.header, EP_CD_ACK);
}

void Convert_ADV_CYCLE_Step(int advStepNo, int bd, int ch)
{
	unsigned char endC_std_type, endP_std_type, endZ_std_type, rangeI;
   	unsigned short c_rate;
	int i, stepType;
    long stdC_val;
	unsigned long advStepNo_1;
	long endC_std_sel;
	long endP_std_sel;
	long endZ_std_sel;	

	advStepNo_1 = advStepNo;
	endC_std_type = myData->mData.testCond[bd][ch].step[advStepNo].socCapStepNo;
	endP_std_type = myData->mData.testCond[bd][ch].step[advStepNo].endP_std_type;
	endZ_std_type = myData->mData.testCond[bd][ch].step[advStepNo].endZ_std_type;
	
	c_rate = myData->mData.testCond[bd][ch].step[advStepNo].endTCVGoto;	
	myData->bData[bd].cData[ch].misc.completeFlag
		= myData->mData.testCond[bd][ch].step[advStepNo].gotoCycleCount;

	myData->bData[bd].cData[ch].misc.socCheckCount
		= myData->mData.testCond[bd][ch].step[advStepNo].endVGoto;

	myData->bData[bd].cData[ch].misc.cycleEndC
		= myData->mData.testCond[bd][ch].step[advStepNo].endDeltaV;

	myData->bData[bd].cData[ch].misc.endC_std_sel
		= myData->mData.testCond[bd][ch].step[advStepNo].endCGoto;
	myData->bData[bd].cData[ch].misc.endP_std_sel
		= myData->mData.testCond[bd][ch].step[advStepNo].endP_std_sel;
	myData->bData[bd].cData[ch].misc.endZ_std_sel
		= myData->mData.testCond[bd][ch].step[advStepNo].endZ_std_sel;
	endZ_std_sel = myData->bData[bd].cData[ch].misc.endZ_std_sel;
	myData->bData[bd].cData[ch].misc.endC_std_cycleCount
		= myData->mData.testCond[bd][ch].step[advStepNo].endC_std_cycleCount;
	myData->bData[bd].cData[ch].misc.endP_std_cycleCount
		= myData->mData.testCond[bd][ch].step[advStepNo].endP_std_cycleCount;
	myData->bData[bd].cData[ch].misc.endZ_std_cycleCount
		= myData->mData.testCond[bd][ch].step[advStepNo].endZ_std_cycleCount;
	
	endC_std_sel = myData->bData[bd].cData[ch].misc.endC_std_sel;
	endP_std_sel = myData->bData[bd].cData[ch].misc.endP_std_sel;
	//userlog(DEBUG_LOG,psName,"endC_std_sel : [%d]\n",
		//myData->bData[bd].cData[ch].misc.endC_std_sel);
	//userlog(DEBUG_LOG,psName,"endP_std_sel : [%d]\n",
		//myData->bData[bd].cData[ch].misc.endP_std_sel);
	//User standard input value	
	if(myData->bData[bd].cData[ch].misc.cRateUseFlag == P1) {
		stdC_val = 0;
	} else {
		stdC_val = myData->mData.testCond[bd][ch].step[advStepNo].endP;
	}
	myData->bData[bd].cData[ch].misc.socCountNo[0] = 0;
	myData->bData[bd].cData[ch].misc.socCountNo[1] = 0;
	myData->bData[bd].cData[ch].misc.socCountNo[2] = 0;
	for(i=0; i < MAX_STEP; i++) {
		myData->bData[bd].cData[ch].misc.ahEndRatio[i] = 100; //default : 100
	}
	for(i=0; i < MAX_STEP; i++) {
		myData->bData[bd].cData[ch].misc.ahEndRatio[i] = 100; //default : 100
	}

#if VENDER == 3
	if(myData->bData[bd].cData[ch].misc.standardC != 0 || myData->bData[bd].cData[ch].misc.standardP != 0) {
#else
	if(myData->bData[bd].cData[ch].misc.standardC != 0) {
#endif		
		//LGC C-rate Standard Capacity Auto Calculation ref I value.
		if(VENDER == 1) {
			while(1) { //현재 스탭에서의 Cycle <-> Loop
				advStepNo_1++;
				if(STEP_LOOP 
				== myData->mData.testCond[bd][ch].step[advStepNo_1].type) {
					break;
				}
				if(c_rate == 0) break;
					if(myData->mData.testCond[bd][ch].step[advStepNo_1]
					.cycleEndStepSave == 1) {
					stepType = myData->mData.testCond[bd][ch].step[advStepNo_1].type;
					if(stepType == STEP_CHARGE) {
						if(myData->bData[bd].cData[ch].ChAttribute.chNo_master == P0) {
							myData->mData.testCond[bd][ch].step[advStepNo_1].refI
								= (myData->bData[bd].cData[ch].misc.standardC
								* ((float)c_rate / 100)) / 2;
							myData->mData.testCond[bd][ch-1].step[advStepNo_1].refI
								= myData->mData.testCond[bd][ch].step[advStepNo_1]
								.refI;
						} else {
							myData->mData.testCond[bd][ch].step[advStepNo_1].refI
								= myData->bData[bd].cData[ch].misc.standardC
								* ((float)c_rate / 100);
						}
					} else if(stepType == STEP_DISCHARGE) {
						if(myData->bData[bd].cData[ch].ChAttribute.chNo_master == P0) {
							myData->mData.testCond[bd][ch].step[advStepNo_1].refI
								= ((myData->bData[bd].cData[ch].misc.standardC
								* ((float)c_rate / 100)) / 2) * -1;
							myData->mData.testCond[bd][ch-1].step[advStepNo_1].refI
								= myData->mData.testCond[bd][ch].step[advStepNo_1]
								.refI;
						} else {
							myData->mData.testCond[bd][ch].step[advStepNo_1].refI
								= (myData->bData[bd].cData[ch].misc.standardC
								* ((float)c_rate / 100)) * -1;
						}
					}
						//171102 add for c_rate Change RangeI
					rangeI = 0;
					for(i=MAX_RANGE; i > 0; i--) {
						if(labs(myData->mData.testCond[bd][ch].step[advStepNo_1].refI)
							 <= myData->mData.config.maxCurrent[i - 1]) {
							rangeI = i - 1;
							break;
						}
					}
					
					if(myData->bData[bd].cData[ch].ChAttribute.chNo_master == P0) {
						myData->mData.testCond[bd][ch].step[advStepNo_1]
							.rangeI = rangeI;
						myData->mData.testCond[bd][ch - 1].step[advStepNo_1]
							.rangeI = rangeI;
					} else {
						myData->mData.testCond[bd][ch].step[advStepNo_1]
							.rangeI = rangeI;
					}
						//add for Change standardC
					myData->bData[bd].cData[ch].misc.standardC_Flag = P0;
						//changeRefI
					if(stepType == STEP_CHARGE
						|| stepType == STEP_DISCHARGE) {
						myData->bData[bd].cData[ch].misc.changeRefI[advStepNo] = P1;
					}
				}	
			}
			advStepNo_1 = advStepNo;
			//changeRefI end
		} 
	}
	
	myData->bData[bd].cData[ch].misc.endCycleTime 
		= myData->mData.testCond[bd][ch].step[advStepNo].endT;
	myData->bData[bd].cData[ch].misc.endIntegralC 
		= myData->mData.testCond[bd][ch].step[advStepNo].endC;
	myData->bData[bd].cData[ch].misc.endIntegralWh 
		= myData->mData.testCond[bd][ch].step[advStepNo].endWh;
	myData->bData[bd].cData[ch].misc.endCycleTimeGotoStep
		= myData->mData.testCond[bd][ch].step[advStepNo].endTGoto;

//170518 lyh add	
	myData->bData[bd].cData[ch].misc.cycleEndV 
		= myData->mData.testCond[bd][ch].step[advStepNo].advGotoStep;

// 171121 oys modify
	myData->bData[bd].cData[ch].misc.endIntegralCGotoStep 
		= myData->mData.testCond[bd][ch].step[advStepNo].endIntegralCGoto;
	myData->bData[bd].cData[ch].misc.endIntegralWhGotoStep 
		= myData->mData.testCond[bd][ch].step[advStepNo].endIntegralWhGoto;

	myData->bData[bd].cData[ch].misc.integralTGotoCheck
		= myData->mData.testCond[bd][ch].step[advStepNo].cycleEndStepSave;
//rcv pc	
	myData->bData[bd].cData[ch].misc.integralCGotoCheck
		= myData->mData.testCond[bd][ch].step[advStepNo].cycleEndStepSave;
	myData->bData[bd].cData[ch].misc.integralWhGotoCheck
		= myData->mData.testCond[bd][ch].step[advStepNo].cycleEndStepSave;

// cycle integral Ah , Wh Flag 
	myData->bData[bd].cData[ch].misc.endIntegralCFlag
		= (long)(myData->mData.testCond[bd][ch].step[advStepNo].endV / 1000);
	myData->bData[bd].cData[ch].misc.endIntegralWhFlag
		= (long)(myData->mData.testCond[bd][ch].step[advStepNo].endI / 1000);

	//1.Capacity
	if(endC_std_sel == P1) {
		if(myData->bData[bd].cData[ch].misc.endC_std_cycleCount < 
			myData->bData[bd].cData[ch].misc.advCycle){
			if(endC_std_type != 0){
				if(endC_std_type == 1){
					myData->bData[bd].cData[ch].misc.endC_std_type 
					 = STEP_CHARGE;
				}else if(endC_std_type == 2){
					myData->bData[bd].cData[ch].misc.endC_std_type 
					 = STEP_DISCHARGE;
				}
				myData->bData[bd].cData[ch].misc.standardC_Flag = P1;
			}
		}
	}else if(endC_std_sel > 1) {
		if(myData->bData[bd].cData[ch].misc.endC_std_cycleCount < 
			myData->bData[bd].cData[ch].misc.advCycle){
			if(myData->mData.testCond[bd][ch].step[endC_std_sel-1].socStepCap != 0) {
				myData->bData[bd].cData[ch].misc.endC_std_type 
					= myData->mData.testCond[bd][ch].step[endC_std_sel-1].type;
				myData->bData[bd].cData[ch].misc.standardC_Flag = P1;
			}
		}
	}
	//2.wattHour
	if(endP_std_sel == P1) {
		if(myData->bData[bd].cData[ch].misc.endP_std_cycleCount < 
			myData->bData[bd].cData[ch].misc.advCycle){
			if(endP_std_type != 0){
				if(endP_std_type == 1){
					myData->bData[bd].cData[ch].misc.endP_std_type 
					 = STEP_CHARGE;
				}else if(endP_std_type == 2){
					myData->bData[bd].cData[ch].misc.endP_std_type 
					 = STEP_DISCHARGE;
				}
				myData->bData[bd].cData[ch].misc.standardP_Flag = P1;
			}
		}
	}else if(endP_std_sel > 1) {
		if(myData->bData[bd].cData[ch].misc.endP_std_cycleCount <
			myData->bData[bd].cData[ch].misc.advCycle){
			if(myData->mData.testCond[bd][ch].step[endP_std_sel-1].socStepCap != 0) {
				myData->bData[bd].cData[ch].misc.endP_std_type 
					= myData->mData.testCond[bd][ch].step[endP_std_sel-1].type;
				myData->bData[bd].cData[ch].misc.standardP_Flag = P1;
			}
		}
	}
	//3.DCIR
	if(endZ_std_sel == P1){ //Cycle step
		if(myData->bData[bd].cData[ch].misc.endZ_std_cycleCount <=
			myData->bData[bd].cData[ch].misc.advCycle){
			//if(endZ_std_type < myData->bData[bd].cData[ch].misc.advStepNo+1){
			if(myData->bData[bd].cData[ch].misc.standardZ != 0){
				myData->bData[bd].cData[ch].misc.endZ_std_type 
					= myData->mData.testCond[bd][ch].step[endZ_std_type-1].type;
				myData->bData[bd].cData[ch].misc.standardZ_Flag = P1;
			}
		}
	//20180721 sch modify for reset standardX
	}else if(endZ_std_sel > 1) { //StepNo
		if(myData->bData[bd].cData[ch].misc.endZ_std_cycleCount <=
			myData->bData[bd].cData[ch].misc.advCycle){
			if(myData->mData.testCond[bd][ch].step[endZ_std_sel-1].socStepZ != 0) {
				//if(endZ_std_type < myData->bData[bd].cData[ch].misc.advStepNo+1){
				if(myData->bData[bd].cData[ch].misc.standardZ != 0){
					myData->bData[bd].cData[ch].misc.endZ_std_type 
						= myData->mData.testCond[bd][ch].step[endZ_std_sel-1].type;
					myData->bData[bd].cData[ch].misc.standardZ_Flag = P1;
				}
			}
		}
	}
}

void send_cmd_goto_count_update(int bd, int ch)
{
	int cmd_size, body_size, rtn;
	unsigned short i, j, k;
	unsigned long chFlag;

	S_MAIN_CMD_GOTO_COUNT_RECOVERY cmd;
	
	cmd_size = sizeof(S_MAIN_CMD_GOTO_COUNT_RECOVERY);
	body_size = cmd_size - sizeof(S_MAIN_CMD_HEADER);
	memset((char *)&cmd, 0x00, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
				 MAIN_CMD_TO_PC_GOTO_COUNT_UPDATE, SEQNUM_AUTO, body_size);
	i = myData->mData.config.chPerBd * bd + ch;
	i = myData->CellArray2[i].number1 - 1;
	chFlag = 0x00000001;
	chFlag = chFlag << i;
	
	cmd.header.chFlag[i/32] = chFlag;

	k = 0;

	for(j = 0; j < MAX_STEP; j++){
		if(myData->mData.testCond[bd][ch].step[j].type == STEP_LOOP){
			cmd.gotoCycleCount[k] = 
				(short)myData->bData[bd].cData[ch].misc.gotoCycleCount[j];
			k++;
		}
	}
	
	cmd.LoopCount = k;
	for(j = k; j < 99; j++){
		cmd.gotoCycleCount[j] = 0;
	}
	cmd.ch = myData->mData.config.chPerBd * bd + ch;
	cmd.totalCycle
		= myData->bData[bd].cData[ch].misc.totalCycle;
	rtn = send_command((char *)&cmd, cmd_size,
			MAIN_CMD_TO_PC_GOTO_COUNT_UPDATE);
	if(rtn < 0){
		userlog(DEBUG_LOG, psName, "goto cmd send error!!!! %d %d \n", rtn, 
					MAIN_CMD_TO_PC_GOTO_COUNT_UPDATE);
	}
}


int send_cmd_step_cond_reply(void)
{
	return 0;
}

int send_cmd_safety_cond_reply(void)
{
	return 0;
}

void send_cmd_meter_connect_reply(void)
{
	int cmd_size, body_size, rtn;
	S_MAIN_SEND_CMD_METER_CONNECT_REPLY cmd;

	cmd_size = sizeof(S_MAIN_SEND_CMD_METER_CONNECT_REPLY);
	body_size = cmd_size - sizeof(S_MAIN_CMD_HEADER);
	memset((char *)&cmd, 0x00, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		MAIN_CMD_TO_PC_CALI_METER_CONNECT_REPLY, SEQNUM_AUTO, body_size);
	
	cmd.header.chFlag[0] = 0;
	cmd.header.chFlag[1] = 0;
	cmd.state = 1; //disconnect:0, connect:1
	
	rtn = send_command((char *)&cmd, cmd_size,
		MAIN_CMD_TO_PC_CALI_METER_CONNECT_REPLY);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d %d\n", rtn,
			MAIN_CMD_TO_PC_CALI_METER_CONNECT_REPLY);
	}
}

void send_cmd_cali_start_reply(int bd, int ch, int code)
{
	int cmd_size, body_size, rtn, i;
	unsigned long	chFlag;
	S_MAIN_SEND_CMD_CALI_START_REPLY cmd;

	cmd_size = sizeof(S_MAIN_SEND_CMD_CALI_START_REPLY);
	body_size = cmd_size - sizeof(S_MAIN_CMD_HEADER);
	memset((char *)&cmd, 0x00, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		MAIN_CMD_TO_PC_CALI_START_REPLY, SEQNUM_AUTO, body_size);
	
	i = myData->mData.config.chPerBd * bd + ch;
	i = myData->CellArray2[i].number1 - 1;
	chFlag = 0x00000001;

	chFlag = chFlag << i;
	cmd.header.chFlag[i/32] = chFlag;

	cmd.header.reserved1 = 0;
	cmd.header.reserved2 = 0;
	cmd.response.cmd = MAIN_CMD_TO_SBC_CALI_START;
	cmd.response.code = code;
	
	rtn = send_command((char *)&cmd, cmd_size,
		MAIN_CMD_TO_PC_CALI_START_REPLY);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d %d\n", rtn,
			MAIN_CMD_TO_PC_CALI_START_REPLY);
	}
}

void send_cmd_cali_normal_result(int bd, int ch)
{
	int cmd_size, body_size, rtn, i, type, range, point;
	unsigned long chFlag;
	S_MAIN_SEND_CMD_CALI_NORMAL_RESULT	cmd;
	
	cmd_size = sizeof(S_MAIN_SEND_CMD_CALI_NORMAL_RESULT);
	body_size = cmd_size - sizeof(S_MAIN_CMD_HEADER);
	memset((char *)&cmd, 0x00, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		MAIN_CMD_TO_PC_CALI_NORMAL_RESULT, SEQNUM_AUTO, body_size);
	
	i = myData->mData.config.chPerBd * bd + ch;
	i = myData->CellArray2[i].number1 - 1;
	chFlag = 0x00000001;

	chFlag = chFlag << i;
	cmd.header.chFlag[i/32] = chFlag;

	type = myData->cali.tmpCond[bd][ch].type;
	range = myData->cali.tmpCond[bd][ch].range;

	cmd.result.type = type;
	cmd.result.range = (unsigned char)range;
	cmd.result.setPointNum
		= myData->cali.tmpCond[bd][ch].point[type][range].setPointNum;
	cmd.result.checkPointNum
		= myData->cali.tmpCond[bd][ch].point[type][range].checkPointNum;
	cmd.result.ch = (unsigned char)(i+1);

	for(point=0; point < cmd.result.setPointNum; point++) {
		if(type == 0) {
			cmd.result.setPointAD[point]
				= (long)myData->cali.tmpData[bd][ch].set_ad[type][range][point];
		} else {
			cmd.result.setPointAD[point]
				= (long)myData->cali.tmpData[bd][ch].set_ad[type][range][point];
		}
		cmd.result.setPointDVM[point]
			= myData->cali.tmpData[bd][ch].set_meter[type][range][point];
#ifdef _SK_CALI_TYPE
		cmd.result.set_meterValue[point]
			= myData->cali.tmpData[bd][ch].set_meterValue[type][range][point];
#endif
	}

	for(point=0; point < cmd.result.checkPointNum; point++) {
		cmd.result.checkPointAD[point]
			= myData->cali.tmpData[bd][ch].check_ad[type][range][point];
		cmd.result.checkPointDVM[point]
			= myData->cali.tmpData[bd][ch].check_meter[type][range][point];
#ifdef _SK_CALI_TYPE
		cmd.result.check_meterValue[point]
			= myData->cali.tmpData[bd][ch].check_meterValue[type][range][point];
#endif
	}
	printf("\n");
#if SHUNT_R_RCV >= 1
	for(i = 0; i < MAX_SHUNT_SERIAL_LENGTH; i++){
		cmd.result.shuntSerialNo[i]
			= myData->cali.tmpCond[bd][ch].point[type][range].shuntSerialNo[i];
	}
	cmd.result.shuntValue
		= myData->cali.tmpCond[bd][ch].point[type][range].shuntValue;
#endif
#if SHUNT_R_RCV == 2    //180515 add for hallCT Cali
	cmd.result.shuntValue2
		= myData->cali.tmpCond[bd][ch].point[type][range].shuntValue2;
	for(i = 0; i < MAX_OFFSET_POINT; i++){
		cmd.result.meter_offset[i]
			= myData->cali.tmpCond[bd][ch].point[type][range].meter_offset[i];
	}
#endif
	rtn = send_command((char *)&cmd, cmd_size, MAIN_CMD_TO_PC_CALI_NORMAL_RESULT);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d %d\n", rtn,
			MAIN_CMD_TO_PC_CALI_NORMAL_RESULT);
	}
}

void send_cmd_cali_check_result(int bd, int ch)
{
	int cmd_size, body_size, rtn, i, type, range, point;
	unsigned long chFlag;
	S_MAIN_SEND_CMD_CALI_CHECK_RESULT	cmd;
	
	cmd_size = sizeof(S_MAIN_SEND_CMD_CALI_CHECK_RESULT);
	body_size = cmd_size - sizeof(S_MAIN_CMD_HEADER);
	memset((char *)&cmd, 0x00, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		MAIN_CMD_TO_PC_CALI_CHECK_RESULT, SEQNUM_AUTO, body_size);
	
	i = myData->mData.config.chPerBd * bd + ch;
	i = myData->CellArray2[i].number1 - 1;
	chFlag = 0x00000001;

	chFlag = chFlag << i;
	cmd.header.chFlag[i/32] = chFlag;

	type = myData->cali.tmpCond[bd][ch].type;
	range = myData->cali.tmpCond[bd][ch].range;

	cmd.result.type = type;
	cmd.result.range = (unsigned char)range;
	cmd.result.checkPointNum
		= myData->cali.tmpCond[bd][ch].point[type][range].checkPointNum;
	cmd.result.ch = (unsigned char)(i+1);

	for(point=0; point < cmd.result.checkPointNum; point++) {
		cmd.result.checkPointAD[point]
			= myData->cali.tmpData[bd][ch].check_ad[type][range][point];
		cmd.result.checkPointDVM[point]
			= myData->cali.tmpData[bd][ch].check_meter[type][range][point];
#ifdef _SK_CALI_TYPE
		cmd.result.check_meterValue[point]
			= myData->cali.tmpData[bd][ch].check_meterValue[type][range][point];
#endif
	}
#if SHUNT_R_RCV >= 1
	for(i = 0; i < MAX_SHUNT_SERIAL_LENGTH; i++){
		cmd.result.shuntSerialNo[i]
			= myData->cali.tmpCond[bd][ch].point[type][range].shuntSerialNo[i];
	}
	cmd.result.shuntValue
		= myData->cali.tmpCond[bd][ch].point[type][range].shuntValue;
#endif
#if SHUNT_R_RCV == 2     //180515 add for hallct cali
	cmd.result.shuntValue2
		= myData->cali.tmpCond[bd][ch].point[type][range].shuntValue2;
	for(i = 0; i < MAX_OFFSET_POINT; i++){
		cmd.result.meter_offset[i]
			= myData->cali.tmpCond[bd][ch].point[type][range].meter_offset[i];
	}
#endif
	rtn = send_command((char *)&cmd, cmd_size, MAIN_CMD_TO_PC_CALI_CHECK_RESULT);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d %d\n", rtn,
			MAIN_CMD_TO_PC_CALI_CHECK_RESULT);
	}
}

void send_cmd_comm_check_reply(void)
{
	int cmd_size, body_size, rtn;
	S_MAIN_SEND_CMD_COMM_CHECK_REPLY	cmd;
	
	cmd_size = sizeof(S_MAIN_SEND_CMD_COMM_CHECK_REPLY);
	body_size = cmd_size - sizeof(S_MAIN_CMD_HEADER);
	memset((char *)&cmd, 0x00, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		MAIN_CMD_TO_PC_COMM_CHECK_REPLY, SEQNUM_AUTO, body_size);

	rtn = send_command((char *)&cmd, cmd_size, MAIN_CMD_TO_PC_COMM_CHECK_REPLY);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d %d\n", rtn,
			MAIN_CMD_TO_PC_COMM_CHECK_REPLY);
	}
}

void send_cmd_comm_check(void)
{
	int cmd_size, body_size, rtn;
	S_MAIN_SEND_CMD_COMM_CHECK	cmd;
	
	cmd_size = sizeof(S_MAIN_SEND_CMD_COMM_CHECK);
	body_size = cmd_size - sizeof(S_MAIN_CMD_HEADER);
	memset((char *)&cmd, 0x00, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		MAIN_CMD_TO_PC_COMM_CHECK, SEQNUM_AUTO, body_size);

	rtn = send_command((char *)&cmd, cmd_size, MAIN_CMD_TO_PC_COMM_CHECK);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d %d\n", rtn,
			MAIN_CMD_TO_PC_COMM_CHECK);
	}
}

void send_cmd_emg_status(int code, int val)
{
	int cmd_size, body_size, rtn;
	S_MAIN_SEND_CMD_EMG_STATUS	cmd;
	
	cmd_size = sizeof(S_MAIN_SEND_CMD_EMG_STATUS);
	body_size = cmd_size - sizeof(S_MAIN_CMD_HEADER);
	memset((char *)&cmd, 0x00, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		MAIN_CMD_TO_PC_EMG_STATUS, SEQNUM_AUTO, body_size);

	cmd.code = (long)code;
	cmd.val = (long)val;
	userlog(DEBUG_LOG, psName, "emg_status code:[%d] val[%d]\n", cmd.code,cmd.val);
	rtn = send_command((char *)&cmd, cmd_size, MAIN_CMD_TO_PC_EMG_STATUS);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d %d\n", rtn,
			MAIN_CMD_TO_PC_EMG_STATUS);
	}
}

void send_cmd_ch_data_backup_reply(void)
{
	int bd, ch, cmd_size, body_size, i, rtn;
	unsigned long chFlag;
	S_MAIN_SEND_CMD_CH_DATA_BACKUP_REPLY	cmd;
	
	cmd_size = sizeof(S_MAIN_SEND_CMD_CH_DATA_BACKUP_REPLY);
	body_size = cmd_size - sizeof(S_MAIN_CMD_HEADER);
	
	memset((char *)&cmd, 0x00, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		MAIN_CMD_TO_PC_CH_DATA_BACKUP_REPLY, SEQNUM_AUTO, body_size);

	chFlag = 0x00000001;
	for(i=0; i < myData->mData.config.installedCh; i++) {
		bd = myData->CellArray1[i].bd;
		ch = myData->CellArray1[i].ch;
		if(myData->AppControl.backup[i] == P3
			|| myData->AppControl.backup[i] == P4) {
			if(i < 32){
				cmd.header.chFlag[0] = chFlag << i;
			}else{
				cmd.header.chFlag[1] = chFlag << (i-32);
			}
			if(myData->AppControl.backup[i] == P3){
				cmd.flag = 1;
				myData->bData[bd].cData[ch].op.state = C_IDLE;
				myData->bData[bd].cData[ch].op.phase = P0;
			}else if(myData->AppControl.backup[i] == P4){
				cmd.flag = 0;
			}
			myData->AppControl.backup[i] = P0;
			myData->AppControl.backupFlag[i] = P0;
			printf("chFlag[0] : %lu\n", cmd.header.chFlag[0]);
			printf("chFlag[1] : %lu\n", cmd.header.chFlag[1]);
			printf("backup_reply\n");
			rtn = send_command((char *)&cmd, cmd_size, MAIN_CMD_TO_PC_CH_DATA_BACKUP_REPLY);
			if(rtn < 0) {
				userlog(DEBUG_LOG, psName, "cmd send error!!! %d %d\n", rtn,
					MAIN_CMD_TO_PC_CH_DATA_BACKUP_REPLY);
			}
		}
	}
}

void send_cmd_ch_data_restore_reply(void)
{
	int cmd_size, body_size, i, rtn;
	unsigned long chFlag;
	S_MAIN_SEND_CMD_CH_DATA_RESTORE_REPLY	cmd;
	
	cmd_size = sizeof(S_MAIN_SEND_CMD_CH_DATA_RESTORE_REPLY);
	body_size = cmd_size - sizeof(S_MAIN_CMD_HEADER);
	
	memset((char *)&cmd, 0x00, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		MAIN_CMD_TO_PC_CH_DATA_RESTORE_REPLY, SEQNUM_AUTO, body_size);

	chFlag = 0x00000001;
	for(i=0; i < myData->mData.config.installedCh; i++) {
		if(myData->AppControl.restore[i] == P3
			|| myData->AppControl.restore[i] == P4) {
			if(i < 32){
				cmd.header.chFlag[0] = chFlag << i;
			}else{
				cmd.header.chFlag[1] = chFlag << (i-32);
			}
			if(myData->AppControl.restore[i] == P3){
				cmd.flag = 1;
			}else if(myData->AppControl.restore[i] == P4){
				cmd.flag = 0;
			}
			myData->AppControl.restore[i] = P0;
			myData->AppControl.restoreFlag[i] = P0;
			printf("chFlag[0] : %lu\n", cmd.header.chFlag[0]);
			printf("chFlag[1] : %lu\n", cmd.header.chFlag[1]);
			printf("restore_reply\n");
			rtn = send_command((char *)&cmd, cmd_size, MAIN_CMD_TO_PC_CH_DATA_RESTORE_REPLY);
			if(rtn < 0) {
				userlog(DEBUG_LOG, psName, "cmd send error!!! %d %d\n", rtn,
					MAIN_CMD_TO_PC_CH_DATA_RESTORE_REPLY);
			}
		}
	}
}

void send_cmd_jig_status()
{
	int cmd_size, body_size, rtn;
	S_MAIN_SEND_CMD_JIG_STATUS	cmd;
	cmd_size = sizeof(S_MAIN_SEND_CMD_JIG_STATUS);
	body_size = cmd_size - sizeof(S_MAIN_CMD_HEADER);
	memset((char *)&cmd, 0x00, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		MAIN_CMD_TO_PC_JIG_STATUS, SEQNUM_AUTO, body_size);
	rtn = send_command((char *)&cmd, cmd_size, MAIN_CMD_TO_PC_JIG_STATUS);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d %d\n", rtn,
			MAIN_CMD_TO_PC_JIG_STATUS);
	}
}

void send_cmd_unknown(int seqno, int ret)
{
/*	int	rtn, cmd_size, body_size;
	S_MAIN_SEND_CMD_REQUEST	cmd;
	
	cmd_size = sizeof(S_MAIN_SEND_CMD_REQUEST);
	memset((char *)&cmd, 0x00, cmd_size);
	body_size = 0;
	userlog(DEBUG_LOG, psName, "ack1a %d\n", body_size);
	make_header((char*)&cmd, REPLY_NO, MAIN_CMD_TO_PC_RESPONSE, SEQNUM_AUTO,
		body_size);

	rtn = send_command((char *)&cmd, cmd_size, MAIN_CMD_TO_PC_RESPONSE);
	if(rtn < 0)
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d %d\n", rtn,
			MAIN_CMD_TO_PC_REPONSE);
	*/
}

//131228 oys w : real time add
#if REAL_TIME == 1
void send_cmd_real_time_request(void)
{
	int cmd_size, body_size, rtn;
	S_MAIN_SEND_CMD_REAL_TIME_REQUEST   cmd;

	cmd_size = sizeof(S_MAIN_SEND_CMD_REAL_TIME_REQUEST);
	body_size = cmd_size - sizeof(S_MAIN_CMD_HEADER);
	memset((char *)&cmd, 0x00, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		MAIN_CMD_TO_PC_REAL_TIME_REQUEST, SEQNUM_AUTO, body_size);
	sleep(1);
	rtn = send_command((char *)&cmd, cmd_size,
		MAIN_CMD_TO_PC_REAL_TIME_REQUEST);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d %d\n", rtn,
			MAIN_CMD_TO_PC_REAL_TIME_REQUEST);
	}
}
#endif

void make_header(char *cmd, char reply, int cmd_id, int seqno, int body_size)
{
	int i;
	unsigned long	tmp;
	S_MAIN_CMD_HEADER	header;
	
	memset((char *)&header, 0x00, sizeof(S_MAIN_CMD_HEADER));
	
	cmd_id = (unsigned long)cmd_id;
	switch(cmd_id) {
		case MAIN_CMD_TO_PC_COMM_CHECK_REPLY:
			header.cmd_id = MAIN_CMD_TO_PC_RESPONSE;
			break;
		default:
			header.cmd_id = cmd_id;
			break;
	}

	switch(cmd_id) {
		case MAIN_CMD_TO_PC_CH_DATA:
			tmp = 0x01;
			for(i=0; i < myData->mData.config.installedCh; i++) {
				header.chFlag[i/32] |= tmp;

				tmp = tmp << 1;
				if(i == 31) tmp = 0x01;
			}
			break;
		case MAIN_CMD_TO_PC_CH_PULSE_DATA:
		case MAIN_CMD_TO_PC_CH_PULSE_IEC_DATA:
			break;
		default:
			break;
	}
	
	header.body_size = body_size;

	if(seqno == SEQNUM_AUTO) {
		myPs->misc.cmd_serial++;
		if(myPs->misc.cmd_serial > MAX_MAIN_CMD_SERIAL)
			myPs->misc.cmd_serial = 1;
		header.cmd_serial = myPs->misc.cmd_serial;
	} else if(seqno == SEQNUM_NONE) {
		header.cmd_serial = myPs->misc.cmd_serial;
	}

	memcpy(cmd, (char *)&header.cmd_id, sizeof(S_MAIN_CMD_HEADER));
	
/*	userlog(MAIN_LOG, psName, "header");
	for(i=0; i < sizeof(S_MAIN_CMD_HEADER); i++) {
		userlog2(MAIN_LOG, psName, " %x", *(cmd + i));
	}
	userlog2(MAIN_LOG, psName, ":end\n");
	userlog(MAIN_LOG, psName, "header %s:end\n", cmd); //kjgd */
}

void make_header_2(char *cmd, char reply, int cmd_id, int seqno, int body_size, int ch)
{
	int i;
	unsigned long	tmp;
	S_MAIN_CMD_HEADER	header;
	
	memset((char *)&header, 0x00, sizeof(S_MAIN_CMD_HEADER));
	
	cmd_id = (unsigned long)cmd_id;
	switch(cmd_id) {
		case MAIN_CMD_TO_PC_COMM_CHECK_REPLY:
			header.cmd_id = MAIN_CMD_TO_PC_RESPONSE;
			break;
		default:
			header.cmd_id = cmd_id;
			break;
	}

	switch(cmd_id) {
		case MAIN_CMD_TO_PC_CH_PULSE_DATA:
		case MAIN_CMD_TO_PC_CH_PULSE_IEC_DATA:
			tmp = 0x01;
			for(i=0; i < myData->mData.config.installedCh; i++) {
				if(i == ch) header.chFlag[i/32] |= tmp;

				tmp = tmp << 1;
				if(i == 31) tmp = 0x01;
			}
			break;
		default:
			break;
	}
	
	header.body_size = body_size;

	if(seqno == SEQNUM_AUTO) {
		myPs->misc.cmd_serial++;
		if(myPs->misc.cmd_serial > MAX_MAIN_CMD_SERIAL)
			myPs->misc.cmd_serial = 1;
		header.cmd_serial = myPs->misc.cmd_serial;
	} else if(seqno == SEQNUM_NONE) {
		header.cmd_serial = myPs->misc.cmd_serial;
	}

	memcpy(cmd, (char *)&header.cmd_id, sizeof(S_MAIN_CMD_HEADER));
	
/*	userlog(MAIN_LOG, psName, "header");
	for(i=0; i < sizeof(S_MAIN_CMD_HEADER); i++) {
		userlog2(MAIN_LOG, psName, " %x", *(cmd + i));
	}
	userlog2(MAIN_LOG, psName, ":end\n");
	userlog(MAIN_LOG, psName, "header %s:end\n", cmd); //kjgd */
}

int	send_command(char *cmd, int size, int cmd_id)
{
	char	packet[MAX_MAIN_PACKET_LENGTH];
	unsigned char tmp;
	int i;
	if(size > MAX_MAIN_PACKET_LENGTH) {
		userlog(DEBUG_LOG, psName,
			"CMD SEND FAIL!! TOO LARGE SIZE[%d]\n", size);
		return -1;
	}
	
	memset((char *)&packet[0], 0x00, MAX_MAIN_PACKET_LENGTH);
	memcpy((char *)&packet[0], cmd, size);

/*	if(cmd.header.reply == REPLY_YES) {
		if(myPs->reply.timer_run == P0) {
			memset((char *)&myPs->reply.retry, 0x00, sizeof(S_MAIN_RETRY_DATA));

			memset(buf, 0x00, sizeof buf);
			strncpy(buf, (char *)&cmd.header.seqno[0],
				sizeof(cmd.header.seqno));
			myPs->reply.retry.seqno = atoi(buf);
			
			myPs->reply.retry.replyCmd = get_reply_cmdid(cmd.header.cmd_id);

			strncpy((char *)&myPs->reply.retry.buf[0], cmd, size);
				myPs->reply.retry.size = size;
				myPs->reply.timer = myData->mData.misc.timer_1sec;
				myPs->reply.timer_run = P1;
			}
	}*/
	
	if(myPs->config.CmdSendLog == P1) {
		if(myPs->config.CommCheckLog == P1) {
			userlog(MAIN_LOG, psName, "sendCmd %s:end\n", packet);
		} else {
			switch(cmd_id){
				case MAIN_CMD_TO_PC_CH_DATA:
				case MAIN_CMD_TO_PC_RESPONSE:
					tmp = *(char *)(cmd + sizeof(S_MAIN_CMD_HEADER));
					if(tmp != EP_CD_ACK) {
						userlog(MAIN_LOG, psName, "sendCmd %s:end\n", packet);
					}
					break;
				case MAIN_CMD_TO_PC_CH_PULSE_DATA:
				case MAIN_CMD_TO_PC_CH_PULSE_IEC_DATA:
				case MAIN_CMD_TO_PC_COMM_CHECK_REPLY:
				case MAIN_CMD_TO_PC_COMM_CHECK:
					break;
				default:
					userlog(MAIN_LOG, psName, "sendCmd %s:end\n", packet);
					break;
			}
		}
	}
	
	if(myPs->config.CmdSendLog_Hex == P1) {
		if(myPs->config.CommCheckLog == P1) {
			userlog(MAIN_LOG, psName, "sendCmd");
			for(i=0; i < size; i++) {
				tmp = *(cmd + i);
				userlog2(MAIN_LOG, psName, " %02x", tmp);
			}
			userlog2(MAIN_LOG, psName, ":end\n");
		} else {
			switch(cmd_id){
				case MAIN_CMD_TO_PC_CH_DATA:
				case MAIN_CMD_TO_PC_RESPONSE:
					tmp = *(char *)(cmd + sizeof(S_MAIN_CMD_HEADER));
					if(tmp != EP_CD_ACK) {
						userlog(MAIN_LOG, psName, "sendCmd");
						for(i=0; i < size; i++) {
							tmp = *(cmd + i);
							userlog2(MAIN_LOG, psName, " %02x", tmp);
						}
						userlog2(MAIN_LOG, psName, ":end\n");
					}
					break;
				case MAIN_CMD_TO_PC_CH_PULSE_DATA:
				case MAIN_CMD_TO_PC_CH_PULSE_IEC_DATA:
				case MAIN_CMD_TO_PC_COMM_CHECK_REPLY:
				case MAIN_CMD_TO_PC_COMM_CHECK:
					break;
				default:
					userlog(MAIN_LOG, psName, "sendCmd");
					for(i=0; i < size; i++) {
						tmp = *(cmd + i);
						userlog2(MAIN_LOG, psName, " %02x", tmp);
					}
					userlog2(MAIN_LOG, psName, ":end\n");
					break;
			}
		}
	}
	return writen(myPs->config.network_socket, packet, size);
}

int	get_reply_cmdid(char *cmd_id)
{
	char	buf[10];
	int		cmdId, rtn;
	
	memset(buf, 0x00, sizeof buf);
	strncpy(buf, cmd_id, 4);
	cmdId = atoi(buf);
	switch(cmdId) {
		case MAIN_CMD_TO_SBC_COMM_CHECK:	
			rtn = MAIN_CMD_TO_PC_RESPONSE;	
			break;
		default:	
			rtn = 0;	
			break;
	}
	return rtn;
}

int Check_NetworkState(void)
{
	int rtn=0;

	network_ping();
	check_cmd_reply_timeout();
	if(check_network_timeout() < 0) {
		rtn = -1;
	}

	return rtn;
}

void network_ping(void)
{
	int diff;

	if(myPs->signal[MAIN_SIG_NET_CONNECTED] != P2) return;

	if(myPs->reply.timer_run == P0) {
		diff = (int)(myData->mData.misc.timer_1sec - myPs->pingTimer);
		if(diff > myPs->config.pingTimeout) {
			myPs->pingTimer = myData->mData.misc.timer_1sec;
			send_cmd_comm_check();
		}
	}
}

void check_cmd_reply_timeout(void)
{
	int diff, rtn;
	
	if(myPs->reply.timer_run == P0) return;

	diff = (int)(myData->mData.misc.timer_1sec - myPs->reply.timer);
	if(diff > myPs->config.replyTimeout) {
		userlog(DEBUG_LOG, psName,
			"TIMEOUT: retryCount[%d] replyCmd[%d] replySeqNo[%d]\n",
			myPs->reply.retry.count, myPs->reply.retry.replyCmd,
			myPs->reply.retry.seqno);
		if(myPs->reply.retry.count >= myPs->config.retryCount) {
			myPs->reply.timer_run = P0;
			myPs->pingTimer = myData->mData.misc.timer_1sec;
			memset((char *)&myPs->reply.retry, 0x00, sizeof(S_MAIN_RETRY_DATA));
		} else {
			myPs->reply.timer_run = P1;
			myPs->reply.timer = myData->mData.misc.timer_1sec;
			myPs->reply.retry.count++;
				
			if(myPs->config.CmdSendLog == P1) {
				userlog(DEBUG_LOG, psName, "retry %s\n", myPs->reply.retry.buf);
			}

			rtn = writen(myPs->config.send_socket,
				(char *)&myPs->reply.retry.buf, myPs->reply.retry.size);
		}
	}
}

int	check_network_timeout(void)
{
	int diff;
	
	diff = (int)(myData->mData.misc.timer_1sec - myPs->netTimer);
	if(diff > myPs->config.netTimeout){
		userlog(DEBUG_LOG, psName,
		"netTimeout (netTime:%d > setValue:%d)\n", diff, myPs->config.netTimeout);
		return -1;
	}
	return 0;
}
void recovery_data(int p_ch, int bd, int ch)
{
	userlog(DEBUG_LOG, psName, "resultIndex : %d\n",myData->bData[bd].cData[ch].op.resultIndex);
	userlog(DEBUG_LOG, psName, "type : %d\n",myData->bData[bd].cData[ch].op.type);
	userlog(DEBUG_LOG, psName, "mode : %d\n",myData->bData[bd].cData[ch].op.mode);
	userlog(DEBUG_LOG, psName, "code : %d\n",myData->bData[bd].cData[ch].op.code);
	userlog(DEBUG_LOG, psName, "stepNo : %d\n",myData->bData[bd].cData[ch].op.stepNo);
	userlog(DEBUG_LOG, psName, "advStepNo : %d\n",myData->bData[bd].cData[ch].misc.advStepNo);
	userlog(DEBUG_LOG, psName, "runTime : %d\n",myData->bData[bd].cData[ch].op.runTime);
	userlog(DEBUG_LOG, psName, "totalRunTime : %d\n",myData->bData[bd].cData[ch].op.totalRunTime);
	userlog(DEBUG_LOG, psName, "z : %d\n",myData->bData[bd].cData[ch].op.z);
	userlog(DEBUG_LOG, psName, "reservedCmd : %d\n",myData->bData[bd].cData[ch].op.reservedCmd);
	userlog(DEBUG_LOG, psName, "totalCycle : %d\n",myData->bData[bd].cData[ch].misc.totalCycle);
	userlog(DEBUG_LOG, psName, "currentCycle : %d\n",myData->bData[bd].cData[ch].misc.currentCycle);
	userlog(DEBUG_LOG, psName, "meanVolt : %d\n",myData->bData[bd].cData[ch].op.meanVolt);
	userlog(DEBUG_LOG, psName, "meanCurr : %d\n",myData->bData[bd].cData[ch].op.meanCurr);
	userlog(DEBUG_LOG, psName, "charge_ampareHour : %d\n",myData->bData[bd].cData[ch].op.charge_ampareHour);
	userlog(DEBUG_LOG, psName, "discharge_ampareHour : %d\n",myData->bData[bd].cData[ch].op.discharge_ampareHour);
	userlog(DEBUG_LOG, psName, "watt : %d\n",myData->bData[bd].cData[ch].op.watt);
	userlog(DEBUG_LOG, psName, "charge_wattHour : %d\n",myData->bData[bd].cData[ch].op.charge_wattHour);
	userlog(DEBUG_LOG, psName, "discharge_wattHour : %d\n",myData->bData[bd].cData[ch].op.discharge_wattHour);
	userlog(DEBUG_LOG, psName, "cycleRunTime : %d\n",myData->bData[bd].cData[ch].misc.cycleRunTime);
	userlog(DEBUG_LOG, psName, "temp : %d\n",myData->bData[bd].cData[ch].op.temp);
	userlog(DEBUG_LOG, psName, "minT : %d\n",myData->bData[bd].cData[ch].misc.minT);
	userlog(DEBUG_LOG, psName, "maxT : %d\n",myData->bData[bd].cData[ch].misc.maxT);
	userlog(DEBUG_LOG, psName, "meanTemp : %d\n",myData->bData[bd].cData[ch].op.meanTemp);
	userlog(DEBUG_LOG, psName, "gotoCycleCount[%d] : %d\n",
		myData->bData[bd].cData[ch].op.stepNo,
		myData->bData[bd].cData[ch].misc.gotoCycleCount[myData->bData[bd].cData[ch].op.stepNo]);
	userlog(DEBUG_LOG, psName, "cvTime : %d\n",myData->bData[bd].cData[ch].misc.cvTime);
	userlog(DEBUG_LOG, psName, "integral_ampareHour : %d\n",myData->bData[bd].cData[ch].op.integral_ampareHour);
	userlog(DEBUG_LOG, psName, "integral_WattHour : %d\n",myData->bData[bd].cData[ch].op.integral_WattHour);
	userlog(DEBUG_LOG, psName, "ccTime : %d\n",myData->bData[bd].cData[ch].misc.ccTime);
	userlog(DEBUG_LOG, psName, "chargeCCAh : %d\n",myData->bData[bd].cData[ch].misc.chargeCCAh);
	userlog(DEBUG_LOG, psName, "chargeCVAh : %d\n",myData->bData[bd].cData[ch].misc.chargeCVAh);
	userlog(DEBUG_LOG, psName, "dischargeCCAh : %d\n",myData->bData[bd].cData[ch].misc.dischargeCCAh);
	userlog(DEBUG_LOG, psName, "dischargeCVAh : %d\n",myData->bData[bd].cData[ch].misc.dischargeCVAh);
	userlog(DEBUG_LOG, psName, "startV : %d\n",myData->bData[bd].cData[ch].misc.startV);
	userlog(DEBUG_LOG, psName, "step_count : %d\n",myData->bData[bd].cData[ch].misc.step_count);
	userlog(DEBUG_LOG, psName, "maxV : %d\n",myData->bData[bd].cData[ch].misc.maxV);
	userlog(DEBUG_LOG, psName, "minV : %d\n",myData->bData[bd].cData[ch].misc.minV);

	userlog(DEBUG_LOG, psName, "advCycle : %d\n",myData->bData[bd].cData[ch].misc.advCycle);
	userlog(DEBUG_LOG, psName, "seedintegralCapacity : %d\n",myData->bData[bd].cData[ch].misc.seedintegralCapacity);
	userlog(DEBUG_LOG, psName, "sumintegralCapacity : %d\n",myData->bData[bd].cData[ch].misc.sumintegralCapacity);
	userlog(DEBUG_LOG, psName, "seedintegralWattHour : %d\n",myData->bData[bd].cData[ch].misc.seedintegralWattHour);
	userlog(DEBUG_LOG, psName, "sumintegralWattHour : %d\n",myData->bData[bd].cData[ch].misc.sumintegralWattHour);
	userlog(DEBUG_LOG, psName, "seedChargeAmpareHour : %d\n",myData->bData[bd].cData[ch].misc.seedChargeAmpareHour);
	userlog(DEBUG_LOG, psName, "sumChargeAmpareHour : %d\n",myData->bData[bd].cData[ch].misc.sumChargeAmpareHour);
	userlog(DEBUG_LOG, psName, "seedDischargeAmpareHour : %d\n",myData->bData[bd].cData[ch].misc.seedDischargeAmpareHour);
	userlog(DEBUG_LOG, psName, "sumDischargeAmpareHour : %d\n",myData->bData[bd].cData[ch].misc.sumDischargeAmpareHour);
	userlog(DEBUG_LOG, psName, "seedChargeWattHour : %d\n",myData->bData[bd].cData[ch].misc.seedChargeWattHour);
	userlog(DEBUG_LOG, psName, "sumChargeWattHour : %d\n",myData->bData[bd].cData[ch].misc.sumChargeWattHour);
	userlog(DEBUG_LOG, psName, "seedDischargeWattHour : %d\n",myData->bData[bd].cData[ch].misc.seedDischargeWattHour);
	userlog(DEBUG_LOG, psName, "sumDischargeWattHour : %d\n",myData->bData[bd].cData[ch].misc.sumDischargeWattHour);
	userlog(DEBUG_LOG, psName, "standardC : %d\n",myData->bData[bd].cData[ch].misc.standardC);
	userlog(DEBUG_LOG, psName, "standardP : %d\n",myData->bData[bd].cData[ch].misc.standardP);
	userlog(DEBUG_LOG, psName, "standardZ : %d\n",myData->bData[bd].cData[ch].misc.standardZ);
	userlog(DEBUG_LOG, psName, "cycleSumC : %d\n",myData->bData[bd].cData[ch].misc.cycleSumC);
	userlog(DEBUG_LOG, psName, "cycleSumP : %d\n",myData->bData[bd].cData[ch].misc.cycleSumP);
	userlog(DEBUG_LOG, psName, "cycleEndC : %d\n",myData->bData[bd].cData[ch].misc.cycleEndC);
	userlog(DEBUG_LOG, psName, "pattern_change_flag : %d\n",myData->bData[bd].cData[ch].misc.pattern_change_flag);

	userlog(DEBUG_LOG, psName, "chGroupNo : %d\n",myData->bData[bd].cData[ch].misc.chGroupNo);
	userlog(DEBUG_LOG, psName, "tempDir : %d\n",myData->bData[bd].cData[ch].misc.tempDir);
	userlog(DEBUG_LOG, psName, "sumChargeCCAh : %d\n",myData->bData[bd].cData[ch].misc.sumChargeCCAh);
	userlog(DEBUG_LOG, psName, "seedChargeCCAh : %d\n",myData->bData[bd].cData[ch].misc.seedChargeCCAh);
	userlog(DEBUG_LOG, psName, "sumChargeCVAh : %d\n",myData->bData[bd].cData[ch].misc.sumChargeCVAh);
	userlog(DEBUG_LOG, psName, "seedChargeCVAh : %d\n",myData->bData[bd].cData[ch].misc.seedChargeCVAh);
	userlog(DEBUG_LOG, psName, "sumChargeCCCVAh : %d\n",myData->bData[bd].cData[ch].misc.sumChargeCCCVAh);
	userlog(DEBUG_LOG, psName, "seedChargeCCCVAh : %d\n",myData->bData[bd].cData[ch].misc.seedChargeCCCVAh);
	userlog(DEBUG_LOG, psName, "sumDischargeCCAh : %d\n",myData->bData[bd].cData[ch].misc.sumDischargeCCAh);
	userlog(DEBUG_LOG, psName, "seedDischargeCCAh : %d\n",myData->bData[bd].cData[ch].misc.seedDischargeCCAh);
	userlog(DEBUG_LOG, psName, "sumDischargeCVAh : %d\n",myData->bData[bd].cData[ch].misc.sumDischargeCVAh);
	userlog(DEBUG_LOG, psName, "seedDischargeCVAh : %d\n",myData->bData[bd].cData[ch].misc.seedDischargeCVAh);
	userlog(DEBUG_LOG, psName, "sumDischargeCCCVAh : %d\n",myData->bData[bd].cData[ch].misc.sumDischargeCCCVAh);
	userlog(DEBUG_LOG, psName, "seedDischargeCCCVAh : %d\n",myData->bData[bd].cData[ch].misc.seedDischargeCCCVAh);
	userlog(DEBUG_LOG, psName, "chargeCCAh : %d\n",myData->bData[bd].cData[ch].misc.chargeCCAh);
	userlog(DEBUG_LOG, psName, "chargeCVAh : %d\n",myData->bData[bd].cData[ch].misc.chargeCVAh);
	userlog(DEBUG_LOG, psName, "chargeCCCVAh : %d\n",myData->bData[bd].cData[ch].misc.chargeCCCVAh);
	userlog(DEBUG_LOG, psName, "dischargeCCAh : %d\n",myData->bData[bd].cData[ch].misc.dischargeCCAh);
	userlog(DEBUG_LOG, psName, "dischargeCVAh : %d\n",myData->bData[bd].cData[ch].misc.dischargeCVAh);
	userlog(DEBUG_LOG, psName, "dischargeCCCVAh : %d\n",myData->bData[bd].cData[ch].misc.dischargeCCCVAh);
	userlog(DEBUG_LOG, psName, "cycleSumChargeWatt : %d\n",myData->bData[bd].cData[ch].misc.cycleSumChargeWatt);
	userlog(DEBUG_LOG, psName, "cycleSumChargeWattHour : %d\n",myData->bData[bd].cData[ch].misc.cycleSumChargeWattHour);
	userlog(DEBUG_LOG, psName, "cycleSumChargeAmpareHour : %d\n",myData->bData[bd].cData[ch].misc.cycleSumChargeAmpareHour);
	userlog(DEBUG_LOG, psName, "cycleSumDischargeWatt : %d\n",myData->bData[bd].cData[ch].misc.cycleSumDischargeWatt);
	userlog(DEBUG_LOG, psName, "cycleSumDischargeWattHour : %d\n",myData->bData[bd].cData[ch].misc.cycleSumDischargeWattHour);
	userlog(DEBUG_LOG, psName, "cycleSumDischargeAmpareHour : %d\n",myData->bData[bd].cData[ch].misc.cycleSumDischargeAmpareHour);
	userlog(DEBUG_LOG, psName, "cycleStepCount : %d\n",myData->bData[bd].cData[ch].misc.cycleStepCount);
	userlog(DEBUG_LOG, psName, "cycleSumAvgT : %d\n",myData->bData[bd].cData[ch].misc.cycleSumAvgT);
	userlog(DEBUG_LOG, psName, "fileIndex : %d\n",myData->DataSave.resultData[p_ch].fileIndex);
}	
