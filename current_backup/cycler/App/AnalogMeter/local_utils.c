#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "../../INC/datastore.h"
#include "userlog.h"
#include "serial.h"
#include "local_utils.h"
#include "cali.h"

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
	char temp2[32];
    int tmp, i,j;
	char temp[20], buf[6], fileName[128];
    FILE *fp;

	memset(fileName, 0x00, sizeof(fileName));
	sprintf(fileName, "/root/cycler_data/config/caliData/CALI_T");

	//point = myPs->cali_config.useRange;
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, 
						"Can not open AnalogMeter_CaliData file(Read)");
		return -1;
	}
	if((fp = fopen(fileName, "r")) != NULL){

		for(i = 0; i < MAX_METER_COUNT; i++){
			memset(temp2, 0, sizeof temp2);
			sprintf(temp2, "%s%d%s", "measure_offset[",i,"]");
			tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, temp2) != 0){
				userlog(DEBUG_LOG, psName, "normal[%s] : abnormal[%s]\n",temp2, temp);
				return -1;
			}
			memset(temp, 0, sizeof temp);
			tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, ":") != 0){
				userlog(DEBUG_LOG, psName, "normal[%s] : abnormal[%s]\n",temp2, temp);
				return -1;
			}
			for(j = 0; j < myPs->config.chPerModule; j++){
				memset(buf, 0x00, sizeof buf);
   				tmp = fscanf(fp, "%s", buf);
  		 		myPs->config.measure_offset[i][j] = atol(buf);
			}
		}
		for(i = 0; i < MAX_METER_COUNT; i++){
			memset(temp2, 0, sizeof temp2);
			sprintf(temp2, "%s%d%s", "measure_gain[",i,"]");
			tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, temp2) != 0){
				userlog(DEBUG_LOG, psName, "normal[%s] : abnormal[%s]\n",temp2, temp);
				return -1;
			}
			memset(temp, 0, sizeof temp);
			tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, ":") != 0){
				userlog(DEBUG_LOG, psName, "normal[%s] : abnormal[%s]\n",temp2, temp);
				return -1;
			}
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

int Read_AnalogMeter_Ambient_CaliData(void)
{
    int tmp, i,j;
	char temp[20], buf[6], fileName[128];
    FILE *fp;

	memset(fileName, 0x00, sizeof(fileName));
	sprintf(fileName, "/root/cycler_data/config/caliData/CALI_Ambient_T");

	//point = myPs->cali_config.useRange;
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, 
						"Can not open AnalogMeter_Ambient_CaliData file(Read)");
		return -1;
	}
	if((fp = fopen(fileName, "r")) != NULL){

		for(i = 0; i < MAX_METER_COUNT; i++){
		    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
			for(j = 0; j < myPs->config.chPerModule; j++){
				memset(buf, 0x00, sizeof buf);
   				tmp = fscanf(fp, "%s", buf);
  		 		myPs->config.ambient_offset[i][j] = atol(buf);
			}
		}
		for(i = 0; i < MAX_METER_COUNT; i++){
		    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
			for(j = 0; j < myPs->config.chPerModule; j++){
				memset(buf, 0x00, sizeof buf);
   				tmp = fscanf(fp, "%s", buf);
  		 		myPs->config.ambient_gain[i][j] = atof(buf);
				if(myPs->config.ambient_gain[i][j]  == 0.0)
					myPs->config.ambient_gain[i][j]  = 1.0 ;
			}
		}
	}
	fclose(fp);
	return 0;
}

int Read_AnalogMeter_gas_CaliData(void)
{
    int tmp, i,j;
	char temp[20], buf[6], fileName[128];
    FILE *fp;

	memset(fileName, 0x00, sizeof(fileName));
	sprintf(fileName, "/root/cycler_data/config/caliData/CALI_Gas_Voltage");

	//point = myPs->cali_config.useRange;
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, 
						"Can not open AnalogMeter_Gas_Voltage_CaliData file(Read)");
		return -1;
	}
	if((fp = fopen(fileName, "r")) != NULL){

		for(i = 0; i < MAX_METER_COUNT; i++){
		    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
			//for(j = 0; j < 8; j++){	//M-7003
			//for(j = 0; j < 4; j++){		//M-7002
			for(j = 0; j < myPs->config.chPerModule2 ; j++){	//hun_211019
				memset(buf, 0x00, sizeof buf);
   				tmp = fscanf(fp, "%s", buf);
  		 		myPs->config.gas_offset[i][j] = atol(buf);
			}
		}
		for(i = 0; i < MAX_METER_COUNT; i++){
		    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
			//for(j = 0; j < 8; j++){	//M-7003
			//for(j = 0; j < 4; j++){		//M-7002
			for(j = 0; j < myPs->config.chPerModule2 ; j++){	//hun_211019
				memset(buf, 0x00, sizeof buf);
   				tmp = fscanf(fp, "%s", buf);
  		 		myPs->config.gas_gain[i][j] = atof(buf);
				if(myPs->config.gas_gain[i][j]  == 0.0)
					myPs->config.gas_gain[i][j]  = 1.0 ;
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
	strcpy(fileName, "/root/cycler_data/config/caliData/CALI_T");
	// /root/cycler_data/config/parameter/AnalogMeter_Config
    if((fp = fopen(fileName, "w")) == NULL) {
		userlog(DEBUG_LOG, psName, 
						"Can not open AnalogMeter_CaliData file(Write)");
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

int	Write_AnalogMeter_Ambient_CaliData(void)
{
    int i,j;
	char fileName[128];
    FILE *fp;

	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/cycler_data/config/caliData/CALI_Ambient_T");
	// /root/cycler_data/config/parameter/AnalogMeter_Config
    if((fp = fopen(fileName, "w")) == NULL) {
		userlog(DEBUG_LOG, psName, 
						"Can not open AnalogMeter_Ambient_CaliData file(Write)");
		return -1;
	}
	for(i=0;i<MAX_METER_COUNT;i++) {
  		fprintf(fp, "measure_offset[%d]  :  	",i);
		for(j=0;j<myPs->config.chPerModule;j++) {
  	 		 fprintf(fp, "%ld ",myPs->config.ambient_offset[i][j]);
		}
  	 	fprintf(fp, "\n");
	}
	for(i=0;i<MAX_METER_COUNT;i++) {
  		fprintf(fp, "measure_gain[%d]    :	",i);
		for(j=0;j<myPs->config.chPerModule;j++) {
  	 		 fprintf(fp, "%f ",myPs->config.ambient_gain[i][j]);
		}
  	 	fprintf(fp, "\n");
	}
	fclose(fp);
	return 0;
}

int	Write_AnalogMeter_gas_CaliData(void)
{
    int i,j;
	char fileName[128];
    FILE *fp;

	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/cycler_data/config/caliData/CALI_Gas_Voltage");
	// /root/cycler_data/config/parameter/AnalogMeter_Config
    if((fp = fopen(fileName, "w")) == NULL) {
		userlog(DEBUG_LOG, psName, 
						"Can not open AnalogMeter_Gas_Voltage_CaliData file(Write)");
		return -1;
	}
	for(i=0;i<MAX_METER_COUNT;i++) {
  		fprintf(fp, "measure_offset[%d]  :  	",i);
		//for(j=0;8;j++) {	//M-7003
		for(j=0; j < 4;j++) {	//M-7002
  	 		 fprintf(fp, "%ld ",myPs->config.gas_offset[i][j]);
		}
  	 	fprintf(fp, "\n");
	}
	for(i=0;i<MAX_METER_COUNT;i++) {
  		fprintf(fp, "measure_gain[%d]    :	",i);
		//for(j=0;8;j++) {	//M-7003
		for(j=0; j < 4;j++) {	//M-7002
  	 		 fprintf(fp, "%f ",myPs->config.gas_gain[i][j]);
		}
  	 	fprintf(fp, "\n");
	}
	fclose(fp);
	return 0;
}

#ifdef _TEMP_CALI  
int Read_AnalogMeter_CaliData_2()
{
	char temp[32], buf[32], fileName[256];
   	int tmp, i, j;
	FILE *fp;
		
	memset(fileName, 0x00, sizeof(fileName));
	sprintf(fileName, "/root/cycler_data/config/caliData/CALI_TEMP");
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, 
						"Can not open AnalogMeter_CaliData_2 file(Read)");
		return -1;
	}
   		
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "Temp_point_count:") != 0) {
		fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
   	tmp = fscanf(fp, "%s", buf);
  	myData->temp_cali.point.setPointCount = atoi(buf);

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "Temp_range_count:") != 0) {
		fclose(fp);
		return -3;
	}
	memset(buf, 0, sizeof buf);
   	tmp = fscanf(fp, "%s", buf);
	if((myData->temp_cali.point.setPointCount-1) != atoi(buf)) {
		fclose(fp);
		return -4;
	}

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "index") != 0) {
		fclose(fp);
		return -5;
	}
	//temperature calibration Reference point
	for(i=0; i < MAX_TEMP_POINT; i++) {
		memset(buf, 0, sizeof buf);
    	tmp = fscanf(fp, "%s", buf);
   		myData->temp_cali.point.setTempPoint[i] = atol(buf);
		if(i > 1) {
			if((myData->temp_cali.point.setTempPoint[i] != 99999000) 
				&& (myData->temp_cali.point.setTempPoint[i-1] 
				> myData->temp_cali.point.setTempPoint[i])) {
				fclose(fp);
				return -6;
			}
		}

		if(i == MAX_TEMP_POINT-1) {
		} else {
			memset(temp, 0, sizeof temp);
			tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, "(ratio/offset)") != 0) {
				fclose(fp);
				return -7;
			}
		}

	}

	//temperature calibration value
	for(i=0; i < MAX_TEMP_CH; i++) { // MAX AnalogMeter 100EA
		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		memset(buf, 0, sizeof buf);
		sprintf(buf, "%d", i+1);
		if(strcmp(temp, buf) != 0) {
			fclose(fp);
			return -8;
		}

		for(j=0; j < MAX_TEMP_POINT; j++) {
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->temp_cali.data.setTempValue[j][i] = atol(buf);

			if(j != MAX_TEMP_POINT-1) {
				memset(temp, 0, sizeof temp);
				tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "(") != 0) {
					fclose(fp);
					return -9;
				}
				memset(buf, 0, sizeof buf);
   		 		tmp = fscanf(fp, "%s", buf);
   				myData->temp_cali.measure.gain[j][i] = atof(buf);
			
				memset(temp, 0, sizeof temp);
				tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "/") != 0) {
					fclose(fp);
					return -10;
				}
				memset(buf, 0, sizeof buf);
    			tmp = fscanf(fp, "%s", buf);
   				myData->temp_cali.measure.offset[j][i] = atof(buf);
				
				memset(temp, 0, sizeof temp);
				tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, ")") != 0) {
					fclose(fp);
					return -11;
				}
			}
		}
	}


	fclose(fp);
	return 0;
}

int	Write_AnalogMeter_CaliData_2() //200102
{
	int i, j;
//	int	countMeter, chPerModule, tempNo;
	int	tempNo;
	long temp;
	float ratio, offset;
	char fileName[256];
	int count = 0;
    int	count1 = 0;
    FILE *fp;
	
	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/cycler_data/config/caliData/CALI_TEMP");
    if((fp = fopen(fileName, "w")) == NULL) {
		userlog(DEBUG_LOG, psName, 
						"Can not open AnalogMeter_CaliData_2 file(Write)");
		return -1;
	}

//	countMeter = myPs->config.countMeter;
//	chPerModule = myPs->config.chPerModule;
//	tempNo = countMeter * chPerModule;
	if(myData->AppControl.loadProcess[LOAD_ANALOGMETER] == P1){
		count = myData->AnalogMeter.config.chPerModule * myData->AnalogMeter.config.countMeter;
	}	
	if(myData->AppControl.loadProcess[LOAD_ANALOGMETER2] == P1){
		count1 = myData->AnalogMeter2.config.chPerModule * myData->AnalogMeter2.config.countMeter;
	}	
	tempNo = count + count1;
	
	fprintf(fp,"Temp_point_count: %d\n",
		  			myData->temp_cali.point.setPointCount);
	fprintf(fp,"Temp_range_count: %d\n",
		  			myData->temp_cali.point.setPointCount-1);
	fprintf(fp, "index   ");
	for(i=0; i < MAX_TEMP_POINT -1; i++) {
		fprintf(fp, "%ld (ratio/offset) ",
			   	myData->temp_cali.point.setTempPoint[i]);
	}
	fprintf(fp, "%ld\n",
			myData->temp_cali.point.setTempPoint[MAX_TEMP_POINT-1]);
	for(i=0; i < tempNo; i++) {
		fprintf(fp, "%d ", i+1);
		for(j=0; j < MAX_TEMP_POINT; j++) {
			if(j <= myData->temp_cali.point.setPointCount-1) {
				temp = myData->temp_cali.data.setTempValue[j][i];
				if(j == myData->temp_cali.point.setPointCount-1) {
					ratio = 0;
					offset = 0;
				} else {
					ratio = myData->temp_cali.measure.gain[j][i];
					offset = myData->temp_cali.measure.offset[j][i];
				} 
			} else {
				temp = 99999000;
				ratio = 0;
				offset = 0;
			}
			if(j == MAX_TEMP_POINT - 1) {
				fprintf(fp, "%ld\n", temp);
			} else {
				fprintf(fp, "%ld ( %0.3f / %0.3f )  ", temp, ratio, offset);
			}
		}
	}

	for(i=tempNo; i < MAX_TEMP_CH; i++) {
		fprintf(fp, "%d ", i+1);
		for(j=0; j < MAX_TEMP_POINT; j++) {
			temp = 99999000;
			ratio = 0;
			offset = 0;
			
			if(j == MAX_TEMP_POINT -1) {
				fprintf(fp, "%ld\n", temp);
			} else {
				fprintf(fp, "%ld ( %0.3f / %0.3f )  ", temp, ratio, offset);
			}
		}
	}
	fclose(fp);
	userlog(DEBUG_LOG, psName, "CALI_TEMP file write end\n");
	return 0;
}
#endif

int	Read_AnalogMeter_Config(void)
{
    int tmp, i, j, loop = 1, caliDataFlag = 0, noUseChNo = 0;
	char temp[20], buf[20], buf2[20], fileName[128];
    FILE *fp;

	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/cycler_data/config/parameter/AnalogMeter_Config");

// Find chPerModule start
   if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "AnalogMeter_Config file read error\n");
		system("cp ../Config_backup/AnalogMeter_Config /root/cycler_data/config/parameter");
		userlog(DEBUG_LOG, psName, "AnalogMeter_Config file copy\n");
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
			if(loop >= 350) //20200629 rewrite
				loop = 0;
		}	
	}
	fclose(fp);
//end
	// /root/cycler_data/config/parameter/AnalogMeter_Config
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "AnalogMeter_Config file read error\n");
		system("cp ../Config_backup/AnalogMeter_Config /root/cycler_data/config/parameter");
		userlog(DEBUG_LOG, psName, "AnalogMeter_Config file copy\n");
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

	tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.ambientModuleNo = (unsigned char)atoi(buf);	//hun_211019

	tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.gasModuleNo = (unsigned char)atoi(buf);

	tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.ambientModuleCount = (unsigned char)atoi(buf);	//hun_211019

	tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.gasModuleCount = (unsigned char)atoi(buf);

	tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.chPerModule2 = (unsigned char)atoi(buf);
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
	//hun_211019
	char fileName[128];
    FILE *fp;

	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/cycler_data/config/parameter/AnalogMeter_Config");
	// /root/cycler_data/config/parameter/AnalogMeter_Config
    if((fp = fopen(fileName, "w")) == NULL) {
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
    fprintf(fp, "ambientModuleNo    :	%d\n",myPs->config.ambientModuleNo);
    fprintf(fp, "gasModuleNo   		:	%d\n",myPs->config.gasModuleNo); 
    fprintf(fp, "ambientModuleCount :	%d\n",myPs->config.ambientModuleCount);
    fprintf(fp, "gasModuleCount   	:	%d\n",myPs->config.gasModuleCount); 
    fprintf(fp, "chPerModule2       :	%d\n",myPs->config.chPerModule2); 
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

		if(bd >= myData->mData.config.installedBd){ //220222
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

int	Write_AnalogMeter_CaliData_Default()
{
    int i,j;
	char fileName[128];
    FILE *fp;
	
	memset(fileName, 0x00, sizeof(fileName));
	system("cp -rf /root/cycler_data/config/caliData/CALI_T /root/cycler_data/config/caliData/CALI_T_Backup");
	strcpy(fileName, "/root/cycler_data/config/caliData/CALI_T");
    if((fp = fopen(fileName, "w")) == NULL) {
		userlog(DEBUG_LOG, psName, 
						"Can not open AnalogMeter_CaliData file(Write)");
		return -1;
	}
	for(i=0;i<MAX_METER_COUNT;i++) {
  		fprintf(fp, "measure_offset[%d]  :  	",i);
		for(j=0;j<myPs->config.chPerModule;j++) {
 			 myPs->config.measure_offset[i][j] = 0;
  	 		 fprintf(fp, "%ld ", myPs->config.measure_offset[i][j]);
		}
  	 	fprintf(fp, "\n");
	}
	for(i=0;i<MAX_METER_COUNT;i++) {
  		fprintf(fp, "measure_gain[%d]    :	",i);
		for(j=0;j<myPs->config.chPerModule;j++) {
  		 	myPs->config.measure_gain[i][j] = 1.000000;
  	 		 fprintf(fp, "%f ", myPs->config.measure_gain[i][j]);
		}
  	 	fprintf(fp, "\n");
	}
	fclose(fp);
	return 0;
}

int	Write_AnalogMeter2_CaliData_Default()
{
    int i,j;
	char fileName[128];
    FILE *fp;
	
	memset(fileName, 0x00, sizeof(fileName));
	system("cp -rf /root/cycler_data/config/caliData/CALI_T_2 /root/cycler_data/config/caliData/CALI_T_2_Backup");
	strcpy(fileName, "/root/cycler_data/config/caliData/CALI_T_2");
    if((fp = fopen(fileName, "w")) == NULL) {
		userlog(DEBUG_LOG, psName, 
						"Can not open AnalogMeter2_CaliData file(Write)");
		return -1;
	}
	for(i=0;i<MAX_METER_COUNT;i++) {
  		fprintf(fp, "measure_offset[%d]  :  	",i);
		for(j=0;j<myData->AnalogMeter2.config.chPerModule;j++) {
 			 myData->AnalogMeter2.config.measure_offset[i][j] = 0;
  	 		 fprintf(fp, "%ld ", myData->AnalogMeter2.config.measure_offset[i][j]);
		}
  	 	fprintf(fp, "\n");
	}
	for(i=0;i<MAX_METER_COUNT;i++) {
  		fprintf(fp, "measure_gain[%d]    :	",i);
		for(j=0;j<myData->AnalogMeter2.config.chPerModule;j++) {
  		 	myData->AnalogMeter2.config.measure_gain[i][j] = 1.000000;
  	 		 fprintf(fp, "%f ", myData->AnalogMeter2.config.measure_gain[i][j]);
		}
  	 	fprintf(fp, "\n");
	}
	fclose(fp);
	return 0;
}

