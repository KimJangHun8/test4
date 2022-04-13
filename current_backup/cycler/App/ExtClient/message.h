#ifndef __MESSAGE_H__
#define __MESSAGE_H__

void	Check_Message(void);
void	rcv_msg(int);
int		msgParsing_App_to_Ext(int, int, int, int);
int		msgParsing_Module_to_Ext(int, int, int, int);
int		msgParsing_DataSave_to_Ext(int, int, int, int);
int		msgParsing_Meter_to_Ext(int, int, int, int);
void	send_msg(int, int, int, int);
void 	send_msg_ch_flag(int , char *);
#endif
