#ifndef __CAN_DEF_H__
#define __CAN_DEF_H__

//**************MainBD Definition**************//
//CAN define
#define MAX_CAN_PORT						8
#define MAX_CAN_SIGNAL						16

//can msg id
#define CAN_SEND_BASE_ID	100
#define CAN_RECEIVE_BASE_ID	1

//can Function type
#define CAN_FUNC_MAIN_COMM	1
#define CAN_FUNC_INV_COMM	2
#define CAN_FUNC_IO_COMM	3

//can operation type
#define CAN_MAIN_LINK_CHECK	1
#define CAN_INV_LINK_CHECK	2
#define CAN_IO_LINK_CHECK	3
#define CAN_DATA_READ		4

//inv code
#define INV_CODE_NONE		0

//#ifdef MORNINGPOWER
	//can msg command
	#define CAN_COMMAND_RESET		0x0
	#define CAN_COMMAND_CC			0x1
	#define CAN_COMMAND_CCCV		0x2
	#define CAN_COMMAND_CV			0x3
//	#define CAN_COMMAND_CP			0x4
//	#define CAN_COMMAND_CR			0x5
	#define CAN_COMMAND_I_REF		0x4
	#define CAN_COMMAND_V_REF		0x5
	#define CAN_COMMAND_RANGE		0x6
	#define CAN_COMMAND_RUN			0x7
	#define CAN_COMMAND_LINK		0x8
	#define CAN_COMMAND_RUN_RANGE	0x9
	//can receive command
	#define CAN_RCV_COMMAND_TEMP	0xA
	#define CAN_COMMAND_CP			0xB	//201208
	#define CAN_COMMAND_CR			0xC
	#define CAN_START				0xFF
/*#else
	//can msg command
	#define CAN_COMMAND_RESET		0x0
	#define CAN_COMMAND_CC			0x1
	#define CAN_COMMAND_CCCV		0x2
	#define CAN_COMMAND_CV			0x3
//	#define CAN_COMMAND_CP			0x4
//	#define CAN_COMMAND_CR			0x5
	#define CAN_COMMAND_RUN_RANGE	0xA
	#define CAN_COMMAND_I_REF		0xB1
	#define CAN_COMMAND_V_REF		0xB2
	#define CAN_COMMAND_RANGE		0xB3
	#define CAN_COMMAND_RUN			0xB4
	#define CAN_COMMAND_LINK		0xF0

	//can receive command
	#define CAN_RCV_COMMAND_TEMP	0xF
#endif
*/

//can msg type
#define CAN_MSGTYPE_STANDARD		0x0
#define CAN_MSGTYPE_RTR				0x1
#define CAN_MSGTYPE_EXTENDED		0x2

//can msg length
#define CAN_MSG_LENGTH			8
#define CAN_READ_COUNT			100

//can macro definition
#define CAN_REF_TYPE_VOLTAGE	0x0
#define CAN_DUMMY_PACKET		0x0

//macro funtion
#define BYTE0(x)	((x)& 0xFF)
#define BYTE1(x)	((x >> 8) & 0xFF)
#define BYTE2(x)	((x >> 16) & 0xFF)
#define BYTE3(x)	((x >> 24) & 0xFF)

//define mainBD can error code
#define CAN_MAIN_CH_NONE_ERROR		0
#define CAN_MAIN_CH_VIN_ERROR		1
#define CAN_MAIN_CH_PV_ERROR		5

//define mainBD can error code
//210126 lyhw
#define CAN_MAIN_CH_HARD_ERR		2
#define CAN_MAIN_CH_LOU_ERR			3
#define CAN_MAIN_CH_OTP_ERR			4
#define CAN_MAIN_CH_VOL_OUT_ERR		6
#define CAN_MAIN_CH_BUS_ERR			7

//**************Inverter Definition**************//
#define MAX_INVERTER_NUM   32
#define CAN_INV_SEND_ID    55	
#define CAN_INV_MSG_LENGTH 2		
#define CAN_SBC_ID		   0xBC		
#define INV_CAN_PORT	   2		
//inverter cmd definition
#define INV_CMD_STOP	   0x0
#define INV_CMD_RUN	   	   0x1
#define INV_CMD_RESET	   0x2
#define INV_CMD_STATE	   0x3

//**************JigIO Definition**************//
#define CAN_IO_SEND_ID    	200
#define CAN_IO_RECEIVE_ID 	100
#define CAN_IO_BOARD_ID 	0
#define IO_CAN_PORT	    3		
#define CAN_IO_MSG_LENGTH 8		

//io cmd definition
#define CAN_IO_IN_CMD	   0xC0		
#define CAN_IO_OUT_CMD	   0xC1		
#define CAN_IO_TEMP1_CMD   0xC2		
#define CAN_IO_TEMP2_CMD   0xC3		
#define CAN_IO_TEMP3_CMD   0xC4		
#define CAN_IO_TEMP4_CMD   0xC5		
#define CAN_IO_TEMP5_CMD   0xC6		
#define CAN_IO_TEMP6_CMD   0xC7		
#define CAN_IO_TEMP7_CMD   0xC8		
#define CAN_IO_TEMP8_CMD   0xC9	

//Input and Output count
#define CAN_IO_INPUT_CNT   5
#define CAN_IO_OUTPUT_CNT  2

//CAN INV Signal define(0~15)
#define	CAN_SIG_INV_POWER_ON		0
#define	CAN_SIG_INV_CAN_COMM_ERROR		1

//CAN IO Signal define(0~15)
#define	CAN_SIG_IO_CAN_COMM_ERROR	0

//macro funtion
#define TEMP_HIGH_BYTE(x,y)	(y |= (x << 8) & 0xFF00)
#define TEMP_LOW_BYTE(x,y)	(y |= (x) & 0x00FF)


#endif
