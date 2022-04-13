#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "../../INC/datastore.h"
#include "userlog.h"
#include "local_utils.h"

extern volatile S_SYSTEM_DATA	*myData;
extern volatile S_DATA_SAVE		*myPs;
extern char psName[16];
extern FILE *fpM;

void InitSharedMemory(void)
{
	int i;
	
	for(i=0; i < MAX_SIGNAL; i++) {
		myPs->signal[i] = PHASE0;
	}
}

int	Read_DataSave_Config(void)
{
    int tmp;
	char temp[20], buf[8], fileName[128];
    FILE *fp;

	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, (char *)&myData->AppControl.config.projectPath);
	strcat(fileName, "/config/parameter/DataSave_Config");
	// path/config/parameter/DataSave_Config
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "DataSave_Config file read error\n");
    	fclose(fp);
		return -1;
	}

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.monitoringData_saveFlag = (unsigned char)atoi(buf);

    fclose(fp);
	return 0;
}

void MonitoringDataSave(int meter, int time, int val)
{
	char fileNameM[128];

	if(myPs->config.monitoringData_saveFlag == PHASE0) return;

	memset(fileNameM, 0x00, sizeof(fileNameM));
	sprintf(fileNameM, "%s%d%s", "./monitoringData/m", meter, ".csv");
	if((fpM = fopen(fileNameM, "a")) == NULL) return;

	fprintf(fpM, "%d, %d\n", time, val); //time, val

	fclose(fpM);
	fpM = NULL;
}
