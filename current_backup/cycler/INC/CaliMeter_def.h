#ifndef __CALIMETER_DEF_H__
#define __CALIMETER_DEF_H__

//packet
#define	MAX_CALI_METER_PACKET_LENGTH	(256*10)
#define MAX_CALI_METER_PACKET_COUNT		32

//cmd
#define	CALI_METER_SEND_CMD_REQUEST		0x01
#define CALI_METER_SEND_CMD_TEMP		0x02
#define CALI_METER_SEND_CMD_CONTROL		0x03

//CaliMeter Signal
#define CALI_METER_SIG_INITIALIZE		0
#define CALI_METER_SIG_REQUEST_PHASE	1
#define CALI_METER_SIG_LAN_CONNECT		2	//161220 lyh add
#define CALI_METER_SIG_LAN_USE			3	//161220 lyh add

//readType
#define READ_V_V						0
#define READ_V_I						1

//measureI
#define MEASURE_I_1						1	//shunt 0.010ohm
#define MEASURE_I_2						2	//shunt 0.001ohm
#define MEASURE_I_3						3	//DCCT 600A/400mA
#define MEASURE_I_4						4	//DCCT 150A/200mA
#define MEASURE_I_5						5	//shunt 10ohm, 100ohm
#define MEASURE_I_6						6	//meter DCI
#define MEASURE_I_7						7	//shunt 0.1mOhm
#define MEASURE_I_8						8	//shunt 0.1Ohm
#define MEASURE_I_9						9	//shunt 0.1Ohm

// cali mode
#define CALI_MODE_NORMAL			0
#define CALI_MODE_CHECK				1

#define CALI_V						0
#define CALI_I						1

//140701 nam w : actual measure
#define CALI						0
#define	MEAS						1

#endif
