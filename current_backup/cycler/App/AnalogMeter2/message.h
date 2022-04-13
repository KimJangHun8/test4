#ifndef __MESSAGE_H__
#define __MESSAGE_H__

void	Check_Message(void);
void	rcv_msg(int);
int		msgParsing_App_to_Meter2(int, int, int, int);
void	send_msg(int, int, int, int);
void	send_msg_ch_flag(int , char *);
#endif
