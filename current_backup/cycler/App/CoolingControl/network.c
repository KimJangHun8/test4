#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
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
extern volatile S_COOLING_CONTROL *myPs;
extern char psName[PROCESS_NAME_SIZE];

int InitNetwork(void)
{
	int i = 0;
//	EquipmentInfoPrint();

	if(myPs->config.network_socket > 0)
		close(myPs->config.network_socket);
	
    myPs->config.network_socket
		= SetClientSock(myPs->config.networkPort,
		(char *)&myPs->config.ipAddr);
    if(myPs->config.network_socket < 0) {
		close(myPs->config.network_socket);
		myData->test_val_l[0]++;
	    userlog(DEBUG_LOG, psName, "Can not initialize network : %d\n",
			myPs->config.network_socket);
		switch(myPs->config.network_socket) {
			case -113:
				printf("%c[1;5;%dm", 27, 31); //BLINK RED
				printf("CoolingControl_Config IP setting check please!!!%c[0m\n",27);
				break;
			case -111:
				printf("%c[1;5;%dm", 27, 31); //BLINK RED
				printf("Lan To 485 Convert state check please!!!%c[0m\n",27);
				break;
			default: break;
		}
		for(i = 0 ; i < MAX_COOLING_COUNT ; i++){
			myPs->data[i].error_state = 1;
		}
		sleep(2);
		return -1;
    }

	myPs->signal[COOLING_SIG_NET_CONNECTED] = P1;
	myData->cooling_connected = P1;
	myPs->netTimer = myData->mData.misc.timer_1sec;
	
	myPs->misc.maxGroup = myPs->config.installed_cooling;
	if(myPs->misc.maxGroup == myPs->config.installed_cooling){
		userlog(DEBUG_LOG, psName, "Cooling Count [%d]\n",myPs->misc.maxGroup);
	}

	userlog(DEBUG_LOG, psName, "command socket connected : %d\n",
		myPs->config.network_socket);
	//hun_210625
	userlog(DEBUG_LOG, psName, "Cooling Control connection success!!!\n");
	
	myPs->misc.CoolingNo = 1;
	myPs->signal[COOLING_SIG_PROCESS] = P1;
	
    return 0;
}

int NetworkPacket_Receive(void)
{
	int rcv_size, read_size, i, start, index;
	char maxPacketBuf[MAX_COOLING_PACKET_LENGTH];

	memset(maxPacketBuf, 0x00, MAX_COOLING_PACKET_LENGTH);
		
	if(ioctl(myPs->config.network_socket, FIONREAD, &rcv_size) < 0) {
		userlog(DEBUG_LOG, psName, "packet receive ioctl error\n");
		send_msg(COOLING_TO_APP, MSG_COOLING_APP_PROCESS_KILL, 0, 0); //211007
		close(myPs->config.network_socket);
		return -1;
	}

	if(rcv_size > MAX_COOLING_PACKET_LENGTH) {
		userlog(DEBUG_LOG, psName, "max packet size over\n");
		read_size = readn(myPs->config.network_socket, maxPacketBuf,
					MAX_COOLING_PACKET_LENGTH);
		if(read_size != MAX_COOLING_PACKET_LENGTH)
			userlog(DEBUG_LOG, psName, "packet readn size error1\n");
		send_msg(COOLING_TO_APP, MSG_COOLING_APP_PROCESS_KILL, 0, 0); //211007
		close(myPs->config.network_socket);
		return -2;
	} else if(rcv_size > (MAX_COOLING_PACKET_LENGTH
		- myPs->rcvPacket.usedBufSize)) {
		userlog(DEBUG_LOG, psName, "packet buffer overflow\n");
		read_size = readn(myPs->config.network_socket, maxPacketBuf, rcv_size);
		if(read_size != rcv_size)
			userlog(DEBUG_LOG, psName, "packet readn size error2\n");
		send_msg(COOLING_TO_APP, MSG_COOLING_APP_PROCESS_KILL, 0, 0); //211007
		close(myPs->config.network_socket);
		return -3;
	} else if(rcv_size <= 0) {
		userlog(DEBUG_LOG, psName, "packet sock_rcv error %d\n", rcv_size);
		send_msg(COOLING_TO_APP, MSG_COOLING_APP_PROCESS_KILL, 0, 0); //211007
		close(myPs->config.network_socket);
		return -4;
	} else {
		read_size = readn(myPs->config.network_socket, maxPacketBuf, rcv_size);
		if(read_size != rcv_size) {
			userlog(DEBUG_LOG, psName,
				"packet readn size error3 : %d, %d\n", read_size, rcv_size);
			send_msg(COOLING_TO_APP, MSG_COOLING_APP_PROCESS_KILL, 0, 0);//211007
			close(myPs->config.network_socket);
			return -5;
		}
	}
	
	//userlog(DEBUG_LOG, psName, "recvCmd %s\n", maxPacketBuf); //kjgd

	i = myPs->rcvPacket.rcvCount;
	myPs->rcvPacket.rcvCount++;
	if(myPs->rcvPacket.rcvCount > (MAX_COOLING_PACKET_COUNT-1))
		myPs->rcvPacket.rcvCount = 0;
	
	if(i == 0) index = MAX_COOLING_PACKET_COUNT - 1;
	else index = i - 1;
	start = myPs->rcvPacket.rcvStartPoint[index]
		+ myPs->rcvPacket.rcvSize[index];
	if(start >= MAX_COOLING_PACKET_LENGTH) {
		myPs->rcvPacket.rcvStartPoint[i] = abs(start - MAX_COOLING_PACKET_LENGTH);
	} else {
		myPs->rcvPacket.rcvStartPoint[i] = start;
	}

	myPs->rcvPacket.rcvSize[i] = read_size;
	myPs->rcvPacket.usedBufSize += read_size;
	
	start = myPs->rcvPacket.rcvStartPoint[i];
	if((start + read_size) > MAX_COOLING_PACKET_LENGTH) {
		index = MAX_COOLING_PACKET_LENGTH - start;
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
					MAX_COOLING_PACKET_LENGTH);
				rtn = -1;
				return rtn;
			}
		} else {
			myPs->rcvCmd.cmdFail = 0;
		}
	}
	
	return rtn;
}

void NetworkPacket_Parsing(void)
{
	//char debug[MAX_CHAMBER_PACKET_LENGTH]; //kjgd
	int i, j, k, cmdBuf_index, start_point;
	if(myPs->rcvPacket.rcvCount
		== myPs->rcvPacket.parseCount) return;
	i = myPs->rcvPacket.parseCount;
	myPs->rcvPacket.parseCount++;
	if(myPs->rcvPacket.parseCount > (MAX_COOLING_PACKET_COUNT-1))
		myPs->rcvPacket.parseCount = 0;
	
	cmdBuf_index = myPs->rcvCmd.cmdBufSize;
	myPs->rcvCmd.cmdBufSize
		+= myPs->rcvPacket.rcvSize[i];
	
	start_point = myPs->rcvPacket.parseStartPoint[i]; 
	j = start_point + myPs->rcvPacket.rcvSize[i];
	if(j <= MAX_COOLING_PACKET_LENGTH) {
		memcpy((char *)&myPs->rcvCmd.cmdBuf[cmdBuf_index],
			(char *)&myPs->rcvPacket.rcvPacketBuf[start_point],
			myPs->rcvPacket.rcvSize[i]);
	} else {
		k = MAX_COOLING_PACKET_LENGTH - start_point;
		memcpy((char *)&myPs->rcvCmd.cmdBuf[cmdBuf_index],
			(char *)&myPs->rcvPacket.rcvPacketBuf[start_point], k);
		cmdBuf_index += k;
		start_point = 0;
		k = j - MAX_COOLING_PACKET_LENGTH;
		memcpy((char *)&myPs->rcvCmd.cmdBuf[cmdBuf_index],
			(char *)&myPs->rcvPacket.rcvPacketBuf[start_point], k);
	}
		
	start_point = myPs->rcvPacket.parseStartPoint[i]
		+ myPs->rcvPacket.rcvSize[i];
	if(start_point >= MAX_COOLING_PACKET_LENGTH) {
		j = i + 1;
		if(j >= MAX_COOLING_PACKET_COUNT) j = 0;
		myPs->rcvPacket.parseStartPoint[j]
			= abs(start_point - MAX_COOLING_PACKET_LENGTH);
	} else {
		j = i + 1;
		if(j >= MAX_COOLING_PACKET_COUNT) j = 0;
		myPs->rcvPacket.parseStartPoint[j] = start_point;
	}
		
	myPs->rcvPacket.usedBufSize -= myPs->rcvPacket.rcvSize[i];
}

int NetworkCommand_Receive(void)
{
	int cmd_size, cmdBuf_index,i;
//	unsigned char tmp;

	S_COOLING_CMD_HEADER header;

	if(myPs->rcvCmd.cmdBufSize < sizeof(S_COOLING_CMD_HEADER)) return -1;
	memcpy((char *)&header, (char *)&myPs->rcvCmd.cmdBuf[0],
		sizeof(S_COOLING_CMD_HEADER));
	
	cmd_size = 0;
	
	if(myPs->rcvCmd.cmdBuf[0] == 0x02){	//STX check
		for(i = 0; i < myPs->rcvCmd.cmdBufSize; i++){
			if(myPs->rcvCmd.cmdBuf[i] == 0x0A){	// LF Check
				cmd_size = i+1;
				break;
			}
		}
		if(cmd_size == 0) return -1;
	}else {
		for(i = 0; i < myPs->rcvCmd.cmdBufSize; i++){
			if(myPs->rcvCmd.cmdBuf[i] == 0x0A){
				cmd_size = i+1;
				break;
			}
		}
		if(cmd_size == 0) return -1;
		
		cmdBuf_index = cmd_size;
		myPs->rcvCmd.cmdBufSize -= cmd_size;
		cmd_size = myPs->rcvCmd.cmdBufSize;
		memset((char *)&myPs->rcvCmd.tmpBuf, 0, MAX_COOLING_PACKET_LENGTH);
		memcpy((char *)&myPs->rcvCmd.tmpBuf,
			(char *)&myPs->rcvCmd.cmdBuf[cmdBuf_index], cmd_size);
		return -1;
	}

	if(myPs->rcvCmd.cmdBufSize < cmd_size) return -3;
			
	memset((char *)&myPs->rcvCmd.cmd[0], 0x00, MAX_COOLING_PACKET_LENGTH);
	memcpy((char *)&myPs->rcvCmd.cmd[0],
		(char *)&myPs->rcvCmd.cmdBuf[0], cmd_size);
	myPs->rcvCmd.cmdSize = cmd_size;
	
	cmdBuf_index = cmd_size;
	myPs->rcvCmd.cmdBufSize -= cmd_size;
	cmd_size = myPs->rcvCmd.cmdBufSize;
	memset((char *)&myPs->rcvCmd.tmpBuf[0], 0x00, MAX_COOLING_PACKET_LENGTH);
	memcpy((char *)&myPs->rcvCmd.tmpBuf[0],
		(char *)&myPs->rcvCmd.cmdBuf[cmdBuf_index], cmd_size);
	memset((char *)&myPs->rcvCmd.cmdBuf[0], 0x00, MAX_COOLING_PACKET_LENGTH);
	memcpy((char *)&myPs->rcvCmd.cmdBuf[0],
		(char *)&myPs->rcvCmd.tmpBuf[0], cmd_size);

	return 0;
}

int NetworkCommand_Parsing(void)
{
	unsigned char tmp;
	int	rtn, i;
	S_COOLING_CMD_HEADER	header;

	memset((char *)&header, 0x00, sizeof(S_COOLING_CMD_HEADER));
	memcpy((char *)&header, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_COOLING_CMD_HEADER));
	if(myPs->config.CmdRcvLog == P1) {
		if(myPs->config.CommCheckLog == P1) {
			userlog(DEBUG_LOG, psName, "recvCmd %s:end\n", myPs->rcvCmd.cmd);
		} else {
			userlog(DEBUG_LOG, psName, "recvCmd %s:end\n", myPs->rcvCmd.cmd);
		}
	}
	
	if(myPs->config.CmdRcvLog_Hex == P1) {
		if(myPs->config.CommCheckLog == P1) {
			userlog(DEBUG_LOG, psName, "recvCmd_Hex");
			for(i=0; i < myPs->rcvCmd.cmdSize; i++) {
				tmp = myPs->rcvCmd.cmd[i];
				userlog2(DEBUG_LOG, psName, " %02x", tmp);
			}
			userlog2(DEBUG_LOG, psName, ":end\n");
		} else {
			userlog(DEBUG_LOG, psName, "recvCmd_Hex");
			for(i=0; i < myPs->rcvCmd.cmdSize; i++) {
				tmp = myPs->rcvCmd.cmd[i];
				userlog2(DEBUG_LOG, psName, " %02x", tmp);
			}
			userlog2(DEBUG_LOG, psName, ":end\n");
		}
	}
	rtn = CmdHeader_Check((char *)&header);
	if(rtn < 0) return -1;
	
	rtn = CheckSum_Check_T54();
	if(rtn < 0) return -1;
		
	switch(myPs->misc.state){
		case 1:
			rtn = rcv_cmd_DRS(myPs->misc.cmd_id);
			break;
		case 2:
			rtn = rcv_cmd_DRR(myPs->misc.cmd_id);
			break;
		case 3:
			rtn = rcv_cmd_DWS(myPs->misc.cmd_id);
			break;						
		case 4:
			rtn = rcv_cmd_DWR(myPs->misc.cmd_id);
			break;
		case 5:
			rtn = rcv_cmd_WHO(myPs->misc.cmd_id);
			break;
		case 6:
			break;
		default:
			userlog(DEBUG_LOG, psName, "Can't Find Cmd[%x]\n", header.addr);
			rtn = -10;
			break;												
	}

	switch(rtn){
		case 2:
			myPs->data[myPs->misc.cmd_id].send_pv_ng_cnt = 0;
			break;
		case -2:
			if(myPs->data[myPs->misc.cmd_id].send_pv_ng_cnt++ >= 5){
				userlog(DEBUG_LOG, psName, "pv rcv ng\n");
				myPs->data[myPs->misc.cmd_id].send_pv_ng_cnt = 5;
				myData->AppControl.signal[myPs->misc.psSignal] = P2;
			}
			break;
		case 3:
			myPs->data[myPs->misc.cmd_id].send_sv_ng_cnt = 0;
			break;
		case -3:
			if(myPs->data[myPs->misc.cmd_id].send_sv_ng_cnt++ >= 5){
				userlog(DEBUG_LOG, psName, "sv rcv ng\n");
				myPs->data[myPs->misc.cmd_id].send_sv_ng_cnt = 5;
				myData->AppControl.signal[myPs->misc.psSignal] = P2;
			}
			break;
		default:
		break;	
	}
	return rtn;
}

int CmdHeader_Check(char *rcvHeader)
{
	S_COOLING_CMD_HEADER	header;
	
	memset((char *)&header, 0x00, sizeof(S_COOLING_CMD_HEADER));
	memcpy((char *)&header, (char *)rcvHeader, sizeof(S_COOLING_CMD_HEADER));

	if(header.stx != 0x02){
		userlog(DEBUG_LOG, psName, "RcvCmd stx error %x\n",
			header.stx);
		return -1;
	}

	if(header.addr[0] == 0x30 && header.addr[1] == 0x31){
		myPs->misc.cmd_id = 1;
	}else if(header.addr[0] == 0x30 && header.addr[1] == 0x32){
		myPs->misc.cmd_id = 2;
	}else if(header.addr[0] == 0x30 && header.addr[1] == 0x33){
		myPs->misc.cmd_id = 3;
	}else if(header.addr[0] == 0x30 && header.addr[1] == 0x34){
		myPs->misc.cmd_id = 4;
	}else if(header.addr[0] == 0x30 && header.addr[1] == 0x35){
		myPs->misc.cmd_id = 5;
	}else if(header.addr[0] == 0x30 && header.addr[1] == 0x36){
		myPs->misc.cmd_id = 6;
	}else if(header.addr[0] == 0x30 && header.addr[1] == 0x37){
		myPs->misc.cmd_id = 7;
	}else if(header.addr[0] == 0x30 && header.addr[1] == 0x38){
		myPs->misc.cmd_id = 8;
	}else if(header.addr[0] == 0x30 && header.addr[1] == 0x39){
		myPs->misc.cmd_id = 9;
	}else if(header.addr[0] == 0x31 && header.addr[1] == 0x30){					
		myPs->misc.cmd_id = 10;
	}else{
		userlog(DEBUG_LOG, psName, "RcvCmd addr error [%x][%x]\n",
			header.addr[0],header.addr[1]);
		return -1;	
	}	

	if(header.cmd[0] == 0x44 && header.cmd[1] == 0x52
	  && header.cmd[2] == 0x53 && header.cmd[3] == 0x2C){ //DRS,
		myPs->misc.state = 1;
	}else if(header.cmd[0] == 0x44 && header.cmd[1] == 0x52
	  && header.cmd[2] == 0x52 && header.cmd[3] == 0x2C){ //DRR,
		myPs->misc.state = 2;
	}else if(header.cmd[0] == 0x44 && header.cmd[1] == 0x57
	  && header.cmd[2] == 0x53 && header.cmd[3] == 0x2C){ //DWS,
		myPs->misc.state = 3;	  
	}else if(header.cmd[0] == 0x44 && header.cmd[1] == 0x57
	  && header.cmd[2] == 0x52 && header.cmd[3] == 0x2C){ //DWR,
		myPs->misc.state = 4;	  
	}else if(header.cmd[0] == 0x57 && header.cmd[1] == 0x48
	  && header.cmd[2] == 0x4F && header.cmd[3] == 0x2C){ //WHO,
		myPs->misc.state = 5;	  
	}else if(header.cmd[0] == 0x4E && header.cmd[1] == 0x47){ //NG
		myPs->misc.state = 6;
	}else{
		myPs->misc.state = 0;
		userlog(DEBUG_LOG, psName, "RcvCmd cmd error [%x][%x][%x][%x]\n",
			header.cmd[0],header.cmd[1],header.cmd[2],header.cmd[3]);
		return -1;
	}

//220315 jws 삭제 확인 필요 
/*	
	userlog(DEBUG_LOG, psName, "rcvCmd : %d \n", myPs->rcvCmd.cmdSize);
	userlog(DEBUG_LOG, psName, "length : %d \n", length);
	
	if(length != myPs->rcvCmd.cmdSize) {
		userlog(DEBUG_LOG, psName, "RcvCmd cmd_id [%d] size error (%d : %d) phase (%d)\n",
			myPs->misc.cmd_id, length, myPs->rcvCmd.cmdSize, myPs->data[myPs->misc.cmd_id-1].phase);
		userlog(DEBUG_LOG, psName, "recvCmd_Hex");
			for(i=0; i < myPs->rcvCmd.cmdSize; i++) {
				tmp = myPs->rcvCmd.cmd[i];
				userlog2(DEBUG_LOG, psName, " %02x", tmp);
			}
			userlog2(DEBUG_LOG, psName, ":end\n");
			
		return -2;
	}
*/
	return 0;
}

int CheckSum_Check_T54(void)
{
	char tmp, make_sum1, make_sum2, recv_sum1, recv_sum2;
	int i, sum=0;

	for(i=1; i < (myPs->rcvCmd.cmdSize - 4); i++) {
		sum += (unsigned char)(myPs->rcvCmd.cmd[i]);
	}

	tmp = (char)((sum % 0x100) / 0x10);
	if(tmp <= 9) tmp += 48;
	else tmp += 55;
	make_sum1 = tmp;

	tmp = (char)(sum % 0x10);
	if(tmp <= 9) tmp += 48;
	else tmp += 55;
	make_sum2 = tmp;

	recv_sum1 = (char)myPs->rcvCmd.cmd[myPs->rcvCmd.cmdSize - 4];
	recv_sum2 = (char)myPs->rcvCmd.cmd[myPs->rcvCmd.cmdSize - 3];
	
	if((recv_sum1 != make_sum1) || (recv_sum2 != make_sum2)) {
		userlog(DEBUG_LOG, psName,
			"RcvCmd CheckSum error recv(%02x:%02x) make(%02x:%02x)\n",
			recv_sum1, recv_sum2, make_sum1, make_sum2);
		return -1;
	}
	return 0;
}

int rcv_cmd_DRS(int id)
{
	int i, tmp, index, val;
	
	index = 10;
	val = 0;
	if(myPs->rcvCmd.cmd[7] == 0x4F && myPs->rcvCmd.cmd[8] == 0x4B){ //OK
		for(i = 0; i <= 3; i++){
			tmp = myPs->rcvCmd.cmd[index + i];
			if(tmp <= 57) tmp -= 48;
			else tmp -= 55;
			if(i == 0)	val += (tmp * 0x1000);
			else if(i == 1)	val += (tmp * 0x100);
			else if(i == 2) val += (tmp * 0x10);
			else	val += tmp;
		}
		myPs->data[id].read_pv = val;
		userlog(DEBUG_LOG, psName, "pv : %d \n //unit 0.1",myPs->data[id].read_pv);
		index = 15;
		val = 0;
		for(i = 0; i <= 3; i++){
			tmp = myPs->rcvCmd.cmd[index + i];
			if(tmp <= 57) tmp -= 48;
			else tmp -= 55;
			if(i == 0)	val += (tmp * 0x1000);
			else if(i == 1)	val += (tmp * 0x100);
			else if(i == 2) val += (tmp * 0x10);
			else	val += tmp;
		}
		myPs->data[id].read_sv = val;
		userlog(DEBUG_LOG, psName, "sv : %d //unit 0.1 \n",myPs->data[id].read_sv);
	}else{	//NG
		userlog(DEBUG_LOG, psName, "rcv Data : NG\n");	
		return -2;
	}	
	return 2;
}

int rcv_cmd_DRR(int id)
{
	return 0;
}

int rcv_cmd_DWS(int id)
{
	return 0;
}

int rcv_cmd_DWR(int id)
{
	int rtn = 3;
	if(myPs->rcvCmd.cmd[7] == 0x4F && myPs->rcvCmd.cmd[8] == 0x4B){ //OK
	}else{	//NG
		rtn = -3;	
		userlog(DEBUG_LOG, psName, "send Data : NG\n");	
	}
	return rtn;
}

int rcv_cmd_WHO(int id)
{
	return 0;
}

void send_cmd_request(int num)
{
	char cmd[40];
	int rtn, data_size, cmd_size;
	memset((char *)&cmd, 0, sizeof cmd);
	
	data_size = 14;
	*cmd = 0x02;	//STX

	if(num >= 1 && num < 10){
		*(cmd+1) = 0x30;		//addr 0
		*(cmd+2) = 0x30 + num;	//addr 1
	}else if(num >= 10){
		*(cmd+1) = 0x30 + (num / 10);	//addr 0
		*(cmd+2) = 0x30 + (num % 10);	//addr 1
	}

	*(cmd+3) = 0x44;	//D
	*(cmd+4) = 0x52;	//R
	*(cmd+5) = 0x53;	//S
	*(cmd+6) = 0x2C;	//,

	*(cmd+7) = 0x30;	//0
	*(cmd+8) = 0x32;	//1
	*(cmd+9) = 0x2C;	//,
	
	*(cmd+10) = 0x30;	//0
	*(cmd+11) = 0x30;	//0
	*(cmd+12) = 0x30;	//0
	*(cmd+13) = 0x31;	//1

	cmd_size = make_check_sum((char *)&cmd, data_size);

	rtn = send_command((char *)&cmd, cmd_size, 0);
	if(rtn < 0){
		printf("send cmd error \n");
	}else{
	//	myPs->data[num-1].send_flag = 0;
	}

}

void send_cmd_read_value(int num)
{
	char cmd[40];
	int rtn, data_size, cmd_size;
	memset((char *)&cmd, 0, sizeof cmd);
	
	data_size = 14;
	*cmd = 0x02;	//STX

	if(num >= 1 && num < 10){
		*(cmd+1) = 0x30;		//addr 0
		*(cmd+2) = 0x30 + num;	//addr 1
	}else if(num >= 10){
		*(cmd+1) = 0x30 + (num / 10);	//addr 0
		*(cmd+2) = 0x30 + (num % 10);	//addr 1
	}

	*(cmd+3) = 0x44;	//D
	*(cmd+4) = 0x52;	//R
	*(cmd+5) = 0x53;	//S
	*(cmd+6) = 0x2C;	//,

	*(cmd+7) = 0x30;	//0
	*(cmd+8) = 0x32;	//2
	*(cmd+9) = 0x2C;	//,
	
	*(cmd+10) = 0x30;	//0
	*(cmd+11) = 0x30;	//0
	*(cmd+12) = 0x30;	//0
	*(cmd+13) = 0x31;	//1

	cmd_size = make_check_sum((char *)&cmd, data_size);

	rtn = send_command((char *)&cmd, cmd_size, 0);
	if(rtn < 0){
		printf("send cmd error \n");
	}else{
	//	myPs->data[num-1].send_flag = 0;
	}

}

void send_cmd_set_value(int id)
{
	char cmd[40];
	int rtn, data_size, cmd_size, mod, index, tmp;
	double send_data;
	memset((char *)&cmd, 0, sizeof cmd);
	
	data_size = 19;
	*cmd = 0x02;	//STX

	if(id >= 1 && id < 10){
		*(cmd+1) = 0x30;		//addr 0
		*(cmd+2) = 0x30 + id;	//addr 1
	}else if(id >= 10){
		*(cmd+1) = 0x30 + (id / 10);	//addr 0
		*(cmd+2) = 0x30 + (id % 10);	//addr 1
	}

	*(cmd+3) = 0x44;	//D
	*(cmd+4) = 0x57;	//W
	*(cmd+5) = 0x52;	//R
	*(cmd+6) = 0x2C;	//,

	*(cmd+7) = 0x30;	//0
	*(cmd+8) = 0x31;	//1
	*(cmd+9) = 0x2C;	//,
	
	*(cmd+10) = 0x30;	//0
	*(cmd+11) = 0x33;	//3
	*(cmd+12) = 0x30;	//0
	*(cmd+13) = 0x31;	//1
	*(cmd+14) = 0x2C;	//,
	
	send_data = myPs->data[id].command_sv;
	tmp = send_data * 10;
	index = 18;
	mod = 0;	
	while(index >= 15){
		mod = (tmp % 16);
		if(mod < 10){
			*(cmd+index) = 48 + mod;
		}else{
			*(cmd+index) = 65 + (mod - 10);
		}
		tmp = tmp / 16;
		index--;
	}
	//input value unit 0.1
	//*(cmd+15) = 0x30;	//
//	*(cmd+16) = 0x33;	//
//	*(cmd+17) = 0x32;	//
//	*(cmd+18) = 0x31;	//

	cmd_size = make_check_sum((char *)&cmd, data_size);

	rtn = send_command((char *)&cmd, cmd_size, 0);
	myPs->data[id].send_sv_flag = P1;
	if(rtn < 0){
		printf("send cmd error \n");
	}else{
	//	myPs->data[num-1].send_flag = 0;
	}

}

/*
int rcv_cmd_RSD(int id)
{
	char tmp;
	int val, index, i;

	switch(myPs->data[id-1].phase){
		case 1:	//NPV, NSP
			val = 0;
			index = 10;
			for(i=0; i < 4; i++) {
				tmp = (char)myPs->rcvCmd.cmd[index + i];
				if(tmp <= 57) tmp -= 48;
				else tmp -= 55;
				if(i == 0) val += (tmp * 0x1000);
				else if(i == 1) val += (tmp * 0x100);
				else if(i == 2) val += (tmp * 0x10);
				else val += tmp;
			}
			myPs->data[id-1].TEMP_NPV = (short)val;

			val = 0;
			index += 10;
			for(i=0; i < 4; i++) {
				tmp = (char)myPs->rcvCmd.cmd[index + i];
				if(tmp <= 57) tmp -= 48;
				else tmp -= 55;
				if(i == 0) val += (tmp * 0x1000);
				else if(i == 1) val += (tmp * 0x100);
				else if(i == 2) val += (tmp * 0x10);
				else val += tmp;
			}
			myPs->data[id-1].TEMP_NSP = (short)val;	
			
			
			userlog(DEBUG_LOG, psName, "[%d] TEMP_NPV : %d,  TEMP_NSP : %d \n\n", id, myPs->data[id-1].TEMP_NPV, myPs->data[id-1].TEMP_NSP);
		
			
			myPs->data[id-1].send_flag = 1;
			myPs->data[id-1].phase = 2;
			break;
		case 2:	//NOWSTS -> 0x02 : FIX , 0x04 : PROG
			val = 0;
			index = 10;
			for(i=0; i < 4; i++) {
				tmp = myPs->rcvCmd.cmd[index + i];
				if(tmp <= 57) tmp -= 48;
				else tmp -= 55;
				if(i == 0) val += (tmp * 0x1000);
				else if(i == 1) val += (tmp * 0x100);
				else if(i == 2) val += (tmp * 0x10);
				else val += tmp;
			}
			myPs->data[id-1].NOWSTS = val;
			
			if((myPs->data[id-1].NOWSTS & 0x00000002) != 0) {
				myPs->data[id-1].FIX_MODE = 1;
			} else {
				myPs->data[id-1].FIX_MODE = 0;
			}
			if((myPs->data[id-1].NOWSTS & 0x00000004) != 0) {
				myPs->data[id-1].PROG_MODE = 1;
			} else {
				myPs->data[id-1].PROG_MODE = 0;
			}
			myPs->data[id-1].send_flag = 1;			
			myPs->data[id-1].phase = 3;
			break;
		case 3:	//PROC_TIME_H, PROC_TIME_M, PROC_TIME_S
			val = 0;
			index = 10;
			for(i=0; i < 4; i++) {
				tmp = myPs->rcvCmd.cmd[index + i];
				if(tmp <= 57) tmp -= 48;
				else tmp -= 55;
				if(i == 0) val += (tmp * 0x1000);
				else if(i == 1) val += (tmp * 0x100);
				else if(i == 2) val += (tmp * 0x10);
				else val += tmp;
			}
			myPs->data[id-1].PROC_TIME_H = val;

			val = 0;
			index += 5;
			for(i=0; i < 4; i++) {
				tmp = myPs->rcvCmd.cmd[index + i];
				if(tmp <= 57) tmp -= 48;
				else tmp -= 55;
				if(i == 0) val += (tmp * 0x1000);
				else if(i == 1) val += (tmp * 0x100);
				else if(i == 2) val += (tmp * 0x10);
				else val += tmp;
			}
			myPs->data[id-1].PROC_TIME_M = val;
			
			val = 0;
			index += 5;
			for(i=0; i < 4; i++) {
				tmp = myPs->rcvCmd.cmd[index + i];
				if(tmp <= 57) tmp -= 48;
				else tmp -= 55;
				if(i == 0) val += (tmp * 0x1000);
				else if(i == 1) val += (tmp * 0x100);
				else if(i == 2) val += (tmp * 0x10);
				else val += tmp;
			}
			myPs->data[id-1].PROC_TIME_S = val;
			myPs->data[id-1].phase = 4;
			myPs->data[id-1].send_flag = 1;				
			break;
		case 4: //OTHERSTS -> 0x01 RUN
			val = 0;
			index = 10;
			for(i=0; i < 4; i++){
				tmp = myPs->rcvCmd.cmd[index + i];
				if(tmp <= 57) tmp -= 48;
				else tmp -= 55;
				if(i == 0) val += (tmp * 0x1000);
				else if(i == 1) val += (tmp * 0x100);
				else if(i == 2) val += (tmp * 0x10);
				else val += tmp;
			}
			myPs->data[id-1].OTHERSTS = val;
			if((myPs->data[id-1].OTHERSTS & 0x00000001) != 0){
				myPs->data[id-1].CHAMBER_RUN = 1;
			}else {
				myPs->data[id-1].CHAMBER_RUN = 0;			
			}
			myPs->data[id-1].phase = 1;
			myPs->data[id-1].send_flag = 1;					
			break;
		default:
			break;
	}
	return 0;

}



void send_cmd_request_TEMP_2000_2(int count)
{
	char cmd[40];
	int rtn, data_size, cmd_size;

	memset((char *)&cmd, 0, sizeof cmd);

	data_size = 29;
	*cmd = 0x02; //0x02
	
	if(count >= 1 && count < 10){
		*(cmd+1) = 0x30; //addr 0
		*(cmd+2) = 0x30 + count; //addr 1
	}else if(count >= 10){
		*(cmd+1) = 0x30 + (count / 10); //addr 0
		*(cmd+2) = 0x30 + (count % 10); //addr 1
	}

	*(cmd+3) = 0x52; //R
	*(cmd+4) = 0x52; //R
	*(cmd+5) = 0x44; //D
	*(cmd+6) = 0x2C; //,
	
	*(cmd+7) = 0x30; //0
	*(cmd+8) = 0x34; //4
	*(cmd+9) = 0x2C; //,
	
	*(cmd+10) = 0x30; //0
	*(cmd+11) = 0x30; //0
	*(cmd+12) = 0x30; //0
	*(cmd+13) = 0x31; //1
	*(cmd+14) = 0x2C; //,
	
	*(cmd+15) = 0x30; //0
	*(cmd+16) = 0x30; //0
	*(cmd+17) = 0x30; //0
	*(cmd+18) = 0x33; //3
	*(cmd+19) = 0x2C; //,
	
	*(cmd+20) = 0x30; //0
	*(cmd+21) = 0x30; //0
	*(cmd+22) = 0x32; //2
	*(cmd+23) = 0x34; //4
	*(cmd+24) = 0x2C; //,

	*(cmd+25) = 0x30; //0
	*(cmd+26) = 0x30; //0
	*(cmd+27) = 0x33; //3
	*(cmd+28) = 0x30; //0

	cmd_size = make_check_sum_2((char *)&cmd, data_size);

	rtn = send_command((char *)&cmd, cmd_size, 0);
	if(rtn < 0) {
		printf("temp_2500_2 send cmd error\n");
	}else{
		myPs->data[count-1].send_flag = 0;
	}
}
*/
int make_check_sum(char *cmd, int count)
{
	char check_sum;
	int i, bcc;

	bcc = 0;
	for(i=1; i < count; i++) {
		bcc += *(cmd + i);
	}

	check_sum = (char)((bcc % 0x100) / 0x10);
	if(check_sum <= 9) check_sum += 48;
	else check_sum += 55;
	*(cmd + count) = check_sum;

	check_sum = (char)(bcc % 0x10);
	if(check_sum <= 9) check_sum += 48;
	else check_sum += 55;
	*(cmd + count + 1) = check_sum;

	*(cmd + count + 2) = 0x0D; //0x0D
	*(cmd + count + 3) = 0x0A; //0x0A

	return (count+4);
}




int	send_command(char *cmd, int size, int addr)
{
	char	packet[MAX_COOLING_PACKET_LENGTH];
	unsigned char tmp;
	int i;
	if(size > MAX_COOLING_PACKET_LENGTH) {
		userlog(DEBUG_LOG, psName,
			"CMD SEND FAIL!! TOO LARGE SIZE[%d]\n", size);
		return -1;
	}
	
	memset((char *)&packet[0], 0x00, MAX_COOLING_PACKET_LENGTH);
	memcpy((char *)&packet[0], cmd, size);
		if(myPs->config.CmdSendLog == P1) {
		if(myPs->config.CommCheckLog == P1) {
			userlog(DEBUG_LOG, psName, "sendCmd %s:end\n", packet);
		} 
	}
	
	if(myPs->config.CmdSendLog_Hex == P1) {
		if(myPs->config.CommCheckLog == P1) {
			userlog(DEBUG_LOG, psName, "sendCmd");
			for(i=0; i < size; i++) {
				tmp = *(cmd + i);
				userlog2(DEBUG_LOG, psName, " %02x", tmp);
			}
			userlog2(DEBUG_LOG, psName, ":end\n");
		} 
	}

	return writen(myPs->config.network_socket, packet, size);
}

int Check_NetworkState(void)
{	// communication Check
	int rtn=0;
	
	if(check_cmd_reply_timeout() < 0){
		rtn = -1;
	}
	if(check_network_timeout() < 0) {
		rtn = -1;
	}

	return rtn;
}


int check_cmd_reply_timeout(void)
{
	int i = 0;
	int count = 0;
	int rtn = 0;
	int diff;

	diff = (int)(myData->mData.misc.timer_1sec - myPs->reply.timer);

	if(diff > myPs->config.replyTimeout){
		myPs->reply.timer = myData->mData.misc.timer_1sec;
		for(i = 0 ; i < myPs->misc.maxGroup ; i++){
			if(myPs->data[i].retry_cnt >= myPs->config.retryCount){	
				userlog(DEBUG_LOG, psName,
					"REPLY TIMEOUT: Cooling ID [%d]\n",i+1);
				myPs->data[i].error_state = 1;
				//200829 comunication Error
				myPs->data[i].Status = COOLING_DISCONNECT;
				myPs->data[i].retry_cnt = myPs->config.retryCount;	//hun_210625
				count++;
			}
		}
		if(count >= myPs->misc.maxGroup){
			rtn = -1;
			userlog(DEBUG_LOG, psName, "REPLY TIMEOUT: TOTAL Cooling\n");
		}else if(count < myPs->misc.maxGroup){
			myPs->netTimer = myData->mData.misc.timer_1sec;
			count = 0;
		}
	}

	return rtn;
}

int	check_network_timeout(void)
{
	int diff;
	
	diff = (int)(myData->mData.misc.timer_1sec - myPs->netTimer);
	if(diff > myPs->config.netTimeout){
		userlog(DEBUG_LOG, psName,
		"netTimeout (netTime:%d > setValue:%d)\n", diff, myPs->config.netTimeout);
		myPs->netTimer = myData->mData.misc.timer_1sec;
		return -1;
	}
	return 0;
}

