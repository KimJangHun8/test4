#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include "../../INC/datastore.h"
#include "userlog.h"
#include "message.h"
#include "common_utils.h"
#include "network.h"
#include "local_utils.h"

extern volatile S_SYSTEM_DATA *myData;
extern volatile S_COOLING_CONTROL *myPs;
//volatile S_CH_DATA *myCh; //add <--20150512 oys
extern char psName[PROCESS_NAME_SIZE];

int Initialize(void)
{
	if(Open_SystemMemory(0) < 0) return -1;
	
	myPs = &(myData->CoolingControl);
	
	Init_SystemMemory();
	
	if(Read_CoolingControl_Config() < 0) return -2;
	
	myData->AppControl.signal[myPs->misc.psSignal] = P1;
	
	return 0;
}

void Init_SystemMemory(void)
{
	int i;
	
	memset((char *)&psName[0], 0, PROCESS_NAME_SIZE);
	strcpy(psName, "CoolingControl");
    myPs->misc.psSignal = APP_SIG_COOLING_CONTROL_PROCESS;

	for(i=0; i < MAX_SIGNAL; i++) {
		myPs->signal[i] = P0;
	}
	
	memset((char *)&myPs->reply, 0x00, sizeof(S_COOLING_REPLY));
	myPs->pingTimer = 0;
	//	myPs->netTimer = 0;
	//110402 kji w
	myPs->netTimer = myData->mData.misc.timer_1sec;

	memset((char *)&myPs->rcvCmd, 0x00, sizeof(S_COOLING_RCV_COMMAND));
	memset((char *)&myPs->rcvPacket, 0x00, sizeof(S_COOLING_RCV_PACKET));
	
	myPs->misc.cmd_serial = 0;
	myPs->misc.processPointer = (int)&myData;
}

int	Read_CoolingControl_Config(void)
{
    int tmp;
	char temp[20], buf[12], fileName[128];
    FILE *fp;

	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/cycler_data/config/parameter/CoolingControl_Config");
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "CoolingControl_Config file read error\n");
		system("cp ../Config_backup/CoolingControl_Config /root/cycler_data/config/parameter");
		userlog(DEBUG_LOG, psName, "CoolingControl_Config file copy\n");
    	if((fp = fopen(fileName, "r")) == NULL) {
			return -1;
		}
	}

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

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.send_monitor_data_interval = (unsigned long)atol(buf) * 100;

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.send_save_data_interval = (unsigned long)atol(buf);

	
	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myPs->config.installed_cooling = (unsigned int)atoi(buf);
	
    fclose(fp);
	return 0;
}

