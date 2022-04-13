#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "../../INC/datastore.h"
#include "userlog.h"
#include "serial.h"
#include "local_utils.h"

extern volatile S_SYSTEM_DATA *myData;
extern volatile S_CALI_METER  *myPs;
extern char	psName[PROCESS_NAME_SIZE];

void Init_SystemMemory(void)
{
	int i;
	
	for(i=0; i < MAX_SIGNAL; i++) {
		myPs->signal[i] = P0;
	}

	myPs->misc.processPointer = (int)&myData;
}

int	Read_CaliMeter_Config(void)
{
    int tmp;
	char temp[20], buf[6], fileName[128];
    FILE *fp;

	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/cycler_data/config/parameter/CaliMeter_Config_2");
	// /root/cycler_data/config/parameter/CaliMeter_Config_2
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "CaliMeter_Config_2 file read error\n");
		system("cp ../Config_backup/CaliMeter_Config_2 /root/cycler_data/config/parameter");
		userlog(DEBUG_LOG, psName, "CaliMeter_Config_2 file copy\n");
    	if((fp = fopen(fileName, "r")) == NULL) {
			return -1;
		}
	}

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.comPort = atoi(buf);

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.comBps = atoi(buf);

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
	//0:(READ_V_V)v-v,i-v, 1:(READ_V_I)v-v,i-i
    myPs->config.readType = (unsigned char)atoi(buf);

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
	//1:shunt 0.010ohm, 2:shunt 0.001ohm, 3:DCCT 600A/400mA,
	//4:DCCT 150A/200mA, 5:shunt 10ohm/100ohm, 6:meter DCI
	//7:shunt 0.1mOhm, 8:shunt 0.1 Ohm
    myPs->config.measureI = (unsigned char)atoi(buf);

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.I_offset = atol(buf);

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.Shunt_Sel_Calibrator = (unsigned char)atoi(buf);
	
   	tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.Lan_Use = (unsigned char)atoi(buf);

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(temp, 0x00, sizeof temp);
    tmp = fscanf(fp, "%s", temp);
    memcpy((char *)&myPs->config.CaliMeterIP[0], (char *)&temp[0],
		sizeof(char)*16);

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.MeterSock_Port = atoi(buf);

    fclose(fp);
	return 0;
}

int	Read_Shunt_Select_Calibrator_Config(void)
{
    int tmp, i;
	char temp[64], buf[6], fileName[128];
    FILE *fp;

	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/cycler_data/config/parameter/Shunt_Select_Calibrator_Config");
	// /root/cycler_data/config/parameter/Shunt_Select_Calibrator_Config
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "Shunt_Select_Calibrator_Config file read error\n");
		system("cp ../Config_backup/Shunt_Select_Calibrator_Config /root/cycler_data/config/parameter");
		userlog(DEBUG_LOG, psName, "Shunt_Select_Calibrator_Config file copy\n");
    	if((fp = fopen(fileName, "r")) == NULL) {
			return -1;
		}
	}
	
    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	for(i = 0; i < MAX_RANGE; i++){
		memset(buf, 0x00, sizeof buf);
	    tmp = fscanf(fp, "%s", buf);
    	myPs->caliConfig.Shunt[i] = atof(buf);
	}

   	tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	for(i = 0; i < MAX_RANGE; i++){
		memset(buf, 0x00, sizeof buf);
	    tmp = fscanf(fp, "%s", buf);
    	myPs->caliConfig.Shunt_Range_Select[i] = (unsigned char)atoi(buf);
	}

   	tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	for(i = 0; i < MAX_RANGE; i++){
		memset(buf, 0x00, sizeof buf);
	    tmp = fscanf(fp, "%s", buf);
    	memcpy((char*)&myPs->caliConfig.SerialNo[i], buf, 20);
	}

    fclose(fp);
	return 0;
}

