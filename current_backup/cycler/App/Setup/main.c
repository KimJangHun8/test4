//******************************************************************************
//	SUBJECT		: SYSTEM SETUP PROGRAM
//	VERSION		: REV 0.5
//	DATE		: 2019/12/06
//	WRITER		: OK YOSEOP
//******************************************************************************
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>		//use for open,read close function
#include <fcntl.h>
#include <time.h>
#include <main.h>
#include <locale.h>
#include "/project/Cycler/current/cycler/INC/datastore.h"

volatile S_SYSTEM_DATA *myData;

int main(void)
{
	int i, item;
	
	while(1){	
//0.Initial screen =============================================================
		system("clear");
		for(i=0; i < 80; i++) {
			printf("=");
		}
		printf("\n");
		printf(" SYSTEM SETUP PROGRAM\t\t\t\t\t\tmade by oys\n");
		printf(" \t\t\t\t\t\t\tUpdate : 2019-12-19\n");
		for(i=0; i < 80; i++) {
			printf("=");
		}
		printf("\n");

//1.Select setting item  =======================================================
//		printf(" 1. parameter/mControl_Config
//		printf(" 1. INC/SysConfig.h\n");
		printf("\n");
		printf(" 0. Select Cycler Type(NEW)\n");
		printf(" 1. Select Cycler Type\n");
		printf(" 2. Make Array\n");
 		printf(" 3. Make Default CALI_BD\n");
 		printf(" 4. Make Default AuxSetData\n");
		printf("\n");
		printf(" EXIT : 'ctrl + c'\n");
		printf(" Select No : ");
		scanf("%d", &item);

		switch(item) {
			case 0:
				selectType2();
				break;
			case 1:
				selectType();
				break;
			case 2:
				makeArray();
				break;
			case 3:
				makeDefault_CALI_BD();
				break;
			case 4:
				makeDefault_AuxSetData();
//				SysConfig();	
				break;
			case 5:
	//			mControl_Config();
				break;
			default:
				break;
		}
	}
}

int mControl_Config(void)
{
	int i, j, value;
	char cmd[128], fileName[32], parameter[64];
	unsigned char Range[5], flag;
	float refV, refI, Rg;
	FILE *fp;

	S_MODULE_CONFIG config;

	memset((char *)&config, 0x00, sizeof(S_MODULE_CONFIG));
	memset(parameter, 0x00, sizeof(parameter));
	strcpy(parameter, "/root/cycler_data/config/parameter/");

	memset(cmd, 0x00, sizeof(cmd));
	memset(fileName, 0x00, sizeof(fileName));
	sprintf(fileName, "%smControl_Config", parameter);

	printf("\n");
	printf("WARNING! This file has a critical impact on the operation of the equipment\n");
	printf("주의! 해당 파일은 장비 운영에 중대한 영향을 미칩니다. 세팅에 주의하십시오.\n");
	printf("\n※값을 잘못 기입했을 경우 ctrl + c 입력 후 프로그램 재실행하여\n처음부터 다시 입력 하십시오.\n\n");

	printf("장비에 설치된 메인보드나 백플랜보드의 개수\n");
	printf("Number of installed total main board or backplan board\n>>");
	scanf("%d", &config.installedBd);
	
	printf("\n총 채널 개수\n");
	printf("Number of installed total channel\n>>");
	scanf("%d", &config.installedCh);
		
	printf("\n메인보드나 백플랜보드 단위 채널 개수\n");
	printf("Number of channel per main board or backplan board\n>>");
	scanf("%d", &config.chPerBd);
		
	printf("\n전압 레인지 개수\n");
	printf("Number of total voltage range\n>>");
	scanf("%hd", &config.rangeV);
		
	printf("\n전류 레인지 개수\n");
	printf("Number of total current range\n>>");
	scanf("%hd", &config.rangeI);
		
	for(i = 0; i < config.rangeV; i++) {
		printf("\n전압 레인지%d 의 최대값 ex) 5V : 5\n", i+1);
		printf("Voltage range%d max value\n>>", i+1);
		scanf("%ld", &config.maxVoltage[i]);
		config.maxVoltage[i] *= 1000000;
		
		printf("\n전압 레인지%d 의 최소값 ex) 5V : -5 (Default : 0)\n", i+1);
		printf("Voltage range%d min value\n>>", i+1);
		scanf("%ld", &config.minVoltage[i]);
		config.minVoltage[i] *= 1000000;
	}
		
	for(i = 0; i < config.rangeI; i++) {
		printf("\n전류 레인지%d 의 값 ex) 1A : 1000000\n", i+1);
		printf("Current range%d value\n>>", i+1);
		scanf("%ld", &config.maxCurrent[i]);
		config.minCurrent[i] = config.maxCurrent[i] * -1;
	}
		
	printf("\n전압 값 단위 선택 [0:micro(Default) 1: nano]\n");
	printf("Select voltage value unit\n>>");
	scanf("%c", &config.ratioVoltage);
		
	printf("\n전류 값 단위 선택 [0:micro(Default) 1: nano]\n");
	printf("1A 이하 저전류장비의 경우 nano로 선택해야 함\n");
	printf("Select current value unit\n>>");
	scanf("%c", &config.ratioCurrent);

	config.watchdogFlag = 1;
	config.ADC_type = 1;
	
	printf("\n메인보드 타입에 따른 하드웨어 스펙번호 입력\n");
	printf("\n※주로 사용하는 HWSPEC NO LIST\n");
	printf("80 : 4CH MULTI BD\n");
	printf("81 : BACKPLAN BD (Coin Cell) [MAX64CH]\n");
	printf("82 : BACKPLAN BD [MAX32CH]\n");
	printf("85 : Module Cycler\n");
	printf("87 : Module Cycler Parallel\n");
	printf("88 : MAIN BD REV12\n");
	printf("90 : 32CH MAIN_BD\n");
	printf("202: DIGITAL CYCLER\n");
	printf("※etc : Firmware 담당자 지원 요청 하세요.\n");

	printf("\n[HwSpec] Select MainBoard type\n>>");
	scanf("%c", &config.hwSpec);

	printf("\nCELL Type에 따른 용량산출 방식 선택 [0:AmpareHour(Default) 1:Capacitance]\n");
	printf("Capacity type\n>>");
	scanf("%c", &config.capacityType);

	//No use totalJig , bdInJig
	for(i = 0; i < config.rangeI; i++) {
		printf("\n레인지%d의 Shunt 값 [단위 : Ω]\n", i+1);
		printf("Range%d shunt value\n>>", i+1);
		scanf("%f", &config.shunt[i]);
	}
		
	printf("\n전압 증폭회로 레퍼런스 기준 전압 입력 ex) 8V = 8\n");
	printf("Voltage amp circuit reference voltage\n>>");
	scanf("%f", &refV);
	refV *= 1000000;
	printf("\n전류 증폭회로 레퍼런스 기준 전압 입력 ex) 8V = 8\n");
	printf("Current amp circuit reference voltage\n>>");
	scanf("%f", &refI);
	refI *= 1000000;
	
	for(i = 0; i < config.rangeI; i++) {
		config.gain[i] = refV / (config.maxCurrent[i] * config.shunt[i]);
		printf("Range%d gain value : %f\n", i+1, config.gain[i]);
	}

	printf("\nAD 증폭 배수 입력 (Default : 1)\n");
	printf("AD amp multiple\n>>");
	scanf("%f", &config.adAmp);

	printf("\n전압 측 증폭기 Gain저항 값 입력 [단위 : ㏀]\n");
	printf("Voltage amp gain resistance value [Unit : ㏀]\n>>");
	scanf("%f", &Rg);
	config.voltageAmp = 49.4 / Rg + 1;
	printf("입력 저항 값에 의한 전압 증폭 배수 : %f\n", config.voltageAmp);

	printf("\n전류 측 증폭기 Gain저항 값 입력 [단위 : ㏀]\n");
	printf("Current amp gain resistance value [Unit : ㏀]\n>>");
	scanf("%f", &Rg);
	config.currentAmp = 49.4 / Rg + 1;
	printf("입력 저항 값에 의한 전류 증폭 배수 : %f\n", config.currentAmp);

	printf("\n장비 제어주기 선택\n");
	printf("0 : 100ms	=> ADC 1EA, chPerBD <= 32CH, ADC 2EA, chPerBD <= 64CH\n");
	printf("1 : 50ms	=> ADC 1EA, chPerBD <= 16CH, ADC 2EA, chPerBD <= 32CH\n");
	printf("2 : 25ms	=> ADC 2EA, chPerBD <= 16CH\n");
	printf("3 : 20ms	=> ADC 2EA, chPerBD <=  8CH\n");
	printf("4 : 100ms_2	=> ADC 1EA, chPerBD <= 64CH\n");
	printf("5 : 10ms	=> ADC 2EA, chPerBD <=  8CH\n");
	printf("6 : 100ms_CAN TYPE\n");
	printf("\nSelect equipment control period\n>>");
	scanf("%c", &config.rt_scan_type);

	printf("\nAUX 온도 채널 개수 [Default : 0]\n");
	printf("Number of temperature channels to Aux\n>>");
	scanf("%hd", &config.installedTemp);
		
	printf("\nAUX 전압 채널 개수 [Default : 0]\n");
	printf("Number of voltage channels to to Aux\n>>");
	scanf("%hd", &config.installedAuxV);
		
	config.installedCAN = 0;
	config.auto_v_cali = 1;

	printf("\n병렬 모드 사용유무 입력 [0:사용안함 1:병렬사용]\n");
	printf("Enable parallel mode [0:No Use 1:Use]\n>>");
	scanf("%c", &config.parallelMode);
		
	config.DAC_type = 1;
	config.ADC_num = 2;
	config.AD_offset = 0.00002;
	config.SoftFeedbackFlag = 1;
	config.MainBdType = 1;
	config.FadBdUse = 0;		

	printf("\n레인지 신호 조합 설정\n");
	printf("Default)\n");
	printf("Range0 = 0\n");
	printf("Range1 = 0\n");
	printf("Range2 = 2\n");
	printf("Range3 = 4\n");
	printf("Range4 = 0\n\n");
		
	for(i = 0; i <= config.rangeI; i++) {
		printf("Range%d signal\n>>", i);
		scanf("%c", &Range[i]);
		for(j = 0; j < 4; j++){
			flag = 0x01;
			if(Range[i] & flag << j){
				config.range[i][j] = 1;
			}
		}
	}

// User Value Input
	printf("\n입력완료. 파일에 저장 하시겠습니까? [0:아니오 1:예]");
	printf("\nInput complete. save to file? [0:No 1:Yes]\n>>");
	scanf("%d", &value);
	if(value == 0) return -1;
		
	if((fp = fopen(fileName, "w")) == NULL) {
		printf("mControl_Config file not found\n");
		return -1;
	} else {
// Save File
		fprintf(fp, "installedBd			:	%d\n", config.installedBd);
		fprintf(fp, "installedCh			:	%d\n", config.installedCh);
		fprintf(fp, "chPerBd				:	%d\n", config.chPerBd);
		fprintf(fp, "rangeV				:	%d\n", config.rangeV);
		fprintf(fp, "rangeI				:	%d\n", config.rangeI);
		fprintf(fp, "maxVoltage			:	%ld %ld %ld %ld\n",
					config.maxVoltage[0], config.maxVoltage[1],
					config.maxVoltage[2], config.maxVoltage[3]);
		fprintf(fp, "minVoltage			:	%ld %ld %ld %ld\n", 
					config.minVoltage[0], config.minVoltage[1],
					config.minVoltage[2], config.minVoltage[3]);
		fprintf(fp, "maxCurrent			:	%ld %ld %ld %ld\n",
					config.maxCurrent[0], config.maxCurrent[1],
					config.maxCurrent[2], config.maxCurrent[3]);
		fprintf(fp, "minCurrent			:	%ld %ld %ld %ld\n",
					config.minCurrent[0], config.minCurrent[1],
					config.minCurrent[2], config.minCurrent[3]);
		fprintf(fp, "ratioVoltage		:	%d\n", config.ratioVoltage);
		fprintf(fp, "ratioCurrent		:	%d\n", config.ratioCurrent);
		fprintf(fp, "watchdogFlag		:	%d\n", config.watchdogFlag);
		fprintf(fp, "ADC_type			:	%d\n", config.ADC_type);
		fprintf(fp, "hwSpec				:	%d\n", config.hwSpec);
		fprintf(fp, "capacityType		:	%d\n", config.capacityType);
		fprintf(fp, "totalJig			:	%d\n", config.totalJig);
		fprintf(fp, "bdInJig1_8			:	%d	%d	%d	%d	%d	%d	%d	%d\n",
					config.bdInJig[0], config.bdInJig[1],
					config.bdInJig[2], config.bdInJig[3],
					config.bdInJig[4], config.bdInJig[5],
					config.bdInJig[6],config.bdInJig[7]);
		fprintf(fp, "bdInJig9_16			:	%d	%d	%d	%d	%d	%d	%d	%d\n",
					config.bdInJig[8], config.bdInJig[9],
					config.bdInJig[10], config.bdInJig[11],
					config.bdInJig[12], config.bdInJig[13],
					config.bdInJig[14],config.bdInJig[15]);
		fprintf(fp, "shuntOhm			:	%f	%f	%f	%f\n",
					config.shunt[0], config.shunt[1],
					config.shunt[2], config.shunt[3]);
		fprintf(fp, "gain				:	%f	%f	%f	%f\n",
					config.gain[0], config.gain[1],
					config.gain[2],	config.gain[3]);
		fprintf(fp, "adAmp				:	%f	%f	%f\n",
					config.adAmp, config.voltageAmp,
					config.currentAmp);
		fprintf(fp, "rt_scan_type		:	%d\n", config.rt_scan_type);
		fprintf(fp, "installedTemp		:	%d\n", config.installedTemp);
		fprintf(fp, "installedAuxV		:	%d\n", config.installedAuxV);
		fprintf(fp, "installedCan		:	%d\n", config.installedCAN);
		fprintf(fp, "auto_v_cali			:	%d\n", config.auto_v_cali);
		fprintf(fp, "parallelMode		:	%d\n", config.parallelMode);
		fprintf(fp, "DAC_type			:	%d\n", config.DAC_type);
		fprintf(fp, "ADC_num				:	%d\n", config.ADC_num);
		fprintf(fp, "AD_offset			:	%f\n", config.AD_offset);
		fprintf(fp, "FeedbackFlag		:	%d\n", config.SoftFeedbackFlag);
		fprintf(fp, "MainBdType			:	%d\n", config.MainBdType);
		fprintf(fp, "FadBdUse			:	%d\n", config.FadBdUse);
		fprintf(fp, "RangeSelectFlag		:	0x10	0x20	0x40	0x80\n");
		fprintf(fp, "Range0				:	%d		%d		%d		%d\n",
					config.range[0][0], config.range[0][1],
					config.range[0][2], config.range[0][3]);
		fprintf(fp, "Range1				:	%d		%d		%d		%d\n",
					config.range[1][0], config.range[1][1],
					config.range[1][2], config.range[1][3]);
		fprintf(fp, "Range2				:	%d		%d		%d		%d\n",
					config.range[2][0], config.range[2][1],
					config.range[2][2], config.range[2][3]);
		fprintf(fp, "Range3				:	%d		%d		%d		%d\n",
					config.range[3][0], config.range[3][1],
					config.range[3][2], config.range[3][3]);
		fprintf(fp, "Range4				:	%d		%d		%d		%d\n",
					config.range[4][0], config.range[4][1],
					config.range[4][2], config.range[4][3]);
		fprintf(fp, "\n");	
		fprintf(fp, "0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 \n");
 	    fprintf(fp, "0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 \n");
		fprintf(fp, "================================================================================\n");
		fprintf(fp, "Option Setting Description\n");
		fprintf(fp, "Update : 2017.01.16\n");
		fprintf(fp, "Writer : OYS\n");
		fprintf(fp, "================================================================================\n");
		fprintf(fp, "installedBd			=> insalled MainBoard No.\n");
		fprintf(fp, "installedCh			=> installed Total Ch\n");
		fprintf(fp, "chPerBd				=> Ch Per MainBD\n");
		fprintf(fp, "rangeV				=> Voltage Range No.\n");
		fprintf(fp, "rangeI				=> Current Range No.\n");
		fprintf(fp, "maxVoltage\n");
		fprintf(fp, "minVoltage\n");
		fprintf(fp, "maxCurrent\n");
		fprintf(fp, "minCurrent\n");
		fprintf(fp, "ratioVoltage		=> 0 : Micro, 1 : Nano (Default : 0)\n");
		fprintf(fp, "ratioCurrent		=> 0 : Micro, 1 : Nano (Default : 0)\n");
		fprintf(fp, "watchdogFlag	 	=> WatchDog UseFlag (Default : 1)	\n");
		fprintf(fp, "ADC_type			=> 0 : COMM_TYPE, 1 : 7805 (Default : 1)\n");
		fprintf(fp, "hwSpec				=> H/W SPEC No. ex) 63 : L_5V_150A_R3_AD2\n");
		fprintf(fp, "capacityType		=> 0 : AmpareHour, 1 : Farad\n");
		fprintf(fp, "totalJig\n");
		fprintf(fp, "bdInJig1_8\n");
		fprintf(fp, "bdInJig9_16\n");
		fprintf(fp, "shuntOhm			=> Range Shunt Value\n");
		fprintf(fp, "gain				=> Range Gain Value\n");
		fprintf(fp, "adAmp				=> AD_AMP	VOLTAGE_AMP	 CURRENT_AMP\n");
		fprintf(fp, "rt_scan_type		=> 0 : 100ms	=> ADC 1EA, chPerBD <= 32CH\n");
		fprintf(fp, "									   ADC 2EA, chPerBD <= 64CH\n");
		fprintf(fp, "					   1 : 50ms		=> ADC 1EA, chPerBD <= 16CH\n");
		fprintf(fp, "									   ADC 2EA, chPerBD <= 32CH\n");
		fprintf(fp, "					   2 : 25ms		=> ADC 2EA, chPerBD <= 16CH\n");
		fprintf(fp, "					   3 : 20ms		=> ADC 2EA, chPerBD <=  8CH\n");
		fprintf(fp, "					   4 : 100ms_2	=> ADC 1EA, chPerBD <= 64CH\n");
		fprintf(fp, "					   5 : 10ms		=> ADC 2EA, chPerBD <=  8CH\n");
		fprintf(fp, "					   6 : 100ms_CAN TYPE\n");
		fprintf(fp, "installedTemp		=> AuxTemperature installed Ch No.\n");
		fprintf(fp, "installedAux		=> AuxVoltage installed Ch No.\n");
		fprintf(fp, "installedCan		=> AuxCan installed Ch No.\n");
		fprintf(fp, "auto_v_cali			=> auto Voltage Cali (Default : 1)\n");
		fprintf(fp, "parallelMode		=> parallelMode UseFlag\n");
		fprintf(fp, "DAC_type			=> 0 : 712P, 1 : 7741\n");
		fprintf(fp, "ADC_num				=> ADC No.(1:ADC 1EA, 2:ADC 2EA)\n");
		fprintf(fp, "AD_offset			=> Charge Current AD Offset Value (Default : 0.00002)\n");
		fprintf(fp, "FeedbackFlag		=> SoftFeedback UseFlag\n");
		fprintf(fp, "MainBdType			=> 0 : CPLD TYPE, 1: FPGA TYPE\n");
		fprintf(fp, "FadBdUse			=> FAD BD UseFlag\n");
		fprintf(fp, "RangeSelectFlag\n");
		fprintf(fp, "Range0\n");
		fprintf(fp, "Range1\n");
		fprintf(fp, "Range2\n");
		fprintf(fp, "Range3\n");
		fprintf(fp, "Range4");
	}
	fclose(fp);

	sleep(1);
	return 0;
}

/*
int SysConfig(void)
{
	int i, j = 0, value, loop;
	char cmd[128], fileName[128], INC[64], Scripts[64];
	char str1[20], buf[20]; 
	char release1[] = "RELEASE_VERSION1", release2[] = "RELEASE_VERSION2",
		 release3[] = "RELEASE_VERSION3", release4[] = "RELEASE_VERSION4",
		 release5[] = "RELEASE_VERSION5", release6[] = "RELEASE_VERSION6",
		 release7[] = "RELEASE_VERSION7", release8[] = "RELEASE_VERSION8";
	int  cycler_type = 0, vender = 0, daehwa = 0,
		 change_vi_check = 0, network_ver = 0, 
		 shunt_r_rcv = 0, aux_control = 0, dio_type = 0,
		 release_ver[8], system_type = 0, edlc_type = 0,
		 integratedGUI = 0, userPatternType = 0, tmp;
		 // equ_type = 0, can_type = 0, program_ver[4];
	FILE *fp;
	
	memset(release_ver, 0x00, sizeof(release_ver));
	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/project/Cycler/current/cycler/INC/SysConfig.h");
	if((fp = fopen(fileName, "r")) == NULL){
		printf("SysConfig Not Found.\n");
		return -1;
	}
	
	memset(str1, 0x00, sizeof(str1));
	for(i = 0 ; i < 500 ; i++){
		tmp = fscanf(fp, "%s", str1);
		if(strcmp(str1, release1) == 0){
			memset(str1, 0x00, sizeof(str1));
			tmp = fscanf(fp, "%s", buf);
			release_ver[0] = (unsigned char)atoi(buf);
			j++;
		}else if(strcmp(str1, release2) == 0){
			memset(str1, 0x00, sizeof(str1));
			tmp = fscanf(fp, "%s", buf);
			release_ver[1] = (unsigned char)atoi(buf);
			j++;
		}else if(strcmp(str1, release3) == 0){
			memset(str1, 0x00, sizeof(str1));
			tmp = fscanf(fp, "%s", buf);
			release_ver[2] = (unsigned char)atoi(buf);
			j++;
		}else if(strcmp(str1, release4) == 0){
			memset(str1, 0x00, sizeof(str1));
			tmp = fscanf(fp, "%s", buf);
			release_ver[3] = (unsigned char)atoi(buf);
			j++;
		}else if(strcmp(str1, release5) == 0){
			memset(str1, 0x00, sizeof(str1));
			tmp = fscanf(fp, "%s", buf);
			release_ver[4] = (unsigned char)atoi(buf);
			j++;
		}else if(strcmp(str1, release6) == 0){
			memset(str1, 0x00, sizeof(str1));
			tmp = fscanf(fp, "%s", buf);
			release_ver[5] = (unsigned char)atoi(buf);
			j++;
		}else if(strcmp(str1, release7) == 0){
			memset(str1, 0x00, sizeof(str1));
			tmp = fscanf(fp, "%s", buf);
			release_ver[6] = (unsigned char)atoi(buf);
			j++;
		}else if(strcmp(str1, release8) == 0){
			memset(str1, 0x00, sizeof(str1));
			tmp = fscanf(fp, "%s", buf);
			release_ver[7] = (unsigned char)atoi(buf);
			j++;
		}else{
			memset(str1, 0x00, sizeof(str1));
		}
		if(j == 8) break;
	}
	fclose(fp);
	
	memset(INC, 0x00, sizeof(INC));
	strcpy(INC, "/project/Cycler/current/cycler/INC/");
	memset(Scripts, 0x00, sizeof(Scripts));
	strcpy(Scripts, "/project/Cycler/current/cycler/App/Scripts/");

	memset(cmd, 0x00, sizeof(cmd));

	memset(fileName, 0x00, sizeof(fileName));
	sprintf(fileName, "%sSysConfig.h", INC);

	printf("\n");
	printf("WARNING! This file has a critical impact on the compile of the program.\n");
	printf("\nOperation System Select.\n");
	printf("0 : SBC [DEFAULT]
1 : VMware Workstation");
	printf("\nSelect operation system type\n>>");
	loop = 1;
	while(loop) {
		scanf("%d",&system_type);
		while(getchar() != '\n');
		if(system_type >= 0 && system_type <= 1)
			loop = 0;
		else
			printf("Invalid value.\nValue is no valid. please enter again\n>>");
	}
			
	printf("\nCycler type Select.\n");
	printf("0 : Linear [DEFAULT]
1 : Digital
2 : CAN");
	printf("\nSelect Cycler type\n>>");
	loop = 1;
	while(loop) {
		scanf("%d",&cycler_type);
		while(getchar() != '\n');
		if(cycler_type >= 0 && cycler_type <= 2)
			loop = 0;
		else
			printf("Invalid value.\nValue is no valid. please enter again\n>>");
	}
	
	printf("\nCycler VENDER Select\n");
	printf("1 : LG CHEM [DEFAULT]
2 : SAMSUNG SDI
3 : SK Innovation
9 : JAPAN");
	printf("\nSelect Vender\n>>");
	loop = 1;
	while(loop) {
		scanf("%d",&vender);
		while(getchar() != '\n');
		if((vender >= 0 && vender <= 3) || vender == 9)
			loop = 0;
		else
			printf("Invalid value.\nValue is no valid. please enter again\n>>");
	}

	if(vender == 1) {
		printf("\nUse integrated GUI(LG_NEW) program? [0:No 1:Yes]\n>>");
		loop = 1;
		while(loop) {
			scanf("%d",&integratedGUI);
			if(integratedGUI == 1){
				change_vi_check = 1;
			}
			while(getchar() != '\n');
			if(integratedGUI >= 0 && integratedGUI <= 1)
					loop = 0;
			else
				printf("Invalid value.\nValue is no valid. please enter again\n>>");
		}
	}
	
	printf("\nIs this equipment made in Daehwa? [0:No 1:Yes]\n>>");
	loop = 1;
	while(loop) {
		scanf("%d",&daehwa);
		while(getchar() != '\n');
		if(daehwa >= 0 && daehwa <= 1)
				loop = 0;
		else
			printf("Invalid value.\nValue is no valid. please enter again\n>>");
	}
	
	printf("\nPATTERN_500 USE (500 millian)? [0:No 1:Yes]\n>>");
	loop = 1;
	while(loop) {
		scanf("%d",&userPatternType);
		while(getchar() != '\n');
		if(userPatternType >= 0 && userPatternType <= 1)
				loop = 0;
		else
			printf("Invalid value.\nValue is no valid. please enter again\n>>");
	}

	printf("\nNETWORK_VERSTION Select.\n");
	printf("1 : 4101 (CoinCell)
2 : 4102 (General) [DEFAULT]
3 : 4103 (USE AUX)
4 : 4105 (SDI MES4)");
	printf("\nSelect Network Version\n>>");
	loop = 1;
	while(loop) {
		scanf("%d",&network_ver);
		while(getchar() != '\n');
		if(network_ver >= 1 && network_ver <= 4)
				loop = 0;
		else
			printf("Invalid value.\nValue is no valid. please enter again\n>>");
	}
	if(network_ver == 3) {
		printf("\nAUX_CONTROL USE? [0:No 1:Yes]\n>>");
		loop = 1;
		while(loop) {
			scanf("%d",&aux_control);
			while(getchar() != '\n');
			if(aux_control >= 0 && aux_control <= 1)
					loop = 0;
			else
				printf("Invalid value.\nValue is no valid. please enter again\n>>");
		}
	} else {
		aux_control = 0;
	}
	
	printf("\nSHUNTI_R_RCV Select
0 : NORMAL [OLD TYPE]
1 : SHUNT VALUE RECEIVE [DEFAULT]
2 : SDI TYPE\n");
	printf("Select type of calibration for GUI program\n>>");
	loop = 1;
	while(loop) {
		scanf("%d",&shunt_r_rcv);
		while(getchar() != '\n');
		if(shunt_r_rcv >= 0 && shunt_r_rcv <= 2)
				loop = 0;
		else
			printf("Invalid value.\nValue is no valid. please enter again\n>>");
	}

	printf("\nDIO_TYPE Select.\n");
	printf("0 : Not Use [DEFAULT]
1 : DOOR OPEN DETECTION
2 : DC FAN ERROR DETECTION
3 : DOOR OPEN & SMOKE DETECTION");
	printf("\nSelect I/O function to use\n>>");
	loop = 1;
	while(loop) {
		scanf("%d",&dio_type);
		while(getchar() != '\n');
		if(dio_type >= 0 && dio_type <= 3)
				loop = 0;
		else
			printf("Invalid value.\nValue is no valid. please enter again\n>>");
	}

	printf("\nEDLC_TYPE Select?\n");
	printf("Is it EDLC Equipment? [0:No 1:Yes]\n>>");
	loop = 1;
	while(loop) {
		scanf("%d",&edlc_type);
		while(getchar() != '\n');
		if(edlc_type >= 0 && edlc_type <= 1)
				loop = 0;
		else
			printf("Invalid value.\nValue is no valid. please enter again\n>>");
	}

	printf("\nInput Complete. Save to file? [0:No 1:Yes]\n>>");
	loop = 1;
	while(loop) {
		scanf("%d", &value);
		while(getchar() != '\n');
		if(value >= 0 && value <= 1)
				loop = 0;
		else
			printf("Invalid value.\nValue is no valid. please enter again\n>>");
	}
	if(value == 0) return -1;
	system("/project/Cycler/current/cycler/App/Scripts/SysConfig_delete.sh");
	
	if((fp = fopen(fileName, "w")) == NULL) {
		printf("SysConfig.h file not found\n");
		return -1;
	} else {

		fprintf(fp,"#ifndef __SYSCONFIG_H__\n#define __SYSCONFIG_H__\n");
		fprintf(fp, "\n");
		
		fprintf(fp,"//SBC : 0, Vmware : 1\n");
		fprintf(fp,"#define SYSTEM_TYPE		%d\n", system_type);
		fprintf(fp, "\n");
		
		fprintf(fp,"#define LINEAR_CYC		0\n");
		fprintf(fp,"#define DIGITAL_CYC		1\n");
		fprintf(fp,"#define CAN_CYC			2\n");
		fprintf(fp, "\n");

		fprintf(fp,"//LINEAR : 0, DIGITAL : 1, CAN : 2\n");
		switch(cycler_type){
			case 0:	fprintf(fp,"#define CYCLER_TYPE	0\n");	break;
			case 1:	fprintf(fp,"#define CYCLER_TYPE	1\n");	break;
			case 2:	fprintf(fp,"#define CYCLER_TYPE	1\n");	break;
			default:	fprintf(fp,"#define CYCLER_TYPE	0\n");	break;
		}
		fprintf(fp, "#define _CYCLER\n");
		fprintf(fp, "//#define _PACK_CYCLER\n");
		fprintf(fp, "\n");
		
		fprintf(fp,"#define VENDER	%d\n", vender);
		switch(network_ver) {
			case 1:fprintf(fp,"#define NETWORK_VERSION	4101\n");break;
			case 2:fprintf(fp,"#define NETWORK_VERSION	4102\n");break;
			case 3:fprintf(fp,"#define NETWORK_VERSION	4103\n");break;
			case 4:fprintf(fp,"#define NETWORK_VERSION	4105\n");break;
			default:fprintf(fp,"#define NETWORK_VERSION	4102\n");break;
		}
		fprintf(fp, "\n");

		for(i = 0; i < 8; i++){
			fprintf(fp,"#define RELEASE_VERSION%d	%d\n", i+1, release_ver[i]);
		}
		fprintf(fp, "\n");
		
		fprintf(fp,"#define REAL_TIME	1\n");
		fprintf(fp,"#define SHUNT_R_RCV	%d\n", shunt_r_rcv);
		fprintf(fp,"#define AUX_CONTROL	%d\n", aux_control);
		fprintf(fp,"#define DIO_TYPE	%d\n", dio_type);
		fprintf(fp,"#define SEC_TYPE	0\n");
		fprintf(fp,"#define EDLC_TYPE	%d\n", edlc_type);
		fprintf(fp,"#define MACHINE_TYPE	0\n");
		fprintf(fp,"#define CH_AUX_DATA		0\n");
		fprintf(fp,"#define USER_PATTERN_500	%d\n", userPatternType);
		fprintf(fp,"#define SK_CALI_TYPE	0\n");
		fprintf(fp,"#define TEMP_CALI		0\n");
		fprintf(fp,"#define END_V_COMPARE_GOTO	0\n");

		if(cycler_type == 1) {
			fprintf(fp, "#define _JIG_TYPE_3\n");
		} else {
			fprintf(fp, "#define _JIG_TYPE_0\n");
		}
		
		fprintf(fp, "\n");
		fprintf(fp,"#define DAEHWA	%d\n", daehwa);
		fprintf(fp, "\n");

		fprintf(fp,"#if VENDER == 1
	#define __LG_VER1__
	#if DAEHWA == 1
		#define USER_VI	0
		#define CHANGE_VI_CHECK	0
		#define DATA_SAVE_VER	1
		#define VERSION_DETAIL_SHOW	0
	#else
		#define USER_VI	1
		#define CHANGE_VI_CHECK	1
		#define DATA_SAVE_VER	0
		#define VERSION_DETAIL_SHOW	1
	#endif
	#define PROGRAM_VERSION1	0
	#define PROGRAM_VERSION2	2
	#define PROGRAM_VERSION3	0
	#define PROGRAM_VERSION4	5
#elif VENDER == 2
	#define __SDI_MES_VER4__
	#define USER_VI	0
	#define CHANGE_VI_CHECK	0
	#define DATA_SAVE_VER	1
	#define VERSION_DETAIL_SHOW	0

	#define PROGRAM_VERSION1	0
	#define PROGRAM_VERSION2	1
	#define PROGRAM_VERSION3	0
	#define PROGRAM_VERSION4	5
#elif VENDER == 3
	#define __LG_VER1__
	#define USER_VI	0
	#define CHANGE_VI_CHECK	0
	#define DATA_SAVE_VER	0
	#define VERSION_DETAIL_SHOW	1

	#define PROGRAM_VERSION1	0
	#define PROGRAM_VERSION2	1
	#define PROGRAM_VERSION3	0
	#define PROGRAM_VERSION4	5
#endif\n");
	fprintf(fp, "\n");
	}
	fprintf(fp, "#endif\n");
	fclose(fp);

	printf("start compiling? [0:NO 1:Compile only 2:Compile and reboot]\n>>");
	loop = 1;
	while(loop) {
		scanf("%d", &value);
		while(getchar() != '\n');
		if(value >= 0 && value <= 2)
				loop = 0;
		else
			printf("Invalid value.\nValue is no valid. please enter again\n>>");
	}
	switch(value) {
		case 0:
			break;
		case 1:
//			memset(fileName, 0x00, sizeof(fileName));
//			sprintf(fileName, "%smakeall.sh", Scripts);
//			system(fileName);
			system("cd /project/Cycler/current/cycler/App/AppControl;make all");
			printf("Compile complete.\n");
			sleep(1);
			break;
		case 2:
//			memset(fileName, 0x00, sizeof(fileName));
//			sprintf(fileName, "%smakeallnreboot.sh", Scripts);
//			system(fileName);
			system("cd /project/Cycler/current/cycler/App/AppControl;make all;reboot");
			printf("SBC Rebooting...\n");
			sleep(5);
			break;
		default:
			break;
	}
	return 0;
}
*/

int makeDefault_AuxSetData(void)
{
	int i, val, loop;
	unsigned int ch;
	char fileName[128];
	FILE *fp;
	loop = 1;
	while(loop) {
		printf("Array Type Select (0:Pass, 1:AuxSetData) : ");
		scanf("%d", &val);
		while(getchar() != '\n');
		if(val >= 0 && val <=3) {
			switch(val) {
				case 0:
					loop = 0;
					break;
				case 1:
					printf("installedCh [1BASE]: ");
					scanf("%d", &ch);

					system("rm -rf /root/cycler_data/config/parameter/AuxSetData");
					system("touch /root/cycler_data/config/parameter/AuxSetData");
					memset(fileName, 0x00, sizeof(fileName));
					strcpy(fileName, "/root/cycler_data/config/parameter/AuxSetData");
					if((fp = fopen(fileName, "w")) == NULL) {
						printf("AuxSetData file write error\n");
						loop = 0;
						break;
					}
					for(i = 0 ; i < ch * 3 ; i ++){
						fprintf(fp, "auxChNo		: %d\n",i+1);
						fprintf(fp, "auxType		: %d\n",0);
						fprintf(fp, "chNo		: %d\n",(i/3)+1);
						fprintf(fp, "reserved1	: %d\n",0);
						fprintf(fp, "reserved2	: %d\n",0);
						fprintf(fp, "reserved3	: %d\n",0);
						fprintf(fp, "name		: temp%d\n",i+1);
						fprintf(fp, "fault_upper	: %d\n",0);
						fprintf(fp, "fault_lower	: %d\n",0);
						fprintf(fp, "end_upper	: %d\n",0);
						fprintf(fp, "end_lower	: %d\n",0);
						fprintf(fp, "func_div1	: %d\n",0);
						fprintf(fp, "func_div2	: %d\n",0);
						fprintf(fp, "func_div3	: %d\n",0);
						fprintf(fp, "reserved4	: %d\n",0);
						fprintf(fp, "\n");
					} 
					for(i = ch * 3; i < MAX_AUX_DATA; i++) {
						fprintf(fp, "auxChNo		: %d\n",0);
						fprintf(fp, "auxType		: %d\n",0);
						fprintf(fp, "chNo		: %d\n",0);
						fprintf(fp, "reserved1	: %d\n",0);
						fprintf(fp, "reserved2	: %d\n",0);
						fprintf(fp, "reserved3	: %d\n",0);
						fprintf(fp, "name		: temp%d\n",i+1);
						fprintf(fp, "fault_upper	: %d\n",0);
						fprintf(fp, "fault_lower	: %d\n",0);
						fprintf(fp, "end_upper	: %d\n",0);
						fprintf(fp, "end_lower	: %d\n",0);
						fprintf(fp, "func_div1	: %d\n",0);
						fprintf(fp, "func_div2	: %d\n",0);
						fprintf(fp, "func_div3	: %d\n",0);
						fprintf(fp, "reserved4	: %d\n",0);
						fprintf(fp, "\n");
					}
					fclose(fp);
					printf("\nAuxSetData file write success\n");
					loop = 0;
					break;
				default:
					printf("!!! value is no valid\n");
					loop = 1;
					val = 100;
					break;
			}
		} else {
			printf("!!! Please enter only number in that range\n");
		}
		printf("\n");
	}
	sleep(1);
	return 0;
}

int makeArray(void)
{
	int i, val, loop;
	unsigned int ch;
	char fileName[128];
	FILE *fp;

	loop = 1;
	val = 100;
	while(loop) {
		printf("Array Type Select (0:Pass, 1:CellArray, 2:TempArray, 3:ChamberChNo) : ");
		scanf("%d", &val);
		while(getchar() != '\n');
		if(val >= 0 && val <=3) {
			switch(val) {
				case 0:
					loop = 0;
					break;
				case 1:
					printf("chPerBd [1BASE]: ");
					scanf("%d", &ch);

					system("rm -rf /root/cycler_data/config/parameter/CellArray_A");
					system("touch /root/cycler_data/config/parameter/CellArray_A");
					memset(fileName, 0x00, sizeof(fileName));
					strcpy(fileName, "/root/cycler_data/config/parameter/CellArray_A");
					if((fp = fopen(fileName, "w")) == NULL) {
						printf("CellArray_A file write error\n");
						loop = 0;
						break;
					}
					fprintf(fp, "monitor_no\t:\thw_no\t:\tbd\t:\tch\n");
					for(i = 0; i < MAX_CH_PER_MODULE; i++) {
						fprintf(fp, "%d\t\t\t:\t", i+1);
						fprintf(fp, "%d\t\t:\t", i+1);
						fprintf(fp, "%d\t:\t", i/ch);	
						fprintf(fp, "%d\n", i%ch);	
					}
					
					fclose(fp);
					printf("\nCellArray_A file write success\n");
					loop = 0;
					break;
				case 2:
					printf("chPerBd [1BASE]: ");
					scanf("%d", &ch);
					
					system("rm -rf /root/cycler_data/config/parameter/TempArray_A");
					system("touch /root/cycler_data/config/parameter/TempArray_A");
					memset(fileName, 0x00, sizeof(fileName));
					strcpy(fileName, "/root/cycler_data/config/parameter/TempArray_A");
					if((fp = fopen(fileName, "w")) == NULL) {
						printf("TempArray_A file write error\n");
						loop = 0;
						break;
					}
					fprintf(fp, "monitor_no\t:\thw_no\t:\tbd\t:\tch\n");
					for(i = 0; i < MAX_CH_PER_MODULE; i++) {
						fprintf(fp, "%d\t\t\t:\t", i+1);
						fprintf(fp, "%d\t\t:\t", (i%100)+1);
						fprintf(fp, "%d\t:\t", i/ch);	
						fprintf(fp, "%d\n", i%ch);	
					}
					for(i=i; i < MAX_TEMP_CH; i++) {
						fprintf(fp, "%d\t\t\t:\t", i+1);
						fprintf(fp, "%d\t\t:\t", (i%100)+1);
						fprintf(fp, "%d\t:\t", 0);	
						fprintf(fp, "%d\n", 0);	
					}
					
					fclose(fp);
					loop = 0;
					printf("\nTempArray_A file write success\n");
					break;
				case 3:
					printf("chPerBd [1BASE]: ");
					scanf("%d", &ch);

					system("rm -rf /root/cycler_data/config/parameter/ChamberChNo");
					system("touch /root/cycler_data/config/parameter/ChamberChNo");
					memset(fileName, 0x00, sizeof(fileName));
					strcpy(fileName, "/root/cycler_data/config/parameter/ChamberChNo");
					if((fp = fopen(fileName, "w")) == NULL) {
						printf("ChamberChNo file write error\n");
						loop = 0;
						break;
					}
					fprintf(fp, "chamberChNo\t:\thw_no\t:\tbd\t:\tch\n");
					for(i = 0; i < MAX_CH_PER_MODULE; i++) {
						fprintf(fp, "%d\t\t\t:\t", 0);
						fprintf(fp, "%d\t\t:\t", i+1);
						fprintf(fp, "%d\t:\t", i/ch);	
						fprintf(fp, "%d\n", i%ch);	
					}
					
					fclose(fp);
					printf("\nChamberChNo file write success\n");
					loop = 0;
					break;

				default:
					printf("!!! value is no valid\n");
					loop = 1;
					val = 100;
					break;
			}
		} else {
			printf("!!! Please enter only number in that range\n");
		}
		printf("\n");
	}
	sleep(1);
	return 0;
}

int makeDefault_CALI_BD(void)
{
    int	bd, ch, type, range, point, chPerBd, installedBd, answer = 0;
	char temp[4], fileName[128];
    FILE *fp;

	struct tm *date;
	const time_t t = time(NULL);
	date = localtime(&t);

	printf("Remove previous 'caliData_org/CALI_BD' files ? [0:NO 1:YES] : ");
	scanf("%d", &answer);
	if(answer == 1) {
		system("rm -rf /root/cycler_data/config/caliData_org/CALI_BD*");
	}
	printf("installedBd : ");
	scanf("%d", &installedBd);
	printf("chPerBd : ");
	scanf("%d", &chPerBd);
	
	if(installedBd > 16)
		installedBd = 16;

	if(chPerBd > MAX_CH_PER_BD)
		chPerBd = MAX_CH_PER_BD;

	for(bd = 0; bd < installedBd; bd++) {
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

    if((fp = fopen(fileName, "w")) == NULL) {
		printf("Can not open %d Board CaliData file(write)\n", bd+1);
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

				fprintf(fp, "DA_B			\n");
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
	printf(">>Default BD CaliData file make success\n");
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
	strcat(fileName, "_I_Offset");
	// /root/cycler_data/config/caliData/CALI_BD#_I_Offset
	
    if((fp = fopen(fileName, "w+")) == NULL) {
		printf("Can not open BD[%d] I Offset CaliData file(write)\n", bd+1);
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
				
				fprintf(fp, "AD_A          \n");
				for(point=0; point < MAX_CALI_POINT-1; point++) {
				   	fprintf(fp, "%f ", 1.0);
				}
				fprintf(fp, "\n");

				fprintf(fp, "AD_B\n");
				for(point=0; point < MAX_CALI_POINT-1; point++) {
				   	fprintf(fp, "%f ", 0.0);
				}
				fprintf(fp, "\n\n");
			}
		}
	}
   	fclose(fp);
	printf(">>Default BD I Offset CaliData file make success\n");

	
	}
	printf("Copy 'CALI_BD' files to 'caliData' Directory now? [0:NO, 1:YES] : ");
	scanf("%d", &answer);
	if(answer == 1) {
		system("rm -rf /root/cycler_data/config/caliData/CALI_BD*");
		system("cp -rf /root/cycler_data/config/caliData_org/CALI_BD* /root/cycler_data/config/caliData");
	}
	sleep(1);
	return 0;
}

int selectType2(void)
{	
	//config
	unsigned int	installedBd = 1;
	unsigned int	installedCh = 0;
	unsigned int	chPerBd = 0;
	unsigned short	rangeV = 1;
	unsigned short	rangeI = 1;
	long maxVoltage[4] = {0,};
	long minVoltage[4] = {0,};	
	long maxCurrent[4] = {0,};
	long minCurrent[4] = {0,};
	unsigned char	ratioVoltage = 0;
	unsigned char	ratioCurrent = 0;
	unsigned char	watchdogFlag = 1;
	unsigned char	ADC_type = 1;
	unsigned char	hwSpec = 0;
	unsigned char	capacityType = 0;
	unsigned int	totalJig = 1;
	unsigned int	bdInJig[16] = {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	float			shunt[4] = {0,};	
	float			gain[4] = {0,};
	float			adAmp = 1.494;
	float			voltageAmp;	
	float			currentAmp;	
	unsigned short rt_scan_type = 0;
	unsigned short installedTemp = 0;
	unsigned short installedAuxV = 0;
	unsigned short installedCAN = 0;
	unsigned char 	auto_v_cali = 1;
	int				parallelMode = 0;
	unsigned char	DAC_type = 1;
	unsigned char	ADC_num = 2;
	float			AD_offset = 0.00002;
	int				SoftFeedbackFlag = 1;
	int				MainBdType = 1;
	int				FadBdUse = 0;
	int				range[5][4];
	
	int portPerCh = 0;
	int	installedInverter = 0;
	double	hallCT = 0.0006666;
	float caliV_ohm = 1.0;
	unsigned short pcuCaliUse = 1;
	int	powerNo	= 2;
	float caliV_amp = 2;
	unsigned short setCaliNum = 3;
	unsigned short cali_delay_time = 3000;
	int	invPerCh = 0;
	int parallel_inv_ch = 0;
	int inverterType = 0;
	
	int i = 0;
	int voltage;
	int voltage2;
	int	moduleFlag = 0;
	int	moduleFlag2 = 0;
	int ad_count = 0;
	int	extension_bd_use = 0;
   	double current[4];
	float control_ohm = 0;
	float control_voltage = 0;
	int cyclerType, type; 
	int val, loop;
	int value = 0;
	char parameter[64], backup[64], cmd[256], backup2[64];
	int pcu_inUseFlag[16] = {0,};
	int pcu_inActiveType[16] = {0,};
	char fileName[128];
	int customer = 0;
	FILE *fp;
	int function[26];
	int chamberControl = 0;

	loop = 1;
	val = 100;
	range[0][0] = 0;	range[0][1] = 0;	range[0][2] = 0;	range[0][3] = 0;
	range[1][0] = 0;	range[1][1] = 0;	range[1][2] = 0;	range[1][3] = 0;
	range[2][0] = 0;	range[2][1] = 1;	range[2][2] = 0;	range[2][3] = 0;
	range[3][0] = 0;	range[3][1] = 0;	range[3][2] = 1;	range[3][3] = 0;
	range[4][0] = 0;	range[4][1] = 0;	range[4][2] = 0;	range[4][3] = 0;

	function[0] = 1;	function[1] = 1;	function[2] = 1;	function[3] = 0;
	function[4] = 1;	function[5] = 1;	function[6] = 0;	function[7] = 0;
	function[8] = 0;	function[9] = 0;	function[10] = 0;	function[11] = 0;
	function[12] = 0;	function[13] = 2;	function[14] = 1;	function[15] = 1;
	function[16] = 0;	function[17] = 0;	function[18] = 0;	function[19] = 0;
	function[20] = 0;	function[21] = 0;	function[22] = 0;	function[23] = 0;
	function[24] = 0;	function[25] = 6;	

	memset(parameter, 0x00, sizeof(parameter));
	strcpy(parameter, "/root/cycler_data/config/parameter");
	memset(backup, 0x00, sizeof(backup));
	strcpy(backup, "/root/cycler_data/config/parameter/0_Backup");
	memset(backup2, 0x00, sizeof(backup2));
	strcpy(backup2, "/project/Cycler/current/cycler/INC");
	
	while(loop) {
		printf("챔버 연동 여부 사용(1) 미사용(0) : ");
		scanf("%d", &chamberControl);
		if(chamberControl == 0){
			function[9] = 0;	
		}else{
			function[9] = 1;	
		}
		printf("고객사 설정 (0:LG, 1:삼성SDI, 2:SK,  3:노스볼트, 4:현대자동차, 5:기타업체) : ");
		scanf("%d", &customer);
		printf("Cycler Type Select (0:리니어 ,1:스위칭, 2:pass) : ");
		scanf("%d", &cyclerType);
		while(getchar() != '\n');
			switch(cyclerType) {
				case 0:
					memset(cmd, 0x00, sizeof(cmd));
					sprintf(cmd, "cp -rf %s/AppControl_Config_linear %s/AppControl_Config", backup, parameter);
					system(cmd);
					memset(cmd, 0x00, sizeof(cmd));
					sprintf(cmd, "cp -rf %s/Addr_Interface_linear %s/Addr_Interface", backup, parameter);
					system(cmd);
					memset(cmd, 0x00, sizeof(cmd));
					sprintf(cmd, "cp -rf %s/Addr_Main_linear %s/Addr_Main", backup, parameter);
					system(cmd);
					memset(cmd, 0x00, sizeof(cmd));
					sprintf(cmd, "cp -rf %s/DIN_USE_FLAG_linear %s/DIN_USE_FLAG", backup, parameter);
					system(cmd);
					memset(cmd, 0x00, sizeof(cmd));
					sprintf(cmd, "cp -rf %s/DOUT_USE_FLAG_linear %s/DOUT_USE_FLAG", backup, parameter);
					system(cmd);
					memset(cmd, 0x00, sizeof(cmd));
					sprintf(cmd, "cp -rf %s/mControl_Config_linear %s/mControl_Config", backup, parameter);
					system(cmd);
					memset(cmd, 0x00, sizeof(cmd));
					sprintf(cmd, "cp -rf %s/CaliMeter_Config %s/CaliMeter_Config", backup, parameter);
					system(cmd);
	
					printf("제어 옴 입력 ex) 560옴 -> 560 엔터 : ");
					scanf("%f", &control_ohm);
					printf("제어 전압 입력 ex) 100mV -> 100 엔터 : ");
					scanf("%f", &control_voltage);
					printf("(+)전압 입력(V) : ");
					scanf("%d", &voltage);
					if(voltage >= 8){
						function[0] = 0;	//모듈형
						function[8] = 1;	//모듈형
						adAmp = 1.;
						moduleFlag = 1;
						voltageAmp = voltage / 8.;	
					}else{
						voltageAmp = 1.0;	
					}
					maxVoltage[0] = (long)(voltage * 1000000);
					printf("(-)전압 입력(V) : ");
					scanf("%d", &voltage2);
					minVoltage[0] = (long)(voltage2 * 1000000);
					printf("전류 Range 갯수(1~4): ");
					scanf("%hd",&rangeI);
					if(rangeI == 1){
						printf("1Ragne 전류(A) : ");
						scanf("%lf", &current[0]);
						printf("1Ragne shunt 저항(ex. 10mOhm -> 0.01) : ");
						scanf("%f", &shunt[0]);
					}else if(rangeI == 2){
						printf("1Ragne 전류(A) : ");
						scanf("%lf", &current[0]);
						printf("2Ragne 전류(A) : ");
						scanf("%lf", &current[1]);
						printf("1Ragne shunt 저항(ex. 10mOhm -> 0.01 입력 : ");
						scanf("%f", &shunt[0]);
						printf("2Ragne shunt 저항(ex. 10mOhm -> 0.01 입력 : ");
						scanf("%f", &shunt[1]);
					}else if(rangeI == 3){
						printf("1Ragne 전류(A) : ");
						scanf("%lf", &current[0]);
						printf("2Ragne 전류(A) : ");
						scanf("%lf", &current[1]);
						printf("3Ragne 전류(A) : ");
						scanf("%lf", &current[2]);
						printf("1Ragne shunt 저항(ex. 10mOhm -> 0.01 입력 : ");
						scanf("%f", &shunt[0]);
						printf("2Ragne shunt 저항(ex. 10mOhm -> 0.01 입력 : ");
						scanf("%f", &shunt[1]);
						printf("3Ragne shunt 저항(ex. 10mOhm -> 0.01 입력 : ");
						scanf("%f", &shunt[2]);
					}else if(rangeI == 4){
						printf("1Ragne 전류(A) : ");
						scanf("%lf", &current[0]);
						printf("2Ragne 전류(A) : ");
						scanf("%lf", &current[1]);
						printf("3Ragne 전류(A) : ");
						scanf("%lf", &current[2]);
						printf("4Ragne 전류(A) : ");
						scanf("%lf", &current[3]);
						printf("1Ragne shunt 저항(ex. 10mOhm -> 0.01 입력 : ");
						scanf("%f", &shunt[0]);
						printf("2Ragne shunt 저항(ex. 10mOhm -> 0.01 입력 : ");
						scanf("%f", &shunt[1]);
						printf("3Ragne shunt 저항(ex. 10mOhm -> 0.01 입력 : ");
						scanf("%f", &shunt[2]);
						printf("4Ragne shunt 저항(ex. 10mOhm -> 0.01 입력 : ");
						scanf("%f", &shunt[3]);
						range[4][0] = 0;	range[4][1] = 0;	range[4][2] = 0;	range[4][3] = 1;
					}
					if(current[0] > 20){	//중대형
						type = 0;
						hwSpec = 88;
					}else if(current[0] == 20){	//20A
						type = 1;
						hwSpec = 90;
						range[1][0] = 0;	range[1][1] = 1;	range[1][2] = 1;	range[1][3] = 0;
					}else if(current[0] <= 10 && current[0] >= 1){	//저전류
						memset(cmd, 0x00, sizeof(cmd));
						sprintf(cmd, "cp -rf %s/Addr_Main_linear_10A %s/Addr_Main", backup, parameter);
						system(cmd);
						type = 2;
						hwSpec = 82;
					}else if(current[0] < 1){	//코인셀
						memset(cmd, 0x00, sizeof(cmd));
						sprintf(cmd, "cp -rf %s/Addr_Main_linear_10A %s/Addr_Main", backup, parameter);
						system(cmd);
						type = 2;
						hwSpec = 81;
						ratioCurrent = 1;
					}
					if(type == 0 || type == 1){
						printf("한 유닛당(SBC) 전체 채널 수  : ");
						scanf("%d",&installedCh);
						printf("Main Board 개수 : ");
						scanf("%d",&installedBd);
						printf("Main Board 하나 당 채널 제어 개수(chPerBd) : ");
						scanf("%d",&chPerBd);
						printf("Main Borad 당 ADC 칩 개수(default : 2) : ");
						scanf("%d", &ad_count);
					}else if(type == 2){
						printf("한 유닛당(SBC) 전체 채널 수  : ");
						scanf("%d",&installedCh);
						printf("BackPlan Borad 개수 : ");
						scanf("%d",&installedBd);
						printf("BackPlan Board 하나 당 채널 제어 개수(chPerBd) : ");
						scanf("%d",&chPerBd);
						printf("BackPlan Board 당 ADC 칩 개수(default : 2) : ");
						scanf("%d", &ad_count);
					}
					if(current[0] == 1){
						if(chPerBd == 64){
							hwSpec = 81;
							ratioCurrent = 1;
							currentAmp = (49.4 / (control_ohm/1000.) + 1.) * (control_voltage / 100.);	
						}
					}
					if(ratioCurrent == 0){
						currentAmp = (49.4 / (control_ohm/1000.) + 1.) / adAmp * (control_voltage / 100.);	
					}else if(ratioCurrent == 1){
						currentAmp = (49.4 / (control_ohm/1000.) + 1.) * (control_voltage / 100.);	
					}
					for(i = rangeI ; i < 4 ; i++){
						maxCurrent[i] = 0;
						minCurrent[i] = 0;
						shunt[i] = 0;
						gain[i] = 0;
					}
					for(i = 0 ; i < rangeI ; i++){
						if(ratioCurrent == 1){
							maxCurrent[i] = (long)(current[i] * 1000000000);
							minCurrent[i] = maxCurrent[i] * (-1);
							gain[i] = ((8. / current[i]) / shunt[i]);
						}else{
							maxCurrent[i] = (long)(current[i] * 1000000);
							minCurrent[i] = maxCurrent[i] * (-1);
							gain[i] = ((8. / current[i]) / shunt[i]);
						}
					}

					if((chPerBd == 32 && ad_count == 1 )|| (chPerBd == 64 && ad_count == 2)){
						rt_scan_type = 0;	//100mS
					}else if((chPerBd == 16 && ad_count == 1) || (chPerBd == 32 && ad_count == 2)){
						rt_scan_type = 1;	//50mS
					}else if(chPerBd == 16 && ad_count == 2){ 
						rt_scan_type = 2;	//25mS
					}else if(chPerBd == 8 && ad_count == 2){ 
						rt_scan_type = 3;	//20mS
					}else if(chPerBd == 64 && ad_count == 1){ 
						rt_scan_type = 4;	//100mS_2
					}else if(chPerBd == 8 && ad_count == 2){ 
						rt_scan_type = 5;	//10mS
					}else{
						rt_scan_type = 100;
					}
					printf("병렬모드 사용 유/무 ex)신병렬 : 2, 사용 : 1, 미사용 : 0 : ");
					scanf("%d",&parallelMode);
					printf("보조 온도 개수  : ");
					scanf("%hd",&installedTemp);
					printf("보조 전압 개수  : ");
					scanf("%hd",&installedAuxV);
					if(moduleFlag == 1){
						if(parallelMode == 1) hwSpec = 87;
						else if(parallelMode == 0) hwSpec = 85;
						printf("과충방전기 이면 (1) 아니면 (0) 입력 : ");
						scanf("%d",&moduleFlag2);
						if(moduleFlag2 == 1){
							function[0] = 0;	function[2] = 0;  function[4] = 0;
						}
					}
					if(installedTemp > 0){
						memset(cmd, 0x00, sizeof(cmd));
						sprintf(cmd, "cp -rf %s/AnalogMeter_Config_Aux %s/AnalogMeter_Config", backup, parameter);
						system(cmd);
					}
					if(rt_scan_type == 100){
						if(type == 1){
							printf("중대형 rt_scan_type 오류\n");
							printf("chPerBd : %d , MainBoard 당 AD 개수 : %d 확인 필요\n",chPerBd,ad_count);
						}else if(type == 2){
							printf("저전류 rt_scan_type 오류\n");
							printf("chPerBd : %d , BackPlan Board 당 AD 개수 : %d 확인 필요\n",chPerBd,ad_count);
						}
					}else{
					printf("리니어 mControl_Config\n");
					printf("installedBd		:	%d\n",installedBd);
					printf("installedCh		:	%d\n",installedCh);
					printf("chPerBd			:	%d\n",chPerBd);
					printf("rangeV			:	%d\n",rangeV);
					printf("rangeI			:	%d\n",rangeI);
					printf("maxVoltage		:	%ld %ld %ld %ld\n",maxVoltage[0],maxVoltage[1],maxVoltage[2],maxVoltage[3]);
					printf("minVoltage		:	%ld %ld %ld %ld\n",minVoltage[0],minVoltage[1],minVoltage[2],minVoltage[3]);
					printf("maxCurrent		:	%ld %ld %ld %ld\n",maxCurrent[0],maxCurrent[1],maxCurrent[2],maxCurrent[3]);
					printf("minCurrent		:	%ld %ld %ld %ld\n",minCurrent[0],minCurrent[1],minCurrent[2],minCurrent[3]);
					printf("ratioVoltage		:	%d\n",ratioVoltage);
					printf("ratioCurrent		:	%d\n",ratioCurrent);
					printf("watchdogFlag		:	%d\n",watchdogFlag);
					printf("ADC_type		:	%d\n",ADC_type);
					printf("hwSpec			:	%d\n",hwSpec);
					printf("capacityType		:	%d\n",capacityType);
					printf("totalJig		:	%d\n",totalJig);
					printf("bdInJig1_8		:	%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n",bdInJig[0],bdInJig[1],bdInJig[2],bdInJig[3],
						bdInJig[4],bdInJig[5],bdInJig[6],bdInJig[7]);
					printf("bdInJig1_16		:	%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n",bdInJig[8],bdInJig[9],bdInJig[10],bdInJig[11],
						bdInJig[12],bdInJig[13],bdInJig[14],bdInJig[15]);
					printf("shuntOhm		:	%f\t%f\t%f\t%f\n",shunt[0],shunt[1],shunt[2], shunt[3]);
					printf("gain			:	%f\t%f\t%f\t%f\n",gain[0],gain[1],gain[2], gain[3]);
					printf("adAmp			:	%f\t%f\t%f\n", adAmp,voltageAmp,currentAmp);
					printf("rt_scan_type		:	%d\n",rt_scan_type);
					printf("installedTemp		:	%d\n",installedTemp);	
					printf("installedAuxV		:	%d\n",installedAuxV);	
					printf("installedCan		:	%d\n",installedCAN);	
					printf("auto_v_cali		:	%d\n",auto_v_cali);	
					printf("parallelMode		:	%d\n",parallelMode);	
					printf("DAC_type		:	%d\n",DAC_type);	
					printf("ADC_num			:	%d\n",ADC_num);
					printf("AD_offset		:	%f\n",AD_offset);
					printf("FeedbackFlag		:	%d\n",SoftFeedbackFlag);
					printf("MainBdType		:	%d\n",MainBdType);
					printf("FadBdUse		:	%d\n",FadBdUse);
					printf("RangeSelectFlag		:	0x10\t0x20\t0x40\t0x80\n");
					printf("Range0			:	%d\t%d\t%d\t%d\n",range[0][0],range[0][1],range[0][2],range[0][3]);
					printf("Range1			:	%d\t%d\t%d\t%d\n",range[1][0],range[1][1],range[1][2],range[1][3]);
					printf("Range2			:	%d\t%d\t%d\t%d\n",range[2][0],range[2][1],range[2][2],range[2][3]);
					printf("Range3			:	%d\t%d\t%d\t%d\n",range[3][0],range[3][1],range[3][2],range[3][3]);
					printf("Range4			:	%d\t%d\t%d\t%d\n",range[4][0],range[4][1],range[4][2],range[4][3]);
					}
					printf("\n\n");
					if(customer == 0){ //LG
						function[20] = 1;
						function[21] = 2;
						if(installedTemp == 0 && installedAuxV == 0){
							memset(cmd, 0x00, sizeof(cmd));
							sprintf(cmd, "cp -rf %s/Backup/SysConfig_1_4102_Linear.h %s/SysConfig.h", backup2, backup2);
							system(cmd);
						}else{
							memset(cmd, 0x00, sizeof(cmd));
							sprintf(cmd, "cp -rf %s/Backup/SysConfig_1_4103_Linear.h %s/SysConfig.h", backup2, backup2);
							system(cmd);
						}
						memset(cmd, 0x00, sizeof(cmd));
						sprintf(cmd, "cp -rf %s/HwFault_Config %s/HwFault_Config", backup, parameter);
						system(cmd);
						memset(cmd, 0x00, sizeof(cmd));
						sprintf(cmd, "cp -rf %s/SwFault_Config %s/SwFault_Config", backup, parameter);
						system(cmd);
					}else if(customer == 1){ //SDI
						function[10] = 1;
						function[24] = 1;
						memset(cmd, 0x00, sizeof(cmd));
						sprintf(cmd, "cp -rf %s/HwFault_Config_linear %s/HwFault_Config", backup, parameter);
						system(cmd);
						memset(cmd, 0x00, sizeof(cmd));
						sprintf(cmd, "cp -rf %s/SwFault_Config_other %s/SwFault_Config", backup, parameter);
						system(cmd);
						memset(cmd, 0x00, sizeof(cmd));
						sprintf(cmd, "cp -rf %s/Backup/SysConfig_2_4105_Linear.h %s/SysConfig.h", backup2, backup2);
						system(cmd);
					}else if(customer == 2){ //SK
						function[9] = 1;
						function[18] = 1;
						function[19] = 1;
						memset(cmd, 0x00, sizeof(cmd));
						sprintf(cmd, "cp -rf %s/HwFault_Config_linear %s/HwFault_Config", backup, parameter);
						system(cmd);
						memset(cmd, 0x00, sizeof(cmd));
						sprintf(cmd, "cp -rf %s/SwFault_Config_other %s/SwFault_Config", backup, parameter);
						system(cmd);
						if(installedTemp == 0 && installedAuxV == 0){
							memset(cmd, 0x00, sizeof(cmd));
							sprintf(cmd, "cp -rf %s/Backup/SysConfig_3_4102_Linear.h %s/SysConfig.h", backup2, backup2);
							system(cmd);
						}else{
							memset(cmd, 0x00, sizeof(cmd));
							sprintf(cmd, "cp -rf %s/Backup/SysConfig_3_4103_Linear.h %s/SysConfig.h", backup2, backup2);
							system(cmd);
						}
					}else if(customer == 3){ //NorthVolt
						function[12] = 1;
						function[21] = 2;
						function[22] = 1;
						function[23] = 1;
						memset(cmd, 0x00, sizeof(cmd));
						sprintf(cmd, "cp -rf %s/SwFault_Config_other %s/SwFault_Config", backup, parameter);
						system(cmd);
						memset(cmd, 0x00, sizeof(cmd));
						sprintf(cmd, "cp -rf %s/HwFault_Config_digital %s/HwFault_Config", backup, parameter);
						system(cmd);
						if(installedTemp == 0 && installedAuxV == 0){
							memset(cmd, 0x00, sizeof(cmd));
							sprintf(cmd, "cp -rf %s/Backup/SysConfig_1_4102_Linear.h %s/SysConfig.h", backup2, backup2);
							system(cmd);
						}else{
							memset(cmd, 0x00, sizeof(cmd));
							sprintf(cmd, "cp -rf %s/Backup/SysConfig_1_4103_Linear.h %s/SysConfig.h", backup2, backup2);
							system(cmd);
						}
					}else if(customer == 4){ //현대
						function[21] = 1;
						memset(cmd, 0x00, sizeof(cmd));
						sprintf(cmd, "cp -rf %s/HwFault_Config_linear %s/HwFault_Config", backup, parameter);
						system(cmd);
						memset(cmd, 0x00, sizeof(cmd));
						sprintf(cmd, "cp -rf %s/SwFault_Config_other %s/SwFault_Config", backup, parameter);
						system(cmd);
						if(installedTemp == 0 && installedAuxV == 0){
							memset(cmd, 0x00, sizeof(cmd));
							sprintf(cmd, "cp -rf %s/Backup/SysConfig_1_4102_LInear.h %s/SysConfig.h", backup2, backup2);
							system(cmd);
						}else{
							memset(cmd, 0x00, sizeof(cmd));
							sprintf(cmd, "cp -rf %s/Backup/SysConfig_1_4103_Linear.h %s/SysConfig.h", backup2, backup2);
							system(cmd);
						}
					}else if(customer == 5){ //기타업체
						function[21] = 2;
						memset(cmd, 0x00, sizeof(cmd));
						sprintf(cmd, "cp -rf %s/HwFault_Config_linear %s/HwFault_Config", backup, parameter);
						system(cmd);
						memset(cmd, 0x00, sizeof(cmd));
						sprintf(cmd, "cp -rf %s/SwFault_Config_other %s/SwFault_Config", backup, parameter);
						system(cmd);
						if(installedTemp == 0 && installedAuxV == 0){
							memset(cmd, 0x00, sizeof(cmd));
							sprintf(cmd, "cp -rf %s/Backup/SysConfig_1_4102_Linear.h %s/SysConfig.h", backup2, backup2);
							system(cmd);
						}else{
							memset(cmd, 0x00, sizeof(cmd));
							sprintf(cmd, "cp -rf %s/Backup/SysConfig_1_4103_Linear.h %s/SysConfig.h", backup2, backup2);
							system(cmd);
						}
					}
					printf("OVP			:	%d\n",function[0]);
					printf("ChCheckCurrent		:	%d\n",function[1]);
					printf("OTP			:	%d\n",function[2]);
					printf("MAIN_BD_OT		:	%d\n",function[3]);
					printf("HW_FAULT_COND		:	%d\n",function[4]);
					printf("SW_FAULT_COND		:	%d\n",function[5]);
					printf("MINUS_CELL		:	%d\n",function[6]);
					printf("I_OFFEST_CALI		:	%d\n",function[7]);
					printf("SEMI_SWITCH_TYPE	:	%d\n",function[8]);
					printf("CHAMBER_TEMP_WAIT	:	%d\n",function[9]);
					printf("SDI_MES_USE		:	%d\n",function[10]);
					printf("OT_PAUSE		:	%d\n",function[11]);
					printf("CHAMBER_ERROR_PROC	:	%d\n",function[12]);
					printf("SENS_COUNT_TYPE		:	%d\n",function[13]);
					printf("V_SENS_REM_NO		:	%d\n",function[14]);
					printf("I_SENS_REM_NO		:	%d\n",function[15]);
					printf("DELTA_V_I		:	%d\n",function[16]);
					printf("CHAMBER_TYPE		:	%d\n",function[17]);
					printf("PATTERN_FTP		:	%d\n",function[18]);
					printf("SBC_RECOVERY		:	%d\n",function[19]);
					printf("PAUSE_DATA_SAVE		:	%d\n",function[20]);
					printf("CHANGE_VI_CHECK		:	%d\n",function[21]);
					printf("DCR_TYPE		:	%d\n",function[22]);
					printf("PATTERN_PROCESS		:	%d\n",function[23]);
					printf("PATTERN_CH_SAVE		:	%d\n",function[24]);
					printf("DISCONNECT_DAY		:	%d\n",function[25]);

					//loop = 0;
					printf("\n입력완료. 파일에 저장 하시겠습니까? [0:아니오 1:예]");
					scanf("%d", &value);
					if(value == 0) return -1;
				
					system("rm -rf /root/cycler_data/config/parameter/mControl_Config");
					system("touch /root/cycler_data/config/parameter/mControl_Config");
					memset(fileName, 0x00, sizeof(fileName));
					strcpy(fileName, "/root/cycler_data/config/parameter/mControl_Config");
					if((fp = fopen(fileName, "w")) == NULL) {
						printf("mControl_Config write error\n");
						loop = 0;
						break;
					}
					// Save File
					fprintf(fp,"installedBd			:	%d\n",installedBd);
					fprintf(fp,"installedCh			:	%d\n",installedCh);
					fprintf(fp,"chPerBd				:	%d\n",chPerBd);
					fprintf(fp,"rangeV				:	%d\n",rangeV);
					fprintf(fp,"rangeI				:	%d\n",rangeI);
					fprintf(fp,"maxVoltage			:	%ld %ld %ld %ld\n",maxVoltage[0],maxVoltage[1],maxVoltage[2],maxVoltage[3]);
					fprintf(fp,"minVoltage			:	%ld %ld %ld %ld\n",minVoltage[0],minVoltage[1],minVoltage[2],minVoltage[3]);
					fprintf(fp,"maxCurrent			:	%ld %ld %ld %ld\n",maxCurrent[0],maxCurrent[1],maxCurrent[2],maxCurrent[3]);
					fprintf(fp,"minCurrent			:	%ld %ld %ld %ld\n",minCurrent[0],minCurrent[1],minCurrent[2],minCurrent[3]);
					fprintf(fp,"ratioVoltage		:	%d\n",ratioVoltage);
					fprintf(fp,"ratioCurrent		:	%d\n",ratioCurrent);
					fprintf(fp,"watchdogFlag		:	%d\n",watchdogFlag);
					fprintf(fp,"ADC_type			:	%d\n",ADC_type);
					fprintf(fp,"hwSpec				:	%d\n",hwSpec);
					fprintf(fp,"capacityType		:	%d\n",capacityType);
					fprintf(fp,"totalJig			:	%d\n",totalJig);
					fprintf(fp,"bdInJig1_8			:	%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n",bdInJig[0],bdInJig[1],bdInJig[2],bdInJig[3],
						bdInJig[4],bdInJig[5],bdInJig[6],bdInJig[7]);
					fprintf(fp,"bdInJig1_16			:	%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n",bdInJig[8],bdInJig[9],bdInJig[10],bdInJig[11],
						bdInJig[12],bdInJig[13],bdInJig[14],bdInJig[15]);
					fprintf(fp,"shuntOhm			:	%f\t%f\t%f\t%f\n",shunt[0],shunt[1],shunt[2], shunt[3]);
					fprintf(fp,"gain				:	%f\t%f\t%f\t%f\n",gain[0],gain[1],gain[2], gain[3]);
					fprintf(fp,"adAmp				:	%f\t%f\t%f\n", adAmp,voltageAmp,currentAmp);
					fprintf(fp,"rt_scan_type		:	%d\n",rt_scan_type);
					fprintf(fp,"installedTemp		:	%d\n",installedTemp);	
					fprintf(fp,"installedAuxV		:	%d\n",installedAuxV);	
					fprintf(fp,"installedCan		:	%d\n",installedCAN);	
					fprintf(fp,"auto_v_cali			:	%d\n",auto_v_cali);	
					fprintf(fp,"parallelMode		:	%d\n",parallelMode);	
					fprintf(fp,"DAC_type			:	%d\n",DAC_type);	
					fprintf(fp,"ADC_num				:	%d\n",ADC_num);
					fprintf(fp,"AD_offset			:	%f\n",AD_offset);
					fprintf(fp,"FeedbackFlag		:	%d\n",SoftFeedbackFlag);
					fprintf(fp,"MainBdType			:	%d\n",MainBdType);
					fprintf(fp,"FadBdUse			:	%d\n",FadBdUse);
					fprintf(fp,"RangeSelectFlag		:	0x10\t0x20\t0x40\t0x80\n");
					fprintf(fp,"Range0				:	%d\t\t%d\t\t%d\t\t%d\n",range[0][0],range[0][1],range[0][2],range[0][3]);
					fprintf(fp,"Range1				:	%d\t\t%d\t\t%d\t\t%d\n",range[1][0],range[1][1],range[1][2],range[1][3]);
					fprintf(fp,"Range2				:	%d\t\t%d\t\t%d\t\t%d\n",range[2][0],range[2][1],range[2][2],range[2][3]);
					fprintf(fp,"Range3				:	%d\t\t%d\t\t%d\t\t%d\n",range[3][0],range[3][1],range[3][2],range[3][3]);
					fprintf(fp,"Range4				:	%d\t\t%d\t\t%d\t\t%d\n",range[4][0],range[4][1],range[4][2],range[4][3]);
					fprintf(fp, "\n");	
					fprintf(fp, "0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 \n");
				 	fprintf(fp, "0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 \n");
					fprintf(fp, "================================================================================\n");
					fprintf(fp, "Option Setting Description\n");
					fprintf(fp, "Update : 2021.11.29\n");
					fprintf(fp, "Writer : Hun\n");
					fprintf(fp, "================================================================================\n");
					fprintf(fp, "installedBd			=> insalled MainBoard No.\n");
					fprintf(fp, "installedCh			=> installed Total Ch\n");
					fprintf(fp, "chPerBd				=> Ch Per MainBD\n");
					fprintf(fp, "rangeV				=> Voltage Range No.\n");
					fprintf(fp, "rangeI				=> Current Range No.\n");
					fprintf(fp, "maxVoltage\n");
					fprintf(fp, "minVoltage\n");
					fprintf(fp, "maxCurrent\n");
					fprintf(fp, "minCurrent\n");
					fprintf(fp, "ratioVoltage		=> 0 : Micro, 1 : Nano (Default : 0)\n");
					fprintf(fp, "ratioCurrent		=> 0 : Micro, 1 : Nano (Default : 0)\n");
					fprintf(fp, "watchdogFlag	 	=> WatchDog UseFlag (Default : 1)	\n");
					fprintf(fp, "ADC_type			=> 0 : COMM_TYPE, 1 : 7805 (Default : 1)\n");
					fprintf(fp, "hwSpec				=> H/W SPEC No. ex) 63 : L_5V_150A_R3_AD2\n");
					fprintf(fp, "capacityType		=> 0 : AmpareHour, 1 : Farad\n");
					fprintf(fp, "totalJig\n");
					fprintf(fp, "bdInJig1_8\n");
					fprintf(fp, "bdInJig9_16\n");
					fprintf(fp, "shuntOhm			=> Range Shunt Value\n");
					fprintf(fp, "gain				=> Range Gain Value\n");
					fprintf(fp, "adAmp				=> AD_AMP	VOLTAGE_AMP	 CURRENT_AMP\n");
					fprintf(fp, "rt_scan_type		=> 0 : 100ms	=> ADC 1EA, chPerBD <= 32CH\n");
					fprintf(fp, "									   ADC 2EA, chPerBD <= 64CH\n");
					fprintf(fp, "					   1 : 50ms		=> ADC 1EA, chPerBD <= 16CH\n");
					fprintf(fp, "									   ADC 2EA, chPerBD <= 32CH\n");
					fprintf(fp, "					   2 : 25ms		=> ADC 2EA, chPerBD <= 16CH\n");
					fprintf(fp, "					   3 : 20ms		=> ADC 2EA, chPerBD <=  8CH\n");
					fprintf(fp, "					   4 : 100ms_2	=> ADC 1EA, chPerBD <= 64CH\n");
					fprintf(fp, "					   5 : 10ms		=> ADC 2EA, chPerBD <=  8CH\n");
					fprintf(fp, "					   6 : 100ms_CAN TYPE\n");
					fprintf(fp, "installedTemp		=> AuxTemperature installed Ch No.\n");
					fprintf(fp, "installedAux		=> AuxVoltage installed Ch No.\n");
					fprintf(fp, "installedCan		=> AuxCan installed Ch No.\n");
					fprintf(fp, "auto_v_cali			=> auto Voltage Cali (Default : 1)\n");
					fprintf(fp, "parallelMode		=> parallelMode UseFlag\n");
					fprintf(fp, "DAC_type			=> 0 : 712P, 1 : 7741\n");
					fprintf(fp, "ADC_num				=> ADC No.(1:ADC 1EA, 2:ADC 2EA)\n");
					fprintf(fp, "AD_offset			=> Charge Current AD Offset Value (Default : 0.00002)\n");
					fprintf(fp, "FeedbackFlag		=> SoftFeedback UseFlag\n");
					fprintf(fp, "MainBdType			=> 0 : CPLD TYPE, 1: FPGA TYPE\n");
					fprintf(fp, "FadBdUse			=> FAD BD UseFlag\n");
					fprintf(fp, "RangeSelectFlag\n");
					fprintf(fp, "Range0\n");
					fprintf(fp, "Range1\n");
					fprintf(fp, "Range2\n");
					fprintf(fp, "Range3\n");
					fprintf(fp, "Range4");
					fprintf(fp, "\n");
					fclose(fp);
					printf("mControl_Config file write\n");

					
					system("rm -rf /root/cycler_data/config/parameter/CellArray_A");
					system("touch /root/cycler_data/config/parameter/CellArray_A");
					memset(fileName, 0x00, sizeof(fileName));
					strcpy(fileName, "/root/cycler_data/config/parameter/CellArray_A");
					if((fp = fopen(fileName, "w")) == NULL) {
						printf("CellArray_A file write error\n");
						loop = 0;
						break;
					}
					fprintf(fp, "monitor_no\t:\thw_no\t:\tbd\t:\tch\n");
					for(i = 0; i < MAX_CH_PER_MODULE; i++) {
						fprintf(fp, "%d\t\t\t:\t", i+1);
						fprintf(fp, "%d\t\t:\t", i+1);
						fprintf(fp, "%d\t:\t", i/chPerBd);	
						fprintf(fp, "%d\n", i%chPerBd);	
					}
					fprintf(fp, "\n");
					fclose(fp);
					printf("\nCellArray_A file write success\n");
					
					system("rm -rf /root/cycler_data/config/parameter/TempArray_A");
					system("touch /root/cycler_data/config/parameter/TempArray_A");
					memset(fileName, 0x00, sizeof(fileName));
					strcpy(fileName, "/root/cycler_data/config/parameter/TempArray_A");
					if((fp = fopen(fileName, "w")) == NULL) {
						printf("TempArray_A file write error\n");
						loop = 0;
						break;
					}
					fprintf(fp, "monitor_no\t:\thw_no\t:\tbd\t:\tch\n");
					for(i = 0; i < MAX_CH_PER_MODULE; i++) {
						fprintf(fp, "%d\t\t\t:\t", i+1);
						fprintf(fp, "%d\t\t:\t", (i%100)+1);
						fprintf(fp, "%d\t:\t", i/chPerBd);	
						fprintf(fp, "%d\n", i%chPerBd);	
					}
					for(i=i; i < MAX_TEMP_CH; i++) {
						fprintf(fp, "%d\t\t\t:\t", i+1);
						fprintf(fp, "%d\t\t:\t", (i%100)+1);
						fprintf(fp, "%d\t:\t", 0);	
						fprintf(fp, "%d\n", 0);	
					}
					fprintf(fp, "\n");
					fclose(fp);
					printf("\nTempArray_A file write success\n");
					
					system("rm -rf /root/cycler_data/config/parameter/ChamberChNo");
					system("touch /root/cycler_data/config/parameter/ChamberChNo");
					memset(fileName, 0x00, sizeof(fileName));
					strcpy(fileName, "/root/cycler_data/config/parameter/ChamberChNo");
					if((fp = fopen(fileName, "w")) == NULL) {
						printf("ChamberChNo file write error\n");
						loop = 0;
						break;
					}
					fprintf(fp, "chamberChNo\t:\thw_no\t:\tbd\t:\tch\n");
					for(i = 0; i < MAX_CH_PER_MODULE; i++) {
						fprintf(fp, "%d\t\t\t:\t", 0);
						fprintf(fp, "%d\t\t:\t", i+1);
						fprintf(fp, "%d\t:\t", i/chPerBd);	
						fprintf(fp, "%d\n", i%chPerBd);	
					}
					fprintf(fp, "\n");
					fclose(fp);
					printf("\nChamberChNo file write success\n");
					
					system("rm -rf /root/cycler_data/config/parameter/FUNCTION");
					system("touch /root/cycler_data/config/parameter/FUNCTION");
					memset(fileName, 0x00, sizeof(fileName));
					strcpy(fileName, "/root/cycler_data/config/parameter/FUNCTION");
					if((fp = fopen(fileName, "w")) == NULL) {
						printf("FUNCTION file write error\n");
						loop = 0;
						break;
					}
					fprintf(fp,"OVP					:	%d\n",function[0]);
					fprintf(fp,"ChCheckCurrent		:	%d\n",function[1]);
					fprintf(fp,"OTP					:	%d\n",function[2]);
					fprintf(fp,"MAIN_BD_OT			:	%d\n",function[3]);
					fprintf(fp,"HW_FAULT_COND		:	%d\n",function[4]);
					fprintf(fp,"SW_FAULT_COND		:	%d\n",function[5]);
					fprintf(fp,"MINUS_CELL			:	%d\n",function[6]);
					fprintf(fp,"I_OFFEST_CALI		:	%d\n",function[7]);
					fprintf(fp,"SEMI_SWITCH_TYPE	:	%d\n",function[8]);
					fprintf(fp,"CHAMBER_TEMP_WAIT	:	%d\n",function[9]);
					fprintf(fp,"SDI_MES_USE			:	%d\n",function[10]);
					fprintf(fp,"OT_PAUSE			:	%d\n",function[11]);
					fprintf(fp,"CHAMBER_ERROR_PROC	:	%d\n",function[12]);
					fprintf(fp,"SENS_COUNT_TYPE		:	%d\n",function[13]);
					fprintf(fp,"V_SENS_REM_NO		:	%d\n",function[14]);
					fprintf(fp,"I_SENS_REM_NO		:	%d\n",function[15]);
					fprintf(fp,"DELTA_V_I			:	%d\n",function[16]);
					fprintf(fp,"CHAMBER_TYPE		:	%d\n",function[17]);
					fprintf(fp,"PATTERN_FTP			:	%d\n",function[18]);
					fprintf(fp,"SBC_RECOVERY		:	%d\n",function[19]);
					fprintf(fp,"PAUSE_DATA_SAVE		:	%d\n",function[20]);
					fprintf(fp,"CHANGE_VI_CHECK		:	%d\n",function[21]);
					fprintf(fp,"DCR_TYPE			:	%d\n",function[22]);
					fprintf(fp,"PATTERN_PROCESS		:	%d\n",function[23]);
					fprintf(fp,"PATTERN_CH_SAVE		:	%d\n",function[24]);
					fprintf(fp,"DISCONNECT_DAY		:	%d\n",function[25]);
					fprintf(fp, "\n");	
					fprintf(fp, "0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 \n");
				 	fprintf(fp, "0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 \n");
					fprintf(fp, "================================================================================\n");
					fprintf(fp, "Option Setting Description\n");
					fprintf(fp, "Update : 2021.11.29\n");
					fprintf(fp, "Writer : Hun\n");
					fprintf(fp, "================================================================================\n");
					fprintf(fp, "OVP					=>	Over Voltage Protect					(Default : 1)\n");
					fprintf(fp, "ChCheckCurrent		=>	Standby Channels Current Output Detect	(Default : 1)\n");
					fprintf(fp, "OTP					=>	Over Temperature Protect				(Default : 1)\n");
					fprintf(fp, "MAIN_BD_OT			=>	NO PROCESS\n");
					fprintf(fp, "HW_FAULT_COND		=>	H/W Safety Fault Detect Flag			(Default : 1)\n");
					fprintf(fp, "SW_FAULT_COND		=>	S/W Safety Fault Detect Flag			(Default : 1)\n");					
					fprintf(fp, "MINUS_CELL			=>	NO PROCESS\n");
					fprintf(fp, "I_OFFSET_CALI		=>	Current Offset Calibration Use Flag		(Default : 0)\n");
					fprintf(fp, "SEMI_SWITCH_TYPE	=> in Case of 10V < MaxVoltage, Setting 1.	(Default : 0)\n");
					fprintf(fp, "CHAMBER_TEMP_WAIT 	=> Chamber Temperature Wait UseFlag			(Default : 0)\n");
					fprintf(fp, "SDI_MES_USE			=> SAMSUNG SDI MES4 FLAG					(Default : 0)\n");
					fprintf(fp, "OT_PAUSE			=> OT FAULT -> Fault PowerModule Use Ch Pause\n");
					fprintf(fp, "CHAMBER_ERR_PROC	=> 0 : Unit Shutdown, 1 : Error Chamber Use Ch Pause\n");
					fprintf(fp, "SENS_COUNT_TYPE		=> Sens Count Increment Type\n");																																								
					fprintf(fp, "					   0 ; Filtering not use type\n");
					fprintf(fp, "					   1 : General type\n");
					fprintf(fp, "					   2 : Rt Period division filtering type\n");
					fprintf(fp,	"				   3 : Before modification General type (2019.08.23)\n");
					fprintf(fp, "V_SENS_REM_NO		=> AD Filtering V Sens MIN, MAX Remove Number\n");
					fprintf(fp, "I_SENS_REM_NO		=> AD Filtering I Sens MIN, MAX Remove Number\n");										
					fprintf(fp, "DELTA_V_I			=> DELTA Fault (0: Not Use / 1: END / 2: PAUSE)\n");
					fprintf(fp, "CHAMBER_TYPE		=> 0 : Not Use, 1 : BA Serise(16bit), 2 : SA/CA Serise(6bit)\n");
					fprintf(fp, "PATTERN_FTP			=> 0 : Default, 1 : ftp (Pattern file Receive)\n");
					fprintf(fp, "SBC_RECOVERY		=> 0 : Default, 1 : SBC Recovery Use\n");
					fprintf(fp, "PAUSE_DATA_SAVE		=> 0 : Default, 1 : Fault Recording Use (LG)\n");
					fprintf(fp, "CHANGE_VI_CHECK     => 0 : Not Use, 1 : HYUNDAE, 2: LG\n");
					fprintf(fp, "DCR_TYPE		    => 0 : Default, 1 : NV (Impedance Logic)\n");
					fprintf(fp, "PATTERN_PROCESS	    => 0 : Default, 1 : Pattern input Change\n");
					fprintf(fp, "PATTERN_CH_SAVE		=> 0 : userData Save, 1: pattern Ch Save\n");
					fprintf(fp, "\n");
					fclose(fp);
					printf("\nFUNCTION file write success\n");
					loop = 0;
					break;
				case 1:
					memset(cmd, 0x00, sizeof(cmd));
					sprintf(cmd, "cp -rf %s/AppControl_Config_digital %s/AppControl_Config", backup, parameter);
					system(cmd);
					memset(cmd, 0x00, sizeof(cmd));
					sprintf(cmd, "cp -rf %s/Addr_Interface_digital %s/Addr_Interface", backup, parameter);
					system(cmd);
					memset(cmd, 0x00, sizeof(cmd));
					sprintf(cmd, "cp -rf %s/Addr_Main_digital %s/Addr_Main", backup, parameter);
					system(cmd);
					memset(cmd, 0x00, sizeof(cmd));
					sprintf(cmd, "cp -rf %s/DIN_USE_FLAG_digital %s/DIN_USE_FLAG", backup, parameter);
					system(cmd);
					memset(cmd, 0x00, sizeof(cmd));
					sprintf(cmd, "cp -rf %s/DOUT_USE_FLAG_digital %s/DOUT_USE_FLAG", backup, parameter);
					system(cmd);
					memset(cmd, 0x00, sizeof(cmd));
					sprintf(cmd, "cp -rf %s/mControl_Config_digital %s/mControl_Config", backup, parameter);
					system(cmd);
					memset(cmd, 0x00, sizeof(cmd));
					sprintf(cmd, "cp -rf %s/CaliMeter_Config_digital %s/CaliMeter_Config", backup, parameter);
					system(cmd);

					range[0][0] = 0;	range[0][1] = 0;	range[0][2] = 0;	range[0][3] = 0;
					range[1][0] = 0;	range[1][1] = 0;	range[1][2] = 0;	range[1][3] = 0;
					range[2][0] = 0;	range[2][1] = 0;	range[2][2] = 0;	range[2][3] = 0;
					range[3][0] = 0;	range[3][1] = 0;	range[3][2] = 0;	range[3][3] = 0;
					range[4][0] = 0;	range[4][1] = 0;	range[4][2] = 0;	range[4][3] = 0;

					printf("전압 입력(V) : ");
					scanf("%d", &voltage);
					maxVoltage[0] = (long)(voltage * 1000000);
					printf("1Range 전류(A) : ");
					scanf("%lf", &current[0]);
					for(i = 0 ; i < 4 ; i++){
						maxCurrent[i] = 0;
						minCurrent[i] = 0;
						shunt[i] = 0;
						gain[i] = 0;
					}
					for(i = 0 ; i < 1 ; i++){
						maxCurrent[i] = (long)(current[i] * 1000000);
						minCurrent[i] = maxCurrent[i] * (-1);
						gain[i] = 1.0;
						adAmp = 1.0;
						voltageAmp = 1.0;
						currentAmp = 1.0;
					}
					if(current[0] >= 800){
						portPerCh = 1;
					}else{
						portPerCh = 2;
					}
					printf("한 유닛당(SBC) 전체 채널 수  : ");
					scanf("%d",&installedCh);
					hwSpec = 202;
					if(extension_bd_use == 0){
						rt_scan_type = 5;
					}else if(extension_bd_use == 1){
						rt_scan_type = 3;
					}
					printf("병렬모드 사용 유/무 ex)사용 : 1, 미사용 : 0 : ");
					scanf("%d",&parallelMode);
					printf("보조 온도 개수  : ");
					scanf("%hd",&installedTemp);
					printf("보조 전압 개수  : ");
					scanf("%hd",&installedAuxV);
					DAC_type = 0;
					ADC_num = 0;
					AD_offset = 0.0;
					SoftFeedbackFlag = 0;
					
					if(installedTemp > 0){
						memset(cmd, 0x00, sizeof(cmd));
						sprintf(cmd, "cp -rf %s/AnalogMeter_Config_Aux %s/AnalogMeter_Config", backup, parameter);
						system(cmd);
					}
					
					printf("지우전자 인버터(0) , 판다 인버터(1) 입력 : ");
					scanf("%d",&inverterType);
					printf("장비 전체의 Rack 개수 : ");
					scanf("%d",&installedInverter);
					printf("한 Rack의 인버터 수 : ");
					scanf("%d",&parallel_inv_ch);
					 
					//printf("SBC IU BD Lan포트 1개당 채널 수 : ");
					//scanf("%d",&portPerCh);
					invPerCh = installedCh / installedInverter; 
					for(i = 0 ; i < 16; i++){
						pcu_inActiveType[i] = 0;
						if(i < installedInverter){
							pcu_inUseFlag[i] = 1;
						}else{
							pcu_inUseFlag[i] = 0;
						}
					}	
					if(portPerCh == 1){
						extension_bd_use = 0;
						chPerBd = 16;
					}else{
						if(installedCh > 32){
							extension_bd_use = 1;
							chPerBd = 64;
						}else if(installedCh <= 32){
							extension_bd_use = 0;
							chPerBd = 32;
						}
					}	
					if(customer == 0){ //LG
						function[20] = 1;
						function[21] = 2;
						if(installedTemp == 0 && installedAuxV == 0){
							memset(cmd, 0x00, sizeof(cmd));
							sprintf(cmd, "cp -rf %s/Backup/SysConfig_1_4102_Switching.h %s/SysConfig.h", backup2, backup2);
							system(cmd);
						}else{
							memset(cmd, 0x00, sizeof(cmd));
							sprintf(cmd, "cp -rf %s/Backup/SysConfig_1_4103_Switching.h %s/SysConfig.h", backup2, backup2);
							system(cmd);
						}
						memset(cmd, 0x00, sizeof(cmd));
						sprintf(cmd, "cp -rf %s/HwFault_Config %s/HwFault_Config", backup, parameter);
						system(cmd);
						memset(cmd, 0x00, sizeof(cmd));
						sprintf(cmd, "cp -rf %s/SwFault_Config %s/SwFault_Config", backup, parameter);
						system(cmd);
					}else if(customer == 1){ //SDI
						function[10] = 1;
						function[24] = 1;
						memset(cmd, 0x00, sizeof(cmd));
						sprintf(cmd, "cp -rf %s/HwFault_Config_digital %s/HwFault_Config", backup, parameter);
						system(cmd);
						memset(cmd, 0x00, sizeof(cmd));
						sprintf(cmd, "cp -rf %s/SwFault_Config_other %s/SwFault_Config", backup, parameter);
						system(cmd);
						memset(cmd, 0x00, sizeof(cmd));
						sprintf(cmd, "cp -rf %s/Backup/SysConfig_2_4105_Switching.h %s/SysConfig.h", backup2, backup2);
						system(cmd);
					}else if(customer == 2){ //SK
						function[9] = 1;
						function[18] = 1;
						function[19] = 1;
						memset(cmd, 0x00, sizeof(cmd));
						sprintf(cmd, "cp -rf %s/HwFault_Config_digital %s/HwFault_Config", backup, parameter);
						system(cmd);
						memset(cmd, 0x00, sizeof(cmd));
						sprintf(cmd, "cp -rf %s/SwFault_Config_other %s/SwFault_Config", backup, parameter);
						system(cmd);
						if(installedTemp == 0 && installedAuxV == 0){
							memset(cmd, 0x00, sizeof(cmd));
							sprintf(cmd, "cp -rf %s/Backup/SysConfig_3_4102_Switching.h %s/SysConfig.h", backup2, backup2);
							system(cmd);
						}else{
							memset(cmd, 0x00, sizeof(cmd));
							sprintf(cmd, "cp -rf %s/Backup/SysConfig_3_4103_Switching.h %s/SysConfig.h", backup2, backup2);
							system(cmd);
						}
					}else if(customer == 3){ //NorthVolt
						function[12] = 1;
						function[21] = 2;
						function[22] = 1;
						function[23] = 1;
						memset(cmd, 0x00, sizeof(cmd));
						sprintf(cmd, "cp -rf %s/HwFault_Config_digital %s/HwFault_Config", backup, parameter);
						system(cmd);
						memset(cmd, 0x00, sizeof(cmd));
						sprintf(cmd, "cp -rf %s/SwFault_Config_other %s/SwFault_Config", backup, parameter);
						system(cmd);
						if(installedTemp == 0 && installedAuxV == 0){
							memset(cmd, 0x00, sizeof(cmd));
							sprintf(cmd, "cp -rf %s/Backup/SysConfig_1_4102_Switching.h %s/SysConfig.h", backup2, backup2);
							system(cmd);
						}else{
							memset(cmd, 0x00, sizeof(cmd));
							sprintf(cmd, "cp -rf %s/Backup/SysConfig_1_4103_Switching.h %s/SysConfig.h", backup2, backup2);
							system(cmd);
						}
					}else if(customer == 4){ //현대
						function[21] = 1;
						memset(cmd, 0x00, sizeof(cmd));
						sprintf(cmd, "cp -rf %s/HwFault_Config_digital %s/HwFault_Config", backup, parameter);
						system(cmd);
						memset(cmd, 0x00, sizeof(cmd));
						sprintf(cmd, "cp -rf %s/SwFault_Config_other %s/SwFault_Config", backup, parameter);
						system(cmd);
						if(installedTemp == 0 && installedAuxV == 0){
							memset(cmd, 0x00, sizeof(cmd));
							sprintf(cmd, "cp -rf %s/Backup/SysConfig_1_4102_Switching.h %s/SysConfig.h", backup2, backup2);
							system(cmd);
						}else{
							memset(cmd, 0x00, sizeof(cmd));
							sprintf(cmd, "cp -rf %s/Backup/SysConfig_1_4103_Switching.h %s/SysConfig.h", backup2, backup2);
							system(cmd);
						}
					}else if(customer == 5){ //기타업체
						function[21] = 2;
						memset(cmd, 0x00, sizeof(cmd));
						sprintf(cmd, "cp -rf %s/HwFault_Config_digital %s/HwFault_Config", backup, parameter);
						system(cmd);
						memset(cmd, 0x00, sizeof(cmd));
						sprintf(cmd, "cp -rf %s/SwFault_Config_other %s/SwFault_Config", backup, parameter);
						system(cmd);
						if(installedTemp == 0 && installedAuxV == 0){
							memset(cmd, 0x00, sizeof(cmd));
							sprintf(cmd, "cp -rf %s/Backup/SysConfig_1_4102_Switching.h %s/SysConfig.h", backup2, backup2);
							system(cmd);
						}else{
							memset(cmd, 0x00, sizeof(cmd));
							sprintf(cmd, "cp -rf %s/Backup/SysConfig_1_4103_Switching.h %s/SysConfig.h", backup2, backup2);
							system(cmd);
						}
					}
					printf("스위칭 mControl_Config\n");
					printf("installedBd		:	%d\n",installedBd);
					printf("installedCh		:	%d\n",installedCh);
					printf("chPerBd			:	%d\n",chPerBd);
					printf("rangeV			:	%d\n",rangeV);
					printf("rangeI			:	%d\n",rangeI);
					printf("maxVoltage		:	%ld %ld %ld %ld\n",maxVoltage[0],maxVoltage[1],maxVoltage[2],maxVoltage[3]);
					printf("minVoltage		:	%ld %ld %ld %ld\n",minVoltage[0],minVoltage[1],minVoltage[2],minVoltage[3]);
					printf("maxCurrent		:	%ld %ld %ld %ld\n",maxCurrent[0],maxCurrent[1],maxCurrent[2],maxCurrent[3]);
					printf("minCurrent		:	%ld %ld %ld %ld\n",minCurrent[0],minCurrent[1],minCurrent[2],minCurrent[3]);
					printf("ratioVoltage		:	%d\n",ratioVoltage);
					printf("ratioCurrent		:	%d\n",ratioCurrent);
					printf("watchdogFlag		:	%d\n",watchdogFlag);
					printf("ADC_type		:	%d\n",ADC_type);
					printf("hwSpec			:	%d\n",hwSpec);
					printf("capacityType		:	%d\n",capacityType);
					printf("totalJig		:	%d\n",totalJig);
					printf("bdInJig1_8		:	%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n",bdInJig[0],bdInJig[1],bdInJig[2],bdInJig[3],
						bdInJig[4],bdInJig[5],bdInJig[6],bdInJig[7]);
					printf("bdInJig1_16		:	%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n",bdInJig[8],bdInJig[9],bdInJig[10],bdInJig[11],
						bdInJig[12],bdInJig[13],bdInJig[14],bdInJig[15]);
					printf("shuntOhm		:	%f\t%f\t%f\t%f\n",shunt[0],shunt[1],shunt[2], shunt[3]);
					printf("gain			:	%f\t%f\t%f\t%f\n",gain[0],gain[1],gain[2], gain[3]);
					printf("adAmp			:	%f\t%f\t%f\n", adAmp,voltageAmp,currentAmp);
					printf("rt_scan_type		:	%d\n",rt_scan_type);
					printf("installedTemp		:	%d\n",installedTemp);	
					printf("installedAuxV		:	%d\n",installedAuxV);	
					printf("installedCan		:	%d\n",installedCAN);	
					printf("auto_v_cali		:	%d\n",auto_v_cali);	
					printf("parallelMode		:	%d\n",parallelMode);	
					printf("DAC_type		:	%d\n",DAC_type);	
					printf("ADC_num			:	%d\n",ADC_num);
					printf("AD_offset		:	%f\n",AD_offset);
					printf("FeedbackFlag		:	%d\n",SoftFeedbackFlag);
					printf("MainBdType		:	%d\n",MainBdType);
					printf("FadBdUse		:	%d\n",FadBdUse);
					printf("RangeSelectFlag		:	0x10\t0x20\t0x40\t0x80\n");
					printf("Range0			:	%d\t%d\t%d\t%d\n",range[0][0],range[0][1],range[0][2],range[0][3]);
					printf("Range1			:	%d\t%d\t%d\t%d\n",range[1][0],range[1][1],range[1][2],range[1][3]);
					printf("Range2			:	%d\t%d\t%d\t%d\n",range[2][0],range[2][1],range[2][2],range[2][3]);
					printf("Range3			:	%d\t%d\t%d\t%d\n",range[3][0],range[3][1],range[3][2],range[3][3]);
					printf("Range4			:	%d\t%d\t%d\t%d\n",range[4][0],range[4][1],range[4][2],range[4][3]);
					printf("\n");
					printf("스위칭 PCU_Config\n");
					printf("PortPerCh		:	%d\n",portPerCh);
					printf("installedInverter	:	%d\n",installedInverter);
					printf("hallCT			:	%.7f\n",hallCT);
					printf("caliVohm		:	%f\n",caliV_ohm);
					printf("caliUse			:	%d\n",pcuCaliUse);
					printf("powerModuleNo		:	%d\n",powerNo);
					printf("CaliV_amp		:	%f\n",caliV_amp);
					printf("setCaliNum		:	%d\n",setCaliNum);
					printf("cali_delay_time(mS)	:	%d\n",cali_delay_time);
					printf("invPerCh		:	%d\n",invPerCh);
					printf("parallel_Inv		:	%d\n",parallel_inv_ch);
					printf("inverterType		:	%d\n",inverterType);
					printf("\n");
					printf("스위칭 PCU_INV_USE_FLAG\n");
					printf("No\tFlag\tActive(H/L)\tDescription\n");
					for(i = 0 ; i < 16 ; i++){
						printf("%d\t%d\t%d\t\tP_INVERTER_%d\n",i+1,pcu_inUseFlag[i],pcu_inActiveType[i],i+1);
					}
					printf("\n");
					printf("OVP			:	%d\n",function[0]);
					printf("ChCheckCurrent		:	%d\n",function[1]);
					printf("OTP			:	%d\n",function[2]);
					printf("MAIN_BD_OT		:	%d\n",function[3]);
					printf("HW_FAULT_COND		:	%d\n",function[4]);
					printf("SW_FAULT_COND		:	%d\n",function[5]);
					printf("MINUS_CELL		:	%d\n",function[6]);
					printf("I_OFFEST_CALI		:	%d\n",function[7]);
					printf("SEMI_SWITCH_TYPE	:	%d\n",function[8]);
					printf("CHAMBER_TEMP_WAIT	:	%d\n",function[9]);
					printf("SDI_MES_USE		:	%d\n",function[10]);
					printf("OT_PAUSE		:	%d\n",function[11]);
					printf("CHAMBER_ERROR_PROC	:	%d\n",function[12]);
					printf("SENS_COUNT_TYPE		:	%d\n",function[13]);
					printf("V_SENS_REM_NO		:	%d\n",function[14]);
					printf("I_SENS_REM_NO		:	%d\n",function[15]);
					printf("DELTA_V_I		:	%d\n",function[16]);
					printf("CHAMBER_TYPE		:	%d\n",function[17]);
					printf("PATTERN_FTP		:	%d\n",function[18]);
					printf("SBC_RECOVERY		:	%d\n",function[19]);
					printf("PAUSE_DATA_SAVE		:	%d\n",function[20]);
					printf("CHANGE_VI_CHECK		:	%d\n",function[21]);
					printf("DCR_TYPE		:	%d\n",function[22]);
					printf("PATTERN_PROCESS		:	%d\n",function[23]);
					printf("PATTERN_CH_SAVE		:	%d\n",function[24]);
					printf("DISCONNECT_DAY		:	%d\n",function[25]);
					//loop = 0;
					printf("\n입력완료. 파일에 저장 하시겠습니까? [0:아니오 1:예]");
					scanf("%d", &value);
					if(value == 0) return -1;
				
					system("rm -rf /root/cycler_data/config/parameter/mControl_Config");
					system("touch /root/cycler_data/config/parameter/mControl_Config");
					memset(fileName, 0x00, sizeof(fileName));
					strcpy(fileName, "/root/cycler_data/config/parameter/mControl_Config");
					if((fp = fopen(fileName, "w")) == NULL) {
						printf("mControl_Config write error\n");
						loop = 0;
						break;
					}
					// Save File
					fprintf(fp,"installedBd			:	%d\n",installedBd);
					fprintf(fp,"installedCh			:	%d\n",installedCh);
					fprintf(fp,"chPerBd				:	%d\n",chPerBd);
					fprintf(fp,"rangeV				:	%d\n",rangeV);
					fprintf(fp,"rangeI				:	%d\n",rangeI);
					fprintf(fp,"maxVoltage			:	%ld %ld %ld %ld\n",maxVoltage[0],maxVoltage[1],maxVoltage[2],maxVoltage[3]);
					fprintf(fp,"minVoltage			:	%ld %ld %ld %ld\n",minVoltage[0],minVoltage[1],minVoltage[2],minVoltage[3]);
					fprintf(fp,"maxCurrent			:	%ld %ld %ld %ld\n",maxCurrent[0],maxCurrent[1],maxCurrent[2],maxCurrent[3]);
					fprintf(fp,"minCurrent			:	%ld %ld %ld %ld\n",minCurrent[0],minCurrent[1],minCurrent[2],minCurrent[3]);
					fprintf(fp,"ratioVoltage		:	%d\n",ratioVoltage);
					fprintf(fp,"ratioCurrent		:	%d\n",ratioCurrent);
					fprintf(fp,"watchdogFlag		:	%d\n",watchdogFlag);
					fprintf(fp,"ADC_type			:	%d\n",ADC_type);
					fprintf(fp,"hwSpec				:	%d\n",hwSpec);
					fprintf(fp,"capacityType		:	%d\n",capacityType);
					fprintf(fp,"totalJig			:	%d\n",totalJig);
					fprintf(fp,"bdInJig1_8			:	%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n",
						bdInJig[0],bdInJig[1],bdInJig[2],bdInJig[3],bdInJig[4],bdInJig[5],bdInJig[6],bdInJig[7]);
					fprintf(fp,"bdInJig1_16			:	%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n",
						bdInJig[8],bdInJig[9],bdInJig[10],bdInJig[11],bdInJig[12],bdInJig[13],bdInJig[14],bdInJig[15]);
					fprintf(fp,"shuntOhm			:	%f\t%f\t%f\t%f\n",shunt[0],shunt[1],shunt[2], shunt[3]);
					fprintf(fp,"gain				:	%f\t%f\t%f\t%f\n",gain[0],gain[1],gain[2], gain[3]);
					fprintf(fp,"adAmp				:	%f\t%f\t%f\n", adAmp,voltageAmp,currentAmp);
					fprintf(fp,"rt_scan_type		:	%d\n",rt_scan_type);
					fprintf(fp,"installedTemp		:	%d\n",installedTemp);	
					fprintf(fp,"installedAuxV		:	%d\n",installedAuxV);	
					fprintf(fp,"installedCan		:	%d\n",installedCAN);	
					fprintf(fp,"auto_v_cali			:	%d\n",auto_v_cali);	
					fprintf(fp,"parallelMode		:	%d\n",parallelMode);	
					fprintf(fp,"DAC_type			:	%d\n",DAC_type);	
					fprintf(fp,"ADC_num				:	%d\n",ADC_num);
					fprintf(fp,"AD_offset			:	%f\n",AD_offset);
					fprintf(fp,"FeedbackFlag		:	%d\n",SoftFeedbackFlag);
					fprintf(fp,"MainBdType			:	%d\n",MainBdType);
					fprintf(fp,"FadBdUse			:	%d\n",FadBdUse);
					fprintf(fp,"RangeSelectFlag		:	0x10\t0x20\t0x40\t0x80\n");
					fprintf(fp,"Range0				:	%d\t\t%d\t\t%d\t\t%d\n",range[0][0],range[0][1],range[0][2],range[0][3]);
					fprintf(fp,"Range1				:	%d\t\t%d\t\t%d\t\t%d\n",range[1][0],range[1][1],range[1][2],range[1][3]);
					fprintf(fp,"Range2				:	%d\t\t%d\t\t%d\t\t%d\n",range[2][0],range[2][1],range[2][2],range[2][3]);
					fprintf(fp,"Range3				:	%d\t\t%d\t\t%d\t\t%d\n",range[3][0],range[3][1],range[3][2],range[3][3]);
					fprintf(fp,"Range4				:	%d\t\t%d\t\t%d\t\t%d\n",range[4][0],range[4][1],range[4][2],range[4][3]);
					fprintf(fp, "\n");	
					fprintf(fp, "0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 \n");
			 	    fprintf(fp, "0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 \n");
					fprintf(fp, "================================================================================\n");
					fprintf(fp, "Option Setting Description\n");
					fprintf(fp, "Update : 2021.11.29\n");
					fprintf(fp, "Writer : Hun\n");
					fprintf(fp, "================================================================================\n");
					fprintf(fp, "installedBd			=> insalled MainBoard No.\n");
					fprintf(fp, "installedCh			=> installed Total Ch\n");
					fprintf(fp, "chPerBd				=> Ch Per MainBD\n");
					fprintf(fp, "rangeV				=> Voltage Range No.\n");
					fprintf(fp, "rangeI				=> Current Range No.\n");
					fprintf(fp, "maxVoltage\n");
					fprintf(fp, "minVoltage\n");
					fprintf(fp, "maxCurrent\n");
					fprintf(fp, "minCurrent\n");
					fprintf(fp, "ratioVoltage		=> 0 : Micro, 1 : Nano (Default : 0)\n");
					fprintf(fp, "ratioCurrent		=> 0 : Micro, 1 : Nano (Default : 0)\n");
					fprintf(fp, "watchdogFlag	 	=> WatchDog UseFlag (Default : 1)	\n");
					fprintf(fp, "ADC_type			=> 0 : COMM_TYPE, 1 : 7805 (Default : 1)\n");
					fprintf(fp, "hwSpec				=> H/W SPEC No. ex) 63 : L_5V_150A_R3_AD2\n");
					fprintf(fp, "capacityType		=> 0 : AmpareHour, 1 : Farad\n");
					fprintf(fp, "totalJig\n");
					fprintf(fp, "bdInJig1_8\n");
					fprintf(fp, "bdInJig9_16\n");
					fprintf(fp, "shuntOhm			=> Range Shunt Value\n");
					fprintf(fp, "gain				=> Range Gain Value\n");
					fprintf(fp, "adAmp				=> AD_AMP	VOLTAGE_AMP	 CURRENT_AMP\n");
					fprintf(fp, "rt_scan_type		=> 0 : 100ms	=> ADC 1EA, chPerBD <= 32CH\n");
					fprintf(fp, "									   ADC 2EA, chPerBD <= 64CH\n");
					fprintf(fp, "					   1 : 50ms		=> ADC 1EA, chPerBD <= 16CH\n");
					fprintf(fp, "									   ADC 2EA, chPerBD <= 32CH\n");
					fprintf(fp, "					   2 : 25ms		=> ADC 2EA, chPerBD <= 16CH\n");
					fprintf(fp, "					   3 : 20ms		=> ADC 2EA, chPerBD <=  8CH\n");
					fprintf(fp, "					   4 : 100ms_2	=> ADC 1EA, chPerBD <= 64CH\n");
					fprintf(fp, "					   5 : 10ms		=> ADC 2EA, chPerBD <=  8CH\n");
					fprintf(fp, "					   6 : 100ms_CAN TYPE\n");
					fprintf(fp, "installedTemp		=> AuxTemperature installed Ch No.\n");
					fprintf(fp, "installedAux		=> AuxVoltage installed Ch No.\n");
					fprintf(fp, "installedCan		=> AuxCan installed Ch No.\n");
					fprintf(fp, "auto_v_cali			=> auto Voltage Cali (Default : 1)\n");
					fprintf(fp, "parallelMode		=> parallelMode UseFlag\n");
					fprintf(fp, "DAC_type			=> 0 : 712P, 1 : 7741\n");
					fprintf(fp, "ADC_num				=> ADC No.(1:ADC 1EA, 2:ADC 2EA)\n");
					fprintf(fp, "AD_offset			=> Charge Current AD Offset Value (Default : 0.00002)\n");
					fprintf(fp, "FeedbackFlag		=> SoftFeedback UseFlag\n");
					fprintf(fp, "MainBdType			=> 0 : CPLD TYPE, 1: FPGA TYPE\n");
					fprintf(fp, "FadBdUse			=> FAD BD UseFlag\n");
					fprintf(fp, "RangeSelectFlag\n");
					fprintf(fp, "Range0\n");
					fprintf(fp, "Range1\n");
					fprintf(fp, "Range2\n");
					fprintf(fp, "Range3\n");
					fprintf(fp, "Range4");
					fprintf(fp, "\n");
					fclose(fp);
					printf("mControl_Config write\n");

					system("rm -rf /root/cycler_data/config/parameter/PCU_Config");
					system("touch /root/cycler_data/config/parameter/PCU_Config");
					memset(fileName, 0x00, sizeof(fileName));
					strcpy(fileName, "/root/cycler_data/config/parameter/PCU_Config");
					if((fp = fopen(fileName, "w")) == NULL) {
						printf("PCU_Config file write error\n");
						loop = 0;
						break;
					}
					fprintf(fp,"PortPerCh			:	%d\n",portPerCh);
					fprintf(fp,"installedInverter	:	%d\n",installedInverter);
					fprintf(fp,"hallCT				:	%.7f\n",hallCT);
					fprintf(fp,"caliVohm			:	%f\n",caliV_ohm);
					fprintf(fp,"caliUse				:	%d\n",pcuCaliUse);
					fprintf(fp,"powerModuleNo		:	%d\n",powerNo);
					fprintf(fp,"CaliV_amp			:	%f\n",caliV_amp);
					fprintf(fp,"setCaliNum			:	%d\n",setCaliNum);
					fprintf(fp,"cali_delay_time(mS)	:	%d\n",cali_delay_time);
					fprintf(fp,"invPerCh			:	%d\n",invPerCh);
					fprintf(fp,"parallel_Inv		:	%d\n",parallel_inv_ch);
					fprintf(fp,"inverterType		:	%d\n",inverterType);
					fprintf(fp, "\n");
					fclose(fp);
					printf("PCU_Config write\n");
					
					system("rm -rf /root/cycler_data/config/parameter/PCU_INV_USE_FLAG");
					system("touch /root/cycler_data/config/parameter/PCU_INV_USE_FLAG");
					memset(fileName, 0x00, sizeof(fileName));
					strcpy(fileName, "/root/cycler_data/config/parameter/PCU_INV_USE_FLAG");
					if((fp = fopen(fileName, "w")) == NULL) {
						printf("PCU_INV_USE_FLAG file write error\n");
						loop = 0;
						break;
					}
					fprintf(fp,"No\t\tFlag\tActive(H/L)\t\tDescription\n");
					for(i = 0 ; i < 16 ; i++){
						fprintf(fp,"%d\t\t%d\t\t\t%d\t\t\tP_INVERTER_%d\n",i+1,pcu_inUseFlag[i],pcu_inActiveType[i],i+1);
					}
					fprintf(fp, "\n");
					fclose(fp);
					printf("PCU_INV_USE_FLAG write\n");

					system("rm -rf /root/cycler_data/config/parameter/CellArray_A");
					system("touch /root/cycler_data/config/parameter/CellArray_A");
					memset(fileName, 0x00, sizeof(fileName));
					strcpy(fileName, "/root/cycler_data/config/parameter/CellArray_A");
					if((fp = fopen(fileName, "w")) == NULL) {
						printf("CellArray_A file write error\n");
						loop = 0;
						break;
					}
					fprintf(fp, "monitor_no\t:\thw_no\t:\tbd\t:\tch\n");
					for(i = 0; i < MAX_CH_PER_MODULE; i++) {
						fprintf(fp, "%d\t\t\t:\t", i+1);
						fprintf(fp, "%d\t\t:\t", i+1);
						fprintf(fp, "%d\t:\t", i/chPerBd);	
						fprintf(fp, "%d\n", i%chPerBd);	
					}
					fprintf(fp, "\n");
					fclose(fp);
					printf("\nCellArray_A file write success\n");
					
					system("rm -rf /root/cycler_data/config/parameter/TempArray_A");
					system("touch /root/cycler_data/config/parameter/TempArray_A");
					memset(fileName, 0x00, sizeof(fileName));
					strcpy(fileName, "/root/cycler_data/config/parameter/TempArray_A");
					if((fp = fopen(fileName, "w")) == NULL) {
						printf("TempArray_A file write error\n");
						loop = 0;
						break;
					}
					fprintf(fp, "monitor_no\t:\thw_no\t:\tbd\t:\tch\n");
					for(i = 0; i < MAX_CH_PER_MODULE; i++) {
						fprintf(fp, "%d\t\t\t:\t", i+1);
						fprintf(fp, "%d\t\t:\t", (i%100)+1);
						fprintf(fp, "%d\t:\t", i/chPerBd);	
						fprintf(fp, "%d\n", i%chPerBd);	
					}
					for(i=i; i < MAX_TEMP_CH; i++) {
						fprintf(fp, "%d\t\t\t:\t", i+1);
						fprintf(fp, "%d\t\t:\t", (i%100)+1);
						fprintf(fp, "%d\t:\t", 0);	
						fprintf(fp, "%d\n", 0);	
					}
					fprintf(fp, "\n");
					fclose(fp);
					printf("\nTempArray_A file write success\n");
					
					system("rm -rf /root/cycler_data/config/parameter/ChamberChNo");
					system("touch /root/cycler_data/config/parameter/ChamberChNo");
					memset(fileName, 0x00, sizeof(fileName));
					strcpy(fileName, "/root/cycler_data/config/parameter/ChamberChNo");
					if((fp = fopen(fileName, "w")) == NULL) {
						printf("ChamberChNo file write error\n");
						loop = 0;
						break;
					}
					fprintf(fp, "chamberChNo\t:\thw_no\t:\tbd\t:\tch\n");
					for(i = 0; i < MAX_CH_PER_MODULE; i++) {
						fprintf(fp, "%d\t\t\t:\t", 0);
						fprintf(fp, "%d\t\t:\t", i+1);
						fprintf(fp, "%d\t:\t", i/chPerBd);	
						fprintf(fp, "%d\n", i%chPerBd);	
					}
					fprintf(fp, "\n");
					fclose(fp);
					printf("\nChamberChNo file write success\n");
					
					system("rm -rf /root/cycler_data/config/parameter/FUNCTION");
					system("touch /root/cycler_data/config/parameter/FUNCTION");
					memset(fileName, 0x00, sizeof(fileName));
					strcpy(fileName, "/root/cycler_data/config/parameter/FUNCTION");
					if((fp = fopen(fileName, "w")) == NULL) {
						printf("FUNCTION file write error\n");
						loop = 0;
						break;
					}
					fprintf(fp,"OVP					:	%d\n",function[0]);
					fprintf(fp,"ChCheckCurrent		:	%d\n",function[1]);
					fprintf(fp,"OTP					:	%d\n",function[2]);
					fprintf(fp,"MAIN_BD_OT			:	%d\n",function[3]);
					fprintf(fp,"HW_FAULT_COND		:	%d\n",function[4]);
					fprintf(fp,"SW_FAULT_COND		:	%d\n",function[5]);
					fprintf(fp,"MINUS_CELL			:	%d\n",function[6]);
					fprintf(fp,"I_OFFEST_CALI		:	%d\n",function[7]);
					fprintf(fp,"SEMI_SWITCH_TYPE	:	%d\n",function[8]);
					fprintf(fp,"CHAMBER_TEMP_WAIT	:	%d\n",function[9]);
					fprintf(fp,"SDI_MES_USE			:	%d\n",function[10]);
					fprintf(fp,"OT_PAUSE			:	%d\n",function[11]);
					fprintf(fp,"CHAMBER_ERROR_PROC	:	%d\n",function[12]);
					fprintf(fp,"SENS_COUNT_TYPE		:	%d\n",function[13]);
					fprintf(fp,"V_SENS_REM_NO		:	%d\n",function[14]);
					fprintf(fp,"I_SENS_REM_NO		:	%d\n",function[15]);
					fprintf(fp,"DELTA_V_I			:	%d\n",function[16]);
					fprintf(fp,"CHAMBER_TYPE		:	%d\n",function[17]);
					fprintf(fp,"PATTERN_FTP			:	%d\n",function[18]);
					fprintf(fp,"SBC_RECOVERY		:	%d\n",function[19]);
					fprintf(fp,"PAUSE_DATA_SAVE		:	%d\n",function[20]);
					fprintf(fp,"CHANGE_VI_CHECK		:	%d\n",function[21]);
					fprintf(fp,"DCR_TYPE			:	%d\n",function[22]);
					fprintf(fp,"PATTERN_PROCESS		:	%d\n",function[23]);
					fprintf(fp,"PATTERN_CH_SAVE		:	%d\n",function[24]);
					fprintf(fp,"DISCONNECT_DAY		:	%d\n",function[25]);
					fprintf(fp, "\n");	
					fprintf(fp, "0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 \n");
				 	fprintf(fp, "0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 \n");
					fprintf(fp, "================================================================================\n");
					fprintf(fp, "Option Setting Description\n");
					fprintf(fp, "Update : 2021.11.29\n");
					fprintf(fp, "Writer : Hun\n");
					fprintf(fp, "================================================================================\n");
					fprintf(fp, "OVP					=>	Over Voltage Protect					(Default : 1)\n");
					fprintf(fp, "ChCheckCurrent		=>	Standby Channels Current Output Detect	(Default : 1)\n");
					fprintf(fp, "OTP					=>	Over Temperature Protect				(Default : 1)\n");
					fprintf(fp, "MAIN_BD_OT			=>	NO PROCESS\n");
					fprintf(fp, "HW_FAULT_COND		=>	H/W Safety Fault Detect Flag			(Default : 1)\n");
					fprintf(fp, "SW_FAULT_COND		=>	S/W Safety Fault Detect Flag			(Default : 1)\n");					
					fprintf(fp, "MINUS_CELL			=>	NO PROCESS\n");
					fprintf(fp, "I_OFFSET_CALI		=>	Current Offset Calibration Use Flag		(Default : 0)\n");
					fprintf(fp, "SEMI_SWITCH_TYPE	=> in Case of 10V < MaxVoltage, Setting 1.	(Default : 0)\n");
					fprintf(fp, "CHAMBER_TEMP_WAIT 	=> Chamber Temperature Wait UseFlag			(Default : 0)\n");
					fprintf(fp, "SDI_MES_USE			=> SAMSUNG SDI MES4 FLAG					(Default : 0)\n");
					fprintf(fp, "OT_PAUSE			=> OT FAULT -> Fault PowerModule Use Ch Pause\n");
					fprintf(fp, "CHAMBER_ERR_PROC	=> 0 : Unit Shutdown, 1 : Error Chamber Use Ch Pause\n");
					fprintf(fp, "SENS_COUNT_TYPE		=> Sens Count Increment Type\n");																																								
					fprintf(fp, "					   0 ; Filtering not use type\n");
					fprintf(fp, "					   1 : General type\n");
					fprintf(fp, "					   2 : Rt Period division filtering type\n");
					fprintf(fp,	"				   3 : Before modification General type (2019.08.23)\n");
					fprintf(fp, "V_SENS_REM_NO		=> AD Filtering V Sens MIN, MAX Remove Number\n");
					fprintf(fp, "I_SENS_REM_NO		=> AD Filtering I Sens MIN, MAX Remove Number\n");										
					fprintf(fp, "DELTA_V_I			=> DELTA Fault (0: Not Use / 1: END / 2: PAUSE)\n");
					fprintf(fp, "CHAMBER_TYPE		=> 0 : Not Use, 1 : BA Serise(16bit), 2 : SA/CA Serise(6bit)\n");
					fprintf(fp, "PATTERN_FTP			=> 0 : Default, 1 : ftp (Pattern file Receive)\n");
					fprintf(fp, "SBC_RECOVERY		=> 0 : Default, 1 : SBC Recovery Use\n");
					fprintf(fp, "PAUSE_DATA_SAVE		=> 0 : Default, 1 : Fault Recording Use (LG)\n");
					fprintf(fp, "CHANGE_VI_CHECK     => 0 : Not Use, 1 : HYUNDAE, 2: LG\n");
					fprintf(fp, "DCR_TYPE		    => 0 : Default, 1 : NV (Impedance Logic)\n");
					fprintf(fp, "PATTERN_PROCESS	    => 0 : Default, 1 : Pattern input Change\n");
					fprintf(fp, "PATTERN_CH_SAVE		=> 0 : userData Save, 1: pattern Ch Save\n");
					fprintf(fp, "\n");
					fclose(fp);
					printf("\nFUNCTION file write success\n");
					sleep(1);
					loop = 0;
					break;
				case 2:
					loop = 0;
					break;
				default:
					break;
			}
	}
	sleep(1);
	return 0;
}

int selectType(void)
{
	int val, loop;
	char parameter[64], backup[64], cmd[256];

	loop = 1;
	val = 100;

	memset(parameter, 0x00, sizeof(parameter));
	strcpy(parameter, "/root/cycler_data/config/parameter");
	memset(backup, 0x00, sizeof(backup));
	strcpy(backup, "/root/cycler_data/config/parameter/0_Backup");
	
	while(loop) {
		printf("Cycler Type Select (0:Pass, 1:Linear, 2:Digital, 3:Can) : ");
		scanf("%d", &val);
		while(getchar() != '\n');
		if(val >= 0 && val <=3) {
			switch(val) {
				case 0:
					loop = 0;
					break;
				case 1:
					memset(cmd, 0x00, sizeof(cmd));
					sprintf(cmd, "cp -rf %s/AppControl_Config_linear %s/AppControl_Config", backup, parameter);
					system(cmd);
					memset(cmd, 0x00, sizeof(cmd));
					sprintf(cmd, "cp -rf %s/Addr_Interface_linear %s/Addr_Interface", backup, parameter);
					system(cmd);
					memset(cmd, 0x00, sizeof(cmd));
					sprintf(cmd, "cp -rf %s/Addr_Main_linear %s/Addr_Main", backup, parameter);
					system(cmd);
					memset(cmd, 0x00, sizeof(cmd));
					sprintf(cmd, "cp -rf %s/DIN_USE_FLAG_linear %s/DIN_USE_FLAG", backup, parameter);
					system(cmd);
					memset(cmd, 0x00, sizeof(cmd));
					sprintf(cmd, "cp -rf %s/DOUT_USE_FLAG_linear %s/DOUT_USE_FLAG", backup, parameter);
					system(cmd);
					memset(cmd, 0x00, sizeof(cmd));
					sprintf(cmd, "cp -rf %s/mControl_Config_linear %s/mControl_Config", backup, parameter);
					system(cmd);
					printf("Changed Linear Type files\n\nAppControl_Config\nAddr_Interface\nAddr_Main\nDIN_USE_FLAG\nDOUT_USE_FLAG\nmControl_Config\n");
						loop = 0;
					break;
				case 2:
					memset(cmd, 0x00, sizeof(cmd));
					sprintf(cmd, "cp -rf %s/AppControl_Config_digital %s/AppControl_Config", backup, parameter);
					system(cmd);
					memset(cmd, 0x00, sizeof(cmd));
					sprintf(cmd, "cp -rf %s/Addr_Interface_digital %s/Addr_Interface", backup, parameter);
					system(cmd);
					memset(cmd, 0x00, sizeof(cmd));
					sprintf(cmd, "cp -rf %s/Addr_Main_digital %s/Addr_Main", backup, parameter);
					system(cmd);
					memset(cmd, 0x00, sizeof(cmd));
					sprintf(cmd, "cp -rf %s/DIN_USE_FLAG_digital %s/DIN_USE_FLAG", backup, parameter);
					system(cmd);
					memset(cmd, 0x00, sizeof(cmd));
					sprintf(cmd, "cp -rf %s/DOUT_USE_FLAG_digital %s/DOUT_USE_FLAG", backup, parameter);
					system(cmd);
					memset(cmd, 0x00, sizeof(cmd));
					sprintf(cmd, "cp -rf %s/mControl_Config_digital %s/mControl_Config", backup, parameter);
					system(cmd);
					printf("Changed Digital Type files\n\nAppControl_Config\nAddr_Interface\nAddr_Main\nDIN_USE_FLAG\nDOUT_USE_FLAG\nmControl_Config\n");
						loop = 0;
					break;
				case 3:
					memset(cmd, 0x00, sizeof(cmd));
					sprintf(cmd, "cp -rf %s/AppControl_Config_Can %s/AppControl_Config", backup, parameter);
					system(cmd);
					memset(cmd, 0x00, sizeof(cmd));
					sprintf(cmd, "cp -rf %s/Addr_Interface_Can %s/Addr_Interface", backup, parameter);
					system(cmd);
					memset(cmd, 0x00, sizeof(cmd));
					sprintf(cmd, "cp -rf %s/Addr_Main_Can %s/Addr_Main", backup, parameter);
					system(cmd);
					memset(cmd, 0x00, sizeof(cmd));
					sprintf(cmd, "cp -rf %s/DIN_USE_FLAG_Can %s/DIN_USE_FLAG", backup, parameter);
					system(cmd);
					memset(cmd, 0x00, sizeof(cmd));
					sprintf(cmd, "cp -rf %s/DOUT_USE_FLAG_Can %s/DOUT_USE_FLAG", backup, parameter);
					system(cmd);
					memset(cmd, 0x00, sizeof(cmd));
					sprintf(cmd, "cp -rf %s/mControl_Config_Can %s/mControl_Config", backup, parameter);
					system(cmd);
					printf("Changed Can Type files\n\nAppControl_Config\nAddr_Interface\nAddr_Main\nDIN_USE_FLAG\nDOUT_USE_FLAG\nmControl_Config\n");
						loop = 0;
					break;
				default:
					printf("!!! value is no valid\n");
					loop = 1;
					val = 100;
					break;
			}
		} else {
			printf("!!! Please enter only number in that range\n");
		}
		printf("\n");
	}
	sleep(1);
	return 0;
}
