#ifndef __DATASAVE_STR_H__
#define __DATASAVE_STR_H__

#include "SysDefine.h"
#include "DataSave_def.h"

typedef struct s_data_save_misc_tag {
	char				psName[16]; //process name : warning don't move
} S_DATA_SAVE_MISC;

typedef struct s_data_save_config_tag {
	unsigned char		resultData_saveFlag;
	unsigned char		monitoringData_saveFlag;
	unsigned char		checkData_saveFlag;
} S_DATA_SAVE_CONFIG;

typedef struct s_data_save_tag {
	S_DATA_SAVE_MISC	misc; //warning don't move
	S_DATA_SAVE_CONFIG	config;

	unsigned char		signal[MAX_SIGNAL];
//	int					monitorDataSave[MAX_BD_PER_MODULE][MAX_CH_PER_BD];
} S_DATA_SAVE;

#endif
