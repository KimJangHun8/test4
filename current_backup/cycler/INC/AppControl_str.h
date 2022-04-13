#ifndef __APPCONTROL_STR_H__
#define __APPCONTROL_STR_H__

#include "AppControl_def.h"

typedef struct s_app_misc_tag {
	long				quitDelayTime;
	long				processCheckTime;
	int					processPointer;
	char				saveFlag;
	char				sbcIpAddr[16];
} S_APP_MISC;

typedef struct s_app_config_tag {
	char				modelName[128];
	int					moduleNo;
	unsigned char		bootOnStart;
	unsigned char		DebugLogFlag;
	unsigned char		sbcType;
	unsigned char		reserved1;
	unsigned int		systemType;
	unsigned int		osVersion;
	int					debugType;
	unsigned int		versionNo;
} S_APP_CONFIG;

typedef struct s_app_control_tag {
	S_APP_MISC			misc;
	S_APP_CONFIG		config;
	//110429 loadprocess add
	unsigned char		loadProcess[MAX_PROCESS_NUMBER];
	unsigned char		signal[MAX_SIGNAL];
	//150512 oys add : chData Backup, Restore
	unsigned char		backup[MAX_CH_PER_MODULE];
	unsigned char		restore[MAX_CH_PER_MODULE];
	unsigned char		backupFlag[MAX_CH_PER_MODULE];
	unsigned char		restoreFlag[MAX_CH_PER_MODULE];
} S_APP_CONTROL;
#endif
