#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <string.h>
#include "../../INC/datastore.h"
#include "userlog.h"
#include "common_utils.h"
#include "local_utils.h"
#include "message.h"
#include "main.h"

volatile S_SYSTEM_DATA	*myData;
volatile S_DATA_SAVE	*myPs; //my process : DataSave
char psName[16];
	
int main(void)
{
    int rtn;
    struct timeval tv;
	//220408_hun
	char fileName[128];
	int fileExist = -1;
	
    fd_set rfds;
    
	if(Initialize() < 0) return 0;

    while(myData->AppControl.signal[APP_SIG_DATA_SAVE_PROCESS] == P1) {
    	tv.tv_sec = 0;
		tv.tv_usec = 500000;
		FD_ZERO(&rfds);

		rtn = select(0, &rfds, NULL, NULL, &tv);
		if(rtn == 0) {
			DataSave_Control();
		} else {
		}
		//220408_hun
		memset(fileName, 0x00, sizeof(fileName));
		strcpy(fileName, "/root/Auto_Update.tar.gz");
		fileExist = access(fileName, 0); //update file exist
	
		if(myData->mData.config.Auto_Update == 1){
			myData->mData.config.Auto_Update = 0;
			if(fileExist == 0){ 
				system("rm -rf /root/SBC_Update");
				system("rm -rf /root/project_backup_program");
				system("tar xfz /root/Auto_Update.tar.gz -C /root/");
				system("cd /root/project_backup_program ; tar xfz current_backup.tar.gz");
				system("cd /root/project_backup_program ; tar xfz config_backup.tar.gz");
				system("cd /root/project_backup_program/current_backup/cycler/App/AppControl ; cp SBC_Update /root/");
				system("cd /project/Cycler/current/cycler/App/PSKill/ ; ./PSKill");
				system("cd /root/ ; ./SBC_Update");
			}else if(fileExist == -1){ 
				userlog(DEBUG_LOG,psName,"Update.tar.gz Not exists\n");
			}
		}
    }
	Close_Process();
    return 0;
}

int Initialize(void)
{
	if(Open_SystemMemory(0) < 0) return -1;
	
	myPs = &(myData->DataSave);
	
	Init_SystemMemory();
	
	memset((char *)&psName[0], 0x00, 16);
	strcpy(psName, "DataSave");

	if(Read_DataSave_Config() < 0) return -2;

	myData->AppControl.signal[APP_SIG_DATA_SAVE_PROCESS] = P1;
	return 0;
}

void DataSave_Control(void)
{
	Check_Message();
	Check_Signal();
	Save_10mS_Data_1();			//date_save_type == 1
	Save_ResultData_1();
	Save_PulseData_1();
}

void Check_Signal(void)
{
	int i, tmp, idx, bd, ch;
	unsigned long chFlag;
	S_MSG_CH_FLAG ch_flag;

	if(myPs->signal[DATASAVE_SIG_SAVED_FILE_DELETE] == P1) {
		myPs->signal[DATASAVE_SIG_SAVED_FILE_DELETE] = P0;
		idx = (int)myPs->signal[DATASAVE_SIG_SAVED_FILE_DELETE_IDX];
		memcpy((char *)&ch_flag,
			(char *)&myData->msg[MAIN_TO_DATASAVE].msg_ch_flag[idx],
			sizeof(S_MSG_CH_FLAG));

		for(i=0; i < myData->mData.config.installedCh; i++) {
			tmp = i / 32;
			chFlag = 0x00000001;
			chFlag = chFlag << (i % 32);
			chFlag = chFlag & ch_flag.bit_32[tmp];
			if(chFlag != 0) {
				bd = myData->CellArray1[i].bd;
				ch = myData->CellArray1[i].ch;
				if(myData->bData[bd].cData[ch].op.state != C_STANDBY) {
					continue;
				}
				Open_ResultData_1(i);
			}
		}
		sleep(1);
		
		send_msg_ch_flag(DATASAVE_TO_MODULE, (char *)&ch_flag);
		send_msg(DATASAVE_TO_MODULE, MSG_DATASAVE_MODULE_CH_RUN, 0, 0);
	}

	if(myPs->signal[DATASAVE_SIG_RCVED_CONTINUE_CMD] == P1) {
		myPs->signal[DATASAVE_SIG_RCVED_CONTINUE_CMD] = P0;
		idx = (int)myPs->signal[DATASAVE_SIG_RCVED_CONTINUE_CMD_IDX];
		memcpy((char *)&ch_flag,
			(char *)&myData->msg[MAIN_TO_DATASAVE].msg_ch_flag[idx],
			sizeof(S_MSG_CH_FLAG));

		for(i=0; i < myData->mData.config.installedCh; i++) {
			tmp = i / 32;
			chFlag = 0x00000001;
			chFlag = chFlag << (i % 32);
			chFlag = chFlag & ch_flag.bit_32[tmp];
			if(chFlag != 0) {
				bd = myData->CellArray1[i].bd;
				ch = myData->CellArray1[i].ch;
				if(myData->bData[bd].cData[ch].op.state != C_PAUSE) {
					continue;
				}
				Open_ResultData_2(i);
			}
		}
		sleep(1);
		
		send_msg_ch_flag(DATASAVE_TO_MODULE, (char *)&ch_flag);
		send_msg(DATASAVE_TO_MODULE, MSG_DATASAVE_MODULE_CH_CONTINUE, 0, 0);
	}
}

void Close_Process(void)
{
	Save_ResultData_1();

    myData->AppControl.signal[APP_SIG_DATA_SAVE_PROCESS] = P3;
	
	Close_SystemMemory();
}
