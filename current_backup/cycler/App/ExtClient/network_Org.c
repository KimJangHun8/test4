#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/ioctl.h>
#include "../../INC/datastore.h"
#include "userlog.h"
#include "common_utils.h"
#include "local_utils.h"
#include "message.h"
#include "com_io.h"
#include "com_socket.h"
#include "network.h"

extern volatile S_SYSTEM_DATA *myData;
extern volatile S_EXT_CLIENT *myPs;
extern char psName[PROCESS_NAME_SIZE];

int InitNetwork(void)
{
	sleep(2);

	if(myPs->config.network_socket > 0)
		close(myPs->config.network_socket);
	
	myPs->config.network_socket
		= SetClientSock(myPs->config.networkPort,
						(char *)&myPs->config.ipAddr);
    if(myPs->config.network_socket < 0) {
		close(myPs->config.network_socket);
	    userlog(DEBUG_LOG, psName,
			"Can not initialize network(network Port) : %d\n",
			myPs->config.network_socket);
		return -2;
	}

	myPs->signal[EXT_SIG_NET_CONNECTED] = P1;
	myPs->pingTimer = myData->mData.misc.timer_1sec;
	myPs->netTimer = myData->mData.misc.timer_1sec;

	userlog(DEBUG_LOG, psName,
		"command socket connected network_socket%d)\n",
		myPs->config.network_socket);
	
	return 0;
}

int NetworkPacket_Receive(void)
{
	int rcv_size, read_size, i, start, index;
	char maxPacketBuf[MAX_EXT_CLIENT_PACKET_LENGTH];

	memset(maxPacketBuf, 0x00, MAX_EXT_CLIENT_PACKET_LENGTH);
		
	if(ioctl(myPs->config.network_socket, FIONREAD, &rcv_size) < 0) {
		userlog(DEBUG_LOG, psName,
			"packet receive ioctl error\n");
		return -1;
	}

	if(rcv_size > MAX_EXT_CLIENT_PACKET_LENGTH) {
		userlog(DEBUG_LOG, psName,
			"max packet size over\n");
		read_size = readn(myPs->config.network_socket, maxPacketBuf,
			MAX_EXT_CLIENT_PACKET_LENGTH);
		if(read_size != MAX_EXT_CLIENT_PACKET_LENGTH)
			userlog(DEBUG_LOG, psName,
				"packet readn size error1\n");
		return -2;
	} else if(rcv_size > (MAX_EXT_CLIENT_PACKET_LENGTH - myPs->rcvPacket.usedBufSize)) {
		userlog(DEBUG_LOG, psName,
			"packet buffer overflow\n");
		read_size = readn(myPs->config.network_socket, maxPacketBuf, rcv_size);
		if(read_size != rcv_size)
			userlog(DEBUG_LOG, psName,
				"packet readn size error2\n");
		return -3;
	} else if(rcv_size <= 0) {
		userlog(DEBUG_LOG, psName,
			"packet sock_rcv error %d\n", rcv_size);
//		send_msg(EXT_TO_APP, MAS_EXT_APP_PROCESS_KILL, 0, 0); //khk test
		return -4;
	} else {
		read_size = readn(myPs->config.network_socket, maxPacketBuf, rcv_size);
		if(read_size != rcv_size) {
			userlog(DEBUG_LOG, psName,
				"packet readn size error3 : %d, %d\n", read_size, rcv_size);
			return -5;
		}
	}
	
	/*userlog(DEBUG_LOG, psName,
		"recvCmd %s\n", maxPacketBuf);*/

	i = myPs->rcvPacket.rcvCount;
	myPs->rcvPacket.rcvCount++;
	if(myPs->rcvPacket.rcvCount > (MAX_MAIN_PACKET_COUNT-1))
		myPs->rcvPacket.rcvCount = 0;
	
	if(i == 0) index = MAX_MAIN_PACKET_COUNT - 1;
	else index = i - 1;
	start = myPs->rcvPacket.rcvStartPoint[index]
		+ myPs->rcvPacket.rcvSize[index];
	if(start >= MAX_EXT_CLIENT_PACKET_LENGTH) {
		myPs->rcvPacket.rcvStartPoint[i] = abs(start - MAX_EXT_CLIENT_PACKET_LENGTH);
	} else {
		myPs->rcvPacket.rcvStartPoint[i] = start;
	}

	myPs->rcvPacket.rcvSize[i] = read_size;
	myPs->rcvPacket.usedBufSize += read_size;
	
	start = myPs->rcvPacket.rcvStartPoint[i];
	if((start + read_size) > MAX_EXT_CLIENT_PACKET_LENGTH) {
		index = MAX_EXT_CLIENT_PACKET_LENGTH - start;
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

	j = start_point + myPs->rcvPacket.rcvSize[i];
	if(j <= MAX_EXT_CLIENT_PACKET_LENGTH) {
		memcpy((char *)&myPs->rcvCmd.cmdBuf[cmdBuf_index],
			(char *)&myPs->rcvPacket.rcvPacketBuf[start_point],
			myPs->rcvPacket.rcvSize[i]);
	} else {
		k = MAX_EXT_CLIENT_PACKET_LENGTH - start_point;
		memcpy((char *)&myPs->rcvCmd.cmdBuf[cmdBuf_index],
			(char *)&myPs->rcvPacket.rcvPacketBuf[start_point], k);
		cmdBuf_index += k;
		start_point = 0;
		k = j - MAX_EXT_CLIENT_PACKET_LENGTH;
		memcpy((char *)&myPs->rcvCmd.cmdBuf[cmdBuf_index],
			(char *)&myPs->rcvPacket.rcvPacketBuf[start_point], k);
	}
		
	start_point = myPs->rcvPacket.parseStartPoint[i]
		+ myPs->rcvPacket.rcvSize[i];
	if(start_point >= MAX_EXT_CLIENT_PACKET_LENGTH) {
		j = i + 1;
		if(j >= MAX_MAIN_PACKET_COUNT) j = 0;
		myPs->rcvPacket.parseStartPoint[j]
			= abs(start_point - MAX_EXT_CLIENT_PACKET_LENGTH);
	} else {
		j = i + 1;
		if(j >= MAX_MAIN_PACKET_COUNT) j = 0;
		myPs->rcvPacket.parseStartPoint[j] = start_point;
	}
		
	myPs->rcvPacket.usedBufSize -= myPs->rcvPacket.rcvSize[i];
}

int NetworkCommand_Receive(void)
{
	int cmd_size, cmdBuf_index;
	int	i, flag;
	S_EXT_CLIENT_CMD_HEADER header;

	if(myPs->rcvCmd.cmdBufSize < sizeof(header)) return -1;
	
	if(myPs->rcvCmd.cmdBuf[0] != EXT_CLIENT_STX) {
		userlog(DEBUG_LOG, psName, "packet STX error\n");	
		return -2; 
		//socket close?
		//buffer init?
	}
	
	flag = 0;	
	for(i=0; i < myPs->rcvCmd.cmdBufSize; i++) {
		if(myPs->rcvCmd.cmdBuf[i] == EXT_CLIENT_ETX) {
			flag = 1;
			break;
		}
	}
	// buf is full (rest packet 0 or a little)i
	// if rest packet is 0 : rcvCmd.cmdBufSize = 0;
	// else rcvCmd.cmdBufSize = rest Size;
	// copy rest packet to &rcvCmd.cmdBuf[0]  
	if(flag == 1) {
		cmd_size = i+1;	
		memset((char *)&myPs->rcvCmd.cmd[0], 0x00, MAX_EXT_CLIENT_PACKET_LENGTH);
		memcpy((char *)&myPs->rcvCmd.cmd[0],
			(char *)&myPs->rcvCmd.cmdBuf[0], cmd_size);
		myPs->rcvCmd.cmdSize = cmd_size;
	
		cmdBuf_index = cmd_size;
		myPs->rcvCmd.cmdBufSize -= cmd_size;
		cmd_size = myPs->rcvCmd.cmdBufSize;
		memset((char *)&myPs->rcvCmd.tmpBuf[0], 0x00, MAX_EXT_CLIENT_PACKET_LENGTH);
		memcpy((char *)&myPs->rcvCmd.tmpBuf[0],
			(char *)&myPs->rcvCmd.cmdBuf[cmdBuf_index], cmd_size);
		memset((char *)&myPs->rcvCmd.cmdBuf[0], 0x00, MAX_EXT_CLIENT_PACKET_LENGTH);
		memcpy((char *)&myPs->rcvCmd.cmdBuf[0],
			(char *)&myPs->rcvCmd.tmpBuf[0], cmd_size);
	} else { //buf is not full (wait 1 more socket event)
		return -3;
	}
	
	return 0;
}

int NetworkCommand_Parsing(void)
{
	char			buf[12], tmp[12];
	char			body[MAX_PACKET_LENGTH], ext_log[MAX_PACKET_LENGTH];
	int				cmdid, rtn, i, size;
	S_EXT_CLIENT_CMD_HEADER	header;

	memset((char *)&header, 0x00, sizeof(header));
	memcpy((char *)&header, (char *)&myPs->rcvCmd.cmd[1],
		sizeof(header));
	
	memset(buf, 0x00, sizeof(buf));
	memcpy((char *)&buf[0], (char *)&header.cmdid[0], sizeof(header.cmdid));
	cmdid = atoi(buf);
	
	switch(cmdid){
		case PC_TO_SBC_CMD_CH_DATA_RESPONSE:
			break;
		default:
			memset(buf, 0x00, sizeof(buf));
			memcpy(buf, (char *)&myPs->rcvCmd.cmd[1], sizeof(header));

			size = sizeof(header)+1;	
			memset(body, 0x00, sizeof(MAX_PACKET_LENGTH));
			memset(tmp, 0x00, sizeof(tmp));
			for(i=size; i<MAX_PACKET_LENGTH; i++) {
				if(myPs->rcvCmd.cmd[i] == EXT_CLIENT_ETX) {
					memcpy(tmp, (char *)&body[i-size-2], 2);
					body[i-size-2]=0;	
					body[i-size-1]=0;	
					body[i-size] = 0;
					break;
				}
				body[i-size] = myPs->rcvCmd.cmd[i];
			}
			memset(ext_log, 0x00, sizeof(MAX_PACKET_LENGTH));
			sprintf(ext_log, "[H->M] CmdID=%3d Header=%s Body=%s CRC=%x%x",
				cmdid, (char *)&buf, (char *)&body, (unsigned char)tmp[0], (unsigned char)tmp[1]);

			userlog(EXT_LOG, psName, "%s\n",(char *)&ext_log);
			break;
	}
	
	rtn = CmdHeaderCheck((char *)&header);
	if(rtn < 0)	{
		userlog(DEBUG_LOG, psName, 
			"EXT Client CmdHeaderCheck error %d\n", rtn);
		return 0;
	}
	
//	rtn = CheckReplyCmd(&header); 
//	if(rtn < 0) return -2;

	switch(cmdid) {
		case PC_TO_SBC_CMD_HEARTBEAT_RESPONSE:
			rtn = rcv_cmd_heartbeat_response();
			break;
		case PC_TO_SBC_CMD_MODULE_INFO_RESPONSE:
			rtn = rcv_cmd_response();
			break;
		case PC_TO_SBC_CMD_STEP_DATA:
			rtn = rcv_cmd_step_data();
			break;
		case PC_TO_SBC_CMD_RUN:
			rtn = rcv_cmd_run();
			myData->mData.config.sendAuxFlag = P1;
			break;
		case PC_TO_SBC_CMD_STOP:
			rtn = rcv_cmd_stop();
			break;
		case PC_TO_SBC_CMD_PAUSE:
			rtn = rcv_cmd_pause();
			break;
		case PC_TO_SBC_CMD_CONTINUE:
			rtn = rcv_cmd_continue();
			break;
		case PC_TO_SBC_CMD_NEXTSTEP:
			rtn = rcv_cmd_nextstep();
			break;
		case PC_TO_SBC_CMD_INIT:
			rtn = rcv_cmd_init();
			break;
		case PC_TO_SBC_CMD_PARALLEL:
			rtn = rcv_cmd_parallel();
			break;
		case PC_TO_SBC_CMD_CH_DATA_RESPONSE:
			rtn = rcv_cmd_response();
			break;
		case PC_TO_SBC_CMD_CRC_ERROR:
			rtn = rcv_cmd_crc_error();
			break;
		case PC_TO_SBC_CMD_CRC_ERROR_RESPONSE:
			userlog(DEBUG_LOG, psName, 
				"Rcv Cmd CRC ERROR RESPONSE\n");
			rtn = 0;
			break;
		case PC_TO_SBC_CMD_UNKNOWN_CMD: 
			 rtn = rcv_cmd_unknown();
			 userlog(DEBUG_LOG, psName, 
					"Rcv Unknown Cmd[%d]\n",cmdid);
			rtn = 0;
			break;
		case PC_TO_SBC_CMD_UNKNOWN_CMD_RESPONSE: 
			userlog(DEBUG_LOG, psName, 
				"Rcv Unknown Cmd Response\n");
			rtn = 0;
			break;
		default:
			userlog(DEBUG_LOG, psName, 
				"Can't Find Cmd[%d]\n",cmdid);
			rtn = -1;
			break;
	}
	if(rtn < 0) return -3;
	else return 0;
}

int Parsing_NetworkEvent(void)
{
	int rtn=0;

	NetworkPacket_Parsing();
	if(NetworkCommand_Receive() >= 0) {
		if(NetworkCommand_Parsing() < 0) {
			myPs->rcvCmd.cmdFail++;
			if(myPs->rcvCmd.cmdFail >= 3) {
				myPs->rcvCmd.cmdFail = 0;
				myPs->rcvCmd.cmdBufSize = 0;
				memset((char *)&myPs->rcvCmd.cmdBuf[0], 0x00,
					MAX_EXT_CLIENT_PACKET_LENGTH);
				rtn = -1;
			}
		} else {
			myPs->rcvCmd.cmdFail = 0;
		}
	}
	
	return rtn;
}


int CmdHeaderCheck(char *rcvHeader)
{
	int cmd_id, len, rtn;
	char buf[12];
	S_EXT_CLIENT_CMD_HEADER	header;
	
	memset((char *)&header, 0x00, sizeof(header));
	memcpy((char *)&header, (char *)rcvHeader, sizeof(header));
	memset(buf, 0x00, sizeof(buf));
	memcpy((char *)&buf[0], (char *)&header.cmdid[0], sizeof(header.cmdid));
	cmd_id = atoi(buf);
	
	switch(cmd_id) {
		case PC_TO_SBC_CMD_HEARTBEAT_RESPONSE://102
			len = sizeof(S_EXT_CLIENT_RCV_CMD_RESPONSE);
			break;
		case PC_TO_SBC_CMD_MODULE_INFO_RESPONSE://104
			len = sizeof(S_EXT_CLIENT_RCV_CMD_RESPONSE);
			break;
		case PC_TO_SBC_CMD_STEP_DATA://151
			len = sizeof(S_EXT_CLIENT_RCV_CMD_STEP_DATA);
			break;
		case PC_TO_SBC_CMD_RUN://201
			len = sizeof(S_EXT_CLIENT_RCV_CMD_RUN);
			break;
		case PC_TO_SBC_CMD_STOP://203
			len = sizeof(S_EXT_CLIENT_RCV_CMD_STOP);
			break;
		case PC_TO_SBC_CMD_PAUSE://205
			len = sizeof(S_EXT_CLIENT_RCV_CMD_PAUSE);
			break;
		case PC_TO_SBC_CMD_CONTINUE://207
			len = sizeof(S_EXT_CLIENT_RCV_CMD_CONTINUE);
			break;
		case PC_TO_SBC_CMD_NEXTSTEP://209
			len = sizeof(S_EXT_CLIENT_RCV_CMD_NEXTSTEP);
			break;
		case PC_TO_SBC_CMD_INIT://211
			len = sizeof(S_EXT_CLIENT_RCV_CMD_INIT);
			break;
		case PC_TO_SBC_CMD_PARALLEL://213
			len = sizeof(S_EXT_CLIENT_RCV_CMD_PARALLEL);
			break;
		case PC_TO_SBC_CMD_CH_DATA_RESPONSE://252
			len = sizeof(S_EXT_CLIENT_RCV_CMD_RESPONSE);
			break;
		case PC_TO_SBC_CMD_UNKNOWN_CMD://901
			len = sizeof(S_EXT_CLIENT_RCV_CMD_UNKNOWN_CMD);
			 userlog(DEBUG_LOG, psName, 
					"Rcv Unknown Cmd[%d]\n",cmd_id);
			rtn = 0;
			break;
		case PC_TO_SBC_CMD_UNKNOWN_CMD_RESPONSE://902
			len = sizeof(S_EXT_CLIENT_RCV_CMD_RESPONSE);
			break;
		case PC_TO_SBC_CMD_CRC_ERROR://903
			len = sizeof(S_EXT_CLIENT_RCV_CMD_CRC_ERROR);
			break;
		case PC_TO_SBC_CMD_CRC_ERROR_RESPONSE://904
			len = sizeof(S_EXT_CLIENT_RCV_CMD_RESPONSE);
			break;
		case PC_TO_SBC_CMD_DATA_SIZE_ERROR://905
			len = sizeof(S_EXT_CLIENT_RCV_CMD_DATA_SIZE_ERROR);
			break;
		case PC_TO_SBC_CMD_TROUBLE://906
			len = sizeof(S_EXT_CLIENT_RCV_CMD_TROUBLE);
			break;
		default:
				send_cmd_unknown(SBC_TO_PC_CMD_UNKNOWN_CMD, EXT_REPLY_NO);
				userlog(DEBUG_LOG, psName, 
					"ExtClient RcvCmd command id error : %d\n", cmd_id);
			rtn = -1;
			break;
	}

	len += 2;

	if(len != myPs->rcvCmd.cmdSize) {
		send_cmd_unknown(SBC_TO_PC_CMD_DATA_SIZE_ERROR, EXT_REPLY_NO);
		userlog(DEBUG_LOG, psName, 
			"ExtClient RcvCmd Data Size error (%d : %d)\n",
		len, myPs->rcvCmd.cmdSize);
		return -8;
	}
	return 0;
}

/*
int CheckReplyCmd(S_EXT_CLIENT_CMD_HEADER *header)
{
	int		seqno, cmdid, rtn;
	char	buf[12];
	
	rtn = 0;

	memset(buf, 0x00, sizeof buf);
	memcpy(buf, (char *)&header->cmdid[0], sizeof(header->cmdid));
	cmdid = atoi(buf);
	
	switch(header->boxid) { // boxid => 1 (fix)
		case '1':
			if(myPs->reply.timer_run == P1) {
				if(myPs->reply.retry.seqno == seqno
				&& myPs->reply.retry.replyCmd == cmdid) {
					myPs->reply.timer_run = P0;
				}
			}
			break;
		default:
			userlog(DEBUG_LOG, psName, 
				"box id is invalid : %c\n", header->boxid);
			rtn = -1;
			break;
	}
	return rtn;
}
*/

void make_header(char *buf, char reply, int cmd_id, int ch, int data_size)
{
	char temp[12];
	S_EXT_CLIENT_CMD_HEADER	header;
	
	memset((char *)&header, 0x00, sizeof(header));

	sprintf((char *)&header.cmdid, "%03d", cmd_id);

	memset(temp, 0x00, sizeof(temp));
	sprintf(temp, "%1d", myData->AppControl.config.moduleNo);
	memcpy((char *)&header.moduleNo, temp, 1);

	memset(temp, 0x00, sizeof(temp));
	sprintf(temp, "%02d", ch);
	memcpy((char *)&header.chNo, temp, sizeof(header.chNo));

	if(reply == EXT_REPLY_YES)
		header.reply = 'Y';
	else
		header.reply = 'N';

	memset(temp, 0x00, sizeof(temp));
	sprintf(temp, "%03d", data_size);
	memcpy((char *)&header.data_size, temp, sizeof(header.data_size));

	memcpy(buf, (char *)&header, sizeof(S_EXT_CLIENT_CMD_HEADER));
}

int	send_command(char *cmd, int cmd_size)
{
	char	packet[MAX_EXT_CLIENT_PACKET_LENGTH];
	char	body[MAX_EXT_CLIENT_PACKET_LENGTH];
	char	ext_log[MAX_EXT_CLIENT_PACKET_LENGTH];
	char 	buf[12], tmp[12], header1[20];
	int i, rtn, cmdid, size;
	S_EXT_CLIENT_CMD_HEADER header;

	memset((char *)&header, 0x00, sizeof(S_EXT_CLIENT_CMD_HEADER));
	memcpy((char *)&header, cmd, sizeof(S_EXT_CLIENT_CMD_HEADER));

	memset((char *)&header1, 0x00, sizeof(header1));
	memcpy((char *)&header1, cmd, sizeof(S_EXT_CLIENT_CMD_HEADER));
	
	if(cmd_size > MAX_EXT_CLIENT_PACKET_LENGTH-2) {
		userlog(DEBUG_LOG, psName,
			"CMD SEND FAIL!! TOO LARGE SIZE[%d]\n", cmd_size + 2);
		return -1;
	}
	
	memset((char *)&packet, 0x00, sizeof(packet));
	packet[0] = EXT_CLIENT_STX;
	memcpy((char*)packet + 1, cmd, cmd_size);
	packet[cmd_size + 1] = EXT_CLIENT_ETX;

	rtn = writen(myPs->config.network_socket, packet, cmd_size + 2);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd[%d] send error!!!\n",
			header.cmdid);
		return rtn;
	}

	if(myPs->config.CmdSendLog == P1) {
		size = sizeof(S_EXT_CLIENT_CMD_HEADER)+1;
		memset(body, 0x00, sizeof(MAX_EXT_CLIENT_PACKET_LENGTH));
		memset(tmp, 0x00, sizeof(tmp));
		for(i=size; i<MAX_EXT_CLIENT_PACKET_LENGTH; i++) {
			if(packet[i] == EXT_CLIENT_ETX) {
				memcpy(tmp, (char *)&body[i-size-2], 2);
				body[i-size-2] = 0;
				body[i-size-1] = 0;
				body[i-size] = 0;
				break;
			}
			body[i-size] = packet[i];
		}
		memset(ext_log, 0x00, sizeof(MAX_EXT_CLIENT_PACKET_LENGTH));
		memset(buf, 0x00, sizeof(buf));
		memcpy(buf, (char *)&header.cmdid, sizeof(header.cmdid));
		cmdid = atoi(buf);

		switch(cmdid){
			case SBC_TO_PC_CMD_CH_DATA:
				break;
			default:
				sprintf(ext_log, "[H<-M] CmdID=%3d Header=%s Body=%s CRC=%x%x",
					cmdid, (char *)&header1, (char *)&body, (unsigned char)tmp[0], (unsigned char)tmp[1]);
					userlog(EXT_LOG, psName, "%s\n", (char *)&ext_log);
				break;
		}

	}
	return 0;
}

void make_crc(unsigned short incrc16, char *buf, unsigned int buflen)
{
	int i = 0;
	if(myPs->config.crc_type == P1){
		i = crc16polynomail_buf(incrc16, &buf[0], buflen);
	}else if(myPs->config.crc_type == P2){
		i = crc16ccitt_compute_buf(incrc16, &buf[0], buflen);
	}else if(myPs->config.crc_type == P3){
		i = crc16modbus_compute_buf(incrc16, &buf[0], buflen);
	}else{
	}
	buf[buflen] = (unsigned char)(i);
	buf[buflen + 1] = (unsigned char)(i >> 8);
//	userlog(DEBUG_LOG, psName, "Make CRC : %x, %x \n", (unsigned char)buf[buflen], (unsigned char)buf[buflen + 1]);
}

int check_crc(unsigned short incrc16, char *buf, unsigned int buflen)
{
	int i;
	if(myPs->config.crc_type == P1){
		i = crc16polynomail_buf(incrc16, &buf[0], buflen);
	}else if(myPs->config.crc_type == P2){
		i = crc16ccitt_compute_buf(incrc16, &buf[0], buflen);
	}else if(myPs->config.crc_type == P3){
		i = crc16modbus_compute_buf(incrc16, &buf[0], buflen);
	}else{
		return 0;
	}
/*	userlog(DEBUG_LOG, psName, "CRC Rcv Cmd Check CRC : %x, %x, Check CRC : %x, %x \n", (unsigned char)buf[buflen], (unsigned char)buf[buflen + 1], (unsigned char)i, (unsigned char)(i>>8));
*/
	if((unsigned char)buf[buflen] != (unsigned char)(i) ||
		(unsigned char)buf[buflen + 1] != (unsigned char)(i >> 8)) {
		userlog(DEBUG_LOG, psName, "Rcv Cmd CRC Error, Rcv CRC : %x, %x, Check CRC : %x, %x \n", (unsigned char)buf[buflen], (unsigned char)buf[buflen + 1], (unsigned char)i, (unsigned char)(i>>8));
		return -1;
	}
	return 0;
}


// Compute CRC-16-CCITT for buf; incrc16 - input value (must be 0 for the first call)
unsigned short crc16ccitt_compute_buf(unsigned short incrc16, char *buf, unsigned int buflen)
{
  unsigned int crc16tab[256]={
  0x0000,0x1021,0x2042,0x3063,0x4084,0x50a5,0x60c6,0x70e7,
  0x8108,0x9129,0xa14a,0xb16b,0xc18c,0xd1ad,0xe1ce,0xf1ef,
  0x1231,0x0210,0x3273,0x2252,0x52b5,0x4294,0x72f7,0x62d6,
  0x9339,0x8318,0xb37b,0xa35a,0xd3bd,0xc39c,0xf3ff,0xe3de,
  0x2462,0x3443,0x0420,0x1401,0x64e6,0x74c7,0x44a4,0x5485,
  0xa56a,0xb54b,0x8528,0x9509,0xe5ee,0xf5cf,0xc5ac,0xd58d,
  0x3653,0x2672,0x1611,0x0630,0x76d7,0x66f6,0x5695,0x46b4,
  0xb75b,0xa77a,0x9719,0x8738,0xf7df,0xe7fe,0xd79d,0xc7bc,
  0x48c4,0x58e5,0x6886,0x78a7,0x0840,0x1861,0x2802,0x3823,
  0xc9cc,0xd9ed,0xe98e,0xf9af,0x8948,0x9969,0xa90a,0xb92b,
  0x5af5,0x4ad4,0x7ab7,0x6a96,0x1a71,0x0a50,0x3a33,0x2a12,
  0xdbfd,0xcbdc,0xfbbf,0xeb9e,0x9b79,0x8b58,0xbb3b,0xab1a,
  0x6ca6,0x7c87,0x4ce4,0x5cc5,0x2c22,0x3c03,0x0c60,0x1c41,
  0xedae,0xfd8f,0xcdec,0xddcd,0xad2a,0xbd0b,0x8d68,0x9d49,
  0x7e97,0x6eb6,0x5ed5,0x4ef4,0x3e13,0x2e32,0x1e51,0x0e70,
  0xff9f,0xefbe,0xdfdd,0xcffc,0xbf1b,0xaf3a,0x9f59,0x8f78,
  0x9188,0x81a9,0xb1ca,0xa1eb,0xd10c,0xc12d,0xf14e,0xe16f,
  0x1080,0x00a1,0x30c2,0x20e3,0x5004,0x4025,0x7046,0x6067,
  0x83b9,0x9398,0xa3fb,0xb3da,0xc33d,0xd31c,0xe37f,0xf35e,
  0x02b1,0x1290,0x22f3,0x32d2,0x4235,0x5214,0x6277,0x7256,
  0xb5ea,0xa5cb,0x95a8,0x8589,0xf56e,0xe54f,0xd52c,0xc50d,
  0x34e2,0x24c3,0x14a0,0x0481,0x7466,0x6447,0x5424,0x4405,
  0xa7db,0xb7fa,0x8799,0x97b8,0xe75f,0xf77e,0xc71d,0xd73c,
  0x26d3,0x36f2,0x0691,0x16b0,0x6657,0x7676,0x4615,0x5634,
  0xd94c,0xc96d,0xf90e,0xe92f,0x99c8,0x89e9,0xb98a,0xa9ab,
  0x5844,0x4865,0x7806,0x6827,0x18c0,0x08e1,0x3882,0x28a3,
  0xcb7d,0xdb5c,0xeb3f,0xfb1e,0x8bf9,0x9bd8,0xabbb,0xbb9a,
  0x4a75,0x5a54,0x6a37,0x7a16,0x0af1,0x1ad0,0x2ab3,0x3a92,
  0xfd2e,0xed0f,0xdd6c,0xcd4d,0xbdaa,0xad8b,0x9de8,0x8dc9,
  0x7c26,0x6c07,0x5c64,0x4c45,0x3ca2,0x2c83,0x1ce0,0x0cc1,
  0xef1f,0xff3e,0xcf5d,0xdf7c,0xaf9b,0xbfba,0x8fd9,0x9ff8,
  0x6e17,0x7e36,0x4e55,0x5e74,0x2e93,0x3eb2,0x0ed1,0x1ef0};

  unsigned int i;
  unsigned char *buf1;
  unsigned short crc16;
  
  crc16=incrc16;
  buf1=(unsigned char *)buf;
  for(i=0; i< buflen; i++)
  {
    crc16=(crc16<<8)^crc16tab[((crc16>>8)^*buf1) & 0x00FF];
    buf1++;
  }  
  return(crc16);
}

unsigned short crc16polynomail_buf(unsigned short incrc16, char *buf, unsigned int buflen)
{
//POLYNOMIAL 0x8005
 unsigned short crc16tab[256] = {0x0000,
	0x8005, 0x800F, 0x000A, 0x801B, 0x001E, 0x0014, 0x8011,
	0x8033, 0x0036, 0x003C, 0x8039, 0x0028, 0x802D, 0x8027,
	0x0022, 0x8063, 0x0066, 0x006C, 0x8069, 0x0078, 0x807D,
	0x8077, 0x0072, 0x0050, 0x8055, 0x805F, 0x005A, 0x804B,
	0x004E, 0x0044, 0x8041, 0x80C3, 0x00C6, 0x00CC, 0x80C9,
	0x00D8, 0x80DD, 0x80D7, 0x00D2, 0x00F0, 0x80F5, 0x80FF,
	0x00FA, 0x80EB, 0x00EE, 0x00E4, 0x80E1, 0x00A0, 0x80A5,
	0x80AF, 0x00AA, 0x80BB, 0x00BE, 0x00B4, 0x80B1, 0x8093,
	0x0096, 0x009C, 0x8099, 0x0088, 0x808D, 0x8087, 0x0082,
	0x8183, 0x0186, 0x018C, 0x8189, 0x0198, 0x819D, 0x8197,
	0x0192, 0x01B0, 0x81B5, 0x81BF, 0x01BA, 0x81AB, 0x01AE,
	0x01A4, 0x81A1, 0x01E0, 0x81E5, 0x81EF, 0x01EA, 0x81FB,
	0x01FE, 0x01F4, 0x81F1, 0x81D3, 0x01D6, 0x01DC, 0x81D9,
	0x01C8, 0x81CD, 0x81C7, 0x01C2, 0x0140, 0x8145, 0x814F,
	0x014A, 0x815B, 0x015E, 0x0154, 0x8151, 0x8173, 0x0176,
	0x017C, 0x8179, 0x0168, 0x816D, 0x8167, 0x0162, 0x8123,
	0x0126, 0x012C, 0x8129, 0x0138, 0x813D, 0x8137, 0x0132,
	0x0110, 0x8115, 0x811F, 0x011A, 0x810B, 0x010E, 0x0104,
	0x8101, 0x8303, 0x0306, 0x030C, 0x8309, 0x0318, 0x831D,
	0x8317, 0x0312, 0x0330, 0x8335, 0x833F, 0x033A, 0x832B,
	0x032E, 0x0324, 0x8321, 0x0360, 0x8365, 0x836F, 0x036A,
	0x837B, 0x037E, 0x0374, 0x8371, 0x8353, 0x0356, 0x035C,
	0x8359, 0x0348, 0x834D, 0x8347, 0x0342, 0x03C0, 0x83C5,
	0x83CF, 0x03CA, 0x83DB, 0x03DE, 0x03D4, 0x83D1, 0x83F3,
	0x03F6, 0x03FC, 0x83F9, 0x03E8, 0x83ED, 0x83E7, 0x03E2,
	0x83A3, 0x03A6, 0x03AC, 0x83A9, 0x03B8, 0x83BD, 0x83B7,
	0x03B2, 0x0390, 0x8395, 0x839F, 0x039A, 0x838B, 0x038E,
	0x0384, 0x8381, 0x0280, 0x8285, 0x828F, 0x028A, 0x829B,
	0x029E, 0x0294, 0x8291, 0x82B3, 0x02B6, 0x02BC, 0x82B9,
	0x02A8, 0x82AD, 0x82A7, 0x02A2, 0x82E3, 0x02E6, 0x02EC,
	0x82E9, 0x02F8, 0x82FD, 0x82F7, 0x02F2, 0x02D0, 0x82D5,
	0x82DF, 0x02DA, 0x82CB, 0x02CE, 0x02C4, 0x82C1, 0x8243,
	0x0246, 0x024C, 0x8249, 0x0258, 0x825D, 0x8257, 0x0252,
	0x0270, 0x8275, 0x827F, 0x027A, 0x826B, 0x026E, 0x0264,
	0x8261, 0x0220, 0x8225, 0x822F, 0x022A, 0x823B, 0x023E,
	0x0234, 0x8231, 0x8213, 0x0216, 0x021C, 0x8219, 0x0208,
	0x820D, 0x8207, 0x0202 };

  unsigned int i;
  unsigned char *buf1;
  unsigned short crc16;
  
  crc16=incrc16;
  buf1=(unsigned char *)buf;
  for(i=0; i< buflen; i++)
  {
    crc16=(crc16<<8)^crc16tab[((crc16>>8)^*buf1) & 0x00FF];
    buf1++;
  }  
  return(crc16);
}


// Compute CRC-16-MODBUS for buf; incrc16 - input value (must be 0xFFFF for the first call)
// NOTE: polynom is 0xA001
unsigned short crc16modbus_compute_buf(unsigned short incrc16, char *buf, unsigned int buflen)
{
  unsigned short crc16mod_tab[256]={
  0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
  0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
  0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
  0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
  0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
  0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
  0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
  0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
  0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
  0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
  0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
  0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
  0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
  0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
  0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
  0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
  0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
  0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
  0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
  0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
  0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
  0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
  0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
  0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
  0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
  0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
  0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
  0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
  0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
  0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
  0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
  0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040};

  unsigned short i;
  unsigned char *buf1;
  unsigned short crc16, c;
  
  crc16=incrc16;
  buf1=(unsigned char *)buf;
  for(i=0; i<buflen; i++)
  {
    c=0x00FF & (unsigned short)*buf1;
    crc16=(crc16>>8)^crc16mod_tab[(crc16^c) & 0xFF];
    buf1++;
  }
  return(crc16);
}

int check_network_ping(void)
{
	int rtn = 0;

	if(myPs->signal[EXT_SIG_NET_CONNECTED] != P2) return 0;

	if(myPs->pingCount >= myPs->config.retryCount) { 
		rtn = -1;
	}
	return rtn;
}

int network_ping(void)
{
	int diff, rtn = 0;

	if(myPs->signal[EXT_SIG_NET_CONNECTED] != P2) return 0;

	diff = (int)labs(myData->mData.misc.timer_1sec - myPs->pingTimer);
	if(diff > myPs->config.pingTimeout) { //Sec
		myPs->pingTimer = myData->mData.misc.timer_1sec;
		myPs->pingCount++;
		rtn = send_cmd_heartbeat();
	}

	return rtn;
}

int Check_NetworkState(void)
{
	int rtn=0;

	rtn = check_network_ping();
	if( rtn < 0){
		close(myPs->config.network_socket);
		return rtn;
	}
	rtn = network_ping();
	return rtn;
}

int	send_cmd_heartbeat(void)
{ 
	int	cmd_size, data_size, ch;
	S_EXT_CLIENT_SEND_CMD_HEARTBEAT	cmd;
	
	cmd_size = sizeof(S_EXT_CLIENT_SEND_CMD_HEARTBEAT);
	memset((char *)&cmd, 0x00, cmd_size);

	data_size = 0;
	ch = 0;
	make_header((char *)&cmd, EXT_REPLY_YES, SBC_TO_PC_CMD_HEARTBEAT, ch, data_size);
	make_crc(0, (char *)&cmd, cmd_size - 2);
	if(send_command((char*)&cmd, cmd_size) < 0) {
		userlog(DEBUG_LOG, psName, "CMD[%d] SEND FAIL!\n", atoi(cmd.header.cmdid));
		return -1;
	}
	return 0;
}

int	send_cmd_unknown(int cmdid, int reply)
{ 
	int	cmd_size, data_size, ch;
	S_EXT_CLIENT_SEND_CMD_UNKNOWN	cmd;
	
	cmd_size = sizeof(S_EXT_CLIENT_SEND_CMD_UNKNOWN);
	memset((char *)&cmd, 0x00, cmd_size);

	data_size = 0;
	ch = 0;
	make_header((char *)&cmd, reply, cmdid, ch, data_size);
	make_crc(0, (char *)&cmd, cmd_size - 2);
	if(send_command((char*)&cmd, cmd_size) < 0) {
		userlog(DEBUG_LOG, psName, "CMD[%d] SEND FAIL!\n", atoi(cmd.header.cmdid));
		return -1;
	}
	return 0;
}

int	send_cmd_response(int cmdid, int reply)
{ 
	int	cmd_size, data_size, ch;
	S_EXT_CLIENT_SEND_CMD_RESPONSE	cmd;
	
	cmd_size = sizeof(S_EXT_CLIENT_SEND_CMD_RESPONSE);
	memset((char *)&cmd, 0x00, cmd_size);

	data_size = 0;
	ch = 0;
	make_header((char *)&cmd, reply, cmdid, ch, data_size);
	make_crc(0, (char *)&cmd, cmd_size - 2);
	if(send_command((char*)&cmd, cmd_size) < 0) {
		userlog(DEBUG_LOG, psName, "CMD[%d] SEND FAIL!\n", atoi(cmd.header.cmdid));
		return -1;
	}
	return 0;
}


int send_cmd_trouble(int code)
{
	int cmd_size, body_size, ch, rtn=0;
	S_EXT_CLIENT_SEND_CMD_TROUBLE	cmd;
	
	cmd_size = sizeof(S_EXT_CLIENT_SEND_CMD_TROUBLE);
	body_size = cmd_size - sizeof(S_EXT_CLIENT_CMD_HEADER)-2;
	memset((char *)&cmd, 0x00, cmd_size);
	ch = 0;
	make_header((char *)&cmd, REPLY_NO,
		SBC_TO_PC_CMD_TROUBLE, ch, body_size);

	sprintf(cmd.code, "%03d", code);

	make_crc(0, (char *)&cmd, cmd_size - 2);
	rtn = send_command((char *)&cmd, cmd_size);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d cmdid[%d]\n", rtn,
			SBC_TO_PC_CMD_TROUBLE);
	}
	return rtn;
}

int send_cmd_module_info(void)
{
	int cmd_size, body_size, ch, tmp, rtn;
	char buf[20];
	S_EXT_CLIENT_SEND_CMD_MODULE_INFO cmd;
	
	cmd_size = sizeof(S_EXT_CLIENT_SEND_CMD_MODULE_INFO);
	body_size = cmd_size - sizeof(S_EXT_CLIENT_CMD_HEADER)-2;
	memset((char *)&cmd, 0x00, cmd_size);

	ch = 0;
	make_header((char *)&cmd, EXT_REPLY_YES,
		SBC_TO_PC_CMD_MODULE_INFO, ch, body_size);

	memset(buf, 0x00, sizeof(buf));
	sprintf(buf, "%05d", myPs->config.protocol_version);
	memcpy((char *)&cmd.md_info.protocol_version, 
		buf, sizeof(cmd.md_info.protocol_version));

	memset(buf, 0x00, sizeof(buf));
	sprintf(buf, "%02d", myData->AppControl.config.moduleNo);
	memcpy((char *)&cmd.md_info.moduleNo, buf, sizeof(cmd.md_info.moduleNo));

	memset(buf, 0x00, sizeof(buf));
	sprintf(buf,"%02d", myData->mData.config.installedCh);
	memcpy((char *)&cmd.md_info.chNo, buf, sizeof(cmd.md_info.chNo));

	memset(buf, 0x00, sizeof(buf));
	tmp = (int)(myData->mData.config.maxVoltage[0]/1000);
	sprintf(buf, "%07d", tmp);
	sprintf((char *)&cmd.md_info.voltage_range1, buf, sizeof(cmd.md_info.voltage_range1));

	memset(buf, 0x00, sizeof(buf));
	tmp = (int)(myData->mData.config.maxVoltage[1]/1000);
	sprintf(buf, "%07d", tmp);
	sprintf((char *)&cmd.md_info.voltage_range2, buf, sizeof(cmd.md_info.voltage_range2));

	memset(buf, 0x00, sizeof(buf));
	tmp = (int)(myData->mData.config.maxVoltage[2]/1000);
	sprintf(buf, "%07d", tmp);
	sprintf((char *)&cmd.md_info.voltage_range3, buf, sizeof(cmd.md_info.voltage_range3));

	memset(buf, 0x00, sizeof(buf));
	tmp = (int)(myData->mData.config.maxVoltage[3]/1000);
	sprintf(buf, "%07d", tmp);
	sprintf((char *)&cmd.md_info.voltage_range4, buf, sizeof(cmd.md_info.voltage_range4));

	memset(buf, 0x00, sizeof(buf));
	tmp = (int)(myData->mData.config.maxCurrent[0]/1000);
	sprintf(buf, "%07d", tmp);
	sprintf((char *)&cmd.md_info.current_range1, buf, sizeof(cmd.md_info.current_range1));
	
	memset(buf, 0x00, sizeof(buf));
	tmp = (int)(myData->mData.config.maxCurrent[1]/1000);
	sprintf(buf, "%07d", tmp);
	sprintf((char *)&cmd.md_info.current_range2, buf, sizeof(cmd.md_info.current_range2));

	memset(buf, 0x00, sizeof(buf));
	tmp = (int)(myData->mData.config.maxCurrent[2]/1000);
	sprintf(buf, "%07d", tmp);
	sprintf((char *)&cmd.md_info.current_range3, buf, sizeof(cmd.md_info.current_range3));
	
	memset(buf, 0x00, sizeof(buf));
	tmp = (int)(myData->mData.config.maxCurrent[3]/1000);
	sprintf(buf, "%07d", tmp);
	sprintf((char *)&cmd.md_info.current_range4, buf, sizeof(cmd.md_info.current_range4));

	memset(buf, 0x00, sizeof(buf));
	sprintf(buf,"%1d", (int)myData->mData.config.rangeV);
	memcpy((char *)&cmd.md_info.voltage_range, buf, sizeof(cmd.md_info.voltage_range));

	memset(buf, 0x00, sizeof(buf));
	sprintf(buf,"%1d", (int)myData->mData.config.rangeI);
	memcpy((char *)&cmd.md_info.current_range, buf, sizeof(cmd.md_info.current_range));

	make_crc(0, (char *)&cmd, cmd_size - 2);
	rtn = send_command((char *)&cmd, cmd_size);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d %d\n", rtn,
			SBC_TO_PC_CMD_MODULE_INFO);
	}
	return 0;
}

int send_cmd_ch_data(void)
{
	if(myData->mData.config.parallelMode == P1){
		return send_cmd_ch_data2();
	}else{
		return send_cmd_ch_data1();
	}
}

int send_cmd_ch_data1(void)
{
	int cmd_size, body_size, rtn, bd, ch, i, idx, count, msg, day, hour, min, sec, ms, code, tmp;
	
	S_EXT_CLIENT_SEND_CMD_CHANNEL_DATA	cmd;
	cmd_size = sizeof(S_EXT_CLIENT_SEND_CMD_CHANNEL_DATA);
	body_size = cmd_size - sizeof(S_EXT_CLIENT_CMD_HEADER)-2;

	count = 0;
	msg = 0;

	for(i=0; i < myData->mData.config.installedCh; i++) {
		bd = myData->CellArray1[i].bd;
		ch = myData->CellArray1[i].ch;
		if(myData->save_msg[msg].write_idx[bd][ch]
			== myData->save_msg[msg].read_idx[bd][ch]) {
			myPs->sended_monitor_data_time 
				= myData->mData.misc.timer_1sec;
			memset((char *)&cmd, 0x00, cmd_size);
			make_header((char *)&cmd, REPLY_NO,
				SBC_TO_PC_CMD_CH_DATA, i+1, body_size);	
			sprintf((char *)&cmd.cData.chNo, "%02d",  i + 1);
			cmd.cData.state = get_ch_state(bd, ch, myData->bData[bd].cData[ch].op.state);
			cmd.cData.stepType = get_ch_stepType(bd, ch, myData->bData[bd].cData[ch].op.type);
			cmd.cData.stepMode = get_ch_mode(bd, ch, myData->bData[bd].cData[ch].op.mode);

			code = get_ch_code(bd, ch, myData->bData[bd].cData[ch].op.code);
			sprintf((char *)&cmd.cData.code, "%03d", code);

			sprintf((char *)&cmd.cData.dataType, "%1d", SAVE_FLAG_MONITORING_DATA);

			sprintf((char *)&cmd.cData.stepNo, "%03d",
				(int)myData->bData[bd].cData[ch].misc.extStepNo);

			sprintf((char *)&cmd.cData.Vsens, "%07d", 
				(int)myData->bData[bd].cData[ch].op.Vsens/1000);

			if(myData->bData[bd].cData[ch].op.state != C_RUN){
				sprintf((char *)& cmd.cData.Isens, "%07d", 0);
			}else{
				switch(myData->bData[bd].cData[ch].op.type){
					case STEP_CHARGE:
					case STEP_DISCHARGE:
					case STEP_Z:
						sprintf((char *)& cmd.cData.Isens, "%07d", 
							(int)myData->bData[bd].cData[ch].op.Isens/1000);
						break;
					default:
						sprintf((char *)& cmd.cData.Isens, "%07d", 0);
						break;
				}
			}
			sprintf((char *)&cmd.cData.capacity, "%07d",
				(int)myData->bData[bd].cData[ch].op.ampareHour/1000);

			sprintf((char *)&cmd.cData.watt, "%07d",
				(int)myData->bData[bd].cData[ch].op.watt);
			sprintf((char *)&cmd.cData.wattHour, "%07d",
				(int)myData->bData[bd].cData[ch].op.wattHour);
			sprintf((char *)&cmd.cData.z, "%07d",
				(int)myData->bData[bd].cData[ch].op.z/1000);

			day = myData->bData[bd].cData[ch].op.runTime / (24 * 3600 * 100);
			hour = (myData->bData[bd].cData[ch].op.runTime % (24 * 3600 * 100))/(3600 * 100);
			min = (myData->bData[bd].cData[ch].op.runTime % (3600 * 100))/(60 * 100);
			sec = (myData->bData[bd].cData[ch].op.runTime % (60 * 100))/100;
			ms = myData->bData[bd].cData[ch].op.runTime % 100;
			sprintf((char *)&cmd.cData.step_time_day, "%03d", day);
			sprintf((char *)&cmd.cData.step_time_sec, "%06d", sec);
			sprintf((char *)&cmd.cData.step_time_mSec, "%03d", ms);

			sprintf((char *)&cmd.cData.currentCycleNo, "%06d",
				(int)myData->bData[bd].cData[ch].misc.extCurrentCycle);
			sprintf((char *)&cmd.cData.totalCycleNo, "%06d",
				(int)myData->bData[bd].cData[ch].misc.extTotalCycle);
			sprintf((char *)&cmd.cData.saveSequence, "%010d",
				(int)myData->bData[bd].cData[ch].op.resultIndex);

			make_crc(0, (char *)&cmd, cmd_size - 2);
			rtn = send_command((char *)&cmd, cmd_size);
			if(rtn < 0) {
				userlog(DEBUG_LOG, psName, "cmd send error!!! %d %d\n", rtn,
					SBC_TO_PC_CMD_CH_DATA);
				return rtn;
			}
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
			memset((char *)&cmd, 0x00, cmd_size);
			make_header((char *)&cmd, REPLY_NO,
				SBC_TO_PC_CMD_CH_DATA, i+1, body_size);	

			sprintf((char *)&cmd.cData.chNo, "%02d",  i + 1);

			tmp = myData->save_msg[msg].val[idx][bd][ch].chData.state;
			cmd.cData.state = get_ch_state(bd, ch, tmp);
			tmp = myData->save_msg[msg].val[idx][bd][ch].chData.type;
			cmd.cData.stepType = get_ch_stepType(bd, ch, tmp);
			tmp = myData->save_msg[msg].val[idx][bd][ch].chData.mode;
			cmd.cData.stepMode = get_ch_mode(bd, ch, tmp);

			tmp = myData->save_msg[msg].val[idx][bd][ch].chData.mode;
			code = get_ch_code(bd, ch, tmp);
			sprintf((char *)&cmd.cData.code, "%03d", code);

			tmp = myData->save_msg[msg].val[idx][bd][ch].chData.select;
			sprintf((char *)&cmd.cData.dataType, "%1d", tmp);

			tmp = (int)myData->bData[bd].cData[ch].misc.extStepNo;
			sprintf((char *)&cmd.cData.stepNo, "%03d", tmp);

			tmp = myData->save_msg[msg].val[idx][bd][ch].chData.Vsens/1000;
			sprintf((char *)&cmd.cData.Vsens, "%07d", tmp); 

			tmp = myData->save_msg[msg].val[idx][bd][ch].chData.Isens/1000;
			switch(myData->save_msg[msg].val[idx][bd][ch].chData.type){
				case STEP_CHARGE:
				case STEP_DISCHARGE:
				case STEP_Z:
					sprintf((char *)& cmd.cData.Isens, "%07d", tmp); 
					break;
				default:
					sprintf((char *)& cmd.cData.Isens, "%07d", 0);
					break;
			}

			tmp = myData->save_msg[msg].val[idx][bd][ch].chData.capacity/1000;
			sprintf((char *)&cmd.cData.capacity, "%07d", tmp);

			tmp = myData->save_msg[msg].val[idx][bd][ch].chData.watt;
			sprintf((char *)&cmd.cData.watt, "%07d", tmp);

			tmp = myData->save_msg[msg].val[idx][bd][ch].chData.wattHour;
			sprintf((char *)&cmd.cData.wattHour, "%07d", tmp);

			tmp = myData->save_msg[msg].val[idx][bd][ch].chData.z/1000;
			sprintf((char *)&cmd.cData.z, "%07d", tmp);

			day = myData->save_msg[msg].val[idx][bd][ch].chData.runTime / (24 * 3600 * 100);
			hour = myData->save_msg[msg].val[idx][bd][ch].chData.runTime % (24 * 3600 * 100)/(3600 * 100);
			min = myData->save_msg[msg].val[idx][bd][ch].chData.runTime % (3600 * 100)/(60 * 100);
			sec = myData->save_msg[msg].val[idx][bd][ch].chData.runTime % (60 * 100)/ 100;
			ms = myData->save_msg[msg].val[idx][bd][ch].chData.runTime % 100;
			
			sprintf((char *)&cmd.cData.step_time_day, "%03d", day);
			sprintf((char *)&cmd.cData.step_time_hour, "%02d", hour);
			sprintf((char *)&cmd.cData.step_time_min, "%02d", min);
			sprintf((char *)&cmd.cData.step_time_sec, "%02d", sec);
			sprintf((char *)&cmd.cData.step_time_mSec, "%03d", ms);

			tmp = (int)myData->bData[bd].cData[ch].misc.extCurrentCycle;
			sprintf((char *)&cmd.cData.currentCycleNo, "%06d", tmp);

			tmp = (int)myData->bData[bd].cData[ch].misc.extTotalCycle;
			sprintf((char *)&cmd.cData.totalCycleNo, "%06d", tmp);

			tmp = myData->save_msg[msg].val[idx][bd][ch].chData.resultIndex;
			sprintf((char *)&cmd.cData.saveSequence, "%010d", tmp);

			make_crc(0, (char *)&cmd, cmd_size - 2);
			rtn = send_command((char *)&cmd, cmd_size);
			if(rtn < 0) {
				userlog(DEBUG_LOG, psName, "cmd send error!!! %d %d\n", rtn,
					SBC_TO_PC_CMD_CH_DATA);
				if(myData->save_msg[msg].read_idx[bd][ch] == 0){
					myData->save_msg[msg].read_idx[bd][ch] = MAX_SAVE_MSG-1;
				}else{
					myData->save_msg[msg].read_idx[bd][ch]--;
				}
				return rtn;
			}
		}
	}
	
	return count;
}

int send_cmd_ch_data2(void)
{
	int cmd_size, body_size, rtn, bd, ch, i, idx, count, msg, day, hour, min, sec, ms, code, tmp, slave_ch;
	
	S_EXT_CLIENT_SEND_CMD_CHANNEL_DATA	cmd;
	cmd_size = sizeof(S_EXT_CLIENT_SEND_CMD_CHANNEL_DATA);
	body_size = cmd_size - sizeof(S_EXT_CLIENT_CMD_HEADER)-2;
	count = 0;
	msg = 0;
	for(i=0; i < myData->mData.config.installedCh; i++) {
		bd = myData->CellArray1[i].bd;
		ch = myData->CellArray1[i].ch;
		if(myData->save_msg[msg].write_idx[bd][ch]
			== myData->save_msg[msg].read_idx[bd][ch]) {
			memset((char *)&cmd, 0x00, cmd_size);
			make_header((char *)&cmd, REPLY_NO,
				SBC_TO_PC_CMD_CH_DATA, i+1, body_size);	
			sprintf((char *)&cmd.cData.chNo, "%02d",  i + 1);
			cmd.cData.state = get_ch_state(bd, ch, myData->bData[bd].cData[ch].op.state);
			cmd.cData.stepType = get_ch_stepType(bd, ch, myData->bData[bd].cData[ch].op.type);
			cmd.cData.stepMode = get_ch_mode(bd, ch, myData->bData[bd].cData[ch].op.mode);

			code = get_ch_code(bd, ch, myData->bData[bd].cData[ch].op.code);
			sprintf((char *)&cmd.cData.code, "%03d", code);

			sprintf((char *)&cmd.cData.dataType, "%1d", SAVE_FLAG_MONITORING_DATA);

			sprintf((char *)&cmd.cData.stepNo, "%03d",
				(int)myData->bData[bd].cData[ch].misc.extStepNo);

			sprintf((char *)&cmd.cData.Vsens, "%07d", 
				(int)myData->bData[bd].cData[ch].op.Vsens/1000);

			if(myData->bData[bd].cData[ch].ChAttribute.opType == P1) {
				slave_ch = myData->bData[bd].cData[ch]
					.ChAttribute.chNo_slave[0] - 1 ;  //1base
				tmp = myData->bData[bd].cData[ch].op.Isens
					+ myData->bData[bd].cData[slave_ch].op.Isens;
				if(myData->bData[bd].cData[ch].op.state != C_RUN){
					sprintf((char *)& cmd.cData.Isens, "%07d", 0);
				}else{
					switch(myData->bData[bd].cData[ch].op.type){
						case STEP_CHARGE:
						case STEP_DISCHARGE:
						case STEP_Z:
							sprintf((char *)& cmd.cData.Isens, "%07d", tmp/1000);
							break;
						default:
							sprintf((char *)& cmd.cData.Isens, "%07d", 0);
							break;
					}
				}


				tmp = myData->bData[bd].cData[ch].op.watt
					+ myData->bData[bd].cData[slave_ch].op.watt;
				sprintf((char *)&cmd.cData.watt, "%07d", tmp);

				tmp = myData->bData[bd].cData[ch].op.wattHour
					+ myData->bData[bd].cData[slave_ch].op.wattHour;
				sprintf((char *)&cmd.cData.wattHour, "%07d", tmp);

				tmp = myData->bData[bd].cData[ch].op.ampareHour
					+ myData->bData[bd].cData[slave_ch].op.ampareHour;
				sprintf((char *)&cmd.cData.capacity, "%07d", tmp/1000);
			} else {
				if(myData->bData[bd].cData[ch].op.state != C_RUN){
					sprintf((char *)& cmd.cData.Isens, "%07d", 0);
				}else{
					switch(myData->bData[bd].cData[ch].op.type){
						case STEP_CHARGE:
						case STEP_DISCHARGE:
						case STEP_Z:
							sprintf((char *)& cmd.cData.Isens, "%07d", 
								(int)myData->bData[bd].cData[ch].op.Isens/1000);
							break;
						default:
							sprintf((char *)& cmd.cData.Isens, "%07d", 0);
							break;
					}
				}

				sprintf((char *)&cmd.cData.capacity, "%07d",
					(int)myData->bData[bd].cData[ch].op.ampareHour/1000);
	
				sprintf((char *)&cmd.cData.watt, "%07d",
					(int)myData->bData[bd].cData[ch].op.watt);
				sprintf((char *)&cmd.cData.wattHour, "%07d",
					(int)myData->bData[bd].cData[ch].op.wattHour);
			}

			sprintf((char *)&cmd.cData.z, "%07d",
				(int)myData->bData[bd].cData[ch].op.z/1000);

			day = myData->bData[bd].cData[ch].op.runTime / (24 * 3600 * 100);

			hour = (myData->bData[bd].cData[ch].op.runTime % (24 * 3600 * 100))/(3600 * 100);
			min = (myData->bData[bd].cData[ch].op.runTime % (3600 * 100))/(60 * 100);
			sec = (myData->bData[bd].cData[ch].op.runTime % (60 * 100))/100;
			ms = myData->bData[bd].cData[ch].op.runTime % 100;

			sprintf((char *)&cmd.cData.step_time_day, "%03d", day);
			sprintf((char *)&cmd.cData.step_time_hour, "%02d", hour);
			sprintf((char *)&cmd.cData.step_time_min, "%02d", min);
			sprintf((char *)&cmd.cData.step_time_sec, "%02d", sec);
			sprintf((char *)&cmd.cData.step_time_mSec, "%03d", ms);

			sprintf((char *)&cmd.cData.currentCycleNo, "%06d",
				(int)myData->bData[bd].cData[ch].misc.extCurrentCycle);
			sprintf((char *)&cmd.cData.totalCycleNo, "%06d",
				(int)myData->bData[bd].cData[ch].misc.extTotalCycle);
			sprintf((char *)&cmd.cData.saveSequence, "%010d",
				(int)myData->bData[bd].cData[ch].op.resultIndex);

			make_crc(0, (char *)&cmd, cmd_size - 2);
			rtn = send_command((char *)&cmd, cmd_size);
			if(rtn < 0) {
				userlog(DEBUG_LOG, psName, "cmd send error!!! %d %d\n", rtn,
					SBC_TO_PC_CMD_CH_DATA);
				return rtn;
			}
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
			memset((char *)&cmd, 0x00, cmd_size);
			make_header((char *)&cmd, REPLY_NO,
				SBC_TO_PC_CMD_CH_DATA, i+1, body_size);	

			sprintf((char *)&cmd.cData.chNo, "%02d",  i + 1);

			tmp = myData->save_msg[msg].val[idx][bd][ch].chData.state;
			cmd.cData.state = get_ch_state(bd, ch, tmp);
			tmp = myData->save_msg[msg].val[idx][bd][ch].chData.type;
			cmd.cData.stepType = get_ch_stepType(bd, ch, tmp);
			tmp = myData->save_msg[msg].val[idx][bd][ch].chData.mode;
			cmd.cData.stepMode = get_ch_mode(bd, ch, tmp);

			tmp = myData->save_msg[msg].val[idx][bd][ch].chData.mode;
			code = get_ch_code(bd, ch, tmp);
			sprintf((char *)&cmd.cData.code, "%03d", code);

			tmp = myData->save_msg[msg].val[idx][bd][ch].chData.select;
			sprintf((char *)&cmd.cData.dataType, "%1d", tmp);

			tmp = (int)myData->bData[bd].cData[ch].misc.extStepNo;
			sprintf((char *)&cmd.cData.stepNo, "%03d", tmp);

			tmp = myData->save_msg[msg].val[idx][bd][ch].chData.Vsens/1000;
			sprintf((char *)&cmd.cData.Vsens, "%07d", tmp); 

			tmp = myData->save_msg[msg].val[idx][bd][ch].chData.Isens/1000;

			switch(myData->save_msg[msg].val[idx][bd][ch].chData.type){
				case STEP_CHARGE:
				case STEP_DISCHARGE:
				case STEP_Z:
					sprintf((char *)& cmd.cData.Isens, "%07d", tmp); 
					break;
				default:
					sprintf((char *)& cmd.cData.Isens, "%07d", 0);
					break;
			}

			tmp = myData->save_msg[msg].val[idx][bd][ch].chData.capacity/1000;
			sprintf((char *)&cmd.cData.capacity, "%07d", tmp);

			tmp = myData->save_msg[msg].val[idx][bd][ch].chData.watt;
			sprintf((char *)&cmd.cData.watt, "%07d", tmp);

			tmp = myData->save_msg[msg].val[idx][bd][ch].chData.wattHour;
			sprintf((char *)&cmd.cData.wattHour, "%07d", tmp);

			tmp = myData->save_msg[msg].val[idx][bd][ch].chData.z/1000;
			sprintf((char *)&cmd.cData.z, "%07d", tmp);

			day = myData->save_msg[msg].val[idx][bd][ch].chData.runTime / (24 * 3600 * 100);

			hour = (myData->save_msg[msg].val[idx][bd][ch].chData.runTime % (24 * 3600 * 100))/(3600 * 100);
			min = (myData->save_msg[msg].val[idx][bd][ch].chData.runTime % (3600 * 100))/(60 * 100);
			sec = (myData->save_msg[msg].val[idx][bd][ch].chData.runTime % (60 * 100))/100;
			ms = myData->save_msg[msg].val[idx][bd][ch].chData.runTime % 100;
			
			sprintf((char *)&cmd.cData.step_time_day, "%03d", day);
			sprintf((char *)&cmd.cData.step_time_hour, "%02d", hour);
			sprintf((char *)&cmd.cData.step_time_min, "%02d", min);
			sprintf((char *)&cmd.cData.step_time_sec, "%02d", sec);
			sprintf((char *)&cmd.cData.step_time_mSec, "%03d", ms);

			tmp = (int)myData->bData[bd].cData[ch].misc.extCurrentCycle;
			sprintf((char *)&cmd.cData.currentCycleNo, "%06d", tmp);

			tmp = (int)myData->bData[bd].cData[ch].misc.extTotalCycle;
			sprintf((char *)&cmd.cData.totalCycleNo, "%06d", tmp);

			tmp = myData->save_msg[msg].val[idx][bd][ch].chData.resultIndex;
			sprintf((char *)&cmd.cData.saveSequence, "%010d", tmp);
			
			make_crc(0, (char *)&cmd, cmd_size - 2);
			rtn = send_command((char *)&cmd, cmd_size);
			if(rtn < 0) {
				userlog(DEBUG_LOG, psName, "cmd send error!!! %d %d\n", rtn,
					SBC_TO_PC_CMD_CH_DATA);
				if(myData->save_msg[msg].read_idx[bd][ch] == 0){
					myData->save_msg[msg].read_idx[bd][ch] = MAX_SAVE_MSG-1;
				}else{
					myData->save_msg[msg].read_idx[bd][ch]--;
				}
				return rtn;
			}
		}
	}
	return count;
}

int	rcv_cmd_heartbeat_response(void)
{
	int size, rtn;
	S_EXT_CLIENT_RCV_CMD_RESPONSE	cmd;

	memset((char *)&cmd, 0x00, sizeof(cmd));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[1],
		sizeof(cmd));

	size = sizeof(cmd)-2;
	rtn = check_crc(0, (char *)&cmd, size);
	if(rtn < 0){
		rtn = send_cmd_unknown(SBC_TO_PC_CMD_CRC_ERROR, EXT_REPLY_YES);
		userlog(DEBUG_LOG, psName, 
			"ExtClient RcvCmd  CRC Error : %d\n", atoi(cmd.header.cmdid));
		return rtn;
	}
	myPs->pingCount = 0;

	return 0;
}

int rcv_cmd_response(void)
{
	int size, rtn;
	S_EXT_CLIENT_RCV_CMD_RESPONSE	cmd;

	memset((char *)&cmd, 0x00, sizeof(cmd));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[1],sizeof(cmd));

	size = sizeof(cmd)-2;
	rtn = check_crc(0, (char *)&cmd, size);
	if(rtn < 0){
		rtn = send_cmd_unknown(SBC_TO_PC_CMD_CRC_ERROR, EXT_REPLY_YES);
		userlog(DEBUG_LOG, psName, 
			"ExtClient RcvCmd  CRC Error : %d\n", atoi(cmd.header.cmdid));
		return rtn;
	}
	return 0;
}

int rcv_cmd_crc_error(void)
{
	int size, rtn;
	S_EXT_CLIENT_RCV_CMD_CRC_ERROR	cmd;

	memset((char *)&cmd, 0x00, sizeof(cmd));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[1], sizeof(cmd));

	size = sizeof(cmd)-2;
	rtn = check_crc(0, (char *)&cmd, size);
	if(rtn < 0){
		rtn = send_cmd_unknown(SBC_TO_PC_CMD_CRC_ERROR, EXT_REPLY_NO);
		userlog(DEBUG_LOG, psName, 
			"ExtClient RcvCmd  CRC Error : %d\n", atoi(cmd.header.cmdid));
		return rtn;
	}
	return 0;
}


int rcv_cmd_unknown(void)
{
	int size, rtn;
	S_EXT_CLIENT_RCV_CMD_RESPONSE	cmd;

	memset((char *)&cmd, 0x00, sizeof(cmd));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[1],
		sizeof(cmd));

	size = sizeof(cmd)-2;

	rtn = check_crc(0, (char *)&cmd, size);
	if(rtn < 0){
		rtn = send_cmd_unknown(SBC_TO_PC_CMD_CRC_ERROR, EXT_REPLY_YES);
		userlog(DEBUG_LOG, psName, 
			"ExtClient RcvCmd  CRC Error : %d\n", atoi(cmd.header.cmdid));
		return rtn;
	}
	return 0;
}

int rcv_cmd_run(void)
{
	int bd, ch, i, rtn, size;
	unsigned long chFlag;
	unsigned char slave_ch;

	S_MSG_CH_FLAG	ch_flag;
	S_EXT_CLIENT_RCV_CMD_RUN	cmd;

	memset((char *)&cmd, 0x00, sizeof(S_EXT_CLIENT_RCV_CMD_RUN));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[1],
		sizeof(S_EXT_CLIENT_RCV_CMD_RUN));

	size = sizeof(cmd)-2;
	
	rtn = check_crc(0, (char *)&cmd, size);
	if(rtn < 0){
		rtn = send_cmd_unknown(SBC_TO_PC_CMD_CRC_ERROR, EXT_REPLY_YES);
		userlog(DEBUG_LOG, psName, 
			"ExtClient RcvCmd  CRC Error : %d\n", atoi(cmd.header.cmdid));
		return rtn;
	}

	i = atoi(cmd.header.chNo)-1;
#ifdef DEBUG
	printf("CMD RUN CH: %d\n", i+1);
#endif
	if(i < 0 && i >= MAX_CH_PER_MODULE){
		userlog(DEBUG_LOG, psName, 
			"CMD Run Out of channel range, Rcv Ch No[%d], Range is 1~64\n", i+1);
		return send_cmd_trouble(EP_CD_CH_STATE_ERROR);
	}

	bd = myData->CellArray1[i].bd;
	ch = myData->CellArray1[i].ch;
	if(myData->bData[bd].cData[ch].op.state != C_STANDBY) {
		return send_cmd_trouble(EP_CD_CH_STATE_ERROR);
	}

	if(myPs->misc.rcvRecipeFlag[bd][ch] != P1){
		return send_cmd_trouble(EP_CD_CH_STATE_ERROR);
	}

	myPs->misc.rcvRecipeFlag[bd][ch] = P0;

	chFlag = 0x00000001;
	ch_flag.bit_32[0] = 0;
	ch_flag.bit_32[1] = 0;

	ch_flag.bit_32[i/32] = chFlag << i;
	if(myData->bData[bd].cData[ch].ChAttribute.opType == P1)
	{
		slave_ch = myData->bData[bd].cData[ch].
						ChAttribute.chNo_slave[0] -1 ;
		memcpy((char *)&myData->mData.testCond[bd][slave_ch],
			(char *)&myData->mData.testCond[bd][ch],
				sizeof(S_TEST_CONDITION));
	}

	myData->mData.testCond[bd][ch].reserved.select_run = 0;
	myData->mData.testCond[bd][ch].reserved.select_stepNo = 0;
	myData->mData.testCond[bd][ch].reserved.select_cycleNo = 0;
	myData->mData.testCond[bd][ch].reserved.select_advCycleStep = 0;
	myData->mData.testCond[bd][ch].reserved.reserved_cmd = 0;
	myData->mData.testCond[bd][ch].reserved.reserved_stepNo = 0;
	myData->mData.testCond[bd][ch].reserved.reserved_cycleNo = 0;
	myData->bData[bd].cData[ch].op.reservedCmd = 0;

	send_msg_ch_flag(MAIN_TO_DATASAVE, (char *)&ch_flag);
	send_msg(MAIN_TO_DATASAVE, MSG_EXT_DATASAVE_SAVED_FILE_DELETE, 0, 0);
//	send_msg_ch_flag(EXT_TO_DATASAVE, (char *)&ch_flag);
//	send_msg(EXT_TO_DATASAVE, MSG_EXT_DATASAVE_SAVED_FILE_DELETE, 0, 0);

	return send_cmd_response(SBC_TO_PC_CMD_RUN_RESPONSE, EXT_REPLY_NO);
}

int rcv_cmd_stop(void)
{
	int bd, ch=0, i, rtn, size;
	unsigned long chFlag;
	S_MSG_CH_FLAG	ch_flag;
	S_EXT_CLIENT_RCV_CMD_STOP	cmd;

	memset((char *)&cmd, 0x00, sizeof(S_EXT_CLIENT_RCV_CMD_STOP));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[1],
		sizeof(S_EXT_CLIENT_RCV_CMD_STOP));

	size = sizeof(cmd)-2;

	rtn = check_crc(0, (char *)&cmd, size);
	if(rtn < 0){
		rtn = send_cmd_unknown(SBC_TO_PC_CMD_CRC_ERROR, EXT_REPLY_YES);
		userlog(DEBUG_LOG, psName, 
			"ExtClient RcvCmd  CRC Error : %d\n", atoi(cmd.header.cmdid));
		return rtn;
	}

	i = atoi(cmd.header.chNo)-1;
	if(i < 0 && i >= MAX_CH_PER_MODULE){
		return send_cmd_trouble(EP_CD_CH_STATE_ERROR);
	}

	bd = myData->CellArray1[i].bd;
	ch = myData->CellArray1[i].ch;

	chFlag = 0x00000001;
	ch_flag.bit_32[0] = 0;
	ch_flag.bit_32[1] = 0;
		
	ch_flag.bit_32[i/32] = chFlag << i;
/*	
	if(myData->bData[bd].cData[ch].ChAttribute.opType == P1){
		chFlag = 0x00000001;
		ch_flag.bit_32[i/32] |= chFlag << (i+1);
	}
*/
	if(myData->bData[bd].cData[ch].op.state != C_RUN
		&& myData->bData[bd].cData[ch].op.state != C_PAUSE) {
		return send_cmd_trouble(EP_CD_CH_STATE_ERROR);
	}

	myData->mData.testCond[bd][ch].reserved.select_run = 0;
	myData->mData.testCond[bd][ch].reserved.select_stepNo = 0;
	myData->mData.testCond[bd][ch].reserved.select_cycleNo = 0;
	myData->mData.testCond[bd][ch].reserved.select_advCycleStep = 0;
	myData->mData.testCond[bd][ch].reserved.reserved_cmd = 0;
	myData->mData.testCond[bd][ch].reserved.reserved_stepNo = 0;
	myData->mData.testCond[bd][ch].reserved.reserved_cycleNo = 0;
	myData->bData[bd].cData[ch].op.reservedCmd = 0;
	
	myData->bData[bd].cData[ch].signal[C_SIG_STOP] = P1;
	
	return send_cmd_response(SBC_TO_PC_CMD_STOP_RESPONSE, EXT_REPLY_NO);
}

int rcv_cmd_pause(void)
{
	int bd, ch=0, i, rtn, size;
	unsigned long chFlag;
	S_MSG_CH_FLAG	ch_flag;
	S_EXT_CLIENT_RCV_CMD_PAUSE	cmd;

	memset((char *)&cmd, 0x00, sizeof(S_EXT_CLIENT_RCV_CMD_PAUSE));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[1],	sizeof(S_EXT_CLIENT_RCV_CMD_PAUSE));

	size = sizeof(cmd)-2;
	rtn = check_crc(0, (char *)&cmd, size);
	if(rtn < 0){
		rtn = send_cmd_unknown(SBC_TO_PC_CMD_CRC_ERROR, EXT_REPLY_YES);
		userlog(DEBUG_LOG, psName, 
			"ExtClient RcvCmd  CRC Error : %d\n", atoi(cmd.header.cmdid));
		return rtn;
	}

	i = atoi(cmd.header.chNo)-1;
	if(i < 0 && i >= MAX_CH_PER_MODULE){
		return send_cmd_trouble(EP_CD_CH_STATE_ERROR);
	}

	bd = myData->CellArray1[i].bd;
	ch = myData->CellArray1[i].ch;

	chFlag = 0x00000001;
	ch_flag.bit_32[0] = 0;
	ch_flag.bit_32[1] = 0;
		
	ch_flag.bit_32[i/32] = chFlag << i;
/*	
	if(myData->bData[bd].cData[ch].ChAttribute.opType == P1){
		chFlag = 0x00000001;
		ch_flag.bit_32[i/32] |= chFlag << (i+1);
	}
*/
	if(myData->bData[bd].cData[ch].op.state != C_RUN) {
		return send_cmd_trouble(EP_CD_CH_STATE_ERROR);
	} 

	myData->mData.testCond[bd][ch].reserved.select_run = 0;
	myData->mData.testCond[bd][ch].reserved.select_stepNo = 0;
	myData->mData.testCond[bd][ch].reserved.select_cycleNo = 0;
	myData->mData.testCond[bd][ch].reserved.select_advCycleStep = 0;
	myData->mData.testCond[bd][ch].reserved.reserved_cmd = 0;
	myData->mData.testCond[bd][ch].reserved.reserved_stepNo = 0;
	myData->mData.testCond[bd][ch].reserved.reserved_cycleNo = 0;
	myData->bData[bd].cData[ch].op.reservedCmd = 0;

	myData->bData[bd].cData[ch].signal[C_SIG_PAUSE] = P1;
	
	return send_cmd_response(SBC_TO_PC_CMD_PAUSE_RESPONSE, EXT_REPLY_NO);
}

int rcv_cmd_continue(void)
{
	int bd, ch=0, i, rtn, size;
	unsigned long chFlag;
	S_MSG_CH_FLAG	ch_flag;
	S_EXT_CLIENT_RCV_CMD_CONTINUE	cmd;

	memset((char *)&cmd, 0x00, sizeof(S_EXT_CLIENT_RCV_CMD_CONTINUE));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[1],
		sizeof(S_EXT_CLIENT_RCV_CMD_CONTINUE));

	size = sizeof(cmd)-2;
	rtn = check_crc(0, (char *)&cmd, size);
	if(rtn < 0){
		rtn = send_cmd_unknown(SBC_TO_PC_CMD_CRC_ERROR, EXT_REPLY_YES);
		userlog(DEBUG_LOG, psName, 
			"ExtClient RcvCmd  CRC Error : %d\n", atoi(cmd.header.cmdid));
		return rtn;
	}

	i = atoi(cmd.header.chNo)-1;
	if(i < 0 && i >= MAX_CH_PER_MODULE){
		return send_cmd_trouble(EP_CD_CH_STATE_ERROR);
	}

	bd = myData->CellArray1[i].bd;
	ch = myData->CellArray1[i].ch;

	chFlag = 0x00000001;
	ch_flag.bit_32[0] = 0;
	ch_flag.bit_32[1] = 0;
		
	ch_flag.bit_32[i/32] = chFlag << i;
/*	
	if(myData->bData[bd].cData[ch].ChAttribute.opType == P1){
		chFlag = 0x00000001;
		ch_flag.bit_32[i/32] |= chFlag << (i+1);
	}
*/
	if(myData->bData[bd].cData[ch].op.state != C_PAUSE) {
		return send_cmd_trouble(EP_CD_CH_STATE_ERROR);
	} 

	myData->mData.testCond[bd][ch].reserved.select_run = 0;
	myData->mData.testCond[bd][ch].reserved.select_stepNo = 0;
	myData->mData.testCond[bd][ch].reserved.select_cycleNo = 0;
	myData->mData.testCond[bd][ch].reserved.select_advCycleStep = 0;
	myData->mData.testCond[bd][ch].reserved.reserved_cmd = 0;
	myData->mData.testCond[bd][ch].reserved.reserved_stepNo = 0;
	myData->mData.testCond[bd][ch].reserved.reserved_cycleNo = 0;
	myData->bData[bd].cData[ch].op.reservedCmd = 0;

	send_msg_ch_flag(MAIN_TO_DATASAVE, (char *)&ch_flag);
	send_msg(MAIN_TO_DATASAVE, MSG_MAIN_DATASAVE_RCVED_CONTINUE_CMD, 0, 0);
//	send_msg_ch_flag(EXT_TO_DATASAVE, (char *)&ch_flag);
//	send_msg(EXT_TO_DATASAVE, MSG_MAIN_DATASAVE_RCVED_CONTINUE_CMD, 0, 0);
	
	return send_cmd_response(SBC_TO_PC_CMD_CONTINUE_RESPONSE, EXT_REPLY_NO);
}

int rcv_cmd_init(void)
{
	int bd, ch=0, i, rtn, size;
	unsigned long chFlag;
	S_MSG_CH_FLAG	ch_flag;
	S_EXT_CLIENT_RCV_CMD_INIT	cmd;

	memset((char *)&cmd, 0x00, sizeof(S_EXT_CLIENT_RCV_CMD_INIT));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[1],
		sizeof(S_EXT_CLIENT_RCV_CMD_INIT));

	size = sizeof(cmd)-2;

	rtn = check_crc(0, (char *)&cmd, size);
	if(rtn < 0){
		rtn = send_cmd_unknown(SBC_TO_PC_CMD_CRC_ERROR, EXT_REPLY_YES);
		userlog(DEBUG_LOG, psName, 
			"ExtClient RcvCmd  CRC Error : %d\n", atoi(cmd.header.cmdid));
		return rtn;
	}

	i = atoi(cmd.header.chNo)-1;
	if(i < 0 && i >= MAX_CH_PER_MODULE){
		return send_cmd_trouble(EP_CD_CH_STATE_ERROR);
	}

	bd = myData->CellArray1[i].bd;
	ch = myData->CellArray1[i].ch;

	myPs->misc.rcvRecipeFlag[bd][ch] = P0;

	chFlag = 0x00000001;
	ch_flag.bit_32[0] = 0;
	ch_flag.bit_32[1] = 0;
		
	ch_flag.bit_32[i/32] = chFlag << i;
/*
	if(myData->bData[bd].cData[ch].ChAttribute.opType == P1){
		chFlag = 0x00000001;
		ch_flag.bit_32[i/32] |= chFlag << (i+1);
	}
*/
	myData->mData.testCond[bd][ch].reserved.select_run = 0;
	myData->mData.testCond[bd][ch].reserved.select_stepNo = 0;
	myData->mData.testCond[bd][ch].reserved.select_cycleNo = 0;
	myData->mData.testCond[bd][ch].reserved.select_advCycleStep = 0;
	myData->mData.testCond[bd][ch].reserved.reserved_cmd = 0;
	myData->mData.testCond[bd][ch].reserved.reserved_stepNo = 0;
	myData->mData.testCond[bd][ch].reserved.reserved_cycleNo = 0;
	myData->bData[bd].cData[ch].op.reservedCmd = 0;

	myData->bData[bd].cData[ch].op.state = C_IDLE;
	myData->bData[bd].cData[ch].op.phase = P0;
	if(myData->bData[bd].cData[ch].ChAttribute.opType == P1){
		myData->bData[bd].cData[ch+1].op.state = C_IDLE;
		myData->bData[bd].cData[ch+1].op.phase = P0;
	}
	
	return send_cmd_response(SBC_TO_PC_CMD_INIT_RESPONSE, EXT_REPLY_NO);
}


int rcv_cmd_nextstep(void)
{
	int bd, ch=0, i, rtn, size;
	unsigned long chFlag;
	S_MSG_CH_FLAG	ch_flag;
	S_EXT_CLIENT_RCV_CMD_NEXTSTEP	cmd;

	memset((char *)&cmd, 0x00, sizeof(S_EXT_CLIENT_RCV_CMD_NEXTSTEP));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[1],
		sizeof(S_EXT_CLIENT_RCV_CMD_NEXTSTEP));
	
	size = sizeof(cmd)-2;

	rtn = check_crc(0, (char *)&cmd, size);
	if(rtn < 0){
		rtn = send_cmd_unknown(SBC_TO_PC_CMD_CRC_ERROR, EXT_REPLY_YES);
		userlog(DEBUG_LOG, psName, 
			"ExtClient RcvCmd  CRC Error : %d\n", atoi(cmd.header.cmdid));
		return rtn;
	}

	i = atoi(cmd.header.chNo)-1;
	if(i < 0 && i >= MAX_CH_PER_MODULE){
		return send_cmd_trouble(EP_CD_CH_STATE_ERROR);
	}

	bd = myData->CellArray1[i].bd;
	ch = myData->CellArray1[i].ch;

	chFlag = 0x00000001;
	ch_flag.bit_32[0] = 0;
	ch_flag.bit_32[1] = 0;
		
	ch_flag.bit_32[i/32] = chFlag << i;
/*	
	if(myData->bData[bd].cData[ch].ChAttribute.opType == P1){
		chFlag = 0x00000001;
		ch_flag.bit_32[i/32] |= chFlag << (i+1);
	}
*/
	if(myData->bData[bd].cData[ch].op.state != C_RUN
		&& myData->bData[bd].cData[ch].op.state != C_PAUSE) {
		return send_cmd_trouble(EP_CD_CH_STATE_ERROR);
	} 

	myData->mData.testCond[bd][ch].reserved.select_run = 0;
	myData->mData.testCond[bd][ch].reserved.select_stepNo = 0;
	myData->mData.testCond[bd][ch].reserved.select_cycleNo = 0;
	myData->mData.testCond[bd][ch].reserved.select_advCycleStep = 0;
	myData->mData.testCond[bd][ch].reserved.reserved_cmd = 0;
	myData->mData.testCond[bd][ch].reserved.reserved_stepNo = 0;
	myData->mData.testCond[bd][ch].reserved.reserved_cycleNo = 0;
	myData->bData[bd].cData[ch].op.reservedCmd = 0;

	myData->bData[bd].cData[ch].signal[C_SIG_NEXTSTEP] = P1;
	
	return send_cmd_response(SBC_TO_PC_CMD_NEXTSTEP_RESPONSE, EXT_REPLY_NO);
}

int rcv_cmd_parallel(void)
{
	int bd, ch=0, i, rtn, size, count;
	unsigned char buf[12];
	S_EXT_CLIENT_RCV_CMD_PARALLEL	cmd;

	memset((char *)&cmd, 0x00, sizeof(S_EXT_CLIENT_RCV_CMD_PARALLEL));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[1],
		sizeof(S_EXT_CLIENT_RCV_CMD_PARALLEL));
	
	size = sizeof(cmd)-2;

	rtn = check_crc(0, (char *)&cmd, size);
	if(rtn < 0){
		rtn = send_cmd_unknown(SBC_TO_PC_CMD_CRC_ERROR, EXT_REPLY_YES);
		userlog(DEBUG_LOG, psName, 
			"ExtClient RcvCmd  CRC Error : %d\n", atoi(cmd.header.cmdid));
		return rtn;
	}

	memset(buf, 0x00, sizeof(buf));
	memcpy(buf, (char *)&cmd.parallel.chNo, sizeof(cmd.parallel.chNo));
	i = atoi(buf)-1;
#ifdef DEBUG
	userlog(DEBUG_LOG, psName, "Parallel Start CH[%d]\n", i+1);
#endif
	if(i < 0 && i >= MAX_CH_PER_MODULE){
		return send_cmd_trouble(EP_CD_CH_STATE_ERROR);
	}

	bd = myData->CellArray1[i].bd;
	ch = myData->CellArray1[i].ch;

	if(myData->bData[bd].cData[ch].op.state != C_STANDBY) {
		userlog(DEBUG_LOG, psName, 
			"Ch State is not C_STANDBY BD[%d]CH[%d]\n", bd+1, ch+1);
		return send_cmd_trouble(EP_CD_CH_STATE_ERROR);
	}

	memset(buf, 0x00, sizeof(buf));
	memcpy(buf, (char *)&cmd.header.chNo, sizeof(cmd.header.chNo));
	i = atoi(buf)-1;
	if(i < 0 && i >= MAX_CH_PER_MODULE){
		userlog(DEBUG_LOG, psName, 
			"Out of channel range Rcv Ch No[%d], Range is 1~64\n", i+1);
		return send_cmd_trouble(EP_CD_CH_STATE_ERROR);
	}

	memset(buf, 0x00, sizeof(buf));
	memcpy(buf, (char *)&cmd.parallel.parallel_count, sizeof(cmd.parallel.parallel_count));
	count = atoi(buf);
	if(count != 2){
		userlog(DEBUG_LOG, psName, 
			"Out of range parallel ch set count[%d], Range only is 2\n", count);
		return send_cmd_trouble(EP_CD_CH_STATE_ERROR);
	}

	memset(buf, 0x00, sizeof(buf));
	memcpy(buf, (char *)&cmd.parallel.set, sizeof(cmd.parallel.set));

	i = atoi(buf);
	if(i == 1){
		myData->bData[bd].cData[ch].ChAttribute.opType = 1;
		myData->bData[bd].cData[ch].ChAttribute.chNo_master = ch + 1;//1Base
		myData->bData[bd].cData[ch].ChAttribute.chNo_slave[0] = ch + 2; //1Base 
		myData->bData[bd].cData[ch].ChAttribute.chNo_slave[1] = 0; //1Base 
		myData->bData[bd].cData[ch].ChAttribute.chNo_slave[2] = 0; //1Base 

		myData->bData[bd].cData[ch+1].ChAttribute.opType = 0;
		myData->bData[bd].cData[ch+1].ChAttribute.chNo_master = 0;//1Base
		myData->bData[bd].cData[ch+1].ChAttribute.chNo_slave[0] = 0; //1Base 
		myData->bData[bd].cData[ch+1].ChAttribute.chNo_slave[1] = 0; //1Base 
		myData->bData[bd].cData[ch+1].ChAttribute.chNo_slave[2] = 0; //1Base 
	}else{
		myData->bData[bd].cData[ch].ChAttribute.opType = 0;
		myData->bData[bd].cData[ch].ChAttribute.chNo_master = ch + 1;//1Base
		myData->bData[bd].cData[ch].ChAttribute.chNo_slave[0] = 0; //1Base 
		myData->bData[bd].cData[ch].ChAttribute.chNo_slave[1] = 0; //1Base 
		myData->bData[bd].cData[ch].ChAttribute.chNo_slave[2] = 0; //1Base 

		myData->bData[bd].cData[ch+1].ChAttribute.opType = 0;
		myData->bData[bd].cData[ch+1].ChAttribute.chNo_master = ch + 2;//1Base
		myData->bData[bd].cData[ch+1].ChAttribute.chNo_slave[0] = 0; //1Base 
		myData->bData[bd].cData[ch+1].ChAttribute.chNo_slave[1] = 0; //1Base 
		myData->bData[bd].cData[ch+1].ChAttribute.chNo_slave[2] = 0; //1Base 
	}
	myData->mData.testCond[bd][ch].reserved.select_run = 0;
	myData->mData.testCond[bd][ch].reserved.select_stepNo = 0;
	myData->mData.testCond[bd][ch].reserved.select_cycleNo = 0;
	myData->mData.testCond[bd][ch].reserved.select_advCycleStep = 0;
	myData->mData.testCond[bd][ch].reserved.reserved_cmd = 0;
	myData->mData.testCond[bd][ch].reserved.reserved_stepNo = 0;
	myData->mData.testCond[bd][ch].reserved.reserved_cycleNo = 0;
	myData->bData[bd].cData[ch].op.reservedCmd = 0;

	send_msg(EXT_TO_APP, MSG_EXT_APP_WRITE_CHATTRIBUTE, 0, 0);
	return send_cmd_response(SBC_TO_PC_CMD_PARALLEL_RESPONSE, EXT_REPLY_NO);
}

int rcv_cmd_step_data(void)
{
	int bd, ch=0, i, size, parallel;
	unsigned char buf[20];
	unsigned long chFlag;
	int rangeI, type, mode, rtn;
	long tmp, refV, refP, refI;

	S_MSG_CH_FLAG	ch_flag;
	S_EXT_CLIENT_RCV_CMD_STEP_DATA	cmd;

	memset((char *)&cmd, 0x00, sizeof(S_EXT_CLIENT_RCV_CMD_STEP_DATA));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[1],
		sizeof(S_EXT_CLIENT_RCV_CMD_STEP_DATA));
	
	size = sizeof(cmd)-2;

	rtn = check_crc(0, (char *)&cmd, size);
	if(rtn < 0){
		rtn = send_cmd_unknown(SBC_TO_PC_CMD_CRC_ERROR, EXT_REPLY_NO);
		userlog(DEBUG_LOG, psName, 
			"ExtClient RcvCmd  CRC Error : %d\n", atoi(cmd.header.cmdid));
		return rtn;
	}

	i = atoi(cmd.header.chNo)-1;

#ifdef DEBUG
		userlog(DEBUG_LOG, psName, "chNo : %d\n", i+1);
#endif

	if(i < 0 && i >= MAX_CH_PER_MODULE){
		return send_cmd_trouble(EP_CD_CH_STATE_ERROR);
	}

	bd = myData->CellArray1[i].bd;
	ch = myData->CellArray1[i].ch;

	chFlag = 0x00000001;
	ch_flag.bit_32[0] = 0;
	ch_flag.bit_32[1] = 0;
		
	ch_flag.bit_32[i/32] = chFlag << i;
/*	
	if(myData->bData[bd].cData[ch].ChAttribute.opType == P1){
		chFlag = 0x00000001;
		ch_flag.bit_32[i/32] |= chFlag << (i+1);
	}
*/
	if(myData->bData[bd].cData[ch].op.state != C_STANDBY){

		userlog(DEBUG_LOG, psName, 
			"Channel is not Standby\n", i+1);
		return send_cmd_trouble(EP_CD_CH_STATE_ERROR);
	} 
	
	if(myData->bData[bd].cData[ch].ChAttribute.opType == P1){
		parallel = 1;
	}else{
		parallel = 0;
	}
	memset((char *)&myData->mData.testCond[bd][ch], 0x00, sizeof(S_TEST_CONDITION));

	myData->mData.testCond[bd][ch].reserved.select_run = 0;
	myData->mData.testCond[bd][ch].reserved.select_stepNo = 0;
	myData->mData.testCond[bd][ch].reserved.select_cycleNo = 0;
	myData->mData.testCond[bd][ch].reserved.select_advCycleStep = 0;
	myData->mData.testCond[bd][ch].reserved.reserved_cmd = 0;
	myData->mData.testCond[bd][ch].reserved.reserved_stepNo = 0;
	myData->mData.testCond[bd][ch].reserved.reserved_cycleNo = 0;

	myData->bData[bd].cData[ch].op.reservedCmd = 0;

//	Convert_testCond(bd, ch, parallel, cmd);

	//testcond update not init
	for( i = 0; i < 4; i++){
		rtn = 0;
		switch(i){
			case 0:
				type = STEP_ADV_CYCLE;
				mode = MODE_IDLE;
				break;
			case 1: //Step Data
				rtn = 1;
				break;
			case 2:
				type = STEP_LOOP;
				mode = MODE_IDLE;
				break;
			case 3:
				type = STEP_END;
				mode = MODE_IDLE;
				break;
			default:
				break;
		}

		myData->mData.testCond[bd][ch].step[i].type = type;
		myData->mData.testCond[bd][ch].step[i].stepNo = i;
		myData->mData.testCond[bd][ch].step[i].mode = mode;

		if(rtn == 1){
			memset(buf, 0x00, sizeof(buf));
			memcpy(buf, (char *)&cmd.step.stepType
					, sizeof(cmd.step.stepType));
			tmp = atoi(buf); 
			myData->mData.testCond[bd][ch].step[i].type
				   = (char)convert_ch_stepType(bd, ch, (int)tmp);

			switch(myData->mData.testCond[bd][ch].step[i].type){
				case STEP_CHARGE:
				case STEP_DISCHARGE:
				case STEP_Z:
				case STEP_REST:
				case STEP_OCV:
					break;
				default:
					return send_cmd_trouble(EP_CD_CH_STATE_ERROR);
			}
#ifdef DEBUG
			userlog(DEBUG_LOG, psName, "stepType : %d\n", tmp);
#endif
			memset(buf, 0x00, sizeof(buf));
			memcpy(buf, (char *)&cmd.step.stepNo, sizeof(cmd.step.stepNo));
			tmp = atoi(buf); 
			myData->bData[bd].cData[ch].misc.extStepNo = tmp;

#ifdef DEBUG
			userlog(DEBUG_LOG, psName, "stepNo : %d\n", tmp);
#endif

			memset(buf, 0x00, sizeof(buf));
			memcpy(buf, (char *)&cmd.step.stepMode
					, sizeof(cmd.step.stepMode));
			tmp = atoi(buf); 
			myData->mData.testCond[bd][ch].step[i].mode
				   = (char)convert_ch_mode(bd, ch, (int)tmp);

#ifdef DEBUG
			userlog(DEBUG_LOG, psName, "stepMode : %d\n", tmp);
#endif

			memset(buf, 0x00, sizeof(buf));
			memcpy(buf, (char *)&cmd.step.Vref, sizeof(cmd.step.Vref));
			refV = atoi(buf) * 1000; 
			if(labs(refV) > myData->mData.config.maxVoltage[0]){ // Ref Filtering
				userlog(DEBUG_LOG, psName, 
						"RefV Value Out of Range : %d\n", refV);
					return send_cmd_trouble(EP_CD_CH_STATE_ERROR);
			}
			

#ifdef DEBUG
			userlog(DEBUG_LOG, psName, "RefV : %d mV\n", refV/1000);
#endif
			myData->mData.testCond[bd][ch].step[i].refV = refV;

			memset(buf, 0x00, sizeof(buf));
			memcpy(buf, (char *)&cmd.step.Pref, sizeof(cmd.step.Pref));
			refP = atoi(buf); 

#ifdef DEBUG
			userlog(DEBUG_LOG, psName, "Ref Power : %d\n", refP);
#endif

			memset(buf, 0x00, sizeof(buf));
			memcpy(buf, (char *)&cmd.step.Iref, sizeof(cmd.step.Iref));
			refI = atoi(buf) * 1000; 
			refI = labs(refI);
#ifdef DEBUG
			userlog(DEBUG_LOG, psName, "RefI : %d mA\n", refI/1000);
#endif

			if(parallel){
				if(myData->mData.testCond[bd][ch].step[i].mode == CP) {
					myData->mData.testCond[bd][ch].step[i].refP = 
						(long)refP / 2 ; 
				} else {
					refI =  (long)refI / 2;
				}
			}else{
				myData->mData.testCond[bd][ch].step[i].refP = refP;
			}

			if(labs(refI) > myData->mData.config.maxCurrent[0]){ // Ref Filtering
				userlog(DEBUG_LOG, psName, 
						"RefI Value Out of Range : %d\n", refI);
					return send_cmd_trouble(EP_CD_CH_STATE_ERROR);
			}

			if((myData->mData.testCond[bd][ch].step[i].type == STEP_DISCHARGE) 
			 || (myData->mData.testCond[bd][ch].step[i].type == STEP_Z)){
				myData->mData.testCond[bd][ch].step[i].refI = refI * (-1);
			}else{
				myData->mData.testCond[bd][ch].step[i].refI = refI;
			}

			if(refI <= myData->mData.config.maxCurrent[3]) {
				rangeI = RANGE4 - 1;
			} else if(refI <= myData->mData.config.maxCurrent[2]) {
				rangeI = RANGE3 - 1;
			} else if(refI <= myData->mData.config.maxCurrent[1]) {
				rangeI = RANGE2 - 1;
			} else {
				rangeI = RANGE1 - 1;
			}
			if((rangeI+1) > (int)myData->mData.config.rangeI) {
				rangeI = (int)myData->mData.config.rangeI - 1;
			}
			myData->mData.testCond[bd][ch].step[i].rangeI = (unsigned char)rangeI;

#ifdef DEBUG
			userlog(DEBUG_LOG, psName, "rangeI : %d\n", rangeI+1);
#endif

			memset(buf, 0x00, sizeof(buf));
			memcpy(buf, (char *)&cmd.step.end_time_day, sizeof(cmd.step.end_time_day));
			tmp = atoi(buf) * 24 * 3600 * 100; 

			memset(buf, 0x00, sizeof(buf));
			memcpy(buf, (char *)&cmd.step.end_time_hour, sizeof(cmd.step.end_time_hour));
			tmp += atoi(buf) * 3600 * 100; 

			memset(buf, 0x00, sizeof(buf));
			memcpy(buf, (char *)&cmd.step.end_time_min, sizeof(cmd.step.end_time_min));
			tmp += atoi(buf) * 60 * 100; 


			memset(buf, 0x00, sizeof(buf));
			memcpy(buf, (char *)&cmd.step.end_time_sec, sizeof(cmd.step.end_time_sec));
			tmp += atoi(buf) * 100; 

			myData->mData.testCond[bd][ch].step[i].endT = tmp;

#ifdef DEBUG
			userlog(DEBUG_LOG, psName, "endT : %ld Sec\n", tmp/100);
#endif

			memset(buf, 0x00, sizeof(buf));
			memcpy(buf, (char *)&cmd.step.endV, sizeof(cmd.step.endV));
			tmp = atoi(buf) * 1000; 
			myData->mData.testCond[bd][ch].step[i].endV = tmp;

#ifdef DEBUG
			userlog(DEBUG_LOG, psName, "endV : %ld mV\n", tmp/1000);
#endif
			memset(buf, 0x00, sizeof(buf));
			memcpy(buf, (char *)&cmd.step.endI, sizeof(cmd.step.endI));
			tmp = atoi(buf) * 1000; 
			myData->mData.testCond[bd][ch].step[i].endI = tmp;

#ifdef DEBUG
			userlog(DEBUG_LOG, psName, "endI : %ld mA\n", tmp/1000);
#endif

			memset(buf, 0x00, sizeof(buf));
			memcpy(buf, (char *)&cmd.step.endC, sizeof(cmd.step.endC));
			tmp = atoi(buf)*1000; 
			myData->mData.testCond[bd][ch].step[i].endC = tmp;

#ifdef DEBUG
			userlog(DEBUG_LOG, psName, "endC : %ld\n", tmp);
#endif
			memset(buf, 0x00, sizeof(buf));
			memcpy(buf, (char *)&cmd.step.endW, sizeof(cmd.step.endW));
			tmp = atoi(buf); 
			myData->mData.testCond[bd][ch].step[i].endP = tmp;

#ifdef DEBUG
			userlog(DEBUG_LOG, psName, "endP : %ld\n", tmp);
#endif
			memset(buf, 0x00, sizeof(buf));
			memcpy(buf, (char *)&cmd.step.endWh, sizeof(cmd.step.endWh));
			tmp = atoi(buf); 
			myData->mData.testCond[bd][ch].step[i].endWh = tmp;

#ifdef DEBUG
			userlog(DEBUG_LOG, psName, "endWh : %ld\n", tmp);
#endif
			memset(buf, 0x00, sizeof(buf));
			memcpy(buf, (char *)&cmd.step.highV, sizeof(cmd.step.highV));
			tmp = atoi(buf) * 1000; 
			myData->mData.testCond[bd][ch].step[i].faultUpperV = tmp;

#ifdef DEBUG
			userlog(DEBUG_LOG, psName, "upperV : %ld mV\n", tmp/1000);
#endif
			memset(buf, 0x00, sizeof(buf));
			memcpy(buf, (char *)&cmd.step.lowV, sizeof(cmd.step.lowV));
			tmp = atoi(buf) * 1000; 
			myData->mData.testCond[bd][ch].step[i].faultLowerV = tmp;

#ifdef DEBUG
			userlog(DEBUG_LOG, psName, "lowerV : %ld mV\n", tmp/1000);
#endif
			memset(buf, 0x00, sizeof(buf));
			memcpy(buf, (char *)&cmd.step.highI, sizeof(cmd.step.highI));
			tmp = atoi(buf) * 1000; 
#ifdef DEBUG
			userlog(DEBUG_LOG, psName, "upperI : %ld mA\n", tmp/1000);
#endif

			if((myData->mData.testCond[bd][ch].step[i].type == STEP_DISCHARGE) 
			 || (myData->mData.testCond[bd][ch].step[i].type == STEP_Z)){
				tmp = tmp * (-1);
			}

			myData->mData.testCond[bd][ch].step[i].faultUpperI = tmp;
					
			memset(buf, 0x00, sizeof(buf));
			memcpy(buf, (char *)&cmd.step.lowI, sizeof(cmd.step.lowI));
			tmp = atoi(buf) * 1000; 
#ifdef DEBUG
			userlog(DEBUG_LOG, psName, "lowerI : %ld mA\n", tmp/1000);
#endif
			if((myData->mData.testCond[bd][ch].step[i].type == STEP_DISCHARGE) 
			 || (myData->mData.testCond[bd][ch].step[i].type == STEP_Z)){
				tmp = tmp * (-1);
			}
			myData->mData.testCond[bd][ch].step[i].faultLowerI = tmp;

			memset(buf, 0x00, sizeof(buf));
			memcpy(buf, (char *)&cmd.step.highC, sizeof(cmd.step.highC));
			tmp = atoi(buf)*1000; 
			myData->mData.testCond[bd][ch].step[i].faultUpperC = tmp;

#ifdef DEBUG
			userlog(DEBUG_LOG, psName, "upperC : %ld\n", tmp);
#endif
			memset(buf, 0x00, sizeof(buf));
			memcpy(buf, (char *)&cmd.step.lowC, sizeof(cmd.step.lowC));
			tmp = atoi(buf)*1000; 
			myData->mData.testCond[bd][ch].step[i].faultLowerC = tmp;

#ifdef DEBUG
			userlog(DEBUG_LOG, psName, "lowerC : %ld\n", tmp);
#endif
			memset(buf, 0x00, sizeof(buf));
			memcpy(buf, (char *)&cmd.step.recode_time_day, sizeof(cmd.step.recode_time_day));
			tmp = atoi(buf) * 24 * 3600 * 100; 

			memset(buf, 0x00, sizeof(buf));
			memcpy(buf, (char *)&cmd.step.recode_time_hour, sizeof(cmd.step.recode_time_hour));
			tmp += atoi(buf) * 3600 * 100; 

			memset(buf, 0x00, sizeof(buf));
			memcpy(buf, (char *)&cmd.step.recode_time_min, sizeof(cmd.step.recode_time_min));
			tmp += atoi(buf) * 60 * 100; 

			memset(buf, 0x00, sizeof(buf));
			memcpy(buf, (char *)&cmd.step.recode_time_sec, sizeof(cmd.step.recode_time_sec));
			tmp += atoi(buf) * 100; 

			memset(buf, 0x00, sizeof(buf));
			memcpy(buf, (char *)&cmd.step.recode_time_mSec, sizeof(cmd.step.recode_time_mSec));
			tmp += atoi(buf)/10;//10mS = 1

			myData->mData.testCond[bd][ch].step[i].saveDt = tmp;

#ifdef DEBUG
			userlog(DEBUG_LOG, psName, "saveDt : %ld mSec\n", tmp*10);
#endif
			memset(buf, 0x00, sizeof(buf));
			memcpy(buf, (char *)&cmd.step.deltaV, sizeof(cmd.step.deltaV));
			tmp = atoi(buf) * 1000; 
			myData->mData.testCond[bd][ch].step[i].saveDv = tmp;

#ifdef DEBUG
			userlog(DEBUG_LOG, psName, "saveDv : %ld mV\n", tmp/1000);
#endif
			memset(buf, 0x00, sizeof(buf));
			memcpy(buf, (char *)&cmd.step.deltaI, sizeof(cmd.step.deltaI));
			tmp = atoi(buf) * 1000; 
			myData->mData.testCond[bd][ch].step[i].saveDi = tmp;

#ifdef DEBUG
			userlog(DEBUG_LOG, psName, "saveDi : %ld mA\n", tmp/1000);
#endif
			memset(buf, 0x00, sizeof(buf));
			memcpy(buf, (char *)&cmd.step.current_cycle_no, sizeof(cmd.step.current_cycle_no));
			tmp = atoi(buf); 
			myData->bData[bd].cData[ch].misc.extCurrentCycle = tmp;

#ifdef DEBUG
			userlog(DEBUG_LOG, psName, "currentCycle : %ld\n", tmp);
#endif

			memset(buf, 0x00, sizeof(buf));
			memcpy(buf, (char *)&cmd.step.total_cycle_no, sizeof(cmd.step.total_cycle_no));
			tmp = atoi(buf); 
			myData->bData[bd].cData[ch].misc.extTotalCycle = tmp;

#ifdef DEBUG
			userlog(DEBUG_LOG, psName, "totalCycle : %ld\n", tmp);
#endif
		}
	}

	if(parallel){
		memcpy((char *)&myData->mData.testCond[bd][ch+1],
			(char *)&myData->mData.testCond[bd][ch], sizeof(S_TEST_CONDITION));
	}

	myPs->misc.rcvRecipeFlag[bd][ch] = P1;

	return send_cmd_response(SBC_TO_PC_CMD_STEP_DATA_RESPONSE, EXT_REPLY_NO);
	return 0;
}
