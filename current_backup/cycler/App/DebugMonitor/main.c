#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>		//use for atoi function
#include <asm/io.h>
#include <stdlib.h>
#include <unistd.h>		//use for open,read close function
#include "../../INC/datastore.h"
#include "common_utils.h"
#include "main.h"
#include "time.h"

volatile S_SYSTEM_DATA *myData;
int bd, mode, fpf=0;
long min[9], max[9];

void Test_Print(unsigned char flag)
{
	switch(flag){
		case P0:
			DataSize_Print();
			break;
		case P1:
			Debug_Print();
			break;
		case P2:
			DAQ_Print();
			break;
		case P3:
			Addr_Print();
			break;
		case P4:
			TimeSlot_Print();
			break;
		case P5:
			AppControl_Print();
			break;
		case P6:
			DataSave_Print();
			break;
		case P7:
			CaliMeter_Print();
			break;
		case P8:
			CaliMeter2_Print();
			break;
		case P9:
			AnalogMeter_Print();
			break;
		case P10:
			FADM_Print();
			break;
		case P11:
			ModuleState_Print();
			break;
		case P12:
			GroupState_Print();
			break;
		case P13:
			Reference_IC_Data_Save();
			break;
		case P14:
			BoardState_Print();
			break;
		case P15:
			ChannelState_Print();
			break;
		case P16:
			DIO_Print();
			break;
		case P17:
			TestCond_Print();
			break;
		case P18:
			CaliData_Print();
			break;
		case P19:
			CaliMeter_Config_Print();
			break;
		case P20:
			Chamber_Temp_Change_Print();
			break;
		case P21:
			Temp_Change_Print();
			break;
		case P22:
			HwFault_Config_Print();
			break;
		case P23:
			PCU_State_Print();
			break;
		case P24:
			CAN_State_Print();
			break;
		case P25:
			UserDataNo_Print();
			break;
		case P26:
			AC_Fail_Recovery_print();
			break;
		case P27:
			CAN_JIG_IO_Change_Print();
			break;
		case P28:
			MainClient_Config_Print();
			break;
		case P29:
			GAS_AMBIENT_TEST();
			main();
			break;
		case P99:
			Convert_FormToCyc_CaliData();
			break;
		case P100:
			Read_HwFault_Config();
		default : 
			main();
			break;
	}
}

void DataSize_Print(void)
{
	printf("\nDataSize_Print\n\n");
	
	printf("SYSTEM_DATA\t\t : %d bytes\n", sizeof(S_SYSTEM_DATA));
	printf("MODULE_DATA\t\t : %d bytes\n", sizeof(S_MODULE_DATA));
	printf("BD_DATA\t\t\t : %d bytes\n", sizeof(S_BD_DATA));
	printf("CH_DATA\t\t\t : %d bytes\n", sizeof(S_CH_DATA));
	printf("CH_OP_DATA\t\t : %d bytes\n", sizeof(S_CH_OP_DATA));
	printf("CH_MISC\t\t\t : %d bytes\n", sizeof(S_CH_MISC));
	printf("MODULE_MISC\t\t : %d bytes\n", sizeof(S_MODULE_MISC));
	printf("MAIN_SEND_CH_DATA\t : %d bytes\n", sizeof(S_MAIN_SEND_CMD_CH_DATA));
	printf("MAIN_CH_DATA\t\t : %d bytes\n", sizeof(S_MAIN_CH_DATA));
	printf("MAIN_HEADER\t\t : %d bytes\n", sizeof(S_MAIN_CMD_HEADER));
	printf("MAIN_TEST_CONDITION\t : %d bytes\n", sizeof(S_MAIN_TEST_CONDITION));
	printf("SAVE_MSG_VAL\t\t : %d bytes\n", sizeof(S_SAVE_MSG_VAL));

}

void Debug_Print(void)
{
	int i;
//	int j;
//	int k;
//	int cnt;
//	short int tmpVal;
//	unsigned char tmp;
	printf("\nDebug_Print\n");

/*
	printf("monitor %d, hw %d, bd %d, ch %d\n",
		myData->CellArray1[0].number1-1,
		myData->CellArray1[0].number2-1,
		myData->CellArray1[0].bd,
		myData->CellArray1[0].ch);

	printf("hwSpec : %d\n", myData->mData.config.hwSpec);
	printf("mainBdType : %d\n", myData->mData.config.MainBdType);
	printf("totalJig : %d\n", myData->mData.config.totalJig);
	printf("tempBoundary : %ld\n", myData->mData.config.tempBoundary);
	printf("bdInJig : ");
	for( i = 0; i <16; i++){
		printf("%d, ",myData->mData.config.bdInJig[i]);
	}	
	printf("\n");
    printf("SysData size %d\n", sizeof(S_SYSTEM_DATA));
	printf("save_msg size %d\n", sizeof(S_SAVE_MSG));

	printf("\n");
	printf("ad_offset : %f\n", myData->mData.config.AD_offset);
*/
	printf("\nDecimal value(long)\n");
	for(i=0;i<20;i++){
		printf("%d:%ld ",i,myData->test_val_l[i]); 
	}
	printf("\n");

//	printf("%ld ", myData->bData[bd].cData[i].op.runTime);
//	printf("runtime : %ld, limitTime : %ld", myData->bData[0].cData[0].op.runTime, myData->bData[0].cData[0].misc.limit_current_timeout);
	
	printf("\nDecimal value(float)\n");
	for(i=0;i<16;i++){
		printf("%2d:%f ",i,myData->test_val_f[i]); 
		if((i+1) % 4 == 0){
			printf("\n");
		}
	}
/*	printf("\n");

	printf("line drop %f\n", myData->cali.line_drop);
	printf("line drop i %f\n", myData->cali.line_drop_i);
	printf("line impedcnae %f\n", myData->cali.line_impedance);
*/
	printf("\nDecimal value(char)\n");
	for(i=0;i<16;i++){
		printf("%d:%d ",i,myData->test_val_c[i]); 
	}	
	printf("\n");
/*	
	cnt = (int)(myData->mData.misc.timer_1sec - myData->MainClient.netTimer);
	printf("%d\n",cnt);
	*/
	printf("\nAutoUpdate Process retry time : %d, now : %d\n",
				myData->AutoUpdate.config.retryInterval,
				myData->AutoUpdate.misc.timeCnt);
}

void DAQ_Print(void)
{
	int i;
	printf("\nDAQ_Print\n");

	printf("op aux v\n");
	for(i=0;i<64;i++)
	{
		printf("%1.4f ",(float)myData->daq.op.ch_vsens[i]/1000000);
		if(!((i+1)%8))
				printf("\n");
	}
	
	printf("misc aux v\n");
	for(i=0;i<64;i++)
	{
		printf("%d ",myData->daq.data[0].ad_ch[i]);
		if(!((i+1)%8))
				printf("\n");
	}
	printf("\n");

	printf("aux ref\n");
	for(i=0;i<4;i++)
	{
		printf("%d ",myData->daq.data[0].ad_ref[0][i]);
	}
	printf("\n");

/*	
	for(i=0;i<360;i++) {
		printf("%d : %x, ",i,myData->test_val_daq[i]);
		if(!((i+1)%8))
				printf("\n");
	}
	
	for(i=0;i<myData->mData.config.installedAuxV;i++)
	{
		printf("%d : %f,%f ",i,myData->daq.cali.AD_A[i],myData->daq.cali.AD_B[i]
						,myData->daq.misc.caliFlag[i]);
				printf("\n");
	}*/
/*
	
	printf("\nref \n");
	for(i=0;i<8;i++)
	{
		printf("%f , %f\n ",myData->daq.misc.DAQ_AD_A[i],myData->daq.misc.DAQ_AD_B[i]);
	}
	printf("\nref_N \n");
	for(i=0;i<8;i++)
	{
		printf("%f , %f\n ",myData->daq.misc.DAQ_AD_A_N[i],myData->daq.misc.DAQ_AD_B_N[i]);
	}
*/	
	printf("\n");
}

void Addr_Print(void)
{
	int i;

	printf("\nAddr_Print\n");
	printf("Input : ");
	for(i = 0; i < 32; i++){
		printf("%x\t", myData->mData.addr.interface[0][i]);
	}
	printf("\n");

	printf("Output : ");
	for(i = 0; i < 32; i++){
		printf("%x\t", myData->mData.addr.interface[1][i]);
	}
	printf("\n");

	printf("Expend : ");
	for(i = 0; i < 32; i++){
		printf("%x\t", myData->mData.addr.interface[2][i]);
	}
	printf("\n");

	printf("Addr Main\n");

	printf("Main : ");
	for(i = 0; i < 32; i++){
		printf("%x\t", myData->mData.addr.main[i]);
	}
	printf("\n");
}

void TimeSlot_Print(void)
{
	int i, j, SlotNum;
	long tmp1, tmp2;
	//long max, min;

	printf("\nTimeSlot_Print\n");

	tmp1 = tmp2 = 0;
	SlotNum = 0;
	if(myData->mData.config.hwSpec == L_6V_6A
		|| myData->mData.config.hwSpec == S_5V_200A
		|| myData->mData.config.hwSpec == L_5V_50A) {
		for(i=0; i < 10; i++) {
			printf("Time %02d ", i*10);
			for(j=0; j < 10; j++) {
				printf("%07ld ", myData->mData.runningTime[0][i*10+j]);
				tmp1 += myData->mData.runningTime[0][i*10+j];
				tmp2 += myData->mData.runningTime[1][i*10+j];
			}
			printf("\n");
		}
		printf("runTime average [function]:%07ld, [all]:%07ld\n",
			tmp1/100, tmp2/100);
	} else if(myData->mData.config.hwSpec == L_5V_10mA) {
		for(i=0; i < 10; i++) {
			printf("Time %02d ", i*20);
			for(j=0; j < 20; j++) {
				printf("%07ld ", myData->mData.runningTime[0][i*10+j]);
				tmp1 += myData->mData.runningTime[0][i*10+j];
				tmp2 += myData->mData.runningTime[1][i*10+j];
			}
			printf("\n");
		}
		printf("runTime average [function]:%07ld, [all]:%07ld\n",
			tmp1/200, tmp2/200);
	}
	if(myData->mData.config.rt_scan_type == 0){
		SlotNum = 100;
	}else if(myData->mData.config.rt_scan_type == 2){
		SlotNum = 20;
	}else if(myData->mData.config.rt_scan_type == 3){
		SlotNum = 20;
	}else if(myData->mData.config.rt_scan_type == 5){
		SlotNum = 10;
	}

	printf("runTime max:%07ld[%ld], min:%07ld[%ld]\n",
		myData->mData.runningTime[2][0], myData->mData.runningTime[2][1],
		myData->mData.runningTime[3][0], myData->mData.runningTime[3][1]);
				printf("max\n");
	for(i=0;i< SlotNum;i++){
		printf("%d:%ld ",i,myData->mData.runningTime[6][i]);
		if(!((i+1)%8))
				printf("\n");
	}
	printf("\nnow\n");
	for(i=0;i<SlotNum;i++){
		printf("%d:%ld ",i,myData->mData.runningTime[0][i]);
		if(!((i+1)%8))
				printf("\n");
	}
	printf("\n");
	printf("\nsum\n");
	for(i=0;i<SlotNum;i++){
		printf("%d:%ld ",i,myData->mData.runningTime[4][i]);
		if(!((i+1)%8))
				printf("\n");
	}
	printf("\n");

}

void AppControl_Print(void)
{
	printf("\nAppControl_Print\n");
	printf("app signal %d\n",
		myData->AppControl.signal[APP_SIG_APP_CONTROL_PROCESS]);
	printf("\n");
}

void DataSave_Print(void)
{
	printf("\nDataSave_Print\n");
	printf("signal %d\n",
		myData->DataSave.signal[DATASAVE_SIG_SAVED_FILE_DELETE]);
	printf("\n");
}

void CaliMeter_Print(void)
{
	printf("\nCaliMeter_Print\n");

	printf("signal %d\n",
		myData->CaliMeter.signal[CALI_METER_SIG_REQUEST_PHASE]);

	printf("\n");
}

void CaliMeter2_Print(void)
{
	printf("\nCaliMeter2_Print\n");

	printf("signal %d\n",
		myData->CaliMeter2.signal[CALI_METER_SIG_REQUEST_PHASE]);

	printf("comPort %d\n",
		myData->CaliMeter2.config.comPort);
	printf("comBps %d\n",
		myData->CaliMeter2.config.comBps);
	printf("readType %d\n",
		myData->CaliMeter2.config.readType);
	printf("measureI %d\n",
		myData->CaliMeter2.config.measureI);
	printf("\n");
}

void AnalogMeter_Print(void)
{
	int i;
//	int j;
	//int k;
	printf("\nAnalogMeter_Print\n");
	if(myData->AnalogMeter.config.functionType)
			printf("Type : Yokogawa\n");
	else
			printf("Type : CB7010\n");
/*	
	printf("signal %d\n",
		myData->AnalogMeter.signal[ANALOG_METER_SIG_MEASURE]);
	
	for(i=0;i<2;i++){
		for(j=0;j<8;j++){
			printf("mul = %ld\t", 
							myData->AnalogMeter.cali_config.mulPoint[0][j]);
			printf("gain2 = %f\t", 
							myData->AnalogMeter.cali_config.measure_gain2[0][i][j]);
			printf("offset2 = %ld\t",
							myData->AnalogMeter.cali_config.measure_offset2[0][i][j]);
			printf("caliFalg2 = %ld\n",
							myData->AnalogMeter.cali.caliFlag2[i][j]);
		}
	}
	printf("\n");
	
	printf("test : %d ",myData->AnalogMeter.cali_config.useRange);
	printf("\n");
	*/
	printf("org Data\n");
	if(myData->AnalogMeter.config.functionType)
	{
		for(i=0; i < 100; i++) {
			//printf("%ld ",myData->AnalogMeter.temp[i].temp);
			printf("%ld ",myData->AnalogMeter.misc2.tempData[i]);
			if(!((i+1)%10))
				printf("\n");
		}
		for(i=100; i < 150; i++) {
			//printf("%ld ",myData->AnalogMeter2.temp[i-100].temp);
			//printf("%ld ",myData->AnalogMeter2.misc2.tempData[i-100]);
			//if(!((i+1)%10))
			//	printf("\n");
		}

	} else{
		for(i=0; i < 100; i++) {
			printf("%ld ",myData->AnalogMeter.misc2.tempData[i]);
			//printf("%d CH AnalogMeter TEMP0 : %ld\n ", i, myData->AnalogMeter.temp[i].temp);
			//printf("%d CH AnalogMeter TEMP1 : %ld\n ", i, myData->AnalogMeter.temp[i].temp1);
			//printf("%d CH op.temp TEMP0 : %ld\n ", i, myData->bData[0].cData[i].op.temp);
			//printf("%d CH op.temp1 TEMP1 : %ld\n ", i, myData->bData[0].cData[i].op.temp1);
			//printf("%ld ",myData->bData[0].cData[i].op.temp);
			if(!((i+1)%10))	printf("\n");
		}
		for(i=100; i < 150; i++) {
			//printf("%ld ",myData->AnalogMeter2.temp[i-100].temp);
			//printf("%ld ",myData->AnalogMeter2.misc2.tempData[i-100]);
			//if(!((i+1)%10))	printf("\n");
		}

	}
	printf("\n\n");
	printf("Data\n");
	for(i=0; i < 100; i++) {
		printf("%ld ",myData->AnalogMeter.temp[i].temp);
		if(!((i+1)%10))	printf("\n");
	}
	for(i=100; i < 150; i++) {
//		printf("%ld ",myData->AnalogMeter2.temp[i-100].temp);
//		if(!((i+1)%10))	printf("\n");
	}
	printf("\n");
}

void FADM_Print(void)
{
	int ch, i;
	long val, min, max, tmp;

	printf("\nFADM_Print\n");

	printf("signal[FADM_SIG_MEASURE_READY] %d\n",
		myData->FADM.signal[FADM_SIG_MEASURE_READY]);

	printf("signal[FADM_SIG_MEASURE] %d\n",
		myData->FADM.signal[FADM_SIG_MEASURE]);

	printf("comm_buffer.use_flag %d\n",
		myData->FADM.comm_buffer.use_flag);

	ch = 3;
	val = 0;
	printf("ad_org v\n");
	min = max = myData->FADM.pulse_ad_org[0][ch].value[0][0];
	for(i=0; i < 300; i++) {
		tmp = (long)myData->FADM.pulse_ad_org[0][ch].value[0][i];
		printf("%ld ", tmp);
		val += tmp;
		if(tmp < min) min = tmp;
		else if(tmp > max) max = tmp;
	}
	printf(": %ld, min:%ld, max:%ld\n", val / 300, min, max);

	val = 0;
	printf("ad_org i\n");
	min = max = myData->FADM.pulse_ad_org[0][ch].value[1][0];
	for(i=0; i < 300; i++) {
		tmp = (long)myData->FADM.pulse_ad_org[0][ch].value[1][i];
		printf("%ld ", tmp);
		val += tmp;
		if(tmp < min) min = tmp;
		else if(tmp > max) max = tmp;
	}
	printf(": %ld, min:%ld, max:%ld\n", val / 300, min, max);

	val = 0;
	printf("ad_ref_org v_p\n");
	min = max = myData->FADM.pulse_ad_org[0][ch].ref[0][0][0];
	for(i=0; i < 20; i++) {
		tmp = (long)myData->FADM.pulse_ad_org[0][ch].ref[0][0][i];
		printf("%ld ", tmp);
		val += tmp;
		if(tmp < min) min = tmp;
		else if(tmp > max) max = tmp;
	}
	printf(": %ld, min:%ld, max:%ld\n", val / 20, min, max);

	val = 0;
	printf("ad_ref_org v_n\n");
	min = max = myData->FADM.pulse_ad_org[0][ch].ref[0][1][0];
	for(i=0; i < 20; i++) {
		tmp = (long)myData->FADM.pulse_ad_org[0][ch].ref[0][1][i];
		printf("%ld ", tmp);
		val += tmp;
		if(tmp < min) min = tmp;
		else if(tmp > max) max = tmp;
	}
	printf(": %ld, min:%ld, max:%ld\n", val / 20, min, max);

	val = 0;
	printf("ad_ref_org v_0\n");
	min = max = myData->FADM.pulse_ad_org[0][ch].ref[0][2][0];
	for(i=0; i < 20; i++) {
		tmp = (long)myData->FADM.pulse_ad_org[0][ch].ref[0][2][i];
		printf("%ld ", tmp);
		val += tmp;
		if(tmp < min) min = tmp;
		else if(tmp > max) max = tmp;
	}
	printf(": %ld, min:%ld, max:%ld\n", val / 20, min, max);

	val = 0;
	printf("ad_ref_org i_p\n");
	min = max = myData->FADM.pulse_ad_org[0][ch].ref[1][0][0];
	for(i=0; i < 20; i++) {
		tmp = (long)myData->FADM.pulse_ad_org[0][ch].ref[1][0][i];
		printf("%ld ", tmp);
		val += tmp;
		if(tmp < min) min = tmp;
		else if(tmp > max) max = tmp;
	}
	printf(": %ld, min:%ld, max:%ld\n", val / 20, min, max);

	val = 0;
	printf("ad_ref_org i_n\n");
	min = max = myData->FADM.pulse_ad_org[0][ch].ref[1][1][0];
	for(i=0; i < 20; i++) {
		tmp = (long)myData->FADM.pulse_ad_org[0][ch].ref[1][1][i];
		printf("%ld ", tmp);
		val += tmp;
		if(tmp < min) min = tmp;
		else if(tmp > max) max = tmp;
	}
	printf(": %ld, min:%ld, max:%ld\n", val / 20, min, max);

	val = 0;
	printf("ad_ref_org i_0\n");
	min = max = myData->FADM.pulse_ad_org[0][ch].ref[1][2][0];
	for(i=0; i < 20; i++) {
		tmp = (long)myData->FADM.pulse_ad_org[0][ch].ref[1][2][i];
		printf("%ld ", tmp);
		val += tmp;
		if(tmp < min) min = tmp;
		else if(tmp > max) max = tmp;
	}
	printf(": %ld, min:%ld, max:%ld\n", val / 20, min, max);

	printf("ad v\n");
	min = max = myData->FADM.pulse_ad[0][ch].value[0][0];
	for(i=0; i < 300; i++) {
		tmp = myData->FADM.pulse_ad[0][ch].value[0][i];
		printf("%ld ", tmp);
		if(tmp < min) min = tmp;
		else if(tmp > max) max = tmp;
	}
	printf(", min:%ld, max:%ld\n", min, max);

	printf("ad i\n");
	min = max = myData->FADM.pulse_ad[0][ch].value[1][0];
	for(i=0; i < 300; i++) {
		tmp = myData->FADM.pulse_ad[0][ch].value[1][i];
		printf("%ld ", tmp);
		if(tmp < min) min = tmp;
		else if(tmp > max) max = tmp;
	}
	printf(", min:%ld, max:%ld\n", min, max);

	printf("\n");
}

void ModuleState_Print(void)
{
	int i;

	printf("\nModuleState_Print\n");
	
	printf("installedBd %d\n", myData->mData.config.installedBd);
	printf("installedCh %d\n", myData->mData.config.installedCh);
	printf("chPerBd %d\n", myData->mData.config.chPerBd);
	printf("rangeV %d\n", myData->mData.config.rangeV);
	printf("rangeI %d\n", myData->mData.config.rangeI);
	printf("maxVoltage ");
	for(i=0; i < MAX_RANGE; i++) {
		printf("%ld ", myData->mData.config.maxVoltage[i]);
	}
	printf("\n");

	printf("maxCurrent ");
	for(i=0; i < MAX_RANGE; i++) {
		printf("%ld ", myData->mData.config.maxCurrent[i]);
	}
	printf("\n");

	printf("minVoltage ");
	for(i=0; i < MAX_RANGE; i++) {
		printf("%ld ", myData->mData.config.minVoltage[i]);
	}
	printf("\n");

	printf("minCurrent ");
	for(i=0; i < MAX_RANGE; i++) {
		printf("%ld ", myData->mData.config.minCurrent[i]);
	}
	printf("\n");

	printf("ADC_type %d\n",	myData->mData.config.ADC_type);
	printf("capacityType %d\n",myData->mData.config.capacityType);

	printf("7.5V Fault Flag ");
	for(i=0; i < 8; i++) {
		printf("%x ", myData->mData.fault.SMPS7_5V[i]);
	}
	printf("\n");

	printf("3.3V Fault Flag ");
	for(i=0; i < 8; i++) {
		printf("%x ", myData->mData.fault.SMPS3_3V[i]);
	}
	printf("\n");

	printf("OT Fault Flag ");
	for(i=0; i < 8; i++) {
		printf("%x ", myData->mData.fault.OT[i]);
	}
	printf("\n");
	printf("signal buzzer %d\n", myData->mData.signal[M_SIG_LAMP_BUZZER]);
	printf("signal remote_SMPS %d\n", myData->mData.signal[M_SIG_REMOTE_SMPS1]);

	printf("Shunt : ");
	for(i=0; i < 4; i++) {
		printf("%f ", myData->mData.config.shunt[i]);
	}
	printf("\n");
	
	printf("Gain : ");
	for(i=0; i < 4; i++) {
		printf("%f ", myData->mData.config.gain[i]);
	}
	printf("\n");

	printf("AD Amp : ");
	printf("%f ", myData->mData.config.adAmp);
	printf("%f ", myData->mData.config.voltageAmp);
	printf("%f ", myData->mData.config.currentAmp);
	printf("\n");
	printf("ambientTemp_org : ");
	for(i = 0 ; i < 10 ; i++){
		printf("%ld ",myData->mData.misc.ambientTemp_org[i]);
	}
	printf("\n");
	printf("ambientTemp : ");
	for(i = 0 ; i < 10 ; i++){
		printf("%ld ",myData->mData.misc.ambientTemp[i]);
	}
	printf("\n");
	printf("gasVoltage : ");
	//for(i = 0 ; i < 8 ; i++){	//M-7003
	for(i = 0 ; i < myData->AnalogMeter.config.chPerModule2 ; i++){	//M-7002
		printf("%ld ",myData->mData.misc.gasVoltage[i]);
	}
	printf("\n");
	printf("org_gasVoltage : ");
	//for(i = 0 ; i < 8 ; i++){	//M-7003
	for(i = 0 ; i < myData->AnalogMeter.config.chPerModule2 ; i++){	//M-7002
		printf("%ld ",myData->mData.misc.gasVoltage_org[i]);
	}
	printf("\n");
}

void GroupState_Print(void)
{
	printf("\nGroupState_Print\n");
	printf("\n");
}

void Reference_IC_Data_Save(void)
{
	int type, ch;
	FILE	*fp;
	
	printf("\nReference_IC_Data_Save\n");
	fp = fopen("reference_ic_ad_data.csv","a+");
	if(fp == NULL) printf("reference_ic_ad_data.scv File Open Fail!!!!!\n");
	if(fpf==0){
		fprintf(fp,"V(+),V(-),GND_V1,GND_V2,I(+),I(-),GND_I1,GND_I2\n");
		fpf++;
	}
	for(type = 0; type < 2; type++){
		for(ch = 0; ch < 4; ch++){
			fprintf(fp,"%ld,"
					,myData->bData[0].misc.source[ch].adValue[type][0]);
			printf("%ld"
					,myData->bData[0].misc.source[ch].adValue[type][0]);
		}
	}
	printf("\n");
	fprintf(fp,"\n");
	fclose(fp);
}

void BoardState_Print(void)
{	
	int i;

	printf("\nBoardState bd(%d)\n", bd+1);

	printf("Ch_do\n");
	for(i=0; i < myData->mData.config.chPerBd; i++) {
		printf("%x ", myData->bData[bd].ch_do[i].ch_io);
	}
	printf("\n");

	printf("sumV(+) ");
	for(i=0; i < MAX_AD_COUNT; i++) {
		printf("%d ",(short int)myData->bData[bd].misc.source[0].adValue[0][i]);
		printf("%x ",(short int)myData->bData[bd].misc.source[0].adValue[0][i]);
	}
	printf("\n");

	printf("sumV(-) ");
	for(i=0; i < MAX_AD_COUNT; i++) {
	//	printf("%ld ", myData->bData[bd].misc.source[1].adValue[0][i]);
		printf("%x ",(short int) myData->bData[bd].misc.source[1].adValue[0][i]);
	}
	printf("\n");

	printf("sumV(Gnd) ");
	for(i=0; i < MAX_AD_COUNT; i++) {
		printf("%ld ", myData->bData[bd].misc.source[2].adValue[0][i]);
	}
	printf("\n");

	printf("sensSumV(+) ");
	for(i=0; i < MAX_FILTER_AD_COUNT; i++) {
		printf("%ld ", myData->bData[bd].misc.source[0].sensSumV[i]);
	}
	printf("\n");

	printf("sensSumV(-) ");
	for(i=0; i < MAX_FILTER_AD_COUNT; i++) {
		printf("%ld ", myData->bData[bd].misc.source[1].sensSumV[i]);
	}
	printf("\n");

	printf("sourceV ");
	for(i=0; i < 4; i++) {
		printf("%ld ", myData->bData[bd].misc.source[i].sourceV);
	}
	printf("\n");
	printf("\n");
/*
	printf("sumI(+) ");
	for(i=0; i < MAX_AD_COUNT; i++) {
		printf("%ld ", myData->bData[bd].misc.source[0].adValue[1][i]);
	}
	printf("\n");

	printf("sumI(-) ");
	for(i=0; i < MAX_AD_COUNT; i++) {
		printf("%ld ", myData->bData[bd].misc.source[1].adValue[1][i]);
	}
	printf("\n");

	printf("sumI(Gnd) ");
	for(i=0; i < MAX_AD_COUNT; i++) {
		printf("%ld ", myData->bData[bd].misc.source[2].adValue[1][i]);
	}
	printf("\n");

*/
	printf("Vsource_AD\n");
	for(i=0; i < myData->mData.config.installedBd; i++){
		printf("BD : %d=> A : %f, B : %f\n",i
		,myData->bData[i].misc.Vsource_AD_a
		,myData->bData[i].misc.Vsource_AD_b);
	}

	printf("Isource_AD\n");
	for(i=0; i < myData->mData.config.installedBd; i++){
		printf("BD : %d=> A : %f, B : %f\n",i
		,myData->bData[i].misc.Isource_AD_a
		,myData->bData[i].misc.Isource_AD_b);
	}

	for(i=0; i < MAX_AD_COUNT; i++) {
		if(myData->bData[bd].misc.source[0].adValue[1][i] < min[7])
			min[7] = myData->bData[bd].misc.source[0].adValue[1][i];
		if(myData->bData[bd].misc.source[0].adValue[1][i] > max[7])
			max[7] = myData->bData[bd].misc.source[0].adValue[1][i];
	}
	printf("min %ld, max %ld\n", min[7], max[7]);
/*
	printf("sumI(-) ");
	for(i=0; i < MAX_AD_COUNT; i++) {
		printf("%ld ", myData->bData[bd].misc.source[1].adValue[1][i]);
	}
	printf("\n");

	for(i=0; i < MAX_AD_COUNT; i++) {
		if(myData->bData[bd].misc.source[1].adValue[1][i] < min[8])
			min[8] = myData->bData[bd].misc.source[1].adValue[1][i];
		if(myData->bData[bd].misc.source[1].adValue[1][i] > max[8])
			max[8] = myData->bData[bd].misc.source[1].adValue[1][i];
	}
	printf("min %ld, max %ld\n", min[8], max[8]);

	printf("sensSumI(+) ");
	for(i=0; i < MAX_FILTER_AD_COUNT; i++) {
		printf("%ld ", myData->bData[bd].misc.source[0].sensSumI[i]);
	}
	printf("\n");

	printf("sensSumI(-) ");
	for(i=0; i < MAX_FILTER_AD_COUNT; i++) {
		printf("%ld ", myData->bData[bd].misc.source[1].sensSumI[i]);
	}
	printf("\n");

	printf("sourceI ");
	for(i=0; i < 4; i++) {
		printf("%ld ", myData->bData[bd].misc.source[i].sourceI);
	}
	printf("\n");
	printf("\n");
*/
	printf("calSourceV ");
	for(i=0; i < 4; i++) {
		printf("%ld ", myData->bData[bd].misc.source[i].calSourceV);
	}
	printf("\n");
/*
	printf("calSourceI ");
	for(i=0; i < 4; i++) {
		printf("%ld ", myData->bData[bd].misc.source[i].calSourceI);
	}
	*/
	printf("\n");
	printf("\n");
}

void ChannelState_Print(void)
{
	int i, j =0;

	if(mode == 0) {
		printf("\nChannelState bd(%d)\n", bd+1);
	} else {
		printf("\nChannelState bd(%c)\n", bd+'A');
	}
	if(0){
		printf("tmp state\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
				printf("%x ", myData->bData[bd].cData[i].misc.tmpState);
		}
		printf("\n");
	}
	if(1){
		printf("state\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			if(mode == 0)
				printf("%x ", myData->bData[bd].cData[i].op.state);
			else
				printf("%x ", myData->bData[bd].cData[i].opSave.state);
		}
		printf("\n");
	}
	if(1){
		printf("phase\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			if(mode == 0)
				printf("%d ", myData->bData[bd].cData[i].op.phase);
			else
				printf("%d ", myData->bData[bd].cData[i].opSave.phase);
		}
		printf("\n");
	}
	if(0){
		printf("parallel_cycle_phase\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%d ", myData->bData[bd].cData[i].misc.parallel_cycle_phase);
		}
		printf("\n");
	}
	if(0){
		printf("tmpPhase\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%d ", myData->bData[bd].cData[i].misc.tmpPhase);
		}
		printf("\n");
	}
	if(0){
		printf("userPatternCnt\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%d ", myData->bData[bd].cData[i].misc.userPatternCnt);
		}
		printf("\n");
	}
	if(1){
		printf("type\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			if(mode == 0)
				printf("%x ", myData->bData[bd].cData[i].op.type);
			else
				printf("%x ", myData->bData[bd].cData[i].opSave.type);
		}
		printf("\n");
	}
	if(0){
		printf("preType\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			if(mode == 0)
				printf("%x ", myData->bData[bd].cData[i].op.preType);
			else
				printf("%x ", myData->bData[bd].cData[i].opSave.preType);
		}
		printf("\n");
	}
	if(1){
		printf("mode\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			if(mode == 0) {
				printf("%d ", myData->bData[bd].cData[i].op.mode);
			} else
				printf("%d ", myData->bData[bd].cData[i].opSave.mode);
		}
		printf("\n");
	}
	
	if(1){
		printf("code\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			if(mode == 0) {
				printf("%d ", myData->bData[bd].cData[i].op.code);
			} else
				printf("%d ", myData->bData[bd].cData[i].opSave.code);
		}
		printf("\n");
	}
	if(0){
		printf("tmpCode\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%d ", myData->bData[bd].cData[i].misc.tmpCode);
		}
		printf("\n");
	}
	if(0){	
		printf("opsave code\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
				printf("%d ", myData->bData[bd].cData[i].opSave.code);
		}
		printf("\n");
	}
	if(0){	
		printf("signal next step\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%d ", myData->bData[bd].cData[i].signal[C_SIG_NEXTSTEP]);
		}
		printf("\n");
	}
	if(0){
		printf("Capacity\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].op.capacitance);
		}
		printf("\n");
	}
	if(1){
		printf("groupTemp\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.groupTemp);
		}
		printf("\n");
	}
	if(0){
		printf("chamberWaitFlag\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%d ", myData->bData[bd].cData[i].misc.chamberWaitFlag);
		}
		printf("\n");
	}
	if(0){
		printf("Cycle_P_Shchedule_Flag\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%d ", myData->bData[bd].cData[i].misc.cycle_p_sch_flag);
		}
		printf("\n");
	}
	if(0){
		printf("Pre_change_v\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.Pre_change_V);
		}
		printf("\n");
	}
	if(0){
		printf("signal limit error\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%d ", myData->bData[bd].cData[i].signal[C_SIG_LIMIT_ERROR]);
		}
		printf("\n");
	}
	if(0){
		printf("signal run\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%d ", myData->bData[bd].cData[i].signal[C_SIG_RUN]);
		}
		printf("\n");
	}
	if(0){
		printf("signal pause\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%d ", myData->bData[bd].cData[i].signal[C_SIG_PAUSE]);
		}
		printf("\n");
	}	
	if(0){
		printf("signal out switch\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%d ", myData->bData[bd].cData[i].signal[C_SIG_OUT_SWITCH]);
		}
		printf("\n");
	}	
	if(0){
		printf("signal range switch\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%d ", myData->bData[bd].cData[i].signal[C_SIG_RANGE_SWITCH]);
		}
		printf("\n");
	}
	if(0){
		printf("signal smps fault\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%d ", myData->bData[bd].cData[i].signal[C_SIG_SMPS_FAULT]);
		}
		printf("\n");
	}
	if(0){
		printf("chamberStepNo\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.chamberStepNo);
		}
		printf("\n");
	}
	if(0){
		printf("advStepNo\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.advStepNo);
		}
		printf("\n");
	}	
	if(1){
		printf("stepNo\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			if(mode == 0) {
				printf("%ld ", myData->bData[bd].cData[i].op.stepNo);
			} else
				printf("%ld ", myData->bData[bd].cData[i].opSave.stepNo);
		}
		printf("\n");
	}
	if(0){
		printf("stepCount\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.step_count);
		}
		printf("\n");
	}	
	if(0){
		printf("integralInit\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%d ", myData->bData[bd].cData[i].misc.integralInit);
		}
		printf("\n");
	}
	if(0){
		printf("gotoCycleCount\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%d ", myData->bData[bd].cData[i].misc.gotoCycleCount[7]);
		}
		printf("\n");
	}
	if(0){
		printf("gotoCycleCount_testCond3\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%d ",myData->mData.testCond[bd][i].step[2].gotoCycleCount);
		}
		printf("\n");
	}
	if(0){
		printf("gotoCycleCount_testCond6\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%d ",myData->mData.testCond[bd][i].step[5].gotoCycleCount);
		}
		printf("\n");
	}
	if(0){
		printf("grade\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			if(mode == 0) {
				printf("%d ", myData->bData[bd].cData[i].op.grade);
			} else
				printf("%d ", myData->bData[bd].cData[i].opSave.grade);
		}
		printf("\n");
	}	
	if(0){
		printf("temp\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			if(mode == 0) {
				printf("%ld ", myData->bData[bd].cData[i].op.temp);
			} else
				printf("%ld ", myData->bData[bd].cData[i].opSave.temp);
		}
		printf("\n");
	}	
	if(0){
		printf("temp1\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			if(mode == 0) {
				printf("%ld ", myData->bData[bd].cData[i].op.temp1);
			} else
				printf("%ld ", myData->bData[bd].cData[i].opSave.temp);
		}
		printf("\n");
	}	
	if(0){
		printf("tempDir\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%x ", myData->bData[bd].cData[i].misc.tempDir);
		}
		printf("\n");
	}
	if(0){
		printf("userPatternRunTime\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.userPatternRunTime);
		}
		printf("\n");
	}	
	if(0){
		printf("signal cp dcr flag\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%d ", myData->bData[bd].cData[i].signal[C_SIG_CP_DCR_FLAG]);
		}
		printf("\n");
	}	
	if(0){
		printf("signal v_range\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%d ", myData->bData[bd].cData[i].signal[C_SIG_V_RANGE]);
		}
		printf("\n");
	}
	if(0){
		printf("rangeV\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%d ", myData->bData[bd].cData[i].op.rangeV);
		}
		printf("\n");
	}
	if(0){
		printf("signal parallel switch\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%d ", myData->bData[bd].cData[i].signal[C_SIG_PARALLEL_SWITCH]);
		}
		printf("\n");
	}
	if(0){
		printf("signal i_range\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%d ", myData->bData[bd].cData[i].signal[C_SIG_I_RANGE]);			}
		printf("\n");
	}
	if(1){
		printf("rangeI\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%d ", myData->bData[bd].cData[i].op.rangeI);
		}
		printf("\n");
	}
	if(0){
		printf("signal cali_point\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%d ", myData->bData[bd].cData[i].signal[C_SIG_CALI_POINT]);
		}
		printf("\n");
	}
	if(0){
		printf("signal cali_phase\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%d ", myData->bData[bd].cData[i].signal[C_SIG_CALI_PHASE]);
		}
		printf("\n");
	}	
	if(0){
		printf("signal meter_reply\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%d ", myData->bData[bd].cData[i].signal[C_SIG_METER_REPLY]);
		}
		printf("\n");
	}	
	if(0){
		printf("sumV\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.adValue[0][0]);
		}
		printf("\n");
	}	
	if(0){
		printf("sensSumV\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.sensSumV[0]);
		}
		printf("\n");
	}
	if(0){
		printf("voltage (op.Vsens)\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			if(mode == 0)
				printf("%ld ", myData->bData[bd].cData[i].op.Vsens);
			else
				printf("%ld ", myData->bData[bd].cData[i].opSave.Vsens);
			if(!((i+1)%8))
				printf("\n");
		}
	}
	if(0){
		printf("\n");
		printf("ch1 write idx\n");
		printf("%d ", myData->save_msg[0].write_idx[0][0]);
		printf("\n");
		printf("ch1 read idx\n");
		printf("%d ", myData->save_msg[0].read_idx[0][0]);
		printf("\n");
		printf("10msDataCount\n");
		printf("%d ", myData->bData[bd].cData[0].misc.save10msDataCount);
		printf("\n");
	}
	if(0){
		printf("tmpVsens\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.tmpVsens);
			if(!((i+1)%8))
				printf("\n");
		}
	}
	if(1){
		printf("current (op.Isens)\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			if(mode == 0) {
				printf("%ld ", myData->bData[bd].cData[i].op.Isens);
			} else
				printf("%ld ", myData->bData[bd].cData[i].opSave.Isens);
			if(!((i+1)%8))
				printf("\n");
		}
	}
	if(0){
		printf("tmpIsens\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.tmpIsens);
			if(!((i+1)%8))
				printf("\n");
		}
	}	
	if(0){
		printf("meanCount\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.meanCount);
			if(!((i+1)%8))
				printf("\n");
		}
	}
	if(0){
		printf("AuxTStart Num\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%d ", myData->bData[bd].cData[i].misc.chAuxTCheckNum);
			if(!((i+1)%8))
				printf("\n");
		}
	}	
	if(0){
		printf("chData_AuxV\n");
		for(i=0; i < myData->mData.config.installedAuxV; i++) {
			printf("%ld ", myData->daq.op.chData_AuxV[i]);
			if(!((i+1)%8))
				printf("\n");
		}
	}
	if(0){
		printf("AuxVCheck Num\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%d ", myData->bData[bd].cData[i].misc.chAuxVCheckNum);
			if(!((i+1)%8))
				printf("\n");
		}
	}
	if(0){
		printf("AuxVCnt\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%d ", myData->bData[bd].cData[i].misc.chAuxVCnt);
			if(!((i+1)%8))
				printf("\n");
		}
	}
	if(0){
		printf("auxType\n");
		for(i=0; i < 32; i++) {
			printf("%d ", myData->auxSetData[i].auxType);
		}
		printf("\n");
	}	
	if(0){
		printf("ChNo =========== \n");
		for(i=0; i < 32; i++) {
			printf("%d ", myData->auxSetData[i].chNo);
	
		}
		printf("\n");
	}	
	if(0){
		printf("Ch1 Aux Data\n");
		for(i=0; i < MAX_CH_AUX_DATA; i++) {
			printf("%ld ", myData->bData[bd].cData[0].misc.chAuxVoltage[i]);
		}
		printf("\n");
	}
	if(0){
		printf("Ch2 Aux Data\n");
		for(i=0; i < MAX_CH_AUX_DATA; i++) {
			printf("%ld ", myData->bData[bd].cData[1].misc.chAuxVoltage[i]);
		}
		printf("\n");
	}	
	if(0){
		printf("Ch3 Aux Data\n");
		for(i=0; i < MAX_CH_AUX_DATA; i++) {
			printf("%ld ", myData->bData[bd].cData[2].misc.chAuxVoltage[i]);
		}
		printf("\n");
	}
	if(0){
		printf("tmpIsens2\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.tmpIsens2);
		}
		printf("\n");
	}
	if(0){
		printf("Isens2\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.Isens2);
		}
		printf("\n");
	}	
	if(0){
		printf("cmd_v\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
				printf("%ld ", myData->bData[bd].cData[i].misc.cmd_v);
		}
		printf("\n");
	}	
	if(0){
		printf("sensCount\n");
		for(i=0; i < myData->mData.config.chPerBd; i++){
			printf("%d ", myData->bData[bd].cData[i].misc.sensCount);
		}
		printf("\n");
	}	
	if(0){
		printf("sensCountFlag\n");
		for(i=0; i < myData->mData.config.chPerBd; i++){
			printf("%x ", myData->bData[bd].cData[i].misc.sensCountFlag);
		}
		printf("\n");
	}	
	if(0){
		printf("sensBufCount\n");
		for(i=0; i < myData->mData.config.chPerBd; i++){
			printf("%d ", myData->bData[bd].cData[i].misc.sensBufCount);
		}
		printf("\n");
	}
	if(0){
		printf("sensBufCountFlag\n");
		for(i=0; i < myData->mData.config.chPerBd; i++){
			printf("%x ", myData->bData[bd].cData[i].misc.sensBufCountFlag);
		}
		printf("\n");
	}	
	if(0){
		printf("parallel_sensFlag\n");
		for(i=0; i < myData->mData.config.chPerBd; i++){
			printf("%x ", myData->bData[bd].cData[i].misc.parallel_sensFlag);
		}
		printf("\n");
	}
	if(0){
		printf("saveDt\n");
		for(i=0; i < myData->mData.config.chPerBd; i++){
			printf("%ld ", myData->bData[bd].cData[i].misc.saveDt);
		}
		printf("\n");
	}
	if(0){
		printf("ChAttribute\n");
		for(i=0; i < myData->mData.config.chPerBd; i++){
			printf("ch %d: ", i+1);
			printf("master %d: ", myData->bData[bd].cData[i].ChAttribute.chNo_master);
			printf("slave %d: ", myData->bData[bd].cData[i].ChAttribute.chNo_slave[0]);
			printf("type %d: ", myData->bData[bd].cData[i].ChAttribute.opType);
			printf("\n");
		}
		printf("\n");
	}
	if(0){
		printf("sumI\n ");
		for(i=0; i < MAX_AD_COUNT; i++) {
			printf("%ld ", myData->bData[bd].cData[5].misc.adValue[1][i]);
		}
		printf("\n");
	}
	if(0){
		printf("adValue\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.adValue[1][0]);
		}
		printf("\n");
	}	
	if(0){
		printf("preVref\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.preVref);
		}
		printf("\n");
	}	
	if(0){
		printf("ChCompData\n");
		for(i=0; i < myData->mData.config.chPerBd; i++){
			printf("ch %d: ", i+1);
			printf("use %d: ", myData->bData[bd].cData[i].ChCompData.useFlag);
			printf("plus %ld: ", myData->bData[bd].cData[i].ChCompData.compPlus);
			printf("minus %ld: ", myData->bData[bd].cData[i].ChCompData.compMinus);
			printf("\n");
		}
		printf("\n");
	}	
	if(0){
		printf("preIref\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.preIref);
		}
		printf("\n");
	}
	if(0){
		printf("sensSumI Ch_1\n ");
		for(i=0; i < MAX_FILTER_AD_COUNT; i++) {
			printf("%ld ", myData->bData[bd].cData[0].misc.sensSumI[i]);
		}
		printf("\n");
	}
	if(0){
		printf("sensSumI Ch_2\n ");
		for(i=0; i < MAX_FILTER_AD_COUNT; i++) {
			printf("%ld ", myData->bData[bd].cData[1].misc.sensSumI[i]);
		}
		printf("\n");
	}
	if(0){
		printf("sensSumI\n ");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.sensSumI[0]);
		}
		printf("\n");
	}
	if(0){
		printf("integralcap charge\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.charge_integralCap[0]);
		}
		printf("\n");
	}	
	if(0){
		printf("integralcap discharge\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.discharge_integralCap[0]);
		}
		printf("\n");	
	}
	if(1){
		printf("cvRunTime\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.cvTime);
		}
		printf("\n");
	}
	if(1){
		printf("cvFlag\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%d ", myData->bData[bd].cData[i].misc.cvFlag);
		}
		printf("\n");
	}
	if(1){
		printf("cvFaultCheckFlag\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%d ", myData->bData[bd].cData[i].misc.cvFaultCheckFlag);
		}
		printf("\n");
	}	
	if(1){
		printf("ccRunTime\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.ccTime);
		}
		printf("\n");
	}
	if(0){
		if(myData->bData[bd].cData[0].op.Isens < min[8]) {
			min[8] = myData->bData[bd].cData[0].op.Isens;
			for(i=0; i < MAX_FILTER_AD_COUNT; i++) {
				min[i] = myData->bData[bd].cData[0].misc.sensSumI[i];
			}
		}
		printf("\n");	
		if(myData->bData[bd].cData[0].op.Isens > max[8]) {
			max[8] = myData->bData[bd].cData[0].op.Isens;
			for(i=0; i < MAX_FILTER_AD_COUNT; i++) {
				max[i] = myData->bData[bd].cData[0].misc.sensSumI[i];
			}
		}
		printf("\n");
	
		printf("min %ld : ", min[8]);
		for(i=0; i < MAX_FILTER_AD_COUNT; i++) {
			printf("%ld ", min[i]);	
		}
		printf("\n");
	
		printf("max %ld : ", max[8]);
		for(i=0; i < MAX_FILTER_AD_COUNT; i++) {
			printf("%ld ", max[i]);	
		}
		printf("\n");
	}	
	if(0){
		printf("cycleEndTime\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.endCycleTime);
		}
		printf("\n");
	}
	if(0){
		printf("cycleRunTime\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.cycleRunTime);
		}
		printf("\n");
	}	
	if(0){
		printf("CheckDelayTime\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].op.checkDelayTime);
		}
		printf("\n");
	}	
	if(0){
		printf("fbI\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.fbI);
		}
		printf("\n");
	}	
	if(1){
		printf("runTime\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			if(mode == 0) {
				printf("%ld ", myData->bData[bd].cData[i].op.runTime);
			} else
				printf("%ld ", myData->bData[bd].cData[i].opSave.runTime);
		}
		printf("\n");
	}	
	if(0){
		printf("totalRunTime\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			if(mode == 0) {
				printf("%ld ", myData->bData[bd].cData[i].op.totalRunTime);
			} else
				printf("%ld ", myData->bData[bd].cData[i].opSave.totalRunTime);
		}
		printf("\n");
	}	
	if(0){
		printf("userPatternCnt\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%d ", myData->bData[bd].cData[i].misc.userPatternCnt);
		}
		printf("\n");
	}
	if(0){
		printf("cycleSumChargeWatt\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.cycleSumChargeWatt);
		}
		printf("\n");
	}
	if(0){
		printf("cycleSumDischargeWatt\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.cycleSumDischargeWatt);
		}
		printf("\n");
	}
	if(0){
		printf("watt\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].op.watt);
		}
		printf("\n");
	}
	if(0){
		printf("tmpWatt\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%010ld ", myData->bData[bd].cData[i].misc.tmpWatt);
		}
		printf("\n");
	}
	if(0){
		printf("start soc\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.startSoc);
		}
		printf("\n");
	}
	if(0){
		printf("soc\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.soc);
		}
		printf("\n");
	}
	if(0){
		printf("ampareHour\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			if(mode == 0) {
				printf("%ld ", myData->bData[bd].cData[i].op.ampareHour);
			} else {
				printf("%ld ", myData->bData[bd].cData[i].opSave.ampareHour);
			}
			if(!((i+1)%8))
				printf("\n");
		}
	}
	if(0){
		printf("Charge ampareHour\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			if(mode == 0) {
				printf("%ld,%ld ", myData->bData[bd].cData[i].op.charge_ampareHour,
					myData->bData[bd].cData[i].misc.sumChargeAmpareHour);
			} else
				printf("%ld ", myData->bData[bd].cData[i].opSave.ampareHour);
			if(!((i+1)%8))
				printf("\n");
		}
	}
	if(0){
		printf("DisCharge ampareHour\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			if(mode == 0) {
				printf("%ld,%ld ", myData->bData[bd].cData[i].op.discharge_ampareHour,
					myData->bData[bd].cData[i].misc.sumDischargeAmpareHour);
			} else
				printf("%ld ", myData->bData[bd].cData[i].opSave.ampareHour);
			if(!((i+1)%8))
				printf("\n");
		}
	}
#if CHAMBER_TEMP_HUMIDITY == 1	
	if(0){
		printf("chamber_temp_humid_check\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%d ", myData->bData[bd].cData[i].misc.chamber_temp_humid_check);
		}
		printf("\n");
	}
	if(0){
		printf("cham_tmep_humid\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%d ", myData->bData[bd].cData[i].misc.chamber_temp_humid_check);
		}
		printf("\n");
	}
	if(0){
		printf("cham_check_time_new\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.cham_check_time_new);
		}
		printf("\n");
	}
	if(0){
		printf("cham_check_time_pre\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.cham_check_time_pre);
		}
		printf("\n");
	}
	if(0){
		printf("cham_check_time_flag\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%d ", myData->bData[bd].cData[i].misc.cham_check_time_flag);
		}
		printf("\n");
	}
	if(0){
		printf("cham_temp\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.groupTemp);
		}
		printf("\n");
	}
	if(0){
		printf("ch_temp\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].op.temp);
		}
		printf("\n");
	}
	if(0){
		printf("cham_humid\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.humi);
		}
		printf("\n");
	}
	if(0){
		printf("efficiency Ah[0]\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.efficiency_Ah[0]);
		}
		printf("\n");
	}
	if(0){
		printf("efficiency Ah[1]\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.efficiency_Ah[1]);
		}
		printf("\n");
	}
	if(0){
		printf("calc_retain_Ah\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%.1f ", myData->bData[bd].cData[i].misc.calc_retain_Ah);
		}
		printf("\n");
	}
	if(0){
		printf("loopStepNo\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.loopStepNo);
		}
		printf("\n");
	}
	if(0){
		printf("dischargeStepNo\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.dischargeStepNo);
		}
		printf("\n");
	}
	if(0){
		printf("chargeAccAh\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.chargeAccAh);
		}
		printf("\n");
	}
	if(0){
		printf("dischargeStepNo\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.dischargeAccAh);
		}
		printf("\n");
	}
	if(0){
		printf("faultEfficiencyAh\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.faultEfficiencyAh);
		}
		printf("\n");
	}
#endif
	if(0){
		printf("Charge CCCVAh\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld,%ld ", myData->bData[bd].cData[i].misc.chargeCCCVAh,
							myData->bData[bd].cData[i].misc.sumChargeCCCVAh);
		}
		printf("\n");
	}
	if(0){
		printf("DisCharge CCCVAh\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld,%ld ", myData->bData[bd].cData[i].misc.dischargeCCCVAh,
							myData->bData[bd].cData[i].misc.sumDischargeCCCVAh);
		}
		printf("\n");
	}
	if(0){
		printf("Charge CCAh\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.chargeCCAh);
		}
		printf("\n");
	}
	if(0){
	printf("Charge CVAh\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.chargeCVAh);
		}
		printf("\n");
	}		
	if(0){
		printf("Charge CCCVAh\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.chargeCCCVAh);
		}
		printf("\n");
	}
	if(0){
		printf("disCharge CCAh\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.dischargeCCAh);
		}
		printf("\n");
	}
	if(0){
		printf("disCharge CVAh\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.dischargeCVAh);
		}
		printf("\n");
	}
	if(0){
		printf("disCharge CCCVAh\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.dischargeCCCVAh);
		}
		printf("\n");
	}
	if(0){
		printf("wattHour\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			if(mode == 0) {
				printf("%ld,%ld ", myData->bData[bd].cData[i].op.wattHour,
					myData->bData[bd].cData[i].misc.sumWattHour);
			} else
				printf("%ld ", myData->bData[bd].cData[i].opSave.wattHour);
		}
		printf("\n");
	}
	if(0){
		printf("Charge wattHour\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			if(mode == 0) {
				printf("%ld,%ld ", myData->bData[bd].cData[i].op.charge_wattHour,
						myData->bData[bd].cData[i].misc.sumChargeWattHour);
			} else
				printf("%ld ", myData->bData[bd].cData[i].opSave.ampareHour);
		}
		printf("\n");
	}
	if(0){
		printf("DisCharge wattHour\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			if(mode == 0) {
				printf("%ld,%ld ", myData->bData[bd].cData[i].op.discharge_wattHour,
						myData->bData[bd].cData[i].misc.sumDischargeWattHour);
			} else
				printf("%ld ", myData->bData[bd].cData[i].opSave.ampareHour);
		}
		printf("\n");
	}
	if(0){
		printf("actualcapacity\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.actualCapacity);
		}
	}	
	if(0){
		printf("actualWattHour\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.actualWattHour);
		}
	}	
	if(0){
		printf("\n");
		printf("integralCapacity\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].op.integral_ampareHour);
		}
		printf("\n");
	}
	if(0){
		printf("integralWattHour\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].op.integral_WattHour);
		}
		printf("\n");
	}
	if(0){
		printf("integralWattHourFlag\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.endIntegralWhFlag);
		}
		printf("\n");
	}
	if(0){
		printf("integralCapacityFlag\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.endIntegralCFlag);
		}
		printf("\n");
	}
	if(0){
		printf("endIntegralC\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.endIntegralC);
		}
		printf("\n");
	}
	if(0){
		printf("endIntegralWh\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.endIntegralWh);
		}
		printf("\n");
	}
	if(0){
		printf("capacitiance\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			if(mode == 0) {
				printf("%ld ", myData->bData[bd].cData[i].op.capacitance);
			} else
				printf("%ld ", myData->bData[bd].cData[i].opSave.capacitance);
		}
		printf("\n");
	}
	if(0){
		printf("ccv\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].ccv[0].avg_v);
		}
		printf("\n");
	}
	if(0){
		printf("d_voltage\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.d_voltage);
		}
		printf("\n");
	}
	if(0){
		printf("op.z\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			if(mode == 0) {
				printf("%ld ", myData->bData[bd].cData[i].op.z);
			} else
				printf("%ld ", myData->bData[bd].cData[i].opSave.z);
		}
		printf("\n");
	}
	if(0){
		printf("pid_ui1\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%f ", myData->bData[bd].cData[i].misc.pid_ui1[1]);
		}
		printf("\n");
	}	
	if(0){
		printf("pid_error1\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%f ", myData->bData[bd].cData[i].misc.pid_error1[1]);
		}
		printf("\n");
	}
	if(0){
		printf("cycleNo\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%d ", myData->bData[bd].cData[i].misc.cycleNo);
		}
		printf("\n");
	}
	if(0){
		printf("totalCycle\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.totalCycle);
		}
		printf("\n");
	}
	if(0){
		printf("advCycle\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.advCycle);
		}
		printf("\n");
	}
	if(0){
		printf("advCycleStep\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.advCycleStep);
		}
		printf("\n");
	}
#ifdef _SDI_SAFETY_V2
	if(0){
		printf("MasterFlag\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%d ", myData->bData[bd].cData[i].misc.MasterFlag);
		}
		printf("\n");
	}
#endif
	if(0){
		printf("select\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%d ", myData->bData[bd].cData[i].op.select);
		}
		printf("\n");
	}
	if(0){
		printf("semiSwitch\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%d ", myData->bData[bd].cData[i].misc.semiSwitchState);
		}
		printf("\n");
	}
	if(0){
		printf("currentCycle\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.currentCycle);
		}
		printf("\n");
	}
	if(0){
		printf("reservedCmd\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%d ", myData->bData[bd].cData[i].op.reservedCmd);
		}
		printf("\n");
	}	
	if(0){
		printf("standardC_Flag\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%d ", myData->bData[bd].cData[i].misc.standardC_Flag);
		}
		printf("\n");
	}
	if(0){
		printf("standardP_Flag\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%d ", myData->bData[bd].cData[i].misc.standardP_Flag);
		}
		printf("\n");
	}	
	if(0){
		printf("standardZ_Flag\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%d ", myData->bData[bd].cData[i].misc.standardZ_Flag);
		}
		printf("\n");
	}
	if(0){
		printf("standardC\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.standardC);
		}
		printf("\n");
	}
	if(0){
		printf("standardP\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.standardP);
		}
		printf("\n");	
	}
	if(0){
		printf("standardZ\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.standardZ);
		}
		printf("\n");
	}
	if(0){
		printf("cycleSumC\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.cycleSumC);
		}
		printf("\n");
	}
	if(0){
		printf("cycleSumP\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.cycleSumP);
		}
		printf("\n");
	}
	if(0){
		printf("ahEndRatio\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%d ", myData->bData[bd].cData[i].misc.ahEndRatio[10]);
		}
		printf("\n");
	}
	if(0){
		printf("whEndRatio\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%d ", myData->bData[bd].cData[i].misc.whEndRatio[10]);
		}
		printf("\n");
	}
	if(0){
		printf("socCheckCount\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%d ", myData->bData[bd].cData[i].misc.socCheckCount);
		}
		printf("\n");
	}
	if(0){
		printf("endC countNo\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%d ", myData->bData[bd].cData[i].misc.socCountNo[0]);
		}
		printf("\n");
	}
	if(0){
		printf("endP countNo\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%d ", myData->bData[bd].cData[i].misc.socCountNo[1]);
		}
		printf("\n");
	}
	if(0){
		printf("endZ countNo\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%d ", myData->bData[bd].cData[i].misc.socCountNo[2]);
		}
		printf("\n");
	}
	if(0){
		printf("cycleEndC\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.cycleEndC);
		}
		printf("\n");
	}
	if(0){
		printf("endC_std_type\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%d ", myData->bData[bd].cData[i].misc.endC_std_type);
		}
		printf("\n");
	}
	if(0){
		printf("endP_std_type\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%d ", myData->bData[bd].cData[i].misc.endP_std_type);
		}
		printf("\n");
	}
	if(0){
		printf("endZ_std_type\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%d ", myData->bData[bd].cData[i].misc.endZ_std_type);
		}
		printf("\n");
	}
	if(0){
		printf("endC_std_sel\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%d ", myData->bData[bd].cData[i].misc.endC_std_sel);
		}
		printf("\n");
	}
	if(0){
		printf("endP_std_sel\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%d ", myData->bData[bd].cData[i].misc.endP_std_sel);
		}
		printf("\n");
	}
	if(0){
		printf("endZ_std_sel\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%d ", myData->bData[bd].cData[i].misc.endZ_std_sel);
		}
		printf("\n");
	}
	if(0){
		printf("chamberNo\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%d ", myData->bData[bd].cData[i].misc.chamberNo);
		}
		printf("\n");
	}
#if CYCLER_TYPE == 2
	if(1){	//210311
		printf("CAN1_Signal =============== \n");
		for(i=0; i < 8; i++) {
			printf("%d ", myData->mData.signal[M_SIG_CAN1_0 + i]);
		}
		printf("\n");
	}

	if(1){	//210311
		printf("CAN2_Signal =============== \n");
		for(i=0; i < 8; i++) {
			printf("%d ", myData->mData.signal[M_SIG_CAN2_0 + i]);
		}
		printf("\n");
	}
#endif

	if(0){
		printf("chGroupNo\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%d ", myData->bData[bd].cData[i].misc.chGroupNo);
		}
		printf("\n");
	}
	if(0){
		printf("stepSyncFlag\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%d ", myData->bData[bd].cData[i].misc.stepSyncFlag);
		}
		printf("\n");
	}
#ifdef _GROUP_ERROR
	if(1){
		printf("groupAvgVsens\n");
		//for(i=0; i < myData->mData.config.chPerBd; i++) {
		for(i=0; i < 10; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.groupAvgVsens);
		}
		printf("\n");
	}
	if(1){
		printf("Std_Time\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.Std_Time);
		}
		printf("\n");
	}
	if(1){
		printf("Fault_Check_flag\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%d ", myData->bData[bd].cData[i].misc.Fault_Check_flag);
		}
		printf("\n");
	}
	if(1){
		printf("endState\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.endState);
		}
		printf("\n");
	}
	if(1){
		printf("groupEndTime\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.groupEndTime);
		}
		printf("\n");
	}
#endif
	if(0){
		printf("userPatternFlag\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%d ", myData->bData[bd].cData[i].misc.userPatternFlag);
		}
		printf("\n");
	}
	if(0){
		printf("pause_flag\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%d ", myData->bData[bd].cData[i].misc.pause_flag);
		}
		printf("\n");
	}
	if(0){
		printf("cycleNo_pause\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%d ", myData->bData[bd].cData[i].misc.cycleNo_pause);
		}
		printf("\n");
	}
	if(0){
		printf("stepNo_pause\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%d ", myData->bData[bd].cData[i].misc.stepNo_pause);
		}
		printf("\n");
	}
	if(0){
		printf("efficiency_pause_flag\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%d ", myData->bData[bd].cData[i].misc.efficiency_pause_flag);
		}
		printf("\n");
	}
	if(0){
		printf("save10msDataCount\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%d ", myData->bData[bd].cData[i].misc.save10msDataCount);
		}
		printf("\n");
	}
	if(0){
		printf("save10msDt\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.save10msDt);
		}
		printf("\n");
	}
	if(0){
		printf("cRateUseFlag\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%d ", myData->bData[bd].cData[i].misc.cRateUseFlag);
		}
		printf("\n");
	}
	if(0){
		printf("pre_v_chk_time\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.pre_v_chk_time);
		}
		printf("\n");
	}	
	if(0){
		printf("pre_chk_v\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.pre_chk_v);
		}
		printf("\n");
	}	
	if(0){
	printf("misc.maxV\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.maxV);
		}
		printf("\n");
	}	
	if(0){
		printf("misc.minV\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.minV);
		}
		printf("\n");
	}	
	if(0){
		printf("misc.maxI\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.maxI);
		}
		printf("\n");
	}
	if(0){
		printf("misc.minI\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.minI);
		}
		printf("\n");
	}
#ifdef _ULSAN_SDI_SAFETY
	if(1){
		printf("cvTime_Ulsan\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.cvTime_Ulsan);
		}
		printf("\n");
	}
	if(1){
		printf("misc.humpComp_T\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.humpComp_T);
		}
		printf("\n");
	}
	if(1){
		printf("misc.humpComp_I\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.humpComp_I);
		}
		printf("\n");
	}
	if(1){
		printf("misc.humpCheck_T\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.humpCheck_T);
		}
		printf("\n");
	}
	if(1){
		printf("misc.humpCheck_I\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.humpCheck_I);
		}
		printf("\n");
	}
#endif
	if(0){
		printf("sel_Cyc_C_Cap\n");
		for(i=0; i < 5; i++) {
			printf("StepNo : %d Cap : %ld\n", i, myData->bData[bd].cData[0].misc.sel_Cyc_C_Cap[i]);
		}
		printf("\n");
	}
	if(0){
		printf("sel_Cyc_D_Cap\n");
		for(i=0; i < 5; i++) {
			printf("StepNo : %d Cap : %ld\n", i, myData->bData[bd].cData[0].misc.sel_Cyc_D_Cap[i]);
		}
		printf("\n");
	}
	if(0){
		printf("testCondUpdate\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.testCondUpdate);
		}
		printf("\n");
	}	
	
	if(0){
		printf("inv_power\n");
		for(i=0; i < myData->mData.config.installedCh; i++) {
			printf("%d ", myData->bData[bd].cData[i].inv_power);
		}
		printf("\n");
	}
	if(0){
		printf("INPUT ERROR\n");
		for(i=0; i < myData->mData.config.installedCh; i++) {
			printf("%d ", myData->bData[bd].cData[i].misc.errCnt[C_CNT_PCU_INPUT_FAULT]);
		}
		printf("\n");
	}		
	if(0){
		printf("INV errCnt\n");
		for(i=0; i < myData->mData.config.installedCh; i++) {
			if(i != 0 && i % 4 == 0) printf("\n");
			for(j =0 ; j < 4; j++){
				printf("%d ", myData->bData[bd].cData[i].misc.errCnt[C_CNT_PCU_INV_FLT1 + j]);
			}
			printf(" / ");
		}
		printf("\n");
	}	
	if(0){
		printf("INV errFlag\n");
		for(i=0; i < myData->mData.config.installedCh; i++) {
			if(i != 0 && i % 4 == 0) printf("\n");
			for(j =0 ; j < 4; j++){
				printf("%d ", myData->bData[bd].cData[i].misc.inv_errFlag[j]);
			}
			printf(" / ");
		}
		printf("\n");
	}
	if(0){
		printf("INV errCode\n");
		for(i=0; i < myData->mData.config.installedCh; i++) {
			if(i != 0 && i % 4 == 0) printf("\n");
			for(j =0 ; j < 4; j++){
				printf("%d ", myData->bData[bd].cData[i].misc.inv_errCode[j]);
			}
			printf(" / ");
		}
		printf("\n");
	}
	if(0){
		printf("ambientTemp\n");
		for(i=0; i < myData->mData.config.installedCh; i++) {
			if(i != 0 && i % 4 == 0) printf("\n");
			printf("%ld ", myData->bData[bd].cData[i].misc.ambientTemp);
		}
		printf("\n");
	}
	if(0){
		printf("gasVoltage\n");
		for(i=0; i < myData->mData.config.installedCh; i++) {
			if(i != 0 && i % 4 == 0) printf("\n");
			printf("%ld ", myData->bData[bd].cData[i].misc.gasVoltage);
		}
		printf("\n");
	}
	#if CH_AUX_DATA == 1 
	if(0){
		printf("chAuxTemp\n");
		for(i=0; i < myData->mData.config.installedCh; i++) {
			printf("ch[%d] %ld, %ld, %ld, %ld, %ld\n",
				i+1,myData->bData[0].cData[i].misc.chAuxTemp[0],
				myData->bData[0].cData[i].misc.chAuxTemp[1],
				myData->bData[0].cData[i].misc.chAuxTemp[2],
				myData->bData[0].cData[i].misc.chAuxTemp[3],
				myData->bData[0].cData[i].misc.chAuxTemp[4]);
		}
	}
	#endif
	#ifdef _TEMP_CALI 
	if(0){
		printf("setTempPoint\n");
		for(i=0; i < MAX_TEMP_POINT; i++) {
			if(i != 0 && i % 4 == 0) printf("\n");
			printf("%ld ", myData->temp_cali.point.setTempPoint[i]);
		}
		printf("\n");
	}
	if(0){
		printf("ANALOG_METER_SIG_CALI_NORMAL : %d\n",
			myData->AnalogMeter.temp_cali.signal[ANALOG_METER_SIG_CALI_NORMAL]);
		printf("caliFlagCount : %d\n",
			myData->temp_cali.point.caliFlagCount);
		printf("gain, offset\n");
		for(i = 140 ; i < 150 ; i++){
			if(i != 0 && i % 4 == 0) printf("\n");
			printf("[%f, %f][%f, %f] ", 
				myData->temp_cali.measure.gain[0][i],
				myData->temp_cali.measure.offset[0][i],
				myData->temp_cali.measure.gain[1][i],
				myData->temp_cali.measure.offset[1][i]);
		}
	}

	#endif
	if(0){
		printf("charge_cc_hump_flag\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.charge_cc_hump_flag);
		}
		printf("\n");
	}
	if(0){
		printf("charge_cc_hump_start_time\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.charge_cc_hump_start_time);
		}
		printf("\n");
	}
	if(0){
		printf("charge_cc_hump_start_voltage\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.charge_cc_hump_start_voltage);
		}
		printf("\n");
	}
	if(0){
		printf("charge_cv_hump_flag\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.charge_cv_hump_flag);
		}
		printf("\n");
	}
	if(0){
		printf("charge_cv_hump_start_time\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.charge_cv_hump_start_time);
		}
		printf("\n");
	}
	if(0){
		printf("charge_cv_hump_start_current\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.charge_cv_hump_start_current);
		}
		printf("\n");
	}
	if(0){
		printf("discharge_cc_hump_flag\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.discharge_cc_hump_flag);
		}
		printf("\n");
	}
	if(0){
		printf("discharge_cc_hump_start_time\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.discharge_cc_hump_start_time);
		}
		printf("\n");
	}
	if(0){
		printf("discharge_cc_hump_start_voltage\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.discharge_cc_hump_start_voltage);
		}
		printf("\n");
	}
	if(0){
		printf("discharge_cv_hump_flag\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.discharge_cv_hump_flag);
		}
		printf("\n");
	}
	if(0){
		printf("discharge_cv_hump_start_time\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.discharge_cv_hump_start_time);
		}
		printf("\n");
	}
	if(0){
		printf("discharge_cv_hump_start_current\n");
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.discharge_cv_hump_start_current);
		}
		printf("\n");
	}
#if CYCLER_TYPE == CAN_CYC
	if(1){
		printf("M_SIG_INV_POWER_CAN\n");
			printf("%d ", myData->mData.signal[M_SIG_INV_POWER_CAN]);
		printf("\n");
	}
	if(1){
		printf("M_SIG_RUN_LED\n");
			printf("%d ", myData->mData.signal[M_SIG_RUN_LED]);
		printf("\n");
	}

	if(1){
		printf("M_SIG_POWER_OFF\n");
			printf("%d ", myData->mData.signal[M_SIG_POWER_OFF]);
		printf("\n");
	}
	if(1){
		printf("C_CNT_INV_FAULT_CAN\n");
		for(i=0; i < 64; i++) {
			printf("%d ", myData->bData[bd].cData[i].misc.errCnt[C_CNT_INV_FAULT_CAN]);
		}
		printf("\n");
	}

#endif

	if(myData->AppControl.config.debugType != P0) {
		if(0){
			printf("simVsens\n");
			for(i=0; i < myData->mData.config.chPerBd; i++) {
				printf("%ld ", myData->bData[bd].cData[i].misc.simVsens);
			}
			printf("\n");
		}
		if(0){
			printf("simIsens\n");
			for(i=0; i < myData->mData.config.chPerBd; i++) {
				printf("%ld ", myData->bData[bd].cData[i].misc.simIsens);
			}
			printf("\n");
		}
		if(0){
			printf("testRefV\n");
			for(i=0; i < myData->mData.config.chPerBd; i++) {
				printf("%ld ", myData->bData[bd].cData[i].misc.testRefV);
			}
			printf("\n");
		}
		if(0){
			printf("testRefI\n");
			for(i=0; i < myData->mData.config.chPerBd; i++) {
				printf("%ld ", myData->bData[bd].cData[i].misc.testRefI);
			}
			printf("\n");
		}
	}
	printf("\n");
}

void DIO_Print(void)
{
	int i;

	printf("\nDIO_Print\n");

	printf("delayTimer %ld\n", myData->dio.delayTimer);
	printf("powerSwtichTimeout %ld\n", myData->dio.config.powerSwitchTimeout);
	printf("powerSwtichTimer %ld\n", myData->dio.powerSwitchTimer);
	printf("DIO_SIG_POWER_SWITCH %d\n", myData->dio.signal[DIO_SIG_POWER_SWITCH]);
	printf("outSignal %d\n", myData->dio.dout.outSignal[O_SIG_REMOTE_SMPS1]);
	printf("outFlag %d\n", myData->dio.dout.outFlag[I_OUT_REMOTE_SMPS1]);

	printf("DIN\n");
	for(i = 0; i < 32; i++){
		printf("%x, ",myData->dio.din.inFlag[i]);
	}
	printf("\n");
	for(i = 16; i < 22; i++){
		printf("outFlag %d\n", myData->dio.dout.outFlag[i]);
	}
	printf("\n");
	for(i = 12; i < 20; i++){
		printf("dio sig %d\n", myData->dio.signal[i]);
	}
	printf("\n");
}

void TestCond_Print(void)
{
	int stepNo, monitor_ch, board, channel, startStepNo, endStepNo, i;

	printf("\nTestCond_Print\n");
	printf("Ch No[1BASE] : ");
	scanf("%d", &monitor_ch);
	if((monitor_ch <= 0)
		|| monitor_ch > myData->mData.config.installedCh){
		puts("Invalid value.");
		return;
	}
	printf("startStepNo[1BASE] : ");
	scanf("%d", &startStepNo);
	if(startStepNo <= 0){
		puts("Invalid value.");
		return;
	}
	printf("endStepNo[1BASE] : ");
	scanf("%d", &endStepNo);
	if(endStepNo <= 0){
		puts("Invalid value.");
		return;
	}
	if(endStepNo < startStepNo){
		puts("Invalid value.");
		return;
	}
	board = myData->CellArray1[monitor_ch-1].bd;
	channel = myData->CellArray1[monitor_ch-1].ch;
/*	
	printf("userPattern \n");

	for(i =0; i < 50; i++){
	printf("%ld\n",
		myData->mData.testCond[board][channel].userPattern.data[i].data);
	}
	for(stepNo=0; stepNo < MAX_STEP; stepNo++){
		printf("stepNo : %ld\n"
			,myData->mData.testCond[board][channel].step[stepNo].stepNo);
		printf("socStepCap %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].socStepCap);
		printf("socStepPower %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].socStepPower);
		printf("socStepZ %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].socStepZ);
		printf("\n");
	}
*/

	printf("\ntotalStepNo : %d\n"
		,myData->mData.testCond[board][channel].header.totalStep);

	for(stepNo=(startStepNo-1); stepNo < endStepNo; stepNo++) {
		printf("\n");
		printf("===============================================================================\n\n");
		printf("stepNo : %ld\n"
			,myData->mData.testCond[board][channel].step[stepNo].stepNo);
		printf("type %x\n",
			myData->mData.testCond[board][channel].step[stepNo].type);
		printf("mode %x\n",
			myData->mData.testCond[board][channel].step[stepNo].mode);
		printf("refV %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].refV);
		printf("refI %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].refI);
		printf("refV_H %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].refV_H);
		printf("refV_L %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].refV_L);
		printf("refI_H %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].refI_H);
		printf("refI_L %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].refI_L);
		printf("refTemp %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].refTemp);
		printf("refP %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].refP);
		printf("refR %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].refR);
		printf("endV %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].endV);
		printf("endV goto %d\n",
			myData->mData.testCond[board][channel].step[stepNo].endVGoto);
		printf("endV_L %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].endV_L);
		printf("endI %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].endI);
		printf("endI goto %d\n",
			myData->mData.testCond[board][channel].step[stepNo].endIGoto);
		printf("endT %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].endT);
		printf("endT goto %d\n",
			myData->mData.testCond[board][channel].step[stepNo].endTGoto);
		printf("endC %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].endC);
		printf("endC goto %d\n",
			myData->mData.testCond[board][channel].step[stepNo].endCGoto);
		printf("endT goto %d\n",
			myData->mData.testCond[board][channel].step[stepNo].endTGoto);
		printf("end cv time %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].endT_CV);
		printf("end cv time goto %d\n",
			myData->mData.testCond[board][channel].step[stepNo].endTCVGoto);
		printf("endIntegralC goto %d\n",
			myData->mData.testCond[board][channel].step[stepNo]
				.endIntegralCGoto);
		printf("endIntegralWh goto %d\n",
			myData->mData.testCond[board][channel].step[stepNo]
				.endIntegralWhGoto);
		printf("endWh %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].endWh);
		printf("endP %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].endP);
		printf("endDeltaV %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].endDeltaV);
		printf("temp Type %d\n",
			myData->mData.testCond[board][channel].step[stepNo].tempType);
		printf("temp Dir %d\n",
			myData->mData.testCond[board][channel].step[stepNo].tempDir);
		printf("endTemp %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].endTemp);
		printf("socUseFlag %d\n",
			myData->mData.testCond[board][channel].step[stepNo].useSocFlag);
		printf("SocSoeFlag %d\n",
			myData->mData.testCond[board][channel].step[stepNo].SocSoeFlag);
		printf("socStepCap %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].socStepCap);
		printf("socCapStepNo %d\n",
			myData->mData.testCond[board][channel].step[stepNo].socCapStepNo);
		printf("endSoc %d\n",
			myData->mData.testCond[board][channel].step[stepNo].endSoc);
		printf("endSocGoto %d\n",
			myData->mData.testCond[board][channel].step[stepNo].endSocGoto);
		printf("gotoCycleCount %d\n",
			myData->mData.testCond[board][channel].step[stepNo].gotoCycleCount);
		printf("advCycleCount %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].advCycleCount);
		printf("capa_v1 %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].capacitance_v1);
		printf("capa_v2 %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].capacitance_v2);
		printf("endZeroVoltageFlag %d\n",
			myData->mData.testCond[board][channel].step[stepNo].endZeroVoltageFlag);
		//170501 oys add Cycle Efficiency End
		printf("stepNo_pause %d\n",
			myData->mData.testCond[board][channel].step[stepNo].stepNo_pause);
		printf("cycleNo_pause %d\n",
			myData->mData.testCond[board][channel].step[stepNo].cycleNo_pause);
		printf("endC_soc %d\n",
			myData->mData.testCond[board][channel].step[stepNo].endSoc);
		printf("endP_soc %d\n",
			myData->mData.testCond[board][channel].step[stepNo].endP_soc);
		printf("endZ_soc %d\n",
			myData->mData.testCond[board][channel].step[stepNo].endZ_soc);
		printf("endC_std_sel %d\n",
			(int)myData->mData.testCond[board][channel].step[stepNo].endCGoto);
		printf("endP_std_sel %d\n",
			myData->mData.testCond[board][channel].step[stepNo].endP_std_sel);
		printf("endZ_std_sel %d\n",
			myData->mData.testCond[board][channel].step[stepNo].endZ_std_sel);
		printf("endC_std_cycleCount %d\n",
			(int)myData->mData.testCond[board][channel].step[stepNo].endC_std_cycleCount);
		printf("endP_std_cycleCount %d\n",
			myData->mData.testCond[board][channel].step[stepNo].endP_std_cycleCount);
		printf("endZ_std_cycleCount %d\n",
			myData->mData.testCond[board][channel].step[stepNo].endZ_std_cycleCount);
		printf("endC_std_type %d\n",
			myData->mData.testCond[board][channel].step[stepNo].socCapStepNo);
		printf("endP_std_type %d\n",
			myData->mData.testCond[board][channel].step[stepNo].endP_std_type);
		printf("endZ_std_type %d\n",
			myData->mData.testCond[board][channel].step[stepNo].endZ_std_type);
		printf("endC_proc_type %d\n",
			myData->mData.testCond[board][channel].step[stepNo].endC_proc_type);
		printf("endP_proc_type %d\n",
			myData->mData.testCond[board][channel].step[stepNo].endP_proc_type);
		printf("endZ_proc_type %d\n",
			myData->mData.testCond[board][channel].step[stepNo].endZ_proc_type);
		printf("reduce_ratio_C %d\n",
			(int)myData->mData.testCond[board][channel].step[stepNo].advCycleCount);
		printf("reduce_ratio_P %d\n",
			myData->mData.testCond[board][channel].step[stepNo].reduce_ratio_P);
		printf("noTempWaitFlag %d\n",
			(int)myData->mData.testCond[board][channel].step[stepNo].noTempWaitFlag);
		printf("socStepPower %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].socStepPower);
		printf("socStepZ %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].socStepZ);
		//add end

		printf("userMap mode %x\n",
			myData->mData.testCond[board][channel].userMap.mode);
		printf("rangeV %d\n",
			myData->mData.testCond[board][channel].step[stepNo].rangeV);
		printf("rangeI %d\n",
			myData->mData.testCond[board][channel].step[stepNo].rangeI);
		printf("safetyUpperV %ld\n",
			myData->mData.testCond[board][channel].safety.faultUpperV);
		printf("safetyLowerV %ld\n",
			myData->mData.testCond[board][channel].safety.faultLowerV);
		printf("safetyUpperI %ld\n",
			myData->mData.testCond[board][channel].safety.faultUpperI);
		printf("safetyLowerI %ld\n",
			myData->mData.testCond[board][channel].safety.faultLowerI);
		printf("safetyUpperTemp %ld\n",
			myData->mData.testCond[board][channel].safety.faultUpperTemp);
		printf("safetyLowerTemp %ld\n",
			myData->mData.testCond[board][channel].safety.faultLowerTemp);
		printf("safetyUpperCapacity %ld\n",
			myData->mData.testCond[board][channel].safety.faultUpperC);
		printf("safetyLowerCapacity %ld\n",
			myData->mData.testCond[board][channel].safety.faultLowerC);
		printf("changeV_Dv %d\n",
			myData->mData.testCond[board][channel].safety.changeV_Dv);
		printf("changeV_Dt %d\n",
			myData->mData.testCond[board][channel].safety.changeV_Dt);
		printf("deltaV_Dv %d\n",
			myData->mData.testCond[board][channel].safety.deltaV_Dv);
		printf("deltaV_Dt %d\n",
			myData->mData.testCond[board][channel].safety.deltaV_Dt);
		#if CHAMBER_TEMP_HUMIDITY == 1
		printf("capacityEfficiency %ld\n",
			myData->mData.testCond[board][channel].safety.capacityEfficiency);
		printf("capacityRetain %ld\n",
			myData->mData.testCond[board][channel].safety.capacityRetain);
		#endif
		#if CHANGE_VI_CHECK == 1
		printf("faultRunTime %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].faultRunTime);
		#endif
		#ifdef _SDI_SAFETY_V1
		printf("fault_deltaDv %ld\n",
			myData->mData.testCond[board][channel].safety.fault_deltaDv);
		printf("faultRunTime %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].faultRunTime);
		#endif
		printf("StepSafetyUpperV %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].faultUpperV);
		printf("StepSafetyLowerV %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].faultLowerV);
		printf("StepSafetyUpperI %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].faultUpperI);
		printf("StepSafetyLowerI %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].faultLowerI);
		printf("StepSafetyUpperC %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].faultUpperC);
		printf("StepSafetyLowerC %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].faultLowerC);
		printf("StepSafetyUpperZ %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].faultUpperZ);
		printf("StepSafetyLowerZ %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].faultLowerZ);
		printf("StepSafetyUpperTemp %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].faultUpperTemp);
		printf("StepSafetyLowerTemp %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].faultLowerTemp);
		printf("StepSafetyPauseUpperTemp %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].pauseUpperTemp);
		printf("StepSafetyPauseLowerTemp %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].pauseLowerTemp);
/*		printf("StepSafetyUpperTemp_restart %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].faultUpperTemp_restart);
		printf("startTemp %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].startTemp);
		printf("endTemp %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].endTemp);
*/
		printf("saveDt %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].saveDt);
		printf("saveDv %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].saveDv);
		printf("saveDi %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].saveDi);
		printf("saveDtemp %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].saveDtemp);
		printf("\n");
		printf("DeltaV_Time %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].faultDeltaV_T);
		printf("DeltaI_Time %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].faultDeltaI_T);	
		printf("advGotoStep %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].advGotoStep);
		printf("\n");
#ifdef _SDI_SAFETY_V2
		printf("Master_Recipe_V %ld\n",
			myData->mData.testCond[board][channel].safety.Master_Recipe_V);
		printf("\n");
#endif
		printf("chamber_dev %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].chamber_dev);
		printf("\n");
#if HYUNADE == 1		
		printf("change_V_lower %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].change_V_lower);
		printf("change_V_upper %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].change_V_upper);
		printf("change_V_Time %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].change_V_time);
		printf("change_V %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].change_V);
		printf("chk_V_lower_1 %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].chk_V_lower[0]);
		printf("chk_V_upper_1 %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].chk_V_upper[0]);
		printf("chk_V_Time_1 %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].chk_V_time[0]);

		printf("chk_V_lower_2 %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].chk_V_lower[1]);
		printf("chk_V_upper_2 %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].chk_V_upper[1]);
		printf("chk_V_Time_2 %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].chk_V_time[1]);

		printf("chk_V_lower_3 %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].chk_V_lower[2]);
		printf("chk_V_upper_3 %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].chk_V_upper[2]);
		printf("chk_V_Time_3 %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].chk_V_time[2]);

		printf("chk_I_lower_1 %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].chk_I_lower[0]);
		printf("chk_I_upper_1 %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].chk_I_upper[0]);
		printf("chk_I_Time_1 %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].chk_I_time[0]);

		printf("chk_I_lower_2 %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].chk_I_lower[1]);
		printf("chk_I_upper_2 %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].chk_I_upper[1]);
		printf("chk_I_Time_2 %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].chk_I_time[1]);

		printf("chk_I_lower_3 %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].chk_I_lower[2]);
		printf("chk_I_upper_3 %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].chk_I_upper[2]);
		printf("chk_I_Time_3 %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].chk_I_time[2]);
#endif
#if END_V_COMPARE_GOTO == 1
		printf("endVGoto_upper %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].endVGoto_upper);
		printf("endVGoto_lower %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].endVGoto_lower);
		printf("endVupper_GotoStep %d\n",
			myData->mData.testCond[board][channel].step[stepNo].endVupper_GotoStep);
		printf("endVupper_GotoStep %d\n",
			myData->mData.testCond[board][channel].step[stepNo].endVlower_GotoStep);
#endif

#if CHAMBER_TEMP_HUMIDITY == 1
		printf("cham_sync_T %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].cham_sync_T);
		printf("cham_temp %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].cham_temp);
		printf("cham_humid %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].cham_humid);
		printf("ch_temp %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].ch_temp);
		printf("cham_temp_dev %d\n",
			myData->mData.testCond[board][channel].step[stepNo].cham_temp_dev);
		printf("cham_humid_dev %d\n",
			myData->mData.testCond[board][channel].step[stepNo].cham_humid_dev);
		printf("ch_temp_dev %d\n",
			myData->mData.testCond[board][channel].step[stepNo].ch_temp_dev);
		printf("cham_temp_sig %d\n",
			myData->mData.testCond[board][channel].step[stepNo].cham_temp_sig);
		printf("cham_humid_sig %d\n",
			myData->mData.testCond[board][channel].step[stepNo].cham_humid_sig);
		printf("ch_temp_sig %d\n",
			myData->mData.testCond[board][channel].step[stepNo].ch_temp_sig);
		printf("advGotoStep %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].advGotoStep);

#endif
/*
		printf("z_t1 %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].z_t1);
		printf("z_t2 %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].z_t2);
		printf("\n");
		printf("advGotoStep %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].advGotoStep);
		printf("\n");
	*/
// SK Grading //gradeStep = index
/*
		printf("Grade item1 %d\n",
			myData->mData.testCond[board][channel].step[stepNo].grade[0].gradeStep[0].item);
		printf("Grade item1 LowerValue %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].grade[0].gradeStep[0].lowerValue);
		printf("Grade item1 UpperValue %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].grade[0].gradeStep[0].upperValue);
		printf("Grade item2 %d\n",
			myData->mData.testCond[board][channel].step[stepNo].grade[1].gradeStep[0].item);
		printf("Grade item2 LowerValue %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].grade[1].gradeStep[0].lowerValue);
		printf("Grade item2 UpperValue %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].grade[1].gradeStep[0].upperValue);
		printf("GradeProcess %d\n",
			myData->mData.testCond[board][channel].step[stepNo].grade[1].gradeStep[0].gradeProc);
		printf("GradeCode %c\n",
			myData->mData.testCond[board][channel].step[stepNo].grade[0].gradeStep[0].gradeCode);
		printf("GradeCondition -> AND:26/ OR:2b \n%x\n",
			myData->mData.testCond[board][channel].step[stepNo].grade[1].gradeStep[0].gradeCode);
*/
#ifdef _TRACKING_MODE
		printf("soc_tracking_Flag %d\n",
			myData->mData.testCond[board][channel].step[stepNo].SOC_Tracking_flag);	
		printf("rptSOC %d\n",
			myData->mData.testCond[board][channel].step[stepNo].rptSOC);	
		printf("User_SOC %ld\n",
			myData->mData.testCond[board][channel].safety.soc);	
		printf("User_rptSOC %ld\n",
			myData->mData.testCond[board][channel].safety.rptsoc);	
#endif
#if GAS_DATA_CONTROL == 1
		printf("endGasTVOC %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].endGasTVOC);	
		printf("endGasTVOC_Goto %d\n",
			myData->mData.testCond[board][channel].step[stepNo].endGasTVOC_Goto);	
		printf("endGasECo2 %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].endGasECo2);	
		printf("endGasECoe_Goto %d\n",
			myData->mData.testCond[board][channel].step[stepNo].endGasECo2_Goto);	
		
		printf("StepSafetyUpper_Gas_TVOC %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].faultUpper_GasTVOC);
		printf("StepSafetyLower_Gas_TVOC %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].faultLower_GasTVOC);
		printf("StepSafetyUpper_Gas_ECo2 %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].faultUpper_GasECo2);
		printf("StepSafetyLower_Gas_ECo2 %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].faultLower_GasECo2);
		
#endif
#ifdef _ULSAN_SDI_SAFETY
		printf("humpSet_T %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].humpSet_T);
		printf("humpSet_I %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].humpSet_I);
#endif
#ifdef _GROUP_ERROR
		printf("group_StartVoltage %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].group_StartVoltage);
		printf("group_CheckTime %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].group_CheckTime);
		printf("group_DeltaVoltage %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].group_DeltaVoltage);
		printf("group_EndFaultTime %ld\n",
			myData->mData.testCond[board][channel].step[stepNo].group_EndFaultTime);
#endif
#ifdef _EQUATION_CURRENT
		printf("3rd flag %d\n",
			myData->mData.testCond[board][channel].step[stepNo].equation_current_flag);
		for(i=0; i<4; i++){
		printf("3rd variable%d %ld\n", i, 
			myData->mData.testCond[board][channel].step[stepNo].variable[i]);
		}
#endif

	}
	printf("reserved_cmd:%d, reserved_stepNo:%ld, reserved_cycleNo:%ld\n",
		(int)myData->mData.testCond[board][channel].reserved.reserved_cmd,
		myData->mData.testCond[board][channel].reserved.reserved_stepNo,
		myData->mData.testCond[board][channel].reserved.reserved_cycleNo);
	printf("select_run:%d, select_stepNo:%ld, select_cycleNo:%ld, select_advCycleStep:%ld\n",
		(int)myData->mData.testCond[board][channel].reserved.select_run,
		myData->mData.testCond[board][channel].reserved.select_stepNo,
		myData->mData.testCond[board][channel].reserved.select_cycleNo,
		myData->mData.testCond[board][channel].reserved.select_advCycleStep);
	
	printf("\n");

}

void CaliData_Print(void)
{
	int type,i, ch, rangeI, print;

	printf("\nCaliData_Print\n");

	type = 1;
	rangeI = 0;
	ch = 0;  //0BASE	

	print = 0;
	if(print){
		for(i=0; i < MAX_CALI_POINT-1; i++) {
			printf("%d tmp DA_A:%f, DA_B:%f\n", i,
				myData->cali.tmpData[bd][ch].DA_A[type][rangeI][i],
				myData->cali.tmpData[bd][ch].DA_B[type][rangeI][i]);
			printf("%d DA_A:%f, DA_B:%f\n", i,
				myData->cali.data[bd][ch].DA_A[type][rangeI][i],
				myData->cali.data[bd][ch].DA_B[type][rangeI][i]);
		}
		printf("\n");
	}
	print = 0;
	if(print){
		for(i=0; i < MAX_CALI_POINT-1; i++) {
			printf("%d tmp AD_A:%f, AD_B:%f\n", i,
				myData->cali.tmpData[bd][ch].AD_A[type][rangeI][i],
				myData->cali.tmpData[bd][ch].AD_B[type][rangeI][i]);
			printf("%d AD_A:%f, AD_B:%f\n", i,
				myData->cali.data[bd][ch].AD_A[type][rangeI][i],
				myData->cali.data[bd][ch].AD_B[type][rangeI][i]);
		}
		printf("\n");
	}
	
	print = 0;
	if(print){
		printf("charge_pointNum: %d\n",
			myData->cali.data[bd][ch].point[type][rangeI].charge_pointNum);
		printf("discharge_pointNum: %d\n",
			myData->cali.data[bd][ch].point[type][rangeI].discharge_pointNum);
	}

	print = 0;
	if(print){
		for(i=0; i < 2; i++) {
			printf("set_ad: %f\n",
				myData->cali.tmpData[0][0].set_ad[type][rangeI][i]);
		}
	}

	print = 0;
	if(print){
		for(i=0; i < 2; i++) {
			printf("set_ad cali meter2: %f\n",
				myData->cali.tmpData_caliMeter2[bd][ch].set_ad[type][rangeI][i]);
		}
	}
	print = 0;
	if(print){
		for(i = 0; i < 3 ; i++){
			printf("Range = %d \n", i+1);
			printf("tmp set_ad cali Meter2 : %f \n",
			myData->cali.tmpData_caliMeter2[bd][ch].set_ad[1][i][0]);
			printf("tmp check_ad cali Meter2 : %f \n",
			myData->cali.tmpData_caliMeter2[bd][ch].check_ad[1][i][0]);
			printf("tmp set_meter cali Meter2 : %f \n",
			myData->cali.tmpData_caliMeter2[bd][ch].set_meter[1][i][0]);
			printf("tmp check_meter cali Meter2 : %f \n",
			myData->cali.tmpData_caliMeter2[bd][ch].check_meter[1][i][0]);
			printf("tmp AD_A cali Meter2 : %f \n",
			myData->cali.tmpData_caliMeter2[bd][ch].AD_A[1][i][0]);
			printf("tmp AD_B cali Meter2 : %f \n",
			myData->cali.tmpData_caliMeter2[bd][ch].AD_B[1][i][0]);
		}
	}

	print = 0;
	if(print){
		for(i = 0; i < 3 ; i++){
			printf("Range = %d \n", i+1);
			printf("set_ad cali Meter2 : %f \n",
			myData->cali.data_caliMeter2[bd][ch].set_ad[1][i][0]);
			printf("check_ad cali Meter2 : %f \n",
			myData->cali.data_caliMeter2[bd][ch].check_ad[1][i][0]);
			printf("set_meter cali Meter2 : %f \n",
			myData->cali.data_caliMeter2[bd][ch].set_meter[1][i][0]);
			printf("check_meter cali Meter2 : %f \n",
			myData->cali.data_caliMeter2[bd][ch].check_meter[1][i][0]);
			printf("AD_A cali Meter2 : %f \n",
			myData->cali.data_caliMeter2[bd][ch].AD_A[1][i][0]);
			printf("AD_B cali Meter2 : %f \n",
			myData->cali.data_caliMeter2[bd][ch].AD_B[1][i][0]);
		}
	}

	
	print = 0;
	if(print){
		for(i=0; i < 2; i++) {
			printf("set_meter cali meter2: %f\n",
				myData->cali.tmpData_caliMeter2[bd][ch].set_meter[type][rangeI][i]);
		}
	}

	print = 1;
	if(print){
	printf("CALI tmp caliPoint ");
	for(i=0; i < 15; i++) {
		printf("%ld ", myData->cali.tmpCond[0][ch].point[type][rangeI].setPoint[i]);
	}
	printf("\n");
	}
	print = 1;
	if(print){
	printf("tmp caliCheckPoint ");
	for(i=0; i < 15; i++) {
		printf("%ld ", myData->cali.tmpCond[0][ch].point[type][rangeI].checkPoint[i]);
	}
	printf("\n");
	}


	print = 0;
	if(print){
	printf("tmp caliCheck AD_Point ");
	for(i=0; i < 10; i++) {
		printf("%f", myData->cali.tmpData[0][ch].check_ad[type][rangeI][i]);
	}
	printf("\n");
	}

	
	print = 0;
	if(print){
	printf("tmp caliCheck DVM_Point ");
	for(i=0; i < 10; i++) {
		printf("%f ", myData->cali.tmpData[0][ch].check_meter[type][rangeI][i]);
	}
	printf("\n");
	printf("\n");
	}

	print = 0;
	if(print){
	printf("range : %d\n", myData->cali.tmpCond[0][ch].range);
	printf("\n");
	printf("type : %d\n", myData->cali.tmpCond[0][ch].type);
	printf("\n");
	}
/*	
#if SHUNT_R_RCV == 1
	printf("shuntSerialNo\n");
	for(i=0; i < 16; i++){
		printf("%d:%s ",i, myData->cali.tmpCond[0][i].point[type][rangeI].shuntSerialNo);
	}
	printf("\n");
	printf("\n");
	printf("shuntValue[mOhm]\n");
	for(i=0; i < 16; i++){
	printf("%f ", myData->cali.tmpCond[0][i].point[type][rangeI].shuntValue);
	}
	printf("\n");
	printf("interval : %ld ", myData->mData.cali_meas_time);
	printf("\n");
#endif

	ch = 2;
	printf("orige ad    ");
	for(i=0; i < MAX_CALI_POINT; i++) {
		printf("%f, ", (float)myData->cali.org[bd][ch].ad[type][0][i]);
	}
	printf("\n");

	printf("orige meter ");
	for(i=0; i < MAX_CALI_POINT; i++) {
		printf("%f, ", (float)myData->cali.org[bd][ch].meter[type][0][i]);
	}
	printf("\n");
*/	
	printf("\n");
}

void CaliMeter_Config_Print(void)
{
	printf("\nCaliMeter_Config_Print\n");
	printf("Shunt_Sel_Calibrator: %d\n", myData->CaliMeter.config.Shunt_Sel_Calibrator);
	printf("Shunt_mOhm: %f\n", myData->CaliMeter.shunt_mOhm);
	printf("range: %d\n", myData->cali.tmpCond[0][0].range);
}

void Chamber_Temp_Change_Print(void)
{
	int i, val, bd_1, ch_1;
	
	printf("\nChamber Temperature Change Process\n\n");
	printf("bd select [1BASE] : ");
	scanf("%d", &bd_1);
	if(bd_1 <= 0 || bd_1 > myData->mData.config.installedBd){
		puts("Invalid value.");
		return;
	}
	printf("ch select [0:all] : ");
	scanf("%d", &ch_1);
	if(ch_1 < 0 || ch_1 > myData->mData.config.installedCh){
		puts("Invalid value.");
		return;
	}
	printf("Change Temp Value (1'C=1000) : ");
	scanf("%d", &val);
	if(ch_1 != 0){
		myData->bData[bd_1-1].cData[ch_1-1].misc.groupTemp = val;
	}else{
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			myData->bData[bd_1-1].cData[i].misc.groupTemp = val;
		}
	}
	printf("Chamber Temperature\n");
	for(i=0; i < myData->mData.config.chPerBd; i++) {
		printf("%ld ", myData->bData[bd_1-1].cData[i].misc.groupTemp);
	}
	printf("\n");
}

void Temp_Change_Print(void)
{
	int i, val, bd_1, ch_1;
	
	printf("\nAUX Temperature Change Process\n\n");
	printf("AnalogMeter Process select [1BASE] : ");
	scanf("%d", &bd_1);
	if(bd_1 <= 0 || bd_1 > myData->mData.config.installedBd){
		puts("Invalid value.");
		return;
	}
	printf("ch select [0:all] : ");
	scanf("%d", &ch_1);
	if(ch_1 < 0 || ch_1 > myData->mData.config.installedCh){
		puts("Invalid value.");
		return;
	}
	printf("Aux Temp Value (1'C=1000) : ");
	scanf("%d", &val);
	if(bd_1 == 1){
		if(ch_1 != 0){
		myData->bData[bd_1-1].cData[ch_1-1].op.temp = val;
	//		myData->AnalogMeter.temp[ch_1-1].temp = val;
		}else{
			for(i=0; i < myData->mData.config.installedTemp; i++) {
			myData->bData[bd_1-1].cData[i].op.temp = val;
	//			myData->AnalogMeter.temp[i].temp = val;
			}
		}
	}else{	//AnalogMeter2
		if(ch_1 != 0){
			myData->AnalogMeter2.temp[ch_1-1].temp = val;
		}else{
			for(i=0; i < myData->mData.config.installedTemp; i++) {
				myData->AnalogMeter2.temp[i].temp = val;
			}
		}
	}
	/*
	printf("Aux Temperature\n");
	for(i=0; i < myData->mData.config.chPerBd; i++) {
		printf("%ld ", myData->bData[bd_1-1].cData[i].misc.groupTemp);
	}*/
}

void CAN_JIG_IO_Change_Print(void)
{
#if CYCLER_TYPE == CAN_CYC
	int i, val, num, num1 ;
//	unsigned char hex8, tmpVal;
	
	printf("\nCAN_JIG_IO Change Process\n\n");

	printf("clear : 0 \nJIG_CAN1 : 1 / JIG_CAN1(binary) : 2 \nJIG_CAN2 : 3 / JIG_CAN2(binary) : 4 \n");
	printf("Select num : ");
	scanf("%d", &val);
	if(val > 0 && val <= 4){
		printf("NUMBER OUT : ");
		scanf("%d", &num);
	}else{
		num = 0;
		printf("clear M_SIG_CAN\n ");
	}
	
	switch(val){
		case 1:
			if(num < 0) break;
			for(i = 0; i < 8; i++){
				if(i == (num - 1)){
					myData->mData.signal[M_SIG_CAN1_0 + i] = P1;
				}else{
					myData->mData.signal[M_SIG_CAN1_0 + i] = P0;
				}
			}
			break;
		case 2:
			//binary
			for(i = 0; i < 8; i++){
				num1 = num % 2;
				if(num1 == 1){
					myData->mData.signal[M_SIG_CAN1_0 + i] = P1;
				}else{
					myData->mData.signal[M_SIG_CAN1_0 + i] = P0;
				}
				num = num / 2;
			}
			break;
		case 3:
			if(num < 0) break;
			for(i = 0; i < 8; i++){
				if(i == (num -1)){
					myData->mData.signal[M_SIG_CAN2_0 + i] = P1;
				}else{
					myData->mData.signal[M_SIG_CAN2_0 + i] = P0;
				}
			}
			break;
		case 4:
			//binary
			for(i = 0; i < 8; i++){
				num1 = num % 2;
				if(num1 == 1){
					myData->mData.signal[M_SIG_CAN2_0 + i] = P1;
				}else{
					myData->mData.signal[M_SIG_CAN2_0 + i] = P0;
				}
				num = num / 2;
			}
			break;
		default:
			for(i = 0; i <16; i++){
				myData->mData.signal[M_SIG_CAN1_0 + i] = P0;
			}
			break;
	}
#endif
}

void MainClient_Config_Print(void)
{
	printf("\nMainClient_Config\n");
	printf("ipAddr      : %s\n", 
			myData->MainClient.config.ipAddr);
	printf("sendPort    : %d\n", 
			myData->MainClient.config.sendPort);
	printf("receivePort    : %d\n", 
			myData->MainClient.config.receivePort);
	printf("networkPort    : %d\n", 
			myData->MainClient.config.networkPort);
	printf("replyTimeout    : %d\n", 
			myData->MainClient.config.replyTimeout);
	printf("retryCount    : %d\n", 
			myData->MainClient.config.retryCount);
	printf("netTimeout    : %d\n", 
			myData->MainClient.config.netTimeout);
	printf("pingTimeout    : %d\n", 
			myData->MainClient.config.pingTimeout);
	printf("CmdSendLog    : %d\n", 
			myData->MainClient.config.CmdSendLog);
	printf("CmdRcvLog    : %d\n", 
			myData->MainClient.config.CmdRcvLog);
	printf("CmdSendLog_Hex  : %d\n", 
			myData->MainClient.config.CmdSendLog_Hex);
	printf("CmdRcvLog_Hex   : %d\n", 
			myData->MainClient.config.CmdRcvLog_Hex);
	printf("CommCheckLog    : %d\n", 
			myData->MainClient.config.CommCheckLog);
	printf("send_monitor_data_interval : %ld\n", 
			myData->MainClient.config.send_monitor_data_interval);
	printf("send_save_data_interval : %ld\n", 
			myData->MainClient.config.send_save_data_interval);
	printf("protocol_version : %d\n", 
			myData->MainClient.config.protocol_version);
	printf("state_change : %d\n", 
			myData->MainClient.config.state_change);
	printf("\n");

}

void GAS_AMBIENT_TEST(void)
{
//	int i, j;
//	int chamber_module_no;
//	int ambientTemp_1=0, ambientTemp_2=0, ambientTemp_3=0, ambientTemp_4=0;
//	int gasVoltage_1=0, gasVoltage_2=0, gasVoltage_3=0, gasVoltage_4=0;

	myData->bData[0].cData[0].misc.ambientTemp = 20000;
	myData->bData[0].cData[0].misc.gasVoltage = 2000000;
	myData->bData[0].cData[1].misc.ambientTemp = 30000;
	myData->bData[0].cData[1].misc.gasVoltage = 3000000;
	/*
	printf("What's the set chamber Module Number ?");
	scanf("%d", &chamber_module_no);

	if(chamber_module_no >= 1 && chamber_module_no <= 4){
		printf("\n ChamberChNo 1 Set ");
		printf("\n ambientTemp : ");
		scanf("%d",&ambientTemp_1);
		printf(" gasVoltage : ");
		scanf("%d", &gasVoltage_1);
		if(chamber_module_no >= 2){
			printf("\n ChamberChNo 2 Set ");
			printf("\n ambientTemp : ");
			scanf("%d",&ambientTemp_2);
			printf(" gasVoltage : ");
			scanf("%d", &gasVoltage_2);
			if(chamber_module_no >= 3){
				printf("\n ChamberChNo 3 Set ");
				printf("\n ambientTemp : ");
				scanf("%d",&ambientTemp_3);
				printf(" gasVoltage : ");
				scanf("%d", &gasVoltage_3);
				if(chamber_module_no == 4){
					printf("\n ChamberChNo 4 Set ");
					printf("\n ambientTemp : ");
					scanf("%d",&ambientTemp_4);
					printf(" gasVoltage : ");
					scanf("%d", &gasVoltage_4);
				}
			}
		}
	}else{
	 printf("Check the chamber module number again !!\n");
	}
	for(i=0; i < myData->mData.config.installedCh; i++){
		j = myData->ChamberChNo[i].number1;
		if(j == 1){
			myData->bData[0].cData[i].misc.ambientTemp = ambientTemp_1;
			myData->bData[0].cData[i].misc.gasVoltage = gasVoltage_1;
		}else if(j == 2){
			myData->bData[0].cData[i].misc.ambientTemp = ambientTemp_2;
			myData->bData[0].cData[i].misc.gasVoltage = gasVoltage_2;
		}else if(j == 3){
			myData->bData[0].cData[i].misc.ambientTemp = ambientTemp_3;
			myData->bData[0].cData[i].misc.gasVoltage = gasVoltage_3;
		}else if(j == 4){
			myData->bData[0].cData[i].misc.ambientTemp = ambientTemp_4;
			myData->bData[0].cData[i].misc.gasVoltage = gasVoltage_4;
		}else{
			myData->bData[0].cData[i].misc.ambientTemp = 0;
			myData->bData[0].cData[i].misc.gasVoltage = 0;
		}
	}*/
}

void HwFault_Config_Print(void)
{
	printf(" OVP Voltage      : %ld\n", 
			myData->mData.config.hwFaultConfig[HW_FAULT_OVP]);
	printf(" CCC Current      : %ld\n", 
			myData->mData.config.hwFaultConfig[HW_FAULT_CCC]);
	printf(" OTP Value        : %ld\n", 
			myData->mData.config.hwFaultConfig[HW_FAULT_OTP]);
	printf(" DROP_V Charge    : %ld\n", 
			myData->mData.config.hwFaultConfig[HW_FAULT_DROP_V_1]);
	printf(" DROP_V DisCharge : %ld\n", 
			myData->mData.config.hwFaultConfig[HW_FAULT_DROP_V_2]);
	printf("\n");
}

void PCU_State_Print(void)
{
	int i, val, bd_1, ch_1;
	
	printf("\nTemperature Change Process\n\n");
	printf("bd select [1BASE] : ");
	scanf("%d", &bd_1);
	if(bd_1 <= 0 || bd_1 > myData->mData.config.installedBd){
		puts("Invalid value.");
		return;
	}
	printf("ch select [0:all] : ");
	scanf("%d", &ch_1);
	if(ch_1 < 0 || ch_1 > myData->mData.config.installedCh){
		puts("Invalid value.");
		return;
	}
	printf("Change Temp Value (1'C=1000) : ");
	scanf("%d", &val);
	if(ch_1 != 0){
		myData->bData[bd_1-1].cData[ch_1-1].op.temp = val;
	}else{
		for(i=0; i < myData->mData.config.chPerBd; i++) {
			myData->bData[bd_1-1].cData[i].op.temp = val;
		}
	}
	printf("Temperature\n");
	for(i=0; i < myData->mData.config.chPerBd; i++) {
		printf("%ld ", myData->bData[bd_1-1].cData[i].op.temp);
	}
	printf("\n");
}

void CAN_State_Print(void)
{
	int bd, bdNum, m_bd, inv, i, print = 0;

	m_bd = myData->mData.config.installedBd;
	inv = myData->CAN.config.installedInverter;
	bdNum = 8;
	
	print = 1;
	if(print == 1){
		printf("-----------CAN_MAINBD_STATE DATA_OUT-------------\n");
		print = 1;
		if(print == 1){
			for(bd = 0; bd < bdNum ; bd++){ 
				printf("can_read_v %dbd\n", bd+1);
				for(i = 0; i < 8; i++){
					printf("%ld  ",myData->bData[bd].cData[i].misc.can_read_v);
				}
				printf("\n");
			}
			printf("\n");
		}

		print = 0;
		if(print == 1){
			for(bd = 0; bd < bdNum ; bd++){ 
				printf("can_read_i %dbd\n", bd+1);
				for(i = 0; i < 8; i++){
					printf("%ld  ",myData->bData[bd].cData[i].misc.can_read_i);
				}
				printf("\n");
			}
			printf("\n");
		}
		print = 1;
		if(print == 1){
			printf("can_read_errCnt %dbd\n", bd+1);
			for(i = 0; i < 8; i++){
				printf("%d  ",myData->bData[bd].cData[i].misc.can_read_errCnt);
			}
			printf("\n");

			printf("can_error %dbd\n", bd+1);
			for(i = 0; i < 8; i++){
				printf("%d  ",myData->bData[bd].cData[i].misc.can_error);
			}
			printf("\n");

			printf("can_range_state %dbd\n", bd+1);
			for(i = 0; i < 8; i++){
				printf("%d  ",myData->bData[bd].cData[i].misc.can_range_state);
			}
			printf("\n");

			printf("can_run_state %dbd\n", bd+1);
			for(i = 0; i < 8; i++){
				printf("%d  ",myData->bData[bd].cData[i].misc.can_run_state);
			}
			printf("\n");

			printf("can_read_update_flag %dbd\n", bd+1);
			for(i = 0; i < 8; i++){
				printf("%d  ",myData->bData[bd].cData[i].misc.can_read_update_flag);
			}
			printf("\n");

			printf("range_out_state %dbd\n", bd+1);
			for(i = 0; i < 8; i++){
				printf("%d  ",myData->bData[bd].cData[i].signal[C_SIG_RANGE_SWITCH]);
			}
			printf("\n");

			printf("range_out_on_state %dbd\n", bd+1);
			for(i = 0; i < 8; i++){
				printf("%d  ",myData->bData[bd].cData[i].signal[C_SIG_RANGE_SWITCH_ON]);
			}
			printf("\n");
	
			printf("range_out_off_state %dbd\n", bd+1);
			for(i = 0; i < 8; i++){
				printf("%d  ",myData->bData[bd].cData[i].signal[C_SIG_RANGE_SWITCH_OFF]);
			}
			printf("\n");

			printf("run_out_state %dbd\n", bd+1);
			for(i = 0; i < 8; i++){
				printf("%d  ",myData->bData[bd].cData[i].signal[C_SIG_OUT_SWITCH]);
			}
			printf("\n");
	
			printf("run_out_on_state %dbd\n", bd+1);
			for(i = 0; i < 8; i++){
				printf("%d  ",myData->bData[bd].cData[i].signal[C_SIG_OUT_SWITCH_ON]);
			}
			printf("\n");

			printf("run_out_off_state %dbd\n", bd+1);
			for(i = 0; i < 8; i++){
				printf("%d  ",myData->bData[bd].cData[i].signal[C_SIG_OUT_SWITCH_OFF]);
			}
			printf("\n");

			printf("PRE_REF_V %dbd\n", bd+1);
			for(i = 0; i < 8; i++){
				printf("%ld  ",myData->bData[bd].cData[i].misc.preVref);
			}
			printf("\n");

			printf("PRE_REF_I %dbd\n", bd+1);
			for(i = 0; i < 8; i++){
				printf("%ld  ",myData->bData[bd].cData[i].misc.preIref);
			}
			printf("\n");
		}
	}

	print = 0;
	if(print == 1){
		printf("-----------CAN_CONFIG_FILE-------------\n");
		printf("Installed Can:[%d]\n", myData->CAN.config.installedCAN);
		printf("CAN Function type\n");
		for(i = 0; i < MAX_CAN_PORT; i++){
			printf("%d  ",myData->CAN.config.functionType[i]);
		}
		printf("\n");

		printf("CAN Port\n");
		for(i = 0; i < MAX_CAN_PORT; i++){
			printf("%d  ",myData->CAN.config.canPort[i]);
		}
		printf("\n");

		printf("CAN Bps\n");
		for(i = 0; i < MAX_CAN_PORT; i++){
			printf("%d  ",myData->CAN.config.canBps[i]);
		}
		printf("\n");

		printf("CAN CommType\n");
		for(i = 0; i < MAX_CAN_PORT; i++){
			printf("%d  ",myData->CAN.config.commType[i]);
		}
		printf("\n");

		printf("CAN CmdSendlog\n");
		for(i = 0; i < MAX_CAN_PORT; i++){
			printf("%d  ",myData->CAN.config.CmdSendLog[i]);
		}
		printf("\n");

		printf("CAN CmdRcvlog\n");
		for(i = 0; i < MAX_CAN_PORT; i++){
			printf("%d  ",myData->CAN.config.CmdRcvLog[i]);
		}
		printf("\n");

		printf("CAN CommTimeOut\n");
		for(i = 0; i < MAX_CAN_PORT; i++){
			printf("%ld  ",myData->CAN.config.canCommTimeOut[i]);
		}
		printf("\n");

		printf("CAN Installed Dev Number\n");
		for(i = 0; i < MAX_CAN_PORT; i++){
			printf("%d  ",myData->CAN.config.canInsDevNum[i]);
		}
		printf("\n");

		printf("Ch In Inverter\n");
		for(i = 0; i < MAX_INVERTER_NUM; i++){
			printf("%d  ",myData->CAN.config.chInInv[i]);
		}
		printf("\n");

		printf("CAN In Bd\n");
		for(i = 0; i < MAX_BD_PER_MODULE; i++){
			printf("%d  ",myData->CAN.config.canInBd[i]);
		}
		printf("\n");
	}
	
	print = 0;
	if(print == 1){
		printf("----------CAN_IO_VALUE_OUT-------------\n");
		printf("CanJigIO Input value\n");
		for(i = 0; i < CAN_IO_INPUT_CNT; i++){
			printf("[%d : %d]   ",i ,myData->CAN.io.inputValue[i]);
		}
		printf("\n");

		printf("CanJigIO Output value\n");
		for(i = 0; i < CAN_IO_OUTPUT_CNT; i++){
			printf("[%d : %d]   ",i ,myData->CAN.io.outputValue[i]);

		}
		printf("\n");
	}

	print = 0;
	if(print == 1){
		printf("=== CAN comm.StateFlag\n");
		printf("Main Board\n");
		for(i = 0; i < m_bd; i++) {
			printf("%d ",(int)myData->CAN.main[i].comm.StateFlag);
		}
		printf("\n");
		printf("Inverter\n");
		for(i = 0; i < inv; i++) {
			printf("%d ",(int)myData->CAN.inverter[i].comm.StateFlag);
		}
		printf("\n");
		printf("IO Board\n");
		printf("%d",(int)myData->CAN.io.comm.StateFlag);
		printf("\n");
		printf("=== CAN comm.pingOutCnt\n");
		printf("Main Board\n");
		for(i = 0; i < m_bd; i++) {
			printf("%d ",(int)myData->CAN.main[i].comm.pingOutCnt);
		}
		printf("\n");
		printf("Inverter\n");
		for(i = 0; i < inv; i++) {
			printf("%d ",(int)myData->CAN.inverter[i].comm.pingOutCnt);
		}
		printf("\n");
		printf("IO Board\n");
		printf("%d",(int)myData->CAN.io.comm.pingOutCnt);
		printf("\n");
		printf("=== CAN comm.Error\n");
		printf("Main Board\n");
		for(i = 0; i < m_bd; i++) {
			printf("%d ",(int)myData->bData[i].signal[B_SIG_MAIN_CAN_COMM_ERROR]);
		}
		printf("\n");
		printf("Inverter\n");
		for(i = 0; i < inv; i++) {
			printf("%d ",(int)myData->CAN.inverter[i]
							.signal[CAN_SIG_INV_CAN_COMM_ERROR]);
		}
		printf("\n");
		printf("IO Board\n");
		printf("%d",(int)myData->CAN.io.signal[CAN_SIG_IO_CAN_COMM_ERROR]);
		printf("\n");
	}
	printf("\n");
}

void UserDataNo_Print(void)
{
	int i;
	printf("\n");
		printf("userDataNo\n");
	for(i=0; i < myData->mData.config.chPerBd; i++) {
		printf("[%02d] : %d ", i+1, myData->bData[bd].cData[i].misc.userDataNo);
		if((i+1)%8 == 0)
			printf("\n");
	}
	printf("\n");
		printf("usingUserDataFlag\n");
	for(i = 0; i < MAX_CH_PER_BD; i++) {
		printf("[%02d] : %d ", i+1, myData->mData.misc.usingUserDataFlag[i]);
		if((i+1)%8 == 0)
			printf("\n");
	}
}

void AC_Fail_Recovery_print(void)
{
	int i;

	if(mode == 0) {
		printf("\nChannelState bd(%d)\n", bd+1);
	} else {
		printf("\nChannelState bd(%c)\n", bd+'A');
	}

	printf("advCycle\n");
	for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.advCycle);
	}printf("\n");

	printf("advCycleStep\n");
	for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.advCycleStep);
	}printf("\n");

	printf("seedintegralCapacity\n");
	for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.seedintegralCapacity);
	}printf("\n");
	
	printf("suminetralCapacity\n");
	for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.sumintegralCapacity);
	}printf("\n");
	
	printf("seedintegralWattHour\n");
	for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.seedintegralWattHour);
	}printf("\n");
	
	printf("sumintegralWattHour\n");
	for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.sumintegralWattHour);
	}printf("\n");
	
	printf("seedChargeAmpareHour\n");
	for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.seedChargeAmpareHour);
	}printf("\n");
	
	printf("sumChargeAmpareHour\n");
	for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.sumChargeAmpareHour);
	}printf("\n");

	printf("seedDischargeAmpareHour\n");
	for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.seedDischargeAmpareHour);
	}printf("\n");
	
	printf("sumDischargeAmpareHour\n");
	for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.sumDischargeAmpareHour);
	}printf("\n");

	printf("seedChargeWattHour\n");
	for(i=0; i < myData->mData.config.chPerBd; i++) {
		printf("%ld ", myData->bData[bd].cData[i].misc.seedChargeWattHour);
	}printf("\n");

	printf("sumChargeWattHour\n");
	for(i=0; i < myData->mData.config.chPerBd; i++) {
		printf("%ld ", myData->bData[bd].cData[i].misc.sumChargeWattHour);
	}printf("\n");

	printf("seedDischargeWattHour\n");
	for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.seedDischargeWattHour);
	}printf("\n");

	printf("sumDischargeWattHour\n");
	for(i=0; i < myData->mData.config.chPerBd; i++) {
			printf("%ld ", myData->bData[bd].cData[i].misc.sumDischargeWattHour);
	}printf("\n");
	
	printf("standardP\n");
	for(i=0; i < myData->mData.config.chPerBd; i++) {
		printf("%ld ", myData->bData[bd].cData[i].misc.standardP);
	}printf("\n");

	printf("standardZ\n");
	for(i=0; i < myData->mData.config.chPerBd; i++) {
		printf("%ld ", myData->bData[bd].cData[i].misc.standardZ);
	}printf("\n");

	printf("cycleSumC\n");
	for(i=0; i < myData->mData.config.chPerBd; i++) {
		printf("%ld ", myData->bData[bd].cData[i].misc.cycleSumC);
	}printf("\n");

	printf("cycleSumP\n");
	for(i=0; i < myData->mData.config.chPerBd; i++) {
		printf("%ld ", myData->bData[bd].cData[i].misc.cycleSumP);
	}printf("\n");

	printf("cycleEndC\n");
	for(i=0; i < myData->mData.config.chPerBd; i++) {
		printf("%ld ", myData->bData[bd].cData[i].misc.cycleEndC);
	}printf("\n");
	
	printf("pattern_charge_flag\n");
	for(i=0; i < myData->mData.config.chPerBd; i++) {
		printf("%d ", myData->bData[bd].cData[i].misc.pattern_change_flag);
	}printf("\n");
	
	printf("\n");
}


void Convert_FormToCyc_CaliData(void)
{
	unsigned int m_bd, range, rtn, i, j, answer;
	char	cmd[128];

	m_bd = myData->mData.config.installedBd;
	range = myData->mData.config.rangeI;
		
	system("rm -rf /root/Convert_caliData");
	system("mkdir /root/Convert_caliData");

	for(i = 0; i < m_bd; i++) {
		memset(cmd, 0x00, sizeof(cmd));
		sprintf(cmd, "touch /root/Convert_caliData/CALI_BD%d", i+1);
		system(cmd);
	}

	for(i = 0; i < m_bd; i++) {
		for(j = 0; j < range; j++) {
			rtn = Read_Form_Cali_Data(i, j);
			if(rtn == 0) {
				rtn = Read_Form_Cali_Check_Data(i, j);
			}
		}
	}
	if(rtn == 0) {
		for(i = 0; i < m_bd; i++) {
			rtn = Write_Cycler_BdCaliData(i);
		}
	}
	if(rtn == 0) {
		printf("\n === Formation to Cycler caliData file convert complete ===\n");
		sleep (1);
		printf("\n Apply calibration data right away? [1:YES 0:NO] : ");
		scanf("%d",&answer);
		if(answer == 1){
			system("cp -rf /root/Convert_caliData/* /root/cycler_data/config/caliData");
			printf("\n === Apply calibration data complete ===\n");
			sleep (1);
		}
	}
	for(i = 0; i < m_bd; i++) {
		rtn = Read_Cycler_BdCaliData(i);
	}
	if(rtn == 0) {
		printf("\n === Cycler caliData reload complete ===\n");
		sleep (1);
	}
}

void Read_HwFault_Config(void)
{
	Read_HwFault_Config();
	printf("\n === Read_HwFault_Cofig reload complete ===\n");
}
//Cycler Cali Data
int Read_Cycler_BdCaliData(int m_bd)
{
	int		rtn, tmp, ch, type, range, point, read_ch;
	char	fileName[128], temp[64], buf[64];
	FILE	*fp;

	S_CALI_DATA data[MAX_CH_PER_BD];

	memset(data, 0, sizeof(S_CALI_DATA)*MAX_CH_PER_BD);
	
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
						data[ch].point[type][range].setPoint[point]	= atol(buf);
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
						data[ch].point[type][range].checkPoint[point] = atol(buf);
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

					tmp = fscanf(fp, "%s", temp); //da_a
					for(point=0; point < MAX_CALI_POINT-1; point++) {
						memset(buf, 0x00, sizeof(buf));
						tmp = fscanf(fp, "%s", buf);
						data[ch].DA_A[type][range][point] = (double)atof(buf);
					}

					tmp = fscanf(fp, "%s", temp); //da_b
					for(point=0; point < MAX_CALI_POINT-1; point++) {
						memset(buf, 0x00, sizeof(buf));
						tmp = fscanf(fp, "%s", buf);
						data[ch].DA_B[type][range][point] = (double)atof(buf);
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

					tmp = fscanf(fp, "%s", temp); //ad_ratio
					memset(buf, 0x00, sizeof(buf));
					tmp = fscanf(fp, "%s", buf);
					data[ch].AD_Ratio[type][range][0] = (double)atof(buf);
					memset(buf, 0x00, sizeof(buf));
					tmp = fscanf(fp, "%s", buf);
					data[ch].AD_Ratio[type][range][1] = (double)atof(buf);
				}
			}
		}
	} else {
		printf("Can not open %d Board Cali Data file(read)\n", m_bd+1);
		rtn = -1;
	}
	fclose(fp);

	if(rtn == 0) {
		memcpy((char *)&myData->cali.data[m_bd][0], (char *)&data[0],
			sizeof(S_CALI_DATA)*MAX_CH_PER_BD);
	}
	return rtn;
}

//CAN type Formation Cali Data
int Read_Form_Cali_Data(int m_bd, int range) 
{
	int		rtn=0, tmp, ch, type, point;
	char	fileName[128], temp[64], buf[64];
	FILE	*fp;

	memset(fileName, 0x00, sizeof(fileName));
	sprintf(fileName, "/root/formation_data/config/caliData/CALI_BD%d_R%d", m_bd+1, range+1);
	// /root/formation_data/config/caliData/CALI_BD?_R?#

	if((fp = fopen(fileName, "r")) != NULL) {
		rtn = 0;
		tmp = fscanf(fp, "%s", temp);//first line
		tmp = fscanf(fp, "%s", temp);//second line
		tmp = fscanf(fp, "%s", temp);//day
		tmp = fscanf(fp, "%s", temp);//Month
		tmp = fscanf(fp, "%s", temp);//date
		tmp = fscanf(fp, "%s", temp);//time
		tmp = fscanf(fp, "%s", temp);//year
		
		for(type=0; type < MAX_TYPE; type++) {
			tmp = fscanf(fp, "%s", temp); //voltage, current

			for(ch=0; ch < MAX_CH_PER_BD; ch++) {
				tmp = fscanf(fp, "%s", temp); //ch

				tmp = fscanf(fp, "%s", temp); //PointNum
				
				memset(buf, 0x00, sizeof(buf));
				tmp = fscanf(fp, "%s", buf);
				myData->cali.data[m_bd][ch].point[type][range].setPointNum = (unsigned char)atoi(buf);
				myData->cali.data[m_bd][ch].point[type][range].checkPointNum = (unsigned char)atoi(buf);

				tmp = fscanf(fp, "%s", temp); //Cmd
				for(point=0; point < MAX_CALI_POINT; point++) {
					memset(buf, 0x00, sizeof(buf));
					tmp = fscanf(fp, "%s", buf);
					myData->cali.data[m_bd][ch].point[type][range].setPoint[point] = atol(buf);
				}

				tmp = fscanf(fp, "%s", temp); //AD
				for(point=0; point < MAX_CALI_POINT; point++) {
					memset(buf, 0x00, sizeof(buf));
					tmp = fscanf(fp, "%s", buf);
					myData->cali.data[m_bd][ch].set_ad[type][range][point] = (double)atof(buf);
				}

				tmp = fscanf(fp, "%s", temp); //Meter
				for(point=0; point < MAX_CALI_POINT; point++) {
					memset(buf, 0x00, sizeof(buf));
					tmp = fscanf(fp, "%s", buf);
					myData->cali.data[m_bd][ch].set_meter[type][range][point] = (double)atof(buf);
				}

				tmp = fscanf(fp, "%s", temp); //AD_A
				for(point=0; point < MAX_CALI_POINT-1; point++) {
					memset(buf, 0x00, sizeof(buf));
					tmp = fscanf(fp, "%s", buf);
					myData->cali.data[m_bd][ch].AD_A[type][range][point] = (double)atof(buf);
				}

				tmp = fscanf(fp, "%s", temp); //AD_B
				for(point=0; point < MAX_CALI_POINT-1; point++) {
					memset(buf, 0x00, sizeof(buf));
					tmp = fscanf(fp, "%s", buf);
					myData->cali.data[m_bd][ch].AD_B[type][range][point] = (double)atof(buf);
				}

				tmp = fscanf(fp, "%s", temp); //aux da_a
				for(point=0; point < MAX_CALI_POINT-1; point++) {
					tmp = fscanf(fp, "%s", temp);
				}

				tmp = fscanf(fp, "%s", temp); //aux da_b
				for(point=0; point < MAX_CALI_POINT-1; point++) {
					tmp = fscanf(fp, "%s", temp);
				}

				tmp = fscanf(fp, "%s", temp); //DA_A
				for(point=0; point < MAX_CALI_POINT-1; point++) {
					memset(buf, 0x00, sizeof(buf));
					tmp = fscanf(fp, "%s", buf);
					myData->cali.data[m_bd][ch].DA_A[type][range][point] = (double)atof(buf);
				}

				tmp = fscanf(fp, "%s", temp); //DA_B
				for(point=0; point < MAX_CALI_POINT-1; point++) {
					memset(buf, 0x00, sizeof(buf));
					tmp = fscanf(fp, "%s", buf);
					myData->cali.data[m_bd][ch].DA_B[type][range][point] = (double)atof(buf);
				}
			}
		}
	} else {
		printf("Can not open %d Board Cali Data file(read)\n", m_bd+1);
		rtn = -1;
	}
	fclose(fp);
	
	return rtn;
}

//CAN type Formation Cali Check Data
int Read_Form_Cali_Check_Data(int m_bd, int range)
{
	int		rtn=0, tmp, ch, type, point;
	char	fileName[128], temp[64], buf[64];
	FILE	*fp;

	memset(fileName, 0x00, sizeof(fileName));
	sprintf(fileName, "/root/formation_data/config/caliData/CALI_BD%d_R%d_Check", m_bd+1, range+1);
	// /root/formation_data/config/caliData/CALI_BD?_R?_Check#

	if((fp = fopen(fileName, "r")) != NULL) {
		rtn = 0;
		tmp = fscanf(fp, "%s", temp);//first line 
		tmp = fscanf(fp, "%s", temp);//second line
		tmp = fscanf(fp, "%s", temp);//day
		tmp = fscanf(fp, "%s", temp);//month
		tmp = fscanf(fp, "%s", temp);//date
		tmp = fscanf(fp, "%s", temp);//time
		tmp = fscanf(fp, "%s", temp);//year
		
		for(type=0; type < MAX_TYPE; type++) {
			tmp = fscanf(fp, "%s", temp); //voltage, current

			for(ch=0; ch < MAX_CH_PER_BD; ch++) {
				tmp = fscanf(fp, "%s", temp); //ch

				tmp = fscanf(fp, "%s", temp); //Cmd
				for(point=0; point < MAX_CALI_POINT; point++) {
					memset(buf, 0x00, sizeof(buf));
					tmp = fscanf(fp, "%s", buf);
					myData->cali.data[m_bd][ch].point[type][range].checkPoint[point] = atol(buf);
				}

				tmp = fscanf(fp, "%s", temp); //AD
				for(point=0; point < MAX_CALI_POINT; point++) {
					memset(buf, 0x00, sizeof(buf));
					tmp = fscanf(fp, "%s", buf);
					myData->cali.data[m_bd][ch].check_ad[type][range][point] = (double)atof(buf);
				}

				tmp = fscanf(fp, "%s", temp); //Meter
				for(point=0; point < MAX_CALI_POINT; point++) {
					memset(buf, 0x00, sizeof(buf));
					tmp = fscanf(fp, "%s", buf);
					myData->cali.data[m_bd][ch].check_meter[type][range][point] = (double)atof(buf);
				}
			}
		}
	} else {
		printf("Can not open %d Board Cali Check Data file(read)\n", m_bd+1);
		rtn = -1;
	}
	fclose(fp);

	return rtn;
}

int Write_Cycler_BdCaliData(int m_bd)
{
    int	ch, type, range, point, save_ch, tmp;
	char temp[4], fileName[128], cmd[128];
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
	strcpy(fileName, "/root/Convert_caliData/CALI_BD");
	memset(temp, 0x00, sizeof temp);
	//190624 oys modify start : Support CALI_BD16 file
	if(bd < 9){
		temp[0] = (char)(49+m_bd);
	} else {
		temp[0] = (char)(49);
		temp[1] = (char)(39+m_bd);
	}
	//190624 oys modift end
	strcat(fileName, temp);
	sprintf(cmd,"touch %s",fileName);
	system(cmd);
	// /root/cycler_data/config/caliData/CALI_BD#
	
    if((fp = fopen(fileName, "w")) == NULL) {
		printf("Can not open %d Board CaliData file(write)\n", m_bd+1);
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

//			for(ch=0; ch < MAX_CH_PER_BD; ch++) 
			for(ch=0; ch < save_ch; ch++) {
	    		fprintf(fp, "ch%02d\n", ch+1);
				
				fprintf(fp, "setPointNum   \n");
			   	fprintf(fp, "%d ", myData->cali.data[m_bd][ch]
					.point[type][range].setPointNum);
				fprintf(fp, "\n");

				fprintf(fp, "setPoint      \n");
				for(point=0; point < MAX_CALI_POINT; point++) {
			    	fprintf(fp, "%ld ", myData->cali.data[m_bd][ch]
						.point[type][range].setPoint[point]);
				}
				fprintf(fp, "\n");

				fprintf(fp, "checkPointNum \n");
			   	fprintf(fp, "%d ", myData->cali.data[m_bd][ch]
					.point[type][range].checkPointNum);
				fprintf(fp, "\n");

				fprintf(fp, "checkPoint    \n");
				for(point=0; point < MAX_CALI_POINT; point++) {
			    	fprintf(fp, "%ld ", myData->cali.data[m_bd][ch]
						.point[type][range].checkPoint[point]);
				}
				fprintf(fp, "\n");

				fprintf(fp, "set_ad        \n");
				for(point=0; point < MAX_CALI_POINT; point++) {
#if CYCLER_TYPE == 0
				   	fprintf(fp, "%f ", (double)myData->cali.data[m_bd][ch]
								.set_ad[type][range][point]);
#endif
#if CYCLER_TYPE == 1
					if(myData->cali.data[m_bd][ch]
								.point[type][range].setPointNum > point){
			    		fprintf(fp, "%f ", (double)myData->cali.data[m_bd][ch]
								.set_ad[type][range][point]);
					}else{
			   			fprintf(fp, "%f ", (double)tmp);
					}
#endif
				}
				fprintf(fp, "\n");

				fprintf(fp, "set_meter     \n");
				for(point=0; point < MAX_CALI_POINT; point++) {
#if CYCLER_TYPE == 0
				   	fprintf(fp, "%f ", (double)myData->cali.data[m_bd][ch]
								.set_meter[type][range][point]);
#endif
#if CYCLER_TYPE == 1
					if(myData->cali.data[m_bd][ch]
								.point[type][range].setPointNum > point){
		    			fprintf(fp, "%f ", (double)myData->cali.data[m_bd][ch]
							.set_meter[type][range][point]);
					}else{
		    			fprintf(fp, "%f ", (double)tmp);
					}					
#endif
				}
				fprintf(fp, "\n");

				fprintf(fp, "check_ad      \n");
				for(point=0; point < MAX_CALI_POINT; point++) {
#if CYCLER_TYPE == 0
					fprintf(fp, "%f ", (double)myData->cali.data[m_bd][ch]
						.check_ad[type][range][point]);
#endif
#if CYCLER_TYPE == 1
					if(myData->cali.data[m_bd][ch]
								.point[type][range].checkPointNum > point){
		    		fprintf(fp, "%f ", (double)myData->cali.data[m_bd][ch]
								.check_ad[type][range][point]);
					}else{
		    			fprintf(fp, "%f ", (double)tmp);
					}					
#endif
				}
				fprintf(fp, "\n");

				fprintf(fp, "check_meter   \n");
				for(point=0; point < MAX_CALI_POINT; point++) {
#if CYCLER_TYPE == 0
		    		fprintf(fp, "%f ", (double)myData->cali.data[m_bd][ch]
						.check_meter[type][range][point]);
#endif
#if CYCLER_TYPE == 1
					if(myData->cali.data[m_bd][ch]
								.point[type][range].checkPointNum > point){
				    	fprintf(fp, "%f ", (double)myData->cali.data[m_bd][ch]
										.check_meter[type][range][point]);
					}else{
			    		fprintf(fp, "%f ", (double)tmp);
					}					
#endif
				}
				fprintf(fp, "\n");

				fprintf(fp, "DA_A          \n");
				for(point=0; point < MAX_CALI_POINT-1; point++) {
#if CYCLER_TYPE == 0
		  		  	fprintf(fp, "%f ", (double)myData->cali.data[m_bd][ch]
						.DA_A[type][range][point]);
#endif
#if CYCLER_TYPE == 1
					if(myData->cali.data[m_bd][ch]
								.point[type][range].setPointNum-1 > point){
				    	fprintf(fp, "%f ", (double)myData->cali.data[m_bd][ch]
										.DA_A[type][range][point]);
					}else{
		  		  		fprintf(fp, "%f ", (double)tmp);
					}
#endif
				}
				fprintf(fp, "\n");

				fprintf(fp, "DA_B          \n");
				for(point=0; point < MAX_CALI_POINT-1; point++) {
#if CYCLER_TYPE == 0
		    		fprintf(fp, "%f ", (double)myData->cali.data[m_bd][ch]
						.DA_B[type][range][point]);
#endif
#if CYCLER_TYPE == 1
					if(myData->cali.data[m_bd][ch]
								.point[type][range].setPointNum-1 > point){
		 		   		fprintf(fp, "%f ", (double)myData->cali.data[m_bd][ch]
							.DA_B[type][range][point]);
					}else{
		  		  		fprintf(fp, "%f ", (double)tmp);
					}					
#endif
				}
				fprintf(fp, "\n");

				fprintf(fp, "AD_A          \n");
				for(point=0; point < MAX_CALI_POINT-1; point++) {
#if CYCLER_TYPE == 0
	    			fprintf(fp, "%f ", (double)myData->cali.data[m_bd][ch]
						.AD_A[type][range][point]);
#endif
#if CYCLER_TYPE == 1
					if(myData->cali.data[m_bd][ch]
								.point[type][range].setPointNum-1 > point){
	    				fprintf(fp, "%f ", (double)myData->cali.data[m_bd][ch]
							.AD_A[type][range][point]);
					}else{
		    			fprintf(fp, "%f ",(double)tmp);
					}					
#endif
				}
				fprintf(fp, "\n");

				fprintf(fp, "AD_B\n");
				for(point=0; point < MAX_CALI_POINT-1; point++) {
#if CYCLER_TYPE == 0
		    		fprintf(fp, "%f ", (double)myData->cali.data[m_bd][ch]
						.AD_B[type][range][point]);
#endif
#if CYCLER_TYPE == 1
					if(myData->cali.data[m_bd][ch]
								.point[type][range].setPointNum-1 > point){
			    		fprintf(fp, "%f ", (double)myData->cali.data[m_bd][ch]
										.AD_B[type][range][point]);
					}else{
			 	   		fprintf(fp, "%f ", (double)tmp);
					}					
#endif
				}
				fprintf(fp, "\n");

				fprintf(fp, "AD_Ratio      \n");
				fprintf(fp, "1.000000 1.000000");
				fprintf(fp, "\n\n");
			}
		}
	}
   	fclose(fp);

	return 0;
}

int KeyInput(void)
{
	int i, rtn;
	rtn = 0;
	i = getc(stdin);
	switch(i) {
		case '1':	bd=0; mode=0; break;
		case '2':	bd=1; mode=0; break;
		case '3':	bd=2; mode=0; break;
		case '4':	bd=3; mode=0; break;
		case '5':	bd=4; mode=0; break;
		case '6':	bd=5; mode=0; break;
		case '7':	bd=6; mode=0; break;
		case '8':	bd=7; mode=0; break;
		case 'a':	bd=0; mode=1; break;
		case 'b':	bd=1; mode=1; break;
		case 'c':	bd=2; mode=1; break;
		case 'd':	bd=3; mode=1; break;
		case 'e':	bd=4; mode=1; break;
		case 'f':	bd=5; mode=1; break;
		case 'g':	bd=6; mode=1; break;
		case 'h':	bd=7; mode=1; break;
		case 'q': 	rtn=-1; break;
		case 'n': 	rtn=2; break;
		default: break;
	}
	return rtn;
}

int main(void)
{
    int	retval, rtn, i, flag = 0;
    struct timeval tv;
    fd_set rfds;

	if(Open_SystemMemory(0) < 0) return -1;

	bd = 0;
	mode = 0;

	for(i=0; i < 8; i++) {
		min[i] = 0;
		max[i] = 0;
	}
	max[7] = 0;
	min[7] = myData->mData.config.maxCurrent[0];
	max[8] = myData->mData.config.minCurrent[0];
	min[8] = 0;
	system("clear");
	for(i=0; i < 80; i++) {
		printf("=");
	}
	printf(" DEBUG MONITOR PROGRAM\n");
	for(i=0; i < 80; i++) {
		printf("=");
	}
	printf(" 0 : DataSize_Print\t\t\t 1 : Debug_Print\n");
 	printf(" 2 : DAQ_Print\t\t\t\t 3 : Addr_Print\n");
 	printf(" 4 : TimeSlot_Print\t\t\t 5 : AppControl_Print\n");
 	printf(" 6 : DataSave_Print\t\t\t 7 : CaliMeter_Print\n");
 	printf(" 8 : CaliMeter2_Print\t\t\t 9 : AnalogMeter_Print\n");
 	printf("10 : FADM_Print\t\t\t\t11 : ModuleState_Print\n");
 	printf("12 : GroupState_Print\t\t\t13 : Reference_IC_Data_Save\n");
 	printf("14 : BoardState_Print\t\t\t15 : ChannelState_Print\n");
 	printf("16 : DIO_Print\t\t\t\t17 : TestCond_Print\n");
 	printf("18 : CaliData_Print\t\t\t19 : CaliMeter_Config_Print\n");
 	printf("20 : Chamber_Temp_Change_Print\t\t21 : Temp_Change_Print\n");
 	printf("22 : HwFault_Config_Print\t\t23 : PCU_State_Print\n");
 	printf("24 : CAN_State_Print\t\t\t25 : UserDataNo_Print\n");
	printf("26 : AC_Fail_Recovery_print\t\t27 : CAN_JIG_IO_Change_Print\n");
	printf("28 : MainClient_Config_Print\t\t29 : GAS_AMBIENT_TEST\n");
 	printf("99 : Convert form(CAN)to cyc caliData\n");
 	printf("100 : Read HwFaultConfig\n");
	printf(" Ctrl + c : Exit\n\n");
	printf(" Select mode : ");
	scanf("%d",&flag);

    while(1) {
	    tv.tv_sec = 0;
	    tv.tv_usec = 500000;
	    FD_ZERO(&rfds);
	    FD_SET(0, &rfds);
		retval = select(1, &rfds, NULL, NULL, &tv);
	    if(retval == 0) {
			Test_Print(flag);
			printf("\n'n' : initialize value\n");
			printf("'q' : Main Menu\n");
			printf("Ctrl + c : Exit\n");
		} else {
			printf("rtn %d\n", retval);
			rtn = KeyInput();
			if(rtn < 0){
				main();
				break;
			}else if(rtn == 2){
				memset((long *)&myData->test_val_l, 0x00, sizeof(long)*MAX_TEST_VALUE); 
				memset((char *)&myData->test_val_c,	0x00, sizeof(char)*MAX_TEST_VALUE); 
			}
		}
    }

	Close_SystemMemory();
    return 0;
}
