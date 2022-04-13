#ifndef RT_CAN_P_H
#define RT_CAN_P_H

static unsigned int rt_can_share_isr(unsigned int, struct pt_regs *);
static inline unsigned int rt_can_isr(unsigned int, struct pt_regs *);

spinlock_t rt_can_spinlock = SPIN_LOCK_UNLOCKED;

//internal convinience macros for interrupt handling
#define rt_can_irq_off(state)			rtl_critical(state)
#define rt_can_irq_on(state)			rtl_end_critical(state)
#define rt_can_request_irq(irq, isr)	rtl_request_global_irq(irq, isr); rtl_hard_enable_irq(irq)
#define rt_can_free_irq(irq)			rtl_free_global_irq(irq)

#define RT_CAN_NAME				"rt_can"
#define RT_CAN_BASE_BAUD 		500000
#define RT_CAN_STD_FLAG 		0
#define RT_CAN_RX_MSG_BUF_SIZE	1000
#define RT_CAN_TX_MSG_BUF_SIZE	500
#define RT_CAN_BASE_ADDR		0x800
#define RT_CAN_PORT_SELECT		0x80
#define RT_CAN_PORT_IRQ			0x81
#define RT_CAN_PORT_IRQ_MASK	0x82
#define RT_CAN_BUF_SIZE      	0x1000

//amount of free space on input buffer for buffer full error
#define RT_CAN_BUF_FULL 		20

//Number and descriptions of serial ports to manage.  You also need
//to create an ISR (rt_canN_isr()) for each port.
#define RT_CAN_CNT 				8

// error codes
#define CAN_ERR_OK             0x0000  // no error
#define CAN_ERR_XMTFULL        0x0001  // transmit buffer full
#define CAN_ERR_OVERRUN        0x0002  // overrun in receive buffer
#define CAN_ERR_BUSLIGHT       0x0004  // bus error, errorcounter limit reached
#define CAN_ERR_BUSHEAVY       0x0008  // bus error, errorcounter limit reached
#define CAN_ERR_BUSOFF         0x0010  // bus error, 'bus off' state entered
#define CAN_ERR_QRCVEMPTY      0x0020  // receive queue is empty
#define CAN_ERR_QOVERRUN       0x0040  // receive queue overrun
#define CAN_ERR_QXMTFULL       0x0080  // transmit queue full 
#define CAN_ERR_REGTEST        0x0100  // test of controller registers failed
#define CAN_ERR_NOVXD          0x0200  // Win95/98/ME only
#define CAN_ERR_RESOURCE       0x2000  // can't create resource
#define CAN_ERR_ILLPARAMTYPE   0x4000  // illegal parameter
#define CAN_ERR_ILLPARAMVAL    0x8000  // value out of range
#define CAN_ERRMASK_ILLHANDLE  0x1C00  // wrong handle, handle error

//MSGTYPE
#define MSGTYPE_STATUS		0x80	//used to mark pending status
#define MSGTYPE_EXTENDED	0x02	//declares a extended frame
#define MSGTYPE_RTR			0x01	//marks a remote frame
#define MSGTYPE_STANDARD	0x00	//marks a standard frame

struct rt_can_msg {
	unsigned int	id;
	unsigned char	type;
	unsigned char	length;
	unsigned char	reserved1[2];
	unsigned char	data[8];
	long long		time; //kjg_120619
};

//information about handled ports
struct rt_can_struct{
    int		magic;
    int		baud_base;
    int		port;
    int		irq;
    int		flag;
    unsigned int	(*isr)(unsigned int, struct pt_regs *);
    int		mode;
    int		used;
		//1 if port region must be requested by init_module (standard)
	    //0 if port region must be requested later by rt_can_set_param
    int error; //last error detected
	int	error_count;
	unsigned short status;
	unsigned short reserved1;
    int		type; //0:standard, 1:extedned

	int					rx_msg_write_idx;
	int					rx_msg_read_idx;
	int					rx_msg_count;
	int					tx_msg_write_idx;
	int					tx_msg_read_idx;
	int					tx_msg_count;
	int					tx_msg_error;
	struct rt_can_msg	rx_buf[RT_CAN_RX_MSG_BUF_SIZE];
	struct rt_can_msg	tx_buf[RT_CAN_TX_MSG_BUF_SIZE];
};

int sja1000_read(int, struct rt_can_struct *);
int sja1000_write(int, struct rt_can_struct *);
#endif
