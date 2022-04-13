#ifndef __STANDARDINPUT_H__
#define __STANDARDINPUT_H__

void	StandardInput_Receive(void);
int		WaitKeyInput(void);
int		Parsing_StandardInput(char *);
void	UserCommand_Parsing(int, int);
int		SysProcessing(int, int);
int		ExecProcessing(int, int);
int		LogProcessing(int, int);
#endif
