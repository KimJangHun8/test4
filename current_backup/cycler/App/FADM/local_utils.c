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
extern volatile S_FADM *myPs;
extern char	psName[PROCESS_NAME_SIZE];

void Init_SystemMemory(void)
{
	int i;
	
	for(i=0; i < MAX_SIGNAL; i++) {
		myPs->signal[i] = P0;
	}

	myPs->comm_buffer.use_flag = 0;
	myPs->misc.processPointer = (int)&myData;
}

int	Read_FADM_Config(void)
{
    int tmp;
	char temp[20], buf[6], fileName[128];
    FILE *fp;

	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/cycler_data/config/parameter/FADM_Config");
	// /root/cycler_data/config/parameter/FADM_Config
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "FADM_Config file read error\n");
		system("cp ../Config_backup/FADM_Config /root/cycler_data/config/parameter");
		userlog(DEBUG_LOG, psName, "FADM_Config file copy\n");
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
	//0:READ_T, 1:READ_V, 2:READ_I
    myPs->config.readType = (unsigned char)atoi(buf);

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
	//1:USE, 0:UNUSE
    myPs->config.useFlag = (unsigned char)atoi(buf);

    fclose(fp);
	return 0;
}

void CalPulseAD_Reference(int ch)
{
	int bd, i, type, index, rangeI;
	double AD_a, AD_b, val1, val2;

	bd = ch / myData->mData.config.chPerBd;
	ch = ch % myData->mData.config.chPerBd;

	for(type=0; type < 2; type++) {
		for(index=0; index < 3; index++) {
			val1 = 0.0;
			for(i=0; i < 20; i++) {
				val1 += (double)myPs->pulse_ad_org[bd][ch].ref[type][index][i];
			}
			myPs->pulse_ad_org[bd][ch].avg_ref[type][index]
				= (long)(val1 / 20.0);
		}
	}


	val1 = myData->mData.config.maxVoltage[0];
	rangeI = myPs->pulse_ad[bd][ch].rangeI;
//	val2 = (double)myData->mData.config.maxCurrent[0];
// 100426 고속 AD range 적용
	val2 = (double)myData->mData.config.maxCurrent[rangeI];

	type = 0;
	
	if(myData->mData.config.hwSpec == L_20V_50A_R2_1) {
		myPs->pulse_ad_org[bd][ch].avg_ref[type][0] *= 1.1978;
		myPs->pulse_ad_org[bd][ch].avg_ref[type][1] *= 1.1978;
		myPs->pulse_ad_org[bd][ch].avg_ref[type][2] *= 1.1978;
	}

	AD_a = (double)(val1 - 0.0)
		/ (double)(myPs->pulse_ad_org[bd][ch].avg_ref[type][0]
		- myPs->pulse_ad_org[bd][ch].avg_ref[type][2]);
	AD_b = val1
		- (double)myPs->pulse_ad_org[bd][ch].avg_ref[type][0] * AD_a;
	myPs->pulse_ad[bd][ch].ref_v_a = AD_a;
	myPs->pulse_ad[bd][ch].ref_v_b = AD_b;
	AD_a = (double)(0.0 - (-val1))
		/ (double)(myPs->pulse_ad_org[bd][ch].avg_ref[type][2]
		- myPs->pulse_ad_org[bd][ch].avg_ref[type][1]);
	AD_b = (double)(0.0)
		- (double)myPs->pulse_ad_org[bd][ch].avg_ref[type][2] * AD_a;
	myPs->pulse_ad[bd][ch].ref_v_a_N = AD_a;
	myPs->pulse_ad[bd][ch].ref_v_b_N = AD_b;
	type = 1;
	if(myData->mData.config.hwSpec == L_20V_50A_R2_1 ||
			myData->mData.config.hwSpec == L_5V_1A_R2) {
		myPs->pulse_ad_org[bd][ch].avg_ref[type][0] *= 1.0096;
		myPs->pulse_ad_org[bd][ch].avg_ref[type][1] *= 1.0096;
		myPs->pulse_ad_org[bd][ch].avg_ref[type][2] *= 1.0096;
	}
	AD_a = (double)(val2 - 0.0)
		/ (double)(myPs->pulse_ad_org[bd][ch].avg_ref[type][0]
		- myPs->pulse_ad_org[bd][ch].avg_ref[type][2]);
	AD_b = val2
		- (double)myPs->pulse_ad_org[bd][ch].avg_ref[type][0] * AD_a;
	myPs->pulse_ad[bd][ch].ref_i_a = AD_a;
	myPs->pulse_ad[bd][ch].ref_i_b = AD_b;

	AD_a = (double)(0.0 - (-val2))
		/ (double)(myPs->pulse_ad_org[bd][ch].avg_ref[type][2]
		- myPs->pulse_ad_org[bd][ch].avg_ref[type][1]);
	AD_b = (double)(0.0)
		- (double)myPs->pulse_ad_org[bd][ch].avg_ref[type][2] * AD_a;
	myPs->pulse_ad[bd][ch].ref_i_a_N = AD_a;
	myPs->pulse_ad[bd][ch].ref_i_b_N = AD_b;

	for(index=0; index < 3; index++) {
		type = 0;
		val1 = (double)myPs->pulse_ad_org[bd][ch].avg_ref[type][index];
		if(val1 >= 0.0) {
			myPs->pulse_ad[bd][ch].ref[type][index]
				= (long)(val1 * myPs->pulse_ad[bd][ch].ref_v_a
				+ myPs->pulse_ad[bd][ch].ref_v_b);
		} else {
			myPs->pulse_ad[bd][ch].ref[type][index]
				= (long)(val1 * myPs->pulse_ad[bd][ch].ref_v_a_N
				+ myPs->pulse_ad[bd][ch].ref_v_b_N);
		}

		type = 1;
		val1 = (double)myPs->pulse_ad_org[bd][ch].avg_ref[type][index];
		if(val1 >= 0.0) {
			myPs->pulse_ad[bd][ch].ref[type][index]
				= (long)(val1 * myPs->pulse_ad[bd][ch].ref_i_a
				+ myPs->pulse_ad[bd][ch].ref_v_b);
		} else {
			myPs->pulse_ad[bd][ch].ref[type][index]
				= (long)(val1 * myPs->pulse_ad[bd][ch].ref_i_a_N
				+ myPs->pulse_ad[bd][ch].ref_v_b_N);
		}
	}
}

void CalPulseAD_Value(int ch)
{
	int bd, i, type, val1;

	bd = ch / myData->mData.config.chPerBd;
	ch = ch % myData->mData.config.chPerBd;

	type = 0; 
	for(i=0; i < 300; i++) {
		val1 = (double)myPs->pulse_ad_org[bd][ch].value[type][i];
		if(myData->mData.config.hwSpec == L_20V_110A_R2) {
			if(val1 >= 0.0) {
				myPs->pulse_ad[bd][ch].value[type][i]
					= (long)((val1 * myPs->pulse_ad[bd][ch].ref_v_a
					+ myPs->pulse_ad[bd][ch].ref_v_b)* 3.336);
			} else {
				myPs->pulse_ad[bd][ch].value[type][i]
					= (long)((val1 * myPs->pulse_ad[bd][ch].ref_v_a_N
					+ myPs->pulse_ad[bd][ch].ref_v_b_N)*3.336);
			}
		} else {
			if(val1 >= 0.0) {
				myPs->pulse_ad[bd][ch].value[type][i]
					= (long)(val1 * myPs->pulse_ad[bd][ch].ref_v_a
					+ myPs->pulse_ad[bd][ch].ref_v_b);
			} else {
				myPs->pulse_ad[bd][ch].value[type][i]
					= (long)(val1 * myPs->pulse_ad[bd][ch].ref_v_a_N
					+ myPs->pulse_ad[bd][ch].ref_v_b_N);
			}
		}
	}


	type = 1; 
	for(i=0; i < 300; i++) {
		val1 = (double)myPs->pulse_ad_org[bd][ch].value[type][i];
		if(myData->mData.config.hwSpec == L_20V_110A_R2) {
			if(val1 >= 0.0) {
				myPs->pulse_ad[bd][ch].value[type][i]
					= (long)((val1 * myPs->pulse_ad[bd][ch].ref_i_a
					+ myPs->pulse_ad[bd][ch].ref_i_b) / 1.2);
			} else {
				myPs->pulse_ad[bd][ch].value[type][i]
					= (long)((val1 * myPs->pulse_ad[bd][ch].ref_i_a_N
					+ myPs->pulse_ad[bd][ch].ref_i_b_N) / 1.2);
			}
		}else {
			if(val1 >= 0.0) {
				myPs->pulse_ad[bd][ch].value[type][i]
					= (long)(val1 * myPs->pulse_ad[bd][ch].ref_i_a
					+ myPs->pulse_ad[bd][ch].ref_i_b) ;
			} else {
				myPs->pulse_ad[bd][ch].value[type][i]
					= (long)(val1 * myPs->pulse_ad[bd][ch].ref_i_a_N
					+ myPs->pulse_ad[bd][ch].ref_i_b_N);
			}
		}
	}
}
