#ifndef __MESSAGE_H__
#define __MESSAGE_H__

void	Check_Message(void);
void	rcv_msg(int);
int		msgParsing_Main_to_App(int, int, int, int);
int		msgParsing_Ext_to_App(int, int, int, int);
int		msgParsing_Module_to_App(int, int, int, int);
int		msgParsing_DataSave_to_App(int, int, int, int);
int		msgParsing_Meter_to_App(int, int, int, int);
int		msgParsing_Cooling_to_App(int, int, int, int);
void	send_msg(int, int, int, int);
void	send_msg_ch_flag(int, char *);
int		msgParsing_Module_to_App_value(int, int, int, int, int, long);//20190527
#endif
