#ifndef __MESSAGE_H__
#define __MESSAGE_H__

void	Check_Message(int);
void	rcv_msg(int);
int		msgParsing_Module_to_VTED(int, int, int, int);
int 	msgParsing_Sub_to_VTED(int, int, int, int);
int 	msgParsing_LG_to_VTED(int, int, int, int);
void	send_msg(int, int, int, int);
void	send_msg_ch_flag(int , char *);
#endif
