#ifndef __ANALOGMETER_DEF_H__
#define __ANALOGMETER_DEF_H__

//packet
#define	MAX_ANALOG_METER_PACKET_LENGTH	(256*20)
#define MAX_ANALOG_METER_PACKET_COUNT	128

//cmd
#define	ANALOG_METER_SEND_CMD_REQUEST	0x01
#define ANALOG_METER_SEND_CMD_TEMP		0x02
#define ANALOG_METER_SEND_CMD_CONTROL	0x03

//AnalogMeter Signal
#define ANALOG_METER_SIG_INITIALIZE			0
#define ANALOG_METER_SIG_MEASURE			1
#define ANALOG_METER_SIG_MEASURE_ERROR		2
#define ANALOG_METER_SIG_COMM_BUS_ENABLE	3
#define ANALOG_METER_SIG_CALI_NORMAL		4
#define ANALOG_METER_COUNT					5

//ambient, Gas Modulde Start Num
#define	AMBIENT_MODULE_NO	31
#define	GAS_MODULE_NO		41

//readType
#define READ_T_K						0	// K type
#define READ_V							1
#define READ_I							2
#define READ_T_T						3	// T type
#define MAX_METER_COUNT					10

#define TEMP_CONNECT_ERROR_VALUE		1000000

#define MAX_ANALOG_METER_CH				10
#endif
