#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "../../INC/datastore.h"
#include "userlog.h"
#include "common_utils.h"
#include "local_utils.h"
#include "message.h"
#include "com_io.h"
#include "com_socket.h"
#include "network.h"
#include <math.h> //lyh_20220107 

extern volatile S_SYSTEM_DATA *myData;
extern volatile S_DAQ_CLIENT *myPs;
extern char psName[PROCESS_NAME_SIZE];

int InitNetwork(void)
{
	if(myPs->config.network_socket > 0)
		close(myPs->config.network_socket);
	
    myPs->config.network_socket
		= SetClientSock(myPs->config.networkPort,
		(char *)&myPs->config.ipAddr);

    if(myPs->config.network_socket < 0) {
		close(myPs->config.network_socket);
	    userlog(DEBUG_LOG, psName, "Can not initialize network : %d\n",
			myPs->config.network_socket);
		return -1;
    }

	myPs->netTimer = myData->mData.misc.timer_1sec;
	myPs->signal[DAQ_CLIENT_SIG_NET_CONNECTED] = P1;

	userlog(DEBUG_LOG, psName, "command socket connected : %d\n",
		myPs->config.network_socket);
    return 0;
}

int NetworkPacket_Receive(void)
{
	int rcv_size, read_size, i, start, index;
	char maxPacketBuf[MAX_DAQ_CLIENT_PACKET_LENGTH];

	memset(maxPacketBuf, 0x00, MAX_DAQ_CLIENT_PACKET_LENGTH);
		
	if(ioctl(myPs->config.network_socket, FIONREAD, &rcv_size) < 0) {
		userlog(DEBUG_LOG, psName, "packet receive ioctl error\n");
		close(myPs->config.network_socket);
		return -1;
	}

	if(rcv_size > MAX_DAQ_CLIENT_PACKET_LENGTH) {
		userlog(DEBUG_LOG, psName, "max packet size over\n");
		read_size = readn(myPs->config.network_socket, maxPacketBuf,
			MAX_DAQ_CLIENT_PACKET_LENGTH);
		if(read_size != MAX_DAQ_CLIENT_PACKET_LENGTH)
			userlog(DEBUG_LOG, psName, "packet readn size error1\n");
		close(myPs->config.network_socket);
		return -2;
	} else if(rcv_size > (MAX_DAQ_CLIENT_PACKET_LENGTH
		- myPs->rcvPacket.usedBufSize)) {
		userlog(DEBUG_LOG, psName, "packet buffer overflow\n");
		read_size = readn(myPs->config.network_socket, maxPacketBuf, rcv_size);
		if(read_size != rcv_size)
			userlog(DEBUG_LOG, psName, "packet readn size error2\n");
		close(myPs->config.network_socket);
		return -3;
	} else if(rcv_size <= 0) {
		userlog(DEBUG_LOG, psName, "packet sock_rcv error %d\n", rcv_size);
		myData->AppControl.signal[myPs->misc.psSignal] = P2;
		close(myPs->config.network_socket); //
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
	
	i = myPs->rcvPacket.rcvCount;
	myPs->rcvPacket.rcvCount++;
	if(myPs->rcvPacket.rcvCount > (MAX_DAQ_CLIENT_PACKET_COUNT-1))
		myPs->rcvPacket.rcvCount = 0;
	
	if(i == 0) index = MAX_DAQ_CLIENT_PACKET_COUNT - 1;
	else index = i - 1;
	start = myPs->rcvPacket.rcvStartPoint[index]
		+ myPs->rcvPacket.rcvSize[index];
	if(start >= MAX_DAQ_CLIENT_PACKET_LENGTH) {
		myPs->rcvPacket.rcvStartPoint[i] = abs(start - MAX_DAQ_CLIENT_PACKET_LENGTH);
	} else {
		myPs->rcvPacket.rcvStartPoint[i] = start;
	}

	myPs->rcvPacket.rcvSize[i] = read_size;
	myPs->rcvPacket.usedBufSize += read_size;
	
	start = myPs->rcvPacket.rcvStartPoint[i];
	if((start + read_size) > MAX_DAQ_CLIENT_PACKET_LENGTH) {
		index = MAX_DAQ_CLIENT_PACKET_LENGTH - start;
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
	if(myPs->rcvPacket.parseCount > (MAX_DAQ_CLIENT_PACKET_COUNT-1))
		myPs->rcvPacket.parseCount = 0;
	
	cmdBuf_index = myPs->rcvCmd.cmdBufSize;
	myPs->rcvCmd.cmdBufSize
		+= myPs->rcvPacket.rcvSize[i];
	
	start_point = myPs->rcvPacket.parseStartPoint[i];

	j = start_point + myPs->rcvPacket.rcvSize[i];
	if(j <= MAX_DAQ_CLIENT_PACKET_LENGTH) {
		memcpy((char *)&myPs->rcvCmd.cmdBuf[cmdBuf_index],
			(char *)&myPs->rcvPacket.rcvPacketBuf[start_point],
			myPs->rcvPacket.rcvSize[i]);
	} else {
		k = MAX_DAQ_CLIENT_PACKET_LENGTH - start_point;
		memcpy((char *)&myPs->rcvCmd.cmdBuf[cmdBuf_index],
			(char *)&myPs->rcvPacket.rcvPacketBuf[start_point], k);
		cmdBuf_index += k;
		start_point = 0;
		k = j - MAX_DAQ_CLIENT_PACKET_LENGTH;
		memcpy((char *)&myPs->rcvCmd.cmdBuf[cmdBuf_index],
			(char *)&myPs->rcvPacket.rcvPacketBuf[start_point], k);
	}
		
	start_point = myPs->rcvPacket.parseStartPoint[i]
		+ myPs->rcvPacket.rcvSize[i];
	if(start_point >= MAX_DAQ_CLIENT_PACKET_LENGTH) {
		j = i + 1;
		if(j >= MAX_DAQ_CLIENT_PACKET_COUNT) j = 0;
		myPs->rcvPacket.parseStartPoint[j]
			= abs(start_point - MAX_DAQ_CLIENT_PACKET_LENGTH);
	} else {
		j = i + 1;
		if(j >= MAX_DAQ_CLIENT_PACKET_COUNT) j = 0;
		myPs->rcvPacket.parseStartPoint[j] = start_point;
	}
		
	myPs->rcvPacket.usedBufSize -= myPs->rcvPacket.rcvSize[i];
}

int NetworkCommand_Receive(void)
{
	int cmd_size, cmdBuf_index;
	S_DAQ_CLIENT_CMD_HEADER header;

	myData->test_val_l[0]++;
	if(myPs->rcvCmd.cmdBufSize < sizeof(S_DAQ_CLIENT_CMD_HEADER)) return -1;
	
	memcpy((char *)&header, (char *)&myPs->rcvCmd.cmdBuf[0],
		sizeof(S_DAQ_CLIENT_CMD_HEADER));
	cmd_size = header.cmd_size;
	myData->test_val_l[1] = cmd_size;
	myData->test_val_l[2] = myPs->rcvCmd.cmdBufSize;
	if(myPs->rcvCmd.cmdBufSize < cmd_size) return -2;
			
	memset((char *)&myPs->rcvCmd.cmd[0], 0x00, MAX_DAQ_CLIENT_PACKET_LENGTH);
	memcpy((char *)&myPs->rcvCmd.cmd[0],
		(char *)&myPs->rcvCmd.cmdBuf[0], cmd_size);
	myPs->rcvCmd.cmdSize = cmd_size;
	
	cmdBuf_index = cmd_size;
	myPs->rcvCmd.cmdBufSize -= cmd_size;
	cmd_size = myPs->rcvCmd.cmdBufSize;
	memset((char *)&myPs->rcvCmd.tmpBuf[0], 0x00, MAX_DAQ_CLIENT_PACKET_LENGTH);
	memcpy((char *)&myPs->rcvCmd.tmpBuf[0],
		(char *)&myPs->rcvCmd.cmdBuf[cmdBuf_index], cmd_size);
	memset((char *)&myPs->rcvCmd.cmdBuf[0], 0x00, MAX_DAQ_CLIENT_PACKET_LENGTH);
	memcpy((char *)&myPs->rcvCmd.cmdBuf[0],
		(char *)&myPs->rcvCmd.tmpBuf[0], cmd_size);
	
	return 0;
}

int NetworkCommand_Parsing(void)
{
	unsigned char tmp;
	int	rtn, i;
	S_DAQ_CLIENT_CMD_HEADER	header;
	S_DAQ_CLIENT_RCV_CMD_RESPONSE	cmd;

	memset((char *)&header, 0x00, sizeof(S_DAQ_CLIENT_CMD_HEADER));
	memcpy((char *)&header, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_DAQ_CLIENT_CMD_HEADER));
	
	if(myPs->config.CmdRcvLog == P1) {
		if(myPs->config.CommCheckLog == P1) {
			userlog(DEBUG_LOG, psName, "recvCmd %s:end\n", myPs->rcvCmd.cmd);
		} else {
			if(header.cmd_id == TEMP_TO_SBC_CMD_COMM_CHECK) {
			} else if(header.cmd_id == SBC_TO_TEMP_CMD_RESPONSE) {
				memset((char *)&cmd, 0x00, sizeof(S_DAQ_CLIENT_RCV_CMD_RESPONSE));
				memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
					sizeof(S_DAQ_CLIENT_RCV_CMD_RESPONSE));
				if(cmd.code != DAQ_CLIENT_CD_ACK) {
					userlog(DEBUG_LOG, psName, "recvCmd %s:end\n",
						myPs->rcvCmd.cmd);
				}
			} else {
				userlog(DEBUG_LOG, psName, "recvCmd %s:end\n", myPs->rcvCmd.cmd);
			}
		}
	}
	
	if(myPs->config.CmdRcvLog_Hex == P1) {
		if(myPs->config.CommCheckLog == P1) {
			userlog(DEBUG_LOG, psName, "recvCmd");
			for(i=0; i < myPs->rcvCmd.cmdSize; i++) {
				tmp = myPs->rcvCmd.cmd[i];
				userlog2(DEBUG_LOG, psName, " %02x", tmp);
			}
			userlog2(DEBUG_LOG, psName, ":end\n");
		} else {
			if(header.cmd_id == TEMP_TO_SBC_CMD_COMM_CHECK) {
			} else if(header.cmd_id == SBC_TO_TEMP_CMD_RESPONSE) {
				memset((char *)&cmd, 0x00, sizeof(S_DAQ_CLIENT_RCV_CMD_RESPONSE));
				memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
					sizeof(S_DAQ_CLIENT_RCV_CMD_RESPONSE));
				if(cmd.code != DAQ_CLIENT_CD_ACK) {
					userlog(DEBUG_LOG, psName, "recvCmd");
					for(i=0; i < myPs->rcvCmd.cmdSize; i++) {
						tmp = myPs->rcvCmd.cmd[i];
						userlog2(DEBUG_LOG, psName, " %02x", tmp);
					}
					userlog2(DEBUG_LOG, psName, ":end\n");
				}
			} else {
				userlog(DEBUG_LOG, psName, "recvCmd");
				for(i=0; i < myPs->rcvCmd.cmdSize; i++) {
					tmp = myPs->rcvCmd.cmd[i];
					userlog2(DEBUG_LOG, psName, " %02x", tmp);
				}
				userlog2(DEBUG_LOG, psName, ":end\n");
			}
		}
	}
	
	rtn = CmdHeader_Check((char *)&header);
	myData->test_val_l[3]++;
	if(rtn < 0) return -1;
	
	switch(header.cmd_id) {
		case TEMP_TO_SBC_CMD_RESPONSE: //1
		case TEMP_TO_SBC_CMD_MODULE_INFO://4208
			userlog(DEBUG_LOG, psName, "TEMP_TO_SBC_CMD_MODULE_INFO[%x]\n", header.cmd_id); 
			rtn = rcv_cmd_check(); 				break;		
		case TEMP_TO_SBC_CMD_HEARTBEAT: //4209
			userlog(DEBUG_LOG, psName, "TEMP_TO_SBC_CMD_HEARTBEAT[%x]\n", header.cmd_id); 	
			rtn = rcv_cmd_check(); 				break;
		case TEMP_TO_SBC_CMD_TEMP_DATA: //4302(17153)
			rtn = rcv_cmd_temp_data();			break;			
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
	if(NetworkCommand_Receive() >= 0) {
		if(NetworkCommand_Parsing() < 0) {
			myPs->rcvCmd.cmdFail++;
			if(myPs->rcvCmd.cmdFail >= 3) {
				myPs->rcvCmd.cmdFail = 0;
				myPs->rcvCmd.cmdBufSize = 0;
				memset((char *)&myPs->rcvCmd.cmdBuf[0], 0x00,
					MAX_DAQ_CLIENT_PACKET_LENGTH);
				rtn = -1;
			}
		} else {
			myPs->rcvCmd.cmdFail = 0;
		}
	}
	
	return rtn;
}
	
int CmdHeader_Check(char *rcvHeader)
{
	int length, skip = 0;
	S_DAQ_CLIENT_CMD_HEADER	header;
	
	memset((char *)&header, 0x00, sizeof(S_DAQ_CLIENT_CMD_HEADER));
	memcpy((char *)&header, (char *)rcvHeader, sizeof(S_DAQ_CLIENT_CMD_HEADER));
	
	switch(header.cmd_id) {
		case TEMP_TO_SBC_CMD_RESPONSE: //1
		case TEMP_TO_SBC_CMD_MODULE_INFO: //4208
			length = sizeof(S_DAQ_CLIENT_SEND_CMD_RESPONSE);
			break;
		case TEMP_TO_SBC_CMD_HEARTBEAT: //4209
			skip = 1;
			break;
		case TEMP_TO_SBC_CMD_TEMP_DATA://4302
			length = sizeof(S_DAQ_RCV_CMD_TEMP_DATA);
			break;					
		default:
			userlog(DEBUG_LOG, psName, "RcvCmd command id error %x\n", 
				header.cmd_id);
			return -1;
	}
	if(header.cmd_id != SBC_TO_TEMP_CMD_RESPONSE){
		if(header.group_id != myPs->config.groupId) { 
			userlog(DEBUG_LOG, psName, "RcvCmd(0x%x) group id error %d\n",
				header.cmd_id, header.group_id);
			send_cmd_response((char *)&header, DAQ_CLIENT_CD_GP_ID_ERROR);
			return -3;
		}
	}

	if(skip == 0 && length != myPs->rcvCmd.cmdSize) {
		userlog(DEBUG_LOG, psName, "RcvCmd(0x%x) size error (%d : %d)\n", header.cmd_id, length, myPs->rcvCmd.cmdSize);
		return -4;
	}

	return 0;
}

int Check_ReplyCmd(char *rcvHeader)
{
	int		rtn;
	S_DAQ_CLIENT_CMD_HEADER	header;

	memset((char *)&header, 0x00, sizeof(S_DAQ_CLIENT_CMD_HEADER));
	memcpy((char *)&header, (char *)rcvHeader, sizeof(S_DAQ_CLIENT_CMD_HEADER));
	
	rtn = 0;
	
	if(myPs->reply.timer_run == P1) {
		if(myPs->reply.retry.replyCmd == header.cmd_id) {
			myPs->reply.timer_run = P0;
			memset((char *)&myPs->reply.retry, 0x00, sizeof(S_DAQ_CLIENT_RETRY_DATA));
		}
	}
	return rtn;
}

int rcv_cmd_temp_data(void) 
{
	int i,group, cmdsize; 
	S_DAQ_RCV_CMD_TEMP_DATA	cmd;

	cmdsize = sizeof(S_DAQ_RCV_CMD_TEMP_DATA);
	
	memset((char *)&cmd, 0x00, cmdsize);
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],cmdsize);

	group = 0;
	for(i = 0; i < 256 ; i++) {
		myPs->temp_data[i] = cmd.data[i];
	
		if(myPs->temp_data[i] >= 4999999) { //not connect line ch
//			myPs->temp_data[i] = 4999999;
		}
		myPs->temp_data_R[i] = (myPs->th_table.R1 * myPs->temp_data[i]) 
			/ (myPs->th_table.Vref - myPs->temp_data[i]);	
		myPs->temp_data_log[i] = log(myPs->temp_data_R[i] / myPs->th_table.R25);	
		myPs->temp[i] = (float)((1/((1/(25+273.15))
			+(myPs->temp_data_log[i]/myPs->th_table.Beta))-273.15)*1000);
	}
	return 0;
}

int send_cmd_stop(int group)
{
	int cmd_id, cmd_size;
	S_DAQ_SEND_CMD_STOP cmd;

	cmd_id = SBC_TO_TEMP_CMD_STOP;
	cmd_size = sizeof(S_DAQ_SEND_CMD_STOP);
	memset((char *)&cmd, 0x00, cmd_size);

	make_header(cmd_id, cmd_size, (char *)&cmd);
	
	userlog(DEBUG_LOG, psName, "send_cmd_stop\n", group);

	return send_command((char *)&cmd);
}

int send_cmd_run(int group)
{
	int cmd_id, cmd_size;
	S_DAQ_SEND_CMD_RUN cmd;

	cmd_id = SBC_TO_TEMP_CMD_START;
	cmd_size = sizeof(S_DAQ_SEND_CMD_RUN);
	memset((char *)&cmd, 0x00, cmd_size);
	make_header(cmd_id, cmd_size, (char *)&cmd);
	
	group = 0;
	userlog(DEBUG_LOG, psName, "send_cmd_run\n", group);
	
	return send_command((char *)&cmd);

}

int rcv_cmd_check(void)
{
	S_DAQ_CLIENT_RCV_CMD_CHECK	cmd;

	memset((char *)&cmd, 0x00, sizeof(S_DAQ_CLIENT_RCV_CMD_CHECK));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_DAQ_CLIENT_RCV_CMD_CHECK));

	myPs->netTimer = myData->mData.misc.timer_1sec;
	myPs->signal[DAQ_CLIENT_SIG_NET_CONNECTED] = P1;

	userlog(DEBUG_LOG, psName, "rcv_cmd_check\n");	
	return send_cmd_response((char *)&cmd.header, DAQ_CLIENT_CD_ACK);
}

int send_cmd_response(char *rcvHeader, int code)
{
	int cmd_id, cmd_size;
	S_DAQ_CLIENT_SEND_CMD_RESPONSE cmd;
	S_DAQ_CLIENT_CMD_HEADER header;

	cmd_id = TEMP_TO_SBC_CMD_RESPONSE;
	cmd_size = sizeof(S_DAQ_CLIENT_SEND_CMD_RESPONSE);
	memset((char *)&cmd, 0x00, cmd_size);
	make_header2(cmd_id, cmd_size, (char *)&cmd, rcvHeader);
	
	memcpy((char *)&header, rcvHeader, sizeof(S_DAQ_CLIENT_CMD_HEADER));
	
	cmd.cmd_id = TEMP_TO_SBC_CMD_MODULE_INFO;
	cmd.code = 1;

	return send_command((char *)&cmd);
}

void make_header(int cmd_id, int cmd_size, char *cmd)
{
	S_DAQ_CLIENT_CMD_HEADER header;

	memset((char *)&header, 0x00, sizeof(S_DAQ_CLIENT_CMD_HEADER));
	
	header.group_id = (unsigned short)myPs->config.groupId;

	switch(cmd_id) { //cmd_id
		default:
			header.cmd_id = cmd_id;
			break;
	}

	header.cmd_size = cmd_size;

	memcpy(cmd, (char *)&header, sizeof(S_DAQ_CLIENT_CMD_HEADER));
}

void make_header2(int cmd_id, int cmd_size, char *cmd, char *rcvHeader)
{
	S_DAQ_CLIENT_CMD_HEADER header, rcvHeader2;

	memset((char *)&header, 0x00, sizeof(S_DAQ_CLIENT_CMD_HEADER));
	memcpy((char *)&rcvHeader2, rcvHeader, sizeof(S_DAQ_CLIENT_CMD_HEADER));
	
	header.group_id = (unsigned short)myPs->config.groupId;

	switch(cmd_id) { //cmd_id
		default:
			header.cmd_id = cmd_id;
			break;
	}

	header.cmd_size = cmd_size;
	memcpy(cmd, (char *)&header, sizeof(S_DAQ_CLIENT_CMD_HEADER));
}

int	send_command(char *cmd)
{
	char	packet[MAX_DAQ_CLIENT_PACKET_LENGTH];
	unsigned char tmp;
	int i, rtn;
	S_DAQ_CLIENT_CMD_HEADER header;

	memcpy((char *)&header, cmd, sizeof(S_DAQ_CLIENT_CMD_HEADER));
	
	if(header.cmd_size > MAX_DAQ_CLIENT_PACKET_LENGTH) {
		userlog(DEBUG_LOG, psName,
			"CMD SEND FAIL!! TOO LARGE SIZE[%d]\n", header.cmd_size);
		return -1;
	}

	memset((char *)&packet[0], 0x00, MAX_DAQ_CLIENT_PACKET_LENGTH);
	memcpy((char *)&packet[0], cmd, header.cmd_size);

	rtn = writen(myPs->config.network_socket, packet, header.cmd_size);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d %d\n",
			header.cmd_id, rtn);
		return rtn;
	}

	if(myPs->config.CmdSendLog == P1) {
		if(myPs->config.CommCheckLog == P1) {
			userlog(DEBUG_LOG, psName, "sendCmd %s:end\n", packet);
		} else {
		}
	}
	
	if(myPs->config.CmdSendLog_Hex == P1) {
		if(myPs->config.CommCheckLog == P1) {
			userlog(DEBUG_LOG, psName, "sendCmd");
			for(i=0; i < header.cmd_size; i++) {
				tmp = *(cmd + i);
				userlog2(DEBUG_LOG, psName, " %02x", tmp);
			}
			userlog2(DEBUG_LOG, psName, ":end\n");
		} else {
		}
	}
	return 0;
}

int Check_NetworkState(void)
{
	int group;
	group = (int)myPs->config.groupNo;
	
	check_cmd_reply_timeout();
	network_ping();
	if(check_network_timeout() < 0) {
		close(myPs->config.network_socket);
		userlog(DEBUG_LOG, psName, "network communication error3\n");
		myPs->signal[DAQ_CLIENT_SIG_NET_CONNECTED] = P0;
		return -1;
	}
	return 0;
}

void network_ping(void)
{
	int diff;

	if(myPs->reply.timer_run == P0) {
		diff = (int)(myData->mData.misc.timer_1sec - myPs->pingTimer);
		if(diff > myPs->config.pingTimeout) {
			myPs->pingTimer = myData->mData.misc.timer_1sec;
		}
	}
}

int	check_network_timeout(void)
{
	int diff, group;

	group = (int)myPs->config.groupNo;
	diff = (int)(myData->mData.misc.timer_1sec - myPs->netTimer);
	if(diff > myPs->config.netTimeout) return -1;
	return 0;
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
			memset((char *)&myPs->reply.retry, 0x00, sizeof(S_DAQ_CLIENT_RETRY_DATA));
		} else {
			myPs->reply.timer_run = P1;
			myPs->reply.timer = myData->mData.misc.timer_1sec;
			myPs->reply.retry.count++;
				
			if(myPs->config.CmdSendLog == P1) {
				userlog(DEBUG_LOG, psName, "retry %s\n", myPs->reply.retry.buf);
			} // hex

			rtn = writen(myPs->config.send_socket,
				(char *)&myPs->reply.retry.buf[0], myPs->reply.retry.size);
		}
	}
}
