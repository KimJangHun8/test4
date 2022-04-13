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
extern volatile S_CALI_METER  *myPs;
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
		if(myPs->config.Lan_Use == P1){
			myPs->signal[CALI_METER_SIG_LAN_CONNECT] = P0;
		}
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
	
	if(myPs->rcvCmd.cmdBuf[0] == '+' || myPs->rcvCmd.cmdBuf[0] == '-') {
		for(i=0; i < myPs->rcvCmd.cmdBufSize; i++) {
			if(myPs->rcvCmd.cmdBuf[i] == '\n') {
				cmd_size = i+1; //one base
				break;
			}
		}
		if(cmd_size == 0) return -2;
	} else {
		myPs->rcvCmd.cmdBufSize = 0;
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
{ //Agilent 34401A
	int		rtn, i;
	S_CALI_METER_CMD_HEADER	header;

	memset((char *)&header, 0x00, sizeof(S_CALI_METER_CMD_HEADER));
	memcpy((char *)&header, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_CALI_METER_CMD_HEADER));
	
	if(myPs->config.CmdRcvLog == P1) {
		if(myPs->config.CommCheckLog == P1) {
			userlog(METER_LOG, psName, "recvCmd %s:end\n", myPs->rcvCmd.cmd);
		} else {
			userlog(METER_LOG, psName, "recvCmd %s:end\n", myPs->rcvCmd.cmd);
		}
	}
	
	if(myPs->config.CmdRcvLog_Hex == P1) {
		if(myPs->config.CommCheckLog == P1) {
			userlog(METER_LOG, psName, "recvCmd");
			for(i=0; i < myPs->rcvCmd.cmdSize; i++) {
				userlog2(METER_LOG, psName, " %x", myPs->rcvCmd.cmd[i]);
			}
			userlog2(METER_LOG, psName, ":end\n");
		} else {
			userlog(METER_LOG, psName, "recvCmd");
			for(i=0; i < myPs->rcvCmd.cmdSize; i++) {
				userlog2(METER_LOG, psName, " %x", myPs->rcvCmd.cmd[i]);
			}
			userlog2(METER_LOG, psName, ":end\n");
		}
	}

	rtn = CmdHeader_Check((char *)&header);
	if(rtn < 0) return -1;
	
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
	char buf[12];
	int	length;
	long tmp;
	S_CALI_METER_CMD_HEADER	header;

	length = sizeof(S_CALI_METER_CMD_HEADER);
	memset((char *)&header, 0x00, length);
	memcpy((char *)&header, (char *)rcvHeader, length);


	//161220 lyh add for lan Cali	
	if(myPs->signal[CALI_METER_SIG_LAN_USE] == P1){
		myPs->rcvCmd.cmdSize = myPs->rcvCmd.cmdSize+1;	
	}

	if(length != myPs->rcvCmd.cmdSize) {
		userlog(DEBUG_LOG, psName, "RcvCmd size error (%d:%d)\n",
			length, myPs->rcvCmd.cmdSize);
		return -1;
	}
	
	if(header.sign1 != ' ' && header.sign1 != '+' && header.sign1 != '-') {
		userlog(DEBUG_LOG, psName, "RcvCmd sign1 error : 0x%x\n", header.sign1);
		return -2;
	}
	
	if(header.digit1 < '0' || header.digit1 > '9') {
		userlog(DEBUG_LOG, psName, "RcvCmd digit1 error : 0x%x\n",
			header.digit1);
		return -3;
	}

	if(header.dot != '.') {
		userlog(DEBUG_LOG, psName, "RcvCmd dot error : 0x%x\n", header.dot);
		return -4;
	}

	memset(buf, 0x00, sizeof buf);
	memcpy((char *)&buf[0], (char *)&header.digit2[0], 8);
	tmp = atol(buf);
	if(tmp < 0 || tmp > 99999999) {
		userlog(DEBUG_LOG, psName, "RcvCmd digit2 error : %ld\n", tmp);
		return -5;
	}

	if(header.exponent != 'E' && header.exponent != 'e') {
		userlog(DEBUG_LOG, psName, "RcvCmd exponent error : 0x%x\n",
			header.exponent);
		return -6;
	}

	if(header.sign2 != ' ' && header.sign2 != '+' && header.sign2 != '-') {
		userlog(DEBUG_LOG, psName, "RcvCmd sign2 error : 0x%x\n", header.sign2);
		return -7;
	}
	
	memset(buf, 0x00, sizeof buf);
	memcpy((char *)&buf[0], (char *)&header.digit3[0], 2);
	tmp = atol(buf);
	if(tmp < 0 || tmp > 99) {
		userlog(DEBUG_LOG, psName, "RcvCmd digit3 error : %ld\n", tmp);
		return -8;
	}
	//161220 add header.cr 0x0A for lan cali	
	if(header.cr != 0x0D && header.cr != 0x0A) {
		userlog(DEBUG_LOG, psName, "RcvCmd cr error : 0x%x\n", header.cr);
		return -9;
	}
	//161220 add header.nl 0x0 for lan cali
	if(header.nl != '\n' && header.nl != 0x00) {
		userlog(DEBUG_LOG, psName, "RcvCmd nl error : 0x%x\n", header.nl);
		return -10;
	}

	return 0;
}

int rcv_cmd_answer(void)
{
	char buf[16];
	double val, diff;
	int tmp, i, exp, bd, ch, rangeV = 0, rangeI = 0;
	float ratioV, ratioI;
//	long real, real1, temp;
	double real, real1, temp;
	S_CALI_METER_RCV_CMD_ANSWER	answer;
	
	memset((char *)&answer, 0x00, sizeof(S_CALI_METER_RCV_CMD_ANSWER));
	memcpy((char *)&answer, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_CALI_METER_RCV_CMD_ANSWER));
	
	memset(buf, 0x00, sizeof buf);
	memcpy((char *)&buf[0], (char *)&answer.header.sign1, 15);
	val = strtod(buf, (char **)NULL);
	//userlog(DEBUG_LOG, psName, "RcvCmd value : %f \n", val);
	
	for(i=0; i < myData->mData.config.installedCh; i++) {
		bd = myData->CellArray1[i].bd;
		ch = myData->CellArray1[i].ch;
		if(myData->bData[bd].cData[ch].op.state == C_CALI)
			break;
	}

	if(myData->mData.config.ratioVoltage == 1){ //20190909
		ratioV = 1000000.0;
	} else {
		ratioV = 1000.0;
	}
	if(myData->mData.config.ratioCurrent == 1){
		ratioI = 1000000.0;
	} else {
		ratioI = 1000.0;
	}
	
	temp = 10000000;
	tmp = answer.header.digit1 - 0x30; //char to int
	real1 = tmp * 100000000;
	for(i=0; i < 8; i++) {
		tmp = answer.header.digit2[i] - 0x30;
		real1 += tmp * temp;
		temp /= 10;
	}

	tmp = answer.header.digit3[0] - 0x30;
	exp = tmp * 10;
	tmp = answer.header.digit3[1] - 0x30;
	exp += tmp;

	temp = 1;
	for(i=0; i < exp; i++) temp *= 10;
	
	if(answer.header.sign2 == '-') real = real1 / temp;
	else real = real1 * temp;

	if(answer.header.sign1 == '-') real *= -1;
	
	myPs->value = real;
	myPs->orgValue = real;

	printf("shunt	: ");
	printf("%c[1;31m",27);  //31:RED
	printf("%.3f", myData->CaliMeter.shunt_mOhm);
	printf("%c[0m",27);     //default
	printf(" mOhm\n");

	if(myPs->caliType == CALI_I) {
		rangeI = myData->cali.tmpCond[bd][ch].range;
		diff = (((real/myPs->shunt_mOhm)*1000.0)/100000.0)
					- (myData->bData[bd].cData[ch].misc.cmd_i/ratioI);
		printf("cmd_i	: "); 
		printf("%c[1;33m",27);	//33:YELLOW
//		printf("%ld", myData->bData[bd].cData[ch].misc.cmd_i/ratioI);
		printf("%.3f", (float)myData->bData[bd].cData[ch].misc.cmd_i/ratioI);
		printf("%c[0m",27);		//default
		printf(" mA\n");

		myPs->value = (real / myPs->shunt_mOhm) * 1000.0;
		printf("Meter2	: ");
	   	printf("%c[1;31m",27);  //31:RED
		printf("%.3f", (float)myPs->value/100000.0);
		printf("%c[0m",27);     //default
		printf(" mA\n");
		printf("Diff	: ");
		if(labs(diff) > ((myData->bData[bd].cData[ch].misc.cmd_i/1000)*0.001)) {
			printf("%c[1;5;31m",27);	//31:BLINK RED
			printf("%.3f", (float)diff);
		}else{
			printf("%c[1;32m",27);	//31:GREEN
			printf("%.3f", (float)diff);
		}
		printf("%c[0m",27);		//default
		printf(" mA\n\n");
	} else {
		rangeV = myData->cali.tmpCond[bd][ch].range;
		if(myData->mData.config.function[F_I_OFFSET_CALI] == P1){
			printf("cmd_i : "); 
			printf("%c[1;33m",27);	//33:YELLOW
			printf("%ld", myData->bData[bd].cData[ch].misc.cmd_i/1000);
			printf("%c[0m",27);		//default
			printf(" mA\n");
			
			myPs->value = (real / myData->CaliMeter.shunt_mOhm) * 1000.0;
			printf("Meter2	: ");
			printf("%c[1;31m",27);  //31:RED
			printf("%.3f", (float)myPs->value/100000.0);
			printf("%c[0m",27);     //default
			printf(" mA, AD : ");
			printf("%c[1;31m",27);  //31:RED
			printf("%.3f", myData->cali.orgAD_caliMeter2[1]/1000.0);
			printf("%c[0m",27);     //default
			printf(" mA\n");
			printf("Diff	: ");
			printf("%.3f", diff);
			printf(" mA\n\n");
		}else{
			diff = (myPs->value/100000.0)
						- (myData->bData[bd].cData[ch].misc.cmd_v/ratioV);
			printf("cmd_v	: ");
			printf("%c[1;33m",27);	//33:YELLOW
			printf("%.3f", myData->bData[bd].cData[ch].misc.cmd_v/ratioV);
			printf("%c[0m",27);		//default
			printf(" mV\n");

			printf("Meter2	: ");
			printf("%c[1;31m",27);  //31:RED
			printf("%.3f",(float)myPs->value/100000.0);
			printf("%c[0m",27);     //default
			printf(" mV\n");
			printf("Diff	: ");
			if(labs(diff)
				> labs((myData->mData.config.maxVoltage[rangeV]/1000)*0.001)) {
				printf("%c[1;5;31m",27);	//31:BLINK RED
				printf("%.3f", diff);
			}else{
				printf("%c[1;32m",27);	//31:GREEN
				printf("%.3f", diff);
			}
			printf("%c[0m",27);		//default
			printf(" mV\n\n");
		}
	}
	if(myPs->signal[CALI_METER_SIG_REQUEST_PHASE] == P1) {
		myPs->signal[CALI_METER_SIG_REQUEST_PHASE] = P2;
	}
	return 0;
}

void send_cmd_initialize(int type)
{
	int rtn, cmd_size;
	S_CALI_METER_SEND_CMD_INITIALIZE	cmd;
	
	memset((char *)&cmd, 0x00, sizeof(S_CALI_METER_SEND_CMD_INITIALIZE));

	switch(type) {
		case 1:
			cmd_size = 5;
			memcpy((char *)&cmd.data[0], "*RST\n", cmd_size);
			break;
		case 2:
			if(myPs->signal[CALI_METER_SIG_LAN_USE] == P1){		
				cmd_size = 10;
				memcpy((char *)&cmd.data[0], "SYST:PRES\n", cmd_size);
			}else{
				cmd_size = 9;
				memcpy((char *)&cmd.data[0], "SYST:REM\n", cmd_size);
			}
			break;
		case 3:
			cmd_size = 22;
			memcpy((char *)&cmd.data[0], "CONF:VOLT:DC DEF, DEF\n", cmd_size);
			break;
		case 4:
			cmd_size = 15;
			memcpy((char *)&cmd.data[0], "SAMPLE:COUNT 1\n", cmd_size);
			break;
		case 5:
			cmd_size = 22;
			memcpy((char *)&cmd.data[0], "CONF:CURR:DC DEF, DEF\n", cmd_size);
			break;
		default: break;
	}
	
	rtn = send_command((char *)&cmd, cmd_size, 0);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "send cmd error !!!\n");
	}
}

void send_cmd_request(void)
{
	int rtn, cmd_size;
	S_CALI_METER_SEND_CMD_REQUEST	cmd;
	
	memset((char *)&cmd, 0x00, sizeof(S_CALI_METER_SEND_CMD_REQUEST));

	cmd_size = 6;
	memcpy((char *)&cmd.data[0], "READ?\n", cmd_size);
	
	rtn = send_command((char *)&cmd, cmd_size, 0);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "send cmd error !!!\n");
	}
}

void make_header(char *cmd, char reply, int cmd_id, int seqno, int body_size)
{
	S_CALI_METER_CMD_HEADER	header;
	
	memset((char *)&header, 0x00, sizeof(S_CALI_METER_CMD_HEADER));
	
	memcpy(cmd, (char *)&header, sizeof(S_CALI_METER_CMD_HEADER));
	
/*	userlog(METER_LOG, psName, "header");
	for(i=0; i < sizeof(S_CALI_METER_CMD_HEADER); i++) {
		userlog2(METER_LOG, psName, " %x", *(cmd + i));
	}
	userlog2(METER_LOG, psName, ":end\n");
	userlog(METER_LOG, psName, "header %s:end\n", cmd); //for debug kjg*/
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

	if(myPs->config.CmdSendLog == P1) {
		if(myPs->config.CommCheckLog == P1) {
			userlog(METER_LOG, psName, "sendCmd %s:end\n", packet);
		} else {
			userlog(METER_LOG, psName, "sendCmd %s:end\n", packet);
		}
	}
	
	if(myPs->config.CmdSendLog_Hex == P1) {
		if(myPs->config.CommCheckLog == P1) {
			userlog(METER_LOG, psName, "sendCmd");
			for(i=0; i < size; i++) {
				userlog2(METER_LOG, psName, " %x", *(cmd + i));
			}
			userlog2(METER_LOG, psName, ":end\n");
		} else {
			userlog(METER_LOG, psName, "sendCmd");
			for(i=0; i < size; i++) {
				userlog2(METER_LOG, psName, " %x", *(cmd + i));
			}
			userlog2(METER_LOG, psName, ":end\n");
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
