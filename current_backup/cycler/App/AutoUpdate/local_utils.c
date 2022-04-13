#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "../../INC/datastore.h"
#include "userlog.h"
#include "local_utils.h"

extern volatile S_SYSTEM_DATA *myData;
extern volatile S_AUTO_UPDATE *myPs;
extern char	psName[PROCESS_NAME_SIZE];

void Init_SystemMemory(void)
{
	int i;
	
	for(i=0; i < MAX_SIGNAL; i++) {
		myPs->signal[i] = P0;
	}

	myPs->misc.processPointer = (int)&myData;
}

int	Read_AutoUpdate_Config(void)
{
    int tmp;
	char temp[20], buf[6], fileName[128];
    FILE *fp;

	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/cycler_data/config/parameter/AutoUpdate_Config");
	// /root/cycler_data/config/parameter/AutoUpdate_Config
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "AutoUpdate_Config file read error\n");
		system("cp ../Config_backup/AutoUpdate_Config /root/cycler_data/config/parameter");
		userlog(DEBUG_LOG, psName, "AutoUpdate_Config file copy\n");
    	if((fp = fopen(fileName, "r")) == NULL) {
			return -1;
		}
	}

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.useFlag = atoi(buf);

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof temp);
    tmp = fscanf(fp, "%s", temp);
    memcpy((char *)&myPs->config.ipAddr[0], (char *)&temp[0],
		sizeof(char)*16);
	
    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.serverConnectFlag = atoi(buf);

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.updateServerSet = atoi(buf);

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.retryInterval = atoi(buf);
	
    fclose(fp);
	return 0;
}
