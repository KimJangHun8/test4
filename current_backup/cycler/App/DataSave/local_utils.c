#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/stat.h>
#include "../../INC/datastore.h"
#include "userlog.h"
#include "local_utils.h"

extern volatile S_SYSTEM_DATA	*myData;
extern volatile S_DATA_SAVE		*myPs;
extern char psName[PROCESS_NAME_SIZE];

void Init_SystemMemory(void)
{
	int i;
	
	for(i=0; i < MAX_SIGNAL; i++) {
		myPs->signal[i] = P0;
	}

	myPs->misc.processPointer = (int)&myData;
}

int	Read_DataSave_Config(void)
{
    int tmp;
	char temp[20], buf[8], fileName[128];
    FILE *fp;

	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/cycler_data/config/parameter/DataSave_Config");
	// /root/cycler_data/config/parameter/DataSave_Config
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "DataSave_Config file read error\n");
		system("cp ../Config_backup/DataSave_Config /root/cycler_data/config/parameter");
		userlog(DEBUG_LOG, psName, "DataSave_Config file copy\n");
    	if((fp = fopen(fileName, "r")) == NULL) {
			return -1;
		}
	}

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.resultData_saveFlag = (unsigned char)atoi(buf);

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.monitoringData_saveFlag = (unsigned char)atoi(buf);

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.checkData_saveFlag = (unsigned char)atoi(buf);

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.zero_sec_data_save = (unsigned char)atoi(buf);
	
    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.save_data_type = (unsigned char)atoi(buf);
	if(myPs->config.save_data_type == 1){
		if(VENDER == 2){
			myPs->config.zero_sec_data_save = 0;
		}
	}
    
    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.save_10ms_time = atoi(buf);

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.save_50ms_time = atoi(buf);

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.save_100ms_time = atoi(buf);

	tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.AuxData_saveFlag = (unsigned char)atoi(buf);

	tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.saveCond_Check_time = atoi(buf);

	tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.pause_data_time = atoi(buf);

	fclose(fp);
	return 0;
}

//171212 lyh add
//191029 oys modify
int	Read_User_Pattern(const int realCh, const int stepNo, const int select)
{
    int tmp, i, length, bd, ch, type;
	char temp[20], buf[8], fileName[128], cmd[128];
	unsigned char userDataNo;
	long maxI, data, rangeI, maxP, cpRefV;
    double refI;
	FILE *fp;

	bd = myData->CellArray1[realCh].bd;
	ch = myData->CellArray1[realCh].ch;

	userDataNo = myData->bData[bd].cData[ch].misc.userDataNo;

	if(userDataNo == 0) return 0;
	
	// General
	if(select == 0) {
		memset(fileName, 0x00, sizeof(fileName));
		sprintf(fileName, "/root/cycler_data/userData/%02d/userPattern_step_%03d.csv", userDataNo, stepNo);
		if((fp = fopen(fileName, "r")) == NULL) {
			userlog(DEBUG_LOG, psName, "ch %d : step : %d userPattern file read fail\n", realCh+1, stepNo);
			return -1;
		}

	    tmp = fscanf(fp, "%s", temp); 	//Date
		tmp = fscanf(fp, "%s", temp);   // :
		tmp = fscanf(fp, "%s", temp);   // yy/mm/dd
		tmp = fscanf(fp, "%s", temp); 	// Time
		tmp = fscanf(fp, "%s", temp);   // :
		tmp = fscanf(fp, "%s", temp);   // hh:mm:ss 

		tmp = fscanf(fp, "%s", temp);   // stepNo
		tmp = fscanf(fp, "%s", temp);   // :

		memset(buf, 0x00, sizeof buf);
	    tmp = fscanf(fp, "%s", buf);
		if(atoi(buf) != stepNo){
			userlog(DEBUG_LOG, psName, "ch : %d step : %d userPattern stepNo missmatch\n", realCh+1, stepNo);
			return -1;
		}
		myData->mData.testCond[bd][ch].userPattern.stepNo = atol(buf);

		tmp = fscanf(fp, "%s", temp);   // length
		tmp = fscanf(fp, "%s", temp);   // :
		memset(buf, 0x00, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		length = atoi(buf);
		if(length > MAX_USER_PATTERN_DATA ){
			userlog(DEBUG_LOG, psName, "ch : %d step : %d userPattern length[%d] error\n", realCh+1, stepNo, length);
			return -1;
		}
		myData->mData.testCond[bd][ch].userPattern.length = (long)length;

		tmp = fscanf(fp, "%s", temp);   // type
		tmp = fscanf(fp, "%s", temp);   // :
		memset(buf, 0x00, sizeof buf);
	    tmp = fscanf(fp, "%s", buf);
		type = atol(buf);
	    myData->mData.testCond[bd][ch].userPattern.type = (long)type;
		switch((unsigned char)type){
			case PS_CURRENT:
			case PS_C_RATE:
				myData->mData.testCond[bd][ch].step[stepNo].mode = CC;
				break;
			case PS_WATT:
    			myData->mData.testCond[bd][ch].step[stepNo].mode = CP;
				break;
		}

		tmp = fscanf(fp, "%s", temp);   // stepTime
		tmp = fscanf(fp, "%s", temp);   // :
		tmp = fscanf(fp, "%s", temp);   // data
	}
	// FTP
	if(select == 1) {
		memset(fileName, 0x00, sizeof(fileName));
		memset(cmd, 0x00, sizeof(cmd));

		sprintf(fileName, "/root/cycler_data/pattern/ch%02d/GUI_userPattern_step_%03d.csv", myData->mData.misc.usingUserDataFlag[userDataNo-1], stepNo);
		if(access(fileName, 0) >= 0) {
			sprintf(cmd, "mv /root/cycler_data/pattern/ch%02d/* /root/cycler_data/userData/%02d/", myData->mData.misc.usingUserDataFlag[userDataNo-1], userDataNo);
			system(cmd);
		}
		sprintf(fileName, "/root/cycler_data/userData/%02d/GUI_userPattern_step_%03d.csv", userDataNo, stepNo);
		if((fp = fopen(fileName, "r")) == NULL) {
			userlog(DEBUG_LOG, psName, "ch %d : step : %d GUI_userPattern file read fail\n", realCh+1, stepNo);
			return -1;
	 	}

		tmp = fscanf(fp, "%s", temp);   // stepNo
		tmp = fscanf(fp, "%s", temp);   // :
		memset(buf, 0x00, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		if(atoi(buf) != stepNo){
			userlog(DEBUG_LOG, psName, "ch : %d step : %d GUI_userPattern stepNo missmatch\n", realCh+1, stepNo);
			return -1;
		}
		myData->mData.testCond[bd][ch].userPattern.stepNo = atol(buf);
		
		tmp = fscanf(fp, "%s", temp);   // length
		tmp = fscanf(fp, "%s", temp);   // :
		memset(buf, 0x00, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		length = atoi(buf);
		if(length > MAX_USER_PATTERN_DATA ){
			userlog(DEBUG_LOG, psName, "ch : %d step : %d GUI_userPattern length[%d] error\n", realCh+1, stepNo, length);
			return -1;
		}
		myData->mData.testCond[bd][ch].userPattern.length = (long)length;

		tmp = fscanf(fp, "%s", temp);   // type
		tmp = fscanf(fp, "%s", temp);   // :
		memset(buf, 0x00, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		type = atol(buf);
		myData->mData.testCond[bd][ch].userPattern.type = (long)type;
		switch((unsigned char)type){
			case PS_CURRENT:
			case PS_C_RATE:
				myData->mData.testCond[bd][ch].step[stepNo].mode = CC;
				break;
			case PS_WATT:
    			myData->mData.testCond[bd][ch].step[stepNo].mode = CP;
				break;
		}
/*
		tmp = fscanf(fp, "%s", temp);   // stepTime
		tmp = fscanf(fp, "%s", temp);   // :
		tmp = fscanf(fp, "%s", temp);   // data
*/
	}
	maxI = maxP = 0;

	for(i = 0; i < length; i++){
		memset(buf, 0x00, sizeof buf);
	    tmp = fscanf(fp, "%s", buf);
	    myData->mData.testCond[bd][ch].userPattern.data[i].time = atol(buf);
		
		tmp = fscanf(fp, "%s", temp);   // :

		memset(buf, 0x00, sizeof buf);
	    tmp = fscanf(fp, "%s", buf);
	    data = atol(buf);
	    myData->mData.testCond[bd][ch].userPattern.data[i].data = data;
		if(i == 0){
			if(data != 0){
				if(data > 0) myData->bData[bd].cData[ch].misc.cmdV_dir = CMD_V_PLUS;
				if(data < 0) myData->bData[bd].cData[ch].misc.cmdV_dir = CMD_V_MINUS;
			}
		}else if(i == 1){
	    	if(myData->mData.testCond[bd][ch].userPattern.data[0].data == 0){
				if(data != 0){
					if(data > 0) myData->bData[bd].cData[ch].misc.cmdV_dir = CMD_V_PLUS;
					if(data < 0) myData->bData[bd].cData[ch].misc.cmdV_dir = CMD_V_MINUS;
				}else{
					myData->bData[bd].cData[ch].misc.cmdV_dir = CMD_V_PLUS;
				}
			}
		}
		switch((unsigned char)type){
			case PS_CURRENT:
				if(labs(maxI) < labs(data)) maxI = data;
				break;
			case PS_WATT:
				if(labs(maxP) < labs(data)) maxP = data;
				break;
			case PS_C_RATE:
				if(myData->bData[bd].cData[ch].misc.standardC_Flag == P1) {
					data = myData->bData[bd].cData[ch]
						.misc.standardC * ((float)data / 1000); //non fix
				}
				if(labs(maxI) < labs(data)) maxI = data;
				break;
		}
	}
	rangeI = 0;
	switch((unsigned char)type){
		case PS_CURRENT:
		case PS_C_RATE:
			for(i = MAX_RANGE; i > 0; i--){
				if(labs(maxI) <= myData->mData.config.maxCurrent[i-1]){
		    		rangeI = i-1;
					break;
				}
			}
			if((rangeI + 1) > (int)myData->mData.config.rangeI) {
				rangeI = (int)myData->mData.config.rangeI -1;
			}
    		myData->mData.testCond[bd][ch].step[stepNo].rangeI = (unsigned char)rangeI;
    		myData->mData.testCond[bd][ch].step[stepNo].rangeV = (unsigned char)(RANGE1-1);
			break;
		case PS_WATT:
			//110427 kji pattern cp range select 
    		if(myData->mData.testCond[bd][ch].step[stepNo].endV_L > 0
    			|| myData->mData.testCond[bd][ch].step[stepNo].refV_L > 0){
    			if(myData->mData.testCond[bd][ch].step[stepNo].endV_L >
    				myData->mData.testCond[bd][ch].step[stepNo].refV_L) {
    				cpRefV = myData->mData.testCond[bd][ch].step[stepNo].endV_L;
				} else {
    				cpRefV = myData->mData.testCond[bd][ch].step[stepNo].refV_L;
					if(cpRefV < 500000)
						cpRefV = 500000;
				}
				refI = (labs(maxP) / (double)cpRefV);
				refI *= 1000000000.0;
				if((long)refI <= myData->mData.config.maxCurrent[3]) {
				    rangeI = RANGE4 - 1;
				} else if((long)refI <= myData->mData.config.maxCurrent[2]) {
				    rangeI = RANGE3 - 1;
				} else if((long)refI <= myData->mData.config.maxCurrent[1]) {
				    rangeI = RANGE2 - 1;
				} else {
				    rangeI = RANGE1 - 1;
				}
			}
    		myData->mData.testCond[bd][ch].step[stepNo].rangeI = (unsigned char)rangeI;
    		myData->mData.testCond[bd][ch].step[stepNo].rangeV = (unsigned char)(RANGE1-1);
			break;
	}
    fclose(fp);

	return 0;
}

int	Read_User_Pattern_1(const int realCh, const int stepNo, const int select, const int num)
{
#if USER_PATTERN_500 == 1
    int tmp, i, length, bd, ch, type, totalNumber, Number;
	char temp[20], buf[8], fileName[128], cmd[128];
	unsigned char userDataNo;
	long maxI, data, rangeI, maxP, cpRefV;
    double refI;
	FILE *fp;

	bd = myData->CellArray1[realCh].bd;
	ch = myData->CellArray1[realCh].ch;

	userDataNo = myData->bData[bd].cData[ch].misc.userDataNo;

	if(userDataNo == 0) return 0;
		
	// FTP
	if(select == 1) {
		memset(fileName, 0x00, sizeof(fileName));
		memset(cmd, 0x00, sizeof(cmd));
		sprintf(fileName, "/root/cycler_data/userData/%02d/GUI_userPattern_step_%03d_%03d.csv", userDataNo, stepNo, num);
		if((fp = fopen(fileName, "r")) == NULL) {
			sprintf(cmd, "mv /root/cycler_data/pattern/ch%02d/* /root/cycler_data/userData/%02d/", myData->mData.misc.usingUserDataFlag[userDataNo-1], userDataNo);
			system(cmd);
			if((fp = fopen(fileName, "r")) == NULL) {
				if(USER_PATTERN_500 == P1){
					userlog(DEBUG_LOG, psName, "ch %d : step : %d num : %d GUI_userPattern_A file read fail\n", realCh+1, stepNo , num);
				}else{
					userlog(DEBUG_LOG, psName, "ch %d : step : %d GUI_userPattern file read fail\n", realCh+1, stepNo);
				}
				return -1;
			}
	 	}

		tmp = fscanf(fp, "%s", temp);   // stepNo
		tmp = fscanf(fp, "%s", temp);   // :
		memset(buf, 0x00, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		if(atoi(buf) != stepNo){
			userlog(DEBUG_LOG, psName, "ch : %d step : %d GUI_userPattern stepNo missmatch\n", realCh+1, stepNo);
			return -1;
		}
		myData->mData.testCond[bd][ch].userPattern.stepNo = atol(buf);
		
		tmp = fscanf(fp, "%s", temp);   // length
		tmp = fscanf(fp, "%s", temp);   // :
		memset(buf, 0x00, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		length = atoi(buf);
		if(length > MAX_USER_PATTERN_DATA ){
			userlog(DEBUG_LOG, psName, "ch : %d step : %d GUI_userPattern length[%d] error\n", realCh+1, stepNo, length);
			return -1;
		}
		myData->mData.testCond[bd][ch].userPattern.length = (long)length;

		tmp = fscanf(fp, "%s", temp);   // type
		tmp = fscanf(fp, "%s", temp);   // :
		memset(buf, 0x00, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		type = atol(buf);
		myData->mData.testCond[bd][ch].userPattern.type = (long)type;

		tmp = fscanf(fp, "%s", temp);   // totalNumber
		tmp = fscanf(fp, "%s", temp);   // :
		memset(buf, 0x00, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		totalNumber = atol(buf);
		myData->mData.testCond[bd][ch].userPattern.totalNumber = (long)totalNumber;

		tmp = fscanf(fp, "%s", temp);   // Number
		tmp = fscanf(fp, "%s", temp);   // :
		memset(buf, 0x00, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		Number = atol(buf);
		myData->mData.testCond[bd][ch].userPattern.Number = (long)Number;

		switch((unsigned char)type){
			case PS_CURRENT:
			case PS_C_RATE:
				myData->mData.testCond[bd][ch].step[stepNo].mode = CC;
				break;
			case PS_WATT:
    			myData->mData.testCond[bd][ch].step[stepNo].mode = CP;
				break;
		}
	}
	maxI = maxP = 0;

	for(i = 0; i < length; i++){
		memset(buf, 0x00, sizeof buf);
	    tmp = fscanf(fp, "%s", buf);
	    myData->mData.testCond[bd][ch].userPattern.data[i].time = atol(buf);
		
		tmp = fscanf(fp, "%s", temp);   // :

		memset(buf, 0x00, sizeof buf);
	    tmp = fscanf(fp, "%s", buf);
	    data = atol(buf);
	    myData->mData.testCond[bd][ch].userPattern.data[i].data = data;
		if(i == 0){
			if(data != 0){
				if(data > 0) myData->bData[bd].cData[ch].misc.cmdV_dir = CMD_V_PLUS;
				if(data < 0) myData->bData[bd].cData[ch].misc.cmdV_dir = CMD_V_MINUS;
			}
		}else if(i == 1){
	    	if(myData->mData.testCond[bd][ch].userPattern.data[0].data == 0){
				if(data != 0){
					if(data > 0) myData->bData[bd].cData[ch].misc.cmdV_dir = CMD_V_PLUS;
					if(data < 0) myData->bData[bd].cData[ch].misc.cmdV_dir = CMD_V_MINUS;
				}else{
					myData->bData[bd].cData[ch].misc.cmdV_dir = CMD_V_PLUS;
				}
			}
		}
		switch((unsigned char)type){
			case PS_CURRENT:
				if(labs(maxI) < labs(data)) maxI = data;
				break;
			case PS_WATT:
				if(labs(maxP) < labs(data)) maxP = data;
				break;
			case PS_C_RATE:
				if(myData->bData[bd].cData[ch].misc.standardC_Flag == P1) {
					data = myData->bData[bd].cData[ch]
						.misc.standardC * ((float)data / 1000); //non fix
				}
				if(labs(maxI) < labs(data)) maxI = data;
				break;
		}
	}
	rangeI = 0;
	switch((unsigned char)type){
		case PS_CURRENT:
		case PS_C_RATE:
			for(i = MAX_RANGE; i > 0; i--){
				if(labs(maxI) <= myData->mData.config.maxCurrent[i-1]){
		    		rangeI = i-1;
					break;
				}
			}
			if((rangeI + 1) > (int)myData->mData.config.rangeI) {
				rangeI = (int)myData->mData.config.rangeI -1;
			}
    		myData->mData.testCond[bd][ch].step[stepNo].rangeI = (unsigned char)rangeI;
    		myData->mData.testCond[bd][ch].step[stepNo].rangeV = (unsigned char)(RANGE1-1);
			break;
		case PS_WATT:
			//110427 kji pattern cp range select 
    		if(myData->mData.testCond[bd][ch].step[stepNo].endV_L > 0
    			|| myData->mData.testCond[bd][ch].step[stepNo].refV_L > 0){
    			if(myData->mData.testCond[bd][ch].step[stepNo].endV_L >
    				myData->mData.testCond[bd][ch].step[stepNo].refV_L) {
    				cpRefV = myData->mData.testCond[bd][ch].step[stepNo].endV_L;
				} else {
    				cpRefV = myData->mData.testCond[bd][ch].step[stepNo].refV_L;
					if(cpRefV < 500000)
						cpRefV = 500000;
				}
				refI = (labs(maxP) / (double)cpRefV);
				refI *= 1000000000.0;
				if((long)refI <= myData->mData.config.maxCurrent[3]) {
				    rangeI = RANGE4 - 1;
				} else if((long)refI <= myData->mData.config.maxCurrent[2]) {
				    rangeI = RANGE3 - 1;
				} else if((long)refI <= myData->mData.config.maxCurrent[1]) {
				    rangeI = RANGE2 - 1;
				} else {
				    rangeI = RANGE1 - 1;
				}
			}
    		myData->mData.testCond[bd][ch].step[stepNo].rangeI = (unsigned char)rangeI;
    		myData->mData.testCond[bd][ch].step[stepNo].rangeV = (unsigned char)(RANGE1-1);
			break;
	}
    fclose(fp);
#endif
	return 0;
}

int	Read_User_Pattern_2(const int realCh, const int stepNo, const int select, const int num)
{
#if USER_PATTERN_500 == 1
    int tmp, i, length, bd, ch, type, totalNumber, Number;
	char temp[20], buf[8], fileName[128], cmd[128];
	unsigned char userDataNo;
	long maxI, data, rangeI, maxP, cpRefV;
    double refI;
	FILE *fp;
	
	bd = myData->CellArray1[realCh].bd;
	ch = myData->CellArray1[realCh].ch;

	userDataNo = myData->bData[bd].cData[ch].misc.userDataNo;

	if(userDataNo == 0) return 0;
		
	// FTP
	if(select == 1) {
		memset(fileName, 0x00, sizeof(fileName));
		memset(cmd, 0x00, sizeof(cmd));
		sprintf(fileName, "/root/cycler_data/userData/%02d/GUI_userPattern_step_%03d_%03d.csv", userDataNo, stepNo, num);
		if((fp = fopen(fileName, "r")) == NULL) {
			sprintf(cmd, "mv /root/cycler_data/pattern/ch%02d/* /root/cycler_data/userData/%02d/", myData->mData.misc.usingUserDataFlag[userDataNo-1], userDataNo);
			system(cmd);
			if((fp = fopen(fileName, "r")) == NULL) {
				if(USER_PATTERN_500 == P1){
					userlog(DEBUG_LOG, psName, "ch %d : step : %d num : %d GUI_userPattern_B file read fail\n", realCh+1, stepNo, num);
				}else{
					userlog(DEBUG_LOG, psName, "ch %d : step : %d GUI_userPattern file read fail\n", realCh+1, stepNo);
				}
				return -1;
			}
	 	}

		tmp = fscanf(fp, "%s", temp);   // stepNo
		tmp = fscanf(fp, "%s", temp);   // :
		memset(buf, 0x00, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		if(atoi(buf) != stepNo){
			userlog(DEBUG_LOG, psName, "ch : %d step : %d GUI_userPattern stepNo missmatch\n", realCh+1, stepNo);
			return -1;
		}
		myData->mData.testCond[bd][ch].userPatternBuf.stepNo = atol(buf);
		
		tmp = fscanf(fp, "%s", temp);   // length
		tmp = fscanf(fp, "%s", temp);   // :
		memset(buf, 0x00, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		length = atoi(buf);
		if(length > MAX_USER_PATTERN_DATA ){
			userlog(DEBUG_LOG, psName, "ch : %d step : %d GUI_userPattern length[%d] error\n", realCh+1, stepNo, length);
			return -1;
		}
		myData->mData.testCond[bd][ch].userPatternBuf.length = (long)length;

		tmp = fscanf(fp, "%s", temp);   // type
		tmp = fscanf(fp, "%s", temp);   // :
		memset(buf, 0x00, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		type = atol(buf);
		myData->mData.testCond[bd][ch].userPatternBuf.type = (long)type;

		tmp = fscanf(fp, "%s", temp);   // totalNumber
		tmp = fscanf(fp, "%s", temp);   // :
		memset(buf, 0x00, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		totalNumber = atol(buf);
		myData->mData.testCond[bd][ch].userPatternBuf.totalNumber = (long)totalNumber;

		tmp = fscanf(fp, "%s", temp);   // Number
		tmp = fscanf(fp, "%s", temp);   // :
		memset(buf, 0x00, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		Number = atol(buf);
		myData->mData.testCond[bd][ch].userPatternBuf.Number = (long)Number;
		
		switch((unsigned char)type){
			case PS_CURRENT:
			case PS_C_RATE:
				myData->mData.testCond[bd][ch].step[stepNo].mode = CC;
				break;
			case PS_WATT:
    			myData->mData.testCond[bd][ch].step[stepNo].mode = CP;
				break;
		}
	}
	maxI = maxP = 0;

	for(i = 0; i < length; i++){
		memset(buf, 0x00, sizeof buf);
	    tmp = fscanf(fp, "%s", buf);
	    myData->mData.testCond[bd][ch].userPatternBuf.data[i].time = atol(buf);
		
		tmp = fscanf(fp, "%s", temp);   // :

		memset(buf, 0x00, sizeof buf);
	    tmp = fscanf(fp, "%s", buf);
	    data = atol(buf);
	    myData->mData.testCond[bd][ch].userPatternBuf.data[i].data = data;
		if(i == 0){
			if(data != 0){
				if(data > 0) myData->bData[bd].cData[ch].misc.cmdV_dir = CMD_V_PLUS;
				if(data < 0) myData->bData[bd].cData[ch].misc.cmdV_dir = CMD_V_MINUS;
			}
		}else if(i == 1){
	    	if(myData->mData.testCond[bd][ch].userPatternBuf.data[0].data == 0){
				if(data != 0){
					if(data > 0) myData->bData[bd].cData[ch].misc.cmdV_dir = CMD_V_PLUS;
					if(data < 0) myData->bData[bd].cData[ch].misc.cmdV_dir = CMD_V_MINUS;
				}else{
					myData->bData[bd].cData[ch].misc.cmdV_dir = CMD_V_PLUS;
				}
			}
		}
		switch((unsigned char)type){
			case PS_CURRENT:
				if(labs(maxI) < labs(data)) maxI = data;
				break;
			case PS_WATT:
				if(labs(maxP) < labs(data)) maxP = data;
				break;
			case PS_C_RATE:
				if(myData->bData[bd].cData[ch].misc.standardC_Flag == P1) {
					data = myData->bData[bd].cData[ch]
						.misc.standardC * ((float)data / 1000); //non fix
				}
				if(labs(maxI) < labs(data)) maxI = data;
				break;
		}
	}
	rangeI = 0;
	switch((unsigned char)type){
		case PS_CURRENT:
		case PS_C_RATE:
			for(i = MAX_RANGE; i > 0; i--){
				if(labs(maxI) <= myData->mData.config.maxCurrent[i-1]){
		    		rangeI = i-1;
					break;
				}
			}
			if((rangeI + 1) > (int)myData->mData.config.rangeI) {
				rangeI = (int)myData->mData.config.rangeI -1;
			}
    		myData->mData.testCond[bd][ch].step[stepNo].rangeI = (unsigned char)rangeI;
    		myData->mData.testCond[bd][ch].step[stepNo].rangeV = (unsigned char)(RANGE1-1);
			break;
		case PS_WATT:
			//110427 kji pattern cp range select 
    		if(myData->mData.testCond[bd][ch].step[stepNo].endV_L > 0
    			|| myData->mData.testCond[bd][ch].step[stepNo].refV_L > 0){
    			if(myData->mData.testCond[bd][ch].step[stepNo].endV_L >
    				myData->mData.testCond[bd][ch].step[stepNo].refV_L) {
    				cpRefV = myData->mData.testCond[bd][ch].step[stepNo].endV_L;
				} else {
    				cpRefV = myData->mData.testCond[bd][ch].step[stepNo].refV_L;
					if(cpRefV < 500000)
						cpRefV = 500000;
				}
				refI = (labs(maxP) / (double)cpRefV);
				refI *= 1000000000.0;
				if((long)refI <= myData->mData.config.maxCurrent[3]) {
				    rangeI = RANGE4 - 1;
				} else if((long)refI <= myData->mData.config.maxCurrent[2]) {
				    rangeI = RANGE3 - 1;
				} else if((long)refI <= myData->mData.config.maxCurrent[1]) {
				    rangeI = RANGE2 - 1;
				} else {
				    rangeI = RANGE1 - 1;
				}
			}
    		myData->mData.testCond[bd][ch].step[stepNo].rangeI = (unsigned char)rangeI;
    		myData->mData.testCond[bd][ch].step[stepNo].rangeV = (unsigned char)(RANGE1-1);
			break;
	}
    fclose(fp);

#endif
	return 0;
}

//111215 kji add
int	Read_User_Map(const int realCh, const int stepNo)
{
    int tmp, i, j, bd, ch, type;
	char temp[20], buf[8], fileName[128];
	unsigned char userDataNo;
	long maxI=0, maxP=0, rangeI, cpRefV;
	double refI;
	FILE *fp;

	bd = myData->CellArray1[realCh].bd;
	ch = myData->CellArray1[realCh].ch;

	userDataNo = myData->bData[bd].cData[ch].misc.userDataNo;
	
	if(userDataNo == 0) return 0;

	memset(fileName, 0x00, sizeof(fileName));
	sprintf(fileName, "/root/cycler_data/userData/%02d/userMap_step_%03d.csv", userDataNo, stepNo);
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "ch %d : step : %d userMap file read fail\n", realCh+1, stepNo);
		return -1;
	}

    tmp = fscanf(fp, "%s", temp); 	//Date
	tmp = fscanf(fp, "%s", temp);   // :
	tmp = fscanf(fp, "%s", temp);   // yy/mm/dd
    tmp = fscanf(fp, "%s", temp); 	// Time
	tmp = fscanf(fp, "%s", temp);   // :
	tmp = fscanf(fp, "%s", temp);   // hh:mm:ss 

	tmp = fscanf(fp, "%s", temp);   // stepNo
	tmp = fscanf(fp, "%s", temp);   // :
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
	if(atoi(buf) != stepNo){
		userlog(DEBUG_LOG, psName, "ch : %d step : %d userMap stepNo missmatch\n", realCh+1, stepNo);
		return -1;
	}
    myData->mData.testCond[bd][ch].userMap.stepNo = atol(buf);

	tmp = fscanf(fp, "%s", temp);   // type
	tmp = fscanf(fp, "%s", temp);   // :
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myData->mData.testCond[bd][ch].userMap.type = atol(buf);
	
	tmp = fscanf(fp, "%s", temp);   // type
	tmp = fscanf(fp, "%s", temp);   // :
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
	type = atol(buf);
    myData->mData.testCond[bd][ch].userMap.mode = (long)type;
	switch((unsigned char)type){
		case PS_CURRENT:
    		myData->mData.testCond[bd][ch].step[stepNo].mode = CC;
			break;
		case PS_WATT:
    		myData->mData.testCond[bd][ch].step[stepNo].mode = CP;
			break;
	}
	
	tmp = fscanf(fp, "%s", temp);   // renewalTime
	tmp = fscanf(fp, "%s", temp);   // :
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myData->mData.testCond[bd][ch].userMap.renewalTime = atol(buf);

	tmp = fscanf(fp, "%s", temp);   // maxCapa
	tmp = fscanf(fp, "%s", temp);   // :
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myData->mData.testCond[bd][ch].userMap.maxCapacity = atol(buf);
	
	tmp = fscanf(fp, "%s", temp);   // ocvTableRow
	tmp = fscanf(fp, "%s", temp);   // :
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myData->mData.testCond[bd][ch].userMap.ocvTableRow = atol(buf);
	
	tmp = fscanf(fp, "%s", temp);   // ocvTableCol
	tmp = fscanf(fp, "%s", temp);   // :
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myData->mData.testCond[bd][ch].userMap.ocvTableCol = atol(buf);
	
	tmp = fscanf(fp, "%s", temp);   // dataTableRow
	tmp = fscanf(fp, "%s", temp);   // :
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myData->mData.testCond[bd][ch].userMap.dataTableRow = atol(buf);
	
	tmp = fscanf(fp, "%s", temp);   // dataTableCol
	tmp = fscanf(fp, "%s", temp);   // :
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myData->mData.testCond[bd][ch].userMap.dataTableCol = atol(buf);

	maxI = maxP = 0;

	tmp = fscanf(fp, "%s", temp);   // ocvTable
	
	for(i = 0; i < myData->mData.testCond[bd][ch].userMap.ocvTableRow; i++){
		memset(buf, 0x00, sizeof buf);
	    tmp = fscanf(fp, "%s", buf);
	    myData->mData.testCond[bd][ch].userMap.ocvTable[i][0] = atol(buf);
		
		memset(buf, 0x00, sizeof buf);
	    tmp = fscanf(fp, "%s", buf);
	    myData->mData.testCond[bd][ch].userMap.ocvTable[i][1] = atol(buf);
	
	}

	tmp = fscanf(fp, "%s", temp);   // dataTable

	for(i = 0; i <= myData->mData.testCond[bd][ch].userMap.dataTableRow; i++){
		for(j = 0; j <= myData->mData.testCond[bd][ch].userMap.dataTableCol;j++){
			memset(buf, 0x00, sizeof buf);
	    	tmp = fscanf(fp, "%s", buf);
	    	myData->mData.testCond[bd][ch].userMap.dataTable[i][j] = atol(buf);

////// 120120 oys w : range select process. ////////////////////////////////////
			if(i >=1 && j>= 1) {
				switch((unsigned char)type){
					case PS_CURRENT:
						if(labs(maxI) < labs(atol(buf))) maxI = atol(buf);
						break;
					case PS_WATT:
						if(labs(maxP) < labs(atol(buf))) maxP = atol(buf);
						break;
				}
			}
		}
	}
	rangeI = 0;
	switch((unsigned char)type){
		case PS_CURRENT:
			for(i = MAX_RANGE; i > 0; i--){
				if(labs(maxI) <= myData->mData.config.maxCurrent[i-1]){
		    		rangeI = i-1;
					break;
				}
			}
			if((rangeI + 1) > (int)myData->mData.config.rangeI) {
				rangeI = (int)myData->mData.config.rangeI -1;
			}
    		myData->mData.testCond[bd][ch].step[stepNo].rangeI = (unsigned char)rangeI;
    		myData->mData.testCond[bd][ch].step[stepNo].rangeV = (unsigned char)(RANGE1-1);
			break;
		case PS_WATT:
			//110427 kji pattern cp range select 
    		if(myData->mData.testCond[bd][ch].step[stepNo].endV_L > 0
    			|| myData->mData.testCond[bd][ch].step[stepNo].refV_L > 0){
    			if(myData->mData.testCond[bd][ch].step[stepNo].endV_L >
    				myData->mData.testCond[bd][ch].step[stepNo].refV_L) {
    				cpRefV = myData->mData.testCond[bd][ch].step[stepNo].endV_L;
				} else {
    				cpRefV = myData->mData.testCond[bd][ch].step[stepNo].refV_L;
					if(cpRefV < 500000)
						cpRefV = 500000;
				}
				refI = (labs(maxP) / (double)cpRefV);
				refI *= 1000000000.0;
				if((long)refI <= myData->mData.config.maxCurrent[3]) {
				    rangeI = RANGE4 - 1;
				} else if((long)refI <= myData->mData.config.maxCurrent[2]) {
				    rangeI = RANGE3 - 1;
				} else if((long)refI <= myData->mData.config.maxCurrent[1]) {
				    rangeI = RANGE2 - 1;
				} else {
				    rangeI = RANGE1 - 1;
				}
			}
    		myData->mData.testCond[bd][ch].step[stepNo].rangeI = (unsigned char)rangeI;
    		myData->mData.testCond[bd][ch].step[stepNo].rangeV = (unsigned char)(RANGE1-1);
			break;
	}
    fclose(fp);	
/*
	//save the test condition		pms add for restore
	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "/root/cycler_data/resultData/ch%02d/testCond", realCh+1);
	fp1 = open(cmd, (O_CREAT | O_RDWR), (S_IREAD | S_IWRITE));
	if( fp1 == -1){
		printf("ch%d TestCond file Open error\n",realCh+1);
	}else{
		writeSize = write(fp1, 
		(char*)&(myData->mData.testCond[bd][ch]), sizeof(S_TEST_CONDITION));
		if(writeSize != sizeof(S_TEST_CONDITION)){
			printf("ch%d Test Condition size error\n", realCh+1);
		}
	}
	close(fp1);
	//end of add
*/
	return 0;
}
#ifdef _TRACKING_MODE
long Calculate_Crate_Value(unsigned char crate_mode, int realCh)
{
	float init_val, cur_val, factor, crate , link_rptSOC, cur_rptSOC;
	int bd, ch;
	
	crate = 1000000.0;
	if(crate_mode != 1) return crate;
	
	bd = myData->CellArray1[realCh].bd;
	ch = myData->CellArray1[realCh].ch;
	
	//	init_val = (float)myData->mData.testCond[bd][ch].safety.initial_AH / 1000;
	//	cur_val = (float)myData->mData.testCond[bd][ch].safety.current_AH / 1000;
	init_val = (float)1000000.0 / 1000; //211124 rewrite
	cur_val = (float)1000000.0 / 1000; //211124 rewrite
	factor = (float)myData->mData.testCond[bd][ch].safety.crate_factor;
	link_rptSOC = myData->bData[bd].cData[ch].op.link_rptSOC / 1000;
	cur_rptSOC = myData->bData[bd].cData[ch].op.current_rptSOC / 1000;

	if(link_rptSOC != 0.0)	{
		//init_val = link_rptSOC;
		cur_val = link_rptSOC;
	}
	if(cur_rptSOC != 0.0) {
		cur_val = cur_rptSOC;
	}
	if(crate_mode == 1) {
		crate = cur_val / init_val;
		crate *= factor;
	}
	if(crate < 0.0) {
		crate = 1.0;
	} else if(crate >= factor) {
		crate = factor;
	}

	return crate;
}

//int Read_SOC_Tracking_File(char *psName1, int ch, int pattern_index, unsigned char type)
int Read_SOC_Tracking_File(const int select, const int realCh, const int stepNo, const int type) 
{ 
	char cmd[256], *in_delimiter = ",\t\n\r", *token;
	char temp[512], buf[512];
	unsigned char crate_mode; //jhkw_191108
	int tmp, i, j, k, bd, ch;
	unsigned long crate_val, crate; //jhkw_191108
	float diff_current, diff, A;
	FILE *fp;

//	i = IDX_LOC_OBJ_PATTERN_UPDATED; //kjg_170810
//	div = myData->testCond[ch].local_object[pattern_index][i];
	bd = myData->CellArray1[realCh].bd;
	ch = myData->CellArray1[realCh].ch;
	
	memset(cmd, 0, sizeof cmd);
	if(select == 0) { //first
		sprintf(cmd, "/root/cycler_data/trackingData/ch%02d/SOC_tracking_data_step_%03d.csv", realCh+1, stepNo+1);
	} else { 
		sprintf(cmd, "/root/cycler_data/trackingData/ch%02d/SOC_tracking_data_step_%03d.csv", realCh+1, stepNo+1);
	}
	if((fp = fopen(cmd, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "%s file(load){Open Error}\n", cmd);
		return -1;
	}

	j = k = 0;

FIRST:
	crate_mode = crate_val = crate = 0; //jhkw_191108
	i = 0;
	while(i == 0) {
		j++;
		if(j >= 100) {
			i = -1;
			break;
		}
		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);

		if(temp[0] == 'S' && temp[1] == 'T' && temp[2] == 'X'){
			i = 1;
			//jhkw_191108s
			if(k >= 1) {
				if(temp[14] == 'C' && temp[15] == 'R' && temp[16] == 'A'
					&& temp[17] == 'T' && temp[18] == 'E'){
					crate_mode = 1;
				}
			}
			//jhkw_191108e
			break;
		}
	}

	if(i <= 0) {
		userlog(DEBUG_LOG, psName, "%s file(load){STX Error}\n", cmd);
    	fclose(fp);
		return -2;
	}

	memset((char *)&myData->mData.testCond[bd][ch].SOC_tracking[type], 0,
		sizeof(S_TEST_COND_SOC_TRACKING_DATA));

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	token = strtok(temp, in_delimiter);
	if(token == NULL) {
		userlog(DEBUG_LOG, psName, "%s file(load){Index Error}\n", cmd);
    	fclose(fp);
		return -3;
	}
	memset(buf, 0, sizeof buf);
	strcpy(buf, token);
	token = strtok(NULL, in_delimiter);	
	if(myData->mData.testCond[bd][ch].step[stepNo].trackingMode_flag == 1){
		if((strncmp(buf, "VOL", 3) == 0)) { // 210720
			//Index //VOL/Temp
		} else {
			k++;
			if(k == 1) {
				goto FIRST;
			} else if(k == 2) {
				userlog(DEBUG_LOG, psName, "%s file(load){Index Error Check 2}\n", cmd);
	   	 		fclose(fp);
				return -4;
			}
		}
	}else{
		if((strncmp(buf, "SOC", 3) == 0)) {
			//Index //SOC/Temp
		} else {
			k++;
			if(k == 1) {
				goto FIRST;
			} else if(k == 2) {
				userlog(DEBUG_LOG, psName, "%s file(load){Index Error Check 2}\n", cmd);
	    		fclose(fp);
				return -4;
			}
		}
	}

	userlog(DEBUG_LOG, psName, "c-rate flag = %d\n", crate_mode); //jhkw_191108
	crate = Calculate_Crate_Value(crate_mode, ch);
	userlog(DEBUG_LOG, psName, "c-rate = %ld\n", (long)crate); //jhkw_191108
	myData->mData.testCond[bd][ch].SOC_tracking[type].maxI = 0;
	myData->mData.testCond[bd][ch].SOC_tracking[type].minI
		= myData->mData.config.maxCurrent[0] * 4;
	for(i=0; i < (MAX_SOC_TRACKING_DATA - 5) + 1; i++) { //SOC //25 -> rewrite 210720
		if(i > 0){
			memset(temp, 0, sizeof temp);
			tmp = fscanf(fp, "%s", temp);
			token = strtok(temp, in_delimiter);
		}
//		for(j=0; j < MAX_SOC_TRACKING_DATA + 1; j++) { // Temp //20
		for(j=0; j < (MAX_SOC_TRACKING_DATA-10) + 1; j++) { // Temp //20 -> rewrite 210720
			if(i == 0 && j == 0){
			 	continue;
			}
			memset(buf, 0, sizeof buf);
			if(token == NULL){
			}else{
				strcpy(buf, token);
			}
			if((strncmp(buf, "EOF", 3) == 0) || (strncmp(buf, "eof", 3) == 0)) {
			}else{
				if(i == 0){ 
					if(j > 0){
						if(token != NULL){ //KHKW
							myData->mData.testCond[bd][ch].SOC_tracking[type].temp[j-1] 
								= (long)(atof(buf)*100.0);
							myData->mData.testCond[bd][ch].SOC_tracking[type].temp_num++;
						}
					}
				}
				if(j == 0){
					if(i > 0){
						if(token != NULL){ //KHKW
							if(myData->mData.testCond[bd][ch].step[stepNo].trackingMode_flag == 1){
							myData->mData.testCond[bd][ch].SOC_tracking[type].SOC[i-1] 
								= (long)(atof(buf)*1000000.0); //SOV -> rewirte 210720
							}else{
								myData->mData.testCond[bd][ch].SOC_tracking[type].SOC[i-1] 
								= (long)(atof(buf)*100.0); //SOC -> rewrite 210720
									
							}
							myData->mData.testCond[bd][ch].SOC_tracking[type].soc_num++;
						}
					}
				}
				if(i > 0 && j > 0){
					crate_val = (long)(atof(buf)*crate);
					myData->mData.testCond[bd][ch].SOC_tracking[type].limit_current[i-1][j-1] 
						= crate_val;
					if(myData->mData.testCond[bd][ch].SOC_tracking[type].maxI
						< myData->mData.testCond[bd][ch].SOC_tracking[type].limit_current[i-1][j-1]){
						myData->mData.testCond[bd][ch].SOC_tracking[type].maxI
							= myData->mData.testCond[bd][ch].SOC_tracking[type].limit_current[i-1][j-1];
					}
					if(myData->mData.testCond[bd][ch].SOC_tracking[type].minI
						> myData->mData.testCond[bd][ch].SOC_tracking[type].limit_current[i-1][j-1]){
						if(myData->mData.testCond[bd][ch].SOC_tracking[type].limit_current[i-1][j-1] != 0){
							myData->mData.testCond[bd][ch].SOC_tracking[type].minI
								= myData->mData.testCond[bd][ch].SOC_tracking[type].limit_current[i-1][j-1];
						}
					}

				}
				token = strtok(NULL, in_delimiter);
			}
		}
	}
	for(i=0; i < (MAX_SOC_TRACKING_DATA - 5); i++) { //SOC 
		for(j=0; j < (MAX_SOC_TRACKING_DATA - 10) - 1; j++) { // Temp -> rewrite 210720
			if((i <  myData->mData.testCond[bd][ch].SOC_tracking[type].soc_num) //SOC
					&&(j < myData->mData.testCond[bd][ch].SOC_tracking[type].temp_num-1)){ //Temp
				diff = myData->mData.testCond[bd][ch].SOC_tracking[type].temp[j+1]
					   - myData->mData.testCond[bd][ch].SOC_tracking[type].temp[j];
				if(diff != 0){
					diff_current = myData->mData.testCond[bd][ch].SOC_tracking[type].limit_current[i][j+1]
						- myData->mData.testCond[bd][ch].SOC_tracking[type].limit_current[i][j];
					if(diff_current != 0){
						A = (float)(diff_current/ diff);
						myData->mData.testCond[bd][ch].SOC_tracking[type].tracking_data_A[i][j]
							= A;
						myData->mData.testCond[bd][ch].SOC_tracking[type].tracking_data_B[i][j]
							= myData->mData.testCond[bd][ch].SOC_tracking[type].limit_current[i][j+1]
							- A * (float)myData->mData.testCond[bd][ch].SOC_tracking[type].temp[j+1];
					}else{
						myData->mData.testCond[bd][ch].SOC_tracking[type].tracking_data_A[i][j] = 0.0;
						myData->mData.testCond[bd][ch].SOC_tracking[type].tracking_data_B[i][j]
							= myData->mData.testCond[bd][ch].SOC_tracking[type].limit_current[i][j+1];
					}
				}else{
					myData->mData.testCond[bd][ch].SOC_tracking[type].tracking_data_A[i][j] = 0.0;
					myData->mData.testCond[bd][ch].SOC_tracking[type].tracking_data_B[i][j]
						= myData->mData.testCond[bd][ch].SOC_tracking[type].limit_current[i][j+1];
				}
			}
		}
	}
    fclose(fp);
	if(select == 0) { //first
		userlog(DEBUG_LOG, psName,
			"SOC tracking file read completed ch:%d step:%d\n",
			ch+1, stepNo+1);
	} else { //1 update
		userlog(DEBUG_LOG, psName, "SOC tracking file read completed(update) ch:%d step:%d\n",
			ch+1, stepNo+1);
	}
	return 0;
}
#endif

int Open_ResultData_1(int p_ch)
{
	char cmd[128];
	int bd, ch;
#if CH_AUX_DATA == 0
	int i;
#endif
	int aux_cnt, auxNo, installedTemp, installedAuxV;
	struct tm *tm;
	time_t t;
	FILE *fp;
	
	aux_cnt = 0;
	auxNo = 0;
	installedTemp = myData->mData.config.installedTemp;
	installedAuxV = myData->mData.config.installedAuxV;
	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "rm -rf /root/cycler_data/resultData/ch%02d/*", p_ch+1);
	system(cmd);

	myPs->resultData[p_ch].fileIndex = 1;
	myPs->resultData[p_ch].resultIndex = 1;
	time(&t);
	tm = localtime(&t);
	myPs->resultData[p_ch].open_year = (unsigned char)tm->tm_year-100;
	myPs->resultData[p_ch].open_month = (unsigned char)tm->tm_mon+1;
	myPs->resultData[p_ch].open_day = (unsigned char)tm->tm_mday;

	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "/root/cycler_data/resultData/ch%02d/ch%02d_SaveEndData.csv",
		p_ch+1, p_ch+1);
	fp = fopen(cmd, "w");
	if(fp == NULL || fp < 0) {
		userlog(DEBUG_LOG, psName, "Open_ResultData_1 error -1\n");
		return -1;
	}
	fflush(fp);
	fclose(fp);

	//130226 oys add aux EndData
	//20171205 sch modify
#if CH_AUX_DATA == 0
	if(myPs->config.AuxData_saveFlag == 0){
		if(installedTemp > 0)
			{
			memset(cmd, 0, sizeof(cmd));
			sprintf(cmd, "/root/cycler_data/resultData/ch%02d/ch%02d_SaveEndData_auxT.csv", p_ch+1, p_ch+1);
			fp = fopen(cmd, "w");
			if(fp == NULL || fp < 0) {
				userlog(DEBUG_LOG, psName, "Open_ResultData_1 error -10\n");
			return -10;
			}
			fflush(fp);
			fclose(fp);
		}
		if(installedAuxV > 0)
		{
			memset(cmd, 0, sizeof(cmd));
			sprintf(cmd, "/root/cycler_data/resultData/ch%02d/ch%02d_SaveEndData_auxV.csv", p_ch+1, p_ch+1);
			fp = fopen(cmd, "w");
			if(fp == NULL || fp < 0) {
				userlog(DEBUG_LOG, psName, "Open_ResultData_1 error -20\n");
			return -20;
			}
			fflush(fp);
			fclose(fp);
		}
	}else if(myPs->config.AuxData_saveFlag == 1){
		for(i = 0; i < installedTemp; i++){
			if(p_ch+1 == myData->auxSetData[i].chNo){
					aux_cnt++;
			}
		}
		if((installedTemp > 0) && (aux_cnt > 0))
		{
			memset(cmd, 0, sizeof(cmd));
			sprintf(cmd, "/root/cycler_data/resultData/ch%02d/ch%02d_SaveEndData_auxT.csv", p_ch+1, p_ch+1);
			fp = fopen(cmd, "w");
			if(fp == NULL || fp < 0) {
				userlog(DEBUG_LOG, psName, "Open_ResultData_1 error -10\n");
			return -10;
			}
			fflush(fp);
			fclose(fp);
			aux_cnt = 0;
		}
		for (i = installedTemp; i < installedAuxV + installedTemp; i++)
		{
			if(p_ch+1 == myData->auxSetData[i].chNo){
					aux_cnt++;
			}
		}
		if((installedAuxV > 0) && (aux_cnt > 0))
		{
			memset(cmd, 0, sizeof(cmd));
			sprintf(cmd, "/root/cycler_data/resultData/ch%02d/ch%02d_SaveEndData_auxV.csv", p_ch+1, p_ch+1);
			fp = fopen(cmd, "w");
			if(fp == NULL || fp < 0) {
				userlog(DEBUG_LOG, psName, "Open_ResultData_1 error -20\n");
			return -20;
			}
			fflush(fp);
			fclose(fp);
			aux_cnt = 0;
		}
	}
#endif

	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd,
		"/root/cycler_data/resultData/ch%02d/savingFileIndex_start.csv", p_ch+1);
	fp = fopen(cmd, "w+");
	if(fp == NULL || fp < 0) {
		userlog(DEBUG_LOG, psName, "Open_ResultData_1 error -2\n");
		return -2;
	}

	bd = myData->CellArray1[p_ch].bd;
	ch = myData->CellArray1[p_ch].ch;

	fprintf(fp, "fileIndex %d, ", (int)myPs->resultData[p_ch].fileIndex);
	fprintf(fp, "resultIndex %lu, ", myPs->resultData[p_ch].resultIndex);
	fprintf(fp, "open_year %d, ", (int)myPs->resultData[p_ch].open_year);
	fprintf(fp, "open_month %d, ", (int)myPs->resultData[p_ch].open_month);
	fprintf(fp, "open_day %d\n", (int)myPs->resultData[p_ch].open_day);
	fflush(fp);
	fclose(fp);

	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd,
		"/root/cycler_data/resultData/ch%02d/savingFileIndex_last.csv", p_ch+1);
	fp = fopen(cmd, "w");
	if(fp == NULL || fp < 0) {
		userlog(DEBUG_LOG, psName, "Open_ResultData_1 error -3\n");
		return -3;
	}

	fprintf(fp, "fileIndex %d, ", (int)myPs->resultData[p_ch].fileIndex);
	fprintf(fp, "resultIndex %lu, ", myPs->resultData[p_ch].resultIndex);
	fprintf(fp, "open_year %d, ", (int)myPs->resultData[p_ch].open_year);
	fprintf(fp, "open_month %d, ", (int)myPs->resultData[p_ch].open_month);
	fprintf(fp, "open_day %d\n", (int)myPs->resultData[p_ch].open_day);
	fflush(fp);
	fclose(fp);

	memset(cmd, 0, sizeof(cmd));
#if DATA_SAVE_VER == 1
	sprintf(cmd, "/root/cycler_data/resultData/ch%02d/ch%02d_SaveData%04d.csv",
		p_ch+1, p_ch+1, (int)myPs->resultData[p_ch].fileIndex);
#else
	sprintf(cmd, "/root/cycler_data/resultData/ch%02d/ch%02d_SaveData%02d.csv",
		p_ch+1, p_ch+1, (int)myPs->resultData[p_ch].fileIndex);
#endif
	fp = fopen(cmd, "w+");
	if(fp == NULL || fp < 0) {
		userlog(DEBUG_LOG, psName, "Open_ResultData_1 error -4\n");
		return -4;
	}
	fflush(fp);
	fclose(fp);
	

	//130226 oys add aux SaveData
	//20171205 sch modify
#if CH_AUX_DATA == 0
	if(myPs->config.AuxData_saveFlag == 0){
		if(installedTemp > 0)
		{
			memset(cmd, 0, sizeof(cmd));
#if DATA_SAVE_VER == 1
			sprintf(cmd, "/root/cycler_data/resultData/ch%02d/ch%02d_SaveData%04d_auxT.csv", p_ch+1, p_ch+1, (int)myPs->resultData[p_ch].fileIndex);
#else
			sprintf(cmd, "/root/cycler_data/resultData/ch%02d/ch%02d_SaveData%02d_auxT.csv", p_ch+1, p_ch+1, (int)myPs->resultData[p_ch].fileIndex);
#endif
			fp = fopen(cmd, "w+");
			if(fp == NULL || fp < 0) {
				userlog(DEBUG_LOG, psName, "Open_ResultData_1 error -30\n");
			return -30;
			}
			fflush(fp);
			fclose(fp);
		}
		if(installedAuxV > 0)
		{
			memset(cmd, 0, sizeof(cmd));
#if DATA_SAVE_VER == 1
			sprintf(cmd, "/root/cycler_data/resultData/ch%02d/ch%02d_SaveData%04d_auxV.csv", p_ch+1, p_ch+1, (int)myPs->resultData[p_ch].fileIndex);
#else
			sprintf(cmd, "/root/cycler_data/resultData/ch%02d/ch%02d_SaveData%02d_auxV.csv", p_ch+1, p_ch+1, (int)myPs->resultData[p_ch].fileIndex);
#endif
			fp = fopen(cmd, "w+");
			if(fp == NULL || fp < 0) {
				userlog(DEBUG_LOG, psName, "Open_ResultData_1 error -40\n");
			return -40;
			}
			fflush(fp);
			fclose(fp);
		}
	}else if(myPs->config.AuxData_saveFlag == 1){
		for(i = 0; i < installedTemp; i++){
			if(p_ch+1 == myData->auxSetData[i].chNo){
					aux_cnt++;
			}
		}
		if((installedTemp > 0) && (aux_cnt > 0))
		{
			memset(cmd, 0, sizeof(cmd));
#if DATA_SAVE_VER == 1
			sprintf(cmd, "/root/cycler_data/resultData/ch%02d/ch%02d_SaveData%04d_auxT.csv", p_ch+1, p_ch+1, (int)myPs->resultData[p_ch].fileIndex);
#else
			sprintf(cmd, "/root/cycler_data/resultData/ch%02d/ch%02d_SaveData%02d_auxT.csv", p_ch+1, p_ch+1, (int)myPs->resultData[p_ch].fileIndex);
#endif
			fp = fopen(cmd, "w+");
			if(fp == NULL || fp < 0) {
				userlog(DEBUG_LOG, psName, "Open_ResultData_1 error -30\n");
			return -30;
			}
			fflush(fp);
			fclose(fp);
			aux_cnt = 0;
		}
		for (i = installedTemp; i < installedAuxV + installedTemp; i++)
		{
			if(p_ch+1 == myData->auxSetData[i].chNo){
					aux_cnt++;
			}
		}
		if((installedAuxV > 0) && (aux_cnt > 0))
		{
			memset(cmd, 0, sizeof(cmd));
#if DATA_SAVE_VER == 1
			sprintf(cmd, "/root/cycler_data/resultData/ch%02d/ch%02d_SaveData%04d_auxV.csv", p_ch+1, p_ch+1, (int)myPs->resultData[p_ch].fileIndex);
#else
			sprintf(cmd, "/root/cycler_data/resultData/ch%02d/ch%02d_SaveData%02d_auxV.csv", p_ch+1, p_ch+1, (int)myPs->resultData[p_ch].fileIndex);
#endif
			fp = fopen(cmd, "w+");
			if(fp == NULL || fp < 0) {
				userlog(DEBUG_LOG, psName, "Open_ResultData_1 error -40\n");
			return -40;
			}
			fflush(fp);
			fclose(fp);
			aux_cnt = 0;
		}
	}
#endif

	return 0;
}

int Open_ResultData_2(int p_ch)
{
	char cmd[128];
	int bd, ch;
#if CH_AUX_DATA == 0
	int i;
#endif
	int aux_cnt, auxNo, installedTemp, installedAuxV;
	struct tm *tm;
	time_t t;
	FILE *fp;

	aux_cnt = 0;
	auxNo = 0;
	installedTemp = myData->mData.config.installedTemp;
	installedAuxV = myData->mData.config.installedAuxV;

	time(&t);
	tm = localtime(&t);
	if(myPs->resultData[p_ch].open_year != (unsigned char)tm->tm_year-100
		|| myPs->resultData[p_ch].open_month != (unsigned char)tm->tm_mon+1
		|| myPs->resultData[p_ch].open_day != (unsigned char)tm->tm_mday) {
		myPs->resultData[p_ch].open_year = (unsigned char)tm->tm_year-100;
		myPs->resultData[p_ch].open_month = (unsigned char)tm->tm_mon+1;
		myPs->resultData[p_ch].open_day = (unsigned char)tm->tm_mday;
		
		myPs->resultData[p_ch].fileIndex++;
#if DATA_SAVE_VER == 0
		if(myPs->resultData[p_ch].fileIndex > MAX_RESULT_FILE_INDEX) {
			myPs->resultData[p_ch].fileIndex = 1;
		}
#endif
		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd,
			"/root/cycler_data/resultData/ch%02d/ch%02d_SaveEndData.csv",
			p_ch+1, p_ch+1);
		fp = fopen(cmd, "a+");
		if(fp == NULL || fp < 0) {
			userlog(DEBUG_LOG, psName, "Open_ResultData_2 error -1\n");
			return -1;
		}
		fflush(fp);
		fclose(fp);

		//130226 oys add aux EndData
		//20171205 sch modify
#if CH_AUX_DATA == 0
		if(myPs->config.AuxData_saveFlag == 0){
			if(installedTemp > 0)
			{
				memset(cmd, 0, sizeof(cmd));
				sprintf(cmd,
				"/root/cycler_data/resultData/ch%02d/ch%02d_SaveEndData_auxT.csv",p_ch+1, p_ch+1);
				fp = fopen(cmd, "a+");
				if(fp == NULL || fp < 0) {
					userlog(DEBUG_LOG, psName, "Open_ResultData_2 error -10\n");
				return -10;
				}
				fflush(fp);
				fclose(fp);
			}
			if(installedAuxV > 0)
			{
				memset(cmd, 0, sizeof(cmd));
				sprintf(cmd,
				"/root/cycler_data/resultData/ch%02d/ch%02d_SaveEndData_auxV.csv",p_ch+1, p_ch+1);
				fp = fopen(cmd, "a+");
				if(fp == NULL || fp < 0) {
					userlog(DEBUG_LOG, psName, "Open_ResultData_2 error -20\n");
				return -20;
				}
				fflush(fp);
				fclose(fp);
			}
		}else if(myPs->config.AuxData_saveFlag == 1){
			for(i = 0; i < installedTemp; i++){
				if(p_ch+1 == myData->auxSetData[i].chNo){
						aux_cnt++;
				}
			}
			if((installedTemp > 0) && (aux_cnt > 0))
			{
				memset(cmd, 0, sizeof(cmd));
				sprintf(cmd,
				"/root/cycler_data/resultData/ch%02d/ch%02d_SaveEndData_auxT.csv",p_ch+1, p_ch+1);
				fp = fopen(cmd, "a+");
				if(fp == NULL || fp < 0) {
					userlog(DEBUG_LOG, psName, "Open_ResultData_2 error -10\n");
				return -10;
				}
				fflush(fp);
				fclose(fp);
				aux_cnt = 0;
			}
			for (i = installedTemp; i < installedAuxV + installedTemp; i++)
			{
				if(p_ch+1 == myData->auxSetData[i].chNo){
						aux_cnt++;
				}
			}
			if((installedAuxV > 0) && (aux_cnt > 0))
			{
				memset(cmd, 0, sizeof(cmd));
				sprintf(cmd,
				"/root/cycler_data/resultData/ch%02d/ch%02d_SaveEndData_auxV.csv",p_ch+1, p_ch+1);
				fp = fopen(cmd, "a+");
				if(fp == NULL || fp < 0) {
					userlog(DEBUG_LOG, psName, "Open_ResultData_2 error -20\n");
				return -20;
				}
				fflush(fp);
				fclose(fp);
				aux_cnt = 0;
			}
		}
#endif

		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd,
			"/root/cycler_data/resultData/ch%02d/savingFileIndex_start.csv",
			p_ch+1);
		fp = fopen(cmd, "a+");
		if(fp == NULL || fp < 0) {
			userlog(DEBUG_LOG, psName, "Open_ResultData_2 error -2\n");
			return -2;
		}

		bd = myData->CellArray1[p_ch].bd;
		ch = myData->CellArray1[p_ch].ch;

		fprintf(fp, "fileIndex %d, ", (int)myPs->resultData[p_ch].fileIndex);
		fprintf(fp, "resultIndex %lu, ", myPs->resultData[p_ch].resultIndex+1);
		fprintf(fp, "open_year %d, ", (int)myPs->resultData[p_ch].open_year);
		fprintf(fp, "open_month %d, ", (int)myPs->resultData[p_ch].open_month);
		fprintf(fp, "open_day %d\n", (int)myPs->resultData[p_ch].open_day);
		fflush(fp);
		fclose(fp);

		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd,
			"/root/cycler_data/resultData/ch%02d/savingFileIndex_last.csv",
			p_ch+1);
		fp = fopen(cmd, "w+");
		if(fp == NULL || fp < 0) {
			userlog(DEBUG_LOG, psName, "Open_ResultData_2 error -3\n");
			return -3;
		}

		fprintf(fp, "fileIndex %d, ", (int)myPs->resultData[p_ch].fileIndex);
		fprintf(fp, "resultIndex %lu, ", myPs->resultData[p_ch].resultIndex);
		fprintf(fp, "open_year %d, ", (int)myPs->resultData[p_ch].open_year);
		fprintf(fp, "open_month %d, ", (int)myPs->resultData[p_ch].open_month);
		fprintf(fp, "open_day %d\n", (int)myPs->resultData[p_ch].open_day);
		fflush(fp);
		fclose(fp);

		memset(cmd, 0, sizeof(cmd));
#if DATA_SAVE_VER == 1
		sprintf(cmd,
			"rm -rf /root/cycler_data/resultData/ch%02d/ch%02d_SaveData%04d.csv", p_ch+1, p_ch+1, (int)myPs->resultData[p_ch].fileIndex);
		system(cmd);
		sprintf(cmd,
			"rm -rf /root/cycler_data/resultData/ch%02d/ch%02d_SaveData%04d.csv", p_ch+1, p_ch+1, (int)myPs->resultData[p_ch].fileIndex-7);
#else
		sprintf(cmd,
			"rm -rf /root/cycler_data/resultData/ch%02d/ch%02d_SaveData%02d.csv",
			p_ch+1, p_ch+1, (int)myPs->resultData[p_ch].fileIndex);
#endif
		system(cmd);

		// 130226 oys w : AuxDataSave
		if(myData->mData.config.installedAuxV > 0)
		{
			memset(cmd, 0, sizeof(cmd));
#if DATA_SAVE_VER == 1
			sprintf(cmd,
			"rm -rf /root/cycler_data/resultData/ch%02d/ch%02d_SaveData%04d_auxV.csv", p_ch+1, p_ch+1, (int)myPs->resultData[p_ch].fileIndex);
			system(cmd);
			sprintf(cmd,
			"rm -rf /root/cycler_data/resultData/ch%02d/ch%02d_SaveData%04d_auxV.csv", p_ch+1, p_ch+1, (int)myPs->resultData[p_ch].fileIndex-7);
#else
			sprintf(cmd,
			"rm -rf /root/cycler_data/resultData/ch%02d/ch%02d_SaveData%02d_auxV.csv", p_ch+1, p_ch+1, (int)myPs->resultData[p_ch].fileIndex);
#endif
			system(cmd);
		}

		if(myData->mData.config.installedTemp > 0)
		{
			memset(cmd, 0, sizeof(cmd));
#if DATA_SAVE_VER == 1
			sprintf(cmd,
			"rm -rf /root/cycler_data/resultData/ch%02d/ch%02d_SaveData%04d_auxT.csv", p_ch+1, p_ch+1, (int)myPs->resultData[p_ch].fileIndex);
			system(cmd);
			sprintf(cmd,
			"rm -rf /root/cycler_data/resultData/ch%02d/ch%02d_SaveData%04d_auxT.csv", p_ch+1, p_ch+1, (int)myPs->resultData[p_ch].fileIndex-7);
#else
			sprintf(cmd,
			"rm -rf /root/cycler_data/resultData/ch%02d/ch%02d_SaveData%02d_auxT.csv", p_ch+1, p_ch+1, (int)myPs->resultData[p_ch].fileIndex);
#endif
			system(cmd);
		}
	}
	return 0;
}

void Save_ResultData_1(void)
{
	int p_ch, bd, ch, rtn;

	for(p_ch=0; p_ch < myData->mData.config.installedCh; p_ch++) {
		bd = myData->CellArray1[p_ch].bd;
		ch = myData->CellArray1[p_ch].ch;
		rtn = Save_ResultData_2(p_ch);
		while(rtn) {
			rtn = Save_ResultData_2(p_ch);
		}
	}
}

int Save_ResultData_2(int p_ch)
{
	char cmd[128];
	int bd, ch, msg, idx, count, i;
	int aux_cnt, auxNo, installedTemp, installedAuxV;
	FILE *fp;
	
	bd = myData->CellArray1[p_ch].bd;
	ch = myData->CellArray1[p_ch].ch;
	msg = 1;
	count = 0;
	aux_cnt = 0;
	auxNo = 0;
	installedTemp = myData->mData.config.installedTemp;
	installedAuxV = myData->mData.config.installedAuxV;

	if(myData->save_msg[msg].write_idx[bd][ch]
		== myData->save_msg[msg].read_idx[bd][ch]) {
		return 0;
	}

	myData->save_msg[msg].read_idx[bd][ch]++;
	if(myData->save_msg[msg].read_idx[bd][ch] >= MAX_SAVE_MSG) {
		myData->save_msg[msg].read_idx[bd][ch] = 0;
	}
	idx = myData->save_msg[msg].read_idx[bd][ch];

	myData->save_msg[msg].count[bd][ch]--;
	count = myData->save_msg[msg].count[bd][ch];

	if(Open_ResultData_2(p_ch) < 0) {
		return -1;
	}

	memset(cmd, 0, sizeof(cmd));
#if DATA_SAVE_VER == 1
	sprintf(cmd, "/root/cycler_data/resultData/ch%02d/ch%02d_SaveData%04d.csv",
		p_ch+1, p_ch+1, (int)myPs->resultData[p_ch].fileIndex);
#else
	sprintf(cmd, "/root/cycler_data/resultData/ch%02d/ch%02d_SaveData%02d.csv",
		p_ch+1, p_ch+1, (int)myPs->resultData[p_ch].fileIndex);
#endif
	fp = fopen(cmd, "a+");
	if(fp == NULL || fp < 0) {
		userlog(DEBUG_LOG, psName, "Save_ResultData_2 error -1\n");
		return -2;
	}
#ifdef __LG_VER1__
	#if NETWORK_VERSION >= 4101
	fprintf(fp,
		"%ld,%d,%d,%d,%d,%d,%d,%d,%ld,%ld,%ld,%ld,%ld,%lu,%lu,%ld,%ld,%d,%lu,%lu,%ld,%ld",
		myData->save_msg[msg].val[idx][bd][ch].chData.resultIndex,
		(int)myData->save_msg[msg].val[idx][bd][ch].chData.state,
		(int)myData->save_msg[msg].val[idx][bd][ch].chData.type,
		(int)myData->save_msg[msg].val[idx][bd][ch].chData.mode,
		(int)myData->save_msg[msg].val[idx][bd][ch].chData.select,
		(int)myData->save_msg[msg].val[idx][bd][ch].chData.code,
		(int)myData->save_msg[msg].val[idx][bd][ch].chData.grade,
		(int)myData->save_msg[msg].val[idx][bd][ch].chData.stepNo,
		myData->save_msg[msg].val[idx][bd][ch].chData.Vsens,
		myData->save_msg[msg].val[idx][bd][ch].chData.Isens,
		myData->save_msg[msg].val[idx][bd][ch].chData.capacity,
		myData->save_msg[msg].val[idx][bd][ch].chData.watt,
		myData->save_msg[msg].val[idx][bd][ch].chData.wattHour,
		myData->save_msg[msg].val[idx][bd][ch].chData.runTime,
		myData->save_msg[msg].val[idx][bd][ch].chData.totalRunTime,
		myData->save_msg[msg].val[idx][bd][ch].chData.z,
		myData->save_msg[msg].val[idx][bd][ch].chData.temp,
		(int)myData->save_msg[msg].val[idx][bd][ch].chData.reservedCmd,
		myData->save_msg[msg].val[idx][bd][ch].chData.totalCycle,
		myData->save_msg[msg].val[idx][bd][ch].chData.currentCycle,
		myData->save_msg[msg].val[idx][bd][ch].chData.avgV,
		myData->save_msg[msg].val[idx][bd][ch].chData.avgI);
	fflush(fp);
//22
	#endif
	#if NETWORK_VERSION >= 4102
		#if VENDER != 2 //NOT SDI
	fprintf(fp, ",%ld,%ld,%ld,%ld,%ld,%ld,%ld",
		myData->save_msg[msg].val[idx][bd][ch].chData.IntegralAmpareHour,
		myData->save_msg[msg].val[idx][bd][ch].chData.IntegralWattHour,
		myData->save_msg[msg].val[idx][bd][ch].chData.ChargeAmpareHour,
		myData->save_msg[msg].val[idx][bd][ch].chData.ChargeWattHour,
		myData->save_msg[msg].val[idx][bd][ch].chData.DischargeAmpareHour,
		myData->save_msg[msg].val[idx][bd][ch].chData.DischargeWattHour,
		myData->save_msg[msg].val[idx][bd][ch].chData.cvTime);
	fflush(fp);
//29
			#if PROGRAM_VERSION1 == 0
				#if PROGRAM_VERSION2 >= 1
	fprintf(fp, ",%ld,%d,%d,%ld",
		myData->save_msg[msg].val[idx][bd][ch].chData.Farad,	
		(int)myData->save_msg[msg].val[idx][bd][ch].chData.totalRunTime_carry,
		myData->save_msg[msg].val[idx][bd][ch].chData.cycleNo,
		myData->save_msg[msg].val[idx][bd][ch].chData.temp1);
	fflush(fp);
//33
				#endif	
				#if EDLC_TYPE == 1
	//20160229 khk add start	
	fprintf(fp, ",%ld,%ld,%ld,%ld,%ld,%ld",
		myData->save_msg[msg].val[idx][bd][ch].chData.c_v1,	
		myData->save_msg[msg].val[idx][bd][ch].chData.c_v2,
		myData->save_msg[msg].val[idx][bd][ch].chData.c_t1,
		myData->save_msg[msg].val[idx][bd][ch].chData.c_t2,
		myData->save_msg[msg].val[idx][bd][ch].chData.capacitance_iec,
		myData->save_msg[msg].val[idx][bd][ch].chData.capacitance_maxwell);
	fflush(fp);
//37
				#endif
	//20160229 khk add end	
	//20170105 oys add start
				#if PROGRAM_VERSION2 >= 2
	fprintf(fp, ",%ld,%ld,%ld,%ld",
		myData->save_msg[msg].val[idx][bd][ch].chData.chargeCCAh,
		myData->save_msg[msg].val[idx][bd][ch].chData.chargeCVAh,
		myData->save_msg[msg].val[idx][bd][ch].chData.dischargeCCAh,
		myData->save_msg[msg].val[idx][bd][ch].chData.dischargeCVAh);
	fflush(fp);
		
	fprintf(fp, ",%ld,%ld,%ld,%ld,%ld",
		myData->save_msg[msg].val[idx][bd][ch].chData.z_100mS,
		myData->save_msg[msg].val[idx][bd][ch].chData.z_1S,
		myData->save_msg[msg].val[idx][bd][ch].chData.z_5S,
		myData->save_msg[msg].val[idx][bd][ch].chData.z_30S,
		myData->save_msg[msg].val[idx][bd][ch].chData.z_60S);
	fflush(fp);
				#endif
	//20170105 oys add end
			#endif
			#if PROGRAM_VERSION1 > 0
	fprintf(fp, ",%ld,%d,%ld,%d,%ld,%ld,%ld,%ld,%ld,%ld",
		myData->save_msg[msg].val[idx][bd][ch].chData.Farad,	
		(int)myData->save_msg[msg].val[idx][bd][ch].chData.totalRunTime_carry,
		myData->save_msg[msg].val[idx][bd][ch].chData.temp1,
		myData->save_msg[msg].val[idx][bd][ch].chData.cycleNo,
		myData->save_msg[msg].val[idx][bd][ch].chData.startVoltage,
		myData->save_msg[msg].val[idx][bd][ch].chData.maxVoltage,
		myData->save_msg[msg].val[idx][bd][ch].chData.minVoltage,
		myData->save_msg[msg].val[idx][bd][ch].chData.startTemp,
		myData->save_msg[msg].val[idx][bd][ch].chData.maxTemp,
		myData->save_msg[msg].val[idx][bd][ch].chData.minTemp);
	fflush(fp);
//39	
			#endif
		#endif
		
		#ifdef _AMBIENT_GAS_FLAG
		fprintf(fp, ",%ld,%ld",
			myData->save_msg[msg].val[idx][bd][ch].chData.ambientTemp,
			myData->save_msg[msg].val[idx][bd][ch].chData.gasVoltage);
		fflush(fp);
		#endif

		#if REAL_TIME == 1
		fprintf(fp, ",%ld,%ld",
			myData->save_msg[msg].val[idx][bd][ch].chData.realDate,
			myData->save_msg[msg].val[idx][bd][ch].chData.realClock);
		fflush(fp);	//+2
		#endif
		#if CHAMBER_TEMP_HUMIDITY == 1	//hun_210227
		fprintf(fp, ",%ld,%ld,%ld,%ld,%ld",
			myData->save_msg[msg].val[idx][bd][ch].chData.Chamber_Temp,
			myData->save_msg[msg].val[idx][bd][ch].chData.humi,
			myData->save_msg[msg].val[idx][bd][ch].chData.chargeAccAh,
			myData->save_msg[msg].val[idx][bd][ch].chData.dischargeAccAh,
			myData->save_msg[msg].val[idx][bd][ch].chData.EfficiencyAh);
		#endif
		#ifdef _CH_CHAMBER_DATA 	//210316 lyhw
		fprintf(fp, ",%ld",
			myData->save_msg[msg].val[idx][bd][ch].chData.Chamber_Temp);
		fflush(fp);
		#endif
		#if VENDER == 1 && CH_AUX_DATA == 1		//190807 pthw
		for(i = 0; i < MAX_CH_AUX_DATA; i++){
			fprintf(fp, ",%ld",
				myData->save_msg[msg].val[idx][bd][ch].chData.ch_AuxTemp[i]);
		}
		for(i = 0; i < MAX_CH_AUX_DATA; i++){
			fprintf(fp, ",%ld",
			myData->save_msg[msg].val[idx][bd][ch].chData.ch_AuxVoltage[i]);
		}
		fflush(fp);
		#endif
		#if VENDER == 3 //20200629 ONLY SK
		fprintf(fp, ",%ld",
			myData->save_msg[msg].val[idx][bd][ch].chData.Chamber_Temp);
		fflush(fp);
		#endif	
		#ifdef _CH_SWELLING_DATA 
		for(i = 0; i < MAX_CH_PRESSURE_DATA; i++){
			fprintf(fp, ",%ld",
				myData->save_msg[msg].val[idx][bd][ch].chData.PressureData[i]);
		}
		for(i = 0; i < MAX_CH_THICKNESS_DATA; i++){
			fprintf(fp, ",%ld",
			myData->save_msg[msg].val[idx][bd][ch].chData.ThicknessData[i]);
		}
		fflush(fp);
		#endif	
	
		
		#if CH_SWELLING_DATA == 1		//210316 NV Use lyhw
		for(i = 0; i < MAX_CH_PRESSURE_DATA; i++){
			fprintf(fp, ",%ld",
				myData->save_msg[msg].val[idx][bd][ch].chData.PressureData[i]);
		}
		for(i = 0; i < MAX_CH_THICKNESS_DATA; i++){
			fprintf(fp, ",%ld",
			myData->save_msg[msg].val[idx][bd][ch].chData.ThicknessData[i]);
		}
		fflush(fp);
		#endif
		#ifdef _TRACKING_MODE
		fprintf(fp, ",%ld",
			myData->save_msg[msg].val[idx][bd][ch].chData.SOC);
		fflush(fp);
		fprintf(fp, ",%ld", //211022
			myData->save_msg[msg].val[idx][bd][ch].chData.SOH);
		fflush(fp);
		#endif
		#if GAS_DATA_CONTROL == 1 //210923 lyhw
		fprintf(fp, ",%ld,%ld,%ld,%ld,%ld,%ld,%ld",
			myData->save_msg[msg].val[idx][bd][ch].chData.gas_eCo2,
			myData->save_msg[msg].val[idx][bd][ch].chData.gas_Temp,
			myData->save_msg[msg].val[idx][bd][ch].chData.gas_AH,
			myData->save_msg[msg].val[idx][bd][ch].chData.gas_Baseline,
			myData->save_msg[msg].val[idx][bd][ch].chData.gas_TVOC,
			myData->save_msg[msg].val[idx][bd][ch].chData.gas_Ethanol,
			myData->save_msg[msg].val[idx][bd][ch].chData.gas_H2);
		fflush(fp);
		#endif
		#if VENDER == 3 && CH_AUX_DATA == 1		//211027
		for(i = 0; i < MAX_CH_AUX_DATA; i++){
			fprintf(fp, ",%ld",
				myData->save_msg[msg].val[idx][bd][ch].chData.ch_AuxTemp[i]);
		}
		for(i = 0; i < MAX_CH_AUX_DATA; i++){
			fprintf(fp, ",%ld",
			myData->save_msg[msg].val[idx][bd][ch].chData.ch_AuxVoltage[i]);
		}
		fflush(fp);
		#endif
	#endif
#endif
#ifdef __SDI_MES_VER4__
	fprintf(fp,
		"%ld,%d,%d,%d,%d,%d,%d,%d,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%lu,%lu,%lu,%lu,%ld,%ld,%ld,%ld,%ld,%d,%d,%lu,%lu,%ld,%ld,%lu,%lu,%ld,%ld,%ld,%ld,%lu,%lu,%ld,%ld,%ld,%ld,%ld,%lu,%ld,%ld",
		myData->save_msg[msg].val[idx][bd][ch].chData.resultIndex,
		(int)myData->save_msg[msg].val[idx][bd][ch].chData.state,
		(int)myData->save_msg[msg].val[idx][bd][ch].chData.type,
		(int)myData->save_msg[msg].val[idx][bd][ch].chData.mode,
		(int)myData->save_msg[msg].val[idx][bd][ch].chData.select,
		(int)myData->save_msg[msg].val[idx][bd][ch].chData.cvFlag,
		(int)myData->save_msg[msg].val[idx][bd][ch].chData.code,
		(int)myData->save_msg[msg].val[idx][bd][ch].chData.stepNo,
		myData->save_msg[msg].val[idx][bd][ch].chData.Vsens,
		myData->save_msg[msg].val[idx][bd][ch].chData.Isens,
		myData->save_msg[msg].val[idx][bd][ch].chData.ChargeAmpareHour,
		myData->save_msg[msg].val[idx][bd][ch].chData.DischargeAmpareHour,
		myData->save_msg[msg].val[idx][bd][ch].chData.ChargeWatt,
		myData->save_msg[msg].val[idx][bd][ch].chData.DischargeWatt,
		myData->save_msg[msg].val[idx][bd][ch].chData.ChargeWattHour,
		myData->save_msg[msg].val[idx][bd][ch].chData.DischargeWattHour,
		myData->save_msg[msg].val[idx][bd][ch].chData.runTime_day,
		myData->save_msg[msg].val[idx][bd][ch].chData.runTime,
		myData->save_msg[msg].val[idx][bd][ch].chData.totalRunTime_carry,
		myData->save_msg[msg].val[idx][bd][ch].chData.totalRunTime,
		myData->save_msg[msg].val[idx][bd][ch].chData.z,
		myData->save_msg[msg].val[idx][bd][ch].chData.temp[0][0],
		myData->save_msg[msg].val[idx][bd][ch].chData.temp[0][1],
		myData->save_msg[msg].val[idx][bd][ch].chData.temp[0][2],
		myData->save_msg[msg].val[idx][bd][ch].chData.temp[0][3],
		(int)myData->save_msg[msg].val[idx][bd][ch].chData.reservedCmd,
		myData->save_msg[msg].val[idx][bd][ch].chData.gotoCycleCount,
		myData->save_msg[msg].val[idx][bd][ch].chData.totalCycle,
		myData->save_msg[msg].val[idx][bd][ch].chData.currentCycle,
		myData->save_msg[msg].val[idx][bd][ch].chData.avgV,
		myData->save_msg[msg].val[idx][bd][ch].chData.avgI,
		myData->save_msg[msg].val[idx][bd][ch].chData.cvTime_day,
		myData->save_msg[msg].val[idx][bd][ch].chData.cvTime,
		myData->save_msg[msg].val[idx][bd][ch].chData.realDate,
		myData->save_msg[msg].val[idx][bd][ch].chData.realClock,
		myData->save_msg[msg].val[idx][bd][ch].chData.IntegralAmpareHour,
		myData->save_msg[msg].val[idx][bd][ch].chData.IntegralWattHour,
		myData->save_msg[msg].val[idx][bd][ch].chData.ccTime_day,
		myData->save_msg[msg].val[idx][bd][ch].chData.ccTime,
		myData->save_msg[msg].val[idx][bd][ch].chData.charge_cc_ampare_hour,
		myData->save_msg[msg].val[idx][bd][ch].chData.charge_cv_ampare_hour,
		myData->save_msg[msg].val[idx][bd][ch].chData.discharge_cc_ampare_hour,
		myData->save_msg[msg].val[idx][bd][ch].chData.discharge_cv_ampare_hour,
		myData->save_msg[msg].val[idx][bd][ch].chData.startVoltage,
		myData->save_msg[msg].val[idx][bd][ch].chData.step_count,
		myData->save_msg[msg].val[idx][bd][ch].chData.maxVoltage,
		myData->save_msg[msg].val[idx][bd][ch].chData.minVoltage);
	fflush(fp);
#ifdef _CH_CHAMBER_DATA	//211025 hun
	fprintf(fp, ",%ld",
		myData->save_msg[msg].val[idx][bd][ch].chData.Chamber_Temp);
	fflush(fp);
#endif
#ifdef _CH_SWELLING_DATA		//211025 hun
	for(i = 0; i < MAX_CH_PRESSURE_DATA; i++){
		fprintf(fp, ",%ld",
			myData->save_msg[msg].val[idx][bd][ch].chData.PressureData[i]);
	}
	for(i = 0; i < MAX_CH_THICKNESS_DATA; i++){
		fprintf(fp, ",%ld",
		myData->save_msg[msg].val[idx][bd][ch].chData.ThicknessData[i]);
	}
	fflush(fp);
#endif
#ifdef _ACIR	//220124 hun
	fprintf(fp, ",%ld,%ld",
		myData->save_msg[msg].val[idx][bd][ch].chData.acir_voltage,
		myData->save_msg[msg].val[idx][bd][ch].chData.acir);
	fflush(fp);
#endif
#endif

	fprintf(fp,"\n");
	fflush(fp);
	fclose(fp);

	// 130226 oys w : AuxDataSave
	//20171205 sch modify
#if CH_AUX_DATA == 0
	if(myPs->config.AuxData_saveFlag == 0){
		if(installedTemp > 0)
		{			
			memset(cmd, 0, sizeof(cmd));
#if DATA_SAVE_VER == 1
			sprintf(cmd,
			"/root/cycler_data/resultData/ch%02d/ch%02d_SaveData%04d_auxT.csv",
		   	p_ch+1, p_ch+1, (int)myPs->resultData[p_ch].fileIndex);
#else
			sprintf(cmd,
			"/root/cycler_data/resultData/ch%02d/ch%02d_SaveData%02d_auxT.csv",
		   	p_ch+1, p_ch+1, (int)myPs->resultData[p_ch].fileIndex);
#endif
		
			fp = fopen(cmd, "a+");
		
			if(fp == NULL || fp < 0) {
				userlog(DEBUG_LOG, psName, "Save_ResultData_2 error -10\n");
				return -10;
			}
				fprintf(fp, "%ld",
				myData->save_msg[msg].val[idx][bd][ch].chData.resultIndex);
			
			for (i=0; i < installedTemp; i++)
			{
				fprintf(fp, ",%d,%ld",
				(int)myData->save_msg[msg].auxData[idx][i].auxChNo,
				myData->save_msg[msg].auxData[idx][i].val);
			}
			fprintf(fp, "\n");
			fflush(fp);
			fclose(fp);
		}
		if(installedAuxV > 0)
		{			
			memset(cmd, 0, sizeof(cmd));
#if DATA_SAVE_VER == 1
			sprintf(cmd,
			"/root/cycler_data/resultData/ch%02d/ch%02d_SaveData%04d_auxV.csv",
		   	p_ch+1, p_ch+1, (int)myPs->resultData[p_ch].fileIndex);
#else
			sprintf(cmd,
			"/root/cycler_data/resultData/ch%02d/ch%02d_SaveData%02d_auxV.csv",
		   	p_ch+1, p_ch+1, (int)myPs->resultData[p_ch].fileIndex);
#endif
		
			fp = fopen(cmd, "a+");
		
			if(fp == NULL || fp < 0) {
				userlog(DEBUG_LOG, psName, "Save_ResultData_2 error -20\n");
				return -20;
			}
				fprintf(fp, "%ld",
				myData->save_msg[msg].val[idx][bd][ch].chData.resultIndex);
		
			for (i=MAX_AUX_TEMP_DATA; i<installedAuxV + MAX_AUX_TEMP_DATA; i++)
			{
				fprintf(fp, ",%d,%ld",
				(int)myData->save_msg[msg].auxData[idx][i].auxChNo,
				myData->save_msg[msg].auxData[idx][i].val);
			}
			fprintf(fp, "\n");
			fflush(fp);
			fclose(fp);
		}
	}else if(myPs->config.AuxData_saveFlag == 1){
		for(i = 0; i < installedTemp; i++){
			if(p_ch+1 == myData->auxSetData[i].chNo){
					aux_cnt++;
			}
		}
		if((installedTemp > 0) && (aux_cnt > 0))
		{			
			memset(cmd, 0, sizeof(cmd));
#if DATA_SAVE_VER == 1
			sprintf(cmd,
			"/root/cycler_data/resultData/ch%02d/ch%02d_SaveData%04d_auxT.csv",
		   	p_ch+1, p_ch+1, (int)myPs->resultData[p_ch].fileIndex);
#else
			sprintf(cmd,
			"/root/cycler_data/resultData/ch%02d/ch%02d_SaveData%02d_auxT.csv",
		   	p_ch+1, p_ch+1, (int)myPs->resultData[p_ch].fileIndex);
#endif
		
			fp = fopen(cmd, "a+");
		
			if(fp == NULL || fp < 0) {
				userlog(DEBUG_LOG, psName, "Save_ResultData_2 error -10\n");
				return -10;
			}
				fprintf(fp, "%ld",
				myData->save_msg[msg].val[idx][bd][ch].chData.resultIndex);
			
			for (i=0; i < installedTemp; i++)
			{
				if(p_ch+1 == myData->auxSetData[i].chNo)
				{
					fprintf(fp, ",%d,%ld",
					(int)myData->save_msg[msg].auxData[idx][i].auxChNo,
					myData->save_msg[msg].auxData[idx][i].val);
				}
			}
			fprintf(fp, "\n");
			fflush(fp);
			fclose(fp);
			aux_cnt = 0;
		}
		for (i = installedTemp; i < installedAuxV + installedTemp; i++)
		{
			if(p_ch+1 == myData->auxSetData[i].chNo){
					aux_cnt++;
			}
		}
		if((installedAuxV > 0) && (aux_cnt > 0))
		{			
			memset(cmd, 0, sizeof(cmd));
#if DATA_SAVE_VER == 1
			sprintf(cmd,
			"/root/cycler_data/resultData/ch%02d/ch%02d_SaveData%04d_auxV.csv",
		   	p_ch+1, p_ch+1, (int)myPs->resultData[p_ch].fileIndex);
#else
			sprintf(cmd,
			"/root/cycler_data/resultData/ch%02d/ch%02d_SaveData%02d_auxV.csv",
		   	p_ch+1, p_ch+1, (int)myPs->resultData[p_ch].fileIndex);
#endif
		
			fp = fopen(cmd, "a+");
		
			if(fp == NULL || fp < 0) {
				userlog(DEBUG_LOG, psName, "Save_ResultData_2 error -20\n");
				return -20;
			}
				fprintf(fp, "%ld",
				myData->save_msg[msg].val[idx][bd][ch].chData.resultIndex);
		
			for (i=MAX_AUX_TEMP_DATA; i<installedAuxV+MAX_AUX_TEMP_DATA; i++)
			{
				auxNo = i - MAX_AUX_TEMP_DATA + installedTemp;
				if(p_ch+1 == myData->auxSetData[auxNo].chNo)
				{
					fprintf(fp, ",%d,%ld",
					(int)myData->save_msg[msg].auxData[idx][i].auxChNo,
					myData->save_msg[msg].auxData[idx][i].val);
				}
			}
			fprintf(fp, "\n");
			fflush(fp);
			fclose(fp);
			aux_cnt = 0;
		}
	}
#endif

	if(myData->save_msg[msg].val[idx][bd][ch].chData.select == SAVE_FLAG_SAVING_END) {
		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd, "/root/cycler_data/resultData/ch%02d/ch%02d_SaveEndData.csv",
			p_ch+1, p_ch+1);
		fp = fopen(cmd, "a+");
		if(fp == NULL || fp < 0) {
			userlog(DEBUG_LOG, psName, "Save_ResultData_2 error -2\n");
			return -3;
		}
#ifdef __LG_VER1__
	#if NETWORK_VERSION >= 4101
	fprintf(fp,
		"%ld,%d,%d,%d,%d,%d,%d,%d,%ld,%ld,%ld,%ld,%ld,%lu,%lu,%ld,%ld,%d,%lu,%lu,%ld,%ld",
		myData->save_msg[msg].val[idx][bd][ch].chData.resultIndex,
		(int)myData->save_msg[msg].val[idx][bd][ch].chData.state,
		(int)myData->save_msg[msg].val[idx][bd][ch].chData.type,
		(int)myData->save_msg[msg].val[idx][bd][ch].chData.mode,
		(int)myData->save_msg[msg].val[idx][bd][ch].chData.select,
		(int)myData->save_msg[msg].val[idx][bd][ch].chData.code,
		(int)myData->save_msg[msg].val[idx][bd][ch].chData.grade,
		(int)myData->save_msg[msg].val[idx][bd][ch].chData.stepNo,
		myData->save_msg[msg].val[idx][bd][ch].chData.Vsens,
		myData->save_msg[msg].val[idx][bd][ch].chData.Isens,
		myData->save_msg[msg].val[idx][bd][ch].chData.capacity,
		myData->save_msg[msg].val[idx][bd][ch].chData.watt,
		myData->save_msg[msg].val[idx][bd][ch].chData.wattHour,
		myData->save_msg[msg].val[idx][bd][ch].chData.runTime,
		myData->save_msg[msg].val[idx][bd][ch].chData.totalRunTime,
		myData->save_msg[msg].val[idx][bd][ch].chData.z,
		myData->save_msg[msg].val[idx][bd][ch].chData.temp,
		(int)myData->save_msg[msg].val[idx][bd][ch].chData.reservedCmd,
		myData->save_msg[msg].val[idx][bd][ch].chData.totalCycle,
		myData->save_msg[msg].val[idx][bd][ch].chData.currentCycle,
		myData->save_msg[msg].val[idx][bd][ch].chData.avgV,
		myData->save_msg[msg].val[idx][bd][ch].chData.avgI);
	fflush(fp);
//22
	#endif
	#if NETWORK_VERSION >= 4102
		#if VENDER != 2 //NOT SDI
	fprintf(fp, ",%ld,%ld,%ld,%ld,%ld,%ld,%ld",
		myData->save_msg[msg].val[idx][bd][ch].chData.IntegralAmpareHour,
		myData->save_msg[msg].val[idx][bd][ch].chData.IntegralWattHour,
		myData->save_msg[msg].val[idx][bd][ch].chData.ChargeAmpareHour,
		myData->save_msg[msg].val[idx][bd][ch].chData.ChargeWattHour,
		myData->save_msg[msg].val[idx][bd][ch].chData.DischargeAmpareHour,
		myData->save_msg[msg].val[idx][bd][ch].chData.DischargeWattHour,
		myData->save_msg[msg].val[idx][bd][ch].chData.cvTime);
	fflush(fp);
//29
			#if PROGRAM_VERSION1 == 0
				#if PROGRAM_VERSION2 >= 1
	fprintf(fp, ",%ld,%d,%d,%ld",
		myData->save_msg[msg].val[idx][bd][ch].chData.Farad,	
		(int)myData->save_msg[msg].val[idx][bd][ch].chData.totalRunTime_carry,
		myData->save_msg[msg].val[idx][bd][ch].chData.cycleNo,
		myData->save_msg[msg].val[idx][bd][ch].chData.temp1);
	fflush(fp);
//33
				#endif
				#if EDLC_TYPE == 1
	//20160229 khk add start	
	fprintf(fp, ",%ld,%ld,%ld,%ld,%ld,%ld",
		myData->save_msg[msg].val[idx][bd][ch].chData.c_v1,	
		myData->save_msg[msg].val[idx][bd][ch].chData.c_v2,
		myData->save_msg[msg].val[idx][bd][ch].chData.c_t1,
		myData->save_msg[msg].val[idx][bd][ch].chData.c_t2,
		myData->save_msg[msg].val[idx][bd][ch].chData.capacitance_iec,
		myData->save_msg[msg].val[idx][bd][ch].chData.capacitance_maxwell);
	fflush(fp);
//37
				#endif
	//20160229 khk add end
	//20170105 oys add start
				#if PROGRAM_VERSION2 >= 2
	fprintf(fp, ",%ld,%ld,%ld,%ld",
		myData->save_msg[msg].val[idx][bd][ch].chData.chargeCCAh,
		myData->save_msg[msg].val[idx][bd][ch].chData.chargeCVAh,
		myData->save_msg[msg].val[idx][bd][ch].chData.dischargeCCAh,
		myData->save_msg[msg].val[idx][bd][ch].chData.dischargeCVAh);
	fflush(fp);

	fprintf(fp, ",%ld,%ld,%ld,%ld,%ld",
		myData->save_msg[msg].val[idx][bd][ch].chData.z_100mS,
		myData->save_msg[msg].val[idx][bd][ch].chData.z_1S,
		myData->save_msg[msg].val[idx][bd][ch].chData.z_5S,
		myData->save_msg[msg].val[idx][bd][ch].chData.z_30S,
		myData->save_msg[msg].val[idx][bd][ch].chData.z_60S);
	fflush(fp);
				#endif
	//20170105 oys add end
			#endif
			#if PROGRAM_VERSION1 > 0
	fprintf(fp, ",%ld,%d,%ld,%d,%ld,%ld,%ld,%ld,%ld,%ld",
		myData->save_msg[msg].val[idx][bd][ch].chData.Farad,	
		(int)myData->save_msg[msg].val[idx][bd][ch].chData.totalRunTime_carry,
		myData->save_msg[msg].val[idx][bd][ch].chData.temp1,
		myData->save_msg[msg].val[idx][bd][ch].chData.cycleNo,
		myData->save_msg[msg].val[idx][bd][ch].chData.startVoltage,
		myData->save_msg[msg].val[idx][bd][ch].chData.maxVoltage,
		myData->save_msg[msg].val[idx][bd][ch].chData.minVoltage,
		myData->save_msg[msg].val[idx][bd][ch].chData.startTemp,
		myData->save_msg[msg].val[idx][bd][ch].chData.maxTemp,
		myData->save_msg[msg].val[idx][bd][ch].chData.minTemp);
	fflush(fp);
//39
			#endif
		#endif
		#ifdef _AMBIENT_GAS_FLAG
		fprintf(fp, ",%ld,%ld",
			myData->save_msg[msg].val[idx][bd][ch].chData.ambientTemp,
			myData->save_msg[msg].val[idx][bd][ch].chData.gasVoltage);
		fflush(fp);
		#endif
	
		#if REAL_TIME == 1
		fprintf(fp, ",%ld,%ld",
			myData->save_msg[msg].val[idx][bd][ch].chData.realDate,
			myData->save_msg[msg].val[idx][bd][ch].chData.realClock);
		fflush(fp);
//+2
		#endif
		#if CHAMBER_TEMP_HUMIDITY == 1	//hun_210227
		fprintf(fp, ",%ld,%ld,%ld,%ld,%ld",
			myData->save_msg[msg].val[idx][bd][ch].chData.Chamber_Temp,
			myData->save_msg[msg].val[idx][bd][ch].chData.humi,
			myData->save_msg[msg].val[idx][bd][ch].chData.chargeAccAh,
			myData->save_msg[msg].val[idx][bd][ch].chData.dischargeAccAh,
			myData->save_msg[msg].val[idx][bd][ch].chData.EfficiencyAh);
		#endif
		#ifdef _CH_CHAMBER_DATA 	
		fprintf(fp, ",%ld",
			myData->save_msg[msg].val[idx][bd][ch].chData.Chamber_Temp);
		fflush(fp);
		#endif
		//190807 pthw
		#if VENDER == 1 && CH_AUX_DATA == 1
		for(i =0 ; i < MAX_CH_AUX_DATA; i++){
			fprintf(fp, ",%ld",
			myData->save_msg[msg].val[idx][bd][ch].chData.ch_AuxTemp[i]);
		}
		fflush(fp);
		for(i =0 ; i < MAX_CH_AUX_DATA; i++){
			fprintf(fp, ",%ld",
			myData->save_msg[msg].val[idx][bd][ch].chData.ch_AuxVoltage[i]);
		}
		fflush(fp);
		#endif
		#if VENDER == 3 	//20200629 ONLY SK
		fprintf(fp, ",%ld",
			myData->save_msg[msg].val[idx][bd][ch].chData.Chamber_Temp);
		fflush(fp);
		#endif
		#ifdef _CH_SWELLING_DATA 
		for(i = 0; i < MAX_CH_PRESSURE_DATA; i++){
			fprintf(fp, ",%ld",
				myData->save_msg[msg].val[idx][bd][ch].chData.PressureData[i]);
		}
		for(i = 0; i < MAX_CH_THICKNESS_DATA; i++){
			fprintf(fp, ",%ld",
			myData->save_msg[msg].val[idx][bd][ch].chData.ThicknessData[i]);
		}
		fflush(fp);
		#endif
		
		#if CH_SWELLING_DATA == 1		//210316 NV Use lyhw
		for(i = 0; i < MAX_CH_PRESSURE_DATA; i++){
			fprintf(fp, ",%ld",
				myData->save_msg[msg].val[idx][bd][ch].chData.PressureData[i]);
		}
		for(i = 0; i < MAX_CH_THICKNESS_DATA; i++){
			fprintf(fp, ",%ld",
			myData->save_msg[msg].val[idx][bd][ch].chData.ThicknessData[i]);
		}
		fflush(fp);
		#endif
		#ifdef _TRACKING_MODE
		fprintf(fp, ",%ld",
			myData->save_msg[msg].val[idx][bd][ch].chData.SOC);
		fflush(fp);
		fprintf(fp, ",%ld", //211022
			myData->save_msg[msg].val[idx][bd][ch].chData.SOH);
		fflush(fp);
		#endif
		#if GAS_DATA_CONTROL == 1 //210923 lyhw
		fprintf(fp, ",%ld,%ld,%ld,%ld,%ld,%ld,%ld",
			myData->save_msg[msg].val[idx][bd][ch].chData.gas_eCo2,
			myData->save_msg[msg].val[idx][bd][ch].chData.gas_Temp,
			myData->save_msg[msg].val[idx][bd][ch].chData.gas_AH,
			myData->save_msg[msg].val[idx][bd][ch].chData.gas_Baseline,
			myData->save_msg[msg].val[idx][bd][ch].chData.gas_TVOC,
			myData->save_msg[msg].val[idx][bd][ch].chData.gas_Ethanol,
			myData->save_msg[msg].val[idx][bd][ch].chData.gas_H2);
		fflush(fp);
		#endif
		#if VENDER == 3 && CH_AUX_DATA == 1 //211027
		for(i =0 ; i < MAX_CH_AUX_DATA; i++){
			fprintf(fp, ",%ld",
			myData->save_msg[msg].val[idx][bd][ch].chData.ch_AuxTemp[i]);
		}
		fflush(fp);
		for(i =0 ; i < MAX_CH_AUX_DATA; i++){
			fprintf(fp, ",%ld",
			myData->save_msg[msg].val[idx][bd][ch].chData.ch_AuxVoltage[i]);
		}
		fflush(fp);
		#endif
	#endif
#endif
#ifdef __SDI_MES_VER4__
	fprintf(fp,
		"%ld,%d,%d,%d,%d,%d,%d,%d,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%lu,%lu,%lu,%lu,%ld,%ld,%ld,%ld,%ld,%d,%d,%lu,%lu,%ld,%ld,%lu,%lu,%ld,%ld,%ld,%ld,%lu,%lu,%ld,%ld,%ld,%ld,%ld,%lu,%ld,%ld",
		myData->save_msg[msg].val[idx][bd][ch].chData.resultIndex,
		(int)myData->save_msg[msg].val[idx][bd][ch].chData.state,
		(int)myData->save_msg[msg].val[idx][bd][ch].chData.type,
		(int)myData->save_msg[msg].val[idx][bd][ch].chData.mode,
		(int)myData->save_msg[msg].val[idx][bd][ch].chData.select,
		(int)myData->save_msg[msg].val[idx][bd][ch].chData.cvFlag,
		(int)myData->save_msg[msg].val[idx][bd][ch].chData.code,
		(int)myData->save_msg[msg].val[idx][bd][ch].chData.stepNo,
		myData->save_msg[msg].val[idx][bd][ch].chData.Vsens,
		myData->save_msg[msg].val[idx][bd][ch].chData.Isens,
		myData->save_msg[msg].val[idx][bd][ch].chData.ChargeAmpareHour,
		myData->save_msg[msg].val[idx][bd][ch].chData.DischargeAmpareHour,
		myData->save_msg[msg].val[idx][bd][ch].chData.ChargeWatt,
		myData->save_msg[msg].val[idx][bd][ch].chData.DischargeWatt,
		myData->save_msg[msg].val[idx][bd][ch].chData.ChargeWattHour,
		myData->save_msg[msg].val[idx][bd][ch].chData.DischargeWattHour,
		myData->save_msg[msg].val[idx][bd][ch].chData.runTime_day,
		myData->save_msg[msg].val[idx][bd][ch].chData.runTime,
		myData->save_msg[msg].val[idx][bd][ch].chData.totalRunTime_carry,
		myData->save_msg[msg].val[idx][bd][ch].chData.totalRunTime,
		myData->save_msg[msg].val[idx][bd][ch].chData.z,
		myData->save_msg[msg].val[idx][bd][ch].chData.temp[0][0],
		myData->save_msg[msg].val[idx][bd][ch].chData.temp[0][1],
		myData->save_msg[msg].val[idx][bd][ch].chData.temp[0][2],
		myData->save_msg[msg].val[idx][bd][ch].chData.temp[0][3],
		(int)myData->save_msg[msg].val[idx][bd][ch].chData.reservedCmd,
		myData->save_msg[msg].val[idx][bd][ch].chData.gotoCycleCount,
		myData->save_msg[msg].val[idx][bd][ch].chData.totalCycle,
		myData->save_msg[msg].val[idx][bd][ch].chData.currentCycle,
		myData->save_msg[msg].val[idx][bd][ch].chData.avgV,
		myData->save_msg[msg].val[idx][bd][ch].chData.avgI,
		myData->save_msg[msg].val[idx][bd][ch].chData.cvTime_day,
		myData->save_msg[msg].val[idx][bd][ch].chData.cvTime,
		myData->save_msg[msg].val[idx][bd][ch].chData.realDate,
		myData->save_msg[msg].val[idx][bd][ch].chData.realClock,
		myData->save_msg[msg].val[idx][bd][ch].chData.IntegralAmpareHour,
		myData->save_msg[msg].val[idx][bd][ch].chData.IntegralWattHour,
		myData->save_msg[msg].val[idx][bd][ch].chData.ccTime_day,
		myData->save_msg[msg].val[idx][bd][ch].chData.ccTime,
		myData->save_msg[msg].val[idx][bd][ch].chData.charge_cc_ampare_hour,
		myData->save_msg[msg].val[idx][bd][ch].chData.charge_cv_ampare_hour,
		myData->save_msg[msg].val[idx][bd][ch].chData.discharge_cc_ampare_hour,
		myData->save_msg[msg].val[idx][bd][ch].chData.discharge_cv_ampare_hour,
		myData->save_msg[msg].val[idx][bd][ch].chData.startVoltage,
		myData->save_msg[msg].val[idx][bd][ch].chData.step_count,
		myData->save_msg[msg].val[idx][bd][ch].chData.maxVoltage,
		myData->save_msg[msg].val[idx][bd][ch].chData.minVoltage);
	fflush(fp);
#ifdef _CH_CHAMBER_DATA	//211025 hun
	fprintf(fp, ",%ld",
		myData->save_msg[msg].val[idx][bd][ch].chData.Chamber_Temp);
	fflush(fp);
#endif
#ifdef _CH_SWELLING_DATA		//211025 hun
	for(i = 0; i < MAX_CH_PRESSURE_DATA; i++){
		fprintf(fp, ",%ld",
			myData->save_msg[msg].val[idx][bd][ch].chData.PressureData[i]);
	}
	for(i = 0; i < MAX_CH_THICKNESS_DATA; i++){
		fprintf(fp, ",%ld",
		myData->save_msg[msg].val[idx][bd][ch].chData.ThicknessData[i]);
	}
	fflush(fp);
#endif
#ifdef _ACIR	//220124 hun
	fprintf(fp, ",%ld,%ld",
		myData->save_msg[msg].val[idx][bd][ch].chData.acir_voltage,
		myData->save_msg[msg].val[idx][bd][ch].chData.acir);
	fflush(fp);
#endif
#endif

	fprintf(fp, "\n");
	fflush(fp);
	fclose(fp);

		// 130226 oys w : AuxDataSave
		//20171205 sch modify
#if CH_AUX_DATA == 0
		if(myPs->config.AuxData_saveFlag == 0){
			if(installedTemp > 0)
			{			
				memset(cmd, 0, sizeof(cmd));
				sprintf(cmd,
				"/root/cycler_data/resultData/ch%02d/ch%02d_SaveEndData_auxT.csv",p_ch+1, p_ch+1);
	
				fp = fopen(cmd, "a+");
					if(fp == NULL || fp < 0) {
					userlog(DEBUG_LOG, psName, "Save_ResultData_2 error -30\n");
					return -30;
				}
					fprintf(fp, "%ld",
					myData->save_msg[msg].val[idx][bd][ch].chData.resultIndex);
			
				for (i=0; i < installedTemp; i++)
				{
					fprintf(fp, ",%d,%ld",
					(int)myData->save_msg[msg].auxData[idx][i].auxChNo,
					myData->save_msg[msg].auxData[idx][i].val);
				}
				fprintf(fp, "\n");
				fflush(fp);
				fclose(fp);
			}
			if(installedAuxV > 0)
			{			
				memset(cmd, 0, sizeof(cmd));
				sprintf(cmd, 
				"/root/cycler_data/resultData/ch%02d/ch%02d_SaveEndData_auxV.csv",p_ch+1, p_ch+1);
			
				fp = fopen(cmd, "a+");
				
				if(fp == NULL || fp < 0) {
					userlog(DEBUG_LOG, psName, "Save_ResultData_2 error -40\n");
					return -40;
				}

				fprintf(fp, "%ld",
					myData->save_msg[msg].val[idx][bd][ch].chData.resultIndex);	
				for(i=MAX_AUX_TEMP_DATA; i<installedAuxV+MAX_AUX_TEMP_DATA; i++)
				{
						fprintf(fp, ",%d,%ld",
						(int)myData->save_msg[msg].auxData[idx][i].auxChNo,
						myData->save_msg[msg].auxData[idx][i].val);
				}
				fprintf(fp, "\n");
				fflush(fp);
				fclose(fp);
			}
		}else if(myPs->config.AuxData_saveFlag == 1){
			for(i = 0; i < installedTemp; i++){
				if(p_ch+1 == myData->auxSetData[i].chNo){
						aux_cnt++;
				}
			}
			if((installedTemp > 0) && (aux_cnt > 0))
			{			
				memset(cmd, 0, sizeof(cmd));
				sprintf(cmd,
				"/root/cycler_data/resultData/ch%02d/ch%02d_SaveEndData_auxT.csv",p_ch+1, p_ch+1);
	
				fp = fopen(cmd, "a+");
					if(fp == NULL || fp < 0) {
					userlog(DEBUG_LOG, psName, "Save_ResultData_2 error -30\n");
					return -30;
				}
					fprintf(fp, "%ld",
					myData->save_msg[msg].val[idx][bd][ch].chData.resultIndex);
			
				for (i=0; i < installedTemp; i++)
				{
					if(p_ch+1 == myData->auxSetData[i].chNo)
					{
						fprintf(fp, ",%d,%ld",
						(int)myData->save_msg[msg].auxData[idx][i].auxChNo,
						myData->save_msg[msg].auxData[idx][i].val);
					}
				}
				fprintf(fp, "\n");
				fflush(fp);
				fclose(fp);
				aux_cnt = 0;
			}
			for (i = installedTemp; i < installedAuxV + installedTemp; i++)
			{
				if(p_ch+1 == myData->auxSetData[i].chNo){
						aux_cnt++;
				}
			}
			if((installedAuxV > 0) && (aux_cnt > 0))
			{			
				memset(cmd, 0, sizeof(cmd));
				sprintf(cmd, 
				"/root/cycler_data/resultData/ch%02d/ch%02d_SaveEndData_auxV.csv",p_ch+1, p_ch+1);
			
				fp = fopen(cmd, "a+");
				
				if(fp == NULL || fp < 0) {
					userlog(DEBUG_LOG, psName, "Save_ResultData_2 error -40\n");
					return -40;
				}

				fprintf(fp, "%ld",
					myData->save_msg[msg].val[idx][bd][ch].chData.resultIndex);	
				for(i=MAX_AUX_TEMP_DATA; i<installedAuxV+MAX_AUX_TEMP_DATA; i++)
				{
					auxNo = i - MAX_AUX_TEMP_DATA + installedTemp;
					if(p_ch+1 == myData->auxSetData[auxNo].chNo)
					{
						fprintf(fp, ",%d,%ld",
						(int)myData->save_msg[msg].auxData[idx][i].auxChNo,
						myData->save_msg[msg].auxData[idx][i].val);
					}
				}
				fprintf(fp, "\n");
				fflush(fp);
				fclose(fp);
				aux_cnt = 0;
			}
		}
#endif
	}
	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd,
		"/root/cycler_data/resultData/ch%02d/savingFileIndex_last.csv", p_ch+1);
	fp = fopen(cmd, "w+");
	if(fp == NULL || fp < 0) {
		userlog(DEBUG_LOG, psName, "Save_ResultData_2 error -3\n");
		return -4;
	}

	fprintf(fp, "fileIndex %d, ", (int)myPs->resultData[p_ch].fileIndex);
	fprintf(fp, "resultIndex %ld, ",
		myData->save_msg[msg].val[idx][bd][ch].chData.resultIndex);
	fprintf(fp, "open_year %d, ", (int)myPs->resultData[p_ch].open_year);
	fprintf(fp, "open_month %d, ", (int)myPs->resultData[p_ch].open_month);
	fprintf(fp, "open_day %d\n", (int)myPs->resultData[p_ch].open_day);
	fflush(fp);
	fclose(fp);
	
	myPs->resultData[p_ch].resultIndex
		= myData->save_msg[msg].val[idx][bd][ch].chData.resultIndex;
	return count;
}

void Save_PulseData_1(void)
{
	int p_ch, rtn;

	for(p_ch=0; p_ch < myData->mData.config.installedCh; p_ch++) {
		rtn = Save_PulseData_2(p_ch);
		while(rtn) {
			rtn = Save_PulseData_2(p_ch);
		}
	}
	//171227 lyh
	for(p_ch=0; p_ch < myData->mData.config.installedCh; p_ch++) {
		rtn = Save_PulseData_IEC(p_ch);
		while(rtn) {
			rtn = Save_PulseData_IEC(p_ch);
		}
	}
}

int Save_PulseData_2(int p_ch)
{
	char cmd[128];
	int bd, ch, msg, idx, count;
	FILE *fp;

	bd = myData->CellArray1[p_ch].bd;
	ch = myData->CellArray1[p_ch].ch;
	msg = 1;
	count = 0;

	if(myData->pulse_msg[msg][bd][ch].write_idx
		== myData->pulse_msg[msg][bd][ch].read_idx) {
		return 0;
	}

	myData->pulse_msg[msg][bd][ch].read_idx++;
	if(myData->pulse_msg[msg][bd][ch].read_idx >= MAX_PULSE_MSG) {
		myData->pulse_msg[msg][bd][ch].read_idx = 0;
	}
	idx = myData->pulse_msg[msg][bd][ch].read_idx;

	myData->pulse_msg[msg][bd][ch].count--;
	count = myData->pulse_msg[msg][bd][ch].count;

	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd,
		"/root/cycler_data/resultData/ch%02d/ch%02d_PulseData_T%d_S%d.csv",
		p_ch+1, p_ch+1,
		(int)myData->pulse_msg[msg][bd][ch].val[idx].totalCycle,
		(int)myData->pulse_msg[msg][bd][ch].val[idx].stepNo);
	fp = fopen(cmd, "a+");
	if(fp == NULL || fp < 0) {
		userlog(DEBUG_LOG, psName, "Save_PulseData_2 error -1\n");
		return -1;
	}

	fprintf(fp,
		"%ld,%ld,%ld\n",
		myData->pulse_msg[msg][bd][ch].val[idx].runTime,
		myData->pulse_msg[msg][bd][ch].val[idx].Vsens,
		myData->pulse_msg[msg][bd][ch].val[idx].Isens);
	fflush(fp);
	fclose(fp);

	return count;
}
//171227 lyh
int Save_PulseData_IEC(int p_ch)
{
	char cmd[128];
	int bd, ch, msg, idx, count;
	FILE *fp;

	bd = myData->CellArray1[p_ch].bd;
	ch = myData->CellArray1[p_ch].ch;
	msg = 1;
	count = 0;

	if(myData->pulse_msg_iec[msg][bd][ch].write_idx
		== myData->pulse_msg_iec[msg][bd][ch].read_idx) {
		return 0;
	}

	myData->pulse_msg_iec[msg][bd][ch].read_idx++;
	if(myData->pulse_msg_iec[msg][bd][ch].read_idx >= MAX_PULSE_MSG) {
		myData->pulse_msg_iec[msg][bd][ch].read_idx = 0;
	}
	idx = myData->pulse_msg_iec[msg][bd][ch].read_idx;

	myData->pulse_msg_iec[msg][bd][ch].count--;
	count = myData->pulse_msg_iec[msg][bd][ch].count;

	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd,
		"/root/cycler_data/resultData/ch%02d/ch%02d_PulseData_T_IEC%d_S%d.csv",
		p_ch+1, p_ch+1,
		(int)myData->pulse_msg_iec[msg][bd][ch].val[idx].totalCycle,
		(int)myData->pulse_msg_iec[msg][bd][ch].val[idx].stepNo);
	fp = fopen(cmd, "a+");
	if(fp == NULL || fp < 0) {
		userlog(DEBUG_LOG, psName, "Save_PulseData_IEC error -1\n");
		return -1;
	}

	fprintf(fp,
		"%ld,%ld,%ld\n",
		myData->pulse_msg_iec[msg][bd][ch].val[idx].runTime,
		myData->pulse_msg_iec[msg][bd][ch].val[idx].Vsens,
		myData->pulse_msg_iec[msg][bd][ch].val[idx].Isens);
	fflush(fp);
	fclose(fp);

	return count;
}

void Save_10mS_Data_1(void)
{
	int p_ch, bd, ch, msg, msgCount;

	msg = 3;

	if(myPs->config.save_data_type == P0)
		return;

	for(p_ch=0; p_ch < myData->mData.config.installedCh; p_ch++) {
		bd = myData->CellArray1[p_ch].bd;
		ch = myData->CellArray1[p_ch].ch;
		if(myData->save10ms_msg[msg][bd][ch].flag == 1){
			msgCount = myData->save10ms_msg[msg][bd][ch].write_idx+1;

			Save_10mS_Data_2(p_ch, msgCount);
			
			myData->save10ms_msg[msg][bd][ch].write_idx = 0;
			myData->save10ms_msg[msg][bd][ch].flag = 0;
		}
	}
}

int Save_10mS_Data_2(int p_ch, int msgCount)
{
	char cmd[128];
	int bd, ch, msg, idx, count;
#if CH_AUX_DATA == 0
	int i;
#endif
	int aux_cnt, auxNo, installedTemp, installedAuxV;
	FILE *fp;

	bd = myData->CellArray1[p_ch].bd;
	ch = myData->CellArray1[p_ch].ch;
	
	msg = 3;
	count = 0;

	aux_cnt = 0;
	auxNo = 0;
	installedTemp = myData->mData.config.installedTemp;
	installedAuxV = myData->mData.config.installedAuxV;

	if(Open_ResultData_2(p_ch) < 0) {
		return -1;
	}
	
	memset(cmd, 0, sizeof(cmd));
#if DATA_SAVE_VER == 1
	sprintf(cmd, "/root/cycler_data/resultData/ch%02d/ch%02d_SaveData%04d.csv",
		p_ch+1, p_ch+1, (int)myPs->resultData[p_ch].fileIndex);
#else
	sprintf(cmd, "/root/cycler_data/resultData/ch%02d/ch%02d_SaveData%02d.csv",
		p_ch+1, p_ch+1, (int)myPs->resultData[p_ch].fileIndex);
#endif
	fp = fopen(cmd, "a+");
	if(fp == NULL || fp < 0) {
		userlog(DEBUG_LOG, psName, "Save_ResultData_2 error -1\n");
		return -2;
	}
	for(idx = 0; idx < msgCount; idx++){
		if(myData->save10ms_msg[msg][bd][ch].chData[idx].resultIndex == 0)
			continue;
#ifdef __LG_VER1__
	#if NETWORK_VERSION >= 4101
	fprintf(fp,
		"%ld,%d,%d,%d,%d,%d,%d,%d,%ld,%ld,%ld,%ld,%ld,%lu,%lu,%ld,%ld,%d,%lu,%lu,%ld,%ld",
		myData->save10ms_msg[msg][bd][ch].chData[idx].resultIndex,
		(int)myData->save10ms_msg[msg][bd][ch].chData[idx].state,
		(int)myData->save10ms_msg[msg][bd][ch].chData[idx].type,
		(int)myData->save10ms_msg[msg][bd][ch].chData[idx].mode,
		(int)myData->save10ms_msg[msg][bd][ch].chData[idx].select,
		(int)myData->save10ms_msg[msg][bd][ch].chData[idx].code,
		(int)myData->save10ms_msg[msg][bd][ch].chData[idx].grade,
		(int)myData->save10ms_msg[msg][bd][ch].chData[idx].stepNo,
		myData->save10ms_msg[msg][bd][ch].chData[idx].Vsens,
		myData->save10ms_msg[msg][bd][ch].chData[idx].Isens,
		myData->save10ms_msg[msg][bd][ch].chData[idx].capacity,
		myData->save10ms_msg[msg][bd][ch].chData[idx].watt,
		myData->save10ms_msg[msg][bd][ch].chData[idx].wattHour,
		myData->save10ms_msg[msg][bd][ch].chData[idx].runTime,
		myData->save10ms_msg[msg][bd][ch].chData[idx].totalRunTime,
		myData->save10ms_msg[msg][bd][ch].chData[idx].z,
		myData->save10ms_msg[msg][bd][ch].chData[idx].temp,
		(int)myData->save10ms_msg[msg][bd][ch].chData[idx].reservedCmd,
		myData->save10ms_msg[msg][bd][ch].chData[idx].totalCycle,
		myData->save10ms_msg[msg][bd][ch].chData[idx].currentCycle,
		myData->save10ms_msg[msg][bd][ch].chData[idx].avgV,
		myData->save10ms_msg[msg][bd][ch].chData[idx].avgI);
	fflush(fp);
//22
	#endif
	#if NETWORK_VERSION >= 4102
		#if VENDER != 2 //NOT SDI
	fprintf(fp, ",%ld,%ld,%ld,%ld,%ld,%ld,%ld",
		myData->save10ms_msg[msg][bd][ch].chData[idx].IntegralAmpareHour,
		myData->save10ms_msg[msg][bd][ch].chData[idx].IntegralWattHour,
		myData->save10ms_msg[msg][bd][ch].chData[idx].ChargeAmpareHour,
		myData->save10ms_msg[msg][bd][ch].chData[idx].ChargeWattHour,
		myData->save10ms_msg[msg][bd][ch].chData[idx].DischargeAmpareHour,
		myData->save10ms_msg[msg][bd][ch].chData[idx].DischargeWattHour,
		myData->save10ms_msg[msg][bd][ch].chData[idx].cvTime);
	fflush(fp);
//29
			#if PROGRAM_VERSION1 == 0
				#if PROGRAM_VERSION2 >= 1
	fprintf(fp, ",%ld,%d,%d,%ld",
		myData->save10ms_msg[msg][bd][ch].chData[idx].Farad,	
		(int)myData->save10ms_msg[msg][bd][ch].chData[idx].totalRunTime_carry,
		myData->save10ms_msg[msg][bd][ch].chData[idx].cycleNo,
		myData->save10ms_msg[msg][bd][ch].chData[idx].temp1);
	fflush(fp);
//33
				#endif
				#if EDLC_TYPE == 1
	//20160229 khk add start	
	fprintf(fp, ",%ld,%ld,%ld,%ld",
		myData->save10ms_msg[msg][bd][ch].chData[idx].c_v1,	
		myData->save10ms_msg[msg][bd][ch].chData[idx].c_v2,
		myData->save10ms_msg[msg][bd][ch].chData[idx].c_t1,
		myData->save10ms_msg[msg][bd][ch].chData[idx].c_t2);
	fflush(fp);
//37
				#endif
	//20160229 khk add end	
	//20170105 oys add start
				#if PROGRAM_VERSION2 >= 2
	fprintf(fp, ",%ld,%ld,%ld,%ld",
		myData->save10ms_msg[msg][bd][ch].chData[idx].chargeCCAh,	
		myData->save10ms_msg[msg][bd][ch].chData[idx].chargeCVAh,
		myData->save10ms_msg[msg][bd][ch].chData[idx].dischargeCCAh,
		myData->save10ms_msg[msg][bd][ch].chData[idx].dischargeCVAh);
	fflush(fp);
				#endif
	//20170105 oys add end
			#endif
			#if PROGRAM_VERSION1 > 0
	fprintf(fp, ",%ld,%d,%ld,%d,%ld,%ld,%ld,%ld,%ld,%ld",
		myData->save10ms_msg[msg][bd][ch].chData[idx].Farad,	
		(int)myData->save10ms_msg[msg][bd][ch].chData[idx].totalRunTime_carry,
		myData->save10ms_msg[msg][bd][ch].chData[idx].temp1,
		myData->save10ms_msg[msg][bd][ch].chData[idx].cycleNo,
		myData->save10ms_msg[msg][bd][ch].chData[idx].startVoltage,
		myData->save10ms_msg[msg][bd][ch].chData[idx].maxVoltage,
		myData->save10ms_msg[msg][bd][ch].chData[idx].minVoltage,
		myData->save10ms_msg[msg][bd][ch].chData[idx].startTemp,
		myData->save10ms_msg[msg][bd][ch].chData[idx].maxTemp,
		myData->save10ms_msg[msg][bd][ch].chData[idx].minTemp);
	fflush(fp);
//39
			#endif
		#endif
		#if REAL_TIME == 1
	fprintf(fp, ",%ld,%ld",
		myData->save10ms_msg[msg][bd][ch].chData[idx].realDate,
		myData->save10ms_msg[msg][bd][ch].chData[idx].realClock);
	fflush(fp);
//+2
		#endif
		#ifdef _CH_CHAMBER_DATA
		fprintf(fp, ",%ld",
			myData->save10ms_msg[msg][bd][ch].chData[idx].Chamber_Temp);
		fflush(fp);
		#endif
	#endif
#endif
#ifdef __SDI_MES_VER4__
	fprintf(fp,
		"%ld,%d,%d,%d,%d,%d,%d,%d,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%lu,%lu,%lu,%lu,%ld,%ld,%ld,%ld,%ld,%d,%d,%lu,%lu,%ld,%ld,%lu,%lu,%ld,%ld,%ld,%ld,%lu,%lu,%ld,%ld,%ld,%ld,%ld,%lu,%ld,%ld",
		myData->save10ms_msg[msg][bd][ch].chData[idx].resultIndex,
		(int)myData->save10ms_msg[msg][bd][ch].chData[idx].state,
		(int)myData->save10ms_msg[msg][bd][ch].chData[idx].type,
		(int)myData->save10ms_msg[msg][bd][ch].chData[idx].mode,
		(int)myData->save10ms_msg[msg][bd][ch].chData[idx].select,
		(int)myData->save10ms_msg[msg][bd][ch].chData[idx].cvFlag,
		(int)myData->save10ms_msg[msg][bd][ch].chData[idx].code,
		(int)myData->save10ms_msg[msg][bd][ch].chData[idx].stepNo,
		myData->save10ms_msg[msg][bd][ch].chData[idx].Vsens,
		myData->save10ms_msg[msg][bd][ch].chData[idx].Isens,
		myData->save10ms_msg[msg][bd][ch].chData[idx].ChargeAmpareHour,
		myData->save10ms_msg[msg][bd][ch].chData[idx].DischargeAmpareHour,
		myData->save10ms_msg[msg][bd][ch].chData[idx].ChargeWatt,
		myData->save10ms_msg[msg][bd][ch].chData[idx].DischargeWatt,
		myData->save10ms_msg[msg][bd][ch].chData[idx].ChargeWattHour,
		myData->save10ms_msg[msg][bd][ch].chData[idx].DischargeWattHour,
		myData->save10ms_msg[msg][bd][ch].chData[idx].runTime_day,
		myData->save10ms_msg[msg][bd][ch].chData[idx].runTime,
		myData->save10ms_msg[msg][bd][ch].chData[idx].totalRunTime_carry,
		myData->save10ms_msg[msg][bd][ch].chData[idx].totalRunTime,
		myData->save10ms_msg[msg][bd][ch].chData[idx].z,
		myData->save10ms_msg[msg][bd][ch].chData[idx].temp[0][0],
		myData->save10ms_msg[msg][bd][ch].chData[idx].temp[0][1],
		myData->save10ms_msg[msg][bd][ch].chData[idx].temp[0][2],
		myData->save10ms_msg[msg][bd][ch].chData[idx].temp[0][3],
		(int)myData->save10ms_msg[msg][bd][ch].chData[idx].reservedCmd,
		myData->save10ms_msg[msg][bd][ch].chData[idx].gotoCycleCount,
		myData->save10ms_msg[msg][bd][ch].chData[idx].totalCycle,
		myData->save10ms_msg[msg][bd][ch].chData[idx].currentCycle,
		myData->save10ms_msg[msg][bd][ch].chData[idx].avgV,
		myData->save10ms_msg[msg][bd][ch].chData[idx].avgI,
		myData->save10ms_msg[msg][bd][ch].chData[idx].cvTime_day,
		myData->save10ms_msg[msg][bd][ch].chData[idx].cvTime,
		myData->save10ms_msg[msg][bd][ch].chData[idx].realDate,
		myData->save10ms_msg[msg][bd][ch].chData[idx].realClock,
		myData->save10ms_msg[msg][bd][ch].chData[idx].IntegralAmpareHour,
		myData->save10ms_msg[msg][bd][ch].chData[idx].IntegralWattHour,
		myData->save10ms_msg[msg][bd][ch].chData[idx].ccTime_day,
		myData->save10ms_msg[msg][bd][ch].chData[idx].ccTime,
		myData->save10ms_msg[msg][bd][ch].chData[idx].charge_cc_ampare_hour,
		myData->save10ms_msg[msg][bd][ch].chData[idx].charge_cv_ampare_hour,
		myData->save10ms_msg[msg][bd][ch].chData[idx].discharge_cc_ampare_hour,
		myData->save10ms_msg[msg][bd][ch].chData[idx].discharge_cv_ampare_hour,
		myData->save10ms_msg[msg][bd][ch].chData[idx].startVoltage,
		myData->save10ms_msg[msg][bd][ch].chData[idx].step_count,
		myData->save10ms_msg[msg][bd][ch].chData[idx].maxVoltage,
		myData->save10ms_msg[msg][bd][ch].chData[idx].minVoltage);
	fflush(fp);
#endif
	fprintf(fp,"\n");
	fflush(fp);
	}
	fclose(fp);

	// 130226 oys w : AuxDataSave
	//20171205 sch modify
#if CH_AUX_DATA == 0
	if(myPs->config.AuxData_saveFlag == 0){
		if(installedTemp > 0)
		{			
			memset(cmd, 0, sizeof(cmd));
#if DATA_SAVE_VER == 1
			sprintf(cmd,
			"/root/cycler_data/resultData/ch%02d/ch%02d_SaveData%04d_auxT.csv",
		   	p_ch+1, p_ch+1, (int)myPs->resultData[p_ch].fileIndex);
#else
			sprintf(cmd,
			"/root/cycler_data/resultData/ch%02d/ch%02d_SaveData%02d_auxT.csv",
		   	p_ch+1, p_ch+1, (int)myPs->resultData[p_ch].fileIndex);
#endif
		
			fp = fopen(cmd, "a+");
		
			if(fp == NULL || fp < 0) {
				userlog(DEBUG_LOG, psName, "Save_ResultData_2 error -10\n");
				return -10;
			}
			for(idx = 0; idx < msgCount; idx++){
				if(myData->save10ms_msg[msg][bd][ch].chData[idx].resultIndex == 0)
					continue;
				fprintf(fp, "%ld",
				myData->save10ms_msg[msg][bd][ch].chData[idx].resultIndex);
			
				for (i=0; i < installedTemp; i++)
				{
					fprintf(fp, ",%d,%ld",
					(int)myData->save_msg[1].auxData[idx][i].auxChNo,
					myData->save_msg[1].auxData[idx][i].val);
				}
				fprintf(fp, "\n");
				fflush(fp);
			}
			fclose(fp);
		}
		if(installedAuxV > 0)
		{			
			memset(cmd, 0, sizeof(cmd));
	
#if DATA_SAVE_VER == 1
			sprintf(cmd,
			"/root/cycler_data/resultData/ch%02d/ch%02d_SaveData%04d_auxV.csv",
		   	p_ch+1, p_ch+1, (int)myPs->resultData[p_ch].fileIndex);
#else
			sprintf(cmd,
			"/root/cycler_data/resultData/ch%02d/ch%02d_SaveData%02d_auxV.csv",
		   	p_ch+1, p_ch+1, (int)myPs->resultData[p_ch].fileIndex);
#endif
		
			fp = fopen(cmd, "a+");
		
			if(fp == NULL || fp < 0) {
				userlog(DEBUG_LOG, psName, "Save_ResultData_2 error -20\n");
				return -20;
			}
			for(idx = 0; idx < msgCount; idx++){
				if(myData->save10ms_msg[msg][bd][ch].chData[idx].resultIndex == 0)
					continue;
				fprintf(fp, "%ld",
				myData->save10ms_msg[msg][bd][ch].chData[idx].resultIndex);
			
				for(i=MAX_AUX_TEMP_DATA; i<installedAuxV+MAX_AUX_TEMP_DATA; i++)
				{
					fprintf(fp, ",%d,%ld",
					(int)myData->save_msg[1].auxData[idx][i].auxChNo,
					myData->save_msg[1].auxData[idx][i].val);
				}
				fprintf(fp, "\n");
				fflush(fp);
			}
			fclose(fp);
		}
	}else if(myPs->config.AuxData_saveFlag == 1){
		for(i = 0; i < installedTemp; i++){
			if(p_ch+1 == myData->auxSetData[i].chNo){
					aux_cnt++;
			}
		}
		if((installedTemp > 0) && (aux_cnt > 0))
		{			
			memset(cmd, 0, sizeof(cmd));
#if DATA_SAVE_VER == 1
			sprintf(cmd,
			"/root/cycler_data/resultData/ch%02d/ch%02d_SaveData%04d_auxT.csv",
		   	p_ch+1, p_ch+1, (int)myPs->resultData[p_ch].fileIndex);
#else
			sprintf(cmd,
			"/root/cycler_data/resultData/ch%02d/ch%02d_SaveData%02d_auxT.csv",
		   	p_ch+1, p_ch+1, (int)myPs->resultData[p_ch].fileIndex);
#endif
		
			fp = fopen(cmd, "a+");
		
			if(fp == NULL || fp < 0) {
				userlog(DEBUG_LOG, psName, "Save_ResultData_2 error -10\n");
				return -10;
			}
			for(idx = 0; idx < msgCount; idx++){
				if(myData->save10ms_msg[msg][bd][ch].chData[idx].resultIndex == 0)
					continue;
				fprintf(fp, "%ld",
				myData->save10ms_msg[msg][bd][ch].chData[idx].resultIndex);
			
				for (i=0; i < installedTemp; i++)
				{
					if(p_ch+1 == myData->auxSetData[i].chNo)
					{
						fprintf(fp, ",%d,%ld",
						(int)myData->save_msg[1].auxData[idx][i].auxChNo,
						myData->save_msg[1].auxData[idx][i].val);
					}
				}
				fprintf(fp, "\n");
				fflush(fp);
			}
			fclose(fp);
			aux_cnt = 0;
		}
		for (i = installedTemp; i < installedAuxV+installedTemp; i++)
		{
			if(p_ch+1 == myData->auxSetData[i].chNo){
					aux_cnt++;
			}
		}
		if((installedAuxV > 0) && (aux_cnt > 0))
		{			
			memset(cmd, 0, sizeof(cmd));
	
#if DATA_SAVE_VER == 1
			sprintf(cmd,
			"/root/cycler_data/resultData/ch%02d/ch%02d_SaveData%04d_auxV.csv",
		   	p_ch+1, p_ch+1, (int)myPs->resultData[p_ch].fileIndex);
#else
			sprintf(cmd,
			"/root/cycler_data/resultData/ch%02d/ch%02d_SaveData%02d_auxV.csv",
		   	p_ch+1, p_ch+1, (int)myPs->resultData[p_ch].fileIndex);
#endif
		
			fp = fopen(cmd, "a+");
		
			if(fp == NULL || fp < 0) {
				userlog(DEBUG_LOG, psName, "Save_ResultData_2 error -20\n");
				return -20;
			}
			for(idx = 0; idx < msgCount; idx++){
				if(myData->save10ms_msg[msg][bd][ch].chData[idx].resultIndex == 0)
					continue;
				fprintf(fp, "%ld",
				myData->save10ms_msg[msg][bd][ch].chData[idx].resultIndex);
			
				for(i=MAX_AUX_TEMP_DATA; i<installedAuxV+MAX_AUX_TEMP_DATA; i++)
				{
						auxNo = i - MAX_AUX_TEMP_DATA + installedTemp;
					if(p_ch+1 == myData->auxSetData[auxNo].chNo)
					{
						fprintf(fp, ",%d,%ld",
						(int)myData->save_msg[1].auxData[idx][i].auxChNo,
						myData->save_msg[1].auxData[idx][i].val);
					}
				}
				fprintf(fp, "\n");
				fflush(fp);
			}
			fclose(fp);
			aux_cnt = 0;
		}
	}
#endif

	for(idx = 0; idx < msgCount; idx++){
	if(myData->save10ms_msg[msg][bd][ch].chData[idx].select == SAVE_FLAG_SAVING_END) {
		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd, "/root/cycler_data/resultData/ch%02d/ch%02d_SaveEndData.csv",
			p_ch+1, p_ch+1);
		fp = fopen(cmd, "a+");
		if(fp == NULL || fp < 0) {
			userlog(DEBUG_LOG, psName, "Save_ResultData_2 error -2\n");
			return -3;
		}
#ifdef __LG_VER1__
	#if NETWORK_VERSION >= 4101
	fprintf(fp,
		"%ld,%d,%d,%d,%d,%d,%d,%d,%ld,%ld,%ld,%ld,%ld,%lu,%lu,%ld,%ld,%d,%lu,%lu,%ld,%ld",
		myData->save10ms_msg[msg][bd][ch].chData[idx].resultIndex,
		(int)myData->save10ms_msg[msg][bd][ch].chData[idx].state,
		(int)myData->save10ms_msg[msg][bd][ch].chData[idx].type,
		(int)myData->save10ms_msg[msg][bd][ch].chData[idx].mode,
		(int)myData->save10ms_msg[msg][bd][ch].chData[idx].select,
		(int)myData->save10ms_msg[msg][bd][ch].chData[idx].code,
		(int)myData->save10ms_msg[msg][bd][ch].chData[idx].grade,
		(int)myData->save10ms_msg[msg][bd][ch].chData[idx].stepNo,
		myData->save10ms_msg[msg][bd][ch].chData[idx].Vsens,
		myData->save10ms_msg[msg][bd][ch].chData[idx].Isens,
		myData->save10ms_msg[msg][bd][ch].chData[idx].capacity,
		myData->save10ms_msg[msg][bd][ch].chData[idx].watt,
		myData->save10ms_msg[msg][bd][ch].chData[idx].wattHour,
		myData->save10ms_msg[msg][bd][ch].chData[idx].runTime,
		myData->save10ms_msg[msg][bd][ch].chData[idx].totalRunTime,
		myData->save10ms_msg[msg][bd][ch].chData[idx].z,
		myData->save10ms_msg[msg][bd][ch].chData[idx].temp,
		(int)myData->save10ms_msg[msg][bd][ch].chData[idx].reservedCmd,
		myData->save10ms_msg[msg][bd][ch].chData[idx].totalCycle,
		myData->save10ms_msg[msg][bd][ch].chData[idx].currentCycle,
		myData->save10ms_msg[msg][bd][ch].chData[idx].avgV,
		myData->save10ms_msg[msg][bd][ch].chData[idx].avgI);
	fflush(fp);
//22
	#endif
	#if NETWORK_VERSION >= 4102
		#if VENDER != 2 //NOT SDI
	fprintf(fp, ",%ld,%ld,%ld,%ld,%ld,%ld,%ld",
		myData->save10ms_msg[msg][bd][ch].chData[idx].IntegralAmpareHour,
		myData->save10ms_msg[msg][bd][ch].chData[idx].IntegralWattHour,
		myData->save10ms_msg[msg][bd][ch].chData[idx].ChargeAmpareHour,
		myData->save10ms_msg[msg][bd][ch].chData[idx].ChargeWattHour,
		myData->save10ms_msg[msg][bd][ch].chData[idx].DischargeAmpareHour,
		myData->save10ms_msg[msg][bd][ch].chData[idx].DischargeWattHour,
		myData->save10ms_msg[msg][bd][ch].chData[idx].cvTime);
	fflush(fp);
//29
			#if PROGRAM_VERSION1 == 0
				#if PROGRAM_VERSION2 >= 1
	fprintf(fp, ",%ld,%d,%d,%ld",
		myData->save10ms_msg[msg][bd][ch].chData[idx].Farad,	
		(int)myData->save10ms_msg[msg][bd][ch].chData[idx].totalRunTime_carry,
		myData->save10ms_msg[msg][bd][ch].chData[idx].cycleNo,
		myData->save10ms_msg[msg][bd][ch].chData[idx].temp1);
	fflush(fp);
//33
				#endif
				#if EDLC_TYPE == 1
	//20160229 khk add start	
	fprintf(fp, ",%ld,%ld,%ld,%ld",
		myData->save10ms_msg[msg][bd][ch].chData[idx].c_v1,	
		myData->save10ms_msg[msg][bd][ch].chData[idx].c_v2,
		myData->save10ms_msg[msg][bd][ch].chData[idx].c_t1,
		myData->save10ms_msg[msg][bd][ch].chData[idx].c_t2);
	fflush(fp);
//37
				#endif
	//20160229 khk add end	
	//20170105 oys add start
				#if PROGRAM_VERSION2 >= 2
	fprintf(fp, ",%ld,%ld,%ld,%ld",
		myData->save10ms_msg[msg][bd][ch].chData[idx].chargeCCAh,	
		myData->save10ms_msg[msg][bd][ch].chData[idx].chargeCVAh,
		myData->save10ms_msg[msg][bd][ch].chData[idx].dischargeCCAh,
		myData->save10ms_msg[msg][bd][ch].chData[idx].dischargeCVAh);
	fflush(fp);
				#endif
	//20170105 oys add end
			#endif
			#if PROGRAM_VERSION1 > 0
	fprintf(fp, ",%ld,%d,%ld,%d,%ld,%ld,%ld,%ld,%ld,%ld",
		myData->save10ms_msg[msg][bd][ch].chData[idx].Farad,	
		(int)myData->save10ms_msg[msg][bd][ch].chData[idx].totalRunTime_carry,
		myData->save10ms_msg[msg][bd][ch].chData[idx].temp1,
		myData->save10ms_msg[msg][bd][ch].chData[idx].cycleNo,
		myData->save10ms_msg[msg][bd][ch].chData[idx].startVoltage,
		myData->save10ms_msg[msg][bd][ch].chData[idx].maxVoltage,
		myData->save10ms_msg[msg][bd][ch].chData[idx].minVoltage,
		myData->save10ms_msg[msg][bd][ch].chData[idx].startTemp,
		myData->save10ms_msg[msg][bd][ch].chData[idx].maxTemp,
		myData->save10ms_msg[msg][bd][ch].chData[idx].minTemp);
	fflush(fp);
//39
			#endif
		#endif
		#if REAL_TIME == 1
	fprintf(fp, ",%ld,%ld",
		myData->save10ms_msg[msg][bd][ch].chData[idx].realDate,
		myData->save10ms_msg[msg][bd][ch].chData[idx].realClock);
	fflush(fp);
//+2
		#endif
		#ifdef _CH_CHAMBER_DATA 
		fprintf(fp, ",%ld",
			myData->save10ms_msg[msg][bd][ch].chData[idx].Chamber_Temp);
		fflush(fp);
		#endif
	#endif
#endif
#ifdef __SDI_MES_VER4__
	fprintf(fp,
		"%ld,%d,%d,%d,%d,%d,%d,%d,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%lu,%lu,%lu,%lu,%ld,%ld,%ld,%ld,%ld,%d,%d,%lu,%lu,%ld,%ld,%lu,%lu,%ld,%ld,%ld,%ld,%lu,%lu,%ld,%ld,%ld,%ld,%ld,%lu,%ld,%ld",
		myData->save10ms_msg[msg][bd][ch].chData[idx].resultIndex,
		(int)myData->save10ms_msg[msg][bd][ch].chData[idx].state,
		(int)myData->save10ms_msg[msg][bd][ch].chData[idx].type,
		(int)myData->save10ms_msg[msg][bd][ch].chData[idx].mode,
		(int)myData->save10ms_msg[msg][bd][ch].chData[idx].select,
		(int)myData->save10ms_msg[msg][bd][ch].chData[idx].cvFlag,
		(int)myData->save10ms_msg[msg][bd][ch].chData[idx].code,
		(int)myData->save10ms_msg[msg][bd][ch].chData[idx].stepNo,
		myData->save10ms_msg[msg][bd][ch].chData[idx].Vsens,
		myData->save10ms_msg[msg][bd][ch].chData[idx].Isens,
		myData->save10ms_msg[msg][bd][ch].chData[idx].ChargeAmpareHour,
		myData->save10ms_msg[msg][bd][ch].chData[idx].DischargeAmpareHour,
		myData->save10ms_msg[msg][bd][ch].chData[idx].ChargeWatt,
		myData->save10ms_msg[msg][bd][ch].chData[idx].DischargeWatt,
		myData->save10ms_msg[msg][bd][ch].chData[idx].ChargeWattHour,
		myData->save10ms_msg[msg][bd][ch].chData[idx].DischargeWattHour,
		myData->save10ms_msg[msg][bd][ch].chData[idx].runTime_day,
		myData->save10ms_msg[msg][bd][ch].chData[idx].runTime,
		myData->save10ms_msg[msg][bd][ch].chData[idx].totalRunTime_carry,
		myData->save10ms_msg[msg][bd][ch].chData[idx].totalRunTime,
		myData->save10ms_msg[msg][bd][ch].chData[idx].z,
		myData->save10ms_msg[msg][bd][ch].chData[idx].temp[0][0],
		myData->save10ms_msg[msg][bd][ch].chData[idx].temp[0][1],
		myData->save10ms_msg[msg][bd][ch].chData[idx].temp[0][2],
		myData->save10ms_msg[msg][bd][ch].chData[idx].temp[0][3],
		(int)myData->save10ms_msg[msg][bd][ch].chData[idx].reservedCmd,
		myData->save10ms_msg[msg][bd][ch].chData[idx].gotoCycleCount,
		myData->save10ms_msg[msg][bd][ch].chData[idx].totalCycle,
		myData->save10ms_msg[msg][bd][ch].chData[idx].currentCycle,
		myData->save10ms_msg[msg][bd][ch].chData[idx].avgV,
		myData->save10ms_msg[msg][bd][ch].chData[idx].avgI,
		myData->save10ms_msg[msg][bd][ch].chData[idx].cvTime_day,
		myData->save10ms_msg[msg][bd][ch].chData[idx].cvTime,
		myData->save10ms_msg[msg][bd][ch].chData[idx].realDate,
		myData->save10ms_msg[msg][bd][ch].chData[idx].realClock,
		myData->save10ms_msg[msg][bd][ch].chData[idx].IntegralAmpareHour,
		myData->save10ms_msg[msg][bd][ch].chData[idx].IntegralWattHour,
		myData->save10ms_msg[msg][bd][ch].chData[idx].ccTime_day,
		myData->save10ms_msg[msg][bd][ch].chData[idx].ccTime,
		myData->save10ms_msg[msg][bd][ch].chData[idx].charge_cc_ampare_hour,
		myData->save10ms_msg[msg][bd][ch].chData[idx].charge_cv_ampare_hour,
		myData->save10ms_msg[msg][bd][ch].chData[idx].discharge_cc_ampare_hour,
		myData->save10ms_msg[msg][bd][ch].chData[idx].discharge_cv_ampare_hour,
		myData->save10ms_msg[msg][bd][ch].chData[idx].startVoltage,
		myData->save10ms_msg[msg][bd][ch].chData[idx].step_count,
		myData->save10ms_msg[msg][bd][ch].chData[idx].maxVoltage,
		myData->save10ms_msg[msg][bd][ch].chData[idx].minVoltage);
	fflush(fp);
#endif
	fprintf(fp, "\n");
	fflush(fp);
	fclose(fp);
		// 130226 oys w : AuxDataSave
		//20171205 sch modify
#if CH_AUX_DATA == 0
		if(myPs->config.AuxData_saveFlag == 0){
			if(installedTemp > 0)
			{			
				memset(cmd, 0, sizeof(cmd));
				sprintf(cmd,
				"/root/cycler_data/resultData/ch%02d/ch%02d_SaveEndData_auxT.csv",p_ch+1, p_ch+1);

				fp = fopen(cmd, "a+");
					if(fp == NULL || fp < 0) {
					userlog(DEBUG_LOG, psName, "Save_ResultData_2 error -30\n");
					return -30;
				}
					fprintf(fp, "%ld",
					myData->save10ms_msg[msg][bd][ch].chData[idx].resultIndex);
			
				for (i=0; i < installedTemp; i++)
				{
					fprintf(fp, ",%d,%ld",
					(int)myData->save_msg[1].auxData[idx][i].auxChNo,
					myData->save_msg[1].auxData[idx][i].val);
				}
				fprintf(fp, "\n");
				fflush(fp);
				fclose(fp);
			}
			if(installedAuxV > 0)
			{			
				memset(cmd, 0, sizeof(cmd));
				sprintf(cmd, 
				"/root/cycler_data/resultData/ch%02d/ch%02d_SaveEndData_auxV.csv",
				p_ch+1, p_ch+1);
			
				fp = fopen(cmd, "a+");
			
				if(fp == NULL || fp < 0) {
					userlog(DEBUG_LOG, psName, "Save_ResultData_2 error -40\n");
					return -40;
				}

				fprintf(fp, "%ld",
					myData->save10ms_msg[msg][bd][ch].chData[idx].resultIndex);	
				for(i=MAX_AUX_TEMP_DATA; i<installedAuxV+MAX_AUX_TEMP_DATA; i++)
				{
					fprintf(fp, ",%d,%ld",
					(int)myData->save_msg[1].auxData[idx][i].auxChNo,
					myData->save_msg[1].auxData[idx][i].val);
				}
				fprintf(fp, "\n");
				fflush(fp);
				fclose(fp);
			}
		}else if(myPs->config.AuxData_saveFlag == 1){
			for(i = 0; i < installedTemp; i++){
				if(p_ch+1 == myData->auxSetData[i].chNo){
					aux_cnt++;
				}
			}
			if((installedTemp > 0) && (aux_cnt > 0))
			{			
				memset(cmd, 0, sizeof(cmd));
				sprintf(cmd,
				"/root/cycler_data/resultData/ch%02d/ch%02d_SaveEndData_auxT.csv",p_ch+1, p_ch+1);

				fp = fopen(cmd, "a+");
					if(fp == NULL || fp < 0) {
					userlog(DEBUG_LOG, psName, "Save_ResultData_2 error -30\n");
					return -30;
				}
					fprintf(fp, "%ld",
					myData->save10ms_msg[msg][bd][ch].chData[idx].resultIndex);
			
				for (i=0; i < installedTemp; i++)
				{
				if(p_ch+1 == myData->auxSetData[i].chNo)
				{
					fprintf(fp, ",%d,%ld",
					(int)myData->save_msg[1].auxData[idx][i].auxChNo,
					myData->save_msg[1].auxData[idx][i].val);
				}
				}
				fprintf(fp, "\n");
				fflush(fp);
				fclose(fp);
				aux_cnt = 0;
			}
			for(i = installedTemp; i<installedAuxV+installedTemp; i++)
			{
				if(p_ch+1 == myData->auxSetData[i].chNo){
					aux_cnt++;
				}
			}
			if((installedAuxV > 0) && (aux_cnt > 0))
			{			
				memset(cmd, 0, sizeof(cmd));
				sprintf(cmd, 
				"/root/cycler_data/resultData/ch%02d/ch%02d_SaveEndData_auxV.csv",
				p_ch+1, p_ch+1);
			
				fp = fopen(cmd, "a+");
			
				if(fp == NULL || fp < 0) {
					userlog(DEBUG_LOG, psName, "Save_ResultData_2 error -40\n");
					return -40;
				}

				fprintf(fp, "%ld",
					myData->save10ms_msg[msg][bd][ch].chData[idx].resultIndex);	
				for(i=MAX_AUX_TEMP_DATA; i<installedAuxV+MAX_AUX_TEMP_DATA; i++)
				{
					auxNo = i - MAX_AUX_TEMP_DATA + installedTemp;
					if(p_ch+1 == myData->auxSetData[auxNo].chNo)
					{
						fprintf(fp, ",%d,%ld",
						(int)myData->save_msg[1].auxData[idx][i].auxChNo,
						myData->save_msg[1].auxData[idx][i].val);
					}
				}
				fprintf(fp, "\n");
				fflush(fp);
				fclose(fp);
				aux_cnt = 0;
			}
		}
#endif
	}
	}	
	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd,
		"/root/cycler_data/resultData/ch%02d/savingFileIndex_last.csv", p_ch+1);
	fp = fopen(cmd, "w+");
	if(fp == NULL || fp < 0) {
		userlog(DEBUG_LOG, psName, "Save_ResultData_2 error -3\n");
		return -4;
	}

	fprintf(fp, "fileIndex %d, ", (int)myPs->resultData[p_ch].fileIndex);
	fprintf(fp, "resultIndex %ld, ",
		myData->save10ms_msg[msg][bd][ch].chData[msgCount-1].resultIndex);
	fprintf(fp, "open_year %d, ", (int)myPs->resultData[p_ch].open_year);
	fprintf(fp, "open_month %d, ", (int)myPs->resultData[p_ch].open_month);
	fprintf(fp, "open_day %d\n", (int)myPs->resultData[p_ch].open_day);
	
	myPs->resultData[p_ch].resultIndex
		= myData->save10ms_msg[msg][bd][ch].chData[msgCount-1].resultIndex;

	fflush(fp);
	fclose(fp);
	return count;
}

//20200427 rewrite
int	Read_User_Pattern_NoUserData(const int realCh, const int stepNo, const int select)
{
    int tmp, i, length, bd, ch, type;
	char temp[20], buf[8], fileName[128], cmd[128];
	long maxI, data, rangeI, maxP, cpRefV;
    double refI;
	FILE *fp;

	bd = myData->CellArray1[realCh].bd;
	ch = myData->CellArray1[realCh].ch;
	
	// General
	if(select == 0) {
		memset(fileName, 0x00, sizeof(fileName));
		sprintf(fileName, "/root/cycler_data/pattern/ch%02d/userPattern_step_%03d.csv", realCh+1, stepNo);
		if((fp = fopen(fileName, "r")) == NULL) {
			userlog(DEBUG_LOG, psName, "ch %d : step : %d userPattern file read fail\n", realCh, stepNo);
			return -1;
		}

	    tmp = fscanf(fp, "%s", temp); 	//Date
		tmp = fscanf(fp, "%s", temp);   // :
		tmp = fscanf(fp, "%s", temp);   // yy/mm/dd
		tmp = fscanf(fp, "%s", temp); 	// Time
		tmp = fscanf(fp, "%s", temp);   // :
		tmp = fscanf(fp, "%s", temp);   // hh:mm:ss 

		tmp = fscanf(fp, "%s", temp);   // stepNo
		tmp = fscanf(fp, "%s", temp);   // :

		memset(buf, 0x00, sizeof buf);
	    tmp = fscanf(fp, "%s", buf);
		if(atoi(buf) != stepNo){
			userlog(DEBUG_LOG, psName, "ch : %d step : %d userPattern stepNo missmatch\n", realCh, stepNo);
			return -1;
		}
		myData->mData.testCond[bd][ch].userPattern.stepNo = atol(buf);

		tmp = fscanf(fp, "%s", temp);   // length
		tmp = fscanf(fp, "%s", temp);   // :
		memset(buf, 0x00, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		length = atoi(buf);
		if(length > MAX_USER_PATTERN_DATA ){
			userlog(DEBUG_LOG, psName, "ch : %d step : %d userPattern length[%d] error\n", realCh, stepNo, length);
			return -1;
		}
		myData->mData.testCond[bd][ch].userPattern.length = (long)length;

		tmp = fscanf(fp, "%s", temp);   // type
		tmp = fscanf(fp, "%s", temp);   // :
		memset(buf, 0x00, sizeof buf);
	    tmp = fscanf(fp, "%s", buf);
		type = atol(buf);
	    myData->mData.testCond[bd][ch].userPattern.type = (long)type;
		switch((unsigned char)type){
			case PS_CURRENT:
			case PS_C_RATE:
				myData->mData.testCond[bd][ch].step[stepNo].mode = CC;
				break;
			case PS_WATT:
    			myData->mData.testCond[bd][ch].step[stepNo].mode = CP;
				break;
		}

		tmp = fscanf(fp, "%s", temp);   // stepTime
		tmp = fscanf(fp, "%s", temp);   // :
		tmp = fscanf(fp, "%s", temp);   // data
	}
	// FTP
	if(select == 1) {
		memset(fileName, 0x00, sizeof(fileName));
		memset(cmd, 0x00, sizeof(cmd));

		sprintf(fileName, "/root/cycler_data/pattern/ch%02d/GUI_userPattern_step_%03d.csv", realCh+1, stepNo);
		if((fp = fopen(fileName, "r")) == NULL) {
			userlog(DEBUG_LOG, psName, "ch %d : step : %d GUI_userPattern file read fail\n", realCh+1, stepNo);
			return -1;
		}

		tmp = fscanf(fp, "%s", temp);   // stepNo
		tmp = fscanf(fp, "%s", temp);   // :
		memset(buf, 0x00, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		if(atoi(buf) != stepNo){
			userlog(DEBUG_LOG, psName, "ch : %d step : %d GUI_userPattern stepNo missmatch\n", realCh+1, stepNo);
			return -1;
		}
		myData->mData.testCond[bd][ch].userPattern.stepNo = atol(buf);
		
		tmp = fscanf(fp, "%s", temp);   // length
		tmp = fscanf(fp, "%s", temp);   // :
		memset(buf, 0x00, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		length = atoi(buf);
		if(length > MAX_USER_PATTERN_DATA ){
			userlog(DEBUG_LOG, psName, "ch : %d step : %d GUI_userPattern length[%d] error\n", realCh, stepNo, length);
			return -1;
		}
		myData->mData.testCond[bd][ch].userPattern.length = (long)length;

		tmp = fscanf(fp, "%s", temp);   // type
		tmp = fscanf(fp, "%s", temp);   // :
		memset(buf, 0x00, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		type = atol(buf);
		myData->mData.testCond[bd][ch].userPattern.type = (long)type;
		switch((unsigned char)type){
			case PS_CURRENT:
			case PS_C_RATE:
				myData->mData.testCond[bd][ch].step[stepNo].mode = CC;
				break;
			case PS_WATT:
    			myData->mData.testCond[bd][ch].step[stepNo].mode = CP;
				break;
		}
	}
	maxI = maxP = 0;

	for(i = 0; i < length; i++){
		memset(buf, 0x00, sizeof buf);
	    tmp = fscanf(fp, "%s", buf);
	    myData->mData.testCond[bd][ch].userPattern.data[i].time = atol(buf);
		
		tmp = fscanf(fp, "%s", temp);   // :

		memset(buf, 0x00, sizeof buf);
	    tmp = fscanf(fp, "%s", buf);
	    data = atol(buf);
	    myData->mData.testCond[bd][ch].userPattern.data[i].data = data;
		if(i == 0){
			if(data != 0){
				if(data > 0) myData->bData[bd].cData[ch].misc.cmdV_dir = CMD_V_PLUS;
				if(data < 0) myData->bData[bd].cData[ch].misc.cmdV_dir = CMD_V_MINUS;
			}
		}else if(i == 1){
	    	if(myData->mData.testCond[bd][ch].userPattern.data[0].data == 0){
				if(data != 0){
					if(data > 0) myData->bData[bd].cData[ch].misc.cmdV_dir = CMD_V_PLUS;
					if(data < 0) myData->bData[bd].cData[ch].misc.cmdV_dir = CMD_V_MINUS;
				}else{
					myData->bData[bd].cData[ch].misc.cmdV_dir = CMD_V_PLUS;
				}
			}
		}
		switch((unsigned char)type){
			case PS_CURRENT:
				if(labs(maxI) < labs(data)) maxI = data;
				break;
			case PS_WATT:
				if(labs(maxP) < labs(data)) maxP = data;
				break;
			case PS_C_RATE:
				if(myData->bData[bd].cData[ch].misc.standardC_Flag == P1) {
					data = myData->bData[bd].cData[ch]
						.misc.standardC * ((float)data / 1000); //non fix
				}
				if(labs(maxI) < labs(data)) maxI = data;
				break;
		}
	}
	rangeI = 0;
	switch((unsigned char)type){
		case PS_CURRENT:
		case PS_C_RATE:
			for(i = MAX_RANGE; i > 0; i--){
				if(labs(maxI) <= myData->mData.config.maxCurrent[i-1]){
		    		rangeI = i-1;
					break;
				}
			}
			if((rangeI + 1) > (int)myData->mData.config.rangeI) {
				rangeI = (int)myData->mData.config.rangeI -1;
			}
    		myData->mData.testCond[bd][ch].step[stepNo].rangeI = (unsigned char)rangeI;
    		myData->mData.testCond[bd][ch].step[stepNo].rangeV = (unsigned char)(RANGE1-1);
			break;
		case PS_WATT:
			//110427 kji pattern cp range select 
    		if(myData->mData.testCond[bd][ch].step[stepNo].endV_L > 0
    			|| myData->mData.testCond[bd][ch].step[stepNo].refV_L > 0){
    			if(myData->mData.testCond[bd][ch].step[stepNo].endV_L >
    				myData->mData.testCond[bd][ch].step[stepNo].refV_L) {
    				cpRefV = myData->mData.testCond[bd][ch].step[stepNo].endV_L;
				} else {
    				cpRefV = myData->mData.testCond[bd][ch].step[stepNo].refV_L;
					if(cpRefV < 500000)
						cpRefV = 500000;
				}
				refI = (labs(maxP) / (double)cpRefV);
				refI *= 1000000000.0;
				if((long)refI <= myData->mData.config.maxCurrent[3]) {
				    rangeI = RANGE4 - 1;
				} else if((long)refI <= myData->mData.config.maxCurrent[2]) {
				    rangeI = RANGE3 - 1;
				} else if((long)refI <= myData->mData.config.maxCurrent[1]) {
				    rangeI = RANGE2 - 1;
				} else {
				    rangeI = RANGE1 - 1;
				}
			}
    		myData->mData.testCond[bd][ch].step[stepNo].rangeI = (unsigned char)rangeI;
    		myData->mData.testCond[bd][ch].step[stepNo].rangeV = (unsigned char)(RANGE1-1);
			break;
	}
    fclose(fp);

	return 0;
}
