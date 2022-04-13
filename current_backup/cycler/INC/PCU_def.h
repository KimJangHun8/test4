#ifndef __PCU_DEF_H__
#define __PCU_DEF_H__

//CTRL_BASE_ADDR
#define	CTRL_BASE_ADDR					0x0620
#define CTRL_BASE_ADDR_STEP				0x0020

//System Register
#define	SREGS_BASE_ADDR				CTRL_BASE_ADDR
#define	SREG_TEST1					0x00 //(R/W)
#define	SREG_TEST2					0x01 //(R/W)
#define	SREG_FWID					0x02
#define	SREG_VERMIN					0x03
#define	SREG_VERMAJ					0x04
#define	SREG_VERD2					0x05
#define	SREG_VERD1					0x06
#define	SREG_CTRL1					0x07
#define	SREG_CTRL2					0x08
#define	SREG_STAT1					0x09
#define	SREG_SYSTEM_TYPE			0x0A

//RCU Register
#define	RREG_CTRL_BASE_ADDR			0x0640
#define	RREG_CTRL_REG				0x00	
#define	RREG_CTRL_ADDR				0x01	
#define	RREG_CTRL_CMD				0x02	
#define	RREG_CTRL_PARA1H			0x03	
#define	RREG_CTRL_PARA1L			0x04	
#define	RREG_CTRL_PARA2H			0x05	
#define	RREG_CTRL_PARA2L			0x06	
#define	RREG_CTRL_STATE				0x07	
#define	RREG_CTRL_STATE_REG			0x08	
#define	RREG_CTRL_REL_STATE			0x09	
#define	RREG_CTRL_DIN_STATE			0x0A	

#define	CREG_CTRL_BASE_ADDR			0x0660
#define	CREG_CTRL_REG				0x00	
#define	CREG_CTRL_ADDR				0x01	
#define	CREG_CTRL_CMD				0x02	
#define	CREG_CTRL_PARA1H			0x03	
#define	CREG_CTRL_PARA1L			0x04	
#define	CREG_CTRL_PARA2H			0x05	
#define	CREG_CTRL_PARA2L			0x06	
#define	CREG_CTRL_DUTVH				0x07	
#define	CREG_CTRL_DUTVL				0x08	
#define	CREG_CTRL_DUTIH				0x09	
#define	CREG_CTRL_DUTIL				0x0A	
#define	CREG_CTRL_STATE				0x0B	
#define	CREG_CTRL_CCU1_STATE		0x0C	
#define	CREG_CTRL_CCU2_STATE		0x0D	
#define CREG_CTRL_CCU1_SEQ_NO		0x0E
#define CREG_CTRL_CCU2_SEQ_NO		0x0F
//#define	CREG_CTRL_CCU3_STATE		0x0E	
//#define	CREG_CTRL_CCU4_STATE		0x0F	
/*#define	CREG_CTRL_CCU5_STATE		0x10	
#define	CREG_CTRL_CCU6_STATE		0x11	
#define	CREG_CTRL_HVU1_STATE		0x12	
#define	CREG_CTRL_HVU1VH			0x13	
#define	CREG_CTRL_HVU1VL			0x14	
#define	CREG_CTRL_HVU1IH			0x15	
#define	CREG_CTRL_HVU1IL			0x16	*/
#define CREG_CTRL_DUTVH_CH1			0x10
#define CREG_CTRL_DUTVL_CH1			0x11
#define CREG_CTRL_DUTIH_CH1			0x12
#define CREG_CTRL_DUTIL_CH1			0x13
#define CREG_CTRL_DUTVH_CH2			0x14
#define CREG_CTRL_DUTVL_CH2			0x15
#define CREG_CTRL_DUTIH_CH2			0x16
#define CREG_CTRL_DUTIL_CH2			0x17
/*#define CREG_CTRL_DUTVH_CH3			0x18
#define CREG_CTRL_DUTVL_CH3			0x19
#define CREG_CTRL_DUTIH_CH3			0x1A
#define CREG_CTRL_DUTIL_CH3			0x1B
#define CREG_CTRL_DUTVH_CH4			0x1C
#define CREG_CTRL_DUTVL_CH4			0x1D
#define CREG_CTRL_DUTIH_CH4			0x1E
#define CREG_CTRL_DUTIL_CH4			0x1F */

//address PCU
#define ADDR_PCU_MULTICAST				0x00
#define	ADDR_PCU_CLASS_MULTICAST		0x10
#define	ADDR_PCU_CLASS_1				0x11
#define	ADDR_PCU_CLASS_2				0x12
#define	ADDR_PCU_CLASS_3				0x13
#define	ADDR_PCU_CLASS_4				0x14
#define	ADDR_PCU_CLASS_5				0x15
#define	ADDR_PCU_CLASS_6				0x16
#define	ADDR_HVU_CLASS_MULTICAST		0x20
#define	ADDR_HVU_CLASS_1				0x21
#define	ADDR_RCU_CLASS_MULTICAST		0x30
#define	ADDR_RCU_CLASS_1				0x31

//CMD define SBCIU to PCU
#define CMD_PCU_GET_STATUS				0x00
#define	CMD_PCU_RESET					0x01
#define	CMD_PCU_RANGE_I					0x04	//200409 lyhw
#define	CMD_PCU_ID						0x11

#define	CMD_PCU_MODE_CC					0xA0
#define	CMD_PCU_MODE_CV					0xA1
#define CMD_PCU_MODE_CCCV				0xA2
#define CMD_PCU_MODE_CP					0xA3
#define CMD_PCU_MODE_CR					0xA4
#define CMD_PCU_MODE_CPCV				0xA7
#define CMD_PCU_HVU_ON					0xB0
#define CMD_RCU_RELAY_ONOFF				0xC0
#define CMD_RCU_PARALLEL_ONOFF			0xC1
#define CMD_RCU_INVERTER_ONOFF			0xC2	//180615 add

//PCU CURRENT MODE
#define PCU_MODE_RESET					0x0000
#define PCU_MODE_CC						0x1000
#define PCU_MODE_CC_PULSE				0x2000
#define PCU_MODE_CV						0x3000
#define PCU_MODE_CCCV					0x4000
#define PCU_MODE_CP						0x5000
#define PCU_MODE_CP_PULSE				0x6000
#define PCU_MODE_CR						0x7000
#define PCU_MODE_CPCV					0xA000	//180615 add

//PCU STATE
#define PCU_ERROR_STATE					0x0000
#define PCU_NORMAL						0x0100

//PCU ERROR CODE
#define PCU_CURRENT_NO_ERROR			0x0000
#define PCU_HVU_INPUT_POWER				0x0010
#define PCU_OVP							0x0020
#define PCU_OCP							0x0030
#define PCU_OTP							0x0040
#define PCU_PV_OVP						0x0050
#define PCU_HW_OVP						0x0060
#define PCU_HW_OCP						0x0070
#define PCU_BD_OT						0x0080
#define PCU_SCI_ERR						0x00E0
#define PCU_UNKNOWN_ERR					0x00F0

//PCU COMMAND EXECUTION
#define PCU_CURRENT_CMD_EXE_GOOD		1
#define PCU_CURRENT_CMD_EXE_NG			0

//PCU Cali Cmd
#define CMD_PCU_CALI_V_DATA_CNT			0xD0
#define CMD_PCU_CALI_V_DATA				0xD1
#define CMD_PCU_CALI_I_DATA_CNT			0xD2
#define CMD_PCU_CALI_I_DATA				0xD3
#define CMD_PCU_CALI_MODE				0xD5
#define CMD_PCU_CALI_V_RANGE_POINT		0xD6
#define CMD_PCU_CALI_I_RANGE_POINT		0xD7
#define CMD_PCU_CALI_DATA_SAVE			0xD8

//address INV (CMD code) 180625
#define ADDR_PCU_INV_STATE_1			0x18
#define ADDR_PCU_INV_STATE_2			0x19
#define ADDR_PCU_INV_STATE_3			0x1A
#define ADDR_PCU_INV_STATE_4			0x1B
#define ADDR_PCU_INV_STATE_5			0x1C
#define ADDR_PCU_INV_STATE_6			0x1D
#define ADDR_PCU_INV_STATE_7			0x1E
#define ADDR_PCU_INV_STATE_8			0x1F
//180625 lyhw Digital INV Error
#define P_INV_LAG_SHORT					0x0001
#define P_INV_OVER_CURRENT				0x0002
#define P_INV_OVER_VOLTAGE				0x0004
#define P_INV_PRECHARGE_FAIL			0x0008
#define P_INV_OVER_CURRENT2				0x0010
#define P_INV_CAN_ERR					0x0020
#define P_INV_OVER_LOAD					0x0040
#define P_INV_OVER_HEAT					0x0080
#define P_INV_RESERVED_1				0x0100
#define P_INV_LOW_VOLTAGE				0x0200
#define	P_INV_AC_LOW_VOLTAGE			0x0400
#define P_INV_RESET1					0x0800
#define P_INV_RESET2					0x1000
#define P_INV_AC_INPUT_FAIL				0x2000
#define P_INV_AC_OVER_VOLT				0x4000
#define P_INV_HDC_ERROR					0x8000

//210304 lyhw panda addr
#define ADDR_P_INV_STATE_DC_1			0x18
#define ADDR_P_INV_STATE_DC_2			0x19
#define ADDR_P_INV_STATE_DC_3			0x1A
#define ADDR_P_INV_STATE_DC_4			0x1B

#define ADDR_P_INV_STATE_AC_1			0x1C
#define ADDR_P_INV_STATE_AC_2			0x1D
#define ADDR_P_INV_STATE_AC_3			0x1E
#define ADDR_P_INV_STATE_AC_4			0x1F

//210304 lyhw Digital Panda INV Error
#define P_INV_DC_OVER_TIME				0x0001
#define P_INV_DC_BUS_OVER_V				0x0002
#define P_INV_DC_BUS_LOW_V				0x0004
#define P_INV_DC_OVER_VOLTAGE			0x0008
#define P_INV_DC_LOW_VOLTAGE			0x0010
#define P_INV_DC_OVER_CURRENT			0x0020
#define P_INV_DC_OVER_LOAD				0x0040
#define P_INV_DC_FAULT_LOCK_ERR			0x0080
#define P_INV_DC_SOFT_SHORT				0x0100
#define P_INV_DC_DIP_SWITCH				0x0200
#define	P_INV_DC_PFC_ERR				0x0400

#define P_INV_AC_INPUT_VOLTAGE			0x0001
#define P_INV_AC_INPUT_FRQ				0x0002
#define P_INV_AC_INPUT_CURRENT			0x0004
#define P_INV_AC_PFC_BUS_V				0x0008
#define P_INV_AC_OVER_TIME				0x0010
#define P_INV_AC_OVER_LOAD				0x0020
#define P_INV_AC_OVER_HEAT				0x0040
#define P_INV_AC_FAULT_LOCK_ERR			0x0080

//180710 addr InV
#define MAX_INV_GROUP_NUM				16

//200417 ch Send cmd lyhw
#define P_RANGE0					0
#define P_RANGE1					1
#define P_RANGE2					2
#define P_RANGE3					3
#define P_RANGE4					4
#define P_CC0						5
#define P_RESET						6
#define P_RELAY_ON					7
#define P_RELAY_OFF					8
#define P_PARALLEL_RELAY_ON			9
#define P_PARALLEL_RELAY_OFF		10	
#define P_JW_INV_STOP				11	
#define P_JW_INV_RUN				12	
#define P_JW_INV_RESET				13	

#endif
