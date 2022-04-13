#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include "../../INC/datastore.h"
#include "userlog.h"
#include "message.h"
#include "common_utils.h"
#include "network.h"
#include "local_utils.h"

extern volatile S_SYSTEM_DATA *myData;
extern volatile S_DAQ_CLIENT *myPs;
extern char psName[PROCESS_NAME_SIZE];

int Initialize(void)
{
	if(Open_SystemMemory(0) < 0) return -1;
	
	myPs = &(myData->DAQ_Client);
	
	Init_SystemMemory();

	if(Read_DAQ_Client_Config() < 0) return -1;
	if(Read_DAQ_Client_ChArray() < 0) return -2;
	if(Read_Th_Table_Type() < 0) return -3;
	
	myData->AppControl.signal[myPs->misc.psSignal] = P1;
	userlog(DEBUG_LOG,psName,"DAQ_Client Initialize\n");
	
	return 0;
}

void Init_SystemMemory(void)
{
	int i;
	
	memset((char *)&psName, 0x00, PROCESS_NAME_SIZE);
	strcpy(psName, "DAQ_Client");
	myPs->misc.psSignal = APP_SIG_DAQ_CLIENT_PROCESS;

	for(i=0; i < MAX_SIGNAL; i++) {
		myPs->signal[i] = P0;
	}
	userlog(DEBUG_LOG,psName,"DAQ_Client Init_SystemMemory\n");
	memset((char *)&myPs->reply, 0x00, sizeof(S_DAQ_CLIENT_REPLY));
	memset((char *)&myPs->rcvCmd, 0x00, sizeof(S_DAQ_CLIENT_RCV_COMMAND));
	memset((char *)&myPs->rcvPacket, 0x00, sizeof(S_DAQ_CLIENT_RCV_PACKET));
	
	myPs->misc.sended_monitor_data_time
		= myData->mData.misc.timer_1sec;
	myPs->misc.sended_heart_beat_time 
		= myData->mData.misc.timer_1sec;
	myPs->pingTimer 
		= myData->mData.misc.timer_1sec;
	myPs->netTimer 
		= myData->mData.misc.timer_1sec;
	myPs->misc.processPointer = (int)&myData;

}

int	Read_DAQ_Client_Config(void)
{
    int tmp;
	char temp[32], buf[12], fileName[128];
    FILE *fp;

	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/cycler_data/config/parameter/1_LGES_parameter/DAQ_Client_Config");
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "DAQ_Client_Config file read error\n");
		return -1;
	}
    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.groupId = (short int)atoi(buf);
	
	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	memset(temp, 0x00, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	memcpy((char *)&myPs->config.ipAddr[0], (char *)&temp[0],
		sizeof(char)*16);
	
    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
   	tmp = fscanf(fp, "%s", buf);
   	myPs->config.sendPort = atoi(buf);

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.receivePort = atoi(buf);

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
   	tmp = fscanf(fp, "%s", buf);
    myPs->config.networkPort = atoi(buf);

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.replyTimeout = atoi(buf);

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.retryCount = atoi(buf);

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.netTimeout = atoi(buf);

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.pingTimeout = atoi(buf);

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.CmdSendLog = (unsigned char)atoi(buf);

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.CmdRcvLog = (unsigned char)atoi(buf);

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.CmdSendLog_Hex = (unsigned char)atoi(buf);

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.CmdRcvLog_Hex = (unsigned char)atoi(buf);

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.CommCheckLog = (unsigned char)atoi(buf);

    fclose(fp);
	return 0;
}

int	Read_DAQ_Client_ChArray(void)
{
    int tmp, i;
	char temp[32], buf[12], fileName[128];
    FILE *fp;

	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/cycler_data/config/parameter/1_LGES_parameter/DAQ_Client_ChArray2");
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "DAQ_Client_ChArray2 file read error\n");
		return -1;
	}

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);

	for(i=0; i < MAX_CH_PER_MODULE; i++) {
    	tmp = fscanf(fp, "%s", temp); //hardware number
		memset(buf, 0x00, sizeof buf);
    	tmp = fscanf(fp, "%s", buf);
		tmp = atoi(buf) - 1;
    	myPs->config.ChArray2[i] = (unsigned short)tmp; //monitor number
	}

    fclose(fp);
	return 0;
}

int	Read_Th_Table_Type(void)
{
	char temp[32], buf[32], fileName[256];
    int tmp, i;
	float Vref, R1, R2, Rth, R25, Beta;
//	float val;
    FILE *fp;
	
	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/cycler_data/config/parameter/1_LGES_parameter/DAQ_Client_th_table");
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "th_table file read error\n");
		return -1;
	}
/*
	tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	tmp = fscanf(fp, "%s", temp);
*/
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "Bias_type") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
   	tmp = fscanf(fp, "%s", buf);
   	myPs->th_table.bias_type = (unsigned char)atoi(buf);

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "Vref[mV]") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
   	tmp = fscanf(fp, "%s", buf);
	Vref = atof(buf); //mV
   	myPs->th_table.Vref = Vref * 1000;

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "R1[ohm]") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
   	tmp = fscanf(fp, "%s", buf);
	R1 = atof(buf); //ohm
   	myPs->th_table.R1 = R1;

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "R2[ohm]") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
   	tmp = fscanf(fp, "%s", buf);
	R2 = atof(buf); //ohm
    myPs->th_table.R2 = R2;
	
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "R25[ohm]") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
   	tmp = fscanf(fp, "%s", buf);
	R25 = atof(buf); //ohm 
    myPs->th_table.R25 = R25;
	
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "Beta[K]") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
   	tmp = fscanf(fp, "%s", buf);
	Beta = atof(buf);
    myPs->th_table.Beta = Beta;

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "temp_degreeC") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "R_kOhm") != 0) {
		fclose(fp);
		return -2;
	}
	for(i=0; i < MAX_TH_DATA; i++) {
		memset(buf, 0, sizeof buf);
    	tmp = fscanf(fp, "%s", buf);
   		myPs->th_table.T_R[i][0] = atof(buf) * 1000.0; //temp * 1000
		if(strcmp(buf, "9999") == 0) { //equal
			if(i == 0) {
			} else {
				i = i - 1;
			}
			myPs->th_table.th_data_max_index = (short int)i;
			break;
		}

		memset(buf, 0, sizeof buf);
    	tmp = fscanf(fp, "%s", buf);
		Rth = atof(buf) * 1000.0; //ohm
   		myPs->th_table.T_R[i][1] = Rth;

	//	val = Vref * Rth / (R1 + R2 + Rth);
	//	myPs->th_table.V_TH[i] = (float)val; //mV
	}

    fclose(fp);
	return 0;
}
