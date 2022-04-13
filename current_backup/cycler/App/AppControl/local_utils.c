#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <math.h>
#include "../../INC/datastore.h"
#include "userlog.h"
#include "message.h"
#include "common_utils.h"
#include "local_utils.h"

extern volatile S_SYSTEM_DATA *myData;
extern volatile S_APP_CONTROL *myPs;
extern char psName[PROCESS_NAME_SIZE];

void Kill_Process(char *psName)
{
	int rtn, i;
	char buf[32], psKill[32], cmd[128];
    FILE *fp;

	memset(cmd, 0x00, sizeof cmd);
	strcpy(cmd, "ps -ax | grep ");
	strcat(cmd, psName);
	strcat(cmd, " > /root/cycler_data/config/tmp/");
	strcat(cmd, psName);
	strcat(cmd, "Kill.txt");
	system(cmd);
	// ps -ax | grep psName > /root/cycler_data/config/tmp/psNameKill.txt
	
	memset(cmd, 0x00, sizeof cmd);
	strcpy(cmd, "/root/cycler_data/config/tmp/");
	strcat(cmd, psName);
	strcat(cmd, "Kill.txt");
	// /root/cycler_data/config/tmp/psNameKill.txt
	
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
		fclose(fp);
	}
}

void Check_Process(void)
{
	int rtn;
	long diff;
	time_t the_time;

	(void)time(&the_time);

	diff = the_time - myPs->misc.processCheckTime;
	if(diff < 0) {
		myPs->misc.processCheckTime = the_time;
		return;
	} else if(diff < 2) return;

	myPs->misc.processCheckTime = the_time;
	if(myPs->signal[APP_SIG_MAIN_CLIENT_PROCESS] == P1) {
		rtn = DieCheck_Process("MainClient");
		if(rtn < 0) { //process died
			myPs->signal[APP_SIG_MAIN_CLIENT_PROCESS_CHECK]++;
		} else {
			myPs->signal[APP_SIG_MAIN_CLIENT_PROCESS_CHECK] = P0;
		}
		if(myPs->signal[APP_SIG_MAIN_CLIENT_PROCESS_CHECK] >= P3) {
			myPs->signal[APP_SIG_MAIN_CLIENT_PROCESS_CHECK] = P0;
			rtn = Load_Process("Load_MainClient", "./",
				APP_SIG_MAIN_CLIENT_PROCESS);
		}
	}
	
	if(myPs->signal[APP_SIG_EXT_CLIENT_PROCESS] == P1) {
		rtn = DieCheck_Process("ExtClient");
		if(rtn < 0) { //process died
			myPs->signal[APP_SIG_EXT_CLIENT_PROCESS_CHECK]++;
		} else {
			myPs->signal[APP_SIG_EXT_CLIENT_PROCESS_CHECK] = P0;
		}
		if(myPs->signal[APP_SIG_EXT_CLIENT_PROCESS_CHECK] >= P3) {
			myPs->signal[APP_SIG_EXT_CLIENT_PROCESS_CHECK] = P0;
			rtn = Load_Process("Load_ExtClient", "./",
				APP_SIG_EXT_CLIENT_PROCESS);
		}
	}

	if(myPs->signal[APP_SIG_CALI_METER_PROCESS] == P1) {
		rtn = DieCheck_Process("CaliMeter");
		if(rtn < 0) { //process died
			myPs->signal[APP_SIG_CALI_METER_PROCESS_CHECK]++;
		} else {
			myPs->signal[APP_SIG_CALI_METER_PROCESS_CHECK] = P0;
		}
		if(myPs->signal[APP_SIG_CALI_METER_PROCESS_CHECK] >= P3) {
			myPs->signal[APP_SIG_CALI_METER_PROCESS_CHECK] = P0;
			rtn = Load_Process("Load_CaliMeter", "./",
				APP_SIG_CALI_METER_PROCESS);
		}
	}

	if(myPs->signal[APP_SIG_CALI_METER2_PROCESS] == P1) {
		rtn = DieCheck_Process("CaliMeter2");
		if(rtn < 0) { //process died
			myPs->signal[APP_SIG_CALI_METER2_PROCESS_CHECK]++;
		} else {
			myPs->signal[APP_SIG_CALI_METER2_PROCESS_CHECK] = P0;
		}
		if(myPs->signal[APP_SIG_CALI_METER2_PROCESS_CHECK] >= P3) {
			myPs->signal[APP_SIG_CALI_METER2_PROCESS_CHECK] = P0;
			rtn = Load_Process("Load_CaliMeter2", "./",
				APP_SIG_CALI_METER2_PROCESS);
		}
	}

	if(myPs->signal[APP_SIG_ANALOG_METER_PROCESS] == P1) {
		rtn = DieCheck_Process("AnalogMeter");
		if(rtn < 0) { //process died
			myPs->signal[APP_SIG_ANALOG_METER_PROCESS_CHECK]++;
		} else {
			myPs->signal[APP_SIG_ANALOG_METER_PROCESS_CHECK] = P0;
		}
		if(myPs->signal[APP_SIG_ANALOG_METER_PROCESS_CHECK] >= P3) {
			myPs->signal[APP_SIG_ANALOG_METER_PROCESS_CHECK] = P0;
			rtn = Load_Process("Load_AnalogMeter", "./",
				APP_SIG_ANALOG_METER_PROCESS);
		}
	}

	if(myPs->signal[APP_SIG_FADM_PROCESS] == P1) {
		rtn = DieCheck_Process("FADM");
		if(rtn < 0) { //process died
			myPs->signal[APP_SIG_FADM_PROCESS_CHECK]++;
		} else {
			myPs->signal[APP_SIG_FADM_PROCESS_CHECK] = P0;
		}
		if(myPs->signal[APP_SIG_FADM_PROCESS_CHECK] >= P3) {
			myPs->signal[APP_SIG_FADM_PROCESS_CHECK] = P0;
			rtn = Load_Process("Load_FADM", "./",
				APP_SIG_FADM_PROCESS);
		}
	}

	if(myPs->signal[APP_SIG_FND_METER_PROCESS] == P1) {
		rtn = DieCheck_Process("FndMeter");
		if(rtn < 0) { //process died
			myPs->signal[APP_SIG_FND_METER_PROCESS_CHECK]++;
		} else {
			myPs->signal[APP_SIG_FND_METER_PROCESS_CHECK] = P0;
		}
		if(myPs->signal[APP_SIG_FND_METER_PROCESS_CHECK] >= P3) {
			myPs->signal[APP_SIG_FND_METER_PROCESS_CHECK] = P0;
			rtn = Load_Process("Load_FndMeter", "./",
				APP_SIG_FND_METER_PROCESS);
		}
	}
	if(myPs->signal[APP_SIG_ANALOG_METER2_PROCESS] == P1) {
		rtn = DieCheck_Process("AnalogMeter2");
		if(rtn < 0) { //process died
			myPs->signal[APP_SIG_ANALOG_METER2_PROCESS_CHECK]++;
		} else {
			myPs->signal[APP_SIG_ANALOG_METER2_PROCESS_CHECK] = P0;
		}
		if(myPs->signal[APP_SIG_ANALOG_METER2_PROCESS_CHECK] >= P3) {
			myPs->signal[APP_SIG_ANALOG_METER2_PROCESS_CHECK] = P0;
			rtn = Load_Process("Load_AnalogMeter2", "./",
				APP_SIG_ANALOG_METER2_PROCESS);
		}
	}
	if(myPs->signal[APP_SIG_AUTO_UPDATE_PROCESS] == P1) {
		rtn = DieCheck_Process("AutoUpdate");
		if(rtn < 0) { //process died
			myPs->signal[APP_SIG_AUTO_UPDATE_PROCESS_CHECK]++;
		} else {
			myPs->signal[APP_SIG_AUTO_UPDATE_PROCESS_CHECK] = P0;
		}
		if(myPs->signal[APP_SIG_AUTO_UPDATE_PROCESS_CHECK] >= P3) {
			myPs->signal[APP_SIG_AUTO_UPDATE_PROCESS_CHECK] = P0;
			rtn = Load_Process("Load_AutoUpdate", "./",
				APP_SIG_AUTO_UPDATE_PROCESS);
		}
	}
	if(myPs->signal[APP_SIG_DAQ_CLIENT_PROCESS] == P1) {
		rtn = DieCheck_Process("DAQ_Client");
		if(rtn < 0) { //process died
			myPs->signal[APP_SIG_DAQ_CLIENT_PROCESS_CHECK]++;
		} else {
			myPs->signal[APP_SIG_DAQ_CLIENT_PROCESS_CHECK] = P0;
		}
		if(myPs->signal[APP_SIG_DAQ_CLIENT_PROCESS_CHECK] >= P3) {
			myPs->signal[APP_SIG_DAQ_CLIENT_PROCESS_CHECK] = P0;
			rtn = Load_Process("Load_DAQ_Client", "./",
				APP_SIG_DAQ_CLIENT_PROCESS);
		}
	}

	//220307 jws add
	if(myPs->signal[APP_SIG_COOLING_CONTROL_PROCESS] == P1) {
		rtn = DieCheck_Process("CoolingControl");
		if(rtn < 0) { //process died
			myPs->signal[APP_SIG_COOLING_CONTROL_PROCESS_CHECK]++;
		} else {
			myPs->signal[APP_SIG_COOLING_CONTROL_PROCESS_CHECK] = P0;
		}
		if(myPs->signal[APP_SIG_COOLING_CONTROL_PROCESS_CHECK] == P3) {
			myPs->signal[APP_SIG_COOLING_CONTROL_PROCESS_CHECK] = P0;
			rtn = Load_Process("Load_Cooling", "./",
				APP_SIG_COOLING_CONTROL_PROCESS);
		}
		
	}
}

int DieCheck_Process(char *process)
{
	int rtn;
	char buf[32], buf2[4], cmd[128];
    FILE *fp;

	memset(buf, 0x00, sizeof buf2);
	strcpy(buf, "mcts");
	
	memset(buf, 0x00, sizeof buf);
	memset(cmd, 0x00, sizeof cmd);
	strcpy(cmd, "rm -rf /root/cycler_data/config/tmp/");
	strcat(cmd, process);
	strcat(cmd, "DieCheck.txt");
	system(cmd);
//	sleep(1);
	usleep(500000); //500mS
	// rm -rf /root/cycler_data/config/tmp/processDieCheck.txt

	memset(cmd, 0x00, sizeof cmd);
	strcpy(cmd, "ps -ax | grep ./");
	strcat(cmd, process);
	strcat(cmd, " > /root/cycler_data/config/tmp/");
	strcat(cmd, process);
	strcat(cmd, "DieCheck.txt");
	system(cmd);
//	sleep(1);
	usleep(500000); //500mS
	// ps -ax | grep ./process > /root/cycler_data/config/tmp/processDieCheck.txt

	memset(cmd, 0x00, sizeof cmd);
	strcpy(cmd, "/root/cycler_data/config/tmp/");
	strcat(cmd, process);
	strcat(cmd, "DieCheck.txt");
	// /root/cycler_data/config/tmp/processDieCheck.txt

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
						"process(%s) check - sh : %d\n", process, rtn);
				}
			} else { //equal
//140707 lyh w : forceStart duplication check
				if(!(strcmp(process, "mcts"))){
					memset(buf, 0x00, sizeof buf);
					rtn = fscanf(fp, "%s", buf);
					rtn = atoi(buf);
				}else{
					rtn = 0;
				}
			}
		} else {
			userlog(DEBUG_LOG, psName, "process check : %s\n", process);
			rtn = 0;
		}
		fclose(fp);
	} else {
		rtn = -10;
		userlog(DEBUG_LOG, psName, "%s died2 %d\n", process, rtn);
	}
	
	if(rtn == -1) {
		memset(cmd, 0x00, sizeof cmd);
		strcpy(cmd, "cp -rf /root/cycler_data/config/tmp/");
		strcat(cmd, process);
		strcat(cmd, "DieCheck.txt ");
		strcat(cmd, "/root/cycler_data/config/tmp/");
		strcat(cmd, process);
		strcat(cmd, "DieCheck2.txt");
		system(cmd);
//		sleep(1);
		usleep(500000); //500mS
		// cp -rf /root/cycler_data/config/tmp/processDieCheck.txt
		// /root/cycler_data/config/tmp/processDieCheck2.txt
	}
	return rtn;
}

int Load_Process(char *process, char *path, int signalNo)
{
	char cmd[128];
	int cnt = 0;

	memset(cmd, 0x00, sizeof cmd);
	strcpy(cmd, path);
	strcat(cmd, process);
	cnt = system(cmd); // ./process

	while(1) {
		usleep(200000); //200mS
		if(myPs->signal[signalNo] == P1) break;
		cnt++;
		printf("%s load try %d\n", process, cnt);
		if(cnt >= 10) {
			printf("%s load fail\n", process);
			return -1;
		}
	}
	return 0;
}

void Close_Process(char *process, int pointer, int signalNo)
{
	char buf[64];
	int cnt=0, rtn=0;

	memset(buf, 0, sizeof buf);
	strcpy(buf, process);
	
	while(1) {
		switch(myPs->signal[signalNo]) {
			case P0: rtn = -1;	break;
			case P1: myPs->signal[signalNo] = P2; break;
			case P2:			break;
			case P3: rtn = -2;	break;
			default: rtn = -3;	break;
		}
		if(rtn < 0) break;
//		sleep(1);
		usleep(500000); //500mS
		cnt++;
		if(cnt >= 10) {
			userlog(DEBUG_LOG, psName, "ProcessKill %s\n", buf);
			if(pointer > 0) {
				Close_mbuff(pointer);
			}
			Kill_Process(process);
			break;
		}
	}
}

void Save_SystemMemory(void)
{
	int	fp,rtn;
	char fileName[128],cmd[128];
	FILE *fp1;
	
	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/cycler_data/config/sharedMemory/systemMemory");
	// /root/cycler_data/config/sharedMemory/systemMemory
	if((fp = open(fileName, O_RDWR)) < 0) {
		userlog(DEBUG_LOG, psName,
			"Can not open System Memory file(save)\n");
		close(fp);
		return;
	}
	rtn = write(fp, (char *)myData, sizeof(S_SYSTEM_DATA));
	if(rtn != sizeof(S_SYSTEM_DATA)) {
		userlog(DEBUG_LOG, psName, "error System Memory write %d\n", rtn);
	} else { //100913 kji save memory write 1
		memset(fileName, 0x00, sizeof(fileName));
		strcpy(fileName, "/root/cycler_data/config/parameter/systemMemory");
		// /root/cycler_data/config/parameter/systemMemory
		if((fp1 = fopen(fileName,"w" )) < 0) {
			userlog(DEBUG_LOG, psName,
				"Can not open system Memory saveFlag file\n");
			memset(cmd, 0x00, sizeof(cmd));
			strcpy(cmd, "rm -rf /root/cycler_data/config/parameter/systemMemory");
			system(cmd);
			return;
		} else {
			fprintf(fp1,"1\n");
			fclose(fp1);
		}
	}
	close(fp);
}

int Read_SystemMemory(char *fileName)
{
	int	fp, rtn,tmp,saveFlag;
	char cmd[128],fileName1[128],temp[32];
	FILE *fp1;

	memset(fileName1, 0x00, sizeof(fileName1));
	strcpy(fileName1, "/root/cycler_data/config/parameter/systemMemory");
	// /root/cyclerr_data/config/parameter/systemMemory
	if((fp1 = fopen(fileName1,"r" )) == NULL) {
		printf("Can not open system Memory saveFlag file\n");
	} else {
		tmp = fscanf(fp1,"%s",temp);
		saveFlag = atoi(temp);
		//100913 kji w systemMemory not save systemMemory Initialize
		myPs->misc.saveFlag = 0;
		if(saveFlag == 0 || saveFlag == 2){
			memset(cmd, 0x00, sizeof(cmd));
			strcpy(cmd, "rm -rf /root/cycler_data/config/sharedMemory/");
			strcat(cmd, fileName);
			system(cmd);
			myPs->misc.saveFlag = 1;
			// rm -rf /root/cycler_data/config/sharedMemory/fileName
			printf("system Memory not save system Memory Init\n");
		}
		fclose(fp1);
	}
	
	if((fp1 = fopen(fileName1,"w" )) < 0) {
		printf("Can not open system Memory saveFlag file\n");
	} else {
		fprintf(fp1,"0\n");
		fclose(fp1);
	}

	memset(cmd, 0x00, sizeof(cmd));
	strcpy(cmd, "touch /root/cycler_data/config/sharedMemory/");
	strcat(cmd, fileName);
	system(cmd);
	// touch /root/cycler_data/config/sharedMemory/fileName

	memset(cmd, 0x00, sizeof(cmd));
	strcpy(cmd, "/root/cycler_data/config/sharedMemory/");
	strcat(cmd, fileName);
	// /root/cycler_data/config/sharedMemory/fileName

	if((fp = open(cmd, O_RDONLY)) < 0) {
		printf("Can not open %s file(load)\n", fileName);
		close(fp);
		return -1;
	}

	rtn = read(fp, (char *)myData, sizeof(S_SYSTEM_DATA));
	if(rtn != sizeof(S_SYSTEM_DATA)) {
		printf("error %s read\n", fileName);
	} else {
		system("rm -rf /root/cycler_data/config/sharedMemory/systemMemory");
		system("touch /root/cycler_data/config/sharedMemory/systemMemory");
	}
	close(fp);
	return 0;
}

void Init_SystemMemory(void)
{
	int i;

	memset((char *)&myPs->config, 0, sizeof(S_APP_CONFIG));
	
	memset((char *)&psName[0], 0, PROCESS_NAME_SIZE);
	strcpy(psName, "App");

	for(i=0; i < MAX_SIGNAL; i++) {
		myPs->signal[i] = P0;
	}
	myPs->misc.processPointer = (int)&myData;
	
	memset((char *)&myData->msg[0], 0x00, sizeof(S_MSG)*MAX_MSG_RING);
	memset((char *)&myData->dio, 0x00, sizeof(S_DIGITAL_INOUT));

	memset((char *)&myData->test_val_c[0], 0x00, sizeof(char) * MAX_TEST_VALUE);
	memset((char *)&myData->test_val_c1[0], 0x00, sizeof(char) * MAX_TEST_VALUE);
	memset((char *)&myData->test_val_l[0], 0x00, sizeof(long) * MAX_TEST_VALUE);

	myData->mData.misc.timer_1sec = 0;
	myData->mData.misc.timer_mSec = 0;
}

int Read_AppControl_Config(int bootOnStartComp)
{
    int tmp;
	char temp[128], buf[12], fileName[128];
    FILE *fp;

	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/cycler_data/config/parameter/AppControl_Config");
	// /root/cycler_data/config/parameter/AppControl_Config
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "AppControl_Config file read error\n");
		system("cp ../Config_backup/AppControl_Config /root/cycler_data/config/parameter/");
		userlog(DEBUG_LOG, psName, "AppControl_Config file copy\n");
    	if((fp = fopen(fileName, "r")) == NULL) {
			return -1;
		}
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
	myPs->config.bootOnStart = (unsigned char)atoi(buf);
	//0:exit, 1:execute 2:duplication check
	
	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myPs->config.DebugLogFlag = (unsigned char)atoi(buf);//1:debug log file save
	
	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	//1:hs_6637, 2:web_6580, 3:hs_4020, 4:wafer_e669 5:mark_533 6.em_104
	myPs->config.sbcType = (unsigned char)atoi(buf);
	
	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	//0:formation, 1:IR/OCV, 2:Aging, 3:Grader, 4:Selector, 5:OCV, 6:Cycler(Linear), 7:Cycler(PCU), 8:Cycler(CAN)
	myPs->config.systemType = (unsigned int)atoi(buf);

	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	//0:kernel v.2.0, 1:kernel v.2.2, 2:kernel v.2.4, 3:kernel v.2.6
	// :RTLinux v.1.3  :RTLinux v.2.0  :RTLinux v.2.3   RTLinux v.3.2
	myPs->config.osVersion = (unsigned int)atoi(buf);

	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myPs->config.debugType = atoi(buf);

	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myPs->config.versionNo = atoi(buf);

    fclose(fp);
	
	if(bootOnStartComp == 1 && myPs->config.bootOnStart == 0) return -2;
	
#if CYCLER_TYPE == DIGITAL_CYC
	if(myPs->config.systemType != CYCLER_PCU) {
		userlog(DEBUG_LOG, psName, "AppControl_Config File systemType check please...1\n");
		return -3;
	}
#else
	if(myPs->config.systemType == CYCLER_PCU) {
		userlog(DEBUG_LOG, psName, "AppControl_Config File systemType check please...2\n");
		return -4;
	}
#endif
#if CYCLER_TYPE == CAN_CYC
	if(myPs->config.systemType != CYCLER_CAN) {
		userlog(DEBUG_LOG, psName, "AppControl_Config File systemType check please...3\n");
		return -5;
	}
#endif
				
	return 0;
}

int Read_mControl_Config(void)
{
	char temp[32], buf[12], fileName[128];
    int tmp, i, j;
    FILE *fp;

	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/cycler_data/config/parameter/mControl_Config");
	// /root/cycler_data/config/parameter/mControl_Config
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "mControl_Config file read error\n");
		system("cp ../Config_backup/mControl_Config /root/cycler_data/config/parameter/");
		userlog(DEBUG_LOG, psName, "mControl_Config file copy\n");
    	if((fp = fopen(fileName, "r")) == NULL) {
			return -1;
		}
	}

	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->mData.config.installedBd = atoi(buf);
		
	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->mData.config.installedCh = atoi(buf);
		
	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->mData.config.chPerBd = atoi(buf);
		
	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->mData.config.rangeV = (short int)atoi(buf);
		
	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->mData.config.rangeI = (short int)atoi(buf);
		
	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	for(i=0; i < MAX_RANGE; i++) {
		memset(buf, 0x00, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->mData.config.maxVoltage[i] = atol(buf);
	}

	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	for(i=0; i < MAX_RANGE; i++) {
		memset(buf, 0x00, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->mData.config.minVoltage[i] = atol(buf);
	}

	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	for(i=0; i < MAX_RANGE; i++) {
		memset(buf, 0x00, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->mData.config.maxCurrent[i] = atol(buf);
	}
		
	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	for(i=0; i < MAX_RANGE; i++) {
		memset(buf, 0x00, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->mData.config.minCurrent[i] = atol(buf);
	}
		
	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	//0:uV, 1:nV
	myData->mData.config.ratioVoltage = (unsigned char)atoi(buf);

	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	//0:uA, 1:nA
	myData->mData.config.ratioCurrent = (unsigned char)atoi(buf);

	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	//0:unuse, 1:use
	myData->mData.config.watchdogFlag = (unsigned char)atoi(buf);

	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	//0:normal adc, 1:common adc
	myData->mData.config.ADC_type = (unsigned char)atoi(buf);

	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	//0:L_5V_3A, 1:L_6V_6A, 2:L_5V_2A, 3:L_5V_200A, 4:L_5V_10mA
	//5:L_5V_5A, 6:L_5V_30A, 7:L_2V_60A, 8:L_2V_100A
	//9:L_5V_5A_2, 10:L_5V_50A
	//100:S_5V_200A
	myData->mData.config.hwSpec = (unsigned char)atoi(buf);

	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	//0:CAPACITY_AMPARE_HOURS, 1:CAPACITY_CAPACITANCE
	myData->mData.config.capacityType = (unsigned char)atoi(buf);

	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->mData.config.totalJig = (unsigned int)atoi(buf);

	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	for(i=0; i < 8; i++) {
		memset(buf, 0x00, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->mData.config.bdInJig[i] = (unsigned int)atoi(buf);
	}

	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	for(i=8; i < 16; i++) {
		memset(buf, 0x00, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->mData.config.bdInJig[i] = (unsigned int)atoi(buf);
	}
	
	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	for(i=0; i < MAX_RANGE; i++) {
		memset(buf, 0x00, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->mData.config.shunt[i] = atof(buf);
	}

	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	for(i=0; i < MAX_RANGE; i++) {
		memset(buf, 0x00, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->mData.config.gain[i] = atof(buf);
	}

	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->mData.config.adAmp = atof(buf);
    
	memset(buf, 0x00, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->mData.config.voltageAmp = atof(buf);
	
	memset(buf, 0x00, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->mData.config.currentAmp = atof(buf);

	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->mData.config.rt_scan_type = (unsigned char)atoi(buf);

	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->mData.config.installedTemp = (unsigned short)atoi(buf);

	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->mData.config.installedAuxV = (unsigned short)atoi(buf);


	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->mData.config.installedCAN = (unsigned short)atoi(buf);

	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->mData.config.auto_v_cali = (unsigned char)atoi(buf);
	if(myPs->config.versionNo < 20091124) 
		myData->mData.config.auto_v_cali = P1;

//20100128 kji add
	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->mData.config.parallelMode = (unsigned char)atoi(buf);

//20100723 kji add
	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->mData.config.DAC_type = (unsigned char)atoi(buf);

	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->mData.config.ADC_num = (unsigned char)atoi(buf);
	
	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->mData.config.AD_offset = (float)atof(buf);

	if(myData->mData.config.AD_offset > 0.0005)
		myData->mData.config.AD_offset = 0;
	if(myData->mData.config.AD_offset < -0.0005)
		myData->mData.config.AD_offset = 0;
	
	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->mData.config.SoftFeedbackFlag = (unsigned char)atoi(buf);

	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->mData.config.MainBdType = (unsigned char)atoi(buf);
	
	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->mData.config.FadBdUse = (unsigned char)atoi(buf);
	//2011. 5.21. pjy add
	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	for(i = 0; i < 5; i++){
		tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
		for( j =0; j< 4; j++){
			memset(buf, 0x00, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->mData.config.range[i][j] = (unsigned char)atoi(buf);
		}
	}
	fclose(fp);
	
	if(myPs->config.versionNo < 20110101) {
		myData->mData.config.SoftFeedbackFlag = 1;
		myData->mData.config.AD_offset = 0.0002;
	}

#if CYCLER_TYPE == LINEAR_CYC || CYCLER_TYPE == CAN_CYC
	if(myData->mData.config.rt_scan_type == RT_SCAN_PERIOD_50mS){
		if(myData->mData.config.chPerBd > 32){
			userlog(DEBUG_LOG, psName, 
			"error rt_scan_type : 50mS [It must be chPerBd less then 32]\n");
			return -1;
		}
	}
	if(myData->mData.config.rt_scan_type == RT_SCAN_PERIOD_25mS){
		if(myData->mData.config.chPerBd > 16){
			userlog(DEBUG_LOG, psName, 
			"error rt_scan_type : 25mS [It must be chPerBd less then 16]\n");
			return -1;
		}
	}
	if(myData->mData.config.rt_scan_type == RT_SCAN_PERIOD_20mS){
		if(myData->mData.config.chPerBd > 8){
			userlog(DEBUG_LOG, psName, 
			"error rt_scan_type : 20mS [It must be chPerBd less then 8]\n");
			return -1;
		}
	}
	if(myData->mData.config.rt_scan_type == RT_SCAN_PERIOD_10mS){
		if(myData->mData.config.chPerBd > 8){
			userlog(DEBUG_LOG, psName, 
			"error rt_scan_type : 10mS [It must be chPerBd less then 8]\n");
			return -1;
		}
	}
	if(myData->mData.config.hwSpec >= 200) {
		userlog(DEBUG_LOG, psName, 
			"mControl file hwSpec check please...\n");
		return -1;
	}
#else
	if(myData->mData.config.rt_scan_type == RT_SCAN_PERIOD_10mS){
		if(myData->mData.config.chPerBd > 32){
			userlog(DEBUG_LOG, psName, 
			"error rt_scan_type : 10mS [It must be chPerBd less then 32]\n");
			return -1;
		}
	}

	if(myData->mData.config.hwSpec < 200) {
		userlog(DEBUG_LOG, psName, 
			"mControl file hwSpec check please...\n");
		return -1;
	}
#endif

	return 0;
}

int Read_DIO_Config(void)
{
	if(Read_DIO_Setting() < 0) return -1;
	if(myPs->config.versionNo >= 20090602) {
		if(Read_DIN_USE_FLAG() < 0) return -3;
		if(Read_DOUT_USE_FLAG() < 0) return -4;
#if CYCLER_TYPE == DIGITAL_CYC
		if(Read_PCU_INV_USE_FLAG() < 0) return -5;
#endif
	} else {
		if(Read_DIO_SignalNo() < 0) return -2;
	}
	return 0;
}

int Read_DIO_Setting(void)
{
    int tmp;
	char temp[24], buf[12], fileName[128];
    FILE *fp;
	
	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/cycler_data/config/parameter/DIO_Setting");
	// /root/cycler_data/config/parameter/DIO_Setting
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "DIO_Setting file read error\n");
		system("cp ../Config_backup/DIO_Setting /root/cycler_data/config/parameter/");
		userlog(DEBUG_LOG, psName, "DIO_Setting file copy\n");
    	if((fp = fopen(fileName, "r")) == NULL) {
			return -1;
		}
	}

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myData->dio.config.sensCount = atoi(buf);

	tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myData->dio.config.dioDelay = (unsigned long)atol(buf);

	tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myData->dio.config.powerSwitchTimeout = (unsigned long)atol(buf);

	tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myData->dio.config.resetSwitchTimeout = (unsigned long)atol(buf);

	tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myData->dio.config.powerFailTimeout1 = (unsigned long)atol(buf);

	tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myData->dio.config.powerFailTimeout2 = (unsigned long)atol(buf);

	tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myData->dio.config.upsBatteryFailTimeout = (unsigned long)atol(buf);
	
	tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
	//0:unuse, 1:use
    myData->dio.config.dio_Control_Flag = (unsigned char)atoi(buf);

    fclose(fp);
	return 0;
}

int	Read_DIO_SignalNo(void)
{
    int tmp, i;
	char temp[20], buf[6], fileName[128];
    FILE *fp;

	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/cycler_data/config/parameter/DIN_SignalNo");
	// /root/cycler_data/config/parameter/DIN_SignalNo
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "DIN_SignalNo file read error\n");
		system("cp ../Config_backup/DIN_SignalNo /root/cycler_data/config/parameter");
		userlog(DEBUG_LOG, psName, "DIN_SignalNo file copy\n");
    	if((fp = fopen(fileName, "r")) == NULL) {
			return -1;
		}
	}

	for(i=0; i < MAX_DIGITAL_INPUT; i++) {
    	tmp = fscanf(fp, "%s", temp);
		if(tmp <= 0) break;
		tmp = fscanf(fp, "%s", temp);
		memset(buf, 0x00, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
    	myData->dio.config.inSignalNo[i] = atoi(buf)-1;
	}
    fclose(fp);
		
	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/cycler_data/config/parameter/DOUT_SignalNo");
	// /root/cycler_data/config/parameter/DOUT_SignalNo
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "DOUT_SignalNo file read error\n");
		system("cp ../Config_backup/DOUT_SignalNo /root/cycler_data/config/parameter");
		userlog(DEBUG_LOG, psName, "DOUT_SignalNo file copy\n");
    	if((fp = fopen(fileName, "r")) == NULL) {
			return -2;
		}
	} else {
		system("rm -rf ../Config_backup/DOUT_SignalNo");
		system("cp /root/cycler_data/config/parameter/DOUT_SignalNo ../Config_backup");
	}

	for(i=0; i < MAX_DIGITAL_OUTPUT; i++) {
    	tmp = fscanf(fp, "%s", temp);
		if(tmp <= 0) break;
		tmp = fscanf(fp, "%s", temp);
		memset(buf, 0x00, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
    	myData->dio.config.outSignalNo[i] = atoi(buf)-1;
	}
    fclose(fp);
	return 0;
}

int Read_DOUT_USE_FLAG(void)
{
	char temp[32], buf[12], fileName[128];
    int i, tmp;
    FILE *fp;

	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/cycler_data/config/parameter/DOUT_USE_FLAG");
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "DOUT_USE_FLAG file read error\n");
		system("cp ../Config_backup/DOUT_USE_FLAG /root/cycler_data/config/parameter");
		userlog(DEBUG_LOG, psName, "DOUT_USE_FLAG file copy\n");
    	if((fp = fopen(fileName, "r")) == NULL) {
			return -1;
		}
	}

	tmp = fscanf(fp, "%s", temp); tmp = fscanf(fp, "%s", temp);
	tmp = fscanf(fp, "%s", temp); tmp = fscanf(fp, "%s", temp);
	tmp = fscanf(fp, "%s", temp);

	for(i=0; i < MAX_DIGITAL_OUTPUT; i++) {
		tmp = fscanf(fp, "%s", temp); //Addr
		tmp = fscanf(fp, "%s", temp); //No
		tmp = fscanf(fp, "%s", temp); //Bit
																			
		memset(buf, 0, sizeof buf);
	   	tmp = fscanf(fp, "%s", buf);
		myData->dio.dout.outUseFlag[i] 
			= (unsigned char)atoi(buf);
	
		tmp = fscanf(fp, "%s", temp); //Description
	}

    fclose(fp);
	return 0;
}


int Read_DIN_USE_FLAG(void)
{
	char temp[32], buf[12], fileName[128];
    int i, tmp;
    FILE *fp;

	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/cycler_data/config/parameter/DIN_USE_FLAG");
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "DIN_USE_FLAG file read error\n");
		system("cp ../Config_backup/DIN_USE_FLAG /root/cycler_data/config/parameter");
		userlog(DEBUG_LOG, psName, "DIN_USE_FLAG file copy\n");
    	if((fp = fopen(fileName, "r")) == NULL) {
			return -1;
		}
	}

	tmp = fscanf(fp, "%s", temp); tmp = fscanf(fp, "%s", temp);
	tmp = fscanf(fp, "%s", temp); tmp = fscanf(fp, "%s", temp);
	tmp = fscanf(fp, "%s", temp); tmp = fscanf(fp, "%s", temp);

	for(i=0; i < MAX_DIGITAL_INPUT; i++) {
		tmp = fscanf(fp, "%s", temp); //Addr
		tmp = fscanf(fp, "%s", temp); //No
		tmp = fscanf(fp, "%s", temp); //Bit
																			
		memset(buf, 0, sizeof buf);
	   	tmp = fscanf(fp, "%s", buf);
		myData->dio.din.inUseFlag[i] 
			= (unsigned char)atoi(buf);
	
		memset(buf, 0, sizeof buf);
  		tmp = fscanf(fp, "%s", buf);
		myData->dio.din.inActiveType[i] 
			= (unsigned char)atoi(buf);
	
		tmp = fscanf(fp, "%s", temp); //Description
	}

    fclose(fp);
	return 0;
}

int Read_PCU_INV_USE_FLAG(void)
{	//180626 lyhw
	char temp[32], buf[12], fileName[128];
    int i, tmp;
    FILE *fp;

	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/cycler_data/config/parameter/PCU_INV_USE_FLAG");
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "PCU_INV_USE_FLAG file read error\n");
		system("../Config_backup/PCU_INV_USE_FLAG /root/cycler_data/config/parameter");
		userlog(DEBUG_LOG, psName, "PCU_INV_USE_FLAG file copy\n");
		if((fp = fopen(fileName, "r")) == NULL) {
			return -1;
		}
	}
	tmp = fscanf(fp, "%s", temp); tmp = fscanf(fp, "%s", temp);
	tmp = fscanf(fp, "%s", temp); tmp = fscanf(fp, "%s", temp);

	for(i=0; i < MAX_INV_GROUP_NUM; i++) {
		tmp = fscanf(fp, "%s", temp); //No
																			
		memset(buf, 0, sizeof buf);
	   	tmp = fscanf(fp, "%s", buf);
		myData->dio.din.pcu_inUseFlag[i] 
			= (unsigned char)atoi(buf);
	
		memset(buf, 0, sizeof buf);
  		tmp = fscanf(fp, "%s", buf);
		myData->dio.din.pcu_inActiveType[i] 
			= (unsigned char)atoi(buf);
	
		tmp = fscanf(fp, "%s", temp); //Description
	}

    fclose(fp);
	return 0;
}

int Read_Calibration_Config(void)
{
    if(Read_Calibration_Data() < 0) return -2;
	return 0;
}

int Read_Calibration_Data(void)
{
	int bd;
	
	for(bd=0; bd < myData->mData.config.installedBd; bd++) {
		if(Read_BdCaliData(bd) < 0) return -1;
		if(myData->mData.config.FadBdUse == P1){
			if(Read_BdCaliData_FAD(bd) < 0) return -2;
		}
		if(myData->mData.config.function[F_I_OFFSET_CALI] == P1){
			if(Read_BdCaliData_I_Offset(bd) < 0) return -3;
		}
	}

	if(myData->mData.config.function[F_I_OFFSET_CALI] == P1){
		if(Read_CaliData_Line_Drop() < 0) return -5;
	}

	if(myData->mData.config.FadBdUse == P1){
		if(Read_FadOffsetData() < 0) return -6;
	}
	return 0;
}

int Read_BdCaliData(int bd)
{
	int		rtn, tmp, ch, type, range, point, read_ch;
	char	fileName[128], temp[64], buf[64];
	S_CALI_DATA	data[MAX_CH_PER_BD];
	FILE	*fp;

	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/cycler_data/config/caliData/CALI_BD");
	memset(temp, 0x00, sizeof temp);
//190624 oys modify start : Support CALI_BD16 file
	if(bd < 9){
		temp[0] = (char)(49+bd);
	} else {
		temp[0] = (char)(49);
		temp[1] = (char)(39+bd);
	}
//190624 oys modift end
	strcat(fileName, temp);
	// /root/cycler_data/config/caliData/CALI_BD#

	memset(data, 0, sizeof(S_CALI_DATA)*MAX_CH_PER_BD);

	if(myData->mData.config.chPerBd > 32){
		read_ch = 64;
	} else if(myData->mData.config.chPerBd > 16){
		read_ch = 32;
	}else{
		read_ch = 16;
	}
	//190617 oys add start
	if(read_ch > MAX_CH_PER_BD)
		read_ch = MAX_CH_PER_BD;
	//add end


	if((fp = fopen(fileName, "r")) != NULL) {
		rtn = 0;
		tmp = fscanf(fp, "%s", temp); tmp = fscanf(fp, "%s", temp);
		tmp = fscanf(fp, "%s", temp);
		
		for(type=0; type < MAX_TYPE; type++) {
			tmp = fscanf(fp, "%s", temp); //voltage, current
			for(range=0; range < MAX_RANGE; range++) {
				tmp = fscanf(fp, "%s", temp); //range1, 2, 3, 4
//				for(ch=0; ch < MAX_CH_PER_BD; ch++) {
				for(ch=0; ch < read_ch; ch++) {
					tmp = fscanf(fp, "%s", temp); //ch

					tmp = fscanf(fp, "%s", temp); //setPointNum
					memset(buf, 0x00, sizeof(buf));
					tmp = fscanf(fp, "%s", buf);
					data[ch].point[type][range].setPointNum
						= (unsigned char)atoi(buf);
#if CYCLER_TYPE == DIGITAL_CYC
					if(data[ch].point[type][range].setPointNum <= 0){
						data[ch].point[type][range].setPointNum = 1;
					}
					if(data[ch].point[type][range]
									.setPointNum > MAX_CALI_POINT){
						data[ch].point[type][range].setPointNum=MAX_CALI_POINT;
					}
					myData->cali.tmpCond[bd][ch].point[type][range].setPointNum
						= data[ch].point[type][range].setPointNum;
#endif
					tmp = fscanf(fp, "%s", temp); //setPoint
					for(point=0; point < MAX_CALI_POINT; point++) {
						memset(buf, 0x00, sizeof(buf));
						tmp = fscanf(fp, "%s", buf);
#if CYCLER_TYPE == LINEAR_CYC || CYCLER_TYPE == CAN_CYC
						data[ch].point[type][range].setPoint[point]	= atol(buf);
						if(data[ch].point[type][range].setPoint[point] > 0){
							data[ch].point[type][range].charge_pointNum++;
						}
						if(data[ch].point[type][range].setPoint[point] < 0){
							data[ch].point[type][range].discharge_pointNum++;
						}
#else
						if(data[ch].point[type][range].setPointNum <= point){
							data[ch].point[type][range].setPoint[point] = 0;
							myData->cali.tmpCond[bd][ch].
									point[type][range].setPoint[point]
								= data[ch].point[type][range].setPoint[point];
						}else{
							data[ch].point[type][range].setPoint[point]
								= atol(buf);
							myData->cali.tmpCond[bd][ch].
									point[type][range].setPoint[point]
								= data[ch].point[type][range].setPoint[point];
						}
						if(data[ch].point[type][range].setPoint[point] > 0){
							data[ch].point[type][range].charge_pointNum++;
						}
						if(data[ch].point[type][range].setPoint[point] < 0){
							data[ch].point[type][range].discharge_pointNum++;
						}
#endif
					}
#if CYCLER_TYPE == DIGITAL_CYC
					myData->cali.tmpCond[bd][ch]
							.point[type][range].discharge_pointNum
						= data[ch].point[type][range].discharge_pointNum;
#endif

					tmp = fscanf(fp, "%s", temp); //checkPointNum
					memset(buf, 0x00, sizeof(buf));
					tmp = fscanf(fp, "%s", buf);
					data[ch].point[type][range].checkPointNum
						= (unsigned char)atoi(buf);
#if CYCLER_TYPE == DIGITAL_CYC
					if(data[ch].point[type][range].checkPointNum <= 0){
						data[ch].point[type][range].checkPointNum = 1;
					}
					if(data[ch].point[type][range]
									.checkPointNum > MAX_CALI_POINT){
					data[ch].point[type][range].checkPointNum = MAX_CALI_POINT;
					}
					myData->cali.tmpCond[bd][ch]
							.point[type][range].checkPointNum
						= data[ch].point[type][range].checkPointNum;	
#endif
					tmp = fscanf(fp, "%s", temp); //checkPoint
					for(point=0; point < MAX_CALI_POINT; point++) {
						memset(buf, 0x00, sizeof(buf));
						tmp = fscanf(fp, "%s", buf);
#if CYCLER_TYPE == LINEAR_CYC || CYCLER_TYPE == CAN_CYC
						data[ch].point[type][range].checkPoint[point]
							= atol(buf);
#else
						if(data[ch].point[type][range].checkPointNum <= point){
							data[ch].point[type][range].checkPoint[point] = 0;
							myData->cali.tmpCond[bd][ch]
								.point[type][range].checkPoint[point]
							= data[ch].point[type][range].checkPoint[point];
						}else{
							data[ch].point[type][range].checkPoint[point]
								= atol(buf);
							myData->cali.tmpCond[bd][ch]
									.point[type][range].checkPoint[point]
								= data[ch].point[type][range].checkPoint[point];
						}
#endif
					}

					tmp = fscanf(fp, "%s", temp); //set_ad
					for(point=0; point < MAX_CALI_POINT; point++) {
						memset(buf, 0x00, sizeof(buf));
						tmp = fscanf(fp, "%s", buf);
#if CYCLER_TYPE == LINEAR_CYC || CYCLER_TYPE == CAN_CYC
						data[ch].set_ad[type][range][point]
							= (double)atof(buf);
#else
						if(data[ch].point[type][range].setPointNum <= point){
							data[ch].set_ad[type][range][point] = 0;
							myData->cali.tmpData[bd][ch]
									.set_ad[type][range][point]
							 = data[ch].set_ad[type][range][point];
						}else{
							data[ch].set_ad[type][range][point] 
												= (double)atof(buf);
							myData->cali.tmpData[bd][ch]
									.set_ad[type][range][point]
							 = data[ch].set_ad[type][range][point];
						}
#endif
					}

					tmp = fscanf(fp, "%s", temp); //set_meter
					for(point=0; point < MAX_CALI_POINT; point++) {
						memset(buf, 0x00, sizeof(buf));
						tmp = fscanf(fp, "%s", buf);
#if CYCLER_TYPE == LINEAR_CYC || CYCLER_TYPE == CAN_CYC
						data[ch].set_meter[type][range][point] = (double)atof(buf);
#else
						if(data[ch].point[type][range].setPointNum <= point){
							data[ch].set_meter[type][range][point] = 0;
							myData->cali.tmpData[bd][ch]
								.set_meter[type][range][point]
								= data[ch].set_meter[type][range][point];
						}else{
							data[ch].set_meter[type][range][point] 
									= (double)atof(buf);
							myData->cali.tmpData[bd][ch]
									.set_meter[type][range][point]
								= data[ch].set_meter[type][range][point];
						}
#endif
					}

					tmp = fscanf(fp, "%s", temp); //check_ad
					for(point=0; point < MAX_CALI_POINT; point++) {
						memset(buf, 0x00, sizeof(buf));
						tmp = fscanf(fp, "%s", buf);
#if CYCLER_TYPE == LINEAR_CYC || CYCLER_TYPE == CAN_CYC
						data[ch].check_ad[type][range][point] = (double)atof(buf);
#else
						if(data[ch].point[type][range].checkPointNum <= point){
							data[ch].check_ad[type][range][point] = 0;
							myData->cali.tmpData[bd][ch]
									.check_ad[type][range][point]
								 = data[ch].check_ad[type][range][point];
						}else{
							data[ch].check_ad[type][range][point] = (double)atof(buf);
							myData->cali.tmpData[bd][ch]
									.check_ad[type][range][point]
								 = data[ch].check_ad[type][range][point];
						}
#endif
					}

					tmp = fscanf(fp, "%s", temp); //check_meter
					for(point=0; point < MAX_CALI_POINT; point++) {
						memset(buf, 0x00, sizeof(buf));
						tmp = fscanf(fp, "%s", buf);
#if CYCLER_TYPE == LINEAR_CYC || CYCLER_TYPE == CAN_CYC
						data[ch].check_meter[type][range][point]
							= (double)atof(buf);
#else
						if(data[ch].point[type][range].checkPointNum <= point){
							data[ch].check_meter[type][range][point] =0;
							myData->cali.tmpData[bd][ch]
									.check_meter[type][range][point]
						 		= data[ch].check_meter[type][range][point];
						}else{
							data[ch].check_meter[type][range][point] 
								= (double)atof(buf);
							myData->cali.tmpData[bd][ch]
									.check_meter[type][range][point]
								 = data[ch].check_meter[type][range][point];
						}
#endif
					}

					tmp = fscanf(fp, "%s", temp); //da_a
					for(point=0; point < MAX_CALI_POINT-1; point++) {
						memset(buf, 0x00, sizeof(buf));
						tmp = fscanf(fp, "%s", buf);
#if CYCLER_TYPE == LINEAR_CYC || CYCLER_TYPE == CAN_CYC
						data[ch].DA_A[type][range][point] = (double)atof(buf);
#else
						if(data[ch].point[type][range].setPointNum-1 <= point){
							data[ch].DA_A[type][range][point] = 0;
							myData->cali.tmpData[bd][ch]
									.DA_A[type][range][point]
								 = data[ch].DA_A[type][range][point];
						}else{
							data[ch].DA_A[type][range][point] 
											= (double)atof(buf);
							myData->cali.tmpData[bd][ch]
									.DA_A[type][range][point]
								 = data[ch].DA_A[type][range][point];
						}
#endif
					}

					tmp = fscanf(fp, "%s", temp); //da_b
					for(point=0; point < MAX_CALI_POINT-1; point++) {
						memset(buf, 0x00, sizeof(buf));
						tmp = fscanf(fp, "%s", buf);
#if CYCLER_TYPE == LINEAR_CYC || CYCLER_TYPE == CAN_CYC
						data[ch].DA_B[type][range][point] = (double)atof(buf);
#else
						if(data[ch].point[type][range].setPointNum-1 <= point){
							data[ch].DA_B[type][range][point] = 0;
							myData->cali.tmpData[bd][ch]
									.DA_B[type][range][point]
							 	= data[ch].DA_B[type][range][point];
						}else{
							data[ch].DA_B[type][range][point] 
											= (double)atof(buf);
							myData->cali.tmpData[bd][ch]
										.DA_B[type][range][point]
							 	= data[ch].DA_B[type][range][point];
						}						
#endif
					}
		
					tmp = fscanf(fp, "%s", temp); //ad_a
					for(point=0; point < MAX_CALI_POINT-1; point++) {
						memset(buf, 0x00, sizeof(buf));
						tmp = fscanf(fp, "%s", buf);
#if CYCLER_TYPE == LINEAR_CYC || CYCLER_TYPE == CAN_CYC
						data[ch].AD_A[type][range][point] = (double)atof(buf);
#else
						if(data[ch].point[type][range].setPointNum-1 <= point){
							data[ch].AD_A[type][range][point] = 0;
							myData->cali.tmpData[bd][ch]
									.AD_A[type][range][point]
								 = data[ch].AD_A[type][range][point];
						}else{
							data[ch].AD_A[type][range][point] 
											= (double)atof(buf);
							myData->cali.tmpData[bd][ch]
									.AD_A[type][range][point]
								 = data[ch].AD_A[type][range][point];
						}
#endif
					}

					tmp = fscanf(fp, "%s", temp); //ad_b
					for(point=0; point < MAX_CALI_POINT-1; point++) {
						memset(buf, 0x00, sizeof(buf));
						tmp = fscanf(fp, "%s", buf);
#if CYCLER_TYPE == LINEAR_CYC || CYCLER_TYPE == CAN_CYC
						data[ch].AD_B[type][range][point] = (double)atof(buf);
#else
						if(data[ch].point[type][range].setPointNum-1 <= point){
							data[ch].AD_B[type][range][point] = 0;
							myData->cali.tmpData[bd][ch]
									.AD_B[type][range][point]
								 = data[ch].AD_B[type][range][point];
						}else{
							data[ch].AD_B[type][range][point] 
												= (double)atof(buf);
							myData->cali.tmpData[bd][ch]
									.AD_B[type][range][point]
								 = data[ch].AD_B[type][range][point];
						}					
#endif
					}

					tmp = fscanf(fp, "%s", temp); //ad_ratio
					memset(buf, 0x00, sizeof(buf));
					tmp = fscanf(fp, "%s", buf);
					data[ch].AD_Ratio[type][range][0] = (double)atof(buf);
#if CYCLER_TYPE == DIGITAL_CYC
					myData->cali.tmpData[bd][ch].AD_Ratio[type][range][0]
						 = data[ch].AD_Ratio[type][range][0];
#endif
					memset(buf, 0x00, sizeof(buf));
					tmp = fscanf(fp, "%s", buf);
					data[ch].AD_Ratio[type][range][1] = (double)atof(buf);
#if CYCLER_TYPE == DIGITAL_CYC
					myData->cali.tmpData[bd][ch].AD_Ratio[type][range][1]
						= data[ch].AD_Ratio[type][range][1];
#endif
				}
			}
		}
		fclose(fp);
	} else {
		userlog(DEBUG_LOG, psName,
			"Can not open %d Board CaliData file(read)\n", bd+1);
		rtn = -1;
	}

	if(rtn == 0) {
		memcpy((char *)&myData->cali.data[bd][0], (char *)&data[0],
			sizeof(S_CALI_DATA)*MAX_CH_PER_BD);
	}
	return rtn;
}

int Read_BdCaliData_FAD(int bd)
{
	int		rtn, tmp, ch, type, range, point, read_ch;
	char	fileName[128], temp[64], buf[64];
	S_CALI_DATA	data[MAX_CH_PER_BD];
	FILE	*fp;

	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/cycler_data/config/caliData/CALI_BD");
	memset(temp, 0x00, sizeof temp);
	temp[0] = (char)(49+bd);
	strcat(fileName, temp);
	strcat(fileName, "_FAD");
	// /root/cycler_data/config/caliData/CALI_BD#_FAD

	memset(data, 0, sizeof(S_CALI_DATA)*MAX_CH_PER_BD);

	if(myData->mData.config.chPerBd > 32){
		read_ch = 64;
	} else if(myData->mData.config.chPerBd > 16){
		read_ch = 32;
	}else{
		read_ch = 16;
	}
	//190617 oys add start
	if(read_ch > MAX_CH_PER_BD)
		read_ch = MAX_CH_PER_BD;
	//add end

	if((fp = fopen(fileName, "r")) != NULL) {
		rtn = 0;
		tmp = fscanf(fp, "%s", temp); tmp = fscanf(fp, "%s", temp);
		tmp = fscanf(fp, "%s", temp);
		
		for(type=0; type < MAX_TYPE; type++) {
			tmp = fscanf(fp, "%s", temp); //voltage, current
			for(range=0; range < MAX_RANGE; range++) {
				tmp = fscanf(fp, "%s", temp); //range1, 2, 3, 4
//				for(ch=0; ch < MAX_CH_PER_BD; ch++) {
				for(ch=0; ch < read_ch; ch++) {
					tmp = fscanf(fp, "%s", temp); //ch

					tmp = fscanf(fp, "%s", temp); //setPointNum
					memset(buf, 0x00, sizeof(buf));
					tmp = fscanf(fp, "%s", buf);
					data[ch].point[type][range].setPointNum
						= (unsigned char)atoi(buf);

					tmp = fscanf(fp, "%s", temp); //setPoint
					for(point=0; point < MAX_CALI_POINT; point++) {
						memset(buf, 0x00, sizeof(buf));
						tmp = fscanf(fp, "%s", buf);
						data[ch].point[type][range].setPoint[point]
							= atol(buf);
						if(data[ch].point[type][range].setPoint[point] < 0){
							data[ch].point[type][range].discharge_pointNum++;
						}
					}

					tmp = fscanf(fp, "%s", temp); //checkPointNum
					memset(buf, 0x00, sizeof(buf));
					tmp = fscanf(fp, "%s", buf);
					data[ch].point[type][range].checkPointNum
						= (unsigned char)atoi(buf);

					tmp = fscanf(fp, "%s", temp); //checkPoint
					for(point=0; point < MAX_CALI_POINT; point++) {
						memset(buf, 0x00, sizeof(buf));
						tmp = fscanf(fp, "%s", buf);
						data[ch].point[type][range].checkPoint[point]
							= atol(buf);
					}

					tmp = fscanf(fp, "%s", temp); //set_ad
					for(point=0; point < MAX_CALI_POINT; point++) {
						memset(buf, 0x00, sizeof(buf));
						tmp = fscanf(fp, "%s", buf);
						data[ch].set_ad[type][range][point] = (double)atof(buf);
					}

					tmp = fscanf(fp, "%s", temp); //set_meter
					for(point=0; point < MAX_CALI_POINT; point++) {
						memset(buf, 0x00, sizeof(buf));
						tmp = fscanf(fp, "%s", buf);
						data[ch].set_meter[type][range][point] = (double)atof(buf);
					}

					tmp = fscanf(fp, "%s", temp); //check_ad
					for(point=0; point < MAX_CALI_POINT; point++) {
						memset(buf, 0x00, sizeof(buf));
						tmp = fscanf(fp, "%s", buf);
						data[ch].check_ad[type][range][point] = (double)atof(buf);
					}

					tmp = fscanf(fp, "%s", temp); //check_meter
					for(point=0; point < MAX_CALI_POINT; point++) {
						memset(buf, 0x00, sizeof(buf));
						tmp = fscanf(fp, "%s", buf);
						data[ch].check_meter[type][range][point] = (double)atof(buf);
					}

					tmp = fscanf(fp, "%s", temp); //ad_a
					for(point=0; point < MAX_CALI_POINT-1; point++) {
						memset(buf, 0x00, sizeof(buf));
						tmp = fscanf(fp, "%s", buf);
						data[ch].AD_A[type][range][point] = (double)atof(buf);
					}

					tmp = fscanf(fp, "%s", temp); //ad_b
					for(point=0; point < MAX_CALI_POINT-1; point++) {
						memset(buf, 0x00, sizeof(buf));
						tmp = fscanf(fp, "%s", buf);
						data[ch].AD_B[type][range][point] = (double)atof(buf);
					}
				}
			}
		}
		fclose(fp);
	} else {
		userlog(DEBUG_LOG, psName,
			"Can not open %d FAD CaliData file(read)\n", bd+1);
		rtn = -1;
	}

	if(rtn == 0) {
		memcpy((char *)&myData->cali.data_fad[bd][0], (char *)&data[0],
			sizeof(S_CALI_DATA)*MAX_CH_PER_BD);
	}
	return rtn;
}

int Read_BdCaliData_I_Offset(int bd)
{
	int		rtn, tmp, ch, type, range, point, read_ch;
	char	fileName[128], temp[64], buf[64];
	S_CALI_DATA	data[MAX_CH_PER_BD];
	FILE	*fp;

	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/cycler_data/config/caliData/CALI_BD");
	memset(temp, 0x00, sizeof temp);
//190624 oys modify start : Support CALI_BD16 file
	if(bd < 9){
		temp[0] = (char)(49+bd);
	} else {
		temp[0] = (char)(49);
		temp[1] = (char)(39+bd);
	}
//190624 oys modift endp
	strcat(fileName, temp);
	strcat(fileName, "_I_Offset");
	// /root/cycler_data/config/caliData/CALI_BD#_I_Offset

	memset(data, 0, sizeof(S_CALI_DATA)*MAX_CH_PER_BD);

	if(myData->mData.config.chPerBd > 32){
		read_ch = 64;
	} else if(myData->mData.config.chPerBd > 16){
		read_ch = 32;
	}else{
		read_ch = 16;
	}
	//190617 oys add start
	if(read_ch > MAX_CH_PER_BD)
		read_ch = MAX_CH_PER_BD;
	//add end

	if((fp = fopen(fileName, "r")) != NULL) {
		rtn = 0;
		tmp = fscanf(fp, "%s", temp); tmp = fscanf(fp, "%s", temp);
		tmp = fscanf(fp, "%s", temp);
		
		for(type=0; type < MAX_TYPE; type++) {
			tmp = fscanf(fp, "%s", temp); //voltage, current
			for(range=0; range < MAX_RANGE; range++) {
				tmp = fscanf(fp, "%s", temp); //range1, 2, 3, 4
//				for(ch=0; ch < MAX_CH_PER_BD; ch++) {
				for(ch=0; ch < read_ch; ch++) {
					tmp = fscanf(fp, "%s", temp); //ch

					tmp = fscanf(fp, "%s", temp); //setPointNum
					memset(buf, 0x00, sizeof(buf));
					tmp = fscanf(fp, "%s", buf);
					data[ch].point[type][range].setPointNum
						= (unsigned char)atoi(buf);

					tmp = fscanf(fp, "%s", temp); //setPoint
					for(point=0; point < MAX_CALI_POINT; point++) {
						memset(buf, 0x00, sizeof(buf));
						tmp = fscanf(fp, "%s", buf);
						data[ch].point[type][range].setPoint[point]
							= atol(buf);
						if(data[ch].point[type][range].setPoint[point] < 0){
							data[ch].point[type][range].discharge_pointNum++;
						}
					}

					tmp = fscanf(fp, "%s", temp); //checkPointNum
					memset(buf, 0x00, sizeof(buf));
					tmp = fscanf(fp, "%s", buf);
					data[ch].point[type][range].checkPointNum
						= (unsigned char)atoi(buf);

					tmp = fscanf(fp, "%s", temp); //checkPoint
					for(point=0; point < MAX_CALI_POINT; point++) {
						memset(buf, 0x00, sizeof(buf));
						tmp = fscanf(fp, "%s", buf);
						data[ch].point[type][range].checkPoint[point]
							= atol(buf);
					}

					tmp = fscanf(fp, "%s", temp); //set_ad
					for(point=0; point < MAX_CALI_POINT; point++) {
						memset(buf, 0x00, sizeof(buf));
						tmp = fscanf(fp, "%s", buf);
						data[ch].set_ad[type][range][point] = (double)atof(buf);
					}

					tmp = fscanf(fp, "%s", temp); //set_meter
					for(point=0; point < MAX_CALI_POINT; point++) {
						memset(buf, 0x00, sizeof(buf));
						tmp = fscanf(fp, "%s", buf);
						data[ch].set_meter[type][range][point] = (double)atof(buf);
					}

					tmp = fscanf(fp, "%s", temp); //check_ad
					for(point=0; point < MAX_CALI_POINT; point++) {
						memset(buf, 0x00, sizeof(buf));
						tmp = fscanf(fp, "%s", buf);
						data[ch].check_ad[type][range][point] = (double)atof(buf);
					}

					tmp = fscanf(fp, "%s", temp); //check_meter
					for(point=0; point < MAX_CALI_POINT; point++) {
						memset(buf, 0x00, sizeof(buf));
						tmp = fscanf(fp, "%s", buf);
						data[ch].check_meter[type][range][point] = (double)atof(buf);
					}

					tmp = fscanf(fp, "%s", temp); //ad_a
					for(point=0; point < MAX_CALI_POINT-1; point++) {
						memset(buf, 0x00, sizeof(buf));
						tmp = fscanf(fp, "%s", buf);
						data[ch].AD_A[type][range][point] = (double)atof(buf);
					}

					tmp = fscanf(fp, "%s", temp); //ad_b
					for(point=0; point < MAX_CALI_POINT-1; point++) {
						memset(buf, 0x00, sizeof(buf));
						tmp = fscanf(fp, "%s", buf);
						data[ch].AD_B[type][range][point] = (double)atof(buf);
					}
				}
			}
		}
		fclose(fp);
	} else {
		userlog(DEBUG_LOG, psName,
			"Can not open bd[%d] I Offset CaliData file(read)\n", bd+1);
		rtn = -1;
	}

	if(rtn == 0) {
		memcpy((char *)&myData->cali.data_caliMeter2[bd][0], (char *)&data[0],
			sizeof(S_CALI_DATA)*MAX_CH_PER_BD);
	}
	return rtn;
}

int Read_CaliData_Line_Drop(void)
{
	int tmp;
	char temp[32], buf[12], fileName[128];
    FILE *fp;

	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/cycler_data/config/caliData/Cali_Line_Drop");
	// /root/cycler_data/config/caliData/Cali_Line_Drop
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "Cali_Line_Drop file read error\n");
		return -1;
	}

    tmp = fscanf(fp, "%s", temp); 	
	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
   	tmp = fscanf(fp, "%s", buf);
   	myData->cali.line_drop = (float)atof(buf) * 1000.0; //uV
	myData->cali.line_impedance //Ohm
		= fabs(myData->cali.line_drop / (float)myData->mData.config.maxCurrent[0]);
	myData->cali.line_drop_i  //uA
		= (double)(myData->cali.line_drop / myData->cali.line_impedance)/1000.0; 

    fclose(fp);
	return 0;
}

int Read_CellArray_A(void)
{
	short int monitor_no, hw_no, bd, ch;
    int tmp, i;
	char temp[20], buf[12], fileName[128];
    FILE *fp;

	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/cycler_data/config/parameter/CellArray_A");
	// /root/cycler_data/config/parameter/CellArray_A
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "CellArray_A file read error\n");
		system("cp ../Config_backup/CellArray_A /root/cycler_data/config/parameter");
		userlog(DEBUG_LOG, psName, "CellArray_A file copy\n");
    	if((fp = fopen(fileName, "r")) == NULL) {
			return -1;
		}
	}

   	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
   	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
   	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
   	tmp = fscanf(fp, "%s", temp);

	for(i=0; i < MAX_CH_PER_MODULE; i++) {
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

		myData->CellArray1[i].number1 = monitor_no;
		myData->CellArray1[i].number2 = hw_no;
		myData->CellArray1[i].bd = bd;
		myData->CellArray1[i].ch = ch;

		myData->CellArray2[hw_no-1].number1 = monitor_no;
		myData->CellArray2[hw_no-1].number2 = hw_no;
		myData->CellArray2[hw_no-1].bd = bd;
		myData->CellArray2[hw_no-1].ch = ch;
	}

    fclose(fp);
	return 0;
}
//160120 oys add : Error Chamber Use Ch Pause or Unit Shutdown
int Read_ChamArray_A(void)
{
	short int chamber_no, hw_no, bd, ch;
    int tmp, i;
	char temp[20], buf[12], fileName[128];
    FILE *fp;

	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/cycler_data/config/parameter/ChamArray_A");
	// /root/cycler_data/config/parameter/ChamArray_A
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "ChamArray_A file read error\n");
		system("cp ../Config_backup/ChamArray_A /root/cycler_data/config/parameter");
		userlog(DEBUG_LOG, psName, "ChamArray_A file copy\n");
    	if((fp = fopen(fileName, "r")) == NULL) {
			return -1;
		}
	}

   	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
   	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
   	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
   	tmp = fscanf(fp, "%s", temp);

	for(i=0; i < MAX_CH_PER_MODULE; i++) {
		memset(buf, 0x00, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		chamber_no = (short int)atoi(buf);

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

		myData->ChamArray[i].number1 = chamber_no;
		myData->ChamArray[i].number2 = hw_no;
		myData->ChamArray[i].bd = bd;
		myData->ChamArray[i].ch = ch;
	}

    fclose(fp);
	return 0;
}

//210428 hun
int Read_ChamberChNo(void)
{
	short int monitor_no, hw_no, bd, ch;
    int tmp, i;
	char temp[20], buf[12], fileName[128];
    FILE *fp;

	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/cycler_data/config/parameter/ChamberChNo");
	// /root/cycler_data/config/parameter/ChamberChNo
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "ChamberChNo file read error\n");
			return -1;
	}

   	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
   	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
   	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
   	tmp = fscanf(fp, "%s", temp);

	for(i=0; i < MAX_CH_PER_MODULE; i++) {
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

		myData->ChamberChNo[i].number1 = monitor_no;
		myData->ChamberChNo[i].number2 = hw_no;
		myData->ChamberChNo[i].bd = bd;
		myData->ChamberChNo[i].ch = ch;
	}

    fclose(fp);
	return 0;
}

int Create_BdCaliData_Org(int bd)
{
    int	ch, type, range, point, chPerBd;
	char temp[4], fileName[128];
    FILE *fp;

	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/cycler_data/config/caliData_org/CALI_BD");
	memset(temp, 0x00, sizeof temp);
//190624 oys modify start : Support CALI_BD16 file
	if(bd < 9){
		temp[0] = (char)(49+bd);
	} else {
		temp[0] = (char)(49);
		temp[1] = (char)(39+bd);
	}
//190624 oys modify end
	strcat(fileName, temp);
	// /root/cycler_data/config/caliData/CALI_BD#
	// 
	if(myData->mData.config.chPerBd > 32){
		chPerBd = 64;
	} else if(myData->mData.config.chPerBd > 16){
		chPerBd = 32;
	}else{
		chPerBd = 16;
	}
	
	//190617 oys add start
	if(chPerBd > MAX_CH_PER_BD)
		chPerBd = MAX_CH_PER_BD;
	//add end

    if((fp = fopen(fileName, "w")) == NULL) {
		userlog(DEBUG_LOG, psName,
			"Can not open %d Board CaliData file(write)\n", bd+1);
		return -1;
	}
	
	fprintf(fp, "MCTS_Calibration_File\n");
	fprintf(fp, "KJG001-00\n");
	fprintf(fp, "%s\n\n", "date");
	
	for(type=0; type < MAX_TYPE; type++) {
		if(type == 0) {
			fprintf(fp, "voltage\n");
		} else {
			fprintf(fp, "current\n");
		}
		for(range=0; range < MAX_RANGE; range++) {
			if((range+1) == RANGE1) {
				fprintf(fp, "range1\n");
			} else if((range+1) == RANGE2) {
				fprintf(fp, "range2\n");
			} else if((range+1) == RANGE3) {
				fprintf(fp, "range3\n");
			} else {
				fprintf(fp, "range4\n");
			}

			for(ch=0; ch < chPerBd; ch++) {
	    		fprintf(fp, "ch%02d\n", ch+1);

				fprintf(fp, "setPointNum   \n");
			   	fprintf(fp, "%d ", 0);
				fprintf(fp, "\n");

				fprintf(fp, "setPoint      \n");
				for(point=0; point < MAX_CALI_POINT; point++) {
				   	fprintf(fp, "%f ", 0.0);
				}
				fprintf(fp, "\n");

				fprintf(fp, "checkPointNum   \n");
			   	fprintf(fp, "%d ", 0);
				fprintf(fp, "\n");

				fprintf(fp, "checkPoint      \n");
				for(point=0; point < MAX_CALI_POINT; point++) {
				   	fprintf(fp, "%f ", 0.0);
				}
				fprintf(fp, "\n");

				fprintf(fp, "set_ad        \n");
				for(point=0; point < MAX_CALI_POINT; point++) {
				   	fprintf(fp, "%d ", 0);
				}
				fprintf(fp, "\n");

				fprintf(fp, "set_meter     \n");
				for(point=0; point < MAX_CALI_POINT; point++) {
				   	fprintf(fp, "%d ", 0);
				}
				fprintf(fp, "\n");

				fprintf(fp, "check_ad      \n");
				for(point=0; point < MAX_CALI_POINT; point++) {
				   	fprintf(fp, "%d ", 0);
				}
				fprintf(fp, "\n");

				fprintf(fp, "check_meter   \n");
				for(point=0; point < MAX_CALI_POINT; point++) {
				   	fprintf(fp, "%d ", 0);
				}
				fprintf(fp, "\n");

				fprintf(fp, "DA_A          \n");
				for(point=0; point < MAX_CALI_POINT-1; point++) {
				   	fprintf(fp, "%f ", 1.0);
				}
				fprintf(fp, "\n");

				fprintf(fp, "DA_B\n        ");
				for(point=0; point < MAX_CALI_POINT-1; point++) {
				   	fprintf(fp, "%f ", 0.0);
				}
				fprintf(fp, "\n");

				fprintf(fp, "AD_A          \n");
				for(point=0; point < MAX_CALI_POINT-1; point++) {
				   	fprintf(fp, "%f ", 1.0);
				}
				fprintf(fp, "\n");

				fprintf(fp, "AD_B          \n");
				for(point=0; point < MAX_CALI_POINT-1; point++) {
				   	fprintf(fp, "%f ", 0.0);
				}
				fprintf(fp, "\n");

				fprintf(fp, "AD_Ratio      \n");
				fprintf(fp, "%f %f", 1.0, 1.0);
				fprintf(fp, "\n\n");
			}
		}
	}
   	fclose(fp);

	return 0;
}

int Read_Addr_Interface(void)
{
	char temp[32], buf[12], fileName[128];
	char *tail;
    int i, j, tmp;
    FILE *fp;

	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/cycler_data/config/parameter/Addr_Interface");
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "Addr_Interface file read error\n");
		system("cp ../Config_backup/Addr_Interface /root/cycler_data/config/parameter");
		userlog(DEBUG_LOG, psName, "Addr_Interface file copy\n");
    	if((fp = fopen(fileName, "r")) == NULL) {
			return -1;
		}
	}

	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0, sizeof buf);
   	tmp = fscanf(fp, "%s", buf);
	myData->mData.addr.inputAddrNo = atoi(buf); 

	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0, sizeof buf);
   	tmp = fscanf(fp, "%s", buf);
	myData->mData.addr.outputAddrNo = atoi(buf); 

	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0, sizeof buf);
   	tmp = fscanf(fp, "%s", buf);
	myData->mData.addr.expendAddrNo = atoi(buf); 

	tmp = fscanf(fp, "%s", temp); tmp = fscanf(fp, "%s", temp);
	tmp = fscanf(fp, "%s", temp); tmp = fscanf(fp, "%s", temp);
	tmp = fscanf(fp, "%s", temp);

	for(i=0; i < 32; i++) {
		tmp = fscanf(fp, "%s", temp);
		for(j = 0; j < 3; j++){
			memset(buf, 0, sizeof buf);
		   	tmp = fscanf(fp, "%s", buf);
			myData->mData.addr.interface[j][i] 
				= (int)strtol((char *)&buf, &tail, 16);
		}
		tmp = fscanf(fp, "%s", temp);
	}

    fclose(fp);
	return 0;
}

int Read_Addr_Main(void)
{
	char temp[32], buf[12], fileName[128];
	char *tail;
    int i, tmp;
    FILE *fp;

	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/cycler_data/config/parameter/Addr_Main");
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "Addr_Main file read error\n");
		system("cp ../Config_backup/Addr_Main /root/cycler_data/config/parameter");
		userlog(DEBUG_LOG, psName, "Addr_Main file copy\n");
    	if((fp = fopen(fileName, "r")) == NULL) {
			return -1;
		}
	}

	tmp = fscanf(fp, "%s", temp); 
	tmp = fscanf(fp, "%s", temp);
	tmp = fscanf(fp, "%s", temp);

	for(i=0; i < 32; i++) {
		tmp = fscanf(fp, "%s", temp);
		tmp = fscanf(fp, "%s", temp);
		memset(buf, 0, sizeof buf);
	   	tmp = fscanf(fp, "%s", buf);
		myData->mData.addr.main[i] 
			= (int)strtol((char *)&buf, &tail, 16);
	}

    fclose(fp);
	return 0;
}

int Read_FaultBitSet_SMPS_OT(void)
{
	char temp[32], buf[12], fileName[128];
	char *tail;
    int i, j, tmp;
    FILE *fp;

	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/cycler_data/config/parameter/FaultBitSet_SMPS_OT");
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "FaultBitSet_SMPS_OT file read error\n");
		system("cp ../Config_backup/FaultBitSet_SMPS_OT /root/cycler_data/config/parameter");
		userlog(DEBUG_LOG, psName, "FaultBitSet_SMPS_OT file copy\n");
    	if((fp = fopen(fileName, "r")) == NULL) {
			return -1;
		}
	}

	for(i=0; i < 8; i++) {
		tmp = fscanf(fp, "%s", temp); // Description
		tmp = fscanf(fp, "%s", temp); // :
		memset(buf, 0, sizeof buf); // value
	   	tmp = fscanf(fp, "%s", buf);
		myData->mData.fault.SMPS7_5V[i] 
			= (unsigned char)strtol((char *)&buf, &tail, 16);
	}
	for(i=0; i < 8; i++) {
		tmp = fscanf(fp, "%s", temp); // Description
		tmp = fscanf(fp, "%s", temp); // :
		memset(buf, 0, sizeof buf); // value
	   	tmp = fscanf(fp, "%s", buf);
		myData->mData.fault.SMPS3_3V[i] 
			= (unsigned char)strtol((char *)&buf, &tail, 16);
	}
	for(i=0; i < 8; i++) {
		tmp = fscanf(fp, "%s", temp); // Description
		tmp = fscanf(fp, "%s", temp); // :
		memset(buf, 0, sizeof buf); // value
	   	tmp = fscanf(fp, "%s", buf);
		myData->mData.fault.OT[i] 
			= (unsigned char)strtol((char *)&buf, &tail, 16);
	}

	/*	date	: 7.1.2011
	 *	writer	:pjy
	 *	function: read boardFaul setting parameter
	 *			 file located parameter directory
	 */
	tmp = fscanf(fp, "%s", temp);//BoardFaultBitSet
	for(i = 0; i < 3; i++){
		tmp = fscanf(fp, "%s", temp);	//BaordNo		
		tmp = fscanf(fp, "%s", temp);	//USE_FLAG
		tmp = fscanf(fp, "%s", temp);	//ACTIVE<H/L>	
		tmp = fscanf(fp, "%s", temp);	//OT_BIT
		//firstValue : use_flag, secondValue : active<h/l>, thiredValue : bit
		for(j = 0; j< 4; j++){
			tmp = fscanf(fp, "%s", temp);//BoardNo
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);//value
			myData->mData.board_fault[j][i].useFlag
				=(unsigned char)strtol((char *)&buf, &tail, 16);
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);//value
			myData->mData.board_fault[j][i].active
				=(unsigned char)strtol((char *)&buf, &tail, 16);
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);//value
			myData->mData.board_fault[j][i].bit
				=(unsigned char)strtol((char *)&buf, &tail, 16);
		}
	}
	/*	date	: 4.22.2015
	 *	writer	: lyh
	 *	function: read boardFaul setting parameter
	 *			  file located parameter directory
	 *			  MAIN_NEW_11 SMPS, OT	
	 */
	tmp = fscanf(fp, "%s", temp); // MAIN_REV11_PS_FAULT
	tmp = fscanf(fp, "%s", temp); tmp = fscanf(fp, "%s", temp);//7VPS,USE_ADDR
	tmp = fscanf(fp, "%s", temp); tmp = fscanf(fp, "%s", temp);//CH, FUALT_BIT
	for(i = 0; i < 8 ; i++){
		tmp = fscanf(fp, "%s", temp);//PS_7V[i]
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf); //use_addr[i]
		myData->mData.fault.PS_ADDR_7V[i]
				=(unsigned long)strtol((char *)&buf, &tail, 16);
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf); //activech[i]
		myData->mData.fault.PS_ACTIVECH_7V[i] = atoi(buf);
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf); //ps_bit[i]
		myData->mData.fault.PS_BIT_7V[i]
				=(unsigned char)strtol((char *)&buf, &tail, 16);
	}
	tmp = fscanf(fp, "%s", temp); tmp = fscanf(fp, "%s", temp);//3VPS,USE_ADDR
	tmp = fscanf(fp, "%s", temp); tmp = fscanf(fp, "%s", temp);//CH, FUALT_BIT
	for(i = 0; i < 8 ; i++){
		tmp = fscanf(fp, "%s", temp);//PS_3V[i]
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf); //use_addr[i]
		myData->mData.fault.PS_ADDR_3V[i]
				=(unsigned long)strtol((char *)&buf, &tail, 16);
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf); //activech[i]
		myData->mData.fault.PS_ACTIVECH_3V[i] = atoi(buf);
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf); //ps_bit[i]
		myData->mData.fault.PS_BIT_3V[i] 
				=(unsigned char)strtol((char *)&buf, &tail, 16);
	}
	
	tmp = fscanf(fp, "%s", temp); tmp = fscanf(fp, "%s", temp);//OT,USE_ADDR
	tmp = fscanf(fp, "%s", temp); //OT_BIT
	for(i = 0; i < 4 ; i++){
		tmp = fscanf(fp, "%s", temp);//OT[i]
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf); //use_addr[i]
		myData->mData.fault.ADDR_OT[i]
				=(unsigned long)strtol((char *)&buf, &tail, 16);
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf); //OT_bit[i]
		myData->mData.fault.BIT_OT[i]
				=(unsigned char)strtol((char *)&buf, &tail, 16);
	}
	
    fclose(fp);
	return 0;
}

//20100128 kji add
int Read_ChAttribute(void)
{
	char temp[64], buf[64], fileName[128];
    int bd,ch,i,tmp;
    FILE *fp;

	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/cycler_data/config/parameter/ChAttribute");
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "ChAttribute file read error\n");
		system("cp ../Config_backup/ChAttribute /root/cycler_data/config/parameter");
		userlog(DEBUG_LOG, psName, "ChAttribute file read copy\n");
    	if((fp = fopen(fileName, "r")) == NULL) {
			return -1;
		}
	}
    for(i=0;i<myData->mData.config.installedCh;i++)
	{
		bd = myData->CellArray1[i].bd;
		ch = myData->CellArray1[i].ch;

		tmp = fscanf(fp,"%s",temp); tmp = fscanf(fp,"%s",temp);
		tmp = fscanf(fp,"%s",temp);

		tmp = fscanf(fp,"%s",temp); tmp = fscanf(fp,"%s",temp); 
		memset(buf,0x00,sizeof(buf));
		tmp = fscanf(fp,"%s",buf);
		myData->bData[bd].cData[ch].ChAttribute.chNo_master = 
			(unsigned char)atoi(buf);
			
		tmp = fscanf(fp,"%s",temp); tmp = fscanf(fp,"%s",temp); 
		memset(buf,0x00,sizeof(buf));
		tmp = fscanf(fp,"%s",buf);
		myData->bData[bd].cData[ch].ChAttribute.chNo_slave[0] = 
				(unsigned char)atoi(buf);

		tmp = fscanf(fp,"%s",temp); tmp = fscanf(fp,"%s",temp); 
		memset(buf,0x00,sizeof(buf));
		tmp = fscanf(fp,"%s",buf);
		myData->bData[bd].cData[ch].ChAttribute.chNo_slave[1] = 
				(unsigned char)atoi(buf);
	
		tmp = fscanf(fp,"%s",temp); tmp = fscanf(fp,"%s",temp); 
		memset(buf,0x00,sizeof(buf));
		tmp = fscanf(fp,"%s",buf);
		myData->bData[bd].cData[ch].ChAttribute.chNo_slave[2] = 
				(unsigned char)atoi(buf);
	
		tmp = fscanf(fp,"%s",temp); tmp = fscanf(fp,"%s",temp); 
		memset(buf,0x00,sizeof(buf));
		tmp = fscanf(fp,"%s",buf);
		myData->bData[bd].cData[ch].ChAttribute.opType= 
				(unsigned char)atoi(buf);
		tmp = fscanf(fp,"%s",temp); tmp = fscanf(fp,"%s",temp); 
		memset(buf,0x00,sizeof(buf));
		tmp = fscanf(fp,"%s",buf);
		myData->bData[bd].cData[ch].ChAttribute.reserved1[0]= 
				(unsigned char)atoi(buf);

		tmp = fscanf(fp,"%s",temp); tmp = fscanf(fp,"%s",temp); 
		memset(buf,0x00,sizeof(buf));
		tmp = fscanf(fp,"%s",buf);
		myData->bData[bd].cData[ch].ChAttribute.reserved1[1]= 
				(unsigned char)atoi(buf);
		
		tmp = fscanf(fp,"%s",temp); tmp = fscanf(fp,"%s",temp); 
		memset(buf,0x00,sizeof(buf));
		tmp = fscanf(fp,"%s",buf);
		myData->bData[bd].cData[ch].ChAttribute.reserved1[2]= 
				(unsigned char)atoi(buf);

		if(myData->mData.config.parallelMode == P0
			|| myData->mData.config.parallelMode == P2) { //kjg_180521
			myData->bData[bd].cData[ch].ChAttribute.chNo_master = ch + 1;
			myData->bData[bd].cData[ch].ChAttribute.opType = P0;
			myData->bData[bd].cData[ch].ChAttribute.chNo_slave[0] = 0; 
			myData->bData[bd].cData[ch].ChAttribute.chNo_slave[1] = 0; 
			myData->bData[bd].cData[ch].ChAttribute.chNo_slave[2] = 0; 
		}
	}
	fclose(fp);
	return 0;
}

//20100128 kji add
int Write_ChAttribute(void)
{
	char fileName[128];
    int bd,ch;
    FILE *fp;

	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/cycler_data/config/parameter/ChAttribute");
    if((fp = fopen(fileName, "w")) == NULL) {
		userlog(DEBUG_LOG, psName, "ChAttribute file write error\n");
		return -1;
	}
    for(bd=0;bd<myData->mData.config.installedBd;bd++)
    {
        for(ch=0;ch<myData->mData.config.chPerBd;ch++)
        {
            if((myData->mData.config.chPerBd * bd + ch)
	            > (myData->mData.config.installedCh-1)) {
	            continue;
			}
					
			fprintf(fp, "chNo		: %d\n",
						myData->mData.config.chPerBd * bd + ch);
			fprintf(fp, "chNo_master : %d\n",
						myData->bData[bd].cData[ch].ChAttribute.chNo_master);
			fprintf(fp, "chNo_slave1 : %d\n",
						myData->bData[bd].cData[ch].ChAttribute.chNo_slave[0]);
			fprintf(fp, "chNo_slave2 : %d\n",
						myData->bData[bd].cData[ch].ChAttribute.chNo_slave[1]);
			fprintf(fp, "chNo_slave3 : %d\n",
						myData->bData[bd].cData[ch].ChAttribute.chNo_slave[2]);
			fprintf(fp, "opType		: %d\n",
						myData->bData[bd].cData[ch].ChAttribute.opType);
			fprintf(fp, "reserved1   : %d\n",
						myData->bData[bd].cData[ch].ChAttribute.reserved1[0]);
			fprintf(fp, "reserved2   : %d\n",
						myData->bData[bd].cData[ch].ChAttribute.reserved1[1]);
			fprintf(fp, "reserved3   : %d\n",
						myData->bData[bd].cData[ch].ChAttribute.reserved1[2]);
			fprintf(fp,"\n");
	
		}
	}
	fclose(fp);
	return 0;
}

//210428 hun
int Write_ChamberChNo(void)
{
	char fileName[128];
	int i = 0;
    FILE *fp;

	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/cycler_data/config/parameter/ChamberChNo");
    if((fp = fopen(fileName, "w")) == NULL) {
		userlog(DEBUG_LOG, psName, "ChamberChNo file write error\n");
		return -1;
	}
	fprintf(fp, "chamberNo\t:\thw_no\t:\tbd\t:\tch\n");
	
	for(i=0; i < MAX_CH_PER_MODULE; i++) {
		fprintf(fp, "%d\t\t\t",myData->ChamberChNo[i].number1);
		fprintf(fp, ":\t");
		fprintf(fp, "%d\t\t",myData->ChamberChNo[i].number2);
		fprintf(fp, ":\t");
		fprintf(fp, "%d\t",myData->ChamberChNo[i].bd);		
		fprintf(fp, ":\t");	
		fprintf(fp, "%d\n",myData->ChamberChNo[i].ch);
	}
	fclose(fp);
	return 0;
}

//hun_211020
int Write_Default_ChamberChNo(void)
{
	short int monitor_no, hw_no, bd, ch;
    int tmp;
	char temp[20], buf[12];
	
	char fileName[128];
	int i = 0;
    FILE *fp;
	
	memset(fileName, 0, sizeof(fileName));
	sprintf(fileName, "/root/cycler_data/config/parameter/ChamberChNo");
	fp = fopen(fileName, "w");
    if(fp == NULL || fp < 0){
		userlog(DEBUG_LOG, psName, "ChamberChNo file Default write error\n");
		return -1;
	}
	fprintf(fp, "chamberNo\t:\thw_no\t:\tbd\t:\tch\n");
	
	for(i=0; i < MAX_CH_PER_MODULE; i++) {
		fprintf(fp, "%d\t\t\t",0);
		fprintf(fp, ":\t");
		fprintf(fp, "%d\t\t",i+1);
		fprintf(fp, ":\t");
		fprintf(fp, "%d\t",i / myData->mData.config.chPerBd);		
		fprintf(fp, ":\t");	
		fprintf(fp, "%d\n",i % myData->mData.config.chPerBd);
	}
	fflush(fp);
	fclose(fp);
	userlog(DEBUG_LOG, psName, "ChamberChNo file Default write\n");

	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/cycler_data/config/parameter/ChamberChNo");
	// /root/cycler_data/config/parameter/ChamberChNo
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "ChamberChNo file Default read error\n");
			return -1;
	}

   	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
   	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
   	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
   	tmp = fscanf(fp, "%s", temp);

	for(i=0; i < MAX_CH_PER_MODULE; i++) {
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

		myData->ChamberChNo[i].number1 = monitor_no;
		myData->ChamberChNo[i].number2 = hw_no;
		myData->ChamberChNo[i].bd = bd;
		myData->ChamberChNo[i].ch = ch;
	}

    fclose(fp);
	userlog(DEBUG_LOG, psName, "ChamberChNo file Default read\n");

	return 0;
}


int	Read_AuxSetData(void)
{
    int tmp, i, ch, count1, count2;
	char temp[32], buf[MAX_AUX_NAME_SIZE], fileName[128], *char_tmp;
    FILE *fp;

	memset((char *)&myData->auxDataCount, 0, sizeof(int) * MAX_CH_PER_MODULE);
	memset((char *)&myData->auxSetData[0], 0,
		sizeof(S_AUX_SET_DATA) * MAX_AUX_DATA);

	if(myData->mData.config.installedTemp == 0
		&& myData->mData.config.installedAuxV == 0) return 0;

	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/cycler_data/config/parameter/AuxSetData");
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "AuxSetData file read error\n");
		system("cp ../Config_backup/AuxSetData /root/cycler_data/config/parameter");
		userlog(DEBUG_LOG, psName, "AuxSetData file copy\n");
		if((fp = fopen(fileName, "r")) == NULL) {
			return -1;
		}
	}

	for(i=0; i < MAX_AUX_DATA; i++) {
	    tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->auxSetData[i].auxChNo = (short int)atoi(buf);

	    tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->auxSetData[i].auxType = (short int)atoi(buf);

	    tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->auxSetData[i].chNo = (unsigned char)atoi(buf);

	    tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->auxSetData[i].reserved1[0] = (unsigned char)atoi(buf);

	    tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->auxSetData[i].reserved1[1] = (unsigned char)atoi(buf);

	    tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->auxSetData[i].reserved1[2] = (unsigned char)atoi(buf);

	    tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
		memset(buf, 0, sizeof buf);
		char_tmp = fgets(buf, MAX_AUX_NAME_SIZE, fp);
		memset((char *)&myData->auxSetData[i].name, 0, MAX_AUX_NAME_SIZE);
		if(buf[0] == 0x24) { //$
		} else {
			tmp = strlen(buf) - 2;
			if(tmp > 0) {
				memcpy((char *)&myData->auxSetData[i].name,
					(char *)&buf[1], tmp);
			}
		}
		
		tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
		memset(buf, 0, sizeof buf);
  		tmp = fscanf(fp, "%s", buf);
    	myData->auxSetData[i].fault_upper = atol(buf);
		
		tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
		memset(buf, 0, sizeof buf);
  		tmp = fscanf(fp, "%s", buf);
   		myData->auxSetData[i].fault_lower = atol(buf);
		
		tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
		memset(buf, 0, sizeof buf);
  		tmp = fscanf(fp, "%s", buf);
    	myData->auxSetData[i].end_upper = atol(buf);
		
		tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
		memset(buf, 0, sizeof buf);
  		tmp = fscanf(fp, "%s", buf);
    	myData->auxSetData[i].end_lower = atol(buf);
	
		tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
		memset(buf, 0, sizeof buf);
  		tmp = fscanf(fp, "%s", buf);
   		myData->auxSetData[i].function_div1 = (short int)atoi(buf);
		
		tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
		memset(buf, 0, sizeof buf);
  		tmp = fscanf(fp, "%s", buf);
   		myData->auxSetData[i].function_div2 = (short int)atoi(buf);

		tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
		memset(buf, 0, sizeof buf);
  		tmp = fscanf(fp, "%s", buf);
   		myData->auxSetData[i].function_div3 = (short int)atoi(buf);

		tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
		memset(buf, 0, sizeof buf);
  		tmp = fscanf(fp, "%s", buf);
    	myData->auxSetData[i].reserved2 = atol(buf);
	}
   	fclose(fp);

	for(ch=0; ch < MAX_CH_PER_MODULE; ch++) {
		count1 = count2 = 0;
		for(i=0; i < MAX_AUX_DATA; i++) {
			if((ch+1) == myData->auxSetData[i].chNo) {
				if(myData->auxSetData[i].auxType == 0) { //temperature
					count1++;
				} else if(myData->auxSetData[i].auxType == 1) { //voltage
					count2++;
				}
			}
		}
		myData->auxDataCount[ch][0] = count1;
		myData->auxDataCount[ch][1] = count2;
	}

	return 0;
}

int Read_DaqArray(void)
{
    int tmp, i;
    char temp[32], buf[MAX_AUX_NAME_SIZE], fileName[128];
    FILE *fp;

    memset(fileName, 0x00, sizeof(fileName));
    strcpy(fileName, "/root/cycler_data/config/parameter/DaqArray");
    if((fp = fopen(fileName, "r")) == NULL) {
		system("cp ../Config_backup/DaqArray /root/cycler_data/config/parameter");
		userlog(DEBUG_LOG, psName, "AuxSetData file copy\n");
    	if((fp = fopen(fileName, "r")) == NULL) {
    		for(i=0;i<myData->mData.config.installedAuxV;i++) {
       		 	myData->daq.misc.map[i] = i;
			}
    	    return 0;
		}
	}

    tmp = fscanf(fp, "%s", temp);
    tmp = fscanf(fp, "%s", temp);
    tmp = fscanf(fp, "%s", temp);
    for(i=0;i<myData->mData.config.installedAuxV;i++) {
        tmp = fscanf(fp, "%s", temp);   tmp = fscanf(fp, "%s", temp);
        memset(buf, 0, sizeof buf);
        tmp = fscanf(fp, "%s", buf);
        myData->daq.misc.map[i] = atoi(buf);
    }
    fclose(fp);
    return 0;
}

int	Write_AuxSetData(void)
{
	char fileName[128];
	int i, ch, count1, count2;
    FILE *fp;

	memset((char *)&myData->auxDataCount, 0, sizeof(int) * MAX_CH_PER_MODULE);

	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/cycler_data/config/parameter/AuxSetData");
    if((fp = fopen(fileName, "w")) == NULL) {
		userlog(DEBUG_LOG, psName, "AuxSetData file read error\n");
		return -1;
	}

	for(i=0; i < MAX_AUX_DATA; i++) {
		fprintf(fp, "auxChNo     : %d\n", myData->auxSetData[i].auxChNo);
		fprintf(fp, "auxType     : %d\n", myData->auxSetData[i].auxType);
		fprintf(fp, "chNo        : %d\n", myData->auxSetData[i].chNo);
		fprintf(fp, "reserved1   : %d\n", myData->auxSetData[i].reserved1[0]);
		fprintf(fp, "reserved2   : %d\n", myData->auxSetData[i].reserved1[1]);
		fprintf(fp, "reserved3   : %d\n", myData->auxSetData[i].reserved1[2]);
		if(myData->auxSetData[i].name[0] == 0) {
			fprintf(fp, "name        : $\n");
		} else {
			fprintf(fp, "name        : %s\n", myData->auxSetData[i].name);
		}
   		fprintf(fp, "fault_upper : %ld\n", myData->auxSetData[i].fault_upper);
    	fprintf(fp, "fault_lower : %ld\n", myData->auxSetData[i].fault_lower);
   		fprintf(fp, "end_upper   : %ld\n", myData->auxSetData[i].end_upper);
   		fprintf(fp, "end_lower   : %ld\n", myData->auxSetData[i].end_lower);
    	fprintf(fp, "func_div1    : %d\n", myData->auxSetData[i].function_div1);
    	fprintf(fp, "func_div2    : %d\n", myData->auxSetData[i].function_div2);
    	fprintf(fp, "func_div3    : %d\n", myData->auxSetData[i].function_div3);
   		fprintf(fp, "reserved4   : %d\n", myData->auxSetData[i].reserved2);
		fprintf(fp, "\n");
	}
   	fclose(fp);

	for(ch=0; ch < MAX_CH_PER_MODULE; ch++) {
		count1 = count2 = 0;
		for(i=0; i < MAX_AUX_DATA; i++) {
			if((ch+1) == myData->auxSetData[i].chNo) {
				if(myData->auxSetData[i].auxType == 0) { //temperature
					count1++;
				} else if(myData->auxSetData[i].auxType == 1) { //voltage
					count2++;
				}
			}
		}
		myData->auxDataCount[ch][0] = count1;
		myData->auxDataCount[ch][1] = count2;
	}
	return 0;
}

int Read_AuxCaliData(void)
{
	char temp[64], fileName[128];
    int tmp,i;
	FILE *fp;

	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/cycler_data/config/parameter/AuxCaliData");
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "AuxCaliData file read error\n");
		system("cp ../Config_backup/AuxCaliData /root/cycler_data/config/parameter");
		userlog(DEBUG_LOG, psName, "AuxCaliData file copy\n");
    	if((fp = fopen(fileName, "r")) == NULL) {
			return -1;
		}
	}
	tmp = fscanf(fp,"%s",temp);
	tmp = fscanf(fp,"%s",temp);
	tmp = fscanf(fp,"%s",temp);

    for(i=0;i<MAX_AUX_VOLT_DATA;i++)
	{
		tmp = fscanf(fp,"%s",temp);
		tmp = fscanf(fp,"%s",temp);
		myData->daq.cali.AD_A[i] = atof(temp);
		tmp = fscanf(fp,"%s",temp);
		myData->daq.cali.AD_B[i] = atof(temp);
		if(myData->daq.cali.AD_A[i] == 0.0)
		{
			myData->daq.cali.AD_A[i] = 1.0;
			myData->daq.cali.AD_B[i] = 0.0;
		}
	}
	fclose(fp);
	return 0;
}

int Read_FadOffsetData(void)
{
	char temp[64], fileName[128];
    int i, tmp;
	FILE *fp;

	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/cycler_data/config/parameter/FadOffsetData");
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "FadOffsetData file read error\n");
		system("cp ../Config_backup/FadOffsetData /root/cycler_data/config/parameter");
		userlog(DEBUG_LOG, psName, "FadOffsetData file copy\n");
   	 	if((fp = fopen(fileName, "r")) == NULL) {
			return -1;
		}
	}
	//151130 Chan add for fad_offset, Range
	for(i=0 ; i<4 ; i++){
	tmp = fscanf(fp,"%s",temp);
	tmp = fscanf(fp,"%s",temp);
	tmp = fscanf(fp,"%s",temp);
	myData->FADM.config.fad_offset[i]
		= (atof(temp) * (float)myData->mData.config.maxCurrent[i]) / 1000.0;
	}
	
	fclose(fp);
	return 0;
}

int	Write_AuxCaliData(void)
{
	char fileName[128];
	int i; 
    FILE *fp;

	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/cycler_data/config/parameter/AuxCaliData");
    if((fp = fopen(fileName, "w")) == NULL) {
		userlog(DEBUG_LOG, psName, "AuxSetData file write error\n");
		return -1;
	}
	fprintf(fp, "ch ad_gain ad_offset\n");
	for(i=0; i < MAX_AUX_VOLT_DATA; i++) {
		fprintf(fp, "%d %f %f\n",
			i+1, myData->daq.cali.AD_A[i], myData->daq.cali.AD_B[i]);
	}
	fclose(fp);
	return 0;
}
int Read_FadCaliData(void)
{
	char temp[64], fileName[128];
    int tmp,i,bd,ch;
	FILE *fp;

	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/cycler_data/config/parameter/FadCaliDataV");
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "FadCaliDataV file read error\n");
		system("cp ../Config_backup/FadCaliDataV /root/cycler_data/config/parameter");
		userlog(DEBUG_LOG, psName, "FadCaliDataV file copy\n");
    	if((fp = fopen(fileName, "r")) == NULL) {
			return -1;
		}
	}
	tmp = fscanf(fp,"%s",temp);
	tmp = fscanf(fp,"%s",temp);
	tmp = fscanf(fp,"%s",temp);

    for(i=0;i<myData->mData.config.installedCh;i++)
	{
		bd = myData->CellArray1[i].bd;
		ch = myData->CellArray1[i].ch;
		tmp = fscanf(fp,"%s",temp);
		tmp = fscanf(fp,"%s",temp);
		myData->bData[bd].cData[ch].misc.fadGainV = atof(temp);
		tmp = fscanf(fp,"%s",temp);
		myData->bData[bd].cData[ch].misc.fadOffsetV = atof(temp);
		if(myData->bData[bd].cData[ch].misc.fadGainV == 0.0) {
			myData->bData[bd].cData[ch].misc.fadGainV = 1.0;
			myData->bData[bd].cData[ch].misc.fadOffsetV = 0.0;
		}
	}
	fclose(fp);
	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/cycler_data/config/parameter/FadCaliDataI");
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "FadCaliDataI file read error\n");
		system("cp ../Config_backup/FadCaliDataI /root/cycler_data/config/parameter");
		userlog(DEBUG_LOG, psName, "FadCaliDataI file copy\n");
    	if((fp = fopen(fileName, "r")) == NULL) {
			return -2;
		}
	} else {
		system("rm -rf ../Config_backup/FadCaliDataI");
		system("cp /root/cycler_data/config/parameter/FadCaliDataI ../Config_backup");
	}
	tmp = fscanf(fp,"%s",temp);
	tmp = fscanf(fp,"%s",temp);
	tmp = fscanf(fp,"%s",temp);

    for(i=0;i<myData->mData.config.installedCh;i++)
	{
		bd = myData->CellArray1[i].bd;
		ch = myData->CellArray1[i].ch;
		tmp = fscanf(fp,"%s",temp);
		tmp = fscanf(fp,"%s",temp);
		myData->bData[bd].cData[ch].misc.fadGainI = atof(temp);
		tmp = fscanf(fp,"%s",temp);
		myData->bData[bd].cData[ch].misc.fadOffsetI = atof(temp);
		if(myData->bData[bd].cData[ch].misc.fadGainI == 0.0) {
			myData->bData[bd].cData[ch].misc.fadGainI = 1.0;
			myData->bData[bd].cData[ch].misc.fadOffsetI = 0.0;
		}
	}
	fclose(fp);
	return 0;
}
//110429 loadprocess add
int Read_LoadProcess(void)
{
	char temp[64], fileName[128];
    int tmp,i;
	FILE *fp;

	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/cycler_data/config/parameter/Load_Process");
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "Load_Process file read error\n");
		system("cp ../Config_backup/Load_Process /root/cycler_data/config/parameter");
		userlog(DEBUG_LOG, psName, "Load_Process file copy\n");
    	if((fp = fopen(fileName, "r")) == NULL) {
			return -1;
		}
	}
	tmp = fscanf(fp,"%s",temp);
	tmp = fscanf(fp,"%s",temp);
	tmp = fscanf(fp,"%s",temp);

    for(i=0;i<MAX_PROCESS_NUMBER;i++)
	{
		tmp = fscanf(fp,"%s",temp);
		tmp = fscanf(fp,"%s",temp);
		tmp = fscanf(fp,"%s",temp);
		myPs->loadProcess[i] = atoi(temp);
	}
	fclose(fp);
	return 0;
}

//20120206 kji add
int Write_ChCompData(void)
{
	char fileName[128];
    int bd,ch;
    FILE *fp;

	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/cycler_data/config/parameter/ChCompData");
    if((fp = fopen(fileName, "w")) == NULL) {
		userlog(DEBUG_LOG, psName, "ChAttribute file write error\n");
		return -1;
	}
    for(bd=0;bd<myData->mData.config.installedBd;bd++)
    {
        for(ch=0;ch<myData->mData.config.chPerBd;ch++)
        {
            if((myData->mData.config.chPerBd * bd + ch)
	            > (myData->mData.config.installedCh-1)) {
	            continue;
			}
			fprintf(fp, "chNo		: %d\n",
						myData->mData.config.chPerBd * bd + ch);
			fprintf(fp, "useFlag : %d\n",
						myData->bData[bd].cData[ch].ChCompData.useFlag);
			fprintf(fp, "compPlus : %ld\n",
						myData->bData[bd].cData[ch].ChCompData.compPlus);
			fprintf(fp, "compMinus : %ld\n",
						myData->bData[bd].cData[ch].ChCompData.compMinus);
			fprintf(fp,"\n");
	
		}
	}
	fclose(fp);
	return 0;
}

//20120206 kji add
int Read_ChCompData(void)
{
	char temp[64], buf[64], fileName[128];
    int bd,ch,i,tmp;
    FILE *fp;

	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/cycler_data/config/parameter/ChCompData");
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "ChCompData file write error\n");
		system("cp ../Config_backup/ChCompData /root/cycler_data/config/parameter");
		userlog(DEBUG_LOG, psName, "ChCompData file copy\n");
		if((fp = fopen(fileName, "r")) == NULL) {
			return -1;
		}
	}
    for(i=0;i<myData->mData.config.installedCh;i++)
	{
		bd = myData->CellArray1[i].bd;
		ch = myData->CellArray1[i].ch;

		tmp = fscanf(fp,"%s",temp); tmp = fscanf(fp,"%s",temp);
		tmp = fscanf(fp,"%s",temp);

		tmp = fscanf(fp,"%s",temp); tmp = fscanf(fp,"%s",temp); 
		memset(buf,0x00,sizeof(buf));
		tmp = fscanf(fp,"%s",buf);
		myData->bData[bd].cData[ch].ChCompData.useFlag = 
			(unsigned char)atoi(buf);

		tmp = fscanf(fp,"%s",temp); tmp = fscanf(fp,"%s",temp); 
		memset(buf,0x00,sizeof(buf));
		tmp = fscanf(fp,"%s",buf);
		myData->bData[bd].cData[ch].ChCompData.compPlus = 
			atoi(buf);
			
		tmp = fscanf(fp,"%s",temp); tmp = fscanf(fp,"%s",temp); 
		memset(buf,0x00,sizeof(buf));
		tmp = fscanf(fp,"%s",buf);
		myData->bData[bd].cData[ch].ChCompData.compMinus = 
			atoi(buf);
	}
	fclose(fp);
	return 0;
}
/*
//pms add for Resotre Memeory
int Read_Restore_Memory_Config(void)
{
	char temp[64], fileName[128];
	int tmp;
	FILE *fp;

	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/cycler_data/config/parameter/Restore_Memory_Config");
	if((fp = fopen(fileName, "r")) == NULL){
		userlog(DEBUG_LOG, psName, "Restore_Memory_Config file read error\n");
		return -1;
	}
	tmp = fscanf(fp, "%s", temp);	
	printf("%s", temp);
	tmp = fscanf(fp, "%s", temp);
	printf("%s", temp);
	tmp = fscanf(fp, "%s", temp);
	printf("%s\n", temp);
	myData->DataSave.restoreUseFlag = atoi(temp);
 	
	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	tmp = fscanf(fp, "%s", temp);
	myData->DataSave.maxFileCount = atoi(temp);
	
	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	tmp = fscanf(fp, "%s", temp);
	myData->DataSave.autoRestoreUse = atoi(temp);

	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	tmp = fscanf(fp, "%s", temp);
	myData->DataSave.min_save_peroide = atoi(temp);
	fclose(fp);
	return 0;
}
*/
// 150512 oys add start : chData Backup, Restore
int ChData_Backup(int bd, int ch, int i)
{
	int writeSize, j = 0, fp;
	char fileName[128];
// Ch Index Backup Start
	memset(fileName, 0, sizeof(fileName));
	sprintf(fileName,"/root/cycler_data/tmpData/ch%02d/ch%02d_chIndex", i+1, i+1);
	j = access(fileName, 0);
	if(j == 0){
//		userlog(DEBUG_LOG, psName,
//			"%02dCH : Previous chIndex Backup file Delete.\n", i+1);
		memset(fileName, 0, sizeof(fileName));
		sprintf(fileName,"rm -rf /root/cycler_data/tmpData/ch%02d/ch%02d_chIndex", i+1, i+1);
		system(fileName);
	}
	
	memset(fileName, 0, sizeof(fileName));
	sprintf(fileName,"touch /root/cycler_data/tmpData/ch%02d/ch%02d_chIndex", i+1, i+1);
	system(fileName);

	memset(fileName, 0, sizeof(fileName));
	sprintf(fileName,"/root/cycler_data/tmpData/ch%02d/ch%02d_chIndex", i+1, i+1);
	fp = open(fileName, O_RDWR);
	if(fp < 0){
//		userlog(DEBUG_LOG, psName,
//			"%02dCH : Can not open chIndex Backup file(save)\n",i+1);
		close(fp);
		return -1;
	}else{
		writeSize = write(fp, (char *)&(myData->DataSave.resultData[i]),
					sizeof(S_DATA_SAVE_RESULT_DATA));
		if(writeSize != sizeof(S_DATA_SAVE_RESULT_DATA)){
			userlog(DEBUG_LOG, psName,
				"%02dCH : Error chIndex Backup\n",i+1);
			return -1;
		}else{
//			userlog(DEBUG_LOG, psName,
//				"%02dCH : Write chIndex Backup file Complete\n",i+1);
		}
	}
	close(fp);
// Ch Index Backup End

// Ch Data Backup Start	
	memset(fileName, 0, sizeof(fileName));
	sprintf(fileName,"/root/cycler_data/tmpData/ch%02d/ch%02d_chData", i+1, i+1);
	j = access(fileName, 0);
	if(j == 0){
//		userlog(DEBUG_LOG, psName,
//			"%02dCH : Previous chData Backup file Delete.\n", i+1);
		memset(fileName, 0, sizeof(fileName));
		sprintf(fileName,"rm -rf /root/cycler_data/tmpData/ch%02d/ch%02d_chData", i+1, i+1);
		system(fileName);
	}
	
	memset(fileName, 0, sizeof(fileName));
	sprintf(fileName,"touch /root/cycler_data/tmpData/ch%02d/ch%02d_chData", i+1, i+1);
	system(fileName);

	memset(fileName, 0, sizeof(fileName));
	sprintf(fileName,"/root/cycler_data/tmpData/ch%02d/ch%02d_chData", i+1, i+1);
	fp = open(fileName, O_RDWR);
	if(fp < 0){
//		userlog(DEBUG_LOG, psName,
//			"%02dCH : Can not open chData Backup file(save)\n",i+1);
		close(fp);
		return -1;
	}else{
		writeSize = write(fp, (char *)&(myData->bData[bd].cData[ch]),
					sizeof(S_CH_DATA));
		if(writeSize != sizeof(S_CH_DATA)){
			userlog(DEBUG_LOG, psName,
				"%02dCH : Error chData Backup\n",i+1);
			return -1;
		}else{
//			userlog(DEBUG_LOG, psName,
//				"%02dCH : Write chData Backup file Complete\n",i+1);
		}
	}
	close(fp);
// Ch Data Backup End

// Ch testCond Backup Start	
	memset(fileName, 0, sizeof(fileName));
	sprintf(fileName,"/root/cycler_data/tmpData/ch%02d/ch%02d_testCond", i+1, i+1);
	j = access(fileName, 0);
	if(j == 0){
//		userlog(DEBUG_LOG, psName,
//			"%02dCH : Previous testCond Backup file Delete.\n", i+1);
		memset(fileName, 0, sizeof(fileName));
		sprintf(fileName,"rm -rf /root/cycler_data/tmpData/ch%02d/ch%02d_testCond", i+1, i+1);
		system(fileName);
	}
	
	memset(fileName, 0, sizeof(fileName));
	sprintf(fileName,"touch /root/cycler_data/tmpData/ch%02d/ch%02d_testCond", i+1, i+1);
	system(fileName);

	memset(fileName, 0, sizeof(fileName));
	sprintf(fileName,"/root/cycler_data/tmpData/ch%02d/ch%02d_testCond", i+1, i+1);
	fp = open(fileName, O_RDWR);
	if(fp < 0){
//		userlog(DEBUG_LOG, psName,
//			"%02dCH : Can not open testCond Backup file(save)\n",i+1);
		close(fp);
		return -1;
	}else{
		writeSize = write(fp, (char *)&(myData->mData.testCond[bd][ch]),
					sizeof(S_TEST_CONDITION));
		if(writeSize != sizeof(S_TEST_CONDITION)){
			userlog(DEBUG_LOG, psName,
				"%02dCH : Error testCond Backup\n",i+1);
			return -1;
		}else{
//			userlog(DEBUG_LOG, psName,
//				"%02dCH : Write testCond Backup file Complete\n",i+1);
		}
	}
	close(fp);
// Ch testCond Backup End

// Ch Pattern, Map file Backup Start	
	memset(fileName, 0, sizeof(fileName));
	sprintf(fileName,"/root/cycler_data/pattern/ch%02d_tmp", i+1);
	j = access(fileName, 0);
	if(j == 0){
//		userlog(DEBUG_LOG, psName,
//			"%02dCH : Previous pattern Backup file Delete.\n", i+1);
		memset(fileName, 0, sizeof(fileName));
		sprintf(fileName,"rm -rf /root/cycler_data/pattern/ch%02d_tmp", i+1);
		system(fileName);
	}
	memset(fileName, 0, sizeof(fileName));
	sprintf(fileName,"mv /root/cycler_data/pattern/ch%02d /root/cycler_data/pattern/ch%02d_tmp", i+1, i+1);
	system(fileName);

	memset(fileName, 0, sizeof(fileName));
	sprintf(fileName,"mkdir /root/cycler_data/pattern/ch%02d", i+1);
	system(fileName);
	
	memset(fileName, 0, sizeof(fileName));
	sprintf(fileName,"/root/cycler_data/map/ch%02d_tmp", i+1);
	j = access(fileName, 0);
	if(j == 0){
//		userlog(DEBUG_LOG, psName,
//			"%02dCH : Previous userMap Backup file Delete.\n", i+1);
		memset(fileName, 0, sizeof(fileName));
		sprintf(fileName,"rm -rf /root/cycler_data/map/ch%02d_tmp", i+1);
		system(fileName);
	}
	memset(fileName, 0, sizeof(fileName));
	sprintf(fileName,"mv /root/cycler_data/map/ch%02d /root/cycler_data/map/ch%02d_tmp", i+1, i+1);
	system(fileName);

	memset(fileName, 0, sizeof(fileName));
	sprintf(fileName,"mkdir /root/cycler_data/map/ch%02d", i+1);
	system(fileName);
// Ch Pattern, Map file Backup End

// Ch resultData file Backup Start
	memset(fileName, 0, sizeof(fileName));
	sprintf(fileName,"/root/cycler_data/resultData/ch%02d_tmp", i+1);
	j = access(fileName, 0);
	if(j == 0){
//		userlog(DEBUG_LOG, psName,
//			"%02dCH : previous rawData Backup file Delete.\n", i+1);
		memset(fileName, 0, sizeof(fileName));
		sprintf(fileName,"rm -rf /root/cycler_data/resultData/ch%02d_tmp", i+1);
		system(fileName);
	}
	memset(fileName, 0, sizeof(fileName));
	sprintf(fileName,"mv /root/cycler_data/resultData/ch%02d /root/cycler_data/resultData/ch%02d_tmp", i+1, i+1);
	system(fileName);

	memset(fileName, 0, sizeof(fileName));
	sprintf(fileName,"mkdir /root/cycler_data/resultData/ch%02d", i+1);
	system(fileName);

// Ch resultData file Backup End
	return 0;
}

int ChData_Restore(int bd, int ch, int i)
{
	int readSize, j = 0, fp;
	char fileName[128];

// Ch Index Restore Start
	memset(fileName, 0, sizeof(fileName));
	sprintf(fileName,"/root/cycler_data/tmpData/ch%02d/ch%02d_chIndex", i+1, i+1);
	fp = open(fileName, O_RDONLY);
	if(fp < 0){
		userlog(DEBUG_LOG, psName,
			"%02dCH : Can not open chIndex Backup file(load)\n",i+1);
		close(fp);
		return -1;
	}else{
		memset((char *)&myData->DataSave.resultData[i],
				0, sizeof(S_DATA_SAVE_RESULT_DATA));
		readSize = read(fp, (char *)&(myData->DataSave.resultData[i]),
					sizeof(S_DATA_SAVE_RESULT_DATA));
		if(readSize != sizeof(S_DATA_SAVE_RESULT_DATA)){	
			userlog(DEBUG_LOG, psName,
				"%02dCH : Error chIndex Restore\n",i+1);
			return -1;
		}else{
			memset(fileName, 0, sizeof(fileName));
			sprintf(fileName,"rm -rf /root/cycler_data/tmpData/ch%02d/ch%02d_chIndex", i+1, i+1);
			system(fileName);
//			userlog(DEBUG_LOG, psName,
//				"%02dCH : Read chIndex Backup file Complete\n",i+1);
		}
	}
	close(fp);
// Ch Index Restore End

// Ch Data Restore Start
	memset(fileName, 0, sizeof(fileName));
	sprintf(fileName,"/root/cycler_data/tmpData/ch%02d/ch%02d_chData", i+1, i+1);
	fp = open(fileName, O_RDONLY);
	if(fp < 0){
		userlog(DEBUG_LOG, psName,
			"%02dCH : Can not open chData Backup file(load)\n",i+1);
		close(fp);
		return -1;
	}else{
		memset((char *)&myData->bData[bd].cData[ch],
				0, sizeof(S_CH_DATA));
		readSize = read(fp, (char *)&(myData->bData[bd].cData[ch]),
					sizeof(S_CH_DATA));
		if(readSize != sizeof(S_CH_DATA)){	
			userlog(DEBUG_LOG, psName,
				"%02dCH : Error chData Restore\n",i+1);
			return -1;
		}else{
			memset(fileName, 0, sizeof(fileName));
			sprintf(fileName,"rm -rf /root/cycler_data/tmpData/ch%02d/ch%02d_chData", i+1, i+1);
			system(fileName);
//			userlog(DEBUG_LOG, psName,
//				"%02dCH : Read chData Backup file Complete\n",i+1);
		}
	}
	close(fp);
// Ch Data Restore End

// Ch testCond Restore Start
	memset(fileName, 0, sizeof(fileName));
	sprintf(fileName,"/root/cycler_data/tmpData/ch%02d/ch%02d_testCond", i+1, i+1);
	fp = open(fileName, O_RDONLY);
	if(fp < 0){
		userlog(DEBUG_LOG, psName,
			"%02dCH : Can not open testCond Backup file(load)\n",i+1);
		close(fp);
		return -1;
	}else{
		memset((char *)&myData->mData.testCond[bd][ch],
				0, sizeof(S_TEST_CONDITION));
		readSize = read(fp, (char *)&(myData->mData.testCond[bd][ch]),
					sizeof(S_TEST_CONDITION));
		if(readSize != sizeof(S_TEST_CONDITION)){	
			userlog(DEBUG_LOG, psName,
				"%02dCH : Error testCond Restore\n",i+1);
			return -1;
		}else{
			memset(fileName, 0, sizeof(fileName));
			sprintf(fileName,"rm -rf /root/cycler_data/tmpData/ch%02d/ch%02d_testCond", i+1, i+1);
			system(fileName);
//			userlog(DEBUG_LOG, psName,
//				"%02dCH : Read testCond Backup file Complete\n",i+1);
		}
	}
	close(fp);
// Ch testCond Restore End

// Ch Pattern, Map file Restore Start
	memset(fileName, 0, sizeof(fileName));
	sprintf(fileName,"/root/cycler_data/pattern/ch%02d_tmp", i+1);
	j = access(fileName, 0);
	if(j == 0){
		memset(fileName, 0, sizeof(fileName));
		sprintf(fileName,"rm -rf /root/cycler_data/pattern/ch%02d", i+1);
		system(fileName);

		memset(fileName, 0, sizeof(fileName));
		sprintf(fileName,"mv /root/cycler_data/pattern/ch%02d_tmp /root/cycler_data/pattern/ch%02d", i+1, i+1);
		system(fileName);
	}else{
		userlog(DEBUG_LOG, psName,
			"%02dCH : Can not open pattern Backup file(load)\n",i+1);
		return -1;
	}
	memset(fileName, 0, sizeof(fileName));
	sprintf(fileName,"/root/cycler_data/map/ch%02d_tmp", i+1);
	j = access(fileName, 0);
	if(j == 0){	
		memset(fileName, 0, sizeof(fileName));
		sprintf(fileName,"rm -rf /root/cycler_data/map/ch%02d", i+1);
		system(fileName);

		memset(fileName, 0, sizeof(fileName));
		sprintf(fileName,"mv /root/cycler_data/map/ch%02d_tmp /root/cycler_data/map/ch%02d", i+1, i+1);
		system(fileName);
	}else{
		userlog(DEBUG_LOG, psName,
			"%02dCH : Can not open userMap Backup file(load)\n",i+1);
		return -1;
	}
// Ch Pattern, Map file Restore End

// Ch resultData file Restore Start
	memset(fileName, 0, sizeof(fileName));
	sprintf(fileName,"/root/cycler_data/resultData/ch%02d_tmp", i+1);
	j = access(fileName, 0);
	if(j == 0){
		memset(fileName, 0, sizeof(fileName));
		sprintf(fileName,"rm -rf /root/cycler_data/resultData/ch%02d", i+1);
		system(fileName);

		memset(fileName, 0, sizeof(fileName));
		sprintf(fileName,"mv /root/cycler_data/resultData/ch%02d_tmp /root/cycler_data/resultData/ch%02d", i+1, i+1);
		system(fileName);
	}else{
		userlog(DEBUG_LOG, psName,
			"%02dCH : Can not open resultData Backup file(load)\n",i+1);
		return -1;
	}
	// Ch resultData file Restore End
	return 0;
}
// 150512 oys add end : chData Backup, Restore
//

//2015.11.15 khk add start
int	Read_HwFault_Config(void)
{
    int tmp, i;
	char temp[32], buf[12], fileName[128];
    FILE *fp;

	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/cycler_data/config/parameter/HwFault_Config");
	// /root/cycler_data/config/parameter/HwFault_Cofig
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "HwFault_Config file read error\n");
		system("cp ../Config_backup/HwFault_Config /root/cycler_data/config/parameter/");
		userlog(DEBUG_LOG, psName, "HwFault_Config file copy\n");
    	if((fp = fopen(fileName, "r")) == NULL) {
			return -1;
		}
	}

	for(i = 0; i <MAX_HW_FAULT_NUM; i++){
	    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
		memset(buf, 0x00, sizeof buf);
   		tmp = fscanf(fp, "%s", buf);
		myData->mData.config.hwFaultConfig[i] = atol(buf);
	}

    fclose(fp);
	return 0;
}

//210428 hun
int	Read_SwFault_Config(void)
{
    int tmp, i;
	char temp[64], buf[12], fileName[128];
    FILE *fp;

	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/cycler_data/config/parameter/SwFault_Config");
	// /root/cycler_data/config/parameter/HwFault_Cofig
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "SwFault_Config file read error\n");
		system("cp ../Config_backup/SwFault_Config /root/cycler_data/config/parameter/");
		userlog(DEBUG_LOG, psName, "SwFault_Config file copy\n");
    	if((fp = fopen(fileName, "r")) == NULL) {
			return -1;
		}
	}

	for(i = 0; i < 20 ; i++){
	    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
		memset(buf, 0x00, sizeof buf);
   		tmp = fscanf(fp, "%s", buf);
		myData->mData.config.swFaultConfig[i] = atol(buf);
	}

    fclose(fp);
	return 0;
}

//hun_211027
#ifdef _LGES
int Read_LGES_Fault_Config_Update(void)
{
	int i = 0;
	int size = 0;
	char *fileBuf, *ptr;
    FILE *fp = fopen("/root/cycler_data/config/parameter/1_LGES_parameter/faultConfig_update","r");
    FILE *fp1 = fopen("/root/cycler_data/config/parameter/1_LGES_parameter/faultConfig","a");
    
	if((fp == NULL || fp1 == NULL)){
		userlog(DEBUG_LOG, psName, "faultConfig_Update file empty\n");
		return 0;
	}

	while(!feof(fp)){
		fgetc(fp);
		size++;
	}
	userlog(DEBUG_LOG, psName, "faultConfig_Update file size : %d\n",size);
	
	fileBuf = (char *)malloc(size);
	ptr = fileBuf;
	fseek(fp, 0, SEEK_SET);
	while(!feof(fp)){
		 *ptr++ = fgetc(fp);
	}
	ptr = fileBuf;
	for(i = 0 ; i < size -1 ; i++){
		fputc(*ptr++, fp1);
	}	
	fclose(fp);
	fclose(fp1);
	system("rm -rf /root/cycler_data/config/parameter/1_LGES_parameter/faultConfig_update");
	userlog(DEBUG_LOG, psName, "faultConfig_Update file delete complete\n");
	return 0;
}

int	Read_LGES_Fault_Config_Version_Check(void)
{
	char fileName[128];
	int line = 0;
	int line2 = 0;
	char c;
	char c1;
	int size = 0;
	int write_flag = 0;
	int i = 0;
	char *fileBuf, *ptr;
    FILE *fp;

	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/cycler_data/config/parameter/1_LGES_parameter/faultConfig");
	// /root/cycler_data/config/parameter/1_LGES_parameter/faultConfig
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "LGES_FaultConfig file read error\n");
		system("cp ../Config_backup/1_LGES_parameter/faultConfig /root/cycler_data/config/parameter/1_LGES_parameter/faultConfig");
		userlog(DEBUG_LOG, psName, "LGES_FaultConfig_Config file copy\n");
    	if((fp = fopen(fileName, "r")) == NULL) {
			return -1;
		}
	}
	while(!feof(fp)){
		c = fgetc(fp);
		if(c == '\n') line++;
		if(line < FAULT_CONFIG_LINE){
			size++;
		}
	}
	userlog(DEBUG_LOG, psName, "faultConfig file line count : %d\n",line);
	userlog(DEBUG_LOG, psName, "faultConfig file size : %d\n",size + 1);
	if(line > FAULT_CONFIG_LINE){
		write_flag = 1; 
		userlog(DEBUG_LOG, psName, "faultConfig file line miss Match : [%d:%d]\n",line,FAULT_CONFIG_LINE);
		fileBuf = (char *)malloc(size+1);
		ptr = fileBuf;
		fseek(fp, 0, SEEK_SET);
		for(i = 0 ; i < size + 1; i++){
			*ptr++ = fgetc(fp);
		}
	}
	fclose(fp);
	if(write_flag == 1){
		fp = fopen("/root/cycler_data/config/parameter/1_LGES_parameter/faultConfig","w");
		ptr = fileBuf;
		for(i = 0 ; i < size + 1; i++){
			fputc(*ptr++, fp);
		}
		fclose(fp);
	}

	if(line < FAULT_CONFIG_LINE){
		userlog(DEBUG_LOG, psName, "Backup LGES_FaultConfig file copy\n");
		system("cp ../Config_backup/1_LGES_parameter/faultConfig /root/cycler_data/config/parameter/1_LGES_parameter/faultConfig");
    	if((fp = fopen(fileName, "r")) == NULL) return -2;
		while(!feof(fp)){
			c1 = fgetc(fp);
			if(c1 == '\n') line2++;
		}
		fclose(fp);
		if(line2 != FAULT_CONFIG_LINE){
			userlog(DEBUG_LOG, psName, "Check !!!! Backup LGES_FaultConfig file Fail\n");
			return -2;
		}
	}
	return 0;
}

//211025 hun
int Read_LGES_Ambient2(void)
{
    int tmp;
	char buf[12], fileName[128];
    FILE *fp;
	
	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/cycler_data/config/parameter/1_LGES_parameter/ambient2");
	// /root/cycler_data/config/parameter/1_LGES_parameter/ambient2
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "LGES_ambient2 file no use\n");
		myData->mData.config.ambient2 =	0;
		return 0;
	}
	memset(buf,0x00,sizeof(buf)); tmp = fscanf(fp,"%s",buf);
	myData->mData.config.ambient2 =	(long)atol(buf);
	fclose(fp);
	return 0;
}

int Read_LGES_code_buzzer_on(void)
{
    int tmp;
	char buf[12], fileName[128];
    FILE *fp;
	
	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/cycler_data/config/parameter/1_LGES_parameter/code_buzzer_on");
	// /root/cycler_data/config/parameter/1_LGES_parameter/code_buzzer_on
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "LGES_Code_Buzzer_On file no use\n");
		myData->mData.config.code_buzzer_on = 0;
		return 0;
	}
	memset(buf,0x00,sizeof(buf)); tmp = fscanf(fp,"%s",buf);
	myData->mData.config.code_buzzer_on = (long)atol(buf);
	fclose(fp);
	return 0;

}

//210218 For LGES Semi_Auto_jig_DCR_Calculate LJS
int Read_Semi_Autojig_DCR_Calculate(void)
{
    int tmp;
	char buf[12], fileName[128];
    FILE *fp;
	
	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/cycler_data/config/parameter/1_LGES_parameter/Semi_Autojig_DCR_Calculate");
	// /root/cycler_data/config/parameter/1_LGES_parameter/Semi_Autojig_DCR_Calculate
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "LGES_Semi_Autojig_DCR_Calculate file no use\n");
		myData->mData.config.Semi_Autojig_DCR_Calculate = 0;
		return 0;
	}
	memset(buf,0x00,sizeof(buf)); tmp = fscanf(fp,"%s",buf);
	myData->mData.config.Semi_Autojig_DCR_Calculate = (long)atol(buf);
	fclose(fp);
	return 0;

}

int	Read_LGES_Fault_Config(void)
{
    int tmp;
	char temp[64], buf[12], fileName[128];
    FILE *fp;

	if(FAULT_CONFIG_VERSION == 0) return 0;

	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/cycler_data/config/parameter/1_LGES_parameter/faultConfig");
	// /root/cycler_data/config/parameter/1_LGES_parameter/faultConfig
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "LGES_FaultConfig file read error\n");
		system("cp ../Config_backup/1_LGES_parameter/faultConfig /root/cycler_data/config/parameter/1_LGES_parameter/faultConfig");
		userlog(DEBUG_LOG, psName, "LGES_FaultConfig_Config file copy\n");
    	if((fp = fopen(fileName, "r")) == NULL) {
			return -1;
		}
	}
	if(FAULT_CONFIG_VERSION >= 1){
		tmp = fscanf(fp,"%s",temp); tmp = fscanf(fp,"%s",temp); 
		memset(buf,0x00,sizeof(buf)); tmp = fscanf(fp,"%s",buf);
		myData->mData.config.LGES_fault_config.ovp_check_time =	(long)atol(buf);
	
		tmp = fscanf(fp,"%s",temp); tmp = fscanf(fp,"%s",temp); 
		memset(buf,0x00,sizeof(buf)); tmp = fscanf(fp,"%s",buf);
		myData->mData.config.LGES_fault_config.otp_check_time =	(long)atol(buf);
   	 
		tmp = fscanf(fp,"%s",temp); tmp = fscanf(fp,"%s",temp); 
		memset(buf,0x00,sizeof(buf)); tmp = fscanf(fp,"%s",buf);
		myData->mData.config.LGES_fault_config.drop_v_charge_start_time = (long)atol(buf);
   	 
		tmp = fscanf(fp,"%s",temp); tmp = fscanf(fp,"%s",temp); 
		memset(buf,0x00,sizeof(buf)); tmp = fscanf(fp,"%s",buf);
		myData->mData.config.LGES_fault_config.drop_v_discharge_start_time = (long)atol(buf);
	    
		tmp = fscanf(fp,"%s",temp); tmp = fscanf(fp,"%s",temp); 
		memset(buf,0x00,sizeof(buf)); tmp = fscanf(fp,"%s",buf);
		myData->mData.config.LGES_fault_config.drop_v_charge_check_time = (long)atol(buf);
   	 
		tmp = fscanf(fp,"%s",temp); tmp = fscanf(fp,"%s",temp); 
		memset(buf,0x00,sizeof(buf)); tmp = fscanf(fp,"%s",buf);
		myData->mData.config.LGES_fault_config.drop_v_discharge_check_time = (long)atol(buf);
    
		tmp = fscanf(fp,"%s",temp); tmp = fscanf(fp,"%s",temp); 
		memset(buf,0x00,sizeof(buf)); tmp = fscanf(fp,"%s",buf);
		myData->mData.config.LGES_fault_config.cv_voltage_start_time = (long)atol(buf);
    
		tmp = fscanf(fp,"%s",temp); tmp = fscanf(fp,"%s",temp); 
		memset(buf,0x00,sizeof(buf)); tmp = fscanf(fp,"%s",buf);
		myData->mData.config.LGES_fault_config.cv_current_start_time = (long)atol(buf);
    
		tmp = fscanf(fp,"%s",temp); tmp = fscanf(fp,"%s",temp); 
		memset(buf,0x00,sizeof(buf)); tmp = fscanf(fp,"%s",buf);
		myData->mData.config.LGES_fault_config.ovp_pause_flag = (long)atol(buf);
    
		tmp = fscanf(fp,"%s",temp); tmp = fscanf(fp,"%s",temp); 
		memset(buf,0x00,sizeof(buf)); tmp = fscanf(fp,"%s",buf);
		myData->mData.config.LGES_fault_config.otp_pause_flag = (long)atol(buf);
    
		tmp = fscanf(fp,"%s",temp); tmp = fscanf(fp,"%s",temp); 
		memset(buf,0x00,sizeof(buf)); tmp = fscanf(fp,"%s",buf);
		myData->mData.config.LGES_fault_config.limit_time_v = (long)atol(buf);
	}else if(FAULT_CONFIG_VERSION >= 2){
	}
	fclose(fp);
	return 0;
}

int Write_LGES_Fault_Config(void) 
{ //210204 lyhw
	char fileName[128];
    FILE *fp;

	if(FAULT_CONFIG_VERSION == 0) return 0;
	
	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/cycler_data/config/parameter/1_LGES_parameter/faultConfig");
    if((fp = fopen(fileName, "w")) == NULL) {
		userlog(DEBUG_LOG, psName, "LGES faultConfig file write error\n");
		return -1;
	}

	if(FAULT_CONFIG_VERSION >= 1){
		fprintf(fp, "OVP_CHECK_TIME			: %ld\n",		
				myData->mData.config.LGES_fault_config.ovp_check_time);
		fprintf(fp, "OTP_CHECK_TIME			: %ld\n",
				myData->mData.config.LGES_fault_config.otp_check_time);
		fprintf(fp, "DROP_V_Charge_start_time	: %ld\n",
				myData->mData.config.LGES_fault_config.drop_v_charge_start_time);
		fprintf(fp, "DROP_V_Discharge_start_time	: %ld\n",
				myData->mData.config.LGES_fault_config.drop_v_discharge_start_time);
		fprintf(fp, "DROP_V_charge_Check_time	: %ld\n",
				myData->mData.config.LGES_fault_config.drop_v_charge_check_time);
		fprintf(fp, "DROP_V_Discharge_Check_time	: %ld\n",
				myData->mData.config.LGES_fault_config.drop_v_discharge_check_time);
		fprintf(fp, "CV_Voltage_start_time		: %ld\n",
				myData->mData.config.LGES_fault_config.cv_voltage_start_time);
		fprintf(fp, "CV_Current_start_time		: %ld\n",
				myData->mData.config.LGES_fault_config.cv_current_start_time);
		fprintf(fp, "OVP_Pause_flag			: %ld\n",
				myData->mData.config.LGES_fault_config.ovp_pause_flag);
		fprintf(fp, "OTP_Pause_flag			: %ld\n",
				myData->mData.config.LGES_fault_config.otp_pause_flag);
		fprintf(fp, "limit_time_voltage		: %ld\n",   
				myData->mData.config.LGES_fault_config.limit_time_v);
	}else if(FAULT_CONFIG_VERSION >= 2){
	}
	fclose(fp);
	return 0;
}

#endif

#ifdef _SDI
int Read_SDI_inv_fault_continue(void)
{
    int tmp;
	char buf[12], fileName[128];
    FILE *fp;
	
	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/cycler_data/config/parameter/2_SDI_parameter/inv_fault_continue");
	// /root/cycler_data/config/parameter/2_SDI_parameter/inv_fault_continue
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "SDI_inv_fault_continue file no use\n");
		myData->mData.config.inv_fault_count = 0;
		return 0;
	}
	memset(buf,0x00,sizeof(buf)); tmp = fscanf(fp,"%s",buf);
	myData->mData.config.inv_fault_count = (long)atol(buf);
	fclose(fp);
	return 0;
}
int Read_SDI_CC_CV_hump_Config(void)
{
    int tmp;
	char temp[32], buf[12], fileName[128];
    FILE *fp;
	
	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/cycler_data/config/parameter/2_SDI_parameter/CC_CV_hump_Config");
	// /root/cycler_data/config/parameter/2_SDI_parameter/CC_CV_hump_Config
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "SDI_CC_CV_hump_Config file no use\n");
		myData->mData.config.cc_cv_hump_flag = 0;
		return 0;
	}
	myData->mData.config.cc_cv_hump_flag = 1;
	tmp = fscanf(fp,"%s",temp); tmp = fscanf(fp,"%s",temp); 
	memset(buf,0x00,sizeof(buf)); tmp = fscanf(fp,"%s",buf);
	myData->mData.config.SDI_cc_cv_hump.charge_voltage = (long)atol(buf);
	
	tmp = fscanf(fp,"%s",temp); tmp = fscanf(fp,"%s",temp); 
	memset(buf,0x00,sizeof(buf)); tmp = fscanf(fp,"%s",buf);
	myData->mData.config.SDI_cc_cv_hump.charge_current = (long)atol(buf);
	
	tmp = fscanf(fp,"%s",temp); tmp = fscanf(fp,"%s",temp); 
	memset(buf,0x00,sizeof(buf)); tmp = fscanf(fp,"%s",buf);
	myData->mData.config.SDI_cc_cv_hump.charge_cc_start_time = (long)atol(buf);

	tmp = fscanf(fp,"%s",temp); tmp = fscanf(fp,"%s",temp); 
	memset(buf,0x00,sizeof(buf)); tmp = fscanf(fp,"%s",buf);
	myData->mData.config.SDI_cc_cv_hump.charge_cv_start_time = (long)atol(buf);
	
	tmp = fscanf(fp,"%s",temp); tmp = fscanf(fp,"%s",temp); 
	memset(buf,0x00,sizeof(buf)); tmp = fscanf(fp,"%s",buf);
	myData->mData.config.SDI_cc_cv_hump.charge_cc_period_time = (long)atol(buf);
	
	tmp = fscanf(fp,"%s",temp); tmp = fscanf(fp,"%s",temp); 
	memset(buf,0x00,sizeof(buf)); tmp = fscanf(fp,"%s",buf);
	myData->mData.config.SDI_cc_cv_hump.charge_cv_period_time = (long)atol(buf);
	
	tmp = fscanf(fp,"%s",temp); tmp = fscanf(fp,"%s",temp); 
	memset(buf,0x00,sizeof(buf)); tmp = fscanf(fp,"%s",buf);
	myData->mData.config.SDI_cc_cv_hump.discharge_voltage = (long)atol(buf);
	
	tmp = fscanf(fp,"%s",temp); tmp = fscanf(fp,"%s",temp); 
	memset(buf,0x00,sizeof(buf)); tmp = fscanf(fp,"%s",buf);
	myData->mData.config.SDI_cc_cv_hump.discharge_current = (long)atol(buf);
	
	tmp = fscanf(fp,"%s",temp); tmp = fscanf(fp,"%s",temp); 
	memset(buf,0x00,sizeof(buf)); tmp = fscanf(fp,"%s",buf);
	myData->mData.config.SDI_cc_cv_hump.discharge_cc_start_time = (long)atol(buf);

	tmp = fscanf(fp,"%s",temp); tmp = fscanf(fp,"%s",temp); 
	memset(buf,0x00,sizeof(buf)); tmp = fscanf(fp,"%s",buf);
	myData->mData.config.SDI_cc_cv_hump.discharge_cv_start_time = (long)atol(buf);
	
	tmp = fscanf(fp,"%s",temp); tmp = fscanf(fp,"%s",temp); 
	memset(buf,0x00,sizeof(buf)); tmp = fscanf(fp,"%s",buf);
	myData->mData.config.SDI_cc_cv_hump.discharge_cc_period_time = (long)atol(buf);
	
	tmp = fscanf(fp,"%s",temp); tmp = fscanf(fp,"%s",temp); 
	memset(buf,0x00,sizeof(buf)); tmp = fscanf(fp,"%s",buf);
	myData->mData.config.SDI_cc_cv_hump.discharge_cv_period_time = (long)atol(buf);

	fclose(fp);
	return 0;
}
int Write_SDI_CC_CV_hump_Config(void) 
{ 
	char fileName[128];
    FILE *fp;

	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/cycler_data/config/parameter/2_SDI_parameter/CC_CV_hump_Config");
    if((fp = fopen(fileName, "w")) == NULL) {
		userlog(DEBUG_LOG, psName, "SDI_CC_CV_hump_Config file write error\n");
		myData->mData.config.cc_cv_hump_flag = 0;
		return -1;
	}
	myData->mData.config.cc_cv_hump_flag = 1;
	fprintf(fp, "CHARGE_CC_VOLTAGE(mV)		: %ld\n",		
			myData->mData.config.SDI_cc_cv_hump.charge_voltage);
	fprintf(fp, "CHARGE_CV_CURRENT(uA)		: %ld\n",		
			myData->mData.config.SDI_cc_cv_hump.charge_current);
	fprintf(fp, "CHARGE_CC_START_TIME(sec)		: %ld\n",		
			myData->mData.config.SDI_cc_cv_hump.charge_cc_start_time);
	fprintf(fp, "CHARGE_CV_START_TIME(sec)		: %ld\n",		
			myData->mData.config.SDI_cc_cv_hump.charge_cv_start_time);
	fprintf(fp, "CHARGE_CC_PERIOD_TIME(sec)		: %ld\n",		
			myData->mData.config.SDI_cc_cv_hump.charge_cc_period_time);
	fprintf(fp, "CHARGE_CV_PERIOD_TIME(sec)		: %ld\n",		
			myData->mData.config.SDI_cc_cv_hump.charge_cv_period_time);
	fprintf(fp, "DISCHARGE_CC_VOLTAGE(mV)		: %ld\n",		
			myData->mData.config.SDI_cc_cv_hump.discharge_voltage);
	fprintf(fp, "DISCHARGE_CV_CURRENT(uA)		: %ld\n",		
			myData->mData.config.SDI_cc_cv_hump.discharge_current);
	fprintf(fp, "DISCHARGE_CC_START_TIME(sec)	: %ld\n",		
			myData->mData.config.SDI_cc_cv_hump.discharge_cc_start_time);
	fprintf(fp, "DISCHARGE_CV_START_TIME(sec)	: %ld\n",		
			myData->mData.config.SDI_cc_cv_hump.discharge_cv_start_time);
	fprintf(fp, "DISCHARGE_CC_PERIOD_TIME(sec)	: %ld\n",		
			myData->mData.config.SDI_cc_cv_hump.discharge_cc_period_time);
	fprintf(fp, "DISCHARGE_CV_PERIOD_TIME(sec)	: %ld\n",		
			myData->mData.config.SDI_cc_cv_hump.discharge_cv_period_time);

	fclose(fp);
	return 0;
}
int Read_SDI_Pause_save_Config(void)
{
    int tmp;
	char temp[32], buf[12], fileName[128];
    FILE *fp;
	
	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/cycler_data/config/parameter/2_SDI_parameter/Pause_save_Config");
	// /root/cycler_data/config/parameter/2_SDI_parameter/Pause_save_Config
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "SDI_CC_CV_hump_Config file no use\n");
		myData->mData.config.sdi_pause_save_flag = 0;
		return 0;
	}
	myData->mData.config.sdi_pause_save_flag = 1;
	tmp = fscanf(fp,"%s",temp); tmp = fscanf(fp,"%s",temp); 
	memset(buf,0x00,sizeof(buf)); tmp = fscanf(fp,"%s",buf);
	myData->mData.config.SDI_pause_save.pause_end_time = (long)atol(buf);

	tmp = fscanf(fp,"%s",temp); tmp = fscanf(fp,"%s",temp); 
	memset(buf,0x00,sizeof(buf)); tmp = fscanf(fp,"%s",buf);
	myData->mData.config.SDI_pause_save.pause_period_time = (long)atol(buf);
	
	fclose(fp);
	return 0;
}
int Write_SDI_Pause_save_Config(void)
{ 
	char fileName[128];
    FILE *fp;

	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/cycler_data/config/parameter/2_SDI_parameter/Pause_save_Config");
    if((fp = fopen(fileName, "w")) == NULL) {
		userlog(DEBUG_LOG, psName, "SDI_Pause_save_Config file write error\n");
		myData->mData.config.sdi_pause_save_flag = 0;
		return -1;
	}else{
		myData->mData.config.sdi_pause_save_flag = 1;
	}
	fprintf(fp, "PAUSE_END_TIME(sec) : %ld\n",		
			myData->mData.config.SDI_pause_save.pause_end_time);
	fprintf(fp, "PAUSE_PERIOD_TIME(sec) : %ld\n",		
			myData->mData.config.SDI_pause_save.pause_period_time);
	fclose(fp);
	return 0;
}

#endif

int Read_Function_Flag(void)
{
    int tmp, i;
	char temp[32], buf[12], fileName[128];
    FILE *fp;

	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/cycler_data/config/parameter/FUNCTION");
	// /root/cycler_data/config/parameter/FUNCTION
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "FUNCTION file read error\n");
		system("cp ../Config_backup/FUNCTION /root/cycler_data/config/parameter");
		userlog(DEBUG_LOG, psName, "FUNCTION file copy\n");
    	if((fp = fopen(fileName, "r")) == NULL) {
			return -1;
		}
	}

	for(i = 0; i < MAX_FUNCTION_NUM; i++){
	    tmp = fscanf(fp, "%s", temp); 	
		tmp = fscanf(fp, "%s", temp);
		memset(buf, 0x00, sizeof buf);
   	 	tmp = fscanf(fp, "%s", buf);
   	 	myData->mData.config.function[i] = (unsigned char)atoi(buf);
	}

    fclose(fp);
	return 0;
}
//2015.11.15 khk add end
//
int	Read_Chamber_Motion(void)
{
    int tmp, i;
	char temp[32], buf[12], fileName[128];
    FILE *fp;

	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/cycler_data/config/parameter/Chamber_Motion");
	// /root/cycler_data/config/parameter/Chamber_Motion
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "Chamber_Motion file read error\n");
		system("cp ../Config_backup/Chamber_Motion /root/cycler_data/config/parameter");
		userlog(DEBUG_LOG, psName, "Chamber_Motion file copy\n");
		if((fp = fopen(fileName, "r")) == NULL) {
			return -1;
		}
	} else {
		system("rm -rf ../Config_backup/Chamber_Motion");
		system("cp /root/cycler_data/config/parameter/Chamber_Motion ../Config_backup");
	}

	for(i = 0; i <MAX_CHAMBERMOTION_NUM; i++){
	    tmp = fscanf(fp, "%s", temp); 	
		tmp = fscanf(fp, "%s", temp);
		memset(buf, 0x00, sizeof buf);
   		tmp = fscanf(fp, "%s", buf);
		myData->mData.config.ChamberMotion[i] = atol(buf);
	}
 	//191118 add
	tmp = fscanf(fp, "%s", temp); 	
	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
   	tmp = fscanf(fp, "%s", buf);
	myData->mData.config.ChamberDioUse = atol(buf);

	tmp = fscanf(fp, "%s", temp); 	
	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
   	tmp = fscanf(fp, "%s", buf);
	myData->mData.config.ChamberPerUnit = atol(buf);

	tmp = fscanf(fp, "%s", temp); 	
	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
   	tmp = fscanf(fp, "%s", buf);
	myData->mData.config.ChamberPerSigNum = atol(buf);
	
    fclose(fp);
	return 0;
}

//20171008 sch add
int Write_LimitUserVI(void)
{
	char fileName[128];
    FILE *fp;

	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/cycler_data/config/parameter/LimitUserVI");
    if((fp = fopen(fileName, "w")) == NULL) {
		userlog(DEBUG_LOG, psName, "LimitUserVI file write error\n");
		return -1;
	}
	fprintf(fp, "limit_current_use		: %d\n",
				myData->mData.config.LimitVI.limit_use_I);
	fprintf(fp, "limit_current_action	: %d\n",
				myData->mData.config.LimitVI.limit_action_I);
	fprintf(fp, "limit_current_CH_act	: %d\n",
				myData->mData.config.LimitVI.limit_Ch_I);
	fprintf(fp, "limit_current			: %li\n",
				myData->mData.config.LimitVI.limit_current);
	fprintf(fp, "limit_current_time		: %li\n",
				myData->mData.config.LimitVI.limit_time_I);
	fprintf(fp, "limit_voltage_use		: %d\n",
				myData->mData.config.LimitVI.limit_use_V);
	fprintf(fp, "limit_voltage_action	: %d\n",
				myData->mData.config.LimitVI.limit_action_V);
	fprintf(fp, "limit_voltage_CH_act	: %d\n",
				myData->mData.config.LimitVI.limit_Ch_V);
	fprintf(fp, "limit_voltage			: %li\n",
				myData->mData.config.LimitVI.limit_voltage);
	fprintf(fp,"\n");
	
	fclose(fp);
	return 0;
}

int Write_HwFault_Config(void)  
{ //210204 lyhw
	char fileName[128];
    FILE *fp;

	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/cycler_data/config/parameter/HwFault_Config");
    if((fp = fopen(fileName, "w")) == NULL) {
		userlog(DEBUG_LOG, psName, "HwFault_Config file write error\n");
		return -1;
	}

	fprintf(fp, "OVP(uV)					: %ld\n",
			myData->mData.config.hwFaultConfig[HW_FAULT_OVP]);
	fprintf(fp, "ChCheckCurrent(uA)		: %ld\n",
			myData->mData.config.hwFaultConfig[HW_FAULT_CCC]);
	fprintf(fp, "OT(1'C=1000)			: %ld\n",
			myData->mData.config.hwFaultConfig[HW_FAULT_OTP]);
	fprintf(fp, "Drop_V_Charge(uV)		: %ld\n",
			myData->mData.config.hwFaultConfig[HW_FAULT_DROP_V_1]);
	fprintf(fp, "Drop_V_DisCharge(uV)	: %ld\n",
			myData->mData.config.hwFaultConfig[HW_FAULT_DROP_V_2]);
	fprintf(fp, "CV_Voltage_Drop(uV)		: %ld\n",
			myData->mData.config.hwFaultConfig[HW_FAULT_CV_VOLTAGE]);
	fprintf(fp, "CV_Current_Drop(uV)		: %ld\n",
			myData->mData.config.hwFaultConfig[HW_FAULT_CV_CURRENT]);
	fprintf(fp,"\n");
	
	fclose(fp);
	return 0;
}

//210428 hun
int Write_SwFault_Config(void)  
{ 
	char fileName[128];
    FILE *fp;

	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/cycler_data/config/parameter/SwFault_Config");
    if((fp = fopen(fileName, "w")) == NULL) {
		userlog(DEBUG_LOG, psName, "SwFault_Config file write error\n");
		return -1;
	}
	fprintf(fp, "CHAMBER_GAS_VOLTAGE_MIN(uV)		: %ld\n",
			myData->mData.config.swFaultConfig[CHAMBER_GAS_VOLTAGE_MIN]);
	fprintf(fp, "CHAMBER_GAS_VOLTAGE_MAX(uV)		: %ld\n",
			myData->mData.config.swFaultConfig[CHAMBER_GAS_VOLTAGE_MAX]);
	fprintf(fp, "REST_CHECK_START_TIME(s)			: %ld\n",
			myData->mData.config.swFaultConfig[REST_CHECK_START_TIME]);
	fprintf(fp, "REST_START_COMPARE_VOLTAGE(uV)		: %ld\n",
			myData->mData.config.swFaultConfig[REST_START_COMPARE_VOLTAGE]);
	fprintf(fp, "REST_COMPARE_VOLTAGE_DELTA_V1(uV)	: %ld\n",
			myData->mData.config.swFaultConfig[REST_COMPARE_VOLTAGE_DELTA_V1]);
	fprintf(fp, "REST_COMPARE_VOLTAGE_DELTA_V2(uV)	: %ld\n",
			myData->mData.config.swFaultConfig[REST_COMPARE_VOLTAGE_DELTA_V2]);
	fprintf(fp, "REST_FAULT_CHECK_COUNT				: %ld\n",
			myData->mData.config.swFaultConfig[REST_FAULT_CHECK_COUNT]);
	fprintf(fp, "GAS_CHECK_TIME(s)					: %ld\n",
			myData->mData.config.swFaultConfig[GAS_CHECK_TIME]);
	fprintf(fp, "AMBIENT_TEMP_MAX					: %ld\n",
			myData->mData.config.swFaultConfig[AMBIENT_TEMP_MAX]);
	fprintf(fp, "AMBIENT_TEMP_MAX_CHECK_TIME(s)		: %ld\n",
			myData->mData.config.swFaultConfig[AMBIENT_TEMP_MAX_TIME]);
	fprintf(fp, "AMBIENT_TEMP_DIFF					: %ld\n",
			myData->mData.config.swFaultConfig[AMBIENT_TEMP_DIFF]);
	fprintf(fp, "AMBIENT_TEMP_DIFF_CHECK_TIME(s)	: %ld\n",
			myData->mData.config.swFaultConfig[AMBIENT_TEMP_DIFF_TIME]);
	fprintf(fp, "AMBIENT_TEMP_DIFF_CHECK_TIME(s)	: %ld\n",
			myData->mData.config.swFaultConfig[AMBIENT_TEMP_DIFF_TIME]);
	fprintf(fp, "SOFT_VENTING_COUNT					: %ld\n",
			myData->mData.config.swFaultConfig[SOFT_VENTING_COUNT]);
	fprintf(fp, "SOFT_VENTING_VALUE(uV)				: %ld\n",
			myData->mData.config.swFaultConfig[SOFT_VENTING_VALUE]);
	fprintf(fp, "HARD_VENTING_VALUE(uV)				: %ld\n",
			myData->mData.config.swFaultConfig[HARD_VENTING_VALUE]);

	fprintf(fp,"\n");
	
	fclose(fp);
	return 0;
}

int Read_DYSON_Maintenance_Config(void)
{
    int tmp;
	char temp[32], buf[12], fileName[128];
    FILE *fp;
	
	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/cycler_data/config/parameter/61_DYSON_parameter/Maintenance_Config");
	// /root/cycler_data/config/parameter/61_DYSON_parameter/Maintenance_Config
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "Maintenance_Config file no use\n");
		myData->mData.config.dyson_maintenance_flag = 0;
		return 0;
	}
	myData->mData.config.dyson_maintenance_flag = 1;
	tmp = fscanf(fp,"%s",temp); tmp = fscanf(fp,"%s",temp); 
	memset(buf,0x00,sizeof(buf)); tmp = fscanf(fp,"%s",buf);
	myData->mData.config.DYSON_maintenance.door_pause_flag = (long)atol(buf);
	
	fclose(fp);
	return 0;
}
int Write_DYSON_Maintenance_Config(void)
{ 
	char fileName[128];
    FILE *fp;

	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/cycler_data/config/parameter/61_DYSON_parameter/Maintenance_Config");
    if((fp = fopen(fileName, "w")) == NULL) {
		userlog(DEBUG_LOG, psName, "Maintenance_Config file write error\n");
		myData->mData.config.dyson_maintenance_flag = 0;
		return -1;
	}
	myData->mData.config.dyson_maintenance_flag = 1;
	fprintf(fp, "DOOR_PAUSE_FLAG : %ld\n",		
			myData->mData.config.DYSON_maintenance.door_pause_flag);
	fclose(fp);
	return 0;
}

int Read_Parameter_Update(void)
{
	if(myData->MainClient.signal[MAIN_SIG_PARAMETER_UPDATE] == P1){
		myData->MainClient.signal[MAIN_SIG_PARAMETER_UPDATE] = P0;
		if(Read_mControl_Config() < 0) 		return -1;
		if(Read_DIO_Config() < 0) 			return -2;
		if(Read_Function_Flag() < 0) 		return -3; 
		if(Read_CellArray_A() < 0) 			return -4;
		if(Read_FaultBitSet_SMPS_OT() < 0) 	return -5;
//		if(myPs->config.versionNo >= 20110429) { //20200804 
		if(Read_LoadProcess() < 0)	 		return -6;
//		}
		if(Read_HwFault_Config() < 0)		 return -7; 	
		if(Read_ChamArray_A() < 0)			 return -8;
		if(Read_Chamber_Motion() < 0) 		 return -9; 
		if(Read_LimitUserVI() < 0) 			 return -10; 
#if CYCLER_TYPE == DIGITAL_CYC
		if(Read_PCU_Config() < 0) 		 return -11;	 
		if(Read_PCU_INV_USE_FLAG() < 0)  return -12;
#endif
		if(Read_TempFault_Config() < 0)		return -13; 
		if(Read_SwFault_Config() < 0)		return -14; 
		if(Read_ChamberChNo() < 0){	//211019 hun
			if(Write_Default_ChamberChNo() < 0){
				return -15;
			}
		}
		#ifdef _LGES
		if(Read_LGES_Fault_Config() < 0)	return -16;	//hun_211025
		#endif
	}else{
		userlog(DEBUG_LOG, psName, "Parameter Update Fail...\n");
		return -20;
	}
	userlog(DEBUG_LOG, psName, "Parameter Update Complete\n");
	return 0;
}

int Read_LimitUserVI(void)
{
	char temp[64], buf[64], fileName[128];
    int tmp;
    FILE *fp;

	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/cycler_data/config/parameter/LimitUserVI");
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "LimitUserVI file Read error\n");
		system("cp ../Config_backup/LimitUserVI /root/cycler_data/config/parameter");
		userlog(DEBUG_LOG, psName, "LimitUserVI file copy\n");
		if((fp = fopen(fileName, "r")) == NULL) {
			return -1;
		}
	}
	tmp = fscanf(fp,"%s",temp); tmp = fscanf(fp,"%s",temp); 
	memset(buf,0x00,sizeof(buf));
	tmp = fscanf(fp,"%s",buf);
	myData->mData.config.LimitVI.limit_use_I = 
		(unsigned char)atoi(buf);
	tmp = fscanf(fp,"%s",temp); tmp = fscanf(fp,"%s",temp); 
	memset(buf,0x00,sizeof(buf));
	tmp = fscanf(fp,"%s",buf);
	myData->mData.config.LimitVI.limit_action_I = 
		(unsigned char)atoi(buf);
	tmp = fscanf(fp,"%s",temp); tmp = fscanf(fp,"%s",temp); 
	memset(buf,0x00,sizeof(buf));
	tmp = fscanf(fp,"%s",buf);
	myData->mData.config.LimitVI.limit_Ch_I = 
		(unsigned char)atoi(buf);
	tmp = fscanf(fp,"%s",temp); tmp = fscanf(fp,"%s",temp); 
	memset(buf,0x00,sizeof(buf));
	tmp = fscanf(fp,"%s",buf);
	myData->mData.config.LimitVI.limit_current = 
		(long)atoi(buf);
	tmp = fscanf(fp,"%s",temp); tmp = fscanf(fp,"%s",temp); 
	memset(buf,0x00,sizeof(buf));
	tmp = fscanf(fp,"%s",buf);
	myData->mData.config.LimitVI.limit_time_I = 
		(long)atoi(buf);
	tmp = fscanf(fp,"%s",temp); tmp = fscanf(fp,"%s",temp); 
	memset(buf,0x00,sizeof(buf));
	tmp = fscanf(fp,"%s",buf);
	myData->mData.config.LimitVI.limit_use_V = 
		(unsigned char)atoi(buf);
	tmp = fscanf(fp,"%s",temp); tmp = fscanf(fp,"%s",temp); 
	memset(buf,0x00,sizeof(buf));
	tmp = fscanf(fp,"%s",buf);
	myData->mData.config.LimitVI.limit_action_V = 
		(unsigned char)atoi(buf);
	tmp = fscanf(fp,"%s",temp); tmp = fscanf(fp,"%s",temp); 
	memset(buf,0x00,sizeof(buf));
	tmp = fscanf(fp,"%s",buf);
	myData->mData.config.LimitVI.limit_Ch_V = 
		(unsigned char)atoi(buf);
	tmp = fscanf(fp,"%s",temp); tmp = fscanf(fp,"%s",temp); 
	memset(buf,0x00,sizeof(buf));
	tmp = fscanf(fp,"%s",buf);
	myData->mData.config.LimitVI.limit_voltage = 
		(long)atoi(buf);
	fclose(fp);
	return 0;
}

int Read_PCU_Config(void)
{	//180612 add
	char temp[32], buf[12], fileName[128];
    int tmp;
    FILE *fp;

	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/cycler_data/config/parameter/PCU_Config");
	// /root/cycler_data/config/parameter/PCU_Config
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "PCU_Config file read error\n");
		system("cp ../Config_backup/PCU_Config /root/cycler_data/config/parameter");
		userlog(DEBUG_LOG, psName, "PCU_Config file copy\n");
		if((fp = fopen(fileName, "r")) == NULL) {
			return -1;
		}
	}
	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->mData.pcu_config.portPerCh = (unsigned char)atoi(buf);
		
	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->mData.pcu_config.installedInverter = (unsigned char)atoi(buf);

	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->mData.pcu_config.hallCT = (double)atof(buf);

	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->mData.pcu_config.caliV_ohm = (float)atof(buf);

	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->mData.pcu_config.pcuCaliUse = (unsigned short)atoi(buf);

	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->mData.pcu_config.powerNo = (unsigned char)atoi(buf);

	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->mData.pcu_config.caliV_amp = (float)atof(buf);

	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->mData.pcu_config.setCaliNum = (unsigned short)atoi(buf);
	if(myData->mData.pcu_config.setCaliNum == 0){
		myData->mData.pcu_config.setCaliNum = 3;
	}

	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	
	myData->mData.pcu_config.cali_delay_time = (unsigned short)atoi(buf)/10;
	if(myData->mData.pcu_config.cali_delay_time == 0){
		myData->mData.pcu_config.cali_delay_time = 500;
	}

	//180620 lyh
	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->mData.pcu_config.invPerCh = (unsigned char)atoi(buf);
	
	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->mData.pcu_config.parallel_inv_ch = (unsigned char)atoi(buf);
	
	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->mData.pcu_config.inverterType = (unsigned char)atoi(buf);

	fclose(fp);
	return 0;
}

//20190605 KHK--------------------------------------
int Read_CAN_Config(void)
{
    int tmp, i;
	char temp[32], buf[12], fileName[128];
    FILE *fp;

	memset(fileName, 0, sizeof fileName);
	strcpy(fileName, "/root/cycler_data/config/parameter/CAN_Config");
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "CAN_Config file read error\n");
		system("cp ../Config_backup/CAN_Config /root/cycler_data/config/parameter/");
		userlog(DEBUG_LOG, psName, "CAN_Config file copy\n");
    	if((fp = fopen(fileName, "r")) == NULL) {
			return -1;
		}
	}

	//can use flag
    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myData->CAN.config.canUseFlag = (unsigned char)atoi(buf);

    if(myData->CAN.config.canUseFlag == 0) return 0; 

	//Read Installed can
    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myData->CAN.config.installedCAN = (unsigned short)atoi(buf);

	//Read string
	for(i=0; i < MAX_CAN_PORT; i++) {
	    tmp = fscanf(fp, "%s", buf);
	}

	//Read function Type
	tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	for(i=0; i < MAX_CAN_PORT; i++) {
		memset(buf, 0, sizeof buf);
	    tmp = fscanf(fp, "%s", buf);
    	myData->CAN.config.functionType[i] = (unsigned char)atoi(buf);
	}
	//Read CanPort
	tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	for(i=0; i < MAX_CAN_PORT; i++) {
		memset(buf, 0, sizeof buf);
	    tmp = fscanf(fp, "%s", buf);
    	myData->CAN.config.canPort[i] = atoi(buf);
	}

	//Read CanBps
	tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	for(i=0; i < MAX_CAN_PORT; i++) {
		memset(buf, 0, sizeof buf);
	    tmp = fscanf(fp, "%s", buf);
    	myData->CAN.config.canBps[i] = atoi(buf);
	}

	//Read CommType
	tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	for(i=0; i < MAX_CAN_PORT; i++) {
		memset(buf, 0, sizeof buf);
	    tmp = fscanf(fp, "%s", buf);
    	myData->CAN.config.commType[i] = (unsigned char)atoi(buf);
	}

	//Read cmdsendlog
	tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	for(i=0; i < MAX_CAN_PORT; i++) {
		memset(buf, 0, sizeof buf);
	    tmp = fscanf(fp, "%s", buf);
    	myData->CAN.config.CmdSendLog[i] = (unsigned char)atoi(buf);
	}

	//Read cmdrcvlog
	tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	for(i=0; i < MAX_CAN_PORT; i++) {
		memset(buf, 0, sizeof buf);
	    tmp = fscanf(fp, "%s", buf);
    	myData->CAN.config.CmdRcvLog[i] = (unsigned char)atoi(buf);
	}

	//Read CommTimeOut
	tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	for(i=0; i < MAX_CAN_PORT; i++) {
		memset(buf, 0, sizeof buf);
	    tmp = fscanf(fp, "%s", buf);
    	myData->CAN.config.canCommTimeOut[i] = atol(buf);
	}

	//Read Installed Device Num
	tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	for(i=0; i < MAX_CAN_PORT; i++) {
		memset(buf, 0, sizeof buf);
	    tmp = fscanf(fp, "%s", buf);
    	myData->CAN.config.canInsDevNum[i] = (unsigned char)atoi(buf);
    	if(myData->CAN.config.functionType[i] == CAN_FUNC_INV_COMM){
			myData->CAN.config.installedInverter
    			= myData->CAN.config.canInsDevNum[i];
		}
	}

	//Channel In Inverter 1
	tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	for(i=0; i < MAX_CAN_PORT; i++) {
		memset(buf, 0, sizeof buf);
	    tmp = fscanf(fp, "%s", buf);
    	myData->CAN.config.chInInv[i] = (unsigned char)atoi(buf);
	}

	//Channel In Inverter 2
	tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	for(i=8; i < MAX_CAN_PORT+8; i++) {
		memset(buf, 0, sizeof buf);
	    tmp = fscanf(fp, "%s", buf);
    	myData->CAN.config.chInInv[i] = (unsigned char)atoi(buf);
	}

	//Channel In Inverter 3
	tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	for(i=16; i < MAX_CAN_PORT+16; i++) {
		memset(buf, 0, sizeof buf);
	    tmp = fscanf(fp, "%s", buf);
    	myData->CAN.config.chInInv[i] = (unsigned char)atoi(buf);
	}

	//Channel In Inverter 4
	tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	for(i=24; i < MAX_CAN_PORT+24; i++) {
		memset(buf, 0, sizeof buf);
	    tmp = fscanf(fp, "%s", buf);
    	myData->CAN.config.chInInv[i] = (unsigned char)atoi(buf);
	}

	//CAN In BD 1
	tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	for(i=0; i < MAX_CAN_PORT; i++) {
		memset(buf, 0, sizeof buf);
	    tmp = fscanf(fp, "%s", buf);
    	myData->CAN.config.canInBd[i] = (unsigned char)atoi(buf);
	}

	//CAN In BD 2
	tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	for(i=8; i < MAX_CAN_PORT+8; i++) {
		memset(buf, 0, sizeof buf);
	    tmp = fscanf(fp, "%s", buf);
    	myData->CAN.config.canInBd[i] = (unsigned char)atoi(buf);
	}
    fclose(fp);
	return 0;
}

int	Read_TempFault_Config(void)
{ //210205
    int tmp;
	char temp[32], buf[12], fileName[128];
    FILE *fp;

	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/cycler_data/config/parameter/TempFault_Config");
	// /root/cycler_data/config/parameter/TempFault_Config
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "TempFault_Config file read error\n");
		system("cp ../Config_backup/TempFault_Config /root/cycler_data/config/parameter");
		userlog(DEBUG_LOG, psName, "TempFault_Config file copy\n");
		if((fp = fopen(fileName, "r")) == NULL) {
			return -1;
		}
	} else {
		system("rm -rf ../Config_backup/TempFault_Config");
		system("cp /root/cycler_data/config/parameter/TempFault_Config ../Config_backup");
	}

	tmp = fscanf(fp, "%s", temp); 	
	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
   	tmp = fscanf(fp, "%s", buf);
	myData->mData.config.TempFaultDioUse = atol(buf);

	tmp = fscanf(fp, "%s", temp); 	
	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
   	tmp = fscanf(fp, "%s", buf);
	myData->mData.config.TempFaultMinT = atol(buf);

	tmp = fscanf(fp, "%s", temp); 	
	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
   	tmp = fscanf(fp, "%s", buf);
	myData->mData.config.TempFaultMaxT = atol(buf);

	tmp = fscanf(fp, "%s", temp); 	
	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
   	tmp = fscanf(fp, "%s", buf);
	myData->mData.config.TempFaultCheckTime = atol(buf);
	
    fclose(fp);
	return 0;
}


int Read_SBC_IP_Address(void)
{
	int tmp, i;
	char temp[20], buf[6], fileName[128];
    FILE *fp;
	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/etc/sysconfig/network-scripts/ifcfg-eth0");
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "ifcfg-eth0 file read error\n");
		return -1;
	}
    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
    tmp = fscanf(fp, "%s", temp);
	for(i = 0; i < 8; i++) {
    	tmp = fscanf(fp, "%c", temp);
	}
	memset(buf, 0x00, sizeof temp);
    tmp = fscanf(fp, "%s", temp);
    memcpy((char *)&myPs->misc.sbcIpAddr[0], (char *)&temp[0],
		sizeof(char)*16);
    fclose(fp);
	return 0;
}

void Config_backup(void)
{
	char fileName[128];
	int fp;

	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/project/Cycler/current/cycler/App/Config_backup");
	if((fp = access(fileName, 0)) < 0) {
		printf("%c[1;5;31m",27);
		printf("Config_backup folder not found%c[0m\n", 27);
		system("mkdir /project/Cycler/current/cycler/App/Config_backup");
		printf("Make Config_backup folder\t\t\t\t\t");
		printf("[  %c[1;32mOK%c[0m  ]\n", 27, 27);
	}
//		system("rm -rf /project/Cycler/current/cycler/App/Config_backup/*");
	system("cp -rf /root/cycler_data/config/parameter/* /project/Cycler/current/cycler/App/Config_backup");
	printf("parameter backup\t\t\t\t\t\t");
	printf("[  %c[1;32mOK%c[0m  ]\n", 27, 27);
}

void Update_bashrc(void)
{
	char fileName[128];
	int fp;
	
	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/project/Cycler/current/cycler/App/Scripts/AutoUpdate/.bashrc");
	if((fp = access(fileName, 0)) < 0) {
		printf("%c[1;5;31m",27);
		printf("bashrc file not found%c[0m\n", 27);
		printf("bashrc file update\t\t\t\t\t\t"); 
		printf("[%c[1;31mFAILED%c[0m]\n", 27, 27);
	} else {
		system("cp -rf /project/Cycler/current/cycler/App/Scripts/AutoUpdate/.bashrc /root");
		printf("bashrc file update\t\t\t\t\t\t"); 
		printf("[  %c[1;32mOK%c[0m  ]\n", 27, 27);
	}
}

void Update_proftpd_config(void)
{
	char fileName[128];
	int fp;
	
	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/project/Cycler/current/cycler/App/Scripts/AutoUpdate/proftpd.conf");
	if((fp = access(fileName, 0)) < 0) {
		printf("%c[1;5;31m",27);
		printf("FTP Config file not found%c[0m\n", 27);
		printf("FTP Config file update\t\t\t\t\t\t");
		printf("[%c[1;31mFAILED%c[0m]\n", 27, 27);
	} else {
		system("cp -rf /project/Cycler/current/cycler/App/Scripts/AutoUpdate/proftpd.conf /etc/proftpd/");
		printf("FTP Config file update\t\t\t\t\t\t");
		printf("[  %c[1;32mOK%c[0m  ]\n", 27, 27);
	}
}

//190823 oys add : Find number of processes using mbuff.
int unload_process_using_mbuff(void)
{
	int rtn;
	char buf[32], cmd[128], fileName[128];
	FILE *fp;

	memset(cmd, 0x00, sizeof(cmd));
	memset(fileName, 0x00, sizeof(fileName));

	strcpy(fileName, "/root/cycler_data/config/tmp/unload_mbuff.txt");
	sprintf(cmd, "rm -rf %s", fileName);
	system(cmd);
	usleep(100000); //100mS
	sprintf(cmd, "lsmod | grep mbuff > %s", fileName);
	system(cmd);
	usleep(500000); //500mS

	if((fp = fopen(fileName, "r")) != NULL) {
		memset(buf, 0x00, sizeof(buf));
		rtn = fscanf(fp, "%s",buf);
		memset(buf, 0x00, sizeof(buf));
		rtn = fscanf(fp, "%s",buf);
		memset(buf, 0x00, sizeof(buf));
		rtn = fscanf(fp, "%s",buf);
		rtn = atoi(buf);
		if(rtn > 0) {
			printf("Number of process using mbuff : ");
			printf("[%c[1;31m%d%c[0m]\n",27, rtn, 27);
		} else {
			printf("Not found process using mbuff%c[0m\n", 27);
		}
	} else {
		printf("%c[1;5;31m",27);
		printf("unload_mbuff.txt file not found.%c[0m\n", 27);
		rtn = -1;	
	}
	return rtn;
}
