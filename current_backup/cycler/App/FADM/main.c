#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>
#include "../../INC/datastore.h"
#include "userlog.h"
#include "common_utils.h"
#include "local_utils.h"
#include "message.h"
#include "com_io.h"
#include "comm.h"
#include "serial.h"
#include "main.h"

volatile S_SYSTEM_DATA *myData;
volatile S_FADM *myPs; //my process : FastAnalogToDigitalConversionModule
char	psName[PROCESS_NAME_SIZE];

int main(void)
{
	int rtn;
	struct timeval tv;
	fd_set rfds;
	
	if(Initialize() < 0) return 0;
   	
	while(myData->AppControl.signal[APP_SIG_FADM_PROCESS] == P1) {
		tv.tv_sec = 0;
		tv.tv_usec = 250000;
		FD_ZERO(&rfds);
		if(myPs->config.ttyS_fd > 0)
			FD_SET(myPs->config.ttyS_fd, &rfds);
		
		rtn = Parsing_SerialEvent();

		rtn = select(myPs->config.ttyS_fd+1, &rfds, NULL, NULL, &tv);
		//userlog(DEBUG_LOG, psName, "event %d\n", rtn); //kjgd
		if(rtn > 0) {
			if(FD_ISSET(myPs->config.ttyS_fd, &rfds) == 1) {
				if(SerialPacket_Receive() < 0) {
					rtn = ComPortInitialize(myPs->config.comPort,
						myPs->config.comBps);
					if(rtn < 0) break;
					myPs->config.ttyS_fd = rtn;
					continue;
				}
				rtn = Parsing_SerialEvent();
			}
		} else if(rtn == 0) {
			FADM_Control();
			if(ComPortStateCheck() < 0) {
				rtn = ComPortInitialize(myPs->config.comPort,
					myPs->config.comBps);
				if(rtn < 0) break;
				myPs->config.ttyS_fd = rtn;
				continue;
			}
			rtn = Parsing_SerialEvent();
		} else {
		}
	}

	Close_Process();
	return 0;
}

int	Initialize(void)
{
	int rtn;
	
	if(Open_SystemMemory(0) < 0) return -1;
	
	myPs = &(myData->FADM);

	Init_SystemMemory();

	memset((char *)&psName[0], 0x00, 16);
	strcpy(psName, "FADM");
	
	if(Read_FADM_Config() < 0) return -2;
	
	rtn = ComPortInitialize(myPs->config.comPort, myPs->config.comBps);
	if(rtn < 0) return -3;
	myPs->config.ttyS_fd = rtn;

	myData->AppControl.signal[APP_SIG_FADM_PROCESS] = P1;
	return 0;
}

void FADM_Control(void)
{
	Check_Message();
	Check_Signal();
}

void Check_Signal(void)
{
	int ch, val, comm_idx;

	if(myPs->comm_buffer.write_idx != myPs->comm_buffer.read_idx) {
		comm_idx = myPs->comm_buffer.read_idx;
		comm_idx++;
		if(comm_idx >= MAX_FADM_USE_CH) comm_idx = 0;

		if(myPs->comm_buffer.use_flag == 0) {
			myPs->comm_buffer.use_flag = 1;
			myPs->comm_buffer.read_idx = comm_idx;
			if(myPs->signal[FADM_SIG_MEASURE_READY] == P0) {
				myPs->signal[FADM_SIG_MEASURE_READY] = P1;
				myPs->signal[FADM_SIG_READ_CH_READY]
					= (unsigned char)myPs->comm_buffer.use_ch[comm_idx];
				myPs->misc.use_totalCycle
					= myPs->comm_buffer.use_totalCycle[comm_idx];
				myPs->misc.use_stepNo
					= myPs->comm_buffer.use_stepNo[comm_idx];
			}
		}
	}

	switch(myPs->signal[FADM_SIG_MEASURE_READY]) { //1sec delay
		case P1:
		case P2:
		case P3:
		case P4:
			myPs->signal[FADM_SIG_MEASURE_READY]++;
			break;
		case P5:
			if(myPs->signal[FADM_SIG_MEASURE] == P0) {
				myPs->signal[FADM_SIG_READ_CH]
					= myPs->signal[FADM_SIG_READ_CH_READY];
				myPs->signal[FADM_SIG_MEASURE] = P1;
			}
			myPs->signal[FADM_SIG_MEASURE_READY] = P0;
			break;
		default:
			break;
	}

	switch(myPs->signal[FADM_SIG_MEASURE]) {
		case P1:
			myPs->signal[FADM_SIG_MEASURE] = P11;
			ch = (int)myPs->signal[FADM_SIG_READ_CH];
			val = ch % 2;
			if(val == 0) {
				val = 100;
			} else {
				val = 300;
			}
			ch = ch / 2;
			send_cmd_request(ch, val);
			break;
		case P2:
			myPs->signal[FADM_SIG_MEASURE] = P12;
			ch = (int)myPs->signal[FADM_SIG_READ_CH];
			val = ch % 2;
			if(val == 0) {
				val = 101;
			} else {
				val = 301;
			}
			ch = ch / 2;
			send_cmd_request(ch, val);
			break;
		case P3:
			myPs->signal[FADM_SIG_MEASURE] = P13;
			ch = (int)myPs->signal[FADM_SIG_READ_CH];
			val = ch % 2;
			if(val == 0) {
				val = 102;
			} else {
				val = 302;
			}
			ch = ch / 2;
			send_cmd_request(ch, val);
			break;
		case P4:
			myPs->signal[FADM_SIG_MEASURE] = P14;
			ch = (int)myPs->signal[FADM_SIG_READ_CH];
			val = ch % 2;
			if(val == 0) {
				val = 103;
			} else {
				val = 303;
			}
			ch = ch / 2;
			send_cmd_request(ch, val);
			break;
		case P5:
			myPs->signal[FADM_SIG_MEASURE] = P15;
			ch = (int)myPs->signal[FADM_SIG_READ_CH];
			val = ch % 2;
			if(val == 0) {
				val = 104;
			} else {
				val = 304;
			}
			ch = ch / 2;
			send_cmd_request(ch, val);
			break;
		case P6:
			myPs->signal[FADM_SIG_MEASURE] = P16;
			ch = (int)myPs->signal[FADM_SIG_READ_CH];
			val = ch % 2;
			if(val == 0) {
				val = 105;
			} else {
				val = 305;
			}
			ch = ch / 2;
			send_cmd_request(ch, val);
			break;
		case P7:
			myPs->signal[FADM_SIG_MEASURE] = P17;
			ch = (int)myPs->signal[FADM_SIG_READ_CH];
			val = ch % 2;
			if(val == 0) {
				val = 106;
			} else {
				val = 306;
			}
			ch = ch / 2;
			send_cmd_request(ch, val);
			break;
		case P8:
			myPs->signal[FADM_SIG_MEASURE] = P18;
			ch = (int)myPs->signal[FADM_SIG_READ_CH];
			val = ch % 2;
			if(val == 0) {
				val = 107;
			} else {
				val = 307;
			}
			ch = ch / 2;
			send_cmd_request(ch, val);
			break;
		case P9:
			myPs->signal[FADM_SIG_MEASURE] = P19;
			ch = (int)myPs->signal[FADM_SIG_READ_CH];
			val = ch % 2;
			if(val == 0) {
				val = 108;
			} else {
				val = 308;
			}
			ch = ch / 2;
			send_cmd_request(ch, val);
			break;
		case P10:
			myPs->signal[FADM_SIG_MEASURE] = P20;
			ch = (int)myPs->signal[FADM_SIG_READ_CH];
			val = ch % 2;
			if(val == 0) {
				val = 109;
			} else {
				val = 309;
			}
			ch = ch / 2;
			send_cmd_request(ch, val);
			break;
		case P11:
			myPs->signal[FADM_SIG_MEASURE_ERROR] += 1;
			if(myPs->signal[FADM_SIG_MEASURE_ERROR] >= 3) {
				userlog(DEBUG_LOG, psName, "measure error : %d, %d\n",
					(int)myPs->signal[FADM_SIG_MEASURE],
					(int)myPs->signal[FADM_SIG_MEASURE_ERROR]);
				memset((char *)&myPs->rcvPacket, 0, sizeof(S_FADM_RCV_PACKET));
				memset((char *)&myPs->rcvCmd, 0, sizeof(S_FADM_RCV_COMMAND));
				myPs->signal[FADM_SIG_MEASURE_ERROR] = 0;
				myPs->signal[FADM_SIG_MEASURE] = P1;
			}
			break;
		case P12:
			myPs->signal[FADM_SIG_MEASURE_ERROR] += 1;
			if(myPs->signal[FADM_SIG_MEASURE_ERROR] >= 3) {
				userlog(DEBUG_LOG, psName, "measure error : %d, %d\n",
					(int)myPs->signal[FADM_SIG_MEASURE],
					(int)myPs->signal[FADM_SIG_MEASURE_ERROR]);
				memset((char *)&myPs->rcvPacket, 0, sizeof(S_FADM_RCV_PACKET));
				memset((char *)&myPs->rcvCmd, 0, sizeof(S_FADM_RCV_COMMAND));
				myPs->signal[FADM_SIG_MEASURE_ERROR] = 0;
				myPs->signal[FADM_SIG_MEASURE] = P2;
			}
			break;
		case P13:
			myPs->signal[FADM_SIG_MEASURE_ERROR] += 1;
			if(myPs->signal[FADM_SIG_MEASURE_ERROR] >= 3) {
				userlog(DEBUG_LOG, psName, "measure error : %d, %d\n",
					(int)myPs->signal[FADM_SIG_MEASURE],
					(int)myPs->signal[FADM_SIG_MEASURE_ERROR]);
				memset((char *)&myPs->rcvPacket, 0, sizeof(S_FADM_RCV_PACKET));
				memset((char *)&myPs->rcvCmd, 0, sizeof(S_FADM_RCV_COMMAND));
				myPs->signal[FADM_SIG_MEASURE_ERROR] = 0;
				myPs->signal[FADM_SIG_MEASURE] = P3;
			}
			break;
		case P14:
			myPs->signal[FADM_SIG_MEASURE_ERROR] += 1;
			if(myPs->signal[FADM_SIG_MEASURE_ERROR] >= 3) {
				userlog(DEBUG_LOG, psName, "measure error : %d, %d\n",
					(int)myPs->signal[FADM_SIG_MEASURE],
					(int)myPs->signal[FADM_SIG_MEASURE_ERROR]);
				memset((char *)&myPs->rcvPacket, 0, sizeof(S_FADM_RCV_PACKET));
				memset((char *)&myPs->rcvCmd, 0, sizeof(S_FADM_RCV_COMMAND));
				myPs->signal[FADM_SIG_MEASURE_ERROR] = 0;
				myPs->signal[FADM_SIG_MEASURE] = P4;
			}
			break;
		case P15:
			myPs->signal[FADM_SIG_MEASURE_ERROR] += 1;
			if(myPs->signal[FADM_SIG_MEASURE_ERROR] >= 3) {
				userlog(DEBUG_LOG, psName, "measure error : %d, %d\n",
					(int)myPs->signal[FADM_SIG_MEASURE],
					(int)myPs->signal[FADM_SIG_MEASURE_ERROR]);
				memset((char *)&myPs->rcvPacket, 0, sizeof(S_FADM_RCV_PACKET));
				memset((char *)&myPs->rcvCmd, 0, sizeof(S_FADM_RCV_COMMAND));
				myPs->signal[FADM_SIG_MEASURE_ERROR] = 0;
				myPs->signal[FADM_SIG_MEASURE] = P5;
			}
			break;
		case P16:
			myPs->signal[FADM_SIG_MEASURE_ERROR] += 1;
			if(myPs->signal[FADM_SIG_MEASURE_ERROR] >= 3) {
				userlog(DEBUG_LOG, psName, "measure error : %d, %d\n",
					(int)myPs->signal[FADM_SIG_MEASURE],
					(int)myPs->signal[FADM_SIG_MEASURE_ERROR]);
				memset((char *)&myPs->rcvPacket, 0, sizeof(S_FADM_RCV_PACKET));
				memset((char *)&myPs->rcvCmd, 0, sizeof(S_FADM_RCV_COMMAND));
				myPs->signal[FADM_SIG_MEASURE_ERROR] = 0;
				myPs->signal[FADM_SIG_MEASURE] = P6;
			}
			break;
		case P17:
			myPs->signal[FADM_SIG_MEASURE_ERROR] += 1;
			if(myPs->signal[FADM_SIG_MEASURE_ERROR] >= 3) {
				userlog(DEBUG_LOG, psName, "measure error : %d, %d\n",
					(int)myPs->signal[FADM_SIG_MEASURE],
					(int)myPs->signal[FADM_SIG_MEASURE_ERROR]);
				memset((char *)&myPs->rcvPacket, 0, sizeof(S_FADM_RCV_PACKET));
				memset((char *)&myPs->rcvCmd, 0, sizeof(S_FADM_RCV_COMMAND));
				myPs->signal[FADM_SIG_MEASURE_ERROR] = 0;
				myPs->signal[FADM_SIG_MEASURE] = P7;
			}
			break;
		case P18:
			myPs->signal[FADM_SIG_MEASURE_ERROR] += 1;
			if(myPs->signal[FADM_SIG_MEASURE_ERROR] >= 3) {
				userlog(DEBUG_LOG, psName, "measure error : %d, %d\n",
					(int)myPs->signal[FADM_SIG_MEASURE],
					(int)myPs->signal[FADM_SIG_MEASURE_ERROR]);
				memset((char *)&myPs->rcvPacket, 0, sizeof(S_FADM_RCV_PACKET));
				memset((char *)&myPs->rcvCmd, 0, sizeof(S_FADM_RCV_COMMAND));
				myPs->signal[FADM_SIG_MEASURE_ERROR] = 0;
				myPs->signal[FADM_SIG_MEASURE] = P8;
			}
			break;
		case P19:
			myPs->signal[FADM_SIG_MEASURE_ERROR] += 1;
			if(myPs->signal[FADM_SIG_MEASURE_ERROR] >= 3) {
				userlog(DEBUG_LOG, psName, "measure error : %d, %d\n",
					(int)myPs->signal[FADM_SIG_MEASURE],
					(int)myPs->signal[FADM_SIG_MEASURE_ERROR]);
				memset((char *)&myPs->rcvPacket, 0, sizeof(S_FADM_RCV_PACKET));
				memset((char *)&myPs->rcvCmd, 0, sizeof(S_FADM_RCV_COMMAND));
				myPs->signal[FADM_SIG_MEASURE_ERROR] = 0;
				myPs->signal[FADM_SIG_MEASURE] = P9;
			}
			break;
		case P20:
			myPs->signal[FADM_SIG_MEASURE_ERROR] += 1;
			if(myPs->signal[FADM_SIG_MEASURE_ERROR] >= 3) {
				userlog(DEBUG_LOG, psName, "measure error : %d, %d\n",
					(int)myPs->signal[FADM_SIG_MEASURE],
					(int)myPs->signal[FADM_SIG_MEASURE_ERROR]);
				memset((char *)&myPs->rcvPacket, 0, sizeof(S_FADM_RCV_PACKET));
				memset((char *)&myPs->rcvCmd, 0, sizeof(S_FADM_RCV_COMMAND));
				myPs->signal[FADM_SIG_MEASURE_ERROR] = 0;
				myPs->signal[FADM_SIG_MEASURE] = P10;
			}
			break;
		case P21:
			myPs->signal[FADM_SIG_MEASURE] = P31;
			ch = (int)myPs->signal[FADM_SIG_READ_CH];
			val = ch % 2;
			if(val == 0) {
				val = 200;
			} else {
				val = 400;
			}
			ch = ch / 2;
			send_cmd_request(ch, val);
			break;
		case P22:
			myPs->signal[FADM_SIG_MEASURE] = P32;
			ch = (int)myPs->signal[FADM_SIG_READ_CH];
			val = ch % 2;
			if(val == 0) {
				val = 201;
			} else {
				val = 401;
			}
			ch = ch / 2;
			send_cmd_request(ch, val);
			break;
		case P23:
			myPs->signal[FADM_SIG_MEASURE] = P33;
			ch = (int)myPs->signal[FADM_SIG_READ_CH];
			val = ch % 2;
			if(val == 0) {
				val = 202;
			} else {
				val = 402;
			}
			ch = ch / 2;
			send_cmd_request(ch, val);
			break;
		case P24:
			myPs->signal[FADM_SIG_MEASURE] = P34;
			ch = (int)myPs->signal[FADM_SIG_READ_CH];
			val = ch % 2;
			if(val == 0) {
				val = 203;
			} else {
				val = 403;
			}
			ch = ch / 2;
			send_cmd_request(ch, val);
			break;
		case P25:
			myPs->signal[FADM_SIG_MEASURE] = P35;
			ch = (int)myPs->signal[FADM_SIG_READ_CH];
			val = ch % 2;
			if(val == 0) {
				val = 204;
			} else {
				val = 404;
			}
			ch = ch / 2;
			send_cmd_request(ch, val);
			break;
		case P26:
			myPs->signal[FADM_SIG_MEASURE] = P36;
			ch = (int)myPs->signal[FADM_SIG_READ_CH];
			val = ch % 2;
			if(val == 0) {
				val = 205;
			} else {
				val = 405;
			}
			ch = ch / 2;
			send_cmd_request(ch, val);
			break;
		case P27:
			myPs->signal[FADM_SIG_MEASURE] = P37;
			ch = (int)myPs->signal[FADM_SIG_READ_CH];
			val = ch % 2;
			if(val == 0) {
				val = 206;
			} else {
				val = 406;
			}
			ch = ch / 2;
			send_cmd_request(ch, val);
			break;
		case P28:
			myPs->signal[FADM_SIG_MEASURE] = P38;
			ch = (int)myPs->signal[FADM_SIG_READ_CH];
			val = ch % 2;
			if(val == 0) {
				val = 207;
			} else {
				val = 407;
			}
			ch = ch / 2;
			send_cmd_request(ch, val);
			break;
		case P29:
			myPs->signal[FADM_SIG_MEASURE] = P39;
			ch = (int)myPs->signal[FADM_SIG_READ_CH];
			val = ch % 2;
			if(val == 0) {
				val = 208;
			} else {
				val = 408;
			}
			ch = ch / 2;
			send_cmd_request(ch, val);
			break;
		case P30:
			myPs->signal[FADM_SIG_MEASURE] = P40;
			ch = (int)myPs->signal[FADM_SIG_READ_CH];
			val = ch % 2;
			if(val == 0) {
				val = 209;
			} else {
				val = 409;
			}
			ch = ch / 2;
			send_cmd_request(ch, val);
			break;
		case P31:
			myPs->signal[FADM_SIG_MEASURE_ERROR] += 1;
			if(myPs->signal[FADM_SIG_MEASURE_ERROR] >= 3) {
				userlog(DEBUG_LOG, psName, "measure error : %d, %d\n",
					(int)myPs->signal[FADM_SIG_MEASURE],
					(int)myPs->signal[FADM_SIG_MEASURE_ERROR]);
				memset((char *)&myPs->rcvPacket, 0, sizeof(S_FADM_RCV_PACKET));
				memset((char *)&myPs->rcvCmd, 0, sizeof(S_FADM_RCV_COMMAND));
				myPs->signal[FADM_SIG_MEASURE_ERROR] = 0;
				myPs->signal[FADM_SIG_MEASURE] = P21;
			}
			break;
		case P32:
			myPs->signal[FADM_SIG_MEASURE_ERROR] += 1;
			if(myPs->signal[FADM_SIG_MEASURE_ERROR] >= 3) {
				userlog(DEBUG_LOG, psName, "measure error : %d, %d\n",
					(int)myPs->signal[FADM_SIG_MEASURE],
					(int)myPs->signal[FADM_SIG_MEASURE_ERROR]);
				memset((char *)&myPs->rcvPacket, 0, sizeof(S_FADM_RCV_PACKET));
				memset((char *)&myPs->rcvCmd, 0, sizeof(S_FADM_RCV_COMMAND));
				myPs->signal[FADM_SIG_MEASURE_ERROR] = 0;
				myPs->signal[FADM_SIG_MEASURE] = P22;
			}
			break;
		case P33:
			myPs->signal[FADM_SIG_MEASURE_ERROR] += 1;
			if(myPs->signal[FADM_SIG_MEASURE_ERROR] >= 3) {
				userlog(DEBUG_LOG, psName, "measure error : %d, %d\n",
					(int)myPs->signal[FADM_SIG_MEASURE],
					(int)myPs->signal[FADM_SIG_MEASURE_ERROR]);
				memset((char *)&myPs->rcvPacket, 0, sizeof(S_FADM_RCV_PACKET));
				memset((char *)&myPs->rcvCmd, 0, sizeof(S_FADM_RCV_COMMAND));
				myPs->signal[FADM_SIG_MEASURE_ERROR] = 0;
				myPs->signal[FADM_SIG_MEASURE] = P23;
			}
			break;
		case P34:
			myPs->signal[FADM_SIG_MEASURE_ERROR] += 1;
			if(myPs->signal[FADM_SIG_MEASURE_ERROR] >= 3) {
				userlog(DEBUG_LOG, psName, "measure error : %d, %d\n",
					(int)myPs->signal[FADM_SIG_MEASURE],
					(int)myPs->signal[FADM_SIG_MEASURE_ERROR]);
				memset((char *)&myPs->rcvPacket, 0, sizeof(S_FADM_RCV_PACKET));
				memset((char *)&myPs->rcvCmd, 0, sizeof(S_FADM_RCV_COMMAND));
				myPs->signal[FADM_SIG_MEASURE_ERROR] = 0;
				myPs->signal[FADM_SIG_MEASURE] = P24;
			}
			break;
		case P35:
			myPs->signal[FADM_SIG_MEASURE_ERROR] += 1;
			if(myPs->signal[FADM_SIG_MEASURE_ERROR] >= 3) {
				userlog(DEBUG_LOG, psName, "measure error : %d, %d\n",
					(int)myPs->signal[FADM_SIG_MEASURE],
					(int)myPs->signal[FADM_SIG_MEASURE_ERROR]);
				memset((char *)&myPs->rcvPacket, 0, sizeof(S_FADM_RCV_PACKET));
				memset((char *)&myPs->rcvCmd, 0, sizeof(S_FADM_RCV_COMMAND));
				myPs->signal[FADM_SIG_MEASURE_ERROR] = 0;
				myPs->signal[FADM_SIG_MEASURE] = P25;
			}
			break;
		case P36:
			myPs->signal[FADM_SIG_MEASURE_ERROR] += 1;
			if(myPs->signal[FADM_SIG_MEASURE_ERROR] >= 3) {
				userlog(DEBUG_LOG, psName, "measure error : %d, %d\n",
					(int)myPs->signal[FADM_SIG_MEASURE],
					(int)myPs->signal[FADM_SIG_MEASURE_ERROR]);
				memset((char *)&myPs->rcvPacket, 0, sizeof(S_FADM_RCV_PACKET));
				memset((char *)&myPs->rcvCmd, 0, sizeof(S_FADM_RCV_COMMAND));
				myPs->signal[FADM_SIG_MEASURE_ERROR] = 0;
				myPs->signal[FADM_SIG_MEASURE] = P26;
			}
			break;
		case P37:
			myPs->signal[FADM_SIG_MEASURE_ERROR] += 1;
			if(myPs->signal[FADM_SIG_MEASURE_ERROR] >= 3) {
				userlog(DEBUG_LOG, psName, "measure error : %d, %d\n",
					(int)myPs->signal[FADM_SIG_MEASURE],
					(int)myPs->signal[FADM_SIG_MEASURE_ERROR]);
				memset((char *)&myPs->rcvPacket, 0, sizeof(S_FADM_RCV_PACKET));
				memset((char *)&myPs->rcvCmd, 0, sizeof(S_FADM_RCV_COMMAND));
				myPs->signal[FADM_SIG_MEASURE_ERROR] = 0;
				myPs->signal[FADM_SIG_MEASURE] = P27;
			}
			break;
		case P38:
			myPs->signal[FADM_SIG_MEASURE_ERROR] += 1;
			if(myPs->signal[FADM_SIG_MEASURE_ERROR] >= 3) {
				userlog(DEBUG_LOG, psName, "measure error : %d, %d\n",
					(int)myPs->signal[FADM_SIG_MEASURE],
					(int)myPs->signal[FADM_SIG_MEASURE_ERROR]);
				memset((char *)&myPs->rcvPacket, 0, sizeof(S_FADM_RCV_PACKET));
				memset((char *)&myPs->rcvCmd, 0, sizeof(S_FADM_RCV_COMMAND));
				myPs->signal[FADM_SIG_MEASURE_ERROR] = 0;
				myPs->signal[FADM_SIG_MEASURE] = P28;
			}
			break;
		case P39:
			myPs->signal[FADM_SIG_MEASURE_ERROR] += 1;
			if(myPs->signal[FADM_SIG_MEASURE_ERROR] >= 3) {
				userlog(DEBUG_LOG, psName, "measure error : %d, %d\n",
					(int)myPs->signal[FADM_SIG_MEASURE],
					(int)myPs->signal[FADM_SIG_MEASURE_ERROR]);
				memset((char *)&myPs->rcvPacket, 0, sizeof(S_FADM_RCV_PACKET));
				memset((char *)&myPs->rcvCmd, 0, sizeof(S_FADM_RCV_COMMAND));
				myPs->signal[FADM_SIG_MEASURE_ERROR] = 0;
				myPs->signal[FADM_SIG_MEASURE] = P29;
			}
			break;
		case P40:
			myPs->signal[FADM_SIG_MEASURE_ERROR] += 1;
			if(myPs->signal[FADM_SIG_MEASURE_ERROR] >= 3) {
				userlog(DEBUG_LOG, psName, "measure error : %d, %d\n",
					(int)myPs->signal[FADM_SIG_MEASURE],
					(int)myPs->signal[FADM_SIG_MEASURE_ERROR]);
				memset((char *)&myPs->rcvPacket, 0, sizeof(S_FADM_RCV_PACKET));
				memset((char *)&myPs->rcvCmd, 0, sizeof(S_FADM_RCV_COMMAND));
				myPs->signal[FADM_SIG_MEASURE_ERROR] = 0;
				myPs->signal[FADM_SIG_MEASURE] = P30;
			}
			break;
		case P41:
			myPs->signal[FADM_SIG_MEASURE] = P51;
			ch = (int)myPs->signal[FADM_SIG_READ_CH];
			val = ch % 2;
			if(val == 0) {
				val = 0;
			} else {
				val = 10;
			}
			ch = ch / 2;
			send_cmd_request(ch, val);
			break;
		case P42:
			myPs->signal[FADM_SIG_MEASURE] = P52;
			ch = (int)myPs->signal[FADM_SIG_READ_CH];
			val = ch % 2;
			if(val == 0) {
				val = 1;
			} else {
				val = 11;
			}
			ch = ch / 2;
			send_cmd_request(ch, val);
			break;
		case P43:
			myPs->signal[FADM_SIG_MEASURE] = P53;
			ch = (int)myPs->signal[FADM_SIG_READ_CH];
			val = ch % 2;
			if(val == 0) {
				val = 2;
			} else {
				val = 12;
			}
			ch = ch / 2;
			send_cmd_request(ch, val);
			break;
		case P44:
			myPs->signal[FADM_SIG_MEASURE] = P54;
			ch = (int)myPs->signal[FADM_SIG_READ_CH];
			val = ch % 2;
			if(val == 0) {
				val = 3;
			} else {
				val = 13;
			}
			ch = ch / 2;
			send_cmd_request(ch, val);
			break;
		case P45:
			myPs->signal[FADM_SIG_MEASURE] = P55;
			ch = (int)myPs->signal[FADM_SIG_READ_CH];
			val = ch % 2;
			if(val == 0) {
				val = 4;
			} else {
				val = 14;
			}
			ch = ch / 2;
			send_cmd_request(ch, val);
			break;
		case P46:
			myPs->signal[FADM_SIG_MEASURE] = P56;
			ch = (int)myPs->signal[FADM_SIG_READ_CH];
			val = ch % 2;
			if(val == 0) {
				val = 5;
			} else {
				val = 15;
			}
			ch = ch / 2;
			send_cmd_request(ch, val);
			break;
		case P51:
			myPs->signal[FADM_SIG_MEASURE_ERROR] += 1;
			if(myPs->signal[FADM_SIG_MEASURE_ERROR] >= 3) {
				userlog(DEBUG_LOG, psName, "measure error : %d, %d\n",
					(int)myPs->signal[FADM_SIG_MEASURE],
					(int)myPs->signal[FADM_SIG_MEASURE_ERROR]);
				memset((char *)&myPs->rcvPacket, 0, sizeof(S_FADM_RCV_PACKET));
				memset((char *)&myPs->rcvCmd, 0, sizeof(S_FADM_RCV_COMMAND));
				myPs->signal[FADM_SIG_MEASURE_ERROR] = 0;
				myPs->signal[FADM_SIG_MEASURE] = P41;
			}
			break;
		case P52:
			myPs->signal[FADM_SIG_MEASURE_ERROR] += 1;
			if(myPs->signal[FADM_SIG_MEASURE_ERROR] >= 3) {
				userlog(DEBUG_LOG, psName, "measure error : %d, %d\n",
					(int)myPs->signal[FADM_SIG_MEASURE],
					(int)myPs->signal[FADM_SIG_MEASURE_ERROR]);
				memset((char *)&myPs->rcvPacket, 0, sizeof(S_FADM_RCV_PACKET));
				memset((char *)&myPs->rcvCmd, 0, sizeof(S_FADM_RCV_COMMAND));
				myPs->signal[FADM_SIG_MEASURE_ERROR] = 0;
				myPs->signal[FADM_SIG_MEASURE] = P42;
			}
			break;
		case P53:
			myPs->signal[FADM_SIG_MEASURE_ERROR] += 1;
			if(myPs->signal[FADM_SIG_MEASURE_ERROR] >= 3) {
				userlog(DEBUG_LOG, psName, "measure error : %d, %d\n",
					(int)myPs->signal[FADM_SIG_MEASURE],
					(int)myPs->signal[FADM_SIG_MEASURE_ERROR]);
				memset((char *)&myPs->rcvPacket, 0, sizeof(S_FADM_RCV_PACKET));
				memset((char *)&myPs->rcvCmd, 0, sizeof(S_FADM_RCV_COMMAND));
				myPs->signal[FADM_SIG_MEASURE_ERROR] = 0;
				myPs->signal[FADM_SIG_MEASURE] = P43;
			}
			break;
		case P54:
			myPs->signal[FADM_SIG_MEASURE_ERROR] += 1;
			if(myPs->signal[FADM_SIG_MEASURE_ERROR] >= 3) {
				userlog(DEBUG_LOG, psName, "measure error : %d, %d\n",
					(int)myPs->signal[FADM_SIG_MEASURE],
					(int)myPs->signal[FADM_SIG_MEASURE_ERROR]);
				memset((char *)&myPs->rcvPacket, 0, sizeof(S_FADM_RCV_PACKET));
				memset((char *)&myPs->rcvCmd, 0, sizeof(S_FADM_RCV_COMMAND));
				myPs->signal[FADM_SIG_MEASURE_ERROR] = 0;
				myPs->signal[FADM_SIG_MEASURE] = P44;
			}
			break;
		case P55:
			myPs->signal[FADM_SIG_MEASURE_ERROR] += 1;
			if(myPs->signal[FADM_SIG_MEASURE_ERROR] >= 3) {
				userlog(DEBUG_LOG, psName, "measure error : %d, %d\n",
					(int)myPs->signal[FADM_SIG_MEASURE],
					(int)myPs->signal[FADM_SIG_MEASURE_ERROR]);
				memset((char *)&myPs->rcvPacket, 0, sizeof(S_FADM_RCV_PACKET));
				memset((char *)&myPs->rcvCmd, 0, sizeof(S_FADM_RCV_COMMAND));
				myPs->signal[FADM_SIG_MEASURE_ERROR] = 0;
				myPs->signal[FADM_SIG_MEASURE] = P45;
			}
			break;
		case P56:
			myPs->signal[FADM_SIG_MEASURE_ERROR] += 1;
			if(myPs->signal[FADM_SIG_MEASURE_ERROR] >= 3) {
				userlog(DEBUG_LOG, psName, "measure error : %d, %d\n",
					(int)myPs->signal[FADM_SIG_MEASURE],
					(int)myPs->signal[FADM_SIG_MEASURE_ERROR]);
				memset((char *)&myPs->rcvPacket, 0, sizeof(S_FADM_RCV_PACKET));
				memset((char *)&myPs->rcvCmd, 0, sizeof(S_FADM_RCV_COMMAND));
				myPs->signal[FADM_SIG_MEASURE_ERROR] = 0;
				myPs->signal[FADM_SIG_MEASURE] = P46;
			}
			break;
		case P60:
			ch = (int)myPs->signal[FADM_SIG_READ_CH];
			CalPulseAD_Reference(ch);
			myPs->signal[FADM_SIG_MEASURE] = P61;
			break;
		case P61:
			ch = (int)myPs->signal[FADM_SIG_READ_CH];
			CalPulseAD_Value(ch);
			myPs->signal[FADM_SIG_MEASURE] = P62;
			break;
		case P62:
			ch = (int)myPs->signal[FADM_SIG_READ_CH];
			if(myPs->misc.use_totalCycle == 0
				&& myPs->misc.use_stepNo == 0) {
			} else {
				send_fadm_pulse_msg(ch);
			}
			myPs->comm_buffer.use_flag = 0;
			myPs->signal[FADM_SIG_MEASURE] = P0;
			break;
		default:	break;
	}
}

void Close_Process(void)
{
	if(myPs->config.ttyS_fd > 0) {
		if(closetty(myPs->config.ttyS_fd) < 0) {
			userlog(DEBUG_LOG, psName, "ttyS %d close error\n",
				myPs->config.ttyS_fd);
		}
	}
	
	myData->AppControl.signal[APP_SIG_FADM_PROCESS] = P3;	

	Close_SystemMemory();
}
