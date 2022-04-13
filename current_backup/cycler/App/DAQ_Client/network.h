#ifndef __NETWORK_H__
#define __NETWORK_H__

int		InitNetwork(void);
int		NetworkPacket_Receive(void);
void	NetworkPacket_Parsing(void);
int		NetworkCommand_Receive(void);
int		NetworkCommand_Parsing(void);
int		Parsing_NetworkEvent(void);

int		CmdHeader_Check(char *);
int		Check_ReplyCmd(char *);

int 	rcv_cmd_temp_data(void);
float 	Convert_Vth_to_Temp(float, int);

int		send_cmd_module_info(int);

int		rcv_cmd_version_request(int);
int		send_cmd_version_data(int);

int		rcv_cmd_ng_cell(void);
int		rcv_cmd_temp_data(void);

int		rcv_cmd_stop(int);
int		send_cmd_stop(int);

int		send_cmd_pause(int);
int		send_cmd_recipe_download(int);
int		send_cmd_ch_state(int);

int		rcv_cmd_run(int);
int		send_cmd_run(int);

int		send_cmd_test_header(int); //20090625
int		send_cmd_test_step(int); //20090625

int		send_cmd_next_step(int);

int		send_cmd_continue(int);

int		rcv_cmd_check(void);
int		send_cmd_check_data(int, int, int);

int		rcv_cmd_response(int);
int		send_cmd_response(char *, int);

int		rcv_cmd_comm_check(int);
int		send_cmd_comm_check_reply(char *);

int		rcv_cmd_heart_beat(void);
int		send_cmd_heart_beat(void);

int		rcv_cmd_user_cmd(void);
int		send_cmd_user_cmd(int, int);

int		rcv_cmd_step_response(void);
int		send_test_step_rest(int, int); //20090625
int		send_test_step_charge(int, int); //20090625
int		send_test_step_discharge(int, int); //20090625
int		send_test_step_end(int, int);//20090625
int		send_cmd_step_recipe(int); // 20090625

int		send_cmd_bcr_info(int, int);

int		send_test_current(int, int);
int		send_cmd_ch_stop(int, int);
int		send_cmd_ch_work(int, int);

void	make_header(int, int, char *);
void	make_header2(int, int, char *, char *);
int		send_command(char *);
int		send_command2(char *, int);
int		get_reply_cmdid(char *);

int		Check_NetworkState(void);
void	network_ping(void);
int		check_network_timeout(void);
void	check_cmd_reply_timeout(void);

#endif
