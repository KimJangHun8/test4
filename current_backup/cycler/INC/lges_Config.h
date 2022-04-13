#ifndef __LG_CONFIG_H__
#define __LG_CONFIG_H__

//Lges function
//INC/LGES_Patch_Note.txt  Check!!
#ifdef _LGES
	#define REAL_TIME 	1
	#define VERSION_DETAIL_SHOW	1
	#define CH_AUX_DATA 0
	#define	SHUNT_R_RCV	1
	#if MAIN_P_VER1 == 1001
		#define _USER_VI
		#define	FAULT_CONFIG_VERSION	0
		#define RESERVED 0
	#elif MAIN_P_VER1 == 1003
		#define RESERVED 0
		#define _USER_VI
		#define _END_COMPARE_GOTO
		#if SUB_P_VER == 1
			#define	FAULT_CONFIG_VERSION	0
			#if MAIN_R_VER == 1
			#endif
		#endif
		#define RESERVED 0
	#elif MAIN_P_VER1 == 1004
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
		#if SUB_P_VER == 3
			#define _GROUP_ERROR
		#endif
		#define RESERVED 0
	#elif MAIN_P_VER1 == 1005
		#define _USER_VI
		#define _END_COMPARE_GOTO
		#define	_CH_SWELLING_DATA
		#define _AMBIENT_GAS_FLAG
		#define	_CH_CODE_CONVERT
		#define	FAULT_CONFIG_VERSION	1
		#define	_EQUATION_CURRENT
		#define RESERVED 1
		#define _IMPEDANCE_FUNCTION
	#elif MAIN_P_VER1 == 1101
		#define _USER_VI
		#define _END_COMPARE_GOTO
		#define	_CH_SWELLING_DATA
		#if SUB_P_VER == 1
			#define _EXTERNAL_CONTROL
			#define	FAULT_CONFIG_VERSION	0
			#if MAIN_R_VER == 1
			#endif
		#endif
		#define RESERVED 0
	#elif MAIN_P_VER1 == 1102
		#define _USER_VI
		#define _END_COMPARE_GOTO
		#define	_CH_SWELLING_DATA
		#if SUB_P_VER == 1
			#define _EXTERNAL_CONTROL
			#define _AMBIENT_GAS_FLAG
			#define	_CH_CODE_CONVERT
			#define	FAULT_CONFIG_VERSION	1
			#if MAIN_R_VER == 1
			#endif
		#endif
		#define RESERVED 0
	#elif MAIN_P_VER1 == 1103
		#define _USER_VI
		#define _END_COMPARE_GOTO
		#define	_CH_SWELLING_DATA
		#if SUB_P_VER == 1
			#define _EXTERNAL_CONTROL
			#define _AMBIENT_GAS_FLAG
			#define	_CH_CODE_CONVERT
			#define	FAULT_CONFIG_VERSION	1
			#if MAIN_R_VER == 1
			#endif
		#endif
		#define RESERVED 1
		#define _IMPEDANCE_FUNCTION
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
