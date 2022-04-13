#ifndef __DINOUTCONTROL_STR_H__
#define __DINOUTCONTROL_STR_H__

#include "DInOutControl_def.h"

typedef struct s_dio_config_tag {
	unsigned int		sensCount;
	unsigned long		dioDelay;

	unsigned long		powerSwitchTimeout;
	unsigned long		resetSwitchTimeout;
	unsigned long		powerFailTimeout1;
	unsigned long		powerFailTimeout2;
	unsigned long		upsBatteryFailTimeout;

	unsigned char		dio_Control_Flag;
	unsigned char		watchdogTimerFlag;
	unsigned char		reserved1[2];

	int					inSignalNo[MAX_DIGITAL_INPUT];
	int					outSignalNo[MAX_DIGITAL_OUTPUT];
} S_DIO_CONFIG;

typedef struct s_digital_in_tag {
	unsigned char		inCountFlag[MAX_DIGITAL_INPUT];
	int					inCount[MAX_DIGITAL_INPUT];
	unsigned char		inSignal[MAX_DIGITAL_INPUT];
	unsigned char		inFlag[MAX_DIGITAL_INPUT];
	unsigned char		inUseFlag[MAX_DIGITAL_INPUT];
	unsigned char		inActiveType[MAX_DIGITAL_INPUT];
	unsigned char		pcu_inUseFlag[MAX_DIGITAL_IN_BYTE];		//180627
	unsigned char		pcu_inActiveType[MAX_DIGITAL_IN_BYTE];	//180627
} S_DIGITAL_IN;

typedef struct s_digital_out_tag {
	unsigned char		sys;
	unsigned char		reserved[3];
	unsigned char		outBit[MAX_DIGITAL_IN_BYTE];
	unsigned char		outSignal[MAX_DIGITAL_OUTPUT];
	unsigned char		outFlag[MAX_DIGITAL_OUTPUT];
	unsigned char		outUseFlag[MAX_DIGITAL_OUTPUT];
} S_DIGITAL_OUT;

typedef struct s_digital_inout_tag {
	S_DIO_CONFIG		config;
	S_DIGITAL_IN		din;
	S_DIGITAL_OUT		dout;

	unsigned char		signal[MAX_SIGNAL];

	unsigned long		delayTimer;
	unsigned long		powerSwitchTimer;
	unsigned long		resetSwitchTimer;
	unsigned long		powerFailTimer;
	unsigned long		upsBatteryFailTimer;

	unsigned long		buzzerOnTimer;
	unsigned long		buzzerOffTimer;
	unsigned long		tmpBuzzerOnTimer;
	unsigned long		tmpBuzzerOffTimer;

	unsigned char		buzzerCount;
	unsigned char		tmpBuzzerCount;
	unsigned char		reserved1[2];
} S_DIGITAL_INOUT;

#endif
