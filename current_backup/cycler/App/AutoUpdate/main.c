//******************************************************************************
//	SUBJECT		: CYCLER FIRMWARE AUTOUPDATE PROGRAM
//	VERSION		: REV 0.1
//	DATE		: 2019/06/26
//	WRITER		: OK YOSEOP
//******************************************************************************
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>		//use for open,read close function
#include <fcntl.h>
#include "userlog.h"
#include "../../INC/datastore.h"
#include "common_utils.h"
#include "local_utils.h"
#include "main.h"

volatile S_SYSTEM_DATA *myData;
volatile S_AUTO_UPDATE  *myPs;
char	psName[PROCESS_NAME_SIZE];

int main(void)
{
	int rtn, tv_min;
		
	if(Initialize() < 0) return 0;
	//Program start delay
	sleep(10);


	tv_min = myPs->config.retryInterval;
	if(myPs->config.serverConnectFlag == P1) {
		//server connect minimum value is 5min
		if(tv_min < 200) {
			tv_min = 200;
		}
	}
	myPs->misc.timeCnt = 0;
	
	if(myPs->config.useFlag == 1)
		userlog(DEBUG_LOG, psName, ">> AutoUpdate program standby...\n");

	while(myData->AppControl.signal[APP_SIG_AUTO_UPDATE_PROCESS] == P1) {
		if(myPs->misc.timeCnt == 0) {
			//Check for update folder
			rtn = Update_Folder_Check();
			if(rtn == 1) {
				//Server file version check & file  download
				Get_File_Process();
				//Check for update file
				rtn = Update_File_Check();
				if(rtn == 1) {
					//Channel run state check
					rtn = Run_State_Check();
					if(rtn == 1) {
						userlog(DEBUG_LOG, psName, ">> Firmware update file found\n");
						sleep(1);
						userlog(DEBUG_LOG, psName, ">> Firmware update start...\n\n");
						//Update process start
						Update_Process();
						break;
					}
				}
			}
		}
		myPs->misc.timeCnt++;
		if(myPs->misc.timeCnt >= tv_min) {
			myPs->misc.timeCnt = 0;
		}
		sleep(1);
	}
	myData->AppControl.signal[APP_SIG_AUTO_UPDATE_PROCESS] = P3;
	Close_SystemMemory();
    return 0;
}

int Initialize(void)
{
	if(Open_SystemMemory(0) < 0) return -1;

	myPs = &(myData->AutoUpdate);

	Init_SystemMemory();

	memset((char *)&psName[0], 0x00, 16);
	strcpy(psName, "Update");

	if(Read_AutoUpdate_Config() < 0) return -2;
	
	myData->AppControl.signal[APP_SIG_AUTO_UPDATE_PROCESS] = P1;

	return 0;
}

int Run_State_Check(void)
{
	int rtn, bd, ch, i, chPerBd, installedCh, flag = 0;

	chPerBd = myData->mData.config.chPerBd;
	installedCh = myData->mData.config.installedCh;

	for(i = 0; i < installedCh; i++)
	{
		bd = i / chPerBd;
		ch = i % chPerBd;

		if(myData->bData[bd].cData[ch].op.state != C_STANDBY){
			flag++;
		}
	}
	if(flag != 0) {
		rtn = -1;
	} else {
		rtn = 1;
	}	
	return rtn;
}

int Update_Folder_Check(void)
{
	char fileName[128];
	int fp;
	
	if(myPs->config.useFlag == 0) return -1;

	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "../Scripts/AutoUpdate/update.sh");
	if((fp = access(fileName, 0)) < 0) {
		return -1;
	}

	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/update");
	if((fp = access(fileName, 0)) < 0) {
		userlog(DEBUG_LOG, psName, "Update folder not found\n");
		system("mkdir /root/update");
		system("echo '0' > /root/update/version.txt");
		system("mkdir /root/update/lastUpdate");
		system("echo '0' > /root/update/lastUpdate/version.txt");
		if(myPs->config.updateServerSet == P1) {
			system("mkdir /root/update/shared");
			system("echo '0' > /root/update/shared/version.txt");
		}
		userlog(DEBUG_LOG, psName, "Create update folder\n");
	} else {
		memset(fileName, 0x00, sizeof(fileName));
		strcpy(fileName, "/root/update/lastUpdate");
		if((fp = access(fileName, 0)) < 0) {
			userlog(DEBUG_LOG, psName, "lastUpdate folder not found\n");
			system("mkdir /root/update/lastUpdate");
			system("echo '0' > /root/update/lastUpdate/version.txt");
			userlog(DEBUG_LOG, psName, "Create lastUpdate folder\n");
		}
		
		if(myPs->config.updateServerSet == P1) {
			memset(fileName, 0x00, sizeof(fileName));
			strcpy(fileName, "/root/update/shared");
			if((fp = access(fileName, 0)) < 0) {
				userlog(DEBUG_LOG, psName, "shared folder not found\n");
				system("mkdir /root/update/shared");
				system("echo '0' > /root/update/shared/version.txt");
				userlog(DEBUG_LOG, psName, "Create shared folder\n");
			}
		} else {
			memset(fileName, 0x00, sizeof(fileName));
			strcpy(fileName, "/root/update/shared");
			if((fp = access(fileName, 0)) < 0) {
			} else {
				system("rm -rf /root/update/shared");
			}
		}
	}
	return 1;
}

void Get_File_Process(void)
{
	char fileName[128], temp1[128], temp2[128];
	int fp, tmp, rtn;
	FILE *fp2;

	if(myPs->config.serverConnectFlag == P0) return;
	if(myPs->config.updateServerSet == P1) return;

	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "../Scripts/AutoUpdate/nodisplay.sh");
	if((fp = access(fileName, 0)) < 0) {
	} else {
		memset(fileName, 0x00, sizeof(fileName));
		strcpy(fileName, "../Scripts/AutoUpdate/get_file.sh");
		if((fp = access(fileName, 0)) < 0) {
		} else {
			memset(fileName, 0x00, sizeof(fileName));
			sprintf(fileName, "../Scripts/AutoUpdate/nodisplay.sh %s 1", myPs->config.ipAddr);
			system(fileName);
		}
		sleep(10);
		memset(fileName, 0x00, sizeof(fileName));
		strcpy(fileName, "/root/update/version.txt");
    	if((fp2 = fopen(fileName, "r")) == NULL) {
			return;
		} else {
			memset(temp1, 0x00, sizeof(temp1));
			tmp = fscanf(fp2, "%s", temp1);
			fclose(fp2);
		}
			
		memset(fileName, 0x00, sizeof(fileName));
		strcpy(fileName, "/root/update/lastUpdate/version.txt");
    	if((fp2 = fopen(fileName, "r")) == NULL) {
			return;
		} else {
			memset(temp2, 0x00, sizeof(temp2));
			tmp = fscanf(fp2, "%s", temp2);
			fclose(fp2);
		}

		rtn = strcmp(temp1, temp2);
		if(rtn > 0){
			memset(fileName, 0x00, sizeof(fileName));
			sprintf(fileName, "../Scripts/AutoUpdate/nodisplay.sh %s 2", myPs->config.ipAddr);
			system(fileName);
			sleep(10);
		} else {
			system("rm -rf /root/update/version.txt");
		}
	}
}

int Update_File_Check(void)
{
	char fileName[128], temp1[20], temp2[20], temp3[20];
	int fp, tmp, rtn;
	FILE *fp2;

	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/update/sbc_project_backup.tar.gz");
	if((fp = access(fileName, 0)) < 0) {
		system("echo '0' > /root/update/version.txt");
	} else {
		system("date -r /root/update/sbc_project_backup.tar.gz '+%Y%m%d%H%M' > /root/update/version.txt");
		memset(fileName, 0x00, sizeof(fileName));
		strcpy(fileName, "/root/update/version.txt");
		fp2 = fopen(fileName, "r");
		memset(temp1, 0x00, sizeof(temp1));
		tmp = fscanf(fp2, "%s", temp1);
		fclose(fp2);

		memset(fileName, 0x00, sizeof(fileName));
		strcpy(fileName, "/root/update/lastUpdate/sbc_project_backup.tar.gz");
		if((fp = access(fileName, 0)) < 0) {
			system("echo '0' > /root/update/lastUpdate/version.txt");
		}

		memset(fileName, 0x00, sizeof(fileName));
		strcpy(fileName, "/root/update/lastUpdate/version.txt");
		fp2 = fopen(fileName, "r");
		memset(temp2, 0x00, sizeof(temp2));
		tmp = fscanf(fp2, "%s", temp2);
		fclose(fp2);
	
		rtn = strcmp(temp1, temp2);
		if(rtn > 0){
			return 1;
		}
	}
	if(myPs->config.updateServerSet == P1) {
		memset(fileName, 0x00, sizeof(fileName));
		strcpy(fileName, "/root/update/lastUpdate/version.txt");
		fp2 = fopen(fileName, "r");
		memset(temp2, 0x00, sizeof(temp2));
		tmp = fscanf(fp2, "%s", temp2);
		fclose(fp2);
		
		memset(fileName, 0x00, sizeof(fileName));
		strcpy(fileName, "/root/update/shared/sbc_project_backup.tar.gz");
		if((fp = access(fileName, 0)) < 0) {
			system("echo '0' > /root/update/shared/version.txt");
		}
		memset(fileName, 0x00, sizeof(fileName));
		strcpy(fileName, "/root/update/shared/version.txt");
		fp2 = fopen(fileName, "r");
		memset(temp3, 0x00, sizeof(temp3));
		tmp = fscanf(fp2, "%s", temp3);
		fclose(fp2);

		rtn = strcmp(temp2, temp3);
		if(rtn > 0){
			system("rm -rf /root/update/shared/*");
			system("cp /root/update/lastUpdate/sbc_project_backup.tar.gz /root/update/shared");
			system("cp /root/update/lastUpdate/version.txt /root/update/shared");
		}
	}
	return -1;
}

int Update_Process(void)
{
	char fileName[128];
	int fp;

	system("../PSKill/PSKill");
	sleep(15);
//.bashrc file Update
	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "../Scripts/AutoUpdate/.bashrc");
	if((fp = access(fileName, 0)) == 0) {
		system("cp -rf ../Scripts/AutoUpdate/.bashrc /root");
	}
	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "../Scripts/AutoUpdate/update.sh");
	if((fp = access(fileName, 0)) < 0) {
		userlog(DEBUG_LOG, psName, "Update script file read error\n");
	} else {
		system("/project/Cycler/current/cycler/App/Scripts/AutoUpdate/update.sh");
		sleep(10);
		userlog(DEBUG_LOG, psName, ">> Firmware update complete\n");
		system("/project/Cycler/current/cycler/App/Scripts/AutoUpdate/makenreboot.sh");
		sleep(2);
		userlog(DEBUG_LOG, psName, ">> System reboot...\n");
	}	
	return 0;
}
