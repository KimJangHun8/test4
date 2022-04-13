#ifndef __MESSAGE_H__
#define __MESSAGE_H__

void	MessageCheck(void);
void	rcv_msg(int);
int		msgParsing_App_to_DataSave(int, int, int, int);
int		msgParsing_Meter1_to_DataSave(int, int, int, int);
int		msgParsing_Meter2_to_DataSave(int, int, int, int);
void	send_msg(int, int, int, int);
void	send_msg_ch_flag(int , char *);
#endif
