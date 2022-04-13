#ifndef __MESSAGE_H__
#define __MESSAGE_H__

void	Check_Message(void);
void	rcv_msg(int);
int		msgParsing_App_to_DataSave(int, int, int, int);
int		msgParsing_Main_to_DataSave(int, int, int, int);
int		msgParsing_Ext_to_DataSave(int, int, int, int);
int		msgParsing_Module_to_DataSave(int, int, int, int);
int		msgParsing_Meter_to_DataSave(int, int, int, int);
int		msgParsing_Module_to_DataSave_Ptn(int, int, int, int, int);	//191120
void	send_msg(int, int, int, int);
void	send_msg_ch_flag(int , char *);
#endif
