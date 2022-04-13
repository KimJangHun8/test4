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
extern volatile S_ANALOG_METER  *myPs;
extern char psName[16];

int analog_cali_read(int bd, int value, int flag){
	int i;
	int chPerModule = 0;	
	int chPerModule2 = 0;	
	//value = set Temp
	//cali_point1 = low Temp  cali_point2 = hig Temp
	//ambientCaliPoint1 = low ambient Temp  ambientCaliPoint2 = high ambient Temp
	if(bd  == AMBIENT_MODULE_NO){
		myPs->misc.ambientCaliPoint[bd - AMBIENT_MODULE_NO][flag] = value*1000;
		chPerModule = myPs->config.chPerModule;
		if(flag == 0){
			userlog(METER2_LOG, psName, 
				"org_ambient_temp point_low %d[C] BD %d\n",value,bd);
			for(i=0;i<myPs->config.chPerModule;i++){
				myPs->misc.ambientCaliPoint1[bd - AMBIENT_MODULE_NO][i] 
						= myData->mData.misc.ambientTemp_org[(chPerModule * (bd - AMBIENT_MODULE_NO)) + i];
				userlog(METER2_LOG,psName,
						"low point %d Ch org_ambient_Temp %ld\n ",
					i,
				    myData->mData.misc.ambientTemp_org[(chPerModule * (bd - AMBIENT_MODULE_NO)) + i]);
			}
		}else if(flag == 1){
			userlog(METER2_LOG, psName, 
				"org_ambient_temp point_high %d[C] BD %d\n",value,bd);
			for(i=0;i<myPs->config.chPerModule;i++){
				myPs->misc.ambientCaliPoint2[bd - AMBIENT_MODULE_NO][i] 
						= myData->mData.misc.ambientTemp_org[(chPerModule * (bd - AMBIENT_MODULE_NO)) + i];
				userlog(METER2_LOG,psName,
						"high point %d Ch org_ambient_Temp %ld\n ",
					i,
				    myData->mData.misc.ambientTemp_org[(chPerModule * (bd - AMBIENT_MODULE_NO)) + i]);
			}
		} else {
			return -1;
		}
	}else if(bd == GAS_MODULE_NO || bd == GAS_MODULE_NO + 1){
		myPs->misc.gasCaliPoint[bd - GAS_MODULE_NO][flag] = value*1000;
		chPerModule2 = myPs->config.chPerModule2; 
		if(flag == 0){
			userlog(METER2_LOG, psName, 
				"org_gas_voltage point_low %d[V] BD %d\n",value,bd);
			//for(i=0;i<8;i++) //M-7003
			for(i=0;i < myPs->config.chPerModule2 ;i++){	//M-7002
				myPs->misc.gasCaliPoint1[bd - GAS_MODULE_NO][i] 
					= myData->mData.misc.gasVoltage_org[chPerModule2 * (bd - GAS_MODULE_NO) + i]; 
				userlog(METER2_LOG,psName,
					"low point %d Ch org_gasVoltage %ld\n ",
					i,myData->mData.misc.gasVoltage_org[chPerModule2 * (bd - GAS_MODULE_NO) + i]); 
			}
		}else if(flag == 1){
			userlog(METER2_LOG, psName, 
				"org_ambient_temp point_high %d[V] BD %d\n",value,bd);
			//for(i=0;i<8;i++) //M-7003
			for(i=0;i < myPs->config.chPerModule2 ;i++){	//M-7002
				myPs->misc.gasCaliPoint2[bd - GAS_MODULE_NO][i] 
					= myData->mData.misc.gasVoltage_org[chPerModule2 * (bd - GAS_MODULE_NO) + i];
				userlog(METER2_LOG,psName,
					"high point %d Ch org_gasVoltage %ld\n ",
					i,myData->mData.misc.gasVoltage_org[chPerModule2 * (bd - GAS_MODULE_NO) + i]);
			}
		} else {
			return -1;
		}
	}else{
		myPs->misc.caliPoint[bd-1][flag] = value*1000;
		if(flag == 0)
		{
			userlog(METER2_LOG, psName, "org_temp point_low %dC BD %d\n",value,bd);
			for(i=0;i<myPs->config.chPerModule;i++)
			{
				myPs->misc.cali_point1[bd-1][i] = myPs->misc.org_temp[bd-1][i];
				userlog(METER2_LOG,psName,"low point %d Ch orgTemp %ld\n ",
					i,myPs->misc.cali_point1[bd-1][i]);
			}
		}
		else if(flag == 1)
		{
			userlog(METER2_LOG, psName,"org_temp point_high %dC BD %d\n",value,bd) ;
			for(i=0;i<myPs->config.chPerModule;i++)
			{
				myPs->misc.cali_point2[bd-1][i] = myPs->misc.org_temp[bd-1][i];
				userlog(METER2_LOG,psName,"high point %d Ch orgTemp %ld\n",
					i,myPs->misc.cali_point2[bd-1][i]);
			}
		} else {
			return -1;
		}
	}
	return 0;	
}

int analog_cali_calc(int bd){
	int i,offset;
	long x1,x2,y1,y2;
	float gain;
	
	if(bd == 31){
		y1 = myPs->misc.ambientCaliPoint[bd-31][0];
		y2 = myPs->misc.ambientCaliPoint[bd-31][1];
	
		for(i=0;i<myPs->config.chPerModule;i++)
		{
			x1 = myPs->misc.ambientCaliPoint1[bd-31][i];
			x2 = myPs->misc.ambientCaliPoint2[bd-31][i];
			gain = ((float)(y1-y2))/((float)(x1-x2));
			offset = y1-(long)(gain*x1);
			myPs->misc.ambient_gain[bd-31][i] = gain;
			myPs->misc.ambient_offset[bd-31][i] = offset;
			userlog(METER2_LOG, psName,"ambient %dBD %dCH : %f %d\n",bd-31,i+1,gain,offset);
		}
		myPs->misc.ambient_caliFlag[bd-31] = P1;
	}else if(bd == 41 || bd == 42){
		y1 = myPs->misc.gasCaliPoint[bd-41][0];
		y2 = myPs->misc.gasCaliPoint[bd-41][1];
	
		//for(i=0;i<8;i++) //M-7003
		for(i=0;i < myPs->config.chPerModule2 ;i++) //M-7002
		{
			x1 = myPs->misc.gasCaliPoint1[bd-41][i];
			x2 = myPs->misc.gasCaliPoint2[bd-41][i];
			if(x2 - x1 < 100){
				gain = 1.0;
				offset = 0;
			}else{
				gain = ((float)(y1-y2))/((float)(x1-x2));
				offset = y1-(long)(gain*x1);
			}
			myPs->misc.gas_gain[bd-41][i] = gain;
			myPs->misc.gas_offset[bd-41][i] = offset;
			userlog(METER2_LOG, psName,"gas %dBD %dCH : %f %d\n",bd-41,i+1,gain,offset);
		}
		myPs->misc.gas_caliFlag[bd-41] = P1;
	}else{
		y1 = myPs->misc.caliPoint[bd-1][0];
		y2 = myPs->misc.caliPoint[bd-1][1];
	
		for(i=0;i<myPs->config.chPerModule;i++)
		{
			x1 = myPs->misc.cali_point1[bd-1][i];
			x2 = myPs->misc.cali_point2[bd-1][i];
			gain = ((float)(y1-y2))/((float)(x1-x2));
			offset = y1-(long)(gain*x1);
			myPs->misc.measure_gain[bd-1][i] = gain;
			myPs->misc.measure_offset[bd-1][i] = offset;
			userlog(METER2_LOG, psName,"%dBD %dCH : %f %d\n",bd,i+1,gain,offset);
		}
		myPs->misc.caliFlag[bd-1] = P1;
	}	
	return 0;
}

int analog_cali_update(int bd){
	int i;

	for(i=0;i<MAX_METER_COUNT;i++) {
		if(myPs->misc.caliFlag[i] == P1) {
			myPs->misc.caliFlag[i] = P0;
			memcpy((char *)&myPs->config.measure_gain[i][0],(char *)&myPs->
				misc.measure_gain[i][0],sizeof(myPs->config.measure_gain[i]));
			memcpy((char *)&myPs->config.measure_offset[i][0],(char *)&myPs->
				misc.measure_offset[i][0],sizeof(myPs->config.measure_offset[i]));
		}
		if(myPs->misc.ambient_caliFlag[i] == P1) {
			myPs->misc.ambient_caliFlag[i] = P0;
			memcpy((char *)&myPs->config.ambient_gain[i][0],(char *)&myPs->
				misc.ambient_gain[i][0],sizeof(myPs->config.ambient_gain[i]));
			memcpy((char *)&myPs->config.ambient_offset[i][0],(char *)&myPs->
				misc.ambient_offset[i][0],sizeof(myPs->config.ambient_offset[i]));
		}
		if(myPs->misc.gas_caliFlag[i] == P1) {
			myPs->misc.gas_caliFlag[i] = P0;
			memcpy((char *)&myPs->config.gas_gain[i][0],(char *)&myPs->
				misc.gas_gain[i][0],sizeof(myPs->config.gas_gain[i]));
			memcpy((char *)&myPs->config.gas_offset[i][0],(char *)&myPs->
				misc.gas_offset[i][0],sizeof(myPs->config.gas_offset[i]));
		}

	}
	//20170619 add sch
//	if(myData->AppControl.config.versionNo < 20170619){
		if(Write_AnalogMeter_Config() < 0) {
			userlog(METER2_LOG, psName,"Write AnalogMeter_Config Fail\n");
		} else {
			userlog(METER2_LOG, psName,"Write AnalogMeter_Config Success\n");
		}
//	}else{
		if(Write_AnalogMeter_CaliData() < 0) {
			userlog(METER2_LOG, psName,"Write AnalogMeter_Cali Fail\n");
		} else {
			userlog(METER2_LOG, psName,"Write AnalogMeter_Cali Success\n");
		}
		if(Write_AnalogMeter_Ambient_CaliData() < 0) {
			userlog(METER2_LOG, psName,"Write AnalogMeter_Ambient_Cali Fail\n");
		} else {
			userlog(METER2_LOG, psName,"Write AnalogMeter_Ambient_Cali Success\n");
		}
		if(Write_AnalogMeter_gas_CaliData() < 0) {
			userlog(METER2_LOG, psName,"Write AnalogMeter_gas_Cali Fail\n");
		} else {
			userlog(METER2_LOG, psName,"Write AnalogMeter_gas_Cali Success\n");
		}
//	}
	return 0;		
}

void analog_cali_auto(void)
{
	int count = 0;
	int i = 0;

	switch(myPs->misc.phase){
		case P0:
			break;			
		case P1:
			myPs->misc.check_count += 1;
			if(myPs->misc.check_count >=3){
				userlog(METER2_LOG, psName, "normal_count : [%d]\n",myPs->misc.normal_count);
				myPs->misc.check_count = 0;
				myPs->misc.gasCaliPoint[0][0] = 1000;	//1V
				userlog(METER2_LOG, psName, "org_gas_voltage point_low 1[V]\n");
				myPs->misc.phase++;
			}
			break;
		case P2:
			myPs->misc.check_count += 1;
			for(i=0;i < myPs->config.chPerModule2 ;i++) //M-7002
			{
				if(myData->mData.misc.gasVoltage_org[i] >= 998 &&
					myData->mData.misc.gasVoltage_org[i] <= 1002){
					count++;
				}
			}
			if(count == myPs->misc.normal_count){
				myPs->misc.phase++;
				myPs->misc.check_count = 0;
				userlog(METER2_LOG, psName, "low 1[V] Wait\n");
			}else{
				count = 0;
			}
			if(myPs->misc.check_count >= 180){ // 18sec 
				userlog(METER2_LOG, psName, "org_gasVoltage Not Standby P2\n");
				myPs->misc.check_count = 0;
				myPs->misc.phase = P0;
				myPs->misc.auto_cali_flag = P0;
			}
			break;
		case P3:
			myPs->misc.check_count += 1;
			if(myPs->misc.check_count >= 30){ // 5.4sec 
				for(i = 0 ; i < myPs->misc.normal_count ; i++){
					myPs->misc.gasCaliPoint1[0][i] = myData->mData.misc.gasVoltage_org[i];
					userlog(METER2_LOG,psName,"low point %d Ch org_gasVoltage %ld\n ",
											i,myData->mData.misc.gasVoltage_org[i]);
				}
				myPs->misc.check_count = 0;
				myPs->misc.phase++;
			}
			break;
		case P4:
			myPs->misc.check_count += 1;
			if(myPs->misc.check_count >=3){
				myPs->misc.check_count = 0;
				myPs->misc.gasCaliPoint[0][1] = 5000;	//5V
				userlog(METER2_LOG, psName, "org_gas_voltage point_high 5[V]\n");
				myPs->misc.phase++;
			}
			break;
		case P5:
			myPs->misc.check_count += 1;
			for(i=0;i < myPs->config.chPerModule2 ;i++) //M-7002
			{
				if(myData->mData.misc.gasVoltage_org[i] >= 4998 &&
					myData->mData.misc.gasVoltage_org[i] <= 5002){
					count++;
				}
			}
			if(count == myPs->misc.normal_count){
				myPs->misc.phase++;
				myPs->misc.check_count = 0;
				userlog(METER2_LOG, psName, "high 5[V] Wait\n");
			}else{
				count = 0;
			}
			if(myPs->misc.check_count >= 180){ // 18sec 
				userlog(METER2_LOG, psName, "org_gasVoltage Not Standby P4\n");
				myPs->misc.check_count = 0;
				myPs->misc.phase = P0;
				myPs->misc.auto_cali_flag = P0;
			}
			break;
		case P6:
			myPs->misc.check_count += 1;
			if(myPs->misc.check_count >= 30){ // 5.4sec 
				for(i = 0 ; i < myPs->misc.normal_count ; i++){
					myPs->misc.gasCaliPoint2[0][i] = myData->mData.misc.gasVoltage_org[i];
					userlog(METER2_LOG,psName,"high point %d Ch org_gasVoltage %ld\n ",
											i,myData->mData.misc.gasVoltage_org[i]);
				}
				myPs->misc.check_count = 0;
				myPs->misc.phase++;
			}
			break;
		case P7:
			myPs->misc.check_count += 1;
			if(myPs->misc.check_count >=3){
				analog_cali_calc(41);
				myPs->misc.check_count = 0;
				myPs->misc.phase++;
			}
			break;
		case P8:
			myPs->misc.check_count += 1;
			if(myPs->misc.check_count >=3){
				analog_cali_update(0);
				myPs->misc.check_count = 0;
				myPs->misc.phase = P0;
				myPs->misc.auto_cali_flag = P0;
			}
			break;
	}
}
#ifdef _TEMP_CALI  
// temp_value 1Point ratio & offset calibration
int analog_cali_calc_2(int setPointNo)
{
	unsigned char point_count;
//	int i ,rtn, countMeter, chPerModule, tempNo;
	int i ,rtn, tempNo;
	long temp_ref1, temp_ref2, temp_value1, temp_value2;
	float value_diff = 0;
	float ratio, offset;
	
	rtn = 0;
//	temp_ref1 = temp_ref2 = 0;
//	temp_value1 = temp_value2 = 0;
	point_count = myData->temp_cali.point.setPointCount - 1;
//	countMeter = myPs->config.countMeter;
//	chPerModule = myPs->config.chPerModule;
//	tempNo = countMeter * chPerModule;
	tempNo = myData->mData.config.installedCh;
	if(setPointNo != 0) {
		temp_ref1 = myData->temp_cali
					.point.setTempPoint[setPointNo - 1] * 1000;
		temp_ref2 = myData->temp_cali
					.point.setTempPoint[setPointNo] * 1000;
		for(i=0; i < tempNo; i++) {
			temp_value1 = myData->temp_cali.data
										.setTempValue[setPointNo-1][i];
			temp_value2 = myData->temp_cali.data
										.setTempValue[setPointNo][i];
			if(temp_value1 == temp_value2) {
				value_diff = 0.1;
			} else {
				value_diff = ((float)temp_value2 - (float)temp_value1);
			}
			ratio = ((float)temp_ref2 - (float)temp_ref1) /((float)value_diff);
		   	offset = ((float)temp_ref1) - ((float)ratio * (float)temp_value1);	

//			if((temp_value1 > 10000000)&&(temp_value1 < -10000000)) {
			if(temp_value1 == 99999000) {
				myData->temp_cali.measure.gain[setPointNo-1][i] = 1;
				myData->temp_cali.measure.offset[setPointNo-1][i] = 0;
			} else {
				myData->temp_cali.measure.gain[setPointNo-1][i] = ratio;
				myData->temp_cali.measure.offset[setPointNo-1][i] = offset;
			}	
		}				
	}

	if(setPointNo != 0 && setPointNo < point_count) {
		temp_ref1 = myData->temp_cali
					.point.setTempPoint[setPointNo] * 1000;
		temp_ref2 = myData->temp_cali
					.point.setTempPoint[setPointNo+1] * 1000;
		for(i=0; i < tempNo; i++) {
			temp_value1 = myData->temp_cali.data
										.setTempValue[setPointNo][i];
			temp_value2 = myData->temp_cali.data
										.setTempValue[setPointNo+1][i];
			if(temp_value1 == temp_value2) {
				value_diff = 0.1;
			} else {
				value_diff = ((float)temp_value2 - (float)temp_value1);
			}
			ratio = ((float)temp_ref2 - (float)temp_ref1) / ((float)value_diff);
		   	offset = (float)temp_ref1 - ((float)ratio * (float)temp_value1);	
			
			if(temp_value2 == 99999000) {
//			if((temp_value2 > 10000000)&&(temp_value2 < -10000000)) {
				myData->temp_cali.measure.gain[setPointNo][i] = 1;
				myData->temp_cali.measure.offset[setPointNo][i] = 0;
			} else {
				myData->temp_cali.measure.gain[setPointNo][i] = ratio;
				myData->temp_cali.measure.offset[setPointNo][i] = offset;
			}	
		}
	}

	return 0;
}

//200105
int analog_cali_update_2()
{
	int rtn, setPointNo;	
	
	setPointNo = myPs->temp_cali.pointNo;
	userlog(DEBUG_LOG, psName,"Temp_Calibration End / pointNo : %d\n", setPointNo+1);
	
	rtn = Write_AnalogMeter_CaliData_2();
		
	if(rtn < 0) {
		userlog(METER2_LOG, psName,"Update AnalogMeter_CaliData Fail\n");
	} else {
		userlog(METER2_LOG, psName,"Update AnalogMeter_CaliData Success\n");
	}
	
	return rtn;
}
#endif
