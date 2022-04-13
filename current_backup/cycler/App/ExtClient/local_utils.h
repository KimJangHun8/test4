#ifndef __LOCAL_UTILS_H__
#define __LOCAL_UTILS_H__

int		Initialize(void);
void	Init_SystemMemory(void);
int		Read_ExtClient_Config(void);
void	StateChange_Pause(int);
char 	get_ch_state(int, int, int);
char 	get_ch_stepType(int, int, int);
char 	convert_ch_stepType(int, int, int);
char 	get_ch_mode(int, int, int);
char 	convert_ch_mode(int, int, int);
int 	get_ch_code(int, int, int);
void 	Convert_testCond(int , int , int , char *);
#endif
