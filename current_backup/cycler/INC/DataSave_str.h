#ifndef __DATASAVE_STR_H__
#define __DATASAVE_STR_H__

#include "DataSave_def.h"

typedef struct s_data_save_misc_tag {
	int					processPointer;
} S_DATA_SAVE_MISC;

typedef struct s_data_save_config_tag {
	unsigned char		resultData_saveFlag;
	unsigned char		monitoringData_saveFlag;
	unsigned char		checkData_saveFlag;
	unsigned char		zero_sec_data_save;
	unsigned char		save_data_type;
	unsigned char		reserved[2];
	int					save_10ms_time;
	int					save_50ms_time;
	int					save_100ms_time;
	unsigned char		AuxData_saveFlag;
	
	int					saveCond_Check_time;		//190901 lyhw
	int					pause_data_time;			//190901 lyhw
} S_DATA_SAVE_CONFIG;

typedef struct s_data_save_result_data_tag {
	unsigned int		fileIndex;
	unsigned char		open_year;
	unsigned char		open_month;
	unsigned char		open_day;
	unsigned char		reserved;
	unsigned long		resultIndex;
} S_DATA_SAVE_RESULT_DATA;

typedef struct s_data_save_tag {
	unsigned char		signal[MAX_SIGNAL];

	S_DATA_SAVE_MISC	misc;
	S_DATA_SAVE_CONFIG	config;
	S_DATA_SAVE_RESULT_DATA	resultData[MAX_CH_PER_MODULE];
	int					restoreUseFlag;
	int					maxFileCount;
	int					autoRestoreUse;
	int					min_save_peroide;
	int					currentFile;
} S_DATA_SAVE;
#endif
