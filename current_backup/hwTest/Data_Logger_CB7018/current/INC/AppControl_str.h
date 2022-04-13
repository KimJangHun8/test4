#ifndef __APPCONTROL_STR_H__
#define __APPCONTROL_STR_H__

#include "SysDefine.h"
#include "AppControl_def.h"

typedef struct s_app_misc_tag {
	long				quitDelayTime;
	long				processCheckTime;
} S_APP_MISC;

typedef struct s_app_config_tag {
	char				projectPath[128];
	char				modelName[128];
	int					moduleNo;
	unsigned char		bootOnStart;
	unsigned char		DebugLogFlag;
	unsigned char		sbcType;
	unsigned char		reserved1;
	unsigned int		systemType;
	char				location[8];
	unsigned int		osVersion;
	int					debugType;
} S_APP_CONFIG;

typedef struct s_app_control_tag {
	S_APP_MISC			misc;
	S_APP_CONFIG		config;

	unsigned char		signal[MAX_SIGNAL];
} S_APP_CONTROL;

#endif
