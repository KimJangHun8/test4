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
	strcpy(fileName, "/root/cycler_data/config/parameter/CaliMeter_Config");
	// /root/cycler_data/config/parameter/CaliMeter_Config
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "CaliMeter_Config file read error\n");
		system("cp ../Config_backup/CaliMeter_Config /root/cycler_data/config/parameter");
		userlog(DEBUG_LOG, psName, "CaliMeter_Config file copy\n");
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

int	Write_CaliMeter_Config(void)
{
	char fileName[128];
	FILE *fp;

	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/cycler_data/config/parameter/CaliMeter_Config");
	if((fp = fopen(fileName, "w")) == NULL){
		userlog(DEBUG_LOG, psName, "ClaiMeter Config file write error\n");
		return -1;
	}
	
	if(myPs->rcvMeterSet.Type == P2){	//34461
		//commType - 1 : Lan use / 2 : 232 to Lan
		myPs->config.Lan_Use = myPs->rcvMeterSet.commType;
		if(myPs->config.Lan_Use == P1){
			memcpy((char *)&myPs->config.CaliMeterIP[0], 
						(char *)&myPs->rcvMeterSet.IP[0], sizeof(char)*16); 
			myPs->config.MeterSock_Port = myPs->rcvMeterSet.Port;
		}
	}else{
		myPs->config.Lan_Use = 0;
	}
	
	fprintf(fp, "comPort				:	%d\n", myPs->config.comPort);
	fprintf(fp, "comBps				:	%d\n", myPs->config.comBps);
	fprintf(fp, "CmdSendLog			:	%d\n", myPs->config.CmdSendLog);
	fprintf(fp, "CmdRcvLog			:	%d\n", myPs->config.CmdRcvLog);
	fprintf(fp, "CmdSendLog_Hex		:	%d\n", myPs->config.CmdSendLog_Hex);
	fprintf(fp, "CmdRcvLog_Hex		:	%d\n", myPs->config.CmdRcvLog_Hex);
	fprintf(fp, "commCheckLog		:	%d\n", myPs->config.CommCheckLog);
	fprintf(fp, "readType			:	%d\n", myPs->config.readType);
	fprintf(fp, "measureI			:	%d\n", myPs->config.measureI);
	fprintf(fp, "I_offset			:	%ld\n", myPs->config.I_offset);
	fprintf(fp, "Shunt_Sel_Cali		:	%d\n",
								 myPs->config.Shunt_Sel_Calibrator);
	fprintf(fp, "Lan_Use				:	%d\n", myPs->config.Lan_Use);
	fprintf(fp, "CaliMeterIP			:	%s\n", myPs->config.CaliMeterIP);
	fprintf(fp, "MeterSock_Port		:	%d\n", myPs->config.MeterSock_Port);
	fprintf(fp, "\n");
	fprintf(fp, "0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n");
	fprintf(fp,	"0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n");
	fprintf(fp,	"================================================================================\n");
	fprintf(fp,	"Option Setting Description\n");
	fprintf(fp,	"Date : 2015.07.01\n");
	fprintf(fp,	"Writer : OYS\n");
	fprintf(fp,	"================================================================================\n");
	fprintf(fp,	"comPort				=> Serial Port No. (Default : 1)\n");
	fprintf(fp,	"comBps				=> Communication Speed (Default : 9600)\n");
	fprintf(fp,	"CmdSendLog			=> CmdSendLog Print Flag\n");
	fprintf(fp,	"CmdRcvLog			=> CmdRcvLog Print Flag\n");
	fprintf(fp,	"CmdSendLog_Hex		=> CmdSendLog_Hex Print Flag\n");
	fprintf(fp,	"CmdRcvLog_Hex		=> CmdRcvLog_Hex Print Flag\n");
	fprintf(fp,	"CommCheckLog		=> CommunicationChekLog Print Flag\n");
	fprintf(fp,	"readType			=> Read Type\n");
	fprintf(fp,	"						0 : READ_V_V\n");
	fprintf(fp,	"						1 : READ_V_I\n\n");

	fprintf(fp,	"measureI			=> Shunt Type\n");
	fprintf(fp,	"						1 : 10 mOhm\n");
	fprintf(fp,	"						2 :  1 mOhm\n");
	fprintf(fp,	"						3 : DCCT 600A/400mA\n");
	fprintf(fp,	"						4 : DCCT 150A/200mA\n");
	fprintf(fp,	"						5 : 10 Ohm, 100 Ohm\n");
	fprintf(fp,	"						6 : meter DCI\n");
	fprintf(fp,	"						7 : 0.1 mOhm\n");
	fprintf(fp,	"						8 : 100 mOhm\n\n");

	fprintf(fp,	"I_offset			=> (Default : 0)\n");
	fprintf(fp,	"Shunt_Sel_Cali		=> Shunt Select Calibrator (Default : 1)\n");
	fprintf(fp,	"Lan_Use				=> CaliMeter Select (34401A / 34461A)\n");
	fprintf(fp,	"						0 : 34401A Use 232 protocal\n");
	fprintf(fp,	"						1 : 34461A Use Lan Port\n");
	fprintf(fp,	"						2 : 34461A Use 232 to Lan Converter\n\n");
					
	fprintf(fp,	"CaliMeterIp 		=> 34461A IP Setting\n");
	fprintf(fp,	"MeterSock_Port		=> 34461A Lan Socket Port Setting \n");
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

