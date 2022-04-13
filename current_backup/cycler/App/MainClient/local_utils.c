#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include "../../INC/datastore.h"
#include "userlog.h"
#include "message.h"
#include "common_utils.h"
#include "network.h"
#include "local_utils.h"

extern volatile S_SYSTEM_DATA *myData;
extern volatile S_MAIN_CLIENT *myPs;
//volatile S_CH_DATA *myCh; //add <--20150512 oys
extern char psName[PROCESS_NAME_SIZE];

int Initialize(void)
{
	if(Open_SystemMemory(0) < 0) return -1;
	
	myPs = &(myData->MainClient);
	
	Init_SystemMemory();
	
	if(Read_MainClient_Config() < 0) return -2;
	
	myData->AppControl.signal[myPs->misc.psSignal] = P1;
	return 0;
}

void Init_SystemMemory(void)
{
	int i;
	
	memset((char *)&psName[0], 0, PROCESS_NAME_SIZE);
	strcpy(psName, "Main");
    myPs->misc.psSignal = APP_SIG_MAIN_CLIENT_PROCESS;

	for(i=0; i < MAX_SIGNAL; i++) {
		myPs->signal[i] = P0;
	}
	
	memset((char *)&myPs->reply, 0x00, sizeof(S_MAIN_REPLY));
	myPs->pingTimer = 0;
//	myPs->netTimer = 0;
//110402 kji w
	myPs->netTimer = myData->mData.misc.timer_1sec;

	memset((char *)&myPs->rcvCmd, 0x00, sizeof(S_MAIN_RCV_COMMAND));
	memset((char *)&myPs->rcvPacket, 0x00, sizeof(S_MAIN_RCV_PACKET));
	
	myPs->misc.cmd_serial = 0;
	myPs->misc.processPointer = (int)&myData;
}

int	Read_MainClient_Config(void)
{
    int tmp;
	char temp[20], buf[12], fileName[128];
    FILE *fp;

	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/cycler_data/config/parameter/MainClient_Config");
	// /root/cycler_data/config/parameter/MainClient_Config
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "MainClient_Config file read error\n");
		system("cp ../Config_backup/MainClient_Config /root/cycler_data/config/parameter");
		userlog(DEBUG_LOG, psName, "MainClient_Config file copy\n");
    	if((fp = fopen(fileName, "r")) == NULL) {
			return -1;
		}
	}

    tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	memset(temp, 0x00, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	memcpy((char *)&myPs->config.ipAddr[0], (char *)&temp[0],
		sizeof(char)*16);
		
    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.sendPort = atoi(buf);

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.receivePort = atoi(buf);

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.networkPort = atoi(buf);
	
	#ifdef _EXTERNAL_CONTROL
    myPs->config.networkPort = 6001;
	#endif

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.replyTimeout = atoi(buf);

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.retryCount = atoi(buf);

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.netTimeout = atoi(buf);

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.pingTimeout = atoi(buf);

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
    myPs->config.send_monitor_data_interval = (unsigned long)atol(buf);

    tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myPs->config.send_save_data_interval = (unsigned long)atol(buf);

	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myPs->config.protocol_version = (unsigned int)atoi(buf);
    	
	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0x00, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	//1:pause - network fail, 2:run - network fail
	myPs->config.state_change = (unsigned int)atoi(buf);
	
    fclose(fp);
	return 0;
}

int	Write_MainClient_Config(void)
{
	char fileName[128];
    FILE *fp;

	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/cycler_data/config/parameter/MainClient_Config");
	// /root/cycler_data/config/parameter/MainClient_Config
    if((fp = fopen(fileName, "w")) == NULL) {
		userlog(DEBUG_LOG, psName, "MainClient_Config file write error\n");
		return -1;
	}
	#ifdef _EXTERNAL_CONTROL
    myPs->config.networkPort = 6001;
	#endif
	fprintf(fp, "ipAddress           :   %s\n", myPs->config.ipAddr);
    fprintf(fp, "sendPort            :   %d\n", myPs->config.sendPort);
	fprintf(fp, "receivePort         :   %d\n", myPs->config.receivePort);
	fprintf(fp, "networkPort         :   %d\n", myPs->config.networkPort);
	fprintf(fp, "replyTimeout        :   %d\n", myPs->config.replyTimeout);
	fprintf(fp, "retryCount          :   %d\n", myPs->config.retryCount);
	fprintf(fp, "netTimeout          :   %d\n", myPs->config.netTimeout);
	fprintf(fp, "pingTimeout         :   %d\n", myPs->config.pingTimeout);
	fprintf(fp, "CmdSendLog          :   %d\n", myPs->config.CmdSendLog);
	fprintf(fp, "CmdRcvLog           :   %d\n", myPs->config.CmdRcvLog);
	fprintf(fp, "CmdSendLog_Hex      :   %d\n", myPs->config.CmdSendLog_Hex);
	fprintf(fp, "CmdRcvLog_Hex       :   %d\n", myPs->config.CmdRcvLog_Hex);
	fprintf(fp, "CommCheckLog        :   %d\n", myPs->config.CommCheckLog);
	fprintf(fp, "monitorInterval     :   %ld\n",
		myPs->config.send_monitor_data_interval);
	fprintf(fp, "saveInterval        :   %ld\n",
		myPs->config.send_save_data_interval);
	fprintf(fp, "protocol_version    :   %d\n", myPs->config.protocol_version);
	fprintf(fp, "state_change        :   %d\n", myPs->config.state_change);
    	
    fclose(fp);
	return 0;
}

void CaliUpdateCh(int bd, int ch)
{
	int type, range;

	for(type=0; type < MAX_TYPE; type++) {
		for(range=0; range < MAX_RANGE; range++) {
			if(myData->cali.tmpData[bd][ch].caliFlag[type][range] == 0)
				continue;
			myData->cali.tmpData[bd][ch].caliFlag[type][range] = 0;
			memcpy((char *)&myData->cali.data[bd][ch].point[type][range],
				(char *)&myData->cali.tmpCond[bd][ch].point[type][range],
				sizeof(S_CALI_POINT));

			memcpy((char *)&myData->cali.data[bd][ch].set_ad[type][range][0],
				(char *)&myData->cali.tmpData[bd][ch].set_ad[type][range][0],
				sizeof(double)*MAX_CALI_POINT);
			memcpy((char *)&myData->cali.data[bd][ch].set_meter[type][range][0],
				(char *)&myData->cali.tmpData[bd][ch].set_meter[type][range][0],
				sizeof(double)*MAX_CALI_POINT);
			memcpy((char *)&myData->cali.data[bd][ch].check_ad[type][range][0],
				(char *)&myData->cali.tmpData[bd][ch].check_ad[type][range][0],
				sizeof(double)*MAX_CALI_POINT);
			memcpy((char *)&myData->cali.data[bd][ch]
				.check_meter[type][range][0],
				(char *)&myData->cali.tmpData[bd][ch]
				.check_meter[type][range][0],
				sizeof(double)*MAX_CALI_POINT);

			memcpy((char *)&myData->cali.data[bd][ch].DA_A[type][range][0],
				(char *)&myData->cali.tmpData[bd][ch].DA_A[type][range][0],
				sizeof(double)*(MAX_CALI_POINT-1));
			memcpy((char *)&myData->cali.data[bd][ch].DA_B[type][range][0],
				(char *)&myData->cali.tmpData[bd][ch].DA_B[type][range][0],
				sizeof(double)*(MAX_CALI_POINT-1));
			memcpy((char *)&myData->cali.data[bd][ch].AD_A[type][range][0],
				(char *)&myData->cali.tmpData[bd][ch].AD_A[type][range][0],
				sizeof(double)*(MAX_CALI_POINT-1));
			memcpy((char *)&myData->cali.data[bd][ch].AD_B[type][range][0],
				(char *)&myData->cali.tmpData[bd][ch].AD_B[type][range][0],
				sizeof(double)*(MAX_CALI_POINT-1));
			memcpy((char *)&myData->cali.data[bd][ch].AD_Ratio[type][range][0],
				(char *)&myData->cali.tmpData[bd][ch].AD_Ratio[type][range][0],
				sizeof(double)*2);
//FAD cali data	 20130708
			if(myData->mData.config.FadBdUse == P1){
				memcpy((char *)&myData->cali.data_fad[bd][ch].set_ad[type][range][0],
					(char *)&myData->cali.tmpData_fad[bd][ch].set_ad[type][range][0],
					sizeof(double)*MAX_CALI_POINT);
				memcpy((char *)&myData->cali.data_fad[bd][ch].check_ad[type][range][0],
					(char *)&myData->cali.tmpData_fad[bd][ch].check_ad[type][range][0],
					sizeof(double)*MAX_CALI_POINT);

				memcpy((char *)&myData->cali.data_fad[bd][ch].AD_A[type][range][0],
					(char *)&myData->cali.tmpData_fad[bd][ch].AD_A[type][range][0],
					sizeof(double)*(MAX_CALI_POINT-1));
				memcpy((char *)&myData->cali.data_fad[bd][ch].AD_B[type][range][0],
					(char *)&myData->cali.tmpData_fad[bd][ch].AD_B[type][range][0],
					sizeof(double)*(MAX_CALI_POINT-1));
			}
//I OffSet Cali 20160229 
			if(myData->mData.config.function[F_I_OFFSET_CALI] == P1){
				if(type ==0){
					for(range=0; range < MAX_RANGE; range++) {
						memcpy((char *)&myData->cali.data[bd][ch].point[type][range],
							(char *)&myData->cali.tmpCond[bd][ch].point[type][range],
							sizeof(S_CALI_POINT));
						memcpy((char *)&myData->cali.data_caliMeter2[bd][ch].set_ad[1][range][0],
								(char *)&myData->cali.tmpData_caliMeter2[bd][ch].set_ad[1][range][0],
								sizeof(double)*MAX_CALI_POINT);
						memcpy((char *)&myData->cali.data_caliMeter2[bd][ch].check_ad[1][range][0],
								(char *)&myData->cali.tmpData_caliMeter2[bd][ch].check_ad[1][range][0],
								sizeof(double)*MAX_CALI_POINT);
						memcpy((char *)&myData->cali.data_caliMeter2[bd][ch].set_meter[1][range][0],
								(char *)&myData->cali.tmpData_caliMeter2[bd][ch].set_meter[1][range][0],
								sizeof(double)*MAX_CALI_POINT);
						memcpy((char *)&myData->cali.data_caliMeter2[bd][ch].check_meter[1][range][0],
							(char *)&myData->cali.tmpData_caliMeter2[bd][ch].check_meter[1][range][0],
								sizeof(double)*MAX_CALI_POINT);

						memcpy((char *)&myData->cali.data_caliMeter2[bd][ch].AD_A[1][range][0],
								(char *)&myData->cali.tmpData_caliMeter2[bd][ch].AD_A[1][range][0],
								sizeof(double)*(MAX_CALI_POINT-1));
						memcpy((char *)&myData->cali.data_caliMeter2[bd][ch].AD_B[1][range][0],
								(char *)&myData->cali.tmpData_caliMeter2[bd][ch].AD_B[1][range][0],
								sizeof(double)*(MAX_CALI_POINT-1));
					}
				}
			}
		}
	}
}

int Write_BdCaliData(int bd)
{
    int	ch, type, range, point, save_ch, tmp; 
	char temp[4], fileName[128];
    FILE *fp;

	struct tm *date;
	const time_t t = time(NULL);
	date = localtime(&t);

	tmp = 0;
	
	if(myData->mData.config.chPerBd > 32){
		save_ch = 64;
	}else if(myData->mData.config.chPerBd > 16){
		save_ch = 32;
	}else{
		save_ch = 16;
	}
	//190617 oys add start
	if(save_ch > MAX_CH_PER_BD)
		save_ch = MAX_CH_PER_BD;
	//add end

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
//190624 oys modify end
	strcat(fileName, temp);
	// /root/cycler_data/config/caliData/CALI_BD#
	
    if((fp = fopen(fileName, "w")) == NULL) {
		userlog(DEBUG_LOG, psName,
			"Can not open %d Board CaliData file(write)\n", bd+1);
		return -1;
	}
	
	fprintf(fp, "MCTS_Calibration_File\n");
	fprintf(fp, "KJG001-00\n");
	fprintf(fp, "date:%04d/%02d/%02d/%02d:%02d:%02d\n\n",
					date->tm_year+1900,
					date->tm_mon+1,
					date->tm_mday,
					date->tm_hour,
					date->tm_min,
					date->tm_sec);
//	fprintf(fp, "%s\n\n", "date");
	
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

//			for(ch=0; ch < MAX_CH_PER_BD; ch++) {
			for(ch=0; ch < save_ch; ch++) {
	    		fprintf(fp, "ch%02d\n", ch+1);
				
				fprintf(fp, "setPointNum   \n");
			   	fprintf(fp, "%d ", myData->cali.data[bd][ch]
					.point[type][range].setPointNum);
				fprintf(fp, "\n");

				fprintf(fp, "setPoint      \n");
				for(point=0; point < MAX_CALI_POINT; point++) {
			    	fprintf(fp, "%ld ", myData->cali.data[bd][ch]
						.point[type][range].setPoint[point]);
				}
				fprintf(fp, "\n");

				fprintf(fp, "checkPointNum \n");
			   	fprintf(fp, "%d ", myData->cali.data[bd][ch]
					.point[type][range].checkPointNum);
				fprintf(fp, "\n");

				fprintf(fp, "checkPoint    \n");
				for(point=0; point < MAX_CALI_POINT; point++) {
			    	fprintf(fp, "%ld ", myData->cali.data[bd][ch]
						.point[type][range].checkPoint[point]);
				}
				fprintf(fp, "\n");

				fprintf(fp, "set_ad        \n");
				for(point=0; point < MAX_CALI_POINT; point++) {
#if CYCLER_TYPE == LINEAR_CYC || CYCLER_TYPE == CAN_CYC
				   	fprintf(fp, "%f ", (double)myData->cali.data[bd][ch]
								.set_ad[type][range][point]);
#endif
#if CYCLER_TYPE == DIGITAL_CYC
					if(myData->cali.data[bd][ch]
								.point[type][range].setPointNum > point){
			   			fprintf(fp, "%f ", (double)myData->cali.data[bd][ch]
							.set_ad[type][range][point]);
					}else{
			   			fprintf(fp, "%f ", (double)tmp);
					}
#endif
				}
				fprintf(fp, "\n");

				fprintf(fp, "set_meter     \n");
				for(point=0; point < MAX_CALI_POINT; point++) {
#if CYCLER_TYPE == LINEAR_CYC || CYCLER_TYPE == CAN_CYC
				   	fprintf(fp, "%f ", (double)myData->cali.data[bd][ch]
								.set_meter[type][range][point]);
#endif
#if CYCLER_TYPE == DIGITAL_CYC
					if(myData->cali.data[bd][ch]
								.point[type][range].setPointNum > point){
		    			fprintf(fp, "%f ", (double)myData->cali.data[bd][ch]
							.set_meter[type][range][point]);
					}else{
		    			fprintf(fp, "%f ", (double)tmp);
					}					
#endif
				}
				fprintf(fp, "\n");

				fprintf(fp, "check_ad      \n");
				for(point=0; point < MAX_CALI_POINT; point++) {
#if CYCLER_TYPE == LINEAR_CYC || CYCLER_TYPE == CAN_CYC
			   		fprintf(fp, "%f ", (double)myData->cali.data[bd][ch]
						.check_ad[type][range][point]);
#endif
#if CYCLER_TYPE == DIGITAL_CYC
					if(myData->cali.data[bd][ch]
								.point[type][range].checkPointNum > point){
			    		fprintf(fp, "%f ", (double)myData->cali.data[bd][ch]
								.check_ad[type][range][point]);
					}else{
		    			fprintf(fp, "%f ", (double)tmp);
					}					
#endif
				}
				fprintf(fp, "\n");

				fprintf(fp, "check_meter   \n");
				for(point=0; point < MAX_CALI_POINT; point++) {
#if CYCLER_TYPE == LINEAR_CYC || CYCLER_TYPE == CAN_CYC
			   		fprintf(fp, "%f ", (double)myData->cali.data[bd][ch]
						.check_meter[type][range][point]);
#endif
#if CYCLER_TYPE == DIGITAL_CYC
					if(myData->cali.data[bd][ch]
								.point[type][range].checkPointNum > point){
				    	fprintf(fp, "%f ", (double)myData->cali.data[bd][ch]
											.check_meter[type][range][point]);
					}else{
				   		fprintf(fp, "%f ", (double)tmp);
					}					
#endif
				}
				fprintf(fp, "\n");

				fprintf(fp, "DA_A          \n");
				for(point=0; point < MAX_CALI_POINT-1; point++) {
#if CYCLER_TYPE == LINEAR_CYC || CYCLER_TYPE == CAN_CYC
				  	fprintf(fp, "%f ", (double)myData->cali.data[bd][ch]
						.DA_A[type][range][point]);
#endif
#if CYCLER_TYPE == DIGITAL_CYC
					if(myData->cali.data[bd][ch]
								.point[type][range].setPointNum-1 > point){
				    	fprintf(fp, "%f ", (double)myData->cali.data[bd][ch]
											.DA_A[type][range][point]);
					}else{
				  		fprintf(fp, "%f ", (double)tmp);
					}
#endif
				}
				fprintf(fp, "\n");

				fprintf(fp, "DA_B          \n");
				for(point=0; point < MAX_CALI_POINT-1; point++) {
#if CYCLER_TYPE == LINEAR_CYC || CYCLER_TYPE == CAN_CYC
			   		fprintf(fp, "%f ", (double)myData->cali.data[bd][ch]
						.DA_B[type][range][point]);
#endif
#if CYCLER_TYPE == DIGITAL_CYC
					if(myData->cali.data[bd][ch]
								.point[type][range].setPointNum-1 > point){
				   		fprintf(fp, "%f ", (double)myData->cali.data[bd][ch]
							.DA_B[type][range][point]);
					}else{
				  		fprintf(fp, "%f ", (double)tmp);
					}					
#endif
				}
				fprintf(fp, "\n");

				fprintf(fp, "AD_A          \n");
				for(point=0; point < MAX_CALI_POINT-1; point++) {
#if CYCLER_TYPE == LINEAR_CYC || CYCLER_TYPE == CAN_CYC
		    		fprintf(fp, "%f ", (double)myData->cali.data[bd][ch]
						.AD_A[type][range][point]);
#endif
#if CYCLER_TYPE == DIGITAL_CYC
					if(myData->cali.data[bd][ch]
								.point[type][range].setPointNum-1 > point){
		    			fprintf(fp, "%f ", (double)myData->cali.data[bd][ch]
							.AD_A[type][range][point]);
					}else{
		    			fprintf(fp, "%f ",(double)tmp);
					}					
#endif
				}
				fprintf(fp, "\n");

				fprintf(fp, "AD_B\n");
				for(point=0; point < MAX_CALI_POINT-1; point++) {
#if CYCLER_TYPE == LINEAR_CYC || CYCLER_TYPE == CAN_CYC
			   		fprintf(fp, "%f ", (double)myData->cali.data[bd][ch]
						.AD_B[type][range][point]);
#endif
#if CYCLER_TYPE == DIGITAL_CYC
					if(myData->cali.data[bd][ch]
								.point[type][range].setPointNum-1 > point){
			    		fprintf(fp, "%f ", (double)myData->cali.data[bd][ch]
										.AD_B[type][range][point]);
					}else{
			 	   		fprintf(fp, "%f ", (double)tmp);
					}					
#endif
				}
				fprintf(fp, "\n");

				fprintf(fp, "AD_Ratio      \n");
				fprintf(fp, "%f %f",
					(double)myData->cali.data[bd][ch].AD_Ratio[type][range][0],
					(double)myData->cali.data[bd][ch].AD_Ratio[type][range][1]);
				fprintf(fp, "\n\n");
			}
		}
	}
   	fclose(fp);

	return 0;
}

int Write_BdCaliData_FAD(int bd)
{
    int	ch, type, range, point, save_ch;
	char temp[4], fileName[128];
    FILE *fp;

	struct tm *date;
	const time_t t = time(NULL);
	date = localtime(&t);
	
	if(myData->mData.config.chPerBd > 32){
		save_ch = 64;
	}else if(myData->mData.config.chPerBd > 16){
		save_ch = 32;
	}else{
		save_ch = 16;
	}
	//190617 oys add start
	if(save_ch > MAX_CH_PER_BD)
		save_ch = MAX_CH_PER_BD;
	//add end

	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/cycler_data/config/caliData/CALI_BD");
	memset(temp, 0x00, sizeof temp);
	temp[0] = (char)(49+bd);
	strcat(fileName, temp);
	strcat(fileName, "_FAD");
	// /root/cycler_data/config/caliData/CALI_BD#_FAD
	
    if((fp = fopen(fileName, "w+")) == NULL) {
		userlog(DEBUG_LOG, psName,
			"Can not open %d FAD Board CaliData file(write)\n", bd+1);
		return -1;
	}
	
	fprintf(fp, "MCTS_Calibration_File\n");
	fprintf(fp, "PNE001-00\n");
	fprintf(fp, "date:%04d/%02d/%02d/%02d:%02d:%02d\n\n",
					date->tm_year+1900,
					date->tm_mon+1,
					date->tm_mday,
					date->tm_hour,
					date->tm_min,
					date->tm_sec);
//	fprintf(fp, "%s\n\n", "date");
	
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

//			for(ch=0; ch < MAX_CH_PER_BD; ch++) {
			for(ch=0; ch < save_ch; ch++) {
	    		fprintf(fp, "ch%02d\n", ch+1);

				fprintf(fp, "setPointNum   \n");
			   	fprintf(fp, "%d ", myData->cali.data[bd][ch]
					.point[type][range].setPointNum);
				fprintf(fp, "\n");

				fprintf(fp, "setPoint      \n");
				for(point=0; point < MAX_CALI_POINT; point++) {
			    	fprintf(fp, "%ld ", myData->cali.data[bd][ch]
						.point[type][range].setPoint[point]);
				}
				fprintf(fp, "\n");

				fprintf(fp, "checkPointNum \n");
			   	fprintf(fp, "%d ", myData->cali.data[bd][ch]
					.point[type][range].checkPointNum);
				fprintf(fp, "\n");

				fprintf(fp, "checkPoint    \n");
				for(point=0; point < MAX_CALI_POINT; point++) {
			    	fprintf(fp, "%ld ", myData->cali.data[bd][ch]
						.point[type][range].checkPoint[point]);
				}
				fprintf(fp, "\n");

				fprintf(fp, "set_ad        \n");
				for(point=0; point < MAX_CALI_POINT; point++) {
			    	fprintf(fp, "%f ", (double)myData->cali.data_fad[bd][ch]
						.set_ad[type][range][point]);
				}
				fprintf(fp, "\n");

				fprintf(fp, "set_meter     \n");
				for(point=0; point < MAX_CALI_POINT; point++) {
			    	fprintf(fp, "%f ", (double)myData->cali.data[bd][ch]
						.set_meter[type][range][point]);
				}
				fprintf(fp, "\n");

				fprintf(fp, "check_ad      \n");
				for(point=0; point < MAX_CALI_POINT; point++) {
			    	fprintf(fp, "%f ", (double)myData->cali.data_fad[bd][ch]
						.check_ad[type][range][point]);
				}
				fprintf(fp, "\n");

				fprintf(fp, "check_meter   \n");
				for(point=0; point < MAX_CALI_POINT; point++) {
			    	fprintf(fp, "%f ", (double)myData->cali.data[bd][ch]
						.check_meter[type][range][point]);
				}
				fprintf(fp, "\n");

				fprintf(fp, "AD_A          \n");
				for(point=0; point < MAX_CALI_POINT-1; point++) {
		    		fprintf(fp, "%f ", (double)myData->cali.data_fad[bd][ch]
						.AD_A[type][range][point]);
				}
				fprintf(fp, "\n");

				fprintf(fp, "AD_B\n");
				for(point=0; point < MAX_CALI_POINT-1; point++) {
			    	fprintf(fp, "%f ", (double)myData->cali.data_fad[bd][ch]
						.AD_B[type][range][point]);
				}
				fprintf(fp, "\n\n");
			}
		}
	}
   	fclose(fp);

	return 0;
}

int Write_BdCaliData_I_Offset(int bd)
{
    int	ch, type, range, point, save_ch;
	char temp[4], fileName[128];
    FILE *fp;

	struct tm *date;
	const time_t t = time(NULL);
	date = localtime(&t);

	if(myData->mData.config.chPerBd > 32){
		save_ch = 64;
	}else if(myData->mData.config.chPerBd > 16){
		save_ch = 32;
	}else{
		save_ch = 16;
	}
	//190617 oys add start
	if(save_ch > MAX_CH_PER_BD)
		save_ch = MAX_CH_PER_BD;
	//add end

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
//190624 oys modify end
	strcat(fileName, temp);
	strcat(fileName, "_I_Offset");
	// /root/cycler_data/config/caliData/CALI_BD#_I_Offset
	
    if((fp = fopen(fileName, "w+")) == NULL) {
		userlog(DEBUG_LOG, psName,
			"Can not open BD[%d] I Offset CaliData file(write)\n", bd+1);
		return -1;
	}
	
	fprintf(fp, "MCTS_Calibration_File\n");
	fprintf(fp, "PNE001-00\n");
	fprintf(fp, "date:%04d/%02d/%02d/%02d:%02d:%02d\n\n",
					date->tm_year+1900,
					date->tm_mon+1,
					date->tm_mday,
					date->tm_hour,
					date->tm_min,
					date->tm_sec);
//	fprintf(fp, "%s\n\n", "date");
	
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

			for(ch=0; ch < save_ch; ch++) {
	    		fprintf(fp, "ch%02d\n", ch+1);

				fprintf(fp, "setPointNum   \n");
			   	fprintf(fp, "%d ", myData->cali.data[bd][ch]
					.point[0][range].setPointNum);
				fprintf(fp, "\n");

				fprintf(fp, "setPoint      \n");
				for(point=0; point < MAX_CALI_POINT; point++) {
			    	fprintf(fp, "%ld ", myData->cali.data[bd][ch]
						.point[0][range].setPoint[point]);
				}
				fprintf(fp, "\n");

				fprintf(fp, "checkPointNum \n");
			   	fprintf(fp, "%d ", myData->cali.data[bd][ch]
					.point[0][range].checkPointNum);
				fprintf(fp, "\n");

				fprintf(fp, "checkPoint    \n");
				for(point=0; point < MAX_CALI_POINT; point++) {
			    	fprintf(fp, "%ld ", myData->cali.data[bd][ch]
						.point[0][range].checkPoint[point]);
				}
				fprintf(fp, "\n");

				fprintf(fp, "set_ad        \n");
				for(point=0; point < MAX_CALI_POINT; point++) {
			    	fprintf(fp, "%f ", (double)myData->cali.data_caliMeter2[bd][ch]
						.set_ad[1][range][point]);
				}
				fprintf(fp, "\n");

				fprintf(fp, "set_meter     \n");
				for(point=0; point < MAX_CALI_POINT; point++) {
			    	fprintf(fp, "%f ", (double)myData->cali.data_caliMeter2[bd][ch]
						.set_meter[1][range][point]);
				}
				fprintf(fp, "\n");

				fprintf(fp, "check_ad      \n");
				for(point=0; point < MAX_CALI_POINT; point++) {
			    	fprintf(fp, "%f ", (double)myData->cali.data_caliMeter2[bd][ch]
						.check_ad[1][range][point]);
				}
				fprintf(fp, "\n");

				fprintf(fp, "check_meter   \n");
				for(point=0; point < MAX_CALI_POINT; point++) {
			    	fprintf(fp, "%f ", (double)myData->cali.data_caliMeter2[bd][ch]
						.check_meter[1][range][point]);
				}
				fprintf(fp, "\n");

				fprintf(fp, "AD_A          \n");
				for(point=0; point < MAX_CALI_POINT-1; point++) {
		    		fprintf(fp, "%f ", (double)myData->cali.data_caliMeter2[bd][ch]
						.AD_A[1][range][point]);
				}
				fprintf(fp, "\n");

				fprintf(fp, "AD_B\n");
				for(point=0; point < MAX_CALI_POINT-1; point++) {
			    	fprintf(fp, "%f ", (double)myData->cali.data_caliMeter2[bd][ch]
						.AD_B[1][range][point]);
				}
				fprintf(fp, "\n\n");
			}
		}
	}
   	fclose(fp);

	return 0;
}


// 140623 oys w : Shunt Calibration
#if SHUNT_R_RCV == 1
int Write_Shunt_Cali_Info(int bd)
{
	char fileName[128];
	int	i, range, save_bd, save_ch, installedBd;
	FILE *fp;
	struct tm *now;
	time_t	t;

	t = time(NULL);
	now = localtime(&t);

	installedBd = myData->mData.config.installedBd;

	if(myData->mData.config.chPerBd > 32){
		save_ch = 64;
	} else if(myData->mData.config.chPerBd > 16) {
		save_ch = 32;
	} else if(myData->mData.config.chPerBd > 8) { 
		save_ch = 16;
	} else {
		save_ch = 8;
	}
	//190617 oys add start
	if(save_ch > MAX_CH_PER_BD)
		save_ch = MAX_CH_PER_BD;
	//add end
	
	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/cycler_data/config/parameter/Shunt_Cali_Info");

	if((fp = fopen(fileName, "w")) == NULL) {
		userlog(DEBUG_LOG, psName,
						"Shunt_Cali_Info file write error\n");
		return -1;
	}
	for(save_bd = 0; save_bd < installedBd; save_bd++) {
		fprintf(fp, "BD%02d\n", save_bd+1);
		for(range = 0; range < myData->mData.config.rangeI; range++) {
			if(range+1 == RANGE1) {
				fprintf(fp, "RANGE1\n");
				fprintf(fp, "BD\t\tCH\t\tShunt[mOhm]\t\tSerialNo\n");
			} else if (range+1 == RANGE2) {
				fprintf(fp, "RANGE2\n");
				fprintf(fp, "BD\t\tCH\t\tShunt[mOhm]\t\tSerialNo\n");
			} else if (range+1 == RANGE3) {
				fprintf(fp, "RANGE3\n");
				fprintf(fp, "BD\t\tCH\t\tShunt[mOhm]\t\tSerialNo\n");
			} else {
				fprintf(fp, "RANGE4\n");
				fprintf(fp, "BD\t\tCH\t\tShunt[mOhm]\t\tSerialNo\n");
			}		   	

			for(i = 0; i < save_ch; i++) {	
				fprintf(fp, "%2d\t\t", save_bd+1);
				fprintf(fp, "%2d\t\t", i+1);
				fprintf(fp,	"%6.6f\t\t",
					myData->cali.tmpCond[bd][i]
						.point[1][range].shuntValue);
				fprintf(fp, ">>%s\n",
				myData->cali.tmpCond[bd][i].point[1][range].shuntSerialNo);
			}
		}
		fprintf(fp, "\n");
	}

	fprintf(fp, "Last Update : %d.%d.%2d / %d:%d:%d\n",
		now->tm_year+1900,now->tm_mon+1,now->tm_mday,now->tm_hour,
		now->tm_min,now->tm_sec);
	fclose(fp);

	userlog(DEBUG_LOG, psName, "### Shunt_Cali_Info file Update ###\n");
	return 0;
}
#endif

void Convert_testCond(int bd, int ch, int parallel)
{
	int i, j=0, z,rangeI, parallel_cycle;
	long refI;
	unsigned long time;
	double tmp;
	int k;			   //210602 LJS
#if HYUNDAE == 1
	int z = 0;
#endif

	parallel_cycle = z =0; //kjg_180521
	k = 0;    			   //210604 LJS

	//testcond update not init
	if(myData->bData[bd].cData[ch].misc.testCondUpdate == 0) {
		memset((char *)&myData->mData.testCond[bd][ch], 0,
			sizeof(S_TEST_CONDITION));
	}else{
		myData->bData[bd].cData[ch].misc.testCondUpdate = 0;
	}

	memcpy((char *)&myData->mData.testCond[bd][ch].header,
		(char *)&myPs->testCond.header, sizeof(S_MAIN_TEST_COND_HEADER));

	memcpy((char *)&myData->mData.testCond[bd][ch].safety,
		(char *)&myPs->testCond.safety, sizeof(S_MAIN_ADP_TEST_COND_SAFETY));

	for(i=0; i < myPs->testCond.header.totalStep; i++) {
		myData->mData.testCond[bd][ch].step[i].type
			= (unsigned char)myPs->testCond.step[i].header.type;

		if(myData->mData.config.parallelMode != P2) { //kjg_180521
			if(myData->mData.testCond[bd][ch].step[i].type
				== STEP_PARALLEL_CYCLE) {
				myData->mData.testCond[bd][ch].step[i].type = STEP_ADV_CYCLE;
			}
		}

		myData->mData.testCond[bd][ch].step[i].stepNo
			= (unsigned long)myPs->testCond.step[i].header.stepNo;

		if(myData->mData.testCond[bd][ch].step[i].type != STEP_USER_PATTERN
			&& myData->mData.testCond[bd][ch].step[i].type != STEP_USER_MAP) {
			myData->mData.testCond[bd][ch].step[i].mode
				= myPs->testCond.step[i].header.mode;
		}

		myData->mData.testCond[bd][ch].step[i].testEnd
			= myPs->testCond.step[i].header.testEnd;
		myData->mData.testCond[bd][ch].step[i].subStep
			= myPs->testCond.step[i].header.subStep;
		myData->mData.testCond[bd][ch].step[i].useSocFlag
			= myPs->testCond.step[i].header.useSocFlag;

		//ref V, Temp
		myData->mData.testCond[bd][ch].step[i].refV
			= myPs->testCond.step[i].reference[0].refV;
		myData->mData.testCond[bd][ch].step[i].refTemp
			= myPs->testCond.step[i].reference[0].refTemp * 100;
		//20180618 sch modify for stepSync
		if(myData->mData.testCond[bd][ch].step[i].refTemp == 999000
			&& myData->bData[bd].cData[ch].misc.stepSyncFlag == 0){
			myData->bData[bd].cData[ch].misc.chGroupNo = 0;
			//190624 add lyhw
			if(myData->bData[bd].cData[ch].ChAttribute.opType == P1){
				myData->bData[bd].cData[ch+1].misc.chGroupNo = 0;
			}
		}

		//rangeV
		myData->mData.testCond[bd][ch].step[i].rangeV
			= (unsigned char)(RANGE1 - 1);

		//pjy add for toshiba
		myData->mData.testCond[bd][ch].step[i].integralCapFlag
			= myPs->testCond.step[i].reference[0].integralCapFlag;

		//170626 oys add
		myData->mData.testCond[bd][ch].step[i].endZeroVoltageFlag
			= myPs->testCond.step[i].reference[0].endZeroVoltageFlag;

#if VENDER == 3 //170501 oys add
		myData->mData.testCond[bd][ch].step[i].stepNo_pause
			= myPs->testCond.step[i].reference[0].stepNo_pause;
		myData->mData.testCond[bd][ch].step[i].cycleNo_pause
			= myPs->testCond.step[i].reference[0].cycleNo_pause;

		myData->mData.testCond[bd][ch].step[i].endP_soc
			= myPs->testCond.step[i].reference[0].endP_soc;
		myData->mData.testCond[bd][ch].step[i].endZ_soc
			= myPs->testCond.step[i].reference[0].endZ_soc;

		myData->mData.testCond[bd][ch].step[i].endP_std_sel
			= myPs->testCond.step[i].reference[0].endP_std_sel;
		myData->mData.testCond[bd][ch].step[i].endZ_std_sel
			= myPs->testCond.step[i].reference[0].endZ_std_sel;

		myData->mData.testCond[bd][ch].step[i].endC_std_cycleCount
			= myPs->testCond.step[i].reference[0].endC_std_cycleCount;
		myData->mData.testCond[bd][ch].step[i].endP_std_cycleCount
			= myPs->testCond.step[i].reference[0].endP_std_cycleCount;
		myData->mData.testCond[bd][ch].step[i].endZ_std_cycleCount
			= myPs->testCond.step[i].reference[0].endZ_std_cycleCount;

		myData->mData.testCond[bd][ch].step[i].endP_std_type
			= myPs->testCond.step[i].reference[0].endP_std_type;
		myData->mData.testCond[bd][ch].step[i].endZ_std_type
			= myPs->testCond.step[i].reference[0].endZ_std_type;
		
		myData->mData.testCond[bd][ch].step[i].endC_proc_type
			= myPs->testCond.step[i].reference[0].endC_proc_type;
		myData->mData.testCond[bd][ch].step[i].endP_proc_type
			= myPs->testCond.step[i].reference[0].endP_proc_type;
		myData->mData.testCond[bd][ch].step[i].endZ_proc_type
			= myPs->testCond.step[i].reference[0].endZ_proc_type;
		
		myData->mData.testCond[bd][ch].step[i].noTempWaitFlag
			= myPs->testCond.step[i].reference[0].noTempWaitFlag;

		myData->mData.testCond[bd][ch].step[i].reduce_ratio_P
			= myPs->testCond.step[i].reference[0].reduce_ratio_P;
		
		myData->mData.testCond[bd][ch].step[i].SocSoeFlag //20190214 add
			= myPs->testCond.step[i].reference[0].SocSoeFlag;
		
#endif
#ifdef _TRACKING_MODE
		myData->mData.testCond[bd][ch].step[i].SOC_Tracking_flag
			= myPs->testCond.step[i].reference[0].SOC_Tracking_flag;	
		myData->mData.testCond[bd][ch].step[i].rptSOC
			= myPs->testCond.step[i].reference[0].rptSOC;	
		myData->mData.testCond[bd][ch].step[i].crate_flag 
			= myPs->testCond.step[i].crate_flag;
		myData->mData.testCond[bd][ch].step[i].limitCurrent_Upper
			= 0;
		myData->mData.testCond[bd][ch].step[i].limitCurrent_Lower
			= 0;
		myData->mData.testCond[bd][ch].step[i].trackingMode_flag //210720 
			= myPs->testCond.step[i].reference[0].trackingMode_flag;	
		myData->mData.testCond[bd][ch].step[i].SOH_Tracking_flag //211022
			= myPs->testCond.step[i].reference[0].SOH_Tracking_flag;	
		myData->mData.testCond[bd][ch].step[i].rptSOH
			= myPs->testCond.step[i].reference[0].rptSOH;	
#endif

		if(myData->mData.testCond[bd][ch].step[i].type == STEP_ADV_CYCLE) {
			j++;
		} else if(myData->mData.testCond[bd][ch].step[i].type
			== STEP_PARALLEL_CYCLE) { //kjg_180521
			j++;
			parallel_cycle = 1;
		}		
		myData->mData.testCond[bd][ch].step[i].cycleNo = j;

		if(myData->mData.testCond[bd][ch].step[i].type == STEP_BALANCE) {
			myData->mData.testCond[bd][ch].step[i].balanceStepCheck = 1;
		} else {
			myData->mData.testCond[bd][ch].step[i].balanceStepCheck = 0;
		}

		//ref I, P, R
		if(myData->mData.testCond[bd][ch].step[i].type == STEP_CHARGE 
			|| myData->mData.testCond[bd][ch].step[i].type == STEP_BALANCE) {
			if(myPs->testCond.step[i].header.mode == CP) {
				myData->mData.testCond[bd][ch].step[i].refP
					= myPs->testCond.step[i].reference[0].refI;
				refI = myData->mData.config.maxCurrent[0];
#if VENDER == 3
				myData->mData.testCond[bd][ch].step[i].refV_L
					= myPs->testCond.step[i].reference[0].refV_L;
#endif
			} else if(myPs->testCond.step[i].header.mode == CR) {
				myData->mData.testCond[bd][ch].step[i].refR
					= myPs->testCond.step[i].reference[0].refI;
				tmp = (double)myPs->testCond.step[i].reference[0].refV
					/ (double)myPs->testCond.step[i].reference[0].refI * 1000.0;
				refI = (long)tmp;
			} else {
				refI = myPs->testCond.step[i].reference[0].refI;
			}
		} else if(myData->mData.testCond[bd][ch].step[i].type == STEP_DISCHARGE
			|| myData->mData.testCond[bd][ch].step[i].type == STEP_Z) {
			if(myPs->testCond.step[i].header.mode == CP) {
				myData->mData.testCond[bd][ch].step[i].refP
					= myPs->testCond.step[i].reference[0].refI;
				refI = myData->mData.config.maxCurrent[0];
#if VENDER == 3
				myData->mData.testCond[bd][ch].step[i].refV_L
					= myPs->testCond.step[i].reference[0].refI_L;
#endif
			} else if(myPs->testCond.step[i].header.mode == CR) {
				myData->mData.testCond[bd][ch].step[i].refR
					= myPs->testCond.step[i].reference[0].refI;
				tmp = (double)myData->mData.config.maxVoltage[0]
					/ (double)myPs->testCond.step[i].reference[0].refI * 1000.0;
				refI = (long)tmp;
			} else {
				refI = myPs->testCond.step[i].reference[0].refI;
			}
		} else {
			refI = myPs->testCond.step[i].reference[0].refI;
		}

		if(parallel == 1 || parallel_cycle == 1) { //kjg_180521
			if(myPs->testCond.step[i].header.mode == CP) {
				myData->mData.testCond[bd][ch].step[i].refP
					= (long)myData->mData.testCond[bd][ch].step[i].refP / 2; 
#if VENDER == 3
				myData->mData.testCond[bd][ch].step[i].refV_L
					= myData->mData.testCond[bd][ch].step[i].refV_L / 2;
#endif
			} else {
				refI =  (long)refI / 2;
			}
		}
		myData->mData.testCond[bd][ch].step[i].refI = refI;
		
		myData->mData.testCond[bd][ch].step[i].endP
			= myPs->testCond.step[i].reference[0].endWatt;

		refI = Convert_C_rate_to_I_value(bd, ch, refI);

		//rangeI		
		if(myData->mData.testCond[bd][ch].step[i].type != STEP_USER_PATTERN
			&& myData->mData.testCond[bd][ch].step[i].type != STEP_USER_MAP) {
			if(refI <= myData->mData.config.maxCurrent[3]) {
				rangeI = RANGE4 - 1;
			} else if(refI <= myData->mData.config.maxCurrent[2]) {
				rangeI = RANGE3 - 1;
			} else if(refI <= myData->mData.config.maxCurrent[1]) {
				rangeI = RANGE2 - 1;
			} else {
				rangeI = RANGE1 - 1;
			}
			if((rangeI + 1) > (int)myData->mData.config.rangeI) {
				rangeI = (int)myData->mData.config.rangeI - 1;
			}
			myData->mData.testCond[bd][ch].step[i].rangeI
				= (unsigned char)rangeI;
		}
		//210525 lyhw for 2uA
		if(myData->mData.config.hwSpec == L_5V_500mA_2uA_R4){
			if(myData->mData.testCond[bd][ch].step[i].type == STEP_IDLE
				|| myData->mData.testCond[bd][ch].step[i].type == STEP_REST
				|| myData->mData.testCond[bd][ch].step[i].type == STEP_OCV
				|| myData->mData.testCond[bd][ch].step[i].type == STEP_END
				|| myData->mData.testCond[bd][ch].step[i].type == STEP_ADV_CYCLE
				|| myData->mData.testCond[bd][ch].step[i].type == STEP_LOOP){
				myData->mData.testCond[bd][ch].step[i].rangeI
					= (unsigned char)RANGE3 - 1;
			}
		
		//210706 not use	
		//	if(rangeI == (RANGE4 - 1)){
		//		myData->mData.testCond[bd][ch].step[i].rangeV = RANGE1;
		//	}
		}

		if(parallel_cycle == 1) { //kjg_180521
			myData->mData.testCond[bd][ch].step[i].rangeI = RANGE1 - 1;
		}

		//end Time
		myData->mData.testCond[bd][ch].step[i].endT
			= myPs->testCond.step[i].reference[0].endT;
		myData->mData.testCond[bd][ch].step[i].endTGoto
			= myPs->testCond.step[i].reference[0].endTGoto;
		#if NETWORK_VERSION > 4101
			#if VENDER != 2 //NOT SDI
		myData->mData.testCond[bd][ch].step[i].endT_CV 
			= myPs->testCond.step[i].reference[0].endT_CV;
		myData->mData.testCond[bd][ch].step[i].endTCVGoto
			= myPs->testCond.step[i].reference[0].endTCVGoto;
			#endif
		#endif

		//aux Control
		#if AUX_CONTROL == 1
			#if NETWORK_VERSION >= 4103
		if(myData->mData.config.installedTemp != 0
			|| myData->mData.config.installedAuxV != 0) {
			memcpy((char *)&myData->mData.testCond[bd][ch].step[i].auxType,
				(char *)&myPs->testCond.step[i].reference[0].auxType,
				sizeof(short int) * MAX_AUX_FUNCTION);
			memcpy((char *)&myData->mData.testCond[bd][ch].step[i]
				.auxCompareType,
				(char *)&myPs->testCond.step[i].reference[0].auxCompareType,
				sizeof(char) * MAX_AUX_FUNCTION);
			memcpy((char *)&myData->mData.testCond[bd][ch].step[i].auxGoto,
				(char *)&myPs->testCond.step[i].reference[0].auxGoto,
				sizeof(short int) * MAX_AUX_FUNCTION);
			memcpy((char *)&myData->mData.testCond[bd][ch].step[i].endAuxValue,
				(char *)&myPs->testCond.step[i].reference[0].endAuxValue,
				sizeof(long) * MAX_AUX_FUNCTION);
		}
			#endif
		#endif

		//end V, I
		switch(myData->mData.config.hwSpec) {
			case S_5V_200A:
				if(myData->mData.testCond[bd][ch].step[i].type == STEP_DISCHARGE
					|| myData->mData.testCond[bd][ch].step[i].type == STEP_Z) {
					if(myPs->testCond.step[i].reference[0].endV == 0
						|| myPs->testCond.step[i].reference[0].endV
						<= myPs->testCond.step[i].reference[0].refV) {
						myData->mData.testCond[bd][ch].step[i].endV
							= myPs->testCond.step[i].reference[0].refV;
					} else {
						myData->mData.testCond[bd][ch].step[i].endV
							= myPs->testCond.step[i].reference[0].endV;
					}
				} else {
					myData->mData.testCond[bd][ch].step[i].endV
						= myPs->testCond.step[i].reference[0].endV;
				}
				break;
			case S_5V_200A_75A_15A_AD2:
				if(myData->mData.testCond[bd][ch].step[i].type == STEP_DISCHARGE
					|| myData->mData.testCond[bd][ch].step[i].type == STEP_Z) {
					if(myPs->testCond.step[i].reference[0].endV != 0
						&& myPs->testCond.step[i].reference[0].endV
						< myPs->testCond.step[i].reference[0].refV) {
						myData->mData.testCond[bd][ch].step[i].endV
							= myPs->testCond.step[i].reference[0].refV;
					} else {
						myData->mData.testCond[bd][ch].step[i].endV
							= myPs->testCond.step[i].reference[0].endV;
					}
				} else {
					myData->mData.testCond[bd][ch].step[i].endV
						= myPs->testCond.step[i].reference[0].endV;
				}
				break;
			default: //linear
				myData->mData.testCond[bd][ch].step[i].endV
					= myPs->testCond.step[i].reference[0].endV;
				break;
		}

		myData->mData.testCond[bd][ch].step[i].endVGoto
			= myPs->testCond.step[i].reference[0].endVGoto;

		if(myData->mData.testCond[bd][ch].step[i].type == STEP_USER_PATTERN
			|| myData->mData.testCond[bd][ch].step[i].type == STEP_USER_MAP) {
			myData->mData.testCond[bd][ch].step[i].endV_L
				= myPs->testCond.step[i].reference[0].endI;
			//110411 kji pattern refV
			myData->mData.testCond[bd][ch].step[i].refV_H
				= myPs->testCond.step[i].reference[0].refV;
			myData->mData.testCond[bd][ch].step[i].refV_L
				= myPs->testCond.step[i].reference[0].refI;
#if VENDER == 3
			//170501 oys pattern refI
			myData->mData.testCond[bd][ch].step[i].refI_H
				= myPs->testCond.step[i].reference[0].refV_L;
				//= myPs->testCond.step[i].reference[0].refI_H; //kjg_180521
			myData->mData.testCond[bd][ch].step[i].refI_L
				= myPs->testCond.step[i].reference[0].refI_L;
			if(parallel == 1 || parallel_cycle == 1) { //kjg_180521
				myData->mData.testCond[bd][ch].step[i].refI_H
					= myData->mData.testCond[bd][ch].step[i].refI_H / 2;
				myData->mData.testCond[bd][ch].step[i].refI_L
					= myData->mData.testCond[bd][ch].step[i].refI_L / 2;
			}
#endif
		} else if(myData->mData.testCond[bd][ch].step[i].type == STEP_REST) {
			//110413 oys REST_STEP endV_L
			myData->mData.testCond[bd][ch].step[i].endV_L
				= myPs->testCond.step[i].reference[0].endI;
		} else {
			myData->mData.testCond[bd][ch].step[i].endI
				= myPs->testCond.step[i].reference[0].endI;
		}

		//111215 kji add 
		myData->mData.testCond[bd][ch].step[i].endIGoto
			= myPs->testCond.step[i].reference[0].endIGoto;
		//171121 oys add
		myData->mData.testCond[bd][ch].step[i].endIntegralCGoto
			= myPs->testCond.step[i].reference[0].endIntegralCGoto;
		myData->mData.testCond[bd][ch].step[i].endIntegralWhGoto
			= myPs->testCond.step[i].reference[0].endIntegralWhGoto;
#if VENDER == 2
		//210422 LJS add
		myData->mData.testCond[bd][ch].step[i].chamber_dev
			= myPs->testCond.step[i].reference[0].chamber_dev * 100;
#endif
		myData->mData.testCond[bd][ch].step[i].endC
			= myPs->testCond.step[i].reference[0].endC;
		myData->mData.testCond[bd][ch].step[i].endCGoto
			= myPs->testCond.step[i].reference[0].endCGoto;
		myData->mData.testCond[bd][ch].step[i].advGotoStep
			= myPs->testCond.step[i].reference[0].GotoCondition;
		myData->mData.testCond[bd][ch].step[i].advCycleCount
			= myPs->testCond.step[i].reference[0].cycleCount;
		myData->mData.testCond[bd][ch].step[i].endDeltaV
			= myPs->testCond.step[i].reference[0].endDeltaV;
		myData->mData.testCond[bd][ch].step[i].endSoc
			= myPs->testCond.step[i].reference[0].endSoc;
		myData->mData.testCond[bd][ch].step[i].socCapStepNo
			= myPs->testCond.step[i].reference[0].socCapStepNo;
//		myData->mData.testCond[bd][ch].step[i].endP
//			= myPs->testCond.step[i].reference[0].endWatt;
		myData->mData.testCond[bd][ch].step[i].endWh
			= myPs->testCond.step[i].reference[0].endWattHour;
		myData->mData.testCond[bd][ch].step[i].startTemp
			= (long)(myPs->testCond.step[i].reference[0].startTemp * 100);
		myData->mData.testCond[bd][ch].step[i].endTemp
			= (long)(myPs->testCond.step[i].reference[0].endTemp * 100);
		myData->mData.testCond[bd][ch].step[i].tempType
			= myPs->testCond.step[i].reference[0].tempType;
		myData->mData.testCond[bd][ch].step[i].tempDir
			= myPs->testCond.step[i].reference[0].tempDir;
		myData->mData.testCond[bd][ch].step[i].endTempGoto
			= myPs->testCond.step[i].reference[0].endTempGoto;
		myData->mData.testCond[bd][ch].step[i].endSocGoto
			= myPs->testCond.step[i].reference[0].endSocGoto;
		myData->mData.testCond[bd][ch].step[i].gotoCycleCount
			= myPs->testCond.step[i].reference[0].gotoCycleCount;
		//110215 kji
		#if NETWORK_VERSION > 4101
			#if VENDER != 2 //NOT SDI
		myData->mData.testCond[bd][ch].step[i].cycleEndStepSave
			= myPs->testCond.step[i].reference[0].cycleEndStepSave;
		myData->mData.testCond[bd][ch].step[i].integralInit
			= myPs->testCond.step[i].reference[0].integralInit;
			#endif
		#endif

		//save condition
		time = myPs->testCond.step[i].record.time;
		if(time == 0) {
			myData->mData.testCond[bd][ch].step[i].saveDt = 0;
		} else {
			myData->mData.testCond[bd][ch].step[i].saveDt = time;
		}
		myData->mData.testCond[bd][ch].step[i].saveDv
			= myPs->testCond.step[i].record.deltaV;
		myData->mData.testCond[bd][ch].step[i].saveDi
			= myPs->testCond.step[i].record.deltaI;
		myData->mData.testCond[bd][ch].step[i].saveDtemp
			= myPs->testCond.step[i].record.deltaT;
		myData->mData.testCond[bd][ch].step[i].saveDp
			= myPs->testCond.step[i].record.deltaP;

		myData->mData.testCond[bd][ch].step[i].capacitance_v1
			= myPs->testCond.step[i].edlc.capacitanceV1;
		myData->mData.testCond[bd][ch].step[i].capacitance_v2
			= myPs->testCond.step[i].edlc.capacitanceV2;
		myData->mData.testCond[bd][ch].step[i].z_t1 = 0;
		if(myData->mData.testCond[bd][ch].step[i].type == STEP_Z) {
			myData->mData.testCond[bd][ch].step[i].z_t2
				= myPs->testCond.step[i].reference[0].endT;
		} else {
			myData->mData.testCond[bd][ch].step[i].z_t2
				= myPs->testCond.step[i].reference[0].endT;
			if(myData->mData.testCond[bd][ch].step[i].z_t2 < 0)
				myData->mData.testCond[bd][ch].step[i].z_t2 = 0;
		}
		myData->mData.testCond[bd][ch].step[i].lc_t1
			= myPs->testCond.step[i].edlc.startT_LC;
		myData->mData.testCond[bd][ch].step[i].lc_t2
			= myPs->testCond.step[i].edlc.endT_LC;

		//grade
		memcpy((char *)&myData->mData.testCond[bd][ch].step[i].grade[0],
			(char *)&myPs->testCond.step[i].grade[0],
			sizeof(S_TEST_COND_GRADE)*MAX_GRADE_ITEM);

		#ifdef _END_COMPARE_GOTO
		//End Goto LJS
		for(k=0; k< MAX_COMP_GOTO; k++){
		myData->mData.testCond[bd][ch].step[i].endCompGoto[k].type
			= myPs->testCond.step[i].endCompGoto[k].type;
		myData->mData.testCond[bd][ch].step[i].endCompGoto[k].value
			= myPs->testCond.step[i].endCompGoto[k].value;
		myData->mData.testCond[bd][ch].step[i].endCompGoto[k].sign
			= myPs->testCond.step[i].endCompGoto[k].sign;
		myData->mData.testCond[bd][ch].step[i].endCompGoto[k].gotoStepNo
			= myPs->testCond.step[i].endCompGoto[k].gotoStepNo;
		}
		#endif
		
		//step fault condition
		myData->mData.testCond[bd][ch].step[i].faultUpperV
			= myPs->testCond.step[i].faultUpperV;
		myData->mData.testCond[bd][ch].step[i].faultLowerV
			= myPs->testCond.step[i].faultLowerV;
		myData->mData.testCond[bd][ch].step[i].faultUpperI
			= myPs->testCond.step[i].faultUpperI;
		myData->mData.testCond[bd][ch].step[i].faultLowerI
			= myPs->testCond.step[i].faultLowerI;
		myData->mData.testCond[bd][ch].step[i].faultUpperC
			= myPs->testCond.step[i].faultUpperC;
		myData->mData.testCond[bd][ch].step[i].faultLowerC
			= myPs->testCond.step[i].faultLowerC;
		myData->mData.testCond[bd][ch].step[i].faultUpperZ
			= myPs->testCond.step[i].faultUpperZ;
		myData->mData.testCond[bd][ch].step[i].faultLowerZ
			= myPs->testCond.step[i].faultLowerZ;
		myData->mData.testCond[bd][ch].step[i].faultUpperTemp
			= myPs->testCond.step[i].faultUpperTemp;
		myData->mData.testCond[bd][ch].step[i].faultLowerTemp
			= myPs->testCond.step[i].faultLowerTemp;
		myData->mData.testCond[bd][ch].step[i].faultUpperTemp_restart
			= (long)(myPs->testCond.step[i].faultUpperTemp_restart * 100);
		myData->mData.testCond[bd][ch].step[i].faultLowerTemp_restart
			= (long)(myPs->testCond.step[i].faultLowerTemp_restart * 100);
		//190311 lyh add
		myData->mData.testCond[bd][ch].step[i].pauseUpperTemp
			= (long)(myPs->testCond.step[i].pauseUpperTemp * 100);
		myData->mData.testCond[bd][ch].step[i].pauseLowerTemp
			= (long)(myPs->testCond.step[i].pauseLowerTemp * 100);
		//170215 SCH add for DeltaV/I
		myData->mData.testCond[bd][ch].step[i].faultDeltaV
			 = myPs->testCond.step[i].deltaV.lowerValue;
		myData->mData.testCond[bd][ch].step[i].faultDeltaI
			 = myPs->testCond.step[i].deltaI.lowerValue;
		myData->mData.testCond[bd][ch].step[i].faultDeltaV_T
			 = myPs->testCond.step[i].deltaV.time;
		myData->mData.testCond[bd][ch].step[i].faultDeltaI_T
			 = myPs->testCond.step[i].deltaI.time;
#if CHANGE_VI_CHECK == 1
		myData->mData.testCond[bd][ch].step[i].change_V_lower
			 = myPs->testCond.step[i].changeV.lowerValue;
		myData->mData.testCond[bd][ch].step[i].change_V_upper
			 = myPs->testCond.step[i].changeV.upperValue;
		myData->mData.testCond[bd][ch].step[i].change_V
			 = myPs->testCond.step[i].changeV.changeValue;
		myData->mData.testCond[bd][ch].step[i].change_V_time
			 = myPs->testCond.step[i].changeV.time;
		for(z = 0; z < MAX_CHK_VI_POINT; z++){
			myData->mData.testCond[bd][ch].step[i].chk_V_lower[z]
				= myPs->testCond.step[i].chk_V.lowerValue[z];
			myData->mData.testCond[bd][ch].step[i].chk_V_upper[z]
				= myPs->testCond.step[i].chk_V.upperValue[z];
			myData->mData.testCond[bd][ch].step[i].chk_V_time[z]
				= myPs->testCond.step[i].chk_V.time[z];
			
			myData->mData.testCond[bd][ch].step[i].chk_I_lower[z]
				= myPs->testCond.step[i].chk_I.lowerValue[z];
			myData->mData.testCond[bd][ch].step[i].chk_I_upper[z]
				= myPs->testCond.step[i].chk_I.upperValue[z];
			myData->mData.testCond[bd][ch].step[i].chk_I_time[z]
				= myPs->testCond.step[i].chk_I.time[z];
		}
#endif
#if END_V_COMPARE_GOTO == 1
		myData->mData.testCond[bd][ch].step[i].endVGoto_upper
			= myPs->testCond.step[i].endVGoto_upper;
		myData->mData.testCond[bd][ch].step[i].endVGoto_lower
			= myPs->testCond.step[i].endVGoto_lower;
		myData->mData.testCond[bd][ch].step[i].endVupper_GotoStep
			= myPs->testCond.step[i].endVupper_GotoStep;
		myData->mData.testCond[bd][ch].step[i].endVlower_GotoStep
			= myPs->testCond.step[i].endVlower_GotoStep;
#endif
		//hun_200219
		#ifdef _SDI_SAFETY_V1 
			myData->mData.testCond[bd][ch].step[i].faultRunTime
			 = myPs->testCond.step[i].faultRunTime;
		#endif
		//200706
		#if CHANGE_VI_CHECK == 1
			myData->mData.testCond[bd][ch].step[i].faultRunTime
			 = myPs->testCond.step[i].faultRunTime;
		#endif
#if CHAMBER_TEMP_HUMIDITY == 1	//kjc_210412
		myData->mData.testCond[bd][ch].step[i].cham_sync_T
			= myPs->testCond.step[i].reference[0].cham_sync_T;
		myData->mData.testCond[bd][ch].step[i].cham_temp
			= myPs->testCond.step[i].reference[0].cham_temp * 100;
		myData->mData.testCond[bd][ch].step[i].cham_humid
			= myPs->testCond.step[i].reference[0].cham_humid * 100;
		myData->mData.testCond[bd][ch].step[i].ch_temp
			= myPs->testCond.step[i].reference[0].ch_temp * 100;
		myData->mData.testCond[bd][ch].step[i].cham_temp_dev
			= myPs->testCond.step[i].reference[0].cham_temp_dev * 100;
		myData->mData.testCond[bd][ch].step[i].cham_humid_dev
			= myPs->testCond.step[i].reference[0].cham_humid_dev * 100;
		myData->mData.testCond[bd][ch].step[i].ch_temp_dev
			= myPs->testCond.step[i].reference[0].ch_temp_dev * 100;
		myData->mData.testCond[bd][ch].step[i].cham_temp_sig
			= myPs->testCond.step[i].reference[0].cham_temp_sig;
		myData->mData.testCond[bd][ch].step[i].cham_humid_sig
			= myPs->testCond.step[i].reference[0].cham_humid_sig;
		myData->mData.testCond[bd][ch].step[i].ch_temp_sig
			= myPs->testCond.step[i].reference[0].ch_temp_sig;

		if(myData->mData.testCond[bd][ch].step[i].cham_temp_sig == 0 &&
			myData->mData.testCond[bd][ch].step[i].cham_humid_sig == 0 &&
			myData->mData.testCond[bd][ch].step[i].ch_temp_sig == 0){
		}else{
			myData->bData[bd].cData[ch].misc.stepSyncFlag = 1;
		}
#endif
		//Discharge, Z step condition
		if(myData->mData.testCond[bd][ch].step[i].type == STEP_DISCHARGE
			|| myData->mData.testCond[bd][ch].step[i].type == STEP_Z) {
			myData->mData.testCond[bd][ch].step[i].refI *= (-1);
			myData->mData.testCond[bd][ch].step[i].faultUpperI *= (-1);
			myData->mData.testCond[bd][ch].step[i].faultLowerI *= (-1);
			myData->mData.testCond[bd][ch].step[i].endC
				= labs(myData->mData.testCond[bd][ch].step[i].endC); 
			myData->mData.testCond[bd][ch].step[i].endI
				= labs(myData->mData.testCond[bd][ch].step[i].endI);
			myData->mData.testCond[bd][ch].step[i].endP
				= labs(myData->mData.testCond[bd][ch].step[i].endP); 
			myData->mData.testCond[bd][ch].step[i].endWh
				= labs(myData->mData.testCond[bd][ch].step[i].endWh); 
			if(myData->mData.config.hwSpec == S_5V_200A_75A_15A_AD2) {
				myData->mData.testCond[bd][ch].step[i].rangeV = RANGE2 - 1;
			}
		}

		if(myData->mData.testCond[bd][ch].step[i].type == STEP_LOOP) {
			parallel_cycle = 0; //kjg_180521
		}
		//210205	
#if CAPACITY_CONTROL == 1
		for(j=0; j<2; j++){
		myData->mData.testCond[bd][ch].step[i].endCycleCapaRate[j] 
			= myPs->testCond.step[i].reference[0].endCycleCapaRate[j];
		myData->mData.testCond[bd][ch].step[i].endCycleCurrentRate[j] 
			= myPs->testCond.step[i].reference[0].endCycleCurrentRate[j];	
		myData->mData.testCond[bd][ch].step[i].endCycleCapaStepNo[j] 
			= myPs->testCond.step[i].reference[0].endCycleCapaStepNo[j];
		myData->mData.testCond[bd][ch].step[i].endCycleCurrentStepNo[j] 
			= myPs->testCond.step[i].reference[0].endCycleCurrentStepNo[j];
		myData->mData.testCond[bd][ch].step[i].endCycleCapaSign[j] 
			= myPs->testCond.step[i].reference[0].endCycleCapaSign[j];
		myData->mData.testCond[bd][ch].step[i].endCycleCurrentSign[j] 
			= myPs->testCond.step[i].reference[0].endCycleCurrentSign[j];
		}
		myData->mData.testCond[bd][ch].step[i].endCycleCapaGoto 
			= myPs->testCond.step[i].reference[0].endCycleCapaGoto;
		myData->mData.testCond[bd][ch].step[i].endCycleCurrentGoto 
			= myPs->testCond.step[i].reference[0].endCycleCurrentGoto;
		myData->mData.testCond[bd][ch].step[i].UseCheckCapaFlag 
			= myPs->testCond.step[i].reference[0].UseCheckCapaFlag;
		myData->mData.testCond[bd][ch].step[i].CycleCapaCount 
			= myPs->testCond.step[i].reference[0].CycleCapaCount;
		for(j=0; j<10; j++){			
			myData->mData.testCond[bd][ch].step[i].C_Rate_Persent[j] 
				= myPs->testCond.step[i].reference[0].C_Rate_Persent[j];		
			myData->mData.testCond[bd][ch].step[i].C_Rate_stepNo[j] 
				= myPs->testCond.step[i].reference[0].C_Rate_stepNo[j];			
			myData->mData.testCond[bd][ch].step[i].C_Rate_Sign[j] 
				= myPs->testCond.step[i].reference[0].C_Rate_Sign[j];
		}
#endif
#if GAS_DATA_CONTROL == 1
		myData->mData.testCond[bd][ch].step[i].endGasTVOC
			= myPs->testCond.step[i].reference[0].endGasTVOC;
		myData->mData.testCond[bd][ch].step[i].endGasECo2
			= myPs->testCond.step[i].reference[0].endGasECo2;
		myData->mData.testCond[bd][ch].step[i].endGasTVOC_Goto
			= myPs->testCond.step[i].reference[0].endGasTVOC_Goto;
		myData->mData.testCond[bd][ch].step[i].endGasECo2_Goto
			= myPs->testCond.step[i].reference[0].endGasECo2_Goto;

		myData->mData.testCond[bd][ch].step[i].faultUpper_GasTVOC
			= myPs->testCond.step[i].faultUpper_GasTVOC;
		myData->mData.testCond[bd][ch].step[i].faultLower_GasTVOC
			= myPs->testCond.step[i].faultLower_GasTVOC;
		myData->mData.testCond[bd][ch].step[i].faultUpper_GasECo2
			= myPs->testCond.step[i].faultUpper_GasECo2;
		myData->mData.testCond[bd][ch].step[i].faultLower_GasECo2
			= myPs->testCond.step[i].faultLower_GasECo2;
#endif
		//211025 lyhw add for Ulsan SDI jig TimeOut Fault
		myData->mData.testCond[bd][ch].step[i].jigPressTimeOutFlag
			= myPs->testCond.step[i].reference[0].jigPressTimeOutFlag;
			
#ifdef _ULSAN_SDI_SAFETY
		myData->mData.testCond[bd][ch].step[i].humpSet_T
			= myPs->testCond.step[i].humpSet_T;
		myData->mData.testCond[bd][ch].step[i].humpSet_I
			= myPs->testCond.step[i].humpSet_I;
#endif
#ifdef _GROUP_ERROR
		myData->mData.testCond[bd][ch].step[i].group_StartVoltage
			= myPs->testCond.step[i].group_StartVoltage;
		myData->mData.testCond[bd][ch].step[i].group_CheckTime
			= myPs->testCond.step[i].group_CheckTime;
		myData->mData.testCond[bd][ch].step[i].group_DeltaVoltage
			= myPs->testCond.step[i].group_DeltaVoltage;
		myData->mData.testCond[bd][ch].step[i].group_EndFaultTime
			= myPs->testCond.step[i].group_EndFaultTime;
#endif
#ifdef _EQUATION_CURRENT	//211111
		myData->mData.testCond[bd][ch].step[i].equation_current_flag 
			= myPs->testCond.step[i].reference[0].equation_current_flag;
		myData->mData.testCond[bd][ch].step[i].Max_Current_EQU
			= myPs->testCond.step[i].reference[0].Max_Current_EQU;
		myData->mData.testCond[bd][ch].step[i].variable[0] 
			= myPs->testCond.step[i].reference[0].variable[0];
		myData->mData.testCond[bd][ch].step[i].variable[1] 
			= myPs->testCond.step[i].reference[0].variable[1];
		myData->mData.testCond[bd][ch].step[i].variable[2] 
			= myPs->testCond.step[i].reference[0].variable[2];
		myData->mData.testCond[bd][ch].step[i].variable[3] 
			= myPs->testCond.step[i].reference[0].variable[3];
#endif
	}
}
//20190801 oys add : Convert C_rate to I value process
long Convert_C_rate_to_I_value(int bd, int ch, long refI) {
	long rtn = 0, cellCapacity;
	
	if(myData->bData[bd].cData[ch].misc.cRateUseFlag == P0) return refI;

	cellCapacity = myData->mData.testCond[bd][ch].step[0].endP;
	if(cellCapacity == 0) return refI;

	rtn	= cellCapacity * ((float)refI / 1000);

	return rtn;
}
int GradeCodeCheck(int bd, int ch, unsigned long stepNo, long val)
{
	unsigned char gradeStepCount;
	int i, code;
	long lower, upper;

	gradeStepCount = myData->mData.testCond[bd][ch].step[stepNo].grade[0]
		.gradeStepCount;

	code = 0;
	for(i=0; i < gradeStepCount; i++) {
		lower = myData->mData.testCond[bd][ch].step[stepNo].grade[0]
			.gradeStep[i].lowerValue;
		upper = myData->mData.testCond[bd][ch].step[stepNo].grade[0]
			.gradeStep[i].upperValue;
		if(lower <= val && upper > val) {
			code = (int)myData->mData.testCond[bd][ch].step[stepNo].grade[0]
				.gradeStep[i].gradeCode;
			return code;
		}
	}
	return code;
}

void StateChange_Pause(int num)
{
	int bd, ch, i;
	int idx,sendNum;
	if(myPs->config.state_change == 1) {
		myPs->signal[MAIN_SIG_NET_CONNECTED] = P0;
			for(i=0; i < myData->mData.config.installedCh; i++) {
				bd = myData->CellArray1[i].bd;
				ch = myData->CellArray1[i].ch;
				if(myData->bData[bd].cData[ch].op.state == C_RUN) {
					myData->bData[bd].cData[ch].signal[C_SIG_NETWORK_CONNECT_ERROR] = P1;
				//	myData->bData[bd].cData[ch].signal[C_SIG_PAUSE] = P1;
					idx = myData->save_msg[0].read_idx[bd][ch];
					if(myData->save_msg[0].val[idx][bd][ch].chData.resultIndex < MAX_RETRANS_DATA) {
						sendNum = myData->save_msg[0].
							val[idx][bd][ch].chData.resultIndex;
					} else { 
						sendNum = MAX_RETRANS_DATA;
					}
					if(idx < sendNum) {
						myData->save_msg[0].read_idx[bd][ch] += 
							(MAX_SAVE_MSG - MAX_RETRANS_DATA);
						myData->save_msg[0].count[bd][ch] += sendNum;

					} else {
						myData->save_msg[0].read_idx[bd][ch] -= sendNum;
						myData->save_msg[0].count[bd][ch] += sendNum;

					}
				}
			}
	} else if(myPs->config.state_change == 2) {
	//110402 kji w
	//	send_msg(MAIN_TO_MODULE, MSG_MAIN_MODULE_SAVE_MSG_FLAG, 0, 1); //stop
		if(myData->mData.config.function[F_DISCONNECT_DAY] != P0){
			myData->mData.mainStateCheckFlag = P1;
		}
	//210415 LJS for MASTER RECIPE
	#ifdef _SDI_SAFETY_V2
		for(i=0; i < myData->mData.config.installedCh; i++) {
			bd = myData->CellArray1[i].bd;
			ch = myData->CellArray1[i].ch;
			if(myData->bData[bd].cData[ch].op.state == C_RUN) {
				if(myData->bData[bd].cData[ch].misc.MasterFlag != 0){
					myData->bData[bd].cData[ch].
							signal[C_SIG_M_RECIPE_CONNECT_ERROR] = P1;
					idx = myData->save_msg[0].read_idx[bd][ch];
					if(myData->save_msg[0].val[idx][bd][ch].
							chData.resultIndex < MAX_RETRANS_DATA) {
						sendNum = myData->save_msg[0].
							val[idx][bd][ch].chData.resultIndex;
					} else { 
						sendNum = MAX_RETRANS_DATA;
					}
					if(idx < sendNum) {
						myData->save_msg[0].read_idx[bd][ch] +=
								(MAX_SAVE_MSG - MAX_RETRANS_DATA);
						myData->save_msg[0].count[bd][ch] += sendNum;
					} else {
						myData->save_msg[0].read_idx[bd][ch] -= sendNum;
						myData->save_msg[0].count[bd][ch] += sendNum;
					}
				}
			}
		}
	#endif
	#ifdef _TRACKING_MODE
		for(i=0; i < myData->mData.config.installedCh; i++) {
			bd = myData->CellArray1[i].bd;
			ch = myData->CellArray1[i].ch;
			if(myData->bData[bd].cData[ch].op.state == C_RUN) {
				if(myData->bData[bd].cData[ch].misc.connect_check_flag != 0){
					myData->bData[bd].cData[ch].signal[C_SIG_SELECT_CH_CONNECT_ERROR] = P1;
					idx = myData->save_msg[0].read_idx[bd][ch];
					if(myData->save_msg[0].val[idx][bd][ch].
							chData.resultIndex < MAX_RETRANS_DATA) {
						sendNum = myData->save_msg[0].
							val[idx][bd][ch].chData.resultIndex;
					} else { 
						sendNum = MAX_RETRANS_DATA;
					}
					if(idx < sendNum) {
						myData->save_msg[0].read_idx[bd][ch] += 
							(MAX_SAVE_MSG - MAX_RETRANS_DATA);
						myData->save_msg[0].count[bd][ch] += sendNum;
					} else {
						myData->save_msg[0].read_idx[bd][ch] -= sendNum;
						myData->save_msg[0].count[bd][ch] += sendNum;
					}
				}
			}
		}
	#endif
	}
	close(myPs->config.network_socket);
	userlog(DEBUG_LOG, psName, "network communication error : %d\n", num);
}
// 131228 oys w : real time
#if REAL_TIME == 1 
void Update_RealTime(char *real_time)
{
	char cmd[32];
	// Linux Clock Update
	memset(cmd, 0, sizeof cmd);
	strcpy(cmd, "date -s '");
	strncat(cmd, real_time, 19);
	strcat(cmd, "'");
	system(cmd);
	// SBC CMOS Clock Update
	// 210625 lyhw
	userlog(DEBUG_LOG, psName, "Real Time Update Complete\n");
// 160325 oys modify
/*
	memset(cmd, 0, sizeof cmd);
	strcpy(cmd, "clock -w");
	system(cmd);
*/
	return;
//	userlog(DEBUG_LOG, psName, "Update real_time : %s\n", real_time);
}
#endif

//******************************************************************************
// How to use : Text Color change
// 
// Default setting
// printf("%c[0m", 27);
// 
// color change
// ex) printf("%c[1;33m",27);			<< yellow
// 33 is color code
//
// color code list
// 30 -> gray
// 31 -> red
// 32 -> green
// 33 -> yellow
// 34 -> blue
// 35 -> light purple
// 36 -> bright blue
// 37 -> bright white
//
// blink setting
// ex) printf("%c[1;5;33m", 27); 		<< blink yellow
// ****************************************************************************o
void EquipmentInfoPrint(void)
{
	int i, red = 31, green = 32;
	char version[18];
	sleep(2);
	if(myPs->signal[MAIN_SIG_NET_CONNECTED] == P0){
		for(i=0; i < 80; i++) {
			printf("=");
		}
		printf("\n\tSYSTEM TYPE \t\t    : ");
		printf("%c[1;%dm", 27, green);
#if SYSTEM_TYPE == 1
		printf("%c[1;5;%dm", 27, red); //BLINK RED
		printf("VMWare Workstation\n");
#else
		printf("%c[1;%dm", 27, green);
		switch(myData->AppControl.config.systemType){
			case 0:
				printf("FORMATION\n");
				break;
			case 1:
				printf("IR_OCV\n");
				break;
			case 2:
				printf("AGING\n");
				break;
			case 3:
				printf("GRADER\n");
				break;
			case 4:
				printf("SELECTOR\n");
				break;
			case 5:
				printf("OCV\n");
				break;
			case 6:
				printf("CYCLER (Linear)\n");
				break;
			case 7:
				printf("CYCLER (PCU)\n");
				break;
			case 8:
				printf("CYCLER (CAN)\n");
				break;
			default:
				printf("UNKNOWN\n");
				break;
		}
#endif
		printf("%c[0m", 27); //default
		printf("\tMODEL NAME \t\t    : ");
		printf("%c[1;%dm", 27, green);
		printf("%s", myData->AppControl.config.modelName);
		printf("_M%d\n", myData->AppControl.config.moduleNo);
		printf("%c[0m", 27); //default
		printf("\tSBC TYPE \t\t    : ");
		printf("%c[1;%dm", 27, green);
		switch(myData->AppControl.config.sbcType){
			case 1:
				printf("HS_6637\n");
				break;
			case 2:
				printf("WEB_6580\n");
				break;
			case 3:
				printf("HS_4020\n");
				break;
			case 4:
				printf("WAFER_E669\n");
				break;
			case 5:
				printf("WAFER_MARK\n");
				break;
			case 6:
				printf("EM104_A5362\n");
				break;
			default:
				printf("%d : UNKNOWN\n", myData->AppControl.config.sbcType);
				break;
		}

//START ETC PARAMETER OPTIONS

		printf("%c[0m", 27); //default
		printf("\tRT SCAN PERIOD \t\t    : ");
		printf("%c[1;%dm", 27, green);
		switch(myData->mData.config.rt_scan_type){
			case 0 :
				printf("100ms\n");
				break;
			case 1:
				printf("50ms\n");
				break;
			case 2:
				printf("25ms\n");
				break;
			case 3:
				printf("20ms\n");
				break;
			case 4:
				printf("100ms_2\n");
				break;
			case 5:
				printf("10ms\n");
				break;
			case 6:
				printf("100ms_CAN\n");
				break;
			default:
				printf("[%d] : UNKNOWN\n", myData->mData.config.rt_scan_type);
				break;
		}
		printf("%c[0m", 27); //default
		printf("\tPARALLEL MODE \t\t    : ");
		printf("%c[1;%dm", 27, green);
		switch(myData->mData.config.parallelMode){
			case 0 :
				printf("SINGLE\n");
				break;
			case 1 :
				printf("PARALLEL\n");
				break;
			case 2 :
				printf("CYCLE PARALLEL\n");
				break;
			default:
				printf("[%d] : UNKNOWN\n", myData->mData.config.parallelMode);
				break;
		}
		printf("%c[0m", 27); //default

		if(myData->AppControl.config.debugType != P0) {
			printf("%c[0m", 27); //default
			printf("\tDEBUG MODE TYPE \t    : ");
			printf("%c[1;5;%dm", 27, red); //BLINK RED
			switch(myData->AppControl.config.debugType) {
				case P1:
					printf("VOLTAGE FIXATION MODE (3.5V)\n");
					break;
				case P2:
					printf("SIMULATION MODE\n");
					break;
				default:
					break;
			}
		}
		printf("%c[0m", 27); //default

//END ETC PARAMETER OPTIONS

		if(NETWORK_VERSION == myPs->config.protocol_version){
			printf("\tSBC Program NETWORK VERSION : ");
			printf("%c[1;%dm", 27, green);
			printf("%d(%x)\n", NETWORK_VERSION, NETWORK_VERSION);
			printf("%c[0m", 27); //default
			printf("\tMainClient  NETWORK VERSION : ");
			printf("%c[1;%dm", 27, green);
			printf("%d(%x)\n",
					myPs->config.protocol_version,
					myPs->config.protocol_version);
			printf("%c[0m", 27); //default
		}else{
			printf("\tSBC Program NETWORK VERSION : ");
			printf("%c[1;5;%dm", 27, red); //BLINK RED
			printf("%d(%x) <-- Check Please!\n",
					NETWORK_VERSION, NETWORK_VERSION);
			printf("%c[0m", 27); //default
			printf("\tMainClient  NETWORK VERSION : ");
			printf("%c[1;5;%dm", 27, red); //BLINK RED
			printf("%d(%x) <-- Check Please!\n",
					myPs->config.protocol_version,
					myPs->config.protocol_version);
			printf("%c[0m", 27); //default
		}
		printf("\tOperation PC IP \t    : ");
		printf("%c[1;%dm", 27, green);
		printf("%s\n", myPs->config.ipAddr);
		printf("%c[0m", 27); //default
		
		printf("\tControl SBC IP \t\t    : ");
		printf("%c[1;%dm", 27, green);
		printf("%s\n", myData->AppControl.misc.sbcIpAddr);
		printf("%c[0m", 27); //default
		
		for(i=0; i < 80; i++) {
			printf("=");
		}
		printf("\n");
		printf("%c[1;34m",27);	//blue
		printf("\t///");
		printf("%c[0m",27);	//default
		printf("PNE SOLUTION CO., LTD.\n");
		printf("\tSBC PROGRAM VERSION \t    : ");
		printf("%c[1;31m",27);	//red
		sprintf(version,"%d%d%d%d-S%d%d-R%d%d%d-N%d%d",
				(MAIN_P_VER1/1000), ((MAIN_P_VER1%1000)/100),
				(((MAIN_P_VER1%1000)%100)/10), ((((MAIN_P_VER1%1000)%100)%10)/1),
				(SUB_P_VER/10), ((SUB_P_VER%10)/1), (MAIN_R_VER/100), ((MAIN_R_VER%100)/10),
				(((MAIN_R_VER%100)%10)/1), (SUB_R_VER/10), ((SUB_R_VER%10)/1));
		/*
		sprintf(version,"%d%d.%d%d.%d%d.%d%d%d%d%d%d.%d%d",
											(VENDER/10), (VENDER%10),
											PROGRAM_VERSION1, PROGRAM_VERSION2,
											PROGRAM_VERSION3, PROGRAM_VERSION4,
											RELEASE_VERSION1, RELEASE_VERSION2,
											RELEASE_VERSION3, RELEASE_VERSION4,
											RELEASE_VERSION5, RELEASE_VERSION6,
											RELEASE_VERSION7, RELEASE_VERSION8);
		*/
		printf("%s\n", version);
		printf("%c[0m",27);	//default
		printf("\tSBC PARAMETER VERSION \t    : ");
		printf("%c[1;%dm", 27, red);
		printf("%d\n", myData->AppControl.config.versionNo);
		printf("%c[0m", 27); //default
		for(i=0; i < 80; i++) {
			printf("=");
		}
		if(myData->AppControl.config.debugType != 0){
			printf("\n\tFirmware Manager History\n\tKHK, KJI, JYK, PJY(PMS), OYS, LYH, NAM, SCH, PTH, LJS, KJH and KJC.\n");
			for(i=0; i < 80; i++) {
				printf("=");
			}
		}
		printf("\n");
	}
}
