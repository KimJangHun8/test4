#ifndef __NV_CONFIG_H__
#define __NV_CONFIG_H__

//NorthVolt function
//INC/NorthVolt_Patch_Note.txt  Check!!
#ifdef _NorthVolt
	#define REAL_TIME 	1
	#define VERSION_DETAIL_SHOW	1
	#define _GUI_OPCUA_TYPE
	#define _CH_CHAMBER_DATA
	#define _USER_VI
	#if NETWORK_VERSION == 4102
		#define CH_AUX_DATA 0
	#elif NETWORK_VERSION == 4103
		#define CH_AUX_DATA 1
	#endif
	#if MAIN_P_VER1 == 4001
	#elif MAIN_P_VER1 == 4002
		#define	_TEMP_CALI
	#else
	#endif
#endif

#endif
