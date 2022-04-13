#ifndef __DAQ_STR_H__
#define __DAQ_STR_H__

#include "DAQ_def.h"

typedef struct s_daq_tag {
    short int   ad_ch[MAX_AUX_VOLT_DATA];
    short int   ad_ref[16][3];
} S_DAQ_AD;
typedef struct s_daq_misc_tag {
   long        ad_count;
   long        ch_ad_avr[MAX_AUX_VOLT_DATA];
   long        ref_ad_avr[16][3];
   float       DAQ_AD_A[16];
   float       DAQ_AD_B[16];
   float       DAQ_AD_A_N[16];
   float       DAQ_AD_B_N[16];
   unsigned char caliFlag[MAX_AUX_VOLT_DATA];
   long			low_point[MAX_AUX_VOLT_DATA];
   long			high_point[MAX_AUX_VOLT_DATA];
   long			map[MAX_AUX_VOLT_DATA];
   unsigned long	daq_read_delay;
} S_DAQ_MISC;

// 
typedef struct s_daq_op_data_tag {
   long        	ch_vsens[MAX_AUX_VOLT_DATA];
   long			chData_AuxV[MAX_AUX_VOLT_DATA];
} S_DAQ_OP;
// 
typedef struct s_daq_cali_tag {
   float      	AD_A[MAX_AUX_VOLT_DATA];
   float      	AD_B[MAX_AUX_VOLT_DATA];
} S_DAQ_CALI;

typedef struct s_daq_data_tag {
    S_DAQ_AD 	data[MAX_DAQ_AD_COUNT];
	S_DAQ_MISC  misc;
	S_DAQ_OP	op;
	S_DAQ_OP	org;
	S_DAQ_OP	tmp_low;
	S_DAQ_OP	tmp_high;
	S_DAQ_CALI	cali;
	S_DAQ_CALI	tmpCali;
} S_DAQ_DATA;
#endif
