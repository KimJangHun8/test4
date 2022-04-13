#include <rtl_core.h>
#include <asm/io.h>
#include <pthread.h>
#include "../rt_can/rt_can.h"
#include "../../INC/datastore.h"
#include "common_utils.h"
#include "ChannelControl.h"
#include "message.h"
#include "CAN.h"

extern S_SYSTEM_DATA	*myData;
extern S_MODULE_DATA	*myPs;
extern S_CH_DATA		*myCh;

//#define CAN_LINK_CHECK	1
//#define CAN_DATA_READ		2

void CAN_Control(int type, int bd){
	int i, group = 0;

	if(myData->CAN.config.canUseFlag == 0) return;
	if(bd >= myPs->config.installedBd) return;

	Can_Group_Select(group);

	switch(type){
		case CAN_MAIN_LINK_CHECK:
			Can_Main_Link_Check(bd);
			break;
		case CAN_INV_LINK_CHECK:
			Can_Inv_Link_Check(0);
			break;
		case CAN_IO_LINK_CHECK:
			Can_Io_Link_Check(0);
			break;
		case CAN_DATA_READ:
			for(i = 0; i < myData->CAN.config.installedCAN; i++){
				can_read_msg(i, 0);
			}
			break;
		default:
			break;
	}
}

void Can_Group_Select(int group)
{
	outb(0x01<<group, 0x600);
}

//******Next Function is for Main BD CAN Communication*******//
void Can_Main_Link_Check(int bd){
	unsigned char ch = 0;	
	int ref_idx;
	long diff = 0;
	
	if(myData->AppControl.config.debugType != P0) return;
	
    if(bd >= myPs->config.installedBd) return;

	ref_idx = myData->CAN.config.canInBd[bd]-1;
	if(myData->CAN.config.canPort[ref_idx] == 0) return;
	
	//for communication error check
	if(myData->CAN.main[bd].comm.StateFlag != P1
		&&(myPs->signal[M_SIG_INV_POWER_CAN] == P10
		|| myPs->signal[M_SIG_INV_POWER_CAN] == P99)){
		diff = myPs->misc.timer_1sec 
				- myData->CAN.main[bd].comm.pingOutCnt; 	
		if(diff > myData->CAN.config.canCommTimeOut[ref_idx]){
			//send CAN(main) communication fault 
			if(myData->bData[bd].signal[B_SIG_MAIN_CAN_COMM_ERROR] == P0){
				//Send only one
				myData->bData[bd].signal[B_SIG_MAIN_CAN_COMM_ERROR] = P1;
				myData->CAN.main[bd].comm.code = M_FAIL_MAIN_CAN_COMM_ERROR;
				myPs->code = myData->CAN.main[bd].comm.code;
				send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 31, bd+1);
				send_msg(MODULE_TO_MAIN, 
						MSG_MODULE_MAIN_EMG_STATUS, myPs->code, bd+1);
			}
		}
	}else{
		myData->CAN.main[bd].comm.pingOutCnt = myPs->misc.timer_1sec;
		myData->CAN.main[bd].comm.StateFlag = P0;
		//for checking communication state 
		myData->bData[bd].signal[B_SIG_MAIN_CAN_COMM_ERROR] = P0;
	}

	if(myData->bData[bd].signal[B_SIG_MAIN_CAN_COMM_ERROR] != P0){
		//for next comm error check
		myData->CAN.main[bd].comm.pingOutCnt = myPs->misc.timer_1sec;
	}

	bCan_Each_Link_Out(bd, ch); 
}

//Each can msg function
void bCan_Each_Range_Out(int bd, int ch, int range){
	int group=0;
	U_CAN_FULL_SEND_MSG s_msg;	

//	printk("rangout called %d bd %d ch %d range\n", bd, ch,range);//sungdel

	if(bd < 0 || bd >= myPs->config.installedBd) return;

	Can_Group_Select(group);

	s_msg.data.channel = (char)ch;
	s_msg.data.relay = CAN_DUMMY_PACKET;
	s_msg.data.range = range;
	s_msg.data.command = CAN_COMMAND_RANGE;
	s_msg.data.v_ref[0] = CAN_DUMMY_PACKET;
	s_msg.data.v_ref[1] = CAN_DUMMY_PACKET;
	s_msg.data.i_ref[0] = CAN_DUMMY_PACKET;
	s_msg.data.i_ref[1] = CAN_DUMMY_PACKET;
	s_msg.data.i_ref[2] = CAN_DUMMY_PACKET;
	s_msg.data.i_ref[3] = CAN_DUMMY_PACKET;

	can_transmit(bd, group, (U_CAN_FULL_SEND_MSG *)&s_msg); 
}

void bCan_Each_Run_Out(int bd, int ch, int run){
	int group=0;
	U_CAN_FULL_SEND_MSG s_msg;	
	if(bd < 0 || bd >= myPs->config.installedBd) return;

	Can_Group_Select(group);

	s_msg.data.channel = (char)ch;
	s_msg.data.relay = CAN_DUMMY_PACKET;
	s_msg.data.range = run;
	s_msg.data.command = CAN_COMMAND_RUN;
	s_msg.data.v_ref[0] = CAN_DUMMY_PACKET;
	s_msg.data.v_ref[1] = CAN_DUMMY_PACKET;
	s_msg.data.i_ref[0] = CAN_DUMMY_PACKET;
	s_msg.data.i_ref[1] = CAN_DUMMY_PACKET;
	s_msg.data.i_ref[2] = CAN_DUMMY_PACKET;
	s_msg.data.i_ref[3] = CAN_DUMMY_PACKET;

	can_transmit(bd, group, (U_CAN_FULL_SEND_MSG *)&s_msg); 
}

void bCan_Each_Ref_Cmd_Out(char type, int bd, int ch, int ref){
	int tmp_vRef = 0, tmp_iRef = 0, group=0;
	U_CAN_FULL_SEND_MSG s_msg;	

	if(bd < 0 || bd >= myPs->config.installedBd) return;

	Can_Group_Select(group);

	s_msg.data.channel = (char)ch;
	s_msg.data.relay = CAN_DUMMY_PACKET;
	s_msg.data.range = CAN_DUMMY_PACKET;

	if(type == 0){//voltage
	//	tmp_vRef = ref / 100; //100uV
		tmp_vRef = labs(ref / 1000); //100uV
		tmp_iRef = CAN_DUMMY_PACKET; //1uA
		s_msg.data.command = CAN_COMMAND_V_REF;
	}else{//current
		tmp_vRef = CAN_DUMMY_PACKET;
		tmp_iRef = labs(ref);
		s_msg.data.command = CAN_COMMAND_I_REF;
	}

	s_msg.data.v_ref[0] = BYTE0(tmp_vRef);
	s_msg.data.v_ref[1] = BYTE1(tmp_vRef);
	if(type == 0 && ref < 0) s_msg.data.v_ref[1] |= 0x80;
	s_msg.data.i_ref[0] = BYTE0(tmp_iRef);
	s_msg.data.i_ref[1] = BYTE1(tmp_iRef);
	s_msg.data.i_ref[2] = BYTE2(tmp_iRef);
	s_msg.data.i_ref[3] = BYTE3(tmp_iRef);
	if(type == 1 && ref < 0) s_msg.data.i_ref[3] |= 0x80;

	can_transmit(bd, group, (U_CAN_FULL_SEND_MSG *)&s_msg); 
}

void bCan_Each_Link_Out(int bd, int ch){
	int group=0;
	U_CAN_FULL_SEND_MSG s_msg;	

	if(bd < 0 || bd >= myPs->config.installedBd) return;

	Can_Group_Select(group);

	s_msg.data.channel = (char)ch;
	s_msg.data.relay = CAN_DUMMY_PACKET;
	s_msg.data.range = CAN_DUMMY_PACKET;
	s_msg.data.command = CAN_COMMAND_LINK;
	s_msg.data.v_ref[0] = CAN_DUMMY_PACKET;
	s_msg.data.v_ref[1] = CAN_DUMMY_PACKET;
	s_msg.data.i_ref[0] = CAN_DUMMY_PACKET;
	s_msg.data.i_ref[1] = CAN_DUMMY_PACKET;
	s_msg.data.i_ref[2] = CAN_DUMMY_PACKET;
	s_msg.data.i_ref[3] = CAN_DUMMY_PACKET;
	can_transmit(bd, group, (U_CAN_FULL_SEND_MSG *)&s_msg); 
}
/*
#else
//Each can msg function
void bCan_Each_Range_Out(int bd, char ch, int range){
	int group=0;
	
	U_CAN_EACH_SEND_MSG s_msg;	

	//printk("rangout called %d bd %d ch %d range\n", bd, ch,range);//sungdel

	if(bd < 0 || bd >= myData->mData.config.installedBd) return;

	Can_Group_Select(group);

	s_msg.data.channel = ch;
	s_msg.data.command = CAN_COMMAND_RANGE;
	s_msg.data.run_range = range;
	s_msg.data.reserved = CAN_DUMMY_PACKET;
	s_msg.data.ref[0] = CAN_DUMMY_PACKET;
	s_msg.data.ref[1] = CAN_DUMMY_PACKET;
	s_msg.data.ref[2] = CAN_DUMMY_PACKET;
	s_msg.data.ref[3] = CAN_DUMMY_PACKET;

	can_transmit(bd, group, (U_CAN_FULL_SEND_MSG *)&s_msg); 
}

void bCan_Each_Run_Out(int bd, char ch, int run){
	int group=0;
	U_CAN_EACH_SEND_MSG s_msg;	

	if(bd < 0 || bd >= myData->mData.config.installedBd) return;

	Can_Group_Select(group);

	s_msg.data.channel = ch;
	s_msg.data.command = CAN_COMMAND_RUN;
	s_msg.data.run_range = run;
	s_msg.data.reserved = CAN_DUMMY_PACKET;
	s_msg.data.ref[0] = CAN_DUMMY_PACKET;
	s_msg.data.ref[1] = CAN_DUMMY_PACKET;
	s_msg.data.ref[2] = CAN_DUMMY_PACKET;
	s_msg.data.ref[3] = CAN_DUMMY_PACKET;

	can_transmit(bd, group, (U_CAN_FULL_SEND_MSG *)&s_msg); 
}

void bCan_Each_Ref_Cmd_Out(char type, int bd, char ch, int ref){
	int tmp_Ref, group=0;
	U_CAN_EACH_SEND_MSG s_msg;	

	if(bd < 0 || bd >= myData->mData.config.installedBd) return;

	Can_Group_Select(group);

	s_msg.data.channel = ch;
	if(type == 0){//voltage
		tmp_Ref = ref / 100;
		s_msg.data.command = CAN_COMMAND_V_REF;
	}else{//current
		tmp_Ref = ref;
		s_msg.data.command = CAN_COMMAND_I_REF;
	}
	s_msg.data.reserved = CAN_DUMMY_PACKET;

	s_msg.data.ref[0] = BYTE0(tmp_Ref);
	s_msg.data.ref[1] = BYTE1(tmp_Ref);
	s_msg.data.ref[2] = BYTE2(tmp_Ref);
	s_msg.data.ref[3] = BYTE3(tmp_Ref);

	can_transmit(bd, group, (U_CAN_FULL_SEND_MSG *)&s_msg); 
}

void bCan_Each_Link_Out(int bd, char ch){
	int group=0;
	U_CAN_EACH_SEND_MSG s_msg;	

	if(bd < 0 || bd >= myData->mData.config.installedBd) return;

	Can_Group_Select(group);

	if(bd < 0 || bd >= myData->mData.config.installedBd) return;
	s_msg.data.channel = ch;
	s_msg.data.command = CAN_COMMAND_LINK;
	s_msg.data.run_range = CAN_DUMMY_PACKET;
	s_msg.data.reserved = CAN_DUMMY_PACKET;
	s_msg.data.ref[0] = CAN_DUMMY_PACKET;
	s_msg.data.ref[1] = CAN_DUMMY_PACKET;
	s_msg.data.ref[2] = CAN_DUMMY_PACKET;
	s_msg.data.ref[3] = CAN_DUMMY_PACKET;

	can_transmit(bd, group, (U_CAN_FULL_SEND_MSG *)&s_msg); 
}
#endif
*/

//full can msg function
void bCan_Full_Run_Range_Out(int bd, int ch, char range, char run){
	U_CAN_FULL_SEND_MSG s_msg;	
	int group=0;

	if(bd < 0 || bd >= myPs->config.installedBd) return;

	Can_Group_Select(group);

	s_msg.data.channel = (char)ch;
	s_msg.data.relay = run;
	s_msg.data.range = range;
	s_msg.data.command = CAN_COMMAND_RUN_RANGE;
	s_msg.data.v_ref[0] = CAN_DUMMY_PACKET;
	s_msg.data.v_ref[1] = CAN_DUMMY_PACKET;
	s_msg.data.i_ref[0] = CAN_DUMMY_PACKET;
	s_msg.data.i_ref[1] = CAN_DUMMY_PACKET;
	s_msg.data.i_ref[2] = CAN_DUMMY_PACKET;
	s_msg.data.i_ref[3] = CAN_DUMMY_PACKET;

	can_transmit(bd, group, &s_msg); 
}


void bCan_Full_Ref_Cmd_Out(char cmd, int bd, int ch, int v_ref, long i_ref){
	int tmp_iRef, tmp_vRef, group=0;
	U_CAN_FULL_SEND_MSG s_msg;	
	
	if(bd < 0 || bd >= myPs->config.installedBd) return;
	
	myCh = &(myData->bData[bd].cData[ch]);

//	if(cmd == CAN_COMMAND_RESET){
//		if(myCh->misc.CAN_PreCmd == cmd) return;
//	}
//	myCh->misc.CAN_PreCmd = cmd;

	Can_Group_Select(group);

	s_msg.data.channel = (char)ch;
	switch(cmd){
		case CAN_COMMAND_RESET:
			s_msg.data.relay = CAN_DUMMY_PACKET;
			s_msg.data.range = CAN_DUMMY_PACKET;
			s_msg.data.command = CAN_COMMAND_RESET;
			s_msg.data.v_ref[0] = 0;
			s_msg.data.v_ref[1] = 0;
			s_msg.data.i_ref[0] = 0;
			s_msg.data.i_ref[1] = 0;
			s_msg.data.i_ref[2] = 0;
			s_msg.data.i_ref[3] = 0;
			break;
		case CAN_COMMAND_CC:
			s_msg.data.relay = CAN_DUMMY_PACKET;
			s_msg.data.range = CAN_DUMMY_PACKET;
			s_msg.data.command = CAN_COMMAND_CC;
		//	tmp_vRef = v_ref / 100; //100uV 
			tmp_vRef = labs(v_ref / 1000); //1mV	200608 lyhw 
			s_msg.data.v_ref[0] = BYTE0(tmp_vRef);
			s_msg.data.v_ref[1] = BYTE1(tmp_vRef);
			if(v_ref < 0) 	s_msg.data.v_ref[1] |= 0x80;
			tmp_iRef = labs(i_ref); //1uA
			s_msg.data.i_ref[0] = BYTE0(tmp_iRef);
			s_msg.data.i_ref[1] = BYTE1(tmp_iRef);
			s_msg.data.i_ref[2] = BYTE2(tmp_iRef);
			s_msg.data.i_ref[3] = BYTE3(tmp_iRef);
			if(i_ref < 0)	s_msg.data.i_ref[3] |= 0x80;
			break;
		case CAN_COMMAND_CCCV:
			s_msg.data.relay = CAN_DUMMY_PACKET;
			s_msg.data.range = CAN_DUMMY_PACKET;
			s_msg.data.command = CAN_COMMAND_CCCV;
		//	tmp_vRef = v_ref / 100; //100uV 
			tmp_vRef = labs(v_ref / 1000); //1mV	200608 lyhw 
			s_msg.data.v_ref[0] = BYTE0(tmp_vRef);
			s_msg.data.v_ref[1] = BYTE1(tmp_vRef);
			if(v_ref < 0) 	s_msg.data.v_ref[1] |= 0x80;
			tmp_iRef = labs(i_ref); //1uA
			s_msg.data.i_ref[0] = BYTE0(tmp_iRef);
			s_msg.data.i_ref[1] = BYTE1(tmp_iRef);
			s_msg.data.i_ref[2] = BYTE2(tmp_iRef);
			s_msg.data.i_ref[3] = BYTE3(tmp_iRef);
			if(i_ref < 0)	s_msg.data.i_ref[3] |= 0x80;
			break;
		case CAN_COMMAND_CV:
			s_msg.data.relay = CAN_DUMMY_PACKET;
			s_msg.data.range = CAN_DUMMY_PACKET;
			s_msg.data.command = CAN_COMMAND_CV;
		//	tmp_vRef = v_ref / 100; //100uV
			tmp_vRef = labs(v_ref / 1000); //1mV	200608 lyhw 
			s_msg.data.v_ref[0] = BYTE0(tmp_vRef);
			s_msg.data.v_ref[1] = BYTE1(tmp_vRef);
			if(v_ref < 0)	s_msg.data.v_ref[1] |= 0x80;
			tmp_iRef = labs(i_ref); //1uA
			s_msg.data.i_ref[0] = BYTE0(tmp_iRef);
			s_msg.data.i_ref[1] = BYTE1(tmp_iRef);
			s_msg.data.i_ref[2] = BYTE2(tmp_iRef);
			s_msg.data.i_ref[3] = BYTE3(tmp_iRef);
			if(i_ref < 0)	s_msg.data.i_ref[3] |= 0x80;
			break;
		case CAN_COMMAND_CP:	//200608
			s_msg.data.relay = CAN_DUMMY_PACKET;
			s_msg.data.range = CAN_DUMMY_PACKET;
			s_msg.data.command = CAN_COMMAND_CV;
			tmp_vRef = labs(v_ref / 1000); //1mV	200608 lyhw 
			s_msg.data.v_ref[0] = BYTE0(tmp_vRef);
			s_msg.data.v_ref[1] = BYTE1(tmp_vRef);
			if(v_ref < 0)	s_msg.data.v_ref[1] |= 0x80;
			tmp_iRef = labs(i_ref); //1uA
			s_msg.data.i_ref[0] = BYTE0(tmp_iRef);
			s_msg.data.i_ref[1] = BYTE1(tmp_iRef);
			s_msg.data.i_ref[2] = BYTE2(tmp_iRef);
			s_msg.data.i_ref[3] = BYTE3(tmp_iRef);
			if(i_ref < 0)	s_msg.data.i_ref[3] |= 0x80;
			break;
		case CAN_COMMAND_CR:
			s_msg.data.relay = CAN_DUMMY_PACKET;
			s_msg.data.range = CAN_DUMMY_PACKET;
			s_msg.data.command = CAN_COMMAND_CV;
			tmp_vRef = labs(v_ref / 1000); //1mV	200608 lyhw 
			s_msg.data.v_ref[0] = BYTE0(tmp_vRef);
			s_msg.data.v_ref[1] = BYTE1(tmp_vRef);
			tmp_iRef = labs(i_ref); //1uA
			s_msg.data.i_ref[0] = BYTE0(tmp_iRef);
			s_msg.data.i_ref[1] = BYTE1(tmp_iRef);
			s_msg.data.i_ref[2] = BYTE2(tmp_iRef);
			s_msg.data.i_ref[3] = BYTE3(tmp_iRef);
			break;
		default:
			break;
	}
	can_transmit(bd, 0, &s_msg); 
}


void can_transmit(int s_bd, int group, U_CAN_FULL_SEND_MSG *msg)
{
	int can_port = 0, ref_idx, i, canInsDevNum;
	S_RT_CAN_MSG can_msg;

	//210108
	if(myPs->signal[M_SIG_INV_POWER_CAN] < P10) return;
	if(s_bd < 0 || s_bd >= myPs->config.installedBd) return;
	
	ref_idx = myData->CAN.config.canInBd[s_bd]-1;
	if(ref_idx < 0){ 
		if(myData->CAN.config.CmdSendLog[can_port] == P1)
			printk("[%d]bd Can reference position value is fail\n", s_bd);		
		return;
	}
//	canInsDevNum = myData->CAN.config.canInsDevNum[ref_idx];
	canInsDevNum = myData->CAN.config.canInsDevNum[0]; //KHKW
	if(s_bd < canInsDevNum){
		can_msg.id = CAN_SEND_BASE_ID + s_bd; //100d + bd
	}else{
		can_msg.id = CAN_SEND_BASE_ID + (s_bd - canInsDevNum); //100d + bd
	}
	can_msg.type = CAN_MSGTYPE_STANDARD;
	can_msg.length = CAN_MSG_LENGTH;

	for(i = 0; i < CAN_MSG_LENGTH; i++){
		can_msg.data[i] = msg->byte[i];
	}
	
    //find phyical can port position
	can_port = myData->CAN.config.canPort[ref_idx] - 1;//can_port is start to 0 
	if(can_port < 0){
		if(myData->CAN.config.CmdSendLog[can_port] == P1)
			printk("Can Main Physical Port invalid\n");		
		return;
	}

	if(myData->CAN.config.CmdSendLog[can_port] == P1){
		printk("<Send_msg>can[%d]port [%02d]bd : %02x %02x %02x %02x %02x %02x %02x %02x\n",
			   	can_port, s_bd,	can_msg.data[0], can_msg.data[1], can_msg.data[2],
			   	can_msg.data[3], can_msg.data[4], can_msg.data[5], can_msg.data[6],
			   	can_msg.data[7]);
	}	
	
	rt_can_write(can_port, (char *)&can_msg);
}

void can_main_msg_parsing(unsigned int can_id, int group, char*ptr){
	int bd, tmp_read_val, ref_idx;
	unsigned char temp_save_idx = 0;
	unsigned char can_port = 0, ch, i;
	U_CAN_RECEIVE_MSG r_msg;
	U_CAN_AD_CHANGE t_ad;
	bd = (can_id - 100);

	if(bd < 0 || bd >= myPs->config.installedBd) return;

	for(i = 0; i < CAN_MSG_LENGTH; i++){
		r_msg.byte[i] = ptr[i];
	}	

	//find can reference positoin '0'base
	ref_idx = myData->CAN.config.canInBd[bd] - 1;
	//find physical can_port is start to 0 
	can_port = myData->CAN.config.canPort[ref_idx] - 1;

//		ch = r_msg.data.channel; //test
	if(myData->CAN.config.CmdRcvLog[can_port] == P1){
		printk("< Rcv_msg>can[%d]port [%02d]bd : %02x %02x %02x %02x %02x %02x %02x %02x\n",
			   	can_port, bd, r_msg.byte[0], r_msg.byte[1], r_msg.byte[2],
			   	r_msg.byte[3], r_msg.byte[4], r_msg.byte[5], r_msg.byte[6],
			   	r_msg.byte[7]);
	}
	
	if(r_msg.data.channel == CAN_RCV_COMMAND_TEMP){//Main BD Temp Data Read
		temp_save_idx = can_id - 1;
		//TEMP_HIGH_BYTE(r_msg.byte[2], tmp_temp);
		//TEMP_LOW_BYTE(r_msg.byte[1], tmp_temp);
//		myData->gData[group].temp[temp_save_idx].temp_BD = r_msg.byte[1] * 10000;//mV conversion
//		myData->gData[group].temp[temp_save_idx].temp_BD +=
//			 myData->gData[group].temp[temp_save_idx].temp_BD_Offset; //SBC_S2
//		myPs->temp[group].data[temp_save_idx].temp_BD = 
//			myData->gData[group].temp[temp_save_idx].temp_BD;
		myData->CAN.main[bd].comm.StateFlag = P1; //for communication error check	
	}else{//Ch Data Read
		//for communication error check	
		myData->CAN.main[bd].comm.StateFlag = P1; 
		ch = r_msg.data.channel;

		if(ch >= myPs->config.chPerBd) return;

		myCh = &(myData->bData[bd].cData[ch]);

		myCh->misc.can_read_update_flag = P1;//update flag is set
		myCh->misc.can_read_errCnt = 0;//clear error count
		myCh->misc.can_error = r_msg.data.errcode;
		myCh->misc.can_run_state = r_msg.data.run;
		myCh->misc.can_range_state = r_msg.data.range;
	
		//read voltage value
		t_ad.byte[0] =  r_msg.data.read_v[0];
		t_ad.byte[1] =  r_msg.data.read_v[1];
		t_ad.byte[2] =  r_msg.data.read_v[2];

		tmp_read_val = (t_ad.val & 0x00FFFFFF);
		
		if((t_ad.byte[2] & 0x80) == 0x0){//Positive value
			myCh->misc.can_read_v = tmp_read_val; //1uV
		}else{//negative value
			tmp_read_val = (tmp_read_val & 0x007FFFFF);
			myCh->misc.can_read_v = -(tmp_read_val);//make negative //1uV
		}

		//read current value
		t_ad.byte[0] =  r_msg.data.read_i[0];
		t_ad.byte[1] =  r_msg.data.read_i[1];
		t_ad.byte[2] =  r_msg.data.read_i[2];

		tmp_read_val = (t_ad.val & 0x00FFFFFF);

		if((t_ad.byte[2] & 0x80) == 0x0){//Positive value
			myCh->misc.can_read_i = tmp_read_val * 10; //10uA
		}else{//negative value
			tmp_read_val = (tmp_read_val & 0x007FFFFF);
			myCh->misc.can_read_i = -(tmp_read_val * 10);//make negative //10uA
		}
	}
}

void can_read_msg(int ref_idx, int group){
//Can Array is 0 to 7
	int rtn = 0, i, can_port = 0;
	S_RT_CAN_MSG can_msg;
    //find physical can port position
	//can_port is start to 0 
	can_port = myData->CAN.config.canPort[ref_idx] - 1;
	//no use can port
	if(can_port < 0) return;

	for(i = 0; i < CAN_READ_COUNT; i++){
		rtn = rt_can_read(can_port, (char*)&can_msg);
//		printk("called read msg rtn %d canPort %d\n", rtn, can_port);//sungdel
		if(rtn < 0) break; //No receive data
		if(myData->CAN.config.functionType[ref_idx] == CAN_FUNC_MAIN_COMM){
			if(can_port == 0){//0 ~ 31 MainBoard
				can_main_msg_parsing(can_msg.id, group, (char*)can_msg.data);
			}
			if(can_port == 1){//32 ~ 63 MainBoard
				can_main_msg_parsing(can_msg.id+8 , group, (char*)can_msg.data);
			}
		}else if(myData->CAN.config.functionType[ref_idx] == CAN_FUNC_INV_COMM){
			can_inv_msg_parsing(group, (char*)can_msg.data);
			//printk("called inv read msg\n");//sungdel
		}else if(myData->CAN.config.functionType[ref_idx] == CAN_FUNC_IO_COMM){
		//	printk("called IO read msg\n");//sungdel
			can_io_msg_parsing(can_msg.id, group, (char*)can_msg.data);
		}else{
			printk("Function [%d]Type invalid\n", 
				myData->CAN.config.functionType[ref_idx]);
		}
	}
}


//******Next Function is for Inverter CAN Communication*******//
void Can_Inv_Link_Check(int group)
{
	long diff = 0;
	unsigned char tmp_val = 0, inv_cmd = 0, inv, installedInv;	

	if(myData->AppControl.config.debugType != P0) return;
	if(myData->CAN.config.canPort[INV_CAN_PORT] == 0) return;

	installedInv = myData->CAN.config.installedInverter;

	for(inv = 0; inv < installedInv; inv++){//make heareBeat packet
		if(myData->CAN.inverter[inv].comm.StateFlag != P1){//for communication error check
			diff = myPs->misc.timer_1sec - 
			myData->CAN.inverter[inv].comm.pingOutCnt; //	
			if(diff > myData->CAN.config.canCommTimeOut[INV_CAN_PORT]){
				if(myData->CAN.inverter[inv].signal[CAN_SIG_INV_CAN_COMM_ERROR] == P0){
					myData->CAN.inverter[inv].signal[CAN_SIG_INV_CAN_COMM_ERROR] = P1;

					myData->CAN.inverter[inv].code = M_FAIL_INV_CAN_COMM_ERROR;
					myPs->code
						= myData->CAN.inverter[inv].code;
					send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 32, inv);
					send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS, myPs->code, inv);
					//send inv fault signal
					myData->CAN.invsavefault[inv].code
						= myData->CAN.inverter[inv].faultFull.all; 
					myData->CAN.invsavefault[inv].vdc
						= myData->CAN.inverter[inv].vdc; 
				}
			}
		}else{//comm state is good
			myData->CAN.inverter[inv].comm.pingOutCnt = myPs->misc.timer_1sec;
			//for checking communication state 
			myData->CAN.inverter[inv].comm.StateFlag = P0;
			myData->CAN.inverter[inv].signal[CAN_SIG_INV_CAN_COMM_ERROR] = P0;
		}

		if(myData->CAN.inverter[inv].signal[CAN_SIG_INV_CAN_COMM_ERROR] != P0){
			//for next check comm error
			myData->CAN.inverter[inv].comm.pingOutCnt = myPs->misc.timer_1sec;
		}
	}
	
	for(inv = 0; inv < installedInv; inv++){//make heareBeat packet
		tmp_val = INV_CMD_STATE; 
		myData->CAN.inverter[inv].sendCmd = INV_CMD_STATE;//save last cmd
		inv_cmd |= (tmp_val << (inv * 2));
	}
	
	can_inv_send_cmd(inv_cmd, inv);
}

void can_inv_send_cmd(unsigned char cmd, int inv)
{
	int can_port = 0;
	S_RT_CAN_MSG can_msg;

	can_msg.id = CAN_INV_SEND_ID; //55 + n
//	can_msg.id = CAN_INV_SEND_ID + inv; //55 + n
	can_msg.type = CAN_MSGTYPE_STANDARD;
	can_msg.length = CAN_INV_MSG_LENGTH;

	can_msg.data[0] = CAN_SBC_ID;
	can_msg.data[1] = cmd;

	//can_port is start to 0 
	can_port = myData->CAN.config.canPort[INV_CAN_PORT] - 1;

	if(can_port < 0){
		if(myData->CAN.config.CmdSendLog[can_port] == P1)
			printk("Can Inv Physical Port invalid\n");		
		return;
	}

	if(myData->CAN.config.CmdSendLog[can_port] == P1){
		printk("<Send_msg>[%d]inverter_can[%d]port: %02x %02x\n",
			   	inv, can_port, can_msg.data[0], can_msg.data[1]);
	}
	rt_can_write(can_port, (char *)&can_msg);
}

void can_inv_msg_parsing(int group, char* ptr){
	int inv_num;
	unsigned short tmp_val = 0;
	unsigned long tmp_vdc = 0.0;
	unsigned char can_port = 0, i;
	U_CAN_INV_RECEIVE_MSG r_msg;

	for(i = 0; i < CAN_MSG_LENGTH; i++){
		r_msg.byte[i] = ptr[i];
	}	
	
	//can_port is start to 0 
	can_port = myData->CAN.config.canPort[INV_CAN_PORT] - 1;

	inv_num = r_msg.data.id - 1;
	if((inv_num >= MAX_INVERTER_NUM) || ( inv_num < 0)){
		printk("Inverter number fail %dnum receive\n", inv_num);
		return;
	}

	//for checking communication state 
	myData->CAN.inverter[inv_num].comm.StateFlag = P1;

	if(myData->CAN.config.CmdRcvLog[can_port] == P1){
		printk("< Rcv_msg>[%d]group_can[%d]port [%02d]inv : %02x %02x %02x %02x %02x %02x %02x %02x\n",
			   	group, can_port, inv_num, r_msg.byte[0], r_msg.byte[1], 
				r_msg.byte[2], r_msg.byte[3], r_msg.byte[4], 
				r_msg.byte[5], r_msg.byte[6], r_msg.byte[7]);
	}

	myData->CAN.inverter[inv_num].receiveCmd = r_msg.data.cmd;//receive command 
	myData->CAN.inverter[inv_num].state = r_msg.data.state;//receive state
	tmp_val = r_msg.data.fault[0];//high byte
	tmp_val = tmp_val << 8;//move high byte position
	tmp_val |= r_msg.data.fault[1];//high byte
	myData->CAN.inverter[inv_num].faultFull.all = tmp_val;//receive faultstate 

	tmp_vdc = r_msg.data.dc_volt[0];//read high byte
	tmp_vdc = tmp_vdc << 8;//move high byte position
	tmp_vdc |= r_msg.data.dc_volt[1];//low byte
	myData->CAN.inverter[inv_num].vdc = tmp_vdc * 100.0;//receive tmp_vdc(uV)
}


//******Next Function is for JIG IO CAN Communication*******//
void Can_Io_Link_Check(int group){

	int can_port = 0;
	long diff = 0.0;
	S_RT_CAN_MSG can_msg;

	can_msg.id = CAN_IO_SEND_ID; //100
	can_msg.type = CAN_MSGTYPE_STANDARD;
	can_msg.length = CAN_IO_MSG_LENGTH;

	can_msg.data[0] = CAN_IO_BOARD_ID;
	can_msg.data[1] = CAN_IO_OUT_CMD;
	can_msg.data[2] = myData->CAN.io.outputValue[0];
	can_msg.data[3] = myData->CAN.io.outputValue[1];
	can_msg.data[4] = CAN_DUMMY_PACKET;
	can_msg.data[5] = CAN_DUMMY_PACKET;
	can_msg.data[6] = CAN_DUMMY_PACKET;
	can_msg.data[7] = CAN_DUMMY_PACKET;

	can_port = myData->CAN.config.canPort[IO_CAN_PORT] - 1;//can_port is start to 0 

	if(can_port < 0){
		if(myData->CAN.config.CmdSendLog[can_port] == P1)
			printk("Can JigIO Physical Port invalid\n");		
		return;
	}

	if(myData->CAN.io.comm.StateFlag != P1){//for communication error check
		diff = myPs->misc.timer_1sec - 
		myData->CAN.io.comm.pingOutCnt; //	
		if(diff > myData->CAN.config.canCommTimeOut[IO_CAN_PORT]){
		//send CAN(io) communication fault 
			if(myData->CAN.io.signal[CAN_SIG_IO_CAN_COMM_ERROR] == P0){//Send only one
				myData->CAN.io.signal[CAN_SIG_IO_CAN_COMM_ERROR] = P1;
				myData->CAN.io.code = M_FAIL_IO_CAN_COMM_ERROR;
				myPs->code = myData->CAN.io.code;
				send_msg(MODULE_TO_APP, MSG_MODULE_APP_EMG, 33, 0);
				send_msg(MODULE_TO_MAIN, MSG_MODULE_MAIN_EMG_STATUS, myPs->code, 0);

			}
		}
	}else{
		myData->CAN.io.comm.pingOutCnt = myPs->misc.timer_1sec;
		myData->CAN.io.comm.StateFlag = P0;//for checking communication state 
		myData->CAN.io.signal[CAN_SIG_IO_CAN_COMM_ERROR] = P0;
	}

	if(myData->CAN.io.signal[CAN_SIG_IO_CAN_COMM_ERROR] != P0){//for next check comm error
		myData->CAN.io.comm.pingOutCnt =
			myPs->misc.timer_1sec;
	}

	if(myData->CAN.config.CmdSendLog[can_port] == P1){
		printk("<Send_msg>[%d]can[%d]port: %02x %02x %02x %02x %02x %02x %02x %02x\n",
			   	group, can_port, can_msg.data[0], can_msg.data[1], can_msg.data[2],
			   	can_msg.data[3], can_msg.data[4], can_msg.data[5], can_msg.data[6],
			   	can_msg.data[7]);
	}
	rt_can_write(can_port, (char *)&can_msg);
}

void can_io_msg_parsing(unsigned int can_id, int group, char* ptr){
	unsigned char i;
	int can_port;
	unsigned char temp_save_idx = 0;
	unsigned char tmp_data[CAN_MSG_LENGTH];

	for(i = 0; i < CAN_MSG_LENGTH; i++){
		tmp_data[i] = ptr[i];
	}	
	
	can_port = myData->CAN.config.canPort[IO_CAN_PORT] - 1;//can_port is start to 0 
	if(can_port < 0){
		if(myData->CAN.config.CmdSendLog[can_port] == P1)
			printk("Can JigIO Physical Port invalid\n");		
		return;
	}

	if(myData->CAN.config.CmdRcvLog[can_port] == P1){
		printk("< Rcv_msg>[%d]can[%d]port: %02x %02x %02x %02x %02x %02x %02x %02x\n",
			   	group, can_port, tmp_data[0], tmp_data[1], tmp_data[2],
			   	tmp_data[3], tmp_data[4], tmp_data[5], tmp_data[6],
			   	tmp_data[7]);
	}
	switch(tmp_data[1]){
		case CAN_IO_IN_CMD:
			myData->CAN.io.inputValue[0] = tmp_data[2];
			myData->CAN.io.inputValue[1] = tmp_data[3];
			myData->CAN.io.inputValue[2] = tmp_data[4];
			myData->CAN.io.inputValue[3] = tmp_data[5];
			myData->CAN.io.inputValue[4] = tmp_data[6];
			myData->CAN.io.comm.StateFlag = P1;//for checking communication state 
			break;
		case CAN_IO_TEMP1_CMD://JigTemp 1~3
		case CAN_IO_TEMP2_CMD://JigTemp 4~6
		case CAN_IO_TEMP3_CMD://JigTemp 7~9
		case CAN_IO_TEMP4_CMD://JigTemp 10~12
		case CAN_IO_TEMP5_CMD://JigTemp 13~15
		case CAN_IO_TEMP6_CMD://JigTemp 16~18
		case CAN_IO_TEMP7_CMD://JigTemp 19~21
		case CAN_IO_TEMP8_CMD://JigTemp 22~24
			temp_save_idx = ((tmp_data[1] - CAN_IO_TEMP1_CMD) * 3);
/*			temp_save_pos =  //find monitoring position
				myData->SubClient[group].config.JigTempMap[temp_save_idx];
			if(temp_save_pos >= 0){
				tmp_temp = 0;
				TEMP_HIGH_BYTE(tmp_data[3], tmp_temp);
				TEMP_LOW_BYTE(tmp_data[2], tmp_temp);
				myData->gData[group].temp[temp_save_pos].temp_Jig = tmp_temp * 1000;//mv Conversion
				myData->gData[group].temp[temp_save_pos].temp_Jig +=
					 myData->gData[group].temp[temp_save_pos].temp_Jig_Offset;	//SBC_S2
				myPs->temp[group].data[temp_save_pos].temp_Jig = 
					myData->gData[group].temp[temp_save_pos].temp_Jig;
			}

			temp_save_idx++;//next save position
			temp_save_pos =  //find monitoring position
				myData->SubClient[group].config.JigTempMap[temp_save_idx];
			if(temp_save_pos >= 0){
				tmp_temp = 0;
				TEMP_HIGH_BYTE(tmp_data[5], tmp_temp);
				TEMP_LOW_BYTE(tmp_data[4], tmp_temp);
				myData->gData[group].temp[temp_save_pos].temp_Jig = tmp_temp * 1000;//mv Conversion
				myData->gData[group].temp[temp_save_pos].temp_Jig +=
					 myData->gData[group].temp[temp_save_pos].temp_Jig_Offset; //SBC_S2
				myPs->temp[group].data[temp_save_pos].temp_Jig = 
					myData->gData[group].temp[temp_save_pos].temp_Jig;
			}

			temp_save_idx++;//next save position
			temp_save_pos =  //find monitoring position
				myData->SubClient[group].config.JigTempMap[temp_save_idx];
			if(temp_save_pos >= 0){
				tmp_temp = 0;
				TEMP_HIGH_BYTE(tmp_data[7], tmp_temp);
				TEMP_LOW_BYTE(tmp_data[6], tmp_temp);
				myData->gData[group].temp[temp_save_pos].temp_Jig = tmp_temp * 1000;//mv Conversion
				myData->gData[group].temp[temp_save_pos].temp_Jig +=
					 myData->gData[group].temp[temp_save_pos].temp_Jig_Offset; //SBC_S2
				myPs->temp[group].data[temp_save_pos].temp_Jig = 
					myData->gData[group].temp[temp_save_pos].temp_Jig;
			}*/
			break;
		case CAN_IO_OUT_CMD://no operation
			break;
		default:
			if(myData->CAN.config.CmdRcvLog[can_port] == P1){
				printk("[%d]group IO Receive CAN Unknown [%d]Cmd\n", group, tmp_data[1]);
			}
			break;
	}
}

