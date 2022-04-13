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

extern volatile S_SYSTEM_DATA *myData;
extern volatile S_FADM *myPs;
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
	int cmd_size=0, cmdBuf_index, tmp;

	if(myPs->rcvCmd.cmdBufSize < 1) return -1;
	
	if(myPs->rcvCmd.cmdBuf[0] == 0x02) {
		if(myPs->rcvCmd.cmdBufSize >= 12) {
			tmp = (myPs->rcvCmd.cmdBuf[8] - 0x30) * 10
				+ (myPs->rcvCmd.cmdBuf[9] - 0x30) + 12;
			if(myPs->rcvCmd.cmdBufSize >= tmp) cmd_size = tmp;
		}
		if(cmd_size == 0) return -2;
	} else {
		cmd_size = myPs->rcvCmd.cmdBufSize;
		cmdBuf_index = cmd_size;
		myPs->rcvCmd.cmdBufSize -= cmd_size;
		cmd_size = myPs->rcvCmd.cmdBufSize;
		memset((char *)&myPs->rcvCmd.tmpBuf[0], 0, MAX_SERIAL_PACKET_LENGTH);
		memcpy((char *)&myPs->rcvCmd.tmpBuf[0],
			(char *)&myPs->rcvCmd.cmdBuf[cmdBuf_index], cmd_size);
		memset((char *)&myPs->rcvCmd.cmdBuf[0], 0, MAX_SERIAL_PACKET_LENGTH);
		memcpy((char *)&myPs->rcvCmd.cmdBuf[0],
			(char *)&myPs->rcvCmd.tmpBuf[0], cmd_size);
		return -3;
	}
			
	memset((char *)&myPs->rcvCmd.cmd[0], 0, MAX_SERIAL_PACKET_LENGTH);
	memcpy((char *)&myPs->rcvCmd.cmd[0],
		(char *)&myPs->rcvCmd.cmdBuf[0], cmd_size);
	myPs->rcvCmd.cmdSize = cmd_size;
	
	cmdBuf_index = cmd_size;
	myPs->rcvCmd.cmdBufSize -= cmd_size;
	cmd_size = myPs->rcvCmd.cmdBufSize;
	memset((char *)&myPs->rcvCmd.tmpBuf[0], 0, MAX_SERIAL_PACKET_LENGTH);
	memcpy((char *)&myPs->rcvCmd.tmpBuf[0],
		(char *)&myPs->rcvCmd.cmdBuf[cmdBuf_index], cmd_size);
	memset((char *)&myPs->rcvCmd.cmdBuf[0], 0, MAX_SERIAL_PACKET_LENGTH);
	memcpy((char *)&myPs->rcvCmd.cmdBuf[0],
		(char *)&myPs->rcvCmd.tmpBuf[0], cmd_size);
	
	return 0;
}

int SerialCommand_Parsing(void)
{ //FADM_A_01
	int	 rtn, i;
	S_FADM_CMD_HEADER header;

	memset((char *)&header, 0, sizeof(S_FADM_CMD_HEADER));
	memcpy((char *)&header, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_FADM_CMD_HEADER));
	
	if(myPs->config.CmdRcvLog == P1) {
		if(myPs->config.CommCheckLog == P1) {
			userlog(METER2_LOG, psName, "recvCmd %s:end\n", myPs->rcvCmd.cmd);
		} else {
			userlog(METER2_LOG, psName, "recvCmd %s:end\n", myPs->rcvCmd.cmd);
		}
	}
	
	if(myPs->config.CmdRcvLog_Hex == P1) {
		if(myPs->config.CommCheckLog == P1) {
			userlog(METER2_LOG, psName, "recvCmd");
			for(i=0; i < myPs->rcvCmd.cmdSize; i++) {
				userlog2(METER2_LOG, psName, " %02x",
					(unsigned char)myPs->rcvCmd.cmd[i]);
			}
			userlog2(METER2_LOG, psName, ":end\n");
		} else {
			userlog(METER2_LOG, psName, "recvCmd");
			for(i=0; i < myPs->rcvCmd.cmdSize; i++) {
				userlog2(METER2_LOG, psName, " %02x",
					(unsigned char)myPs->rcvCmd.cmd[i]);
			}
			userlog2(METER2_LOG, psName, ":end\n");
		}
	}

	rtn = CmdHeader_Check((char *)&header);
	if(rtn < 0) return -1;

	rtn = CheckSum_Check();
	if(rtn < 0) return -2;
	
	switch((unsigned char)myPs->rcvCmd.cmd[7]) {
		case 0x81: //READ_CH1_AD_V_REPLY
			rcv_cmd_ad_reply(0, 0);
			break;
		case 0x85: //READ_CH2_AD_V_REPLY
			rcv_cmd_ad_reply(0, 1);
			break;
		case 0x83: //READ_CH1_AD_I_REPLY
			rcv_cmd_ad_reply(1, 0);
			break;
		case 0x87: //READ_CH2_AD_I_REPLY
			rcv_cmd_ad_reply(1, 1);
			break;
		case 0x89: //READ_CH1_AD_REF_V_P_REPLY
			rcv_cmd_ad_ref_reply(0, 0, 0);
			break;
		case 0x8B: //READ_CH1_AD_REF_V_N_REPLY
			rcv_cmd_ad_ref_reply(0, 0, 1);
			break;
		case 0x8D: //READ_CH1_AD_REF_V_0_REPLY
			rcv_cmd_ad_ref_reply(0, 0, 2);
			break;
		case 0x95: //READ_CH2_AD_REF_V_P_REPLY
			rcv_cmd_ad_ref_reply(0, 1, 0);
			break;
		case 0x97: //READ_CH2_AD_REF_V_N_REPLY
			rcv_cmd_ad_ref_reply(0, 1, 1);
			break;
		case 0x99: //READ_CH2_AD_REF_V_0_REPLY
			rcv_cmd_ad_ref_reply(0, 1, 2);
			break;
		case 0x8F: //READ_CH1_AD_REF_I_P_REPLY
			rcv_cmd_ad_ref_reply(1, 0, 0);
			break;
		case 0x91: //READ_CH1_AD_REF_I_N_REPLY
			rcv_cmd_ad_ref_reply(1, 0, 1);
			break;
		case 0x93: //READ_CH1_AD_REF_I_0_REPLY
			rcv_cmd_ad_ref_reply(1, 0, 2);
			break;
		case 0x9B: //READ_CH2_AD_REF_I_P_REPLY
			rcv_cmd_ad_ref_reply(1, 1, 0);
			break;
		case 0x9D: //READ_CH2_AD_REF_I_N_REPLY
			rcv_cmd_ad_ref_reply(1, 1, 1);
			break;
		case 0x9F: //READ_CH2_AD_REF_I_0_REPLY
			rcv_cmd_ad_ref_reply(1, 1, 2);
			break;
	}

	return 0;
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
				memset((char *)&myPs->rcvCmd.cmdBuf[0], 0,
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
	S_FADM_CMD_HEADER header;

	length = sizeof(S_FADM_CMD_HEADER);
	memset((char *)&header, 0, length);
	memcpy((char *)&header, (char *)rcvHeader, length);

	if(length >= myPs->rcvCmd.cmdSize) {
		userlog(DEBUG_LOG, psName, "RcvCmd size error (%d:%d)\n",
			length, myPs->rcvCmd.cmdSize);
		return -1;
	}
	
	if(header.stx != 0x02) {
		userlog(DEBUG_LOG, psName, "RcvCmd stx error : 0x%x\n", header.stx);
		return -2;
	}
	
	if(header.base_addr1 < 0x30 || header.base_addr1 > 0x39) {
		userlog(DEBUG_LOG, psName,
			"RcvCmd base_addr1 error : 0x%x\n", header.base_addr1);
		return -3;
	}
	if(header.base_addr2 < 0x30 || header.base_addr2 > 0x39) {
		userlog(DEBUG_LOG, psName,
			"RcvCmd base_addr2 error : 0x%x\n", header.base_addr2);
		return -4;
	}
	if(header.base_addr3 < 0x30 || header.base_addr3 > 0x39) {
		userlog(DEBUG_LOG, psName,
			"RcvCmd base_addr3 error : 0x%x\n", header.base_addr3);
		return -5;
	}

	if(header.id_addr1 < 0x30 || header.id_addr1 > 0x39) {
		userlog(DEBUG_LOG, psName,
			"RcvCmd id_addr1 error : 0x%x\n", header.id_addr1);
		return -6;
	}
	if(header.id_addr2 < 0x30 || header.id_addr2 > 0x39) {
		userlog(DEBUG_LOG, psName,
			"RcvCmd id_addr2 error : 0x%x\n", header.id_addr2);
		return -7;
	}
	if(header.id_addr3 < 0x30 || header.id_addr3 > 0x39) {
		userlog(DEBUG_LOG, psName,
			"RcvCmd id_addr3 error : 0x%x\n", header.id_addr3);
		return -8;
	}
	return 0;
}

int CheckSum_Check(void)
{
	char bcc;
	int i, cmd_size;
	S_FADM_RCV_CMD_ANSWER answer;
	
	memset((char *)&answer, 0, sizeof(S_FADM_RCV_CMD_ANSWER));
	memcpy((char *)&answer, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_FADM_RCV_CMD_ANSWER));
	
	cmd_size = myPs->rcvCmd.cmdSize;

	bcc = 0x00;
	for(i=0; i < (cmd_size - 2); i++) {
		bcc ^= answer.data[i];
	}

	if(bcc == answer.data[cmd_size - 2]) {
		return 0;
	} else {
		userlog(DEBUG_LOG, psName, "RcvCmd CheckSum error (%02x:%02x)\n",
			(unsigned char)bcc, (unsigned char)answer.data[cmd_size - 2]);
		return -1;
	}
}

void rcv_cmd_ad_reply(int type, int ch)
{
	unsigned char tmp;
	short int val;
	int i, bd, id, count;
	S_FADM_RCV_CMD_ANSWER answer;
	
	memset((char *)&answer, 0, sizeof(S_FADM_RCV_CMD_ANSWER));
	memcpy((char *)&answer, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_FADM_RCV_CMD_ANSWER));

	id = (int)(answer.data[4] - 0x30) * 100;
	id += (int)(answer.data[5] - 0x30) * 10;
	id += (int)(answer.data[6] - 0x30);
	ch += (id * 2);
	bd = ch / myData->mData.config.chPerBd;
	ch = ch % myData->mData.config.chPerBd;
	
	count = (int)(answer.data[10] - 0x30) * 30;
	for(i=0; i < 60; i++) {
		if((i % 2) != 0) {
			tmp = answer.data[11 + i];
			if((tmp & 0x80) == 0) {
				tmp |= 0x80;
			} else {
				tmp &= 0x7F;
			}
			val = (short int)tmp << 8;
			val += (unsigned char)answer.data[11 + i - 1];
			myPs->pulse_ad_org[bd][ch].value[type][count] = val;
			count++;
		}
	}

	if(myPs->signal[FADM_SIG_MEASURE] >= P11
		&& myPs->signal[FADM_SIG_MEASURE] <= P19) {
		myPs->signal[FADM_SIG_MEASURE] -= 9;
	} else if(myPs->signal[FADM_SIG_MEASURE] == P20) {
		myPs->signal[FADM_SIG_MEASURE] = P21;
	} else if(myPs->signal[FADM_SIG_MEASURE] >= P31
		&& myPs->signal[FADM_SIG_MEASURE] <= P39) {
		myPs->signal[FADM_SIG_MEASURE] -= 9;
	} else if(myPs->signal[FADM_SIG_MEASURE] == P40) {
		myPs->signal[FADM_SIG_MEASURE] = P41;
	}
}

void rcv_cmd_ad_ref_reply(int type, int ch, int index)
{
	unsigned char tmp;
	short int val;
	int i, bd, id, count;
	S_FADM_RCV_CMD_ANSWER answer;
	
	memset((char *)&answer, 0, sizeof(S_FADM_RCV_CMD_ANSWER));
	memcpy((char *)&answer, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_FADM_RCV_CMD_ANSWER));

	id = (int)(answer.data[4] - 0x30) * 100;
	id += (int)(answer.data[5] - 0x30) * 10;
	id += (int)(answer.data[6] - 0x30);
	ch += (id * 2);
	bd = ch / myData->mData.config.chPerBd;
	ch = ch % myData->mData.config.chPerBd;
	
	count = 0;
	for(i=0; i < 40; i++) {
		if((i % 2) != 0) {
			tmp = answer.data[10 + i];
			if((tmp & 0x80) == 0) {
				tmp |= 0x80;
			} else {
				tmp &= 0x7F;
			}
			val = (short int)tmp << 8;
			val += (unsigned char)answer.data[10 + i - 1];
			myPs->pulse_ad_org[bd][ch].ref[type][index][count] = val;
			count++;
		}
	}

	if(myPs->signal[FADM_SIG_MEASURE] >= P51
		&& myPs->signal[FADM_SIG_MEASURE] <= P55) {
		myPs->signal[FADM_SIG_MEASURE] -= 9;
	} else if(myPs->signal[FADM_SIG_MEASURE] == P56) {
		myPs->signal[FADM_SIG_MEASURE] = P60;
	}
}

void send_cmd_request(int ch, int val)
{
	int rtn, cmd_size, body_size, base_addr, id_addr;
	S_FADM_SEND_CMD	cmd;
	
	memset((char *)&cmd, 0, sizeof(S_FADM_SEND_CMD));

	base_addr = 0;
	id_addr = ch;

	make_header((char *)&cmd, base_addr, id_addr);

	if(val >= 0 && val <= 15) {
		if(val == 0) {
			cmd.data[7] = 0x88; //READ_CH1_AD_REF_V_P
		} else if(val == 1) {
			cmd.data[7] = 0x8A; //READ_CH1_AD_REF_V_N
		} else if(val == 2) {
			cmd.data[7] = 0x8C; //READ_CH1_AD_REF_V_0
		} else if(val == 3) {
			cmd.data[7] = 0x8E; //READ_CH1_AD_REF_I_P
		} else if(val == 4) {
			cmd.data[7] = 0x90; //READ_CH1_AD_REF_I_N
		} else if(val == 5) {
			cmd.data[7] = 0x92; //READ_CH1_AD_REF_I_0
		} else if(val == 10) {
			cmd.data[7] = 0x94; //READ_CH2_AD_REF_V_P
		} else if(val == 11) {
			cmd.data[7] = 0x96; //READ_CH2_AD_REF_V_N
		} else if(val == 12) {
			cmd.data[7] = 0x98; //READ_CH2_AD_REF_V_0
		} else if(val == 13) {
			cmd.data[7] = 0x9A; //READ_CH2_AD_REF_I_P
		} else if(val == 14) {
			cmd.data[7] = 0x9C; //READ_CH2_AD_REF_I_N
		} else if(val == 15) {
			cmd.data[7] = 0x9E; //READ_CH2_AD_REF_I_0
		} else {
			return;
		}
		body_size = 0;
		cmd.data[8] = (char)(body_size / 10 + 0x30);
		cmd.data[9] = (char)(body_size % 10 + 0x30);
	} else if(val >= 100 && val <= 499) {
		if((val / 100) == 1) {
			cmd.data[7] = 0x80; //READ_CH1_AD_V
		} else if((val / 100) == 2) {
			cmd.data[7] = 0x82; //READ_CH1_AD_I
		} else if((val / 100) == 3) {
			cmd.data[7] = 0x84; //READ_CH2_AD_V
		} else if((val / 100) == 4) {
			cmd.data[7] = 0x86; //READ_CH2_AD_I
		} else {
			return;
		}
		body_size = 1;
		cmd.data[8] = (char)(body_size / 10 + 0x30);
		cmd.data[9] = (char)(body_size % 10 + 0x30);
		cmd.data[10] = (char)(val % 100 + 0x30);
	} else {
		return;
	}

	cmd_size = 12 + body_size;
	make_check_sum((char *)&cmd, cmd_size);
	
	rtn = send_command((char *)&cmd, cmd_size, 0);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "send cmd error !!!\n");
	}
}

void send_cmd_comm_check(int ch, int val)
{
	int rtn, cmd_size, body_size, base_addr, id_addr;
	S_FADM_SEND_CMD	cmd;
	
	memset((char *)&cmd, 0, sizeof(S_FADM_SEND_CMD));

	base_addr = 0;
	id_addr = ch;

	make_header((char *)&cmd, base_addr, id_addr);

	cmd.data[7] = 0xE0;
	body_size = 0;
	cmd.data[8] = (char)(body_size / 10 + 0x30);
	cmd.data[9] = (char)(body_size % 10 + 0x30);

	cmd_size = 12 + body_size;
	make_check_sum((char *)&cmd, cmd_size);
	
	rtn = send_command((char *)&cmd, cmd_size, 0);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "send cmd error !!!\n");
	}
}

void send_cmd_wr_base_addr(int ch, int val)
{
	int rtn, cmd_size, body_size, base_addr, id_addr;
	S_FADM_SEND_CMD	cmd;
	
	memset((char *)&cmd, 0, sizeof(S_FADM_SEND_CMD));

	base_addr = 0;
	id_addr = ch;

	make_header((char *)&cmd, base_addr, id_addr);

	cmd.data[7] = 0xED;
	body_size = 3;
	cmd.data[8] = (char)(body_size / 10 + 0x30);
	cmd.data[9] = (char)(body_size % 10 + 0x30);

	cmd.data[10] = (char)(val / 100 + 0x30);
	cmd.data[11] = (char)((val % 100) / 10 + 0x30);
	cmd.data[12] = (char)((val % 100) % 10 + 0x30);

	cmd_size = 12 + body_size;
	make_check_sum((char *)&cmd, cmd_size);
	
	rtn = send_command((char *)&cmd, cmd_size, 0);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "send cmd error !!!\n");
	}
}

void make_header(char *cmd, int base_addr, int id_addr)
{
	S_FADM_CMD_HEADER header;
	
	header.stx = 0x02;
	header.base_addr1 = (char)(base_addr / 100 + 0x30);
	header.base_addr2 = (char)((base_addr % 100) / 10 + 0x30);
	header.base_addr3 = (char)((base_addr % 100) % 10 + 0x30);
	header.id_addr1 = (char)(id_addr / 100 + 0x30);
	header.id_addr2 = (char)((id_addr % 100) / 10 + 0x30);
	header.id_addr3 = (char)((id_addr % 100) % 10 + 0x30);
	
	memcpy(cmd, (char *)&header, sizeof(S_FADM_CMD_HEADER));
	
/*	userlog(METER2_LOG, psName, "header");
	for(i=0; i < sizeof(S_FADM_CMD_HEADER); i++) {
		userlog2(METER2_LOG, psName, " %02x", (unsigned char)*(cmd + i));
	}
	userlog2(METER2_LOG, psName, ":end\n");
	userlog(METER2_LOG, psName, "header %s:end\n", cmd); //kjgd*/
}

void make_check_sum(char *cmd, int cmd_size)
{
	char check_sum = 0x00;
	int i;
	
	for(i=0; i < (cmd_size - 2); i++) {
		check_sum ^= *(cmd + i);
	}
	
	*(cmd + cmd_size - 2) = check_sum;
	*(cmd + cmd_size - 1) = 0x03;
}

int	send_command(char *cmd, int size, int cmd_id)
{
	char packet[MAX_SERIAL_PACKET_LENGTH];
	int i;

	if(size > MAX_SERIAL_PACKET_LENGTH) {
		userlog(DEBUG_LOG, psName, "CMD SEND FAIL!! TOO LARGE SIZE:%d\n", size);
		return -1;
	}
	
	memset((char *)&packet[0], 0x00, MAX_SERIAL_PACKET_LENGTH);
	memcpy((char *)&packet[0], cmd, size);

	if(myPs->config.CmdSendLog == P1) {
		if(myPs->config.CommCheckLog == P1) {
			userlog(METER2_LOG, psName, "sendCmd %s:end\n", packet);
		} else {
			userlog(METER2_LOG, psName, "sendCmd %s:end\n", packet);
		}
	}
	
	if(myPs->config.CmdSendLog_Hex == P1) {
		if(myPs->config.CommCheckLog == P1) {
			userlog(METER2_LOG, psName, "sendCmd");
			for(i=0; i < size; i++) {
				userlog2(METER2_LOG, psName, " %02x",
					(unsigned char)*(cmd + i));
			}
			userlog2(METER2_LOG, psName, ":end\n");
		} else {
			userlog(METER2_LOG, psName, "sendCmd");
			for(i=0; i < size; i++) {
				userlog2(METER2_LOG, psName, " %02x",
					(unsigned char)*(cmd + i));
			}
			userlog2(METER2_LOG, psName, ":end\n");
		}
	}

	i = writen(myPs->config.ttyS_fd, packet, size);
	tcdrain(myPs->config.ttyS_fd);
	//usleep(100000);
	usleep(20000);
	return i;
}

int ComPortStateCheck(void)
{
	return 0;
}
