#ifndef __DATASTORE_H__
#define __DATASTORE_H__

#include "SysDefine.h"
#include "AppControl_str.h"
#include "DataSave_str.h"
#include "Meter_str.h"

typedef struct s_msg_val_tag {
	int					msg;
	int					ch;
	int					val;
} S_MSG_VAL;

typedef struct s_msg_ch_flag_tag {
	unsigned long		bit_32[8];
} S_MSG_CH_FLAG;

typedef struct s_msg_tag {
	int					write_idx;
	int					read_idx;
	S_MSG_VAL			msg_val[MAX_MSG];
	S_MSG_CH_FLAG		msg_ch_flag[MAX_MSG];
} S_MSG;

typedef struct s_save_msg_tag {
/*	int					write_idx[MAX_BD_PER_MODULE][MAX_CH_PER_BD];
	int					read_idx[MAX_BD_PER_MODULE][MAX_CH_PER_BD];
	int					count[MAX_BD_PER_MODULE][MAX_CH_PER_BD];
	S_MAIN_CH_DATA
		save_msg_val[MAX_BD_PER_MODULE][MAX_CH_PER_BD][MAX_SAVE_MSG];*/
} S_SAVE_MSG;

typedef struct s_logfile_tag {
	char				LogPath[128];
	char				LogFile[128];
	char				OpenLogFile[128];
	int					LogDirection;
} S_LOGFILE;

typedef struct s_system_data_tag {
	S_MSG				msg[MAX_MSG_RING];
	S_SAVE_MSG			save_msg;
	S_LOGFILE			log[5];

	S_APP_CONTROL		AppControl;
	S_DATA_SAVE			DataSave;
	S_METER				Meter1;
	S_METER				Meter2;
		
	unsigned char		test_val_c[MAX_TEST_VALUE];
	long				test_val_l[MAX_TEST_VALUE];
} S_SYSTEM_DATA;
#endif
