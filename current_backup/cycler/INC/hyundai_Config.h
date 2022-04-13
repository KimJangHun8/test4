#ifndef __HYUNDAI_CONFIG_H__
#define __HYUNDAI_CONFIG_H__

//Hyundai function
//INC/HYUNDAI_Patch_Note.txt  Check!!
#ifdef _HYUNDAI
	#define REAL_TIME 	0
	#define VERSION_DETAIL_SHOW	1
	#define CH_AUX_DATA 0
	#define	SHUNT_R_RCV	1
	#if MAIN_P_VER1 == 5001
		#define _USER_VI
	#else
		#define _USER_VI
	#endif
#endif
#endif
