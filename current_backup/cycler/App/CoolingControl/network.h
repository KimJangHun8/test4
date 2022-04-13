#ifndef __NETWORK_H__
#define __NETWORK_H__

int		InitNetwork(void);
int		NetworkPacket_Receive(void);
int		Parsing_NetworkEvent(void);
void	NetworkPacket_Parsing(void);
int		NetworkCommand_Receive(void);
int		NetworkCommand_Parsing(void);

int		CmdHeader_Check(char *);
int		CheckSum_Check_T54(void);
int 	rcv_cmd_DRS(int);
int 	rcv_cmd_DRR(int);
int 	rcv_cmd_DWS(int);
int 	rcv_cmd_DWR(int);
int 	rcv_cmd_WHO(int);

void	send_cmd_request(int);
void	send_cmd_read_value(int);
void	send_cmd_set_value(int);

void	send_cmd_run_stop_set(int, int);
void 	send_cmd_run_stop_TEMP_2000(int, int);

void 	send_cmd_temp_set(int,short);
void 	send_cmd_temp_set_TEMP_2000(int,short);

int		make_check_sum(char *,int);

int		send_command(char *, int, int);
int		Check_NetworkState(void);
int 	check_cmd_reply_timeout(void);
int		check_network_timeout(void);

#endif
