#ifndef __SYSCONFIG_H__
#define __SYSCONFIG_H__

//SBC : 0, Vmware : 1
#define SYSTEM_TYPE		0

#define LINEAR_CYC		0
#define DIGITAL_CYC		1
#define CAN_CYC			2

//LINEAR : 0, DIGITAL : 1, CAN : 2
#define CYCLER_TYPE		1
#define _CYCLER
//#define _PACK_CYCLER
#if CYCLER_TYPE == 0
	#define _JIG_TYPE_0
#elif CYCLER_TYPE == 1
	#define _JIG_TYPE_3
#elif CYCLER_TYPE == 2
	#define _JIG_TYPE_0
#endif

#define VENDER	3
#define NETWORK_VERSION	4103

#define AUX_CONTROL		0
#define EDLC_TYPE		0
#define MACHINE_TYPE	0
#define CAPACITY_CONTROL	0
#define CHAMBER_TEMP_HUMIDITY 	0
//Lges function
#define	GAS_DATA_CONTROL	0
//NorthVolt function
#define USER_PATTERN_500	0
#define	CH_SWELLING_DATA	0	
//etc function
#define END_V_COMPARE_GOTO	0 //Hyndai sungwoo solite

#define DAEHWA	0

#if VENDER == 1
	#define __LG_VER1__
	#if DAEHWA == 1
		#define CHANGE_VI_CHECK	0
		#define DATA_SAVE_VER	1
		#define VERSION_DETAIL_SHOW	0
	#else
		#define CHANGE_VI_CHECK	1
		#define DATA_SAVE_VER	0
	#endif
	#define PROGRAM_VERSION1	0
	#define PROGRAM_VERSION2	2
	#define PROGRAM_VERSION3	0
	#define PROGRAM_VERSION4	5
#elif VENDER == 2
	#define __SDI_MES_VER4__
	#define CHANGE_VI_CHECK	0
	#define DATA_SAVE_VER	1

	#define PROGRAM_VERSION1	0
	#define PROGRAM_VERSION2	1
	#define PROGRAM_VERSION3	0
	#define PROGRAM_VERSION4	5
#elif VENDER == 3
	#define __LG_VER1__
	#define CHANGE_VI_CHECK	0
	#define DATA_SAVE_VER	0
	#define SHUNT_R_RCV			1

	#define PROGRAM_VERSION1	0
	#define PROGRAM_VERSION2	1
	#define PROGRAM_VERSION3	0
	#define PROGRAM_VERSION4	5
#endif

#endif
