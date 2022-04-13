#ifndef __MESSAGE_H__
#define __MESSAGE_H__

void	Check_Message(void);
void	rcv_msg(int);
int		msgParsing_App_to_Module(int, int, int, int);
int		msgParsing_MainClient_to_Module(int, int, int, int);
int		msgParsing_DataSave_to_Module(int, int, int, int);
int		msgParsing_Meter_to_Module(int, int, int, int);
int		msgParsing_PSKill_to_Module(int, int, int, int);
int		msgParsing_CaliMeter2_to_Module(int, int, int, int); //20160229

void	send_msg(int, int, int, int);
void	send_msg_ch_flag(int , char *);
void	send_msg_2(int, int, int, int, int, long);  //20190527
void	send_msg_ptn(int, int, int, int, int); 		//20191120

void	send_save_msg(int, int, unsigned long, int);
void	send_save_msg_2(int, int, unsigned long, int, int);
void	send_save_msg_aux(int, int, int, int);

void	send_pulse_msg(int, int, int);
void	send_pulse_msg_2(int, int, int, int);
void	send_pulse_msg_3(int, int, int, int);
void	send_10ms_msg_2(int, int, unsigned long, int, int);
#endif
