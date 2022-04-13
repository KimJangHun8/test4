#ifndef __SK_CONFIG_H__
#define __Sk_CONFIG_H__

//SK function
//INC/SK_Patch_Note.txt  Check!!
#ifdef _SK
	#define REAL_TIME 	1
	#define _TEMP_CALI
	#define _SK_CALI_TYPE
	#define	_AC_FAIL_RECOVERY
	#define VERSION_DETAIL_SHOW	1
	#if NETWORK_VERSION == 4102
		#define CH_AUX_DATA 0
	#elif NETWORK_VERSION == 4103
		#define CH_AUX_DATA 1
	#endif
	#if MAIN_P_VER1 == 2001
	#elif MAIN_P_VER1 == 2002
		#define _TRACKING_MODE
	#else
	#endif
#endif

#endif
