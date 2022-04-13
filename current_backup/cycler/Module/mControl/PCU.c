#include <asm/io.h>
#include <rtl_core.h>
#include <pthread.h>
#include <math.h>
#include "../../INC/datastore.h"
#include "PCU.h"
#include "ChannelControl.h"

extern S_SYSTEM_DATA    *myData;
extern S_MODULE_DATA	*myPs;
extern S_CH_DATA		*myCh;

unsigned char SREG8_read1(unsigned long	addr)
{
	unsigned char i;

	i=inb(addr);
	i = inb(0x608);
	return i;
}
unsigned char SREG8_read2(unsigned long	addr)
{
	unsigned char i;

	i=inb(addr);
	i = inb(0x609);
	return i;
}

unsigned short int SREG16_read(unsigned long	addr)
{
	unsigned short int i;
	U_ADDA	val;

	i=inb(addr); // data latch
	val.byte[1] = inb(0x608); // data high read
	val.byte[0] = inb(0x609); // data low read

	return val.val;
}

void SREG8_write(unsigned long	addr, unsigned char data)
{
	U_ADDA	val;
	val.val = data;
	outb(val.byte[0], 0x609); // data low write
	outb(data, addr);
}

void SREG16_write(unsigned long	addr, unsigned short int data)
{
	U_ADDA	val;
	
	val.val = data;
	outb(val.byte[1], 0x608); // data high latch
	outb(val.byte[0], 0x609); // data low write
	outb(val.byte[0], addr); // data low write
}

unsigned char ch_send_cmd(int bd, int ch, unsigned char	addr_data, unsigned char	cmd, long	par1, long	par2)
{
	int base_addr, addr_step, addr, addr1 = 0, ch_div;
	
	//par1 current, par2 voltage
	base_addr = myPs->addr.main[BASE_ADDR];
	addr_step = myPs->addr.main[ADDR_STEP];
	ch_div = myPs->pcu_config.portPerCh;

	myData->bData[bd].cData[ch].misc.send_pcu_seq_no++;

	if(myData->bData[bd].cData[ch].ChAttribute.chNo_master == P0 ){
		if(ch<32){
			addr1 = base_addr + addr_step * (ch / ch_div);
		}else{
			addr1 = base_addr + addr_step * ((ch-32) / ch_div);
		}
		myData->bData[bd].cData[ch-1].misc.send_pcu_seq_no 
			= myData->bData[bd].cData[ch].misc.send_pcu_seq_no;
	}else{
		if(ch<32){
			addr1 = base_addr + addr_step * (ch / ch_div);
		}else{
			addr1 = base_addr + addr_step * ((ch-32) / ch_div);
		}
	//	if(myData->bData[bd].cData[ch+1].ChAttribute.chNo_master == P0){
		if(myData->bData[bd].cData[ch].ChAttribute.opType == P1){
			if(cmd == CMD_PCU_RESET){
			}
		}else{
			addr_data = addr_data + (unsigned char)(ch % ch_div) + 0x01;
		}
	}
	
	addr = addr1 + CREG_CTRL_ADDR;
	SREG16_write(addr, (short int)addr_data);

	addr = addr1 + CREG_CTRL_PARA1H;
	SREG16_write(addr, (short int)(par1 >> 16));

	addr = addr1 + CREG_CTRL_PARA1L;
	SREG16_write(addr, (short int)par1);

	addr = addr1 + CREG_CTRL_PARA2H;
	SREG16_write(addr, (short int)(par2 >> 16));

	addr = addr1 + CREG_CTRL_PARA2L;
	SREG16_write(addr, (short int)(par2));	
	
	addr = addr1 + CREG_CTRL_DUTIH;
	SREG16_write(addr, myData->bData[bd].cData[ch].misc.send_pcu_seq_no);
	
	addr = addr1 + CREG_CTRL_CMD;
	SREG16_write(addr, (short int)cmd);

	return 0;
}

unsigned char ch_mode_set(int bd, int ch, unsigned char type, unsigned char	mode, long	par1, long	par2)
{
	switch(mode){
		case CC:
			if(type == STEP_DISCHARGE){
				if(par2 >= 0) par2 *= -1;
			}
		//	if(myData->bData[bd].cData[ch].ChAttribute.chNo_master != P0)
			ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, CMD_PCU_MODE_CC, par2, par1); //par2: current
			break;
		case CV:
			if(type == STEP_DISCHARGE){
				if(par1 >= 0) par1 *= -1;
			}	
			ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, CMD_PCU_MODE_CV, par1, 0); //par1: voltage V >= 0: charge, V < 0: discharge
			break;
		case CCCV:
			ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, CMD_PCU_MODE_CCCV, par2, par1); // par1: voltage, par2: current  current >=0: charge, current < 0: discharge
			break;
		case CR:
			if(type == STEP_DISCHARGE){
				if(par1 >= 0) par1 *= -1;
			}
			ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, CMD_PCU_MODE_CR, par1, 0); // par1:resistance, resistance >= 0: charge, resistance < 0: discharge
			break;
		case CP:
			if(type == STEP_DISCHARGE){
				if(par1 >= 0) par1 *= -1;
			}
			ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, CMD_PCU_MODE_CP, par1, par2); //par1: power  Power >= 0: charge, Power < 0: discharge
			break;
		case CPCV:
			if(type == STEP_DISCHARGE){
				if(par1 >= 0) par1 *= -1;
			}
			ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, CMD_PCU_MODE_CPCV, par1, par2); //par1: power, par2: Voltage
			break;
		case CCP:
			if(type == STEP_DISCHARGE){
				if(par2 >= 0) par2 *= -1;
			}
			ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, CMD_PCU_MODE_CC, par2, par1); // par2: current, par1: time
			break;
		case CPP:
			if(type == STEP_DISCHARGE){
				if(par1 >= 0) par1 *= -1;
			}
			ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, CMD_PCU_MODE_CP, par1, par2); //par1: Power, par2: time
			break;
		case DC:
			ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, CMD_PCU_MODE_CC, par2, par1);
			break;
		case AC:
			break;
	}

	return 0;
}

void PCU_Relay_OnOff(int bd, int ch)
{
//	unsigned long relay_state;
	int base_addr, addr_step, addr1, addr, ch_div;

	myCh = &(myData->bData[bd].cData[ch]);
	
	base_addr = myPs->addr.main[BASE_ADDR];
	addr_step = myPs->addr.main[ADDR_STEP];
	ch_div = myPs->pcu_config.portPerCh;
    if(ch<32){
		addr1 = base_addr + addr_step * (ch / ch_div);
	}else{
		addr1 = base_addr + addr_step * ((ch-32) / ch_div);
	}
	addr = addr1 + CREG_CTRL_CCU1_STATE + (ch % ch_div);

//	relay_state = SREG8_read2(addr) & 0x02;
//	myCh->pcu_misc.Rcv_relay = relay_state;
	
	if(myData->bData[bd].cData[ch].signal[C_SIG_OUT_SWITCH_ON] == P1) {
		myData->bData[bd].cData[ch].signal[C_SIG_OUT_SWITCH_ON] = P3;
		myData->bData[bd].cData[ch].signal[C_SIG_OUT_SWITCH_OFF] = P0;
		ch_send_cmd(bd,ch, ADDR_PCU_CLASS_MULTICAST, CMD_RCU_RELAY_ONOFF, 1, 0);
	}else if(myData->bData[bd].cData[ch].signal[C_SIG_OUT_SWITCH_OFF] == P1) {
		myData->bData[bd].cData[ch].signal[C_SIG_OUT_SWITCH_OFF] = P3;
		myData->bData[bd].cData[ch].signal[C_SIG_OUT_SWITCH_ON] = P0;
		ch_send_cmd(bd,ch, ADDR_PCU_CLASS_MULTICAST, CMD_RCU_RELAY_ONOFF, 0, 0);
	}


}

void HVU_OnOff(int bd, int ch, int On)
{
	if(On == 1){
		ch_send_cmd(bd, ch, ADDR_HVU_CLASS_MULTICAST, CMD_PCU_HVU_ON, 0, 0);
	}else{
		ch_send_cmd(bd, ch, ADDR_HVU_CLASS_MULTICAST, CMD_PCU_RESET, 0, 0);
	}
}

void PCU_ParallelSwitch_OnOff(int bd, int ch)
{
//	unsigned long relay_state;
	int base_addr, addr_step, addr1, addr, ch_div;
	
	myCh = &(myData->bData[bd].cData[ch]);

	base_addr = myPs->addr.main[BASE_ADDR];
	addr_step = myPs->addr.main[ADDR_STEP];
	ch_div = myPs->pcu_config.portPerCh;

    if(ch<32){
		addr1 = base_addr + addr_step * (ch / ch_div);
	}else{
		addr1 = base_addr + addr_step * ((ch-32) / ch_div);
	}
	addr = addr1 + CREG_CTRL_CCU1_STATE + (ch % ch_div);

//	relay_state = SREG8_read2(addr) & 0x04;
//	myCh->pcu_misc.Rcv_parallel_relay = relay_state;

	if(myData->bData[bd].cData[ch].signal[C_SIG_PARALLEL_SWITCH_ON] == P1) {
		myData->bData[bd].cData[ch].signal[C_SIG_PARALLEL_SWITCH_ON] = P3;
		myData->bData[bd].cData[ch].signal[C_SIG_PARALLEL_SWITCH_OFF] = P0;
		ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, CMD_RCU_PARALLEL_ONOFF, 1, 0);
	}else if(myData->bData[bd].cData[ch].signal[C_SIG_PARALLEL_SWITCH_OFF] == P1) {
		myData->bData[bd].cData[ch].signal[C_SIG_PARALLEL_SWITCH_ON] = P0;
		myData->bData[bd].cData[ch].signal[C_SIG_PARALLEL_SWITCH_OFF] = P3;
		ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, CMD_RCU_PARALLEL_ONOFF, 0, 0);
	}
}

void PCU_Inverter_OnOff(int bd, int ch)
{	//180615 add
	if(myData->bData[bd].cData[ch].signal[C_SIG_DIGITAL_INV_STOP] == P1){
		myData->bData[bd].cData[ch].signal[C_SIG_DIGITAL_INV_STOP] = P0;
		ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, 
										CMD_RCU_INVERTER_ONOFF, 0, 0);
	}else if(myData->bData[bd].cData[ch].signal[C_SIG_DIGITAL_INV_RUN] == P1){
		myData->bData[bd].cData[ch].signal[C_SIG_DIGITAL_INV_RUN] = P0;
		ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, 
										CMD_RCU_INVERTER_ONOFF, 1, 0);
	}else if(myData->bData[bd].cData[ch].signal[C_SIG_DIGITAL_INV_RESET] == P1){
		myData->bData[bd].cData[ch].signal[C_SIG_DIGITAL_INV_RESET] = P0;
		ch_send_cmd(bd, ch, ADDR_PCU_CLASS_MULTICAST, 
										CMD_RCU_INVERTER_ONOFF, 2, 0);
	}
}
