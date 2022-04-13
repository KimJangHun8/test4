#ifndef __VERSION_H__
#define __VERSION_H__
	
#define RELEASE_VERSION1	2
#define RELEASE_VERSION2	2
#define RELEASE_VERSION3	0
#define RELEASE_VERSION4	4
#define RELEASE_VERSION5	0
#define RELEASE_VERSION6	6
#define RELEASE_VERSION7	0
#define RELEASE_VERSION8	6

////////////////////////////////
//Deafult 					  //
//LGES    : 1xxx-S01-R001-N01 //
//SK	  : 2xxx-S01-R001-N01 //
//SDI	  : 3xxx-S01-R001-N01 //
//NV	  : 4xxx-S01-R001-N01 //
//Hyundai : 5xxx-S01-R001-N01 //
//ETC	  : 6xxx-S01-R001-N01 //
//DYSON	  :	61xx-S01-R001-N01 //
//////////////////////////////////////////////////////////////////
//MAIN_P_VER1 : Large-scale program change(GUI Ver = SBC Ver)   //
//              (GUI<->SBC Connect possible)	                //
//SUB_P_VER   : Small program change (GUI Ver = SBC Ver)        //
//              (GUI<->SBC charge/discharge possible)           //
//MAIN_R_VER  : Only SBC Function Add (Customer manager request)//
//SUB_R_VER   : Bug Fix											//
//if MAIN_P_VER1, SUB_P_VER, MAIN_R_VER, SUB_R_VER Add 			//
//Path : INC/Patch_Note.txt Write							    //
//////////////////////////////////////////////////////////////////

#if VENDER == 1		//LG, NortVolt, Hyundai, DYSON, ETC
	#define MAIN_P_VER1 1005

	#if MAIN_P_VER1 < 2000
		#define	_LGES
	#elif MAIN_P_VER1 < 5000 
		#define	_NorthVolt
	#elif MAIN_P_VER1 < 6000 
		#define	_HYUNDAI
	#elif MAIN_P_VER1 < 6200 
		#define	_DYSON
	#elif MAIN_P_VER1 < 7000 
		#define	_ETC
	#endif

	#define SUB_P_VER1 'S'
	#define SUB_P_VER   2

	#define MAIN_R_VER1 'R'
	#define MAIN_R_VER   1

	#define SUB_R_VER1 'N'
	#define SUB_R_VER	1
#elif VENDER == 2  //SDI
	#define	_SDI
	#define MAIN_P_VER1 3002
	
	#define SUB_P_VER1 'S'
	#define SUB_P_VER   1

	#define MAIN_R_VER1 'R'
	#define MAIN_R_VER   1

	#define SUB_R_VER1 'N'
	#define SUB_R_VER	1

#elif VENDER == 3 //SK
	#define	_SK
	#define MAIN_P_VER1 2002
	
	#define SUB_P_VER1 'S'
	#define SUB_P_VER   1

	#define MAIN_R_VER1 'R'
	#define MAIN_R_VER   1

	#define SUB_R_VER1 'N'
	#define SUB_R_VER	1

#endif

#endif
