#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "../../INC/datastore.h"
#include "userlog.h"
#include "common_utils.h"
#include "local_utils.h"
#include "message.h"
#include "com_io.h"
#include "comm.h"
#include "serial.h"

extern volatile S_SYSTEM_DATA 	*myData;
extern volatile S_METER  		*myPs;
extern char	psName[16];
extern CommParam comm_param;// = {"/dev/ttyS0", TMT_B96, TMT_FXONXOFF};

int ComPortInitialize(int tty, int bps)
{
	memset((char *)&comm_param.device[0], 0x00, COM_DEVICE_SIZE);
	switch(tty) {
		case 1:
			memcpy((char *)&comm_param.device[0], "/dev/ttyS0", 10);	break;
		case 2:
			memcpy((char *)&comm_param.device[0], "/dev/ttyS1", 10);	break;
		case 3:
			memcpy((char *)&comm_param.device[0], "/dev/ttyS2", 10);	break;
		case 4:
			memcpy((char *)&comm_param.device[0], "/dev/ttyS3", 10);	break;
		default:
			memcpy((char *)&comm_param.device[0], "/dev/ttyS0", 10);	break;
	}
	switch(bps) {
		case 300:	comm_param.bps = TMT_B3;	break;
		case 600:	comm_param.bps = TMT_B6;	break;
		case 1200:	comm_param.bps = TMT_B12;	break;
		case 2400:	comm_param.bps = TMT_B24;	break;
		case 4800:	comm_param.bps = TMT_B48;	break;
		case 9600:	comm_param.bps = TMT_B96;	break;
		case 19200:	comm_param.bps = TMT_B192;	break;
		case 38400:	comm_param.bps = TMT_B384;	break;
	}
	//comm_param.flow = TMT_FNONE;
	comm_param.flow = TMT_FXONXOFF;
	return opentty();
}
	
int SerialPacket_Receive(void)
{
	int rcv_size, read_size, i, start, index;
	char maxPacketBuf[MAX_SERIAL_PACKET_LENGTH];

	memset(maxPacketBuf, 0x00, MAX_SERIAL_PACKET_LENGTH);
		
	if(ioctl(myPs->config.ttyS_fd, FIONREAD, &rcv_size) < 0) {
		userlog(DEBUG_LOG, psName, "packet receive ioctl error\n");
		close(myPs->config.ttyS_fd);
		return -1;
	}

	if(rcv_size > MAX_SERIAL_PACKET_LENGTH) {
		userlog(DEBUG_LOG, psName, "max packet size over\n");
		read_size = readn(myPs->config.ttyS_fd, maxPacketBuf,
			MAX_SERIAL_PACKET_LENGTH);
		if(read_size != MAX_SERIAL_PACKET_LENGTH)
			userlog(DEBUG_LOG, psName, "packet readn size error1\n");
		close(myPs->config.ttyS_fd);
		return -2;
	} else if(rcv_size > (MAX_SERIAL_PACKET_LENGTH
		- myPs->rcvPacket.usedBufSize)) {
		userlog(DEBUG_LOG, psName, "packet buffer overflow\n");
		read_size = readn(myPs->config.ttyS_fd, maxPacketBuf,
			rcv_size);
		if(read_size != rcv_size)
			userlog(DEBUG_LOG, psName, "packet readn size error2\n");
		close(myPs->config.ttyS_fd);
		return -3;
	} else if(rcv_size <= 0) {
		userlog(DEBUG_LOG, psName, "packet sock_rcv error %d\n", rcv_size);
		close(myPs->config.ttyS_fd);
		return -4;
	} else {
		read_size = readn(myPs->config.ttyS_fd, maxPacketBuf,
			rcv_size);
		if(read_size != rcv_size) {
			userlog(DEBUG_LOG, psName, "packet readn size error3 : %d, %d\n",
				read_size, rcv_size);
			close(myPs->config.ttyS_fd);
			return -5;
		}
	}

	i = myPs->rcvPacket.rcvCount;
	myPs->rcvPacket.rcvCount++;
	if(myPs->rcvPacket.rcvCount > (MAX_SERIAL_PACKET_COUNT-1))
		myPs->rcvPacket.rcvCount = 0;
	
	if(i == 0) index = MAX_SERIAL_PACKET_COUNT - 1;
	else index = i - 1;
	start = myPs->rcvPacket.rcvStartPoint[index]
		+ myPs->rcvPacket.rcvSize[index];
	if(start >= MAX_SERIAL_PACKET_LENGTH) {
		myPs->rcvPacket.rcvStartPoint[i]
			= abs(start - MAX_SERIAL_PACKET_LENGTH);
	} else {
		myPs->rcvPacket.rcvStartPoint[i] = start;
	}

	myPs->rcvPacket.rcvSize[i] = read_size;
	myPs->rcvPacket.usedBufSize += read_size;
	
	start = myPs->rcvPacket.rcvStartPoint[i];
	if((start + read_size) > MAX_SERIAL_PACKET_LENGTH) {
		index = MAX_SERIAL_PACKET_LENGTH - start;
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

void SerialPacket_Parsing(void)
{
	int i, j, k, cmdBuf_index, start_point;
	
	if(myPs->rcvPacket.rcvCount
		== myPs->rcvPacket.parseCount) return;
	
	i = myPs->rcvPacket.parseCount;
	myPs->rcvPacket.parseCount++;
	if(myPs->rcvPacket.parseCount > (MAX_SERIAL_PACKET_COUNT-1))
		myPs->rcvPacket.parseCount = 0;
	
	cmdBuf_index = myPs->rcvCmd.cmdBufSize;
	myPs->rcvCmd.cmdBufSize
		+= myPs->rcvPacket.rcvSize[i];
	
	start_point = myPs->rcvPacket.parseStartPoint[i];
	
	j = start_point + myPs->rcvPacket.rcvSize[i];
	if(j <= MAX_SERIAL_PACKET_LENGTH) {
		memcpy((char *)&myPs->rcvCmd.cmdBuf[cmdBuf_index],
			(char *)&myPs->rcvPacket.rcvPacketBuf[start_point],
			myPs->rcvPacket.rcvSize[i]);
	} else {
		k = MAX_SERIAL_PACKET_LENGTH - start_point;
		memcpy((char *)&myPs->rcvCmd.cmdBuf[cmdBuf_index],
			(char *)&myPs->rcvPacket.rcvPacketBuf[start_point], k);
		cmdBuf_index += k;
		start_point = 0;
		k = j - MAX_SERIAL_PACKET_LENGTH;
		memcpy((char *)&myPs->rcvCmd.cmdBuf[cmdBuf_index],
			(char *)&myPs->rcvPacket.rcvPacketBuf[start_point], k);
	}
		
	start_point = myPs->rcvPacket.parseStartPoint[i]
		+ myPs->rcvPacket.rcvSize[i];
	if(start_point >= MAX_SERIAL_PACKET_LENGTH) {
		j = i + 1;
		if(j >= MAX_SERIAL_PACKET_COUNT) j = 0;
		myPs->rcvPacket.parseStartPoint[j]
			= abs(start_point - MAX_SERIAL_PACKET_LENGTH);
	} else {
		j = i + 1;
		if(j >= MAX_SERIAL_PACKET_COUNT) j = 0;
		myPs->rcvPacket.parseStartPoint[j] = start_point;
	}
		
	myPs->rcvPacket.usedBufSize
		-= myPs->rcvPacket.rcvSize[i];
}

int SerialCommand_Receive(void)
{
	int i, cmd_size=0, cmdBuf_index;

	if(myPs->rcvCmd.cmdBufSize < 1) return -1;
	
	if(myPs->rcvCmd.cmdBuf[0] == 0x21 || myPs->rcvCmd.cmdBuf[1] == 0x3F) {
		for(i=0; i < myPs->rcvCmd.cmdBufSize; i++) {
			if(myPs->rcvCmd.cmdBuf[i] == 0x0D) {
				cmd_size = i+1; //one base
				break;
			}
		}
		if(cmd_size == 0) return -2;
	} else if(myPs->rcvCmd.cmdBuf[0] == 0x24) {
		for(i=0; i < myPs->rcvCmd.cmdBufSize; i++) {
			if(myPs->rcvCmd.cmdBuf[i] == 0x0D) {
				cmd_size = i+1; //one base
				break;
			}
		}
	cmdBuf_index = cmd_size;
	myPs->rcvCmd.cmdBufSize -= cmd_size;
	cmd_size = myPs->rcvCmd.cmdBufSize;
	memset((char *)&myPs->rcvCmd.tmpBuf[0], 0x00,
		MAX_SERIAL_PACKET_LENGTH);
	memcpy((char *)&myPs->rcvCmd.tmpBuf[0],
		(char *)&myPs->rcvCmd.cmdBuf[cmdBuf_index], cmd_size);
	memset((char *)&myPs->rcvCmd.cmdBuf[0], 0x00,
		MAX_SERIAL_PACKET_LENGTH);
	memcpy((char *)&myPs->rcvCmd.cmdBuf[0],
		(char *)&myPs->rcvCmd.tmpBuf[0], cmd_size);
		return -4;
	} else {
		return -3;
	}

	memset((char *)&myPs->rcvCmd.cmd[0], 0x00,
		MAX_SERIAL_PACKET_LENGTH);
	memcpy((char *)&myPs->rcvCmd.cmd[0],
		(char *)&myPs->rcvCmd.cmdBuf[0], cmd_size);
	myPs->rcvCmd.cmdSize = cmd_size;
	
	cmdBuf_index = cmd_size;
	myPs->rcvCmd.cmdBufSize -= cmd_size;
	cmd_size = myPs->rcvCmd.cmdBufSize;
	memset((char *)&myPs->rcvCmd.tmpBuf[0], 0x00,
		MAX_SERIAL_PACKET_LENGTH);
	memcpy((char *)&myPs->rcvCmd.tmpBuf[0],
		(char *)&myPs->rcvCmd.cmdBuf[cmdBuf_index], cmd_size);
	memset((char *)&myPs->rcvCmd.cmdBuf[0], 0x00,
		MAX_SERIAL_PACKET_LENGTH);
	memcpy((char *)&myPs->rcvCmd.cmdBuf[0],
		(char *)&myPs->rcvCmd.tmpBuf[0], cmd_size);
	
	return 0;
}

int SerialCommand_Parsing(void)
{ //Duksung Power Supply Controller
	int		rtn, i;
	S_METER2_CMD_HEADER	header;

	memset((char *)&header, 0x00, sizeof(S_METER2_CMD_HEADER));
	memcpy((char *)&header, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_METER2_CMD_HEADER));
	
	if(myPs->config.CmdRcvLog == PHASE1) {
		if(myPs->config.CommCheckLog == PHASE1) {
			userlog(METER2_LOG, psName, "recvCmd %s:end\n", myPs->rcvCmd.cmd);
		} else {
			userlog(METER2_LOG, psName, "recvCmd %s:end\n", myPs->rcvCmd.cmd);
		}
	}
	
	if(myPs->config.CmdRcvLog_Hex == PHASE1) {
		if(myPs->config.CommCheckLog == PHASE1) {
			userlog(METER2_LOG, psName, "recvCmd");
			for(i=0; i < myPs->rcvCmd.cmdSize; i++) {
				userlog2(METER2_LOG, psName, " %x", myPs->rcvCmd.cmd[i]);
			}
			userlog2(METER2_LOG, psName, ":end\n");
		} else {
			userlog(METER2_LOG, psName, "recvCmd");
			for(i=0; i < myPs->rcvCmd.cmdSize; i++) {
				userlog2(METER2_LOG, psName, " %x", myPs->rcvCmd.cmd[i]);
			}
			userlog2(METER2_LOG, psName, ":end\n");
		}
	}

	rtn = CmdHeader_Check((char *)&header);
	if(rtn < 0) return -1;
	
/*	rtn = CheckSum_Check((char *)&header);
	if(rtn < 0) return -2; kjgw*/
	
	rtn = rcv_cmd_answer();

	return rtn;
}

int Parsing_SerialEvent(void)
{
	int rtn=0;

	SerialPacket_Parsing();
	if(SerialCommand_Receive() >= 0) {
		if(SerialCommand_Parsing() < 0) {
			myPs->rcvCmd.cmdFail++;
			if(myPs->rcvCmd.cmdFail >= 3) {
				myPs->rcvCmd.cmdFail = 0;
				myPs->rcvCmd.cmdBufSize = 0;
				memset((char *)&myPs->rcvCmd.cmdBuf[0], 0x00,
					MAX_SERIAL_PACKET_LENGTH);
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
	int	length;
	S_METER2_CMD_HEADER	header;

	length = sizeof(S_METER2_CMD_HEADER);
	memset((char *)&header, 0x00, length);
	memcpy((char *)&header, (char *)rcvHeader, length);
/*kjgw
	if(length <= myPs->rcvCmd.cmdSize) {
		userlog(DEBUG_LOG, psName, "RcvCmd size error (%d:%d)\n",
			length, myPs->rcvCmd.cmdSize);
		return -1;
	}*/
	
	if(header.stx != 0x02) {
		userlog(DEBUG_LOG, psName, "RcvCmd stx error : 0x%x\n", header.stx);
		return -2;
	}
	
	if(header.addr != 0x00) {
		userlog(DEBUG_LOG, psName, "RcvCmd address error : 0x%x\n",
			header.addr);
		return -3;
	}

	if(header.cmd != 0x11 && header.cmd != 0x13) {
		userlog(DEBUG_LOG, psName, "RcvCmd command error : 0x%x\n", header.cmd);
		return -4;
	}

	if(header.body_size != 0x08) {
		userlog(DEBUG_LOG, psName, "RcvCmd body size error : 0x%x\n",
			header.body_size);
		return -5;
	}

	return 0;
}

int rcv_cmd_answer(void)
{
	S_METER2_RCV_CMD_ANSWER	answer;
	
	memset((char *)&answer, 0x00, sizeof(S_METER2_RCV_CMD_ANSWER));
	memcpy((char *)&answer, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_METER2_RCV_CMD_ANSWER));
	
	userlog(DEBUG_LOG, psName, "RcvCmd : %x %x %x : %x %x %x %x %x\n",
		answer.header.stx, answer.header.addr, answer.header.cmd,
		answer.header.body_size, answer.body[0], answer.body[1], answer.body[2]);

	if(myPs->signal[METER_SIG_REQUEST_PHASE] == PHASE1) {
		myPs->signal[METER_SIG_REQUEST_PHASE] = PHASE2;
	} else if(myPs->signal[METER_SIG_REQUEST_PHASE] == PHASE3) {
		myPs->signal[METER_SIG_REQUEST_PHASE] = PHASE4;
	}

	return 0;
}

void send_cmd_request(void)
{
	unsigned char bcc;
	int rtn, cmd_size;
	S_METER2_SEND_CMD_REQUEST	cmd;
	
	memset((char *)&cmd, 0x00, sizeof(S_METER2_SEND_CMD_REQUEST));

	/*make_header((char *)&cmd, REPLY_NO,
		METER2_SEND_CMD_REQUEST, SEQNUM_DFLT, body_size); kjgw*/
	
	cmd_size = 5;
	cmd.data[0] = '$';	//STX
	bcc = cmd.data[0];
	cmd.data[1] = 0x30;	//ADDRESS1
	bcc ^= cmd.data[1];
	cmd.data[2] = 0x31; //ADDRESS2
	bcc ^= cmd.data[2];
	cmd.data[3] = 'F'; //COMMAND
	bcc ^= cmd.data[3];
	cmd.data[4] = 0x0D; //ETX
	
/*	cmd_size = 7;
	cmd.data[0] = '$';	//STX
	bcc = cmd.data[0];
	cmd.data[1] = 0x30;	//ADDRESS1
	bcc ^= cmd.data[1];
	cmd.data[2] = 0x31; //ADDRESS2
	bcc ^= cmd.data[2];
	cmd.data[3] = 'M'; //COMMAND
	bcc ^= cmd.data[3];
	cmd.data[4] = 0x00; //BCC
	cmd.data[5] = 0x00; //BCC
	cmd.data[6] = 0x0D; //ETX
*/	
	//kjgw make_check_sum((char *)&cmd, cmd_size);

	rtn = send_command((char *)&cmd, cmd_size, 0);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "send cmd error !!!\n");
	}
}

void send_cmd_request2(void)
{
	unsigned char bcc;
	int rtn, cmd_size;
	S_METER2_SEND_CMD_REQUEST	cmd;
	
	memset((char *)&cmd, 0x00, sizeof(S_METER2_SEND_CMD_REQUEST));

	/*make_header((char *)&cmd, REPLY_NO,
		METER2_SEND_CMD_REQUEST, SEQNUM_DFLT, body_size); kjgw*/
	
	cmd_size = 6;
	cmd.data[0] = 0x02;	//STX
	bcc = cmd.data[0];
	cmd.data[1] = 0x00;	//ADDRESS
	bcc ^= cmd.data[1];
	cmd.data[2] = 0x12; //CMD_READ_MAGNET_V
	bcc ^= cmd.data[2];
	cmd.data[3] = 0x00; //BODY SIZE
	bcc ^= cmd.data[3];
	cmd.data[4] = bcc; //BCC
	cmd.data[5] = 0x03; //ETX
	
	//kjgw make_check_sum((char *)&cmd, cmd_size);

	rtn = send_command((char *)&cmd, cmd_size, 0);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "send cmd error !!!\n");
	}
}

void make_header(char *cmd, char reply, int cmd_id, int seqno, int body_size)
{
	S_METER2_CMD_HEADER	header;
	
	memset((char *)&header, 0x00, sizeof(S_METER2_CMD_HEADER));
	
	memcpy(cmd, (char *)&header, sizeof(S_METER2_CMD_HEADER));
	
/*	userlog(METER2_LOG, psName, "header");
	for(i=0; i < sizeof(S_METER_CMD_HEADER); i++) {
		userlog2(METER2_LOG, psName, " %x", *(cmd + i));
	}
	userlog2(METER2_LOG, psName, ":end\n");
	userlog(METER2_LOG, psName, "header %s:end\n", cmd); //for debug kjg*/
}

void make_check_sum(char *cmd, int size)
{
	int i;
	char check_sum = 0x00;
	
	for(i=1; i < (size - 2); i++) {
		check_sum ^= *(cmd + i);
	}
	
	*(cmd + size - 1) = check_sum;
}

int	send_command(char *cmd, int size, int cmd_id)
{
	char	packet[MAX_SERIAL_PACKET_LENGTH];
	int i;

	if(size > MAX_SERIAL_PACKET_LENGTH) {
		userlog(DEBUG_LOG, psName, "CMD SEND FAIL!! TOO LARGE SIZE:%d\n", size);
		return -1;
	}
	
	memset((char *)&packet[0], 0x00, MAX_SERIAL_PACKET_LENGTH);
	memcpy((char *)&packet[0], cmd, size);

	if(myPs->config.CmdSendLog == PHASE1) {
		if(myPs->config.CommCheckLog == PHASE1) {
			userlog(METER2_LOG, psName, "sendCmd %s:end\n", packet);
		} else {
			userlog(METER2_LOG, psName, "sendCmd %s:end\n", packet);
		}
	}
	
	if(myPs->config.CmdSendLog_Hex == PHASE1) {
		if(myPs->config.CommCheckLog == PHASE1) {
			userlog(METER2_LOG, psName, "sendCmd");
			for(i=0; i < size; i++) {
				userlog2(METER2_LOG, psName, " %x", *(cmd + i));
			}
			userlog2(METER2_LOG, psName, ":end\n");
		} else {
			userlog(METER2_LOG, psName, "sendCmd");
			for(i=0; i < size; i++) {
				userlog2(METER2_LOG, psName, " %x", *(cmd + i));
			}
			userlog2(METER2_LOG, psName, ":end\n");
		}
	}

	i = writen(myPs->config.ttyS_fd, packet, size);
	tcdrain(myPs->config.ttyS_fd);
	usleep(100000);
	return i;
}

int ComPortStateCheck(void)
{
	return 0;
}
