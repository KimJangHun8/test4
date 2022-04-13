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
extern volatile S_FND_METER  *myPs;
extern char	psName[16];
extern CommParam comm_param;// = {"/dev/ttyS0", TMT_B96, TMT_FXONXOFF};

int ComPortInitialize()
{
	int tty , bps , commType, rtn;

	sleep(2);

	tty = myPs->config.comPort;
	bps = myPs->config.comBps;
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
		memset((char *)&myPs->rcvPacket,0,sizeof(S_FND_METER_RCV_PACKET));
		memset((char *)&myPs->rcvCmd,0,sizeof(S_FND_METER_RCV_COMMAND));
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


	if(myPs->rcvCmd.cmdBuf[0] == 0x02) {
		for(i=0; i < myPs->rcvCmd.cmdBufSize; i++) {
			if(myPs->rcvCmd.cmdBuf[i] == 0x03) {
				cmd_size = i+1; //one base
				break;
			}
		}
		if(cmd_size == 0) return -2;
	}else {
		for(i=0; i < myPs->rcvCmd.cmdBufSize; i++) {
			if(myPs->rcvCmd.cmdBuf[i] == 0x03) {
				cmd_size = i+1; //one base
			    break;
			}
		}
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
	int		rtn=0, i;
	S_FND_METER_CMD_HEADER	header;

	memset((char *)&header, 0x00, sizeof(S_FND_METER_CMD_HEADER));
	memcpy((char *)&header, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_FND_METER_CMD_HEADER));
	
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

//	rtn = rcv_cmd_answer();
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
	S_FND_METER_CMD_HEADER	header;

	length = sizeof(S_FND_METER_CMD_HEADER);
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
		if(header.addr2 < 0x31 || header.addr2 > 0x38){
			userlog(DEBUG_LOG, psName, "RcvCmd addr2 error : 0x%x\n",
				header.addr2);
			return -4;
		}
		#if 0
		// Increase Fnd Meter Module(CB-7018), Before support 4 analog 
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

void send_cmd_request(int fndbd, int fndch, int type)
{
	int rtn, cmd_size,i,checksum,tmp,checksum_type, ch;
	long val,val2;
	unsigned char val3;
	S_FND_METER_SEND_CMD	cmd;

	//display bd per ch = 2
	ch = 2*fndbd + fndch;
	
	memset((char *)&cmd, 0x00, sizeof(S_FND_METER_SEND_CMD));
	
	if(type== 1)
		cmd_size = 12;
	else
		cmd_size = 16;
	cmd.data[0] = 0x02;
	cmd.data[1] = '0';
	cmd.data[2] = 0x30+fndbd;

	if(type == 1) {
		val = 0;
		cmd.data[3] = 0x30+fndch;
		cmd.data[4] = '0';
		if(myData->bData[0].cData[ch].op.state == C_RUN)
		{
			val |= 0x08; //run
			if(myData->bData[0].cData[ch].ChAttribute.chNo_master == P0 
				|| myData->bData[0].cData[ch].ChAttribute.opType == P1){
				val |= 0x40;
				if(myData->bData[0].cData[ch].ChAttribute.chNo_master == P0){
					if((myData->bData[0].cData[ch-1].op.Isens 
						+ myData->bData[0].cData[ch].op.Isens) >= 0){ //charge
						val |= 0x10;
					}else if((myData->bData[0].cData[ch-1].op.Isens 
						+ myData->bData[0].cData[ch].op.Isens) < 0){ //charge
						val |= 0x20;
					}
				}
				if(myData->bData[0].cData[ch].ChAttribute.opType == P1){
					if((myData->bData[0].cData[ch].op.Isens 
						+ myData->bData[0].cData[ch+1].op.Isens) >= 0){ //charge
						val |= 0x10;
					}else if((myData->bData[0].cData[ch].op.Isens 
						+ myData->bData[0].cData[ch+1].op.Isens) < 0){ //charge
						val |= 0x20;
					}
				}
			}else{
				if(myData->bData[0].cData[ch].op.Isens >= 0) //charge
					val |= 0x10;
				else if(myData->bData[0].cData[ch].op.Isens < 0) //discharge
					val |= 0x20;
			}
			if(myData->bData[0].cData[ch].op.type == STEP_REST){
				//CHARGE & DISCHARGE LED OFF
				val &= 0xCF;
			}
		}
		val |= 0x04; //power

		val3 = (val & 0xF0) >> 4;
		if(val3 < 10) val3 += 0x30;
		else val3 += 0x37;
		cmd.data[5] = val3;
		val3 = val & 0x0F;
		if(val3 < 10) val3 += 0x30;
		else val3 += 0x37;
		cmd.data[6] = val3;
		val3 = 0;
	} else if(type == 2) {
		cmd.data[3] = 0x30+fndch;
		cmd.data[4] = '1';
		val = myData->bData[0].cData[ch].op.Vsens;
		if(myData->bData[0].cData[ch].ChAttribute.opType == P1){
			val = myData->bData[0].cData[ch].op.Vsens; 
		}
		if(myData->bData[0].cData[ch].ChAttribute.chNo_master == P0){ 
			val = 0;
		}
	} else if(type == 3) {
		cmd.data[3] = 0x30+fndch;
		cmd.data[4] = '2';
		if(myData->bData[0].cData[ch].op.state == C_RUN){
			val = myData->bData[0].cData[ch].op.Isens;
			if(myData->bData[0].cData[ch].ChAttribute.opType == P1){
				val = myData->bData[0].cData[ch].op.Isens 
						+ myData->bData[0].cData[ch+1].op.Isens;
			}
			if(myData->bData[0].cData[ch].ChAttribute.chNo_master == P0){ 
				val = 0;
			}
			if(myData->bData[0].cData[ch].op.type == STEP_REST){
				val = 0;
			}
		}else val = 0;
	//timer 1sec display test
	//	val = myData->mData.misc.timer_1sec * 1000;
	} else if(type == 4) {
		cmd.data[3] = 0x30+fndch;
		cmd.data[4] = '3';

		if(myData->bData[0].cData[ch].op.state == C_RUN) {
			if(myData->bData[0].cData[ch].op.type != STEP_USER_PATTERN) {
				if(myData->bData[0].cData[ch].op.Isens > 0){
					val = myData->bData[0].cData[ch].op.watt;
					if(myData->bData[0].cData[ch].ChAttribute.opType == P1){ 
						val = myData->bData[0].cData[ch].op.watt 
								+ myData->bData[0].cData[ch+1].op.watt;
					}
					if(myData->bData[0].cData[ch].ChAttribute.chNo_master== P0){
						val = 0;   
					}	
				}else{
					val = myData->bData[0].cData[ch].op.watt;
					if(myData->bData[0].cData[ch].ChAttribute.opType == P1){ 
			  			val = myData->bData[0].cData[ch].op.watt*(-1) 
								+ myData->bData[0].cData[ch+1].op.watt*(-1); 
					}
					if(myData->bData[0].cData[ch].ChAttribute.chNo_master== P0){
			  			val = 0;
					}
				}
			}else{	
				val = myData->bData[0].cData[ch].op.watt;
				if(myData->bData[0].cData[ch].ChAttribute.opType == P1){ 
			 	   val = myData->bData[0].cData[ch].op.watt 
						   + myData->bData[0].cData[ch+1].op.watt; 
				}
				if(myData->bData[0].cData[ch].ChAttribute.chNo_master == P0){
					val = 0;
				}
			}
			if(myData->bData[0].cData[ch].op.type == STEP_REST){
				val = 0;
			}
		} else val = 0;
	}
	
	if(type > 1)
	{
		if(val >= 0) {
			cmd.data[5] = ' ';
		} else {
			cmd.data[5] = '-';
			val = val * (-1);
		}
		if(val == 0x80000000) val = 2000000000;
		if(val >= 2000000000) {
			cmd.data[6] = 'F';
			cmd.data[7] = 'F';
			cmd.data[8] = 'F';
			cmd.data[9] = 'F';
			cmd.data[10] = 'F';
		} else if(val >= 1000000000) { //1000.
			cmd.data[6] = val / 1000000000 + 0x30;
			val2 = val % 1000000000;
				cmd.data[7] = val2 / 100000000 + 0x30;
			val2 = val2 % 100000000;
			cmd.data[8] = val2 / 10000000 + 0x30;
			val2 = val2 % 10000000;
			cmd.data[9] = val2 / 1000000 + 0x30;
				val2 = val2 % 1000000;
			if(val2 >= 500000 && cmd.data[9] < 0x39) cmd.data[9] += 1;
			cmd.data[10] = '.';
		} else if(val >= 100000000) { //100.0
			cmd.data[6] = val / 100000000 + 0x30;
			val2 = val % 100000000;
			cmd.data[7] = val2 / 10000000 + 0x30;
			val2 = val2 % 10000000;
			cmd.data[8] = val2 / 1000000 + 0x30;
			val2 = val2 % 1000000;
			cmd.data[9] = '.';
			cmd.data[10] = val2 / 100000 + 0x30;
			val2 = val2 % 100000;
			if(val2 >= 50000 && cmd.data[10] < 0x39) cmd.data[10] += 1;
		} else if(val >= 10000000) { //10.00
			cmd.data[6] = val / 10000000 + 0x30;
			val2 = val % 10000000;
			cmd.data[7] = val2 / 1000000 + 0x30;
			val2 = val2 % 1000000;
			cmd.data[8] = '.';
			cmd.data[9] = val2 / 100000 + 0x30;
			val2 = val2 % 100000;
			cmd.data[10] = val2 / 10000 + 0x30;
			val2 = val2 % 10000;
			if(val2 >= 5000 && cmd.data[10] < 0x39) cmd.data[10] += 1;
		} else if(val >= 1000000) { //1.000
			cmd.data[6] = val / 1000000 + 0x30;
			val2 = val % 1000000;
			cmd.data[7] = '.';
			cmd.data[8] = val2 / 100000 + 0x30;
			val2 = val2 % 100000;
			cmd.data[9] = val2 / 10000 + 0x30;
			val2 = val2 % 10000;
			cmd.data[10] = val2 / 1000 + 0x30;
			val2 = val2 % 1000;
			if(val2 >= 500 && cmd.data[10] < 0x39) cmd.data[10] += 1;
		} else if(val >= 100000) { //0.100
			cmd.data[6] = 0x30;
			cmd.data[7] = '.';
			cmd.data[8] = val / 100000 + 0x30;
			val2 = val % 100000;
			cmd.data[9] = val2 / 10000 + 0x30;
			val2 = val2 % 10000;
			cmd.data[10] = val2 / 1000 + 0x30;
			val2 = val2 % 1000;
			if(val2 >= 500 && cmd.data[10] < 0x39) cmd.data[10] += 1;
		} else if(val >= 10000) { //0.010
			cmd.data[6] = 0x30;
			cmd.data[7] = '.';
			cmd.data[8] = 0x30;
			cmd.data[9] = val / 10000 + 0x30;
			val2 = val % 10000;
			cmd.data[10] = val2 / 1000 + 0x30;
			val2 = val2 % 1000;
			if(val2 >= 500 && cmd.data[10] < 0x39) cmd.data[10] += 1;
		} else { //if(val >= 1000)  //0.001
			cmd.data[6] = 0x30;
			cmd.data[7] = '.';
			cmd.data[8] = 0x30;
			cmd.data[9] = 0x30;
			cmd.data[10] = val / 1000 + 0x30;
			val2 = val % 1000;
			if(val2 >= 500 && cmd.data[10] < 0x39) cmd.data[10] += 1;
		}
	}
	val3 = 1;

	checksum = 0;
	checksum_type = 1; //0:dec, 1:hex
	for(i=1; i < (cmd_size - 5); i++) {
		checksum += (int)cmd.data[i];
	}

	if(checksum_type == 0) { //dec
		tmp = (checksum / 1000) + 0x30;
		cmd.data[i] = (char)tmp;

		checksum = checksum % 1000;
		tmp = (checksum / 100) + 0x30;
		i++;
		cmd.data[i] = (char)tmp;

		checksum = checksum % 100;
		tmp = (checksum / 10) + 0x30;
		i++;
		cmd.data[i] = (char)tmp;

		tmp = (checksum % 10) + 0x30;
		i++;
		cmd.data[i] = (char)tmp;
	} else { //hex
		tmp = (checksum & 0xF000) >> 12;
		if(tmp < 10) {
			tmp += 0x30;
		} else {
			tmp += 0x37;
		}
		cmd.data[i] = (char)tmp;

		tmp = (checksum & 0x0F00) >> 8;
		if(tmp < 10) {
			tmp += 0x30;
		} else {
			tmp += 0x37;
		}
		i++;
		cmd.data[i] = (char)tmp;

		tmp = (checksum & 0x00F0) >> 4;
		if(tmp < 10) {
			tmp += 0x30;
		} else {
			tmp += 0x37;
		}
		i++;
		cmd.data[i] = (char)tmp;

		tmp = checksum & 0x000F;
		if(tmp < 10) {
			tmp += 0x30;
		} else {
			tmp += 0x37;
		}
		i++;
		cmd.data[i] = (char)tmp;
	}

	i++;
	cmd.data[i] = 0x03; //ETX

	rtn = send_command((char *)&cmd, cmd_size, 0);
	if(rtn < 0) {
			userlog(DEBUG_LOG,psName,"send error\n");
	}
}

void make_header(char *cmd, char reply, int cmd_id, int seqno, int body_size)
{
	S_FND_METER_CMD_HEADER	header;
	
	memset((char *)&header, 0x00, sizeof(S_FND_METER_CMD_HEADER));
	
	memcpy(cmd, (char *)&header, sizeof(S_FND_METER_CMD_HEADER));
	
/*	userlog(METER2_LOG, psName, "header");
	for(i=0; i < sizeof(S_FND_METER_CMD_HEADER); i++) {
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
//	int modemctlline, txemptystate;

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
	i = writen(myPs->config.ttyS_fd, packet, size);
		
	tcdrain(myPs->config.ttyS_fd);
	usleep(20000);
	return i;
}

int ComPortStateCheck(void)
{
	return 0;
}
