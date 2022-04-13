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
#include "/project/Cycler/current/cycler/INC/datastore.h"

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
		printf("
 1. INC/SysConfig.h
 2. Make Array
 3. Make Default CALI_BD
 4. Select Cycler Type
\n");

		printf(" EXIT : 'ctrl + c'\n");
		printf(" Select No : ");
		scanf("%d", &item);

		switch(item) {
			case 1:
				SysConfig();
				break;
			case 2:
				makeArray();
				break;
			case 3:
				makeDefault_CALI_BD();
				break;
			case 4:
				selectType();
				break;
			case 5:
			//	mControl_Config();
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
	printf("0 : 100ms	=> ADC 1EA, chPerBD <= 32CH, ADC 2EA, chPerBD <= 64CH
1 : 50ms	=> ADC 1EA, chPerBD <= 16CH, ADC 2EA, chPerBD <= 32CH
2 : 25ms	=> ADC 2EA, chPerBD <= 16CH
3 : 20ms	=> ADC 2EA, chPerBD <=  8CH
4 : 100ms_2	=> ADC 1EA, chPerBD <= 64CH
5 : 10ms	=> ADC 2EA, chPerBD <=  8CH
6 : 100ms_CAN TYPE\n");
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
	printf("Default)
Range0 = 0
Range1 = 0 
Range2 = 2
Range3 = 4
Range4 = 0\n\n");
		
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
		fprintf(fp, "0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 
0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 
================================================================================
Option Setting Description
Update : 2017.01.16
Writer : OYS
================================================================================
installedBd			=> insalled MainBoard No.
installedCh			=> installed Total Ch
chPerBd				=> Ch Per MainBD
rangeV				=> Voltage Range No.
rangeI				=> Current Range No.
maxVoltage
minVoltage
maxCurrent
minCurrent
ratioVoltage		=> 0 : Micro, 1 : Nano (Default : 0)
ratioCurrent		=> 0 : Micro, 1 : Nano (Default : 0)
watchdogFlag	 	=> WatchDog UseFlag (Default : 1)	
ADC_type			=> 0 : COMM_TYPE, 1 : 7805 (Default : 1)
hwSpec				=> H/W SPEC No. ex) 63 : L_5V_150A_R3_AD2
capacityType		=> 0 : AmpareHour, 1 : Farad
totalJig
bdInJig1_8
bdInJig9_16
shuntOhm			=> Range Shunt Value
gain				=> Range Gain Value
adAmp				=> AD_AMP	VOLTAGE_AMP	 CURRENT_AMP
rt_scan_type		=> 0 : 100ms	=> ADC 1EA, chPerBD <= 32CH
									   ADC 2EA, chPerBD <= 64CH
					   1 : 50ms		=> ADC 1EA, chPerBD <= 16CH
									   ADC 2EA, chPerBD <= 32CH
					   2 : 25ms		=> ADC 2EA, chPerBD <= 16CH
					   3 : 20ms		=> ADC 2EA, chPerBD <=  8CH
					   4 : 100ms_2	=> ADC 1EA, chPerBD <= 64CH
					   5 : 10ms		=> ADC 2EA, chPerBD <=  8CH
					   6 : 100ms_CAN TYPE
installedTemp		=> AuxTemperature installed Ch No.
installedAux		=> AuxVoltage installed Ch No.
installedCan		=> AuxCan installed Ch No.
auto_v_cali			=> auto Voltage Cali (Default : 1)
parallelMode		=> parallelMode UseFlag
DAC_type			=> 0 : 712P, 1 : 7741
ADC_num				=> ADC No.(1:ADC 1EA, 2:ADC 2EA)
AD_offset			=> Charge Current AD Offset Value (Default : 0.00002)
FeedbackFlag		=> SoftFeedback UseFlag
MainBdType			=> 0 : CPLD TYPE, 1: FPGA TYPE
FadBdUse			=> FAD BD UseFlag
RangeSelectFlag
Range0
Range1
Range2
Range3
Range4");
	}
	fclose(fp);

	sleep(1);
	return 0;
}

int SysConfig(void)
{
	int i, value, loop;
	char cmd[128], fileName[32], INC[64], Scripts[64];
	int  equ_type = 0, cycler_type = 0, vender = 0, daehwa = 0,
		 change_vi_check = 0, network_ver = 0, can_type = 0,
		 shunt_r_rcv = 0, aux_control = 0, dio_type = 0,
		 edlc_type = 0, program_ver[4], system_type = 0,
		 integratedGUI = 0, userPatternType = 0;
	FILE *fp;
	
	memset(program_ver, 0x00, sizeof(program_ver));
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
1 : VMware Workstation
			\n");
	printf("Select operation system type\n>>");
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
2 : CAN
\n");
	printf("Select Cycler type\n>>");
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
9 : JAPAN
\n");
	printf("Select Vender\n>>");
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
4 : 4105 (SDI MES4)
\n");
	printf("Select Network Version\n>>");
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
/*
	printf("\nCAN통신 타입의 MainBoard 입니까?\n");
	printf("Is it CAN communication type main board? [0:No 1:Yes]\n>>");
	loop = 1;
	while(loop) {
		scanf("%d",&can_type);
		while(getchar() != '\n');
		if(can_type >= 0 && can_type <= 1)
				loop = 0;
		else
			printf("유효하지 않은 값입니다. 다시 입력하십시오.\nValue is no valid. please enter again\n>>");
	}
*/
	printf("\nSHUNTI_R_RCV Select\n
0 : NORMAL [OLD TYPE]
1 : SHUNT VALUE RECEIVE [DEFAULT]
2 : SDI TYPE
\n");
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
3 : DOOR OPEN & SMOKE DETECTION
\n");
	printf("Select I/O function to use\n>>");
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

	/*
	program_ver[0] = 0;			//PROGRAM_VERSION1
	if(cycler_type == 0) {
		if(daehwa == 1)
			program_ver[1] = 2;	//PROGRAM_VERSION2
		else
			if(integratedGUI == 0) {
				program_ver[1] = 1;	//PROGRAM_VERSION2
			} else {
				program_ver[1] = 2;	//PROGRAM_VERSION2
			}
	} else {
		program_ver[1] = 2;		//PROGRAM_VERSION2
	}
	program_ver[2] = 0;	//PROGRAM_VERSION3
	program_ver[3] = 5;	//PROGRAM_VERSION4
	*/
	
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
		
		fprintf(fp,"#define SYSTEM_TYPE	%d\n", system_type);
		fprintf(fp, "\n");
		
		fprintf(fp,"#define LINEAR_CYC		0\n");
		fprintf(fp,"#define DIGITAL_CYC		1\n");
		fprintf(fp,"#define CAN_CYC			2\n");
		fprintf(fp, "\n");
		
		fprintf(fp,"#define VENDER	%d\n", vender);
		switch(network_ver) {
			case 1:
				fprintf(fp,"#define NETWORK_VERSION	4101\n");
				break;
			case 2:
				fprintf(fp,"#define NETWORK_VERSION	4102\n");
				break;
			case 3:
				fprintf(fp,"#define NETWORK_VERSION	4103\n");
				break;
			case 4:
				fprintf(fp,"#define NETWORK_VERSION	4105\n");
				break;
			default:
				fprintf(fp,"#define NETWORK_VERSION	4102\n");
				break;
		}
		fprintf(fp, "\n");
		
		switch(cycler_type){
			case 0:
				fprintf(fp, "#define CYCLER_TYPE	0\n");
				break;
			case 1:
				fprintf(fp, "#define CYCLER_TYPE	1\n");
				break;
			case 2:
				fprintf(fp, "#define CYCLER_TYPE	1\n");
				break;
			default:
				fprintf(fp, "#define CYCLER_TYPE	0\n");
				break;
		}
		fprintf(fp, "#define _CYCLER\n");
		fprintf(fp, "//#define _PACK_CYCLER\n");
		fprintf(fp, "\n");

		fprintf(fp,"#define REAL_TIME	1\n");
		fprintf(fp,"#define SHUNT_R_RCV	%d\n", shunt_r_rcv);
		fprintf(fp,"#define AUX_CONTROL	%d\n", aux_control);
		fprintf(fp,"#define DIO_TYPE	%d\n", dio_type);
		fprintf(fp,"#define SEC_TYPE	0\n");
		fprintf(fp,"#define EDLC_TYPE	%d\n", edlc_type);
		fprintf(fp,"#define MACHINE_TYPE	0\n");
		fprintf(fp,"#define CH_AUX_DATA	0\n");
		fprintf(fp,"#define USER_PATTERN_500	%d\n", userPatternType);
		fprintf(fp,"#define SK_CALI_TYPE	0\n");
		fprintf(fp,"#define TEMP_CALI	0\n");

		if(cycler_type == 1) {
			fprintf(fp, "#define _JIG_TYPE_3\n");
		} else {
			fprintf(fp, "#define _JIG_TYPE_0\n");
		}
		
		fprintf(fp, "\n");
		fprintf(fp,"#define DAEHWA	%d\n", daehwa);
		fprintf(fp, "\n");
		
		/*
		for(i = 0; i < 4; i++){
			fprintf(fp,"#define PROGRAM_VERSION%d	%d\n", i+1, program_ver[i]);
		}
		fprintf(fp, "\n");

		fprintf(fp,"#define VENDER	%d\n", vender);
		
		if(cycler_type == 0) {
			fprintf(fp, "#define CYCLER_TYPE	0\n");
		} else {
			fprintf(fp, "#define CYCLER_TYPE	1\n");
		}

		if(equ_type == 0) {
			fprintf(fp, "#define _CYCLER\n");
		} else {
			fprintf(fp, "#define _PACK_CYCLER\n");
		}

		switch(network_ver) {
			case 1:
				fprintf(fp, "#define __LG_VER1__\n");
				fprintf(fp,"#define NETWORK_VERSION	4101\n");
				fprintf(fp, "#define DATA_SAVE_VER	0\n");
				break;
			case 2:
				fprintf(fp, "#define __LG_VER1__\n");
				fprintf(fp,"#define NETWORK_VERSION	4102\n");
				fprintf(fp, "#define DATA_SAVE_VER	0\n");
				break;
			case 3:
				fprintf(fp, "#define __LG_VER1__\n");
				fprintf(fp,"#define NETWORK_VERSION	4103\n");
				fprintf(fp, "#define DATA_SAVE_VER	0\n");
				break;
			case 4:
				fprintf(fp, "#define __SDI_MES_VER4__\n");
				fprintf(fp,"#define NETWORK_VERSION	4105\n");
				fprintf(fp, "#define DATA_SAVE_VER	1\n");
				break;
			default:
				fprintf(fp, "#define __LG_VER1__\n");
				fprintf(fp,"#define NETWORK_VERSION	4102\n");
				fprintf(fp, "#define DATA_SAVE_VER	0\n");
				break;
		}
		
		switch(vender) {
			case 1:
				if(cycler_type == 0) {	//Linear
					if(daehwa == 1) {
						fprintf(fp, "#define USER_VI	0\n");
						fprintf(fp, "#define REAL_TIME	1\n");
						fprintf(fp, "#define VERSION_DETAIL_SHOW	0\n");
					} else {
						fprintf(fp, "#define USER_VI	1\n");
						if(integratedGUI == 0) {
							fprintf(fp, "#define REAL_TIME	0\n");
						} else {
							fprintf(fp, "#define REAL_TIME	1\n");
						}
						fprintf(fp, "#define VERSION_DETAIL_SHOW	1\n");
					}
				} else {	//Digital
					if(daehwa == 1) {
						fprintf(fp, "#define USER_VI	0\n");
						fprintf(fp, "#define REAL_TIME	1\n");
						fprintf(fp, "#define VERSION_DETAIL_SHOW	0\n");
					} else {
						fprintf(fp, "#define USER_VI	1\n");
						fprintf(fp, "#define REAL_TIME	1\n");
						fprintf(fp, "#define VERSION_DETAIL_SHOW	1\n");
					}
				}
				break;
			case 2:
				fprintf(fp, "#define USER_VI	0\n");
				if(network_ver == 4){
					fprintf(fp, "#define REAL_TIME	1\n");
				} else {
					fprintf(fp, "#define REAL_TIME	0\n");
				}
				fprintf(fp, "#define VERSION_DETAIL_SHOW	0\n");
				break;
			case 3:
				fprintf(fp, "#define USER_VI	0\n");
				fprintf(fp, "#define REAL_TIME	1\n");
				fprintf(fp, "#define VERSION_DETAIL_SHOW	1\n");
				break;
			default:
				fprintf(fp, "#define USER_VI	0\n");
				fprintf(fp, "#define REAL_TIME	1\n");
				fprintf(fp, "#define VERSION_DETAIL_SHOW	0\n");
				break;
		}

		fprintf(fp,"#define DAEHWA	%d\n", daehwa);
		fprintf(fp,"#define CHANGE_VI_CHECK	%d\n", change_vi_check);
		fprintf(fp,"#define SEC_TYPE	0\n");
		fprintf(fp,"#define CAN_TYPE	%d\n", can_type);
		fprintf(fp,"#define SHUNT_R_RCV	%d\n", shunt_r_rcv);
		fprintf(fp,"#define AUX_CONTROL	%d\n", aux_control);
		fprintf(fp,"#define DIO_TYPE	%d\n", dio_type);
		fprintf(fp,"#define EDLC_TYPE	%d\n", edlc_type);
		fprintf(fp,"#define MACHINE_TYPE	0\n");
		fprintf(fp,"#define CH_AUX_DATA	0\n");
		fprintf(fp,"#define USER_PATTERN_500	%d\n", userPatternType);

		if(cycler_type == 0) {
			fprintf(fp, "#define _JIG_TYPE_0\n");
		} else {
			fprintf(fp, "#define _JIG_TYPE_3\n");
		}

		fprintf(fp, "\n");
		fprintf(fp, "#endif\n");
	}
	fclose(fp);

	*/
	printf("프로그램 컴파일을 진행하겠습니까?\n");
	printf("start compiling? [0:NO 1:Compile only 2:Compile and reboot]\n>>");
	loop = 1;
	while(loop) {
		scanf("%d", &value);
		while(getchar() != '\n');
		if(value >= 0 && value <= 2)
				loop = 0;
		else
			printf("유효하지 않은 값입니다. 다시 입력하십시오.\nValue is no valid. please enter again\n>>");
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

int makeArray(void)
{
	int i, val, loop;
	unsigned int ch;
	char fileName[128];
	FILE *fp;

	loop = 1;
	val = 100;
	while(loop) {
		printf("Array Type Select (0:Pass, 1:CellArray, 2:TempArray) : ");
		scanf("%d", &val);
		while(getchar() != '\n');
		if(val >= 0 && val <=2) {
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
		printf("Cycler Type Select (0:Pass, 1:Linear, 2:Digital) : ");
		scanf("%d", &val);
		while(getchar() != '\n');
		if(val >= 0 && val <=2) {
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
