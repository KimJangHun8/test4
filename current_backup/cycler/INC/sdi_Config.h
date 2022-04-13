#ifndef __SDI_CONFIG_H__
#define __SDI_CONFIG_H__

//SDI function
//INC/SDI_Patch_Note.txt  Check!!
#ifdef _SDI
	#define REAL_TIME 	1
	#define	SHUNT_R_RCV	2
	#define CH_AUX_DATA 0
	#if MAIN_P_VER1 == 3001
		#define VERSION_DETAIL_SHOW 0
		#define	_SDI_SAFETY_V1
	#elif MAIN_P_VER1 == 3002
		#define VERSION_DETAIL_SHOW 1
		#define	_SDI_SAFETY_V1
		#define	_SDI_SAFETY_V2
		#define	_AC_FAIL_RECOVERY
	#elif MAIN_P_VER1 == 3003
		#define VERSION_DETAIL_SHOW 1
		#define	_SDI_SAFETY_V1
		#define	_SDI_SAFETY_V2
		#define	_AC_FAIL_RECOVERY
		#define _ACIR
	#elif MAIN_P_VER1 == 3101
		#define	_SDI_SAFETY_V0
		#define VERSION_DETAIL_SHOW 1
		#if SUB_P_VER == 1
			#define _ULSAN_SDI_SAFETY
		#elif SUB_P_VER == 2
		#endif
	#elif MAIN_P_VER1 == 3111
		#define	_TEMP_CALI
		#define VERSION_DETAIL_SHOW 1
		#define _CH_CHAMBER_DATA
		#define	_CH_SWELLING_DATA
		#define	_SDI_SAFETY_V0
		#if SUB_P_VER == 1
			#define _ULSAN_SDI_SAFETY
		#elif SUB_P_VER == 2
		#endif
	#else
		#define VERSION_DETAIL_SHOW 0
		#define	_SDI_SAFETY_V0
	#endif
#endif

#endif
