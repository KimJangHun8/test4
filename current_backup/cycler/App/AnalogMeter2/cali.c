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

int analog_cali_read(int bd,int value,int flag){
	int i;
	myPs->misc.caliPoint[bd-1][flag] = value*1000;
	if(flag == 0)
	{
		userlog(METER2_LOG, psName, "org_temp point_low %dC BD %d\n",value,bd);
		for(i=0;i<myPs->config.chPerModule;i++)
		{
			myPs->misc.cali_point1[bd-1][i] = myPs->misc.org_temp[bd-1][i];
			userlog(METER2_LOG,psName,"low point %d Ch orgTemp %ld\n ",i,myPs->misc.cali_point1[bd-1][i]);
		}
	}
	else if(flag == 1)
	{
		userlog(METER2_LOG, psName,"org_temp point_high %dC BD %d\n",value,bd) ;
		for(i=0;i<myPs->config.chPerModule;i++)
		{
			myPs->misc.cali_point2[bd-1][i] = myPs->misc.org_temp[bd-1][i];
			userlog(METER2_LOG,psName,"high point %d Ch orgTemp %ld\n",i,myPs->misc.cali_point2[bd-1][i]);
		}
	} else {
		return -1;
	}
	return 0;	
}

int analog_cali_calc(int bd){
	int i,offset;
	long x1,x2,y1,y2;
	float gain;
/*	for(i=0;i<8;i++)
	{
		if(labs(CALI_POINT1 - myPs->misc.cali_point1[bd-1][i]) >= 5000)
		{
			printf("%d BD %d CH point1 value is unusual\n",bd,i);
			return -1;
		}
		if(labs(CALI_POINT2 - myPs->misc.cali_point2[bd-1][i]) >= 5000)
		{
			printf("%d BD %d CH point2 value is unusual\n",bd,i);
			return -1;
		}
	}
*/	
//	memcpy((char *)&myPs->misc.measure_gain[bd-1][0],(char *)&myPs->
//		config.measure_gain[bd-1][0],sizeof(myPs->misc.measure_gain[bd-1]));
//	memcpy((char *)&myPs->misc.measure_offset[bd-1][0],(char *)&myPs->
//		config.measure_offset[bd-1][0],sizeof(myPs->misc.measure_offset[bd-1]));
	
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
	return 0;
}

int analog_cali_update(){
	int i;

	for(i=0;i<MAX_METER_COUNT;i++) {
		if(myPs->misc.caliFlag[i] == P1) {
			myPs->misc.caliFlag[i] = P0;
			memcpy((char *)&myPs->config.measure_gain[i][0],(char *)&myPs->
				misc.measure_gain[i][0],sizeof(myPs->config.measure_gain[i]));
			memcpy((char *)&myPs->config.measure_offset[i][0],(char *)&myPs->
				misc.measure_offset[i][0],sizeof(myPs->config.measure_offset[i]));
		}
	}
	//20170619 Modify sch
//	if(myData->AppControl.config.versionNo < 20170619){
		if(Write_AnalogMeter_Config()<0) {
			userlog(METER2_LOG, psName,"Write AnalogMeter_Config_2 Fail\n");
		} else {
			userlog(METER2_LOG, psName,"Write AnalogMeter_Config_2 Success\n");
		}
//	}else{
		if(Write_AnalogMeter_CaliData()<0) {
			userlog(METER2_LOG, psName,"Write AnalogMeter_Cali_2 Fail\n");
		} else {
			userlog(METER2_LOG, psName,"Write AnalogMeter_Cali_2 Success\n");
		}
//	}
	return 0;		
}
