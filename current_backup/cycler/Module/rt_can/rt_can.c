//rt_can
//RT-Linux kernel module for CAN.

#include <linux/config.h>
#include <linux/errno.h>
#include <linux/ioport.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/version.h>

#include <asm/system.h>
#include <asm/io.h>

#include <rtl_conf.h>
#include <rtl_core.h>
#include <rtl_sync.h>
#include <pthread.h>

#include "sja1000.h"
#include "rt_can.h"
#include "rt_canP.h"
#include "../../INC/SysConfig.h"

//used=0 - port and irq setting by rt_can_set_param.
//	If you want to work like a standard rt_can you can set used=1.

#if SYSTEM_TYPE == 0 && CYCLER_TYPE == CAN_CYC	//SBC
struct rt_can_struct rt_can_table[RT_CAN_CNT] =
{ //sbc : a5362 //vmware 9 , sbc 5
  //{magic, baud_base, port, irq, flag, isr, mode, used}
    {0, RT_CAN_BASE_BAUD, 0x890, 5, RT_CAN_STD_FLAG, rt_can_share_isr, 0, 1},
    {0, RT_CAN_BASE_BAUD, 0x898, 5, RT_CAN_STD_FLAG, rt_can_share_isr, 0, 1},
    {0, RT_CAN_BASE_BAUD, 0x8A0, 5, RT_CAN_STD_FLAG, rt_can_share_isr, 0, 1},
    {0, RT_CAN_BASE_BAUD, 0x8A8, 5, RT_CAN_STD_FLAG, rt_can_share_isr, 0, 1},
    {0, RT_CAN_BASE_BAUD, 0x8B0, 5, RT_CAN_STD_FLAG, rt_can_share_isr, 0, 0},
    {0, RT_CAN_BASE_BAUD, 0x8B8, 5, RT_CAN_STD_FLAG, rt_can_share_isr, 0, 0},
    {0, RT_CAN_BASE_BAUD, 0x8C0, 5, RT_CAN_STD_FLAG, rt_can_share_isr, 0, 0},
    {0, RT_CAN_BASE_BAUD, 0x8C8, 5, RT_CAN_STD_FLAG, rt_can_share_isr, 0, 0}
};
#else	//VMware
struct rt_can_struct rt_can_table[RT_CAN_CNT] =
{ //sbc : a5362 //vmware 9 , sbc 5
  //{magic, baud_base, port, irq, flag, isr, mode, used}
    {0, RT_CAN_BASE_BAUD, 0x890, 9, RT_CAN_STD_FLAG, rt_can_share_isr, 0, 1},
    {0, RT_CAN_BASE_BAUD, 0x898, 9, RT_CAN_STD_FLAG, rt_can_share_isr, 0, 1},
    {0, RT_CAN_BASE_BAUD, 0x8A0, 9, RT_CAN_STD_FLAG, rt_can_share_isr, 0, 1},
    {0, RT_CAN_BASE_BAUD, 0x8A8, 9, RT_CAN_STD_FLAG, rt_can_share_isr, 0, 1},
    {0, RT_CAN_BASE_BAUD, 0x8B0, 9, RT_CAN_STD_FLAG, rt_can_share_isr, 0, 0},
    {0, RT_CAN_BASE_BAUD, 0x8B8, 9, RT_CAN_STD_FLAG, rt_can_share_isr, 0, 0},
    {0, RT_CAN_BASE_BAUD, 0x8C0, 9, RT_CAN_STD_FLAG, rt_can_share_isr, 0, 0},
    {0, RT_CAN_BASE_BAUD, 0x8C8, 9, RT_CAN_STD_FLAG, rt_can_share_isr, 0, 0}
};
#endif

int rt_can_write(int can, const char *msg)
{
	int idx;
	unsigned int base, tmp;//, mask;
	long state = 0;
	struct rt_can_struct *p;

    if(can >= RT_CAN_CNT) return -1;

	p = &(rt_can_table[can]);

	if(p->used <= 0) return -2;

	rt_can_irq_off(state);

	base = RT_CAN_BASE_ADDR;
	outb((unsigned char)(can + 1), base + RT_CAN_PORT_SELECT);

/*	tmp = 0x01;
	tmp = tmp << can;
	mask = inb(base + RT_CAN_PORT_IRQ_MASK);
	if((mask & tmp) == 0) {
		rt_can_irq_on(state);
		return -3;
	}*/

	tmp = 0;
	if(p->tx_msg_count >= RT_CAN_TX_MSG_BUF_SIZE) {
		p->tx_msg_error++;
		if(p->tx_msg_error < 10) {
			return -4;
		} else {
			tmp = 1;
		}
	}

	if(tmp == 1) {
		p->tx_msg_error = 0;
		p->tx_msg_count = 0;
		p->tx_msg_read_idx = p->tx_msg_write_idx;
	}

	idx = p->tx_msg_write_idx;
	idx++;
	if(idx >= RT_CAN_TX_MSG_BUF_SIZE) idx = 0;

	memcpy((char *)&p->tx_buf[idx], msg, sizeof(struct rt_can_msg));

	p->tx_msg_write_idx = idx;
	p->tx_msg_count++;

	tmp = inb(base + CHIPSTATUS) & TRANS_BUFFER_STATUS;

	if((p->tx_msg_count == 1) && (tmp != 0)) {

		tmp = sja1000_write(can, p);
	}

    rt_can_irq_on(state);
	return 0;
}

int rt_can_read(int can, char *ptr)
{
	//unsigned char tmp, mask;
	int idx;
	unsigned int base;
	long state = 0;
	struct rt_can_struct *p;
    if(can >= RT_CAN_CNT) return -1;
	p = &(rt_can_table[can]);

	if(p->used <= 0) return -2;
	if(p->rx_msg_count <= 0) return -3;

	if(p->rx_msg_write_idx == p->rx_msg_read_idx) return -4;

	rt_can_irq_off(state);

	base = RT_CAN_BASE_ADDR;
	outb((unsigned char)(can + 1), base + RT_CAN_PORT_SELECT);

/*	tmp = 0x01;
	tmp = tmp << can;
	mask = inb(base + RT_CAN_PORT_IRQ_MASK);
	if((mask & tmp) == 0) {
		rt_can_irq_on(state);
		return -5;
	}*/

	p->rx_msg_read_idx++;
	if(p->rx_msg_read_idx >= RT_CAN_RX_MSG_BUF_SIZE) p->rx_msg_read_idx = 0;

	idx = p->rx_msg_read_idx;
	memcpy(ptr, (char *)&p->rx_buf[idx], sizeof(struct rt_can_msg));
	p->rx_msg_count--;

    rt_can_irq_on(state);

    return p->rx_msg_count;
}

// read CAN-data from chip, supposed a message is available
int sja1000_read(int can, struct rt_can_struct *p) 
{
	unsigned char tmp, rxfi, ucLen;
	int i=MAX_MESSAGES_PER_INTERRUPT, j, idx;
	unsigned int base, id;
	struct rt_can_msg tmp_msg;
  
	base = RT_CAN_BASE_ADDR;

	do {
		unsigned char ucReadoutBase;
		int k, nRtrFrame;

		rxfi = inb(base + RECEIVE_FRAME_BASE);
    
		ucLen = rxfi & BUFFER_DLC_MASK;    
		if(ucLen > 8) ucLen = 8;
				
		nRtrFrame = rxfi & BUFFER_RTR;
  
		if(nRtrFrame) tmp_msg.type = MSGTYPE_RTR;
		else tmp_msg.type = MSGTYPE_STANDARD;
			
		j = 0;
		if((p->type == 1) || ((rxfi & BUFFER_EFF) != 0)) {
			tmp_msg.type |= MSGTYPE_EXTENDED;
  
			//#ifdef __LITTLE_ENDIAN
				tmp = inb(base + RECEIVE_FRAME_BASE + 1);
				id = (unsigned int)tmp << 24;
				tmp = inb(base + RECEIVE_FRAME_BASE + 2);
				id |= ((unsigned int)tmp << 16);
				tmp = inb(base + RECEIVE_FRAME_BASE + 3);
				id |= ((unsigned int)tmp << 8);
				tmp = inb(base + RECEIVE_FRAME_BASE + 4);
				id |= (unsigned int)tmp;
				id >>= 3;
			//#else
				//local.ucID[0] = dev->readreg(dev, RECEIVE_FRAME_BASE + 1);
				//local.ucID[1] = dev->readreg(dev, RECEIVE_FRAME_BASE + 2);
				//local.ucID[2] = dev->readreg(dev, RECEIVE_FRAME_BASE + 3);
				//local.ucID[3] = dev->readreg(dev, RECEIVE_FRAME_BASE + 4);
			//#endif
  
			ucReadoutBase = RECEIVE_FRAME_BASE + 5;
		} else {
			//#ifdef __LITTLE_ENDIAN
				tmp = inb(base + RECEIVE_FRAME_BASE + 1);
				id = (unsigned int)tmp << 3;
				tmp = inb(base + RECEIVE_FRAME_BASE + 2);
				id |= ((unsigned int)tmp >> 5);
			//#else
				//local.ucID[0] = dev->readreg(dev, RECEIVE_FRAME_BASE + 1);
				//local.ucID[1] = dev->readreg(dev, RECEIVE_FRAME_BASE + 2);
			//#endif
  
			ucReadoutBase = RECEIVE_FRAME_BASE + 3;
  		}
		tmp_msg.id = id;

		k = ucLen;
		if(nRtrFrame) {
			//only for beauty, replace useless telegram content with zeros
			while(k--) tmp_msg.data[j++] = 0;
		} else {
			while(k--) {
				tmp = inb(base + ucReadoutBase + j);
				tmp_msg.data[j] = tmp;
				j++;
			}
		}

		tmp_msg.time = gethrtime(); //kjg_120619

		//step forward in fifo
		if(p->rx_msg_count >= RT_CAN_RX_MSG_BUF_SIZE) return -1;

		idx = p->rx_msg_write_idx;
		idx++;
		if(idx >= RT_CAN_RX_MSG_BUF_SIZE) idx = 0;

		p->rx_buf[idx].id = tmp_msg.id;
		p->rx_buf[idx].type = tmp_msg.type;
		

		for(j=0; j < 8; j++) p->rx_buf[idx].data[j] = tmp_msg.data[j];
		p->rx_buf[idx].time = tmp_msg.time; //kjg_120619

		p->rx_msg_write_idx = idx;
		p->rx_msg_count++;
	   

//		printk("%d can cnt %d id%x data %x %x %x %x %x %x %x %x  read called\n", can,p->rx_msg_count,  tmp_msg.id,
//						((tmp_msg.data[0] >> 4) & 0xF) ,tmp_msg.data[1],tmp_msg.data[2],tmp_msg.data[3],
//						tmp_msg.data[4],tmp_msg.data[5],tmp_msg.data[6],tmp_msg.data[7]);//sungdel
	
		outb(RELEASE_RECEIVE_BUFFER, base + COMMAND);
		udelay(1); //give time to settle
	} while((inb(base + CHIPSTATUS) & RECEIVE_BUFFER_STATUS) && (i--));
      
	return 0;
}

int sja1000_write(int can, struct rt_can_struct *p) 
{
	unsigned char txfi, ucLen, ucWriteoutBase;
	int	i=0, nRtrFrame, idx;
	unsigned int base, id;
  
	base = RT_CAN_BASE_ADDR;

	if(p->tx_msg_count == 0){
		 return -1;
	}

	if(p->tx_msg_write_idx == p->tx_msg_read_idx){
		 return -2;
	}

	p->tx_msg_read_idx++;
	if(p->tx_msg_read_idx >= RT_CAN_TX_MSG_BUF_SIZE)
		p->tx_msg_read_idx = 0;
	idx = p->tx_msg_read_idx;

	p->tx_msg_count--;
	id = p->tx_buf[idx].id;
	txfi = ucLen = p->tx_buf[idx].length;  
	nRtrFrame = p->tx_buf[idx].type & MSGTYPE_RTR;  
  
	if((p->tx_buf[idx].type & MSGTYPE_EXTENDED) || (p->type == 1)) {
    	txfi |= BUFFER_EFF;
		id <<= 3;

    	//#ifdef __LITTLE_ENDIAN
		outb((unsigned char)((id & 0xFF000000) >> 24),
			base + TRANSMIT_FRAME_BASE + 1);
		outb((unsigned char)((id & 0x00FF0000) >> 16),
			base + TRANSMIT_FRAME_BASE + 2);
		outb((unsigned char)((id & 0x0000FF00) >> 8),
			base + TRANSMIT_FRAME_BASE + 3);
		outb((unsigned char)(id & 0x000000FF),
			base + TRANSMIT_FRAME_BASE + 4);
    	/*#else
		outb(id[0], base + TRANSMIT_FRAME_BASE + 1);
		outb(id[1], base + TRANSMIT_FRAME_BASE + 2);
		outb(id[2], base + TRANSMIT_FRAME_BASE + 3);
		outb(id[3], base + TRANSMIT_FRAME_BASE + 4);
    	#endif*/
  
		ucWriteoutBase = TRANSMIT_FRAME_BASE + 5;
	} else {
    	id <<= 5;
  
	    //#ifdef __LITTLE_ENDIAN
		outb((unsigned char)((id & 0xFF00) >> 8),
			base + TRANSMIT_FRAME_BASE + 1);
		outb((unsigned char)(id & 0xFF),
			base + TRANSMIT_FRAME_BASE + 2);
	    /*#else
		outb(id[2], base + TRANSMIT_FRAME_BASE + 1);
		outb(id[3], base + TRANSMIT_FRAME_BASE + 2);
	    #endif*/
  
		ucWriteoutBase = TRANSMIT_FRAME_BASE + 3;
  }

	// add data only if it is a RTR frame
	if(nRtrFrame) txfi |= BUFFER_RTR;
	else {
		while(ucLen--) {
			outb(p->tx_buf[idx].data[i], base + ucWriteoutBase + i);
			i++;
		}
	}
  
	//finish message construction
	outb(txfi, base + TRANSMIT_FRAME_BASE);

	outb(TRANSMISSION_REQUEST, base + COMMAND);
	return 0;
}

//Registered interrupt handlers.
//These simply call the general interrupt handler for the current
//line to do the work.
static unsigned int rt_can_share_isr(unsigned int num, struct pt_regs *r)
{
	unsigned char data, tmp;
	int i, j;

	j = 0;
	while(0 != (data = inb(RT_CAN_BASE_ADDR + RT_CAN_PORT_IRQ))) {
		tmp = 0x01;
		for(i=0; i < RT_CAN_CNT; i++) {
			if((data & tmp) != 0x00) {
				rt_can_isr((unsigned int)i, NULL);
			}
			tmp = tmp << 1;
		}
		j++;
		if(j > 15) break;
	}
	return 0;
}

//Real interrupt handler.
//Called by the registered ISRs to do the actual work.
//@param can Port to use corresponding to internal numbering scheme.
unsigned int rt_can_isr(unsigned int can, struct pt_regs *r)
{
    char loop=10;
    unsigned char status, port_sel;
    int err;
    unsigned int base;
    struct rt_can_struct *p;

	p = &(rt_can_table[can]);

	base = RT_CAN_BASE_ADDR;
	port_sel = inb(base + RT_CAN_PORT_SELECT);
	outb((unsigned char)(can + 1), base + RT_CAN_PORT_SELECT);

	while((status = inb(base + INTERRUPT_STATUS)) && (loop--)) {
		if(status & DATA_OVERRUN_INTERRUPT) {
			p->status |= CAN_ERR_OVERRUN;
			p->error_count++;
			outb(CLEAR_DATA_OVERRUN, base + COMMAND);
		}

		if(status & RECEIVE_INTERRUPT) {
			if((err = sja1000_read(can, p))) {
				p->error = err;
				p->error_count++;
				p->status |= CAN_ERR_QOVERRUN;
				//throw away last message which was refused by fifo
				outb(RELEASE_RECEIVE_BUFFER, base + COMMAND);
			}
		}

		if(status & TRANSMIT_INTERRUPT) {
			if((err = sja1000_write(can, p))) {
				if(err == -ENODATA) {
				} else {
					p->error = err;
					p->error_count++;
					p->status |= CAN_ERR_QXMTFULL; //fatal error!!!
				}
			}
		}

		if(status & ERROR_PASSIV_INTERRUPT) {
			p->status |= CAN_ERR_BUSHEAVY;
			p->error_count++;
		}

		if(status & BUS_ERROR_INTERRUPT) {
			p->status |= CAN_ERR_BUSOFF;
			p->error_count++;
		}
    }

	outb(port_sel, base + RT_CAN_PORT_SELECT);

    rtl_hard_enable_irq(p->irq);
    return 0;
}

//switches the chip onto reset mode
int set_reset_mode(int can)
{
	unsigned char tmp, tmp2;
	unsigned int base;
	struct rt_can_struct *p;

	p = &(rt_can_table[can]);
	base = RT_CAN_BASE_ADDR;

	tmp2 = 0;
	tmp = (inb(base + MODE) & 0x1F);
	while((tmp != RESET_MODE) && tmp2 < 100) {
		outb(RESET_MODE, base + MODE);
		tmp2++;
		tmp = (inb(base + MODE) & 0x1F);
	}
	if(tmp != RESET_MODE) return -1;
	else return 0;
}

//switches the chip back from reset mode
int set_normal_mode(int can, unsigned char ucModifier)
{
	unsigned char tmp, tmp2;
	unsigned int base;
	struct rt_can_struct *p;

	p = &(rt_can_table[can]);
	base = RT_CAN_BASE_ADDR;

	tmp2 = 0;
	tmp = inb(base + MODE);
	while((tmp != ucModifier) && tmp2 < 100) {
		outb(ucModifier, base + MODE);
		tmp2++;
		tmp = (inb(base + MODE) & 0x1F);
	}

	if(tmp != ucModifier) return -1;
	else return 0;
}

// calculate BTR0BTR1 for odd bitrates
// Set communication parameters.
// param rate baud rate in Hz
// param clock frequency of sja1000 clock in Hz
// param sjw synchronization jump width (0-3) prescaled clock cycles
// param sampl_pt sample point in % (0-100) sets (TSEG1+2)/(TSEG1+TSEG2+3) ratio
// param flags fields BTR1_SAM, OCMODE, OCPOL, OCTP, OCTN, CLK_OFF, CBP
int sja1000_baud_rate(int rate, int flags, int set_sjw)
{
	int best_error = 1000000000;
	int error;
	int best_tseg=0, best_brp=0, best_rate=0, brp=0;
	int tseg=0, tseg1=0, tseg2=0;
	int clock = CLOCK_HZ / 2;
	u16 wBTR0BTR1;
	int sjw;
	int sampl_pt = 90;

	// some heuristic specials
	if (rate > ((1000000 + 500000) / 2)) sampl_pt = 75; //750000
	if (rate < ((12500 + 10000) / 2)) sampl_pt = 75; //11250
//	if (rate < ((100000 + 125000) / 2)) sjw = 1; //112500

	switch(set_sjw) { //kjg_100826
		case 0:
			sjw = 2;
			if(rate < ((100000 + 125000) / 2)) sjw = 1; //112500
			break;
		default:
			if(set_sjw < 0 || set_sjw > 4) set_sjw = 3;
			sjw = set_sjw - 1;
			break;
	}

	// tseg even = round down, odd = round up
	for (tseg = (0 + 0 + 2) * 2; tseg <= (MAX_TSEG2 + MAX_TSEG1 + 2) * 2 + 1;
		tseg++) {
		brp = clock / ((1 + tseg / 2) * rate) + tseg % 2;
		if((brp == 0) || (brp > 64)) continue;
		
		error = rate - clock / (brp * (1 + tseg / 2));
		if(error < 0) error = -error;
			
		if(error <= best_error) {
			best_error = error;
			best_tseg = tseg/2;
			best_brp = brp-1;
			best_rate = clock/(brp*(1+tseg/2));
		}
	}
	
	if(best_error && (rate / best_error < 10))
		return 0;
	
	tseg2 = best_tseg - (sampl_pt * (best_tseg + 1)) / 100;
	
	if(tseg2 < 0) tseg2 = 0;
		
	if(tseg2 > MAX_TSEG2) tseg2 = MAX_TSEG2;
		
	tseg1 = best_tseg - tseg2 - 2;
	
	if(tseg1 > MAX_TSEG1) {
		tseg1 = MAX_TSEG1;
		tseg2 = best_tseg-tseg1-2;
	}

	wBTR0BTR1 = ((sjw<<6 | best_brp) << 8)
		| (((flags & BTR1_SAM) != 0)<<7 | tseg2<<4 | tseg1);

	return wBTR0BTR1;	
}

int rt_can_set_param(int can, int baud, int sjw, int extended, int inc)
{
	unsigned char tmp, clkdivider, mask;
	unsigned short btr0btr1;
	int rtn;
	unsigned int base;
	long state;
	struct rt_can_struct *p;

	if(can >= RT_CAN_CNT) return -1;

	//if(inc == 0) MOD_INC_USE_COUNT;

	mask = 0;
	state = 0;
//	rt_can_irq_off(state); //kjg_120427
	p = &(rt_can_table[can]);

	base = RT_CAN_BASE_ADDR;
	outb((unsigned char)(can + 1), base + RT_CAN_PORT_SELECT);

	//kjg_120427
	tmp = 0x01;

	tmp = tmp << can;
	mask = inb(base + RT_CAN_PORT_IRQ_MASK);
	mask &= (~tmp); //IRQ_OFF
	outb(mask, base + RT_CAN_PORT_IRQ_MASK);

	outb(ABORT_TRANSMISSION, base + COMMAND);
	outb(0, base + INTERRUPT_ENABLE);
	outb(RELEASE_RECEIVE_BUFFER, base + COMMAND);

	clkdivider = PELICAN_SINGLE;
	p->type = extended;
	p->rx_msg_write_idx = 0;
	p->rx_msg_read_idx = 0;
	p->rx_msg_count = 0;
	p->tx_msg_write_idx = 0;
	p->tx_msg_read_idx = 0;
	p->tx_msg_count = 0;
	p->tx_msg_error = 0;
	p->error = 0; //init. last error code
	p->error_count = 0;

	rtn = set_reset_mode(can);
	outb(clkdivider, base + CLKDIVIDER);

	//configure output control registers
	outb(OUTPUT_CONTROL_SETUP, base + OUTPUT_CONTROL);

	//configure acceptance code registers
	outb(0, base + ACCEPTANCE_CODE_BASE + 0);
	outb(0, base + ACCEPTANCE_CODE_BASE + 1);
	outb(0, base + ACCEPTANCE_CODE_BASE + 2);
	outb(0, base + ACCEPTANCE_CODE_BASE + 3);

	//configure all acceptance mask registers to don't care
	outb(0xFF, base + ACCEPTANCE_MASK_BASE + 0);
	outb(0xFF, base + ACCEPTANCE_MASK_BASE + 1);
	outb(0xFF, base + ACCEPTANCE_MASK_BASE + 2);
	outb(0xFF, base + ACCEPTANCE_MASK_BASE + 3);

	//configure bus timing registers
	btr0btr1 = (unsigned short)sja1000_baud_rate(baud, 0, sjw);
	outb((unsigned char)((btr0btr1 >> 8) & 0xFF), base + TIMING0);
	outb((unsigned char)((btr0btr1) & 0xFF), base + TIMING1);

	//clear any pending interrupt
	tmp = inb(base + INTERRUPT_STATUS);

	rtn = set_normal_mode(can, NORMAL_MODE);
	
	outb(INTERRUPT_ENABLE_SETUP, base + INTERRUPT_ENABLE);

	p->used |= 0x02; //mark setup done

	//kjg_120427
	tmp = 0x01;
	tmp = tmp << can;
	mask = inb(base + RT_CAN_PORT_IRQ_MASK);
	mask |= tmp; //IRQ_ON
	outb(mask, base + RT_CAN_PORT_IRQ_MASK);

	outb(RELEASE_RECEIVE_BUFFER, base + COMMAND);

	//rt_can_irq_on(state); //kjg_120427
	return 0;
}

//Setup one port
//Calls from init_module + cleanup_module have baud == 0; in these
//cases we only do some cleanup.
//To allocate a port, give usefull setup parameter, to deallocate
//give negative baud.
//@param can        Number corresponding to internal port numbering scheme.
//                  This is esp. the index of the rt_can_table to use.
//@param baud       Data transmission rate to use [Byte/s].
//@return           -1 if error, 0 if all right
int rt_can_setup(int can, int baud, int sjw, unsigned char extended)
{
	unsigned char tmp, clkdivider;
	unsigned short btr0btr1;
	unsigned int base;
	struct rt_can_struct *p;

	if(can >= RT_CAN_CNT) return -1;

	p = &(rt_can_table[can]);
	
	if(0 == p->used) return -2;

	base = RT_CAN_BASE_ADDR;
	outb((unsigned char)(can + 1), base + RT_CAN_PORT_SELECT);

	clkdivider = PELICAN_SINGLE;
	p->error = 0; //init. last error code

	if(0 == baud) {
		p->rx_msg_write_idx = 0;
		p->rx_msg_read_idx = 0;
		p->rx_msg_count = 0;
		p->tx_msg_write_idx = 0;
		p->tx_msg_read_idx = 0;
		p->tx_msg_count = 0;
		p->tx_msg_error = 0;
		p->type = extended;

		//trace the clockdivider register to test for sja1000
		tmp = inb(base + CLKDIVIDER);
		if(tmp & 0x10) return -10;

		//until here, it's either a sja1000
		if(set_reset_mode(can)) return -11;

		outb(clkdivider, base + CLKDIVIDER); //switch to PeliCAN mode
		outb(0, base + INTERRUPT_ENABLE); //prospective clear all interrupt

		tmp = inb(base + CHIPSTATUS);
		if((tmp & 0x30) != 0x30) return -12;

		tmp = inb(base + INTERRUPT_STATUS);
		if(tmp & 0xFB) return -13;

		tmp = inb(base + RECEIVE_MSG_COUNTER);
		if(tmp) return -14;
	} else if(0 > baud) {
		MOD_DEC_USE_COUNT;

		//abort pending transmissions
		outb(ABORT_TRANSMISSION, base + COMMAND);

		//disable CAN interrupts and set chip in reset mode
		outb(0, base + INTERRUPT_ENABLE);
		set_reset_mode(can);
	} else {
		MOD_INC_USE_COUNT;

		p->type = extended;

		if(set_reset_mode(can)) return -30;
		outb(clkdivider, base + CLKDIVIDER);

		//configure acceptance code registers
		outb(0, base + ACCEPTANCE_CODE_BASE + 0);
		outb(0, base + ACCEPTANCE_CODE_BASE + 1);
		outb(0, base + ACCEPTANCE_CODE_BASE + 2);
		outb(0, base + ACCEPTANCE_CODE_BASE + 3);

		//configure all acceptance mask registers to don't care
		outb(0xFF, base + ACCEPTANCE_MASK_BASE + 0);
		outb(0xFF, base + ACCEPTANCE_MASK_BASE + 1);
		outb(0xFF, base + ACCEPTANCE_MASK_BASE + 2);
		outb(0xFF, base + ACCEPTANCE_MASK_BASE + 3);

		//configure bus timing registers
		btr0btr1 = (unsigned short)sja1000_baud_rate(baud, 0, sjw);
		outb((unsigned char)((btr0btr1 >> 8) & 0xFF), base + TIMING0);
		outb((unsigned char)((btr0btr1) & 0xFF), base + TIMING1);

		//configure output control registers
		outb(OUTPUT_CONTROL_SETUP, base + OUTPUT_CONTROL);

		//clear any pending interrupt
		tmp = inb(base + INTERRUPT_STATUS);

		//enter normal operating mode
		if(set_normal_mode(can, NORMAL_MODE)) return -31;

		//enable CAN interrupts
		outb(INTERRUPT_ENABLE_SETUP, base + INTERRUPT_ENABLE);

		p->used |= 0x02; //mark setup done
	}

	return 0;
}

//Initialization
//Request port memory and register ISRs, if we cannot get the memory
//of all ports, release all already requested ports and return an error.
//@return Success status, zero on success.
int init_module(void)
{
    int errorcode=0, i, j;
    struct rt_can_struct *p;

    for(i=0; i < RT_CAN_CNT; i++) {
		p = &(rt_can_table[i]);
		if(p->used > 0) {
		    if(-EBUSY == check_region(p->port, 8)) {
				errorcode = 1;
				break;
	    	}
		    request_region(p->port, 8, "rt_can");
		    rt_can_request_irq(p->irq, p->isr);
	    	rt_can_setup(i, 0, 0, 0);
		}
    }

	//rt_can_set_param(3, 500000, 0, 0, 0);//3can
	//rt_can_set_param(4, 500000, 0, 0, 0);//4can
			

    if(0 == errorcode) {
		printk(KERN_INFO
			"rt_can: CAN driver (version " VERSION ") sucessfully loaded.
		   	\n");
    } else {
		printk(KERN_WARNING
			"rt_can: cannot request all port regions,\nrt_can: giving up.\n");
		for(j=0; j < i; j++) {
		    p = &(rt_can_table[j]);
	    	if(0 < p->used) {
				rt_can_free_irq(p->irq);
				release_region(p->port, 8);
	    	}
		}
    }

    return errorcode;
}

//Cleanup
//Unregister ISR and releases memory for all ports
void cleanup_module(void)
{
    int i;
    struct rt_can_struct *p;

    for(i=0; i < RT_CAN_CNT; i++) {
		p = &(rt_can_table[i]);
		if(p->used > 0) {
		    rt_can_free_irq(p->irq);
		    rt_can_setup(i, 0, 0, 0);
	    	release_region(p->port, 8);
		}
    }
    printk(KERN_INFO "rt_can unloaded.\n");

    return;
}
