#ifndef __FADM_DEF_H__
#define __FADM_DEF_H__

//packet
#define	MAX_FADM_PACKET_LENGTH			(256*10)
#define MAX_FADM_PACKET_COUNT			32

//cmd

//FADM Signal
#define FADM_SIG_INITIALIZE				0
#define FADM_SIG_MEASURE				1
#define FADM_SIG_MEASURE_ERROR			2
#define FADM_SIG_READ_CH				3
#define FADM_SIG_MEASURE_READY			4
#define FADM_SIG_READ_CH_READY			5

//etc
#define MAX_FADM_USE_CH					32

#endif
