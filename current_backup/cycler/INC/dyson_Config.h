#ifndef __DYSON_CONFIG_H__
#define __DYSON_CONFIG_H__

//Dyson function
//INC/DYSON_Patch_Note.txt  Check!!
#ifdef _DYSON
	#define REAL_TIME 	1
	#define VERSION_DETAIL_SHOW	1
	#define CH_AUX_DATA 0
	#define	SHUNT_R_RCV	1
	#if MAIN_P_VER1 == 6100
		#define _USER_VI
		#define _END_COMPARE_GOTO
		#define	_CH_SWELLING_DATA
		#define _AMBIENT_GAS_FLAG
		#define	_CH_CODE_CONVERT
		#define	FAULT_CONFIG_VERSION	1
		#if SUB_P_VER == 1
			#if MAIN_R_VER == 1
			#endif
		#endif
		#if SUB_P_VER == 2
			#define	_EQUATION_CURRENT
			#if MAIN_R_VER == 1
			#endif
		#endif
	#else
		#define _USER_VI
		#define _END_COMPARE_GOTO
		#define	FAULT_CONFIG_VERSION	0
	#endif

	#if	FAULT_CONFIG_VERSION == 0
		#define FAULT_CONFIG_LINE 11
	#elif FAULT_CONFIG_VERSION == 1
		#define FAULT_CONFIG_LINE 11
	#endif
#endif

#endif
