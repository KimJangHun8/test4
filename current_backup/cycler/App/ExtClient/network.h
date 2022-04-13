#ifndef __NETWORK_H__
#define __NETWORK_H__

int		InitNetwork(void);
int		NetworkPacket_Receive(void);
void	NetworkPacket_Parsing(void);
int		NetworkCommand_Receive(void);
int		NetworkCommand_Parsing(void);
int		Parsing_NetworkEvent(void);

int		CmdHeaderCheck(char *);
int		CheckReplyCmd(char *);

int 	Check_NetworkState(void);//
int 	check_network_ping(void);//
int 	network_ping(void);//

int		send_cmd_heartbeat(void);//
int		send_cmd_unknown(int, int); //
int		send_cmd_response(int, int); //
int 	send_cmd_trouble(int); //
int 	send_cmd_module_info(void);//
int 	send_cmd_ch_data(void); //
int 	send_cmd_ch_data_idle(void); //
int 	send_cmd_ch_data_save(void); //

int		rcv_cmd_heartbeat_response(void);//
int		rcv_cmd_response(void); //
int		rcv_cmd_unknown(void); //
int		rcv_cmd_step_data(void);
int		rcv_cmd_run(void);
int		rcv_cmd_stop(void);
int		rcv_cmd_pause(void);
int		rcv_cmd_continue(void);
int		rcv_cmd_nextstep(void);
int		rcv_cmd_init(void);
int		rcv_cmd_parallel(void);
int		rcv_cmd_crc_error(void);

void	make_header(char *, char, int, int, int); //
void 	make_crc(unsigned short , char *, unsigned int); //
int 	check_crc(unsigned short , char *, unsigned int);  //
unsigned short crc16ccitt_compute_buf(unsigned short , char *, unsigned int); //
unsigned short crc16polynomail_buf(unsigned short , char *, unsigned int);
unsigned short crc16modbus_compute_buf(unsigned short , char *, unsigned int);//
#endif
