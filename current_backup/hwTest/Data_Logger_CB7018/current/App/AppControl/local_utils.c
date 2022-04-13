#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include "../../INC/datastore.h"
#include "userlog.h"
#include "local_utils.h"

extern volatile S_SYSTEM_DATA *myData;
extern volatile S_APP_CONTROL *myPs;
extern char psName[16];

void ProcessKill(char *psName, char *path)
{
	int rtn, i;
	char buf[32], psKill[32], cmd[128];
    FILE *fp;

	memset(cmd, 0x00, sizeof cmd);
	strcpy(cmd, "ps -ax | grep ");
	strcat(cmd, psName);
	strcat(cmd, " > ");
	strcat(cmd, path);
	strcat(cmd, "/config/tmp/");
	strcat(cmd, psName);
	strcat(cmd, "Kill.txt");
	system(cmd);
	// ps -ax | grep psName > path/config/tmp/psNameKill.txt
	
	memset(cmd, 0x00, sizeof cmd);
	strcpy(cmd, path);
	strcat(cmd, "/config/tmp/");
	strcat(cmd, psName);
	strcat(cmd, "Kill.txt");
	// path/config/tmp/psNameKill.txt
	
    if((fp = fopen(cmd, "r")) != NULL) {
		i = 0;
		while(1) {
			i++;
			memset(buf, 0x00, sizeof buf);
	    	rtn = fscanf(fp, "%s", buf);
			if(rtn > 0) {
				if(i == 1) {
					memset(psKill, 0x00, sizeof psKill);
					strcpy(psKill, "kill ");
					strcat(psKill, buf);
				} else if(i == 5) {
					memset(cmd, 0x00, sizeof cmd);
					strcpy(cmd, "./");
					strcat(cmd, psName);
					if(strcmp(cmd, buf) == 0) {
						i = 10;
						break;
					}
				}
			}
			if(i >= 5) break;
		}
		if(i == 10) {
			system(psKill);
			printf("%s %s\n", psName, psKill);
		}
	}
	fclose(fp);
}

void ProcessCheck(void)
{
	int rtn;
	long diff;
	time_t the_time;

	(void)time(&the_time);

	diff = the_time - myPs->misc.processCheckTime;
	if(diff < 10) return;

	myPs->misc.processCheckTime = the_time;
	if(myPs->signal[APP_SIG_METER1_PROCESS] == PHASE1) {
		rtn = ProcessDieCheck("Meter1", (char *)&myPs->config.projectPath);
		if(rtn < 0) { //process died
			myPs->signal[APP_SIG_METER1_PROCESS_CHECK]++;
		} else {
			myPs->signal[APP_SIG_METER1_PROCESS_CHECK] = PHASE0;
		}
		if(myPs->signal[APP_SIG_METER1_PROCESS_CHECK] >= PHASE3) {
			myPs->signal[APP_SIG_METER1_PROCESS_CHECK] = PHASE0;
			rtn = ProcessLoad("Load_Meter1", "./",
				APP_SIG_METER1_PROCESS);
		}
	}

	if(myPs->signal[APP_SIG_METER2_PROCESS] == PHASE1) {
		rtn = ProcessDieCheck("Meter2", (char *)&myPs->config.projectPath);
		if(rtn < 0) { //process died
			myPs->signal[APP_SIG_METER2_PROCESS_CHECK]++;
		} else {
			myPs->signal[APP_SIG_METER2_PROCESS_CHECK] = PHASE0;
		}
		if(myPs->signal[APP_SIG_METER2_PROCESS_CHECK] >= PHASE3) {
			myPs->signal[APP_SIG_METER2_PROCESS_CHECK] = PHASE0;
			rtn = ProcessLoad("Load_Meter2", "./",
				APP_SIG_METER2_PROCESS);
		}
	}
}

int ProcessDieCheck(char *process, char *path)
{
	int rtn;
	char buf[32], cmd[128];
    FILE *fp;

	memset(buf, 0x00, sizeof buf);
	memset(cmd, 0x00, sizeof cmd);
	strcpy(cmd, "rm -rf ");
	strcat(cmd, path);
	strcat(cmd, "/config/tmp/");
	strcat(cmd, process);
	strcat(cmd, "DieCheck.txt");
	system(cmd);
	sleep(1);
	// rm -rf projectPath/config/tmp/processDieCheck.txt

	memset(cmd, 0x00, sizeof cmd);
	strcpy(cmd, "ps -ax | grep ./");
	strcat(cmd, process);
	strcat(cmd, " > ");
	strcat(cmd, path);
	strcat(cmd, "/config/tmp/");
	strcat(cmd, process);
	strcat(cmd, "DieCheck.txt");
	system(cmd);
	sleep(1);
	// ps -ax | grep ./process > path/config/tmp/processDieCheck.txt

	memset(cmd, 0x00, sizeof cmd);
	strcpy(cmd, path);
	strcat(cmd, "/config/tmp/");
	strcat(cmd, process);
	strcat(cmd, "DieCheck.txt");
	// path/config/tmp/processDieCheck.txt

    if((fp = fopen(cmd, "r")) != NULL) {
		memset(cmd, 0x00, sizeof cmd);
		strcpy(cmd, "./");
		strcat(cmd, process);

		memset(buf, 0x00, sizeof buf);
    	rtn = fscanf(fp, "%s", buf);
		memset(buf, 0x00, sizeof buf);
    	rtn = fscanf(fp, "%s", buf);
		memset(buf, 0x00, sizeof buf);
    	rtn = fscanf(fp, "%s", buf);
		memset(buf, 0x00, sizeof buf);
    	rtn = fscanf(fp, "%s", buf);
		memset(buf, 0x00, sizeof buf);
    	rtn = fscanf(fp, "%s", buf);
		if(rtn > 0) {
			if(strcmp(buf, cmd) != 0) { //not equal
				if(strcmp(buf, "sh") != 0) { // != sh
					rtn = -1;
					userlog(DEBUG_LOG, psName, "%s died %d : %s\n",
						process, rtn, buf);
				} else {
					rtn = -2;
					userlog(DEBUG_LOG, psName,
						"process check - sh : %d\n", rtn);
				}
			} else { //equal
				rtn = 0;
			}
		} else {
			userlog(DEBUG_LOG, psName, "process check : %s\n", process);
			rtn = 0;
		}
	} else {
		rtn = -10;
		userlog(DEBUG_LOG, psName, "%s died %d : %s\n",
			process, rtn, buf);
	}
	fclose(fp);
	
	if(rtn == -1) {
		memset(cmd, 0x00, sizeof cmd);
		strcpy(cmd, "cp -rf ");
		strcat(cmd, path);
		strcat(cmd, "/config/tmp/");
		strcat(cmd, process);
		strcat(cmd, "DieCheck.txt ");
		strcat(cmd, path);
		strcat(cmd, "/config/tmp/");
		strcat(cmd, process);
		strcat(cmd, "DieCheck2.txt");
		system(cmd);
		sleep(1);
		// cp -rf projectPath/config/tmp/processDieCheck.txt
		// projectPath/config/tmp/processDieCheck2.txt
	}
	return rtn;
}

int ProcessLoad(char *process, char *path, int signalNo)
{
	int cnt = 0;
	char cmd[128];

	memset(cmd, 0x00, sizeof cmd);
	strcpy(cmd, path);
	strcat(cmd, process);
	cnt = system(cmd);
	// ./process

	while(1) {
		usleep(200000); //200mS
		if(myPs->signal[signalNo] == PHASE1) break;
		cnt++;
		printf("%s load try %d\n", process, cnt);
		if(cnt >= 10) {
			printf("%s load fail\n", process);
			return -1;
		}
	}
	return 0;
}

void CloseProcess(char *process, int signalNo)
{
	char buf[64];
	int cnt=0, rtn=0;
	
	memset(buf, 0x00, sizeof buf);
	strcpy(buf, process);
	
	while(1) {
		switch(myPs->signal[signalNo]) {
			case PHASE0:	rtn = -1;	break;
			case PHASE1:
				myPs->signal[signalNo] = PHASE2;
				break;
			case PHASE2:				
				break;
			case PHASE3:	rtn = -2;	break;
			default:		rtn = -3;	break;
		}
		if(rtn < 0) break;
		usleep(250000); //250mS
		cnt++;
		if(cnt >= 10) {
			userlog(DEBUG_LOG, psName, "ProcessKill %s\n", buf);
			ProcessKill(process, (char *)&myPs->config.projectPath);
			break;
		}
	}
}

void SaveSharedMemory(void)
{
	int	fp, rtn;
	char fileName[128];
	
	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, (char *)&myPs->config.projectPath);
	strcat(fileName, "/config/sharedMemory/sharedMemory");
	// path/config/sharedMemory/sharedMemory
	if((fp = open(fileName, O_RDWR)) < 0) {
		userlog(DEBUG_LOG, psName,
			"Can not open sharedMemory file(save)\n");
		close(fp);
		return;
	}
	rtn = write(fp, (char *)myData, sizeof(S_SYSTEM_DATA));
	if(rtn != sizeof(S_SYSTEM_DATA)) {
		userlog(DEBUG_LOG, psName, "error sharedMemory write %d\n", rtn);
	}
	close(fp);
}

int LoadSharedMemory(char *fileName, char *path)
{
	int	fp, rtn;
	char cmd[128];
	
	//sharedMemory file check : create - kjgw
	
	memset(cmd, 0x00, sizeof(cmd));
	strcpy(cmd, "touch ");
	strcat(cmd, path);
	strcat(cmd, "/config/sharedMemory/");
	strcat(cmd, fileName);
	system(cmd);
	// touch path/config/sharedMemory/fileName

	memset(cmd, 0x00, sizeof(cmd));
	strcpy(cmd, path);
	strcat(cmd, "/config/sharedMemory/");
	strcat(cmd, fileName);
	// path/config/sharedMemory/fileName

	if((fp = open(cmd, O_RDONLY)) < 0) {
		printf("Can not open %s file(load)\n", fileName);
		close(fp);
		return -1;
	}

	rtn = read(fp, (char *)myData, sizeof(S_SYSTEM_DATA));
	if(rtn != sizeof(S_SYSTEM_DATA)) {
		printf("error %s read\n", fileName);
	}
	close(fp);
	return 0;
}

void InitSharedMemory(void)
{
	int i;
	
	for(i=0; i < MAX_SIGNAL; i++) {
		myPs->signal[i] = PHASE0;
	}
	
	memset((char *)&myData->msg[0], 0x00, sizeof(S_MSG)*MAX_MSG_RING);

	memset((char *)&myData->test_val_c[0], 0x00,
		sizeof(unsigned char) * MAX_TEST_VALUE);
	memset((char *)&myData->test_val_l[0], 0x00, sizeof(long) * MAX_TEST_VALUE);
}

int Read_AppControl_Config(int bootOnStartComp)
{
    int tmp;
	char temp[128], buf[12], fileName[128];
    FILE *fp;

	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, (char *)&myPs->config.projectPath);
	strcat(fileName, "/config/parameter/AppControl_Config");
	// path/config/parameter/AppControl_Config
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName,
			"AppControl_Config file read error\n");
    	fclose(fp);
		return -1;
	}

   	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	memset(temp, 0x00, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	memcpy((char *)&myPs->config.modelName[0], (char *)&temp[0], 128);
		
	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myPs->config.moduleNo = atoi(buf);
		
	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myPs->config.bootOnStart = (unsigned char)atoi(buf);//0:exit, 1:execute
	
	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myPs->config.DebugLogFlag = (unsigned char)atoi(buf);//1:debug log file save
	
	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	//1:hs_6637, 2:web_6580, 3:hs_4020, 4:wafer_e669
	myPs->config.sbcType = (unsigned char)atoi(buf);
	
	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	//0:formation, 1:IR/OCV, 2:Aging, 3:Grader, 4:Selector, 5:OCV
	myPs->config.systemType = (unsigned int)atoi(buf);

   	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	memset(temp, 0x00, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	memcpy((char *)&myPs->config.location[0], (char *)&temp[0], 8);

	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myPs->config.osVersion = (unsigned int)atoi(buf);

	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myPs->config.debugType = atoi(buf);

    fclose(fp);
	
	if(bootOnStartComp == 1 && myPs->config.bootOnStart == 0) return -2;
	return 0;
}
