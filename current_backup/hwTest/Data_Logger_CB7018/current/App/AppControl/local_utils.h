#ifndef __LOCAL_UTILS_H__
#define __LOCAL_UTILS_H__

void	ProcessKill(char *, char *);
void 	ProcessCheck(void);
int		ProcessDieCheck(char *, char *);
int		ProcessLoad(char *, char *, int);
void	CloseProcess(char *, int);

void	SaveSharedMemory(void);
int		LoadSharedMemory(char *, char *);
void	InitSharedMemory(void);

int 	Read_AppControl_Config(int);
#endif

