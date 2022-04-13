#ifndef __AUTOUPDATE_STR_H__
#define __AUTOUPDATE_STR_H__

#include "AutoUpdate_def.h"

typedef struct s_auto_update_misc_tag {
	int					processPointer;
	int					timeCnt;
} S_AUTO_UPDATE_MISC;

typedef struct s_auto_update_config_tag {
	unsigned char	useFlag;
	unsigned char	reserved1[3];
	
   	char			ipAddr[16];

	unsigned char	serverConnectFlag;
	unsigned char	updateServerSet;
	unsigned char	reserved2[2];

	int				retryInterval;
} S_AUTO_UPDATE_CONFIG;
	
typedef struct s_auto_update_tag {
	S_AUTO_UPDATE_MISC		misc;
	S_AUTO_UPDATE_CONFIG	config;
	unsigned char		signal[MAX_SIGNAL];
} S_AUTO_UPDATE;

#endif
