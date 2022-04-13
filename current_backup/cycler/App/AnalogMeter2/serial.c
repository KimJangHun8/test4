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
extern volatile S_ANALOG_METER  *myPs;
extern char	psName[16];
extern CommParam comm_param;// = {"/dev/ttyS0", TMT_B96, TMT_FXONXOFF};

int ComPortInitialize()
{
	int tty , bps , commType, rtn;

	sleep(2);

	tty = myPs->config.comPort;
	if(myPs->config.functionType == 0) {
		bps = myPs->config.comBps;
	} else {
		bps = myPs->config.comBps2;
	}
	commType = (int)myPs->config.commType2;
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
	comm_param.flow = TMT_FNONE;
//	comm_param.flow = TMT_FXONXOFF;
	comm_param.type = commType;
	rtn = opentty();
	myPs->config.ttyS_fd = rtn;
// 130606 khk w	
	myPs->signal[ANALOG_METER_SIG_MEASURE] = P1;
	memset((char *)&myPs->rcvPacket, 0x00, sizeof(S_ANALOG_METER_RCV_PACKET));

	return rtn;
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
		memset((char *)&myPs->rcvPacket,0,sizeof(S_ANALOG_METER_RCV_PACKET));
		memset((char *)&myPs->rcvCmd,0,sizeof(S_ANALOG_METER_RCV_COMMAND));
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
	
	if(myPs->config.functionType == 0) { //Measurement Computing CB-7018
	
		if(myPs->rcvCmd.cmdBuf[0] == '!' || myPs->rcvCmd.cmdBuf[0] == '?'
			|| myPs->rcvCmd.cmdBuf[0] == '>') {
			for(i=0; i < myPs->rcvCmd.cmdBufSize; i++) {
				if(myPs->rcvCmd.cmdBuf[i] == '\r') {
					cmd_size = i+1; //one base
					break;
				}
			}
			if(cmd_size == 0) return -2;
		} else if(myPs->rcvCmd.cmdBuf[0] == '%' || myPs->rcvCmd.cmdBuf[0] == '$'
			|| myPs->rcvCmd.cmdBuf[0] == '#') {
			for(i=0; i < myPs->rcvCmd.cmdBufSize; i++) {
				if(myPs->rcvCmd.cmdBuf[i] == '\r') {
					cmd_size = i+1; //one base
					break;
				}
			}
	//		if(cmd_size == 0) return -3;
			cmdBuf_index = cmd_size;
			myPs->rcvCmd.cmdBufSize -= cmd_size;
			cmd_size = myPs->rcvCmd.cmdBufSize;
			memset((char *)&myPs->rcvCmd.tmpBuf[0],0x00,
				MAX_SERIAL_PACKET_LENGTH);
			memcpy((char *)&myPs->rcvCmd.tmpBuf[0],
				(char *)&myPs->rcvCmd.cmdBuf[cmdBuf_index], cmd_size);
			memset((char *)&myPs->rcvCmd.cmdBuf[0], 0x00,
				MAX_SERIAL_PACKET_LENGTH);
			memcpy((char *)&myPs->rcvCmd.cmdBuf[0],
				(char *)&myPs->rcvCmd.tmpBuf[0], cmd_size);
			return -3;
		} else {
			for(i=0; i < myPs->rcvCmd.cmdBufSize; i++) {
				if(myPs->rcvCmd.cmdBuf[i] == '\r') {
					cmd_size = i+1; //one base
					break;
				}
			}
	//		if(cmd_size == 0) return -4;
			cmdBuf_index = cmd_size;
			myPs->rcvCmd.cmdBufSize -= cmd_size;
			cmd_size = myPs->rcvCmd.cmdBufSize;
			memset((char *)&myPs->rcvCmd.tmpBuf[0],0x00,
				MAX_SERIAL_PACKET_LENGTH);
			memcpy((char *)&myPs->rcvCmd.tmpBuf[0],
				(char *)&myPs->rcvCmd.cmdBuf[cmdBuf_index], cmd_size);
			memset((char *)&myPs->rcvCmd.cmdBuf[0], 0x00,
				MAX_SERIAL_PACKET_LENGTH);
			memcpy((char *)&myPs->rcvCmd.cmdBuf[0],
				(char *)&myPs->rcvCmd.tmpBuf[0], cmd_size);
			return -4;
		}
	} else { //Yokogawa XL122
		if((myPs->rcvCmd.cmdBuf[0] == 0x1B && myPs->rcvCmd.cmdBuf[1] == 'O')
			|| (myPs->rcvCmd.cmdBuf[0] == 0x1B && myPs->rcvCmd.cmdBuf[1] == 'C')
			|| (myPs->rcvCmd.cmdBuf[0] == 'N' && myPs->rcvCmd.cmdBuf[1] == ' ')
			|| (myPs->rcvCmd.cmdBuf[0] == 'S' && myPs->rcvCmd.cmdBuf[1] == ' ')
			|| (myPs->rcvCmd.cmdBuf[0] == 'O' && myPs->rcvCmd.cmdBuf[1] == ' ')
			|| (myPs->rcvCmd.cmdBuf[0] == 'E' && myPs->rcvCmd.cmdBuf[1] == ' ')
			) {
			for(i=0; i < myPs->rcvCmd.cmdBufSize; i++) {
				if(myPs->rcvCmd.cmdBuf[i] == '\n') { //0x0A
					cmd_size = i+1; //one base
					break;
				}
			}
			if(cmd_size == 0) return -2;
		} else if(myPs->rcvCmd.cmdBuf[0] == 0x09
			|| (myPs->rcvCmd.cmdBuf[0] == 'E' && myPs->rcvCmd.cmdBuf[1] == '0')
			|| (myPs->rcvCmd.cmdBuf[0] == 'E' && myPs->rcvCmd.cmdBuf[1] == '1')
			|| (myPs->rcvCmd.cmdBuf[0] == 'E' && myPs->rcvCmd.cmdBuf[1] == '2')
			|| (myPs->rcvCmd.cmdBuf[0] == 'E' && myPs->rcvCmd.cmdBuf[1] == 'A')
			|| (myPs->rcvCmd.cmdBuf[0] == 'D' && myPs->rcvCmd.cmdBuf[1] == 'A')
			|| (myPs->rcvCmd.cmdBuf[0] == 'T' && myPs->rcvCmd.cmdBuf[1] == 'I')
			|| (myPs->rcvCmd.cmdBuf[0] == 'E' && myPs->rcvCmd.cmdBuf[1] == 'N')
			) {
			for(i=0; i < myPs->rcvCmd.cmdBufSize; i++) {
				if(myPs->rcvCmd.cmdBuf[i] == '\n') { //0x0A
					cmd_size = i+1; //one base
					break;
				}
			}
			if(cmd_size == 0) return -3;

			cmdBuf_index = cmd_size;
			myPs->rcvCmd.cmdBufSize -= cmd_size;
			cmd_size = myPs->rcvCmd.cmdBufSize;
			memset((char *)&myPs->rcvCmd.tmpBuf, 0, MAX_SERIAL_PACKET_LENGTH);
			memcpy((char *)&myPs->rcvCmd.tmpBuf,
				(char *)&myPs->rcvCmd.cmdBuf[cmdBuf_index], cmd_size);
			memset((char *)&myPs->rcvCmd.cmdBuf, 0, MAX_SERIAL_PACKET_LENGTH);
			memcpy((char *)&myPs->rcvCmd.cmdBuf,
				(char *)&myPs->rcvCmd.tmpBuf, cmd_size);
			return 1;
		} else {
			for(i=0; i < myPs->rcvCmd.cmdBufSize; i++) {
				if(myPs->rcvCmd.cmdBuf[i] == '\n') { //0x0A
					cmd_size = i+1; //one base
					break;
				}
			}
			if(cmd_size == 0) return -4;

			cmdBuf_index = cmd_size;
			myPs->rcvCmd.cmdBufSize -= cmd_size;
			cmd_size = myPs->rcvCmd.cmdBufSize;
			memset((char *)&myPs->rcvCmd.tmpBuf, 0, MAX_SERIAL_PACKET_LENGTH);
			memcpy((char *)&myPs->rcvCmd.tmpBuf,
				(char *)&myPs->rcvCmd.cmdBuf[cmdBuf_index], cmd_size);
			memset((char *)&myPs->rcvCmd.cmdBuf, 0, MAX_SERIAL_PACKET_LENGTH);
			memcpy((char *)&myPs->rcvCmd.cmdBuf,
				(char *)&myPs->rcvCmd.tmpBuf, cmd_size);

			return 2;
		}
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
{ //Measurement Computing CB-7018
	int		rtn, i;
	S_ANALOG_METER_CMD_HEADER	header;

	memset((char *)&header, 0x00, sizeof(S_ANALOG_METER_CMD_HEADER));
	memcpy((char *)&header, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_ANALOG_METER_CMD_HEADER));
	
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
	if(myPs->config.functionType == 0) { //Measurement Computing CB-7018
		rtn = CmdHeader_Check((char *)&header);
		if(rtn < 0) return -1;
		rtn = rcv_cmd_answer();
	} else { //Yokogawa XL122
		rtn = CmdHeader_Check2();
		if(rtn == 1) {
			rtn = rcv_cmd_open_close();
		} else if(rtn == 2) {
			rtn = rcv_cmd_answer2();
		} else rtn = (-1);
	}
	
	return rtn;
}

int Parsing_SerialEvent(void)
{
	int rtn=0;

	SerialPacket_Parsing();

	while(1) {
		rtn = SerialCommand_Receive();
		if(rtn < 0) break;
		else if (rtn > 0) continue;
		else {
			if(SerialCommand_Parsing() < 0){
				myPs->rcvCmd.cmdFail++;
				if(myPs->rcvCmd.cmdFail >= 3) {
					myPs->rcvCmd.cmdFail = 0;
					myPs->rcvCmd.cmdBufSize = 0;
					memset((char *)&myPs->rcvCmd.cmdBuf[0], 0x00,
						MAX_SERIAL_PACKET_LENGTH);
					rtn = -1;
					return rtn;
				}
			} else {
				myPs->rcvCmd.cmdFail = 0;
			}
		}
	}

	return rtn;
}

int CmdHeader_Check(char *rcvHeader)
{
	int	length;
	S_ANALOG_METER_CMD_HEADER	header;

	length = sizeof(S_ANALOG_METER_CMD_HEADER);
	memset((char *)&header, 0x00, length);
	memcpy((char *)&header, (char *)rcvHeader, length);

	if(length > myPs->rcvCmd.cmdSize) {
		userlog(DEBUG_LOG, psName, "RcvCmd size error (%d:%d)\n",
			length, myPs->rcvCmd.cmdSize);
		return -1;
	}
	
	if(header.stx != '!' && header.stx != '?' && header.stx != '>') {
		userlog(DEBUG_LOG, psName, "RcvCmd stx error : 0x%x\n", header.stx);
		return -2;
	}
	
	if(header.stx == '!' || header.stx == '?') {
		if(header.addr1 != '0') {
			userlog(DEBUG_LOG, psName, "RcvCmd addr1 error : 0x%x\n",
				header.addr1);
			return -3;
		}
		//check address 1 - 8 
	//	if(header.addr2 < 0x31 || header.addr2 > 0x38){
		//check address 1 - 10
		if(header.addr2 < 0x31 || header.addr2 > 0x41){
			userlog(DEBUG_LOG, psName, "RcvCmd addr2 error : 0x%x\n",
				header.addr2);
			return -4;
		}
		#if 0
		// Increase Analog Meter Module(CB-7018), Before support 4 analog 
		// meter module now we can support 8 analog meter module.
		if(header.addr2 != '1' && header.addr2 != '2'
			&& header.addr2 != '3' && header.addr2 != '4') {
			userlog(DEBUG_LOG, psName, "RcvCmd addr2 error : 0x%x\n",
				header.addr2);
			return -4;
		}
		#endif
	}

	return 0;
}

int CmdHeader_Check2(void)
{
	int	val;
	S_ANALOG_METER_RCV_CMD_ANSWER2	answer;
	
	//printf("cmdsize %d %d\n",myPs->rcvCmd.cmdSize,myPs->signal[ANALOG_METER_SIG_MEASURE]);
	if(myPs->rcvCmd.cmdSize == 7) {
		if((myPs->rcvCmd.cmd[0] == 0x1B)
			&& (myPs->rcvCmd.cmd[1] == 'O' || myPs->rcvCmd.cmd[1] == 'C')
			&& (myPs->rcvCmd.cmd[2] == ' ') && (myPs->rcvCmd.cmd[5] == 0x0D)) {
			return 1;
		} else {
			userlog(DEBUG_LOG, psName, "RcvCmd2 open_close error %d\n",
				myPs->signal[ANALOG_METER_SIG_MEASURE]);
			return -1;
		}
	} else if(myPs->rcvCmd.cmdSize == 27) {
		memcpy((char *)&answer, (char *)myPs->rcvCmd.cmd, 27);
	} else {
		/*kjgd userlog(DEBUG_LOG, psName, "RcvCmd2 size error1 %d : %d\n",
			myPs->signal[ANALOG_METER_SIG_MEASURE], myPs->rcvCmd.cmdSize);*/
		return -1;
	}

	if((answer.Status != 'N') && (answer.Status != 'S')
		&& (answer.Status != 'O') && (answer.Status != 'E')) {
		userlog(DEBUG_LOG, psName, "RcvCmd2 data Status error %d : 0x%x\n",
			myPs->signal[ANALOG_METER_SIG_MEASURE], answer.Status);
		return -2;
	}

	if(answer.SP1 != ' ') {
		userlog(DEBUG_LOG, psName, "RcvCmd2 data SP1 error %d : 0x%x\n",
			myPs->signal[ANALOG_METER_SIG_MEASURE], answer.SP1);
		return -3;
	}
	
	if(answer.AnalogChannel != '0') {
		userlog(DEBUG_LOG, psName, "RcvCmd2 data AnalogChannel error %d:0x%x\n",
			myPs->signal[ANALOG_METER_SIG_MEASURE], answer.AnalogChannel);
		return -4;
	}

	val = (((int)answer.CH[0] - 0x30) * 10);
	val += ((int)answer.CH[1] - 0x30);
	if((val < 1) || (val > 16)) {
		userlog(DEBUG_LOG, psName, "RcvCmd2 data CH error %d : %d : %x %x\n",
			myPs->signal[ANALOG_METER_SIG_MEASURE],
			val, answer.CH[0], answer.CH[1]);
		return -5;
	}

	if((answer.Alarm[1] != ' ') || (answer.Alarm[2] != ' ')
		|| (answer.Alarm[3] != ' ')) {
		userlog(DEBUG_LOG, psName, "RcvCmd2 data Alarm error %d:%x %x %x %x\n",
			myPs->signal[ANALOG_METER_SIG_MEASURE],
			answer.Alarm[0], answer.Alarm[1], answer.Alarm[2], answer.Alarm[3]);
		return -6;
	}
	
	if((answer.Units[0] != '^') || (answer.Units[1] != 'C')
		|| (answer.Units[2] != ' ') || (answer.Units[3] != ' ')
		|| (answer.Units[4] != ' ') || (answer.Units[5] != ' ')) {
		userlog(DEBUG_LOG, psName,
			"RcvCmd2 data Units error %d:%x %x %x %x %x %x\n",
			myPs->signal[ANALOG_METER_SIG_MEASURE],
			answer.Units[0], answer.Units[1], answer.Units[2],
			answer.Units[3], answer.Units[4], answer.Units[5]);
		return -7;
	}
	if((answer.Sign != '+') && (answer.Sign != '-')) {
		userlog(DEBUG_LOG, psName, "RcvCmd2 data Sign error %d : 0x%x\n",
			myPs->signal[ANALOG_METER_SIG_MEASURE], answer.Sign);
		return -8;
	}
	if(answer.E != 'E') {
		userlog(DEBUG_LOG, psName, "RcvCmd2 data E error %d : 0x%x\n",
			myPs->signal[ANALOG_METER_SIG_MEASURE], answer.E);
		return -9;
	}
	
	if((answer.Plus_Minus != '+') && (answer.Plus_Minus != '-')) {
		userlog(DEBUG_LOG, psName, "RcvCmd2 data Plus_Minus error %d : 0x%x\n",
			myPs->signal[ANALOG_METER_SIG_MEASURE], answer.Plus_Minus);
		return -10;
	}

	if(answer.A_CR != 0x0D) {
		userlog(DEBUG_LOG, psName, "RcvCmd2 data CR error %d : 0x%x\n",
			myPs->signal[ANALOG_METER_SIG_MEASURE], answer.A_CR);
		return -11;
	}
	
	return 2;
}

//hun_210824
int rcv_cmd_answer(void)
{
	char buf[10], data_size;
	int i, j, index, temp_bd, temp_ch, sum=0;
	int	chPerModule = 0;	
	long temp[myPs->config.chPerModule];
	char checksum0 , checksum1;

	S_ANALOG_METER_RCV_CMD_ANSWER	answer;
	
	memset((char *)&answer, 0, sizeof(S_ANALOG_METER_RCV_CMD_ANSWER));
	memcpy((char *)&answer, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_ANALOG_METER_RCV_CMD_ANSWER));
	
//	printf("RcvCmd : ");
	for(i=0; i < 100; i++) {
		if(answer.data[i] == 0) {
			index = i;
			break;
		} else {
//			printf("%x ", answer.data[i]);
		}
	}
//	printf("\n");

	if(answer.data[0] == '!' || answer.data[0] == '$') {
		return 0;
	}
	
	if(myPs->config.checkSumFlag){
		data_size = (7*myPs->config.chPerModule) + 4;
	} else {
		data_size = (7*myPs->config.chPerModule) + 2;
	}

	if(answer.data[0] == '>') {
		if(index == 9) {
			if(myPs->signal[ANALOG_METER_SIG_MEASURE] == P0) {
			} else {
				userlog(DEBUG_LOG, psName, "answer size error1 %d \n", index);
/*				index = (int)myPs->signal[ANALOG_METER_SIG_MEASURE] % 10;
				if(index > myPs->config.countMeter) {
					index = 1;
				}
				myPs->signal[ANALOG_METER_SIG_MEASURE] = (unsigned char)index;
				myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] = 0;
				*/
				//120927 kji 
				next_module_request();
			}
			return 0;
		} else if(index == data_size) {
		} else {
			userlog(DEBUG_LOG, psName, "answer size error2 %d\n", index);
		/*	index = (int)myPs->signal[ANALOG_METER_SIG_MEASURE] % 10;
			if(index > myPs->config.countMeter) {
				index = 1;
			}
			myPs->signal[ANALOG_METER_SIG_MEASURE] = (unsigned char)index;
			myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] = 0;
			*/
			//120927 kji 
			next_module_request();
			return 0;
		}
	} else {
		userlog(DEBUG_LOG, psName, "answer data error0 \n");
		next_module_request();
		return 0;
	}
	
	//170319	
	if(myPs->signal[ANALOG_METER_SIG_MEASURE] <= 19){
		temp_bd = (int)(myPs->signal[ANALOG_METER_SIG_MEASURE] % 10) - 1;
	}else{	//for 10 Module
		temp_bd = 9;
	}
//	temp_bd = (int)(myPs->signal[ANALOG_METER_SIG_MEASURE] % 10) - 1;
	if(temp_bd < 0) return 0;
	
	//120927 add kji '+' '-' error
	for(i=0; i < myPs->config.chPerModule; i++) {
		if((answer.data[(i*7)+1] != '+') && ( answer.data[(i*7)+1] != '-')) {
			userlog(DEBUG_LOG, psName, "answer data error1 \n");
			next_module_request();
			return 0;
		}
			
	}
	//120927 add kji checksum 
	if(myPs->config.checkSumFlag){
		sum = 0;
		for(i = 0;i<((7*myPs->config.chPerModule) + 1);i++) {
			sum += answer.data[i];
		}
		checksum0 = ((sum % 0x100)/0x10);
		if(checksum0 < 0x0A)
			checksum0 += 0x30;
		else 
			checksum0 += 0x37;

		checksum1 = ((sum % 0x100)%0x10);
		if(checksum1 < 0x0A)
			checksum1 += 0x30;
		else 
			checksum1 += 0x37;	
	
		if((answer.data[(7*myPs->config.chPerModule) +1] != checksum0) || 
						(answer.data[(7*myPs->config.chPerModule)+2] != checksum1)) {
			userlog(DEBUG_LOG, psName, "checksum error \n");
			next_module_request();
			return 0;
		}
	}
	temp_ch = 0;
	index = 0;
	memset(buf, 0, 10);
	for(i = 1 ; i<100; i++) {
		if(answer.data[i] == '+' || answer.data[i] == '-') {
			memset(buf, 0, 10);
			for(j=0;j<7;j++) {
				buf[j] = answer.data[i+j];
				if(j > 0){
					if(buf[j] == 46){
					}else if((buf[j] >= 48) && (buf[j] <= 57)) {
					} else {
						userlog(DEBUG_LOG, psName, "data error2 %d\n",j);
						next_module_request();
						return 0;
					}
				}
			}
			temp[temp_ch] = (long)(atof(buf) * 1000.0);
			if(temp_ch == (myPs->config.chPerModule -1))
				break;
			else
				temp_ch++;
		}
	}

	for(temp_ch=0; temp_ch < myPs->config.chPerModule; temp_ch++) {
		chPerModule = myPs->config.chPerModule;
		myPs->misc2.tempData[chPerModule * temp_bd + temp_ch] = temp[temp_ch];
		myPs->misc.org_temp[temp_bd][temp_ch] = temp[temp_ch];
	}
// 110621 oys w
	myData->serialCheckTime2 = myData->mData.misc.timer_1sec;	
//	printf("\n");
	next_module_request();
	return 0;
}
void next_module_request() {
	if(myPs->signal[ANALOG_METER_SIG_MEASURE] == P11) {
		if(myPs->config.countMeter > 1) {
			myPs->signal[ANALOG_METER_SIG_MEASURE] = P2;
		} else {
			myPs->signal[ANALOG_METER_SIG_MEASURE] = P1;
		}
		myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] = 0;
	} else if(myPs->signal[ANALOG_METER_SIG_MEASURE] == P12) {
		if(myPs->config.countMeter > 2) {
			myPs->signal[ANALOG_METER_SIG_MEASURE] = P3;
		} else {
			myPs->signal[ANALOG_METER_SIG_MEASURE] = P1;
		}
		myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] = 0;
	} else if(myPs->signal[ANALOG_METER_SIG_MEASURE] == P13) {
		if(myPs->config.countMeter > 3) {
			myPs->signal[ANALOG_METER_SIG_MEASURE] = P4;
		} else {
			myPs->signal[ANALOG_METER_SIG_MEASURE] = P1;
		}
		myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] = 0;
	} else if(myPs->signal[ANALOG_METER_SIG_MEASURE] == P14) {
		if(myPs->config.countMeter > 4) {
			myPs->signal[ANALOG_METER_SIG_MEASURE] = P5;
		} else {
			myPs->signal[ANALOG_METER_SIG_MEASURE] = P1;
		}
		myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] = 0;
	} else if(myPs->signal[ANALOG_METER_SIG_MEASURE] == P15) {
		if(myPs->config.countMeter > 5) {
			myPs->signal[ANALOG_METER_SIG_MEASURE] = P6;
		} else {
			myPs->signal[ANALOG_METER_SIG_MEASURE] = P1;
		}
		myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] = 0;
	} else if(myPs->signal[ANALOG_METER_SIG_MEASURE] == P16) {
		if(myPs->config.countMeter > 6) {
			myPs->signal[ANALOG_METER_SIG_MEASURE] = P7;
		} else {
			myPs->signal[ANALOG_METER_SIG_MEASURE] = P1;
		}
		myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] = 0;
	} else if(myPs->signal[ANALOG_METER_SIG_MEASURE] == P17) {
		if(myPs->config.countMeter > 7) {
			myPs->signal[ANALOG_METER_SIG_MEASURE] = P8;
		} else {
			myPs->signal[ANALOG_METER_SIG_MEASURE] = P1;
		}
		myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] = 0;
	}else if(myPs->signal[ANALOG_METER_SIG_MEASURE] == P18) {
		if(myPs->config.countMeter > 8) {
			myPs->signal[ANALOG_METER_SIG_MEASURE] = P9;
		} else {
			myPs->signal[ANALOG_METER_SIG_MEASURE] = P1;
		}
		myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] = 0;
	}else if(myPs->signal[ANALOG_METER_SIG_MEASURE] == P19) {
		if(myPs->config.countMeter > 9) {
			myPs->signal[ANALOG_METER_SIG_MEASURE] = P10;
		} else {
			myPs->signal[ANALOG_METER_SIG_MEASURE] = P1;
		}
		myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] = 0;
	}else if(myPs->signal[ANALOG_METER_SIG_MEASURE] == P20) {
		myPs->signal[ANALOG_METER_SIG_MEASURE] = P1;
		myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] = 0;
	}
}

void send_cmd_initialize(int type, int addr, int val)
{
	int rtn, cmd_size, i;
	S_ANALOG_METER_SEND_CMD_INITIALIZE	cmd;
	
	memset((char *)&cmd, 0x00, sizeof(S_ANALOG_METER_SEND_CMD_INITIALIZE));

	switch(type) {
		case 1: //set module configuration
			cmd_size = 12;
			if(addr == 1) {
				if(val == 1) {
					if(myPs->config.readType == READ_T_K) { //k type
						memcpy((char *)&cmd.data[0], "%01010F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0101050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0101100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0101060600\r", cmd_size);
					}
				} else if(val == 2) {
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%01020F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0102050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0102100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0102060600\r", cmd_size);
					}
				} else if(val == 3) {
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%01030F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0103050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0103100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0103060600\r", cmd_size);
					}
				} else if(val == 4) {
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%01040F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0104050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0104100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0104060600\r", cmd_size);
					}
				} else if(val == 5) {
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%01050F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0105050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0105100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0105060600\r", cmd_size);
					}				
				} else if(val == 6) {
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%01060F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0106050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0106100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0106060600\r", cmd_size);
					}				
				} else if(val == 7) {
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%01070F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0107050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0107100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0107060600\r", cmd_size);
					}				
				} else if(val == 8) {
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%01080F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0108050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0108100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0108060600\r", cmd_size);
					}
				} else if(val == 9) {	//170319 add for Max 10EA
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%01090F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0109050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0109100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0109060600\r", cmd_size);
					}
				} else if(val == 10) {		//170319 add for Max 10EA
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%010A0F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%010A050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%010A100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%010A060600\r", cmd_size);
					}
				}
			} else if(addr == 2) {
				if(val == 1) {
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%02010F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0201050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0201100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0201060600\r", cmd_size);
					}
				} else if(val == 2) {
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%02020F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0202050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0202100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0202060600\r", cmd_size);
					}
				} else if(val == 3) {
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%02030F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0203050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0203100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0203060600\r", cmd_size);
					}
				} else if(val == 4) {
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%02040F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0204050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0204100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0204060600\r", cmd_size);
					}
				} else if(val == 5) {
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%02050F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0205050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0205100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0205060600\r", cmd_size);
					}
				} else if(val == 6) {
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%02060F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0206050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0206100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0206060600\r", cmd_size);
					}
				} else if(val == 7) {
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%02070F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0207050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0207100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0207060600\r", cmd_size);
					}
				} else if(val == 8) {
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%02080F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0208050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0208100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0208060600\r", cmd_size);
					}
				} else if(val == 9) {	//170319 add for Max 10EA
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%02090F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0209050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0209100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0209060600\r", cmd_size);
					}
				} else if(val == 10) {		//170319 add for Max 10EA
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%020A0F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%020A050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%020A100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%020A60600\r", cmd_size);
					}
				}
			} else if(addr == 3) {
				if(val == 1) {
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%03010F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0301050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0301100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0301060600\r", cmd_size);
					}
				} else if(val == 2) {
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%03020F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0302050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0302100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0302060600\r", cmd_size);
					}
				} else if(val == 3) {
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%03030F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0303050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0303100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0303060600\r", cmd_size);
					}
				} else if(val == 4) {
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%03040F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0304050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0304100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0304060600\r", cmd_size);
					}
				} else if(val == 5) {
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%03050F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0305050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0305100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0305060600\r", cmd_size);
					}
				} else if(val == 6) {
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%03060F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0306050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0306100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0306060600\r", cmd_size);
					}
				} else if(val == 7) {
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%03070F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0307050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0307100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0307060600\r", cmd_size);
					}
				} else if(val == 8) {
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%03080F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0308050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0308100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0308060600\r", cmd_size);
					}
				} else if(val == 9) {	//170319 add for Max 10EA
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%03090F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0309050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0309100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0309060600\r", cmd_size);
					}
				} else if(val == 10) {		//170319 add for Max 10EA
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%030A0F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%030A050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%030A100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%030A060600\r", cmd_size);
					}
				}
			} else if(addr == 4) {
				if(val == 1) {
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%04010F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0401050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0401100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0401060600\r", cmd_size);
					}
				} else if(val == 2) {
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%04020F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0402050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0402100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0402060600\r", cmd_size);
					}
				} else if(val == 3) {
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%04030F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0403050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0403100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0403060600\r", cmd_size);
					}
 				} else if(val == 4) {
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%04040F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0404050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0404100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0404060600\r", cmd_size);
					}
 				} else if(val == 5) {
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%04050F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0405050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0405100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0405060600\r", cmd_size);
					}
 				} else if(val == 6) {
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%04060F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0406050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0406100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0406060600\r", cmd_size);
					}
 				} else if(val == 7) {
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%04070F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0407050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0407100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0407060600\r", cmd_size);
					}
 				} else if(val == 8) {
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%04080F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0408050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0408100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0408060600\r", cmd_size);
					}
				} else if(val == 9) {	//170319 add for Max 10EA
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%04090F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0409050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0409100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0409060600\r", cmd_size);
					}
				} else if(val == 10) {		//170319 add for Max 10EA
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%040A0F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%040A050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%040A100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%040A060600\r", cmd_size);
					}
				}
			} else if(addr == 5) {
				if(val == 1) {
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%05010F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0501050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0501100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0401060600\r", cmd_size);
					}
				} else if(val == 2) {
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%05020F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0502050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0502100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0502060600\r", cmd_size);
					}
				} else if(val == 3) {
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%05030F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0503050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0503100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0503060600\r", cmd_size);
					}
 				} else if(val == 4) {
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%05040F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0504050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0504100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0504060600\r", cmd_size);
					}
 				} else if(val == 5) {
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%05050F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0505050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0505100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0505060600\r", cmd_size);
					}
 				} else if(val == 6) {
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%05060F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0506050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0506100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0506060600\r", cmd_size);
					}
 				} else if(val == 7) {
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%05070F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0507050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0507100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0507060600\r", cmd_size);
					}
 				} else if(val == 8) {
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%05080F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0508050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0508100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0508060600\r", cmd_size);
					}
				} else if(val == 9) {	//170319 add for Max 10EA
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%05090F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0509050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0509100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0509060600\r", cmd_size);
					}
				} else if(val == 10) {		//170319 add for Max 10EA
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%050A0F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%050A050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%050A100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%050A060600\r", cmd_size);
					}
				}
			} else if(addr == 6) {
				if(val == 1) {
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%06010F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0601050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0601100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0601060600\r", cmd_size);
					}
				} else if(val == 2) {
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%06020F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0602050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0402100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0602060600\r", cmd_size);
					}
				} else if(val == 3) {
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%06030F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0603050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0403100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0603060600\r", cmd_size);
					}
 				} else if(val == 4) {
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%06040F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0604050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0604100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0604060600\r", cmd_size);
					}
 				} else if(val == 5) {
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%06050F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0605050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0605100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0605060600\r", cmd_size);
					}
 				} else if(val == 6) {
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%06060F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0606050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0606100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0606060600\r", cmd_size);
					}
 				} else if(val == 7) {
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%06070F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0607050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0607100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0607060600\r", cmd_size);
					}
 				} else if(val == 8) {
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%06080F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0608050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0608100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0608060600\r", cmd_size);
					}
				} else if(val == 9) {	//170319 add for Max 10EA
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%06090F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0609050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0609100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0609060600\r", cmd_size);
					}
				} else if(val == 10) {		//170319 add for Max 10EA
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%060A0F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%060A050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%060A100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%060A060600\r", cmd_size);
					}
				}
			} else if(addr == 7) {
				if(val == 1) {
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%07010F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0701050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0701100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0701060600\r", cmd_size);
					}
				} else if(val == 2) {
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%07020F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0702050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0702100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0702060600\r", cmd_size);
					}
				} else if(val == 3) {
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%07030F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0703050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0703100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0703060600\r", cmd_size);
					}
 				} else if(val == 4) {
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%07040F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0704050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0704100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0704060600\r", cmd_size);
					}
 				} else if(val == 5) {
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%07050F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0705050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0705100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0705060600\r", cmd_size);
					}
 				} else if(val == 6) {
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%07060F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0706050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0706100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0706060600\r", cmd_size);
					}
 				} else if(val == 7) {
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%07070F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0707050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0707100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0707060600\r", cmd_size);
					}
 				} else if(val == 8) {
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%07080F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0708050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0708100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0708060600\r", cmd_size);
					}
				} else if(val == 9) {	//170319 add for Max 10EA
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%07090F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0709050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0709100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0709060600\r", cmd_size);
					}
				} else if(val == 10) {		//170319 add for Max 10EA
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%070A0F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%070A050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%070A100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%070A060600\r", cmd_size);
					}
				}
			} else if(addr == 8) {
				if(val == 1) {
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%08010F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0801050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0801100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0801060600\r", cmd_size);
					}
				} else if(val == 2) {
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%08020F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0802050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0802100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0802060600\r", cmd_size);
					}
				} else if(val == 3) {
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%08030F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0803050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0803100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0803060600\r", cmd_size);
					}
 				} else if(val == 4) {
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%08040F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0804050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0804100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0804060600\r", cmd_size);
					}
 				} else if(val == 5) {
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%08050F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0805050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0805100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0805060600\r", cmd_size);
					}
 				} else if(val == 6) {
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%08060F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0806050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0806100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0806060600\r", cmd_size);
					}
 				} else if(val == 7) {
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%08070F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0807050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0807100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0807060600\r", cmd_size);
					}
 				} else if(val == 8) {
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%08080F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0808050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0808100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0808060600\r", cmd_size);
					}
				} else if(val == 9) {	//170319 add for Max 10EA
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%08090F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0809050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0809100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0809060600\r", cmd_size);
					}
				} else if(val == 10) {		//170319 add for Max 10EA
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%080A0F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%080A050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%080A100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%080A060600\r", cmd_size);
					}
				}
			} else if(addr == 9) {			//170319 add
				if(val == 1) {
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%09010F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0901050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0901100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0901060600\r", cmd_size);
					}
				} else if(val == 2) {
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%09020F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0902050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0902100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0902060600\r", cmd_size);
					}
				} else if(val == 3) {
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%09030F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0903050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0903100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0903060600\r", cmd_size);
					}
 				} else if(val == 4) {
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%09040F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0904050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0904100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0904060600\r", cmd_size);
					}
 				} else if(val == 5) {
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%09050F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0905050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0905100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0905060600\r", cmd_size);
					}
 				} else if(val == 6) {
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%09060F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0906050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0906100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0906060600\r", cmd_size);
					}
 				} else if(val == 7) {
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%09070F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0907050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0907100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0907060600\r", cmd_size);
					}
 				} else if(val == 8) {
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%09080F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0908050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0908100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0908060600\r", cmd_size);
					}
				} else if(val == 9) {	//170319 add for Max 10EA
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%09090F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0909050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0909100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0909060600\r", cmd_size);
					}
				} else if(val == 10) {		//170319 add for Max 10EA
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%090A0F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%090A050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%090A100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%090A060600\r", cmd_size);
					}
				}
			} else if(addr == 10) {			//170319 add
				if(val == 1) {
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%0A010F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0A01050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0A01100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0A01060600\r", cmd_size);
					}
				} else if(val == 2) {
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%0A020F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0A02050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0A02100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0A02060600\r", cmd_size);
					}
				} else if(val == 3) {
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%0A030F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0A03050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0A03100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0A03060600\r", cmd_size);
					}
 				} else if(val == 4) {
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%0A040F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0A04050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0A04100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0A04060600\r", cmd_size);
					}
 				} else if(val == 5) {
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%0A050F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0A05050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0A05100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0A05060600\r", cmd_size);
					}
 				} else if(val == 6) {
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%0A060F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0A06050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0A06100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0A06060600\r", cmd_size);
					}
 				} else if(val == 7) {
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%0A070F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0A07050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0A07100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0A07060600\r", cmd_size);
					}
 				} else if(val == 8) {
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%0A080F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0A08050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0A08100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0A08060600\r", cmd_size);
					}
				} else if(val == 9) {	//170319 add for Max 10EA
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%0A090F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0A09050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0A09100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0A09060600\r", cmd_size);
					}
				} else if(val == 10) {		//170319 add for Max 10EA
					if(myPs->config.readType == READ_T_K) {
						memcpy((char *)&cmd.data[0], "%0A0A0F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data[0], "%0A0A050600\r", cmd_size);
					} else if(myPs->config.readType == READ_T_T) { //t type
						memcpy((char *)&cmd.data[0], "%0A0A100600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data[0], "%0A0A060600\r", cmd_size);
					}
				}
			}
			break;
		case 2: //set channel enable
			cmd_size = 7;
			if(val == 1) {
				memcpy((char *)&cmd.data[0], "$015FF\r", cmd_size);
			} else if(addr == 2) {
				memcpy((char *)&cmd.data[0], "$025FF\r", cmd_size);
			} else if(addr == 3) {
				memcpy((char *)&cmd.data[0], "$035FF\r", cmd_size);
			} else if(addr == 4) {
				memcpy((char *)&cmd.data[0], "$045FF\r", cmd_size);
			} else if(addr == 5) {
				memcpy((char *)&cmd.data[0], "$055FF\r", cmd_size);
			} else if(addr == 6) {
				memcpy((char *)&cmd.data[0], "$065FF\r", cmd_size);
			} else if(addr == 7) {
				memcpy((char *)&cmd.data[0], "$075FF\r", cmd_size);
			} else if(addr == 8) {
				memcpy((char *)&cmd.data[0], "$085FF\r", cmd_size);
			} else if(addr == 9) {			//170319 add
				memcpy((char *)&cmd.data[0], "$095FF\r", cmd_size);
			} else if(addr == 10){			//170319 add
				memcpy((char *)&cmd.data[0], "$0A5FF\r", cmd_size);
			}
			break;
		default: break;
	}	
	rtn = send_command((char *)&cmd, cmd_size, 0);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "send cmd error1 !!!\n");
	}
	if(type == 1) {
		switch(myPs->config.readType) {
			case 0:
				printf("Set readType : READ_T (K type)\n"); 	
				break;
			case 1:
				printf("Set readType : READ_V\n"); 	
				break;
			case 2:
				printf("Set readType : READ_I\n"); 	
				break;
			case 3:
				printf("Set readType : READ_T (T type)\n"); 	
				break;
				
			default: break;
		}
		if(myPs->config.chPerModule == 10){
			for(i=0; i<10; i++) {
				usleep(150000);
				send_cmd_initialize2(val, i);
			}
		}
	}
}
// 161130 oys add : I-7018Z readType Setting.
void send_cmd_initialize2(int addr, int ch)
{
	int rtn, cmd_size = 10;
	S_ANALOG_METER_SEND_CMD_INITIALIZE	cmd;
	
	memset((char *)&cmd, 0x00, sizeof(S_ANALOG_METER_SEND_CMD_INITIALIZE));

	if(addr == 1) {
		if(ch == 0) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$017C0R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$017C0R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$017C0R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$017C0R06\r", cmd_size);
			}
		} else if(ch == 1) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$017C1R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$017C1R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$017C1R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$017C1R06\r", cmd_size);
			}
		} else if(ch == 2) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$017C2R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$017C2R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$017C2R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$017C2R06\r", cmd_size);
			}
		} else if(ch == 3) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$017C3R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$017C3R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$017C3R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$017C3R06\r", cmd_size);
			}
		} else if(ch == 4) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$017C4R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$017C4R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$017C4R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$017C4R06\r", cmd_size);
			}
		} else if(ch == 5) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$017C5R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$017C5R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$017C5R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$017C5R06\r", cmd_size);
			}
		} else if(ch == 6) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$017C6R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$017C6R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$017C6R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$017C6R06\r", cmd_size);
			}
		} else if(ch == 7) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$017C7R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$017C7R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$017C7R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$017C7R06\r", cmd_size);
			}
		} else if(ch == 8) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$017C8R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$017C8R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$017C8R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$017C8R06\r", cmd_size);
			}
		} else if(ch == 9) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$017C9R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$017C9R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$017C9R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$017C9R06\r", cmd_size);
			}
		}
	} else if(addr == 2) {
		if(ch == 0) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$027C0R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$027C0R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$027C0R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$027C0R06\r", cmd_size);
			}
		} else if(ch == 1) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$027C1R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$027C1R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$027C1R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$027C1R06\r", cmd_size);
			}
		} else if(ch == 2) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$027C2R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$027C2R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$027C2R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$027C2R06\r", cmd_size);
			}
		} else if(ch == 3) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$027C3R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$027C3R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$027C3R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$027C3R06\r", cmd_size);
			}
		} else if(ch == 4) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$027C4R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$027C4R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$027C4R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$027C4R06\r", cmd_size);
			}
		} else if(ch == 5) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$027C5R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$027C5R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$027C5R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$027C5R06\r", cmd_size);
			}
		} else if(ch == 6) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$027C6R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$027C6R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$027C6R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$027C6R06\r", cmd_size);
			}
		} else if(ch == 7) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$027C7R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$027C7R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$027C7R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$027C7R06\r", cmd_size);
			}
		} else if(ch == 8) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$027C8R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$027C8R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$027C8R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$027C8R06\r", cmd_size);
			}
		} else if(ch == 9) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$027C9R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$027C9R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$027C9R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$027C9R06\r", cmd_size);
			}
		}
	} else if(addr == 3) {
		if(ch == 0) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$037C0R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$037C0R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$037C0R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$037C0R06\r", cmd_size);
			}
		} else if(ch == 1) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$037C1R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$037C1R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$037C1R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$037C1R06\r", cmd_size);
			}
		} else if(ch == 2) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$037C2R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$037C2R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$037C2R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$037C2R06\r", cmd_size);
			}
		} else if(ch == 3) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$037C3R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$037C3R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$037C3R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$037C3R06\r", cmd_size);
			}
		} else if(ch == 4) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$037C4R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$037C4R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$037C4R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$037C4R06\r", cmd_size);
			}
		} else if(ch == 5) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$037C5R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$037C5R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$037C5R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$037C5R06\r", cmd_size);
			}
		} else if(ch == 6) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$037C6R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$037C6R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$037C6R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$037C6R06\r", cmd_size);
			}
		} else if(ch == 7) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$037C7R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$037C7R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$037C7R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$037C7R06\r", cmd_size);
			}
		} else if(ch == 8) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$037C8R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$037C8R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$037C8R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$037C8R06\r", cmd_size);
			}
		} else if(ch == 9) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$037C9R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$037C9R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$037C9R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$037C9R06\r", cmd_size);
			}
		}
	} else if(addr == 4) {
		if(ch == 0) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$047C0R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$047C0R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$047C0R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$047C0R06\r", cmd_size);
			}
		} else if(ch == 1) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$047C1R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$047C1R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$047C1R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$047C1R06\r", cmd_size);
			}
		} else if(ch == 2) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$047C2R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$047C2R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$047C2R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$047C2R06\r", cmd_size);
			}
		} else if(ch == 3) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$047C3R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$047C3R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$047C3R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$047C3R06\r", cmd_size);
			}
		} else if(ch == 4) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$047C4R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$047C4R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$047C4R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$047C4R06\r", cmd_size);
			}
		} else if(ch == 5) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$047C5R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$047C5R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$047C5R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$047C5R06\r", cmd_size);
			}
		} else if(ch == 6) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$047C6R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$047C6R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$047C6R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$047C6R06\r", cmd_size);
			}
		} else if(ch == 7) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$047C7R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$047C7R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$047C7R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$047C7R06\r", cmd_size);
			}
		} else if(ch == 8) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$047C8R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$047C8R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$047C8R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$047C8R06\r", cmd_size);
			}
		} else if(ch == 9) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$047C9R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$047C9R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$047C9R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$047C9R06\r", cmd_size);
			}
		}
	} else if(addr == 5) {
		if(ch == 0) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$057C0R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$057C0R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$057C0R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$057C0R06\r", cmd_size);
			}
		} else if(ch == 1) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$057C1R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$057C1R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$057C1R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$057C1R06\r", cmd_size);
			}
		} else if(ch == 2) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$057C2R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$057C2R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$057C2R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$057C2R06\r", cmd_size);
			}
		} else if(ch == 3) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$057C3R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$057C3R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$057C3R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$057C3R06\r", cmd_size);
			}
		} else if(ch == 4) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$057C4R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$057C4R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$057C4R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$057C4R06\r", cmd_size);
			}
		} else if(ch == 5) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$057C5R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$057C5R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$057C5R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$057C5R06\r", cmd_size);
			}
		} else if(ch == 6) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$057C6R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$057C6R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$057C6R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$057C6R06\r", cmd_size);
			}
		} else if(ch == 7) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$057C7R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$057C7R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$057C7R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$057C7R06\r", cmd_size);
			}
		} else if(ch == 8) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$057C8R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$057C8R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$057C8R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$057C8R06\r", cmd_size);
			}
		} else if(ch == 9) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$057C9R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$057C9R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$057C9R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$057C9R06\r", cmd_size);
			}
		}
	} else if(addr == 6) {
		if(ch == 0) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$067C0R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$067C0R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$067C0R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$067C0R06\r", cmd_size);
			}
		} else if(ch == 1) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$067C1R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$067C1R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$067C1R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$067C1R06\r", cmd_size);
			}
		} else if(ch == 2) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$067C2R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$067C2R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$067C2R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$067C2R06\r", cmd_size);
			}
		} else if(ch == 3) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$067C3R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$067C3R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$067C3R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$067C3R06\r", cmd_size);
			}
		} else if(ch == 4) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$067C4R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$067C4R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$067C4R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$067C4R06\r", cmd_size);
			}
		} else if(ch == 5) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$067C5R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$067C5R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$067C5R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$067C5R06\r", cmd_size);
			}
		} else if(ch == 6) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$067C6R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$067C6R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$067C6R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$067C6R06\r", cmd_size);
			}
		} else if(ch == 7) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$067C7R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$067C7R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$067C7R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$067C7R06\r", cmd_size);
			}
		} else if(ch == 8) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$067C8R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$067C8R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$067C8R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$067C8R06\r", cmd_size);
			}
		} else if(ch == 9) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$067C9R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$067C9R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$067C9R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$067C9R06\r", cmd_size);
			}
		}
	} else if(addr == 7) {
		if(ch == 0) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$077C0R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$077C0R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$077C0R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$077C0R06\r", cmd_size);
			}
		} else if(ch == 1) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$077C1R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$077C1R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$077C1R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$077C1R06\r", cmd_size);
			}
		} else if(ch == 2) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$077C2R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$077C2R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$077C2R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$077C2R06\r", cmd_size);
			}
		} else if(ch == 3) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$077C3R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$077C3R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$077C3R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$077C3R06\r", cmd_size);
			}
		} else if(ch == 4) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$077C4R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$077C4R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$077C4R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$077C4R06\r", cmd_size);
			}
		} else if(ch == 5) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$077C5R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$077C5R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$077C5R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$077C5R06\r", cmd_size);
			}
		} else if(ch == 6) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$077C6R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$077C6R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$077C6R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$077C6R06\r", cmd_size);
			}
		} else if(ch == 7) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$077C7R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$077C7R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$077C7R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$077C7R06\r", cmd_size);
			}
		} else if(ch == 8) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$077C8R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$077C8R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$077C8R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$077C8R06\r", cmd_size);
			}
		} else if(ch == 9) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$077C9R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$077C9R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$077C9R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$077C9R06\r", cmd_size);
			}
		}
	} else if(addr == 8) {
		if(ch == 0) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$087C0R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$087C0R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$087C0R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$087C0R06\r", cmd_size);
			}
		} else if(ch == 1) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$087C1R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$087C1R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$087C1R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$087C1R06\r", cmd_size);
			}
		} else if(ch == 2) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$087C2R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$087C2R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$087C2R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$087C2R06\r", cmd_size);
			}
		} else if(ch == 3) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$087C3R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$087C3R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$087C3R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$087C3R06\r", cmd_size);
			}
		} else if(ch == 4) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$087C4R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$087C4R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$087C4R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$087C4R06\r", cmd_size);
			}
		} else if(ch == 5) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$087C5R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$087C5R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$087C5R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$087C5R06\r", cmd_size);
			}
		} else if(ch == 6) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$087C6R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$087C6R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$087C6R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$087C6R06\r", cmd_size);
			}
		} else if(ch == 7) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$087C7R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$087C7R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$087C7R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$087C7R06\r", cmd_size);
			}
		} else if(ch == 8) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$087C8R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$087C8R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$087C8R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$087C8R06\r", cmd_size);
			}
		} else if(ch == 9) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$087C9R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$087C9R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$087C9R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$087C9R06\r", cmd_size);
			}
		}
	} else if(addr == 9) {				//170319
		if(ch == 0) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$097C0R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$097C0R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$097C0R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$097C0R06\r", cmd_size);
			}
		} else if(ch == 1) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$097C1R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$097C1R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$097C1R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$097C1R06\r", cmd_size);
			}
		} else if(ch == 2) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$097C2R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$097C2R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$097C2R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$097C2R06\r", cmd_size);
			}
		} else if(ch == 3) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$097C3R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$097C3R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$097C3R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$097C3R06\r", cmd_size);
			}
		} else if(ch == 4) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$097C4R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$097C4R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$097C4R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$097C4R06\r", cmd_size);
			}
		} else if(ch == 5) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$097C5R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$097C5R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$097C5R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$097C5R06\r", cmd_size);
			}
		} else if(ch == 6) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$097C6R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$097C6R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$097C6R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$097C6R06\r", cmd_size);
			}
		} else if(ch == 7) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$097C7R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$097C7R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$097C7R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$097C7R06\r", cmd_size);
			}
		} else if(ch == 8) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$097C8R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$097C8R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$097C8R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$097C8R06\r", cmd_size);
			}
		} else if(ch == 9) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$097C9R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$097C9R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$097C9R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$097C9R06\r", cmd_size);
			}
		}
	} else if(addr == 10) {			//170319 add
		if(ch == 0) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$0A7C0R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$0A7C0R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$0A7C0R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$0A7C0R06\r", cmd_size);
			}
		} else if(ch == 1) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$0A7C1R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$0A7C1R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$0A7C1R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$0A7C1R06\r", cmd_size);
			}
		} else if(ch == 2) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$0A7C2R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$0A7C2R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$0A7C2R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$0A7C2R06\r", cmd_size);
			}
		} else if(ch == 3) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$0A7C3R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$0A7C3R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$0A7C3R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$0A7C3R06\r", cmd_size);
			}
		} else if(ch == 4) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$0A7C4R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$0A7C4R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$0A7C4R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$0A7C4R06\r", cmd_size);
			}
		} else if(ch == 5) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$0A7C5R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$0A7C5R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$0A7C5R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$0A7C5R06\r", cmd_size);
			}
		} else if(ch == 6) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$0A7C6R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$0A7C6R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$0A7C6R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$0A7C6R06\r", cmd_size);
			}
		} else if(ch == 7) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$0A7C7R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$0A7C7R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$0A7C7R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$0A7C7R06\r", cmd_size);
			}
		} else if(ch == 8) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$0A7C8R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$0A7C8R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$0A7C8R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$0A7C8R06\r", cmd_size);
			}
		} else if(ch == 9) {
			if(myPs->config.readType == READ_T_K) { //k type
				memcpy((char *)&cmd.data[0], "$0A7C9R0F\r", cmd_size);
			} else if(myPs->config.readType == READ_V) {
				memcpy((char *)&cmd.data[0], "$0A7C9R05\r", cmd_size);
			} else if(myPs->config.readType == READ_T_T) { //t type
				memcpy((char *)&cmd.data[0], "$0A7C9R10\r", cmd_size);
			} else { //READ_I
				memcpy((char *)&cmd.data[0], "$0A7C9R06\r", cmd_size);
			}
		}
	}
	rtn = send_command((char *)&cmd, cmd_size, 0);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "send cmd error1 !!!\n");
	}
}

void send_cmd_request(int type, int bd)
{
	if(myPs->config.functionType == 0) {
		if(myPs->config.checkSumFlag) {
			send_cmd_request1(type+2, bd);
		} else {
			send_cmd_request1(type, bd);
		}
	} else {
		send_cmd_request2(type, bd);
	}
}

void send_cmd_request1(int type, int bd)
{
	int rtn, cmd_size;
	S_ANALOG_METER_SEND_CMD_REQUEST	cmd;
	
	memset((char *)&cmd, 0x00, sizeof(S_ANALOG_METER_SEND_CMD_REQUEST));

	if(type == 0) {
		cmd_size = 4;
		if(bd == 1) {
			memcpy((char *)&cmd.data[0], "#01\r", cmd_size);
		} else if(bd == 2) {
			memcpy((char *)&cmd.data[0], "#02\r", cmd_size);
		} else if(bd == 3) {
			memcpy((char *)&cmd.data[0], "#03\r", cmd_size);
		} else if(bd == 4) {
			memcpy((char *)&cmd.data[0], "#04\r", cmd_size);
		} else if(bd == 5) {
			memcpy((char *)&cmd.data[0], "#05\r", cmd_size);
		} else if(bd == 6) {
			memcpy((char *)&cmd.data[0], "#06\r", cmd_size);
		} else if(bd == 7) {
			memcpy((char *)&cmd.data[0], "#07\r", cmd_size);
		} else if(bd == 8) {
			memcpy((char *)&cmd.data[0], "#08\r", cmd_size);
		} else if(bd == 9) {		//170319 add
			memcpy((char *)&cmd.data[0], "#09\r", cmd_size);
		} else if(bd == 10){		//170319 add
			memcpy((char *)&cmd.data[0], "#0A\r", cmd_size);
		}
	} else if(type == 1) {
		cmd_size = 5;
		if(bd == 1) {
			memcpy((char *)&cmd.data[0], "$013\r", cmd_size);
		} else if(bd == 2) {
			memcpy((char *)&cmd.data[0], "$023\r", cmd_size);
		} else if(bd == 3) {
			memcpy((char *)&cmd.data[0], "$033\r", cmd_size);
		} else if(bd == 4) {
			memcpy((char *)&cmd.data[0], "$043\r", cmd_size);
		} else if(bd == 5) {
			memcpy((char *)&cmd.data[0], "$053\r", cmd_size);
		} else if(bd == 6) {
			memcpy((char *)&cmd.data[0], "$063\r", cmd_size);
		} else if(bd == 7) {
			memcpy((char *)&cmd.data[0], "$073\r", cmd_size);
		} else if(bd == 8) {
			memcpy((char *)&cmd.data[0], "$083\r", cmd_size);
		} else if(bd == 9) {		//170319 add
			memcpy((char *)&cmd.data[0], "$093\r", cmd_size);
		} else if(bd == 10){		//170319 add
			memcpy((char *)&cmd.data[0], "$0A3\r", cmd_size);
		}
	} else if(type == 2) {
		cmd_size = 6;
		if(bd == 1) {
			memcpy((char *)&cmd.data[0], "#0184\r", cmd_size);
		} else if(bd == 2) {
			memcpy((char *)&cmd.data[0], "#0285\r", cmd_size);
		} else if(bd == 3) {
			memcpy((char *)&cmd.data[0], "#0386\r", cmd_size);
		} else if(bd == 4) {
			memcpy((char *)&cmd.data[0], "#0487\r", cmd_size);
		} else if(bd == 5) {
			memcpy((char *)&cmd.data[0], "#0588\r", cmd_size);
		} else if(bd == 6) {
			memcpy((char *)&cmd.data[0], "#0689\r", cmd_size);
		} else if(bd == 7) {
			memcpy((char *)&cmd.data[0], "#078A\r", cmd_size);
		} else if(bd == 8) {
			memcpy((char *)&cmd.data[0], "#088B\r", cmd_size);
		} else if(bd == 9) {			//170319 add
			memcpy((char *)&cmd.data[0], "#098C\r", cmd_size);
		} else if(bd == 10){			//170319 add
			memcpy((char *)&cmd.data[0], "#0A8D\r", cmd_size);
		}
	}
	rtn = send_command((char *)&cmd, cmd_size, 0);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "send cmd error2 !!!\n");
	}
}

void send_cmd_request2(int type, int bd)
{
	char buf[12];
	int rtn, cmd_size;
	S_ANALOG_METER_SEND_CMD_REQUEST	cmd;
	
	memset((char *)&myPs->rcvPacket, 0, sizeof(S_ANALOG_METER_RCV_PACKET));
	memset((char *)&myPs->rcvCmd, 0, sizeof(S_ANALOG_METER_RCV_COMMAND));

	memset((char *)&cmd, 0, sizeof(S_ANALOG_METER_SEND_CMD_REQUEST));

	switch(type) {
		case 0:
			cmd_size = 12;
			if(bd == 1) {
				memcpy((char *)&cmd.data, "FD 0,01,16\r\n", cmd_size);
			} else if(bd == 2) {
				memcpy((char *)&cmd.data, "FD 0,01,16\r\n", cmd_size);
			} else if(bd == 3) {
				memcpy((char *)&cmd.data, "FD 0,01,16\r\n", cmd_size);
			} else if(bd == 4) {
				memcpy((char *)&cmd.data, "FD 0,01,16\r\n", cmd_size);
			} else {
				memset(buf, 0, sizeof(buf));
				sprintf(buf, "#%02d\r", bd);
				memcpy((char *)&cmd.data, (char *)&buf, cmd_size);
			}
			break;
		case 1:
			cmd_size = 5;
			if(bd == 1) {
				memcpy((char *)&cmd.data, "$013\r", cmd_size);
			} else if(bd == 2) {
				memcpy((char *)&cmd.data, "$023\r", cmd_size);
			} else if(bd == 3) {
				memcpy((char *)&cmd.data, "$033\r", cmd_size);
			} else if(bd == 4) {
				memcpy((char *)&cmd.data, "$043\r", cmd_size);
			}
			break;
		default:
			userlog(DEBUG_LOG, psName, "Unknown cmd\n");
			return;
	}
	rtn = send_command((char *)&cmd, cmd_size, 0);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "send cmd error3 !!!\n");
	}
}
void send_cmd_open(int bd)
{
	char buf[12], tmp[4];
	int rtn, cmd_size;
	
	cmd_size = 7;
//	printf("analog cmd open\n");
	memset(buf, 0, sizeof(buf));
	buf[0] = 0x1B; //ESC
	buf[1] = 'O';
	buf[2] = ' ';

	memset(tmp, 0, sizeof(tmp));
	sprintf(tmp, "%02d", bd);
	memcpy((char *)&buf[3], (char *)&tmp, 2);

	buf[5] = 0x0D; //CR
	buf[6] = 0x0A; //\n
	
	rtn = send_command((char *)&buf, cmd_size, 0);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "send cmd error4 !!!\n");
	}

	//usleep(250000);

	memset((char *)&myPs->rcvPacket, 0, sizeof(S_ANALOG_METER_RCV_PACKET));
	memset((char *)&myPs->rcvCmd, 0, sizeof(S_ANALOG_METER_RCV_COMMAND));
}

void send_cmd_close(int bd)
{
	char buf[12], tmp[4];
	int rtn, cmd_size;
	
	//printf("analog cmd close\n");
	
	cmd_size = 7;
	memset(buf, 0, sizeof(buf));
	buf[0] = 0x1B; //ESC
	buf[1] = 'C';
	buf[2] = ' ';

	memset(tmp, 0, sizeof(tmp));
	sprintf(tmp, "%02d", bd);
	memcpy((char *)&buf[3], (char *)&tmp, 2);

	buf[5] = 0x0D; //CR
	buf[6] = 0x0A; //\n
	
	rtn = send_command((char *)&buf, cmd_size, 0);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "send cmd error5 !!!\n");
	}

	//usleep(250000);
	
	memset((char *)&myPs->rcvPacket, 0, sizeof(S_ANALOG_METER_RCV_PACKET));
	memset((char *)&myPs->rcvCmd, 0, sizeof(S_ANALOG_METER_RCV_COMMAND));
}
int rcv_cmd_answer2(void)
{
	char buf[12];
	int i, temp_bd, temp_ch;
	long temp;
	S_ANALOG_METER_RCV_CMD_ANSWER2	answer;
	
//	userlog(DEBUG_LOG, psName, "test1 %d %d\n",
//		sizeof(S_ANALOG_METER_RCV_CMD_ANSWER2), myPs->rcvCmd.cmdSize);
	memset((char *)&answer, 0, sizeof(S_ANALOG_METER_RCV_CMD_ANSWER2));
	memcpy((char *)&answer, (char *)&myPs->rcvCmd.cmd, myPs->rcvCmd.cmdSize);
	if(answer.Status == 'N') {
		memset(buf, 0, sizeof(buf));
		memcpy((char *)&buf, (char *)&answer.Sign, 10);
		temp = atoi(buf) * 100;
	} else if(answer.Status == 'O') {
		temp = 9999000;
	} else if((answer.Status == 'S') || (answer.Status == 'E')) {
		temp = -9999000;
	} else temp = -9999000;

	if((answer.CH[0] >= '0') && (answer.CH[0] <= '9')) {
		if((answer.CH[1] >= '0') && (answer.CH[1] <= '9')) {
		} else temp = -9999000;
	} else temp = -9999000;

	if(temp != (-9999000)) {
		memset(buf, 0, sizeof(buf));
		memcpy((char *)&buf, (char *)&answer.CH[0], 2);
		temp_ch = atoi(buf) - 1;
		if((temp_ch < 0) || (temp_ch > 15)) temp_ch = (-1);
	} else temp_ch = (-1);

	//userlog(DEBUG_LOG, psName, "test2 %ld %d\n", temp, temp_ch);
	////170319	
	if(myPs->signal[ANALOG_METER_SIG_MEASURE] <= 19){
		temp_bd = (int)(myPs->signal[ANALOG_METER_SIG_MEASURE] % 10) - 1;
	}else{	//for 10 Module
		temp_bd = 9;
	}
//	temp_bd = ((int)myPs->signal[ANALOG_METER_SIG_MEASURE] % 10) - 1;
	if(temp_bd < 0) {
		userlog(DEBUG_LOG, psName, "AnalogValue sig:%d, ch:%d val:%f\n",
			myPs->signal[ANALOG_METER_SIG_MEASURE], temp_ch,
			(float)temp / 1000.0);
		return 0;
	}

	if(temp_bd != ((int)myPs->signal[ANALOG_METER_SIG_COMM_BUS_ENABLE] - 1)) {
		userlog(DEBUG_LOG, psName, "AnalogValue bd_num %d %d\n",
			myPs->signal[ANALOG_METER_SIG_COMM_BUS_ENABLE]-1, temp_bd);
		return 0;
	}

	if((temp_ch >= 0) && (temp != (-9999000))) {
	//	userlog(DEBUG_LOG, psName, "test3 %d %d\n", temp_bd, temp_ch);
		i = temp_bd * 16 + temp_ch; //temp_hw_no index
		myPs->temp[i].temp = temp;
	//	myData->bData[0].cData[i].op.temp = temp;
		          
	}
		
/*		index = (int)myPs->Array2[i][0] - 1; //temp_monitor_no index
		if(index >= 0) {
			//ch = myPs->Array1[index][1] - 1;
			ch = index;
			myPs->tmp_value[ch] = temp + myPs->config.measure_offset[temp_bd];
			if(ch == 1) myPs->tmp_value[ch] = 21200;
			else if(ch == 15) myPs->tmp_value[ch] = 36300;
			else if(ch == 17) myPs->tmp_value[ch] = 20500;
			else if(ch == 30) myPs->tmp_value[ch] = 23900;
			else myPs->tmp_value[ch] = 9999000;
			myPs->value[ch] = myPs->tmp_value[ch];
		}
	}*/

	if(temp_ch != 15) return 0;

	memset((char *)&myPs->rcvPacket, 0, sizeof(S_ANALOG_METER_RCV_PACKET));
	memset((char *)&myPs->rcvCmd, 0, sizeof(S_ANALOG_METER_RCV_COMMAND));

//	110621 oys w : For AnalogMeter Process Kill (restart)
	myData->serialCheckTime2 = myData->mData.misc.timer_1sec;	

//	userlog(DEBUG_LOG, psName, "test4 %d %d\n",
//		myPs->signal[ANALOG_METER_SIG_MEASURE],
//		myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR]);
	if(myPs->signal[ANALOG_METER_SIG_MEASURE] == P31) {
		if(myPs->config.countMeter >= 1) {
			myPs->signal[ANALOG_METER_SIG_MEASURE] = P41;
		} else {
			myPs->signal[ANALOG_METER_SIG_MEASURE] = P21;
		}
		myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] = 0;
	} else if(myPs->signal[ANALOG_METER_SIG_MEASURE] == P32) {
		myPs->signal[ANALOG_METER_SIG_MEASURE] += 10;
		myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] = 0;
	} else if(myPs->signal[ANALOG_METER_SIG_MEASURE] == P33) {
		myPs->signal[ANALOG_METER_SIG_MEASURE] += 10;
		myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] = 0;
	} else if(myPs->signal[ANALOG_METER_SIG_MEASURE] == P34) {
		myPs->signal[ANALOG_METER_SIG_MEASURE] += 10;
		myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] = 0;
	}

	return 0;
}
int rcv_cmd_open_close(void)
{
	char buf[4];
	int bd, measure_bd;

	memset((char *)&buf, 0, sizeof buf);
	memcpy((char *)&buf, (char *)&myPs->rcvCmd.cmd[3], 2);
	bd = atoi(buf);
	measure_bd = (int)myPs->signal[ANALOG_METER_SIG_MEASURE] % 10;

	if(myPs->rcvCmd.cmd[1] == 'O') {
		if(bd >= 1 && bd <= myPs->config.countMeter) {
			if(bd == measure_bd) {
				myPs->signal[ANALOG_METER_SIG_COMM_BUS_ENABLE]
					= (unsigned char)bd;
				myPs->signal[ANALOG_METER_SIG_MEASURE] += 10;
				myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] = 0;
				memset((char *)&myPs->rcvPacket, 0,
					sizeof(S_ANALOG_METER_RCV_PACKET));
				memset((char *)&myPs->rcvCmd, 0,
					sizeof(S_ANALOG_METER_RCV_COMMAND));
			}
		}
	} else if(myPs->rcvCmd.cmd[1] == 'C') {
		if(bd >= 1 && bd <= myPs->config.countMeter) {
			if(bd == measure_bd) {
				myPs->signal[ANALOG_METER_SIG_COMM_BUS_ENABLE] = P0;
				/*if(myPs->signal[ANALOG_METER_SIG_MEASURE] < P71) {
					myPs->signal[ANALOG_METER_SIG_MEASURE] += 10;
				} else {*/
					if((bd+1) <= myPs->config.countMeter) {
						myPs->signal[ANALOG_METER_SIG_MEASURE] = (int)(bd+1);
					} else {
						myPs->signal[ANALOG_METER_SIG_MEASURE] = P1;
					}
				//}
				myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] = 0;
				memset((char *)&myPs->rcvPacket, 0,
					sizeof(S_ANALOG_METER_RCV_PACKET));
				memset((char *)&myPs->rcvCmd, 0,
					sizeof(S_ANALOG_METER_RCV_COMMAND));
			}
		}
	} else {
		userlog(DEBUG_LOG, psName, "Open_Close error %d : 0x%x\n",
			myPs->signal[ANALOG_METER_SIG_MEASURE], (char)myPs->rcvCmd.cmd[1]);
		return -1;
	}

	return 0;
}
void make_header(char *cmd, char reply, int cmd_id, int seqno, int body_size)
{
	S_ANALOG_METER_CMD_HEADER	header;
	
	memset((char *)&header, 0x00, sizeof(S_ANALOG_METER_CMD_HEADER));
	
	memcpy(cmd, (char *)&header, sizeof(S_ANALOG_METER_CMD_HEADER));
	
/*	userlog(METER2_LOG, psName, "header");
	for(i=0; i < sizeof(S_ANALOG_METER_CMD_HEADER); i++) {
		userlog2(METER2_LOG, psName, " %x", *(cmd + i));
	}
	userlog2(METER2_LOG, psName, ":end\n");
	userlog(METER2_LOG, psName, "header %s:end\n", cmd); //kjgd*/
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
	/*
	ioctl(myPs->config.ttyS_fd, TIOCMBIS, &modemctlline);
	modemctlline = TIOCM_RTS;
	ioctl(myPs->config.ttyS_fd, TIOCMBIS, &modemctlline);
*/
	i = writen(myPs->config.ttyS_fd, packet, size);
		
/*
	while(1)
	{
	    ioctl(myPs->config.ttyS_fd, TIOCSERGETLSR, &txemptystate);
	    if(txemptystate) break;
	}
	modemctlline = TIOCM_RTS;
	ioctl(myPs->config.ttyS_fd, TIOCMBIC, &modemctlline);
*/
	tcdrain(myPs->config.ttyS_fd);
	usleep(20000);
	return i;
}

int ComPortStateCheck(void)
{
	return 0;
}
