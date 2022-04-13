#ifndef __DATASTORE_H__
#define __DATASTORE_H__

#include "SysConfig.h"
#include "SysDefine.h"
#include "Version.h"
#include "lges_Config.h"
#include "northVolt_Config.h"
#include "sk_Config.h"
#include "sdi_Config.h"
#include "hyundai_Config.h"
#include "dyson_Config.h"
#include "Message_str.h"
#include "AppControl_str.h"
#include "MainClient_str.h"
#include "ExtClient_str.h"
#include "DataSave_str.h"
#include "CaliMeter_str.h"
#include "AnalogMeter_str.h"
#include "FndMeter_str.h"
#include "FADM_str.h"
#include "AutoUpdate_str.h"
#include "ModuleControl_str.h"
#include "DInOutControl_str.h"
#include "DAQ_str.h"
#include "PCU_def.h"
#include "CAN_str.h"
#include "DAQ_Client_str.h"
#include "CoolingControl_str.h"

typedef struct s_logfile_tag {
	char				LogPath[128];
	char				LogFile[128];
	char				OpenLogFile[128];
	int					LogDirection;
} S_LOGFILE;
#pragma pack(1)
typedef struct s_cali_point_tag {
	unsigned char		setPointNum;
    unsigned char		checkPointNum;
    unsigned char		charge_pointNum;
    unsigned char		discharge_pointNum;
	long				setPoint[MAX_CALI_POINT];
	long				checkPoint[MAX_CALI_POINT];
#if SHUNT_R_RCV >= 1
	double				shuntValue;
	unsigned char		shuntSerialNo[MAX_SHUNT_SERIAL_LENGTH];
#endif
#if SHUNT_R_RCV == 2	//180515 add for hallct
	double				shuntValue2;
	double				meter_offset[MAX_OFFSET_POINT];
#endif
} S_CALI_POINT;
#pragma pack()
typedef struct s_cali_tmp_cond_tag {
	int					type;
	int					range;
	int					mode;
	S_CALI_POINT		point[MAX_TYPE][MAX_RANGE];
} S_CALI_TMP_COND;

typedef struct s_cali_tmp_data_tag {
	unsigned char		caliFlag[MAX_TYPE][MAX_RANGE];
	double				set_ad[MAX_TYPE][MAX_RANGE][MAX_CALI_POINT];
	double				set_meter[MAX_TYPE][MAX_RANGE][MAX_CALI_POINT];
	double				check_ad[MAX_TYPE][MAX_RANGE][MAX_CALI_POINT];
	double				check_meter[MAX_TYPE][MAX_RANGE][MAX_CALI_POINT];
	double				DA_A[MAX_TYPE][MAX_RANGE][MAX_CALI_POINT-1];
	double				DA_B[MAX_TYPE][MAX_RANGE][MAX_CALI_POINT-1];
	double				AD_A[MAX_TYPE][MAX_RANGE][MAX_CALI_POINT-1];
	double				AD_B[MAX_TYPE][MAX_RANGE][MAX_CALI_POINT-1];
	double				AD_Ratio[MAX_TYPE][MAX_RANGE][2];
#ifdef _SK_CALI_TYPE
	double				set_meterValue[MAX_TYPE][MAX_RANGE][MAX_CALI_POINT];
	double				check_meterValue[MAX_TYPE][MAX_RANGE][MAX_CALI_POINT];
#endif
} S_CALI_TMP_DATA;

typedef struct s_cali_data_tag {
	S_CALI_POINT		point[MAX_TYPE][MAX_RANGE];
	double				set_ad[MAX_TYPE][MAX_RANGE][MAX_CALI_POINT];
	double				set_meter[MAX_TYPE][MAX_RANGE][MAX_CALI_POINT];
	double				check_ad[MAX_TYPE][MAX_RANGE][MAX_CALI_POINT];
	double				check_meter[MAX_TYPE][MAX_RANGE][MAX_CALI_POINT];
	double				DA_A[MAX_TYPE][MAX_RANGE][MAX_CALI_POINT-1];
	double				DA_B[MAX_TYPE][MAX_RANGE][MAX_CALI_POINT-1];
	double				AD_A[MAX_TYPE][MAX_RANGE][MAX_CALI_POINT-1];
	double				AD_B[MAX_TYPE][MAX_RANGE][MAX_CALI_POINT-1];
	double				AD_Ratio[MAX_TYPE][MAX_RANGE][2];
} S_CALI_DATA;

typedef struct s_calibration_tag {
	double				orgAD[MAX_TYPE];
	double				orgAD_fad[MAX_TYPE];
	double				orgAD_caliMeter2[MAX_TYPE];
	double				line_impedance;
	double				line_drop;
	double				line_drop_i;
	S_CALI_TMP_COND		tmpCond[MAX_BD_PER_MODULE][MAX_CH_PER_BD];
	S_CALI_TMP_DATA		tmpData[MAX_BD_PER_MODULE][MAX_CH_PER_BD];
	S_CALI_DATA			data[MAX_BD_PER_MODULE][MAX_CH_PER_BD];

	S_CALI_TMP_DATA		tmpData_caliMeter2[MAX_BD_PER_MODULE][MAX_CH_PER_BD];
	S_CALI_DATA			data_caliMeter2[MAX_BD_PER_MODULE][MAX_CH_PER_BD];

	S_CALI_TMP_DATA		tmpData_fad[MAX_BD_PER_MODULE][MAX_CH_PER_BD];
	S_CALI_DATA			data_fad[MAX_BD_PER_MODULE][MAX_CH_PER_BD];
} S_CALIBRATION;

#ifdef _TEMP_CALI  
typedef struct s_temp_cali_point_set_tag { //200102 add
	unsigned char		setPointCount;
	unsigned char		caliFlagCount;
	unsigned char		reserved1[2];
	long				setTempPoint[MAX_TEMP_POINT];
} S_TEMP_CALI_POINT;

typedef struct s_temp_cali_data_tag { //200102 add
	unsigned char		temp_caliFlag[MAX_TEMP_POINT][MAX_TEMP_CH];
	long				setTempValue[MAX_TEMP_POINT][MAX_TEMP_CH];
} S_TEMP_CALI_DATA;

typedef struct s_temp_cali_measure_tag {
	float				gain[MAX_TEMP_POINT][MAX_TEMP_CH];
	float				offset[MAX_TEMP_POINT][MAX_TEMP_CH];	
} S_TEMP_CALI_MEASURE;

typedef struct s_temp_calibration_tag {
	S_TEMP_CALI_DATA		data;
   	S_TEMP_CALI_POINT		point;
	S_TEMP_CALI_MEASURE		measure;			
} S_TEMP_CALIBRATION;
#endif

typedef struct s_ch_tag {
	short int			number1;
	short int			number2;
	short int			bd;
	short int			ch;
} S_CH;

typedef struct s_system_data_tag {
	S_MSG				msg[MAX_MSG_RING];
	S_SAVE_MSG			save_msg[2];
	S_PULSE_MSG			pulse_msg[3][MAX_BD_PER_MODULE][MAX_CH_PER_BD];
	S_PULSE_MSG			pulse_msg_iec[3][MAX_BD_PER_MODULE][MAX_CH_PER_BD];
	S_10MS_MSG			save10ms_msg[4][MAX_BD_PER_MODULE][MAX_CH_PER_BD];

	S_LOGFILE			log[10];

	S_CALIBRATION		cali;

	S_CH				CellArray1[MAX_CH_PER_MODULE]; //index : monitor_no
	S_CH				CellArray2[MAX_CH_PER_MODULE]; //index : hw_no
	S_CH				TempArray1[MAX_TEMP_CH]; //index : monitor_no
	S_CH				TempArray2[MAX_TEMP_CH]; //index : hw_no
	S_CH				ChamArray[MAX_CH_PER_MODULE]; //20160120 add
	S_CH				ChamberChNo[MAX_CH_PER_MODULE]; //index : chamberNo
	
	S_APP_CONTROL		AppControl;
	S_MAIN_CLIENT		MainClient;
	S_EXT_CLIENT		ExtClient;
	S_DATA_SAVE			DataSave;
	S_CALI_METER		CaliMeter;
	S_CALI_METER		CaliMeter2;
	S_ANALOG_METER		AnalogMeter;
	S_ANALOG_METER		AnalogMeter2;
	S_FND_METER			FndMeter;
	S_FADM				FADM;
	S_AUTO_UPDATE		AutoUpdate;	//20190626 oys
	S_DAQ_CLIENT		DAQ_Client;	//220310 hun		
	S_COOLING_CONTROL	CoolingControl;	//22020302 jws add	
		
    S_MODULE_DATA		mData;
    S_BD_DATA			bData[MAX_BD_PER_MODULE];    
	S_DIGITAL_INOUT		dio;
	S_DAQ_DATA			daq;
	S_CAN				CAN; //20190527 KHK
	S_MAIN_AUX_DATA		auxData[MAX_AUX_DATA];
	#ifdef _TEMP_CALI  
	S_TEMP_CALIBRATION	temp_cali;  //200102 add
	#endif

	int					auxDataCount[MAX_CH_PER_MODULE][MAX_AUX_TYPE];
	#if AUX_CONTROL == 1
		#if NETWORK_VERSION >= 4103
		S_AUX_MISC			auxMisc;
		#endif
	#endif
	S_AUX_SET_DATA		auxSetData[MAX_AUX_DATA];
	unsigned char		test_val_c[MAX_TEST_VALUE];
	long				test_val_l[MAX_TEST_VALUE];
	long				test_val_c1[MAX_TEST_VALUE];
	float				test_val_f[MAX_TEST_VALUE];
	//110621 oys w : for AnalogMeter Process Kill (restart)
	long				serialCheckTime;
	long				serialCheckTime2;
	int					cooling_connected;
} S_SYSTEM_DATA;
#endif
