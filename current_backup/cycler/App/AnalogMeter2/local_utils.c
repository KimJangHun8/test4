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
extern volatile S_ANALOG_METER  *myPs;
extern char	psName[PROCESS_NAME_SIZE];

void Init_SystemMemory(void)
{
	int i;
	
	for(i=0; i < MAX_SIGNAL; i++) {
		myPs->signal[i] = P0;
	}

	myPs->misc.processPointer = (int)&myData;
}

int Read_AnalogMeter_CaliData(void)
{
    int tmp, i,j;
	char temp[20], buf[6], fileName[128];
    FILE *fp;

	memset(fileName, 0x00, sizeof(fileName));
	sprintf(fileName, "/root/cycler_data/config/caliData/CALI_T_2");

	//point = myPs->cali_config.useRange;
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, 
						"Can not open AnalogMeter2_CaliData file(Read)");
		return -1;
	}
	if((fp = fopen(fileName, "r")) != NULL){

		for(i = 0; i < MAX_METER_COUNT; i++){
		    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
			for(j = 0; j < myPs->config.chPerModule; j++){
				memset(buf, 0x00, sizeof buf);
   				tmp = fscanf(fp, "%s", buf);
  		 		myPs->config.measure_offset[i][j] = atol(buf);
			}
		}
		for(i = 0; i < MAX_METER_COUNT; i++){
		    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
			for(j = 0; j < myPs->config.chPerModule; j++){
				memset(buf, 0x00, sizeof buf);
   				tmp = fscanf(fp, "%s", buf);
  		 		myPs->config.measure_gain[i][j] = atof(buf);
				if(myPs->config.measure_gain[i][j]  == 0.0)
					myPs->config.measure_gain[i][j]  = 1.0 ;
			}
		}
	}
	fclose(fp);
	return 0;
}

int	Write_AnalogMeter_CaliData(void)
{
    int i,j;
	char fileName[128];
    FILE *fp;

	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/cycler_data/config/caliData/CALI_T_2");
	// /root/cycler_data/config/parameter/AnalogMeter_Config
    if((fp = fopen(fileName, "w")) == NULL) {
		userlog(DEBUG_LOG, psName, 
						"Can not open AnalogMeter2_CaliData file(Write)");
		return -1;
	}
	for(i=0;i<MAX_METER_COUNT;i++) {
  		fprintf(fp, "measure_offset[%d]  :  	",i);
		for(j=0;j<myPs->config.chPerModule;j++) {
  	 		 fprintf(fp, "%ld ",myPs->config.measure_offset[i][j]);
		}
  	 	fprintf(fp, "\n");
	}
	for(i=0;i<MAX_METER_COUNT;i++) {
  		fprintf(fp, "measure_gain[%d]    :	",i);
		for(j=0;j<myPs->config.chPerModule;j++) {
  	 		 fprintf(fp, "%f ",myPs->config.measure_gain[i][j]);
		}
  	 	fprintf(fp, "\n");
	}
	fclose(fp);
	return 0;
}


int	Read_AnalogMeter_Config(void)
{
    int tmp, i, j, loop = 1, caliDataFlag = 0, noUseChNo = 0;
	char temp[20], buf[20], buf2[20], fileName[128];
    FILE *fp;

	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/cycler_data/config/parameter/AnalogMeter_Config_2");

// Find chPerModule start
   if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "AnalogMeter_Config_2 file read error\n");
		system("cp ../Config_backup/AnalogMeter_Config_2 /root/cycler_data/config/parameter");
		userlog(DEBUG_LOG, psName, "AnalogMeter_Config_2 file copy\n");
    	if((fp = fopen(fileName, "r")) == NULL) {
			return -1;
		}
	}
	while(loop) {
		memset(buf, 0x00, sizeof(buf));
		fscanf(fp, "%s",buf);
		strcpy(buf2, "chPerModule");
		if(!strcmp(buf, buf2)) {
			tmp = fscanf(fp, "%s",buf);
			tmp = fscanf(fp, "%s",buf);
   			myPs->config.chPerModule = (unsigned char)atoi(buf);
			loop = 0;
		} else {
   			myPs->config.chPerModule = 8;
			loop++;
			if(loop >= 250)
				loop = 0;
		} 
	}
	fclose(fp);
//end
	
	// /root/cycler_data/config/parameter/AnalogMeter_Config
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "AnalogMeter_Config_2 file read error\n");
		system("cp ../Config_backup/AnalogMeter_Config_2 /root/cycler_data/config/parameter");
		userlog(DEBUG_LOG, psName, "AnalogMeter_Config_2 file copy\n");
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
    myPs->config.countMeter = (unsigned char)atoi(buf);

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
	//0:READ_T_K, 1:READ_V, 2:READ_I 3:READ_T_T
    myPs->config.readType = (unsigned char)atoi(buf);
	
	memset(buf, 0x00, sizeof(buf));
	memset(buf2, 0x00, sizeof(buf2));
	tmp = fscanf(fp, "%s", buf);
	strcpy(buf2, "measure_offset[0]");
	if(!strcmp(buf, buf2)) {
		i = 0;
		tmp = fscanf(fp, "%s", temp);
		for(j = 0; j < myPs->config.chPerModule; j++){
			memset(buf, 0x00, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
		 		myPs->config.measure_offset[i][j] = atol(buf);
		}
		for(i = 1; i < MAX_METER_COUNT; i++){
			loop = 1;
			memset(buf2, 0x00, sizeof(buf2));
			sprintf(buf2, "measure_offset[%d]", i);
			while(loop){
				if(i == 1) {
					noUseChNo++;
				}
				memset(buf, 0x00, sizeof(buf));
				tmp = fscanf(fp, "%s", buf);
				if(!strcmp(buf, buf2)) {
					loop = 0;
				}
				memset(temp, 0x00, sizeof(temp));
				strcpy(temp, "functionType");
				if(!strcmp(buf, temp)) {
					caliDataFlag = 2;
					break;
				}
			}
			if(caliDataFlag == 2) {
				i = MAX_METER_COUNT;
			} else {
				tmp = fscanf(fp, "%s", temp);
				for(j = 0; j < myPs->config.chPerModule; j++){
					memset(buf, 0x00, sizeof buf);
					tmp = fscanf(fp, "%s", buf);
 			 		myPs->config.measure_offset[i][j] = atol(buf);
				}
				caliDataFlag = 1;
			}
		}
	}

	if(caliDataFlag == 1){
		for(i = 0; i < noUseChNo; i++) {
			tmp = fscanf(fp, "%s", temp);
		}
	}
	caliDataFlag = 0;

    tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.functionType = (unsigned char)atoi(buf);
   
    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.comBps2 = atoi(buf);
   
    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.commType2 = (unsigned char)atoi(buf);
   
   	tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.autoStart = (unsigned char)atoi(buf);

	memset(buf, 0x00, sizeof(buf));
	memset(buf2, 0x00, sizeof(buf2));
	tmp = fscanf(fp, "%s", buf);
	strcpy(buf2, "measure_gain[0]");
	if(!strcmp(buf, buf2)) {
		i = 0;
	    tmp = fscanf(fp, "%s", temp);
		for(j = 0; j < myPs->config.chPerModule; j++){
			memset(buf, 0x00, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
	 		myPs->config.measure_gain[i][j] = atof(buf);
			if(myPs->config.measure_gain[i][j]  == 0.0)
				myPs->config.measure_gain[i][j]  = 1.0 ;
		}
		for(i = 1; i < MAX_METER_COUNT; i++){
			loop = 1;
			memset(buf2, 0x00, sizeof(buf2));
			sprintf(buf2, "measure_gain[%d]", i);
			while(loop) {
				memset(buf, 0x00, sizeof(buf));
				tmp = fscanf(fp, "%s", buf);
				if(!strcmp(buf, buf2)) {
					loop = 0;
				}
				memset(temp, 0x00, sizeof(temp));
				strcpy(temp, "multiNum");
				if(!strcmp(buf, temp)) {
					caliDataFlag = 2;
					break;
				}
			}
			if(caliDataFlag == 2) {
				i = MAX_METER_COUNT;
			} else {
				tmp = fscanf(fp, "%s", temp);
				for(j = 0; j < myPs->config.chPerModule; j++){
					memset(buf, 0x00, sizeof buf);
					tmp = fscanf(fp, "%s", buf);
			 		myPs->config.measure_gain[i][j] = atof(buf);
					if(myPs->config.measure_gain[i][j]  == 0.0)
						myPs->config.measure_gain[i][j]  = 1.0 ;
				}
				caliDataFlag = 1;
			}
		}
	}

	if(caliDataFlag == 1){
		for(i = 0; i < noUseChNo; i++) {
			tmp = fscanf(fp, "%s", temp);
		}
	}
	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.multiNum = (unsigned char)atoi(buf);
	
	tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.connectionCheck = (unsigned char)atoi(buf);

	tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.checkSumFlag = (unsigned char)atoi(buf);
	
	tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.chPerModule = (unsigned char)atoi(buf);

	tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.tempNonDisplay = (unsigned char)atoi(buf);

	tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.auxStartCh = (unsigned char)atoi(buf);

	tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.auxControlFlag = (unsigned char)atoi(buf);

	fclose(fp);

	if(caliDataFlag >= 1) {
		Write_AnalogMeter_CaliData();
		userlog(DEBUG_LOG, psName, "Generated CALI_T\n");
		Write_AnalogMeter_Config();
		userlog(DEBUG_LOG, psName, "Rewrited AnalogMeter_Config\n");
	}

	return 0;
}

int	Write_AnalogMeter_Config(void)
{
	char fileName[128];
    FILE *fp;

	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/cycler_data/config/parameter/AnalogMeter_Config_2");
	// /root/cycler_data/config/parameter/AnalogMeter_Config
    if((fp = fopen(fileName, "w")) == NULL) {
		userlog(DEBUG_LOG, psName, "AnalogMeter_Config_2 file read error\n");
		return -1;
	}
    fprintf(fp, "comPort            :	%d\n",myPs->config.comPort); 
    fprintf(fp, "comBps             :	%d\n",myPs->config.comBps); 
    fprintf(fp, "CmdSendLog         :	%d\n",myPs->config.CmdSendLog); 
    fprintf(fp, "CmdRcvLog          :	%d\n",myPs->config.CmdRcvLog); 
    fprintf(fp, "CmdSendLog_Hex     :	%d\n",myPs->config.CmdSendLog_Hex); 
    fprintf(fp, "CmdRcvLog_Hex      :	%d\n",myPs->config.CmdRcvLog_Hex); 
    fprintf(fp, "CommCheckLog       :	%d\n",myPs->config.CommCheckLog); 
    fprintf(fp, "countMeter         :	%d\n",myPs->config.countMeter); 
    fprintf(fp, "readType           :	%d\n",myPs->config.readType); 
    fprintf(fp, "functionType       :	%d\n",myPs->config.functionType); 
    fprintf(fp, "comBps2            :	%d\n",myPs->config.comBps2); 
    fprintf(fp, "comType2           :	%d\n",myPs->config.commType2); 
    fprintf(fp, "autoStart          :	%d\n",myPs->config.autoStart); 
    fprintf(fp, "multiNum           :	%d\n",myPs->config.multiNum); 
    fprintf(fp, "connectionCheck    :	%d\n",myPs->config.connectionCheck); 
    fprintf(fp, "checkSumFlag       :	%d\n",myPs->config.checkSumFlag); 
    fprintf(fp, "chPerModule        :	%d\n",myPs->config.chPerModule); 
    fprintf(fp, "tempNonDisplay     :	%d\n",myPs->config.tempNonDisplay); 
    fprintf(fp, "auxStartCh[0BASE]  :	%d\n",myPs->config.auxStartCh); 
    fprintf(fp, "auxControlFlag     :	%d\n",myPs->config.auxControlFlag); 
	fclose(fp);
	return 0;
}

int Read_TempArray_A(void)
{
	short int monitor_no, hw_no, bd, ch;
    int tmp, i;
	char temp[20], buf[12], fileName[128];
    FILE *fp;

	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/cycler_data/config/parameter/TempArray_A");
	// /root/cycler_data/config/parameter/TempArray_A
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "TempArray_A file read error\n");
		system("cp ../Config_backup/TempArray_A /root/cycler_data/config/parameter");
		userlog(DEBUG_LOG, psName, "TempArray_A file copy\n");
    	if((fp = fopen(fileName, "r")) == NULL) {
			return -1;
		}
	}

   	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
   	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
   	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
   	tmp = fscanf(fp, "%s", temp);

	for(i=0; i < MAX_TEMP_CH; i++) {
		memset(buf, 0x00, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		monitor_no = (short int)atoi(buf);

   		tmp = fscanf(fp, "%s", temp);
		memset(buf, 0x00, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		hw_no = (short int)atoi(buf);

   		tmp = fscanf(fp, "%s", temp);
		memset(buf, 0x00, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		bd = (short int)atoi(buf);

   		tmp = fscanf(fp, "%s", temp);
		memset(buf, 0x00, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		ch = (short int)atoi(buf);

		if(bd >= myData->mData.config.installedBd){
			bd = 0;
			ch = 0;
		}

		myData->TempArray1[i].number1 = monitor_no;
		myData->TempArray1[i].number2 = hw_no;
		myData->TempArray1[i].bd = bd;
		myData->TempArray1[i].ch = ch;

		myData->TempArray2[hw_no-1].number1 = monitor_no;
		myData->TempArray2[hw_no-1].number2 = hw_no;
		myData->TempArray2[hw_no-1].bd = bd;
		myData->TempArray2[hw_no-1].ch = ch;
	}

    fclose(fp);
	return 0;
}

