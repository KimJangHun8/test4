#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <termios.h>
#include <fcntl.h>
#include "../../INC/datastore.h"
#include "userlog.h"
#include "common_utils.h"
#include "local_utils.h"
#include "message.h"
#include "StandardInput.h"
#include "main.h"

volatile S_SYSTEM_DATA *myData;
volatile S_APP_CONTROL *myPs; //my process : AppControl
char psName[16];

int main(int argc, char *argv[])
{
    int		rtn, rtn2, saveFlag, tmp, i;
    struct	timeval tv;
	char fileName[128], temp[128], flag[0], value = 0, key = 0;
    fd_set	rfds;
	FILE *fp1;

	if(argc != 2) {
		printf("System loader start fail %d\n", argc);
		return 0;
	}
//140707 lyh w : forceStart duplication check
	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/cycler_data/config/parameter/systemMemory");
	if((fp1 = fopen(fileName,"r" )) == NULL) {
		userlog(DEBUG_LOG, psName, "systemMemory file read error\n");
		system("touch /root/cycler_data/config/parameter/systemMemory");
		userlog(DEBUG_LOG, psName, "systemMemory file create\n");
		return -1;
	}
	memset(flag, 0x00, sizeof flag);
	tmp = fscanf(fp1, "%s", flag);
	rtn = atoi(flag);
	fclose(fp1);

//boot on start compare (2:duplication checked 1:boot, 0:force)
	rtn2 = atoi(argv[1]);

	if(rtn2 != 1){
		if(rtn == 0){
			memset(fileName, 0x00, sizeof(fileName));
			strcpy(fileName, "/root/cycler_data/config/parameter/systemMemory");
			if((fp1 = fopen(fileName, "w+")) == NULL) {
				userlog(DEBUG_LOG, psName, "systemMemory file read error\n");
				system("touch /root/cycler_data/config/parameter/systemMemory");
				userlog(DEBUG_LOG, psName, "systemMemory file create\n");
				return -1;
			}
			fprintf(fp1, "2\n");
			fclose(fp1);
			printf("AppControl Program Running......Warning\n");
			printf("AppControl Program Running......Warning\n");
			printf("AppControl Program Running......Warning\n");
			return -1;
		}
		if(rtn == 2){
			rtn = DieCheck_Process("mcts");
			if(rtn == 1 || rtn == 2){
				printf("restart program\n");
				Kill_Process("mcts");
			}
		}
	}
//add end

// forceStart start check start
	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/cycler_data/config/parameter/AppControl_Config");
	// /root/cycler_data/config/parameter/AppControl_Config
    if((fp1 = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "AppControl_Config file read error\n");
		system("cp ../Config_backup/AppControl_Config /root/cycler_data/config/parameter/");
		userlog(DEBUG_LOG, psName, "AppControl_Config file copy\n");
    	if((fp1 = fopen(fileName, "r")) == NULL) {
			return -1;
		}
	}
	for(i = 0; i < 8; i++) {
 	  	tmp = fscanf(fp1, "%s", temp);
	}
	memset(temp, 0x00, sizeof temp);
	tmp = fscanf(fp1, "%s", temp);
	value = (unsigned char)atoi(temp);
    fclose(fp1);

	rtn = atoi(argv[1]); //boot on start compare (1:boot, force:0)	
	if(rtn == 0) {
		if(value == 0) {
#if SYSTEM_TYPE == 0
			printf("\nSYSTEM TYPE is [SBC] >>> Press any key to skip forceStart...\n");
			for(i = 0; i < 3; i++){
				printf("%d\n",3-i);
				sleep(1);
				key = kbhit();
				if(key == 1) break;
			}
			if(key == 1) {
				return -1;
			}
#endif
			printf("Start Program...\n");
		}
	} else {
		if(value == 0) {
			return -1;
		} else {
#if SYSTEM_TYPE == 1
			printf("\nPress any key to skip bootOnStart...\n");
			for(i = 0; i < 3; i++){
				printf("%d\n",3-i);
				sleep(1);
				key = kbhit();
				if(key == 1) break;
			}
			if(key == 1) {
				return -1;
			}
#endif
			printf("Start Program...\n");
		}
	}
// forceStart start check end


	Unload_Module();
	Load_Module();

	rtn = Initialize(rtn);
	if(rtn < 0) {
		if(rtn <= -10) {
			userlog(DEBUG_LOG, psName, "System initialize fail %d\n", rtn);
		} else if(rtn < 0) {
			printf("System initialize fail %d\n", rtn);
			Close_SystemMemory();
			system("rmmod mbuff");
		}
		saveFlag = myPs->misc.saveFlag;
		Close_SystemMemory();
		system("rmmod mbuff");
		memset(fileName, 0x00, sizeof(fileName));
		strcpy(fileName, "/root/cycler_data/config/parameter/systemMemory");
		// /root/cycler_data/config/parameter/systemMemory
		if((fp1 = fopen(fileName,"w" )) < 0) {
			userlog(DEBUG_LOG, psName,
				"Can not open system Memory saveFlag file\n");
			system("rm -rf /root/cycler_data/config/parameter/systemMemory");
			system("touch /root/cycler_data/config/parameter/systemMemory");
			userlog(DEBUG_LOG, psName, "systemMemory file delete -> create\n");
			return 0;
		} else {

			if(saveFlag == 1) {
				fprintf(fp1,"0\n");
			} else {
				fprintf(fp1,"1\n");
			}
			fclose(fp1);
		}
		return 0;
	}
    while(myPs->signal[APP_SIG_APP_CONTROL_PROCESS] == P1) {
	   	tv.tv_sec = 1;
	    tv.tv_usec = 0;
	    FD_ZERO(&rfds);
	   	FD_SET(0, &rfds);
			
		rtn = select(1, &rfds, NULL, NULL, &tv);
		//userlog(DEBUG_LOG, psName, "event %d\n", rtn); //kjgd
		if(rtn) {
			StandardInput_Receive();
			AppControl();
		} else {
			AppControl();
		}//110621 oys w : AnalogMeter Process Kill(restart)
		if(myData->AppControl.config.debugType == P0){	//hun_211020
			if((myData->mData.misc.timer_1sec - myData->serialCheckTime) > 60){
				myData->serialCheckTime = myData->mData.misc.timer_1sec;
				if(myPs->signal[APP_SIG_ANALOG_METER_PROCESS] == P1){
					Kill_Process("AnalogMeter");
					if(myPs->signal[APP_SIG_CALI_METER_PROCESS] == P1)
						Kill_Process("CaliMeter");
				}
			}
			if((myData->mData.misc.timer_1sec - myData->serialCheckTime2) > 60){
				myData->serialCheckTime2 = myData->mData.misc.timer_1sec;
				if(myPs->signal[APP_SIG_ANALOG_METER2_PROCESS] == P1){
					Kill_Process("AnalogMeter2");
					if(myPs->signal[APP_SIG_CALI_METER_PROCESS] == P1)
						Kill_Process("CaliMeter");
				}
			}
		}
	}
	CloseAppControl();
    return 0;
}

int kbhit(void)
{
	struct termios oldt, newt;
	int ch;
	int oldf;

	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
	fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

	ch = getchar();

	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	fcntl(STDIN_FILENO, F_SETFL, oldf);

	if(ch != EOF)
	{
		ungetc(ch, stdin);
		return 1;
	}
	return 0;
}

int Initialize(int bootOnStartComp)
{
	int load = 0, i, rtn;
	char cmd[128];
	
	Kill_Process("MainClient");
	Kill_Process("DataSave");
	Kill_Process("CaliMeter");
	Kill_Process("CaliMeter2");
	Kill_Process("AnalogMeter");
	Kill_Process("AnalogMeter2");
	Kill_Process("FADM");
	Kill_Process("AutoUpdate");
	Kill_Process("DAQ_Client");
	Kill_Process("CoolingControl");
	
	//190823 oys add : Unload processes using mbuff
	rtn = unload_process_using_mbuff();
	if(rtn > 0) {
		for(i = 0; i < rtn; i++) {
			Close_SystemMemory();
			usleep(100000);
		}
	}
	
	if(Open_SystemMemory(1) < 0) return -1;

	myPs = &(myData->AppControl);

	if(Read_SystemMemory("systemMemory") < 0)
		return -2; //saved system memory data load
	Init_SystemMemory();

//190624 oys add : Delete log data a year ago
/*
	memset(cmd, 0x00, sizeof(cmd));
	strcpy(cmd, "find /root/cycler_data/log -name '*.log' -ctime +365 |xargs rm -f");
	system(cmd);
	printf("=== Delete log data a year ago ===\n");
*/
	InitLogfile("DEBUG_LOG", 0);
	InitLogfile("MAIN_LOG", 1);
	InitLogfile("METER_LOG", 2);
	InitLogfile("METER2_LOG", 3);
	InitLogfile("EXT_LOG", 4);

	//190910 oys add
	Update_bashrc();
	//191206 oys add
	Update_proftpd_config();

	if(Read_AppControl_Config(bootOnStartComp) < 0) return -10;
	if(Read_mControl_Config() < 0) return -11;
	if(Read_DIO_Config() < 0) return -12;
	if(Read_Function_Flag() < 0) return -13; //20151115 khk
	if(Read_Calibration_Config() < 0) return -14;
	if(Read_CellArray_A() < 0) return -15;
	if(Read_Addr_Interface() < 0) return -16;
	if(Read_Addr_Main() < 0) return -17;
	if(Read_FaultBitSet_SMPS_OT() < 0) return -18;
	if(Read_ChAttribute() < 0) return -19;
#if CYCLER_TYPE == DIGITAL_CYC
	if(Read_PCU_Config() < 0) return -20;	//180615 add 
#endif
	if(myPs->config.versionNo >= 20101218) {
		if(Read_AuxSetData() < 0) return -21;
	}
	if(myData->mData.config.installedAuxV > 0)
	{
		if(Read_AuxCaliData() < 0) return -22;
		Read_DaqArray();
	}
	if(myData->mData.config.FadBdUse > 0)
	{
		if(Read_FadCaliData() < 0) return -23;
	}
	//110429 load process add kji
//	if(myPs->config.versionNo >= 20110429) { //20200804
	if(Read_LoadProcess() < 0) return -24;
//	}
	if(myData->mData.config.hwSpec == L_5V_150A_R2_P) {
		if(Read_ChCompData() < 0) return -25;
	}

	//pms add for restore memeory 2012.10.31
//	if(Read_Restore_Memory_Config() < 0) return -25;

	if(Read_HwFault_Config() < 0) return -26; //20151115 khk
	
	// 160120 oys add : Error Chamber Use Ch Pause or Unit Shutdown
	if(Read_ChamArray_A() < 0) return -27;
	
	if(Read_Chamber_Motion() < 0) return -28; //2017 sch add
	
	if(Read_LimitUserVI() < 0) return -29; //20171008 sch add

	if(myPs->config.systemType == CYCLER_CAN){
		if(Read_CAN_Config() < 0) return -30;  //20190605 KHK
	}
	if(Read_SBC_IP_Address() < 0) return -31; //20190712 oys
	if(Read_ChamberChNo() < 0) {		//211019 hun
		if(Write_Default_ChamberChNo() < 0){
			return -32;
		}
	}
	if(Read_SwFault_Config() < 0) return -33; //210428 hun
	if(Read_TempFault_Config() < 0) return -34; //210629 
	#ifdef _LGES
	if(Read_LGES_Fault_Config_Update() < 0) return -35;	//hun_211027
	if(Read_LGES_Fault_Config_Version_Check() < 0)	return -36;	//hun_211025
	if(Read_LGES_Fault_Config() < 0)	return -37;	//hun_211025
	if(Read_LGES_Ambient2() < 0)	return -38;	// INC/LGES_Path_Note.txt Check
	if(Read_LGES_code_buzzer_on() < 0)	return -40;	// INC/LGES_Path_Note.txt Check
	if(Read_Semi_Autojig_DCR_Calculate() < 0)	return -43;	// INC/LGES_Path_Note.txt Check
	#endif	
	#ifdef _SDI
	if(Read_SDI_inv_fault_continue() < 0)	return -38;	// INC/SDI_Path_Note.txt Check
	if(Read_SDI_CC_CV_hump_Config() < 0)	return -39;	// INC/SDI_Path_Note.txt Check
	if(Read_SDI_Pause_save_Config() < 0)	return -41;	// INC/SDI_Path_Note.txt Check
	#endif
	#ifdef _DYSON
	if(Read_DYSON_Maintenance_Config() < 0)	return -42; // 220121 JSH
	#endif
	//190723 oys add	
	Config_backup();

//for rt_can communication
	if(myPs->config.systemType == CYCLER_CAN){
		for(i = 0; i < myData->CAN.config.installedCAN; i++) {
			memset(cmd, 0, sizeof cmd);
			sprintf(cmd, "setserial /dev/ttyS1%d uart none", i);
			system(cmd);
		}
	}
//for rt_com communication
/*
	for(i = 0; i < myData->CAN.config.installedCOM; i++) {
		memset(cmd, 0, sizeof cmd);
		sprintf(cmd, "setserial /dev/ttyS%d uart none", i);
		system(cmd);
	}
*/

/*
	//110429 load process add kji
	if(myPs->config.versionNo >= 20110429) {
		load = myPs->loadProcess[LOAD_MAINCLIENT] ; 
	} else {
		load = 1;
	}
*/
	load = myPs->loadProcess[LOAD_MAINCLIENT]; //200715 rewrite 
	if(load){
		if(Load_Process("Load_MainClient", "./", APP_SIG_MAIN_CLIENT_PROCESS) < 0) {
			Close_Process("MainClient", myData->MainClient.misc.processPointer,
				APP_SIG_MAIN_CLIENT_PROCESS);
			return -101;
		}
	}
/*	
	if(myPs->config.versionNo >= 20110429) {
		load = myPs->loadProcess[LOAD_DATASAVE] ; 
	} else {
		load = 1;
	}
*/
	load = myPs->loadProcess[LOAD_DATASAVE]; //200715 rewrite 
	if(load){
		if(Load_Process("Load_DataSave", "./", APP_SIG_DATA_SAVE_PROCESS) < 0) {
			Close_Process("MainClient", myData->MainClient.misc.processPointer,
				APP_SIG_MAIN_CLIENT_PROCESS);
			Close_Process("DataSave", myData->DataSave.misc.processPointer,
				APP_SIG_DATA_SAVE_PROCESS);
			return -102;
		}
	}
/* 
	if(myPs->config.versionNo >= 20110429) {
		load = myPs->loadProcess[LOAD_CALIMETER] ; 
	} else {
		load = 1;
	}
*/
	load = myPs->loadProcess[LOAD_CALIMETER]; //200715 rewrite 
	if(load){
		if(Load_Process("Load_CaliMeter", "./", APP_SIG_CALI_METER_PROCESS) < 0) {
			Close_Process("MainClient", myData->MainClient.misc.processPointer,
				APP_SIG_MAIN_CLIENT_PROCESS);
			Close_Process("DataSave", myData->DataSave.misc.processPointer,
				APP_SIG_DATA_SAVE_PROCESS);
			Close_Process("CaliMeter", myData->CaliMeter.misc.processPointer,
				APP_SIG_CALI_METER_PROCESS);
			return -103;
		}
	}
/*
	if(myPs->config.versionNo >= 20110429) {
		if(myPs->loadProcess[LOAD_CALIMETER2] == P1){
			load = 0;
			myPs->loadProcess[LOAD_ANALOGMETER] = 0; 
		}else{
			load = myPs->loadProcess[LOAD_ANALOGMETER]; 
		}
	} else {
		load = 1;
	}
*/
	if(myPs->loadProcess[LOAD_CALIMETER2] == P1){ //200715 rewrite
		load = 0;
		myPs->loadProcess[LOAD_ANALOGMETER] = 0; 
	}else{
		load = myPs->loadProcess[LOAD_ANALOGMETER]; 
	}
	if(load){
		if(Load_Process("Load_AnalogMeter", "./",
			APP_SIG_ANALOG_METER_PROCESS) < 0) {
			Close_Process("MainClient", myData->MainClient.misc.processPointer,
				APP_SIG_MAIN_CLIENT_PROCESS);
			Close_Process("DataSave", myData->DataSave.misc.processPointer,
				APP_SIG_DATA_SAVE_PROCESS);
			Close_Process("CaliMeter", myData->CaliMeter.misc.processPointer,
				APP_SIG_CALI_METER_PROCESS);
			Close_Process("AnalogMeter", myData->AnalogMeter.misc.processPointer,
				APP_SIG_ANALOG_METER_PROCESS);
			return -104;
		}
	}
/*
	if(myPs->config.versionNo >= 20110429) {
		if(myPs->loadProcess[LOAD_CALIMETER2] == P1){
			load = 0;
			myPs->loadProcess[LOAD_FADM] = 0;
		}else{
			load = myPs->loadProcess[LOAD_FADM] ; 
		}
	} else {
		load = 0;
	}
*/
	if(myPs->loadProcess[LOAD_CALIMETER2] == P1){ //200715 rewrite
		load = 0;
		myPs->loadProcess[LOAD_FADM] = 0;
	}else{
		load = myPs->loadProcess[LOAD_FADM] ; 
	}	
	if(load){
		if(Load_Process("Load_FADM", "./",
			APP_SIG_FADM_PROCESS) < 0) {
			Close_Process("MainClient", myData->MainClient.misc.processPointer,
				APP_SIG_MAIN_CLIENT_PROCESS);
			Close_Process("DataSave", myData->DataSave.misc.processPointer,
				APP_SIG_DATA_SAVE_PROCESS);
			Close_Process("CaliMeter", myData->CaliMeter.misc.processPointer,
				APP_SIG_CALI_METER_PROCESS);
			Close_Process("AnalogMeter", myData->AnalogMeter.misc.processPointer,
				APP_SIG_ANALOG_METER_PROCESS);
			Close_Process("FADM", myData->FADM.misc.processPointer,
				APP_SIG_FADM_PROCESS);
			return -105;
		}
	}
/*
	if(myPs->config.versionNo >= 20110429) {
		if(myPs->loadProcess[LOAD_CALIMETER2] == P1){
			load = 0;
			myPs->loadProcess[LOAD_FNDMETER] = 0; 
		}else{
			load = myPs->loadProcess[LOAD_FNDMETER] ; 
		}
	} else {
		load = 0;
	}
*/
	if(myPs->loadProcess[LOAD_CALIMETER2] == P1){ //200715 rewrite
		load = 0;
		myPs->loadProcess[LOAD_FNDMETER] = 0; 
	}else{
		load = myPs->loadProcess[LOAD_FNDMETER] ; 
	}
	if(load){
		if(Load_Process("Load_FndMeter", "./",
			APP_SIG_FND_METER_PROCESS) < 0) {
			Close_Process("MainClient", myData->MainClient.misc.processPointer,
				APP_SIG_MAIN_CLIENT_PROCESS);
			Close_Process("DataSave", myData->DataSave.misc.processPointer,
				APP_SIG_DATA_SAVE_PROCESS);
			Close_Process("CaliMeter", myData->CaliMeter.misc.processPointer,
				APP_SIG_CALI_METER_PROCESS);
			Close_Process("AnalogMeter", myData->AnalogMeter.misc.processPointer,
				APP_SIG_ANALOG_METER_PROCESS);
			Close_Process("FADM", myData->FADM.misc.processPointer,
				APP_SIG_FADM_PROCESS);
			Close_Process("FndMeter", myData->FndMeter.misc.processPointer,
				APP_SIG_FND_METER_PROCESS);
			return -106;
		}
	}
/*
	if(myPs->config.versionNo >= 20110429) {
		load = myPs->loadProcess[LOAD_CALIMETER2] ; 
	} else {
		load = 0;
	}
*/
	load = myPs->loadProcess[LOAD_CALIMETER2] ; //200715 rewrite
	if(load){
		if(Load_Process("Load_CaliMeter2", "./", APP_SIG_CALI_METER2_PROCESS) < 0) {
			Close_Process("MainClient", myData->MainClient.misc.processPointer,
				APP_SIG_MAIN_CLIENT_PROCESS);
			Close_Process("DataSave", myData->DataSave.misc.processPointer,
				APP_SIG_DATA_SAVE_PROCESS);
			Close_Process("CaliMeter", myData->CaliMeter.misc.processPointer,
				APP_SIG_CALI_METER_PROCESS);
			Close_Process("AnalogMeter", myData->AnalogMeter.misc.processPointer,
				APP_SIG_ANALOG_METER_PROCESS);
			Close_Process("FADM", myData->FADM.misc.processPointer,
				APP_SIG_FADM_PROCESS);
			Close_Process("FndMeter", myData->FndMeter.misc.processPointer,
				APP_SIG_FND_METER_PROCESS);
			Close_Process("CaliMeter2", myData->CaliMeter2.misc.processPointer,
				APP_SIG_CALI_METER2_PROCESS);
			return -107;
		}
	}
/*
	if(myPs->config.versionNo >= 20110429) {
		load = myPs->loadProcess[LOAD_EXTCLIENT] ; 
	} else {
		load = 0;
	}
*/
	load = myPs->loadProcess[LOAD_EXTCLIENT] ; //200715 rewrite
	if(load){
		if(Load_Process("Load_ExtClient", "./", APP_SIG_EXT_CLIENT_PROCESS) < 0) {
			Close_Process("MainClient", myData->MainClient.misc.processPointer,
				APP_SIG_MAIN_CLIENT_PROCESS);
			Close_Process("DataSave", myData->DataSave.misc.processPointer,
				APP_SIG_DATA_SAVE_PROCESS);
			Close_Process("CaliMeter", myData->CaliMeter.misc.processPointer,
				APP_SIG_CALI_METER_PROCESS);
			Close_Process("AnalogMeter", myData->AnalogMeter.misc.processPointer,
				APP_SIG_ANALOG_METER_PROCESS);
			Close_Process("FADM", myData->FADM.misc.processPointer,
				APP_SIG_FADM_PROCESS);
			Close_Process("FndMeter", myData->FndMeter.misc.processPointer,
				APP_SIG_FND_METER_PROCESS);
			Close_Process("CaliMeter2", myData->CaliMeter2.misc.processPointer,
				APP_SIG_CALI_METER2_PROCESS);
			Close_Process("ExtClient", myData->ExtClient.misc.processPointer,
				APP_SIG_EXT_CLIENT_PROCESS);
			return -108;
		}
	}
/*
	if(myPs->config.versionNo >= 20110429) {
		load = myPs->loadProcess[LOAD_ANALOGMETER2] ; 
	} else {
		load = 0;
	}
*/
	load = myPs->loadProcess[LOAD_ANALOGMETER2] ; //200715 rewrite
	if(load){
		if(Load_Process("Load_AnalogMeter2", "./",
			APP_SIG_ANALOG_METER2_PROCESS) < 0) {
			Close_Process("MainClient", myData->MainClient.misc.processPointer,
				APP_SIG_MAIN_CLIENT_PROCESS);
			Close_Process("DataSave", myData->DataSave.misc.processPointer,
				APP_SIG_DATA_SAVE_PROCESS);
			Close_Process("CaliMeter", myData->CaliMeter.misc.processPointer,
				APP_SIG_CALI_METER_PROCESS);
			Close_Process("AnalogMeter", myData->AnalogMeter.misc.processPointer,
				APP_SIG_ANALOG_METER_PROCESS);
			Close_Process("FADM", myData->FADM.misc.processPointer,
				APP_SIG_FADM_PROCESS);
			Close_Process("FndMeter", myData->FndMeter.misc.processPointer,
				APP_SIG_FND_METER_PROCESS);
			Close_Process("CaliMeter2", myData->CaliMeter2.misc.processPointer,
				APP_SIG_CALI_METER2_PROCESS);
			Close_Process("ExtClient", myData->ExtClient.misc.processPointer,
				APP_SIG_EXT_CLIENT_PROCESS);
			Close_Process("AnalogMeter2", myData->AnalogMeter2.misc.processPointer,
				APP_SIG_ANALOG_METER2_PROCESS);
			return -109;
		}
	}
/*
	if(myPs->config.versionNo >= 20190626) {
		load = myPs->loadProcess[LOAD_AUTOUPDATE] ; 
	} else {
		load = 1;
	}
*/
	load = myPs->loadProcess[LOAD_AUTOUPDATE] ;  //200715 rewrite
	if(load){
		if(Load_Process("Load_AutoUpdate", "./",
			APP_SIG_AUTO_UPDATE_PROCESS) < 0) {
			Close_Process("MainClient", myData->MainClient.misc.processPointer,
				APP_SIG_MAIN_CLIENT_PROCESS);
			Close_Process("DataSave", myData->DataSave.misc.processPointer,
				APP_SIG_DATA_SAVE_PROCESS);
			Close_Process("CaliMeter", myData->CaliMeter.misc.processPointer,
				APP_SIG_CALI_METER_PROCESS);
			Close_Process("AnalogMeter", myData->AnalogMeter.misc.processPointer,
				APP_SIG_ANALOG_METER_PROCESS);
			Close_Process("FADM", myData->FADM.misc.processPointer,
				APP_SIG_FADM_PROCESS);
			Close_Process("FndMeter", myData->FndMeter.misc.processPointer,
				APP_SIG_FND_METER_PROCESS);
			Close_Process("CaliMeter2", myData->CaliMeter2.misc.processPointer,
				APP_SIG_CALI_METER2_PROCESS);
			Close_Process("ExtClient", myData->ExtClient.misc.processPointer,
				APP_SIG_EXT_CLIENT_PROCESS);
			Close_Process("AnalogMeter2", myData->AnalogMeter2.misc.processPointer,
				APP_SIG_ANALOG_METER2_PROCESS);
			Close_Process("AutoUpdate", myData->AutoUpdate.misc.processPointer,
				APP_SIG_AUTO_UPDATE_PROCESS);
			return -110;
		}
	}
/*
	if(myPs->config.versionNo >= 20110429) {
		load = myPs->loadProcess[LOAD_MCONTROL] ; 
	} else {
		load = 1;
	}
*/
	load = myPs->loadProcess[LOAD_DAQ_CLIENT] ;  //200715 rewrite
	if(load){
		if(Load_Process("Load_DAQ_Client", "./",
			APP_SIG_DAQ_CLIENT_PROCESS) < 0) {
			Close_Process("MainClient", myData->MainClient.misc.processPointer,
				APP_SIG_MAIN_CLIENT_PROCESS);
			Close_Process("DataSave", myData->DataSave.misc.processPointer,
				APP_SIG_DATA_SAVE_PROCESS);
			Close_Process("CaliMeter", myData->CaliMeter.misc.processPointer,
				APP_SIG_CALI_METER_PROCESS);
			Close_Process("AnalogMeter", myData->AnalogMeter.misc.processPointer,
				APP_SIG_ANALOG_METER_PROCESS);
			Close_Process("FADM", myData->FADM.misc.processPointer,
				APP_SIG_FADM_PROCESS);
			Close_Process("FndMeter", myData->FndMeter.misc.processPointer,
				APP_SIG_FND_METER_PROCESS);
			Close_Process("CaliMeter2", myData->CaliMeter2.misc.processPointer,
				APP_SIG_CALI_METER2_PROCESS);
			Close_Process("ExtClient", myData->ExtClient.misc.processPointer,
				APP_SIG_EXT_CLIENT_PROCESS);
			Close_Process("AnalogMeter2", myData->AnalogMeter2.misc.processPointer,
				APP_SIG_ANALOG_METER2_PROCESS);
			Close_Process("AutoUpdate", myData->AutoUpdate.misc.processPointer,
				APP_SIG_AUTO_UPDATE_PROCESS);
			Close_Process("DAQ_Client", myData->DAQ_Client.misc.processPointer,
				APP_SIG_DAQ_CLIENT_PROCESS);
			return -111;
		}
	}
		//220316 jws add
	load = myPs->loadProcess[LOAD_COOLINGCONTROL];  //
	if(load){
		if(Load_Process("Load_CoolingControl", "./",
			APP_SIG_COOLING_CONTROL_PROCESS) < 0) {
			Close_Process("MainClient", myData->MainClient.misc.processPointer,
				APP_SIG_MAIN_CLIENT_PROCESS);
			Close_Process("DataSave", myData->DataSave.misc.processPointer,
				APP_SIG_DATA_SAVE_PROCESS);
			Close_Process("CaliMeter", myData->CaliMeter.misc.processPointer,
				APP_SIG_CALI_METER_PROCESS);
			Close_Process("AnalogMeter", myData->AnalogMeter.misc.processPointer,
				APP_SIG_ANALOG_METER_PROCESS);
			Close_Process("FADM", myData->FADM.misc.processPointer,
				APP_SIG_FADM_PROCESS);
			Close_Process("FndMeter", myData->FndMeter.misc.processPointer,
				APP_SIG_FND_METER_PROCESS);
			Close_Process("CaliMeter2", myData->CaliMeter2.misc.processPointer,
				APP_SIG_CALI_METER2_PROCESS);
			Close_Process("ExtClient", myData->ExtClient.misc.processPointer,
				APP_SIG_EXT_CLIENT_PROCESS);
			Close_Process("AnalogMeter2", myData->AnalogMeter2.misc.processPointer,
				APP_SIG_ANALOG_METER2_PROCESS);
			Close_Process("AutoUpdate", myData->AutoUpdate.misc.processPointer,
				APP_SIG_AUTO_UPDATE_PROCESS);
			Close_Process("DAQ_Client", myData->DAQ_Client.misc.processPointer,
				APP_SIG_DAQ_CLIENT_PROCESS);
			Close_Process("CoolingControl", myData->CoolingControl.misc.processPointer,
				APP_SIG_COOLING_CONTROL_PROCESS);
			return -112;
		}
	}

	load = myPs->loadProcess[LOAD_MCONTROL] ; //200715 rewrite
	if(load){
		if(Load_Process("Load_mControl", "./",
			APP_SIG_MODULE_CONTROL_PROCESS) < 0) {
			Close_Process("MainClient", myData->MainClient.misc.processPointer,
				APP_SIG_MAIN_CLIENT_PROCESS);
			Close_Process("DataSave", myData->DataSave.misc.processPointer,
				APP_SIG_DATA_SAVE_PROCESS);
			Close_Process("CaliMeter", myData->CaliMeter.misc.processPointer,
				APP_SIG_CALI_METER_PROCESS);
			Close_Process("AnalogMeter", myData->AnalogMeter.misc.processPointer,
				APP_SIG_ANALOG_METER_PROCESS);
			Close_Process("FADM", myData->FADM.misc.processPointer,
				APP_SIG_FADM_PROCESS);
			Close_Process("FndMeter", myData->FndMeter.misc.processPointer,
				APP_SIG_FND_METER_PROCESS);
			Close_Process("CaliMeter2", myData->CaliMeter2.misc.processPointer,
				APP_SIG_CALI_METER2_PROCESS);
			Close_Process("ExtClient", myData->ExtClient.misc.processPointer,
				APP_SIG_EXT_CLIENT_PROCESS);
			Close_Process("AnalogMeter2", myData->AnalogMeter2.misc.processPointer,
				APP_SIG_ANALOG_METER2_PROCESS);
			Close_Process("AutoUpdate", myData->AutoUpdate.misc.processPointer,
				APP_SIG_AUTO_UPDATE_PROCESS);
			Close_Process("DAQ_Client", myData->DAQ_Client.misc.processPointer,
				APP_SIG_DAQ_CLIENT_PROCESS);
			Close_Process("CoolingControl", myData->CoolingControl.misc.processPointer,
				APP_SIG_COOLING_CONTROL_PROCESS);
			Unload_Module();
			return -112;
		}
	}

	myPs->signal[APP_SIG_APP_CONTROL_PROCESS] = P1;
	userlog(DEBUG_LOG, psName, "System start\n");

	return 0;
}

void AppControl(void)
{
	Check_Message();
	Check_Signal();
	Check_Process();
}

void Check_Signal(void)
{
	int bd, ch, i, rtn, checkTime = 0;
	long diff;
	time_t the_time;

	if(myPs->signal[APP_SIG_QUIT] != P0) {
		(void)time(&the_time);
		diff = the_time - myPs->misc.quitDelayTime;
		
#if CYCLER_TYPE == DIGITAL_CYC
		checkTime = 5;
#else
		checkTime = 2;
#endif
	//	if(diff > 2) { //2sec
		if(diff > checkTime) {
			myPs->signal[APP_SIG_APP_CONTROL_PROCESS] = P2;
			if(myPs->signal[APP_SIG_QUIT] == P1 //normal power off
				|| myPs->signal[APP_SIG_QUIT] == P2 //force power off
				|| myPs->signal[APP_SIG_QUIT] == P4 //terminal halt
				|| myPs->signal[APP_SIG_QUIT] == P5 //main emg.
				|| myPs->signal[APP_SIG_QUIT] == P6 //sub emg.
				|| myPs->signal[APP_SIG_QUIT] == P7 //ac power fail
				|| myPs->signal[APP_SIG_QUIT] == P8 //ups battery fail
				|| myPs->signal[APP_SIG_QUIT] == P9 //smps fail
				|| myPs->signal[APP_SIG_QUIT] == P10 //OT fail
				|| myPs->signal[APP_SIG_QUIT] == P11 //meter fail
				|| myPs->signal[APP_SIG_QUIT] == P12 //chamber fail
				|| myPs->signal[APP_SIG_QUIT] == P13 //OVP fail
				|| myPs->signal[APP_SIG_QUIT] == P14 //OTP fail
				|| myPs->signal[APP_SIG_QUIT] == P15 //CCC fail
				|| myPs->signal[APP_SIG_QUIT] == P16  //SMOKE SENSOR fail
				|| myPs->signal[APP_SIG_QUIT] == P17  //GUI shutdown signal
				|| myPs->signal[APP_SIG_QUIT] == P18  //CAN main shutdown
				|| myPs->signal[APP_SIG_QUIT] == P19  //DYSON Maintenance shutdown
				|| myPs->signal[APP_SIG_QUIT] == P20  //Chamber Fault shutdown
				|| myPs->signal[APP_SIG_QUIT] == P21) {  //C_FAULT_HARD_VENTING shutdown
				myPs->signal[APP_SIG_APP_CONTROL_PROCESS] = P10; //halt
			}
		}
		if(myPs->signal[APP_SIG_QUIT] == P3) {
			myPs->signal[APP_SIG_APP_CONTROL_PROCESS] = P2;
		} //terminal quit
	}
// 150512 oys add start : chData Backup, Restore
	for(i = 0; i < myData->mData.config.installedCh; i++){
		bd = myData->CellArray1[i].bd;	
		ch = myData->CellArray1[i].ch;
		if(myData->bData[bd].cData[ch].op.state == C_PAUSE){
			if(myPs->backup[i] == P1){
				myPs->backup[i] = P2;
				rtn = ChData_Backup(bd, ch, i);
				if(rtn < 0){
					myPs->backup[i] = P4;
				}else{
					myPs->backup[i] = P3;
				}
				myPs->backupFlag[i] = P1;
			}
		}		
		if(myData->bData[bd].cData[ch].op.state == C_IDLE
			|| myData->bData[bd].cData[ch].op.state == C_STANDBY){
			if(myPs->restore[i] == P1){
				myPs->restore[i] = P2;
				rtn = ChData_Restore(bd, ch, i);
				if(rtn < 0){
					myPs->restore[i] = P4;
				}else{
					myPs->restore[i] = P3;
				}
				myPs->restoreFlag[i] = P1;
			}
		}
	}	
// 150512 oys add end : chData Backup, Restore
}

void CloseAppControl(void)
{
	Close_Process("MainClient", myData->MainClient.misc.processPointer,
		APP_SIG_MAIN_CLIENT_PROCESS);
	Close_Process("DataSave", myData->DataSave.misc.processPointer,
		APP_SIG_DATA_SAVE_PROCESS);
	Close_Process("CaliMeter", myData->CaliMeter.misc.processPointer,
		APP_SIG_CALI_METER_PROCESS);
	Close_Process("AnalogMeter", myData->AnalogMeter.misc.processPointer,
		APP_SIG_ANALOG_METER_PROCESS);
	Close_Process("FADM", myData->AnalogMeter.misc.processPointer,
		APP_SIG_FADM_PROCESS);
	Close_Process("FndMeter", myData->FndMeter.misc.processPointer,
		APP_SIG_FND_METER_PROCESS);
	Close_Process("CaliMeter2", myData->CaliMeter2.misc.processPointer,
		APP_SIG_CALI_METER2_PROCESS);
	Close_Process("ExtClient", myData->ExtClient.misc.processPointer,
		APP_SIG_EXT_CLIENT_PROCESS);
	Close_Process("AnalogMeter2", myData->AnalogMeter2.misc.processPointer,
		APP_SIG_ANALOG_METER2_PROCESS);
	Close_Process("AutoUpdate", myData->AutoUpdate.misc.processPointer,
		APP_SIG_AUTO_UPDATE_PROCESS);
	Close_Process("DAQ_Client", myData->DAQ_Client.misc.processPointer,
		APP_SIG_DAQ_CLIENT_PROCESS);
	Close_Process("CoolingControl", myData->CoolingControl.misc.processPointer,
		APP_SIG_COOLING_CONTROL_PROCESS);
	Unload_Module();
	
	Save_SystemMemory();

	userlog(DEBUG_LOG, psName, "runningTime[0] max:%ld[%ld], min:%ld[%ld]\n",
		myData->mData.runningTime[2][0], myData->mData.runningTime[2][1],
		myData->mData.runningTime[3][0], myData->mData.runningTime[3][1]);
	userlog(DEBUG_LOG, psName, "runningTime[1] max:%ld[%ld], min:%ld[%ld]\n",
		myData->mData.runningTime[4][0], myData->mData.runningTime[4][1],
		myData->mData.runningTime[5][0], myData->mData.runningTime[5][1]);

	if(myPs->signal[APP_SIG_APP_CONTROL_PROCESS] == P10) {
		userlog(DEBUG_LOG, psName, "system shutdown\n");
		system("cd /");
		system("update -s1");
		system("shutdown -h now");
	} else {
		userlog(DEBUG_LOG, psName, "system quit\n");
		system("cd /");
		system("update -s1");
	}

	Close_SystemMemory();
	system("rmmod mbuff");
}
